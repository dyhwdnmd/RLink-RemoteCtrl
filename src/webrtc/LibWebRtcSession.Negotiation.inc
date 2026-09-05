// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

OperationId LibWebRtcSession::CreateOffer()
{
    const OperationId operationId = NextOperationId();
    CreateLocalDescription(operationId, SessionDescriptionType::kOffer);
    return operationId;
}

OperationId LibWebRtcSession::CreateIceRestartOffer()
{
    const OperationId operationId = NextOperationId();
    CreateLocalDescription(
        operationId, SessionDescriptionType::kOffer, true);
    return operationId;
}

OperationId LibWebRtcSession::CreateAnswer()
{
    const OperationId operationId = NextOperationId();
    CreateLocalDescription(operationId, SessionDescriptionType::kAnswer);
    return operationId;
}

OperationId LibWebRtcSession::ApplyRemoteDescription(
    const SessionDescription& description)
{
    const OperationId operationId = NextOperationId();
    auto peer = PeerConnection();
    if (!peer) {
        FailOperation(operationId, "session_not_started",
                      "Cannot apply SDP before the session is started.");
        return operationId;
    }

    webrtc::SdpParseError parseError;
    auto nativeDescription = webrtc::CreateSessionDescription(
        ToNativeSdpType(description.type), description.sdp, &parseError);
    if (!nativeDescription) {
        FailOperation(operationId, "remote_sdp_parse_failed",
                      parseError.description);
        return operationId;
    }

    auto callbackGate = callbackGate_;
    auto callback = webrtc::make_ref_counted<SetDescriptionCallback>(
        [callbackGate, operationId] {
            auto lease = callbackGate->Enter();
            if (auto* owner = lease.Owner()) {
                owner->CompleteOperation(operationId);
            }
        },
        [callbackGate, operationId](webrtc::RTCError error) {
            auto lease = callbackGate->Enter();
            if (auto* owner = lease.Owner()) {
                owner->FailOperation(operationId,
                                     "set_remote_sdp_failed",
                                     error.message());
            }
        });
    peer->SetRemoteDescription(callback.get(), nativeDescription.release());
    return operationId;
}

OperationId LibWebRtcSession::AddRemoteIceCandidate(
    const IceCandidate& candidate)
{
    const OperationId operationId = NextOperationId();
    auto peer = PeerConnection();
    if (!peer) {
        FailOperation(operationId, "session_not_started",
                      "Cannot add ICE before the session is started.");
        return operationId;
    }

    webrtc::SdpParseError parseError;
    std::unique_ptr<webrtc::IceCandidate> nativeCandidate(
        webrtc::CreateIceCandidate(candidate.sdpMid,
                                   candidate.sdpMLineIndex,
                                   candidate.candidate, &parseError));
    if (!nativeCandidate) {
        FailOperation(operationId, "ice_candidate_parse_failed",
                      parseError.description);
        return operationId;
    }
    if (!peer->AddIceCandidate(nativeCandidate.get())) {
        FailOperation(operationId, "add_ice_candidate_failed",
                      "PeerConnection rejected the ICE candidate.");
        return operationId;
    }

    CompleteOperation(operationId);
    return operationId;
}
