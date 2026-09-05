// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#pragma once

#include <atomic>
#include <cstdint>
#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>

#include "api/media_stream_interface.h"
#include "api/peer_connection_interface.h"
#include "api/rtc_error.h"
#include "api/rtp_transceiver_interface.h"
#include "api/scoped_refptr.h"
#include "api/video/video_frame.h"
#include "api/video/video_sink_interface.h"
#include "src/core/ScreenNetworkPolicy.h"
#include "src/webrtc/IWebRtcSession.h"

namespace remote {

class PeerConnectionStatsCollector;

class LibWebRtcSession final : public IWebRtcSession,
                               public webrtc::PeerConnectionObserver {
public:
    explicit LibWebRtcSession(
        webrtc::scoped_refptr<webrtc::PeerConnectionFactoryInterface> factory);
    ~LibWebRtcSession() override;

    LibWebRtcSession(const LibWebRtcSession&) = delete;
    LibWebRtcSession& operator=(const LibWebRtcSession&) = delete;

    void SetObserver(IWebRtcSessionObserver* observer) override;
    OperationId Start(const WebRtcSessionConfig& config) override;
    OperationId CreateOffer() override;
    OperationId CreateIceRestartOffer() override;
    OperationId CreateAnswer() override;
    OperationId ApplyRemoteDescription(
        const SessionDescription& description) override;
    OperationId AddRemoteIceCandidate(
        const IceCandidate& candidate) override;
    OperationId CreateDataChannels(
        const std::vector<DataChannelSpec>& channels) override;
    SendResult SendData(const std::string& channelName,
                        std::span<const std::uint8_t> data,
                        bool binary) override;
    std::optional<std::uint64_t> DataChannelBufferedAmount(
        const std::string& channelName) const override;
    void RequestStats() override;
    WebRtcSessionStatsSnapshot StatsSnapshot() const override;
    void Close() override;

    // Media-track adapters will own these responsibilities in the full
    // transport layer. They are exposed here now so the existing end-to-end
    // test can validate the formal session without bypassing it.
    webrtc::RTCErrorOr<
        webrtc::scoped_refptr<webrtc::RtpTransceiverInterface>>
    AddVideoTrack(
        webrtc::scoped_refptr<webrtc::MediaStreamTrackInterface> track);
    webrtc::RTCErrorOr<
        webrtc::scoped_refptr<webrtc::RtpTransceiverInterface>>
    AddVideoReceiveTransceiver();
    webrtc::RTCError PrepareVideoTransceiverSlot(
        const std::string& slot);
    webrtc::RTCError BindNegotiatedVideoTransceiverSlots(
        const std::vector<std::string>& slots);
    webrtc::RTCError SetVideoSlotTrack(
        const std::string& slot,
        webrtc::scoped_refptr<webrtc::MediaStreamTrackInterface> track);
    webrtc::RTCError SetVideoSlotSendingActive(
        const std::string& slot,
        bool active);
    void SetFastDesktopBweStartupEnabled(bool enabled);
    void SetAdaptiveDesktopNetworkFrameRateEnabled(bool enabled);
    void SetScreenContentActivity(ScreenContentActivity activity);
    void RestartVideoSlotBandwidthEstimation(const std::string& slot);
    void FinishVideoSlotBandwidthBootstrap(const std::string& slot);
    webrtc::RTCError SetVideoSlotEncodingPolicy(
        const std::string& slot,
        std::uint32_t framesPerSecond,
        std::uint32_t width,
        std::uint32_t height);
    webrtc::RTCError PrepareAudioTransceiverSlot(
        const std::string& slot);
    webrtc::RTCError BindNegotiatedAudioTransceiverSlot(
        const std::string& slot);
    webrtc::RTCError SetAudioSlotTrack(
        const std::string& slot,
        webrtc::scoped_refptr<webrtc::MediaStreamTrackInterface> track);
    void SetRemoteAudioSlotEnabled(const std::string& slot, bool enabled);
    bool AudioSlotPrepared(const std::string& slot) const;
    std::size_t PreparedVideoSlotCount() const;
    void SetRemoteVideoSink(
        webrtc::VideoSinkInterface<webrtc::VideoFrame>* sink);
    void SetRemoteVideoSlotSink(
        const std::string& slot,
        webrtc::VideoSinkInterface<webrtc::VideoFrame>* sink);

    void OnSignalingChange(
        webrtc::PeerConnectionInterface::SignalingState state) override;
    void OnDataChannel(
        webrtc::scoped_refptr<webrtc::DataChannelInterface> channel) override;
    void OnIceGatheringChange(
        webrtc::PeerConnectionInterface::IceGatheringState state) override;
    void OnIceCandidate(const webrtc::IceCandidate* candidate) override;
    void OnConnectionChange(
        webrtc::PeerConnectionInterface::PeerConnectionState state) override;
    void OnIceConnectionChange(
        webrtc::PeerConnectionInterface::IceConnectionState state) override;
    void OnTrack(
        webrtc::scoped_refptr<webrtc::RtpTransceiverInterface> transceiver)
        override;

private:
    class CallbackGate;
    class DataChannelBinding;
    struct VideoSlotBinding {
        webrtc::scoped_refptr<webrtc::RtpTransceiverInterface> transceiver;
        webrtc::scoped_refptr<webrtc::VideoTrackInterface> remoteTrack;
        webrtc::VideoSinkInterface<webrtc::VideoFrame>* remoteSink = nullptr;
        std::uint32_t configuredMaxFrameRate = 0;
        std::uint32_t configuredOutputWidth = 0;
        std::uint32_t configuredOutputHeight = 0;
        std::uint64_t configuredStartBitrateBps = 0;
        std::uint64_t configuredMaxBitrateBps = 0;
        AdaptiveScreenFrameRateState adaptiveFrameRate;
        std::uint64_t adaptiveFrameRateRevision = 0;
        std::string adaptiveFrameRateError;
        bool sendingActive = false;
        bool startBitrateBootstrapPending = true;
        std::uint32_t bitrateBootstrapAttempts = 0;
        std::uint32_t bitrateBootstrapSuccesses = 0;
        std::uint32_t mediaReadyBitrateRestarts = 0;
        std::uint32_t allocationProbePulses = 0;
        std::uint32_t bitrateProbeFloorReleases = 0;
        bool bitrateProbeFloorActive = false;
        std::string bitrateBootstrapError;
    };
    struct AudioSlotBinding {
        std::string name;
        webrtc::scoped_refptr<webrtc::RtpTransceiverInterface> transceiver;
        webrtc::scoped_refptr<webrtc::AudioTrackInterface> remoteTrack;
        bool remotePlaybackEnabled = true;
    };

    OperationId NextOperationId();
    webrtc::scoped_refptr<webrtc::PeerConnectionInterface>
        PeerConnection() const;
    void CreateLocalDescription(OperationId operationId,
                                 SessionDescriptionType type,
                                 bool iceRestart = false);
    void SetLocalDescription(
        OperationId operationId,
        SessionDescription description,
        std::unique_ptr<webrtc::SessionDescriptionInterface> nativeDescription);
    void AttachDataChannel(
        webrtc::scoped_refptr<webrtc::DataChannelInterface> channel);
    void HandleDataChannelState(
        webrtc::scoped_refptr<webrtc::DataChannelInterface> channel);
    void HandleDataMessage(const std::string& label,
                           const webrtc::DataBuffer& buffer);
    void UpdatePeerConnectionState(
        webrtc::PeerConnectionInterface::PeerConnectionState state);
    void UpdateIceConnectionState(
        webrtc::PeerConnectionInterface::IceConnectionState state);
    void ApplyPendingVideoStartBitrateBootstrap();
    void HandleCompletedStatsSample();
    webrtc::RTCError ApplyProgressiveBitrateCeilingDecision(
        const ProgressiveBitrateCeilingDecision& decision,
        std::uint64_t decisionRevision,
        const ProgressiveBitrateCeilingState& previousState);
    webrtc::RTCError ApplyAdaptiveScreenFrameRateDecision(
        const AdaptiveScreenFrameRateDecision& decision,
        std::uint64_t decisionRevision,
        const AdaptiveScreenFrameRateState& previousState,
        webrtc::scoped_refptr<webrtc::RtpTransceiverInterface> transceiver);
    bool PulseVideoSlotAllocationProbe(
        webrtc::scoped_refptr<webrtc::RtpTransceiverInterface> transceiver,
        std::uint64_t startBitrate,
        std::uint64_t maxBitrate,
        std::string* error);
    WebRtcSessionState CombinedConnectionStateLocked() const;
    void ChangeState(WebRtcSessionState state);
    void CompleteOperation(OperationId operationId);
    void FailOperation(OperationId operationId,
                       std::string code,
                       std::string message);
    IWebRtcSessionObserver* Observer() const;
    void DetachRemoteVideoSink();

    mutable std::mutex mutex_;
    // Serializes read-modify-write updates to RtpSender parameters. User
    // preference changes, share activation and stats-driven per-viewer FPS
    // adaptation may arrive on different threads.
    std::mutex videoSenderParametersMutex_;
    std::atomic<OperationId> nextOperationId_{1};
    std::shared_ptr<CallbackGate> callbackGate_;
    webrtc::scoped_refptr<webrtc::PeerConnectionFactoryInterface> factory_;
    std::unique_ptr<PeerConnectionStatsCollector> statsCollector_;
    webrtc::scoped_refptr<webrtc::PeerConnectionInterface> peerConnection_;
    IWebRtcSessionObserver* observer_ = nullptr;
    bool fastDesktopBweStartup_ = false;
    bool adaptiveDesktopNetworkFrameRate_ = false;
    ProgressiveBitrateCeilingState progressiveBitrateCeiling_;
    std::uint64_t progressiveBitrateCeilingRevision_ = 0;
    std::string progressiveBitrateCeilingError_;
    ScreenContentActivity screenContentActivity_ =
        ScreenContentActivity::kUnknown;
    WebRtcSessionState state_ = WebRtcSessionState::kNew;
    webrtc::PeerConnectionInterface::PeerConnectionState
        peerConnectionState_ =
            webrtc::PeerConnectionInterface::PeerConnectionState::kNew;
    webrtc::PeerConnectionInterface::IceConnectionState iceConnectionState_ =
        webrtc::PeerConnectionInterface::kIceConnectionNew;
    std::unordered_map<std::string, std::unique_ptr<DataChannelBinding>>
        dataChannels_;
    std::unordered_map<std::string, VideoSlotBinding> videoSlots_;
    std::vector<std::string> videoSlotOrder_;
    AudioSlotBinding audioSlot_;
    webrtc::VideoSinkInterface<webrtc::VideoFrame>* remoteVideoSink_ = nullptr;
    webrtc::scoped_refptr<webrtc::VideoTrackInterface> remoteVideoTrack_;
};

}  // namespace remote
