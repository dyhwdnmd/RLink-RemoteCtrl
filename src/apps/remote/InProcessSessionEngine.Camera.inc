// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

SessionCommandResult InProcessSessionEngine::SetLocalCameraEnabled(bool enabled)
{
    const auto snapshot = Snapshot();
    if (snapshot.room.membership != RoomMembershipState::kActive) {
        return Failure("room_not_active",
                       "Join a room before publishing the local camera.");
    }
    const bool alreadyEnabled =
        snapshot.localCamera == LocalCameraState::kPublishing;
    if (alreadyEnabled == enabled) {
        return Success();
    }

    if (enabled) {
        if (!options_.enableRealCameraCapture) {
            return Failure("camera_capture_disabled",
                           "Real camera capture is disabled for this engine.");
        }
        const std::string preferredCameraId =
            snapshot.localMediaDevices.camera.preferredDeviceId;
        if ((preferredCameraId == kSystemDefaultMediaDeviceId &&
             snapshot.localMediaDevices.camera.devices.empty()) ||
            (preferredCameraId != kSystemDefaultMediaDeviceId &&
             !ContainsMediaDevice(
                 snapshot.localMediaDevices.camera.devices,
                 preferredCameraId))) {
            return Failure(
                "camera_device_unavailable",
                "The selected camera is no longer available.");
        }
        {
            std::lock_guard lock(mutex_);
            snapshot_.localCamera = LocalCameraState::kStarting;
        }
        PublishSnapshot();
        auto source =
            webrtc::make_ref_counted<WindowsCameraCaptureSource>();
        if (!source->StartCapture(
                preferredCameraId == kSystemDefaultMediaDeviceId
                    ? std::string{}
                    : preferredCameraId)) {
            const std::string error = source->LastError();
            {
                std::lock_guard lock(mutex_);
                snapshot_.localCamera = LocalCameraState::kFailed;
                auto& category =
                    snapshot_.localMediaDevices.camera;
                category.state =
                    MediaDeviceSelectionState::kFailed;
                category.errorCode =
                    "camera_capture_start_failed";
                category.errorMessage = error;
                ++snapshot_.localMediaDevices.revision;
                snapshot_.errorCode = "camera_capture_start_failed";
                snapshot_.errorMessage = error;
            }
            PublishSnapshot();
            return Failure("camera_capture_start_failed", error);
        }
        const auto publishResult = SetRoomVideoSource(
            kCameraMainVideoSlot, source,
            "room-camera-" + snapshot.localDeviceId);
        if (!publishResult.accepted) {
            source->StopCapture();
            {
                std::lock_guard lock(mutex_);
                snapshot_.localCamera = LocalCameraState::kFailed;
            }
            PublishSnapshot();
            return publishResult;
        }

        webrtc::VideoSinkInterface<webrtc::VideoFrame>* previewSink = nullptr;
        webrtc::scoped_refptr<webrtc::VideoTrackInterface> previewTrack;
        {
            std::lock_guard lock(mutex_);
            localCameraCaptureSource_ = source;
            previewSink = localCameraPreviewSink_;
            const auto trackIt = localRoomVideoTracks_.find(
                kCameraMainVideoSlot);
            if (trackIt != localRoomVideoTracks_.end()) {
                previewTrack = static_cast<webrtc::VideoTrackInterface*>(
                    trackIt->second.get());
            }
            localCameraPreviewTrack_ = previewTrack;
            auto& category =
                snapshot_.localMediaDevices.camera;
            category.activeDeviceId =
                source->ActiveDeviceId();
            category.state =
                MediaDeviceSelectionState::kReady;
            category.errorCode.clear();
            category.errorMessage.clear();
            ++snapshot_.localMediaDevices.revision;
        }
        if (previewTrack && previewSink) {
            previewTrack->AddOrUpdateSink(
                previewSink, webrtc::VideoSinkWants());
        }

        const bool microphonePublishing =
            snapshot.localMicrophone == LocalMicrophoneState::kPublishing;
        const auto signalResult = signaling_->SetRoomMediaState(
            snapshot.room.roomId, true, microphonePublishing);
        if (!signalResult.accepted) {
            if (previewTrack && previewSink) {
                previewTrack->RemoveSink(previewSink);
            }
            (void)ClearRoomVideoSource(kCameraMainVideoSlot);
            source->StopCapture();
            {
                std::lock_guard lock(mutex_);
                localCameraCaptureSource_ = nullptr;
                localCameraPreviewTrack_ = nullptr;
                snapshot_.localCamera = LocalCameraState::kFailed;
                snapshot_.localMediaDevices.camera.activeDeviceId.clear();
                ++snapshot_.localMediaDevices.revision;
            }
            PublishSnapshot();
            return Failure(signalResult.errorCode, signalResult.errorMessage);
        }

        {
            std::lock_guard lock(mutex_);
            snapshot_.localCamera = LocalCameraState::kPublishing;
            snapshot_.errorCode.clear();
            snapshot_.errorMessage.clear();
        }
        PublishSnapshot();
        return Success();
    }

    webrtc::scoped_refptr<WindowsCameraCaptureSource> source;
    webrtc::scoped_refptr<webrtc::VideoTrackInterface> previewTrack;
    webrtc::VideoSinkInterface<webrtc::VideoFrame>* previewSink = nullptr;
    {
        std::lock_guard lock(mutex_);
        ++localCameraTrackGeneration_;
        snapshot_.localCamera = LocalCameraState::kStopping;
        source = std::exchange(localCameraCaptureSource_, nullptr);
        previewTrack = std::exchange(localCameraPreviewTrack_, nullptr);
        previewSink = localCameraPreviewSink_;
    }
    PublishSnapshot();
    const auto clearResult = ClearRoomVideoSource(kCameraMainVideoSlot);
    if (!clearResult.accepted) {
        {
            std::lock_guard lock(mutex_);
            localCameraCaptureSource_ = source;
            localCameraPreviewTrack_ = previewTrack;
            snapshot_.localCamera = LocalCameraState::kFailed;
        }
        PublishSnapshot();
        return clearResult;
    }
    if (previewTrack && previewSink) {
        previewTrack->RemoveSink(previewSink);
    }
    if (source) {
        // StopCapture can block in a camera driver's StopCapture call. The
        // UI has already detached the WebRTC track and published kOff, so
        // complete only the physical-device teardown on a retained worker.
        std::jthread stopThread(
            [capture = std::move(source)](std::stop_token) {
                capture->StopCapture();
            });
        std::lock_guard lock(mutex_);
        retiredCameraStopThreads_.push_back(std::move(stopThread));
    }
    const bool microphonePublishing =
        snapshot.localMicrophone == LocalMicrophoneState::kPublishing;
    const auto signalResult = signaling_->SetRoomMediaState(
        snapshot.room.roomId, false, microphonePublishing);
    {
        std::lock_guard lock(mutex_);
        snapshot_.localCamera = LocalCameraState::kOff;
        auto& category = snapshot_.localMediaDevices.camera;
        category.activeDeviceId.clear();
        category.state = MediaDeviceSelectionState::kReady;
        category.errorCode.clear();
        category.errorMessage.clear();
        ++snapshot_.localMediaDevices.revision;
    }
    PublishSnapshot();
    return signalResult.accepted
               ? Success()
               : Failure(signalResult.errorCode, signalResult.errorMessage);
}
