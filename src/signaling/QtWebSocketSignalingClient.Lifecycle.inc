// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

    Impl()
    {
        socket_.setMaxAllowedIncomingFrameSize(kMaximumMessageBytes);
        socket_.setMaxAllowedIncomingMessageSize(kMaximumMessageBytes);

        QObject::connect(&socket_, &QWebSocket::connected, &socket_,
                         [this] { OnConnected(); });
        QObject::connect(&socket_, &QWebSocket::disconnected, &socket_,
                         [this] { OnDisconnected(); });
        QObject::connect(&socket_, &QWebSocket::textMessageReceived, &socket_,
                         [this](const QString& message) {
                             OnTextMessage(message);
                         });
        QObject::connect(&socket_, &QWebSocket::errorOccurred, &socket_,
                         [this](QAbstractSocket::SocketError) {
                             if (State() !=
                                 SignalingConnectionState::kFailed) {
                                 Fail("websocket_error",
                                      ToString(socket_.errorString()));
                             }
                         });
        QObject::connect(
            &socket_, &QWebSocket::sslErrors, &socket_,
            [this](const QList<QSslError>& errors) {
                QStringList descriptions;
                for (const auto& error : errors) {
                    descriptions.push_back(error.errorString());
                }
                // Certificate validation must never be silently downgraded.
                fatalDisconnect_ = true;
                Fail("tls_validation_failed",
                     ToString(descriptions.join(QStringLiteral("; "))));
                socket_.abort();
            });
        heartbeatTimer_.setSingleShot(false);
        QObject::connect(&heartbeatTimer_, &QTimer::timeout, &socket_,
                         [this] { SendHeartbeat(); });
        heartbeatDeadline_.setSingleShot(true);
        QObject::connect(&heartbeatDeadline_, &QTimer::timeout, &socket_,
                         [this] {
                             if (!heartbeatOutstanding_) {
                                 return;
                             }
                             heartbeatOutstanding_ = false;
                             Fail("heartbeat_timeout",
                                  "The signaling server did not acknowledge the heartbeat.");
                             socket_.abort();
                         });
        authenticationDeadline_.setSingleShot(true);
        QObject::connect(
            &authenticationDeadline_, &QTimer::timeout, &socket_,
            [this] {
                if (State() !=
                    SignalingConnectionState::kAuthenticating) {
                    return;
                }
                fatalDisconnect_ = true;
                Fail("signaling_authentication_timeout",
                     "The signaling server did not complete authentication in time.");
                socket_.close(
                    QWebSocketProtocol::CloseCodePolicyViolated,
                    QStringLiteral("authentication timeout"));
            });
        reconnectTimer_.setSingleShot(true);
        QObject::connect(&reconnectTimer_, &QTimer::timeout, &socket_,
                         [this] { OpenConfiguredSocket(); });
    }

    ~Impl()
    {
        manualDisconnect_ = true;
        authenticationDeadline_.stop();
        reconnectTimer_.stop();
        observer_ = nullptr;
        socket_.abort();
    }

    void SetObserver(ISignalingClientObserver* observer)
    {
        observer_ = observer;
        if (observer_) {
            observer_->OnSignalingStateChanged(State());
        }
    }

    SignalingOperationResult Connect(const SignalingClientConfig& config)
    {
        const auto state = State();
        if (state != SignalingConnectionState::kDisconnected &&
            state != SignalingConnectionState::kFailed) {
            return Failure("signaling_already_started",
                           "The signaling client is already active.");
        }

        const QUrl endpoint(ToQString(config.endpoint));
        if (!endpoint.isValid() || endpoint.scheme() != QStringLiteral("wss") ||
            endpoint.host().isEmpty()) {
            return Failure("wss_endpoint_required",
                           "A valid wss:// signaling endpoint is required.");
        }
        if (ShouldBypassProxy(endpoint)) {
            socket_.setProxy(QNetworkProxy(QNetworkProxy::NoProxy));
        } else {
            socket_.setProxy(QNetworkProxy(QNetworkProxy::DefaultProxy));
        }
        if (config.accessToken.empty() ||
            config.accessToken.size() > kMaximumAccessTokenBytes) {
            return Failure("access_token_required",
                           "A signaling access token of a supported size is required.");
        }
        if (config.deviceName.empty()) {
            return Failure("device_name_required",
                           "A local device name is required.");
        }
        if (config.heartbeatIntervalMs < 100 ||
            config.heartbeatTimeoutMs < 100 ||
            config.authenticationTimeoutMs < 1000 ||
            config.heartbeatIntervalMs >
                static_cast<std::uint32_t>(
                    std::numeric_limits<int>::max()) ||
            config.heartbeatTimeoutMs >
                static_cast<std::uint32_t>(
                    std::numeric_limits<int>::max()) ||
            config.authenticationTimeoutMs >
                static_cast<std::uint32_t>(
                    std::numeric_limits<int>::max())) {
            return Failure("invalid_heartbeat_configuration",
                           "Authentication and heartbeat timeouts must fit the supported range.");
        }
        if (config.reconnectAttemptLimit > 100 ||
            (config.reconnectAttemptLimit > 0 &&
             (config.reconnectInitialDelayMs < 100 ||
              config.reconnectMaximumDelayMs <
                  config.reconnectInitialDelayMs ||
              config.reconnectMaximumDelayMs >
                  static_cast<std::uint32_t>(
                      std::numeric_limits<int>::max())))) {
            return Failure("invalid_reconnect_configuration",
                           "Reconnect delays and attempt limit are invalid.");
        }

        QSslConfiguration tls = QSslConfiguration::defaultConfiguration();
        if (!config.trustedCaPem.empty()) {
            const auto certificates = QSslCertificate::fromData(
                QByteArray(config.trustedCaPem.data(),
                           static_cast<qsizetype>(
                               config.trustedCaPem.size())),
                QSsl::Pem);
            if (certificates.isEmpty()) {
                return Failure("trusted_ca_invalid",
                               "The configured signaling CA PEM is invalid.");
            }
            auto authorities = tls.caCertificates();
            authorities.append(certificates);
            tls.setCaCertificates(authorities);
        }
        tls.setProtocol(QSsl::TlsV1_2OrLater);
        socket_.setSslConfiguration(tls);

        config_ = config;
        endpoint_ = endpoint;
        receivedMessageIds_.clear();
        receivedMessageOrder_.clear();
        heartbeatTimer_.stop();
        heartbeatDeadline_.stop();
        authenticationDeadline_.stop();
        reconnectTimer_.stop();
        heartbeatOutstanding_ = false;
        reconnectAttempt_ = 0;
        manualDisconnect_ = false;
        fatalDisconnect_ = false;
        SetState(SignalingConnectionState::kConnecting);
        OpenConfiguredSocket();
        return Success();
    }

    SignalingOperationResult UpdateAccessToken(
        const std::string& accessToken)
    {
        if (accessToken.empty() ||
            accessToken.size() > kMaximumAccessTokenBytes) {
            return Failure("access_token_required",
                           "A signaling access token of a supported size is required.");
        }
        config_.accessToken = accessToken;
        return Success();
    }

    SignalingOperationResult RequestAccountDeletion()
    {
        const auto ready = RequireRegistered();
        if (!ready.accepted) {
            return ready;
        }
        return SendEnvelope(
            QStringLiteral("account_delete_request"), {}, {});
    }

    void Disconnect()
    {
        manualDisconnect_ = true;
        authenticationDeadline_.stop();
        reconnectTimer_.stop();
        if (State() == SignalingConnectionState::kDisconnected &&
            socket_.state() == QAbstractSocket::UnconnectedState) {
            return;
        }
        heartbeatTimer_.stop();
        heartbeatDeadline_.stop();
        authenticationDeadline_.stop();
        heartbeatOutstanding_ = false;
        SetState(SignalingConnectionState::kClosing);
        if (socket_.state() == QAbstractSocket::UnconnectedState) {
            SetState(SignalingConnectionState::kDisconnected);
        } else {
            socket_.close(QWebSocketProtocol::CloseCodeNormal,
                          QStringLiteral("client shutdown"));
        }
    }

    SignalingConnectionState State() const
    {
        return state_.load(std::memory_order_acquire);
    }
