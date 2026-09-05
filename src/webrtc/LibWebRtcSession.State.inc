// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

void LibWebRtcSession::UpdatePeerConnectionState(
    webrtc::PeerConnectionInterface::PeerConnectionState state)
{
    IWebRtcSessionObserver* observer = nullptr;
    WebRtcSessionState combined = WebRtcSessionState::kNew;
    bool applyPendingBitrateBootstrap = false;
    {
        std::lock_guard lock(mutex_);
        peerConnectionState_ = state;
        combined = CombinedConnectionStateLocked();
        applyPendingBitrateBootstrap =
            combined == WebRtcSessionState::kConnected;
        if (state_ != combined) {
            state_ = combined;
            observer = observer_;
        }
    }
    if (applyPendingBitrateBootstrap) {
        ApplyPendingVideoStartBitrateBootstrap();
    }
    if (observer) {
        observer->OnSessionStateChanged(combined);
    }
}

void LibWebRtcSession::UpdateIceConnectionState(
    webrtc::PeerConnectionInterface::IceConnectionState state)
{
    if (state == webrtc::PeerConnectionInterface::kIceConnectionMax) {
        return;
    }
    IWebRtcSessionObserver* observer = nullptr;
    WebRtcSessionState combined = WebRtcSessionState::kNew;
    bool applyPendingBitrateBootstrap = false;
    {
        std::lock_guard lock(mutex_);
        iceConnectionState_ = state;
        combined = CombinedConnectionStateLocked();
        applyPendingBitrateBootstrap =
            combined == WebRtcSessionState::kConnected;
        if (state_ != combined) {
            state_ = combined;
            observer = observer_;
        }
    }
    if (applyPendingBitrateBootstrap) {
        ApplyPendingVideoStartBitrateBootstrap();
    }
    if (observer) {
        observer->OnSessionStateChanged(combined);
    }
}

WebRtcSessionState LibWebRtcSession::CombinedConnectionStateLocked() const
{
    using PeerState =
        webrtc::PeerConnectionInterface::PeerConnectionState;
    using IceState = webrtc::PeerConnectionInterface::IceConnectionState;

    const bool peerClosed = peerConnectionState_ == PeerState::kClosed;
    const bool iceClosed =
        iceConnectionState_ == IceState::kIceConnectionClosed;
    if (peerClosed || iceClosed) {
        return WebRtcSessionState::kClosed;
    }
    if (state_ == WebRtcSessionState::kClosing ||
        state_ == WebRtcSessionState::kClosed) {
        return state_;
    }

    const bool peerFailed = peerConnectionState_ == PeerState::kFailed;
    const bool iceFailed =
        iceConnectionState_ == IceState::kIceConnectionFailed;
    if (peerFailed || iceFailed) {
        return WebRtcSessionState::kFailed;
    }

    const bool peerDisconnected =
        peerConnectionState_ == PeerState::kDisconnected;
    const bool iceDisconnected =
        iceConnectionState_ == IceState::kIceConnectionDisconnected;
    if (peerDisconnected || iceDisconnected) {
        return WebRtcSessionState::kDisconnected;
    }

    const bool peerConnected =
        peerConnectionState_ == PeerState::kConnected;
    const bool iceConnected =
        iceConnectionState_ == IceState::kIceConnectionConnected ||
        iceConnectionState_ == IceState::kIceConnectionCompleted;
    if (peerConnected && iceConnected) {
        return WebRtcSessionState::kConnected;
    }

    const bool peerConnecting =
        peerConnectionState_ == PeerState::kConnecting;
    const bool iceChecking =
        iceConnectionState_ == IceState::kIceConnectionChecking;
    if (peerConnecting || iceChecking || peerConnected || iceConnected) {
        return WebRtcSessionState::kConnecting;
    }

    // Native "new" callbacks can arrive while the wrapper is already Ready.
    // They carry no transport regression and must not overwrite that state.
    return state_;
}

void LibWebRtcSession::ChangeState(WebRtcSessionState state)
{
    IWebRtcSessionObserver* observer = nullptr;
    {
        std::lock_guard lock(mutex_);
        if (state_ == state) {
            return;
        }
        state_ = state;
        observer = observer_;
    }
    if (observer) {
        observer->OnSessionStateChanged(state);
    }
}

void LibWebRtcSession::CompleteOperation(OperationId operationId)
{
    if (auto* observer = Observer()) {
        observer->OnOperationCompleted(operationId);
    }
}

void LibWebRtcSession::FailOperation(OperationId operationId,
                                     std::string code,
                                     std::string message)
{
    if (auto* observer = Observer()) {
        observer->OnWebRtcError(
            operationId, {std::move(code), std::move(message)});
    }
}

IWebRtcSessionObserver* LibWebRtcSession::Observer() const
{
    std::lock_guard lock(mutex_);
    return observer_;
}
