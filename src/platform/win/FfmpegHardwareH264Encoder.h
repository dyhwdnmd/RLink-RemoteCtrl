// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#pragma once

#include <memory>
#include <string>
#include <vector>

#include "api/video_codecs/video_encoder.h"
#include "src/webrtc/VideoEncoderRuntimeStatus.h"

namespace remote {

struct FfmpegHardwareEncoderAvailability {
    FfmpegHardwareBackend backend = FfmpegHardwareBackend::kAutomatic;
    bool compiled = false;
    bool driverRuntimePresent = false;
    std::string implementation;
    std::string detail;
};

struct FfmpegHardwareEncoderSelfTestResult {
    bool succeeded = false;
    FfmpegHardwareBackend backend = FfmpegHardwareBackend::kAutomatic;
    std::string implementation;
    std::string error;
    std::uint32_t frames = 0;
    std::uint32_t keyFrames = 0;
    std::size_t encodedBytes = 0;
};

// Hardware H264 encoder backed by FFmpeg's QSV, NVENC or AMF wrappers.
// Native DXGI BGRA textures stay on their capture adapter and are converted to
// D3D11 NV12 hardware frames when the selected FFmpeg backend exposes the
// D3D11 hw-frames interface. CPU/libwebrtc inputs and unsupported backends keep
// the deterministic CPU NV12 upload path. A fatal hardware error requests
// WebRTC's one-way software fallback; the backend is not probed every frame.
class FfmpegHardwareH264Encoder final : public webrtc::VideoEncoder {
public:
    explicit FfmpegHardwareH264Encoder(
        FfmpegHardwareBackend backend = FfmpegHardwareBackend::kAutomatic,
        std::shared_ptr<VideoEncoderRuntimeState> runtimeState = {},
        FfmpegX264Preset quality = FfmpegX264Preset::kMedium);
    ~FfmpegHardwareH264Encoder() override;

    static std::vector<FfmpegHardwareEncoderAvailability>
        EnumerateAvailability();
    static bool RuntimeAvailable(
        FfmpegHardwareBackend backend = FfmpegHardwareBackend::kAutomatic,
        std::string* error = nullptr);

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
    std::string LastError() const;

private:
    class Impl;
    std::unique_ptr<Impl> impl_;
};

FfmpegHardwareEncoderSelfTestResult RunFfmpegHardwareEncoderSelfTest(
    FfmpegHardwareBackend backend = FfmpegHardwareBackend::kAutomatic);

}  // namespace remote
