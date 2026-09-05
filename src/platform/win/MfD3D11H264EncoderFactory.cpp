// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#include "MfD3D11H264EncoderFactory.h"

#include "MfD3D11H264Encoder.h"
#include "modules/video_coding/codecs/h264/include/h264.h"

namespace remote {

MfD3D11H264EncoderFactory::MfD3D11H264EncoderFactory(
    std::shared_ptr<VideoEncoderRuntimeState> runtimeState,
    FfmpegX264Preset quality)
    : runtimeState_(std::move(runtimeState)), quality_(quality)
{}

std::vector<webrtc::SdpVideoFormat>
MfD3D11H264EncoderFactory::GetSupportedFormats() const
{
    return webrtc::SupportedH264Codecs();
}

webrtc::VideoEncoderFactory::CodecSupport
MfD3D11H264EncoderFactory::QueryCodecSupport(
    const webrtc::SdpVideoFormat& format,
    std::optional<std::string> scalabilityMode,
    std::optional<webrtc::Resolution> /* resolution */) const
{
    if (scalabilityMode &&
        *scalabilityMode != "L1T1") {
        return {};
    }
    if (!format.IsCodecInList(GetSupportedFormats())) {
        return {};
    }
    return {
        .is_supported = true,
        .is_power_efficient = true,
    };
}

std::unique_ptr<webrtc::VideoEncoder>
MfD3D11H264EncoderFactory::Create(
    const webrtc::Environment& /* environment */,
    const webrtc::SdpVideoFormat& format)
{
    if (!QueryCodecSupport(
            format, std::nullopt, std::nullopt).is_supported) {
        return nullptr;
    }
    return std::make_unique<MfD3D11H264Encoder>(
        runtimeState_, quality_);
}

}  // namespace remote
