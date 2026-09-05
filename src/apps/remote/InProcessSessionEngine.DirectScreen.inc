// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

SessionCommandResult InProcessSessionEngine::SetDirectScreenStreamPreference(
    const ScreenStreamPreferenceRequest& preference)
{
    const bool originalSize = preference.maxWidth == 0 &&
        preference.maxHeight == 0;
    const bool boundedSize = preference.maxWidth > 0 &&
        preference.maxHeight > 0 &&
        preference.maxWidth <= kMaximumScreenStreamWidth &&
        preference.maxHeight <= kMaximumScreenStreamHeight;
    if ((!originalSize && !boundedSize) ||
        preference.framesPerSecond < kMinimumScreenFrameRate ||
        preference.framesPerSecond > kMaximumScreenFrameRate) {
        return Failure("screen_stream_preference_invalid",
                       "The requested screen stream preference is invalid.");
    }

    SessionControllerBase* controller = nullptr;
    ScreenStreamPreferenceRequest request = preference;
    {
        std::lock_guard lock(mutex_);
        if (!sessionController_ ||
            snapshot_.state != SessionEngineState::kActive ||
            snapshot_.purpose != SessionPurpose::kRemoteControl ||
            snapshot_.remoteControlRole != RemoteControlRole::kController ||
            snapshot_.sessionId.empty() ||
            !directSession_.IsChannelOpen(kControlReliableChannel)) {
            return Failure("direct_screen_stream_unavailable",
                           "The direct screen control channel is unavailable.");
        }
        if (request.framesPerSecond >
            snapshot_.directScreenMaximumFrameRate) {
            return Failure("screen_stream_frame_rate_unsupported",
                           "The remote capture backend does not support the requested frame rate.");
        }
        if (directSession_.nextScreenControlSequence ==
            std::numeric_limits<std::uint64_t>::max()) {
            return Failure("screen_stream_sequence_exhausted",
                           "The screen control sequence is exhausted.");
        }
        request.roomId = snapshot_.sessionId;
        request.senderDeviceId = snapshot_.localDeviceId;
        request.sequence = ++directSession_.nextScreenControlSequence;
        snapshot_.directScreenPreferencePending = true;
        snapshot_.directScreenPreferenceSequence = request.sequence;
        controller = sessionController_.get();
    }
    std::vector<std::uint8_t> encoded;
    std::string error;
    if (!EncodeScreenStreamPreferenceRequest(request, &encoded, &error)) {
        std::lock_guard lock(mutex_);
        if (snapshot_.directScreenPreferenceSequence == request.sequence) {
            snapshot_.directScreenPreferencePending = false;
        }
        return Failure("direct_screen_stream_encode_failed", error);
    }
    if (!controller->QueueData(
            std::string(kControlReliableChannel), encoded, true)) {
        std::lock_guard lock(mutex_);
        if (snapshot_.directScreenPreferenceSequence == request.sequence) {
            snapshot_.directScreenPreferencePending = false;
        }
        return Failure("direct_screen_stream_send_failed",
                       "The screen stream request could not be queued.");
    }
    PublishSnapshot();
    return Success();
}

SessionCommandResult InProcessSessionEngine::RequestDirectSharedDisplaySwitch(
    const std::string& stableDisplayKey)
{
    if (stableDisplayKey.empty()) {
        return Failure("shared_display_key_empty",
                       "A remote display must be selected.");
    }
    SessionControllerBase* controller = nullptr;
    SharedDisplaySwitchRequest request;
    {
        std::lock_guard lock(mutex_);
        if (!sessionController_ ||
            snapshot_.state != SessionEngineState::kActive ||
            snapshot_.remoteControlRole != RemoteControlRole::kController ||
            !snapshot_.directRemoteDisplayCatalogReported ||
            !directSession_.IsChannelOpen(kControlReliableChannel)) {
            return Failure("shared_display_switch_not_authorized",
                           "The direct remote display list is unavailable.");
        }
        const auto selected = std::find_if(
            snapshot_.directRemoteDisplays.begin(),
            snapshot_.directRemoteDisplays.end(),
            [&stableDisplayKey](const DisplayDescriptor& display) {
                return display.stableDisplayKey == stableDisplayKey;
            });
        if (selected == snapshot_.directRemoteDisplays.end()) {
            return Failure("shared_display_not_available",
                           "The selected remote display is no longer available.");
        }
        if (snapshot_.directRemoteDisplay.stableDisplayKey ==
            stableDisplayKey) {
            return Success();
        }
        if (directSession_.nextScreenControlSequence ==
            std::numeric_limits<std::uint64_t>::max()) {
            return Failure("shared_display_sequence_exhausted",
                           "The screen control sequence is exhausted.");
        }
        request.roomId = snapshot_.sessionId;
        request.senderDeviceId = snapshot_.localDeviceId;
        request.sequence = ++directSession_.nextScreenControlSequence;
        request.screenShareGeneration =
            snapshot_.directRemoteScreenShareGeneration;
        request.stableDisplayKey = stableDisplayKey;
        snapshot_.directRemoteDisplaySwitchPending = true;
        snapshot_.directRemoteDisplaySwitchSequence = request.sequence;
        snapshot_.directRemoteDisplaySwitchError.clear();
        controller = sessionController_.get();
    }
    std::vector<std::uint8_t> encoded;
    std::string error;
    if (!EncodeSharedDisplaySwitchRequest(request, &encoded, &error) ||
        !controller->QueueData(
            std::string(kControlReliableChannel), encoded, true)) {
        std::lock_guard lock(mutex_);
        if (snapshot_.directRemoteDisplaySwitchSequence ==
            request.sequence) {
            snapshot_.directRemoteDisplaySwitchPending = false;
        }
        return Failure("shared_display_switch_send_failed",
                       error.empty()
                           ? "The remote display switch request could not be queued."
                           : error);
    }
    PublishSnapshot();
    return Success();
}

void InProcessSessionEngine::BroadcastDirectSharedDisplayCatalog()
{
    SharedDisplayCatalog catalog;
    SessionControllerBase* controller = nullptr;
    {
        std::lock_guard lock(mutex_);
        if (!sessionController_ ||
            snapshot_.state != SessionEngineState::kActive ||
            snapshot_.remoteControlRole != RemoteControlRole::kControlled ||
            snapshot_.sessionId.empty() ||
            snapshot_.screenShareGeneration == 0 ||
            snapshot_.localDisplayTopology.layoutVersion == 0 ||
            snapshot_.localDisplayTopology.displays.empty() ||
            !directSession_.IsChannelOpen(kControlReliableChannel)) {
            return;
        }
        catalog.roomId = snapshot_.sessionId;
        catalog.senderDeviceId = snapshot_.localDeviceId;
        catalog.screenShareGeneration = snapshot_.screenShareGeneration;
        catalog.layoutVersion = snapshot_.localDisplayTopology.layoutVersion;
        catalog.displays = snapshot_.localDisplayTopology.displays;
        controller = sessionController_.get();
    }
    std::vector<std::uint8_t> encoded;
    if (EncodeSharedDisplayCatalog(catalog, &encoded)) {
        (void)controller->QueueData(
            std::string(kControlReliableChannel), encoded, true);
    }
}

SessionCommandResult InProcessSessionEngine::SwitchLocalDirectDisplay(
    const std::string& stableDisplayKey)
{
    const DisplayTopologySnapshot topology =
        EnumerateWindowsDisplayTopology();
    const DisplayDescriptor* selected =
        FindDisplayByStableKey(topology, stableDisplayKey);
    if (!selected) {
        return Failure("shared_display_not_available",
                       "The selected display is no longer attached.");
    }

    std::string sessionId;
    std::uint64_t nextGeneration = 0;
    std::uint64_t previousAdapterLuid = 0;
    std::uint32_t targetFrameRate = kDefaultScreenFrameRate;
    ScreenStreamPreferenceRequest preference;
    webrtc::scoped_refptr<WindowsDesktopCaptureSource> previousSource;
    webrtc::scoped_refptr<webrtc::MediaStreamTrackInterface> previousTrack;
    LibWebRtcSession* session = nullptr;
    {
        std::lock_guard lock(mutex_);
        if (!webRtcSession_ || !directSession_.mediaSlotsPrepared ||
            snapshot_.state != SessionEngineState::kActive ||
            snapshot_.remoteControlRole != RemoteControlRole::kControlled ||
            !localDesktopCaptureSource_) {
            return Failure("local_screen_share_not_active",
                           "The direct desktop capture is not active.");
        }
        if (snapshot_.activeSharedDisplay.stableDisplayKey ==
            stableDisplayKey) {
            return Success();
        }
        sessionId = snapshot_.sessionId;
        nextGeneration = localScreenShareGeneration_ + 1;
        previousSource = localDesktopCaptureSource_;
        previousTrack = directSession_.screenTrack;
        previousAdapterLuid = previousSource->CaptureTarget().adapterLuid;
        session = webRtcSession_.get();
        if (directSession_.screenPreferenceApplied) {
            preference = directSession_.screenPreference;
            targetFrameRate = preference.framesPerSecond;
        }
    }

    auto replacement =
        webrtc::make_ref_counted<WindowsDesktopCaptureSource>(
            options_.desktopCaptureImplementation, *selected);
    if (!replacement->SetTargetFrameRate(targetFrameRate) ||
        !replacement->StartCapture()) {
        const std::string error = replacement->LastError();
        replacement->StopCapture();
        return Failure("shared_display_capture_start_failed",
                       error.empty()
                           ? "The selected display could not be captured."
                           : error);
    }
    const auto factory = runtime_ ? runtime_->PeerConnectionFactory()
                                  : nullptr;
    auto replacementTrack = factory ? factory->CreateVideoTrack(
        replacement,
        "direct-screen-switch-" + Snapshot().localDeviceId + "-" +
            std::to_string(nextGeneration)) : nullptr;
    if (!replacementTrack) {
        replacement->StopCapture();
        return Failure("direct_video_track_create_failed",
                       "The replacement desktop track could not be created.");
    }

    if (runtime_) {
        runtime_->SetDesktopCaptureAdapterLuid(selected->adapterLuid);
    }
    auto result = session->SetVideoSlotSendingActive(
        kScreenMainVideoSlot, false);
    if (result.ok()) {
        result = session->SetVideoSlotTrack(
            kScreenMainVideoSlot, replacementTrack);
    }
    const auto effective = ResolveScreenPolicy(
        replacement->CapturedWidth(), replacement->CapturedHeight(),
        preference);
    if (result.ok()) {
        result = session->SetVideoSlotEncodingPolicy(
            kScreenMainVideoSlot, effective.framesPerSecond,
            effective.width, effective.height);
    }
    if (result.ok()) {
        result = session->SetVideoSlotSendingActive(
            kScreenMainVideoSlot, true);
    }
    if (!result.ok()) {
        if (runtime_) {
            runtime_->SetDesktopCaptureAdapterLuid(previousAdapterLuid);
        }
        (void)session->SetVideoSlotTrack(
            kScreenMainVideoSlot, previousTrack);
        (void)session->SetVideoSlotSendingActive(
            kScreenMainVideoSlot, true);
        replacement->StopCapture();
        return Failure("shared_display_track_replace_failed",
                       std::string(result.message()));
    }

    bool stillCurrent = false;
    {
        std::lock_guard lock(mutex_);
        stillCurrent = webRtcSession_.get() == session &&
            snapshot_.sessionId == sessionId &&
            snapshot_.state == SessionEngineState::kActive &&
            snapshot_.remoteControlRole == RemoteControlRole::kControlled &&
            localDesktopCaptureSource_.get() == previousSource.get();
        if (stillCurrent) {
            localDesktopCaptureSource_ = replacement;
            directSession_.screenTrack = replacementTrack;
            localScreenShareGeneration_ = nextGeneration;
            snapshot_.screenShareGeneration = nextGeneration;
            snapshot_.localDisplayTopology = topology;
            snapshot_.selectedLocalDisplayKey = selected->stableDisplayKey;
            snapshot_.activeSharedDisplay = *selected;
            snapshot_.activeSharedDisplayLayoutVersion =
                topology.layoutVersion;
            if (runtime_) {
                runtime_->SetDesktopShareGeneration(nextGeneration);
            }
        }
    }
    if (!stillCurrent) {
        (void)session->SetVideoSlotTrack(
            kScreenMainVideoSlot, previousTrack);
        (void)session->SetVideoSlotSendingActive(
            kScreenMainVideoSlot, true);
        replacement->StopCapture();
        return Failure("shared_display_switch_superseded",
                       "The direct session changed while switching displays.");
    }
    StartRemoteCursorPublishing(*selected, topology.layoutVersion);
    replacement->RequestStartupFrameBurst();
    std::jthread stopThread(
        [capture = std::move(previousSource)](std::stop_token) {
            capture->StopCapture();
        });
    {
        std::lock_guard lock(mutex_);
        retiredDesktopStopThreads_.push_back(std::move(stopThread));
    }
    PublishSnapshot();
    BroadcastDirectSharedDisplayLayout();
    BroadcastDirectSharedDisplayCatalog();
    return Success();
}

bool InProcessSessionEngine::DispatchDirectScreenData(
    const std::string& label,
    std::span<const std::uint8_t> payload)
{
    if (label != kControlReliableChannel) {
        return false;
    }

    SharedDisplayCatalog catalog;
    if (DecodeSharedDisplayCatalog(payload, &catalog)) {
        {
            std::lock_guard lock(mutex_);
            if (snapshot_.state != SessionEngineState::kActive ||
                snapshot_.remoteControlRole != RemoteControlRole::kController ||
                catalog.roomId != snapshot_.sessionId ||
                catalog.senderDeviceId != snapshot_.peerDeviceId ||
                catalog.screenShareGeneration == 0) {
                return true;
            }
            snapshot_.directRemoteDisplayCatalogReported = true;
            snapshot_.directRemoteDisplayCatalogLayoutVersion =
                catalog.layoutVersion;
            snapshot_.directRemoteScreenShareGeneration =
                catalog.screenShareGeneration;
            snapshot_.directRemoteDisplays = std::move(catalog.displays);
        }
        PublishSnapshot();
        return true;
    }

    SharedDisplaySwitchApplied switchApplied;
    if (DecodeSharedDisplaySwitchApplied(payload, &switchApplied)) {
        {
            std::lock_guard lock(mutex_);
            if (switchApplied.roomId != snapshot_.sessionId ||
                switchApplied.senderDeviceId != snapshot_.peerDeviceId ||
                !snapshot_.directRemoteDisplaySwitchPending ||
                switchApplied.requestSequence !=
                    snapshot_.directRemoteDisplaySwitchSequence) {
                return true;
            }
            snapshot_.directRemoteDisplaySwitchPending = false;
            snapshot_.directRemoteDisplaySwitchError =
                switchApplied.accepted ? std::string{} : switchApplied.error;
        }
        PublishSnapshot();
        return true;
    }

    SharedDisplaySwitchRequest switchRequest;
    if (DecodeSharedDisplaySwitchRequest(payload, &switchRequest)) {
        SessionControllerBase* controller = nullptr;
        bool authorized = false;
        {
            std::lock_guard lock(mutex_);
            if (switchRequest.roomId != snapshot_.sessionId ||
                switchRequest.senderDeviceId != snapshot_.peerDeviceId ||
                switchRequest.sequence <=
                    directSession_.lastScreenControlSequence) {
                return true;
            }
            directSession_.lastScreenControlSequence =
                switchRequest.sequence;
            authorized = sessionController_ &&
                snapshot_.state == SessionEngineState::kActive &&
                snapshot_.remoteControlRole == RemoteControlRole::kControlled &&
                switchRequest.screenShareGeneration ==
                    snapshot_.screenShareGeneration;
            controller = sessionController_.get();
        }
        const auto result = authorized
            ? SwitchLocalDirectDisplay(switchRequest.stableDisplayKey)
            : Failure("shared_display_switch_not_authorized",
                      "The direct display switch request is stale or unauthorized.");
        SharedDisplaySwitchApplied response;
        response.roomId = switchRequest.roomId;
        response.senderDeviceId = Snapshot().localDeviceId;
        response.requestSequence = switchRequest.sequence;
        response.screenShareGeneration =
            switchRequest.screenShareGeneration;
        response.stableDisplayKey = switchRequest.stableDisplayKey;
        response.accepted = result.accepted;
        response.error = result.accepted ? std::string{} : result.errorMessage;
        std::vector<std::uint8_t> encoded;
        if (controller && EncodeSharedDisplaySwitchApplied(response, &encoded)) {
            (void)controller->QueueData(
                std::string(kControlReliableChannel), encoded, true);
        }
        return true;
    }

    ScreenCaptureCapability capability;
    if (DecodeScreenCaptureCapability(payload, &capability)) {
        {
            std::lock_guard lock(mutex_);
            if (capability.roomId != snapshot_.sessionId ||
                capability.senderDeviceId != snapshot_.peerDeviceId) {
                return true;
            }
            snapshot_.directScreenMaximumFrameRate =
                capability.maximumFrameRate;
        }
        PublishSnapshot();
        return true;
    }

    ScreenStreamPreferenceApplied applied;
    if (DecodeScreenStreamPreferenceAppliedV2(payload, &applied) ||
        DecodeScreenStreamPreferenceApplied(payload, &applied)) {
        {
            std::lock_guard lock(mutex_);
            if (applied.roomId != snapshot_.sessionId ||
                applied.senderDeviceId != snapshot_.peerDeviceId ||
                !snapshot_.directScreenPreferencePending ||
                applied.requestSequence !=
                    snapshot_.directScreenPreferenceSequence ||
                (applied.screenShareGeneration != 0 &&
                 snapshot_.directRemoteScreenShareGeneration != 0 &&
                 applied.screenShareGeneration !=
                    snapshot_.directRemoteScreenShareGeneration)) {
                return true;
            }
            snapshot_.directScreenPreferencePending = false;
            if (applied.accepted) {
                snapshot_.directScreenWidth = applied.width;
                snapshot_.directScreenHeight = applied.height;
                snapshot_.directScreenFramesPerSecond =
                    applied.framesPerSecond;
                snapshot_.directScreenMaxBitrateBps =
                    applied.maxBitrateBps;
            } else {
                snapshot_.errorCode = "direct_screen_preference_rejected";
                snapshot_.errorMessage = applied.error;
            }
        }
        PublishSnapshot();
        return true;
    }

    ScreenStreamPreferenceRequest request;
    if (DecodeScreenStreamPreferenceRequest(payload, &request)) {
        WindowsDesktopCaptureSource* source = nullptr;
        LibWebRtcSession* session = nullptr;
        SessionControllerBase* controller = nullptr;
        std::uint64_t generation = 0;
        std::uint32_t maximumFrameRate = kMaximumScreenFrameRate;
        bool authorized = false;
        {
            std::lock_guard lock(mutex_);
            if (request.roomId != snapshot_.sessionId ||
                request.senderDeviceId != snapshot_.peerDeviceId ||
                request.sequence <= directSession_.lastScreenControlSequence) {
                return true;
            }
            directSession_.lastScreenControlSequence = request.sequence;
            authorized = sessionController_ && webRtcSession_ &&
                snapshot_.state == SessionEngineState::kActive &&
                snapshot_.remoteControlRole == RemoteControlRole::kControlled &&
                directSession_.mediaSlotsPrepared &&
                localDesktopCaptureSource_;
            source = localDesktopCaptureSource_.get();
            session = webRtcSession_.get();
            controller = sessionController_.get();
            generation = snapshot_.screenShareGeneration;
            maximumFrameRate = MaximumDesktopCaptureFrameRate(
                options_.desktopCaptureImplementation, source);
        }
        ScreenStreamPreferenceApplied response;
        response.roomId = request.roomId;
        response.senderDeviceId = Snapshot().localDeviceId;
        response.requestSequence = request.sequence;
        response.screenShareGeneration = generation;
        if (!authorized || request.framesPerSecond > maximumFrameRate) {
            response.error = "The direct screen preference is unavailable or unsupported.";
        } else {
            const auto effective = ResolveScreenPolicy(
                source->CapturedWidth(), source->CapturedHeight(), request);
            const bool captureAccepted =
                source->SetTargetFrameRate(effective.framesPerSecond);
            const auto policy = captureAccepted
                ? session->SetVideoSlotEncodingPolicy(
                      kScreenMainVideoSlot, effective.framesPerSecond,
                      effective.width, effective.height)
                : webrtc::RTCError(webrtc::RTCErrorType::INVALID_STATE,
                                   "The desktop capturer rejected the frame rate.");
            response.accepted = policy.ok();
            response.width = effective.width;
            response.height = effective.height;
            response.framesPerSecond = effective.framesPerSecond;
            response.maxBitrateBps = effective.maxBitrateBps;
            response.scaleBackend = ScreenScaleBackend::kWebRtc;
            response.error = policy.ok() ? std::string{}
                                         : std::string(policy.message());
            if (policy.ok()) {
                std::lock_guard lock(mutex_);
                directSession_.screenPreferenceApplied = true;
                directSession_.screenPreference = request;
            }
        }
        ScreenCaptureCapability captureCapability;
        captureCapability.roomId = request.roomId;
        captureCapability.senderDeviceId = response.senderDeviceId;
        captureCapability.maximumFrameRate = maximumFrameRate;
        std::vector<std::uint8_t> encoded;
        if (controller && EncodeScreenCaptureCapability(
                captureCapability, &encoded)) {
            (void)controller->QueueData(
                std::string(kControlReliableChannel), encoded, true);
        }
        if (controller && EncodeScreenStreamPreferenceAppliedV2(
                response, &encoded)) {
            (void)controller->QueueData(
                std::string(kControlReliableChannel), encoded, true);
        }
        if (controller && EncodeScreenStreamPreferenceApplied(
                response, &encoded)) {
            (void)controller->QueueData(
                std::string(kControlReliableChannel), encoded, true);
        }
        return true;
    }
    return false;
}
