// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#pragma once

#include <cstdint>

namespace remote {

enum class ProgressiveBitrateCeilingStatus {
    kDisabled,
    kWaitingForStats,
    kStabilizing,
    kStable,
    kProbePending,
    kCooldown,
};

struct ProgressiveBitrateCeilingConfig {
    double capacityEmaAlpha = 0.25;
    double growthFactor = 1.5;
    double capacityUtilizationThreshold = 0.75;
    double maximumLossPercent = 2.0;
    double probeFailureCapacityRatio = 0.65;
    double probeFailureRttRatio = 1.5;
    double probeFailureRttIncreaseMs = 50.0;
    std::uint32_t requiredStableSamples = 3;
    std::uint32_t requiredFailureSamples = 2;
    std::uint64_t minimumIncreaseIntervalMs = 5'000;
    std::uint64_t probeEvaluationDelayMs = 2'000;
    std::uint64_t probeSuccessDelayMs = 3'000;
    std::uint64_t failureCooldownMs = 10'000;
    std::uint64_t minimumIncreaseBps = 250'000;
};

struct ProgressiveBitrateCeilingSample {
    std::uint64_t timestampMs = 0;
    std::uint64_t availableOutgoingBitrateBps = 0;
    std::uint64_t targetBitrateBps = 0;
    double roundTripTimeMs = 0.0;
    double lossPercent = 0.0;
    std::uint64_t packetsDiscardedOnSend = 0;
};

struct ProgressiveBitrateCeilingState {
    bool enabled = false;
    std::uint64_t desiredMaxBitrateBps = 0;
    std::uint64_t appliedMaxBitrateBps = 0;
    std::uint64_t previousValidatedMaxBitrateBps = 0;
    std::uint64_t smoothedCapacityBps = 0;
    double smoothedRoundTripTimeMs = 0.0;
    std::uint32_t stableSampleCount = 0;
    std::uint32_t failureSampleCount = 0;
    std::uint64_t lastSampleTimestampMs = 0;
    std::uint64_t lastIncreaseTimestampMs = 0;
    std::uint64_t cooldownUntilMs = 0;
    std::uint64_t probeStartedTimestampMs = 0;
    std::uint64_t probeBaselineCapacityBps = 0;
    double probeBaselineRoundTripTimeMs = 0.0;
    std::uint64_t lastPacketsDiscardedOnSend = 0;
    bool hasDiscardCounter = false;
    bool probePending = false;
    bool lastProbeFailed = false;
    ProgressiveBitrateCeilingStatus status =
        ProgressiveBitrateCeilingStatus::kDisabled;
};

struct ProgressiveBitrateCeilingDecision {
    bool applyPeerConnectionMax = false;
    std::uint64_t peerConnectionMaxBitrateBps = 0;
};

// Initializes the controller after the screen-share startup bitrate bootstrap
// has installed its first PeerConnection-wide maximum. The controller only
// raises that ceiling; it never replaces GoogCC's target bitrate calculation.
void ResetProgressiveBitrateCeiling(
    ProgressiveBitrateCeilingState* state,
    bool enabled,
    std::uint64_t desiredMaxBitrateBps,
    std::uint64_t appliedMaxBitrateBps,
    std::uint64_t timestampMs);

// Records a new theoretical screen maximum. A lower target is applied
// immediately, while a higher target remains pending until stable BWE samples
// allow a bounded upward step.
ProgressiveBitrateCeilingDecision RetargetProgressiveBitrateCeiling(
    ProgressiveBitrateCeilingState* state,
    std::uint64_t desiredMaxBitrateBps,
    std::uint64_t timestampMs);

// Consumes one completed WebRTC stats sample and optionally requests one
// bounded PeerConnection maximum increase. Missing or stale stats never lower
// the current ceiling.
ProgressiveBitrateCeilingDecision EvaluateProgressiveBitrateCeiling(
    ProgressiveBitrateCeilingState* state,
    const ProgressiveBitrateCeilingSample& sample,
    const ProgressiveBitrateCeilingConfig& config = {});

enum class ScreenContentActivity {
    kUnknown,
    kStarting,
    kActive,
    kIdle,
};

enum class AdaptiveScreenFrameRateStatus {
    kDisabled,
    kWaitingForActivity,
    kIdleSuspended,
    kStartupGrace,
    kWaitingForCapacity,
    kStable,
    kReducing,
    kRecovering,
};

struct AdaptiveScreenFrameRateConfig {
    double capacityEmaAlpha = 0.25;
    double capacitySafetyRatio = 0.85;
    double recoveryHeadroomRatio = 1.15;
    std::uint32_t requiredReductionSamples = 2;
    std::uint32_t requiredRecoverySamples = 5;
    std::uint64_t startupGraceMs = 8'000;
    std::uint64_t minimumReductionIntervalMs = 2'000;
    std::uint64_t minimumRecoveryIntervalMs = 5'000;
};

struct AdaptiveScreenFrameRateSample {
    std::uint64_t timestampMs = 0;
    ScreenContentActivity activity = ScreenContentActivity::kUnknown;
    std::uint64_t capacityBps = 0;
};

struct AdaptiveScreenFrameRateState {
    bool enabled = false;
    std::uint32_t requestedFrameRate = 0;
    std::uint32_t effectiveFrameRate = 0;
    std::uint32_t outputWidth = 0;
    std::uint32_t outputHeight = 0;
    std::uint64_t smoothedCapacityBps = 0;
    std::uint32_t reductionSampleCount = 0;
    std::uint32_t recoverySampleCount = 0;
    std::uint64_t lastSampleTimestampMs = 0;
    std::uint64_t lastRetargetTimestampMs = 0;
    std::uint64_t lastChangeTimestampMs = 0;
    AdaptiveScreenFrameRateStatus status =
        AdaptiveScreenFrameRateStatus::kDisabled;
};

struct AdaptiveScreenFrameRateDecision {
    bool applyEffectiveFrameRate = false;
    std::uint32_t effectiveFrameRate = 0;
};

// Starts one per-PeerConnection network frame-rate controller. It changes only
// that connection's RTP sender maximum; the shared desktop capture scheduler
// and the user's requested frame rate remain untouched.
void ResetAdaptiveScreenFrameRate(
    AdaptiveScreenFrameRateState* state,
    bool enabled,
    std::uint32_t requestedFrameRate,
    std::uint32_t outputWidth,
    std::uint32_t outputHeight,
    std::uint64_t timestampMs);

// A user FPS or resolution change takes effect immediately. Later stats may
// temporarily reduce the effective sender FPS without overwriting the request.
AdaptiveScreenFrameRateDecision RetargetAdaptiveScreenFrameRate(
    AdaptiveScreenFrameRateState* state,
    std::uint32_t requestedFrameRate,
    std::uint32_t outputWidth,
    std::uint32_t outputHeight,
    std::uint64_t timestampMs);

// Uses the BWE capacity estimate rather than actual media bitrate. Evaluation
// is suspended while the desktop is starting or idle, so libwebrtc's unchanged
// frame suppression cannot be mistaken for network congestion.
AdaptiveScreenFrameRateDecision EvaluateAdaptiveScreenFrameRate(
    AdaptiveScreenFrameRateState* state,
    const AdaptiveScreenFrameRateSample& sample,
    const AdaptiveScreenFrameRateConfig& config = {});

}  // namespace remote
