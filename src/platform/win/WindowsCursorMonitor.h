// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#pragma once

#include <atomic>
#include <cstdint>
#include <functional>
#include <mutex>
#include <optional>
#include <thread>

#include "src/core/DisplayTopology.h"
#include "src/protocol/RemoteCursorProtocol.h"

namespace remote {

struct WindowsCursorObservation {
    RemoteCursorPosition position;
    std::optional<RemoteCursorShape> changedShape;
};

// Captures the process-global Windows cursor independently from desktop video.
// The monitor owns one lightweight 120 Hz waitable-timer thread only while a
// local desktop is actively being shared.
class WindowsCursorMonitor final {
public:
    using Callback = std::function<void(WindowsCursorObservation)>;

    WindowsCursorMonitor() = default;
    ~WindowsCursorMonitor();

    WindowsCursorMonitor(const WindowsCursorMonitor&) = delete;
    WindowsCursorMonitor& operator=(const WindowsCursorMonitor&) = delete;

    bool Start(const DisplayDescriptor& display,
               std::uint64_t layoutVersion,
               Callback callback);
    void UpdateTarget(const DisplayDescriptor& display,
                      std::uint64_t layoutVersion);
    void SetLastAppliedInputSequence(std::uint64_t sequence);
    void Stop();
    bool running() const;

private:
    struct Target {
        DisplayDescriptor display;
        std::uint64_t layoutVersion = 0;
    };

    void Run(std::stop_token stopToken);

    mutable std::mutex mutex_;
    Target target_;
    Callback callback_;
    std::jthread thread_;
    std::atomic<bool> running_{false};
    std::atomic<std::uint64_t> lastAppliedInputSequence_{0};
};

}  // namespace remote
