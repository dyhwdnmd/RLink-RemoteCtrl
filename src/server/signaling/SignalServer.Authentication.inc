// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

    void HandleMessageAuthentication(
        ClientState* client,
        const QJsonObject& payload)
    {
        if (!client || !client->socket) {
            return;
        }
        const bool usesMessageHmac =
            config_.authenticationMode ==
            SignalServerAuthenticationMode::
                kLegacyUpgradeBearerWithMessageHmacTestOnly;
        const bool usesLogto =
            config_.authenticationMode ==
            SignalServerAuthenticationMode::kLogtoUserInfo;
        if (!usesMessageHmac && !usesLogto) {
            SendAuthenticationError(
                client->socket,
                QStringLiteral("message_authentication_disabled"),
                QStringLiteral(
                    "Message authentication is not enabled on this server."),
                false);
            client->socket->close(
                QWebSocketProtocol::CloseCodePolicyViolated,
                QStringLiteral("message authentication disabled"));
            return;
        }
        const qint64 now = QDateTime::currentMSecsSinceEpoch();
        if (!AllowAuthenticationAttempt(client->socket, now)) {
            SendAuthenticationError(
                client->socket, QStringLiteral("rate_limited"),
                QStringLiteral(
                    "Too many authentication attempts. Wait before trying again."),
                false);
            client->socket->close(
                QWebSocketProtocol::CloseCodePolicyViolated,
                QStringLiteral("authentication rate limited"));
            return;
        }
        if (client->authenticated || client->authenticationPending) {
            SendAuthenticationError(
                client->socket, QStringLiteral("already_authenticated"),
                QStringLiteral(
                    "Authentication is already complete or in progress."),
                false);
            client->socket->close(
                QWebSocketProtocol::CloseCodePolicyViolated,
                QStringLiteral("authentication already started"));
            return;
        }
        if (payload.size() != 1 ||
            !payload.value(QStringLiteral("accessToken")).isString()) {
            SendAuthenticationError(
                client->socket, QStringLiteral("invalid_auth_payload"),
                QStringLiteral(
                    "The authentication payload must contain only an Access Token."),
                false);
            client->socket->close(
                QWebSocketProtocol::CloseCodePolicyViolated,
                QStringLiteral("invalid authentication payload"));
            return;
        }
        const QByteArray accessToken = payload.value(
            QStringLiteral("accessToken")).toString().toUtf8();
        if (accessToken.isEmpty() || accessToken.size() > 16 * 1024) {
            SendAuthenticationError(
                client->socket, QStringLiteral("invalid_token"),
                QStringLiteral("The Access Token is missing or too large."),
                false);
            client->socket->close(
                QWebSocketProtocol::CloseCodePolicyViolated,
                QStringLiteral("authentication rejected"));
            return;
        }

        if (usesMessageHmac) {
            AccessTokenClaims claims;
            QString tokenError;
            if (!tokenService_.Verify(accessToken, &claims, &tokenError)) {
                SendAuthenticationError(
                    client->socket, QStringLiteral("invalid_token"),
                    QStringLiteral("The Access Token was rejected."), false);
                client->socket->close(
                    QWebSocketProtocol::CloseCodePolicyViolated,
                    QStringLiteral("authentication rejected"));
                return;
            }
            client->claims = std::move(claims);
            CompleteClientAuthentication(client);
            client->messageAuthenticated = true;
            SendEnvelope(client->socket, QStringLiteral("auth_success"), {}, {});
            return;
        }

        client->authenticationPending = true;
        const QPointer<QWebSocket> guardedSocket(client->socket);
        client->userInfoReply = userInfoClient_.Fetch(
            accessToken,
            [this, guardedSocket](remote::server_auth::UserInfoResult result) {
                if (!guardedSocket) {
                    return;
                }
                ClientState* current = FindClient(guardedSocket.data());
                if (!current) {
                    return;
                }
                current->authenticationPending = false;
                current->userInfoReply = nullptr;
                if (!result.authenticated()) {
                    SendAuthenticationError(
                        current->socket, result.code, result.message,
                        result.retryable);
                    current->socket->close(
                        result.retryable
                            ? QWebSocketProtocol::CloseCodeNormal
                            : QWebSocketProtocol::CloseCodePolicyViolated,
                        result.retryable
                            ? QStringLiteral("authentication service unavailable")
                            : QStringLiteral("authentication rejected"));
                    return;
                }

                QString storeError;
                const auto user = identityStore_.UpsertUser(
                    result.claims, &storeError);
                if (!user.isValid()) {
                    SendAuthenticationError(
                        current->socket,
                        QStringLiteral("identity_store_unavailable"),
                        QStringLiteral(
                            "The RemoteC identity store is unavailable."),
                        true);
                    current->socket->close(
                        QWebSocketProtocol::CloseCodeNormal,
                        QStringLiteral("identity store unavailable"));
                    return;
                }

                current->trustedUserId = user.id;
                current->logtoSubject = user.logtoSubject;
                CompleteClientAuthentication(current);
                current->messageAuthenticated = true;
                AddAccountConnection(current);
                SendEnvelope(current->socket,
                             QStringLiteral("auth_success"), {}, {});
            });
        if (!client->userInfoReply) {
            client->authenticationPending = false;
            SendAuthenticationError(
                client->socket,
                QStringLiteral("authentication_service_unavailable"),
                QStringLiteral("Logto UserInfo could not be requested."), true);
            client->socket->close(
                QWebSocketProtocol::CloseCodeNormal,
                QStringLiteral("authentication service unavailable"));
        }
    }

    void SendAuthenticationError(
        QWebSocket* socket,
        const QString& reason,
        const QString& message,
        bool retryable)
    {
        QJsonObject payload;
        payload.insert(QStringLiteral("reason"), reason);
        payload.insert(QStringLiteral("message"), message);
        payload.insert(QStringLiteral("retryable"), retryable);
        SendEnvelope(socket, QStringLiteral("auth_error"), {}, payload);
    }
