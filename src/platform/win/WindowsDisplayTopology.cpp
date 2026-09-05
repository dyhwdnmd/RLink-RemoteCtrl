// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#include "WindowsDisplayTopology.h"

#include <Windows.h>

#include <dxgi1_2.h>
#include <wrl/client.h>

#include <algorithm>
#include <array>
#include <cstdint>
#include <string>
#include <unordered_map>

namespace remote {
namespace {

using Microsoft::WRL::ComPtr;

std::string Utf8(const wchar_t* value)
{
    if (!value || !*value) {
        return {};
    }
    const int length = WideCharToMultiByte(
        CP_UTF8, 0, value, -1, nullptr, 0, nullptr, nullptr);
    if (length <= 1) {
        return {};
    }
    std::string result(static_cast<std::size_t>(length), '\0');
    WideCharToMultiByte(
        CP_UTF8, 0, value, -1, result.data(), length, nullptr, nullptr);
    result.pop_back();
    return result;
}

std::uint64_t Fnv1a64(const void* data,
                      std::size_t size,
                      std::uint64_t hash = 14695981039346656037ULL)
{
    const auto* bytes = static_cast<const unsigned char*>(data);
    for (std::size_t index = 0; index < size; ++index) {
        hash ^= bytes[index];
        hash *= 1099511628211ULL;
    }
    return hash;
}

std::uint64_t HashString(const std::string& value,
                         std::uint64_t hash = 14695981039346656037ULL)
{
    return Fnv1a64(value.data(), value.size(), hash);
}

std::uint32_t SessionDisplayId(const std::string& stableKey)
{
    const std::uint64_t hash = HashString(stableKey);
    std::uint32_t result = static_cast<std::uint32_t>(
        hash ^ (hash >> 32));
    return result == 0 ? 1 : result;
}

std::uint32_t RotationDegrees(DWORD orientation)
{
    switch (orientation) {
    case DMDO_90:
        return 90;
    case DMDO_180:
        return 180;
    case DMDO_270:
        return 270;
    default:
        return 0;
    }
}

void ReadMonitorDpi(HMONITOR monitor,
                    std::uint32_t* dpiX,
                    std::uint32_t* dpiY)
{
    if (!dpiX || !dpiY) {
        return;
    }
    *dpiX = USER_DEFAULT_SCREEN_DPI;
    *dpiY = USER_DEFAULT_SCREEN_DPI;
    HMODULE shcore = LoadLibraryW(L"Shcore.dll");
    if (!shcore) {
        return;
    }
    using GetDpiForMonitorFn =
        HRESULT(WINAPI*)(HMONITOR, int, UINT*, UINT*);
    const auto getDpi = reinterpret_cast<GetDpiForMonitorFn>(
        GetProcAddress(shcore, "GetDpiForMonitor"));
    UINT x = USER_DEFAULT_SCREEN_DPI;
    UINT y = USER_DEFAULT_SCREEN_DPI;
    if (getDpi && SUCCEEDED(getDpi(monitor, 0, &x, &y))) {
        *dpiX = x;
        *dpiY = y;
    }
    FreeLibrary(shcore);
}

struct DxgiOutputIdentity {
    std::uint64_t adapterLuid = 0;
    std::uint32_t outputIndex = 0;
};

std::unordered_map<std::string, DxgiOutputIdentity>
EnumerateDxgiOutputs()
{
    std::unordered_map<std::string, DxgiOutputIdentity> result;
    ComPtr<IDXGIFactory1> factory;
    if (FAILED(CreateDXGIFactory1(IID_PPV_ARGS(&factory)))) {
        return result;
    }
    for (UINT adapterIndex = 0;; ++adapterIndex) {
        ComPtr<IDXGIAdapter1> adapter;
        if (factory->EnumAdapters1(adapterIndex, &adapter) ==
            DXGI_ERROR_NOT_FOUND) {
            break;
        }
        DXGI_ADAPTER_DESC1 adapterDescription{};
        if (FAILED(adapter->GetDesc1(&adapterDescription))) {
            continue;
        }
        const std::uint64_t luid =
            static_cast<std::uint32_t>(
                adapterDescription.AdapterLuid.LowPart) |
            (static_cast<std::uint64_t>(
                 static_cast<std::uint32_t>(
                     adapterDescription.AdapterLuid.HighPart))
             << 32);
        for (UINT outputIndex = 0;; ++outputIndex) {
            ComPtr<IDXGIOutput> output;
            if (adapter->EnumOutputs(outputIndex, &output) ==
                DXGI_ERROR_NOT_FOUND) {
                break;
            }
            DXGI_OUTPUT_DESC description{};
            if (FAILED(output->GetDesc(&description)) ||
                !description.AttachedToDesktop) {
                continue;
            }
            result.emplace(
                Utf8(description.DeviceName),
                DxgiOutputIdentity{luid, outputIndex});
        }
    }
    return result;
}

struct EnumerationContext {
    const std::unordered_map<std::string, DxgiOutputIdentity>* dxgi =
        nullptr;
    std::vector<DisplayDescriptor>* displays = nullptr;
};

BOOL CALLBACK AddMonitor(HMONITOR monitor,
                         HDC,
                         LPRECT,
                         LPARAM parameter)
{
    auto* context =
        reinterpret_cast<EnumerationContext*>(parameter);
    if (!context || !context->displays) {
        return FALSE;
    }
    MONITORINFOEXW information{};
    information.cbSize = sizeof(information);
    if (!GetMonitorInfoW(monitor, &information)) {
        return TRUE;
    }

    DISPLAY_DEVICEW monitorDevice{};
    monitorDevice.cb = sizeof(monitorDevice);
    const bool hasMonitorDevice = EnumDisplayDevicesW(
        information.szDevice, 0, &monitorDevice, EDD_GET_DEVICE_INTERFACE_NAME);

    DEVMODEW mode{};
    mode.dmSize = sizeof(mode);
    const bool hasMode = EnumDisplaySettingsExW(
        information.szDevice, ENUM_CURRENT_SETTINGS, &mode, 0);

    DisplayDescriptor display;
    display.deviceName = Utf8(information.szDevice);
    display.friendlyName = hasMonitorDevice
        ? Utf8(monitorDevice.DeviceString)
        : display.deviceName;
    display.stableDisplayKey =
        hasMonitorDevice && monitorDevice.DeviceID[0] != L'\0'
            ? Utf8(monitorDevice.DeviceID)
            : display.deviceName;
    display.sessionDisplayId =
        SessionDisplayId(display.stableDisplayKey);
    display.left = information.rcMonitor.left;
    display.top = information.rcMonitor.top;
    display.width = static_cast<std::uint32_t>(
        information.rcMonitor.right - information.rcMonitor.left);
    display.height = static_cast<std::uint32_t>(
        information.rcMonitor.bottom - information.rcMonitor.top);
    display.primary =
        (information.dwFlags & MONITORINFOF_PRIMARY) != 0;
    display.rotationDegrees =
        hasMode ? RotationDegrees(mode.dmDisplayOrientation) : 0;
    ReadMonitorDpi(monitor, &display.dpiX, &display.dpiY);
    display.scalePercent = static_cast<std::uint32_t>(
        (static_cast<std::uint64_t>(display.dpiX) * 100 + 48) / 96);
    display.nativeMonitorHandle =
        static_cast<std::uint64_t>(
            reinterpret_cast<std::uintptr_t>(monitor));
    if (context->dxgi) {
        const auto output = context->dxgi->find(display.deviceName);
        if (output != context->dxgi->end()) {
            display.adapterLuid = output->second.adapterLuid;
            display.outputIndex = output->second.outputIndex;
        }
    }
    if (!display.stableDisplayKey.empty() &&
        display.width > 0 && display.height > 0) {
        context->displays->push_back(std::move(display));
    }
    return TRUE;
}

std::uint64_t ComputeLayoutVersion(
    const std::vector<DisplayDescriptor>& displays)
{
    std::uint64_t hash = 14695981039346656037ULL;
    for (const auto& display : displays) {
        hash = HashString(display.stableDisplayKey, hash);
        const std::array<std::uint64_t, 9> values = {
            display.sessionDisplayId,
            static_cast<std::uint32_t>(display.left),
            static_cast<std::uint32_t>(display.top),
            display.width,
            display.height,
            display.dpiX,
            display.dpiY,
            display.rotationDegrees,
            display.primary ? 1ULL : 0ULL};
        hash = Fnv1a64(values.data(), sizeof(values), hash);
    }
    return hash == 0 ? 1 : hash;
}

}  // namespace

DisplayTopologySnapshot EnumerateWindowsDisplayTopology()
{
    DisplayTopologySnapshot result;
    const auto dxgi = EnumerateDxgiOutputs();
    EnumerationContext context{&dxgi, &result.displays};
    EnumDisplayMonitors(
        nullptr, nullptr, AddMonitor,
        reinterpret_cast<LPARAM>(&context));
    std::sort(
        result.displays.begin(), result.displays.end(),
        [](const DisplayDescriptor& left,
           const DisplayDescriptor& right) {
            if (left.primary != right.primary) {
                return left.primary;
            }
            if (left.left != right.left) {
                return left.left < right.left;
            }
            if (left.top != right.top) {
                return left.top < right.top;
            }
            return left.stableDisplayKey < right.stableDisplayKey;
        });
    result.layoutVersion = ComputeLayoutVersion(result.displays);
    return result;
}

}  // namespace remote
