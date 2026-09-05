// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#include "MfD3D11H264Decoder.h"

#include <algorithm>
#include <array>
#include <atomic>
#include <chrono>
#include <condition_variable>
#include <cstring>
#include <deque>
#include <exception>
#include <iomanip>
#include <iterator>
#include <mutex>
#include <optional>
#include <sstream>
#include <thread>
#include <utility>
#include <vector>

#include <Windows.h>
#include <codecapi.h>
#include <d3d10_1.h>
#include <d3d11.h>
#include <mfapi.h>
#include <mferror.h>
#include <mfidl.h>
#include <mftransform.h>
#include <strmif.h>
#include <wrl/client.h>

#include "api/make_ref_counted.h"
#include "api/video/video_frame.h"
#include "rtc_base/numerics/sequence_number_unwrapper.h"
#include "modules/video_coding/include/video_error_codes.h"
#include "D3D11NativeFrameBuffer.h"
#include "src/webrtc/VideoCodecTimingTelemetry.h"

namespace remote {
namespace {

using Microsoft::WRL::ComPtr;

constexpr uint32_t kDefaultProbeWidth = 1920;
constexpr uint32_t kDefaultProbeHeight = 1080;
// The renderer can retain the last presented sample while a newer native
// sample waits in its mailbox. Keep enough decoder-owned DXVA surfaces in the
// circular output pool so those references cannot starve ProcessOutput. Do
// not lower a decoder/driver supplied value when it already requests more.
constexpr UINT32 kMinimumLowLatencyOutputSurfaces = 8;

std::string HResultError(const char* operation, HRESULT result)
{
    std::ostringstream stream;
    stream << operation << " failed with HRESULT 0x" << std::hex
           << std::uppercase << static_cast<uint32_t>(result) << '.';
    return stream.str();
}

std::string Narrow(const std::wstring& text)
{
    if (text.empty()) {
        return {};
    }

    const int required = WideCharToMultiByte(
        CP_UTF8,
        0,
        text.data(),
        static_cast<int>(text.size()),
        nullptr,
        0,
        nullptr,
        nullptr);
    if (required <= 0) {
        return {};
    }

    std::string result(static_cast<size_t>(required), '\0');
    WideCharToMultiByte(
        CP_UTF8,
        0,
        text.data(),
        static_cast<int>(text.size()),
        result.data(),
        required,
        nullptr,
        nullptr);
    return result;
}

std::wstring ActivationName(IMFActivate* activation)
{
    WCHAR* name = nullptr;
    UINT32 length = 0;
    std::wstring result;
    if (activation && SUCCEEDED(activation->GetAllocatedString(
                              MFT_FRIENDLY_NAME_Attribute,
                              &name,
                              &length))) {
        result.assign(name, length);
    }
    CoTaskMemFree(name);
    return result;
}

bool ActivationIsHardware(IMFActivate* activation)
{
    if (!activation) {
        return false;
    }

    WCHAR* hardwareUrl = nullptr;
    UINT32 length = 0;
    const HRESULT result = activation->GetAllocatedString(
        MFT_ENUM_HARDWARE_URL_Attribute,
        &hardwareUrl,
        &length);
    const bool isHardware = SUCCEEDED(result) && length > 0;
    CoTaskMemFree(hardwareUrl);
    return isHardware;
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

}  // namespace

class MfD3D11H264Decoder::Impl final {
public:
    explicit Impl(MfD3D11H264DecoderSelection selection,
                  std::string preferredDecoderName,
                  bool requirePreferredDecoder)
        : selection_(selection),
          preferredDecoderName_(std::move(preferredDecoderName)),
          requirePreferredDecoder_(requirePreferredDecoder)
    {}

    bool Configure(const Settings& settings)
    {
        auto* registeredCallback = callback_;
        Release();
        callback_ = registeredCallback;
        lastError_.clear();

        if (settings.codec_type() != webrtc::kVideoCodecH264) {
            lastError_ = "MfD3D11H264Decoder only supports H264.";
            return false;
        }

        const HRESULT comResult = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
        if (SUCCEEDED(comResult)) {
            shouldUninitializeCom_ = true;
        } else if (comResult != RPC_E_CHANGED_MODE) {
            lastError_ = HResultError("CoInitializeEx", comResult);
            return false;
        }

        const HRESULT startupResult = MFStartup(MF_VERSION, MFSTARTUP_FULL);
        if (FAILED(startupResult)) {
            lastError_ = HResultError("MFStartup", startupResult);
            Release();
            return false;
        }
        mediaFoundationStarted_ = true;

        if (!CreateD3D11Device()) {
            Release();
            return false;
        }

        const auto maxResolution = settings.max_render_resolution();
        const UINT32 requestedWidth = maxResolution.Valid()
            ? static_cast<UINT32>(maxResolution.Width())
            : kDefaultProbeWidth;
        const UINT32 requestedHeight = maxResolution.Valid()
            ? static_cast<UINT32>(maxResolution.Height())
            : kDefaultProbeHeight;
        hardwareDecodeSupported_ =
            SupportsH264HardwareDecoding(
                requestedWidth, requestedHeight);
        if (selection_ ==
                MfD3D11H264DecoderSelection::kHardwareOnly &&
            !hardwareDecodeSupported_) {
            lastError_ =
                "The D3D11 device exposes no H264/NV12 hardware decode "
                "configuration for " +
                std::to_string(requestedWidth) + "x" +
                std::to_string(requestedHeight) + ".";
            Release();
            return false;
        }

        if (!CreateDeviceManager() ||
            !CreateAndConfigureTransform(settings)) {
            Release();
            return false;
        }
        if (selection_ ==
                MfD3D11H264DecoderSelection::kHardwareOnly &&
            !supportsNativeOutputSamples_) {
            lastError_ =
                "The selected H264 decoder cannot provide native D3D11 "
                "output samples.";
            Release();
            return false;
        }

        transform_->ProcessMessage(MFT_MESSAGE_NOTIFY_BEGIN_STREAMING, 0);
        transform_->ProcessMessage(MFT_MESSAGE_NOTIFY_START_OF_STREAM, 0);
        configured_ = true;
        if (asynchronous_ && !StartAsyncEventLoop()) {
            Release();
            return false;
        }
        return true;
    }

    int32_t Decode(const webrtc::EncodedImage& inputImage,
                   int64_t renderTimeMs)
    {
        if (!configured_ || !transform_) {
            return WEBRTC_VIDEO_CODEC_UNINITIALIZED;
        }
        if (!callback_ || !inputImage.data() || inputImage.size() == 0) {
            return WEBRTC_VIDEO_CODEC_FALLBACK_SOFTWARE;
        }

        const std::int64_t decodeStartedAtUs =
            VideoCodecTimingRegistry::SteadyNowUs();
        ComPtr<IMFSample> inputSample;
        FrameMetadata metadata;
        metadata.decodeStartedAtUs = decodeStartedAtUs;
        HRESULT result = CreateInputSample(
            inputImage, renderTimeMs, &inputSample, metadata);
        if (FAILED(result)) {
            lastError_ = HResultError("Create H264 input sample", result);
            return WEBRTC_VIDEO_CODEC_FALLBACK_SOFTWARE;
        }
        metadata.inputReadyAtUs =
            VideoCodecTimingRegistry::SteadyNowUs();

        if (asynchronous_) {
            if (asyncFailed_.load(std::memory_order_acquire)) {
                return WEBRTC_VIDEO_CODEC_FALLBACK_SOFTWARE;
            }
            {
                std::lock_guard lock(asyncMutex_);
                // Do not build a seconds-long hardware queue. Returning the
                // fallback code lets WebRTC switch to the proven software
                // decoder without silently dropping an H264 reference frame.
                constexpr std::size_t kMaximumLowLatencyBacklog = 8;
                const std::size_t backlog = queuedInputs_.size() +
                    inFlightFrameDepth_.load(std::memory_order_relaxed);
                if (backlog >= kMaximumLowLatencyBacklog) {
                    lastError_ =
                        "The asynchronous hardware decoder exceeded the "
                        "8-frame low-latency backlog limit.";
                    backpressureFallbacks_.fetch_add(
                        1, std::memory_order_relaxed);
                    asyncFailed_.store(true, std::memory_order_release);
                    return WEBRTC_VIDEO_CODEC_FALLBACK_SOFTWARE;
                }
                queuedInputs_.push_back(
                    {std::move(inputSample), std::move(metadata)});
                queuedInputDepth_.store(
                    static_cast<std::uint32_t>(queuedInputs_.size()),
                    std::memory_order_relaxed);
                UpdatePeakBacklog();
            }
            asyncCondition_.notify_one();
            return WEBRTC_VIDEO_CODEC_OK;
        }

        result = transform_->ProcessInput(
            inputStreamId_, inputSample.Get(), 0);
        if (result == MF_E_NOTACCEPTING) {
            int drainedOutputCount = 0;
            result = DrainOutput(drainedOutputCount);
            if (FAILED(result)) {
                lastError_ = HResultError("Drain H264 decoder output", result);
                return WEBRTC_VIDEO_CODEC_FALLBACK_SOFTWARE;
            }
            result = transform_->ProcessInput(
                inputStreamId_, inputSample.Get(), 0);
        }
        if (FAILED(result)) {
            lastError_ = HResultError("IMFTransform::ProcessInput", result);
            return WEBRTC_VIDEO_CODEC_FALLBACK_SOFTWARE;
        }
        metadata.submittedAtUs =
            VideoCodecTimingRegistry::SteadyNowUs();
        pendingFrames_.push_back(std::move(metadata));
        inFlightFrameDepth_.store(
            static_cast<std::uint32_t>(pendingFrames_.size()),
            std::memory_order_relaxed);
        UpdatePeakBacklog();

        int outputCount = 0;
        result = DrainOutput(outputCount);
        if (FAILED(result)) {
            lastError_ = HResultError("IMFTransform::ProcessOutput", result);
            return WEBRTC_VIDEO_CODEC_FALLBACK_SOFTWARE;
        }

        // A synchronous DXVA-backed MFT may accept an input sample while
        // retaining one to three frames internally before exposing the next
        // output sample. WEBRTC_VIDEO_CODEC_NO_OUTPUT is not an innocuous
        // "accepted but delayed" result in this libwebrtc revision: the
        // receive stream treats it as a decode failure, clears the timestamp
        // map and requests a key frame. That produced the characteristic
        // frames-dropped/PLI/key-frame cycle even with zero RTP packet loss.
        // Successful ProcessInput means ownership of this frame has entered
        // the decoder pipeline; its eventual output is delivered through the
        // registered callback, just like the asynchronous MFT path.
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
        StopAsyncEventLoop();
        callback_ = nullptr;

        if (transform_) {
            transform_->ProcessMessage(MFT_MESSAGE_COMMAND_FLUSH, 0);
            transform_->ProcessMessage(MFT_MESSAGE_NOTIFY_END_OF_STREAM, 0);
            transform_->ProcessMessage(MFT_MESSAGE_NOTIFY_END_STREAMING, 0);
        }
        transform_.Reset();
        if (activation_) {
            activation_->ShutdownObject();
        }
        activation_.Reset();

        dxgiDeviceManager_.Reset();
        d3dContext_.Reset();
        d3dDevice_.Reset();
        inputStreamId_ = 0;
        outputStreamId_ = 0;
        resetToken_ = 0;
        visibleWidth_ = 0;
        visibleHeight_ = 0;
        supportsNativeOutputSamples_ = false;
        asynchronous_ = false;
        hardwareDecodeSupported_ = false;
        decoderKind_ = MfD3D11H264DecoderKind::kUnknown;
        rtpTimestampUnwrapper_.Reset();
        baseRtpTimestamp_.reset();
        previousRtpTimestamp_.reset();
        pendingFrames_.clear();
        {
            std::lock_guard lock(asyncMutex_);
            queuedInputs_.clear();
            pendingInputRequests_ = 0;
        }
        asyncFailed_.store(false, std::memory_order_release);
        queuedInputDepth_.store(0, std::memory_order_relaxed);
        inFlightFrameDepth_.store(0, std::memory_order_relaxed);
        peakBacklogFrames_.store(0, std::memory_order_relaxed);
        backpressureFallbacks_.store(0, std::memory_order_relaxed);
        decoderName_.clear();
        d3dAdapterName_.clear();

        if (mediaFoundationStarted_) {
            MFShutdown();
            mediaFoundationStarted_ = false;
        }
        if (shouldUninitializeCom_) {
            CoUninitialize();
            shouldUninitializeCom_ = false;
        }
        return WEBRTC_VIDEO_CODEC_OK;
    }

    bool IsConfigured() const noexcept { return configured_; }
    bool SupportsNativeOutputSamples() const noexcept
    {
        return supportsNativeOutputSamples_;
    }
    MfD3D11H264DecoderKind DecoderKind() const noexcept
    {
        return decoderKind_;
    }
    bool IsHardwareAccelerated() const noexcept
    {
        return decoderKind_ == MfD3D11H264DecoderKind::kHardware;
    }
    bool IsAsynchronous() const noexcept { return asynchronous_; }
    const std::string& LastError() const noexcept { return lastError_; }
    const std::string& DecoderName() const noexcept { return decoderName_; }

private:
    struct FrameMetadata {
        LONGLONG sampleTime100ns = 0;
        uint32_t rtpTimestamp = 0;
        int64_t renderTimeMs = 0;
        int64_t ntpTimeMs = 0;
        webrtc::VideoRotation rotation = webrtc::kVideoRotation_0;
        std::optional<webrtc::ColorSpace> colorSpace;
        webrtc::RtpPacketInfos packetInfos;
        std::int64_t decodeStartedAtUs = 0;
        std::int64_t inputReadyAtUs = 0;
        std::int64_t submittedAtUs = 0;
    };

    struct QueuedInput {
        ComPtr<IMFSample> sample;
        FrameMetadata metadata;
    };

    void UpdatePeakBacklog()
    {
        const std::uint32_t backlog =
            queuedInputDepth_.load(std::memory_order_relaxed) +
            inFlightFrameDepth_.load(std::memory_order_relaxed);
        std::uint32_t peak =
            peakBacklogFrames_.load(std::memory_order_relaxed);
        while (backlog > peak &&
               !peakBacklogFrames_.compare_exchange_weak(
                   peak, backlog, std::memory_order_relaxed)) {
        }
    }

    bool CreateD3D11Device()
    {
        constexpr std::array featureLevels{
            D3D_FEATURE_LEVEL_11_1,
            D3D_FEATURE_LEVEL_11_0,
            D3D_FEATURE_LEVEL_10_1,
            D3D_FEATURE_LEVEL_10_0,
        };
        D3D_FEATURE_LEVEL selectedFeatureLevel = D3D_FEATURE_LEVEL_10_0;
        const UINT flags = D3D11_CREATE_DEVICE_BGRA_SUPPORT |
                           D3D11_CREATE_DEVICE_VIDEO_SUPPORT;

        HRESULT result = D3D11CreateDevice(
            nullptr,
            D3D_DRIVER_TYPE_HARDWARE,
            nullptr,
            flags,
            featureLevels.data(),
            static_cast<UINT>(featureLevels.size()),
            D3D11_SDK_VERSION,
            &d3dDevice_,
            &selectedFeatureLevel,
            &d3dContext_);
        if (result == E_INVALIDARG) {
            result = D3D11CreateDevice(
                nullptr,
                D3D_DRIVER_TYPE_HARDWARE,
                nullptr,
                flags,
                featureLevels.data() + 1,
                static_cast<UINT>(featureLevels.size() - 1),
                D3D11_SDK_VERSION,
                &d3dDevice_,
                &selectedFeatureLevel,
                &d3dContext_);
        }
        if (FAILED(result)) {
            lastError_ = HResultError("D3D11CreateDevice", result);
            return false;
        }

        ComPtr<ID3D10Multithread> multithread;
        if (SUCCEEDED(d3dDevice_.As(&multithread))) {
            multithread->SetMultithreadProtected(TRUE);
        }
        ComPtr<IDXGIDevice> dxgiDevice;
        ComPtr<IDXGIAdapter> adapter;
        DXGI_ADAPTER_DESC adapterDescription{};
        if (SUCCEEDED(d3dDevice_.As(&dxgiDevice)) &&
            SUCCEEDED(dxgiDevice->GetAdapter(&adapter)) &&
            SUCCEEDED(adapter->GetDesc(&adapterDescription))) {
            d3dAdapterName_ = Narrow(adapterDescription.Description);
        }
        return true;
    }

    bool CreateDeviceManager()
    {
        HRESULT result = MFCreateDXGIDeviceManager(
            &resetToken_,
            &dxgiDeviceManager_);
        if (SUCCEEDED(result)) {
            result = dxgiDeviceManager_->ResetDevice(
                d3dDevice_.Get(),
                resetToken_);
        }
        if (FAILED(result)) {
            lastError_ = HResultError("MFCreateDXGIDeviceManager", result);
            return false;
        }
        return true;
    }

    bool SupportsH264HardwareDecoding(
        UINT32 width,
        UINT32 height) const
    {
        if (!d3dDevice_ || width == 0 || height == 0) {
            return false;
        }

        ComPtr<ID3D11VideoDevice> videoDevice;
        if (FAILED(d3dDevice_.As(&videoDevice)) || !videoDevice) {
            return false;
        }

        // Use local GUID values so every executable that consumes the static
        // transport library does not also need to link dxguid.lib.
        constexpr std::array<GUID, 10> h264Profiles{{
            {0x1b81be64, 0xa0c7, 0x11d3,
             {0xb9, 0x84, 0x00, 0xc0, 0x4f, 0x2e, 0x73, 0xc5}},
            {0x1b81be65, 0xa0c7, 0x11d3,
             {0xb9, 0x84, 0x00, 0xc0, 0x4f, 0x2e, 0x73, 0xc5}},
            {0x1b81be66, 0xa0c7, 0x11d3,
             {0xb9, 0x84, 0x00, 0xc0, 0x4f, 0x2e, 0x73, 0xc5}},
            {0x1b81be67, 0xa0c7, 0x11d3,
             {0xb9, 0x84, 0x00, 0xc0, 0x4f, 0x2e, 0x73, 0xc5}},
            {0x1b81be68, 0xa0c7, 0x11d3,
             {0xb9, 0x84, 0x00, 0xc0, 0x4f, 0x2e, 0x73, 0xc5}},
            {0x1b81be69, 0xa0c7, 0x11d3,
             {0xb9, 0x84, 0x00, 0xc0, 0x4f, 0x2e, 0x73, 0xc5}},
            {0xd5f04ff9, 0x3418, 0x45d8,
             {0x95, 0x61, 0x32, 0xa7, 0x6a, 0xae, 0x2d, 0xdd}},
            {0xd79be8da, 0x0cf1, 0x4c81,
             {0xb8, 0x2a, 0x69, 0xa4, 0xe2, 0x36, 0xf4, 0x3d}},
            {0xf9aaccbb, 0xc2b6, 0x4cfc,
             {0x87, 0x79, 0x57, 0x07, 0xb1, 0x76, 0x05, 0x52}},
            {0x705b9d82, 0x76cf, 0x49d6,
             {0xb7, 0xe6, 0xac, 0x88, 0x72, 0xdb, 0x01, 0x3c}},
        }};

        const UINT profileCount =
            videoDevice->GetVideoDecoderProfileCount();
        for (UINT index = 0; index < profileCount; ++index) {
            GUID profile = GUID_NULL;
            if (FAILED(videoDevice->GetVideoDecoderProfile(
                    index, &profile))) {
                continue;
            }
            const bool isH264 = std::any_of(
                h264Profiles.begin(),
                h264Profiles.end(),
                [&profile](const GUID& candidate) {
                    return IsEqualGUID(profile, candidate);
                });
            if (!isH264) {
                continue;
            }

            BOOL nv12Supported = FALSE;
            if (FAILED(videoDevice->CheckVideoDecoderFormat(
                    &profile,
                    DXGI_FORMAT_NV12,
                    &nv12Supported)) ||
                nv12Supported == FALSE) {
                continue;
            }

            constexpr UINT32 kMacroblockSize = 16;
            const UINT32 codedWidth =
                (width + kMacroblockSize - 1) &
                ~(kMacroblockSize - 1);
            const UINT32 codedHeight =
                (height + kMacroblockSize - 1) &
                ~(kMacroblockSize - 1);
            const D3D11_VIDEO_DECODER_DESC description{
                profile,
                codedWidth,
                codedHeight,
                DXGI_FORMAT_NV12,
            };
            UINT configurationCount = 0;
            if (SUCCEEDED(videoDevice->GetVideoDecoderConfigCount(
                    &description, &configurationCount)) &&
                configurationCount != 0) {
                return true;
            }
        }
        return false;
    }

    bool CreateAndConfigureTransform(const Settings& settings)
    {
        const MFT_REGISTER_TYPE_INFO inputType{
            MFMediaType_Video,
            MFVideoFormat_H264,
        };
        const MFT_REGISTER_TYPE_INFO outputType{
            MFMediaType_Video,
            MFVideoFormat_NV12,
        };

        IMFActivate** activations = nullptr;
        UINT32 activationCount = 0;
        UINT32 enumerationFlags = MFT_ENUM_FLAG_SORTANDFILTER;
        switch (selection_) {
        case MfD3D11H264DecoderSelection::kHardwareOnly:
            // The Windows H264 decoder is commonly a synchronous MFT that
            // delegates decode operations to the D3D11/DXVA decoder device.
            // It is therefore not necessarily registered as a hardware MFT.
            enumerationFlags |= MFT_ENUM_FLAG_ALL;
            break;
        case MfD3D11H264DecoderSelection::kSoftwareOnly:
            enumerationFlags |=
                MFT_ENUM_FLAG_SYNCMFT | MFT_ENUM_FLAG_LOCALMFT;
            break;
        case MfD3D11H264DecoderSelection::kAny:
        default:
            enumerationFlags |= MFT_ENUM_FLAG_ALL;
            break;
        }

        const HRESULT enumerateResult = MFTEnumEx(
            MFT_CATEGORY_VIDEO_DECODER,
            enumerationFlags,
            &inputType,
            &outputType,
            &activations,
            &activationCount);
        if (FAILED(enumerateResult) || activationCount == 0) {
            lastError_ = FAILED(enumerateResult)
                ? HResultError("MFTEnumEx", enumerateResult)
                : selection_ ==
                          MfD3D11H264DecoderSelection::kHardwareOnly
                      ? "MFTEnumEx found no H264 hardware decoder."
                      : "MFTEnumEx found no H264 to NV12 decoder.";
            ReleaseActivations(activations, activationCount);
            return false;
        }

        bool configured = false;
        std::string candidateError;
        std::vector<UINT32> candidateOrder(activationCount);
        for (UINT32 index = 0; index < activationCount; ++index) {
            candidateOrder[index] = index;
        }
        std::stable_sort(
            candidateOrder.begin(),
            candidateOrder.end(),
            [this, activations](UINT32 left, UINT32 right) {
                const bool leftPreferred =
                    !preferredDecoderName_.empty() &&
                    Narrow(ActivationName(activations[left])) ==
                        preferredDecoderName_;
                const bool rightPreferred =
                    !preferredDecoderName_.empty() &&
                    Narrow(ActivationName(activations[right])) ==
                        preferredDecoderName_;
                if (leftPreferred != rightPreferred) {
                    return leftPreferred;
                }
                return ActivationIsHardware(activations[left]) &&
                       !ActivationIsHardware(activations[right]);
            });

        for (const UINT32 index : candidateOrder) {
            const std::string activationName =
                Narrow(ActivationName(activations[index]));
            if (requirePreferredDecoder_ &&
                activationName != preferredDecoderName_) {
                continue;
            }
            const bool activationIsHardware =
                ActivationIsHardware(activations[index]);
            if (selection_ ==
                    MfD3D11H264DecoderSelection::kSoftwareOnly &&
                activationIsHardware) {
                continue;
            }

            ComPtr<IMFTransform> candidate;
            HRESULT result = activations[index]->ActivateObject(
                IID_PPV_ARGS(&candidate));
            if (FAILED(result)) {
                candidateError = HResultError("IMFActivate::ActivateObject", result);
                continue;
            }

            ComPtr<IMFAttributes> attributes;
            UINT32 d3d11Aware = FALSE;
            result = candidate->GetAttributes(&attributes);
            if (FAILED(result) || FAILED(attributes->GetUINT32(
                                      MF_SA_D3D11_AWARE,
                                      &d3d11Aware)) ||
                d3d11Aware == FALSE) {
                candidateError = "H264 decoder MFT is not D3D11-aware.";
                activations[index]->ShutdownObject();
                continue;
            }

            UINT32 asynchronous = FALSE;
            if (SUCCEEDED(attributes->GetUINT32(
                    MF_TRANSFORM_ASYNC, &asynchronous)) &&
                asynchronous != FALSE) {
                result = attributes->SetUINT32(
                    MF_TRANSFORM_ASYNC_UNLOCK, TRUE);
                if (FAILED(result)) {
                    candidateError = HResultError(
                        "Unlock asynchronous H264 decoder MFT", result);
                    activations[index]->ShutdownObject();
                    continue;
                }
            }

            ComPtr<ICodecAPI> codecApi;
            if (SUCCEEDED(candidate.As(&codecApi)) && codecApi) {
                VARIANT lowLatency;
                VariantInit(&lowLatency);
                // The Microsoft H264 decoder specifically expects VT_UI4
                // for CODECAPI_AVLowLatencyMode.
                lowLatency.vt = VT_UI4;
                lowLatency.ulVal = 1;
                codecApi->SetValue(
                    &CODECAPI_AVLowLatencyMode, &lowLatency);
                VariantClear(&lowLatency);
            }

            result = ConfigureTransform(candidate.Get(), settings);
            if (FAILED(result)) {
                candidateError = HResultError("Configure H264 decoder MFT", result);
                activations[index]->ShutdownObject();
                continue;
            }

            transform_ = std::move(candidate);
            activation_ = activations[index];
            decoderName_ = activationName;
            if (decoderName_.empty()) {
                decoderName_ = "D3D11-aware H264 Decoder MFT";
            }
            decoderKind_ =
                activationIsHardware ||
                        (selection_ !=
                             MfD3D11H264DecoderSelection::kSoftwareOnly &&
                         hardwareDecodeSupported_)
                ? MfD3D11H264DecoderKind::kHardware
                : MfD3D11H264DecoderKind::kSoftware;
            asynchronous_ = asynchronous != FALSE;
            if (decoderKind_ ==
                    MfD3D11H264DecoderKind::kHardware &&
                !d3dAdapterName_.empty()) {
                decoderName_ += " | DXVA: " + d3dAdapterName_;
            }
            configured = true;
            break;
        }

        ReleaseActivations(activations, activationCount);
        if (!configured) {
            lastError_ = candidateError.empty()
                ? "No usable D3D11-aware H264 decoder MFT was found."
                : std::move(candidateError);
        }
        return configured;
    }

    HRESULT ConfigureTransform(IMFTransform* transform,
                               const Settings& settings)
    {
        HRESULT result = transform->GetStreamIDs(
            1, &inputStreamId_, 1, &outputStreamId_);
        if (result == E_NOTIMPL) {
            inputStreamId_ = 0;
            outputStreamId_ = 0;
            result = S_OK;
        }
        if (FAILED(result)) {
            return result;
        }

        result = transform->ProcessMessage(
            MFT_MESSAGE_SET_D3D_MANAGER,
            reinterpret_cast<ULONG_PTR>(dxgiDeviceManager_.Get()));
        if (FAILED(result)) {
            return result;
        }

        ComPtr<IMFAttributes> attributes;
        if (SUCCEEDED(transform->GetAttributes(&attributes))) {
            attributes->SetUINT32(MF_LOW_LATENCY, TRUE);
            UINT32 configuredSurfaceCount = 0;
            if (FAILED(attributes->GetUINT32(
                    MF_SA_MINIMUM_OUTPUT_SAMPLE_COUNT,
                    &configuredSurfaceCount)) ||
                configuredSurfaceCount <
                    kMinimumLowLatencyOutputSurfaces) {
                (void)attributes->SetUINT32(
                    MF_SA_MINIMUM_OUTPUT_SAMPLE_COUNT,
                    kMinimumLowLatencyOutputSurfaces);
            }
            configuredSurfaceCount = 0;
            if (FAILED(attributes->GetUINT32(
                    MF_SA_MINIMUM_OUTPUT_SAMPLE_COUNT_PROGRESSIVE,
                    &configuredSurfaceCount)) ||
                configuredSurfaceCount <
                    kMinimumLowLatencyOutputSurfaces) {
                (void)attributes->SetUINT32(
                    MF_SA_MINIMUM_OUTPUT_SAMPLE_COUNT_PROGRESSIVE,
                    kMinimumLowLatencyOutputSurfaces);
            }
        }

        // Microsoft documents the output-stream attribute as the contract a
        // D3D11 decoder uses to size its renderer-facing surface pool. Some
        // inbox/driver MFTs read the transform attribute, others the stream
        // attribute, so publish the same lower bound to both stores.
        ComPtr<IMFAttributes> outputAttributes;
        if (SUCCEEDED(transform->GetOutputStreamAttributes(
                outputStreamId_, &outputAttributes))) {
            UINT32 configuredSurfaceCount = 0;
            if (FAILED(outputAttributes->GetUINT32(
                    MF_SA_MINIMUM_OUTPUT_SAMPLE_COUNT,
                    &configuredSurfaceCount)) ||
                configuredSurfaceCount <
                    kMinimumLowLatencyOutputSurfaces) {
                (void)outputAttributes->SetUINT32(
                    MF_SA_MINIMUM_OUTPUT_SAMPLE_COUNT,
                    kMinimumLowLatencyOutputSurfaces);
            }
            configuredSurfaceCount = 0;
            if (FAILED(outputAttributes->GetUINT32(
                    MF_SA_MINIMUM_OUTPUT_SAMPLE_COUNT_PROGRESSIVE,
                    &configuredSurfaceCount)) ||
                configuredSurfaceCount <
                    kMinimumLowLatencyOutputSurfaces) {
                (void)outputAttributes->SetUINT32(
                    MF_SA_MINIMUM_OUTPUT_SAMPLE_COUNT_PROGRESSIVE,
                    kMinimumLowLatencyOutputSurfaces);
            }
        }

        const auto maxResolution = settings.max_render_resolution();
        const UINT32 width = maxResolution.Valid()
            ? static_cast<UINT32>(maxResolution.Width())
            : kDefaultProbeWidth;
        const UINT32 height = maxResolution.Valid()
            ? static_cast<UINT32>(maxResolution.Height())
            : kDefaultProbeHeight;

        ComPtr<IMFMediaType> inputMediaType;
        result = MFCreateMediaType(&inputMediaType);
        if (SUCCEEDED(result)) {
            result = inputMediaType->SetGUID(
                MF_MT_MAJOR_TYPE, MFMediaType_Video);
        }
        if (SUCCEEDED(result)) {
            result = inputMediaType->SetGUID(
                MF_MT_SUBTYPE, MFVideoFormat_H264_ES);
        }
        if (SUCCEEDED(result)) {
            result = MFSetAttributeSize(
                inputMediaType.Get(), MF_MT_FRAME_SIZE, width, height);
        }
        if (SUCCEEDED(result)) {
            result = inputMediaType->SetUINT32(
                MF_MT_INTERLACE_MODE, MFVideoInterlace_Progressive);
        }
        if (SUCCEEDED(result)) {
            result = transform->SetInputType(
                inputStreamId_, inputMediaType.Get(), 0);
        }
        if (FAILED(result)) {
            return result;
        }

        return ConfigureOutputType(transform);
    }

    HRESULT ConfigureOutputType(IMFTransform* transform)
    {
        for (DWORD typeIndex = 0;; ++typeIndex) {
            ComPtr<IMFMediaType> availableType;
            HRESULT result = transform->GetOutputAvailableType(
                outputStreamId_, typeIndex, &availableType);
            if (result == MF_E_NO_MORE_TYPES) {
                break;
            }
            if (FAILED(result)) {
                continue;
            }

            GUID subtype = GUID_NULL;
            if (SUCCEEDED(availableType->GetGUID(MF_MT_SUBTYPE, &subtype)) &&
                subtype == MFVideoFormat_NV12 &&
                SUCCEEDED(transform->SetOutputType(
                    outputStreamId_, availableType.Get(), 0))) {
                HRESULT dimensionsResult = UpdateOutputDimensions(transform);
                if (FAILED(dimensionsResult)) {
                    return dimensionsResult;
                }

                MFT_OUTPUT_STREAM_INFO streamInfo{};
                HRESULT streamInfoResult = transform->GetOutputStreamInfo(
                    outputStreamId_, &streamInfo);
                if (SUCCEEDED(streamInfoResult)) {
                    supportsNativeOutputSamples_ =
                        (streamInfo.dwFlags &
                         (MFT_OUTPUT_STREAM_PROVIDES_SAMPLES |
                          MFT_OUTPUT_STREAM_CAN_PROVIDE_SAMPLES)) != 0;
                }
                return streamInfoResult;
            }
        }
        return MF_E_INVALIDMEDIATYPE;
    }

    HRESULT UpdateOutputDimensions(IMFTransform* transform)
    {
        ComPtr<IMFMediaType> currentType;
        HRESULT result = transform->GetOutputCurrentType(
            outputStreamId_, &currentType);
        UINT32 width = 0;
        UINT32 height = 0;
        if (SUCCEEDED(result)) {
            result = MFGetAttributeSize(
                currentType.Get(), MF_MT_FRAME_SIZE, &width, &height);
        }
        if (SUCCEEDED(result)) {
            // H264 decoders commonly expose a macroblock-aligned coded size
            // (for example 320x192 for a 320x180 picture). Prefer the display
            // aperture so the WebRTC frame and renderer use the visible size.
            const std::array<const GUID*, 2> apertureAttributes{
                &MF_MT_MINIMUM_DISPLAY_APERTURE,
                &MF_MT_GEOMETRIC_APERTURE,
            };
            for (const auto* attribute : apertureAttributes) {
                MFVideoArea area{};
                UINT32 blobSize = 0;
                if (SUCCEEDED(currentType->GetBlob(
                        *attribute,
                        reinterpret_cast<UINT8*>(&area),
                        sizeof(area),
                        &blobSize)) &&
                    blobSize >= sizeof(area) && area.Area.cx > 0 &&
                    area.Area.cy > 0) {
                    width = static_cast<UINT32>(area.Area.cx);
                    height = static_cast<UINT32>(area.Area.cy);
                    break;
                }
            }
        }
        if (SUCCEEDED(result)) {
            visibleWidth_ = static_cast<int>(width);
            visibleHeight_ = static_cast<int>(height);
        }
        return result;
    }

    HRESULT CreateInputSample(
        const webrtc::EncodedImage& inputImage,
        int64_t renderTimeMs,
        IMFSample** sample,
        FrameMetadata& metadata)
    {
        if (!sample || !inputImage.data() || inputImage.size() == 0 ||
            inputImage.size() > MAXDWORD) {
            return E_INVALIDARG;
        }
        *sample = nullptr;

        ComPtr<IMFMediaBuffer> buffer;
        HRESULT result = MFCreateMemoryBuffer(
            static_cast<DWORD>(inputImage.size()), &buffer);
        BYTE* destination = nullptr;
        if (SUCCEEDED(result)) {
            result = buffer->Lock(&destination, nullptr, nullptr);
        }
        if (SUCCEEDED(result)) {
            std::memcpy(destination, inputImage.data(), inputImage.size());
            result = buffer->Unlock();
            destination = nullptr;
        }
        if (destination) {
            buffer->Unlock();
        }
        if (SUCCEEDED(result)) {
            result = buffer->SetCurrentLength(
                static_cast<DWORD>(inputImage.size()));
        }

        ComPtr<IMFSample> inputSample;
        if (SUCCEEDED(result)) {
            result = MFCreateSample(&inputSample);
        }
        if (SUCCEEDED(result)) {
            result = inputSample->AddBuffer(buffer.Get());
        }

        constexpr std::int64_t kRtpVideoClockHz = 90'000;
        constexpr std::int64_t kHundredNanosecondsPerSecond = 10'000'000;
        const auto toHundredNanoseconds = [](std::int64_t rtpTicks) {
            return static_cast<LONGLONG>(
                (rtpTicks / kRtpVideoClockHz) *
                    kHundredNanosecondsPerSecond +
                (rtpTicks % kRtpVideoClockHz) *
                    kHundredNanosecondsPerSecond /
                    kRtpVideoClockHz);
        };

        const std::int64_t unwrappedRtpTimestamp =
            rtpTimestampUnwrapper_.Unwrap(inputImage.RtpTimestamp());
        if (!baseRtpTimestamp_) {
            baseRtpTimestamp_ = unwrappedRtpTimestamp;
        }
        const LONGLONG sampleTime = toHundredNanoseconds(
            unwrappedRtpTimestamp - *baseRtpTimestamp_);
        if (SUCCEEDED(result)) {
            result = inputSample->SetSampleTime(sampleTime);
        }
        if (SUCCEEDED(result) && previousRtpTimestamp_ &&
            unwrappedRtpTimestamp > *previousRtpTimestamp_) {
            const LONGLONG sampleDuration100ns =
                toHundredNanoseconds(
                    unwrappedRtpTimestamp - *previousRtpTimestamp_);
            if (sampleDuration100ns > 0) {
                result = inputSample->SetSampleDuration(
                    sampleDuration100ns);
            }
        }
        if (SUCCEEDED(result) && inputImage.IsKey()) {
            inputSample->SetUINT32(MFSampleExtension_CleanPoint, TRUE);
        }
        if (FAILED(result)) {
            return result;
        }

        previousRtpTimestamp_ = unwrappedRtpTimestamp;
        metadata.sampleTime100ns = sampleTime;
        metadata.rtpTimestamp = inputImage.RtpTimestamp();
        metadata.renderTimeMs = renderTimeMs;
        metadata.ntpTimeMs = inputImage.NtpTimeMs();
        metadata.rotation = inputImage.rotation();
        metadata.packetInfos = inputImage.PacketInfos();
        if (inputImage.ColorSpace()) {
            metadata.colorSpace = *inputImage.ColorSpace();
        }

        *sample = inputSample.Detach();
        return S_OK;
    }

    bool StartAsyncEventLoop()
    {
        if (!asynchronous_ || !transform_) {
            return true;
        }

        HRESULT result = transform_.As(&mediaEventGenerator_);
        if (FAILED(result) || !mediaEventGenerator_) {
            lastError_ = HResultError(
                "Query IMFMediaEventGenerator from hardware decoder",
                result);
            return false;
        }

        asyncStopRequested_.store(false, std::memory_order_release);
        asyncFailed_.store(false, std::memory_order_release);
        try {
            asyncEventThread_ = std::thread([this] {
                AsyncEventLoop();
            });
        } catch (const std::exception& exception) {
            lastError_ =
                std::string("Failed to start the hardware decoder event "
                            "thread: ") +
                exception.what();
            mediaEventGenerator_.Reset();
            return false;
        }
        return true;
    }

    void StopAsyncEventLoop()
    {
        asyncStopRequested_.store(true, std::memory_order_release);
        asyncCondition_.notify_all();
        if (asyncEventThread_.joinable()) {
            asyncEventThread_.join();
        }
        mediaEventGenerator_.Reset();
    }

    void FailAsyncDecoder(const std::string& error)
    {
        lastError_ = error;
        asyncFailed_.store(true, std::memory_order_release);
        asyncCondition_.notify_all();
    }

    void SubmitQueuedAsyncInputs()
    {
        while (!asyncStopRequested_.load(std::memory_order_acquire)) {
            QueuedInput input;
            {
                std::lock_guard lock(asyncMutex_);
                if (pendingInputRequests_ == 0 ||
                    queuedInputs_.empty()) {
                    return;
                }
                --pendingInputRequests_;
                input = std::move(queuedInputs_.front());
                queuedInputs_.pop_front();
                queuedInputDepth_.store(
                    static_cast<std::uint32_t>(queuedInputs_.size()),
                    std::memory_order_relaxed);
            }

            const HRESULT result = transform_->ProcessInput(
                inputStreamId_, input.sample.Get(), 0);
            if (FAILED(result)) {
                FailAsyncDecoder(HResultError(
                    "Asynchronous IMFTransform::ProcessInput", result));
                return;
            }
            input.metadata.submittedAtUs =
                VideoCodecTimingRegistry::SteadyNowUs();
            // Only the event thread consumes pendingFrames_ for an
            // asynchronous MFT, so the metadata queue remains ordered without
            // holding asyncMutex_ while ProcessOutput invokes WebRTC.
            pendingFrames_.push_back(std::move(input.metadata));
            inFlightFrameDepth_.store(
                static_cast<std::uint32_t>(pendingFrames_.size()),
                std::memory_order_relaxed);
            UpdatePeakBacklog();
        }
    }

    void ProcessAsyncOutput()
    {
        while (!asyncStopRequested_.load(std::memory_order_acquire)) {
            bool delivered = false;
            HRESULT result = ReadOneOutput(delivered);
            if (result == MF_E_TRANSFORM_STREAM_CHANGE) {
                result = ConfigureOutputType(transform_.Get());
                if (SUCCEEDED(result)) {
                    continue;
                }
            }
            if (result == MF_E_TRANSFORM_NEED_MORE_INPUT) {
                return;
            }
            if (FAILED(result)) {
                FailAsyncDecoder(HResultError(
                    "Asynchronous IMFTransform::ProcessOutput", result));
            }
            return;
        }
    }

    void AsyncEventLoop()
    {
        const HRESULT comResult =
            CoInitializeEx(nullptr, COINIT_MULTITHREADED);
        const bool uninitializeCom = SUCCEEDED(comResult);

        while (!asyncStopRequested_.load(std::memory_order_acquire)) {
            SubmitQueuedAsyncInputs();
            if (asyncFailed_.load(std::memory_order_acquire)) {
                break;
            }

            ComPtr<IMFMediaEvent> event;
            const HRESULT result = mediaEventGenerator_->GetEvent(
                MF_EVENT_FLAG_NO_WAIT, &event);
            if (result == MF_E_NO_EVENTS_AVAILABLE) {
                std::unique_lock lock(asyncMutex_);
                asyncCondition_.wait_for(
                    lock,
                    std::chrono::milliseconds(2),
                    [this] {
                        return asyncStopRequested_.load(
                                   std::memory_order_acquire) ||
                               (!queuedInputs_.empty() &&
                                pendingInputRequests_ != 0);
                    });
                continue;
            }
            if (FAILED(result)) {
                if (!asyncStopRequested_.load(
                        std::memory_order_acquire)) {
                    FailAsyncDecoder(HResultError(
                        "IMFMediaEventGenerator::GetEvent", result));
                }
                break;
            }

            HRESULT eventStatus = S_OK;
            const HRESULT statusResult =
                event->GetStatus(&eventStatus);
            if (FAILED(statusResult)) {
                FailAsyncDecoder(HResultError(
                    "Read asynchronous H264 decoder event status",
                    statusResult));
                break;
            }
            if (FAILED(eventStatus)) {
                FailAsyncDecoder(HResultError(
                    "Asynchronous H264 decoder event", eventStatus));
                break;
            }

            MediaEventType eventType = MEUnknown;
            if (FAILED(event->GetType(&eventType))) {
                FailAsyncDecoder(
                    "The asynchronous H264 decoder returned an event "
                    "without a valid type.");
                break;
            }

            switch (eventType) {
            case METransformNeedInput:
                {
                    std::lock_guard lock(asyncMutex_);
                    ++pendingInputRequests_;
                }
                SubmitQueuedAsyncInputs();
                break;
            case METransformHaveOutput:
                ProcessAsyncOutput();
                break;
            case MEError:
                FailAsyncDecoder(
                    "The asynchronous H264 decoder reported MEError.");
                break;
            default:
                break;
            }
        }

        if (uninitializeCom) {
            CoUninitialize();
        }
    }

    HRESULT DrainOutput(int& outputCount)
    {
        while (true) {
            bool delivered = false;
            HRESULT result = ReadOneOutput(delivered);
            if (result == MF_E_TRANSFORM_NEED_MORE_INPUT) {
                return S_OK;
            }
            if (result == MF_E_TRANSFORM_STREAM_CHANGE) {
                result = ConfigureOutputType(transform_.Get());
                if (FAILED(result)) {
                    return result;
                }
                continue;
            }
            if (FAILED(result)) {
                return result;
            }
            if (delivered) {
                ++outputCount;
            }
        }
    }

    HRESULT ReadOneOutput(bool& delivered)
    {
        delivered = false;
        MFT_OUTPUT_STREAM_INFO streamInfo{};
        HRESULT result = transform_->GetOutputStreamInfo(
            outputStreamId_, &streamInfo);
        if (FAILED(result)) {
            return result;
        }

        // With a DXGI device manager the decoder must provide, or be able to
        // provide, its allocator-backed sample. A plain memory sample here
        // would discard the native D3D11 surface path.
        if ((streamInfo.dwFlags &
             (MFT_OUTPUT_STREAM_PROVIDES_SAMPLES |
              MFT_OUTPUT_STREAM_CAN_PROVIDE_SAMPLES)) == 0) {
            return E_NOTIMPL;
        }

        MFT_OUTPUT_DATA_BUFFER output{};
        output.dwStreamID = outputStreamId_;
        DWORD status = 0;
        result = transform_->ProcessOutput(0, 1, &output, &status);
        const std::int64_t outputReadyAtUs =
            VideoCodecTimingRegistry::SteadyNowUs();

        ComPtr<IMFCollection> events;
        events.Attach(output.pEvents);
        if (FAILED(result)) {
            if (output.pSample) {
                output.pSample->Release();
            }
            return result;
        }
        if (!output.pSample) {
            return E_UNEXPECTED;
        }

        ComPtr<IMFSample> outputSample;
        outputSample.Attach(output.pSample);
        ComPtr<IMFMediaBuffer> mediaBuffer;
        result = outputSample->GetBufferByIndex(0, &mediaBuffer);
        if (FAILED(result)) {
            return result;
        }

        ComPtr<IMFDXGIBuffer> dxgiBuffer;
        result = mediaBuffer.As(&dxgiBuffer);
        if (FAILED(result)) {
            return result;
        }

        ComPtr<ID3D11Texture2D> texture;
        result = dxgiBuffer->GetResource(IID_PPV_ARGS(&texture));
        UINT subresourceIndex = 0;
        if (SUCCEEDED(result)) {
            result = dxgiBuffer->GetSubresourceIndex(&subresourceIndex);
        }
        if (FAILED(result)) {
            return result;
        }

        D3D11_TEXTURE2D_DESC description{};
        texture->GetDesc(&description);
        if (description.Format != DXGI_FORMAT_NV12 ||
            visibleWidth_ <= 0 || visibleHeight_ <= 0) {
            return MF_E_INVALIDMEDIATYPE;
        }

        LONGLONG outputTime = 0;
        outputSample->GetSampleTime(&outputTime);
        FrameMetadata metadata = TakeFrameMetadata(outputTime);

        auto nativeBuffer =
            webrtc::make_ref_counted<D3D11NativeFrameBuffer>(
                outputSample.Get(),
                texture.Get(),
                subresourceIndex,
                visibleWidth_,
                visibleHeight_);
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
            timing.asynchronous = asynchronous_;
            timing.inputPreparationUs =
                static_cast<std::uint64_t>(
                    metadata.inputReadyAtUs -
                    metadata.decodeStartedAtUs);
            timing.inputQueueWaitUs =
                static_cast<std::uint64_t>(
                    metadata.submittedAtUs -
                    metadata.inputReadyAtUs);
            timing.transformWaitUs =
                static_cast<std::uint64_t>(
                    outputReadyAtUs - metadata.submittedAtUs);
            timing.outputDeliveryUs =
                static_cast<std::uint64_t>(
                    callbackAtUs - outputReadyAtUs);
            timing.queuedInputFrames = queuedInputDepth_.load(
                std::memory_order_relaxed);
            timing.inFlightFrames = inFlightFrameDepth_.load(
                std::memory_order_relaxed);
            timing.peakBacklogFrames = peakBacklogFrames_.load(
                std::memory_order_relaxed);
            timing.backpressureFallbacks =
                backpressureFallbacks_.load(std::memory_order_relaxed);
            VideoCodecTimingRegistry::StageDecodePipelineTiming(
                metadata.rtpTimestamp, timing);
        }

        // Let WebRTC match the output RTP timestamp to its high-resolution
        // decode-start timestamp. This is both more precise than GetTickCount64
        // and correct when an MFT pipelines input and output frames.
        callback_->Decoded(frame, std::nullopt, std::nullopt);
        delivered = true;
        return S_OK;
    }

    FrameMetadata TakeFrameMetadata(LONGLONG outputTime)
    {
        if (pendingFrames_.empty()) {
            return {};
        }

        const auto match = std::find_if(
            pendingFrames_.begin(),
            pendingFrames_.end(),
            [outputTime](const FrameMetadata& metadata) {
                return metadata.sampleTime100ns == outputTime;
            });
        if (match == pendingFrames_.end()) {
            FrameMetadata metadata = std::move(pendingFrames_.front());
            pendingFrames_.pop_front();
            inFlightFrameDepth_.store(
                static_cast<std::uint32_t>(pendingFrames_.size()),
                std::memory_order_relaxed);
            return metadata;
        }

        FrameMetadata metadata = std::move(*match);
        pendingFrames_.erase(match);
        inFlightFrameDepth_.store(
            static_cast<std::uint32_t>(pendingFrames_.size()),
            std::memory_order_relaxed);
        return metadata;
    }

    const MfD3D11H264DecoderSelection selection_;
    const std::string preferredDecoderName_;
    const bool requirePreferredDecoder_ = false;
    bool configured_ = false;
    bool mediaFoundationStarted_ = false;
    bool shouldUninitializeCom_ = false;
    DWORD inputStreamId_ = 0;
    DWORD outputStreamId_ = 0;
    UINT resetToken_ = 0;
    int visibleWidth_ = 0;
    int visibleHeight_ = 0;
    bool supportsNativeOutputSamples_ = false;
    bool asynchronous_ = false;
    bool hardwareDecodeSupported_ = false;
    MfD3D11H264DecoderKind decoderKind_ =
        MfD3D11H264DecoderKind::kUnknown;
    webrtc::RtpTimestampUnwrapper rtpTimestampUnwrapper_;
    std::optional<std::int64_t> baseRtpTimestamp_;
    std::optional<std::int64_t> previousRtpTimestamp_;
    webrtc::DecodedImageCallback* callback_ = nullptr;
    std::deque<FrameMetadata> pendingFrames_;
    std::mutex asyncMutex_;
    std::condition_variable asyncCondition_;
    std::deque<QueuedInput> queuedInputs_;
    std::size_t pendingInputRequests_ = 0;
    std::atomic_bool asyncStopRequested_{false};
    std::atomic_bool asyncFailed_{false};
    std::atomic<std::uint32_t> queuedInputDepth_{0};
    std::atomic<std::uint32_t> inFlightFrameDepth_{0};
    std::atomic<std::uint32_t> peakBacklogFrames_{0};
    std::atomic<std::uint64_t> backpressureFallbacks_{0};
    std::thread asyncEventThread_;
    std::string lastError_;
    std::string decoderName_;
    std::string d3dAdapterName_;
    ComPtr<ID3D11Device> d3dDevice_;
    ComPtr<ID3D11DeviceContext> d3dContext_;
    ComPtr<IMFDXGIDeviceManager> dxgiDeviceManager_;
    ComPtr<IMFActivate> activation_;
    ComPtr<IMFTransform> transform_;
    ComPtr<IMFMediaEventGenerator> mediaEventGenerator_;
};

MfD3D11H264Decoder::MfD3D11H264Decoder(
    MfD3D11H264DecoderSelection selection,
    std::string preferredDecoderName,
    bool requirePreferredDecoder)
    : impl_(std::make_unique<Impl>(
          selection,
          std::move(preferredDecoderName),
          requirePreferredDecoder))
{}

MfD3D11H264Decoder::~MfD3D11H264Decoder()
{
    Release();
}

bool MfD3D11H264Decoder::Configure(const Settings& settings)
{
    return impl_->Configure(settings);
}

int32_t MfD3D11H264Decoder::Decode(
    const webrtc::EncodedImage& inputImage,
    int64_t renderTimeMs)
{
    return impl_->Decode(inputImage, renderTimeMs);
}

int32_t MfD3D11H264Decoder::RegisterDecodeCompleteCallback(
    webrtc::DecodedImageCallback* callback)
{
    return impl_->RegisterDecodeCompleteCallback(callback);
}

int32_t MfD3D11H264Decoder::Release()
{
    return impl_->Release();
}

webrtc::VideoDecoder::DecoderInfo MfD3D11H264Decoder::GetDecoderInfo() const
{
    return {
        .implementation_name = ImplementationName(),
        .is_hardware_accelerated = impl_->IsHardwareAccelerated(),
    };
}

const char* MfD3D11H264Decoder::ImplementationName() const
{
    switch (impl_->DecoderKind()) {
    case MfD3D11H264DecoderKind::kHardware:
        return "MediaFoundation D3D11/DXVA H264 Hardware Decoder";
    case MfD3D11H264DecoderKind::kSoftware:
        return "MediaFoundation D3D11 H264 Software Decoder";
    case MfD3D11H264DecoderKind::kUnknown:
    default:
        return "MediaFoundation D3D11 H264 Decoder";
    }
}

bool MfD3D11H264Decoder::IsConfigured() const noexcept
{
    return impl_->IsConfigured();
}

bool MfD3D11H264Decoder::SupportsNativeOutputSamples() const noexcept
{
    return impl_->SupportsNativeOutputSamples();
}

MfD3D11H264DecoderKind
MfD3D11H264Decoder::DecoderKind() const noexcept
{
    return impl_->DecoderKind();
}

bool MfD3D11H264Decoder::IsHardwareAccelerated() const noexcept
{
    return impl_->IsHardwareAccelerated();
}

bool MfD3D11H264Decoder::IsAsynchronous() const noexcept
{
    return impl_->IsAsynchronous();
}

const std::string& MfD3D11H264Decoder::LastError() const noexcept
{
    return impl_->LastError();
}

const std::string& MfD3D11H264Decoder::DecoderName() const noexcept
{
    return impl_->DecoderName();
}

}  // namespace remote
