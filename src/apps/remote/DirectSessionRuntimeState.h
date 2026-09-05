// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#pragma once

#include <cstdint>
#include <string>
#include <string_view>
#include <unordered_map>

#include "api/media_stream_interface.h"
#include "api/scoped_refptr.h"
#include "src/protocol/ScreenShareControlProtocol.h"

namespace remote::app {

// Mutable transport/media state that belongs to one direct session. Keeping it
// together prevents owned-device and verification-code entry paths from
// growing separate state machines while they share the same WebRTC pipeline.
struct DirectSessionRuntimeState {
    bool audioDevicesApplied = false;
    bool mediaSlotsPreparing = false;
    bool mediaSlotsPrepared = false;
    std::unordered_map<std::string, bool> openDataChannels;
    webrtc::scoped_refptr<webrtc::MediaStreamTrackInterface> screenTrack;
    std::uint64_t nextInputSequence = 0;
    std::uint64_t nextScreenControlSequence = 0;
    std::uint64_t lastScreenControlSequence = 0;
    std::uint64_t lastFastInputSequence = 0;
    std::uint64_t lastReliableInputSequence = 0;
    std::uint64_t lastPointerStateSequence = 0;
    std::uint64_t lastCursorPositionSequence = 0;
    std::uint64_t lastCursorShapeSequence = 0;
    std::uint64_t nextFileTransferSequence = 0;
    std::uint64_t lastFileTransferSequence = 0;
    std::uint64_t nextClipboardReliableSequence = 0;
    std::uint64_t lastClipboardReliableSequence = 0;
    std::uint64_t nextClipboardTransferSequence = 0;
    std::uint64_t lastClipboardTransferSequence = 0;
    bool screenPreferenceApplied = false;
    ScreenStreamPreferenceRequest screenPreference;

    [[nodiscard]] bool IsChannelOpen(std::string_view label) const
    {
        const auto it = openDataChannels.find(std::string(label));
        return it != openDataChannels.end() && it->second;
    }

    void Reset()
    {
        audioDevicesApplied = false;
        mediaSlotsPreparing = false;
        mediaSlotsPrepared = false;
        openDataChannels.clear();
        screenTrack = nullptr;
        nextInputSequence = 0;
        nextScreenControlSequence = 0;
        lastScreenControlSequence = 0;
        lastFastInputSequence = 0;
        lastReliableInputSequence = 0;
        lastPointerStateSequence = 0;
        lastCursorPositionSequence = 0;
        lastCursorShapeSequence = 0;
        nextFileTransferSequence = 0;
        lastFileTransferSequence = 0;
        nextClipboardReliableSequence = 0;
        lastClipboardReliableSequence = 0;
        nextClipboardTransferSequence = 0;
        lastClipboardTransferSequence = 0;
        screenPreferenceApplied = false;
        screenPreference = {};
    }
};

}  // namespace remote::app
