// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

    bool IsOnlineRoomMember(const RoomState& room,
                            const QString& deviceId) const
    {
        const auto member = room.members.constFind(deviceId);
        return member != room.members.cend() &&
               member->disconnectedAtMs == 0 &&
               devices_.value(deviceId, nullptr);
    }

    void SendRoomScreenShareGranted(QWebSocket* socket,
                                    const RoomState& room)
    {
        if (!socket || room.screenShareGrantId.isEmpty()) {
            return;
        }
        QJsonObject payload;
        payload.insert(QStringLiteral("roomId"), room.roomId);
        payload.insert(QStringLiteral("grantId"),
                       room.screenShareGrantId);
        payload.insert(QStringLiteral("epoch"),
                       static_cast<qint64>(room.screenShareEpoch));
        SendEnvelope(socket, QStringLiteral("room_screen_share_granted"),
                     {}, payload);
    }

    void SendRoomScreenShareSwitchPending(QWebSocket* socket,
                                          const RoomState& room)
    {
        if (!socket || room.screenShareSwitchRequestId.isEmpty() ||
            room.screenSharerDeviceId.isEmpty()) {
            return;
        }
        QJsonObject payload;
        payload.insert(QStringLiteral("roomId"), room.roomId);
        payload.insert(QStringLiteral("requestId"),
                       room.screenShareSwitchRequestId);
        payload.insert(QStringLiteral("screenSharerDeviceId"),
                       room.screenSharerDeviceId);
        SendEnvelope(
            socket, QStringLiteral("room_screen_share_switch_pending"), {},
            payload);
    }

    void SendRoomScreenShareSwitchRequested(QWebSocket* socket,
                                            const RoomState& room)
    {
        if (!socket || room.screenShareSwitchRequestId.isEmpty() ||
            room.screenShareSwitchRequesterDeviceId.isEmpty()) {
            return;
        }
        const auto member = room.members.constFind(
            room.screenShareSwitchRequesterDeviceId);
        QJsonObject payload;
        payload.insert(QStringLiteral("roomId"), room.roomId);
        payload.insert(QStringLiteral("requestId"),
                       room.screenShareSwitchRequestId);
        payload.insert(QStringLiteral("requesterDeviceId"),
                       room.screenShareSwitchRequesterDeviceId);
        payload.insert(
            QStringLiteral("requesterDeviceName"),
            member == room.members.cend() ? QString{} : member->deviceName);
        SendEnvelope(
            socket, QStringLiteral("room_screen_share_switch_requested"),
            {}, payload);
    }

    void SendRoomScreenShareSwitchResult(
        QWebSocket* socket,
        const RoomState& room,
        const QString& requestId,
        bool accepted,
        const QString& reasonCode,
        const QString& reasonMessage)
    {
        if (!socket) {
            return;
        }
        QJsonObject payload;
        payload.insert(QStringLiteral("roomId"), room.roomId);
        payload.insert(QStringLiteral("requestId"), requestId);
        payload.insert(QStringLiteral("accepted"), accepted);
        payload.insert(QStringLiteral("reasonCode"), reasonCode.left(64));
        payload.insert(QStringLiteral("reasonMessage"), reasonMessage);
        SendEnvelope(
            socket, QStringLiteral("room_screen_share_switch_result"), {},
            payload);
    }

    void CancelPendingRoomScreenShareSwitch(
        RoomState& room,
        const QString& reasonCode,
        const QString& reasonMessage)
    {
        if (room.screenShareSwitchRequestId.isEmpty()) {
            room.screenShareSwitchRequesterDeviceId.clear();
            room.screenShareSwitchRequestExpiresAtMs = 0;
            return;
        }
        const QString requestId = room.screenShareSwitchRequestId;
        QWebSocket* requester = devices_.value(
            room.screenShareSwitchRequesterDeviceId, nullptr);
        QWebSocket* sharer =
            devices_.value(room.screenSharerDeviceId, nullptr);
        SendRoomScreenShareSwitchResult(
            requester, room, requestId, false, reasonCode, reasonMessage);
        if (sharer && sharer != requester) {
            SendRoomScreenShareSwitchResult(
                sharer, room, requestId, false, reasonCode, reasonMessage);
        }
        room.screenShareSwitchRequestId.clear();
        room.screenShareSwitchRequesterDeviceId.clear();
        room.screenShareSwitchRequestExpiresAtMs = 0;
    }

    void SendRoomControlResult(QWebSocket* socket,
                               const RoomState& room,
                               const QString& requestId,
                               bool accepted,
                               const QString& reasonCode,
                               const QString& reasonMessage)
    {
        if (!socket) {
            return;
        }
        QJsonObject payload;
        payload.insert(QStringLiteral("roomId"), room.roomId);
        payload.insert(QStringLiteral("requestId"), requestId);
        payload.insert(QStringLiteral("accepted"), accepted);
        payload.insert(QStringLiteral("reasonCode"), reasonCode.left(64));
        payload.insert(QStringLiteral("reasonMessage"), reasonMessage);
        SendEnvelope(socket, QStringLiteral("room_control_result"), {},
                     payload);
    }

    void SendRoomControlGranted(QWebSocket* socket,
                                const RoomState& room)
    {
        if (!socket || room.controlGrantId.isEmpty() ||
            room.screenSharerDeviceId.isEmpty() ||
            room.activeControllerDeviceId.isEmpty()) {
            return;
        }
        QJsonObject payload;
        payload.insert(QStringLiteral("roomId"), room.roomId);
        payload.insert(QStringLiteral("grantId"), room.controlGrantId);
        payload.insert(QStringLiteral("screenSharerDeviceId"),
                       room.screenSharerDeviceId);
        payload.insert(QStringLiteral("controllerDeviceId"),
                       room.activeControllerDeviceId);
        SendEnvelope(socket, QStringLiteral("room_control_granted"), {},
                     payload);
    }

    void SendRoomControlRevoked(QWebSocket* socket,
                                const RoomState& room,
                                const QString& initiatorDeviceId,
                                const QString& reasonCode)
    {
        if (!socket) {
            return;
        }
        QJsonObject payload;
        payload.insert(QStringLiteral("roomId"), room.roomId);
        payload.insert(QStringLiteral("initiatorDeviceId"),
                       initiatorDeviceId);
        payload.insert(QStringLiteral("reasonCode"), reasonCode.left(64));
        SendEnvelope(socket, QStringLiteral("room_control_revoked"), {},
                     payload);
    }

    void CancelPendingRoomControl(RoomState& room,
                                  const QString& reasonCode,
                                  const QString& reasonMessage)
    {
        if (room.controlRequestId.isEmpty()) {
            room.pendingControllerDeviceId.clear();
            room.controlRequestExpiresAtMs = 0;
            return;
        }
        const QString requestId = room.controlRequestId;
        const QString requesterDeviceId = room.pendingControllerDeviceId;
        QWebSocket* requester =
            devices_.value(requesterDeviceId, nullptr);
        QWebSocket* sharer =
            devices_.value(room.screenSharerDeviceId, nullptr);
        SendRoomControlResult(requester, room, requestId, false,
                              reasonCode, reasonMessage);
        if (sharer && sharer != requester) {
            SendRoomControlResult(sharer, room, requestId, false,
                                  reasonCode, reasonMessage);
        }
        room.controlRequestId.clear();
        room.pendingControllerDeviceId.clear();
        room.controlRequestExpiresAtMs = 0;
    }

    void RevokeRoomControl(RoomState& room,
                           const QString& initiatorDeviceId,
                           const QString& reasonCode)
    {
        if (room.activeControllerDeviceId.isEmpty()) {
            room.controlGrantId.clear();
            return;
        }
        QWebSocket* controller =
            devices_.value(room.activeControllerDeviceId, nullptr);
        QWebSocket* sharer =
            devices_.value(room.screenSharerDeviceId, nullptr);
        SendRoomControlRevoked(controller, room, initiatorDeviceId,
                               reasonCode);
        if (sharer && sharer != controller) {
            SendRoomControlRevoked(sharer, room, initiatorDeviceId,
                                   reasonCode);
        }
        room.activeControllerDeviceId.clear();
        room.controlGrantId.clear();
    }

    void ResetRoomScreenShare(RoomState& room,
                              const QString& initiatorDeviceId,
                              const QString& reasonCode)
    {
        CancelPendingRoomScreenShareSwitch(
            room, reasonCode,
            QStringLiteral("The takeover request ended because screen sharing changed."));
        CancelPendingRoomControl(
            room, reasonCode,
            QStringLiteral("The control request ended because screen sharing changed."));
        RevokeRoomControl(room, initiatorDeviceId, reasonCode);
        if (room.screenShareState != QStringLiteral("idle") ||
            !room.screenSharerDeviceId.isEmpty() ||
            !room.pendingScreenSharerDeviceId.isEmpty()) {
            ++room.screenShareEpoch;
        }
        room.screenShareState = QStringLiteral("idle");
        room.screenSharerDeviceId.clear();
        room.pendingScreenSharerDeviceId.clear();
        room.screenShareGrantId.clear();
    }

    void HandleRoomScreenShareRequest(ClientState* client,
                                      const QJsonObject& payload)
    {
        const QString roomId =
            payload.value(QStringLiteral("roomId")).toString();
        if (roomId.isEmpty() || roomId.size() > 128) {
            SendError(client->socket, {},
                      QStringLiteral("invalid_room_screen_share_request"),
                      QStringLiteral("A valid room ID is required."));
            return;
        }
        auto roomIt = rooms_.find(roomId);
        const QString deviceId = client->claims.deviceId;
        if (roomIt == rooms_.end()) {
            SendError(client->socket, {}, QStringLiteral("room_not_found"),
                      QStringLiteral("The room does not exist."));
            return;
        }
        if (!IsOnlineRoomMember(*roomIt, deviceId)) {
            SendError(client->socket, {},
                      QStringLiteral("room_resume_required"),
                      QStringLiteral("The room membership must be active before sharing."));
            return;
        }
        if (roomIt->screenShareState == QStringLiteral("recovering")) {
            SendError(client->socket, {},
                      QStringLiteral("room_screen_share_recovering"),
                      QStringLiteral("Screen sharing cannot switch while the current sharer is recovering."));
            return;
        }
        if ((roomIt->screenShareState == QStringLiteral("active") &&
             roomIt->screenSharerDeviceId == deviceId) ||
            (roomIt->screenShareState == QStringLiteral("switching") &&
             roomIt->pendingScreenSharerDeviceId == deviceId)) {
            SendRoomScreenShareGranted(client->socket, *roomIt);
            return;
        }

        if (roomIt->screenShareState == QStringLiteral("active") &&
            !roomIt->screenSharerDeviceId.isEmpty()) {
            if (!roomIt->screenShareSwitchRequestId.isEmpty()) {
                if (roomIt->screenShareSwitchRequesterDeviceId ==
                    deviceId) {
                    SendRoomScreenShareSwitchPending(
                        client->socket, *roomIt);
                    return;
                }
                SendError(
                    client->socket, {},
                    QStringLiteral("room_screen_share_switch_pending"),
                    QStringLiteral("Another screen-share takeover request is already pending."));
                return;
            }
            if (!IsOnlineRoomMember(
                    *roomIt, roomIt->screenSharerDeviceId)) {
                SendError(
                    client->socket, {},
                    QStringLiteral("room_screen_share_recovering"),
                    QStringLiteral("The current screen sharer is not available to approve takeover."));
                return;
            }
            roomIt->screenShareSwitchRequestId =
                QUuid::createUuid().toString(QUuid::WithoutBraces);
            roomIt->screenShareSwitchRequesterDeviceId = deviceId;
            roomIt->screenShareSwitchRequestExpiresAtMs =
                QDateTime::currentMSecsSinceEpoch() +
                kRoomScreenShareSwitchRequestTimeoutMs;
            SendRoomScreenShareSwitchPending(client->socket, *roomIt);
            SendRoomScreenShareSwitchRequested(
                devices_.value(roomIt->screenSharerDeviceId, nullptr),
                *roomIt);
            return;
        }

        CancelPendingRoomControl(
            *roomIt, QStringLiteral("screen_sharer_changed"),
            QStringLiteral("The control request ended because the screen sharer changed."));
        RevokeRoomControl(*roomIt, deviceId,
                          QStringLiteral("screen_sharer_changed"));
        ++roomIt->screenShareEpoch;
        roomIt->screenShareState = QStringLiteral("switching");
        roomIt->screenSharerDeviceId.clear();
        roomIt->pendingScreenSharerDeviceId = deviceId;
        roomIt->screenShareGrantId = CreateRecoveryToken();
        BroadcastRoomState(*roomIt);
        // Deliver the authoritative switching snapshot first so the client
        // can attach media before acknowledging this private grant.
        SendRoomScreenShareGranted(client->socket, *roomIt);
    }

    void HandleRoomScreenShareReady(ClientState* client,
                                    const QJsonObject& payload)
    {
        const QString roomId =
            payload.value(QStringLiteral("roomId")).toString();
        const QString grantId =
            payload.value(QStringLiteral("grantId")).toString();
        if (roomId.isEmpty() || roomId.size() > 128 || grantId.isEmpty() ||
            grantId.size() > 256) {
            SendError(client->socket, {},
                      QStringLiteral("invalid_room_screen_share_ready"),
                      QStringLiteral("Room ID and screen-share grant are required."));
            return;
        }
        auto roomIt = rooms_.find(roomId);
        const QString deviceId = client->claims.deviceId;
        if (roomIt == rooms_.end() ||
            !IsOnlineRoomMember(*roomIt, deviceId) ||
            roomIt->screenShareState != QStringLiteral("switching") ||
            roomIt->pendingScreenSharerDeviceId != deviceId ||
            roomIt->screenShareGrantId != grantId) {
            SendError(client->socket, {},
                      QStringLiteral("room_screen_share_grant_invalid"),
                      QStringLiteral("The screen-share grant is stale or unauthorized."));
            return;
        }
        roomIt->screenShareState = QStringLiteral("active");
        roomIt->screenSharerDeviceId = deviceId;
        roomIt->pendingScreenSharerDeviceId.clear();
        BroadcastRoomState(*roomIt);
    }

    void HandleRoomScreenShareSwitchResponse(
        ClientState* client,
        const QJsonObject& payload)
    {
        const QString roomId =
            payload.value(QStringLiteral("roomId")).toString();
        const QString requestId =
            payload.value(QStringLiteral("requestId")).toString();
        const QJsonValue acceptedValue =
            payload.value(QStringLiteral("accepted"));
        const QString reasonCode =
            payload.value(QStringLiteral("reasonCode")).toString();
        if (roomId.isEmpty() || roomId.size() > 128 ||
            requestId.isEmpty() || requestId.size() > 128 ||
            !acceptedValue.isBool() || reasonCode.size() > 64) {
            SendError(
                client->socket, {},
                QStringLiteral("invalid_room_screen_share_switch_response"),
                QStringLiteral("The screen-share takeover response is invalid."));
            return;
        }
        auto roomIt = rooms_.find(roomId);
        const QString sharerId = client->claims.deviceId;
        if (roomIt == rooms_.end() ||
            roomIt->screenShareState != QStringLiteral("active") ||
            roomIt->screenSharerDeviceId != sharerId ||
            roomIt->screenShareSwitchRequestId != requestId) {
            SendError(
                client->socket, {},
                QStringLiteral("room_screen_share_switch_not_found"),
                QStringLiteral("The screen-share takeover request is no longer pending."));
            return;
        }
        if (roomIt->screenShareSwitchRequestExpiresAtMs <=
            QDateTime::currentMSecsSinceEpoch()) {
            CancelPendingRoomScreenShareSwitch(
                *roomIt,
                QStringLiteral("screen_share_switch_timeout"),
                QStringLiteral("The screen-share takeover request expired."));
            return;
        }
        if (!acceptedValue.toBool()) {
            CancelPendingRoomScreenShareSwitch(
                *roomIt,
                reasonCode.isEmpty()
                    ? QStringLiteral("screen_share_switch_rejected_by_sharer")
                    : reasonCode,
                QStringLiteral("The current screen sharer rejected the takeover request."));
            return;
        }

        const QString requesterId =
            roomIt->screenShareSwitchRequesterDeviceId;
        QWebSocket* requester = devices_.value(requesterId, nullptr);
        if (!requester ||
            !IsOnlineRoomMember(*roomIt, requesterId)) {
            CancelPendingRoomScreenShareSwitch(
                *roomIt,
                QStringLiteral("screen_share_switch_requester_offline"),
                QStringLiteral("The requesting screen sharer is offline."));
            return;
        }

        SendRoomScreenShareSwitchResult(
            requester, *roomIt, requestId, true, {}, {});
        SendRoomScreenShareSwitchResult(
            client->socket, *roomIt, requestId, true, {}, {});
        roomIt->screenShareSwitchRequestId.clear();
        roomIt->screenShareSwitchRequesterDeviceId.clear();
        roomIt->screenShareSwitchRequestExpiresAtMs = 0;

        CancelPendingRoomControl(
            *roomIt, QStringLiteral("screen_sharer_changed"),
            QStringLiteral("The control request ended because the screen sharer changed."));
        RevokeRoomControl(
            *roomIt, sharerId, QStringLiteral("screen_sharer_changed"));
        ++roomIt->screenShareEpoch;
        roomIt->screenShareState = QStringLiteral("switching");
        roomIt->screenSharerDeviceId.clear();
        roomIt->pendingScreenSharerDeviceId = requesterId;
        roomIt->screenShareGrantId = CreateRecoveryToken();
        BroadcastRoomState(*roomIt);
        SendRoomScreenShareGranted(requester, *roomIt);
    }

    void HandleRoomScreenShareSwitchCancel(
        ClientState* client,
        const QJsonObject& payload)
    {
        const QString roomId =
            payload.value(QStringLiteral("roomId")).toString();
        const QString requestId =
            payload.value(QStringLiteral("requestId")).toString();
        const QString reasonCode =
            payload.value(QStringLiteral("reasonCode")).toString();
        if (roomId.isEmpty() || roomId.size() > 128 ||
            requestId.isEmpty() || requestId.size() > 128 ||
            reasonCode.size() > 64) {
            SendError(
                client->socket, {},
                QStringLiteral("invalid_room_screen_share_switch_cancel"),
                QStringLiteral("The screen-share takeover cancellation is invalid."));
            return;
        }
        auto roomIt = rooms_.find(roomId);
        if (roomIt == rooms_.end() ||
            roomIt->screenShareSwitchRequestId != requestId ||
            roomIt->screenShareSwitchRequesterDeviceId !=
                client->claims.deviceId) {
            SendError(
                client->socket, {},
                QStringLiteral("room_screen_share_switch_not_found"),
                QStringLiteral("The screen-share takeover request is no longer pending."));
            return;
        }
        CancelPendingRoomScreenShareSwitch(
            *roomIt,
            reasonCode.isEmpty()
                ? QStringLiteral("screen_share_switch_cancelled")
                : reasonCode,
            QStringLiteral("The requester cancelled the screen-share takeover."));
    }

    void HandleRoomScreenShareStop(ClientState* client,
                                   const QJsonObject& payload)
    {
        const QString roomId =
            payload.value(QStringLiteral("roomId")).toString();
        const QString grantId =
            payload.value(QStringLiteral("grantId")).toString();
        const QString reasonCode =
            payload.value(QStringLiteral("reasonCode")).toString();
        if (roomId.isEmpty() || roomId.size() > 128 || grantId.isEmpty() ||
            grantId.size() > 256 || reasonCode.size() > 64) {
            SendError(client->socket, {},
                      QStringLiteral("invalid_room_screen_share_stop"),
                      QStringLiteral("The screen-share stop request is invalid."));
            return;
        }
        auto roomIt = rooms_.find(roomId);
        const QString deviceId = client->claims.deviceId;
        const bool ownsActive = roomIt != rooms_.end() &&
            roomIt->screenSharerDeviceId == deviceId;
        const bool ownsPending = roomIt != rooms_.end() &&
            roomIt->pendingScreenSharerDeviceId == deviceId;
        if (roomIt == rooms_.end() || (!ownsActive && !ownsPending) ||
            roomIt->screenShareGrantId != grantId) {
            SendError(client->socket, {},
                      QStringLiteral("room_screen_share_grant_invalid"),
                      QStringLiteral("Only the current screen sharer can stop this lease."));
            return;
        }
        ResetRoomScreenShare(
            *roomIt, deviceId,
            reasonCode.isEmpty() ? QStringLiteral("screen_share_stopped")
                                 : reasonCode);
        BroadcastRoomState(*roomIt);
    }

    void HandleRoomControlRequest(ClientState* client,
                                  const QJsonObject& payload)
    {
        const QString roomId =
            payload.value(QStringLiteral("roomId")).toString();
        if (roomId.isEmpty() || roomId.size() > 128) {
            SendError(client->socket, {},
                      QStringLiteral("invalid_room_control_request"),
                      QStringLiteral("A valid room ID is required."));
            return;
        }
        auto roomIt = rooms_.find(roomId);
        const QString requesterId = client->claims.deviceId;
        if (roomIt == rooms_.end()) {
            SendError(client->socket, {}, QStringLiteral("room_not_found"),
                      QStringLiteral("The room does not exist."));
            return;
        }
        if (!IsOnlineRoomMember(*roomIt, requesterId)) {
            SendError(client->socket, {},
                      QStringLiteral("room_resume_required"),
                      QStringLiteral("The room membership must be active before requesting control."));
            return;
        }
        if (roomIt->screenShareState != QStringLiteral("active") ||
            roomIt->screenSharerDeviceId.isEmpty() ||
            !IsOnlineRoomMember(*roomIt,
                                roomIt->screenSharerDeviceId)) {
            SendError(client->socket, {},
                      QStringLiteral("room_screen_share_not_active"),
                      QStringLiteral("There is no available screen sharer."));
            return;
        }
        if (roomIt->screenSharerDeviceId == requesterId) {
            SendError(client->socket, {},
                      QStringLiteral("room_control_self_not_allowed"),
                      QStringLiteral("A screen sharer cannot control itself."));
            return;
        }
        if (!roomIt->activeControllerDeviceId.isEmpty()) {
            SendError(client->socket, {},
                      QStringLiteral("room_control_occupied"),
                      QStringLiteral("Another member currently controls the shared screen."));
            return;
        }
        if (!roomIt->controlRequestId.isEmpty()) {
            SendError(client->socket, {},
                      QStringLiteral("room_control_request_pending"),
                      QStringLiteral("Another control request is already pending."));
            return;
        }

        roomIt->controlRequestId =
            QUuid::createUuid().toString(QUuid::WithoutBraces);
        roomIt->pendingControllerDeviceId = requesterId;
        roomIt->controlRequestExpiresAtMs =
            QDateTime::currentMSecsSinceEpoch() +
            kRoomControlRequestTimeoutMs;
        QJsonObject forwarded;
        forwarded.insert(QStringLiteral("roomId"), roomId);
        forwarded.insert(QStringLiteral("requestId"),
                         roomIt->controlRequestId);
        forwarded.insert(QStringLiteral("requesterDeviceId"), requesterId);
        forwarded.insert(QStringLiteral("requesterDeviceName"),
                         client->deviceName);
        BroadcastRoomState(*roomIt);
        SendEnvelope(devices_.value(roomIt->screenSharerDeviceId, nullptr),
                     QStringLiteral("room_control_requested"), {},
                     forwarded);
    }

    void HandleRoomControlResponse(ClientState* client,
                                   const QJsonObject& payload)
    {
        const QString roomId =
            payload.value(QStringLiteral("roomId")).toString();
        const QString requestId =
            payload.value(QStringLiteral("requestId")).toString();
        const QJsonValue acceptedValue =
            payload.value(QStringLiteral("accepted"));
        const QString reasonCode =
            payload.value(QStringLiteral("reasonCode")).toString();
        if (roomId.isEmpty() || roomId.size() > 128 || requestId.isEmpty() ||
            requestId.size() > 128 || !acceptedValue.isBool() ||
            reasonCode.size() > 64) {
            SendError(client->socket, {},
                      QStringLiteral("invalid_room_control_response"),
                      QStringLiteral("The room control response is invalid."));
            return;
        }
        auto roomIt = rooms_.find(roomId);
        const QString sharerId = client->claims.deviceId;
        if (roomIt == rooms_.end() ||
            roomIt->screenShareState != QStringLiteral("active") ||
            roomIt->screenSharerDeviceId != sharerId ||
            roomIt->controlRequestId != requestId) {
            SendError(client->socket, {},
                      QStringLiteral("room_control_request_not_found"),
                      QStringLiteral("The control request is no longer pending."));
            return;
        }
        if (roomIt->controlRequestExpiresAtMs <=
            QDateTime::currentMSecsSinceEpoch()) {
            CancelPendingRoomControl(
                *roomIt, QStringLiteral("control_request_timeout"),
                QStringLiteral("The control request expired."));
            BroadcastRoomState(*roomIt);
            return;
        }

        const QString requesterId = roomIt->pendingControllerDeviceId;
        QWebSocket* requester = devices_.value(requesterId, nullptr);
        if (!acceptedValue.toBool()) {
            const QString rejectedReason = reasonCode.isEmpty()
                ? QStringLiteral("rejected_by_screen_sharer")
                : reasonCode;
            CancelPendingRoomControl(
                *roomIt, rejectedReason,
                QStringLiteral("The screen sharer rejected the control request."));
            BroadcastRoomState(*roomIt);
            return;
        }
        if (!IsOnlineRoomMember(*roomIt, requesterId) || !requester) {
            CancelPendingRoomControl(
                *roomIt, QStringLiteral("controller_offline"),
                QStringLiteral("The requesting controller is offline."));
            BroadcastRoomState(*roomIt);
            return;
        }

        SendRoomControlResult(requester, *roomIt, requestId, true, {}, {});
        if (requester != client->socket) {
            SendRoomControlResult(client->socket, *roomIt, requestId, true,
                                  {}, {});
        }
        roomIt->controlRequestId.clear();
        roomIt->pendingControllerDeviceId.clear();
        roomIt->controlRequestExpiresAtMs = 0;
        roomIt->activeControllerDeviceId = requesterId;
        roomIt->controlGrantId = CreateRecoveryToken();
        SendRoomControlGranted(requester, *roomIt);
        SendRoomControlGranted(client->socket, *roomIt);
        BroadcastRoomState(*roomIt);
    }

    void HandleRoomControlRelease(ClientState* client,
                                  const QJsonObject& payload)
    {
        const QString roomId =
            payload.value(QStringLiteral("roomId")).toString();
        const QString grantId =
            payload.value(QStringLiteral("grantId")).toString();
        const QString reasonCode =
            payload.value(QStringLiteral("reasonCode")).toString();
        if (roomId.isEmpty() || roomId.size() > 128 || grantId.isEmpty() ||
            grantId.size() > 256 || reasonCode.size() > 64) {
            SendError(client->socket, {},
                      QStringLiteral("invalid_room_control_release"),
                      QStringLiteral("The control release request is invalid."));
            return;
        }
        auto roomIt = rooms_.find(roomId);
        const QString deviceId = client->claims.deviceId;
        if (roomIt == rooms_.end() || roomIt->controlGrantId != grantId ||
            (roomIt->activeControllerDeviceId != deviceId &&
             roomIt->screenSharerDeviceId != deviceId)) {
            SendError(client->socket, {},
                      QStringLiteral("room_control_grant_invalid"),
                      QStringLiteral("Only the active controller or screen sharer can release control."));
            return;
        }
        RevokeRoomControl(
            *roomIt, deviceId,
            reasonCode.isEmpty() ? QStringLiteral("control_released")
                                 : reasonCode);
        BroadcastRoomState(*roomIt);
    }

    QJsonArray SerializeIceServers() const
    {
        QJsonArray iceServers;
        if (config_.iceServerUrls.isEmpty()) {
            return iceServers;
        }
        QJsonArray urls;
        for (const auto& url : config_.iceServerUrls) {
            urls.append(url);
        }
        QJsonObject server;
        server.insert(QStringLiteral("urls"), urls);
        iceServers.append(server);
        return iceServers;
    }
