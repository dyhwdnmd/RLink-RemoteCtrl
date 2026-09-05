// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

    QVBoxLayout* settingsPageLayout = nullptr;
    auto* settingsPage = MakePageSurface(
        QStringLiteral("设置"),
        QStringLiteral("按分类调整 RLink 在这台电脑上的行为。"),
        pageStack_, &settingsPageLayout);
    auto* settingsWorkspace = new QHBoxLayout();
    settingsWorkspace->setSpacing(20);

    auto* settingsCategories = new QFrame(settingsPage);
    settingsCategories->setObjectName(QStringLiteral("settingsCategoryPanel"));
    settingsCategories->setFixedWidth(120);
    settingsCategories->setFixedHeight(486);
    auto* categoryLayout = new QVBoxLayout(settingsCategories);
    categoryLayout->setContentsMargins(10, 12, 10, 12);
    categoryLayout->setSpacing(5);
    auto* categoryCaption = new QLabel(
        QStringLiteral("设置分类"), settingsCategories);
    categoryCaption->setProperty("muted", true);
    categoryCaption->setContentsMargins(10, 3, 0, 7);
    categoryLayout->addWidget(categoryCaption);

    auto* categoryGroup = new QButtonGroup(settingsPage);
    categoryGroup->setExclusive(true);
    const auto makeCategoryButton =
        [settingsCategories, categoryLayout, categoryGroup](
            const QString& text, int index) {
            auto* button = new QPushButton(text, settingsCategories);
            button->setObjectName(QStringLiteral("settingsCategoryButton"));
            button->setCheckable(true);
            button->setCursor(Qt::PointingHandCursor);
            button->setFixedHeight(58);
            categoryGroup->addButton(button, index);
            categoryLayout->addWidget(button);
            return button;
        };
    auto* generalCategoryButton = makeCategoryButton(
        QStringLiteral("常规"), 0);
    makeCategoryButton(QStringLiteral("远程桌面"), 1);
    makeCategoryButton(QStringLiteral("音视频设备"), 2);
    makeCategoryButton(QStringLiteral("文件传输"), 3);
    makeCategoryButton(QStringLiteral("远程粘贴"), 4);
    makeCategoryButton(QStringLiteral("快捷键"), 5);
    categoryLayout->addStretch(1);
    settingsWorkspace->addWidget(settingsCategories, 0, Qt::AlignTop);

    auto* settingsDetailStack = new QStackedWidget(settingsPage);
    settingsDetailStack->setObjectName(QStringLiteral("settingsDetailStack"));
    settingsDetailStack->setMinimumWidth(0);
    settingsDetailStack->setSizePolicy(
        QSizePolicy::Ignored, QSizePolicy::Expanding);
    settingsWorkspace->addWidget(settingsDetailStack, 1);

    const auto makeDetailHeader = [](QVBoxLayout* layout, QWidget* parent,
                                     const QString& title,
                                     const QString& description) {
        auto* titleLabel = new QLabel(title, parent);
        titleLabel->setObjectName(QStringLiteral("settingsDetailTitle"));
        layout->addWidget(titleLabel);
        auto* descriptionLabel = new QLabel(description, parent);
        descriptionLabel->setProperty("muted", true);
        descriptionLabel->setWordWrap(true);
        layout->addWidget(descriptionLabel);
        layout->addSpacing(8);
    };
    const auto makeSettingRow = [](QWidget* parent, const QString& title,
                                   const QString& description) {
        auto* row = new QFrame(parent);
        row->setObjectName(QStringLiteral("settingRow"));
        row->setFixedHeight(108);
        auto* rowLayout = new QHBoxLayout(row);
        rowLayout->setContentsMargins(20, 17, 20, 17);
        rowLayout->setSpacing(18);
        auto* textHost = new QWidget(row);
        textHost->setFixedWidth(390);
        auto* textLayout = new QVBoxLayout(textHost);
        textLayout->setContentsMargins(0, 0, 0, 0);
        textLayout->setSpacing(3);
        auto* titleLabel = new QLabel(title, row);
        titleLabel->setObjectName(QStringLiteral("settingTitle"));
        auto* hintLabel = new QLabel(description, row);
        hintLabel->setProperty("muted", true);
        hintLabel->setWordWrap(false);
        hintLabel->setToolTip(description);
        textLayout->addWidget(titleLabel);
        textLayout->addWidget(hintLabel);
        rowLayout->addWidget(textHost, 0, Qt::AlignVCenter);
        return std::pair<QFrame*, QHBoxLayout*>(row, rowLayout);
    };
    const auto makePathSettingRow = [](QWidget* parent,
                                       const QString& title,
                                       const QString& description) {
        auto* row = new QFrame(parent);
        row->setObjectName(QStringLiteral("settingRow"));
        row->setFixedHeight(132);
        auto* rowLayout = new QVBoxLayout(row);
        rowLayout->setContentsMargins(20, 15, 20, 15);
        rowLayout->setSpacing(9);
        auto* textHost = new QWidget(row);
        auto* textLayout = new QVBoxLayout(textHost);
        textLayout->setContentsMargins(0, 0, 0, 0);
        textLayout->setSpacing(3);
        auto* titleLabel = new QLabel(title, textHost);
        titleLabel->setObjectName(QStringLiteral("settingTitle"));
        auto* hintLabel = new QLabel(description, textHost);
        hintLabel->setProperty("muted", true);
        hintLabel->setWordWrap(true);
        textLayout->addWidget(titleLabel);
        textLayout->addWidget(hintLabel);
        rowLayout->addWidget(textHost);
        auto* pathLayout = new QHBoxLayout();
        pathLayout->setContentsMargins(0, 0, 0, 0);
        pathLayout->setSpacing(10);
        rowLayout->addLayout(pathLayout);
        return std::pair<QFrame*, QHBoxLayout*>(row, pathLayout);
    };

    const QSettings currentSettings;

    auto* generalSettingsPage = new QWidget(settingsDetailStack);
    auto* generalSettingsLayout = new QVBoxLayout(generalSettingsPage);
    generalSettingsLayout->setContentsMargins(0, 0, 0, 0);
    generalSettingsLayout->setSpacing(12);
    makeDetailHeader(
        generalSettingsLayout, generalSettingsPage,
        QStringLiteral("常规"),
        QStringLiteral("调整界面显示、交互反馈和本机配置。"));

    const auto [themeRow, themeLayout] = makeSettingRow(
        generalSettingsPage, QStringLiteral("界面主题"),
        QStringLiteral("选择浅色、深色，或随 Windows 外观自动切换。"));
    themeModeSelector_ = new RemoteCComboBox(themeRow);
    themeModeSelector_->setObjectName(QStringLiteral("capacitySelector"));
    themeModeSelector_->addItem(
        QStringLiteral("跟随系统（推荐）"), QStringLiteral("system"));
    themeModeSelector_->addItem(
        QStringLiteral("浅色"), QStringLiteral("light"));
    themeModeSelector_->addItem(
        QStringLiteral("深色"), QStringLiteral("dark"));
    const QString configuredTheme =
        ui::RemoteCTheme::PreferenceValue(
            ui::RemoteCTheme::LoadPreference());
    themeModeSelector_->setCurrentIndex(std::max(
        0, themeModeSelector_->findData(configuredTheme)));
    themeModeSelector_->setFixedWidth(kSettingsControlWidth);
    themeLayout->addWidget(themeModeSelector_, 0, Qt::AlignVCenter);
    generalSettingsLayout->addWidget(themeRow);

    const auto [motionRow, motionLayout] = makeSettingRow(
        generalSettingsPage, QStringLiteral("界面动画"),
        QStringLiteral("调整提示、弹窗和轻量面板的动画效果。"));
    animationLevelSelector_ = new RemoteCComboBox(motionRow);
    animationLevelSelector_->setObjectName(QStringLiteral("capacitySelector"));
    animationLevelSelector_->addItem(QStringLiteral("关闭"), 0);
    animationLevelSelector_->addItem(QStringLiteral("简洁"), 1);
    animationLevelSelector_->addItem(QStringLiteral("完整（推荐）"), 2);
    const int legacyAnimationLevel = currentSettings.value(
        QStringLiteral("ui/animationsEnabled"), true).toBool() ? 2 : 0;
    const int configuredAnimationLevel = std::clamp(
        currentSettings.value(QStringLiteral("ui/animationLevel"),
                              legacyAnimationLevel).toInt(),
        0, 2);
    animationLevelSelector_->setCurrentIndex(std::max(
        0, animationLevelSelector_->findData(configuredAnimationLevel)));
    animationLevelSelector_->setFixedWidth(kSettingsControlWidth);
    motionLayout->addWidget(animationLevelSelector_, 0, Qt::AlignVCenter);
    generalSettingsLayout->addWidget(motionRow);

    const auto [fontFamilyRow, fontFamilyLayout] = makeSettingRow(
        generalSettingsPage, QStringLiteral("界面字体"),
        QStringLiteral("使用这台电脑已安装且支持中文的字体。"));
    fontFamilySelector_ = new RemoteCComboBox(fontFamilyRow);
    fontFamilySelector_->setObjectName(QStringLiteral("capacitySelector"));
    fontFamilySelector_->addItem(
        QStringLiteral("默认（Microsoft YaHei UI）"), QString());
    const QStringList installedInterfaceFonts =
        InstalledChineseInterfaceFonts();
    for (const QString& family : installedInterfaceFonts) {
        fontFamilySelector_->addItem(family, family);
        fontFamilySelector_->setItemData(
            fontFamilySelector_->count() - 1, QFont(family), Qt::FontRole);
    }
    const QString configuredFontFamily = currentSettings.value(
        QString::fromLatin1(kInterfaceFontFamilySetting)).toString();
    const int configuredFontFamilyIndex =
        fontFamilySelector_->findData(configuredFontFamily);
    fontFamilySelector_->setCurrentIndex(
        configuredFontFamilyIndex >= 0 ? configuredFontFamilyIndex : 0);
    fontFamilySelector_->setFixedWidth(kSettingsControlWidth);
    fontFamilySelector_->setToolTip(
        QStringLiteral("列表来自 Windows 系统字体，不会向安装包添加中文字体文件。"));
    fontFamilyLayout->addWidget(
        fontFamilySelector_, 0, Qt::AlignVCenter);
    generalSettingsLayout->addWidget(fontFamilyRow);

    const auto [fontRow, fontLayout] = makeSettingRow(
        generalSettingsPage, QStringLiteral("字体大小"),
        QStringLiteral("调整主窗口和独立窗口的界面文字大小。"));
    fontSizeSelector_ = new RemoteCComboBox(fontRow);
    fontSizeSelector_->setObjectName(QStringLiteral("capacitySelector"));
    fontSizeSelector_->addItem(QStringLiteral("小"), 12);
    fontSizeSelector_->addItem(QStringLiteral("标准"), 13);
    fontSizeSelector_->addItem(QStringLiteral("大"), 15);
    fontSizeSelector_->addItem(QStringLiteral("特大"), 17);
    const int configuredFontSize = currentSettings.value(
        QStringLiteral("ui/fontPixelSize"), 13).toInt();
    fontSizeSelector_->setCurrentIndex(std::max(
        0, fontSizeSelector_->findData(configuredFontSize)));
    fontSizeSelector_->setFixedWidth(kSettingsControlWidth);
    fontLayout->addWidget(fontSizeSelector_, 0, Qt::AlignVCenter);
    generalSettingsLayout->addWidget(fontRow);

    const auto [autoStartRow, autoStartLayout] = makeSettingRow(
        generalSettingsPage, QStringLiteral("开机启动"),
        QStringLiteral("登录 Windows 后自动启动 RLink。"));
    autoStartSelector_ = new RemoteCComboBox(autoStartRow);
    autoStartSelector_->setObjectName(QStringLiteral("capacitySelector"));
    autoStartSelector_->addItem(
        QStringLiteral("关闭"), false);
    autoStartSelector_->addItem(
        QStringLiteral("开启"), true);
    autoStartSelector_->setCurrentIndex(std::max(
        0, autoStartSelector_->findData(WindowsAutoStartEnabled())));
    autoStartSelector_->setFixedWidth(kSettingsControlWidth);
    autoStartLayout->addWidget(
        autoStartSelector_, 0, Qt::AlignVCenter);
    generalSettingsLayout->addWidget(autoStartRow);

    const auto [startupRow, startupLayout] = makeSettingRow(
        generalSettingsPage, QStringLiteral("启动后"),
        QStringLiteral("选择显示主窗口，或仅在系统托盘中运行。"));
    startupVisibilitySelector_ = new RemoteCComboBox(startupRow);
    startupVisibilitySelector_->setObjectName(QStringLiteral("capacitySelector"));
    startupVisibilitySelector_->addItem(
        QStringLiteral("显示主窗口"), QStringLiteral("window"));
    startupVisibilitySelector_->addItem(
        QStringLiteral("在托盘中运行"), QStringLiteral("tray"));
    startupVisibilitySelector_->setCurrentIndex(std::max(
        0, startupVisibilitySelector_->findData(currentSettings.value(
            QString::fromLatin1(kStartupVisibilitySetting),
            QStringLiteral("window")).toString())));
    startupVisibilitySelector_->setFixedWidth(kSettingsControlWidth);
    startupLayout->addWidget(startupVisibilitySelector_, 0, Qt::AlignVCenter);
    generalSettingsLayout->addWidget(startupRow);

    const auto [closeBehaviorRow, closeBehaviorLayout] = makeSettingRow(
        generalSettingsPage, QStringLiteral("关闭主窗口"),
        QStringLiteral("选择隐藏到托盘，或直接退出 RLink。"));
    closeButtonBehaviorSelector_ = new RemoteCComboBox(closeBehaviorRow);
    closeButtonBehaviorSelector_->setObjectName(QStringLiteral("capacitySelector"));
    closeButtonBehaviorSelector_->addItem(
        QStringLiteral("隐藏到托盘（推荐）"), QStringLiteral("tray"));
    closeButtonBehaviorSelector_->addItem(
        QStringLiteral("退出程序"), QStringLiteral("exit"));
    closeButtonBehaviorSelector_->setCurrentIndex(std::max(
        0, closeButtonBehaviorSelector_->findData(currentSettings.value(
            QString::fromLatin1(kCloseButtonBehaviorSetting),
            QStringLiteral("tray")).toString())));
    closeButtonBehaviorSelector_->setFixedWidth(kSettingsControlWidth);
    closeBehaviorLayout->addWidget(
        closeButtonBehaviorSelector_, 0, Qt::AlignVCenter);
    generalSettingsLayout->addWidget(closeBehaviorRow);

    const auto [defaultCapacityRow, defaultCapacityLayout] = makeSettingRow(
        generalSettingsPage, QStringLiteral("默认房间人数"),
        QStringLiteral("设置创建新房间时默认选择的人数上限。"));
    defaultRoomCapacitySelector_ = new RemoteCComboBox(defaultCapacityRow);
    defaultRoomCapacitySelector_->setObjectName(QStringLiteral("capacitySelector"));
    AddRoomCapacityItems(defaultRoomCapacitySelector_);
    defaultRoomCapacitySelector_->setFixedWidth(kSettingsControlWidth);
    defaultCapacityLayout->addWidget(
        defaultRoomCapacitySelector_, 0, Qt::AlignVCenter);
    generalSettingsLayout->addWidget(defaultCapacityRow);

    auto* configRow = new QFrame(generalSettingsPage);
    configRow->setObjectName(QStringLiteral("settingRow"));
    auto* configLayout = new QVBoxLayout(configRow);
    configLayout->setContentsMargins(20, 17, 20, 17);
    configLayout->setSpacing(5);
    auto* configTitle = new QLabel(QStringLiteral("配置文件"), configRow);
    configTitle->setObjectName(QStringLiteral("settingTitle"));
    configLayout->addWidget(configTitle);
    auto* configPath = new QLabel(currentSettings.fileName(), configRow);
    configPath->setProperty("muted", true);
    configPath->setWordWrap(true);
    configPath->setTextInteractionFlags(Qt::TextSelectableByMouse);
    configLayout->addWidget(configPath);
    generalSettingsLayout->addWidget(configRow);
    generalSettingsLayout->addStretch(1);

    const QString installedVersion =
        QCoreApplication::applicationVersion().trimmed();
    if (!installedVersion.isEmpty()) {
        auto* aboutCard = new QFrame(generalSettingsPage);
        aboutCard->setObjectName(QStringLiteral("aboutCard"));
        auto* aboutLayout = new QHBoxLayout(aboutCard);
        aboutLayout->setContentsMargins(20, 16, 20, 16);
        aboutLayout->setSpacing(14);

        auto* aboutLogo = new QLabel(aboutCard);
        aboutLogo->setObjectName(QStringLiteral("aboutLogo"));
        aboutLogo->setFixedSize(48, 48);
        aboutLogo->setAlignment(Qt::AlignCenter);
        aboutLogo->setPixmap(CreateRemoteCIcon().pixmap(44, 44));
        aboutLayout->addWidget(aboutLogo, 0, Qt::AlignVCenter);

        auto* aboutTextHost = new QWidget(aboutCard);
        auto* aboutTextLayout = new QVBoxLayout(aboutTextHost);
        aboutTextLayout->setContentsMargins(0, 0, 0, 0);
        aboutTextLayout->setSpacing(4);
        auto* aboutTitle = new QLabel(
            QStringLiteral("关于 RLink"), aboutTextHost);
        aboutTitle->setObjectName(QStringLiteral("aboutTitle"));
        aboutTextLayout->addWidget(aboutTitle);
        softwareUpdateStatusLabel_ = new QLabel(
            QStringLiteral("当前安装版本由安装程序统一管理"),
            aboutTextHost);
        softwareUpdateStatusLabel_->setProperty("muted", true);
        aboutTextLayout->addWidget(softwareUpdateStatusLabel_);
        aboutLayout->addWidget(aboutTextHost, 1, Qt::AlignVCenter);

        auto* versionLabel = new QLabel(
            QStringLiteral("v%1").arg(installedVersion), aboutCard);
        versionLabel->setObjectName(QStringLiteral("aboutVersion"));
        versionLabel->setTextInteractionFlags(
            Qt::TextSelectableByMouse);
        aboutLayout->addWidget(versionLabel, 0, Qt::AlignVCenter);

        auto* copyVersionButton = new QPushButton(
            QStringLiteral("复制版本"), aboutCard);
        copyVersionButton->setObjectName(QStringLiteral("softButton"));
        copyVersionButton->setCursor(Qt::PointingHandCursor);
        aboutLayout->addWidget(
            copyVersionButton, 0, Qt::AlignVCenter);
        softwareUpdateCheckButton_ = new QPushButton(
            QStringLiteral("检查更新"), aboutCard);
        softwareUpdateCheckButton_->setObjectName(
            QStringLiteral("softButton"));
        softwareUpdateCheckButton_->setCursor(Qt::PointingHandCursor);
        aboutLayout->addWidget(
            softwareUpdateCheckButton_, 0, Qt::AlignVCenter);
        connect(softwareUpdateCheckButton_, &QPushButton::clicked,
                this, [this] { OpenSoftwareUpdate(); });
        connect(copyVersionButton, &QPushButton::clicked, this,
                [this, installedVersion] {
                    QApplication::clipboard()->setText(
                        QStringLiteral("RLink v%1")
                            .arg(installedVersion));
                    RemoteCToast::Show(
                        this, QStringLiteral("版本号已复制"),
                        RemoteCToast::Tone::kSuccess);
                });
        generalSettingsLayout->addWidget(aboutCard);
    }
    settingsDetailStack->addWidget(generalSettingsPage);

    auto* remoteSettingsPage = new QWidget(settingsDetailStack);
    auto* remoteSettingsLayout = new QVBoxLayout(remoteSettingsPage);
    remoteSettingsLayout->setContentsMargins(0, 0, 0, 0);
    remoteSettingsLayout->setSpacing(12);
    makeDetailHeader(
        remoteSettingsLayout, remoteSettingsPage,
        QStringLiteral("远程桌面"),
        QStringLiteral("选择屏幕采集、视频编解码和远端画面的渲染策略。"));

    const auto configurePreferenceSelector =
        [&currentSettings](QComboBox* selector,
                           const char* settingKey,
                           bool renderer) {
            selector->setObjectName(QStringLiteral("capacitySelector"));
            if (renderer) {
                selector->addItem(QStringLiteral("自动（D3D11 优先）"),
                                  QStringLiteral("auto"));
                selector->addItem(QStringLiteral("D3D11"),
                                  QStringLiteral("d3d11"));
                selector->addItem(QStringLiteral("CPU 兼容"),
                                  QStringLiteral("cpu"));
            } else {
                const bool decoder =
                    QByteArray(settingKey) ==
                    QByteArray(kVideoDecoderPreferenceSetting);
                selector->addItem(
                    decoder
                        ? QStringLiteral(
                              "自动（FFmpeg 硬件优先，软件回退）")
                        : QStringLiteral("自动（硬件优先）"),
                    QStringLiteral("auto"));
                selector->addItem(QStringLiteral("仅硬件"),
                                  QStringLiteral("hardware"));
                selector->addItem(QStringLiteral("仅软件"),
                                  QStringLiteral("software"));
            }
            const QString configured = currentSettings.value(
                QString::fromLatin1(settingKey),
                QStringLiteral("auto")).toString();
            selector->setCurrentIndex(std::max(
                0, selector->findData(configured)));
            selector->setFixedWidth(kSettingsControlWidth);
        };

    const auto [captureRow, captureLayout] = makeSettingRow(
        remoteSettingsPage, QStringLiteral("屏幕采集器"),
        QStringLiteral("选择屏幕采集方式。"));
    desktopCaptureSelector_ = new RemoteCComboBox(captureRow);
    desktopCaptureSelector_->setObjectName(
        QStringLiteral("capacitySelector"));
    desktopCaptureSelector_->addItem(
        QStringLiteral("libwebrtc（静态省带宽）"),
        QStringLiteral("libwebrtc"));
    desktopCaptureSelector_->addItem(
        QStringLiteral("自研 DXGI（动态低延迟）"),
        QStringLiteral("native_dxgi"));
    desktopCaptureSelector_->setItemData(
        0,
        QStringLiteral(
            "带宽要求低，支持最高 120 FPS，静止画面流量消耗小。"),
        Qt::ToolTipRole);
    desktopCaptureSelector_->setItemData(
        1,
        QStringLiteral(
            "带宽要求高，FPS 上限 120 FPS，流量消耗大。"),
        Qt::ToolTipRole);
    desktopCaptureSelector_->setCurrentIndex(std::max(
        0, desktopCaptureSelector_->findData(currentSettings.value(
            QString::fromLatin1(kDesktopCaptureBackendSetting),
            QStringLiteral("native_dxgi")).toString())));
    desktopCaptureSelector_->setFixedWidth(kSettingsControlWidth);
    const auto updateDesktopCaptureToolTip =
        [this](int) {
            desktopCaptureSelector_->setToolTip(
                desktopCaptureSelector_->currentData(
                    Qt::ToolTipRole).toString());
        };
    connect(desktopCaptureSelector_, &QComboBox::currentIndexChanged,
            this, updateDesktopCaptureToolTip);
    updateDesktopCaptureToolTip(desktopCaptureSelector_->currentIndex());
    captureLayout->addWidget(
        desktopCaptureSelector_, 0, Qt::AlignVCenter);
    remoteSettingsLayout->addWidget(captureRow);

    const auto [encoderRow, encoderLayout] = makeSettingRow(
        remoteSettingsPage, QStringLiteral("视频编码器"),
        QStringLiteral("选择发送屏幕和摄像头时的编码方式。"));
    videoEncoderSelector_ = new RemoteCComboBox(encoderRow);
    videoEncoderSelector_->addItem(
        QStringLiteral(
            "自动（首次 FFmpeg 硬件，检测后采用最优）"),
        QStringLiteral("auto"));
    videoEncoderSelector_->addItem(
        QStringLiteral("硬件编码（失败回退 FFmpeg/libx264）"),
        QStringLiteral("hardware"));
    videoEncoderSelector_->addItem(
        QStringLiteral("FFmpeg 硬件编码（失败回退 FFmpeg/libx264）"),
        QStringLiteral("ffmpeg_hardware"));
    videoEncoderSelector_->addItem(
        QStringLiteral("软件编码（OpenH264）"),
        QStringLiteral("software"));
    videoEncoderSelector_->addItem(
        QStringLiteral("软件编码（FFmpeg/libx264）"),
        QStringLiteral("ffmpeg"));
    videoEncoderSelector_->setCurrentIndex(std::max(
        0, videoEncoderSelector_->findData(currentSettings.value(
            QString::fromLatin1(kVideoEncoderPreferenceSetting),
            QStringLiteral("auto")).toString())));
    videoEncoderSelector_->setFixedWidth(kSettingsControlWidth);
    encoderLayout->addWidget(videoEncoderSelector_, 0, Qt::AlignVCenter);
    remoteSettingsLayout->addWidget(encoderRow);

    const auto [ffmpegHardwareRow, ffmpegHardwareLayout] = makeSettingRow(
        remoteSettingsPage, QStringLiteral("FFmpeg 硬件后端"),
        QStringLiteral(
            "自动模式按当前显示适配器和真实运行时能力选择；也可固定 QSV、NVENC 或 AMF 用于兼容性测试。"));
    ffmpegHardwareBackendSelector_ =
        new RemoteCComboBox(ffmpegHardwareRow);
    ffmpegHardwareBackendSelector_->addItem(
        QStringLiteral("自动选择（推荐）"), QStringLiteral("auto"));
    for (const auto& candidate :
         FfmpegHardwareH264Encoder::EnumerateAvailability()) {
        if (!candidate.compiled || !candidate.driverRuntimePresent) {
            continue;
        }
        const auto selfTest = RunFfmpegHardwareEncoderSelfTest(
            candidate.backend);
        if (!selfTest.succeeded) {
            continue;
        }
        switch (candidate.backend) {
        case FfmpegHardwareBackend::kQsv:
            ffmpegHardwareBackendSelector_->addItem(
                QStringLiteral("Intel Quick Sync（QSV）"),
                QStringLiteral("qsv"));
            break;
        case FfmpegHardwareBackend::kNvenc:
            ffmpegHardwareBackendSelector_->addItem(
                QStringLiteral("NVIDIA NVENC"),
                QStringLiteral("nvenc"));
            break;
        case FfmpegHardwareBackend::kAmf:
            ffmpegHardwareBackendSelector_->addItem(
                QStringLiteral("AMD AMF"), QStringLiteral("amf"));
            break;
        case FfmpegHardwareBackend::kAutomatic:
            break;
        }
    }
    const QString savedFfmpegHardwareBackend = currentSettings.value(
        QString::fromLatin1(kFfmpegHardwareBackendSetting),
        QStringLiteral("auto")).toString();
    const int savedFfmpegHardwareBackendIndex =
        ffmpegHardwareBackendSelector_->findData(
            savedFfmpegHardwareBackend);
    ffmpegHardwareBackendSelector_->setCurrentIndex(
        std::max(0, savedFfmpegHardwareBackendIndex));
    if (savedFfmpegHardwareBackendIndex < 0) {
        QSettings settings;
        settings.setValue(
            QString::fromLatin1(kFfmpegHardwareBackendSetting),
            QStringLiteral("auto"));
    }
    ffmpegHardwareBackendSelector_->setFixedWidth(kSettingsControlWidth);
    const auto updateFfmpegHardwareBackendEnabled = [this](int) {
        if (engine_) {
            UpdateVideoPipelineSettingsAvailability(engine_->Snapshot());
        }
    };
    connect(videoEncoderSelector_, &QComboBox::currentIndexChanged,
            this, updateFfmpegHardwareBackendEnabled);
    updateFfmpegHardwareBackendEnabled(videoEncoderSelector_->currentIndex());
    ffmpegHardwareLayout->addWidget(
        ffmpegHardwareBackendSelector_, 0, Qt::AlignVCenter);
    remoteSettingsLayout->addWidget(ffmpegHardwareRow);

    const auto [ffmpegPresetRow, ffmpegPresetLayout] = makeSettingRow(
        remoteSettingsPage, QStringLiteral("编码质量"),
        QStringLiteral(
            "编码质量越高，FPS 可能稍低；性能较差的电脑可适当降低编码质量。"));
    ffmpegX264PresetSelector_ = new RemoteCComboBox(ffmpegPresetRow);
    ffmpegX264PresetSelector_->addItem(
        QStringLiteral("极高"), QStringLiteral("veryslow"));
    ffmpegX264PresetSelector_->addItem(
        QStringLiteral("高"), QStringLiteral("slow"));
    ffmpegX264PresetSelector_->addItem(
        QStringLiteral("中（推荐）"), QStringLiteral("medium"));
    ffmpegX264PresetSelector_->addItem(
        QStringLiteral("低"), QStringLiteral("veryfast"));
    ffmpegX264PresetSelector_->addItem(
        QStringLiteral("极低"), QStringLiteral("ultrafast"));
    QString configuredEncoderQuality = currentSettings.value(
        QString::fromLatin1(kFfmpegX264PresetSetting),
        QStringLiteral("medium")).toString().toLower();
    // Map values from the former nine-level FFmpeg-only selector onto the
    // nearest shared quality level without silently changing the saved key.
    if (configuredEncoderQuality == QStringLiteral("superfast")) {
        configuredEncoderQuality = QStringLiteral("ultrafast");
    } else if (configuredEncoderQuality == QStringLiteral("faster") ||
               configuredEncoderQuality == QStringLiteral("fast")) {
        configuredEncoderQuality = QStringLiteral("veryfast");
    } else if (configuredEncoderQuality == QStringLiteral("slower")) {
        configuredEncoderQuality = QStringLiteral("slow");
    }
    ffmpegX264PresetSelector_->setCurrentIndex(std::max(
        0, ffmpegX264PresetSelector_->findData(
            configuredEncoderQuality)));
    ffmpegX264PresetSelector_->setFixedWidth(kSettingsControlWidth);
    ffmpegX264PresetSelector_->setToolTip(QStringLiteral(
        "极高：libx264 veryslow / NVENC p7 / QSV veryslow / AMF quality / MFT 100 / OpenH264 HIGH\n"
        "高：libx264 slow / NVENC p6 / QSV slow / AMF quality / MFT 75 / OpenH264 HIGH\n"
        "中：libx264 medium / NVENC p4 / QSV medium / AMF balanced / MFT 50 / OpenH264 MEDIUM\n"
        "低：libx264 veryfast / NVENC p2 / QSV faster / AMF speed / MFT 25 / OpenH264 LOW\n"
        "极低：libx264 ultrafast / NVENC p1 / QSV veryfast / AMF speed / MFT 0 / OpenH264 LOW"));
    ffmpegPresetLayout->addWidget(
        ffmpegX264PresetSelector_, 0, Qt::AlignVCenter);
    remoteSettingsLayout->addWidget(ffmpegPresetRow);

    auto* encoderProbeRow = new QFrame(remoteSettingsPage);
    encoderProbeRow->setObjectName(QStringLiteral("settingRow"));
    auto* encoderProbeControls = new QVBoxLayout(encoderProbeRow);
    encoderProbeControls->setContentsMargins(20, 17, 20, 17);
    encoderProbeControls->setSpacing(12);
    auto* encoderProbeHeader = new QHBoxLayout();
    encoderProbeHeader->setContentsMargins(0, 0, 0, 0);
    encoderProbeHeader->setSpacing(16);
    auto* encoderProbeHeading = new QWidget(encoderProbeRow);
    auto* encoderProbeHeadingLayout = new QVBoxLayout(encoderProbeHeading);
    encoderProbeHeadingLayout->setContentsMargins(0, 0, 0, 0);
    encoderProbeHeadingLayout->setSpacing(3);
    auto* encoderProbeTitle = new QLabel(
        QStringLiteral("编码器性能检测"), encoderProbeHeading);
    encoderProbeTitle->setObjectName(QStringLiteral("settingTitle"));
    auto* encoderProbeHint = new QLabel(
        QStringLiteral(
            "按当前采集方式实测 MFT、FFmpeg 硬件、OpenH264 与 libx264"),
        encoderProbeHeading);
    encoderProbeHint->setProperty("muted", true);
    encoderProbeHeadingLayout->addWidget(encoderProbeTitle);
    encoderProbeHeadingLayout->addWidget(encoderProbeHint);
    encoderProbeHeader->addWidget(encoderProbeHeading, 1);
    encoderBenchmarkButton_ = new QPushButton(
        QStringLiteral("重新检测"), encoderProbeRow);
    encoderBenchmarkButton_->setObjectName(QStringLiteral("softButton"));
    encoderBenchmarkButton_->setCursor(Qt::PointingHandCursor);
    encoderProbeHeader->addWidget(
        encoderBenchmarkButton_, 0, Qt::AlignTop);
    encoderProbeControls->addLayout(encoderProbeHeader);
    encoderBenchmarkSummary_ = new QLabel(encoderProbeRow);
    encoderBenchmarkSummary_->setObjectName(
        QStringLiteral("encoderBenchmarkSummary"));
    encoderBenchmarkSummary_->setTextFormat(Qt::RichText);
    encoderBenchmarkSummary_->setWordWrap(true);
    encoderBenchmarkSummary_->setSizePolicy(
        QSizePolicy::Preferred, QSizePolicy::Minimum);
    encoderProbeControls->addWidget(encoderBenchmarkSummary_);
    remoteSettingsLayout->addWidget(encoderProbeRow);
    RefreshEncoderBenchmarkSummary();

    const auto [decoderRow, decoderLayout] = makeSettingRow(
        remoteSettingsPage, QStringLiteral("视频解码器"),
        QStringLiteral("选择接收远端画面时的解码方式。"));
    videoDecoderSelector_ = new RemoteCComboBox(decoderRow);
    configurePreferenceSelector(videoDecoderSelector_,
                                kVideoDecoderPreferenceSetting, false);
    decoderLayout->addWidget(videoDecoderSelector_, 0, Qt::AlignVCenter);
    remoteSettingsLayout->addWidget(decoderRow);

    auto* decoderProbeRow = new QFrame(remoteSettingsPage);
    decoderProbeRow->setObjectName(QStringLiteral("settingRow"));
    auto* decoderProbeControls = new QVBoxLayout(decoderProbeRow);
    decoderProbeControls->setContentsMargins(20, 17, 20, 17);
    decoderProbeControls->setSpacing(12);
    auto* decoderProbeHeader = new QHBoxLayout();
    decoderProbeHeader->setContentsMargins(0, 0, 0, 0);
    decoderProbeHeader->setSpacing(16);
    auto* decoderProbeHeading = new QWidget(decoderProbeRow);
    auto* decoderProbeHeadingLayout = new QVBoxLayout(decoderProbeHeading);
    decoderProbeHeadingLayout->setContentsMargins(0, 0, 0, 0);
    decoderProbeHeadingLayout->setSpacing(3);
    auto* decoderProbeTitle = new QLabel(
        QStringLiteral("解码器性能检测"), decoderProbeHeading);
    decoderProbeTitle->setObjectName(QStringLiteral("settingTitle"));
    auto* decoderProbeHint = new QLabel(
        QStringLiteral("按 1920 × 1080、连续 60 FPS 与稀疏画面检测本机解码器；结果按当前硬件环境保存。"),
        decoderProbeHeading);
    decoderProbeHint->setProperty("muted", true);
    decoderProbeHeadingLayout->addWidget(decoderProbeTitle);
    decoderProbeHeadingLayout->addWidget(decoderProbeHint);
    decoderProbeHeader->addWidget(decoderProbeHeading, 1);
    decoderBenchmarkButton_ = new QPushButton(
        QStringLiteral("重新检测"), decoderProbeRow);
    decoderBenchmarkButton_->setObjectName(QStringLiteral("softButton"));
    decoderBenchmarkButton_->setCursor(Qt::PointingHandCursor);
    decoderProbeHeader->addWidget(
        decoderBenchmarkButton_, 0, Qt::AlignTop);
    decoderProbeControls->addLayout(decoderProbeHeader);
    decoderBenchmarkSummary_ = new QLabel(decoderProbeRow);
    decoderBenchmarkSummary_->setObjectName(
        QStringLiteral("decoderBenchmarkSummary"));
    decoderBenchmarkSummary_->setTextFormat(Qt::RichText);
    decoderBenchmarkSummary_->setWordWrap(true);
    decoderBenchmarkSummary_->setSizePolicy(
        QSizePolicy::Expanding, QSizePolicy::Minimum);
    decoderProbeControls->addWidget(decoderBenchmarkSummary_);
    remoteSettingsLayout->addWidget(decoderProbeRow);
    RefreshDecoderBenchmarkSummary();
    RefreshDecoderHardwareSelectionAvailability();

    const auto [rendererRow, rendererLayout] = makeSettingRow(
        remoteSettingsPage, QStringLiteral("画面渲染器"),
        QStringLiteral("选择远端画面的显示渲染方式。"));
    videoRendererSelector_ = new RemoteCComboBox(rendererRow);
    configurePreferenceSelector(videoRendererSelector_,
                                kVideoRendererPreferenceSetting, true);
    rendererLayout->addWidget(videoRendererSelector_, 0, Qt::AlignVCenter);
    remoteSettingsLayout->addWidget(rendererRow);

    const auto [dragSampleRateRow, dragSampleRateLayout] = makeSettingRow(
        remoteSettingsPage, QStringLiteral("拖动采样率"),
        QStringLiteral(
            "按住鼠标拖动时读取真实光标位置的频率；越高越丝滑，输入包也越多。"));
    dragPointerSampleRateSelector_ =
        new RemoteCComboBox(dragSampleRateRow);
    dragPointerSampleRateSelector_->setObjectName(
        QStringLiteral("capacitySelector"));
    for (const std::uint32_t hertz :
         { 240u, 170u, 120u, 80u, 60u }) {
        dragPointerSampleRateSelector_->addItem(
            hertz == 240
                ? QStringLiteral("%1 Hz（最丝滑）").arg(hertz)
                : QStringLiteral("%1 Hz").arg(hertz),
            QVariant::fromValue(hertz));
    }
    const std::uint32_t configuredDragSampleRate =
        currentSettings.value(
            QString::fromLatin1(kDragPointerSampleRateSetting),
            QVariant::fromValue(240u)).toUInt();
    dragPointerSampleRateSelector_->setCurrentIndex((std::max)(
        0, dragPointerSampleRateSelector_->findData(
            QVariant::fromValue(configuredDragSampleRate))));
    dragPointerSampleRateSelector_->setFixedWidth(
        kSettingsControlWidth);
    dragSampleRateLayout->addWidget(
        dragPointerSampleRateSelector_, 0, Qt::AlignVCenter);
    remoteSettingsLayout->addWidget(dragSampleRateRow);

    auto* mediaRestartHint = new QLabel(
        QStringLiteral("屏幕采集与视频编码模式在运行时创建时固定；修改后重启 RLink 生效。"),
        remoteSettingsPage);
    mediaRestartHint->setProperty("muted", true);
    remoteSettingsLayout->addWidget(mediaRestartHint);
    remoteSettingsLayout->addStretch(1);
    settingsDetailStack->addWidget(remoteSettingsPage);

    auto* audioVideoSettingsPage = new QWidget(settingsDetailStack);
    auto* audioVideoSettingsLayout = new QVBoxLayout(audioVideoSettingsPage);
    audioVideoSettingsLayout->setContentsMargins(0, 0, 0, 0);
    audioVideoSettingsLayout->setSpacing(12);
    makeDetailHeader(
        audioVideoSettingsLayout, audioVideoSettingsPage,
        QStringLiteral("音视频设备"),
        QStringLiteral("调整摄像头画廊与本机音视频窗口的显示方式。"));
    const auto [cameraGalleryRow, cameraGalleryLayout] = makeSettingRow(
        audioVideoSettingsPage, QStringLiteral("远端摄像头画廊"),
        QStringLiteral("选择成员开启摄像头时是否自动显示画廊。"));
    cameraGalleryBehaviorSelector_ = new RemoteCComboBox(cameraGalleryRow);
    cameraGalleryBehaviorSelector_->setObjectName(
        QStringLiteral("capacitySelector"));
    cameraGalleryBehaviorSelector_->addItem(
        QStringLiteral("自动显示（推荐）"), true);
    cameraGalleryBehaviorSelector_->addItem(
        QStringLiteral("仅手动打开"), false);
    cameraGalleryBehaviorSelector_->setCurrentIndex(std::max(
        0, cameraGalleryBehaviorSelector_->findData(currentSettings.value(
            QString::fromLatin1(kAutoOpenCameraGallerySetting),
            true).toBool())));
    cameraGalleryBehaviorSelector_->setFixedWidth(kSettingsControlWidth);
    cameraGalleryLayout->addWidget(
        cameraGalleryBehaviorSelector_, 0, Qt::AlignVCenter);
    audioVideoSettingsLayout->addWidget(cameraGalleryRow);

    const auto [cameraDeviceRow, cameraDeviceLayout] = makeSettingRow(
        audioVideoSettingsPage, QStringLiteral("本机摄像头"),
        QStringLiteral("选择开启本机摄像头时使用的设备，房间中可直接切换。"));
    cameraDeviceSelector_ = new RemoteCComboBox(cameraDeviceRow);
    cameraDeviceSelector_->addItem(
        QStringLiteral("正在读取设备…"),
        QStringLiteral("default"));
    cameraDeviceSelector_->setFixedWidth(kSettingsControlWidth);
    cameraDeviceSelector_->setEnabled(false);
    cameraDeviceLayout->addWidget(
        cameraDeviceSelector_, 0, Qt::AlignVCenter);
    audioVideoSettingsLayout->addWidget(cameraDeviceRow);

    const auto [microphoneDeviceRow, microphoneDeviceLayout] =
        makeSettingRow(
            audioVideoSettingsPage, QStringLiteral("本机麦克风"),
            QStringLiteral("选择房间麦克风使用的录音设备，切换时保持现有音频 Track。"));
    microphoneDeviceSelector_ =
        new RemoteCComboBox(microphoneDeviceRow);
    microphoneDeviceSelector_->addItem(
        QStringLiteral("正在读取设备…"),
        QStringLiteral("default"));
    microphoneDeviceSelector_->setFixedWidth(
        kSettingsControlWidth);
    microphoneDeviceSelector_->setEnabled(false);
    microphoneDeviceLayout->addWidget(
        microphoneDeviceSelector_, 0, Qt::AlignVCenter);
    audioVideoSettingsLayout->addWidget(microphoneDeviceRow);

    const auto [speakerDeviceRow, speakerDeviceLayout] =
        makeSettingRow(
            audioVideoSettingsPage, QStringLiteral("本机扬声器"),
            QStringLiteral("选择接收房间音频时使用的播放设备，可在会话中切换。"));
    speakerDeviceSelector_ =
        new RemoteCComboBox(speakerDeviceRow);
    speakerDeviceSelector_->addItem(
        QStringLiteral("正在读取设备…"),
        QStringLiteral("default"));
    speakerDeviceSelector_->setFixedWidth(
        kSettingsControlWidth);
    speakerDeviceSelector_->setEnabled(false);
    speakerDeviceLayout->addWidget(
        speakerDeviceSelector_, 0, Qt::AlignVCenter);
    audioVideoSettingsLayout->addWidget(speakerDeviceRow);

    auto* mediaDeviceFooter = new QFrame(audioVideoSettingsPage);
    mediaDeviceFooter->setObjectName(QStringLiteral("settingRow"));
    auto* mediaDeviceFooterLayout =
        new QHBoxLayout(mediaDeviceFooter);
    mediaDeviceFooterLayout->setContentsMargins(20, 14, 20, 14);
    mediaDeviceFooterLayout->setSpacing(12);
    mediaDeviceStatusLabel_ = new QLabel(
        QStringLiteral("正在读取本机音视频设备…"),
        mediaDeviceFooter);
    mediaDeviceStatusLabel_->setProperty("muted", true);
    mediaDeviceStatusLabel_->setWordWrap(true);
    mediaDeviceFooterLayout->addWidget(
        mediaDeviceStatusLabel_, 1);
    refreshMediaDevicesButton_ = new QPushButton(
        QStringLiteral("刷新设备"), mediaDeviceFooter);
    refreshMediaDevicesButton_->setObjectName(
        QStringLiteral("softButton"));
    refreshMediaDevicesButton_->setCursor(
        Qt::PointingHandCursor);
    mediaDeviceFooterLayout->addWidget(
        refreshMediaDevicesButton_, 0, Qt::AlignRight);
    audioVideoSettingsLayout->addWidget(mediaDeviceFooter);
    audioVideoSettingsLayout->addStretch(1);
    settingsDetailStack->addWidget(audioVideoSettingsPage);

    auto* fileSettingsPage = new QWidget(settingsDetailStack);
    auto* fileSettingsLayout = new QVBoxLayout(fileSettingsPage);
    fileSettingsLayout->setContentsMargins(0, 0, 0, 0);
    fileSettingsLayout->setSpacing(12);
    makeDetailHeader(
        fileSettingsLayout, fileSettingsPage,
        QStringLiteral("文件传输"),
        QStringLiteral("管理接收文件时使用的默认存储位置。"));
    const auto [saveDirectoryRow, saveDirectoryPathLayout] =
        makePathSettingRow(
        fileSettingsPage, QStringLiteral("文件默认存储路径"),
        QStringLiteral("点击“接受”时直接保存到此目录。"));
    auto* saveDirectoryPath = new QLabel(saveDirectoryRow);
    saveDirectoryPath->setProperty("muted", true);
    saveDirectoryPath->setWordWrap(true);
    saveDirectoryPath->setTextInteractionFlags(Qt::TextSelectableByMouse);
    const QString configuredSaveDirectory = QDir::cleanPath(
        currentSettings.value(
            QString::fromLatin1(kDefaultFileSaveDirectorySetting),
            InitialFileSaveDirectory()).toString());
    saveDirectoryPath->setText(QDir::toNativeSeparators(configuredSaveDirectory));
    saveDirectoryPath->setToolTip(saveDirectoryPath->text());
    saveDirectoryPath->setMinimumWidth(0);
    saveDirectoryPath->setSizePolicy(
        QSizePolicy::Expanding, QSizePolicy::Preferred);
    auto* chooseSaveDirectory = new QPushButton(
        QStringLiteral("选择路径"), saveDirectoryRow);
    chooseSaveDirectory->setObjectName(QStringLiteral("softButton"));
    chooseSaveDirectory->setCursor(Qt::PointingHandCursor);
    saveDirectoryPathLayout->addWidget(saveDirectoryPath, 1);
    saveDirectoryPathLayout->addWidget(chooseSaveDirectory);
    fileSettingsLayout->addWidget(saveDirectoryRow);
    fileSettingsLayout->addStretch(1);
    auto* fileSettingsScroll = new QScrollArea(settingsDetailStack);
    fileSettingsScroll->setObjectName(
        QStringLiteral("settingsDetailScroll"));
    fileSettingsScroll->setFrameShape(QFrame::NoFrame);
    fileSettingsScroll->setWidgetResizable(true);
    fileSettingsScroll->setHorizontalScrollBarPolicy(
        Qt::ScrollBarAlwaysOff);
    fileSettingsScroll->setWidget(fileSettingsPage);
    settingsDetailStack->addWidget(fileSettingsScroll);

    auto* remotePasteSettingsPage = new QWidget(settingsDetailStack);
    auto* remotePasteSettingsLayout =
        new QVBoxLayout(remotePasteSettingsPage);
    remotePasteSettingsLayout->setContentsMargins(0, 0, 0, 0);
    remotePasteSettingsLayout->setSpacing(12);
    makeDetailHeader(
        remotePasteSettingsLayout, remotePasteSettingsPage,
        QStringLiteral("远程粘贴"),
        QStringLiteral("管理远程粘贴内容、传输限制和临时缓存。"));

    const auto [remotePasteEnabledRow, remotePasteEnabledLayout] =
        makeSettingRow(
            remotePasteSettingsPage, QStringLiteral("远程粘贴"),
            QStringLiteral(
                "仅在指定粘贴动作发生时传输，不会持续同步剪贴板正文。"));
    remotePasteEnabledSelector_ =
        new RemoteCComboBox(remotePasteEnabledRow);
    remotePasteEnabledSelector_->setObjectName(
        QStringLiteral("capacitySelector"));
    remotePasteEnabledSelector_->addItem(QStringLiteral("开启"), true);
    remotePasteEnabledSelector_->addItem(QStringLiteral("关闭"), false);
    remotePasteEnabledSelector_->setCurrentIndex((std::max)(
        0, remotePasteEnabledSelector_->findData(
            currentSettings.value(
                QString::fromLatin1(kRemotePasteEnabledSetting),
                true).toBool())));
    remotePasteEnabledSelector_->setFixedWidth(kSettingsControlWidth);
    remotePasteEnabledLayout->addWidget(
        remotePasteEnabledSelector_, 0, Qt::AlignVCenter);
    remotePasteSettingsLayout->addWidget(remotePasteEnabledRow);

    const auto [clipboardFormatsRow, clipboardFormatsLayout] =
        makeSettingRow(
            remotePasteSettingsPage, QStringLiteral("远程粘贴内容"),
            QStringLiteral("选择远程粘贴允许传输的内容格式。"));
    clipboardFormatsSelector_ = new RemoteCComboBox(clipboardFormatsRow);
    clipboardFormatsSelector_->setObjectName(
        QStringLiteral("capacitySelector"));
    clipboardFormatsSelector_->addItem(
        QStringLiteral("文本、富文本、图片和文件"),
        QStringLiteral("all"));
    clipboardFormatsSelector_->addItem(
        QStringLiteral("文本、富文本和图片"),
        QStringLiteral("no_files"));
    clipboardFormatsSelector_->addItem(
        QStringLiteral("文本和富文本"),
        QStringLiteral("rich_text"));
    clipboardFormatsSelector_->addItem(
        QStringLiteral("仅纯文本"), QStringLiteral("text"));
    clipboardFormatsSelector_->setCurrentIndex((std::max)(
        0, clipboardFormatsSelector_->findData(
            currentSettings.value(
                QString::fromLatin1(kClipboardFormatsSetting),
                QStringLiteral("all")).toString())));
    clipboardFormatsSelector_->setFixedWidth(kSettingsControlWidth);
    clipboardFormatsLayout->addWidget(
        clipboardFormatsSelector_, 0, Qt::AlignVCenter);
    remotePasteSettingsLayout->addWidget(clipboardFormatsRow);

    const auto [clipboardFileLimitRow, clipboardFileLimitLayout] =
        makeSettingRow(
            remotePasteSettingsPage, QStringLiteral("单次文件粘贴上限"),
            QStringLiteral("超过上限的文件或目录会被拒绝，避免误传大目录。"));
    clipboardLargeFileLimitSelector_ =
        new RemoteCComboBox(clipboardFileLimitRow);
    clipboardLargeFileLimitSelector_->setObjectName(
        QStringLiteral("capacitySelector"));
    for (const int mebibytes : {25, 100, 512}) {
        clipboardLargeFileLimitSelector_->addItem(
            QStringLiteral("%1 MiB").arg(mebibytes),
            mebibytes);
    }
    const int configuredClipboardFileLimitMiB = (std::min)(
        currentSettings.value(
            QString::fromLatin1(kClipboardFileLimitSetting),
            100).toInt(),
        512);
    clipboardLargeFileLimitSelector_->setCurrentIndex((std::max)(
        0, clipboardLargeFileLimitSelector_->findData(
            configuredClipboardFileLimitMiB)));
    clipboardLargeFileLimitSelector_->setFixedWidth(kSettingsControlWidth);
    clipboardFileLimitLayout->addWidget(
        clipboardLargeFileLimitSelector_, 0, Qt::AlignVCenter);
    remotePasteSettingsLayout->addWidget(clipboardFileLimitRow);

    auto* clipboardCacheSection = new QLabel(
        QStringLiteral("远程粘贴缓存"), remotePasteSettingsPage);
    clipboardCacheSection->setObjectName(
        QStringLiteral("settingsDetailTitle"));
    remotePasteSettingsLayout->addSpacing(6);
    remotePasteSettingsLayout->addWidget(clipboardCacheSection);

    const QString configuredCacheBase = QDir::cleanPath(
        currentSettings.value(
            QString::fromLatin1(kClipboardCacheBaseDirectorySetting),
            InitialClipboardCacheBaseDirectory()).toString());
    const auto [cachePathRow, cachePathContentLayout] =
        makePathSettingRow(
        remotePasteSettingsPage, QStringLiteral("缓存存储位置"),
        QStringLiteral(
            "选择存储磁盘；RLink 会在其中创建专用缓存目录。"));
    clipboardCachePathLabel_ = new QLabel(cachePathRow);
    clipboardCachePathLabel_->setProperty("muted", true);
    clipboardCachePathLabel_->setWordWrap(true);
    clipboardCachePathLabel_->setAlignment(
        Qt::AlignLeft | Qt::AlignVCenter);
    clipboardCachePathLabel_->setSizePolicy(
        QSizePolicy::Expanding, QSizePolicy::Preferred);
    clipboardCachePathLabel_->setTextInteractionFlags(
        Qt::TextSelectableByMouse);
    clipboardCachePathLabel_->setText(QDir::toNativeSeparators(
        ClipboardCacheRootForBase(configuredCacheBase)));
    clipboardCachePathLabel_->setToolTip(
        clipboardCachePathLabel_->text());
    auto* cachePathButtons = new QHBoxLayout();
    cachePathButtons->setSpacing(8);
    auto* resetClipboardCacheDirectory = new QPushButton(
        QStringLiteral("恢复默认"), cachePathRow);
    resetClipboardCacheDirectory->setObjectName(
        QStringLiteral("softButton"));
    resetClipboardCacheDirectory->setCursor(Qt::PointingHandCursor);
    auto* chooseClipboardCacheDirectory = new QPushButton(
        QStringLiteral("选择路径"), cachePathRow);
    chooseClipboardCacheDirectory->setObjectName(
        QStringLiteral("softButton"));
    chooseClipboardCacheDirectory->setCursor(Qt::PointingHandCursor);
    auto* openClipboardCacheDirectory = new QPushButton(
        QStringLiteral("打开缓存路径"), cachePathRow);
    openClipboardCacheDirectory->setObjectName(
        QStringLiteral("softButton"));
    openClipboardCacheDirectory->setCursor(Qt::PointingHandCursor);
    cachePathButtons->addWidget(resetClipboardCacheDirectory);
    cachePathButtons->addWidget(chooseClipboardCacheDirectory);
    cachePathButtons->addWidget(openClipboardCacheDirectory);
    cachePathContentLayout->addWidget(clipboardCachePathLabel_, 1);
    cachePathContentLayout->addLayout(cachePathButtons);
    remotePasteSettingsLayout->addWidget(cachePathRow);

    const auto [cacheRetentionRow, cacheRetentionLayout] = makeSettingRow(
        remotePasteSettingsPage, QStringLiteral("缓存保留时间"),
        QStringLiteral(
            "当前剪贴板引用和正在传输的文件不受保留时间影响。"));
    clipboardCacheRetentionSelector_ =
        new RemoteCComboBox(cacheRetentionRow);
    clipboardCacheRetentionSelector_->setObjectName(
        QStringLiteral("capacitySelector"));
    for (const auto [minutes, label] : {
            std::pair{30, QStringLiteral("30 分钟")},
            std::pair{60, QStringLiteral("1 小时（推荐）")},
            std::pair{360, QStringLiteral("6 小时")},
            std::pair{1440, QStringLiteral("24 小时")} }) {
        clipboardCacheRetentionSelector_->addItem(label, minutes);
    }
    clipboardCacheRetentionSelector_->setCurrentIndex((std::max)(
        0, clipboardCacheRetentionSelector_->findData(
            currentSettings.value(
                QString::fromLatin1(kClipboardCacheRetentionSetting),
                60).toInt())));
    clipboardCacheRetentionSelector_->setFixedWidth(kSettingsControlWidth);
    cacheRetentionLayout->addWidget(
        clipboardCacheRetentionSelector_, 0, Qt::AlignVCenter);
    remotePasteSettingsLayout->addWidget(cacheRetentionRow);

    const auto [cacheCapacityRow, cacheCapacityLayout] = makeSettingRow(
        remotePasteSettingsPage, QStringLiteral("缓存空间上限"),
        QStringLiteral(
            "最大不超过所选磁盘可用空间的 50%，每次接收前都会复查。"));
    clipboardCacheCapacitySelector_ =
        new RemoteCComboBox(cacheCapacityRow);
    clipboardCacheCapacitySelector_->setObjectName(
        QStringLiteral("capacitySelector"));
    clipboardCacheCapacitySelector_->setFixedWidth(kSettingsControlWidth);
    const auto refreshClipboardCacheCapacityOptions =
        [this](const QString& baseDirectory, qulonglong preferredGiB) {
            QSignalBlocker blocker(clipboardCacheCapacitySelector_);
            clipboardCacheCapacitySelector_->clear();
            const qulonglong safeGiB =
                SafeClipboardCacheCapacityGiB(baseDirectory);
            for (const qulonglong capacity : {1ull, 2ull, 5ull, 10ull}) {
                if (capacity > safeGiB) continue;
                const bool recommended =
                    capacity == (std::min)(2ull, safeGiB);
                clipboardCacheCapacitySelector_->addItem(
                    recommended
                        ? QStringLiteral("%1 GiB（推荐）").arg(capacity)
                        : QStringLiteral("%1 GiB").arg(capacity),
                    QVariant::fromValue(capacity));
            }
            if (safeGiB > 10) {
                clipboardCacheCapacitySelector_->addItem(
                    QStringLiteral("不限制（安全上限 %1 GiB）")
                        .arg(safeGiB),
                    QVariant::fromValue(safeGiB));
            }
            if (clipboardCacheCapacitySelector_->count() == 0) {
                clipboardCacheCapacitySelector_->addItem(
                    QStringLiteral("空间不足（至少需 2 GiB 可用）"),
                    QVariant::fromValue(0ull));
                clipboardCacheCapacitySelector_->setEnabled(false);
                return;
            }
            clipboardCacheCapacitySelector_->setEnabled(true);
            int index = clipboardCacheCapacitySelector_->findData(
                QVariant::fromValue(preferredGiB));
            if (index < 0) {
                index = clipboardCacheCapacitySelector_->count() - 1;
            }
            clipboardCacheCapacitySelector_->setCurrentIndex(index);
        };
    refreshClipboardCacheCapacityOptions(
        configuredCacheBase,
        currentSettings.value(
            QString::fromLatin1(kClipboardCacheCapacitySetting),
            2).toULongLong());
    cacheCapacityLayout->addWidget(
        clipboardCacheCapacitySelector_, 0, Qt::AlignVCenter);
    remotePasteSettingsLayout->addWidget(cacheCapacityRow);

    const auto [cacheUsageRow, cacheUsageLayout] = makeSettingRow(
        remotePasteSettingsPage, QStringLiteral("当前缓存"),
        QStringLiteral(
            "清理全部非活动项目；当前剪贴板和正在传输的文件会继续保留。"));
    clipboardCacheUsageLabel_ = new QLabel(
        QStringLiteral("正在统计…"), cacheUsageRow);
    clipboardCacheUsageLabel_->setProperty("muted", true);
    clearClipboardCacheButton_ = new QPushButton(
        QStringLiteral("立即清理"), cacheUsageRow);
    clearClipboardCacheButton_->setObjectName(QStringLiteral("softButton"));
    clearClipboardCacheButton_->setCursor(Qt::PointingHandCursor);
    clearClipboardCacheButton_->setEnabled(false);
    auto* cacheUsageControlHost = new QWidget(cacheUsageRow);
    cacheUsageControlHost->setFixedWidth(kSettingsControlWidth);
    auto* cacheUsageControls = new QVBoxLayout(cacheUsageControlHost);
    cacheUsageControls->setContentsMargins(0, 0, 0, 0);
    cacheUsageControls->setSpacing(8);
    cacheUsageControls->addWidget(
        clipboardCacheUsageLabel_, 0, Qt::AlignLeft);
    cacheUsageControls->addWidget(
        clearClipboardCacheButton_, 0, Qt::AlignRight);
    cacheUsageLayout->addWidget(
        cacheUsageControlHost, 0, Qt::AlignVCenter);
    remotePasteSettingsLayout->addWidget(cacheUsageRow);
    remotePasteSettingsLayout->addStretch(1);
    auto* remotePasteSettingsScroll = new QScrollArea(settingsDetailStack);
    remotePasteSettingsScroll->setObjectName(
        QStringLiteral("settingsDetailScroll"));
    remotePasteSettingsScroll->setFrameShape(QFrame::NoFrame);
    remotePasteSettingsScroll->setWidgetResizable(true);
    remotePasteSettingsScroll->setHorizontalScrollBarPolicy(
        Qt::ScrollBarAlwaysOff);
    remotePasteSettingsScroll->setWidget(remotePasteSettingsPage);
    settingsDetailStack->addWidget(remotePasteSettingsScroll);

    auto* shortcutSettingsPage = new QWidget(settingsDetailStack);
    auto* shortcutSettingsLayout = new QVBoxLayout(shortcutSettingsPage);
    shortcutSettingsLayout->setContentsMargins(0, 0, 0, 0);
    shortcutSettingsLayout->setSpacing(12);
    makeDetailHeader(
        shortcutSettingsLayout, shortcutSettingsPage,
        QStringLiteral("快捷键"),
        QStringLiteral("查看 RLink 当前可用的本地快捷操作。"));
    const auto addShortcutRow =
        [&makeSettingRow, shortcutSettingsPage,
         shortcutSettingsLayout](const QString& title,
                                 const QString& description,
                                 const QString& keys) {
            const auto [row, rowLayout] = makeSettingRow(
                shortcutSettingsPage, title, description);
            auto* badge = new QLabel(keys, row);
            badge->setObjectName(QStringLiteral("aboutVersion"));
            badge->setAlignment(Qt::AlignCenter);
            badge->setFixedWidth(132);
            badge->setTextInteractionFlags(Qt::TextSelectableByMouse);
            rowLayout->addWidget(
                badge, 0, Qt::AlignRight | Qt::AlignVCenter);
            shortcutSettingsLayout->addWidget(row);
        };
    addShortcutRow(
        QStringLiteral("切换全屏"),
        QStringLiteral("在监控窗口中进入或退出全屏显示。"),
        QStringLiteral("F11"));
    addShortcutRow(
        QStringLiteral("平移原始像素画面"),
        QStringLiteral("仅在 100% 原始像素模式下生效。"),
        QStringLiteral("Alt + 左键拖动"));
    addShortcutRow(
        QStringLiteral("远程粘贴"),
        QStringLiteral("控制远端时粘贴本机剪贴板中的内容。"),
        QStringLiteral("Ctrl + V"));
    addShortcutRow(
        QStringLiteral("远程复制"),
        QStringLiteral("控制远端时将复制命令发送给被控端。"),
        QStringLiteral("Ctrl + C"));
    shortcutSettingsLayout->addStretch(1);
    settingsDetailStack->addWidget(shortcutSettingsPage);

    connect(categoryGroup, &QButtonGroup::idClicked, this,
            [this, settingsDetailStack,
             refreshClipboardCacheCapacityOptions](int index) {
                settingsDetailStack->setCurrentIndex(index);
                if (index == 2) {
                    RequestMediaDeviceRefresh(false);
                } else if (index == 4) {
                    QSettings settings;
                    const QString base = settings.value(
                        QString::fromLatin1(
                            kClipboardCacheBaseDirectorySetting),
                        InitialClipboardCacheBaseDirectory()).toString();
                    const qulonglong preferred =
                        clipboardCacheCapacitySelector_->currentData()
                            .toULongLong();
                    refreshClipboardCacheCapacityOptions(
                        base, preferred);
                    ApplyClipboardConfigurationFromUi(false);
                    if (clipboardController_) {
                        (void)clipboardController_
                            ->RefreshCacheStatistics();
                    }
                }
            });
    generalCategoryButton->setChecked(true);
    settingsDetailStack->setCurrentIndex(0);
    // Let wheel events continue to the settings scroll area unless a combo's
    // popup is open, preventing accidental value changes while scrolling.
    const auto settingsSelectors = settingsPage->findChildren<QComboBox*>();
    for (auto* selector : settingsSelectors) {
        if (auto* remoteSelector =
                dynamic_cast<RemoteCComboBox*>(selector)) {
            remoteSelector->SetWheelSelectionEnabled(false);
        }
    }
    settingsPageLayout->addLayout(settingsWorkspace, 1);
    pageStack_->addWidget(settingsPage);
