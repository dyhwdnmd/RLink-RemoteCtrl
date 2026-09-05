// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#include "MfH264EncoderSelfTest.h"

#include <algorithm>
#include <chrono>
#include <cstdint>
#include <iomanip>
#include <iterator>
#include <limits>
#include <sstream>
#include <thread>
#include <vector>

#include <Windows.h>
#include <strmif.h>
#include <codecapi.h>
#include <d3d11.h>
#include <mfapi.h>
#include <mferror.h>
#include <mfidl.h>
#include <mftransform.h>
#include <wrl/client.h>

namespace remote {
namespace {

using Microsoft::WRL::ComPtr;

constexpr uint32_t kWidth = 1920;
constexpr uint32_t kHeight = 1080;
constexpr uint32_t kFrameRate = 30;
constexpr uint32_t kFrameCount = 90;
constexpr uint32_t kInitialBitrate = 4'000'000;
constexpr uint32_t kAdjustedBitrate = 2'500'000;
constexpr LONGLONG kFrameDuration100ns = 10'000'000LL / kFrameRate;
constexpr auto kTestTimeout = std::chrono::seconds(30);

std::string HResultMessage(const char* operation, HRESULT result)
{
    std::ostringstream stream;
    stream << operation << " failed with HRESULT 0x" << std::hex
           << std::uppercase << static_cast<uint32_t>(result) << '.';
    return stream.str();
}

std::string WideToUtf8(const wchar_t* text, size_t length)
{
    if (!text || length == 0) {
        return {};
    }
    const int sourceLength =
        length > static_cast<size_t>((std::numeric_limits<int>::max)())
            ? (std::numeric_limits<int>::max)()
            : static_cast<int>(length);
    const int required = WideCharToMultiByte(
        CP_UTF8, 0, text, sourceLength, nullptr, 0, nullptr, nullptr);
    if (required <= 0) {
        return {};
    }
    std::string value(static_cast<size_t>(required), '\0');
    WideCharToMultiByte(CP_UTF8,
                        0,
                        text,
                        sourceLength,
                        value.data(),
                        required,
                        nullptr,
                        nullptr);
    return value;
}

bool ReadBooleanAttribute(IMFAttributes* attributes, REFGUID key)
{
    UINT32 value = FALSE;
    return attributes &&
           SUCCEEDED(attributes->GetUINT32(key, &value)) &&
           value != FALSE;
}

bool HasAnnexBStartCode(const uint8_t* bytes, size_t length)
{
    if (!bytes || length < 4) {
        return false;
    }
    for (size_t index = 0; index + 3 < length; ++index) {
        if (bytes[index] == 0 && bytes[index + 1] == 0 &&
            (bytes[index + 2] == 1 ||
             (bytes[index + 2] == 0 && bytes[index + 3] == 1))) {
            return true;
        }
    }
    return false;
}

ComPtr<IMFMediaType> CreateVideoType(REFGUID subtype)
{
    ComPtr<IMFMediaType> type;
    if (FAILED(MFCreateMediaType(&type)) ||
        FAILED(type->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Video)) ||
        FAILED(type->SetGUID(MF_MT_SUBTYPE, subtype)) ||
        FAILED(MFSetAttributeSize(
            type.Get(), MF_MT_FRAME_SIZE, kWidth, kHeight)) ||
        FAILED(MFSetAttributeRatio(
            type.Get(), MF_MT_FRAME_RATE, kFrameRate, 1)) ||
        FAILED(MFSetAttributeRatio(
            type.Get(), MF_MT_PIXEL_ASPECT_RATIO, 1, 1)) ||
        FAILED(type->SetUINT32(
            MF_MT_INTERLACE_MODE,
            MFVideoInterlace_Progressive))) {
        return nullptr;
    }
    return type;
}

class D3D11Nv12FrameGenerator final {
public:
    bool Initialize(std::string& error)
    {
        constexpr D3D_FEATURE_LEVEL featureLevels[] = {
            D3D_FEATURE_LEVEL_11_1,
            D3D_FEATURE_LEVEL_11_0,
            D3D_FEATURE_LEVEL_10_1,
            D3D_FEATURE_LEVEL_10_0,
        };
        D3D_FEATURE_LEVEL selectedFeatureLevel = D3D_FEATURE_LEVEL_10_0;
        HRESULT result = D3D11CreateDevice(
            nullptr,
            D3D_DRIVER_TYPE_HARDWARE,
            nullptr,
            D3D11_CREATE_DEVICE_BGRA_SUPPORT |
                D3D11_CREATE_DEVICE_VIDEO_SUPPORT,
            featureLevels,
            static_cast<UINT>(std::size(featureLevels)),
            D3D11_SDK_VERSION,
            &device_,
            &selectedFeatureLevel,
            &context_);
        if (result == E_INVALIDARG) {
            result = D3D11CreateDevice(
                nullptr,
                D3D_DRIVER_TYPE_HARDWARE,
                nullptr,
                D3D11_CREATE_DEVICE_BGRA_SUPPORT |
                    D3D11_CREATE_DEVICE_VIDEO_SUPPORT,
                featureLevels + 1,
                static_cast<UINT>(std::size(featureLevels) - 1),
                D3D11_SDK_VERSION,
                &device_,
                &selectedFeatureLevel,
                &context_);
        }
        if (FAILED(result)) {
            error = HResultMessage("D3D11CreateDevice", result);
            return false;
        }
        if (FAILED(device_.As(&videoDevice_)) ||
            FAILED(context_.As(&videoContext_))) {
            error = "The D3D11 device does not expose video processing.";
            return false;
        }

        UINT resetToken = 0;
        result = MFCreateDXGIDeviceManager(
            &resetToken, &deviceManager_);
        if (FAILED(result) ||
            FAILED(deviceManager_->ResetDevice(
                device_.Get(), resetToken))) {
            error = HResultMessage(
                "MFCreateDXGIDeviceManager/ResetDevice", result);
            return false;
        }

        D3D11_TEXTURE2D_DESC inputDescription{};
        inputDescription.Width = kWidth;
        inputDescription.Height = kHeight;
        inputDescription.MipLevels = 1;
        inputDescription.ArraySize = 1;
        inputDescription.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
        inputDescription.SampleDesc.Count = 1;
        inputDescription.Usage = D3D11_USAGE_DEFAULT;
        inputDescription.BindFlags =
            D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
        result = device_->CreateTexture2D(
            &inputDescription, nullptr, &bgraTexture_);
        if (FAILED(result)) {
            error = HResultMessage(
                "CreateTexture2D(BGRA)", result);
            return false;
        }

        D3D11_VIDEO_PROCESSOR_CONTENT_DESC content{};
        content.InputFrameFormat =
            D3D11_VIDEO_FRAME_FORMAT_PROGRESSIVE;
        content.InputFrameRate.Numerator = kFrameRate;
        content.InputFrameRate.Denominator = 1;
        content.InputWidth = kWidth;
        content.InputHeight = kHeight;
        content.OutputFrameRate.Numerator = kFrameRate;
        content.OutputFrameRate.Denominator = 1;
        content.OutputWidth = kWidth;
        content.OutputHeight = kHeight;
        content.Usage = D3D11_VIDEO_USAGE_OPTIMAL_SPEED;
        result = videoDevice_->CreateVideoProcessorEnumerator(
            &content, &processorEnumerator_);
        if (FAILED(result)) {
            error = HResultMessage(
                "CreateVideoProcessorEnumerator", result);
            return false;
        }

        UINT formatFlags = 0;
        result = processorEnumerator_->CheckVideoProcessorFormat(
            DXGI_FORMAT_NV12, &formatFlags);
        if (FAILED(result) ||
            (formatFlags &
             D3D11_VIDEO_PROCESSOR_FORMAT_SUPPORT_OUTPUT) == 0) {
            error = "The D3D11 video processor cannot output NV12.";
            return false;
        }
        result = videoDevice_->CreateVideoProcessor(
            processorEnumerator_.Get(), 0, &processor_);
        if (FAILED(result)) {
            error = HResultMessage(
                "CreateVideoProcessor", result);
            return false;
        }

        D3D11_VIDEO_PROCESSOR_INPUT_VIEW_DESC inputViewDescription{};
        inputViewDescription.ViewDimension =
            D3D11_VPIV_DIMENSION_TEXTURE2D;
        inputViewDescription.Texture2D.MipSlice = 0;
        inputViewDescription.Texture2D.ArraySlice = 0;
        result = videoDevice_->CreateVideoProcessorInputView(
            bgraTexture_.Get(),
            processorEnumerator_.Get(),
            &inputViewDescription,
            &inputView_);
        if (FAILED(result)) {
            error = HResultMessage(
                "CreateVideoProcessorInputView", result);
            return false;
        }

        const RECT frameRectangle{
            0, 0, static_cast<LONG>(kWidth), static_cast<LONG>(kHeight)};
        videoContext_->VideoProcessorSetStreamFrameFormat(
            processor_.Get(),
            0,
            D3D11_VIDEO_FRAME_FORMAT_PROGRESSIVE);
        videoContext_->VideoProcessorSetStreamSourceRect(
            processor_.Get(), 0, TRUE, &frameRectangle);
        videoContext_->VideoProcessorSetStreamDestRect(
            processor_.Get(), 0, TRUE, &frameRectangle);
        videoContext_->VideoProcessorSetOutputTargetRect(
            processor_.Get(), TRUE, &frameRectangle);
        videoContext_->VideoProcessorSetStreamAutoProcessingMode(
            processor_.Get(), 0, FALSE);

        bgraPixels_.resize(
            static_cast<size_t>(kWidth) * kHeight * 4);
        return true;
    }

    ComPtr<IMFDXGIDeviceManager> DeviceManager() const
    {
        return deviceManager_;
    }

    ComPtr<IMFSample> CreateFrameSample(uint32_t frameIndex,
                                        LONGLONG sampleTime,
                                        std::string& error)
    {
        FillBgraPattern(frameIndex);
        context_->UpdateSubresource(
            bgraTexture_.Get(),
            0,
            nullptr,
            bgraPixels_.data(),
            kWidth * 4,
            0);

        D3D11_TEXTURE2D_DESC outputDescription{};
        outputDescription.Width = kWidth;
        outputDescription.Height = kHeight;
        outputDescription.MipLevels = 1;
        outputDescription.ArraySize = 1;
        outputDescription.Format = DXGI_FORMAT_NV12;
        outputDescription.SampleDesc.Count = 1;
        outputDescription.Usage = D3D11_USAGE_DEFAULT;
        outputDescription.BindFlags =
            D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;

        ComPtr<ID3D11Texture2D> nv12Texture;
        HRESULT result = device_->CreateTexture2D(
            &outputDescription, nullptr, &nv12Texture);
        if (FAILED(result)) {
            error = HResultMessage(
                "CreateTexture2D(NV12)", result);
            return nullptr;
        }

        D3D11_VIDEO_PROCESSOR_OUTPUT_VIEW_DESC outputViewDescription{};
        outputViewDescription.ViewDimension =
            D3D11_VPOV_DIMENSION_TEXTURE2D;
        outputViewDescription.Texture2D.MipSlice = 0;
        ComPtr<ID3D11VideoProcessorOutputView> outputView;
        result = videoDevice_->CreateVideoProcessorOutputView(
            nv12Texture.Get(),
            processorEnumerator_.Get(),
            &outputViewDescription,
            &outputView);
        if (FAILED(result)) {
            error = HResultMessage(
                "CreateVideoProcessorOutputView", result);
            return nullptr;
        }

        D3D11_VIDEO_PROCESSOR_STREAM stream{};
        stream.Enable = TRUE;
        stream.OutputIndex = 0;
        stream.InputFrameOrField = frameIndex;
        stream.pInputSurface = inputView_.Get();
        result = videoContext_->VideoProcessorBlt(
            processor_.Get(), outputView.Get(), 0, 1, &stream);
        if (FAILED(result)) {
            error = HResultMessage("VideoProcessorBlt", result);
            return nullptr;
        }
        context_->Flush();

        ComPtr<IMFMediaBuffer> surfaceBuffer;
        result = MFCreateDXGISurfaceBuffer(
            __uuidof(ID3D11Texture2D),
            nv12Texture.Get(),
            0,
            FALSE,
            &surfaceBuffer);
        if (FAILED(result)) {
            error = HResultMessage(
                "MFCreateDXGISurfaceBuffer", result);
            return nullptr;
        }

        ComPtr<IMFSample> sample;
        if (FAILED(MFCreateSample(&sample)) ||
            FAILED(sample->AddBuffer(surfaceBuffer.Get())) ||
            FAILED(sample->SetSampleTime(sampleTime)) ||
            FAILED(sample->SetSampleDuration(kFrameDuration100ns))) {
            error = "Failed to create the D3D11 NV12 input sample.";
            return nullptr;
        }
        return sample;
    }

private:
    void FillBgraPattern(uint32_t frameIndex)
    {
        for (uint32_t y = 0; y < kHeight; ++y) {
            for (uint32_t x = 0; x < kWidth; ++x) {
                const size_t offset =
                    (static_cast<size_t>(y) * kWidth + x) * 4;
                bgraPixels_[offset] =
                    static_cast<uint8_t>((x + frameIndex * 3) & 0xFF);
                bgraPixels_[offset + 1] =
                    static_cast<uint8_t>((y + frameIndex * 2) & 0xFF);
                bgraPixels_[offset + 2] =
                    static_cast<uint8_t>(
                        ((x / 8) ^ (y / 8) ^ frameIndex) & 0xFF);
                bgraPixels_[offset + 3] = 0xFF;
            }
        }
    }

    ComPtr<ID3D11Device> device_;
    ComPtr<ID3D11DeviceContext> context_;
    ComPtr<ID3D11VideoDevice> videoDevice_;
    ComPtr<ID3D11VideoContext> videoContext_;
    ComPtr<IMFDXGIDeviceManager> deviceManager_;
    ComPtr<ID3D11Texture2D> bgraTexture_;
    ComPtr<ID3D11VideoProcessorEnumerator> processorEnumerator_;
    ComPtr<ID3D11VideoProcessor> processor_;
    ComPtr<ID3D11VideoProcessorInputView> inputView_;
    std::vector<uint8_t> bgraPixels_;
};

class HardwareEncoderRunner final {
public:
    bool Initialize(D3D11Nv12FrameGenerator& frameGenerator,
                    MfH264EncoderSelfTestResult& result,
                    std::string& error)
    {
        const MFT_REGISTER_TYPE_INFO inputType{
            MFMediaType_Video,
            MFVideoFormat_NV12,
        };
        const MFT_REGISTER_TYPE_INFO outputType{
            MFMediaType_Video,
            MFVideoFormat_H264,
        };
        IMFActivate** activations = nullptr;
        UINT32 activationCount = 0;
        HRESULT hr = MFTEnumEx(
            MFT_CATEGORY_VIDEO_ENCODER,
            MFT_ENUM_FLAG_HARDWARE | MFT_ENUM_FLAG_SORTANDFILTER,
            &inputType,
            &outputType,
            &activations,
            &activationCount);
        if (FAILED(hr) || activationCount == 0) {
            error = FAILED(hr)
                        ? HResultMessage(
                              "MFTEnumEx(H264 hardware encoder)", hr)
                        : "No hardware H264 encoder MFT was found.";
            CoTaskMemFree(activations);
            return false;
        }

        for (UINT32 index = 0; index < activationCount; ++index) {
            ComPtr<IMFTransform> candidate;
            if (SUCCEEDED(activations[index]->ActivateObject(
                    IID_PPV_ARGS(&candidate)))) {
                ComPtr<IMFAttributes> attributes;
                candidate->GetAttributes(&attributes);
                if (ReadBooleanAttribute(
                        attributes.Get(), MF_SA_D3D11_AWARE)) {
                    transform_ = std::move(candidate);
                    activation_ = activations[index];
                    activation_->AddRef();
                    WCHAR* friendlyName = nullptr;
                    UINT32 friendlyNameLength = 0;
                    if (SUCCEEDED(activations[index]->GetAllocatedString(
                            MFT_FRIENDLY_NAME_Attribute,
                            &friendlyName,
                            &friendlyNameLength))) {
                        result.encoderName =
                            WideToUtf8(friendlyName, friendlyNameLength);
                        CoTaskMemFree(friendlyName);
                    }
                    break;
                }
                activations[index]->ShutdownObject();
            }
        }
        for (UINT32 index = 0; index < activationCount; ++index) {
            activations[index]->Release();
        }
        CoTaskMemFree(activations);
        if (!transform_) {
            error = "No D3D11-aware hardware H264 encoder MFT was found.";
            return false;
        }

        ComPtr<IMFAttributes> attributes;
        transform_->GetAttributes(&attributes);
        asynchronous_ =
            ReadBooleanAttribute(attributes.Get(), MF_TRANSFORM_ASYNC);
        result.asynchronousTransform = asynchronous_;
        if (asynchronous_ &&
            FAILED(attributes->SetUINT32(
                MF_TRANSFORM_ASYNC_UNLOCK, TRUE))) {
            error = "The asynchronous encoder could not be unlocked.";
            return false;
        }
        if (FAILED(transform_->ProcessMessage(
                MFT_MESSAGE_SET_D3D_MANAGER,
                reinterpret_cast<ULONG_PTR>(
                    frameGenerator.DeviceManager().Get())))) {
            error = "The encoder rejected the D3D11 device manager.";
            return false;
        }

        auto outputMediaType = CreateVideoType(MFVideoFormat_H264);
        auto inputMediaType = CreateVideoType(MFVideoFormat_NV12);
        if (!outputMediaType || !inputMediaType ||
            FAILED(outputMediaType->SetUINT32(
                MF_MT_AVG_BITRATE, kInitialBitrate)) ||
            FAILED(outputMediaType->SetUINT32(
                MF_MT_MPEG2_PROFILE,
                eAVEncH264VProfile_Main))) {
            error = "Failed to create the H264 encoder media types.";
            return false;
        }
        hr = transform_->SetOutputType(0, outputMediaType.Get(), 0);
        if (FAILED(hr)) {
            error = HResultMessage("SetOutputType(H264)", hr);
            return false;
        }
        hr = transform_->SetInputType(0, inputMediaType.Get(), 0);
        if (FAILED(hr)) {
            error = HResultMessage("SetInputType(NV12)", hr);
            return false;
        }

        transform_.As(&codecApi_);
        if (codecApi_) {
            SetCodecApiBoolean(CODECAPI_AVLowLatencyMode, true);
            SetCodecApiUInt32(
                CODECAPI_AVEncCommonRateControlMode,
                eAVEncCommonRateControlMode_CBR);
            SetCodecApiUInt32(
                CODECAPI_AVEncCommonMeanBitRate, kInitialBitrate);
            result.dynamicBitrateSupported =
                codecApi_->IsSupported(
                    &CODECAPI_AVEncCommonMeanBitRate) == S_OK;
        }

        if (asynchronous_ &&
            FAILED(transform_.As(&eventGenerator_))) {
            error = "The asynchronous encoder has no media event generator.";
            return false;
        }
        return true;
    }

    bool Run(D3D11Nv12FrameGenerator& frameGenerator,
             MfH264EncoderSelfTestResult& result,
             std::string& error)
    {
        if (FAILED(transform_->ProcessMessage(
                MFT_MESSAGE_NOTIFY_BEGIN_STREAMING, 0)) ||
            FAILED(transform_->ProcessMessage(
                MFT_MESSAGE_NOTIFY_START_OF_STREAM, 0))) {
            error = "The encoder rejected the start-of-stream messages.";
            return false;
        }
        return asynchronous_
                   ? RunAsynchronous(frameGenerator, result, error)
                   : RunSynchronous(frameGenerator, result, error);
    }

    void Shutdown()
    {
        if (transform_) {
            transform_->ProcessMessage(MFT_MESSAGE_COMMAND_FLUSH, 0);
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
    }

private:
    bool SetCodecApiBoolean(const GUID& key, bool value)
    {
        if (!codecApi_ || codecApi_->IsSupported(&key) != S_OK) {
            return false;
        }
        VARIANT setting;
        VariantInit(&setting);
        setting.vt = VT_BOOL;
        setting.boolVal = value ? VARIANT_TRUE : VARIANT_FALSE;
        return codecApi_->SetValue(&key, &setting) == S_OK;
    }

    bool SetCodecApiUInt32(const GUID& key, uint32_t value)
    {
        if (!codecApi_ || codecApi_->IsSupported(&key) != S_OK) {
            return false;
        }
        VARIANT setting;
        VariantInit(&setting);
        setting.vt = VT_UI4;
        setting.ulVal = value;
        return codecApi_->SetValue(&key, &setting) == S_OK;
    }

    void RequestKeyFrame()
    {
        SetCodecApiUInt32(CODECAPI_AVEncVideoForceKeyFrame, 1);
    }

    HRESULT SubmitFrame(D3D11Nv12FrameGenerator& frameGenerator,
                        uint32_t frameIndex,
                        MfH264EncoderSelfTestResult& result,
                        std::string& error)
    {
        if (frameIndex == 0 || frameIndex == kFrameCount / 2) {
            RequestKeyFrame();
        }
        if (frameIndex == kFrameCount / 2 &&
            result.dynamicBitrateSupported) {
            result.dynamicBitrateChanged = SetCodecApiUInt32(
                CODECAPI_AVEncCommonMeanBitRate,
                kAdjustedBitrate);
        }
        auto sample = frameGenerator.CreateFrameSample(
            frameIndex,
            static_cast<LONGLONG>(frameIndex) * kFrameDuration100ns,
            error);
        if (!sample) {
            return E_FAIL;
        }
        const HRESULT hr =
            transform_->ProcessInput(0, sample.Get(), 0);
        if (FAILED(hr)) {
            if (hr != MF_E_NOTACCEPTING) {
                error = HResultMessage("ProcessInput", hr);
            }
            return hr;
        }
        ++result.submittedFrames;
        result.d3d11PathUsed = true;
        return S_OK;
    }

    HRESULT ReadOneOutput(MfH264EncoderSelfTestResult& result,
                          std::string& error)
    {
        MFT_OUTPUT_STREAM_INFO streamInfo{};
        HRESULT hr = transform_->GetOutputStreamInfo(0, &streamInfo);
        if (FAILED(hr)) {
            error = HResultMessage("GetOutputStreamInfo", hr);
            return hr;
        }

        ComPtr<IMFSample> callerSample;
        if ((streamInfo.dwFlags &
             MFT_OUTPUT_STREAM_PROVIDES_SAMPLES) == 0) {
            ComPtr<IMFMediaBuffer> buffer;
            const DWORD bufferSize =
                (std::max<DWORD>)(streamInfo.cbSize, 2 * 1024 * 1024);
            if (FAILED(MFCreateSample(&callerSample)) ||
                FAILED(MFCreateMemoryBuffer(bufferSize, &buffer)) ||
                FAILED(callerSample->AddBuffer(buffer.Get()))) {
                error = "Failed to allocate the encoder output sample.";
                return E_OUTOFMEMORY;
            }
        }

        MFT_OUTPUT_DATA_BUFFER output{};
        output.dwStreamID = 0;
        output.pSample = callerSample.Get();
        DWORD status = 0;
        hr = transform_->ProcessOutput(0, 1, &output, &status);
        if (output.pEvents) {
            output.pEvents->Release();
        }
        if (FAILED(hr)) {
            if (output.pSample && !callerSample) {
                output.pSample->Release();
            }
            return hr;
        }

        IMFSample* encodedSample =
            output.pSample ? output.pSample : callerSample.Get();
        if (!encodedSample) {
            error = "The encoder returned success without an output sample.";
            return E_UNEXPECTED;
        }

        ComPtr<IMFMediaBuffer> contiguous;
        hr = encodedSample->ConvertToContiguousBuffer(&contiguous);
        if (SUCCEEDED(hr)) {
            BYTE* bytes = nullptr;
            DWORD maximumLength = 0;
            DWORD currentLength = 0;
            hr = contiguous->Lock(
                &bytes, &maximumLength, &currentLength);
            if (SUCCEEDED(hr)) {
                result.encodedBytes += currentLength;
                if (HasAnnexBStartCode(bytes, currentLength)) {
                    annexBSeen_ = true;
                }
                contiguous->Unlock();
            }
        }
        UINT32 cleanPoint = FALSE;
        if (SUCCEEDED(encodedSample->GetUINT32(
                MFSampleExtension_CleanPoint, &cleanPoint)) &&
            cleanPoint != FALSE) {
            ++result.keyFrames;
        }
        ++result.encodedSamples;
        if (output.pSample && !callerSample) {
            output.pSample->Release();
        }
        return S_OK;
    }

    bool BeginDrain(std::string& error)
    {
        if (drainSent_) {
            return true;
        }
        HRESULT hr = transform_->ProcessMessage(
            MFT_MESSAGE_NOTIFY_END_OF_STREAM, 0);
        if (SUCCEEDED(hr)) {
            hr = transform_->ProcessMessage(
                MFT_MESSAGE_COMMAND_DRAIN, 0);
        }
        if (FAILED(hr)) {
            error = HResultMessage("MFT drain", hr);
            return false;
        }
        drainSent_ = true;
        return true;
    }

    bool RunAsynchronous(D3D11Nv12FrameGenerator& frameGenerator,
                         MfH264EncoderSelfTestResult& result,
                         std::string& error)
    {
        const auto deadline = std::chrono::steady_clock::now() +
                              kTestTimeout;
        while (std::chrono::steady_clock::now() < deadline) {
            ComPtr<IMFMediaEvent> event;
            const HRESULT eventResult = eventGenerator_->GetEvent(
                MF_EVENT_FLAG_NO_WAIT, &event);
            if (eventResult == MF_E_NO_EVENTS_AVAILABLE) {
                std::this_thread::sleep_for(
                    std::chrono::milliseconds(1));
                continue;
            }
            if (FAILED(eventResult)) {
                error = HResultMessage("IMFMediaEventGenerator::GetEvent",
                                       eventResult);
                return false;
            }
            HRESULT eventStatus = S_OK;
            event->GetStatus(&eventStatus);
            if (FAILED(eventStatus)) {
                error = HResultMessage("Asynchronous MFT event",
                                       eventStatus);
                return false;
            }
            MediaEventType eventType = MEUnknown;
            event->GetType(&eventType);
            if (eventType == METransformNeedInput && !drainSent_) {
                if (result.submittedFrames < kFrameCount) {
                    const HRESULT submitResult = SubmitFrame(
                        frameGenerator,
                        result.submittedFrames,
                        result,
                        error);
                    if (FAILED(submitResult)) {
                        if (error.empty()) {
                            error = HResultMessage(
                                "ProcessInput", submitResult);
                        }
                        return false;
                    }
                    if (result.submittedFrames == kFrameCount &&
                        !BeginDrain(error)) {
                        return false;
                    }
                }
            } else if (eventType == METransformHaveOutput) {
                const HRESULT outputResult =
                    ReadOneOutput(result, error);
                if (FAILED(outputResult) &&
                    outputResult != MF_E_TRANSFORM_NEED_MORE_INPUT) {
                    if (error.empty()) {
                        error = HResultMessage(
                            "ProcessOutput", outputResult);
                    }
                    return false;
                }
            } else if (eventType == METransformDrainComplete) {
                return true;
            }
        }
        error = "The asynchronous hardware encoder self-test timed out.";
        return false;
    }

    bool RunSynchronous(D3D11Nv12FrameGenerator& frameGenerator,
                        MfH264EncoderSelfTestResult& result,
                        std::string& error)
    {
        while (result.submittedFrames < kFrameCount) {
            const HRESULT submitResult = SubmitFrame(
                frameGenerator,
                result.submittedFrames,
                result,
                error);
            if (submitResult == MF_E_NOTACCEPTING) {
                const HRESULT outputResult =
                    ReadOneOutput(result, error);
                if (outputResult == MF_E_TRANSFORM_NEED_MORE_INPUT) {
                    error =
                        "The synchronous encoder rejected input but had no output.";
                    return false;
                }
                if (FAILED(outputResult)) {
                    if (error.empty()) {
                        error = HResultMessage(
                            "ProcessOutput", outputResult);
                    }
                    return false;
                }
                continue;
            }
            if (FAILED(submitResult)) {
                if (error.empty()) {
                    error = HResultMessage(
                        "ProcessInput", submitResult);
                }
                return false;
            }
            for (;;) {
                const HRESULT outputResult =
                    ReadOneOutput(result, error);
                if (outputResult == MF_E_TRANSFORM_NEED_MORE_INPUT) {
                    error.clear();
                    break;
                }
                if (FAILED(outputResult)) {
                    if (error.empty()) {
                        error = HResultMessage(
                            "ProcessOutput", outputResult);
                    }
                    return false;
                }
            }
        }
        if (!BeginDrain(error)) {
            return false;
        }
        for (;;) {
            const HRESULT outputResult = ReadOneOutput(result, error);
            if (outputResult == MF_E_TRANSFORM_NEED_MORE_INPUT) {
                error.clear();
                return true;
            }
            if (FAILED(outputResult)) {
                if (error.empty()) {
                    error = HResultMessage(
                        "ProcessOutput(drain)", outputResult);
                }
                return false;
            }
        }
    }

    IMFActivate* activation_ = nullptr;
    ComPtr<IMFTransform> transform_;
    ComPtr<IMFMediaEventGenerator> eventGenerator_;
    ComPtr<ICodecAPI> codecApi_;
    bool asynchronous_ = false;
    bool drainSent_ = false;
    bool annexBSeen_ = false;
};

}  // namespace

MfH264EncoderSelfTestResult RunMfH264EncoderD3D11SelfTest()
{
    MfH264EncoderSelfTestResult result;
    std::ostringstream report;
    std::string error;

    const HRESULT comResult = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
    const bool shouldUninitializeCom = SUCCEEDED(comResult);
    if (FAILED(comResult) && comResult != RPC_E_CHANGED_MODE) {
        result.report = HResultMessage("CoInitializeEx", comResult);
        return result;
    }
    const HRESULT startupResult = MFStartup(MF_VERSION, MFSTARTUP_FULL);
    if (FAILED(startupResult)) {
        result.report = HResultMessage("MFStartup", startupResult);
        if (shouldUninitializeCom) {
            CoUninitialize();
        }
        return result;
    }

    D3D11Nv12FrameGenerator frameGenerator;
    HardwareEncoderRunner encoder;
    const auto startedAt = std::chrono::steady_clock::now();
    bool initialized = frameGenerator.Initialize(error);
    if (initialized) {
        initialized = encoder.Initialize(frameGenerator, result, error);
    }
    const bool completed =
        initialized && encoder.Run(frameGenerator, result, error);
    result.elapsedMilliseconds =
        std::chrono::duration<double, std::milli>(
            std::chrono::steady_clock::now() - startedAt)
            .count();
    encoder.Shutdown();

    result.passed = completed &&
                    result.d3d11PathUsed &&
                    result.submittedFrames == kFrameCount &&
                    result.encodedSamples > 0 &&
                    result.encodedBytes > 0 &&
                    result.keyFrames > 0;
    report << "Media Foundation H264 D3D11 encoder self-test\n"
           << "================================================\n"
           << "Encoder: "
           << (result.encoderName.empty() ? "<none>"
                                          : result.encoderName)
           << '\n'
           << "Transform mode: "
           << (result.asynchronousTransform ? "asynchronous"
                                            : "synchronous")
           << '\n'
           << "Input path: D3D11 BGRA -> VideoProcessor -> D3D11 NV12\n"
           << "Resolution: " << kWidth << 'x' << kHeight << " @ "
           << kFrameRate << " FPS\n"
           << "Submitted frames: " << result.submittedFrames << " / "
           << kFrameCount << '\n'
           << "Encoded samples: " << result.encodedSamples << '\n'
           << "Encoded bytes: " << result.encodedBytes << '\n'
           << "Key frames: " << result.keyFrames << '\n'
           << "Dynamic bitrate supported: "
           << (result.dynamicBitrateSupported ? "YES" : "NO") << '\n'
           << "Dynamic bitrate changed: "
           << (result.dynamicBitrateChanged ? "YES" : "NO") << '\n'
           << "Elapsed: " << std::fixed << std::setprecision(1)
           << result.elapsedMilliseconds << " ms\n"
           << "Result: " << (result.passed ? "PASS" : "FAIL");
    if (!error.empty()) {
        report << "\nError: " << error;
    }
    result.report = report.str();

    MFShutdown();
    if (shouldUninitializeCom) {
        CoUninitialize();
    }
    return result;
}

}  // namespace remote
