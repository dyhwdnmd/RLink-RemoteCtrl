// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

SessionCommandResult InProcessSessionEngine::RequireReady(
    const char* operation) const
{
    const auto snapshot = Snapshot();
    if (snapshot.state == SessionEngineState::kReady) {
        return Success();
    }
    return Failure("engine_not_ready",
                   std::string("Cannot ") + operation +
                       " while the session engine is not ready.");
}

bool InProcessSessionEngine::SignalingIsOnline() const
{
    std::lock_guard lock(mutex_);
    return snapshot_.connectivity == SessionConnectivityState::kOnline;
}

void InProcessSessionEngine::DisposeClosedSession()
{
    std::unique_ptr<SessionControllerBase> controller;
    std::unique_ptr<LibWebRtcSession> session;
    {
        std::lock_guard lock(mutex_);
        if (!sessionController_ ||
            sessionController_->Snapshot().state !=
                SessionControllerState::kClosed) {
            return;
        }
        controller = std::move(sessionController_);
        session = std::move(webRtcSession_);
    }
    controller->SetObserver(nullptr);
    controller.reset();
    session.reset();
    {
        std::lock_guard lock(mutex_);
        directSession_.audioDevicesApplied = false;
    }
}

void InProcessSessionEngine::ResetSessionStateLocked()
{
    snapshot_.state = SessionEngineState::kReady;
    snapshot_.purpose = SessionPurpose::kNone;
    snapshot_.origin = SessionOrigin::kNone;
    snapshot_.remoteControlRole = RemoteControlRole::kNone;
    snapshot_.localCamera = LocalCameraState::kOff;
    snapshot_.remoteCameraPublishing = false;
    snapshot_.sessionId.clear();
    snapshot_.peerDeviceId.clear();
    snapshot_.directSignalingSessionReady = false;
    snapshot_.directMediaSlotsPrepared = false;
    snapshot_.directControlReliableChannelOpen = false;
    snapshot_.directInputFastChannelOpen = false;
    snapshot_.directFileTransferChannelOpen = false;
    snapshot_.directClipboardReliableChannelOpen = false;
    snapshot_.directClipboardTransferChannelOpen = false;
    snapshot_.directSessionEverActive = false;
    snapshot_.directIceRestartAttempt = 0;
    snapshot_.directRemoteDisplay = {};
    snapshot_.directRemoteDisplayLayoutVersion = 0;
    snapshot_.directRemoteScreenShareGeneration = 0;
    snapshot_.directScreenPreferencePending = false;
    snapshot_.directScreenPreferenceSequence = 0;
    snapshot_.directScreenWidth = 0;
    snapshot_.directScreenHeight = 0;
    snapshot_.directScreenFramesPerSecond = kDefaultScreenFrameRate;
    snapshot_.directScreenMaximumFrameRate = kMaximumScreenFrameRate;
    snapshot_.directScreenMaxBitrateBps = 0;
    snapshot_.directRemoteDisplayCatalogReported = false;
    snapshot_.directRemoteDisplayCatalogLayoutVersion = 0;
    snapshot_.directRemoteDisplays.clear();
    snapshot_.directRemoteDisplaySwitchPending = false;
    snapshot_.directRemoteDisplaySwitchSequence = 0;
    snapshot_.directRemoteDisplaySwitchError.clear();
    snapshot_.errorCode.clear();
    snapshot_.errorMessage.clear();
    localIsOfferer_ = false;
    offerNegotiationStarted_ = false;
    directSession_.Reset();
    sessionCloseRequested_ = false;
    sessionEndSignalSent_ = false;
    cancelWhenSessionIdKnown_ = false;
    serverSessionActive_ = false;
    signalingRecoveryPending_ = false;
    peerSignalingSuspended_ = false;
    sessionRecoveryToken_.clear();
    pendingRemoteDescription_.reset();
    pendingRemoteCandidates_.clear();
}

void InProcessSessionEngine::ResetRoomStateLocked()
{
    snapshot_.room = {};
    snapshot_.incomingRoomJoinRequests.clear();
    snapshot_.incomingRoomScreenShareSwitchRequests.clear();
    snapshot_.incomingRoomControlRequests.clear();
    snapshot_.incomingRoomScreenShareViewRequests.clear();
    snapshot_.roomMemberActionResults.clear();
    snapshot_.outgoingRoomScreenShareSwitchRequestId.clear();
    snapshot_.roomPeerConnections.clear();
    snapshot_.activeSharedDisplay = {};
    snapshot_.activeSharedDisplayLayoutVersion = 0;
    roomRecoveryToken_.clear();
    roomScreenShareGrantId_.clear();
    ++localScreenShareGeneration_;
    snapshot_.screenShareGeneration = localScreenShareGeneration_;
    if (runtime_) {
        runtime_->SetDesktopShareGeneration(localScreenShareGeneration_);
        runtime_->SetDesktopCaptureAdapterLuid(0);
    }
    roomControlGrantId_.clear();
    roomControlGrantScreenSharerDeviceId_.clear();
    roomControlGrantControllerDeviceId_.clear();
    snapshot_.roomControlGrantActive = false;
    ++localCameraTrackGeneration_;
    roomAudioDevicesApplied_ = false;
    nextRoomInputSequence_ = 0;
    nextRoomScreenControlSequence_ = 0;
    localScreenFrameRate_ = kDefaultScreenFrameRate;
    roomScreenStreamPreferences_.clear();
    roomRecoveryPending_ = false;
    roomLeaveRequested_ = false;
    localRoomVideoTracks_.clear();
    if (auto capture =
            std::exchange(localDesktopCaptureSource_, nullptr)) {
        std::jthread stopThread(
            [capture = std::move(capture)](std::stop_token) {
                capture->StopCapture();
            });
        retiredDesktopStopThreads_.push_back(std::move(stopThread));
    }
    if (localCameraCaptureSource_) {
        if (localCameraPreviewTrack_ && localCameraPreviewSink_) {
            localCameraPreviewTrack_->RemoveSink(localCameraPreviewSink_);
        }
        localCameraCaptureSource_->StopCapture();
        localCameraCaptureSource_ = nullptr;
    }
    localCameraPreviewTrack_ = nullptr;
    if (localMicrophoneAudioTrack_) {
        localMicrophoneAudioTrack_->set_enabled(false);
    }
    localMicrophoneAudioTrack_ = nullptr;
    localMicrophoneAudioSource_ = nullptr;
    snapshot_.localCamera = LocalCameraState::kOff;
    snapshot_.localMediaDevices.camera.activeDeviceId.clear();
    if (snapshot_.localMediaDevices.camera.state ==
        MediaDeviceSelectionState::kSwitching) {
        snapshot_.localMediaDevices.camera.state =
            MediaDeviceSelectionState::kReady;
        snapshot_.localMediaDevices.camera.errorCode.clear();
        snapshot_.localMediaDevices.camera.errorMessage.clear();
        ++snapshot_.localMediaDevices.revision;
    }
    snapshot_.localMicrophone = LocalMicrophoneState::kOff;
    snapshot_.roomAudioPlaybackMuted = false;
}

void InProcessSessionEngine::StopLocalDesktopCapture()
{
    StopRemoteCursorPublishing();
    webrtc::scoped_refptr<WindowsDesktopCaptureSource> source;
    {
        std::lock_guard lock(mutex_);
        ++localScreenShareGeneration_;
        snapshot_.screenShareGeneration = localScreenShareGeneration_;
        if (runtime_) {
            runtime_->SetDesktopShareGeneration(localScreenShareGeneration_);
            runtime_->SetDesktopCaptureAdapterLuid(0);
        }
        source = std::exchange(localDesktopCaptureSource_, nullptr);
        snapshot_.activeSharedDisplay = {};
        snapshot_.activeSharedDisplayLayoutVersion = 0;
    }

    (void)ClearRoomVideoSource(kScreenMainVideoSlot);
    if (!source) {
        return;
    }
    // OnFrame feeds WebRTC synchronously. If an encoder or driver is busy,
    // joining that capture thread here freezes the Qt/signaling caller. The
    // track has already been replaced with the idle source, so physical
    // capture teardown can safely finish in the background.
    std::jthread stopThread(
        [capture = std::move(source)](std::stop_token) {
            capture->StopCapture();
        });
    std::lock_guard lock(mutex_);
    retiredDesktopStopThreads_.push_back(std::move(stopThread));
}

void InProcessSessionEngine::BroadcastSharedDisplayLayout()
{
    SharedDisplayLayout layout;
    std::vector<std::shared_ptr<RoomPairRuntime>> recipients;
    {
        std::lock_guard lock(mutex_);
        if (snapshot_.room.membership != RoomMembershipState::kActive ||
            snapshot_.room.screenSharerDeviceId !=
                snapshot_.localDeviceId ||
            snapshot_.activeSharedDisplay.sessionDisplayId == 0 ||
            snapshot_.activeSharedDisplayLayoutVersion == 0) {
            return;
        }
        layout.roomId = snapshot_.room.roomId;
        layout.senderDeviceId = snapshot_.localDeviceId;
        // Use the server-authoritative share epoch so every room member can
        // reject a late layout packet from an earlier lease.
        layout.screenShareGeneration =
            snapshot_.room.screenShareEpoch;
        layout.layoutVersion =
            snapshot_.activeSharedDisplayLayoutVersion;
        layout.selectedDisplay = snapshot_.activeSharedDisplay;
        for (const auto& [pairId, pair] : roomPairs_) {
            (void)pairId;
            const auto channel = pair->openDataChannels.find(
                std::string(kControlReliableChannel));
            if (pair->controller &&
                channel != pair->openDataChannels.end() &&
                channel->second) {
                recipients.push_back(pair);
            }
        }
    }

    std::vector<std::uint8_t> encoded;
    if (!EncodeSharedDisplayLayout(layout, &encoded)) {
        return;
    }
    for (const auto& pair : recipients) {
        (void)pair->controller->QueueData(
            std::string(kControlReliableChannel), encoded, true);
    }
}

void InProcessSessionEngine::BroadcastSharedDisplayCatalog()
{
    SharedDisplayCatalog catalog;
    std::vector<std::shared_ptr<RoomPairRuntime>> recipients;
    {
        std::lock_guard lock(mutex_);
        if (snapshot_.room.membership !=
                RoomMembershipState::kActive ||
            snapshot_.room.screenSharerDeviceId !=
                snapshot_.localDeviceId ||
            snapshot_.room.screenShareEpoch == 0 ||
            snapshot_.localDisplayTopology.layoutVersion == 0 ||
            snapshot_.localDisplayTopology.displays.empty()) {
            return;
        }
        catalog.roomId = snapshot_.room.roomId;
        catalog.senderDeviceId = snapshot_.localDeviceId;
        catalog.screenShareGeneration =
            snapshot_.room.screenShareEpoch;
        catalog.layoutVersion =
            snapshot_.localDisplayTopology.layoutVersion;
        catalog.displays =
            snapshot_.localDisplayTopology.displays;
        for (const auto& [pairId, pair] : roomPairs_) {
            (void)pairId;
            const auto channel = pair->openDataChannels.find(
                std::string(kControlReliableChannel));
            if (pair->controller &&
                channel != pair->openDataChannels.end() &&
                channel->second) {
                recipients.push_back(pair);
            }
        }
    }
    std::vector<std::uint8_t> encoded;
    if (!EncodeSharedDisplayCatalog(catalog, &encoded)) {
        return;
    }
    for (const auto& pair : recipients) {
        (void)pair->controller->QueueData(
            std::string(kControlReliableChannel), encoded, true);
    }
}
