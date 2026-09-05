// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#pragma once

#include <memory>

#include "api/video_codecs/video_encoder_factory.h"
#include "src/webrtc/VideoEncoderRuntimeStatus.h"

namespace remote {

class FfmpegHardwareH264EncoderFactory final
    : public webrtc::VideoEncoderFactory {
public:
    explicit FfmpegHardwareH264EncoderFactory(
        FfmpegHardwareBackend backend = FfmpegHardwareBackend::kAutomatic,
        std::shared_ptr<VideoEncoderRuntimeState> runtimeState = {},
        FfmpegX264Preset quality = FfmpegX264Preset::kMedium);

    std::vector<webrtc::SdpVideoFormat> GetSupportedFormats() const override;
    CodecSupport QueryCodecSupport(
        const webrtc::SdpVideoFormat& format,
        std::optional<std::string> scalabilityMode,
        std::optional<webrtc::Resolution> resolution) const override;
    std::unique_ptr<webrtc::VideoEncoder> Create(
        const webrtc::Environment& environment,
        const webrtc::SdpVideoFormat& format) override;

private:
    FfmpegHardwareBackend backend_ = FfmpegHardwareBackend::kAutomatic;
    std::shared_ptr<VideoEncoderRuntimeState> runtimeState_;
    FfmpegX264Preset quality_ = FfmpegX264Preset::kMedium;
};

}  // namespace remote
