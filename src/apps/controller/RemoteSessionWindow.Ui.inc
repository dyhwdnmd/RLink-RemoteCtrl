// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

    void RemoteSessionWindow::BuildUi()
    {
        setWindowTitle(QStringLiteral("%1 - RLink 远程会话")
                           .arg(binding_.peerDeviceName));
        setMinimumSize(720, 480);
        resize(1280, 780);
        ApplyUiStyleSheet(QString::fromUtf8(kSessionStyle));

        auto* rootWidget = new QWidget(this);
        rootWidget->setObjectName(QStringLiteral("sessionRoot"));
        auto* root = new QVBoxLayout(rootWidget);
        root->setContentsMargins(0, 0, 0, 0);
        root->setSpacing(0);

        sessionTitleBar_ = new CustomTitleBar(
            this, QStringLiteral("%1 · 远程会话")
                      .arg(binding_.peerDeviceName), rootWidget);
        root->addWidget(sessionTitleBar_);

        contentHost_ = new QWidget(rootWidget);
        contentHost_->setObjectName(QStringLiteral("sessionContentHost"));
        contentHost_->setMouseTracking(true);
        contentHost_->installEventFilter(this);
        root->addWidget(contentHost_, 1);

        const std::weak_ptr<RemoteInputDispatcher>
            inputDispatchState = remoteInputDispatcher_;
        desktopCanvas_ = new RemoteDesktopCanvas(
            [inputDispatchState](const RemoteInputEvent& event) {
                const auto state = inputDispatchState.lock();
                return state && state->Send(event);
            },
            [this](const QStringList& localFiles, bool keyboardPaste) {
                return remotePasteHandler_ &&
                    remotePasteHandler_(localFiles, keyboardPaste);
            },
            [this](std::uint64_t generation) {
                QMetaObject::invokeMethod(
                    this,
                    [this, generation] {
                        HandleFirstScreenPresentation(generation);
                    },
                    Qt::QueuedConnection);
            },
            binding_.peerDeviceId.toStdString(),
            contentHost_);
        static_cast<RemoteDesktopCanvas*>(desktopCanvas_)
            ->SetTargetFrameRate(selectedFrameRate_);
        static_cast<RemoteDesktopCanvas*>(desktopCanvas_)
            ->SetDragPointerSampleRate(
                dragPointerSampleRateHz_);
        // Every monitoring window starts in fit mode. The 100% mode is a
        // temporary inspection tool rather than a persisted session choice.
        static_cast<RemoteDesktopCanvas*>(desktopCanvas_)
            ->SetActualPixelDisplayMode(false);
        desktopCanvas_->installEventFilter(this);

        auto* toolbar = new QFrame(contentHost_);
        toolbar->setObjectName(QStringLiteral("sessionToolbar"));
        toolbar->setFixedHeight(94);
        toolbar->setMouseTracking(true);
        toolbar->installEventFilter(this);
        sessionToolbar_ = toolbar;
        auto* toolbarLayout = new QGridLayout(toolbar);
        toolbarLayout->setContentsMargins(4, 7, 4, 7);
        toolbarLayout->setHorizontalSpacing(6);
        toolbarLayout->setVerticalSpacing(0);
        toolbarLayout->setColumnStretch(0, 1);
        toolbarLayout->setColumnStretch(2, 1);

        auto* leftZone = new QWidget(toolbar);
        leftZone->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Fixed);
        auto* leftLayout = new QHBoxLayout(leftZone);
        leftLayout->setContentsMargins(8, 0, 12, 0);
        leftLayout->setSpacing(10);
        leftLayout->setAlignment(Qt::AlignCenter);
        auto* rightZone = new QWidget(toolbar);
        rightZone->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Fixed);
        auto* rightLayout = new QHBoxLayout(rightZone);
        rightLayout->setContentsMargins(0, 0, 0, 0);
        rightLayout->setSpacing(5);
        rightLayout->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);

        const auto addSeparator = [](QHBoxLayout* layout, QWidget* parent) {
            auto* separator = new QFrame(parent);
            separator->setObjectName(QStringLiteral("toolbarSeparator"));
            separator->setFixedHeight(40);
            layout->addWidget(separator);
            };
        const auto makeMetric =
            [](QWidget* parent, const QString& icon, const QString& title,
                const QString& initialValue, QLabel** valueOut,
                int width, bool good = false) {
                    auto* block = new QFrame(parent);
                    block->setObjectName(QStringLiteral("sessionMetricBlock"));
                    block->setFixedSize(width, 58);
                    auto* row = new QHBoxLayout(block);
                    row->setContentsMargins(5, 3, 5, 3);
                    row->setSpacing(7);
                    auto* iconLabel = new QLabel(icon, block);
                    iconLabel->setObjectName(QStringLiteral("metricIcon"));
                    iconLabel->setAlignment(Qt::AlignCenter);
                    iconLabel->setFixedWidth(25);
                    row->addWidget(iconLabel);
                    auto* text = new QVBoxLayout();
                    text->setSpacing(0);
                    auto* titleLabel = new QLabel(title, block);
                    titleLabel->setObjectName(QStringLiteral("metricTitle"));
                    auto* valueLabel = new QLabel(initialValue, block);
                    valueLabel->setObjectName(
                        good ? QStringLiteral("metricGoodValue")
                        : QStringLiteral("metricValue"));
                    text->addWidget(titleLabel);
                    text->addWidget(valueLabel);
                    if (icon.isEmpty()) {
                        iconLabel->hide();
                        titleLabel->setAlignment(Qt::AlignCenter);
                        valueLabel->setAlignment(Qt::AlignCenter);
                    }
                    row->addLayout(text, 1);
                    if (valueOut) {
                        *valueOut = valueLabel;
                    }
                    return block;
            };

        auto* deviceCard = new QFrame(leftZone);
        deviceCard->setObjectName(QStringLiteral("sessionDeviceCard"));
        deviceCard->setFixedSize(150, 62);
        auto* deviceLayout = new QHBoxLayout(deviceCard);
        deviceLayout->setContentsMargins(10, 5, 10, 5);
        deviceLayout->setSpacing(8);
        auto* deviceIcon = new QLabel(QStringLiteral("🖥"), deviceCard);
        deviceIcon->setObjectName(QStringLiteral("deviceIcon"));
        deviceIcon->setAlignment(Qt::AlignCenter);
        deviceIcon->setFixedWidth(24);
        deviceLayout->addWidget(deviceIcon);
        auto* nameGroup = new QVBoxLayout();
        nameGroup->setSpacing(0);
        auto* name = new QLabel(binding_.peerDeviceName, deviceCard);
        name->setObjectName(QStringLiteral("sessionName"));
        sessionSourceLabel_ = new QLabel(
            QStringLiteral("%1 · %2")
                .arg(binding_.peerDeviceId, binding_.SourceText()),
            deviceCard);
        sessionSourceLabel_->setObjectName(QStringLiteral("sessionMeta"));
        sessionSourceLabel_->setTextInteractionFlags(Qt::NoTextInteraction);
        nameGroup->addWidget(name);
        nameGroup->addWidget(sessionSourceLabel_);
        deviceLayout->addLayout(nameGroup, 1);
        leftLayout->addWidget(deviceCard);

        previewBadge_ = new QLabel(QStringLiteral("等待画面"), toolbar);
        previewBadge_->setObjectName(QStringLiteral("previewBadge"));
        previewBadge_->hide();

        addSeparator(leftLayout, leftZone);
        auto* connectionMetric = makeMetric(
            leftZone, QString(), QStringLiteral("连接状态"),
            QStringLiteral("正在连接"), &connectionStatusLabel_, 124, true);
        auto* connectionMetricLayout =
            qobject_cast<QHBoxLayout*>(connectionMetric->layout());
        auto* oldConnectionIcon =
            qobject_cast<QLabel*>(connectionMetricLayout->itemAt(0)->widget());
        oldConnectionIcon->hide();
        networkSignalIndicator_ = new NetworkSignalIndicator(connectionMetric);
        connectionMetricLayout->insertWidget(0, networkSignalIndicator_);
        auto* connectionTextLayout = qobject_cast<QVBoxLayout*>(
            connectionMetricLayout->itemAt(2)->layout());
        latencyLabel_ = new QLabel(QStringLiteral("延迟 -- ms"), connectionMetric);
        latencyLabel_->setObjectName(QStringLiteral("metricGoodValue"));
        connectionTextLayout->addWidget(latencyLabel_);
        leftLayout->addWidget(connectionMetric);

        addSeparator(leftLayout, leftZone);
        qualityButton_ = MakeToolButton(
            QStringLiteral("分辨率\n等待画面"),
            QStringLiteral("点击调整远端输出分辨率"), leftZone);
        qualityButton_->setObjectName(QStringLiteral("sessionDisplayTile"));
        qualityButton_->setFixedSize(116, 58);
        qualityMenu_ = new QMenu(qualityButton_);
        qualityGroup_ = new QActionGroup(qualityMenu_);
        qualityGroup_->setExclusive(true);
        RebuildQualityMenu();
        connect(qualityMenu_, &QMenu::triggered, this, [this](QAction* action) {
            if (action) {
                HandleQualitySelection(
                    static_cast<ScreenQualityTier>(action->data().toInt()));
            }
            });
        qualityButton_->setMenu(qualityMenu_);
        qualityButton_->setPopupMode(QToolButton::InstantPopup);
        leftLayout->addWidget(qualityButton_);

        codecLabel_ = new QLabel(QStringLiteral("等待视频流"), toolbar);
        codecLabel_->hide();

        auto* durationMetric = makeMetric(
            toolbar, QString(), QStringLiteral("连接时长"),
            QStringLiteral("00:00:00"), &durationLabel_, 82);

        frameRateButton_ = MakeToolButton(
            QStringLiteral("目标帧率\n%1 FPS").arg(selectedFrameRate_),
            QStringLiteral(
                "等待当前共享端上报最高帧率，暂按 120 FPS 上限显示"),
            rightZone);
        frameRateButton_->setObjectName(QStringLiteral("sessionFrameRateTile"));
        frameRateButton_->setFixedSize(80, 60);
        frameRateMenu_ = new QMenu(frameRateButton_);
        frameRateGroup_ = new QActionGroup(frameRateMenu_);
        frameRateGroup_->setExclusive(true);
        RebuildFrameRateMenu();
        connect(frameRateMenu_, &QMenu::triggered, this,
            [this](QAction* action) {
                if (action) {
                    HandleFrameRateSelection(action->data().toUInt());
                }
            });
        frameRateButton_->setMenu(frameRateMenu_);
        frameRateButton_->setPopupMode(QToolButton::InstantPopup);
        rightLayout->addWidget(frameRateButton_);

        sessionHud_ = new QWidget(
            this,
            Qt::Tool | Qt::FramelessWindowHint |
                Qt::NoDropShadowWindowHint);
        sessionHud_->setObjectName(QStringLiteral("sessionHud"));
        sessionHud_->setAttribute(Qt::WA_StyledBackground, true);
        sessionHud_->setAttribute(Qt::WA_TranslucentBackground, true);
        sessionHud_->setAttribute(Qt::WA_ShowWithoutActivating, true);
        sessionHud_->setWindowFlag(Qt::WindowDoesNotAcceptFocus, true);
        sessionHud_->setFixedSize(58, 128);
        auto* hudLayout = new QVBoxLayout(sessionHud_);
        hudLayout->setContentsMargins(0, 0, 0, 0);
        hudLayout->setSpacing(4);
        hudLayout->setAlignment(Qt::AlignTop | Qt::AlignHCenter);
        hudFrameRateLabel_ = new QLabel(QStringLiteral("-- FPS"), sessionHud_);
        hudFrameRateLabel_->setObjectName(QStringLiteral("hudFrameRateLabel"));
        hudFrameRateLabel_->setAlignment(Qt::AlignCenter);
        hudFrameRateLabel_->setFixedHeight(24);
        hudLayout->addWidget(hudFrameRateLabel_);
        mediaDeviceButton_ = MakeToolButton(
            QStringLiteral(":/ui/icons/actions/settings.svg"),
            QStringLiteral("显示与音视频设备设置"),
            sessionHud_);
        mediaDeviceButton_->setObjectName(
            QStringLiteral("sessionHudButton"));
        mediaDeviceButton_->setFixedSize(36, 28);
        hudLayout->addWidget(
            mediaDeviceButton_, 0, Qt::AlignHCenter);
        remoteDisplayButton_ = MakeToolButton(
            QStringLiteral(":/ui/icons/actions/display.svg"),
            QStringLiteral("切换远端显示器"),
            sessionHud_);
        remoteDisplayButton_->setObjectName(
            QStringLiteral("sessionHudButton"));
        remoteDisplayButton_->setFixedSize(36, 28);
        remoteDisplayButton_->setEnabled(false);
        hudLayout->addWidget(
            remoteDisplayButton_, 0, Qt::AlignHCenter);
        toolbarLockButton_ = new MorphIconToolButton(
            QStringLiteral(":/ui/icons/lucide/base/lock-keyhole-open.svg"),
            QStringLiteral(":/ui/icons/lucide/base/lock-keyhole.svg"),
            sessionHud_);
        toolbarLockButton_->setToolTip(QStringLiteral("锁定顶部控制面板"));
        toolbarLockButton_->setObjectName(QStringLiteral("sessionHudButton"));
        toolbarLockButton_->setFixedSize(36, 28);
        toolbarLockButton_->setProperty("locked", false);
        hudLayout->addWidget(toolbarLockButton_, 0, Qt::AlignHCenter);
        remotePasteStatusHost_ = new QWidget(sessionHud_);
        // The 28-pixel transfer button starts at y = 6, so its host must be
        // at least 34 pixels high or the bottom two pixels are clipped.
        remotePasteStatusHost_->setFixedSize(58, 34);
        remotePasteStatusHost_->hide();
        remotePasteStatusButton_ = new LiquidTransferButton(
            remotePasteStatusHost_);
        // Center the compact 24-pixel transfer body in the same 58-pixel HUD
        // slot as the three controls above it.
        // QWidget positions use integer logical pixels; 15.5 was implicitly
        // truncated to 15, so make the effective position explicit.
        remotePasteStatusButton_->move(15, 6);
        remotePasteStatusButton_->setObjectName(
            QStringLiteral("sessionHudButton"));
        remotePasteStatusButton_->setProperty("transferActive", true);
        remotePasteStatusButton_->show();
        hudLayout->addWidget(
            remotePasteStatusHost_, 0, Qt::AlignHCenter);

        auto* actionCluster = new QFrame(rightZone);
        actionCluster->setObjectName(QStringLiteral("sessionActionCluster"));
        actionCluster->setFixedHeight(70);
        actionCluster->setSizePolicy(
            QSizePolicy::Expanding, QSizePolicy::Fixed);

        auto* actionLayout = new QHBoxLayout(actionCluster);
        actionLayout->setContentsMargins(6, 5, 6, 5);
        actionLayout->setSpacing(5);
        actionLayout->setAlignment(Qt::AlignVCenter);

        const auto addActionSeparator = [actionCluster, actionLayout] {
            auto* separator = new QFrame(actionCluster);
            separator->setObjectName(QStringLiteral("actionInnerSeparator"));
            separator->setFixedHeight(44);
            actionLayout->addWidget(separator);
            };

        controlBadge_ = new ActionTile(
            QStringLiteral(":/ui/icons/actions/view.svg"),
            QStringLiteral("仅观看"), 76, actionCluster);
        controlBadge_->SetInteractive(false);
        controlBadge_->SetTone(QStringLiteral("muted"));
        controlBadge_->hide();

        controlButton_ = new ActionTile(
            QString(), QStringLiteral("申请控制"), 110, actionCluster);
        controlButton_->SetTone(QStringLiteral("primary"));

        actionLayout->addWidget(controlButton_);

        addActionSeparator();

        speakerButton_ = new ActionTile(
            QString(), QStringLiteral("声音"), 70, actionCluster);

        speakerButton_->setToolTip(QStringLiteral("关闭远端声音"));
        actionLayout->addWidget(speakerButton_);

        microphoneButton_ = new ActionTile(
            QString(), QStringLiteral("麦克风"), 90, actionCluster);

        microphoneButton_->SetSlashVisible(true);
        microphoneButton_->setToolTip(QStringLiteral("开启本机麦克风"));
        actionLayout->addWidget(microphoneButton_);

        fileTransferButton_ = new ActionTile(
            QString(), QStringLiteral("文件"), 70, actionCluster);
        fileTransferButton_->setToolTip(QStringLiteral("打开文件传输"));
        actionLayout->addWidget(fileTransferButton_);

        addActionSeparator();

        fullScreenButton_ = new ActionTile(
            QString(), QStringLiteral("全屏"), 70, actionCluster);

        fullScreenButton_->setToolTip(QStringLiteral("切换全屏"));
        actionLayout->addWidget(fullScreenButton_);

        addActionSeparator();

        auto* disconnect = new ActionTile(
            QString(), QStringLiteral("断开"), 82, actionCluster);
        disconnect->SetTone(QStringLiteral("danger"));
        disconnect->setFixedSize(82, 60);
        disconnect->setToolTip(QStringLiteral("断开当前远程会话"));
        actionLayout->addWidget(disconnect);

        rightLayout->addWidget(actionCluster, 1, Qt::AlignVCenter);

        controlButton_->hide();

        toolbarLayout->addWidget(leftZone, 0, 0);
        toolbarLayout->addWidget(durationMetric, 0, 1, Qt::AlignCenter);
        toolbarLayout->addWidget(rightZone, 0, 2);

        setCentralWidget(rootWidget);
        desktopCanvas_->show();
        toolbar->show();
        sessionHud_->show();
        LayoutSessionOverlays();

        toolbarAnimation_ = new QPropertyAnimation(toolbar, "pos", this);
        toolbarAnimation_->setEasingCurve(QEasingCurve::InQuint);
        toolbarHideTimer_ = new QTimer(this);
        toolbarHideTimer_->setSingleShot(true);
        connect(toolbarHideTimer_, &QTimer::timeout, this, [this] {
            if ((sessionToolbar_ && sessionToolbar_->underMouse()) ||
                (qualityMenu_ && qualityMenu_->isVisible()) ||
                (frameRateMenu_ && frameRateMenu_->isVisible()) ||
                mediaDeviceMenuOpen_) {
                ScheduleSessionToolbarHide();
                return;
            }
            HideSessionToolbar();
            });
        for (QMenu* menu : { qualityMenu_, frameRateMenu_ }) {
            connect(menu, &QMenu::aboutToShow, this, [this] {
                ShowSessionToolbar(false);
                toolbarHideTimer_->stop();
                });
            connect(menu, &QMenu::aboutToHide, this, [this] {
                ScheduleSessionToolbarHide();
                });
        }

        durationTimer_ = new QTimer(this);
        connect(durationTimer_, &QTimer::timeout, this,
            &RemoteSessionWindow::UpdateSessionDuration);
        connect(controlButton_, &QPushButton::clicked, this,
            &RemoteSessionWindow::HandleControlAction);
        connect(speakerButton_, &QPushButton::clicked, this,
            &RemoteSessionWindow::ToggleRemoteSound);
        connect(microphoneButton_, &QPushButton::clicked, this,
            &RemoteSessionWindow::ToggleLocalMicrophone);
        connect(fileTransferButton_, &QPushButton::clicked, this,
            [this] {
                if (fileTransferHandler_) fileTransferHandler_();
            });
        connect(mediaDeviceButton_, &QToolButton::clicked, this,
            &RemoteSessionWindow::ShowMediaDeviceMenu);
        connect(remoteDisplayButton_, &QToolButton::clicked, this,
            &RemoteSessionWindow::ShowRemoteDisplayMenu);
        connect(toolbarLockButton_, &QToolButton::clicked, this,
            &RemoteSessionWindow::ToggleToolbarLock);
        connect(remotePasteStatusButton_, &QToolButton::clicked, this,
            &RemoteSessionWindow::RestoreRemotePasteProgress);
        connect(fullScreenButton_, &QPushButton::clicked, this,
            &RemoteSessionWindow::ToggleFullScreenMode);
        auto* fullScreenShortcut = new QShortcut(
            QKeySequence(Qt::Key_F11), this);
        fullScreenShortcut->setContext(Qt::WindowShortcut);
        connect(fullScreenShortcut, &QShortcut::activated, this,
            &RemoteSessionWindow::ToggleFullScreenMode);
        connect(disconnect, &QPushButton::clicked, this,
            &RemoteSessionWindow::HandleDisconnectAction);
        QTimer::singleShot(1800, this, [this] {
            ScheduleSessionToolbarHide();
            });
    }
