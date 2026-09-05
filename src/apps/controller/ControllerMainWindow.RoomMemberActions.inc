// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

void ControllerMainWindow::HandleRoomMemberActionResults(
    const SessionEngineSnapshot& snapshot)
{
    for (const auto& result : snapshot.roomMemberActionResults) {
        const QString key = QStringLiteral("%1:%2:%3:%4")
            .arg(QString::fromStdString(result.roomId),
                 QString::fromStdString(result.peerDeviceId))
            .arg(static_cast<qulonglong>(result.sequence))
            .arg(static_cast<int>(result.action));
        if (handledRoomMemberActionResultKeys_.contains(key)) {
            continue;
        }
        handledRoomMemberActionResultKeys_.insert(key);
        const QString peerName = MemberDisplayName(
            snapshot.room, result.peerDeviceId,
            snapshot.localDeviceId);
        if (result.accepted) {
            QString message;
            switch (result.action) {
            case RoomMemberAction::kRequestScreenShare:
                message = QStringLiteral(
                    "%1 已允许观看，正在建立屏幕共享。")
                    .arg(peerName);
                break;
            case RoomMemberAction::kStopScreenShare:
                message = QStringLiteral(
                    "%1 已停止屏幕共享。").arg(peerName);
                break;
            case RoomMemberAction::kDisableMicrophone:
                message = QStringLiteral(
                    "%1 的麦克风已关闭。").arg(peerName);
                break;
            }
            RemoteCToast::Show(
                this, message, RemoteCToast::Tone::kSuccess);
            continue;
        }

        QString title;
        QString message;
        switch (result.action) {
        case RoomMemberAction::kRequestScreenShare:
            title = QStringLiteral("屏幕观看申请被拒绝");
            message = QStringLiteral(
                "%1 拒绝了你的屏幕观看申请。").arg(peerName);
            break;
        case RoomMemberAction::kStopScreenShare:
            title = QStringLiteral("未能停止对方共享");
            message = QStringLiteral(
                "%1 未能停止屏幕共享，请检查当前控制权和网络状态。")
                .arg(peerName);
            break;
        case RoomMemberAction::kDisableMicrophone:
            title = QStringLiteral("未能关闭对方麦克风");
            message = QStringLiteral(
                "无法关闭 %1 的麦克风，请检查房主权限和 P2P 状态。")
                .arg(peerName);
            break;
        }
        QTimer::singleShot(0, this, [this, title, message] {
            RemoteCDialog::Alert(
                this, title, message, QStringLiteral("知道了"),
                RemoteCDialog::Tone::kDanger, true);
        });
    }
}

void ControllerMainWindow::ShowRoomMemberContextMenu(
    const QPoint& position)
{
    if (!engine_ || !roomMemberList_) {
        return;
    }
    QListWidgetItem* item = roomMemberList_->itemAt(position);
    if (!item) {
        return;
    }
    const QString peerDeviceId =
        item->data(Qt::UserRole).toString();
    const QString peerName =
        item->data(Qt::UserRole + 1).toString();
    const auto snapshot = engine_->Snapshot();
    if (peerDeviceId.isEmpty() ||
        peerDeviceId.toStdString() == snapshot.localDeviceId ||
        snapshot.room.membership != RoomMembershipState::kActive) {
        return;
    }
    const auto member = std::find_if(
        snapshot.room.members.begin(), snapshot.room.members.end(),
        [&peerDeviceId](const RoomMemberSnapshot& current) {
            return current.deviceId == peerDeviceId.toStdString();
        });
    if (member == snapshot.room.members.end() || !member->online) {
        return;
    }
    const auto pair = std::find_if(
        snapshot.roomPeerConnections.begin(),
        snapshot.roomPeerConnections.end(),
        [&peerDeviceId](const RoomPeerConnectionSnapshot& current) {
            return current.peerDeviceId ==
                       peerDeviceId.toStdString() &&
                   current.state ==
                       RoomPeerConnectionState::kActive;
        });
    const bool pairActive =
        pair != snapshot.roomPeerConnections.end();
    const bool peerSharing =
        snapshot.room.screenSharerDeviceId ==
        peerDeviceId.toStdString();
    const bool localOwner =
        snapshot.room.ownerDeviceId == snapshot.localDeviceId;
    const bool localController =
        snapshot.room.activeControllerDeviceId ==
        snapshot.localDeviceId;

    RoundedPopupMenu menu(this);
    menu.setObjectName(QStringLiteral("roomMemberActionMenu"));
    menu.setStyleSheet(QStringLiteral(R"(
QMenu#roomMemberActionMenu {
    background:#ffffff;
    border:1px solid #dce2eb;
    border-radius:12px;
    padding:7px;
    color:#263248;
}
QMenu#roomMemberActionMenu::item {
    min-width:220px;
    min-height:34px;
    padding:4px 14px;
    margin:1px 0;
    border-radius:8px;
}
QMenu#roomMemberActionMenu::item:selected {
    background:#e7f0ff;
    color:#0868e8;
}
QMenu#roomMemberActionMenu::item:disabled {
    color:#8995a8;
}
QMenu#roomMemberActionMenu::separator {
    height:1px;
    background:#e7ebf1;
    margin:6px 8px;
}
)"));
    if (darkInterfaceTheme_) {
        menu.setStyleSheet(menu.styleSheet() + QStringLiteral(R"(
QMenu#roomMemberActionMenu {
    background:#151F2E; border-color:#34445B; color:#E4EBF5;
}
QMenu#roomMemberActionMenu::item:selected {
    background:#263A61; color:#9BB4FF;
}
QMenu#roomMemberActionMenu::item:disabled { color:#69788E; }
QMenu#roomMemberActionMenu::separator { background:#2A394E; }
)"));
    }
    QAction* title = menu.addAction(
        peerName.isEmpty() ? peerDeviceId : peerName);
    title->setEnabled(false);
    menu.addSeparator();

    QAction* screenAction = menu.addAction(
        peerSharing ? QStringLiteral("打开共享画面")
                    : QStringLiteral("申请观看对方屏幕"));
    screenAction->setEnabled(pairActive);
    QAction* controlAction = nullptr;
    QAction* stopShareAction = nullptr;
    if (peerSharing && !localController &&
        snapshot.room.activeControllerDeviceId.empty()) {
        controlAction = menu.addAction(
            QStringLiteral("申请远程控制"));
        controlAction->setEnabled(pairActive);
    }
    if (peerSharing && (localOwner || localController)) {
        stopShareAction = menu.addAction(
            QStringLiteral("停止对方屏幕共享"));
        stopShareAction->setEnabled(pairActive);
    }
    QAction* muteAction = nullptr;
    if (localOwner && member->microphonePublishing) {
        muteAction = menu.addAction(
            QStringLiteral("关闭对方麦克风"));
        muteAction->setEnabled(pairActive);
    }
    menu.addSeparator();
    QAction* copyAction = menu.addAction(
        QStringLiteral("复制设备 ID"));

    QAction* selected = menu.exec(
        roomMemberList_->viewport()->mapToGlobal(position));
    if (!selected) {
        return;
    }
    SessionCommandResult result;
    QString successText;
    if (selected == screenAction) {
        if (peerSharing) {
            dismissedRemoteScreenSharerDeviceId_.clear();
            dismissedRemoteScreenShareEpoch_ = 0;
            OpenRemoteSession(RemoteSessionBinding::Room(
                peerDeviceId, peerName,
                QString::fromStdString(pair->pairId)));
            return;
        }
        result = engine_->RequestRoomMemberScreenShare(
            peerDeviceId.toStdString());
        successText = QStringLiteral("屏幕观看申请已发送");
    } else if (selected == controlAction) {
        result = engine_->RequestRoomControl();
        successText = QStringLiteral("远程控制申请已发送");
    } else if (selected == stopShareAction) {
        dismissedRemoteScreenSharerDeviceId_ = peerDeviceId;
        dismissedRemoteScreenShareEpoch_ =
            snapshot.room.screenShareEpoch;
        result = engine_->RequestRemoteRoomScreenShareStop(
            peerDeviceId.toStdString(),
            snapshot.room.screenShareEpoch);
        successText = QStringLiteral("已通知对方停止屏幕共享");
    } else if (selected == muteAction) {
        result = engine_->RequestRoomMemberMicrophoneMute(
            peerDeviceId.toStdString());
        successText = QStringLiteral("关闭对方麦克风的指令已发送");
    } else if (selected == copyAction) {
        QApplication::clipboard()->setText(peerDeviceId);
        RemoteCToast::ShowAbove(
            roomMemberList_, QStringLiteral("设备 ID 已复制"),
            RemoteCToast::Tone::kSuccess);
        return;
    } else {
        return;
    }
    if (!result.accepted) {
        RemoteCToast::ShowAbove(
            roomMemberList_,
            QString::fromStdString(result.errorMessage),
            RemoteCToast::Tone::kError);
        return;
    }
    RemoteCToast::ShowAbove(
        roomMemberList_, successText,
        RemoteCToast::Tone::kSuccess);
}
