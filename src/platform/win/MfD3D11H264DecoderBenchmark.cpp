// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#include "MfD3D11H264DecoderBenchmark.h"

#include <algorithm>
#include <chrono>
#include <cmath>
#include <condition_variable>
#include <cstdint>
#include <cstring>
#include <functional>
#include <map>
#include <mutex>
#include <numeric>
#include <string>
#include <thread>
#include <utility>
#include <vector>

#include <Windows.h>
#include <mfapi.h>
#include <mfidl.h>
#include <mftransform.h>

#include "api/environment/environment_factory.h"
#include "api/make_ref_counted.h"
#include "api/video/encoded_image.h"
#include "api/video/i420_buffer.h"
#include "api/video/video_frame.h"
#include "api/video/video_frame_type.h"
#include "api/video_codecs/builtin_video_encoder_factory.h"
#include "api/video_codecs/builtin_video_decoder_factory.h"
#include "api/video/video_bitrate_allocation.h"
#include "api/video_codecs/video_codec.h"
#include "api/video_codecs/video_encoder.h"
#include "modules/video_coding/include/video_error_codes.h"
#include "MfD3D11H264Decoder.h"
#include "FfmpegD3D11H264Decoder.h"
#include "D3D11NativeFrameBuffer.h"

namespace remote {
namespace {

using Clock = std::chrono::steady_clock;

// Match the application's common desktop workload. A decoder that only
// passes at 720p can still exhibit severe queueing at the actual 1080p/60
// target, especially during the sparse phase below.
constexpr int kWidth = 1920;
constexpr int kHeight = 1080;
constexpr std::uint32_t kFrameRate = 60;
constexpr std::size_t kFrameCount = 72;
constexpr std::size_t kWarmupFrameCount = 8;
constexpr std::uint32_t kBitrateBps = 6'000'000;
constexpr std::size_t kRealtimeFrameCount = 52;
constexpr std::uint32_t kSparseFrameRate = 5;
constexpr double kMaximumRealtimeAverageLatencyMs = 45.0;
constexpr double kMaximumRealtimeP95LatencyMs = 80.0;
constexpr double kMaximumSparseAverageLatencyMs = 120.0;
constexpr double kMaximumSparseP95LatencyMs = 180.0;

std::string WideToUtf8(const std::wstring& value)
{
    if (value.empty()) {
        return {};
    }
    const int required = WideCharToMultiByte(
        CP_UTF8, 0, value.data(), static_cast<int>(value.size()),
        nullptr, 0, nullptr, nullptr);
    if (required <= 0) {
        return {};
    }
    std::string result(static_cast<std::size_t>(required), '\0');
    WideCharToMultiByte(
        CP_UTF8, 0, value.data(), static_cast<int>(value.size()),
        result.data(), required, nullptr, nullptr);
    return result;
}

std::string ActivationName(IMFActivate* activation)
{
    WCHAR* value = nullptr;
    UINT32 length = 0;
    std::wstring name;
    if (activation && SUCCEEDED(activation->GetAllocatedString(
            MFT_FRIENDLY_NAME_Attribute, &value, &length))) {
        name.assign(value, length);
    }
    CoTaskMemFree(value);
    return WideToUtf8(name);
}

void ReleaseActivations(IMFActivate** activations, UINT32 count)
{
    if (!activations) {
        return;
    }
    for (UINT32 index = 0; index < count; ++index) {
        if (activations[index]) {
            activations[index]->Release();
        }
    }
    CoTaskMemFree(activations);
}

std::vector<std::string> EnumerateCandidateNames(std::string& error)
{
    const MFT_REGISTER_TYPE_INFO inputType{
        MFMediaType_Video, MFVideoFormat_H264};
    const MFT_REGISTER_TYPE_INFO outputType{
        MFMediaType_Video, MFVideoFormat_NV12};
    IMFActivate** activations = nullptr;
    UINT32 count = 0;
    const HRESULT result = MFTEnumEx(
        MFT_CATEGORY_VIDEO_DECODER,
        MFT_ENUM_FLAG_ALL | MFT_ENUM_FLAG_SORTANDFILTER,
        &inputType, &outputType, &activations, &count);
    if (FAILED(result) || count == 0) {
        error = "No H264 decoder MFT candidate was found.";
        ReleaseActivations(activations, count);
        return {};
    }
    std::vector<std::string> names;
    for (UINT32 index = 0; index < count; ++index) {
        auto name = ActivationName(activations[index]);
        if (!name.empty() &&
            std::find(names.begin(), names.end(), name) == names.end()) {
            names.push_back(std::move(name));
        }
    }
    ReleaseActivations(activations, count);
    if (names.empty()) {
        error = "The H264 decoder MFT candidates have no usable names.";
    }
    return names;
}

struct EncodedFrame {
    std::vector<std::uint8_t> bytes;
    std::uint32_t rtpTimestamp = 0;
    webrtc::VideoFrameType frameType =
        webrtc::VideoFrameType::kVideoFrameDelta;
};

class EncodeCollector final : public webrtc::EncodedImageCallback {
public:
    Result OnEncodedImage(
        const webrtc::EncodedImage& image,
        const webrtc::CodecSpecificInfo*) override
    {
        EncodedFrame frame;
        frame.bytes.assign(image.data(), image.data() + image.size());
        frame.rtpTimestamp = image.RtpTimestamp();
        frame.frameType = image._frameType;
        {
            std::lock_guard lock(mutex_);
            frames_.push_back(std::move(frame));
        }
        condition_.notify_all();
        return Result(Result::OK, image.RtpTimestamp());
    }

    void OnFrameDropped(std::uint32_t, int, bool) override {}

    bool WaitForFrames(std::size_t count, std::chrono::milliseconds timeout)
    {
        std::unique_lock lock(mutex_);
        return condition_.wait_for(
            lock, timeout, [this, count] { return frames_.size() >= count; });
    }

    std::vector<EncodedFrame> TakeFrames()
    {
        std::lock_guard lock(mutex_);
        return std::move(frames_);
    }

private:
    std::mutex mutex_;
    std::condition_variable condition_;
    std::vector<EncodedFrame> frames_;
};

void FillSyntheticFrame(webrtc::I420Buffer* buffer, std::size_t frameIndex)
{
    std::memset(buffer->MutableDataY(), 32,
                static_cast<std::size_t>(buffer->StrideY()) * kHeight);
    std::memset(buffer->MutableDataU(), 128,
                static_cast<std::size_t>(buffer->StrideU()) *
                    ((kHeight + 1) / 2));
    std::memset(buffer->MutableDataV(), 128,
                static_cast<std::size_t>(buffer->StrideV()) *
                    ((kHeight + 1) / 2));
    const int squareSize = 160;
    const int startX = static_cast<int>((frameIndex * 19) %
        (kWidth - squareSize));
    const int startY = static_cast<int>((frameIndex * 11) %
        (kHeight - squareSize));
    for (int y = startY; y < startY + squareSize; ++y) {
        std::memset(buffer->MutableDataY() + y * buffer->StrideY() + startX,
                    static_cast<int>(160 + (frameIndex % 64)), squareSize);
    }
}

std::vector<EncodedFrame> BuildSyntheticH264(std::string& error)
{
    auto factory = webrtc::CreateBuiltinVideoEncoderFactory();
    if (!factory) {
        error = "The builtin video encoder factory is unavailable.";
        return {};
    }
    const auto formats = factory->GetSupportedFormats();
    const auto format = std::find_if(
        formats.begin(), formats.end(), [](const auto& candidate) {
            return candidate.name == "H264";
        });
    if (format == formats.end()) {
        error = "The builtin OpenH264 encoder is unavailable.";
        return {};
    }
    auto encoder = factory->Create(webrtc::CreateEnvironment(), *format);
    if (!encoder) {
        error = "The builtin OpenH264 encoder could not be created.";
        return {};
    }

    EncodeCollector collector;
    encoder->RegisterEncodeCompleteCallback(&collector);
    webrtc::VideoCodec codec;
    codec.codecType = webrtc::kVideoCodecH264;
    codec.width = kWidth;
    codec.height = kHeight;
    codec.startBitrate = kBitrateBps / 1000;
    codec.minBitrate = 300;
    codec.maxBitrate = kBitrateBps / 1000;
    codec.maxFramerate = kFrameRate;
    codec.active = true;
    codec.qpMax = 51;
    codec.mode = webrtc::VideoCodecMode::kScreensharing;
    codec.numberOfSimulcastStreams = 0;
    *codec.H264() = webrtc::VideoEncoder::GetDefaultH264Settings();
    const webrtc::VideoEncoder::Settings settings(
        webrtc::VideoEncoder::Capabilities(false), 4, 1200);
    if (encoder->InitEncode(&codec, settings) != WEBRTC_VIDEO_CODEC_OK) {
        error = "The builtin OpenH264 encoder rejected the probe settings.";
        encoder->Release();
        return {};
    }
    webrtc::VideoBitrateAllocation allocation;
    allocation.SetBitrate(0, 0, kBitrateBps);
    encoder->SetRates(
        webrtc::VideoEncoder::RateControlParameters(allocation, kFrameRate));

    std::uint32_t rtpTimestamp = 90'000;
    const std::uint32_t timestampStep = 90'000 / kFrameRate;
    for (std::size_t index = 0; index < kFrameCount; ++index) {
        auto buffer = webrtc::I420Buffer::Create(kWidth, kHeight);
        FillSyntheticFrame(buffer.get(), index);
        auto frame = webrtc::VideoFrame::Builder()
            .set_video_frame_buffer(buffer)
            .set_rtp_timestamp(rtpTimestamp)
            .set_timestamp_us(static_cast<std::int64_t>(index) *
                              1'000'000 / kFrameRate)
            .build();
        const std::vector<webrtc::VideoFrameType> frameTypes{
            index == 0 ? webrtc::VideoFrameType::kVideoFrameKey
                       : webrtc::VideoFrameType::kVideoFrameDelta};
        if (encoder->Encode(frame, &frameTypes) < 0) {
            error = "OpenH264 failed while generating probe frames.";
            encoder->Release();
            return {};
        }
        rtpTimestamp += timestampStep;
    }
    collector.WaitForFrames(kFrameCount, std::chrono::seconds(10));
    encoder->Release();
    auto frames = collector.TakeFrames();
    if (frames.size() < kFrameCount / 2) {
        error = "OpenH264 produced too few probe frames.";
        return {};
    }
    return frames;
}

class DecodeCollector final : public webrtc::DecodedImageCallback {
public:
    struct Submission {
        Clock::time_point submittedAt;
        bool sparse = false;
    };

    explicit DecodeCollector(
        std::map<std::uint32_t, Submission>* submitted)
        : submitted_(submitted)
    {}

    int32_t Decoded(webrtc::VideoFrame& frame) override
    {
        const auto now = Clock::now();
        std::lock_guard lock(mutex_);
        const auto found = submitted_->find(frame.rtp_timestamp());
        if (found != submitted_->end()) {
            const double latencyMs =
                std::chrono::duration<double, std::milli>(
                    now - found->second.submittedAt).count();
            latenciesMs_.push_back(latencyMs);
            (found->second.sparse ? sparseLatenciesMs_
                                  : realtimeLatenciesMs_)
                .push_back(latencyMs);
        }
        const auto buffer = frame.video_frame_buffer();
        nativeOutput_ = nativeOutput_ ||
            D3D11NativeFrameBuffer::From(buffer.get()) != nullptr;
        condition_.notify_all();
        return WEBRTC_VIDEO_CODEC_OK;
    }

    bool WaitForCount(std::size_t count, std::chrono::milliseconds timeout)
    {
        std::unique_lock lock(mutex_);
        return condition_.wait_for(lock, timeout, [this, count] {
            return latenciesMs_.size() >= count;
        });
    }

    std::vector<double> Latencies() const
    {
        std::lock_guard lock(mutex_);
        return latenciesMs_;
    }

    std::vector<double> RealtimeLatencies() const
    {
        std::lock_guard lock(mutex_);
        return realtimeLatenciesMs_;
    }

    std::vector<double> SparseLatencies() const
    {
        std::lock_guard lock(mutex_);
        return sparseLatenciesMs_;
    }

    bool NativeOutput() const
    {
        std::lock_guard lock(mutex_);
        return nativeOutput_;
    }

private:
    std::map<std::uint32_t, Submission>* submitted_;
    mutable std::mutex mutex_;
    std::condition_variable condition_;
    std::vector<double> latenciesMs_;
    std::vector<double> realtimeLatenciesMs_;
    std::vector<double> sparseLatenciesMs_;
    bool nativeOutput_ = false;
};

void SummarizeLatencies(
    std::vector<double> latencies,
    std::size_t discardLeading,
    std::uint32_t* decodedFrames,
    double* averageLatencyMs,
    double* p95LatencyMs)
{
    if (!decodedFrames || !averageLatencyMs || !p95LatencyMs) {
        return;
    }
    if (latencies.size() > discardLeading) {
        latencies.erase(
            latencies.begin(), latencies.begin() + discardLeading);
    }
    *decodedFrames = static_cast<std::uint32_t>(latencies.size());
    if (latencies.empty()) {
        *averageLatencyMs = 0.0;
        *p95LatencyMs = 0.0;
        return;
    }
    const double sum = std::accumulate(
        latencies.begin(), latencies.end(), 0.0);
    *averageLatencyMs = sum / latencies.size();
    std::sort(latencies.begin(), latencies.end());
    const std::size_t p95Index = std::min(
        latencies.size() - 1,
        static_cast<std::size_t>(
            std::ceil(latencies.size() * 0.95)) - 1);
    *p95LatencyMs = latencies[p95Index];
}

MfD3D11H264DecoderBenchmarkCandidate BenchmarkDecoder(
    const std::string& name,
    const std::vector<EncodedFrame>& frames,
    std::unique_ptr<webrtc::VideoDecoder> decoder,
    bool hardware,
    bool asynchronous,
    bool requireNativeD3D11Output,
    std::function<bool()> queryAsynchronous = {},
    std::function<std::string()> queryError = {})
{
    MfD3D11H264DecoderBenchmarkCandidate result;
    result.name = name;
    result.hardware = hardware;
    result.asynchronous = asynchronous;
    if (!decoder) {
        result.error = "The decoder factory did not create a decoder.";
        return result;
    }
    webrtc::VideoDecoder::Settings settings;
    settings.set_codec_type(webrtc::kVideoCodecH264);
    settings.set_number_of_cores(1);
    settings.set_max_render_resolution(
        webrtc::RenderResolution(kWidth, kHeight));
    if (!decoder->Configure(settings)) {
        result.error = queryError ? queryError() : std::string{};
        if (result.error.empty()) {
            result.error = "The decoder could not be configured.";
        }
        return result;
    }
    if (queryAsynchronous) {
        result.asynchronous = queryAsynchronous();
    }
    std::map<std::uint32_t, DecodeCollector::Submission> submitted;
    DecodeCollector collector(&submitted);
    decoder->RegisterDecodeCompleteCallback(&collector);
    auto deadline = Clock::now();
    std::uint32_t errors = 0;
    std::size_t frameIndex = 0;
    for (const auto& encoded : frames) {
        webrtc::EncodedImage image;
        image.SetEncodedData(webrtc::EncodedImageBuffer::Create(
            encoded.bytes.data(), encoded.bytes.size()));
        image.SetRtpTimestamp(encoded.rtpTimestamp);
        image.SetFrameType(encoded.frameType);
        image._encodedWidth = kWidth;
        image._encodedHeight = kHeight;
        const bool sparse = frameIndex >= kRealtimeFrameCount;
        submitted[encoded.rtpTimestamp] = {
            Clock::now(), sparse};
        if (decoder->Decode(image, 0) < 0) {
            ++errors;
        }
        const std::uint32_t deliveryRate =
            frameIndex++ < kRealtimeFrameCount
                ? kFrameRate
                : kSparseFrameRate;
        deadline += std::chrono::microseconds(
            1'000'000 / deliveryRate);
        std::this_thread::sleep_until(deadline);
    }
    collector.WaitForCount(
        frames.size() > 2 ? frames.size() - 2 : frames.size(),
        std::chrono::seconds(3));
    auto latencies = collector.Latencies();
    result.decodedFrames = static_cast<std::uint32_t>(latencies.size());
    result.nativeD3D11Output = collector.NativeOutput();
    decoder->Release();
    if (errors != 0 || latencies.size() < frames.size() * 3 / 4 ||
        (requireNativeD3D11Output && !result.nativeD3D11Output)) {
        result.error = errors != 0
            ? "The decoder returned errors during the probe."
            : "The decoder produced too few native D3D11 frames.";
        return result;
    }
    SummarizeLatencies(
        latencies, kWarmupFrameCount, &result.decodedFrames,
        &result.averageLatencyMs, &result.p95LatencyMs);
    SummarizeLatencies(
        collector.RealtimeLatencies(), kWarmupFrameCount,
        &result.realtimeDecodedFrames,
        &result.realtimeAverageLatencyMs,
        &result.realtimeP95LatencyMs);
    SummarizeLatencies(
        collector.SparseLatencies(), 0,
        &result.sparseDecodedFrames,
        &result.sparseAverageLatencyMs,
        &result.sparseP95LatencyMs);
    if (result.realtimeDecodedFrames == 0 ||
        result.sparseDecodedFrames == 0 ||
        result.realtimeAverageLatencyMs >
            kMaximumRealtimeAverageLatencyMs ||
        result.realtimeP95LatencyMs >
            kMaximumRealtimeP95LatencyMs ||
        result.sparseAverageLatencyMs >
            kMaximumSparseAverageLatencyMs ||
        result.sparseP95LatencyMs >
            kMaximumSparseP95LatencyMs) {
        result.error =
            "The decoder exceeded the remote-control low-latency "
            "limit in the continuous or sparse cadence phase.";
        return result;
    }
    result.passed = true;
    return result;
}

MfD3D11H264DecoderBenchmarkCandidate BenchmarkHardwareCandidate(
    const std::string& name,
    const std::vector<EncodedFrame>& frames)
{
    auto decoder = std::make_unique<MfD3D11H264Decoder>(
        MfD3D11H264DecoderSelection::kHardwareOnly, name, true);
    auto* concreteDecoder = decoder.get();
    return BenchmarkDecoder(name, frames, std::move(decoder), true,
                            false, true,
                            [concreteDecoder] {
                                return concreteDecoder->IsAsynchronous();
                            },
                            [concreteDecoder] {
                                return concreteDecoder->LastError();
                            });
}

MfD3D11H264DecoderBenchmarkCandidate BenchmarkD3D11VaCandidate(
    const std::vector<EncodedFrame>& frames)
{
    auto decoder = std::make_unique<FfmpegD3D11H264Decoder>();
    auto* concreteDecoder = decoder.get();
    return BenchmarkDecoder(
        kFfmpegD3D11H264DecoderName, frames, std::move(decoder), true,
        false, true, {},
        [concreteDecoder] { return concreteDecoder->LastError(); });
}

MfD3D11H264DecoderBenchmarkCandidate BenchmarkSoftwareCandidate(
    const std::vector<EncodedFrame>& frames)
{
    constexpr auto kName = "FFmpeg (WebRTC Builtin)";
    auto factory = webrtc::CreateBuiltinVideoDecoderFactory();
    if (!factory) {
        MfD3D11H264DecoderBenchmarkCandidate result;
        result.name = kName;
        result.hardware = false;
        result.error = "The built-in decoder factory is unavailable.";
        return result;
    }
    const auto formats = factory->GetSupportedFormats();
    const auto format = std::find_if(
        formats.begin(), formats.end(), [](const auto& candidate) {
            return candidate.name == "H264";
        });
    if (format == formats.end()) {
        MfD3D11H264DecoderBenchmarkCandidate result;
        result.name = kName;
        result.hardware = false;
        result.error = "The built-in factory does not provide H264.";
        return result;
    }
    return BenchmarkDecoder(
        kName, frames,
        factory->Create(webrtc::CreateEnvironment(), *format), false,
        false, false);
}

}  // namespace

MfD3D11H264DecoderBenchmarkResult
RunMfD3D11H264DecoderBenchmark()
{
    MfD3D11H264DecoderBenchmarkResult result;
    const HRESULT comResult = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
    const bool uninitializeCom = SUCCEEDED(comResult);
    if (FAILED(comResult) && comResult != RPC_E_CHANGED_MODE) {
        result.error = "CoInitializeEx failed for the decoder probe.";
        return result;
    }
    const HRESULT startupResult = MFStartup(MF_VERSION, MFSTARTUP_FULL);
    if (FAILED(startupResult)) {
        result.error = "MFStartup failed for the decoder probe.";
        if (uninitializeCom) {
            CoUninitialize();
        }
        return result;
    }

    std::string enumerateError;
    const auto candidateNames = EnumerateCandidateNames(enumerateError);
    std::string encodeError;
    const auto frames = BuildSyntheticH264(encodeError);
    if (frames.empty()) {
        result.error = encodeError;
    } else {
        result.candidates.push_back(BenchmarkSoftwareCandidate(frames));
        result.candidates.push_back(BenchmarkD3D11VaCandidate(frames));
        for (const auto& name : candidateNames) {
            result.candidates.push_back(
                BenchmarkHardwareCandidate(name, frames));
        }
        const auto best = std::min_element(
            result.candidates.begin(), result.candidates.end(),
            [](const auto& left, const auto& right) {
                const bool leftEligible = left.hardware && left.passed;
                const bool rightEligible = right.hardware && right.passed;
                if (leftEligible != rightEligible) {
                    return leftEligible;
                }
                if (!leftEligible) {
                    return false;
                }
                if (left.asynchronous != right.asynchronous) {
                    return left.asynchronous;
                }
                if (left.sparseP95LatencyMs !=
                    right.sparseP95LatencyMs) {
                    return left.sparseP95LatencyMs <
                           right.sparseP95LatencyMs;
                }
                if (left.realtimeP95LatencyMs !=
                    right.realtimeP95LatencyMs) {
                    return left.realtimeP95LatencyMs <
                           right.realtimeP95LatencyMs;
                }
                return left.averageLatencyMs < right.averageLatencyMs;
            });
        if (best != result.candidates.end() && best->hardware &&
            best->passed) {
            result.passed = true;
            result.bestDecoderName = best->name;
            result.bestAverageLatencyMs = best->averageLatencyMs;
            result.bestP95LatencyMs = best->p95LatencyMs;
        } else {
            result.error = "No candidate passed the native D3D11 benchmark.";
        }
    }
    MFShutdown();
    if (uninitializeCom) {
        CoUninitialize();
    }
    return result;
}

}  // namespace remote
