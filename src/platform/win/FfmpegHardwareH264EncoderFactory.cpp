// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#include "FfmpegHardwareH264EncoderFactory.h"

#include "FfmpegHardwareH264Encoder.h"
#include "modules/video_coding/codecs/h264/include/h264.h"

namespace remote {

FfmpegHardwareH264EncoderFactory::FfmpegHardwareH264EncoderFactory(
    FfmpegHardwareBackend backend,
    std::shared_ptr<VideoEncoderRuntimeState> runtimeState,
    FfmpegX264Preset quality)
    : backend_(backend), runtimeState_(std::move(runtimeState)),
      quality_(quality)
{}

std::vector<webrtc::SdpVideoFormat>
FfmpegHardwareH264EncoderFactory::GetSupportedFormats() const
{
    if (!FfmpegHardwareH264Encoder::RuntimeAvailable(backend_)) {
        return {};
    }
    return webrtc::SupportedH264Codecs();
}

webrtc::VideoEncoderFactory::CodecSupport
FfmpegHardwareH264EncoderFactory::QueryCodecSupport(
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
        .is_power_efficient = true,
    };
}

std::unique_ptr<webrtc::VideoEncoder>
FfmpegHardwareH264EncoderFactory::Create(
    const webrtc::Environment& /* environment */,
    const webrtc::SdpVideoFormat& format)
{
    if (!QueryCodecSupport(
            format, std::nullopt, std::nullopt).is_supported) {
        return nullptr;
    }
    return std::make_unique<FfmpegHardwareH264Encoder>(
        backend_, runtimeState_, quality_);
}

}  // namespace remote
