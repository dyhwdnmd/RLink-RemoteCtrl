// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#include "LibWebRtcSession.h"

#include <algorithm>
#include <chrono>
#include <cctype>
#include <condition_variable>
#include <functional>
#include <limits>
#include <utility>
#include <vector>

#include "api/data_channel_interface.h"
#include "api/jsep.h"
#include "api/make_ref_counted.h"
#include "rtc_base/copy_on_write_buffer.h"
#include "src/core/ScreenNetworkPolicy.h"
#include "src/webrtc/PeerConnectionStatsCollector.h"
#include "src/core/ScreenStreamPolicy.h"

namespace remote {
namespace {

constexpr int kMaximumScreenBitrateBps = 100'000'000;
constexpr int kDesktopStartupProbeFloorBps = 2'000'000;
constexpr int kDefaultWebRtcMinimumBitrateBps = 30'000;

std::uint64_t SteadyNowMs()
{
    return static_cast<std::uint64_t>(
        std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::steady_clock::now().time_since_epoch()).count());
}

const char* ProgressiveBitrateCeilingStatusName(
    ProgressiveBitrateCeilingStatus status)
{
    switch (status) {
    case ProgressiveBitrateCeilingStatus::kDisabled:
        return "disabled";
    case ProgressiveBitrateCeilingStatus::kWaitingForStats:
        return "waiting_for_stats";
    case ProgressiveBitrateCeilingStatus::kStabilizing:
        return "stabilizing";
    case ProgressiveBitrateCeilingStatus::kStable:
        return "stable";
    case ProgressiveBitrateCeilingStatus::kProbePending:
        return "probe_pending";
    case ProgressiveBitrateCeilingStatus::kCooldown:
        return "cooldown";
    }
    return "unknown";
}

const char* AdaptiveScreenFrameRateStatusName(
    AdaptiveScreenFrameRateStatus status)
{
    switch (status) {
    case AdaptiveScreenFrameRateStatus::kDisabled:
        return "disabled";
    case AdaptiveScreenFrameRateStatus::kWaitingForActivity:
        return "waiting_for_activity";
    case AdaptiveScreenFrameRateStatus::kIdleSuspended:
        return "idle_suspended";
    case AdaptiveScreenFrameRateStatus::kStartupGrace:
        return "startup_grace";
    case AdaptiveScreenFrameRateStatus::kWaitingForCapacity:
        return "waiting_for_capacity";
    case AdaptiveScreenFrameRateStatus::kStable:
        return "stable";
    case AdaptiveScreenFrameRateStatus::kReducing:
        return "reducing";
    case AdaptiveScreenFrameRateStatus::kRecovering:
        return "recovering";
    }
    return "unknown";
}

class CreateDescriptionCallback
    : public webrtc::CreateSessionDescriptionObserver {
public:
    using SuccessCallback = std::function<void(
        std::unique_ptr<webrtc::SessionDescriptionInterface>)>;
    using FailureCallback = std::function<void(webrtc::RTCError)>;

    CreateDescriptionCallback(SuccessCallback success,
                              FailureCallback failure)
        : success_(std::move(success)), failure_(std::move(failure))
    {}

    void OnSuccess(webrtc::SessionDescriptionInterface* description) override
    {
        success_(
            std::unique_ptr<webrtc::SessionDescriptionInterface>(description));
    }

    void OnFailure(webrtc::RTCError error) override
    {
        failure_(std::move(error));
    }

private:
    SuccessCallback success_;
    FailureCallback failure_;
};

class SetDescriptionCallback
    : public webrtc::SetSessionDescriptionObserver {
public:
    using SuccessCallback = std::function<void()>;
    using FailureCallback = std::function<void(webrtc::RTCError)>;

    SetDescriptionCallback(SuccessCallback success, FailureCallback failure)
        : success_(std::move(success)), failure_(std::move(failure))
    {}

    void OnSuccess() override { success_(); }

    void OnFailure(webrtc::RTCError error) override
    {
        failure_(std::move(error));
    }

private:
    SuccessCallback success_;
    FailureCallback failure_;
};

webrtc::SdpType ToNativeSdpType(SessionDescriptionType type)
{
    return type == SessionDescriptionType::kOffer ? webrtc::SdpType::kOffer
                                                  : webrtc::SdpType::kAnswer;
}

DataChannelState ToPublicDataChannelState(
    webrtc::DataChannelInterface::DataState state)
{
    switch (state) {
    case webrtc::DataChannelInterface::kConnecting:
        return DataChannelState::kConnecting;
    case webrtc::DataChannelInterface::kOpen:
        return DataChannelState::kOpen;
    case webrtc::DataChannelInterface::kClosing:
        return DataChannelState::kClosing;
    case webrtc::DataChannelInterface::kClosed:
        return DataChannelState::kClosed;
    }
    return DataChannelState::kClosed;
}

WebRtcIceGatheringState ToPublicIceGatheringState(
    webrtc::PeerConnectionInterface::IceGatheringState state)
{
    switch (state) {
    case webrtc::PeerConnectionInterface::kIceGatheringNew:
        return WebRtcIceGatheringState::kNew;
    case webrtc::PeerConnectionInterface::kIceGatheringGathering:
        return WebRtcIceGatheringState::kGathering;
    case webrtc::PeerConnectionInterface::kIceGatheringComplete:
        return WebRtcIceGatheringState::kComplete;
    }
    return WebRtcIceGatheringState::kNew;
}

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

}  // namespace

#include "LibWebRtcSessionInternal.inc"
#include "LibWebRtcSession.Lifecycle.inc"
#include "LibWebRtcSession.Negotiation.inc"
#include "LibWebRtcSession.DataChannels.inc"
#include "LibWebRtcSession.StatsClose.inc"
#include "LibWebRtcSession.VideoSlots.inc"
#include "LibWebRtcSession.AudioSlot.inc"
#include "LibWebRtcSession.Observers.inc"
#include "LibWebRtcSession.NegotiationInternals.inc"
#include "LibWebRtcSession.DataChannelCallbacks.inc"
#include "LibWebRtcSession.State.inc"
void LibWebRtcSession::DetachRemoteVideoSink()
{
    SetRemoteVideoSink(nullptr);
}

}  // namespace remote
