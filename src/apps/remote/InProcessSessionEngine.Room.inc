// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

SessionCommandResult InProcessSessionEngine::CreateRoom(
    std::uint32_t capacity)
{
    if (auto ready = RequireReady("create a room"); !ready.accepted) {
        return ready;
    }
    if (capacity < kMinimumRoomMembers ||
        capacity > kProtocolMaximumRoomMembers) {
        return Failure("invalid_room_capacity",
                       "Room capacity must be between 2 and 5 members.");
    }
    if (!signaling_ || !SignalingIsOnline()) {
        return Failure("signaling_not_online",
                       "The device is not registered with signaling.");
    }
    {
        std::lock_guard lock(mutex_);
        if (snapshot_.room.membership != RoomMembershipState::kNone &&
            snapshot_.room.membership != RoomMembershipState::kFailed) {
            return Failure("room_already_active",
                           "The device already has a room operation in progress.");
        }
    }
    const auto result = signaling_->CreateRoom(capacity);
    if (!result.accepted) {
        return {false, result.errorCode, result.errorMessage};
    }
    {
        std::lock_guard lock(mutex_);
        ResetRoomStateLocked();
        snapshot_.room.membership = RoomMembershipState::kCreating;
        snapshot_.room.capacity = capacity;
        snapshot_.room.errorCode.clear();
        snapshot_.room.errorMessage.clear();
    }
    PublishSnapshot();
    return Success();
}

SessionCommandResult InProcessSessionEngine::JoinRoom(
    const std::string& roomId)
{
    if (auto ready = RequireReady("join a room"); !ready.accepted) {
        return ready;
    }
    if (roomId.empty()) {
        return Failure("room_id_empty", "A room ID is required.");
    }
    if (!signaling_ || !SignalingIsOnline()) {
        return Failure("signaling_not_online",
                       "The device is not registered with signaling.");
    }
    {
        std::lock_guard lock(mutex_);
        if (snapshot_.room.membership != RoomMembershipState::kNone &&
            snapshot_.room.membership != RoomMembershipState::kFailed) {
            return Failure("room_already_active",
                           "The device already has a room operation in progress.");
        }
    }
    const auto result = signaling_->RequestRoomJoin(roomId);
    if (!result.accepted) {
        return {false, result.errorCode, result.errorMessage};
    }
    {
        std::lock_guard lock(mutex_);
        ResetRoomStateLocked();
        snapshot_.room.membership = RoomMembershipState::kJoinPending;
        snapshot_.room.roomId = roomId;
        snapshot_.room.errorCode.clear();
        snapshot_.room.errorMessage.clear();
    }
    PublishSnapshot();
    return Success();
}

SessionCommandResult InProcessSessionEngine::QueryRoomAvailability(
    const std::vector<std::string>& roomIds)
{
    if (roomIds.empty() || roomIds.size() > 8) {
        return Failure("invalid_room_availability_query",
                       "Between 1 and 8 room IDs are required.");
    }
    if (!signaling_ || !SignalingIsOnline()) {
        return Failure("signaling_not_online",
                       "The device is not registered with signaling.");
    }
    const auto result = signaling_->QueryRoomAvailability(roomIds);
    if (!result.accepted) {
        return {false, result.errorCode, result.errorMessage};
    }
    {
        std::lock_guard lock(mutex_);
        snapshot_.roomAvailabilities.clear();
        snapshot_.roomAvailabilities.reserve(roomIds.size());
        for (const auto& roomId : roomIds) {
            snapshot_.roomAvailabilities.push_back(
                {roomId, RoomAvailabilityState::kChecking});
        }
    }
    PublishSnapshot();
    return Success();
}

SessionCommandResult InProcessSessionEngine::RespondToRoomJoin(
    const std::string& requestId,
    bool accepted)
{
    if (requestId.empty()) {
        return Failure("room_join_request_id_empty",
                       "A room join request ID is required.");
    }
    if (!signaling_ || !SignalingIsOnline()) {
        return Failure("signaling_not_online",
                       "The device is not registered with signaling.");
    }
    std::string roomId;
    {
        std::lock_guard lock(mutex_);
        const auto requestIt = std::find_if(
            snapshot_.incomingRoomJoinRequests.begin(),
            snapshot_.incomingRoomJoinRequests.end(),
            [&requestId](const RoomJoinRequest& request) {
                return request.requestId == requestId;
            });
        if (requestIt == snapshot_.incomingRoomJoinRequests.end() ||
            snapshot_.room.membership != RoomMembershipState::kActive ||
            snapshot_.room.ownerDeviceId != snapshot_.localDeviceId) {
            return Failure("room_join_request_not_found",
                           "The room join request is no longer pending.");
        }
        roomId = requestIt->roomId;
    }
    const auto result = signaling_->RespondToRoomJoin(
        roomId, requestId, accepted,
        accepted ? std::string{} : std::string{"rejected_by_owner"});
    if (!result.accepted) {
        return {false, result.errorCode, result.errorMessage};
    }
    {
        std::lock_guard lock(mutex_);
        std::erase_if(snapshot_.incomingRoomJoinRequests,
                      [&requestId](const RoomJoinRequest& request) {
                          return request.requestId == requestId;
                      });
    }
    PublishSnapshot();
    return Success();
}

SessionCommandResult InProcessSessionEngine::SetRoomCapacity(
    std::uint32_t capacity)
{
    if (capacity < kMinimumRoomMembers ||
        capacity > kProtocolMaximumRoomMembers) {
        return Failure("invalid_room_capacity",
                       "Room capacity must be between 2 and 5 members.");
    }
    if (!signaling_ || !SignalingIsOnline()) {
        return Failure("signaling_not_online",
                       "The device is not registered with signaling.");
    }
    std::string roomId;
    {
        std::lock_guard lock(mutex_);
        if (snapshot_.room.membership != RoomMembershipState::kActive ||
            snapshot_.room.ownerDeviceId != snapshot_.localDeviceId) {
            return Failure("room_owner_required",
                           "Only the active room owner can change capacity.");
        }
        if (static_cast<std::size_t>(capacity) <
            snapshot_.room.members.size()) {
            return Failure("room_capacity_below_occupancy",
                           "Room capacity cannot be smaller than current occupancy.");
        }
        roomId = snapshot_.room.roomId;
    }
    const auto result = signaling_->SetRoomCapacity(roomId, capacity);
    return result.accepted
               ? Success()
               : Failure(result.errorCode, result.errorMessage);
}

SessionCommandResult InProcessSessionEngine::LeaveRoom()
{
    if (!signaling_ || !SignalingIsOnline()) {
        return Failure("signaling_not_online",
                       "The device is not registered with signaling.");
    }
    std::string roomId;
    {
        std::lock_guard lock(mutex_);
        if (snapshot_.room.membership != RoomMembershipState::kActive ||
            snapshot_.room.roomId.empty()) {
            return Failure("room_not_active",
                           "The device is not an active room member.");
        }
        roomId = snapshot_.room.roomId;
        snapshot_.room.membership = RoomMembershipState::kLeaving;
        roomLeaveRequested_ = true;
    }
    PublishSnapshot();
    const auto result = signaling_->LeaveRoom(roomId,
                                               "left_by_local_user");
    if (!result.accepted) {
        {
            std::lock_guard lock(mutex_);
            snapshot_.room.membership = RoomMembershipState::kActive;
            roomLeaveRequested_ = false;
            snapshot_.room.errorCode = result.errorCode;
            snapshot_.room.errorMessage = result.errorMessage;
        }
        PublishSnapshot();
        return Failure(result.errorCode, result.errorMessage);
    }
    return Success();
}

SessionCommandResult
InProcessSessionEngine::ExitRoomAfterRecoveryFailure()
{
    std::vector<std::shared_ptr<RoomPairRuntime>> roomPairs;
    ISignalingClient* signaling = nullptr;
    std::string roomId;
    bool signalingOnline = false;
    {
        std::lock_guard lock(mutex_);
        roomId = snapshot_.room.roomId;
        if (roomId.empty()) {
            return Success();
        }
        signaling = signaling_.get();
        signalingOnline = signaling &&
            snapshot_.connectivity == SessionConnectivityState::kOnline;
        roomPairs.reserve(roomPairs_.size());
        for (auto& [pairId, pair] : roomPairs_) {
            (void)pairId;
            roomPairs.push_back(std::move(pair));
        }
        roomPairs_.clear();
        ResetRoomStateLocked();
        if (!signalingOnline) {
            deferredRoomLeaveId_ = roomId;
        }
    }

    for (auto& pair : roomPairs) {
        RetireRoomPair(std::move(pair));
    }

    if (signalingOnline) {
        const auto leave = signaling->LeaveRoom(
            roomId, "p2p_recovery_failed");
        if (!leave.accepted) {
            std::lock_guard lock(mutex_);
            deferredRoomLeaveId_ = roomId;
        }
    }
    PublishSnapshot();
    return Success();
}
