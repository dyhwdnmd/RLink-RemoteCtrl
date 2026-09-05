// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

QString ControllerMainWindow::RecentSettingsKey(
    const QString& listName) const
{
    const QString accountNamespace = recentHistoryAccountKey_.isEmpty()
        ? QStringLiteral("unavailable")
        : recentHistoryAccountKey_;
    return QStringLiteral("ui/accounts/%1/%2")
        .arg(accountNamespace, listName);
}

void ControllerMainWindow::MigrateLegacyRecentHistory()
{
    if (recentHistoryAccountKey_.isEmpty()) {
        return;
    }
    QSettings settings;
    const auto migrate = [&settings, this](const QString& legacyKey,
                                            const QString& listName,
                                            int maximumRecords) {
        const QString scopedKey = RecentSettingsKey(listName);
        if (!settings.contains(scopedKey) && settings.contains(legacyKey)) {
            QVariantList records = settings.value(legacyKey).toList();
            while (records.size() > maximumRecords) {
                records.removeLast();
            }
            settings.setValue(scopedKey, records);
        }
        settings.remove(legacyKey);
    };
    migrate(QStringLiteral("ui/recentRooms"),
            QStringLiteral("recentRooms"), 3);
    migrate(QStringLiteral("ui/recentDevices"),
            QStringLiteral("recentDevices"), 5);
}

void ControllerMainWindow::RememberRecentRoom(
    const SessionEngineSnapshot& snapshot)
{
    if (recentHistoryAccountKey_.isEmpty()) {
        return;
    }
    if (snapshot.room.membership != RoomMembershipState::kActive ||
        snapshot.room.roomId.empty()) {
        if (snapshot.room.membership == RoomMembershipState::kNone ||
            snapshot.room.membership == RoomMembershipState::kFailed) {
            const bool hadRememberedRoom =
                !lastRememberedRoomId_.isEmpty();
            lastRememberedRoomId_.clear();
            if (hadRememberedRoom &&
                snapshot.connectivity == SessionConnectivityState::kOnline) {
                recentRoomAvailabilityRequested_ = true;
                QTimer::singleShot(
                    0, this,
                    [this] { RequestRecentRoomAvailability(); });
            }
        }
        return;
    }

    const QString roomId = QString::fromStdString(snapshot.room.roomId);
    if (lastRememberedRoomId_ == roomId) {
        return;
    }
    lastRememberedRoomId_ = roomId;

    QSettings settings;
    QVariantList records = settings.value(
        RecentSettingsKey(QStringLiteral("recentRooms"))).toList();
    for (int index = records.size() - 1; index >= 0; --index) {
        if (records[index].toMap().value(
                QStringLiteral("roomId")).toString() == roomId) {
            records.removeAt(index);
        }
    }
    QVariantMap record;
    record.insert(QStringLiteral("roomId"), roomId);
    record.insert(
        QStringLiteral("ownerId"),
        QString::fromStdString(snapshot.room.ownerDeviceId));
    record.insert(QStringLiteral("capacity"), snapshot.room.capacity);
    record.insert(
        QStringLiteral("availability"),
        static_cast<int>(RoomAvailabilityState::kAvailable));
    record.insert(
        QStringLiteral("lastUsed"),
        QDateTime::currentDateTime().toMSecsSinceEpoch());
    records.prepend(record);
    while (records.size() > 3) {
        records.removeLast();
    }
    settings.setValue(
        RecentSettingsKey(QStringLiteral("recentRooms")), records);
    RefreshRecentRooms();
}

void ControllerMainWindow::RefreshRecentRooms()
{
    if (!recentRoomsLayout_ || !recentEmptyState_) {
        return;
    }
    while (auto* item = recentRoomsLayout_->takeAt(0)) {
        if (auto* widget = item->widget();
            widget && widget != recentEmptyState_) {
            widget->deleteLater();
        }
        delete item;
    }

    QSettings settings;
    QVariantList records = settings.value(
        RecentSettingsKey(QStringLiteral("recentRooms"))).toList();
    bool removedLegacyRoom = false;
    for (int index = records.size() - 1; index >= 0; --index) {
        const QString roomId = records[index].toMap().value(
            QStringLiteral("roomId")).toString();
        if (!IsNineDigitPublicId(roomId)) {
            records.removeAt(index);
            removedLegacyRoom = true;
        }
    }
    while (records.size() > 3) {
        records.removeLast();
        removedLegacyRoom = true;
    }
    if (removedLegacyRoom) {
        settings.setValue(
            RecentSettingsKey(QStringLiteral("recentRooms")), records);
    }
    if (records.isEmpty()) {
        recentEmptyState_->show();
        recentRoomsLayout_->addWidget(recentEmptyState_);
        return;
    }
    recentEmptyState_->hide();

    for (const QVariant& value : records) {
        const QVariantMap record = value.toMap();
        const QString roomId = record.value(
            QStringLiteral("roomId")).toString();
        if (!IsNineDigitPublicId(roomId)) {
            continue;
        }
        const QString ownerId = record.value(
            QStringLiteral("ownerId")).toString();
        const auto capacity = record.value(
            QStringLiteral("capacity"), 2).toUInt();
        const QDateTime lastUsed = QDateTime::fromMSecsSinceEpoch(
            record.value(QStringLiteral("lastUsed")).toLongLong());
        const auto availability = static_cast<RoomAvailabilityState>(
            record.value(
                QStringLiteral("availability"),
                static_cast<int>(RoomAvailabilityState::kChecking)).toInt());

        auto* card = new QFrame(this);
        card->setProperty("card", true);
        auto* cardLayout = new QHBoxLayout(card);
        cardLayout->setContentsMargins(16, 10, 16, 10);
        cardLayout->setSpacing(13);
        auto* roomIcon = new QLabel(card);
        roomIcon->setObjectName(QStringLiteral("recentRoomIcon"));
        roomIcon->setAlignment(Qt::AlignCenter);
        roomIcon->setFixedSize(44, 44);
        ui::RemoteCTheme::SetPixmap(
            roomIcon, QStringLiteral(":/ui/icons/light/room.svg"),
            QSize(23, 23), ui::ThemeIconTone::kPrimary);
        cardLayout->addWidget(roomIcon);
        auto* textLayout = new QVBoxLayout();
        textLayout->setSpacing(4);
        auto* idLabel = new QLabel(roomId, card);
        idLabel->setObjectName(QStringLiteral("cardTitle"));
        idLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);
        textLayout->addWidget(idLabel);
        const QString ownerText = ownerId.isEmpty()
            ? QStringLiteral("房主未知")
            : QStringLiteral("房主 %1").arg(ownerId);
        QString availabilityText;
        QString availabilityTone;
        switch (availability) {
        case RoomAvailabilityState::kAvailable:
            availabilityText = QStringLiteral("房间有效");
            availabilityTone = QStringLiteral("available");
            break;
        case RoomAvailabilityState::kTemporarilyUnavailable:
            availabilityText = QStringLiteral("房间暂不可加入");
            availabilityTone = QStringLiteral("warning");
            break;
        case RoomAvailabilityState::kClosed:
            availabilityText = QStringLiteral("房间已关闭");
            availabilityTone = QStringLiteral("closed");
            break;
        case RoomAvailabilityState::kChecking:
            availabilityText = QStringLiteral("正在检查房间状态");
            availabilityTone = QStringLiteral("checking");
            break;
        }
        auto* detailLabel = new QLabel(
            QStringLiteral("%1 · 上限 %2 人 · %3")
                .arg(ownerText)
                .arg(capacity)
                .arg(lastUsed.toString(QStringLiteral("MM-dd HH:mm"))), card);
        detailLabel->setProperty("muted", true);
        textLayout->addWidget(detailLabel);
        auto* availabilityLabel = new QLabel(availabilityText, card);
        availabilityLabel->setProperty("availabilityTone", availabilityTone);
        cardLayout->addLayout(textLayout, 1);
        availabilityLabel->setObjectName(
            QStringLiteral("recentAvailabilityPill"));
        availabilityLabel->setAlignment(Qt::AlignCenter);
        cardLayout->addWidget(availabilityLabel);
        const bool canJoin =
            availability == RoomAvailabilityState::kAvailable;
        const QString buttonText = canJoin
            ? QStringLiteral("申请加入")
            : (availability == RoomAvailabilityState::kClosed
                   ? QStringLiteral("已关闭")
                   : (availability ==
                              RoomAvailabilityState::kTemporarilyUnavailable
                          ? QStringLiteral("暂不可用")
                          : QStringLiteral("检查中")));
        auto* useButton = new QPushButton(buttonText, card);
        useButton->setObjectName(QStringLiteral("softButton"));
        useButton->setCursor(Qt::PointingHandCursor);
        useButton->setEnabled(canJoin);
        useButton->setIconSize(QSize(16, 16));
        ui::RemoteCTheme::SetIcon(
            useButton,
            QStringLiteral(":/ui/icons/lucide/base/users-round.svg"),
            ui::ThemeIconTone::kPrimary);
        const bool darkTheme = ui::RemoteCTheme::IsDark(
            ui::RemoteCTheme::LoadPreference());
        remotec::ui::morph::MorphIconButtonBinding::attach(
            useButton,
            QStringLiteral(":/ui/icons/lucide/base/users-round.svg"),
            QStringLiteral(":/ui/icons/lucide/base/handshake.svg"),
            remotec::ui::morph::MorphIconButtonBinding::Interaction::Hover,
            QSize(16, 16),
            QColor(darkTheme ? QStringLiteral("#8EA5FF")
                             : QStringLiteral("#315EFB")),
            QColor(darkTheme ? QStringLiteral("#4FF0B5")
                             : QStringLiteral("#168A5B")));
        cardLayout->addWidget(useButton);
        connect(useButton, &QPushButton::clicked, this, [this, roomId] {
            SelectMainPage(0, roomNavButton_, QStringLiteral("协作房间"));
            const auto snapshot = engine_->Snapshot();
            if (snapshot.room.membership == RoomMembershipState::kActive) {
                RemoteCToast::Show(
                    this, QStringLiteral("当前已在一个活动房间中"),
                    RemoteCToast::Tone::kInformation);
                return;
            }
            roomIdEdit_->setText(roomId);
            const auto result = engine_->JoinRoom(roomId.toStdString());
            if (!result.accepted) {
                SetRoomActionHint(
                    QStringLiteral("申请加入失败：%1")
                        .arg(QString::fromStdString(result.errorMessage)),
                    true);
            }
        });
        recentRoomsLayout_->addWidget(card);
    }
}

void ControllerMainWindow::RememberRecentDevice(
    const SessionEngineSnapshot& snapshot)
{
    if (recentHistoryAccountKey_.isEmpty()) {
        return;
    }
    if (snapshot.state != SessionEngineState::kActive ||
        snapshot.remoteControlRole != RemoteControlRole::kController ||
        snapshot.sessionId.empty() || snapshot.peerDeviceId.empty() ||
        (snapshot.origin != SessionOrigin::kOwnedDevice &&
         snapshot.origin != SessionOrigin::kRemoteAssistance)) {
        if (snapshot.state == SessionEngineState::kReady) {
            lastRememberedDirectSessionId_.clear();
        }
        return;
    }

    const QString sessionId = QString::fromStdString(snapshot.sessionId);
    if (lastRememberedDirectSessionId_ == sessionId) {
        return;
    }
    lastRememberedDirectSessionId_ = sessionId;
    const QString deviceId = QString::fromStdString(snapshot.peerDeviceId);
    QString deviceName = pendingDeviceName_.trimmed();
    for (const auto& owned : snapshot.ownedDevices) {
        if (owned.deviceId == snapshot.peerDeviceId &&
            !owned.deviceName.empty()) {
            deviceName = QString::fromStdString(owned.deviceName);
            break;
        }
    }
    if (deviceName.isEmpty()) {
        deviceName = snapshot.origin == SessionOrigin::kOwnedDevice
            ? QStringLiteral("我的设备")
            : QStringLiteral("远程协助设备");
    }

    QSettings settings;
    QVariantList records = settings.value(
        RecentSettingsKey(QStringLiteral("recentDevices"))).toList();
    for (int index = records.size() - 1; index >= 0; --index) {
        if (records[index].toMap().value(
                QStringLiteral("deviceId")).toString() == deviceId) {
            records.removeAt(index);
        }
    }
    QVariantMap record;
    record.insert(QStringLiteral("deviceId"), deviceId);
    record.insert(QStringLiteral("deviceName"), deviceName);
    record.insert(QStringLiteral("origin"),
                  static_cast<int>(snapshot.origin));
    record.insert(QStringLiteral("lastUsed"),
                  QDateTime::currentMSecsSinceEpoch());
    records.prepend(record);
    while (records.size() > 5) {
        records.removeLast();
    }
    settings.setValue(
        RecentSettingsKey(QStringLiteral("recentDevices")), records);
    RefreshRecentDevices();
}

void ControllerMainWindow::RefreshRecentDevices()
{
    if (!recentDevicesLayout_ || !recentDevicesEmptyState_ || !engine_) {
        return;
    }
    while (auto* item = recentDevicesLayout_->takeAt(0)) {
        if (auto* widget = item->widget();
            widget && widget != recentDevicesEmptyState_) {
            widget->deleteLater();
        }
        delete item;
    }

    QSettings settings;
    QVariantList records = settings.value(
        RecentSettingsKey(QStringLiteral("recentDevices"))).toList();
    bool recordsChanged = false;
    for (int index = records.size() - 1; index >= 0; --index) {
        if (!IsNineDigitPublicId(records[index].toMap().value(
                QStringLiteral("deviceId")).toString())) {
            records.removeAt(index);
            recordsChanged = true;
        }
    }
    while (records.size() > 5) {
        records.removeLast();
        recordsChanged = true;
    }
    if (recordsChanged) {
        settings.setValue(
            RecentSettingsKey(QStringLiteral("recentDevices")), records);
    }
    if (records.isEmpty()) {
        recentDevicesEmptyState_->show();
        recentDevicesLayout_->addWidget(recentDevicesEmptyState_);
        return;
    }
    recentDevicesEmptyState_->hide();

    const auto snapshot = engine_->Snapshot();
    for (const QVariant& value : records) {
        const QVariantMap record = value.toMap();
        const QString deviceId = record.value(
            QStringLiteral("deviceId")).toString();
        QString deviceName = record.value(
            QStringLiteral("deviceName")).toString().trimmed();
        if (deviceName.isEmpty()) {
            deviceName = QStringLiteral("远程设备");
        }
        const auto origin = static_cast<SessionOrigin>(record.value(
            QStringLiteral("origin"),
            static_cast<int>(SessionOrigin::kRemoteAssistance)).toInt());
        const bool ownedDevice = origin == SessionOrigin::kOwnedDevice;
        const auto owned = std::find_if(
            snapshot.ownedDevices.begin(), snapshot.ownedDevices.end(),
            [&deviceId](const OwnedDeviceSnapshot& candidate) {
                return candidate.deviceId == deviceId.toStdString();
            });
        const bool ownedDeviceOnline =
            ownedDevice && owned != snapshot.ownedDevices.end() &&
            owned->online && !owned->current;
        const QDateTime lastUsed = QDateTime::fromMSecsSinceEpoch(
            record.value(QStringLiteral("lastUsed")).toLongLong());

        auto* card = new QFrame(this);
        card->setProperty("card", true);
        auto* row = new QHBoxLayout(card);
        row->setContentsMargins(16, 10, 16, 10);
        row->setSpacing(13);
        auto* icon = new QLabel(card);
        icon->setObjectName(QStringLiteral("recentDeviceIcon"));
        icon->setAlignment(Qt::AlignCenter);
        icon->setFixedSize(44, 44);
        ui::RemoteCTheme::SetPixmap(
            icon, QStringLiteral(":/ui/icons/light/monitor.svg"),
            QSize(23, 23), ownedDeviceOnline
                ? ui::ThemeIconTone::kPrimary
                : ui::ThemeIconTone::kNeutral);
        row->addWidget(icon);
        auto* text = new QVBoxLayout();
        text->setSpacing(4);
        auto* name = new QLabel(deviceName, card);
        name->setObjectName(QStringLiteral("cardTitle"));
        text->addWidget(name);
        auto* details = new QLabel(
            QStringLiteral("设备 ID  %1 · %2 · %3")
                .arg(deviceId,
                     ownedDevice ? QStringLiteral("我的设备")
                                 : QStringLiteral("验证码协助"),
                     lastUsed.toString(QStringLiteral("MM-dd HH:mm"))),
            card);
        details->setProperty("muted", true);
        text->addWidget(details);
        row->addLayout(text, 1);

        auto* action = new QPushButton(
            ownedDevice
                ? (ownedDeviceOnline ? QStringLiteral("进入桌面")
                                     : QStringLiteral("设备离线"))
                : QStringLiteral("再次连接  →"),
            card);
        action->setObjectName(ownedDeviceOnline || !ownedDevice
                                  ? QStringLiteral("primaryButton")
                                  : QStringLiteral("softButton"));
        const bool engineReady =
            snapshot.connectivity == SessionConnectivityState::kOnline &&
            snapshot.state == SessionEngineState::kReady;
        action->setEnabled(engineReady &&
                           (!ownedDevice || ownedDeviceOnline));
        action->setCursor(action->isEnabled()
                              ? Qt::PointingHandCursor
                              : Qt::ArrowCursor);
        action->setIconSize(QSize(21, 21));
        ui::RemoteCTheme::SetIcon(
            action, QStringLiteral(":/ui/icons/lucide/base/monitor.svg"),
            (ownedDeviceOnline || !ownedDevice)
                ? ui::ThemeIconTone::kOnDark
                : ui::ThemeIconTone::kNeutral);
        remotec::ui::morph::MorphIconButtonBinding::attach(
            action,
            QStringLiteral(":/ui/icons/lucide/base/monitor.svg"),
            QStringLiteral(":/ui/icons/lucide/base/monitor-check.svg"),
            remotec::ui::morph::MorphIconButtonBinding::Interaction::Hover,
            QSize(21, 21), QColor(QStringLiteral("#FFFFFF")),
            QColor(QStringLiteral("#FFFFFF")));
        if (ownedDevice) {
            connect(action, &QPushButton::clicked, this,
                    [this, deviceId, deviceName] {
                        StartOwnedDeviceSession(deviceId, deviceName);
                    });
        } else {
            connect(action, &QPushButton::clicked, this,
                    [this, deviceId] {
                        SelectMainPage(
                            1, deviceNavButton_, QStringLiteral("远程协助"));
                        deviceIdEdit_->setText(deviceId);
                        verificationCodeEdit_->clear();
                        verificationCodeEdit_->setFocus(
                            Qt::OtherFocusReason);
                        RemoteCToast::Show(
                            this,
                            QStringLiteral("请输入对方当前显示的 6 位验证码"),
                            RemoteCToast::Tone::kInformation);
                    });
        }
        row->addWidget(action);
        recentDevicesLayout_->addWidget(card);
    }
}

void ControllerMainWindow::RequestRecentRoomAvailability()
{
    if (recentHistoryAccountKey_.isEmpty()) {
        recentRoomAvailabilityRequested_ = true;
        return;
    }
    const QSettings settings;
    const QVariantList records = settings.value(
        RecentSettingsKey(QStringLiteral("recentRooms"))).toList();
    std::vector<std::string> roomIds;
    roomIds.reserve(static_cast<std::size_t>(records.size()));
    for (const QVariant& value : records) {
        const QString roomId = value.toMap().value(
            QStringLiteral("roomId")).toString();
        if (IsNineDigitPublicId(roomId)) {
            roomIds.push_back(roomId.toStdString());
        }
    }
    if (roomIds.empty()) {
        recentRoomAvailabilityRequested_ = true;
        return;
    }
    const auto result = engine_->QueryRoomAvailability(roomIds);
    recentRoomAvailabilityRequested_ = result.accepted;
}

void ControllerMainWindow::SelectMainPage(
    int pageIndex,
    QPushButton* navigationButton,
    const QString& title)
{
    if (!pageStack_ || pageIndex < 0 ||
        pageIndex >= pageStack_->count()) {
        return;
    }

    AnimateNavigationIndicator(navigationButton);
    for (auto* button : pageNavigationButtons_) {
        if (!button) {
            continue;
        }
        button->setProperty("navActive", button == navigationButton);
        button->setChecked(button == navigationButton);
        button->style()->unpolish(button);
        button->style()->polish(button);
        button->update();
    }
    if (titleBar_) {
        titleBar_->SetTitle(title);
    }
    setWindowTitle(QStringLiteral("RLink - %1").arg(title));

    if (pageStack_->currentIndex() == pageIndex) {
        return;
    }
    pageStack_->setCurrentIndex(pageIndex);
}

void ControllerMainWindow::AnimateNavigationIndicator(
    QPushButton* navigationButton)
{
    if (!navigationIndicator_ || !navigationButton) {
        return;
    }
    const QPoint topLeft = navigationButton->mapTo(
        navigationIndicator_->parentWidget(), QPoint(0, 0));
    const QRect destination(
        5, topLeft.y() + 9, 3,
        std::max(18, navigationButton->height() - 18));
    if (CurrentUiAnimationLevel() <= 0 ||
        !navigationIndicator_->isVisible() ||
        navigationIndicator_->geometry().isEmpty()) {
        navigationIndicator_->setGeometry(destination);
        return;
    }
    navigationAnimation_->stop();
    navigationAnimation_->setDuration(
        CurrentUiAnimationLevel() == 1 ? 95 : 155);
    navigationAnimation_->setStartValue(navigationIndicator_->geometry());
    navigationAnimation_->setEndValue(destination);
    navigationAnimation_->start();
}

void ControllerMainWindow::QueueRoomWorkspaceActive(bool active)
{
    QWidget* desired = active
        ? static_cast<QWidget*>(roomPanel_)
        : static_cast<QWidget*>(roomEntryPanel_);
    if (!roomWorkspaceStack_ || !desired) {
        return;
    }
    if ((roomWorkspaceAnimation_ || roomWorkspaceTransitionPending_) &&
        roomWorkspaceTargetActive_ == active) {
        return;
    }
    if (!roomWorkspaceAnimation_ && !roomWorkspaceTransitionPending_ &&
        roomWorkspaceStack_->currentWidget() == desired) {
        return;
    }

    roomWorkspaceTargetActive_ = active;
    roomWorkspaceTransitionPending_ = true;
    const quint64 request = ++roomWorkspaceTransitionRequest_;
    // Let all labels, member rows and media states from the current snapshot
    // settle through one UI frame before the transition captures the page.
    QTimer::singleShot(16, this, [this, active, request] {
        if (request != roomWorkspaceTransitionRequest_) {
            return;
        }
        roomWorkspaceTransitionPending_ = false;
        SetRoomWorkspaceActive(active);
    });
}

void ControllerMainWindow::SetRoomWorkspaceActive(bool active)
{
    if (!roomWorkspaceStack_ || !roomEntryPanel_ || !roomPanel_) {
        return;
    }

    QWidget* incoming = active
        ? static_cast<QWidget*>(roomPanel_)
        : static_cast<QWidget*>(roomEntryPanel_);

    if (roomWorkspaceAnimation_) {
        // Session snapshots can arrive several times while the room remains
        // active. They must not cancel an already-running transition toward
        // the same page.
        if (roomWorkspaceTargetActive_ == active) {
            return;
        }
        roomWorkspaceAnimation_->stop();
        roomWorkspaceAnimation_->deleteLater();
        roomWorkspaceAnimation_ = nullptr;
        if (roomWorkspaceTransitionLayer_) {
            roomWorkspaceTransitionLayer_->deleteLater();
            roomWorkspaceTransitionLayer_ = nullptr;
        }
        roomWorkspaceStack_->setCurrentWidget(
            active ? static_cast<QWidget*>(roomPanel_)
                   : static_cast<QWidget*>(roomEntryPanel_));
        if (auto* stack =
                dynamic_cast<CurrentPageStack*>(roomWorkspaceStack_)) {
            stack->RefreshCurrentHeight();
        }
    }
    roomWorkspaceTargetActive_ = active;

    QWidget* outgoing = roomWorkspaceStack_->currentWidget();
    if (outgoing == incoming) {
        return;
    }

    const QRect viewport = roomWorkspaceStack_->contentsRect();
    const int level = CurrentUiAnimationLevel();
    if (level <= 0 || !isVisible() || viewport.width() <= 0) {
        roomWorkspaceStack_->setCurrentWidget(incoming);
        if (auto* stack =
                dynamic_cast<CurrentPageStack*>(roomWorkspaceStack_)) {
            stack->RefreshCurrentHeight();
        }
        return;
    }

    // QStackedWidget owns its page geometry, so animating the real pages is
    // immediately overwritten by its layout. Render both pages into a short-
    // lived overlay and animate those independent layers instead.
    const int direction = active ? 1 : -1;
    auto* currentStack =
        dynamic_cast<CurrentPageStack*>(roomWorkspaceStack_);
    const int outgoingHeight = viewport.height();
    const int incomingHeight = currentStack
        ? currentStack->PageHeightForWidth(incoming)
        : outgoingHeight;
    const int transitionHeight = std::max(outgoingHeight, incomingHeight);
    const int transitionWidth = viewport.width();
    const QSize outgoingSize(transitionWidth, outgoingHeight);
    const QSize incomingSize(transitionWidth, incomingHeight);
    const QColor transitionColor(
        darkInterfaceTheme_ ? QStringLiteral("#0E1522")
                            : QStringLiteral("#F5F5F7"));
    QPixmap outgoingPixmap(outgoingSize);
    outgoingPixmap.fill(transitionColor);
    outgoing->render(&outgoingPixmap, QPoint(),
                     QRegion(QRect(QPoint(), outgoingSize)));

    incoming->ensurePolished();
    incoming->resize(incomingSize);
    QPixmap incomingPixmap(incomingSize);
    incomingPixmap.fill(transitionColor);
    incoming->render(&incomingPixmap, QPoint(),
                     QRegion(QRect(QPoint(), incomingSize)));

    auto* transitionLayer = new QWidget(roomWorkspaceStack_);
    roomWorkspaceTransitionLayer_ = transitionLayer;
    transitionLayer->setAttribute(Qt::WA_StyledBackground, true);
    transitionLayer->setStyleSheet(QStringLiteral("background:%1;")
        .arg(transitionColor.name()));
    transitionLayer->setGeometry(
        QRect(viewport.topLeft(), QSize(transitionWidth, transitionHeight)));

    auto* outgoingLayer = new QLabel(transitionLayer);
    outgoingLayer->setPixmap(outgoingPixmap);
    outgoingLayer->setGeometry(QRect(QPoint(), outgoingSize));

    auto* incomingLayer = new QLabel(transitionLayer);
    incomingLayer->setPixmap(incomingPixmap);
    incomingLayer->setGeometry(
        QRect(QPoint(direction * transitionWidth, 0), incomingSize));
    transitionLayer->show();
    transitionLayer->raise();

    auto* group = new QParallelAnimationGroup(this);
    roomWorkspaceAnimation_ = group;
    const int duration = level == 1 ? 180 : 270;

    auto* outgoingAnimation =
        new QPropertyAnimation(outgoingLayer, "pos", group);
    outgoingAnimation->setDuration(duration);
    outgoingAnimation->setStartValue(QPoint(0, 0));
    outgoingAnimation->setEndValue(
        QPoint(-direction * transitionWidth, 0));
    outgoingAnimation->setEasingCurve(QEasingCurve::InOutCubic);

    auto* incomingAnimation =
        new QPropertyAnimation(incomingLayer, "pos", group);
    incomingAnimation->setDuration(duration);
    incomingAnimation->setStartValue(
        QPoint(direction * transitionWidth, 0));
    incomingAnimation->setEndValue(QPoint(0, 0));
    incomingAnimation->setEasingCurve(QEasingCurve::OutCubic);

    auto* minimumHeightAnimation = new QPropertyAnimation(
        roomWorkspaceStack_, "minimumHeight", group);
    minimumHeightAnimation->setDuration(duration);
    minimumHeightAnimation->setStartValue(outgoingHeight);
    minimumHeightAnimation->setEndValue(incomingHeight);
    minimumHeightAnimation->setEasingCurve(QEasingCurve::InOutCubic);

    auto* maximumHeightAnimation = new QPropertyAnimation(
        roomWorkspaceStack_, "maximumHeight", group);
    maximumHeightAnimation->setDuration(duration);
    maximumHeightAnimation->setStartValue(outgoingHeight);
    maximumHeightAnimation->setEndValue(incomingHeight);
    maximumHeightAnimation->setEasingCurve(QEasingCurve::InOutCubic);

    connect(group, &QParallelAnimationGroup::finished, this,
            [this, group, incoming, transitionLayer] {
                if (roomWorkspaceAnimation_ != group) {
                    return;
                }
                roomWorkspaceStack_->setCurrentWidget(incoming);
                if (auto* stack = dynamic_cast<CurrentPageStack*>(
                        roomWorkspaceStack_)) {
                    stack->RefreshCurrentHeight();
                }
                transitionLayer->deleteLater();
                roomWorkspaceTransitionLayer_ = nullptr;
                roomWorkspaceAnimation_ = nullptr;
                group->deleteLater();
            });
    group->start();
}

void ControllerMainWindow::SetAnimationLevel(int level)
{
    level = std::clamp(level, 0, 2);
    QSettings settings;
    settings.setValue(QStringLiteral("ui/animationLevel"), level);
    // Keep the old boolean in sync for older public-test builds that may read
    // the same INI file during a rollback.
    settings.setValue(QStringLiteral("ui/animationsEnabled"), level > 0);
    if (level == 0) {
        SetBusyStatusAnimation(connectivityPill_, false);
        SetBusyStatusAnimation(serviceStatus_, false);
    }
    const QString label = level == 0
        ? QStringLiteral("关闭")
        : (level == 1 ? QStringLiteral("简洁")
                      : QStringLiteral("完整"));
    RemoteCToast::Show(
        this, QStringLiteral("界面动画已设为“%1”").arg(label),
        RemoteCToast::Tone::kSuccess);
}
