// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#include "WindowsVideoCapabilityProbe.h"

#include <iomanip>
#include <iterator>
#include <sstream>

#include <Windows.h>
#include <d3d11.h>
#include <dxgi.h>
#include <mfapi.h>
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

void ProbeD3D11(WindowsVideoCapabilityReport& report)
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
    const UINT flags = D3D11_CREATE_DEVICE_BGRA_SUPPORT |
                       D3D11_CREATE_DEVICE_VIDEO_SUPPORT;

    HRESULT result = D3D11CreateDevice(
        nullptr,
        D3D_DRIVER_TYPE_HARDWARE,
        nullptr,
        flags,
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
            flags,
            featureLevels + 1,
            static_cast<UINT>(std::size(featureLevels) - 1),
            D3D11_SDK_VERSION,
            &device,
            &selectedFeatureLevel,
            &context);
    }

    if (FAILED(result)) {
        report.warnings.push_back(HResultMessage("D3D11CreateDevice", result));
        return;
    }

    report.d3d11DeviceCreated = true;
    report.featureLevel = static_cast<uint32_t>(selectedFeatureLevel);

    ComPtr<ID3D11VideoDevice> videoDevice;
    report.d3d11VideoDeviceAvailable = SUCCEEDED(device.As(&videoDevice));

    UINT formatSupport = 0;
    if (SUCCEEDED(device->CheckFormatSupport(DXGI_FORMAT_NV12, &formatSupport))) {
        report.nv12TextureSupported =
            (formatSupport & D3D11_FORMAT_SUPPORT_TEXTURE2D) != 0;
    }

    ComPtr<IDXGIDevice> dxgiDevice;
    ComPtr<IDXGIAdapter> adapter;
    if (SUCCEEDED(device.As(&dxgiDevice)) &&
        SUCCEEDED(dxgiDevice->GetAdapter(&adapter))) {
        DXGI_ADAPTER_DESC description{};
        if (SUCCEEDED(adapter->GetDesc(&description))) {
            report.adapterName = description.Description;
            report.adapterLuidLowPart = description.AdapterLuid.LowPart;
            report.adapterLuidHighPart = description.AdapterLuid.HighPart;
        }
    }
}

void ProbeH264HardwareDecoders(WindowsVideoCapabilityReport& report)
{
    const HRESULT comResult = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
    const bool shouldUninitializeCom = SUCCEEDED(comResult);
    if (FAILED(comResult) && comResult != RPC_E_CHANGED_MODE) {
        report.warnings.push_back(HResultMessage("CoInitializeEx", comResult));
        return;
    }

    const HRESULT startupResult = MFStartup(MF_VERSION, MFSTARTUP_FULL);
    if (FAILED(startupResult)) {
        report.warnings.push_back(HResultMessage("MFStartup", startupResult));
        if (shouldUninitializeCom) {
            CoUninitialize();
        }
        return;
    }

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
    const HRESULT enumerateResult = MFTEnumEx(
        MFT_CATEGORY_VIDEO_DECODER,
        MFT_ENUM_FLAG_ALL | MFT_ENUM_FLAG_SORTANDFILTER,
        &inputType,
        &outputType,
        &activations,
        &activationCount);

    if (FAILED(enumerateResult)) {
        report.warnings.push_back(HResultMessage("MFTEnumEx", enumerateResult));
    } else {
        report.h264DecoderMftCount = activationCount;
        for (UINT32 index = 0; index < activationCount; ++index) {
            IMFActivate* activation = activations[index];
            WCHAR* friendlyName = nullptr;
            UINT32 friendlyNameLength = 0;
            if (SUCCEEDED(activation->GetAllocatedString(
                    MFT_FRIENDLY_NAME_Attribute,
                    &friendlyName,
                    &friendlyNameLength))) {
                report.h264DecoderMftNames.emplace_back(
                    friendlyName,
                    friendlyNameLength);
                CoTaskMemFree(friendlyName);
            }

            WCHAR* hardwareUrl = nullptr;
            UINT32 hardwareUrlLength = 0;
            if (SUCCEEDED(activation->GetAllocatedString(
                    MFT_ENUM_HARDWARE_URL_Attribute,
                    &hardwareUrl,
                    &hardwareUrlLength))) {
                if (hardwareUrlLength > 0) {
                    ++report.h264HardwareRegisteredMftCount;
                }
                CoTaskMemFree(hardwareUrl);
            }

            ComPtr<IMFTransform> transform;
            if (SUCCEEDED(activation->ActivateObject(
                    IID_PPV_ARGS(transform.GetAddressOf())))) {
                ComPtr<IMFAttributes> attributes;
                UINT32 d3d11Aware = FALSE;
                if (SUCCEEDED(transform->GetAttributes(&attributes)) &&
                    SUCCEEDED(attributes->GetUINT32(
                        MF_SA_D3D11_AWARE,
                        &d3d11Aware)) &&
                    d3d11Aware != FALSE) {
                    ++report.h264D3D11AwareDecoderCount;
                }
                activation->ShutdownObject();
            }

            activation->Release();
        }
        CoTaskMemFree(activations);
    }

    MFShutdown();
    if (shouldUninitializeCom) {
        CoUninitialize();
    }
}

}  // namespace

WindowsVideoCapabilityReport ProbeWindowsVideoCapabilities()
{
    WindowsVideoCapabilityReport report;
    ProbeD3D11(report);
    ProbeH264HardwareDecoders(report);
    return report;
}

}  // namespace remote
