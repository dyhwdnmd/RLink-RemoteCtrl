// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#pragma once

#include <chrono>
#include <condition_variable>
#include <memory>
#include <mutex>
#include <optional>
#include <span>
#include <thread>
#include <unordered_map>
#include <vector>

#include "api/media_stream_interface.h"
#include "api/scoped_refptr.h"
#include "api/video/video_frame.h"
#include "api/video/video_sink_interface.h"
#include "DirectSessionRuntimeState.h"
#include "src/core/ISessionEngine.h"
#include "src/core/SessionController.h"
#include "src/signaling/ISignalingClient.h"
#include "src/platform/win/WindowsDesktopCaptureSource.h"
#include "src/platform/win/WindowsCameraCaptureSource.h"
#include "src/platform/win/WindowsCursorMonitor.h"
#include "src/platform/win/MfH264EncoderCapabilityProbe.h"
#include "src/webrtc/VideoDecoderRuntimeStatus.h"
#include "src/webrtc/VideoEncoderRuntimeStatus.h"

namespace remote {

class WebRtcRuntime;
class LibWebRtcSession;
struct RoomMemberActionEnvelope;

namespace testing {
class InProcessSessionEngineTestAccess;
}

namespace app {

struct InProcessSessionEngineOptions {
    bool includeLoopbackAdapter = false;
    bool enableRealDesktopCapture = true;
    bool enableRealCameraCapture = true;
    DesktopCaptureImplementation desktopCaptureImplementation =
        DesktopCaptureImplementation::kNativeDxgi;
    VideoEncoderPreference videoEncoderPreference =
        VideoEncoderPreference::kAutomatic;
    FfmpegX264Preset ffmpegX264Preset = FfmpegX264Preset::kMedium;
    FfmpegHardwareBackend ffmpegHardwareBackend =
        FfmpegHardwareBackend::kAutomatic;
    std::string preferredAutomaticEncoderId;
    VideoDecoderPreference videoDecoderPreference =
        VideoDecoderPreference::kAutomatic;
    std::string preferredHardwareDecoderName;
    std::string hardwareFingerprint;
    std::string operatingSystemDescription;
    std::string nativeArchitecture;
    bool remoteSession = false;
    std::vector<std::string> graphicsAdapterDescriptions;
    std::string graphicsEnumerationError;
    std::optional<MfH264EncoderCapabilityCache>
        encoderCapabilityCache;
    std::string preferredCameraDeviceId =
        kSystemDefaultMediaDeviceId;
    std::string preferredMicrophoneDeviceId =
        kSystemDefaultMediaDeviceId;
    std::string preferredSpeakerDeviceId =
        kSystemDefaultMediaDeviceId;
    int iceMinPort = kDefaultIceMinPort;
    int iceMaxPort = kDefaultIceMaxPort;
    std::chrono::milliseconds negotiationTimeout{15000};
    std::chrono::milliseconds reconnectTimeout{60000};
};

class InProcessSessionEngine final : public ISessionEngine,
                                     private ISignalingClientObserver,
                                     private ISessionSignalingSender,
                                     private ISessionControllerObserver {
public:
    InProcessSessionEngine();
    InProcessSessionEngine(std::unique_ptr<ISignalingClient> signaling,
                           SignalingClientConfig signalingConfig,
                           InProcessSessionEngineOptions options = {});
    ~InProcessSessionEngine() override;

    InProcessSessionEngine(const InProcessSessionEngine&) = delete;
    InProcessSessionEngine& operator=(const InProcessSessionEngine&) = delete;

    void SetObserver(ISessionEngineObserver* observer) override;
    SessionCommandResult Start() override;
    // Staged startup keeps the Qt-owned signaling transport on its owner
    // thread while allowing expensive WebRTC/media initialization to run on
    // a worker. Start() remains the synchronous compatibility entry point.
    SessionCommandResult BeginStart();
    SessionCommandResult InitializeRuntimeForStart();
    SessionCommandResult CompleteStart(
        const SessionCommandResult& runtimeResult);
    void Stop() override;
    // Updates only the signaling credential retained for future
    // authentication/reconnect. Active PeerConnections and DataChannels are
    // intentionally left untouched.
    SessionCommandResult UpdateSignalingAccessToken(
        std::string accessToken);
    SessionCommandResult RequestAccountDeletion();
    void SetAccountDeletionResultCallback(
        std::function<void(const SignalingAccountDeletionResult&)> callback);

    SessionEngineSnapshot Snapshot() const override;
    SessionEngineCapabilities Capabilities() const override;
    SessionDiagnosticsSnapshot Diagnostics() const override;

    SessionCommandResult ConnectDirectDevice(
        const DirectSessionConnectRequest& request) override;
    SessionCommandResult ConnectDevice(
        const std::string& deviceId,
        SessionPurpose purpose) override;
    SessionCommandResult ConnectOwnedDevice(
        const std::string& deviceId,
        SessionPurpose purpose) override;
    SessionCommandResult ConnectAssistedDevice(
        const std::string& deviceId,
        const std::string& verificationCode) override;
    SessionCommandResult RefreshOwnedDevices() override;
    SessionCommandResult AcceptIncomingSession(
        const std::string& sessionId) override;
    SessionCommandResult RejectIncomingSession(
        const std::string& sessionId) override;
    SessionCommandResult Disconnect() override;
    SessionCommandResult CreateRoom(std::uint32_t capacity) override;
    SessionCommandResult JoinRoom(const std::string& roomId) override;
    SessionCommandResult QueryRoomAvailability(
        const std::vector<std::string>& roomIds) override;
    SessionCommandResult RespondToRoomJoin(
        const std::string& requestId,
        bool accepted) override;
    SessionCommandResult SetRoomCapacity(
        std::uint32_t capacity) override;
    SessionCommandResult LeaveRoom() override;
    // Used after an unrecoverable P2P failure. Local media/session state is
    // torn down immediately; a server leave is sent now or after WSS returns.
    SessionCommandResult ExitRoomAfterRecoveryFailure();
    SessionCommandResult RefreshLocalDisplays() override;
    SessionCommandResult SelectRoomScreenShareDisplay(
        const std::string& stableDisplayKey) override;
    SessionCommandResult StartRoomScreenShare() override;
    SessionCommandResult StopRoomScreenShare() override;
    SessionCommandResult RespondToRoomScreenShareSwitch(
        const std::string& requestId,
        bool accepted) override;
    SessionCommandResult CancelRoomScreenShareSwitch() override;
    SessionCommandResult RequestRoomControl() override;
    SessionCommandResult RespondToRoomControl(
        const std::string& requestId,
        bool accepted) override;
    SessionCommandResult ReleaseRoomControl() override;
    SessionCommandResult RequestRoomMemberScreenShare(
        const std::string& peerDeviceId) override;
    SessionCommandResult RespondToRoomMemberScreenShare(
        const std::string& requesterDeviceId,
        std::uint64_t sequence,
        bool accepted) override;
    SessionCommandResult RequestRoomMemberMicrophoneMute(
        const std::string& peerDeviceId) override;
    SessionCommandResult RequestRemoteRoomScreenShareStop(
        const std::string& peerDeviceId,
        std::uint64_t screenShareEpoch) override;
    SessionCommandResult SendRoomInput(
        const RemoteInputEvent& event) override;
    SessionCommandResult SendDirectInput(
        const RemoteInputEvent& event);
    SessionCommandResult SendRemoteInput(
        const RemoteInputEvent& event);
    SessionCommandResult SetRoomScreenFrameRate(
        const std::string& pairId,
        std::uint32_t framesPerSecond) override;
    SessionCommandResult SetRoomScreenStreamPreference(
        const std::string& pairId,
        const ScreenStreamPreferenceRequest& preference) override;
    SessionCommandResult RequestRemoteSharedDisplaySwitch(
        const std::string& pairId,
        const std::string& stableDisplayKey) override;
    SessionCommandResult SendRoomFileMessage(
        const std::string& peerDeviceId,
        const FileTransferMessage& message) override;
    SessionCommandResult SendRoomClipboardMessage(
        const std::string& peerDeviceId,
        const std::string& clipboardSessionId,
        const ClipboardMessage& message) override;
    SessionCommandResult SendRemoteFileMessage(
        const std::string& peerDeviceId,
        const FileTransferMessage& message);
    SessionCommandResult SendRemoteClipboardMessage(
        const std::string& peerDeviceId,
        const std::string& clipboardSessionId,
        const ClipboardMessage& message);
    SessionCommandResult SetDirectScreenStreamPreference(
        const ScreenStreamPreferenceRequest& preference);
    SessionCommandResult RequestDirectSharedDisplaySwitch(
        const std::string& stableDisplayKey);
    SessionCommandResult SetLocalCameraEnabled(bool enabled) override;
    SessionCommandResult SetLocalMicrophoneEnabled(bool enabled) override;
    SessionCommandResult SetRemoteAudioPlaybackMuted(bool muted);
    SessionCommandResult SetRoomAudioPlaybackMuted(bool muted) override;
    SessionCommandResult RefreshLocalMediaDevices() override;
    SessionCommandResult SelectLocalCameraDevice(
        const std::string& deviceId) override;
    SessionCommandResult SelectLocalMicrophoneDevice(
        const std::string& deviceId) override;
    SessionCommandResult SelectLocalSpeakerDevice(
        const std::string& deviceId) override;

    // Media adapters use these concrete-engine hooks. UI code continues to
    // depend only on ISessionEngine; desktop/camera capture sources and Qt
    // render sinks remain outside the control-plane interface.
    SessionCommandResult SetRoomVideoSource(
        const std::string& slot,
        webrtc::scoped_refptr<webrtc::VideoTrackSourceInterface> source,
        const std::string& trackId,
        std::optional<std::string> expectedRoomId = std::nullopt,
        std::optional<std::uint64_t> expectedCameraGeneration =
            std::nullopt);
    SessionCommandResult ClearRoomVideoSource(
        const std::string& slot);
    SessionCommandResult SetRoomVideoSlotSendingActive(
        const std::string& slot,
        bool active);
    SessionCommandResult SetRoomRemoteVideoSink(
        const std::string& pairId,
        const std::string& slot,
        webrtc::VideoSinkInterface<webrtc::VideoFrame>* sink);
    SessionCommandResult SetDirectRemoteVideoSink(
        webrtc::VideoSinkInterface<webrtc::VideoFrame>* sink);
    SessionCommandResult NotifyRoomScreenFirstFramePresented(
        const std::string& pairId,
        std::uint64_t screenShareGeneration,
        std::uint32_t startupElapsedMs);
    void SetLocalCameraPreviewSink(
        webrtc::VideoSinkInterface<webrtc::VideoFrame>* sink);
    void SetRemoteInputSink(IRemoteInputSink* sink);
    void SetRemoteFileTransferSink(IFileTransferSink* sink);
    void SetRemoteClipboardSink(IClipboardSink* sink);
    using RemoteCursorCallback = std::function<void(
        const std::string& pairId,
        const RemoteCursorEnvelope& envelope)>;
    void SetRemoteCursorCallback(RemoteCursorCallback callback);
    void SetPreferredHardwareDecoderName(std::string name);
    SessionCommandResult ApplyVideoPipelinePreferences(
        DesktopCaptureImplementation desktopCaptureImplementation,
        VideoEncoderPreference videoEncoderPreference,
        FfmpegX264Preset quality,
        FfmpegHardwareBackend ffmpegHardwareBackend,
        std::string preferredAutomaticEncoderId,
        VideoDecoderPreference videoDecoderPreference);

private:
    friend class remote::testing::InProcessSessionEngineTestAccess;

    void OnSignalingStateChanged(SignalingConnectionState state) override;
    void OnDeviceRegistered(const std::string& deviceId) override;
    void OnIncomingSessionRequest(
        const IncomingSessionRequest& request) override;
    void OnSessionResponse(
        const SignalingSessionResponse& response) override;
    void OnSessionPending(
        const SignalingSessionPending& pending) override;
    void OnSessionReady(const SignalingSessionReady& ready) override;
    void OnSessionSuspended(
        const SignalingSessionSuspended& suspended) override;
    void OnSessionResumed(
        const SignalingSessionResumed& resumed) override;
    void OnSessionEnded(const SignalingSessionEnded& ended) override;
    void OnRemoteDescription(
        const SignalingSessionDescription& description) override;
    void OnRemoteIceCandidate(
        const SignalingIceCandidate& candidate) override;
    void OnIceRestartRequested(
        const SignalingIceRestartRequest& request) override;
    void OnIceRestartCancelled(
        const SignalingIceRestartCancel& cancel) override;
    void OnHeartbeatAcknowledged(std::uint32_t roundTripMs) override;
    void OnSignalingError(const std::string& code,
                          const std::string& message) override;
    void OnAccountDeletionResult(
        const SignalingAccountDeletionResult& result) override;
    void OnOwnedDevicesChanged(
        const SignalingOwnedDevicesSnapshot& snapshot) override;
    void OnRoomReady(const SignalingRoomReady& ready) override;
    void OnRoomState(const RoomSnapshot& room) override;
    void OnRoomJoinPending(
        const SignalingRoomJoinPending& pending) override;
    void OnRoomJoinRequested(const RoomJoinRequest& request) override;
    void OnRoomJoinResult(
        const SignalingRoomJoinResult& result) override;
    void OnRoomAvailabilityResult(
        const SignalingRoomAvailabilityResult& result) override;
    void OnRoomClosed(const SignalingRoomClosed& closed) override;
    void OnRoomPairReady(const SignalingRoomPairReady& ready) override;
    void OnRoomPairClosed(const SignalingRoomPairClosed& closed) override;
    void OnRoomScreenShareGranted(
        const SignalingRoomScreenShareGranted& granted) override;
    void OnRoomScreenShareSwitchPending(
        const SignalingRoomScreenShareSwitchPending& pending) override;
    void OnRoomScreenShareSwitchRequested(
        const RoomScreenShareSwitchRequest& request) override;
    void OnRoomScreenShareSwitchResult(
        const SignalingRoomScreenShareSwitchResult& result) override;
    void OnRoomControlRequested(
        const RoomControlRequest& request) override;
    void OnRoomControlResult(
        const SignalingRoomControlResult& result) override;
    void OnRoomControlGranted(
        const SignalingRoomControlGranted& granted) override;
    void OnRoomControlRevoked(
        const SignalingRoomControlRevoked& revoked) override;

    bool SendDescription(
        const SessionDescription& description) override;
    bool SendIceCandidate(const IceCandidate& candidate) override;
    bool RequestIceRestart(std::uint64_t observedGeneration,
                           std::uint64_t requestSequence) override;
    bool CancelIceRestart(std::uint64_t observedGeneration,
                          std::uint64_t requestSequence) override;
    void OnControllerSnapshot(
        const SessionControllerSnapshot& snapshot) override;
    void OnDataChannelStateChanged(
        const DataChannelInfo& channel) override;
    void OnDataMessage(const std::string& label,
                       std::span<const std::uint8_t> payload,
                       bool binary) override;
    void OnRemoteTrackAdded(const RemoteTrackInfo& track) override;

    SessionCommandResult RequireReady(const char* operation) const;
    bool SignalingIsOnline() const;
    void DisposeClosedSession();
    void ResetSessionStateLocked();
    void ResetRoomStateLocked();
    void StopLocalDesktopCapture();
    void StopDirectDesktopCapture();
    void StartRemoteCursorPublishing(
        const DisplayDescriptor& display,
        std::uint64_t layoutVersion);
    void StopRemoteCursorPublishing();
    void OnLocalCursorObservation(WindowsCursorObservation observation);
    void RepublishRemoteCursor();
    bool DispatchRemoteCursorData(
        const std::string& pairId,
        const std::string& label,
        std::span<const std::uint8_t> payload);
    void BroadcastDirectSharedDisplayLayout();
    void RequestDirectSharedDisplayLayout();
    SessionCommandResult SendDirectFileMessage(
        const std::string& peerDeviceId,
        const FileTransferMessage& message);
    SessionCommandResult SendDirectClipboardMessage(
        const std::string& peerDeviceId,
        const std::string& clipboardSessionId,
        const ClipboardMessage& message);
    bool DispatchDirectAuxiliaryData(
        const std::string& label,
        std::span<const std::uint8_t> payload);
    bool DispatchDirectScreenData(
        const std::string& label,
        std::span<const std::uint8_t> payload);
    SessionCommandResult SwitchLocalDirectDisplay(
        const std::string& stableDisplayKey);
    void BroadcastDirectSharedDisplayCatalog();
    void BroadcastSharedDisplayLayout();
    bool SendRoomPairDescription(
        const std::string& pairId,
        const SessionDescription& description);
    bool SendRoomPairIceCandidate(
        const std::string& pairId,
        const IceCandidate& candidate);
    bool SendRoomPairIceRestartRequest(
        const std::string& pairId,
        std::uint64_t observedGeneration,
        std::uint64_t requestSequence);
    bool SendRoomPairIceRestartCancel(
        const std::string& pairId,
        std::uint64_t observedGeneration,
        std::uint64_t requestSequence);
    void OnRoomPairControllerSnapshot(
        const std::string& pairId,
        const SessionControllerSnapshot& snapshot);
    void OnRoomPairDataChannelStateChanged(
        const std::string& pairId,
        const DataChannelInfo& channel);
    void OnRoomPairDataMessage(
        const std::string& pairId,
        const std::string& label,
        std::span<const std::uint8_t> payload,
        bool binary);
    void OnRoomPairRemoteTrackAdded(
        const std::string& pairId,
        const RemoteTrackInfo& track);
    std::optional<OperationError> PrepareRoomPairAnswer(
        const std::string& pairId);
    std::optional<OperationError> PrepareRoomPairMedia(
        const std::string& pairId,
        bool bindNegotiatedSlots,
        bool preparationAlreadyClaimed = false);
    std::optional<OperationError> PrepareDirectMedia(
        bool bindNegotiatedSlots);
    void StopDirectMicrophoneCapture();
    void PublishSnapshot();
    void StartStatsPolling();
    void StopStatsPolling();

    class RoomPairBridge;
    struct RoomPairRuntime;
    void RetireRoomPair(std::shared_ptr<RoomPairRuntime> pair);
    SessionCommandResult ApplyLocalScreenFrameRate(
        std::uint32_t framesPerSecond);
    SessionCommandResult SwitchLocalSharedDisplay(
        const std::string& stableDisplayKey);
    void BroadcastSharedDisplayCatalog();
    void StartClipboardWarmup(
        const std::shared_ptr<RoomPairRuntime>& pair);
    SessionCommandResult SendRoomMemberAction(
        const std::string& peerDeviceId,
        RoomMemberAction action,
        std::uint64_t screenShareEpoch = 0);
    SendResult SendRoomMemberActionResponse(
        const std::shared_ptr<RoomPairRuntime>& pair,
        const RoomMemberActionEnvelope& request,
        bool accepted,
        const std::string& error);

    std::unique_ptr<WebRtcRuntime> runtime_;
    std::unique_ptr<ISignalingClient> signaling_;
    SignalingClientConfig signalingConfig_;
    InProcessSessionEngineOptions options_;
    std::unique_ptr<LibWebRtcSession> webRtcSession_;
    std::unique_ptr<SessionControllerBase> sessionController_;
    std::unordered_map<std::string, std::shared_ptr<RoomPairRuntime>>
        roomPairs_;
    std::vector<std::jthread> retiredRoomPairThreads_;
    // Device drivers may block while stopping a capture module. Keep those
    // joins outside the Qt UI path, but retain ownership until engine stop.
    std::vector<std::jthread> retiredDesktopStopThreads_;
    std::vector<std::jthread> retiredCameraStopThreads_;
    std::vector<std::jthread> mediaDeviceOperationThreads_;
    std::vector<std::jthread> clipboardWarmupThreads_;
    std::jthread statsPollingThread_;
    std::condition_variable_any statsPollingCondition_;
    std::mutex statsPollingWaitMutex_;
    std::unordered_map<
        std::string,
        webrtc::scoped_refptr<webrtc::MediaStreamTrackInterface>>
        localRoomVideoTracks_;
    std::unordered_map<
        std::string,
        webrtc::scoped_refptr<webrtc::MediaStreamTrackInterface>>
        idleRoomVideoTracks_;
    webrtc::scoped_refptr<WindowsDesktopCaptureSource>
        localDesktopCaptureSource_;
    webrtc::scoped_refptr<WindowsCameraCaptureSource>
        localCameraCaptureSource_;
    webrtc::VideoSinkInterface<webrtc::VideoFrame>*
        localCameraPreviewSink_ = nullptr;
    webrtc::scoped_refptr<webrtc::VideoTrackInterface>
        localCameraPreviewTrack_;
    webrtc::scoped_refptr<webrtc::AudioSourceInterface>
        localMicrophoneAudioSource_;
    webrtc::scoped_refptr<webrtc::AudioTrackInterface>
        localMicrophoneAudioTrack_;
    std::optional<SessionDescription> pendingRemoteDescription_;
    std::vector<IceCandidate> pendingRemoteCandidates_;
    mutable std::mutex mutex_;
    ISessionEngineObserver* observer_ = nullptr;
    std::function<void(const SignalingAccountDeletionResult&)>
        accountDeletionResultCallback_;
    SessionEngineSnapshot snapshot_;
    SessionEngineCapabilities capabilities_;
    bool localIsOfferer_ = false;
    bool offerNegotiationStarted_ = false;
    DirectSessionRuntimeState directSession_;
    bool roomAudioDevicesApplied_ = false;
    bool sessionCloseRequested_ = false;
    bool sessionEndSignalSent_ = false;
    bool cancelWhenSessionIdKnown_ = false;
    bool serverSessionActive_ = false;
    bool signalingRecoveryPending_ = false;
    bool peerSignalingSuspended_ = false;
    std::string sessionRecoveryToken_;
    std::string roomRecoveryToken_;
    std::string roomScreenShareGrantId_;
    std::string roomControlGrantId_;
    std::string roomControlGrantScreenSharerDeviceId_;
    std::string roomControlGrantControllerDeviceId_;
    IRemoteInputSink* remoteInputSink_ = nullptr;
    IFileTransferSink* remoteFileTransferSink_ = nullptr;
    IClipboardSink* remoteClipboardSink_ = nullptr;
    RemoteCursorCallback remoteCursorCallback_;
    WindowsCursorMonitor cursorMonitor_;
    std::optional<RemoteCursorPosition> latestLocalCursorPosition_;
    std::optional<RemoteCursorShape> latestLocalCursorShape_;
    std::uint64_t nextCursorSequence_ = 0;
    std::uint64_t cursorPositionsPublished_ = 0;
    std::uint64_t cursorShapesPublished_ = 0;
    std::uint64_t cursorPositionsReceived_ = 0;
    std::uint64_t cursorShapesReceived_ = 0;
    // One sequence spans both input DataChannels so late reliable button
    // transitions cannot overwrite newer fast pointer state.
    std::uint64_t nextRoomInputSequence_ = 0;
    std::uint64_t nextRoomScreenControlSequence_ = 0;
    std::uint32_t localScreenFrameRate_ = kDefaultScreenFrameRate;
    std::uint64_t localScreenShareGeneration_ = 0;
    std::uint64_t localCameraTrackGeneration_ = 0;
    std::unordered_map<std::string, ScreenStreamPreferenceRequest>
        roomScreenStreamPreferences_;
    bool roomRecoveryPending_ = false;
    bool roomLeaveRequested_ = false;
    std::string deferredRoomLeaveId_;
};

}  // namespace app
}  // namespace remote
