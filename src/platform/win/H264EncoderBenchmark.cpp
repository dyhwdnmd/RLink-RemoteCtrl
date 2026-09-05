// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#include "H264EncoderBenchmark.h"

#include <Windows.h>
#include <d3d11.h>
#include <wrl/client.h>

#include <algorithm>
#include <chrono>
#include <cmath>
#include <condition_variable>
#include <cstdint>
#include <cstring>
#include <map>
#include <memory>
#include <mutex>
#include <numeric>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

#include "api/environment/environment_factory.h"
#include "api/make_ref_counted.h"
#include "api/video/encoded_image.h"
#include "api/video/i420_buffer.h"
#include "api/video/video_frame.h"
#include "api/video_codecs/builtin_video_decoder_factory.h"
#include "api/video_codecs/builtin_video_encoder_factory.h"
#include "api/video_codecs/sdp_video_format.h"
#include "modules/video_coding/include/video_error_codes.h"
#include "libyuv/convert.h"
#include "D3D11DesktopFrameBuffer.h"
#include "DesktopBgraFrameBuffer.h"
#include "FfmpegHardwareH264Encoder.h"
#include "FfmpegX264H264Encoder.h"
#include "MfD3D11H264Encoder.h"
#include "QualityOpenH264Encoder.h"

namespace remote {
namespace {

using Clock = std::chrono::steady_clock;
using Microsoft::WRL::ComPtr;

constexpr int kWidth = 1920;
constexpr int kHeight = 1080;
constexpr std::uint32_t kFrameRate = 60;
constexpr std::uint32_t kFrameCount = 108;
constexpr std::uint32_t kWarmupFrames = 12;
// Match ScreenStreamPolicy's 0.15 bits-per-pixel-rate ceiling for
// 1920x1080@60 instead of starving the quality comparison at 4 Mbps.
constexpr std::uint32_t kBitrateBps = 18'662'400;

void AppendWarning(std::string& destination, std::string warning)
{
    if (warning.empty()) {
        return;
    }
    if (!destination.empty()) {
        destination += " ";
    }
    destination += std::move(warning);
}

bool HasAnnexBStartCode(const std::uint8_t* data, std::size_t size)
{
    if (!data || size < 4) {
        return false;
    }
    for (std::size_t index = 0; index + 3 < size; ++index) {
        if (data[index] == 0 && data[index + 1] == 0 &&
            (data[index + 2] == 1 ||
             (data[index + 2] == 0 && data[index + 3] == 1))) {
            return true;
        }
    }
    return false;
}

double ProcessCpuMilliseconds()
{
    FILETIME creation{}, exit{}, kernel{}, user{};
    if (!GetProcessTimes(
            GetCurrentProcess(), &creation, &exit, &kernel, &user)) {
        return 0.0;
    }
    ULARGE_INTEGER kernelValue{};
    kernelValue.LowPart = kernel.dwLowDateTime;
    kernelValue.HighPart = kernel.dwHighDateTime;
    ULARGE_INTEGER userValue{};
    userValue.LowPart = user.dwLowDateTime;
    userValue.HighPart = user.dwHighDateTime;
    return static_cast<double>(kernelValue.QuadPart + userValue.QuadPart) /
           10'000.0;
}

class HighResolutionFramePacer final {
public:
    HighResolutionFramePacer()
    {
        timer_ = CreateWaitableTimerExW(
            nullptr, nullptr, CREATE_WAITABLE_TIMER_HIGH_RESOLUTION,
            TIMER_MODIFY_STATE | SYNCHRONIZE);
        if (!timer_) {
            // Retain compatibility with older Windows 10 builds while still
            // using a kernel waitable timer rather than thread sleep.
            timer_ = CreateWaitableTimerExW(
                nullptr, nullptr, 0, TIMER_MODIFY_STATE | SYNCHRONIZE);
        }
    }

    ~HighResolutionFramePacer()
    {
        if (timer_) {
            CloseHandle(timer_);
        }
    }

    bool IsValid() const noexcept { return timer_ != nullptr; }

    bool WaitUntil(Clock::time_point deadline)
    {
        const auto remaining = deadline - Clock::now();
        if (remaining <= Clock::duration::zero()) {
            return true;
        }
        const auto remainingNanoseconds =
            std::chrono::duration_cast<std::chrono::nanoseconds>(
                remaining).count();
        LARGE_INTEGER dueTime{};
        dueTime.QuadPart = -(std::max<std::int64_t>)(
            1, (remainingNanoseconds + 99) / 100);
        if (!SetWaitableTimerEx(
                timer_, &dueTime, 0, nullptr, nullptr, nullptr, 0)) {
            return false;
        }
        return WaitForSingleObject(timer_, INFINITE) == WAIT_OBJECT_0;
    }

private:
    HANDLE timer_ = nullptr;
};

const std::vector<std::uint8_t>& SyntheticDesktopBackground()
{
    static const std::vector<std::uint8_t> background = [] {
        std::vector<std::uint8_t> result(
            static_cast<std::size_t>(kWidth) * kHeight * 4);
        for (int y = 0; y < kHeight; ++y) {
            for (int x = 0; x < kWidth; ++x) {
                const std::size_t offset =
                    (static_cast<std::size_t>(y) * kWidth + x) * 4;
                const bool grid = (x % 64) < 2 || (y % 48) < 2;
                result[offset] = grid ? 92 :
                    static_cast<std::uint8_t>(32 + (x / 8) % 96);
                result[offset + 1] = grid ? 118 :
                    static_cast<std::uint8_t>(45 + (y / 6) % 80);
                result[offset + 2] = grid ? 146 : 54;
                result[offset + 3] = 0xff;
            }
        }
        return result;
    }();
    return background;
}

void FillMovingDesktop(std::vector<std::uint8_t>& pixels,
                       std::uint32_t frameIndex)
{
    pixels = SyntheticDesktopBackground();
    const int movingX = static_cast<int>((frameIndex * 23) % (kWidth - 220));
    const int movingY = static_cast<int>((frameIndex * 11) % (kHeight - 140));
    for (int y = movingY; y < movingY + 140; ++y) {
        for (int x = movingX; x < movingX + 220; ++x) {
            const std::size_t offset =
                (static_cast<std::size_t>(y) * kWidth + x) * 4;
            pixels[offset] = 36;
            pixels[offset + 1] = static_cast<std::uint8_t>(
                110 + (x + frameIndex) % 120);
            pixels[offset + 2] = 238;
            pixels[offset + 3] = 0xff;
        }
    }
}

class BenchmarkFrameSource final {
public:
    ~BenchmarkFrameSource()
    {
        if (native_) {
            PublishDesktopCaptureD3D11Device(nullptr);
        }
    }

    bool Initialize(H264EncoderBenchmarkInput input, std::string& error)
    {
        native_ = input == H264EncoderBenchmarkInput::kD3D11Bgra;
        if (!native_) {
            pixels_ = std::make_shared<std::vector<std::uint8_t>>();
            FillMovingDesktop(*pixels_, 0);
            return true;
        }
        constexpr D3D_FEATURE_LEVEL levels[] = {
            D3D_FEATURE_LEVEL_11_1,
            D3D_FEATURE_LEVEL_11_0,
            D3D_FEATURE_LEVEL_10_1,
            D3D_FEATURE_LEVEL_10_0,
        };
        D3D_FEATURE_LEVEL selected{};
        HRESULT result = D3D11CreateDevice(
            nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr,
            D3D11_CREATE_DEVICE_BGRA_SUPPORT |
                D3D11_CREATE_DEVICE_VIDEO_SUPPORT,
            levels, static_cast<UINT>(std::size(levels)),
            D3D11_SDK_VERSION, &device_, &selected, &context_);
        if (result == E_INVALIDARG) {
            result = D3D11CreateDevice(
                nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr,
                D3D11_CREATE_DEVICE_BGRA_SUPPORT |
                    D3D11_CREATE_DEVICE_VIDEO_SUPPORT,
                levels + 1, static_cast<UINT>(std::size(levels) - 1),
                D3D11_SDK_VERSION, &device_, &selected, &context_);
        }
        if (FAILED(result) || !device_ || !context_) {
            error = "The encoder probe could not create a D3D11 device.";
            return false;
        }
        D3D11_TEXTURE2D_DESC description{};
        description.Width = kWidth;
        description.Height = kHeight;
        description.MipLevels = 1;
        description.ArraySize = 1;
        description.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
        description.SampleDesc.Count = 1;
        description.Usage = D3D11_USAGE_DEFAULT;
        description.BindFlags =
            D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
        if (FAILED(device_->CreateTexture2D(
                &description, nullptr, &texture_)) || !texture_) {
            error = "The encoder probe could not create its BGRA texture.";
            return false;
        }
        pixels_ = std::make_shared<std::vector<std::uint8_t>>();
        FillMovingDesktop(*pixels_, 0);
        PublishDesktopCaptureD3D11Device(device_.Get());
        return true;
    }

    webrtc::scoped_refptr<webrtc::VideoFrameBuffer> Frame(
        std::uint32_t index)
    {
        FillMovingDesktop(*pixels_, index);
        if (!native_) {
            return webrtc::make_ref_counted<DesktopBgraFrameBuffer>(
                pixels_, kWidth * 4, kWidth, kHeight,
                0, 0, kWidth, kHeight, kWidth, kHeight);
        }
        context_->UpdateSubresource(
            texture_.Get(), 0, nullptr, pixels_->data(), kWidth * 4, 0);
        return webrtc::make_ref_counted<D3D11DesktopFrameBuffer>(
            texture_.Get(), kWidth, kHeight);
    }

private:
    bool native_ = false;
    std::shared_ptr<std::vector<std::uint8_t>> pixels_;
    ComPtr<ID3D11Device> device_;
    ComPtr<ID3D11DeviceContext> context_;
    ComPtr<ID3D11Texture2D> texture_;
};

class EncodeCollector final : public webrtc::EncodedImageCallback {
public:
    struct EncodedSample {
        std::uint32_t timestamp = 0;
        webrtc::VideoFrameType frameType =
            webrtc::VideoFrameType::kVideoFrameDelta;
        std::vector<std::uint8_t> bytes;
    };

    void OnFrameDropped(std::uint32_t, int, bool) override
    {
        std::lock_guard lock(mutex_);
        ++droppedFrames_;
    }

    void Submitted(std::uint32_t timestamp)
    {
        std::lock_guard lock(mutex_);
        submitted_[timestamp] = Clock::now();
    }

    Result OnEncodedImage(
        const webrtc::EncodedImage& image,
        const webrtc::CodecSpecificInfo*) override
    {
        std::lock_guard lock(mutex_);
        const auto found = submitted_.find(image.RtpTimestamp());
        if (found != submitted_.end()) {
            latencies_.push_back(
                std::chrono::duration<double, std::milli>(
                    Clock::now() - found->second).count());
            submitted_.erase(found);
        }
        ++frames_;
        bytes_ += image.size();
        EncodedSample sample;
        sample.timestamp = image.RtpTimestamp();
        sample.frameType = image._frameType;
        sample.bytes.assign(image.data(), image.data() + image.size());
        samples_.push_back(std::move(sample));
        if (image._frameType ==
            webrtc::VideoFrameType::kVideoFrameKey) {
            ++keyFrames_;
        }
        if (!HasAnnexBStartCode(image.data(), image.size())) {
            invalidOutput_ = true;
        }
        condition_.notify_all();
        return Result(Result::Error::OK);
    }

    bool WaitFor(std::uint32_t count, std::chrono::milliseconds timeout)
    {
        std::unique_lock lock(mutex_);
        return condition_.wait_for(lock, timeout, [&] {
            return frames_ >= count;
        });
    }

    void CopyTo(H264EncoderBenchmarkCandidate& result)
    {
        std::lock_guard lock(mutex_);
        result.encodedFrames = frames_;
        result.keyFrames = keyFrames_;
        result.encodedBytes = bytes_;
        if (droppedFrames_ != 0 && result.error.empty()) {
            result.error = "The encoder reported " +
                std::to_string(droppedFrames_) + " dropped frames.";
        }
        if (latencies_.size() > kWarmupFrames) {
            latencies_.erase(
                latencies_.begin(), latencies_.begin() + kWarmupFrames);
        }
        if (!latencies_.empty()) {
            result.averageLatencyMs = std::accumulate(
                latencies_.begin(), latencies_.end(), 0.0) /
                latencies_.size();
            std::sort(latencies_.begin(), latencies_.end());
            const std::size_t p95Index = (std::min)(
                latencies_.size() - 1,
                static_cast<std::size_t>(
                    std::ceil(latencies_.size() * 0.95)) - 1);
            result.p95LatencyMs = latencies_[p95Index];
        }
        if (invalidOutput_ && result.error.empty()) {
            result.error = "The encoder produced non-Annex-B H264 output.";
        }
    }

    std::vector<EncodedSample> Samples() const
    {
        std::lock_guard lock(mutex_);
        return samples_;
    }

private:
    mutable std::mutex mutex_;
    std::condition_variable condition_;
    std::map<std::uint32_t, Clock::time_point> submitted_;
    std::vector<double> latencies_;
    std::uint32_t frames_ = 0;
    std::uint32_t keyFrames_ = 0;
    std::uint64_t bytes_ = 0;
    std::uint32_t droppedFrames_ = 0;
    bool invalidOutput_ = false;
    std::vector<EncodedSample> samples_;
};

class QualityCollector final : public webrtc::DecodedImageCallback {
public:
    int32_t Decoded(webrtc::VideoFrame& frame) override
    {
        const auto i420 = frame.video_frame_buffer()
            ? frame.video_frame_buffer()->ToI420()
            : nullptr;
        if (!i420 || i420->width() != kWidth ||
            i420->height() != kHeight ||
            frame.rtp_timestamp() < 90'000) {
            return WEBRTC_VIDEO_CODEC_ERROR;
        }
        const std::uint32_t index =
            (frame.rtp_timestamp() - 90'000) / 3'000;
        std::vector<std::uint8_t> bgra;
        FillMovingDesktop(bgra, index);
        auto reference = webrtc::I420Buffer::Create(kWidth, kHeight);
        if (libyuv::ARGBToI420(
                bgra.data(), kWidth * 4,
                reference->MutableDataY(), reference->StrideY(),
                reference->MutableDataU(), reference->StrideU(),
                reference->MutableDataV(), reference->StrideV(),
                kWidth, kHeight) != 0) {
            return WEBRTC_VIDEO_CODEC_ERROR;
        }
        double squaredError = 0.0;
        for (int row = 0; row < kHeight; ++row) {
            const std::uint8_t* expected =
                reference->DataY() + row * reference->StrideY();
            const std::uint8_t* actual =
                i420->DataY() + row * i420->StrideY();
            for (int column = 0; column < kWidth; ++column) {
                const double difference =
                    static_cast<double>(expected[column]) - actual[column];
                squaredError += difference * difference;
            }
        }
        const double mse = squaredError /
            (static_cast<double>(kWidth) * kHeight);
        const double psnr = mse <= 1e-9
            ? 100.0
            : 10.0 * std::log10(255.0 * 255.0 / mse);
        {
            std::lock_guard lock(mutex_);
            psnr_.push_back(psnr);
        }
        condition_.notify_all();
        return WEBRTC_VIDEO_CODEC_OK;
    }

    bool WaitFor(std::size_t count)
    {
        std::unique_lock lock(mutex_);
        return condition_.wait_for(lock, std::chrono::seconds(5), [&] {
            return psnr_.size() >= count;
        });
    }

    double Average() const
    {
        std::lock_guard lock(mutex_);
        return psnr_.empty()
            ? 0.0
            : std::accumulate(psnr_.begin(), psnr_.end(), 0.0) /
                  psnr_.size();
    }

    std::size_t Count() const
    {
        std::lock_guard lock(mutex_);
        return psnr_.size();
    }

private:
    mutable std::mutex mutex_;
    std::condition_variable condition_;
    std::vector<double> psnr_;
};

double MeasureQuality(
    const std::vector<EncodeCollector::EncodedSample>& samples,
    std::string& error)
{
    if (samples.empty()) {
        error = "The encoder produced no frames for quality validation.";
        return 0.0;
    }
    auto factory = webrtc::CreateBuiltinVideoDecoderFactory();
    if (!factory) {
        error = "The quality validator could not create a decoder factory.";
        return 0.0;
    }
    const auto formats = factory->GetSupportedFormats();
    const auto format = std::find_if(
        formats.begin(), formats.end(), [](const auto& candidate) {
            return candidate.name == "H264";
        });
    if (format == formats.end()) {
        error = "The quality validator has no H264 decoder.";
        return 0.0;
    }
    auto decoder = factory->Create(webrtc::CreateEnvironment(), *format);
    if (!decoder) {
        error = "The quality validator could not create H264 decoder.";
        return 0.0;
    }
    webrtc::VideoDecoder::Settings settings;
    settings.set_codec_type(webrtc::kVideoCodecH264);
    settings.set_number_of_cores(2);
    settings.set_max_render_resolution(
        webrtc::RenderResolution(kWidth, kHeight));
    if (!decoder->Configure(settings)) {
        error = "The quality validator could not configure H264 decoding.";
        return 0.0;
    }
    QualityCollector collector;
    decoder->RegisterDecodeCompleteCallback(&collector);
    std::size_t submitted = 0;
    for (const auto& sample : samples) {
        webrtc::EncodedImage image;
        image.SetEncodedData(webrtc::EncodedImageBuffer::Create(
            sample.bytes.data(), sample.bytes.size()));
        image.SetRtpTimestamp(sample.timestamp);
        image.SetFrameType(sample.frameType);
        image._encodedWidth = kWidth;
        image._encodedHeight = kHeight;
        if (decoder->Decode(image, 0) >= 0) {
            ++submitted;
        }
    }
    collector.WaitFor(submitted > 2 ? submitted - 2 : submitted);
    decoder->Release();
    if (collector.Count() < submitted * 3 / 4) {
        error = "The encoded stream could not be decoded reliably for "
                "quality validation.";
        return 0.0;
    }
    return collector.Average();
}

std::unique_ptr<webrtc::VideoEncoder> BuiltinH264Encoder(
    FfmpegX264Preset quality, std::string& error)
{
    auto factory = std::make_unique<QualityOpenH264EncoderFactory>(quality);
    if (!factory) {
        error = "The WebRTC built-in encoder factory is unavailable.";
        return nullptr;
    }
    const auto formats = factory->GetSupportedFormats();
    const auto format = std::find_if(
        formats.begin(), formats.end(), [](const auto& candidate) {
            return candidate.name == "H264";
        });
    if (format == formats.end()) {
        error = "The WebRTC built-in factory does not provide H264.";
        return nullptr;
    }
    return factory->Create(webrtc::CreateEnvironment(), *format);
}

H264EncoderBenchmarkCandidate BenchmarkCandidate(
    std::string id,
    std::string fallbackName,
    bool hardware,
    std::unique_ptr<webrtc::VideoEncoder> encoder,
    BenchmarkFrameSource& source)
{
    H264EncoderBenchmarkCandidate result;
    result.id = std::move(id);
    result.name = std::move(fallbackName);
    result.hardware = hardware;
    if (!encoder) {
        result.error = "The encoder candidate is unavailable.";
        return result;
    }
    const auto info = encoder->GetEncoderInfo();
    if (!info.implementation_name.empty()) {
        result.name = info.implementation_name;
    }
    EncodeCollector collector;
    encoder->RegisterEncodeCompleteCallback(&collector);
    webrtc::VideoCodec codec{};
    codec.codecType = webrtc::kVideoCodecH264;
    codec.width = kWidth;
    codec.height = kHeight;
    codec.minBitrate = 300;
    codec.startBitrate = kBitrateBps / 1000;
    codec.maxBitrate = kBitrateBps / 1000;
    codec.maxFramerate = kFrameRate;
    codec.mode = webrtc::VideoCodecMode::kScreensharing;
    *codec.H264() = webrtc::VideoEncoder::GetDefaultH264Settings();
    const auto settings = webrtc::VideoEncoder::Settings(
        webrtc::VideoEncoder::Capabilities(false), 4, 1200);
    const int initialize = encoder->InitEncode(&codec, settings);
    if (initialize != WEBRTC_VIDEO_CODEC_OK) {
        result.error = "InitEncode returned " +
            std::to_string(initialize) + ".";
        encoder->Release();
        return result;
    }
    const auto initializedInfo = encoder->GetEncoderInfo();
    if (!initializedInfo.implementation_name.empty()) {
        result.name = initializedInfo.implementation_name;
    }
    webrtc::VideoBitrateAllocation allocation;
    allocation.SetBitrate(0, 0, kBitrateBps);
    encoder->SetRates(webrtc::VideoEncoder::RateControlParameters(
        allocation, kFrameRate));

    HighResolutionFramePacer framePacer;
    if (!framePacer.IsValid()) {
        result.error = "The high-resolution frame timer is unavailable.";
        encoder->Release();
        return result;
    }
    const double cpuStarted = ProcessCpuMilliseconds();
    const auto deliveryStarted = Clock::now();
    auto deliveryDeadline = deliveryStarted;
    constexpr auto frameInterval =
        std::chrono::microseconds(1'000'000 / kFrameRate);
    for (std::uint32_t index = 0; index < kFrameCount; ++index) {
        auto buffer = source.Frame(index);
        const std::uint32_t timestamp = 90'000 + index * 3'000;
        auto frame = webrtc::VideoFrame::Builder()
            .set_video_frame_buffer(std::move(buffer))
            .set_timestamp_us(static_cast<std::int64_t>(index) * 33'333)
            .set_rtp_timestamp(timestamp)
            .build();
        const std::vector<webrtc::VideoFrameType> frameTypes{
            index == 0 || index == kFrameCount / 2
                ? webrtc::VideoFrameType::kVideoFrameKey
                : webrtc::VideoFrameType::kVideoFrameDelta};
        collector.Submitted(timestamp);
        const int encoded = encoder->Encode(frame, &frameTypes);
        if (encoded != WEBRTC_VIDEO_CODEC_OK) {
            result.error = "Encode returned " +
                std::to_string(encoded) + " at frame " +
                std::to_string(index) + ".";
            if (const auto* ffmpegHardware = dynamic_cast<
                    FfmpegHardwareH264Encoder*>(encoder.get())) {
                const std::string detail = ffmpegHardware->LastError();
                if (!detail.empty()) {
                    result.error += " " + detail;
                }
            }
            break;
        }
        ++result.submittedFrames;
        // Match the configured desktop cadence instead of submitting the
        // entire sample as a burst. This prevents an asynchronous hardware
        // encoder's driver queue from being scored as avoidable per-frame
        // latency while retaining the same end-to-end callback measurement.
        deliveryDeadline += frameInterval;
        if (!framePacer.WaitUntil(deliveryDeadline)) {
            result.error = "The high-resolution frame timer wait failed.";
            break;
        }
    }
    collector.WaitFor(result.submittedFrames, std::chrono::seconds(5));
    const double deliverySeconds = std::chrono::duration<double>(
        Clock::now() - deliveryStarted).count();
    const double cpuEnded = ProcessCpuMilliseconds();
    collector.CopyTo(result);
    if (deliverySeconds > 0.0) {
        // This is the observed end-to-end output rate, including any wait for
        // asynchronous driver callbacks, rather than merely the submit pace.
        result.inputFramesPerSecond =
            result.encodedFrames / deliverySeconds;
    }
    const auto encodedSamples = collector.Samples();
    std::string executionWarning;
    if (!result.error.empty() && result.submittedFrames != 0 &&
        result.encodedFrames >= result.submittedFrames * 3 / 4) {
        // Old hardware drivers may reject a later frame after a long stable
        // run. Such a backend remains useful in production and must compete
        // by its measured output rate; retain the incident as a warning.
        executionWarning = std::move(result.error);
        result.error.clear();
    }
    if (result.error.empty() &&
        result.submittedFrames == kFrameCount &&
        result.encodedFrames >= result.submittedFrames * 9 / 10) {
        // Keep live bitrate reconfiguration out of the performance score.
        // Some old QSV drivers reject it even though their steady-state
        // desktop encoding is fast and reliable. Probe it after all measured
        // samples have been captured and report it as compatibility evidence.
        result.dynamicRateControlTested = true;
        webrtc::VideoBitrateAllocation reduced;
        reduced.SetBitrate(0, 0, kBitrateBps * 4 / 5);
        encoder->SetRates(webrtc::VideoEncoder::RateControlParameters(
            reduced, kFrameRate));
        const std::uint32_t probeTimestamp =
            90'000 + kFrameCount * 3'000;
        auto probeFrame = webrtc::VideoFrame::Builder()
            .set_video_frame_buffer(source.Frame(kFrameCount))
            .set_timestamp_us(
                static_cast<std::int64_t>(kFrameCount) * 33'333)
            .set_rtp_timestamp(probeTimestamp)
            .build();
        const std::vector<webrtc::VideoFrameType> probeFrameTypes{
            webrtc::VideoFrameType::kVideoFrameDelta};
        const int rateProbeResult =
            encoder->Encode(probeFrame, &probeFrameTypes);
        if (rateProbeResult != WEBRTC_VIDEO_CODEC_OK) {
            result.warning =
                "Dynamic bitrate adjustment returned " +
                std::to_string(rateProbeResult) + ".";
            if (const auto* ffmpegHardware = dynamic_cast<
                    FfmpegHardwareH264Encoder*>(encoder.get())) {
                const std::string detail = ffmpegHardware->LastError();
                if (!detail.empty()) {
                    result.warning += " " + detail;
                }
            }
        } else if (!collector.WaitFor(
                       result.encodedFrames + 1,
                       std::chrono::seconds(1))) {
            result.warning =
                "Dynamic bitrate adjustment was accepted but produced no "
                "frame within one second.";
        } else {
            result.dynamicRateControlPassed = true;
        }
    }
    encoder->Release();
    if (result.submittedFrames != 0) {
        result.cpuTimePerFrameMs =
            (std::max)(0.0, cpuEnded - cpuStarted) /
            result.submittedFrames;
    }
    if (result.error.empty() &&
        (result.submittedFrames == 0 || result.encodedFrames == 0 ||
         result.encodedFrames < result.submittedFrames * 3 / 4 ||
         result.keyFrames == 0 || result.encodedBytes < 4096)) {
        result.error =
            "The encoder dropped too many frames or produced too little "
            "valid H264 data.";
    }
    if (result.error.empty()) {
        result.averageLumaPsnrDb =
            MeasureQuality(encodedSamples, result.error);
    }
    AppendWarning(result.warning, std::move(executionWarning));
    result.passed = result.error.empty();
    if (result.passed) {
        // P95 protects interaction consistency. CPU cost has equal weight so
        // a software encoder whose callback is fast but which occupies most
        // of a core cannot incorrectly beat an efficient hardware path.
        result.score = result.p95LatencyMs * 0.45 +
                       result.averageLatencyMs * 0.15 +
                       result.cpuTimePerFrameMs * 0.40;
    }
    return result;
}

}  // namespace

H264EncoderBenchmarkResult RunH264EncoderBenchmark(
    H264EncoderBenchmarkInput input,
    FfmpegX264Preset ffmpegX264Preset)
{
    H264EncoderBenchmarkResult result;
    result.input = input;
    BenchmarkFrameSource source;
    if (!source.Initialize(input, result.error)) {
        return result;
    }

    result.candidates.push_back(BenchmarkCandidate(
        kAutomaticEncoderMf, "Media Foundation H264", true,
        std::make_unique<MfD3D11H264Encoder>(
            nullptr, ffmpegX264Preset), source));
    for (const auto [backend, id, name] : {
             std::tuple{FfmpegHardwareBackend::kQsv,
                        kAutomaticEncoderFfmpegQsv, "FFmpeg/QSV"},
             std::tuple{FfmpegHardwareBackend::kNvenc,
                        kAutomaticEncoderFfmpegNvenc, "FFmpeg/NVENC"},
             std::tuple{FfmpegHardwareBackend::kAmf,
                        kAutomaticEncoderFfmpegAmf, "FFmpeg/AMF"}}) {
        if (!FfmpegHardwareH264Encoder::RuntimeAvailable(backend)) {
            continue;
        }
        auto candidate = BenchmarkCandidate(
            id, name, true,
            std::make_unique<FfmpegHardwareH264Encoder>(
                backend, nullptr, ffmpegX264Preset), source);
        // A backend that cannot initialize and produce even one frame is not
        // supported by this machine and should not appear as a selectable or
        // benchmark candidate.
        if (candidate.submittedFrames >= 3 &&
            candidate.encodedFrames >= 3 &&
            candidate.keyFrames >= 1 && candidate.encodedBytes != 0) {
            result.candidates.push_back(std::move(candidate));
        }
    }
    std::string builtinError;
    auto builtin = BuiltinH264Encoder(
        ffmpegX264Preset, builtinError);
    auto builtinResult = BenchmarkCandidate(
        kAutomaticEncoderOpenH264, "OpenH264", false,
        std::move(builtin), source);
    if (!builtinError.empty() && builtinResult.error.empty()) {
        builtinResult.error = std::move(builtinError);
    }
    result.candidates.push_back(std::move(builtinResult));
    if (FfmpegX264H264Encoder::RuntimeAvailable()) {
        result.candidates.push_back(BenchmarkCandidate(
            kAutomaticEncoderFfmpegX264,
            std::string("FFmpeg/libx264 (") +
                VideoEncoderQualityProfileForPreset(ffmpegX264Preset)
                    .x264Preset + ")",
            false,
            std::make_unique<FfmpegX264H264Encoder>(
                ffmpegX264Preset), source));
    }

    const std::string inputPath =
        input == H264EncoderBenchmarkInput::kD3D11Bgra
            ? "D3D11 BGRA desktop texture"
            : "CPU BGRA desktop frame";
    for (auto& candidate : result.candidates) {
        candidate.inputPath = inputPath;
    }

    const auto best = std::min_element(
        result.candidates.begin(), result.candidates.end(),
        [](const auto& left, const auto& right) {
            if (left.passed != right.passed) {
                return left.passed;
            }
            if (!left.passed) {
                return false;
            }
            if (std::abs(left.inputFramesPerSecond -
                         right.inputFramesPerSecond) > 0.25) {
                return left.inputFramesPerSecond >
                       right.inputFramesPerSecond;
            }
            return left.score < right.score;
        });
    if (best == result.candidates.end() || !best->passed) {
        result.error =
            "No usable H264 encoder produced valid output in the benchmark.";
        return result;
    }
    result.passed = true;
    result.bestEncoderId = best->id;
    result.bestEncoderName = best->name;
    return result;
}

}  // namespace remote
