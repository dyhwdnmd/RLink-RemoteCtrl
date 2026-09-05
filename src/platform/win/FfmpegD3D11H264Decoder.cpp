// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#include "FfmpegD3D11H264Decoder.h"

#include <Windows.h>
#include <d3d11.h>

#include <algorithm>
#include <cstdint>
#include <cstring>
#include <deque>
#include <filesystem>
#include <memory>
#include <mutex>
#include <optional>
#include <string>
#include <utility>

extern "C" {
#include "libavcodec/avcodec.h"
#include "libavcodec/packet.h"
#include "libavutil/buffer.h"
#include "libavutil/error.h"
#include "libavutil/frame.h"
#include "libavutil/hwcontext.h"
#include "libavutil/pixfmt.h"
}

#include "api/make_ref_counted.h"
#include "api/video/video_frame.h"
#include "modules/video_coding/include/video_error_codes.h"
#include "D3D11NativeFrameBuffer.h"
#include "src/webrtc/VideoCodecTimingTelemetry.h"

namespace remote {
namespace {

template <typename Function>
bool ResolveFunction(HMODULE module, const char* name, Function* target)
{
    if (!module || !name || !target) {
        return false;
    }
    *target = reinterpret_cast<Function>(GetProcAddress(module, name));
    return *target != nullptr;
}

std::filesystem::path ExecutableDirectory()
{
    std::wstring path(512, L'\0');
    while (true) {
        const DWORD length = GetModuleFileNameW(
            nullptr, path.data(), static_cast<DWORD>(path.size()));
        if (length == 0) {
            return {};
        }
        if (length < path.size() - 1) {
            path.resize(length);
            return std::filesystem::path(path).parent_path();
        }
        path.resize(path.size() * 2);
    }
}

class FfmpegD3D11Api final {
public:
    static FfmpegD3D11Api& Instance()
    {
        // The modules intentionally remain loaded for the process lifetime.
        // Native frame buffers can outlive a decoder instance and their
        // AVFrame deleter must remain callable until the last displayed frame
        // is released.
        static auto* instance = new FfmpegD3D11Api();
        return *instance;
    }

    bool EnsureLoaded()
    {
        std::call_once(loadOnce_, [this] { Load(); });
        return loaded_;
    }

    const std::string& Error() const noexcept { return error_; }

    decltype(&::avcodec_find_decoder) avcodecFindDecoder = nullptr;
    decltype(&::avcodec_alloc_context3) avcodecAllocContext3 = nullptr;
    decltype(&::avcodec_free_context) avcodecFreeContext = nullptr;
    decltype(&::avcodec_open2) avcodecOpen2 = nullptr;
    decltype(&::avcodec_send_packet) avcodecSendPacket = nullptr;
    decltype(&::avcodec_receive_frame) avcodecReceiveFrame = nullptr;
    decltype(&::av_packet_alloc) avPacketAlloc = nullptr;
    decltype(&::av_packet_free) avPacketFree = nullptr;
    decltype(&::av_packet_unref) avPacketUnref = nullptr;
    decltype(&::av_new_packet) avNewPacket = nullptr;

    decltype(&::av_buffer_ref) avBufferRef = nullptr;
    decltype(&::av_buffer_unref) avBufferUnref = nullptr;
    decltype(&::av_frame_alloc) avFrameAlloc = nullptr;
    decltype(&::av_frame_clone) avFrameClone = nullptr;
    decltype(&::av_frame_free) avFrameFree = nullptr;
    decltype(&::av_frame_unref) avFrameUnref = nullptr;
    decltype(&::av_hwdevice_ctx_create) avHwDeviceCtxCreate = nullptr;
    decltype(&::av_strerror) avStrError = nullptr;

private:
    void Load()
    {
        const auto directory = ExecutableDirectory();
        if (directory.empty()) {
            error_ = "Cannot resolve the RemoteC executable directory.";
            return;
        }
        const auto avutilPath = directory / L"avutil-60.dll";
        const auto avcodecPath = directory / L"avcodec-62.dll";
        avutilModule_ = LoadLibraryExW(
            avutilPath.c_str(), nullptr,
            LOAD_LIBRARY_SEARCH_DLL_LOAD_DIR |
                LOAD_LIBRARY_SEARCH_DEFAULT_DIRS);
        if (!avutilModule_) {
            error_ = "avutil-60.dll is missing or cannot be loaded.";
            return;
        }
        avcodecModule_ = LoadLibraryExW(
            avcodecPath.c_str(), nullptr,
            LOAD_LIBRARY_SEARCH_DLL_LOAD_DIR |
                LOAD_LIBRARY_SEARCH_DEFAULT_DIRS);
        if (!avcodecModule_) {
            error_ = "avcodec-62.dll is missing or cannot be loaded.";
            return;
        }

        const bool codecFunctions =
            ResolveFunction(avcodecModule_, "avcodec_find_decoder",
                            &avcodecFindDecoder) &&
            ResolveFunction(avcodecModule_, "avcodec_alloc_context3",
                            &avcodecAllocContext3) &&
            ResolveFunction(avcodecModule_, "avcodec_free_context",
                            &avcodecFreeContext) &&
            ResolveFunction(avcodecModule_, "avcodec_open2",
                            &avcodecOpen2) &&
            ResolveFunction(avcodecModule_, "avcodec_send_packet",
                            &avcodecSendPacket) &&
            ResolveFunction(avcodecModule_, "avcodec_receive_frame",
                            &avcodecReceiveFrame) &&
            ResolveFunction(avcodecModule_, "av_packet_alloc",
                            &avPacketAlloc) &&
            ResolveFunction(avcodecModule_, "av_packet_free",
                            &avPacketFree) &&
            ResolveFunction(avcodecModule_, "av_packet_unref",
                            &avPacketUnref) &&
            ResolveFunction(avcodecModule_, "av_new_packet", &avNewPacket);
        const bool utilityFunctions =
            ResolveFunction(avutilModule_, "av_buffer_ref", &avBufferRef) &&
            ResolveFunction(avutilModule_, "av_buffer_unref",
                            &avBufferUnref) &&
            ResolveFunction(avutilModule_, "av_frame_alloc", &avFrameAlloc) &&
            ResolveFunction(avutilModule_, "av_frame_clone", &avFrameClone) &&
            ResolveFunction(avutilModule_, "av_frame_free", &avFrameFree) &&
            ResolveFunction(avutilModule_, "av_frame_unref", &avFrameUnref) &&
            ResolveFunction(avutilModule_, "av_hwdevice_ctx_create",
                            &avHwDeviceCtxCreate) &&
            ResolveFunction(avutilModule_, "av_strerror", &avStrError);
        if (!codecFunctions || !utilityFunctions) {
            error_ = "The bundled FFmpeg DLL exports do not match RemoteC.";
            return;
        }
        loaded_ = true;
    }

    std::once_flag loadOnce_;
    HMODULE avcodecModule_ = nullptr;
    HMODULE avutilModule_ = nullptr;
    bool loaded_ = false;
    std::string error_;
};

std::string FfmpegError(FfmpegD3D11Api& api,
                        const char* operation,
                        int result)
{
    char description[AV_ERROR_MAX_STRING_SIZE]{};
    if (!api.avStrError ||
        api.avStrError(result, description, sizeof(description)) < 0) {
        return std::string(operation) + " failed with FFmpeg error " +
               std::to_string(result) + ".";
    }
    return std::string(operation) + " failed: " + description;
}

}  // namespace

class FfmpegD3D11H264Decoder::Impl final {
public:
    bool Configure(const Settings& settings)
    {
        auto* registeredCallback = callback_;
        Release();
        callback_ = registeredCallback;
        lastError_.clear();

        if (settings.codec_type() != webrtc::kVideoCodecH264) {
            lastError_ = "FFmpeg D3D11VA decoder only supports H264.";
            return false;
        }
        auto& api = FfmpegD3D11Api::Instance();
        if (!api.EnsureLoaded()) {
            lastError_ = api.Error();
            return false;
        }
        const AVCodec* decoder = api.avcodecFindDecoder(AV_CODEC_ID_H264);
        if (!decoder) {
            lastError_ = "The bundled FFmpeg has no H264 decoder.";
            return false;
        }
        codecContext_ = api.avcodecAllocContext3(decoder);
        packet_ = api.avPacketAlloc();
        outputFrame_ = api.avFrameAlloc();
        if (!codecContext_ || !packet_ || !outputFrame_) {
            lastError_ = "FFmpeg could not allocate its H264 decoder state.";
            Release();
            return false;
        }

        int result = api.avHwDeviceCtxCreate(
            &hardwareDevice_, AV_HWDEVICE_TYPE_D3D11VA, nullptr, nullptr, 0);
        if (result < 0 || !hardwareDevice_) {
            lastError_ = FfmpegError(
                api, "Create FFmpeg D3D11VA device", result);
            Release();
            return false;
        }
        codecContext_->hw_device_ctx = api.avBufferRef(hardwareDevice_);
        if (!codecContext_->hw_device_ctx) {
            lastError_ = "FFmpeg could not retain the D3D11VA device.";
            Release();
            return false;
        }
        codecContext_->opaque = this;
        codecContext_->get_format = &SelectHardwareFormat;
        codecContext_->flags |= AV_CODEC_FLAG_LOW_DELAY;
        codecContext_->thread_count = 1;
        codecContext_->thread_type = 0;
        codecContext_->pkt_timebase = AVRational{1, 90'000};
        codecContext_->time_base = AVRational{1, 90'000};
        result = api.avcodecOpen2(codecContext_, decoder, nullptr);
        if (result < 0) {
            lastError_ = FfmpegError(api, "Open FFmpeg H264 decoder", result);
            Release();
            return false;
        }
        configured_ = true;
        return true;
    }

    int32_t Decode(const webrtc::EncodedImage& inputImage,
                   int64_t renderTimeMs)
    {
        if (!configured_ || !codecContext_) {
            return WEBRTC_VIDEO_CODEC_UNINITIALIZED;
        }
        if (!callback_ || !inputImage.data() || inputImage.size() == 0 ||
            inputImage.size() > static_cast<std::size_t>(INT_MAX)) {
            return WEBRTC_VIDEO_CODEC_FALLBACK_SOFTWARE;
        }
        if (pendingFrames_.size() >= 8) {
            lastError_ =
                "FFmpeg D3D11VA exceeded the 8-frame low-latency backlog.";
            return WEBRTC_VIDEO_CODEC_FALLBACK_SOFTWARE;
        }

        auto& api = FfmpegD3D11Api::Instance();
        const std::int64_t decodeStartedAtUs =
            VideoCodecTimingRegistry::SteadyNowUs();
        api.avPacketUnref(packet_);
        int result = api.avNewPacket(
            packet_, static_cast<int>(inputImage.size()));
        if (result < 0) {
            lastError_ = FfmpegError(api, "Allocate FFmpeg H264 packet", result);
            return WEBRTC_VIDEO_CODEC_FALLBACK_SOFTWARE;
        }
        std::memcpy(packet_->data, inputImage.data(), inputImage.size());
        const std::int64_t presentationId = nextPresentationId_++;
        packet_->pts = presentationId;
        packet_->dts = presentationId;

        FrameMetadata metadata;
        metadata.presentationId = presentationId;
        metadata.rtpTimestamp = inputImage.RtpTimestamp();
        metadata.renderTimeMs = renderTimeMs;
        metadata.ntpTimeMs = inputImage.NtpTimeMs();
        metadata.rotation = inputImage.rotation();
        metadata.packetInfos = inputImage.PacketInfos();
        if (inputImage.ColorSpace()) {
            metadata.colorSpace = *inputImage.ColorSpace();
        }
        metadata.decodeStartedAtUs = decodeStartedAtUs;
        metadata.inputReadyAtUs =
            VideoCodecTimingRegistry::SteadyNowUs();

        result = api.avcodecSendPacket(codecContext_, packet_);
        if (result == AVERROR(EAGAIN)) {
            if (!DrainOutput()) {
                return WEBRTC_VIDEO_CODEC_FALLBACK_SOFTWARE;
            }
            result = api.avcodecSendPacket(codecContext_, packet_);
        }
        if (result < 0) {
            lastError_ = FfmpegError(api, "Submit FFmpeg H264 packet", result);
            return WEBRTC_VIDEO_CODEC_FALLBACK_SOFTWARE;
        }
        metadata.submittedAtUs =
            VideoCodecTimingRegistry::SteadyNowUs();
        pendingFrames_.push_back(std::move(metadata));
        if (!DrainOutput()) {
            return WEBRTC_VIDEO_CODEC_FALLBACK_SOFTWARE;
        }
        return WEBRTC_VIDEO_CODEC_OK;
    }

    int32_t RegisterDecodeCompleteCallback(
        webrtc::DecodedImageCallback* callback)
    {
        callback_ = callback;
        return WEBRTC_VIDEO_CODEC_OK;
    }

    int32_t Release()
    {
        configured_ = false;
        callback_ = nullptr;
        pendingFrames_.clear();
        nextPresentationId_ = 1;
        auto& api = FfmpegD3D11Api::Instance();
        if (api.EnsureLoaded()) {
            if (packet_) {
                api.avPacketFree(&packet_);
            }
            if (outputFrame_) {
                api.avFrameFree(&outputFrame_);
            }
            if (codecContext_) {
                api.avcodecFreeContext(&codecContext_);
            }
            if (hardwareDevice_) {
                api.avBufferUnref(&hardwareDevice_);
            }
        }
        return WEBRTC_VIDEO_CODEC_OK;
    }

    bool IsConfigured() const noexcept { return configured_; }
    bool SupportsNativeOutputSamples() const noexcept
    {
        return configured_;
    }
    bool IsHardwareAccelerated() const noexcept { return configured_; }
    bool IsAsynchronous() const noexcept { return false; }
    const std::string& LastError() const noexcept { return lastError_; }

private:
    struct FrameMetadata {
        std::int64_t presentationId = AV_NOPTS_VALUE;
        std::uint32_t rtpTimestamp = 0;
        std::int64_t renderTimeMs = 0;
        std::int64_t ntpTimeMs = 0;
        webrtc::VideoRotation rotation = webrtc::kVideoRotation_0;
        std::optional<webrtc::ColorSpace> colorSpace;
        webrtc::RtpPacketInfos packetInfos;
        std::int64_t decodeStartedAtUs = 0;
        std::int64_t inputReadyAtUs = 0;
        std::int64_t submittedAtUs = 0;
    };

    static AVPixelFormat SelectHardwareFormat(
        AVCodecContext* context,
        const AVPixelFormat* formats)
    {
        for (const AVPixelFormat* format = formats;
             format && *format != AV_PIX_FMT_NONE; ++format) {
            if (*format == AV_PIX_FMT_D3D11) {
                return *format;
            }
        }
        if (context && context->opaque) {
            static_cast<Impl*>(context->opaque)->lastError_ =
                "FFmpeg H264 decoder did not offer a D3D11 output format.";
        }
        return AV_PIX_FMT_NONE;
    }

    FrameMetadata TakeMetadata(std::int64_t presentationId)
    {
        if (pendingFrames_.empty()) {
            return {};
        }
        const auto found = std::find_if(
            pendingFrames_.begin(), pendingFrames_.end(),
            [presentationId](const FrameMetadata& candidate) {
                return presentationId != AV_NOPTS_VALUE &&
                       candidate.presentationId == presentationId;
            });
        if (found == pendingFrames_.end()) {
            FrameMetadata result = std::move(pendingFrames_.front());
            pendingFrames_.pop_front();
            return result;
        }
        FrameMetadata result = std::move(*found);
        pendingFrames_.erase(found);
        return result;
    }

    bool DrainOutput()
    {
        auto& api = FfmpegD3D11Api::Instance();
        while (true) {
            const int result =
                api.avcodecReceiveFrame(codecContext_, outputFrame_);
            if (result == AVERROR(EAGAIN) || result == AVERROR_EOF) {
                return true;
            }
            if (result < 0) {
                lastError_ = FfmpegError(
                    api, "Receive FFmpeg D3D11VA frame", result);
                return false;
            }
            if (!DeliverOutputFrame()) {
                api.avFrameUnref(outputFrame_);
                return false;
            }
            api.avFrameUnref(outputFrame_);
        }
    }

    bool DeliverOutputFrame()
    {
        auto& api = FfmpegD3D11Api::Instance();
        if (outputFrame_->format != AV_PIX_FMT_D3D11 ||
            !outputFrame_->data[0]) {
            lastError_ =
                "FFmpeg returned a non-D3D11 H264 output frame.";
            return false;
        }
        AVFrame* retainedFrame = api.avFrameClone(outputFrame_);
        if (!retainedFrame) {
            lastError_ = "FFmpeg could not retain its D3D11 output frame.";
            return false;
        }
        const auto freeFrame = api.avFrameFree;
        std::shared_ptr<void> lifetimeOwner(
            retainedFrame,
            [freeFrame](void* value) {
                AVFrame* frame = static_cast<AVFrame*>(value);
                freeFrame(&frame);
            });
        auto* texture = reinterpret_cast<ID3D11Texture2D*>(
            retainedFrame->data[0]);
        const auto subresourceIndex = static_cast<std::uint32_t>(
            reinterpret_cast<std::uintptr_t>(retainedFrame->data[1]));
        const int visibleWidth = retainedFrame->width > 0
            ? retainedFrame->width
            : codecContext_->width;
        const int visibleHeight = retainedFrame->height > 0
            ? retainedFrame->height
            : codecContext_->height;
        if (!texture || visibleWidth <= 0 || visibleHeight <= 0) {
            lastError_ = "FFmpeg returned an invalid D3D11 output surface.";
            return false;
        }

        const std::int64_t outputReadyAtUs =
            VideoCodecTimingRegistry::SteadyNowUs();
        FrameMetadata metadata = TakeMetadata(
            retainedFrame->pts != AV_NOPTS_VALUE
                ? retainedFrame->pts
                : retainedFrame->best_effort_timestamp);
        auto nativeBuffer =
            webrtc::make_ref_counted<D3D11NativeFrameBuffer>(
                std::move(lifetimeOwner), texture, subresourceIndex,
                visibleWidth, visibleHeight);
        auto frameBuilder = webrtc::VideoFrame::Builder()
            .set_video_frame_buffer(nativeBuffer)
            .set_rtp_timestamp(metadata.rtpTimestamp)
            .set_timestamp_ms(metadata.renderTimeMs)
            .set_ntp_time_ms(metadata.ntpTimeMs)
            .set_rotation(metadata.rotation)
            .set_packet_infos(std::move(metadata.packetInfos));
        if (metadata.colorSpace) {
            frameBuilder.set_color_space(*metadata.colorSpace);
        }
        webrtc::VideoFrame frame = frameBuilder.build();

        const std::int64_t callbackAtUs =
            VideoCodecTimingRegistry::SteadyNowUs();
        if (metadata.decodeStartedAtUs > 0 &&
            metadata.inputReadyAtUs >= metadata.decodeStartedAtUs &&
            metadata.submittedAtUs >= metadata.inputReadyAtUs &&
            outputReadyAtUs >= metadata.submittedAtUs &&
            callbackAtUs >= outputReadyAtUs) {
            VideoDecodePipelineTimingSnapshot timing;
            timing.asynchronous = false;
            timing.inputPreparationUs = static_cast<std::uint64_t>(
                metadata.inputReadyAtUs - metadata.decodeStartedAtUs);
            timing.inputQueueWaitUs = static_cast<std::uint64_t>(
                metadata.submittedAtUs - metadata.inputReadyAtUs);
            timing.transformWaitUs = static_cast<std::uint64_t>(
                outputReadyAtUs - metadata.submittedAtUs);
            timing.outputDeliveryUs = static_cast<std::uint64_t>(
                callbackAtUs - outputReadyAtUs);
            timing.inFlightFrames = static_cast<std::uint32_t>(
                pendingFrames_.size());
            timing.peakBacklogFrames = timing.inFlightFrames;
            VideoCodecTimingRegistry::StageDecodePipelineTiming(
                metadata.rtpTimestamp, timing);
        }
        callback_->Decoded(frame, std::nullopt, std::nullopt);
        return true;
    }

    AVCodecContext* codecContext_ = nullptr;
    AVBufferRef* hardwareDevice_ = nullptr;
    AVPacket* packet_ = nullptr;
    AVFrame* outputFrame_ = nullptr;
    webrtc::DecodedImageCallback* callback_ = nullptr;
    std::deque<FrameMetadata> pendingFrames_;
    std::int64_t nextPresentationId_ = 1;
    bool configured_ = false;
    std::string lastError_;
};

FfmpegD3D11H264Decoder::FfmpegD3D11H264Decoder()
    : impl_(std::make_unique<Impl>())
{}

FfmpegD3D11H264Decoder::~FfmpegD3D11H264Decoder()
{
    Release();
}

bool FfmpegD3D11H264Decoder::Configure(const Settings& settings)
{
    return impl_->Configure(settings);
}

int32_t FfmpegD3D11H264Decoder::Decode(
    const webrtc::EncodedImage& inputImage,
    int64_t renderTimeMs)
{
    return impl_->Decode(inputImage, renderTimeMs);
}

int32_t FfmpegD3D11H264Decoder::RegisterDecodeCompleteCallback(
    webrtc::DecodedImageCallback* callback)
{
    return impl_->RegisterDecodeCompleteCallback(callback);
}

int32_t FfmpegD3D11H264Decoder::Release()
{
    return impl_->Release();
}

webrtc::VideoDecoder::DecoderInfo
FfmpegD3D11H264Decoder::GetDecoderInfo() const
{
    return {
        .implementation_name = ImplementationName(),
        .is_hardware_accelerated = impl_->IsHardwareAccelerated(),
    };
}

const char* FfmpegD3D11H264Decoder::ImplementationName() const
{
    return kFfmpegD3D11H264DecoderName;
}

bool FfmpegD3D11H264Decoder::IsConfigured() const noexcept
{
    return impl_->IsConfigured();
}

bool FfmpegD3D11H264Decoder::SupportsNativeOutputSamples() const noexcept
{
    return impl_->SupportsNativeOutputSamples();
}

bool FfmpegD3D11H264Decoder::IsHardwareAccelerated() const noexcept
{
    return impl_->IsHardwareAccelerated();
}

bool FfmpegD3D11H264Decoder::IsAsynchronous() const noexcept
{
    return impl_->IsAsynchronous();
}

const std::string& FfmpegD3D11H264Decoder::LastError() const noexcept
{
    return impl_->LastError();
}

}  // namespace remote
