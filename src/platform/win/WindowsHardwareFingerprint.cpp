// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#include "WindowsHardwareFingerprint.h"

#include <Windows.h>
#include <dxgi1_2.h>
#include <winternl.h>

#include <algorithm>
#include <cstdint>
#include <optional>
#include <sstream>
#include <utility>
#include <vector>

namespace remote {
namespace {

using RtlGetVersionFunction = LONG(WINAPI*)(PRTL_OSVERSIONINFOW);

RTL_OSVERSIONINFOW ReadWindowsVersion()
{
    RTL_OSVERSIONINFOW version{};
    version.dwOSVersionInfoSize = sizeof(version);
    const HMODULE ntdll = GetModuleHandleW(L"ntdll.dll");
    const auto rtlGetVersion = ntdll
        ? reinterpret_cast<RtlGetVersionFunction>(
              GetProcAddress(ntdll, "RtlGetVersion"))
        : nullptr;
    if (rtlGetVersion) {
        (void)rtlGetVersion(&version);
    }
    return version;
}

std::string Utf8FromWide(const wchar_t* text)
{
    if (!text || *text == L'\0') {
        return {};
    }
    const int required = WideCharToMultiByte(
        CP_UTF8, 0, text, -1, nullptr, 0, nullptr, nullptr);
    if (required <= 1) {
        return {};
    }
    std::string result(static_cast<std::size_t>(required), '\0');
    (void)WideCharToMultiByte(
        CP_UTF8, 0, text, -1, result.data(), required,
        nullptr, nullptr);
    result.pop_back();
    return result;
}

std::string NativeArchitectureName(WORD architecture)
{
    switch (architecture) {
    case PROCESSOR_ARCHITECTURE_AMD64:
        return "x64";
    case PROCESSOR_ARCHITECTURE_ARM64:
        return "ARM64";
    case PROCESSOR_ARCHITECTURE_INTEL:
        return "x86";
    case PROCESSOR_ARCHITECTURE_ARM:
        return "ARM";
    case PROCESSOR_ARCHITECTURE_IA64:
        return "IA64";
    default:
        return "unknown";
    }
}

std::string GraphicsVendorName(std::uint32_t vendorId)
{
    switch (vendorId) {
    case 0x8086:
        return "Intel";
    case 0x1002:
    case 0x1022:
        return "AMD";
    case 0x10DE:
        return "NVIDIA";
    case 0x1414:
        return "Microsoft";
    default:
        return "Other";
    }
}

std::string DriverVersionText(const LARGE_INTEGER& version)
{
    std::ostringstream stream;
    stream << HIWORD(version.HighPart) << '.'
           << LOWORD(version.HighPart) << '.'
           << HIWORD(version.LowPart) << '.'
           << LOWORD(version.LowPart);
    return stream.str();
}

struct AdapterProfile {
    std::string persistentIdentity;
    std::string displayDescription;
    bool software = false;
};

std::optional<AdapterProfile> ReadAdapterProfile(IDXGIAdapter1* adapter)
{
    DXGI_ADAPTER_DESC1 description{};
    if (!adapter || FAILED(adapter->GetDesc1(&description))) {
        return std::nullopt;
    }

    LARGE_INTEGER driverVersion{};
    const bool hasDriverVersion = SUCCEEDED(
        adapter->CheckInterfaceSupport(
            __uuidof(IDXGIDevice), &driverVersion));
    AdapterProfile profile;
    // Microsoft Basic Render Driver can be exposed with Flags == 0 inside
    // an RDP session even though it has no dedicated video memory and is not
    // a physical GPU. Do not persist it as a hardware capability.
    const bool microsoftBasicRenderDriver =
        description.VendorId == 0x1414 &&
        description.DeviceId == 0x008C &&
        description.DedicatedVideoMemory == 0;
    profile.software =
        (description.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) != 0 ||
        microsoftBasicRenderDriver;

    std::ostringstream identity;
    identity << std::hex << std::uppercase
             << description.VendorId << ':'
             << description.DeviceId << ':'
             << description.SubSysId << ':'
             << description.Revision << ':'
             << description.Flags;
    if (hasDriverVersion) {
        identity << ':'
                 << static_cast<std::uint32_t>(
                        driverVersion.HighPart)
                 << ':'
                 << static_cast<std::uint32_t>(
                        driverVersion.LowPart);
    } else {
        identity << ":driver-version-unavailable";
    }
    profile.persistentIdentity = identity.str();

    std::ostringstream display;
    const std::string name = Utf8FromWide(description.Description);
    display << (name.empty() ? "Unnamed DXGI adapter" : name)
            << " | vendor="
            << GraphicsVendorName(description.VendorId)
            << " | PCI=" << std::hex << std::uppercase
            << description.VendorId << ':'
            << description.DeviceId << std::dec
            << " | dedicatedVideoMemory="
            << (description.DedicatedVideoMemory /
                (1024ULL * 1024ULL))
            << " MiB"
            << " | type="
            << (profile.software ? "software" : "hardware")
            << " | driver="
            << (hasDriverVersion
                    ? DriverVersionText(driverVersion)
                    : "unavailable");
    profile.displayDescription = display.str();
    return profile;
}

}  // namespace

WindowsCompatibilityProfile QueryWindowsCompatibilityProfile()
{
    WindowsCompatibilityProfile profile;
    const auto version = ReadWindowsVersion();
    {
        std::ostringstream operatingSystem;
        operatingSystem << "Windows "
                        << version.dwMajorVersion << '.'
                        << version.dwMinorVersion
                        << " build "
                        << version.dwBuildNumber;
        profile.operatingSystem = operatingSystem.str();
    }
    SYSTEM_INFO systemInfo{};
    GetNativeSystemInfo(&systemInfo);
    profile.nativeArchitecture =
        NativeArchitectureName(
            systemInfo.wProcessorArchitecture);
    profile.remoteSession =
        GetSystemMetrics(SM_REMOTESESSION) != 0;

    std::ostringstream fingerprint;
    fingerprint << "remote-media-v2"
                << "|windows=" << version.dwMajorVersion << '.'
                << version.dwMinorVersion << '.'
                << version.dwBuildNumber
                << "|rdp="
                << (profile.remoteSession ? '1' : '0');

    IDXGIFactory1* factory = nullptr;
    if (SUCCEEDED(CreateDXGIFactory1(
            __uuidof(IDXGIFactory1),
            reinterpret_cast<void**>(&factory))) &&
        factory) {
        std::vector<std::string> adapters;
        for (UINT index = 0;; ++index) {
            IDXGIAdapter1* adapter = nullptr;
            const HRESULT result =
                factory->EnumAdapters1(index, &adapter);
            if (result == DXGI_ERROR_NOT_FOUND) {
                break;
            }
            if (FAILED(result)) {
                std::ostringstream error;
                error << "DXGI adapter enumeration failed: HRESULT 0x"
                      << std::hex << std::uppercase
                      << static_cast<std::uint32_t>(result);
                profile.graphicsEnumerationError = error.str();
                fingerprint << "|gpu-enumeration-error="
                            << std::hex << std::uppercase
                            << static_cast<std::uint32_t>(result)
                            << std::dec;
                if (adapter) {
                    adapter->Release();
                }
                break;
            }
            if (auto adapterProfile =
                    ReadAdapterProfile(adapter)) {
                profile.graphicsAdapters.push_back(
                    adapterProfile->displayDescription);
                if (!adapterProfile->software) {
                    adapters.push_back(
                        std::move(
                            adapterProfile->persistentIdentity));
                }
            }
            adapter->Release();
        }
        std::sort(adapters.begin(), adapters.end());
        if (adapters.empty()) {
            fingerprint << "|gpu=none";
        } else {
            for (const auto& adapter : adapters) {
                fingerprint << "|gpu=" << adapter;
            }
        }
        factory->Release();
    } else {
        profile.graphicsEnumerationError =
            "CreateDXGIFactory1 failed; GPU adapters are unavailable.";
        fingerprint << "|gpu=unavailable";
    }
    profile.hardwareFingerprint = fingerprint.str();
    return profile;
}

std::string BuildWindowsHardwareFingerprint()
{
    return QueryWindowsCompatibilityProfile().hardwareFingerprint;
}

}  // namespace remote
