// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#include <algorithm>
#include <chrono>
#include <cstring>
#include <memory>
#include <thread>

#include <Windows.h>

#include <QApplication>
#include <QAbstractItemView>
#include <QCryptographicHash>
#include <QDateTime>
#include <QDir>
#include <QEvent>
#include <QFile>
#include <QFileInfo>
#include <QFont>
#include <QFontDatabase>
#include <QGraphicsDropShadowEffect>
#include <QHelpEvent>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLabel>
#include <QLocalServer>
#include <QLocalSocket>
#include <QPointer>
#include <QRandomGenerator>
#include <QScreen>
#include <QSettings>
#include <QStandardPaths>
#include <QStringList>
#include <QSysInfo>
#include <QSystemTrayIcon>
#include <QTextStream>
#include <QTemporaryDir>
#include <QTimer>
#include <QUuid>
#include <QVBoxLayout>
#include <QWidget>
#include <QtGlobal>

#include "api/make_ref_counted.h"
#include "InProcessSessionEngine.h"
#include "EncoderBenchmarkProfileCache.h"
#include "src/platform/win/WindowsDesktopCaptureSource.h"
#include "src/platform/win/WindowsHardwareFingerprint.h"
#include "src/platform/win/MfD3D11H264DecoderBenchmark.h"
#include "src/platform/win/FfmpegD3D11H264Decoder.h"
#include "src/platform/win/H264EncoderBenchmark.h"
#include "src/platform/win/VideoEncoderProbePolicy.h"
#include "src/platform/win/FfmpegX264H264Encoder.h"
#include "src/platform/win/FfmpegHardwareH264Encoder.h"
#include "src/platform/win/VideoDecoderProbePolicy.h"
#include "src/apps/controller/ControllerMainWindow.h"
#include "src/apps/controller/RemoteCDialog.h"
#include "src/apps/remote/RemoteCApplicationCoordinator.h"
#include "src/auth/AuthConfig.h"
#include "src/auth/DpapiTokenStore.h"
#include "src/signaling/QtWebSocketSignalingClient.h"

namespace {

constexpr auto kInstallationMetadataFileName =
    "remotec-installation.json";

QString RegisterBundledFont(const QString& resourcePath,
                            const QString& fallbackFamily)
{
    const int fontId = QFontDatabase::addApplicationFont(resourcePath);
    if (fontId < 0) {
        return fallbackFamily;
    }
    const QStringList families =
        QFontDatabase::applicationFontFamilies(fontId);
    return families.isEmpty() ? fallbackFamily : families.front();
}

QString InstalledApplicationVersion()
{
    QFile metadataFile(QDir(QCoreApplication::applicationDirPath())
                           .filePath(QString::fromLatin1(
                               kInstallationMetadataFileName)));
    if (!metadataFile.open(QIODevice::ReadOnly) ||
        metadataFile.size() > 4096) {
        return {};
    }
    QJsonParseError parseError;
    const QJsonDocument document = QJsonDocument::fromJson(
        metadataFile.readAll(), &parseError);
    if (parseError.error != QJsonParseError::NoError ||
        !document.isObject()) {
        return {};
    }
    const QJsonObject metadata = document.object();
    if (metadata.value(QStringLiteral("schemaVersion")).toInt() != 1 ||
        metadata.value(QStringLiteral("product")).toString() !=
            QStringLiteral("RLink")) {
        return {};
    }
    const QString version =
        metadata.value(QStringLiteral("version")).toString().trimmed();
    if (version.isEmpty() || version.size() > 32) {
        return {};
    }
    return version;
}

class RemoteCToolTipBubble final : public QWidget {
public:
    RemoteCToolTipBubble()
        : QWidget(nullptr, Qt::ToolTip | Qt::FramelessWindowHint |
                               Qt::NoDropShadowWindowHint)
    {
        setAttribute(Qt::WA_TranslucentBackground, true);
        setAttribute(Qt::WA_TransparentForMouseEvents, true);
        auto* layout = new QVBoxLayout(this);
        layout->setContentsMargins(9, 9, 9, 9);
        label_ = new QLabel(this);
        label_->setTextFormat(Qt::PlainText);
        label_->setWordWrap(true);
        label_->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
        label_->setStyleSheet(QStringLiteral(R"(
QLabel {
    color: #f7f9fc;
    background: #202633;
    border: 1px solid #343d4e;
    border-radius: 8px;
    padding: 6px 9px;
    font-size: 12px;
}
)"));
        auto* shadow = new QGraphicsDropShadowEffect(label_);
        shadow->setBlurRadius(16);
        shadow->setOffset(0, 4);
        shadow->setColor(QColor(0, 0, 0, 92));
        label_->setGraphicsEffect(shadow);
        layout->addWidget(label_);
        hideTimer_.setSingleShot(true);
        QObject::connect(&hideTimer_, &QTimer::timeout,
                         this, &QWidget::hide);
    }

    void ShowText(const QString& text, const QPoint& globalPosition)
    {
        if (text.trimmed().isEmpty()) {
            hide();
            return;
        }
        label_->setText(text.trimmed());
        const QFontMetrics metrics(label_->font());
        // The fixed QLabel width includes stylesheet padding and borders.
        // Keep extra safety pixels because fractional DPI scaling can make
        // horizontalAdvance() slightly underestimate the painted text width.
        constexpr int kHorizontalChrome = 30;
        constexpr int kMaximumLabelWidth = 280;
        const int textWidth = std::max(
            metrics.horizontalAdvance(text.trimmed()),
            metrics.boundingRect(text.trimmed()).width());
        const bool needsWrapping =
            textWidth + kHorizontalChrome > kMaximumLabelWidth;
        label_->setWordWrap(needsWrapping);
        const int contentWidth = needsWrapping
            ? kMaximumLabelWidth
            : std::clamp(textWidth + kHorizontalChrome,
                         58, kMaximumLabelWidth);
        label_->setFixedWidth(contentWidth);
        label_->setMinimumHeight(0);
        label_->setMaximumHeight(QWIDGETSIZE_MAX);
        label_->adjustSize();
        adjustSize();

        QPoint position = globalPosition + QPoint(12, 18);
        QScreen* screen = QApplication::screenAt(globalPosition);
        if (!screen) {
            screen = QApplication::primaryScreen();
        }
        if (screen) {
            const QRect available = screen->availableGeometry();
            position.setX(std::clamp(
                position.x(), available.left() + 4,
                std::max(available.left() + 4,
                         available.right() - width() - 4)));
            position.setY(std::clamp(
                position.y(), available.top() + 4,
                std::max(available.top() + 4,
                         available.bottom() - height() - 4)));
        }
        move(position);
        show();
        raise();
        hideTimer_.start(4500);
    }

private:
    QLabel* label_ = nullptr;
    QTimer hideTimer_;
};

class RemoteCToolTipController final : public QObject {
public:
    using QObject::QObject;

protected:
    bool eventFilter(QObject* watched, QEvent* event) override
    {
        auto* widget = qobject_cast<QWidget*>(watched);
        if (!widget) {
            return QObject::eventFilter(watched, event);
        }
        if (event->type() == QEvent::ToolTip) {
            auto* helpEvent = static_cast<QHelpEvent*>(event);
            QString text = widget->toolTip();
            if (text.isEmpty()) {
                auto* view = qobject_cast<QAbstractItemView*>(
                    widget->parentWidget());
                if (view && view->viewport() == widget) {
                    const QModelIndex index = view->indexAt(helpEvent->pos());
                    if (index.isValid()) {
                        text = index.data(Qt::ToolTipRole).toString();
                    }
                }
            }
            if (!text.isEmpty()) {
                const QString normalizedText = text.trimmed();
                if (bubble_.isVisible() && source_ == widget &&
                    currentText_ == normalizedText) {
                    return true;
                }
                source_ = widget;
                currentText_ = normalizedText;
                bubble_.ShowText(normalizedText, helpEvent->globalPos());
                return true;
            }
        } else if ((event->type() == QEvent::Leave && source_ == widget) ||
                   event->type() == QEvent::MouseButtonPress ||
                   event->type() == QEvent::Wheel ||
                   event->type() == QEvent::Hide) {
            bubble_.hide();
            if (source_ == widget) {
                source_.clear();
                currentText_.clear();
            }
        }
        return QObject::eventFilter(watched, event);
    }

private:
    RemoteCToolTipBubble bubble_;
    QPointer<QWidget> source_;
    QString currentText_;
};

std::string ToUtf8(const QString& value)
{
    const QByteArray utf8 = value.toUtf8();
    return {utf8.constData(), static_cast<std::size_t>(utf8.size())};
}

struct StartupSignalingConfiguration {
    bool configured = false;
    bool invalid = false;
    bool authenticationRequired = false;
    QString endpoint;
    QString accessToken;
    QString deviceId;
    QString deviceName;
    QString caFile;
    QString sslBackend;
    QString source;
    QString error;
    remote::auth::AuthConfig authConfig;
};

StartupSignalingConfiguration InvalidStartupConfiguration(
    const QString& source,
    const QString& error,
    bool authenticationRequired = false)
{
    StartupSignalingConfiguration configuration;
    configuration.invalid = true;
    configuration.authenticationRequired = authenticationRequired;
    configuration.source = source;
    configuration.error = error;
    return configuration;
}

QString LoadOrCreateLocalDeviceId()
{
    constexpr auto kDeviceIdSetting = "app/deviceId";
    QSettings settings;
    const QString stored = settings.value(
        QString::fromLatin1(kDeviceIdSetting)).toString().trimmed();
    if (!QUuid(stored).isNull()) {
        return QUuid(stored).toString(QUuid::WithoutBraces).toLower();
    }
    const QString generated = QUuid::createUuid()
        .toString(QUuid::WithoutBraces).toLower();
    settings.setValue(QString::fromLatin1(kDeviceIdSetting), generated);
    settings.sync();
    return generated;
}

QString GenerateSessionVerificationCode()
{
    const quint32 value = QRandomGenerator::system()->bounded(1'000'000u);
    return QStringLiteral("%1").arg(value, 6, 10, QChar('0'));
}

StartupSignalingConfiguration LoadStartupSignalingConfiguration()
{
    const bool hasLogtoEnvironmentConfiguration =
        qEnvironmentVariableIsSet("REMOTEC_LOGTO_ISSUER") ||
        qEnvironmentVariableIsSet("REMOTEC_LOGTO_CLIENT_ID") ||
        qEnvironmentVariableIsSet("REMOTEC_LOGTO_CALLBACK_URL");
    if (hasLogtoEnvironmentConfiguration) {
        StartupSignalingConfiguration configuration;
        configuration.authenticationRequired = true;
        configuration.source = QStringLiteral("Logto 环境变量");
        configuration.endpoint =
            qEnvironmentVariable("REMOTEC_SIGNAL_URL").trimmed();
        configuration.deviceId =
            qEnvironmentVariable("REMOTEC_DEVICE_ID").trimmed();
        if (configuration.deviceId.isEmpty()) {
            configuration.deviceId = LoadOrCreateLocalDeviceId();
        }
        configuration.deviceName = qEnvironmentVariable(
            "REMOTEC_DEVICE_NAME", QSysInfo::machineHostName()).trimmed();
        configuration.caFile =
            qEnvironmentVariable("REMOTEC_SIGNAL_CA_FILE").trimmed();
        configuration.sslBackend =
            qEnvironmentVariable("QT_SSL_BACKEND").trimmed();
        configuration.authConfig.issuer = QUrl(
            qEnvironmentVariable("REMOTEC_LOGTO_ISSUER").trimmed());
        configuration.authConfig.clientId = qEnvironmentVariable(
            "REMOTEC_LOGTO_CLIENT_ID").trimmed();
        const QString callbackUrl = qEnvironmentVariable(
            "REMOTEC_LOGTO_CALLBACK_URL").trimmed();
        if (!callbackUrl.isEmpty()) {
            configuration.authConfig.callbackUrl = QUrl(callbackUrl);
        }
        if (qEnvironmentVariableIsSet("REMOTEC_ACCESS_TOKEN")) {
            return InvalidStartupConfiguration(
                configuration.source,
                QStringLiteral(
                    "Logto 模式不得同时提供 REMOTEC_ACCESS_TOKEN。"),
                true);
        }
        QString authError;
        if (configuration.endpoint.isEmpty() ||
            !configuration.endpoint.startsWith(
                QStringLiteral("wss://"), Qt::CaseInsensitive) ||
            configuration.deviceId.isEmpty() ||
            !configuration.authConfig.Validate(&authError)) {
            return InvalidStartupConfiguration(
                configuration.source,
                authError.isEmpty()
                    ? QStringLiteral(
                          "Logto 启动配置缺少有效的 wss 信令地址或设备 ID。")
                    : authError,
                true);
        }
        if (!configuration.caFile.isEmpty() &&
            !QFileInfo::exists(configuration.caFile)) {
            return InvalidStartupConfiguration(
                configuration.source,
                QStringLiteral("环境变量指定的 CA 证书不存在：%1")
                    .arg(configuration.caFile),
                true);
        }
        configuration.configured = true;
        return configuration;
    }

    const bool hasEnvironmentConfiguration =
        qEnvironmentVariableIsSet("REMOTEC_SIGNAL_URL") ||
        qEnvironmentVariableIsSet("REMOTEC_ACCESS_TOKEN") ||
        qEnvironmentVariableIsSet("REMOTEC_DEVICE_ID") ||
        qEnvironmentVariableIsSet("REMOTEC_SIGNAL_CA_FILE");
    if (hasEnvironmentConfiguration) {
        StartupSignalingConfiguration configuration;
        configuration.source = QStringLiteral("环境变量");
        const QString explicitTestOnly =
            qEnvironmentVariable("REMOTEC_LEGACY_AUTH_TEST_ONLY")
                .trimmed();
        if (explicitTestOnly != QStringLiteral("1")) {
            return InvalidStartupConfiguration(
                configuration.source,
                QStringLiteral(
                    "旧版环境变量 Access Token 认证只允许显式测试模式；"
                    "测试时必须设置 REMOTEC_LEGACY_AUTH_TEST_ONLY=1。"));
        }
        configuration.endpoint =
            qEnvironmentVariable("REMOTEC_SIGNAL_URL").trimmed();
        configuration.accessToken =
            qEnvironmentVariable("REMOTEC_ACCESS_TOKEN").trimmed();
        configuration.deviceId =
            qEnvironmentVariable("REMOTEC_DEVICE_ID").trimmed();
        configuration.deviceName = qEnvironmentVariable(
            "REMOTEC_DEVICE_NAME", QSysInfo::machineHostName()).trimmed();
        configuration.caFile =
            qEnvironmentVariable("REMOTEC_SIGNAL_CA_FILE").trimmed();
        configuration.sslBackend =
            qEnvironmentVariable("QT_SSL_BACKEND").trimmed();
        if (configuration.endpoint.isEmpty() ||
            configuration.accessToken.isEmpty() ||
            configuration.deviceId.isEmpty()) {
            return InvalidStartupConfiguration(
                configuration.source,
                QStringLiteral(
                    "环境变量启动配置不完整，必须同时提供信令地址、Access Token 和设备 ID。"));
        }
        if (!configuration.caFile.isEmpty() &&
            !QFileInfo::exists(configuration.caFile)) {
            return InvalidStartupConfiguration(
                configuration.source,
                QStringLiteral("环境变量指定的 CA 证书不存在：%1")
                    .arg(configuration.caFile));
        }
        configuration.configured = true;
        return configuration;
    }

    const QString configurationPath = QDir(
        QCoreApplication::applicationDirPath())
        .filePath(QStringLiteral("RemoteC.bootstrap.json"));
    QFile file(configurationPath);
    if (!file.exists()) {
        return {};
    }
    if (!file.open(QIODevice::ReadOnly)) {
        return InvalidStartupConfiguration(
            configurationPath,
            QStringLiteral("启动配置文件无法读取：%1").arg(file.errorString()));
    }
    if (file.size() > 64 * 1024) {
        return InvalidStartupConfiguration(
            configurationPath,
            QStringLiteral("启动配置文件超过 64 KiB，已拒绝加载。"));
    }

    QJsonParseError parseError;
    const QJsonDocument document =
        QJsonDocument::fromJson(file.readAll(), &parseError);
    if (parseError.error != QJsonParseError::NoError ||
        !document.isObject()) {
        return InvalidStartupConfiguration(
            configurationPath,
            QStringLiteral("启动配置文件格式错误：%1")
                .arg(parseError.errorString()));
    }

    const QJsonObject root = document.object();
    const int schemaVersion =
        root.value(QStringLiteral("schemaVersion")).toInt(-1);
    if (schemaVersion == 2) {
        StartupSignalingConfiguration configuration;
        configuration.authenticationRequired = true;
        configuration.source = configurationPath;
        if (root.contains(QStringLiteral("accessToken"))) {
            return InvalidStartupConfiguration(
                configurationPath,
                QStringLiteral(
                    "Logto 启动配置不得包含 Access Token。"),
                true);
        }
        QString authError;
        const auto authConfig =
            remote::auth::AuthConfig::FromJson(root, &authError);
        if (!authConfig) {
            return InvalidStartupConfiguration(
                configurationPath, authError, true);
        }
        configuration.authConfig = *authConfig;
        configuration.endpoint =
            root.value(QStringLiteral("endpoint")).toString().trimmed();
        configuration.deviceId =
            root.value(QStringLiteral("deviceId")).toString().trimmed();
        if (configuration.deviceId.isEmpty()) {
            configuration.deviceId = LoadOrCreateLocalDeviceId();
        }
        configuration.deviceName =
            root.value(QStringLiteral("deviceName")).toString().trimmed();
        if (configuration.deviceName.isEmpty()) {
            configuration.deviceName = QSysInfo::machineHostName();
        }
        configuration.sslBackend =
            root.value(QStringLiteral("qtSslBackend")).toString().trimmed();
        if (configuration.endpoint.isEmpty() ||
            !configuration.endpoint.startsWith(
                QStringLiteral("wss://"), Qt::CaseInsensitive)) {
            return InvalidStartupConfiguration(
                configurationPath,
                QStringLiteral("Logto 信令地址必须使用 wss://。"),
                true);
        }
        const QString caCertificate = root.value(
            QStringLiteral("caCertificate")).toString().trimmed();
        if (!caCertificate.isEmpty()) {
            configuration.caFile = QDir(
                QFileInfo(configurationPath).absolutePath())
                .absoluteFilePath(caCertificate);
            if (!QFileInfo::exists(configuration.caFile)) {
                return InvalidStartupConfiguration(
                    configurationPath,
                    QStringLiteral("CA 证书不存在：%1")
                        .arg(configuration.caFile),
                    true);
            }
        }
        configuration.configured = true;
        return configuration;
    }
    if (schemaVersion != 1) {
        return InvalidStartupConfiguration(
            configurationPath,
            QStringLiteral("启动配置版本不受支持，请重新生成安装包。"));
    }
    if (root.value(QStringLiteral("testOnly")).toBool(false) != true) {
        return InvalidStartupConfiguration(
            configurationPath,
            QStringLiteral(
                "旧版明文 Access Token 配置只允许显式 testOnly 模式。"));
    }

    StartupSignalingConfiguration configuration;
    configuration.source = configurationPath;
    configuration.endpoint =
        root.value(QStringLiteral("endpoint")).toString().trimmed();
    configuration.accessToken =
        root.value(QStringLiteral("accessToken")).toString().trimmed();
    configuration.deviceId =
        root.value(QStringLiteral("deviceId")).toString().trimmed();
    configuration.deviceName =
        root.value(QStringLiteral("deviceName")).toString().trimmed();
    configuration.sslBackend =
        root.value(QStringLiteral("qtSslBackend")).toString().trimmed();
    if (configuration.deviceName.isEmpty()) {
        configuration.deviceName = QSysInfo::machineHostName();
    }
    if (configuration.endpoint.isEmpty() ||
        configuration.accessToken.isEmpty() ||
        configuration.deviceId.isEmpty()) {
        return InvalidStartupConfiguration(
            configurationPath,
            QStringLiteral(
                "启动配置缺少信令地址、Access Token 或设备 ID。"));
    }
    if (!configuration.endpoint.startsWith(
            QStringLiteral("wss://"), Qt::CaseInsensitive)) {
        return InvalidStartupConfiguration(
            configurationPath,
            QStringLiteral("信令地址必须使用 wss://。"));
    }

    const QString expiresAtText =
        root.value(QStringLiteral("expiresAtUtc")).toString().trimmed();
    if (!expiresAtText.isEmpty()) {
        const QDateTime expiresAt =
            QDateTime::fromString(expiresAtText, Qt::ISODateWithMs);
        if (!expiresAt.isValid()) {
            return InvalidStartupConfiguration(
                configurationPath,
                QStringLiteral("Token 过期时间格式无效，请重新生成测试包。"));
        }
        if (QDateTime::currentDateTimeUtc() >= expiresAt.toUTC()) {
            return InvalidStartupConfiguration(
                configurationPath,
                QStringLiteral("测试 Token 已于 %1 过期，请重新生成测试包。")
                    .arg(expiresAt.toLocalTime().toString(
                        QStringLiteral("yyyy-MM-dd HH:mm:ss"))));
        }
    }

    const QString caCertificate =
        root.value(QStringLiteral("caCertificate")).toString().trimmed();
    if (!caCertificate.isEmpty()) {
        configuration.caFile = QDir(
            QFileInfo(configurationPath).absolutePath())
            .absoluteFilePath(caCertificate);
        if (!QFileInfo::exists(configuration.caFile)) {
            return InvalidStartupConfiguration(
                configurationPath,
                QStringLiteral("CA 证书不存在：%1")
                    .arg(configuration.caFile));
        }
    }
    configuration.configured = true;
    return configuration;
}

QString SingleInstanceServerName()
{
    const QByteArray userScope = QCryptographicHash::hash(
        QStandardPaths::writableLocation(
            QStandardPaths::AppConfigLocation).toUtf8(),
        QCryptographicHash::Sha256).toHex().left(16);
    return QStringLiteral("RemoteCApp-%1")
        .arg(QString::fromLatin1(userScope));
}

constexpr wchar_t kSingleInstanceMutexName[] =
    L"Local\\RemoteCApp.SingleInstance.v1";

class ScopedWinHandle final {
public:
    explicit ScopedWinHandle(HANDLE handle = nullptr) : handle_(handle) {}
    ~ScopedWinHandle()
    {
        if (handle_) {
            CloseHandle(handle_);
        }
    }

    ScopedWinHandle(const ScopedWinHandle&) = delete;
    ScopedWinHandle& operator=(const ScopedWinHandle&) = delete;
    ScopedWinHandle(ScopedWinHandle&& other) noexcept
        : handle_(other.handle_)
    {
        other.handle_ = nullptr;
    }
    ScopedWinHandle& operator=(ScopedWinHandle&& other) noexcept
    {
        if (this != &other) {
            if (handle_) {
                CloseHandle(handle_);
            }
            handle_ = other.handle_;
            other.handle_ = nullptr;
        }
        return *this;
    }

    HANDLE get() const { return handle_; }

private:
    HANDLE handle_ = nullptr;
};

bool IsUtilityInvocation(const QStringList& arguments)
{
    for (const QString& argument : arguments) {
        if (argument == QStringLiteral("--status-once") ||
            argument == QStringLiteral("--startup-config-self-test") ||
            argument == QStringLiteral("--auth-coordinator-self-test") ||
            argument == QStringLiteral("--theme-roundtrip-self-test") ||
            argument == QStringLiteral("--signaling-policy-self-test") ||
            argument == QStringLiteral("--decoder-optimal-probe") ||
            argument.startsWith(
                QStringLiteral("--encoder-optimal-probe=")) ||
            argument == QStringLiteral(
                "--ffmpeg-x264-encoder-self-test") ||
            argument.startsWith(
                QStringLiteral("--desktop-capture-self-test"))) {
            return true;
        }
    }
    return false;
}

bool NotifyRunningInstance(const QString& serverName)
{
    QLocalSocket socket;
    socket.connectToServer(serverName, QIODevice::WriteOnly);
    if (!socket.waitForConnected(700)) {
        return false;
    }
    if (socket.write("activate\n") < 0) {
        return false;
    }
    socket.flush();
    return socket.bytesToWrite() == 0 ||
           socket.waitForBytesWritten(700);
}

bool WriteCurrentUserRegistryString(
    const wchar_t* subkey,
    const wchar_t* valueName,
    const QString& value)
{
    HKEY key = nullptr;
    if (RegCreateKeyExW(HKEY_CURRENT_USER, subkey, 0, nullptr,
                        REG_OPTION_NON_VOLATILE, KEY_SET_VALUE,
                        nullptr, &key, nullptr) != ERROR_SUCCESS) {
        return false;
    }
    const std::wstring encoded = value.toStdWString();
    const auto* bytes = reinterpret_cast<const BYTE*>(encoded.c_str());
    const DWORD byteCount = static_cast<DWORD>(
        (encoded.size() + 1) * sizeof(wchar_t));
    const LONG status = RegSetValueExW(
        key, valueName, 0, REG_SZ, bytes, byteCount);
    RegCloseKey(key);
    return status == ERROR_SUCCESS;
}

bool EnsureRemoteCUrlProtocolRegistration()
{
    const QString executable = QDir::toNativeSeparators(
        QCoreApplication::applicationFilePath());
    const QString command = QStringLiteral("\"") + executable +
        QStringLiteral("\" --activate-from-browser \"%1\"");
    return WriteCurrentUserRegistryString(
               L"Software\\Classes\\remotec", nullptr,
               QStringLiteral("URL:RemoteC Protocol")) &&
           WriteCurrentUserRegistryString(
               L"Software\\Classes\\remotec", L"URL Protocol", {}) &&
           WriteCurrentUserRegistryString(
               L"Software\\Classes\\remotec\\DefaultIcon", nullptr,
               QStringLiteral("\"%1\",0").arg(executable)) &&
           WriteCurrentUserRegistryString(
               L"Software\\Classes\\remotec\\shell\\open\\command",
               nullptr, command);
}

remote::VideoEncoderPreference ConfiguredVideoEncoderPreference()
{
    const QString configured = QSettings().value(
        QStringLiteral("media/videoEncoderPreference"),
        QStringLiteral("auto")).toString();
    if (configured == QStringLiteral("hardware")) {
        return remote::VideoEncoderPreference::kHardwareOnly;
    }
    if (configured == QStringLiteral("software")) {
        return remote::VideoEncoderPreference::kSoftwareOnly;
    }
    if (configured == QStringLiteral("ffmpeg_hardware")) {
        return remote::VideoEncoderPreference::kFfmpegHardware;
    }
    if (configured == QStringLiteral("ffmpeg")) {
        return remote::VideoEncoderPreference::kFfmpegX264Only;
    }
    return remote::VideoEncoderPreference::kAutomatic;
}

remote::FfmpegHardwareBackend ConfiguredFfmpegHardwareBackend()
{
    const QString configured = QSettings().value(
        QStringLiteral("media/ffmpegHardwareBackend"),
        QStringLiteral("auto")).toString().toLower();
    if (configured == QStringLiteral("qsv")) {
        return remote::FfmpegHardwareBackend::kQsv;
    }
    if (configured == QStringLiteral("nvenc")) {
        return remote::FfmpegHardwareBackend::kNvenc;
    }
    if (configured == QStringLiteral("amf")) {
        return remote::FfmpegHardwareBackend::kAmf;
    }
    return remote::FfmpegHardwareBackend::kAutomatic;
}

remote::FfmpegX264Preset ConfiguredFfmpegX264Preset()
{
    QSettings settings;
    QString configured = settings.value(
        QStringLiteral("media/ffmpegX264Preset"),
        QStringLiteral("medium")).toString().toLower();
    // Migrate the former nine-level FFmpeg-only preset onto the nearest one
    // of the five shared encoder-quality levels. This keeps the value shown
    // in Settings identical to the value used by every encoder at startup.
    QString canonical = configured;
    if (canonical == QStringLiteral("superfast")) {
        canonical = QStringLiteral("ultrafast");
    } else if (canonical == QStringLiteral("faster") ||
               canonical == QStringLiteral("fast")) {
        canonical = QStringLiteral("veryfast");
    } else if (canonical == QStringLiteral("slower")) {
        canonical = QStringLiteral("slow");
    }
    if (canonical != configured) {
        settings.setValue(
            QStringLiteral("media/ffmpegX264Preset"), canonical);
        settings.sync();
        configured = canonical;
    }
    if (configured == QStringLiteral("ultrafast")) {
        return remote::FfmpegX264Preset::kUltraFast;
    }
    if (configured == QStringLiteral("superfast")) {
        return remote::FfmpegX264Preset::kSuperFast;
    }
    if (configured == QStringLiteral("veryfast")) {
        return remote::FfmpegX264Preset::kVeryFast;
    }
    if (configured == QStringLiteral("faster")) {
        return remote::FfmpegX264Preset::kFaster;
    }
    if (configured == QStringLiteral("fast")) {
        return remote::FfmpegX264Preset::kFast;
    }
    if (configured == QStringLiteral("slow")) {
        return remote::FfmpegX264Preset::kSlow;
    }
    if (configured == QStringLiteral("slower")) {
        return remote::FfmpegX264Preset::kSlower;
    }
    if (configured == QStringLiteral("veryslow")) {
        return remote::FfmpegX264Preset::kVerySlow;
    }
    return remote::FfmpegX264Preset::kMedium;
}

remote::VideoDecoderPreference ConfiguredVideoDecoderPreference()
{
    QSettings settings;
    const QString configured = settings.value(
        QStringLiteral("media/videoDecoderPreference"),
        QStringLiteral("auto")).toString();
    if (configured == QStringLiteral("hardware")) {
        const QString currentFingerprint = QString::fromStdString(
            remote::BuildWindowsHardwareFingerprint());
        const bool hardwareDecoderVerified =
            settings.value(
                QStringLiteral("media/decoderProbe/completed"),
                false).toBool() &&
            settings.value(
                QStringLiteral("media/decoderProbe/passed"),
                false).toBool() &&
            settings.value(
                QStringLiteral("media/decoderProbe/policyVersion"),
                0).toInt() ==
                remote::kVideoDecoderProbePolicyVersion &&
            settings.value(
                QStringLiteral(
                    "media/decoderProbe/hardwareFingerprint"))
                    .toString() == currentFingerprint;
        if (!hardwareDecoderVerified) {
            // A stale hardware-only preference must not make the whole
            // WebRTC runtime unavailable after a GPU/driver/RDP change.
            // Automatic mode keeps the software H264 decoder wired.
            settings.setValue(
                QStringLiteral("media/videoDecoderPreference"),
                QStringLiteral("auto"));
            settings.sync();
            return remote::VideoDecoderPreference::kAutomatic;
        }
        return remote::VideoDecoderPreference::kHardwareOnly;
    }
    if (configured == QStringLiteral("software")) {
        return remote::VideoDecoderPreference::kSoftwareOnly;
    }
    return remote::VideoDecoderPreference::kAutomatic;
}

remote::DesktopCaptureImplementation ConfiguredDesktopCaptureImplementation()
{
    const QString configured = QSettings().value(
        QStringLiteral("media/desktopCaptureBackend"),
        QStringLiteral("native_dxgi")).toString();
    if (configured == QStringLiteral("libwebrtc")) {
        return remote::DesktopCaptureImplementation::kLibWebRtc;
    }
    return remote::DesktopCaptureImplementation::kNativeDxgi;
}

std::unique_ptr<remote::app::InProcessSessionEngine> CreateSessionEngine(
    const StartupSignalingConfiguration& startupConfiguration,
    const QString& deviceVerificationCode)
{
    remote::app::InProcessSessionEngineOptions engineOptions;
    const QSettings mediaSettings;
    const auto hardwareProfile =
        remote::QueryWindowsCompatibilityProfile();
    const std::string& hardwareFingerprint =
        hardwareProfile.hardwareFingerprint;
    engineOptions.hardwareFingerprint = hardwareFingerprint;
    engineOptions.operatingSystemDescription =
        hardwareProfile.operatingSystem;
    engineOptions.nativeArchitecture =
        hardwareProfile.nativeArchitecture;
    engineOptions.remoteSession =
        hardwareProfile.remoteSession;
    engineOptions.graphicsAdapterDescriptions =
        hardwareProfile.graphicsAdapters;
    engineOptions.graphicsEnumerationError =
        hardwareProfile.graphicsEnumerationError;
    engineOptions.desktopCaptureImplementation =
        ConfiguredDesktopCaptureImplementation();
    engineOptions.videoEncoderPreference =
        ConfiguredVideoEncoderPreference();
    engineOptions.ffmpegX264Preset = ConfiguredFfmpegX264Preset();
    engineOptions.ffmpegHardwareBackend =
        ConfiguredFfmpegHardwareBackend();
    engineOptions.videoDecoderPreference =
        ConfiguredVideoDecoderPreference();
    const QString currentFingerprint =
        QString::fromStdString(hardwareFingerprint);
    const QString configuredCaptureBackend = mediaSettings.value(
        QStringLiteral("media/desktopCaptureBackend"),
        QStringLiteral("native_dxgi")).toString();
    const QJsonObject encoderProfile =
        remote::app::LoadEncoderBenchmarkProfile(
            mediaSettings, currentFingerprint, configuredCaptureBackend,
            mediaSettings.value(
                QStringLiteral("media/ffmpegX264Preset"),
                QStringLiteral("medium")).toString(),
            remote::kVideoEncoderProbePolicyVersion);
    if (encoderProfile.value(QStringLiteral("passed")).toBool()) {
        engineOptions.preferredAutomaticEncoderId = ToUtf8(
            encoderProfile.value(QStringLiteral("bestEncoderId")).toString());
    } else {
        // Before the first benchmark, automatic mode deliberately starts on
        // FFmpeg hardware. If no supported vendor backend exists, the normal
        // FFmpeg/libx264 software fallback is used instead of trying MFT.
        engineOptions.preferredAutomaticEncoderId =
            remote::kAutomaticEncoderFfmpegHardwareDefault;
    }
    if (mediaSettings.value(
            QStringLiteral(
                "media/decoderProbe/hardwareFingerprint"))
            .toString() == currentFingerprint &&
        mediaSettings.value(
            QStringLiteral("media/decoderProbe/completed"),
            false).toBool() &&
        mediaSettings.value(
            QStringLiteral("media/decoderProbe/passed"),
            false).toBool() &&
        mediaSettings.value(
            QStringLiteral("media/decoderProbe/policyVersion"),
            0).toInt() ==
            remote::kVideoDecoderProbePolicyVersion) {
        engineOptions.preferredHardwareDecoderName = ToUtf8(
            mediaSettings.value(
                QStringLiteral(
                    "media/decoderProbe/bestDecoderName"))
                .toString());
    } else {
        // Before the first benchmark, automatic decoding starts on FFmpeg
        // D3D11VA and retains the normal software decoder as its fallback.
        engineOptions.preferredHardwareDecoderName =
            remote::kFfmpegD3D11H264DecoderName;
    }
    engineOptions.preferredCameraDeviceId = ToUtf8(
        mediaSettings.value(
            QStringLiteral("media/cameraDeviceId"),
            QStringLiteral("default")).toString());
    engineOptions.preferredMicrophoneDeviceId = ToUtf8(
        mediaSettings.value(
            QStringLiteral("media/microphoneDeviceId"),
            QStringLiteral("default")).toString());
    engineOptions.preferredSpeakerDeviceId = ToUtf8(
        mediaSettings.value(
            QStringLiteral("media/speakerDeviceId"),
            QStringLiteral("default")).toString());

    const QString encoderCacheFingerprint =
        mediaSettings.value(
            QStringLiteral(
                "media/hardwareProbe/fingerprint"))
            .toString();
    if (mediaSettings.value(
            QStringLiteral("media/hardwareProbe/valid"),
            false).toBool() &&
        encoderCacheFingerprint == currentFingerprint) {
        remote::MfH264EncoderCapabilityCache cache;
        cache.valid = true;
        cache.hardwareFingerprint = hardwareFingerprint;
        cache.hardwareEncoderAvailable = mediaSettings.value(
            QStringLiteral(
                "media/hardwareProbe/encoderAvailable"),
            false).toBool();
        cache.cpuNv12InputSupported = mediaSettings.value(
            QStringLiteral(
                "media/hardwareProbe/cpuNv12InputSupported"),
            false).toBool();
        cache.d3d11InputCandidate = mediaSettings.value(
            QStringLiteral(
                "media/hardwareProbe/d3d11InputCandidate"),
            false).toBool();
        cache.hardwareEncoderCount = mediaSettings.value(
            QStringLiteral(
                "media/hardwareProbe/encoderCount"),
            0).toUInt();
        for (const QString& description : mediaSettings.value(
                 QStringLiteral(
                     "media/hardwareProbe/descriptions"))
                 .toStringList()) {
            cache.descriptions.push_back(
                ToUtf8(description));
        }
        for (const QString& warning : mediaSettings.value(
                 QStringLiteral(
                     "media/hardwareProbe/warnings"))
                 .toStringList()) {
            cache.warnings.push_back(ToUtf8(warning));
        }
        engineOptions.encoderCapabilityCache =
            std::move(cache);
    }

    if (!startupConfiguration.configured ||
        startupConfiguration.invalid) {
        return std::make_unique<remote::app::InProcessSessionEngine>(
            nullptr, remote::SignalingClientConfig{},
            engineOptions);
    }

    remote::SignalingClientConfig config;
    config.endpoint = ToUtf8(startupConfiguration.endpoint);
    config.accessToken = ToUtf8(startupConfiguration.accessToken);
    config.authenticationMode = startupConfiguration.authenticationRequired
        ? remote::SignalingAuthenticationMode::kMessageAccessToken
        : remote::SignalingAuthenticationMode::kLegacyUpgradeBearer;
    config.deviceId = ToUtf8(startupConfiguration.deviceId);
    config.deviceName = ToUtf8(startupConfiguration.deviceName);
    config.deviceVerificationCode = ToUtf8(deviceVerificationCode);
    config.appVersion = QCoreApplication::applicationVersion().isEmpty()
        ? std::string("development")
        : ToUtf8(QCoreApplication::applicationVersion());
    const QString caFile = startupConfiguration.caFile;
    if (!caFile.isEmpty()) {
        QFile file(caFile);
        if (file.open(QIODevice::ReadOnly)) {
            config.trustedCaPem = ToUtf8(QString::fromLatin1(file.readAll()));
        } else {
            // Force an explicit configuration error instead of silently
            // falling back to the system trust store.
            config.trustedCaPem = "invalid-ca-file";
        }
    }
    return std::make_unique<remote::app::InProcessSessionEngine>(
        std::make_unique<remote::QtWebSocketSignalingClient>(),
        std::move(config), engineOptions);
}

int RunSignalingPolicySelfTest()
{
    remote::QtWebSocketSignalingClient client;
    remote::SignalingClientConfig config;
    config.endpoint = "ws://127.0.0.1:65535/signaling";
    config.accessToken = "test-token";
    config.deviceName = "policy-self-test";
    const auto insecureEndpoint = client.Connect(config);

    config.endpoint = "wss://example.invalid/signaling";
    config.accessToken.clear();
    const auto missingToken = client.Connect(config);
    const auto emptyTokenUpdate = client.UpdateAccessToken({});
    const auto validTokenUpdate = client.UpdateAccessToken(
        "updated-memory-only-token");

    const bool passed =
        !insecureEndpoint.accepted &&
        insecureEndpoint.errorCode == "wss_endpoint_required" &&
        !missingToken.accepted &&
        missingToken.errorCode == "access_token_required" &&
        !emptyTokenUpdate.accepted && validTokenUpdate.accepted;
    QTextStream output(stdout);
    output << "WSS_ONLY_POLICY="
           << (!insecureEndpoint.accepted ? "PASS" : "FAIL") << Qt::endl;
    output << "ACCESS_TOKEN_POLICY="
           << (!missingToken.accepted ? "PASS" : "FAIL") << Qt::endl;
    output << "ACCESS_TOKEN_UPDATE_POLICY="
           << (!emptyTokenUpdate.accepted && validTokenUpdate.accepted
                   ? "PASS"
                   : "FAIL")
           << Qt::endl;
    output << "SIGNALING_POLICY_SELF_TEST="
           << (passed ? "PASS" : "FAIL") << Qt::endl;
    return passed ? 0 : 1;
}

int RunAuthCoordinatorSelfTest()
{
    QTemporaryDir temporaryDirectory;
    int mainWindowFactoryCalls = 0;
    remote::app::RemoteCApplicationCoordinator::Options options;
    options.authenticationRequired = true;
    options.configurationError = QStringLiteral("expected self-test error");
    options.configurationSource = QStringLiteral("self-test");
    remote::app::RemoteCApplicationCoordinator coordinator(
        std::move(options),
        [&mainWindowFactoryCalls](const QString&) {
            ++mainWindowFactoryCalls;
            remote::app::RemoteCApplicationCoordinator::MainWindowSession
                session;
            session.window = std::make_unique<
                remote::controller::ControllerMainWindow>(
                    std::make_unique<
                        remote::app::InProcessSessionEngine>(),
                    false);
            return session;
        },
        std::make_unique<remote::auth::DpapiTokenStore>(
            temporaryDirectory.filePath(
                QStringLiteral("credentials.dat"))));
    const bool started = temporaryDirectory.isValid() && coordinator.Start();
    QEventLoop startupLoop;
    QTimer::singleShot(150, &startupLoop, &QEventLoop::quit);
    startupLoop.exec();
    bool loginWindowCreated = false;
    QWidget* loginWindow = nullptr;
    for (QWidget* widget : QApplication::allWidgets()) {
        if (widget && widget->objectName() ==
                          QStringLiteral("loginWindow")) {
            loginWindowCreated = true;
            loginWindow = widget;
            break;
        }
    }
    const bool previousQuitOnLastWindowClosed =
        QApplication::quitOnLastWindowClosed();
    QApplication::setQuitOnLastWindowClosed(true);
    if (loginWindow) {
        loginWindow->show();
        loginWindow->close();
        QCoreApplication::processEvents();
    }
    const bool loginWindowAcceptsApplicationExit =
        loginWindow && !loginWindow->isVisible();
    QApplication::setQuitOnLastWindowClosed(
        previousQuitOnLastWindowClosed);
    const bool mainWindowCreatedAfterStartupYield =
        mainWindowFactoryCalls == 1;
    const bool passed = started && loginWindowCreated &&
        mainWindowCreatedAfterStartupYield && loginWindowAcceptsApplicationExit;
    QTextStream output(stdout);
    output << "AUTH_LOGIN_WINDOW="
           << (loginWindowCreated ? "PASS" : "FAIL") << Qt::endl;
    output << "AUTH_MAIN_WINDOW_CREATED_AFTER_STARTUP_YIELD="
           << (mainWindowCreatedAfterStartupYield ? "PASS" : "FAIL")
           << Qt::endl;
    output << "AUTH_LOGIN_WINDOW_ACCEPTS_APP_EXIT="
           << (loginWindowAcceptsApplicationExit ? "PASS" : "FAIL")
           << Qt::endl;
    output << "AUTH_COORDINATOR_SELF_TEST="
           << (passed ? "PASS" : "FAIL") << Qt::endl;
    return passed ? 0 : 1;
}

}  // namespace

int main(int argc, char* argv[])
{
    QApplication application(argc, argv);
    application.setApplicationName(QStringLiteral("RemoteCApp"));
    application.setApplicationDisplayName(QStringLiteral("RLink"));
    application.setOrganizationName(QStringLiteral("RemoteC"));
    application.setApplicationVersion(InstalledApplicationVersion());
    QSettings::setDefaultFormat(QSettings::IniFormat);
    QSettings legacySettings(
        QSettings::NativeFormat, QSettings::UserScope,
        QStringLiteral("RemoteC"), QStringLiteral("RemoteCApp"));
    QSettings settings;
    const QStringList migratedKeys = {
        QStringLiteral("app/deviceId"),
        QStringLiteral("ui/animationsEnabled"),
        QStringLiteral("ui/animationLevel"),
        QStringLiteral("ui/systemFontFamily"),
        QStringLiteral("ui/fontPixelSize"),
        QStringLiteral("ui/recentRooms"),
        QStringLiteral("ui/recentDevices"),
        QStringLiteral("files/defaultSaveDirectory"),
        QStringLiteral("remoteSession/remotePasteEnabled"),
        QStringLiteral("remoteSession/clipboardFormats"),
        QStringLiteral("remoteSession/clipboardFileLimitMiB"),
        QStringLiteral("media/videoEncoderPreference"),
        QStringLiteral("media/videoDecoderPreference"),
        QStringLiteral("media/videoRendererPreference"),
        QStringLiteral("media/cameraDeviceId"),
        QStringLiteral("media/microphoneDeviceId"),
        QStringLiteral("media/speakerDeviceId")};
    const QString recentMigrationMarker = QStringLiteral(
        "migration/nativeRecentHistoryCompleted");
    const bool nativeRecentHistoryCompleted =
        settings.value(recentMigrationMarker, false).toBool();
    for (const QString& key : migratedKeys) {
        const bool recentHistoryKey =
            key == QStringLiteral("ui/recentRooms") ||
            key == QStringLiteral("ui/recentDevices");
        if (recentHistoryKey && nativeRecentHistoryCompleted) {
            continue;
        }
        if (!settings.contains(key) && legacySettings.contains(key)) {
            settings.setValue(key, legacySettings.value(key));
        }
    }
    if (!nativeRecentHistoryCompleted) {
        settings.setValue(recentMigrationMarker, true);
    }
    if (!settings.contains(QStringLiteral("ui/animationLevel"))) {
        settings.setValue(
            QStringLiteral("ui/animationLevel"),
            settings.value(
                QStringLiteral("ui/animationsEnabled"), true).toBool()
                ? 2 : 0);
    }
    settings.sync();
    const QString latinFontFamily = RegisterBundledFont(
        QStringLiteral(":/ui/fonts/Inter-Variable.ttf"),
        QStringLiteral("Segoe UI"));
    application.setProperty(
        "remoteCDefaultLatinFontFamily", latinFontFamily);
    QString configuredSystemFontFamily = settings.value(
        QStringLiteral("ui/systemFontFamily")).toString().trimmed();
    if (!configuredSystemFontFamily.isEmpty() &&
        !QFontDatabase::families().contains(
            configuredSystemFontFamily, Qt::CaseInsensitive)) {
        configuredSystemFontFamily.clear();
        settings.remove(QStringLiteral("ui/systemFontFamily"));
    }
    QFont interfaceFont;
    if (configuredSystemFontFamily.isEmpty()) {
        interfaceFont.setFamilies({
            QStringLiteral("Microsoft YaHei UI"),
            QStringLiteral("Segoe UI")});
    } else {
        interfaceFont.setFamilies({
            configuredSystemFontFamily,
            QStringLiteral("Microsoft YaHei UI"),
            QStringLiteral("Segoe UI")});
    }
    interfaceFont.setHintingPreference(QFont::PreferDefaultHinting);
    interfaceFont.setStyleStrategy(QFont::PreferAntialias);
    interfaceFont.setPixelSize(std::clamp(
        settings.value(QStringLiteral("ui/fontPixelSize"), 13).toInt(),
        12, 17));
    application.setFont(interfaceFont);
    application.setQuitOnLastWindowClosed(false);
    RemoteCToolTipController toolTipController;
    application.installEventFilter(&toolTipController);

    QLocalServer singleInstanceServer;
    ScopedWinHandle singleInstanceMutex;
    if (!IsUtilityInvocation(application.arguments())) {
        const QString serverName = SingleInstanceServerName();
        SetLastError(ERROR_SUCCESS);
        ScopedWinHandle candidateMutex(
            CreateMutexW(nullptr, FALSE, kSingleInstanceMutexName));
        if (!candidateMutex.get()) {
            remote::controller::RemoteCDialog::Alert(
                nullptr, QStringLiteral("RLink 启动失败"),
                QStringLiteral("无法建立 Windows 单实例锁，请重新启动 RLink。"),
                QStringLiteral("知道了"),
                remote::controller::RemoteCDialog::Tone::kDanger);
            return 2;
        }

        if (GetLastError() == ERROR_ALREADY_EXISTS) {
            // The primary process may still be creating its local IPC server.
            // Give it a short time to become ready, but never allow this
            // secondary process to continue into normal application startup.
            for (int attempt = 0; attempt < 30; ++attempt) {
                if (NotifyRunningInstance(serverName)) {
                    return 0;
                }
                Sleep(50);
            }
            return 0;
        }

        singleInstanceMutex = std::move(candidateMutex);
        QLocalServer::removeServer(serverName);
        if (!singleInstanceServer.listen(serverName)) {
            remote::controller::RemoteCDialog::Alert(
                nullptr, QStringLiteral("RLink 启动失败"),
                QStringLiteral(
                    "无法建立单实例通信通道，请结束已有 RLink 进程后重试。"),
                QStringLiteral("知道了"),
                remote::controller::RemoteCDialog::Tone::kDanger);
            return 2;
        }
        // Register only after this executable becomes the primary instance.
        // A test copy or protocol-launched secondary must not replace the
        // canonical command owned by the running RemoteC process.
        (void)EnsureRemoteCUrlProtocolRegistration();
    }

    if (application.arguments().contains(
            QStringLiteral("--signaling-policy-self-test"))) {
        return RunSignalingPolicySelfTest();
    }

    if (application.arguments().contains(
            QStringLiteral("--startup-config-self-test"))) {
        const StartupSignalingConfiguration configuration =
            LoadStartupSignalingConfiguration();
        QTextStream output(stdout);
        output << "STARTUP_CONFIG_CONFIGURED="
               << (configuration.configured ? "YES" : "NO") << Qt::endl;
        output << "STARTUP_CONFIG_VALID="
               << (!configuration.invalid ? "YES" : "NO") << Qt::endl;
        output << "STARTUP_CONFIG_AUTH_REQUIRED="
               << (configuration.authenticationRequired ? "YES" : "NO")
               << Qt::endl;
        output << "STARTUP_CONFIG_DEVICE_ID="
               << configuration.deviceId << Qt::endl;
        output << "STARTUP_CONFIG_ENDPOINT="
               << configuration.endpoint << Qt::endl;
        if (!configuration.error.isEmpty()) {
            output << "STARTUP_CONFIG_ERROR="
                   << configuration.error << Qt::endl;
        }
        return configuration.configured && !configuration.invalid ? 0 : 1;
    }

    if (application.arguments().contains(
            QStringLiteral("--auth-coordinator-self-test"))) {
        return RunAuthCoordinatorSelfTest();
    }

    if (application.arguments().contains(
            QStringLiteral("--decoder-optimal-probe"))) {
        SetPriorityClass(GetCurrentProcess(), BELOW_NORMAL_PRIORITY_CLASS);
        const auto result =
            remote::RunMfD3D11H264DecoderBenchmark();
        QJsonObject root;
        root.insert(QStringLiteral("passed"), result.passed);
        root.insert(QStringLiteral("bestDecoderName"),
                    QString::fromStdString(result.bestDecoderName));
        root.insert(QStringLiteral("bestAverageLatencyMs"),
                    result.bestAverageLatencyMs);
        root.insert(QStringLiteral("bestP95LatencyMs"),
                    result.bestP95LatencyMs);
        root.insert(QStringLiteral("error"),
                    QString::fromStdString(result.error));
        QJsonArray candidates;
        for (const auto& candidate : result.candidates) {
            QJsonObject item;
            item.insert(QStringLiteral("name"),
                        QString::fromStdString(candidate.name));
            item.insert(QStringLiteral("hardware"), candidate.hardware);
            item.insert(QStringLiteral("passed"), candidate.passed);
            item.insert(QStringLiteral("asynchronous"),
                        candidate.asynchronous);
            item.insert(QStringLiteral("nativeD3D11Output"),
                        candidate.nativeD3D11Output);
            item.insert(QStringLiteral("decodedFrames"),
                        static_cast<int>(candidate.decodedFrames));
            item.insert(QStringLiteral("averageLatencyMs"),
                        candidate.averageLatencyMs);
            item.insert(QStringLiteral("p95LatencyMs"),
                        candidate.p95LatencyMs);
            item.insert(QStringLiteral("realtimeDecodedFrames"),
                        static_cast<int>(candidate.realtimeDecodedFrames));
            item.insert(QStringLiteral("realtimeAverageLatencyMs"),
                        candidate.realtimeAverageLatencyMs);
            item.insert(QStringLiteral("realtimeP95LatencyMs"),
                        candidate.realtimeP95LatencyMs);
            item.insert(QStringLiteral("sparseDecodedFrames"),
                        static_cast<int>(candidate.sparseDecodedFrames));
            item.insert(QStringLiteral("sparseAverageLatencyMs"),
                        candidate.sparseAverageLatencyMs);
            item.insert(QStringLiteral("sparseP95LatencyMs"),
                        candidate.sparseP95LatencyMs);
            item.insert(QStringLiteral("error"),
                        QString::fromStdString(candidate.error));
            candidates.push_back(item);
        }
        root.insert(QStringLiteral("candidates"), candidates);
        QTextStream(stdout)
            << QString::fromUtf8(
                   QJsonDocument(root).toJson(QJsonDocument::Compact))
            << Qt::endl;
        return result.passed ? 0 : 1;
    }

    QString encoderProbeInput;
    for (const QString& argument : application.arguments()) {
        constexpr auto kPrefix = "--encoder-optimal-probe=";
        if (argument.startsWith(QString::fromLatin1(kPrefix))) {
            encoderProbeInput = argument.mid(
                static_cast<int>(std::strlen(kPrefix))).toLower();
            break;
        }
    }
    if (!encoderProbeInput.isEmpty()) {
        SetPriorityClass(GetCurrentProcess(), BELOW_NORMAL_PRIORITY_CLASS);
        const auto input = encoderProbeInput == QStringLiteral("native_dxgi")
            ? remote::H264EncoderBenchmarkInput::kD3D11Bgra
            : remote::H264EncoderBenchmarkInput::kCpuBgra;
        const auto result = remote::RunH264EncoderBenchmark(
            input, ConfiguredFfmpegX264Preset());
        QJsonObject root;
        root.insert(QStringLiteral("passed"), result.passed);
        root.insert(QStringLiteral("bestEncoderId"),
                    QString::fromStdString(result.bestEncoderId));
        root.insert(QStringLiteral("bestEncoderName"),
                    QString::fromStdString(result.bestEncoderName));
        root.insert(QStringLiteral("captureBackend"),
                    input == remote::H264EncoderBenchmarkInput::kD3D11Bgra
                        ? QStringLiteral("native_dxgi")
                        : QStringLiteral("libwebrtc"));
        root.insert(QStringLiteral("error"),
                    QString::fromStdString(result.error));
        QJsonArray candidates;
        for (const auto& candidate : result.candidates) {
            QJsonObject item;
            item.insert(QStringLiteral("id"),
                        QString::fromStdString(candidate.id));
            item.insert(QStringLiteral("name"),
                        QString::fromStdString(candidate.name));
            item.insert(QStringLiteral("inputPath"),
                        QString::fromStdString(candidate.inputPath));
            item.insert(QStringLiteral("hardware"), candidate.hardware);
            item.insert(QStringLiteral("passed"), candidate.passed);
            item.insert(QStringLiteral("submittedFrames"),
                        static_cast<int>(candidate.submittedFrames));
            item.insert(QStringLiteral("encodedFrames"),
                        static_cast<int>(candidate.encodedFrames));
            item.insert(QStringLiteral("keyFrames"),
                        static_cast<int>(candidate.keyFrames));
            item.insert(QStringLiteral("encodedBytes"),
                        static_cast<double>(candidate.encodedBytes));
            item.insert(QStringLiteral("averageLatencyMs"),
                        candidate.averageLatencyMs);
            item.insert(QStringLiteral("p95LatencyMs"),
                        candidate.p95LatencyMs);
            item.insert(QStringLiteral("cpuTimePerFrameMs"),
                        candidate.cpuTimePerFrameMs);
            item.insert(QStringLiteral("inputFramesPerSecond"),
                        candidate.inputFramesPerSecond);
            item.insert(QStringLiteral("averageLumaPsnrDb"),
                        candidate.averageLumaPsnrDb);
            item.insert(QStringLiteral("score"), candidate.score);
            item.insert(QStringLiteral("dynamicRateControlTested"),
                        candidate.dynamicRateControlTested);
            item.insert(QStringLiteral("dynamicRateControlPassed"),
                        candidate.dynamicRateControlPassed);
            item.insert(QStringLiteral("warning"),
                        QString::fromStdString(candidate.warning));
            item.insert(QStringLiteral("error"),
                        QString::fromStdString(candidate.error));
            candidates.push_back(item);
        }
        root.insert(QStringLiteral("candidates"), candidates);
        QTextStream(stdout)
            << QString::fromUtf8(
                   QJsonDocument(root).toJson(QJsonDocument::Compact))
            << Qt::endl;
        return result.passed ? 0 : 1;
    }

    if (application.arguments().contains(
            QStringLiteral("--ffmpeg-x264-encoder-self-test"))) {
        const auto result =
            remote::RunFfmpegX264EncoderSelfTest();
        QTextStream output(stdout);
        output << "FFMPEG_X264_ENCODER_SELF_TEST="
               << (result.passed ? "PASS" : "FAIL") << Qt::endl;
        output << "FFMPEG_X264_ENCODED_FRAMES="
               << result.encodedFrames << Qt::endl;
        output << "FFMPEG_X264_ENCODED_BYTES="
               << result.encodedBytes << Qt::endl;
        if (!result.error.empty()) {
            output << "FFMPEG_X264_ERROR="
                   << QString::fromStdString(result.error) << Qt::endl;
        }
        return result.passed ? 0 : 1;
    }

    QString ffmpegHardwareSelfTestBackend;
    for (const QString& argument : application.arguments()) {
        constexpr auto kPrefix = "--ffmpeg-hardware-encoder-self-test=";
        if (argument.startsWith(QString::fromLatin1(kPrefix))) {
            ffmpegHardwareSelfTestBackend =
                argument.mid(static_cast<int>(std::strlen(kPrefix)))
                    .toLower();
        }
    }
    if (!ffmpegHardwareSelfTestBackend.isEmpty()) {
        remote::FfmpegHardwareBackend backend =
            remote::FfmpegHardwareBackend::kAutomatic;
        if (ffmpegHardwareSelfTestBackend == QStringLiteral("qsv")) {
            backend = remote::FfmpegHardwareBackend::kQsv;
        } else if (ffmpegHardwareSelfTestBackend ==
                   QStringLiteral("nvenc")) {
            backend = remote::FfmpegHardwareBackend::kNvenc;
        } else if (ffmpegHardwareSelfTestBackend ==
                   QStringLiteral("amf")) {
            backend = remote::FfmpegHardwareBackend::kAmf;
        }
        const auto result =
            remote::RunFfmpegHardwareEncoderSelfTest(backend);
        QTextStream output(stdout);
        output << "FFMPEG_HARDWARE_ENCODER_SELF_TEST="
               << (result.succeeded ? "PASS" : "FAIL") << Qt::endl;
        output << "FFMPEG_HARDWARE_IMPLEMENTATION="
               << QString::fromStdString(result.implementation) << Qt::endl;
        output << "FFMPEG_HARDWARE_ENCODED_FRAMES="
               << result.frames << Qt::endl;
        output << "FFMPEG_HARDWARE_ENCODED_BYTES="
               << static_cast<qulonglong>(result.encodedBytes) << Qt::endl;
        if (!result.error.empty()) {
            output << "FFMPEG_HARDWARE_ERROR="
                   << QString::fromStdString(result.error) << Qt::endl;
        }
        return result.succeeded ? 0 : 1;
    }

    QString desktopCaptureSelfTestBackend;
    for (const QString& argument : application.arguments()) {
        constexpr auto kDesktopCaptureSelfTestPrefix =
            "--desktop-capture-self-test=";
        if (argument.startsWith(
                QString::fromLatin1(kDesktopCaptureSelfTestPrefix))) {
            desktopCaptureSelfTestBackend = argument.mid(
                static_cast<int>(
                    std::char_traits<char>::length(
                        kDesktopCaptureSelfTestPrefix)));
            break;
        }
    }
    if (application.arguments().contains(
            QStringLiteral("--desktop-capture-self-test")) ||
        !desktopCaptureSelfTestBackend.isEmpty()) {
        auto implementation = ConfiguredDesktopCaptureImplementation();
        if (desktopCaptureSelfTestBackend == QStringLiteral("libwebrtc")) {
            implementation =
                remote::DesktopCaptureImplementation::kLibWebRtc;
        } else if (desktopCaptureSelfTestBackend ==
                   QStringLiteral("native_dxgi")) {
            implementation =
                remote::DesktopCaptureImplementation::kNativeDxgi;
        }
        auto source =
            webrtc::make_ref_counted<remote::WindowsDesktopCaptureSource>(
                implementation);
        const bool ready = source->StartCapture();
        QTextStream output(stdout);
        output << "DESKTOP_CAPTURE_FIRST_FRAME="
               << (ready ? "YES" : "NO") << Qt::endl;
        output << "DESKTOP_CAPTURE_CONFIGURED="
               << (source->ConfiguredImplementation() ==
                           remote::DesktopCaptureImplementation::kLibWebRtc
                       ? "LIBWEBRTC"
                       : "NATIVE_DXGI")
               << Qt::endl;
        QString backendName;
        switch (source->Backend()) {
        case remote::WindowsDesktopCaptureSource::CaptureBackend::
            kDxgiNativeTexture:
            backendName = QStringLiteral("NATIVE_DXGI_TEXTURE");
            break;
        case remote::WindowsDesktopCaptureSource::CaptureBackend::
            kDxgiPreferred:
            backendName = QStringLiteral(
                "LIBWEBRTC_DXGI_WITH_GDI_FALLBACK");
            break;
        case remote::WindowsDesktopCaptureSource::CaptureBackend::kGdi:
            backendName = QStringLiteral("LIBWEBRTC_GDI");
            break;
        }
        output << "DESKTOP_CAPTURE_BACKEND="
               << backendName << Qt::endl;
        if (!source->FallbackReason().empty()) {
            output << "DESKTOP_CAPTURE_FALLBACK="
                   << QString::fromStdString(source->FallbackReason())
                   << Qt::endl;
        }
        if (ready) {
            std::this_thread::sleep_for(std::chrono::seconds(3));
            const auto stats = source->CaptureRuntimeStats();
            QString activity = QStringLiteral("STARTING");
            if (stats.activityState ==
                remote::WindowsDesktopCaptureSource::
                    CaptureActivityState::kActive) {
                activity = QStringLiteral("ACTIVE");
            } else if (stats.activityState ==
                       remote::WindowsDesktopCaptureSource::
                           CaptureActivityState::kIdle) {
                activity = QStringLiteral("IDLE");
            }
            output << "DESKTOP_CAPTURE_ACTIVITY=" << activity
                   << Qt::endl;
            output << "DESKTOP_CAPTURE_ATTEMPT_FPS="
                   << QString::number(
                          stats.captureAttemptsPerSecond, 'f', 3)
                   << Qt::endl;
            output << "DESKTOP_CAPTURE_CHANGED_FPS="
                   << QString::number(
                          stats.changedFramesPerSecond, 'f', 3)
                   << Qt::endl;
            output << "DESKTOP_CAPTURE_DELIVERED_FPS="
                   << QString::number(
                          stats.deliveredFramesPerSecond, 'f', 3)
                   << Qt::endl;
            output << "DESKTOP_CAPTURE_HEARTBEAT_FPS="
                   << QString::number(
                          stats.idleHeartbeatFramesPerSecond, 'f', 3)
                   << Qt::endl;
            output << "DESKTOP_CAPTURE_SUPPRESSED_TOTAL="
                   << stats.totalSuppressedUnchangedFrames
                   << Qt::endl;
            output << "DESKTOP_CAPTURE_INPUT_BOOSTS="
                   << stats.totalInputBoosts << Qt::endl;
            output << "DESKTOP_CAPTURE_FORCED_REFRESHES="
                   << stats.totalForcedRefreshFrames << Qt::endl;
        }
        if (!ready) {
            output << "DESKTOP_CAPTURE_ERROR="
                   << QString::fromStdString(source->LastError())
                   << Qt::endl;
        }
        source->StopCapture();
        output << "DESKTOP_CAPTURE_STOPPED=YES" << Qt::endl;
        source = nullptr;
        output << "DESKTOP_CAPTURE_SOURCE_RELEASED=YES" << Qt::endl;
        return ready ? 0 : 1;
    }

    if (application.arguments().contains(QStringLiteral("--status-once"))) {
        remote::app::InProcessSessionEngine engine;
        const auto result = engine.Start();
        QTextStream output(stdout);
        output << "REMOTEC_PRODUCT_SHELL_READY="
               << (result.accepted ? "YES" : "NO") << Qt::endl;
        // Transitional marker retained for existing product smoke scripts.
        output << "CONTROLLER_PRODUCT_SHELL_READY="
               << (result.accepted ? "YES" : "NO") << Qt::endl;
        if (!result.accepted) {
            output << QString::fromStdString(result.errorMessage) << Qt::endl;
        }
        engine.Stop();
        return result.accepted ? 0 : 1;
    }

    if (application.arguments().contains(
            QStringLiteral("--theme-roundtrip-self-test"))) {
        auto engine = std::make_unique<remote::app::InProcessSessionEngine>();
        remote::controller::ControllerMainWindow window(
            std::move(engine), false);
        window.setAttribute(Qt::WA_DontShowOnScreen, true);
        window.resize(1600, 900);
        window.show();
        QCoreApplication::processEvents(QEventLoop::AllEvents);
        QString error;
        const bool passed = window.RunThemeRoundTripSelfTest(&error);
        QTextStream output(stdout);
        output << "THEME_COLOR_ONLY_LAYOUT="
               << (passed ? "PASS" : "FAIL") << Qt::endl;
        if (!error.isEmpty()) output << error << Qt::endl;
        window.close();
        return passed ? 0 : 1;
    }

    const StartupSignalingConfiguration startupConfiguration =
        LoadStartupSignalingConfiguration();
    if (!startupConfiguration.sslBackend.isEmpty() &&
        !qEnvironmentVariableIsSet("QT_SSL_BACKEND")) {
        qputenv("QT_SSL_BACKEND",
                startupConfiguration.sslBackend.toUtf8());
    }
    const bool startInTray =
        settings.value(QStringLiteral("app/startupVisibility"),
                       QStringLiteral("window")).toString() ==
        QStringLiteral("tray");
    const QString deviceVerificationCode =
        GenerateSessionVerificationCode();
    remote::app::RemoteCApplicationCoordinator::Options coordinatorOptions;
    coordinatorOptions.authenticationRequired =
        startupConfiguration.authenticationRequired;
    coordinatorOptions.startMainWindowInTray = startInTray;
    coordinatorOptions.authConfig = startupConfiguration.authConfig;
    coordinatorOptions.configurationError = startupConfiguration.error;
    coordinatorOptions.configurationSource = startupConfiguration.source;
    std::unique_ptr<remote::auth::TokenStore> tokenStore;
    if (startupConfiguration.authenticationRequired) {
        tokenStore =
            std::make_unique<remote::auth::DpapiTokenStore>();
    }
    remote::app::RemoteCApplicationCoordinator coordinator(
        std::move(coordinatorOptions),
        [startupConfiguration, deviceVerificationCode](
            const QString& accessToken) mutable {
            StartupSignalingConfiguration configuration =
                startupConfiguration;
            if (configuration.authenticationRequired) {
                configuration.accessToken = accessToken;
                configuration.configured =
                    !configuration.invalid &&
                    !configuration.endpoint.isEmpty();
            }
            auto engine = CreateSessionEngine(
                configuration, deviceVerificationCode);
            auto* enginePointer = engine.get();
            remote::app::RemoteCApplicationCoordinator::MainWindowSession
                session;
            session.window = std::make_unique<
                remote::controller::ControllerMainWindow>(
                    std::move(engine),
                    !configuration.authenticationRequired);
            session.updateAccessToken =
                [enginePointer](const QString& latestAccessToken) {
                    if (!enginePointer) {
                        return false;
                    }
                    return enginePointer->UpdateSignalingAccessToken(
                        ToUtf8(latestAccessToken)).accepted;
                };
            session.requestAccountDeletion =
                [enginePointer](QString* errorMessage) {
                    if (!enginePointer) {
                        if (errorMessage) {
                            *errorMessage = QStringLiteral(
                                "RLink 会话引擎不可用。");
                        }
                        return false;
                    }
                    const auto result =
                        enginePointer->RequestAccountDeletion();
                    if (!result.accepted && errorMessage) {
                        *errorMessage = QString::fromStdString(
                            result.errorMessage);
                    }
                    return result.accepted;
                };
            session.setAccountDeletionResultCallback =
                [enginePointer](
                    std::function<void(
                        remote::app::RemoteCApplicationCoordinator::
                            AccountDeletionResult)> callback) {
                    if (!enginePointer) {
                        return;
                    }
                    enginePointer->SetAccountDeletionResultCallback(
                        [callback = std::move(callback)](
                            const remote::SignalingAccountDeletionResult&
                                result) {
                            if (!callback) {
                                return;
                            }
                            remote::app::RemoteCApplicationCoordinator::
                                AccountDeletionResult converted;
                            converted.deleted = result.deleted;
                            converted.code = QString::fromStdString(
                                result.errorCode);
                            converted.message = QString::fromStdString(
                                result.errorMessage);
                            converted.retryable = result.retryable;
                            callback(std::move(converted));
                        });
                };
            return session;
        },
        std::move(tokenStore));
    QObject::connect(
        &singleInstanceServer, &QLocalServer::newConnection,
        &coordinator, [&singleInstanceServer, &coordinator] {
            while (singleInstanceServer.hasPendingConnections()) {
                QLocalSocket* socket =
                    singleInstanceServer.nextPendingConnection();
                if (!socket) {
                    continue;
                }
                QObject::connect(socket, &QLocalSocket::readyRead,
                                 socket, [socket, &coordinator] {
                    if (socket->readAll().contains("activate")) {
                        coordinator.ActivateFromExternalLaunch();
                    }
                });
                QTimer::singleShot(0, socket, [socket, &coordinator] {
                    if (socket->bytesAvailable() > 0 &&
                        socket->readAll().contains("activate")) {
                        coordinator.ActivateFromExternalLaunch();
                    }
                });
                QObject::connect(socket, &QLocalSocket::disconnected,
                                 socket, &QObject::deleteLater);
            }
        });
    if (!coordinator.Start()) {
        remote::controller::RemoteCDialog::Alert(
            nullptr, QStringLiteral("RLink 启动失败"),
            QStringLiteral("无法创建 RLink 应用窗口。"),
            QStringLiteral("知道了"),
            remote::controller::RemoteCDialog::Tone::kDanger);
        return 2;
    }
    return application.exec();
}
