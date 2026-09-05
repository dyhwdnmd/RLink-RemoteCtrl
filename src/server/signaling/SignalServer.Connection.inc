// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

    void AcceptConnections()
    {
        while (server_.hasPendingConnections()) {
            QWebSocket* socket = server_.nextPendingConnection();
            if (!socket) {
                continue;
            }
            socket->setMaxAllowedIncomingFrameSize(kMaximumMessageBytes);
            socket->setMaxAllowedIncomingMessageSize(kMaximumMessageBytes);

            const QNetworkRequest request = socket->request();
            if (socket->requestUrl().path() != QStringLiteral("/signaling") ||
                request.rawHeader("X-RemoteC-Protocol") !=
                    QByteArray::number(kProtocolVersion)) {
                RejectConnection(socket, QStringLiteral("protocol mismatch"));
                continue;
            }

            const QString peerKey = PeerRateLimitKey(socket);
            if (static_cast<int>(clients_.size()) >=
                config_.maximumConnections) {
                RejectConnection(socket, QStringLiteral("server busy"));
                continue;
            }
            if (config_.maximumConnectionsPerIp > 0 &&
                connectionCountByIp_.value(peerKey) >=
                    config_.maximumConnectionsPerIp) {
                RejectConnection(socket,
                                 QStringLiteral("connection limit reached"));
                continue;
            }

            const QByteArray authorization =
                request.rawHeader("Authorization");
            constexpr QByteArrayView bearerPrefix("Bearer ");
            AccessTokenClaims claims;
            bool authenticated = false;
            const bool usesLogto =
                config_.authenticationMode ==
                SignalServerAuthenticationMode::kLogtoUserInfo;
            if (config_.authenticationMode ==
                    SignalServerAuthenticationMode::kLegacyUpgradeBearer &&
                !AllowAuthenticationAttempt(
                    socket, QDateTime::currentMSecsSinceEpoch())) {
                RejectConnection(
                    socket, QStringLiteral("authentication rate limited"));
                continue;
            }
            if (usesLogto && !authorization.isEmpty()) {
                RejectConnection(
                    socket,
                    QStringLiteral("send authentication after connecting"));
                continue;
            }
            if (!usesLogto && !authorization.isEmpty()) {
                if (!authorization.startsWith(bearerPrefix)) {
                    RejectConnection(
                        socket, QStringLiteral("authentication required"));
                    continue;
                }
                QString tokenError;
                if (!tokenService_.Verify(
                        authorization.mid(bearerPrefix.size()), &claims,
                        &tokenError)) {
                    RejectConnection(
                        socket, QStringLiteral("authentication failed"));
                    continue;
                }
                authenticated = true;
            } else if (!usesLogto && config_.authenticationMode !=
                       SignalServerAuthenticationMode::
                           kLegacyUpgradeBearerWithMessageHmacTestOnly) {
                RejectConnection(
                    socket, QStringLiteral("authentication required"));
                continue;
            }
            if (!authenticated &&
                unauthenticatedConnectionCount_ >=
                    config_.maximumUnauthenticatedConnections) {
                RejectConnection(
                    socket,
                    QStringLiteral("authentication capacity reached"));
                continue;
            }

            auto state = std::make_unique<ClientState>();
            state->socket = socket;
            state->peerKey = peerKey;
            state->claims = std::move(claims);
            state->authenticated = authenticated;
            state->lastActivityMs = QDateTime::currentMSecsSinceEpoch();
            if (!authenticated) {
                ++unauthenticatedConnectionCount_;
                state->authenticationDeadlineMs =
                    state->lastActivityMs + config_.authenticationTimeoutMs;
            }
            ++connectionCountByIp_[peerKey];
            clients_.emplace(socket, std::move(state));
            ++acceptedConnectionsTotal_;
            QObject::connect(socket, &QWebSocket::textMessageReceived,
                             socket, [this, socket](const QString& message) {
                                 OnTextMessage(socket, message);
                             });
            QObject::connect(socket, &QWebSocket::binaryMessageReceived,
                             socket, [socket](const QByteArray&) {
                                 socket->close(
                                     QWebSocketProtocol::CloseCodeDatatypeNotSupported,
                                     QStringLiteral("binary messages are unsupported"));
                             });
            QObject::connect(socket, &QWebSocket::disconnected, socket,
                             [this, socket] {
                                 OnDisconnected(socket);
                                 socket->deleteLater();
                             });
        }
    }

    void RejectConnection(QWebSocket* socket, const QString& reason)
    {
        ++rejectedConnectionsTotal_;
        QObject::connect(socket, &QWebSocket::disconnected, socket,
                         &QObject::deleteLater);
        socket->close(QWebSocketProtocol::CloseCodePolicyViolated, reason);
    }

    void OnTextMessage(QWebSocket* socket, const QString& message)
    {
        auto* client = FindClient(socket);
        if (!client) {
            return;
        }
        const QByteArray utf8 = message.toUtf8();
        ++receivedMessagesTotal_;
        receivedBytesTotal_ += utf8.size();
        if (static_cast<quint64>(utf8.size()) > kMaximumMessageBytes) {
            socket->close(QWebSocketProtocol::CloseCodeTooMuchData,
                          QStringLiteral("message too large"));
            return;
        }

        QJsonParseError parseError;
        const QJsonDocument document =
            QJsonDocument::fromJson(utf8, &parseError);
        if (parseError.error != QJsonParseError::NoError ||
            !document.isObject()) {
            SendError(socket, {}, QStringLiteral("invalid_json"),
                      QStringLiteral("The signaling message is not valid JSON."));
            return;
        }

        const QJsonObject envelope = document.object();
        const QString messageId =
            envelope.value(QStringLiteral("messageId")).toString();
        const QString type =
            envelope.value(QStringLiteral("type")).toString();
        if (envelope.value(QStringLiteral("protocolVersion")).toInt(-1) !=
                kProtocolVersion ||
            messageId.isEmpty() || messageId.size() > 128 ||
            type.isEmpty() || type.size() > 64 ||
            !envelope.value(QStringLiteral("payload")).isObject()) {
            SendError(socket, {}, QStringLiteral("invalid_envelope"),
                      QStringLiteral("The signaling envelope is invalid."));
            return;
        }
        if (!RememberMessageId(client, messageId.toUtf8())) {
            return;
        }
        client->lastActivityMs = QDateTime::currentMSecsSinceEpoch();

        const QString sessionId =
            envelope.value(QStringLiteral("sessionId")).toString();
        const QJsonObject payload =
            envelope.value(QStringLiteral("payload")).toObject();

        if (type == QStringLiteral("auth")) {
            HandleMessageAuthentication(client, payload);
            return;
        }
        if (client->authenticated && client->claims.expiresAtSeconds > 0 &&
            client->claims.expiresAtSeconds <=
                QDateTime::currentSecsSinceEpoch()) {
            if (client->messageAuthenticated) {
                SendAuthenticationError(
                    socket, QStringLiteral("access_token_expired"),
                    QStringLiteral(
                        "The signaling Access Token has expired; reconnect with the latest token."),
                    true);
                socket->close(
                    QWebSocketProtocol::CloseCodeNormal,
                    QStringLiteral("authentication refresh required"));
            } else {
                socket->close(
                    QWebSocketProtocol::CloseCodePolicyViolated,
                    QStringLiteral("access token expired"));
            }
            return;
        }
        if (type == QStringLiteral("register_device")) {
            if (!client->authenticated) {
                SendAuthenticationError(
                    socket, QStringLiteral("authentication_required"),
                    QStringLiteral(
                        "Authenticate before registering the device."),
                    false);
                socket->close(
                    QWebSocketProtocol::CloseCodePolicyViolated,
                    QStringLiteral("authentication required"));
                return;
            }
            HandleRegister(client, payload);
            return;
        }
        if (type == QStringLiteral("heartbeat")) {
            SendEnvelope(socket, QStringLiteral("heartbeat_ack"), {}, {});
            return;
        }
        if (!client->authenticated) {
            SendAuthenticationError(
                socket, QStringLiteral("authentication_required"),
                QStringLiteral(
                    "Authenticate before sending signaling messages."),
                false);
            socket->close(
                QWebSocketProtocol::CloseCodePolicyViolated,
                QStringLiteral("authentication required"));
            return;
        }
        if (type == QStringLiteral("account_delete_request")) {
            HandleAccountDelete(client);
            return;
        }
        if (!client->registered) {
            SendError(socket, sessionId, QStringLiteral("device_not_registered"),
                      QStringLiteral("Register the device before starting a session."));
            return;
        }

        if (type == QStringLiteral("my_devices_request")) {
            HandleMyDevicesRequest(client);
        } else if (type == QStringLiteral("room_create")) {
            HandleRoomCreate(client, payload);
        } else if (type == QStringLiteral("room_availability_query")) {
            HandleRoomAvailabilityQuery(client, payload);
        } else if (type == QStringLiteral("room_join_request")) {
            HandleRoomJoinRequest(client, payload);
        } else if (type == QStringLiteral("room_join_response")) {
            HandleRoomJoinResponse(client, payload);
        } else if (type == QStringLiteral("room_set_capacity")) {
            HandleRoomSetCapacity(client, payload);
        } else if (type == QStringLiteral("room_leave")) {
            HandleRoomLeave(client, payload);
        } else if (type == QStringLiteral("room_resume")) {
            HandleRoomResume(client, payload);
        } else if (type == QStringLiteral("room_media_state")) {
            HandleRoomMediaState(client, payload);
        } else if (type == QStringLiteral("room_screen_share_request")) {
            HandleRoomScreenShareRequest(client, payload);
        } else if (type == QStringLiteral("room_screen_share_ready")) {
            HandleRoomScreenShareReady(client, payload);
        } else if (type == QStringLiteral("room_screen_share_stop")) {
            HandleRoomScreenShareStop(client, payload);
        } else if (
            type == QStringLiteral("room_screen_share_switch_response")) {
            HandleRoomScreenShareSwitchResponse(client, payload);
        } else if (
            type == QStringLiteral("room_screen_share_switch_cancel")) {
            HandleRoomScreenShareSwitchCancel(client, payload);
        } else if (type == QStringLiteral("room_control_request")) {
            HandleRoomControlRequest(client, payload);
        } else if (type == QStringLiteral("room_control_response")) {
            HandleRoomControlResponse(client, payload);
        } else if (type == QStringLiteral("room_control_release")) {
            HandleRoomControlRelease(client, payload);
        } else if (type == QStringLiteral("session_request")) {
            HandleSessionRequest(client, payload);
        } else if (type == QStringLiteral("session_response")) {
            HandleSessionResponse(client, sessionId, payload);
        } else if (type == QStringLiteral("session_resume")) {
            HandleSessionResume(client, sessionId, payload);
        } else if (type == QStringLiteral("sdp") ||
                   type == QStringLiteral("ice_candidate") ||
                   type == QStringLiteral("ice_restart_request") ||
                   type == QStringLiteral("ice_restart_cancel")) {
            RelaySessionMessage(client, sessionId, type, payload);
        } else if (type == QStringLiteral("session_cancel") ||
                   type == QStringLiteral("session_close")) {
            CloseSession(client, sessionId, type, payload);
        } else {
            SendError(socket, sessionId, QStringLiteral("unknown_message_type"),
                      QStringLiteral("The signaling message type is unsupported."));
        }
    }
