// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#include "ControllerMainWindow.LocalDeviceV2Patch.inc"

    QVBoxLayout* ownedDevicesPageLayout = nullptr;
    auto* ownedDevicesPage = MakePageSurface(
        QStringLiteral("我的设备"),
        QStringLiteral("同一 RLink 账户下的电脑；在线设备可直接进入桌面。"),
        pageStack_, &ownedDevicesPageLayout);
    ownedDevicesPage->setObjectName(QStringLiteral("ownedDevicesPage"));
    ownedDevicesPage_ = ownedDevicesPage;
    {
        QFile pageStyle(QStringLiteral(":/ui/theme/owned-devices.qss"));
        if (pageStyle.open(QIODevice::ReadOnly | QIODevice::Text)) {
            ownedDevicesPage->setStyleSheet(QString::fromUtf8(pageStyle.readAll()));
        }
    }

    auto* ownedHeader = new QFrame(ownedDevicesPage);
    ownedHeader->setProperty("card", true);
    ownedHeader->setObjectName(QStringLiteral("ownedDevicesSummaryCard"));
    auto* ownedHeaderLayout = new QHBoxLayout(ownedHeader);
    ownedHeaderLayout->setContentsMargins(22, 18, 18, 18);
    ownedHeaderLayout->setSpacing(16);

    auto* ownedHeaderIcon = new QLabel(ownedHeader);
    ownedHeaderIcon->setObjectName(QStringLiteral("ownedDevicesHeaderIcon"));
    ownedHeaderIcon->setAlignment(Qt::AlignCenter);
    ownedHeaderIcon->setFixedSize(48, 48);
    ui::RemoteCTheme::SetPixmap(
        ownedHeaderIcon, QStringLiteral(":/ui/icons/light/devices.svg"),
        QSize(26, 26), ui::ThemeIconTone::kPrimary);
    ownedHeaderLayout->addWidget(ownedHeaderIcon);

    auto* ownedHeaderText = new QVBoxLayout();
    ownedHeaderText->setSpacing(3);
    auto* ownedHeaderTitle = new QLabel(
        QStringLiteral("同账号设备"), ownedHeader);
    ownedHeaderTitle->setObjectName(QStringLiteral("ownedDevicesHeaderTitle"));
    ownedDevicesSummaryLabel_ = new QLabel(
        QStringLiteral("正在读取设备列表…"), ownedHeader);
    ownedDevicesSummaryLabel_->setObjectName(
        QStringLiteral("ownedDevicesSummaryText"));
    ownedHeaderText->addWidget(ownedHeaderTitle);
    ownedHeaderText->addWidget(ownedDevicesSummaryLabel_);
    ownedHeaderLayout->addLayout(ownedHeaderText, 1);

    auto* scopePill = new QLabel(QStringLiteral("安全设备列表"), ownedHeader);
    scopePill->setObjectName(QStringLiteral("ownedDevicesScopePill"));
    scopePill->setAlignment(Qt::AlignCenter);
    ownedHeaderLayout->addWidget(scopePill, 0, Qt::AlignVCenter);

    ownedDevicesRefreshButton_ = new QPushButton(ownedHeader);
    ownedDevicesRefreshButton_->setObjectName(
        QStringLiteral("ownedDevicesRefreshButton"));
    ownedDevicesRefreshButton_->setIconSize(QSize(18, 18));
    ui::RemoteCTheme::SetIcon(
        ownedDevicesRefreshButton_,
        QStringLiteral(":/ui/icons/actions/refresh.svg"),
        ui::ThemeIconTone::kPrimary);
    remotec::ui::morph::MorphIconButtonBinding::attach(
        ownedDevicesRefreshButton_,
        QStringLiteral(":/ui/icons/lucide/base/refresh-cw.svg"),
        QStringLiteral(":/ui/icons/lucide/base/refresh-ccw.svg"),
        remotec::ui::morph::MorphIconButtonBinding::Interaction::Feedback,
        QSize(18, 18), QColor(QStringLiteral("#2563EB")),
        QColor(QStringLiteral("#2563EB")));
    ownedDevicesRefreshButton_->setToolTip(QStringLiteral("刷新设备列表"));
    ownedDevicesRefreshButton_->setCursor(Qt::PointingHandCursor);
    ownedDevicesRefreshButton_->setFixedSize(42, 42);
    ownedHeaderLayout->addWidget(ownedDevicesRefreshButton_);
    ownedDevicesPageLayout->addWidget(ownedHeader);

    ownedDevicesCardsLayout_ = new QVBoxLayout();
    ownedDevicesCardsLayout_->setSpacing(10);
    ownedDevicesPageLayout->addLayout(ownedDevicesCardsLayout_);

    ownedDevicesEmptyState_ = new QFrame(ownedDevicesPage);
    ownedDevicesEmptyState_->setObjectName(
        QStringLiteral("ownedDevicesEmptyState"));
    auto* ownedEmptyLayout = new QVBoxLayout(ownedDevicesEmptyState_);
    ownedEmptyLayout->setContentsMargins(32, 30, 32, 34);
    ownedEmptyLayout->setSpacing(11);
    ownedEmptyLayout->setAlignment(Qt::AlignCenter);

    ownedDevicesEmptyArtwork_ = new QLabel(ownedDevicesEmptyState_);
    ownedDevicesEmptyArtwork_->setAlignment(Qt::AlignCenter);
    ownedDevicesEmptyArtwork_->setFixedSize(220, 128);
    ownedDevicesEmptyArtwork_->setPixmap(
        QIcon(QStringLiteral(":/ui/illustrations/devices/no-devices.svg"))
            .pixmap(210, 122));
    ownedEmptyLayout->addWidget(
        ownedDevicesEmptyArtwork_, 0, Qt::AlignCenter);
    ownedEmptyLayout->addSpacing(2);

    auto* ownedEmptyTitle = new QLabel(
        QStringLiteral("还没有其他设备"), ownedDevicesEmptyState_);
    ownedEmptyTitle->setObjectName(QStringLiteral("ownedDevicesEmptyTitle"));
    ownedEmptyLayout->addWidget(ownedEmptyTitle, 0, Qt::AlignCenter);

    auto* ownedEmptyHint = new QLabel(
        QStringLiteral("在另一台电脑登录同一账户后，它会自动出现在这里。"),
        ownedDevicesEmptyState_);
    ownedEmptyHint->setObjectName(QStringLiteral("ownedDevicesEmptyHint"));
    ownedEmptyHint->setAlignment(Qt::AlignCenter);
    ownedEmptyHint->setWordWrap(false);
    ownedEmptyHint->setSizePolicy(
        QSizePolicy::Preferred, QSizePolicy::Fixed);
    ownedEmptyLayout->addWidget(ownedEmptyHint);

    ownedDevicesPageLayout->addWidget(ownedDevicesEmptyState_);
    ownedDevicesPageLayout->addStretch(1);
    pageStack_->addWidget(ownedDevicesPage);
