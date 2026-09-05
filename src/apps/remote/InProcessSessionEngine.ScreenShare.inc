// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

SessionCommandResult InProcessSessionEngine::StartRoomScreenShare()
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
        if (snapshot_.room.screenSharerDeviceId ==
                snapshot_.localDeviceId ||
            snapshot_.room.pendingScreenSharerDeviceId ==
                snapshot_.localDeviceId) {
            return Success();
        }
        if (snapshot_.room.screenShareState ==
            RoomScreenShareState::kRecovering) {
            return Failure("room_screen_share_recovering",
                           "Wait for the current screen sharer to recover or expire.");
        }
        roomId = snapshot_.room.roomId;
    }
    const auto result = signaling_->RequestRoomScreenShare(roomId);
    return result.accepted
               ? Success()
               : Failure(result.errorCode, result.errorMessage);
}

SessionCommandResult InProcessSessionEngine::RefreshLocalDisplays()
{
    const DisplayTopologySnapshot topology =
        EnumerateWindowsDisplayTopology();
    if (topology.displays.empty()) {
        return Failure("display_not_available",
                       "Windows did not report an attached desktop display.");
    }
    bool activeDisplayChanged = false;
    bool activeDisplayRemoved = false;
    std::optional<DisplayDescriptor> refreshedActiveDisplay;
    std::string stoppedRoomId;
    std::string stoppedGrantId;
    ISignalingClient* signaling = nullptr;
    {
        std::lock_guard lock(mutex_);
        const std::uint64_t previousLayoutVersion =
            snapshot_.activeSharedDisplayLayoutVersion;
        const std::string activeDisplayKey =
            snapshot_.activeSharedDisplay.stableDisplayKey;
        snapshot_.localDisplayTopology = topology;
        if (!activeDisplayKey.empty()) {
            if (const auto* active = FindDisplayByStableKey(
                    topology, activeDisplayKey)) {
                snapshot_.activeSharedDisplay = *active;
                refreshedActiveDisplay = *active;
                snapshot_.activeSharedDisplayLayoutVersion =
                    topology.layoutVersion;
                activeDisplayChanged =
                    previousLayoutVersion != topology.layoutVersion;
            } else {
                snapshot_.activeSharedDisplay = {};
                snapshot_.activeSharedDisplayLayoutVersion = 0;
                snapshot_.room.errorCode =
                    "shared_display_disconnected";
                snapshot_.room.errorMessage =
                    "The shared display was disconnected.";
                activeDisplayRemoved = true;
                stoppedRoomId = snapshot_.room.roomId;
                stoppedGrantId = roomScreenShareGrantId_;
                signaling = signaling_.get();
            }
        }
        if (!FindDisplayByStableKey(
                topology, snapshot_.selectedLocalDisplayKey)) {
            const auto* primary = FindPrimaryDisplay(topology);
            snapshot_.selectedLocalDisplayKey =
                primary ? primary->stableDisplayKey : std::string{};
        }
    }
    PublishSnapshot();
    if (activeDisplayRemoved) {
        StopLocalDesktopCapture();
        if (signaling && !stoppedRoomId.empty() &&
            !stoppedGrantId.empty()) {
            (void)signaling->StopRoomScreenShare(
                stoppedRoomId, stoppedGrantId,
                "shared_display_disconnected");
        }
        return Failure(
            "shared_display_disconnected",
            "The shared display was disconnected; screen sharing stopped.");
    }
    if (activeDisplayChanged) {
        if (refreshedActiveDisplay) {
            cursorMonitor_.UpdateTarget(
                *refreshedActiveDisplay, topology.layoutVersion);
        }
        BroadcastSharedDisplayLayout();
        BroadcastSharedDisplayCatalog();
    }
    return Success();
}

SessionCommandResult
InProcessSessionEngine::SelectRoomScreenShareDisplay(
    const std::string& stableDisplayKey)
{
    if (stableDisplayKey.empty()) {
        return Failure("display_key_empty",
                       "A display must be selected.");
    }
    {
        std::lock_guard lock(mutex_);
        if (!FindDisplayByStableKey(
                snapshot_.localDisplayTopology, stableDisplayKey)) {
            return Failure("display_not_available",
                           "The selected display is no longer attached.");
        }
        if (snapshot_.room.screenSharerDeviceId ==
                snapshot_.localDeviceId ||
            snapshot_.room.pendingScreenSharerDeviceId ==
                snapshot_.localDeviceId) {
            return Failure(
                "display_switch_requires_restart",
                "Stop the current screen share before selecting another display.");
        }
        snapshot_.selectedLocalDisplayKey = stableDisplayKey;
    }
    PublishSnapshot();
    return Success();
}

SessionCommandResult InProcessSessionEngine::StopRoomScreenShare()
{
    if (!signaling_ || !SignalingIsOnline()) {
        return Failure("signaling_not_online",
                       "The device is not registered with signaling.");
    }
    std::string roomId;
    std::string grantId;
    {
        std::lock_guard lock(mutex_);
        const bool localOwnsShare =
            snapshot_.room.screenSharerDeviceId == snapshot_.localDeviceId ||
            snapshot_.room.pendingScreenSharerDeviceId ==
                snapshot_.localDeviceId;
        if (snapshot_.room.membership != RoomMembershipState::kActive ||
            !localOwnsShare || roomScreenShareGrantId_.empty()) {
            return Failure("room_screen_share_not_owned",
                           "The local device does not own the screen-share lease.");
        }
        roomId = snapshot_.room.roomId;
        grantId = roomScreenShareGrantId_;
    }
    const auto result = signaling_->StopRoomScreenShare(
        roomId, grantId, "stopped_by_screen_sharer");
    return result.accepted
               ? Success()
               : Failure(result.errorCode, result.errorMessage);
}

SessionCommandResult
InProcessSessionEngine::RespondToRoomScreenShareSwitch(
    const std::string& requestId,
    bool accepted)
{
    if (requestId.empty()) {
        return Failure("room_screen_share_switch_request_id_empty",
                       "A screen-share takeover request ID is required.");
    }
    if (!signaling_ || !SignalingIsOnline()) {
        return Failure("signaling_not_online",
                       "The device is not registered with signaling.");
    }
    std::string roomId;
    {
        std::lock_guard lock(mutex_);
        const auto request = std::find_if(
            snapshot_.incomingRoomScreenShareSwitchRequests.begin(),
            snapshot_.incomingRoomScreenShareSwitchRequests.end(),
            [&requestId](const RoomScreenShareSwitchRequest& current) {
                return current.requestId == requestId;
            });
        if (request ==
                snapshot_.incomingRoomScreenShareSwitchRequests.end() ||
            snapshot_.room.screenShareState !=
                RoomScreenShareState::kActive ||
            snapshot_.room.screenSharerDeviceId !=
                snapshot_.localDeviceId) {
            return Failure("room_screen_share_switch_not_found",
                           "The screen-share takeover request is no longer pending.");
        }
        roomId = request->roomId;
    }
    const auto result = signaling_->RespondToRoomScreenShareSwitch(
        roomId, requestId, accepted,
        accepted ? std::string{}
                 : std::string{
                       "screen_share_switch_rejected_by_sharer"});
    if (!result.accepted) {
        return Failure(result.errorCode, result.errorMessage);
    }
    {
        std::lock_guard lock(mutex_);
        std::erase_if(
            snapshot_.incomingRoomScreenShareSwitchRequests,
            [&requestId](const RoomScreenShareSwitchRequest& request) {
                return request.requestId == requestId;
            });
    }
    PublishSnapshot();
    return Success();
}

SessionCommandResult
InProcessSessionEngine::CancelRoomScreenShareSwitch()
{
    if (!signaling_ || !SignalingIsOnline()) {
        return Failure("signaling_not_online",
                       "The device is not registered with signaling.");
    }
    std::string roomId;
    std::string requestId;
    {
        std::lock_guard lock(mutex_);
        roomId = snapshot_.room.roomId;
        requestId =
            snapshot_.outgoingRoomScreenShareSwitchRequestId;
        if (roomId.empty() || requestId.empty()) {
            return Failure("room_screen_share_switch_not_found",
                           "There is no pending screen-share takeover request.");
        }
    }
    const auto result = signaling_->CancelRoomScreenShareSwitch(
        roomId, requestId, "screen_share_switch_cancelled");
    return result.accepted
               ? Success()
               : Failure(result.errorCode, result.errorMessage);
}
