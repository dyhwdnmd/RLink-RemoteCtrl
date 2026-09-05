// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

ControllerMainWindow::ControllerMainWindow(
    std::unique_ptr<ISessionEngine> engine,
    bool startEngineImmediately,
    QWidget* parent)
    : FramelessMainWindow(parent),
      inputExecutor_(std::make_unique<WindowsInputExecutor>()),
      engine_(std::move(engine))
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
                connectButton_->setEnabled(false);
                connectButton_->setText(QStringLiteral("连接已超时"));
                connectionHint_->setProperty("error", true);
                connectionHint_->setText(QStringLiteral(
                    "连接请求超时，请确认对方在线并使用最新验证码后重试。"));
                connectionHint_->setStyleSheet(
                    QStringLiteral("color:#d14343;"));
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
                    if (refreshMediaDevicesButton_) {
                        refreshMediaDevicesButton_->setEnabled(true);
                    }
                    if (mediaDeviceStatusLabel_) {
                        mediaDeviceStatusLabel_->setText(
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
    if (auto* roomEngine = dynamic_cast<app::InProcessSessionEngine*>(
            engine_.get())) {
        roomEngine->SetRemoteInputSink(inputExecutor_.get());
        fileTransferController_ =
            std::make_unique<app::FileTransferController>(
                [roomEngine](const std::string& peerDeviceId,
                             const FileTransferMessage& message) {
                    return roomEngine->SendRemoteFileMessage(
                        peerDeviceId, message);
                });
        clipboardController_ =
            std::make_unique<app::ClipboardController>(
                [roomEngine](const std::string& peerDeviceId,
                             const std::string& clipboardSessionId,
                             const ClipboardMessage& message) {
                    return roomEngine->SendRemoteClipboardMessage(
                        peerDeviceId, clipboardSessionId, message);
                },
                [roomEngine] {
                    const auto sendKey = [roomEngine](
                            std::uint16_t virtualKey,
                            std::uint16_t scanCode,
                            bool pressed) {
                        RemoteInputEvent input;
                        input.type = RemoteInputMessageType::kKey;
                        input.virtualKey = virtualKey;
                        input.scanCode = scanCode;
                        input.pressed = pressed;
                        return roomEngine->SendRemoteInput(input);
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
                            (void)roomEngine->SendRemoteInput(release);
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
        roomEngine->SetRemoteFileTransferSink(
            fileTransferController_.get());
        roomEngine->SetRemoteClipboardSink(
            clipboardController_.get());
        ApplyClipboardConfigurationFromUi(false);

        const auto scheduleDisplayRefresh = [this, roomEngine] {
            QTimer::singleShot(120, this, [this, roomEngine] {
                if (engine_.get() != roomEngine) {
                    return;
                }
                (void)roomEngine->RefreshLocalDisplays();
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
    const QString currentHardwareFingerprint =
        QString::fromStdString(
            BuildWindowsHardwareFingerprint());
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
        QStringLiteral("native_dxgi")).toString();
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
    // The startup worker owns no Qt objects, but it may still be publishing
    // runtime capabilities into engine_ while the window is being destroyed.
    // Join it before any observer, sink or engine lifetime is changed.
    engineStartThread_.request_stop();
    if (engineStartThread_.joinable()) {
        engineStartThread_.join();
    }
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
    if (engine_) {
        if (auto* roomEngine = dynamic_cast<app::InProcessSessionEngine*>(
                engine_.get())) {
            roomEngine->SetRemoteFileTransferSink(nullptr);
            roomEngine->SetRemoteClipboardSink(nullptr);
        }
    }
    DestroyAuxiliaryWindowsForExit();
    if (clipboardController_) {
        clipboardController_->SetObserver(nullptr);
    }
    fileTransferController_.reset();
    clipboardController_.reset();
    if (engine_) {
        if (auto* roomEngine = dynamic_cast<app::InProcessSessionEngine*>(
                engine_.get())) {
            roomEngine->SetRemoteInputSink(nullptr);
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
    if (deviceLoginStatus_) {
        deviceLoginStatus_->setText(
            message.isEmpty()
                ? QStringLiteral("登录后即可注册本机设备、创建或加入协作房间。")
                : message);
    }
    if (deviceLoginButton_) {
        deviceLoginButton_->setText(QStringLiteral("立即登录   →"));
        deviceLoginButton_->setEnabled(true);
    }
    if (deviceAccountLabel_) {
        deviceAccountLabel_->setText(QStringLiteral("未登录"));
    }
    if (deviceLoginPrompt_) {
        deviceLoginPrompt_->show();
    }
    if (deviceIdentityCard_) {
        deviceIdentityCard_->hide();
    }
    if (deviceDirectAssistCard_) {
        deviceDirectAssistCard_->hide();
    }
    if (deviceStatusCard_) {
        deviceStatusCard_->hide();
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
    if (deviceLoginStatus_) {
        deviceLoginStatus_->setText(message);
    }
    if (deviceLoginButton_) {
        deviceLoginButton_->setText(QStringLiteral("登录进行中"));
        deviceLoginButton_->setEnabled(false);
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
    if (deviceLoginPrompt_) {
        deviceLoginPrompt_->hide();
    }
    if (deviceIdentityCard_) {
        deviceIdentityCard_->show();
    }
    if (deviceDirectAssistCard_) {
        deviceDirectAssistCard_->show();
    }
    if (deviceStatusCard_) {
        deviceStatusCard_->show();
    }
    if (deviceAccountLabel_) {
        deviceAccountLabel_->setText(
            accountDetail.isEmpty() ? accountLabel : accountDetail);
        deviceAccountLabel_->setToolTip(deviceAccountLabel_->text());
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
    if (!sessionEngineStarted_ || !engine_) {
        return;
    }
    inputExecutor_->ReleaseAllRemoteInputs();
    engineStartThread_.request_stop();
    if (engineStartThread_.joinable()) {
        engineStartThread_.join();
    }
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
        if (createRoomButton_) {
            createRoomButton_->setEnabled(false);
        }
        if (joinRoomButton_) {
            joinRoomButton_->setEnabled(false);
        }
        if (connectButton_) {
            connectButton_->setEnabled(false);
        }
        SetRoomActionHint(QStringLiteral(
            "请先登录 RLink 账户，再创建或加入协作房间。"));
    }
}

void ControllerMainWindow::RequestMediaDeviceRefresh(
    bool userInitiated)
{
    mediaDeviceRefreshUserRequested_ |= userInitiated;
    if (mediaDeviceStatusLabel_) {
        mediaDeviceStatusLabel_->setText(
            QStringLiteral("正在刷新本机音视频设备…"));
    }
    if (refreshMediaDevicesButton_) {
        refreshMediaDevicesButton_->setEnabled(false);
    }
    if (mediaDeviceRefreshDebounceTimer_) {
        mediaDeviceRefreshDebounceTimer_->start();
    }
}
