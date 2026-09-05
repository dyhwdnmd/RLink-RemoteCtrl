// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

SessionCommandResult InProcessSessionEngine::SetRoomScreenFrameRate(
    const std::string& pairId,
    std::uint32_t framesPerSecond)
{
    ScreenStreamPreferenceRequest request;
    request.framesPerSecond = framesPerSecond;
    request.quality = ScreenQualityTier::kOriginal;
    return SetRoomScreenStreamPreference(pairId, request);
}

SessionCommandResult InProcessSessionEngine::SetRoomScreenStreamPreference(
    const std::string& pairId,
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

    std::shared_ptr<RoomPairRuntime> pair;
    ScreenStreamPreferenceRequest request = preference;
    {
        std::lock_guard lock(mutex_);
        const auto pairIt = roomPairs_.find(pairId);
        const bool activeRemoteShare =
            snapshot_.room.screenShareState ==
                RoomScreenShareState::kActive &&
            !snapshot_.room.screenSharerDeviceId.empty() &&
            snapshot_.room.screenSharerDeviceId !=
                snapshot_.localDeviceId;
        const bool startingRemoteShare =
            snapshot_.room.screenShareState ==
                RoomScreenShareState::kSwitching &&
            !snapshot_.room.pendingScreenSharerDeviceId.empty() &&
            snapshot_.room.pendingScreenSharerDeviceId !=
                snapshot_.localDeviceId;
        const std::string& remoteSharer = activeRemoteShare
            ? snapshot_.room.screenSharerDeviceId
            : snapshot_.room.pendingScreenSharerDeviceId;
        if (snapshot_.room.membership != RoomMembershipState::kActive ||
            (!activeRemoteShare && !startingRemoteShare) ||
            pairIt == roomPairs_.end() ||
            pairIt->second->peerDeviceId != remoteSharer ||
            !pairIt->second->controller) {
            return Failure("screen_stream_pair_unavailable",
                           "The active or starting screen sharer P2P connection is unavailable.");
        }
        if (!pairIt->second->openDataChannels[
                std::string(kControlReliableChannel)]) {
            return Failure("screen_stream_channel_not_open",
                           "The reliable control channel is not open.");
        }
        const std::size_t onlineMemberCount =
            static_cast<std::size_t>(std::count_if(
                snapshot_.room.members.begin(),
                snapshot_.room.members.end(),
                [](const RoomMemberSnapshot& member) {
                    return member.online;
                }));
        if (onlineMemberCount > kHighOccupancyRoomMemberThreshold) {
            request.framesPerSecond = (std::min)(
                request.framesPerSecond,
                kMultiMemberMaximumScreenFrameRate);
        }
        if (nextRoomScreenControlSequence_ ==
            std::numeric_limits<std::uint64_t>::max()) {
            return Failure("screen_stream_sequence_exhausted",
                           "The screen control sequence is exhausted.");
        }
        pair = pairIt->second;
        request.roomId = snapshot_.room.roomId;
        request.senderDeviceId = snapshot_.localDeviceId;
        request.sequence = ++nextRoomScreenControlSequence_;
        auto snapshotIt = std::find_if(
            snapshot_.roomPeerConnections.begin(),
            snapshot_.roomPeerConnections.end(),
            [&pairId](const auto& current) {
                return current.pairId == pairId;
            });
        if (snapshotIt != snapshot_.roomPeerConnections.end()) {
            snapshotIt->screenPreferencePending = true;
            snapshotIt->screenPreferenceSequence = request.sequence;
            snapshotIt->screenPreferenceGeneration =
                snapshot_.room.screenShareEpoch;
        }
    }

    std::vector<std::uint8_t> encoded;
    std::string encodeError;
    if (!EncodeScreenStreamPreferenceRequest(
            request, &encoded, &encodeError)) {
        return Failure("screen_stream_encode_failed", encodeError);
    }
    const SendResult sendResult = pair->controller->SendData(
        std::string(kControlReliableChannel), encoded, true);
    if (sendResult != SendResult::kSent) {
        std::lock_guard lock(mutex_);
        auto snapshotIt = std::find_if(
            snapshot_.roomPeerConnections.begin(),
            snapshot_.roomPeerConnections.end(),
            [&pairId](const auto& current) {
                return current.pairId == pairId;
            });
        if (snapshotIt != snapshot_.roomPeerConnections.end() &&
            snapshotIt->screenPreferenceSequence == request.sequence) {
            snapshotIt->screenPreferencePending = false;
            snapshotIt->screenPreferenceGeneration = 0;
        }
    }
    switch (sendResult) {
    case SendResult::kSent:
        PublishSnapshot();
        return Success();
    case SendResult::kChannelNotFound:
        return Failure("screen_stream_channel_not_found",
                       "The reliable control channel was not negotiated.");
    case SendResult::kChannelNotOpen:
        return Failure("screen_stream_channel_not_open",
                       "The reliable control channel is not open.");
    case SendResult::kSessionNotStarted:
        return Failure("screen_stream_session_not_started",
                       "The room P2P session is not active.");
    case SendResult::kSendFailed:
        return Failure("screen_stream_send_failed",
                       "WebRTC rejected the screen stream request.");
    }
    return Failure("screen_stream_send_failed",
                   "The screen stream request could not be sent.");
}

SessionCommandResult
InProcessSessionEngine::RequestRemoteSharedDisplaySwitch(
    const std::string& pairId,
    const std::string& stableDisplayKey)
{
    if (stableDisplayKey.empty()) {
        return Failure("shared_display_key_empty",
                       "A remote display must be selected.");
    }
    std::shared_ptr<RoomPairRuntime> pair;
    SharedDisplaySwitchRequest request;
    {
        std::lock_guard lock(mutex_);
        const auto pairIt = roomPairs_.find(pairId);
        auto pairSnapshot = std::find_if(
            snapshot_.roomPeerConnections.begin(),
            snapshot_.roomPeerConnections.end(),
            [&pairId](const RoomPeerConnectionSnapshot& current) {
                return current.pairId == pairId;
            });
        const bool authorized =
            snapshot_.room.ownerDeviceId == snapshot_.localDeviceId ||
            snapshot_.room.activeControllerDeviceId ==
                snapshot_.localDeviceId;
        if (!authorized ||
            snapshot_.room.membership != RoomMembershipState::kActive ||
            snapshot_.room.screenShareState !=
                RoomScreenShareState::kActive ||
            snapshot_.room.screenSharerDeviceId.empty() ||
            snapshot_.room.screenSharerDeviceId ==
                snapshot_.localDeviceId ||
            pairIt == roomPairs_.end() ||
            !pairIt->second->controller ||
            pairIt->second->peerDeviceId !=
                snapshot_.room.screenSharerDeviceId ||
            pairSnapshot == snapshot_.roomPeerConnections.end()) {
            return Failure(
                "shared_display_switch_not_authorized",
                "Only the room owner or active controller can switch the remote shared display.");
        }
        if (!pairSnapshot->remoteDisplayCatalogReported ||
            std::none_of(
                pairSnapshot->remoteDisplays.begin(),
                pairSnapshot->remoteDisplays.end(),
                [&stableDisplayKey](
                    const DisplayDescriptor& display) {
                    return display.stableDisplayKey ==
                           stableDisplayKey;
                })) {
            return Failure(
                "shared_display_not_available",
                "The selected remote display is no longer available.");
        }
        if (pairSnapshot->sharedDisplayStableKey ==
            stableDisplayKey) {
            return Success();
        }
        const auto channel = pairIt->second->openDataChannels.find(
            std::string(kControlReliableChannel));
        if (channel == pairIt->second->openDataChannels.end() ||
            !channel->second) {
            return Failure("shared_display_channel_not_open",
                           "The reliable control channel is not open.");
        }
        if (nextRoomScreenControlSequence_ ==
            std::numeric_limits<std::uint64_t>::max()) {
            return Failure("shared_display_sequence_exhausted",
                           "The screen control sequence is exhausted.");
        }
        pair = pairIt->second;
        request.roomId = snapshot_.room.roomId;
        request.senderDeviceId = snapshot_.localDeviceId;
        request.sequence = ++nextRoomScreenControlSequence_;
        request.screenShareGeneration =
            snapshot_.room.screenShareEpoch;
        request.stableDisplayKey = stableDisplayKey;
        pairSnapshot->remoteDisplaySwitchPending = true;
        pairSnapshot->remoteDisplaySwitchSequence =
            request.sequence;
        pairSnapshot->remoteDisplaySwitchError.clear();
    }

    std::vector<std::uint8_t> encoded;
    std::string encodeError;
    if (!EncodeSharedDisplaySwitchRequest(
            request, &encoded, &encodeError)) {
        std::lock_guard lock(mutex_);
        auto pairSnapshot = std::find_if(
            snapshot_.roomPeerConnections.begin(),
            snapshot_.roomPeerConnections.end(),
            [&pairId](const RoomPeerConnectionSnapshot& current) {
                return current.pairId == pairId;
            });
        if (pairSnapshot != snapshot_.roomPeerConnections.end() &&
            pairSnapshot->remoteDisplaySwitchSequence ==
                request.sequence) {
            pairSnapshot->remoteDisplaySwitchPending = false;
        }
        return Failure("shared_display_switch_encode_failed",
                       encodeError);
    }
    const SendResult sendResult = pair->controller->SendData(
        std::string(kControlReliableChannel), encoded, true);
    if (sendResult == SendResult::kSent) {
        PublishSnapshot();
        return Success();
    }
    {
        std::lock_guard lock(mutex_);
        auto pairSnapshot = std::find_if(
            snapshot_.roomPeerConnections.begin(),
            snapshot_.roomPeerConnections.end(),
            [&pairId](const RoomPeerConnectionSnapshot& current) {
                return current.pairId == pairId;
            });
        if (pairSnapshot != snapshot_.roomPeerConnections.end() &&
            pairSnapshot->remoteDisplaySwitchSequence ==
                request.sequence) {
            pairSnapshot->remoteDisplaySwitchPending = false;
        }
    }
    return Failure(
        "shared_display_switch_send_failed",
        "The remote display switch request could not be sent.");
}

SessionCommandResult InProcessSessionEngine::ApplyLocalScreenFrameRate(
    std::uint32_t framesPerSecond)
{
    if (framesPerSecond > MaximumDesktopCaptureFrameRate(
            options_.desktopCaptureImplementation)) {
        return Failure(
            "desktop_capture_frame_rate_rejected",
            "The selected desktop capture mode does not support the "
            "requested frame rate.");
    }
    webrtc::scoped_refptr<WindowsDesktopCaptureSource> source;
    std::vector<std::pair<std::string, std::shared_ptr<RoomPairRuntime>>> pairs;
    std::unordered_map<std::string, ScreenStreamPreferenceRequest>
        preferences;
    {
        std::lock_guard lock(mutex_);
        const bool localShareOwnsOrIsStarting =
            snapshot_.room.screenSharerDeviceId == snapshot_.localDeviceId ||
            snapshot_.room.pendingScreenSharerDeviceId ==
                snapshot_.localDeviceId;
        if (!localShareOwnsOrIsStarting ||
            !localDesktopCaptureSource_) {
            return Failure("local_screen_share_not_active",
                           "The local desktop capture is not active.");
        }
        localScreenFrameRate_ = framesPerSecond;
        source = localDesktopCaptureSource_;
        pairs.reserve(roomPairs_.size());
        for (const auto& [id, pair] : roomPairs_) {
            if (pair && pair->session && pair->mediaSlotsPrepared) {
                pairs.emplace_back(id, pair);
            }
        }
        preferences = roomScreenStreamPreferences_;
    }

    if (!source->SetTargetFrameRate(framesPerSecond)) {
        return Failure("desktop_capture_frame_rate_rejected",
                       "The desktop capturer rejected the frame rate.");
    }
    const auto sourceWidth = source->CapturedWidth();
    const auto sourceHeight = source->CapturedHeight();
    for (const auto& [pairId, pair] : pairs) {
        std::uint32_t outputFrameRate = framesPerSecond;
        std::uint32_t outputWidth = sourceWidth;
        std::uint32_t outputHeight = sourceHeight;
        if (const auto preference = preferences.find(pairId);
            preference != preferences.end()) {
            const auto effective = ResolveScreenPolicy(
                sourceWidth, sourceHeight, preference->second);
            outputFrameRate = effective.framesPerSecond;
            outputWidth = effective.width;
            outputHeight = effective.height;
        }
        const auto result = pair->session->SetVideoSlotEncodingPolicy(
            kScreenMainVideoSlot,
            outputFrameRate,
            outputWidth,
            outputHeight);
        if (!result.ok()) {
            return Failure("screen_sender_frame_rate_rejected",
                           result.message());
        }
    }
    return Success();
}

SessionCommandResult InProcessSessionEngine::SwitchLocalSharedDisplay(
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

    std::string roomId;
    std::uint64_t screenShareEpoch = 0;
    std::uint64_t nextTrackGeneration = 0;
    std::uint32_t targetFrameRate = kDefaultScreenFrameRate;
    std::uint64_t previousAdapterLuid = 0;
    webrtc::scoped_refptr<WindowsDesktopCaptureSource> previousSource;
    std::vector<std::pair<
        std::string, std::shared_ptr<RoomPairRuntime>>> targetPairs;
    std::unordered_map<std::string, ScreenStreamPreferenceRequest>
        preferences;
    {
        std::lock_guard lock(mutex_);
        if (snapshot_.room.membership != RoomMembershipState::kActive ||
            snapshot_.room.screenShareState !=
                RoomScreenShareState::kActive ||
            snapshot_.room.screenSharerDeviceId !=
                snapshot_.localDeviceId ||
            roomScreenShareGrantId_.empty() ||
            !localDesktopCaptureSource_) {
            return Failure("local_screen_share_not_active",
                           "The local screen share is not active.");
        }
        if (snapshot_.activeSharedDisplay.stableDisplayKey ==
            stableDisplayKey) {
            return Success();
        }
        roomId = snapshot_.room.roomId;
        screenShareEpoch = snapshot_.room.screenShareEpoch;
        nextTrackGeneration = localScreenShareGeneration_ + 1;
        targetFrameRate = localScreenFrameRate_;
        previousSource = localDesktopCaptureSource_;
        previousAdapterLuid =
            previousSource->CaptureTarget().adapterLuid;
        targetPairs.reserve(roomPairs_.size());
        for (const auto& [pairId, pair] : roomPairs_) {
            if (pair && pair->session && pair->mediaSlotsPrepared) {
                targetPairs.emplace_back(pairId, pair);
            }
        }
        preferences = roomScreenStreamPreferences_;
    }

    auto replacement =
        webrtc::make_ref_counted<WindowsDesktopCaptureSource>(
            options_.desktopCaptureImplementation, *selected);
    if (!replacement->SetTargetFrameRate(targetFrameRate) ||
        !replacement->StartCapture()) {
        const std::string captureError = replacement->LastError();
        replacement->StopCapture();
        return Failure(
            "shared_display_capture_start_failed",
            captureError.empty()
                ? "The selected display could not be captured."
                : captureError);
    }

    const std::string trackId =
        "room-screen-switch-" +
        Snapshot().localDeviceId + "-" +
        std::to_string(nextTrackGeneration);
    // Publish the replacement adapter before SetTrack can create its encoder.
    // This lets FFmpeg automatic hardware selection follow the display being
    // shared on hybrid-GPU systems instead of using the previous display.
    if (runtime_) {
        runtime_->SetDesktopCaptureAdapterLuid(selected->adapterLuid);
    }
    const auto attachResult = SetRoomVideoSource(
        kScreenMainVideoSlot, replacement, trackId, roomId);
    if (!attachResult.accepted) {
        if (runtime_) {
            runtime_->SetDesktopCaptureAdapterLuid(previousAdapterLuid);
        }
        replacement->StopCapture();
        (void)SetRoomVideoSlotSendingActive(
            kScreenMainVideoSlot, true);
        return attachResult;
    }

    // SetRoomVideoSource deliberately leaves the screen sender inactive.
    // Reapply the existing per-viewer policy to the replacement track before
    // activating it; otherwise SetTrack can expose libwebrtc's temporary
    // 120-FPS default between activation and the later policy update.
    const std::uint32_t sourceWidth = replacement->CapturedWidth();
    const std::uint32_t sourceHeight = replacement->CapturedHeight();
    for (const auto& [pairId, pair] : targetPairs) {
        std::uint32_t outputFrameRate = targetFrameRate;
        std::uint32_t outputWidth = sourceWidth;
        std::uint32_t outputHeight = sourceHeight;
        if (const auto preference = preferences.find(pairId);
            preference != preferences.end()) {
            const auto effective = ResolveScreenPolicy(
                sourceWidth, sourceHeight, preference->second);
            outputFrameRate = effective.framesPerSecond;
            outputWidth = effective.width;
            outputHeight = effective.height;
        }
        const auto policy = pair->session->SetVideoSlotEncodingPolicy(
            kScreenMainVideoSlot,
            outputFrameRate,
            outputWidth,
            outputHeight);
        if (!policy.ok()) {
            if (runtime_) {
                runtime_->SetDesktopCaptureAdapterLuid(previousAdapterLuid);
            }
            (void)SetRoomVideoSource(
                kScreenMainVideoSlot, previousSource,
                "room-screen-switch-policy-rollback-" +
                    std::to_string(nextTrackGeneration),
                roomId);
            (void)SetRoomVideoSlotSendingActive(
                kScreenMainVideoSlot, true);
            replacement->StopCapture();
            return Failure(
                "shared_display_encoding_policy_failed",
                std::string(policy.message()));
        }
    }
    const auto activateResult = SetRoomVideoSlotSendingActive(
        kScreenMainVideoSlot, true);
    if (!activateResult.accepted) {
        if (runtime_) {
            runtime_->SetDesktopCaptureAdapterLuid(previousAdapterLuid);
        }
        (void)SetRoomVideoSource(
            kScreenMainVideoSlot, previousSource,
            "room-screen-switch-rollback-" +
                std::to_string(nextTrackGeneration),
            roomId);
        (void)SetRoomVideoSlotSendingActive(
            kScreenMainVideoSlot, true);
        replacement->StopCapture();
        return activateResult;
    }

    bool switchStillCurrent = false;
    {
        std::lock_guard lock(mutex_);
        switchStillCurrent =
            snapshot_.room.membership ==
                RoomMembershipState::kActive &&
            snapshot_.room.roomId == roomId &&
            snapshot_.room.screenShareState ==
                RoomScreenShareState::kActive &&
            snapshot_.room.screenSharerDeviceId ==
                snapshot_.localDeviceId &&
            snapshot_.room.screenShareEpoch == screenShareEpoch &&
            localDesktopCaptureSource_.get() ==
                previousSource.get();
        if (switchStillCurrent) {
            localDesktopCaptureSource_ = replacement;
            localScreenShareGeneration_ = nextTrackGeneration;
            snapshot_.screenShareGeneration =
                localScreenShareGeneration_;
            snapshot_.localDisplayTopology = topology;
            snapshot_.selectedLocalDisplayKey =
                selected->stableDisplayKey;
            snapshot_.activeSharedDisplay = *selected;
            snapshot_.activeSharedDisplayLayoutVersion =
                topology.layoutVersion;
            snapshot_.room.errorCode.clear();
            snapshot_.room.errorMessage.clear();
            if (runtime_) {
                runtime_->SetDesktopShareGeneration(
                    localScreenShareGeneration_);
                runtime_->SetDesktopCaptureAdapterLuid(
                    selected->adapterLuid);
            }
        }
    }
    if (!switchStillCurrent) {
        (void)SetRoomVideoSource(
            kScreenMainVideoSlot, previousSource,
            "room-screen-switch-rollback-" +
                std::to_string(nextTrackGeneration),
            roomId);
        (void)SetRoomVideoSlotSendingActive(
            kScreenMainVideoSlot, true);
        replacement->StopCapture();
        return Failure(
            "shared_display_switch_superseded",
            "The screen share changed while switching displays.");
    }

    StartRemoteCursorPublishing(*selected, topology.layoutVersion);

    replacement->RequestStartupFrameBurst();
    std::jthread stopThread(
        [capture = std::move(previousSource)](std::stop_token) {
            capture->StopCapture();
        });
    {
        std::lock_guard lock(mutex_);
        retiredDesktopStopThreads_.push_back(
            std::move(stopThread));
    }
    PublishSnapshot();
    BroadcastSharedDisplayLayout();
    BroadcastSharedDisplayCatalog();
    return Success();
}

SessionCommandResult InProcessSessionEngine::SendRoomFileMessage(
    const std::string& peerDeviceId,
    const FileTransferMessage& message)
{
    const std::string channel(kFileTransferChannel);
    std::shared_ptr<RoomPairRuntime> pair;
    FileTransferEnvelope envelope;
    {
        std::lock_guard lock(mutex_);
        if (snapshot_.room.membership != RoomMembershipState::kActive) {
            return Failure("room_file_transfer_not_active",
                           "Join an active room before transferring files.");
        }
        if (peerDeviceId.empty() ||
            peerDeviceId == snapshot_.localDeviceId) {
            return Failure("room_file_transfer_peer_invalid",
                           "A different room member must be selected.");
        }
        const auto memberIt = std::find_if(
            snapshot_.room.members.begin(), snapshot_.room.members.end(),
            [&peerDeviceId](const RoomMemberSnapshot& member) {
                return member.deviceId == peerDeviceId && member.online;
            });
        if (memberIt == snapshot_.room.members.end()) {
            return Failure("room_file_transfer_peer_offline",
                           "The selected room member is not online.");
        }
        const auto pairIt = std::find_if(
            roomPairs_.begin(), roomPairs_.end(),
            [&peerDeviceId](const auto& entry) {
                return entry.second->peerDeviceId == peerDeviceId;
            });
        if (pairIt == roomPairs_.end() || !pairIt->second->controller ||
            !pairIt->second->session) {
            return Failure("room_file_transfer_pair_unavailable",
                           "The target member P2P connection is unavailable.");
        }
        if (!pairIt->second->openDataChannels[channel]) {
            return Failure("room_file_transfer_channel_not_open",
                           "The file-transfer DataChannel is not open.");
        }
        if (pairIt->second->nextFileTransferSequence ==
            std::numeric_limits<std::uint64_t>::max()) {
            return Failure("room_file_transfer_sequence_exhausted",
                           "The file-transfer sequence is exhausted.");
        }
        pair = pairIt->second;
        envelope.roomId = snapshot_.room.roomId;
        envelope.senderDeviceId = snapshot_.localDeviceId;
        envelope.receiverDeviceId = peerDeviceId;
        envelope.sequence = ++pair->nextFileTransferSequence;
        envelope.timestampUs = static_cast<std::uint64_t>(
            std::chrono::duration_cast<std::chrono::microseconds>(
                std::chrono::steady_clock::now().time_since_epoch()).count());
        envelope.message = message;
    }
    const auto bufferedAmount =
        pair->session->DataChannelBufferedAmount(channel);
    if (!bufferedAmount) {
        return Failure("room_file_transfer_channel_not_open",
                       "The file-transfer DataChannel is unavailable.");
    }
    if (*bufferedAmount > kMaximumFileTransferBufferedBytes) {
        return Failure("room_file_transfer_backpressure",
                       "The file-transfer send buffer is full; retry later.");
    }
    std::vector<std::uint8_t> encoded;
    std::string encodeError;
    if (!EncodeFileTransferMessage(envelope, &encoded, &encodeError)) {
        return Failure("room_file_transfer_encode_failed", encodeError);
    }
    switch (pair->controller->SendData(channel, encoded, true)) {
    case SendResult::kSent:
        return Success();
    case SendResult::kChannelNotFound:
        return Failure("room_file_transfer_channel_not_found",
                       "The file-transfer DataChannel was not negotiated.");
    case SendResult::kChannelNotOpen:
        return Failure("room_file_transfer_channel_not_open",
                       "The file-transfer DataChannel is not open.");
    case SendResult::kSessionNotStarted:
        return Failure("room_file_transfer_session_not_started",
                       "The room P2P session is not active.");
    case SendResult::kSendFailed:
        return Failure("room_file_transfer_send_failed",
                       "WebRTC rejected the file-transfer message.");
    }
    return Failure("room_file_transfer_send_failed",
                   "The file-transfer send result is unknown.");
}

SessionCommandResult InProcessSessionEngine::SendRoomClipboardMessage(
    const std::string& peerDeviceId,
    const std::string& clipboardSessionId,
    const ClipboardMessage& message)
{
    const std::string channel = IsClipboardTransferMessage(message.type)
        ? std::string(kClipboardTransferChannel)
        : std::string(kClipboardReliableChannel);
    std::shared_ptr<RoomPairRuntime> pair;
    ClipboardEnvelope envelope;
    {
        std::lock_guard lock(mutex_);
        if (snapshot_.room.membership != RoomMembershipState::kActive ||
            roomControlGrantId_.empty()) {
            return Failure("room_clipboard_not_authorized",
                           "An active remote-control grant is required.");
        }
        const bool localIsSharer =
            snapshot_.room.screenSharerDeviceId == snapshot_.localDeviceId &&
            snapshot_.room.activeControllerDeviceId == peerDeviceId;
        const bool localIsController =
            snapshot_.room.activeControllerDeviceId == snapshot_.localDeviceId &&
            snapshot_.room.screenSharerDeviceId == peerDeviceId;
        if (!localIsSharer && !localIsController) {
            return Failure("room_clipboard_peer_invalid",
                           "Clipboard data may only target the active control pair.");
        }
        const auto pairIt = std::find_if(
            roomPairs_.begin(), roomPairs_.end(),
            [&peerDeviceId](const auto& entry) {
                return entry.second &&
                       entry.second->peerDeviceId == peerDeviceId;
            });
        if (pairIt == roomPairs_.end() || !pairIt->second->controller ||
            !pairIt->second->session ||
            !pairIt->second->openDataChannels[channel]) {
            return Failure("room_clipboard_channel_not_open",
                           "The clipboard DataChannel is not open.");
        }
        if (IsClipboardTransferMessage(message.type)) {
            // A real Offer/Accept/Chunk/Commit always preempts synthetic
            // warmup. The worker observes this flag before its next 60-KiB
            // chunk, so a user paste never waits behind the remaining 4 MiB.
            pairIt->second->clipboardWarmupCancelled = true;
            pairIt->second->clipboardWarmupPending = false;
            pairIt->second->clipboardTransferPrimed = true;
        }
        auto& nextSequence = channel == kClipboardTransferChannel
            ? pairIt->second->nextClipboardTransferSequence
            : pairIt->second->nextClipboardReliableSequence;
        if (nextSequence == std::numeric_limits<std::uint64_t>::max()) {
            return Failure("room_clipboard_sequence_exhausted",
                           "The clipboard sequence is exhausted.");
        }
        pair = pairIt->second;
        envelope.roomId = snapshot_.room.roomId;
        envelope.senderDeviceId = snapshot_.localDeviceId;
        envelope.receiverDeviceId = peerDeviceId;
        envelope.controlGrantId = roomControlGrantId_;
        envelope.clipboardSessionId = clipboardSessionId;
        envelope.sequence = ++nextSequence;
        envelope.timestampUs = static_cast<std::uint64_t>(
            std::chrono::duration_cast<std::chrono::microseconds>(
                std::chrono::steady_clock::now().time_since_epoch()).count());
        envelope.message = message;
    }
    const auto buffered = pair->session->DataChannelBufferedAmount(channel);
    if (!buffered) {
        return Failure("room_clipboard_channel_not_open",
                       "The clipboard DataChannel is unavailable.");
    }
    if (*buffered > kMaximumClipboardBufferedBytes) {
        return Failure("room_clipboard_backpressure",
                       "The clipboard send buffer is full; retry later.");
    }
    std::vector<std::uint8_t> encoded;
    std::string encodeError;
    if (!EncodeClipboardMessage(envelope, &encoded, &encodeError)) {
        return Failure("room_clipboard_encode_failed", encodeError);
    }
    switch (pair->controller->SendData(channel, encoded, true)) {
    case SendResult::kSent:
        return Success();
    case SendResult::kChannelNotFound:
        return Failure("room_clipboard_channel_not_found",
                       "The clipboard DataChannel was not negotiated.");
    case SendResult::kChannelNotOpen:
        return Failure("room_clipboard_channel_not_open",
                       "The clipboard DataChannel is not open.");
    case SendResult::kSessionNotStarted:
        return Failure("room_clipboard_session_not_started",
                       "The room P2P session is not active.");
    case SendResult::kSendFailed:
        return Failure("room_clipboard_send_failed",
                       "WebRTC rejected the clipboard message.");
    }
    return Failure("room_clipboard_send_failed",
                   "The clipboard send result is unknown.");
}
