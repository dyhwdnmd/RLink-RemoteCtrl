// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

    void OnDisconnected(QWebSocket* socket)
    {
        auto clientIt = clients_.find(socket);
        if (clientIt == clients_.end()) {
            return;
        }
        if (clientIt->second->userInfoReply) {
            userInfoClient_.Cancel(clientIt->second->userInfoReply);
            clientIt->second->userInfoReply = nullptr;
        }
        const QString deviceId = clientIt->second->claims.deviceId;
        const qint64 trustedUserId = clientIt->second->trustedUserId;
        const bool ownsRegistration =
            clientIt->second->registered &&
            devices_.value(deviceId, nullptr) == socket;
        if (ownsRegistration) {
            devices_.remove(deviceId);
            MarkDeviceDisconnected(deviceId);
        }

        RemoveAccountConnection(clientIt->second.get());
        if (!clientIt->second->authenticated) {
            unauthenticatedConnectionCount_ =
                std::max(0, unauthenticatedConnectionCount_ - 1);
        }
        if (!clientIt->second->peerKey.isEmpty()) {
            auto ipIt = connectionCountByIp_.find(
                clientIt->second->peerKey);
            if (ipIt != connectionCountByIp_.end()) {
                if (*ipIt <= 1) {
                    connectionCountByIp_.erase(ipIt);
                } else {
                    --(*ipIt);
                }
            }
        }
        totalRememberedMessageIds_ = std::max<qint64>(
            0, totalRememberedMessageIds_ -
                   clientIt->second->receivedMessageIds.size());
        clients_.erase(clientIt);
        if (ownsRegistration) {
            BroadcastOwnedDevices(trustedUserId);
        }
    }

    void MarkDeviceDisconnected(const QString& deviceId)
    {
        const qint64 now = QDateTime::currentMSecsSinceEpoch();
        CancelPendingRoomJoinForDevice(deviceId);

        const QString roomId = deviceRooms_.value(deviceId);
        auto roomIt = rooms_.find(roomId);
        if (roomIt != rooms_.end()) {
            auto memberIt = roomIt->members.find(deviceId);
            if (memberIt != roomIt->members.end() &&
                memberIt->disconnectedAtMs == 0) {
                memberIt->disconnectedAtMs = now;
                if (roomIt->pendingControllerDeviceId == deviceId ||
                    (!roomIt->controlRequestId.isEmpty() &&
                     roomIt->screenSharerDeviceId == deviceId)) {
                    CancelPendingRoomControl(
                        *roomIt, QStringLiteral("signaling_disconnected"),
                        QStringLiteral("The control request ended because a participant disconnected."));
                }
                if (roomIt->screenShareSwitchRequesterDeviceId == deviceId ||
                    (!roomIt->screenShareSwitchRequestId.isEmpty() &&
                     roomIt->screenSharerDeviceId == deviceId)) {
                    CancelPendingRoomScreenShareSwitch(
                        *roomIt,
                        roomIt->screenSharerDeviceId == deviceId
                            ? QStringLiteral("screen_share_switch_sharer_offline")
                            : QStringLiteral("screen_share_switch_requester_offline"),
                        QStringLiteral("The takeover request ended because a participant disconnected."));
                }
                if (roomIt->screenSharerDeviceId == deviceId &&
                    roomIt->screenShareState == QStringLiteral("active")) {
                    roomIt->screenShareState =
                        QStringLiteral("recovering");
                }
                BroadcastRoomState(*roomIt);
            }
        }

        QStringList sessionsToRemove;
        for (auto sessionIt = sessions_.begin();
             sessionIt != sessions_.end(); ++sessionIt) {
            QString peerDeviceId;
            qint64* disconnectedAtMs = nullptr;
            if (sessionIt->requesterDeviceId == deviceId) {
                peerDeviceId = sessionIt->targetDeviceId;
                disconnectedAtMs = &sessionIt->requesterDisconnectedAtMs;
            } else if (sessionIt->targetDeviceId == deviceId) {
                peerDeviceId = sessionIt->requesterDeviceId;
                disconnectedAtMs = &sessionIt->targetDisconnectedAtMs;
            } else {
                continue;
            }
            if (sessionIt->phase == SessionState::Phase::kActive) {
                if (*disconnectedAtMs == 0) {
                    *disconnectedAtMs = now;
                    if (QWebSocket* peer =
                            devices_.value(peerDeviceId, nullptr)) {
                        QJsonObject payload;
                        payload.insert(QStringLiteral("peerDeviceId"),
                                       deviceId);
                        payload.insert(QStringLiteral("recoveryWindowMs"),
                                       config_.sessionRecoveryWindowMs);
                        SendEnvelope(peer,
                                     QStringLiteral("session_suspended"),
                                     sessionIt.key(), payload);
                    }
                }
                continue;
            }
            if (QWebSocket* peer = devices_.value(peerDeviceId, nullptr)) {
                SendSessionEnded(
                    peer, sessionIt.key(), deviceId,
                    QStringLiteral("peer_signaling_disconnected"),
                    QStringLiteral("cancelled"));
            }
            sessionsToRemove.append(sessionIt.key());
        }
        for (const auto& sessionId : sessionsToRemove) {
            RemoveSession(sessionId);
        }
    }

    void SweepIdleClients()
    {
        const qint64 now = QDateTime::currentMSecsSinceEpoch();
        QList<QWebSocket*> idleSockets;
        for (const auto& [socket, state] : clients_) {
            if (!state->authenticated &&
                state->authenticationDeadlineMs > 0 &&
                state->authenticationDeadlineMs <= now) {
                idleSockets.append(socket);
                continue;
            }
            if (config_.clientIdleTimeoutMs > 0) {
                if (now - state->lastActivityMs >
                    config_.clientIdleTimeoutMs) {
                    idleSockets.append(socket);
                }
            }
        }
        for (QWebSocket* socket : idleSockets) {
            socket->abort();
        }
        SweepPendingSessions(now);
        SweepRecoverySessions(now);
        SweepRoomJoinRequests(now);
        SweepRoomScreenShareSwitchRequests(now);
        SweepRoomControlRequests(now);
        SweepRoomRecoveries(now);
        PruneRateLimiters(now);
    }

    void SweepPendingSessions(qint64 now)
    {
        QStringList sessionsToRemove;
        for (auto sessionIt = sessions_.cbegin();
             sessionIt != sessions_.cend(); ++sessionIt) {
            if (sessionIt->phase != SessionState::Phase::kPending ||
                sessionIt->pendingExpiresAtMs <= 0 ||
                sessionIt->pendingExpiresAtMs > now) {
                continue;
            }
            if (QWebSocket* requester = devices_.value(
                    sessionIt->requesterDeviceId, nullptr)) {
                QJsonObject response;
                response.insert(QStringLiteral("accepted"), false);
                response.insert(QStringLiteral("reasonCode"),
                                QStringLiteral("session_request_timeout"));
                response.insert(
                    QStringLiteral("reasonMessage"),
                    QStringLiteral("The session request timed out."));
                SendEnvelope(requester, QStringLiteral("session_response"),
                             sessionIt.key(), response);
            }
            if (QWebSocket* target = devices_.value(
                    sessionIt->targetDeviceId, nullptr)) {
                SendSessionEnded(
                    target, sessionIt.key(),
                    sessionIt->requesterDeviceId,
                    QStringLiteral("session_request_timeout"),
                    QStringLiteral("cancelled"));
            }
            sessionsToRemove.append(sessionIt.key());
        }
        for (const auto& sessionId : sessionsToRemove) {
            RemoveSession(sessionId);
        }
    }

    void SweepRecoverySessions(qint64 now)
    {
        QStringList sessionsToRemove;
        for (auto sessionIt = sessions_.cbegin();
             sessionIt != sessions_.cend(); ++sessionIt) {
            if (sessionIt->phase != SessionState::Phase::kActive) {
                continue;
            }
            QString expiredDeviceId;
            if (sessionIt->requesterDisconnectedAtMs > 0 &&
                now - sessionIt->requesterDisconnectedAtMs >
                    config_.sessionRecoveryWindowMs) {
                expiredDeviceId = sessionIt->requesterDeviceId;
            } else if (sessionIt->targetDisconnectedAtMs > 0 &&
                       now - sessionIt->targetDisconnectedAtMs >
                           config_.sessionRecoveryWindowMs) {
                expiredDeviceId = sessionIt->targetDeviceId;
            }
            if (expiredDeviceId.isEmpty()) {
                continue;
            }
            if (QWebSocket* requester = devices_.value(
                    sessionIt->requesterDeviceId, nullptr)) {
                SendSessionEnded(requester, sessionIt.key(), expiredDeviceId,
                                 QStringLiteral("signaling_recovery_timeout"),
                                 QStringLiteral("closed"));
            }
            if (QWebSocket* target = devices_.value(
                    sessionIt->targetDeviceId, nullptr)) {
                SendSessionEnded(target, sessionIt.key(), expiredDeviceId,
                                 QStringLiteral("signaling_recovery_timeout"),
                                 QStringLiteral("closed"));
            }
            sessionsToRemove.append(sessionIt.key());
        }
        for (const auto& sessionId : sessionsToRemove) {
            RemoveSession(sessionId);
        }
    }

    void SweepRoomJoinRequests(qint64 now)
    {
        QStringList requestsToExpire;
        for (auto requestIt = roomJoinRequests_.cbegin();
             requestIt != roomJoinRequests_.cend(); ++requestIt) {
            if (requestIt->expiresAtMs <= now) {
                requestsToExpire.append(requestIt.key());
            }
        }

        for (const auto& requestId : requestsToExpire) {
            const RoomJoinRequestState request =
                roomJoinRequests_.take(requestId);
            if (request.requestId.isEmpty()) {
                continue;
            }
            if (pendingRoomJoinRequestByDevice_.value(
                    request.requesterDeviceId) == requestId) {
                pendingRoomJoinRequestByDevice_.remove(
                    request.requesterDeviceId);
            }

            QWebSocket* requesterSocket =
                devices_.value(request.requesterDeviceId, nullptr);
            if (requesterSocket) {
                SendRoomJoinResult(
                    requesterSocket, request.roomId, requestId, false,
                    QStringLiteral("join_request_timeout"),
                    QStringLiteral("The room join request expired."));
            }

            const auto roomIt = rooms_.constFind(request.roomId);
            if (roomIt == rooms_.cend()) {
                continue;
            }
            const auto ownerMemberIt =
                roomIt->members.constFind(roomIt->ownerDeviceId);
            if (ownerMemberIt == roomIt->members.cend() ||
                ownerMemberIt->disconnectedAtMs != 0) {
                continue;
            }
            if (QWebSocket* ownerSocket =
                    devices_.value(roomIt->ownerDeviceId, nullptr);
                ownerSocket && ownerSocket != requesterSocket) {
                SendRoomJoinResult(
                    ownerSocket, request.roomId, requestId, false,
                    QStringLiteral("join_request_timeout"),
                    QStringLiteral("The room join request expired."));
            }
        }
    }

    void SweepRoomControlRequests(qint64 now)
    {
        for (auto roomIt = rooms_.begin(); roomIt != rooms_.end(); ++roomIt) {
            if (roomIt->controlRequestId.isEmpty() ||
                roomIt->controlRequestExpiresAtMs > now) {
                continue;
            }
            CancelPendingRoomControl(
                *roomIt, QStringLiteral("control_request_timeout"),
                QStringLiteral("The control request expired."));
            BroadcastRoomState(*roomIt);
        }
    }

    void SweepRoomScreenShareSwitchRequests(qint64 now)
    {
        for (auto roomIt = rooms_.begin(); roomIt != rooms_.end();
             ++roomIt) {
            if (roomIt->screenShareSwitchRequestId.isEmpty() ||
                roomIt->screenShareSwitchRequestExpiresAtMs > now) {
                continue;
            }
            CancelPendingRoomScreenShareSwitch(
                *roomIt,
                QStringLiteral("screen_share_switch_timeout"),
                QStringLiteral("The screen-share takeover request expired."));
        }
    }

    void SweepRoomRecoveries(qint64 now)
    {
        QStringList roomsToClose;
        for (auto roomIt = rooms_.begin();
             roomIt != rooms_.end(); ++roomIt) {
            const auto ownerIt =
                roomIt->members.constFind(roomIt->ownerDeviceId);
            if (ownerIt == roomIt->members.cend() ||
                (ownerIt->disconnectedAtMs > 0 &&
                 now - ownerIt->disconnectedAtMs >
                     config_.sessionRecoveryWindowMs)) {
                roomsToClose.append(roomIt.key());
                continue;
            }

            QStringList expiredMembers;
            for (auto memberIt = roomIt->members.cbegin();
                 memberIt != roomIt->members.cend(); ++memberIt) {
                if (memberIt.key() == roomIt->ownerDeviceId ||
                    memberIt->disconnectedAtMs == 0) {
                    continue;
                }
                if (now - memberIt->disconnectedAtMs >
                    config_.sessionRecoveryWindowMs) {
                    expiredMembers.append(memberIt.key());
                }
            }
            for (const auto& deviceId : expiredMembers) {
                if (roomIt->screenSharerDeviceId == deviceId ||
                    roomIt->pendingScreenSharerDeviceId == deviceId) {
                    ResetRoomScreenShare(
                        *roomIt, deviceId,
                        QStringLiteral("signaling_recovery_timeout"));
                } else {
                    if (roomIt->pendingControllerDeviceId == deviceId) {
                        CancelPendingRoomControl(
                            *roomIt,
                            QStringLiteral("signaling_recovery_timeout"),
                            QStringLiteral("The requesting controller did not recover signaling."));
                    }
                    if (roomIt->activeControllerDeviceId == deviceId) {
                        RevokeRoomControl(
                            *roomIt, deviceId,
                            QStringLiteral("signaling_recovery_timeout"));
                    }
                }
                CloseRoomPairsForMember(
                    *roomIt, deviceId, deviceId,
                    QStringLiteral("signaling_recovery_timeout"));
                roomIt->members.remove(deviceId);
                if (deviceRooms_.value(deviceId) == roomIt.key()) {
                    deviceRooms_.remove(deviceId);
                }
            }
            if (!expiredMembers.isEmpty()) {
                BroadcastRoomState(*roomIt);
            }
        }

        for (const auto& roomId : roomsToClose) {
            const auto roomIt = rooms_.constFind(roomId);
            if (roomIt == rooms_.cend()) {
                continue;
            }
            const QString ownerDeviceId = roomIt->ownerDeviceId;
            CloseRoom(roomId, ownerDeviceId,
                      QStringLiteral("signaling_recovery_timeout"));
        }
    }

    ClientState* FindClient(QWebSocket* socket)
    {
        const auto it = clients_.find(socket);
        return it == clients_.end() ? nullptr : it->second.get();
    }
