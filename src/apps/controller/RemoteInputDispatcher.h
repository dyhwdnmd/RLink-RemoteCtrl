// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#pragma once

#include <Windows.h>

#include <chrono>
#include <condition_variable>
#include <functional>
#include <mutex>
#include <optional>
#include <thread>

#include "src/apps/remote/InProcessSessionEngine.h"
#include "src/protocol/RemoteInputProtocol.h"

namespace remote::controller {

class RemoteInputDispatcher final {
public:
    void SetEngine(app::InProcessSessionEngine* engine,
                   bool directSession = false)
    {
        std::lock_guard lock(mutex_);
        engine_ = engine;
        directSession_ = directSession;
    }

    void SetEnabled(bool enabled)
    {
        std::lock_guard lock(mutex_);
        enabled_ = enabled;
    }

    void Clear()
    {
        std::lock_guard lock(mutex_);
        enabled_ = false;
        engine_ = nullptr;
        directSession_ = false;
    }

    bool Send(const RemoteInputEvent& event)
    {
        std::lock_guard lock(mutex_);
        return enabled_ && engine_ &&
            (directSession_ ? engine_->SendDirectInput(event)
                            : engine_->SendRoomInput(event)).accepted;
    }

private:
    std::mutex mutex_;
    app::InProcessSessionEngine* engine_ = nullptr;
    bool enabled_ = false;
    bool directSession_ = false;
};

class HighResolutionPointerMoveScheduler final {
public:
    using InputSender =
        std::function<bool(const RemoteInputEvent&)>;
    using DragSampleProvider = std::function<
        std::optional<RemoteInputEvent>(
            const RemoteInputEvent&)>;

    explicit HighResolutionPointerMoveScheduler(
        InputSender sender,
        DragSampleProvider dragSampleProvider)
        : sender_(std::move(sender)),
        dragSampleProvider_(
            std::move(dragSampleProvider))
    {
        wakeEvent_ = CreateEventW(
            nullptr, FALSE, FALSE, nullptr);
        constexpr DWORD
            kCreateWaitableTimerHighResolution = 0x00000002;
        waitableTimer_ = CreateWaitableTimerExW(
            nullptr,
            nullptr,
            kCreateWaitableTimerHighResolution,
            TIMER_ALL_ACCESS);
        if (!waitableTimer_) {
            waitableTimer_ = CreateWaitableTimerW(
                nullptr, FALSE, nullptr);
        }
        if (wakeEvent_ && waitableTimer_) {
            worker_ = std::jthread(
                [this](std::stop_token stopToken) {
                    Run(stopToken);
                });
        }
    }

    ~HighResolutionPointerMoveScheduler()
    {
        Shutdown();
    }

    HighResolutionPointerMoveScheduler(
        const HighResolutionPointerMoveScheduler&) = delete;
    HighResolutionPointerMoveScheduler& operator=(
        const HighResolutionPointerMoveScheduler&) = delete;

    bool Queue(
        const RemoteInputEvent& input,
        std::uint32_t rateLimitHz)
    {
        rateLimitHz = (std::max)(rateLimitHz, 1u);
        RemoteInputTelemetry::Instance()
            .SetMoveDispatchRateLimit(rateLimitHz);
        if (!worker_.joinable()) {
            return Dispatch(input);
        }

        const auto now = std::chrono::steady_clock::now();
        const auto interval = std::chrono::microseconds(
            1'000'000u / rateLimitHz);
        bool coalesced = false;
        bool dispatchImmediately = false;
        {
            std::lock_guard lock(stateMutex_);
            if (stopping_) {
                return false;
            }
            if (dragActive_ &&
                input.pressedMouseButtons != 0) {
                dragInput_ = input;
                dragInterval_ = interval;
                RemoteInputTelemetry::Instance()
                    .RecordCoalescedMouseMove();
                return true;
            }
            coalesced = pendingInput_.has_value();
            pendingInterval_ = interval;
            if (!hasCadence_ || nextDispatchAt_ <= now) {
                pendingInput_.reset();
                lastDispatchAt_ = now;
                nextDispatchAt_ = now + interval;
                hasCadence_ = true;
                dispatchImmediately = true;
            } else {
                pendingInput_ = input;
                const auto revisedDeadline =
                    lastDispatchAt_ + interval;
                nextDispatchAt_ = (std::min)(
                    nextDispatchAt_, revisedDeadline);
            }
        }
        if (coalesced) {
            RemoteInputTelemetry::Instance()
                .RecordCoalescedMouseMove();
        }
        if (dispatchImmediately) {
            // Keep the first move and any already-due move synchronous.
            // The worker only owns sub-frame deadline completion, so
            // moving dispatch off the Qt timer does not add a new
            // first-response hop.
            std::lock_guard dispatchLock(dispatchMutex_);
            return Dispatch(input);
        }
        SetEvent(wakeEvent_);
        return true;
    }

    void BeginDrag(
        const RemoteInputEvent& input,
        std::uint32_t sampleRateHz)
    {
        if (!worker_.joinable()) {
            return;
        }
        sampleRateHz = (std::max)(sampleRateHz, 1u);
        RemoteInputTelemetry::Instance()
            .SetMoveDispatchRateLimit(sampleRateHz);
        std::lock_guard dispatchLock(dispatchMutex_);
        {
            std::lock_guard stateLock(stateMutex_);
            if (stopping_) {
                return;
            }
            const auto now =
                std::chrono::steady_clock::now();
            dragActive_ = true;
            dragInput_ = input;
            dragInterval_ = std::chrono::microseconds(
                1'000'000u / sampleRateHz);
            lastDragX_ = input.normalizedX;
            lastDragY_ = input.normalizedY;
            lastDragPointValid_ = true;
            pendingInput_.reset();
            lastDispatchAt_ = now;
            nextDispatchAt_ = now + dragInterval_;
            hasCadence_ = true;
        }
        SetEvent(wakeEvent_);
    }

    void SetActiveDragSampleRate(
        std::uint32_t sampleRateHz)
    {
        if (!worker_.joinable()) {
            return;
        }
        sampleRateHz = (std::max)(sampleRateHz, 1u);
        const auto interval = std::chrono::microseconds(
            1'000'000u / sampleRateHz);
        {
            std::lock_guard stateLock(stateMutex_);
            if (!dragActive_ || stopping_) {
                return;
            }
            dragInterval_ = interval;
            nextDispatchAt_ = (std::min)(
                nextDispatchAt_,
                std::chrono::steady_clock::now() +
                    interval);
        }
        RemoteInputTelemetry::Instance()
            .SetMoveDispatchRateLimit(sampleRateHz);
        SetEvent(wakeEvent_);
    }

    void EndDrag()
    {
        if (!worker_.joinable()) {
            return;
        }
        std::lock_guard dispatchLock(dispatchMutex_);
        {
            std::lock_guard stateLock(stateMutex_);
            dragActive_ = false;
            lastDragPointValid_ = false;
            pendingInput_.reset();
            hasCadence_ = false;
        }
        CancelWaitableTimer(waitableTimer_);
        SetEvent(wakeEvent_);
    }

    bool Flush()
    {
        if (!worker_.joinable()) {
            return true;
        }
        std::lock_guard dispatchLock(dispatchMutex_);
        std::optional<RemoteInputEvent> input;
        bool dragSample = false;
        {
            std::lock_guard stateLock(stateMutex_);
            if (stopping_) {
                return true;
            }
            if (dragActive_) {
                input = dragInput_;
                dragSample = true;
            } else if (pendingInput_) {
                input = *pendingInput_;
                pendingInput_.reset();
            } else {
                return true;
            }
            const auto now =
                std::chrono::steady_clock::now();
            lastDispatchAt_ = now;
            nextDispatchAt_ = now +
                (dragSample
                    ? dragInterval_
                    : pendingInterval_);
            hasCadence_ = true;
        }
        SetEvent(wakeEvent_);
        if (dragSample) {
            input = SampleDragInput(*input);
            if (!input || !AcceptDragPoint(*input)) {
                return true;
            }
        }
        return Dispatch(*input);
    }

    void Cancel()
    {
        if (!worker_.joinable()) {
            return;
        }
        std::lock_guard dispatchLock(dispatchMutex_);
        {
            std::lock_guard stateLock(stateMutex_);
            pendingInput_.reset();
            dragActive_ = false;
            lastDragPointValid_ = false;
            hasCadence_ = false;
        }
        CancelWaitableTimer(waitableTimer_);
        SetEvent(wakeEvent_);
    }

    void Shutdown()
    {
        if (worker_.joinable()) {
            {
                std::lock_guard lock(stateMutex_);
                stopping_ = true;
                pendingInput_.reset();
                dragActive_ = false;
            }
            worker_.request_stop();
            SetEvent(wakeEvent_);
            worker_.join();
        }
        if (waitableTimer_) {
            CancelWaitableTimer(waitableTimer_);
            CloseHandle(waitableTimer_);
            waitableTimer_ = nullptr;
        }
        if (wakeEvent_) {
            CloseHandle(wakeEvent_);
            wakeEvent_ = nullptr;
        }
    }

private:
    bool Dispatch(const RemoteInputEvent& input)
    {
        RemoteInputTelemetry::Instance()
            .RecordDispatchedMouseMove();
        return sender_ && sender_(input);
    }

    std::optional<RemoteInputEvent> SampleDragInput(
        const RemoteInputEvent& fallback) const
    {
        if (!dragSampleProvider_) {
            return fallback;
        }
        const auto sampled =
            dragSampleProvider_(fallback);
        return sampled ? sampled
                       : std::optional<RemoteInputEvent>(
                             fallback);
    }

    bool AcceptDragPoint(const RemoteInputEvent& input)
    {
        std::lock_guard stateLock(stateMutex_);
        if (!dragActive_) {
            return false;
        }
        if (lastDragPointValid_ &&
            lastDragX_ == input.normalizedX &&
            lastDragY_ == input.normalizedY) {
            return false;
        }
        lastDragX_ = input.normalizedX;
        lastDragY_ = input.normalizedY;
        lastDragPointValid_ = true;
        dragInput_.normalizedX = input.normalizedX;
        dragInput_.normalizedY = input.normalizedY;
        return true;
    }

    void DispatchDueInput()
    {
        std::lock_guard dispatchLock(dispatchMutex_);
        std::optional<RemoteInputEvent> input;
        bool dragSample = false;
        {
            std::lock_guard stateLock(stateMutex_);
            if (stopping_ ||
                (!dragActive_ && !pendingInput_)) {
                return;
            }
            const auto now =
                std::chrono::steady_clock::now();
            if (now < nextDispatchAt_) {
                return;
            }
            if (dragActive_) {
                input = dragInput_;
                dragSample = true;
            } else {
                input = *pendingInput_;
                pendingInput_.reset();
            }
            const auto previousDeadline = nextDispatchAt_;
            const auto interval = dragSample
                ? dragInterval_
                : pendingInterval_;
            lastDispatchAt_ = now;
            if (now - previousDeadline >= interval) {
                nextDispatchAt_ = now + interval;
            } else {
                nextDispatchAt_ =
                    previousDeadline + interval;
            }
            hasCadence_ = true;
        }
        if (dragSample) {
            input = SampleDragInput(*input);
            if (!input || !AcceptDragPoint(*input)) {
                return;
            }
        }
        (void)Dispatch(*input);
    }

    bool ArmTimer(
        std::chrono::steady_clock::time_point deadline)
    {
        const auto now = std::chrono::steady_clock::now();
        if (deadline <= now) {
            return true;
        }
        const auto remaining100ns =
            (std::max<std::int64_t>)(
                std::chrono::duration_cast<
                    std::chrono::nanoseconds>(
                        deadline - now)
                        .count() /
                    100,
                1);
        LARGE_INTEGER dueTime{};
        dueTime.QuadPart = -remaining100ns;
        return SetWaitableTimerEx(
            waitableTimer_,
            &dueTime,
            0,
            nullptr,
            nullptr,
            nullptr,
            0) != FALSE;
    }

    void Run(std::stop_token stopToken)
    {
        SetThreadPriority(
            GetCurrentThread(), THREAD_PRIORITY_ABOVE_NORMAL);
        while (!stopToken.stop_requested()) {
            std::optional<
                std::chrono::steady_clock::time_point> deadline;
            {
                std::lock_guard lock(stateMutex_);
                if (stopping_) {
                    break;
                }
                if (pendingInput_ || dragActive_) {
                    deadline = nextDispatchAt_;
                }
            }

            if (!deadline) {
                WaitForSingleObject(wakeEvent_, INFINITE);
                continue;
            }
            if (*deadline <=
                std::chrono::steady_clock::now()) {
                DispatchDueInput();
                continue;
            }
            if (!ArmTimer(*deadline)) {
                WaitForSingleObject(wakeEvent_, 1);
                continue;
            }
            const HANDLE waits[] = {
                wakeEvent_, waitableTimer_ };
            (void)WaitForMultipleObjects(
                static_cast<DWORD>(std::size(waits)),
                waits,
                FALSE,
                INFINITE);
            CancelWaitableTimer(waitableTimer_);
        }
    }

    InputSender sender_;
    DragSampleProvider dragSampleProvider_;
    HANDLE wakeEvent_ = nullptr;
    HANDLE waitableTimer_ = nullptr;
    std::jthread worker_;
    std::mutex stateMutex_;
    std::mutex dispatchMutex_;
    std::optional<RemoteInputEvent> pendingInput_;
    RemoteInputEvent dragInput_;
    std::chrono::microseconds pendingInterval_{8'333};
    std::chrono::microseconds dragInterval_{4'166};
    std::chrono::steady_clock::time_point lastDispatchAt_;
    std::chrono::steady_clock::time_point nextDispatchAt_;
    std::uint16_t lastDragX_ = 0;
    std::uint16_t lastDragY_ = 0;
    bool hasCadence_ = false;
    bool dragActive_ = false;
    bool lastDragPointValid_ = false;
    bool stopping_ = false;
};

}  // namespace remote::controller
