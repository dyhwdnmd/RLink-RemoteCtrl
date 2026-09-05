// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

    const QString previousConnectivityText = connectivityPill_->text();
    const QString previousServiceText = serviceStatus_->text();
    const auto setConnectivityPill = [this](const QString& text,
                                            const QString& foreground,
                                            const QString& background,
                                            const QString& border) {
        connectivityPill_->setText(text);
        connectivityPill_->setStyleSheet(
            QStringLiteral(
                "background:%1;color:%2;border:1px solid %3;"
                "border-radius:14px;padding:5px 12px;font-weight:700;")
                .arg(background, foreground, border));
    };
    switch (snapshot.connectivity) {
    case SessionConnectivityState::kNotConfigured:
        setConnectivityPill(QStringLiteral("●  信令未配置"),
                            QStringLiteral("#a66b12"),
                            QStringLiteral("#fff6df"),
                            QStringLiteral("#f1d99e"));
        serviceStatus_->setText(QStringLiteral("● 本地引擎就绪"));
        SetRoomActionHint(QStringLiteral(
            "尚未配置 WSS。请检查 RLink 启动配置或重新登录。"));
        break;
    case SessionConnectivityState::kConnecting:
        setConnectivityPill(QStringLiteral("●  正在连接信令"),
                            QStringLiteral("#a66b12"),
                            QStringLiteral("#fff6df"),
                            QStringLiteral("#f1d99e"));
        serviceStatus_->setText(QStringLiteral("● 正在注册设备"));
        SetRoomActionHint(QStringLiteral("正在通过 WSS 认证并注册本机设备。"));
        break;
    case SessionConnectivityState::kOnline:
        setConnectivityPill(QStringLiteral("●  信令在线"),
                            QStringLiteral("#0d8b50"),
                            QStringLiteral("#e9f8ef"),
                            QStringLiteral("#bde8cd"));
        serviceStatus_->setText(QStringLiteral("● 可创建或加入房间"));
        SetRoomActionHint(QStringLiteral(
            "设备已注册，房间控制面可用。默认 2 人房间天然兼容两人远控。"));
        break;
    case SessionConnectivityState::kOffline:
        setConnectivityPill(QStringLiteral("●  信令离线"),
                            QStringLiteral("#a66b12"),
                            QStringLiteral("#fff6df"),
                            QStringLiteral("#f1d99e"));
        serviceStatus_->setText(QStringLiteral("● 等待信令连接"));
        SetRoomActionHint(QStringLiteral(
            "WSS 当前离线；如果已经在房间中，席位会在恢复窗口内保留。"));
        break;
    case SessionConnectivityState::kFailed:
        setConnectivityPill(QStringLiteral("●  信令连接失败"),
                            QStringLiteral("#b4232f"),
                            QStringLiteral("#fff0f1"),
                            QStringLiteral("#f2bec3"));
        serviceStatus_->setText(QStringLiteral("● 本地引擎可用"));
        SetRoomActionHint(
            snapshot.errorMessage.empty()
                ? QStringLiteral("WSS 信令连接失败，请检查地址、令牌和证书。")
                : QString::fromStdString(snapshot.errorMessage),
            true);
        break;
    }
    const bool connectivityBusy =
        snapshot.connectivity == SessionConnectivityState::kConnecting ||
        snapshot.connectivity == SessionConnectivityState::kOffline;
    if (devicePageSignalStatus_) {
        devicePageSignalStatus_->setText(connectivityPill_->text());
        devicePageSignalStatus_->setStyleSheet(
            connectivityPill_->styleSheet());
    }

    if (!snapshot.roomAvailabilities.empty()) {
        QSettings settings;
        QVariantList records = settings.value(
            RecentSettingsKey(QStringLiteral("recentRooms"))).toList();
        bool changed = false;
        for (const auto& availability : snapshot.roomAvailabilities) {
            const QString roomId = QString::fromStdString(
                availability.roomId);
            for (QVariant& value : records) {
                QVariantMap record = value.toMap();
                if (record.value(QStringLiteral("roomId")).toString() !=
                    roomId) {
                    continue;
                }
                const int state = static_cast<int>(availability.state);
                if (record.value(QStringLiteral("availability"), -1)
                        .toInt() != state) {
                    record.insert(QStringLiteral("availability"), state);
                    value = record;
                    changed = true;
                }
                break;
            }
        }
        if (changed) {
            settings.setValue(
                RecentSettingsKey(QStringLiteral("recentRooms")), records);
            RefreshRecentRooms();
        }
    }
    if (snapshot.room.membership == RoomMembershipState::kFailed &&
        (snapshot.room.errorCode == "room_unavailable" ||
         snapshot.room.errorCode == "room_not_found") &&
        !snapshot.room.roomId.empty()) {
        QSettings settings;
        QVariantList records = settings.value(
            RecentSettingsKey(QStringLiteral("recentRooms"))).toList();
        const QString failedRoomId = QString::fromStdString(
            snapshot.room.roomId);
        bool changed = false;
        for (QVariant& value : records) {
            QVariantMap record = value.toMap();
            if (record.value(QStringLiteral("roomId")).toString() ==
                failedRoomId) {
                record.insert(
                    QStringLiteral("availability"),
                    static_cast<int>(RoomAvailabilityState::kClosed));
                value = record;
                changed = true;
                break;
            }
        }
        if (changed) {
            settings.setValue(
                RecentSettingsKey(QStringLiteral("recentRooms")), records);
            RefreshRecentRooms();
        }
    }
    if (snapshot.connectivity == SessionConnectivityState::kOnline) {
        if (!recentRoomAvailabilityRequested_) {
            recentRoomAvailabilityRequested_ = true;
            QTimer::singleShot(
                0, this,
                [this] { RequestRecentRoomAvailability(); });
        }
    } else {
        recentRoomAvailabilityRequested_ = false;
    }
    RefreshOwnedDevicesUi(snapshot);
