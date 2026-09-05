// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#pragma once

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <cstdint>
#include <functional>
#include <memory>
#include <mutex>
#include <optional>
#include <stop_token>
#include <string>
#include <thread>

#include "api/video/adapted_video_track_source.h"
#include "modules/desktop_capture/desktop_capturer.h"
#include "src/core/DisplayTopology.h"

namespace remote {

enum class DesktopCaptureImplementation : std::uint8_t {
    kLibWebRtc,
    kNativeDxgi,
};

// Captures one Windows display on a dedicated thread. WebRTC selects DXGI
// Desktop Duplication once when the capturer is created and keeps GDI as the
// permanent-error fallback; there is no per-frame capability probing.
class WindowsDesktopCaptureSource
    : public webrtc::AdaptedVideoTrackSource,
      private webrtc::DesktopCapturer::Callback {
public:
    enum class CaptureActivityState : std::uint8_t {
        kStarting,
        kActive,
        kIdle,
    };

    struct RuntimeStats {
        std::uint32_t targetFrameRate = 0;
        CaptureActivityState activityState =
            CaptureActivityState::kStarting;
        bool adaptiveFrameDeliveryEnabled = false;
        double captureAttemptsPerSecond = 0.0;
        double deliveredFramesPerSecond = 0.0;
        double changedFramesPerSecond = 0.0;
        double idleHeartbeatFramesPerSecond = 0.0;
        std::uint64_t totalCaptureAttempts = 0;
        std::uint64_t totalDeliveredFrames = 0;
        std::uint64_t totalChangedFrames = 0;
        std::uint64_t totalIdleHeartbeatFrames = 0;
        std::uint64_t totalSuppressedUnchangedFrames = 0;
        std::uint64_t totalActivityTransitions = 0;
        std::uint64_t totalFailedCaptures = 0;
        bool inputBoostActive = false;
        std::uint64_t totalInputBoosts = 0;
        std::uint64_t totalForcedRefreshFrames = 0;
        double latestCaptureCallMs = 0.0;
    };

    enum class CaptureBackend {
        kDxgiNativeTexture,
        kDxgiPreferred,
        kGdi
    };

    explicit WindowsDesktopCaptureSource(
        DesktopCaptureImplementation implementation =
            DesktopCaptureImplementation::kNativeDxgi,
        DisplayDescriptor captureTarget = {});
    ~WindowsDesktopCaptureSource() override;

    bool StartCapture(
        std::chrono::milliseconds firstFrameTimeout =
            std::chrono::seconds(3));
    void StopCapture();
    bool SetTargetFrameRate(std::uint32_t framesPerSecond);
    // Chrome Remote Desktop-style short capture boost. The caller invokes
    // this only after the remote input has been injected into Windows.
    void NotifyRemoteInputActivity();
    // Forces the next successful capture to be delivered as a full update.
    // Used after ICE recovery and sender/track reactivation.
    void RequestRefreshFrame();
    // Delivers a short, bounded run of full frames after a new sender or sink
    // becomes active. This gives WebRTC enough stable input for its first
    // keyframe and startup bandwidth probe without changing the steady-state
    // desktop activity policy.
    void RequestStartupFrameBurst(
        std::uint32_t frameCount = 6,
        std::function<void()> firstDeliveredFrameCallback = {},
        std::function<void()> burstCompletedCallback = {});
    std::uint32_t TargetFrameRate() const noexcept;
    std::uint32_t CapturedWidth() const noexcept;
    std::uint32_t CapturedHeight() const noexcept;
    RuntimeStats CaptureRuntimeStats() const noexcept;

    DesktopCaptureImplementation ConfiguredImplementation() const noexcept;
    CaptureBackend Backend() const;
    std::string FallbackReason() const;
    std::string LastError() const;
    const DisplayDescriptor& CaptureTarget() const noexcept;

    SourceState state() const override;
    bool remote() const override;
    bool is_screencast() const override;
    std::optional<bool> needs_denoising() const override;

private:
    enum class FrameDeliveryReason : std::uint8_t {
        kInitial,
        kDesktopChanged,
        kStartupPrime,
        kScheduledRepeat,
        kIdleHeartbeat,
        kForcedRefresh,
    };

    struct FrameUpdateRegion {
        int offsetX = 0;
        int offsetY = 0;
        int width = 0;
        int height = 0;
    };

    void CaptureLoop(std::stop_token stopToken);
    void OnCaptureResult(
        webrtc::DesktopCapturer::Result result,
        std::unique_ptr<webrtc::DesktopFrame> frame) override;
    void SetInitializationFailure(std::string message);
    void ResetActivityTracking();
    bool ShouldDeliverFrame(
        bool desktopChanged,
        bool forceRefresh,
        std::chrono::steady_clock::time_point now,
        FrameDeliveryReason* reason);
    bool ShouldDeliverLibWebRtcFrame(
        bool desktopChanged,
        bool forceRefresh,
        std::chrono::steady_clock::time_point now,
        FrameDeliveryReason* reason);
    void DeliverFrame(
        webrtc::scoped_refptr<webrtc::VideoFrameBuffer> buffer,
        FrameDeliveryReason reason);
    void DeliverLibWebRtcFrame(
        webrtc::scoped_refptr<webrtc::VideoFrameBuffer> buffer,
        FrameDeliveryReason reason,
        const FrameUpdateRegion& updateRegion,
        bool repeatFrame);
    bool ConsumeForcedRefreshFrame();
    void ScheduleForcedRefreshFrames(std::uint32_t frameCount);
    void SignalCaptureSchedule();

    mutable std::mutex mutex_;
    std::condition_variable firstFrameCondition_;
    std::jthread captureThread_;
    const DesktopCaptureImplementation configuredImplementation_;
    const DisplayDescriptor captureTarget_;
    CaptureBackend backend_ = CaptureBackend::kGdi;
    bool initializationFinished_ = false;
    bool firstFrameReady_ = false;
    bool running_ = false;
    std::string fallbackReason_;
    std::string lastError_;
    std::atomic<std::uint32_t> targetFrameRate_{60};
    std::atomic<CaptureActivityState> activityState_{
        CaptureActivityState::kStarting};
    std::atomic<bool> adaptiveFrameDeliveryEnabled_{false};
    std::atomic<std::uint32_t> capturedWidth_{0};
    std::atomic<std::uint32_t> capturedHeight_{0};
    std::atomic<std::uint64_t> totalCaptureAttempts_{0};
    std::atomic<std::uint64_t> totalDeliveredFrames_{0};
    std::atomic<std::uint64_t> totalChangedFrames_{0};
    std::atomic<std::uint64_t> totalIdleHeartbeatFrames_{0};
    std::atomic<std::uint64_t> totalSuppressedUnchangedFrames_{0};
    std::atomic<std::uint64_t> totalActivityTransitions_{0};
    std::atomic<std::uint64_t> totalFailedCaptures_{0};
    std::atomic<std::uint64_t> totalInputBoosts_{0};
    std::atomic<std::uint64_t> totalForcedRefreshFrames_{0};
    std::atomic<std::uint64_t> captureAttemptsPerSecondMilli_{0};
    std::atomic<std::uint64_t> deliveredFramesPerSecondMilli_{0};
    std::atomic<std::uint64_t> changedFramesPerSecondMilli_{0};
    std::atomic<std::uint64_t> idleHeartbeatFramesPerSecondMilli_{0};
    std::atomic<std::uint64_t> latestCaptureCallUs_{0};
    std::atomic<std::int64_t> inputBoostUntilSteadyUs_{0};
    std::atomic<std::int64_t> startupPrimeUntilSteadyUs_{0};
    std::atomic<std::uint32_t> forcedRefreshFramesRemaining_{0};
    // HANDLE is kept opaque in the header. It is created and destroyed by the
    // active capture thread while mutex_ protects publication/lifetime.
    void* captureScheduleWakeEvent_ = nullptr;
    bool activityHasDeliveredFrame_ = false;
    std::chrono::steady_clock::time_point activityLastChangedAt_{};
    std::chrono::steady_clock::time_point activityLastDeliveredAt_{};
    std::function<void()> startupFrameDeliveredCallback_;
    std::function<void()> startupBurstCompletedCallback_;
    std::uint32_t startupCallbackFramesRemaining_ = 0;
};

}  // namespace remote
