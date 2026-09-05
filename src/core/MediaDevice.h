// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace remote {

inline constexpr char kSystemDefaultMediaDeviceId[] = "default";

enum class MediaDeviceKind {
    kCamera,
    kMicrophone,
    kSpeaker,
};

struct MediaDeviceDescriptor {
    std::string id;
    std::string name;
    bool available = true;
};

enum class MediaDeviceSelectionState {
    kReady,
    kSwitching,
    kUnavailable,
    kFailed,
};

struct MediaDeviceCategorySnapshot {
    std::vector<MediaDeviceDescriptor> devices;
    std::string preferredDeviceId = kSystemDefaultMediaDeviceId;
    std::string activeDeviceId;
    // Resolved physical endpoint name. When activeDeviceId is "default",
    // this identifies the Windows default device that is actually in use.
    std::string activeDeviceName;
    MediaDeviceSelectionState state =
        MediaDeviceSelectionState::kReady;
    std::string errorCode;
    std::string errorMessage;
};

struct MediaDeviceSnapshot {
    MediaDeviceCategorySnapshot camera;
    MediaDeviceCategorySnapshot microphone;
    MediaDeviceCategorySnapshot speaker;
    bool refreshing = false;
    std::uint64_t revision = 0;
};

}  // namespace remote
