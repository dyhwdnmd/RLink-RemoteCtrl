// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#pragma once

#include <memory>
#include <string>

#include "api/video_codecs/video_decoder.h"

namespace remote {

inline constexpr char kFfmpegD3D11H264DecoderName[] =
    "FFmpeg D3D11VA H264 Hardware Decoder";

class FfmpegD3D11H264Decoder final : public webrtc::VideoDecoder {
public:
    FfmpegD3D11H264Decoder();
    ~FfmpegD3D11H264Decoder() override;

    FfmpegD3D11H264Decoder(const FfmpegD3D11H264Decoder&) = delete;
    FfmpegD3D11H264Decoder& operator=(
        const FfmpegD3D11H264Decoder&) = delete;

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
    bool IsHardwareAccelerated() const noexcept;
    bool IsAsynchronous() const noexcept;
    const std::string& LastError() const noexcept;

private:
    class Impl;
    std::unique_ptr<Impl> impl_;
};

}  // namespace remote
