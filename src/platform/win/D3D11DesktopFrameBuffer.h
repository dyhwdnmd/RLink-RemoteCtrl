// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#pragma once

#include <cstdint>
#include <memory>

#include "api/scoped_refptr.h"
#include "api/video/video_frame_buffer.h"

struct ID3D11Device;
struct ID3D11Texture2D;

namespace remote {

// Immutable BGRA desktop texture captured through DXGI Desktop Duplication.
// CropAndScale keeps a lazy description so each sender can use the same GPU
// texture while applying its own crop and output size in the D3D11 video
// processor. ToI420 is intentionally retained as the software fallback.
class D3D11DesktopFrameBuffer : public webrtc::VideoFrameBuffer {
public:
    D3D11DesktopFrameBuffer(
        ID3D11Texture2D* texture,
        int sourceWidth,
        int sourceHeight);
    D3D11DesktopFrameBuffer(
        ID3D11Texture2D* texture,
        int sourceWidth,
        int sourceHeight,
        int cropX,
        int cropY,
        int cropWidth,
        int cropHeight,
        int outputWidth,
        int outputHeight);
    ~D3D11DesktopFrameBuffer() override;

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

    ID3D11Texture2D* Texture() const noexcept;
    ID3D11Device* Device() const noexcept;
    int SourceWidth() const noexcept;
    int SourceHeight() const noexcept;
    int CropX() const noexcept;
    int CropY() const noexcept;
    int CropWidth() const noexcept;
    int CropHeight() const noexcept;

    static D3D11DesktopFrameBuffer* From(
        webrtc::VideoFrameBuffer* buffer);
    static const D3D11DesktopFrameBuffer* From(
        const webrtc::VideoFrameBuffer* buffer);

private:
    class Impl;
    std::unique_ptr<Impl> impl_;
};

// The desktop capturer publishes its D3D11 device before the WebRTC encoder is
// created. The MFT encoder acquires a reference to the same device, eliminating
// cross-device texture sharing and preserving GPU command ordering. The
// returned pointer owns one COM reference and must be released by the caller.
void PublishDesktopCaptureD3D11Device(ID3D11Device* device);
ID3D11Device* AcquireDesktopCaptureD3D11Device();

}  // namespace remote
