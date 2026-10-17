// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#include "ControllerMainWindow.h"
#include "ControllerMainWindowSupport.h"

#include <QApplication>
#include <QEasingCurve>
#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QPropertyAnimation>
#include <QPushButton>
#include <QScreen>
#include <QStackedWidget>
#include <QTimer>
#include <QToolButton>
#include <QVBoxLayout>

#include <algorithm>

#include "src/apps/controller/ui/RemoteCTheme.h"
#include "pages/RoomPage.h"

namespace remote::controller {

using namespace detail;

void ControllerMainWindow::BuildShellAndRoomPage()
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
        QStringLiteral("我的设备"), NavigationIcon::kOwnedDevices, false,
        sidebar);
    recentNavButton_ = MakeNavigationButton(
        QStringLiteral("最近连接"), NavigationIcon::kRecent, false, sidebar);
    sidebarLayout->addWidget(roomNavButton_);
    sidebarLayout->addWidget(deviceNavButton_);
    sidebarLayout->addWidget(myDevicesNavButton_);
    sidebarLayout->addWidget(recentNavButton_);
    fileTransferNavButton_ = MakeNavigationButton(
        QStringLiteral("文件传输"), NavigationIcon::kTransfer, false,
        sidebar);
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
        QStringLiteral("问题反馈"), NavigationIcon::kHelp, false, sidebar);
    authorNavButton_ = MakeNavigationButton(
        QStringLiteral("找到作者"), NavigationIcon::kAuthor, false, sidebar);
    sidebarLayout->addWidget(settingsNavButton_);
    sidebarLayout->addWidget(helpNavButton_);
    sidebarLayout->addWidget(authorNavButton_);
    pageNavigationButtons_ = {
        roomNavButton_, deviceNavButton_, myDevicesNavButton_,
        recentNavButton_, debugNavButton_, settingsNavButton_,
        helpNavButton_, authorNavButton_};
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
    serviceStatus_ =
        new QLabel(QStringLiteral("● 本地引擎启动中"), profileCard_);
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
    profileUpdateButton_->setAccessibleName(QStringLiteral("下载软件更新"));
    profileUpdateButton_->setFixedSize(32, 32);
    profileUpdateButton_->setIconSize(QSize(18, 18));
    ui::RemoteCTheme::SetIcon(
        profileUpdateButton_,
        QStringLiteral(":/ui/icons/lucide/base/download.svg"),
        ui::ThemeIconTone::kPrimary);
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
            button,
            QStringLiteral(":/ui/icons/lucide/base/%1.svg").arg(icon),
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

    roomPage_ = new RoomPage(central);
    connectivityPill_ = RoomControls().connectivityLabel;
    connect(roomPage_, &RoomPage::MemberContextMenuRequested,
            this, &ControllerMainWindow::ShowRoomMemberContextMenu);
    connect(roomPage_, &RoomPage::MediaDeviceMenuRequested,
            this, &ControllerMainWindow::ShowMediaDeviceMenu);

    pageStack_ = new QStackedWidget(central);
    pageStack_->addWidget(roomPage_);
    shell->addWidget(pageStack_, 1);
    outer->addWidget(body, 1);
    setCentralWidget(central);
}

}  // namespace remote::controller
