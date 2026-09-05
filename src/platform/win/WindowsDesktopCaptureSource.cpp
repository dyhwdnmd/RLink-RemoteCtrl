// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#include "WindowsDesktopCaptureSource.h"

#include <Windows.h>

#include <algorithm>
#include <intrin.h>
#include <utility>

#include "api/make_ref_counted.h"
#include "api/video/video_frame.h"
#include "modules/desktop_capture/desktop_capture_options.h"
#include "modules/desktop_capture/desktop_frame.h"
#include "modules/desktop_capture/win/screen_capturer_win_directx.h"
#include "rtc_base/time_utils.h"
#include "D3D11DesktopFrameBuffer.h"
#include "DesktopBgraFrameBuffer.h"
#include "DxgiNativeDesktopCapturer.h"
#include "WindowsDisplayTopology.h"

namespace remote {
namespace {

constexpr std::uint32_t kMinimumCaptureFrameRate = 5;
constexpr std::uint32_t kMaximumCaptureFrameRate = 120;
constexpr auto kIdleTransitionDelay = std::chrono::milliseconds(500);
constexpr auto kLibWebRtcKeepAliveInterval = std::chrono::seconds(2);
constexpr auto kInputBoostDuration = std::chrono::milliseconds(50);
constexpr auto kStartupPrimeDuration = std::chrono::seconds(2);
constexpr auto kCaptureSpinThreshold =
    std::chrono::microseconds(200);

std::int64_t SteadyNowUs()
{
    return std::chrono::duration_cast<std::chrono::microseconds>(
               std::chrono::steady_clock::now().time_since_epoch())
        .count();
}

std::string Utf8FromWide(const wchar_t* text)
{
    if (!text || *text == L'\0') {
        return {};
    }
    const int required = WideCharToMultiByte(
        CP_UTF8, 0, text, -1, nullptr, 0, nullptr, nullptr);
    if (required <= 1) {
        return {};
    }
    std::string result(static_cast<std::size_t>(required), '\0');
    WideCharToMultiByte(
        CP_UTF8, 0, text, -1, result.data(), required,
        nullptr, nullptr);
    result.pop_back();
    return result;
}

// libwebrtc's Windows screen SourceId is the device_index passed to
// EnumDisplayDevicesW(nullptr, device_index, ...). It is not an HMONITOR.
// Keep the persisted/display-topology identity based on the device name and
// translate it to libwebrtc's process-local source id only at capture startup.
std::optional<webrtc::DesktopCapturer::SourceId>
FindLibWebRtcScreenSource(
    const DisplayDescriptor& target,
    const webrtc::DesktopCapturer::SourceList& sources)
{
    std::optional<webrtc::DesktopCapturer::SourceId> primarySource;
    for (DWORD deviceIndex = 0;; ++deviceIndex) {
        DISPLAY_DEVICEW device{};
        device.cb = sizeof(device);
        if (!EnumDisplayDevicesW(
                nullptr, deviceIndex, &device, 0)) {
            break;
        }
        if ((device.StateFlags & DISPLAY_DEVICE_ACTIVE) == 0) {
            continue;
        }

        const auto source = std::find_if(
            sources.begin(), sources.end(),
            [deviceIndex](
                const webrtc::DesktopCapturer::Source& candidate) {
                return candidate.id ==
                       static_cast<
                           webrtc::DesktopCapturer::SourceId>(
                           deviceIndex);
            });
        if (source == sources.end()) {
            continue;
        }
        if ((device.StateFlags &
             DISPLAY_DEVICE_PRIMARY_DEVICE) != 0) {
            primarySource = source->id;
        }
        if (!target.deviceName.empty() &&
            Utf8FromWide(device.DeviceName) ==
                target.deviceName) {
            return source->id;
        }
    }

    if (target.primary && primarySource) {
        return primarySource;
    }
    if (target.deviceName.empty() && sources.size() == 1) {
        return sources.front().id;
    }
    return std::nullopt;
}

class UniqueWinHandle {
public:
    explicit UniqueWinHandle(HANDLE handle = nullptr) noexcept
        : handle_(handle)
    {
    }

    ~UniqueWinHandle()
    {
        if (handle_) {
            CloseHandle(handle_);
        }
    }

    UniqueWinHandle(const UniqueWinHandle&) = delete;
    UniqueWinHandle& operator=(const UniqueWinHandle&) = delete;

    UniqueWinHandle(UniqueWinHandle&& other) noexcept
        : handle_(std::exchange(other.handle_, nullptr))
    {
    }

    UniqueWinHandle& operator=(UniqueWinHandle&& other) noexcept
    {
        if (this != &other) {
            if (handle_) {
                CloseHandle(handle_);
            }
            handle_ = std::exchange(other.handle_, nullptr);
        }
        return *this;
    }

    HANDLE get() const noexcept
    {
        return handle_;
    }

private:
    HANDLE handle_ = nullptr;
};

UniqueWinHandle CreateCaptureTimer()
{
    HANDLE timer = CreateWaitableTimerExW(
        nullptr,
        nullptr,
        CREATE_WAITABLE_TIMER_HIGH_RESOLUTION,
        TIMER_MODIFY_STATE | SYNCHRONIZE);
    if (!timer) {
        // CREATE_WAITABLE_TIMER_HIGH_RESOLUTION is unavailable on older
        // Windows builds. A regular waitable timer remains a better coarse
        // wait primitive than repeated one-millisecond sleeps.
        timer = CreateWaitableTimerExW(
            nullptr,
            nullptr,
            0,
            TIMER_MODIFY_STATE | SYNCHRONIZE);
    }
    return UniqueWinHandle(timer);
}

enum class CaptureWaitResult {
    kDeadlineReached,
    kScheduleChanged,
    kStopped,
};

CaptureWaitResult WaitForLibWebRtcCaptureDeadline(
    HANDLE timer,
    HANDLE stopEvent,
    HANDLE scheduleWakeEvent,
    std::chrono::steady_clock::time_point deadline,
    std::stop_token stopToken)
{
    const auto coarseDeadline = deadline - kCaptureSpinThreshold;
    auto now = std::chrono::steady_clock::now();
    if (now < coarseDeadline) {
        const auto remaining100ns =
            (std::max<std::int64_t>)(
                1,
                std::chrono::duration_cast<std::chrono::nanoseconds>(
                    coarseDeadline - now)
                        .count() /
                    100);
        LARGE_INTEGER dueTime{};
        dueTime.QuadPart = -remaining100ns;
        if (timer &&
            SetWaitableTimerEx(
                timer, &dueTime, 0, nullptr, nullptr, nullptr, 0)) {
            const HANDLE waits[] = {
                stopEvent, scheduleWakeEvent, timer};
            const DWORD result =
                WaitForMultipleObjects(3, waits, FALSE, INFINITE);
            if (result == WAIT_OBJECT_0) {
                return CaptureWaitResult::kStopped;
            }
            if (result == WAIT_OBJECT_0 + 1) {
                CancelWaitableTimer(timer);
                return CaptureWaitResult::kScheduleChanged;
            }
        } else {
            const auto remainingMs =
                (std::max<std::int64_t>)(
                    1,
                    std::chrono::duration_cast<std::chrono::milliseconds>(
                        coarseDeadline - now)
                        .count());
            const HANDLE waits[] = {stopEvent, scheduleWakeEvent};
            const DWORD result = WaitForMultipleObjects(
                2,
                waits,
                FALSE,
                static_cast<DWORD>((std::min<std::int64_t>)(
                    remainingMs, MAXDWORD - 1)));
            if (result == WAIT_OBJECT_0) {
                return CaptureWaitResult::kStopped;
            }
            if (result == WAIT_OBJECT_0 + 1) {
                return CaptureWaitResult::kScheduleChanged;
            }
        }
    }

    while (!stopToken.stop_requested() &&
           std::chrono::steady_clock::now() < deadline) {
        if (scheduleWakeEvent &&
            WaitForSingleObject(scheduleWakeEvent, 0) ==
                WAIT_OBJECT_0) {
            return CaptureWaitResult::kScheduleChanged;
        }
        YieldProcessor();
    }
    return stopToken.stop_requested()
        ? CaptureWaitResult::kStopped
        : CaptureWaitResult::kDeadlineReached;
}

struct UpdateBounds {
    int offsetX = 0;
    int offsetY = 0;
    int width = 0;
    int height = 0;
};

UpdateBounds BoundingUpdateRegion(
    const webrtc::DesktopRegion& region,
    int frameWidth,
    int frameHeight)
{
    UpdateBounds result;
    bool hasRect = false;
    int left = frameWidth;
    int top = frameHeight;
    int right = 0;
    int bottom = 0;
    for (webrtc::DesktopRegion::Iterator it(region);
         !it.IsAtEnd();
         it.Advance()) {
        const auto& rect = it.rect();
        left = (std::min)(left, rect.left());
        top = (std::min)(top, rect.top());
        right = (std::max)(right, rect.right());
        bottom = (std::max)(bottom, rect.bottom());
        hasRect = true;
    }
    if (!hasRect) {
        return result;
    }
    left = std::clamp(left, 0, frameWidth);
    top = std::clamp(top, 0, frameHeight);
    right = std::clamp(right, left, frameWidth);
    bottom = std::clamp(bottom, top, frameHeight);
    result.offsetX = left;
    result.offsetY = top;
    result.width = right - left;
    result.height = bottom - top;
    return result;
}

}  // namespace

WindowsDesktopCaptureSource::WindowsDesktopCaptureSource(
    DesktopCaptureImplementation implementation,
    DisplayDescriptor captureTarget)
    : configuredImplementation_(implementation),
      captureTarget_(std::move(captureTarget))
{
}

WindowsDesktopCaptureSource::~WindowsDesktopCaptureSource()
{
    StopCapture();
}

bool WindowsDesktopCaptureSource::StartCapture(
    std::chrono::milliseconds firstFrameTimeout)
{
    std::unique_lock lock(mutex_);
    if (running_) {
        return firstFrameReady_;
    }

    initializationFinished_ = false;
    firstFrameReady_ = false;
    fallbackReason_.clear();
    lastError_.clear();
    ResetActivityTracking();
    running_ = true;
    captureThread_ = std::jthread(
        [this](std::stop_token stopToken) { CaptureLoop(stopToken); });

    const bool ready = firstFrameCondition_.wait_for(
        lock, firstFrameTimeout, [this] {
            return firstFrameReady_ ||
                   (initializationFinished_ && !lastError_.empty());
        });
    if (ready && firstFrameReady_) {
        return true;
    }
    if (lastError_.empty()) {
        lastError_ = "Timed out waiting for the first desktop frame.";
    }
    lock.unlock();
    StopCapture();
    return false;
}

void WindowsDesktopCaptureSource::StopCapture()
{
    std::jthread thread;
    {
        std::lock_guard lock(mutex_);
        if (!captureThread_.joinable()) {
            running_ = false;
            return;
        }
        captureThread_.request_stop();
        thread = std::move(captureThread_);
    }
    thread.join();
    std::lock_guard lock(mutex_);
    running_ = false;
}

bool WindowsDesktopCaptureSource::SetTargetFrameRate(
    std::uint32_t framesPerSecond)
{
    if (framesPerSecond < kMinimumCaptureFrameRate ||
        framesPerSecond > kMaximumCaptureFrameRate) {
        return false;
    }
    targetFrameRate_.store(framesPerSecond, std::memory_order_release);
    SignalCaptureSchedule();
    return true;
}

void WindowsDesktopCaptureSource::NotifyRemoteInputActivity()
{
    if (configuredImplementation_ !=
        DesktopCaptureImplementation::kLibWebRtc) {
        return;
    }
    const auto requestedUntil =
        SteadyNowUs() +
        std::chrono::duration_cast<std::chrono::microseconds>(
            kInputBoostDuration)
            .count();
    auto current =
        inputBoostUntilSteadyUs_.load(std::memory_order_acquire);
    while (current < requestedUntil &&
           !inputBoostUntilSteadyUs_.compare_exchange_weak(
               current,
               requestedUntil,
               std::memory_order_acq_rel,
               std::memory_order_acquire)) {
    }
    totalInputBoosts_.fetch_add(1, std::memory_order_relaxed);
    // During this bounded window ShouldDeliverLibWebRtcFrame allows a frame
    // through even if DWM has not yet exposed a changed region. The scheduler
    // is merely woken and still enforces the user-selected target rate.
    SignalCaptureSchedule();
}

void WindowsDesktopCaptureSource::RequestRefreshFrame()
{
    ScheduleForcedRefreshFrames(1);
}

void WindowsDesktopCaptureSource::RequestStartupFrameBurst(
    std::uint32_t frameCount,
    std::function<void()> firstDeliveredFrameCallback,
    std::function<void()> burstCompletedCallback)
{
    if (firstDeliveredFrameCallback || burstCompletedCallback) {
        std::lock_guard lock(mutex_);
        startupFrameDeliveredCallback_ =
            std::move(firstDeliveredFrameCallback);
        startupBurstCompletedCallback_ =
            std::move(burstCompletedCallback);
        startupCallbackFramesRemaining_ =
            std::clamp<std::uint32_t>(frameCount, 1u, 60u);
    }
    if (configuredImplementation_ ==
        DesktopCaptureImplementation::kLibWebRtc) {
        const auto requestedUntil =
            SteadyNowUs() +
            std::chrono::duration_cast<std::chrono::microseconds>(
                kStartupPrimeDuration).count();
        auto current = startupPrimeUntilSteadyUs_.load(
            std::memory_order_acquire);
        while (current < requestedUntil &&
               !startupPrimeUntilSteadyUs_.compare_exchange_weak(
                   current, requestedUntil,
                   std::memory_order_acq_rel,
                   std::memory_order_acquire)) {
        }
    }
    ScheduleForcedRefreshFrames(
        std::clamp<std::uint32_t>(frameCount, 1u, 12u));
}

void WindowsDesktopCaptureSource::ScheduleForcedRefreshFrames(
    std::uint32_t frameCount)
{
    auto current = forcedRefreshFramesRemaining_.load(
        std::memory_order_acquire);
    while (current < frameCount &&
           !forcedRefreshFramesRemaining_.compare_exchange_weak(
               current,
               frameCount,
               std::memory_order_acq_rel,
               std::memory_order_acquire)) {
    }
    SignalCaptureSchedule();
}

bool WindowsDesktopCaptureSource::ConsumeForcedRefreshFrame()
{
    auto remaining = forcedRefreshFramesRemaining_.load(
        std::memory_order_acquire);
    while (remaining > 0) {
        if (forcedRefreshFramesRemaining_.compare_exchange_weak(
                remaining,
                remaining - 1,
                std::memory_order_acq_rel,
                std::memory_order_acquire)) {
            return true;
        }
    }
    return false;
}

std::uint32_t WindowsDesktopCaptureSource::TargetFrameRate() const noexcept
{
    return targetFrameRate_.load(std::memory_order_acquire);
}

std::uint32_t WindowsDesktopCaptureSource::CapturedWidth() const noexcept
{
    return capturedWidth_.load(std::memory_order_acquire);
}

std::uint32_t WindowsDesktopCaptureSource::CapturedHeight() const noexcept
{
    return capturedHeight_.load(std::memory_order_acquire);
}

WindowsDesktopCaptureSource::RuntimeStats
WindowsDesktopCaptureSource::CaptureRuntimeStats() const noexcept
{
    RuntimeStats stats;
    stats.targetFrameRate = TargetFrameRate();
    stats.activityState =
        activityState_.load(std::memory_order_acquire);
    stats.adaptiveFrameDeliveryEnabled =
        adaptiveFrameDeliveryEnabled_.load(std::memory_order_acquire);
    stats.captureAttemptsPerSecond =
        static_cast<double>(captureAttemptsPerSecondMilli_.load(
            std::memory_order_acquire)) / 1000.0;
    stats.deliveredFramesPerSecond =
        static_cast<double>(deliveredFramesPerSecondMilli_.load(
            std::memory_order_acquire)) / 1000.0;
    stats.changedFramesPerSecond =
        static_cast<double>(changedFramesPerSecondMilli_.load(
            std::memory_order_acquire)) / 1000.0;
    stats.idleHeartbeatFramesPerSecond =
        static_cast<double>(idleHeartbeatFramesPerSecondMilli_.load(
            std::memory_order_acquire)) / 1000.0;
    stats.totalCaptureAttempts =
        totalCaptureAttempts_.load(std::memory_order_acquire);
    stats.totalDeliveredFrames =
        totalDeliveredFrames_.load(std::memory_order_acquire);
    stats.totalChangedFrames =
        totalChangedFrames_.load(std::memory_order_acquire);
    stats.totalIdleHeartbeatFrames =
        totalIdleHeartbeatFrames_.load(std::memory_order_acquire);
    stats.totalSuppressedUnchangedFrames =
        totalSuppressedUnchangedFrames_.load(std::memory_order_acquire);
    stats.totalActivityTransitions =
        totalActivityTransitions_.load(std::memory_order_acquire);
    stats.totalFailedCaptures =
        totalFailedCaptures_.load(std::memory_order_acquire);
    stats.inputBoostActive =
        configuredImplementation_ ==
            DesktopCaptureImplementation::kLibWebRtc &&
        SteadyNowUs() <
            inputBoostUntilSteadyUs_.load(std::memory_order_acquire);
    stats.totalInputBoosts =
        totalInputBoosts_.load(std::memory_order_acquire);
    stats.totalForcedRefreshFrames =
        totalForcedRefreshFrames_.load(std::memory_order_acquire);
    stats.latestCaptureCallMs =
        static_cast<double>(latestCaptureCallUs_.load(
            std::memory_order_acquire)) / 1000.0;
    return stats;
}

void WindowsDesktopCaptureSource::SignalCaptureSchedule()
{
    std::lock_guard lock(mutex_);
    if (captureScheduleWakeEvent_) {
        SetEvent(static_cast<HANDLE>(captureScheduleWakeEvent_));
    }
}

WindowsDesktopCaptureSource::CaptureBackend
WindowsDesktopCaptureSource::Backend() const
{
    std::lock_guard lock(mutex_);
    return backend_;
}

DesktopCaptureImplementation
WindowsDesktopCaptureSource::ConfiguredImplementation() const noexcept
{
    return configuredImplementation_;
}

std::string WindowsDesktopCaptureSource::FallbackReason() const
{
    std::lock_guard lock(mutex_);
    return fallbackReason_;
}

std::string WindowsDesktopCaptureSource::LastError() const
{
    std::lock_guard lock(mutex_);
    return lastError_;
}

const DisplayDescriptor&
WindowsDesktopCaptureSource::CaptureTarget() const noexcept
{
    return captureTarget_;
}

webrtc::MediaSourceInterface::SourceState
WindowsDesktopCaptureSource::state() const
{
    std::lock_guard lock(mutex_);
    return running_ ? kLive : kEnded;
}

bool WindowsDesktopCaptureSource::remote() const
{
    return false;
}

bool WindowsDesktopCaptureSource::is_screencast() const
{
    return true;
}

std::optional<bool> WindowsDesktopCaptureSource::needs_denoising() const
{
    return false;
}

void WindowsDesktopCaptureSource::CaptureLoop(std::stop_token stopToken)
{
    DisplayDescriptor captureTarget = captureTarget_;
    if (captureTarget.stableDisplayKey.empty()) {
        const auto topology = EnumerateWindowsDisplayTopology();
        if (const auto* primary = FindPrimaryDisplay(topology)) {
            captureTarget = *primary;
        }
    }
    if (captureTarget.stableDisplayKey.empty()) {
        SetInitializationFailure(
            "No Windows display is available for capture.");
        return;
    }
    const bool remoteDesktopSession =
        GetSystemMetrics(SM_REMOTESESSION) != 0;

    // The application-owned Desktop Duplication path keeps the desktop in a
    // D3D11 texture. When explicitly selected, initialization or runtime
    // failure falls through to libwebrtc's CPU BGRA capture path below.
    if (configuredImplementation_ ==
            DesktopCaptureImplementation::kNativeDxgi &&
        !remoteDesktopSession) {
        DxgiNativeDesktopCapturer nativeCapturer;
        if (nativeCapturer.Initialize(captureTarget)) {
            // The application-owned DXGI texture path deliberately follows
            // the selected fixed target rate. Static-frame suppression is a
            // libwebrtc-only policy.
            adaptiveFrameDeliveryEnabled_.store(
                false, std::memory_order_release);
            auto captureTimer = CreateCaptureTimer();
            UniqueWinHandle stopEvent(
                CreateEventW(nullptr, TRUE, FALSE, nullptr));
            UniqueWinHandle scheduleWakeEvent(
                CreateEventW(nullptr, FALSE, FALSE, nullptr));
            if (!stopEvent.get() || !scheduleWakeEvent.get()) {
                SetInitializationFailure(
                    "WebRTC could not create the native capture scheduler "
                    "events.");
                return;
            }
            {
                std::lock_guard lock(mutex_);
                captureScheduleWakeEvent_ = scheduleWakeEvent.get();
            }
            std::stop_callback stopWake(
                stopToken, [event = stopEvent.get()] {
                    if (event) {
                        SetEvent(event);
                    }
                });
            {
                std::lock_guard lock(mutex_);
                backend_ = CaptureBackend::kDxgiNativeTexture;
                initializationFinished_ = true;
            }
            auto rateWindowStartedAt =
                std::chrono::steady_clock::now();
            std::uint64_t windowAttempts = 0;
            std::uint64_t windowDelivered =
                totalDeliveredFrames_.load(
                    std::memory_order_acquire);
            std::uint64_t windowChanged =
                totalChangedFrames_.load(std::memory_order_acquire);
            std::uint64_t windowHeartbeats =
                totalIdleHeartbeatFrames_.load(
                    std::memory_order_acquire);
            bool nativeFailed = false;
            bool nativeHasFrame = false;
            auto nextFrameAt =
                std::chrono::steady_clock::now();
            while (!stopToken.stop_requested()) {
                const auto startedAt =
                    std::chrono::steady_clock::now();
                totalCaptureAttempts_.fetch_add(
                    1, std::memory_order_relaxed);
                ++windowAttempts;
                webrtc::scoped_refptr<D3D11DesktopFrameBuffer>
                    nativeFrame;
                const auto result = nativeCapturer.Capture(
                    nativeHasFrame ? 0u : 100u, &nativeFrame);
                if (result ==
                    DxgiNativeDesktopCapturer::Result::kFailed) {
                    nativeFailed = true;
                    break;
                }
                if (nativeFrame) {
                    nativeHasFrame = true;
                    capturedWidth_.store(
                        static_cast<std::uint32_t>(
                            nativeFrame->width()),
                        std::memory_order_release);
                    capturedHeight_.store(
                        static_cast<std::uint32_t>(
                            nativeFrame->height()),
                        std::memory_order_release);
                    const bool forceRefresh =
                        ConsumeForcedRefreshFrame();
                    const auto deliveryReason = forceRefresh
                        ? FrameDeliveryReason::kForcedRefresh
                        : result == DxgiNativeDesktopCapturer::Result::kSuccess
                            ? FrameDeliveryReason::kDesktopChanged
                            : FrameDeliveryReason::kScheduledRepeat;
                    if (forceRefresh) {
                        totalForcedRefreshFrames_.fetch_add(
                            1, std::memory_order_relaxed);
                    }
                    DeliverFrame(nativeFrame, deliveryReason);
                }

                const auto captureFinishedAt =
                    std::chrono::steady_clock::now();
                latestCaptureCallUs_.store(
                    static_cast<std::uint64_t>(
                        std::chrono::duration_cast<
                            std::chrono::microseconds>(
                            captureFinishedAt - startedAt)
                            .count()),
                    std::memory_order_release);
                const auto rateWindowElapsed =
                    std::chrono::duration_cast<
                        std::chrono::microseconds>(
                        captureFinishedAt -
                        rateWindowStartedAt);
                if (rateWindowElapsed >=
                    std::chrono::seconds(1)) {
                    const auto currentDelivered =
                        totalDeliveredFrames_.load(
                            std::memory_order_acquire);
                    const auto deliveredInWindow =
                        currentDelivered >= windowDelivered
                            ? currentDelivered - windowDelivered
                            : 0;
                    const auto currentChanged =
                        totalChangedFrames_.load(
                            std::memory_order_acquire);
                    const auto changedInWindow =
                        currentChanged >= windowChanged
                            ? currentChanged - windowChanged
                            : 0;
                    const auto currentHeartbeats =
                        totalIdleHeartbeatFrames_.load(
                            std::memory_order_acquire);
                    const auto heartbeatsInWindow =
                        currentHeartbeats >= windowHeartbeats
                            ? currentHeartbeats - windowHeartbeats
                            : 0;
                    const auto elapsedUs =
                        (std::max<std::int64_t>)(
                            rateWindowElapsed.count(), 1);
                    captureAttemptsPerSecondMilli_.store(
                        windowAttempts * 1'000'000'000ULL /
                            static_cast<std::uint64_t>(
                                elapsedUs),
                        std::memory_order_release);
                    deliveredFramesPerSecondMilli_.store(
                        deliveredInWindow *
                            1'000'000'000ULL /
                            static_cast<std::uint64_t>(
                                elapsedUs),
                        std::memory_order_release);
                    changedFramesPerSecondMilli_.store(
                        changedInWindow * 1'000'000'000ULL /
                            static_cast<std::uint64_t>(elapsedUs),
                        std::memory_order_release);
                    idleHeartbeatFramesPerSecondMilli_.store(
                        heartbeatsInWindow * 1'000'000'000ULL /
                            static_cast<std::uint64_t>(elapsedUs),
                        std::memory_order_release);
                    rateWindowStartedAt = captureFinishedAt;
                    windowAttempts = 0;
                    windowDelivered = currentDelivered;
                    windowChanged = currentChanged;
                    windowHeartbeats = currentHeartbeats;
                }

                const auto framesPerSecond =
                    targetFrameRate_.load(
                        std::memory_order_acquire);
                const auto interval =
                    std::chrono::microseconds(
                        1'000'000 /
                        (std::max)(framesPerSecond, 1u));
                // Keep an absolute cadence instead of rebuilding the deadline
                // from the end of every capture. This prevents sub-millisecond
                // wake-up error from accumulating over the session.
                nextFrameAt += interval;
                const auto now =
                    std::chrono::steady_clock::now();
                if (now - nextFrameAt >= interval) {
                    // Do not emit a burst of stale catch-up frames after a
                    // debugger pause or a temporary GPU scheduling stall.
                    nextFrameAt = now;
                }
                const auto waitResult =
                    WaitForLibWebRtcCaptureDeadline(
                        captureTimer.get(),
                        stopEvent.get(),
                        scheduleWakeEvent.get(),
                        nextFrameAt,
                        stopToken);
                if (waitResult == CaptureWaitResult::kStopped) {
                    break;
                }
                if (waitResult == CaptureWaitResult::kScheduleChanged) {
                    nextFrameAt = std::chrono::steady_clock::now();
                }
            }
            {
                std::lock_guard lock(mutex_);
                if (captureScheduleWakeEvent_ ==
                    scheduleWakeEvent.get()) {
                    captureScheduleWakeEvent_ = nullptr;
                }
            }
            if (!nativeFailed || stopToken.stop_requested()) {
                return;
            }
            totalFailedCaptures_.fetch_add(
                1, std::memory_order_relaxed);
        }
        if (!stopToken.stop_requested()) {
            std::string reason = nativeCapturer.LastError();
            if (reason.empty()) {
                reason = "The native DXGI desktop capturer failed; "
                         "using libwebrtc capture instead.";
            }
            std::lock_guard lock(mutex_);
            fallbackReason_ = std::move(reason);
        }
    } else if (configuredImplementation_ ==
                   DesktopCaptureImplementation::kNativeDxgi &&
               remoteDesktopSession) {
        std::lock_guard lock(mutex_);
        fallbackReason_ =
            "Windows RDP session detected; native DXGI Desktop "
            "Duplication is unavailable, using libwebrtc GDI capture.";
    }

    adaptiveFrameDeliveryEnabled_.store(
        true, std::memory_order_release);
    auto options = webrtc::DesktopCaptureOptions::CreateDefault();
    // Cursor shape and position are transported independently and composed
    // by the viewer's Qt/D3D11 renderer. Avoid burning the pointer into the
    // desktop video, which would duplicate it and tie motion to video FPS.
    options.set_prefer_cursor_embedded(false);
    const bool directXSupported =
        !remoteDesktopSession &&
        webrtc::ScreenCapturerWinDirectx::IsSupported();
    options.set_allow_wgc_screen_capturer(false);
    options.set_allow_directx_capturer(directXSupported);
    // CreateScreenCapturer wraps the raw DXGI/GDI capturer in
    // DesktopCapturerDifferWrapper when this flag is enabled. The wrapper
    // compares actual BGRA pixels and reports only genuinely changed regions.
    options.set_detect_updated_region(true);

    auto capturer =
        webrtc::DesktopCapturer::CreateScreenCapturer(options);
    if (!capturer) {
        SetInitializationFailure(
            "WebRTC could not create a Windows screen capturer.");
        return;
    }

    webrtc::DesktopCapturer::SourceList sources;
    if (!capturer->GetSourceList(&sources) || sources.empty()) {
        SetInitializationFailure(
            "No Windows display is available for capture.");
        return;
    }
    const auto selectedSource =
        FindLibWebRtcScreenSource(captureTarget, sources);
    if (!selectedSource ||
        !capturer->SelectSource(*selectedSource)) {
        SetInitializationFailure(
            "The selected Windows display is no longer available.");
        return;
    }

    {
        std::lock_guard lock(mutex_);
        backend_ = directXSupported ? CaptureBackend::kDxgiPreferred
                                    : CaptureBackend::kGdi;
        initializationFinished_ = true;
    }
    capturer->Start(this);

    auto captureTimer = CreateCaptureTimer();
    UniqueWinHandle stopEvent(
        CreateEventW(nullptr, TRUE, FALSE, nullptr));
    UniqueWinHandle scheduleWakeEvent(
        CreateEventW(nullptr, FALSE, FALSE, nullptr));
    if (!stopEvent.get() || !scheduleWakeEvent.get()) {
        SetInitializationFailure(
            "WebRTC could not create the capture scheduler wake event.");
        return;
    }
    {
        std::lock_guard lock(mutex_);
        captureScheduleWakeEvent_ = scheduleWakeEvent.get();
    }
    std::stop_callback stopWake(
        stopToken, [event = stopEvent.get()] {
            if (event) {
                SetEvent(event);
            }
        });
    auto rateWindowStartedAt = std::chrono::steady_clock::now();
    std::uint64_t windowAttempts = 0;
    std::uint64_t windowDelivered =
        totalDeliveredFrames_.load(std::memory_order_acquire);
    std::uint64_t windowChanged =
        totalChangedFrames_.load(std::memory_order_acquire);
    std::uint64_t windowHeartbeats =
        totalIdleHeartbeatFrames_.load(std::memory_order_acquire);
    auto nextFrameAt = std::chrono::steady_clock::now();
    while (!stopToken.stop_requested()) {
        const auto startedAt = std::chrono::steady_clock::now();
        totalCaptureAttempts_.fetch_add(1, std::memory_order_relaxed);
        ++windowAttempts;
        capturer->CaptureFrame();
        const auto captureFinishedAt = std::chrono::steady_clock::now();
        latestCaptureCallUs_.store(
            static_cast<std::uint64_t>(
                std::chrono::duration_cast<std::chrono::microseconds>(
                    captureFinishedAt - startedAt).count()),
            std::memory_order_release);

        const auto rateWindowElapsed =
            std::chrono::duration_cast<std::chrono::microseconds>(
                captureFinishedAt - rateWindowStartedAt);
        if (rateWindowElapsed >= std::chrono::seconds(1)) {
            const auto currentDelivered =
                totalDeliveredFrames_.load(std::memory_order_acquire);
            const std::uint64_t deliveredInWindow =
                currentDelivered >= windowDelivered
                    ? currentDelivered - windowDelivered
                    : 0;
            const auto currentChanged =
                totalChangedFrames_.load(std::memory_order_acquire);
            const auto changedInWindow =
                currentChanged >= windowChanged
                    ? currentChanged - windowChanged
                    : 0;
            const auto currentHeartbeats =
                totalIdleHeartbeatFrames_.load(std::memory_order_acquire);
            const auto heartbeatsInWindow =
                currentHeartbeats >= windowHeartbeats
                    ? currentHeartbeats - windowHeartbeats
                    : 0;
            const auto elapsedUs =
                (std::max<std::int64_t>)(rateWindowElapsed.count(), 1);
            captureAttemptsPerSecondMilli_.store(
                windowAttempts * 1'000'000'000ULL /
                    static_cast<std::uint64_t>(elapsedUs),
                std::memory_order_release);
            deliveredFramesPerSecondMilli_.store(
                deliveredInWindow * 1'000'000'000ULL /
                    static_cast<std::uint64_t>(elapsedUs),
                std::memory_order_release);
            changedFramesPerSecondMilli_.store(
                changedInWindow * 1'000'000'000ULL /
                    static_cast<std::uint64_t>(elapsedUs),
                std::memory_order_release);
            idleHeartbeatFramesPerSecondMilli_.store(
                heartbeatsInWindow * 1'000'000'000ULL /
                    static_cast<std::uint64_t>(elapsedUs),
                std::memory_order_release);
            rateWindowStartedAt = captureFinishedAt;
            windowAttempts = 0;
            windowDelivered = currentDelivered;
            windowChanged = currentChanged;
            windowHeartbeats = currentHeartbeats;
        }
        const std::uint32_t framesPerSecond =
            targetFrameRate_.load(std::memory_order_acquire);
        // Input activity wakes the scheduler immediately, but it must not
        // silently raise a 30/60 FPS user preference to the encoder's 120 FPS
        // ceiling. The wake-up reduces phase latency while this interval
        // remains the hard capture-rate limit.
        const auto interval = std::chrono::microseconds(
            1'000'000 / (std::max)(framesPerSecond, 1u));
        // Keep the same absolute, high-resolution cadence as the native DXGI
        // path. A repeated one-millisecond sleep loses several capture slots
        // per second at 60 FPS and aliases with desktop presentation timing.
        nextFrameAt += interval;
        const auto now = std::chrono::steady_clock::now();
        if (now - nextFrameAt >= interval) {
            nextFrameAt = now;
        }
        bool stopCapture = false;
        for (;;) {
            const auto waitResult =
                WaitForLibWebRtcCaptureDeadline(
                    captureTimer.get(),
                    stopEvent.get(),
                    scheduleWakeEvent.get(),
                    nextFrameAt,
                    stopToken);
            if (waitResult == CaptureWaitResult::kStopped) {
                stopCapture = true;
                break;
            }
            if (waitResult ==
                CaptureWaitResult::kDeadlineReached) {
                break;
            }

            const auto scheduleNow =
                std::chrono::steady_clock::now();
            if (forcedRefreshFramesRemaining_.load(
                    std::memory_order_acquire)) {
                nextFrameAt = scheduleNow;
                break;
            }
            const auto revisedInterval = std::chrono::microseconds(
                1'000'000 /
                (std::max)(
                    targetFrameRate_.load(
                        std::memory_order_acquire),
                    1u));
            // A wake-up advances the next deadline but never permits input
            // messages to exceed the configured target frame rate.
            nextFrameAt = (std::min)(
                nextFrameAt, startedAt + revisedInterval);
            if (nextFrameAt <= scheduleNow) {
                break;
            }
        }
        if (stopCapture) {
            break;
        }
    }
    {
        std::lock_guard lock(mutex_);
        if (captureScheduleWakeEvent_ == scheduleWakeEvent.get()) {
            captureScheduleWakeEvent_ = nullptr;
        }
    }
}

void WindowsDesktopCaptureSource::OnCaptureResult(
    webrtc::DesktopCapturer::Result result,
    std::unique_ptr<webrtc::DesktopFrame> frame)
{
    if (result != webrtc::DesktopCapturer::Result::SUCCESS || !frame ||
        !frame->data() || frame->size().width() <= 0 ||
        frame->size().height() <= 0) {
        totalFailedCaptures_.fetch_add(1, std::memory_order_relaxed);
        if (result == webrtc::DesktopCapturer::Result::ERROR_PERMANENT) {
            std::lock_guard lock(mutex_);
            lastError_ = "Windows desktop capture failed permanently.";
            firstFrameCondition_.notify_all();
        }
        return;
    }
    const int width = frame->size().width();
    const int height = frame->size().height();
    const auto previousWidth =
        capturedWidth_.load(std::memory_order_acquire);
    const auto previousHeight =
        capturedHeight_.load(std::memory_order_acquire);
    capturedWidth_.store(
        static_cast<std::uint32_t>(width), std::memory_order_release);
    capturedHeight_.store(
        static_cast<std::uint32_t>(height), std::memory_order_release);
    const bool desktopChanged = !frame->updated_region().is_empty();
    const bool crdPolicyEnabled =
        configuredImplementation_ ==
        DesktopCaptureImplementation::kLibWebRtc;
    const bool dimensionsChanged =
        previousWidth != 0 && previousHeight != 0 &&
        (previousWidth != static_cast<std::uint32_t>(width) ||
         previousHeight != static_cast<std::uint32_t>(height));
    const bool forceRefresh =
        ConsumeForcedRefreshFrame() || dimensionsChanged;
    FrameDeliveryReason deliveryReason{};
    const bool deliver = crdPolicyEnabled
        ? ShouldDeliverLibWebRtcFrame(
              desktopChanged,
              forceRefresh,
              std::chrono::steady_clock::now(),
              &deliveryReason)
        : ShouldDeliverFrame(
              desktopChanged,
              forceRefresh,
              std::chrono::steady_clock::now(),
              &deliveryReason);
    if (deliver) {
        auto bgra = webrtc::make_ref_counted<DesktopBgraFrameBuffer>(
            frame->data(), frame->stride(), width, height);
        if (!crdPolicyEnabled) {
            DeliverFrame(std::move(bgra), deliveryReason);
            return;
        }
        FrameUpdateRegion updateRegion;
        if (deliveryReason == FrameDeliveryReason::kInitial ||
            deliveryReason == FrameDeliveryReason::kForcedRefresh ||
            deliveryReason == FrameDeliveryReason::kStartupPrime) {
            updateRegion.width = width;
            updateRegion.height = height;
        } else if (deliveryReason ==
                   FrameDeliveryReason::kDesktopChanged) {
            const auto bounds = BoundingUpdateRegion(
                frame->updated_region(), width, height);
            updateRegion.offsetX = bounds.offsetX;
            updateRegion.offsetY = bounds.offsetY;
            updateRegion.width = bounds.width;
            updateRegion.height = bounds.height;
            if (updateRegion.width <= 0 ||
                updateRegion.height <= 0) {
                updateRegion = {};
                updateRegion.width = width;
                updateRegion.height = height;
            }
        }
        DeliverLibWebRtcFrame(
            std::move(bgra),
            deliveryReason,
            updateRegion,
            deliveryReason == FrameDeliveryReason::kIdleHeartbeat);
    }
}

void WindowsDesktopCaptureSource::ResetActivityTracking()
{
    activityState_.store(
        CaptureActivityState::kStarting, std::memory_order_release);
    adaptiveFrameDeliveryEnabled_.store(false, std::memory_order_release);
    activityHasDeliveredFrame_ = false;
    inputBoostUntilSteadyUs_.store(0, std::memory_order_release);
    startupPrimeUntilSteadyUs_.store(0, std::memory_order_release);
    forcedRefreshFramesRemaining_.store(0, std::memory_order_release);
    startupFrameDeliveredCallback_ = {};
    startupBurstCompletedCallback_ = {};
    startupCallbackFramesRemaining_ = 0;
    const auto now = std::chrono::steady_clock::now();
    activityLastChangedAt_ = now;
    activityLastDeliveredAt_ = now;
}

bool WindowsDesktopCaptureSource::ShouldDeliverFrame(
    bool desktopChanged,
    bool forceRefresh,
    std::chrono::steady_clock::time_point now,
    FrameDeliveryReason* reason)
{
    if (!reason) {
        return false;
    }
    const bool initial = !activityHasDeliveredFrame_;
    activityHasDeliveredFrame_ = true;
    activityLastDeliveredAt_ = now;
    activityState_.store(
        CaptureActivityState::kActive, std::memory_order_release);
    if (initial) {
        activityLastChangedAt_ = now;
        *reason = FrameDeliveryReason::kInitial;
        return true;
    }
    if (forceRefresh) {
        activityLastDeliveredAt_ = now;
        totalForcedRefreshFrames_.fetch_add(
            1, std::memory_order_relaxed);
        *reason = FrameDeliveryReason::kForcedRefresh;
        return true;
    }
    if (desktopChanged) {
        activityLastChangedAt_ = now;
        *reason = FrameDeliveryReason::kDesktopChanged;
        return true;
    }
    *reason = FrameDeliveryReason::kScheduledRepeat;
    return true;
}

bool WindowsDesktopCaptureSource::ShouldDeliverLibWebRtcFrame(
    bool desktopChanged,
    bool forceRefresh,
    std::chrono::steady_clock::time_point now,
    FrameDeliveryReason* reason)
{
    if (!reason) {
        return false;
    }
    if (!activityHasDeliveredFrame_) {
        activityHasDeliveredFrame_ = true;
        activityLastChangedAt_ = now;
        activityLastDeliveredAt_ = now;
        activityState_.store(
            CaptureActivityState::kActive, std::memory_order_release);
        *reason = FrameDeliveryReason::kInitial;
        return true;
    }
    if (forceRefresh) {
        activityLastDeliveredAt_ = now;
        totalForcedRefreshFrames_.fetch_add(
            1, std::memory_order_relaxed);
        *reason = FrameDeliveryReason::kForcedRefresh;
        return true;
    }
    if (desktopChanged) {
        if (activityState_.exchange(
                CaptureActivityState::kActive,
                std::memory_order_acq_rel) !=
            CaptureActivityState::kActive) {
            totalActivityTransitions_.fetch_add(
                1, std::memory_order_relaxed);
        }
        activityLastChangedAt_ = now;
        activityLastDeliveredAt_ = now;
        *reason = FrameDeliveryReason::kDesktopChanged;
        return true;
    }
    if (SteadyNowUs() <
        startupPrimeUntilSteadyUs_.load(std::memory_order_acquire)) {
        activityState_.store(
            CaptureActivityState::kActive, std::memory_order_release);
        activityLastDeliveredAt_ = now;
        *reason = FrameDeliveryReason::kStartupPrime;
        return true;
    }
    if (SteadyNowUs() <
        inputBoostUntilSteadyUs_.load(std::memory_order_acquire)) {
        if (activityState_.exchange(
                CaptureActivityState::kActive,
                std::memory_order_acq_rel) !=
            CaptureActivityState::kActive) {
            totalActivityTransitions_.fetch_add(
                1, std::memory_order_relaxed);
        }
        activityLastDeliveredAt_ = now;
        *reason = FrameDeliveryReason::kScheduledRepeat;
        return true;
    }

    if (now - activityLastChangedAt_ >= kIdleTransitionDelay &&
        activityState_.exchange(
            CaptureActivityState::kIdle,
            std::memory_order_acq_rel) != CaptureActivityState::kIdle) {
        totalActivityTransitions_.fetch_add(
            1, std::memory_order_relaxed);
    }
    if (activityState_.load(std::memory_order_acquire) ==
            CaptureActivityState::kIdle &&
        now - activityLastDeliveredAt_ >=
            kLibWebRtcKeepAliveInterval) {
        activityLastDeliveredAt_ = now;
        *reason = FrameDeliveryReason::kIdleHeartbeat;
        return true;
    }
    totalSuppressedUnchangedFrames_.fetch_add(
        1, std::memory_order_relaxed);
    return false;
}

void WindowsDesktopCaptureSource::DeliverFrame(
    webrtc::scoped_refptr<webrtc::VideoFrameBuffer> buffer,
    FrameDeliveryReason reason)
{
    if (!buffer) {
        return;
    }
    totalDeliveredFrames_.fetch_add(1, std::memory_order_relaxed);
    if (reason == FrameDeliveryReason::kDesktopChanged) {
        totalChangedFrames_.fetch_add(1, std::memory_order_relaxed);
    } else if (reason == FrameDeliveryReason::kIdleHeartbeat) {
        totalIdleHeartbeatFrames_.fetch_add(
            1, std::memory_order_relaxed);
    }
    webrtc::VideoFrame videoFrame =
        webrtc::VideoFrame::Builder()
            .set_video_frame_buffer(std::move(buffer))
            .set_timestamp_us(webrtc::TimeMicros())
            .build();
    OnFrame(videoFrame);
    {
        std::lock_guard lock(mutex_);
        firstFrameReady_ = true;
        lastError_.clear();
    }
    firstFrameCondition_.notify_all();
}

void WindowsDesktopCaptureSource::DeliverLibWebRtcFrame(
    webrtc::scoped_refptr<webrtc::VideoFrameBuffer> buffer,
    FrameDeliveryReason reason,
    const FrameUpdateRegion& updateRegion,
    bool repeatFrame)
{
    if (!buffer) {
        return;
    }
    totalDeliveredFrames_.fetch_add(1, std::memory_order_relaxed);
    if (reason == FrameDeliveryReason::kDesktopChanged) {
        totalChangedFrames_.fetch_add(1, std::memory_order_relaxed);
    } else if (reason == FrameDeliveryReason::kIdleHeartbeat) {
        totalIdleHeartbeatFrames_.fetch_add(
            1, std::memory_order_relaxed);
    }
    webrtc::VideoFrame::UpdateRect updateRect;
    updateRect.offset_x = updateRegion.offsetX;
    updateRect.offset_y = updateRegion.offsetY;
    updateRect.width = updateRegion.width;
    updateRect.height = updateRegion.height;
    webrtc::VideoFrame videoFrame =
        webrtc::VideoFrame::Builder()
            .set_video_frame_buffer(std::move(buffer))
            .set_timestamp_us(webrtc::TimeMicros())
            .set_update_rect(updateRect)
            .set_is_repeat_frame(repeatFrame)
            .build();
    OnFrame(videoFrame);
    std::function<void()> startupFrameDeliveredCallback;
    std::function<void()> startupBurstCompletedCallback;
    {
        std::lock_guard lock(mutex_);
        firstFrameReady_ = true;
        lastError_.clear();
        startupFrameDeliveredCallback =
            std::move(startupFrameDeliveredCallback_);
        if (startupCallbackFramesRemaining_ > 0 &&
            --startupCallbackFramesRemaining_ == 0) {
            startupBurstCompletedCallback =
                std::move(startupBurstCompletedCallback_);
        }
    }
    firstFrameCondition_.notify_all();
    if (startupFrameDeliveredCallback) {
        startupFrameDeliveredCallback();
    }
    if (startupBurstCompletedCallback) {
        startupBurstCompletedCallback();
    }
}

void WindowsDesktopCaptureSource::SetInitializationFailure(
    std::string message)
{
    {
        std::lock_guard lock(mutex_);
        initializationFinished_ = true;
        running_ = false;
        lastError_ = std::move(message);
    }
    firstFrameCondition_.notify_all();
}

}  // namespace remote
