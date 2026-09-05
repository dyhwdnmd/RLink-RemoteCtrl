// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

OperationId LibWebRtcSession::NextOperationId()
{
    return nextOperationId_.fetch_add(1, std::memory_order_relaxed);
}

webrtc::scoped_refptr<webrtc::PeerConnectionInterface>
LibWebRtcSession::PeerConnection() const
{
    std::lock_guard lock(mutex_);
    return peerConnection_;
}

void LibWebRtcSession::CreateLocalDescription(
    OperationId operationId,
    SessionDescriptionType type,
    bool iceRestart)
{
    auto peer = PeerConnection();
    if (!peer) {
        FailOperation(operationId, "session_not_started",
                      "Cannot create SDP before the session is started.");
        return;
    }
    ChangeState(WebRtcSessionState::kConnecting);

    auto callbackGate = callbackGate_;
    auto callback = webrtc::make_ref_counted<CreateDescriptionCallback>(
        [callbackGate, operationId, type](
            std::unique_ptr<webrtc::SessionDescriptionInterface>
                nativeDescription) {
            auto lease = callbackGate->Enter();
            auto* owner = lease.Owner();
            if (!owner) {
                return;
            }
            std::string sdp;
            if (!nativeDescription || !nativeDescription->ToString(&sdp)) {
                owner->FailOperation(
                    operationId, "local_sdp_serialize_failed",
                    "Failed to serialize the local SDP.");
                return;
            }
            owner->SetLocalDescription(operationId,
                                       {type, std::move(sdp)},
                                       std::move(nativeDescription));
        },
        [callbackGate, operationId](webrtc::RTCError error) {
            auto lease = callbackGate->Enter();
            if (auto* owner = lease.Owner()) {
                owner->FailOperation(operationId,
                                     "create_local_sdp_failed",
                                     error.message());
            }
        });
    if (type == SessionDescriptionType::kOffer) {
        webrtc::PeerConnectionInterface::RTCOfferAnswerOptions options;
        options.ice_restart = iceRestart;
        peer->CreateOffer(callback.get(), options);
    } else {
        peer->CreateAnswer(callback.get(), {});
    }
}

void LibWebRtcSession::SetLocalDescription(
    OperationId operationId,
    SessionDescription description,
    std::unique_ptr<webrtc::SessionDescriptionInterface> nativeDescription)
{
    auto peer = PeerConnection();
    if (!peer) {
        FailOperation(operationId, "session_closed",
                      "The session closed before local SDP was applied.");
        return;
    }

    auto callbackGate = callbackGate_;
    auto callback = webrtc::make_ref_counted<SetDescriptionCallback>(
        [callbackGate, operationId,
         description = std::move(description)] {
            auto lease = callbackGate->Enter();
            if (auto* owner = lease.Owner()) {
                if (auto* observer = owner->Observer()) {
                    observer->OnLocalDescription(description);
                }
                owner->CompleteOperation(operationId);
            }
        },
        [callbackGate, operationId](webrtc::RTCError error) {
            auto lease = callbackGate->Enter();
            if (auto* owner = lease.Owner()) {
                owner->FailOperation(operationId,
                                     "set_local_sdp_failed",
                                     error.message());
            }
        });
    peer->SetLocalDescription(callback.get(), nativeDescription.release());
}
