// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#pragma once

#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "api/audio/audio_device.h"
#include "api/peer_connection_interface.h"
#include "api/scoped_refptr.h"
#include "src/core/MediaDevice.h"
#include "src/platform/win/MfH264EncoderCapabilityProbe.h"
#include "VideoDecoderRuntimeStatus.h"
#include "VideoEncoderRuntimeStatus.h"

namespace webrtc {
class Thread;
class WinsockInitializer;
}

namespace remote {

class MfD3D11H264DecoderPreferenceState;
class WindowsPreferredVideoEncoderFactory;
class WindowsPreferredVideoDecoderFactory;

struct WebRtcCapabilityReport {
    bool factoryCreated = false;
    bool windowsPreferredEncoderFactoryCreated = false;
    bool windowsPreferredDecoderFactoryCreated = false;
    bool hasH264Encoder = false;
    bool hasH264Decoder = false;
    bool h264HardwareEncoderAvailable = false;
    bool h264HardwareEncoderCpuNv12InputSupported = false;
    bool h264HardwareEncoderD3D11InputCandidate = false;
    uint32_t h264HardwareEncoderCount = 0;
    bool h264HardwareEncoderWired = false;
    bool h264SoftwareEncoderWired = false;
    bool h264FfmpegX264EncoderWired = false;
    std::string h264FfmpegX264EncoderError;
    bool h264FfmpegHardwareEncoderAvailable = false;
    bool h264FfmpegHardwareEncoderWired = false;
    std::string h264FfmpegHardwareEncoderError;
    std::vector<std::string> h264FfmpegHardwareEncoderDescriptions;
    bool h264SoftwareEncoderFallbackWired = false;
    bool h264MfDecoderWired = false;
    bool h264MfDecoderConfigured = false;
    bool h264MfDecoderHardware = false;
    bool h264MfDecoderSoftware = false;
    bool h264MfDecoderNativeOutputSupported = false;
    bool h264MfDecoderAsynchronous = false;
    bool h264FfmpegSoftwareDecoderWired = false;
    std::string h264MfDecoderName;
    std::string h264MfDecoderError;
    std::vector<std::string> h264HardwareEncoderDescriptions;
    std::vector<std::string> h264HardwareEncoderWarnings;
    std::vector<std::string> videoEncoderFormats;
    std::vector<std::string> videoDecoderFormats;
    std::string error;
    std::string hardwareFingerprint;
    bool h264HardwareEncoderProbeSucceeded = false;
    bool h264HardwareEncoderProbeFromCache = false;
    bool audioDeviceModuleCreated = false;
    std::string audioDeviceError;
    VideoEncoderPreference videoEncoderPreference =
        VideoEncoderPreference::kAutomatic;
    VideoDecoderPreference videoDecoderPreference =
        VideoDecoderPreference::kAutomatic;
};

struct AudioDeviceEnumeration {
    std::vector<MediaDeviceDescriptor> microphones;
    std::vector<MediaDeviceDescriptor> speakers;
    std::string activeMicrophoneId;
    std::string activeMicrophoneName;
    std::string activeSpeakerId;
    std::string activeSpeakerName;
    std::string error;
};

struct AudioDeviceOperationResult {
    bool succeeded = false;
    std::string activeDeviceId;
    std::string errorCode;
    std::string errorMessage;
};

class WebRtcRuntime final {
public:
    explicit WebRtcRuntime(
        VideoEncoderPreference videoEncoderPreference =
            VideoEncoderPreference::kAutomatic,
        VideoDecoderPreference videoDecoderPreference =
            VideoDecoderPreference::kAutomatic,
        std::string preferredHardwareDecoderName = {},
        std::string hardwareFingerprint = {},
        std::optional<MfH264EncoderCapabilityCache>
            encoderCapabilityCache = std::nullopt,
        std::string preferredRecordingDeviceId =
            kSystemDefaultMediaDeviceId,
        std::string preferredPlayoutDeviceId =
            kSystemDefaultMediaDeviceId,
        FfmpegX264Preset ffmpegX264Preset =
            FfmpegX264Preset::kMedium,
        FfmpegHardwareBackend ffmpegHardwareBackend =
            FfmpegHardwareBackend::kAutomatic,
        std::string preferredAutomaticEncoderId = {});
    ~WebRtcRuntime();

    WebRtcRuntime(const WebRtcRuntime&) = delete;
    WebRtcRuntime& operator=(const WebRtcRuntime&) = delete;

    bool Initialize();
    void Shutdown();

    const WebRtcCapabilityReport& CapabilityReport() const noexcept;
    VideoEncoderRuntimeSnapshot EncoderRuntimeStatus() const;
    void SetDesktopShareGeneration(std::uint64_t generation);
    void SetDesktopCaptureAdapterLuid(std::uint64_t adapterLuid);
    void SetPreferredHardwareDecoderName(std::string name);
    bool ApplyVideoCodecPreferences(
        VideoEncoderPreference videoEncoderPreference,
        FfmpegX264Preset quality,
        FfmpegHardwareBackend ffmpegHardwareBackend,
        std::string preferredAutomaticEncoderId,
        VideoDecoderPreference videoDecoderPreference,
        std::string* error = nullptr);
    AudioDeviceEnumeration EnumerateAudioDevices() const;
    AudioDeviceOperationResult SelectRecordingDevice(
        const std::string& deviceId);
    AudioDeviceOperationResult SetRecordingEnabled(bool enabled);
    AudioDeviceOperationResult SelectPlayoutDevice(
        const std::string& deviceId);
    AudioDeviceOperationResult ReapplyPreferredAudioDevices();
    webrtc::scoped_refptr<webrtc::PeerConnectionFactoryInterface>
        PeerConnectionFactory() const;

private:
    std::unique_ptr<webrtc::Thread> networkThread_;
    std::unique_ptr<webrtc::Thread> workerThread_;
    std::unique_ptr<webrtc::Thread> signalingThread_;
    std::unique_ptr<webrtc::WinsockInitializer> winsockInitializer_;

    bool networkThreadStarted_ = false;
    bool workerThreadStarted_ = false;
    bool signalingThreadStarted_ = false;
    bool sslInitialized_ = false;

    webrtc::scoped_refptr<webrtc::PeerConnectionFactoryInterface> factory_;
    // Owned by factory_. These non-owning pointers remain valid until
    // factory_ is released in Shutdown().
    WindowsPreferredVideoEncoderFactory* videoEncoderFactory_ = nullptr;
    WindowsPreferredVideoDecoderFactory* videoDecoderFactory_ = nullptr;
    webrtc::scoped_refptr<webrtc::AudioDeviceModule> audioDeviceModule_;
    WebRtcCapabilityReport report_;
    VideoEncoderPreference videoEncoderPreference_;
    FfmpegX264Preset ffmpegX264Preset_ = FfmpegX264Preset::kMedium;
    FfmpegHardwareBackend ffmpegHardwareBackend_ =
        FfmpegHardwareBackend::kAutomatic;
    std::string preferredAutomaticEncoderId_;
    VideoDecoderPreference videoDecoderPreference_;
    std::shared_ptr<VideoEncoderRuntimeState> encoderRuntimeState_;
    std::shared_ptr<MfD3D11H264DecoderPreferenceState>
        decoderPreferenceState_;
    std::string hardwareFingerprint_;
    std::optional<MfH264EncoderCapabilityCache>
        encoderCapabilityCache_;
    std::string preferredRecordingDeviceId_;
    std::string preferredPlayoutDeviceId_;
    std::string activeRecordingDeviceId_;
    std::string activePlayoutDeviceId_;
};

}  // namespace remote
