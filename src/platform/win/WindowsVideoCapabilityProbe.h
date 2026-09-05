// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace remote {

struct WindowsVideoCapabilityReport {
    bool d3d11DeviceCreated = false;
    bool d3d11VideoDeviceAvailable = false;
    bool nv12TextureSupported = false;
    uint32_t featureLevel = 0;
    uint32_t adapterLuidLowPart = 0;
    int32_t adapterLuidHighPart = 0;
    std::wstring adapterName;

    uint32_t h264DecoderMftCount = 0;
    uint32_t h264HardwareRegisteredMftCount = 0;
    uint32_t h264D3D11AwareDecoderCount = 0;
    std::vector<std::wstring> h264DecoderMftNames;
    std::vector<std::string> warnings;
};

WindowsVideoCapabilityReport ProbeWindowsVideoCapabilities();

}  // namespace remote
