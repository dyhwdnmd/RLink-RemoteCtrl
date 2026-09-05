// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#pragma once

#include <string>

namespace remote {

struct MfD3D11H264DecoderSelfTestResult {
    bool passed = false;
    std::string report;
};

MfD3D11H264DecoderSelfTestResult RunMfD3D11H264DecoderSelfTest(
    const std::wstring& annexBFilePath);

}  // namespace remote
