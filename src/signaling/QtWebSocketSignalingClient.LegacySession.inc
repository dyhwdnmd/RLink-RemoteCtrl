// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

    SignalingOperationResult RequestSession(
        const std::string& targetDeviceId,
        SessionPurpose purpose,
        const std::vector<std::string>& permissions)
    {
        if (auto ready = RequireRegistered(); !ready.accepted) {
            return ready;
        }
        if (targetDeviceId.empty() || purpose == SessionPurpose::kNone) {
            return Failure("invalid_session_request",
                           "Target device and session purpose are required.");
        }
        QJsonObject payload;
        payload.insert(QStringLiteral("targetDeviceId"),
                       ToQString(targetDeviceId));
        payload.insert(QStringLiteral("purpose"), PurposeToString(purpose));
        payload.insert(QStringLiteral("permissions"),
                       StringArray(permissions));
        return SendEnvelope(QStringLiteral("session_request"), {}, payload);
    }

    SignalingOperationResult RequestOwnedDeviceSession(
        const std::string& targetDeviceId,
        SessionPurpose purpose,
        const std::vector<std::string>& permissions)
    {
        if (auto ready = RequireRegistered(); !ready.accepted) {
            return ready;
        }
        if (targetDeviceId.empty() || purpose == SessionPurpose::kNone) {
            return Failure("invalid_session_request",
                           "Target device and session purpose are required.");
        }
        QJsonObject payload;
        payload.insert(QStringLiteral("targetDeviceId"),
                       ToQString(targetDeviceId));
        payload.insert(QStringLiteral("purpose"), PurposeToString(purpose));
        payload.insert(QStringLiteral("permissions"),
                       StringArray(permissions));
        payload.insert(QStringLiteral("ownedDeviceRequest"), true);
        return SendEnvelope(QStringLiteral("session_request"), {}, payload);
    }

    SignalingOperationResult RequestAssistedSession(
        const std::string& targetDeviceId,
        const std::string& verificationCode,
        const std::vector<std::string>& permissions)
    {
        if (auto ready = RequireRegistered(); !ready.accepted) {
            return ready;
        }
        if (!IsNineDigitPublicId(targetDeviceId) ||
            verificationCode.size() != 6 ||
            !std::all_of(verificationCode.cbegin(), verificationCode.cend(),
                         [](char value) { return value >= '0' && value <= '9'; })) {
            return Failure("invalid_assistance_credentials",
                           "A valid device ID and six-digit verification code are required.");
        }
        QJsonObject payload;
        payload.insert(QStringLiteral("targetDeviceId"),
                       ToQString(targetDeviceId));
        payload.insert(QStringLiteral("purpose"),
                       QStringLiteral("remote_control"));
        payload.insert(QStringLiteral("permissions"),
                       StringArray(permissions));
        payload.insert(QStringLiteral("assistedRequest"), true);
        payload.insert(QStringLiteral("verificationCode"),
                       ToQString(verificationCode));
        return SendEnvelope(QStringLiteral("session_request"), {}, payload);
    }

    SignalingOperationResult RespondToSession(
        const std::string& sessionId,
        bool accepted,
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
        payload.insert(QStringLiteral("accepted"), accepted);
        if (!reasonCode.empty()) {
            payload.insert(QStringLiteral("reasonCode"),
                           ToQString(reasonCode));
        }
        return SendEnvelope(QStringLiteral("session_response"),
                            sessionId, payload);
    }

    SignalingOperationResult CancelSession(
        const std::string& sessionId,
        const std::string& reasonCode)
    {
        return SendSessionEndAction(QStringLiteral("session_cancel"),
                                    sessionId, reasonCode);
    }

    SignalingOperationResult CloseSession(
        const std::string& sessionId,
        const std::string& reasonCode)
    {
        return SendSessionEndAction(QStringLiteral("session_close"),
                                    sessionId, reasonCode);
    }

    SignalingOperationResult ResumeSession(
        const std::string& sessionId,
        const std::string& recoveryToken)
    {
        if (auto ready = RequireRegistered(); !ready.accepted) {
            return ready;
        }
        if (sessionId.empty() || recoveryToken.empty()) {
            return Failure("invalid_session_resume",
                           "Session ID and recovery token are required.");
        }
        QJsonObject payload;
        payload.insert(QStringLiteral("recoveryToken"),
                       ToQString(recoveryToken));
        return SendEnvelope(QStringLiteral("session_resume"), sessionId,
                            payload);
    }
