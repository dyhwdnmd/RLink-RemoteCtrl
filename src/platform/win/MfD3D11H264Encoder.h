// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#pragma once

#include <memory>

#include "api/video_codecs/video_encoder.h"
#include "src/webrtc/VideoEncoderRuntimeStatus.h"

namespace remote {

// Windows H264 encoder backed by a D3D11-aware Media Foundation hardware MFT.
// It accepts DesktopBgraFrameBuffer without an eager I420 conversion. Other
// buffer types deliberately request WebRTC's software fallback.
class MfD3D11H264Encoder final : public webrtc::VideoEncoder {
public:
    MfD3D11H264Encoder();
    explicit MfD3D11H264Encoder(
        std::shared_ptr<VideoEncoderRuntimeState> runtimeState,
        FfmpegX264Preset quality = FfmpegX264Preset::kMedium);
    ~MfD3D11H264Encoder() override;

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

}  // namespace remote
