// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

    QJsonObject BuildOwnedDevicesPayloadFromRecords(
        ClientState* recipient,
        quint64 revision,
        const QList<remote::server_persistence::OwnedDeviceRecord>& owned,
        const QString& storeError)
    {
        QJsonObject payload;
        payload.insert(QStringLiteral("revision"),
                       static_cast<double>(revision));
        QJsonArray devices;
        if (!recipient || recipient->trustedUserId <= 0 ||
            !identityStore_.IsOpen()) {
            payload.insert(QStringLiteral("devices"), devices);
            return payload;
        }
        if (!storeError.isEmpty()) {
            payload.insert(QStringLiteral("errorCode"),
                           QStringLiteral("identity_store_unavailable"));
            payload.insert(QStringLiteral("errorMessage"), storeError);
            payload.insert(QStringLiteral("devices"), devices);
            return payload;
        }
        for (const auto& device : owned) {
            QJsonObject item;
            item.insert(QStringLiteral("deviceId"), device.publicDeviceId);
            item.insert(QStringLiteral("deviceName"), device.deviceName);
            item.insert(QStringLiteral("createdAt"),
                        static_cast<double>(device.createdAt));
            item.insert(QStringLiteral("lastSeenAt"),
                        static_cast<double>(device.lastSeenAt));
            ClientState* onlineClient = FindClient(
                devices_.value(device.publicDeviceId, nullptr));
            const bool online = onlineClient && onlineClient->registered &&
                onlineClient->trustedUserId == recipient->trustedUserId;
            item.insert(QStringLiteral("online"), online);
            item.insert(QStringLiteral("current"),
                        device.publicDeviceId == recipient->claims.deviceId);
            devices.append(item);
        }
        payload.insert(QStringLiteral("devices"), devices);
        return payload;
    }

    QJsonObject BuildOwnedDevicesPayload(ClientState* recipient,
                                         quint64 revision)
    {
        QString storeError;
        const auto owned = identityStore_.ListDevicesForUser(
            recipient ? recipient->trustedUserId : 0, &storeError);
        return BuildOwnedDevicesPayloadFromRecords(
            recipient, revision, owned, storeError);
    }

    void HandleMyDevicesRequest(ClientState* client)
    {
        if (client->trustedUserId <= 0 || !identityStore_.IsOpen()) {
            SendError(client->socket, {},
                      QStringLiteral("my_devices_unavailable"),
                      QStringLiteral(
                          "Owned devices require Logto authentication."));
            return;
        }
        SendEnvelope(client->socket, QStringLiteral("my_devices_result"), {},
                     BuildOwnedDevicesPayload(client,
                                              ownedDevicesRevision_));
    }

    void BroadcastOwnedDevices(qint64 trustedUserId)
    {
        if (trustedUserId <= 0 || !identityStore_.IsOpen()) {
            return;
        }
        ++ownedDevicesRevision_;
        const auto accountIt = accountConnections_.constFind(trustedUserId);
        if (accountIt == accountConnections_.cend()) {
            return;
        }
        QString storeError;
        const auto owned = identityStore_.ListDevicesForUser(
            trustedUserId, &storeError);
        const QList<QWebSocket*> sockets = accountIt->values();
        for (QWebSocket* socket : sockets) {
            ClientState* state = FindClient(socket);
            if (!state || !state->registered ||
                state->trustedUserId != trustedUserId) {
                continue;
            }
            SendEnvelope(socket, QStringLiteral("my_devices_changed"), {},
                         BuildOwnedDevicesPayloadFromRecords(
                             state, ownedDevicesRevision_, owned,
                             storeError));
        }
    }
