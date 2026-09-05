// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#pragma once

#include <memory>
#include <mutex>
#include <optional>
#include <vector>

#include "api/video_codecs/video_encoder_factory.h"
#include "VideoEncoderRuntimeStatus.h"

namespace remote {

class WindowsPreferredVideoEncoderFactoryState;

// Prefers the Windows Media Foundation/D3D11 H264 encoder and delegates
// runtime failure to WebRTC's official software-fallback wrapper.
class WindowsPreferredVideoEncoderFactory final
    : public webrtc::VideoEncoderFactory {
public:
    WindowsPreferredVideoEncoderFactory(
        std::unique_ptr<webrtc::VideoEncoderFactory> softwareFactory,
        std::unique_ptr<webrtc::VideoEncoderFactory> hardwareFactory,
        std::shared_ptr<VideoEncoderRuntimeState> runtimeState = nullptr);
    ~WindowsPreferredVideoEncoderFactory() override;

    std::vector<webrtc::SdpVideoFormat>
    GetSupportedFormats() const override;
    CodecSupport QueryCodecSupport(
        const webrtc::SdpVideoFormat& format,
        std::optional<std::string> scalabilityMode,
        std::optional<webrtc::Resolution> resolution) const override;
    std::unique_ptr<webrtc::VideoEncoder> Create(
        const webrtc::Environment& environment,
        const webrtc::SdpVideoFormat& format) override;

    bool HasHardwareEncoderFor(
        const webrtc::SdpVideoFormat& format) const;
    bool HasSoftwareEncoderFor(
        const webrtc::SdpVideoFormat& format) const;

    // Existing PeerConnections keep this factory for their whole lifetime.
    // Replacing the child factories while no video is being published lets
    // the next track create an encoder from the newly selected policy without
    // rebuilding the room or its DataChannels.
    void ReplaceFactories(
        std::unique_ptr<webrtc::VideoEncoderFactory> softwareFactory,
        std::unique_ptr<webrtc::VideoEncoderFactory> hardwareFactory);

private:
    std::shared_ptr<WindowsPreferredVideoEncoderFactoryState> state_;
};

}  // namespace remote
