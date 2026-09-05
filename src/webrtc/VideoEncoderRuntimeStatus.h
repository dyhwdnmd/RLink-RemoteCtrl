// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#pragma once

#include <chrono>
#include <cstdint>
#include <mutex>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

namespace remote {

enum class VideoEncoderPreference {
    kAutomatic,
    kHardwareOnly,
    kFfmpegHardware,
    kSoftwareOnly,
    kFfmpegX264Only,
};

enum class FfmpegHardwareBackend {
    kAutomatic,
    kQsv,
    kNvenc,
    kAmf,
};

inline const char* FfmpegHardwareBackendName(
    FfmpegHardwareBackend backend)
{
    switch (backend) {
    case FfmpegHardwareBackend::kQsv:
        return "qsv";
    case FfmpegHardwareBackend::kNvenc:
        return "nvenc";
    case FfmpegHardwareBackend::kAmf:
        return "amf";
    case FfmpegHardwareBackend::kAutomatic:
    default:
        return "auto";
    }
}

enum class FfmpegX264Preset {
    kUltraFast,
    kSuperFast,
    kVeryFast,
    kFaster,
    kFast,
    kMedium,
    kSlow,
    kSlower,
    kVerySlow,
};

inline const char* FfmpegX264PresetName(FfmpegX264Preset preset)
{
    switch (preset) {
    case FfmpegX264Preset::kUltraFast:
        return "ultrafast";
    case FfmpegX264Preset::kSuperFast:
        return "superfast";
    case FfmpegX264Preset::kVeryFast:
        return "veryfast";
    case FfmpegX264Preset::kFaster:
        return "faster";
    case FfmpegX264Preset::kFast:
        return "fast";
    case FfmpegX264Preset::kMedium:
        return "medium";
    case FfmpegX264Preset::kSlow:
        return "slow";
    case FfmpegX264Preset::kSlower:
        return "slower";
    case FfmpegX264Preset::kVerySlow:
        return "veryslow";
    }
    return "medium";
}

enum class OpenH264QualityComplexity {
    kLow,
    kMedium,
    kHigh,
};

struct VideoEncoderQualityProfile {
    const char* displayName;
    const char* x264Preset;
    const char* nvencPreset;
    const char* qsvPreset;
    const char* amfQuality;
    std::uint32_t mfQualityVsSpeed;
    OpenH264QualityComplexity openH264Complexity;
};

// The public setting has five levels. Values from the former nine-level
// libx264-only selector are deliberately folded into the nearest shared level
// so every backend observes one authoritative mapping.
inline VideoEncoderQualityProfile VideoEncoderQualityProfileForPreset(
    FfmpegX264Preset preset)
{
    switch (preset) {
    case FfmpegX264Preset::kVerySlow:
        return {"very_high", "veryslow", "p7", "veryslow", "quality",
                100, OpenH264QualityComplexity::kHigh};
    case FfmpegX264Preset::kSlower:
    case FfmpegX264Preset::kSlow:
        return {"high", "slow", "p6", "slow", "quality",
                75, OpenH264QualityComplexity::kHigh};
    case FfmpegX264Preset::kMedium:
        return {"medium", "medium", "p4", "medium", "balanced",
                50, OpenH264QualityComplexity::kMedium};
    case FfmpegX264Preset::kFaster:
    case FfmpegX264Preset::kFast:
    case FfmpegX264Preset::kVeryFast:
        return {"low", "veryfast", "p2", "faster", "speed",
                25, OpenH264QualityComplexity::kLow};
    case FfmpegX264Preset::kSuperFast:
    case FfmpegX264Preset::kUltraFast:
        return {"very_low", "ultrafast", "p1", "veryfast", "speed",
                0, OpenH264QualityComplexity::kLow};
    }
    return {"medium", "medium", "p4", "medium", "balanced",
            50, OpenH264QualityComplexity::kMedium};
}

struct VideoEncoderInstanceRuntimeStatus {
    std::uint64_t instanceId = 0;
    std::string implementation;
    std::string inputFormat;
    std::string state;
    std::string fallbackReason;
    std::uint32_t width = 0;
    std::uint32_t height = 0;
    std::uint32_t frameRate = 0;
    std::uint32_t configuredFrameRate = 0;
    std::uint32_t observedInputFrameRate = 0;
    std::uint32_t observedOutputFrameRate = 0;
    std::uint64_t totalInputFrames = 0;
    std::uint64_t totalOutputFrames = 0;
    std::uint64_t totalDroppedFrames = 0;
    std::uint32_t targetBitrateBps = 0;
    std::uint32_t configuredBitrateBps = 0;
    std::uint32_t initMinBitrateBps = 0;
    std::uint32_t initStartBitrateBps = 0;
    std::uint32_t initMaxBitrateBps = 0;
    std::uint32_t initMaxFrameRate = 0;
    std::uint64_t rateUpdateSequence = 0;
    std::uint64_t lastRateUpdateUnixMs = 0;
    bool bitrateConfigurationAccepted = false;
    bool bitrateReadbackAvailable = false;
};

struct VideoEncoderRuntimeSnapshot {
    VideoEncoderPreference preference =
        VideoEncoderPreference::kAutomatic;
    std::vector<VideoEncoderInstanceRuntimeStatus> instances;
    std::string lastFallbackReason;
};

// Shared by the runtime, encoder factory and Media Foundation encoders.
// WebRTC creates and drives encoders on worker threads, so the UI must only
// consume immutable snapshots from this synchronized state object.
class VideoEncoderRuntimeState final {
public:
    explicit VideoEncoderRuntimeState(VideoEncoderPreference preference)
        : preference_(preference)
    {}

    std::uint64_t RegisterHardwareEncoder(
        std::string implementation = "MediaFoundationD3D11H264")
    {
        std::lock_guard lock(mutex_);
        const std::uint64_t id = nextInstanceId_++;
        instances_.emplace(
            id,
            VideoEncoderInstanceRuntimeStatus{
                .instanceId = id,
                .implementation = std::move(implementation),
                .state = "hardware_created",
            });
        return id;
    }

    std::uint64_t RegisterSoftwareEncoder(std::string implementation)
    {
        std::lock_guard lock(mutex_);
        const std::uint64_t id = nextInstanceId_++;
        instances_.emplace(
            id,
            VideoEncoderInstanceRuntimeStatus{
                .instanceId = id,
                .implementation = implementation.empty()
                                      ? "Builtin/OpenH264"
                                      : std::move(implementation),
                .state = "software_active",
            });
        return id;
    }

    void UnregisterEncoder(std::uint64_t id)
    {
        std::lock_guard lock(mutex_);
        instances_.erase(id);
    }

    void MarkHardwareInitialized(
        std::uint64_t id,
        std::uint32_t width,
        std::uint32_t height,
        std::uint32_t frameRate,
        std::uint32_t minBitrateBps,
        std::uint32_t startBitrateBps,
        std::uint32_t maxBitrateBps)
    {
        std::lock_guard lock(mutex_);
        auto& status = instances_[id];
        status.instanceId = id;
        if (status.implementation.empty()) {
            status.implementation = "MediaFoundationD3D11H264";
        }
        status.state = "hardware_active";
        status.fallbackReason.clear();
        status.width = width;
        status.height = height;
        status.frameRate = frameRate;
        status.configuredFrameRate = frameRate;
        status.targetBitrateBps = startBitrateBps;
        status.initMinBitrateBps = minBitrateBps;
        status.initStartBitrateBps = startBitrateBps;
        status.initMaxBitrateBps = maxBitrateBps;
        status.initMaxFrameRate = frameRate;
    }

    void MarkInputFormat(std::uint64_t id, std::string inputFormat)
    {
        std::lock_guard lock(mutex_);
        auto found = instances_.find(id);
        if (found != instances_.end()) {
            found->second.inputFormat = std::move(inputFormat);
        }
    }

    void MarkRates(
        std::uint64_t id,
        std::uint32_t frameRate,
        std::uint32_t targetBitrateBps,
        std::uint32_t configuredFrameRate = 0,
        std::uint32_t observedInputFrameRate = 0,
        std::uint32_t configuredBitrateBps = 0,
        bool bitrateConfigurationAccepted = false,
        bool bitrateReadbackAvailable = false)
    {
        std::lock_guard lock(mutex_);
        auto found = instances_.find(id);
        if (found != instances_.end()) {
            found->second.frameRate = frameRate;
            found->second.configuredFrameRate =
                configuredFrameRate != 0
                    ? configuredFrameRate
                    : frameRate;
            found->second.observedInputFrameRate =
                observedInputFrameRate;
            found->second.targetBitrateBps = targetBitrateBps;
            found->second.configuredBitrateBps =
                configuredBitrateBps;
            found->second.bitrateConfigurationAccepted =
                bitrateConfigurationAccepted;
            found->second.bitrateReadbackAvailable =
                bitrateReadbackAvailable;
            ++found->second.rateUpdateSequence;
            found->second.lastRateUpdateUnixMs =
                static_cast<std::uint64_t>(
                    std::chrono::duration_cast<
                        std::chrono::milliseconds>(
                        std::chrono::system_clock::now()
                            .time_since_epoch())
                        .count());
        }
    }

    void MarkObservedFrameRate(
        std::uint64_t id,
        std::uint32_t observedInputFrameRate,
        std::uint32_t configuredFrameRate)
    {
        std::lock_guard lock(mutex_);
        auto found = instances_.find(id);
        if (found != instances_.end()) {
            found->second.observedInputFrameRate =
                observedInputFrameRate;
            found->second.configuredFrameRate =
                configuredFrameRate;
        }
    }

    void MarkFramePipeline(
        std::uint64_t id,
        std::uint32_t observedInputFrameRate,
        std::uint32_t observedOutputFrameRate,
        std::uint64_t totalInputFrames,
        std::uint64_t totalOutputFrames,
        std::uint64_t totalDroppedFrames)
    {
        std::lock_guard lock(mutex_);
        auto found = instances_.find(id);
        if (found == instances_.end()) {
            return;
        }
        found->second.observedInputFrameRate =
            observedInputFrameRate;
        found->second.observedOutputFrameRate =
            observedOutputFrameRate;
        found->second.totalInputFrames = totalInputFrames;
        found->second.totalOutputFrames = totalOutputFrames;
        found->second.totalDroppedFrames = totalDroppedFrames;
    }

    void MarkSoftwareInitialized(
        std::uint64_t id,
        std::string implementation,
        std::uint32_t width,
        std::uint32_t height,
        std::uint32_t frameRate,
        std::uint32_t minBitrateBps,
        std::uint32_t startBitrateBps,
        std::uint32_t maxBitrateBps)
    {
        std::lock_guard lock(mutex_);
        auto& status = instances_[id];
        status.instanceId = id;
        status.implementation = implementation.empty()
                                    ? "Builtin/OpenH264"
                                    : std::move(implementation);
        status.state = "software_active";
        status.width = width;
        status.height = height;
        status.frameRate = frameRate;
        status.configuredFrameRate = frameRate;
        status.targetBitrateBps = startBitrateBps;
        status.initMinBitrateBps = minBitrateBps;
        status.initStartBitrateBps = startBitrateBps;
        status.initMaxBitrateBps = maxBitrateBps;
        status.initMaxFrameRate = frameRate;
    }

    void MarkHardwareStopped(std::uint64_t id)
    {
        std::lock_guard lock(mutex_);
        auto found = instances_.find(id);
        if (found != instances_.end() &&
            found->second.state == "hardware_active") {
            found->second.state = "hardware_stopped";
        }
    }

    void MarkSoftwareFallback(std::uint64_t id, std::string reason)
    {
        std::lock_guard lock(mutex_);
        auto& status = instances_[id];
        status.instanceId = id;
        status.implementation = "Builtin/OpenH264";
        status.state = "software_fallback";
        status.fallbackReason = reason;
        lastFallbackReason_ = std::move(reason);
    }

    VideoEncoderRuntimeSnapshot Snapshot() const
    {
        std::lock_guard lock(mutex_);
        VideoEncoderRuntimeSnapshot snapshot;
        snapshot.preference = preference_;
        snapshot.lastFallbackReason = lastFallbackReason_;
        snapshot.instances.reserve(instances_.size());
        for (const auto& [id, status] : instances_) {
            (void)id;
            snapshot.instances.push_back(status);
        }
        return snapshot;
    }

    void SetPreference(VideoEncoderPreference preference)
    {
        std::lock_guard lock(mutex_);
        preference_ = preference;
    }

    void SetDesktopShareGeneration(std::uint64_t generation)
    {
        std::lock_guard lock(mutex_);
        desktopShareGeneration_ = generation;
    }

    std::uint64_t DesktopShareGeneration() const
    {
        std::lock_guard lock(mutex_);
        return desktopShareGeneration_;
    }

    void SetDesktopCaptureAdapterLuid(std::uint64_t adapterLuid)
    {
        std::lock_guard lock(mutex_);
        desktopCaptureAdapterLuid_ = adapterLuid;
    }

    std::uint64_t DesktopCaptureAdapterLuid() const
    {
        std::lock_guard lock(mutex_);
        return desktopCaptureAdapterLuid_;
    }

private:
    mutable std::mutex mutex_;
    VideoEncoderPreference preference_;
    std::uint64_t nextInstanceId_ = 1;
    std::uint64_t desktopShareGeneration_ = 0;
    std::uint64_t desktopCaptureAdapterLuid_ = 0;
    std::unordered_map<
        std::uint64_t,
        VideoEncoderInstanceRuntimeStatus> instances_;
    std::string lastFallbackReason_;
};

}  // namespace remote
