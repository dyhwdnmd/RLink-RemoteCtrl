// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#pragma once

#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <deque>
#include <mutex>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

namespace remote {

enum class VideoPresentationPath {
    kUnknown,
    kCpuQt,
    kCpuD3D11,
    kCpuNv12D3D11,
    kCpuI420D3D11,
    kD3D11,
};

struct VideoPresentationTelemetrySnapshot {
    bool available = false;
    VideoPresentationPath path = VideoPresentationPath::kUnknown;
    double localRefreshRateHz = 0.0;
    double arrivalFramesPerSecond = 0.0;
    double presentedFramesPerSecond = 0.0;
    std::uint64_t arrivedFrames = 0;
    std::uint64_t presentedFrames = 0;
    std::uint64_t supersededFrames = 0;
    std::uint64_t conversionFrames = 0;
    std::uint64_t presentFailures = 0;
    double latestConversionMs = 0.0;
    double averageConversionMs = 0.0;
    double latestRenderSubmitMs = 0.0;
    double averageRenderSubmitMs = 0.0;
    double latestPresentCallMs = 0.0;
    double averagePresentCallMs = 0.0;
    double averagePresentedIntervalMs = 0.0;
    double p95PresentedIntervalMs = 0.0;
    double maximumPresentedIntervalMs = 0.0;
    double latestReceiverPipelineMs = 0.0;
    double averageReceiverPipelineMs = 0.0;
    double p95ReceiverPipelineMs = 0.0;
    double maximumReceiverPipelineMs = 0.0;
};

// Application-side video display telemetry. Renderers only publish narrowly
// defined timing events; the diagnostics layer consumes immutable snapshots.
// This keeps Qt/D3D implementation details out of the WebRTC stats collector.
class VideoPresentationTelemetryRegistry final {
public:
    static VideoPresentationTelemetryRegistry& Instance()
    {
        static VideoPresentationTelemetryRegistry registry;
        return registry;
    }

    std::uint64_t Register(std::string peerDeviceId)
    {
        std::lock_guard lock(mutex_);
        const std::uint64_t id = nextInstanceId_++;
        Entry& entry = entries_[id];
        entry.peerDeviceId = std::move(peerDeviceId);
        entry.registeredAt = Clock::now();
        return id;
    }

    void Unregister(std::uint64_t id)
    {
        std::lock_guard lock(mutex_);
        entries_.erase(id);
    }

    void SetLocalRefreshRate(std::uint64_t id, double refreshRateHz)
    {
        std::lock_guard lock(mutex_);
        if (auto* entry = Find(id)) {
            entry->localRefreshRateHz = refreshRateHz;
        }
    }

    void RecordArrival(std::uint64_t id, VideoPresentationPath path)
    {
        const auto now = Clock::now();
        std::lock_guard lock(mutex_);
        if (auto* entry = Find(id)) {
            entry->path = path;
            ++entry->arrivedFrames;
            PushTimestamp(entry->arrivalTimes, now);
        }
    }

    void RecordSuperseded(std::uint64_t id)
    {
        std::lock_guard lock(mutex_);
        if (auto* entry = Find(id)) {
            ++entry->supersededFrames;
        }
    }

    // Time from the final RTP packet of a frame reaching this receiver to the
    // decoded frame reaching the application sink. Both timestamps use
    // libwebrtc's local monotonic clock, so this does not depend on clock
    // synchronization between the two computers.
    void RecordReceiverPipeline(std::uint64_t id, std::uint64_t durationUs)
    {
        std::lock_guard lock(mutex_);
        if (auto* entry = Find(id)) {
            PushDuration(entry->receiverPipelineTimesUs, durationUs);
        }
    }

    void RecordConversion(std::uint64_t id, std::uint64_t durationUs)
    {
        std::lock_guard lock(mutex_);
        if (auto* entry = Find(id)) {
            ++entry->conversionFrames;
            PushDuration(entry->conversionTimesUs, durationUs);
        }
    }

    void RecordPresented(std::uint64_t id,
        VideoPresentationPath path,
        std::uint64_t videoProcessorSubmitUs,
        std::uint64_t presentCallUs)
    {
        const auto now = Clock::now();
        std::lock_guard lock(mutex_);
        if (auto* entry = Find(id)) {
            entry->path = path;
            ++entry->presentedFrames;
            if (entry->lastPresentedAt != Clock::time_point{}) {
                const auto intervalUs =
                    std::chrono::duration_cast<std::chrono::microseconds>(
                        now - entry->lastPresentedAt)
                        .count();
                if (intervalUs > 0) {
                    PushDuration(entry->presentedIntervalsUs,
                        static_cast<std::uint64_t>(intervalUs));
                }
            }
            entry->lastPresentedAt = now;
            PushTimestamp(entry->presentedTimes, now);
            PushDuration(entry->videoProcessorSubmitTimesUs,
                videoProcessorSubmitUs);
            PushDuration(entry->presentCallTimesUs, presentCallUs);
        }
    }

    void RecordPresentFailure(std::uint64_t id)
    {
        std::lock_guard lock(mutex_);
        if (auto* entry = Find(id)) {
            ++entry->presentFailures;
        }
    }

    std::optional<VideoPresentationTelemetrySnapshot> SnapshotForPeer(
        const std::string& peerDeviceId) const
    {
        const auto now = Clock::now();
        std::lock_guard lock(mutex_);
        const Entry* selected = nullptr;
        for (const auto& [id, entry] : entries_) {
            (void)id;
            if (entry.peerDeviceId != peerDeviceId) {
                continue;
            }
            if (!selected || entry.registeredAt > selected->registeredAt) {
                selected = &entry;
            }
        }
        if (!selected || selected->arrivedFrames == 0) {
            return std::nullopt;
        }

        VideoPresentationTelemetrySnapshot snapshot;
        snapshot.available = true;
        snapshot.path = selected->path;
        snapshot.localRefreshRateHz = selected->localRefreshRateHz;
        snapshot.arrivalFramesPerSecond =
            TrailingRate(selected->arrivalTimes, now);
        snapshot.presentedFramesPerSecond =
            TrailingRate(selected->presentedTimes, now);
        snapshot.arrivedFrames = selected->arrivedFrames;
        snapshot.presentedFrames = selected->presentedFrames;
        snapshot.supersededFrames = selected->supersededFrames;
        snapshot.conversionFrames = selected->conversionFrames;
        snapshot.presentFailures = selected->presentFailures;
        snapshot.latestConversionMs = LatestMs(
            selected->conversionTimesUs);
        snapshot.averageConversionMs = AverageMs(
            selected->conversionTimesUs);
        snapshot.latestRenderSubmitMs = LatestMs(
            selected->videoProcessorSubmitTimesUs);
        snapshot.averageRenderSubmitMs = AverageMs(
            selected->videoProcessorSubmitTimesUs);
        snapshot.latestPresentCallMs = LatestMs(
            selected->presentCallTimesUs);
        snapshot.averagePresentCallMs = AverageMs(
            selected->presentCallTimesUs);
        snapshot.averagePresentedIntervalMs = AverageMs(
            selected->presentedIntervalsUs);
        snapshot.p95PresentedIntervalMs = PercentileMs(
            selected->presentedIntervalsUs, 0.95);
        snapshot.maximumPresentedIntervalMs = MaximumMs(
            selected->presentedIntervalsUs);
        snapshot.latestReceiverPipelineMs = LatestMs(
            selected->receiverPipelineTimesUs);
        snapshot.averageReceiverPipelineMs = AverageMs(
            selected->receiverPipelineTimesUs);
        snapshot.p95ReceiverPipelineMs = PercentileMs(
            selected->receiverPipelineTimesUs, 0.95);
        snapshot.maximumReceiverPipelineMs = MaximumMs(
            selected->receiverPipelineTimesUs);
        return snapshot;
    }

private:
    using Clock = std::chrono::steady_clock;
    static constexpr std::size_t kTimingWindowSize = 240;

    struct Entry {
        std::string peerDeviceId;
        VideoPresentationPath path = VideoPresentationPath::kUnknown;
        double localRefreshRateHz = 0.0;
        std::uint64_t arrivedFrames = 0;
        std::uint64_t presentedFrames = 0;
        std::uint64_t supersededFrames = 0;
        std::uint64_t conversionFrames = 0;
        std::uint64_t presentFailures = 0;
        Clock::time_point registeredAt{};
        Clock::time_point lastPresentedAt{};
        std::deque<Clock::time_point> arrivalTimes;
        std::deque<Clock::time_point> presentedTimes;
        std::deque<std::uint64_t> conversionTimesUs;
        std::deque<std::uint64_t> videoProcessorSubmitTimesUs;
        std::deque<std::uint64_t> presentCallTimesUs;
        std::deque<std::uint64_t> presentedIntervalsUs;
        std::deque<std::uint64_t> receiverPipelineTimesUs;
    };

    Entry* Find(std::uint64_t id)
    {
        const auto found = entries_.find(id);
        return found == entries_.end() ? nullptr : &found->second;
    }

    static void PushTimestamp(
        std::deque<Clock::time_point>& values,
        Clock::time_point value)
    {
        values.push_back(value);
        while (values.size() > kTimingWindowSize) {
            values.pop_front();
        }
    }

    static void PushDuration(
        std::deque<std::uint64_t>& values,
        std::uint64_t value)
    {
        values.push_back(value);
        while (values.size() > kTimingWindowSize) {
            values.pop_front();
        }
    }

    static double TrailingRate(
        const std::deque<Clock::time_point>& values,
        Clock::time_point now)
    {
        constexpr auto window = std::chrono::seconds(1);
        const auto first = std::lower_bound(
            values.begin(), values.end(), now - window);
        return static_cast<double>(std::distance(first, values.end()));
    }

    static double LatestMs(const std::deque<std::uint64_t>& values)
    {
        return values.empty() ? 0.0
                              : static_cast<double>(values.back()) / 1000.0;
    }

    static double AverageMs(const std::deque<std::uint64_t>& values)
    {
        if (values.empty()) {
            return 0.0;
        }
        long double total = 0.0;
        for (const auto value : values) {
            total += value;
        }
        return static_cast<double>(total / values.size()) / 1000.0;
    }

    static double MaximumMs(const std::deque<std::uint64_t>& values)
    {
        return values.empty()
                   ? 0.0
                   : static_cast<double>(
                         *std::max_element(values.begin(), values.end())) /
                         1000.0;
    }

    static double PercentileMs(
        const std::deque<std::uint64_t>& values,
        double percentile)
    {
        if (values.empty()) {
            return 0.0;
        }
        std::vector<std::uint64_t> sorted(values.begin(), values.end());
        std::sort(sorted.begin(), sorted.end());
        const std::size_t index = static_cast<std::size_t>(std::ceil(
            percentile * static_cast<double>(sorted.size()))) - 1;
        return static_cast<double>(sorted[(std::min)(
            index, sorted.size() - 1)]) / 1000.0;
    }

    mutable std::mutex mutex_;
    std::uint64_t nextInstanceId_ = 1;
    std::unordered_map<std::uint64_t, Entry> entries_;
};

}  // namespace remote
