// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#pragma once

#include <mutex>
#include <unordered_map>
#include <unordered_set>

#include "src/core/DisplayTopology.h"
#include "src/protocol/RemoteInputProtocol.h"

namespace remote {

// Applies already-authorized remote input to the interactive Windows desktop.
// The session engine validates room identity, control grants and sequencing
// before invoking this sink. Windows UIPI still prevents injection into a
// higher-integrity process or the secure desktop.
class WindowsInputExecutor final : public IRemoteInputSink {
public:
    WindowsInputExecutor() = default;
    ~WindowsInputExecutor() override;

    WindowsInputExecutor(const WindowsInputExecutor&) = delete;
    WindowsInputExecutor& operator=(const WindowsInputExecutor&) = delete;

    void OnRemoteInput(const RemoteInputEvent& event) override;
    void ReleaseAllRemoteInputs() override;
    void SetActiveDisplay(const DisplayTopologySnapshot& topology,
                          const std::string& stableDisplayKey);
    void ClearActiveDisplay();

private:
    bool ApplyMouseMove(const RemoteInputEvent& event);
    bool ApplyMouseButton(const RemoteInputEvent& event);
    bool ApplyMouseWheel(const RemoteInputEvent& event);
    bool ApplyKey(const RemoteInputEvent& event);
    bool ApplyPointerPositionAndState(const RemoteInputEvent& event);
    void ReleaseAllRemoteInputsLocked();

    std::mutex mutex_;
    std::unordered_set<RemoteMouseButton> pressedMouseButtons_;
    std::unordered_map<std::uint64_t, RemoteInputEvent> pressedKeys_;
    DisplayDescriptor activeDisplay_;
    std::uint64_t activeLayoutVersion_ = 0;
};

}  // namespace remote
