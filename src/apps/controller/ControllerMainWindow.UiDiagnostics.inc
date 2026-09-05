// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

    QVBoxLayout* debugPageLayout = nullptr;
    auto* debugPage = MakePageSurface(
        QStringLiteral("调试信息"),
        QStringLiteral("按分类查看运行状态和媒体能力；反馈问题时可以一键复制。"),
        pageStack_, &debugPageLayout);
    debugPage_ = debugPage;
    auto* debugToolbar = new QHBoxLayout();
    auto* debugNotice = new QLabel(
        QStringLiteral("状态会随信令、房间和成员连接自动更新"), debugPage);
    debugNotice->setProperty("muted", true);
    debugToolbar->addWidget(debugNotice);
    debugToolbar->addStretch(1);
    screenFrameRateLogEnabled_ = QSettings().value(
        QString::fromLatin1(
            kScreenFrameRateLogEnabledSetting),
        false).toBool();
    screenFrameRateLogButton_ = new QPushButton(debugPage);
    screenFrameRateLogButton_->setObjectName(
        QStringLiteral("screenFrameRateLogButton"));
    screenFrameRateLogButton_->setCheckable(true);
    screenFrameRateLogButton_->setChecked(
        screenFrameRateLogEnabled_);
    screenFrameRateLogButton_->setCursor(
        Qt::PointingHandCursor);
    const auto updateFrameRateLogButton =
        [this] {
            if (!screenFrameRateLogButton_) {
                return;
            }
            screenFrameRateLogButton_->setText(
                screenFrameRateLogEnabled_
                    ? QStringLiteral("FPS 日志：已开启")
                    : QStringLiteral("FPS 日志：已关闭"));
            screenFrameRateLogButton_->setToolTip(
                screenFrameRateLogEnabled_
                    ? QStringLiteral(
                          "正在向本机应用数据目录写入 screen-frame-rate-进程ID.csv，点击关闭")
                    : QStringLiteral(
                          "默认关闭；点击后开始记录采集、编码、发送和显示帧率"));
        };
    updateFrameRateLogButton();
    connect(
        screenFrameRateLogButton_,
        &QPushButton::toggled,
        this,
        [this, updateFrameRateLogButton](bool enabled) {
            screenFrameRateLogEnabled_ = enabled;
            QSettings settings;
            settings.setValue(
                QString::fromLatin1(
                    kScreenFrameRateLogEnabledSetting),
                enabled);
            updateFrameRateLogButton();
            RemoteCToast::Show(
                this,
                enabled
                    ? QStringLiteral(
                          "FPS记录日志已开启，将写入程序目录")
                    : QStringLiteral(
                          "FPS记录日志已关闭"),
                RemoteCToast::Tone::kInformation);
        });
    debugToolbar->addWidget(screenFrameRateLogButton_);
    copyDebugButton_ = new QPushButton(
        QStringLiteral("复制调试信息"), debugPage);
    copyDebugButton_->setObjectName(QStringLiteral("softButton"));
    copyDebugButton_->setCursor(Qt::PointingHandCursor);
    debugToolbar->addWidget(copyDebugButton_);
    debugPageLayout->addLayout(debugToolbar);

    auto* debugWorkspace = new QHBoxLayout();
    debugWorkspace->setSpacing(20);
    auto* debugCategories = new QFrame(debugPage);
    debugCategories->setObjectName(QStringLiteral("settingsCategoryPanel"));
    debugCategories->setFixedWidth(190);
    auto* debugCategoryLayout = new QVBoxLayout(debugCategories);
    debugCategoryLayout->setContentsMargins(10, 12, 10, 12);
    debugCategoryLayout->setSpacing(5);
    auto* debugCategoryCaption = new QLabel(
        QStringLiteral("信息分类"), debugCategories);
    debugCategoryCaption->setProperty("muted", true);
    debugCategoryCaption->setContentsMargins(10, 3, 0, 7);
    debugCategoryLayout->addWidget(debugCategoryCaption);

    auto* debugCategoryGroup = new QButtonGroup(debugPage);
    debugCategoryGroup->setExclusive(true);
    const auto makeDebugCategoryButton =
        [debugCategories, debugCategoryLayout, debugCategoryGroup](
            const QString& text, int index) {
            auto* button = new QPushButton(text, debugCategories);
            button->setObjectName(
                QStringLiteral("settingsCategoryButton"));
            button->setCheckable(true);
            button->setCursor(Qt::PointingHandCursor);
            button->setMinimumHeight(46);
            debugCategoryGroup->addButton(button, index);
            debugCategoryLayout->addWidget(button);
            return button;
        };
    auto* firstDebugCategory = makeDebugCategoryButton(
        QStringLiteral("设备与信令"), 0);
    makeDebugCategoryButton(QStringLiteral("媒体能力"), 1);
    makeDebugCategoryButton(QStringLiteral("连接质量"), 2);
    makeDebugCategoryButton(QStringLiteral("房间状态"), 3);
    makeDebugCategoryButton(QStringLiteral("成员连接"), 4);
    makeDebugCategoryButton(QStringLiteral("鼠标与键盘"), 5);
    makeDebugCategoryButton(QStringLiteral("远程粘贴"), 6);
    makeDebugCategoryButton(QStringLiteral("最近错误"), 7);
    debugCategoryLayout->addStretch(1);
    debugWorkspace->addWidget(debugCategories, 0, Qt::AlignTop);

    auto* debugDetailStack = new QStackedWidget(debugPage);
    debugDetailStack->setObjectName(QStringLiteral("settingsDetailStack"));
    debugWorkspace->addWidget(debugDetailStack, 1);

    const auto makeDebugDetailPage =
        [debugDetailStack](const QString& title,
                           const QString& description,
                           QPushButton** copyButton = nullptr) {
            auto* page = new QWidget(debugDetailStack);
            auto* layout = new QVBoxLayout(page);
            layout->setContentsMargins(0, 0, 0, 0);
            layout->setSpacing(12);
            auto* titleRow = new QHBoxLayout();
            titleRow->setContentsMargins(0, 0, 0, 0);
            auto* titleLabel = new QLabel(title, page);
            titleLabel->setObjectName(
                QStringLiteral("settingsDetailTitle"));
            titleRow->addWidget(titleLabel);
            titleRow->addStretch(1);
            if (copyButton) {
                *copyButton = new QPushButton(
                    QStringLiteral("复制"), page);
                (*copyButton)->setObjectName(
                    QStringLiteral("softButton"));
                (*copyButton)->setCursor(Qt::PointingHandCursor);
                (*copyButton)->setToolTip(
                    QStringLiteral("复制当前媒体能力页面信息"));
                titleRow->addWidget(*copyButton);
            }
            layout->addLayout(titleRow);
            auto* descriptionLabel = new QLabel(description, page);
            descriptionLabel->setProperty("muted", true);
            descriptionLabel->setWordWrap(true);
            layout->addWidget(descriptionLabel);
            layout->addSpacing(8);
            debugDetailStack->addWidget(page);
            return layout;
        };
    const auto addDebugValue =
        [this](QVBoxLayout* layout, const QString& key,
               const QString& title, bool expanded = false) {
            auto* page = layout->parentWidget();
            auto* row = new QFrame(page);
            row->setObjectName(QStringLiteral("settingRow"));
            auto* rowLayout = new QHBoxLayout(row);
            rowLayout->setContentsMargins(20, 17, 20, 17);
            rowLayout->setSpacing(18);
            auto* titleLabel = new QLabel(title, row);
            titleLabel->setObjectName(QStringLiteral("settingTitle"));
            titleLabel->setFixedWidth(expanded ? 140 : 170);
            titleLabel->setAlignment(expanded ? Qt::AlignTop
                                              : Qt::AlignVCenter);
            auto* valueLabel = new QLabel(
                QStringLiteral("正在获取…"), row);
            valueLabel->setObjectName(QStringLiteral("debugValue"));
            valueLabel->setProperty("tone", "muted");
            valueLabel->setWordWrap(expanded);
            valueLabel->setTextInteractionFlags(
                Qt::TextSelectableByMouse);
            valueLabel->setAlignment(expanded ? Qt::AlignTop
                                              : Qt::AlignVCenter);
            rowLayout->addWidget(titleLabel);
            rowLayout->addWidget(valueLabel, 1);
            layout->addWidget(row);
            debugValueLabels_.insert(key, valueLabel);
        };

    auto* deviceDebugLayout = makeDebugDetailPage(
        QStringLiteral("设备与信令"),
        QStringLiteral("查看本机身份、信令连接和 WebRTC 运行状态。"));
    addDebugValue(deviceDebugLayout, QStringLiteral("deviceId"),
                  QStringLiteral("本机设备 ID"));
    addDebugValue(deviceDebugLayout, QStringLiteral("signaling"),
                  QStringLiteral("信令连接"));
    addDebugValue(deviceDebugLayout, QStringLiteral("webrtc"),
                  QStringLiteral("WebRTC 运行时"));
    addDebugValue(deviceDebugLayout, QStringLiteral("systemEnvironment"),
                  QStringLiteral("Windows 环境"));
    addDebugValue(deviceDebugLayout, QStringLiteral("sessionEnvironment"),
                  QStringLiteral("会话类型"));
    addDebugValue(deviceDebugLayout, QStringLiteral("graphicsAdapters"),
                  QStringLiteral("图形适配器"), true);
    addDebugValue(deviceDebugLayout, QStringLiteral("hardwareFingerprint"),
                  QStringLiteral("硬件能力指纹"), true);
    addDebugValue(deviceDebugLayout, QStringLiteral("encoderProbeSource"),
                  QStringLiteral("编码能力探测"));
    addDebugValue(deviceDebugLayout, QStringLiteral("audioDeviceModule"),
                  QStringLiteral("WebRTC 音频模块"), true);
    addDebugValue(deviceDebugLayout, QStringLiteral("cameraDevice"),
                  QStringLiteral("摄像头设备"), true);
    addDebugValue(deviceDebugLayout, QStringLiteral("microphoneDevice"),
                  QStringLiteral("麦克风设备"), true);
    addDebugValue(deviceDebugLayout, QStringLiteral("speakerDevice"),
                  QStringLiteral("扬声器设备"), true);
    deviceDebugLayout->addStretch(1);

    auto* mediaDebugLayout = makeDebugDetailPage(
        QStringLiteral("媒体能力"),
        QStringLiteral("显示当前机器的 H264 编解码和 D3D11 能力。"),
        &copyMediaDebugButton_);
    addDebugValue(mediaDebugLayout, QStringLiteral("builtinEncoder"),
                  QStringLiteral("H264 编码链路"));
    addDebugValue(mediaDebugLayout, QStringLiteral("desktopCapture"),
                  QStringLiteral("屏幕采集模式"));
    addDebugValue(mediaDebugLayout, QStringLiteral("encoderPreference"),
                  QStringLiteral("视频编码模式"));
    addDebugValue(mediaDebugLayout, QStringLiteral("encoderQuality"),
                  QStringLiteral("编码质量"), true);
    addDebugValue(mediaDebugLayout, QStringLiteral("hardwareEncoderCount"),
                  QStringLiteral("硬件编码器"));
    addDebugValue(mediaDebugLayout, QStringLiteral("cpuNv12"),
                  QStringLiteral("CPU NV12 输入"));
    addDebugValue(mediaDebugLayout, QStringLiteral("d3d11Encoder"),
                  QStringLiteral("D3D11 硬件编码"));
    addDebugValue(mediaDebugLayout, QStringLiteral("encoderFallback"),
                  QStringLiteral("软件编码链路"));
    addDebugValue(mediaDebugLayout, QStringLiteral("encoderRuntime"),
                  QStringLiteral("当前编码实例"), true);
    addDebugValue(mediaDebugLayout, QStringLiteral("encoderFallbackReason"),
                  QStringLiteral("最近回退原因"), true);
    addDebugValue(mediaDebugLayout, QStringLiteral("decoder"),
                  QStringLiteral("H264 解码"));
    addDebugValue(mediaDebugLayout, QStringLiteral("decoderPreference"),
                  QStringLiteral("视频解码模式"));
    addDebugValue(mediaDebugLayout, QStringLiteral("mfDecoderType"),
                  QStringLiteral("MF D3D11 解码类型"));
    addDebugValue(mediaDebugLayout, QStringLiteral("mfDecoderName"),
                  QStringLiteral("MF 解码器实例"), true);
    addDebugValue(mediaDebugLayout, QStringLiteral("d3d11Output"),
                  QStringLiteral("D3D11 原生输出"));
    addDebugValue(mediaDebugLayout, QStringLiteral("mfDecoderAsync"),
                  QStringLiteral("硬件 MFT 驱动"));
    addDebugValue(mediaDebugLayout, QStringLiteral("softwareFallback"),
                  QStringLiteral("FFmpeg 软件解码"));
    addDebugValue(mediaDebugLayout, QStringLiteral("mfDecoderError"),
                  QStringLiteral("MF 解码检测说明"), true);
    addDebugValue(mediaDebugLayout, QStringLiteral("encoderDetails"),
                  QStringLiteral("检测到的编码器"), true);
    mediaDebugLayout->addStretch(1);

    auto* connectionDebugLayout = makeDebugDetailPage(
        QStringLiteral("连接质量"),
        QStringLiteral(
            "每秒读取 WebRTC 标准 Stats，显示真实链路、媒体流和 DataChannel 状态。"));
    statsCardsWidget_ =
        new DiagnosticsCardsWidget(connectionDebugLayout->parentWidget());
    connectionDebugLayout->addWidget(statsCardsWidget_);
    connectionDebugLayout->addStretch(1);

    auto* roomDebugLayout = makeDebugDetailPage(
        QStringLiteral("房间状态"),
        QStringLiteral("查看当前房间、席位和控制租约状态。"));
    addDebugValue(roomDebugLayout, QStringLiteral("membership"),
                  QStringLiteral("房间状态"));
    addDebugValue(roomDebugLayout, QStringLiteral("roomId"),
                  QStringLiteral("房间 ID"));
    addDebugValue(roomDebugLayout, QStringLiteral("members"),
                  QStringLiteral("成员与席位"));
    addDebugValue(roomDebugLayout, QStringLiteral("screenSharer"),
                  QStringLiteral("当前主机器"));
    addDebugValue(roomDebugLayout, QStringLiteral("screenShareGeneration"),
                  QStringLiteral("屏幕共享代次"));
    addDebugValue(roomDebugLayout, QStringLiteral("controller"),
                  QStringLiteral("当前控制者"));
    addDebugValue(roomDebugLayout, QStringLiteral("controlGrant"),
                  QStringLiteral("控制授权令牌"));
    roomDebugLayout->addStretch(1);

    auto* peerDebugLayout = makeDebugDetailPage(
        QStringLiteral("成员连接"),
        QStringLiteral("查看每个成员对的 P2P、DataChannel 和媒体槽状态。"));
    addDebugValue(peerDebugLayout, QStringLiteral("peerSummary"),
                  QStringLiteral("连接概况"));
    addDebugValue(peerDebugLayout, QStringLiteral("peerDetails"),
                  QStringLiteral("成员对明细"), true);
    peerDebugLayout->addStretch(1);

    auto* inputDebugLayout = makeDebugDetailPage(
        QStringLiteral("鼠标与键盘"),
        QStringLiteral(
            "按秒查看远控输入从本机生成、网络发送到远端 Windows 注入的完整链路。"));
    inputEventStatsEnabled_ = QSettings().value(
        QString::fromLatin1(kInputEventStatsEnabledSetting),
        false).toBool();
    RemoteInputTelemetry::Instance().SetEnabled(
        inputEventStatsEnabled_);
    auto* inputStatsRow =
        new QFrame(inputDebugLayout->parentWidget());
    inputStatsRow->setObjectName(QStringLiteral("settingRow"));
    auto* inputStatsRowLayout = new QHBoxLayout(inputStatsRow);
    inputStatsRowLayout->setContentsMargins(20, 14, 20, 14);
    inputStatsRowLayout->setSpacing(18);
    auto* inputStatsTitle = new QLabel(
        QStringLiteral("输入事件统计"), inputStatsRow);
    inputStatsTitle->setObjectName(QStringLiteral("settingTitle"));
    inputStatsTitle->setFixedWidth(170);
    auto* inputStatsDescription = new QLabel(
        QStringLiteral("默认关闭；开启后每秒刷新计数，不写入日志文件。"),
        inputStatsRow);
    inputStatsDescription->setProperty("muted", true);
    inputStatsDescription->setWordWrap(true);
    inputEventStatsButton_ = new QPushButton(inputStatsRow);
    inputEventStatsButton_->setObjectName(QStringLiteral("softButton"));
    inputEventStatsButton_->setCheckable(true);
    inputEventStatsButton_->setChecked(inputEventStatsEnabled_);
    inputEventStatsButton_->setCursor(Qt::PointingHandCursor);
    inputEventStatsButton_->setMinimumWidth(130);
    const auto updateInputStatsButton = [this] {
        if (!inputEventStatsButton_) {
            return;
        }
        inputEventStatsButton_->setText(
            inputEventStatsEnabled_
                ? QStringLiteral("统计已开启")
                : QStringLiteral("开启统计"));
        inputEventStatsButton_->setToolTip(
            inputEventStatsEnabled_
                ? QStringLiteral("点击停止鼠标与键盘事件计数")
                : QStringLiteral("点击开始统计远控输入链路"));
    };
    updateInputStatsButton();
    connect(
        inputEventStatsButton_,
        &QPushButton::toggled,
        this,
        [this, updateInputStatsButton](bool enabled) {
            inputEventStatsEnabled_ = enabled;
            RemoteInputTelemetry::Instance().SetEnabled(enabled);
            QSettings settings;
            settings.setValue(
                QString::fromLatin1(
                    kInputEventStatsEnabledSetting),
                enabled);
            updateInputStatsButton();
            RefreshDiagnosticsUi();
            RemoteCToast::Show(
                this,
                enabled
                    ? QStringLiteral("鼠标与键盘统计已开启")
                    : QStringLiteral("鼠标与键盘统计已关闭"),
                RemoteCToast::Tone::kInformation);
        });
    inputStatsRowLayout->addWidget(inputStatsTitle);
    inputStatsRowLayout->addWidget(inputStatsDescription, 1);
    inputStatsRowLayout->addWidget(inputEventStatsButton_);
    inputDebugLayout->addWidget(inputStatsRow);
    addDebugValue(inputDebugLayout, QStringLiteral("inputMovePolicy"),
                  QStringLiteral("移动调度策略"), true);
    addDebugValue(inputDebugLayout, QStringLiteral("inputMoveGenerated"),
                  QStringLiteral("Qt 移动事件"));
    addDebugValue(inputDebugLayout, QStringLiteral("inputMoveDispatched"),
                  QStringLiteral("实际调度移动"));
    addDebugValue(inputDebugLayout, QStringLiteral("inputImmediateEvents"),
                  QStringLiteral("按键与滚轮"));
    addDebugValue(inputDebugLayout, QStringLiteral("inputPacketsSent"),
                  QStringLiteral("网络发送包"));
    addDebugValue(inputDebugLayout, QStringLiteral("inputPacketsReceived"),
                  QStringLiteral("网络接收包"));
    addDebugValue(inputDebugLayout, QStringLiteral("inputInjected"),
                  QStringLiteral("Windows 注入"), true);
    inputDebugLayout->addStretch(1);

    auto* clipboardDebugLayout = makeDebugDetailPage(
        QStringLiteral("远程粘贴"),
        QStringLiteral(
            "只显示格式、大小和状态；正文、文件名和完整哈希永不进入调试信息。"));
    addDebugValue(clipboardDebugLayout, QStringLiteral("clipboardState"),
                  QStringLiteral("粘贴状态"));
    addDebugValue(clipboardDebugLayout, QStringLiteral("clipboardPeer"),
                  QStringLiteral("当前成员对"));
    addDebugValue(clipboardDebugLayout, QStringLiteral("clipboardTransfer"),
                  QStringLiteral("当前传输"));
    addDebugValue(clipboardDebugLayout, QStringLiteral("clipboardLast"),
                  QStringLiteral("最近项目"));
    addDebugValue(clipboardDebugLayout, QStringLiteral("clipboardCounters"),
                  QStringLiteral("累计统计"), true);
    addDebugValue(clipboardDebugLayout, QStringLiteral("clipboardError"),
                  QStringLiteral("最近错误"), true);
    clipboardDebugLayout->addStretch(1);

    auto* errorDebugLayout = makeDebugDetailPage(
        QStringLiteral("最近错误"),
        QStringLiteral("显示会话和房间最近一次报告的错误。"));
    addDebugValue(errorDebugLayout, QStringLiteral("sessionError"),
                  QStringLiteral("会话错误"), true);
    addDebugValue(errorDebugLayout, QStringLiteral("roomError"),
                  QStringLiteral("房间错误"), true);
    errorDebugLayout->addStretch(1);

    connect(debugCategoryGroup, &QButtonGroup::idClicked, this,
            [this, debugDetailStack](int index) {
                debugDetailStack->setCurrentIndex(index);
                if (index == 2) {
                    RefreshDiagnosticsUi();
                }
            });
    firstDebugCategory->setChecked(true);
    debugDetailStack->setCurrentIndex(0);
    debugPageLayout->addLayout(debugWorkspace, 1);
    pageStack_->addWidget(debugPage);
