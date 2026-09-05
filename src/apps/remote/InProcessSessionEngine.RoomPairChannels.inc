// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

void InProcessSessionEngine::OnRoomPairDataChannelStateChanged(
    const std::string& pairId,
    const DataChannelInfo& channel)
{
    bool publishSharedDisplayLayout = false;
    bool republishCursor = false;
    std::shared_ptr<RoomPairRuntime> clipboardWarmupPair;
    {
        std::lock_guard lock(mutex_);
        const auto pairIt = roomPairs_.find(pairId);
        if (pairIt == roomPairs_.end()) {
            return;
        }
        pairIt->second->openDataChannels[channel.label] =
            channel.state == DataChannelState::kOpen;
        if (channel.label == kClipboardTransferChannel) {
            if (channel.state == DataChannelState::kOpen &&
                !pairIt->second->clipboardTransferPrimed &&
                !pairIt->second->clipboardWarmupPending &&
                !pairIt->second->clipboardWarmupRunning &&
                pairIt->second->controller) {
                auto& pair = pairIt->second;
                pair->clipboardWarmupCancelled = false;
                const bool activeShareUsesPair =
                    snapshot_.room.screenShareState ==
                        RoomScreenShareState::kActive &&
                    (snapshot_.room.screenSharerDeviceId ==
                         snapshot_.localDeviceId ||
                     snapshot_.room.screenSharerDeviceId ==
                         pair->peerDeviceId);
                const bool startingShareUsesPair =
                    snapshot_.room.screenShareState ==
                        RoomScreenShareState::kSwitching &&
                    (snapshot_.room.pendingScreenSharerDeviceId ==
                         snapshot_.localDeviceId ||
                     snapshot_.room.pendingScreenSharerDeviceId ==
                         pair->peerDeviceId);
                const bool firstFrameStillPending =
                    startingShareUsesPair ||
                    (activeShareUsesPair &&
                     pair->screenFirstFramePresentedGeneration !=
                         snapshot_.room.screenShareEpoch);
                if (firstFrameStillPending) {
                    // Video startup has strict priority over synthetic SCTP
                    // warmup. The first real Present acknowledgement releases
                    // this pending warmup.
                    pair->clipboardWarmupPending = true;
                } else {
                    pair->clipboardWarmupRunning = true;
                    clipboardWarmupPair = pair;
                }
            } else if (channel.state != DataChannelState::kOpen) {
                pairIt->second->clipboardTransferPrimed = false;
                pairIt->second->clipboardWarmupPending = false;
                pairIt->second->clipboardWarmupCancelled = true;
            }
        }
        std::uint32_t openCount = 0;
        for (const auto& [label, open] :
             pairIt->second->openDataChannels) {
            (void)label;
            if (open) {
                ++openCount;
            }
        }
        const auto snapshotIt = std::find_if(
            snapshot_.roomPeerConnections.begin(),
            snapshot_.roomPeerConnections.end(),
            [&pairId](const RoomPeerConnectionSnapshot& current) {
                return current.pairId == pairId;
            });
        if (snapshotIt != snapshot_.roomPeerConnections.end()) {
            snapshotIt->openDataChannelCount = openCount;
            if (channel.label == kControlReliableChannel) {
                snapshotIt->controlReliableChannelOpen =
                    channel.state == DataChannelState::kOpen;
            } else if (channel.label == kInputFastChannel) {
                snapshotIt->inputFastChannelOpen =
                    channel.state == DataChannelState::kOpen;
            } else if (channel.label == kClipboardReliableChannel) {
                snapshotIt->clipboardReliableChannelOpen =
                    channel.state == DataChannelState::kOpen;
            } else if (channel.label == kClipboardTransferChannel) {
                snapshotIt->clipboardTransferChannelOpen =
                    channel.state == DataChannelState::kOpen;
            }
        }
        publishSharedDisplayLayout =
            channel.label == kControlReliableChannel &&
            channel.state == DataChannelState::kOpen &&
            snapshot_.room.screenSharerDeviceId ==
                snapshot_.localDeviceId &&
            snapshot_.activeSharedDisplay.sessionDisplayId != 0 &&
            snapshot_.activeSharedDisplayLayoutVersion != 0;
        republishCursor =
            channel.state == DataChannelState::kOpen &&
            (channel.label == kControlReliableChannel ||
             channel.label == kTelemetryChannel) &&
            snapshot_.room.screenSharerDeviceId ==
                snapshot_.localDeviceId;
    }
    PublishSnapshot();
    if (clipboardWarmupPair) {
        StartClipboardWarmup(clipboardWarmupPair);
    }
    if (publishSharedDisplayLayout) {
        BroadcastSharedDisplayLayout();
        BroadcastSharedDisplayCatalog();
    }
    if (republishCursor) {
        RepublishRemoteCursor();
    }
}

void InProcessSessionEngine::StartClipboardWarmup(
    const std::shared_ptr<RoomPairRuntime>& pair)
{
    if (!pair) return;
    std::lock_guard lock(mutex_);
    const auto current = roomPairs_.find(pair->pairId);
    if (current == roomPairs_.end() || current->second != pair) return;
    // V8 no longer pushes a synthetic 4 MiB reliable burst. That burst could
    // not be retracted after cancel and continued starving screen video. The
    // normal Offer/Accept exchange is sufficient to initialize SCTP state.
    pair->clipboardWarmupRunning = false;
    pair->clipboardWarmupPending = false;
    pair->clipboardWarmupCancelled = true;
    pair->clipboardTransferPrimed = true;
}
