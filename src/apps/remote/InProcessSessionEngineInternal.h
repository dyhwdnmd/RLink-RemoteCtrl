// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#pragma once

#include <cstdint>
#include <memory>
#include <span>
#include <string>
#include <unordered_map>

#include "InProcessSessionEngine.h"
#include "src/webrtc/LibWebRtcSession.h"

namespace remote::app {

class InProcessSessionEngine::RoomPairBridge final
    : public ISessionSignalingSender,
      public ISessionControllerObserver {
public:
    RoomPairBridge(InProcessSessionEngine& engine, std::string pairId)
        : engine_(engine), pairId_(std::move(pairId))
    {}

    bool SendDescription(
        const SessionDescription& description) override
    {
        return engine_.SendRoomPairDescription(pairId_, description);
    }

    bool SendIceCandidate(const IceCandidate& candidate) override
    {
        return engine_.SendRoomPairIceCandidate(pairId_, candidate);
    }

    bool RequestIceRestart(std::uint64_t observedGeneration,
                           std::uint64_t requestSequence) override
    {
        return engine_.SendRoomPairIceRestartRequest(
            pairId_, observedGeneration, requestSequence);
    }

    bool CancelIceRestart(std::uint64_t observedGeneration,
                          std::uint64_t requestSequence) override
    {
        return engine_.SendRoomPairIceRestartCancel(
            pairId_, observedGeneration, requestSequence);
    }

    void OnControllerSnapshot(
        const SessionControllerSnapshot& snapshot) override
    {
        engine_.OnRoomPairControllerSnapshot(pairId_, snapshot);
    }

    void OnDataChannelStateChanged(
        const DataChannelInfo& channel) override
    {
        engine_.OnRoomPairDataChannelStateChanged(pairId_, channel);
    }

    void OnDataMessage(const std::string& label,
                       std::span<const std::uint8_t> payload,
                       bool binary) override
    {
        engine_.OnRoomPairDataMessage(
            pairId_, label, payload, binary);
    }

    void OnRemoteTrackAdded(const RemoteTrackInfo& track) override
    {
        engine_.OnRoomPairRemoteTrackAdded(pairId_, track);
    }

private:
    InProcessSessionEngine& engine_;
    std::string pairId_;
};

struct InProcessSessionEngine::RoomPairRuntime {
    std::string pairId;
    std::string roomId;
    std::string peerDeviceId;
    bool localIsOfferer = false;
    bool offerNegotiationStarted = false;
    bool mediaSlotsPreparing = false;
    bool mediaSlotsPrepared = false;
    bool clipboardTransferPrimed = false;
    bool clipboardWarmupPending = false;
    bool clipboardWarmupRunning = false;
    bool clipboardWarmupCancelled = false;
    std::uint64_t screenFirstFramePresentedGeneration = 0;
    std::uint32_t screenFirstFrameStartupMs = 0;
    std::uint32_t screenStartupRefreshRequests = 0;
    std::uint64_t screenStartupRefreshGeneration = 0;
    std::unordered_map<std::string, bool> openDataChannels;
    std::uint64_t lastFastInputSequence = 0;
    std::uint64_t lastReliableInputSequence = 0;
    // Mouse-button state transitions can be duplicated onto the reliable
    // channel. Track only state transitions here: comparing them with newer
    // unordered move packets can incorrectly discard the reliable button-up
    // fallback and leave Windows believing that a button is still pressed.
    std::uint64_t lastPointerStateSequence = 0;
    std::uint64_t lastCursorPositionSequence = 0;
    std::uint64_t lastCursorShapeSequence = 0;
    std::uint64_t lastScreenControlSequence = 0;
    std::uint64_t nextRoomMemberActionSequence = 0;
    std::uint64_t lastRoomMemberActionSequence = 0;
    std::uint64_t nextFileTransferSequence = 0;
    std::uint64_t lastFileTransferSequence = 0;
    std::uint64_t nextClipboardReliableSequence = 0;
    std::uint64_t lastClipboardReliableSequence = 0;
    std::uint64_t nextClipboardTransferSequence = 0;
    std::uint64_t lastClipboardTransferSequence = 0;
    std::unique_ptr<RoomPairBridge> bridge;
    std::unique_ptr<LibWebRtcSession> session;
    std::unique_ptr<SessionControllerBase> controller;
};

}  // namespace remote::app
