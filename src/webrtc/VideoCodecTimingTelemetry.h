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
#include <utility>
#include <vector>

namespace remote {

enum class VideoCodecTimingDirection {
    kEncode,
    kDecode,
};

struct VideoDecodePipelineTimingSnapshot {
    bool available = false;
    bool asynchronous = false;
    std::uint64_t inputPreparationUs = 0;
    std::uint64_t inputQueueWaitUs = 0;
    std::uint64_t transformWaitUs = 0;
    std::uint64_t outputDeliveryUs = 0;
    std::uint64_t averageTransformWaitUs = 0;
    std::uint64_t p95TransformWaitUs = 0;
    std::uint64_t maximumTransformWaitUs = 0;
    std::uint32_t queuedInputFrames = 0;
    std::uint32_t inFlightFrames = 0;
    std::uint32_t peakBacklogFrames = 0;
    std::uint64_t backpressureFallbacks = 0;
};

struct VideoCodecFrameTimingSnapshot {
    bool valid = false;
    std::uint64_t instanceId = 0;
    VideoCodecTimingDirection direction =
        VideoCodecTimingDirection::kEncode;
    std::string implementation;
    std::uint32_t rtpTimestamp = 0;
    std::uint64_t latestFrameDurationUs = 0;
    std::uint64_t averageFrameDurationUs = 0;
    std::uint64_t p95FrameDurationUs = 0;
    std::uint64_t maximumFrameDurationUs = 0;
    std::uint32_t frameWidth = 0;
    std::uint32_t frameHeight = 0;
    std::uint64_t encodedBytes = 0;
    bool qpAvailable = false;
    std::int32_t qp = 0;
    std::int64_t completedAtSteadyUs = 0;
    std::uint64_t completedFrames = 0;
    std::uint64_t droppedFrames = 0;
    VideoDecodePipelineTimingSnapshot decodePipeline;
};

class VideoCodecTimingRegistry final {
public:
    static VideoCodecTimingRegistry& Instance()
    {
        static VideoCodecTimingRegistry registry;
        return registry;
    }

    std::uint64_t Register(VideoCodecTimingDirection direction)
    {
        std::lock_guard lock(mutex_);
        const std::uint64_t id = nextInstanceId_++;
        auto& snapshot = instances_[id];
        snapshot.instanceId = id;
        snapshot.direction = direction;
        return id;
    }

    void Unregister(std::uint64_t id)
    {
        std::lock_guard lock(mutex_);
        instances_.erase(id);
        timingWindows_.erase(id);
    }

    void RecordCompleted(
        std::uint64_t id,
        std::string implementation,
        std::uint32_t rtpTimestamp,
        std::uint64_t durationUs,
        std::uint32_t frameWidth,
        std::uint32_t frameHeight,
        std::uint64_t encodedBytes,
        std::optional<std::int32_t> qp)
    {
        std::lock_guard lock(mutex_);
        const auto found = instances_.find(id);
        if (found == instances_.end()) {
            return;
        }
        auto& snapshot = found->second;
        snapshot.valid = true;
        snapshot.implementation = std::move(implementation);
        snapshot.rtpTimestamp = rtpTimestamp;
        snapshot.latestFrameDurationUs = durationUs;
        auto& windows = timingWindows_[id];
        PushDuration(windows.frameDurationsUs, durationUs);
        snapshot.averageFrameDurationUs = Average(
            windows.frameDurationsUs);
        snapshot.p95FrameDurationUs = Percentile(
            windows.frameDurationsUs, 0.95);
        snapshot.maximumFrameDurationUs = Maximum(
            windows.frameDurationsUs);
        snapshot.frameWidth = frameWidth;
        snapshot.frameHeight = frameHeight;
        snapshot.encodedBytes = encodedBytes;
        snapshot.qpAvailable = qp.has_value();
        snapshot.qp = qp.value_or(0);
        snapshot.completedAtSteadyUs = SteadyNowUs();
        if (snapshot.direction == VideoCodecTimingDirection::kDecode) {
            auto& pending = PendingDecodePipelineTiming();
            if (pending && pending->first == rtpTimestamp) {
                snapshot.decodePipeline = pending->second;
                PushDuration(windows.transformWaitDurationsUs,
                    snapshot.decodePipeline.transformWaitUs);
                snapshot.decodePipeline.averageTransformWaitUs =
                    Average(windows.transformWaitDurationsUs);
                snapshot.decodePipeline.p95TransformWaitUs =
                    Percentile(
                        windows.transformWaitDurationsUs, 0.95);
                snapshot.decodePipeline.maximumTransformWaitUs =
                    Maximum(windows.transformWaitDurationsUs);
                pending.reset();
            } else {
                snapshot.decodePipeline = {};
            }
        }
        ++snapshot.completedFrames;
    }

    // A hardware decoder calls this immediately before invoking WebRTC's
    // decoded-frame callback. The timing wrapper consumes it synchronously on
    // the same thread, so no decoder/factory/UI coupling or RTP-global lookup
    // is needed.
    static void StageDecodePipelineTiming(
        std::uint32_t rtpTimestamp,
        VideoDecodePipelineTimingSnapshot timing)
    {
        timing.available = true;
        PendingDecodePipelineTiming() =
            std::make_pair(rtpTimestamp, std::move(timing));
    }

    void RecordDropped(std::uint64_t id)
    {
        std::lock_guard lock(mutex_);
        const auto found = instances_.find(id);
        if (found != instances_.end()) {
            ++found->second.droppedFrames;
        }
    }

    std::optional<VideoCodecFrameTimingSnapshot>
    SnapshotForImplementation(const std::string& implementation) const
    {
        const auto id = ExtractInstanceId(implementation);
        if (!id) {
            return std::nullopt;
        }
        std::lock_guard lock(mutex_);
        const auto found = instances_.find(*id);
        if (found == instances_.end() || !found->second.valid) {
            return std::nullopt;
        }
        return found->second;
    }

    static std::string TaggedImplementation(
        std::string implementation,
        std::uint64_t id)
    {
        if (implementation.empty()) {
            implementation = "Unknown video codec";
        }
        return implementation + kTagPrefix + std::to_string(id);
    }

    static std::string UntaggedImplementation(
        const std::string& implementation)
    {
        const auto position = implementation.rfind(kTagPrefix);
        return position == std::string::npos
                   ? implementation
                   : implementation.substr(0, position);
    }

    static std::int64_t SteadyNowUs()
    {
        return std::chrono::duration_cast<std::chrono::microseconds>(
                   std::chrono::steady_clock::now().time_since_epoch())
            .count();
    }

private:
    static constexpr const char* kTagPrefix =
        " | RemoteC timing #";
    static constexpr std::size_t kTimingWindowSize = 240;

    struct TimingWindows {
        std::deque<std::uint64_t> frameDurationsUs;
        std::deque<std::uint64_t> transformWaitDurationsUs;
    };

    static void PushDuration(
        std::deque<std::uint64_t>& values,
        std::uint64_t value)
    {
        values.push_back(value);
        while (values.size() > kTimingWindowSize) {
            values.pop_front();
        }
    }

    static std::uint64_t Average(
        const std::deque<std::uint64_t>& values)
    {
        if (values.empty()) {
            return 0;
        }
        long double total = 0.0;
        for (const auto value : values) {
            total += value;
        }
        return static_cast<std::uint64_t>(
            total / static_cast<long double>(values.size()));
    }

    static std::uint64_t Percentile(
        const std::deque<std::uint64_t>& values,
        double percentile)
    {
        if (values.empty()) {
            return 0;
        }
        std::vector<std::uint64_t> sorted(values.begin(), values.end());
        std::sort(sorted.begin(), sorted.end());
        const std::size_t index = static_cast<std::size_t>(std::ceil(
            percentile * static_cast<double>(sorted.size()))) - 1;
        return sorted[(std::min)(index, sorted.size() - 1)];
    }

    static std::uint64_t Maximum(
        const std::deque<std::uint64_t>& values)
    {
        return values.empty()
                   ? 0
                   : *std::max_element(values.begin(), values.end());
    }

    static std::optional<std::uint64_t> ExtractInstanceId(
        const std::string& implementation)
    {
        const auto position = implementation.rfind(kTagPrefix);
        if (position == std::string::npos) {
            return std::nullopt;
        }
        const std::string text =
            implementation.substr(position + std::char_traits<char>::length(
                                             kTagPrefix));
        if (text.empty()) {
            return std::nullopt;
        }
        try {
            return std::stoull(text);
        } catch (...) {
            return std::nullopt;
        }
    }

    static std::optional<std::pair<
        std::uint32_t,
        VideoDecodePipelineTimingSnapshot>>&
    PendingDecodePipelineTiming()
    {
        static thread_local std::optional<std::pair<
            std::uint32_t,
            VideoDecodePipelineTimingSnapshot>> pending;
        return pending;
    }

    mutable std::mutex mutex_;
    std::uint64_t nextInstanceId_ = 1;
    std::unordered_map<
        std::uint64_t,
        VideoCodecFrameTimingSnapshot> instances_;
    std::unordered_map<std::uint64_t, TimingWindows> timingWindows_;
};

}  // namespace remote
