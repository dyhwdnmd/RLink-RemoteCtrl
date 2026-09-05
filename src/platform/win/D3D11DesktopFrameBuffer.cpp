// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#include "D3D11DesktopFrameBuffer.h"

#include <algorithm>
#include <cstring>
#include <mutex>
#include <vector>

#include <Windows.h>
#include <d3d11.h>
#include <wrl/client.h>

#include "api/make_ref_counted.h"
#include "api/video/i420_buffer.h"
#include "libyuv/convert.h"
#include "libyuv/scale_argb.h"

namespace remote {
namespace {

using Microsoft::WRL::ComPtr;

constexpr char kStorageRepresentation[] = "D3D11_BGRA_DESKTOP";

std::mutex gCaptureDeviceMutex;
ComPtr<ID3D11Device> gCaptureDevice;

}  // namespace

class D3D11DesktopFrameBuffer::Impl final {
public:
    Impl(ID3D11Texture2D* texture,
         int sourceWidth,
         int sourceHeight,
         int cropX,
         int cropY,
         int cropWidth,
         int cropHeight,
         int outputWidth,
         int outputHeight)
        : texture(texture),
          sourceWidth(sourceWidth),
          sourceHeight(sourceHeight),
          cropX(cropX),
          cropY(cropY),
          cropWidth(cropWidth),
          cropHeight(cropHeight),
          outputWidth(outputWidth),
          outputHeight(outputHeight)
    {
        if (this->texture) {
            this->texture->GetDevice(&device);
        }
    }

    ComPtr<ID3D11Texture2D> texture;
    ComPtr<ID3D11Device> device;
    int sourceWidth = 0;
    int sourceHeight = 0;
    int cropX = 0;
    int cropY = 0;
    int cropWidth = 0;
    int cropHeight = 0;
    int outputWidth = 0;
    int outputHeight = 0;
};

D3D11DesktopFrameBuffer::D3D11DesktopFrameBuffer(
    ID3D11Texture2D* texture,
    int sourceWidth,
    int sourceHeight)
    : D3D11DesktopFrameBuffer(
          texture, sourceWidth, sourceHeight,
          0, 0, sourceWidth, sourceHeight,
          sourceWidth, sourceHeight)
{}

D3D11DesktopFrameBuffer::D3D11DesktopFrameBuffer(
    ID3D11Texture2D* texture,
    int sourceWidth,
    int sourceHeight,
    int cropX,
    int cropY,
    int cropWidth,
    int cropHeight,
    int outputWidth,
    int outputHeight)
    : impl_(std::make_unique<Impl>(
          texture, sourceWidth, sourceHeight,
          cropX, cropY, cropWidth, cropHeight,
          outputWidth, outputHeight))
{}

D3D11DesktopFrameBuffer::~D3D11DesktopFrameBuffer() = default;

webrtc::VideoFrameBuffer::Type D3D11DesktopFrameBuffer::type() const
{
    return Type::kNative;
}

int D3D11DesktopFrameBuffer::width() const
{
    return impl_->outputWidth;
}

int D3D11DesktopFrameBuffer::height() const
{
    return impl_->outputHeight;
}

webrtc::scoped_refptr<webrtc::I420BufferInterface>
D3D11DesktopFrameBuffer::ToI420()
{
    if (!impl_->device || !impl_->texture ||
        impl_->sourceWidth <= 0 || impl_->sourceHeight <= 0 ||
        impl_->outputWidth <= 0 || impl_->outputHeight <= 0) {
        return nullptr;
    }

    D3D11_TEXTURE2D_DESC source{};
    impl_->texture->GetDesc(&source);
    if (source.Format != DXGI_FORMAT_B8G8R8A8_UNORM &&
        source.Format != DXGI_FORMAT_B8G8R8A8_UNORM_SRGB) {
        return nullptr;
    }

    D3D11_TEXTURE2D_DESC staging = source;
    staging.Usage = D3D11_USAGE_STAGING;
    staging.BindFlags = 0;
    staging.MiscFlags = 0;
    staging.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
    ComPtr<ID3D11Texture2D> stagingTexture;
    if (FAILED(impl_->device->CreateTexture2D(
            &staging, nullptr, &stagingTexture))) {
        return nullptr;
    }
    ComPtr<ID3D11DeviceContext> context;
    impl_->device->GetImmediateContext(&context);
    if (!context) {
        return nullptr;
    }
    context->CopyResource(stagingTexture.Get(), impl_->texture.Get());
    D3D11_MAPPED_SUBRESOURCE mapped{};
    if (FAILED(context->Map(
            stagingTexture.Get(), 0, D3D11_MAP_READ, 0, &mapped))) {
        return nullptr;
    }

    const auto* cropped =
        static_cast<const std::uint8_t*>(mapped.pData) +
        static_cast<std::size_t>(impl_->cropY) * mapped.RowPitch +
        static_cast<std::size_t>(impl_->cropX) * 4;
    const std::uint8_t* bgra = cropped;
    int bgraStride = static_cast<int>(mapped.RowPitch);
    std::vector<std::uint8_t> scaled;
    if (impl_->cropWidth != impl_->outputWidth ||
        impl_->cropHeight != impl_->outputHeight) {
        scaled.resize(
            static_cast<std::size_t>(impl_->outputWidth) *
            impl_->outputHeight * 4);
        bgraStride = impl_->outputWidth * 4;
        if (libyuv::ARGBScale(
                cropped, static_cast<int>(mapped.RowPitch),
                impl_->cropWidth, impl_->cropHeight,
                scaled.data(), bgraStride,
                impl_->outputWidth, impl_->outputHeight,
                libyuv::kFilterBilinear) != 0) {
            context->Unmap(stagingTexture.Get(), 0);
            return nullptr;
        }
        bgra = scaled.data();
    }

    auto result = webrtc::I420Buffer::Create(
        impl_->outputWidth, impl_->outputHeight);
    const int conversion = libyuv::ARGBToI420(
        bgra, bgraStride,
        result->MutableDataY(), result->StrideY(),
        result->MutableDataU(), result->StrideU(),
        result->MutableDataV(), result->StrideV(),
        impl_->outputWidth, impl_->outputHeight);
    context->Unmap(stagingTexture.Get(), 0);
    return conversion == 0 ? result : nullptr;
}

webrtc::scoped_refptr<webrtc::VideoFrameBuffer>
D3D11DesktopFrameBuffer::CropAndScale(
    int offsetX,
    int offsetY,
    int cropWidth,
    int cropHeight,
    int scaledWidth,
    int scaledHeight)
{
    if (offsetX < 0 || offsetY < 0 || cropWidth <= 0 ||
        cropHeight <= 0 || scaledWidth <= 0 || scaledHeight <= 0 ||
        offsetX + cropWidth > width() ||
        offsetY + cropHeight > height()) {
        return nullptr;
    }
    const int mappedX = impl_->cropX +
        static_cast<int>(
            static_cast<std::int64_t>(offsetX) *
            impl_->cropWidth / width());
    const int mappedY = impl_->cropY +
        static_cast<int>(
            static_cast<std::int64_t>(offsetY) *
            impl_->cropHeight / height());
    const int mappedWidth = (std::max)(
        1, static_cast<int>(
               static_cast<std::int64_t>(cropWidth) *
               impl_->cropWidth / width()));
    const int mappedHeight = (std::max)(
        1, static_cast<int>(
               static_cast<std::int64_t>(cropHeight) *
               impl_->cropHeight / height()));
    return webrtc::make_ref_counted<D3D11DesktopFrameBuffer>(
        impl_->texture.Get(),
        impl_->sourceWidth,
        impl_->sourceHeight,
        mappedX,
        mappedY,
        (std::min)(mappedWidth, impl_->sourceWidth - mappedX),
        (std::min)(mappedHeight, impl_->sourceHeight - mappedY),
        scaledWidth,
        scaledHeight);
}

std::string D3D11DesktopFrameBuffer::storage_representation() const
{
    return kStorageRepresentation;
}

ID3D11Texture2D* D3D11DesktopFrameBuffer::Texture() const noexcept
{
    return impl_->texture.Get();
}

ID3D11Device* D3D11DesktopFrameBuffer::Device() const noexcept
{
    return impl_->device.Get();
}

int D3D11DesktopFrameBuffer::SourceWidth() const noexcept
{
    return impl_->sourceWidth;
}

int D3D11DesktopFrameBuffer::SourceHeight() const noexcept
{
    return impl_->sourceHeight;
}

int D3D11DesktopFrameBuffer::CropX() const noexcept
{
    return impl_->cropX;
}

int D3D11DesktopFrameBuffer::CropY() const noexcept
{
    return impl_->cropY;
}

int D3D11DesktopFrameBuffer::CropWidth() const noexcept
{
    return impl_->cropWidth;
}

int D3D11DesktopFrameBuffer::CropHeight() const noexcept
{
    return impl_->cropHeight;
}

D3D11DesktopFrameBuffer* D3D11DesktopFrameBuffer::From(
    webrtc::VideoFrameBuffer* buffer)
{
    if (!buffer ||
        buffer->type() != webrtc::VideoFrameBuffer::Type::kNative ||
        buffer->storage_representation() != kStorageRepresentation) {
        return nullptr;
    }
    return static_cast<D3D11DesktopFrameBuffer*>(buffer);
}

const D3D11DesktopFrameBuffer* D3D11DesktopFrameBuffer::From(
    const webrtc::VideoFrameBuffer* buffer)
{
    return From(const_cast<webrtc::VideoFrameBuffer*>(buffer));
}

void PublishDesktopCaptureD3D11Device(ID3D11Device* device)
{
    std::lock_guard lock(gCaptureDeviceMutex);
    gCaptureDevice = device;
}

ID3D11Device* AcquireDesktopCaptureD3D11Device()
{
    std::lock_guard lock(gCaptureDeviceMutex);
    ID3D11Device* result = gCaptureDevice.Get();
    if (result) {
        result->AddRef();
    }
    return result;
}

}  // namespace remote
