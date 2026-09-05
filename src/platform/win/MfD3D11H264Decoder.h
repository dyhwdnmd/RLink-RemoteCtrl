// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#pragma once

#include <cstdint>
#include <memory>
#include <string>

#include "api/video_codecs/video_decoder.h"

namespace remote {

enum class MfD3D11H264DecoderKind {
    kUnknown,
    kHardware,
    kSoftware,
};

enum class MfD3D11H264DecoderSelection {
    kHardwareOnly,
    kSoftwareOnly,
    kAny,
};

class MfD3D11H264Decoder final : public webrtc::VideoDecoder {
public:
    explicit MfD3D11H264Decoder(
        MfD3D11H264DecoderSelection selection =
            MfD3D11H264DecoderSelection::kHardwareOnly,
        std::string preferredDecoderName = {},
        bool requirePreferredDecoder = false);
    ~MfD3D11H264Decoder() override;

    MfD3D11H264Decoder(const MfD3D11H264Decoder&) = delete;
    MfD3D11H264Decoder& operator=(const MfD3D11H264Decoder&) = delete;

    bool Configure(const Settings& settings) override;
    int32_t Decode(const webrtc::EncodedImage& inputImage,
                   int64_t renderTimeMs) override;
    int32_t RegisterDecodeCompleteCallback(
        webrtc::DecodedImageCallback* callback) override;
    int32_t Release() override;
    DecoderInfo GetDecoderInfo() const override;
    const char* ImplementationName() const override;

    bool IsConfigured() const noexcept;
    bool SupportsNativeOutputSamples() const noexcept;
    MfD3D11H264DecoderKind DecoderKind() const noexcept;
    bool IsHardwareAccelerated() const noexcept;
    bool IsAsynchronous() const noexcept;
    const std::string& LastError() const noexcept;
    const std::string& DecoderName() const noexcept;

private:
    class Impl;
    std::unique_ptr<Impl> impl_;
};

}  // namespace remote
