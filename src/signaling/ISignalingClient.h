// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "src/core/ISessionEngine.h"

namespace remote {

enum class SignalingConnectionState {
    kDisconnected,
    kConnecting,
    kAuthenticating,
    kRegistering,
    kRegistered,
    kClosing,
    kFailed,
};

enum class SignalingAuthenticationMode {
    // Transitional HMAC test-token path. The bearer token is supplied in the
    // WebSocket upgrade request and verified before the socket is accepted.
    kLegacyUpgradeBearer,
    // Production Logto path. The WebSocket opens without an Authorization
    // header, then sends an auth envelope and waits for auth_success before
    // registering the device.
    kMessageAccessToken,
};

struct SignalingCapabilities {
    bool screenCapture = true;
    bool cameraCapture = true;
    bool inputControl = true;
    bool h264Encode = false;
    bool h264Decode = false;
    bool d3d11NativeDecode = false;
    std::uint32_t protocolVersion = 5;
};

struct SignalingClientConfig {
    std::string endpoint;
    std::string accessToken;
    SignalingAuthenticationMode authenticationMode =
        SignalingAuthenticationMode::kLegacyUpgradeBearer;
    std::string deviceId;
    std::string deviceName;
    // Six-digit code generated locally for this process. It is never stored
    // by the signaling server and is only relayed over authenticated WSS.
    std::string deviceVerificationCode;
    std::string appVersion;
    // Optional PEM CA bundle for local/private deployments. Supplying it adds
    // a trust anchor; certificate and hostname verification remain enabled.
    std::string trustedCaPem;
    std::uint32_t heartbeatIntervalMs = 15000;
    std::uint32_t heartbeatTimeoutMs = 10000;
    std::uint32_t authenticationTimeoutMs = 10000;
    std::uint32_t reconnectInitialDelayMs = 1000;
    std::uint32_t reconnectMaximumDelayMs = 10000;
    // Zero disables automatic reconnect.
    std::uint32_t reconnectAttemptLimit = 8;
    SignalingCapabilities capabilities;
};

struct SignalingOperationResult {
    bool accepted = false;
    std::string errorCode;
    std::string errorMessage;
};

struct SignalingAccountDeletionResult {
    bool deleted = false;
    std::string errorCode;
    std::string errorMessage;
    bool retryable = false;
};

struct IncomingSessionRequest {
    std::string sessionId;
    std::string requesterDeviceId;
    SessionPurpose purpose = SessionPurpose::kNone;
    std::vector<std::string> requestedPermissions;
    bool sameAccount = false;
    bool autoAccept = false;
    DirectAuthorizationMethod authorization =
        DirectAuthorizationMethod::kManualApproval;
    std::string verificationCode;
};

struct SignalingOwnedDevice {
    std::string deviceId;
    std::string deviceName;
    bool online = false;
    bool current = false;
    std::int64_t createdAt = 0;
    std::int64_t lastSeenAt = 0;
};

struct SignalingOwnedDevicesSnapshot {
    std::uint64_t revision = 0;
    std::vector<SignalingOwnedDevice> devices;
};

struct SignalingSessionResponse {
    std::string sessionId;
    bool accepted = false;
    std::string reasonCode;
    std::string reasonMessage;
};

struct SignalingSessionPending {
    std::string sessionId;
    std::string peerDeviceId;
};

enum class SignalingSessionEndKind {
    kCancelled,
    kClosed,
};

struct SignalingSessionEnded {
    std::string sessionId;
    std::string initiatorDeviceId;
    std::string reasonCode;
    SignalingSessionEndKind kind = SignalingSessionEndKind::kClosed;
};

struct SignalingIceServer {
    std::vector<std::string> urls;
    std::string username;
    std::string credential;
};

struct SignalingSessionReady {
    std::string sessionId;
    std::string peerDeviceId;
    std::string recoveryToken;
    std::vector<SignalingIceServer> iceServers;
};

struct SignalingSessionSuspended {
    std::string sessionId;
    std::string peerDeviceId;
    std::uint32_t recoveryWindowMs = 0;
};

struct SignalingSessionResumed {
    std::string sessionId;
    std::string peerDeviceId;
    std::string resumedDeviceId;
};

struct SignalingSessionDescription {
    std::string sessionId;
    std::string type;
    std::string sdp;
    std::uint64_t negotiationGeneration = 1;
};

struct SignalingIceCandidate {
    std::string sessionId;
    std::string candidate;
    std::string sdpMid;
    std::int32_t sdpMLineIndex = -1;
    std::uint64_t negotiationGeneration = 1;
};

struct SignalingIceRestartRequest {
    std::string sessionId;
    std::uint64_t observedGeneration = 1;
    std::uint64_t requestSequence = 0;
};

struct SignalingIceRestartCancel {
    std::string sessionId;
    std::uint64_t observedGeneration = 1;
    std::uint64_t requestSequence = 0;
};

struct SignalingRoomReady {
    RoomSnapshot room;
    std::string recoveryToken;
};

struct SignalingRoomJoinPending {
    std::string roomId;
    std::string requestId;
};

struct SignalingRoomJoinResult {
    std::string roomId;
    std::string requestId;
    bool accepted = false;
    std::string reasonCode;
    std::string reasonMessage;
};

struct SignalingRoomAvailability {
    std::string roomId;
    bool exists = false;
    bool joinable = false;
};

struct SignalingRoomAvailabilityResult {
    std::vector<SignalingRoomAvailability> rooms;
};

struct SignalingRoomClosed {
    std::string roomId;
    std::string initiatorDeviceId;
    std::string reasonCode;
};

struct SignalingRoomPairReady {
    std::string pairId;
    std::string roomId;
    std::string peerDeviceId;
    bool localIsOfferer = false;
    std::vector<SignalingIceServer> iceServers;
};

struct SignalingRoomPairClosed {
    std::string pairId;
    std::string roomId;
    std::string peerDeviceId;
    std::string initiatorDeviceId;
    std::string reasonCode;
};

struct SignalingRoomScreenShareGranted {
    std::string roomId;
    std::string grantId;
    std::uint64_t epoch = 0;
};

struct SignalingRoomScreenShareSwitchPending {
    std::string roomId;
    std::string requestId;
    std::string screenSharerDeviceId;
};

struct SignalingRoomScreenShareSwitchResult {
    std::string roomId;
    std::string requestId;
    bool accepted = false;
    std::string reasonCode;
    std::string reasonMessage;
};

struct SignalingRoomControlResult {
    std::string roomId;
    std::string requestId;
    bool accepted = false;
    std::string reasonCode;
    std::string reasonMessage;
};

struct SignalingRoomControlGranted {
    std::string roomId;
    std::string grantId;
    std::string screenSharerDeviceId;
    std::string controllerDeviceId;
};

struct SignalingRoomControlRevoked {
    std::string roomId;
    std::string initiatorDeviceId;
    std::string reasonCode;
};

class ISignalingClientObserver {
public:
    virtual ~ISignalingClientObserver() = default;

    virtual void OnSignalingStateChanged(
        SignalingConnectionState state) = 0;
    virtual void OnDeviceRegistered(const std::string& deviceId) = 0;
    virtual void OnIncomingSessionRequest(
        const IncomingSessionRequest& request) = 0;
    virtual void OnSessionResponse(
        const SignalingSessionResponse& response) = 0;
    virtual void OnSessionPending(
        const SignalingSessionPending& pending) = 0;
    virtual void OnSessionReady(const SignalingSessionReady& ready) = 0;
    virtual void OnSessionSuspended(
        const SignalingSessionSuspended& suspended) = 0;
    virtual void OnSessionResumed(
        const SignalingSessionResumed& resumed) = 0;
    virtual void OnSessionEnded(const SignalingSessionEnded& ended) = 0;
    virtual void OnRemoteDescription(
        const SignalingSessionDescription& description) = 0;
    virtual void OnRemoteIceCandidate(
        const SignalingIceCandidate& candidate) = 0;
    virtual void OnIceRestartRequested(
        const SignalingIceRestartRequest& request)
    {
        (void)request;
    }
    virtual void OnIceRestartCancelled(
        const SignalingIceRestartCancel& cancel)
    {
        (void)cancel;
    }
    virtual void OnHeartbeatAcknowledged(std::uint32_t roundTripMs) = 0;
    virtual void OnSignalingError(const std::string& code,
                                  const std::string& message) = 0;
    virtual void OnAccountDeletionResult(
        const SignalingAccountDeletionResult& result)
    {
        (void)result;
    }
    virtual void OnOwnedDevicesChanged(
        const SignalingOwnedDevicesSnapshot& snapshot)
    {
        (void)snapshot;
    }

    // Room callbacks have default no-op implementations so legacy 1V1 test
    // observers remain source-compatible while the room path is introduced.
    virtual void OnRoomReady(const SignalingRoomReady& ready)
    {
        (void)ready;
    }
    virtual void OnRoomState(const RoomSnapshot& room)
    {
        (void)room;
    }
    virtual void OnRoomJoinPending(
        const SignalingRoomJoinPending& pending)
    {
        (void)pending;
    }
    virtual void OnRoomJoinRequested(const RoomJoinRequest& request)
    {
        (void)request;
    }
    virtual void OnRoomJoinResult(
        const SignalingRoomJoinResult& result)
    {
        (void)result;
    }
    virtual void OnRoomAvailabilityResult(
        const SignalingRoomAvailabilityResult& result)
    {
        (void)result;
    }
    virtual void OnRoomClosed(const SignalingRoomClosed& closed)
    {
        (void)closed;
    }
    virtual void OnRoomPairReady(const SignalingRoomPairReady& ready)
    {
        (void)ready;
    }
    virtual void OnRoomPairClosed(const SignalingRoomPairClosed& closed)
    {
        (void)closed;
    }
    virtual void OnRoomScreenShareGranted(
        const SignalingRoomScreenShareGranted& granted)
    {
        (void)granted;
    }
    virtual void OnRoomScreenShareSwitchPending(
        const SignalingRoomScreenShareSwitchPending& pending)
    {
        (void)pending;
    }
    virtual void OnRoomScreenShareSwitchRequested(
        const RoomScreenShareSwitchRequest& request)
    {
        (void)request;
    }
    virtual void OnRoomScreenShareSwitchResult(
        const SignalingRoomScreenShareSwitchResult& result)
    {
        (void)result;
    }
    virtual void OnRoomControlRequested(const RoomControlRequest& request)
    {
        (void)request;
    }
    virtual void OnRoomControlResult(
        const SignalingRoomControlResult& result)
    {
        (void)result;
    }
    virtual void OnRoomControlGranted(
        const SignalingRoomControlGranted& granted)
    {
        (void)granted;
    }
    virtual void OnRoomControlRevoked(
        const SignalingRoomControlRevoked& revoked)
    {
        (void)revoked;
    }
};

class ISignalingClient {
public:
    virtual ~ISignalingClient() = default;

    virtual void SetObserver(ISignalingClientObserver* observer) = 0;
    virtual SignalingOperationResult Connect(
        const SignalingClientConfig& config) = 0;
    // Replaces the in-memory token used by the next message authentication or
    // reconnect. It deliberately does not tear down an active WebRTC session.
    virtual SignalingOperationResult UpdateAccessToken(
        const std::string& accessToken) = 0;
    virtual SignalingOperationResult RequestAccountDeletion()
    {
        return {false, "account_deletion_unsupported",
                "Account deletion is not supported."};
    }
    virtual SignalingOperationResult RequestOwnedDevices()
    {
        return {false, "my_devices_unsupported",
                "Owned-device discovery is not supported."};
    }
    virtual void Disconnect() = 0;
    virtual SignalingConnectionState State() const = 0;

    virtual SignalingOperationResult RequestSession(
        const std::string& targetDeviceId,
        SessionPurpose purpose,
        const std::vector<std::string>& permissions) = 0;
    virtual SignalingOperationResult RequestOwnedDeviceSession(
        const std::string& targetDeviceId,
        SessionPurpose purpose,
        const std::vector<std::string>& permissions)
    {
        return RequestSession(targetDeviceId, purpose, permissions);
    }
    virtual SignalingOperationResult RequestAssistedSession(
        const std::string& targetDeviceId,
        const std::string& verificationCode,
        const std::vector<std::string>& permissions)
    {
        (void)verificationCode;
        return RequestSession(
            targetDeviceId, SessionPurpose::kRemoteControl, permissions);
    }
    virtual SignalingOperationResult RespondToSession(
        const std::string& sessionId,
        bool accepted,
        const std::string& reasonCode) = 0;
    virtual SignalingOperationResult CancelSession(
        const std::string& sessionId,
        const std::string& reasonCode) = 0;
    virtual SignalingOperationResult CloseSession(
        const std::string& sessionId,
        const std::string& reasonCode) = 0;
    virtual SignalingOperationResult ResumeSession(
        const std::string& sessionId,
        const std::string& recoveryToken) = 0;

    virtual SignalingOperationResult CreateRoom(
        std::uint32_t capacity) = 0;
    virtual SignalingOperationResult RequestRoomJoin(
        const std::string& roomId) = 0;
    virtual SignalingOperationResult QueryRoomAvailability(
        const std::vector<std::string>& roomIds)
    {
        (void)roomIds;
        return {false, "room_availability_unsupported",
                "Room availability queries are not supported."};
    }
    virtual SignalingOperationResult RespondToRoomJoin(
        const std::string& roomId,
        const std::string& requestId,
        bool accepted,
        const std::string& reasonCode) = 0;
    virtual SignalingOperationResult SetRoomCapacity(
        const std::string& roomId,
        std::uint32_t capacity) = 0;
    virtual SignalingOperationResult LeaveRoom(
        const std::string& roomId,
        const std::string& reasonCode) = 0;
    virtual SignalingOperationResult ResumeRoom(
        const std::string& roomId,
        const std::string& recoveryToken) = 0;
    virtual SignalingOperationResult SetRoomMediaState(
        const std::string& roomId,
        bool cameraPublishing,
        bool microphonePublishing) = 0;
    virtual SignalingOperationResult RequestRoomScreenShare(
        const std::string& roomId) = 0;
    virtual SignalingOperationResult ConfirmRoomScreenShare(
        const std::string& roomId,
        const std::string& grantId) = 0;
    virtual SignalingOperationResult StopRoomScreenShare(
        const std::string& roomId,
        const std::string& grantId,
        const std::string& reasonCode) = 0;
    virtual SignalingOperationResult RespondToRoomScreenShareSwitch(
        const std::string& roomId,
        const std::string& requestId,
        bool accepted,
        const std::string& reasonCode)
    {
        (void)roomId;
        (void)requestId;
        (void)accepted;
        (void)reasonCode;
        return {false, "room_screen_share_switch_unsupported",
                "Screen-share takeover approval is not supported."};
    }
    virtual SignalingOperationResult CancelRoomScreenShareSwitch(
        const std::string& roomId,
        const std::string& requestId,
        const std::string& reasonCode)
    {
        (void)roomId;
        (void)requestId;
        (void)reasonCode;
        return {false, "room_screen_share_switch_unsupported",
                "Screen-share takeover cancellation is not supported."};
    }
    virtual SignalingOperationResult RequestRoomControl(
        const std::string& roomId) = 0;
    virtual SignalingOperationResult RespondToRoomControl(
        const std::string& roomId,
        const std::string& requestId,
        bool accepted,
        const std::string& reasonCode) = 0;
    virtual SignalingOperationResult ReleaseRoomControl(
        const std::string& roomId,
        const std::string& grantId,
        const std::string& reasonCode) = 0;

    virtual SignalingOperationResult SendDescription(
        const SignalingSessionDescription& description) = 0;
    virtual SignalingOperationResult SendIceCandidate(
        const SignalingIceCandidate& candidate) = 0;
    virtual SignalingOperationResult SendIceRestartRequest(
        const SignalingIceRestartRequest& request)
    {
        (void)request;
        return {false, "ice_restart_unsupported",
                "ICE restart signaling is not supported."};
    }
    virtual SignalingOperationResult SendIceRestartCancel(
        const SignalingIceRestartCancel& cancel)
    {
        (void)cancel;
        return {false, "ice_restart_unsupported",
                "ICE restart signaling is not supported."};
    }
};

}  // namespace remote
