// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

    SignalingOperationResult SendDescriptionOnOwnerThread(
        const SignalingSessionDescription& description)
    {
        if (auto ready = RequireRegistered(); !ready.accepted) {
            return ready;
        }
        QJsonObject payload;
        payload.insert(QStringLiteral("type"), ToQString(description.type));
        payload.insert(QStringLiteral("sdp"), ToQString(description.sdp));
        payload.insert(QStringLiteral("negotiationGeneration"),
                       static_cast<qint64>(description.negotiationGeneration));
        return SendEnvelope(QStringLiteral("sdp"), description.sessionId,
                            payload);
    }

    SignalingOperationResult SendIceCandidateOnOwnerThread(
        const SignalingIceCandidate& candidate)
    {
        if (auto ready = RequireRegistered(); !ready.accepted) {
            return ready;
        }
        QJsonObject payload;
        payload.insert(QStringLiteral("candidate"),
                       ToQString(candidate.candidate));
        payload.insert(QStringLiteral("sdpMid"), ToQString(candidate.sdpMid));
        payload.insert(QStringLiteral("sdpMLineIndex"),
                       candidate.sdpMLineIndex);
        payload.insert(QStringLiteral("negotiationGeneration"),
                       static_cast<qint64>(candidate.negotiationGeneration));
        return SendEnvelope(QStringLiteral("ice_candidate"),
                            candidate.sessionId, payload);
    }
