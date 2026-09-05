// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#include "FfmpegX264H264Encoder.h"

#include <Windows.h>

#include <algorithm>
#include <cstdint>
#include <cstring>
#include <deque>
#include <filesystem>
#include <mutex>
#include <string>
#include <utility>

extern "C" {
#include "libavcodec/avcodec.h"
#include "libavcodec/packet.h"
#include "libavutil/error.h"
#include "libavutil/frame.h"
#include "libavutil/opt.h"
#include "libavutil/pixfmt.h"
}

#include "api/video/encoded_image.h"
#include "api/video/i420_buffer.h"
#include "api/video_codecs/video_codec.h"
#include "modules/video_coding/codecs/h264/include/h264_globals.h"
#include "modules/video_coding/include/video_codec_interface.h"
#include "modules/video_coding/include/video_error_codes.h"
#include "src/core/ScreenStreamPolicy.h"

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

class FfmpegX264Api final {
public:
    static FfmpegX264Api& Instance()
    {
        // Keep the dynamically loaded modules alive for the entire process.
        static auto* instance = new FfmpegX264Api();
        return *instance;
    }

    bool EnsureLoaded()
    {
        std::call_once(loadOnce_, [this] { Load(); });
        return loaded_;
    }

    const std::string& Error() const noexcept { return error_; }

    decltype(&::avcodec_find_encoder_by_name) avcodecFindEncoderByName =
        nullptr;
    decltype(&::avcodec_alloc_context3) avcodecAllocContext3 = nullptr;
    decltype(&::avcodec_free_context) avcodecFreeContext = nullptr;
    decltype(&::avcodec_open2) avcodecOpen2 = nullptr;
    decltype(&::avcodec_send_frame) avcodecSendFrame = nullptr;
    decltype(&::avcodec_receive_packet) avcodecReceivePacket = nullptr;
    decltype(&::av_packet_alloc) avPacketAlloc = nullptr;
    decltype(&::av_packet_free) avPacketFree = nullptr;
    decltype(&::av_packet_unref) avPacketUnref = nullptr;
    decltype(&::av_frame_alloc) avFrameAlloc = nullptr;
    decltype(&::av_frame_free) avFrameFree = nullptr;
    decltype(&::av_frame_get_buffer) avFrameGetBuffer = nullptr;
    decltype(&::av_frame_make_writable) avFrameMakeWritable = nullptr;
    decltype(&::av_opt_set) avOptSet = nullptr;
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
        const auto x264Path = directory / L"libx264-165.dll";
        const auto avcodecPath = directory / L"avcodec-62.dll";
        avutilModule_ = LoadLibraryExW(
            avutilPath.c_str(), nullptr,
            LOAD_LIBRARY_SEARCH_DLL_LOAD_DIR |
                LOAD_LIBRARY_SEARCH_DEFAULT_DIRS);
        if (!avutilModule_) {
            error_ = "avutil-60.dll is missing or cannot be loaded.";
            return;
        }
        x264Module_ = LoadLibraryExW(
            x264Path.c_str(), nullptr,
            LOAD_LIBRARY_SEARCH_DLL_LOAD_DIR |
                LOAD_LIBRARY_SEARCH_DEFAULT_DIRS);
        if (!x264Module_) {
            error_ = "libx264-165.dll is missing or cannot be loaded.";
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
            ResolveFunction(avcodecModule_, "avcodec_find_encoder_by_name",
                            &avcodecFindEncoderByName) &&
            ResolveFunction(avcodecModule_, "avcodec_alloc_context3",
                            &avcodecAllocContext3) &&
            ResolveFunction(avcodecModule_, "avcodec_free_context",
                            &avcodecFreeContext) &&
            ResolveFunction(avcodecModule_, "avcodec_open2", &avcodecOpen2) &&
            ResolveFunction(avcodecModule_, "avcodec_send_frame",
                            &avcodecSendFrame) &&
            ResolveFunction(avcodecModule_, "avcodec_receive_packet",
                            &avcodecReceivePacket) &&
            ResolveFunction(avcodecModule_, "av_packet_alloc",
                            &avPacketAlloc) &&
            ResolveFunction(avcodecModule_, "av_packet_free", &avPacketFree) &&
            ResolveFunction(avcodecModule_, "av_packet_unref", &avPacketUnref);
        const bool utilityFunctions =
            ResolveFunction(avutilModule_, "av_frame_alloc", &avFrameAlloc) &&
            ResolveFunction(avutilModule_, "av_frame_free", &avFrameFree) &&
            ResolveFunction(avutilModule_, "av_frame_get_buffer",
                            &avFrameGetBuffer) &&
            ResolveFunction(avutilModule_, "av_frame_make_writable",
                            &avFrameMakeWritable) &&
            ResolveFunction(avutilModule_, "av_opt_set", &avOptSet) &&
            ResolveFunction(avutilModule_, "av_strerror", &avStrError);
        if (!codecFunctions || !utilityFunctions) {
            error_ = "The bundled FFmpeg/libx264 DLL exports do not match RemoteC.";
            return;
        }
        if (!avcodecFindEncoderByName("libx264")) {
            error_ = "The bundled FFmpeg runtime does not contain libx264.";
            return;
        }
        loaded_ = true;
    }

    std::once_flag loadOnce_;
    HMODULE avcodecModule_ = nullptr;
    HMODULE avutilModule_ = nullptr;
    HMODULE x264Module_ = nullptr;
    bool loaded_ = false;
    std::string error_;
};

std::string FfmpegError(FfmpegX264Api& api,
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

void CopyPlane(const uint8_t* source,
               int sourceStride,
               uint8_t* destination,
               int destinationStride,
               int rowBytes,
               int rows)
{
    for (int row = 0; row < rows; ++row) {
        std::memcpy(destination, source, rowBytes);
        source += sourceStride;
        destination += destinationStride;
    }
}

}  // namespace

class FfmpegX264H264Encoder::Impl final {
public:
    explicit Impl(FfmpegX264Preset preset) : preset_(preset) {}

    ~Impl() { Release(); }

    const char* PresetName() const
    {
        return VideoEncoderQualityProfileForPreset(preset_).x264Preset;
    }

    int Init(const webrtc::VideoCodec* codecSettings)
    {
        std::lock_guard lock(mutex_);
        ReleaseUnlocked();
        lastError_.clear();
        if (!codecSettings ||
            codecSettings->codecType != webrtc::kVideoCodecH264 ||
            codecSettings->width == 0 || codecSettings->height == 0 ||
            (codecSettings->width & 1) != 0 ||
            (codecSettings->height & 1) != 0) {
            return WEBRTC_VIDEO_CODEC_ERR_PARAMETER;
        }
        if (!codecSettings->IsSinglecast()) {
            return WEBRTC_VIDEO_CODEC_ERR_SIMULCAST_PARAMETERS_NOT_SUPPORTED;
        }

        auto& api = FfmpegX264Api::Instance();
        if (!api.EnsureLoaded()) {
            lastError_ = api.Error();
            return WEBRTC_VIDEO_CODEC_ERROR;
        }
        const AVCodec* encoder =
            api.avcodecFindEncoderByName("libx264");
        codecContext_ = api.avcodecAllocContext3(encoder);
        inputFrame_ = api.avFrameAlloc();
        outputPacket_ = api.avPacketAlloc();
        if (!codecContext_ || !inputFrame_ || !outputPacket_) {
            lastError_ = "FFmpeg could not allocate its libx264 encoder state.";
            ReleaseUnlocked();
            return WEBRTC_VIDEO_CODEC_MEMORY;
        }

        width_ = codecSettings->width;
        height_ = codecSettings->height;
        frameRate_ = (std::max<uint32_t>)(codecSettings->maxFramerate, 1);
        requestedBitrateBps_ = (std::max<uint32_t>)(
            codecSettings->startBitrate * 1000, 100'000);
        maximumBitrateBps_ = (std::max<uint32_t>)(
            codecSettings->maxBitrate * 1000, requestedBitrateBps_);
        startupBitrateFloorBps_ = 0;
        startupKeyFramePending_ =
            codecSettings->mode == webrtc::VideoCodecMode::kScreensharing;
        if (startupKeyFramePending_) {
            const auto screenPolicy = ResolveScreenStreamPolicy(
                width_, height_, {width_, height_, frameRate_});
            startupBitrateFloorBps_ = (std::min)(
                screenPolicy.startBitrateBps, maximumBitrateBps_);
        }
        bitrateBps_ = (std::max)(
            requestedBitrateBps_, startupBitrateFloorBps_);

        codecContext_->codec_type = AVMEDIA_TYPE_VIDEO;
        codecContext_->codec_id = AV_CODEC_ID_H264;
        codecContext_->width = static_cast<int>(width_);
        codecContext_->height = static_cast<int>(height_);
        codecContext_->pix_fmt = AV_PIX_FMT_YUV420P;
        codecContext_->time_base = AVRational{1, static_cast<int>(frameRate_)};
        codecContext_->framerate = AVRational{static_cast<int>(frameRate_), 1};
        codecContext_->bit_rate = bitrateBps_;
        codecContext_->rc_max_rate = maximumBitrateBps_;
        codecContext_->rc_buffer_size =
            static_cast<int>((std::min<uint64_t>)(
                maximumBitrateBps_, static_cast<std::uint64_t>(INT_MAX)));
        codecContext_->gop_size = static_cast<int>(frameRate_ * 2);
        codecContext_->max_b_frames = 0;
        codecContext_->flags |= AV_CODEC_FLAG_LOW_DELAY;
        codecContext_->thread_count = 0;
        codecContext_->thread_type = FF_THREAD_SLICE;

        // These options make libx264 emit one low-latency Annex-B access unit
        // for each submitted WebRTC frame and repeat SPS/PPS on keyframes.
        // WebRTC remains the bandwidth controller and supplies the current
        // allocation through SetRates. The user-selected preset controls the
        // CPU/quality tradeoff. Zerolatency and Baseline are kept
        // for one-frame delivery and negotiated H264 compatibility; neither
        // one replaces WebRTC's rate controller.
        api.avOptSet(codecContext_->priv_data, "preset", PresetName(), 0);
        api.avOptSet(codecContext_->priv_data, "tune", "zerolatency", 0);
        api.avOptSet(codecContext_->priv_data, "profile", "baseline", 0);
        api.avOptSet(
            codecContext_->priv_data, "x264-params",
            "annexb=1:repeat-headers=1:scenecut=0:bframes=0:sliced-threads=1",
            0);

        int result = api.avcodecOpen2(codecContext_, encoder, nullptr);
        if (result < 0) {
            lastError_ = FfmpegError(api, "Open FFmpeg libx264 encoder", result);
            ReleaseUnlocked();
            return WEBRTC_VIDEO_CODEC_ERROR;
        }
        inputFrame_->format = AV_PIX_FMT_YUV420P;
        inputFrame_->width = static_cast<int>(width_);
        inputFrame_->height = static_cast<int>(height_);
        result = api.avFrameGetBuffer(inputFrame_, 32);
        if (result < 0) {
            lastError_ = FfmpegError(api, "Allocate libx264 input frame", result);
            ReleaseUnlocked();
            return WEBRTC_VIDEO_CODEC_MEMORY;
        }
        initialized_ = true;
        return WEBRTC_VIDEO_CODEC_OK;
    }

    int RegisterCallback(webrtc::EncodedImageCallback* callback)
    {
        std::lock_guard lock(mutex_);
        callback_ = callback;
        return WEBRTC_VIDEO_CODEC_OK;
    }

    int Release()
    {
        std::lock_guard lock(mutex_);
        ReleaseUnlocked();
        return WEBRTC_VIDEO_CODEC_OK;
    }

    int Encode(const webrtc::VideoFrame& frame,
               const std::vector<webrtc::VideoFrameType>* frameTypes)
    {
        std::lock_guard lock(mutex_);
        if (!initialized_ || !codecContext_ || !callback_) {
            return WEBRTC_VIDEO_CODEC_UNINITIALIZED;
        }
        const auto source = frame.video_frame_buffer();
        if (!source || source->width() != static_cast<int>(width_) ||
            source->height() != static_cast<int>(height_)) {
            return WEBRTC_VIDEO_CODEC_ERR_PARAMETER;
        }
        const auto i420 = source->ToI420();
        if (!i420) {
            lastError_ = "The input frame cannot be converted to CPU I420.";
            return WEBRTC_VIDEO_CODEC_ERROR;
        }

        auto& api = FfmpegX264Api::Instance();
        int result = api.avFrameMakeWritable(inputFrame_);
        if (result < 0) {
            lastError_ = FfmpegError(api, "Make libx264 input writable", result);
            return WEBRTC_VIDEO_CODEC_ERROR;
        }
        CopyPlane(i420->DataY(), i420->StrideY(),
                  inputFrame_->data[0], inputFrame_->linesize[0],
                  static_cast<int>(width_), static_cast<int>(height_));
        CopyPlane(i420->DataU(), i420->StrideU(),
                  inputFrame_->data[1], inputFrame_->linesize[1],
                  static_cast<int>(width_ / 2), static_cast<int>(height_ / 2));
        CopyPlane(i420->DataV(), i420->StrideV(),
                  inputFrame_->data[2], inputFrame_->linesize[2],
                  static_cast<int>(width_ / 2), static_cast<int>(height_ / 2));

        const bool requestedKeyFrame =
            frameTypes &&
            std::find(frameTypes->begin(), frameTypes->end(),
                      webrtc::VideoFrameType::kVideoFrameKey) !=
                frameTypes->end();
        const bool rateRecoveryKeyFrame =
            forceKeyFrameForRateIncrease_;
        const bool startupKeyFrame = startupKeyFramePending_;
        const bool keyFrame = requestedKeyFrame || rateRecoveryKeyFrame ||
                              startupKeyFrame;
        const std::int64_t presentationId = nextPresentationId_++;
        inputFrame_->pts = presentationId;
        inputFrame_->pict_type = keyFrame
                                    ? AV_PICTURE_TYPE_I
                                    : AV_PICTURE_TYPE_NONE;

        if (pendingFrames_.size() >= 8) {
            lastError_ = "FFmpeg/libx264 exceeded the 8-frame low-latency backlog.";
            return WEBRTC_VIDEO_CODEC_ERROR;
        }
        pendingFrames_.push_back(FrameMetadata{presentationId, frame});
        result = api.avcodecSendFrame(codecContext_, inputFrame_);
        if (result == AVERROR(EAGAIN)) {
            if (!DrainOutput()) {
                pendingFrames_.pop_back();
                return WEBRTC_VIDEO_CODEC_ERROR;
            }
            result = api.avcodecSendFrame(codecContext_, inputFrame_);
        }
        if (result < 0) {
            pendingFrames_.pop_back();
            lastError_ = FfmpegError(api, "Submit frame to libx264", result);
            return WEBRTC_VIDEO_CODEC_ERROR;
        }
        if (rateRecoveryKeyFrame) {
            forceKeyFrameForRateIncrease_ = false;
            lastRateRecoveryKeyFramePresentationId_ = presentationId;
        }
        if (startupKeyFrame) {
            // The startup floor protects only the first desktop reference
            // frame. Once it has been submitted, immediately return to the
            // latest WebRTC allocation; this is not a persistent minimum.
            startupKeyFramePending_ = false;
            if (requestedBitrateBps_ != 0 &&
                requestedBitrateBps_ != bitrateBps_) {
                bitrateBps_ = requestedBitrateBps_;
                codecContext_->bit_rate = bitrateBps_;
            }
        }
        return DrainOutput()
                   ? WEBRTC_VIDEO_CODEC_OK
                   : WEBRTC_VIDEO_CODEC_ERROR;
    }

    void SetRates(const webrtc::VideoEncoder::RateControlParameters& parameters)
    {
        std::lock_guard lock(mutex_);
        const uint32_t bitrate = parameters.bitrate.get_sum_bps();
        if (bitrate != 0) {
            const std::uint32_t previousBitrateBps = bitrateBps_;
            requestedBitrateBps_ =
                (std::min)(bitrate, maximumBitrateBps_);
            const std::uint32_t nextBitrateBps = startupKeyFramePending_
                ? (std::max)(requestedBitrateBps_, startupBitrateFloorBps_)
                : requestedBitrateBps_;
            // A first-share BWE ramp can begin with a very small allocation.
            // If the initial IDR is encoded there, later skip-heavy desktop
            // frames keep referencing that blurry picture even after WebRTC
            // raises the target. Refresh the reference picture on a material
            // recovery instead of requiring the user to stop and re-share.
            if (initialized_ && previousBitrateBps != 0 &&
                static_cast<std::uint64_t>(nextBitrateBps) * 4 >=
                    static_cast<std::uint64_t>(previousBitrateBps) * 5 &&
                (lastRateRecoveryKeyFramePresentationId_ == 0 ||
                 nextPresentationId_ >=
                     lastRateRecoveryKeyFramePresentationId_ +
                         (std::max<std::uint32_t>)(frameRate_, 1))) {
                forceKeyFrameForRateIncrease_ = true;
            }
            bitrateBps_ = nextBitrateBps;
            if (codecContext_) {
                // FFmpeg's libx264 wrapper observes AVCodecContext::bit_rate
                // before every frame and reconfigures x264 when it changes.
                // WebRTC therefore owns the changing ABR target.  VBV stays
                // at the application's maximum as a safety ceiling: FFmpeg
                // applies VBV before ABR during reconfiguration, so lowering
                // both at once can transiently place the peak below the old
                // average and make x264 switch to CBR.
                codecContext_->bit_rate = bitrateBps_;
            }
        }
        if (parameters.framerate_fps > 0.0) {
            frameRate_ = static_cast<uint32_t>((std::max)(
                1.0, parameters.framerate_fps));
        }
    }

private:
    struct FrameMetadata {
        std::int64_t presentationId = 0;
        webrtc::VideoFrame frame;
    };

    bool DrainOutput()
    {
        auto& api = FfmpegX264Api::Instance();
        while (true) {
            api.avPacketUnref(outputPacket_);
            const int result =
                api.avcodecReceivePacket(codecContext_, outputPacket_);
            if (result == AVERROR(EAGAIN) || result == AVERROR_EOF) {
                return true;
            }
            if (result < 0) {
                lastError_ =
                    FfmpegError(api, "Receive packet from libx264", result);
                return false;
            }
            auto metadata = pendingFrames_.begin();
            if (outputPacket_->pts != AV_NOPTS_VALUE) {
                metadata = std::find_if(
                    pendingFrames_.begin(), pendingFrames_.end(),
                    [this](const FrameMetadata& candidate) {
                        return candidate.presentationId == outputPacket_->pts;
                    });
            }
            if (metadata == pendingFrames_.end() && !pendingFrames_.empty()) {
                metadata = pendingFrames_.begin();
            }
            if (metadata == pendingFrames_.end()) {
                lastError_ = "libx264 returned a packet without input metadata.";
                return false;
            }

            webrtc::EncodedImage image;
            image.SetEncodedData(webrtc::EncodedImageBuffer::Create(
                outputPacket_->data,
                static_cast<std::size_t>(outputPacket_->size)));
            image.SetRtpTimestamp(metadata->frame.rtp_timestamp());
            image.capture_time_ms_ = metadata->frame.timestamp_us() / 1000;
            image._encodedWidth = width_;
            image._encodedHeight = height_;
            image.rotation_ = metadata->frame.rotation();
            const bool keyFrame =
                (outputPacket_->flags & AV_PKT_FLAG_KEY) != 0;
            image.SetFrameType(
                keyFrame ? webrtc::VideoFrameType::kVideoFrameKey
                         : webrtc::VideoFrameType::kVideoFrameDelta);

            webrtc::CodecSpecificInfo codecInfo;
            codecInfo.codecType = webrtc::kVideoCodecH264;
            codecInfo.codecSpecific.H264.packetization_mode =
                webrtc::H264PacketizationMode::NonInterleaved;
            codecInfo.codecSpecific.H264.temporal_idx =
                webrtc::kNoTemporalIdx;
            codecInfo.codecSpecific.H264.idr_frame = keyFrame;
            codecInfo.codecSpecific.H264.base_layer_sync = false;
            callback_->OnEncodedImage(image, &codecInfo);
            pendingFrames_.erase(metadata);
        }
    }

    void ReleaseUnlocked()
    {
        auto& api = FfmpegX264Api::Instance();
        if (outputPacket_ && api.avPacketFree) {
            api.avPacketFree(&outputPacket_);
        }
        if (inputFrame_ && api.avFrameFree) {
            api.avFrameFree(&inputFrame_);
        }
        if (codecContext_ && api.avcodecFreeContext) {
            api.avcodecFreeContext(&codecContext_);
        }
        pendingFrames_.clear();
        forceKeyFrameForRateIncrease_ = false;
        startupKeyFramePending_ = false;
        startupBitrateFloorBps_ = 0;
        requestedBitrateBps_ = 0;
        initialized_ = false;
        width_ = 0;
        height_ = 0;
        nextPresentationId_ = 1;
        lastRateRecoveryKeyFramePresentationId_ = 0;
    }

    std::mutex mutex_;
    AVCodecContext* codecContext_ = nullptr;
    AVFrame* inputFrame_ = nullptr;
    AVPacket* outputPacket_ = nullptr;
    webrtc::EncodedImageCallback* callback_ = nullptr;
    std::deque<FrameMetadata> pendingFrames_;
    std::uint32_t width_ = 0;
    std::uint32_t height_ = 0;
    std::uint32_t frameRate_ = 30;
    std::uint32_t bitrateBps_ = 1'000'000;
    std::uint32_t requestedBitrateBps_ = 0;
    std::uint32_t startupBitrateFloorBps_ = 0;
    std::uint32_t maximumBitrateBps_ = 1'000'000;
    std::int64_t nextPresentationId_ = 1;
    std::int64_t lastRateRecoveryKeyFramePresentationId_ = 0;
    FfmpegX264Preset preset_ = FfmpegX264Preset::kMedium;
    bool forceKeyFrameForRateIncrease_ = false;
    bool startupKeyFramePending_ = false;
    bool initialized_ = false;
    std::string lastError_;
};

FfmpegX264H264Encoder::FfmpegX264H264Encoder(FfmpegX264Preset preset)
    : impl_(std::make_unique<Impl>(preset))
{}

FfmpegX264H264Encoder::~FfmpegX264H264Encoder() = default;

bool FfmpegX264H264Encoder::RuntimeAvailable(std::string* error)
{
    auto& api = FfmpegX264Api::Instance();
    const bool available = api.EnsureLoaded();
    if (error) {
        *error = available ? std::string{} : api.Error();
    }
    return available;
}

int FfmpegX264H264Encoder::InitEncode(
    const webrtc::VideoCodec* codecSettings,
    const Settings& /* settings */)
{
    return impl_->Init(codecSettings);
}

int32_t FfmpegX264H264Encoder::RegisterEncodeCompleteCallback(
    webrtc::EncodedImageCallback* callback)
{
    return impl_->RegisterCallback(callback);
}

int32_t FfmpegX264H264Encoder::Release()
{
    return impl_->Release();
}

int32_t FfmpegX264H264Encoder::Encode(
    const webrtc::VideoFrame& frame,
    const std::vector<webrtc::VideoFrameType>* frameTypes)
{
    return impl_->Encode(frame, frameTypes);
}

void FfmpegX264H264Encoder::SetRates(
    const RateControlParameters& parameters)
{
    impl_->SetRates(parameters);
}

webrtc::VideoEncoder::EncoderInfo
FfmpegX264H264Encoder::GetEncoderInfo() const
{
    EncoderInfo info;
    info.supports_native_handle = false;
    info.implementation_name =
        std::string("FFmpeg/libx264 (") + impl_->PresetName() + ")";
    info.has_trusted_rate_controller = false;
    info.is_hardware_accelerated = false;
    info.enable_cpu_overuse_detection = true;
    info.supports_simulcast = false;
    info.requested_resolution_alignment = 2;
    info.apply_alignment_to_all_simulcast_layers = true;
    info.preferred_pixel_formats.push_back(
        webrtc::VideoFrameBuffer::Type::kI420);
    info.scaling_settings = ScalingSettings::kOff;
    return info;
}

FfmpegX264EncoderSelfTestResult RunFfmpegX264EncoderSelfTest()
{
    class Callback final : public webrtc::EncodedImageCallback {
    public:
        Result OnEncodedImage(
            const webrtc::EncodedImage& image,
            const webrtc::CodecSpecificInfo* codecInfo) override
        {
            if (!codecInfo ||
                codecInfo->codecType != webrtc::kVideoCodecH264 ||
                !image.data() || image.size() < 4) {
                invalidOutput = true;
                return Result(Result::ERROR_SEND_FAILED);
            }
            bool annexB = false;
            for (std::size_t index = 0;
                 index + 3 < image.size() && index < 64; ++index) {
                if (image.data()[index] == 0 &&
                    image.data()[index + 1] == 0 &&
                    (image.data()[index + 2] == 1 ||
                     (image.data()[index + 2] == 0 &&
                      image.data()[index + 3] == 1))) {
                    annexB = true;
                    break;
                }
            }
            invalidOutput |= !annexB;
            if (image._frameType ==
                webrtc::VideoFrameType::kVideoFrameKey) {
                ++keyFrames;
            }
            ++frames;
            bytes += image.size();
            return Result(Result::OK);
        }

        void OnFrameDropped(
            uint32_t /* rtpTimestamp */,
            int /* spatialId */,
            bool /* isEndOfTemporalUnit */) override
        {
            invalidOutput = true;
        }

        std::uint32_t frames = 0;
        std::uint32_t keyFrames = 0;
        std::size_t bytes = 0;
        bool invalidOutput = false;
    } callback;

    FfmpegX264EncoderSelfTestResult test;
    if (!FfmpegX264H264Encoder::RuntimeAvailable(&test.error)) {
        return test;
    }
    constexpr int kWidth = 320;
    constexpr int kHeight = 180;
    FfmpegX264H264Encoder encoder;
    webrtc::VideoCodec codec{};
    codec.codecType = webrtc::kVideoCodecH264;
    codec.width = kWidth;
    codec.height = kHeight;
    codec.maxFramerate = 30;
    codec.minBitrate = 100;
    codec.startBitrate = 500;
    codec.maxBitrate = 1'000;
    codec.mode = webrtc::VideoCodecMode::kScreensharing;
    codec.SetFrameDropEnabled(true);
    const webrtc::VideoEncoder::Capabilities capabilities(false);
    const webrtc::VideoEncoder::Settings settings(
        capabilities, 4, 1200);
    int result = encoder.InitEncode(&codec, settings);
    if (result != WEBRTC_VIDEO_CODEC_OK) {
        test.error = "FFmpeg/libx264 InitEncode returned " +
                     std::to_string(result) + ".";
        return test;
    }
    encoder.RegisterEncodeCompleteCallback(&callback);
    webrtc::VideoBitrateAllocation initialAllocation;
    initialAllocation.SetBitrate(0, 0, 200'000);
    encoder.SetRates(webrtc::VideoEncoder::RateControlParameters(
        initialAllocation, 30.0));
    auto buffer = webrtc::I420Buffer::Create(kWidth, kHeight);
    std::memset(buffer->MutableDataY(), 48,
                static_cast<std::size_t>(buffer->StrideY()) * kHeight);
    std::memset(buffer->MutableDataU(), 112,
                static_cast<std::size_t>(buffer->StrideU()) * (kHeight / 2));
    std::memset(buffer->MutableDataV(), 144,
                static_cast<std::size_t>(buffer->StrideV()) * (kHeight / 2));
    for (std::uint32_t index = 0; index < 3; ++index) {
        if (index == 1) {
            webrtc::VideoBitrateAllocation recoveredAllocation;
            recoveredAllocation.SetBitrate(0, 0, 800'000);
            encoder.SetRates(webrtc::VideoEncoder::RateControlParameters(
                recoveredAllocation, 30.0));
        }
        const auto frame = webrtc::VideoFrame::Builder()
                               .set_video_frame_buffer(buffer)
                               .set_timestamp_us(index * 33'333)
                               .set_rtp_timestamp(index * 3'000)
                               .build();
        const std::vector<webrtc::VideoFrameType> frameTypes{
            index == 0 ? webrtc::VideoFrameType::kVideoFrameKey
                       : webrtc::VideoFrameType::kVideoFrameDelta};
        result = encoder.Encode(frame, &frameTypes);
        if (result != WEBRTC_VIDEO_CODEC_OK) {
            test.error = "FFmpeg/libx264 Encode returned " +
                         std::to_string(result) + ".";
            encoder.Release();
            return test;
        }
    }
    encoder.Release();
    test.encodedFrames = callback.frames;
    test.encodedBytes = callback.bytes;
    test.passed = callback.frames == 3 && callback.keyFrames >= 2 &&
                  callback.bytes != 0 && !callback.invalidOutput;
    if (!test.passed) {
        test.error =
            "FFmpeg/libx264 did not emit valid Annex-B frames and a "
            "rate-recovery keyframe.";
    }
    return test;
}

}  // namespace remote
