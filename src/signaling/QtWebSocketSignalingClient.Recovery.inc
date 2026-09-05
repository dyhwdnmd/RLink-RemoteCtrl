// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

    void OnConnected()
    {
        if (config_.authenticationMode ==
            SignalingAuthenticationMode::kMessageAccessToken) {
            SetState(SignalingConnectionState::kAuthenticating);
            QJsonObject payload;
            payload.insert(QStringLiteral("accessToken"),
                           ToQString(config_.accessToken));
            const auto result = SendEnvelope(
                QStringLiteral("auth"), {}, payload);
            if (!result.accepted) {
                Fail("signaling_authentication_send_failed",
                     "Failed to send signaling authentication.");
                socket_.abort();
                return;
            }
            authenticationDeadline_.start(
                static_cast<int>(config_.authenticationTimeoutMs));
            return;
        }
        BeginDeviceRegistration();
    }

    void BeginDeviceRegistration()
    {
        authenticationDeadline_.stop();
        SetState(SignalingConnectionState::kRegistering);
        QJsonObject capabilities;
        capabilities.insert(QStringLiteral("screenCapture"),
                            config_.capabilities.screenCapture);
        capabilities.insert(QStringLiteral("cameraCapture"),
                            config_.capabilities.cameraCapture);
        capabilities.insert(QStringLiteral("inputControl"),
                            config_.capabilities.inputControl);
        capabilities.insert(QStringLiteral("h264Encode"),
                            config_.capabilities.h264Encode);
        capabilities.insert(QStringLiteral("h264Decode"),
                            config_.capabilities.h264Decode);
        capabilities.insert(QStringLiteral("d3d11NativeDecode"),
                            config_.capabilities.d3d11NativeDecode);

        QJsonObject payload;
        if (!config_.deviceId.empty()) {
            payload.insert(QStringLiteral("deviceId"),
                           ToQString(config_.deviceId));
        }
        payload.insert(QStringLiteral("deviceName"),
                       ToQString(config_.deviceName));
        payload.insert(QStringLiteral("appVersion"),
                       ToQString(config_.appVersion));
        payload.insert(QStringLiteral("capabilities"), capabilities);
        if (!SendEnvelope(QStringLiteral("register_device"), {}, payload)
                 .accepted) {
            Fail("device_registration_send_failed",
                 "Failed to send device registration.");
        }
    }

    void OnDisconnected()
    {
        heartbeatTimer_.stop();
        heartbeatDeadline_.stop();
        authenticationDeadline_.stop();
        heartbeatOutstanding_ = false;
        if (manualDisconnect_) {
            SetState(SignalingConnectionState::kDisconnected);
            return;
        }
        if (socket_.closeCode() ==
            QWebSocketProtocol::CloseCodePolicyViolated) {
            fatalDisconnect_ = true;
            Fail("signaling_policy_rejected",
                 ToString(socket_.closeReason()));
        } else {
            SetState(SignalingConnectionState::kDisconnected);
        }
        ScheduleReconnect();
    }

    void OnTextMessage(const QString& message)
    {
        const QByteArray utf8 = message.toUtf8();
        if (static_cast<quint64>(utf8.size()) > kMaximumMessageBytes) {
            Fail("signaling_message_too_large",
                 "The signaling message exceeds the configured limit.");
            socket_.abort();
            return;
        }

        QJsonParseError parseError;
        const QJsonDocument document =
            QJsonDocument::fromJson(utf8, &parseError);
        if (parseError.error != QJsonParseError::NoError ||
            !document.isObject()) {
            NotifyError("invalid_signaling_json",
                        ToString(parseError.errorString()));
            return;
        }

        const QJsonObject envelope = document.object();
        if (envelope.value(QStringLiteral("protocolVersion")).toInt(-1) !=
            kProtocolVersion) {
            NotifyError("unsupported_signaling_protocol",
                        "The signaling protocol version is unsupported.");
            return;
        }
        const QString messageId =
            envelope.value(QStringLiteral("messageId")).toString();
        const QString type = envelope.value(QStringLiteral("type")).toString();
        if (messageId.isEmpty() || type.isEmpty()) {
            NotifyError("invalid_signaling_envelope",
                        "Message ID and type are required.");
            return;
        }
        const std::string messageKey = ToString(messageId);
        if (!RememberMessageId(messageKey)) {
            return;
        }

        const std::string sessionId = ToString(
            envelope.value(QStringLiteral("sessionId")).toString());
        if (!envelope.value(QStringLiteral("payload")).isObject()) {
            NotifyError("invalid_signaling_payload",
                        "The signaling payload must be an object.");
            return;
        }
        const QJsonObject payload =
            envelope.value(QStringLiteral("payload")).toObject();
        DispatchMessage(type, sessionId, payload);
    }
