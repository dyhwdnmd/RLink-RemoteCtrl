// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

void ControllerMainWindow::RefreshOwnedDevicesUi(
    const SessionEngineSnapshot& snapshot)
{
    if (!ownedDevicesCardsLayout_ || !ownedDevicesSummaryLabel_) {
        return;
    }
    if (renderedOwnedDevicesRevision_ == snapshot.ownedDevicesRevision &&
        renderedOwnedDevicesConnectivity_ == snapshot.connectivity) {
        return;
    }
    renderedOwnedDevicesRevision_ = snapshot.ownedDevicesRevision;
    renderedOwnedDevicesConnectivity_ = snapshot.connectivity;

    while (QLayoutItem* item = ownedDevicesCardsLayout_->takeAt(0)) {
        if (QWidget* widget = item->widget()) {
            widget->deleteLater();
        }
        delete item;
    }

    const bool online =
        snapshot.connectivity == SessionConnectivityState::kOnline;
    const int onlineCount = static_cast<int>(std::count_if(
        snapshot.ownedDevices.begin(), snapshot.ownedDevices.end(),
        [](const OwnedDeviceSnapshot& device) { return device.online; }));
    ownedDevicesSummaryLabel_->setText(
        !online ? QStringLiteral("信令离线，在线状态暂不可用")
                : QStringLiteral("%1 台设备，%2 台在线")
                      .arg(static_cast<int>(snapshot.ownedDevices.size()))
                      .arg(onlineCount));
    ownedDevicesRefreshButton_->setEnabled(online);

    const bool hasOtherDevice = std::any_of(
        snapshot.ownedDevices.begin(), snapshot.ownedDevices.end(),
        [](const OwnedDeviceSnapshot& device) { return !device.current; });
    ownedDevicesEmptyState_->setVisible(!hasOtherDevice);

    for (const auto& device : snapshot.ownedDevices) {
        auto* card = new QFrame(pageStack_);
        card->setProperty("card", true);
        auto* row = new QHBoxLayout(card);
        row->setContentsMargins(18, 14, 18, 14);
        row->setSpacing(14);
        auto* icon = new QLabel(card);
        icon->setObjectName(QStringLiteral("ownedDeviceIcon"));
        icon->setAlignment(Qt::AlignCenter);
        icon->setFixedSize(52, 52);
        icon->setProperty("online", device.online);
        ui::RemoteCTheme::SetPixmap(
            icon, QStringLiteral(":/ui/icons/light/monitor.svg"),
            QSize(27, 27), device.online ? ui::ThemeIconTone::kPrimary
                                         : ui::ThemeIconTone::kNeutral);
        row->addWidget(icon);

        auto* text = new QVBoxLayout();
        text->setSpacing(4);
        auto* name = new QLabel(
            device.deviceName.empty()
                ? QStringLiteral("RLink 设备")
                : QString::fromStdString(device.deviceName),
            card);
        name->setObjectName(QStringLiteral("cardTitle"));
        auto* details = new QLabel(
            QStringLiteral("设备 ID  %1  ·  %2")
                .arg(QString::fromStdString(device.deviceId),
                     device.current
                         ? QStringLiteral("当前设备")
                         : (device.online ? QStringLiteral("在线")
                                          : QStringLiteral("离线"))),
            card);
        details->setProperty("muted", true);
        text->addWidget(name);
        text->addWidget(details);
        if (!device.online && device.lastSeenAt > 0) {
            auto* lastSeen = new QLabel(
                QStringLiteral("最近在线：%1")
                    .arg(QDateTime::fromSecsSinceEpoch(device.lastSeenAt)
                             .toLocalTime()
                             .toString(QStringLiteral("yyyy-MM-dd HH:mm"))),
                card);
            lastSeen->setProperty("muted", true);
            text->addWidget(lastSeen);
        }
        row->addLayout(text, 1);

        auto* status = new QLabel(
            device.online ? QStringLiteral("● 在线")
                          : QStringLiteral("● 离线"), card);
        status->setObjectName(QStringLiteral("ownedDeviceStatus"));
        status->setProperty("online", device.online);
        status->setAlignment(Qt::AlignCenter);
        row->addWidget(status);
        auto* action = new QPushButton(
            device.current ? QStringLiteral("本机")
                           : QStringLiteral("进入桌面"), card);
        action->setObjectName(device.current
            ? QStringLiteral("softButton")
            : QStringLiteral("primaryButton"));
        action->setCursor(device.online && !device.current
                              ? Qt::PointingHandCursor
                              : Qt::ArrowCursor);
        action->setEnabled(device.online && !device.current);
        action->setIconSize(QSize(21, 21));
        ui::RemoteCTheme::SetIcon(
            action,
            device.current
                ? QStringLiteral(":/ui/icons/light/monitor.svg")
                : QStringLiteral(":/ui/icons/actions/monitor.svg"),
            device.current ? ui::ThemeIconTone::kNeutral
                           : ui::ThemeIconTone::kOnDark);
        if (!device.current) {
            remotec::ui::morph::MorphIconButtonBinding::attach(
                action,
                QStringLiteral(":/ui/icons/lucide/base/monitor.svg"),
                QStringLiteral(":/ui/icons/lucide/base/monitor-check.svg"),
                remotec::ui::morph::MorphIconButtonBinding::Interaction::Hover,
                QSize(21, 21), QColor(QStringLiteral("#FFFFFF")),
                QColor(QStringLiteral("#FFFFFF")));
        }
        if (!device.current) {
            const QString deviceId = QString::fromStdString(device.deviceId);
            const QString deviceName = device.deviceName.empty()
                ? QStringLiteral("远程设备")
                : QString::fromStdString(device.deviceName);
            connect(action, &QPushButton::clicked, this,
                    [this, deviceId, deviceName] {
                        StartOwnedDeviceSession(deviceId, deviceName);
                    });
        }
        row->addWidget(action);
        ownedDevicesCardsLayout_->addWidget(card);
    }
    RefreshRecentDevices();
}

void ControllerMainWindow::StartOwnedDeviceSession(
    const QString& deviceId,
    const QString& deviceName)
{
    pendingDeviceName_ = deviceName;
    lastDirectSessionToastError_.clear();
    ownedDeviceSessionPending_ = true;
    const auto beforeConnect = engine_->Snapshot();
    const auto result = engine_->ConnectOwnedDevice(
        deviceId.toStdString(), SessionPurpose::kRemoteControl);
    if (!result.accepted) {
        ownedDeviceSessionPending_ = false;
        RemoteCToast::Show(
            this,
            LocalizedDirectSessionError(
                result.errorCode, result.errorMessage, &beforeConnect),
            RemoteCToast::Tone::kError);
        return;
    }
}
