// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#include "MfD3D11H264DecoderFactory.h"

#include "MfD3D11H264Decoder.h"
#include "FfmpegD3D11H264Decoder.h"
#include "modules/video_coding/codecs/h264/include/h264.h"

namespace remote {

void MfD3D11H264DecoderPreferenceState::SetPreferredDecoderName(
    std::string name)
{
    std::lock_guard lock(mutex_);
    preferredDecoderName_ = std::move(name);
}

std::string
MfD3D11H264DecoderPreferenceState::PreferredDecoderName() const
{
    std::lock_guard lock(mutex_);
    return preferredDecoderName_;
}

MfD3D11H264DecoderFactory::MfD3D11H264DecoderFactory(
    MfD3D11H264DecoderSelection selection,
    std::shared_ptr<MfD3D11H264DecoderPreferenceState> preferenceState)
    : selection_(selection),
      preferenceState_(std::move(preferenceState))
{}

std::vector<webrtc::SdpVideoFormat>
MfD3D11H264DecoderFactory::GetSupportedFormats() const
{
    return webrtc::SupportedH264DecoderCodecs();
}

webrtc::VideoDecoderFactory::CodecSupport
MfD3D11H264DecoderFactory::QueryCodecSupport(
    const webrtc::SdpVideoFormat& format,
    bool referenceScaling,
    std::optional<webrtc::Resolution> /* resolution */) const
{
    if (referenceScaling || !format.IsCodecInList(GetSupportedFormats())) {
        return {};
    }
    return {
        .is_supported = true,
        // Preserve the preferred Windows MF/D3D11 route during codec
        // negotiation. The configured decoder reports its actual hardware
        // status later through VideoDecoder::GetDecoderInfo().
        .is_power_efficient = true,
    };
}

std::unique_ptr<webrtc::VideoDecoder>
MfD3D11H264DecoderFactory::Create(
    const webrtc::Environment& /* environment */,
    const webrtc::SdpVideoFormat& format)
{
    if (!QueryCodecSupport(format, false, std::nullopt).is_supported) {
        return nullptr;
    }
    const std::string preferredName = preferenceState_
        ? preferenceState_->PreferredDecoderName()
        : std::string{};
    if (preferenceState_ && preferredName.empty()) {
        // Hardware is exposed only after the isolated benchmark has selected
        // a concrete backend for the current hardware fingerprint. Keeping
        // the factory itself wired allows a just-completed benchmark to take
        // effect for the next decoder instance without restarting RemoteC.
        return nullptr;
    }
    if (preferredName ==
            kFfmpegD3D11H264DecoderName) {
        return std::make_unique<FfmpegD3D11H264Decoder>();
    }
    return std::make_unique<MfD3D11H264Decoder>(
        selection_,
        preferredName);
}

}  // namespace remote
