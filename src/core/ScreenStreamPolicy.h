// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#pragma once

#include <cstdint>

namespace remote {

struct ScreenStreamPolicyRequest {
    std::uint32_t maxWidth = 0;
    std::uint32_t maxHeight = 0;
    std::uint32_t framesPerSecond = 60;
};

struct ScreenStreamPolicyResult {
    std::uint32_t width = 0;
    std::uint32_t height = 0;
    std::uint32_t framesPerSecond = 60;
    std::uint32_t startBitrateBps = 0;
    std::uint32_t maxBitrateBps = 0;
};

// Resolves the encoded desktop dimensions and bitrate without depending on
// capture, signaling, Qt, or WebRTC state.
ScreenStreamPolicyResult ResolveScreenStreamPolicy(
    std::uint32_t sourceWidth,
    std::uint32_t sourceHeight,
    const ScreenStreamPolicyRequest& request);

}  // namespace remote
