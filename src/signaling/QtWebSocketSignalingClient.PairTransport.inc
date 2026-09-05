// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

    void AbortConnectionForTesting()
    {
        if (socket_.state() == QAbstractSocket::UnconnectedState) {
            return;
        }
        manualDisconnect_ = false;
        fatalDisconnect_ = false;
        socket_.abort();
    }

    SignalingOperationResult SendDescription(
        const SignalingSessionDescription& description)
    {
        if (description.sessionId.empty() || description.type.empty() ||
            description.sdp.empty()) {
            return Failure("invalid_session_description",
                           "Session ID, SDP type and SDP are required.");
        }
        if (QThread::currentThread() != socket_.thread()) {
            if (auto ready = RequireRegistered(); !ready.accepted) {
                return ready;
            }
            const bool queued = QMetaObject::invokeMethod(
                &socket_,
                [this, description] {
                    const auto result = SendDescriptionOnOwnerThread(
                        description);
                    if (!result.accepted) {
                        NotifyError(result.errorCode,
                                    result.errorMessage);
                    }
                },
                Qt::QueuedConnection);
            return queued
                       ? Success()
                       : Failure("signaling_dispatch_failed",
                                 "The SDP could not be posted to the signaling thread.");
        }
        return SendDescriptionOnOwnerThread(description);
    }

    SignalingOperationResult SendIceCandidate(
        const SignalingIceCandidate& candidate)
    {
        if (candidate.sessionId.empty() || candidate.candidate.empty() ||
            candidate.sdpMLineIndex < 0) {
            return Failure("invalid_ice_candidate",
                           "Session ID, candidate and m-line index are required.");
        }
        if (QThread::currentThread() != socket_.thread()) {
            if (auto ready = RequireRegistered(); !ready.accepted) {
                return ready;
            }
            const bool queued = QMetaObject::invokeMethod(
                &socket_,
                [this, candidate] {
                    const auto result = SendIceCandidateOnOwnerThread(
                        candidate);
                    if (!result.accepted) {
                        NotifyError(result.errorCode,
                                    result.errorMessage);
                    }
                },
                Qt::QueuedConnection);
            return queued
                       ? Success()
                       : Failure("signaling_dispatch_failed",
                                 "The ICE candidate could not be posted to the signaling thread.");
        }
        return SendIceCandidateOnOwnerThread(candidate);
    }

    SignalingOperationResult SendIceRestartRequest(
        const SignalingIceRestartRequest& request)
    {
        if (request.sessionId.empty() || request.observedGeneration == 0 ||
            request.requestSequence == 0) {
            return Failure("invalid_ice_restart_request",
                           "Session ID, negotiation generation, and request sequence are required.");
        }
        if (QThread::currentThread() != socket_.thread()) {
            if (auto ready = RequireRegistered(); !ready.accepted) {
                return ready;
            }
            const bool queued = QMetaObject::invokeMethod(
                &socket_, [this, request] {
                    const auto result = SendIceRestartRequest(request);
                    if (!result.accepted) {
                        NotifyError(result.errorCode, result.errorMessage);
                    }
                }, Qt::QueuedConnection);
            return queued ? Success()
                          : Failure("signaling_dispatch_failed",
                                    "The ICE restart request could not be posted to the signaling thread.");
        }
        if (auto ready = RequireRegistered(); !ready.accepted) {
            return ready;
        }
        QJsonObject payload;
        payload.insert(QStringLiteral("observedGeneration"),
                       static_cast<qint64>(request.observedGeneration));
        payload.insert(QStringLiteral("requestSequence"),
                       static_cast<qint64>(request.requestSequence));
        return SendEnvelope(QStringLiteral("ice_restart_request"),
                            request.sessionId, payload);
    }

    SignalingOperationResult SendIceRestartCancel(
        const SignalingIceRestartCancel& cancel)
    {
        if (cancel.sessionId.empty() || cancel.observedGeneration == 0 ||
            cancel.requestSequence == 0) {
            return Failure("invalid_ice_restart_cancel",
                           "Session ID, negotiation generation, and request sequence are required.");
        }
        if (QThread::currentThread() != socket_.thread()) {
            if (auto ready = RequireRegistered(); !ready.accepted) {
                return ready;
            }
            const bool queued = QMetaObject::invokeMethod(
                &socket_, [this, cancel] {
                    const auto result = SendIceRestartCancel(cancel);
                    if (!result.accepted) {
                        NotifyError(result.errorCode, result.errorMessage);
                    }
                }, Qt::QueuedConnection);
            return queued ? Success()
                          : Failure("signaling_dispatch_failed",
                                    "The ICE restart cancellation could not be posted to the signaling thread.");
        }
        if (auto ready = RequireRegistered(); !ready.accepted) {
            return ready;
        }
        QJsonObject payload;
        payload.insert(QStringLiteral("observedGeneration"),
                       static_cast<qint64>(cancel.observedGeneration));
        payload.insert(QStringLiteral("requestSequence"),
                       static_cast<qint64>(cancel.requestSequence));
        return SendEnvelope(QStringLiteral("ice_restart_cancel"),
                            cancel.sessionId, payload);
    }

private:
    SignalingOperationResult SendSessionEndAction(
        const QString& type,
        const std::string& sessionId,
        const std::string& reasonCode)
    {
        if (auto ready = RequireRegistered(); !ready.accepted) {
            return ready;
        }
        if (sessionId.empty()) {
            return Failure("session_id_required",
                           "A session ID is required.");
        }
        QJsonObject payload;
        if (!reasonCode.empty()) {
            payload.insert(QStringLiteral("reasonCode"),
                           ToQString(reasonCode));
        }
        return SendEnvelope(type, sessionId, payload);
    }
