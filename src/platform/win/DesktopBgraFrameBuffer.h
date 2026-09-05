// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#pragma once

#include <cstdint>
#include <memory>
#include <vector>

#include "api/video/video_frame_buffer.h"

namespace remote {

// A desktop frame that keeps the capturer's native BGRA pixels until the
// selected encoder is known. The Media Foundation encoder uploads these pixels
// directly to D3D11; software encoders can still request a lazy I420 mapping.
class DesktopBgraFrameBuffer : public webrtc::VideoFrameBuffer {
public:
    DesktopBgraFrameBuffer(const uint8_t* data,
                           int stride,
                           int width,
                           int height);
    DesktopBgraFrameBuffer(std::shared_ptr<std::vector<uint8_t>> pixels,
                           int sourceStride,
                           int sourceWidth,
                           int sourceHeight,
                           int cropX,
                           int cropY,
                           int cropWidth,
                           int cropHeight,
                           int outputWidth,
                           int outputHeight);

    Type type() const override;
    int width() const override;
    int height() const override;
    webrtc::scoped_refptr<webrtc::I420BufferInterface> ToI420() override;
    webrtc::scoped_refptr<webrtc::VideoFrameBuffer> CropAndScale(
        int offsetX,
        int offsetY,
        int cropWidth,
        int cropHeight,
        int scaledWidth,
        int scaledHeight) override;
    std::string storage_representation() const override;

    const uint8_t* Data() const noexcept;
    int Stride() const noexcept;
    int SourceWidth() const noexcept;
    int SourceHeight() const noexcept;
    int CropX() const noexcept;
    int CropY() const noexcept;
    int CropWidth() const noexcept;
    int CropHeight() const noexcept;

private:
    int width_ = 0;
    int height_ = 0;
    int sourceWidth_ = 0;
    int sourceHeight_ = 0;
    int sourceStride_ = 0;
    int cropX_ = 0;
    int cropY_ = 0;
    int cropWidth_ = 0;
    int cropHeight_ = 0;
    std::shared_ptr<std::vector<uint8_t>> pixels_;
};

}  // namespace remote
