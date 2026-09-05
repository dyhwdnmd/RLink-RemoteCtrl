// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#pragma once

#include <cstdint>
#include <string>

namespace remote {

struct MfH264EncoderSelfTestResult {
    bool passed = false;
    bool asynchronousTransform = false;
    bool d3d11PathUsed = false;
    bool dynamicBitrateSupported = false;
    bool dynamicBitrateChanged = false;
    uint32_t submittedFrames = 0;
    uint32_t encodedSamples = 0;
    uint32_t keyFrames = 0;
    uint64_t encodedBytes = 0;
    double elapsedMilliseconds = 0.0;
    std::string encoderName;
    std::string report;
};

MfH264EncoderSelfTestResult RunMfH264EncoderD3D11SelfTest();

}  // namespace remote
