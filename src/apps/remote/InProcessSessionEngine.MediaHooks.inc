// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

void InProcessSessionEngine::SetLocalCameraPreviewSink(
    webrtc::VideoSinkInterface<webrtc::VideoFrame>* sink)
{
    webrtc::scoped_refptr<webrtc::VideoTrackInterface> track;
    webrtc::VideoSinkInterface<webrtc::VideoFrame>* previous = nullptr;
    {
        std::lock_guard lock(mutex_);
        track = localCameraPreviewTrack_;
        previous = localCameraPreviewSink_;
        localCameraPreviewSink_ = sink;
    }
    if (track && previous && previous != sink) {
        track->RemoveSink(previous);
    }
    if (track && sink && previous != sink) {
        track->AddOrUpdateSink(sink, webrtc::VideoSinkWants());
    }
}

void InProcessSessionEngine::SetRemoteInputSink(IRemoteInputSink* sink)
{
    std::lock_guard lock(mutex_);
    remoteInputSink_ = sink;
}

void InProcessSessionEngine::SetRemoteFileTransferSink(
    IFileTransferSink* sink)
{
    std::lock_guard lock(mutex_);
    remoteFileTransferSink_ = sink;
}

void InProcessSessionEngine::SetRemoteClipboardSink(IClipboardSink* sink)
{
    std::lock_guard lock(mutex_);
    remoteClipboardSink_ = sink;
}

void InProcessSessionEngine::SetPreferredHardwareDecoderName(
    std::string name)
{
    if (runtime_) {
        runtime_->SetPreferredHardwareDecoderName(std::move(name));
    }
}

SessionCommandResult InProcessSessionEngine::ApplyVideoPipelinePreferences(
    DesktopCaptureImplementation desktopCaptureImplementation,
    VideoEncoderPreference videoEncoderPreference,
    FfmpegX264Preset quality,
    FfmpegHardwareBackend ffmpegHardwareBackend,
    std::string preferredAutomaticEncoderId,
    VideoDecoderPreference videoDecoderPreference)
{
    std::string runtimeError;
    std::vector<std::shared_ptr<RoomPairRuntime>> roomPairs;
    {
        std::lock_guard lock(mutex_);
        const bool roomCameraActive =
            snapshot_.localCamera != LocalCameraState::kOff ||
            std::any_of(
                snapshot_.room.members.begin(),
                snapshot_.room.members.end(),
                [](const RoomMemberSnapshot& member) {
                    return member.cameraPublishing;
                });
        const bool directMediaActive =
            snapshot_.state == SessionEngineState::kStarting ||
            snapshot_.state == SessionEngineState::kConnecting ||
            snapshot_.state ==
                SessionEngineState::kAwaitingLocalApproval ||
            snapshot_.state == SessionEngineState::kActive ||
            snapshot_.state == SessionEngineState::kStopping;
        if (snapshot_.room.screenShareState !=
                RoomScreenShareState::kIdle ||
            roomCameraActive || directMediaActive) {
            return Failure(
                "video_pipeline_settings_busy",
                "Stop the active video share before changing capture or codec settings.");
        }
        if (!runtime_ || !runtime_->ApplyVideoCodecPreferences(
                videoEncoderPreference, quality,
                ffmpegHardwareBackend,
                preferredAutomaticEncoderId,
                videoDecoderPreference, &runtimeError)) {
            return Failure(
                "video_pipeline_settings_apply_failed",
                runtimeError.empty()
                    ? "The selected video codec settings could not be applied."
                    : std::move(runtimeError));
        }

        options_.desktopCaptureImplementation =
            desktopCaptureImplementation;
        options_.videoEncoderPreference = videoEncoderPreference;
        options_.ffmpegX264Preset = quality;
        options_.ffmpegHardwareBackend = ffmpegHardwareBackend;
        options_.preferredAutomaticEncoderId =
            std::move(preferredAutomaticEncoderId);
        options_.videoDecoderPreference = videoDecoderPreference;
        const auto& report = runtime_->CapabilityReport();
        capabilities_.hasH264Encoder = report.hasH264Encoder;
        capabilities_.hasH264Decoder = report.hasH264Decoder;
        capabilities_.h264HardwareEncoderWired =
            report.h264HardwareEncoderWired;
        capabilities_.h264SoftwareEncoderWired =
            report.h264SoftwareEncoderWired;
        capabilities_.ffmpegX264EncoderWired =
            report.h264FfmpegX264EncoderWired;
        capabilities_.ffmpegX264EncoderError =
            report.h264FfmpegX264EncoderError;
        capabilities_.ffmpegHardwareEncoderWired =
            report.h264FfmpegHardwareEncoderWired;
        capabilities_.h264SoftwareEncoderFallback =
            report.h264SoftwareEncoderFallbackWired;
        capabilities_.videoEncoderPreference =
            EncoderPreferenceName(report.videoEncoderPreference);
        capabilities_.videoDecoderPreference =
            DecoderPreferenceName(report.videoDecoderPreference);
        capabilities_.desktopCapturePreference =
            DesktopCaptureImplementationName(
                options_.desktopCaptureImplementation);
        roomPairs.reserve(roomPairs_.size());
        for (const auto& [pairId, pair] : roomPairs_) {
            (void)pairId;
            if (pair && pair->session) {
                roomPairs.push_back(pair);
            }
        }
    }
    const bool fastDesktopBweStartup =
        desktopCaptureImplementation ==
            DesktopCaptureImplementation::kLibWebRtc;
    for (const auto& pair : roomPairs) {
        pair->session->SetFastDesktopBweStartupEnabled(
            fastDesktopBweStartup);
        pair->session->SetAdaptiveDesktopNetworkFrameRateEnabled(true);
    }
    PublishSnapshot();
    return Success();
}
