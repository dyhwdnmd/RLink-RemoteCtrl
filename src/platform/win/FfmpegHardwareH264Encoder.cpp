// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#include "FfmpegHardwareH264Encoder.h"

#include <Windows.h>
#include <d3d11.h>
#include <dxgi1_2.h>
#include <wrl/client.h>

#include <algorithm>
#include <array>
#include <cstdint>
#include <cstring>
#include <deque>
#include <filesystem>
#include <mutex>
#include <string>
#include <utility>
#include <vector>

extern "C" {
#include "libavcodec/avcodec.h"
#include "libavcodec/packet.h"
#include "libavutil/buffer.h"
#include "libavutil/error.h"
#include "libavutil/frame.h"
#include "libavutil/hwcontext.h"
#include "libavutil/hwcontext_d3d11va.h"
#include "libavutil/opt.h"
#include "libavutil/pixfmt.h"
}

#include "api/make_ref_counted.h"
#include "api/video/encoded_image.h"
#include "api/video/i420_buffer.h"
#include "api/video/video_frame.h"
#include "api/video_codecs/video_codec.h"
#include "modules/video_coding/codecs/h264/include/h264_globals.h"
#include "modules/video_coding/include/video_codec_interface.h"
#include "modules/video_coding/include/video_error_codes.h"
#include "D3D11DesktopFrameBuffer.h"
#include "DesktopBgraFrameBuffer.h"
#include "libyuv/convert_from.h"

namespace remote {
namespace {

using Microsoft::WRL::ComPtr;

struct BackendDescriptor {
    FfmpegHardwareBackend backend;
    const char* codecName;
    const char* implementation;
    UINT vendorId;
    const wchar_t* driverModule;
};

constexpr BackendDescriptor kBackendDescriptors[] = {
    {FfmpegHardwareBackend::kQsv,
     "h264_qsv", "FFmpeg/QSV", 0x8086, nullptr},
    {FfmpegHardwareBackend::kNvenc,
     "h264_nvenc", "FFmpeg/NVENC", 0x10DE, L"nvEncodeAPI64.dll"},
    {FfmpegHardwareBackend::kAmf,
     "h264_amf", "FFmpeg/AMF", 0x1002, L"amfrt64.dll"},
};

const BackendDescriptor* DescriptorFor(FfmpegHardwareBackend backend)
{
    const auto found = std::find_if(
        std::begin(kBackendDescriptors), std::end(kBackendDescriptors),
        [backend](const BackendDescriptor& candidate) {
            return candidate.backend == backend;
        });
    return found == std::end(kBackendDescriptors) ? nullptr : &*found;
}

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

bool SystemModuleAvailable(const wchar_t* moduleName)
{
    if (!moduleName) {
        return true;
    }
    HMODULE module = LoadLibraryExW(
        moduleName, nullptr, LOAD_LIBRARY_SEARCH_SYSTEM32);
    if (!module) {
        return false;
    }
    FreeLibrary(module);
    return true;
}

struct HardwareAdapterDescriptor {
    UINT vendorId = 0;
    std::uint64_t luid = 0;
    bool drivesDisplay = false;
};

std::uint64_t PackAdapterLuid(const LUID& luid)
{
    return static_cast<std::uint64_t>(
               static_cast<std::uint32_t>(luid.LowPart)) |
           (static_cast<std::uint64_t>(
                static_cast<std::uint32_t>(luid.HighPart)) << 32);
}

std::vector<HardwareAdapterDescriptor> HardwareAdapters()
{
    std::vector<HardwareAdapterDescriptor> adapters;
    ComPtr<IDXGIFactory1> factory;
    if (FAILED(CreateDXGIFactory1(IID_PPV_ARGS(&factory)))) {
        return adapters;
    }
    for (UINT index = 0;; ++index) {
        ComPtr<IDXGIAdapter1> adapter;
        if (factory->EnumAdapters1(index, &adapter) == DXGI_ERROR_NOT_FOUND) {
            break;
        }
        DXGI_ADAPTER_DESC1 description{};
        if (FAILED(adapter->GetDesc1(&description)) ||
            (description.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) != 0) {
            continue;
        }
        ComPtr<IDXGIOutput> output;
        const bool drivesDisplay = SUCCEEDED(adapter->EnumOutputs(0, &output));
        adapters.push_back({
            .vendorId = description.VendorId,
            .luid = PackAdapterLuid(description.AdapterLuid),
            .drivesDisplay = drivesDisplay,
        });
    }
    std::stable_sort(
        adapters.begin(), adapters.end(),
        [](const auto& left, const auto& right) {
            return left.drivesDisplay && !right.drivesDisplay;
        });
    return adapters;
}

bool HasVendor(UINT vendorId)
{
    const auto adapters = HardwareAdapters();
    return std::any_of(
        adapters.begin(), adapters.end(),
        [vendorId](const auto& adapter) {
            return adapter.vendorId == vendorId;
        });
}

class FfmpegHardwareApi final {
public:
    static FfmpegHardwareApi& Instance()
    {
        static auto* instance = new FfmpegHardwareApi();
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
    decltype(&::avcodec_get_hw_config) avcodecGetHwConfig = nullptr;
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
    decltype(&::av_frame_unref) avFrameUnref = nullptr;
    decltype(&::av_buffer_ref) avBufferRef = nullptr;
    decltype(&::av_buffer_unref) avBufferUnref = nullptr;
    decltype(&::av_hwdevice_ctx_alloc) avHwDeviceCtxAlloc = nullptr;
    decltype(&::av_hwdevice_ctx_init) avHwDeviceCtxInit = nullptr;
    decltype(&::av_hwframe_ctx_alloc) avHwFrameCtxAlloc = nullptr;
    decltype(&::av_hwframe_ctx_init) avHwFrameCtxInit = nullptr;
    decltype(&::av_hwframe_get_buffer) avHwFrameGetBuffer = nullptr;
    decltype(&::av_opt_set) avOptSet = nullptr;
    decltype(&::av_opt_set_int) avOptSetInt = nullptr;
    decltype(&::av_strerror) avStrError = nullptr;

private:
    void Load()
    {
        const auto directory = ExecutableDirectory();
        if (directory.empty()) {
            error_ = "Cannot resolve the RemoteC executable directory.";
            return;
        }
        avutilModule_ = LoadLibraryExW(
            (directory / L"avutil-60.dll").c_str(), nullptr,
            LOAD_LIBRARY_SEARCH_DLL_LOAD_DIR |
                LOAD_LIBRARY_SEARCH_DEFAULT_DIRS);
        if (!avutilModule_) {
            error_ = "avutil-60.dll is missing or cannot be loaded.";
            return;
        }
        // The combined FFmpeg build still contains libx264. Loading it first
        // keeps avcodec loadable while hardware backends remain optional.
        x264Module_ = LoadLibraryExW(
            (directory / L"libx264-165.dll").c_str(), nullptr,
            LOAD_LIBRARY_SEARCH_DLL_LOAD_DIR |
                LOAD_LIBRARY_SEARCH_DEFAULT_DIRS);
        if (!x264Module_) {
            error_ = "libx264-165.dll is missing or cannot be loaded.";
            return;
        }
        avcodecModule_ = LoadLibraryExW(
            (directory / L"avcodec-62.dll").c_str(), nullptr,
            LOAD_LIBRARY_SEARCH_DLL_LOAD_DIR |
                LOAD_LIBRARY_SEARCH_DEFAULT_DIRS);
        if (!avcodecModule_) {
            error_ = "avcodec-62.dll or one of its application runtimes "
                     "is missing or cannot be loaded.";
            return;
        }
        const bool codecFunctions =
            ResolveFunction(avcodecModule_, "avcodec_find_encoder_by_name",
                            &avcodecFindEncoderByName) &&
            ResolveFunction(avcodecModule_, "avcodec_get_hw_config",
                            &avcodecGetHwConfig) &&
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
            ResolveFunction(avutilModule_, "av_frame_unref",
                            &avFrameUnref) &&
            ResolveFunction(avutilModule_, "av_buffer_ref",
                            &avBufferRef) &&
            ResolveFunction(avutilModule_, "av_buffer_unref",
                            &avBufferUnref) &&
            ResolveFunction(avutilModule_, "av_hwdevice_ctx_alloc",
                            &avHwDeviceCtxAlloc) &&
            ResolveFunction(avutilModule_, "av_hwdevice_ctx_init",
                            &avHwDeviceCtxInit) &&
            ResolveFunction(avutilModule_, "av_hwframe_ctx_alloc",
                            &avHwFrameCtxAlloc) &&
            ResolveFunction(avutilModule_, "av_hwframe_ctx_init",
                            &avHwFrameCtxInit) &&
            ResolveFunction(avutilModule_, "av_hwframe_get_buffer",
                            &avHwFrameGetBuffer) &&
            ResolveFunction(avutilModule_, "av_opt_set", &avOptSet) &&
            ResolveFunction(avutilModule_, "av_opt_set_int", &avOptSetInt) &&
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
    HMODULE x264Module_ = nullptr;
    bool loaded_ = false;
    std::string error_;
};

std::string FfmpegError(FfmpegHardwareApi& api,
                        const char* operation,
                        int result)
{
    char description[AV_ERROR_MAX_STRING_SIZE]{};
    if (!api.avStrError ||
        api.avStrError(result, description, sizeof(description)) < 0) {
        return std::string(operation) + " failed with FFmpeg error " +
               std::to_string(result) + ".";
    }
    return std::string(operation) + " failed: " + description +
           " (FFmpeg error " + std::to_string(result) + ").";
}

std::vector<const BackendDescriptor*> CandidateBackends(
    FfmpegHardwareBackend requested,
    std::uint64_t preferredAdapterLuid)
{
    if (const auto* selected = DescriptorFor(requested)) {
        return {selected};
    }
    std::vector<const BackendDescriptor*> candidates;
    auto adapters = HardwareAdapters();
    if (preferredAdapterLuid != 0) {
        const auto selected = std::find_if(
            adapters.begin(), adapters.end(),
            [preferredAdapterLuid](const auto& adapter) {
                return adapter.luid == preferredAdapterLuid;
            });
        if (selected != adapters.end()) {
            std::rotate(adapters.begin(), selected, selected + 1);
        }
    }
    for (const auto& adapter : adapters) {
        for (const auto& descriptor : kBackendDescriptors) {
            if (descriptor.vendorId == adapter.vendorId &&
                std::find(candidates.begin(), candidates.end(),
                          &descriptor) == candidates.end()) {
                candidates.push_back(&descriptor);
            }
        }
    }
    for (const auto& descriptor : kBackendDescriptors) {
        if (std::find(candidates.begin(), candidates.end(), &descriptor) ==
            candidates.end()) {
            candidates.push_back(&descriptor);
        }
    }
    return candidates;
}

bool AnnexBStartCode(const uint8_t* data, std::size_t size)
{
    if (!data || size < 4) {
        return false;
    }
    for (std::size_t index = 0; index + 3 < size && index < 128; ++index) {
        if (data[index] == 0 && data[index + 1] == 0 &&
            (data[index + 2] == 1 ||
             (data[index + 2] == 0 && data[index + 3] == 1))) {
            return true;
        }
    }
    return false;
}

const char* NvencPresetForQuality(FfmpegX264Preset quality)
{
    return VideoEncoderQualityProfileForPreset(quality).nvencPreset;
}

const char* QsvPresetForQuality(FfmpegX264Preset quality)
{
    return VideoEncoderQualityProfileForPreset(quality).qsvPreset;
}

const char* AmfQualityForPreset(FfmpegX264Preset quality)
{
    return VideoEncoderQualityProfileForPreset(quality).amfQuality;
}

const char* HardwareQualityOption(
    FfmpegHardwareBackend backend, FfmpegX264Preset quality)
{
    switch (backend) {
    case FfmpegHardwareBackend::kNvenc:
        return NvencPresetForQuality(quality);
    case FfmpegHardwareBackend::kQsv:
        return QsvPresetForQuality(quality);
    case FfmpegHardwareBackend::kAmf:
        return AmfQualityForPreset(quality);
    case FfmpegHardwareBackend::kAutomatic:
        return "auto";
    }
    return "auto";
}

}  // namespace

class FfmpegHardwareH264Encoder::Impl final {
public:
    Impl(FfmpegHardwareBackend requestedBackend,
         std::shared_ptr<VideoEncoderRuntimeState> runtimeState,
         FfmpegX264Preset quality)
        : requestedBackend_(requestedBackend),
          runtimeState_(std::move(runtimeState)),
          quality_(quality)
    {}

    ~Impl() { Release(); }

    const std::string& ImplementationName() const noexcept
    {
        return implementationName_;
    }

    std::string LastError() const
    {
        std::lock_guard lock(mutex_);
        return lastError_;
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
        auto& api = FfmpegHardwareApi::Instance();
        if (!api.EnsureLoaded()) {
            return RequestSoftwareFallback(api.Error());
        }

        const BackendDescriptor* selected = nullptr;
        const AVCodec* encoder = nullptr;
        std::string unavailable;
        const std::uint64_t preferredAdapterLuid = runtimeState_
            ? runtimeState_->DesktopCaptureAdapterLuid()
            : 0;
        for (const auto* candidate : CandidateBackends(
                 requestedBackend_, preferredAdapterLuid)) {
            if (!api.avcodecFindEncoderByName(candidate->codecName)) {
                unavailable += std::string(candidate->implementation) +
                    " is not compiled into the bundled FFmpeg runtime; ";
                continue;
            }
            if (!HasVendor(candidate->vendorId)) {
                unavailable += std::string(candidate->implementation) +
                    " has no matching hardware adapter; ";
                continue;
            }
            if (!SystemModuleAvailable(candidate->driverModule)) {
                unavailable += std::string(candidate->implementation) +
                    " driver runtime is unavailable; ";
                continue;
            }
            selected = candidate;
            encoder = api.avcodecFindEncoderByName(candidate->codecName);
            break;
        }
        if (!selected || !encoder) {
            if (unavailable.empty()) {
                unavailable = "No FFmpeg hardware H264 encoder is available.";
            }
            return RequestSoftwareFallback(unavailable);
        }

        selectedBackend_ = selected->backend;
        implementationName_ = std::string(selected->implementation) +
            " (" + HardwareQualityOption(selectedBackend_, quality_) + ")";
        if (runtimeState_) {
            runtimeInstanceId_ = runtimeState_->RegisterHardwareEncoder(
                implementationName_);
        }
        codecContext_ = api.avcodecAllocContext3(encoder);
        inputFrame_ = api.avFrameAlloc();
        outputPacket_ = api.avPacketAlloc();
        if (!codecContext_ || !inputFrame_ || !outputPacket_) {
            return RequestSoftwareFallback(
                "FFmpeg could not allocate hardware encoder state.");
        }

        width_ = codecSettings->width;
        height_ = codecSettings->height;
        configuredFrameRate_ = (std::max<std::uint32_t>)(
            codecSettings->maxFramerate, 1);
        frameRate_ = configuredFrameRate_;
        requestedBitrateBps_ = (std::max<std::uint32_t>)(
            codecSettings->startBitrate * 1000, 100'000);
        maximumBitrateBps_ = (std::max<std::uint32_t>)(
            codecSettings->maxBitrate * 1000, requestedBitrateBps_);
        startupKeyFramePending_ =
            codecSettings->mode == webrtc::VideoCodecMode::kScreensharing;
        // The encoder must start at libwebrtc's current allocation. A private
        // multi-megabit floor makes the first desktop IDR much larger than
        // GoogCC's still-cold budget; WebRTC then drops nearly every following
        // source frame and cannot probe upward. The PeerConnection owns the
        // start-bitrate prior and screencast padding, while NVENC follows the
        // actual rate allocation and preserves motion with a temporarily
        // higher QP.
        bitrateBps_ = requestedBitrateBps_;

        // Native DXGI publishes its exact D3D11 device before WebRTC creates
        // the encoder. When the selected FFmpeg backend accepts D3D11 frames,
        // bind that same adapter and keep BGRA -> NV12 entirely on the GPU.
        // libwebrtc CPU capture and unsupported backends retain the proven
        // CPU NV12 upload path below.
        InitializeD3D11Input(api, encoder, selected->vendorId);

        codecContext_->codec_type = AVMEDIA_TYPE_VIDEO;
        codecContext_->codec_id = AV_CODEC_ID_H264;
        codecContext_->width = static_cast<int>(width_);
        codecContext_->height = static_cast<int>(height_);
        codecContext_->pix_fmt = useD3D11Input_
            ? AV_PIX_FMT_D3D11
            : AV_PIX_FMT_NV12;
        if (useD3D11Input_) {
            codecContext_->hw_frames_ctx =
                api.avBufferRef(hardwareFrames_);
            if (!codecContext_->hw_frames_ctx) {
                return RequestSoftwareFallback(
                    "FFmpeg could not retain the D3D11 input frame pool.");
            }
        }
        codecContext_->time_base = {1, static_cast<int>(frameRate_)};
        codecContext_->framerate = {static_cast<int>(frameRate_), 1};
        codecContext_->bit_rate = bitrateBps_;
        const bool nvencRateControl =
            selectedBackend_ == FfmpegHardwareBackend::kNvenc;
        codecContext_->rc_max_rate = nvencRateControl
            ? bitrateBps_
            : maximumBitrateBps_;
        const std::uint64_t rateControlBufferBps = nvencRateControl
            ? (std::max<std::uint64_t>)(bitrateBps_ / 2, 100'000)
            : maximumBitrateBps_;
        codecContext_->rc_buffer_size = static_cast<int>((std::min)(
            rateControlBufferBps,
            static_cast<std::uint64_t>(INT_MAX)));
        codecContext_->gop_size = static_cast<int>(frameRate_ * 2);
        codecContext_->max_b_frames = 0;
        codecContext_->flags |= AV_CODEC_FLAG_LOW_DELAY;
        codecContext_->thread_count = 1;

        if (!ConfigureBackendOptions(api, selectedBackend_)) {
            return RequestSoftwareFallback(lastError_);
        }
        const int openResult =
            api.avcodecOpen2(codecContext_, encoder, nullptr);
        if (openResult < 0) {
            return RequestSoftwareFallback(FfmpegError(
                api, implementationName_.c_str(), openResult));
        }
        inputFrame_->format = useD3D11Input_
            ? AV_PIX_FMT_D3D11
            : AV_PIX_FMT_NV12;
        inputFrame_->width = static_cast<int>(width_);
        inputFrame_->height = static_cast<int>(height_);
        if (!useD3D11Input_) {
            const int bufferResult = api.avFrameGetBuffer(inputFrame_, 32);
            if (bufferResult < 0) {
                return RequestSoftwareFallback(FfmpegError(
                    api, "Allocate FFmpeg hardware input frame",
                    bufferResult));
            }
        }
        initialized_ = true;
        lastBitrateApplyTime_ = std::chrono::steady_clock::now();
        pendingBitrateBps_ = bitrateBps_;
        if (runtimeState_) {
            runtimeState_->MarkHardwareInitialized(
                runtimeInstanceId_, width_, height_, frameRate_,
                codecSettings->minBitrate * 1000,
                bitrateBps_, maximumBitrateBps_);
            runtimeState_->MarkRates(
                runtimeInstanceId_, frameRate_, bitrateBps_, frameRate_, 0,
                bitrateBps_, true, false);
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
        std::lock_guard lock(mutex_);
        if (!initialized_ || !codecContext_ || !callback_) {
            return WEBRTC_VIDEO_CODEC_UNINITIALIZED;
        }
        MaybeApplyPendingBitrate();
        const auto source = frame.video_frame_buffer();
        if (!source || source->width() <= 0 || source->height() <= 0) {
            return RequestSoftwareFallback(
                "FFmpeg hardware encoder received an invalid input frame.");
        }
        ReportInputFormat(source);

        auto& api = FfmpegHardwareApi::Instance();
        if (useD3D11Input_) {
            if (!PrepareD3D11Frame(api, source)) {
                return RequestSoftwareFallback(
                    "FFmpeg could not convert the D3D11 desktop texture "
                    "to its hardware NV12 input frame.");
            }
        } else {
            webrtc::scoped_refptr<webrtc::VideoFrameBuffer>
                normalizedSource = source;
            if (source->width() != static_cast<int>(width_) ||
                source->height() != static_cast<int>(height_)) {
                normalizedSource = source->CropAndScale(
                    0, 0, source->width(), source->height(),
                    static_cast<int>(width_),
                    static_cast<int>(height_));
            }
            const auto i420 = normalizedSource
                ? normalizedSource->ToI420()
                : nullptr;
            if (!i420) {
                return RequestSoftwareFallback(
                    "The input frame cannot be mapped to CPU I420 for the "
                    "FFmpeg hardware upload path.");
            }
            const int writableResult = api.avFrameMakeWritable(inputFrame_);
            if (writableResult < 0) {
                return RequestSoftwareFallback(FfmpegError(
                    api, "Make FFmpeg hardware input writable",
                    writableResult));
            }
            for (int row = 0; row < static_cast<int>(height_); ++row) {
                std::memcpy(
                    inputFrame_->data[0] + row * inputFrame_->linesize[0],
                    i420->DataY() + row * i420->StrideY(), width_);
            }
            for (int row = 0; row < static_cast<int>(height_ / 2); ++row) {
                uint8_t* destination =
                    inputFrame_->data[1] + row * inputFrame_->linesize[1];
                const uint8_t* sourceU =
                    i420->DataU() + row * i420->StrideU();
                const uint8_t* sourceV =
                    i420->DataV() + row * i420->StrideV();
                for (std::uint32_t column = 0;
                     column < width_ / 2; ++column) {
                    destination[column * 2] = sourceU[column];
                    destination[column * 2 + 1] = sourceV[column];
                }
            }
        }

        const bool requestedKeyFrame =
            frameTypes &&
            std::find(frameTypes->begin(), frameTypes->end(),
                      webrtc::VideoFrameType::kVideoFrameKey) !=
                frameTypes->end();
        const bool startupKeyFrame = startupKeyFramePending_;
        const bool keyFrame = requestedKeyFrame || startupKeyFrame;
        const std::int64_t presentationId = nextPresentationId_++;
        inputFrame_->pts = presentationId;
        inputFrame_->pict_type =
            keyFrame ? AV_PICTURE_TYPE_I : AV_PICTURE_TYPE_NONE;
        if (pendingFrames_.size() >= 8) {
            return RequestSoftwareFallback(
                "FFmpeg hardware encoder exceeded the 8-frame low-latency "
                "backlog.");
        }
        pendingFrames_.push_back({presentationId, frame});
        int result = api.avcodecSendFrame(codecContext_, inputFrame_);
        if (result == AVERROR(EAGAIN)) {
            if (!DrainOutput()) {
                pendingFrames_.pop_back();
                return RequestSoftwareFallback(lastError_);
            }
            result = api.avcodecSendFrame(codecContext_, inputFrame_);
        }
        if (result < 0) {
            pendingFrames_.pop_back();
            return RequestSoftwareFallback(FfmpegError(
                api, "Submit frame to FFmpeg hardware encoder", result));
        }
        if (startupKeyFrame) {
            startupKeyFramePending_ = false;
        }
        if (!DrainOutput()) {
            return RequestSoftwareFallback(lastError_);
        }
        return WEBRTC_VIDEO_CODEC_OK;
    }

    void SetRates(const webrtc::VideoEncoder::RateControlParameters& parameters)
    {
        std::lock_guard lock(mutex_);
        const std::uint32_t bitrate = parameters.bitrate.get_sum_bps();
        if (bitrate != 0) {
            requestedBitrateBps_ = (std::min)(bitrate, maximumBitrateBps_);
            ScheduleBitrate(requestedBitrateBps_);
        }
        if (parameters.framerate_fps > 0.0) {
            frameRate_ = static_cast<std::uint32_t>((std::max)(
                parameters.framerate_fps, 1.0));
        }
        if (runtimeState_ && runtimeInstanceId_ != 0) {
            runtimeState_->MarkRates(
                runtimeInstanceId_, frameRate_, bitrateBps_, frameRate_, 0,
                bitrateBps_, true, false);
        }
    }

private:
    struct FrameMetadata {
        std::int64_t presentationId;
        webrtc::VideoFrame frame;
    };

    bool EncoderSupportsD3D11(FfmpegHardwareApi& api,
                              const AVCodec* encoder) const
    {
        if (!encoder || !api.avcodecGetHwConfig) {
            return false;
        }
        for (int index = 0;; ++index) {
            const AVCodecHWConfig* config =
                api.avcodecGetHwConfig(encoder, index);
            if (!config) {
                return false;
            }
            if (config->pix_fmt == AV_PIX_FMT_D3D11 &&
                (config->methods &
                 AV_CODEC_HW_CONFIG_METHOD_HW_FRAMES_CTX) != 0) {
                return true;
            }
        }
    }

    bool InitializeD3D11Input(FfmpegHardwareApi& api,
                              const AVCodec* encoder,
                              UINT selectedVendorId)
    {
        if (!EncoderSupportsD3D11(api, encoder)) {
            return false;
        }
        ComPtr<ID3D11Device> captureDevice;
        captureDevice.Attach(AcquireDesktopCaptureD3D11Device());
        if (!captureDevice) {
            return false;
        }
        ComPtr<IDXGIDevice> dxgiDevice;
        ComPtr<IDXGIAdapter> adapter;
        DXGI_ADAPTER_DESC adapterDescription{};
        if (FAILED(captureDevice.As(&dxgiDevice)) || !dxgiDevice ||
            FAILED(dxgiDevice->GetAdapter(&adapter)) || !adapter ||
            FAILED(adapter->GetDesc(&adapterDescription)) ||
            adapterDescription.VendorId != selectedVendorId) {
            return false;
        }

        hardwareDevice_ =
            api.avHwDeviceCtxAlloc(AV_HWDEVICE_TYPE_D3D11VA);
        if (!hardwareDevice_) {
            return false;
        }
        auto* deviceContext = reinterpret_cast<AVHWDeviceContext*>(
            hardwareDevice_->data);
        auto* d3d11Context =
            static_cast<AVD3D11VADeviceContext*>(deviceContext->hwctx);
        d3d11Context->device = captureDevice.Get();
        d3d11Context->device->AddRef();
        d3d11Context->BindFlags =
            D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
        if (api.avHwDeviceCtxInit(hardwareDevice_) < 0) {
            api.avBufferUnref(&hardwareDevice_);
            return false;
        }
        hardwareFrames_ = api.avHwFrameCtxAlloc(hardwareDevice_);
        if (!hardwareFrames_) {
            api.avBufferUnref(&hardwareDevice_);
            return false;
        }
        auto* frames = reinterpret_cast<AVHWFramesContext*>(
            hardwareFrames_->data);
        frames->format = AV_PIX_FMT_D3D11;
        frames->sw_format = AV_PIX_FMT_NV12;
        frames->width = static_cast<int>(width_);
        frames->height = static_cast<int>(height_);
        // Encoding does not require a decoder-style fixed array texture.
        // Individual NV12 textures are accepted by D3D11 video processing and
        // avoid driver restrictions on render-target array textures.
        frames->initial_pool_size = 0;
        auto* d3d11Frames =
            static_cast<AVD3D11VAFramesContext*>(frames->hwctx);
        d3d11Frames->BindFlags =
            D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
        if (api.avHwFrameCtxInit(hardwareFrames_) < 0) {
            api.avBufferUnref(&hardwareFrames_);
            api.avBufferUnref(&hardwareDevice_);
            return false;
        }
        if (FAILED(captureDevice.As(&videoDevice_))) {
            api.avBufferUnref(&hardwareFrames_);
            api.avBufferUnref(&hardwareDevice_);
            return false;
        }
        captureDevice->GetImmediateContext(&d3dContext_);
        if (!d3dContext_ || FAILED(d3dContext_.As(&videoContext_))) {
            videoDevice_.Reset();
            api.avBufferUnref(&hardwareFrames_);
            api.avBufferUnref(&hardwareDevice_);
            return false;
        }
        d3dDevice_ = std::move(captureDevice);
        useD3D11Input_ = true;
        return true;
    }

    bool EnsureVideoProcessor(UINT sourceWidth, UINT sourceHeight)
    {
        if (processor_ && processorEnumerator_ && uploadBgraTexture_ &&
            uploadInputView_ && processorSourceWidth_ == sourceWidth &&
            processorSourceHeight_ == sourceHeight) {
            return true;
        }
        processor_.Reset();
        processorEnumerator_.Reset();
        uploadInputView_.Reset();
        uploadBgraTexture_.Reset();

        D3D11_TEXTURE2D_DESC upload{};
        upload.Width = sourceWidth;
        upload.Height = sourceHeight;
        upload.MipLevels = 1;
        upload.ArraySize = 1;
        upload.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
        upload.SampleDesc.Count = 1;
        upload.Usage = D3D11_USAGE_DEFAULT;
        upload.BindFlags =
            D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
        if (FAILED(d3dDevice_->CreateTexture2D(
                &upload, nullptr, &uploadBgraTexture_))) {
            return false;
        }
        D3D11_VIDEO_PROCESSOR_CONTENT_DESC content{};
        content.InputFrameFormat = D3D11_VIDEO_FRAME_FORMAT_PROGRESSIVE;
        content.InputFrameRate = {frameRate_, 1};
        content.InputWidth = sourceWidth;
        content.InputHeight = sourceHeight;
        content.OutputFrameRate = {frameRate_, 1};
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
        D3D11_VIDEO_PROCESSOR_INPUT_VIEW_DESC inputView{};
        inputView.ViewDimension = D3D11_VPIV_DIMENSION_TEXTURE2D;
        if (FAILED(videoDevice_->CreateVideoProcessorInputView(
                uploadBgraTexture_.Get(), processorEnumerator_.Get(),
                &inputView, &uploadInputView_))) {
            return false;
        }
        videoContext_->VideoProcessorSetStreamFrameFormat(
            processor_.Get(), 0,
            D3D11_VIDEO_FRAME_FORMAT_PROGRESSIVE);
        videoContext_->VideoProcessorSetStreamAutoProcessingMode(
            processor_.Get(), 0, FALSE);
        processorSourceWidth_ = sourceWidth;
        processorSourceHeight_ = sourceHeight;
        return true;
    }

    bool PrepareD3D11Frame(
        FfmpegHardwareApi& api,
        const webrtc::scoped_refptr<webrtc::VideoFrameBuffer>& source)
    {
        if (!useD3D11Input_ || !hardwareFrames_ || !inputFrame_ ||
            !source) {
            return false;
        }
        api.avFrameUnref(inputFrame_);
        if (api.avHwFrameGetBuffer(
                hardwareFrames_, inputFrame_, 0) < 0 ||
            inputFrame_->format != AV_PIX_FMT_D3D11 ||
            !inputFrame_->data[0]) {
            return false;
        }

        UINT sourceWidth = static_cast<UINT>(source->width());
        UINT sourceHeight = static_cast<UINT>(source->height());
        RECT sourceRect{0, 0, static_cast<LONG>(sourceWidth),
                        static_cast<LONG>(sourceHeight)};
        ComPtr<ID3D11VideoProcessorInputView> activeInput;
        auto* native = D3D11DesktopFrameBuffer::From(source.get());
        if (native && native->Device() == d3dDevice_.Get()) {
            sourceWidth = static_cast<UINT>(native->SourceWidth());
            sourceHeight = static_cast<UINT>(native->SourceHeight());
            sourceRect = {
                native->CropX(), native->CropY(),
                native->CropX() + native->CropWidth(),
                native->CropY() + native->CropHeight()};
            if (!EnsureVideoProcessor(sourceWidth, sourceHeight)) {
                return false;
            }
            D3D11_VIDEO_PROCESSOR_INPUT_VIEW_DESC inputView{};
            inputView.ViewDimension = D3D11_VPIV_DIMENSION_TEXTURE2D;
            if (FAILED(videoDevice_->CreateVideoProcessorInputView(
                    native->Texture(), processorEnumerator_.Get(),
                    &inputView, &activeInput))) {
                return false;
            }
        } else {
            auto i420 = source->ToI420();
            if (!i420 || !EnsureVideoProcessor(
                    sourceWidth, sourceHeight)) {
                return false;
            }
            convertedBgra_.resize(
                static_cast<std::size_t>(sourceWidth) *
                sourceHeight * 4);
            if (libyuv::I420ToARGB(
                    i420->DataY(), i420->StrideY(),
                    i420->DataU(), i420->StrideU(),
                    i420->DataV(), i420->StrideV(),
                    convertedBgra_.data(),
                    static_cast<int>(sourceWidth) * 4,
                    static_cast<int>(sourceWidth),
                    static_cast<int>(sourceHeight)) != 0) {
                return false;
            }
            d3dContext_->UpdateSubresource(
                uploadBgraTexture_.Get(), 0, nullptr,
                convertedBgra_.data(), sourceWidth * 4, 0);
            activeInput = uploadInputView_;
        }

        auto* outputTexture = reinterpret_cast<ID3D11Texture2D*>(
            inputFrame_->data[0]);
        const UINT outputIndex = static_cast<UINT>(
            reinterpret_cast<std::uintptr_t>(inputFrame_->data[1]));
        D3D11_VIDEO_PROCESSOR_OUTPUT_VIEW_DESC outputViewDescription{};
        D3D11_TEXTURE2D_DESC outputDescription{};
        outputTexture->GetDesc(&outputDescription);
        if (outputDescription.ArraySize > 1) {
            outputViewDescription.ViewDimension =
                D3D11_VPOV_DIMENSION_TEXTURE2DARRAY;
            outputViewDescription.Texture2DArray.FirstArraySlice =
                outputIndex;
            outputViewDescription.Texture2DArray.ArraySize = 1;
        } else {
            outputViewDescription.ViewDimension =
                D3D11_VPOV_DIMENSION_TEXTURE2D;
        }
        ComPtr<ID3D11VideoProcessorOutputView> outputView;
        if (FAILED(videoDevice_->CreateVideoProcessorOutputView(
                outputTexture, processorEnumerator_.Get(),
                &outputViewDescription, &outputView))) {
            return false;
        }
        const RECT targetRect{
            0, 0, static_cast<LONG>(width_), static_cast<LONG>(height_)};
        videoContext_->VideoProcessorSetStreamSourceRect(
            processor_.Get(), 0, TRUE, &sourceRect);
        videoContext_->VideoProcessorSetStreamDestRect(
            processor_.Get(), 0, TRUE, &targetRect);
        videoContext_->VideoProcessorSetOutputTargetRect(
            processor_.Get(), TRUE, &targetRect);
        D3D11_VIDEO_PROCESSOR_STREAM stream{};
        stream.Enable = TRUE;
        stream.InputFrameOrField = videoProcessorFrameIndex_++;
        stream.pInputSurface = activeInput.Get();
        return SUCCEEDED(videoContext_->VideoProcessorBlt(
            processor_.Get(), outputView.Get(), 0, 1, &stream));
    }

    bool ConfigureBackendOptions(FfmpegHardwareApi& api,
                                 FfmpegHardwareBackend backend)
    {
        api.avOptSet(codecContext_->priv_data, "profile", "baseline", 0);
        switch (backend) {
        case FfmpegHardwareBackend::kNvenc:
            // FFmpeg's NVENC wrapper has its own output queue in addition to
            // the NVIDIA encoder. Its default `delay=INT_MAX` is clipped to
            // surfaces - 1 (normally three frames), so output_ready() waits
            // for three submitted frames before returning the first packet.
            // That is only ~67 ms at a steady 30 FPS, but becomes 2-3 seconds
            // when WebRTC initially feeds the encoder at 1 FPS. `tune=ull`
            // and `zerolatency=1` disable SDK reordering; they do not disable
            // this FFmpeg-side queue. Remote desktop needs one submitted
            // frame to produce one immediately available access unit.
            for (const auto& [name, value] :
                 std::array<std::pair<const char*, const char*>, 6>{
                     {{"preset", NvencPresetForQuality(quality_)},
                      {"tune", "ull"},
                      {"rc", "cbr"},
                      {"delay", "0"},
                      {"zerolatency", "1"},
                      {"forced-idr", "1"}}}) {
                const int optionResult = api.avOptSet(
                    codecContext_->priv_data, name, value, 0);
                if (optionResult < 0) {
                    lastError_ = FfmpegError(
                        api,
                        (std::string("Configure NVENC option ") + name)
                            .c_str(),
                        optionResult);
                    return false;
                }
            }
            break;
        case FfmpegHardwareBackend::kAmf:
            api.avOptSet(
                codecContext_->priv_data, "usage", "ultralowlatency", 0);
            api.avOptSet(codecContext_->priv_data, "quality",
                         AmfQualityForPreset(quality_), 0);
            api.avOptSet(codecContext_->priv_data, "header_insertion_mode",
                         "gop", 0);
            break;
        case FfmpegHardwareBackend::kQsv:
            api.avOptSet(codecContext_->priv_data, "preset",
                         QsvPresetForQuality(quality_), 0);
            api.avOptSet(codecContext_->priv_data, "look_ahead", "0", 0);
            // Old QSV implementations commonly keep several submitted frames
            // in flight. That throughput-oriented default is especially
            // harmful when the libwebrtc capturer suppresses unchanged
            // desktop frames: the encoder may not emit the first access unit
            // until another changed frame arrives. Keep only one frame in
            // flight and make every WebRTC key-frame request a real IDR with
            // parameter sets that a newly attached decoder can consume.
            api.avOptSet(codecContext_->priv_data, "async_depth", "1", 0);
            api.avOptSet(codecContext_->priv_data, "forced_idr", "1", 0);
            api.avOptSet(codecContext_->priv_data, "repeat_pps", "1", 0);
            break;
        case FfmpegHardwareBackend::kAutomatic:
            break;
        }
        return true;
    }

    void ApplyBitrate(std::uint32_t bitrate)
    {
        bitrateBps_ = (std::min)(bitrate, maximumBitrateBps_);
        pendingBitrateBps_ = bitrateBps_;
        lastBitrateApplyTime_ = std::chrono::steady_clock::now();
        if (!codecContext_) {
            return;
        }
        codecContext_->bit_rate = bitrateBps_;
        if (selectedBackend_ == FfmpegHardwareBackend::kNvenc) {
            codecContext_->rc_max_rate = bitrateBps_;
            codecContext_->rc_buffer_size = static_cast<int>(
                (std::max<std::uint32_t>)(bitrateBps_ / 2, 100'000));
        }
        auto& api = FfmpegHardwareApi::Instance();
        if (api.avOptSetInt) {
            api.avOptSetInt(codecContext_, "b", bitrateBps_, 0);
            if (selectedBackend_ == FfmpegHardwareBackend::kNvenc) {
                api.avOptSetInt(
                    codecContext_, "maxrate", bitrateBps_, 0);
                api.avOptSetInt(
                    codecContext_, "bufsize",
                    codecContext_->rc_buffer_size, 0);
            }
        }
    }

    void ScheduleBitrate(std::uint32_t bitrate)
    {
        pendingBitrateBps_ = (std::min)(bitrate, maximumBitrateBps_);
        if (!initialized_) {
            ApplyBitrate(pendingBitrateBps_);
            return;
        }
        MaybeApplyPendingBitrate();
    }

    void MaybeApplyPendingBitrate()
    {
        if (pendingBitrateBps_ == 0 ||
            pendingBitrateBps_ == bitrateBps_ ||
            !codecContext_) {
            return;
        }
        if (selectedBackend_ != FfmpegHardwareBackend::kNvenc &&
            selectedBackend_ != FfmpegHardwareBackend::kQsv) {
            ApplyBitrate(pendingBitrateBps_);
            return;
        }
        // Do not ask a newly opened QSV session to reset its rate-control
        // parameters before it has accepted and emitted even one frame.
        // FFmpeg implements every QSV bitrate change through
        // MFXVideoENCODE_Reset; several Intel drivers reject that reset on
        // the first avcodec_send_frame with EINVAL, which makes libwebrtc
        // permanently select the software fallback after a resolution
        // change. The InitEncode allocation is already valid for the startup
        // key frame, so retain it until the hardware path is demonstrably
        // active.
        if (selectedBackend_ == FfmpegHardwareBackend::kQsv &&
            outputFrames_ == 0) {
            return;
        }

        // GoogCC may publish a slightly different allocation on almost every
        // frame. FFmpeg's NVENC wrapper forces an IDR for a bitrate change,
        // while its QSV wrapper performs a full MFXVideoENCODE_Reset. Coalesce
        // both backends' updates while retaining a quicker path for a material
        // bandwidth reduction.
        const auto now = std::chrono::steady_clock::now();
        const auto sinceLastApply =
            std::chrono::duration_cast<std::chrono::milliseconds>(
                now - lastBitrateApplyTime_);
        const std::uint64_t current = bitrateBps_;
        const std::uint64_t pending = pendingBitrateBps_;
        const std::uint64_t difference = current > pending
            ? current - pending
            : pending - current;
        const bool materialChange = difference * 8 >= current;
        const bool urgentReduction =
            pending * 4 <= current * 3 &&
            sinceLastApply >= std::chrono::milliseconds(500);
        const bool periodicUpdate =
            materialChange &&
            sinceLastApply >= std::chrono::milliseconds(2000);
        const bool finalCatchUp =
            sinceLastApply >= std::chrono::milliseconds(5000);
        if (urgentReduction || periodicUpdate || finalCatchUp) {
            ApplyBitrate(pendingBitrateBps_);
        }
    }

    void ReportInputFormat(
        const webrtc::scoped_refptr<webrtc::VideoFrameBuffer>& buffer)
    {
        if (!runtimeState_ || runtimeInstanceId_ == 0 || !buffer) {
            return;
        }
        std::string format;
        const std::string dimensions =
            std::to_string(buffer->width()) + "x" +
            std::to_string(buffer->height()) + " -> " +
            std::to_string(width_) + "x" +
            std::to_string(height_);
        if (useD3D11Input_ &&
            D3D11DesktopFrameBuffer::From(buffer.get())) {
            format = "D3D11 BGRA " + dimensions +
                " -> D3D11 NV12 hardware frame";
        } else if (useD3D11Input_) {
            format = "CPU frame " + dimensions +
                " -> D3D11 BGRA -> D3D11 NV12 hardware frame";
        } else if (buffer->type() ==
                   webrtc::VideoFrameBuffer::Type::kNative) {
            format = buffer->storage_representation() + " " + dimensions +
                " -> CPU NV12 upload";
        } else {
            format = "CPU I420 " + dimensions + " -> CPU NV12 upload";
        }
        if (format != lastInputFormat_) {
            lastInputFormat_ = format;
            runtimeState_->MarkInputFormat(runtimeInstanceId_, format);
        }
    }

    int RequestSoftwareFallback(std::string reason)
    {
        if (reason.empty()) {
            reason = "Unknown FFmpeg hardware encoder failure.";
        }
        if (width_ > 0 && height_ > 0) {
            reason += " [target=" + std::to_string(width_) + "x" +
                std::to_string(height_) + "@" +
                std::to_string(configuredFrameRate_) + "fps, allocated=" +
                std::to_string(frameRate_) + "fps, start=" +
                std::to_string(requestedBitrateBps_) + "bps, max=" +
                std::to_string(maximumBitrateBps_) + "bps, backend=" +
                implementationName_ + "]";
        }
        lastError_ = reason;
        if (runtimeState_ && runtimeInstanceId_ != 0) {
            runtimeState_->MarkSoftwareFallback(runtimeInstanceId_, reason);
        }
        return WEBRTC_VIDEO_CODEC_FALLBACK_SOFTWARE;
    }

    bool DrainOutput()
    {
        auto& api = FfmpegHardwareApi::Instance();
        while (true) {
            api.avPacketUnref(outputPacket_);
            const int result =
                api.avcodecReceivePacket(codecContext_, outputPacket_);
            if (result == AVERROR(EAGAIN) || result == AVERROR_EOF) {
                return true;
            }
            if (result < 0) {
                lastError_ = FfmpegError(
                    api, "Receive packet from FFmpeg hardware encoder",
                    result);
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
                lastError_ =
                    "FFmpeg hardware encoder returned output without input "
                    "metadata.";
                return false;
            }
            if (!AnnexBStartCode(outputPacket_->data, outputPacket_->size)) {
                lastError_ =
                    "FFmpeg hardware encoder did not return Annex-B H264.";
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
            codecInfo.codecSpecific.H264.temporal_idx = webrtc::kNoTemporalIdx;
            codecInfo.codecSpecific.H264.idr_frame = keyFrame;
            codecInfo.codecSpecific.H264.base_layer_sync = false;
            callback_->OnEncodedImage(image, &codecInfo);
            ++outputFrames_;
            pendingFrames_.erase(metadata);
        }
    }

    void ReleaseUnlocked()
    {
        auto& api = FfmpegHardwareApi::Instance();
        if (outputPacket_ && api.avPacketFree) {
            api.avPacketFree(&outputPacket_);
        }
        if (inputFrame_ && api.avFrameFree) {
            api.avFrameFree(&inputFrame_);
        }
        if (codecContext_ && api.avcodecFreeContext) {
            api.avcodecFreeContext(&codecContext_);
        }
        if (hardwareFrames_ && api.avBufferUnref) {
            api.avBufferUnref(&hardwareFrames_);
        }
        if (hardwareDevice_ && api.avBufferUnref) {
            api.avBufferUnref(&hardwareDevice_);
        }
        uploadInputView_.Reset();
        processor_.Reset();
        processorEnumerator_.Reset();
        uploadBgraTexture_.Reset();
        videoContext_.Reset();
        videoDevice_.Reset();
        d3dContext_.Reset();
        d3dDevice_.Reset();
        convertedBgra_.clear();
        processorSourceWidth_ = 0;
        processorSourceHeight_ = 0;
        videoProcessorFrameIndex_ = 0;
        useD3D11Input_ = false;
        if (runtimeState_ && runtimeInstanceId_ != 0) {
            runtimeState_->MarkHardwareStopped(runtimeInstanceId_);
            runtimeState_->UnregisterEncoder(runtimeInstanceId_);
            runtimeInstanceId_ = 0;
        }
        pendingFrames_.clear();
        initialized_ = false;
        startupKeyFramePending_ = false;
        requestedBitrateBps_ = 0;
        pendingBitrateBps_ = 0;
        lastBitrateApplyTime_ = {};
        width_ = 0;
        height_ = 0;
        configuredFrameRate_ = 30;
        frameRate_ = 30;
        outputFrames_ = 0;
        nextPresentationId_ = 1;
        implementationName_ = "FFmpeg/Hardware";
        selectedBackend_ = FfmpegHardwareBackend::kAutomatic;
    }

    mutable std::mutex mutex_;
    AVCodecContext* codecContext_ = nullptr;
    AVFrame* inputFrame_ = nullptr;
    AVPacket* outputPacket_ = nullptr;
    AVBufferRef* hardwareDevice_ = nullptr;
    AVBufferRef* hardwareFrames_ = nullptr;
    webrtc::EncodedImageCallback* callback_ = nullptr;
    std::deque<FrameMetadata> pendingFrames_;
    FfmpegHardwareBackend requestedBackend_;
    FfmpegHardwareBackend selectedBackend_ =
        FfmpegHardwareBackend::kAutomatic;
    FfmpegX264Preset quality_ = FfmpegX264Preset::kMedium;
    std::shared_ptr<VideoEncoderRuntimeState> runtimeState_;
    std::uint64_t runtimeInstanceId_ = 0;
    std::string implementationName_ = "FFmpeg/Hardware";
    std::string lastInputFormat_;
    std::string lastError_;
    ComPtr<ID3D11Device> d3dDevice_;
    ComPtr<ID3D11DeviceContext> d3dContext_;
    ComPtr<ID3D11VideoDevice> videoDevice_;
    ComPtr<ID3D11VideoContext> videoContext_;
    ComPtr<ID3D11Texture2D> uploadBgraTexture_;
    ComPtr<ID3D11VideoProcessorEnumerator> processorEnumerator_;
    ComPtr<ID3D11VideoProcessor> processor_;
    ComPtr<ID3D11VideoProcessorInputView> uploadInputView_;
    std::vector<std::uint8_t> convertedBgra_;
    UINT processorSourceWidth_ = 0;
    UINT processorSourceHeight_ = 0;
    UINT videoProcessorFrameIndex_ = 0;
    std::uint32_t width_ = 0;
    std::uint32_t height_ = 0;
    std::uint32_t configuredFrameRate_ = 30;
    std::uint32_t frameRate_ = 30;
    std::uint32_t bitrateBps_ = 1'000'000;
    std::uint32_t requestedBitrateBps_ = 0;
    std::uint32_t pendingBitrateBps_ = 0;
    std::uint32_t maximumBitrateBps_ = 1'000'000;
    std::int64_t nextPresentationId_ = 1;
    std::uint64_t outputFrames_ = 0;
    std::chrono::steady_clock::time_point lastBitrateApplyTime_{};
    bool startupKeyFramePending_ = false;
    bool useD3D11Input_ = false;
    bool initialized_ = false;
};

FfmpegHardwareH264Encoder::FfmpegHardwareH264Encoder(
    FfmpegHardwareBackend backend,
    std::shared_ptr<VideoEncoderRuntimeState> runtimeState,
    FfmpegX264Preset quality)
    : impl_(std::make_unique<Impl>(
          backend, std::move(runtimeState), quality))
{}

FfmpegHardwareH264Encoder::~FfmpegHardwareH264Encoder() = default;

std::vector<FfmpegHardwareEncoderAvailability>
FfmpegHardwareH264Encoder::EnumerateAvailability()
{
    std::vector<FfmpegHardwareEncoderAvailability> result;
    auto& api = FfmpegHardwareApi::Instance();
    if (!api.EnsureLoaded()) {
        result.push_back({
            .detail = api.Error(),
        });
        return result;
    }
    for (const auto& descriptor : kBackendDescriptors) {
        const bool compiled =
            api.avcodecFindEncoderByName(descriptor.codecName) != nullptr;
        const bool driver =
            HasVendor(descriptor.vendorId) &&
            SystemModuleAvailable(descriptor.driverModule);
        result.push_back({
            .backend = descriptor.backend,
            .compiled = compiled,
            .driverRuntimePresent = driver,
            .implementation = descriptor.implementation,
            .detail = !compiled
                ? "not compiled into the bundled FFmpeg runtime"
                : !driver
                    ? "matching GPU or vendor driver runtime is unavailable"
                    : "candidate available; real initialization is required",
        });
    }
    return result;
}

bool FfmpegHardwareH264Encoder::RuntimeAvailable(
    FfmpegHardwareBackend backend,
    std::string* error)
{
    const auto availability = EnumerateAvailability();
    for (const auto& candidate : availability) {
        if ((backend == FfmpegHardwareBackend::kAutomatic ||
             candidate.backend == backend) &&
            candidate.compiled && candidate.driverRuntimePresent) {
            if (error) {
                error->clear();
            }
            return true;
        }
    }
    if (error) {
        const auto requested = std::find_if(
            availability.begin(), availability.end(),
            [backend](const auto& candidate) {
                return backend == FfmpegHardwareBackend::kAutomatic ||
                       candidate.backend == backend;
            });
        *error = requested == availability.end()
            ? "No FFmpeg hardware backend was detected."
            : std::string(requested->implementation.empty()
                              ? "FFmpeg hardware encoder"
                              : requested->implementation) +
                  ": " + requested->detail + ".";
    }
    return false;
}

int FfmpegHardwareH264Encoder::InitEncode(
    const webrtc::VideoCodec* codecSettings,
    const Settings& /* settings */)
{
    return impl_->Init(codecSettings);
}

int32_t FfmpegHardwareH264Encoder::RegisterEncodeCompleteCallback(
    webrtc::EncodedImageCallback* callback)
{
    return impl_->RegisterCallback(callback);
}

int32_t FfmpegHardwareH264Encoder::Release()
{
    return impl_->Release();
}

int32_t FfmpegHardwareH264Encoder::Encode(
    const webrtc::VideoFrame& frame,
    const std::vector<webrtc::VideoFrameType>* frameTypes)
{
    return impl_->Encode(frame, frameTypes);
}

void FfmpegHardwareH264Encoder::SetRates(
    const RateControlParameters& parameters)
{
    impl_->SetRates(parameters);
}

webrtc::VideoEncoder::EncoderInfo
FfmpegHardwareH264Encoder::GetEncoderInfo() const
{
    EncoderInfo info;
    info.supports_native_handle = true;
    info.implementation_name = impl_->ImplementationName();
    info.has_trusted_rate_controller = false;
    info.is_hardware_accelerated = true;
    info.enable_cpu_overuse_detection = true;
    info.supports_simulcast = false;
    info.requested_resolution_alignment = 2;
    info.apply_alignment_to_all_simulcast_layers = true;
    info.preferred_pixel_formats.push_back(
        webrtc::VideoFrameBuffer::Type::kNV12);
    info.preferred_pixel_formats.push_back(
        webrtc::VideoFrameBuffer::Type::kI420);
    info.scaling_settings = ScalingSettings::kOff;
    return info;
}

std::string FfmpegHardwareH264Encoder::LastError() const
{
    return impl_->LastError();
}

FfmpegHardwareEncoderSelfTestResult RunFfmpegHardwareEncoderSelfTest(
    FfmpegHardwareBackend backend)
{
    class Callback final : public webrtc::EncodedImageCallback {
    public:
        Result OnEncodedImage(
            const webrtc::EncodedImage& image,
            const webrtc::CodecSpecificInfo* codecInfo) override
        {
            if (!codecInfo || codecInfo->codecType != webrtc::kVideoCodecH264 ||
                !AnnexBStartCode(image.data(), image.size())) {
                invalid = true;
                return Result(Result::ERROR_SEND_FAILED);
            }
            ++frames;
            bytes += image.size();
            if (image._frameType ==
                webrtc::VideoFrameType::kVideoFrameKey) {
                ++keyFrames;
            }
            return Result(Result::OK);
        }
        void OnFrameDropped(
            uint32_t /* rtpTimestamp */,
            int /* spatialId */,
            bool /* isEndOfTemporalUnit */) override
        {
            invalid = true;
        }
        std::uint32_t frames = 0;
        std::uint32_t keyFrames = 0;
        std::size_t bytes = 0;
        bool invalid = false;
    } callback;

    FfmpegHardwareEncoderSelfTestResult test;
    test.backend = backend;
    std::string availabilityError;
    if (!FfmpegHardwareH264Encoder::RuntimeAvailable(
            backend, &availabilityError)) {
        test.error = availabilityError;
        return test;
    }
    FfmpegHardwareH264Encoder encoder(backend);
    webrtc::VideoCodec codec{};
    codec.codecType = webrtc::kVideoCodecH264;
    codec.width = 1280;
    codec.height = 720;
    codec.maxFramerate = 30;
    codec.minBitrate = 300;
    codec.startBitrate = 2'500;
    codec.maxBitrate = 8'000;
    codec.mode = webrtc::VideoCodecMode::kScreensharing;
    if (encoder.RegisterEncodeCompleteCallback(&callback) !=
            WEBRTC_VIDEO_CODEC_OK ||
        encoder.InitEncode(&codec, webrtc::VideoEncoder::Settings(
            webrtc::VideoEncoder::Capabilities(false), 4, 1200)) !=
            WEBRTC_VIDEO_CODEC_OK) {
        test.error = "FFmpeg hardware encoder initialization failed.";
        return test;
    }
    webrtc::VideoBitrateAllocation initialAllocation;
    initialAllocation.SetBitrate(0, 0, 2'500'000);
    encoder.SetRates(webrtc::VideoEncoder::RateControlParameters(
        initialAllocation, 30.0));
    auto buffer = webrtc::I420Buffer::Create(codec.width, codec.height);
    // Some drivers retain a very small fixed pipeline even with low-latency
    // options enabled. Feed enough frames to validate steady-state output
    // instead of incorrectly treating the first two delayed packets as a
    // failed encoder.
    for (int index = 0; index < 10; ++index) {
        std::memset(buffer->MutableDataY(), 32 + index * 32,
                    buffer->StrideY() * buffer->height());
        std::memset(buffer->MutableDataU(), 96 + index * 8,
                    buffer->StrideU() * ((buffer->height() + 1) / 2));
        std::memset(buffer->MutableDataV(), 160 - index * 8,
                    buffer->StrideV() * ((buffer->height() + 1) / 2));
        const auto frame = webrtc::VideoFrame::Builder()
            .set_video_frame_buffer(buffer)
            .set_timestamp_us(1'000'000 + index * 33'333)
            .set_rtp_timestamp(90'000 + index * 3'000)
            .build();
        const std::vector<webrtc::VideoFrameType> frameTypes{
            index == 0 ? webrtc::VideoFrameType::kVideoFrameKey
                       : webrtc::VideoFrameType::kVideoFrameDelta};
        if (encoder.Encode(frame, &frameTypes) != WEBRTC_VIDEO_CODEC_OK) {
            test.error = "FFmpeg hardware encoder rejected a self-test frame.";
            encoder.Release();
            return test;
        }
    }
    test.implementation = encoder.GetEncoderInfo().implementation_name;
    encoder.Release();
    test.frames = callback.frames;
    test.keyFrames = callback.keyFrames;
    test.encodedBytes = callback.bytes;
    test.succeeded = !callback.invalid && callback.frames >= 3 &&
                     callback.keyFrames >= 1 && callback.bytes != 0;
    if (!test.succeeded) {
        test.error = "FFmpeg hardware encoder did not produce valid Annex-B "
                     "H264 output.";
    }
    return test;
}

}  // namespace remote
