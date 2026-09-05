// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#include "DesktopBgraFrameBuffer.h"

#include <algorithm>
#include <cstring>

#include "api/make_ref_counted.h"
#include "api/video/i420_buffer.h"
#include "libyuv/convert.h"
#include "libyuv/scale_argb.h"

namespace remote {

DesktopBgraFrameBuffer::DesktopBgraFrameBuffer(const uint8_t* data,
                                               int stride,
                                               int width,
                                               int height)
    : width_(width),
      height_(height),
      sourceWidth_(width),
      sourceHeight_(height),
      sourceStride_(width * 4),
      cropWidth_(width),
      cropHeight_(height),
      pixels_(std::make_shared<std::vector<uint8_t>>(
          static_cast<size_t>(sourceStride_) * height_))
{
    for (int row = 0; row < height_; ++row) {
        std::memcpy(pixels_->data() +
                        static_cast<size_t>(row) * sourceStride_,
                    data + static_cast<size_t>(row) * stride,
                    static_cast<size_t>(sourceStride_));
    }
}

DesktopBgraFrameBuffer::DesktopBgraFrameBuffer(
    std::shared_ptr<std::vector<uint8_t>> pixels,
    int sourceStride,
    int sourceWidth,
    int sourceHeight,
    int cropX,
    int cropY,
    int cropWidth,
    int cropHeight,
    int outputWidth,
    int outputHeight)
    : width_(outputWidth),
      height_(outputHeight),
      sourceWidth_(sourceWidth),
      sourceHeight_(sourceHeight),
      sourceStride_(sourceStride),
      cropX_(cropX),
      cropY_(cropY),
      cropWidth_(cropWidth),
      cropHeight_(cropHeight),
      pixels_(std::move(pixels))
{}

webrtc::VideoFrameBuffer::Type DesktopBgraFrameBuffer::type() const
{
    return Type::kNative;
}

int DesktopBgraFrameBuffer::width() const
{
    return width_;
}

int DesktopBgraFrameBuffer::height() const
{
    return height_;
}

webrtc::scoped_refptr<webrtc::I420BufferInterface>
DesktopBgraFrameBuffer::ToI420()
{
    auto i420 = webrtc::I420Buffer::Create(width_, height_);
    const uint8_t* cropped = pixels_->data() +
        static_cast<size_t>(cropY_) * sourceStride_ + cropX_ * 4;
    if (cropWidth_ != width_ || cropHeight_ != height_) {
        std::vector<uint8_t> scaled(
            static_cast<size_t>(width_) * height_ * 4);
        const int scaledStride = width_ * 4;
        if (libyuv::ARGBScale(
                cropped, sourceStride_, cropWidth_, cropHeight_,
                scaled.data(), scaledStride, width_, height_,
                libyuv::kFilterBilinear) != 0 ||
            libyuv::ARGBToI420(
                scaled.data(), scaledStride,
                i420->MutableDataY(), i420->StrideY(),
                i420->MutableDataU(), i420->StrideU(),
                i420->MutableDataV(), i420->StrideV(),
                width_, height_) != 0) {
            return nullptr;
        }
        return i420;
    }
    if (libyuv::ARGBToI420(
            cropped, sourceStride_,
            i420->MutableDataY(), i420->StrideY(),
            i420->MutableDataU(), i420->StrideU(),
            i420->MutableDataV(), i420->StrideV(),
            width_, height_) != 0) {
        return nullptr;
    }
    return i420;
}

webrtc::scoped_refptr<webrtc::VideoFrameBuffer>
DesktopBgraFrameBuffer::CropAndScale(
    int offsetX,
    int offsetY,
    int cropWidth,
    int cropHeight,
    int scaledWidth,
    int scaledHeight)
{
    if (offsetX < 0 || offsetY < 0 || cropWidth <= 0 || cropHeight <= 0 ||
        scaledWidth <= 0 || scaledHeight <= 0 ||
        offsetX + cropWidth > width_ ||
        offsetY + cropHeight > height_) {
        return nullptr;
    }

    // Map a crop expressed in the current logical frame back into the one
    // captured BGRA allocation. This keeps all sender-specific resizing lazy:
    // D3D11 hardware encoding consumes the original pixels and software
    // encoding maps the same description to a scaled I420 buffer.
    const int mappedX = cropX_ +
        static_cast<int>(static_cast<std::int64_t>(offsetX) * cropWidth_ /
                         width_);
    const int mappedY = cropY_ +
        static_cast<int>(static_cast<std::int64_t>(offsetY) * cropHeight_ /
                         height_);
    const int mappedWidth = (std::max)(1,
        static_cast<int>(static_cast<std::int64_t>(cropWidth) * cropWidth_ /
                         width_));
    const int mappedHeight = (std::max)(1,
        static_cast<int>(static_cast<std::int64_t>(cropHeight) * cropHeight_ /
                         height_));
    return webrtc::make_ref_counted<DesktopBgraFrameBuffer>(
        pixels_, sourceStride_, sourceWidth_, sourceHeight_,
        mappedX, mappedY,
        (std::min)(mappedWidth, sourceWidth_ - mappedX),
        (std::min)(mappedHeight, sourceHeight_ - mappedY),
        scaledWidth, scaledHeight);
}

std::string DesktopBgraFrameBuffer::storage_representation() const
{
    return "CPU_BGRA_DESKTOP";
}

const uint8_t* DesktopBgraFrameBuffer::Data() const noexcept
{
    return pixels_->data();
}

int DesktopBgraFrameBuffer::Stride() const noexcept
{
    return sourceStride_;
}

int DesktopBgraFrameBuffer::SourceWidth() const noexcept { return sourceWidth_; }
int DesktopBgraFrameBuffer::SourceHeight() const noexcept { return sourceHeight_; }
int DesktopBgraFrameBuffer::CropX() const noexcept { return cropX_; }
int DesktopBgraFrameBuffer::CropY() const noexcept { return cropY_; }
int DesktopBgraFrameBuffer::CropWidth() const noexcept { return cropWidth_; }
int DesktopBgraFrameBuffer::CropHeight() const noexcept { return cropHeight_; }

}  // namespace remote
