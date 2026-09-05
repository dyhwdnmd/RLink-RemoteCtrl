// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#include "DxgiNativeDesktopCapturer.h"

#include <array>
#include <sstream>

#include <Windows.h>
#include <d3d10_1.h>
#include <d3d11.h>
#include <dxgi1_2.h>
#include <wrl/client.h>

#include "api/make_ref_counted.h"
#include "D3D11DesktopFrameBuffer.h"

namespace remote {
namespace {

using Microsoft::WRL::ComPtr;

std::string HResultText(const char* operation, HRESULT result)
{
    std::ostringstream stream;
    stream << operation << " failed with HRESULT 0x"
           << std::hex << std::uppercase
           << static_cast<unsigned long>(result) << '.';
    return stream.str();
}

bool IsPrimaryOutput(const DXGI_OUTPUT_DESC& description)
{
    return description.AttachedToDesktop &&
           description.DesktopCoordinates.left == 0 &&
           description.DesktopCoordinates.top == 0;
}

}  // namespace

class DxgiNativeDesktopCapturer::Impl final {
public:
    ~Impl()
    {
        Reset();
    }

    void Reset()
    {
        duplication.Reset();
        output.Reset();
        context.Reset();
        device.Reset();
        adapter.Reset();
        lastFrame = nullptr;
        PublishDesktopCaptureD3D11Device(nullptr);
    }

    bool Initialize(const DisplayDescriptor& target)
    {
        Reset();
        ComPtr<IDXGIFactory1> factory;
        HRESULT result = CreateDXGIFactory1(
            IID_PPV_ARGS(&factory));
        if (FAILED(result)) {
            error = HResultText("CreateDXGIFactory1", result);
            return false;
        }

        ComPtr<IDXGIAdapter1> firstAdapter;
        ComPtr<IDXGIOutput> firstOutput;
        const bool targetSpecified =
            !target.stableDisplayKey.empty();
        for (UINT adapterIndex = 0;; ++adapterIndex) {
            ComPtr<IDXGIAdapter1> candidateAdapter;
            if (factory->EnumAdapters1(
                    adapterIndex, &candidateAdapter) ==
                DXGI_ERROR_NOT_FOUND) {
                break;
            }
            DXGI_ADAPTER_DESC1 adapterDescription{};
            if (FAILED(candidateAdapter->GetDesc1(
                    &adapterDescription))) {
                continue;
            }
            const std::uint64_t candidateLuid =
                static_cast<std::uint32_t>(
                    adapterDescription.AdapterLuid.LowPart) |
                (static_cast<std::uint64_t>(
                     static_cast<std::uint32_t>(
                         adapterDescription.AdapterLuid.HighPart))
                 << 32);
            for (UINT outputIndex = 0;; ++outputIndex) {
                ComPtr<IDXGIOutput> candidateOutput;
                if (candidateAdapter->EnumOutputs(
                        outputIndex, &candidateOutput) ==
                    DXGI_ERROR_NOT_FOUND) {
                    break;
                }
                DXGI_OUTPUT_DESC description{};
                if (FAILED(candidateOutput->GetDesc(&description)) ||
                    !description.AttachedToDesktop) {
                    continue;
                }
                if (!firstOutput) {
                    firstAdapter = candidateAdapter;
                    firstOutput = candidateOutput;
                }
                const bool selectedTarget =
                    targetSpecified &&
                    target.adapterLuid == candidateLuid &&
                    target.outputIndex == outputIndex;
                if (selectedTarget ||
                    (!targetSpecified &&
                     IsPrimaryOutput(description))) {
                    adapter = candidateAdapter;
                    output = candidateOutput;
                    break;
                }
            }
            if (output) {
                break;
            }
        }
        if (!output && !targetSpecified) {
            adapter = firstAdapter;
            output = firstOutput;
        }
        if (!adapter || !output) {
            error = targetSpecified
                ? "The selected DXGI output is no longer available."
                : "No attached DXGI output is available.";
            return false;
        }

        constexpr D3D_FEATURE_LEVEL levels[] = {
            D3D_FEATURE_LEVEL_11_1,
            D3D_FEATURE_LEVEL_11_0,
            D3D_FEATURE_LEVEL_10_1,
            D3D_FEATURE_LEVEL_10_0,
        };
        D3D_FEATURE_LEVEL selected{};
        result = D3D11CreateDevice(
            adapter.Get(),
            D3D_DRIVER_TYPE_UNKNOWN,
            nullptr,
            D3D11_CREATE_DEVICE_BGRA_SUPPORT |
                D3D11_CREATE_DEVICE_VIDEO_SUPPORT,
            levels,
            static_cast<UINT>(std::size(levels)),
            D3D11_SDK_VERSION,
            &device,
            &selected,
            &context);
        if (FAILED(result) || !device || !context) {
            error = HResultText("D3D11CreateDevice", result);
            Reset();
            return false;
        }

        ComPtr<ID3D10Multithread> multithread;
        if (SUCCEEDED(device.As(&multithread)) && multithread) {
            multithread->SetMultithreadProtected(TRUE);
        }

        ComPtr<IDXGIOutput1> output1;
        result = output.As(&output1);
        if (FAILED(result) || !output1) {
            error = HResultText("IDXGIOutput1", result);
            Reset();
            return false;
        }
        DXGI_OUTPUT_DESC description{};
        output->GetDesc(&description);
        if (description.Rotation != DXGI_MODE_ROTATION_IDENTITY &&
            description.Rotation != DXGI_MODE_ROTATION_UNSPECIFIED) {
            error =
                "The native DXGI path currently requires an unrotated display.";
            Reset();
            return false;
        }
        result = output1->DuplicateOutput(
            device.Get(), &duplication);
        if (FAILED(result) || !duplication) {
            error = HResultText("DuplicateOutput", result);
            Reset();
            return false;
        }
        PublishDesktopCaptureD3D11Device(device.Get());
        error.clear();
        return true;
    }

    Result Capture(
        std::uint32_t timeoutMs,
        webrtc::scoped_refptr<D3D11DesktopFrameBuffer>* resultFrame)
    {
        if (!resultFrame || !duplication || !device || !context) {
            error = "The native DXGI capturer is not initialized.";
            return Result::kFailed;
        }
        DXGI_OUTDUPL_FRAME_INFO information{};
        ComPtr<IDXGIResource> resource;
        const HRESULT acquire = duplication->AcquireNextFrame(
            timeoutMs, &information, &resource);
        if (acquire == DXGI_ERROR_WAIT_TIMEOUT) {
            *resultFrame = lastFrame;
            return lastFrame ? Result::kTimeout : Result::kFailed;
        }
        if (FAILED(acquire) || !resource) {
            error = HResultText("AcquireNextFrame", acquire);
            return Result::kFailed;
        }

        ComPtr<ID3D11Texture2D> desktopTexture;
        HRESULT result = resource.As(&desktopTexture);
        if (FAILED(result) || !desktopTexture) {
            duplication->ReleaseFrame();
            error = HResultText("Desktop texture query", result);
            return Result::kFailed;
        }
        D3D11_TEXTURE2D_DESC source{};
        desktopTexture->GetDesc(&source);
        if (source.Format != DXGI_FORMAT_B8G8R8A8_UNORM &&
            source.Format != DXGI_FORMAT_B8G8R8A8_UNORM_SRGB) {
            duplication->ReleaseFrame();
            error = "The native DXGI desktop texture is not BGRA8.";
            return Result::kFailed;
        }

        D3D11_TEXTURE2D_DESC copy = source;
        copy.Usage = D3D11_USAGE_DEFAULT;
        copy.CPUAccessFlags = 0;
        copy.MiscFlags = 0;
        copy.BindFlags =
            D3D11_BIND_SHADER_RESOURCE |
            D3D11_BIND_RENDER_TARGET;
        ComPtr<ID3D11Texture2D> ownedTexture;
        result = device->CreateTexture2D(
            &copy, nullptr, &ownedTexture);
        if (SUCCEEDED(result)) {
            context->CopyResource(
                ownedTexture.Get(), desktopTexture.Get());
        }
        const HRESULT release = duplication->ReleaseFrame();
        if (FAILED(result) || !ownedTexture || FAILED(release)) {
            error = FAILED(result)
                ? HResultText("Create desktop texture", result)
                : HResultText("ReleaseFrame", release);
            return Result::kFailed;
        }

        lastFrame =
            webrtc::make_ref_counted<D3D11DesktopFrameBuffer>(
                ownedTexture.Get(),
                static_cast<int>(source.Width),
                static_cast<int>(source.Height));
        *resultFrame = lastFrame;
        error.clear();
        return Result::kSuccess;
    }

    ComPtr<IDXGIAdapter1> adapter;
    ComPtr<IDXGIOutput> output;
    ComPtr<ID3D11Device> device;
    ComPtr<ID3D11DeviceContext> context;
    ComPtr<IDXGIOutputDuplication> duplication;
    webrtc::scoped_refptr<D3D11DesktopFrameBuffer> lastFrame;
    std::string error;
};

DxgiNativeDesktopCapturer::DxgiNativeDesktopCapturer()
    : impl_(std::make_unique<Impl>())
{}

DxgiNativeDesktopCapturer::~DxgiNativeDesktopCapturer() = default;

bool DxgiNativeDesktopCapturer::Initialize(
    const DisplayDescriptor& target)
{
    return impl_->Initialize(target);
}

DxgiNativeDesktopCapturer::Result
DxgiNativeDesktopCapturer::Capture(
    std::uint32_t timeoutMs,
    webrtc::scoped_refptr<D3D11DesktopFrameBuffer>* frame)
{
    return impl_->Capture(timeoutMs, frame);
}

std::string DxgiNativeDesktopCapturer::LastError() const
{
    return impl_->error;
}

}  // namespace remote
