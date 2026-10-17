// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#include "WindowsAutoStart.h"

#include <QCoreApplication>
#include <QDir>
#include <QFutureWatcher>
#include <QSettings>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>
#include <QtConcurrentRun>

#include <qt_windows.h>
#include <taskschd.h>
#include <sddl.h>
#include <wrl/client.h>
#include <vector>
#include <mutex>

namespace remote::controller::detail {
extern const char kWindowsAutoStartValueName[];
namespace {
using Microsoft::WRL::ComPtr;
constexpr auto kTaskMarker = "RLink interactive elevated autostart v1";
constexpr auto kAutoStartChoice = "app/autoStartEnabled";
constexpr auto kAutoStartArgument = "--autostart";
// Serialize scheduler changes with preference commits. A manual off choice
// must not be overwritten by a concurrently finishing first-run initializer.
std::recursive_mutex kAutoStartOperationMutex;

struct AutoStartInitializationResult { bool enabled; QString error; };
std::mutex kAutoStartInitializationMutex;
QFuture<AutoStartInitializationResult> kAutoStartInitializationFuture;
bool kAutoStartInitializationStarted = false;

QString LegacyAutoStartCommand() {
    return QStringLiteral("\"%1\"").arg(QDir::toNativeSeparators(QCoreApplication::applicationFilePath()));
}

bool MatchesCurrentExecutable(const QString& path) {
    return QDir::cleanPath(QDir::fromNativeSeparators(path)).compare(
        QDir::cleanPath(QCoreApplication::applicationFilePath()), Qt::CaseInsensitive) == 0;
}

bool SaveAutoStartChoice(QSettings& settings, bool enabled, QString* error) {
    settings.setValue(QString::fromLatin1(kAutoStartChoice), enabled);
    settings.sync();
    if (settings.status() == QSettings::NoError) return true;
    if (error) *error = QStringLiteral("保存开机启动选择失败");
    return false;
}

bool ApplyAutoStartDefault(QSettings& settings,
    const std::function<bool(bool, QString*)>& configure, QString* error,
    const std::function<bool(QString*)>& migrate = {}) {
    if (error) error->clear();
    // An explicit false is different from a missing preference. Apply the
    // default once, and retry on a later launch if task registration failed.
    if (settings.contains(QString::fromLatin1(kAutoStartChoice))) {
        return !settings.value(QString::fromLatin1(kAutoStartChoice)).toBool() ||
               !migrate || migrate(error);
    }
    if (!configure(true, error)) return false;
    return SaveAutoStartChoice(settings, true, error);
}

class Bstr final {
public:
    explicit Bstr(const QString& text)
        : value_(SysAllocStringLen(reinterpret_cast<const OLECHAR*>(text.utf16()),
                                   static_cast<UINT>(text.size()))) {}
    ~Bstr() { SysFreeString(value_); }
    operator BSTR() const { return value_; }
private:
    BSTR value_;
};

class TaskConnection final {
public:
    TaskConnection() : initialized_(CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED)) {}
    ~TaskConnection() {
        folder.Reset(); service.Reset();
        if (SUCCEEDED(initialized_)) CoUninitialize();
    }
    HRESULT Open() {
        if (FAILED(initialized_) && initialized_ != RPC_E_CHANGED_MODE) return initialized_;
        HRESULT hr = CoCreateInstance(CLSID_TaskScheduler, nullptr, CLSCTX_INPROC_SERVER,
                                      IID_PPV_ARGS(&service));
        VARIANT empty;
        VariantInit(&empty);
        if (SUCCEEDED(hr)) hr = service->Connect(empty, empty, empty, empty);
        if (SUCCEEDED(hr)) hr = service->GetFolder(Bstr(QStringLiteral("\\")), &folder);
        return hr;
    }
    ComPtr<ITaskService> service;
    ComPtr<ITaskFolder> folder;
private:
    HRESULT initialized_;
};

QString CurrentUserSid() {
    HANDLE token = nullptr;
    if (!OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &token)) return {};
    DWORD bytes = 0;
    GetTokenInformation(token, TokenUser, nullptr, 0, &bytes);
    std::vector<unsigned char> buffer(bytes);
    const bool ok = bytes && GetTokenInformation(token, TokenUser, buffer.data(), bytes, &bytes);
    CloseHandle(token);
    if (!ok) return {};
    LPWSTR sid = nullptr;
    if (!ConvertSidToStringSidW(reinterpret_cast<TOKEN_USER*>(buffer.data())->User.Sid, &sid)) return {};
    const QString result = QString::fromWCharArray(sid);
    LocalFree(sid);
    return result;
}

QString TaskName(const QString& sid) { return QStringLiteral("RLink.AutoStart.%1").arg(sid); }
bool SameAccount(const QString& account, const QString& sid) {
    if (account.compare(sid, Qt::CaseInsensitive) == 0) return true;
    // Task Scheduler preserves the principal SID but can normalize a logon
    // trigger's UserId to DOMAIN\\name when reading back its XML.
    DWORD bytes = 0, domainChars = 0;
    SID_NAME_USE use;
    const std::wstring name = account.toStdWString();
    LookupAccountNameW(nullptr, name.c_str(), nullptr, &bytes, nullptr, &domainChars, &use);
    if (!bytes) return false;
    std::vector<unsigned char> buffer(bytes);
    std::vector<wchar_t> domain(domainChars);
    if (!LookupAccountNameW(nullptr, name.c_str(), buffer.data(), &bytes,
                            domain.data(), &domainChars, &use)) return false;
    LPWSTR resolved = nullptr;
    if (!ConvertSidToStringSidW(buffer.data(), &resolved)) return false;
    const bool matches = QString::fromWCharArray(resolved).compare(sid, Qt::CaseInsensitive) == 0;
    LocalFree(resolved);
    return matches;
}
bool MissingTask(HRESULT hr) {
    return hr == HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND) ||
           hr == HRESULT_FROM_WIN32(ERROR_PATH_NOT_FOUND);
}

QString TaskXml(const QString& sid) {
    QString xml;
    QXmlStreamWriter out(&xml);
    out.writeStartDocument();
    out.writeStartElement(QStringLiteral("Task"));
    out.writeDefaultNamespace(QStringLiteral("http://schemas.microsoft.com/windows/2004/02/mit/task"));
    out.writeAttribute(QStringLiteral("version"), QStringLiteral("1.2"));
    out.writeStartElement(QStringLiteral("RegistrationInfo"));
    out.writeTextElement(QStringLiteral("Description"), QString::fromLatin1(kTaskMarker));
    out.writeEndElement();
    out.writeStartElement(QStringLiteral("Triggers"));
    out.writeStartElement(QStringLiteral("LogonTrigger"));
    out.writeTextElement(QStringLiteral("Enabled"), QStringLiteral("true"));
    out.writeTextElement(QStringLiteral("UserId"), sid);
    out.writeEndElement(); out.writeEndElement();
    out.writeStartElement(QStringLiteral("Principals"));
    out.writeStartElement(QStringLiteral("Principal"));
    out.writeAttribute(QStringLiteral("id"), QStringLiteral("CurrentUser"));
    out.writeTextElement(QStringLiteral("UserId"), sid);
    out.writeTextElement(QStringLiteral("LogonType"), QStringLiteral("InteractiveToken"));
    out.writeTextElement(QStringLiteral("RunLevel"), QStringLiteral("HighestAvailable"));
    out.writeEndElement(); out.writeEndElement();
    out.writeStartElement(QStringLiteral("Settings"));
    out.writeTextElement(QStringLiteral("MultipleInstancesPolicy"), QStringLiteral("IgnoreNew"));
    out.writeTextElement(QStringLiteral("DisallowStartIfOnBatteries"), QStringLiteral("false"));
    out.writeTextElement(QStringLiteral("StopIfGoingOnBatteries"), QStringLiteral("false"));
    out.writeTextElement(QStringLiteral("StartWhenAvailable"), QStringLiteral("true"));
    out.writeTextElement(QStringLiteral("Enabled"), QStringLiteral("true"));
    out.writeTextElement(QStringLiteral("ExecutionTimeLimit"), QStringLiteral("PT0S"));
    out.writeEndElement();
    out.writeStartElement(QStringLiteral("Actions"));
    out.writeAttribute(QStringLiteral("Context"), QStringLiteral("CurrentUser"));
    out.writeStartElement(QStringLiteral("Exec"));
    out.writeTextElement(QStringLiteral("Command"), QDir::toNativeSeparators(QCoreApplication::applicationFilePath()));
    out.writeTextElement(QStringLiteral("Arguments"), QString::fromLatin1(kAutoStartArgument));
    out.writeTextElement(QStringLiteral("WorkingDirectory"), QDir::toNativeSeparators(QCoreApplication::applicationDirPath()));
    out.writeEndElement(); out.writeEndElement(); out.writeEndElement();
    out.writeEndDocument();
    return xml;
}

// Scope ownership checks to our marker and the current account. Never overwrite
// or remove an unrelated task that happens to have the same name.
bool OwnedTask(IRegisteredTask* task, const QString& sid) {
    BSTR raw = nullptr;
    if (FAILED(task->get_Xml(&raw))) return false;
    const QString xml = QString::fromWCharArray(raw);
    SysFreeString(raw);
    QXmlStreamReader in(xml);
    bool marker = false, user = false;
    while (!in.atEnd()) {
        in.readNext();
        if (!in.isStartElement()) continue;
        if (in.name() == QStringLiteral("Description")) marker = in.readElementText() == QString::fromLatin1(kTaskMarker);
        else if (in.name() == QStringLiteral("UserId")) {
            if (!SameAccount(in.readElementText(), sid)) return false;
            user = true;
        }
    }
    return !in.hasError() && marker && user;
}

// Automatic defaults/migration must not repoint another installation's task.
// Explicit user changes can still update the task after moving the EXE.
bool CurrentInstallationTask(IRegisteredTask* task) {
    ComPtr<ITaskDefinition> definition;
    ComPtr<IActionCollection> actions;
    ComPtr<IAction> action;
    ComPtr<IExecAction> exec;
    LONG count = 0;
    if (FAILED(task->get_Definition(&definition)) ||
        FAILED(definition->get_Actions(&actions)) || FAILED(actions->get_Count(&count)) || count != 1 ||
        FAILED(actions->get_Item(1, &action)) || FAILED(action.As(&exec))) return false;
    BSTR raw = nullptr;
    if (FAILED(exec->get_Path(&raw))) return false;
    const bool current = MatchesCurrentExecutable(QString::fromWCharArray(raw));
    SysFreeString(raw);
    if (!current) return false;
    BSTR arguments = nullptr;
    if (FAILED(exec->get_Arguments(&arguments))) return false;
    const QString text = QString::fromWCharArray(arguments);
    SysFreeString(arguments);
    return text.isEmpty() || text == QString::fromLatin1(kAutoStartArgument);
}

bool ActiveTask(IRegisteredTask* task, bool requireEnabled = true, bool legacyArguments = false) {
    VARIANT_BOOL enabled = VARIANT_FALSE;
    if (FAILED(task->get_Enabled(&enabled)) || (requireEnabled && enabled != VARIANT_TRUE)) return false;
    ComPtr<ITaskDefinition> definition;
    ComPtr<IPrincipal> principal;
    ComPtr<IActionCollection> actions;
    ComPtr<IAction> action;
    ComPtr<IExecAction> exec;
    ComPtr<ITriggerCollection> triggers;
    ComPtr<ITrigger> trigger;
    ComPtr<ILogonTrigger> logonTrigger;
    TASK_RUNLEVEL_TYPE level = TASK_RUNLEVEL_LUA;
    TASK_LOGON_TYPE logon = TASK_LOGON_NONE;
    LONG count = 0;
    if (FAILED(task->get_Definition(&definition)) ||
        FAILED(definition->get_Principal(&principal)) ||
        FAILED(principal->get_RunLevel(&level)) || level != TASK_RUNLEVEL_HIGHEST ||
        FAILED(principal->get_LogonType(&logon)) || logon != TASK_LOGON_INTERACTIVE_TOKEN ||
        FAILED(definition->get_Actions(&actions)) || FAILED(actions->get_Count(&count)) || count != 1 ||
        FAILED(actions->get_Item(1, &action)) || FAILED(action.As(&exec)) ||
        FAILED(definition->get_Triggers(&triggers)) || FAILED(triggers->get_Count(&count)) || count != 1 ||
        FAILED(triggers->get_Item(1, &trigger)) || FAILED(trigger.As(&logonTrigger)) ||
        FAILED(trigger->get_Enabled(&enabled)) || enabled != VARIANT_TRUE) return false;
    BSTR triggerUser = nullptr;
    if (FAILED(logonTrigger->get_UserId(&triggerUser))) return false;
    const bool correctUser = SameAccount(QString::fromWCharArray(triggerUser), CurrentUserSid());
    SysFreeString(triggerUser);
    if (!correctUser) return false;
    BSTR arguments = nullptr;
    if (FAILED(exec->get_Arguments(&arguments))) return false;
    const QString argumentText = QString::fromWCharArray(arguments);
    SysFreeString(arguments);
    if (argumentText != (legacyArguments ? QString() : QString::fromLatin1(kAutoStartArgument))) return false;
    BSTR raw = nullptr;
    if (FAILED(exec->get_Path(&raw))) return false;
    const QString path = QString::fromWCharArray(raw);
    SysFreeString(raw);
    return MatchesCurrentExecutable(path);
}

bool Failed(QString* error, const QString& stage, HRESULT hr) {
    if (error) {
        LPWSTR message = nullptr;
        FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                       nullptr, static_cast<DWORD>(hr), 0, reinterpret_cast<LPWSTR>(&message), 0, nullptr);
        *error = QStringLiteral("%1（0x%2）%3").arg(stage)
            .arg(static_cast<quint32>(hr), 8, 16, QLatin1Char('0'))
            .arg(message ? QString::fromWCharArray(message).trimmed() : QString());
        if (message) LocalFree(message);
    }
    return false;
}

bool MigrateBackgroundAutoStart(QString* error) {
    const QString sid = CurrentUserSid();
    if (sid.isEmpty()) return Failed(error, QStringLiteral("读取当前账户身份失败"), E_FAIL);
    TaskConnection connection;
    HRESULT hr = connection.Open();
    if (FAILED(hr)) return Failed(error, QStringLiteral("连接 Windows 任务计划程序失败"), hr);
    ComPtr<IRegisteredTask> task;
    hr = connection.folder->GetTask(Bstr(TaskName(sid)), &task);
    if (MissingTask(hr)) return true;
    if (FAILED(hr)) return Failed(error, QStringLiteral("读取启动任务失败"), hr);
    // Never recreate deleted/disabled tasks or repoint another installation.
    // Re-register only an enabled, fully validated legacy definition.
    if (!OwnedTask(task.Get(), sid) || !CurrentInstallationTask(task.Get()) ||
        !ActiveTask(task.Get(), true, true)) return true;
    return SetWindowsAutoStartEnabled(true, error);
}

bool ConfigureAutoStartDefault(bool enabled, QString* error) {
    const QString sid = CurrentUserSid();
    if (sid.isEmpty()) return Failed(error, QStringLiteral("读取当前账户身份失败"), E_FAIL);
    TaskConnection connection;
    HRESULT hr = connection.Open();
    if (FAILED(hr)) return Failed(error, QStringLiteral("连接 Windows 任务计划程序失败"), hr);
    ComPtr<IRegisteredTask> task;
    hr = connection.folder->GetTask(Bstr(TaskName(sid)), &task);
    if (FAILED(hr) && !MissingTask(hr)) return Failed(error, QStringLiteral("读取启动任务失败"), hr);
    if (task && (!OwnedTask(task.Get(), sid) || !CurrentInstallationTask(task.Get())))
        return Failed(error, QStringLiteral("已有启动任务属于其他安装，默认设置未改动"), E_ACCESSDENIED);
    return SetWindowsAutoStartEnabled(enabled, error);
}
} // namespace

QString WindowsAutoStartCommand() {
    return LegacyAutoStartCommand() + QStringLiteral(" ") + QString::fromLatin1(kAutoStartArgument);
}

bool WindowsAutoStartEnabled() {
    const std::lock_guard lock(kAutoStartOperationMutex);
    const QString sid = CurrentUserSid();
    if (sid.isEmpty()) return false;
    TaskConnection connection;
    if (FAILED(connection.Open())) return false;
    ComPtr<IRegisteredTask> task;
    return SUCCEEDED(connection.folder->GetTask(Bstr(TaskName(sid)), &task)) &&
           OwnedTask(task.Get(), sid) && ActiveTask(task.Get());
}

bool SetWindowsAutoStartEnabled(bool enabled, QString* error) {
    const std::lock_guard lock(kAutoStartOperationMutex);
    if (error) error->clear();
    const QString sid = CurrentUserSid();
    if (sid.isEmpty()) return Failed(error, QStringLiteral("读取当前账户身份失败"), E_FAIL);
    TaskConnection connection;
    HRESULT hr = connection.Open();
    if (FAILED(hr)) return Failed(error, QStringLiteral("连接 Windows 任务计划程序失败"), hr);
    ComPtr<IRegisteredTask> task;
    hr = connection.folder->GetTask(Bstr(TaskName(sid)), &task);
    if (FAILED(hr) && !MissingTask(hr)) return Failed(error, QStringLiteral("读取启动任务失败"), hr);
    if (task && !OwnedTask(task.Get(), sid)) return Failed(error, QStringLiteral("同名任务不属于 RLink，未改动"), E_ACCESSDENIED);
    if (enabled) {
        VARIANT empty; VariantInit(&empty);
        ComPtr<IRegisteredTask> registered;
        hr = connection.folder->RegisterTask(Bstr(TaskName(sid)), Bstr(TaskXml(sid)),
            TASK_CREATE_OR_UPDATE, empty, empty, TASK_LOGON_INTERACTIVE_TOKEN, empty, &registered);
        if (FAILED(hr)) return Failed(error, QStringLiteral("创建管理员启动任务失败"), hr);
        if (!OwnedTask(registered.Get(), sid) || !ActiveTask(registered.Get()))
            return Failed(error, QStringLiteral("启动任务回读校验失败"), E_FAIL);
    } else if (task) {
        hr = connection.folder->DeleteTask(Bstr(TaskName(sid)), 0);
        if (FAILED(hr)) return Failed(error, QStringLiteral("删除启动任务失败"), hr);
    }
    // Migrate only this executable's old Run entry, and only after task creation
    // succeeded. Do not remove another installation's autostart entry.
    QSettings runKey(QStringLiteral("HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Run"), QSettings::NativeFormat);
    const QString name = QString::fromLatin1(kWindowsAutoStartValueName);
    const QString runCommand = runKey.value(name).toString();
    if (runCommand == LegacyAutoStartCommand() || runCommand == WindowsAutoStartCommand()) {
        runKey.remove(name);
        runKey.sync();
        if (runKey.status() != QSettings::NoError)
            return Failed(error, QStringLiteral("清理旧注册表启动项失败"), E_ACCESSDENIED);
    }
    return true;
}

void QueryWindowsAutoStartAsync(QObject* context, std::function<void(bool)> completed) {
    auto* watcher = new QFutureWatcher<bool>(context);
    QObject::connect(watcher, &QFutureWatcher<bool>::finished, context,
        [watcher, completed = std::move(completed)] {
            const bool enabled = watcher->result();
            watcher->deleteLater();
            completed(enabled);
        });
    watcher->setFuture(QtConcurrent::run([] { return WindowsAutoStartEnabled(); }));
}

void InitializeWindowsAutoStartAsync(QObject* context,
    std::function<void(bool, QString)> completed) {
    QFuture<AutoStartInitializationResult> future;
    bool alreadyFinished = false;
    {
        const std::lock_guard lock(kAutoStartInitializationMutex);
        if (!kAutoStartInitializationStarted) {
            kAutoStartInitializationFuture = QtConcurrent::run([] {
                const std::lock_guard operationLock(kAutoStartOperationMutex);
                QSettings settings;
                QString error;
                ApplyAutoStartDefault(settings, ConfigureAutoStartDefault, &error, MigrateBackgroundAutoStart);
                return AutoStartInitializationResult{WindowsAutoStartEnabled(), error};
            });
            kAutoStartInitializationStarted = true;
        } else {
            alreadyFinished = kAutoStartInitializationFuture.isFinished();
        }
        future = kAutoStartInitializationFuture;
    }
    if (alreadyFinished) {
        // Settings may be opened after the user changed the choice. Re-query
        // instead of replaying the initial result or reapplying the default.
        QueryWindowsAutoStartAsync(context,
            [completed = std::move(completed)](bool enabled) { completed(enabled, {}); });
        return;
    }
    auto* watcher = new QFutureWatcher<AutoStartInitializationResult>(context);
    QObject::connect(watcher, &QFutureWatcher<AutoStartInitializationResult>::finished, context,
        [watcher, completed = std::move(completed)] {
            const auto result = watcher->result();
            watcher->deleteLater();
            completed(result.enabled, result.error);
        });
    watcher->setFuture(future);
}

void SetWindowsAutoStartAsync(bool enabled, QObject* context,
    std::function<void(bool, bool, QString)> completed) {
    struct Result { bool success; bool enabled; QString error; };
    auto* watcher = new QFutureWatcher<Result>(context);
    QObject::connect(watcher, &QFutureWatcher<Result>::finished, context,
        [watcher, completed = std::move(completed)] {
            const auto result = watcher->result();
            watcher->deleteLater();
            completed(result.success, result.enabled, result.error);
        });
    watcher->setFuture(QtConcurrent::run([enabled] {
        const std::lock_guard operationLock(kAutoStartOperationMutex);
        QString error;
        bool success = SetWindowsAutoStartEnabled(enabled, &error);
        const bool actualEnabled = WindowsAutoStartEnabled();
        if (success && actualEnabled != enabled) {
            success = false;
            error = QStringLiteral("完成后启动任务状态核对失败");
        }
        if (success) {
            QSettings settings;
            success = SaveAutoStartChoice(settings, enabled, &error);
        }
        return Result{success, actualEnabled, error};
    }));
}
} // namespace remote::controller::detail
