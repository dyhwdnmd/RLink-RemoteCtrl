// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

void InProcessSessionEngine::OnRoomScreenShareGranted(
    const SignalingRoomScreenShareGranted& granted)
{
    bool confirmReady = false;
    ISignalingClient* signaling = nullptr;
    std::uint64_t shareGeneration = 0;
    std::uint32_t targetFrameRate = kDefaultScreenFrameRate;
    DisplayDescriptor captureTarget;
    DisplayTopologySnapshot captureTopology;
    {
        std::lock_guard lock(mutex_);
        if (snapshot_.room.membership != RoomMembershipState::kActive ||
            snapshot_.room.roomId != granted.roomId ||
            granted.grantId.empty()) {
            return;
        }
        roomScreenShareGrantId_ = granted.grantId;
        confirmReady =
            snapshot_.room.screenShareState ==
                RoomScreenShareState::kSwitching &&
            snapshot_.room.pendingScreenSharerDeviceId ==
                snapshot_.localDeviceId &&
            snapshot_.room.screenShareEpoch == granted.epoch;
        if (confirmReady) {
            shareGeneration = ++localScreenShareGeneration_;
            snapshot_.screenShareGeneration = shareGeneration;
            if (runtime_) {
                runtime_->SetDesktopShareGeneration(shareGeneration);
            }
            targetFrameRate = localScreenFrameRate_;
            captureTopology = snapshot_.localDisplayTopology;
            const auto* selected = FindDisplayByStableKey(
                captureTopology, snapshot_.selectedLocalDisplayKey);
            if (!selected) {
                selected = FindPrimaryDisplay(captureTopology);
            }
            if (selected) {
                captureTarget = *selected;
                if (runtime_) {
                    runtime_->SetDesktopCaptureAdapterLuid(
                        captureTarget.adapterLuid);
                }
            }
        }
        signaling = signaling_.get();
    }
    if (!confirmReady || !signaling) {
        return;
    }
    if (options_.enableRealDesktopCapture &&
        (captureTarget.sessionDisplayId == 0 ||
         captureTopology.layoutVersion == 0)) {
        signaling->StopRoomScreenShare(
            granted.roomId, granted.grantId, "display_not_available");
        {
            std::lock_guard lock(mutex_);
            snapshot_.room.errorCode = "display_not_available";
            snapshot_.room.errorMessage =
                "The selected display is no longer attached.";
        }
        PublishSnapshot();
        return;
    }

    if (options_.enableRealDesktopCapture) {
        auto source =
            webrtc::make_ref_counted<WindowsDesktopCaptureSource>(
                options_.desktopCaptureImplementation, captureTarget);
        if (!source->SetTargetFrameRate(targetFrameRate)) {
            signaling->StopRoomScreenShare(
                granted.roomId, granted.grantId,
                "capture_frame_rate_rejected");
            {
                std::lock_guard lock(mutex_);
                snapshot_.room.errorCode =
                    "desktop_capture_frame_rate_rejected";
                snapshot_.room.errorMessage =
                    "The desktop capturer rejected the saved frame rate.";
            }
            PublishSnapshot();
            return;
        }
        if (!source->StartCapture()) {
            const std::string message = source->LastError();
            signaling->StopRoomScreenShare(
                granted.roomId, granted.grantId, "capture_failed");
            {
                std::lock_guard lock(mutex_);
                snapshot_.room.errorCode = "desktop_capture_start_failed";
                snapshot_.room.errorMessage = message;
            }
            PublishSnapshot();
            return;
        }

        std::string localDeviceId;
        {
            std::lock_guard lock(mutex_);
            localDeviceId = snapshot_.localDeviceId;
        }
        const auto attachResult = SetRoomVideoSource(
            kScreenMainVideoSlot, source,
            "room-screen-" + localDeviceId + "-" +
                std::to_string(shareGeneration));
        if (!attachResult.accepted) {
            source->StopCapture();
            signaling->StopRoomScreenShare(
                granted.roomId, granted.grantId,
                "capture_track_attach_failed");
            {
                std::lock_guard lock(mutex_);
                snapshot_.room.errorCode = attachResult.errorCode;
                snapshot_.room.errorMessage = attachResult.errorMessage;
            }
            PublishSnapshot();
            return;
        }

        bool grantStillCurrent = false;
        {
            std::lock_guard lock(mutex_);
            grantStillCurrent =
                localScreenShareGeneration_ == shareGeneration &&
                roomScreenShareGrantId_ == granted.grantId &&
                snapshot_.room.roomId == granted.roomId &&
                snapshot_.room.pendingScreenSharerDeviceId ==
                    snapshot_.localDeviceId;
            if (grantStillCurrent) {
                localDesktopCaptureSource_ = source;
                snapshot_.activeSharedDisplay = captureTarget;
                snapshot_.activeSharedDisplayLayoutVersion =
                    captureTopology.layoutVersion;
            }
        }
        if (!grantStillCurrent) {
            (void)ClearRoomVideoSource(kScreenMainVideoSlot);
            source->StopCapture();
            return;
        }
        StartRemoteCursorPublishing(
            captureTarget, captureTopology.layoutVersion);
        // Start every sender with the same resolution/FPS-aware bitrate
        // ceiling used by later viewer frame-rate requests. The share is still
        // in its granted/pending phase here, so ApplyLocalScreenFrameRate also
        // accepts the pending local sharer.
        const auto policyResult = ApplyLocalScreenFrameRate(targetFrameRate);
        if (!policyResult.accepted) {
            StopLocalDesktopCapture();
            signaling->StopRoomScreenShare(
                granted.roomId, granted.grantId,
                "screen_encoding_policy_failed");
            {
                std::lock_guard lock(mutex_);
                snapshot_.room.errorCode = policyResult.errorCode;
                snapshot_.room.errorMessage = policyResult.errorMessage;
            }
            PublishSnapshot();
            return;
        }
        // Start the libwebrtc full-frame prime before activating the sender.
        // SetRoomVideoSlotSendingActive applies the mid-call BWE bootstrap and
        // requests its first keyframe synchronously, so its input window must
        // already be open at that point.
        source->RequestStartupFrameBurst(12);
        const auto activateResult = SetRoomVideoSlotSendingActive(
            kScreenMainVideoSlot, true);
        if (!activateResult.accepted) {
            StopLocalDesktopCapture();
            signaling->StopRoomScreenShare(
                granted.roomId, granted.grantId,
                "screen_sender_activate_failed");
            {
                std::lock_guard lock(mutex_);
                snapshot_.room.errorCode = activateResult.errorCode;
                snapshot_.room.errorMessage = activateResult.errorMessage;
            }
            PublishSnapshot();
            return;
        }
        if (source->ConfiguredImplementation() ==
            DesktopCaptureImplementation::kLibWebRtc) {
            std::vector<std::shared_ptr<RoomPairRuntime>> startupPairs;
            {
                std::lock_guard lock(mutex_);
                startupPairs.reserve(roomPairs_.size());
                for (const auto& [pairId, pair] : roomPairs_) {
                    if (pair && pair->session && pair->mediaSlotsPrepared) {
                        startupPairs.push_back(pair);
                    }
                }
            }
            // Recreate GoogCC once more only after an active sender has
            // received its first real desktop frame. With libwebrtc's
            // difference capture, doing this solely at the active transition
            // can run before media is available and leave the first share at
            // the old audio-only estimate. Capturing the pair runtimes keeps
            // their sessions alive until this one-shot callback has run,
            // without retaining the engine itself.
            auto releasePairs = startupPairs;
            source->RequestStartupFrameBurst(
                30,
                [pairs = std::move(startupPairs)] {
                    for (const auto& pair : pairs) {
                        if (pair && pair->session) {
                            pair->session->
                                RestartVideoSlotBandwidthEstimation(
                                    kScreenMainVideoSlot);
                        }
                    }
                },
                [pairs = std::move(releasePairs)] {
                    for (const auto& pair : pairs) {
                        if (pair && pair->session) {
                            pair->session->
                                FinishVideoSlotBandwidthBootstrap(
                                    kScreenMainVideoSlot);
                        }
                    }
                });
        }
    } else if (captureTarget.sessionDisplayId != 0 &&
               captureTopology.layoutVersion != 0) {
        std::lock_guard lock(mutex_);
        snapshot_.activeSharedDisplay = captureTarget;
        snapshot_.activeSharedDisplayLayoutVersion =
            captureTopology.layoutVersion;
    }

    // Confirm only after the real source produced its first frame and the
    // negotiated screen-main senders were switched to that Track. Tests can
    // disable capture and continue supplying their synthetic source.
    const auto result = signaling->ConfirmRoomScreenShare(
        granted.roomId, granted.grantId);
    if (!result.accepted) {
        {
            std::lock_guard lock(mutex_);
            snapshot_.room.errorCode = result.errorCode;
            snapshot_.room.errorMessage = result.errorMessage;
        }
        PublishSnapshot();
    } else {
        PublishSnapshot();
        BroadcastSharedDisplayLayout();
        BroadcastSharedDisplayCatalog();
    }
}

void InProcessSessionEngine::OnRoomScreenShareSwitchPending(
    const SignalingRoomScreenShareSwitchPending& pending)
{
    {
        std::lock_guard lock(mutex_);
        if (snapshot_.room.membership !=
                RoomMembershipState::kActive ||
            snapshot_.room.roomId != pending.roomId ||
            pending.requestId.empty() ||
            snapshot_.room.screenSharerDeviceId ==
                snapshot_.localDeviceId) {
            return;
        }
        snapshot_.outgoingRoomScreenShareSwitchRequestId =
            pending.requestId;
        snapshot_.room.errorCode.clear();
        snapshot_.room.errorMessage.clear();
    }
    PublishSnapshot();
}

void InProcessSessionEngine::OnRoomScreenShareSwitchRequested(
    const RoomScreenShareSwitchRequest& request)
{
    {
        std::lock_guard lock(mutex_);
        if (snapshot_.room.membership !=
                RoomMembershipState::kActive ||
            snapshot_.room.roomId != request.roomId ||
            snapshot_.room.screenShareState !=
                RoomScreenShareState::kActive ||
            snapshot_.room.screenSharerDeviceId !=
                snapshot_.localDeviceId) {
            return;
        }
        const auto existing = std::find_if(
            snapshot_.incomingRoomScreenShareSwitchRequests.begin(),
            snapshot_.incomingRoomScreenShareSwitchRequests.end(),
            [&request](const RoomScreenShareSwitchRequest& current) {
                return current.requestId == request.requestId;
            });
        if (existing ==
            snapshot_.incomingRoomScreenShareSwitchRequests.end()) {
            snapshot_.incomingRoomScreenShareSwitchRequests.push_back(
                request);
        }
    }
    PublishSnapshot();
}

void InProcessSessionEngine::OnRoomScreenShareSwitchResult(
    const SignalingRoomScreenShareSwitchResult& result)
{
    bool publish = false;
    {
        std::lock_guard lock(mutex_);
        if (snapshot_.room.roomId != result.roomId) {
            return;
        }
        const auto previousSize =
            snapshot_.incomingRoomScreenShareSwitchRequests.size();
        std::erase_if(
            snapshot_.incomingRoomScreenShareSwitchRequests,
            [&result](const RoomScreenShareSwitchRequest& request) {
                return request.requestId == result.requestId;
            });
        publish = previousSize !=
                  snapshot_.incomingRoomScreenShareSwitchRequests.size();
        const bool localRequested =
            snapshot_.outgoingRoomScreenShareSwitchRequestId ==
            result.requestId;
        if (localRequested) {
            snapshot_.outgoingRoomScreenShareSwitchRequestId.clear();
            publish = true;
            if (!result.accepted) {
                snapshot_.room.errorCode =
                    result.reasonCode.empty()
                        ? "room_screen_share_switch_rejected"
                        : result.reasonCode;
                snapshot_.room.errorMessage =
                    result.reasonMessage.empty()
                        ? "The screen-share takeover request was rejected."
                        : result.reasonMessage;
            } else {
                snapshot_.room.errorCode.clear();
                snapshot_.room.errorMessage.clear();
            }
        }
    }
    if (publish) {
        PublishSnapshot();
    }
}

void InProcessSessionEngine::OnRoomControlRequested(
    const RoomControlRequest& request)
{
    {
        std::lock_guard lock(mutex_);
        if (snapshot_.room.membership != RoomMembershipState::kActive ||
            snapshot_.room.roomId != request.roomId ||
            snapshot_.room.screenSharerDeviceId !=
                snapshot_.localDeviceId ||
            snapshot_.room.pendingControllerDeviceId !=
                request.requesterDeviceId) {
            return;
        }
        const auto existing = std::find_if(
            snapshot_.incomingRoomControlRequests.begin(),
            snapshot_.incomingRoomControlRequests.end(),
            [&request](const RoomControlRequest& current) {
                return current.requestId == request.requestId;
            });
        if (existing == snapshot_.incomingRoomControlRequests.end()) {
            snapshot_.incomingRoomControlRequests.push_back(request);
        }
    }
    PublishSnapshot();
}

void InProcessSessionEngine::OnRoomControlResult(
    const SignalingRoomControlResult& result)
{
    bool publish = false;
    {
        std::lock_guard lock(mutex_);
        if (snapshot_.room.roomId != result.roomId) {
            return;
        }
        const auto previousSize =
            snapshot_.incomingRoomControlRequests.size();
        std::erase_if(snapshot_.incomingRoomControlRequests,
                      [&result](const RoomControlRequest& request) {
                          return request.requestId == result.requestId;
                      });
        publish = previousSize !=
                  snapshot_.incomingRoomControlRequests.size();
        if (!result.accepted &&
            snapshot_.room.pendingControllerDeviceId ==
                snapshot_.localDeviceId) {
            snapshot_.room.errorCode = result.reasonCode.empty()
                                           ? "room_control_rejected"
                                           : result.reasonCode;
            snapshot_.room.errorMessage = result.reasonMessage.empty()
                                              ? "The control request was rejected."
                                              : result.reasonMessage;
            publish = true;
        }
    }
    if (publish) {
        PublishSnapshot();
    }
}

void InProcessSessionEngine::OnRoomControlGranted(
    const SignalingRoomControlGranted& granted)
{
    bool publishSharedDisplayMetadata = false;
    {
        std::lock_guard lock(mutex_);
        if (snapshot_.room.roomId != granted.roomId ||
            granted.grantId.empty() ||
            (granted.screenSharerDeviceId != snapshot_.localDeviceId &&
             granted.controllerDeviceId != snapshot_.localDeviceId)) {
            return;
        }
        roomControlGrantId_ = granted.grantId;
        roomControlGrantScreenSharerDeviceId_ =
            granted.screenSharerDeviceId;
        roomControlGrantControllerDeviceId_ =
            granted.controllerDeviceId;
        snapshot_.roomControlGrantActive = true;
        // The grant event is authoritative and is sent before the following
        // room-state broadcast. Apply its participants immediately so the UI
        // cannot display an approved-but-inactive control interval.
        snapshot_.room.screenSharerDeviceId =
            granted.screenSharerDeviceId;
        snapshot_.room.activeControllerDeviceId =
            granted.controllerDeviceId;
        snapshot_.room.pendingControllerDeviceId.clear();
        snapshot_.room.errorCode.clear();
        snapshot_.room.errorMessage.clear();
        publishSharedDisplayMetadata =
            granted.screenSharerDeviceId == snapshot_.localDeviceId &&
            snapshot_.activeSharedDisplay.sessionDisplayId != 0 &&
            snapshot_.activeSharedDisplayLayoutVersion != 0;
    }
    PublishSnapshot();
    if (publishSharedDisplayMetadata) {
        // A viewer can receive the grant before it has the display identity
        // needed to validate pointer coordinates. Re-announce the mapping at
        // the authorization boundary instead of waiting for another room or
        // DataChannel state transition that may never occur.
        BroadcastSharedDisplayLayout();
        BroadcastSharedDisplayCatalog();
    }
}

void InProcessSessionEngine::OnRoomControlRevoked(
    const SignalingRoomControlRevoked& revoked)
{
    IRemoteInputSink* releaseInputSink = nullptr;
    {
        std::lock_guard lock(mutex_);
        if (snapshot_.room.roomId != revoked.roomId) {
            return;
        }
        if (snapshot_.room.screenSharerDeviceId ==
                snapshot_.localDeviceId &&
            !roomControlGrantId_.empty()) {
            releaseInputSink = remoteInputSink_;
        }
        roomControlGrantId_.clear();
        roomControlGrantScreenSharerDeviceId_.clear();
        roomControlGrantControllerDeviceId_.clear();
        snapshot_.roomControlGrantActive = false;
        snapshot_.room.activeControllerDeviceId.clear();
        snapshot_.room.pendingControllerDeviceId.clear();
        snapshot_.incomingRoomControlRequests.clear();
    }
    if (releaseInputSink) {
        releaseInputSink->ReleaseAllRemoteInputs();
    }
    PublishSnapshot();
}
