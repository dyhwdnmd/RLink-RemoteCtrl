// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "DisplayTopology.h"

namespace remote {

inline constexpr std::uint32_t kMinimumRoomMembers = 2;
inline constexpr std::uint32_t kDefaultRoomCapacity = 2;
inline constexpr std::uint32_t kProtocolMaximumRoomMembers = 5;

enum class RoomMembershipState {
    kNone,
    kCreating,
    kJoinPending,
    kActive,
    kLeaving,
    kFailed,
};

enum class RoomScreenShareState {
    kIdle,
    kSwitching,
    kActive,
    kRecovering,
};

enum class RoomPeerConnectionState {
    kStarting,
    kNegotiating,
    kConnecting,
    kActive,
    kDisconnected,
    kRecovering,
    kFailed,
    kClosed,
};

enum class RoomAvailabilityState {
    kChecking,
    kAvailable,
    kTemporarilyUnavailable,
    kClosed,
};

enum class RoomMemberAction : std::uint8_t {
    kRequestScreenShare = 1,
    kStopScreenShare = 2,
    kDisableMicrophone = 3,
};

struct RoomAvailabilitySnapshot {
    std::string roomId;
    RoomAvailabilityState state = RoomAvailabilityState::kChecking;
};

struct RoomMemberSnapshot {
    std::string deviceId;
    std::string deviceName;
    bool online = false;
    bool cameraPublishing = false;
    bool microphonePublishing = false;
};

struct RoomSnapshot {
    RoomMembershipState membership = RoomMembershipState::kNone;
    std::string roomId;
    std::string ownerDeviceId;
    std::uint32_t capacity = kDefaultRoomCapacity;
    RoomScreenShareState screenShareState = RoomScreenShareState::kIdle;
    std::string screenSharerDeviceId;
    std::string pendingScreenSharerDeviceId;
    std::uint64_t screenShareEpoch = 0;
    std::string pendingControllerDeviceId;
    std::string activeControllerDeviceId;
    std::vector<RoomMemberSnapshot> members;
    std::string errorCode;
    std::string errorMessage;
};

struct RoomJoinRequest {
    std::string roomId;
    std::string requestId;
    std::string requesterDeviceId;
    std::string requesterDeviceName;
};

struct RoomControlRequest {
    std::string roomId;
    std::string requestId;
    std::string requesterDeviceId;
    std::string requesterDeviceName;
};

struct RoomScreenShareSwitchRequest {
    std::string roomId;
    std::string requestId;
    std::string requesterDeviceId;
    std::string requesterDeviceName;
};

struct RoomScreenShareViewRequest {
    std::string roomId;
    std::uint64_t sequence = 0;
    std::string requesterDeviceId;
    std::string requesterDeviceName;
};

struct RoomMemberActionResult {
    std::string roomId;
    std::uint64_t sequence = 0;
    std::string peerDeviceId;
    RoomMemberAction action = RoomMemberAction::kRequestScreenShare;
    bool accepted = false;
    std::string error;
};

struct RoomPeerConnectionSnapshot {
    std::string pairId;
    std::string peerDeviceId;
    bool localIsOfferer = false;
    RoomPeerConnectionState state = RoomPeerConnectionState::kStarting;
    std::uint32_t openDataChannelCount = 0;
    // Input readiness is tracked per channel instead of inferred from the
    // aggregate count. Clipboard channels can already be open while the
    // low-latency pointer channel is still negotiating.
    bool controlReliableChannelOpen = false;
    bool inputFastChannelOpen = false;
    bool clipboardReliableChannelOpen = false;
    bool clipboardTransferChannelOpen = false;
    std::uint32_t preparedVideoSlotCount = 0;
    bool screenPreferencePending = false;
    std::uint64_t screenPreferenceSequence = 0;
    // Epoch for which screenPreferenceSequence was sent and acknowledged.
    // Width/FPS values from an older share must never satisfy a new share.
    std::uint64_t screenPreferenceGeneration = 0;
    // Startup telemetry is generation-bound. A non-zero presented generation
    // means that the first frame reached the viewer's actual presentation
    // path, not merely the decoder callback.
    std::uint64_t screenFirstFramePresentedGeneration = 0;
    std::uint32_t screenFirstFrameStartupMs = 0;
    std::uint32_t screenStartupRefreshRequests = 0;
    std::uint32_t screenWidth = 0;
    std::uint32_t screenHeight = 0;
    // A prepared screen sender starts at the product's 60 FPS default until a
    // viewer preference is acknowledged.
    std::uint32_t screenFramesPerSecond = 60;
    // Both normal capture implementations support up to 120 FPS. A runtime
    // fallback may report a lower ceiling after capture starts.
    std::uint32_t screenMaximumFrameRate = 120;
    bool screenCaptureCapabilityReported = false;
    std::string screenCaptureConfiguredBackend;
    std::string screenCaptureActiveBackend;
    std::string screenCaptureFallbackReason;
    std::uint32_t screenMaxBitrateBps = 0;
    std::uint8_t screenScaleBackend = 0;
    // Identity of the exact display currently carried by screen-main.
    // Remote pointer input stays disabled until this reliable metadata arrives.
    std::uint64_t sharedDisplayLayoutVersion = 0;
    std::uint32_t sharedDisplayId = 0;
    std::string sharedDisplayName;
    std::string sharedDisplayStableKey;
    std::uint32_t sharedDisplayWidth = 0;
    std::uint32_t sharedDisplayHeight = 0;
    std::uint64_t remoteDisplayCatalogLayoutVersion = 0;
    bool remoteDisplayCatalogReported = false;
    std::vector<DisplayDescriptor> remoteDisplays;
    bool remoteDisplaySwitchPending = false;
    std::uint64_t remoteDisplaySwitchSequence = 0;
    std::string remoteDisplaySwitchError;
    std::uint64_t negotiationGeneration = 1;
    std::uint32_t iceRestartAttempt = 0;
    std::string errorCode;
    std::string errorMessage;
};

}  // namespace remote
