// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

void InProcessSessionEngine::OnRoomPairReady(
    const SignalingRoomPairReady& ready)
{
    if (ready.pairId.empty() || ready.roomId.empty() ||
        ready.peerDeviceId.empty()) {
        return;
    }

    SessionControllerConfig controllerConfig;
    controllerConfig.negotiationTimeout = options_.negotiationTimeout;
    controllerConfig.reconnectTimeout = options_.reconnectTimeout;
    controllerConfig.webRtc.includeLoopbackAdapter =
        options_.includeLoopbackAdapter;
    controllerConfig.webRtc.fastDesktopBweStartup =
        options_.desktopCaptureImplementation ==
        DesktopCaptureImplementation::kLibWebRtc;
    controllerConfig.webRtc.adaptiveDesktopNetworkFrameRate = true;
    controllerConfig.webRtc.iceMinPort = options_.iceMinPort;
    controllerConfig.webRtc.iceMaxPort = options_.iceMaxPort;
    for (const auto& signalingServer : ready.iceServers) {
        IceServerConfig server;
        server.urls = signalingServer.urls;
        server.username = signalingServer.username;
        server.password = signalingServer.credential;
        controllerConfig.webRtc.iceServers.push_back(std::move(server));
    }

    auto pair = std::make_shared<RoomPairRuntime>();
    pair->pairId = ready.pairId;
    pair->roomId = ready.roomId;
    pair->peerDeviceId = ready.peerDeviceId;
    pair->localIsOfferer = ready.localIsOfferer;
    pair->bridge = std::make_unique<RoomPairBridge>(*this, ready.pairId);
    pair->session = std::make_unique<LibWebRtcSession>(
        runtime_->PeerConnectionFactory());
    if (ready.localIsOfferer) {
        pair->controller =
            std::make_unique<ControllerSessionController>(
                *pair->session, *pair->bridge);
    } else {
        pair->controller =
            std::make_unique<AgentSessionController>(
                *pair->session, *pair->bridge);
        static_cast<AgentSessionController*>(pair->controller.get())
            ->SetAnswerPreparation(
                [this, pairId = ready.pairId] {
                    return PrepareRoomPairAnswer(pairId);
                });
    }
    pair->controller->SetObserver(pair->bridge.get());

    std::shared_ptr<RoomPairRuntime> pairToStart;
    {
        std::lock_guard lock(mutex_);
        if (snapshot_.room.membership != RoomMembershipState::kActive ||
            snapshot_.room.roomId != ready.roomId ||
            ready.peerDeviceId == snapshot_.localDeviceId) {
            return;
        }
        const auto peerMember = std::find_if(
            snapshot_.room.members.begin(), snapshot_.room.members.end(),
            [&ready](const RoomMemberSnapshot& member) {
                return member.deviceId == ready.peerDeviceId;
            });
        if (peerMember == snapshot_.room.members.end()) {
            return;
        }
        const auto existing = roomPairs_.find(ready.pairId);
        if (existing != roomPairs_.end()) {
            if (existing->second->roomId != ready.roomId ||
                existing->second->peerDeviceId != ready.peerDeviceId ||
                existing->second->localIsOfferer != ready.localIsOfferer) {
                snapshot_.room.errorCode = "room_pair_identity_mismatch";
                snapshot_.room.errorMessage =
                    "The signaling server changed an existing room pair identity.";
            }
            return;
        }
        if (roomPairs_.size() >=
            static_cast<std::size_t>(kProtocolMaximumRoomMembers - 1)) {
            snapshot_.room.errorCode = "room_pair_limit_exceeded";
            snapshot_.room.errorMessage =
                "The local room pair count exceeded the protocol limit.";
            return;
        }

        RoomPeerConnectionSnapshot pairSnapshot;
        pairSnapshot.pairId = ready.pairId;
        pairSnapshot.peerDeviceId = ready.peerDeviceId;
        pairSnapshot.localIsOfferer = ready.localIsOfferer;
        pairSnapshot.state = RoomPeerConnectionState::kStarting;
        snapshot_.roomPeerConnections.push_back(
            std::move(pairSnapshot));
        std::sort(snapshot_.roomPeerConnections.begin(),
                  snapshot_.roomPeerConnections.end(),
                  [](const RoomPeerConnectionSnapshot& left,
                     const RoomPeerConnectionSnapshot& right) {
                      return left.peerDeviceId < right.peerDeviceId;
                  });

        pairToStart = pair;
        roomPairs_.emplace(ready.pairId, std::move(pair));
    }
    // Start can synchronously enter observer callbacks. Calling it while the
    // engine mutex is held can deadlock those callbacks when they publish the
    // first pair snapshot.
    pairToStart->controller->Start(controllerConfig);
    PublishSnapshot();
}

void InProcessSessionEngine::OnRoomPairClosed(
    const SignalingRoomPairClosed& closed)
{
    std::shared_ptr<RoomPairRuntime> pair;
    webrtc::scoped_refptr<WindowsDesktopCaptureSource> source;
    std::uint32_t captureFrameRate = kDefaultScreenFrameRate;
    {
        std::lock_guard lock(mutex_);
        auto pairIt = roomPairs_.find(closed.pairId);
        if (pairIt == roomPairs_.end()) {
            return;
        }
        if ((!snapshot_.room.roomId.empty() &&
             snapshot_.room.roomId != closed.roomId) ||
            pairIt->second->peerDeviceId != closed.peerDeviceId) {
            return;
        }
        pair = std::move(pairIt->second);
        roomPairs_.erase(pairIt);
        if (roomPairs_.empty()) {
            roomAudioDevicesApplied_ = false;
        }
        roomScreenStreamPreferences_.erase(closed.pairId);
        if (!roomScreenStreamPreferences_.empty()) {
            captureFrameRate = kMinimumScreenFrameRate;
        }
        for (const auto& [requestPairId, preference] :
             roomScreenStreamPreferences_) {
            (void)requestPairId;
            captureFrameRate = (std::max)(
                captureFrameRate, preference.framesPerSecond);
        }
        localScreenFrameRate_ = captureFrameRate;
        source = localDesktopCaptureSource_;
        std::erase_if(snapshot_.roomPeerConnections,
                      [&closed](const RoomPeerConnectionSnapshot& current) {
                          return current.pairId == closed.pairId;
                      });
    }
    if (source) {
        source->SetTargetFrameRate(captureFrameRate);
    }
    RetireRoomPair(std::move(pair));
    PublishSnapshot();
}

void InProcessSessionEngine::RetireRoomPair(
    std::shared_ptr<RoomPairRuntime> pair)
{
    if (!pair) {
        return;
    }
    if (pair->controller) {
        pair->controller->SetObserver(nullptr);
    }
    // The bridge references this engine. Remove it before ownership crosses
    // to the cleanup thread; the detached controller no longer publishes.
    pair->bridge.reset();
    std::jthread cleanup([pair = std::move(pair)](std::stop_token) mutable {
        pair.reset();
    });
    std::lock_guard lock(mutex_);
    retiredRoomPairThreads_.push_back(std::move(cleanup));
}

bool InProcessSessionEngine::SendRoomPairDescription(
    const std::string& pairId,
    const SessionDescription& description)
{
    ISignalingClient* signaling = nullptr;
    {
        std::lock_guard lock(mutex_);
        if (!signaling_ ||
            snapshot_.connectivity != SessionConnectivityState::kOnline ||
            snapshot_.room.membership != RoomMembershipState::kActive ||
            !roomPairs_.contains(pairId)) {
            return false;
        }
        signaling = signaling_.get();
    }
    SignalingSessionDescription outgoing;
    outgoing.sessionId = pairId;
    outgoing.type = description.type == SessionDescriptionType::kOffer
                        ? "offer"
                        : "answer";
    outgoing.sdp = description.sdp;
    outgoing.negotiationGeneration = description.negotiationGeneration;
    return signaling->SendDescription(outgoing).accepted;
}

bool InProcessSessionEngine::SendRoomPairIceCandidate(
    const std::string& pairId,
    const IceCandidate& candidate)
{
    ISignalingClient* signaling = nullptr;
    {
        std::lock_guard lock(mutex_);
        if (!signaling_ ||
            snapshot_.connectivity != SessionConnectivityState::kOnline ||
            snapshot_.room.membership != RoomMembershipState::kActive ||
            !roomPairs_.contains(pairId)) {
            return false;
        }
        signaling = signaling_.get();
    }
    SignalingIceCandidate outgoing;
    outgoing.sessionId = pairId;
    outgoing.candidate = candidate.candidate;
    outgoing.sdpMid = candidate.sdpMid;
    outgoing.sdpMLineIndex = candidate.sdpMLineIndex;
    outgoing.negotiationGeneration = candidate.negotiationGeneration;
    return signaling->SendIceCandidate(outgoing).accepted;
}

bool InProcessSessionEngine::SendRoomPairIceRestartRequest(
    const std::string& pairId,
    std::uint64_t observedGeneration,
    std::uint64_t requestSequence)
{
    ISignalingClient* signaling = nullptr;
    {
        std::lock_guard lock(mutex_);
        if (!signaling_ ||
            snapshot_.connectivity != SessionConnectivityState::kOnline ||
            snapshot_.room.membership != RoomMembershipState::kActive ||
            !roomPairs_.contains(pairId)) {
            return false;
        }
        signaling = signaling_.get();
    }
    SignalingIceRestartRequest request;
    request.sessionId = pairId;
    request.observedGeneration = observedGeneration;
    request.requestSequence = requestSequence;
    return signaling->SendIceRestartRequest(request).accepted;
}

bool InProcessSessionEngine::SendRoomPairIceRestartCancel(
    const std::string& pairId,
    std::uint64_t observedGeneration,
    std::uint64_t requestSequence)
{
    ISignalingClient* signaling = nullptr;
    {
        std::lock_guard lock(mutex_);
        if (!signaling_ ||
            snapshot_.connectivity != SessionConnectivityState::kOnline ||
            snapshot_.room.membership != RoomMembershipState::kActive ||
            !roomPairs_.contains(pairId)) {
            return false;
        }
        signaling = signaling_.get();
    }
    SignalingIceRestartCancel cancel;
    cancel.sessionId = pairId;
    cancel.observedGeneration = observedGeneration;
    cancel.requestSequence = requestSequence;
    return signaling->SendIceRestartCancel(cancel).accepted;
}
