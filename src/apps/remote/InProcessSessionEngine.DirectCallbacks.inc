// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

void InProcessSessionEngine::OnSignalingStateChanged(
    SignalingConnectionState state)
{
    std::vector<std::shared_ptr<RoomPairRuntime>> pairs;
    SessionControllerBase* legacyController = nullptr;
    bool signalingAvailable = false;
    {
        std::lock_guard lock(mutex_);
        switch (state) {
        case SignalingConnectionState::kConnecting:
        case SignalingConnectionState::kAuthenticating:
        case SignalingConnectionState::kRegistering:
            snapshot_.connectivity = SessionConnectivityState::kConnecting;
            break;
        case SignalingConnectionState::kRegistered:
            snapshot_.connectivity = SessionConnectivityState::kOnline;
            signalingAvailable = !roomRecoveryPending_;
            break;
        case SignalingConnectionState::kFailed:
            snapshot_.connectivity = SessionConnectivityState::kFailed;
            snapshot_.ownedDevicesLoaded = false;
            for (auto& device : snapshot_.ownedDevices) {
                device.online = false;
            }
            snapshot_.incomingRoomScreenShareSwitchRequests.clear();
            snapshot_.outgoingRoomScreenShareSwitchRequestId.clear();
            if (serverSessionActive_ && sessionController_) {
                signalingRecoveryPending_ = true;
            }
            if (snapshot_.room.membership == RoomMembershipState::kActive &&
                !roomRecoveryToken_.empty()) {
                roomRecoveryPending_ = true;
                snapshot_.room.errorCode = "room_signaling_recovering";
                snapshot_.room.errorMessage =
                    "Room signaling is temporarily unavailable; the membership slot is being preserved.";
            }
            break;
        case SignalingConnectionState::kDisconnected:
        case SignalingConnectionState::kClosing:
            snapshot_.connectivity = SessionConnectivityState::kOffline;
            snapshot_.ownedDevicesLoaded = false;
            for (auto& device : snapshot_.ownedDevices) {
                device.online = false;
            }
            snapshot_.incomingRoomScreenShareSwitchRequests.clear();
            snapshot_.outgoingRoomScreenShareSwitchRequestId.clear();
            if (state == SignalingConnectionState::kDisconnected &&
                serverSessionActive_ && sessionController_ &&
                snapshot_.state != SessionEngineState::kStopping &&
                snapshot_.state != SessionEngineState::kStopped) {
                signalingRecoveryPending_ = true;
            }
            if (state == SignalingConnectionState::kDisconnected &&
                snapshot_.room.membership == RoomMembershipState::kActive &&
                !roomRecoveryToken_.empty() &&
                snapshot_.state != SessionEngineState::kStopping &&
                snapshot_.state != SessionEngineState::kStopped) {
                roomRecoveryPending_ = true;
                snapshot_.room.errorCode = "room_signaling_recovering";
                snapshot_.room.errorMessage =
                    "Room signaling is temporarily unavailable; the membership slot is being preserved.";
            }
            break;
        }
        legacyController = sessionController_.get();
        for (const auto& [pairId, pair] : roomPairs_) {
            (void)pairId;
            pairs.push_back(pair);
        }
    }
    if (legacyController) {
        legacyController->SetSignalingAvailable(
            state == SignalingConnectionState::kRegistered);
    }
    for (const auto& pair : pairs) {
        if (pair && pair->controller) {
            pair->controller->SetSignalingAvailable(signalingAvailable);
        }
    }
    PublishSnapshot();
}

void InProcessSessionEngine::OnDeviceRegistered(const std::string& deviceId)
{
    bool resumeSession = false;
    bool resumeRoom = false;
    std::string sessionId;
    std::string recoveryToken;
    std::string roomId;
    std::string roomRecoveryToken;
    std::string deferredRoomLeaveId;
    {
        std::lock_guard lock(mutex_);
        snapshot_.connectivity = SessionConnectivityState::kOnline;
        snapshot_.localDeviceId = deviceId;
        deferredRoomLeaveId = std::exchange(deferredRoomLeaveId_, {});
        resumeSession = signalingRecoveryPending_ &&
                        serverSessionActive_ && sessionController_ &&
                        !snapshot_.sessionId.empty() &&
                        !sessionRecoveryToken_.empty();
        if (resumeSession) {
            sessionId = snapshot_.sessionId;
            recoveryToken = sessionRecoveryToken_;
            snapshot_.errorCode = "signaling_session_recovering";
            snapshot_.errorMessage =
                "The signaling connection was restored; the active session is being resumed.";
        }
        resumeRoom = roomRecoveryPending_ &&
                     snapshot_.room.membership ==
                         RoomMembershipState::kActive &&
                     !snapshot_.room.roomId.empty() &&
                     !roomRecoveryToken_.empty();
        if (resumeRoom) {
            roomId = snapshot_.room.roomId;
            roomRecoveryToken = roomRecoveryToken_;
            snapshot_.room.errorCode = "room_signaling_recovering";
            snapshot_.room.errorMessage =
                "The signaling connection was restored; room membership is being resumed.";
        }
        if (!resumeSession && !resumeRoom && !peerSignalingSuspended_) {
            snapshot_.errorCode.clear();
            snapshot_.errorMessage.clear();
        }
    }
    PublishSnapshot();
    if (!deferredRoomLeaveId.empty()) {
        const auto result = signaling_->LeaveRoom(
            deferredRoomLeaveId, "p2p_recovery_failed");
        if (!result.accepted) {
            std::lock_guard lock(mutex_);
            if (deferredRoomLeaveId_.empty()) {
                deferredRoomLeaveId_ = deferredRoomLeaveId;
            }
        }
    }
    if (resumeSession) {
        const auto result = signaling_->ResumeSession(sessionId,
                                                      recoveryToken);
        if (!result.accepted) {
            OnSignalingError(result.errorCode, result.errorMessage);
        }
    }
    if (resumeRoom) {
        const auto result = signaling_->ResumeRoom(roomId,
                                                   roomRecoveryToken);
        if (!result.accepted) {
            OnSignalingError(result.errorCode, result.errorMessage);
        }
    }
    if (signalingConfig_.authenticationMode ==
        SignalingAuthenticationMode::kMessageAccessToken) {
        const auto ownedDevicesResult = signaling_->RequestOwnedDevices();
        if (!ownedDevicesResult.accepted) {
            OnSignalingError(ownedDevicesResult.errorCode,
                             ownedDevicesResult.errorMessage);
        }
    }
}

void InProcessSessionEngine::OnOwnedDevicesChanged(
    const SignalingOwnedDevicesSnapshot& owned)
{
    {
        std::lock_guard lock(mutex_);
        if (owned.revision < snapshot_.ownedDevicesRevision) {
            return;
        }
        snapshot_.ownedDevicesRevision = owned.revision;
        snapshot_.ownedDevicesLoaded = true;
        snapshot_.ownedDevices.clear();
        snapshot_.ownedDevices.reserve(owned.devices.size());
        for (const auto& source : owned.devices) {
            OwnedDeviceSnapshot device;
            device.deviceId = source.deviceId;
            device.deviceName = source.deviceName;
            device.online = source.online;
            device.current = source.current;
            device.createdAt = source.createdAt;
            device.lastSeenAt = source.lastSeenAt;
            snapshot_.ownedDevices.push_back(std::move(device));
        }
    }
    PublishSnapshot();
}

void InProcessSessionEngine::OnIncomingSessionRequest(
    const IncomingSessionRequest& request)
{
    DisposeClosedSession();
    const bool ownedDeviceRequest =
        request.authorization ==
        DirectAuthorizationMethod::kOwnedAccount;
    const bool assistedRequest =
        request.authorization ==
        DirectAuthorizationMethod::kVerificationCode;
    bool busy = false;
    const bool ownedDeviceAutoAccept =
        ownedDeviceRequest && request.sameAccount && request.autoAccept;
    bool assistedCodeValid = false;
    {
        std::lock_guard lock(mutex_);
        assistedCodeValid = assistedRequest &&
            !signalingConfig_.deviceVerificationCode.empty() &&
            request.verificationCode ==
                signalingConfig_.deviceVerificationCode;
    }
    if (assistedRequest && !assistedCodeValid) {
        signaling_->RespondToSession(
            request.sessionId, false, "verification_code_invalid");
        return;
    }
    const bool autoAccept = ownedDeviceAutoAccept || assistedCodeValid;
    {
        std::lock_guard lock(mutex_);
        busy = snapshot_.state != SessionEngineState::kReady ||
               (snapshot_.room.membership != RoomMembershipState::kNone &&
                snapshot_.room.membership != RoomMembershipState::kFailed);
        if (!busy) {
            snapshot_.state = autoAccept
                ? SessionEngineState::kConnecting
                : SessionEngineState::kAwaitingLocalApproval;
            snapshot_.purpose = request.purpose;
            snapshot_.origin = ownedDeviceAutoAccept
                ? SessionOrigin::kOwnedDevice
                : (assistedRequest
                       ? SessionOrigin::kRemoteAssistance
                       : SessionOrigin::kManualDeviceId);
            snapshot_.remoteControlRole =
                request.purpose == SessionPurpose::kRemoteControl
                    ? RemoteControlRole::kControlled
                    : RemoteControlRole::kNone;
            snapshot_.sessionId = request.sessionId;
            snapshot_.peerDeviceId = request.requesterDeviceId;
            snapshot_.directSessionEverActive = false;
            snapshot_.errorCode.clear();
            snapshot_.errorMessage.clear();
            localIsOfferer_ = false;
            offerNegotiationStarted_ = false;
            sessionCloseRequested_ = false;
            sessionEndSignalSent_ = false;
            cancelWhenSessionIdKnown_ = false;
            serverSessionActive_ = false;
            snapshot_.directSignalingSessionReady = false;
            signalingRecoveryPending_ = false;
            peerSignalingSuspended_ = false;
            sessionRecoveryToken_.clear();
            pendingRemoteDescription_.reset();
            pendingRemoteCandidates_.clear();
            if (assistedCodeValid) {
                signalingConfig_.deviceVerificationCode =
                    GenerateRotatedVerificationCode(
                        signalingConfig_.deviceVerificationCode);
                snapshot_.localVerificationCode =
                    signalingConfig_.deviceVerificationCode;
            }
        }
    }
    if (busy) {
        signaling_->RespondToSession(request.sessionId, false, "device_busy");
        return;
    }
    if (autoAccept) {
        const auto result = signaling_->RespondToSession(
            request.sessionId, true, {});
        if (!result.accepted) {
            std::lock_guard lock(mutex_);
            ResetSessionStateLocked();
            snapshot_.errorCode = result.errorCode;
            snapshot_.errorMessage = result.errorMessage;
        }
    }
    PublishSnapshot();
}

void InProcessSessionEngine::OnSessionResponse(
    const SignalingSessionResponse& response)
{
    {
        std::lock_guard lock(mutex_);
        if (snapshot_.state != SessionEngineState::kConnecting ||
            (!snapshot_.sessionId.empty() &&
             snapshot_.sessionId != response.sessionId)) {
            return;
        }
        snapshot_.sessionId = response.sessionId;
        if (!response.accepted) {
            ResetSessionStateLocked();
            snapshot_.errorCode = response.reasonCode.empty()
                                      ? "session_rejected"
                                      : response.reasonCode;
            snapshot_.errorMessage = response.reasonMessage.empty()
                                         ? "The peer rejected the session."
                                         : response.reasonMessage;
        }
    }
    PublishSnapshot();
}

void InProcessSessionEngine::OnSessionPending(
    const SignalingSessionPending& pending)
{
    bool cancel = false;
    ISignalingClient* signaling = nullptr;
    {
        std::lock_guard lock(mutex_);
        if (!localIsOfferer_ ||
            snapshot_.state != SessionEngineState::kConnecting ||
            (!snapshot_.sessionId.empty() &&
             snapshot_.sessionId != pending.sessionId)) {
            return;
        }
        snapshot_.sessionId = pending.sessionId;
        if (!pending.peerDeviceId.empty()) {
            snapshot_.peerDeviceId = pending.peerDeviceId;
        }
        if (cancelWhenSessionIdKnown_) {
            cancel = true;
            sessionEndSignalSent_ = true;
            signaling = signaling_.get();
        }
    }
    if (cancel && signaling) {
        const auto result = signaling->CancelSession(
            pending.sessionId, "cancelled_by_local_user");
        {
            std::lock_guard lock(mutex_);
            ResetSessionStateLocked();
            if (!result.accepted) {
                snapshot_.errorCode = result.errorCode;
                snapshot_.errorMessage = result.errorMessage;
            }
        }
    }
    PublishSnapshot();
}

void InProcessSessionEngine::OnSessionReady(
    const SignalingSessionReady& ready)
{
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

    {
        std::lock_guard lock(mutex_);
        if (snapshot_.state != SessionEngineState::kConnecting ||
            (!snapshot_.sessionId.empty() &&
             snapshot_.sessionId != ready.sessionId)) {
            return;
        }
        snapshot_.sessionId = ready.sessionId;
        if (!ready.peerDeviceId.empty()) {
            snapshot_.peerDeviceId = ready.peerDeviceId;
        }
        serverSessionActive_ = true;
        snapshot_.directSignalingSessionReady = true;
        signalingRecoveryPending_ = false;
        peerSignalingSuspended_ = false;
        sessionRecoveryToken_ = ready.recoveryToken;
        if (!sessionController_) {
            auto session = std::make_unique<LibWebRtcSession>(
                runtime_->PeerConnectionFactory());
            std::unique_ptr<SessionControllerBase> controller;
            if (localIsOfferer_) {
                controller =
                    std::make_unique<ControllerSessionController>(
                        *session,
                        static_cast<ISessionSignalingSender&>(*this));
            } else {
                auto agent = std::make_unique<AgentSessionController>(
                    *session,
                    static_cast<ISessionSignalingSender&>(*this));
                if (snapshot_.purpose ==
                    SessionPurpose::kRemoteControl) {
                    agent->SetAnswerPreparation([this] {
                        return PrepareDirectMedia(true);
                    });
                }
                controller = std::move(agent);
            }
            webRtcSession_ = std::move(session);
            sessionController_ = std::move(controller);
            sessionController_->SetObserver(this);
            sessionController_->Start(controllerConfig);
            if (pendingRemoteDescription_) {
                sessionController_->HandleRemoteDescription(
                    *pendingRemoteDescription_);
                pendingRemoteDescription_.reset();
            }
            for (const auto& candidate : pendingRemoteCandidates_) {
                sessionController_->HandleRemoteIceCandidate(candidate);
            }
            pendingRemoteCandidates_.clear();
        }
        // Remain kConnecting until the PeerConnection reaches its active
        // state. A signaling session alone is not a working remote session.
    }
    PublishSnapshot();
}

void InProcessSessionEngine::OnSessionSuspended(
    const SignalingSessionSuspended& suspended)
{
    {
        std::lock_guard lock(mutex_);
        if (suspended.sessionId.empty() ||
            snapshot_.sessionId != suspended.sessionId ||
            suspended.peerDeviceId != snapshot_.peerDeviceId ||
            !sessionController_) {
            return;
        }
        peerSignalingSuspended_ = true;
        snapshot_.errorCode = "peer_signaling_recovering";
        snapshot_.errorMessage =
            "The peer signaling connection is temporarily unavailable; the P2P session remains active.";
    }
    PublishSnapshot();
}

void InProcessSessionEngine::OnSessionResumed(
    const SignalingSessionResumed& resumed)
{
    {
        std::lock_guard lock(mutex_);
        if (resumed.sessionId.empty() ||
            snapshot_.sessionId != resumed.sessionId ||
            !sessionController_) {
            return;
        }
        if (resumed.resumedDeviceId == snapshot_.localDeviceId) {
            signalingRecoveryPending_ = false;
            serverSessionActive_ = true;
            snapshot_.directSignalingSessionReady = true;
        } else if (resumed.resumedDeviceId == snapshot_.peerDeviceId) {
            peerSignalingSuspended_ = false;
        } else {
            return;
        }
        if (!signalingRecoveryPending_ && !peerSignalingSuspended_ &&
            (snapshot_.errorCode == "signaling_session_recovering" ||
             snapshot_.errorCode == "peer_signaling_recovering" ||
             snapshot_.errorCode == "websocket_error" ||
             snapshot_.errorCode == "heartbeat_timeout")) {
            snapshot_.errorCode.clear();
            snapshot_.errorMessage.clear();
        }
    }
    PublishSnapshot();
}

void InProcessSessionEngine::OnSessionEnded(
    const SignalingSessionEnded& ended)
{
    bool publish = false;
    {
        std::lock_guard lock(mutex_);
        if (ended.sessionId.empty() ||
            snapshot_.sessionId != ended.sessionId) {
            return;
        }
        const bool initiatedLocally =
            ended.initiatorDeviceId == snapshot_.localDeviceId;
        if (!initiatedLocally) {
            sessionCloseRequested_ = false;
            snapshot_.errorCode =
                ended.reasonCode.empty()
                    ? (ended.kind == SignalingSessionEndKind::kCancelled
                           ? "session_cancelled_by_peer"
                           : "session_closed_by_peer")
                    : ended.reasonCode;
            snapshot_.errorMessage =
                ended.kind == SignalingSessionEndKind::kCancelled
                    ? "The peer cancelled the session."
                    : "The peer closed the session.";
        }
        serverSessionActive_ = false;
        snapshot_.directSignalingSessionReady = false;
        if (sessionController_) {
            sessionController_->Close();
        } else {
            const std::string errorCode = snapshot_.errorCode;
            const std::string errorMessage = snapshot_.errorMessage;
            ResetSessionStateLocked();
            if (!initiatedLocally) {
                snapshot_.errorCode = errorCode;
                snapshot_.errorMessage = errorMessage;
            }
        }
        publish = true;
    }
    if (publish) {
        PublishSnapshot();
    }
}

void InProcessSessionEngine::OnRemoteDescription(
    const SignalingSessionDescription& description)
{
    SessionDescription remoteDescription;
    remoteDescription.type = description.type == "answer"
                                 ? SessionDescriptionType::kAnswer
                                 : SessionDescriptionType::kOffer;
    remoteDescription.sdp = description.sdp;
    remoteDescription.negotiationGeneration =
        description.negotiationGeneration;
    std::shared_ptr<RoomPairRuntime> roomPair;
    {
        std::lock_guard lock(mutex_);
        const auto roomPairIt = roomPairs_.find(description.sessionId);
        if (roomPairIt != roomPairs_.end()) {
            roomPair = roomPairIt->second;
        }
        else {
            if (description.sessionId.empty() ||
                snapshot_.sessionId != description.sessionId ||
                snapshot_.state != SessionEngineState::kConnecting) {
                return;
            }
            if (sessionController_) {
                sessionController_->HandleRemoteDescription(
                    remoteDescription);
            }
            else {
                pendingRemoteDescription_ = std::move(remoteDescription);
            }
            return;
        }
    }
    // SDP application may synchronously invoke answer preparation and pair
    // observer callbacks. Keep the engine mutex out of that call chain.
    if (roomPair && roomPair->controller) {
        roomPair->controller->HandleRemoteDescription(remoteDescription);
    }
}

void InProcessSessionEngine::OnRemoteIceCandidate(
    const SignalingIceCandidate& candidate)
{
    IceCandidate remoteCandidate;
    remoteCandidate.candidate = candidate.candidate;
    remoteCandidate.sdpMid = candidate.sdpMid;
    remoteCandidate.sdpMLineIndex = candidate.sdpMLineIndex;
    remoteCandidate.negotiationGeneration =
        candidate.negotiationGeneration;
    std::shared_ptr<RoomPairRuntime> roomPair;
    {
        std::lock_guard lock(mutex_);
        const auto roomPairIt = roomPairs_.find(candidate.sessionId);
        if (roomPairIt != roomPairs_.end()) {
            roomPair = roomPairIt->second;
        }
        else {
            if (candidate.sessionId.empty() ||
                snapshot_.sessionId != candidate.sessionId ||
                (snapshot_.state != SessionEngineState::kConnecting &&
                 snapshot_.state != SessionEngineState::kActive)) {
                return;
            }
            if (sessionController_) {
                sessionController_->HandleRemoteIceCandidate(
                    remoteCandidate);
            }
            else {
                pendingRemoteCandidates_.push_back(
                    std::move(remoteCandidate));
            }
            return;
        }
    }
    if (roomPair && roomPair->controller) {
        roomPair->controller->HandleRemoteIceCandidate(remoteCandidate);
    }
}

void InProcessSessionEngine::OnIceRestartRequested(
    const SignalingIceRestartRequest& request)
{
    std::shared_ptr<RoomPairRuntime> roomPair;
    SessionControllerBase* legacyController = nullptr;
    {
        std::lock_guard lock(mutex_);
        const auto found = roomPairs_.find(request.sessionId);
        if (found != roomPairs_.end()) {
            roomPair = found->second;
        } else if (snapshot_.sessionId == request.sessionId) {
            legacyController = sessionController_.get();
        }
    }
    if (roomPair && roomPair->controller) {
        roomPair->controller->HandleRemoteIceRestartRequest(
            request.observedGeneration, request.requestSequence);
    } else if (legacyController) {
        legacyController->HandleRemoteIceRestartRequest(
            request.observedGeneration, request.requestSequence);
    }
}

void InProcessSessionEngine::OnIceRestartCancelled(
    const SignalingIceRestartCancel& cancel)
{
    std::shared_ptr<RoomPairRuntime> roomPair;
    SessionControllerBase* legacyController = nullptr;
    {
        std::lock_guard lock(mutex_);
        const auto found = roomPairs_.find(cancel.sessionId);
        if (found != roomPairs_.end()) {
            roomPair = found->second;
        } else if (snapshot_.sessionId == cancel.sessionId) {
            legacyController = sessionController_.get();
        }
    }
    if (roomPair && roomPair->controller) {
        roomPair->controller->HandleRemoteIceRestartCancel(
            cancel.observedGeneration, cancel.requestSequence);
    } else if (legacyController) {
        legacyController->HandleRemoteIceRestartCancel(
            cancel.observedGeneration, cancel.requestSequence);
    }
}

void InProcessSessionEngine::OnHeartbeatAcknowledged(
    std::uint32_t roundTripMs)
{
    (void)roundTripMs;
}

void InProcessSessionEngine::OnSignalingError(
    const std::string& code,
    const std::string& message)
{
    std::vector<std::shared_ptr<RoomPairRuntime>> removedPairs;
    {
        std::lock_guard lock(mutex_);
        const bool pendingDirectRequestFailed =
            snapshot_.state == SessionEngineState::kConnecting &&
            localIsOfferer_ && !sessionController_ &&
            !serverSessionActive_ &&
            (code == "target_offline" || code == "device_busy" ||
             code == "rate_limited" ||
             code == "invalid_session_request" ||
             code == "invalid_session_permissions" ||
             code == "session_request_timeout");
        if (pendingDirectRequestFailed) {
            ResetSessionStateLocked();
        }
        snapshot_.errorCode = code;
        snapshot_.errorMessage = message;
        const bool pendingRoomOperation =
            snapshot_.room.membership == RoomMembershipState::kCreating ||
            snapshot_.room.membership == RoomMembershipState::kJoinPending;
        if (pendingRoomOperation) {
            snapshot_.room.membership = RoomMembershipState::kFailed;
            snapshot_.room.errorCode = code;
            snapshot_.room.errorMessage = message;
        }
        if (code == "session_resume_rejected" && sessionController_) {
            signalingRecoveryPending_ = false;
            serverSessionActive_ = false;
            snapshot_.directSignalingSessionReady = false;
            sessionCloseRequested_ = false;
            sessionController_->Close();
        }
        if (code.rfind("room_", 0) == 0) {
            snapshot_.room.errorCode = code;
            snapshot_.room.errorMessage = message;
            if (code == "room_resume_rejected") {
                const std::string roomId = snapshot_.room.roomId;
                removedPairs.reserve(roomPairs_.size());
                for (auto& [pairId, pair] : roomPairs_) {
                    (void)pairId;
                    removedPairs.push_back(std::move(pair));
                }
                roomPairs_.clear();
                ResetRoomStateLocked();
                snapshot_.room.membership = RoomMembershipState::kFailed;
                snapshot_.room.roomId = roomId;
                snapshot_.room.errorCode = code;
                snapshot_.room.errorMessage = message;
            }
        }
    }
    for (auto& pair : removedPairs) {
        RetireRoomPair(std::move(pair));
    }
    PublishSnapshot();
}

void InProcessSessionEngine::OnAccountDeletionResult(
    const SignalingAccountDeletionResult& result)
{
    std::function<void(const SignalingAccountDeletionResult&)> callback;
    {
        std::lock_guard lock(mutex_);
        callback = accountDeletionResultCallback_;
    }
    if (callback) {
        callback(result);
    }
}
