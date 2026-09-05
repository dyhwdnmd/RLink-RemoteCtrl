// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace remote {

// A process-local snapshot of one attached Windows display. stableDisplayKey
// is persisted in settings; sessionDisplayId and layoutVersion travel in the
// remote-input protocol so stale pointer packets cannot target another
// monitor after a topology change.
struct DisplayDescriptor {
    std::uint32_t sessionDisplayId = 0;
    std::string stableDisplayKey;
    std::string deviceName;
    std::string friendlyName;
    std::int32_t left = 0;
    std::int32_t top = 0;
    std::uint32_t width = 0;
    std::uint32_t height = 0;
    std::uint32_t dpiX = 96;
    std::uint32_t dpiY = 96;
    std::uint32_t scalePercent = 100;
    std::uint32_t rotationDegrees = 0;
    bool primary = false;
    std::uint64_t adapterLuid = 0;
    std::uint32_t outputIndex = 0;
    // HMONITOR is deliberately stored as an integer so this DTO remains free
    // of Windows headers. It is valid only for the current process/topology.
    std::uint64_t nativeMonitorHandle = 0;
};

struct DisplayTopologySnapshot {
    std::uint64_t layoutVersion = 0;
    std::vector<DisplayDescriptor> displays;
};

inline const DisplayDescriptor* FindDisplayByStableKey(
    const DisplayTopologySnapshot& topology,
    const std::string& stableDisplayKey)
{
    for (const auto& display : topology.displays) {
        if (display.stableDisplayKey == stableDisplayKey) {
            return &display;
        }
    }
    return nullptr;
}

inline const DisplayDescriptor* FindDisplayBySessionId(
    const DisplayTopologySnapshot& topology,
    std::uint32_t sessionDisplayId)
{
    for (const auto& display : topology.displays) {
        if (display.sessionDisplayId == sessionDisplayId) {
            return &display;
        }
    }
    return nullptr;
}

inline const DisplayDescriptor* FindPrimaryDisplay(
    const DisplayTopologySnapshot& topology)
{
    for (const auto& display : topology.displays) {
        if (display.primary) {
            return &display;
        }
    }
    return topology.displays.empty() ? nullptr : &topology.displays.front();
}

}  // namespace remote
