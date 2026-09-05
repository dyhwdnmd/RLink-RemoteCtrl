// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#include "ScreenStreamPolicy.h"

#include <algorithm>
#include <cstdint>

namespace remote {

ScreenStreamPolicyResult ResolveScreenStreamPolicy(
    std::uint32_t sourceWidth,
    std::uint32_t sourceHeight,
    const ScreenStreamPolicyRequest& request)
{
    ScreenStreamPolicyResult result;
    result.framesPerSecond = request.framesPerSecond;
    sourceWidth = (std::max)(sourceWidth, 2u);
    sourceHeight = (std::max)(sourceHeight, 2u);
    if (request.maxWidth == 0 || request.maxHeight == 0 ||
        (sourceWidth <= request.maxWidth &&
         sourceHeight <= request.maxHeight)) {
        result.width = sourceWidth;
        result.height = sourceHeight;
    } else {
        const double scale = (std::min)(
            static_cast<double>(request.maxWidth) / sourceWidth,
            static_cast<double>(request.maxHeight) / sourceHeight);
        result.width =
            static_cast<std::uint32_t>(sourceWidth * scale);
        result.height =
            static_cast<std::uint32_t>(sourceHeight * scale);
    }
    result.width = (std::max)(2u, result.width & ~1u);
    result.height = (std::max)(2u, result.height & ~1u);

    constexpr std::uint64_t kMinimumStartBitrateBps = 2'000'000;
    constexpr std::uint64_t kMinimumBitrateBps = 4'000'000;
    constexpr std::uint64_t kMaximumBitrateBps = 100'000'000;
    const std::uint64_t pixelRate =
        static_cast<std::uint64_t>(result.width) * result.height *
        result.framesPerSecond;
    result.maxBitrateBps = static_cast<std::uint32_t>(std::clamp(
        pixelRate * 15 / 100,
        kMinimumBitrateBps,
        kMaximumBitrateBps));
    result.startBitrateBps = static_cast<std::uint32_t>(std::clamp(
        pixelRate * 8 / 100,
        kMinimumStartBitrateBps,
        static_cast<std::uint64_t>(result.maxBitrateBps)));
    return result;
}

}  // namespace remote
