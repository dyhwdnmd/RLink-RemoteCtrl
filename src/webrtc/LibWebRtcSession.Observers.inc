// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

void LibWebRtcSession::OnSignalingChange(
    webrtc::PeerConnectionInterface::SignalingState)
{}

void LibWebRtcSession::OnDataChannel(
    webrtc::scoped_refptr<webrtc::DataChannelInterface> channel)
{
    AttachDataChannel(std::move(channel));
}

void LibWebRtcSession::OnIceGatheringChange(
    webrtc::PeerConnectionInterface::IceGatheringState state)
{
    if (auto* observer = Observer()) {
        observer->OnIceGatheringStateChanged(
            ToPublicIceGatheringState(state));
    }
}

void LibWebRtcSession::OnIceCandidate(const webrtc::IceCandidate* candidate)
{
    if (!candidate) {
        return;
    }
    IceCandidate publicCandidate;
    publicCandidate.sdpMid = candidate->sdp_mid();
    publicCandidate.sdpMLineIndex = candidate->sdp_mline_index();
    publicCandidate.candidate = candidate->ToString();
    if (auto* observer = Observer()) {
        observer->OnLocalIceCandidate(publicCandidate);
    }
}

void LibWebRtcSession::OnConnectionChange(
    webrtc::PeerConnectionInterface::PeerConnectionState state)
{
    UpdatePeerConnectionState(state);
}

void LibWebRtcSession::OnIceConnectionChange(
    webrtc::PeerConnectionInterface::IceConnectionState state)
{
    UpdateIceConnectionState(state);
}

void LibWebRtcSession::OnTrack(
    webrtc::scoped_refptr<webrtc::RtpTransceiverInterface> transceiver)
{
    if (!transceiver || !transceiver->receiver()) {
        return;
    }
    auto track = transceiver->receiver()->track();
    if (!track) {
        return;
    }

    if (track->kind() ==
        webrtc::MediaStreamTrackInterface::kAudioKind) {
        auto* audioTrack =
            static_cast<webrtc::AudioTrackInterface*>(track.get());
        webrtc::scoped_refptr<webrtc::AudioTrackInterface> currentTrack(
            audioTrack);
        webrtc::scoped_refptr<webrtc::RtpTransceiverInterface>
            previousTransceiver;
        webrtc::scoped_refptr<webrtc::AudioTrackInterface> previousTrack;
        bool enabled = true;
        std::string slot;
        {
            std::lock_guard lock(mutex_);
            if (audioSlot_.transceiver == transceiver ||
                !audioSlot_.name.empty()) {
                previousTransceiver =
                    std::move(audioSlot_.transceiver);
                previousTrack = std::move(audioSlot_.remoteTrack);
                audioSlot_.transceiver = transceiver;
                audioSlot_.remoteTrack = currentTrack;
                enabled = audioSlot_.remotePlaybackEnabled;
                slot = audioSlot_.name;
            }
        }
        currentTrack->set_enabled(enabled);
        if (auto* observer = Observer()) {
            observer->OnRemoteTrackAdded(
                {track->id(), track->kind(), slot});
        }
        return;
    }

    std::vector<webrtc::scoped_refptr<
        webrtc::RtpTransceiverInterface>> negotiatedVideo;
    if (auto peer = PeerConnection()) {
        for (const auto& current : peer->GetTransceivers()) {
            if (current &&
                current->media_type() == webrtc::MediaType::VIDEO &&
                current->mid().has_value() && !current->stopped()) {
                negotiatedVideo.push_back(current);
            }
        }
    }
    std::string slot;
    {
        std::lock_guard lock(mutex_);
        for (const auto& slotName : videoSlotOrder_) {
            const auto found = videoSlots_.find(slotName);
            if (found != videoSlots_.end() &&
                found->second.transceiver == transceiver) {
                slot = slotName;
                break;
            }
        }
        // libwebrtc may create a receiving transceiver while applying the
        // remote offer instead of reusing the locally prepared sender
        // transceiver. In that case map the negotiated video m-lines by their
        // fixed video order. Unassociated local transceivers have no MID and
        // are deliberately skipped.
        if (slot.empty() && !videoSlotOrder_.empty()) {
            for (std::size_t negotiatedVideoIndex = 0;
                 negotiatedVideoIndex < negotiatedVideo.size();
                 ++negotiatedVideoIndex) {
                const auto& current =
                    negotiatedVideo[negotiatedVideoIndex];
                if (current == transceiver) {
                    if (negotiatedVideoIndex < videoSlotOrder_.size()) {
                        slot = videoSlotOrder_[negotiatedVideoIndex];
                    }
                    break;
                }
            }
        }
    }
    if (auto* observer = Observer()) {
        observer->OnRemoteTrackAdded({track->id(), track->kind(), slot});
    }
    if (track->kind() != webrtc::MediaStreamTrackInterface::kVideoKind) {
        return;
    }

    auto* videoTrack = static_cast<webrtc::VideoTrackInterface*>(track.get());
    webrtc::scoped_refptr<webrtc::VideoTrackInterface> currentTrack(
        videoTrack);
    if (!slot.empty()) {
        webrtc::scoped_refptr<webrtc::VideoTrackInterface> previousTrack;
        webrtc::VideoSinkInterface<webrtc::VideoFrame>* sink = nullptr;
        {
            std::lock_guard lock(mutex_);
            const auto found = videoSlots_.find(slot);
            if (found == videoSlots_.end()) {
                return;
            }
            previousTrack = std::move(found->second.remoteTrack);
            found->second.remoteTrack = currentTrack;
            sink = found->second.remoteSink;
        }
        if (previousTrack && sink) {
            previousTrack->RemoveSink(sink);
        }
        if (sink) {
            currentTrack->AddOrUpdateSink(sink, webrtc::VideoSinkWants());
        }
        return;
    }
    webrtc::scoped_refptr<webrtc::VideoTrackInterface> previousTrack;
    webrtc::VideoSinkInterface<webrtc::VideoFrame>* sink = nullptr;
    {
        std::lock_guard lock(mutex_);
        previousTrack = std::move(remoteVideoTrack_);
        remoteVideoTrack_ = currentTrack;
        sink = remoteVideoSink_;
    }
    if (previousTrack && sink) {
        previousTrack->RemoveSink(sink);
    }
    if (sink) {
        currentTrack->AddOrUpdateSink(sink, webrtc::VideoSinkWants());
    }
}
