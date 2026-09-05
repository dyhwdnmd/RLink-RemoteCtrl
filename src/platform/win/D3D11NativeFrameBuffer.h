// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#pragma once

#include <cstdint>
#include <memory>

#include "api/scoped_refptr.h"
#include "api/video/video_frame_buffer.h"

struct ID3D11Device;
struct ID3D11Texture2D;
struct IMFSample;

namespace remote {

// Owns a decoder-specific lifetime token and its D3D11 NV12 texture. Keeping
// the token alive prevents either Media Foundation or FFmpeg from recycling
// the decode surface before the WebRTC frame is released by the renderer.
class D3D11NativeFrameBuffer : public webrtc::VideoFrameBuffer {
public:
    D3D11NativeFrameBuffer(IMFSample* sample,
                           ID3D11Texture2D* texture,
                           uint32_t subresourceIndex,
                           int visibleWidth,
                           int visibleHeight);
    D3D11NativeFrameBuffer(std::shared_ptr<void> lifetimeOwner,
                           ID3D11Texture2D* texture,
                           uint32_t subresourceIndex,
                           int visibleWidth,
                           int visibleHeight);
    ~D3D11NativeFrameBuffer() override;

    Type type() const override;
    int width() const override;
    int height() const override;
    webrtc::scoped_refptr<webrtc::I420BufferInterface> ToI420() override;
    std::string storage_representation() const override;

    ID3D11Texture2D* Texture() const noexcept;
    ID3D11Device* Device() const noexcept;
    uint32_t SubresourceIndex() const noexcept;

    // libwebrtc is built without C++ RTTI, so dynamic_cast must never be
    // used on VideoFrameBuffer instances produced by its software decoder.
    // Identify our native buffer through WebRTC's virtual type/representation
    // contract before using a static cast.
    static D3D11NativeFrameBuffer* From(
        webrtc::VideoFrameBuffer* buffer);
    static const D3D11NativeFrameBuffer* From(
        const webrtc::VideoFrameBuffer* buffer);

private:
    class Impl;
    std::unique_ptr<Impl> impl_;
};

}  // namespace remote
