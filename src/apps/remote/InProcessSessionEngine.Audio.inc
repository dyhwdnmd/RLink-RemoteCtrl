// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

SessionCommandResult InProcessSessionEngine::SetLocalMicrophoneEnabled(
    bool enabled)
{
    const auto snapshot = Snapshot();
    const bool roomActive =
        snapshot.room.membership == RoomMembershipState::kActive;
    const bool directActive =
        snapshot.state == SessionEngineState::kActive &&
        snapshot.purpose == SessionPurpose::kRemoteControl &&
        !snapshot.sessionId.empty();
    if (!roomActive && !directActive) {
        return Failure("session_not_active",
                       "Start a remote session before publishing the microphone.");
    }
    const bool alreadyEnabled =
        snapshot.localMicrophone == LocalMicrophoneState::kPublishing;
    if (enabled && alreadyEnabled) {
        return Success();
    }

    if (enabled) {
        const auto& microphoneDevice =
            snapshot.localMediaDevices.microphone;
        const bool followsDefault =
            microphoneDevice.preferredDeviceId.empty() ||
            microphoneDevice.preferredDeviceId ==
                kSystemDefaultMediaDeviceId;
        const bool selectedAvailable = followsDefault
            ? !microphoneDevice.devices.empty()
            : ContainsMediaDevice(
                  microphoneDevice.devices,
                  microphoneDevice.preferredDeviceId);
        if (!selectedAvailable) {
            return Failure(
                "microphone_device_unavailable",
                "The selected microphone is no longer available.");
        }
        {
            std::lock_guard lock(mutex_);
            snapshot_.localMicrophone = LocalMicrophoneState::kStarting;
        }
        PublishSnapshot();
        const auto captureResult =
            runtime_->SetRecordingEnabled(true);
        if (!captureResult.succeeded) {
            {
                std::lock_guard lock(mutex_);
                snapshot_.localMicrophone =
                    LocalMicrophoneState::kFailed;
                snapshot_.errorCode = captureResult.errorCode;
                snapshot_.errorMessage = captureResult.errorMessage;
            }
            PublishSnapshot();
            return Failure(
                captureResult.errorCode,
                captureResult.errorMessage);
        }
        const auto factory = runtime_ ? runtime_->PeerConnectionFactory()
                                      : nullptr;
        if (!factory) {
            (void)runtime_->SetRecordingEnabled(false);
            {
                std::lock_guard lock(mutex_);
                snapshot_.localMicrophone = LocalMicrophoneState::kFailed;
            }
            PublishSnapshot();
            return Failure("webrtc_factory_unavailable",
                           "The WebRTC factory is not ready.");
        }
        webrtc::AudioOptions options;
        auto source = factory->CreateAudioSource(options);
        auto track = factory->CreateAudioTrack(
            "remote-microphone-" + snapshot.localDeviceId, source.get());
        if (!source || !track) {
            (void)runtime_->SetRecordingEnabled(false);
            {
                std::lock_guard lock(mutex_);
                snapshot_.localMicrophone = LocalMicrophoneState::kFailed;
            }
            PublishSnapshot();
            return Failure("microphone_track_create_failed",
                           "The local microphone track could not be created.");
        }
        // Keep the new track silent until every sender is attached and the
        // room media-state update has been accepted by signaling.
        track->set_enabled(false);

        std::vector<std::shared_ptr<RoomPairRuntime>> candidates;
        LibWebRtcSession* directMediaSession = nullptr;
        {
            std::lock_guard lock(mutex_);
            if (roomActive) {
                candidates.reserve(roomPairs_.size());
                for (const auto& [pairId, pair] : roomPairs_) {
                    (void)pairId;
                    if (pair && pair->session && pair->mediaSlotsPrepared) {
                        candidates.push_back(pair);
                    }
                }
            }
            if (directActive && webRtcSession_ &&
                directSession_.mediaSlotsPrepared) {
                directMediaSession = webRtcSession_.get();
            }
        }
        std::vector<std::shared_ptr<RoomPairRuntime>> attached;
        bool directAttached = false;
        std::optional<SessionCommandResult> attachFailure;
        for (const auto& pair : candidates) {
            if (!pair->session->AudioSlotPrepared(
                        kMicrophoneMainAudioSlot)) {
                continue;
            }
            const auto result = pair->session->SetAudioSlotTrack(
                kMicrophoneMainAudioSlot, track);
            if (!result.ok()) {
                for (const auto& previous : attached) {
                    previous->session->SetAudioSlotTrack(
                        kMicrophoneMainAudioSlot, nullptr);
                }
                attachFailure = Failure(
                    "microphone_track_attach_failed",
                    std::string(result.message()));
                break;
            }
            attached.push_back(pair);
        }
        if (!attachFailure && directActive) {
            if (!directMediaSession ||
                !directMediaSession->AudioSlotPrepared(
                    kMicrophoneMainAudioSlot)) {
                attachFailure = Failure(
                    "direct_audio_slot_unavailable",
                    "The direct microphone channel is not ready.");
            } else {
                const auto result = directMediaSession->SetAudioSlotTrack(
                    kMicrophoneMainAudioSlot, track);
                if (!result.ok()) {
                    attachFailure = Failure(
                        "microphone_track_attach_failed",
                        std::string(result.message()));
                } else {
                    directAttached = true;
                }
            }
        }
        if (attachFailure) {
            for (const auto& previous : attached) {
                previous->session->SetAudioSlotTrack(
                    kMicrophoneMainAudioSlot, nullptr);
            }
            if (directAttached && directMediaSession) {
                directMediaSession->SetAudioSlotTrack(
                    kMicrophoneMainAudioSlot, nullptr);
            }
        }
        {
            std::lock_guard lock(mutex_);
            if (attachFailure) {
                snapshot_.localMicrophone =
                    LocalMicrophoneState::kFailed;
            }
            else {
                localMicrophoneAudioSource_ = source;
                localMicrophoneAudioTrack_ = track;
            }
        }
        if (attachFailure) {
            (void)runtime_->SetRecordingEnabled(false);
            PublishSnapshot();
            return *attachFailure;
        }
        SignalingOperationResult signalResult = {true, {}, {}};
        if (roomActive) {
            const bool cameraPublishing =
                snapshot.localCamera == LocalCameraState::kPublishing;
            signalResult = signaling_->SetRoomMediaState(
                snapshot.room.roomId, cameraPublishing, true);
        }
        if (!signalResult.accepted) {
            for (const auto& pair : attached) {
                pair->session->SetAudioSlotTrack(
                    kMicrophoneMainAudioSlot, nullptr);
            }
            if (directAttached && directMediaSession) {
                directMediaSession->SetAudioSlotTrack(
                    kMicrophoneMainAudioSlot, nullptr);
            }
            {
                std::lock_guard lock(mutex_);
                localMicrophoneAudioTrack_ = nullptr;
                localMicrophoneAudioSource_ = nullptr;
                snapshot_.localMicrophone = LocalMicrophoneState::kFailed;
            }
            (void)runtime_->SetRecordingEnabled(false);
            PublishSnapshot();
            return Failure(signalResult.errorCode, signalResult.errorMessage);
        }
        track->set_enabled(true);
        {
            std::lock_guard lock(mutex_);
            snapshot_.localMicrophone = LocalMicrophoneState::kPublishing;
        }
        PublishSnapshot();
        return Success();
    }

    std::vector<std::shared_ptr<RoomPairRuntime>> pairs;
    LibWebRtcSession* directMediaSession = nullptr;
    webrtc::scoped_refptr<webrtc::AudioTrackInterface>
        trackToDisable;
    {
        std::lock_guard lock(mutex_);
        snapshot_.localMicrophone = LocalMicrophoneState::kStopping;
        if (roomActive) {
            pairs.reserve(roomPairs_.size());
            for (const auto& [pairId, pair] : roomPairs_) {
                (void)pairId;
                if (pair && pair->session && pair->mediaSlotsPrepared) {
                    pairs.push_back(pair);
                }
            }
        }
        if (directActive && webRtcSession_ &&
            directSession_.mediaSlotsPrepared) {
            directMediaSession = webRtcSession_.get();
        }
        trackToDisable = localMicrophoneAudioTrack_;
        if (trackToDisable) {
            // Silence immediately on the caller thread. Sender detachment and
            // physical ADM shutdown may cross WebRTC worker threads.
            trackToDisable->set_enabled(false);
        }
        localMicrophoneAudioTrack_ = nullptr;
        localMicrophoneAudioSource_ = nullptr;
    }
    for (const auto& pair : pairs) {
        if (pair->session->AudioSlotPrepared(kMicrophoneMainAudioSlot)) {
            pair->session->SetAudioSlotTrack(
                kMicrophoneMainAudioSlot, nullptr);
        }
    }
    if (directMediaSession && directMediaSession->AudioSlotPrepared(
            kMicrophoneMainAudioSlot)) {
        directMediaSession->SetAudioSlotTrack(
            kMicrophoneMainAudioSlot, nullptr);
    }
    PublishSnapshot();
    SignalingOperationResult signalResult = {true, {}, {}};
    if (roomActive) {
        const bool cameraPublishing =
            snapshot.localCamera == LocalCameraState::kPublishing;
        signalResult = signaling_->SetRoomMediaState(
            snapshot.room.roomId, cameraPublishing, false);
    }
    const auto captureResult =
        runtime_->SetRecordingEnabled(false);
    {
        std::lock_guard lock(mutex_);
        snapshot_.localMicrophone = captureResult.succeeded
            ? LocalMicrophoneState::kOff
            : LocalMicrophoneState::kFailed;
        if (!captureResult.succeeded) {
            snapshot_.errorCode = captureResult.errorCode;
            snapshot_.errorMessage = captureResult.errorMessage;
        }
    }
    PublishSnapshot();
    if (!captureResult.succeeded) {
        return Failure(
            captureResult.errorCode,
            captureResult.errorMessage);
    }
    return signalResult.accepted
        ? Success()
        : Failure(signalResult.errorCode, signalResult.errorMessage);
}
