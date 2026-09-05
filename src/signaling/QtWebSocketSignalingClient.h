// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#pragma once

#include <memory>

#include "ISignalingClient.h"

namespace remote {

class QtWebSocketSignalingClient final : public ISignalingClient {
public:
    QtWebSocketSignalingClient();
    ~QtWebSocketSignalingClient() override;

    QtWebSocketSignalingClient(const QtWebSocketSignalingClient&) = delete;
    QtWebSocketSignalingClient& operator=(
        const QtWebSocketSignalingClient&) = delete;

    void SetObserver(ISignalingClientObserver* observer) override;
    SignalingOperationResult Connect(
        const SignalingClientConfig& config) override;
    SignalingOperationResult UpdateAccessToken(
        const std::string& accessToken) override;
    SignalingOperationResult RequestAccountDeletion() override;
    SignalingOperationResult RequestOwnedDevices() override;
    void Disconnect() override;
    SignalingConnectionState State() const override;

    SignalingOperationResult RequestSession(
        const std::string& targetDeviceId,
        SessionPurpose purpose,
        const std::vector<std::string>& permissions) override;
    SignalingOperationResult RequestOwnedDeviceSession(
        const std::string& targetDeviceId,
        SessionPurpose purpose,
        const std::vector<std::string>& permissions) override;
    SignalingOperationResult RequestAssistedSession(
        const std::string& targetDeviceId,
        const std::string& verificationCode,
        const std::vector<std::string>& permissions) override;
    SignalingOperationResult RespondToSession(
        const std::string& sessionId,
        bool accepted,
        const std::string& reasonCode) override;
    SignalingOperationResult CancelSession(
        const std::string& sessionId,
        const std::string& reasonCode) override;
    SignalingOperationResult CloseSession(
        const std::string& sessionId,
        const std::string& reasonCode) override;
    SignalingOperationResult ResumeSession(
        const std::string& sessionId,
        const std::string& recoveryToken) override;
    SignalingOperationResult CreateRoom(
        std::uint32_t capacity) override;
    SignalingOperationResult RequestRoomJoin(
        const std::string& roomId) override;
    SignalingOperationResult QueryRoomAvailability(
        const std::vector<std::string>& roomIds) override;
    SignalingOperationResult RespondToRoomJoin(
        const std::string& roomId,
        const std::string& requestId,
        bool accepted,
        const std::string& reasonCode) override;
    SignalingOperationResult SetRoomCapacity(
        const std::string& roomId,
        std::uint32_t capacity) override;
    SignalingOperationResult LeaveRoom(
        const std::string& roomId,
        const std::string& reasonCode) override;
    SignalingOperationResult ResumeRoom(
        const std::string& roomId,
        const std::string& recoveryToken) override;
    SignalingOperationResult SetRoomMediaState(
        const std::string& roomId,
        bool cameraPublishing,
        bool microphonePublishing) override;
    SignalingOperationResult RequestRoomScreenShare(
        const std::string& roomId) override;
    SignalingOperationResult ConfirmRoomScreenShare(
        const std::string& roomId,
        const std::string& grantId) override;
    SignalingOperationResult StopRoomScreenShare(
        const std::string& roomId,
        const std::string& grantId,
        const std::string& reasonCode) override;
    SignalingOperationResult RespondToRoomScreenShareSwitch(
        const std::string& roomId,
        const std::string& requestId,
        bool accepted,
        const std::string& reasonCode) override;
    SignalingOperationResult CancelRoomScreenShareSwitch(
        const std::string& roomId,
        const std::string& requestId,
        const std::string& reasonCode) override;
    SignalingOperationResult RequestRoomControl(
        const std::string& roomId) override;
    SignalingOperationResult RespondToRoomControl(
        const std::string& roomId,
        const std::string& requestId,
        bool accepted,
        const std::string& reasonCode) override;
    SignalingOperationResult ReleaseRoomControl(
        const std::string& roomId,
        const std::string& grantId,
        const std::string& reasonCode) override;
    SignalingOperationResult SendDescription(
        const SignalingSessionDescription& description) override;
    SignalingOperationResult SendIceCandidate(
        const SignalingIceCandidate& candidate) override;
    SignalingOperationResult SendIceRestartRequest(
        const SignalingIceRestartRequest& request) override;
    SignalingOperationResult SendIceRestartCancel(
        const SignalingIceRestartCancel& cancel) override;

    // Integration tests use this to emulate a transient transport failure
    // without stopping the signaling server or the active PeerConnection.
    void AbortConnectionForTesting();

private:
    class Impl;
    std::unique_ptr<Impl> impl_;
};

}  // namespace remote
