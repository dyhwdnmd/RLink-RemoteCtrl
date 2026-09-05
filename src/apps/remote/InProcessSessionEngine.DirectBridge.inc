// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

void InProcessSessionEngine::OnRoomPairRemoteTrackAdded(
    const std::string& pairId,
    const RemoteTrackInfo& track)
{
    (void)pairId;
    (void)track;
}

bool InProcessSessionEngine::SendDescription(
    const SessionDescription& description)
{
    SignalingSessionDescription signalingDescription;
    ISignalingClient* signaling = nullptr;
    {
        std::lock_guard lock(mutex_);
        if (!signaling_ || snapshot_.sessionId.empty() ||
            snapshot_.state == SessionEngineState::kStopping ||
            snapshot_.state == SessionEngineState::kStopped) {
            return false;
        }
        signaling = signaling_.get();
        signalingDescription.sessionId = snapshot_.sessionId;
    }
    signalingDescription.type =
        description.type == SessionDescriptionType::kOffer ? "offer"
                                                            : "answer";
    signalingDescription.sdp = description.sdp;
    signalingDescription.negotiationGeneration =
        description.negotiationGeneration;
    return signaling->SendDescription(signalingDescription).accepted;
}

bool InProcessSessionEngine::SendIceCandidate(
    const IceCandidate& candidate)
{
    SignalingIceCandidate signalingCandidate;
    ISignalingClient* signaling = nullptr;
    {
        std::lock_guard lock(mutex_);
        if (!signaling_ || snapshot_.sessionId.empty() ||
            snapshot_.state == SessionEngineState::kStopping ||
            snapshot_.state == SessionEngineState::kStopped) {
            return false;
        }
        signaling = signaling_.get();
        signalingCandidate.sessionId = snapshot_.sessionId;
    }
    signalingCandidate.candidate = candidate.candidate;
    signalingCandidate.sdpMid = candidate.sdpMid;
    signalingCandidate.sdpMLineIndex = candidate.sdpMLineIndex;
    signalingCandidate.negotiationGeneration =
        candidate.negotiationGeneration;
    return signaling->SendIceCandidate(signalingCandidate).accepted;
}

bool InProcessSessionEngine::RequestIceRestart(
    std::uint64_t observedGeneration,
    std::uint64_t requestSequence)
{
    SignalingIceRestartRequest request;
    ISignalingClient* signaling = nullptr;
    {
        std::lock_guard lock(mutex_);
        if (!signaling_ || snapshot_.sessionId.empty() ||
            snapshot_.connectivity != SessionConnectivityState::kOnline) {
            return false;
        }
        signaling = signaling_.get();
        request.sessionId = snapshot_.sessionId;
    }
    request.observedGeneration = observedGeneration;
    request.requestSequence = requestSequence;
    return signaling->SendIceRestartRequest(request).accepted;
}

bool InProcessSessionEngine::CancelIceRestart(
    std::uint64_t observedGeneration,
    std::uint64_t requestSequence)
{
    SignalingIceRestartCancel cancel;
    ISignalingClient* signaling = nullptr;
    {
        std::lock_guard lock(mutex_);
        if (!signaling_ || snapshot_.sessionId.empty() ||
            snapshot_.connectivity != SessionConnectivityState::kOnline) {
            return false;
        }
        signaling = signaling_.get();
        cancel.sessionId = snapshot_.sessionId;
    }
    cancel.observedGeneration = observedGeneration;
    cancel.requestSequence = requestSequence;
    return signaling->SendIceRestartCancel(cancel).accepted;
}

void InProcessSessionEngine::OnControllerSnapshot(
    const SessionControllerSnapshot& controllerSnapshot)
{
    bool reapplyAudioDevices = false;
    bool startOffer = false;
    bool stopDirectCapture = false;
    std::string failedSessionIdToClose;
    std::string failedSessionReason;
    ISignalingClient* signaling = nullptr;
    ControllerSessionController* offerController = nullptr;
    std::vector<DataChannelSpec> offerChannels;
    {
        std::lock_guard lock(mutex_);
        if (snapshot_.state == SessionEngineState::kStopping ||
            snapshot_.state == SessionEngineState::kStopped) {
            return;
        }
        snapshot_.directIceRestartAttempt =
            controllerSnapshot.iceRestartAttempt;
        switch (controllerSnapshot.state) {
        case SessionControllerState::kReady:
            if (!directSession_.audioDevicesApplied) {
                directSession_.audioDevicesApplied = true;
                reapplyAudioDevices = true;
            }
            if (localIsOfferer_ && !offerNegotiationStarted_ &&
                sessionController_) {
                offerNegotiationStarted_ = true;
                offerChannels =
                    snapshot_.purpose == SessionPurpose::kCameraOnly
                        ? DefaultCameraSessionDataChannels()
                        : DefaultRemoteControlDataChannels();
                offerController = static_cast<
                    ControllerSessionController*>(sessionController_.get());
                startOffer = true;
            }
            break;
        case SessionControllerState::kNegotiating:
        case SessionControllerState::kDisconnected:
        case SessionControllerState::kWaitingForSignaling:
        case SessionControllerState::kRestartingIce:
            snapshot_.state = SessionEngineState::kConnecting;
            break;
        case SessionControllerState::kConnected:
            snapshot_.state = SessionEngineState::kActive;
            snapshot_.directSessionEverActive = true;
            snapshot_.errorCode.clear();
            snapshot_.errorMessage.clear();
            break;
        case SessionControllerState::kFailed:
            snapshot_.state = SessionEngineState::kFailed;
            snapshot_.errorCode = controllerSnapshot.errorCode;
            snapshot_.errorMessage = controllerSnapshot.errorMessage;
            break;
        case SessionControllerState::kClosed: {
            stopDirectCapture =
                localDesktopCaptureSource_ != nullptr ||
                directSession_.mediaSlotsPrepared;
            const bool preserveError = !sessionCloseRequested_;
            const std::string errorCode =
                controllerSnapshot.errorCode.empty()
                    ? snapshot_.errorCode
                    : controllerSnapshot.errorCode;
            const std::string errorMessage =
                controllerSnapshot.errorMessage.empty()
                    ? snapshot_.errorMessage
                    : controllerSnapshot.errorMessage;
            // A negotiation/recovery failure closes SessionController without
            // going through Disconnect().  The signaling session is already
            // active at this point, so explicitly close it before discarding
            // its identity.  Otherwise the server keeps both devices busy and
            // rejects the next assisted/owned-device request.
            if (preserveError && serverSessionActive_ && signaling_ &&
                !snapshot_.sessionId.empty() && !sessionEndSignalSent_) {
                failedSessionIdToClose = snapshot_.sessionId;
                failedSessionReason = errorCode.empty()
                    ? "p2p_connection_failed"
                    : errorCode;
                signaling = signaling_.get();
                sessionEndSignalSent_ = true;
            }
            ResetSessionStateLocked();
            if (preserveError) {
                snapshot_.errorCode = errorCode;
                snapshot_.errorMessage = errorMessage;
            }
            break;
        }
        default:
            break;
        }
    }
    if (reapplyAudioDevices) {
        (void)runtime_->ReapplyPreferredAudioDevices();
        (void)RefreshLocalMediaDevices();
    }
    if (startOffer && offerController) {
        if (controllerSnapshot.state == SessionControllerState::kReady &&
            Snapshot().purpose == SessionPurpose::kRemoteControl) {
            if (const auto error = PrepareDirectMedia(false)) {
                {
                    std::lock_guard lock(mutex_);
                    snapshot_.state = SessionEngineState::kFailed;
                    snapshot_.errorCode = error->code;
                    snapshot_.errorMessage = error->message;
                }
                PublishSnapshot();
                return;
            }
        }
        offerController->Connect(offerChannels);
    }
    if (stopDirectCapture) {
        StopDirectMicrophoneCapture();
        StopDirectDesktopCapture();
    }
    if (signaling && !failedSessionIdToClose.empty()) {
        (void)signaling->CloseSession(
            failedSessionIdToClose, failedSessionReason);
    }
    PublishSnapshot();
}

void InProcessSessionEngine::OnDataChannelStateChanged(
    const DataChannelInfo& channel)
{
    bool publishLayout = false;
    bool publishCatalog = false;
    bool requestLayout = false;
    bool republishCursor = false;
    {
        std::lock_guard lock(mutex_);
        directSession_.openDataChannels[channel.label] =
            channel.state == DataChannelState::kOpen;
        if (channel.label == kControlReliableChannel) {
            snapshot_.directControlReliableChannelOpen =
                channel.state == DataChannelState::kOpen;
            publishLayout =
                snapshot_.directControlReliableChannelOpen &&
                snapshot_.remoteControlRole ==
                    RemoteControlRole::kControlled &&
                snapshot_.activeSharedDisplay.sessionDisplayId != 0 &&
                snapshot_.activeSharedDisplayLayoutVersion != 0;
            requestLayout =
                snapshot_.directControlReliableChannelOpen &&
                snapshot_.remoteControlRole ==
                    RemoteControlRole::kController;
            publishCatalog =
                snapshot_.directControlReliableChannelOpen &&
                snapshot_.remoteControlRole ==
                    RemoteControlRole::kControlled;
        } else if (channel.label == kInputFastChannel) {
            snapshot_.directInputFastChannelOpen =
                channel.state == DataChannelState::kOpen;
        } else if (channel.label == kFileTransferChannel) {
            snapshot_.directFileTransferChannelOpen =
                channel.state == DataChannelState::kOpen;
        } else if (channel.label == kClipboardReliableChannel) {
            snapshot_.directClipboardReliableChannelOpen =
                channel.state == DataChannelState::kOpen;
        } else if (channel.label == kClipboardTransferChannel) {
            snapshot_.directClipboardTransferChannelOpen =
                channel.state == DataChannelState::kOpen;
        }
        republishCursor =
            channel.state == DataChannelState::kOpen &&
            (channel.label == kControlReliableChannel ||
             channel.label == kTelemetryChannel) &&
            snapshot_.remoteControlRole == RemoteControlRole::kControlled;
    }
    PublishSnapshot();
    if (publishLayout) {
        BroadcastDirectSharedDisplayLayout();
    }
    if (publishCatalog) {
        BroadcastDirectSharedDisplayCatalog();
    }
    if (requestLayout) {
        RequestDirectSharedDisplayLayout();
    }
    if (republishCursor) {
        RepublishRemoteCursor();
    }
}

void InProcessSessionEngine::OnDataMessage(
    const std::string& label,
    std::span<const std::uint8_t> payload,
    bool binary)
{
    if (!binary) {
        return;
    }
    if (DispatchRemoteCursorData({}, label, payload)) {
        return;
    }
    if (DispatchDirectAuxiliaryData(label, payload)) {
        return;
    }
    if (DispatchDirectScreenData(label, payload)) {
        return;
    }
    if (label == kControlReliableChannel) {
        ScreenRefreshRequest refreshRequest;
        if (DecodeScreenRefreshRequest(payload, &refreshRequest)) {
            webrtc::scoped_refptr<WindowsDesktopCaptureSource> source;
            {
                std::lock_guard lock(mutex_);
                if (snapshot_.remoteControlRole !=
                        RemoteControlRole::kControlled ||
                    refreshRequest.roomId != snapshot_.sessionId ||
                    refreshRequest.senderDeviceId !=
                        snapshot_.peerDeviceId) {
                    return;
                }
                source = localDesktopCaptureSource_;
            }
            if (source) {
                source->RequestStartupFrameBurst(4);
            }
            BroadcastDirectSharedDisplayLayout();
            return;
        }
        SharedDisplayLayout layout;
        if (DecodeSharedDisplayLayout(payload, &layout)) {
            {
                std::lock_guard lock(mutex_);
                if (snapshot_.remoteControlRole !=
                        RemoteControlRole::kController ||
                    layout.roomId != snapshot_.sessionId ||
                    layout.senderDeviceId != snapshot_.peerDeviceId ||
                    layout.screenShareGeneration == 0 ||
                    layout.layoutVersion == 0 ||
                    layout.selectedDisplay.sessionDisplayId == 0) {
                    return;
                }
                snapshot_.directRemoteDisplay = layout.selectedDisplay;
                snapshot_.directRemoteDisplayLayoutVersion =
                    layout.layoutVersion;
                snapshot_.directRemoteScreenShareGeneration =
                    layout.screenShareGeneration;
            }
            PublishSnapshot();
            return;
        }
    }

    const bool fastChannel = label == kInputFastChannel;
    const bool reliableChannel = label == kControlReliableChannel;
    if (!fastChannel && !reliableChannel) {
        return;
    }
    RemoteInputEnvelope input;
    if (!DecodeRemoteInput(payload, &input)) {
        return;
    }
    const bool buttonTransition =
        input.event.type == RemoteInputMessageType::kMouseButton;
    if (!buttonTransition &&
        UsesFastInputChannel(input.event.type) != fastChannel) {
        return;
    }
    IRemoteInputSink* sink = nullptr;
    webrtc::scoped_refptr<WindowsDesktopCaptureSource> sourceToBoost;
    {
        std::lock_guard lock(mutex_);
        if (snapshot_.state != SessionEngineState::kActive ||
            snapshot_.remoteControlRole != RemoteControlRole::kControlled ||
            input.roomId != snapshot_.sessionId ||
            input.senderDeviceId != snapshot_.peerDeviceId ||
            input.controlGrantId != snapshot_.sessionId) {
            return;
        }
        if (IsPointerInput(input.event.type) &&
            (snapshot_.activeSharedDisplay.sessionDisplayId == 0 ||
             snapshot_.activeSharedDisplayLayoutVersion == 0 ||
             input.event.displayId !=
                 snapshot_.activeSharedDisplay.sessionDisplayId ||
             input.event.displayLayoutVersion !=
                 snapshot_.activeSharedDisplayLayoutVersion)) {
            return;
        }
        std::uint64_t& lastSequence = fastChannel
            ? directSession_.lastFastInputSequence
            : directSession_.lastReliableInputSequence;
        if (input.sequence <= lastSequence) {
            return;
        }
        lastSequence = input.sequence;
        if (buttonTransition ||
            input.event.type == RemoteInputMessageType::kReleaseAll) {
            if (input.sequence <= directSession_.lastPointerStateSequence) {
                return;
            }
            directSession_.lastPointerStateSequence = input.sequence;
        }
        input.event.deliverySequence = input.sequence;
        sink = remoteInputSink_;
        if (sink && ShouldBoostDesktopCaptureForInput(input.event)) {
            sourceToBoost = localDesktopCaptureSource_;
        }
    }
    if (sink) {
        sink->OnRemoteInput(input.event);
        cursorMonitor_.SetLastAppliedInputSequence(input.sequence);
        if (sourceToBoost) {
            sourceToBoost->NotifyRemoteInputActivity();
        }
    }
}

void InProcessSessionEngine::OnRemoteTrackAdded(
    const RemoteTrackInfo& track)
{
    (void)track;
}
