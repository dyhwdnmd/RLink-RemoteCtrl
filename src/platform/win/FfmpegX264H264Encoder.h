// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#pragma once

#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>

#include "api/video_codecs/video_encoder.h"
#include "src/webrtc/VideoEncoderRuntimeStatus.h"

namespace remote {

struct FfmpegX264EncoderSelfTestResult {
    bool passed = false;
    std::uint32_t encodedFrames = 0;
    std::size_t encodedBytes = 0;
    std::string error;
};

// CPU H264 encoder backed by the bundled FFmpeg libavcodec/libx264 runtime.
// It is deliberately an explicit software option; the existing automatic
// hardware -> FFmpeg/libx264 fallback chain remains unchanged.
class FfmpegX264H264Encoder final : public webrtc::VideoEncoder {
public:
    explicit FfmpegX264H264Encoder(
        FfmpegX264Preset preset = FfmpegX264Preset::kMedium);
    ~FfmpegX264H264Encoder() override;

    static bool RuntimeAvailable(std::string* error = nullptr);

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

FfmpegX264EncoderSelfTestResult RunFfmpegX264EncoderSelfTest();

}  // namespace remote
