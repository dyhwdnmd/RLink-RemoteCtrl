// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#include "MfD3D11H264DecoderSelfTest.h"

#include <cstddef>
#include <cstdint>
#include <chrono>
#include <condition_variable>
#include <filesystem>
#include <fstream>
#include <mutex>
#include <sstream>
#include <utility>
#include <vector>

#include "api/video/encoded_image.h"
#include "api/video/video_frame.h"
#include "api/video/video_frame_type.h"
#include "modules/video_coding/include/video_error_codes.h"
#include "D3D11NativeFrameBuffer.h"
#include "MfD3D11H264Decoder.h"

namespace remote {
namespace {

class DecodeCallback final : public webrtc::DecodedImageCallback {
public:
    int32_t Decoded(webrtc::VideoFrame& frame) override
    {
        std::lock_guard lock(mutex_);
        ++frameCount;
        lastWidth = frame.width();
        lastHeight = frame.height();

        auto frameBuffer = frame.video_frame_buffer();
        nativeFrameSeen =
            frameBuffer->type() == webrtc::VideoFrameBuffer::Type::kNative &&
            D3D11NativeFrameBuffer::From(frameBuffer.get()) != nullptr;
        auto i420 = frameBuffer->ToI420();
        i420FallbackSucceeded = i420 &&
            i420->width() == frame.width() &&
            i420->height() == frame.height();
        condition_.notify_all();
        return WEBRTC_VIDEO_CODEC_OK;
    }

    bool WaitForFrame(std::chrono::milliseconds timeout)
    {
        std::unique_lock lock(mutex_);
        return condition_.wait_for(
            lock, timeout, [this] { return frameCount > 0; });
    }

    int frameCount = 0;
    int lastWidth = 0;
    int lastHeight = 0;
    bool nativeFrameSeen = false;
    bool i420FallbackSucceeded = false;

private:
    std::mutex mutex_;
    std::condition_variable condition_;
};

struct NaluPosition {
    size_t start = 0;
    size_t payload = 0;
    uint8_t type = 0;
};

std::vector<NaluPosition> FindNalus(const std::vector<uint8_t>& bytes)
{
    std::vector<NaluPosition> nalus;
    for (size_t index = 0; index + 3 < bytes.size();) {
        size_t prefixLength = 0;
        if (bytes[index] == 0 && bytes[index + 1] == 0 &&
            bytes[index + 2] == 1) {
            prefixLength = 3;
        } else if (index + 4 < bytes.size() && bytes[index] == 0 &&
                   bytes[index + 1] == 0 && bytes[index + 2] == 0 &&
                   bytes[index + 3] == 1) {
            prefixLength = 4;
        }

        if (prefixLength == 0) {
            ++index;
            continue;
        }
        const size_t payload = index + prefixLength;
        if (payload < bytes.size()) {
            nalus.push_back({index, payload,
                             static_cast<uint8_t>(bytes[payload] & 0x1F)});
        }
        index = payload + 1;
    }
    return nalus;
}

std::vector<std::vector<uint8_t>> SplitAccessUnits(
    const std::vector<uint8_t>& bytes)
{
    const auto nalus = FindNalus(bytes);
    std::vector<size_t> accessUnitStarts;
    for (const auto& nalu : nalus) {
        if (nalu.type == 9) {
            accessUnitStarts.push_back(nalu.start);
        }
    }

    if (accessUnitStarts.empty()) {
        return {bytes};
    }
    accessUnitStarts.front() = 0;

    std::vector<std::vector<uint8_t>> accessUnits;
    for (size_t index = 0; index < accessUnitStarts.size(); ++index) {
        const size_t begin = accessUnitStarts[index];
        const size_t end = index + 1 < accessUnitStarts.size()
            ? accessUnitStarts[index + 1]
            : bytes.size();
        if (end > begin) {
            accessUnits.emplace_back(bytes.begin() + begin, bytes.begin() + end);
        }
    }
    return accessUnits;
}

}  // namespace

MfD3D11H264DecoderSelfTestResult RunMfD3D11H264DecoderSelfTest(
    const std::wstring& annexBFilePath)
{
    MfD3D11H264DecoderSelfTestResult result;
    std::ostringstream report;

    std::ifstream input(
        std::filesystem::path(annexBFilePath),
        std::ios::binary | std::ios::ate);
    if (!input) {
        result.report = "Failed to open H264 Annex-B test file.";
        return result;
    }
    const auto fileSize = input.tellg();
    if (fileSize <= 0) {
        result.report = "H264 Annex-B test file is empty.";
        return result;
    }
    input.seekg(0, std::ios::beg);
    std::vector<uint8_t> bytes(static_cast<size_t>(fileSize));
    if (!input.read(reinterpret_cast<char*>(bytes.data()), fileSize)) {
        result.report = "Failed to read H264 Annex-B test file.";
        return result;
    }

    const auto accessUnits = SplitAccessUnits(bytes);
    report << "H264 access units: " << accessUnits.size() << '\n';

    MfD3D11H264Decoder decoder;
    webrtc::VideoDecoder::Settings settings;
    settings.set_codec_type(webrtc::kVideoCodecH264);
    settings.set_number_of_cores(1);
    settings.set_max_render_resolution(webrtc::RenderResolution(128, 96));
    if (!decoder.Configure(settings)) {
        report << "Configure: NO\nError: " << decoder.LastError();
        result.report = report.str();
        return result;
    }

    DecodeCallback callback;
    decoder.RegisterDecodeCompleteCallback(&callback);
    int fallbackCount = 0;
    int errorCount = 0;
    uint32_t timestamp = 90000;
    for (const auto& accessUnit : accessUnits) {
        webrtc::EncodedImage image;
        image.SetEncodedData(webrtc::EncodedImageBuffer::Create(
            accessUnit.data(), accessUnit.size()));
        image.SetRtpTimestamp(timestamp);
        image.SetFrameType(webrtc::VideoFrameType::kVideoFrameKey);
        image._encodedWidth = 128;
        image._encodedHeight = 96;

        const int32_t decodeResult = decoder.Decode(image, 0);
        if (decodeResult == WEBRTC_VIDEO_CODEC_FALLBACK_SOFTWARE) {
            ++fallbackCount;
        } else if (decodeResult < 0) {
            ++errorCount;
        }
        timestamp += 45000;
    }

    const bool frameArrived =
        callback.WaitForFrame(std::chrono::seconds(3));
    const bool nativeOutput =
        decoder.SupportsNativeOutputSamples();
    const bool hardwareDecoder =
        decoder.IsHardwareAccelerated();
    const bool asynchronousDecoder =
        decoder.IsAsynchronous();
    decoder.Release();

    report << "Configure: YES\n"
           << "Hardware decoder: "
           << (hardwareDecoder ? "YES" : "NO") << '\n'
           << "Asynchronous MFT: "
           << (asynchronousDecoder ? "YES" : "NO") << '\n'
           << "Native DXGI output samples: "
           << (nativeOutput ? "YES" : "NO") << '\n'
           << "Decoded callback frames: " << callback.frameCount << '\n'
           << "Native D3D11 frame seen: "
           << (callback.nativeFrameSeen ? "YES" : "NO") << '\n'
           << "ToI420 fallback: "
           << (callback.i420FallbackSucceeded ? "YES" : "NO") << '\n'
           << "Decoded size: " << callback.lastWidth << 'x'
           << callback.lastHeight << '\n'
           << "Software fallback returns: " << fallbackCount << '\n'
           << "Decode errors: " << errorCount;
    if (!decoder.LastError().empty()) {
        report << "\nLast error: " << decoder.LastError();
    }

    result.passed = frameArrived && hardwareDecoder &&
        callback.frameCount > 0 && callback.nativeFrameSeen &&
        callback.i420FallbackSucceeded && fallbackCount == 0 &&
        errorCount == 0;
    result.report = report.str();
    return result;
}

}  // namespace remote
