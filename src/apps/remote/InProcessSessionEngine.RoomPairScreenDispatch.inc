// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

        SharedDisplayLayout sharedDisplay;
        if (DecodeSharedDisplayLayout(payload, &sharedDisplay)) {
            {
                std::lock_guard lock(mutex_);
                const auto pairIt = roomPairs_.find(pairId);
                auto snapshotIt = std::find_if(
                    snapshot_.roomPeerConnections.begin(),
                    snapshot_.roomPeerConnections.end(),
                    [&pairId](const auto& current) {
                        return current.pairId == pairId;
                    });
                if (pairIt == roomPairs_.end() ||
                    snapshotIt == snapshot_.roomPeerConnections.end() ||
                    sharedDisplay.roomId != snapshot_.room.roomId ||
                    sharedDisplay.senderDeviceId !=
                        pairIt->second->peerDeviceId ||
                    snapshot_.room.screenSharerDeviceId !=
                        sharedDisplay.senderDeviceId ||
                    sharedDisplay.screenShareGeneration !=
                        snapshot_.room.screenShareEpoch) {
                    return;
                }
                snapshotIt->sharedDisplayLayoutVersion =
                    sharedDisplay.layoutVersion;
                snapshotIt->sharedDisplayId =
                    sharedDisplay.selectedDisplay.sessionDisplayId;
                snapshotIt->sharedDisplayName =
                    sharedDisplay.selectedDisplay.friendlyName;
                snapshotIt->sharedDisplayStableKey =
                    sharedDisplay.selectedDisplay.stableDisplayKey;
                snapshotIt->sharedDisplayWidth =
                    sharedDisplay.selectedDisplay.width;
                snapshotIt->sharedDisplayHeight =
                    sharedDisplay.selectedDisplay.height;
            }
            PublishSnapshot();
            return;
        }

        ScreenCaptureRuntimeCapability runtimeCapability;
        if (DecodeScreenCaptureRuntimeCapability(
                payload, &runtimeCapability)) {
            {
                std::lock_guard lock(mutex_);
                const auto pairIt = roomPairs_.find(pairId);
                auto snapshotIt = std::find_if(
                    snapshot_.roomPeerConnections.begin(),
                    snapshot_.roomPeerConnections.end(),
                    [&pairId](const auto& current) {
                        return current.pairId == pairId;
                    });
                if (pairIt == roomPairs_.end() ||
                    snapshotIt == snapshot_.roomPeerConnections.end() ||
                    runtimeCapability.roomId != snapshot_.room.roomId ||
                    runtimeCapability.senderDeviceId !=
                        pairIt->second->peerDeviceId ||
                    snapshot_.room.screenSharerDeviceId !=
                        runtimeCapability.senderDeviceId) {
                    return;
                }
                snapshotIt->screenMaximumFrameRate =
                    runtimeCapability.maximumFrameRate;
                snapshotIt->screenCaptureCapabilityReported = true;
                snapshotIt->screenCaptureConfiguredBackend =
                    runtimeCapability.configuredBackend;
                snapshotIt->screenCaptureActiveBackend =
                    runtimeCapability.activeBackend;
                snapshotIt->screenCaptureFallbackReason =
                    runtimeCapability.fallbackReason;
            }
            PublishSnapshot();
            return;
        }

        ScreenCaptureCapability captureCapability;
        if (DecodeScreenCaptureCapability(payload, &captureCapability)) {
            {
                std::lock_guard lock(mutex_);
                const auto pairIt = roomPairs_.find(pairId);
                auto snapshotIt = std::find_if(
                    snapshot_.roomPeerConnections.begin(),
                    snapshot_.roomPeerConnections.end(),
                    [&pairId](const auto& current) {
                        return current.pairId == pairId;
                    });
                if (pairIt == roomPairs_.end() ||
                    snapshotIt == snapshot_.roomPeerConnections.end() ||
                    captureCapability.roomId != snapshot_.room.roomId ||
                    captureCapability.senderDeviceId !=
                        pairIt->second->peerDeviceId ||
                    snapshot_.room.screenSharerDeviceId !=
                        captureCapability.senderDeviceId) {
                    return;
                }
                snapshotIt->screenMaximumFrameRate =
                    captureCapability.maximumFrameRate;
            }
            PublishSnapshot();
            return;
        }

        ScreenStreamPreferenceApplied applied;
        const bool generationBoundPreferenceResult =
            DecodeScreenStreamPreferenceAppliedV2(payload, &applied);
        if (generationBoundPreferenceResult ||
            DecodeScreenStreamPreferenceApplied(payload, &applied)) {
            {
                std::lock_guard lock(mutex_);
                const auto pairIt = roomPairs_.find(pairId);
                auto snapshotIt = std::find_if(
                    snapshot_.roomPeerConnections.begin(),
                    snapshot_.roomPeerConnections.end(),
                    [&pairId](const auto& current) {
                        return current.pairId == pairId;
                    });
                if (pairIt == roomPairs_.end() ||
                    snapshotIt == snapshot_.roomPeerConnections.end() ||
                    applied.roomId != snapshot_.room.roomId ||
                    applied.senderDeviceId != pairIt->second->peerDeviceId ||
                    !snapshotIt->screenPreferencePending ||
                    applied.requestSequence !=
                        snapshotIt->screenPreferenceSequence ||
                    snapshotIt->screenPreferenceGeneration == 0 ||
                    snapshotIt->screenPreferenceGeneration !=
                        snapshot_.room.screenShareEpoch ||
                    (generationBoundPreferenceResult &&
                     applied.screenShareGeneration !=
                         snapshotIt->screenPreferenceGeneration)) {
                    return;
                }
                snapshotIt->screenPreferencePending = false;
                if (applied.accepted) {
                    snapshotIt->screenWidth = applied.width;
                    snapshotIt->screenHeight = applied.height;
                    snapshotIt->screenFramesPerSecond =
                        applied.framesPerSecond;
                    snapshotIt->screenMaxBitrateBps =
                        applied.maxBitrateBps;
                    snapshotIt->screenScaleBackend =
                        static_cast<std::uint8_t>(applied.scaleBackend);
                    snapshotIt->errorCode.clear();
                    snapshotIt->errorMessage.clear();
                } else {
                    snapshotIt->errorCode =
                        "screen_stream_preference_rejected";
                    snapshotIt->errorMessage = applied.error;
                }
            }
            PublishSnapshot();
            return;
        }

        ScreenStreamPreferenceRequest streamRequest;
        bool decodedStreamRequest = DecodeScreenStreamPreferenceRequest(
            payload, &streamRequest);
        if (!decodedStreamRequest) {
            ScreenFrameRateRequest legacyRequest;
            if (DecodeScreenFrameRateRequest(payload, &legacyRequest)) {
                streamRequest.roomId = std::move(legacyRequest.roomId);
                streamRequest.senderDeviceId =
                    std::move(legacyRequest.senderDeviceId);
                streamRequest.sequence = legacyRequest.sequence;
                streamRequest.framesPerSecond =
                    legacyRequest.framesPerSecond;
                streamRequest.quality = ScreenQualityTier::kOriginal;
                decodedStreamRequest = true;
            }
        }
        if (decodedStreamRequest) {
            std::uint32_t captureFrameRate = 30;
            std::uint32_t maximumFrameRate = 60;
            std::uint32_t sourceWidth = 0;
            std::uint32_t sourceHeight = 0;
            std::uint64_t screenShareGeneration = 0;
            bool preflightPreference = false;
            std::shared_ptr<RoomPairRuntime> pair;
            webrtc::scoped_refptr<WindowsDesktopCaptureSource> source;
            {
                std::lock_guard lock(mutex_);
                const auto pairIt = roomPairs_.find(pairId);
                const bool activeLocalShare =
                    snapshot_.room.screenShareState ==
                        RoomScreenShareState::kActive &&
                    snapshot_.room.screenSharerDeviceId ==
                        snapshot_.localDeviceId;
                const bool startingLocalShare =
                    snapshot_.room.screenShareState ==
                        RoomScreenShareState::kSwitching &&
                    snapshot_.room.pendingScreenSharerDeviceId ==
                        snapshot_.localDeviceId;
                if (pairIt == roomPairs_.end() ||
                    snapshot_.room.membership !=
                        RoomMembershipState::kActive ||
                    (!activeLocalShare && !startingLocalShare) ||
                    streamRequest.roomId != snapshot_.room.roomId ||
                    streamRequest.roomId != pairIt->second->roomId ||
                    streamRequest.senderDeviceId !=
                        pairIt->second->peerDeviceId ||
                    streamRequest.sequence <=
                        pairIt->second->lastScreenControlSequence) {
                    return;
                }
                pairIt->second->lastScreenControlSequence =
                    streamRequest.sequence;
                source = localDesktopCaptureSource_;
                maximumFrameRate = MaximumDesktopCaptureFrameRate(
                    options_.desktopCaptureImplementation,
                    source.get());
                const std::size_t onlineMemberCount =
                    static_cast<std::size_t>(std::count_if(
                        snapshot_.room.members.begin(),
                        snapshot_.room.members.end(),
                        [](const RoomMemberSnapshot& member) {
                            return member.online;
                        }));
                std::uint32_t effectiveFrameRateLimit =
                    maximumFrameRate;
                if (onlineMemberCount >
                    kHighOccupancyRoomMemberThreshold) {
                    effectiveFrameRateLimit = (std::min)(
                        effectiveFrameRateLimit,
                        kMultiMemberMaximumScreenFrameRate);
                }
                streamRequest.framesPerSecond = (std::min)(
                    streamRequest.framesPerSecond,
                    effectiveFrameRateLimit);
                roomScreenStreamPreferences_[pairId] = streamRequest;
                captureFrameRate = 5;
                for (const auto& [requestPairId, preference] :
                     roomScreenStreamPreferences_) {
                    (void)requestPairId;
                    captureFrameRate = (std::max)(
                        captureFrameRate, preference.framesPerSecond);
                }
                localScreenFrameRate_ = captureFrameRate;
                screenShareGeneration =
                    snapshot_.room.screenShareEpoch;
                pair = pairIt->second;
                sourceWidth = source ? source->CapturedWidth() : 0;
                sourceHeight = source ? source->CapturedHeight() : 0;
                preflightPreference =
                    startingLocalShare && !source;
                if (preflightPreference) {
                    const auto* display = FindDisplayByStableKey(
                        snapshot_.localDisplayTopology,
                        snapshot_.selectedLocalDisplayKey);
                    if (!display) {
                        display = FindPrimaryDisplay(
                            snapshot_.localDisplayTopology);
                    }
                    if (display) {
                        sourceWidth = display->width;
                        sourceHeight = display->height;
                    }
                }
            }

            ScreenStreamPreferenceApplied response;
            response.roomId = streamRequest.roomId;
            response.senderDeviceId = Snapshot().localDeviceId;
            response.requestSequence = streamRequest.sequence;
            response.screenShareGeneration =
                screenShareGeneration;
            if (source &&
                !source->SetTargetFrameRate(captureFrameRate)) {
                response.error = "The desktop capture source rejected the requested frame rate.";
            } else if (sourceWidth < 2 || sourceHeight < 2) {
                response.error = preflightPreference
                    ? "The selected display is not available for screen-share preflight."
                    : "The first desktop frame is not ready yet.";
            } else {
                const auto effective = ResolveScreenPolicy(
                    sourceWidth, sourceHeight, streamRequest);
                const auto result = pair->session->SetVideoSlotEncodingPolicy(
                    kScreenMainVideoSlot,
                    effective.framesPerSecond,
                    effective.width,
                    effective.height);
                response.accepted = result.ok();
                response.width = effective.width;
                response.height = effective.height;
                response.framesPerSecond = effective.framesPerSecond;
                response.maxBitrateBps = effective.maxBitrateBps;
                response.scaleBackend = ScreenScaleBackend::kWebRtc;
                if (!result.ok()) {
                    response.error = std::string(result.message());
                }
            }

            ScreenCaptureCapability capability;
            capability.roomId = response.roomId;
            capability.senderDeviceId = response.senderDeviceId;
            capability.maximumFrameRate = maximumFrameRate;
            std::vector<std::uint8_t> capabilityBytes;
            if (EncodeScreenCaptureCapability(
                    capability, &capabilityBytes)) {
                pair->controller->SendData(
                    std::string(kControlReliableChannel),
                    capabilityBytes, true);
            }

            ScreenCaptureRuntimeCapability runtimeCapability;
            runtimeCapability.roomId = response.roomId;
            runtimeCapability.senderDeviceId = response.senderDeviceId;
            runtimeCapability.maximumFrameRate = maximumFrameRate;
            runtimeCapability.configuredBackend =
                DesktopCaptureImplementationName(
                    options_.desktopCaptureImplementation);
            runtimeCapability.activeBackend = source
                ? DesktopCaptureBackendName(source->Backend())
                : "not_initialized";
            runtimeCapability.fallbackReason = source
                ? source->FallbackReason()
                : "desktop_capture_source_not_initialized";
            std::vector<std::uint8_t> runtimeCapabilityBytes;
            if (EncodeScreenCaptureRuntimeCapability(
                    runtimeCapability, &runtimeCapabilityBytes)) {
                pair->controller->SendData(
                    std::string(kControlReliableChannel),
                    runtimeCapabilityBytes, true);
            }

            // Send the generation-bound result first. Current peers consume
            // it and then ignore the following legacy duplicate because the
            // request is no longer pending; older peers ignore type-11 and
            // continue to consume type-3.
            std::vector<std::uint8_t> responseBytes;
            if (EncodeScreenStreamPreferenceAppliedV2(
                    response, &responseBytes)) {
                pair->controller->SendData(
                    std::string(kControlReliableChannel),
                    responseBytes, true);
            }
            if (EncodeScreenStreamPreferenceApplied(
                    response, &responseBytes)) {
                pair->controller->SendData(
                    std::string(kControlReliableChannel),
                    responseBytes, true);
            }
            if (!response.accepted) {
                {
                    std::lock_guard lock(mutex_);
                    snapshot_.room.errorCode =
                        "screen_stream_preference_failed";
                    snapshot_.room.errorMessage = response.error;
                }
                PublishSnapshot();
            }
            BroadcastSharedDisplayLayout();
            BroadcastSharedDisplayCatalog();
            return;
        }
    }
