// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

LibWebRtcSession::LibWebRtcSession(
    webrtc::scoped_refptr<webrtc::PeerConnectionFactoryInterface> factory)
    : callbackGate_(std::make_shared<CallbackGate>(this)),
      factory_(std::move(factory)),
      statsCollector_(
          std::make_unique<PeerConnectionStatsCollector>())
{}

LibWebRtcSession::~LibWebRtcSession()
{
    callbackGate_->DetachAndWait();
    SetObserver(nullptr);
    Close();
}

void LibWebRtcSession::SetObserver(IWebRtcSessionObserver* observer)
{
    std::lock_guard lock(mutex_);
    observer_ = observer;
}

OperationId LibWebRtcSession::Start(const WebRtcSessionConfig& config)
{
    const OperationId operationId = NextOperationId();
    bool canStart = false;
    {
        std::lock_guard lock(mutex_);
        canStart = state_ == WebRtcSessionState::kNew && !peerConnection_;
        if (canStart) {
            fastDesktopBweStartup_ = config.fastDesktopBweStartup;
            adaptiveDesktopNetworkFrameRate_ =
                config.adaptiveDesktopNetworkFrameRate;
        }
    }
    if (!canStart) {
        FailOperation(operationId, "already_started",
                      "A WebRTC session can only be started once.");
        return operationId;
    }
    ChangeState(WebRtcSessionState::kStarting);
    if (!factory_) {
        ChangeState(WebRtcSessionState::kFailed);
        FailOperation(operationId, "factory_unavailable",
                      "PeerConnectionFactory is null.");
        return operationId;
    }

    if (config.includeLoopbackAdapter) {
        webrtc::PeerConnectionFactoryInterface::Options options;
        options.network_ignore_mask = 0;
        factory_->SetOptions(options);
    }

    webrtc::PeerConnectionInterface::RTCConfiguration rtcConfiguration;
    rtcConfiguration.sdp_semantics = webrtc::SdpSemantics::kUnifiedPlan;
    const bool hasConfiguredPortRange =
        config.iceMinPort != 0 || config.iceMaxPort != 0;
    if (hasConfiguredPortRange &&
        (config.iceMinPort < 1 || config.iceMaxPort > 65535 ||
         config.iceMinPort > config.iceMaxPort)) {
        ChangeState(WebRtcSessionState::kFailed);
        FailOperation(operationId, "invalid_ice_port_range",
                      "The ICE UDP port range must be between 1 and 65535, "
                      "and the minimum must not exceed the maximum.");
        return operationId;
    }
    if (hasConfiguredPortRange) {
        rtcConfiguration.set_min_port(config.iceMinPort);
        rtcConfiguration.set_max_port(config.iceMaxPort);
    }
    for (const auto& configuredServer : config.iceServers) {
        webrtc::PeerConnectionInterface::IceServer server;
        server.urls = configuredServer.urls;
        server.username = configuredServer.username;
        server.password = configuredServer.password;
        rtcConfiguration.servers.push_back(std::move(server));
    }

    auto peerOrError = factory_->CreatePeerConnectionOrError(
        rtcConfiguration, webrtc::PeerConnectionDependencies(this));
    if (!peerOrError.ok()) {
        ChangeState(WebRtcSessionState::kFailed);
        FailOperation(operationId, "peer_connection_create_failed",
                      peerOrError.error().message());
        return operationId;
    }

    auto peer = peerOrError.MoveValue();
    bool installed = false;
    {
        std::lock_guard lock(mutex_);
        if (!peerConnection_) {
            peerConnection_ = std::move(peer);
            installed = true;
        }
    }
    if (!installed) {
        FailOperation(operationId, "concurrent_start",
                      "Another thread started the WebRTC session first.");
        return operationId;
    }
    ChangeState(WebRtcSessionState::kReady);
    CompleteOperation(operationId);
    return operationId;
}
