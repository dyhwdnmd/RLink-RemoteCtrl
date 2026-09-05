// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#pragma once

#include <chrono>
#include <cstdint>
#include <functional>
#include <memory>
#include <optional>
#include <span>
#include <string>
#include <vector>

#include "src/webrtc/IWebRtcSession.h"

namespace remote {

enum class SessionControllerState {
    kIdle,
    kStarting,
    kReady,
    kNegotiating,
    kConnected,
    kDisconnected,
    kWaitingForSignaling,
    kRestartingIce,
    kFailed,
    kClosing,
    kClosed,
};

struct SessionControllerConfig {
    WebRtcSessionConfig webRtc;
    std::chrono::milliseconds negotiationTimeout{10000};
    std::chrono::milliseconds reconnectTimeout{60000};
    std::chrono::milliseconds disconnectedGracePeriod{3000};
    std::chrono::milliseconds iceRestartTimeout{10000};
    std::chrono::milliseconds transportVerificationDelay{350};
    std::chrono::milliseconds transportVerificationTimeout{2000};
    std::chrono::milliseconds remoteIceRestartRequestDebounce{1000};
    std::chrono::milliseconds connectedStabilityPeriod{2000};
    std::uint32_t maximumIceRestartAttempts = 3;
};

struct SessionControllerSnapshot {
    SessionControllerState state = SessionControllerState::kIdle;
    WebRtcSessionState webRtcState = WebRtcSessionState::kNew;
    std::string errorCode;
    std::string errorMessage;
    std::uint64_t negotiationGeneration = 1;
    std::uint32_t iceRestartAttempt = 0;
};

class ISessionSignalingSender {
public:
    virtual ~ISessionSignalingSender() = default;
    virtual bool SendDescription(
        const SessionDescription& description) = 0;
    virtual bool SendIceCandidate(const IceCandidate& candidate) = 0;
    virtual bool RequestIceRestart(std::uint64_t observedGeneration,
                                   std::uint64_t requestSequence)
    {
        (void)observedGeneration;
        (void)requestSequence;
        return false;
    }
    virtual bool CancelIceRestart(std::uint64_t observedGeneration,
                                  std::uint64_t requestSequence)
    {
        (void)observedGeneration;
        (void)requestSequence;
        return false;
    }
};

class ISessionControllerObserver {
public:
    virtual ~ISessionControllerObserver() = default;

    // Callbacks run on the controller's serial executor. Implementations must
    // return quickly and post UI or business work to the appropriate thread.
    virtual void OnControllerSnapshot(
        const SessionControllerSnapshot& snapshot) = 0;
    virtual void OnDataChannelStateChanged(
        const DataChannelInfo& channel) = 0;
    virtual void OnDataMessage(const std::string& label,
                               std::span<const std::uint8_t> payload,
                               bool binary) = 0;
    virtual void OnRemoteTrackAdded(const RemoteTrackInfo& track) = 0;
};

class SessionControllerBase : public IWebRtcSessionObserver {
public:
    ~SessionControllerBase() override;

    SessionControllerBase(const SessionControllerBase&) = delete;
    SessionControllerBase& operator=(const SessionControllerBase&) = delete;

    void SetObserver(ISessionControllerObserver* observer);
    void Start(const SessionControllerConfig& config);
    void HandleRemoteDescription(const SessionDescription& description);
    void HandleRemoteIceCandidate(const IceCandidate& candidate);
    void HandleRemoteIceRestartRequest(std::uint64_t observedGeneration,
                                       std::uint64_t requestSequence);
    void HandleRemoteIceRestartCancel(std::uint64_t observedGeneration,
                                      std::uint64_t requestSequence);
    void SetSignalingAvailable(bool available);
    SendResult SendData(const std::string& channelName,
                        std::span<const std::uint8_t> data,
                        bool binary);
    bool QueueData(
        const std::string& channelName,
        std::span<const std::uint8_t> data,
        bool binary,
        std::function<void(SendResult)> completion = {});
    void Close();
    SessionControllerSnapshot Snapshot() const;

    void OnSessionStateChanged(WebRtcSessionState state) final;
    void OnIceGatheringStateChanged(
        WebRtcIceGatheringState state) final;
    void OnLocalDescription(
        const SessionDescription& description) final;
    void OnLocalIceCandidate(const IceCandidate& candidate) final;
    void OnDataChannelStateChanged(
        const DataChannelInfo& channel) final;
    void OnDataMessage(const std::string& label,
                       std::span<const std::uint8_t> payload,
                       bool binary) final;
    void OnRemoteTrackAdded(const RemoteTrackInfo& track) final;
    void OnOperationCompleted(OperationId operationId) final;
    void OnWebRtcError(OperationId operationId,
                       const OperationError& error) final;

protected:
    enum class Role {
        kController,
        kAgent,
    };

    SessionControllerBase(Role role,
                          IWebRtcSession& session,
                          ISessionSignalingSender& signaling);
    void BeginNegotiation(
        const std::vector<DataChannelSpec>& dataChannels);
    void SetAgentAnswerPreparation(
        std::function<std::optional<OperationError>()> preparation);

private:
    class Impl;
    std::unique_ptr<Impl> impl_;
};

class ControllerSessionController final : public SessionControllerBase {
public:
    ControllerSessionController(IWebRtcSession& session,
                                ISessionSignalingSender& signaling);

    void Connect(const std::vector<DataChannelSpec>& dataChannels);
};

class AgentSessionController final : public SessionControllerBase {
public:
    AgentSessionController(IWebRtcSession& session,
                           ISessionSignalingSender& signaling);

    // Runs after the remote Offer has been applied and before CreateAnswer().
    // Agent media tracks should be attached here so they bind to the offered
    // transceiver instead of creating an unrelated m-line.
    void SetAnswerPreparation(
        std::function<std::optional<OperationError>()> preparation);
};

}  // namespace remote
