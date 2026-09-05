// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#include "ScreenNetworkPolicy.h"

#include "ScreenStreamPolicy.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <limits>

namespace remote {
namespace {

std::uint64_t BlendUnsigned(
    std::uint64_t previous,
    std::uint64_t current,
    double alpha)
{
    if (previous == 0) {
        return current;
    }
    const long double blended =
        static_cast<long double>(previous) * (1.0 - alpha) +
        static_cast<long double>(current) * alpha;
    return static_cast<std::uint64_t>(std::clamp<long double>(
        blended,
        0.0L,
        static_cast<long double>(
            (std::numeric_limits<std::uint64_t>::max)())));
}

double BlendDouble(double previous, double current, double alpha)
{
    if (current <= 0.0) {
        return previous;
    }
    return previous <= 0.0
        ? current
        : previous * (1.0 - alpha) + current * alpha;
}

bool HasNewDiscardedPackets(
    ProgressiveBitrateCeilingState* state,
    std::uint64_t packetsDiscardedOnSend)
{
    if (!state->hasDiscardCounter) {
        state->lastPacketsDiscardedOnSend = packetsDiscardedOnSend;
        state->hasDiscardCounter = true;
        return false;
    }
    const bool increased =
        packetsDiscardedOnSend > state->lastPacketsDiscardedOnSend;
    state->lastPacketsDiscardedOnSend = packetsDiscardedOnSend;
    return increased;
}

bool RttRegressed(
    double currentRttMs,
    double baselineRttMs,
    const ProgressiveBitrateCeilingConfig& config)
{
    if (currentRttMs <= 0.0 || baselineRttMs <= 0.0) {
        return false;
    }
    return currentRttMs >
        (std::max)(
            baselineRttMs * config.probeFailureRttRatio,
            baselineRttMs + config.probeFailureRttIncreaseMs);
}

constexpr std::array<std::uint32_t, 9> kAdaptiveFrameRateCandidates{
    15, 20, 24, 30, 45, 60, 80, 100, 120};

std::uint64_t RequiredStartBitrate(
    std::uint32_t width,
    std::uint32_t height,
    std::uint32_t framesPerSecond)
{
    return ResolveScreenStreamPolicy(
        width,
        height,
        {width, height, framesPerSecond}).startBitrateBps;
}

std::uint32_t HighestSupportedFrameRate(
    const AdaptiveScreenFrameRateState& state,
    std::uint64_t usableCapacityBps)
{
    const std::uint32_t floor =
        (std::min)(state.requestedFrameRate, 15u);
    std::uint32_t selected = floor;
    for (const auto candidate : kAdaptiveFrameRateCandidates) {
        if (candidate > state.requestedFrameRate) {
            break;
        }
        if (RequiredStartBitrate(
                state.outputWidth, state.outputHeight, candidate) <=
            usableCapacityBps) {
            selected = candidate;
        }
    }
    if (RequiredStartBitrate(
            state.outputWidth,
            state.outputHeight,
            state.requestedFrameRate) <= usableCapacityBps) {
        selected = state.requestedFrameRate;
    }
    return selected;
}

std::uint32_t NextRecoveryFrameRate(
    const AdaptiveScreenFrameRateState& state,
    std::uint32_t supportedFrameRate)
{
    std::uint32_t next = supportedFrameRate;
    for (const auto candidate : kAdaptiveFrameRateCandidates) {
        if (candidate > state.effectiveFrameRate &&
            candidate <= supportedFrameRate) {
            next = candidate;
            break;
        }
    }
    if (state.requestedFrameRate > state.effectiveFrameRate &&
        state.requestedFrameRate <= supportedFrameRate) {
        next = (std::min)(next, state.requestedFrameRate);
    }
    return next;
}

}  // namespace

void ResetProgressiveBitrateCeiling(
    ProgressiveBitrateCeilingState* state,
    bool enabled,
    std::uint64_t desiredMaxBitrateBps,
    std::uint64_t appliedMaxBitrateBps,
    std::uint64_t timestampMs)
{
    if (!state) {
        return;
    }
    *state = {};
    state->enabled = enabled;
    state->desiredMaxBitrateBps = desiredMaxBitrateBps;
    state->appliedMaxBitrateBps = appliedMaxBitrateBps;
    state->previousValidatedMaxBitrateBps = appliedMaxBitrateBps;
    state->lastIncreaseTimestampMs = timestampMs;
    state->status = enabled
        ? ProgressiveBitrateCeilingStatus::kWaitingForStats
        : ProgressiveBitrateCeilingStatus::kDisabled;
}

ProgressiveBitrateCeilingDecision RetargetProgressiveBitrateCeiling(
    ProgressiveBitrateCeilingState* state,
    std::uint64_t desiredMaxBitrateBps,
    std::uint64_t timestampMs)
{
    ProgressiveBitrateCeilingDecision decision;
    if (!state) {
        return decision;
    }
    state->desiredMaxBitrateBps = desiredMaxBitrateBps;
    if (!state->enabled) {
        state->appliedMaxBitrateBps = desiredMaxBitrateBps;
        state->previousValidatedMaxBitrateBps = desiredMaxBitrateBps;
        state->status = ProgressiveBitrateCeilingStatus::kDisabled;
        decision.applyPeerConnectionMax = true;
        decision.peerConnectionMaxBitrateBps = desiredMaxBitrateBps;
        return decision;
    }
    if (state->appliedMaxBitrateBps == 0) {
        state->status = ProgressiveBitrateCeilingStatus::kWaitingForStats;
        return decision;
    }
    if (desiredMaxBitrateBps < state->appliedMaxBitrateBps) {
        state->appliedMaxBitrateBps = desiredMaxBitrateBps;
        state->previousValidatedMaxBitrateBps = desiredMaxBitrateBps;
        state->stableSampleCount = 0;
        state->failureSampleCount = 0;
        state->cooldownUntilMs = 0;
        state->probePending = false;
        state->lastProbeFailed = false;
        state->lastIncreaseTimestampMs = timestampMs;
        state->status = ProgressiveBitrateCeilingStatus::kStabilizing;
        decision.applyPeerConnectionMax = true;
        decision.peerConnectionMaxBitrateBps = desiredMaxBitrateBps;
    }
    return decision;
}

ProgressiveBitrateCeilingDecision EvaluateProgressiveBitrateCeiling(
    ProgressiveBitrateCeilingState* state,
    const ProgressiveBitrateCeilingSample& sample,
    const ProgressiveBitrateCeilingConfig& config)
{
    ProgressiveBitrateCeilingDecision decision;
    if (!state || !state->enabled) {
        return decision;
    }
    if (sample.timestampMs == 0 ||
        sample.timestampMs <= state->lastSampleTimestampMs) {
        return decision;
    }
    state->lastSampleTimestampMs = sample.timestampMs;

    const std::uint64_t capacity =
        sample.availableOutgoingBitrateBps != 0
        ? sample.availableOutgoingBitrateBps
        : sample.targetBitrateBps;
    const bool discarded = HasNewDiscardedPackets(
        state, sample.packetsDiscardedOnSend);
    if (capacity == 0) {
        state->stableSampleCount = 0;
        state->status = sample.timestampMs < state->cooldownUntilMs
            ? ProgressiveBitrateCeilingStatus::kCooldown
            : ProgressiveBitrateCeilingStatus::kWaitingForStats;
        return decision;
    }

    const double alpha = std::clamp(config.capacityEmaAlpha, 0.0, 1.0);
    state->smoothedCapacityBps = BlendUnsigned(
        state->smoothedCapacityBps, capacity, alpha);
    state->smoothedRoundTripTimeMs = BlendDouble(
        state->smoothedRoundTripTimeMs,
        sample.roundTripTimeMs,
        alpha);

    if (state->probePending &&
        sample.timestampMs - state->probeStartedTimestampMs >=
            config.probeEvaluationDelayMs) {
        const bool capacityCollapsed =
            state->probeBaselineCapacityBps > 0 &&
            state->smoothedCapacityBps < static_cast<std::uint64_t>(
                state->probeBaselineCapacityBps *
                config.probeFailureCapacityRatio);
        const bool failed = capacityCollapsed || discarded ||
            sample.lossPercent > config.maximumLossPercent ||
            RttRegressed(
                state->smoothedRoundTripTimeMs,
                state->probeBaselineRoundTripTimeMs,
                config);
        state->failureSampleCount = failed
            ? state->failureSampleCount + 1
            : 0;
        if (state->failureSampleCount >= config.requiredFailureSamples) {
            state->probePending = false;
            state->lastProbeFailed = true;
            state->stableSampleCount = 0;
            state->cooldownUntilMs =
                sample.timestampMs + config.failureCooldownMs;
            state->status = ProgressiveBitrateCeilingStatus::kCooldown;
            return decision;
        }
        if (!failed &&
            sample.timestampMs - state->probeStartedTimestampMs >=
                config.probeSuccessDelayMs &&
            state->smoothedCapacityBps >= static_cast<std::uint64_t>(
                state->appliedMaxBitrateBps *
                config.capacityUtilizationThreshold)) {
            state->probePending = false;
            state->lastProbeFailed = false;
            state->failureSampleCount = 0;
            state->previousValidatedMaxBitrateBps =
                state->appliedMaxBitrateBps;
        }
    }

    if (state->probePending) {
        state->status = ProgressiveBitrateCeilingStatus::kProbePending;
        return decision;
    }
    if (sample.timestampMs < state->cooldownUntilMs) {
        state->status = ProgressiveBitrateCeilingStatus::kCooldown;
        return decision;
    }
    if (state->desiredMaxBitrateBps <= state->appliedMaxBitrateBps) {
        state->status = ProgressiveBitrateCeilingStatus::kStable;
        return decision;
    }

    const bool healthy = !discarded &&
        sample.lossPercent <= config.maximumLossPercent &&
        state->smoothedCapacityBps >= static_cast<std::uint64_t>(
            state->appliedMaxBitrateBps *
            config.capacityUtilizationThreshold);
    state->stableSampleCount = healthy
        ? state->stableSampleCount + 1
        : 0;
    if (!healthy ||
        state->stableSampleCount < config.requiredStableSamples ||
        sample.timestampMs - state->lastIncreaseTimestampMs <
            config.minimumIncreaseIntervalMs) {
        state->status = ProgressiveBitrateCeilingStatus::kStabilizing;
        return decision;
    }

    const long double currentGrowth =
        state->appliedMaxBitrateBps * config.growthFactor;
    const long double capacityGrowth =
        state->smoothedCapacityBps * config.growthFactor;
    const std::uint64_t boundedGrowth = static_cast<std::uint64_t>(
        (std::min)(currentGrowth, capacityGrowth));
    const std::uint64_t nextMax = (std::min)(
        state->desiredMaxBitrateBps,
        (std::max)(state->appliedMaxBitrateBps, boundedGrowth));
    if (nextMax <= state->appliedMaxBitrateBps ||
        (nextMax != state->desiredMaxBitrateBps &&
         nextMax - state->appliedMaxBitrateBps <
             config.minimumIncreaseBps)) {
        state->status = ProgressiveBitrateCeilingStatus::kStabilizing;
        return decision;
    }

    state->previousValidatedMaxBitrateBps =
        state->appliedMaxBitrateBps;
    state->probeBaselineCapacityBps = state->smoothedCapacityBps;
    state->probeBaselineRoundTripTimeMs =
        state->smoothedRoundTripTimeMs;
    state->appliedMaxBitrateBps = nextMax;
    state->lastIncreaseTimestampMs = sample.timestampMs;
    state->probeStartedTimestampMs = sample.timestampMs;
    state->stableSampleCount = 0;
    state->failureSampleCount = 0;
    state->probePending = true;
    state->lastProbeFailed = false;
    state->status = ProgressiveBitrateCeilingStatus::kProbePending;
    decision.applyPeerConnectionMax = true;
    decision.peerConnectionMaxBitrateBps = nextMax;
    return decision;
}

void ResetAdaptiveScreenFrameRate(
    AdaptiveScreenFrameRateState* state,
    bool enabled,
    std::uint32_t requestedFrameRate,
    std::uint32_t outputWidth,
    std::uint32_t outputHeight,
    std::uint64_t timestampMs)
{
    if (!state) {
        return;
    }
    *state = {};
    state->enabled = enabled;
    state->requestedFrameRate = requestedFrameRate;
    state->effectiveFrameRate = requestedFrameRate;
    state->outputWidth = outputWidth;
    state->outputHeight = outputHeight;
    state->lastRetargetTimestampMs = timestampMs;
    state->lastChangeTimestampMs = timestampMs;
    state->status = enabled
        ? AdaptiveScreenFrameRateStatus::kWaitingForActivity
        : AdaptiveScreenFrameRateStatus::kDisabled;
}

AdaptiveScreenFrameRateDecision RetargetAdaptiveScreenFrameRate(
    AdaptiveScreenFrameRateState* state,
    std::uint32_t requestedFrameRate,
    std::uint32_t outputWidth,
    std::uint32_t outputHeight,
    std::uint64_t timestampMs)
{
    AdaptiveScreenFrameRateDecision decision;
    if (!state) {
        return decision;
    }
    const bool changed =
        state->requestedFrameRate != requestedFrameRate ||
        state->outputWidth != outputWidth ||
        state->outputHeight != outputHeight;
    state->requestedFrameRate = requestedFrameRate;
    state->outputWidth = outputWidth;
    state->outputHeight = outputHeight;
    if (!changed) {
        return decision;
    }
    state->effectiveFrameRate = requestedFrameRate;
    state->reductionSampleCount = 0;
    state->recoverySampleCount = 0;
    state->lastRetargetTimestampMs = timestampMs;
    state->lastChangeTimestampMs = timestampMs;
    state->status = state->enabled
        ? AdaptiveScreenFrameRateStatus::kStartupGrace
        : AdaptiveScreenFrameRateStatus::kDisabled;
    decision.applyEffectiveFrameRate = true;
    decision.effectiveFrameRate = requestedFrameRate;
    return decision;
}

AdaptiveScreenFrameRateDecision EvaluateAdaptiveScreenFrameRate(
    AdaptiveScreenFrameRateState* state,
    const AdaptiveScreenFrameRateSample& sample,
    const AdaptiveScreenFrameRateConfig& config)
{
    AdaptiveScreenFrameRateDecision decision;
    if (!state || !state->enabled || state->requestedFrameRate == 0 ||
        state->outputWidth == 0 || state->outputHeight == 0 ||
        sample.timestampMs == 0 ||
        sample.timestampMs <= state->lastSampleTimestampMs) {
        return decision;
    }
    state->lastSampleTimestampMs = sample.timestampMs;

    if (sample.activity != ScreenContentActivity::kActive) {
        state->reductionSampleCount = 0;
        state->recoverySampleCount = 0;
        state->status = sample.activity == ScreenContentActivity::kIdle
            ? AdaptiveScreenFrameRateStatus::kIdleSuspended
            : AdaptiveScreenFrameRateStatus::kWaitingForActivity;
        return decision;
    }
    if (sample.timestampMs - state->lastRetargetTimestampMs <
        config.startupGraceMs) {
        state->reductionSampleCount = 0;
        state->recoverySampleCount = 0;
        state->status = AdaptiveScreenFrameRateStatus::kStartupGrace;
        return decision;
    }
    if (sample.capacityBps == 0) {
        state->reductionSampleCount = 0;
        state->recoverySampleCount = 0;
        state->status = AdaptiveScreenFrameRateStatus::kWaitingForCapacity;
        return decision;
    }

    state->smoothedCapacityBps = BlendUnsigned(
        state->smoothedCapacityBps,
        sample.capacityBps,
        std::clamp(config.capacityEmaAlpha, 0.0, 1.0));

    const auto usableCapacityBps = static_cast<std::uint64_t>(
        state->smoothedCapacityBps *
        std::clamp(config.capacitySafetyRatio, 0.1, 1.0));
    const auto supportedFrameRate = HighestSupportedFrameRate(
        *state, usableCapacityBps);
    if (supportedFrameRate < state->effectiveFrameRate) {
        state->recoverySampleCount = 0;
        ++state->reductionSampleCount;
        state->status = AdaptiveScreenFrameRateStatus::kReducing;
        if (state->reductionSampleCount >=
                config.requiredReductionSamples &&
            sample.timestampMs - state->lastChangeTimestampMs >=
                config.minimumReductionIntervalMs) {
            state->effectiveFrameRate = supportedFrameRate;
            state->lastChangeTimestampMs = sample.timestampMs;
            state->reductionSampleCount = 0;
            decision.applyEffectiveFrameRate = true;
            decision.effectiveFrameRate = supportedFrameRate;
        }
        return decision;
    }
    if (supportedFrameRate > state->effectiveFrameRate) {
        state->reductionSampleCount = 0;
        const auto nextFrameRate = NextRecoveryFrameRate(
            *state, supportedFrameRate);
        const auto nextRequiredBitrate = RequiredStartBitrate(
            state->outputWidth,
            state->outputHeight,
            nextFrameRate);
        const bool hasHeadroom = usableCapacityBps >=
            static_cast<std::uint64_t>(
                nextRequiredBitrate *
                (std::max)(1.0, config.recoveryHeadroomRatio));
        state->recoverySampleCount = hasHeadroom
            ? state->recoverySampleCount + 1
            : 0;
        state->status = AdaptiveScreenFrameRateStatus::kRecovering;
        if (hasHeadroom &&
            state->recoverySampleCount >= config.requiredRecoverySamples &&
            sample.timestampMs - state->lastChangeTimestampMs >=
                config.minimumRecoveryIntervalMs) {
            state->effectiveFrameRate = nextFrameRate;
            state->lastChangeTimestampMs = sample.timestampMs;
            state->recoverySampleCount = 0;
            decision.applyEffectiveFrameRate = true;
            decision.effectiveFrameRate = nextFrameRate;
        }
        return decision;
    }

    state->reductionSampleCount = 0;
    state->recoverySampleCount = 0;
    state->status = AdaptiveScreenFrameRateStatus::kStable;
    return decision;
}

}  // namespace remote
