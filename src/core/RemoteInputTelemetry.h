// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#pragma once

#include <algorithm>
#include <array>
#include <atomic>
#include <chrono>
#include <cstdint>
#include <limits>
#include <mutex>

namespace remote {

struct RemoteInputTelemetryCounters {
    std::uint64_t generatedMouseMoves = 0;
    std::uint64_t dispatchedMouseMoves = 0;
    std::uint64_t coalescedMouseMoves = 0;
    std::uint64_t generatedMouseButtons = 0;
    std::uint64_t generatedMouseWheels = 0;
    std::uint64_t generatedKeys = 0;
    std::uint64_t fastPacketsQueued = 0;
    std::uint64_t reliablePacketsQueued = 0;
    std::uint64_t packetsSent = 0;
    std::uint64_t packetsSendFailed = 0;
    std::uint64_t packetsReceived = 0;
    std::uint64_t packetsDropped = 0;
    std::uint64_t injectedMouseMoves = 0;
    std::uint64_t injectedMouseButtons = 0;
    std::uint64_t injectedMouseWheels = 0;
    std::uint64_t injectedKeys = 0;
    std::uint64_t injectionFailures = 0;
};

struct RemoteInputTelemetrySnapshot {
    bool enabled = false;
    std::uint32_t moveDispatchRateLimitHz = 0;
    std::uint32_t sampleWindowMs = 0;
    std::uint32_t moveDispatchIntervalSamples = 0;
    std::uint32_t moveDispatchIntervalAverageUs = 0;
    std::uint32_t moveDispatchIntervalP95Us = 0;
    std::uint32_t moveDispatchIntervalMaximumUs = 0;
    RemoteInputTelemetryCounters totals;
    RemoteInputTelemetryCounters perSecond;
};

// Process-wide, opt-in counters for the controller, DataChannel and Windows
// SendInput stages. Keeping this independent from a particular room pair lets
// the controlled and controlling application expose the same diagnostics UI.
class RemoteInputTelemetry final {
public:
    static RemoteInputTelemetry& Instance()
    {
        static RemoteInputTelemetry instance;
        return instance;
    }

    void SetEnabled(bool enabled)
    {
        enabled_.store(false, std::memory_order_release);
        ResetCounters();
        {
            std::lock_guard lock(sampleMutex_);
            previousSample_ = {};
            previousSampleAt_ = std::chrono::steady_clock::now();
        }
        enabled_.store(enabled, std::memory_order_release);
    }

    bool Enabled() const
    {
        return enabled_.load(std::memory_order_acquire);
    }

    void SetMoveDispatchRateLimit(std::uint32_t hertz)
    {
        moveDispatchRateLimitHz_.store(hertz, std::memory_order_release);
    }

    void RecordGeneratedMouseMove()
    {
        Increment(generatedMouseMoves_);
    }

    void RecordDispatchedMouseMove()
    {
        if (!Enabled()) {
            return;
        }
        dispatchedMouseMoves_.fetch_add(1, std::memory_order_relaxed);
        const auto nowUs = static_cast<std::uint64_t>(
            std::chrono::duration_cast<std::chrono::microseconds>(
                std::chrono::steady_clock::now().time_since_epoch())
                .count());
        const auto previousUs = lastMoveDispatchAtUs_.exchange(
            nowUs, std::memory_order_relaxed);
        if (!previousUs || nowUs <= previousUs) {
            return;
        }
        const auto intervalUs = nowUs - previousUs;
        // Long idle gaps do not describe scheduler cadence and would make the
        // first movement after an idle period dominate the sample.
        if (intervalUs > kMaximumMeasuredMoveIntervalUs) {
            return;
        }
        std::lock_guard intervalLock(sampleMutex_);
        moveDispatchIntervalSamples_.fetch_add(
            1, std::memory_order_relaxed);
        moveDispatchIntervalSumUs_.fetch_add(
            intervalUs, std::memory_order_relaxed);
        auto maximum = moveDispatchIntervalMaximumUs_.load(
            std::memory_order_relaxed);
        while (maximum < intervalUs &&
               !moveDispatchIntervalMaximumUs_.compare_exchange_weak(
                   maximum,
                   intervalUs,
                   std::memory_order_relaxed,
                   std::memory_order_relaxed)) {
        }
        const auto bucket = (std::min<std::size_t>)(
            static_cast<std::size_t>(
                intervalUs / kMoveIntervalHistogramBucketUs),
            kMoveIntervalHistogramBuckets - 1);
        moveDispatchIntervalHistogram_[bucket].fetch_add(
            1, std::memory_order_relaxed);
    }

    void RecordCoalescedMouseMove()
    {
        Increment(coalescedMouseMoves_);
    }

    void RecordGeneratedMouseButton()
    {
        Increment(generatedMouseButtons_);
    }

    void RecordGeneratedMouseWheel()
    {
        Increment(generatedMouseWheels_);
    }

    void RecordGeneratedKey()
    {
        Increment(generatedKeys_);
    }

    void RecordPacketQueued(bool fastChannel)
    {
        Increment(fastChannel ? fastPacketsQueued_
                              : reliablePacketsQueued_);
    }

    void RecordPacketSendResult(bool succeeded)
    {
        Increment(succeeded ? packetsSent_ : packetsSendFailed_);
    }

    void RecordPacketReceived()
    {
        Increment(packetsReceived_);
    }

    void RecordPacketDropped()
    {
        Increment(packetsDropped_);
    }

    enum class InjectionType {
        kMouseMove,
        kMouseButton,
        kMouseWheel,
        kKey,
    };

    void RecordInjection(InjectionType type, bool succeeded)
    {
        if (!Enabled()) {
            return;
        }
        switch (type) {
        case InjectionType::kMouseMove:
            injectedMouseMoves_.fetch_add(1, std::memory_order_relaxed);
            break;
        case InjectionType::kMouseButton:
            injectedMouseButtons_.fetch_add(1, std::memory_order_relaxed);
            break;
        case InjectionType::kMouseWheel:
            injectedMouseWheels_.fetch_add(1, std::memory_order_relaxed);
            break;
        case InjectionType::kKey:
            injectedKeys_.fetch_add(1, std::memory_order_relaxed);
            break;
        }
        if (!succeeded) {
            injectionFailures_.fetch_add(1, std::memory_order_relaxed);
        }
    }

    RemoteInputTelemetrySnapshot TakeSnapshot()
    {
        RemoteInputTelemetrySnapshot snapshot;
        snapshot.enabled = Enabled();
        snapshot.moveDispatchRateLimitHz =
            moveDispatchRateLimitHz_.load(std::memory_order_acquire);
        if (!snapshot.enabled) {
            return snapshot;
        }

        snapshot.totals = LoadCounters();
        const auto now = std::chrono::steady_clock::now();
        std::lock_guard lock(sampleMutex_);
        const auto elapsedMs = std::chrono::duration_cast<
            std::chrono::milliseconds>(now - previousSampleAt_).count();
        snapshot.sampleWindowMs = static_cast<std::uint32_t>(
            (std::max<std::int64_t>)(elapsedMs, 0));
        if (elapsedMs > 0) {
            snapshot.perSecond = Rate(
                snapshot.totals, previousSample_, elapsedMs);
        }
        const auto intervalSamples =
            moveDispatchIntervalSamples_.exchange(
                0, std::memory_order_relaxed);
        const auto intervalSumUs =
            moveDispatchIntervalSumUs_.exchange(
                0, std::memory_order_relaxed);
        snapshot.moveDispatchIntervalSamples =
            static_cast<std::uint32_t>((std::min<std::uint64_t>)(
                intervalSamples,
                std::numeric_limits<std::uint32_t>::max()));
        if (intervalSamples) {
            snapshot.moveDispatchIntervalAverageUs =
                static_cast<std::uint32_t>(intervalSumUs /
                    intervalSamples);
        }
        snapshot.moveDispatchIntervalMaximumUs =
            static_cast<std::uint32_t>((std::min<std::uint64_t>)(
                moveDispatchIntervalMaximumUs_.exchange(
                    0, std::memory_order_relaxed),
                std::numeric_limits<std::uint32_t>::max()));
        const auto percentileTarget =
            intervalSamples ? (intervalSamples * 95 + 99) / 100 : 0;
        std::uint64_t cumulativeSamples = 0;
        for (std::size_t index = 0;
             index < moveDispatchIntervalHistogram_.size(); ++index) {
            cumulativeSamples +=
                moveDispatchIntervalHistogram_[index].exchange(
                    0, std::memory_order_relaxed);
            if (!snapshot.moveDispatchIntervalP95Us &&
                percentileTarget &&
                cumulativeSamples >= percentileTarget) {
                snapshot.moveDispatchIntervalP95Us =
                    static_cast<std::uint32_t>(
                        (index + 1) *
                        kMoveIntervalHistogramBucketUs);
            }
        }
        previousSample_ = snapshot.totals;
        previousSampleAt_ = now;
        return snapshot;
    }

private:
    RemoteInputTelemetry() = default;

    static constexpr std::uint64_t
        kMaximumMeasuredMoveIntervalUs = 250'000;
    static constexpr std::uint64_t
        kMoveIntervalHistogramBucketUs = 500;
    static constexpr std::size_t
        kMoveIntervalHistogramBuckets = 128;

    void Increment(std::atomic<std::uint64_t>& counter)
    {
        if (Enabled()) {
            counter.fetch_add(1, std::memory_order_relaxed);
        }
    }

    RemoteInputTelemetryCounters LoadCounters() const
    {
        RemoteInputTelemetryCounters result;
        result.generatedMouseMoves =
            generatedMouseMoves_.load(std::memory_order_relaxed);
        result.dispatchedMouseMoves =
            dispatchedMouseMoves_.load(std::memory_order_relaxed);
        result.coalescedMouseMoves =
            coalescedMouseMoves_.load(std::memory_order_relaxed);
        result.generatedMouseButtons =
            generatedMouseButtons_.load(std::memory_order_relaxed);
        result.generatedMouseWheels =
            generatedMouseWheels_.load(std::memory_order_relaxed);
        result.generatedKeys =
            generatedKeys_.load(std::memory_order_relaxed);
        result.fastPacketsQueued =
            fastPacketsQueued_.load(std::memory_order_relaxed);
        result.reliablePacketsQueued =
            reliablePacketsQueued_.load(std::memory_order_relaxed);
        result.packetsSent =
            packetsSent_.load(std::memory_order_relaxed);
        result.packetsSendFailed =
            packetsSendFailed_.load(std::memory_order_relaxed);
        result.packetsReceived =
            packetsReceived_.load(std::memory_order_relaxed);
        result.packetsDropped =
            packetsDropped_.load(std::memory_order_relaxed);
        result.injectedMouseMoves =
            injectedMouseMoves_.load(std::memory_order_relaxed);
        result.injectedMouseButtons =
            injectedMouseButtons_.load(std::memory_order_relaxed);
        result.injectedMouseWheels =
            injectedMouseWheels_.load(std::memory_order_relaxed);
        result.injectedKeys =
            injectedKeys_.load(std::memory_order_relaxed);
        result.injectionFailures =
            injectionFailures_.load(std::memory_order_relaxed);
        return result;
    }

    static std::uint64_t PerSecond(std::uint64_t current,
                                   std::uint64_t previous,
                                   std::int64_t elapsedMs)
    {
        const std::uint64_t delta =
            current >= previous ? current - previous : 0;
        return delta * 1000ULL /
            static_cast<std::uint64_t>((std::max<std::int64_t>)(
                elapsedMs, 1));
    }

    static RemoteInputTelemetryCounters Rate(
        const RemoteInputTelemetryCounters& current,
        const RemoteInputTelemetryCounters& previous,
        std::int64_t elapsedMs)
    {
        RemoteInputTelemetryCounters result;
#define REMOTEC_INPUT_RATE(field) \
        result.field = PerSecond(current.field, previous.field, elapsedMs)
        REMOTEC_INPUT_RATE(generatedMouseMoves);
        REMOTEC_INPUT_RATE(dispatchedMouseMoves);
        REMOTEC_INPUT_RATE(coalescedMouseMoves);
        REMOTEC_INPUT_RATE(generatedMouseButtons);
        REMOTEC_INPUT_RATE(generatedMouseWheels);
        REMOTEC_INPUT_RATE(generatedKeys);
        REMOTEC_INPUT_RATE(fastPacketsQueued);
        REMOTEC_INPUT_RATE(reliablePacketsQueued);
        REMOTEC_INPUT_RATE(packetsSent);
        REMOTEC_INPUT_RATE(packetsSendFailed);
        REMOTEC_INPUT_RATE(packetsReceived);
        REMOTEC_INPUT_RATE(packetsDropped);
        REMOTEC_INPUT_RATE(injectedMouseMoves);
        REMOTEC_INPUT_RATE(injectedMouseButtons);
        REMOTEC_INPUT_RATE(injectedMouseWheels);
        REMOTEC_INPUT_RATE(injectedKeys);
        REMOTEC_INPUT_RATE(injectionFailures);
#undef REMOTEC_INPUT_RATE
        return result;
    }

    void ResetCounters()
    {
#define REMOTEC_RESET_INPUT_COUNTER(counter) \
        counter.store(0, std::memory_order_relaxed)
        REMOTEC_RESET_INPUT_COUNTER(generatedMouseMoves_);
        REMOTEC_RESET_INPUT_COUNTER(dispatchedMouseMoves_);
        REMOTEC_RESET_INPUT_COUNTER(coalescedMouseMoves_);
        REMOTEC_RESET_INPUT_COUNTER(generatedMouseButtons_);
        REMOTEC_RESET_INPUT_COUNTER(generatedMouseWheels_);
        REMOTEC_RESET_INPUT_COUNTER(generatedKeys_);
        REMOTEC_RESET_INPUT_COUNTER(fastPacketsQueued_);
        REMOTEC_RESET_INPUT_COUNTER(reliablePacketsQueued_);
        REMOTEC_RESET_INPUT_COUNTER(packetsSent_);
        REMOTEC_RESET_INPUT_COUNTER(packetsSendFailed_);
        REMOTEC_RESET_INPUT_COUNTER(packetsReceived_);
        REMOTEC_RESET_INPUT_COUNTER(packetsDropped_);
        REMOTEC_RESET_INPUT_COUNTER(injectedMouseMoves_);
        REMOTEC_RESET_INPUT_COUNTER(injectedMouseButtons_);
        REMOTEC_RESET_INPUT_COUNTER(injectedMouseWheels_);
        REMOTEC_RESET_INPUT_COUNTER(injectedKeys_);
        REMOTEC_RESET_INPUT_COUNTER(injectionFailures_);
#undef REMOTEC_RESET_INPUT_COUNTER
        lastMoveDispatchAtUs_.store(0, std::memory_order_relaxed);
        moveDispatchIntervalSamples_.store(0, std::memory_order_relaxed);
        moveDispatchIntervalSumUs_.store(0, std::memory_order_relaxed);
        moveDispatchIntervalMaximumUs_.store(
            0, std::memory_order_relaxed);
        for (auto& bucket : moveDispatchIntervalHistogram_) {
            bucket.store(0, std::memory_order_relaxed);
        }
    }

    std::atomic_bool enabled_{false};
    std::atomic<std::uint32_t> moveDispatchRateLimitHz_{0};
    std::atomic<std::uint64_t> generatedMouseMoves_{0};
    std::atomic<std::uint64_t> dispatchedMouseMoves_{0};
    std::atomic<std::uint64_t> coalescedMouseMoves_{0};
    std::atomic<std::uint64_t> generatedMouseButtons_{0};
    std::atomic<std::uint64_t> generatedMouseWheels_{0};
    std::atomic<std::uint64_t> generatedKeys_{0};
    std::atomic<std::uint64_t> fastPacketsQueued_{0};
    std::atomic<std::uint64_t> reliablePacketsQueued_{0};
    std::atomic<std::uint64_t> packetsSent_{0};
    std::atomic<std::uint64_t> packetsSendFailed_{0};
    std::atomic<std::uint64_t> packetsReceived_{0};
    std::atomic<std::uint64_t> packetsDropped_{0};
    std::atomic<std::uint64_t> injectedMouseMoves_{0};
    std::atomic<std::uint64_t> injectedMouseButtons_{0};
    std::atomic<std::uint64_t> injectedMouseWheels_{0};
    std::atomic<std::uint64_t> injectedKeys_{0};
    std::atomic<std::uint64_t> injectionFailures_{0};
    std::atomic<std::uint64_t> lastMoveDispatchAtUs_{0};
    std::atomic<std::uint64_t> moveDispatchIntervalSamples_{0};
    std::atomic<std::uint64_t> moveDispatchIntervalSumUs_{0};
    std::atomic<std::uint64_t> moveDispatchIntervalMaximumUs_{0};
    std::array<
        std::atomic<std::uint64_t>,
        kMoveIntervalHistogramBuckets>
        moveDispatchIntervalHistogram_{};
    std::mutex sampleMutex_;
    RemoteInputTelemetryCounters previousSample_;
    std::chrono::steady_clock::time_point previousSampleAt_ =
        std::chrono::steady_clock::now();
};

}  // namespace remote
