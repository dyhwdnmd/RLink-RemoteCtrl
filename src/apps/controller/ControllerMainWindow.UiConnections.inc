// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

    QVBoxLayout* helpPageLayout = nullptr;
    auto* helpPage = MakePageSurface(
        QStringLiteral("帮助与反馈"),
        QStringLiteral("了解连接方式，或在遇到问题时快速定位原因。"),
        pageStack_, &helpPageLayout);
    auto* helpCard = new QFrame(helpPage);
    helpCard->setProperty("card", true);
    auto* helpLayout = new QVBoxLayout(helpCard);
    helpLayout->setContentsMargins(22, 20, 22, 20);
    helpLayout->setSpacing(10);
    auto* helpTitle = new QLabel(
        QStringLiteral("连接说明"), helpCard);
    helpTitle->setObjectName(QStringLiteral("cardTitle"));
    helpLayout->addWidget(helpTitle);
    auto* helpText = new QLabel(
        QStringLiteral(
            "双方先通过 WSS 完成设备与房间协调，屏幕、摄像头、语音、输入和文件数据随后通过 WebRTC P2P 传输。"),
        helpCard);
    helpText->setProperty("muted", true);
    helpText->setWordWrap(true);
    helpLayout->addWidget(helpText);
    helpPageLayout->addWidget(helpCard);
    helpPageLayout->addStretch(1);
    pageStack_->addWidget(helpPage);

    QVBoxLayout* authorPageLayout = nullptr;
    auto* authorPage = MakePageSurface(
        QStringLiteral("找到作者"),
        QStringLiteral("扫描二维码，在小红书关注 RLink 作者。"),
        pageStack_, &authorPageLayout);
    auto* authorCard = new QFrame(authorPage);
    authorCard->setProperty("card", true);
    authorCard->setMaximumWidth(520);
    authorCard->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Maximum);
    auto* authorLayout = new QVBoxLayout(authorCard);
    authorLayout->setContentsMargins(28, 24, 28, 26);
    authorLayout->setSpacing(10);
    authorLayout->setAlignment(Qt::AlignHCenter);
    auto* authorTitle = new QLabel(QStringLiteral("过期酸奶"), authorCard);
    authorTitle->setObjectName(QStringLiteral("cardTitle"));
    authorTitle->setAlignment(Qt::AlignCenter);
    authorLayout->addWidget(authorTitle);
    auto* authorAccount = new QLabel(
        QStringLiteral("小红书号：9938599840"), authorCard);
    authorAccount->setProperty("muted", true);
    authorAccount->setAlignment(Qt::AlignCenter);
    auto* authorAccountRow = new QHBoxLayout();
    authorAccountRow->setSpacing(8);
    authorAccountRow->setAlignment(Qt::AlignCenter);
    authorAccountRow->addWidget(authorAccount);
    auto* copyAuthorAccountButton = new QPushButton(
        QStringLiteral("复制"), authorCard);
    copyAuthorAccountButton->setObjectName(QStringLiteral("softButton"));
    copyAuthorAccountButton->setCursor(Qt::PointingHandCursor);
    copyAuthorAccountButton->setIconSize(QSize(15, 15));
    copyAuthorAccountButton->setFixedHeight(34);
    copyAuthorAccountButton->setMinimumWidth(72);
    ui::RemoteCTheme::SetIcon(
        copyAuthorAccountButton,
        QStringLiteral(":/ui/icons/actions/copy.svg"),
        ui::ThemeIconTone::kPrimary);
    authorAccountRow->addWidget(copyAuthorAccountButton);
    authorLayout->addLayout(authorAccountRow);
    connect(copyAuthorAccountButton, &QPushButton::clicked,
            this, [this, copyAuthorAccountButton] {
        QApplication::clipboard()->setText(QStringLiteral("9938599840"));
        copyAuthorAccountButton->setText(QStringLiteral("已复制"));
        RemoteCToast::Show(
            this, QStringLiteral("小红书号已复制"),
            RemoteCToast::Tone::kSuccess);
        QTimer::singleShot(1300, copyAuthorAccountButton,
                           [copyAuthorAccountButton] {
            copyAuthorAccountButton->setText(QStringLiteral("复制"));
        });
    });
    auto* authorProfileLink = new QLabel(
        QStringLiteral(
            "<a style=\"color:#4F6EF7;text-decoration:none;font-weight:600;\" "
            "href=\"https://www.xiaohongshu.com/user/profile/64f699ef000000000603375f\">"
            "打开小红书个人主页</a>"),
        authorCard);
    authorProfileLink->setTextFormat(Qt::RichText);
    authorProfileLink->setTextInteractionFlags(Qt::TextBrowserInteraction);
    authorProfileLink->setOpenExternalLinks(true);
    authorProfileLink->setAlignment(Qt::AlignCenter);
    authorProfileLink->setCursor(Qt::PointingHandCursor);
    authorLayout->addWidget(authorProfileLink);
    authorLayout->addSpacing(8);
    auto* authorQrCode = new QPushButton(authorCard);
    const QPixmap authorQrPixmap(
        QStringLiteral(":/ui/branding/xiaohongshu-author.png"));
    authorQrCode->setIcon(QIcon(authorQrPixmap));
    authorQrCode->setIconSize(QSize(240, 328));
    authorQrCode->setFixedSize(240, 328);
    authorQrCode->setFlat(true);
    authorQrCode->setCursor(Qt::PointingHandCursor);
    authorQrCode->setToolTip(QStringLiteral("打开小红书个人主页"));
    authorQrCode->setStyleSheet(QStringLiteral(
        "QPushButton { border: none; background: transparent; padding: 0; }"));
    connect(authorQrCode, &QPushButton::clicked, this, [] {
        QDesktopServices::openUrl(QUrl(QStringLiteral(
            "https://www.xiaohongshu.com/user/profile/64f699ef000000000603375f")));
    });
    authorLayout->addWidget(authorQrCode, 0, Qt::AlignHCenter);
    auto* authorHint = new QLabel(
        QStringLiteral("扫描二维码，在小红书找到我"), authorCard);
    authorHint->setProperty("muted", true);
    authorHint->setAlignment(Qt::AlignCenter);
    authorLayout->addWidget(authorHint);
    authorPageLayout->addWidget(authorCard, 0, Qt::AlignHCenter);
    authorPageLayout->addStretch(1);
    pageStack_->addWidget(authorPage);

    shell->addWidget(pageStack_, 1);
    outer->addWidget(body, 1);
    setCentralWidget(central);

    connect(roomNavButton_, &QPushButton::clicked, this, [this] {
        SelectMainPage(0, roomNavButton_, QStringLiteral("协作房间"));
    });
    connect(deviceNavButton_, &QPushButton::clicked, this, [this] {
        SelectMainPage(1, deviceNavButton_, QStringLiteral("远程协助"));
    });
    const auto refreshAssistanceButton = [this] {
        if (!engine_) return;
        const auto snapshot = engine_->Snapshot();
        const QString deviceId = deviceIdEdit_->text().trimmed();
        const QString verificationCode =
            verificationCodeEdit_->text().trimmed();
        const bool ready = authenticationAvailable_ &&
            snapshot.connectivity == SessionConnectivityState::kOnline &&
            snapshot.state == SessionEngineState::kReady;
        connectButton_->setEnabled(
            ready && IsNineDigitPublicId(deviceId) &&
            verificationCode.size() == 6 &&
            deviceId.toStdString() != snapshot.localDeviceId);
        if (connectionHint_->property("error").toBool()) {
            connectionHint_->setProperty("error", false);
            connectionHint_->setText(
                QStringLiteral(
                    "验证码正确后将自动打开远程桌面，无需对方确认。"));
            connectionHint_->setStyleSheet({});
        }
    };
    connect(deviceIdEdit_, &QLineEdit::textChanged, this,
            [refreshAssistanceButton](const QString&) {
                refreshAssistanceButton();
            });
    connect(verificationCodeEdit_, &QLineEdit::textChanged, this,
            [refreshAssistanceButton](const QString&) {
                refreshAssistanceButton();
            });
    connect(deviceIdEdit_, &QLineEdit::returnPressed,
            connectButton_, &QPushButton::click);
    connect(verificationCodeEdit_, &QLineEdit::returnPressed,
            connectButton_, &QPushButton::click);
    connect(connectButton_, &QPushButton::clicked, this, [this] {
        if (!engine_) return;
        const auto snapshot = engine_->Snapshot();
        if (assistedSessionPending_ &&
            snapshot.state == SessionEngineState::kConnecting) {
            assistedSessionCancellationPending_ = true;
            assistedSessionTimeoutTimer_->stop();
            connectButton_->setEnabled(false);
            connectButton_->setText(QStringLiteral("正在取消…"));
            connectionHint_->setProperty("error", false);
            connectionHint_->setText(QStringLiteral("正在取消本次连接…"));
            connectionHint_->setStyleSheet(
                QStringLiteral("color:#a66b12;"));
            const auto result = engine_->Disconnect();
            if (!result.accepted) {
                assistedSessionPending_ = false;
                assistedSessionCancellationPending_ = false;
                connectionHint_->setProperty("error", true);
                connectionHint_->setText(
                    QStringLiteral("取消连接失败，请稍后重试。"));
                connectionHint_->setStyleSheet(
                    QStringLiteral("color:#d14343;"));
            }
            return;
        }
        const QString deviceId = deviceIdEdit_->text().trimmed();
        const QString verificationCode =
            verificationCodeEdit_->text().trimmed();
        if (!IsNineDigitPublicId(deviceId) ||
            verificationCode.size() != 6) return;
        StartAssistedSession(deviceId, verificationCode);
    });
    connect(copyVerificationCodeButton_, &QPushButton::clicked,
            this, [this] {
        const QString code = localVerificationCode_->text().trimmed();
        if (code.size() != 6) return;
        QApplication::clipboard()->setText(code);
        if (auto* morph =
                remotec::ui::morph::MorphIconButtonBinding::attach(
                    copyVerificationCodeButton_,
                    QStringLiteral(":/ui/icons/lucide/base/copy.svg"),
                    QStringLiteral(":/ui/icons/lucide/base/circle-check-big.svg"),
                    remotec::ui::morph::MorphIconButtonBinding::Interaction::Feedback,
                    QSize(15, 15), QColor(QStringLiteral("#2563EB")),
                    QColor(QStringLiteral("#12B76A")))) {
            morph->pulse();
        }
        RemoteCToast::Show(
            this, QStringLiteral("验证码已复制"),
            RemoteCToast::Tone::kSuccess);
    });
    connect(shareLocalCredentialsButton_, &QPushButton::clicked,
            this, [this] {
        const QString deviceId = localDeviceId_->text().trimmed();
        const QString code = localVerificationCode_->text().trimmed();
        if (!IsNineDigitPublicId(deviceId) || code.size() != 6) {
            return;
        }
        const QString shareText = QStringLiteral(
            "RLink 远程协助\n"
            "设备 ID：%1\n"
            "验证码：%2\n\n"
            "请在验证码有效期内使用。")
            .arg(deviceId, code);
        QApplication::clipboard()->setText(shareText);
        shareLocalCredentialsButton_->setText(QStringLiteral("已复制"));
        AnimateSmallUiChange(shareLocalCredentialsButton_);
        QTimer::singleShot(1300, shareLocalCredentialsButton_, [this] {
            if (shareLocalCredentialsButton_) {
                shareLocalCredentialsButton_->setText(QStringLiteral("分享"));
                AnimateSmallUiChange(shareLocalCredentialsButton_);
            }
        });
        RemoteCToast::Show(
            this, QStringLiteral("分享信息已复制，可直接发送给对方"),
            RemoteCToast::Tone::kSuccess);
    });
    connect(myDevicesNavButton_, &QPushButton::clicked, this, [this] {
        SelectMainPage(2, myDevicesNavButton_, QStringLiteral("我的设备"));
        if (engine_) {
            const auto result = engine_->RefreshOwnedDevices();
            if (!result.accepted &&
                result.errorCode != "signaling_not_online") {
                RemoteCToast::Show(
                    this, QString::fromStdString(result.errorMessage),
                    RemoteCToast::Tone::kError);
            }
        }
    });
    connect(ownedDevicesRefreshButton_, &QPushButton::clicked, this, [this] {
        const bool dark = ui::RemoteCTheme::IsDark(
            ui::RemoteCTheme::LoadPreference());
        const QColor refreshColor(
            dark ? QStringLiteral("#F7F9FF")
                 : QStringLiteral("#2563EB"));
        if (auto* morph =
                remotec::ui::morph::MorphIconButtonBinding::attach(
                    ownedDevicesRefreshButton_,
                    QStringLiteral(":/ui/icons/lucide/base/refresh-cw.svg"),
                    QStringLiteral(":/ui/icons/lucide/base/refresh-ccw.svg"),
                    remotec::ui::morph::MorphIconButtonBinding::Interaction::Feedback,
                    QSize(18, 18), refreshColor, refreshColor)) {
            morph->pulse(500);
        }
        if (!engine_) return;
        const auto result = engine_->RefreshOwnedDevices();
        if (!result.accepted) {
            RemoteCToast::Show(
                this, QString::fromStdString(result.errorMessage),
                RemoteCToast::Tone::kError);
        }
    });
    connect(deviceLoginButton_, &QPushButton::clicked, this, [this] {
        if (accountInteractionCallback_) {
            accountInteractionCallback_();
        }
    });
    connect(recentNavButton_, &QPushButton::clicked, this, [this] {
        SelectMainPage(3, recentNavButton_, QStringLiteral("最近连接"));
        RefreshRecentRooms();
        RefreshRecentDevices();
    });
    connect(debugNavButton_, &QPushButton::clicked, this, [this] {
        SelectMainPage(4, debugNavButton_, QStringLiteral("调试信息"));
        if (engine_) {
            OnSessionEngineSnapshot(engine_->Snapshot());
            RefreshDiagnosticsUi();
        }
    });
    connect(settingsNavButton_, &QPushButton::clicked, this,
            [this, settingsDetailStack] {
        SelectMainPage(5, settingsNavButton_, QStringLiteral("设置"));
        if (settingsDetailStack->currentIndex() == 2) {
            RequestMediaDeviceRefresh(false);
        }
    });
    connect(helpNavButton_, &QPushButton::clicked, this, [this] {
        SelectMainPage(6, helpNavButton_, QStringLiteral("帮助与反馈"));
    });
    connect(authorNavButton_, &QPushButton::clicked, this, [this] {
        SelectMainPage(7, authorNavButton_, QStringLiteral("找到作者"));
    });
    connect(themeModeSelector_, &QComboBox::currentIndexChanged,
            this, [this](int) {
                SetInterfaceThemePreference(
                    themeModeSelector_->currentData().toString());
            });
    connect(animationLevelSelector_, &QComboBox::currentIndexChanged,
            this, [this](int) {
                SetAnimationLevel(
                    animationLevelSelector_->currentData().toInt());
            });
    connect(fontFamilySelector_, &QComboBox::currentIndexChanged,
            this, [this](int) {
                const QString family =
                    fontFamilySelector_->currentData().toString();
                QSettings settings;
                settings.setValue(
                    QString::fromLatin1(kInterfaceFontFamilySetting), family);
                ApplyInterfaceFontPreference(
                    family,
                    settings.value(QStringLiteral("ui/fontPixelSize"), 13)
                        .toInt());
                FramelessMainWindow::RefreshAllWindowStyles();
                RemoteCToast::Show(
                    this,
                    family.isEmpty()
                        ? QStringLiteral("已恢复默认界面字体")
                        : QStringLiteral("界面字体已切换为 %1").arg(family),
                    RemoteCToast::Tone::kSuccess);
            });
    connect(fontSizeSelector_, &QComboBox::currentIndexChanged,
            this, [this](int) {
                const int pixelSize = fontSizeSelector_->currentData().toInt();
                QSettings settings;
                settings.setValue(
                    QStringLiteral("ui/fontPixelSize"), pixelSize);
                ApplyInterfaceFontPreference(
                    settings.value(
                        QString::fromLatin1(kInterfaceFontFamilySetting))
                        .toString(),
                    pixelSize);
                FramelessMainWindow::RefreshAllWindowStyles();
                RemoteCToast::Show(
                    this, QStringLiteral("字体大小已更新"),
                    RemoteCToast::Tone::kSuccess);
            });
    connect(autoStartSelector_, &QComboBox::currentIndexChanged,
            this, [this](int) {
                const bool enabled =
                    autoStartSelector_->currentData().toBool();
                if (!SetWindowsAutoStartEnabled(enabled)) {
                    const QSignalBlocker blocker(autoStartSelector_);
                    autoStartSelector_->setCurrentIndex(std::max(
                        0, autoStartSelector_->findData(
                            WindowsAutoStartEnabled())));
                    RemoteCToast::Show(
                        this, QStringLiteral("开机启动设置失败"),
                        RemoteCToast::Tone::kError);
                    return;
                }
                RemoteCToast::Show(
                    this,
                    enabled ? QStringLiteral("已开启开机启动")
                            : QStringLiteral("已关闭开机启动"),
                    RemoteCToast::Tone::kSuccess);
            });
    connect(startupVisibilitySelector_, &QComboBox::currentIndexChanged,
            this, [this](int) {
                QSettings settings;
                settings.setValue(
                    QString::fromLatin1(kStartupVisibilitySetting),
                    startupVisibilitySelector_->currentData().toString());
                RemoteCToast::Show(
                    this, QStringLiteral("启动方式已保存，下次启动生效"),
                    RemoteCToast::Tone::kSuccess);
            });
    connect(closeButtonBehaviorSelector_, &QComboBox::currentIndexChanged,
            this, [this](int) {
                QSettings settings;
                settings.setValue(
                    QString::fromLatin1(kCloseButtonBehaviorSetting),
                    closeButtonBehaviorSelector_->currentData().toString());
                RemoteCToast::Show(
                    this, QStringLiteral("关闭按钮行为已更新"),
                    RemoteCToast::Tone::kSuccess);
            });
    connect(defaultRoomCapacitySelector_, &QComboBox::currentIndexChanged,
            this, [this](int) {
                const std::uint32_t capacity =
                    defaultRoomCapacitySelector_->currentData().toUInt();
                QSettings settings;
                settings.setValue(
                    QString::fromLatin1(kDefaultRoomCapacitySetting),
                    QVariant::fromValue(capacity));
                if (createRoomCapacity_) {
                    createRoomCapacity_->setCurrentIndex(
                        createRoomCapacity_->findData(
                            QVariant::fromValue(capacity)));
                }
                RemoteCToast::Show(
                    this, QStringLiteral("默认房间人数已更新"),
                    RemoteCToast::Tone::kSuccess);
            });
    connect(cameraGalleryBehaviorSelector_,
            &QComboBox::currentIndexChanged, this, [this](int) {
                const bool autoOpen =
                    cameraGalleryBehaviorSelector_->currentData().toBool();
                QSettings settings;
                settings.setValue(
                    QString::fromLatin1(kAutoOpenCameraGallerySetting),
                    autoOpen);
                if (autoOpen && engine_) {
                    OnSessionEngineSnapshot(engine_->Snapshot());
                }
                RemoteCToast::Show(
                    this, QStringLiteral("摄像头画廊行为已更新"),
                    RemoteCToast::Tone::kSuccess);
            });
    const auto beginDeviceSelection =
        [this](QComboBox* selector,
               QString* pendingDeviceId,
               const QString& deviceName,
               auto selectOperation) {
            connect(
                selector, &QComboBox::currentIndexChanged,
                this,
                [this, selector, pendingDeviceId,
                 deviceName, selectOperation](int) {
                    if (!engine_ || !pendingDeviceId ||
                        selector->currentIndex() < 0) {
                        return;
                    }
                    const QString selectedId =
                        selector->currentData().toString();
                    if (selectedId.isEmpty()) {
                        return;
                    }
                    *pendingDeviceId = selectedId;
                    selector->setEnabled(false);
                    if (mediaDeviceStatusLabel_) {
                        mediaDeviceStatusLabel_->setText(
                            QStringLiteral("正在切换%1，请稍候…")
                                .arg(deviceName));
                    }
                    const auto result =
                        (engine_.get()->*selectOperation)(
                            selectedId.toStdString());
                    if (!result.accepted) {
                        pendingDeviceId->clear();
                        mediaDeviceRevision_ = 0;
                        RemoteCToast::Show(
                            this,
                            QStringLiteral("%1切换失败：%2")
                                .arg(
                                    deviceName,
                                    QString::fromStdString(
                                        result.errorMessage)),
                            RemoteCToast::Tone::kError);
                        UpdateLocalMediaDevicesUi(
                            engine_->Snapshot());
                    }
                });
        };
    beginDeviceSelection(
        cameraDeviceSelector_, &pendingCameraDeviceId_,
        QStringLiteral("摄像头"),
        &ISessionEngine::SelectLocalCameraDevice);
    beginDeviceSelection(
        microphoneDeviceSelector_,
        &pendingMicrophoneDeviceId_,
        QStringLiteral("麦克风"),
        &ISessionEngine::SelectLocalMicrophoneDevice);
    beginDeviceSelection(
        speakerDeviceSelector_, &pendingSpeakerDeviceId_,
        QStringLiteral("扬声器"),
        &ISessionEngine::SelectLocalSpeakerDevice);
    connect(refreshMediaDevicesButton_, &QPushButton::clicked,
            this, [this] {
                RequestMediaDeviceRefresh(true);
            });
    const auto connectClipboardSetting = [this](QComboBox* selector) {
        connect(selector, &QComboBox::currentIndexChanged,
                this, [this](int) {
                    ApplyClipboardConfigurationFromUi(true);
                });
    };
    connectClipboardSetting(remotePasteEnabledSelector_);
    connectClipboardSetting(clipboardFormatsSelector_);
    connectClipboardSetting(clipboardLargeFileLimitSelector_);
    connectClipboardSetting(clipboardCacheRetentionSelector_);
    connectClipboardSetting(clipboardCacheCapacitySelector_);
    const auto persistMediaPreference =
        [this](QComboBox* selector, const char* settingKey,
               const QString& displayName) {
            connect(selector, &QComboBox::currentIndexChanged,
                    this, [this, selector, settingKey, displayName](int) {
                        QSettings settings;
                        settings.setValue(
                            QString::fromLatin1(settingKey),
                            selector->currentData().toString());
                        RemoteCToast::Show(
                            this,
                            QStringLiteral(
                                "%1已更新，重新打开监控窗口后生效")
                                .arg(displayName),
                            RemoteCToast::Tone::kSuccess);
                    });
        };
    const auto applyVideoPipelineSetting =
        [this](QComboBox* selector, const QString& displayName) {
        connect(selector, &QComboBox::currentIndexChanged,
                this, [this, displayName](int) {
                    if (engine_) {
                        UpdateVideoPipelineSettingsAvailability(
                            engine_->Snapshot());
                    }
                    ApplyVideoPipelineSettingsFromUi(true, displayName);
                });
    };
    applyVideoPipelineSetting(
        desktopCaptureSelector_, QStringLiteral("屏幕采集器"));
    applyVideoPipelineSetting(
        videoEncoderSelector_, QStringLiteral("视频编码器"));
    applyVideoPipelineSetting(
        ffmpegHardwareBackendSelector_, QStringLiteral("硬件编码后端"));
    applyVideoPipelineSetting(
        ffmpegX264PresetSelector_, QStringLiteral("编码质量"));
    applyVideoPipelineSetting(
        videoDecoderSelector_, QStringLiteral("视频解码器"));
    connect(ffmpegX264PresetSelector_, &QComboBox::currentIndexChanged,
            this, [this] { RefreshEncoderBenchmarkSummary(); });
    persistMediaPreference(
        videoRendererSelector_, kVideoRendererPreferenceSetting,
        QStringLiteral("画面渲染器"));
    connect(
        dragPointerSampleRateSelector_,
        &QComboBox::currentIndexChanged,
        this,
        [this](int) {
            const std::uint32_t hertz =
                dragPointerSampleRateSelector_
                    ->currentData().toUInt();
            QSettings settings;
            settings.setValue(
                QString::fromLatin1(
                    kDragPointerSampleRateSetting),
                QVariant::fromValue(hertz));
            if (remoteSessionWindow_) {
                remoteSessionWindow_
                    ->SetDragPointerSampleRate(hertz);
            }
            RemoteCToast::Show(
                this,
                QStringLiteral("拖动采样率已更新为 %1 Hz")
                    .arg(hertz),
                RemoteCToast::Tone::kSuccess);
        });
    connect(decoderBenchmarkButton_, &QPushButton::clicked,
            this, [this] { StartDecoderBenchmark(true); });
    connect(encoderBenchmarkButton_, &QPushButton::clicked,
            this, [this] { StartEncoderBenchmark(true); });
    connect(desktopCaptureSelector_, &QComboBox::currentIndexChanged,
            this, [this](int) { RefreshEncoderBenchmarkSummary(); });
    connect(chooseSaveDirectory, &QPushButton::clicked, this,
            [this, saveDirectoryPath] {
                QSettings settings;
                const QString currentDirectory = settings.value(
                    QString::fromLatin1(kDefaultFileSaveDirectorySetting),
                    InitialFileSaveDirectory()).toString();
                const QString selectedDirectory =
                    QFileDialog::getExistingDirectory(
                        this, QStringLiteral("选择文件默认存储路径"),
                        currentDirectory);
                if (selectedDirectory.isEmpty()) {
                    return;
                }
                const QString normalizedDirectory =
                    QDir::cleanPath(selectedDirectory);
                settings.setValue(
                    QString::fromLatin1(kDefaultFileSaveDirectorySetting),
                    normalizedDirectory);
                saveDirectoryPath->setText(
                    QDir::toNativeSeparators(normalizedDirectory));
                saveDirectoryPath->setToolTip(saveDirectoryPath->text());
                RemoteCToast::Show(
                    this, QStringLiteral("默认存储路径已更新"),
                    RemoteCToast::Tone::kSuccess);
            });
    const auto applyClipboardCacheBaseDirectory =
        [this, refreshClipboardCacheCapacityOptions](
            const QString& selectedDirectory) {
            const QString normalized = QDir::cleanPath(selectedDirectory);
            QTemporaryFile writeProbe(QDir(normalized).filePath(
                QStringLiteral(".remotec-write-test-XXXXXX")));
            if (!writeProbe.open()) {
                RemoteCDialog::Alert(
                    this, QStringLiteral("缓存路径不可写"),
                    QStringLiteral(
                        "RLink 无法在所选位置创建缓存文件，请选择当前用户具有写入权限的目录。"));
                return;
            }
            writeProbe.close();
            const qulonglong safeGiB =
                SafeClipboardCacheCapacityGiB(normalized);
            if (safeGiB < 1) {
                RemoteCDialog::Alert(
                    this, QStringLiteral("缓存路径空间不足"),
                    QStringLiteral(
                        "所选磁盘的可用空间不足 2 GiB，无法设置至少 1 GiB 的安全缓存上限。"));
                return;
            }
            const qulonglong previous =
                clipboardCacheCapacitySelector_->currentData()
                    .toULongLong();
            QSettings settings;
            settings.setValue(
                QString::fromLatin1(
                    kClipboardCacheBaseDirectorySetting),
                normalized);
            clipboardCachePathLabel_->setText(
                QDir::toNativeSeparators(
                    ClipboardCacheRootForBase(normalized)));
            clipboardCachePathLabel_->setToolTip(
                clipboardCachePathLabel_->text());
            refreshClipboardCacheCapacityOptions(
                normalized, previous);
            ApplyClipboardConfigurationFromUi(false);
            if (clipboardController_) {
                (void)clipboardController_->RefreshCacheStatistics();
            }
            RemoteCToast::Show(
                this, QStringLiteral("远程粘贴缓存路径已更新"),
                RemoteCToast::Tone::kSuccess);
        };
    connect(chooseClipboardCacheDirectory, &QPushButton::clicked,
            this, [this, applyClipboardCacheBaseDirectory] {
                QSettings settings;
                const QString currentDirectory = settings.value(
                    QString::fromLatin1(
                        kClipboardCacheBaseDirectorySetting),
                    InitialClipboardCacheBaseDirectory()).toString();
                const QString selected =
                    QFileDialog::getExistingDirectory(
                        this, QStringLiteral("选择远程粘贴缓存位置"),
                        currentDirectory);
                if (!selected.isEmpty()) {
                    applyClipboardCacheBaseDirectory(selected);
                }
            });
    connect(resetClipboardCacheDirectory, &QPushButton::clicked,
            this, [applyClipboardCacheBaseDirectory] {
                applyClipboardCacheBaseDirectory(
                    InitialClipboardCacheBaseDirectory());
            });
    connect(openClipboardCacheDirectory, &QPushButton::clicked,
            this, [this] {
                QSettings settings;
                const QString base = settings.value(
                    QString::fromLatin1(
                        kClipboardCacheBaseDirectorySetting),
                    InitialClipboardCacheBaseDirectory()).toString();
                const QString root = ClipboardCacheRootForBase(base);
                if (!QDir().mkpath(root) ||
                    !QDesktopServices::openUrl(
                        QUrl::fromLocalFile(root))) {
                    RemoteCDialog::Alert(
                        this, QStringLiteral("无法打开缓存路径"),
                        QStringLiteral(
                            "RLink 无法创建或打开当前缓存目录。"));
                }
            });
    connect(clearClipboardCacheButton_, &QPushButton::clicked, this,
            [this] {
                if (!clipboardController_) return;
                if (!RemoteCDialog::Confirm(
                        this, QStringLiteral("清理远程粘贴缓存"),
                        QStringLiteral(
                            "将删除所有未被当前剪贴板引用的缓存。\n"
                            "正在传输和当前剪贴板使用的文件会继续保留。"),
                        QStringLiteral("立即清理"),
                        QStringLiteral("取消"),
                        RemoteCDialog::Tone::kWarning)) {
                    return;
                }
                (void)clipboardController_->RequestCacheCleanup();
                RemoteCToast::Show(
                    this, QStringLiteral("正在后台清理远程粘贴缓存"),
                    RemoteCToast::Tone::kSuccess);
                for (const int delay : {750, 2000, 5000}) {
                    QTimer::singleShot(delay, this, [this] {
                        if (clipboardController_) {
                            (void)clipboardController_
                                ->RefreshCacheStatistics();
                        }
                    });
                }
            });
    connect(copyDebugButton_, &QPushButton::clicked, this, [this] {
        diagnosticsCopyTextRequested_ = true;
        if (engine_) {
            OnSessionEngineSnapshot(engine_->Snapshot());
            RefreshDiagnosticsUi();
        }
        QString text = debugCopyText_;
        if (!statsDebugCopyText_.isEmpty()) {
            if (!text.isEmpty()) {
                text += QStringLiteral("\n\n");
            }
            text += statsDebugCopyText_;
        }
        QApplication::clipboard()->setText(text);
        diagnosticsCopyTextRequested_ = false;
        RemoteCToast::Show(
            this, QStringLiteral("调试信息已复制"),
            RemoteCToast::Tone::kSuccess);
    });
    connect(copyMediaDebugButton_, &QPushButton::clicked, this, [this] {
        diagnosticsCopyTextRequested_ = true;
        if (engine_) {
            OnSessionEngineSnapshot(engine_->Snapshot());
        }
        QApplication::clipboard()->setText(mediaDebugCopyText_);
        diagnosticsCopyTextRequested_ = false;
        RemoteCToast::Show(
            this, QStringLiteral("媒体能力信息已复制"),
            RemoteCToast::Tone::kSuccess);
    });

    diagnosticsRefreshTimer_ = new QTimer(this);
    diagnosticsRefreshTimer_->setInterval(1000);
    connect(diagnosticsRefreshTimer_, &QTimer::timeout, this, [this] {
        RefreshDiagnosticsUi();
    });
    diagnosticsRefreshTimer_->start();
    QTimer::singleShot(0, this, [this] { RefreshDiagnosticsUi(); });

    connect(createRoomButton_, &QPushButton::clicked, this, [this] {
        const auto capacity = createRoomCapacity_->currentData().toUInt();
        const auto result = engine_->CreateRoom(capacity);
        if (!result.accepted) {
            SetRoomActionHint(
                QStringLiteral("创建房间失败：%1")
                    .arg(QString::fromStdString(result.errorMessage)),
                true);
        }
    });
    connect(joinRoomButton_, &QPushButton::clicked, this, [this] {
        const QString roomId = roomIdEdit_->text().trimmed();
        if (!IsNineDigitPublicId(roomId)) {
            roomIdEdit_->setFocus();
            SetRoomActionHint(QStringLiteral("请输入有效的 9 位房间号。"),
                              true);
            return;
        }
        const auto result = engine_->JoinRoom(roomId.toStdString());
        if (!result.accepted) {
            SetRoomActionHint(
                QStringLiteral("申请加入失败：%1")
                    .arg(QString::fromStdString(result.errorMessage)),
                true);
        }
    });
    connect(roomIdEdit_, &QLineEdit::returnPressed, joinRoomButton_,
            &QPushButton::click);
    connect(activeRoomCapacity_, &QComboBox::currentIndexChanged, this,
            [this](int) {
                const auto snapshot = engine_->Snapshot();
                const auto selected = activeRoomCapacity_->currentData().toUInt();
                const bool owner =
                    snapshot.room.ownerDeviceId == snapshot.localDeviceId;
                const bool valid =
                    selected >= static_cast<std::uint32_t>(
                                    snapshot.room.members.size()) &&
                    selected != snapshot.room.capacity;
                applyRoomCapacityButton_->setEnabled(
                    snapshot.connectivity == SessionConnectivityState::kOnline &&
                    snapshot.room.membership == RoomMembershipState::kActive &&
                    owner && valid);
            });
    connect(applyRoomCapacityButton_, &QPushButton::clicked, this, [this] {
        const auto snapshot = engine_->Snapshot();
        const auto capacity = activeRoomCapacity_->currentData().toUInt();
        if (capacity < static_cast<std::uint32_t>(
                           snapshot.room.members.size())) {
            SetRoomActionHint(QStringLiteral("人数上限不能小于当前房间人数。"),
                              true);
            return;
        }
        const auto result = engine_->SetRoomCapacity(capacity);
        if (!result.accepted) {
            SetRoomActionHint(
                QStringLiteral("调整人数上限失败：%1")
                    .arg(QString::fromStdString(result.errorMessage)),
                true);
            return;
        }
        applyRoomCapacityButton_->setEnabled(false);
        SetRoomActionHint(QStringLiteral("人数上限调整请求已发送。"));
        RemoteCToast::Show(
            this, QStringLiteral("人数上限调整请求已发送"),
            RemoteCToast::Tone::kSuccess);
    });
    connect(roomScreenShareButton_, &QPushButton::clicked, this, [this] {
        auto snapshot = engine_->Snapshot();
        const bool takeoverPending =
            !snapshot.outgoingRoomScreenShareSwitchRequestId.empty();
        const bool localOwns =
            snapshot.room.screenSharerDeviceId == snapshot.localDeviceId ||
            snapshot.room.pendingScreenSharerDeviceId ==
                snapshot.localDeviceId;
        SessionCommandResult result;
        if (takeoverPending) {
            result = engine_->CancelRoomScreenShareSwitch();
        } else if (localOwns) {
            result = engine_->StopRoomScreenShare();
        } else {
            if (!snapshot.room.screenSharerDeviceId.empty()) {
                const QString current = MemberDisplayName(
                    snapshot.room, snapshot.room.screenSharerDeviceId,
                    snapshot.localDeviceId);
                const QString prompt = QStringLiteral(
                    "%1 正在共享屏幕。切换到本机后，现有控制权会立即撤销，是否继续？")
                    .arg(current);
                if (!RemoteCDialog::Confirm(
                        this, QStringLiteral("切换主机器"), prompt,
                        QStringLiteral("切换共享"), QStringLiteral("取消"),
                        RemoteCDialog::Tone::kWarning)) {
                    return;
                }
            }
            const auto refresh = engine_->RefreshLocalDisplays();
            if (!refresh.accepted) {
                SetRoomActionHint(
                    QStringLiteral("读取显示器失败：%1")
                        .arg(QString::fromStdString(
                            refresh.errorMessage)),
                    true);
                return;
            }
            snapshot = engine_->Snapshot();
            const auto& displays =
                snapshot.localDisplayTopology.displays;
            if (displays.empty()) {
                SetRoomActionHint(
                    QStringLiteral("当前没有可共享的显示器。"), true);
                return;
            }

            std::string selectedKey;
            const std::string rememberedKey =
                QSettings()
                    .value(QStringLiteral(
                        "media/lastSharedDisplayKey"))
                    .toString()
                    .toStdString();
            if (FindDisplayByStableKey(
                    snapshot.localDisplayTopology, rememberedKey)) {
                selectedKey = rememberedKey;
            } else if (FindDisplayByStableKey(
                           snapshot.localDisplayTopology,
                           snapshot.selectedLocalDisplayKey)) {
                selectedKey = snapshot.selectedLocalDisplayKey;
            }

            if (displays.size() == 1) {
                selectedKey = displays.front().stableDisplayKey;
            } else {
                RoundedPopupMenu displayMenu(this);
                displayMenu.setObjectName(
                    QStringLiteral("displaySelectionMenu"));
                displayMenu.setStyleSheet(QStringLiteral(R"(
QMenu#displaySelectionMenu {
    background:#ffffff;
    border:1px solid #dce2eb;
    border-radius:12px;
    padding:7px;
    color:#263248;
    font-size:13px;
}
QMenu#displaySelectionMenu::item {
    min-width:250px;
    min-height:24px;
    border-radius:8px;
    padding:8px 36px 8px 14px;
}
QMenu#displaySelectionMenu::item:selected {
    background:#f3f6fb;
    color:#1769e8;
}
QMenu#displaySelectionMenu::item:checked {
    background:#e9f1ff;
    color:#1769e8;
    font-weight:700;
}
)"));
                if (darkInterfaceTheme_) {
                    displayMenu.setStyleSheet(
                        displayMenu.styleSheet() + QStringLiteral(R"(
QMenu#displaySelectionMenu {
    background:#151F2E; border-color:#34445B; color:#E4EBF5;
}
QMenu#displaySelectionMenu::item:selected {
    background:#202D40; color:#9BB4FF;
}
QMenu#displaySelectionMenu::item:checked {
    background:#263A61; color:#9BB4FF;
}
)"));
                }
                for (std::size_t index = 0;
                     index < displays.size(); ++index) {
                    const auto& display = displays[index];
                    const QString name = display.friendlyName.empty()
                        ? QStringLiteral("显示器 %1").arg(index + 1)
                        : QString::fromStdString(
                              display.friendlyName);
                    const QString label =
                        QStringLiteral("%1  ·  %2 × %3  ·  %4%5")
                            .arg(name)
                            .arg(display.width)
                            .arg(display.height)
                            .arg(display.scalePercent)
                            .arg(display.primary
                                     ? QStringLiteral("%（主显示器）")
                                     : QStringLiteral("%"));
                    QAction* action = displayMenu.addAction(label);
                    action->setCheckable(true);
                    action->setChecked(
                        display.stableDisplayKey == selectedKey);
                    action->setData(QString::fromStdString(
                        display.stableDisplayKey));
                }
                displayMenu.ensurePolished();
                displayMenu.adjustSize();
                const QSize popupSize = displayMenu.sizeHint();
                const QPoint anchorTop =
                    roomScreenShareButton_->mapToGlobal(QPoint(0, 0));
                QPoint popupPosition(
                    anchorTop.x() +
                        (roomScreenShareButton_->width() -
                         popupSize.width()) /
                            2,
                    anchorTop.y() - popupSize.height() - 6);
                if (QScreen* screen =
                        QGuiApplication::screenAt(anchorTop)) {
                    const QRect available =
                        screen->availableGeometry();
                    popupPosition.setX(std::clamp(
                        popupPosition.x(), available.left(),
                        (std::max)(
                            available.left(),
                            available.right() -
                                popupSize.width() + 1)));
                    popupPosition.setY((std::max)(
                        available.top(), popupPosition.y()));
                }
                QAction* selectedAction =
                    displayMenu.exec(popupPosition);
                if (!selectedAction) {
                    return;
                }
                selectedKey =
                    selectedAction->data().toString().toStdString();
            }

            const auto select = engine_->SelectRoomScreenShareDisplay(
                selectedKey);
            if (!select.accepted) {
                SetRoomActionHint(
                    QStringLiteral("选择显示器失败：%1")
                        .arg(QString::fromStdString(
                            select.errorMessage)),
                    true);
                return;
            }
            QSettings().setValue(
                QStringLiteral("media/lastSharedDisplayKey"),
                QString::fromStdString(selectedKey));
            result = engine_->StartRoomScreenShare();
        }
        if (!result.accepted) {
            SetRoomActionHint(
                QStringLiteral("屏幕分享操作失败：%1")
                    .arg(QString::fromStdString(result.errorMessage)),
                true);
            return;
        }
        RemoteCToast::ShowAbove(
            roomScreenShareButton_,
            takeoverPending
                ? QStringLiteral("已取消接替申请")
                : (localOwns ? QStringLiteral("屏幕共享已关闭")
                             : QStringLiteral("正在开启屏幕共享")),
            RemoteCToast::Tone::kSuccess);
    });
    connect(roomCameraButton_, &QPushButton::clicked, this, [this] {
        const auto snapshot = engine_->Snapshot();
        const bool enabled =
            snapshot.localCamera == LocalCameraState::kPublishing ||
            snapshot.localCamera == LocalCameraState::kStarting;
        // Reflect the user's action before starting camera initialization.
        // SetLocalCameraEnabled may synchronously touch the capture stack;
        // doing that first prevents Qt from painting this visual response.
        SetMediaStateButton(
            roomCameraButton_, MediaStateIcon::kCamera, !enabled,
            enabled ? QStringLiteral("开启摄像头")
                    : QStringLiteral("关闭摄像头"));
        roomCameraButton_->setEnabled(false);
        localCameraStopRequested_ = enabled;
        if (!enabled) {
            cameraGalleryManuallyHidden_ = false;
        }
        RemoteCToast::ShowAbove(
            roomCameraButton_,
            enabled ? QStringLiteral("正在关闭摄像头")
                    : QStringLiteral("正在开启摄像头"),
            RemoteCToast::Tone::kSuccess);
        if (!enabled) {
            if (auto* roomEngine =
                    dynamic_cast<app::InProcessSessionEngine*>(engine_.get())) {
                if (!roomCameraWindow_) {
                    roomCameraWindow_ =
                        new RoomCameraWindow(roomEngine, nullptr);
                    roomCameraWindow_->SetHiddenByUserCallback([this] {
                        cameraGalleryManuallyHidden_ = true;
                        if (engine_) UpdateRoomUi(engine_->Snapshot());
                    });
                }
                // Prepare the gallery while camera startup is pending, but do
                // not expose an empty top-level window yet. The authoritative
                // kStarting/kPublishing snapshot opens it exactly once.
                roomCameraWindow_->SyncSnapshot(engine_->Snapshot());
            }
        } else {
            const bool anotherCameraPublished = std::any_of(
                snapshot.room.members.begin(), snapshot.room.members.end(),
                [&snapshot](const RoomMemberSnapshot& member) {
                    return member.online && member.cameraPublishing &&
                        member.deviceId != snapshot.localDeviceId;
                });
            if (!anotherCameraPublished && roomCameraWindow_) {
                roomCameraWindow_->hide();
            }
        }

        // Leave enough time for Windows/Qt to present the optimistic button
        // state before camera startup work begins.
        QTimer::singleShot(24, this, [this, enabled] {
            const auto result = engine_->SetLocalCameraEnabled(!enabled);
            if (result.accepted) {
                return;
            }
            SetRoomActionHint(
                QStringLiteral("摄像头操作失败：%1")
                    .arg(QString::fromStdString(result.errorMessage)),
                true);
            localCameraStopRequested_ = false;
            // Restore every dependent button/window from the authoritative
            // engine state when the optimistic operation is rejected.
            OnSessionEngineSnapshot(engine_->Snapshot());
        });
    });
    connect(roomMicrophoneButton_, &QPushButton::clicked, this, [this] {
        const auto snapshot = engine_->Snapshot();
        const bool enabled =
            snapshot.localMicrophone == LocalMicrophoneState::kPublishing ||
            snapshot.localMicrophone == LocalMicrophoneState::kStarting;
        const auto result = engine_->SetLocalMicrophoneEnabled(!enabled);
        if (!result.accepted) {
            SetRoomActionHint(
                QStringLiteral("麦克风操作失败：%1")
                    .arg(QString::fromStdString(result.errorMessage)),
                true);
            return;
        }
        RemoteCToast::ShowAbove(
            roomMicrophoneButton_,
            enabled ? QStringLiteral("正在关闭麦克风")
                    : QStringLiteral("正在开启麦克风"),
            RemoteCToast::Tone::kSuccess);
    });
    connect(roomSpeakerButton_, &QPushButton::clicked, this, [this] {
        const auto snapshot = engine_->Snapshot();
        const auto result = engine_->SetRoomAudioPlaybackMuted(
            !snapshot.roomAudioPlaybackMuted);
        if (!result.accepted) {
            SetRoomActionHint(
                QStringLiteral("远端声音操作失败：%1")
                    .arg(QString::fromStdString(result.errorMessage)),
                true);
            return;
        }
        RemoteCToast::ShowAbove(
            roomSpeakerButton_,
            snapshot.roomAudioPlaybackMuted
                ? QStringLiteral("远端声音已开启")
                : QStringLiteral("远端声音已关闭"),
            RemoteCToast::Tone::kSuccess);
    });
    connect(roomCameraGalleryButton_, &QPushButton::clicked, this, [this] {
        auto* roomEngine = dynamic_cast<app::InProcessSessionEngine*>(
            engine_.get());
        if (!roomEngine) {
            return;
        }
        if (!roomCameraWindow_) {
            roomCameraWindow_ = new RoomCameraWindow(roomEngine, nullptr);
            roomCameraWindow_->SetHiddenByUserCallback([this] {
                cameraGalleryManuallyHidden_ = true;
                if (engine_) UpdateRoomUi(engine_->Snapshot());
            });
        }
        if (roomCameraWindow_->isVisible()) {
            cameraGalleryManuallyHidden_ = true;
            roomCameraWindow_->hide();
            UpdateRoomUi(engine_->Snapshot());
            RemoteCToast::ShowAbove(
                roomCameraGalleryButton_,
                QStringLiteral("摄像头画廊已隐藏"),
                RemoteCToast::Tone::kSuccess);
            return;
        }
        cameraGalleryManuallyHidden_ = false;
        roomCameraWindow_->SyncSnapshot(engine_->Snapshot());
        roomCameraWindow_->OpenBesideMainWindow(frameGeometry());
        UpdateRoomUi(engine_->Snapshot());
        RemoteCToast::ShowAbove(
            roomCameraGalleryButton_, QStringLiteral("摄像头画廊已打开"),
            RemoteCToast::Tone::kSuccess);
    });
    const auto openFileTransferWindow = [this] {
        if (!fileTransferWindow_) {
            return;
        }
        fileTransferWindow_->OpenBesideMainWindow(frameGeometry());
    };
    connect(roomFileTransferButton_, &QPushButton::clicked,
            this, openFileTransferWindow);
    connect(fileTransferNavButton_, &QPushButton::clicked,
            this, [this, openFileTransferWindow] {
                if (fileTransferWindow_ &&
                    fileTransferWindow_->isVisible() &&
                    !fileTransferWindow_->IsHiding()) {
                    fileTransferWindow_->HideWithAnimation();
                    return;
                }
                openFileTransferWindow();
            });
    connect(leaveRoomButton_, &QPushButton::clicked, this, [this] {
        const auto snapshot = engine_->Snapshot();
        const bool owner =
            snapshot.room.ownerDeviceId == snapshot.localDeviceId;
        const QString detail = owner
                                   ? QStringLiteral(
                                         "你是房主。离开后当前房间将关闭，确定继续吗？")
                                   : QStringLiteral("确定离开当前房间吗？");
        if (!RemoteCDialog::Confirm(
                this, QStringLiteral("离开房间"), detail,
                QStringLiteral("离开房间"), QStringLiteral("留在房间"),
                RemoteCDialog::Tone::kDanger)) {
            return;
        }
        const auto result = engine_->LeaveRoom();
        if (!result.accepted) {
            SetRoomActionHint(
                QStringLiteral("离开房间失败：%1")
                    .arg(QString::fromStdString(result.errorMessage)),
                true);
        }
    });
    connect(copyDeviceIdButton_, &QPushButton::clicked, this, [this] {
        QApplication::clipboard()->setText(localDeviceId_->text());
        copyDeviceIdButton_->setText(QStringLiteral("已复制"));
        if (auto* morph =
                remotec::ui::morph::MorphIconButtonBinding::attach(
                    copyDeviceIdButton_,
                    QStringLiteral(":/ui/icons/lucide/base/copy.svg"),
                    QStringLiteral(":/ui/icons/lucide/base/circle-check-big.svg"),
                    remotec::ui::morph::MorphIconButtonBinding::Interaction::Feedback,
                    QSize(15, 15), QColor(QStringLiteral("#2563EB")),
                    QColor(QStringLiteral("#12B76A")))) {
            morph->pulse(1300);
        }
        AnimateSmallUiChange(copyDeviceIdButton_);
        QTimer::singleShot(1300, copyDeviceIdButton_, [this] {
            if (copyDeviceIdButton_) {
                copyDeviceIdButton_->setText(QStringLiteral("复制"));
                AnimateSmallUiChange(copyDeviceIdButton_);
            }
        });
        RemoteCToast::Show(
            this, QStringLiteral("设备 ID 已复制"),
            RemoteCToast::Tone::kSuccess);
    });
    connect(copyRoomIdButton_, &QPushButton::clicked, this, [this] {
        const QString roomId = roomIdLabel_->text().trimmed();
        if (roomId.isEmpty() || roomId == QStringLiteral("—")) {
            return;
        }
        QApplication::clipboard()->setText(roomId);
        copyRoomIdButton_->setText(QStringLiteral("已复制"));
        if (auto* morph =
                remotec::ui::morph::MorphIconButtonBinding::attach(
                    copyRoomIdButton_,
                    QStringLiteral(":/ui/icons/lucide/base/copy.svg"),
                    QStringLiteral(":/ui/icons/lucide/base/circle-check-big.svg"),
                    remotec::ui::morph::MorphIconButtonBinding::Interaction::Feedback,
                    QSize(16, 16), QColor(QStringLiteral("#2563EB")),
                    QColor(QStringLiteral("#12B76A")))) {
            morph->pulse(1300);
        }
        AnimateSmallUiChange(copyRoomIdButton_);
        QTimer::singleShot(1300, copyRoomIdButton_, [this] {
            if (copyRoomIdButton_) {
                copyRoomIdButton_->setText(
                    QStringLiteral("复制房间 ID"));
                AnimateSmallUiChange(copyRoomIdButton_);
            }
        });
        RemoteCToast::Show(
            this, QStringLiteral("房间 ID 已复制"),
            RemoteCToast::Tone::kSuccess);
    });
    ApplyInterfaceTheme(false);
    RefreshRecentRooms();
    RefreshRecentDevices();
}
