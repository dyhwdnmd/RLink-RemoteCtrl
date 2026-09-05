// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

void ControllerMainWindow::QueueRoomJoinApproval(
    const SessionEngineSnapshot& snapshot)
{
    const QString roomId = QString::fromStdString(snapshot.room.roomId);
    if (approvalRoomId_ != roomId) {
        approvalRoomId_ = roomId;
        promptedRoomJoinRequestIds_.clear();
        promptedRoomScreenShareSwitchRequestIds_.clear();
        promptedRoomControlRequestIds_.clear();
        promptedRoomScreenShareViewRequestIds_.clear();
        handledRoomMemberActionResultKeys_.clear();
        roomJoinApprovalPromptPending_ = false;
        roomScreenShareSwitchApprovalPromptPending_ = false;
        roomControlApprovalPromptPending_ = false;
        roomScreenShareViewApprovalPromptPending_ = false;
    }
    if (roomJoinApprovalPromptPending_ ||
        roomScreenShareSwitchApprovalPromptPending_ ||
        roomControlApprovalPromptPending_ ||
        roomScreenShareViewApprovalPromptPending_ ||
        snapshot.connectivity != SessionConnectivityState::kOnline ||
        snapshot.room.membership != RoomMembershipState::kActive ||
        snapshot.room.ownerDeviceId != snapshot.localDeviceId) {
        return;
    }

    const auto request = std::find_if(
        snapshot.incomingRoomJoinRequests.begin(),
        snapshot.incomingRoomJoinRequests.end(),
        [this](const RoomJoinRequest& candidate) {
            return !promptedRoomJoinRequestIds_.contains(
                QString::fromStdString(candidate.requestId));
        });
    if (request == snapshot.incomingRoomJoinRequests.end()) {
        return;
    }

    const RoomJoinRequest pendingRequest = *request;
    const QString requestId =
        QString::fromStdString(pendingRequest.requestId);
    promptedRoomJoinRequestIds_.insert(requestId);
    roomJoinApprovalPromptPending_ = true;
    QTimer::singleShot(0, this, [this, pendingRequest, requestId] {
        const auto current = engine_->Snapshot();
        const auto stillPending = std::find_if(
            current.incomingRoomJoinRequests.begin(),
            current.incomingRoomJoinRequests.end(),
            [&pendingRequest](const RoomJoinRequest& candidate) {
                return candidate.requestId == pendingRequest.requestId;
            });
        if (current.connectivity != SessionConnectivityState::kOnline ||
            current.room.membership != RoomMembershipState::kActive ||
            current.room.ownerDeviceId != current.localDeviceId ||
            stillPending == current.incomingRoomJoinRequests.end()) {
            roomJoinApprovalPromptPending_ = false;
            promptedRoomJoinRequestIds_.remove(requestId);
            QueueRoomJoinApproval(current);
            return;
        }

        const QString requesterName =
            pendingRequest.requesterDeviceName.empty()
                ? QString::fromStdString(pendingRequest.requesterDeviceId)
                : QString::fromStdString(pendingRequest.requesterDeviceName);
        const QString requesterId =
            QString::fromStdString(pendingRequest.requesterDeviceId);
        const QString prompt =
            QStringLiteral(
                "设备 %1（%2）申请加入房间 %3。\n\n当前人数：%4 / %5。是否允许加入？")
                .arg(requesterName)
                .arg(requesterId)
                .arg(QString::fromStdString(pendingRequest.roomId))
                .arg(static_cast<qulonglong>(current.room.members.size()))
                .arg(current.room.capacity);
        const bool accepted = RemoteCDialog::Confirm(
            this, QStringLiteral("收到入房申请"), prompt,
            QStringLiteral("允许加入"), QStringLiteral("拒绝"),
            RemoteCDialog::Tone::kQuestion, true);
        const auto result = engine_->RespondToRoomJoin(
            pendingRequest.requestId, accepted);
        roomJoinApprovalPromptPending_ = false;
        if (!result.accepted) {
            promptedRoomJoinRequestIds_.remove(requestId);
            SetRoomActionHint(
                QStringLiteral("处理入房申请失败：%1")
                    .arg(QString::fromStdString(result.errorMessage)),
                true);
        } else {
            SetRoomActionHint(
                accepted ? QStringLiteral("已同意 %1 加入房间。")
                               .arg(requesterName)
                         : QStringLiteral("已拒绝 %1 的入房申请。")
                               .arg(requesterName));
        }
        QueueRoomJoinApproval(engine_->Snapshot());
        QueueRoomScreenShareSwitchApproval(engine_->Snapshot());
        QueueRoomControlApproval(engine_->Snapshot());
        QueueRoomScreenShareViewApproval(engine_->Snapshot());
    });
}

void ControllerMainWindow::QueueRoomScreenShareSwitchApproval(
    const SessionEngineSnapshot& snapshot)
{
    const QString roomId = QString::fromStdString(snapshot.room.roomId);
    if (approvalRoomId_ != roomId) {
        approvalRoomId_ = roomId;
        promptedRoomJoinRequestIds_.clear();
        promptedRoomScreenShareSwitchRequestIds_.clear();
        promptedRoomControlRequestIds_.clear();
        promptedRoomScreenShareViewRequestIds_.clear();
        handledRoomMemberActionResultKeys_.clear();
        roomJoinApprovalPromptPending_ = false;
        roomScreenShareSwitchApprovalPromptPending_ = false;
        roomControlApprovalPromptPending_ = false;
        roomScreenShareViewApprovalPromptPending_ = false;
    }
    if (roomScreenShareSwitchApprovalPromptPending_ ||
        roomJoinApprovalPromptPending_ ||
        roomControlApprovalPromptPending_ ||
        roomScreenShareViewApprovalPromptPending_ ||
        snapshot.connectivity != SessionConnectivityState::kOnline ||
        snapshot.room.membership != RoomMembershipState::kActive ||
        snapshot.room.screenShareState !=
            RoomScreenShareState::kActive ||
        snapshot.room.screenSharerDeviceId != snapshot.localDeviceId) {
        return;
    }

    const auto request = std::find_if(
        snapshot.incomingRoomScreenShareSwitchRequests.begin(),
        snapshot.incomingRoomScreenShareSwitchRequests.end(),
        [this](const RoomScreenShareSwitchRequest& candidate) {
            return !promptedRoomScreenShareSwitchRequestIds_.contains(
                QString::fromStdString(candidate.requestId));
        });
    if (request ==
        snapshot.incomingRoomScreenShareSwitchRequests.end()) {
        return;
    }

    const RoomScreenShareSwitchRequest pendingRequest = *request;
    const QString requestId =
        QString::fromStdString(pendingRequest.requestId);
    promptedRoomScreenShareSwitchRequestIds_.insert(requestId);
    roomScreenShareSwitchApprovalPromptPending_ = true;
    QTimer::singleShot(0, this, [this, pendingRequest, requestId] {
        const auto current = engine_->Snapshot();
        const auto stillPending = std::find_if(
            current.incomingRoomScreenShareSwitchRequests.begin(),
            current.incomingRoomScreenShareSwitchRequests.end(),
            [&pendingRequest](
                const RoomScreenShareSwitchRequest& candidate) {
                return candidate.requestId ==
                       pendingRequest.requestId;
            });
        if (current.connectivity !=
                SessionConnectivityState::kOnline ||
            current.room.membership !=
                RoomMembershipState::kActive ||
            current.room.screenShareState !=
                RoomScreenShareState::kActive ||
            current.room.screenSharerDeviceId !=
                current.localDeviceId ||
            stillPending ==
                current.incomingRoomScreenShareSwitchRequests.end()) {
            roomScreenShareSwitchApprovalPromptPending_ = false;
            promptedRoomScreenShareSwitchRequestIds_.remove(requestId);
            QueueRoomScreenShareSwitchApproval(current);
            return;
        }

        const QString requesterName =
            pendingRequest.requesterDeviceName.empty()
                ? QString::fromStdString(
                      pendingRequest.requesterDeviceId)
                : QString::fromStdString(
                      pendingRequest.requesterDeviceName);
        const QString requesterId =
            QString::fromStdString(pendingRequest.requesterDeviceId);
        const QString prompt = QStringLiteral(
            "设备 %1（%2）申请接替你成为主机器。\n\n同意后，本机屏幕共享会停止，现有远程控制授权会立即撤销，并切换到对方画面；拒绝则保持当前状态。")
                                   .arg(requesterName, requesterId);
        const bool accepted = RemoteCDialog::Confirm(
            this, QStringLiteral("收到主机器接替申请"), prompt,
            QStringLiteral("同意接替"), QStringLiteral("保持当前共享"),
            RemoteCDialog::Tone::kWarning, true);
        const auto result =
            engine_->RespondToRoomScreenShareSwitch(
                pendingRequest.requestId, accepted);
        roomScreenShareSwitchApprovalPromptPending_ = false;
        if (!result.accepted) {
            promptedRoomScreenShareSwitchRequestIds_.remove(requestId);
            SetRoomActionHint(
                QStringLiteral("处理主机器接替申请失败：%1")
                    .arg(QString::fromStdString(result.errorMessage)),
                true);
        } else {
            SetRoomActionHint(
                accepted
                    ? QStringLiteral(
                          "已同意 %1 接替，正在安全切换主机器。")
                          .arg(requesterName)
                    : QStringLiteral(
                          "已拒绝 %1 的主机器接替申请。")
                          .arg(requesterName));
        }
        QueueRoomScreenShareSwitchApproval(engine_->Snapshot());
        QueueRoomControlApproval(engine_->Snapshot());
        QueueRoomJoinApproval(engine_->Snapshot());
        QueueRoomScreenShareViewApproval(engine_->Snapshot());
    });
}

void ControllerMainWindow::QueueRoomControlApproval(
    const SessionEngineSnapshot& snapshot)
{
    const QString roomId = QString::fromStdString(snapshot.room.roomId);
    if (approvalRoomId_ != roomId) {
        approvalRoomId_ = roomId;
        promptedRoomJoinRequestIds_.clear();
        promptedRoomScreenShareSwitchRequestIds_.clear();
        promptedRoomControlRequestIds_.clear();
        promptedRoomScreenShareViewRequestIds_.clear();
        handledRoomMemberActionResultKeys_.clear();
        roomJoinApprovalPromptPending_ = false;
        roomScreenShareSwitchApprovalPromptPending_ = false;
        roomControlApprovalPromptPending_ = false;
        roomScreenShareViewApprovalPromptPending_ = false;
    }
    if (roomControlApprovalPromptPending_ ||
        roomJoinApprovalPromptPending_ ||
        roomScreenShareSwitchApprovalPromptPending_ ||
        roomScreenShareViewApprovalPromptPending_ ||
        snapshot.connectivity != SessionConnectivityState::kOnline ||
        snapshot.room.membership != RoomMembershipState::kActive ||
        snapshot.room.screenSharerDeviceId != snapshot.localDeviceId) {
        return;
    }

    const auto request = std::find_if(
        snapshot.incomingRoomControlRequests.begin(),
        snapshot.incomingRoomControlRequests.end(),
        [this](const RoomControlRequest& candidate) {
            return !promptedRoomControlRequestIds_.contains(
                QString::fromStdString(candidate.requestId));
        });
    if (request == snapshot.incomingRoomControlRequests.end()) {
        return;
    }

    const RoomControlRequest pendingRequest = *request;
    const QString requestId =
        QString::fromStdString(pendingRequest.requestId);
    promptedRoomControlRequestIds_.insert(requestId);
    roomControlApprovalPromptPending_ = true;
    QTimer::singleShot(0, this, [this, pendingRequest, requestId] {
        const auto current = engine_->Snapshot();
        const auto stillPending = std::find_if(
            current.incomingRoomControlRequests.begin(),
            current.incomingRoomControlRequests.end(),
            [&pendingRequest](const RoomControlRequest& candidate) {
                return candidate.requestId == pendingRequest.requestId;
            });
        if (current.connectivity != SessionConnectivityState::kOnline ||
            current.room.membership != RoomMembershipState::kActive ||
            current.room.screenSharerDeviceId != current.localDeviceId ||
            stillPending == current.incomingRoomControlRequests.end()) {
            roomControlApprovalPromptPending_ = false;
            promptedRoomControlRequestIds_.remove(requestId);
            QueueRoomControlApproval(current);
            return;
        }

        const QString requesterName =
            pendingRequest.requesterDeviceName.empty()
                ? QString::fromStdString(pendingRequest.requesterDeviceId)
                : QString::fromStdString(pendingRequest.requesterDeviceName);
        const QString requesterId =
            QString::fromStdString(pendingRequest.requesterDeviceId);
        const QString prompt = QStringLiteral(
            "设备 %1（%2）申请控制你正在分享的屏幕。\n\n同意后，房间中其他成员仍可观看，但只有该设备能够发送输入。")
            .arg(requesterName, requesterId);
        bool allowClipboard = QSettings().value(
            QString::fromLatin1(kRemotePasteEnabledSetting), true).toBool();
        const bool accepted = RemoteCDialog::ConfirmWithOption(
            this, QStringLiteral("收到控制申请"), prompt,
            QStringLiteral("允许控制"), QStringLiteral("拒绝"),
            QStringLiteral("允许本次控制会话接收远程粘贴"),
            allowClipboard, &allowClipboard,
            RemoteCDialog::Tone::kQuestion, true);
        clipboardAllowedForCurrentControl_ = accepted && allowClipboard;
        const auto result = engine_->RespondToRoomControl(
            pendingRequest.requestId, accepted);
        roomControlApprovalPromptPending_ = false;
        if (!result.accepted) {
            promptedRoomControlRequestIds_.remove(requestId);
            SetRoomActionHint(
                QStringLiteral("处理控制申请失败：%1")
                    .arg(QString::fromStdString(result.errorMessage)),
                true);
        } else {
            SetRoomActionHint(
                accepted ? QStringLiteral("已允许 %1 控制本机屏幕。")
                               .arg(requesterName)
                         : QStringLiteral("已拒绝 %1 的控制申请。")
                               .arg(requesterName));
        }
        QueueRoomControlApproval(engine_->Snapshot());
        QueueRoomScreenShareSwitchApproval(engine_->Snapshot());
        QueueRoomJoinApproval(engine_->Snapshot());
        QueueRoomScreenShareViewApproval(engine_->Snapshot());
    });
}

void ControllerMainWindow::QueueRoomScreenShareViewApproval(
    const SessionEngineSnapshot& snapshot)
{
    const QString roomId = QString::fromStdString(snapshot.room.roomId);
    if (approvalRoomId_ != roomId) {
        approvalRoomId_ = roomId;
        promptedRoomJoinRequestIds_.clear();
        promptedRoomScreenShareSwitchRequestIds_.clear();
        promptedRoomControlRequestIds_.clear();
        promptedRoomScreenShareViewRequestIds_.clear();
        handledRoomMemberActionResultKeys_.clear();
        roomJoinApprovalPromptPending_ = false;
        roomScreenShareSwitchApprovalPromptPending_ = false;
        roomControlApprovalPromptPending_ = false;
        roomScreenShareViewApprovalPromptPending_ = false;
    }
    if (roomScreenShareViewApprovalPromptPending_ ||
        roomJoinApprovalPromptPending_ ||
        roomScreenShareSwitchApprovalPromptPending_ ||
        roomControlApprovalPromptPending_ ||
        snapshot.connectivity != SessionConnectivityState::kOnline ||
        snapshot.room.membership != RoomMembershipState::kActive) {
        return;
    }

    const auto request = std::find_if(
        snapshot.incomingRoomScreenShareViewRequests.begin(),
        snapshot.incomingRoomScreenShareViewRequests.end(),
        [this](const RoomScreenShareViewRequest& candidate) {
            const QString key = QStringLiteral("%1:%2")
                .arg(QString::fromStdString(
                         candidate.requesterDeviceId))
                .arg(static_cast<qulonglong>(candidate.sequence));
            return !promptedRoomScreenShareViewRequestIds_.contains(key);
        });
    if (request ==
        snapshot.incomingRoomScreenShareViewRequests.end()) {
        return;
    }

    const RoomScreenShareViewRequest pendingRequest = *request;
    const QString requestKey = QStringLiteral("%1:%2")
        .arg(QString::fromStdString(
                 pendingRequest.requesterDeviceId))
        .arg(static_cast<qulonglong>(pendingRequest.sequence));
    promptedRoomScreenShareViewRequestIds_.insert(requestKey);
    roomScreenShareViewApprovalPromptPending_ = true;
    QTimer::singleShot(
        0, this, [this, pendingRequest, requestKey] {
            const auto current = engine_->Snapshot();
            const auto stillPending = std::find_if(
                current.incomingRoomScreenShareViewRequests.begin(),
                current.incomingRoomScreenShareViewRequests.end(),
                [&pendingRequest](
                    const RoomScreenShareViewRequest& candidate) {
                    return candidate.sequence ==
                               pendingRequest.sequence &&
                           candidate.requesterDeviceId ==
                               pendingRequest.requesterDeviceId;
                });
            if (current.connectivity !=
                    SessionConnectivityState::kOnline ||
                current.room.membership !=
                    RoomMembershipState::kActive ||
                stillPending ==
                    current.incomingRoomScreenShareViewRequests.end()) {
                roomScreenShareViewApprovalPromptPending_ = false;
                promptedRoomScreenShareViewRequestIds_.remove(
                    requestKey);
                QueueRoomScreenShareViewApproval(current);
                return;
            }

            const QString requesterName =
                pendingRequest.requesterDeviceName.empty()
                    ? QString::fromStdString(
                          pendingRequest.requesterDeviceId)
                    : QString::fromStdString(
                          pendingRequest.requesterDeviceName);
            const QString prompt = QStringLiteral(
                "%1 请求观看你的屏幕。\n\n允许后，本机会立即开始屏幕共享；如果房间中已有主机器，将进入现有的主机器切换确认流程。")
                .arg(requesterName);
            const bool accepted = RemoteCDialog::Confirm(
                this, QStringLiteral("收到屏幕观看申请"), prompt,
                QStringLiteral("允许共享"), QStringLiteral("拒绝"),
                RemoteCDialog::Tone::kQuestion, true);
            const auto result =
                engine_->RespondToRoomMemberScreenShare(
                    pendingRequest.requesterDeviceId,
                    pendingRequest.sequence, accepted);
            roomScreenShareViewApprovalPromptPending_ = false;
            if (!result.accepted) {
                promptedRoomScreenShareViewRequestIds_.remove(
                    requestKey);
                SetRoomActionHint(
                    QStringLiteral("处理屏幕观看申请失败：%1")
                        .arg(QString::fromStdString(
                            result.errorMessage)),
                    true);
            } else {
                SetRoomActionHint(
                    accepted
                        ? QStringLiteral(
                              "已允许 %1 观看本机屏幕，正在建立共享。")
                              .arg(requesterName)
                        : QStringLiteral(
                              "已拒绝 %1 的屏幕观看申请。")
                              .arg(requesterName));
            }
            const auto latest = engine_->Snapshot();
            QueueRoomScreenShareViewApproval(latest);
            QueueRoomJoinApproval(latest);
            QueueRoomScreenShareSwitchApproval(latest);
            QueueRoomControlApproval(latest);
        });
}
