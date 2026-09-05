// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#pragma once

#include <memory>
#include <atomic>
#include <optional>
#include <vector>

#include "api/video/resolution.h"
#include "api/video_codecs/video_decoder_factory.h"

namespace remote {

class WindowsPreferredVideoDecoderFactoryState;

// Composes a Windows hardware decoder factory with WebRTC's built-in software
// decoder factory. Automatic mode prefers the proven low-latency software
// decoder; users can still explicitly select the hardware-only path.
class WindowsPreferredVideoDecoderFactory final
    : public webrtc::VideoDecoderFactory {
public:
    WindowsPreferredVideoDecoderFactory(
        std::unique_ptr<webrtc::VideoDecoderFactory> softwareFactory,
        std::unique_ptr<webrtc::VideoDecoderFactory> hardwareFactory,
        bool preferSoftware);
    ~WindowsPreferredVideoDecoderFactory() override;

    WindowsPreferredVideoDecoderFactory(
        const WindowsPreferredVideoDecoderFactory&) = delete;
    WindowsPreferredVideoDecoderFactory& operator=(
        const WindowsPreferredVideoDecoderFactory&) = delete;

    std::vector<webrtc::SdpVideoFormat> GetSupportedFormats() const override;

    CodecSupport QueryCodecSupport(
        const webrtc::SdpVideoFormat& format,
        bool referenceScaling,
        std::optional<webrtc::Resolution> resolution) const override;

    std::unique_ptr<webrtc::VideoDecoder> Create(
        const webrtc::Environment& environment,
        const webrtc::SdpVideoFormat& format) override;

    bool HasHardwareDecoderFor(
        const webrtc::SdpVideoFormat& format) const;
    bool HasSoftwareDecoderFor(
        const webrtc::SdpVideoFormat& format) const;

    void SetPreferSoftware(bool preferSoftware) noexcept;
    void NotifyConfigurationChanged() noexcept;

private:
    std::shared_ptr<WindowsPreferredVideoDecoderFactoryState> state_;
};

}  // namespace remote
