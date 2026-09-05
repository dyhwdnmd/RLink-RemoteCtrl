// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

std::optional<OperationError> InProcessSessionEngine::PrepareDirectMedia(
    bool bindNegotiatedSlots)
{
    LibWebRtcSession* session = nullptr;
    webrtc::scoped_refptr<webrtc::MediaStreamTrackInterface> track;
    webrtc::scoped_refptr<webrtc::AudioTrackInterface> microphoneTrack;
    DisplayDescriptor captureTarget;
    DisplayTopologySnapshot captureTopology;
    std::string localDeviceId;
    std::string sessionId;
    bool publishDesktop = false;
    std::uint64_t shareGeneration = 0;
    bool publishLayout = false;
    bool remoteAudioEnabled = true;
    {
        std::lock_guard lock(mutex_);
        if (!webRtcSession_ || !sessionController_ ||
            snapshot_.purpose != SessionPurpose::kRemoteControl ||
            snapshot_.sessionId.empty()) {
            return OperationError{
                "direct_media_session_unavailable",
                "The direct remote-control session is not ready for media."};
        }
        if (directSession_.mediaSlotsPrepared) {
            return std::nullopt;
        }
        if (directSession_.mediaSlotsPreparing) {
            return OperationError{
                "direct_media_prepare_in_progress",
                "The direct media slot is already being prepared."};
        }
        directSession_.mediaSlotsPreparing = true;
        session = webRtcSession_.get();
        publishDesktop =
            snapshot_.remoteControlRole == RemoteControlRole::kControlled;
        localDeviceId = snapshot_.localDeviceId;
        sessionId = snapshot_.sessionId;
        microphoneTrack = localMicrophoneAudioTrack_;
        remoteAudioEnabled = !snapshot_.roomAudioPlaybackMuted;
        const auto idle = idleRoomVideoTracks_.find(kScreenMainVideoSlot);
        if (idle != idleRoomVideoTracks_.end()) {
            track = idle->second;
        }
        if (publishDesktop) {
            captureTopology = snapshot_.localDisplayTopology;
            const auto* selected = FindDisplayByStableKey(
                captureTopology, snapshot_.selectedLocalDisplayKey);
            if (!selected) {
                selected = FindPrimaryDisplay(captureTopology);
            }
            if (selected) {
                captureTarget = *selected;
            }
            shareGeneration = ++localScreenShareGeneration_;
            snapshot_.screenShareGeneration = shareGeneration;
            if (runtime_) {
                runtime_->SetDesktopShareGeneration(shareGeneration);
                runtime_->SetDesktopCaptureAdapterLuid(
                    captureTarget.adapterLuid);
            }
        }
    }

    webrtc::scoped_refptr<WindowsDesktopCaptureSource> source;
    const auto fail = [this, session](std::string code,
                                     std::string message,
                                     webrtc::scoped_refptr<
                                         WindowsDesktopCaptureSource> source)
        -> std::optional<OperationError> {
        if (source) {
            source->StopCapture();
        }
        std::lock_guard lock(mutex_);
        if (webRtcSession_.get() == session) {
            directSession_.mediaSlotsPreparing = false;
        }
        return OperationError{std::move(code), std::move(message)};
    };

    if (publishDesktop && options_.enableRealDesktopCapture) {
        if (captureTarget.sessionDisplayId == 0 ||
            captureTopology.layoutVersion == 0) {
            return fail("direct_display_not_available",
                        "The selected display is no longer attached.",
                        nullptr);
        }
        source = webrtc::make_ref_counted<WindowsDesktopCaptureSource>(
            options_.desktopCaptureImplementation, captureTarget);
        if (!source->SetTargetFrameRate(kDefaultScreenFrameRate)) {
            return fail("direct_capture_frame_rate_rejected",
                        "The desktop capturer rejected the default frame rate.",
                        source);
        }
        if (!source->StartCapture()) {
            return fail("direct_desktop_capture_start_failed",
                        source->LastError(), source);
        }
        const auto factory = runtime_->PeerConnectionFactory();
        track = factory ? factory->CreateVideoTrack(
            source,
            "direct-screen-" + localDeviceId + "-" +
                std::to_string(shareGeneration)) : nullptr;
        if (!track) {
            return fail("direct_video_track_create_failed",
                        "The direct desktop video track could not be created.",
                        source);
        }
    }
    if (!track) {
        return fail("idle_video_track_unavailable",
                    "The direct video placeholder is unavailable.", source);
    }

    webrtc::RTCError result;
    if (bindNegotiatedSlots) {
        result = session->BindNegotiatedVideoTransceiverSlots(
            {kScreenMainVideoSlot});
        if (result.ok()) {
            result = session->BindNegotiatedAudioTransceiverSlot(
                kMicrophoneMainAudioSlot);
        }
    } else {
        result = session->PrepareVideoTransceiverSlot(
            kScreenMainVideoSlot);
        if (result.ok()) {
            result = session->PrepareAudioTransceiverSlot(
                kMicrophoneMainAudioSlot);
        }
    }
    if (!result.ok()) {
        return fail("direct_media_slot_prepare_failed",
                    std::string(result.message()), source);
    }
    result = session->SetVideoSlotSendingActive(
        kScreenMainVideoSlot, false);
    if (result.ok()) {
        result = session->SetVideoSlotTrack(kScreenMainVideoSlot, track);
    }
    if (result.ok() && source) {
        const std::uint32_t width = captureTarget.width > 0
            ? captureTarget.width : source->CapturedWidth();
        const std::uint32_t height = captureTarget.height > 0
            ? captureTarget.height : source->CapturedHeight();
        result = session->SetVideoSlotEncodingPolicy(
            kScreenMainVideoSlot, kDefaultScreenFrameRate, width, height);
    }
    if (result.ok() && source) {
        source->RequestStartupFrameBurst(12);
        result = session->SetVideoSlotSendingActive(
            kScreenMainVideoSlot, true);
    }
    if (result.ok()) {
        result = session->SetAudioSlotTrack(
            kMicrophoneMainAudioSlot, microphoneTrack);
    }
    if (result.ok()) {
        session->SetRemoteAudioSlotEnabled(
            kMicrophoneMainAudioSlot, remoteAudioEnabled);
    }
    if (!result.ok()) {
        return fail("direct_video_track_attach_failed",
                    std::string(result.message()), source);
    }

    {
        std::lock_guard lock(mutex_);
        if (webRtcSession_.get() != session ||
            snapshot_.sessionId != sessionId) {
            directSession_.mediaSlotsPreparing = false;
            if (source) {
                (void)session->SetVideoSlotSendingActive(
                    kScreenMainVideoSlot, false);
            }
            return OperationError{
                "direct_session_changed_during_media_prepare",
                "The direct session changed while its media was prepared."};
        }
        directSession_.mediaSlotsPreparing = false;
        directSession_.mediaSlotsPrepared = true;
        snapshot_.directMediaSlotsPrepared = true;
        directSession_.screenTrack = track;
        if (publishDesktop &&
            captureTarget.sessionDisplayId != 0 &&
            captureTopology.layoutVersion != 0) {
            if (source) {
                localDesktopCaptureSource_ = source;
            }
            snapshot_.activeSharedDisplay = captureTarget;
            snapshot_.activeSharedDisplayLayoutVersion =
                captureTopology.layoutVersion;
            publishLayout = directSession_.IsChannelOpen(
                kControlReliableChannel);
        }
    }
    if (publishLayout) {
        BroadcastDirectSharedDisplayLayout();
        BroadcastDirectSharedDisplayCatalog();
    }
    if (source) {
        StartRemoteCursorPublishing(
            captureTarget, captureTopology.layoutVersion);
    }
    return std::nullopt;
}

SessionCommandResult InProcessSessionEngine::SetDirectRemoteVideoSink(
    webrtc::VideoSinkInterface<webrtc::VideoFrame>* sink)
{
    LibWebRtcSession* session = nullptr;
    {
        std::lock_guard lock(mutex_);
        if (!webRtcSession_ || !directSession_.mediaSlotsPrepared ||
            snapshot_.purpose != SessionPurpose::kRemoteControl) {
            return Failure("direct_media_not_ready",
                           "The direct desktop video is not ready.");
        }
        session = webRtcSession_.get();
    }
    session->SetRemoteVideoSlotSink(kScreenMainVideoSlot, sink);
    if (sink) {
        RequestDirectSharedDisplayLayout();
    }
    return Success();
}

void InProcessSessionEngine::StopDirectDesktopCapture()
{
    StopRemoteCursorPublishing();
    webrtc::scoped_refptr<WindowsDesktopCaptureSource> source;
    LibWebRtcSession* session = nullptr;
    {
        std::lock_guard lock(mutex_);
        source = std::exchange(localDesktopCaptureSource_, nullptr);
        directSession_.screenTrack = nullptr;
        directSession_.mediaSlotsPreparing = false;
        directSession_.mediaSlotsPrepared = false;
        snapshot_.directMediaSlotsPrepared = false;
        snapshot_.activeSharedDisplay = {};
        snapshot_.activeSharedDisplayLayoutVersion = 0;
        session = webRtcSession_.get();
    }
    if (session) {
        (void)session->SetVideoSlotSendingActive(
            kScreenMainVideoSlot, false);
    }
    if (source) {
        std::jthread stopThread(
            [capture = std::move(source)](std::stop_token) {
                capture->StopCapture();
            });
        std::lock_guard lock(mutex_);
        retiredDesktopStopThreads_.push_back(std::move(stopThread));
    }
}

void InProcessSessionEngine::StopDirectMicrophoneCapture()
{
    webrtc::scoped_refptr<webrtc::AudioTrackInterface> track;
    LibWebRtcSession* session = nullptr;
    {
        std::lock_guard lock(mutex_);
        track = std::exchange(localMicrophoneAudioTrack_, nullptr);
        localMicrophoneAudioSource_ = nullptr;
        snapshot_.localMicrophone = LocalMicrophoneState::kOff;
        session = webRtcSession_.get();
    }
    if (track) {
        track->set_enabled(false);
    }
    if (session && session->AudioSlotPrepared(kMicrophoneMainAudioSlot)) {
        (void)session->SetAudioSlotTrack(
            kMicrophoneMainAudioSlot, nullptr);
    }
    if (runtime_) {
        (void)runtime_->SetRecordingEnabled(false);
    }
}

void InProcessSessionEngine::BroadcastDirectSharedDisplayLayout()
{
    SharedDisplayLayout layout;
    SessionControllerBase* controller = nullptr;
    {
        std::lock_guard lock(mutex_);
        if (!sessionController_ ||
            snapshot_.remoteControlRole != RemoteControlRole::kControlled ||
            snapshot_.sessionId.empty() ||
            snapshot_.activeSharedDisplay.sessionDisplayId == 0 ||
            snapshot_.activeSharedDisplayLayoutVersion == 0 ||
            !directSession_.IsChannelOpen(kControlReliableChannel)) {
            return;
        }
        layout.roomId = snapshot_.sessionId;
        layout.senderDeviceId = snapshot_.localDeviceId;
        layout.screenShareGeneration = snapshot_.screenShareGeneration;
        layout.layoutVersion = snapshot_.activeSharedDisplayLayoutVersion;
        layout.selectedDisplay = snapshot_.activeSharedDisplay;
        controller = sessionController_.get();
    }
    std::vector<std::uint8_t> encoded;
    if (EncodeSharedDisplayLayout(layout, &encoded)) {
        (void)controller->QueueData(
            std::string(kControlReliableChannel), encoded, true);
    }
}

void InProcessSessionEngine::RequestDirectSharedDisplayLayout()
{
    ScreenRefreshRequest request;
    SessionControllerBase* controller = nullptr;
    {
        std::lock_guard lock(mutex_);
        if (!sessionController_ ||
            snapshot_.remoteControlRole != RemoteControlRole::kController ||
            snapshot_.sessionId.empty() ||
            !directSession_.IsChannelOpen(kControlReliableChannel) ||
            directSession_.nextScreenControlSequence ==
                std::numeric_limits<std::uint64_t>::max()) {
            return;
        }
        request.roomId = snapshot_.sessionId;
        request.senderDeviceId = snapshot_.localDeviceId;
        request.sequence = ++directSession_.nextScreenControlSequence;
        // The direct controller does not know the remote generation until it
        // receives the layout. Generation 1 is a non-zero discovery marker;
        // the controlled peer validates the authenticated session identity
        // and responds with its authoritative current generation.
        request.screenShareGeneration = 1;
        controller = sessionController_.get();
    }
    std::vector<std::uint8_t> encoded;
    if (EncodeScreenRefreshRequest(request, &encoded)) {
        (void)controller->QueueData(
            std::string(kControlReliableChannel), encoded, true);
    }
}

SessionCommandResult InProcessSessionEngine::SendDirectInput(
    const RemoteInputEvent& event)
{
    const bool fast = UsesFastInputChannel(event.type);
    const bool reliableButtonReleaseFallback =
        event.type == RemoteInputMessageType::kMouseButton && !event.pressed;
    const std::string channel = fast
        ? std::string(kInputFastChannel)
        : std::string(kControlReliableChannel);
    SessionControllerBase* controller = nullptr;
    RemoteInputEnvelope envelope;
    bool reliableOpen = false;
    {
        std::lock_guard lock(mutex_);
        if (!sessionController_ ||
            snapshot_.state != SessionEngineState::kActive ||
            snapshot_.purpose != SessionPurpose::kRemoteControl ||
            snapshot_.remoteControlRole != RemoteControlRole::kController ||
            snapshot_.sessionId.empty()) {
            return Failure("direct_input_not_authorized",
                           "The direct session is not controlling a peer.");
        }
        if (IsPointerInput(event.type) &&
            (snapshot_.directRemoteDisplay.sessionDisplayId == 0 ||
             snapshot_.directRemoteDisplayLayoutVersion == 0 ||
             event.displayId !=
                 snapshot_.directRemoteDisplay.sessionDisplayId ||
             event.displayLayoutVersion !=
                 snapshot_.directRemoteDisplayLayoutVersion)) {
            return Failure("direct_input_display_layout_stale",
                           "The remote display layout is not ready or changed.");
        }
        if (!directSession_.IsChannelOpen(channel)) {
            return Failure("direct_input_channel_not_open",
                           "The required direct input channel is not open.");
        }
        if (directSession_.nextInputSequence ==
            std::numeric_limits<std::uint64_t>::max()) {
            return Failure("direct_input_sequence_exhausted",
                           "The direct input sequence is exhausted.");
        }
        controller = sessionController_.get();
        reliableOpen = directSession_.IsChannelOpen(
            kControlReliableChannel);
        envelope.roomId = snapshot_.sessionId;
        envelope.senderDeviceId = snapshot_.localDeviceId;
        envelope.controlGrantId = snapshot_.sessionId;
        envelope.sequence = ++directSession_.nextInputSequence;
        envelope.timestampUs = static_cast<std::uint64_t>(
            std::chrono::duration_cast<std::chrono::microseconds>(
                std::chrono::steady_clock::now().time_since_epoch()).count());
        envelope.event = event;
    }
    std::vector<std::uint8_t> encoded;
    if (!EncodeRemoteInput(envelope, &encoded)) {
        return Failure("direct_input_encode_failed",
                       "The direct input event could not be encoded.");
    }
    const auto queuePacket =
        [controller, &encoded](const std::string& targetChannel,
                               bool fastChannel) {
            auto& telemetry = RemoteInputTelemetry::Instance();
            telemetry.RecordPacketQueued(fastChannel);
            const bool queued = controller->QueueData(
                targetChannel, encoded, true,
                [](SendResult result) {
                    RemoteInputTelemetry::Instance()
                        .RecordPacketSendResult(
                            result == SendResult::kSent);
                });
            if (!queued) {
                telemetry.RecordPacketSendResult(false);
            }
            return queued;
        };
    if (event.type == RemoteInputMessageType::kMouseButton && fast) {
        (void)queuePacket(std::string(kInputFastChannel), true);
    }
    const bool primaryQueued = queuePacket(channel, fast);
    bool reliableFallbackQueued = false;
    if (reliableButtonReleaseFallback && reliableOpen) {
        // A failed fast enqueue must not suppress the reliable button-up
        // fallback; otherwise the controlled Windows session can retain a
        // logically pressed mouse button until somebody uses the physical
        // mouse locally.
        reliableFallbackQueued = queuePacket(
            std::string(kControlReliableChannel), false);
    }
    return primaryQueued || reliableFallbackQueued
        ? Success()
        : Failure("direct_input_send_queue_failed",
                  "The direct input event could not be queued.");
}
