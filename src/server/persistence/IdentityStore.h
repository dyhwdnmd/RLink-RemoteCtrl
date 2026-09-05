// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#pragma once

#include <QList>
#include <QString>

#include "src/server/auth/AuthTypes.h"

QT_BEGIN_NAMESPACE
class QSqlDatabase;
QT_END_NAMESPACE

namespace remote::server_persistence {

struct RemoteUser {
    qint64 id = 0;
    QString logtoSubject;
    QString username;
    QString displayName;
    QString email;

    bool isValid() const { return id > 0 && !logtoSubject.isEmpty(); }
};

enum class DeviceRegistrationStatus {
    kRegistered,
    kRevoked,
    kStoreError,
};

struct DeviceRegistrationResult {
    DeviceRegistrationStatus status = DeviceRegistrationStatus::kStoreError;
    QString publicDeviceId;
    QString errorMessage;

    bool registered() const {
        return status == DeviceRegistrationStatus::kRegistered;
    }
};

struct OwnedDeviceRecord {
    QString publicDeviceId;
    QString installationId;
    qint64 ownerUserId = 0;
    QString deviceName;
    qint64 createdAt = 0;
    qint64 lastSeenAt = 0;
    bool revoked = false;
};

class IdentityStore final {
public:
    IdentityStore();
    ~IdentityStore();

    IdentityStore(const IdentityStore&) = delete;
    IdentityStore& operator=(const IdentityStore&) = delete;

    bool Open(const QString& databaseFile, QString* errorMessage = nullptr);
    void Close();
    bool IsOpen() const;

    RemoteUser UpsertUser(
        const remote::server_auth::UserInfoClaims& claims,
        QString* errorMessage = nullptr);
    DeviceRegistrationResult RegisterDevice(
        qint64 ownerUserId,
        const QString& installationId,
        const QString& deviceName);
    QList<OwnedDeviceRecord> ListDevicesForUser(
        qint64 ownerUserId,
        QString* errorMessage = nullptr) const;
    bool DeleteUserByLogtoSubject(
        const QString& logtoSubject,
        bool* existed = nullptr,
        QString* errorMessage = nullptr);

private:
    bool EnsureSchema(QString* errorMessage);

    QString connectionName_;
};

}  // namespace remote::server_persistence
