// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace remote {

struct MfH264EncoderCapability {
    std::string name;
    std::string clsid;
    std::string hardwareUrl;

    bool asynchronous = false;
    bool d3d11Aware = false;
    bool d3d11DeviceManagerAccepted = false;
    bool cpuNv12InputSupported = false;
    bool lowLatencyModeSupported = false;
    bool dynamicBitrateSupported = false;
    bool dynamicBitrateModifiable = false;
    bool dynamicFormatChangeSupported = false;

    uint32_t maximumWidth = 0;
    uint32_t maximumHeight = 0;
    uint32_t maximumFrameRateNumerator = 0;
    uint32_t maximumFrameRateDenominator = 0;
    std::vector<uint32_t> h264Profiles;
    std::vector<std::string> warnings;
};

struct MfH264EncoderCapabilityReport {
    bool mediaFoundationStarted = false;
    bool d3d11ProbeDeviceCreated = false;
    bool enumerationSucceeded = false;
    uint32_t hardwareEncoderCount = 0;
    std::vector<MfH264EncoderCapability> encoders;
    std::vector<std::string> warnings;
};

// Persistent cache stores descriptors only. Hardware MFTs, COM interfaces and
// D3D11 objects are recreated for every process/session.
struct MfH264EncoderCapabilityCache {
    std::string hardwareFingerprint;
    bool valid = false;
    bool hardwareEncoderAvailable = false;
    bool cpuNv12InputSupported = false;
    bool d3d11InputCandidate = false;
    uint32_t hardwareEncoderCount = 0;
    std::vector<std::string> descriptions;
    std::vector<std::string> warnings;
};

MfH264EncoderCapabilityReport ProbeMfH264EncoderCapabilities();

}  // namespace remote
