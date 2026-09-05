// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

    if (reliableChannel) {
        ScreenRefreshRequest refreshRequest;
        if (DecodeScreenRefreshRequest(payload, &refreshRequest)) {
            webrtc::scoped_refptr<WindowsDesktopCaptureSource>
                sourceToRefresh;
            {
                std::lock_guard lock(mutex_);
                const auto pairIt = roomPairs_.find(pairId);
                if (pairIt == roomPairs_.end() ||
                    !pairIt->second ||
                    refreshRequest.roomId !=
                        snapshot_.room.roomId ||
                    refreshRequest.roomId !=
                        pairIt->second->roomId ||
                    refreshRequest.senderDeviceId !=
                        pairIt->second->peerDeviceId ||
                    refreshRequest.sequence <=
                        pairIt->second->lastScreenControlSequence ||
                    snapshot_.room.membership !=
                        RoomMembershipState::kActive ||
                    snapshot_.room.screenShareState !=
                        RoomScreenShareState::kActive ||
                    snapshot_.room.screenSharerDeviceId !=
                        snapshot_.localDeviceId ||
                    refreshRequest.screenShareGeneration !=
                        snapshot_.room.screenShareEpoch) {
                    return;
                }
                pairIt->second->lastScreenControlSequence =
                    refreshRequest.sequence;
                if (pairIt->second->screenStartupRefreshGeneration !=
                    refreshRequest.screenShareGeneration) {
                    pairIt->second->screenStartupRefreshGeneration =
                        refreshRequest.screenShareGeneration;
                    pairIt->second->screenStartupRefreshRequests = 0;
                    pairIt->second
                        ->screenFirstFramePresentedGeneration = 0;
                    pairIt->second->screenFirstFrameStartupMs = 0;
                }
                ++pairIt->second->screenStartupRefreshRequests;
                const auto snapshotIt = std::find_if(
                    snapshot_.roomPeerConnections.begin(),
                    snapshot_.roomPeerConnections.end(),
                    [&pairId](const RoomPeerConnectionSnapshot& current) {
                        return current.pairId == pairId;
                    });
                if (snapshotIt != snapshot_.roomPeerConnections.end()) {
                    snapshotIt->screenStartupRefreshRequests =
                        pairIt->second->screenStartupRefreshRequests;
                    snapshotIt->screenFirstFramePresentedGeneration = 0;
                    snapshotIt->screenFirstFrameStartupMs = 0;
                }
                sourceToRefresh = localDesktopCaptureSource_;
            }
            if (sourceToRefresh) {
                sourceToRefresh->RequestStartupFrameBurst(6);
            }
            // A newly attached viewer also needs the current display identity
            // before its first pointer event can be authorized. Re-publish
            // metadata even when tests or alternate producers supply a video
            // track without a WindowsDesktopCaptureSource.
            BroadcastSharedDisplayLayout();
            BroadcastSharedDisplayCatalog();
            return;
        }

        ScreenFirstFramePresented firstFramePresented;
        if (DecodeScreenFirstFramePresented(
                payload, &firstFramePresented)) {
            std::shared_ptr<RoomPairRuntime> clipboardWarmupPair;
            {
                std::lock_guard lock(mutex_);
                const auto pairIt = roomPairs_.find(pairId);
                if (pairIt == roomPairs_.end() || !pairIt->second ||
                    firstFramePresented.roomId != snapshot_.room.roomId ||
                    firstFramePresented.roomId != pairIt->second->roomId ||
                    firstFramePresented.senderDeviceId !=
                        pairIt->second->peerDeviceId ||
                    firstFramePresented.sequence <=
                        pairIt->second->lastScreenControlSequence ||
                    snapshot_.room.membership !=
                        RoomMembershipState::kActive ||
                    snapshot_.room.screenShareState !=
                        RoomScreenShareState::kActive ||
                    snapshot_.room.screenSharerDeviceId !=
                        snapshot_.localDeviceId ||
                    firstFramePresented.screenShareGeneration !=
                        snapshot_.room.screenShareEpoch) {
                    return;
                }
                pairIt->second->lastScreenControlSequence =
                    firstFramePresented.sequence;
                pairIt->second->screenFirstFramePresentedGeneration =
                    firstFramePresented.screenShareGeneration;
                pairIt->second->screenFirstFrameStartupMs =
                    firstFramePresented.startupElapsedMs;
                const auto snapshotIt = std::find_if(
                    snapshot_.roomPeerConnections.begin(),
                    snapshot_.roomPeerConnections.end(),
                    [&pairId](const RoomPeerConnectionSnapshot& current) {
                        return current.pairId == pairId;
                    });
                if (snapshotIt != snapshot_.roomPeerConnections.end()) {
                    snapshotIt->screenFirstFramePresentedGeneration =
                        firstFramePresented.screenShareGeneration;
                    snapshotIt->screenFirstFrameStartupMs =
                        firstFramePresented.startupElapsedMs;
                    snapshotIt->screenStartupRefreshRequests =
                        pairIt->second->screenStartupRefreshRequests;
                }
                if (pairIt->second->clipboardWarmupPending &&
                    !pairIt->second->clipboardTransferPrimed &&
                    !pairIt->second->clipboardWarmupRunning) {
                    pairIt->second->clipboardWarmupPending = false;
                    pairIt->second->clipboardWarmupRunning = true;
                    clipboardWarmupPair = pairIt->second;
                }
            }
            if (clipboardWarmupPair) {
                StartClipboardWarmup(clipboardWarmupPair);
            }
            PublishSnapshot();
            return;
        }

        RoomMemberActionEnvelope memberAction;
        if (DecodeRoomMemberAction(payload, &memberAction)) {
            std::shared_ptr<RoomPairRuntime> memberPair;
            bool publishMemberAction = false;
            bool executeMemberAction = false;
            bool memberActionAuthorized = false;
            std::string memberActionError;
            {
                std::lock_guard lock(mutex_);
                const auto pairIt = roomPairs_.find(pairId);
                if (pairIt == roomPairs_.end() || !pairIt->second ||
                    memberAction.roomId != snapshot_.room.roomId ||
                    memberAction.roomId != pairIt->second->roomId ||
                    memberAction.senderDeviceId !=
                        pairIt->second->peerDeviceId ||
                    memberAction.receiverDeviceId !=
                        snapshot_.localDeviceId ||
                    snapshot_.room.membership !=
                        RoomMembershipState::kActive) {
                    return;
                }
                memberPair = pairIt->second;
                if (memberAction.response) {
                    const auto duplicate = std::find_if(
                        snapshot_.roomMemberActionResults.begin(),
                        snapshot_.roomMemberActionResults.end(),
                        [&memberAction](
                            const RoomMemberActionResult& current) {
                            return current.sequence ==
                                       memberAction.sequence &&
                                   current.peerDeviceId ==
                                       memberAction.senderDeviceId &&
                                   current.action == memberAction.action;
                        });
                    if (duplicate ==
                        snapshot_.roomMemberActionResults.end()) {
                        RoomMemberActionResult result;
                        result.roomId = memberAction.roomId;
                        result.sequence = memberAction.sequence;
                        result.peerDeviceId =
                            memberAction.senderDeviceId;
                        result.action = memberAction.action;
                        result.accepted = memberAction.accepted;
                        result.error = memberAction.error;
                        snapshot_.roomMemberActionResults.push_back(
                            std::move(result));
                        if (snapshot_.roomMemberActionResults.size() > 32) {
                            snapshot_.roomMemberActionResults.erase(
                                snapshot_.roomMemberActionResults.begin(),
                                snapshot_.roomMemberActionResults.begin() +
                                    (snapshot_.roomMemberActionResults.size() -
                                     32));
                        }
                        publishMemberAction = true;
                    }
                } else {
                    if (memberAction.sequence <=
                        memberPair->lastRoomMemberActionSequence) {
                        return;
                    }
                    memberPair->lastRoomMemberActionSequence =
                        memberAction.sequence;
                    if (memberAction.action ==
                        RoomMemberAction::kRequestScreenShare) {
                        const auto member = std::find_if(
                            snapshot_.room.members.begin(),
                            snapshot_.room.members.end(),
                            [&memberAction](
                                const RoomMemberSnapshot& current) {
                                return current.deviceId ==
                                           memberAction.senderDeviceId &&
                                       current.online;
                            });
                        if (member != snapshot_.room.members.end()) {
                            RoomScreenShareViewRequest request;
                            request.roomId = memberAction.roomId;
                            request.sequence = memberAction.sequence;
                            request.requesterDeviceId =
                                memberAction.senderDeviceId;
                            request.requesterDeviceName =
                                member->deviceName;
                            snapshot_.
                                incomingRoomScreenShareViewRequests.
                                push_back(std::move(request));
                            publishMemberAction = true;
                        }
                    } else {
                        executeMemberAction = true;
                        if (memberAction.action ==
                            RoomMemberAction::kStopScreenShare) {
                            memberActionAuthorized =
                                (snapshot_.room.ownerDeviceId ==
                                     memberAction.senderDeviceId ||
                                 snapshot_.room.
                                         activeControllerDeviceId ==
                                     memberAction.senderDeviceId) &&
                                snapshot_.room.screenShareState ==
                                    RoomScreenShareState::kActive &&
                                snapshot_.room.screenSharerDeviceId ==
                                    snapshot_.localDeviceId &&
                                snapshot_.room.screenShareEpoch ==
                                    memberAction.screenShareEpoch;
                            if (!memberActionAuthorized) {
                                memberActionError =
                                    "The requester is not allowed to stop this screen share.";
                            }
                        } else if (
                            memberAction.action ==
                            RoomMemberAction::kDisableMicrophone) {
                            memberActionAuthorized =
                                snapshot_.room.ownerDeviceId ==
                                memberAction.senderDeviceId;
                            if (!memberActionAuthorized) {
                                memberActionError =
                                    "Only the room owner can disable another member's microphone.";
                            }
                        }
                    }
                }
            }
            if (publishMemberAction) {
                PublishSnapshot();
            }
            if (!executeMemberAction) {
                return;
            }

            SessionCommandResult actionResult;
            if (!memberActionAuthorized) {
                actionResult = Failure(
                    "room_member_action_not_authorized",
                    memberActionError);
            } else if (
                memberAction.action ==
                RoomMemberAction::kStopScreenShare) {
                actionResult = StopRoomScreenShare();
            } else {
                actionResult = SetLocalMicrophoneEnabled(false);
            }
            (void)SendRoomMemberActionResponse(
                memberPair, memberAction, actionResult.accepted,
                actionResult.accepted
                    ? std::string{}
                    : actionResult.errorMessage);
            return;
        }

        SharedDisplaySwitchApplied displaySwitchApplied;
        if (DecodeSharedDisplaySwitchApplied(
                payload, &displaySwitchApplied)) {
            {
                std::lock_guard lock(mutex_);
                const auto pairIt = roomPairs_.find(pairId);
                auto snapshotIt = std::find_if(
                    snapshot_.roomPeerConnections.begin(),
                    snapshot_.roomPeerConnections.end(),
                    [&pairId](const RoomPeerConnectionSnapshot& current) {
                        return current.pairId == pairId;
                    });
                if (pairIt == roomPairs_.end() ||
                    snapshotIt == snapshot_.roomPeerConnections.end() ||
                    displaySwitchApplied.roomId !=
                        snapshot_.room.roomId ||
                    displaySwitchApplied.senderDeviceId !=
                        pairIt->second->peerDeviceId ||
                    snapshot_.room.screenSharerDeviceId !=
                        displaySwitchApplied.senderDeviceId ||
                    displaySwitchApplied.screenShareGeneration !=
                        snapshot_.room.screenShareEpoch ||
                    !snapshotIt->remoteDisplaySwitchPending ||
                    displaySwitchApplied.requestSequence !=
                        snapshotIt->remoteDisplaySwitchSequence) {
                    return;
                }
                snapshotIt->remoteDisplaySwitchPending = false;
                snapshotIt->remoteDisplaySwitchError =
                    displaySwitchApplied.accepted
                        ? std::string{}
                        : displaySwitchApplied.error;
                if (!displaySwitchApplied.accepted) {
                    snapshotIt->errorCode =
                        "shared_display_switch_rejected";
                    snapshotIt->errorMessage =
                        displaySwitchApplied.error;
                }
            }
            PublishSnapshot();
            return;
        }

        SharedDisplayCatalog displayCatalog;
        if (DecodeSharedDisplayCatalog(payload, &displayCatalog)) {
            {
                std::lock_guard lock(mutex_);
                const auto pairIt = roomPairs_.find(pairId);
                auto snapshotIt = std::find_if(
                    snapshot_.roomPeerConnections.begin(),
                    snapshot_.roomPeerConnections.end(),
                    [&pairId](const RoomPeerConnectionSnapshot& current) {
                        return current.pairId == pairId;
                    });
                if (pairIt == roomPairs_.end() ||
                    snapshotIt == snapshot_.roomPeerConnections.end() ||
                    displayCatalog.roomId != snapshot_.room.roomId ||
                    displayCatalog.senderDeviceId !=
                        pairIt->second->peerDeviceId ||
                    snapshot_.room.screenSharerDeviceId !=
                        displayCatalog.senderDeviceId ||
                    displayCatalog.screenShareGeneration !=
                        snapshot_.room.screenShareEpoch) {
                    return;
                }
                snapshotIt->remoteDisplayCatalogReported = true;
                snapshotIt->remoteDisplayCatalogLayoutVersion =
                    displayCatalog.layoutVersion;
                snapshotIt->remoteDisplays =
                    std::move(displayCatalog.displays);
            }
            PublishSnapshot();
            return;
        }

        SharedDisplaySwitchRequest displaySwitchRequest;
        if (DecodeSharedDisplaySwitchRequest(
                payload, &displaySwitchRequest)) {
            std::shared_ptr<RoomPairRuntime> requestPair;
            bool authorized = false;
            std::string authorizationError;
            {
                std::lock_guard lock(mutex_);
                const auto pairIt = roomPairs_.find(pairId);
                if (pairIt == roomPairs_.end() ||
                    displaySwitchRequest.roomId !=
                        snapshot_.room.roomId ||
                    displaySwitchRequest.roomId !=
                        pairIt->second->roomId ||
                    displaySwitchRequest.senderDeviceId !=
                        pairIt->second->peerDeviceId ||
                    displaySwitchRequest.sequence <=
                        pairIt->second->lastScreenControlSequence) {
                    return;
                }
                pairIt->second->lastScreenControlSequence =
                    displaySwitchRequest.sequence;
                requestPair = pairIt->second;
                authorized =
                    snapshot_.room.membership ==
                        RoomMembershipState::kActive &&
                    snapshot_.room.screenShareState ==
                        RoomScreenShareState::kActive &&
                    snapshot_.room.screenSharerDeviceId ==
                        snapshot_.localDeviceId &&
                    displaySwitchRequest.screenShareGeneration ==
                        snapshot_.room.screenShareEpoch &&
                    (snapshot_.room.ownerDeviceId ==
                         displaySwitchRequest.senderDeviceId ||
                     snapshot_.room.activeControllerDeviceId ==
                         displaySwitchRequest.senderDeviceId);
                if (!authorized) {
                    authorizationError =
                        "The requester is not allowed to switch this shared display.";
                }
            }
            const SessionCommandResult switchResult =
                authorized
                    ? SwitchLocalSharedDisplay(
                          displaySwitchRequest.stableDisplayKey)
                    : Failure(
                          "shared_display_switch_not_authorized",
                          authorizationError);
            SharedDisplaySwitchApplied response;
            response.roomId = displaySwitchRequest.roomId;
            response.senderDeviceId = Snapshot().localDeviceId;
            response.requestSequence =
                displaySwitchRequest.sequence;
            response.screenShareGeneration =
                displaySwitchRequest.screenShareGeneration;
            response.stableDisplayKey =
                displaySwitchRequest.stableDisplayKey;
            response.accepted = switchResult.accepted;
            response.error = switchResult.accepted
                ? std::string{}
                : switchResult.errorMessage;
            std::vector<std::uint8_t> responseBytes;
            if (EncodeSharedDisplaySwitchApplied(
                    response, &responseBytes)) {
                (void)requestPair->controller->SendData(
                    std::string(kControlReliableChannel),
                    responseBytes, true);
            }
            return;
        }
