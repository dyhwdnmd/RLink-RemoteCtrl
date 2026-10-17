// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#include "ControllerMainWindow.h"
#include "ControllerMainWindowSupport.h"

#include <QAction>
#include <QAbstractItemModel>
#include <QApplication>
#include <QGuiApplication>
#include <QCloseEvent>
#include <QColor>
#include <QComboBox>
#include <QCryptographicHash>
#include <QCursor>
#include <QEasingCurve>
#include <QEvent>
#include <QGraphicsOpacityEffect>
#include <QHBoxLayout>
#include <QLabel>
#include <QMenu>
#include <QMouseEvent>
#include <QProcess>
#include <QPushButton>
#include <QResizeEvent>
#include <QScreen>
#include <QScrollBar>
#include <QSettings>
#include <QSignalBlocker>
#include <QStackedWidget>
#include <QStyleHints>
#include <QStringList>
#include <QTextStream>
#include <QTimer>
#include <QSystemTrayIcon>
#include <QToolButton>
#include <QVariant>
#include <QWidgetAction>
#include <QWindow>
#include <array>
#include <chrono>
#include <filesystem>
#include <tuple>
#include <utility>
#include <algorithm>
#include "RoundedPopupMenu.h"
#include "CameraWindow.h"
#include "ControlledSessionIndicator.h"
#include "FileTransferWindow.h"
#include "LoginWindow.h"
#include "RemoteSessionWindow.h"
#include "RemoteCComboBox.h"
#include "RemoteCDialog.h"
#include "RemoteCToast.h"
#include "RoomCameraWindow.h"
#include "src/apps/remote/FileTransferController.h"
#include "src/apps/remote/ClipboardController.h"
#include "src/apps/remote/EncoderBenchmarkProfileCache.h"
#include "src/apps/remote/ISessionMediaAccess.h"
#include "src/apps/controller/ui/morph/MorphIconButtonBinding.h"
#include "src/apps/controller/ui/RemoteCTheme.h"
#include "src/platform/win/WindowsHardwareFingerprint.h"
#include "src/platform/win/WindowsInputExecutor.h"
#ifdef Q_OS_WIN
#include <qt_windows.h>
#include <dbt.h>
#endif
#include "pages/DirectConnectPage.h"
#include "pages/OwnedDevicesPage.h"
#include "pages/RecentConnectionsPage.h"
#include "pages/RoomPage.h"
#include "pages/SettingsPage.h"

namespace remote::controller {
using namespace detail;

SettingsPageControls& ControllerMainWindow::SettingsControls()
{
    Q_ASSERT(settingsPage_);
    return settingsPage_->Controls();
}

RoomPageControls& ControllerMainWindow::RoomControls()
{
    Q_ASSERT(roomPage_);
    return roomPage_->Controls();
}

ControllerMainWindow::ControllerMainWindow(
    std::unique_ptr<ISessionEngine> engine,
    bool startEngineImmediately,
    app::ISessionMediaAccess* sessionMedia,
    QWidget* parent)
    : FramelessMainWindow(parent),
      inputExecutor_(std::make_unique<WindowsInputExecutor>()),
      engine_(std::move(engine)),
      sessionMedia_(sessionMedia)
{
    const QSettings settings;
    BuildUi();
    connect(QGuiApplication::styleHints(),
            &QStyleHints::colorSchemeChanged, this,
            [this](Qt::ColorScheme) {
                if (ui::RemoteCTheme::LoadPreference() ==
                    ui::ThemePreference::kSystem) {
                    ApplyInterfaceTheme(false);
                }
            });
    assistedSessionTimeoutTimer_ = new QTimer(this);
    assistedSessionTimeoutTimer_->setSingleShot(true);
    assistedSessionTimeoutTimer_->setInterval(20000);
    connect(assistedSessionTimeoutTimer_, &QTimer::timeout,
            this, [this] {
                if (!engine_ || !assistedSessionPending_) {
                    return;
                }
                const auto snapshot = engine_->Snapshot();
                if (snapshot.state != SessionEngineState::kConnecting) {
                    return;
                }
                assistedSessionTimedOut_ = true;
                assistedSessionCancellationPending_ = true;
                engine_->Disconnect();
                directConnectPage_->SetActionState(
                    false, QStringLiteral("连接已超时"));
                directConnectPage_->SetAssistHint(
                    QStringLiteral(
                        "连接请求超时，请确认对方在线并使用最新验证码后重试。"),
                    DirectConnectPage::AssistHintTone::kError);
            });
    mediaDeviceRefreshDebounceTimer_ = new QTimer(this);
    mediaDeviceRefreshDebounceTimer_->setSingleShot(true);
    mediaDeviceRefreshDebounceTimer_->setInterval(350);
    connect(mediaDeviceRefreshDebounceTimer_, &QTimer::timeout,
            this, [this] {
                const bool userInitiated =
                    std::exchange(
                        mediaDeviceRefreshUserRequested_, false);
                if (!engine_) {
                    return;
                }
                const auto result =
                    engine_->RefreshLocalMediaDevices();
                if (!result.accepted) {
                    if (SettingsControls().refreshMediaDevicesButton) {
                        SettingsControls().refreshMediaDevicesButton->setEnabled(true);
                    }
                    if (SettingsControls().mediaDeviceStatusLabel) {
                        SettingsControls().mediaDeviceStatusLabel->setText(
                            QStringLiteral("音视频设备暂时无法刷新"));
                    }
                    if (userInitiated) {
                        RemoteCToast::Show(
                            this,
                            QStringLiteral("刷新音视频设备失败：%1")
                                .arg(QString::fromStdString(
                                    result.errorMessage)),
                            RemoteCToast::Tone::kError);
                    }
                }
            });
    BuildSystemTray();
    softwareUpdateController_ =
        std::make_unique<update::SoftwareUpdateController>(this);
    softwareUpdateController_->SetStateChangedCallback(
        [this](const update::SoftwareUpdateController::Snapshot& snapshot) {
            HandleSoftwareUpdateState(snapshot);
        });
    softwareUpdateController_->ScheduleAutomaticCheck();
    if (sessionMedia_) {
        sessionMedia_->SetRemoteInputSink(inputExecutor_.get());
        fileTransferController_ =
            std::make_unique<app::FileTransferController>(
                [media = sessionMedia_](const std::string& peerDeviceId,
                             const FileTransferMessage& message) {
                    return media->SendRemoteFileMessage(
                        peerDeviceId, message);
                });
        clipboardController_ =
            std::make_unique<app::ClipboardController>(
                [media = sessionMedia_](const std::string& peerDeviceId,
                             const std::string& clipboardSessionId,
                             const ClipboardMessage& message) {
                    return media->SendRemoteClipboardMessage(
                        peerDeviceId, clipboardSessionId, message);
                },
                [media = sessionMedia_] {
                    const auto sendKey = [media](
                            std::uint16_t virtualKey,
                            std::uint16_t scanCode,
                            bool pressed) {
                        RemoteInputEvent input;
                        input.type = RemoteInputMessageType::kKey;
                        input.virtualKey = virtualKey;
                        input.scanCode = scanCode;
                        input.pressed = pressed;
                        return media->SendRemoteInput(input);
                    };
                    const std::array<
                        std::tuple<std::uint16_t, std::uint16_t, bool>, 4>
                        pasteKeys = {{
                            {VK_CONTROL, 0x1d, true},
                            {static_cast<std::uint16_t>('V'), 0x2f, true},
                            {static_cast<std::uint16_t>('V'), 0x2f, false},
                            {VK_CONTROL, 0x1d, false}}};
                    for (const auto& key : pasteKeys) {
                        const auto result = sendKey(
                            std::get<0>(key), std::get<1>(key),
                            std::get<2>(key));
                        if (!result.accepted) {
                            RemoteInputEvent release;
                            release.type =
                                RemoteInputMessageType::kReleaseAll;
                            (void)media->SendRemoteInput(release);
                            return result;
                        }
                    }
                    return SessionCommandResult{true, {}, {}};
                });
        clipboardController_->SetObserver(this);
        fileTransferWindow_ = new FileTransferWindow(
            fileTransferController_.get(), nullptr);
        fileTransferWindow_->AttachAsDrawer(this);
        // The camera gallery is created on first use. Building this complete
        // top-level window here delayed the first visible login-status frame
        // even though most launches never open the gallery.
        sessionMedia_->SetRemoteFileTransferSink(
            fileTransferController_.get());
        sessionMedia_->SetRemoteClipboardSink(
            clipboardController_.get());
        ApplyClipboardConfigurationFromUi(false);

        const auto scheduleDisplayRefresh = [this] {
            QTimer::singleShot(120, this, [this] {
                if (engine_) {
                    (void)engine_->RefreshLocalDisplays();
                }
            });
        };
        const auto watchScreen =
            [this, scheduleDisplayRefresh](QScreen* screen) {
                if (!screen) {
                    return;
                }
                connect(screen, &QScreen::geometryChanged, this,
                        [scheduleDisplayRefresh](const QRect&) {
                            scheduleDisplayRefresh();
                        });
                connect(screen, &QScreen::logicalDotsPerInchChanged, this,
                        [scheduleDisplayRefresh](qreal) {
                            scheduleDisplayRefresh();
                        });
                connect(screen, &QScreen::orientationChanged, this,
                        [scheduleDisplayRefresh](Qt::ScreenOrientation) {
                            scheduleDisplayRefresh();
                        });
            };
        for (QScreen* screen : QGuiApplication::screens()) {
            watchScreen(screen);
        }
        connect(qApp, &QGuiApplication::screenAdded, this,
                [watchScreen, scheduleDisplayRefresh](QScreen* screen) {
                    watchScreen(screen);
                    scheduleDisplayRefresh();
                });
        connect(qApp, &QGuiApplication::screenRemoved, this,
                [scheduleDisplayRefresh](QScreen*) {
                    scheduleDisplayRefresh();
                });
    }
    qApp->installEventFilter(this);
    if (startEngineImmediately) {
        (void)StartSessionEngine();
    } else {
        ApplyAuthenticationAvailability(false);
    }
    const QSettings decoderProbeSettings;
    const QString currentHardwareFingerprint = HardwareFingerprintForUi();
    const bool decoderProbeCacheValid =
        decoderProbeSettings.value(
            QString::fromLatin1(
                kDecoderBenchmarkCompletedSetting),
            false).toBool() &&
        decoderProbeSettings.value(
            QString::fromLatin1(
                kDecoderBenchmarkPolicyVersionSetting),
            0).toInt() == kDecoderBenchmarkPolicyVersion &&
        decoderProbeSettings.value(
            QString::fromLatin1(
                kDecoderHardwareFingerprintSetting))
            .toString() == currentHardwareFingerprint;
    if (!decoderProbeCacheValid) {
        // The timer starts only after QApplication enters its event loop, so
        // the main window is usable before the one-time isolated probe starts.
        QTimer::singleShot(
            std::chrono::seconds(5), this,
            [this] { StartDecoderBenchmark(false); });
    }
    const QString configuredCaptureBackend = decoderProbeSettings.value(
        QString::fromLatin1(kDesktopCaptureBackendSetting),
        QStringLiteral("libwebrtc")).toString();
    const QString configuredX264Preset = decoderProbeSettings.value(
        QString::fromLatin1(kFfmpegX264PresetSetting),
        QStringLiteral("medium")).toString();
    const bool encoderProbeCacheValid =
        !app::LoadEncoderBenchmarkProfile(
             decoderProbeSettings, currentHardwareFingerprint,
             configuredCaptureBackend, configuredX264Preset,
             kEncoderBenchmarkPolicyVersion)
             .isEmpty();
    if (!encoderProbeCacheValid) {
        QTimer::singleShot(
            std::chrono::seconds(8), this,
            [this] { StartEncoderBenchmark(false); });
    }
}

ControllerMainWindow::~ControllerMainWindow()
{
    if (decoderBenchmarkProcess_) {
        decoderBenchmarkProcess_->kill();
        decoderBenchmarkProcess_->waitForFinished(1000);
    }
    if (encoderBenchmarkProcess_) {
        encoderBenchmarkProcess_->kill();
        encoderBenchmarkProcess_->waitForFinished(1000);
    }
    qApp->removeEventFilter(this);
    if (trayIcon_) {
        trayIcon_->hide();
    }
    if (sessionMedia_) {
        sessionMedia_->SetRemoteFileTransferSink(nullptr);
        sessionMedia_->SetRemoteClipboardSink(nullptr);
    }
    DestroyAuxiliaryWindowsForExit();
    if (clipboardController_) {
        clipboardController_->SetObserver(nullptr);
    }
    fileTransferController_.reset();
    clipboardController_.reset();
    if (engine_) {
        if (sessionMedia_) {
            sessionMedia_->SetRemoteInputSink(nullptr);
        }
        inputExecutor_->ReleaseAllRemoteInputs();
        engine_->SetObserver(nullptr);
        engine_->Stop();
    }
}

void ControllerMainWindow::ActivateFromExternalLaunch()
{
    ShowFromSystemTray();
}

void ControllerMainWindow::SetAccountInteractionCallback(
    std::function<void()> callback)
{
    accountInteractionCallback_ = std::move(callback);
}

void ControllerMainWindow::SetAccountSwitchCallback(
    std::function<void()> callback)
{
    accountSwitchCallback_ = std::move(callback);
}

void ControllerMainWindow::SetAccountDeletionCallback(
    std::function<void()> callback)
{
    accountDeletionCallback_ = std::move(callback);
}

void ControllerMainWindow::SetAccountSignedOut(const QString& message)
{
    HideAccountMenu(false);
    authenticationAvailable_ = false;
    recentHistoryAccountKey_.clear();
    lastRememberedRoomId_.clear();
    lastRememberedDirectSessionId_.clear();
    recentRoomAvailabilityRequested_ = false;
    signOutCallback_ = {};
    if (profileAvatar_) {
        profileAvatar_->setText(QStringLiteral("登"));
    }
    if (profileName_) {
        profileName_->setText(QStringLiteral("登录账户"));
    }
    if (serviceStatus_) {
        serviceStatus_->setText(QStringLiteral("● 点击登录"));
        serviceStatus_->setStyleSheet(QStringLiteral("color:#8fa0b8;"));
    }
    if (directConnectPage_) {
        directConnectPage_->ShowSignedOut(message);
    }
    if (directConnectPage_) {
        directConnectPage_->SetAccountLabel(QString{});
    }
    if (trayIdentityLabel_) {
        trayIdentityLabel_->setText(QStringLiteral(
            "<b>RLink</b><br><span style='color:#8290a6;'>未登录</span>"));
    }
    if (traySignOutAction_) {
        traySignOutAction_->setVisible(false);
    }
    ApplyAuthenticationAvailability(false);
    RefreshRecentRooms();
    RefreshRecentDevices();
}

void ControllerMainWindow::SetAccountBusy(const QString& message)
{
    HideAccountMenu(false);
    authenticationAvailable_ = false;
    if (profileName_) {
        profileName_->setText(QStringLiteral("登录账户"));
    }
    if (serviceStatus_) {
        serviceStatus_->setText(QStringLiteral("● 等待浏览器登录"));
        serviceStatus_->setStyleSheet(QStringLiteral("color:#f1bd62;"));
    }
    if (directConnectPage_) {
        directConnectPage_->ShowLoginBusy(message);
    }
    ApplyAuthenticationAvailability(false);
}

void ControllerMainWindow::SetAccountSession(
    const QString& accountId,
    const QString& accountLabel,
    const QString& accountDetail,
    std::function<void()> signOutCallback)
{
    authenticationAvailable_ = true;
    const QString normalizedAccountId = accountId.trimmed();
    recentHistoryAccountKey_ = normalizedAccountId.isEmpty()
        ? QString()
        : QString::fromLatin1(
              QCryptographicHash::hash(normalizedAccountId.toUtf8(),
                                       QCryptographicHash::Sha256).toHex());
    lastRememberedRoomId_.clear();
    lastRememberedDirectSessionId_.clear();
    recentRoomAvailabilityRequested_ = false;
    MigrateLegacyRecentHistory();
    signOutCallback_ = std::move(signOutCallback);
    if (profileAvatar_) {
        const QString trimmed = accountLabel.trimmed();
        profileAvatar_->setText(trimmed.isEmpty()
                                    ? QStringLiteral("R")
                                    : trimmed.left(1).toUpper());
        if (accountMenuAvatar_) {
            accountMenuAvatar_->setText(profileAvatar_->text());
        }
    }
    if (profileName_) {
        profileName_->setText(accountLabel.isEmpty()
                                  ? QStringLiteral("RLink 用户")
                                  : accountLabel);
        profileName_->setToolTip(accountLabel);
        if (accountMenuName_) {
            accountMenuName_->setText(profileName_->text());
        }
        if (accountMenuDetail_) {
            accountMenuDetail_->setText(accountDetail);
            accountMenuDetail_->setToolTip(accountDetail);
            accountMenuDetail_->setVisible(!accountDetail.trimmed().isEmpty());
        }
    }
    if (serviceStatus_) {
        serviceStatus_->setText(QStringLiteral("● 账户已登录"));
        serviceStatus_->setStyleSheet(
            darkInterfaceTheme_ ? QStringLiteral("color:#4FF0B5;")
                                : QStringLiteral("color:#168A5B;"));
    }
    if (directConnectPage_) {
        directConnectPage_->SetAuthenticated(true);
    }
    if (directConnectPage_) {
        directConnectPage_->SetAccountLabel(
            accountDetail.isEmpty() ? accountLabel : accountDetail);
    }
    if (trayIdentityLabel_) {
        trayIdentityLabel_->setText(
            QStringLiteral(
                "<b>RLink</b><br><span style='color:#8290a6;'>%1</span>")
                .arg((accountDetail.isEmpty() ? accountLabel : accountDetail)
                         .toHtmlEscaped()));
    }
    if (traySignOutAction_) {
        traySignOutAction_->setVisible(
            static_cast<bool>(signOutCallback_));
    }
    ApplyAuthenticationAvailability(true);
    RefreshRecentRooms();
    RefreshRecentDevices();
}

bool ControllerMainWindow::StartSessionEngine()
{
    if (sessionEngineStarted_) {
        return true;
    }
    sessionEngineStarted_ = true;
    if (!InitializeEngine()) {
        // The engine may still have connected signaling to expose the failure,
        // so retain the started state until an explicit stop.
        return false;
    }
    return true;
}

void ControllerMainWindow::StopSessionEngine()
{
    if (controlledSessionWindow_) controlledSessionWindow_->Reset();
    if (!sessionEngineStarted_ || !engine_) {
        return;
    }
    inputExecutor_->ReleaseAllRemoteInputs();
    engine_->Stop();
    sessionEngineStarted_ = false;
}

void ControllerMainWindow::PrepareForApplicationExit()
{
    if (applicationExitPrepared_) {
        return;
    }
    applicationExitPrepared_ = true;
    StopSessionEngine();
}

void ControllerMainWindow::ApplyAuthenticationAvailability(bool authenticated)
{
    authenticationAvailable_ = authenticated;
    if (!authenticated) {
        roomPage_->SetEntryActionsEnabled(false);
        directConnectPage_->SetActionEnabled(false);
        SetRoomActionHint(QStringLiteral(
            "请先登录 RLink 账户，再创建或加入协作房间。"));
    }
}

void ControllerMainWindow::RequestMediaDeviceRefresh(
    bool userInitiated)
{
    mediaDeviceRefreshUserRequested_ |= userInitiated;
    if (SettingsControls().mediaDeviceStatusLabel) {
        SettingsControls().mediaDeviceStatusLabel->setText(
            QStringLiteral("正在刷新本机音视频设备…"));
    }
    if (SettingsControls().refreshMediaDevicesButton) {
        SettingsControls().refreshMediaDevicesButton->setEnabled(false);
    }
    if (mediaDeviceRefreshDebounceTimer_) {
        mediaDeviceRefreshDebounceTimer_->start();
    }
}
void ControllerMainWindow::ApplyInterfaceTheme(bool showFeedback)
{
    // Apply atomically: setStyleSheet() already performs the required Qt
    // repolish. A manual root-only unpolish/polish leaves descendants (most
    // visibly custom combo boxes and category buttons) unpolished after a
    // dark -> light round trip and makes them fall back to native geometry.
    setUpdatesEnabled(false);
    struct ScrollPosition {
        QPointer<QAbstractScrollArea> area;
        int horizontal = 0;
        int vertical = 0;
    };
    QVector<ScrollPosition> scrollPositions;
    const auto scrollAreas = findChildren<QAbstractScrollArea*>();
    scrollPositions.reserve(scrollAreas.size());
    for (auto* area : scrollAreas) {
        scrollPositions.push_back({area,
                                   area->horizontalScrollBar()->value(),
                                   area->verticalScrollBar()->value()});
    }
    const ui::ThemePreference preference =
        ui::RemoteCTheme::LoadPreference();
    const bool dark = ui::RemoteCTheme::IsDark(preference);
    darkInterfaceTheme_ = dark;
    if (controlledSessionWindow_) controlledSessionWindow_->ApplyTheme(dark);

    setProperty("themeRoot", dark ? QStringLiteral("dark")
                                   : QStringLiteral("light"));
    ApplyUiStyleSheet(
        QString::fromUtf8(kMainStyle) +
        ui::RemoteCTheme::MainWindowColorOverrides(dark));

    const auto applyPage = [dark](QWidget* page,
                                  const QString& resourcePath) {
        if (!page) return;
        page->setProperty("themeRoot", dark ? QStringLiteral("dark")
                                             : QStringLiteral("light"));
        page->setStyleSheet(
            ui::RemoteCTheme::PageStyleSheet(resourcePath, dark));
    };
    applyPage(localDevicePage_,
              QStringLiteral(":/ui/theme/local-device.qss"));
    applyPage(ownedDevicesPage_,
              QStringLiteral(":/ui/theme/owned-devices.qss"));
    applyPage(recentConnectionsPage_,
              QStringLiteral(":/ui/theme/recent-connections.qss"));

    if (titleBar_) titleBar_->RefreshThemeStyle(dark);
    const QColor navigationNormal(
        dark ? QStringLiteral("#AEBBD0") : QStringLiteral("#64748B"));
    const QColor navigationActive(
        dark ? QStringLiteral("#8EA5FF") : QStringLiteral("#315EFB"));
    const auto navigationButtons = findChildren<QPushButton*>();
    for (auto* button : navigationButtons) {
        if (!button->property("nav").toBool()) continue;
        const auto icon = static_cast<NavigationIcon>(
            button->property("navigationIcon").toInt());
        button->setIcon(MakeNavigationIcon(icon, dark));
        button->setProperty("remoteCMorphCheckedColor",
                            navigationActive.name());
        const QString source =
            button->property("navigationMorphSource").toString();
        const QString target =
            button->property("navigationMorphTarget").toString();
        if (!source.isEmpty() && !target.isEmpty()) {
            remotec::ui::morph::MorphIconButtonBinding::attach(
                button,
                QStringLiteral(":/ui/icons/lucide/base/%1.svg").arg(source),
                QStringLiteral(":/ui/icons/lucide/base/%1.svg").arg(target),
                remotec::ui::morph::MorphIconButtonBinding::Interaction::Hover,
                QSize(18, 18), navigationNormal, navigationActive);
        }
    }
    const auto selectors = findChildren<QComboBox*>();
    for (auto* selector : selectors) {
        if (auto* remoteSelector =
                dynamic_cast<RemoteCComboBox*>(selector)) {
            remoteSelector->RefreshThemeStyle();
        }
    }
    ui::RemoteCTheme::RefreshIcons(this);
    const auto featureIcons = findChildren<QLabel*>(
        QStringLiteral("localDeviceFeatureIcon"));
    for (auto* icon : featureIcons) {
        const QString resource =
            icon->property("localFeatureIconResource").toString();
        if (resource.isEmpty()) continue;
        const auto tone = resource.contains(QStringLiteral("shield-check"))
            ? ui::ThemeIconTone::kSuccess
            : (dark ? ui::ThemeIconTone::kOnDark
                    : ui::ThemeIconTone::kPrimary);
        ui::RemoteCTheme::SetPixmap(icon, resource, QSize(20, 20), tone);
    }
    if (ownedDevicesPage_) {
        ownedDevicesPage_->RefreshThemeStyle();
    }
    // Benchmark summaries use rich text, so QSS cannot recolor their inline
    // spans. Rebuild them whenever the theme changes.
    RefreshEncoderBenchmarkSummary(false);
    RefreshDecoderBenchmarkSummary(false);
    for (auto* widget : QApplication::topLevelWidgets()) {
        if (auto* login = dynamic_cast<LoginWindow*>(widget)) {
            login->RefreshThemeStyle();
        }
        if (auto* status = dynamic_cast<LoginStatusWindow*>(widget)) {
            status->RefreshThemeStyle();
        }
        if (auto* transfer = dynamic_cast<FileTransferWindow*>(widget)) {
            transfer->RefreshThemeStyle();
        }
        for (auto* descendant : widget->findChildren<QWidget*>()) {
            if (auto* login = dynamic_cast<LoginWindow*>(descendant)) {
                login->RefreshThemeStyle();
            }
            if (auto* status =
                    dynamic_cast<LoginStatusWindow*>(descendant)) {
                status->RefreshThemeStyle();
            }
            if (auto* transfer =
                    dynamic_cast<FileTransferWindow*>(descendant)) {
                transfer->RefreshThemeStyle();
            }
        }
    }

    if (SettingsControls().themeModeSelector) {
        const QSignalBlocker blocker(SettingsControls().themeModeSelector);
        const int index = SettingsControls().themeModeSelector->findData(
            ui::RemoteCTheme::PreferenceValue(preference));
        SettingsControls().themeModeSelector->setCurrentIndex(index >= 0 ? index : 0);
    }

    // QStyleSheetStyle recalculates scroll ranges while selectors are being
    // repolished. Preserve the user's exact viewport so a color switch cannot
    // shift page content by a few pixels.
    for (const auto& position : scrollPositions) {
        if (!position.area) continue;
        position.area->horizontalScrollBar()->setValue(position.horizontal);
        position.area->verticalScrollBar()->setValue(position.vertical);
    }
    QTimer::singleShot(0, this, [this, scrollPositions] {
        for (const auto& position : scrollPositions) {
            if (!position.area) continue;
            position.area->horizontalScrollBar()->setValue(
                position.horizontal);
            position.area->verticalScrollBar()->setValue(position.vertical);
        }
        update();
    });

    // Room-member cards contain inline colors which are not recolored by the
    // root style sheet. Force their render key to refresh in the same frame.
    renderedRoomMemberKey_.clear();
    if (engine_ && RoomControls().memberList) {
        UpdateRoomUi(engine_->Snapshot());
    }

    setUpdatesEnabled(true);
    update();
    if (showFeedback) {
        const QString modeName =
            preference == ui::ThemePreference::kSystem
                ? QStringLiteral("跟随系统")
                : (dark ? QStringLiteral("深色")
                        : QStringLiteral("浅色"));
        RemoteCToast::Show(
            this, QStringLiteral("界面主题已切换为%1").arg(modeName),
            RemoteCToast::Tone::kSuccess);
    }
}

void ControllerMainWindow::SetInterfaceThemePreference(
    const QString& value)
{
    ui::RemoteCTheme::SavePreference(
        ui::RemoteCTheme::PreferenceFromValue(value));
    ApplyInterfaceTheme(true);
}

bool ControllerMainWindow::RunThemeRoundTripSelfTest(QString* errorMessage)
{
    QSettings settings;
    const QString key = QStringLiteral("ui/themeMode");
    const bool hadOriginal = settings.contains(key);
    const QVariant original = settings.value(key);
    const int originalPage = pageStack_ ? pageStack_->currentIndex() : -1;
    const int originalCategory = settingsPage_
        ? settingsPage_->DetailStack()->currentIndex() : -1;
    const auto measureSettingsNavigation = [this](const char* phase) {
        QElapsedTimer elapsed;
        elapsed.start();
        if (pageStack_) pageStack_->setCurrentIndex(5);
        const double switchMs = elapsed.nsecsElapsed() / 1.0e6;
        QCoreApplication::processEvents(QEventLoop::AllEvents);
        const double layoutMs = elapsed.nsecsElapsed() / 1.0e6;
        if (settingsPage_) {
            // Include initial layout/text painting, not just setCurrentIndex.
            const auto rendered = settingsPage_->viewport()->grab();
            (void)rendered;
        }
        QTextStream(stdout) << "SETTINGS_NAV_" << phase << "_MS="
                            << elapsed.nsecsElapsed() / 1.0e6
                            << " SWITCH=" << switchMs
                            << " EVENTS=" << layoutMs - switchMs << '\n';
    };
    measureSettingsNavigation("COLD");
    if (pageStack_ && originalPage >= 0) pageStack_->setCurrentIndex(originalPage);
    QCoreApplication::processEvents(QEventLoop::AllEvents);
    measureSettingsNavigation("WARM");

    const auto settle = [this] {
        QCoreApplication::processEvents(QEventLoop::AllEvents);
        const auto widgets = findChildren<QWidget*>();
        for (auto* widget : widgets) {
            if (widget->layout()) widget->layout()->activate();
        }
        if (layout()) layout()->activate();
        QCoreApplication::processEvents(QEventLoop::AllEvents);
    };
    const auto fingerprint = [this] {
        QStringList result;
        const auto widgets = findChildren<QWidget*>();
        int ordinal = 0;
        for (auto* widget : widgets) {
            if (!widget->isVisibleTo(this)) continue;
            const bool tracked =
                widget->objectName() == QStringLiteral("settingRow") ||
                widget->objectName() == QStringLiteral("customTitleBar") ||
                widget->objectName() == QStringLiteral("sidebar") ||
                widget->objectName() == QStringLiteral("profileCard") ||
                widget->property("nav").toBool() ||
                widget->objectName() == QStringLiteral("settingsCategoryPanel") ||
                widget->objectName() == QStringLiteral("settingsCategoryButton") ||
                widget->objectName() == QStringLiteral("pageTitle") ||
                widget->objectName() == QStringLiteral("pageSubtitle") ||
                widget->objectName() == QStringLiteral("settingsDetailTitle") ||
                dynamic_cast<RemoteCComboBox*>(widget) != nullptr;
            if (!tracked) continue;
            const QRect geometry = widget->geometry();
            const QSize hint = widget->sizeHint();
            const QStringList values = {
                QString::number(ordinal++),
                QString::fromLatin1(widget->metaObject()->className()),
                widget->objectName(),
                QStringLiteral("g=%1,%2,%3,%4")
                    .arg(geometry.x()).arg(geometry.y())
                    .arg(geometry.width()).arg(geometry.height()),
                QStringLiteral("hint=%1,%2")
                    .arg(hint.width()).arg(hint.height()),
                QStringLiteral("min=%1,%2")
                    .arg(widget->minimumWidth()).arg(widget->minimumHeight()),
                QStringLiteral("max=%1,%2")
                    .arg(widget->maximumWidth()).arg(widget->maximumHeight())};
            result.push_back(values.join(QLatin1Char('|')));
        }
        return result;
    };
    const auto apply = [this, &settle](ui::ThemePreference preference) {
        ui::RemoteCTheme::SavePreference(preference);
        ApplyInterfaceTheme(false);
        settle();
    };

    apply(ui::ThemePreference::kLight);
    const QStringList lightBefore = fingerprint();
    apply(ui::ThemePreference::kDark);
    const QStringList dark = fingerprint();
    apply(ui::ThemePreference::kLight);
    const QStringList lightAfter = fingerprint();

    bool passed = !lightBefore.isEmpty() &&
                        lightBefore == dark &&
                        lightBefore == lightAfter;
    if (!passed && errorMessage) {
        const int count = std::max({lightBefore.size(), dark.size(),
                                    lightAfter.size()});
        for (int index = 0; index < count; ++index) {
            const QString before = index < lightBefore.size()
                ? lightBefore[index] : QStringLiteral("<missing>");
            const QString during = index < dark.size()
                ? dark[index] : QStringLiteral("<missing>");
            const QString after = index < lightAfter.size()
                ? lightAfter[index] : QStringLiteral("<missing>");
            if (before != during || before != after) {
                *errorMessage = QStringLiteral(
                    "theme geometry mismatch at %1\nLIGHT_1=%2\nDARK=%3\nLIGHT_2=%4")
                    .arg(index).arg(before, during, after);
                break;
            }
        }
        if (errorMessage->isEmpty()) {
            *errorMessage = QStringLiteral("theme geometry fingerprint is empty");
        }
    }

    // Exercise every real settings category, including long codec summaries,
    // without triggering device enumeration or clipboard configuration actions.
    if (settingsPage_) {
        auto* stack = settingsPage_->DetailStack();
        for (int category = 0; passed && category < stack->count(); ++category) {
            stack->setCurrentIndex(category);
            settingsPage_->verticalScrollBar()->setValue(0);
            apply(ui::ThemePreference::kLight);
            const QStringList before = fingerprint();
            apply(ui::ThemePreference::kDark);
            const QStringList during = fingerprint();
            apply(ui::ThemePreference::kLight);
            const QStringList after = fingerprint();
            bool fits = true;
            for (auto* row : stack->currentWidget()->findChildren<QWidget*>(
                     QStringLiteral("settingRow"))) {
                if (!row->isVisibleTo(stack)) continue;
                // File-transfer/paste categories already own an inner scroll
                // area. Their rows must fit that content, not the outer viewport.
                QScrollArea* scroll = settingsPage_;
                for (auto* parent = row->parentWidget(); parent; parent = parent->parentWidget()) {
                    if (auto* owner = qobject_cast<QScrollArea*>(parent)) {
                        scroll = owner;
                        break;
                    }
                }
                const QPoint top = row->mapTo(scroll->widget(), QPoint());
                fits = fits && (row->minimumHeight() != row->maximumHeight() ||
                                row->height() == row->minimumHeight()) &&
                       top.y() + row->height() <= scroll->widget()->height() &&
                       top.y() + row->height() <= scroll->verticalScrollBar()->maximum() +
                                                     scroll->viewport()->height();
                scroll->verticalScrollBar()->setValue(scroll->verticalScrollBar()->maximum());
            }
            settingsPage_->verticalScrollBar()->setValue(
                settingsPage_->verticalScrollBar()->maximum());
            QCoreApplication::processEvents(QEventLoop::AllEvents);
            const auto rendered = settingsPage_->viewport()->grab();
            fits = fits && !rendered.isNull();
            passed = fits && !before.isEmpty() &&
                     before == during && before == after;
            QTextStream(stdout) << "SETTINGS_CATEGORY_" << category
                                << "_LAYOUT=" << (passed ? "PASS" : "FAIL") << '\n';
            if (!passed && errorMessage) {
                *errorMessage = QStringLiteral(
                    "settings category %1 theme geometry or scroll extent mismatch (fits=%2)")
                    .arg(category).arg(fits);
                for (int index = 0; index < before.size(); ++index) {
                    const auto d = during.value(index, QStringLiteral("<missing>"));
                    const auto a = after.value(index, QStringLiteral("<missing>"));
                    if (before[index] != d || before[index] != a) {
                        *errorMessage += QStringLiteral("\nLIGHT_1=%1\nDARK=%2\nLIGHT_2=%3")
                            .arg(before[index], d, a);
                        break;
                    }
                }
            }
        }
        if (originalCategory >= 0) stack->setCurrentIndex(originalCategory);

        auto* capacity = settingsPage_->Controls().clipboardCacheCapacitySelector;
        if (capacity) {
            const auto optionFingerprint = [capacity] {
                QStringList result;
                for (int index = 0; index < capacity->count(); ++index) {
                    result.push_back(capacity->itemText(index) + QLatin1Char('|') +
                                     capacity->itemData(index).toString());
                }
                return result;
            };
            settingsPage_->RefreshClipboardCacheCapacityOptions();
            const auto before = optionFingerprint();
            const QVariant selectedBefore = capacity->currentData();
            const bool enabledBefore = capacity->isEnabled();
            int resets = 0;
            const auto connection = connect(capacity->model(),
                &QAbstractItemModel::modelReset, this, [&resets] { ++resets; });
            settingsPage_->RefreshClipboardCacheCapacityOptions();
            disconnect(connection);
            // A real disk-capacity change is allowed to rebuild the options.
            const bool unchanged = before == optionFingerprint();
            const bool capacityPassed = !unchanged ||
                (resets == 0 && selectedBefore == capacity->currentData() &&
                 enabledBefore == capacity->isEnabled());
            passed = passed && capacityPassed;
            QTextStream(stdout) << "SETTINGS_CAPACITY_REFRESH="
                                << (capacityPassed ? "PASS" : "FAIL") << '\n';
            if (!capacityPassed && errorMessage) {
                *errorMessage = QStringLiteral("unchanged clipboard capacity options rebuilt");
            }
        }
    }

    // Feedback remains an embedded page, with theme-aware buttons and only
    // explicit browser navigation (the probe does not open external URLs).
    bool feedbackPassed = pageStack_ && pageStack_->count() > 6 &&
        helpNavButton_ && helpNavButton_->text() == QStringLiteral("问题反馈");
    if (feedbackPassed) {
        pageStack_->setCurrentIndex(6);
        auto* feedbackPage = pageStack_->currentWidget();
        const auto* title = feedbackPage->findChild<QLabel*>(
            QStringLiteral("pageTitle"));
        feedbackPassed = title && title->text() == QStringLiteral("问题反馈");
        for (const auto preference : {ui::ThemePreference::kLight,
                                      ui::ThemePreference::kDark}) {
            apply(preference);
            int matchingButtons = 0;
            for (auto* button : feedbackPage->findChildren<QPushButton*>()) {
                const bool correctTarget =
                    (button->text() == QStringLiteral("提交问题") &&
                     button->toolTip() == QStringLiteral(
                         "https://github.com/dyhwdnmd/RLink-RemoteCtrl/issues/new/choose")) ||
                    (button->text() == QStringLiteral("查看已有问题") &&
                     button->toolTip() == QStringLiteral(
                         "https://github.com/dyhwdnmd/RLink-RemoteCtrl/issues"));
                if (correctTarget && button->isVisibleTo(this) &&
                    button->isEnabled() && button->height() == 40) ++matchingButtons;
            }
            feedbackPassed = feedbackPassed && matchingButtons == 2 &&
                !feedbackPage->grab().isNull();
        }
    }
    passed = passed && feedbackPassed;
    QTextStream(stdout) << "FEEDBACK_PAGE_LIGHT_DARK="
                        << (feedbackPassed ? "PASS" : "FAIL") << '\n';
    if (!feedbackPassed && errorMessage) {
        *errorMessage = QStringLiteral("feedback navigation or issue buttons mismatch");
    }

    if (hadOriginal) settings.setValue(key, original);
    else settings.remove(key);
    settings.sync();
    ui::RemoteCTheme::ReloadPreference();
    ApplyInterfaceTheme(false);
    if (pageStack_ && originalPage >= 0) {
        pageStack_->setCurrentIndex(originalPage);
    }
    settle();
    return passed;
}
void ControllerMainWindow::ShowMediaDeviceMenu(
    MediaDeviceKind kind, QWidget* anchor)
{
    if (!engine_ || !anchor) {
        return;
    }
    RoundedPopupMenu menu(this);
    menu.SetToggleAnchor(anchor);
    menu.setObjectName(QStringLiteral("mediaDeviceMenu"));
    menu.setAttribute(Qt::WA_TranslucentBackground, false);
    menu.setWindowFlag(Qt::NoDropShadowWindowHint, true);
    menu.setStyleSheet(QStringLiteral(R"(
QMenu#mediaDeviceMenu {
    background: #ffffff;
    border: 1px solid #dce2eb;
    border-radius: 12px;
    padding: 7px;
    color: #263248;
    font-size: 13px;
}
QMenu#mediaDeviceMenu::item {
    min-width: 210px;
    min-height: 22px;
    border-radius: 8px;
    padding: 8px 36px 8px 14px;
}
QMenu#mediaDeviceMenu::item:selected {
    background: #f3f6fb;
    color: #1769e8;
}
QMenu#mediaDeviceMenu::item:checked {
    background: #e9f1ff;
    color: #1769e8;
    font-weight: 700;
}
QMenu#mediaDeviceMenu::item:disabled {
    color: #9aa3b2;
}
QMenu#mediaDeviceMenu::separator {
    height: 1px;
    background: #edf0f4;
    margin: 6px 8px;
}
QPushButton#mediaDeviceRefreshButton {
    background: transparent;
    border: none;
    border-radius: 8px;
    color: #263248;
    min-height: 38px;
    padding: 0 14px;
    text-align: left;
}
QPushButton#mediaDeviceRefreshButton:hover {
    background: #f3f6fb;
    color: #1769e8;
}
QPushButton#mediaDeviceRefreshButton:disabled {
    color: #9aa3b2;
}
)"));
    if (darkInterfaceTheme_) {
        menu.setStyleSheet(menu.styleSheet() + QStringLiteral(R"(
QMenu#mediaDeviceMenu {
    background:#151F2E; border-color:#34445B; color:#E4EBF5;
}
QMenu#mediaDeviceMenu::item:selected {
    background:#202D40; color:#9BB4FF;
}
QMenu#mediaDeviceMenu::item:checked {
    background:#263A61; color:#9BB4FF;
}
QMenu#mediaDeviceMenu::item:disabled { color:#69788E; }
QMenu#mediaDeviceMenu::separator { background:#2A394E; }
QPushButton#mediaDeviceRefreshButton { color:#E4EBF5; }
QPushButton#mediaDeviceRefreshButton:hover {
    background:#202D40; color:#9BB4FF;
}
QPushButton#mediaDeviceRefreshButton:disabled { color:#69788E; }
)"));
    }

    const auto categoryForKind =
        [kind](const MediaDeviceSnapshot& media)
            -> const MediaDeviceCategorySnapshot& {
            switch (kind) {
            case MediaDeviceKind::kCamera:
                return media.camera;
            case MediaDeviceKind::kMicrophone:
                return media.microphone;
            case MediaDeviceKind::kSpeaker:
                return media.speaker;
            }
            return media.camera;
        };
    const auto positionMenu = [&menu, anchor] {
        menu.ensurePolished();
        menu.adjustSize();
        const QSize popupSize = menu.sizeHint();
        menu.resize(popupSize);
        const QPoint anchorTop =
            anchor->mapToGlobal(QPoint(0, 0));
        QPoint popupPosition(
            anchorTop.x() +
                (anchor->width() - popupSize.width()) / 2,
            anchorTop.y() - popupSize.height() - 6);
        QScreen* screen =
            QGuiApplication::screenAt(anchorTop);
        if (!screen) {
            screen = QApplication::primaryScreen();
        }
        if (screen) {
            const QRect available =
                screen->availableGeometry();
            popupPosition.setX(std::clamp(
                popupPosition.x(),
                available.left(),
                (std::max)(
                    available.left(),
                    available.right() -
                        popupSize.width() + 1)));
            popupPosition.setY((std::max)(
                available.top(), popupPosition.y()));
        }
        menu.move(popupPosition);
        return popupPosition;
    };

    std::uint64_t refreshRevision = 0;
    QString renderedDeviceSignature;
    QPointer<QPushButton> activeRefreshButton;
    QTimer refreshPoll(&menu);
    refreshPoll.setInterval(80);
    const auto deviceSignature =
        [](const MediaDeviceCategorySnapshot& category) {
            QStringList parts;
            parts << QString::fromStdString(
                         category.preferredDeviceId)
                  << QString::fromStdString(
                         category.activeDeviceId)
                  << QString::fromStdString(
                         category.activeDeviceName)
                  << QString::number(
                         static_cast<int>(category.state));
            for (const auto& device : category.devices) {
                parts << QString::fromStdString(device.id)
                      << QString::fromStdString(device.name)
                      << QString::number(device.available ? 1 : 0);
            }
            return parts.join(QChar(0x001f));
        };
    std::function<void()> rebuildMenu;
    rebuildMenu = [this, &menu, &refreshPoll,
                   &refreshRevision,
                   &renderedDeviceSignature,
                   &activeRefreshButton,
                   &positionMenu, kind,
                   categoryForKind,
                   deviceSignature] {
        menu.setUpdatesEnabled(false);
        menu.clear();
        const auto media =
            engine_->Snapshot().media.localMediaDevices;
        const auto& category = categoryForKind(media);
        renderedDeviceSignature =
            deviceSignature(category);
        const std::string preferredId =
            category.preferredDeviceId.empty()
                ? std::string(
                      kSystemDefaultMediaDeviceId)
                : category.preferredDeviceId;
        const auto addDeviceAction =
            [this, &menu, kind, &preferredId](
                const QString& name,
                const std::string& deviceId,
                bool available) {
                auto* action = menu.addAction(name);
                action->setCheckable(true);
                action->setChecked(
                    deviceId == preferredId);
                action->setEnabled(available);
                connect(
                    action, &QAction::triggered, this,
                    [this, kind, deviceId] {
                        BeginMediaDeviceSelection(
                            kind,
                            QString::fromStdString(
                                deviceId));
                    });
            };
        addDeviceAction(
            category.activeDeviceName.empty()
                ? QStringLiteral("跟随系统默认")
                : QStringLiteral("跟随系统默认（当前：%1）")
                      .arg(QString::fromStdString(
                          category.activeDeviceName)),
            kSystemDefaultMediaDeviceId,
            !category.devices.empty());
        for (const auto& device : category.devices) {
            addDeviceAction(
                QString::fromStdString(device.name),
                device.id, device.available);
        }
        if (category.devices.empty()) {
            menu.addSeparator();
            auto* unavailable = menu.addAction(
                QStringLiteral("没有检测到可用设备"));
            unavailable->setEnabled(false);
        }
        menu.addSeparator();
        auto* refreshAction =
            new QWidgetAction(&menu);
        auto* refreshButton =
            new QPushButton(
                media.refreshing
                    ? QStringLiteral("正在刷新设备…")
                    : QStringLiteral("刷新设备列表"),
                &menu);
        refreshButton->setObjectName(
            QStringLiteral("mediaDeviceRefreshButton"));
        refreshButton->setCursor(
            Qt::PointingHandCursor);
        refreshButton->setFocusPolicy(Qt::NoFocus);
        refreshButton->setEnabled(!media.refreshing);
        activeRefreshButton = refreshButton;
        refreshAction->setDefaultWidget(refreshButton);
        menu.addAction(refreshAction);
        connect(
            refreshButton, &QPushButton::clicked,
            &menu,
            [this, &menu, refreshButton, &refreshPoll,
             &refreshRevision] {
                menu.setActiveAction(nullptr);
                refreshButton->clearFocus();
                refreshRevision =
                    engine_->Snapshot()
                        .media.localMediaDevices.revision;
                refreshButton->setText(
                    QStringLiteral("正在刷新设备…"));
                refreshButton->setEnabled(false);
                const auto result =
                    engine_->RefreshLocalMediaDevices();
                if (!result.accepted) {
                    refreshButton->setText(
                        QStringLiteral("刷新设备列表"));
                    refreshButton->setEnabled(true);
                    RemoteCToast::Show(
                        this,
                        QStringLiteral(
                            "刷新音视频设备失败：%1")
                            .arg(QString::fromStdString(
                                result.errorMessage)),
                        RemoteCToast::Tone::kError);
                    return;
                }
                refreshPoll.start();
            });
        positionMenu();
        menu.setActiveAction(nullptr);
        menu.setUpdatesEnabled(true);
        menu.update();
    };
    connect(
        &refreshPoll, &QTimer::timeout, &menu,
        [this, &menu, &refreshPoll, &refreshRevision,
         &rebuildMenu, &renderedDeviceSignature,
         &activeRefreshButton, categoryForKind,
         deviceSignature] {
            const auto media =
                engine_->Snapshot().media.localMediaDevices;
            if (!media.refreshing &&
                media.revision != refreshRevision) {
                refreshPoll.stop();
                const QString newSignature =
                    deviceSignature(
                        categoryForKind(media));
                if (newSignature ==
                        renderedDeviceSignature &&
                    activeRefreshButton) {
                    activeRefreshButton->setText(
                        QStringLiteral("刷新设备列表"));
                    activeRefreshButton->setEnabled(true);
                    activeRefreshButton->clearFocus();
                    menu.setActiveAction(nullptr);
                    activeRefreshButton->update();
                } else {
                    rebuildMenu();
                }
            }
        });
    rebuildMenu();
    const QPoint popupPosition = positionMenu();
    menu.exec(popupPosition);
}

void ControllerMainWindow::BeginMediaDeviceSelection(
    MediaDeviceKind kind, const QString& deviceId)
{
    if (!engine_ || deviceId.isEmpty()) {
        return;
    }
    const auto snapshot = engine_->Snapshot();
    const MediaDeviceCategorySnapshot* category = nullptr;
    QString* pendingId = nullptr;
    QComboBox* selector = nullptr;
    QString label;
    SessionCommandResult (ISessionEngine::*operation)(
        const std::string&) = nullptr;
    switch (kind) {
    case MediaDeviceKind::kCamera:
        category = &snapshot.media.localMediaDevices.camera;
        pendingId = &pendingCameraDeviceId_;
        selector = SettingsControls().cameraDeviceSelector;
        label = QStringLiteral("摄像头");
        operation = &ISessionEngine::SelectLocalCameraDevice;
        break;
    case MediaDeviceKind::kMicrophone:
        category = &snapshot.media.localMediaDevices.microphone;
        pendingId = &pendingMicrophoneDeviceId_;
        selector = SettingsControls().microphoneDeviceSelector;
        label = QStringLiteral("麦克风");
        operation = &ISessionEngine::SelectLocalMicrophoneDevice;
        break;
    case MediaDeviceKind::kSpeaker:
        category = &snapshot.media.localMediaDevices.speaker;
        pendingId = &pendingSpeakerDeviceId_;
        selector = SettingsControls().speakerDeviceSelector;
        label = QStringLiteral("扬声器");
        operation = &ISessionEngine::SelectLocalSpeakerDevice;
        break;
    }
    if (!category || !pendingId || !operation) {
        return;
    }
    if (category->state == MediaDeviceSelectionState::kReady &&
        QString::fromStdString(category->preferredDeviceId) ==
            deviceId) {
        RemoteCToast::ShowAbove(
            selector ? static_cast<QWidget*>(selector) : this,
            QStringLiteral("当前已经在使用该%1").arg(label),
            RemoteCToast::Tone::kInformation);
        return;
    }

    *pendingId = deviceId;
    if (selector) {
        selector->setEnabled(false);
    }
    if (SettingsControls().mediaDeviceStatusLabel) {
        SettingsControls().mediaDeviceStatusLabel->setText(
            QStringLiteral("正在切换%1，请稍候…").arg(label));
    }
    const auto result =
        (engine_.get()->*operation)(deviceId.toStdString());
    if (!result.accepted) {
        pendingId->clear();
        mediaDeviceRevision_ = 0;
        RemoteCToast::Show(
            this,
            QStringLiteral("%1切换失败：%2")
                .arg(label,
                     QString::fromStdString(result.errorMessage)),
            RemoteCToast::Tone::kError);
        UpdateLocalMediaDevicesUi(engine_->Snapshot());
    }
}
bool ControllerMainWindow::eventFilter(QObject* watched, QEvent* event)
{
    if (profileUpdateButton_ &&
        (watched == profileUpdateButton_ ||
         profileUpdateButton_->isAncestorOf(
             qobject_cast<QWidget*>(watched)))) {
        return FramelessMainWindow::eventFilter(watched, event);
    }
    if (event) {
        auto* accountAction = qobject_cast<QPushButton*>(watched);
        if (accountAction &&
            accountAction->objectName() == QStringLiteral("accountMenuAction") &&
            (event->type() == QEvent::Enter ||
             event->type() == QEvent::HoverEnter ||
             event->type() == QEvent::Leave ||
             event->type() == QEvent::HoverLeave)) {
            const bool hovered = event->type() == QEvent::Enter ||
                event->type() == QEvent::HoverEnter;
            if (accountAction->property("accountHover").toBool() != hovered) {
                accountAction->setProperty("accountHover", hovered);
                accountAction->style()->unpolish(accountAction);
                accountAction->style()->polish(accountAction);
                accountAction->update();
            }
        }
    }
    if (event && event->type() == QEvent::MouseButtonRelease &&
        profileCard_) {
        const auto* mouseEvent = static_cast<QMouseEvent*>(event);
        const QPoint globalPosition =
            mouseEvent->globalPosition().toPoint();
        const QRect profileGeometry(
            profileCard_->mapToGlobal(QPoint(0, 0)),
            profileCard_->size());
        if (mouseEvent->button() == Qt::LeftButton &&
            profileGeometry.contains(globalPosition)) {
            if (authenticationAvailable_) {
                ToggleAccountMenu();
            } else if (accountInteractionCallback_) {
                accountInteractionCallback_();
            }
            return true;
        }
    }
    if (event && event->type() == QEvent::MouseButtonPress &&
        accountMenu_ && accountMenu_->isVisible()) {
        const auto* mouseEvent = static_cast<QMouseEvent*>(event);
        const QPoint globalPosition =
            mouseEvent->globalPosition().toPoint();
        const QRect menuGeometry(
            accountMenu_->mapToGlobal(QPoint(0, 0)), accountMenu_->size());
        const QRect profileGeometry(
            profileCard_->mapToGlobal(QPoint(0, 0)), profileCard_->size());
        if (!menuGeometry.contains(globalPosition) &&
            !profileGeometry.contains(globalPosition)) {
            HideAccountMenu();
        }
    }
    if (event && event->type() == QEvent::MouseButtonPress &&
        fileTransferWindow_ && fileTransferWindow_->isVisible() &&
        !fileTransferWindow_->IsHiding()) {
        auto* clickedWidget = qobject_cast<QWidget*>(watched);
        const auto* mouseEvent = static_cast<QMouseEvent*>(event);
        const QPoint globalPosition =
            mouseEvent->globalPosition().toPoint();
        const QRect drawerGeometry(
            fileTransferWindow_->mapToGlobal(QPoint(0, 0)),
            fileTransferWindow_->size());
        const bool insideDrawer =
            drawerGeometry.contains(globalPosition);
        const bool onDrawerToggle = fileTransferNavButton_ &&
            QRect(fileTransferNavButton_->mapToGlobal(QPoint(0, 0)),
                  fileTransferNavButton_->size())
                .contains(globalPosition);
        const bool insideMainWindow =
            clickedWidget &&
            (clickedWidget == this || isAncestorOf(clickedWidget));
        const bool insidePopup =
            clickedWidget && clickedWidget->window() &&
            clickedWidget->window()->windowFlags().testFlag(Qt::Popup);
        if (insideMainWindow && !insideDrawer && !onDrawerToggle &&
            !insidePopup) {
            fileTransferWindow_->HideWithAnimation();
        }
    }
    return FramelessMainWindow::eventFilter(watched, event);
}

bool ControllerMainWindow::nativeEvent(
    const QByteArray& eventType,
    void* message,
    qintptr* result)
{
#ifdef Q_OS_WIN
    const auto* nativeMessage = static_cast<MSG*>(message);
    if (nativeMessage &&
        nativeMessage->message == WM_DEVICECHANGE &&
        (nativeMessage->wParam == DBT_DEVICEARRIVAL ||
         nativeMessage->wParam == DBT_DEVICEREMOVECOMPLETE ||
         nativeMessage->wParam == DBT_DEVNODES_CHANGED ||
         nativeMessage->wParam == DBT_CONFIGCHANGED)) {
        RequestMediaDeviceRefresh(false);
    }
#endif
    return FramelessMainWindow::nativeEvent(
        eventType, message, result);
}

void ControllerMainWindow::closeEvent(QCloseEvent* event)
{
    if (quitting_ || !trayIcon_ || !trayIcon_->isVisible()) {
        FramelessMainWindow::closeEvent(event);
        return;
    }
    const QString closeBehavior = QSettings().value(
        QString::fromLatin1(kCloseButtonBehaviorSetting),
        QStringLiteral("tray")).toString();
    if (closeBehavior == QStringLiteral("exit")) {
        event->ignore();
        QTimer::singleShot(0, this,
                           &ControllerMainWindow::QuitFromSystemTray);
        return;
    }
    if (fileTransferWindow_) {
        fileTransferWindow_->HideImmediately();
    }
    if (roomCameraWindow_) {
        roomCameraWindow_->hide();
    }
    if (cameraWindow_) {
        cameraWindow_->hide();
    }
    if (remoteSessionWindow_) {
        remoteSessionWindow_->hide();
    }
    hide();
    event->ignore();
}

void ControllerMainWindow::changeEvent(QEvent* event)
{
    FramelessMainWindow::changeEvent(event);
    if (!event || event->type() != QEvent::WindowStateChange ||
        !fileTransferWindow_) {
        return;
    }
    if (isMinimized()) {
        fileTransferWindow_->HideImmediately();
    }
}

void ControllerMainWindow::resizeEvent(QResizeEvent* event)
{
    FramelessMainWindow::resizeEvent(event);
    UpdateAccountMenuGeometry();
    if (fileTransferWindow_) {
        fileTransferWindow_->UpdateDrawerGeometry();
    }
}

}  // namespace remote::controller
