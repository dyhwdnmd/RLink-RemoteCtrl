// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#pragma once

#include <cstdint>
#include <span>
#include <string>
#include <vector>

namespace remote {

inline constexpr std::uint8_t kRemoteInputProtocolVersion = 3;
inline constexpr std::size_t kMaximumRemoteInputMessageBytes = 512;
inline constexpr std::uint8_t kRemoteMouseButtonsMask = 0x1f;

enum class RemoteInputMessageType : std::uint8_t {
    kMouseMove = 1,
    kMouseButton = 2,
    kMouseWheel = 3,
    kKey = 4,
    kReleaseAll = 5,
};

enum class RemoteMouseButton : std::uint8_t {
    kNone = 0,
    kLeft = 1,
    kRight = 2,
    kMiddle = 3,
    kX1 = 4,
    kX2 = 5,
};

// Coordinates are normalized to [0, 65535] so the transport is independent
// from the controller window size and the controlled display resolution.
// The UI renderer performs letterbox-aware mapping before creating an event.
struct RemoteInputEvent {
    RemoteInputMessageType type = RemoteInputMessageType::kMouseMove;
    std::uint32_t displayId = 0;
    std::uint64_t displayLayoutVersion = 0;
    std::uint16_t normalizedX = 0;
    std::uint16_t normalizedY = 0;
    RemoteMouseButton mouseButton = RemoteMouseButton::kNone;
    bool pressed = false;
    // Bit 0..4 represent left, right, middle, X1 and X2. Every pointer
    // message carries the complete post-event state so an unreliable move
    // can repair a delayed or lost button transition.
    std::uint8_t pressedMouseButtons = 0;
    std::int16_t wheelDeltaX = 0;
    std::int16_t wheelDeltaY = 0;
    std::uint16_t virtualKey = 0;
    std::uint16_t scanCode = 0;
    bool extendedKey = false;
    bool repeat = false;
    // Receiver-local metadata copied from the envelope after decoding. It is
    // not serialized inside the event payload.
    std::uint64_t deliverySequence = 0;
};

struct RemoteInputEnvelope {
    std::string roomId;
    std::string senderDeviceId;
    std::string controlGrantId;
    std::uint64_t sequence = 0;
    std::uint64_t timestampUs = 0;
    RemoteInputEvent event;
};

class IRemoteInputSink {
public:
    virtual ~IRemoteInputSink() = default;

    // Called only after the engine validates the room, pair identity,
    // control grant, channel and monotonically increasing sequence.
    virtual void OnRemoteInput(const RemoteInputEvent& event) = 0;
    virtual void ReleaseAllRemoteInputs() = 0;
};

bool UsesFastInputChannel(RemoteInputMessageType type);
bool IsPointerInput(RemoteInputMessageType type);

bool EncodeRemoteInput(const RemoteInputEnvelope& input,
                       std::vector<std::uint8_t>* encoded,
                       std::string* error = nullptr);

bool DecodeRemoteInput(std::span<const std::uint8_t> encoded,
                       RemoteInputEnvelope* input,
                       std::string* error = nullptr);

}  // namespace remote
