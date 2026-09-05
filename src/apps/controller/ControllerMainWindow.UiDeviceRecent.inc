// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

    QVBoxLayout* devicePageLayout = nullptr;
    auto* devicePage = MakePageSurface(
        QStringLiteral("远程协助"),
        QStringLiteral("查看本机身份、信令状态和可用的媒体能力。"),
        pageStack_, &devicePageLayout);
    devicePage->setObjectName(QStringLiteral("localDevicePage"));
    localDevicePage_ = devicePage;
    {
        QFile pageStyle(QStringLiteral(":/ui/theme/local-device.qss"));
        if (pageStyle.open(QIODevice::ReadOnly | QIODevice::Text)) {
            devicePage->setStyleSheet(QString::fromUtf8(pageStyle.readAll()));
        }
    }

    // Logged-out hero. Keep the existing authentication flow and only replace
    // the presentation with the same visual language used by the signed-in
    // workspace.
    deviceLoginPrompt_ = new QFrame(devicePage);
    deviceLoginPrompt_->setObjectName(QStringLiteral("localDeviceLoginHero"));
    auto* loginPromptLayout = new QVBoxLayout(deviceLoginPrompt_);
    loginPromptLayout->setContentsMargins(30, 28, 30, 24);
    loginPromptLayout->setSpacing(18);

    auto* loginHeroRow = new QHBoxLayout();
    loginHeroRow->setSpacing(30);
    auto* loginHeroCopy = new QVBoxLayout();
    loginHeroCopy->setSpacing(8);
    loginHeroCopy->setAlignment(Qt::AlignVCenter);
    auto* heroEyebrow = new QLabel(
        QStringLiteral("安全设备接入"), deviceLoginPrompt_);
    heroEyebrow->setObjectName(QStringLiteral("localDeviceHeroEyebrow"));
    loginHeroCopy->addWidget(heroEyebrow);
    auto* loginTitle = new QLabel(
        QStringLiteral("登录后，让这台电脑成为你的 RLink 设备"),
        deviceLoginPrompt_);
    loginTitle->setObjectName(QStringLiteral("localDeviceHeroTitle"));
    loginTitle->setWordWrap(true);
    loginHeroCopy->addWidget(loginTitle);
    deviceLoginStatus_ = new QLabel(
        QStringLiteral("设备身份、同账号设备、协作房间和远程协助会在登录后自动启用。"),
        deviceLoginPrompt_);
    deviceLoginStatus_->setObjectName(QStringLiteral("localDeviceHeroBody"));
    deviceLoginStatus_->setWordWrap(true);
    loginHeroCopy->addWidget(deviceLoginStatus_);
    loginHeroCopy->addSpacing(8);
    deviceLoginButton_ = new QPushButton(
        QStringLiteral("立即登录  →"), deviceLoginPrompt_);
    deviceLoginButton_->setObjectName(QStringLiteral("deviceLoginButton"));
    deviceLoginButton_->setCursor(Qt::PointingHandCursor);
    deviceLoginButton_->setMinimumWidth(178);
    deviceLoginButton_->setMaximumWidth(224);
    deviceLoginButton_->setFixedHeight(46);
    loginHeroCopy->addWidget(deviceLoginButton_, 0, Qt::AlignLeft);
    auto* secureLoginNote = new QLabel(
        QStringLiteral("身份验证和会话控制使用加密连接。"),
        deviceLoginPrompt_);
    secureLoginNote->setObjectName(QStringLiteral("localDeviceSecureNote"));
    loginHeroCopy->addWidget(secureLoginNote);
    loginHeroRow->addLayout(loginHeroCopy, 1);

    auto* loginHeroArtwork = new QLabel(deviceLoginPrompt_);
    loginHeroArtwork->setAlignment(Qt::AlignCenter);
    loginHeroArtwork->setFixedSize(330, 180);
    loginHeroArtwork->setPixmap(
        QIcon(QStringLiteral(":/ui/illustrations/devices/local-device-hero.svg"))
            .pixmap(320, 170));
    loginHeroRow->addWidget(loginHeroArtwork, 0, Qt::AlignVCenter);
    loginPromptLayout->addLayout(loginHeroRow);

    auto* featureStrip = new QFrame(deviceLoginPrompt_);
    featureStrip->setObjectName(QStringLiteral("localDeviceFeatureStrip"));
    auto* featureLayout = new QHBoxLayout(featureStrip);
    featureLayout->setContentsMargins(14, 12, 14, 12);
    featureLayout->setSpacing(8);
    const auto addLoginFeature =
        [featureStrip, featureLayout](const QString& resource,
                                     const QString& title,
                                     const QString& detail) {
            auto* feature = new QWidget(featureStrip);
            auto* row = new QHBoxLayout(feature);
            row->setContentsMargins(5, 0, 5, 0);
            row->setSpacing(10);
            auto* iconLabel = new QLabel(feature);
            iconLabel->setObjectName(QStringLiteral("localDeviceFeatureIcon"));
            iconLabel->setProperty("localFeatureIconResource", resource);
            iconLabel->setAlignment(Qt::AlignCenter);
            iconLabel->setFixedSize(40, 40);
            const bool dark = ui::RemoteCTheme::IsDark(
                ui::RemoteCTheme::LoadPreference());
            ui::RemoteCTheme::SetPixmap(
                iconLabel, resource, QSize(20, 20),
                resource.contains(QStringLiteral("shield-check"))
                    ? ui::ThemeIconTone::kSuccess
                    : (dark ? ui::ThemeIconTone::kOnDark
                            : ui::ThemeIconTone::kPrimary));
            auto* labels = new QVBoxLayout();
            labels->setSpacing(1);
            auto* titleLabel = new QLabel(title, feature);
            titleLabel->setObjectName(QStringLiteral("localDeviceFeatureTitle"));
            auto* detailLabel = new QLabel(detail, feature);
            detailLabel->setObjectName(QStringLiteral("localDeviceFeatureDetail"));
            labels->addWidget(titleLabel);
            labels->addWidget(detailLabel);
            row->addWidget(iconLabel);
            row->addLayout(labels, 1);
            featureLayout->addWidget(feature, 1);
        };
    addLoginFeature(QStringLiteral(":/ui/icons/lucide/base/shield-check.svg"),
                    QStringLiteral("安全"),
                    QStringLiteral("加密保护每一次远程连接"));
    addLoginFeature(QStringLiteral(":/ui/icons/lucide/base/monitor-check.svg"),
                    QStringLiteral("4K 120FPS"),
                    QStringLiteral("超高清高帧率远程桌面"));
    addLoginFeature(QStringLiteral(":/ui/icons/lucide/base/star-check.svg"),
                    QStringLiteral("免费"),
                    QStringLiteral("核心远程功能免费使用"));
    loginPromptLayout->addWidget(featureStrip);
    devicePageLayout->addWidget(deviceLoginPrompt_);

    // Signed-in workspace. deviceIdentityCard_ intentionally remains the
    // visibility root used by the existing authentication/state code.
    deviceIdentityCard_ = new QFrame(devicePage);
    auto* loggedInWorkspace = deviceIdentityCard_;
    loggedInWorkspace->setObjectName(QStringLiteral("localLoggedInWorkspace"));
    auto* workspaceLayout = new QVBoxLayout(loggedInWorkspace);
    workspaceLayout->setContentsMargins(0, 0, 0, 0);
    workspaceLayout->setSpacing(14);

    auto* heroBand = new QFrame(loggedInWorkspace);
    heroBand->setObjectName(QStringLiteral("localHeroBand"));
    auto* heroBandLayout = new QHBoxLayout(heroBand);
    heroBandLayout->setContentsMargins(24, 18, 20, 18);
    heroBandLayout->setSpacing(18);

    auto* heroStatusCopy = new QVBoxLayout();
    heroStatusCopy->setSpacing(6);
    auto* heroBandTitle = new QLabel(
        QStringLiteral("这台电脑已准备好远程连接"), heroBand);
    heroBandTitle->setObjectName(QStringLiteral("localHeroTitle"));
    heroStatusCopy->addWidget(heroBandTitle);
    auto* heroBandSubtitle = new QLabel(
        QStringLiteral("设备凭据、远程协助和连接能力集中在一个工作区中。"),
        heroBand);
    heroBandSubtitle->setObjectName(QStringLiteral("localHeroSubtitle"));
    heroBandSubtitle->setWordWrap(true);
    heroStatusCopy->addWidget(heroBandSubtitle);
    heroStatusCopy->addSpacing(6);

    auto* statusPills = new QHBoxLayout();
    statusPills->setSpacing(8);
    devicePageSignalStatus_ = new QLabel(
        QStringLiteral("●  信令未连接"), heroBand);
    devicePageSignalStatus_->setObjectName(QStringLiteral("localSignalPill"));
    devicePageSignalStatus_->setAlignment(Qt::AlignCenter);
    runtimeStatus_ = new QLabel(
        QStringLiteral("●  WebRTC 初始化中"), heroBand);
    runtimeStatus_->setObjectName(QStringLiteral("localRuntimePill"));
    runtimeStatus_->setAlignment(Qt::AlignCenter);
    decoderStatus_ = new QLabel(
        QStringLiteral("●  正在检测解码器"), heroBand);
    decoderStatus_->setObjectName(QStringLiteral("localDecoderPill"));
    decoderStatus_->setAlignment(Qt::AlignCenter);
    statusPills->addWidget(devicePageSignalStatus_);
    statusPills->addWidget(runtimeStatus_);
    statusPills->addWidget(decoderStatus_);
    statusPills->addStretch(1);
    heroStatusCopy->addLayout(statusPills);
    heroBandLayout->addLayout(heroStatusCopy, 1);

    auto* heroArtwork = new QLabel(heroBand);
    heroArtwork->setAlignment(Qt::AlignCenter);
    heroArtwork->setFixedSize(300, 145);
    heroArtwork->setPixmap(
        QIcon(QStringLiteral(":/ui/illustrations/devices/local-device-hero.svg"))
            .pixmap(286, 136));
    heroBandLayout->addWidget(heroArtwork, 0, Qt::AlignVCenter);
    workspaceLayout->addWidget(heroBand);

    auto* mainColumns = new QHBoxLayout();
    mainColumns->setSpacing(14);

    auto* identityPanel = new QFrame(loggedInWorkspace);
    identityPanel->setObjectName(QStringLiteral("localIdentityPanel"));
    auto* identityLayout = new QVBoxLayout(identityPanel);
    identityLayout->setContentsMargins(20, 18, 20, 18);
    identityLayout->setSpacing(10);

    auto* identityHeader = new QHBoxLayout();
    auto* identityIcon = new QLabel(identityPanel);
    identityIcon->setObjectName(QStringLiteral("localPanelIcon"));
    identityIcon->setAlignment(Qt::AlignCenter);
    identityIcon->setFixedSize(40, 40);
    ui::RemoteCTheme::SetPixmap(
        identityIcon, QStringLiteral(":/ui/icons/light/monitor.svg"),
        QSize(21, 21), ui::ThemeIconTone::kPrimary);
    identityHeader->addWidget(identityIcon);
    auto* identityHeaderText = new QVBoxLayout();
    identityHeaderText->setSpacing(1);
    auto* identityTitle = new QLabel(
        QStringLiteral("本机身份"), identityPanel);
    identityTitle->setObjectName(QStringLiteral("localPanelTitle"));
    auto* identitySubtitle = new QLabel(
        QStringLiteral("用于识别和授权当前电脑"), identityPanel);
    identitySubtitle->setObjectName(QStringLiteral("localPanelSubtitle"));
    identityHeaderText->addWidget(identityTitle);
    identityHeaderText->addWidget(identitySubtitle);
    identityHeader->addLayout(identityHeaderText, 1);
    shareLocalCredentialsButton_ = new QPushButton(
        QStringLiteral("分享"), identityPanel);
    shareLocalCredentialsButton_->setObjectName(QStringLiteral("softButton"));
    shareLocalCredentialsButton_->setIconSize(QSize(15, 15));
    shareLocalCredentialsButton_->setToolTip(
        QStringLiteral("复制设备 ID 和验证码，可直接发送给对方"));
    ui::RemoteCTheme::SetIcon(
        shareLocalCredentialsButton_,
        QStringLiteral(":/ui/icons/actions/copy.svg"),
        ui::ThemeIconTone::kPrimary);
    shareLocalCredentialsButton_->setCursor(Qt::PointingHandCursor);
    shareLocalCredentialsButton_->setEnabled(false);
    identityHeader->addWidget(
        shareLocalCredentialsButton_, 0, Qt::AlignVCenter);
    identityLayout->addLayout(identityHeader);

    const auto makeCredentialRow =
        [identityPanel](const QString& caption) {
            auto* row = new QFrame(identityPanel);
            row->setObjectName(QStringLiteral("localCredentialRow"));
            auto* layout = new QHBoxLayout(row);
            layout->setContentsMargins(14, 10, 10, 10);
            layout->setSpacing(10);
            auto* textLayout = new QVBoxLayout();
            textLayout->setSpacing(2);
            auto* captionLabel = new QLabel(caption, row);
            captionLabel->setObjectName(QStringLiteral("localCredentialCaption"));
            textLayout->addWidget(captionLabel);
            layout->addLayout(textLayout, 1);
            return std::pair<QFrame*, QVBoxLayout*>(row, textLayout);
        };

    auto [deviceIdBox, deviceIdText] = makeCredentialRow(
        QStringLiteral("本机设备 ID（9 位）"));
    localDeviceId_ = new QLabel(QStringLiteral("未注册"), deviceIdBox);
    localDeviceId_->setObjectName(QStringLiteral("localCredentialValue"));
    localDeviceId_->setTextInteractionFlags(Qt::TextSelectableByMouse);
    deviceIdText->addWidget(localDeviceId_);
    copyDeviceIdButton_ = new QPushButton(
        QStringLiteral("复制"), deviceIdBox);
    copyDeviceIdButton_->setObjectName(QStringLiteral("softButton"));
    copyDeviceIdButton_->setIconSize(QSize(15, 15));
    ui::RemoteCTheme::SetIcon(
        copyDeviceIdButton_, QStringLiteral(":/ui/icons/actions/copy.svg"),
        ui::ThemeIconTone::kPrimary);
    remotec::ui::morph::MorphIconButtonBinding::attach(
        copyDeviceIdButton_,
        QStringLiteral(":/ui/icons/lucide/base/copy.svg"),
        QStringLiteral(":/ui/icons/lucide/base/circle-check-big.svg"),
        remotec::ui::morph::MorphIconButtonBinding::Interaction::Feedback,
        QSize(15, 15), QColor(QStringLiteral("#2563EB")),
        QColor(QStringLiteral("#12B76A")));
    copyDeviceIdButton_->setCursor(Qt::PointingHandCursor);
    copyDeviceIdButton_->setEnabled(false);
    deviceIdBox->layout()->addWidget(copyDeviceIdButton_);
    identityLayout->addWidget(deviceIdBox);

    auto [verificationBox, verificationText] = makeCredentialRow(
        QStringLiteral("一次性验证码（6 位）"));
    localVerificationCode_ = new QLabel(
        QStringLiteral("------"), verificationBox);
    localVerificationCode_->setObjectName(QStringLiteral("localCredentialValue"));
    localVerificationCode_->setProperty("verification", true);
    localVerificationCode_->setTextInteractionFlags(Qt::TextSelectableByMouse);
    verificationText->addWidget(localVerificationCode_);
    copyVerificationCodeButton_ = new QPushButton(
        QStringLiteral("复制"), verificationBox);
    copyVerificationCodeButton_->setObjectName(QStringLiteral("softButton"));
    copyVerificationCodeButton_->setIconSize(QSize(15, 15));
    ui::RemoteCTheme::SetIcon(
        copyVerificationCodeButton_,
        QStringLiteral(":/ui/icons/actions/copy.svg"),
        ui::ThemeIconTone::kPrimary);
    remotec::ui::morph::MorphIconButtonBinding::attach(
        copyVerificationCodeButton_,
        QStringLiteral(":/ui/icons/lucide/base/copy.svg"),
        QStringLiteral(":/ui/icons/lucide/base/circle-check-big.svg"),
        remotec::ui::morph::MorphIconButtonBinding::Interaction::Feedback,
        QSize(15, 15), QColor(QStringLiteral("#2563EB")),
        QColor(QStringLiteral("#12B76A")));
    copyVerificationCodeButton_->setCursor(Qt::PointingHandCursor);
    copyVerificationCodeButton_->setEnabled(false);
    verificationBox->layout()->addWidget(copyVerificationCodeButton_);
    identityLayout->addWidget(verificationBox);

    auto [accountBox, accountText] = makeCredentialRow(
        QStringLiteral("当前账户"));
    deviceAccountLabel_ = new QLabel(
        QStringLiteral("未登录"), accountBox);
    deviceAccountLabel_->setObjectName(QStringLiteral("localAccountValue"));
    deviceAccountLabel_->setTextInteractionFlags(Qt::TextSelectableByMouse);
    accountText->addWidget(deviceAccountLabel_);
    identityLayout->addWidget(accountBox);
    mainColumns->addWidget(identityPanel, 1);

    deviceDirectAssistCard_ = new QFrame(loggedInWorkspace);
    auto* directAssistCard = deviceDirectAssistCard_;
    directAssistCard->setObjectName(QStringLiteral("localAssistPanel"));
    auto* directAssistLayout = new QVBoxLayout(directAssistCard);
    directAssistLayout->setContentsMargins(20, 18, 20, 18);
    directAssistLayout->setSpacing(10);

    auto* assistHeader = new QHBoxLayout();
    auto* assistIcon = new QLabel(directAssistCard);
    assistIcon->setObjectName(QStringLiteral("localPanelIcon"));
    assistIcon->setAlignment(Qt::AlignCenter);
    assistIcon->setFixedSize(40, 40);
    ui::RemoteCTheme::SetPixmap(
        assistIcon, QStringLiteral(":/ui/icons/light/control.svg"),
        QSize(21, 21), ui::ThemeIconTone::kPrimary);
    assistHeader->addWidget(assistIcon);
    auto* assistHeaderText = new QVBoxLayout();
    assistHeaderText->setSpacing(1);
    auto* directAssistTitle = new QLabel(
        QStringLiteral("远程协助"), directAssistCard);
    directAssistTitle->setObjectName(QStringLiteral("localPanelTitle"));
    auto* directAssistSubtitle = new QLabel(
        QStringLiteral("输入对方设备 ID 和验证码，快速建立远程控制。"),
        directAssistCard);
    directAssistSubtitle->setObjectName(QStringLiteral("localPanelSubtitle"));
    directAssistSubtitle->setWordWrap(true);
    assistHeaderText->addWidget(directAssistTitle);
    assistHeaderText->addWidget(directAssistSubtitle);
    assistHeader->addLayout(assistHeaderText, 1);
    auto* approvalBadge = new QLabel(
        QStringLiteral("验证码直连"), directAssistCard);
    approvalBadge->setObjectName(QStringLiteral("directAssistBadge"));
    assistHeader->addWidget(approvalBadge, 0, Qt::AlignTop);
    directAssistLayout->addLayout(assistHeader);
    directAssistLayout->addSpacing(2);

    deviceIdEdit_ = new QLineEdit(directAssistCard);
    deviceIdEdit_->setObjectName(QStringLiteral("assistDeviceIdInput"));
    deviceIdEdit_->setPlaceholderText(QStringLiteral("对方设备 ID（9 位）"));
    deviceIdEdit_->setMaxLength(9);
    deviceIdEdit_->setClearButtonEnabled(true);
    deviceIdEdit_->setValidator(new QRegularExpressionValidator(
        QRegularExpression(QStringLiteral("[1-9][0-9]{0,8}")),
        deviceIdEdit_));
    deviceIdEdit_->setContextMenuPolicy(Qt::NoContextMenu);
    deviceIdEdit_->setFixedHeight(48);
    auto* assistDeviceAction = deviceIdEdit_->addAction(
        QIcon(), QLineEdit::LeadingPosition);
    ui::RemoteCTheme::SetIcon(
        assistDeviceAction, QStringLiteral(":/ui/icons/light/monitor.svg"),
        ui::ThemeIconTone::kNeutral);
    directAssistLayout->addWidget(deviceIdEdit_);

    verificationCodeEdit_ = new QLineEdit(directAssistCard);
    verificationCodeEdit_->setObjectName(QStringLiteral("assistVerificationInput"));
    verificationCodeEdit_->setPlaceholderText(QStringLiteral("对方验证码（6 位）"));
    verificationCodeEdit_->setMaxLength(6);
    verificationCodeEdit_->setClearButtonEnabled(true);
    verificationCodeEdit_->setValidator(new QRegularExpressionValidator(
        QRegularExpression(QStringLiteral("[0-9]{0,6}")),
        verificationCodeEdit_));
    verificationCodeEdit_->setContextMenuPolicy(Qt::NoContextMenu);
    verificationCodeEdit_->setFixedHeight(48);
    auto* assistCodeAction = verificationCodeEdit_->addAction(
        QIcon(), QLineEdit::LeadingPosition);
    ui::RemoteCTheme::SetIcon(
        assistCodeAction, QStringLiteral(":/ui/icons/status/success.svg"),
        ui::ThemeIconTone::kSuccess);
    directAssistLayout->addWidget(verificationCodeEdit_);

    connectButton_ = new QPushButton(
        QStringLiteral("发起连接"), directAssistCard);
    connectButton_->setObjectName(QStringLiteral("primaryButton"));
    connectButton_->setIconSize(QSize(19, 19));
    ui::RemoteCTheme::SetIcon(
        connectButton_, QStringLiteral(":/ui/icons/actions/control.svg"),
        ui::ThemeIconTone::kOnDark);
    remotec::ui::morph::MorphIconButtonBinding::attach(
        connectButton_,
        QStringLiteral(":/ui/icons/lucide/base/mouse-pointer-2.svg"),
        QStringLiteral(":/ui/icons/lucide/base/screen-share.svg"),
        remotec::ui::morph::MorphIconButtonBinding::Interaction::Hover,
        QSize(19, 19), QColor(QStringLiteral("#FFFFFF")),
        QColor(QStringLiteral("#FFFFFF")));
    connectButton_->setCursor(Qt::PointingHandCursor);
    connectButton_->setFixedHeight(48);
    connectButton_->setEnabled(false);
    directAssistLayout->addWidget(connectButton_);

    connectionHint_ = new QLabel(
        QStringLiteral("验证成功后自动打开远程桌面；每次成功协助后验证码自动刷新。"),
        directAssistCard);
    connectionHint_->setObjectName(QStringLiteral("localPanelSubtitle"));
    connectionHint_->setWordWrap(true);
    directAssistLayout->addWidget(connectionHint_);
    directAssistLayout->addStretch(1);
    mainColumns->addWidget(directAssistCard, 1);
    workspaceLayout->addLayout(mainColumns);

    auto* trustStrip = new QFrame(loggedInWorkspace);
    trustStrip->setObjectName(QStringLiteral("localTrustStrip"));
    auto* trustLayout = new QHBoxLayout(trustStrip);
    trustLayout->setContentsMargins(16, 11, 16, 11);
    trustLayout->setSpacing(10);
    const auto addTrustItem =
        [trustStrip, trustLayout](const QString& iconPath,
                                 const QString& title,
                                 const QString& detail) {
            auto* item = new QWidget(trustStrip);
            auto* row = new QHBoxLayout(item);
            row->setContentsMargins(4, 0, 4, 0);
            row->setSpacing(9);
            auto* icon = new QLabel(item);
            icon->setObjectName(QStringLiteral("localTrustIcon"));
            icon->setAlignment(Qt::AlignCenter);
            icon->setFixedSize(36, 36);
            ui::RemoteCTheme::SetPixmap(
                icon, iconPath, QSize(18, 18),
                iconPath.contains(QStringLiteral("status/success"))
                    ? ui::ThemeIconTone::kSuccess
                    : ui::ThemeIconTone::kPrimary);
            auto* text = new QVBoxLayout();
            text->setSpacing(0);
            auto* titleLabel = new QLabel(title, item);
            titleLabel->setObjectName(QStringLiteral("localTrustTitle"));
            auto* detailLabel = new QLabel(detail, item);
            detailLabel->setObjectName(QStringLiteral("localTrustDetail"));
            text->addWidget(titleLabel);
            text->addWidget(detailLabel);
            row->addWidget(icon);
            row->addLayout(text, 1);
            trustLayout->addWidget(item, 1);
        };
    addTrustItem(QStringLiteral(":/ui/icons/status/success.svg"),
                 QStringLiteral("加密信令"),
                 QStringLiteral("控制消息通过 WSS"));
    addTrustItem(QStringLiteral(":/ui/icons/actions/monitor.svg"),
                 QStringLiteral("P2P 传输"),
                 QStringLiteral("媒体与输入走 WebRTC"));
    addTrustItem(QStringLiteral(":/ui/icons/navigation/diagnostics.svg"),
                 QStringLiteral("能力检测"),
                 QStringLiteral("自动检测媒体与解码器"));
    workspaceLayout->addWidget(trustStrip);

    // Preserve the legacy member used by existing visibility code, but retire
    // the old standalone runtime-status card from the actual layout.
    deviceStatusCard_ = new QFrame(loggedInWorkspace);
    deviceStatusCard_->setObjectName(QStringLiteral("localStatusCard"));
    deviceStatusCard_->setFixedSize(0, 0);
    deviceStatusCard_->hide();

    devicePageLayout->addWidget(loggedInWorkspace);
    devicePageLayout->addStretch(1);
    pageStack_->addWidget(devicePage);

#include "ControllerMainWindow.UiOwnedDevices.inc"

    QVBoxLayout* recentPageLayout = nullptr;
    auto* recentPage = MakePageSurface(
        QStringLiteral("最近连接"),
        QStringLiteral("快速回到近期使用过的设备和协作房间。"),
        pageStack_, &recentPageLayout);
    recentPage->setObjectName(QStringLiteral("recentConnectionsPage"));
    recentConnectionsPage_ = recentPage;
    {
        QFile pageStyle(QStringLiteral(":/ui/theme/recent-connections.qss"));
        if (pageStyle.open(QIODevice::ReadOnly | QIODevice::Text)) {
            recentPage->setStyleSheet(QString::fromUtf8(pageStyle.readAll()));
        }
    }

    auto* recentDevicesHeader = new QHBoxLayout();
    auto* recentDevicesTitle = new QLabel(
        QStringLiteral("最近设备"), recentPage);
    recentDevicesTitle->setObjectName(QStringLiteral("recentSectionTitle"));
    recentDevicesHeader->addWidget(recentDevicesTitle);
    auto* recentDevicesLimit = new QLabel(
        QStringLiteral("最多 5 个"), recentPage);
    recentDevicesLimit->setObjectName(QStringLiteral("recentLimitPill"));
    recentDevicesHeader->addWidget(recentDevicesLimit);
    recentDevicesHeader->addStretch(1);
    recentPageLayout->addLayout(recentDevicesHeader);

    recentDevicesLayout_ = new QVBoxLayout();
    recentDevicesLayout_->setSpacing(10);
    recentDevicesEmptyState_ = new QFrame(recentPage);
    recentDevicesEmptyState_->setObjectName(QStringLiteral("recentEmptyState"));
    auto* recentDevicesEmptyLayout = new QVBoxLayout(recentDevicesEmptyState_);
    recentDevicesEmptyLayout->setContentsMargins(26, 26, 26, 28);
    recentDevicesEmptyLayout->setSpacing(11);
    recentDevicesEmptyLayout->setAlignment(Qt::AlignCenter);
    auto* recentDevicesIcon = new QLabel(recentDevicesEmptyState_);
    recentDevicesIcon->setObjectName(QStringLiteral("recentEmptyIcon"));
    recentDevicesIcon->setAlignment(Qt::AlignCenter);
    recentDevicesIcon->setFixedSize(52, 52);
    ui::RemoteCTheme::SetPixmap(
        recentDevicesIcon, QStringLiteral(":/ui/icons/light/devices.svg"),
        QSize(26, 26), ui::ThemeIconTone::kPrimary);
    recentDevicesEmptyLayout->addWidget(recentDevicesIcon, 0, Qt::AlignCenter);
    auto* recentDevicesEmptyTitle = new QLabel(
        QStringLiteral("暂无最近设备"), recentDevicesEmptyState_);
    recentDevicesEmptyTitle->setObjectName(QStringLiteral("recentEmptyTitle"));
    recentDevicesEmptyLayout->addWidget(
        recentDevicesEmptyTitle, 0, Qt::AlignCenter);
    auto* recentDevicesHint = new QLabel(
        QStringLiteral("成功进入远程桌面后，这里会保留最近使用的 5 台设备。"),
        recentDevicesEmptyState_);
    recentDevicesHint->setObjectName(QStringLiteral("recentEmptyHint"));
    recentDevicesHint->setAlignment(Qt::AlignCenter);
    recentDevicesHint->setWordWrap(false);
    recentDevicesHint->setSizePolicy(
        QSizePolicy::Preferred, QSizePolicy::Fixed);
    recentDevicesEmptyLayout->addWidget(recentDevicesHint);
    recentDevicesLayout_->addWidget(recentDevicesEmptyState_);
    recentPageLayout->addLayout(recentDevicesLayout_);

    recentPageLayout->addSpacing(18);

    auto* recentRoomsHeader = new QHBoxLayout();
    auto* recentRoomsTitle = new QLabel(
        QStringLiteral("最近房间"), recentPage);
    recentRoomsTitle->setObjectName(QStringLiteral("recentSectionTitle"));
    recentRoomsHeader->addWidget(recentRoomsTitle);
    auto* recentRoomsLimit = new QLabel(
        QStringLiteral("最多 3 个"), recentPage);
    recentRoomsLimit->setObjectName(QStringLiteral("recentLimitPill"));
    recentRoomsHeader->addWidget(recentRoomsLimit);
    recentRoomsHeader->addStretch(1);
    recentPageLayout->addLayout(recentRoomsHeader);

    recentRoomsLayout_ = new QVBoxLayout();
    recentRoomsLayout_->setSpacing(10);
    recentEmptyState_ = new QFrame(recentPage);
    recentEmptyState_->setObjectName(QStringLiteral("recentEmptyState"));
    auto* recentEmptyLayout = new QVBoxLayout(recentEmptyState_);
    recentEmptyLayout->setContentsMargins(26, 26, 26, 28);
    recentEmptyLayout->setSpacing(11);
    recentEmptyLayout->setAlignment(Qt::AlignCenter);
    auto* recentIcon = new QLabel(recentEmptyState_);
    recentIcon->setObjectName(QStringLiteral("recentEmptyIcon"));
    recentIcon->setAlignment(Qt::AlignCenter);
    recentIcon->setFixedSize(52, 52);
    ui::RemoteCTheme::SetPixmap(
        recentIcon, QStringLiteral(":/ui/icons/light/room.svg"),
        QSize(26, 26), ui::ThemeIconTone::kPrimary);
    recentEmptyLayout->addWidget(recentIcon, 0, Qt::AlignCenter);
    auto* recentTitle = new QLabel(
        QStringLiteral("暂无最近房间"), recentEmptyState_);
    recentTitle->setObjectName(QStringLiteral("recentEmptyTitle"));
    recentEmptyLayout->addWidget(recentTitle, 0, Qt::AlignCenter);
    auto* recentHint = new QLabel(
        QStringLiteral("创建或加入房间后，这里会保留最近使用的 3 个房间。"),
        recentEmptyState_);
    recentHint->setObjectName(QStringLiteral("recentEmptyHint"));
    recentHint->setAlignment(Qt::AlignCenter);
    recentHint->setWordWrap(false);
    recentHint->setSizePolicy(
        QSizePolicy::Preferred, QSizePolicy::Fixed);
    recentEmptyLayout->addWidget(recentHint);
    recentRoomsLayout_->addWidget(recentEmptyState_);
    recentPageLayout->addLayout(recentRoomsLayout_);
    recentPageLayout->addStretch(1);
    pageStack_->addWidget(recentPage);
