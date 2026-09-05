// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

    void DispatchMessage(const QString& type,
                         const std::string& sessionId,
                         const QJsonObject& payload)
    {
        if (type == QStringLiteral("auth_success")) {
            if (State() != SignalingConnectionState::kAuthenticating) {
                fatalDisconnect_ = true;
                Fail("unexpected_auth_success",
                     "Authentication succeeded outside the authentication state.");
                socket_.close(
                    QWebSocketProtocol::CloseCodeProtocolError,
                    QStringLiteral("unexpected auth_success"));
                return;
            }
            BeginDeviceRegistration();
            return;
        }

        if (type == QStringLiteral("auth_error")) {
            const SignalingConnectionState state = State();
            if (state != SignalingConnectionState::kAuthenticating &&
                state != SignalingConnectionState::kRegistering &&
                state != SignalingConnectionState::kRegistered) {
                fatalDisconnect_ = true;
                Fail("unexpected_auth_error",
                     "Authentication failed outside the authentication state.");
                socket_.close(
                    QWebSocketProtocol::CloseCodeProtocolError,
                    QStringLiteral("unexpected auth_error"));
                return;
            }
            authenticationDeadline_.stop();
            QString reason = payload.value(
                QStringLiteral("reason")).toString().trimmed();
            if (reason.isEmpty()) {
                reason = QStringLiteral("authentication_rejected");
            }
            QString message = payload.value(
                QStringLiteral("message")).toString().trimmed();
            if (message.isEmpty()) {
                message = QStringLiteral(
                    "The signaling server rejected authentication.");
            }
            const bool retryable = payload.value(
                QStringLiteral("retryable")).toBool(false);
            fatalDisconnect_ = !retryable;
            Fail(ToString(reason), ToString(message));
            socket_.close(
                retryable
                    ? QWebSocketProtocol::CloseCodeNormal
                    : QWebSocketProtocol::CloseCodePolicyViolated,
                retryable
                    ? QStringLiteral("authentication retry")
                    : QStringLiteral("authentication rejected"));
            return;
        }

        if (State() == SignalingConnectionState::kAuthenticating) {
            fatalDisconnect_ = true;
            Fail("authentication_protocol_violation",
                 "Only auth_success or auth_error is allowed while authentication is pending.");
            socket_.close(
                QWebSocketProtocol::CloseCodeProtocolError,
                QStringLiteral("authentication protocol violation"));
            return;
        }

        if (type == QStringLiteral("device_registered")) {
            if (State() != SignalingConnectionState::kRegistering) {
                fatalDisconnect_ = true;
                Fail("unexpected_device_registered",
                     "Device registration completed in an invalid state.");
                socket_.close(
                    QWebSocketProtocol::CloseCodeProtocolError,
                    QStringLiteral("unexpected device_registered"));
                return;
            }
            const std::string deviceId = ToString(
                payload.value(QStringLiteral("deviceId")).toString());
            if (deviceId.empty()) {
                NotifyError("invalid_device_registered",
                            "Registered device ID is missing.");
                return;
            }
            SetState(SignalingConnectionState::kRegistered);
            reconnectAttempt_ = 0;
            if (observer_) {
                observer_->OnDeviceRegistered(deviceId);
            }
            heartbeatTimer_.start(
                static_cast<int>(config_.heartbeatIntervalMs));
            return;
        }

        if (type == QStringLiteral("account_delete_result")) {
            const QJsonValue deletedValue =
                payload.value(QStringLiteral("deleted"));
            if (!deletedValue.isBool()) {
                NotifyError("invalid_account_delete_result",
                            "Account deletion result is invalid.");
                return;
            }
            SignalingAccountDeletionResult result;
            result.deleted = deletedValue.toBool();
            result.errorCode = ToString(
                payload.value(QStringLiteral("code")).toString());
            result.errorMessage = ToString(
                payload.value(QStringLiteral("message")).toString());
            result.retryable =
                payload.value(QStringLiteral("retryable")).toBool(false);
            if (observer_) {
                observer_->OnAccountDeletionResult(result);
            }
            return;
        }

        if (type == QStringLiteral("account_deleted")) {
            SignalingAccountDeletionResult result;
            result.deleted = true;
            result.errorCode = "account_deleted";
            result.errorMessage =
                "The RemoteC account was deleted.";
            if (observer_) {
                observer_->OnAccountDeletionResult(result);
            }
            return;
        }

        if (type == QStringLiteral("my_devices_result") ||
            type == QStringLiteral("my_devices_changed")) {
            const QString errorCode = payload.value(
                QStringLiteral("errorCode")).toString();
            if (!errorCode.isEmpty()) {
                NotifyError(ToString(errorCode), ToString(payload.value(
                    QStringLiteral("errorMessage")).toString()));
                return;
            }
            const double revisionValue = payload.value(
                QStringLiteral("revision")).toDouble(-1.0);
            const QJsonValue devicesValue = payload.value(
                QStringLiteral("devices"));
            if (revisionValue < 1.0 ||
                revisionValue > 9007199254740991.0 ||
                revisionValue != static_cast<double>(
                    static_cast<std::uint64_t>(revisionValue)) ||
                !devicesValue.isArray() ||
                devicesValue.toArray().size() > 100) {
                NotifyError("invalid_my_devices_snapshot",
                            "The owned-device snapshot is invalid.");
                return;
            }
            SignalingOwnedDevicesSnapshot snapshot;
            snapshot.revision = static_cast<std::uint64_t>(revisionValue);
            const QJsonArray devices = devicesValue.toArray();
            snapshot.devices.reserve(static_cast<std::size_t>(devices.size()));
            std::unordered_set<std::string> deviceIds;
            for (const QJsonValue& value : devices) {
                if (!value.isObject()) {
                    NotifyError("invalid_my_devices_snapshot",
                                "An owned-device entry is invalid.");
                    return;
                }
                const QJsonObject object = value.toObject();
                SignalingOwnedDevice device;
                device.deviceId = ToString(object.value(
                    QStringLiteral("deviceId")).toString());
                device.deviceName = ToString(object.value(
                    QStringLiteral("deviceName")).toString());
                const double createdAt = object.value(
                    QStringLiteral("createdAt")).toDouble(-1.0);
                const double lastSeenAt = object.value(
                    QStringLiteral("lastSeenAt")).toDouble(-1.0);
                if (!IsNineDigitPublicId(device.deviceId) ||
                    device.deviceName.size() > 128 ||
                    !object.value(QStringLiteral("online")).isBool() ||
                    !object.value(QStringLiteral("current")).isBool() ||
                    createdAt < 0.0 || lastSeenAt < 0.0 ||
                    !deviceIds.insert(device.deviceId).second) {
                    NotifyError("invalid_my_devices_snapshot",
                                "An owned-device entry is invalid.");
                    return;
                }
                device.online = object.value(
                    QStringLiteral("online")).toBool();
                device.current = object.value(
                    QStringLiteral("current")).toBool();
                device.createdAt = static_cast<std::int64_t>(createdAt);
                device.lastSeenAt = static_cast<std::int64_t>(lastSeenAt);
                snapshot.devices.push_back(std::move(device));
            }
            if (observer_) {
                observer_->OnOwnedDevicesChanged(snapshot);
            }
            return;
        }

        if (type == QStringLiteral("session_pending")) {
            SignalingSessionPending pending;
            pending.sessionId = sessionId;
            pending.peerDeviceId = ToString(
                payload.value(QStringLiteral("peerDeviceId")).toString());
            if (pending.sessionId.empty() ||
                pending.peerDeviceId.empty()) {
                NotifyError("invalid_session_pending",
                            "Pending-session identity fields are invalid.");
                return;
            }
            if (observer_) {
                observer_->OnSessionPending(pending);
            }
            return;
        }

        if (type == QStringLiteral("session_request")) {
            IncomingSessionRequest request;
            request.sessionId = sessionId;
            request.requesterDeviceId = ToString(
                payload.value(QStringLiteral("requesterDeviceId")).toString());
            request.purpose = PurposeFromString(
                payload.value(QStringLiteral("purpose")).toString());
            request.requestedPermissions = ReadStringArray(
                payload.value(QStringLiteral("permissions")));
            request.sameAccount = payload.value(
                QStringLiteral("sameAccount")).toBool(false);
            request.autoAccept = payload.value(
                QStringLiteral("autoAccept")).toBool(false);
            const bool assistedRequest = payload.value(
                QStringLiteral("assistedRequest")).toBool(false);
            request.authorization = assistedRequest
                ? DirectAuthorizationMethod::kVerificationCode
                : (request.sameAccount && request.autoAccept
                       ? DirectAuthorizationMethod::kOwnedAccount
                       : DirectAuthorizationMethod::kManualApproval);
            request.verificationCode = ToString(payload.value(
                QStringLiteral("verificationCode")).toString());
            if (request.sessionId.empty() ||
                request.requesterDeviceId.empty() ||
                request.purpose == SessionPurpose::kNone) {
                NotifyError("invalid_incoming_session_request",
                            "Incoming session request fields are invalid.");
                return;
            }
            if (observer_) {
                observer_->OnIncomingSessionRequest(request);
            }
            return;
        }

        if (type == QStringLiteral("session_response")) {
            if (sessionId.empty() ||
                !payload.value(QStringLiteral("accepted")).isBool()) {
                NotifyError("invalid_session_response",
                            "Session response fields are invalid.");
                return;
            }
            SignalingSessionResponse response;
            response.sessionId = sessionId;
            response.accepted =
                payload.value(QStringLiteral("accepted")).toBool(false);
            response.reasonCode = ToString(
                payload.value(QStringLiteral("reasonCode")).toString());
            response.reasonMessage = ToString(
                payload.value(QStringLiteral("reasonMessage")).toString());
            if (observer_) {
                observer_->OnSessionResponse(response);
            }
            return;
        }

        if (type == QStringLiteral("session_ready")) {
            SignalingSessionReady ready;
            ready.sessionId = sessionId;
            ready.peerDeviceId = ToString(
                payload.value(QStringLiteral("peerDeviceId")).toString());
            ready.recoveryToken = ToString(
                payload.value(QStringLiteral("recoveryToken")).toString());
            if (ready.sessionId.empty() || ready.peerDeviceId.empty() ||
                ready.recoveryToken.empty()) {
                NotifyError("invalid_session_ready",
                            "Session-ready identity fields are invalid.");
                return;
            }
            const auto servers =
                payload.value(QStringLiteral("iceServers")).toArray();
            for (const auto& serverValue : servers) {
                const auto serverObject = serverValue.toObject();
                SignalingIceServer server;
                server.urls = ReadStringArray(
                    serverObject.value(QStringLiteral("urls")));
                server.username = ToString(
                    serverObject.value(QStringLiteral("username")).toString());
                server.credential = ToString(
                    serverObject.value(QStringLiteral("credential")).toString());
                if (!server.urls.empty()) {
                    ready.iceServers.push_back(std::move(server));
                }
            }
            if (observer_) {
                observer_->OnSessionReady(ready);
            }
            return;
        }

        if (type == QStringLiteral("session_suspended")) {
            SignalingSessionSuspended suspended;
            suspended.sessionId = sessionId;
            suspended.peerDeviceId = ToString(
                payload.value(QStringLiteral("peerDeviceId")).toString());
            const int recoveryWindowMs =
                payload.value(QStringLiteral("recoveryWindowMs")).toInt(-1);
            if (suspended.sessionId.empty() ||
                suspended.peerDeviceId.empty() || recoveryWindowMs < 0) {
                NotifyError("invalid_session_suspended",
                            "Session-suspended fields are invalid.");
                return;
            }
            suspended.recoveryWindowMs =
                static_cast<std::uint32_t>(recoveryWindowMs);
            if (observer_) {
                observer_->OnSessionSuspended(suspended);
            }
            return;
        }

        if (type == QStringLiteral("session_resumed")) {
            SignalingSessionResumed resumed;
            resumed.sessionId = sessionId;
            resumed.peerDeviceId = ToString(
                payload.value(QStringLiteral("peerDeviceId")).toString());
            resumed.resumedDeviceId = ToString(
                payload.value(QStringLiteral("resumedDeviceId")).toString());
            if (resumed.sessionId.empty() || resumed.peerDeviceId.empty() ||
                resumed.resumedDeviceId.empty()) {
                NotifyError("invalid_session_resumed",
                            "Session-resumed fields are invalid.");
                return;
            }
            if (observer_) {
                observer_->OnSessionResumed(resumed);
            }
            return;
        }

        if (type == QStringLiteral("session_ended")) {
            SignalingSessionEnded ended;
            ended.sessionId = sessionId;
            ended.initiatorDeviceId = ToString(
                payload.value(QStringLiteral("initiatorDeviceId")).toString());
            ended.reasonCode = ToString(
                payload.value(QStringLiteral("reasonCode")).toString());
            const QString disposition =
                payload.value(QStringLiteral("disposition")).toString();
            ended.kind = disposition == QStringLiteral("cancelled")
                             ? SignalingSessionEndKind::kCancelled
                             : SignalingSessionEndKind::kClosed;
            if (ended.sessionId.empty() ||
                ended.initiatorDeviceId.empty() ||
                (disposition != QStringLiteral("cancelled") &&
                 disposition != QStringLiteral("closed"))) {
                NotifyError("invalid_session_ended",
                            "Session-ended fields are invalid.");
                return;
            }
            if (observer_) {
                observer_->OnSessionEnded(ended);
            }
            return;
        }

        if (type == QStringLiteral("room_ready")) {
            SignalingRoomReady ready;
            ready.recoveryToken = ToString(
                payload.value(QStringLiteral("recoveryToken")).toString());
            if (ready.recoveryToken.empty() ||
                !ReadRoomSnapshot(payload.value(QStringLiteral("room")),
                                  &ready.room)) {
                NotifyError("invalid_room_ready",
                            "Room-ready fields are invalid.");
                return;
            }
            if (observer_) {
                observer_->OnRoomReady(ready);
            }
            return;
        }

        if (type == QStringLiteral("room_state")) {
            RoomSnapshot room;
            if (!ReadRoomSnapshot(payload.value(QStringLiteral("room")),
                                  &room)) {
                NotifyError("invalid_room_state",
                            "Room-state fields are invalid.");
                return;
            }
            if (observer_) {
                observer_->OnRoomState(room);
            }
            return;
        }

        if (type == QStringLiteral("room_availability_result")) {
            const QJsonValue roomsValue =
                payload.value(QStringLiteral("rooms"));
            if (!roomsValue.isArray()) {
                NotifyError("invalid_room_availability_result",
                            "Room availability entries are missing.");
                return;
            }
            SignalingRoomAvailabilityResult result;
            const QJsonArray rooms = roomsValue.toArray();
            result.rooms.reserve(static_cast<std::size_t>(rooms.size()));
            for (const QJsonValue& value : rooms) {
                if (!value.isObject()) {
                    NotifyError("invalid_room_availability_result",
                                "A room availability entry is invalid.");
                    return;
                }
                const QJsonObject entry = value.toObject();
                SignalingRoomAvailability availability;
                availability.roomId = ToString(
                    entry.value(QStringLiteral("roomId")).toString());
                if (availability.roomId.empty() ||
                    !entry.value(QStringLiteral("exists")).isBool() ||
                    !entry.value(QStringLiteral("joinable")).isBool()) {
                    NotifyError("invalid_room_availability_result",
                                "Room availability fields are invalid.");
                    return;
                }
                availability.exists = entry.value(
                    QStringLiteral("exists")).toBool();
                availability.joinable = entry.value(
                    QStringLiteral("joinable")).toBool();
                result.rooms.push_back(std::move(availability));
            }
            if (observer_) {
                observer_->OnRoomAvailabilityResult(result);
            }
            return;
        }

        if (type == QStringLiteral("room_join_pending")) {
            SignalingRoomJoinPending pending;
            pending.roomId = ToString(
                payload.value(QStringLiteral("roomId")).toString());
            pending.requestId = ToString(
                payload.value(QStringLiteral("requestId")).toString());
            if (pending.roomId.empty() || pending.requestId.empty()) {
                NotifyError("invalid_room_join_pending",
                            "Room join-pending fields are invalid.");
                return;
            }
            if (observer_) {
                observer_->OnRoomJoinPending(pending);
            }
            return;
        }

        if (type == QStringLiteral("room_join_requested")) {
            RoomJoinRequest request;
            request.roomId = ToString(
                payload.value(QStringLiteral("roomId")).toString());
            request.requestId = ToString(
                payload.value(QStringLiteral("requestId")).toString());
            request.requesterDeviceId = ToString(
                payload.value(QStringLiteral("requesterDeviceId")).toString());
            request.requesterDeviceName = ToString(
                payload.value(QStringLiteral("requesterDeviceName")).toString());
            if (request.roomId.empty() || request.requestId.empty() ||
                request.requesterDeviceId.empty()) {
                NotifyError("invalid_room_join_request",
                            "Incoming room join-request fields are invalid.");
                return;
            }
            if (observer_) {
                observer_->OnRoomJoinRequested(request);
            }
            return;
        }

        if (type == QStringLiteral("room_join_result")) {
            SignalingRoomJoinResult result;
            result.roomId = ToString(
                payload.value(QStringLiteral("roomId")).toString());
            result.requestId = ToString(
                payload.value(QStringLiteral("requestId")).toString());
            if (!payload.value(QStringLiteral("accepted")).isBool()) {
                NotifyError("invalid_room_join_result",
                            "Room join-result acceptance is missing.");
                return;
            }
            result.accepted =
                payload.value(QStringLiteral("accepted")).toBool();
            result.reasonCode = ToString(
                payload.value(QStringLiteral("reasonCode")).toString());
            result.reasonMessage = ToString(
                payload.value(QStringLiteral("reasonMessage")).toString());
            if (result.roomId.empty() ||
                (result.accepted && result.requestId.empty())) {
                NotifyError("invalid_room_join_result",
                            "Room join-result identity fields are invalid.");
                return;
            }
            if (observer_) {
                observer_->OnRoomJoinResult(result);
            }
            return;
        }

        if (type == QStringLiteral("room_closed")) {
            SignalingRoomClosed closed;
            closed.roomId = ToString(
                payload.value(QStringLiteral("roomId")).toString());
            closed.initiatorDeviceId = ToString(
                payload.value(QStringLiteral("initiatorDeviceId")).toString());
            closed.reasonCode = ToString(
                payload.value(QStringLiteral("reasonCode")).toString());
            if (closed.roomId.empty() || closed.initiatorDeviceId.empty() ||
                closed.reasonCode.empty()) {
                NotifyError("invalid_room_closed",
                            "Room-closed fields are invalid.");
                return;
            }
            if (observer_) {
                observer_->OnRoomClosed(closed);
            }
            return;
        }

        if (type == QStringLiteral("room_screen_share_granted")) {
            SignalingRoomScreenShareGranted granted;
            granted.roomId = ToString(
                payload.value(QStringLiteral("roomId")).toString());
            granted.grantId = ToString(
                payload.value(QStringLiteral("grantId")).toString());
            const double epoch =
                payload.value(QStringLiteral("epoch")).toDouble(-1.0);
            if (granted.roomId.empty() || granted.grantId.empty() ||
                epoch < 0.0 || epoch > 9007199254740991.0) {
                NotifyError("invalid_room_screen_share_granted",
                            "Room screen-share grant fields are invalid.");
                return;
            }
            granted.epoch = static_cast<std::uint64_t>(epoch);
            if (observer_) {
                observer_->OnRoomScreenShareGranted(granted);
            }
            return;
        }

        if (type == QStringLiteral("room_screen_share_switch_pending")) {
            SignalingRoomScreenShareSwitchPending pending;
            pending.roomId = ToString(
                payload.value(QStringLiteral("roomId")).toString());
            pending.requestId = ToString(
                payload.value(QStringLiteral("requestId")).toString());
            pending.screenSharerDeviceId = ToString(
                payload.value(QStringLiteral("screenSharerDeviceId"))
                    .toString());
            if (pending.roomId.empty() || pending.requestId.empty() ||
                pending.screenSharerDeviceId.empty()) {
                NotifyError("invalid_room_screen_share_switch_pending",
                            "Screen-share takeover pending fields are invalid.");
                return;
            }
            if (observer_) {
                observer_->OnRoomScreenShareSwitchPending(pending);
            }
            return;
        }

        if (type == QStringLiteral("room_screen_share_switch_requested")) {
            RoomScreenShareSwitchRequest request;
            request.roomId = ToString(
                payload.value(QStringLiteral("roomId")).toString());
            request.requestId = ToString(
                payload.value(QStringLiteral("requestId")).toString());
            request.requesterDeviceId = ToString(
                payload.value(QStringLiteral("requesterDeviceId")).toString());
            request.requesterDeviceName = ToString(
                payload.value(QStringLiteral("requesterDeviceName")).toString());
            if (request.roomId.empty() || request.requestId.empty() ||
                request.requesterDeviceId.empty()) {
                NotifyError("invalid_room_screen_share_switch_requested",
                            "Screen-share takeover request fields are invalid.");
                return;
            }
            if (observer_) {
                observer_->OnRoomScreenShareSwitchRequested(request);
            }
            return;
        }

        if (type == QStringLiteral("room_screen_share_switch_result")) {
            SignalingRoomScreenShareSwitchResult result;
            result.roomId = ToString(
                payload.value(QStringLiteral("roomId")).toString());
            result.requestId = ToString(
                payload.value(QStringLiteral("requestId")).toString());
            const QJsonValue accepted =
                payload.value(QStringLiteral("accepted"));
            result.reasonCode = ToString(
                payload.value(QStringLiteral("reasonCode")).toString());
            result.reasonMessage = ToString(
                payload.value(QStringLiteral("reasonMessage")).toString());
            if (result.roomId.empty() || result.requestId.empty() ||
                !accepted.isBool()) {
                NotifyError("invalid_room_screen_share_switch_result",
                            "Screen-share takeover result fields are invalid.");
                return;
            }
            result.accepted = accepted.toBool();
            if (observer_) {
                observer_->OnRoomScreenShareSwitchResult(result);
            }
            return;
        }

        if (type == QStringLiteral("room_control_requested")) {
            RoomControlRequest request;
            request.roomId = ToString(
                payload.value(QStringLiteral("roomId")).toString());
            request.requestId = ToString(
                payload.value(QStringLiteral("requestId")).toString());
            request.requesterDeviceId = ToString(
                payload.value(QStringLiteral("requesterDeviceId")).toString());
            request.requesterDeviceName = ToString(
                payload.value(QStringLiteral("requesterDeviceName")).toString());
            if (request.roomId.empty() || request.requestId.empty() ||
                request.requesterDeviceId.empty()) {
                NotifyError("invalid_room_control_requested",
                            "Room control-request fields are invalid.");
                return;
            }
            if (observer_) {
                observer_->OnRoomControlRequested(request);
            }
            return;
        }

        if (type == QStringLiteral("room_control_result")) {
            SignalingRoomControlResult result;
            result.roomId = ToString(
                payload.value(QStringLiteral("roomId")).toString());
            result.requestId = ToString(
                payload.value(QStringLiteral("requestId")).toString());
            const QJsonValue accepted =
                payload.value(QStringLiteral("accepted"));
            result.reasonCode = ToString(
                payload.value(QStringLiteral("reasonCode")).toString());
            result.reasonMessage = ToString(
                payload.value(QStringLiteral("reasonMessage")).toString());
            if (result.roomId.empty() || result.requestId.empty() ||
                !accepted.isBool()) {
                NotifyError("invalid_room_control_result",
                            "Room control-result fields are invalid.");
                return;
            }
            result.accepted = accepted.toBool();
            if (observer_) {
                observer_->OnRoomControlResult(result);
            }
            return;
        }

        if (type == QStringLiteral("room_control_granted")) {
            SignalingRoomControlGranted granted;
            granted.roomId = ToString(
                payload.value(QStringLiteral("roomId")).toString());
            granted.grantId = ToString(
                payload.value(QStringLiteral("grantId")).toString());
            granted.screenSharerDeviceId = ToString(
                payload.value(QStringLiteral("screenSharerDeviceId")).toString());
            granted.controllerDeviceId = ToString(
                payload.value(QStringLiteral("controllerDeviceId")).toString());
            if (granted.roomId.empty() || granted.grantId.empty() ||
                granted.screenSharerDeviceId.empty() ||
                granted.controllerDeviceId.empty()) {
                NotifyError("invalid_room_control_granted",
                            "Room control-grant fields are invalid.");
                return;
            }
            if (observer_) {
                observer_->OnRoomControlGranted(granted);
            }
            return;
        }

        if (type == QStringLiteral("room_control_revoked")) {
            SignalingRoomControlRevoked revoked;
            revoked.roomId = ToString(
                payload.value(QStringLiteral("roomId")).toString());
            revoked.initiatorDeviceId = ToString(
                payload.value(QStringLiteral("initiatorDeviceId")).toString());
            revoked.reasonCode = ToString(
                payload.value(QStringLiteral("reasonCode")).toString());
            if (revoked.roomId.empty() ||
                revoked.initiatorDeviceId.empty() ||
                revoked.reasonCode.empty()) {
                NotifyError("invalid_room_control_revoked",
                            "Room control-revoked fields are invalid.");
                return;
            }
            if (observer_) {
                observer_->OnRoomControlRevoked(revoked);
            }
            return;
        }

        if (type == QStringLiteral("room_pair_ready")) {
            SignalingRoomPairReady ready;
            ready.pairId = sessionId;
            ready.roomId = ToString(
                payload.value(QStringLiteral("roomId")).toString());
            ready.peerDeviceId = ToString(
                payload.value(QStringLiteral("peerDeviceId")).toString());
            const QJsonValue offererValue =
                payload.value(QStringLiteral("localIsOfferer"));
            if (ready.pairId.empty() || ready.roomId.empty() ||
                ready.peerDeviceId.empty() || !offererValue.isBool()) {
                NotifyError("invalid_room_pair_ready",
                            "Room pair-ready fields are invalid.");
                return;
            }
            ready.localIsOfferer = offererValue.toBool();
            const auto servers =
                payload.value(QStringLiteral("iceServers")).toArray();
            for (const auto& serverValue : servers) {
                const auto serverObject = serverValue.toObject();
                SignalingIceServer server;
                server.urls = ReadStringArray(
                    serverObject.value(QStringLiteral("urls")));
                server.username = ToString(
                    serverObject.value(QStringLiteral("username")).toString());
                server.credential = ToString(
                    serverObject.value(QStringLiteral("credential")).toString());
                if (!server.urls.empty()) {
                    ready.iceServers.push_back(std::move(server));
                }
            }
            if (observer_) {
                observer_->OnRoomPairReady(ready);
            }
            return;
        }

        if (type == QStringLiteral("room_pair_closed")) {
            SignalingRoomPairClosed closed;
            closed.pairId = sessionId;
            closed.roomId = ToString(
                payload.value(QStringLiteral("roomId")).toString());
            closed.peerDeviceId = ToString(
                payload.value(QStringLiteral("peerDeviceId")).toString());
            closed.initiatorDeviceId = ToString(
                payload.value(QStringLiteral("initiatorDeviceId")).toString());
            closed.reasonCode = ToString(
                payload.value(QStringLiteral("reasonCode")).toString());
            if (closed.pairId.empty() || closed.roomId.empty() ||
                closed.peerDeviceId.empty() ||
                closed.initiatorDeviceId.empty() ||
                closed.reasonCode.empty()) {
                NotifyError("invalid_room_pair_closed",
                            "Room pair-closed fields are invalid.");
                return;
            }
            if (observer_) {
                observer_->OnRoomPairClosed(closed);
            }
            return;
        }

        if (type == QStringLiteral("sdp")) {
            SignalingSessionDescription description;
            description.sessionId = sessionId;
            description.type = ToString(
                payload.value(QStringLiteral("type")).toString());
            description.sdp = ToString(
                payload.value(QStringLiteral("sdp")).toString());
            description.negotiationGeneration = static_cast<std::uint64_t>(
                payload.value(QStringLiteral("negotiationGeneration"))
                    .toVariant().toULongLong());
            if (description.sessionId.empty() || description.sdp.empty() ||
                description.negotiationGeneration == 0 ||
                (description.type != "offer" &&
                 description.type != "answer")) {
                NotifyError("invalid_remote_description",
                            "Remote SDP fields are invalid.");
                return;
            }
            if (observer_) {
                observer_->OnRemoteDescription(description);
            }
            return;
        }

        if (type == QStringLiteral("ice_candidate")) {
            SignalingIceCandidate candidate;
            candidate.sessionId = sessionId;
            candidate.candidate = ToString(
                payload.value(QStringLiteral("candidate")).toString());
            candidate.sdpMid = ToString(
                payload.value(QStringLiteral("sdpMid")).toString());
            candidate.sdpMLineIndex = payload
                .value(QStringLiteral("sdpMLineIndex"))
                .toInt(-1);
            candidate.negotiationGeneration = static_cast<std::uint64_t>(
                payload.value(QStringLiteral("negotiationGeneration"))
                    .toVariant().toULongLong());
            if (candidate.sessionId.empty() ||
                candidate.candidate.empty() ||
                candidate.sdpMLineIndex < 0 ||
                candidate.negotiationGeneration == 0) {
                NotifyError("invalid_remote_ice_candidate",
                            "Remote ICE candidate fields are invalid.");
                return;
            }
            if (observer_) {
                observer_->OnRemoteIceCandidate(candidate);
            }
            return;
        }

        if (type == QStringLiteral("ice_restart_request")) {
            SignalingIceRestartRequest request;
            request.sessionId = sessionId;
            request.observedGeneration = static_cast<std::uint64_t>(
                payload.value(QStringLiteral("observedGeneration"))
                    .toVariant().toULongLong());
            request.requestSequence = static_cast<std::uint64_t>(
                payload.value(QStringLiteral("requestSequence"))
                    .toVariant().toULongLong());
            if (request.sessionId.empty() || request.observedGeneration == 0 ||
                request.requestSequence == 0) {
                NotifyError("invalid_ice_restart_request",
                            "ICE restart request fields are invalid.");
                return;
            }
            if (observer_) {
                observer_->OnIceRestartRequested(request);
            }
            return;
        }

        if (type == QStringLiteral("ice_restart_cancel")) {
            SignalingIceRestartCancel cancel;
            cancel.sessionId = sessionId;
            cancel.observedGeneration = static_cast<std::uint64_t>(
                payload.value(QStringLiteral("observedGeneration"))
                    .toVariant().toULongLong());
            cancel.requestSequence = static_cast<std::uint64_t>(
                payload.value(QStringLiteral("requestSequence"))
                    .toVariant().toULongLong());
            if (cancel.sessionId.empty() || cancel.observedGeneration == 0 ||
                cancel.requestSequence == 0) {
                NotifyError("invalid_ice_restart_cancel",
                            "ICE restart cancellation fields are invalid.");
                return;
            }
            if (observer_) {
                observer_->OnIceRestartCancelled(cancel);
            }
            return;
        }

        if (type == QStringLiteral("error")) {
            NotifyError(
                ToString(payload.value(QStringLiteral("code")).toString()),
                ToString(payload.value(QStringLiteral("message")).toString()));
            return;
        }

        if (type == QStringLiteral("heartbeat_ack")) {
            if (heartbeatOutstanding_) {
                heartbeatOutstanding_ = false;
                heartbeatDeadline_.stop();
                const auto elapsed = heartbeatRoundTrip_.elapsed();
                if (observer_) {
                    observer_->OnHeartbeatAcknowledged(
                        static_cast<std::uint32_t>(
                            elapsed < 0 ? 0 : elapsed));
                }
            }
            return;
        }

        NotifyError("unknown_signaling_message_type",
                    "An unknown signaling message type was received.");
    }
