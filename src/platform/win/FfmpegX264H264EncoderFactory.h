// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#pragma once

#include "api/video_codecs/video_encoder_factory.h"
#include "src/webrtc/VideoEncoderRuntimeStatus.h"

namespace remote {

class FfmpegX264H264EncoderFactory final
    : public webrtc::VideoEncoderFactory {
public:
    explicit FfmpegX264H264EncoderFactory(
        FfmpegX264Preset preset = FfmpegX264Preset::kMedium)
        : preset_(preset)
    {}

    std::vector<webrtc::SdpVideoFormat>
    GetSupportedFormats() const override;

    CodecSupport QueryCodecSupport(
        const webrtc::SdpVideoFormat& format,
        std::optional<std::string> scalabilityMode,
        std::optional<webrtc::Resolution> resolution) const override;

    std::unique_ptr<webrtc::VideoEncoder> Create(
        const webrtc::Environment& environment,
        const webrtc::SdpVideoFormat& format) override;

private:
    FfmpegX264Preset preset_ = FfmpegX264Preset::kMedium;
};

}  // namespace remote
