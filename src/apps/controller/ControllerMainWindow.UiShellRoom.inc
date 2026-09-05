// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

void ControllerMainWindow::BuildUi()
{
    setWindowTitle(QStringLiteral("RLink - 协作房间"));
    QSize initialSize(1156, 731);
    if (const auto* screen = QApplication::primaryScreen()) {
        const QRect available = screen->availableGeometry();
        initialSize.setWidth(std::max(
            980, qRound(static_cast<qreal>(available.width()) * 0.8)));
        initialSize.setHeight(std::max(
            680, qRound(static_cast<qreal>(available.height()) * 0.6)));
        initialSize = initialSize.boundedTo(available.size());
        setMinimumSize(
            std::min(1060, initialSize.width()),
            std::min(700, initialSize.height()));
        resize(initialSize);
        move(available.center() - rect().center());
    } else {
        setMinimumSize(1060, 700);
        resize(initialSize);
    }
    ApplyUiStyleSheet(QString::fromUtf8(kMainStyle));

    auto* central = new QWidget(this);
    auto* outer = new QVBoxLayout(central);
    outer->setContentsMargins(0, 0, 0, 0);
    outer->setSpacing(0);
    titleBar_ = new CustomTitleBar(
        this, QStringLiteral("协作房间"), central);
    outer->addWidget(titleBar_);

    auto* body = new QWidget(central);
    auto* shell = new QHBoxLayout(body);
    shell->setContentsMargins(0, 0, 0, 0);
    shell->setSpacing(0);

    auto* sidebar = new QFrame(central);
    sidebar->setObjectName(QStringLiteral("sidebar"));
    sidebar->setFixedWidth(143);
    auto* sidebarLayout = new QVBoxLayout(sidebar);
    sidebarLayout->setContentsMargins(12, 16, 12, 14);
    sidebarLayout->setSpacing(6);

    auto* menuCaption = new QLabel(QStringLiteral("工作台"), sidebar);
    menuCaption->setObjectName(QStringLiteral("sidebarCaption"));
    sidebarLayout->addWidget(menuCaption);
    sidebarLayout->addSpacing(3);
    roomNavButton_ = MakeNavigationButton(
        QStringLiteral("协作房间"), NavigationIcon::kRoom, true, sidebar);
    deviceNavButton_ = MakeNavigationButton(
        QStringLiteral("远程协助"), NavigationIcon::kDevice, false, sidebar);
    myDevicesNavButton_ = MakeNavigationButton(
        QStringLiteral("我的设备"), NavigationIcon::kOwnedDevices, false, sidebar);
    recentNavButton_ = MakeNavigationButton(
        QStringLiteral("最近连接"), NavigationIcon::kRecent, false, sidebar);
    sidebarLayout->addWidget(roomNavButton_);
    sidebarLayout->addWidget(deviceNavButton_);
    sidebarLayout->addWidget(myDevicesNavButton_);
    sidebarLayout->addWidget(recentNavButton_);
    fileTransferNavButton_ = MakeNavigationButton(
        QStringLiteral("文件传输"), NavigationIcon::kTransfer, false, sidebar);
    fileTransferNavButton_->setCheckable(false);
    fileTransferNavButton_->setFocusPolicy(Qt::NoFocus);
    sidebarLayout->addWidget(fileTransferNavButton_);
    debugNavButton_ = MakeNavigationButton(
        QStringLiteral("调试信息"), NavigationIcon::kDebug, false, sidebar);
    sidebarLayout->addWidget(debugNavButton_);
    sidebarLayout->addSpacing(18);

    auto* toolsCaption = new QLabel(QStringLiteral("系统"), sidebar);
    toolsCaption->setObjectName(QStringLiteral("sidebarCaption"));
    sidebarLayout->addWidget(toolsCaption);
    sidebarLayout->addSpacing(3);
    settingsNavButton_ = MakeNavigationButton(
        QStringLiteral("设置"), NavigationIcon::kSettings, false, sidebar);
    helpNavButton_ = MakeNavigationButton(
        QStringLiteral("帮助与反馈"), NavigationIcon::kHelp, false, sidebar);
    authorNavButton_ = MakeNavigationButton(
        QStringLiteral("找到作者"), NavigationIcon::kAuthor, false, sidebar);
    sidebarLayout->addWidget(settingsNavButton_);
    sidebarLayout->addWidget(helpNavButton_);
    sidebarLayout->addWidget(authorNavButton_);
    pageNavigationButtons_ = {
        roomNavButton_, deviceNavButton_, myDevicesNavButton_, recentNavButton_,
        debugNavButton_, settingsNavButton_, helpNavButton_, authorNavButton_};
    navigationIndicator_ = new QFrame(sidebar);
    navigationIndicator_->setObjectName(
        QStringLiteral("navigationIndicator"));
    navigationIndicator_->setFixedWidth(3);
    navigationIndicator_->raise();
    navigationAnimation_ = new QPropertyAnimation(
        navigationIndicator_, "geometry", this);
    navigationAnimation_->setEasingCurve(QEasingCurve::OutCubic);
    QTimer::singleShot(0, this, [this] {
        AnimateNavigationIndicator(roomNavButton_);
    });
    sidebarLayout->addStretch(1);

    profileCard_ = new QFrame(sidebar);
    profileCard_->setObjectName(QStringLiteral("profileCard"));
    profileCard_->setCursor(Qt::PointingHandCursor);
    auto* profileRow = new QHBoxLayout(profileCard_);
    profileRow->setContentsMargins(10, 10, 10, 10);
    profileRow->setSpacing(9);
    profileAvatar_ = new QLabel(QStringLiteral("本"), profileCard_);
    profileAvatar_->setObjectName(QStringLiteral("profileAvatar"));
    profileAvatar_->setAlignment(Qt::AlignCenter);
    profileAvatar_->setFixedSize(34, 34);
    profileRow->addWidget(profileAvatar_);
    auto* profileLabels = new QVBoxLayout();
    profileLabels->setSpacing(0);
    profileName_ = new QLabel(QStringLiteral("本机设备"), profileCard_);
    profileName_->setObjectName(QStringLiteral("profileName"));
    serviceStatus_ = new QLabel(QStringLiteral("● 本地引擎启动中"), profileCard_);
    serviceStatus_->setObjectName(QStringLiteral("profileState"));
    serviceStatus_->hide();
    profileLabels->setAlignment(Qt::AlignVCenter);
    profileLabels->addWidget(profileName_);
    profileRow->addLayout(profileLabels, 1);
    profileUpdateButton_ = new QToolButton(profileCard_);
    profileUpdateButton_->setObjectName(
        QStringLiteral("profileUpdateButton"));
    profileUpdateButton_->setCursor(Qt::PointingHandCursor);
    profileUpdateButton_->setToolTip(QStringLiteral("有新版本可用"));
    profileUpdateButton_->setFixedSize(30, 30);
    profileUpdateButton_->setIconSize(QSize(18, 18));
    ui::RemoteCTheme::SetIcon(
        profileUpdateButton_,
        QStringLiteral(":/ui/icons/lucide/base/download.svg"),
        ui::ThemeIconTone::kOnDark);
    profileUpdateButton_->hide();
    profileRow->addWidget(profileUpdateButton_, 0, Qt::AlignVCenter);
    connect(profileUpdateButton_, &QToolButton::clicked,
            this, [this] { OpenSoftwareUpdate(); });
    sidebarLayout->addWidget(profileCard_);
    shell->addWidget(sidebar);

    accountMenu_ = new QFrame(central);
    accountMenu_->setObjectName(QStringLiteral("accountMenu"));
    accountMenu_->setFixedSize(252, 317);
    accountMenu_->setAttribute(Qt::WA_Hover, true);
    accountMenu_->setMouseTracking(true);
    accountMenu_->hide();
    auto* accountMenuLayout = new QVBoxLayout(accountMenu_);
    accountMenuLayout->setContentsMargins(12, 12, 12, 12);
    accountMenuLayout->setSpacing(5);

    auto* accountHeader = new QWidget(accountMenu_);
    accountHeader->setObjectName(QStringLiteral("accountMenuHeader"));
    auto* accountHeaderLayout = new QHBoxLayout(accountHeader);
    accountHeaderLayout->setContentsMargins(6, 3, 6, 7);
    accountHeaderLayout->setSpacing(10);
    accountMenuAvatar_ = new QLabel(QStringLiteral("R"), accountHeader);
    accountMenuAvatar_->setObjectName(QStringLiteral("accountMenuAvatar"));
    accountMenuAvatar_->setAlignment(Qt::AlignCenter);
    accountMenuAvatar_->setFixedSize(38, 38);
    accountHeaderLayout->addWidget(accountMenuAvatar_);
    auto* accountHeaderText = new QVBoxLayout();
    accountHeaderText->setSpacing(1);
    accountMenuName_ = new QLabel(QStringLiteral("RLink 用户"), accountHeader);
    accountMenuName_->setObjectName(QStringLiteral("accountMenuName"));
    accountMenuDetail_ = new QLabel(accountHeader);
    accountMenuDetail_->setObjectName(QStringLiteral("accountMenuDetail"));
    accountMenuDetail_->setTextInteractionFlags(Qt::NoTextInteraction);
    accountHeaderText->addWidget(accountMenuName_);
    accountHeaderText->addWidget(accountMenuDetail_);
    accountHeaderLayout->addLayout(accountHeaderText, 1);
    accountMenuLayout->addWidget(accountHeader);

    auto* accountSeparator = new QFrame(accountMenu_);
    accountSeparator->setObjectName(QStringLiteral("accountMenuSeparator"));
    accountSeparator->setFixedHeight(1);
    accountMenuLayout->addWidget(accountSeparator);

    const auto makeAccountAction = [this, accountMenuLayout](
            const QString& text, const QString& icon,
            const QString& tone = QString()) {
        auto* button = new QPushButton(text, accountMenu_);
        button->setObjectName(QStringLiteral("accountMenuAction"));
        button->setProperty("tone", tone);
        button->setCursor(Qt::PointingHandCursor);
        button->setAttribute(Qt::WA_Hover, true);
        button->setMouseTracking(true);
        button->setFixedHeight(40);
        button->setIconSize(QSize(18, 18));
        button->setStyleSheet({});
        ui::RemoteCTheme::SetIcon(
            button, QStringLiteral(":/ui/icons/lucide/base/%1.svg").arg(icon),
            tone == QStringLiteral("danger")
                ? ui::ThemeIconTone::kDanger
                : ui::ThemeIconTone::kNeutral);
        accountMenuLayout->addWidget(button);
        return button;
    };
    softwareUpdateAction_ = makeAccountAction(
        QStringLiteral("软件更新"), QStringLiteral("download"));
    auto* accountCenterAction = makeAccountAction(
        QStringLiteral("账户中心"), QStringLiteral("circle-user-round"));
    auto* switchAccountAction = makeAccountAction(
        QStringLiteral("切换用户"), QStringLiteral("refresh-cw"));
    auto* signOutAction = makeAccountAction(
        QStringLiteral("退出登录"), QStringLiteral("log-out"));
    auto* deleteAccountAction = makeAccountAction(
        QStringLiteral("注销账号"), QStringLiteral("trash-2"),
        QStringLiteral("danger"));

    connect(softwareUpdateAction_, &QPushButton::clicked, this, [this] {
        HideAccountMenu();
        OpenSoftwareUpdate();
    });
    connect(accountCenterAction, &QPushButton::clicked, this, [this] {
        HideAccountMenu();
        if (accountInteractionCallback_) accountInteractionCallback_();
    });
    connect(switchAccountAction, &QPushButton::clicked, this, [this] {
        HideAccountMenu();
        if (accountSwitchCallback_) accountSwitchCallback_();
    });
    connect(signOutAction, &QPushButton::clicked, this, [this] {
        HideAccountMenu();
        if (signOutCallback_) signOutCallback_();
    });
    connect(deleteAccountAction, &QPushButton::clicked, this, [this] {
        HideAccountMenu();
        if (accountDeletionCallback_) accountDeletionCallback_();
    });

    auto* scrollArea = new QScrollArea(central);
    scrollArea->setWidgetResizable(true);
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    // Keep the content width stable when room status text changes between
    // one and two lines. Otherwise the vertical scrollbar appearing for a
    // moment makes the complete room page jump horizontally.
    scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    EnableSmoothWheelScrolling(scrollArea);
    auto* content = new QWidget(scrollArea);
    content->setObjectName(QStringLiteral("content"));
    auto* contentLayout = new QVBoxLayout(content);
    contentLayout->setContentsMargins(32, 28, 32, 30);
    contentLayout->setSpacing(20);

    auto* headerRow = new QHBoxLayout();
    auto* roomHeaderIcon = new QLabel(content);
    roomHeaderIcon->setObjectName(QStringLiteral("roomHeaderIcon"));
    roomHeaderIcon->setFixedSize(44, 44);
    roomHeaderIcon->setAlignment(Qt::AlignCenter);
    roomHeaderIcon->setStyleSheet(QStringLiteral(
        "background:#0b7cff;border-radius:11px;"));
    QPixmap roomHeaderPixmap(30, 30);
    roomHeaderPixmap.fill(Qt::transparent);
    {
        QPainter iconPainter(&roomHeaderPixmap);
        iconPainter.setRenderHint(QPainter::Antialiasing, true);
        iconPainter.setPen(QPen(Qt::white, 2.4, Qt::SolidLine,
                                Qt::RoundCap, Qt::RoundJoin));
        iconPainter.drawRoundedRect(QRectF(3, 4, 24, 17), 2, 2);
        iconPainter.drawLine(QPointF(15, 21), QPointF(15, 26));
        iconPainter.drawLine(QPointF(10, 27), QPointF(20, 27));
    }
    roomHeaderIcon->setPixmap(roomHeaderPixmap);
    headerRow->addWidget(roomHeaderIcon, 0, Qt::AlignTop);
    auto* headerLabels = new QVBoxLayout();
    headerLabels->setSpacing(3);
    auto* title = new QLabel(QStringLiteral("协作房间"), content);
    title->setObjectName(QStringLiteral("pageTitle"));
    auto* subtitle = new QLabel(
        QStringLiteral("创建或加入房间；任意成员之后都可以成为主屏幕分享者"), content);
    subtitle->setObjectName(QStringLiteral("pageSubtitle"));
    headerLabels->addWidget(title);
    headerLabels->addWidget(subtitle);
    headerRow->addLayout(headerLabels, 1);
    connectivityPill_ = new QLabel(QStringLiteral("●  信令未连接"), content);
    connectivityPill_->setObjectName(QStringLiteral("readyPill"));
    headerRow->addWidget(connectivityPill_, 0, Qt::AlignTop);
    contentLayout->addLayout(headerRow);

    auto* columns = new QHBoxLayout();
    columns->setSpacing(18);

    auto* primaryColumn = new QVBoxLayout();
    primaryColumn->setSpacing(18);

    auto* connectCard = new QFrame(content);
    roomEntryPanel_ = connectCard;
    connectCard->setProperty("card", true);
    auto* connectLayout = new QVBoxLayout(connectCard);
    connectLayout->setContentsMargins(22, 21, 22, 22);
    connectLayout->setSpacing(13);
    auto* eyebrow = new QLabel(QStringLiteral("房间入口"), connectCard);
    eyebrow->setObjectName(QStringLiteral("eyebrow"));
    connectLayout->addWidget(eyebrow);
    auto* connectTitle =
        new QLabel(QStringLiteral("创建或加入协作房间"), connectCard);
    connectTitle->setObjectName(QStringLiteral("cardTitle"));
    connectLayout->addWidget(connectTitle);
    auto* connectHint = new QLabel(
        QStringLiteral(
            "房间人数可以动态增加到设置的上限。默认 2 人时，同一套逻辑就是普通的两人远控。"),
        connectCard);
    connectHint->setProperty("muted", true);
    connectHint->setWordWrap(true);
    connectLayout->addWidget(connectHint);

    auto* createCaption = new QLabel(QStringLiteral("创建房间"), connectCard);
    createCaption->setObjectName(QStringLiteral("deviceName"));
    connectLayout->addWidget(createCaption);
    auto* createRow = new QHBoxLayout();
    createRow->setSpacing(10);
    auto* capacityCaption = new QLabel(QStringLiteral("人数上限"), connectCard);
    capacityCaption->setProperty("muted", true);
    createRow->addWidget(capacityCaption);
    createRoomCapacity_ = new RemoteCComboBox(connectCard);
    createRoomCapacity_->setObjectName(QStringLiteral("capacitySelector"));
    AddRoomCapacityItems(createRoomCapacity_);
    createRoomCapacity_->setFixedHeight(43);
    createRoomCapacity_->setEnabled(false);
    createRow->addWidget(createRoomCapacity_);
    createRow->addStretch(1);
    createRoomButton_ = new QPushButton(QStringLiteral("创建房间"), connectCard);
    createRoomButton_->setObjectName(QStringLiteral("primaryButton"));
    createRoomButton_->setCursor(Qt::PointingHandCursor);
    createRoomButton_->setFixedHeight(43);
    createRoomButton_->setEnabled(false);
    createRow->addWidget(createRoomButton_);
    connectLayout->addLayout(createRow);
    connectLayout->addWidget(MakeDivider(connectCard));

    auto* joinCaption = new QLabel(QStringLiteral("加入已有房间"), connectCard);
    joinCaption->setObjectName(QStringLiteral("deviceName"));
    connectLayout->addWidget(joinCaption);
    auto* joinRow = new QHBoxLayout();
    joinRow->setSpacing(10);
    roomIdEdit_ = new QLineEdit(connectCard);
    roomIdEdit_->setObjectName(QStringLiteral("roomIdInput"));
    roomIdEdit_->setPlaceholderText(QStringLiteral("输入 9 位房间号"));
    roomIdEdit_->setMaxLength(9);
    roomIdEdit_->setValidator(new QRegularExpressionValidator(
        QRegularExpression(QStringLiteral("[0-9]{0,9}")), roomIdEdit_));
    roomIdEdit_->setClearButtonEnabled(true);
    roomIdEdit_->setContextMenuPolicy(Qt::NoContextMenu);
    roomIdEdit_->setFixedHeight(43);
    roomIdEdit_->setEnabled(false);
    joinRow->addWidget(roomIdEdit_, 1);
    joinRoomButton_ = new QPushButton(QStringLiteral("申请加入  →"), connectCard);
    joinRoomButton_->setObjectName(QStringLiteral("primaryButton"));
    joinRoomButton_->setCursor(Qt::PointingHandCursor);
    joinRoomButton_->setFixedHeight(43);
    joinRoomButton_->setEnabled(false);
    joinRow->addWidget(joinRoomButton_);
    connectLayout->addLayout(joinRow);

    roomActionHint_ = new QLabel(
        QStringLiteral("正在启动本地引擎并检查 WSS 房间控制面。"), connectCard);
    roomActionHint_->setObjectName(QStringLiteral("previewHint"));
    roomActionHint_->setWordWrap(true);
    connectLayout->addWidget(roomActionHint_);
    connectionHint_ = roomActionHint_;
    roomPanel_ = new QFrame(content);
    roomPanel_->setObjectName(QStringLiteral("activeRoomSurface"));
    auto* activeRoomShell = new QHBoxLayout(roomPanel_);
    activeRoomShell->setContentsMargins(0, 0, 0, 0);
    activeRoomShell->setSpacing(16);
    auto* activeRoomMainCard = new QFrame(roomPanel_);
    activeRoomMainCard->setProperty("card", true);
    auto* roomLayout = new QVBoxLayout(activeRoomMainCard);
    roomLayout->setContentsMargins(22, 20, 22, 22);
    roomLayout->setSpacing(12);
    auto* roomHeader = new QHBoxLayout();
    auto* roomTitle = new QLabel(QStringLiteral("活动房间"), roomPanel_);
    roomTitle->setObjectName(QStringLiteral("sectionTitle"));
    roomHeader->addWidget(roomTitle);
    roomHeader->addStretch(1);
    roomOccupancyLabel_ = new QLabel(
        QStringLiteral("0 在线 · 0/2 席位"), roomPanel_);
    roomOccupancyLabel_->setObjectName(QStringLiteral("readyPill"));
    roomOccupancyLabel_->setAlignment(Qt::AlignCenter);
    roomOccupancyLabel_->setSizePolicy(
        QSizePolicy::Maximum, QSizePolicy::Fixed);
    roomOccupancyLabel_->setFixedHeight(32);
    roomOccupancyLabel_->setMaximumWidth(176);
    roomHeader->addWidget(
        roomOccupancyLabel_, 0, Qt::AlignTop | Qt::AlignRight);
    roomLayout->addLayout(roomHeader);

    auto* roomIdCaption = new QLabel(QStringLiteral("房间 ID"), roomPanel_);
    roomIdCaption->setProperty("muted", true);
    roomIdLabel_ = new QLabel(QStringLiteral("—"), roomPanel_);
    roomIdLabel_->setObjectName(QStringLiteral("roomIdValue"));
    roomIdLabel_->setTextInteractionFlags(Qt::TextSelectableByMouse);
    auto* roomIdRow = new QHBoxLayout();
    roomIdRow->setSpacing(10);
    roomIdRow->addWidget(roomIdCaption);
    roomIdRow->addWidget(roomIdLabel_, 1);
    copyRoomIdButton_ = new QPushButton(
        QStringLiteral("复制房间 ID"), roomPanel_);
    copyRoomIdButton_->setObjectName(QStringLiteral("softButton"));
    copyRoomIdButton_->setIconSize(QSize(16, 16));
    ui::RemoteCTheme::SetIcon(
        copyRoomIdButton_, QStringLiteral(":/ui/icons/actions/copy.svg"),
        ui::ThemeIconTone::kPrimary);
    remotec::ui::morph::MorphIconButtonBinding::attach(
        copyRoomIdButton_,
        QStringLiteral(":/ui/icons/lucide/base/copy.svg"),
        QStringLiteral(":/ui/icons/lucide/base/circle-check-big.svg"),
        remotec::ui::morph::MorphIconButtonBinding::Interaction::Feedback,
        QSize(16, 16), QColor(QStringLiteral("#2563EB")),
        QColor(QStringLiteral("#12B76A")));
    copyRoomIdButton_->setCursor(Qt::PointingHandCursor);
    copyRoomIdButton_->setEnabled(false);
    copyRoomIdButton_->ensurePolished();
    const int copyRoomIdWidth = copyRoomIdButton_->sizeHint().width();
    copyRoomIdButton_->setText(QStringLiteral("已复制"));
    copyRoomIdButton_->setFixedWidth(
        std::max(copyRoomIdWidth, copyRoomIdButton_->sizeHint().width()));
    copyRoomIdButton_->setText(QStringLiteral("复制房间 ID"));
    roomIdRow->addWidget(copyRoomIdButton_);
    roomLayout->addLayout(roomIdRow);

    auto* memberPanel = new QFrame(roomPanel_);
    memberPanel->setProperty("card", true);
    memberPanel->setObjectName(QStringLiteral("roomMembersPanel"));
    auto* memberLayout = new QVBoxLayout(memberPanel);
    memberLayout->setContentsMargins(16, 16, 16, 14);
    memberLayout->setSpacing(10);
    roomMemberSummaryLabel_ = new QLabel(
        QStringLiteral("房间成员  (0/2)"), memberPanel);
    roomMemberSummaryLabel_->setObjectName(QStringLiteral("sectionTitle"));
    memberLayout->addWidget(roomMemberSummaryLabel_);
    memberLayout->addWidget(MakeDivider(memberPanel));
    roomMemberList_ = new QListWidget(memberPanel);
    roomMemberList_->setObjectName(QStringLiteral("roomMemberList"));
    roomMemberList_->setSelectionMode(QAbstractItemView::NoSelection);
    roomMemberList_->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(roomMemberList_, &QListWidget::customContextMenuRequested,
            this, &ControllerMainWindow::ShowRoomMemberContextMenu);
    roomMemberList_->setMinimumHeight(360);
    EnableSmoothWheelScrolling(roomMemberList_);
    memberLayout->addWidget(roomMemberList_, 1);
    memberLayout->addWidget(MakeDivider(memberPanel));
    roomMemberFooterLabel_ = new QLabel(
        QStringLiteral("人数上限：2 人        0 在线"), memberPanel);
    roomMemberFooterLabel_->setProperty("muted", true);
    memberLayout->addWidget(roomMemberFooterLabel_);

    auto* statusPanel = new QFrame(roomPanel_);
    statusPanel->setObjectName(QStringLiteral("roomSectionPanel"));
    auto* roomStatusLayout = new QVBoxLayout(statusPanel);
    roomStatusLayout->setContentsMargins(14, 12, 14, 12);
    roomStatusLayout->setSpacing(8);
    auto* roomStatusTitle =
        new QLabel(QStringLiteral("房间状态"), statusPanel);
    roomStatusTitle->setObjectName(QStringLiteral("roomMediaTitle"));
    roomStatusLayout->addWidget(roomStatusTitle);
    auto* roomInfo = new QGridLayout();
    roomInfo->setHorizontalSpacing(12);
    roomInfo->setVerticalSpacing(6);
    auto addRoomInfoRow = [statusPanel, roomInfo](
                              int row, int column, RoomStatusIcon icon,
                              const QString& caption,
                              QLabel** valueLabel) {
        const int baseColumn = column * 4;
        roomInfo->addWidget(
            CreateRoomStatusIndicator(statusPanel, icon),
            row, baseColumn, Qt::AlignVCenter);
        auto* label = new QLabel(caption, statusPanel);
        label->setProperty("muted", true);
        roomInfo->addWidget(label, row, baseColumn + 1);
        *valueLabel = new QLabel(QStringLiteral("—"), statusPanel);
        (*valueLabel)->setProperty("roomValue", true);
        (*valueLabel)->setTextInteractionFlags(Qt::TextSelectableByMouse);
        (*valueLabel)->setWordWrap(false);
        // Transient values such as "switching to ..." must not change the
        // status grid's column width and shift the complete panel.
        (*valueLabel)->setSizePolicy(
            QSizePolicy::Ignored, QSizePolicy::Preferred);
        roomInfo->addWidget(*valueLabel, row, baseColumn + 2);
    };
    addRoomInfoRow(0, 0, RoomStatusIcon::kPerson,
                   QStringLiteral("房主"), &roomOwnerLabel_);
    addRoomInfoRow(1, 0, RoomStatusIcon::kScreen,
                   QStringLiteral("当前主机器"), &roomScreenSharerLabel_);
    addRoomInfoRow(2, 0, RoomStatusIcon::kController,
                   QStringLiteral("当前控制者"), &roomControllerLabel_);
    addRoomInfoRow(0, 1, RoomStatusIcon::kNetwork,
                   QStringLiteral("本机 P2P"),
                   &roomPeerConnectivityLabel_);
    roomInfo->setColumnStretch(2, 1);
    roomInfo->setColumnMinimumWidth(3, 24);
    roomInfo->setColumnStretch(6, 1);

    roomInfo->addWidget(CreateRoomStatusIndicator(
                            statusPanel, RoomStatusIcon::kSeats),
                        1, 4, Qt::AlignVCenter);
    auto* activeCapacityCaption =
        new QLabel(QStringLiteral("人数上限"), statusPanel);
    activeCapacityCaption->setProperty("muted", true);
    roomInfo->addWidget(activeCapacityCaption, 1, 5);
    auto* capacityControls = new QHBoxLayout();
    capacityControls->setContentsMargins(0, 0, 0, 0);
    capacityControls->setSpacing(6);
    capacityControls->addStretch(1);
    activeRoomCapacity_ = new RemoteCComboBox(statusPanel);
    activeRoomCapacity_->setObjectName(QStringLiteral("capacitySelector"));
    AddRoomCapacityItems(activeRoomCapacity_);
    activeRoomCapacity_->setEnabled(false);
    activeRoomCapacity_->setFixedHeight(36);
    activeRoomCapacity_->setMinimumWidth(92);
    capacityControls->addWidget(activeRoomCapacity_);
    applyRoomCapacityButton_ =
        new QPushButton(QStringLiteral("应用"), statusPanel);
    applyRoomCapacityButton_->setObjectName(QStringLiteral("softButton"));
    applyRoomCapacityButton_->setCursor(Qt::PointingHandCursor);
    applyRoomCapacityButton_->setEnabled(false);
    applyRoomCapacityButton_->setMinimumHeight(36);
    capacityControls->addWidget(applyRoomCapacityButton_);
    roomInfo->addLayout(capacityControls, 1, 6);
    addRoomInfoRow(2, 1, RoomStatusIcon::kSeats,
                   QStringLiteral("当前席位"),
                   &roomSeatUsageLabel_);
    roomStatusLayout->addLayout(roomInfo);
    roomLayout->addWidget(statusPanel);

    auto* mediaBar = new QFrame(roomPanel_);
    mediaBar->setObjectName(QStringLiteral("roomMediaBar"));
    auto* mediaLayout = new QVBoxLayout(mediaBar);
    mediaLayout->setContentsMargins(11, 10, 11, 11);
    mediaLayout->setSpacing(8);
    auto* mediaTitle = new QLabel(QStringLiteral("快速控制"), mediaBar);
    mediaTitle->setObjectName(QStringLiteral("roomMediaTitle"));
    mediaLayout->addWidget(mediaTitle);
    auto* mediaGrid = new QGridLayout();
    mediaGrid->setHorizontalSpacing(8);
    mediaGrid->setVerticalSpacing(8);
    roomScreenShareButton_ = new QPushButton(mediaBar);
    roomScreenShareButton_->setObjectName(
        QStringLiteral("mediaIconButton"));
    roomScreenShareButton_->setCursor(Qt::PointingHandCursor);
    roomScreenShareButton_->setFocusPolicy(Qt::NoFocus);
    roomScreenShareButton_->setEnabled(false);
    roomScreenShareButton_->setFixedHeight(54);
    SetMediaStateButton(roomScreenShareButton_, MediaStateIcon::kScreen,
                        false, QStringLiteral("共享本机屏幕"));
    mediaGrid->addWidget(roomScreenShareButton_, 0, 0);
    auto* roomCameraDeviceButton =
        new MediaDeviceButton(mediaBar);
    roomCameraButton_ = roomCameraDeviceButton;
    roomCameraButton_->setObjectName(QStringLiteral("mediaIconButton"));
    roomCameraButton_->setCursor(Qt::PointingHandCursor);
    roomCameraButton_->setFocusPolicy(Qt::NoFocus);
    roomCameraButton_->setEnabled(false);
    roomCameraButton_->setFixedHeight(54);
    SetMediaStateButton(roomCameraButton_, MediaStateIcon::kCamera, false,
                        QStringLiteral("开启摄像头；右侧箭头选择设备"));
    roomCameraDeviceButton->SetDeviceMenuHandler(
        [this] {
            ShowMediaDeviceMenu(
                MediaDeviceKind::kCamera, roomCameraButton_);
        });
    mediaGrid->addWidget(roomCameraButton_, 0, 1);
    auto* roomMicrophoneDeviceButton =
        new MediaDeviceButton(mediaBar);
    roomMicrophoneButton_ = roomMicrophoneDeviceButton;
    roomMicrophoneButton_->setObjectName(QStringLiteral("mediaIconButton"));
    roomMicrophoneButton_->setCursor(Qt::PointingHandCursor);
    roomMicrophoneButton_->setFocusPolicy(Qt::NoFocus);
    roomMicrophoneButton_->setEnabled(false);
    roomMicrophoneButton_->setFixedHeight(54);
    SetMediaStateButton(
        roomMicrophoneButton_, MediaStateIcon::kMicrophone, false,
        QStringLiteral("开启麦克风；右侧箭头选择设备"));
    roomMicrophoneDeviceButton->SetDeviceMenuHandler(
        [this] {
            ShowMediaDeviceMenu(
                MediaDeviceKind::kMicrophone,
                roomMicrophoneButton_);
        });
    mediaGrid->addWidget(roomMicrophoneButton_, 0, 2);
    auto* roomSpeakerDeviceButton =
        new MediaDeviceButton(mediaBar);
    roomSpeakerButton_ = roomSpeakerDeviceButton;
    roomSpeakerButton_->setObjectName(QStringLiteral("mediaIconButton"));
    roomSpeakerButton_->setCursor(Qt::PointingHandCursor);
    roomSpeakerButton_->setFocusPolicy(Qt::NoFocus);
    roomSpeakerButton_->setEnabled(false);
    roomSpeakerButton_->setFixedHeight(54);
    SetMediaStateButton(roomSpeakerButton_, MediaStateIcon::kSpeaker, true,
                        QStringLiteral("关闭远端声音；右侧箭头选择设备"));
    roomSpeakerDeviceButton->SetDeviceMenuHandler(
        [this] {
            ShowMediaDeviceMenu(
                MediaDeviceKind::kSpeaker, roomSpeakerButton_);
        });
    mediaGrid->addWidget(roomSpeakerButton_, 0, 3);
    roomCameraGalleryButton_ = new QPushButton(mediaBar);
    roomCameraGalleryButton_->setObjectName(
        QStringLiteral("mediaIconButton"));
    roomCameraGalleryButton_->setCursor(Qt::PointingHandCursor);
    roomCameraGalleryButton_->setFocusPolicy(Qt::NoFocus);
    roomCameraGalleryButton_->setEnabled(false);
    roomCameraGalleryButton_->setFixedHeight(54);
    SetCameraGalleryStateButton(
        roomCameraGalleryButton_, false, false,
        QStringLiteral("摄像头画廊 · 0 人开启"));
    mediaGrid->addWidget(roomCameraGalleryButton_, 0, 4);
    mediaGrid->setColumnStretch(0, 1);
    mediaGrid->setColumnStretch(1, 1);
    mediaGrid->setColumnStretch(2, 1);
    mediaGrid->setColumnStretch(3, 1);
    mediaGrid->setColumnStretch(4, 1);
    mediaLayout->addLayout(mediaGrid);
    roomLayout->addWidget(mediaBar);

    roomStageLabel_ = new QLabel(
        QStringLiteral("房间控制面已连接；成员加入后将自动建立 P2P 连接。"),
        roomPanel_);
    roomStageLabel_->setObjectName(QStringLiteral("roomStageHint"));
    roomStageLabel_->setWordWrap(true);

    auto* roomFooter = new QHBoxLayout();
    roomFooter->setSpacing(12);
    roomFooter->addWidget(roomStageLabel_, 1);
    roomFileTransferButton_ =
        new QPushButton(QStringLiteral("文件传输"), roomPanel_);
    roomFileTransferButton_->setObjectName(QStringLiteral("softButton"));
    roomFileTransferButton_->setCursor(Qt::PointingHandCursor);
    roomFileTransferButton_->setEnabled(false);
    roomFileTransferButton_->setMinimumHeight(40);
    roomFileTransferButton_->hide();
    leaveRoomButton_ = new QPushButton(QStringLiteral("离开房间"), roomPanel_);
    leaveRoomButton_->setObjectName(QStringLiteral("dangerButton"));
    leaveRoomButton_->setCursor(Qt::PointingHandCursor);
    leaveRoomButton_->setEnabled(false);
    roomFooter->addWidget(leaveRoomButton_);
    roomLayout->addLayout(roomFooter);
    activeRoomShell->addWidget(activeRoomMainCard, 2);
    activeRoomShell->addWidget(memberPanel, 1);
    roomWorkspaceStack_ = new CurrentPageStack(content);
    roomWorkspaceStack_->setObjectName(QStringLiteral("roomWorkspaceStack"));
    roomWorkspaceStack_->setSizePolicy(
        QSizePolicy::Expanding, QSizePolicy::Fixed);
    roomWorkspaceStack_->addWidget(roomEntryPanel_);
    roomWorkspaceStack_->addWidget(roomPanel_);
    roomWorkspaceStack_->setCurrentWidget(roomEntryPanel_);
    primaryColumn->addWidget(roomWorkspaceStack_);
    QTimer::singleShot(0, roomWorkspaceStack_, [this] {
        if (auto* stack =
                dynamic_cast<CurrentPageStack*>(roomWorkspaceStack_)) {
            stack->RefreshCurrentHeight();
        }
    });
    primaryColumn->addStretch(1);
    columns->addLayout(primaryColumn, 1);

    contentLayout->addLayout(columns, 1);
    scrollArea->setWidget(content);

    pageStack_ = new QStackedWidget(central);
    pageStack_->addWidget(scrollArea);
