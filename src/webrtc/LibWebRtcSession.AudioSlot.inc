// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

webrtc::RTCError LibWebRtcSession::PrepareAudioTransceiverSlot(
    const std::string& slot)
{
    if (slot.empty()) {
        return webrtc::RTCError::InvalidParameter(
            "The audio slot name is empty.");
    }
    auto peer = PeerConnection();
    if (!peer) {
        return webrtc::RTCError::InvalidState(
            "PeerConnection is not ready.");
    }
    {
        std::lock_guard lock(mutex_);
        if (audioSlot_.transceiver) {
            return audioSlot_.name == slot
                       ? webrtc::RTCError::OK()
                       : webrtc::RTCError::InvalidState(
                             "A different audio slot is already prepared.");
        }
    }
    webrtc::RtpTransceiverInit init;
    init.direction = webrtc::RtpTransceiverDirection::kSendRecv;
    auto transceiverOrError =
        peer->AddTransceiver(webrtc::MediaType::AUDIO, init);
    if (!transceiverOrError.ok()) {
        return transceiverOrError.MoveError();
    }
    auto transceiver = transceiverOrError.MoveValue();
    {
        std::lock_guard lock(mutex_);
        if (audioSlot_.transceiver) {
            return webrtc::RTCError::InvalidModification(
                "The audio slot was concurrently prepared.");
        }
        audioSlot_.name = slot;
        audioSlot_.transceiver = std::move(transceiver);
    }
    return webrtc::RTCError::OK();
}

webrtc::RTCError LibWebRtcSession::BindNegotiatedAudioTransceiverSlot(
    const std::string& slot)
{
    if (slot.empty()) {
        return webrtc::RTCError::InvalidParameter(
            "The audio slot name is empty.");
    }
    auto peer = PeerConnection();
    if (!peer) {
        return webrtc::RTCError::InvalidState(
            "PeerConnection is not ready.");
    }
    webrtc::scoped_refptr<webrtc::RtpTransceiverInterface> audio;
    for (const auto& transceiver : peer->GetTransceivers()) {
        if (transceiver &&
            transceiver->media_type() == webrtc::MediaType::AUDIO &&
            transceiver->mid().has_value() && !transceiver->stopped()) {
            audio = transceiver;
            break;
        }
    }
    if (!audio) {
        return webrtc::RTCError::InvalidState(
            "The remote offer does not contain the microphone audio slot.");
    }
    auto directionResult = audio->SetDirectionWithError(
        webrtc::RtpTransceiverDirection::kSendRecv);
    if (!directionResult.ok()) {
        return directionResult;
    }
    std::lock_guard lock(mutex_);
    if (audioSlot_.transceiver) {
        return webrtc::RTCError::InvalidState(
            "The audio slot is already bound.");
    }
    audioSlot_.name = slot;
    audioSlot_.transceiver = std::move(audio);
    return webrtc::RTCError::OK();
}

webrtc::RTCError LibWebRtcSession::SetAudioSlotTrack(
    const std::string& slot,
    webrtc::scoped_refptr<webrtc::MediaStreamTrackInterface> track)
{
    if (track &&
        track->kind() != webrtc::MediaStreamTrackInterface::kAudioKind) {
        return webrtc::RTCError::InvalidParameter(
            "Only an audio track may be attached to an audio slot.");
    }
    webrtc::scoped_refptr<webrtc::RtpTransceiverInterface> transceiver;
    {
        std::lock_guard lock(mutex_);
        if (audioSlot_.name != slot || !audioSlot_.transceiver) {
            return webrtc::RTCError::InvalidState(
                "The requested audio slot is not prepared.");
        }
        transceiver = audioSlot_.transceiver;
    }
    auto sender = transceiver->sender();
    if (!sender) {
        return webrtc::RTCError::InternalError(
            "The audio transceiver has no sender.");
    }

    // Muting the previous track before replacing it is the privacy boundary.
    // Even if libwebrtc rejects SetTrack(nullptr), captured samples from the
    // old microphone can no longer enter its RTP sender.
    auto previousTrack = sender->track();
    if (previousTrack && previousTrack.get() != track.get()) {
        previousTrack->set_enabled(false);
    }
    if (!sender->SetTrack(track.get())) {
        return webrtc::RTCError::InternalError(
            "The audio sender rejected the track.");
    }
    return webrtc::RTCError::OK();
}

void LibWebRtcSession::SetRemoteAudioSlotEnabled(
    const std::string& slot, bool enabled)
{
    webrtc::scoped_refptr<webrtc::AudioTrackInterface> track;
    {
        std::lock_guard lock(mutex_);
        if (audioSlot_.name != slot) {
            return;
        }
        audioSlot_.remotePlaybackEnabled = enabled;
        track = audioSlot_.remoteTrack;
    }
    if (track) {
        track->set_enabled(enabled);
    }
}

bool LibWebRtcSession::AudioSlotPrepared(const std::string& slot) const
{
    std::lock_guard lock(mutex_);
    return audioSlot_.name == slot && audioSlot_.transceiver;
}

std::size_t LibWebRtcSession::PreparedVideoSlotCount() const
{
    std::lock_guard lock(mutex_);
    return videoSlots_.size();
}

void LibWebRtcSession::SetRemoteVideoSink(
    webrtc::VideoSinkInterface<webrtc::VideoFrame>* sink)
{
    webrtc::scoped_refptr<webrtc::VideoTrackInterface> track;
    webrtc::VideoSinkInterface<webrtc::VideoFrame>* previousSink = nullptr;
    {
        std::lock_guard lock(mutex_);
        track = remoteVideoTrack_;
        previousSink = remoteVideoSink_;
        remoteVideoSink_ = sink;
    }
    if (track && previousSink && previousSink != sink) {
        track->RemoveSink(previousSink);
    }
    if (track && sink && previousSink != sink) {
        track->AddOrUpdateSink(sink, webrtc::VideoSinkWants());
        // Binding a viewer after the RTP stream has already started can miss
        // the first keyframe and leave the window blank until the encoder's
        // next periodic keyframe. The remote RTP track source turns this into
        // an immediate keyframe request (PLI) to the sender.
        if (auto* source = track->GetSource()) {
            source->GenerateKeyFrame();
        }
    }
}

void LibWebRtcSession::SetRemoteVideoSlotSink(
    const std::string& slot,
    webrtc::VideoSinkInterface<webrtc::VideoFrame>* sink)
{
    webrtc::scoped_refptr<webrtc::VideoTrackInterface> track;
    webrtc::VideoSinkInterface<webrtc::VideoFrame>* previousSink = nullptr;
    {
        std::lock_guard lock(mutex_);
        const auto found = videoSlots_.find(slot);
        if (found == videoSlots_.end()) {
            return;
        }
        track = found->second.remoteTrack;
        previousSink = found->second.remoteSink;
        found->second.remoteSink = sink;
    }
    if (track && previousSink && previousSink != sink) {
        track->RemoveSink(previousSink);
    }
    if (track && sink && previousSink != sink) {
        track->AddOrUpdateSink(sink, webrtc::VideoSinkWants());
        // Room screen/camera windows bind negotiated named slots through this
        // method. A late-bound screen sink must request a fresh keyframe from
        // the remote RTP source instead of waiting for the next periodic one.
        if (auto* source = track->GetSource()) {
            source->GenerateKeyFrame();
        }
    }
}
