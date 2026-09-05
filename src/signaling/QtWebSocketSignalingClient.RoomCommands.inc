// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

    SignalingOperationResult CreateRoom(std::uint32_t capacity)
    {
        if (auto ready = RequireRegistered(); !ready.accepted) {
            return ready;
        }
        if (capacity < kMinimumRoomMembers ||
            capacity > kProtocolMaximumRoomMembers) {
            return Failure("invalid_room_capacity",
                           "Room capacity must be between 2 and 5 members.");
        }
        QJsonObject payload;
        payload.insert(QStringLiteral("capacity"),
                       static_cast<int>(capacity));
        return SendEnvelope(QStringLiteral("room_create"), {}, payload);
    }

    SignalingOperationResult RequestRoomJoin(const std::string& roomId)
    {
        if (auto ready = RequireRegistered(); !ready.accepted) {
            return ready;
        }
        if (!IsNineDigitPublicId(roomId)) {
            return Failure("invalid_room_id",
                           "A 9-digit room ID is required.");
        }
        QJsonObject payload;
        payload.insert(QStringLiteral("roomId"), ToQString(roomId));
        return SendEnvelope(QStringLiteral("room_join_request"), {}, payload);
    }

    SignalingOperationResult QueryRoomAvailability(
        const std::vector<std::string>& roomIds)
    {
        if (auto ready = RequireRegistered(); !ready.accepted) {
            return ready;
        }
        if (roomIds.empty() || roomIds.size() > 8) {
            return Failure("invalid_room_availability_query",
                           "Between 1 and 8 room IDs are required.");
        }
        QJsonArray ids;
        for (const auto& roomId : roomIds) {
            if (!IsNineDigitPublicId(roomId)) {
                return Failure("invalid_room_availability_query",
                               "Every room ID must contain exactly 9 digits.");
            }
            ids.append(ToQString(roomId));
        }
        QJsonObject payload;
        payload.insert(QStringLiteral("roomIds"), ids);
        return SendEnvelope(
            QStringLiteral("room_availability_query"), {}, payload);
    }

    SignalingOperationResult RespondToRoomJoin(
        const std::string& roomId,
        const std::string& requestId,
        bool accepted,
        const std::string& reasonCode)
    {
        if (auto ready = RequireRegistered(); !ready.accepted) {
            return ready;
        }
        if (roomId.empty() || roomId.size() > 128 || requestId.empty() ||
            requestId.size() > 128 || reasonCode.size() > 64) {
            return Failure("invalid_room_join_response",
                           "Room ID and join request ID are required.");
        }
        QJsonObject payload;
        payload.insert(QStringLiteral("roomId"), ToQString(roomId));
        payload.insert(QStringLiteral("requestId"), ToQString(requestId));
        payload.insert(QStringLiteral("accepted"), accepted);
        if (!reasonCode.empty()) {
            payload.insert(QStringLiteral("reasonCode"),
                           ToQString(reasonCode));
        }
        return SendEnvelope(QStringLiteral("room_join_response"), {}, payload);
    }

    SignalingOperationResult SetRoomCapacity(const std::string& roomId,
                                             std::uint32_t capacity)
    {
        if (auto ready = RequireRegistered(); !ready.accepted) {
            return ready;
        }
        if (roomId.empty() || roomId.size() > 128 ||
            capacity < kMinimumRoomMembers ||
            capacity > kProtocolMaximumRoomMembers) {
            return Failure("invalid_room_capacity",
                           "Room ID and a capacity between 2 and 5 are required.");
        }
        QJsonObject payload;
        payload.insert(QStringLiteral("roomId"), ToQString(roomId));
        payload.insert(QStringLiteral("capacity"),
                       static_cast<int>(capacity));
        return SendEnvelope(QStringLiteral("room_set_capacity"), {}, payload);
    }

    SignalingOperationResult LeaveRoom(const std::string& roomId,
                                       const std::string& reasonCode)
    {
        if (auto ready = RequireRegistered(); !ready.accepted) {
            return ready;
        }
        if (roomId.empty() || roomId.size() > 128 || reasonCode.size() > 64) {
            return Failure("invalid_room_leave",
                           "A valid room ID is required.");
        }
        QJsonObject payload;
        payload.insert(QStringLiteral("roomId"), ToQString(roomId));
        if (!reasonCode.empty()) {
            payload.insert(QStringLiteral("reasonCode"),
                           ToQString(reasonCode));
        }
        return SendEnvelope(QStringLiteral("room_leave"), {}, payload);
    }

    SignalingOperationResult ResumeRoom(const std::string& roomId,
                                        const std::string& recoveryToken)
    {
        if (auto ready = RequireRegistered(); !ready.accepted) {
            return ready;
        }
        if (roomId.empty() || roomId.size() > 128 || recoveryToken.empty() ||
            recoveryToken.size() > 256) {
            return Failure("invalid_room_resume",
                           "Room ID and recovery token are required.");
        }
        QJsonObject payload;
        payload.insert(QStringLiteral("roomId"), ToQString(roomId));
        payload.insert(QStringLiteral("recoveryToken"),
                       ToQString(recoveryToken));
        return SendEnvelope(QStringLiteral("room_resume"), {}, payload);
    }

    SignalingOperationResult SetRoomMediaState(
        const std::string& roomId,
        bool cameraPublishing,
        bool microphonePublishing)
    {
        if (auto ready = RequireRegistered(); !ready.accepted) {
            return ready;
        }
        if (roomId.empty() || roomId.size() > 128) {
            return Failure("invalid_room_media_state",
                           "A valid room ID is required.");
        }
        QJsonObject payload;
        payload.insert(QStringLiteral("roomId"), ToQString(roomId));
        payload.insert(QStringLiteral("cameraPublishing"),
                       cameraPublishing);
        payload.insert(QStringLiteral("microphonePublishing"),
                       microphonePublishing);
        return SendEnvelope(QStringLiteral("room_media_state"), {}, payload);
    }

    SignalingOperationResult RequestRoomScreenShare(
        const std::string& roomId)
    {
        if (auto ready = RequireRegistered(); !ready.accepted) {
            return ready;
        }
        if (roomId.empty() || roomId.size() > 128) {
            return Failure("invalid_room_screen_share_request",
                           "A valid room ID is required.");
        }
        QJsonObject payload;
        payload.insert(QStringLiteral("roomId"), ToQString(roomId));
        return SendEnvelope(QStringLiteral("room_screen_share_request"),
                            {}, payload);
    }

    SignalingOperationResult ConfirmRoomScreenShare(
        const std::string& roomId,
        const std::string& grantId)
    {
        if (auto ready = RequireRegistered(); !ready.accepted) {
            return ready;
        }
        if (roomId.empty() || roomId.size() > 128 || grantId.empty() ||
            grantId.size() > 256) {
            return Failure("invalid_room_screen_share_ready",
                           "Room ID and screen-share grant are required.");
        }
        QJsonObject payload;
        payload.insert(QStringLiteral("roomId"), ToQString(roomId));
        payload.insert(QStringLiteral("grantId"), ToQString(grantId));
        return SendEnvelope(QStringLiteral("room_screen_share_ready"),
                            {}, payload);
    }

    SignalingOperationResult StopRoomScreenShare(
        const std::string& roomId,
        const std::string& grantId,
        const std::string& reasonCode)
    {
        if (auto ready = RequireRegistered(); !ready.accepted) {
            return ready;
        }
        if (roomId.empty() || roomId.size() > 128 || grantId.empty() ||
            grantId.size() > 256 || reasonCode.size() > 64) {
            return Failure("invalid_room_screen_share_stop",
                           "Room ID and screen-share grant are required.");
        }
        QJsonObject payload;
        payload.insert(QStringLiteral("roomId"), ToQString(roomId));
        payload.insert(QStringLiteral("grantId"), ToQString(grantId));
        if (!reasonCode.empty()) {
            payload.insert(QStringLiteral("reasonCode"),
                           ToQString(reasonCode));
        }
        return SendEnvelope(QStringLiteral("room_screen_share_stop"),
                            {}, payload);
    }

    SignalingOperationResult RespondToRoomScreenShareSwitch(
        const std::string& roomId,
        const std::string& requestId,
        bool accepted,
        const std::string& reasonCode)
    {
        if (auto ready = RequireRegistered(); !ready.accepted) {
            return ready;
        }
        if (roomId.empty() || roomId.size() > 128 || requestId.empty() ||
            requestId.size() > 128 || reasonCode.size() > 64) {
            return Failure("invalid_room_screen_share_switch_response",
                           "The screen-share takeover response is invalid.");
        }
        QJsonObject payload;
        payload.insert(QStringLiteral("roomId"), ToQString(roomId));
        payload.insert(QStringLiteral("requestId"), ToQString(requestId));
        payload.insert(QStringLiteral("accepted"), accepted);
        if (!reasonCode.empty()) {
            payload.insert(QStringLiteral("reasonCode"),
                           ToQString(reasonCode));
        }
        return SendEnvelope(
            QStringLiteral("room_screen_share_switch_response"), {},
            payload);
    }

    SignalingOperationResult CancelRoomScreenShareSwitch(
        const std::string& roomId,
        const std::string& requestId,
        const std::string& reasonCode)
    {
        if (auto ready = RequireRegistered(); !ready.accepted) {
            return ready;
        }
        if (roomId.empty() || roomId.size() > 128 || requestId.empty() ||
            requestId.size() > 128 || reasonCode.size() > 64) {
            return Failure("invalid_room_screen_share_switch_cancel",
                           "The screen-share takeover cancellation is invalid.");
        }
        QJsonObject payload;
        payload.insert(QStringLiteral("roomId"), ToQString(roomId));
        payload.insert(QStringLiteral("requestId"), ToQString(requestId));
        if (!reasonCode.empty()) {
            payload.insert(QStringLiteral("reasonCode"),
                           ToQString(reasonCode));
        }
        return SendEnvelope(
            QStringLiteral("room_screen_share_switch_cancel"), {},
            payload);
    }

    SignalingOperationResult RequestRoomControl(const std::string& roomId)
    {
        if (auto ready = RequireRegistered(); !ready.accepted) {
            return ready;
        }
        if (roomId.empty() || roomId.size() > 128) {
            return Failure("invalid_room_control_request",
                           "A valid room ID is required.");
        }
        QJsonObject payload;
        payload.insert(QStringLiteral("roomId"), ToQString(roomId));
        return SendEnvelope(QStringLiteral("room_control_request"), {},
                            payload);
    }

    SignalingOperationResult RespondToRoomControl(
        const std::string& roomId,
        const std::string& requestId,
        bool accepted,
        const std::string& reasonCode)
    {
        if (auto ready = RequireRegistered(); !ready.accepted) {
            return ready;
        }
        if (roomId.empty() || roomId.size() > 128 || requestId.empty() ||
            requestId.size() > 128 || reasonCode.size() > 64) {
            return Failure("invalid_room_control_response",
                           "Room ID and control request ID are required.");
        }
        QJsonObject payload;
        payload.insert(QStringLiteral("roomId"), ToQString(roomId));
        payload.insert(QStringLiteral("requestId"), ToQString(requestId));
        payload.insert(QStringLiteral("accepted"), accepted);
        if (!reasonCode.empty()) {
            payload.insert(QStringLiteral("reasonCode"),
                           ToQString(reasonCode));
        }
        return SendEnvelope(QStringLiteral("room_control_response"), {},
                            payload);
    }

    SignalingOperationResult ReleaseRoomControl(
        const std::string& roomId,
        const std::string& grantId,
        const std::string& reasonCode)
    {
        if (auto ready = RequireRegistered(); !ready.accepted) {
            return ready;
        }
        if (roomId.empty() || roomId.size() > 128 || grantId.empty() ||
            grantId.size() > 256 || reasonCode.size() > 64) {
            return Failure("invalid_room_control_release",
                           "Room ID and control grant are required.");
        }
        QJsonObject payload;
        payload.insert(QStringLiteral("roomId"), ToQString(roomId));
        payload.insert(QStringLiteral("grantId"), ToQString(grantId));
        if (!reasonCode.empty()) {
            payload.insert(QStringLiteral("reasonCode"),
                           ToQString(reasonCode));
        }
        return SendEnvelope(QStringLiteral("room_control_release"), {},
                            payload);
    }
