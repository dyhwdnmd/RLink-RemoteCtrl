// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#pragma once

#include <cstdint>
#include <span>
#include <string>
#include <vector>

namespace remote {

inline constexpr std::uint32_t kMaximumRemoteCursorDimension = 256;
inline constexpr std::size_t kMaximumRemoteCursorPixelBytes =
    static_cast<std::size_t>(kMaximumRemoteCursorDimension) *
    kMaximumRemoteCursorDimension * 4;

enum class RemoteCursorMessageType : std::uint8_t {
    kPosition = 1,
    kShape = 2,
    kReset = 3,
};

struct RemoteCursorPosition {
    std::uint32_t displayId = 0;
    std::uint64_t displayLayoutVersion = 0;
    std::uint16_t normalizedX = 0;
    std::uint16_t normalizedY = 0;
    bool visible = false;
    std::uint64_t shapeId = 0;
    // Highest controller input sequence known to have reached the controlled
    // machine. It lets viewers and diagnostics correlate the authoritative
    // cursor echo with recently transmitted input.
    std::uint64_t lastAppliedInputSequence = 0;
};

struct RemoteCursorShape {
    std::uint64_t shapeId = 0;
    std::uint16_t width = 0;
    std::uint16_t height = 0;
    std::uint16_t hotspotX = 0;
    std::uint16_t hotspotY = 0;
    // Premultiplied BGRA pixels in top-to-bottom row order.
    std::vector<std::uint8_t> premultipliedBgra;
    // One byte per pixel. Zero uses premultipliedBgra; non-zero requests the
    // legacy Windows XOR/invert cursor operation. Empty means no invert pixels.
    std::vector<std::uint8_t> invertMask;
};

struct RemoteCursorEnvelope {
    RemoteCursorMessageType type = RemoteCursorMessageType::kPosition;
    // Direct sessions use the signaling session id; rooms use the room id.
    std::string contextId;
    std::string senderDeviceId;
    std::uint64_t sequence = 0;
    std::uint64_t timestampUs = 0;
    std::uint64_t screenShareGeneration = 0;
    RemoteCursorPosition position;
    RemoteCursorShape shape;
};

bool EncodeRemoteCursorMessage(
    const RemoteCursorEnvelope& envelope,
    std::vector<std::uint8_t>* output,
    std::string* error = nullptr);

bool DecodeRemoteCursorMessage(
    std::span<const std::uint8_t> payload,
    RemoteCursorEnvelope* envelope,
    std::string* error = nullptr);

}  // namespace remote
