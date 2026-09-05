// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

    void HandleSessionRequest(ClientState* requester,
                              const QJsonObject& payload)
    {
        const QString targetDeviceId =
            payload.value(QStringLiteral("targetDeviceId")).toString();
        const QString purpose =
            payload.value(QStringLiteral("purpose")).toString();
        const QJsonValue permissionsValue =
            payload.value(QStringLiteral("permissions"));
        const bool ownedDeviceRequest = payload.value(
            QStringLiteral("ownedDeviceRequest")).toBool(false);
        const bool assistedRequest = payload.value(
            QStringLiteral("assistedRequest")).toBool(false);
        const QString verificationCode = payload.value(
            QStringLiteral("verificationCode")).toString();
        if (!AccessTokenService::IsValidDeviceId(targetDeviceId) ||
            targetDeviceId == requester->claims.deviceId ||
            !IsValidPurpose(purpose) || !permissionsValue.isArray() ||
            permissionsValue.toArray().size() > kMaximumPermissions ||
            (ownedDeviceRequest && assistedRequest) ||
            (assistedRequest &&
             (purpose != QStringLiteral("remote_control") ||
              verificationCode.size() != 6 ||
              !std::all_of(verificationCode.cbegin(),
                           verificationCode.cend(),
                           [](QChar value) { return value.isDigit(); })))) {
            SendError(requester->socket, {}, QStringLiteral("invalid_session_request"),
                      QStringLiteral("Session request fields are invalid."));
            return;
        }
        if (assistedRequest && !AllowAssistanceAttempt(
                requester, targetDeviceId,
                QDateTime::currentMSecsSinceEpoch())) {
            SendError(requester->socket, {}, QStringLiteral("rate_limited"),
                      QStringLiteral(
                          "Too many remote-assistance attempts. Wait before trying again."));
            return;
        }

        QJsonArray permissions;
        QSet<QString> uniquePermissions;
        for (const auto& permissionValue : permissionsValue.toArray()) {
            const QString permission = permissionValue.toString();
            if (permission.isEmpty() || permission.size() > 64 ||
                uniquePermissions.contains(permission)) {
                SendError(requester->socket, {},
                          QStringLiteral("invalid_session_permissions"),
                          QStringLiteral("Session permissions are invalid."));
                return;
            }
            uniquePermissions.insert(permission);
            permissions.append(permission);
        }

        if (DeviceHasRoom(requester->claims.deviceId) ||
            DeviceHasRoom(targetDeviceId)) {
            SendError(requester->socket, {}, QStringLiteral("device_busy"),
                      QStringLiteral("A room member cannot start a legacy session."));
            return;
        }

        QWebSocket* targetSocket = devices_.value(targetDeviceId, nullptr);
        if (!targetSocket) {
            SendError(requester->socket, {}, QStringLiteral("target_offline"),
                      QStringLiteral("The requested device is offline."));
            return;
        }
        ClientState* target = FindClient(targetSocket);
        const bool sameAccount = target && requester->trustedUserId > 0 &&
            target->trustedUserId == requester->trustedUserId;
        const bool autoAccept = sameAccount && ownedDeviceRequest;
        if (sessions_.size() >= kMaximumSessions ||
            DeviceHasSession(requester->claims.deviceId) ||
            DeviceHasSession(targetDeviceId)) {
            SendError(requester->socket, {}, QStringLiteral("device_busy"),
                      QStringLiteral("One of the devices already has a session."));
            return;
        }

        SessionState session;
        session.sessionId =
            QUuid::createUuid().toString(QUuid::WithoutBraces);
        session.requesterDeviceId = requester->claims.deviceId;
        session.targetDeviceId = targetDeviceId;
        session.purpose = purpose;
        session.permissions = permissions;
        session.pendingExpiresAtMs =
            QDateTime::currentMSecsSinceEpoch() +
            config_.pendingSessionTimeoutMs;
        const QString createdSessionId = session.sessionId;
        if (!InsertSession(std::move(session))) {
            SendError(requester->socket, {}, QStringLiteral("device_busy"),
                      QStringLiteral(
                          "One of the devices already has a session."));
            return;
        }

        QJsonObject pending;
        pending.insert(QStringLiteral("peerDeviceId"), targetDeviceId);
        SendEnvelope(requester->socket, QStringLiteral("session_pending"),
                     createdSessionId, pending);

        QJsonObject forwarded;
        forwarded.insert(QStringLiteral("requesterDeviceId"),
                         requester->claims.deviceId);
        forwarded.insert(QStringLiteral("purpose"), purpose);
        forwarded.insert(QStringLiteral("permissions"), permissions);
        // This privilege is derived only from authenticated server-side user
        // IDs. A client request can never assert or override it.
        forwarded.insert(QStringLiteral("sameAccount"), sameAccount);
        forwarded.insert(QStringLiteral("autoAccept"), autoAccept);
        forwarded.insert(QStringLiteral("assistedRequest"),
                         assistedRequest);
        if (assistedRequest) {
            forwarded.insert(QStringLiteral("verificationCode"),
                             verificationCode);
        }
        SendEnvelope(targetSocket, QStringLiteral("session_request"),
                     createdSessionId, forwarded);
    }

    void HandleSessionResponse(ClientState* target,
                               const QString& sessionId,
                               const QJsonObject& payload)
    {
        auto sessionIt = sessions_.find(sessionId);
        if (sessionIt == sessions_.end() ||
            sessionIt->phase != SessionState::Phase::kPending ||
            sessionIt->targetDeviceId != target->claims.deviceId ||
            !payload.value(QStringLiteral("accepted")).isBool()) {
            SendError(target->socket, sessionId,
                      QStringLiteral("invalid_session_response"),
                      QStringLiteral("The pending session response is invalid."));
            return;
        }

        const bool accepted =
            payload.value(QStringLiteral("accepted")).toBool();
        const QString reasonCode =
            payload.value(QStringLiteral("reasonCode")).toString();
        const QString reasonMessage =
            payload.value(QStringLiteral("reasonMessage")).toString();
        QWebSocket* requesterSocket =
            devices_.value(sessionIt->requesterDeviceId, nullptr);
        if (!requesterSocket) {
            SendError(target->socket, sessionId,
                      QStringLiteral("requester_offline"),
                      QStringLiteral("The requesting device is offline."));
            RemoveSession(sessionId);
            return;
        }

        QJsonObject response;
        response.insert(QStringLiteral("accepted"), accepted);
        if (!reasonCode.isEmpty()) {
            response.insert(QStringLiteral("reasonCode"), reasonCode.left(64));
        }
        if (!reasonMessage.isEmpty()) {
            response.insert(QStringLiteral("reasonMessage"),
                            reasonMessage.left(256));
        }
        SendEnvelope(requesterSocket, QStringLiteral("session_response"),
                     sessionId, response);

        if (!accepted) {
            RemoveSession(sessionId);
            return;
        }

        sessionIt->phase = SessionState::Phase::kActive;
        sessionIt->pendingExpiresAtMs = 0;
        sessionIt->requesterRecoveryToken = CreateRecoveryToken();
        sessionIt->targetRecoveryToken = CreateRecoveryToken();
        SendSessionReady(requesterSocket, sessionId,
                         sessionIt->targetDeviceId,
                         sessionIt->requesterRecoveryToken);
        SendSessionReady(target->socket, sessionId,
                         sessionIt->requesterDeviceId,
                         sessionIt->targetRecoveryToken);
    }

    void SendSessionReady(QWebSocket* socket,
                          const QString& sessionId,
                          const QString& peerDeviceId,
                          const QString& recoveryToken)
    {
        QJsonObject payload;
        payload.insert(QStringLiteral("peerDeviceId"), peerDeviceId);
        payload.insert(QStringLiteral("recoveryToken"), recoveryToken);
        payload.insert(QStringLiteral("iceServers"), SerializeIceServers());
        SendEnvelope(socket, QStringLiteral("session_ready"), sessionId,
                     payload);
    }

    void HandleSessionResume(ClientState* client,
                             const QString& sessionId,
                             const QJsonObject& payload)
    {
        const QString recoveryToken =
            payload.value(QStringLiteral("recoveryToken")).toString();
        auto sessionIt = sessions_.find(sessionId);
        if (sessionId.isEmpty() || recoveryToken.isEmpty() ||
            recoveryToken.size() > 256 || sessionIt == sessions_.end() ||
            sessionIt->phase != SessionState::Phase::kActive) {
            SendError(client->socket, sessionId,
                      QStringLiteral("session_resume_rejected"),
                      QStringLiteral("The active session cannot be resumed."));
            return;
        }

        qint64* disconnectedAtMs = nullptr;
        QString expectedToken;
        QString peerDeviceId;
        if (sessionIt->requesterDeviceId == client->claims.deviceId) {
            disconnectedAtMs = &sessionIt->requesterDisconnectedAtMs;
            expectedToken = sessionIt->requesterRecoveryToken;
            peerDeviceId = sessionIt->targetDeviceId;
        } else if (sessionIt->targetDeviceId == client->claims.deviceId) {
            disconnectedAtMs = &sessionIt->targetDisconnectedAtMs;
            expectedToken = sessionIt->targetRecoveryToken;
            peerDeviceId = sessionIt->requesterDeviceId;
        } else {
            SendError(client->socket, sessionId,
                      QStringLiteral("session_resume_rejected"),
                      QStringLiteral("The active session cannot be resumed."));
            return;
        }

        const qint64 now = QDateTime::currentMSecsSinceEpoch();
        if (recoveryToken != expectedToken ||
            (*disconnectedAtMs > 0 &&
             now - *disconnectedAtMs >
                 config_.sessionRecoveryWindowMs)) {
            SendError(client->socket, sessionId,
                      QStringLiteral("session_resume_rejected"),
                      QStringLiteral("The active session cannot be resumed."));
            return;
        }

        *disconnectedAtMs = 0;
        SendSessionResumed(client->socket, sessionId, peerDeviceId,
                           client->claims.deviceId);
        if (QWebSocket* peerSocket =
                devices_.value(peerDeviceId, nullptr)) {
            SendSessionResumed(peerSocket, sessionId,
                               client->claims.deviceId,
                               client->claims.deviceId);
        }
    }

    void SendSessionResumed(QWebSocket* socket,
                            const QString& sessionId,
                            const QString& peerDeviceId,
                            const QString& resumedDeviceId)
    {
        QJsonObject payload;
        payload.insert(QStringLiteral("peerDeviceId"), peerDeviceId);
        payload.insert(QStringLiteral("resumedDeviceId"), resumedDeviceId);
        SendEnvelope(socket, QStringLiteral("session_resumed"), sessionId,
                     payload);
    }
