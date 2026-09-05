// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

    bool RelayRoomPairMessage(ClientState* sender,
                              const QString& pairId,
                              const QString& type,
                              const QJsonObject& payload)
    {
        const QString roomId = pairRooms_.value(pairId);
        if (roomId.isEmpty()) {
            return false;
        }
        auto roomIt = rooms_.find(roomId);
        if (roomIt == rooms_.end()) {
            pairRooms_.remove(pairId);
            return true;
        }
        const auto pairIt = roomIt->pairs.constFind(pairId);
        if (pairIt == roomIt->pairs.cend()) {
            pairRooms_.remove(pairId);
            return true;
        }

        const QString senderDeviceId = sender->claims.deviceId;
        const QString peerDeviceId =
            PairPeerDeviceId(*pairIt, senderDeviceId);
        if (peerDeviceId.isEmpty() ||
            !roomIt->members.contains(senderDeviceId)) {
            SendError(sender->socket, pairId,
                      QStringLiteral("room_pair_access_denied"),
                      QStringLiteral("This device is not a participant in the room pair."));
            return true;
        }
        if (type == QStringLiteral("sdp")) {
            const QString sdpType =
                payload.value(QStringLiteral("type")).toString();
            const bool senderIsOfferer =
                pairIt->offererDeviceId == senderDeviceId;
            if ((sdpType == QStringLiteral("offer") && !senderIsOfferer) ||
                (sdpType == QStringLiteral("answer") && senderIsOfferer)) {
                SendError(sender->socket, pairId,
                          QStringLiteral("room_pair_sdp_role_violation"),
                          QStringLiteral("Room pair SDP does not match the assigned offerer role."));
                return true;
            }
            if (payload.value(QStringLiteral("negotiationGeneration"))
                    .toVariant().toULongLong() == 0) {
                SendError(sender->socket, pairId,
                          QStringLiteral("invalid_negotiation_generation"),
                          QStringLiteral("The SDP negotiation generation is invalid."));
                return true;
            }
        } else if (type == QStringLiteral("ice_candidate")) {
            if (payload.value(QStringLiteral("negotiationGeneration"))
                    .toVariant().toULongLong() == 0) {
                SendError(sender->socket, pairId,
                          QStringLiteral("invalid_negotiation_generation"),
                          QStringLiteral("The ICE candidate negotiation generation is invalid."));
                return true;
            }
        } else if (type == QStringLiteral("ice_restart_request") ||
                   type == QStringLiteral("ice_restart_cancel")) {
            const bool senderIsOfferer =
                pairIt->offererDeviceId == senderDeviceId;
            if (senderIsOfferer ||
                payload.value(QStringLiteral("observedGeneration"))
                    .toVariant().toULongLong() == 0 ||
                payload.value(QStringLiteral("requestSequence"))
                    .toVariant().toULongLong() == 0) {
                SendError(sender->socket, pairId,
                          type == QStringLiteral("ice_restart_request")
                              ? QStringLiteral("invalid_ice_restart_request")
                              : QStringLiteral("invalid_ice_restart_cancel"),
                          QStringLiteral("Only the fixed answerer may manage an ICE restart request."));
                return true;
            }
        }

        const auto peerMemberIt =
            roomIt->members.constFind(peerDeviceId);
        QWebSocket* peerSocket = devices_.value(peerDeviceId, nullptr);
        if (peerMemberIt == roomIt->members.cend() ||
            peerMemberIt->disconnectedAtMs != 0 || !peerSocket) {
            SendError(sender->socket, pairId,
                      QStringLiteral("room_pair_peer_offline"),
                      QStringLiteral("The room pair peer is temporarily offline."));
            return true;
        }
        SendEnvelope(peerSocket, type, pairId, payload);
        return true;
    }

    void RelaySessionMessage(ClientState* sender,
                             const QString& sessionId,
                             const QString& type,
                             const QJsonObject& payload)
    {
        auto sessionIt = sessions_.find(sessionId);
        if (sessionIt == sessions_.end()) {
            if (RelayRoomPairMessage(sender, sessionId, type, payload)) {
                return;
            }
            // A candidate already queued by Qt may arrive after an idempotent
            // session_close. Silently discard it instead of turning a normal
            // close race into a client-visible signaling failure.
            return;
        }
        if (sessionIt->phase != SessionState::Phase::kActive) {
            SendError(sender->socket, sessionId,
                      QStringLiteral("active_session_not_found"),
                      QStringLiteral("The active session was not found."));
            return;
        }
        QString peerDeviceId;
        if (sessionIt->requesterDeviceId == sender->claims.deviceId) {
            peerDeviceId = sessionIt->targetDeviceId;
        } else if (sessionIt->targetDeviceId == sender->claims.deviceId) {
            peerDeviceId = sessionIt->requesterDeviceId;
        } else {
            SendError(sender->socket, sessionId,
                      QStringLiteral("session_access_denied"),
                      QStringLiteral("This device is not a session participant."));
            return;
        }
        if ((type == QStringLiteral("ice_restart_request") ||
             type == QStringLiteral("ice_restart_cancel")) &&
            (sessionIt->requesterDeviceId == sender->claims.deviceId ||
             payload.value(QStringLiteral("observedGeneration"))
                     .toVariant().toULongLong() == 0 ||
             payload.value(QStringLiteral("requestSequence"))
                     .toVariant().toULongLong() == 0)) {
            SendError(sender->socket, sessionId,
                      type == QStringLiteral("ice_restart_request")
                          ? QStringLiteral("invalid_ice_restart_request")
                          : QStringLiteral("invalid_ice_restart_cancel"),
                      QStringLiteral("Only the fixed answerer may manage an ICE restart request."));
            return;
        }
        QWebSocket* peerSocket = devices_.value(peerDeviceId, nullptr);
        if (!peerSocket) {
            SendError(sender->socket, sessionId,
                      QStringLiteral("peer_offline"),
                      QStringLiteral("The peer device is offline."));
            return;
        }
        SendEnvelope(peerSocket, type, sessionId, payload);
    }

    void CloseSession(ClientState* sender,
                      const QString& sessionId,
                      const QString& type,
                      const QJsonObject& payload)
    {
        if (sessionId.isEmpty()) {
            SendError(sender->socket, {},
                      QStringLiteral("session_id_required"),
                      QStringLiteral("A session ID is required."));
            return;
        }
        const QString disposition =
            type == QStringLiteral("session_cancel")
                ? QStringLiteral("cancelled")
                : QStringLiteral("closed");
        QString reasonCode =
            payload.value(QStringLiteral("reasonCode")).toString().left(64);
        if (reasonCode.isEmpty()) {
            reasonCode = disposition == QStringLiteral("cancelled")
                             ? QStringLiteral("cancelled_by_local_user")
                             : QStringLiteral("closed_by_local_user");
        }
        auto sessionIt = sessions_.find(sessionId);
        if (sessionIt == sessions_.end()) {
            // Closing an already-ended session is successful and reveals no
            // information about any previous participants.
            SendSessionEnded(sender->socket, sessionId,
                             sender->claims.deviceId, reasonCode,
                             disposition);
            return;
        }
        QString peerDeviceId;
        if (sessionIt->requesterDeviceId == sender->claims.deviceId) {
            peerDeviceId = sessionIt->targetDeviceId;
        } else if (sessionIt->targetDeviceId == sender->claims.deviceId) {
            peerDeviceId = sessionIt->requesterDeviceId;
        } else {
            SendError(sender->socket, sessionId,
                      QStringLiteral("session_access_denied"),
                      QStringLiteral("This device is not a session participant."));
            return;
        }
        if (QWebSocket* peer = devices_.value(peerDeviceId, nullptr)) {
            SendSessionEnded(peer, sessionId, sender->claims.deviceId,
                             reasonCode, disposition);
        }
        SendSessionEnded(sender->socket, sessionId,
                         sender->claims.deviceId, reasonCode,
                         disposition);
        RemoveSession(sessionId);
    }

    void SendSessionEnded(QWebSocket* socket,
                          const QString& sessionId,
                          const QString& initiatorDeviceId,
                          const QString& reasonCode,
                          const QString& disposition)
    {
        QJsonObject payload;
        payload.insert(QStringLiteral("initiatorDeviceId"),
                       initiatorDeviceId);
        payload.insert(QStringLiteral("reasonCode"), reasonCode);
        payload.insert(QStringLiteral("disposition"), disposition);
        SendEnvelope(socket, QStringLiteral("session_ended"), sessionId,
                     payload);
    }
