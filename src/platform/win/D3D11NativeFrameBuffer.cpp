// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#include "D3D11NativeFrameBuffer.h"

#include <algorithm>
#include <cstring>
#include <utility>

#include <Windows.h>
#include <d3d11.h>
#include <mfidl.h>
#include <wrl/client.h>

#include "api/video/i420_buffer.h"

namespace remote {

namespace {

constexpr char kD3D11StorageRepresentation[] = "D3D11 NV12 texture";

}  // namespace

using Microsoft::WRL::ComPtr;

class D3D11NativeFrameBuffer::Impl final {
public:
    Impl(IMFSample* sample,
         ID3D11Texture2D* texture,
         uint32_t subresourceIndex,
         int visibleWidth,
         int visibleHeight)
        : sample_(sample),
          texture_(texture),
          subresourceIndex_(subresourceIndex),
          visibleWidth_(visibleWidth),
          visibleHeight_(visibleHeight)
    {
        if (texture_) {
            texture_->GetDevice(&device_);
        }
    }

    Impl(std::shared_ptr<void> lifetimeOwner,
         ID3D11Texture2D* texture,
         uint32_t subresourceIndex,
         int visibleWidth,
         int visibleHeight)
        : lifetimeOwner_(std::move(lifetimeOwner)),
          texture_(texture),
          subresourceIndex_(subresourceIndex),
          visibleWidth_(visibleWidth),
          visibleHeight_(visibleHeight)
    {
        if (texture_) {
            texture_->GetDevice(&device_);
        }
    }

    webrtc::scoped_refptr<webrtc::I420BufferInterface> ToI420()
    {
        if (!device_ || !texture_ || visibleWidth_ <= 0 ||
            visibleHeight_ <= 0) {
            return nullptr;
        }

        D3D11_TEXTURE2D_DESC sourceDescription{};
        texture_->GetDesc(&sourceDescription);
        if (sourceDescription.Format != DXGI_FORMAT_NV12 ||
            sourceDescription.Width < static_cast<UINT>(visibleWidth_) ||
            sourceDescription.Height < static_cast<UINT>(visibleHeight_)) {
            return nullptr;
        }

        D3D11_TEXTURE2D_DESC stagingDescription{};
        stagingDescription.Width = sourceDescription.Width;
        stagingDescription.Height = sourceDescription.Height;
        stagingDescription.MipLevels = 1;
        stagingDescription.ArraySize = 1;
        stagingDescription.Format = DXGI_FORMAT_NV12;
        stagingDescription.SampleDesc.Count = 1;
        stagingDescription.Usage = D3D11_USAGE_STAGING;
        stagingDescription.CPUAccessFlags = D3D11_CPU_ACCESS_READ;

        ComPtr<ID3D11Texture2D> stagingTexture;
        if (FAILED(device_->CreateTexture2D(
                &stagingDescription, nullptr, &stagingTexture))) {
            return nullptr;
        }

        ComPtr<ID3D11DeviceContext> context;
        device_->GetImmediateContext(&context);
        if (!context) {
            return nullptr;
        }

        context->CopySubresourceRegion(
            stagingTexture.Get(),
            0,
            0,
            0,
            0,
            texture_.Get(),
            subresourceIndex_,
            nullptr);

        D3D11_MAPPED_SUBRESOURCE mapped{};
        if (FAILED(context->Map(
                stagingTexture.Get(), 0, D3D11_MAP_READ, 0, &mapped))) {
            return nullptr;
        }

        auto result = webrtc::I420Buffer::Create(
            visibleWidth_, visibleHeight_);
        const auto* sourceY = static_cast<const uint8_t*>(mapped.pData);
        const auto* sourceUV = sourceY +
            static_cast<size_t>(mapped.RowPitch) * sourceDescription.Height;

        for (int row = 0; row < visibleHeight_; ++row) {
            std::memcpy(
                result->MutableDataY() +
                    static_cast<size_t>(row) * result->StrideY(),
                sourceY + static_cast<size_t>(row) * mapped.RowPitch,
                static_cast<size_t>(visibleWidth_));
        }

        const int chromaWidth = (visibleWidth_ + 1) / 2;
        const int chromaHeight = (visibleHeight_ + 1) / 2;
        for (int row = 0; row < chromaHeight; ++row) {
            const uint8_t* sourceRow =
                sourceUV + static_cast<size_t>(row) * mapped.RowPitch;
            uint8_t* destinationU = result->MutableDataU() +
                static_cast<size_t>(row) * result->StrideU();
            uint8_t* destinationV = result->MutableDataV() +
                static_cast<size_t>(row) * result->StrideV();
            for (int column = 0; column < chromaWidth; ++column) {
                destinationU[column] = sourceRow[column * 2];
                destinationV[column] = sourceRow[column * 2 + 1];
            }
        }

        context->Unmap(stagingTexture.Get(), 0);
        return result;
    }

    ComPtr<IMFSample> sample_;
    std::shared_ptr<void> lifetimeOwner_;
    ComPtr<ID3D11Texture2D> texture_;
    ComPtr<ID3D11Device> device_;
    uint32_t subresourceIndex_ = 0;
    int visibleWidth_ = 0;
    int visibleHeight_ = 0;
};

D3D11NativeFrameBuffer::D3D11NativeFrameBuffer(
    IMFSample* sample,
    ID3D11Texture2D* texture,
    uint32_t subresourceIndex,
    int visibleWidth,
    int visibleHeight)
    : impl_(std::make_unique<Impl>(
          sample,
          texture,
          subresourceIndex,
          visibleWidth,
          visibleHeight))
{}

D3D11NativeFrameBuffer::D3D11NativeFrameBuffer(
    std::shared_ptr<void> lifetimeOwner,
    ID3D11Texture2D* texture,
    uint32_t subresourceIndex,
    int visibleWidth,
    int visibleHeight)
    : impl_(std::make_unique<Impl>(
          std::move(lifetimeOwner),
          texture,
          subresourceIndex,
          visibleWidth,
          visibleHeight))
{}

D3D11NativeFrameBuffer::~D3D11NativeFrameBuffer() = default;

webrtc::VideoFrameBuffer::Type D3D11NativeFrameBuffer::type() const
{
    return Type::kNative;
}

int D3D11NativeFrameBuffer::width() const
{
    return impl_->visibleWidth_;
}

int D3D11NativeFrameBuffer::height() const
{
    return impl_->visibleHeight_;
}

webrtc::scoped_refptr<webrtc::I420BufferInterface>
D3D11NativeFrameBuffer::ToI420()
{
    return impl_->ToI420();
}

std::string D3D11NativeFrameBuffer::storage_representation() const
{
    return kD3D11StorageRepresentation;
}

ID3D11Texture2D* D3D11NativeFrameBuffer::Texture() const noexcept
{
    return impl_->texture_.Get();
}

ID3D11Device* D3D11NativeFrameBuffer::Device() const noexcept
{
    return impl_->device_.Get();
}

uint32_t D3D11NativeFrameBuffer::SubresourceIndex() const noexcept
{
    return impl_->subresourceIndex_;
}

D3D11NativeFrameBuffer* D3D11NativeFrameBuffer::From(
    webrtc::VideoFrameBuffer* buffer)
{
    if (!buffer ||
        buffer->type() != webrtc::VideoFrameBuffer::Type::kNative ||
        buffer->storage_representation() !=
            kD3D11StorageRepresentation) {
        return nullptr;
    }
    return static_cast<D3D11NativeFrameBuffer*>(buffer);
}

const D3D11NativeFrameBuffer* D3D11NativeFrameBuffer::From(
    const webrtc::VideoFrameBuffer* buffer)
{
    return From(const_cast<webrtc::VideoFrameBuffer*>(buffer));
}

}  // namespace remote
