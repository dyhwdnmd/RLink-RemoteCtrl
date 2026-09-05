// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#pragma once

#include <memory>

#include "api/video_codecs/video_encoder.h"
#include "api/video_codecs/video_encoder_factory.h"
#include "src/webrtc/VideoEncoderRuntimeStatus.h"

namespace remote {

// Single-stream OpenH264 encoder used by RemoteC desktop sharing.  WebRTC's
// built-in wrapper does not expose OpenH264's iComplexityMode, so this small
// adapter keeps WebRTC rate control while applying the shared quality setting.
class QualityOpenH264Encoder final : public webrtc::VideoEncoder {
public:
    explicit QualityOpenH264Encoder(
        FfmpegX264Preset quality = FfmpegX264Preset::kMedium);
    ~QualityOpenH264Encoder() override;

    int InitEncode(const webrtc::VideoCodec* codecSettings,
                   const Settings& settings) override;
    int32_t RegisterEncodeCompleteCallback(
        webrtc::EncodedImageCallback* callback) override;
    int32_t Release() override;
    int32_t Encode(
        const webrtc::VideoFrame& frame,
        const std::vector<webrtc::VideoFrameType>* frameTypes) override;
    void SetRates(const RateControlParameters& parameters) override;
    EncoderInfo GetEncoderInfo() const override;

private:
    class Impl;
    std::unique_ptr<Impl> impl_;
};

class QualityOpenH264EncoderFactory final
    : public webrtc::VideoEncoderFactory {
public:
    explicit QualityOpenH264EncoderFactory(
        FfmpegX264Preset quality = FfmpegX264Preset::kMedium)
        : quality_(quality)
    {}

    std::vector<webrtc::SdpVideoFormat> GetSupportedFormats() const override;
    CodecSupport QueryCodecSupport(
        const webrtc::SdpVideoFormat& format,
        std::optional<std::string> scalabilityMode,
        std::optional<webrtc::Resolution> resolution) const override;
    std::unique_ptr<webrtc::VideoEncoder> Create(
        const webrtc::Environment& environment,
        const webrtc::SdpVideoFormat& format) override;

private:
    FfmpegX264Preset quality_ = FfmpegX264Preset::kMedium;
};

}  // namespace remote
