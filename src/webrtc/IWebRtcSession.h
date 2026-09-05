// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#pragma once

#include <cstdint>
#include <optional>
#include <span>
#include <string>
#include <vector>

#include "src/core/SessionDiagnostics.h"

namespace remote {

using OperationId = std::uint64_t;

inline constexpr char kScreenMainVideoSlot[] = "screen-main";
inline constexpr char kCameraMainVideoSlot[] = "camera-main";
inline constexpr char kMicrophoneMainAudioSlot[] = "microphone-main";
inline constexpr int kDefaultIceMinPort = 50000;
inline constexpr int kDefaultIceMaxPort = 50020;

enum class SessionDescriptionType {
    kOffer,
    kAnswer,
};

enum class WebRtcSessionState {
    kNew,
    kStarting,
    kReady,
    kConnecting,
    kConnected,
    kDisconnected,
    kFailed,
    kClosing,
    kClosed,
};

enum class WebRtcIceGatheringState {
    kNew,
    kGathering,
    kComplete,
};

enum class DataChannelState {
    kConnecting,
    kOpen,
    kClosing,
    kClosed,
};

enum class SendResult {
    kSent,
    kSessionNotStarted,
    kChannelNotFound,
    kChannelNotOpen,
    kSendFailed,
};

struct SessionDescription {
    SessionDescriptionType type = SessionDescriptionType::kOffer;
    std::string sdp;
    // Monotonically increasing per PeerConnection. Generation 1 is the
    // initial negotiation; later generations are ICE restarts.
    std::uint64_t negotiationGeneration = 1;
};

struct IceCandidate {
    std::string sdpMid;
    int sdpMLineIndex = 0;
    std::string candidate;
    std::uint64_t negotiationGeneration = 1;
};

struct IceServerConfig {
    std::vector<std::string> urls;
    std::string username;
    std::string password;
};

struct WebRtcSessionConfig {
    std::vector<IceServerConfig> iceServers;
    bool includeLoopbackAdapter = false;
    bool fastDesktopBweStartup = false;
    bool adaptiveDesktopNetworkFrameRate = false;
    int iceMinPort = 0;
    int iceMaxPort = 0;
};

struct DataChannelSpec {
    enum class Priority {
        kLow,
        kMedium,
        kHigh,
    };

    std::string label;
    bool ordered = true;
    std::optional<int> maxRetransmits;
    std::optional<int> maxPacketLifeTimeMs;
    std::string protocol;
    std::optional<Priority> priority;
};

struct DataChannelInfo {
    std::string label;
    DataChannelState state = DataChannelState::kConnecting;
    bool ordered = true;
    std::optional<int> maxRetransmits;
    std::optional<int> maxPacketLifeTimeMs;
    std::string protocol;
};

struct OperationError {
    std::string code;
    std::string message;
};

struct RemoteTrackInfo {
    std::string id;
    std::string kind;
    std::string slot;
};

class IWebRtcSessionObserver {
public:
    virtual ~IWebRtcSessionObserver() = default;

    // Callbacks are emitted from libwebrtc callback threads. The observer must
    // outlive the session or be cleared with SetObserver(nullptr). The payload
    // span is valid only for the duration of OnDataMessage. Handlers must not
    // call WebRTC session methods re-entrantly or destroy the session from a
    // callback; post that work to the owning session executor instead.
    virtual void OnSessionStateChanged(WebRtcSessionState state) = 0;
    virtual void OnIceGatheringStateChanged(
        WebRtcIceGatheringState state) = 0;
    virtual void OnLocalDescription(
        const SessionDescription& description) = 0;
    virtual void OnLocalIceCandidate(const IceCandidate& candidate) = 0;
    virtual void OnDataChannelStateChanged(
        const DataChannelInfo& channel) = 0;
    virtual void OnDataMessage(const std::string& label,
                               std::span<const std::uint8_t> payload,
                               bool binary) = 0;
    virtual void OnRemoteTrackAdded(const RemoteTrackInfo& track) = 0;
    virtual void OnOperationCompleted(OperationId operationId) = 0;
    virtual void OnWebRtcError(OperationId operationId,
                               const OperationError& error) = 0;
};

class IWebRtcSession {
public:
    virtual ~IWebRtcSession() = default;

    virtual void SetObserver(IWebRtcSessionObserver* observer) = 0;
    virtual OperationId Start(const WebRtcSessionConfig& config) = 0;
    virtual OperationId CreateOffer() = 0;
    virtual OperationId CreateIceRestartOffer() = 0;
    virtual OperationId CreateAnswer() = 0;
    virtual OperationId ApplyRemoteDescription(
        const SessionDescription& description) = 0;
    virtual OperationId AddRemoteIceCandidate(
        const IceCandidate& candidate) = 0;
    virtual OperationId CreateDataChannels(
        const std::vector<DataChannelSpec>& channels) = 0;
    virtual SendResult SendData(const std::string& channelName,
                                std::span<const std::uint8_t> data,
                                bool binary) = 0;
    virtual std::optional<std::uint64_t> DataChannelBufferedAmount(
        const std::string& channelName) const = 0;
    virtual void RequestStats() {}
    virtual WebRtcSessionStatsSnapshot StatsSnapshot() const { return {}; }
    virtual void Close() = 0;
};

}  // namespace remote
