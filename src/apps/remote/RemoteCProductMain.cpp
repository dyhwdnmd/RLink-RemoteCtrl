// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#include <algorithm>
#include <atomic>
#include <chrono>
#include <cstdint>
#include <cstring>
#include <functional>
#include <memory>
#include <thread>
#include <utility>

#include <Windows.h>

#include <QApplication>
#include <QAbstractItemView>
#include <QCryptographicHash>
#include <QDoubleSpinBox>
#include <QComboBox>
#include <QLineEdit>
#include <QValidator>
#include <cmath>
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
#include <QPushButton>
#include <QRandomGenerator>
#include <QScreen>
#include <QScrollBar>
#include <QSettings>
#include <QStandardPaths>
#include <QStringList>
#include <QSysInfo>
#include <QSystemTrayIcon>
#include <QStackedWidget>
#include <QTextStream>
#include <QToolButton>
#include <QTemporaryDir>
#include <QTimer>
#include <QUuid>
#include <QVBoxLayout>
#include <QWidget>
#include <QtGlobal>

#include "api/make_ref_counted.h"
#include "InProcessSessionEngine.h"
#include "DirectSessionCoordinator.h"
#include "LocalMediaCoordinator.h"
#include "RoomSessionCoordinator.h"
#include "ScreenShareCoordinator.h"
#include "EncoderBenchmarkProfileCache.h"
#include "src/platform/win/WindowsDesktopCaptureSource.h"
#include "src/platform/win/DesktopCaptureTiming.h"
#include "src/platform/win/WindowsHardwareFingerprint.h"
#include "src/platform/win/MfD3D11H264DecoderBenchmark.h"
#include "src/platform/win/FfmpegD3D11H264Decoder.h"
#include "src/platform/win/H264EncoderBenchmark.h"
#include "src/platform/win/VideoEncoderProbePolicy.h"
#include "src/platform/win/FfmpegX264H264Encoder.h"
#include "src/platform/win/FfmpegHardwareH264Encoder.h"
#include "src/platform/win/VideoDecoderProbePolicy.h"
#include "src/apps/controller/ControllerMainWindow.h"
#include "src/apps/controller/pages/SettingsPage.h"
#include "src/apps/controller/pages/DiagnosticsPage.h"
#include "src/apps/controller/pages/ContentPolicyCards.h"
#include "src/apps/controller/ControllerMainWindowSupport.h"
#include "src/apps/controller/ui/RemoteCTheme.h"
#include "src/apps/controller/RemoteCDialog.h"
#include "src/apps/controller/WindowsAutoStart.h"
#include "src/apps/controller/LoginWindow.h"
#include "src/apps/remote/RemoteCApplicationCoordinator.h"
#include "src/apps/remote/adapters/VisionApiFrameAnalyzer.h"
#include "src/auth/AuthConfig.h"
#include "src/core/ScreenStreamPolicy.h"
#include "src/auth/DpapiTokenStore.h"
#include "src/signaling/QtWebSocketSignalingClient.h"

namespace remote::app {

// Test-only entry, invoked only by --auth-coordinator-self-test. All stores and
// settings are isolated; failure injection never reaches a real OAuth service.
class StartupAuthenticationProbe final {
public:
    static bool VerifyRetry(RemoteCApplicationCoordinator& coordinator,
                            int& loadCalls) {
        QSettings settings;
        settings.setValue(QStringLiteral("auth/accountLabel"),
                          QStringLiteral("retry-test-account"));
        coordinator.startupAuthenticationPending_ = true;
        const auth::AuthError temporaryFailure{
            auth::AuthErrorCode::kDiscoveryFailed,
            QStringLiteral("isolated temporary failure"), true};
        coordinator.HandleAuthFailure(temporaryFailure);
        const bool firstRetry = coordinator.startupAuthenticationPending_ &&
            coordinator.authenticationRetryTimer_.isActive() &&
            coordinator.authenticationRetryTimer_.interval() == 5000 &&
            settings.value(QStringLiteral("auth/accountLabel")).toString() ==
                QStringLiteral("retry-test-account") &&
            !coordinator.mainWindow_->isVisible() &&
            !coordinator.loginWindow_->isVisible();
        for (int i = 0; i < 8; ++i) {
            coordinator.HandleAuthFailure(temporaryFailure);
        }
        const bool cappedRetry =
            coordinator.authenticationRetryTimer_.interval() == 60000 &&
            coordinator.authenticationRetryAttempt_ == 4;
        const int previousLoads = loadCalls;
        coordinator.authenticationRetryTimer_.start(0);
        QEventLoop loop;
        QTimer::singleShot(20, &loop, &QEventLoop::quit);
        loop.exec();
        const bool retriedRestore = loadCalls == previousLoads + 1 &&
            !coordinator.startupAuthenticationPending_ &&
            !coordinator.authenticationRetryTimer_.isActive() &&
            !coordinator.mainWindow_->isVisible() &&
            !coordinator.loginWindow_->isVisible();

        // The UI's real cancel callback must invalidate a scheduled retry.
        coordinator.startupAuthenticationPending_ = true;
        coordinator.HandleAuthFailure(temporaryFailure);
        bool cancelClicked = false;
        for (auto* button : coordinator.loginWindow_->findChildren<QPushButton*>()) {
            if (button->text() == QStringLiteral("取消当前操作")) {
                button->click();
                cancelClicked = true;
                break;
            }
        }
        const bool cancelled = cancelClicked &&
            !coordinator.startupAuthenticationPending_ &&
            !coordinator.authenticationRetryTimer_.isActive();

        coordinator.startupAuthenticationPending_ = true;
        coordinator.HandleAuthFailure(temporaryFailure);
        coordinator.HandleAuthFailure({auth::AuthErrorCode::kRefreshRejected,
            QStringLiteral("isolated invalid credentials"), false});
        const bool permanentStops = !coordinator.startupAuthenticationPending_ &&
            !coordinator.authenticationRetryTimer_.isActive() &&
            !settings.contains(QStringLiteral("auth/accountLabel")) &&
            !coordinator.loginWindow_->isVisible();

        // A routine refresh failure must also restore from the store instead
        // of attempting RefreshAccessToken while AuthManager is in kError.
        coordinator.authenticated_ = true;
        coordinator.HandleAuthFailure(temporaryFailure);
        const bool refreshRetry = coordinator.authenticationRetryTimer_.isActive();
        const int previousRefreshLoads = loadCalls;
        coordinator.authenticationRetryTimer_.start(0);
        QEventLoop refreshLoop;
        QTimer::singleShot(20, &refreshLoop, &QEventLoop::quit);
        refreshLoop.exec();
        const bool refreshRestored = loadCalls == previousRefreshLoads + 1 &&
            !coordinator.authenticated_ &&
            !coordinator.authenticationRetryTimer_.isActive();
        return firstRetry && cappedRetry && retriedRestore && cancelled &&
            permanentStops && refreshRetry && refreshRestored;
    }
};

}  // namespace remote::app

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
            argument == QStringLiteral(
                "--direct-session-coordinator-self-test") ||
            argument == QStringLiteral(
                "--room-session-coordinator-self-test") ||
            argument == QStringLiteral(
                "--local-media-coordinator-self-test") ||
            argument == QStringLiteral(
                "--screen-share-coordinator-self-test") ||
            argument == QStringLiteral("--theme-roundtrip-self-test") ||
            argument == QStringLiteral("--content-policy-layout-preview") ||
            argument == QStringLiteral("--screen-bpp-settings-self-test") ||
            argument == QStringLiteral("--signaling-policy-self-test") ||
            argument == QStringLiteral("--decoder-optimal-probe") ||
            argument.startsWith(
                QStringLiteral("--encoder-optimal-probe=")) ||
            argument == QStringLiteral(
                "--ffmpeg-x264-encoder-self-test") ||
            argument.startsWith(
                QStringLiteral("--ffmpeg-hardware-encoder-self-test=")) ||
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
        QStringLiteral("libwebrtc")).toString();
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
    const QPointer<QObject> ownerThreadContext(
        QCoreApplication::instance());
    engineOptions.ownerThreadDispatcher =
        [ownerThreadContext](std::function<void()> task) {
            if (!ownerThreadContext || !task) {
                return false;
            }
            return QMetaObject::invokeMethod(
                ownerThreadContext.data(), std::move(task),
                Qt::QueuedConnection);
        };
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
    engineOptions.screenVideoBitrateBppProvider = [] {
        return remote::NormalizeScreenVideoBitrateBppHundredths(
            QSettings().value(
                QStringLiteral("media/screenVideoBitrateBppHundredths"),
                remote::kDefaultScreenVideoBitrateBppHundredths).toUInt());
    };
    engineOptions.screenQualityDeficitShareProvider = [] {
        return remote::controller::detail::ConfiguredScreenQualityDeficitShareHundredths();
    };
#if defined(RLINK_ENABLE_CONTENT_ANALYZER) && \
    RLINK_ENABLE_CONTENT_ANALYZER
    engineOptions.contentAnalyzerEnabled = mediaSettings.value(
        QStringLiteral("media/contentAnalyzerEnabled"),
        false).toBool();
    engineOptions.contentAnalyzerRateHz = static_cast<std::uint32_t>(
        std::clamp(
            mediaSettings.value(
                QStringLiteral("media/contentAnalyzerRateHz"),
                3).toInt(),
            2,
            5));
#if defined(RLINK_ENABLE_REMOTE_VISION_API) && \
    RLINK_ENABLE_REMOTE_VISION_API
    const QString analyzerMode = mediaSettings.value(
        QStringLiteral("media/contentAnalyzerMode"),
        QStringLiteral("local")).toString();
    const std::uint64_t configRevision = mediaSettings.value(
        QStringLiteral("media/visionApiConfigRevision"),
        0).toULongLong();
    const std::uint64_t testedRevision = mediaSettings.value(
        QStringLiteral("media/visionApiTestedRevision"),
        0).toULongLong();
    const std::uint64_t consentRevision = mediaSettings.value(
        QStringLiteral("media/visionApiConsentRevision"),
        0).toULongLong();
    const QString credentialId = mediaSettings.value(
        QStringLiteral("media/visionApiCredentialId")).toString();
    if (engineOptions.contentAnalyzerEnabled &&
        analyzerMode == QStringLiteral("vision_api") &&
        configRevision != 0 && testedRevision == configRevision &&
        consentRevision == 1 && !credentialId.isEmpty()) {
        remote::media_intelligence::VisionApiRuntimeConfig visionConfig;
        const QString visionProvider = mediaSettings.value(
            QStringLiteral("media/visionApiProvider"),
            QStringLiteral("openai_compatible")).toString();
        if (visionProvider == QStringLiteral("deepseek")) {
            visionConfig.endpoint =
                remote::media_intelligence::DeepSeekVisionApiPreset();
        }
        visionConfig.endpoint.providerId = ToUtf8(visionProvider);
        visionConfig.endpoint.baseUrl = ToUtf8(mediaSettings.value(
            QStringLiteral("media/visionApiBaseUrl")).toString().trimmed());
        visionConfig.endpoint.model = ToUtf8(mediaSettings.value(
            QStringLiteral("media/visionApiModel")).toString().trimmed());
        visionConfig.endpoint.credentialId = ToUtf8(credentialId);
        visionConfig.endpoint.imageDetail =
            remote::media_intelligence::VisionImageDetail::kLow;
        visionConfig.endpoint.timeoutMs = 8000;
        visionConfig.endpoint.maximumResponseBytes = 16 * 1024;
        visionConfig.minimumRequestIntervalMs =
            remote::media_intelligence::NormalizeVisionApiRequestIntervalMs(
                mediaSettings.value(
                    QStringLiteral("media/visionApiRequestIntervalSeconds"),
                    remote::media_intelligence::kDefaultVisionApiRequestIntervalMs / 1000.0).toDouble());
        visionConfig.maximumConsecutiveFailures = 3;
        visionConfig.circuitBreakDurationMs = 5 * 60 * 1000;
        const auto activationCheck = [configRevision] {
            const QSettings current;
            return current.value(
                       QStringLiteral("media/contentAnalyzerEnabled"),
                       false).toBool() &&
                current.value(
                       QStringLiteral("media/contentAnalyzerMode"),
                       QStringLiteral("local")).toString() ==
                    QStringLiteral("vision_api") &&
                current.value(
                       QStringLiteral("media/visionApiConsentRevision"),
                       0).toULongLong() == 1 &&
                current.value(
                       QStringLiteral("media/visionApiConfigRevision"),
                       0).toULongLong() == configRevision &&
                current.value(
                       QStringLiteral("media/visionApiTestedRevision"),
                       0).toULongLong() == configRevision;
        };
        engineOptions.remoteVisionAnalyzer =
            remote::app::VisionApiFrameAnalyzer::Create(
                std::move(visionConfig),
                {
                    .activationCheck = activationCheck,
                    .encodingOptionsProvider = [] {
                        const QSettings current;
                        return remote::app::VisionApiFrameAnalyzer::
                            FrameEncodingOptions{
                                static_cast<std::uint32_t>(std::clamp(
                                    current.value(
                                        QStringLiteral(
                                            "media/visionApiMaximumImageDimension"),
                                        remote::media_intelligence::kDefaultVisionApiMaximumImageDimension).toInt(),
                                    256,
                                    1280)),
                                std::clamp(
                                    current.value(
                                        QStringLiteral(
                                            "media/visionApiJpegQuality"),
                                        remote::media_intelligence::kDefaultVisionApiJpegQuality).toInt(),
                                    30,
                                    90)};
                    },
                    .maximumImageDimension =
                        static_cast<std::uint32_t>(std::clamp(
                            mediaSettings.value(
                                QStringLiteral(
                                    "media/visionApiMaximumImageDimension"),
                                remote::media_intelligence::kDefaultVisionApiMaximumImageDimension).toInt(),
                            256,
                            1280)),
                    .jpegQuality = std::clamp(
                        mediaSettings.value(
                            QStringLiteral("media/visionApiJpegQuality"),
                            remote::media_intelligence::kDefaultVisionApiJpegQuality).toInt(),
                        30,
                        90),
                });
    }
#endif
#endif
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
        QStringLiteral("libwebrtc")).toString();
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

int RunDirectSessionCoordinatorSelfTest()
{
    remote::app::DirectSessionCoordinator coordinator;
    remote::app::DirectSessionStartPlan plan;

    remote::DirectSessionConnectRequest invalidAssistance;
    invalidAssistance.targetDeviceId = "123456789";
    invalidAssistance.purpose = remote::SessionPurpose::kRemoteControl;
    invalidAssistance.authorization =
        remote::DirectAuthorizationMethod::kVerificationCode;
    invalidAssistance.verificationCode = "12x456";
    const auto invalidResult = coordinator.PrepareOutgoingStart(
        invalidAssistance, &plan);

    remote::DirectSessionConnectRequest ownedRequest;
    ownedRequest.targetDeviceId = "987654321";
    ownedRequest.purpose = remote::SessionPurpose::kRemoteControl;
    ownedRequest.authorization =
        remote::DirectAuthorizationMethod::kOwnedAccount;
    const auto ownedResult = coordinator.PrepareOutgoingStart(
        ownedRequest, &plan);
    remote::SessionEngineSnapshot outgoingSnapshot;
    outgoingSnapshot.error.code = "old_error";
    outgoingSnapshot.error.message = "old message";
    coordinator.ApplyOutgoingStart(&outgoingSnapshot, plan);

    remote::SessionEngineSnapshot incomingSnapshot;
    incomingSnapshot.state =
        remote::SessionEngineState::kAwaitingLocalApproval;
    incomingSnapshot.sessionId = "incoming-session";
    incomingSnapshot.error.code = "old_error";
    incomingSnapshot.error.message = "old message";
    const auto wrongIncoming = coordinator.ValidateIncomingDecision(
        incomingSnapshot, "other-session", false);
    const auto validIncoming = coordinator.ValidateIncomingDecision(
        incomingSnapshot, "incoming-session", false);
    coordinator.ApplyIncomingAccepted(&incomingSnapshot);

    const bool validationPassed =
        !invalidResult.accepted &&
        invalidResult.errorCode == "invalid_assistance_credentials" &&
        ownedResult.accepted &&
        plan.origin == remote::SessionOrigin::kOwnedDevice &&
        plan.permissions ==
            std::vector<std::string>({"viewScreen", "controlInput"});
    const bool outgoingTransitionPassed =
        outgoingSnapshot.state ==
            remote::SessionEngineState::kConnecting &&
        outgoingSnapshot.origin == remote::SessionOrigin::kOwnedDevice &&
        outgoingSnapshot.remoteControlRole ==
            remote::RemoteControlRole::kController &&
        outgoingSnapshot.peerDeviceId == "987654321" &&
        outgoingSnapshot.error.code.empty() &&
        outgoingSnapshot.error.message.empty();
    const bool incomingTransitionPassed =
        !wrongIncoming.accepted &&
        wrongIncoming.errorCode == "incoming_session_not_found" &&
        validIncoming.accepted &&
        incomingSnapshot.state ==
            remote::SessionEngineState::kConnecting &&
        incomingSnapshot.error.code.empty() &&
        incomingSnapshot.error.message.empty();
    const bool passed = validationPassed && outgoingTransitionPassed &&
        incomingTransitionPassed;

    QTextStream output(stdout);
    output << "DIRECT_SESSION_REQUEST_POLICY="
           << (validationPassed ? "PASS" : "FAIL") << Qt::endl;
    output << "DIRECT_SESSION_OUTGOING_TRANSITION="
           << (outgoingTransitionPassed ? "PASS" : "FAIL") << Qt::endl;
    output << "DIRECT_SESSION_INCOMING_TRANSITION="
           << (incomingTransitionPassed ? "PASS" : "FAIL") << Qt::endl;
    output << "DIRECT_SESSION_COORDINATOR_SELF_TEST="
           << (passed ? "PASS" : "FAIL") << Qt::endl;
    return passed ? 0 : 1;
}

int RunRoomSessionCoordinatorSelfTest()
{
    remote::app::RoomSessionCoordinator coordinator;

    remote::SessionEngineSnapshot createSnapshot;
    const auto invalidCapacity = coordinator.ValidateCapacity(1);
    const auto validCreate = coordinator.ValidateCreate(createSnapshot, 4);
    coordinator.ApplyCreateRequested(&createSnapshot, 4);
    const bool createPassed =
        !invalidCapacity.accepted &&
        invalidCapacity.errorCode == "invalid_room_capacity" &&
        validCreate.accepted &&
        createSnapshot.room.membership ==
            remote::RoomMembershipState::kCreating &&
        createSnapshot.room.capacity == 4;

    remote::SessionEngineSnapshot joinSnapshot;
    const auto emptyJoin = coordinator.ValidateJoin(joinSnapshot, {});
    const auto validJoin = coordinator.ValidateJoin(joinSnapshot, "room-1");
    coordinator.ApplyJoinRequested(&joinSnapshot, "room-1");
    const bool joinPassed =
        !emptyJoin.accepted &&
        emptyJoin.errorCode == "room_id_empty" &&
        validJoin.accepted &&
        joinSnapshot.room.membership ==
            remote::RoomMembershipState::kJoinPending &&
        joinSnapshot.room.roomId == "room-1";

    const auto invalidAvailability =
        coordinator.ValidateAvailabilityQuery({});
    remote::SessionEngineSnapshot availabilitySnapshot;
    coordinator.ApplyAvailabilityQuery(
        &availabilitySnapshot, {"room-1", "room-2"});
    const bool availabilityPassed =
        !invalidAvailability.accepted &&
        invalidAvailability.errorCode ==
            "invalid_room_availability_query" &&
        availabilitySnapshot.roomActivity.availabilities.size() == 2 &&
        availabilitySnapshot.roomActivity.availabilities.front().state ==
            remote::RoomAvailabilityState::kChecking;

    remote::SessionEngineSnapshot leaveSnapshot;
    leaveSnapshot.room.membership = remote::RoomMembershipState::kActive;
    leaveSnapshot.room.roomId = "room-1";
    std::string roomId;
    const auto leave = coordinator.PrepareLeave(&leaveSnapshot, &roomId);
    coordinator.ApplyLeaveFailed(
        &leaveSnapshot, "network_error", "network unavailable");
    const bool leavePassed =
        leave.accepted && roomId == "room-1" &&
        leaveSnapshot.room.membership ==
            remote::RoomMembershipState::kActive &&
        leaveSnapshot.room.errorCode == "network_error";

    const bool passed = createPassed && joinPassed &&
        availabilityPassed && leavePassed;
    QTextStream output(stdout);
    output << "ROOM_SESSION_CREATE_POLICY="
           << (createPassed ? "PASS" : "FAIL") << Qt::endl;
    output << "ROOM_SESSION_JOIN_POLICY="
           << (joinPassed ? "PASS" : "FAIL") << Qt::endl;
    output << "ROOM_SESSION_AVAILABILITY_POLICY="
           << (availabilityPassed ? "PASS" : "FAIL") << Qt::endl;
    output << "ROOM_SESSION_LEAVE_TRANSITION="
           << (leavePassed ? "PASS" : "FAIL") << Qt::endl;
    output << "ROOM_SESSION_COORDINATOR_SELF_TEST="
           << (passed ? "PASS" : "FAIL") << Qt::endl;
    return passed ? 0 : 1;
}

int RunLocalMediaCoordinatorSelfTest()
{
    remote::app::LocalMediaCoordinator coordinator;
    remote::MediaDeviceCategorySnapshot category;
    category.devices.push_back({"camera-1", "Camera 1", true});

    const bool normalizationPassed =
        remote::app::LocalMediaCoordinator::NormalizeDeviceId({}) ==
            remote::kSystemDefaultMediaDeviceId &&
        remote::app::LocalMediaCoordinator::NormalizeDeviceId(
            "camera-1") == "camera-1";
    const auto valid = coordinator.ValidateSelection(
        category, "camera-1",
        remote::app::LocalMediaDeviceKind::kCamera);
    const auto unavailable = coordinator.ValidateSelection(
        category, "camera-2",
        remote::app::LocalMediaDeviceKind::kCamera);
    category.state = remote::MediaDeviceSelectionState::kSwitching;
    const auto switching = coordinator.ValidateSelection(
        category, "camera-1",
        remote::app::LocalMediaDeviceKind::kCamera);
    const bool validationPassed = valid.accepted &&
        !unavailable.accepted &&
        unavailable.errorCode == "camera_device_unavailable" &&
        !switching.accepted &&
        switching.errorCode == "camera_device_switch_in_progress";

    category.state = remote::MediaDeviceSelectionState::kReady;
    category.preferredDeviceId = "camera-2";
    coordinator.UpdateAvailability(
        category, remote::app::LocalMediaDeviceKind::kCamera);
    const bool availabilityPassed =
        category.state ==
            remote::MediaDeviceSelectionState::kUnavailable &&
        category.errorCode == "camera_device_unavailable" &&
        category.errorMessage ==
            "The selected camera is not connected.";

    const auto cameraGeneration = coordinator.BeginCameraOperation();
    const bool cameraGenerationPassed = cameraGeneration == 1 &&
        coordinator.IsCurrentCameraOperation(cameraGeneration) &&
        !coordinator.IsCurrentCameraOperation(cameraGeneration + 1);

    const bool passed = normalizationPassed && validationPassed &&
        availabilityPassed && cameraGenerationPassed;
    QTextStream output(stdout);
    output << "LOCAL_MEDIA_NORMALIZATION="
           << (normalizationPassed ? "PASS" : "FAIL") << Qt::endl;
    output << "LOCAL_MEDIA_SELECTION_POLICY="
           << (validationPassed ? "PASS" : "FAIL") << Qt::endl;
    output << "LOCAL_MEDIA_AVAILABILITY="
           << (availabilityPassed ? "PASS" : "FAIL") << Qt::endl;
    output << "LOCAL_MEDIA_CAMERA_GENERATION="
           << (cameraGenerationPassed ? "PASS" : "FAIL") << Qt::endl;
    output << "LOCAL_MEDIA_COORDINATOR_SELF_TEST="
           << (passed ? "PASS" : "FAIL") << Qt::endl;
    return passed ? 0 : 1;
}

int RunScreenShareCoordinatorSelfTest()
{
    remote::app::ScreenShareCoordinator coordinator;
    const bool generationPassed =
        coordinator.NextGeneration() == 1 &&
        coordinator.BeginShare() == 1 &&
        coordinator.IsCurrentGeneration(1) &&
        coordinator.NextGeneration() == 2;
    coordinator.CommitGeneration(2);
    const bool committedGenerationPassed =
        coordinator.IsCurrentGeneration(2) &&
        !coordinator.IsCurrentGeneration(1);

    remote::DisplayTopologySnapshot topology;
    remote::DisplayDescriptor primary;
    primary.sessionDisplayId = 1;
    primary.stableDisplayKey = "primary";
    primary.primary = true;
    remote::DisplayDescriptor secondary;
    secondary.sessionDisplayId = 2;
    secondary.stableDisplayKey = "secondary";
    topology.displays = {primary, secondary};
    const auto preferred =
        remote::app::ScreenShareCoordinator::SelectDisplay(
            topology, "secondary");
    const auto fallback =
        remote::app::ScreenShareCoordinator::SelectDisplay(
            topology, "missing");
    const bool selectionPassed = preferred && fallback &&
        preferred->stableDisplayKey == "secondary" &&
        fallback->stableDisplayKey == "primary";

    remote::ScreenStreamPreferenceRequest request;
    request.maxWidth = 1280;
    request.maxHeight = 720;
    request.framesPerSecond = 30;
    const auto policy =
        remote::app::ScreenShareCoordinator::ResolvePolicy(
            1920, 1080, request);
    const bool policyPassed = policy.width == 1280 &&
        policy.height == 720 && policy.framesPerSecond == 30 &&
        remote::app::ScreenShareCoordinator::MaximumCaptureFrameRate(
            remote::DesktopCaptureImplementation::kLibWebRtc) ==
            remote::kMaximumScreenFrameRate;

    const bool passed = generationPassed &&
        committedGenerationPassed && selectionPassed && policyPassed;
    QTextStream output(stdout);
    output << "SCREEN_SHARE_GENERATION="
           << (generationPassed && committedGenerationPassed
                   ? "PASS" : "FAIL") << Qt::endl;
    output << "SCREEN_SHARE_DISPLAY_SELECTION="
           << (selectionPassed ? "PASS" : "FAIL") << Qt::endl;
    output << "SCREEN_SHARE_POLICY="
           << (policyPassed ? "PASS" : "FAIL") << Qt::endl;
    output << "SCREEN_SHARE_COORDINATOR_SELF_TEST="
           << (passed ? "PASS" : "FAIL") << Qt::endl;
    return passed ? 0 : 1;
}

int RunAuthCoordinatorSelfTest()
{
    QTemporaryDir temporaryDirectory;
    if (!temporaryDirectory.isValid()) {
        QTextStream(stdout) << "AUTH_COORDINATOR_SELF_TEST=FAIL" << Qt::endl;
        return 1;
    }
    // This self-test exits the process afterwards. Isolate both scopes before
    // any coordinator/UI settings access, especially the failure callback that
    // removes persisted account labels.
    QSettings::setDefaultFormat(QSettings::IniFormat);
    QSettings::setPath(QSettings::IniFormat, QSettings::UserScope,
                      temporaryDirectory.path());
    QSettings::setPath(QSettings::IniFormat, QSettings::SystemScope,
                      temporaryDirectory.path());
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

    class StartupTokenStore final : public remote::auth::TokenStore {
    public:
        StartupTokenStore(remote::auth::TokenStoreLoadStatus status,
                          int* loadCalls)
            : status_(status), loadCalls_(loadCalls) {}

        remote::auth::TokenStoreLoadStatus Load(
            remote::auth::StoredRefreshToken*, QString* error) override
        {
            ++*loadCalls_;
            if (status_ == remote::auth::TokenStoreLoadStatus::kError && error) {
                *error = QStringLiteral("expected credential read failure");
            }
            return status_;
        }
        bool Save(const remote::auth::StoredRefreshToken&, QString*) override
        {
            return false;
        }
        bool Clear(QString*) override { return true; }

    private:
        remote::auth::TokenStoreLoadStatus status_;
        int* loadCalls_;
    };

    const auto verifyEarlyRestore = [](
        remote::auth::TokenStoreLoadStatus status, bool background = false) {
        QSettings settings;
        settings.setValue(QStringLiteral("auth/accountLabel"),
                          QStringLiteral("isolated self-test account"));
        settings.sync();
        int loadCalls = 0;
        int factoryCalls = 0;
        int tokenUpdates = 0;
        bool storeLoadedBeforeFactory = false;
        remote::controller::ControllerMainWindow* mainWindow = nullptr;
        remote::app::InProcessSessionEngine* engine = nullptr;
        remote::app::RemoteCApplicationCoordinator::Options restoreOptions;
        restoreOptions.authenticationRequired = true;
        restoreOptions.backgroundStartup = background;
        restoreOptions.startMainWindowInTray = background;
        restoreOptions.authConfig.issuer = QUrl(
            QStringLiteral("https://example.invalid"));
        restoreOptions.authConfig.clientId = QStringLiteral("selftest");
        remote::app::RemoteCApplicationCoordinator restoreCoordinator(
            std::move(restoreOptions),
            [&](const QString& token) {
                ++factoryCalls;
                storeLoadedBeforeFactory = loadCalls == 1 && token.isEmpty();
                auto sessionEngine =
                    std::make_unique<remote::app::InProcessSessionEngine>();
                engine = sessionEngine.get();
                remote::app::RemoteCApplicationCoordinator::MainWindowSession
                    session;
                session.window = std::make_unique<
                    remote::controller::ControllerMainWindow>(
                        std::move(sessionEngine), false);
                mainWindow = session.window.get();
                session.updateAccessToken = [&](const QString&) {
                    ++tokenUpdates;
                    return true;
                };
                return session;
            },
            std::make_unique<StartupTokenStore>(status, &loadCalls));
        const bool restoreStarted = restoreCoordinator.Start();
        bool noStartupSplash = true;
        if (background) {
            for (QWidget* widget : QApplication::allWidgets()) {
                if (widget->objectName() == QStringLiteral("loginStatusWindow") &&
                    widget->isVisible()) noStartupSplash = false;
            }
        }
        const bool restorationPrecedesUi = loadCalls == 1 && factoryCalls == 0;
        const bool synchronousFailureDeferred = settings.contains(
            QStringLiteral("auth/accountLabel"));
        QEventLoop restoreLoop;
        QTimer::singleShot(150, &restoreLoop, &QEventLoop::quit);
        restoreLoop.exec();

        QWidget* restoredLogin = mainWindow
            ? mainWindow->findChild<QWidget*>(QStringLiteral("loginWindow"))
            : nullptr;
        QLabel* loginStatus = restoredLogin
            ? restoredLogin->findChild<QLabel*>(QStringLiteral("loginStatus"))
            : nullptr;
        const bool errorExpected =
            status == remote::auth::TokenStoreLoadStatus::kError;
        const bool correctLoginState = loginStatus &&
            loginStatus->property("tone").toString() ==
                (errorExpected ? QStringLiteral("error")
                               : QStringLiteral("ready")) &&
            (!errorExpected || loginStatus->text() ==
                QStringLiteral("expected credential read failure"));
        const bool accountSettingsCorrect =
            settings.contains(QStringLiteral("auth/accountLabel")) !=
                errorExpected;
        // Both fake stores terminate before OIDC discovery, so no network
        // request or token validation can occur. The real engine stays stopped.
        const bool engineStopped = engine &&
            engine->Snapshot().state == remote::SessionEngineState::kStopped &&
            tokenUpdates == 0;
        const bool visibilityCorrect = mainWindow && restoredLogin &&
            mainWindow->isVisible() != background &&
            restoredLogin->isVisible() != background;
        bool retryPassed = true;
        bool manualActivationPassed = true;
        if (background && !errorExpected) {
            retryPassed = remote::app::StartupAuthenticationProbe::VerifyRetry(
                restoreCoordinator, loadCalls);
        }
        if (background) {
            restoreCoordinator.ActivateFromExternalLaunch();
            manualActivationPassed = mainWindow && mainWindow->isVisible();
        }
        return restoreStarted && restorationPrecedesUi && noStartupSplash &&
            synchronousFailureDeferred && storeLoadedBeforeFactory &&
            factoryCalls == 1 && visibilityCorrect && correctLoginState &&
            accountSettingsCorrect && engineStopped && retryPassed &&
            manualActivationPassed;
    };
    const bool missingCredentialsPassed = verifyEarlyRestore(
        remote::auth::TokenStoreLoadStatus::kNotFound);
    const bool unreadableCredentialsPassed = verifyEarlyRestore(
        remote::auth::TokenStoreLoadStatus::kError);
    const bool backgroundMissingPassed = verifyEarlyRestore(
        remote::auth::TokenStoreLoadStatus::kNotFound, true);
    const bool backgroundUnreadablePassed = verifyEarlyRestore(
        remote::auth::TokenStoreLoadStatus::kError, true);
    const bool passed = started && loginWindowCreated &&
        mainWindowCreatedAfterStartupYield && loginWindowAcceptsApplicationExit &&
        missingCredentialsPassed && unreadableCredentialsPassed &&
        backgroundMissingPassed && backgroundUnreadablePassed;
    QTextStream output(stdout);
    output << "AUTH_LOGIN_WINDOW="
           << (loginWindowCreated ? "PASS" : "FAIL") << Qt::endl;
    output << "AUTH_MAIN_WINDOW_CREATED_AFTER_STARTUP_YIELD="
           << (mainWindowCreatedAfterStartupYield ? "PASS" : "FAIL")
           << Qt::endl;
    output << "AUTH_LOGIN_WINDOW_ACCEPTS_APP_EXIT="
           << (loginWindowAcceptsApplicationExit ? "PASS" : "FAIL")
           << Qt::endl;
    output << "AUTH_RESTORE_BEFORE_UI_MISSING_CREDENTIALS="
           << (missingCredentialsPassed ? "PASS" : "FAIL") << Qt::endl;
    output << "AUTH_RESTORE_BEFORE_UI_UNREADABLE_CREDENTIALS="
           << (unreadableCredentialsPassed ? "PASS" : "FAIL") << Qt::endl;
    output << "AUTH_COORDINATOR_SELF_TEST="
           << (passed ? "PASS" : "FAIL") << Qt::endl;
    output << "AUTH_BACKGROUND_STARTUP_AND_RETRY="
           << (backgroundMissingPassed ? "PASS" : "FAIL") << Qt::endl;
    output << "AUTH_BACKGROUND_PERMANENT_FAILURE="
           << (backgroundUnreadablePassed ? "PASS" : "FAIL") << Qt::endl;
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
    const bool backgroundStartup = application.arguments().contains(
        QStringLiteral("--autostart"));
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
            // An automatic second launch must not raise the existing client.
            if (backgroundStartup) return 0;
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
            QStringLiteral("--direct-session-coordinator-self-test"))) {
        return RunDirectSessionCoordinatorSelfTest();
    }

    if (application.arguments().contains(
            QStringLiteral("--room-session-coordinator-self-test"))) {
        return RunRoomSessionCoordinatorSelfTest();
    }

    if (application.arguments().contains(
            QStringLiteral("--local-media-coordinator-self-test"))) {
        return RunLocalMediaCoordinatorSelfTest();
    }

    if (application.arguments().contains(
            QStringLiteral("--screen-share-coordinator-self-test"))) {
        return RunScreenShareCoordinatorSelfTest();
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
        class CaptureSmokeSink final : public webrtc::VideoSinkInterface<webrtc::VideoFrame> {
        public:
            void OnFrame(const webrtc::VideoFrame&) override { ++frames; }
            std::atomic<std::uint64_t> frames{0};
        } smokeSink;
        unsigned testFps = 60;
        for (const QString& argument : application.arguments()) {
            constexpr auto prefix = "--desktop-capture-self-test-fps=";
            if (argument.startsWith(QString::fromLatin1(prefix))) {
                bool valid = false;
                testFps = argument.mid(static_cast<int>(std::char_traits<char>::length(prefix))).toUInt(&valid);
                if (!valid || testFps < 5 || testFps > 120) return 1;
            }
        }
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
        source->SetTargetFrameRate(testFps);
        webrtc::VideoSinkWants smokeWants;
        smokeWants.max_framerate_fps = testFps;
        source->AddOrUpdateSink(&smokeSink, smokeWants);
        const bool ready = source->StartCapture();
        bool activityReady = false;
        bool deliveryReady = false;
        const bool wakeStress = application.arguments().contains(
            QStringLiteral("--desktop-capture-wake-stress"));
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
            const auto deliveredBefore = smokeSink.frames.load();
            const auto dispatchBefore = source->CaptureRuntimeStats().totalDeliveredFrames;
            const auto deliveryStarted = std::chrono::steady_clock::now();
            std::atomic<bool> wakeStressHealthy{true};
            std::atomic<std::uint64_t> wakeRequests{0};
            std::jthread wakeStorm;
            if (wakeStress) {
                wakeStorm = std::jthread([&](std::stop_token token) {
                    const auto timer = CreateWaitableTimerExW(nullptr, nullptr,
                        CREATE_WAITABLE_TIMER_HIGH_RESOLUTION, TIMER_MODIFY_STATE | SYNCHRONIZE);
                    const auto stop = CreateEventW(nullptr, TRUE, FALSE, nullptr);
                    const auto wake = CreateEventW(nullptr, FALSE, FALSE, nullptr);
                    {
                        std::stop_callback cancel(token, [stop] { if (stop) SetEvent(stop); });
                        wakeStressHealthy = timer && stop && wake;
                        while (wakeStressHealthy && !token.stop_requested()) {
                            // Refresh/input flags must not override the capture period,
                            // including repeated writes of the same user target.
                            source->SetTargetFrameRate(testFps);
                            source->NotifyRemoteInputActivity();
                            source->RequestRefreshFrame();
                            ++wakeRequests;
                            const auto result = remote::desktop_capture_timing::WaitForDeadline(
                                timer, stop, wake, std::chrono::steady_clock::now() +
                                    std::chrono::milliseconds(2), token);
                            if (result == remote::desktop_capture_timing::WaitResult::kFailed)
                                wakeStressHealthy = false;
                        }
                    }
                    if (wake) CloseHandle(wake);
                    if (stop) CloseHandle(stop);
                    if (timer) CloseHandle(timer);
                });
            }
            std::this_thread::sleep_for(std::chrono::seconds(3));
            if (wakeStorm.joinable()) {
                wakeStorm.request_stop();
                wakeStorm.join();
            }
            const double duration = std::chrono::duration<double>(
                std::chrono::steady_clock::now() - deliveryStarted).count();
            const auto sinkFrames = smokeSink.frames.load() - deliveredBefore;
            const auto stats = source->CaptureRuntimeStats();
            const auto dispatchFrames = stats.totalDeliveredFrames - dispatchBefore;
            const double sinkFps = sinkFrames / duration;
            const double dispatchFps = dispatchFrames / duration;
            // Libwebrtc's static-desktop suppression may produce fewer input
            // frames by design. Judge proxy loss against actual dispatch,
            // rather than falsely requiring static capture to run at its cap.
            deliveryReady = sinkFrames + 1 >= dispatchFrames * .98 &&
                sinkFrames <= dispatchFrames + 1 && sinkFps <= testFps * 1.10;
            if (implementation == remote::DesktopCaptureImplementation::kNativeDxgi)
                deliveryReady = deliveryReady && sinkFps >= testFps * .90;
            if (wakeStress) {
                deliveryReady = deliveryReady && wakeStressHealthy && wakeRequests > 100 &&
                    dispatchFps <= testFps + 1.0 && sinkFps <= testFps + 1.0;
                output << "DESKTOP_CAPTURE_STRESS_WAKE_REQUESTS=" << wakeRequests.load() << Qt::endl;
                output << "DESKTOP_CAPTURE_WAKE_STORM_RESPECTS_TARGET="
                       << (deliveryReady ? "PASS" : "FAIL") << Qt::endl;
            }
            output << "DESKTOP_CAPTURE_USER_TARGET_FPS=" << testFps << Qt::endl;
            output << "DESKTOP_CAPTURE_DISPATCH_INPUT_FPS=" << QString::number(dispatchFps, 'f', 3) << Qt::endl;
            output << "DESKTOP_CAPTURE_ACTUAL_SINK_FPS=" << QString::number(sinkFps, 'f', 3) << Qt::endl;
            output << "DESKTOP_CAPTURE_NO_SPURIOUS_SINK_DROPS=" << (deliveryReady ? "PASS" : "FAIL") << Qt::endl;
            activityReady = stats.totalDeliveredFrames > 0 && stats.activityState !=
                remote::WindowsDesktopCaptureSource::CaptureActivityState::kStarting;
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
            output << "DESKTOP_CAPTURE_ACTIVITY_INITIALIZED="
                   << (activityReady ? "PASS" : "FAIL") << Qt::endl;
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
        source->RemoveSink(&smokeSink);
        output << "DESKTOP_CAPTURE_STOPPED=YES" << Qt::endl;
        source = nullptr;
        output << "DESKTOP_CAPTURE_SOURCE_RELEASED=YES" << Qt::endl;
        return ready && activityReady && deliveryReady ? 0 : 1;
    }

    if (application.arguments().contains(QStringLiteral("--status-once"))) {
        remote::app::InProcessSessionEngine engine;
        const auto result = engine.Start();
        auto snapshot = engine.Snapshot();
        const auto deadline = std::chrono::steady_clock::now() +
            std::chrono::seconds(60);
        while (result.accepted &&
               snapshot.state == remote::SessionEngineState::kStarting &&
               std::chrono::steady_clock::now() < deadline) {
            QCoreApplication::processEvents(QEventLoop::AllEvents, 10);
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            snapshot = engine.Snapshot();
        }
        const bool ready = result.accepted &&
            snapshot.state == remote::SessionEngineState::kReady;
        QTextStream output(stdout);
        output << "REMOTEC_PRODUCT_SHELL_READY="
               << (ready ? "YES" : "NO") << Qt::endl;
        // Transitional marker retained for existing product smoke scripts.
        output << "CONTROLLER_PRODUCT_SHELL_READY="
               << (ready ? "YES" : "NO") << Qt::endl;
        if (!result.accepted) {
            output << QString::fromStdString(result.errorMessage) << Qt::endl;
        } else if (!ready) {
            output << QString::fromStdString(snapshot.error.message)
                   << Qt::endl;
        }
        engine.Stop();
        return ready ? 0 : 1;
    }

    if (application.arguments().contains(QStringLiteral("--screen-bpp-settings-self-test"))) {
        // Exercise actual input validation/persistence without changing user settings.
        QTemporaryDir testSettingsDirectory;
        if (!testSettingsDirectory.isValid()) return 1;
        QSettings::setDefaultFormat(QSettings::IniFormat);
        QSettings::setPath(QSettings::IniFormat, QSettings::UserScope,
                           testSettingsDirectory.path());
        application.setOrganizationName(QStringLiteral("RLinkSelfTests"));
        application.setApplicationName(QStringLiteral("ScreenConnectionBpp"));
        auto testEngine = std::make_unique<remote::app::InProcessSessionEngine>();
        auto* testMedia = testEngine->MediaAccess();
        remote::controller::ControllerMainWindow testWindow(
            std::move(testEngine), false, testMedia);
        testWindow.setAttribute(Qt::WA_DontShowOnScreen, true);
        testWindow.resize(1600, 1000);
        auto* settingsPage = testWindow.findChild<remote::controller::SettingsPage*>();
        if (!settingsPage) return 1;
        auto& page = *settingsPage;
        if (auto* pages = qobject_cast<QStackedWidget*>(page.parentWidget())) pages->setCurrentWidget(&page);
        page.setAttribute(Qt::WA_DontShowOnScreen, true);
        page.DetailStack()->setCurrentIndex(1);
        testWindow.show();
        page.show();
        QCoreApplication::processEvents(QEventLoop::AllEvents);
        auto* input = page.Controls().screenVideoBitrateBppInput;
        bool passed = input && input->minimum() == 0.03 && input->maximum() == 0.50 &&
            input->decimals() == 2 && input->value() == 0.15;
        QTextStream output(stdout);
        output << "SCREEN_BPP_INPUT_RANGE_AND_DEFAULT=" << (passed ? "PASS" : "FAIL") << Qt::endl;
        auto* lineEdit = input ? input->findChild<QLineEdit*>() : nullptr;
        bool validationPassed = lineEdit && lineEdit->validator();
        if (validationPassed) {
            for (const QString& value : {QStringLiteral("0.03"), QStringLiteral("0.17"), QStringLiteral("0.50")}) {
                QString text = value;
                int position = text.size();
                validationPassed = validationPassed && lineEdit->validator()->validate(text, position) == QValidator::Acceptable;
            }
            for (const QString& value : {QStringLiteral("0.02"), QStringLiteral("0.51"), QStringLiteral("0.031"), QStringLiteral("abc")}) {
                QString text = value;
                int position = text.size();
                validationPassed = validationPassed && lineEdit->validator()->validate(text, position) != QValidator::Acceptable;
            }
        }
        passed = passed && validationPassed;
        output << "SCREEN_BPP_INPUT_REJECTS_OUT_OF_RANGE_AND_EXTRA_DECIMALS=" << (validationPassed ? "PASS" : "FAIL") << Qt::endl;
        std::uint32_t lastPublishedBpp = 0;
        unsigned publishedBppCount = 0;
        QObject::connect(&page, &remote::controller::SettingsPage::ScreenVideoBitrateBppChanged,
            &page, [&](std::uint32_t value) { lastPublishedBpp = value; ++publishedBppCount; });
        for (const auto hundredths : {3u, 10u, 17u, 20u, 25u, 30u, 40u, 50u}) {
            if (input) input->setValue(hundredths / 100.0);
            QSettings settings;
            settings.sync();
            passed = passed && settings.value(
                QStringLiteral("media/screenVideoBitrateBppHundredths")).toUInt() == hundredths;
        }
        passed = passed && lastPublishedBpp == 50 && publishedBppCount == 8;
        output << "SCREEN_BPP_INPUT_PERSISTS_CUSTOM_VALUES=" << (passed ? "PASS" : "FAIL") << Qt::endl;
        page.close();
        remote::controller::SettingsPage restoredPage;
        passed = passed && restoredPage.Controls().screenVideoBitrateBppInput->value() == 0.50;
        output << "SCREEN_BPP_SETTINGS_RELOAD=" << (passed ? "PASS" : "FAIL") << Qt::endl;
        auto* qualityInput = page.Controls().screenQualityDeficitShareInput;
        bool qualityPassed = qualityInput && qualityInput->minimum() == 0.00 &&
            qualityInput->maximum() == 1.00 && qualityInput->decimals() == 2 &&
            qualityInput->singleStep() == 0.01 && qualityInput->value() == 0.50 &&
            !qualityInput->keyboardTracking() &&
            remote::controller::detail::ConfiguredScreenQualityDeficitShareHundredths() == 50;
        output << "SCREEN_QUALITY_DEFICIT_RANGE_AND_DEFAULT=" <<
            (qualityPassed ? "PASS" : "FAIL") << Qt::endl;
        auto* qualityEdit = qualityInput ? qualityInput->findChild<QLineEdit*>() : nullptr;
        bool qualityValidationPassed = qualityEdit && qualityEdit->validator();
        if (qualityValidationPassed) {
            for (const QString& value : {QStringLiteral("0"), QStringLiteral("0.00"), QStringLiteral("0.03"), QStringLiteral("0.23"), QStringLiteral("0.80"), QStringLiteral("0.99"), QStringLiteral("1"), QStringLiteral("1.00")}) {
                QString text = value;
                int position = text.size();
                qualityValidationPassed = qualityValidationPassed &&
                    qualityEdit->validator()->validate(text, position) == QValidator::Acceptable;
            }
            for (const QString& value : {QStringLiteral("-0.01"), QStringLiteral("1.01"), QStringLiteral("1.001"), QStringLiteral("0.201"), QStringLiteral("abc")}) {
                QString text = value;
                int position = text.size();
                qualityValidationPassed = qualityValidationPassed &&
                    qualityEdit->validator()->validate(text, position) != QValidator::Acceptable;
            }
        }
        qualityPassed = qualityPassed && qualityValidationPassed;
        output << "SCREEN_QUALITY_DEFICIT_INPUT_VALIDATION=" <<
            (qualityValidationPassed ? "PASS" : "FAIL") << Qt::endl;
        std::uint32_t lastPublishedQualityShare = 0;
        unsigned publishedQualityShareCount = 0;
        QObject::connect(&page, &remote::controller::SettingsPage::ScreenQualityDeficitShareChanged,
            &page, [&](std::uint32_t value) { lastPublishedQualityShare = value; ++publishedQualityShareCount; });
        for (const auto hundredths : {0u, 19u, 50u, 100u}) {
            if (qualityInput) qualityInput->setValue(hundredths / 100.0);
            qualityPassed = qualityPassed &&
                remote::controller::detail::ConfiguredScreenQualityDeficitShareHundredths() == hundredths;
        }
        qualityPassed = qualityPassed && lastPublishedQualityShare == 100 && publishedQualityShareCount == 4;
        if (qualityEdit) qualityEdit->setText(QStringLiteral("0.23"));
        qualityPassed = qualityPassed && publishedQualityShareCount == 4 &&
            remote::controller::detail::ConfiguredScreenQualityDeficitShareHundredths() == 100;
        if (qualityInput) qualityInput->interpretText();
        qualityPassed = qualityPassed && lastPublishedQualityShare == 23 && publishedQualityShareCount == 5 &&
            remote::controller::detail::ConfiguredScreenQualityDeficitShareHundredths() == 23 &&
            QSettings().value(QStringLiteral("media/screenVideoBitrateBppHundredths")).toUInt() == 50;
        output << "SCREEN_QUALITY_DEFICIT_PERSISTENCE_AND_COMMIT_SIGNAL=" <<
            (qualityPassed ? "PASS" : "FAIL") << Qt::endl;
        {
            remote::controller::SettingsPage qualityReloadPage;
            qualityPassed = qualityPassed && qualityReloadPage.Controls().screenQualityDeficitShareInput->value() == 0.23;
        }
        for (const auto hundredths : {0u, 3u, 100u}) {
            QSettings().setValue(QStringLiteral("media/screenQualityDeficitShareHundredths"), hundredths);
            remote::controller::SettingsPage endpointReloadPage;
            qualityPassed = qualityPassed &&
                remote::controller::detail::ConfiguredScreenQualityDeficitShareHundredths() == hundredths &&
                endpointReloadPage.Controls().screenQualityDeficitShareInput->value() == hundredths / 100.0;
        }
        QSettings().setValue(QStringLiteral("media/screenQualityDeficitShareHundredths"), 101u);
        qualityPassed = qualityPassed && remote::controller::detail::ConfiguredScreenQualityDeficitShareHundredths() == 100;
        if (qualityInput) qualityInput->setValue(0.50);
        qualityPassed = qualityPassed && remote::controller::detail::ConfiguredScreenQualityDeficitShareHundredths() == 50;
        output << "SCREEN_QUALITY_DEFICIT_RELOAD_AND_INVALID_SETTINGS_CLAMP=" <<
            (qualityPassed ? "PASS" : "FAIL") << Qt::endl;
        passed = passed && qualityPassed;
        if (input) input->setValue(0.20);
        auto* traffic = page.Controls().screenVideoTrafficEstimate;
        bool trafficPassed = traffic && traffic->text().contains(QStringLiteral("3.11 MB/s")) &&
            traffic->text().contains(QStringLiteral("示例"));
        remote::SessionDiagnosticsSnapshot trafficDiagnostics;
        remote::PeerConnectionDiagnosticsSnapshot sendingPeer;
        remote::RtpStreamStatsSnapshot screenStream;
        screenStream.kind = "video";
        screenStream.slot = remote::kScreenMainVideoSlot;
        screenStream.sourceWidth = screenStream.configuredOutputWidth = 1920;
        screenStream.sourceHeight = screenStream.configuredOutputHeight = 1080;
        screenStream.configuredMaxFrameRate = 30;
        sendingPeer.stats.rtpStreams.push_back(screenStream);
        trafficDiagnostics.peerConnections.push_back(sendingPeer);
        page.UpdateScreenVideoTrafficEstimate(trafficDiagnostics);
        trafficPassed = trafficPassed && traffic->text().contains(QStringLiteral("1.56 MB/s")) &&
            traffic->text().contains(QStringLiteral("30 FPS")) && !traffic->text().contains(QStringLiteral("示例"));
        if (input) input->setValue(0.30);
        trafficPassed = trafficPassed && traffic->text().contains(QStringLiteral("2.33 MB/s"));
        trafficDiagnostics.peerConnections.front().stats.rtpStreams.push_back(screenStream);
        sendingPeer.stats.rtpStreams.front().configuredMaxFrameRate = 60;
        trafficDiagnostics.peerConnections.push_back(sendingPeer);
        page.UpdateScreenVideoTrafficEstimate(trafficDiagnostics);
        trafficPassed = trafficPassed && traffic->text().contains(QStringLiteral("7.00 MB/s")) &&
            traffic->text().contains(QStringLiteral("2 路"));
        auto& measuredPeer = trafficDiagnostics.peerConnections.front();
        measuredPeer.stats.transport.collected = true;
        measuredPeer.stats.rtpStreams.front().sampleWindowMs = 1000;
        measuredPeer.stats.rtpStreams.front().bitrateBps = 8'800'000;
        page.UpdateScreenVideoTrafficEstimate(trafficDiagnostics);
        trafficPassed = trafficPassed && traffic->text().contains(QStringLiteral("实测 1.10 MB/s")) &&
            traffic->text().contains(QStringLiteral("7.00 MB/s"));
        measuredPeer.stats.rtpStreams.front().bitrateBps = 4'000'000;
        page.UpdateScreenVideoTrafficEstimate(trafficDiagnostics);
        trafficPassed = trafficPassed && traffic->text().contains(QStringLiteral("实测 0.50 MB/s"));
        page.UpdateScreenVideoTrafficEstimate({});
        if (input) input->setValue(0.03);
        trafficPassed = trafficPassed && traffic->text().contains(QStringLiteral("0.47 MB/s")) &&
            traffic->text().contains(QStringLiteral("示例"));
        if (input) input->setValue(0.15);
        trafficPassed = trafficPassed && traffic->text().contains(QStringLiteral("2.33 MB/s"));
        passed = passed && trafficPassed;
        output << "SCREEN_BPP_VIDEO_CAP_AND_MEASURED_TRAFFIC=" <<
            (trafficPassed ? "PASS" : "FAIL") << Qt::endl;
        page.show();
        QCoreApplication::processEvents(QEventLoop::AllEvents);
        const auto screenshot = QDir(QCoreApplication::applicationDirPath()).absoluteFilePath(
            QStringLiteral("../../build/screen-bpp-settings.png"));
        auto* themeInput = page.Controls().themeModeSelector;
        themeInput->setCurrentIndex(themeInput->findData(QStringLiteral("light")));
        QCoreApplication::processEvents(QEventLoop::AllEvents);
        const bool capturedLight = page.grab().save(screenshot);
        themeInput->setCurrentIndex(themeInput->findData(QStringLiteral("dark")));
        QCoreApplication::processEvents(QEventLoop::AllEvents);
        const bool capturedDark = page.grab().save(QDir(QCoreApplication::applicationDirPath()).absoluteFilePath(
            QStringLiteral("../../build/screen-bpp-settings-dark.png")));
        const bool captured = capturedLight && capturedDark;
        passed = passed && captured;
        output << "SCREEN_BPP_SETTINGS_LAYOUT_CAPTURE=" << (captured ? "PASS" : "FAIL") << Qt::endl;
        return passed ? 0 : 1;
    }

    if (application.arguments().contains(QStringLiteral("--content-policy-layout-preview"))) {
        // Render synthetic coefficient diagnostics without connections or settings writes.
        using namespace remote::controller;
        using namespace remote::controller::detail;
        DiagnosticsPage page;
        page.setAttribute(Qt::WA_DontShowOnScreen, true);
        page.resize(1100, 850);
        auto* cards = static_cast<DiagnosticsCardsWidget*>(page.PolicyCardsWidget());
        for (auto* button : page.findChildren<QPushButton*>()) {
            if (button->text() == QStringLiteral("场景优化状态")) button->click();
        }
        remote::RtpStreamStatsSnapshot stream;
        stream.statsId = "outbound-current";
        stream.direction = remote::RtpStreamDirection::kOutbound;
        stream.slot = "screen-main";
        stream.kind = "video";
        stream.codec = "video/H264";
        stream.sampleWindowMs = 1000;
        stream.bitrateBps = 12'000'000;
        stream.bytes = 100;
        stream.frameWidth = 1920;
        stream.frameHeight = 1080;
        stream.encodedFramesPerSecond = 59.8;
        stream.sceneQualitySmoothing.enabled = true;
        stream.sceneQualitySmoothing.observed = true;
        stream.sceneQualitySmoothing.scene = "web_app";
        stream.sceneQualitySmoothing.minimumCoefficient = 0.35;
        stream.sceneQualitySmoothing.maximumCoefficient = 0.65;
        stream.sceneQualitySmoothing.targetCoefficient = 0.50;
        stream.sceneQualitySmoothing.currentCoefficient = 0.50;
        stream.sceneQualitySmoothing.manualCoefficientHundredths = 50;
        stream.sceneQualitySmoothing.status = "已应用场景推荐设置";
        const auto selected = MakeContentPolicySections("peer-a", "916955702", {stream});
        auto inactive = stream;
        inactive.statsId = "outbound-inactive";
        inactive.bitrateBps = 0;
        inactive.sampleWindowMs = 0;
        inactive.encodedFramesPerSecond = inactive.sentFramesPerSecond = 0;
        inactive.frameWidth = 640;
        inactive.frameHeight = 360;
        inactive.bytes = 1'000'000;
        inactive.sceneQualitySmoothing.currentCoefficient = 0.01;
        auto inbound = stream;
        inbound.statsId = "inbound-mirror";
        inbound.direction = remote::RtpStreamDirection::kInbound;
        auto repair = stream;
        repair.statsId = "outbound-rtx";
        repair.codec = "video/rtx";
        repair.bytes = 2'000'000;
        auto audio = stream;
        audio.statsId = "outbound-audio";
        audio.kind = "audio";
        const auto duplicate = MakeContentPolicySections("peer-a", "916955702",
            {inactive, inbound, repair, audio, stream});
        auto noDimensions = stream;
        noDimensions.statsId = "outbound-missing-size";
        noDimensions.frameWidth = noDimensions.frameHeight = 0;
        noDimensions.bytes = 3'000'000;
        noDimensions.sceneQualitySmoothing.currentCoefficient = 0.02;
        auto smallerCounter = stream;
        smallerCounter.statsId = "outbound-other-primary";
        smallerCounter.bytes = 99;
        smallerCounter.encodedFramesPerSecond = 25;
        smallerCounter.sceneQualitySmoothing.currentCoefficient = 0.03;
        const auto qualified = MakeContentPolicySections("peer-a", "916955702",
            {noDimensions, smallerCounter, stream});
        auto secondSlot = stream;
        secondSlot.slot = "camera-main";
        const auto distinctSlots = MakeContentPolicySections("peer-a", "916955702", {stream, secondSlot});
        auto restarted = stream;
        restarted.statsId = "outbound-after-restart";
        const auto stable = MakeContentPolicySections("peer-a", "916955702", {restarted});
        const auto otherPeer = MakeContentPolicySections("peer-b", "916955702", {stream});
        const auto copyText = ContentPolicyCopyText(selected);
        const bool selectionPassed = selected.size() == 1 && duplicate.size() == 1 &&
            ContentPolicyCopyText(duplicate) == copyText && qualified.size() == 1 &&
            ContentPolicyCopyText(qualified) == copyText && distinctSlots.size() == 2 &&
            distinctSlots[0].key != distinctSlots[1].key && stable.size() == 1 &&
            stable[0].key == selected[0].key && otherPeer.size() == 1 &&
            otherPeer[0].key != selected[0].key;
        const bool singleCardPassed = selected.size() == 1 && selected[0].cards.size() == 1 &&
            copyText.contains(QStringLiteral("当前取舍系数：0.50")) &&
            copyText.contains(QStringLiteral("场景推荐系数：0.50")) &&
            copyText.contains(QStringLiteral("场景推荐范围：0.35 ～ 0.65")) &&
            copyText.contains(QStringLiteral("手动设置值：0.50")) &&
            !copyText.contains(QStringLiteral("候选规格")) &&
            !copyText.contains(QStringLiteral("码率预算")) &&
            !copyText.contains(QStringLiteral("GoogCC 网络判断")) &&
            !copyText.contains(QStringLiteral("质量门槛"));
        auto transitioning = stream;
        transitioning.sceneQualitySmoothing.scene = "code_terminal";
        transitioning.sceneQualitySmoothing.minimumCoefficient = 0.15;
        transitioning.sceneQualitySmoothing.maximumCoefficient = 0.35;
        transitioning.sceneQualitySmoothing.targetCoefficient = 0.25;
        transitioning.sceneQualitySmoothing.currentCoefficient = 0.38;
        transitioning.sceneQualitySmoothing.remainingMs = 500;
        transitioning.sceneQualitySmoothing.transitioning = true;
        transitioning.sceneQualitySmoothing.status = "正在调整取舍";
        const auto transitionText = ContentPolicyCopyText(
            MakeContentPolicySections("peer-a", "916955702", {transitioning}));
        const bool transitionPassed = transitionText.contains(QStringLiteral("当前场景：编程与终端")) &&
            transitionText.contains(QStringLiteral("当前取舍系数：0.38")) &&
            transitionText.contains(QStringLiteral("场景推荐系数：0.25")) &&
            transitionText.contains(QStringLiteral("调整剩余时间：0.50 秒"));
        auto waiting = stream;
        waiting.sceneQualitySmoothing.observed = false;
        waiting.sceneQualitySmoothing.scene.clear();
        waiting.sceneQualitySmoothing.status.clear();
        const auto waitingText = ContentPolicyCopyText(
            MakeContentPolicySections("peer-a", "916955702", {waiting}));
        const bool waitingPassed = waitingText.contains(QStringLiteral("当前场景：尚未确认")) &&
            waitingText.contains(QStringLiteral("场景推荐系数：尚未确定")) &&
            waitingText.contains(QStringLiteral("场景推荐范围：尚未确定")) &&
            waitingText.contains(QStringLiteral("等待场景识别，保持当前设置"));
        auto disabled = waiting;
        disabled.sceneQualitySmoothing.enabled = false;
        const bool disabledPassed = MakeContentPolicySections("peer-a", "916955702", {disabled}).empty() &&
            ContentPolicyCopyText({MakeContentPolicySection("disabled", "916955702", disabled)})
                .contains(QStringLiteral("AI 场景优化已关闭，使用手动设置"));
        QTextStream(stdout) << "SCENE_SMOOTHING_PER_PEER_SLOT_SELECTION=" <<
            (selectionPassed ? "PASS" : "FAIL") << Qt::endl;
        QTextStream(stdout) << "SCENE_SMOOTHING_SINGLE_CARD=" <<
            (singleCardPassed ? "PASS" : "FAIL") << Qt::endl;
        QTextStream(stdout) << "SCENE_SMOOTHING_TRANSITION_DIAGNOSTICS=" <<
            (transitionPassed ? "PASS" : "FAIL") << Qt::endl;
        QTextStream(stdout) << "SCENE_SMOOTHING_UNKNOWN_SCENE_DIAGNOSTICS=" <<
            (waitingPassed ? "PASS" : "FAIL") << Qt::endl;
        QTextStream(stdout) << "SCENE_SMOOTHING_DISABLED_DIAGNOSTICS=" <<
            (disabledPassed ? "PASS" : "FAIL") << Qt::endl;
        cards->SetSections({MakeContentPolicySection("preview", "916955702", stream)}, {});
        page.show();
        bool saved = true;
        const auto capture = [&](const QString& name, bool dark) {
            page.setStyleSheet(QString::fromUtf8(kMainStyle) + ui::RemoteCTheme::MainWindowColorOverrides(dark));
            QCoreApplication::processEvents(QEventLoop::AllEvents);
            return page.grab().save(QDir(application.applicationDirPath()).filePath(
                QStringLiteral("../../build/content-policy-%1.png").arg(name)));
        };
        saved &= capture("light", false);
        saved &= capture("dark", true);
        cards->SetSections({MakeContentPolicySection("preview", "916955702", transitioning)}, {});
        saved &= capture("transition", true);
        cards->SetSections({MakeContentPolicySection("preview", "916955702", waiting)}, {});
        saved &= capture("waiting-scene", false);
        cards->SetSections({}, QStringLiteral("暂无场景优化数据\n开启 AI 场景优化并共享屏幕后，可查看每个连接的场景和取舍设置。"));
        saved &= capture("empty", false);
        QTextStream(stdout) << "SCENE_SMOOTHING_LAYOUT_CAPTURE=" << (saved ? "PASS" : "FAIL") << Qt::endl;
        return saved && selectionPassed && singleCardPassed && transitionPassed && waitingPassed && disabledPassed ? 0 : 1;
    }

    if (application.arguments().contains(
            QStringLiteral("--theme-roundtrip-self-test"))) {
        auto engine = std::make_unique<remote::app::InProcessSessionEngine>();
        auto* sessionMedia = engine->MediaAccess();
        remote::controller::ControllerMainWindow window(
            std::move(engine), false, sessionMedia);
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

    // Set only for normal primary-client startup, after all isolated self-tests.
    application.setProperty("remoteCInitializeAutoStartDefault", true);
    application.setProperty("remoteCBackgroundStartup", backgroundStartup);
    remote::controller::detail::InitializeWindowsAutoStartAsync(
        &application, [](bool, QString) {});
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
    coordinatorOptions.startMainWindowInTray = startInTray || backgroundStartup;
    coordinatorOptions.backgroundStartup = backgroundStartup;
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
            auto* sessionMedia = engine->MediaAccess();
            remote::app::RemoteCApplicationCoordinator::MainWindowSession
                session;
            session.window = std::make_unique<
                remote::controller::ControllerMainWindow>(
                    std::move(engine),
                    !configuration.authenticationRequired,
                    sessionMedia);
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
