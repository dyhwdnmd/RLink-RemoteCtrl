// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

InProcessSessionEngine::InProcessSessionEngine()
    : InProcessSessionEngine(nullptr, {})
{}

InProcessSessionEngine::InProcessSessionEngine(
    std::unique_ptr<ISignalingClient> signaling,
    SignalingClientConfig signalingConfig,
    InProcessSessionEngineOptions options)
    : runtime_(
          std::make_unique<WebRtcRuntime>(
              options.videoEncoderPreference,
              options.videoDecoderPreference,
              options.preferredHardwareDecoderName,
              options.hardwareFingerprint,
              options.encoderCapabilityCache,
              NormalizeMediaDeviceId(
                  options.preferredMicrophoneDeviceId),
              NormalizeMediaDeviceId(
                  options.preferredSpeakerDeviceId),
              options.ffmpegX264Preset,
              options.ffmpegHardwareBackend,
              options.preferredAutomaticEncoderId))
    , signaling_(std::move(signaling))
    , signalingConfig_(std::move(signalingConfig))
    , options_(options)
{
    snapshot_.localVerificationCode =
        signalingConfig_.deviceVerificationCode;
}

InProcessSessionEngine::~InProcessSessionEngine()
{
    Stop();
}

SessionCommandResult InProcessSessionEngine::UpdateSignalingAccessToken(
    std::string accessToken)
{
    if (!signaling_) {
        return Failure("signaling_not_configured",
                       "Signaling is not configured for this session engine.");
    }
    const auto result = signaling_->UpdateAccessToken(accessToken);
    if (!result.accepted) {
        return Failure(result.errorCode, result.errorMessage);
    }
    signalingConfig_.accessToken = std::move(accessToken);
    return Success();
}

SessionCommandResult InProcessSessionEngine::RequestAccountDeletion()
{
    if (!signaling_) {
        return Failure("signaling_not_configured",
                       "Signaling is not configured for this session engine.");
    }
    const auto result = signaling_->RequestAccountDeletion();
    return result.accepted
        ? Success()
        : Failure(result.errorCode, result.errorMessage);
}

void InProcessSessionEngine::SetAccountDeletionResultCallback(
    std::function<void(const SignalingAccountDeletionResult&)> callback)
{
    std::lock_guard lock(mutex_);
    accountDeletionResultCallback_ = std::move(callback);
}

void InProcessSessionEngine::SetObserver(ISessionEngineObserver* observer)
{
    SessionEngineSnapshot snapshot;
    {
        std::lock_guard lock(mutex_);
        observer_ = observer;
        snapshot = snapshot_;
    }
    if (observer) {
        observer->OnSessionEngineSnapshot(snapshot);
    }
}

SessionCommandResult InProcessSessionEngine::Start()
{
    const auto beginResult = BeginStart();
    if (!beginResult.accepted) {
        return beginResult;
    }
    return CompleteStart(InitializeRuntimeForStart());
}

SessionCommandResult InProcessSessionEngine::BeginStart()
{
    {
        std::lock_guard lock(mutex_);
        if (snapshot_.state == SessionEngineState::kReady) {
            return Success();
        }
        if (snapshot_.state != SessionEngineState::kStopped &&
            snapshot_.state != SessionEngineState::kFailed) {
            return Failure("engine_start_in_invalid_state",
                           "The session engine cannot start in its current state.");
        }
        snapshot_ = {};
        // Start() rebuilds the public snapshot, but the one-time assistance
        // code belongs to the engine configuration and must remain visible to
        // the local UI. The signaling/authorization path already retained the
        // value; restore the corresponding snapshot field as well.
        snapshot_.localVerificationCode =
            signalingConfig_.deviceVerificationCode;
        snapshot_.localMediaDevices.camera.preferredDeviceId =
            NormalizeMediaDeviceId(
                options_.preferredCameraDeviceId);
        snapshot_.localMediaDevices.microphone.preferredDeviceId =
            NormalizeMediaDeviceId(
                options_.preferredMicrophoneDeviceId);
        snapshot_.localMediaDevices.speaker.preferredDeviceId =
            NormalizeMediaDeviceId(
                options_.preferredSpeakerDeviceId);
        snapshot_.state = SessionEngineState::kStarting;
        snapshot_.connectivity = signaling_
                                     ? SessionConnectivityState::kOffline
                                     : SessionConnectivityState::kNotConfigured;
        capabilities_ = {};
    }
    PublishSnapshot();
    return Success();
}

SessionCommandResult InProcessSessionEngine::InitializeRuntimeForStart()
{
    const DisplayTopologySnapshot displayTopology =
        EnumerateWindowsDisplayTopology();
    {
        std::lock_guard lock(mutex_);
        snapshot_.localDisplayTopology = displayTopology;
        if (const auto* primary = FindPrimaryDisplay(displayTopology)) {
            snapshot_.selectedLocalDisplayKey =
                primary->stableDisplayKey;
        }
    }
    PublishSnapshot();

    const bool ready = runtime_->Initialize();
    const auto& runtimeReport = runtime_->CapabilityReport();
    {
        std::lock_guard lock(mutex_);
        capabilities_.webRtcReady = ready;
        capabilities_.hasH264Encoder = runtimeReport.hasH264Encoder;
        capabilities_.hasH264Decoder = runtimeReport.hasH264Decoder;
        capabilities_.h264HardwareEncoderAvailable =
            runtimeReport.h264HardwareEncoderAvailable;
        capabilities_.h264HardwareEncoderCpuNv12InputSupported =
            runtimeReport.h264HardwareEncoderCpuNv12InputSupported;
        capabilities_.h264HardwareEncoderD3D11InputCandidate =
            runtimeReport.h264HardwareEncoderD3D11InputCandidate;
        capabilities_.h264HardwareEncoderCount =
            runtimeReport.h264HardwareEncoderCount;
        capabilities_.h264HardwareEncoderWired =
            runtimeReport.h264HardwareEncoderWired;
        capabilities_.h264SoftwareEncoderWired =
            runtimeReport.h264SoftwareEncoderWired;
        capabilities_.ffmpegX264EncoderWired =
            runtimeReport.h264FfmpegX264EncoderWired;
        capabilities_.ffmpegX264EncoderError =
            runtimeReport.h264FfmpegX264EncoderError;
        capabilities_.ffmpegHardwareEncoderAvailable =
            runtimeReport.h264FfmpegHardwareEncoderAvailable;
        capabilities_.ffmpegHardwareEncoderWired =
            runtimeReport.h264FfmpegHardwareEncoderWired;
        capabilities_.ffmpegHardwareEncoderError =
            runtimeReport.h264FfmpegHardwareEncoderError;
        capabilities_.ffmpegHardwareEncoderDescriptions =
            runtimeReport.h264FfmpegHardwareEncoderDescriptions;
        capabilities_.h264SoftwareEncoderFallback =
            runtimeReport.h264SoftwareEncoderFallbackWired;
        capabilities_.desktopCapturePreference =
            DesktopCaptureImplementationName(
                options_.desktopCaptureImplementation);
        capabilities_.videoEncoderPreference =
            EncoderPreferenceName(
                runtimeReport.videoEncoderPreference);
        capabilities_.videoDecoderPreference =
            DecoderPreferenceName(
                runtimeReport.videoDecoderPreference);
        capabilities_.h264HardwareEncoderDescriptions =
            runtimeReport.h264HardwareEncoderDescriptions;
        capabilities_.h264HardwareEncoderWarnings =
            runtimeReport.h264HardwareEncoderWarnings;
        capabilities_.hardwareFingerprint =
            runtimeReport.hardwareFingerprint;
        capabilities_.h264HardwareEncoderProbeSucceeded =
            runtimeReport.h264HardwareEncoderProbeSucceeded;
        capabilities_.h264HardwareEncoderProbeFromCache =
            runtimeReport.h264HardwareEncoderProbeFromCache;
        capabilities_.audioDeviceModuleCreated =
            runtimeReport.audioDeviceModuleCreated;
        capabilities_.audioDeviceError =
            runtimeReport.audioDeviceError;
        capabilities_.mfD3D11DecoderConfigured =
            runtimeReport.h264MfDecoderConfigured;
        capabilities_.mfD3D11DecoderHardware =
            runtimeReport.h264MfDecoderHardware;
        capabilities_.mfD3D11DecoderSoftware =
            runtimeReport.h264MfDecoderSoftware;
        capabilities_.d3d11NativeDecoderOutput =
            runtimeReport.h264MfDecoderNativeOutputSupported;
        capabilities_.mfD3D11DecoderAsynchronous =
            runtimeReport.h264MfDecoderAsynchronous;
        capabilities_.ffmpegSoftwareH264Decoder =
            runtimeReport.h264FfmpegSoftwareDecoderWired;
        capabilities_.mfD3D11DecoderName =
            runtimeReport.h264MfDecoderName;
        capabilities_.mfD3D11DecoderError =
            runtimeReport.h264MfDecoderError;
        capabilities_.operatingSystemDescription =
            options_.operatingSystemDescription;
        capabilities_.nativeArchitecture =
            options_.nativeArchitecture;
        capabilities_.remoteSession =
            options_.remoteSession;
        capabilities_.graphicsAdapterDescriptions =
            options_.graphicsAdapterDescriptions;
        capabilities_.graphicsEnumerationError =
            options_.graphicsEnumerationError;
        capabilities_.error = runtimeReport.error;
        snapshot_.state = ready ? SessionEngineState::kReady
                                : SessionEngineState::kFailed;
        if (!ready) {
            snapshot_.errorCode = "webrtc_runtime_initialization_failed";
            snapshot_.errorMessage = runtimeReport.error;
        }
    }
    PublishSnapshot();

    if (!ready) {
        return Failure("webrtc_runtime_initialization_failed",
                       runtimeReport.error);
    }

    (void)RefreshLocalMediaDevices();

    const auto factory = runtime_->PeerConnectionFactory();
    auto idleScreenTrack = factory->CreateVideoTrack(
        webrtc::make_ref_counted<IdleRoomVideoSource>(),
        "screen-main-idle");
    auto idleCameraTrack = factory->CreateVideoTrack(
        webrtc::make_ref_counted<IdleRoomVideoSource>(),
        "camera-main-idle");
    if (!idleScreenTrack || !idleCameraTrack) {
        {
            std::lock_guard lock(mutex_);
            snapshot_.state = SessionEngineState::kFailed;
            snapshot_.errorCode = "idle_video_track_create_failed";
            snapshot_.errorMessage =
                "The pre-negotiated video placeholders could not be created.";
        }
        PublishSnapshot();
        return Failure("idle_video_track_create_failed",
                       "The pre-negotiated video placeholders could not be created.");
    }
    {
        std::lock_guard lock(mutex_);
        idleRoomVideoTracks_[kScreenMainVideoSlot] =
            std::move(idleScreenTrack);
        idleRoomVideoTracks_[kCameraMainVideoSlot] =
            std::move(idleCameraTrack);
    }
    StartStatsPolling();
    return Success();
}

SessionCommandResult InProcessSessionEngine::CompleteStart(
    const SessionCommandResult& runtimeResult)
{

    const auto connectSignaling = [this] {
        if (!signaling_) {
            return;
        }

        signalingConfig_.capabilities.h264Encode =
            capabilities_.hasH264Encoder;
        signalingConfig_.capabilities.h264Decode =
            capabilities_.hasH264Decoder;
        signalingConfig_.capabilities.d3d11NativeDecode =
            capabilities_.d3d11NativeDecoderOutput;
        signaling_->SetObserver(this);
        const auto signalingResult = signaling_->Connect(signalingConfig_);
        if (!signalingResult.accepted) {
            {
                std::lock_guard lock(mutex_);
                snapshot_.connectivity =
                    SessionConnectivityState::kFailed;
                // Keep a media-runtime failure as the primary session error.
                // The signaling failure is still represented by connectivity.
                if (snapshot_.errorMessage.empty()) {
                    snapshot_.errorCode = signalingResult.errorCode;
                    snapshot_.errorMessage = signalingResult.errorMessage;
                }
            }
            PublishSnapshot();
        }
    };

    if (!runtimeResult.accepted) {
        // Signaling is a control-plane service and must remain independently
        // observable even when an explicit hardware-only media preference
        // cannot be satisfied on this machine.  Keep the runtime failed so
        // room/media operations stay unavailable, but still authenticate and
        // register the device for accurate WSS status and diagnostics.
        connectSignaling();
        return runtimeResult;
    }

    if (!signaling_) {
        return Success();
    }

    connectSignaling();

    // Runtime readiness and signaling connectivity are deliberately separate.
    // A temporary WSS failure must not make local WebRTC teardown unsafe.
    return Success();
}

void InProcessSessionEngine::Stop()
{
    StopRemoteCursorPublishing();
    StopStatsPolling();
    std::unique_ptr<SessionControllerBase> controller;
    std::unique_ptr<LibWebRtcSession> session;
    std::vector<std::shared_ptr<RoomPairRuntime>> roomPairs;
    std::vector<std::jthread> retiredRoomPairThreads;
    std::vector<std::jthread> retiredDesktopStopThreads;
    std::vector<std::jthread> retiredCameraStopThreads;
    std::vector<std::jthread> mediaDeviceOperationThreads;
    std::vector<std::jthread> clipboardWarmupThreads;
    std::string directSessionIdToEnd;
    bool directSessionWasActive = false;
    bool canNotifyDirectSessionEnd = false;
    std::string roomIdToLeave;
    bool canNotifyRoomLeave = false;
    std::string roomScreenShareGrantIdToStop;
    bool canNotifyRoomScreenShareStop = false;
    webrtc::scoped_refptr<WindowsDesktopCaptureSource> desktopCapture;
    webrtc::scoped_refptr<WindowsCameraCaptureSource> cameraCapture;
    webrtc::VideoSinkInterface<webrtc::VideoFrame>* cameraPreviewSink = nullptr;
    webrtc::scoped_refptr<webrtc::VideoTrackInterface> cameraPreviewTrack;
    {
        std::lock_guard lock(mutex_);
        if (snapshot_.state == SessionEngineState::kStopped) {
            return;
        }
        const bool signalingOnline = signaling_ &&
            snapshot_.connectivity == SessionConnectivityState::kOnline;
        if (!sessionEndSignalSent_ && !snapshot_.sessionId.empty() &&
            snapshot_.state != SessionEngineState::kReady &&
            snapshot_.state != SessionEngineState::kStopping) {
            directSessionIdToEnd = snapshot_.sessionId;
            directSessionWasActive = serverSessionActive_;
            canNotifyDirectSessionEnd = signalingOnline;
        }
        snapshot_.state = SessionEngineState::kStopping;
        ++localCameraTrackGeneration_;
        controller = std::move(sessionController_);
        session = std::move(webRtcSession_);
        roomPairs.reserve(roomPairs_.size());
        for (auto& [pairId, pair] : roomPairs_) {
            (void)pairId;
            roomPairs.push_back(std::move(pair));
        }
        roomPairs_.clear();
        retiredRoomPairThreads = std::move(retiredRoomPairThreads_);
        retiredDesktopStopThreads =
            std::move(retiredDesktopStopThreads_);
        retiredCameraStopThreads = std::move(retiredCameraStopThreads_);
        mediaDeviceOperationThreads =
            std::move(mediaDeviceOperationThreads_);
        for (const auto& pair : roomPairs) {
            if (pair) {
                pair->clipboardWarmupCancelled = true;
            }
        }
        clipboardWarmupThreads =
            std::move(clipboardWarmupThreads_);
        desktopCapture = std::exchange(
            localDesktopCaptureSource_, nullptr);
        cameraCapture = std::exchange(
            localCameraCaptureSource_, nullptr);
        cameraPreviewSink = localCameraPreviewSink_;
        cameraPreviewTrack = std::exchange(
            localCameraPreviewTrack_, nullptr);
        if (localMicrophoneAudioTrack_) {
            localMicrophoneAudioTrack_->set_enabled(false);
        }
        localMicrophoneAudioTrack_ = nullptr;
        localMicrophoneAudioSource_ = nullptr;
        snapshot_.roomPeerConnections.clear();
        pendingRemoteDescription_.reset();
        pendingRemoteCandidates_.clear();
        sessionEndSignalSent_ = false;
        cancelWhenSessionIdKnown_ = false;
        serverSessionActive_ = false;
        snapshot_.directSignalingSessionReady = false;
        signalingRecoveryPending_ = false;
        peerSignalingSuspended_ = false;
        sessionRecoveryToken_.clear();
        if (!snapshot_.room.roomId.empty() &&
            (snapshot_.room.membership == RoomMembershipState::kActive ||
             snapshot_.room.membership == RoomMembershipState::kJoinPending ||
             snapshot_.room.membership == RoomMembershipState::kLeaving)) {
            roomIdToLeave = snapshot_.room.roomId;
            canNotifyRoomLeave = signalingOnline;
            const bool localOwnsScreenShare =
                snapshot_.room.screenSharerDeviceId ==
                    snapshot_.localDeviceId ||
                snapshot_.room.pendingScreenSharerDeviceId ==
                    snapshot_.localDeviceId;
            if (localOwnsScreenShare &&
                !roomScreenShareGrantId_.empty()) {
                roomScreenShareGrantIdToStop =
                    roomScreenShareGrantId_;
                canNotifyRoomScreenShareStop = signalingOnline;
            }
        }
    }
    PublishSnapshot();
    // Send ownership/session releases while the registered WSS connection is
    // still alive. The later socket close remains the server-side fallback,
    // but a normal application exit should never depend on lease expiry.
    if (signaling_) {
        if (canNotifyDirectSessionEnd) {
            if (directSessionWasActive) {
                (void)signaling_->CloseSession(
                    directSessionIdToEnd, "application_stopping");
            } else {
                (void)signaling_->CancelSession(
                    directSessionIdToEnd, "application_stopping");
            }
        }
        if (canNotifyRoomScreenShareStop) {
            (void)signaling_->StopRoomScreenShare(
                roomIdToLeave, roomScreenShareGrantIdToStop,
                "application_stopping");
        }
        if (canNotifyRoomLeave) {
            (void)signaling_->LeaveRoom(
                roomIdToLeave, "application_stopping");
        }
    }
    if (desktopCapture) {
        desktopCapture->StopCapture();
    }
    if (cameraPreviewTrack && cameraPreviewSink) {
        cameraPreviewTrack->RemoveSink(cameraPreviewSink);
    }
    if (cameraCapture) {
        cameraCapture->StopCapture();
    }
    if (controller) {
        controller->SetObserver(nullptr);
        controller.reset();
    }
    session.reset();
    for (auto& thread : clipboardWarmupThreads) {
        thread.request_stop();
    }
    // Join warmup workers before destroying their pair/controller objects.
    clipboardWarmupThreads.clear();
    for (auto& pair : roomPairs) {
        if (pair->controller) {
            pair->controller->SetObserver(nullptr);
            pair->controller.reset();
        }
        pair->session.reset();
        pair->bridge.reset();
    }
    roomPairs.clear();
    // Retired Pair teardown is performed away from the Qt event thread, but
    // must finish before the shared WebRTC runtime is shut down.
    retiredRoomPairThreads.clear();
    retiredDesktopStopThreads.clear();
    retiredCameraStopThreads.clear();
    mediaDeviceOperationThreads.clear();
    {
        std::lock_guard lock(mutex_);
        localRoomVideoTracks_.clear();
        idleRoomVideoTracks_.clear();
    }
    if (signaling_) {
        signaling_->SetObserver(nullptr);
        signaling_->Disconnect();
    }
    runtime_->Shutdown();
    {
        std::lock_guard lock(mutex_);
        snapshot_ = {};
        capabilities_.webRtcReady = false;
    }
    PublishSnapshot();
}

SessionEngineSnapshot InProcessSessionEngine::Snapshot() const
{
    std::lock_guard lock(mutex_);
    return snapshot_;
}

SessionEngineCapabilities InProcessSessionEngine::Capabilities() const
{
    SessionEngineCapabilities result;
    {
        std::lock_guard lock(mutex_);
        result = capabilities_;
    }
    if (runtime_) {
        const auto runtimeStatus = runtime_->EncoderRuntimeStatus();
        result.videoEncoderRuntimeDetails.clear();
        result.videoEncoderRuntimeDetails.reserve(
            runtimeStatus.instances.size());
        for (const auto& instance : runtimeStatus.instances) {
            result.videoEncoderRuntimeDetails.push_back(
                DescribeEncoderRuntimeInstance(instance));
        }
        result.videoEncoderLastFallbackReason =
            runtimeStatus.lastFallbackReason;
    }
    return result;
}

SessionDiagnosticsSnapshot InProcessSessionEngine::Diagnostics() const
{
    SessionDiagnosticsSnapshot diagnostics;
    diagnostics.remoteInput =
        RemoteInputTelemetry::Instance().TakeSnapshot();
    if (runtime_) {
        diagnostics.videoEncoderLastFallbackReason =
            runtime_->EncoderRuntimeStatus().lastFallbackReason;
    }
    std::vector<std::shared_ptr<RoomPairRuntime>> roomPairs;
    webrtc::scoped_refptr<WindowsDesktopCaptureSource>
        desktopCaptureSource;
    {
        std::lock_guard lock(mutex_);
        diagnostics.remoteCursor.publishing = cursorMonitor_.running();
        diagnostics.remoteCursor.shapeAvailable =
            latestLocalCursorShape_.has_value();
        if (latestLocalCursorPosition_) {
            diagnostics.remoteCursor.displayId =
                latestLocalCursorPosition_->displayId;
            diagnostics.remoteCursor.displayLayoutVersion =
                latestLocalCursorPosition_->displayLayoutVersion;
            diagnostics.remoteCursor.lastAppliedInputSequence =
                latestLocalCursorPosition_->lastAppliedInputSequence;
        }
        if (latestLocalCursorShape_) {
            diagnostics.remoteCursor.shapeId =
                latestLocalCursorShape_->shapeId;
        }
        diagnostics.remoteCursor.positionMessagesPublished =
            cursorPositionsPublished_;
        diagnostics.remoteCursor.shapeMessagesPublished =
            cursorShapesPublished_;
        diagnostics.remoteCursor.positionMessagesReceived =
            cursorPositionsReceived_;
        diagnostics.remoteCursor.shapeMessagesReceived =
            cursorShapesReceived_;
        desktopCaptureSource = localDesktopCaptureSource_;
        if (webRtcSession_) {
            PeerConnectionDiagnosticsSnapshot direct;
            direct.peerDeviceId = snapshot_.peerDeviceId;
            direct.stats = webRtcSession_->StatsSnapshot();
            diagnostics.collectedAtMs = (std::max)(
                diagnostics.collectedAtMs,
                direct.stats.transport.timestampMs);
            diagnostics.peerConnections.push_back(std::move(direct));
        }
        roomPairs.reserve(roomPairs_.size());
        for (const auto& [pairId, pair] : roomPairs_) {
            (void)pairId;
            if (pair && pair->session) {
                roomPairs.push_back(pair);
            }
        }
    }

    const auto captureStats = desktopCaptureSource
        ? std::make_optional(
              desktopCaptureSource->CaptureRuntimeStats())
        : std::nullopt;
    const auto annotateDesktopCapture =
        [&captureStats, &desktopCaptureSource](
            WebRtcSessionStatsSnapshot& stats) {
            if (!captureStats) {
                return;
            }
            for (auto& stream : stats.rtpStreams) {
                if (stream.direction !=
                        RtpStreamDirection::kOutbound ||
                    stream.kind != "video" ||
                    stream.slot != kScreenMainVideoSlot) {
                    continue;
                }
                stream.captureTargetFrameRate =
                    captureStats->targetFrameRate;
                stream.captureConfiguredBackend =
                    DesktopCaptureImplementationName(
                        desktopCaptureSource->ConfiguredImplementation());
                stream.captureActiveBackend =
                    DesktopCaptureBackendName(
                        desktopCaptureSource->Backend());
                stream.captureFallbackReason =
                    desktopCaptureSource->FallbackReason();
                switch (captureStats->activityState) {
                case WindowsDesktopCaptureSource::CaptureActivityState::
                    kStarting:
                    stream.captureActivityState = "starting";
                    break;
                case WindowsDesktopCaptureSource::CaptureActivityState::
                    kActive:
                    stream.captureActivityState = "active";
                    break;
                case WindowsDesktopCaptureSource::CaptureActivityState::kIdle:
                    stream.captureActivityState = "idle";
                    break;
                }
                stream.captureAdaptiveFrameDeliveryEnabled =
                    captureStats->adaptiveFrameDeliveryEnabled;
                stream.captureAttemptsPerSecond =
                    captureStats->captureAttemptsPerSecond;
                stream.captureDeliveredFramesPerSecond =
                    captureStats->deliveredFramesPerSecond;
                stream.captureChangedFramesPerSecond =
                    captureStats->changedFramesPerSecond;
                stream.captureIdleHeartbeatFramesPerSecond =
                    captureStats->idleHeartbeatFramesPerSecond;
                stream.captureAttempts =
                    captureStats->totalCaptureAttempts;
                stream.captureDeliveredFrames =
                    captureStats->totalDeliveredFrames;
                stream.captureChangedFrames =
                    captureStats->totalChangedFrames;
                stream.captureIdleHeartbeatFrames =
                    captureStats->totalIdleHeartbeatFrames;
                stream.captureSuppressedUnchangedFrames =
                    captureStats->totalSuppressedUnchangedFrames;
                stream.captureActivityTransitions =
                    captureStats->totalActivityTransitions;
                stream.captureFailures =
                    captureStats->totalFailedCaptures;
                stream.captureInputBoostActive =
                    captureStats->inputBoostActive;
                stream.captureInputBoosts =
                    captureStats->totalInputBoosts;
                stream.captureForcedRefreshFrames =
                    captureStats->totalForcedRefreshFrames;
                stream.latestCaptureCallMs =
                    captureStats->latestCaptureCallMs;
                // RTCVideoSourceStats is optional and is absent for custom
                // native frame buffers in some libwebrtc builds. Every
                // delivered capture frame is passed directly to OnFrame, so
                // the application-side delivery rate is the exact source
                // output fallback for this track.
                if (stream.sourceWidth == 0 ||
                    stream.sourceHeight == 0) {
                    stream.sourceWidth =
                        desktopCaptureSource->CapturedWidth();
                    stream.sourceHeight =
                        desktopCaptureSource->CapturedHeight();
                }
                if (stream.sourceFramesPerSecond <= 0.0) {
                    stream.sourceFramesPerSecond =
                        captureStats->deliveredFramesPerSecond;
                }
            }
        };
    const auto annotatePresentation = [](
        const std::string& peerDeviceId,
        WebRtcSessionStatsSnapshot& stats) {
        const auto presentation =
            VideoPresentationTelemetryRegistry::Instance()
                .SnapshotForPeer(peerDeviceId);
        if (!presentation) {
            return;
        }
        for (auto& stream : stats.rtpStreams) {
            if (stream.direction != RtpStreamDirection::kInbound ||
                stream.kind != "video" ||
                stream.slot != kScreenMainVideoSlot) {
                continue;
            }
            stream.presentationTimingAvailable = presentation->available;
            switch (presentation->path) {
            case VideoPresentationPath::kCpuQt:
                stream.presentationPath = "CPU/Qt";
                break;
            case VideoPresentationPath::kCpuD3D11:
                stream.presentationPath = "CPU/D3D11";
                break;
            case VideoPresentationPath::kCpuNv12D3D11:
                stream.presentationPath = "CPU NV12/D3D11";
                break;
            case VideoPresentationPath::kCpuI420D3D11:
                stream.presentationPath = "CPU I420/D3D11";
                break;
            case VideoPresentationPath::kD3D11:
                stream.presentationPath = "D3D11";
                break;
            case VideoPresentationPath::kUnknown:
                stream.presentationPath = "Unknown";
                break;
            }
            stream.localRefreshRateHz = presentation->localRefreshRateHz;
            stream.presentationArrivalFramesPerSecond =
                presentation->arrivalFramesPerSecond;
            stream.presentedFramesPerSecond =
                presentation->presentedFramesPerSecond;
            stream.presentationArrivedFrames = presentation->arrivedFrames;
            stream.presentedFrames = presentation->presentedFrames;
            stream.presentationSupersededFrames =
                presentation->supersededFrames;
            stream.presentationConvertedFrames =
                presentation->conversionFrames;
            stream.presentationFailures = presentation->presentFailures;
            stream.latestPresentationConversionMs =
                presentation->latestConversionMs;
            stream.averagePresentationConversionMs =
                presentation->averageConversionMs;
            stream.latestPresentationRenderSubmitMs =
                presentation->latestRenderSubmitMs;
            stream.averagePresentationRenderSubmitMs =
                presentation->averageRenderSubmitMs;
            stream.latestPresentCallMs = presentation->latestPresentCallMs;
            stream.averagePresentCallMs = presentation->averagePresentCallMs;
            stream.averagePresentedIntervalMs =
                presentation->averagePresentedIntervalMs;
            stream.p95PresentedIntervalMs =
                presentation->p95PresentedIntervalMs;
            stream.maximumPresentedIntervalMs =
                presentation->maximumPresentedIntervalMs;
            stream.latestReceiverPipelineMs =
                presentation->latestReceiverPipelineMs;
            stream.averageReceiverPipelineMs =
                presentation->averageReceiverPipelineMs;
            stream.p95ReceiverPipelineMs =
                presentation->p95ReceiverPipelineMs;
            stream.maximumReceiverPipelineMs =
                presentation->maximumReceiverPipelineMs;
        }
    };
    for (auto& connection : diagnostics.peerConnections) {
        annotateDesktopCapture(connection.stats);
        annotatePresentation(
            connection.peerDeviceId, connection.stats);
    }

    // RoomPairRuntime is shared, so its lifetime remains valid after the
    // engine lock is released. A diagnostics refresh must never prevent room
    // negotiation or a UI snapshot from acquiring the engine mutex.
    diagnostics.peerConnections.reserve(
        diagnostics.peerConnections.size() + roomPairs.size());
    for (const auto& pair : roomPairs) {
        if (!pair || !pair->session) {
            continue;
        }
        PeerConnectionDiagnosticsSnapshot current;
        current.pairId = pair->pairId;
        current.peerDeviceId = pair->peerDeviceId;
        current.stats = pair->session->StatsSnapshot();
        annotateDesktopCapture(current.stats);
        annotatePresentation(current.peerDeviceId, current.stats);
        diagnostics.collectedAtMs = (std::max)(
            diagnostics.collectedAtMs,
            current.stats.transport.timestampMs);
        diagnostics.peerConnections.push_back(std::move(current));
    }
    return diagnostics;
}

void InProcessSessionEngine::StartStatsPolling()
{
    StopStatsPolling();
    statsPollingThread_ = std::jthread(
        [this](std::stop_token stopToken) {
            while (!stopToken.stop_requested()) {
                std::vector<std::shared_ptr<RoomPairRuntime>> roomPairs;
                ScreenContentActivity screenActivity =
                    ScreenContentActivity::kUnknown;
                {
                    std::lock_guard lock(mutex_);
                    if (localDesktopCaptureSource_) {
                        switch (localDesktopCaptureSource_
                                    ->CaptureRuntimeStats().activityState) {
                        case WindowsDesktopCaptureSource::
                            CaptureActivityState::kStarting:
                            screenActivity =
                                ScreenContentActivity::kStarting;
                            break;
                        case WindowsDesktopCaptureSource::
                            CaptureActivityState::kActive:
                            screenActivity =
                                ScreenContentActivity::kActive;
                            break;
                        case WindowsDesktopCaptureSource::
                            CaptureActivityState::kIdle:
                            screenActivity = ScreenContentActivity::kIdle;
                            break;
                        }
                    }
                    // The legacy direct session is uniquely owned and can be
                    // disposed independently of room pairs. Its GetStats call
                    // only posts an asynchronous request, so keep its lifetime
                    // protected here. Room sessions below are shared and can
                    // safely be invoked after releasing the engine mutex.
                    if (webRtcSession_) {
                        webRtcSession_->SetScreenContentActivity(
                            screenActivity);
                        webRtcSession_->RequestStats();
                    }
                    roomPairs.reserve(roomPairs_.size());
                    for (const auto& [pairId, pair] : roomPairs_) {
                        (void)pairId;
                        if (pair && pair->session) {
                            roomPairs.push_back(pair);
                        }
                    }
                }
                // GetStats is an external WebRTC call. Never make it while
                // holding the engine mutex: a busy signaling thread would
                // otherwise prevent the Qt thread from even reading a
                // snapshot and Windows would report the app as hung.
                for (const auto& pair : roomPairs) {
                    pair->session->SetScreenContentActivity(screenActivity);
                    pair->session->RequestStats();
                }
                std::unique_lock waitLock(statsPollingWaitMutex_);
                statsPollingCondition_.wait_for(
                    waitLock, stopToken,
                    std::chrono::seconds(1), [] { return false; });
            }
        });
}

void InProcessSessionEngine::StopStatsPolling()
{
    if (!statsPollingThread_.joinable()) {
        return;
    }
    statsPollingThread_.request_stop();
    statsPollingCondition_.notify_all();
    statsPollingThread_.join();
}
