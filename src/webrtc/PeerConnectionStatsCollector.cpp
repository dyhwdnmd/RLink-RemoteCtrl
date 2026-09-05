// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#include "PeerConnectionStatsCollector.h"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <limits>
#include <mutex>
#include <string>
#include <unordered_map>
#include <utility>

#include "api/make_ref_counted.h"
#include "api/stats/rtc_stats_collector_callback.h"
#include "api/stats/rtc_stats_report.h"
#include "api/stats/rtcstats_objects.h"
#include "VideoCodecTimingTelemetry.h"

namespace remote {
namespace {

struct CounterSample {
    std::int64_t timestampUs = 0;
    std::uint64_t value = 0;
};

struct AggregateSample {
    std::int64_t timestampUs = 0;
    double total = 0.0;
    std::uint64_t count = 0;
};

struct WindowAverage {
    bool available = false;
    double value = 0.0;
    std::uint32_t windowMs = 0;
};

std::uint64_t CalculateRate(
    std::unordered_map<std::string, CounterSample>& history,
    const std::string& key,
    std::uint64_t value,
    std::int64_t timestampUs,
    std::uint32_t* windowMs = nullptr)
{
    std::uint64_t rate = 0;
    const auto previous = history.find(key);
    if (previous != history.end() &&
        timestampUs > previous->second.timestampUs &&
        value >= previous->second.value) {
        const std::uint64_t deltaBytes =
            value - previous->second.value;
        const std::int64_t deltaUs =
            timestampUs - previous->second.timestampUs;
        if (windowMs) {
            *windowMs = static_cast<std::uint32_t>(
                (std::min<std::int64_t>)(
                    deltaUs / 1000,
                    std::numeric_limits<std::uint32_t>::max()));
        }
        rate = static_cast<std::uint64_t>(
            (static_cast<long double>(deltaBytes) * 8'000'000.0L) /
            static_cast<long double>(deltaUs));
    }
    history[key] = {timestampUs, value};
    return rate;
}

double CalculateCounterPerSecond(
    std::unordered_map<std::string, CounterSample>& history,
    const std::string& key,
    std::uint64_t value,
    std::int64_t timestampUs)
{
    double rate = 0.0;
    const auto previous = history.find(key);
    if (previous != history.end() &&
        timestampUs > previous->second.timestampUs &&
        value >= previous->second.value) {
        const std::uint64_t delta =
            value - previous->second.value;
        const std::int64_t deltaUs =
            timestampUs - previous->second.timestampUs;
        rate =
            static_cast<double>(delta) * 1'000'000.0 /
            static_cast<double>(deltaUs);
    }
    history[key] = {timestampUs, value};
    return rate;
}

WindowAverage CalculateWindowAverage(
    std::unordered_map<std::string, AggregateSample>& history,
    const std::string& key,
    double total,
    std::uint64_t count,
    std::int64_t timestampUs,
    double scale)
{
    WindowAverage result;
    const auto previous = history.find(key);
    if (previous != history.end() &&
        timestampUs > previous->second.timestampUs &&
        count > previous->second.count &&
        total >= previous->second.total) {
        const std::uint64_t deltaCount =
            count - previous->second.count;
        result.available = true;
        result.value =
            (total - previous->second.total) * scale /
            static_cast<double>(deltaCount);
        result.windowMs = static_cast<std::uint32_t>(
            (std::min<std::int64_t>)(
                (timestampUs - previous->second.timestampUs) / 1000,
                std::numeric_limits<std::uint32_t>::max()));
    }
    history[key] = {timestampUs, total, count};
    return result;
}

void ApplyLatestFrameTiming(
    RtpStreamStatsSnapshot& stream,
    std::string& implementation,
    VideoCodecTimingDirection expectedDirection)
{
    const auto timing =
        VideoCodecTimingRegistry::Instance()
            .SnapshotForImplementation(implementation);
    implementation =
        VideoCodecTimingRegistry::UntaggedImplementation(
            implementation);
    if (!timing || timing->direction != expectedDirection) {
        return;
    }

    implementation = timing->implementation;
    stream.latestFrameTimingAvailable = true;
    stream.latestFrameTimeMs =
        static_cast<double>(timing->latestFrameDurationUs) / 1000.0;
    stream.averageFrameTimingMs =
        static_cast<double>(timing->averageFrameDurationUs) / 1000.0;
    stream.p95FrameTimingMs =
        static_cast<double>(timing->p95FrameDurationUs) / 1000.0;
    stream.maximumFrameTimingMs =
        static_cast<double>(timing->maximumFrameDurationUs) / 1000.0;
    stream.latestFrameRtpTimestamp = timing->rtpTimestamp;
    stream.latestFrameWidth = timing->frameWidth;
    stream.latestFrameHeight = timing->frameHeight;
    stream.latestEncodedBytes = timing->encodedBytes;
    stream.latestFrameQpAvailable = timing->qpAvailable;
    stream.latestFrameQp = timing->qp;
    stream.timingCompletedFrames = timing->completedFrames;
    stream.timingDroppedFrames = timing->droppedFrames;
    const auto& pipeline = timing->decodePipeline;
    stream.decodePipelineTimingAvailable = pipeline.available;
    if (pipeline.available) {
        stream.decodePipelineAsynchronous = pipeline.asynchronous;
        stream.decodeInputPreparationMs =
            static_cast<double>(pipeline.inputPreparationUs) / 1000.0;
        stream.decodeInputQueueWaitMs =
            static_cast<double>(pipeline.inputQueueWaitUs) / 1000.0;
        stream.decodeTransformWaitMs =
            static_cast<double>(pipeline.transformWaitUs) / 1000.0;
        stream.averageDecodeTransformWaitMs =
            static_cast<double>(pipeline.averageTransformWaitUs) / 1000.0;
        stream.p95DecodeTransformWaitMs =
            static_cast<double>(pipeline.p95TransformWaitUs) / 1000.0;
        stream.maximumDecodeTransformWaitMs =
            static_cast<double>(pipeline.maximumTransformWaitUs) / 1000.0;
        stream.decodeOutputDeliveryMs =
            static_cast<double>(pipeline.outputDeliveryUs) / 1000.0;
        stream.decodeQueuedInputFrames = pipeline.queuedInputFrames;
        stream.decodeInFlightFrames = pipeline.inFlightFrames;
        stream.decodePeakBacklogFrames = pipeline.peakBacklogFrames;
        stream.decodeBackpressureFallbacks =
            pipeline.backpressureFallbacks;
    }
    const std::int64_t ageUs =
        VideoCodecTimingRegistry::SteadyNowUs() -
        timing->completedAtSteadyUs;
    if (ageUs > 0) {
        stream.latestFrameTimingAgeMs =
            static_cast<std::uint32_t>(
                (std::min<std::int64_t>)(
                    ageUs / 1000,
                    std::numeric_limits<std::uint32_t>::max()));
    }
}

std::string CodecName(
    const webrtc::RTCStatsReport& report,
    const std::optional<std::string>& codecId)
{
    if (!codecId) {
        return {};
    }
    const auto* codec =
        report.GetAs<webrtc::RTCCodecStats>(*codecId);
    return codec && codec->mime_type
               ? *codec->mime_type
               : std::string{};
}

IceCandidateStatsSnapshot CandidateSnapshot(
    const webrtc::RTCIceCandidateStats* candidate)
{
    IceCandidateStatsSnapshot snapshot;
    if (!candidate) {
        return snapshot;
    }
    snapshot.candidateType =
        candidate->candidate_type.value_or("");
    snapshot.protocol = candidate->protocol.value_or("");
    snapshot.address = candidate->address.value_or(
        candidate->ip.value_or(""));
    snapshot.port = candidate->port.value_or(0);
    snapshot.networkType = candidate->network_type.value_or("");
    snapshot.adapterType =
        candidate->network_adapter_type.value_or("");
    snapshot.relayProtocol =
        candidate->relay_protocol.value_or("");
    return snapshot;
}

std::string RouteType(
    const IceCandidateStatsSnapshot& local,
    const IceCandidateStatsSnapshot& remote)
{
    if (local.candidateType == "relay" ||
        remote.candidateType == "relay") {
        return "turn";
    }
    if (local.candidateType == "srflx" ||
        local.candidateType == "prflx" ||
        remote.candidateType == "srflx" ||
        remote.candidateType == "prflx") {
        return "stun";
    }
    if (local.candidateType == "host" ||
        remote.candidateType == "host") {
        return "host";
    }
    return "unknown";
}

double LossPercent(std::int64_t lost, std::uint64_t received)
{
    if (lost <= 0) {
        return 0.0;
    }
    const long double total =
        static_cast<long double>(lost) +
        static_cast<long double>(received);
    return total > 0.0L
               ? static_cast<double>(
                     static_cast<long double>(lost) * 100.0L / total)
               : 0.0;
}

}  // namespace

struct PeerConnectionStatsCollector::State {
    mutable std::mutex mutex;
    bool requestPending = false;
    WebRtcSessionStatsSnapshot snapshot;
    std::unordered_map<std::string, CounterSample> counterHistory;
    std::unordered_map<std::string, AggregateSample> aggregateHistory;
};

class PeerConnectionStatsCollector::Callback
    : public webrtc::RTCStatsCollectorCallback {
public:
    Callback(
        std::shared_ptr<State> state,
        CompletionCallback completion)
        : state_(std::move(state)), completion_(std::move(completion))
    {}

    void OnStatsDelivered(
        const webrtc::scoped_refptr<const webrtc::RTCStatsReport>& report)
        override
    {
        ProcessReport(state_, report);
        if (completion_) {
            completion_();
        }
    }

private:
    std::shared_ptr<State> state_;
    CompletionCallback completion_;
};

PeerConnectionStatsCollector::PeerConnectionStatsCollector()
    : state_(std::make_shared<State>())
{}

PeerConnectionStatsCollector::~PeerConnectionStatsCollector() = default;

void PeerConnectionStatsCollector::Request(
    webrtc::scoped_refptr<webrtc::PeerConnectionInterface> peer,
    CompletionCallback completion)
{
    if (!peer) {
        return;
    }
    {
        std::lock_guard lock(state_->mutex);
        if (state_->requestPending) {
            return;
        }
        state_->requestPending = true;
    }
    auto callback = webrtc::make_ref_counted<Callback>(
        state_, std::move(completion));
    peer->GetStats(callback.get());
}

WebRtcSessionStatsSnapshot
PeerConnectionStatsCollector::Snapshot() const
{
    std::lock_guard lock(state_->mutex);
    return state_->snapshot;
}

void PeerConnectionStatsCollector::ProcessReport(
    const std::shared_ptr<State>& state,
    const webrtc::scoped_refptr<const webrtc::RTCStatsReport>& report)
{
    if (!state) {
        return;
    }
    if (!report) {
        std::lock_guard lock(state->mutex);
        state->requestPending = false;
        return;
    }

    std::lock_guard lock(state->mutex);
    const std::int64_t timestampUs = report->timestamp().us();
    WebRtcSessionStatsSnapshot next;
    next.transport.collected = true;
    next.transport.timestampMs = report->timestamp().ms();

    const webrtc::RTCTransportStats* selectedTransport = nullptr;
    const webrtc::RTCIceCandidatePairStats* selectedPair = nullptr;
    for (const auto* transport :
         report->GetStatsOfType<webrtc::RTCTransportStats>()) {
        if (!transport->selected_candidate_pair_id) {
            continue;
        }
        const auto* pair =
            report->GetAs<webrtc::RTCIceCandidatePairStats>(
                *transport->selected_candidate_pair_id);
        if (!pair) {
            continue;
        }
        selectedTransport = transport;
        selectedPair = pair;
        if (pair->nominated.value_or(false)) {
            break;
        }
    }

    if (selectedTransport) {
        next.transport.iceState =
            selectedTransport->ice_state.value_or("");
        next.transport.dtlsState =
            selectedTransport->dtls_state.value_or("");
        next.transport.iceRole =
            selectedTransport->ice_role.value_or("");
        next.transport.tlsVersion =
            selectedTransport->tls_version.value_or("");
        next.transport.dtlsCipher =
            selectedTransport->dtls_cipher.value_or("");
        next.transport.srtpCipher =
            selectedTransport->srtp_cipher.value_or("");
        next.transport.selectedCandidatePairChanges =
            selectedTransport->selected_candidate_pair_changes
                .value_or(0);
    }
    if (selectedPair) {
        next.transport.candidatePairState =
            selectedPair->state.value_or("");
        next.transport.nominated =
            selectedPair->nominated.value_or(false);
        next.transport.writable =
            selectedPair->writable.value_or(false);
        next.transport.packetsDiscardedOnSend =
            selectedPair->packets_discarded_on_send.value_or(0);
        next.transport.bytesDiscardedOnSend =
            selectedPair->bytes_discarded_on_send.value_or(0);
        next.transport.currentRoundTripTimeMs =
            selectedPair->current_round_trip_time.value_or(0.0) *
            1000.0;
        next.transport.availableOutgoingBitrateBps =
            static_cast<std::uint64_t>(
                (std::max)(0.0,
                    selectedPair->available_outgoing_bitrate
                        .value_or(0.0)));
        next.transport.availableIncomingBitrateBps =
            static_cast<std::uint64_t>(
                (std::max)(0.0,
                    selectedPair->available_incoming_bitrate
                        .value_or(0.0)));
        next.transport.bytesSent =
            selectedPair->bytes_sent.value_or(0);
        next.transport.bytesReceived =
            selectedPair->bytes_received.value_or(0);
        next.transport.sendBitrateBps = CalculateRate(
            state->counterHistory,
            "transport:sent",
            next.transport.bytesSent,
            timestampUs,
            &next.transport.sampleWindowMs);
        next.transport.receiveBitrateBps = CalculateRate(
            state->counterHistory,
            "transport:received",
            next.transport.bytesReceived,
            timestampUs);

        const webrtc::RTCLocalIceCandidateStats* local = nullptr;
        const webrtc::RTCRemoteIceCandidateStats* remote = nullptr;
        if (selectedPair->local_candidate_id) {
            local =
                report->GetAs<webrtc::RTCLocalIceCandidateStats>(
                    *selectedPair->local_candidate_id);
        }
        if (selectedPair->remote_candidate_id) {
            remote =
                report->GetAs<webrtc::RTCRemoteIceCandidateStats>(
                    *selectedPair->remote_candidate_id);
        }
        next.transport.localCandidate = CandidateSnapshot(local);
        next.transport.remoteCandidate = CandidateSnapshot(remote);
        next.transport.routeType = RouteType(
            next.transport.localCandidate,
            next.transport.remoteCandidate);
    }

    std::unordered_map<
        std::string,
        const webrtc::RTCRemoteInboundRtpStreamStats*> remoteInbound;
    for (const auto* remote :
         report->GetStatsOfType<
             webrtc::RTCRemoteInboundRtpStreamStats>()) {
        if (remote->local_id) {
            remoteInbound[*remote->local_id] = remote;
        }
    }

    for (const auto* outbound :
         report->GetStatsOfType<
             webrtc::RTCOutboundRtpStreamStats>()) {
        RtpStreamStatsSnapshot stream;
        stream.statsId = std::string(outbound->id());
        stream.direction = RtpStreamDirection::kOutbound;
        stream.kind = outbound->kind.value_or("");
        stream.mid = outbound->mid.value_or("");
        stream.codec = CodecName(*report, outbound->codec_id);
        stream.bytes = outbound->bytes_sent.value_or(0);
        stream.bitrateBps = CalculateRate(
            state->counterHistory,
            "rtp:out:" + stream.statsId,
            stream.bytes,
            timestampUs,
            &stream.sampleWindowMs);
        stream.packets = outbound->packets_sent.value_or(0);
        stream.retransmittedPackets =
            outbound->retransmitted_packets_sent.value_or(0);
        stream.retransmittedBytes =
            outbound->retransmitted_bytes_sent.value_or(0);
        stream.targetBitrateBps =
            static_cast<std::uint64_t>((std::max)(
                0.0, outbound->target_bitrate.value_or(0.0)));
        stream.frameWidth = outbound->frame_width.value_or(0);
        stream.frameHeight = outbound->frame_height.value_or(0);
        stream.framesPerSecond =
            outbound->frames_per_second.value_or(0.0);
        stream.framesEncoded =
            outbound->frames_encoded.value_or(0);
        stream.framesSent =
            outbound->frames_sent.value_or(0);
        stream.encodedFramesPerSecond = CalculateCounterPerSecond(
            state->counterHistory,
            "frames:encoded:" + stream.statsId,
            stream.framesEncoded,
            timestampUs);
        stream.sentFramesPerSecond = CalculateCounterPerSecond(
            state->counterHistory,
            "frames:sent:" + stream.statsId,
            stream.framesSent,
            timestampUs);
        stream.keyFrames =
            outbound->key_frames_encoded.value_or(0);
        if (stream.framesEncoded > 0) {
            stream.averageEncodeTimeMs =
                outbound->total_encode_time.value_or(0.0) *
                1000.0 / stream.framesEncoded;
            stream.averageQp =
                static_cast<double>(
                    outbound->qp_sum.value_or(0)) /
                stream.framesEncoded;
        }
        const auto windowEncode = CalculateWindowAverage(
            state->aggregateHistory,
            "encode:" + stream.statsId,
            outbound->total_encode_time.value_or(0.0),
            stream.framesEncoded,
            timestampUs,
            1000.0);
        stream.windowEncodeTimeAvailable = windowEncode.available;
        stream.windowEncodeTimeMs = windowEncode.value;
        if (stream.sampleWindowMs == 0) {
            stream.sampleWindowMs = windowEncode.windowMs;
        }
        if (outbound->qp_sum) {
            const auto windowQp = CalculateWindowAverage(
                state->aggregateHistory,
                "qp:out:" + stream.statsId,
                static_cast<double>(*outbound->qp_sum),
                stream.framesEncoded,
                timestampUs,
                1.0);
            stream.windowQpAvailable = windowQp.available;
            stream.windowQp = windowQp.value;
        }
        if (outbound->media_source_id) {
            const auto* source =
                report->GetAs<webrtc::RTCVideoSourceStats>(
                    *outbound->media_source_id);
            if (source) {
                stream.sourceWidth = source->width.value_or(0);
                stream.sourceHeight = source->height.value_or(0);
                stream.sourceFramesPerSecond =
                    source->frames_per_second.value_or(0.0);
            }
        }
        stream.nackCount = outbound->nack_count.value_or(0);
        stream.pliCount = outbound->pli_count.value_or(0);
        stream.firCount = outbound->fir_count.value_or(0);
        stream.encoderImplementation =
            outbound->encoder_implementation.value_or("");
        ApplyLatestFrameTiming(
            stream,
            stream.encoderImplementation,
            VideoCodecTimingDirection::kEncode);
        stream.qualityLimitationReason =
            outbound->quality_limitation_reason.value_or("");
        stream.active = outbound->active.value_or(true);
        stream.powerEfficient =
            outbound->power_efficient_encoder.value_or(false);
        const auto remote = remoteInbound.find(stream.statsId);
        if (remote != remoteInbound.end()) {
            stream.packetsLost =
                remote->second->packets_lost.value_or(0);
            stream.lossPercent =
                remote->second->fraction_lost.value_or(0.0) *
                100.0;
            stream.roundTripTimeMs =
                remote->second->round_trip_time.value_or(0.0) *
                1000.0;
        }
        next.rtpStreams.push_back(std::move(stream));
    }

    for (const auto* inbound :
         report->GetStatsOfType<
             webrtc::RTCInboundRtpStreamStats>()) {
        RtpStreamStatsSnapshot stream;
        stream.statsId = std::string(inbound->id());
        stream.direction = RtpStreamDirection::kInbound;
        stream.kind = inbound->kind.value_or("");
        stream.mid = inbound->mid.value_or("");
        stream.codec = CodecName(*report, inbound->codec_id);
        stream.bytes = inbound->bytes_received.value_or(0);
        stream.bitrateBps = CalculateRate(
            state->counterHistory,
            "rtp:in:" + stream.statsId,
            stream.bytes,
            timestampUs,
            &stream.sampleWindowMs);
        stream.packets = inbound->packets_received.value_or(0);
        stream.packetsLost = inbound->packets_lost.value_or(0);
        stream.retransmittedPackets =
            inbound->retransmitted_packets_received.value_or(0);
        stream.retransmittedBytes =
            inbound->retransmitted_bytes_received.value_or(0);
        stream.lossPercent =
            LossPercent(stream.packetsLost, stream.packets);
        stream.jitterMs =
            inbound->jitter.value_or(0.0) * 1000.0;
        stream.frameWidth = inbound->frame_width.value_or(0);
        stream.frameHeight = inbound->frame_height.value_or(0);
        stream.framesPerSecond =
            inbound->frames_per_second.value_or(0.0);
        stream.framesDecoded =
            inbound->frames_decoded.value_or(0);
        stream.framesDropped =
            inbound->frames_dropped.value_or(0);
        stream.keyFrames =
            inbound->key_frames_decoded.value_or(0);
        if (stream.framesDecoded > 0) {
            stream.averageDecodeTimeMs =
                inbound->total_decode_time.value_or(0.0) *
                1000.0 / stream.framesDecoded;
            stream.averageProcessingDelayMs =
                inbound->total_processing_delay.value_or(0.0) *
                1000.0 / stream.framesDecoded;
            stream.averageQp =
                static_cast<double>(
                    inbound->qp_sum.value_or(0)) /
                stream.framesDecoded;
        }
        const auto windowDecode = CalculateWindowAverage(
            state->aggregateHistory,
            "decode:" + stream.statsId,
            inbound->total_decode_time.value_or(0.0),
            stream.framesDecoded,
            timestampUs,
            1000.0);
        stream.windowDecodeTimeAvailable = windowDecode.available;
        stream.windowDecodeTimeMs = windowDecode.value;
        if (stream.sampleWindowMs == 0) {
            stream.sampleWindowMs = windowDecode.windowMs;
        }
        const auto windowProcessing = CalculateWindowAverage(
            state->aggregateHistory,
            "processing:" + stream.statsId,
            inbound->total_processing_delay.value_or(0.0),
            stream.framesDecoded,
            timestampUs,
            1000.0);
        stream.windowProcessingDelayAvailable =
            windowProcessing.available;
        stream.windowProcessingDelayMs = windowProcessing.value;
        if (inbound->qp_sum) {
            const auto windowQp = CalculateWindowAverage(
                state->aggregateHistory,
                "qp:in:" + stream.statsId,
                static_cast<double>(*inbound->qp_sum),
                stream.framesDecoded,
                timestampUs,
                1.0);
            stream.windowQpAvailable = windowQp.available;
            stream.windowQp = windowQp.value;
        }
        const auto jitterBufferEmitted =
            inbound->jitter_buffer_emitted_count.value_or(0);
        if (jitterBufferEmitted > 0) {
            stream.averageJitterBufferDelayMs =
                inbound->jitter_buffer_delay.value_or(0.0) *
                1000.0 / jitterBufferEmitted;
        }
        const auto windowJitterBuffer = CalculateWindowAverage(
            state->aggregateHistory,
            "jitter-buffer:" + stream.statsId,
            inbound->jitter_buffer_delay.value_or(0.0),
            jitterBufferEmitted,
            timestampUs,
            1000.0);
        stream.windowJitterBufferDelayAvailable =
            windowJitterBuffer.available;
        stream.windowJitterBufferDelayMs =
            windowJitterBuffer.value;
        stream.audioLevel = inbound->audio_level.value_or(0.0);
        stream.totalAudioSamples =
            inbound->total_samples_received.value_or(0);
        stream.concealedAudioSamples =
            inbound->concealed_samples.value_or(0);
        stream.concealmentEvents =
            inbound->concealment_events.value_or(0);
        stream.freezeCount = inbound->freeze_count.value_or(0);
        stream.totalFreezeDurationMs =
            inbound->total_freezes_duration.value_or(0.0) *
            1000.0;
        stream.pauseCount = inbound->pause_count.value_or(0);
        stream.totalPauseDurationMs =
            inbound->total_pauses_duration.value_or(0.0) *
            1000.0;
        stream.nackCount = inbound->nack_count.value_or(0);
        stream.pliCount = inbound->pli_count.value_or(0);
        stream.firCount = inbound->fir_count.value_or(0);
        stream.decoderImplementation =
            inbound->decoder_implementation.value_or("");
        ApplyLatestFrameTiming(
            stream,
            stream.decoderImplementation,
            VideoCodecTimingDirection::kDecode);
        stream.powerEfficient =
            inbound->power_efficient_decoder.value_or(false);
        stream.active = true;
        next.rtpStreams.push_back(std::move(stream));
    }

    for (const auto* channel :
         report->GetStatsOfType<webrtc::RTCDataChannelStats>()) {
        DataChannelStatsSnapshot snapshot;
        snapshot.label = channel->label.value_or("");
        snapshot.protocol = channel->protocol.value_or("");
        snapshot.state = channel->state.value_or("");
        snapshot.bytesSent = channel->bytes_sent.value_or(0);
        snapshot.bytesReceived =
            channel->bytes_received.value_or(0);
        snapshot.sendBitrateBps = CalculateRate(
            state->counterHistory,
            "dc:out:" + std::string(channel->id()),
            snapshot.bytesSent,
            timestampUs,
            &snapshot.sampleWindowMs);
        snapshot.receiveBitrateBps = CalculateRate(
            state->counterHistory,
            "dc:in:" + std::string(channel->id()),
            snapshot.bytesReceived,
            timestampUs);
        snapshot.messagesSent =
            channel->messages_sent.value_or(0);
        snapshot.messagesReceived =
            channel->messages_received.value_or(0);
        next.dataChannels.push_back(std::move(snapshot));
    }

    state->snapshot = std::move(next);
    state->requestPending = false;
}

}  // namespace remote
