// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

SessionCommandResult InProcessSessionEngine::SetRemoteAudioPlaybackMuted(
    bool muted)
{
    std::vector<std::shared_ptr<RoomPairRuntime>> pairs;
    LibWebRtcSession* directSession = nullptr;
    {
        std::lock_guard lock(mutex_);
        snapshot_.roomAudioPlaybackMuted = muted;
        if (webRtcSession_ && directSession_.mediaSlotsPrepared &&
            snapshot_.purpose == SessionPurpose::kRemoteControl) {
            directSession = webRtcSession_.get();
        }
        pairs.reserve(roomPairs_.size());
        for (const auto& [pairId, pair] : roomPairs_) {
            (void)pairId;
            if (pair && pair->session && pair->mediaSlotsPrepared) {
                pairs.push_back(pair);
            }
        }
    }
    for (const auto& pair : pairs) {
        pair->session->SetRemoteAudioSlotEnabled(
            kMicrophoneMainAudioSlot, !muted);
    }
    if (directSession) {
        directSession->SetRemoteAudioSlotEnabled(
            kMicrophoneMainAudioSlot, !muted);
    }
    PublishSnapshot();
    return Success();
}

SessionCommandResult InProcessSessionEngine::SetRoomAudioPlaybackMuted(
    bool muted)
{
    return SetRemoteAudioPlaybackMuted(muted);
}

SessionCommandResult
InProcessSessionEngine::RefreshLocalMediaDevices()
{
    if (!runtime_) {
        return Failure(
            "webrtc_runtime_unavailable",
            "The WebRTC runtime is unavailable.");
    }

    {
        std::lock_guard lock(mutex_);
        if (snapshot_.state == SessionEngineState::kStopping ||
            snapshot_.state == SessionEngineState::kStopped) {
            return Failure(
                "session_engine_stopping",
                "The session engine is stopping.");
        }
        if (snapshot_.localMediaDevices.refreshing) {
            return Success();
        }
        snapshot_.localMediaDevices.refreshing = true;
        ++snapshot_.localMediaDevices.revision;
        mediaDeviceOperationThreads_.emplace_back(
            [this](std::stop_token stopToken) {
                std::vector<MediaDeviceDescriptor> cameras;
                for (const auto& device :
                     WindowsCameraCaptureSource::EnumerateDevices()) {
                    if (stopToken.stop_requested()) {
                        return;
                    }
                    cameras.push_back(
                        {device.id, device.name, true});
                }
                const auto audioDevices =
                    runtime_->EnumerateAudioDevices();

                {
                    std::lock_guard lock(mutex_);
                    if (snapshot_.state ==
                            SessionEngineState::kStopping ||
                        snapshot_.state ==
                            SessionEngineState::kStopped ||
                        stopToken.stop_requested()) {
                        snapshot_.localMediaDevices.refreshing = false;
                        return;
                    }
                    auto& camera =
                        snapshot_.localMediaDevices.camera;
                    auto& microphone =
                        snapshot_.localMediaDevices.microphone;
                    auto& speaker =
                        snapshot_.localMediaDevices.speaker;
                    camera.devices = std::move(cameras);
                    microphone.devices =
                        audioDevices.microphones;
                    speaker.devices = audioDevices.speakers;

                    if (localCameraCaptureSource_) {
                        camera.activeDeviceId =
                            localCameraCaptureSource_
                                ->ActiveDeviceId();
                        const auto activeCamera = std::find_if(
                            camera.devices.begin(),
                            camera.devices.end(),
                            [&camera](const MediaDeviceDescriptor& device) {
                                return device.id ==
                                    camera.activeDeviceId;
                            });
                        camera.activeDeviceName =
                            activeCamera == camera.devices.end()
                                ? std::string{}
                                : activeCamera->name;
                    } else {
                        camera.activeDeviceId.clear();
                        camera.activeDeviceName.clear();
                    }
                    microphone.activeDeviceId =
                        audioDevices.activeMicrophoneId;
                    microphone.activeDeviceName =
                        audioDevices.activeMicrophoneName;
                    speaker.activeDeviceId =
                        audioDevices.activeSpeakerId;
                    speaker.activeDeviceName =
                        audioDevices.activeSpeakerName;

                    const auto updateAvailability =
                        [](MediaDeviceCategorySnapshot& category,
                           const char* unavailableCode,
                           const char* unavailableMessage) {
                            if (category.state ==
                                MediaDeviceSelectionState::kSwitching) {
                                return;
                            }
                            const bool followsDefault =
                                category.preferredDeviceId.empty() ||
                                category.preferredDeviceId ==
                                    kSystemDefaultMediaDeviceId;
                            const bool available = followsDefault
                                ? !category.devices.empty()
                                : ContainsMediaDevice(
                                      category.devices,
                                      category.preferredDeviceId);
                            if (available) {
                                category.state =
                                    MediaDeviceSelectionState::kReady;
                                category.errorCode.clear();
                                category.errorMessage.clear();
                            } else {
                                category.state =
                                    MediaDeviceSelectionState::kUnavailable;
                                category.errorCode = unavailableCode;
                                category.errorMessage =
                                    unavailableMessage;
                            }
                        };
                    updateAvailability(
                        camera, "camera_device_unavailable",
                        "The selected camera is not connected.");
                    updateAvailability(
                        microphone,
                        "microphone_device_unavailable",
                        "The selected microphone is not connected.");
                    updateAvailability(
                        speaker, "speaker_device_unavailable",
                        "The selected speaker is not connected.");
                    if (!audioDevices.error.empty()) {
                        if (microphone.devices.empty()) {
                            microphone.errorMessage =
                                audioDevices.error;
                        }
                        if (speaker.devices.empty()) {
                            speaker.errorMessage =
                                audioDevices.error;
                        }
                    }
                    snapshot_.localMediaDevices.refreshing = false;
                    ++snapshot_.localMediaDevices.revision;
                }
                PublishSnapshot();
            });
    }
    PublishSnapshot();
    return Success();
}

SessionCommandResult
InProcessSessionEngine::SelectLocalCameraDevice(
    const std::string& deviceId)
{
    const std::string normalizedId =
        NormalizeMediaDeviceId(deviceId);
    bool cameraPublishing = false;
    std::uint64_t trackGeneration = 0;
    std::string expectedRoomId;
    {
        std::lock_guard lock(mutex_);
        auto& category = snapshot_.localMediaDevices.camera;
        if (snapshot_.state == SessionEngineState::kStopping ||
            snapshot_.state == SessionEngineState::kStopped) {
            return Failure(
                "session_engine_stopping",
                "The session engine is stopping.");
        }
        if (category.state ==
            MediaDeviceSelectionState::kSwitching) {
            return Failure(
                "camera_device_switch_in_progress",
                "A camera switch is already in progress.");
        }
        if (normalizedId != kSystemDefaultMediaDeviceId &&
            !ContainsMediaDevice(category.devices, normalizedId)) {
            return Failure(
                "camera_device_unavailable",
                "The selected camera is no longer available.");
        }
        if (normalizedId == kSystemDefaultMediaDeviceId &&
            category.devices.empty()) {
            return Failure(
                "camera_device_unavailable",
                "No camera device is available.");
        }
        if (category.preferredDeviceId == normalizedId &&
            category.state ==
                MediaDeviceSelectionState::kReady) {
            return Success();
        }
        cameraPublishing =
            snapshot_.localCamera ==
            LocalCameraState::kPublishing;
        if (!cameraPublishing) {
            category.preferredDeviceId = normalizedId;
            category.activeDeviceId.clear();
            category.state =
                MediaDeviceSelectionState::kReady;
            category.errorCode.clear();
            category.errorMessage.clear();
            options_.preferredCameraDeviceId = normalizedId;
            ++snapshot_.localMediaDevices.revision;
        } else {
            category.state =
                MediaDeviceSelectionState::kSwitching;
            category.errorCode.clear();
            category.errorMessage.clear();
            trackGeneration = ++localCameraTrackGeneration_;
            expectedRoomId = snapshot_.room.roomId;
            ++snapshot_.localMediaDevices.revision;
        }
    }
    PublishSnapshot();
    if (!cameraPublishing) {
        return Success();
    }

    {
        std::lock_guard lock(mutex_);
        if (snapshot_.state == SessionEngineState::kStopping ||
            snapshot_.state == SessionEngineState::kStopped) {
            return Failure(
                "session_engine_stopping",
                "The session engine is stopping.");
        }
        mediaDeviceOperationThreads_.emplace_back(
            [this, normalizedId, trackGeneration,
             expectedRoomId](std::stop_token) {
            auto source =
                webrtc::make_ref_counted<
                    WindowsCameraCaptureSource>();
            const std::string requestedId =
                normalizedId == kSystemDefaultMediaDeviceId
                    ? std::string{}
                    : normalizedId;
            if (!source->StartCapture(requestedId)) {
                const std::string error = source->LastError();
                {
                    std::lock_guard lock(mutex_);
                    auto& category =
                        snapshot_.localMediaDevices.camera;
                    if (localCameraTrackGeneration_ ==
                            trackGeneration &&
                        category.state ==
                            MediaDeviceSelectionState::kSwitching) {
                        category.state =
                            MediaDeviceSelectionState::kFailed;
                        category.errorCode =
                            "camera_device_switch_failed";
                        category.errorMessage = error;
                        ++snapshot_.localMediaDevices.revision;
                    }
                }
                PublishSnapshot();
                return;
            }

            webrtc::scoped_refptr<WindowsCameraCaptureSource>
                previousSource;
            webrtc::scoped_refptr<webrtc::VideoTrackInterface>
                previousPreviewTrack;
            webrtc::VideoSinkInterface<webrtc::VideoFrame>*
                previewSink = nullptr;
            bool switchStillCurrent = false;
            {
                std::lock_guard lock(mutex_);
                previousSource = localCameraCaptureSource_;
                previousPreviewTrack = localCameraPreviewTrack_;
                previewSink = localCameraPreviewSink_;
                const auto& category =
                    snapshot_.localMediaDevices.camera;
                switchStillCurrent =
                    localCameraTrackGeneration_ ==
                        trackGeneration &&
                    category.state ==
                        MediaDeviceSelectionState::kSwitching &&
                    snapshot_.localCamera ==
                        LocalCameraState::kPublishing &&
                    snapshot_.room.membership ==
                        RoomMembershipState::kActive &&
                    snapshot_.room.roomId == expectedRoomId;
            }
            if (!switchStillCurrent) {
                source->StopCapture();
                return;
            }

            const auto publishResult = SetRoomVideoSource(
                kCameraMainVideoSlot, source,
                "room-camera-switch-" +
                    std::to_string(trackGeneration),
                expectedRoomId, trackGeneration);
            if (!publishResult.accepted) {
                source->StopCapture();
                {
                    std::lock_guard lock(mutex_);
                    auto& category =
                        snapshot_.localMediaDevices.camera;
                    if (localCameraTrackGeneration_ ==
                            trackGeneration &&
                        category.state ==
                            MediaDeviceSelectionState::kSwitching) {
                        category.state =
                            MediaDeviceSelectionState::kFailed;
                        category.errorCode =
                            publishResult.errorCode;
                        category.errorMessage =
                            publishResult.errorMessage;
                        ++snapshot_.localMediaDevices.revision;
                    }
                }
                PublishSnapshot();
                return;
            }

            webrtc::scoped_refptr<webrtc::VideoTrackInterface>
                newPreviewTrack;
            bool committed = false;
            {
                std::lock_guard lock(mutex_);
                auto& category =
                    snapshot_.localMediaDevices.camera;
                if (localCameraTrackGeneration_ ==
                        trackGeneration &&
                    category.state ==
                        MediaDeviceSelectionState::kSwitching &&
                    snapshot_.localCamera ==
                        LocalCameraState::kPublishing &&
                    snapshot_.room.roomId == expectedRoomId) {
                    const auto track = localRoomVideoTracks_.find(
                        kCameraMainVideoSlot);
                    if (track != localRoomVideoTracks_.end()) {
                        newPreviewTrack =
                            static_cast<webrtc::VideoTrackInterface*>(
                                track->second.get());
                    }
                    localCameraCaptureSource_ = source;
                    localCameraPreviewTrack_ = newPreviewTrack;
                    category.preferredDeviceId = normalizedId;
                    category.activeDeviceId =
                        source->ActiveDeviceId();
                    category.state =
                        MediaDeviceSelectionState::kReady;
                    category.errorCode.clear();
                    category.errorMessage.clear();
                    options_.preferredCameraDeviceId = normalizedId;
                    ++snapshot_.localMediaDevices.revision;
                    committed = true;
                }
            }
            if (!committed) {
                source->StopCapture();
                return;
            }
            if (newPreviewTrack && previewSink) {
                newPreviewTrack->AddOrUpdateSink(
                    previewSink, webrtc::VideoSinkWants());
            }
            if (previousPreviewTrack && previewSink &&
                previousPreviewTrack != newPreviewTrack) {
                previousPreviewTrack->RemoveSink(previewSink);
            }
            if (previousSource && previousSource != source) {
                previousSource->StopCapture();
            }
            PublishSnapshot();
        });
    }
    return Success();
}

SessionCommandResult
InProcessSessionEngine::SelectLocalMicrophoneDevice(
    const std::string& deviceId)
{
    const std::string normalizedId =
        NormalizeMediaDeviceId(deviceId);
    {
        std::lock_guard lock(mutex_);
        auto& category =
            snapshot_.localMediaDevices.microphone;
        if (snapshot_.state == SessionEngineState::kStopping ||
            snapshot_.state == SessionEngineState::kStopped) {
            return Failure(
                "session_engine_stopping",
                "The session engine is stopping.");
        }
        if (category.state ==
            MediaDeviceSelectionState::kSwitching) {
            return Failure(
                "microphone_device_switch_in_progress",
                "A microphone switch is already in progress.");
        }
        if (normalizedId != kSystemDefaultMediaDeviceId &&
            !ContainsMediaDevice(category.devices, normalizedId)) {
            return Failure(
                "microphone_device_unavailable",
                "The selected microphone is no longer available.");
        }
        if (normalizedId == kSystemDefaultMediaDeviceId &&
            category.devices.empty()) {
            return Failure(
                "microphone_device_unavailable",
                "No microphone device is available.");
        }
        if (category.preferredDeviceId == normalizedId &&
            category.state ==
                MediaDeviceSelectionState::kReady) {
            return Success();
        }
        category.state =
            MediaDeviceSelectionState::kSwitching;
        category.errorCode.clear();
        category.errorMessage.clear();
        ++snapshot_.localMediaDevices.revision;
    }
    PublishSnapshot();

    {
        std::lock_guard lock(mutex_);
        if (snapshot_.state == SessionEngineState::kStopping ||
            snapshot_.state == SessionEngineState::kStopped) {
            return Failure(
                "session_engine_stopping",
                "The session engine is stopping.");
        }
        mediaDeviceOperationThreads_.emplace_back(
            [this, normalizedId](std::stop_token) {
            const auto result =
                runtime_->SelectRecordingDevice(normalizedId);
            {
                std::lock_guard lock(mutex_);
                auto& category =
                    snapshot_.localMediaDevices.microphone;
                category.activeDeviceId =
                    result.activeDeviceId;
                if (result.succeeded) {
                    category.preferredDeviceId =
                        normalizedId;
                    category.state =
                        MediaDeviceSelectionState::kReady;
                    category.errorCode.clear();
                    category.errorMessage.clear();
                    options_.preferredMicrophoneDeviceId =
                        normalizedId;
                } else {
                    category.state =
                        MediaDeviceSelectionState::kFailed;
                    category.errorCode = result.errorCode;
                    category.errorMessage =
                        result.errorMessage;
                }
                ++snapshot_.localMediaDevices.revision;
            }
            PublishSnapshot();
        });
    }
    return Success();
}

SessionCommandResult
InProcessSessionEngine::SelectLocalSpeakerDevice(
    const std::string& deviceId)
{
    const std::string normalizedId =
        NormalizeMediaDeviceId(deviceId);
    {
        std::lock_guard lock(mutex_);
        auto& category =
            snapshot_.localMediaDevices.speaker;
        if (snapshot_.state == SessionEngineState::kStopping ||
            snapshot_.state == SessionEngineState::kStopped) {
            return Failure(
                "session_engine_stopping",
                "The session engine is stopping.");
        }
        if (category.state ==
            MediaDeviceSelectionState::kSwitching) {
            return Failure(
                "speaker_device_switch_in_progress",
                "A speaker switch is already in progress.");
        }
        if (normalizedId != kSystemDefaultMediaDeviceId &&
            !ContainsMediaDevice(category.devices, normalizedId)) {
            return Failure(
                "speaker_device_unavailable",
                "The selected speaker is no longer available.");
        }
        if (normalizedId == kSystemDefaultMediaDeviceId &&
            category.devices.empty()) {
            return Failure(
                "speaker_device_unavailable",
                "No speaker device is available.");
        }
        if (category.preferredDeviceId == normalizedId &&
            category.state ==
                MediaDeviceSelectionState::kReady) {
            return Success();
        }
        category.state =
            MediaDeviceSelectionState::kSwitching;
        category.errorCode.clear();
        category.errorMessage.clear();
        ++snapshot_.localMediaDevices.revision;
    }
    PublishSnapshot();

    {
        std::lock_guard lock(mutex_);
        if (snapshot_.state == SessionEngineState::kStopping ||
            snapshot_.state == SessionEngineState::kStopped) {
            return Failure(
                "session_engine_stopping",
                "The session engine is stopping.");
        }
        mediaDeviceOperationThreads_.emplace_back(
            [this, normalizedId](std::stop_token) {
            const auto result =
                runtime_->SelectPlayoutDevice(normalizedId);
            {
                std::lock_guard lock(mutex_);
                auto& category =
                    snapshot_.localMediaDevices.speaker;
                category.activeDeviceId =
                    result.activeDeviceId;
                if (result.succeeded) {
                    category.preferredDeviceId =
                        normalizedId;
                    category.state =
                        MediaDeviceSelectionState::kReady;
                    category.errorCode.clear();
                    category.errorMessage.clear();
                    options_.preferredSpeakerDeviceId =
                        normalizedId;
                } else {
                    category.state =
                        MediaDeviceSelectionState::kFailed;
                    category.errorCode = result.errorCode;
                    category.errorMessage =
                        result.errorMessage;
                }
                ++snapshot_.localMediaDevices.revision;
            }
            PublishSnapshot();
        });
    }
    return Success();
}
