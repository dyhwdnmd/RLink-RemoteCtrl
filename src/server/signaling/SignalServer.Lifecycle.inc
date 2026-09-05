// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

    explicit Impl(SignalServerConfig config)
        : config_(std::move(config))
        , tokenService_(config_.tokenSecret)
        , authenticationIpRateLimiter_(SlidingWindowRateLimitPolicy{
              config_.authenticationIpLimitPerMinute,
              kRateLimitWindowMs,
              kRateLimitMaximumKeys})
        , availabilityIpRateLimiter_(kAvailabilityIpRateLimit)
        , availabilityUserRateLimiter_(kAvailabilityUserRateLimit)
        , availabilityDeviceRateLimiter_(kAvailabilityDeviceRateLimit)
        , roomJoinIpRateLimiter_(kRoomJoinIpRateLimit)
        , roomJoinUserRateLimiter_(kRoomJoinUserRateLimit)
        , roomJoinDeviceRateLimiter_(kRoomJoinDeviceRateLimit)
        , server_(QStringLiteral("RemoteC Signaling"),
                  QWebSocketServer::SecureMode)
    {
        clients_.reserve(static_cast<std::size_t>(kInitialStateReserve));
        devices_.reserve(kInitialStateReserve);
        connectionCountByIp_.reserve(kInitialStateReserve);
        accountConnections_.reserve(kInitialStateReserve);
        sessions_.reserve(kInitialStateReserve);
        deviceSessions_.reserve(kInitialStateReserve);
        rooms_.reserve(kInitialStateReserve);
        deviceRooms_.reserve(kInitialStateReserve);
        pairRooms_.reserve(kInitialStateReserve);
        roomJoinRequests_.reserve(kInitialStateReserve);
        pendingRoomJoinRequestByDevice_.reserve(kInitialStateReserve);
        server_.setMaxPendingConnections(config_.maximumPendingConnections);
        server_.setHandshakeTimeout(10000);
        QObject::connect(&server_, &QWebSocketServer::newConnection,
                         &server_, [this] { AcceptConnections(); });
        maintenanceTimer_.setSingleShot(false);
        QObject::connect(&maintenanceTimer_, &QTimer::timeout, &server_,
                         [this] { SweepIdleClients(); });
        diagnosticProbeTimer_.setSingleShot(false);
        diagnosticsTimer_.setSingleShot(false);
        QObject::connect(&diagnosticProbeTimer_, &QTimer::timeout, &server_,
                         [this] { ProbeEventLoopLag(); });
        QObject::connect(&diagnosticsTimer_, &QTimer::timeout, &server_,
                         [this] { WriteDiagnostics(); });
    }

    bool Start(QString* error)
    {
        if (server_.isListening()) {
            return true;
        }
        if (!QSslSocket::supportsSsl()) {
            SetError(error, QStringLiteral(
                                "The Qt runtime has no TLS backend."));
            return false;
        }
        const bool usesLogto =
            config_.authenticationMode ==
            SignalServerAuthenticationMode::kLogtoUserInfo;
        if (!usesLogto && !tokenService_.IsConfigured()) {
            SetError(error, QStringLiteral(
                                "The token secret must contain at least 32 bytes."));
            return false;
        }
        if (config_.maximumRoomMembers < 2 ||
            config_.maximumRoomMembers > kProtocolMaximumRoomMembers) {
            SetError(error, QStringLiteral(
                                "The maximum room member count must be between 2 and 5."));
            return false;
        }
        if (config_.sessionRecoveryWindowMs < 1000) {
            SetError(error, QStringLiteral(
                                "The session recovery window is invalid."));
            return false;
        }
        if (config_.pendingSessionTimeoutMs < 1000 ||
            config_.pendingSessionTimeoutMs > 120000) {
            SetError(error, QStringLiteral(
                                "The pending session timeout is invalid."));
            return false;
        }
        if (config_.authenticationTimeoutMs < 1000 ||
            config_.authenticationTimeoutMs > 60000) {
            SetError(error, QStringLiteral(
                                "The authentication timeout is invalid."));
            return false;
        }
        if (config_.authenticationIpLimitPerMinute < 1 ||
            config_.authenticationIpLimitPerMinute > 100000) {
            SetError(error, QStringLiteral(
                                "The authentication IP limit must be between 1 and 100000 per minute."));
            return false;
        }
        if (config_.maximumConnections < 1 ||
            config_.maximumConnections > 100000) {
            SetError(error, QStringLiteral(
                                "The connection limit must be between 1 and 100000."));
            return false;
        }
        if (config_.maximumUnauthenticatedConnections < 1 ||
            config_.maximumUnauthenticatedConnections >
                config_.maximumConnections) {
            SetError(error, QStringLiteral(
                                "The unauthenticated connection limit must be between 1 and the total connection limit."));
            return false;
        }
        if (config_.maximumConnectionsPerIp < 0 ||
            config_.maximumConnectionsPerIp >
                config_.maximumConnections) {
            SetError(error, QStringLiteral(
                                "The per-IP connection limit must be zero or no greater than the total connection limit."));
            return false;
        }
        if (config_.diagnosticsIntervalMs < 0 ||
            config_.diagnosticsIntervalMs > 3600 * 1000) {
            SetError(error, QStringLiteral(
                                "The diagnostics interval must be between 0 and 3600 seconds."));
            return false;
        }
        if (usesLogto) {
            QString authError;
            if (!userInfoClient_.Configure(
                    config_.logtoIssuer, config_.userInfoTimeoutMs,
                    &authError)) {
                SetError(error, authError);
                return false;
            }
            if (!identityStore_.Open(
                    config_.identityDatabaseFile, &authError)) {
                SetError(error, authError);
                return false;
            }
            if (!config_.logtoManagementClientId.isEmpty() ||
                !config_.logtoManagementClientSecret.isEmpty()) {
                if (!managementClient_.Configure(
                        config_.logtoIssuer,
                        config_.logtoManagementClientId,
                        config_.logtoManagementClientSecret,
                        config_.managementTimeoutMs,
                        &authError)) {
                    identityStore_.Close();
                    SetError(error, authError);
                    return false;
                }
                config_.logtoManagementClientSecret.fill('\0');
                config_.logtoManagementClientSecret.clear();
            }
            if (config_.webhookPort != 0) {
                if (!webhookServer_.Start(
                        config_.webhookListenAddress,
                        config_.webhookPort,
                        config_.webhookSigningKey,
                        [this](const QString& subject, QString* deleteError) {
                            return DeleteLocalAccount(subject, deleteError);
                        },
                        &authError)) {
                    identityStore_.Close();
                    SetError(error, authError);
                    return false;
                }
                config_.webhookSigningKey.fill('\0');
                config_.webhookSigningKey.clear();
            }
        }

        QByteArray certificateData;
        QByteArray keyData;
        if (!ReadFile(config_.certificateFile, &certificateData, error) ||
            !ReadFile(config_.privateKeyFile, &keyData, error)) {
            return false;
        }
        const auto certificates =
            QSslCertificate::fromData(certificateData, QSsl::Pem);
        if (certificates.isEmpty()) {
            SetError(error, QStringLiteral("The TLS certificate is invalid."));
            return false;
        }
        const QSslKey privateKey(keyData, QSsl::Rsa, QSsl::Pem);
        if (privateKey.isNull()) {
            SetError(error, QStringLiteral(
                                "The TLS private key is invalid or not RSA PEM."));
            return false;
        }

        QSslConfiguration tls = QSslConfiguration::defaultConfiguration();
        tls.setProtocol(QSsl::TlsV1_2OrLater);
        tls.setLocalCertificateChain(certificates);
        tls.setPrivateKey(privateKey);
        tls.setPeerVerifyMode(QSslSocket::VerifyNone);
        server_.setSslConfiguration(tls);

        if (!server_.listen(config_.listenAddress, config_.port)) {
            SetError(error, server_.errorString());
            webhookServer_.Stop();
            identityStore_.Close();
            return false;
        }
        if (config_.diagnosticsIntervalMs > 0 &&
            !config_.diagnosticsLogFile.isEmpty()) {
            diagnosticsFile_.setFileName(config_.diagnosticsLogFile);
            if (!diagnosticsFile_.open(
                    QIODevice::WriteOnly | QIODevice::Append |
                    QIODevice::Text)) {
                SetError(error, QStringLiteral(
                                    "Cannot open the diagnostics log file: %1")
                                    .arg(diagnosticsFile_.errorString()));
                server_.close();
                webhookServer_.Stop();
                identityStore_.Close();
                return false;
            }
        }
        if (config_.clientIdleTimeoutMs > 0 ||
            config_.sessionRecoveryWindowMs > 0 ||
            config_.pendingSessionTimeoutMs > 0) {
            int maintenanceBaseMs = std::min(
                config_.sessionRecoveryWindowMs,
                kRoomJoinRequestTimeoutMs);
            maintenanceBaseMs = std::min(
                maintenanceBaseMs, config_.pendingSessionTimeoutMs);
            if (config_.clientIdleTimeoutMs > 0) {
                maintenanceBaseMs = std::min(
                    maintenanceBaseMs, config_.clientIdleTimeoutMs);
            }
            maintenanceBaseMs = std::min(
                maintenanceBaseMs, config_.authenticationTimeoutMs);
            maintenanceTimer_.start(
                std::max(1000, maintenanceBaseMs / 3));
        }
        if (config_.diagnosticsIntervalMs > 0) {
            diagnosticsClock_.start();
            diagnosticProbeExpectedMs_ = 1000;
            maximumEventLoopLagMs_ = 0;
            diagnosticProbeTimer_.start(1000);
            diagnosticsTimer_.start(config_.diagnosticsIntervalMs);
            QTimer::singleShot(0, &server_, [this] { WriteDiagnostics(); });
        }
        return true;
    }

    void Stop()
    {
        userInfoClient_.CancelAll();
        managementClient_.CancelAll();
        webhookServer_.Stop();
        server_.close();
        maintenanceTimer_.stop();
        diagnosticProbeTimer_.stop();
        diagnosticsTimer_.stop();
        diagnosticsFile_.close();
        for (auto& [socket, state] : clients_) {
            Q_UNUSED(state);
            socket->close(QWebSocketProtocol::CloseCodeGoingAway,
                          QStringLiteral("server shutdown"));
        }
        clients_.clear();
        unauthenticatedConnectionCount_ = 0;
        connectionCountByIp_.clear();
        totalRememberedMessageIds_ = 0;
        diagnosticProbeExpectedMs_ = 0;
        maximumEventLoopLagMs_ = 0;
        lastDiagnosticsAtMs_ = 0;
        lastReceivedMessagesTotal_ = 0;
        lastReceivedBytesTotal_ = 0;
        lastSentMessagesTotal_ = 0;
        lastSentBytesTotal_ = 0;
        acceptedConnectionsTotal_ = 0;
        rejectedConnectionsTotal_ = 0;
        receivedMessagesTotal_ = 0;
        receivedBytesTotal_ = 0;
        sentMessagesTotal_ = 0;
        sentBytesTotal_ = 0;
        devices_.clear();
        accountConnections_.clear();
        ClearSessions();
        rooms_.clear();
        deviceRooms_.clear();
        pairRooms_.clear();
        roomJoinRequests_.clear();
        pendingRoomJoinRequestByDevice_.clear();
        ClearRateLimiters();
        identityStore_.Close();
    }

    bool IsListening() const
    {
        return server_.isListening();
    }

    quint16 ServerPort() const
    {
        return server_.serverPort();
    }

    quint16 WebhookPort() const
    {
        return webhookServer_.ServerPort();
    }

private:
    struct ClientState {
        QWebSocket* socket = nullptr;
        AccessTokenClaims claims;
        bool authenticated = false;
        bool authenticationPending = false;
        bool accountDeletionPending = false;
        bool messageAuthenticated = false;
        bool registered = false;
        QPointer<QNetworkReply> userInfoReply;
        qint64 trustedUserId = 0;
        QString logtoSubject;
        QString peerKey;
        QString deviceName;
        QSet<QByteArray> receivedMessageIds;
        QQueue<QByteArray> receivedMessageOrder;
        qint64 lastActivityMs = 0;
        qint64 authenticationDeadlineMs = 0;
    };

    struct SessionState {
        enum class Phase {
            kPending,
            kActive,
        };

        QString sessionId;
        QString requesterDeviceId;
        QString targetDeviceId;
        QString purpose;
        QJsonArray permissions;
        Phase phase = Phase::kPending;
        QString requesterRecoveryToken;
        QString targetRecoveryToken;
        qint64 requesterDisconnectedAtMs = 0;
        qint64 targetDisconnectedAtMs = 0;
        qint64 pendingExpiresAtMs = 0;
    };

    struct RoomMemberState {
        QString deviceId;
        QString deviceName;
        QString recoveryToken;
        qint64 disconnectedAtMs = 0;
        bool cameraPublishing = false;
        bool microphonePublishing = false;
    };

    struct RoomPairState {
        QString pairId;
        QString firstDeviceId;
        QString secondDeviceId;
        QString offererDeviceId;
    };

    struct RoomState {
        QString roomId;
        QString ownerDeviceId;
        int capacity = 2;
        QString screenShareState = QStringLiteral("idle");
        QString screenSharerDeviceId;
        QString pendingScreenSharerDeviceId;
        quint64 screenShareEpoch = 0;
        QString screenShareGrantId;
        QString screenShareSwitchRequestId;
        QString screenShareSwitchRequesterDeviceId;
        qint64 screenShareSwitchRequestExpiresAtMs = 0;
        QString pendingControllerDeviceId;
        QString controlRequestId;
        qint64 controlRequestExpiresAtMs = 0;
        QString activeControllerDeviceId;
        QString controlGrantId;
        QHash<QString, RoomMemberState> members;
        QHash<QString, RoomPairState> pairs;
    };

    struct RoomJoinRequestState {
        QString requestId;
        QString roomId;
        QString requesterDeviceId;
        QString requesterDeviceName;
        qint64 expiresAtMs = 0;
    };
