// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

    void SendHeartbeat()
    {
        if (State() != SignalingConnectionState::kRegistered ||
            heartbeatOutstanding_) {
            return;
        }
        QJsonObject payload;
        payload.insert(QStringLiteral("sentAtMs"),
                       QDateTime::currentMSecsSinceEpoch());
        const auto result =
            SendEnvelope(QStringLiteral("heartbeat"), {}, payload);
        if (!result.accepted) {
            Fail(result.errorCode, result.errorMessage);
            socket_.abort();
            return;
        }
        heartbeatOutstanding_ = true;
        heartbeatRoundTrip_.restart();
        heartbeatDeadline_.start(
            static_cast<int>(config_.heartbeatTimeoutMs));
    }

    void OpenConfiguredSocket()
    {
        if (manualDisconnect_) {
            return;
        }
        fatalDisconnect_ = false;
        SetState(SignalingConnectionState::kConnecting);
        QNetworkRequest request(endpoint_);
        if (config_.authenticationMode ==
            SignalingAuthenticationMode::kLegacyUpgradeBearer) {
            request.setRawHeader(
                "Authorization",
                QByteArray("Bearer ") +
                    ToQString(config_.accessToken).toUtf8());
        }
        request.setRawHeader("X-RemoteC-Protocol",
                             QByteArray::number(kProtocolVersion));
        socket_.open(request);
    }

    void ScheduleReconnect()
    {
        if (manualDisconnect_ || fatalDisconnect_ ||
            config_.reconnectAttemptLimit == 0) {
            if (!manualDisconnect_) {
                SetState(SignalingConnectionState::kFailed);
            }
            return;
        }
        if (reconnectAttempt_ >= config_.reconnectAttemptLimit) {
            Fail("signaling_reconnect_exhausted",
                 "The signaling reconnect attempt limit was reached.");
            return;
        }
        std::uint64_t delay = config_.reconnectInitialDelayMs;
        for (std::uint32_t index = 0;
             index < reconnectAttempt_ &&
             delay < config_.reconnectMaximumDelayMs;
             ++index) {
            delay = std::min<std::uint64_t>(
                delay * 2, config_.reconnectMaximumDelayMs);
        }
        ++reconnectAttempt_;
        SetState(SignalingConnectionState::kConnecting);
        reconnectTimer_.start(static_cast<int>(delay));
    }
