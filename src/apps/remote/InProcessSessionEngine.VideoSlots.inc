// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

SessionCommandResult InProcessSessionEngine::SetRoomVideoSource(
    const std::string& slot,
    webrtc::scoped_refptr<webrtc::VideoTrackSourceInterface> source,
    const std::string& trackId,
    std::optional<std::string> expectedRoomId,
    std::optional<std::uint64_t> expectedCameraGeneration)
{
    if (!IsRoomVideoSlot(slot)) {
        return Failure("room_video_slot_invalid",
                       "The requested room video slot is invalid.");
    }
    if (!source || trackId.empty()) {
        return Failure("room_video_source_invalid",
                       "A video source and track ID are required.");
    }
    const auto factory = runtime_ ? runtime_->PeerConnectionFactory()
                                  : nullptr;
    if (!factory) {
        return Failure("webrtc_factory_unavailable",
                       "The WebRTC factory is not ready.");
    }
    auto track = factory->CreateVideoTrack(std::move(source), trackId);
    if (!track) {
        return Failure("room_video_track_create_failed",
                       "The room video track could not be created.");
    }

    std::vector<std::shared_ptr<RoomPairRuntime>> candidatePairs;
    webrtc::scoped_refptr<webrtc::MediaStreamTrackInterface> fallback;
    {
        std::lock_guard lock(mutex_);
        if (snapshot_.state == SessionEngineState::kStopping ||
            snapshot_.state == SessionEngineState::kStopped) {
            return Failure("session_engine_stopping",
                           "The session engine is stopping.");
        }
        if (snapshot_.room.membership != RoomMembershipState::kActive) {
            return Failure("room_not_active",
                           "Join a room before publishing room video.");
        }
        if ((expectedRoomId &&
             snapshot_.room.roomId != *expectedRoomId) ||
            (expectedCameraGeneration &&
             localCameraTrackGeneration_ !=
                 *expectedCameraGeneration)) {
            return Failure(
                "camera_switch_superseded",
                "The camera switch no longer belongs to the active room.");
        }
        const auto previous = localRoomVideoTracks_.find(slot);
        if (previous != localRoomVideoTracks_.end()) {
            fallback = previous->second;
        } else if (const auto idle = idleRoomVideoTracks_.find(slot);
                   idle != idleRoomVideoTracks_.end()) {
            fallback = idle->second;
        }
        candidatePairs.reserve(roomPairs_.size());
        for (const auto& [pairId, pair] : roomPairs_) {
            (void)pairId;
            if (pair && pair->session && pair->mediaSlotsPrepared) {
                candidatePairs.push_back(pair);
            }
        }
    }

    // RtpSender::SetTrack may synchronously marshal work to WebRTC's
    // signaling thread. Do not hold the engine mutex while waiting for it.
    std::vector<std::shared_ptr<RoomPairRuntime>> attachedPairs;
    for (const auto& pair : candidatePairs) {
        if (slot == kScreenMainVideoSlot) {
            const auto inactive = pair->session->SetVideoSlotSendingActive(
                slot, false);
            if (!inactive.ok()) {
                return Failure("room_video_sender_deactivate_failed",
                               std::string(inactive.message()));
            }
        }
        auto result = pair->session->SetVideoSlotTrack(slot, track);
        if (!result.ok()) {
            for (const auto& attached : attachedPairs) {
                (void)attached->session->SetVideoSlotTrack(slot, fallback);
            }
            return Failure("room_video_track_attach_failed",
                           std::string(result.message()));
        }
        attachedPairs.push_back(pair);
    }
    bool roomStillActive = false;
    {
        std::lock_guard lock(mutex_);
        roomStillActive =
            snapshot_.state != SessionEngineState::kStopping &&
            snapshot_.state != SessionEngineState::kStopped &&
            snapshot_.room.membership == RoomMembershipState::kActive;
        if (roomStillActive && expectedRoomId) {
            roomStillActive =
                snapshot_.room.roomId == *expectedRoomId;
        }
        if (roomStillActive && expectedCameraGeneration) {
            roomStillActive =
                localCameraTrackGeneration_ ==
                *expectedCameraGeneration;
        }
        if (roomStillActive) {
            localRoomVideoTracks_[slot] = std::move(track);
        }
    }
    if (!roomStillActive) {
        for (const auto& attached : attachedPairs) {
            (void)attached->session->SetVideoSlotTrack(slot, fallback);
        }
        return Failure("room_not_active",
                       "The room closed while attaching room video.");
    }
    return Success();
}

SessionCommandResult InProcessSessionEngine::ClearRoomVideoSource(
    const std::string& slot)
{
    if (!IsRoomVideoSlot(slot)) {
        return Failure("room_video_slot_invalid",
                       "The requested room video slot is invalid.");
    }
    const bool detachScreenTrack = slot == kScreenMainVideoSlot;
    webrtc::scoped_refptr<webrtc::MediaStreamTrackInterface> idleTrack;
    webrtc::scoped_refptr<webrtc::MediaStreamTrackInterface> previousTrack;
    std::vector<std::shared_ptr<RoomPairRuntime>> candidatePairs;
    {
        std::lock_guard lock(mutex_);
        if (!detachScreenTrack) {
            const auto idle = idleRoomVideoTracks_.find(slot);
            if (idle == idleRoomVideoTracks_.end() || !idle->second) {
                return Failure("idle_video_track_unavailable",
                               "The room video placeholder is unavailable.");
            }
            idleTrack = idle->second;
        }
        if (const auto previous = localRoomVideoTracks_.find(slot);
            previous != localRoomVideoTracks_.end()) {
            previousTrack = previous->second;
        }
        candidatePairs.reserve(roomPairs_.size());
        for (const auto& [pairId, pair] : roomPairs_) {
            (void)pairId;
            if (pair && pair->session && pair->mediaSlotsPrepared) {
                candidatePairs.push_back(pair);
            }
        }
    }
    for (const auto& pair : candidatePairs) {
        if (detachScreenTrack) {
            const auto inactive = pair->session->SetVideoSlotSendingActive(
                slot, false);
            if (!inactive.ok()) {
                return Failure("room_video_sender_deactivate_failed",
                               std::string(inactive.message()));
            }
        }
        auto result = pair->session->SetVideoSlotTrack(slot, idleTrack);
        if (!result.ok()) {
            return Failure("room_video_track_detach_failed",
                           std::string(result.message()));
        }
    }
    {
        std::lock_guard lock(mutex_);
        const auto current = localRoomVideoTracks_.find(slot);
        if (current != localRoomVideoTracks_.end() &&
            current->second == previousTrack) {
            localRoomVideoTracks_.erase(current);
        }
    }
    return Success();
}

SessionCommandResult InProcessSessionEngine::SetRoomVideoSlotSendingActive(
    const std::string& slot,
    bool active)
{
    if (!IsRoomVideoSlot(slot)) {
        return Failure("room_video_slot_invalid",
                       "The requested room video slot is invalid.");
    }
    std::vector<std::shared_ptr<RoomPairRuntime>> candidatePairs;
    {
        std::lock_guard lock(mutex_);
        candidatePairs.reserve(roomPairs_.size());
        for (const auto& [pairId, pair] : roomPairs_) {
            (void)pairId;
            if (pair && pair->session && pair->mediaSlotsPrepared) {
                candidatePairs.push_back(pair);
            }
        }
    }
    for (const auto& pair : candidatePairs) {
        const auto result = pair->session->SetVideoSlotSendingActive(
            slot, active);
        if (!result.ok()) {
            return Failure(active
                               ? "room_video_sender_activate_failed"
                               : "room_video_sender_deactivate_failed",
                           std::string(result.message()));
        }
    }
    return Success();
}

SessionCommandResult InProcessSessionEngine::SetRoomRemoteVideoSink(
    const std::string& pairId,
    const std::string& slot,
    webrtc::VideoSinkInterface<webrtc::VideoFrame>* sink)
{
    if (!IsRoomVideoSlot(slot)) {
        return Failure("room_video_slot_invalid",
                       "The requested room video slot is invalid.");
    }
    std::shared_ptr<RoomPairRuntime> pair;
    ScreenRefreshRequest refreshRequest;
    bool requestRemoteRefresh = false;
    {
        std::lock_guard lock(mutex_);
        const auto found = roomPairs_.find(pairId);
        if (found == roomPairs_.end() ||
            !found->second->mediaSlotsPrepared ||
            !found->second->session) {
            return Failure("room_pair_not_ready",
                           "The requested room pair is not ready.");
        }
        pair = found->second;
        const auto channel = pair->openDataChannels.find(
            std::string(kControlReliableChannel));
        requestRemoteRefresh =
            sink && slot == kScreenMainVideoSlot &&
            snapshot_.room.membership ==
                RoomMembershipState::kActive &&
            snapshot_.room.screenShareState ==
                RoomScreenShareState::kActive &&
            snapshot_.room.screenShareEpoch != 0 &&
            snapshot_.room.screenSharerDeviceId ==
                pair->peerDeviceId &&
            channel != pair->openDataChannels.end() &&
            channel->second &&
            nextRoomScreenControlSequence_ !=
                std::numeric_limits<std::uint64_t>::max();
        if (requestRemoteRefresh) {
            if (pair->screenStartupRefreshGeneration !=
                snapshot_.room.screenShareEpoch) {
                pair->screenStartupRefreshGeneration =
                    snapshot_.room.screenShareEpoch;
                pair->screenStartupRefreshRequests = 0;
                pair->screenFirstFramePresentedGeneration = 0;
                pair->screenFirstFrameStartupMs = 0;
            }
            ++pair->screenStartupRefreshRequests;
            refreshRequest.roomId = snapshot_.room.roomId;
            refreshRequest.senderDeviceId =
                snapshot_.localDeviceId;
            refreshRequest.sequence =
                ++nextRoomScreenControlSequence_;
            refreshRequest.screenShareGeneration =
                snapshot_.room.screenShareEpoch;
            const auto snapshotIt = std::find_if(
                snapshot_.roomPeerConnections.begin(),
                snapshot_.roomPeerConnections.end(),
                [&pairId](const RoomPeerConnectionSnapshot& current) {
                    return current.pairId == pairId;
                });
            if (snapshotIt != snapshot_.roomPeerConnections.end()) {
                snapshotIt->screenStartupRefreshRequests =
                    pair->screenStartupRefreshRequests;
                snapshotIt->screenFirstFramePresentedGeneration = 0;
                snapshotIt->screenFirstFrameStartupMs = 0;
            }
        }
    }

    // AddSink/RemoveSink enters libwebrtc and may wait for its worker thread.
    // Never hold the engine mutex across that boundary: the worker can publish
    // a pair snapshot at the same time, and the Qt UI is allowed to bind a
    // sink while negotiation is converging.
    pair->session->SetRemoteVideoSlotSink(slot, sink);
    if (requestRemoteRefresh) {
        std::vector<std::uint8_t> encoded;
        if (EncodeScreenRefreshRequest(
                refreshRequest, &encoded)) {
            (void)pair->controller->SendData(
                std::string(kControlReliableChannel),
                encoded,
                true);
        }
    }
    return Success();
}

SessionCommandResult
InProcessSessionEngine::NotifyRoomScreenFirstFramePresented(
    const std::string& pairId,
    std::uint64_t screenShareGeneration,
    std::uint32_t startupElapsedMs)
{
    std::shared_ptr<RoomPairRuntime> pair;
    std::shared_ptr<RoomPairRuntime> clipboardWarmupPair;
    ScreenFirstFramePresented presented;
    {
        std::lock_guard lock(mutex_);
        const auto pairIt = roomPairs_.find(pairId);
        if (pairIt == roomPairs_.end() || !pairIt->second ||
            !pairIt->second->controller ||
            snapshot_.room.membership != RoomMembershipState::kActive ||
            snapshot_.room.screenShareState !=
                RoomScreenShareState::kActive ||
            snapshot_.room.screenShareEpoch != screenShareGeneration ||
            snapshot_.room.screenSharerDeviceId !=
                pairIt->second->peerDeviceId ||
            !pairIt->second->openDataChannels[
                std::string(kControlReliableChannel)]) {
            return Failure(
                "screen_first_present_pair_unavailable",
                "The active screen-share pair is unavailable.");
        }
        if (nextRoomScreenControlSequence_ ==
            std::numeric_limits<std::uint64_t>::max()) {
            return Failure("screen_first_present_sequence_exhausted",
                           "The screen control sequence is exhausted.");
        }
        pair = pairIt->second;
        pair->screenFirstFramePresentedGeneration =
            screenShareGeneration;
        pair->screenFirstFrameStartupMs = startupElapsedMs;
        const auto snapshotIt = std::find_if(
            snapshot_.roomPeerConnections.begin(),
            snapshot_.roomPeerConnections.end(),
            [&pairId](const RoomPeerConnectionSnapshot& current) {
                return current.pairId == pairId;
            });
        if (snapshotIt != snapshot_.roomPeerConnections.end()) {
            snapshotIt->screenFirstFramePresentedGeneration =
                screenShareGeneration;
            snapshotIt->screenFirstFrameStartupMs = startupElapsedMs;
            snapshotIt->screenStartupRefreshRequests =
                pair->screenStartupRefreshRequests;
        }
        if (pair->clipboardWarmupPending &&
            !pair->clipboardTransferPrimed &&
            !pair->clipboardWarmupRunning) {
            pair->clipboardWarmupPending = false;
            pair->clipboardWarmupRunning = true;
            clipboardWarmupPair = pair;
        }
        presented.roomId = snapshot_.room.roomId;
        presented.senderDeviceId = snapshot_.localDeviceId;
        presented.sequence = ++nextRoomScreenControlSequence_;
        presented.screenShareGeneration = screenShareGeneration;
        presented.startupElapsedMs = startupElapsedMs;
    }

    std::vector<std::uint8_t> encoded;
    std::string encodeError;
    if (!EncodeScreenFirstFramePresented(
            presented, &encoded, &encodeError)) {
        return Failure("screen_first_present_encode_failed", encodeError);
    }
    const SendResult sendResult = pair->controller->SendData(
        std::string(kControlReliableChannel), encoded, true);
    if (clipboardWarmupPair) {
        StartClipboardWarmup(clipboardWarmupPair);
    }
    PublishSnapshot();
    return sendResult == SendResult::kSent
        ? Success()
        : Failure("screen_first_present_send_failed",
                  "The first-frame presentation acknowledgement could not be sent.");
}
