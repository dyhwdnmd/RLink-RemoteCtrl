// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#pragma once

#include <memory>
#include <mutex>
#include <optional>
#include <string>
#include <vector>

#include "api/video/resolution.h"
#include "api/video_codecs/video_decoder_factory.h"
#include "MfD3D11H264Decoder.h"

namespace remote {

class MfD3D11H264DecoderPreferenceState final {
public:
    void SetPreferredDecoderName(std::string name);
    std::string PreferredDecoderName() const;

private:
    mutable std::mutex mutex_;
    std::string preferredDecoderName_;
};

class MfD3D11H264DecoderFactory final : public webrtc::VideoDecoderFactory {
public:
    explicit MfD3D11H264DecoderFactory(
        MfD3D11H264DecoderSelection selection =
            MfD3D11H264DecoderSelection::kHardwareOnly,
        std::shared_ptr<MfD3D11H264DecoderPreferenceState>
            preferenceState = nullptr);

    std::vector<webrtc::SdpVideoFormat> GetSupportedFormats() const override;

    CodecSupport QueryCodecSupport(
        const webrtc::SdpVideoFormat& format,
        bool referenceScaling,
        std::optional<webrtc::Resolution> resolution) const override;

    std::unique_ptr<webrtc::VideoDecoder> Create(
        const webrtc::Environment& environment,
        const webrtc::SdpVideoFormat& format) override;

private:
    MfD3D11H264DecoderSelection selection_;
    std::shared_ptr<MfD3D11H264DecoderPreferenceState> preferenceState_;
};

}  // namespace remote
