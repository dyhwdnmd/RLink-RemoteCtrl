// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

SessionCommandResult InProcessSessionEngine::ConnectDevice(
    const std::string& deviceId,
    SessionPurpose purpose)
{
    return ConnectDirectDevice(DirectSessionConnectRequest{
        deviceId, purpose, DirectAuthorizationMethod::kManualApproval, {}});
}

SessionCommandResult InProcessSessionEngine::ConnectDirectDevice(
    const DirectSessionConnectRequest& request)
{
    DisposeClosedSession();
    if (auto ready = RequireReady("connect device"); !ready.accepted) {
        return ready;
    }
    if (request.targetDeviceId.empty()) {
        return Failure("device_id_empty", "A device ID is required.");
    }
    if (request.purpose == SessionPurpose::kNone) {
        return Failure("session_purpose_missing",
                       "A session purpose is required.");
    }
    const bool usesVerificationCode =
        request.authorization ==
            DirectAuthorizationMethod::kVerificationCode;
    if (usesVerificationCode &&
        (request.purpose != SessionPurpose::kRemoteControl ||
         request.verificationCode.size() != 6 ||
         !std::all_of(
             request.verificationCode.cbegin(),
             request.verificationCode.cend(),
             [](char value) { return value >= '0' && value <= '9'; }))) {
        return Failure(
            "invalid_assistance_credentials",
            "A six-digit verification code is required for remote assistance.");
    }
    if (!usesVerificationCode && !request.verificationCode.empty()) {
        return Failure(
            "unexpected_verification_code",
            "This direct authorization method does not use a verification code.");
    }
    {
        std::lock_guard lock(mutex_);
        if (snapshot_.room.membership != RoomMembershipState::kNone &&
            snapshot_.room.membership != RoomMembershipState::kFailed) {
            return Failure("room_active",
                           "Leave the current room before starting a legacy direct session.");
        }
    }
    if (!signaling_) {
        return Failure("signaling_not_configured",
                       "WSS signaling is not configured.");
    }
    if (!SignalingIsOnline()) {
        return Failure("signaling_not_online",
                       "The device is not registered with signaling.");
    }

    const std::vector<std::string> permissions =
        request.purpose == SessionPurpose::kRemoteControl
            ? std::vector<std::string>{"viewScreen", "controlInput"}
            : std::vector<std::string>{"cameraMedia"};
    SignalingOperationResult result;
    SessionOrigin origin = SessionOrigin::kManualDeviceId;
    if (request.authorization == DirectAuthorizationMethod::kOwnedAccount) {
        origin = SessionOrigin::kOwnedDevice;
        result = signaling_->RequestOwnedDeviceSession(
            request.targetDeviceId, request.purpose, permissions);
    } else if (usesVerificationCode) {
        origin = SessionOrigin::kRemoteAssistance;
        result = signaling_->RequestAssistedSession(
            request.targetDeviceId, request.verificationCode, permissions);
    } else {
        result = signaling_->RequestSession(
            request.targetDeviceId, request.purpose, permissions);
    }
    if (!result.accepted) {
        return {false, result.errorCode, result.errorMessage};
    }
    {
        std::lock_guard lock(mutex_);
        snapshot_.state = SessionEngineState::kConnecting;
        snapshot_.purpose = request.purpose;
        snapshot_.origin = origin;
        snapshot_.remoteControlRole =
            request.purpose == SessionPurpose::kRemoteControl
                ? RemoteControlRole::kController
                : RemoteControlRole::kNone;
        snapshot_.peerDeviceId = request.targetDeviceId;
        snapshot_.sessionId.clear();
        snapshot_.directSignalingSessionReady = false;
        snapshot_.directSessionEverActive = false;
        snapshot_.errorCode.clear();
        snapshot_.errorMessage.clear();
        localIsOfferer_ = true;
        offerNegotiationStarted_ = false;
        sessionCloseRequested_ = false;
        sessionEndSignalSent_ = false;
        cancelWhenSessionIdKnown_ = false;
        serverSessionActive_ = false;
        signalingRecoveryPending_ = false;
        peerSignalingSuspended_ = false;
        sessionRecoveryToken_.clear();
        pendingRemoteDescription_.reset();
        pendingRemoteCandidates_.clear();
    }
    PublishSnapshot();
    return Success();
}

SessionCommandResult InProcessSessionEngine::ConnectOwnedDevice(
    const std::string& deviceId,
    SessionPurpose purpose)
{
    return ConnectDirectDevice(DirectSessionConnectRequest{
        deviceId, purpose, DirectAuthorizationMethod::kOwnedAccount, {}});
}

SessionCommandResult InProcessSessionEngine::ConnectAssistedDevice(
    const std::string& deviceId,
    const std::string& verificationCode)
{
    return ConnectDirectDevice(DirectSessionConnectRequest{
        deviceId,
        SessionPurpose::kRemoteControl,
        DirectAuthorizationMethod::kVerificationCode,
        verificationCode});
}

SessionCommandResult InProcessSessionEngine::RefreshOwnedDevices()
{
    if (!signaling_ || !SignalingIsOnline()) {
        return Failure("signaling_not_online",
                       "The device is not registered with signaling.");
    }
    const auto result = signaling_->RequestOwnedDevices();
    return {result.accepted, result.errorCode, result.errorMessage};
}

SessionCommandResult InProcessSessionEngine::AcceptIncomingSession(
    const std::string& sessionId)
{
    if (sessionId.empty()) {
        return Failure("session_id_empty", "A session ID is required.");
    }
    if (!signaling_ || !SignalingIsOnline()) {
        return Failure("signaling_not_online",
                       "The device is not registered with signaling.");
    }
    {
        std::lock_guard lock(mutex_);
        if (snapshot_.state != SessionEngineState::kAwaitingLocalApproval ||
            snapshot_.sessionId != sessionId) {
            return Failure("incoming_session_not_found",
                           "The incoming session is no longer pending.");
        }
    }
    const auto result = signaling_->RespondToSession(sessionId, true, {});
    if (!result.accepted) {
        return {false, result.errorCode, result.errorMessage};
    }
    {
        std::lock_guard lock(mutex_);
        snapshot_.state = SessionEngineState::kConnecting;
        snapshot_.errorCode.clear();
        snapshot_.errorMessage.clear();
    }
    PublishSnapshot();
    return Success();
}

SessionCommandResult InProcessSessionEngine::RejectIncomingSession(
    const std::string& sessionId)
{
    if (sessionId.empty()) {
        return Failure("session_id_empty", "A session ID is required.");
    }
    if (!signaling_ || !SignalingIsOnline()) {
        return Failure("signaling_not_online",
                       "The device is not registered with signaling.");
    }
    {
        std::lock_guard lock(mutex_);
        if (snapshot_.state != SessionEngineState::kAwaitingLocalApproval ||
            snapshot_.sessionId != sessionId) {
            return Failure("incoming_session_not_found",
                           "There is no matching incoming session to reject.");
        }
    }
    const auto result =
        signaling_->RespondToSession(sessionId, false, "rejected_by_user");
    if (!result.accepted) {
        return {false, result.errorCode, result.errorMessage};
    }
    {
        std::lock_guard lock(mutex_);
        ResetSessionStateLocked();
    }
    PublishSnapshot();
    return Success();
}

SessionCommandResult InProcessSessionEngine::Disconnect()
{
    enum class EndAction {
        kNone,
        kCancel,
        kClose,
    };
    EndAction action = EndAction::kNone;
    std::string sessionId;
    ISignalingClient* signaling = nullptr;
    bool resetImmediately = false;
    {
        std::lock_guard lock(mutex_);
        if (snapshot_.state == SessionEngineState::kReady ||
            snapshot_.state == SessionEngineState::kStopped) {
            return Success();
        }
        if (sessionEndSignalSent_) {
            return Success();
        }
        sessionCloseRequested_ = true;
        signaling = signaling_.get();
        sessionId = snapshot_.sessionId;
        if (sessionController_) {
            sessionController_->Close();
        }
        if (sessionId.empty()) {
            if (snapshot_.state == SessionEngineState::kConnecting &&
                localIsOfferer_) {
                cancelWhenSessionIdKnown_ = true;
                return Success();
            }
        } else {
            action = serverSessionActive_ ? EndAction::kClose
                                          : EndAction::kCancel;
            sessionEndSignalSent_ = true;
        }
        if (sessionController_) {
            // The controller's Closed callback returns the engine to Ready.
            // Keep the immutable session identity until then.
        } else if (snapshot_.state == SessionEngineState::kConnecting ||
                   snapshot_.state ==
                       SessionEngineState::kAwaitingLocalApproval) {
            resetImmediately = true;
        } else {
            return Failure("session_not_disconnectable",
                           "There is no live session to disconnect.");
        }
    }

    SignalingOperationResult result = {true, {}, {}};
    if (signaling && action == EndAction::kCancel) {
        result = signaling->CancelSession(sessionId,
                                          "cancelled_by_local_user");
    } else if (signaling && action == EndAction::kClose) {
        result = signaling->CloseSession(sessionId,
                                         "closed_by_local_user");
    }
    if (resetImmediately || !result.accepted) {
        std::lock_guard lock(mutex_);
        ResetSessionStateLocked();
        if (!result.accepted) {
            snapshot_.errorCode = result.errorCode;
            snapshot_.errorMessage = result.errorMessage;
        }
    }
    PublishSnapshot();
    return result.accepted
               ? Success()
               : Failure(result.errorCode, result.errorMessage);
}
