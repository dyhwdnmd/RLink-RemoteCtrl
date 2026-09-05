// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

void LibWebRtcSession::RequestStats()
{
    if (statsCollector_) {
        const auto gate = callbackGate_;
        statsCollector_->Request(
            PeerConnection(),
            [gate] {
                const auto lease = gate ? gate->Enter()
                                        : CallbackGate::Lease{};
                if (auto* owner = lease.Owner()) {
                    owner->HandleCompletedStatsSample();
                }
            });
    }
}

void LibWebRtcSession::HandleCompletedStatsSample()
{
    const auto snapshot = StatsSnapshot();
    if (!snapshot.transport.collected) {
        return;
    }

    ProgressiveBitrateCeilingSample sample;
    sample.timestampMs = SteadyNowMs();
    sample.availableOutgoingBitrateBps =
        snapshot.transport.availableOutgoingBitrateBps;
    sample.roundTripTimeMs =
        snapshot.transport.currentRoundTripTimeMs;
    sample.packetsDiscardedOnSend =
        snapshot.transport.packetsDiscardedOnSend;
    bool hasActiveScreenSender = false;
    for (const auto& stream : snapshot.rtpStreams) {
        if (stream.direction != RtpStreamDirection::kOutbound ||
            stream.kind != "video" ||
            stream.slot != kScreenMainVideoSlot) {
            continue;
        }
        hasActiveScreenSender = true;
        sample.targetBitrateBps = (std::max)(
            sample.targetBitrateBps, stream.targetBitrateBps);
        sample.lossPercent = (std::max)(
            sample.lossPercent, stream.lossPercent);
    }
    if (!hasActiveScreenSender) {
        return;
    }

    ProgressiveBitrateCeilingDecision decision;
    ProgressiveBitrateCeilingState previousState;
    std::uint64_t decisionRevision = 0;
    AdaptiveScreenFrameRateDecision frameRateDecision;
    AdaptiveScreenFrameRateState previousFrameRateState;
    std::uint64_t frameRateDecisionRevision = 0;
    webrtc::scoped_refptr<webrtc::RtpTransceiverInterface>
        screenTransceiver;
    {
        std::lock_guard lock(mutex_);
        const auto screen = videoSlots_.find(kScreenMainVideoSlot);
        if (screen == videoSlots_.end() ||
            !screen->second.sendingActive) {
            return;
        }
        if (fastDesktopBweStartup_ &&
            progressiveBitrateCeiling_.enabled) {
            previousState = progressiveBitrateCeiling_;
            decision = EvaluateProgressiveBitrateCeiling(
                &progressiveBitrateCeiling_, sample);
            if (decision.applyPeerConnectionMax) {
                decisionRevision = ++progressiveBitrateCeilingRevision_;
            }
        }
        if (adaptiveDesktopNetworkFrameRate_ &&
            screen->second.adaptiveFrameRate.enabled) {
            previousFrameRateState = screen->second.adaptiveFrameRate;
            frameRateDecision = EvaluateAdaptiveScreenFrameRate(
                &screen->second.adaptiveFrameRate,
                {.timestampMs = sample.timestampMs,
                 .activity = screenContentActivity_,
                 .capacityBps =
                     sample.availableOutgoingBitrateBps != 0
                     ? sample.availableOutgoingBitrateBps
                     : sample.targetBitrateBps});
            if (frameRateDecision.applyEffectiveFrameRate) {
                frameRateDecisionRevision =
                    ++screen->second.adaptiveFrameRateRevision;
                screenTransceiver = screen->second.transceiver;
            }
        }
    }
    (void)ApplyProgressiveBitrateCeilingDecision(
        decision, decisionRevision, previousState);
    (void)ApplyAdaptiveScreenFrameRateDecision(
        frameRateDecision,
        frameRateDecisionRevision,
        previousFrameRateState,
        std::move(screenTransceiver));
}

webrtc::RTCError LibWebRtcSession::ApplyProgressiveBitrateCeilingDecision(
    const ProgressiveBitrateCeilingDecision& decision,
    std::uint64_t decisionRevision,
    const ProgressiveBitrateCeilingState& previousState)
{
    if (!decision.applyPeerConnectionMax ||
        decision.peerConnectionMaxBitrateBps == 0) {
        return webrtc::RTCError::OK();
    }

    bool keepStartupProbeFloor = false;
    {
        std::lock_guard lock(mutex_);
        for (const auto& [slot, binding] : videoSlots_) {
            (void)slot;
            keepStartupProbeFloor = keepStartupProbeFloor ||
                (binding.sendingActive &&
                 binding.bitrateProbeFloorActive);
        }
    }

    auto peer = PeerConnection();
    webrtc::RTCError result = webrtc::RTCError::InvalidState(
        "PeerConnection is not ready for the progressive bitrate update.");
    if (peer) {
        webrtc::BitrateSettings settings;
        settings.min_bitrate_bps = keepStartupProbeFloor
            ? kDesktopStartupProbeFloorBps
            : kDefaultWebRtcMinimumBitrateBps;
        settings.max_bitrate_bps = static_cast<int>((std::min)(
            decision.peerConnectionMaxBitrateBps,
            static_cast<std::uint64_t>(kMaximumScreenBitrateBps)));
        result = peer->SetBitrate(settings);
    }

    std::lock_guard lock(mutex_);
    if (progressiveBitrateCeilingRevision_ != decisionRevision) {
        return result;
    }
    if (result.ok()) {
        progressiveBitrateCeilingError_.clear();
        return result;
    }
    progressiveBitrateCeiling_ = previousState;
    ++progressiveBitrateCeilingRevision_;
    progressiveBitrateCeilingError_ = std::string(result.message());
    return result;
}

webrtc::RTCError LibWebRtcSession::ApplyAdaptiveScreenFrameRateDecision(
    const AdaptiveScreenFrameRateDecision& decision,
    std::uint64_t decisionRevision,
    const AdaptiveScreenFrameRateState& previousState,
    webrtc::scoped_refptr<webrtc::RtpTransceiverInterface> transceiver)
{
    if (!decision.applyEffectiveFrameRate ||
        decision.effectiveFrameRate == 0 || !transceiver) {
        return webrtc::RTCError::OK();
    }
    std::unique_lock senderParametersLock(videoSenderParametersMutex_);
    {
        std::lock_guard lock(mutex_);
        const auto found = videoSlots_.find(kScreenMainVideoSlot);
        if (found == videoSlots_.end() ||
            found->second.transceiver != transceiver ||
            found->second.adaptiveFrameRateRevision != decisionRevision) {
            return webrtc::RTCError::OK();
        }
    }
    auto sender = transceiver->sender();
    webrtc::RTCError result = webrtc::RTCError::InvalidState(
        "The screen sender is unavailable for network FPS adaptation.");
    if (sender) {
        auto parameters = sender->GetParameters();
        if (parameters.encodings.empty()) {
            result = webrtc::RTCError::InvalidState(
                "The screen sender has no encoding for network FPS "
                "adaptation.");
        } else {
            for (auto& encoding : parameters.encodings) {
                encoding.max_framerate =
                    static_cast<double>(decision.effectiveFrameRate);
            }
            result = sender->SetParameters(parameters);
        }
    }

    std::lock_guard lock(mutex_);
    const auto found = videoSlots_.find(kScreenMainVideoSlot);
    if (found == videoSlots_.end() ||
        found->second.transceiver != transceiver ||
        found->second.adaptiveFrameRateRevision != decisionRevision) {
        return result;
    }
    if (result.ok()) {
        found->second.adaptiveFrameRateError.clear();
        return result;
    }
    found->second.adaptiveFrameRate = previousState;
    ++found->second.adaptiveFrameRateRevision;
    found->second.adaptiveFrameRateError =
        std::string(result.message());
    return result;
}

WebRtcSessionStatsSnapshot LibWebRtcSession::StatsSnapshot() const
{
    if (!statsCollector_) {
        return {};
    }
    auto snapshot = statsCollector_->Snapshot();
    std::vector<std::string> videoSlots;
    std::string audioSlot;
    struct EncodingPolicy {
        std::uint32_t maxFrameRate = 0;
        std::uint32_t outputWidth = 0;
        std::uint32_t outputHeight = 0;
        std::uint64_t startBitrateBps = 0;
        std::uint64_t maxBitrateBps = 0;
        AdaptiveScreenFrameRateState adaptiveFrameRate;
        std::string adaptiveFrameRateError;
        std::uint32_t bootstrapAttempts = 0;
        std::uint32_t bootstrapSuccesses = 0;
        std::uint32_t mediaReadyRestarts = 0;
        std::uint32_t allocationProbePulses = 0;
        std::uint32_t probeFloorReleases = 0;
        bool probeFloorActive = false;
        std::string bootstrapError;
    };
    std::unordered_map<std::string, EncodingPolicy> policies;
    ProgressiveBitrateCeilingState progressiveCeiling;
    std::string progressiveCeilingError;
    {
        std::lock_guard lock(mutex_);
        videoSlots = videoSlotOrder_;
        audioSlot = audioSlot_.name;
        progressiveCeiling = progressiveBitrateCeiling_;
        progressiveCeilingError = progressiveBitrateCeilingError_;
        policies.reserve(videoSlots_.size());
        for (const auto& [slot, binding] : videoSlots_) {
            policies.emplace(
                slot,
                EncodingPolicy{
                    .maxFrameRate = binding.configuredMaxFrameRate,
                    .outputWidth = binding.configuredOutputWidth,
                    .outputHeight = binding.configuredOutputHeight,
                    .startBitrateBps =
                        binding.configuredStartBitrateBps,
                    .maxBitrateBps = binding.configuredMaxBitrateBps,
                    .adaptiveFrameRate = binding.adaptiveFrameRate,
                    .adaptiveFrameRateError =
                        binding.adaptiveFrameRateError,
                    .bootstrapAttempts =
                        binding.bitrateBootstrapAttempts,
                    .bootstrapSuccesses =
                        binding.bitrateBootstrapSuccesses,
                    .mediaReadyRestarts =
                        binding.mediaReadyBitrateRestarts,
                    .allocationProbePulses =
                        binding.allocationProbePulses,
                    .probeFloorReleases =
                        binding.bitrateProbeFloorReleases,
                    .probeFloorActive =
                        binding.bitrateProbeFloorActive,
                    .bootstrapError = binding.bitrateBootstrapError,
                });
        }
    }

    // StatsSnapshot is read synchronously by the Qt diagnostics UI. Never call
    // a libwebrtc proxy from here: methods such as RtpTransceiver::mid() marshal
    // to the signaling thread and can block the GUI indefinitely while that
    // thread is busy. The stats report already contains each RTP MID, so map
    // the negotiated video m-lines to their fixed slot order using plain data.
    std::vector<std::string> videoMids;
    for (const auto& stream : snapshot.rtpStreams) {
        if (stream.kind != "video" || stream.mid.empty() ||
            std::find(videoMids.begin(), videoMids.end(), stream.mid) !=
                videoMids.end()) {
            continue;
        }
        videoMids.push_back(stream.mid);
    }
    std::sort(videoMids.begin(), videoMids.end());

    std::unordered_map<std::string, std::string> slotsByMid;
    const std::size_t mappedVideoSlotCount =
        (std::min)(videoMids.size(), videoSlots.size());
    for (std::size_t index = 0; index < mappedVideoSlotCount; ++index) {
        slotsByMid.emplace(videoMids[index], videoSlots[index]);
    }
    for (auto& stream : snapshot.rtpStreams) {
        if (stream.kind == "audio") {
            stream.slot = audioSlot;
        } else {
            const auto found = slotsByMid.find(stream.mid);
            if (found != slotsByMid.end()) {
                stream.slot = found->second;
            } else if (stream.kind == "video" &&
                       videoSlots.size() == 1) {
                stream.slot = videoSlots.front();
            }
        }
        const auto policy = policies.find(stream.slot);
        if (policy != policies.end()) {
            stream.configuredMaxFrameRate = policy->second.maxFrameRate;
            stream.configuredOutputWidth = policy->second.outputWidth;
            stream.configuredOutputHeight = policy->second.outputHeight;
            stream.configuredStartBitrateBps =
                policy->second.startBitrateBps;
            stream.configuredMaxBitrateBps =
                policy->second.maxBitrateBps;
            stream.effectiveNetworkFrameRate =
                policy->second.adaptiveFrameRate.effectiveFrameRate;
            stream.adaptiveNetworkFrameRateEnabled =
                policy->second.adaptiveFrameRate.enabled;
            stream.adaptiveNetworkFrameRateReductionSamples =
                policy->second.adaptiveFrameRate.reductionSampleCount;
            stream.adaptiveNetworkFrameRateRecoverySamples =
                policy->second.adaptiveFrameRate.recoverySampleCount;
            stream.adaptiveNetworkFrameRateCapacityBps =
                policy->second.adaptiveFrameRate.smoothedCapacityBps;
            stream.adaptiveNetworkFrameRateStatus =
                AdaptiveScreenFrameRateStatusName(
                    policy->second.adaptiveFrameRate.status);
            stream.adaptiveNetworkFrameRateError =
                policy->second.adaptiveFrameRateError;
            stream.bitrateBootstrapAttempts =
                policy->second.bootstrapAttempts;
            stream.bitrateBootstrapSuccesses =
                policy->second.bootstrapSuccesses;
            stream.mediaReadyBitrateRestarts =
                policy->second.mediaReadyRestarts;
            stream.allocationProbePulses =
                policy->second.allocationProbePulses;
            stream.bitrateProbeFloorReleases =
                policy->second.probeFloorReleases;
            stream.bitrateProbeFloorActive =
                policy->second.probeFloorActive;
            stream.bitrateBootstrapError =
                policy->second.bootstrapError;
        }
        if (stream.slot == kScreenMainVideoSlot) {
            stream.progressiveBitrateCeilingEnabled =
                progressiveCeiling.enabled;
            stream.desiredPeerConnectionMaxBitrateBps =
                progressiveCeiling.desiredMaxBitrateBps;
            stream.appliedPeerConnectionMaxBitrateBps =
                progressiveCeiling.appliedMaxBitrateBps;
            stream.smoothedOutgoingCapacityBps =
                progressiveCeiling.smoothedCapacityBps;
            stream.bitrateCeilingStableSamples =
                progressiveCeiling.stableSampleCount;
            const auto nowMs = SteadyNowMs();
            const auto cooldownRemaining =
                progressiveCeiling.cooldownUntilMs > nowMs
                ? progressiveCeiling.cooldownUntilMs - nowMs
                : 0;
            stream.bitrateCeilingCooldownRemainingMs =
                static_cast<std::uint32_t>((std::min)(
                    cooldownRemaining,
                    static_cast<std::uint64_t>(
                        (std::numeric_limits<std::uint32_t>::max)())));
            stream.bitrateCeilingProbePending =
                progressiveCeiling.probePending;
            stream.bitrateCeilingLastProbeFailed =
                progressiveCeiling.lastProbeFailed;
            stream.bitrateCeilingStatus =
                ProgressiveBitrateCeilingStatusName(
                    progressiveCeiling.status);
            stream.bitrateCeilingError = progressiveCeilingError;
        }
    }
    return snapshot;
}

void LibWebRtcSession::Close()
{
    webrtc::scoped_refptr<webrtc::PeerConnectionInterface> peer;
    webrtc::scoped_refptr<webrtc::VideoTrackInterface> remoteTrack;
    webrtc::VideoSinkInterface<webrtc::VideoFrame>* remoteSink = nullptr;
    std::unordered_map<std::string, std::unique_ptr<DataChannelBinding>>
        channels;
    std::unordered_map<std::string, VideoSlotBinding> videoSlots;

    {
        std::lock_guard lock(mutex_);
        if (!peerConnection_ && state_ == WebRtcSessionState::kClosed) {
            return;
        }
    }
    ChangeState(WebRtcSessionState::kClosing);
    {
        std::lock_guard lock(mutex_);
        peer = std::move(peerConnection_);
        channels = std::move(dataChannels_);
        videoSlots = std::move(videoSlots_);
        videoSlotOrder_.clear();
        progressiveBitrateCeiling_ = {};
        ++progressiveBitrateCeilingRevision_;
        progressiveBitrateCeilingError_.clear();
        screenContentActivity_ = ScreenContentActivity::kUnknown;
        adaptiveDesktopNetworkFrameRate_ = false;
        remoteTrack = std::move(remoteVideoTrack_);
        remoteSink = remoteVideoSink_;
        remoteVideoSink_ = nullptr;
    }

    if (remoteTrack && remoteSink) {
        remoteTrack->RemoveSink(remoteSink);
    }
    for (auto& [slot, binding] : videoSlots) {
        (void)slot;
        if (binding.remoteTrack && binding.remoteSink) {
            binding.remoteTrack->RemoveSink(binding.remoteSink);
        }
    }
    for (auto& [label, binding] : channels) {
        binding->Close();
    }
    channels.clear();
    if (peer) {
        peer->Close();
    }
    ChangeState(WebRtcSessionState::kClosed);
}

webrtc::RTCErrorOr<
    webrtc::scoped_refptr<webrtc::RtpTransceiverInterface>>
LibWebRtcSession::AddVideoTrack(
    webrtc::scoped_refptr<webrtc::MediaStreamTrackInterface> track)
{
    auto peer = PeerConnection();
    if (!peer) {
        return webrtc::RTCError(webrtc::RTCErrorType::INVALID_STATE,
                                "The WebRTC session is not started.");
    }
    auto senderOrError = peer->AddTrack(std::move(track), {"screen-stream"});
    if (!senderOrError.ok()) {
        return senderOrError.MoveError();
    }

    const auto sender = senderOrError.value();
    for (const auto& transceiver : peer->GetTransceivers()) {
        if (transceiver && transceiver->sender() == sender) {
            return transceiver;
        }
    }
    return webrtc::RTCError(
        webrtc::RTCErrorType::INTERNAL_ERROR,
        "AddTrack succeeded but its transceiver was not found.");
}

webrtc::RTCErrorOr<
    webrtc::scoped_refptr<webrtc::RtpTransceiverInterface>>
LibWebRtcSession::AddVideoReceiveTransceiver()
{
    auto peer = PeerConnection();
    if (!peer) {
        return webrtc::RTCError(
            webrtc::RTCErrorType::INVALID_STATE,
            "PeerConnection is not ready.");
    }

    webrtc::RtpTransceiverInit init;
    init.direction = webrtc::RtpTransceiverDirection::kRecvOnly;
    return peer->AddTransceiver(webrtc::MediaType::VIDEO, init);
}
