// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#include "src/server/persistence/IdentityStore.h"

#include <QDateTime>
#include <QDir>
#include <QFileInfo>
#include <QList>
#include <QRandomGenerator>
#include <QRegularExpression>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QStringList>
#include <QVariant>
#include <QUuid>

#include <utility>

namespace remote::server_persistence {
namespace {

void SetError(QString* errorMessage, const QString& message) {
    if (errorMessage) {
        *errorMessage = message;
    }
}

QString QueryError(const QString& operation, const QSqlQuery& query) {
    return QStringLiteral("%1: %2")
        .arg(operation, query.lastError().text());
}

bool IsNineDigitPublicCode(const QString& value) {
    static const QRegularExpression pattern(
        QStringLiteral("^[1-9][0-9]{8}$"));
    return pattern.match(value).hasMatch();
}

QString AllocatePublicCode(QSqlDatabase database,
                           QString* errorMessage) {
    constexpr quint32 kFirstPublicCode = 100000000;
    constexpr quint32 kPublicCodeCount = 900000000;
    constexpr int kMaximumAttempts = 128;
    for (int attempt = 0; attempt < kMaximumAttempts; ++attempt) {
        const QString candidate = QString::number(
            kFirstPublicCode +
            QRandomGenerator::system()->bounded(kPublicCodeCount));
        QSqlQuery existing(database);
        existing.prepare(QStringLiteral(
            "SELECT 1 FROM devices WHERE public_code=:public_code LIMIT 1"));
        existing.bindValue(QStringLiteral(":public_code"), candidate);
        if (!existing.exec()) {
            SetError(errorMessage,
                     QueryError(QStringLiteral("Check device public code"),
                                existing));
            return {};
        }
        if (!existing.next()) {
            return candidate;
        }
    }
    SetError(errorMessage,
             QStringLiteral("Cannot allocate a unique 9-digit device ID."));
    return {};
}

QString DeviceTableSql(const QString& tableName) {
    return QStringLiteral(
        "CREATE TABLE %1 ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "public_id TEXT NOT NULL,"
        "public_code TEXT NOT NULL UNIQUE,"
        "owner_user_id INTEGER NOT NULL,"
        "device_name TEXT NOT NULL,"
        "created_at INTEGER NOT NULL,"
        "last_seen_at INTEGER NOT NULL,"
        "revoked INTEGER NOT NULL DEFAULT 0 CHECK(revoked IN (0, 1)),"
        "FOREIGN KEY(owner_user_id) REFERENCES users(id) ON DELETE CASCADE,"
        "UNIQUE(owner_user_id, public_id))")
        .arg(tableName);
}

bool HasUniqueDeviceIndex(QSqlDatabase database,
                          const QStringList& expectedColumns,
                          bool* found,
                          QString* errorMessage) {
    *found = false;
    QSqlQuery indexes(database);
    if (!indexes.exec(QStringLiteral("PRAGMA index_list(devices)"))) {
        SetError(errorMessage,
                 QueryError(QStringLiteral("Inspect device indexes"), indexes));
        return false;
    }
    QStringList uniqueIndexNames;
    while (indexes.next()) {
        if (indexes.value(2).toBool()) {
            uniqueIndexNames.push_back(indexes.value(1).toString());
        }
    }
    indexes.finish();
    for (QString indexName : uniqueIndexNames) {
        indexName.replace(QChar('\''), QStringLiteral("''"));
        QSqlQuery columns(database);
        if (!columns.exec(QStringLiteral("PRAGMA index_info('%1')")
                              .arg(indexName))) {
            SetError(errorMessage,
                     QueryError(QStringLiteral("Inspect device index columns"),
                                columns));
            return false;
        }
        QStringList actualColumns;
        while (columns.next()) {
            actualColumns.push_back(columns.value(2).toString());
        }
        columns.finish();
        if (actualColumns == expectedColumns) {
            *found = true;
            return true;
        }
    }
    return true;
}

bool RebuildDeviceTableForAccountScope(QSqlDatabase database,
                                       QString* errorMessage) {
    QSqlQuery query(database);
    const QStringList statements = {
        QStringLiteral("DROP INDEX IF EXISTS idx_devices_owner"),
        QStringLiteral("DROP INDEX IF EXISTS idx_devices_public_code"),
        QStringLiteral(
            "ALTER TABLE devices RENAME TO devices_account_scope_legacy"),
        DeviceTableSql(QStringLiteral("devices")),
        QStringLiteral(
            "INSERT INTO devices(id, public_id, public_code, owner_user_id, "
            "device_name, created_at, last_seen_at, revoked) "
            "SELECT id, public_id, public_code, owner_user_id, device_name, "
            "created_at, last_seen_at, revoked "
            "FROM devices_account_scope_legacy"),
        QStringLiteral("DROP TABLE devices_account_scope_legacy"),
    };
    for (const QString& statement : statements) {
        if (!query.exec(statement)) {
            SetError(errorMessage,
                     QueryError(QStringLiteral("Migrate account-scoped devices"),
                                query));
            return false;
        }
    }
    return true;
}

}  // namespace

IdentityStore::IdentityStore()
    : connectionName_(QStringLiteral("remotec-identity-%1").arg(
          QUuid::createUuid().toString(QUuid::WithoutBraces))) {}

IdentityStore::~IdentityStore() {
    Close();
}

bool IdentityStore::Open(const QString& databaseFile,
                         QString* errorMessage) {
    Close();
    if (databaseFile.trimmed().isEmpty()) {
        SetError(errorMessage, QStringLiteral("Identity database path is empty."));
        return false;
    }

    if (databaseFile != QStringLiteral(":memory:")) {
        const QFileInfo info(databaseFile);
        QDir directory = info.dir();
        if (!directory.exists() && !directory.mkpath(QStringLiteral("."))) {
            SetError(errorMessage,
                     QStringLiteral("Cannot create the identity database directory."));
            return false;
        }
    }

    QSqlDatabase database =
        QSqlDatabase::addDatabase(QStringLiteral("QSQLITE"), connectionName_);
    database.setDatabaseName(databaseFile);
    database.setConnectOptions(QStringLiteral("QSQLITE_BUSY_TIMEOUT=5000"));
    if (!database.open()) {
        const QString detail = database.lastError().text();
        database = {};
        QSqlDatabase::removeDatabase(connectionName_);
        SetError(errorMessage,
                 QStringLiteral("Cannot open the identity database: %1").arg(detail));
        return false;
    }

    QSqlQuery pragma(database);
    if (!pragma.exec(QStringLiteral("PRAGMA foreign_keys = ON"))) {
        const QString detail = QueryError(QStringLiteral("Enable foreign keys"), pragma);
        Close();
        SetError(errorMessage, detail);
        return false;
    }
    if (databaseFile != QStringLiteral(":memory:") &&
        !pragma.exec(QStringLiteral("PRAGMA journal_mode = WAL"))) {
        const QString detail = QueryError(QStringLiteral("Enable WAL"), pragma);
        Close();
        SetError(errorMessage, detail);
        return false;
    }
    pragma.finish();
    database = {};
    if (!EnsureSchema(errorMessage)) {
        Close();
        return false;
    }
    return true;
}

void IdentityStore::Close() {
    if (!QSqlDatabase::contains(connectionName_)) {
        return;
    }
    {
        QSqlDatabase database = QSqlDatabase::database(connectionName_, false);
        if (database.isValid()) {
            database.close();
        }
    }
    QSqlDatabase::removeDatabase(connectionName_);
}

bool IdentityStore::IsOpen() const {
    if (!QSqlDatabase::contains(connectionName_)) {
        return false;
    }
    return QSqlDatabase::database(connectionName_, false).isOpen();
}

bool IdentityStore::EnsureSchema(QString* errorMessage) {
    QSqlDatabase database = QSqlDatabase::database(connectionName_, false);
    QSqlQuery query(database);
    const QStringList statements = {
        QStringLiteral(
            "CREATE TABLE IF NOT EXISTS users ("
            "id INTEGER PRIMARY KEY AUTOINCREMENT,"
            "logto_subject TEXT NOT NULL UNIQUE,"
            "username TEXT NOT NULL DEFAULT '',"
            "display_name TEXT NOT NULL DEFAULT '',"
            "email TEXT NOT NULL DEFAULT '',"
            "created_at INTEGER NOT NULL,"
            "last_login_at INTEGER NOT NULL)"),
        DeviceTableSql(QStringLiteral("IF NOT EXISTS devices")),
        QStringLiteral(
            "CREATE INDEX IF NOT EXISTS idx_devices_owner "
            "ON devices(owner_user_id)"),
    };
    for (const QString& statement : statements) {
        if (!query.exec(statement)) {
            SetError(errorMessage,
                     QueryError(QStringLiteral("Create identity schema"), query));
            return false;
        }
    }
    query.finish();

    bool hasUsername = false;
    QSqlQuery userColumns(database);
    if (!userColumns.exec(QStringLiteral("PRAGMA table_info(users)"))) {
        SetError(errorMessage,
                 QueryError(QStringLiteral("Inspect user schema"),
                            userColumns));
        return false;
    }
    while (userColumns.next()) {
        if (userColumns.value(1).toString() == QStringLiteral("username")) {
            hasUsername = true;
            break;
        }
    }
    userColumns.finish();

    bool hasPublicCode = false;
    QSqlQuery columns(database);
    if (!columns.exec(QStringLiteral("PRAGMA table_info(devices)"))) {
        SetError(errorMessage,
                 QueryError(QStringLiteral("Inspect device schema"), columns));
        return false;
    }
    while (columns.next()) {
        if (columns.value(1).toString() == QStringLiteral("public_code")) {
            hasPublicCode = true;
            break;
        }
    }
    columns.finish();

    bool hasGlobalInstallationUnique = false;
    bool hasAccountInstallationUnique = false;
    if (!HasUniqueDeviceIndex(
            database, {QStringLiteral("public_id")},
            &hasGlobalInstallationUnique, errorMessage) ||
        !HasUniqueDeviceIndex(
            database,
            {QStringLiteral("owner_user_id"), QStringLiteral("public_id")},
            &hasAccountInstallationUnique, errorMessage)) {
        return false;
    }

    if (!database.transaction()) {
        SetError(errorMessage,
                 QStringLiteral("Cannot start the identity schema migration."));
        return false;
    }
    if (!hasUsername) {
        QSqlQuery alter(database);
        if (!alter.exec(QStringLiteral(
                "ALTER TABLE users ADD COLUMN username TEXT NOT NULL "
                "DEFAULT ''"))) {
            database.rollback();
            SetError(errorMessage,
                     QueryError(QStringLiteral("Add user username"), alter));
            return false;
        }
    }
    if (!hasPublicCode) {
        QSqlQuery alter(database);
        if (!alter.exec(QStringLiteral(
                "ALTER TABLE devices ADD COLUMN public_code TEXT"))) {
            database.rollback();
            SetError(errorMessage,
                     QueryError(QStringLiteral("Add device public code"), alter));
            return false;
        }
    }

    QSqlQuery devices(database);
    if (!devices.exec(QStringLiteral(
            "SELECT id, public_code FROM devices ORDER BY id"))) {
        database.rollback();
        SetError(errorMessage,
                 QueryError(QStringLiteral("Load device public codes"), devices));
        return false;
    }
    QList<qint64> devicesNeedingCode;
    while (devices.next()) {
        if (!IsNineDigitPublicCode(devices.value(1).toString())) {
            devicesNeedingCode.push_back(devices.value(0).toLongLong());
        }
    }
    devices.finish();
    for (const qint64 deviceRowId : devicesNeedingCode) {
        QString allocationError;
        const QString publicCode =
            AllocatePublicCode(database, &allocationError);
        if (publicCode.isEmpty()) {
            database.rollback();
            SetError(errorMessage, allocationError);
            return false;
        }
        QSqlQuery update(database);
        update.prepare(QStringLiteral(
            "UPDATE devices SET public_code=:public_code WHERE id=:id"));
        update.bindValue(QStringLiteral(":public_code"), publicCode);
        update.bindValue(QStringLiteral(":id"), deviceRowId);
        if (!update.exec()) {
            database.rollback();
            SetError(errorMessage,
                     QueryError(QStringLiteral("Migrate device public code"),
                                update));
            return false;
        }
    }

    if (hasGlobalInstallationUnique || !hasAccountInstallationUnique) {
        if (!RebuildDeviceTableForAccountScope(database, errorMessage)) {
            database.rollback();
            return false;
        }
    }

    QSqlQuery publicCodeIndex(database);
    if (!publicCodeIndex.exec(QStringLiteral(
            "CREATE UNIQUE INDEX IF NOT EXISTS idx_devices_public_code "
            "ON devices(public_code)"))) {
        database.rollback();
        SetError(errorMessage,
                 QueryError(QStringLiteral("Index device public code"),
                            publicCodeIndex));
        return false;
    }
    publicCodeIndex.finish();
    QSqlQuery ownerIndex(database);
    if (!ownerIndex.exec(QStringLiteral(
            "CREATE INDEX IF NOT EXISTS idx_devices_owner "
            "ON devices(owner_user_id)"))) {
        database.rollback();
        SetError(errorMessage,
                 QueryError(QStringLiteral("Index device owner"), ownerIndex));
        return false;
    }
    ownerIndex.finish();
    if (!database.commit()) {
        const QString detail = database.lastError().text();
        database.rollback();
        SetError(errorMessage,
                 QStringLiteral(
                     "Cannot commit the identity schema migration: %1")
                     .arg(detail));
        return false;
    }
    return true;
}

RemoteUser IdentityStore::UpsertUser(
    const remote::server_auth::UserInfoClaims& claims,
    QString* errorMessage) {
    RemoteUser user;
    if (!IsOpen() || claims.subject.isEmpty()) {
        SetError(errorMessage, QStringLiteral("The trusted Logto subject is invalid."));
        return user;
    }

    QSqlDatabase database = QSqlDatabase::database(connectionName_, false);
    const qint64 now = QDateTime::currentSecsSinceEpoch();
    QSqlQuery upsert(database);
    upsert.prepare(QStringLiteral(
        "INSERT INTO users(logto_subject, username, display_name, email, "
        "created_at, last_login_at) "
        "VALUES(:subject, :username, :name, :email, :now, :now) "
        "ON CONFLICT(logto_subject) DO UPDATE SET "
        "username=excluded.username, display_name=excluded.display_name, "
        "email=excluded.email, "
        "last_login_at=excluded.last_login_at"));
    upsert.bindValue(QStringLiteral(":subject"), claims.subject);
    upsert.bindValue(
        QStringLiteral(":username"),
        claims.username.isNull() ? QStringLiteral("") : claims.username);
    upsert.bindValue(
        QStringLiteral(":name"),
        claims.displayName.isNull() ? QStringLiteral("")
                                    : claims.displayName);
    upsert.bindValue(
        QStringLiteral(":email"),
        claims.email.isNull() ? QStringLiteral("") : claims.email);
    upsert.bindValue(QStringLiteral(":now"), now);
    if (!upsert.exec()) {
        SetError(errorMessage, QueryError(QStringLiteral("Upsert user"), upsert));
        return user;
    }

    QSqlQuery select(database);
    select.prepare(QStringLiteral(
        "SELECT id, logto_subject, username, display_name, email "
        "FROM users WHERE logto_subject=:subject"));
    select.bindValue(QStringLiteral(":subject"), claims.subject);
    if (!select.exec() || !select.next()) {
        SetError(errorMessage, QueryError(QStringLiteral("Load user"), select));
        return user;
    }
    user.id = select.value(0).toLongLong();
    user.logtoSubject = select.value(1).toString();
    user.username = select.value(2).toString();
    user.displayName = select.value(3).toString();
    user.email = select.value(4).toString();
    return user;
}

DeviceRegistrationResult IdentityStore::RegisterDevice(
    qint64 ownerUserId,
    const QString& installationId,
    const QString& deviceName) {
    DeviceRegistrationResult result;
    if (!IsOpen() || ownerUserId <= 0 || installationId.isEmpty() ||
        deviceName.isEmpty()) {
        result.errorMessage = QStringLiteral("The device registration is invalid.");
        return result;
    }

    QSqlDatabase database = QSqlDatabase::database(connectionName_, false);
    QSqlQuery existing(database);
    existing.prepare(QStringLiteral(
        "SELECT revoked, public_code "
        "FROM devices WHERE owner_user_id=:owner_user_id "
        "AND public_id=:installation_id"));
    existing.bindValue(QStringLiteral(":owner_user_id"), ownerUserId);
    existing.bindValue(QStringLiteral(":installation_id"), installationId);
    if (!existing.exec()) {
        result.errorMessage = QueryError(QStringLiteral("Load device"), existing);
        return result;
    }

    const qint64 now = QDateTime::currentSecsSinceEpoch();
    if (existing.next()) {
        if (existing.value(0).toBool()) {
            result.status = DeviceRegistrationStatus::kRevoked;
            return result;
        }
        QString publicCode = existing.value(1).toString();
        if (!IsNineDigitPublicCode(publicCode)) {
            publicCode = AllocatePublicCode(database, &result.errorMessage);
            if (publicCode.isEmpty()) {
                return result;
            }
        }
        QSqlQuery update(database);
        update.prepare(QStringLiteral(
            "UPDATE devices SET device_name=:name, last_seen_at=:now, "
            "public_code=:public_code "
            "WHERE public_id=:installation_id "
            "AND owner_user_id=:owner_user_id"));
        update.bindValue(QStringLiteral(":name"), deviceName);
        update.bindValue(QStringLiteral(":now"), now);
        update.bindValue(QStringLiteral(":public_code"), publicCode);
        update.bindValue(QStringLiteral(":installation_id"), installationId);
        update.bindValue(QStringLiteral(":owner_user_id"), ownerUserId);
        if (!update.exec()) {
            result.errorMessage = QueryError(QStringLiteral("Update device"), update);
            return result;
        }
        result.status = DeviceRegistrationStatus::kRegistered;
        result.publicDeviceId = publicCode;
        return result;
    }

    const QString publicCode =
        AllocatePublicCode(database, &result.errorMessage);
    if (publicCode.isEmpty()) {
        return result;
    }
    QSqlQuery insert(database);
    insert.prepare(QStringLiteral(
        "INSERT INTO devices(public_id, public_code, owner_user_id, "
        "device_name, created_at, last_seen_at, revoked) "
        "VALUES(:installation_id, :public_code, :owner_user_id, "
        ":name, :now, :now, 0)"));
    insert.bindValue(QStringLiteral(":installation_id"), installationId);
    insert.bindValue(QStringLiteral(":public_code"), publicCode);
    insert.bindValue(QStringLiteral(":owner_user_id"), ownerUserId);
    insert.bindValue(QStringLiteral(":name"), deviceName);
    insert.bindValue(QStringLiteral(":now"), now);
    if (!insert.exec()) {
        result.errorMessage = QueryError(QStringLiteral("Insert device"), insert);
        return result;
    }
    result.status = DeviceRegistrationStatus::kRegistered;
    result.publicDeviceId = publicCode;
    return result;
}

QList<OwnedDeviceRecord> IdentityStore::ListDevicesForUser(
    qint64 ownerUserId,
    QString* errorMessage) const {
    QList<OwnedDeviceRecord> result;
    if (!IsOpen() || ownerUserId <= 0) {
        SetError(errorMessage,
                 QStringLiteral("The trusted user identity is invalid."));
        return result;
    }

    QSqlDatabase database = QSqlDatabase::database(connectionName_, false);
    QSqlQuery query(database);
    query.prepare(QStringLiteral(
        "SELECT public_code, public_id, owner_user_id, device_name, "
        "created_at, last_seen_at, revoked "
        "FROM devices WHERE owner_user_id=:owner_user_id "
        "ORDER BY revoked ASC, last_seen_at DESC, id ASC"));
    query.bindValue(QStringLiteral(":owner_user_id"), ownerUserId);
    if (!query.exec()) {
        SetError(errorMessage,
                 QueryError(QStringLiteral("List owned devices"), query));
        return result;
    }
    while (query.next()) {
        OwnedDeviceRecord device;
        device.publicDeviceId = query.value(0).toString();
        device.installationId = query.value(1).toString();
        device.ownerUserId = query.value(2).toLongLong();
        device.deviceName = query.value(3).toString();
        device.createdAt = query.value(4).toLongLong();
        device.lastSeenAt = query.value(5).toLongLong();
        device.revoked = query.value(6).toBool();
        if (!device.revoked && IsNineDigitPublicCode(device.publicDeviceId)) {
            result.push_back(std::move(device));
        }
    }
    return result;
}

bool IdentityStore::DeleteUserByLogtoSubject(
    const QString& logtoSubject,
    bool* existed,
    QString* errorMessage) {
    if (existed) {
        *existed = false;
    }
    const QString subject = logtoSubject.trimmed();
    if (!IsOpen() || subject.isEmpty() || subject.size() > 256) {
        SetError(errorMessage,
                 QStringLiteral("The trusted Logto subject is invalid."));
        return false;
    }

    QSqlDatabase database = QSqlDatabase::database(connectionName_, false);
    QSqlQuery remove(database);
    remove.prepare(QStringLiteral(
        "DELETE FROM users WHERE logto_subject=:subject"));
    remove.bindValue(QStringLiteral(":subject"), subject);
    if (!remove.exec()) {
        SetError(errorMessage, QueryError(QStringLiteral("Delete user"), remove));
        return false;
    }
    if (existed) {
        *existed = remove.numRowsAffected() > 0;
    }
    return true;
}

}  // namespace remote::server_persistence
