// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#include "WebRtcRuntime.h"

#include <algorithm>
#include <array>
#include <cctype>
#include <sstream>
#include <utility>

#include <mmdeviceapi.h>
#include <wrl/client.h>

#include "api/audio/create_audio_device_module.h"
#include "api/audio/audio_device_defines.h"
#include "api/audio_codecs/builtin_audio_decoder_factory.h"
#include "api/audio_codecs/builtin_audio_encoder_factory.h"
#include "api/create_peerconnection_factory.h"
#include "api/environment/environment_factory.h"
#include "api/video/render_resolution.h"
#include "api/video/video_codec_type.h"
#include "api/video_codecs/builtin_video_decoder_factory.h"
#include "api/video_codecs/builtin_video_encoder_factory.h"
#include "api/video_codecs/sdp_video_format.h"
#include "api/video_codecs/video_decoder_factory.h"
#include "api/video_codecs/video_encoder_factory.h"
#include "rtc_base/thread.h"
#include "rtc_base/ssl_adapter.h"
#include "rtc_base/win32_socket_init.h"
#include "src/platform/win/MfH264EncoderCapabilityProbe.h"
#include "src/platform/win/MfD3D11H264EncoderFactory.h"
#include "src/platform/win/MfD3D11H264Decoder.h"
#include "src/platform/win/FfmpegD3D11H264Decoder.h"
#include "src/platform/win/MfD3D11H264DecoderFactory.h"
#include "WindowsPreferredVideoDecoderFactory.h"
#include "WindowsPreferredVideoEncoderFactory.h"
#include "src/platform/win/FfmpegX264H264Encoder.h"
#include "src/platform/win/FfmpegX264H264EncoderFactory.h"
#include "src/platform/win/FfmpegHardwareH264Encoder.h"
#include "src/platform/win/FfmpegHardwareH264EncoderFactory.h"
#include "src/platform/win/H264EncoderBenchmark.h"
#include "src/platform/win/QualityOpenH264Encoder.h"

namespace remote {
namespace {

std::vector<std::string> DescribeFormats(
    const std::vector<webrtc::SdpVideoFormat>& formats)
{
    std::vector<std::string> result;
    result.reserve(formats.size());
    for (const auto& format : formats) {
        result.push_back(format.ToString());
    }
    return result;
}

bool ContainsCodec(const std::vector<webrtc::SdpVideoFormat>& formats,
                   const std::string& codecName)
{
    return std::any_of(formats.begin(), formats.end(), [&](const auto& format) {
        if (format.name.size() != codecName.size()) {
            return false;
        }

        for (size_t index = 0; index < format.name.size(); ++index) {
            const auto left = static_cast<unsigned char>(format.name[index]);
            const auto right = static_cast<unsigned char>(codecName[index]);
            if (std::tolower(left) != std::tolower(right)) {
                return false;
            }
        }
        return true;
    });
}

bool IsSystemDefaultDeviceId(const std::string& deviceId)
{
    return deviceId.empty() ||
           deviceId == kSystemDefaultMediaDeviceId;
}

std::string WideToUtf8(const wchar_t* value)
{
    if (!value || *value == L'\0') {
        return {};
    }
    const int length = WideCharToMultiByte(
        CP_UTF8, 0, value, -1, nullptr, 0, nullptr, nullptr);
    if (length <= 1) {
        return {};
    }
    std::string result(static_cast<std::size_t>(length), '\0');
    (void)WideCharToMultiByte(
        CP_UTF8, 0, value, -1, result.data(), length,
        nullptr, nullptr);
    result.resize(static_cast<std::size_t>(length - 1));
    return result;
}

std::string DefaultWindowsAudioEndpointId(EDataFlow flow)
{
    const HRESULT initializeResult =
        CoInitializeEx(nullptr, COINIT_MULTITHREADED);
    const bool uninitialize =
        initializeResult == S_OK || initializeResult == S_FALSE;
    if (FAILED(initializeResult) &&
        initializeResult != RPC_E_CHANGED_MODE) {
        return {};
    }

    Microsoft::WRL::ComPtr<IMMDeviceEnumerator> enumerator;
    const HRESULT createResult = CoCreateInstance(
        __uuidof(MMDeviceEnumerator), nullptr, CLSCTX_INPROC_SERVER,
        IID_PPV_ARGS(&enumerator));
    Microsoft::WRL::ComPtr<IMMDevice> endpoint;
    LPWSTR endpointId = nullptr;
    std::string result;
    if (SUCCEEDED(createResult) &&
        SUCCEEDED(enumerator->GetDefaultAudioEndpoint(
            flow, eConsole, &endpoint)) &&
        SUCCEEDED(endpoint->GetId(&endpointId))) {
        result = WideToUtf8(endpointId);
    }
    if (endpointId) {
        CoTaskMemFree(endpointId);
    }
    if (uninitialize) {
        CoUninitialize();
    }
    return result;
}

std::string ResolveActiveDeviceName(
    const std::vector<MediaDeviceDescriptor>& devices,
    const std::string& activeDeviceId,
    bool recording)
{
    const std::string physicalDeviceId =
        IsSystemDefaultDeviceId(activeDeviceId)
            ? DefaultWindowsAudioEndpointId(
                  recording ? eCapture : eRender)
            : activeDeviceId;
    const auto found = std::find_if(
        devices.begin(), devices.end(),
        [&physicalDeviceId](const MediaDeviceDescriptor& device) {
            return device.id == physicalDeviceId;
        });
    return found == devices.end() ? std::string{} : found->name;
}

std::vector<MediaDeviceDescriptor> EnumerateAudioDevices(
    webrtc::AudioDeviceModule* module,
    bool recording)
{
    std::vector<MediaDeviceDescriptor> devices;
    if (!module || !module->Initialized()) {
        return devices;
    }
    const int16_t count = recording
        ? module->RecordingDevices()
        : module->PlayoutDevices();
    if (count <= 0) {
        return devices;
    }
    devices.reserve(static_cast<std::size_t>(count));
    for (int16_t index = 0; index < count; ++index) {
        std::array<char, webrtc::kAdmMaxDeviceNameSize> name{};
        std::array<char, webrtc::kAdmMaxGuidSize> guid{};
        const int32_t result = recording
            ? module->RecordingDeviceName(
                  static_cast<uint16_t>(index),
                  name.data(), guid.data())
            : module->PlayoutDeviceName(
                  static_cast<uint16_t>(index),
                  name.data(), guid.data());
        if (result != 0 || guid.front() == '\0') {
            continue;
        }
        devices.push_back(
            {guid.data(),
             name.front() == '\0' ? guid.data() : name.data(),
             true});
    }
    return devices;
}

bool SelectAudioDevice(webrtc::AudioDeviceModule* module,
                       bool recording,
                       const std::string& deviceId)
{
    if (!module || !module->Initialized()) {
        return false;
    }
    if (IsSystemDefaultDeviceId(deviceId)) {
        return (recording
                    ? module->SetRecordingDevice(
                          webrtc::AudioDeviceModule::kDefaultDevice)
                    : module->SetPlayoutDevice(
                          webrtc::AudioDeviceModule::kDefaultDevice)) == 0;
    }
    const int16_t count = recording
        ? module->RecordingDevices()
        : module->PlayoutDevices();
    for (int16_t index = 0; index < count; ++index) {
        std::array<char, webrtc::kAdmMaxDeviceNameSize> name{};
        std::array<char, webrtc::kAdmMaxGuidSize> guid{};
        const int32_t result = recording
            ? module->RecordingDeviceName(
                  static_cast<uint16_t>(index),
                  name.data(), guid.data())
            : module->PlayoutDeviceName(
                  static_cast<uint16_t>(index),
                  name.data(), guid.data());
        if (result == 0 && deviceId == guid.data()) {
            return (recording
                        ? module->SetRecordingDevice(
                              static_cast<uint16_t>(index))
                        : module->SetPlayoutDevice(
                              static_cast<uint16_t>(index))) == 0;
        }
    }
    return false;
}

std::string DescribeHardwareEncoder(
    const MfH264EncoderCapability& capability)
{
    std::ostringstream stream;
    stream << (capability.name.empty() ? "未命名编码器" : capability.name)
           << " | CLSID："
           << (capability.clsid.empty() ? "未知" : capability.clsid)
           << " | CPU NV12："
           << (capability.cpuNv12InputSupported ? "支持" : "未确认")
           << " | D3D11 感知："
           << (capability.d3d11Aware ? "是" : "否")
           << " | D3D11 设备管理器："
           << (capability.d3d11DeviceManagerAccepted ? "接受" : "未接受")
           << " | 低延迟："
           << (capability.lowLatencyModeSupported ? "支持" : "未报告")
           << " | 动态码率："
           << (capability.dynamicBitrateSupported
                   ? (capability.dynamicBitrateModifiable ? "可修改"
                                                          : "支持")
                   : "未报告");
    if (capability.maximumWidth != 0 && capability.maximumHeight != 0) {
        stream << " | 最大模式：" << capability.maximumWidth << 'x'
               << capability.maximumHeight;
        if (capability.maximumFrameRateDenominator != 0) {
            stream << '@'
                   << capability.maximumFrameRateNumerator /
                          capability.maximumFrameRateDenominator;
        }
    }
    return stream.str();
}

struct VideoEncoderFactorySelection {
    std::unique_ptr<webrtc::VideoEncoderFactory> software;
    std::unique_ptr<webrtc::VideoEncoderFactory> hardware;
    bool softwareWired = false;
    bool ffmpegX264Wired = false;
    bool hardwareWired = false;
    bool ffmpegHardwareWired = false;
    std::string ffmpegX264Error;
    std::string error;
};

VideoEncoderFactorySelection BuildVideoEncoderFactorySelection(
    VideoEncoderPreference preference,
    FfmpegX264Preset quality,
    FfmpegHardwareBackend ffmpegHardwareBackend,
    const std::string& preferredAutomaticEncoderId,
    bool mfHardwareAvailable,
    bool ffmpegHardwareAvailable,
    const std::shared_ptr<VideoEncoderRuntimeState>& runtimeState)
{
    VideoEncoderFactorySelection selection;
    const bool automaticPrefersOpenH264 =
        preference == VideoEncoderPreference::kAutomatic &&
        preferredAutomaticEncoderId == kAutomaticEncoderOpenH264;
    const bool automaticPrefersX264 =
        preference == VideoEncoderPreference::kAutomatic &&
        preferredAutomaticEncoderId == kAutomaticEncoderFfmpegX264;
    const bool automaticPrefersFfmpegHardware =
        preference == VideoEncoderPreference::kAutomatic &&
        (preferredAutomaticEncoderId == kAutomaticEncoderFfmpegQsv ||
         preferredAutomaticEncoderId == kAutomaticEncoderFfmpegNvenc ||
         preferredAutomaticEncoderId == kAutomaticEncoderFfmpegAmf ||
         preferredAutomaticEncoderId ==
             kAutomaticEncoderFfmpegHardwareDefault);
    const bool automaticDisablesHardware =
        automaticPrefersOpenH264 || automaticPrefersX264;

    const bool explicitlyUsesOpenH264 =
        preference == VideoEncoderPreference::kSoftwareOnly ||
        automaticPrefersOpenH264;
    if (explicitlyUsesOpenH264) {
        selection.software =
            std::make_unique<QualityOpenH264EncoderFactory>(quality);
        if (!selection.software) {
            selection.error =
                "Failed to create the quality-configured OpenH264 factory.";
            return selection;
        }
        selection.softwareWired = ContainsCodec(
            selection.software->GetSupportedFormats(), "H264");
    }

    // Every hardware path uses FFmpeg/libx264 as its software fallback. An
    // automatic benchmark may still select OpenH264 as the primary encoder;
    // that is a direct selection, not a hardware-failure fallback.
    const bool usesFfmpegSoftwareFallback =
        (preference == VideoEncoderPreference::kAutomatic &&
         !automaticPrefersOpenH264) ||
        preference == VideoEncoderPreference::kHardwareOnly ||
        preference == VideoEncoderPreference::kFfmpegHardware;
    if (usesFfmpegSoftwareFallback) {
        std::string ffmpegError;
        if (FfmpegX264H264Encoder::RuntimeAvailable(&ffmpegError)) {
            selection.software =
                std::make_unique<FfmpegX264H264EncoderFactory>(quality);
            selection.ffmpegX264Wired = true;
            selection.softwareWired = true;
        } else {
            selection.ffmpegX264Error = std::move(ffmpegError);
            selection.error =
                "The hardware encoder fallback requires the bundled "
                "FFmpeg/libx264 runtime: " +
                selection.ffmpegX264Error;
            return selection;
        }
    }

    if ((preference == VideoEncoderPreference::kAutomatic ||
         preference == VideoEncoderPreference::kHardwareOnly) &&
        !automaticDisablesHardware &&
        !automaticPrefersFfmpegHardware && mfHardwareAvailable) {
        selection.hardware =
            std::make_unique<MfD3D11H264EncoderFactory>(
                runtimeState, quality);
        selection.hardwareWired =
            selection.hardware->QueryCodecSupport(
                webrtc::SdpVideoFormat::H264(), std::nullopt,
                std::nullopt).is_supported;
    }

    const bool shouldUseFfmpegHardware =
        preference == VideoEncoderPreference::kFfmpegHardware ||
        automaticPrefersFfmpegHardware ||
        ((preference == VideoEncoderPreference::kAutomatic ||
          preference == VideoEncoderPreference::kHardwareOnly) &&
         !automaticDisablesHardware && !selection.hardwareWired);
    if (shouldUseFfmpegHardware && ffmpegHardwareAvailable) {
        FfmpegHardwareBackend selectedBackend = ffmpegHardwareBackend;
        if (automaticPrefersFfmpegHardware) {
            selectedBackend =
                preferredAutomaticEncoderId == kAutomaticEncoderFfmpegQsv
                    ? FfmpegHardwareBackend::kQsv
                    : preferredAutomaticEncoderId ==
                              kAutomaticEncoderFfmpegNvenc
                    ? FfmpegHardwareBackend::kNvenc
                    : preferredAutomaticEncoderId ==
                              kAutomaticEncoderFfmpegAmf
                    ? FfmpegHardwareBackend::kAmf
                    : ffmpegHardwareBackend;
        }
        selection.hardware =
            std::make_unique<FfmpegHardwareH264EncoderFactory>(
                selectedBackend, runtimeState, quality);
        selection.ffmpegHardwareWired =
            selection.hardware->QueryCodecSupport(
                webrtc::SdpVideoFormat::H264(), std::nullopt,
                std::nullopt).is_supported;
        selection.hardwareWired = selection.ffmpegHardwareWired;
    }

    if (preference == VideoEncoderPreference::kFfmpegX264Only) {
        std::string ffmpegError;
        if (!FfmpegX264H264Encoder::RuntimeAvailable(&ffmpegError)) {
            selection.ffmpegX264Error = ffmpegError;
            selection.error =
                "FFmpeg/libx264 video encoding was selected, but its "
                "runtime is unavailable: " + ffmpegError;
            return selection;
        }
        selection.software =
            std::make_unique<FfmpegX264H264EncoderFactory>(quality);
        selection.ffmpegX264Wired =
            selection.software->QueryCodecSupport(
                webrtc::SdpVideoFormat::H264(), std::nullopt,
                std::nullopt).is_supported;
        selection.softwareWired = selection.ffmpegX264Wired;
    }

    if (!selection.softwareWired && !selection.hardwareWired) {
        selection.error =
            "The selected video encoder mode has no available H264 encoder.";
    }
    return selection;
}

}  // namespace

WebRtcRuntime::WebRtcRuntime(
    VideoEncoderPreference videoEncoderPreference,
    VideoDecoderPreference videoDecoderPreference,
    std::string preferredHardwareDecoderName,
    std::string hardwareFingerprint,
    std::optional<MfH264EncoderCapabilityCache>
        encoderCapabilityCache,
    std::string preferredRecordingDeviceId,
    std::string preferredPlayoutDeviceId,
    FfmpegX264Preset ffmpegX264Preset,
    FfmpegHardwareBackend ffmpegHardwareBackend,
    std::string preferredAutomaticEncoderId)
    : videoEncoderPreference_(videoEncoderPreference),
      ffmpegX264Preset_(ffmpegX264Preset),
      ffmpegHardwareBackend_(ffmpegHardwareBackend),
      preferredAutomaticEncoderId_(
          std::move(preferredAutomaticEncoderId)),
      videoDecoderPreference_(videoDecoderPreference),
      decoderPreferenceState_(
          std::make_shared<MfD3D11H264DecoderPreferenceState>()),
      hardwareFingerprint_(std::move(hardwareFingerprint)),
      encoderCapabilityCache_(std::move(encoderCapabilityCache)),
      preferredRecordingDeviceId_(
          preferredRecordingDeviceId.empty()
              ? kSystemDefaultMediaDeviceId
              : std::move(preferredRecordingDeviceId)),
      preferredPlayoutDeviceId_(
          preferredPlayoutDeviceId.empty()
              ? kSystemDefaultMediaDeviceId
              : std::move(preferredPlayoutDeviceId))
{
    decoderPreferenceState_->SetPreferredDecoderName(
        std::move(preferredHardwareDecoderName));
}

WebRtcRuntime::~WebRtcRuntime()
{
    Shutdown();
}

bool WebRtcRuntime::Initialize()
{
    if (factory_) {
        return true;
    }

    report_ = {};
    report_.videoEncoderPreference = videoEncoderPreference_;
    report_.videoDecoderPreference = videoDecoderPreference_;
    report_.hardwareFingerprint = hardwareFingerprint_;
    encoderRuntimeState_ =
        std::make_shared<VideoEncoderRuntimeState>(
            videoEncoderPreference_);

    const bool encoderCacheHit =
        encoderCapabilityCache_ &&
        encoderCapabilityCache_->valid &&
        !hardwareFingerprint_.empty() &&
        encoderCapabilityCache_->hardwareFingerprint ==
            hardwareFingerprint_;
    if (encoderCacheHit) {
        const auto& cache = *encoderCapabilityCache_;
        report_.h264HardwareEncoderProbeSucceeded = true;
        report_.h264HardwareEncoderProbeFromCache = true;
        report_.h264HardwareEncoderCount =
            cache.hardwareEncoderCount;
        report_.h264HardwareEncoderAvailable =
            cache.hardwareEncoderAvailable;
        report_.h264HardwareEncoderCpuNv12InputSupported =
            cache.cpuNv12InputSupported;
        report_.h264HardwareEncoderD3D11InputCandidate =
            cache.d3d11InputCandidate;
        report_.h264HardwareEncoderDescriptions =
            cache.descriptions;
        report_.h264HardwareEncoderWarnings = cache.warnings;
    } else {
        const auto encoderCapabilityReport =
            ProbeMfH264EncoderCapabilities();
        report_.h264HardwareEncoderProbeSucceeded =
            encoderCapabilityReport.enumerationSucceeded;
        report_.h264HardwareEncoderCount =
            encoderCapabilityReport.hardwareEncoderCount;
        report_.h264HardwareEncoderAvailable =
            encoderCapabilityReport.hardwareEncoderCount != 0;
        report_.h264HardwareEncoderWarnings =
            encoderCapabilityReport.warnings;
        for (const auto& encoder : encoderCapabilityReport.encoders) {
            report_.h264HardwareEncoderCpuNv12InputSupported |=
                encoder.cpuNv12InputSupported;
            report_.h264HardwareEncoderD3D11InputCandidate |=
                encoder.d3d11Aware &&
                encoder.d3d11DeviceManagerAccepted;
            report_.h264HardwareEncoderDescriptions.push_back(
                DescribeHardwareEncoder(encoder));
            for (const auto& warning : encoder.warnings) {
                report_.h264HardwareEncoderWarnings.push_back(
                    (encoder.name.empty()
                         ? "未命名编码器"
                         : encoder.name) +
                    ": " + warning);
            }
        }
    }

    const auto ffmpegHardwareAvailability =
        FfmpegHardwareH264Encoder::EnumerateAvailability();
    for (const auto& candidate : ffmpegHardwareAvailability) {
        if (candidate.implementation.empty()) {
            if (!candidate.detail.empty()) {
                report_.h264FfmpegHardwareEncoderError = candidate.detail;
            }
            continue;
        }
        std::string description = candidate.implementation + " | FFmpeg：" +
            (candidate.compiled ? "已编译" : "未编译") +
            " | 驱动：" +
            (candidate.driverRuntimePresent ? "可用" : "不可用");
        if (!candidate.detail.empty()) {
            description += " | " + candidate.detail;
        }
        report_.h264FfmpegHardwareEncoderDescriptions.push_back(
            std::move(description));
        if (candidate.compiled && candidate.driverRuntimePresent) {
            report_.h264FfmpegHardwareEncoderAvailable = true;
        }
    }
    if (!report_.h264FfmpegHardwareEncoderAvailable &&
        report_.h264FfmpegHardwareEncoderError.empty()) {
        report_.h264FfmpegHardwareEncoderError =
            "No compiled FFmpeg hardware H264 backend has a matching GPU "
            "and vendor driver runtime.";
    }
    if (videoEncoderPreference_ ==
            VideoEncoderPreference::kHardwareOnly &&
        !report_.h264HardwareEncoderAvailable &&
        !report_.h264FfmpegHardwareEncoderAvailable) {
        report_.error =
            "Hardware-only video encoding was selected, but no compatible "
            "Media Foundation or FFmpeg H264 hardware encoder is available.";
        encoderRuntimeState_.reset();
        return false;
    }

    winsockInitializer_ = std::make_unique<webrtc::WinsockInitializer>();
    if (winsockInitializer_->error() != 0) {
        report_.error = "WSAStartup failed with error " +
                        std::to_string(winsockInitializer_->error()) + ".";
        Shutdown();
        return false;
    }

    if (!webrtc::InitializeSSL()) {
        report_.error = "WebRTC InitializeSSL failed.";
        Shutdown();
        return false;
    }
    sslInitialized_ = true;

    networkThread_ = webrtc::Thread::CreateWithSocketServer();
    workerThread_ = webrtc::Thread::Create();
    signalingThread_ = webrtc::Thread::Create();
    if (!networkThread_ || !workerThread_ || !signalingThread_) {
        report_.error = "Failed to create WebRTC threads.";
        Shutdown();
        return false;
    }

    if (!networkThread_->Start()) {
        report_.error = "Failed to start the WebRTC network thread.";
        Shutdown();
        return false;
    }
    networkThreadStarted_ = true;

    if (!workerThread_->Start()) {
        report_.error = "Failed to start the WebRTC worker thread.";
        Shutdown();
        return false;
    }
    workerThreadStarted_ = true;

    if (!signalingThread_->Start()) {
        report_.error = "Failed to start the WebRTC signaling thread.";
        Shutdown();
        return false;
    }
    signalingThreadStarted_ = true;

    const auto initializeAudioDevice = [this] {
        audioDeviceModule_ = webrtc::CreateAudioDeviceModule(
            webrtc::CreateEnvironment(),
            webrtc::AudioDeviceModule::kPlatformDefaultAudio);
        if (!audioDeviceModule_) {
            return std::string(
                "The Windows WebRTC audio device module could not be created.");
        }
        if (audioDeviceModule_->Init() != 0) {
            audioDeviceModule_ = nullptr;
            return std::string(
                "The Windows WebRTC audio device module could not be initialized.");
        }

        if (!SelectAudioDevice(
                audioDeviceModule_.get(), true,
                preferredRecordingDeviceId_)) {
            activeRecordingDeviceId_.clear();
            if (!SelectAudioDevice(
                    audioDeviceModule_.get(), true,
                    kSystemDefaultMediaDeviceId)) {
                return std::string(
                    "Neither the selected nor the default microphone is available.");
            }
            activeRecordingDeviceId_ =
                kSystemDefaultMediaDeviceId;
        } else {
            activeRecordingDeviceId_ =
                IsSystemDefaultDeviceId(
                    preferredRecordingDeviceId_)
                    ? kSystemDefaultMediaDeviceId
                    : preferredRecordingDeviceId_;
        }

        if (!SelectAudioDevice(
                audioDeviceModule_.get(), false,
                preferredPlayoutDeviceId_)) {
            if (!SelectAudioDevice(
                    audioDeviceModule_.get(), false,
                    kSystemDefaultMediaDeviceId)) {
                return std::string(
                    "Neither the selected nor the default speaker is available.");
            }
            activePlayoutDeviceId_ =
                kSystemDefaultMediaDeviceId;
        } else {
            activePlayoutDeviceId_ =
                IsSystemDefaultDeviceId(
                    preferredPlayoutDeviceId_)
                    ? kSystemDefaultMediaDeviceId
                    : preferredPlayoutDeviceId_;
        }
        return std::string{};
    };
    report_.audioDeviceError = workerThread_->BlockingCall(
        initializeAudioDevice);
    report_.audioDeviceModuleCreated =
        audioDeviceModule_ != nullptr;

    auto audioEncoderFactory = webrtc::CreateBuiltinAudioEncoderFactory();
    auto audioDecoderFactory = webrtc::CreateBuiltinAudioDecoderFactory();
    if (!audioEncoderFactory || !audioDecoderFactory) {
        report_.error =
            "Failed to create one or more built-in audio codec factories.";
        Shutdown();
        return false;
    }

    // Always construct the proven software decoder. A stale hardware-only
    // preference must not prevent WebRTC from starting after a GPU/driver/RDP
    // environment change. Even hardware-only mode retains this as a one-way
    // emergency fallback after the verified hardware decoder is selected.
    std::unique_ptr<webrtc::VideoDecoderFactory>
        builtinVideoDecoderFactory =
            webrtc::CreateBuiltinVideoDecoderFactory();
    if (!builtinVideoDecoderFactory) {
        report_.error =
            "Failed to create the built-in video decoder factory.";
        Shutdown();
        return false;
    }

    auto encoderSelection = BuildVideoEncoderFactorySelection(
        videoEncoderPreference_, ffmpegX264Preset_,
        ffmpegHardwareBackend_, preferredAutomaticEncoderId_,
        report_.h264HardwareEncoderAvailable,
        report_.h264FfmpegHardwareEncoderAvailable,
        encoderRuntimeState_);
    report_.h264SoftwareEncoderWired =
        encoderSelection.softwareWired;
    report_.h264FfmpegX264EncoderWired =
        encoderSelection.ffmpegX264Wired;
    report_.h264HardwareEncoderWired =
        encoderSelection.hardwareWired;
    report_.h264FfmpegHardwareEncoderWired =
        encoderSelection.ffmpegHardwareWired;
    report_.h264FfmpegX264EncoderError =
        encoderSelection.ffmpegX264Error;
    if (!encoderSelection.error.empty()) {
        report_.error = encoderSelection.error;
        Shutdown();
        return false;
    }
    auto videoEncoderFactory =
        std::make_unique<WindowsPreferredVideoEncoderFactory>(
            std::move(encoderSelection.software),
            std::move(encoderSelection.hardware),
            encoderRuntimeState_);
    videoEncoderFactory_ = videoEncoderFactory.get();
    report_.windowsPreferredEncoderFactoryCreated = true;
    report_.h264SoftwareEncoderFallbackWired =
        (videoEncoderPreference_ ==
             VideoEncoderPreference::kAutomatic ||
         videoEncoderPreference_ ==
             VideoEncoderPreference::kFfmpegHardware) &&
        videoEncoderFactory->HasSoftwareEncoderFor(
            webrtc::SdpVideoFormat::H264()) &&
        videoEncoderFactory->HasHardwareEncoderFor(
            webrtc::SdpVideoFormat::H264());
    const auto encoderFormats =
        videoEncoderFactory->GetSupportedFormats();
    std::unique_ptr<webrtc::VideoDecoderFactory>
        hardwareVideoDecoderFactory;
    const std::string preferredHardwareDecoderName =
        decoderPreferenceState_->PreferredDecoderName();
    {
        auto candidateHardwareFactory =
            std::make_unique<MfD3D11H264DecoderFactory>(
                MfD3D11H264DecoderSelection::kHardwareOnly,
                decoderPreferenceState_);
        if (!preferredHardwareDecoderName.empty()) {
            const auto hardwareDecoderFormats =
                candidateHardwareFactory->GetSupportedFormats();
            const auto hardwareH264Format = std::find_if(
                hardwareDecoderFormats.begin(),
                hardwareDecoderFormats.end(),
                [](const auto& format) {
                    return ContainsCodec({format}, "H264");
                });
            if (hardwareH264Format != hardwareDecoderFormats.end()) {
                auto probeDecoder = candidateHardwareFactory->Create(
                    webrtc::CreateEnvironment(), *hardwareH264Format);
                webrtc::VideoDecoder::Settings settings;
                settings.set_codec_type(webrtc::kVideoCodecH264);
                settings.set_number_of_cores(1);
                settings.set_max_render_resolution(
                    webrtc::RenderResolution(1920, 1080));
                if (!probeDecoder) {
                    report_.h264MfDecoderError =
                        "The selected hardware decoder factory returned no decoder.";
                } else if (preferredHardwareDecoderName ==
                           kFfmpegD3D11H264DecoderName) {
                auto* d3d11VaProbeDecoder =
                    static_cast<FfmpegD3D11H264Decoder*>(
                        probeDecoder.get());
                report_.h264MfDecoderConfigured =
                    d3d11VaProbeDecoder->Configure(settings);
                report_.h264MfDecoderNativeOutputSupported =
                    d3d11VaProbeDecoder->SupportsNativeOutputSamples();
                report_.h264MfDecoderHardware =
                    d3d11VaProbeDecoder->IsHardwareAccelerated();
                report_.h264MfDecoderSoftware = false;
                report_.h264MfDecoderAsynchronous =
                    d3d11VaProbeDecoder->IsAsynchronous();
                report_.h264MfDecoderName =
                    kFfmpegD3D11H264DecoderName;
                report_.h264MfDecoderError =
                    d3d11VaProbeDecoder->LastError();
                d3d11VaProbeDecoder->Release();
                } else {
                auto* mfProbeDecoder =
                    static_cast<MfD3D11H264Decoder*>(
                        probeDecoder.get());
                report_.h264MfDecoderConfigured =
                    mfProbeDecoder->Configure(settings);
                report_.h264MfDecoderNativeOutputSupported =
                    mfProbeDecoder->SupportsNativeOutputSamples();
                report_.h264MfDecoderHardware =
                    mfProbeDecoder->DecoderKind() ==
                    MfD3D11H264DecoderKind::kHardware;
                report_.h264MfDecoderSoftware =
                    mfProbeDecoder->DecoderKind() ==
                    MfD3D11H264DecoderKind::kSoftware;
                report_.h264MfDecoderAsynchronous =
                    mfProbeDecoder->IsAsynchronous();
                report_.h264MfDecoderName =
                    mfProbeDecoder->DecoderName();
                report_.h264MfDecoderError =
                    mfProbeDecoder->LastError();
                mfProbeDecoder->Release();
                }
            }

            if (report_.h264MfDecoderConfigured &&
                report_.h264MfDecoderHardware) {
                report_.h264MfDecoderWired = true;
            }
        }
        hardwareVideoDecoderFactory =
            std::move(candidateHardwareFactory);
    }

    if (videoDecoderPreference_ ==
            VideoDecoderPreference::kHardwareOnly &&
        !report_.h264MfDecoderWired) {
        report_.h264MfDecoderError =
            "Hardware-only decoding is unavailable in the current "
            "hardware/driver/session environment; WebRTC started with the "
            "software H264 decoder instead. " +
            report_.h264MfDecoderError;
    }

    auto videoDecoderFactory =
        std::make_unique<WindowsPreferredVideoDecoderFactory>(
            std::move(builtinVideoDecoderFactory),
            std::move(hardwareVideoDecoderFactory),
            videoDecoderPreference_ ==
                VideoDecoderPreference::kSoftwareOnly);
    videoDecoderFactory_ = videoDecoderFactory.get();
    const auto decoderFormats = videoDecoderFactory->GetSupportedFormats();
    report_.videoEncoderFormats = DescribeFormats(encoderFormats);
    report_.videoDecoderFormats = DescribeFormats(decoderFormats);
    report_.hasH264Encoder = ContainsCodec(encoderFormats, "H264");
    report_.hasH264Decoder = ContainsCodec(decoderFormats, "H264");
    report_.windowsPreferredDecoderFactoryCreated = true;
    report_.h264FfmpegSoftwareDecoderWired =
        videoDecoderFactory->HasSoftwareDecoderFor(
            webrtc::SdpVideoFormat::H264());

    factory_ = webrtc::CreatePeerConnectionFactory(
        networkThread_.get(),
        workerThread_.get(),
        signalingThread_.get(),
        audioDeviceModule_,
        std::move(audioEncoderFactory),
        std::move(audioDecoderFactory),
        std::move(videoEncoderFactory),
        std::move(videoDecoderFactory),
        nullptr,
        nullptr,
        nullptr,
        nullptr);

    report_.factoryCreated = factory_ != nullptr;
    if (!factory_) {
        report_.error = "CreatePeerConnectionFactory failed.";
        Shutdown();
        return false;
    }

    return true;
}

void WebRtcRuntime::Shutdown()
{
    factory_ = nullptr;
    videoEncoderFactory_ = nullptr;
    videoDecoderFactory_ = nullptr;

    // Factory destruction may enqueue final cleanup work on WebRTC-owned
    // threads. An application that initializes and immediately shuts down can
    // otherwise stop those threads while the cleanup tasks are still crossing
    // between queues. Drain each queue, then revisit signaling once more for
    // work posted by the worker/network drains before stopping any thread.
    if (signalingThread_ && signalingThreadStarted_) {
        signalingThread_->BlockingCall([] {});
    }

    if (workerThread_ && workerThreadStarted_) {
        workerThread_->BlockingCall([this] {
            if (!audioDeviceModule_) {
                return;
            }
            if (audioDeviceModule_->RecordingIsInitialized()) {
                (void)audioDeviceModule_->StopRecording();
            }
            if (audioDeviceModule_->PlayoutIsInitialized()) {
                (void)audioDeviceModule_->StopPlayout();
            }
            (void)audioDeviceModule_->Terminate();
            audioDeviceModule_ = nullptr;
        });
    } else {
        audioDeviceModule_ = nullptr;
    }
    if (workerThread_ && workerThreadStarted_) {
        workerThread_->BlockingCall([] {});
    }
    if (networkThread_ && networkThreadStarted_) {
        networkThread_->BlockingCall([] {});
    }
    if (signalingThread_ && signalingThreadStarted_) {
        signalingThread_->BlockingCall([] {});
    }

    if (signalingThread_ && signalingThreadStarted_) {
        signalingThread_->Stop();
    }
    signalingThreadStarted_ = false;

    if (workerThread_ && workerThreadStarted_) {
        workerThread_->Stop();
    }
    workerThreadStarted_ = false;

    if (networkThread_ && networkThreadStarted_) {
        networkThread_->Stop();
    }
    networkThreadStarted_ = false;

    signalingThread_.reset();
    workerThread_.reset();
    networkThread_.reset();

    if (sslInitialized_) {
        webrtc::CleanupSSL();
    }
    sslInitialized_ = false;
    winsockInitializer_.reset();
    encoderRuntimeState_.reset();
    activeRecordingDeviceId_.clear();
    activePlayoutDeviceId_.clear();
}

const WebRtcCapabilityReport& WebRtcRuntime::CapabilityReport() const noexcept
{
    return report_;
}

VideoEncoderRuntimeSnapshot WebRtcRuntime::EncoderRuntimeStatus() const
{
    if (encoderRuntimeState_) {
        return encoderRuntimeState_->Snapshot();
    }
    VideoEncoderRuntimeSnapshot snapshot;
    snapshot.preference = videoEncoderPreference_;
    return snapshot;
}

void WebRtcRuntime::SetDesktopShareGeneration(std::uint64_t generation)
{
    if (encoderRuntimeState_) {
        encoderRuntimeState_->SetDesktopShareGeneration(generation);
    }
}

void WebRtcRuntime::SetDesktopCaptureAdapterLuid(std::uint64_t adapterLuid)
{
    if (encoderRuntimeState_) {
        encoderRuntimeState_->SetDesktopCaptureAdapterLuid(adapterLuid);
    }
}

void WebRtcRuntime::SetPreferredHardwareDecoderName(std::string name)
{
    if (decoderPreferenceState_) {
        decoderPreferenceState_->SetPreferredDecoderName(std::move(name));
    }
    if (videoDecoderFactory_) {
        videoDecoderFactory_->NotifyConfigurationChanged();
    }
}

bool WebRtcRuntime::ApplyVideoCodecPreferences(
    VideoEncoderPreference videoEncoderPreference,
    FfmpegX264Preset quality,
    FfmpegHardwareBackend ffmpegHardwareBackend,
    std::string preferredAutomaticEncoderId,
    VideoDecoderPreference videoDecoderPreference,
    std::string* error)
{
    if (!factory_ || !videoEncoderFactory_ || !videoDecoderFactory_) {
        if (error) {
            *error = "The WebRTC codec factories are not initialized.";
        }
        return false;
    }

    auto selection = BuildVideoEncoderFactorySelection(
        videoEncoderPreference, quality, ffmpegHardwareBackend,
        preferredAutomaticEncoderId,
        report_.h264HardwareEncoderAvailable,
        report_.h264FfmpegHardwareEncoderAvailable,
        encoderRuntimeState_);
    if (!selection.error.empty()) {
        if (error) {
            *error = selection.error;
        }
        return false;
    }

    videoEncoderFactory_->ReplaceFactories(
        std::move(selection.software),
        std::move(selection.hardware));
    videoDecoderFactory_->SetPreferSoftware(
        videoDecoderPreference == VideoDecoderPreference::kSoftwareOnly);

    videoEncoderPreference_ = videoEncoderPreference;
    ffmpegX264Preset_ = quality;
    ffmpegHardwareBackend_ = ffmpegHardwareBackend;
    preferredAutomaticEncoderId_ =
        std::move(preferredAutomaticEncoderId);
    videoDecoderPreference_ = videoDecoderPreference;
    if (encoderRuntimeState_) {
        encoderRuntimeState_->SetPreference(videoEncoderPreference_);
    }

    report_.videoEncoderPreference = videoEncoderPreference_;
    report_.videoDecoderPreference = videoDecoderPreference_;
    report_.h264SoftwareEncoderWired = selection.softwareWired;
    report_.h264FfmpegX264EncoderWired = selection.ffmpegX264Wired;
    report_.h264HardwareEncoderWired = selection.hardwareWired;
    report_.h264FfmpegHardwareEncoderWired =
        selection.ffmpegHardwareWired;
    report_.h264FfmpegX264EncoderError =
        std::move(selection.ffmpegX264Error);
    report_.videoEncoderFormats = DescribeFormats(
        videoEncoderFactory_->GetSupportedFormats());
    report_.videoDecoderFormats = DescribeFormats(
        videoDecoderFactory_->GetSupportedFormats());
    report_.hasH264Encoder = ContainsCodec(
        videoEncoderFactory_->GetSupportedFormats(), "H264");
    report_.hasH264Decoder = ContainsCodec(
        videoDecoderFactory_->GetSupportedFormats(), "H264");
    report_.h264SoftwareEncoderFallbackWired =
        (videoEncoderPreference_ == VideoEncoderPreference::kAutomatic ||
         videoEncoderPreference_ ==
             VideoEncoderPreference::kFfmpegHardware) &&
        videoEncoderFactory_->HasSoftwareEncoderFor(
            webrtc::SdpVideoFormat::H264()) &&
        videoEncoderFactory_->HasHardwareEncoderFor(
            webrtc::SdpVideoFormat::H264());
    if (error) {
        error->clear();
    }
    return true;
}

AudioDeviceEnumeration WebRtcRuntime::EnumerateAudioDevices() const
{
    AudioDeviceEnumeration enumeration;
    if (!workerThread_ || !workerThreadStarted_ ||
        !audioDeviceModule_) {
        enumeration.error = report_.audioDeviceError.empty()
            ? "The Windows audio device module is unavailable."
            : report_.audioDeviceError;
        return enumeration;
    }

    const auto enumerate = [this] {
        AudioDeviceEnumeration result;
        if (!audioDeviceModule_->Initialized() &&
            audioDeviceModule_->Init() != 0) {
            result.error =
                "The Windows audio device module could not be reinitialized.";
            return result;
        }
        result.microphones = remote::EnumerateAudioDevices(
            audioDeviceModule_.get(), true);
        result.speakers = remote::EnumerateAudioDevices(
            audioDeviceModule_.get(), false);
        result.activeMicrophoneId = activeRecordingDeviceId_;
        result.activeSpeakerId = activePlayoutDeviceId_;
        result.activeMicrophoneName = ResolveActiveDeviceName(
            result.microphones, result.activeMicrophoneId, true);
        result.activeSpeakerName = ResolveActiveDeviceName(
            result.speakers, result.activeSpeakerId, false);
        if (result.microphones.empty() &&
            result.speakers.empty()) {
            result.error =
                "Windows did not report any microphone or speaker devices.";
        }
        return result;
    };
    return workerThread_->IsCurrent()
        ? enumerate()
        : workerThread_->BlockingCall(enumerate);
}

AudioDeviceOperationResult WebRtcRuntime::SelectRecordingDevice(
    const std::string& deviceId)
{
    const std::string normalizedId =
        IsSystemDefaultDeviceId(deviceId)
            ? kSystemDefaultMediaDeviceId
            : deviceId;
    if (!workerThread_ || !workerThreadStarted_ ||
        !audioDeviceModule_) {
        return {
            false, activeRecordingDeviceId_,
            "audio_device_module_unavailable",
            "The Windows audio device module is unavailable."};
    }

    const auto select = [this, normalizedId] {
        AudioDeviceOperationResult result;
        result.activeDeviceId = activeRecordingDeviceId_;
        const bool moduleWasInitialized =
            audioDeviceModule_->Initialized();
        if (!moduleWasInitialized &&
            audioDeviceModule_->Init() != 0) {
            result.errorCode =
                "audio_device_module_initialize_failed";
            result.errorMessage =
                "The Windows audio device module could not be initialized.";
            return result;
        }
        if (moduleWasInitialized &&
            normalizedId != kSystemDefaultMediaDeviceId &&
            normalizedId == activeRecordingDeviceId_) {
            result.succeeded = true;
            return result;
        }

        const bool wasInitialized =
            audioDeviceModule_->RecordingIsInitialized();
        const bool wasRecording =
            audioDeviceModule_->Recording();
        const std::string previousId =
            activeRecordingDeviceId_.empty()
                ? std::string(kSystemDefaultMediaDeviceId)
                : activeRecordingDeviceId_;
        if (wasInitialized &&
            audioDeviceModule_->StopRecording() != 0) {
            result.errorCode = "microphone_stop_failed";
            result.errorMessage =
                "The current microphone could not be stopped.";
            return result;
        }

        const auto restorePrevious = [&] {
            (void)audioDeviceModule_->StopRecording();
            const bool selected = SelectAudioDevice(
                audioDeviceModule_.get(), true, previousId);
            bool restored = selected;
            if (restored && wasInitialized) {
                restored =
                    audioDeviceModule_->InitRecording() == 0;
            }
            if (restored && wasRecording) {
                restored =
                    audioDeviceModule_->StartRecording() == 0;
            }
            activeRecordingDeviceId_ =
                restored ? previousId : std::string{};
            return restored;
        };

        if (!SelectAudioDevice(
                audioDeviceModule_.get(), true, normalizedId)) {
            (void)restorePrevious();
            result.activeDeviceId = activeRecordingDeviceId_;
            result.errorCode = "microphone_device_unavailable";
            result.errorMessage =
                "The selected microphone is no longer available.";
            return result;
        }
        if (wasInitialized &&
            audioDeviceModule_->InitRecording() != 0) {
            (void)restorePrevious();
            result.activeDeviceId = activeRecordingDeviceId_;
            result.errorCode = "microphone_initialize_failed";
            result.errorMessage =
                "The selected microphone could not be initialized.";
            return result;
        }
        if (wasRecording &&
            audioDeviceModule_->StartRecording() != 0) {
            (void)restorePrevious();
            result.activeDeviceId = activeRecordingDeviceId_;
            result.errorCode = "microphone_restart_failed";
            result.errorMessage =
                "The selected microphone could not start recording.";
            return result;
        }

        preferredRecordingDeviceId_ = normalizedId;
        activeRecordingDeviceId_ = normalizedId;
        result.succeeded = true;
        result.activeDeviceId = normalizedId;
        return result;
    };
    return workerThread_->IsCurrent()
        ? select()
        : workerThread_->BlockingCall(select);
}

AudioDeviceOperationResult WebRtcRuntime::SetRecordingEnabled(bool enabled)
{
    if (!workerThread_ || !workerThreadStarted_ ||
        !audioDeviceModule_) {
        return {
            false, activeRecordingDeviceId_,
            "audio_device_module_unavailable",
            "The Windows audio device module is unavailable."};
    }

    const auto apply = [this, enabled] {
        AudioDeviceOperationResult result;
        result.activeDeviceId = activeRecordingDeviceId_;
        if (!audioDeviceModule_->Initialized() &&
            audioDeviceModule_->Init() != 0) {
            result.errorCode =
                "audio_device_module_initialize_failed";
            result.errorMessage =
                "The Windows audio device module could not be initialized.";
            return result;
        }

        if (enabled) {
            if (!audioDeviceModule_->RecordingIsInitialized() &&
                audioDeviceModule_->InitRecording() != 0) {
                result.errorCode = "microphone_initialize_failed";
                result.errorMessage =
                    "The selected microphone could not be initialized.";
                return result;
            }
            if (!audioDeviceModule_->Recording() &&
                audioDeviceModule_->StartRecording() != 0) {
                result.errorCode = "microphone_start_failed";
                result.errorMessage =
                    "The selected microphone could not start recording.";
                return result;
            }
        } else if (audioDeviceModule_->RecordingIsInitialized() &&
                   audioDeviceModule_->StopRecording() != 0) {
            result.errorCode = "microphone_stop_failed";
            result.errorMessage =
                "The current microphone could not be stopped.";
            return result;
        }

        result.succeeded = true;
        return result;
    };
    return workerThread_->IsCurrent()
        ? apply()
        : workerThread_->BlockingCall(apply);
}

AudioDeviceOperationResult WebRtcRuntime::SelectPlayoutDevice(
    const std::string& deviceId)
{
    const std::string normalizedId =
        IsSystemDefaultDeviceId(deviceId)
            ? kSystemDefaultMediaDeviceId
            : deviceId;
    if (!workerThread_ || !workerThreadStarted_ ||
        !audioDeviceModule_) {
        return {
            false, activePlayoutDeviceId_,
            "audio_device_module_unavailable",
            "The Windows audio device module is unavailable."};
    }

    const auto select = [this, normalizedId] {
        AudioDeviceOperationResult result;
        result.activeDeviceId = activePlayoutDeviceId_;
        const bool moduleWasInitialized =
            audioDeviceModule_->Initialized();
        if (!moduleWasInitialized &&
            audioDeviceModule_->Init() != 0) {
            result.errorCode =
                "audio_device_module_initialize_failed";
            result.errorMessage =
                "The Windows audio device module could not be initialized.";
            return result;
        }
        if (moduleWasInitialized &&
            normalizedId != kSystemDefaultMediaDeviceId &&
            normalizedId == activePlayoutDeviceId_) {
            result.succeeded = true;
            return result;
        }

        const bool wasInitialized =
            audioDeviceModule_->PlayoutIsInitialized();
        const bool wasPlaying = audioDeviceModule_->Playing();
        const std::string previousId =
            activePlayoutDeviceId_.empty()
                ? std::string(kSystemDefaultMediaDeviceId)
                : activePlayoutDeviceId_;
        if (wasInitialized &&
            audioDeviceModule_->StopPlayout() != 0) {
            result.errorCode = "speaker_stop_failed";
            result.errorMessage =
                "The current speaker could not be stopped.";
            return result;
        }

        const auto restorePrevious = [&] {
            (void)audioDeviceModule_->StopPlayout();
            const bool selected = SelectAudioDevice(
                audioDeviceModule_.get(), false, previousId);
            bool restored = selected;
            if (restored && wasInitialized) {
                restored =
                    audioDeviceModule_->InitPlayout() == 0;
            }
            if (restored && wasPlaying) {
                restored =
                    audioDeviceModule_->StartPlayout() == 0;
            }
            activePlayoutDeviceId_ =
                restored ? previousId : std::string{};
            return restored;
        };

        if (!SelectAudioDevice(
                audioDeviceModule_.get(), false, normalizedId)) {
            (void)restorePrevious();
            result.activeDeviceId = activePlayoutDeviceId_;
            result.errorCode = "speaker_device_unavailable";
            result.errorMessage =
                "The selected speaker is no longer available.";
            return result;
        }
        if (wasInitialized &&
            audioDeviceModule_->InitPlayout() != 0) {
            (void)restorePrevious();
            result.activeDeviceId = activePlayoutDeviceId_;
            result.errorCode = "speaker_initialize_failed";
            result.errorMessage =
                "The selected speaker could not be initialized.";
            return result;
        }
        if (wasPlaying &&
            audioDeviceModule_->StartPlayout() != 0) {
            (void)restorePrevious();
            result.activeDeviceId = activePlayoutDeviceId_;
            result.errorCode = "speaker_restart_failed";
            result.errorMessage =
                "The selected speaker could not resume playback.";
            return result;
        }

        preferredPlayoutDeviceId_ = normalizedId;
        activePlayoutDeviceId_ = normalizedId;
        result.succeeded = true;
        result.activeDeviceId = normalizedId;
        return result;
    };
    return workerThread_->IsCurrent()
        ? select()
        : workerThread_->BlockingCall(select);
}

AudioDeviceOperationResult
WebRtcRuntime::ReapplyPreferredAudioDevices()
{
    if (!workerThread_ || !workerThreadStarted_ ||
        !audioDeviceModule_) {
        return {
            false, {},
            "audio_device_module_unavailable",
            "The Windows audio device module is unavailable."};
    }

    const auto reapply = [this] {
        const std::string recordingId =
            preferredRecordingDeviceId_.empty()
                ? std::string(kSystemDefaultMediaDeviceId)
                : preferredRecordingDeviceId_;
        const std::string playoutId =
            preferredPlayoutDeviceId_.empty()
                ? std::string(kSystemDefaultMediaDeviceId)
                : preferredPlayoutDeviceId_;

        // WebRtcVoiceEngine initializes a supplied legacy ADM with the
        // Windows communication defaults. Force the persisted choices back
        // after media-engine/PeerConnection initialization, even when the
        // cached active IDs appear unchanged.
        activeRecordingDeviceId_.clear();
        const auto recording =
            SelectRecordingDevice(recordingId);
        activePlayoutDeviceId_.clear();
        const auto playout =
            SelectPlayoutDevice(playoutId);
        if (!recording.succeeded) {
            return recording;
        }
        if (!playout.succeeded) {
            return playout;
        }
        AudioDeviceOperationResult result;
        result.succeeded = true;
        result.activeDeviceId =
            recording.activeDeviceId + "|" +
            playout.activeDeviceId;
        return result;
    };
    return workerThread_->IsCurrent()
        ? reapply()
        : workerThread_->BlockingCall(reapply);
}

webrtc::scoped_refptr<webrtc::PeerConnectionFactoryInterface>
WebRtcRuntime::PeerConnectionFactory() const
{
    return factory_;
}

}  // namespace remote
