// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#include "MfH264EncoderCapabilityProbe.h"

#include <algorithm>
#include <iomanip>
#include <iterator>
#include <limits>
#include <sstream>

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

std::string GuidToString(REFGUID guid)
{
    wchar_t value[64]{};
    const int length =
        StringFromGUID2(guid, value, static_cast<int>(std::size(value)));
    return length > 1
               ? WideToUtf8(value, static_cast<size_t>(length - 1))
               : std::string{};
}

bool ReadBooleanAttribute(IMFAttributes* attributes, REFGUID key)
{
    if (!attributes) {
        return false;
    }

    UINT32 value = FALSE;
    return SUCCEEDED(attributes->GetUINT32(key, &value)) && value != FALSE;
}

ComPtr<IMFMediaType> CreateVideoType(REFGUID subtype,
                                     uint32_t width,
                                     uint32_t height,
                                     uint32_t frameRateNumerator,
                                     uint32_t frameRateDenominator)
{
    ComPtr<IMFMediaType> type;
    if (FAILED(MFCreateMediaType(&type)) ||
        FAILED(type->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Video)) ||
        FAILED(type->SetGUID(MF_MT_SUBTYPE, subtype)) ||
        FAILED(MFSetAttributeSize(type.Get(), MF_MT_FRAME_SIZE, width, height)) ||
        FAILED(MFSetAttributeRatio(type.Get(),
                                   MF_MT_FRAME_RATE,
                                   frameRateNumerator,
                                   frameRateDenominator)) ||
        FAILED(MFSetAttributeRatio(type.Get(), MF_MT_PIXEL_ASPECT_RATIO, 1, 1)) ||
        FAILED(type->SetUINT32(
            MF_MT_INTERLACE_MODE,
            MFVideoInterlace_Progressive))) {
        return nullptr;
    }
    return type;
}

bool TestCpuNv12Input(IMFTransform* transform)
{
    if (!transform) {
        return false;
    }

    auto inputType = CreateVideoType(MFVideoFormat_NV12, 1920, 1080, 30, 1);
    if (!inputType) {
        return false;
    }

    HRESULT result =
        transform->SetInputType(0, inputType.Get(), MFT_SET_TYPE_TEST_ONLY);
    if (SUCCEEDED(result)) {
        return true;
    }

    auto outputType = CreateVideoType(MFVideoFormat_H264, 1920, 1080, 30, 1);
    if (!outputType ||
        FAILED(outputType->SetUINT32(MF_MT_AVG_BITRATE, 4'000'000)) ||
        FAILED(outputType->SetUINT32(
            MF_MT_MPEG2_PROFILE,
            eAVEncH264VProfile_Main))) {
        return false;
    }

    result =
        transform->SetOutputType(0, outputType.Get(), MFT_SET_TYPE_TEST_ONLY);
    if (FAILED(result)) {
        return false;
    }

    // Some hardware encoders do not expose or validate their input types until
    // an output type has actually been selected. This transform instance is
    // probe-only, so committing the disposable output type is safe and avoids
    // reporting a false negative for CPU NV12 input.
    if (FAILED(transform->SetOutputType(0, outputType.Get(), 0))) {
        return false;
    }
    return SUCCEEDED(
        transform->SetInputType(0, inputType.Get(), MFT_SET_TYPE_TEST_ONLY));
}

void UpdateMaximumVideoMode(IMFMediaType* type,
                            MfH264EncoderCapability& capability)
{
    if (!type) {
        return;
    }

    GUID subtype{};
    if (FAILED(type->GetGUID(MF_MT_SUBTYPE, &subtype)) ||
        subtype != MFVideoFormat_NV12) {
        return;
    }

    UINT32 width = 0;
    UINT32 height = 0;
    if (SUCCEEDED(MFGetAttributeSize(
            type, MF_MT_FRAME_SIZE, &width, &height))) {
        const uint64_t currentArea =
            static_cast<uint64_t>(capability.maximumWidth) *
            capability.maximumHeight;
        const uint64_t candidateArea =
            static_cast<uint64_t>(width) * height;
        if (candidateArea > currentArea) {
            capability.maximumWidth = width;
            capability.maximumHeight = height;
        }
    }

    UINT32 numerator = 0;
    UINT32 denominator = 0;
    if (SUCCEEDED(MFGetAttributeRatio(
            type, MF_MT_FRAME_RATE, &numerator, &denominator)) &&
        denominator != 0) {
        const uint64_t left =
            static_cast<uint64_t>(numerator) *
            capability.maximumFrameRateDenominator;
        const uint64_t right =
            static_cast<uint64_t>(
                capability.maximumFrameRateNumerator) *
            denominator;
        if (capability.maximumFrameRateDenominator == 0 || left > right) {
            capability.maximumFrameRateNumerator = numerator;
            capability.maximumFrameRateDenominator = denominator;
        }
    }
}

void EnumerateInputModes(IMFTransform* transform,
                         MfH264EncoderCapability& capability)
{
    for (DWORD index = 0;; ++index) {
        ComPtr<IMFMediaType> type;
        const HRESULT result =
            transform->GetInputAvailableType(0, index, &type);
        if (result == MF_E_NO_MORE_TYPES) {
            break;
        }
        if (FAILED(result)) {
            capability.warnings.push_back(
                HResultMessage("GetInputAvailableType", result));
            break;
        }
        UpdateMaximumVideoMode(type.Get(), capability);
    }
}

void EnumerateH264Profiles(IMFTransform* transform,
                           MfH264EncoderCapability& capability)
{
    for (DWORD index = 0;; ++index) {
        ComPtr<IMFMediaType> type;
        const HRESULT result =
            transform->GetOutputAvailableType(0, index, &type);
        if (result == MF_E_NO_MORE_TYPES) {
            break;
        }
        if (FAILED(result)) {
            capability.warnings.push_back(
                HResultMessage("GetOutputAvailableType", result));
            break;
        }

        GUID subtype{};
        if (FAILED(type->GetGUID(MF_MT_SUBTYPE, &subtype)) ||
            subtype != MFVideoFormat_H264) {
            continue;
        }

        UINT32 profile = 0;
        if (SUCCEEDED(type->GetUINT32(MF_MT_MPEG2_PROFILE, &profile)) &&
            std::find(capability.h264Profiles.begin(),
                      capability.h264Profiles.end(),
                      profile) == capability.h264Profiles.end()) {
            capability.h264Profiles.push_back(profile);
        }
    }
}

ComPtr<ID3D11Device> CreateD3D11ProbeDevice()
{
    constexpr D3D_FEATURE_LEVEL featureLevels[] = {
        D3D_FEATURE_LEVEL_11_1,
        D3D_FEATURE_LEVEL_11_0,
        D3D_FEATURE_LEVEL_10_1,
        D3D_FEATURE_LEVEL_10_0,
    };

    ComPtr<ID3D11Device> device;
    ComPtr<ID3D11DeviceContext> context;
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
        &device,
        &selectedFeatureLevel,
        &context);
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
            &device,
            &selectedFeatureLevel,
            &context);
    }
    return SUCCEEDED(result) ? device : nullptr;
}

bool TestD3D11DeviceManager(IMFTransform* transform, ID3D11Device* device)
{
    if (!transform || !device) {
        return false;
    }

    UINT resetToken = 0;
    ComPtr<IMFDXGIDeviceManager> manager;
    if (FAILED(MFCreateDXGIDeviceManager(&resetToken, &manager)) ||
        FAILED(manager->ResetDevice(device, resetToken))) {
        return false;
    }

    return SUCCEEDED(transform->ProcessMessage(
        MFT_MESSAGE_SET_D3D_MANAGER,
        reinterpret_cast<ULONG_PTR>(manager.Get())));
}

void QueryCodecApi(IMFTransform* transform,
                   MfH264EncoderCapability& capability)
{
    ComPtr<ICodecAPI> codecApi;
    if (!transform || FAILED(transform->QueryInterface(IID_PPV_ARGS(&codecApi)))) {
        return;
    }

    capability.lowLatencyModeSupported =
        codecApi->IsSupported(&CODECAPI_AVLowLatencyMode) == S_OK;
    capability.dynamicBitrateSupported =
        codecApi->IsSupported(&CODECAPI_AVEncCommonMeanBitRate) == S_OK;
    capability.dynamicBitrateModifiable =
        capability.dynamicBitrateSupported &&
        codecApi->IsModifiable(&CODECAPI_AVEncCommonMeanBitRate) == S_OK;
}

MfH264EncoderCapability InspectEncoder(IMFActivate* activation,
                                       ID3D11Device* d3d11Device)
{
    MfH264EncoderCapability capability;

    WCHAR* name = nullptr;
    UINT32 nameLength = 0;
    if (SUCCEEDED(activation->GetAllocatedString(
            MFT_FRIENDLY_NAME_Attribute, &name, &nameLength))) {
        capability.name = WideToUtf8(name, nameLength);
        CoTaskMemFree(name);
    }

    WCHAR* hardwareUrl = nullptr;
    UINT32 hardwareUrlLength = 0;
    if (SUCCEEDED(activation->GetAllocatedString(
            MFT_ENUM_HARDWARE_URL_Attribute,
            &hardwareUrl,
            &hardwareUrlLength))) {
        capability.hardwareUrl =
            WideToUtf8(hardwareUrl, hardwareUrlLength);
        CoTaskMemFree(hardwareUrl);
    }

    GUID clsid{};
    if (SUCCEEDED(
            activation->GetGUID(MFT_TRANSFORM_CLSID_Attribute, &clsid))) {
        capability.clsid = GuidToString(clsid);
    }

    ComPtr<IMFTransform> transform;
    const HRESULT activateResult =
        activation->ActivateObject(IID_PPV_ARGS(&transform));
    if (FAILED(activateResult)) {
        capability.warnings.push_back(
            HResultMessage("ActivateObject", activateResult));
        return capability;
    }

    ComPtr<IMFAttributes> attributes;
    if (SUCCEEDED(transform->GetAttributes(&attributes))) {
        capability.asynchronous =
            ReadBooleanAttribute(attributes.Get(), MF_TRANSFORM_ASYNC);
        capability.d3d11Aware =
            ReadBooleanAttribute(attributes.Get(), MF_SA_D3D11_AWARE);
        capability.dynamicFormatChangeSupported =
            ReadBooleanAttribute(attributes.Get(),
                                 MFT_SUPPORT_DYNAMIC_FORMAT_CHANGE);
    }

    capability.cpuNv12InputSupported = TestCpuNv12Input(transform.Get());
    EnumerateInputModes(transform.Get(), capability);
    EnumerateH264Profiles(transform.Get(), capability);
    QueryCodecApi(transform.Get(), capability);

    if (capability.d3d11Aware) {
        capability.d3d11DeviceManagerAccepted =
            TestD3D11DeviceManager(transform.Get(), d3d11Device);
    }

    transform.Reset();
    activation->ShutdownObject();
    return capability;
}

}  // namespace

MfH264EncoderCapabilityReport ProbeMfH264EncoderCapabilities()
{
    MfH264EncoderCapabilityReport report;

    const HRESULT comResult = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
    const bool shouldUninitializeCom = SUCCEEDED(comResult);
    if (FAILED(comResult) && comResult != RPC_E_CHANGED_MODE) {
        report.warnings.push_back(
            HResultMessage("CoInitializeEx", comResult));
        return report;
    }

    const HRESULT startupResult = MFStartup(MF_VERSION, MFSTARTUP_FULL);
    if (FAILED(startupResult)) {
        report.warnings.push_back(
            HResultMessage("MFStartup", startupResult));
        if (shouldUninitializeCom) {
            CoUninitialize();
        }
        return report;
    }
    report.mediaFoundationStarted = true;

    auto d3d11Device = CreateD3D11ProbeDevice();
    report.d3d11ProbeDeviceCreated = d3d11Device != nullptr;

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
    const HRESULT enumerateResult = MFTEnumEx(
        MFT_CATEGORY_VIDEO_ENCODER,
        MFT_ENUM_FLAG_HARDWARE | MFT_ENUM_FLAG_SORTANDFILTER,
        &inputType,
        &outputType,
        &activations,
        &activationCount);
    if (FAILED(enumerateResult)) {
        report.warnings.push_back(
            HResultMessage("MFTEnumEx(H264 hardware encoder)",
                           enumerateResult));
    } else {
        report.enumerationSucceeded = true;
        report.hardwareEncoderCount = activationCount;
        report.encoders.reserve(activationCount);
        for (UINT32 index = 0; index < activationCount; ++index) {
            report.encoders.push_back(
                InspectEncoder(activations[index], d3d11Device.Get()));
            activations[index]->Release();
        }
        CoTaskMemFree(activations);
    }

    MFShutdown();
    if (shouldUninitializeCom) {
        CoUninitialize();
    }
    return report;
}

}  // namespace remote
