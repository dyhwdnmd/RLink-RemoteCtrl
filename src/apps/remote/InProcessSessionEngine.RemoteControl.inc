// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

SessionCommandResult InProcessSessionEngine::RequestRoomControl()
{
    if (!signaling_ || !SignalingIsOnline()) {
        return Failure("signaling_not_online",
                       "The device is not registered with signaling.");
    }
    std::string roomId;
    {
        std::lock_guard lock(mutex_);
        if (snapshot_.room.membership != RoomMembershipState::kActive ||
            snapshot_.room.screenShareState !=
                RoomScreenShareState::kActive ||
            snapshot_.room.screenSharerDeviceId.empty()) {
            return Failure("room_screen_share_not_active",
                           "There is no active shared screen to control.");
        }
        if (snapshot_.room.screenSharerDeviceId ==
            snapshot_.localDeviceId) {
            return Failure("room_control_self_not_allowed",
                           "The screen sharer cannot request control of itself.");
        }
        if (!snapshot_.room.activeControllerDeviceId.empty()) {
            return Failure("room_control_occupied",
                           "Another member already controls the shared screen.");
        }
        if (!snapshot_.room.pendingControllerDeviceId.empty()) {
            return Failure("room_control_request_pending",
                           "A control request is already pending.");
        }
        roomId = snapshot_.room.roomId;
    }
    const auto result = signaling_->RequestRoomControl(roomId);
    return result.accepted
               ? Success()
               : Failure(result.errorCode, result.errorMessage);
}

SessionCommandResult InProcessSessionEngine::RespondToRoomControl(
    const std::string& requestId,
    bool accepted)
{
    if (requestId.empty()) {
        return Failure("room_control_request_id_empty",
                       "A control request ID is required.");
    }
    if (!signaling_ || !SignalingIsOnline()) {
        return Failure("signaling_not_online",
                       "The device is not registered with signaling.");
    }
    std::string roomId;
    {
        std::lock_guard lock(mutex_);
        const auto request = std::find_if(
            snapshot_.incomingRoomControlRequests.begin(),
            snapshot_.incomingRoomControlRequests.end(),
            [&requestId](const RoomControlRequest& current) {
                return current.requestId == requestId;
            });
        if (request == snapshot_.incomingRoomControlRequests.end() ||
            snapshot_.room.screenSharerDeviceId != snapshot_.localDeviceId) {
            return Failure("room_control_request_not_found",
                           "The control request is no longer pending.");
        }
        roomId = request->roomId;
    }
    const auto result = signaling_->RespondToRoomControl(
        roomId, requestId, accepted,
        accepted ? std::string{} :
                   std::string{"rejected_by_screen_sharer"});
    if (!result.accepted) {
        return Failure(result.errorCode, result.errorMessage);
    }
    {
        std::lock_guard lock(mutex_);
        std::erase_if(snapshot_.incomingRoomControlRequests,
                      [&requestId](const RoomControlRequest& request) {
                          return request.requestId == requestId;
                      });
    }
    PublishSnapshot();
    return Success();
}

SessionCommandResult InProcessSessionEngine::ReleaseRoomControl()
{
    if (!signaling_ || !SignalingIsOnline()) {
        return Failure("signaling_not_online",
                       "The device is not registered with signaling.");
    }
    std::string roomId;
    std::string grantId;
    {
        std::lock_guard lock(mutex_);
        const bool localParticipates =
            snapshot_.room.screenSharerDeviceId == snapshot_.localDeviceId ||
            snapshot_.room.activeControllerDeviceId ==
                snapshot_.localDeviceId;
        if (!localParticipates || roomControlGrantId_.empty()) {
            return Failure("room_control_not_owned",
                           "The local device is not part of the active control lease.");
        }
        roomId = snapshot_.room.roomId;
        grantId = roomControlGrantId_;
    }
    const auto result = signaling_->ReleaseRoomControl(
        roomId, grantId, "released_by_participant");
    return result.accepted
               ? Success()
               : Failure(result.errorCode, result.errorMessage);
}

SessionCommandResult InProcessSessionEngine::SendRoomMemberAction(
    const std::string& peerDeviceId,
    RoomMemberAction action,
    std::uint64_t screenShareEpoch)
{
    const std::string channel(kControlReliableChannel);
    std::shared_ptr<RoomPairRuntime> pair;
    RoomMemberActionEnvelope message;
    {
        std::lock_guard lock(mutex_);
        if (snapshot_.room.membership != RoomMembershipState::kActive) {
            return Failure("room_member_action_not_active",
                           "Join an active room before using member actions.");
        }
        if (peerDeviceId.empty() ||
            peerDeviceId == snapshot_.localDeviceId) {
            return Failure("room_member_action_peer_invalid",
                           "A different room member must be selected.");
        }
        const auto member = std::find_if(
            snapshot_.room.members.begin(), snapshot_.room.members.end(),
            [&peerDeviceId](const RoomMemberSnapshot& current) {
                return current.deviceId == peerDeviceId && current.online;
            });
        if (member == snapshot_.room.members.end()) {
            return Failure("room_member_action_peer_offline",
                           "The selected room member is not online.");
        }
        if (action == RoomMemberAction::kRequestScreenShare &&
            snapshot_.room.screenSharerDeviceId == peerDeviceId) {
            return Failure("room_member_already_sharing",
                           "The selected member is already sharing its screen.");
        }
        if (action == RoomMemberAction::kDisableMicrophone) {
            if (snapshot_.room.ownerDeviceId != snapshot_.localDeviceId) {
                return Failure("room_member_mute_not_authorized",
                               "Only the room owner can disable another member's microphone.");
            }
            if (!member->microphonePublishing) {
                return Failure("room_member_microphone_not_publishing",
                               "The selected member's microphone is already off.");
            }
        }
        if (action == RoomMemberAction::kStopScreenShare) {
            const bool authorized =
                snapshot_.room.ownerDeviceId == snapshot_.localDeviceId ||
                snapshot_.room.activeControllerDeviceId ==
                    snapshot_.localDeviceId;
            if (!authorized ||
                snapshot_.room.screenShareState !=
                    RoomScreenShareState::kActive ||
                snapshot_.room.screenSharerDeviceId != peerDeviceId ||
                screenShareEpoch == 0 ||
                snapshot_.room.screenShareEpoch != screenShareEpoch) {
                return Failure("room_screen_share_stop_not_authorized",
                               "Only the room owner or active controller can stop the current remote share.");
            }
        }
        const auto pairIt = std::find_if(
            roomPairs_.begin(), roomPairs_.end(),
            [&peerDeviceId](const auto& entry) {
                return entry.second &&
                       entry.second->peerDeviceId == peerDeviceId;
            });
        if (pairIt == roomPairs_.end() ||
            !pairIt->second->controller ||
            !pairIt->second->session) {
            return Failure("room_member_action_pair_unavailable",
                           "The target member P2P connection is unavailable.");
        }
        const auto channelIt =
            pairIt->second->openDataChannels.find(channel);
        if (channelIt == pairIt->second->openDataChannels.end() ||
            !channelIt->second) {
            return Failure("room_member_action_channel_not_open",
                           "The reliable control channel is not open.");
        }
        if (pairIt->second->nextRoomMemberActionSequence ==
            std::numeric_limits<std::uint64_t>::max()) {
            return Failure("room_member_action_sequence_exhausted",
                           "The room member action sequence is exhausted.");
        }
        pair = pairIt->second;
        message.roomId = snapshot_.room.roomId;
        message.senderDeviceId = snapshot_.localDeviceId;
        message.receiverDeviceId = peerDeviceId;
        message.sequence = ++pair->nextRoomMemberActionSequence;
        message.screenShareEpoch = screenShareEpoch;
        message.action = action;
    }

    std::vector<std::uint8_t> encoded;
    std::string encodeError;
    if (!EncodeRoomMemberAction(message, &encoded, &encodeError)) {
        return Failure("room_member_action_encode_failed", encodeError);
    }
    switch (pair->controller->SendData(channel, encoded, true)) {
    case SendResult::kSent:
        return Success();
    case SendResult::kChannelNotFound:
        return Failure("room_member_action_channel_not_found",
                       "The reliable control channel was not negotiated.");
    case SendResult::kChannelNotOpen:
        return Failure("room_member_action_channel_not_open",
                       "The reliable control channel is not open.");
    case SendResult::kSessionNotStarted:
        return Failure("room_member_action_session_not_started",
                       "The room P2P session is not active.");
    case SendResult::kSendFailed:
        return Failure("room_member_action_send_failed",
                       "WebRTC rejected the room member action.");
    }
    return Failure("room_member_action_send_failed",
                   "The room member action could not be sent.");
}

SessionCommandResult InProcessSessionEngine::RequestRoomMemberScreenShare(
    const std::string& peerDeviceId)
{
    return SendRoomMemberAction(
        peerDeviceId, RoomMemberAction::kRequestScreenShare);
}

SessionCommandResult
InProcessSessionEngine::RequestRoomMemberMicrophoneMute(
    const std::string& peerDeviceId)
{
    return SendRoomMemberAction(
        peerDeviceId, RoomMemberAction::kDisableMicrophone);
}

SessionCommandResult
InProcessSessionEngine::RequestRemoteRoomScreenShareStop(
    const std::string& peerDeviceId,
    std::uint64_t screenShareEpoch)
{
    return SendRoomMemberAction(
        peerDeviceId, RoomMemberAction::kStopScreenShare,
        screenShareEpoch);
}

SendResult InProcessSessionEngine::SendRoomMemberActionResponse(
    const std::shared_ptr<RoomPairRuntime>& pair,
    const RoomMemberActionEnvelope& request,
    bool accepted,
    const std::string& error)
{
    if (!pair || !pair->controller) {
        return SendResult::kSessionNotStarted;
    }
    RoomMemberActionEnvelope response;
    response.roomId = request.roomId;
    response.senderDeviceId = request.receiverDeviceId;
    response.receiverDeviceId = request.senderDeviceId;
    response.sequence = request.sequence;
    response.screenShareEpoch = request.screenShareEpoch;
    response.action = request.action;
    response.response = true;
    response.accepted = accepted;
    response.error = error;
    std::vector<std::uint8_t> encoded;
    if (!EncodeRoomMemberAction(response, &encoded)) {
        return SendResult::kSendFailed;
    }
    return pair->controller->SendData(
        std::string(kControlReliableChannel), encoded, true);
}

SessionCommandResult
InProcessSessionEngine::RespondToRoomMemberScreenShare(
    const std::string& requesterDeviceId,
    std::uint64_t sequence,
    bool accepted)
{
    if (requesterDeviceId.empty() || sequence == 0) {
        return Failure("room_screen_view_request_invalid",
                       "A pending screen-view request is required.");
    }

    RoomMemberActionEnvelope request;
    std::shared_ptr<RoomPairRuntime> pair;
    {
        std::lock_guard lock(mutex_);
        const auto pending = std::find_if(
            snapshot_.incomingRoomScreenShareViewRequests.begin(),
            snapshot_.incomingRoomScreenShareViewRequests.end(),
            [&requesterDeviceId, sequence](
                const RoomScreenShareViewRequest& current) {
                return current.requesterDeviceId == requesterDeviceId &&
                       current.sequence == sequence;
            });
        if (pending ==
            snapshot_.incomingRoomScreenShareViewRequests.end()) {
            return Failure("room_screen_view_request_not_found",
                           "The screen-view request is no longer pending.");
        }
        const auto pairIt = std::find_if(
            roomPairs_.begin(), roomPairs_.end(),
            [&requesterDeviceId](const auto& entry) {
                return entry.second &&
                       entry.second->peerDeviceId == requesterDeviceId;
            });
        if (pairIt == roomPairs_.end()) {
            return Failure("room_screen_view_pair_unavailable",
                           "The requester P2P connection is unavailable.");
        }
        pair = pairIt->second;
        request.roomId = pending->roomId;
        request.senderDeviceId = requesterDeviceId;
        request.receiverDeviceId = snapshot_.localDeviceId;
        request.sequence = sequence;
        request.action = RoomMemberAction::kRequestScreenShare;
    }

    SessionCommandResult shareResult = Success();
    if (accepted) {
        shareResult = RefreshLocalDisplays();
        if (shareResult.accepted) {
            shareResult = StartRoomScreenShare();
        }
    }
    const bool responseAccepted = accepted && shareResult.accepted;
    const std::string responseError =
        responseAccepted
            ? std::string{}
            : (accepted
                   ? shareResult.errorMessage
                   : std::string{"screen_share_request_rejected"});
    const SendResult sendResult = SendRoomMemberActionResponse(
        pair, request, responseAccepted, responseError);
    {
        std::lock_guard lock(mutex_);
        std::erase_if(
            snapshot_.incomingRoomScreenShareViewRequests,
            [&requesterDeviceId, sequence](
                const RoomScreenShareViewRequest& current) {
                return current.requesterDeviceId == requesterDeviceId &&
                       current.sequence == sequence;
            });
    }
    PublishSnapshot();
    if (sendResult != SendResult::kSent) {
        return Failure("room_screen_view_response_send_failed",
                       "The screen-view response could not be sent.");
    }
    return shareResult.accepted || !accepted
               ? Success()
               : shareResult;
}

SessionCommandResult InProcessSessionEngine::SendRoomInput(
    const RemoteInputEvent& event)
{
    const bool fast = UsesFastInputChannel(event.type);
    const bool reliableButtonReleaseFallback =
        event.type == RemoteInputMessageType::kMouseButton &&
        !event.pressed;
    const std::string channel = fast
        ? std::string(kInputFastChannel)
        : std::string(kControlReliableChannel);
    std::shared_ptr<RoomPairRuntime> pair;
    RemoteInputEnvelope envelope;
    bool reliableChannelOpen = false;
    {
        std::lock_guard lock(mutex_);
        if (snapshot_.room.membership != RoomMembershipState::kActive ||
            snapshot_.room.screenShareState != RoomScreenShareState::kActive ||
            snapshot_.room.activeControllerDeviceId != snapshot_.localDeviceId ||
            snapshot_.room.screenSharerDeviceId.empty() ||
            snapshot_.room.screenSharerDeviceId == snapshot_.localDeviceId ||
            roomControlGrantId_.empty()) {
            return Failure(
                "room_input_not_authorized",
                "The local device does not hold the active control lease.");
        }
        const auto pairIt = std::find_if(
            roomPairs_.begin(), roomPairs_.end(),
            [this](const auto& entry) {
                return entry.second->peerDeviceId ==
                       snapshot_.room.screenSharerDeviceId;
            });
        if (pairIt == roomPairs_.end() || !pairIt->second->controller) {
            return Failure("room_input_pair_unavailable",
                           "The screen sharer P2P connection is unavailable.");
        }
        const auto pairSnapshot = std::find_if(
            snapshot_.roomPeerConnections.begin(),
            snapshot_.roomPeerConnections.end(),
            [&pairIt](const RoomPeerConnectionSnapshot& current) {
                return current.pairId == pairIt->first;
            });
        if (IsPointerInput(event.type) &&
            (pairSnapshot == snapshot_.roomPeerConnections.end() ||
             pairSnapshot->sharedDisplayId == 0 ||
             pairSnapshot->sharedDisplayLayoutVersion == 0 ||
             event.displayId != pairSnapshot->sharedDisplayId ||
             event.displayLayoutVersion !=
                 pairSnapshot->sharedDisplayLayoutVersion)) {
            return Failure(
                "room_input_display_layout_stale",
                "The shared display layout is not ready or has changed.");
        }
        if (!pairIt->second->openDataChannels[channel]) {
            return Failure("room_input_channel_not_open",
                           "The required input DataChannel is not open.");
        }
        if (nextRoomInputSequence_ ==
            std::numeric_limits<std::uint64_t>::max()) {
            return Failure("room_input_sequence_exhausted",
                           "The input sequence counter is exhausted.");
        }
        pair = pairIt->second;
        reliableChannelOpen = pair->openDataChannels[
            std::string(kControlReliableChannel)];
        envelope.roomId = snapshot_.room.roomId;
        envelope.senderDeviceId = snapshot_.localDeviceId;
        envelope.controlGrantId = roomControlGrantId_;
        envelope.sequence = ++nextRoomInputSequence_;
        envelope.timestampUs = static_cast<std::uint64_t>(
            std::chrono::duration_cast<std::chrono::microseconds>(
                std::chrono::steady_clock::now().time_since_epoch()).count());
        envelope.event = event;
    }

    std::vector<std::uint8_t> encoded;
    std::string encodeError;
    if (!EncodeRemoteInput(envelope, &encoded, &encodeError)) {
        return Failure("room_input_encode_failed", encodeError);
    }
    const auto queuePacket =
        [&pair, &encoded](const std::string& targetChannel,
                          bool fastChannel) {
            auto& telemetry = RemoteInputTelemetry::Instance();
            telemetry.RecordPacketQueued(fastChannel);
            const bool queued = pair->controller->QueueData(
                targetChannel,
                encoded,
                true,
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
        // Mouse-button transitions and drag moves share the unordered,
        // zero-retransmit channel. Send one extra best-effort copy so an
        // isolated click remains robust without ever placing button-down
        // behind a reliable SCTP retransmission queue. Every later pointer
        // packet carries the complete button state and repairs either copy.
        (void)queuePacket(std::string(kInputFastChannel), true);
    }
    const bool primaryQueued = queuePacket(channel, fast);
    bool reliableFallbackQueued = false;
    if (reliableButtonReleaseFallback && reliableChannelOpen) {
        // Only button-up receives a reliable safety copy. It cannot delay a
        // drag, but it guarantees eventual release if both fast copies are
        // lost or the fast queue is temporarily full. Do not make this
        // fallback conditional on the primary fast enqueue succeeding: the
        // failure case is exactly where Windows most needs the authoritative
        // button-up state.
        reliableFallbackQueued = queuePacket(
            std::string(kControlReliableChannel), false);
    }
    if (primaryQueued || reliableFallbackQueued) {
        return Success();
    }
    return Failure(
        "room_input_send_queue_failed",
        "The input message could not be queued for WebRTC.");
}
