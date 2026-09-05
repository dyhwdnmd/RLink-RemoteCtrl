// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#include "LocalPeerConnectionH264SelfTest.h"

#include <algorithm>
#include <chrono>
#include <cctype>
#include <condition_variable>
#include <cstdint>
#include <cstring>
#include <memory>
#include <mutex>
#include <optional>
#include <sstream>
#include <string>
#include <string_view>
#include <thread>
#include <unordered_map>
#include <utility>
#include <vector>

#include "api/make_ref_counted.h"
#include "api/media_stream_interface.h"
#include "api/rtp_parameters.h"
#include "api/rtp_transceiver_interface.h"
#include "api/video/adapted_video_track_source.h"
#include "api/video/i420_buffer.h"
#include "api/video/video_frame.h"
#include "api/video/video_sink_interface.h"
#include "rtc_base/time_utils.h"
#include "src/platform/win/D3D11NativeFrameBuffer.h"
#include "src/core/SessionController.h"
#include "src/protocol/DataChannelCatalog.h"
#include "src/webrtc/LibWebRtcSession.h"

namespace remote {
namespace {

using namespace std::chrono_literals;

constexpr int kFrameWidth = 320;
constexpr int kFrameHeight = 180;
constexpr int kFrameRate = 30;
constexpr int kMaximumFrames = 180;
constexpr auto kOperationTimeout = 10s;

bool EqualsIgnoreCase(const std::string& left, const char* right)
{
    const std::string rightText(right);
    if (left.size() != rightText.size()) {
        return false;
    }

    return std::equal(left.begin(), left.end(), rightText.begin(),
                      [](char a, char b) {
                          return std::tolower(
                                     static_cast<unsigned char>(a)) ==
                                 std::tolower(
                                     static_cast<unsigned char>(b));
                      });
}

class SyntheticVideoSource : public webrtc::AdaptedVideoTrackSource {
public:
    SourceState state() const override { return kLive; }
    bool remote() const override { return false; }
    bool is_screencast() const override { return true; }
    std::optional<bool> needs_denoising() const override { return false; }

    void PushFrame(const webrtc::VideoFrame& frame) { OnFrame(frame); }
};

class NativeFrameSink final
    : public webrtc::VideoSinkInterface<webrtc::VideoFrame> {
public:
    void OnFrame(const webrtc::VideoFrame& frame) override
    {
        const auto buffer = frame.video_frame_buffer();
        const bool native =
            D3D11NativeFrameBuffer::From(buffer.get()) != nullptr;

        std::lock_guard lock(mutex_);
        ++frameCount_;
        if (native) {
            ++nativeFrameCount_;
        }
        width_ = frame.width();
        height_ = frame.height();
        condition_.notify_all();
    }

    bool WaitForNativeFrame(std::chrono::milliseconds timeout)
    {
        std::unique_lock lock(mutex_);
        return condition_.wait_for(lock, timeout,
                                   [this] { return nativeFrameCount_ > 0; });
    }

    int FrameCount() const
    {
        std::lock_guard lock(mutex_);
        return frameCount_;
    }

    int NativeFrameCount() const
    {
        std::lock_guard lock(mutex_);
        return nativeFrameCount_;
    }

    int Width() const
    {
        std::lock_guard lock(mutex_);
        return width_;
    }

    int Height() const
    {
        std::lock_guard lock(mutex_);
        return height_;
    }

private:
    mutable std::mutex mutex_;
    std::condition_variable condition_;
    int frameCount_ = 0;
    int nativeFrameCount_ = 0;
    int width_ = 0;
    int height_ = 0;
};

class ControllerSelfTestObserver final : public ISessionControllerObserver {
public:
    void OnControllerSnapshot(
        const SessionControllerSnapshot& snapshot) override
    {
        std::lock_guard lock(mutex_);
        snapshot_ = snapshot;
        condition_.notify_all();
    }

    void OnDataChannelStateChanged(
        const DataChannelInfo& channel) override
    {
        std::lock_guard lock(mutex_);
        channels_[channel.label] = channel;
        condition_.notify_all();
    }

    void OnDataMessage(const std::string& label,
                       std::span<const std::uint8_t> payload,
                       bool) override
    {
        std::string message;
        if (!payload.empty()) {
            message.assign(
                reinterpret_cast<const char*>(payload.data()),
                payload.size());
        }
        std::lock_guard lock(mutex_);
        messages_[label].push_back(std::move(message));
        condition_.notify_all();
    }

    void OnRemoteTrackAdded(const RemoteTrackInfo&) override
    {
        std::lock_guard lock(mutex_);
        ++remoteTrackCount_;
        condition_.notify_all();
    }

    bool WaitForState(SessionControllerState expected,
                      std::chrono::milliseconds timeout)
    {
        std::unique_lock lock(mutex_);
        return condition_.wait_for(lock, timeout, [this, expected] {
                   return snapshot_.state == expected ||
                          (expected != SessionControllerState::kClosed &&
                           !snapshot_.errorCode.empty());
               }) &&
               snapshot_.state == expected;
    }

    bool WaitForChannelOpen(const std::string& label,
                            std::chrono::milliseconds timeout)
    {
        std::unique_lock lock(mutex_);
        return condition_.wait_for(lock, timeout, [this, &label] {
            const auto found = channels_.find(label);
            return found != channels_.end() &&
                   found->second.state == DataChannelState::kOpen;
        });
    }

    std::optional<DataChannelInfo> ChannelInfo(
        const std::string& label) const
    {
        std::lock_guard lock(mutex_);
        const auto found = channels_.find(label);
        if (found == channels_.end()) {
            return std::nullopt;
        }
        return found->second;
    }

    bool WaitForMessage(const std::string& label,
                        const std::string& expected,
                        std::chrono::milliseconds timeout)
    {
        std::unique_lock lock(mutex_);
        return condition_.wait_for(lock, timeout,
                                   [this, &label, &expected] {
            const auto found = messages_.find(label);
            return found != messages_.end() &&
                   std::find(found->second.begin(), found->second.end(),
                             expected) != found->second.end();
        });
    }

    std::string ErrorText() const
    {
        std::lock_guard lock(mutex_);
        if (snapshot_.errorCode.empty()) {
            return {};
        }
        return snapshot_.errorCode + ": " + snapshot_.errorMessage;
    }

    bool WaitForErrorCode(const std::string& expected,
                          std::chrono::milliseconds timeout)
    {
        std::unique_lock lock(mutex_);
        return condition_.wait_for(lock, timeout, [this, &expected] {
            return snapshot_.errorCode == expected;
        });
    }

private:
    mutable std::mutex mutex_;
    std::condition_variable condition_;
    SessionControllerSnapshot snapshot_;
    std::unordered_map<std::string, DataChannelInfo> channels_;
    std::unordered_map<std::string, std::vector<std::string>> messages_;
    int remoteTrackCount_ = 0;
};

class LocalSignalingEndpoint final : public ISessionSignalingSender {
public:
    void SetPeer(SessionControllerBase* peer)
    {
        std::lock_guard lock(mutex_);
        peer_ = peer;
    }

    bool SendDescription(const SessionDescription& description) override
    {
        SessionControllerBase* peer = nullptr;
        bool holdForCandidate = false;
        {
            std::lock_guard lock(mutex_);
            descriptions_.push_back(description);
            peer = peer_;
            holdForCandidate = !candidateBeforeDescriptionExercised_;
            if (holdForCandidate) {
                pendingDescription_ = description;
            }
        }
        if (!peer) {
            return false;
        }
        if (holdForCandidate) {
            return true;
        }
        peer->HandleRemoteDescription(description);
        return true;
    }

    bool SendIceCandidate(const IceCandidate& candidate) override
    {
        SessionControllerBase* peer = nullptr;
        std::optional<SessionDescription> pendingDescription;
        {
            std::lock_guard lock(mutex_);
            ++candidateCount_;
            peer = peer_;
            if (pendingDescription_) {
                pendingDescription = std::move(pendingDescription_);
                pendingDescription_.reset();
                candidateBeforeDescriptionExercised_ = true;
            }
        }
        if (!peer) {
            return false;
        }
        peer->HandleRemoteIceCandidate(candidate);
        if (pendingDescription) {
            peer->HandleRemoteDescription(*pendingDescription);
        }
        return true;
    }

    std::optional<SessionDescription> Description(
        SessionDescriptionType type) const
    {
        std::lock_guard lock(mutex_);
        const auto found = std::find_if(
            descriptions_.begin(), descriptions_.end(),
            [type](const SessionDescription& description) {
                return description.type == type;
            });
        if (found == descriptions_.end()) {
            return std::nullopt;
        }
        return *found;
    }

    int CandidateCount() const
    {
        std::lock_guard lock(mutex_);
        return candidateCount_;
    }

    bool CandidateBeforeDescriptionExercised() const
    {
        std::lock_guard lock(mutex_);
        return candidateBeforeDescriptionExercised_;
    }

private:
    mutable std::mutex mutex_;
    SessionControllerBase* peer_ = nullptr;
    std::vector<SessionDescription> descriptions_;
    std::optional<SessionDescription> pendingDescription_;
    int candidateCount_ = 0;
    bool candidateBeforeDescriptionExercised_ = false;
};

class DiscardSignalingSender final : public ISessionSignalingSender {
public:
    bool SendDescription(const SessionDescription&) override { return true; }
    bool SendIceCandidate(const IceCandidate&) override { return true; }
};

std::vector<webrtc::RtpCodecCapability> H264CodecPreferences(
    webrtc::PeerConnectionFactoryInterface* factory)
{
    auto codecs = factory
                      ->GetRtpReceiverCapabilities(webrtc::MediaType::VIDEO)
                      .codecs;
    std::erase_if(codecs, [](const webrtc::RtpCodecCapability& codec) {
        return !codec.IsResiliencyCodec() &&
               !EqualsIgnoreCase(codec.name, "H264");
    });
    return codecs;
}

webrtc::VideoFrame CreateSyntheticFrame(int frameIndex)
{
    auto buffer = webrtc::I420Buffer::Create(kFrameWidth, kFrameHeight);
    const uint8_t luma = static_cast<uint8_t>(32 + (frameIndex * 3) % 180);
    for (int row = 0; row < kFrameHeight; ++row) {
        std::memset(buffer->MutableDataY() + row * buffer->StrideY(),
                    static_cast<uint8_t>(luma + row % 16),
                    kFrameWidth);
    }

    const int chromaWidth = (kFrameWidth + 1) / 2;
    const int chromaHeight = (kFrameHeight + 1) / 2;
    for (int row = 0; row < chromaHeight; ++row) {
        std::memset(buffer->MutableDataU() + row * buffer->StrideU(),
                    static_cast<uint8_t>(90 + frameIndex % 60),
                    chromaWidth);
        std::memset(buffer->MutableDataV() + row * buffer->StrideV(),
                    static_cast<uint8_t>(160 - frameIndex % 60),
                    chromaWidth);
    }

    return webrtc::VideoFrame::Builder()
        .set_video_frame_buffer(buffer)
        .set_timestamp_us(webrtc::TimeMicros())
        .set_rotation(webrtc::kVideoRotation_0)
        .build();
}

void AppendResult(std::ostringstream& report,
                  std::string_view label,
                  bool value)
{
    report << label << ": " << (value ? "YES" : "NO") << '\n';
}

}  // namespace

LocalPeerConnectionH264SelfTestResult RunLocalPeerConnectionH264SelfTest(
    webrtc::scoped_refptr<webrtc::PeerConnectionFactoryInterface> factory,
    std::function<bool()> reconfigureEncoder)
{
    LocalPeerConnectionH264SelfTestResult result;
    std::ostringstream report;

    if (!factory) {
        result.report = "PeerConnection factory: NO\nError: factory is null.\n";
        return result;
    }

    NativeFrameSink receiverSink;
    ControllerSelfTestObserver senderObserver;
    ControllerSelfTestObserver receiverObserver;
    LocalSignalingEndpoint senderSignaling;
    LocalSignalingEndpoint receiverSignaling;
    LibWebRtcSession sender(factory);
    LibWebRtcSession receiver(factory);
    ControllerSessionController senderController(sender, senderSignaling);
    AgentSessionController receiverController(receiver, receiverSignaling);
    senderSignaling.SetPeer(&receiverController);
    receiverSignaling.SetPeer(&senderController);
    senderController.SetObserver(&senderObserver);
    receiverController.SetObserver(&receiverObserver);
    receiver.SetRemoteVideoSink(&receiverSink);

    const auto operationTimeout =
        std::chrono::duration_cast<std::chrono::milliseconds>(
            kOperationTimeout);
    SessionControllerConfig controllerConfig;
    // libwebrtc ignores loopback adapters by default. This test deliberately
    // hosts both formal sessions in one process.
    controllerConfig.webRtc.includeLoopbackAdapter = true;
    // Mirror native DXGI: it needs the common first-share bitrate bootstrap,
    // but must not enable libwebrtc's media-ready retry or progressive cap.
    controllerConfig.webRtc.fastDesktopBweStartup = false;
    controllerConfig.negotiationTimeout = operationTimeout;
    senderController.Start(controllerConfig);
    receiverController.Start(controllerConfig);
    const bool senderStarted = senderObserver.WaitForState(
        SessionControllerState::kReady, operationTimeout);
    const bool receiverStarted = receiverObserver.WaitForState(
        SessionControllerState::kReady, operationTimeout);
    const bool peersCreated = senderStarted && receiverStarted;
    AppendResult(report, "PeerConnections created", peersCreated);
    AppendResult(report, "Formal LibWebRtcSession path used", true);
    AppendResult(report, "Role-specific session controllers ready",
                 peersCreated);
    if (!peersCreated) {
        if (!senderStarted) {
            report << "Sender controller start error: "
                   << senderObserver.ErrorText() << '\n';
        }
        if (!receiverStarted) {
            report << "Receiver controller start error: "
                   << receiverObserver.ErrorText() << '\n';
        }
        result.report = report.str();
        return result;
    }

    const auto dataChannelSpecs = DefaultRemoteControlDataChannels();

    const auto prepareVideoResult =
        sender.PrepareVideoTransceiverSlot(kScreenMainVideoSlot);
    const bool transceiverCreated = prepareVideoResult.ok();
    AppendResult(report, "Video transceiver created", transceiverCreated);
    if (!transceiverCreated) {
        report << "Transceiver error: "
               << prepareVideoResult.message() << '\n';
        result.report = report.str();
        return result;
    }

    auto codecPreferences = H264CodecPreferences(factory.get());
    const bool hasH264 = std::any_of(
        codecPreferences.begin(), codecPreferences.end(),
        [](const auto& codec) { return EqualsIgnoreCase(codec.name, "H264"); });
    const bool h264Forced = hasH264 && prepareVideoResult.ok();
    AppendResult(report, "H264 codec preference applied", h264Forced);
    report << "H264/resiliency codec preferences: "
           << codecPreferences.size() << '\n';
    if (!h264Forced) {
        report << "Codec preference error: H264 is unavailable.\n";
        result.report = report.str();
        return result;
    }

    const auto inactiveResult = sender.SetVideoSlotSendingActive(
        kScreenMainVideoSlot, false);
    if (!inactiveResult.ok()) {
        report << "Screen sender deactivate error: "
               << inactiveResult.message() << '\n';
        result.report = report.str();
        return result;
    }

    senderController.Connect(dataChannelSpecs);
    const bool senderConnected = senderObserver.WaitForState(
        SessionControllerState::kConnected, operationTimeout);
    const bool receiverConnected = receiverObserver.WaitForState(
        SessionControllerState::kConnected, operationTimeout);
    AppendResult(report, "Controller-driven negotiation completed",
                 senderConnected && receiverConnected);
    if (!senderConnected || !receiverConnected) {
        report << "Sender controller error: " << senderObserver.ErrorText()
               << '\n';
        report << "Receiver controller error: " << receiverObserver.ErrorText()
               << '\n';
    }

    auto source = webrtc::make_ref_counted<SyntheticVideoSource>();
    auto videoTrack = factory->CreateVideoTrack(source, "synthetic-screen");
    const auto policyResult = sender.SetVideoSlotEncodingPolicy(
        kScreenMainVideoSlot, kFrameRate, kFrameWidth, kFrameHeight);
    const auto trackResult = sender.SetVideoSlotTrack(
        kScreenMainVideoSlot, videoTrack);
    const auto activeResult = sender.SetVideoSlotSendingActive(
        kScreenMainVideoSlot, true);
    const bool screenSlotActivated = policyResult.ok() &&
        trackResult.ok() && activeResult.ok();
    AppendResult(report, "Screen slot configured and activated",
                 screenSlotActivated);
    if (!screenSlotActivated) {
        report << "Screen policy error: " << policyResult.message() << '\n'
               << "Screen track error: " << trackResult.message() << '\n'
               << "Screen activate error: " << activeResult.message()
               << '\n';
        result.report = report.str();
        return result;
    }

    const auto offer =
        senderSignaling.Description(SessionDescriptionType::kOffer);
    const auto answer =
        receiverSignaling.Description(SessionDescriptionType::kAnswer);
    const bool offerContainsH264 = offer &&
                                   offer->sdp.find("H264/90000") !=
                                       std::string::npos;
    const bool answerContainsH264 = answer &&
                                    answer->sdp.find("H264/90000") !=
                                        std::string::npos;
    constexpr std::string_view kPlayoutDelayExtension =
        "http://www.webrtc.org/experiments/rtp-hdrext/playout-delay";
    const bool playoutDelayNegotiated =
        offer && answer &&
        offer->sdp.find(kPlayoutDelayExtension) != std::string::npos &&
        answer->sdp.find(kPlayoutDelayExtension) != std::string::npos;
    AppendResult(report, "Offer contains H264", offerContainsH264);
    AppendResult(report, "Answer contains H264", answerContainsH264);
    AppendResult(report, "Low-latency playout extension negotiated",
                 playoutDelayNegotiated);
    report << "Sender ICE candidates signaled: "
           << senderSignaling.CandidateCount() << '\n';
    report << "Receiver ICE candidates signaled: "
           << receiverSignaling.CandidateCount() << '\n';
    const bool candidateCacheExercised =
        senderSignaling.CandidateBeforeDescriptionExercised() &&
        receiverSignaling.CandidateBeforeDescriptionExercised();
    AppendResult(report, "ICE-before-SDP candidate cache exercised",
                 candidateCacheExercised);
    AppendResult(report, "Sender connected", senderConnected);
    AppendResult(report, "Receiver connected", receiverConnected);

    const auto asBytes = [](const std::string& text) {
        return std::span<const std::uint8_t>(
            reinterpret_cast<const std::uint8_t*>(text.data()), text.size());
    };

    const auto parametersMatch = [](const DataChannelInfo& channel,
                                    const DataChannelSpec& spec) {
        return channel.label == spec.label &&
               channel.ordered == spec.ordered &&
               channel.maxRetransmits == spec.maxRetransmits &&
               channel.maxPacketLifeTimeMs == spec.maxPacketLifeTimeMs &&
               channel.protocol == spec.protocol;
    };

    bool allChannelsOpened = true;
    bool allChannelParametersMatch = true;
    for (const auto& spec : dataChannelSpecs) {
        const bool senderOpen = senderObserver.WaitForChannelOpen(
            spec.label, operationTimeout);
        const bool receiverOpen = receiverObserver.WaitForChannelOpen(
            spec.label, operationTimeout);
        const auto senderInfo = senderObserver.ChannelInfo(spec.label);
        const auto receiverInfo = receiverObserver.ChannelInfo(spec.label);
        const bool opened = senderOpen && receiverOpen;
        const bool parametersValid =
            senderInfo && receiverInfo &&
            parametersMatch(*senderInfo, spec) &&
            parametersMatch(*receiverInfo, spec);
        allChannelsOpened = allChannelsOpened && opened;
        allChannelParametersMatch =
            allChannelParametersMatch && parametersValid;
        AppendResult(report, "Channel " + spec.label + " open", opened);
        AppendResult(report,
                     "Channel " + spec.label + " parameters match",
                     parametersValid);
    }

    bool allChannelMessagesPassed = true;
    for (const auto& spec : dataChannelSpecs) {
        const std::string request = spec.label + "-probe";
        const std::string response = spec.label + "-ack";
        const bool requestSent =
            senderController.SendData(spec.label, asBytes(request), false) ==
            SendResult::kSent;
        const bool requestReceived = requestSent &&
            receiverObserver.WaitForMessage(spec.label, request,
                                            operationTimeout);
        const bool responseSent = requestReceived &&
            receiverController.SendData(spec.label, asBytes(response), false) ==
                SendResult::kSent;
        const bool responseReceived = responseSent &&
            senderObserver.WaitForMessage(spec.label, response,
                                          operationTimeout);
        const bool messagePassed = requestSent && requestReceived &&
                                   responseSent && responseReceived;
        allChannelMessagesPassed =
            allChannelMessagesPassed && messagePassed;
        AppendResult(report, "Channel " + spec.label + " bidirectional message",
                     messagePassed);
    }
    AppendResult(report, "All six DataChannels open", allChannelsOpened);
    AppendResult(report, "All six DataChannel parameters verified",
                 allChannelParametersMatch);
    AppendResult(report, "All six DataChannels exchanged messages",
                 allChannelMessagesPassed);

    int framesPushed = 0;
    bool encoderReconfigured = !reconfigureEncoder;
    std::uint64_t nativeFramesBeforeReconfigure = 0;
    std::uint64_t framesBeforeReconfigure = 0;
    for (int index = 0; index < kMaximumFrames; ++index) {
        source->PushFrame(CreateSyntheticFrame(index));
        ++framesPushed;
        if (reconfigureEncoder && !encoderReconfigured &&
            receiverSink.NativeFrameCount() >= 3) {
            nativeFramesBeforeReconfigure =
                receiverSink.NativeFrameCount();
            framesBeforeReconfigure = receiverSink.FrameCount();
            encoderReconfigured = reconfigureEncoder();
        }
        const std::uint64_t requiredFrames = reconfigureEncoder
            ? framesBeforeReconfigure + 3
            : 3;
        if (encoderReconfigured &&
            receiverSink.FrameCount() >= requiredFrames) {
            break;
        }
        std::this_thread::sleep_for(1000ms / kFrameRate);
    }
    receiverSink.WaitForNativeFrame(2s);

    const bool frameReceived = receiverSink.FrameCount() > 0;
    const bool nativeFrameReceived = receiverSink.NativeFrameCount() > 0;
    const bool decodedSizeMatches = receiverSink.Width() == kFrameWidth &&
                                    receiverSink.Height() == kFrameHeight;
    report << "Synthetic frames pushed: " << framesPushed << '\n';
    report << "Decoded frames received: " << receiverSink.FrameCount() << '\n';
    report << "D3D11 native frames received: "
           << receiverSink.NativeFrameCount() << '\n';
    report << "Decoded size: " << receiverSink.Width() << 'x'
           << receiverSink.Height() << '\n';
    AppendResult(report, "WebRTC decoded frame received", frameReceived);
    AppendResult(report, "D3D11 native decode path confirmed",
                 nativeFrameReceived);
    AppendResult(report, "Decoded visible size matches source",
                 decodedSizeMatches);
    const bool framesReceivedAfterReconfigure =
        !reconfigureEncoder ||
        (encoderReconfigured && nativeFramesBeforeReconfigure != 0 &&
         receiverSink.FrameCount() >= framesBeforeReconfigure + 3);
    AppendResult(report, "Codec policy reapplied",
                 encoderReconfigured);
    AppendResult(report, "Frames received after codec replacement",
                 framesReceivedAfterReconfigure);

    const auto commonBootstrapSucceeded = [&sender] {
        const auto snapshot = sender.StatsSnapshot();
        return std::any_of(
            snapshot.rtpStreams.begin(), snapshot.rtpStreams.end(),
            [](const RtpStreamStatsSnapshot& stream) {
                return stream.direction ==
                           RtpStreamDirection::kOutbound &&
                       stream.slot == kScreenMainVideoSlot &&
                       stream.configuredStartBitrateBps > 0 &&
                       stream.bitrateBootstrapAttempts > 0 &&
                       stream.bitrateBootstrapSuccesses > 0 &&
                       stream.mediaReadyBitrateRestarts == 0 &&
                       !stream.progressiveBitrateCeilingEnabled &&
                       stream.bitrateBootstrapError.empty();
            });
    };
    bool baseBootstrapWithoutFastStartup = false;
    for (int attempt = 0; attempt < 20; ++attempt) {
        sender.RequestStats();
        std::this_thread::sleep_for(100ms);
        if (commonBootstrapSucceeded()) {
            baseBootstrapWithoutFastStartup = true;
            break;
        }
    }
    AppendResult(
        report,
        "Base screen bitrate bootstrap without libwebrtc fast startup",
        baseBootstrapWithoutFastStartup);

    receiver.SetRemoteVideoSink(nullptr);
    (void)sender.SetVideoSlotSendingActive(kScreenMainVideoSlot, false);
    videoTrack = nullptr;
    source = nullptr;
    senderController.Close();
    senderController.Close();
    receiverController.Close();
    receiverController.Close();
    const bool senderClosed = senderObserver.WaitForState(
        SessionControllerState::kClosed, operationTimeout);
    const bool receiverClosed = receiverObserver.WaitForState(
        SessionControllerState::kClosed, operationTimeout);
    AppendResult(report, "Sender session closed", senderClosed);
    AppendResult(report, "Receiver session closed", receiverClosed);
    AppendResult(report, "Idempotent controller Close verified",
                 senderClosed && receiverClosed);

    bool negotiationTimeoutHandled = false;
    bool timeoutSessionClosed = false;
    {
        DiscardSignalingSender discardSignaling;
        ControllerSelfTestObserver timeoutObserver;
        LibWebRtcSession timeoutSession(factory);
        ControllerSessionController timeoutController(timeoutSession,
                                                      discardSignaling);
        timeoutController.SetObserver(&timeoutObserver);

        SessionControllerConfig timeoutConfig = controllerConfig;
        timeoutConfig.negotiationTimeout = 150ms;
        timeoutController.Start(timeoutConfig);
        const bool timeoutControllerReady = timeoutObserver.WaitForState(
            SessionControllerState::kReady, operationTimeout);
        if (timeoutControllerReady) {
            DataChannelSpec timeoutChannel;
            timeoutChannel.label = "timeout-control";
            timeoutController.Connect({timeoutChannel});
            negotiationTimeoutHandled = timeoutObserver.WaitForErrorCode(
                "negotiation_timeout", operationTimeout);
        }
        timeoutController.Close();
        timeoutController.Close();
        timeoutSessionClosed = timeoutObserver.WaitForState(
            SessionControllerState::kClosed, operationTimeout);
        timeoutController.SetObserver(nullptr);
    }
    AppendResult(report, "Negotiation timeout handled",
                 negotiationTimeoutHandled);
    AppendResult(report, "Timed-out session closed", timeoutSessionClosed);

    result.passed = peersCreated && transceiverCreated &&
                    h264Forced && offerContainsH264 && answerContainsH264 &&
                    playoutDelayNegotiated &&
                    candidateCacheExercised && senderConnected &&
                    receiverConnected && allChannelsOpened &&
                    allChannelParametersMatch && allChannelMessagesPassed &&
                    frameReceived && nativeFrameReceived &&
                    decodedSizeMatches && encoderReconfigured &&
                    framesReceivedAfterReconfigure &&
                    baseBootstrapWithoutFastStartup &&
                    senderClosed && receiverClosed &&
                    negotiationTimeoutHandled && timeoutSessionClosed;
    AppendResult(report, "SELF TEST PASSED", result.passed);

    senderController.SetObserver(nullptr);
    receiverController.SetObserver(nullptr);
    result.report = report.str();
    return result;
}

}  // namespace remote
