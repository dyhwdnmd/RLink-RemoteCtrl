// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#include "MfD3D11H264Encoder.h"

#include <algorithm>
#include <array>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <iterator>
#include <limits>
#include <mutex>
#include <optional>
#include <string>
#include <thread>
#include <vector>

#include <Windows.h>
#include <codecapi.h>
#include <d3d11.h>
#include <mfapi.h>
#include <mferror.h>
#include <mfidl.h>
#include <mftransform.h>
#include <strmif.h>
#include <wrl/client.h>

#include "api/video/encoded_image.h"
#include "modules/video_coding/codecs/h264/include/h264_globals.h"
#include "modules/video_coding/include/video_codec_interface.h"
#include "modules/video_coding/include/video_error_codes.h"
#include "libyuv/convert_argb.h"
#include "rtc_base/time_utils.h"
#include "D3D11DesktopFrameBuffer.h"
#include "DesktopBgraFrameBuffer.h"

namespace remote {
namespace {

using Microsoft::WRL::ComPtr;

constexpr auto kEventTimeout = std::chrono::milliseconds(750);

uint32_t SupportedFrameRateForObservedInput(double framesPerSecond)
{
    constexpr std::array<uint32_t, 8> kSupportedFrameRates{
        15, 24, 30, 45, 60, 80, 100, 120};
    // Capture/encode throughput normally lands slightly below the requested
    // tier (for example 108-112 FPS for a 120 FPS request). Choosing the
    // numerically nearest tier would incorrectly declare that stream as
    // 100 FPS. Pick the highest tier for which at least 84% was observed.
    // This maps 29->30, 57->60, 93->100 and 108->120 without turning a small
    // timer overshoot such as 31 FPS into the 45 FPS tier.
    for (auto current = kSupportedFrameRates.rbegin();
         current != kSupportedFrameRates.rend(); ++current) {
        if (framesPerSecond >=
            static_cast<double>(*current) * 0.84) {
            return *current;
        }
    }
    return kSupportedFrameRates.front();
}

bool ReadBooleanAttribute(IMFAttributes* attributes, REFGUID key)
{
    UINT32 value = FALSE;
    return attributes &&
           SUCCEEDED(attributes->GetUINT32(key, &value)) &&
           value != FALSE;
}

ComPtr<IMFMediaType> CreateVideoType(REFGUID subtype,
                                     uint32_t width,
                                     uint32_t height,
                                     uint32_t frameRate)
{
    ComPtr<IMFMediaType> type;
    if (FAILED(MFCreateMediaType(&type)) ||
        FAILED(type->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Video)) ||
        FAILED(type->SetGUID(MF_MT_SUBTYPE, subtype)) ||
        FAILED(MFSetAttributeSize(
            type.Get(), MF_MT_FRAME_SIZE, width, height)) ||
        FAILED(MFSetAttributeRatio(
            type.Get(), MF_MT_FRAME_RATE, frameRate, 1)) ||
        FAILED(MFSetAttributeRatio(
            type.Get(), MF_MT_PIXEL_ASPECT_RATIO, 1, 1)) ||
        FAILED(type->SetUINT32(
            MF_MT_INTERLACE_MODE, MFVideoInterlace_Progressive)) ||
        // Desktop RGB is converted to studio-range NV12 before it reaches
        // the MFT. Declare that range explicitly on both the NV12 input and
        // H264 output so a driver cannot tag full-range samples as limited
        // range and clip near-white UI detail at the receiver.
        FAILED(type->SetUINT32(
            MF_MT_VIDEO_NOMINAL_RANGE, MFNominalRange_16_235)) ||
        FAILED(type->SetUINT32(
            MF_MT_YUV_MATRIX, MFVideoTransferMatrix_BT709)) ||
        FAILED(type->SetUINT32(
            MF_MT_VIDEO_PRIMARIES, MFVideoPrimaries_BT709)) ||
        FAILED(type->SetUINT32(
            MF_MT_TRANSFER_FUNCTION, MFVideoTransFunc_709))) {
        return nullptr;
    }
    return type;
}

bool IsAnnexB(const uint8_t* bytes, size_t length)
{
    if (!bytes || length < 4) {
        return false;
    }
    for (size_t i = 0; i + 3 < length && i < 64; ++i) {
        if (bytes[i] == 0 && bytes[i + 1] == 0 &&
            (bytes[i + 2] == 1 ||
             (bytes[i + 2] == 0 && bytes[i + 3] == 1))) {
            return true;
        }
    }
    return false;
}

std::uint32_t MfQualityVsSpeed(FfmpegX264Preset quality)
{
    return VideoEncoderQualityProfileForPreset(quality).mfQualityVsSpeed;
}

}  // namespace

class MfD3D11H264Encoder::Impl {
public:
    explicit Impl(
        std::shared_ptr<VideoEncoderRuntimeState> runtimeState = nullptr,
        FfmpegX264Preset quality = FfmpegX264Preset::kMedium)
        : runtimeState_(std::move(runtimeState)), quality_(quality)
    {
        if (runtimeState_) {
            runtimeInstanceId_ =
                runtimeState_->RegisterHardwareEncoder();
        }
    }

    ~Impl()
    {
        Release();
        if (runtimeState_ && runtimeInstanceId_ != 0) {
            runtimeState_->UnregisterEncoder(runtimeInstanceId_);
        }
    }

    std::uint32_t QualityVsSpeed() const
    {
        return MfQualityVsSpeed(quality_);
    }

    int Init(const webrtc::VideoCodec* codecSettings)
    {
        std::lock_guard lock(mutex_);
        ReleaseUnlocked();
        if (!codecSettings ||
            codecSettings->codecType != webrtc::kVideoCodecH264 ||
            codecSettings->width == 0 || codecSettings->height == 0) {
            return WEBRTC_VIDEO_CODEC_ERR_PARAMETER;
        }
        if (!codecSettings->IsSinglecast()) {
            return WEBRTC_VIDEO_CODEC_ERR_SIMULCAST_PARAMETERS_NOT_SUPPORTED;
        }
        if ((codecSettings->width & 1) != 0 ||
            (codecSettings->height & 1) != 0) {
            return WEBRTC_VIDEO_CODEC_ERR_PARAMETER;
        }

        if (runtimeState_) {
            if (runtimeInstanceId_ != 0) {
                runtimeState_->UnregisterEncoder(runtimeInstanceId_);
            }
            runtimeInstanceId_ = runtimeState_->RegisterHardwareEncoder();
            desktopShareGeneration_ =
                runtimeState_->DesktopShareGeneration();
        }
        lastCodecSettings_ = *codecSettings;

        width_ = codecSettings->width;
        height_ = codecSettings->height;
        frameRate_ = (std::max<uint32_t>)(codecSettings->maxFramerate, 1);
        configuredFrameRate_ = frameRate_;
        bitrateBps_ =
            (std::max<uint32_t>)(codecSettings->startBitrate * 1000, 100'000);
        frameDuration100ns_ = 10'000'000LL / frameRate_;

        const HRESULT comResult =
            CoInitializeEx(nullptr, COINIT_MULTITHREADED);
        comInitialized_ = SUCCEEDED(comResult);
        if (FAILED(comResult) && comResult != RPC_E_CHANGED_MODE) {
            return RequestSoftwareFallback(
                "硬件编码器初始化 COM 失败。");
        }
        if (FAILED(MFStartup(MF_VERSION, MFSTARTUP_FULL))) {
            ReleaseUnlocked();
            return RequestSoftwareFallback(
                "Media Foundation 启动失败。");
        }
        mfStarted_ = true;

        if (!InitializeD3D()) {
            ReleaseUnlocked();
            return RequestSoftwareFallback(
                "D3D11 视频设备初始化失败。");
        }
        if (!InitializeTransform()) {
            ReleaseUnlocked();
            return RequestSoftwareFallback(
                "没有可成功初始化的 Media Foundation H264 编码器。");
        }
        if (FAILED(transform_->ProcessMessage(
                MFT_MESSAGE_NOTIFY_BEGIN_STREAMING, 0)) ||
            FAILED(transform_->ProcessMessage(
                MFT_MESSAGE_NOTIFY_START_OF_STREAM, 0))) {
            ReleaseUnlocked();
            return RequestSoftwareFallback(
                "Media Foundation H264 编码器拒绝启动码流。");
        }
        initialized_ = true;
        if (runtimeState_) {
            runtimeState_->MarkHardwareInitialized(
                runtimeInstanceId_, width_, height_, frameRate_,
                codecSettings->minBitrate * 1000,
                codecSettings->startBitrate * 1000,
                codecSettings->maxBitrate * 1000);
            runtimeState_->MarkRates(
                runtimeInstanceId_, frameRate_, bitrateBps_,
                configuredFrameRate_, observedInputFrameRate_,
                configuredBitrateBps_,
                bitrateConfigurationAccepted_,
                bitrateReadbackAvailable_);
        }
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
        std::unique_lock lock(mutex_);
        if (!initialized_ || !transform_ || !callback_) {
            return WEBRTC_VIDEO_CODEC_UNINITIALIZED;
        }
        if (rateReconfigurationFailed_) {
            return RequestSoftwareFallback(
                "硬件编码器无法应用新的动态帧率，已回退软件编码。");
        }
        const auto buffer = frame.video_frame_buffer();
        if (!buffer ||
            buffer->width() !=
                static_cast<int>(width_) ||
            buffer->height() !=
                static_cast<int>(height_)) {
            return RequestSoftwareFallback(
                "输入帧尺寸与已初始化的硬件编码器不一致。");
        }

        const bool desktopFrame =
            buffer->type() == webrtc::VideoFrameBuffer::Type::kNative &&
            (buffer->storage_representation() == "D3D11_BGRA_DESKTOP" ||
             buffer->storage_representation() == "CPU_BGRA_DESKTOP");
        if (desktopFrame && runtimeState_ && lastCodecSettings_) {
            const auto currentGeneration =
                runtimeState_->DesktopShareGeneration();
            if (currentGeneration != desktopShareGeneration_) {
                auto settings = *lastCodecSettings_;
                settings.maxFramerate = frameRate_;
                settings.startBitrate =
                    (std::max<std::uint32_t>)(bitrateBps_ / 1000, 100);
                settings.maxBitrate = (std::max)(
                    settings.maxBitrate, settings.startBitrate);
                lock.unlock();
                const int resetResult = Init(&settings);
                if (resetResult != WEBRTC_VIDEO_CODEC_OK) {
                    return resetResult;
                }
                lock.lock();
                if (!initialized_ || !transform_ || !callback_) {
                    return WEBRTC_VIDEO_CODEC_UNINITIALIZED;
                }
            }
        }

        ReportInputFormat(buffer);
        ++totalInputFrames_;
        ObserveInputFrameRate();
        if (rateReconfigurationFailed_) {
            return RequestSoftwareFallback(
                "硬件编码器无法应用实际输入帧率，已回退软件编码。");
        }

        const bool keyFrame =
            frameTypes &&
            std::find(frameTypes->begin(), frameTypes->end(),
                      webrtc::VideoFrameType::kVideoFrameKey) !=
                frameTypes->end();
        if (keyFrame) {
            SetCodecApiUInt32(CODECAPI_AVEncVideoForceKeyFrame, 1);
        }

        if (!WaitForNeedInput()) {
            return RequestSoftwareFallback(
                "Media Foundation 编码器等待输入请求超时。");
        }
        const LONGLONG sampleTime =
            static_cast<LONGLONG>(frame.timestamp_us()) * 10;
        auto sample = CreateInputSample(frame, sampleTime);
        if (!sample ||
            FAILED(transform_->ProcessInput(0, sample.Get(), 0))) {
            return RequestSoftwareFallback(
                "Media Foundation 编码器拒绝输入帧。");
        }
        pendingFrame_ = frame;

        if (!WaitForOutput()) {
            return RequestSoftwareFallback(
                "Media Foundation 编码器等待输出超时。");
        }
        return WEBRTC_VIDEO_CODEC_OK;
    }

    void SetRates(const webrtc::VideoEncoder::RateControlParameters& parameters)
    {
        std::lock_guard lock(mutex_);
        const uint32_t bitrate = parameters.bitrate.get_sum_bps();
        if (bitrate != 0) {
            bitrateBps_ = bitrate;
            ApplyBitrate(bitrateBps_);
        }
        if (parameters.framerate_fps > 0.0) {
            const uint32_t requestedFrameRate = static_cast<uint32_t>(
                (std::max<long>)(1L, std::lround(
                    parameters.framerate_fps)));
            frameRate_ = requestedFrameRate;
            // Apply an explicit upward sender policy immediately. If an MFT
            // left at 60 FPS is allowed to infer its rate only from Encode()
            // throughput, its own ~16 ms synchronous output wait prevents it
            // from ever observing enough input to return to 120 FPS.
            const uint32_t configuredTarget =
                SupportedFrameRateForObservedInput(requestedFrameRate);
            if (initialized_ && transform_ &&
                configuredTarget > configuredFrameRate_ &&
                !ReconfigureFrameRate(configuredTarget)) {
                rateReconfigurationFailed_ = true;
            }
        }
        if (runtimeState_) {
            runtimeState_->MarkRates(
                runtimeInstanceId_, frameRate_, bitrateBps_,
                configuredFrameRate_, observedInputFrameRate_,
                configuredBitrateBps_,
                bitrateConfigurationAccepted_,
                bitrateReadbackAvailable_);
        }
    }

private:
    int RequestSoftwareFallback(const char* reason)
    {
        if (runtimeState_) {
            runtimeState_->MarkSoftwareFallback(
                runtimeInstanceId_, reason ? reason : "未知硬件编码错误。");
        }
        return WEBRTC_VIDEO_CODEC_FALLBACK_SOFTWARE;
    }

    void ReportInputFormat(
        const webrtc::scoped_refptr<webrtc::VideoFrameBuffer>& buffer)
    {
        if (!runtimeState_ || !buffer) {
            return;
        }
        std::string inputFormat;
        if (buffer->type() ==
                webrtc::VideoFrameBuffer::Type::kNative &&
            buffer->storage_representation() ==
                "D3D11_BGRA_DESKTOP") {
            inputFormat = "D3D11 BGRA desktop texture";
        } else if (buffer->type() ==
                webrtc::VideoFrameBuffer::Type::kNative &&
            buffer->storage_representation() ==
                "CPU_BGRA_DESKTOP") {
            inputFormat = "CPU BGRA desktop";
        } else {
            switch (buffer->type()) {
            case webrtc::VideoFrameBuffer::Type::kI420:
                inputFormat = "CPU I420";
                break;
            case webrtc::VideoFrameBuffer::Type::kI420A:
                inputFormat = "CPU I420A";
                break;
            case webrtc::VideoFrameBuffer::Type::kI422:
                inputFormat = "CPU I422";
                break;
            case webrtc::VideoFrameBuffer::Type::kI444:
                inputFormat = "CPU I444";
                break;
            case webrtc::VideoFrameBuffer::Type::kI010:
                inputFormat = "CPU I010";
                break;
            case webrtc::VideoFrameBuffer::Type::kNV12:
                inputFormat = "CPU NV12";
                break;
            case webrtc::VideoFrameBuffer::Type::kNative:
                inputFormat = buffer->storage_representation();
                break;
            default:
                inputFormat = "Other CPU video buffer";
                break;
            }
        }
        if (inputFormat != lastReportedInputFormat_) {
            lastReportedInputFormat_ = inputFormat;
            runtimeState_->MarkInputFormat(
                runtimeInstanceId_, std::move(inputFormat));
        }
    }

    void ObserveInputFrameRate()
    {
        const auto now = std::chrono::steady_clock::now();
        if (inputRateWindowFrames_ == 0) {
            inputRateWindowStart_ = now;
        }
        ++inputRateWindowFrames_;
        const auto elapsed =
            std::chrono::duration_cast<std::chrono::milliseconds>(
                now - inputRateWindowStart_);
        if (elapsed < std::chrono::milliseconds(1000)) {
            return;
        }

        const double measured =
            static_cast<double>(inputRateWindowFrames_) * 1000.0 /
            static_cast<double>((std::max<std::int64_t>)(
                elapsed.count(), 1));
        observedInputFrameRate_ = static_cast<uint32_t>(
            (std::max<long>)(1L, std::lround(measured)));
        inputRateWindowFrames_ = 0;
        inputRateWindowStart_ = now;

        if (runtimeState_) {
            runtimeState_->MarkObservedFrameRate(
                runtimeInstanceId_, observedInputFrameRate_,
                configuredFrameRate_);
            runtimeState_->MarkFramePipeline(
                runtimeInstanceId_,
                observedInputFrameRate_,
                observedOutputFrameRate_,
                totalInputFrames_,
                totalOutputFrames_,
                totalDroppedFrames_);
        }

        // Screen capture can become nearly idle when nothing changes. Do not
        // turn that content-dependent lull into a permanent MFT media-type
        // change. Real user-selectable rates begin at 15 FPS.
        if (observedInputFrameRate_ < 12) {
            stableObservedWindows_ = 0;
            return;
        }

        const uint32_t candidate =
            SupportedFrameRateForObservedInput(observedInputFrameRate_);
        if (candidate == observedFrameRateCandidate_) {
            ++stableObservedWindows_;
        } else {
            observedFrameRateCandidate_ = candidate;
            stableObservedWindows_ = 1;
        }

        const uint32_t difference =
            candidate > configuredFrameRate_
                ? candidate - configuredFrameRate_
                : configuredFrameRate_ - candidate;
        const bool cooldownElapsed =
            lastFrameRateReconfiguration_.time_since_epoch().count() == 0 ||
            now - lastFrameRateReconfiguration_ >=
                std::chrono::seconds(3);
        // Explicit SetRates handles requested reductions at the WebRTC and
        // capture layers. Never lower the MFT media type from observed Encode
        // throughput: the synchronous transform itself may be limiting that
        // throughput, creating a permanent 60-FPS feedback loop.
        if (candidate <= configuredFrameRate_) {
            stableObservedWindows_ = 0;
            return;
        }
        const std::uint32_t requiredStableWindows = 1u;
        if (stableObservedWindows_ < requiredStableWindows ||
            difference < 3 || !cooldownElapsed) {
            return;
        }
        if (!ReconfigureFrameRate(candidate)) {
            rateReconfigurationFailed_ = true;
            return;
        }
        lastFrameRateReconfiguration_ = now;
        stableObservedWindows_ = 0;
        if (runtimeState_) {
            runtimeState_->MarkObservedFrameRate(
                runtimeInstanceId_, observedInputFrameRate_,
                configuredFrameRate_);
        }
    }

    void ObserveOutputFrameRate()
    {
        const auto now = std::chrono::steady_clock::now();
        if (outputRateWindowFrames_ == 0) {
            outputRateWindowStart_ = now;
        }
        ++outputRateWindowFrames_;
        ++totalOutputFrames_;
        const auto elapsed =
            std::chrono::duration_cast<std::chrono::milliseconds>(
                now - outputRateWindowStart_);
        if (elapsed < std::chrono::milliseconds(1000)) {
            return;
        }
        const double measured =
            static_cast<double>(outputRateWindowFrames_) * 1000.0 /
            static_cast<double>((std::max<std::int64_t>)(
                elapsed.count(), 1));
        observedOutputFrameRate_ = static_cast<uint32_t>(
            (std::max<long>)(1L, std::lround(measured)));
        outputRateWindowFrames_ = 0;
        outputRateWindowStart_ = now;
        if (runtimeState_) {
            runtimeState_->MarkFramePipeline(
                runtimeInstanceId_,
                observedInputFrameRate_,
                observedOutputFrameRate_,
                totalInputFrames_,
                totalOutputFrames_,
                totalDroppedFrames_);
        }
    }

    bool InitializeD3D()
    {
        // The native desktop capturer is initialized before WebRTC creates the
        // encoder. Reusing its device lets the video processor consume the
        // captured texture directly without a cross-device CPU readback.
        ComPtr<ID3D11Device> captureDevice;
        captureDevice.Attach(AcquireDesktopCaptureD3D11Device());
        if (captureDevice) {
            device_ = std::move(captureDevice);
            device_->GetImmediateContext(&context_);
            usingDesktopCaptureDevice_ = context_ != nullptr;
        }

        constexpr D3D_FEATURE_LEVEL levels[] = {
            D3D_FEATURE_LEVEL_11_1,
            D3D_FEATURE_LEVEL_11_0,
            D3D_FEATURE_LEVEL_10_1,
            D3D_FEATURE_LEVEL_10_0,
        };
        D3D_FEATURE_LEVEL selected = D3D_FEATURE_LEVEL_10_0;
        HRESULT hr = S_OK;
        if (!device_ || !context_) {
            usingDesktopCaptureDevice_ = false;
            hr = D3D11CreateDevice(
                nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr,
                D3D11_CREATE_DEVICE_BGRA_SUPPORT |
                    D3D11_CREATE_DEVICE_VIDEO_SUPPORT,
                levels, static_cast<UINT>(std::size(levels)),
                D3D11_SDK_VERSION, &device_, &selected, &context_);
            if (hr == E_INVALIDARG) {
                hr = D3D11CreateDevice(
                    nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr,
                    D3D11_CREATE_DEVICE_BGRA_SUPPORT |
                        D3D11_CREATE_DEVICE_VIDEO_SUPPORT,
                    levels + 1,
                    static_cast<UINT>(std::size(levels) - 1),
                    D3D11_SDK_VERSION, &device_, &selected, &context_);
            }
        }
        if (FAILED(hr) || FAILED(device_.As(&videoDevice_)) ||
            FAILED(context_.As(&videoContext_))) {
            return false;
        }
        UINT resetToken = 0;
        if (FAILED(MFCreateDXGIDeviceManager(
                &resetToken, &deviceManager_)) ||
            FAILED(deviceManager_->ResetDevice(
                device_.Get(), resetToken))) {
            return false;
        }

        // Input texture and video processor are created lazily from the first
        // frame. The encoder output size is fixed by WebRTC, while the source
        // desktop can remain at its original resolution and be scaled on GPU.
        return true;
    }

    bool InitializeTransform()
    {
        const MFT_REGISTER_TYPE_INFO input{
            MFMediaType_Video, MFVideoFormat_NV12};
        const MFT_REGISTER_TYPE_INFO output{
            MFMediaType_Video, MFVideoFormat_H264};
        IMFActivate** activations = nullptr;
        UINT32 count = 0;
        const HRESULT enumResult = MFTEnumEx(
            MFT_CATEGORY_VIDEO_ENCODER,
            MFT_ENUM_FLAG_HARDWARE | MFT_ENUM_FLAG_SORTANDFILTER,
            &input, &output, &activations, &count);
        if (FAILED(enumResult) || count == 0) {
            CoTaskMemFree(activations);
            return false;
        }
        for (UINT32 i = 0; i < count; ++i) {
            ComPtr<IMFTransform> candidate;
            ComPtr<IMFAttributes> attributes;
            if (SUCCEEDED(activations[i]->ActivateObject(
                    IID_PPV_ARGS(&candidate))) &&
                SUCCEEDED(candidate->GetAttributes(&attributes)) &&
                ReadBooleanAttribute(
                    attributes.Get(), MF_SA_D3D11_AWARE)) {
                transform_ = std::move(candidate);
                activation_ = activations[i];
                activation_->AddRef();
                break;
            }
            activations[i]->ShutdownObject();
        }
        for (UINT32 i = 0; i < count; ++i) {
            activations[i]->Release();
        }
        CoTaskMemFree(activations);
        if (!transform_) {
            return false;
        }

        ComPtr<IMFAttributes> attributes;
        transform_->GetAttributes(&attributes);
        asynchronous_ =
            ReadBooleanAttribute(attributes.Get(), MF_TRANSFORM_ASYNC);
        if (asynchronous_ &&
            FAILED(attributes->SetUINT32(
                MF_TRANSFORM_ASYNC_UNLOCK, TRUE))) {
            return false;
        }
        if (FAILED(transform_->ProcessMessage(
                MFT_MESSAGE_SET_D3D_MANAGER,
                reinterpret_cast<ULONG_PTR>(
                    deviceManager_.Get())))) {
            return false;
        }

        if (!ConfigureTransformMediaTypes(frameRate_)) {
            return false;
        }
        configuredFrameRate_ = frameRate_;

        transform_.As(&codecApi_);
        SetCodecApiBoolean(CODECAPI_AVLowLatencyMode, true);
        SetCodecApiUInt32(
            CODECAPI_AVEncCommonQualityVsSpeed,
            MfQualityVsSpeed(quality_));
        SetCodecApiUInt32(
            CODECAPI_AVEncCommonRateControlMode,
            eAVEncCommonRateControlMode_CBR);
        ApplyBitrate(bitrateBps_);
        SetCodecApiUInt32(
            CODECAPI_AVEncMPVDefaultBPictureCount, 0);
        if (asynchronous_ &&
            FAILED(transform_.As(&eventGenerator_))) {
            return false;
        }
        return true;
    }

    bool ConfigureTransformMediaTypes(uint32_t frameRate)
    {
        auto outputType = CreateVideoType(
            MFVideoFormat_H264, width_, height_, frameRate);
        auto inputType = CreateVideoType(
            MFVideoFormat_NV12, width_, height_, frameRate);
        return outputType && inputType &&
               SUCCEEDED(outputType->SetUINT32(
                   MF_MT_AVG_BITRATE, bitrateBps_)) &&
               SUCCEEDED(outputType->SetUINT32(
                   MF_MT_MPEG2_PROFILE,
                   eAVEncH264VProfile_Base)) &&
               SUCCEEDED(transform_->SetOutputType(
                   0, outputType.Get(), 0)) &&
               SUCCEEDED(transform_->SetInputType(
                   0, inputType.Get(), 0));
    }

    void ResetVideoProcessor()
    {
        activeInputView_.Reset();
        inputView_.Reset();
        processor_.Reset();
        processorEnumerator_.Reset();
        bgraTexture_.Reset();
        inputTextureWidth_ = 0;
        inputTextureHeight_ = 0;
        videoProcessorFrameRate_ = 0;
    }

    void DiscardPendingTransformEvents()
    {
        if (!asynchronous_ || !eventGenerator_) {
            return;
        }
        for (;;) {
            ComPtr<IMFMediaEvent> event;
            const HRESULT result = eventGenerator_->GetEvent(
                MF_EVENT_FLAG_NO_WAIT, &event);
            if (result == MF_E_NO_EVENTS_AVAILABLE || FAILED(result)) {
                return;
            }
        }
    }

    bool ReconfigureFrameRate(uint32_t requestedFrameRate)
    {
        if (!transform_ || requestedFrameRate == 0) {
            return false;
        }
        const uint32_t previousFrameRate = configuredFrameRate_;
        if (FAILED(transform_->ProcessMessage(
                MFT_MESSAGE_COMMAND_FLUSH, 0))) {
            return false;
        }
        DiscardPendingTransformEvents();
        pendingFrame_.reset();
        if (!ConfigureTransformMediaTypes(requestedFrameRate)) {
            // Best effort restoration keeps the current encoder usable when a
            // driver rejects a dynamic frame-rate media-type change.
            ConfigureTransformMediaTypes(previousFrameRate);
            transform_->ProcessMessage(
                MFT_MESSAGE_NOTIFY_START_OF_STREAM, 0);
            return false;
        }
        configuredFrameRate_ = requestedFrameRate;
        frameDuration100ns_ = 10'000'000LL / configuredFrameRate_;
        ResetVideoProcessor();
        frameIndex_ = 0;
        if (FAILED(transform_->ProcessMessage(
                MFT_MESSAGE_NOTIFY_START_OF_STREAM, 0))) {
            return false;
        }
        ApplyBitrate(bitrateBps_);
        SetCodecApiUInt32(CODECAPI_AVEncVideoForceKeyFrame, 1);
        rateReconfigurationFailed_ = false;
        return true;
    }

    bool EnsureVideoProcessor(UINT sourceWidth, UINT sourceHeight)
    {
        if (bgraTexture_ && processorEnumerator_ && processor_ && inputView_ &&
            inputTextureWidth_ == sourceWidth &&
            inputTextureHeight_ == sourceHeight &&
            videoProcessorFrameRate_ == configuredFrameRate_) {
            return true;
        }

        ResetVideoProcessor();

        D3D11_TEXTURE2D_DESC input{};
        input.Width = sourceWidth;
        input.Height = sourceHeight;
        input.MipLevels = 1;
        input.ArraySize = 1;
        input.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
        input.SampleDesc.Count = 1;
        input.Usage = D3D11_USAGE_DEFAULT;
        input.BindFlags =
            D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
        if (FAILED(device_->CreateTexture2D(
                &input, nullptr, &bgraTexture_))) {
            return false;
        }

        D3D11_VIDEO_PROCESSOR_CONTENT_DESC content{};
        content.InputFrameFormat = D3D11_VIDEO_FRAME_FORMAT_PROGRESSIVE;
        content.InputFrameRate = {configuredFrameRate_, 1};
        content.InputWidth = sourceWidth;
        content.InputHeight = sourceHeight;
        content.OutputFrameRate = {configuredFrameRate_, 1};
        content.OutputWidth = width_;
        content.OutputHeight = height_;
        content.Usage = D3D11_VIDEO_USAGE_OPTIMAL_SPEED;
        if (FAILED(videoDevice_->CreateVideoProcessorEnumerator(
                &content, &processorEnumerator_)) ||
            FAILED(videoDevice_->CreateVideoProcessor(
                processorEnumerator_.Get(), 0, &processor_))) {
            return false;
        }
        UINT flags = 0;
        if (FAILED(processorEnumerator_->CheckVideoProcessorFormat(
                DXGI_FORMAT_NV12, &flags)) ||
            (flags & D3D11_VIDEO_PROCESSOR_FORMAT_SUPPORT_OUTPUT) == 0) {
            return false;
        }
        D3D11_VIDEO_PROCESSOR_INPUT_VIEW_DESC view{};
        view.ViewDimension = D3D11_VPIV_DIMENSION_TEXTURE2D;
        if (FAILED(videoDevice_->CreateVideoProcessorInputView(
                bgraTexture_.Get(), processorEnumerator_.Get(),
                &view, &inputView_))) {
            return false;
        }
        videoContext_->VideoProcessorSetStreamFrameFormat(
            processor_.Get(), 0,
            D3D11_VIDEO_FRAME_FORMAT_PROGRESSIVE);
        videoContext_->VideoProcessorSetStreamAutoProcessingMode(
            processor_.Get(), 0, FALSE);
        // The desktop texture is full-range RGB. Produce conventional
        // BT.709 studio-range NV12 to match the MFT media-type metadata above.
        // Leaving these states at driver defaults made some hardware MFTs
        // wash near-white borders into the white background.
        D3D11_VIDEO_PROCESSOR_COLOR_SPACE inputColorSpace{};
        inputColorSpace.Usage = 1;
        inputColorSpace.RGB_Range = 0;
        inputColorSpace.YCbCr_Matrix = 1;
        inputColorSpace.Nominal_Range =
            D3D11_VIDEO_PROCESSOR_NOMINAL_RANGE_0_255;
        videoContext_->VideoProcessorSetStreamColorSpace(
            processor_.Get(), 0, &inputColorSpace);

        D3D11_VIDEO_PROCESSOR_COLOR_SPACE outputColorSpace{};
        outputColorSpace.Usage = 1;
        outputColorSpace.RGB_Range = 0;
        outputColorSpace.YCbCr_Matrix = 1;
        outputColorSpace.Nominal_Range =
            D3D11_VIDEO_PROCESSOR_NOMINAL_RANGE_16_235;
        videoContext_->VideoProcessorSetOutputColorSpace(
            processor_.Get(), &outputColorSpace);
        inputTextureWidth_ = sourceWidth;
        inputTextureHeight_ = sourceHeight;
        videoProcessorFrameRate_ = configuredFrameRate_;
        return true;
    }

    bool UploadBgraFrame(const webrtc::VideoFrame& frame)
    {
        activeInputView_.Reset();
        const uint8_t* bgraData = nullptr;
        int bgraStride = 0;
        UINT sourceWidth = width_;
        UINT sourceHeight = height_;
        RECT sourceRectangle{
            0, 0, static_cast<LONG>(width_), static_cast<LONG>(height_)};
        const auto buffer = frame.video_frame_buffer();
        auto* nativeDesktop =
            D3D11DesktopFrameBuffer::From(buffer.get());
        if (nativeDesktop && nativeDesktop->Device() == device_.Get()) {
            sourceWidth =
                static_cast<UINT>(nativeDesktop->SourceWidth());
            sourceHeight =
                static_cast<UINT>(nativeDesktop->SourceHeight());
            sourceRectangle = {
                nativeDesktop->CropX(),
                nativeDesktop->CropY(),
                nativeDesktop->CropX() +
                    nativeDesktop->CropWidth(),
                nativeDesktop->CropY() +
                    nativeDesktop->CropHeight()};
            if (!EnsureVideoProcessor(sourceWidth, sourceHeight)) {
                return false;
            }
            D3D11_VIDEO_PROCESSOR_INPUT_VIEW_DESC view{};
            view.ViewDimension = D3D11_VPIV_DIMENSION_TEXTURE2D;
            if (FAILED(videoDevice_->CreateVideoProcessorInputView(
                    nativeDesktop->Texture(),
                    processorEnumerator_.Get(),
                    &view, &activeInputView_))) {
                return false;
            }
            const RECT targetRectangle{
                0, 0, static_cast<LONG>(width_),
                static_cast<LONG>(height_)};
            videoContext_->VideoProcessorSetStreamSourceRect(
                processor_.Get(), 0, TRUE, &sourceRectangle);
            videoContext_->VideoProcessorSetStreamDestRect(
                processor_.Get(), 0, TRUE, &targetRectangle);
            videoContext_->VideoProcessorSetOutputTargetRect(
                processor_.Get(), TRUE, &targetRectangle);
            return true;
        }

        const bool isDesktopBgra =
            buffer->type() ==
                webrtc::VideoFrameBuffer::Type::kNative &&
            buffer->storage_representation() ==
                "CPU_BGRA_DESKTOP";
        if (isDesktopBgra) {
            // libwebrtc is built without C++ RTTI. Never dynamic_cast a
            // VideoFrameBuffer supplied by libwebrtc: an I420 camera buffer
            // has no MSVC RTTI metadata and __RTDynamicCast will crash.
            auto* desktop =
                static_cast<DesktopBgraFrameBuffer*>(
                    buffer.get());
            bgraData = desktop->Data();
            bgraStride = desktop->Stride();
            sourceWidth = static_cast<UINT>(desktop->SourceWidth());
            sourceHeight = static_cast<UINT>(desktop->SourceHeight());
            sourceRectangle = {
                desktop->CropX(), desktop->CropY(),
                desktop->CropX() + desktop->CropWidth(),
                desktop->CropY() + desktop->CropHeight()};
        } else {
            // This also covers a native D3D11 frame produced on a different
            // device. ToI420 performs the explicit software fallback rather
            // than trying to open an unsafe cross-device texture.
            auto i420 = buffer->ToI420();
            if (!i420) {
                return false;
            }
            convertedBgra_.resize(
                static_cast<size_t>(width_) * height_ * 4);
            bgraStride = static_cast<int>(width_) * 4;
            if (libyuv::I420ToARGB(
                    i420->DataY(), i420->StrideY(),
                    i420->DataU(), i420->StrideU(),
                    i420->DataV(), i420->StrideV(),
                    convertedBgra_.data(), bgraStride,
                    static_cast<int>(width_),
                    static_cast<int>(height_)) != 0) {
                return false;
            }
            bgraData = convertedBgra_.data();
        }
        if (!EnsureVideoProcessor(sourceWidth, sourceHeight)) {
            return false;
        }
        context_->UpdateSubresource(
            bgraTexture_.Get(), 0, nullptr, bgraData,
            bgraStride, 0);
        activeInputView_ = inputView_;
        const RECT targetRectangle{
            0, 0, static_cast<LONG>(width_), static_cast<LONG>(height_)};
        videoContext_->VideoProcessorSetStreamSourceRect(
            processor_.Get(), 0, TRUE, &sourceRectangle);
        videoContext_->VideoProcessorSetStreamDestRect(
            processor_.Get(), 0, TRUE, &targetRectangle);
        videoContext_->VideoProcessorSetOutputTargetRect(
            processor_.Get(), TRUE, &targetRectangle);
        return true;
    }

    ComPtr<IMFSample> CreateInputSample(
        const webrtc::VideoFrame& frame,
        LONGLONG sampleTime)
    {
        if (!UploadBgraFrame(frame)) {
            return nullptr;
        }
        D3D11_TEXTURE2D_DESC output{};
        output.Width = width_;
        output.Height = height_;
        output.MipLevels = 1;
        output.ArraySize = 1;
        output.Format = DXGI_FORMAT_NV12;
        output.SampleDesc.Count = 1;
        output.Usage = D3D11_USAGE_DEFAULT;
        output.BindFlags =
            D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
        ComPtr<ID3D11Texture2D> nv12;
        if (FAILED(device_->CreateTexture2D(
                &output, nullptr, &nv12))) {
            return nullptr;
        }
        D3D11_VIDEO_PROCESSOR_OUTPUT_VIEW_DESC view{};
        view.ViewDimension = D3D11_VPOV_DIMENSION_TEXTURE2D;
        ComPtr<ID3D11VideoProcessorOutputView> outputView;
        if (FAILED(videoDevice_->CreateVideoProcessorOutputView(
                nv12.Get(), processorEnumerator_.Get(),
                &view, &outputView))) {
            return nullptr;
        }
        D3D11_VIDEO_PROCESSOR_STREAM stream{};
        stream.Enable = TRUE;
        stream.InputFrameOrField =
            static_cast<UINT>(frameIndex_++);
        stream.pInputSurface = activeInputView_.Get();
        if (!stream.pInputSurface) {
            return nullptr;
        }
        if (FAILED(videoContext_->VideoProcessorBlt(
                processor_.Get(), outputView.Get(),
                0, 1, &stream))) {
            return nullptr;
        }

        ComPtr<IMFMediaBuffer> buffer;
        ComPtr<IMFSample> sample;
        if (FAILED(MFCreateDXGISurfaceBuffer(
                __uuidof(ID3D11Texture2D), nv12.Get(),
                0, FALSE, &buffer)) ||
            FAILED(MFCreateSample(&sample)) ||
            FAILED(sample->AddBuffer(buffer.Get())) ||
            FAILED(sample->SetSampleTime(sampleTime)) ||
            FAILED(sample->SetSampleDuration(frameDuration100ns_))) {
            return nullptr;
        }
        return sample;
    }

    bool WaitForNeedInput()
    {
        if (!asynchronous_) {
            return true;
        }
        const auto deadline =
            std::chrono::steady_clock::now() + kEventTimeout;
        while (std::chrono::steady_clock::now() < deadline) {
            MediaEventType type = MEUnknown;
            if (!ReadEvent(type)) {
                continue;
            }
            if (type == METransformNeedInput) {
                return true;
            }
            if (type == METransformHaveOutput) {
                if (!ReadOutput()) {
                    return false;
                }
            }
        }
        return false;
    }

    bool WaitForOutput()
    {
        if (!asynchronous_) {
            for (;;) {
                const HRESULT result = ReadOutputSample();
                if (result == MF_E_TRANSFORM_NEED_MORE_INPUT) {
                    return true;
                }
                if (FAILED(result)) {
                    return false;
                }
                return true;
            }
        }
        const auto deadline =
            std::chrono::steady_clock::now() + kEventTimeout;
        while (std::chrono::steady_clock::now() < deadline) {
            MediaEventType type = MEUnknown;
            if (!ReadEvent(type)) {
                continue;
            }
            if (type == METransformHaveOutput) {
                return ReadOutput();
            }
        }
        return false;
    }

    bool ReadEvent(MediaEventType& type)
    {
        ComPtr<IMFMediaEvent> event;
        const HRESULT result = eventGenerator_->GetEvent(
            MF_EVENT_FLAG_NO_WAIT, &event);
        if (result == MF_E_NO_EVENTS_AVAILABLE) {
            std::this_thread::sleep_for(
                std::chrono::milliseconds(1));
            return false;
        }
        HRESULT status = S_OK;
        if (FAILED(result) ||
            FAILED(event->GetStatus(&status)) ||
            FAILED(status) ||
            FAILED(event->GetType(&type))) {
            type = MEError;
            return false;
        }
        return true;
    }

    bool ReadOutput()
    {
        const HRESULT result = ReadOutputSample();
        return SUCCEEDED(result) ||
               result == MF_E_TRANSFORM_NEED_MORE_INPUT;
    }

    HRESULT ReadOutputSample()
    {
        MFT_OUTPUT_STREAM_INFO info{};
        HRESULT result =
            transform_->GetOutputStreamInfo(0, &info);
        if (FAILED(result)) {
            return result;
        }
        ComPtr<IMFSample> callerSample;
        if ((info.dwFlags &
             MFT_OUTPUT_STREAM_PROVIDES_SAMPLES) == 0) {
            ComPtr<IMFMediaBuffer> buffer;
            const DWORD size =
                (std::max<DWORD>)(info.cbSize, 2 * 1024 * 1024);
            if (FAILED(MFCreateSample(&callerSample)) ||
                FAILED(MFCreateMemoryBuffer(size, &buffer)) ||
                FAILED(callerSample->AddBuffer(buffer.Get()))) {
                return E_OUTOFMEMORY;
            }
        }
        MFT_OUTPUT_DATA_BUFFER output{};
        output.pSample = callerSample.Get();
        DWORD status = 0;
        result = transform_->ProcessOutput(
            0, 1, &output, &status);
        if (output.pEvents) {
            output.pEvents->Release();
        }
        if (FAILED(result)) {
            if (output.pSample && !callerSample) {
                output.pSample->Release();
            }
            return result;
        }
        IMFSample* sample =
            output.pSample ? output.pSample : callerSample.Get();
        ComPtr<IMFMediaBuffer> contiguous;
        if (!sample ||
            FAILED(sample->ConvertToContiguousBuffer(&contiguous))) {
            if (output.pSample && !callerSample) {
                output.pSample->Release();
            }
            return E_FAIL;
        }
        BYTE* bytes = nullptr;
        DWORD maximumLength = 0;
        DWORD currentLength = 0;
        result = contiguous->Lock(
            &bytes, &maximumLength, &currentLength);
        if (SUCCEEDED(result)) {
            if (!IsAnnexB(bytes, currentLength)) {
                result = MF_E_INVALIDMEDIATYPE;
            } else {
                DeliverEncodedImage(sample, bytes, currentLength);
            }
            contiguous->Unlock();
        }
        if (output.pSample && !callerSample) {
            output.pSample->Release();
        }
        return result;
    }

    void DeliverEncodedImage(IMFSample* sample,
                             const uint8_t* bytes,
                             size_t length)
    {
        if (!callback_) {
            return;
        }
        UINT32 cleanPoint = FALSE;
        sample->GetUINT32(
            MFSampleExtension_CleanPoint, &cleanPoint);

        webrtc::EncodedImage image;
        image.SetEncodedData(
            webrtc::EncodedImageBuffer::Create(bytes, length));
        if (!pendingFrame_) {
            return;
        }
        image.SetRtpTimestamp(pendingFrame_->rtp_timestamp());
        image.capture_time_ms_ =
            pendingFrame_->timestamp_us() / 1000;
        image._encodedWidth = width_;
        image._encodedHeight = height_;
        image.rotation_ = pendingFrame_->rotation();
        image.SetFrameType(
            cleanPoint != FALSE
                ? webrtc::VideoFrameType::kVideoFrameKey
                : webrtc::VideoFrameType::kVideoFrameDelta);

        webrtc::CodecSpecificInfo codecInfo;
        codecInfo.codecType = webrtc::kVideoCodecH264;
        codecInfo.codecSpecific.H264.packetization_mode =
            webrtc::H264PacketizationMode::NonInterleaved;
        codecInfo.codecSpecific.H264.temporal_idx =
            webrtc::kNoTemporalIdx;
        codecInfo.codecSpecific.H264.idr_frame =
            cleanPoint != FALSE;
        codecInfo.codecSpecific.H264.base_layer_sync = false;
        ObserveOutputFrameRate();
        callback_->OnEncodedImage(image, &codecInfo);
    }

    bool SetCodecApiBoolean(const GUID& key, bool value)
    {
        if (!codecApi_ ||
            codecApi_->IsSupported(&key) != S_OK) {
            return false;
        }
        VARIANT setting;
        VariantInit(&setting);
        setting.vt = VT_BOOL;
        setting.boolVal =
            value ? VARIANT_TRUE : VARIANT_FALSE;
        return codecApi_->SetValue(&key, &setting) == S_OK;
    }

    bool SetCodecApiUInt32(const GUID& key, uint32_t value)
    {
        if (!codecApi_ ||
            codecApi_->IsSupported(&key) != S_OK) {
            return false;
        }
        VARIANT setting;
        VariantInit(&setting);
        setting.vt = VT_UI4;
        setting.ulVal = value;
        return codecApi_->SetValue(&key, &setting) == S_OK;
    }

    std::optional<uint32_t> ReadCodecApiUInt32(const GUID& key) const
    {
        if (!codecApi_ ||
            codecApi_->IsSupported(&key) != S_OK) {
            return std::nullopt;
        }
        VARIANT value;
        VariantInit(&value);
        if (codecApi_->GetValue(&key, &value) != S_OK) {
            VariantClear(&value);
            return std::nullopt;
        }

        std::optional<uint32_t> result;
        switch (value.vt) {
        case VT_UI4:
            result = value.ulVal;
            break;
        case VT_I4:
            if (value.lVal >= 0) {
                result = static_cast<uint32_t>(value.lVal);
            }
            break;
        case VT_UINT:
            result = value.uintVal;
            break;
        case VT_INT:
            if (value.intVal >= 0) {
                result = static_cast<uint32_t>(value.intVal);
            }
            break;
        case VT_UI8:
            if (value.ullVal <=
                (std::numeric_limits<uint32_t>::max)()) {
                result = static_cast<uint32_t>(value.ullVal);
            }
            break;
        case VT_I8:
            if (value.llVal >= 0 &&
                static_cast<unsigned long long>(value.llVal) <=
                    (std::numeric_limits<uint32_t>::max)()) {
                result = static_cast<uint32_t>(value.llVal);
            }
            break;
        default:
            break;
        }
        VariantClear(&value);
        return result;
    }

    void ApplyBitrate(uint32_t bitrateBps)
    {
        bitrateConfigurationAccepted_ = SetCodecApiUInt32(
            CODECAPI_AVEncCommonMeanBitRate, bitrateBps);
        const auto configured = ReadCodecApiUInt32(
            CODECAPI_AVEncCommonMeanBitRate);
        bitrateReadbackAvailable_ = configured.has_value();
        configuredBitrateBps_ = configured.value_or(0);
    }

    void ReleaseUnlocked()
    {
        if (initialized_ && runtimeState_) {
            runtimeState_->MarkHardwareStopped(runtimeInstanceId_);
        }
        initialized_ = false;
        rateReconfigurationFailed_ = false;
        configuredBitrateBps_ = 0;
        bitrateConfigurationAccepted_ = false;
        bitrateReadbackAvailable_ = false;
        observedInputFrameRate_ = 0;
        observedOutputFrameRate_ = 0;
        inputRateWindowFrames_ = 0;
        outputRateWindowFrames_ = 0;
        totalInputFrames_ = 0;
        totalOutputFrames_ = 0;
        totalDroppedFrames_ = 0;
        observedFrameRateCandidate_ = 0;
        stableObservedWindows_ = 0;
        inputRateWindowStart_ = {};
        outputRateWindowStart_ = {};
        lastFrameRateReconfiguration_ = {};
        pendingFrame_.reset();
        if (transform_) {
            transform_->ProcessMessage(
                MFT_MESSAGE_COMMAND_FLUSH, 0);
            transform_->ProcessMessage(
                MFT_MESSAGE_NOTIFY_END_STREAMING, 0);
        }
        codecApi_.Reset();
        eventGenerator_.Reset();
        transform_.Reset();
        if (activation_) {
            activation_->ShutdownObject();
            activation_->Release();
            activation_ = nullptr;
        }
        ResetVideoProcessor();
        deviceManager_.Reset();
        videoContext_.Reset();
        videoDevice_.Reset();
        context_.Reset();
        device_.Reset();
        if (mfStarted_) {
            MFShutdown();
            mfStarted_ = false;
        }
        if (comInitialized_) {
            CoUninitialize();
            comInitialized_ = false;
        }
    }

    std::mutex mutex_;
    webrtc::EncodedImageCallback* callback_ = nullptr;
    std::optional<webrtc::VideoFrame> pendingFrame_;
    uint32_t width_ = 0;
    uint32_t height_ = 0;
    uint32_t frameRate_ = 30;
    uint32_t configuredFrameRate_ = 30;
    uint32_t observedInputFrameRate_ = 0;
    uint32_t observedOutputFrameRate_ = 0;
    uint32_t bitrateBps_ = 2'000'000;
    uint32_t configuredBitrateBps_ = 0;
    uint64_t frameIndex_ = 0;
    LONGLONG frameDuration100ns_ = 333'333;
    bool comInitialized_ = false;
    bool mfStarted_ = false;
    bool initialized_ = false;
    bool asynchronous_ = false;
    bool rateReconfigurationFailed_ = false;
    bool bitrateConfigurationAccepted_ = false;
    bool bitrateReadbackAvailable_ = false;
    bool usingDesktopCaptureDevice_ = false;
    std::chrono::steady_clock::time_point inputRateWindowStart_{};
    std::chrono::steady_clock::time_point outputRateWindowStart_{};
    std::chrono::steady_clock::time_point lastFrameRateReconfiguration_{};
    std::uint32_t inputRateWindowFrames_ = 0;
    std::uint32_t outputRateWindowFrames_ = 0;
    std::uint64_t totalInputFrames_ = 0;
    std::uint64_t totalOutputFrames_ = 0;
    std::uint64_t totalDroppedFrames_ = 0;
    std::uint32_t observedFrameRateCandidate_ = 0;
    std::uint32_t stableObservedWindows_ = 0;
    IMFActivate* activation_ = nullptr;
    ComPtr<ID3D11Device> device_;
    ComPtr<ID3D11DeviceContext> context_;
    ComPtr<ID3D11VideoDevice> videoDevice_;
    ComPtr<ID3D11VideoContext> videoContext_;
    ComPtr<IMFDXGIDeviceManager> deviceManager_;
    ComPtr<ID3D11Texture2D> bgraTexture_;
    ComPtr<ID3D11VideoProcessorEnumerator> processorEnumerator_;
    ComPtr<ID3D11VideoProcessor> processor_;
    ComPtr<ID3D11VideoProcessorInputView> inputView_;
    ComPtr<ID3D11VideoProcessorInputView> activeInputView_;
    UINT inputTextureWidth_ = 0;
    UINT inputTextureHeight_ = 0;
    UINT videoProcessorFrameRate_ = 0;
    ComPtr<IMFTransform> transform_;
    ComPtr<IMFMediaEventGenerator> eventGenerator_;
    ComPtr<ICodecAPI> codecApi_;
    std::vector<uint8_t> convertedBgra_;
    std::shared_ptr<VideoEncoderRuntimeState> runtimeState_;
    FfmpegX264Preset quality_ = FfmpegX264Preset::kMedium;
    std::uint64_t runtimeInstanceId_ = 0;
    std::uint64_t desktopShareGeneration_ = 0;
    std::optional<webrtc::VideoCodec> lastCodecSettings_;
    std::string lastReportedInputFormat_;
};

MfD3D11H264Encoder::MfD3D11H264Encoder()
    : impl_(std::make_unique<Impl>())
{}

MfD3D11H264Encoder::MfD3D11H264Encoder(
    std::shared_ptr<VideoEncoderRuntimeState> runtimeState,
    FfmpegX264Preset quality)
    : impl_(std::make_unique<Impl>(
          std::move(runtimeState), quality))
{}

MfD3D11H264Encoder::~MfD3D11H264Encoder() = default;

int MfD3D11H264Encoder::InitEncode(
    const webrtc::VideoCodec* codecSettings,
    const Settings& /* settings */)
{
    return impl_->Init(codecSettings);
}

int32_t MfD3D11H264Encoder::RegisterEncodeCompleteCallback(
    webrtc::EncodedImageCallback* callback)
{
    return impl_->RegisterCallback(callback);
}

int32_t MfD3D11H264Encoder::Release()
{
    return impl_->Release();
}

int32_t MfD3D11H264Encoder::Encode(
    const webrtc::VideoFrame& frame,
    const std::vector<webrtc::VideoFrameType>* frameTypes)
{
    return impl_->Encode(frame, frameTypes);
}

void MfD3D11H264Encoder::SetRates(
    const RateControlParameters& parameters)
{
    impl_->SetRates(parameters);
}

webrtc::VideoEncoder::EncoderInfo
MfD3D11H264Encoder::GetEncoderInfo() const
{
    EncoderInfo info;
    info.supports_native_handle = true;
    info.implementation_name =
        std::string("MediaFoundationD3D11H264 (quality=") +
        std::to_string(impl_->QualityVsSpeed()) + ")";
    info.has_trusted_rate_controller = false;
    info.is_hardware_accelerated = true;
    // Keep WebRTC's send-processing detector initialized. Disabling it leaves
    // OveruseFrameDetector::usage_ empty while target-framerate updates can
    // still arrive, which crashes on the first camera frame.
    info.enable_cpu_overuse_detection = true;
    info.supports_simulcast = false;
    info.requested_resolution_alignment = 2;
    info.apply_alignment_to_all_simulcast_layers = true;
    info.preferred_pixel_formats.push_back(
        webrtc::VideoFrameBuffer::Type::kNative);
    info.preferred_pixel_formats.push_back(
        webrtc::VideoFrameBuffer::Type::kI420);
    info.scaling_settings = ScalingSettings::kOff;
    return info;
}

}  // namespace remote
