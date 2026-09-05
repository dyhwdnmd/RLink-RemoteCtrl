// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#pragma once

#include <cstdint>
#include <memory>
#include <string>

#include "api/scoped_refptr.h"
#include "src/core/DisplayTopology.h"

namespace remote {

class D3D11DesktopFrameBuffer;

// Captures one selected Windows display as an immutable D3D11 BGRA texture.
// It deliberately performs a GPU CopyResource into an application-owned
// texture before releasing the Desktop Duplication frame, but never maps the
// texture to CPU memory on the hardware encoder path.
class DxgiNativeDesktopCapturer final {
public:
    enum class Result {
        kSuccess,
        kTimeout,
        kFailed,
    };

    DxgiNativeDesktopCapturer();
    ~DxgiNativeDesktopCapturer();

    bool Initialize(const DisplayDescriptor& target);
    Result Capture(
        std::uint32_t timeoutMs,
        webrtc::scoped_refptr<D3D11DesktopFrameBuffer>* frame);
    std::string LastError() const;

private:
    class Impl;
    std::unique_ptr<Impl> impl_;
};

}  // namespace remote
