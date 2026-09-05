// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#include "FfmpegX264H264EncoderFactory.h"

#include "FfmpegX264H264Encoder.h"
#include "modules/video_coding/codecs/h264/include/h264.h"

namespace remote {

std::vector<webrtc::SdpVideoFormat>
FfmpegX264H264EncoderFactory::GetSupportedFormats() const
{
    if (!FfmpegX264H264Encoder::RuntimeAvailable()) {
        return {};
    }
    return webrtc::SupportedH264Codecs();
}

webrtc::VideoEncoderFactory::CodecSupport
FfmpegX264H264EncoderFactory::QueryCodecSupport(
    const webrtc::SdpVideoFormat& format,
    std::optional<std::string> scalabilityMode,
    std::optional<webrtc::Resolution> /* resolution */) const
{
    if ((scalabilityMode && *scalabilityMode != "L1T1") ||
        !format.IsCodecInList(GetSupportedFormats())) {
        return {};
    }
    return {
        .is_supported = true,
        .is_power_efficient = false,
    };
}

std::unique_ptr<webrtc::VideoEncoder>
FfmpegX264H264EncoderFactory::Create(
    const webrtc::Environment& /* environment */,
    const webrtc::SdpVideoFormat& format)
{
    if (!QueryCodecSupport(
            format, std::nullopt, std::nullopt).is_supported) {
        return nullptr;
    }
    return std::make_unique<FfmpegX264H264Encoder>(preset_);
}

}  // namespace remote
