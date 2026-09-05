// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "DisplayTopology.h"
#include "DirectSessionRequest.h"
#include "MediaDevice.h"
#include "RoomState.h"
#include "SessionDiagnostics.h"
#include "src/protocol/FileTransferProtocol.h"
#include "src/protocol/ClipboardProtocol.h"
#include "src/protocol/RemoteInputProtocol.h"
#include "src/protocol/ScreenShareControlProtocol.h"

namespace remote {

enum class RemoteControlRole {
    kNone,
    kController,
    kControlled,
};

enum class SessionOrigin {
    kNone,
    kManualDeviceId,
    kRemoteAssistance,
    kOwnedDevice,
    kCollaborationRoom,
};

struct OwnedDeviceSnapshot {
    std::string deviceId;
    std::string deviceName;
    bool online = false;
    bool current = false;
    std::int64_t createdAt = 0;
    std::int64_t lastSeenAt = 0;
};

enum class SessionEngineState {
    kStopped,
    kStarting,
    kReady,
    kConnecting,
    kAwaitingLocalApproval,
    kActive,
    kStopping,
    kFailed,
};

enum class SessionConnectivityState {
    kNotConfigured,
    kConnecting,
    kOnline,
    kOffline,
    kFailed,
};

enum class LocalCameraState {
    kOff,
    kStarting,
    kPublishing,
    kStopping,
    kFailed,
};

enum class LocalMicrophoneState {
    kOff,
    kStarting,
    kPublishing,
    kStopping,
    kFailed,
};

struct SessionEngineCapabilities {
    bool webRtcReady = false;
    bool hasH264Encoder = false;
    bool hasH264Decoder = false;
    bool h264HardwareEncoderAvailable = false;
    bool h264HardwareEncoderCpuNv12InputSupported = false;
    bool h264HardwareEncoderD3D11InputCandidate = false;
    uint32_t h264HardwareEncoderCount = 0;
    bool h264HardwareEncoderWired = false;
    bool h264SoftwareEncoderWired = false;
    bool ffmpegX264EncoderWired = false;
    std::string ffmpegX264EncoderError;
    bool ffmpegHardwareEncoderAvailable = false;
    bool ffmpegHardwareEncoderWired = false;
    std::string ffmpegHardwareEncoderError;
    std::vector<std::string> ffmpegHardwareEncoderDescriptions;
    bool h264SoftwareEncoderFallback = false;
    std::string desktopCapturePreference;
    std::string videoEncoderPreference;
    std::string videoDecoderPreference;
    std::vector<std::string> videoEncoderRuntimeDetails;
    std::string videoEncoderLastFallbackReason;
    std::vector<std::string> h264HardwareEncoderDescriptions;
    std::vector<std::string> h264HardwareEncoderWarnings;
    bool mfD3D11DecoderConfigured = false;
    bool mfD3D11DecoderHardware = false;
    bool mfD3D11DecoderSoftware = false;
    bool d3d11NativeDecoderOutput = false;
    bool mfD3D11DecoderAsynchronous = false;
    bool ffmpegSoftwareH264Decoder = false;
    std::string mfD3D11DecoderName;
    std::string mfD3D11DecoderError;
    std::string hardwareFingerprint;
    std::string operatingSystemDescription;
    std::string nativeArchitecture;
    bool remoteSession = false;
    std::vector<std::string> graphicsAdapterDescriptions;
    std::string graphicsEnumerationError;
    bool h264HardwareEncoderProbeSucceeded = false;
    bool h264HardwareEncoderProbeFromCache = false;
    bool audioDeviceModuleCreated = false;
    std::string audioDeviceError;
    std::string error;
};

struct SessionEngineSnapshot {
    SessionEngineState state = SessionEngineState::kStopped;
    SessionConnectivityState connectivity =
        SessionConnectivityState::kNotConfigured;
    SessionPurpose purpose = SessionPurpose::kNone;
    SessionOrigin origin = SessionOrigin::kNone;
    RemoteControlRole remoteControlRole = RemoteControlRole::kNone;
    LocalCameraState localCamera = LocalCameraState::kOff;
    LocalMicrophoneState localMicrophone = LocalMicrophoneState::kOff;
    bool roomAudioPlaybackMuted = false;
    // True only after the signaling server's control-grant event has supplied
    // the lease token used to authenticate input and clipboard packets.
    bool roomControlGrantActive = false;
    bool remoteCameraPublishing = false;
    std::string localDeviceId;
    std::string localVerificationCode;
    std::string sessionId;
    std::string peerDeviceId;
    // True after signaling has accepted the direct-session authorization and
    // issued session-ready data. WebRTC may still be negotiating.
    bool directSignalingSessionReady = false;
    bool directMediaSlotsPrepared = false;
    bool directControlReliableChannelOpen = false;
    bool directInputFastChannelOpen = false;
    bool directFileTransferChannelOpen = false;
    bool directClipboardReliableChannelOpen = false;
    bool directClipboardTransferChannelOpen = false;
    // Remains true across temporary signaling/P2P interruptions after this
    // direct session has reached Connected at least once.
    bool directSessionEverActive = false;
    // Recovery telemetry for owned-device and verification-code sessions.
    // Initial negotiation also uses kConnecting, so the UI combines this
    // attempt count with its own "was active" latch before showing recovery.
    std::uint32_t directIceRestartAttempt = 0;
    DisplayDescriptor directRemoteDisplay;
    std::uint64_t directRemoteDisplayLayoutVersion = 0;
    std::uint64_t directRemoteScreenShareGeneration = 0;
    bool directScreenPreferencePending = false;
    std::uint64_t directScreenPreferenceSequence = 0;
    std::uint32_t directScreenWidth = 0;
    std::uint32_t directScreenHeight = 0;
    std::uint32_t directScreenFramesPerSecond = kDefaultScreenFrameRate;
    std::uint32_t directScreenMaximumFrameRate = kMaximumScreenFrameRate;
    std::uint32_t directScreenMaxBitrateBps = 0;
    bool directRemoteDisplayCatalogReported = false;
    std::uint64_t directRemoteDisplayCatalogLayoutVersion = 0;
    std::vector<DisplayDescriptor> directRemoteDisplays;
    bool directRemoteDisplaySwitchPending = false;
    std::uint64_t directRemoteDisplaySwitchSequence = 0;
    std::string directRemoteDisplaySwitchError;
    std::string errorCode;
    std::string errorMessage;
    std::uint64_t screenShareGeneration = 0;
    DisplayTopologySnapshot localDisplayTopology;
    MediaDeviceSnapshot localMediaDevices;
    std::string selectedLocalDisplayKey;
    DisplayDescriptor activeSharedDisplay;
    std::uint64_t activeSharedDisplayLayoutVersion = 0;
    RoomSnapshot room;
    std::vector<RoomJoinRequest> incomingRoomJoinRequests;
    std::vector<RoomScreenShareSwitchRequest>
        incomingRoomScreenShareSwitchRequests;
    std::vector<RoomControlRequest> incomingRoomControlRequests;
    std::vector<RoomScreenShareViewRequest>
        incomingRoomScreenShareViewRequests;
    std::vector<RoomMemberActionResult> roomMemberActionResults;
    std::string outgoingRoomScreenShareSwitchRequestId;
    std::vector<RoomPeerConnectionSnapshot> roomPeerConnections;
    std::vector<RoomAvailabilitySnapshot> roomAvailabilities;
    std::uint64_t ownedDevicesRevision = 0;
    bool ownedDevicesLoaded = false;
    std::vector<OwnedDeviceSnapshot> ownedDevices;
};

struct SessionCommandResult {
    bool accepted = false;
    std::string errorCode;
    std::string errorMessage;
};

class ISessionEngineObserver {
public:
    virtual ~ISessionEngineObserver() = default;

    // V1 callbacks are synchronous on the thread that changes engine state.
    // That may be the Qt signaling thread or the session controller executor;
    // UI observers must marshal the immutable snapshot to the UI thread. The
    // future IPC implementation keeps this interface and the same payload.
    virtual void OnSessionEngineSnapshot(
        const SessionEngineSnapshot& snapshot) = 0;
};

class ISessionEngine {
public:
    virtual ~ISessionEngine() = default;

    virtual void SetObserver(ISessionEngineObserver* observer) = 0;
    virtual SessionCommandResult Start() = 0;
    virtual void Stop() = 0;

    virtual SessionEngineSnapshot Snapshot() const = 0;
    virtual SessionEngineCapabilities Capabilities() const = 0;
    virtual SessionDiagnosticsSnapshot Diagnostics() const { return {}; }

    virtual SessionCommandResult ConnectDirectDevice(
        const DirectSessionConnectRequest& request) = 0;

    // Compatibility entry points. Implementations translate these to one
    // validated DirectSessionConnectRequest so media setup has one path.
    virtual SessionCommandResult ConnectDevice(
        const std::string& deviceId,
        SessionPurpose purpose) = 0;
    virtual SessionCommandResult ConnectOwnedDevice(
        const std::string& deviceId,
        SessionPurpose purpose) = 0;
    virtual SessionCommandResult ConnectAssistedDevice(
        const std::string& deviceId,
        const std::string& verificationCode) = 0;
    virtual SessionCommandResult RefreshOwnedDevices() = 0;
    virtual SessionCommandResult AcceptIncomingSession(
        const std::string& sessionId) = 0;
    virtual SessionCommandResult RejectIncomingSession(
        const std::string& sessionId) = 0;
    virtual SessionCommandResult Disconnect() = 0;

    // Room control-plane operations are intentionally independent from the
    // legacy direct-session API during migration. Room membership and
    // member-pair WebRTC connections use the same engine snapshot without
    // overwriting the verified direct-session fields above.
    virtual SessionCommandResult CreateRoom(std::uint32_t capacity) = 0;
    virtual SessionCommandResult JoinRoom(const std::string& roomId) = 0;
    virtual SessionCommandResult QueryRoomAvailability(
        const std::vector<std::string>& roomIds)
    {
        (void)roomIds;
        return {false, "room_availability_unsupported",
                "Room availability queries are not supported."};
    }
    virtual SessionCommandResult RespondToRoomJoin(
        const std::string& requestId,
        bool accepted) = 0;
    virtual SessionCommandResult SetRoomCapacity(
        std::uint32_t capacity) = 0;
    virtual SessionCommandResult LeaveRoom() = 0;
    virtual SessionCommandResult RefreshLocalDisplays() = 0;
    virtual SessionCommandResult SelectRoomScreenShareDisplay(
        const std::string& stableDisplayKey) = 0;
    virtual SessionCommandResult StartRoomScreenShare() = 0;
    virtual SessionCommandResult StopRoomScreenShare() = 0;
    virtual SessionCommandResult RespondToRoomScreenShareSwitch(
        const std::string& requestId,
        bool accepted) = 0;
    virtual SessionCommandResult CancelRoomScreenShareSwitch() = 0;
    virtual SessionCommandResult RequestRoomControl() = 0;
    virtual SessionCommandResult RespondToRoomControl(
        const std::string& requestId,
        bool accepted) = 0;
    virtual SessionCommandResult ReleaseRoomControl() = 0;
    virtual SessionCommandResult RequestRoomMemberScreenShare(
        const std::string& peerDeviceId) = 0;
    virtual SessionCommandResult RespondToRoomMemberScreenShare(
        const std::string& requesterDeviceId,
        std::uint64_t sequence,
        bool accepted) = 0;
    virtual SessionCommandResult RequestRoomMemberMicrophoneMute(
        const std::string& peerDeviceId) = 0;
    virtual SessionCommandResult RequestRemoteRoomScreenShareStop(
        const std::string& peerDeviceId,
        std::uint64_t screenShareEpoch) = 0;
    virtual SessionCommandResult SendRoomInput(
        const RemoteInputEvent& event) = 0;
    virtual SessionCommandResult SetRoomScreenFrameRate(
        const std::string& pairId,
        std::uint32_t framesPerSecond) = 0;
    virtual SessionCommandResult SetRoomScreenStreamPreference(
        const std::string& pairId,
        const ScreenStreamPreferenceRequest& preference) = 0;
    virtual SessionCommandResult RequestRemoteSharedDisplaySwitch(
        const std::string& pairId,
        const std::string& stableDisplayKey) = 0;
    virtual SessionCommandResult SendRoomFileMessage(
        const std::string& peerDeviceId,
        const FileTransferMessage& message) = 0;
    virtual SessionCommandResult SendRoomClipboardMessage(
        const std::string& peerDeviceId,
        const std::string& clipboardSessionId,
        const ClipboardMessage& message) = 0;

    // A peer may only enable its own camera. Enabling starts local preview and
    // publishes the local Camera Track; disabling stops capture and sending.
    virtual SessionCommandResult SetLocalCameraEnabled(bool enabled) = 0;
    virtual SessionCommandResult SetLocalMicrophoneEnabled(bool enabled) = 0;
    virtual SessionCommandResult SetRoomAudioPlaybackMuted(bool muted) = 0;
    virtual SessionCommandResult RefreshLocalMediaDevices() = 0;
    virtual SessionCommandResult SelectLocalCameraDevice(
        const std::string& deviceId) = 0;
    virtual SessionCommandResult SelectLocalMicrophoneDevice(
        const std::string& deviceId) = 0;
    virtual SessionCommandResult SelectLocalSpeakerDevice(
        const std::string& deviceId) = 0;
};

}  // namespace remote
