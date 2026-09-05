// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

    void HandleRegister(ClientState* client, const QJsonObject& payload)
    {
        if (client->registered) {
            SendError(client->socket, {}, QStringLiteral("already_registered"),
                      QStringLiteral("This connection is already registered."));
            return;
        }
        const QString requestedDeviceId =
            payload.value(QStringLiteral("deviceId")).toString();
        const QString deviceName =
            payload.value(QStringLiteral("deviceName")).toString();
        const QString appVersion =
            payload.value(QStringLiteral("appVersion")).toString();
        const bool usesLogto =
            config_.authenticationMode ==
            SignalServerAuthenticationMode::kLogtoUserInfo;
        if ((!usesLogto && !requestedDeviceId.isEmpty() &&
             requestedDeviceId != client->claims.deviceId) ||
            (usesLogto &&
             !AccessTokenService::IsValidDeviceId(requestedDeviceId)) ||
            deviceName.isEmpty() || deviceName.size() > 128 ||
            appVersion.size() > 64 ||
            !payload.value(QStringLiteral("capabilities")).isObject()) {
            SendError(client->socket, {}, QStringLiteral("invalid_registration"),
                      QStringLiteral("Device registration fields are invalid."));
            return;
        }
        if (usesLogto) {
            const auto registration = identityStore_.RegisterDevice(
                client->trustedUserId, requestedDeviceId, deviceName);
            using remote::server_persistence::DeviceRegistrationStatus;
            if (registration.status == DeviceRegistrationStatus::kRevoked) {
                SendError(client->socket, {},
                          QStringLiteral("device_revoked"),
                          QStringLiteral("This device has been revoked."));
                return;
            }
            if (!registration.registered()) {
                SendError(client->socket, {},
                          QStringLiteral("identity_store_unavailable"),
                          QStringLiteral(
                              "The device identity could not be stored."));
                return;
            }
            // The client-provided ID identifies the local installation only.
            // Room and session protocols use the server-assigned public code.
            client->claims.deviceId = registration.publicDeviceId;
        }
        if (QWebSocket* previousSocket =
                devices_.value(client->claims.deviceId, nullptr)) {
            // A reconnect can arrive before the server-side idle sweep notices
            // the old half-open TCP connection. The same authenticated device
            // takes over its registration; the active session still requires
            // its separate session or room recovery token before it is
            // resumed.
            MarkDeviceDisconnected(client->claims.deviceId);
            if (ClientState* previousClient = FindClient(previousSocket)) {
                previousClient->registered = false;
            }
            devices_.remove(client->claims.deviceId);
            previousSocket->abort();
        }

        client->registered = true;
        client->deviceName = deviceName;
        devices_.insert(client->claims.deviceId, client->socket);
        QJsonObject response;
        response.insert(QStringLiteral("deviceId"), client->claims.deviceId);
        SendEnvelope(client->socket, QStringLiteral("device_registered"), {},
                     response);
        BroadcastOwnedDevices(client->trustedUserId);
    }

    void HandleRoomCreate(ClientState* creator,
                          const QJsonObject& payload)
    {
        const QJsonValue capacityValue =
            payload.value(QStringLiteral("capacity"));
        const int capacity = capacityValue.toInt(-1);
        if (!capacityValue.isDouble() ||
            capacityValue.toDouble(-1.0) !=
                static_cast<double>(capacity) ||
            capacity < 2 ||
            capacity > config_.maximumRoomMembers) {
            SendError(creator->socket, {},
                      QStringLiteral("invalid_room_capacity"),
                      QStringLiteral("Room capacity must be between 2 and the configured server limit."));
            return;
        }

        const QString deviceId = creator->claims.deviceId;
        ReleaseDisconnectedRoomForFreshStart(deviceId);
        if (pendingRoomJoinRequestByDevice_.contains(deviceId)) {
            SendError(creator->socket, {},
                      QStringLiteral("room_join_already_pending"),
                      QStringLiteral("The device already has a pending room join request."));
            return;
        }
        if (DeviceHasRoom(deviceId) || DeviceHasSession(deviceId)) {
            SendError(creator->socket, {}, QStringLiteral("device_busy"),
                      QStringLiteral("The device already belongs to a room or legacy session."));
            return;
        }
        if (rooms_.size() >= kMaximumRooms) {
            SendError(creator->socket, {},
                      QStringLiteral("room_limit_reached"),
                      QStringLiteral("The signaling server cannot create another room."));
            return;
        }

        RoomState room;
        for (int attempt = 0; attempt < 128; ++attempt) {
            const QString candidate = GenerateNineDigitPublicId();
            if (!rooms_.contains(candidate)) {
                room.roomId = candidate;
                break;
            }
        }
        if (room.roomId.isEmpty()) {
            SendError(creator->socket, {},
                      QStringLiteral("room_id_allocation_failed"),
                      QStringLiteral(
                          "The signaling server could not allocate a room ID."));
            return;
        }
        room.ownerDeviceId = deviceId;
        room.capacity = capacity;

        RoomMemberState owner;
        owner.deviceId = deviceId;
        owner.deviceName = creator->deviceName;
        owner.recoveryToken = CreateRecoveryToken();
        const QString recoveryToken = owner.recoveryToken;
        room.members.insert(deviceId, std::move(owner));

        const QString roomId = room.roomId;
        rooms_.insert(roomId, std::move(room));
        deviceRooms_.insert(deviceId, roomId);

        const RoomState& createdRoom = rooms_[roomId];
        SendRoomReady(creator->socket, createdRoom, recoveryToken);
        BroadcastRoomState(createdRoom);
    }

    void HandleRoomAvailabilityQuery(ClientState* client,
                                     const QJsonObject& payload)
    {
        const QJsonValue idsValue = payload.value(
            QStringLiteral("roomIds"));
        if (!idsValue.isArray()) {
            SendError(client->socket, {},
                      QStringLiteral("invalid_room_availability_query"),
                      QStringLiteral("Room IDs must be an array."));
            return;
        }
        const QJsonArray ids = idsValue.toArray();
        if (ids.isEmpty() || ids.size() > 8) {
            SendError(client->socket, {},
                      QStringLiteral("invalid_room_availability_query"),
                      QStringLiteral("Between 1 and 8 room IDs are required."));
            return;
        }
        if (!AllowRoomAvailabilityQuery(
                client, ids.size(), QDateTime::currentMSecsSinceEpoch())) {
            SendError(client->socket, {}, QStringLiteral("rate_limited"),
                      QStringLiteral(
                          "Too many room lookups. Wait before trying again."));
            return;
        }

        QJsonArray statuses;
        for (const QJsonValue& value : ids) {
            const QString roomId = value.toString();
            if (!value.isString() || !IsNineDigitPublicId(roomId)) {
                SendError(client->socket, {},
                          QStringLiteral("invalid_room_availability_query"),
                          QStringLiteral("Every room ID must be valid."));
                return;
            }
            const auto roomIt = rooms_.constFind(roomId);
            bool joinable = false;
            if (roomIt != rooms_.cend() &&
                roomIt->members.size() < roomIt->capacity) {
                const auto owner = roomIt->members.constFind(
                    roomIt->ownerDeviceId);
                joinable = owner != roomIt->members.cend() &&
                           owner->disconnectedAtMs == 0 &&
                           devices_.value(roomIt->ownerDeviceId, nullptr);
            }
            QJsonObject status;
            status.insert(QStringLiteral("roomId"), roomId);
            // Deliberately do not distinguish a missing room from an existing
            // room that cannot currently accept a join request.
            status.insert(QStringLiteral("exists"), joinable);
            status.insert(QStringLiteral("joinable"), joinable);
            statuses.append(status);
        }
        QJsonObject response;
        response.insert(QStringLiteral("rooms"), statuses);
        SendEnvelope(client->socket,
                     QStringLiteral("room_availability_result"), {},
                     response);
    }

    void HandleRoomJoinRequest(ClientState* requester,
                               const QJsonObject& payload)
    {
        const QString roomId =
            payload.value(QStringLiteral("roomId")).toString();
        if (!IsNineDigitPublicId(roomId)) {
            SendError(requester->socket, {},
                      QStringLiteral("invalid_room_join_request"),
                      QStringLiteral("A valid room ID is required."));
            return;
        }
        if (!AllowRoomJoinAttempt(
                requester, QDateTime::currentMSecsSinceEpoch())) {
            SendError(requester->socket, {}, QStringLiteral("rate_limited"),
                      QStringLiteral(
                          "Too many room join attempts. Wait before trying again."));
            return;
        }

        const QString requesterDeviceId = requester->claims.deviceId;
        ReleaseDisconnectedRoomForFreshStart(requesterDeviceId);
        const QString existingRequestId =
            pendingRoomJoinRequestByDevice_.value(requesterDeviceId);
        if (!existingRequestId.isEmpty()) {
            const auto existingIt = roomJoinRequests_.constFind(
                existingRequestId);
            if (existingIt != roomJoinRequests_.cend() &&
                existingIt->roomId == roomId) {
                SendRoomJoinPending(requester->socket, roomId,
                                    existingRequestId);
                return;
            }
            if (existingIt == roomJoinRequests_.cend()) {
                pendingRoomJoinRequestByDevice_.remove(requesterDeviceId);
            } else {
                SendError(requester->socket, {},
                          QStringLiteral("room_join_already_pending"),
                          QStringLiteral("The device already has a pending room join request."));
                return;
            }
        }

        const QString requestId =
            QUuid::createUuid().toString(QUuid::WithoutBraces);
        if (DeviceHasRoom(requesterDeviceId) ||
            DeviceHasSession(requesterDeviceId)) {
            SendRoomJoinResult(requester->socket, roomId, requestId, false,
                               QStringLiteral("device_busy"),
                               QStringLiteral("The device already belongs to a room or legacy session."));
            return;
        }

        const auto sendUnavailable = [&] {
            SendRoomJoinResult(
                requester->socket, roomId, requestId, false,
                QStringLiteral("room_unavailable"),
                QStringLiteral(
                    "The room is unavailable or cannot accept join requests."));
        };
        auto roomIt = rooms_.find(roomId);
        if (roomIt == rooms_.end()) {
            sendUnavailable();
            return;
        }
        if (roomIt->members.size() >= roomIt->capacity) {
            sendUnavailable();
            return;
        }

        const auto ownerMemberIt =
            roomIt->members.constFind(roomIt->ownerDeviceId);
        QWebSocket* ownerSocket =
            devices_.value(roomIt->ownerDeviceId, nullptr);
        if (ownerMemberIt == roomIt->members.cend() ||
            ownerMemberIt->disconnectedAtMs != 0 || !ownerSocket) {
            sendUnavailable();
            return;
        }

        RoomJoinRequestState request;
        request.requestId = requestId;
        request.roomId = roomId;
        request.requesterDeviceId = requesterDeviceId;
        request.requesterDeviceName = requester->deviceName;
        request.expiresAtMs = QDateTime::currentMSecsSinceEpoch() +
                              kRoomJoinRequestTimeoutMs;
        roomJoinRequests_.insert(requestId, request);
        pendingRoomJoinRequestByDevice_.insert(requesterDeviceId,
                                                requestId);

        SendRoomJoinPending(requester->socket, roomId, requestId);
        QJsonObject forwarded;
        forwarded.insert(QStringLiteral("roomId"), roomId);
        forwarded.insert(QStringLiteral("requestId"), requestId);
        forwarded.insert(QStringLiteral("requesterDeviceId"),
                         requesterDeviceId);
        forwarded.insert(QStringLiteral("requesterDeviceName"),
                         requester->deviceName);
        SendEnvelope(ownerSocket, QStringLiteral("room_join_requested"), {},
                     forwarded);
    }

    void HandleRoomJoinResponse(ClientState* owner,
                                const QJsonObject& payload)
    {
        const QString roomId =
            payload.value(QStringLiteral("roomId")).toString();
        const QString requestId =
            payload.value(QStringLiteral("requestId")).toString();
        const QJsonValue acceptedValue =
            payload.value(QStringLiteral("accepted"));
        const QJsonValue reasonValue =
            payload.value(QStringLiteral("reasonCode"));
        if (roomId.isEmpty() || roomId.size() > 128 ||
            requestId.isEmpty() || requestId.size() > 128 ||
            !acceptedValue.isBool() ||
            (!reasonValue.isUndefined() && !reasonValue.isString()) ||
            reasonValue.toString().size() > 64) {
            SendError(owner->socket, {},
                      QStringLiteral("invalid_room_join_response"),
                      QStringLiteral("The room join response is invalid."));
            return;
        }

        auto roomIt = rooms_.find(roomId);
        if (roomIt == rooms_.end()) {
            SendError(owner->socket, {}, QStringLiteral("room_not_found"),
                      QStringLiteral("The room does not exist."));
            return;
        }
        if (roomIt->ownerDeviceId != owner->claims.deviceId) {
            SendError(owner->socket, {},
                      QStringLiteral("room_owner_required"),
                      QStringLiteral("Only the room owner can decide join requests."));
            return;
        }
        const auto ownerMemberIt =
            roomIt->members.constFind(owner->claims.deviceId);
        if (ownerMemberIt == roomIt->members.cend() ||
            ownerMemberIt->disconnectedAtMs != 0) {
            SendError(owner->socket, {},
                      QStringLiteral("room_resume_required"),
                      QStringLiteral("The room membership must be resumed before changing room state."));
            return;
        }

        auto requestIt = roomJoinRequests_.find(requestId);
        if (requestIt == roomJoinRequests_.end() ||
            requestIt->roomId != roomId) {
            SendError(owner->socket, {},
                      QStringLiteral("room_join_request_not_found"),
                      QStringLiteral("The room join request is no longer pending."));
            return;
        }

        const RoomJoinRequestState request = *requestIt;
        roomJoinRequests_.erase(requestIt);
        if (pendingRoomJoinRequestByDevice_.value(
                request.requesterDeviceId) == requestId) {
            pendingRoomJoinRequestByDevice_.remove(
                request.requesterDeviceId);
        }

        QWebSocket* requesterSocket =
            devices_.value(request.requesterDeviceId, nullptr);
        const auto sendResult = [&](bool accepted,
                                    const QString& reasonCode,
                                    const QString& reasonMessage) {
            SendRoomJoinResult(owner->socket, roomId, requestId, accepted,
                               reasonCode, reasonMessage);
            if (requesterSocket && requesterSocket != owner->socket) {
                SendRoomJoinResult(requesterSocket, roomId, requestId,
                                   accepted, reasonCode, reasonMessage);
            }
        };

        if (request.expiresAtMs <= QDateTime::currentMSecsSinceEpoch()) {
            sendResult(false, QStringLiteral("join_request_timeout"),
                       QStringLiteral("The room join request expired."));
            return;
        }

        if (!acceptedValue.toBool()) {
            QString reasonCode = reasonValue.toString().left(64);
            if (reasonCode.isEmpty()) {
                reasonCode = QStringLiteral("rejected_by_owner");
            }
            sendResult(false, reasonCode,
                       QStringLiteral("The room owner rejected the join request."));
            return;
        }

        ClientState* requesterClient = FindClient(requesterSocket);
        if (!requesterSocket || !requesterClient ||
            !requesterClient->registered ||
            requesterClient->claims.deviceId != request.requesterDeviceId) {
            sendResult(false, QStringLiteral("requester_offline"),
                       QStringLiteral("The requesting device is offline."));
            return;
        }
        if (DeviceHasRoom(request.requesterDeviceId) ||
            DeviceHasSession(request.requesterDeviceId)) {
            sendResult(false, QStringLiteral("device_busy"),
                       QStringLiteral("The requesting device joined another room or legacy session."));
            return;
        }
        if (roomIt->members.size() >= roomIt->capacity) {
            sendResult(false, QStringLiteral("room_full"),
                       QStringLiteral("The room reached capacity before the request was approved."));
            return;
        }

        RoomMemberState member;
        member.deviceId = request.requesterDeviceId;
        member.deviceName = requesterClient->deviceName;
        member.recoveryToken = CreateRecoveryToken();
        const QString recoveryToken = member.recoveryToken;
        const QString memberDeviceId = member.deviceId;
        QStringList existingMemberIds = roomIt->members.keys();
        existingMemberIds.sort(Qt::CaseSensitive);
        roomIt->members.insert(memberDeviceId, std::move(member));
        deviceRooms_.insert(request.requesterDeviceId, roomId);
        for (const auto& peerDeviceId : existingMemberIds) {
            CreateRoomPair(*roomIt, peerDeviceId, memberDeviceId);
        }

        sendResult(true, {}, {});
        SendRoomReady(requesterSocket, *roomIt, recoveryToken);
        BroadcastRoomState(*roomIt);
        SendRoomPairsReadyForMember(*roomIt, memberDeviceId);
    }

    void HandleRoomSetCapacity(ClientState* owner,
                               const QJsonObject& payload)
    {
        const QString roomId =
            payload.value(QStringLiteral("roomId")).toString();
        const QJsonValue capacityValue =
            payload.value(QStringLiteral("capacity"));
        const int capacity = capacityValue.toInt(-1);
        if (roomId.isEmpty() || roomId.size() > 128 ||
            !capacityValue.isDouble() ||
            capacityValue.toDouble(-1.0) !=
                static_cast<double>(capacity) ||
            capacity < 2 ||
            capacity > config_.maximumRoomMembers) {
            SendError(owner->socket, {},
                      QStringLiteral("invalid_room_capacity"),
                      QStringLiteral("The room ID or requested capacity is invalid."));
            return;
        }

        auto roomIt = rooms_.find(roomId);
        if (roomIt == rooms_.end()) {
            SendError(owner->socket, {}, QStringLiteral("room_not_found"),
                      QStringLiteral("The room does not exist."));
            return;
        }
        if (roomIt->ownerDeviceId != owner->claims.deviceId) {
            SendError(owner->socket, {},
                      QStringLiteral("room_owner_required"),
                      QStringLiteral("Only the room owner can change capacity."));
            return;
        }
        const auto ownerMemberIt =
            roomIt->members.constFind(owner->claims.deviceId);
        if (ownerMemberIt == roomIt->members.cend() ||
            ownerMemberIt->disconnectedAtMs != 0) {
            SendError(owner->socket, {},
                      QStringLiteral("room_resume_required"),
                      QStringLiteral("The room membership must be resumed before changing room state."));
            return;
        }
        if (capacity < roomIt->members.size()) {
            SendError(owner->socket, {},
                      QStringLiteral("room_capacity_below_occupancy"),
                      QStringLiteral("Room capacity cannot be lower than the current member count."));
            return;
        }

        roomIt->capacity = capacity;
        BroadcastRoomState(*roomIt);
    }

    void HandleRoomLeave(ClientState* memberClient,
                         const QJsonObject& payload)
    {
        const QString roomId =
            payload.value(QStringLiteral("roomId")).toString();
        const QJsonValue reasonValue =
            payload.value(QStringLiteral("reasonCode"));
        if (roomId.isEmpty() || roomId.size() > 128 ||
            (!reasonValue.isUndefined() && !reasonValue.isString()) ||
            reasonValue.toString().size() > 64) {
            SendError(memberClient->socket, {},
                      QStringLiteral("invalid_room_leave"),
                      QStringLiteral("The room leave request is invalid."));
            return;
        }

        auto roomIt = rooms_.find(roomId);
        if (roomIt == rooms_.end()) {
            SendError(memberClient->socket, {},
                      QStringLiteral("room_not_found"),
                      QStringLiteral("The room does not exist."));
            return;
        }
        const QString deviceId = memberClient->claims.deviceId;
        if (!roomIt->members.contains(deviceId)) {
            SendError(memberClient->socket, {},
                      QStringLiteral("room_access_denied"),
                      QStringLiteral("The device is not a room member."));
            return;
        }

        QString reasonCode = reasonValue.toString().left(64);
        if (deviceId == roomIt->ownerDeviceId) {
            if (reasonCode.isEmpty()) {
                reasonCode = QStringLiteral("room_owner_left");
            }
            CloseRoom(roomId, deviceId, reasonCode);
            return;
        }

        if (reasonCode.isEmpty()) {
            reasonCode = QStringLiteral("room_member_left");
        }
        if (roomIt->screenSharerDeviceId == deviceId ||
            roomIt->pendingScreenSharerDeviceId == deviceId) {
            ResetRoomScreenShare(*roomIt, deviceId, reasonCode);
        } else {
            if (roomIt->screenShareSwitchRequesterDeviceId == deviceId) {
                CancelPendingRoomScreenShareSwitch(
                    *roomIt,
                    QStringLiteral("screen_share_switch_requester_left"),
                    QStringLiteral("The takeover requester left the room."));
            }
            if (roomIt->pendingControllerDeviceId == deviceId) {
                CancelPendingRoomControl(
                    *roomIt, reasonCode,
                    QStringLiteral("The requesting controller left the room."));
            }
            if (roomIt->activeControllerDeviceId == deviceId) {
                RevokeRoomControl(*roomIt, deviceId, reasonCode);
            }
        }
        CloseRoomPairsForMember(*roomIt, deviceId, deviceId,
                                reasonCode);

        roomIt->members.remove(deviceId);
        if (deviceRooms_.value(deviceId) == roomId) {
            deviceRooms_.remove(deviceId);
        }
        // The leaving member no longer receives the room broadcast, so send
        // an explicit terminal acknowledgement that lets its local engine
        // clear the kLeaving state deterministically.
        SendRoomClosed(memberClient->socket, roomId, deviceId, reasonCode);
        BroadcastRoomState(*roomIt);
    }

    void HandleRoomMediaState(ClientState* memberClient,
                              const QJsonObject& payload)
    {
        const QString roomId =
            payload.value(QStringLiteral("roomId")).toString();
        const QJsonValue cameraValue =
            payload.value(QStringLiteral("cameraPublishing"));
        const QJsonValue microphoneValue =
            payload.value(QStringLiteral("microphonePublishing"));
        if (roomId.isEmpty() || roomId.size() > 128 ||
            !cameraValue.isBool() || !microphoneValue.isBool()) {
            SendError(memberClient->socket, {},
                      QStringLiteral("invalid_room_media_state"),
                      QStringLiteral("The room media state is invalid."));
            return;
        }

        auto roomIt = rooms_.find(roomId);
        if (roomIt == rooms_.end()) {
            SendError(memberClient->socket, {},
                      QStringLiteral("room_not_found"),
                      QStringLiteral("The room does not exist."));
            return;
        }
        auto memberIt = roomIt->members.find(memberClient->claims.deviceId);
        if (memberIt == roomIt->members.end()) {
            SendError(memberClient->socket, {},
                      QStringLiteral("room_access_denied"),
                      QStringLiteral("The device is not a room member."));
            return;
        }
        if (memberIt->disconnectedAtMs != 0) {
            SendError(memberClient->socket, {},
                      QStringLiteral("room_resume_required"),
                      QStringLiteral("Resume the room before changing media state."));
            return;
        }

        memberIt->cameraPublishing = cameraValue.toBool();
        memberIt->microphonePublishing = microphoneValue.toBool();
        BroadcastRoomState(*roomIt);
    }

    void HandleRoomResume(ClientState* client,
                          const QJsonObject& payload)
    {
        const QString roomId =
            payload.value(QStringLiteral("roomId")).toString();
        const QString recoveryToken =
            payload.value(QStringLiteral("recoveryToken")).toString();
        if (roomId.isEmpty() || roomId.size() > 128 ||
            recoveryToken.isEmpty() || recoveryToken.size() > 256) {
            SendError(client->socket, {},
                      QStringLiteral("room_resume_rejected"),
                      QStringLiteral("The room membership cannot be resumed."));
            return;
        }

        auto roomIt = rooms_.find(roomId);
        if (roomIt == rooms_.end() ||
            deviceRooms_.value(client->claims.deviceId) != roomId) {
            SendError(client->socket, {},
                      QStringLiteral("room_resume_rejected"),
                      QStringLiteral("The room membership cannot be resumed."));
            return;
        }
        auto memberIt = roomIt->members.find(client->claims.deviceId);
        if (memberIt == roomIt->members.end() ||
            memberIt->recoveryToken != recoveryToken) {
            SendError(client->socket, {},
                      QStringLiteral("room_resume_rejected"),
                      QStringLiteral("The room membership cannot be resumed."));
            return;
        }

        const qint64 now = QDateTime::currentMSecsSinceEpoch();
        if (memberIt->disconnectedAtMs > 0 &&
            now - memberIt->disconnectedAtMs >
                config_.sessionRecoveryWindowMs) {
            const QString deviceId = client->claims.deviceId;
            if (deviceId == roomIt->ownerDeviceId) {
                CloseRoom(roomId, deviceId,
                          QStringLiteral("signaling_recovery_timeout"));
            } else {
                roomIt->members.erase(memberIt);
                deviceRooms_.remove(deviceId);
                BroadcastRoomState(*roomIt);
                SendError(client->socket, {},
                          QStringLiteral("room_resume_rejected"),
                          QStringLiteral("The room recovery window expired."));
            }
            return;
        }

        memberIt->disconnectedAtMs = 0;
        memberIt->deviceName = client->deviceName;
        if (roomIt->screenShareState == QStringLiteral("recovering") &&
            roomIt->screenSharerDeviceId == client->claims.deviceId) {
            roomIt->screenShareState = QStringLiteral("active");
        }
        SendRoomReady(client->socket, *roomIt, memberIt->recoveryToken);
        BroadcastRoomState(*roomIt);
        if (roomIt->pendingScreenSharerDeviceId ==
                client->claims.deviceId ||
            roomIt->screenSharerDeviceId == client->claims.deviceId) {
            SendRoomScreenShareGranted(client->socket, *roomIt);
        }
        if (roomIt->activeControllerDeviceId ==
                client->claims.deviceId ||
            roomIt->screenSharerDeviceId == client->claims.deviceId) {
            SendRoomControlGranted(client->socket, *roomIt);
        }
        if (!roomIt->controlRequestId.isEmpty() &&
            roomIt->screenSharerDeviceId == client->claims.deviceId) {
            const auto requester = roomIt->members.constFind(
                roomIt->pendingControllerDeviceId);
            if (requester != roomIt->members.cend()) {
                QJsonObject forwarded;
                forwarded.insert(QStringLiteral("roomId"), roomIt->roomId);
                forwarded.insert(QStringLiteral("requestId"),
                                 roomIt->controlRequestId);
                forwarded.insert(QStringLiteral("requesterDeviceId"),
                                 requester->deviceId);
                forwarded.insert(QStringLiteral("requesterDeviceName"),
                                 requester->deviceName);
                SendEnvelope(client->socket,
                             QStringLiteral("room_control_requested"), {},
                             forwarded);
            }
        }
        SendRoomPairsReadyForMember(*roomIt, client->claims.deviceId);
    }
