// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#include <QCommandLineOption>
#include <QCommandLineParser>
#include <QCoreApplication>
#include <QCryptographicHash>
#include <QFile>
#include <QHostAddress>
#include <QMessageAuthenticationCode>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QTemporaryDir>
#include <QTextStream>
#include <QUrl>

#include <algorithm>

#include "AccessTokenService.h"
#include "SignalServer.h"
#include "src/server/auth/LogtoUserInfoClient.h"
#include "src/server/auth/LogtoWebhookServer.h"
#include "src/server/persistence/IdentityStore.h"

namespace {

QByteArray ReadSecret(const QString& path, QString* error)
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) {
        if (error) {
            *error = QStringLiteral("Cannot open token secret file %1: %2")
                         .arg(path, file.errorString());
        }
        return {};
    }
    return file.readAll().trimmed();
}

int Fail(const QString& message)
{
    QTextStream(stderr) << "SIGNAL_SERVER_ERROR=" << message << Qt::endl;
    return 1;
}

bool IsNineDigitPublicId(const QString& value)
{
    return value.size() == 9 && value.front() != QChar('0') &&
           std::all_of(value.cbegin(), value.cend(),
                       [](QChar character) { return character.isDigit(); });
}

int RunAuthStorageSelfTest()
{
    using remote::server_auth::LogtoUserInfoClient;
    using remote::server_auth::LogtoWebhookServer;
    using remote::server_auth::UserInfoStatus;
    using remote::server_persistence::DeviceRegistrationStatus;
    using remote::server_persistence::IdentityStore;

    const auto valid = LogtoUserInfoClient::ParseResponseForTesting(
        200,
        QByteArrayLiteral(
            "{\"sub\":\"logto-user-a\",\"username\":\"alice\","
            "\"name\":\"Alice\","
            "\"email\":\"alice@example.test\"}"));
    const auto missingSubject =
        LogtoUserInfoClient::ParseResponseForTesting(
            200, QByteArrayLiteral("{\"name\":\"Missing sub\"}"));
    const auto invalidToken =
        LogtoUserInfoClient::ParseResponseForTesting(401, {});
    const auto throttled =
        LogtoUserInfoClient::ParseResponseForTesting(429, {});
    if (!valid.authenticated() || valid.claims.subject !=
            QStringLiteral("logto-user-a") ||
        valid.claims.username != QStringLiteral("alice") ||
        missingSubject.status != UserInfoStatus::kInvalidResponse ||
        invalidToken.status != UserInfoStatus::kInvalidToken ||
        invalidToken.retryable ||
        throttled.status != UserInfoStatus::kServiceUnavailable ||
        !throttled.retryable) {
        return Fail(QStringLiteral("UserInfo response classification self-test failed."));
    }

    QTemporaryDir directory;
    if (!directory.isValid()) {
        return Fail(QStringLiteral("Cannot create the identity self-test directory."));
    }
    IdentityStore store;
    QString error;
    if (!store.Open(directory.filePath(QStringLiteral("identity.sqlite")),
                    &error)) {
        return Fail(error);
    }
    const auto firstUser = store.UpsertUser(valid.claims, &error);
    auto updatedClaims = valid.claims;
    updatedClaims.username = QStringLiteral("alice-updated");
    updatedClaims.displayName = QStringLiteral("Alice Updated");
    const auto sameUser = store.UpsertUser(updatedClaims, &error);
    remote::server_auth::UserInfoClaims secondClaims;
    secondClaims.subject = QStringLiteral("logto-user-b");
    secondClaims.username = QStringLiteral("bob");
    secondClaims.displayName = QStringLiteral("Bob");
    const auto secondUser = store.UpsertUser(secondClaims, &error);
    if (!firstUser.isValid() || !sameUser.isValid() ||
        !secondUser.isValid() || firstUser.id != sameUser.id ||
        firstUser.id == secondUser.id ||
        firstUser.username != QStringLiteral("alice") ||
        sameUser.username != QStringLiteral("alice-updated") ||
        secondUser.username != QStringLiteral("bob")) {
        return Fail(error.isEmpty()
                        ? QStringLiteral("RemoteC user mapping self-test failed.")
                        : error);
    }
    const auto firstDevice = store.RegisterDevice(
        firstUser.id, QStringLiteral("device-self-test"),
        QStringLiteral("First device"));
    const auto sameOwner = store.RegisterDevice(
        firstUser.id, QStringLiteral("device-self-test"),
        QStringLiteral("Renamed device"));
    const auto otherOwner = store.RegisterDevice(
        secondUser.id, QStringLiteral("device-self-test"),
        QStringLiteral("Shared computer"));
    if (!firstDevice.registered() || !sameOwner.registered() ||
        !otherOwner.registered() ||
        !IsNineDigitPublicId(firstDevice.publicDeviceId) ||
        sameOwner.publicDeviceId != firstDevice.publicDeviceId ||
        !IsNineDigitPublicId(otherOwner.publicDeviceId) ||
        otherOwner.publicDeviceId == firstDevice.publicDeviceId) {
        return Fail(QStringLiteral("Account-scoped device self-test failed."));
    }
    const auto firstOwnedDevices = store.ListDevicesForUser(
        firstUser.id, &error);
    const auto secondOwnedDevices = store.ListDevicesForUser(
        secondUser.id, &error);
    if (firstOwnedDevices.size() != 1 ||
        secondOwnedDevices.size() != 1 ||
        firstOwnedDevices.front().publicDeviceId !=
            firstDevice.publicDeviceId ||
        firstOwnedDevices.front().deviceName !=
            QStringLiteral("Renamed device") ||
        secondOwnedDevices.front().publicDeviceId !=
            otherOwner.publicDeviceId) {
        return Fail(error.isEmpty()
                        ? QStringLiteral(
                              "Owned-device isolation self-test failed.")
                        : error);
    }
    bool deletedUserExisted = false;
    if (!store.DeleteUserByLogtoSubject(
            QStringLiteral("logto-user-b"), &deletedUserExisted, &error) ||
        !deletedUserExisted ||
        !store.DeleteUserByLogtoSubject(
            QStringLiteral("logto-user-b"), &deletedUserExisted, &error) ||
        deletedUserExisted) {
        return Fail(error.isEmpty()
                        ? QStringLiteral("Idempotent account deletion self-test failed.")
                        : error);
    }
    const QByteArray webhookBody = QByteArrayLiteral(
        "{\"event\":\"User.Deleted\",\"params\":{\"userId\":\"u1\"}}");
    const QByteArray webhookKey = QByteArrayLiteral("webhook-test-signing-key");
    const QByteArray webhookSignature = QMessageAuthenticationCode::hash(
        webhookBody, webhookKey, QCryptographicHash::Sha256).toHex();
    if (!LogtoWebhookServer::VerifySignatureForTesting(
            webhookBody, webhookKey, webhookSignature) ||
        LogtoWebhookServer::VerifySignatureForTesting(
            webhookBody + ' ', webhookKey, webhookSignature)) {
        return Fail(QStringLiteral("Webhook signature self-test failed."));
    }

    const QString legacyDatabaseFile = directory.filePath(
        QStringLiteral("legacy-identity.sqlite"));
    const QString legacyConnection = QStringLiteral(
        "remotec-legacy-schema-self-test");
    {
        QSqlDatabase legacy = QSqlDatabase::addDatabase(
            QStringLiteral("QSQLITE"), legacyConnection);
        legacy.setDatabaseName(legacyDatabaseFile);
        if (!legacy.open()) {
            return Fail(QStringLiteral(
                "Cannot create the legacy identity self-test database."));
        }
        QSqlQuery query(legacy);
        const QStringList legacyStatements = {
            QStringLiteral(
                "CREATE TABLE users (id INTEGER PRIMARY KEY AUTOINCREMENT, "
                "logto_subject TEXT NOT NULL UNIQUE, display_name TEXT NOT NULL "
                "DEFAULT '', email TEXT NOT NULL DEFAULT '', created_at INTEGER "
                "NOT NULL, last_login_at INTEGER NOT NULL)"),
            QStringLiteral(
                "CREATE TABLE devices (id INTEGER PRIMARY KEY AUTOINCREMENT, "
                "public_id TEXT NOT NULL UNIQUE, owner_user_id INTEGER NOT NULL, "
                "device_name TEXT NOT NULL, created_at INTEGER NOT NULL, "
                "last_seen_at INTEGER NOT NULL, revoked INTEGER NOT NULL DEFAULT 0)"),
            QStringLiteral(
                "INSERT INTO users(logto_subject, display_name, email, "
                "created_at, last_login_at) VALUES('legacy-user', 'Legacy', '', 1, 1)"),
            QStringLiteral(
                "INSERT INTO devices(public_id, owner_user_id, device_name, "
                "created_at, last_seen_at, revoked) "
                "VALUES('legacy-installation', 1, 'Legacy device', 1, 1, 0)"),
        };
        for (const QString& statement : legacyStatements) {
            if (!query.exec(statement)) {
                return Fail(QStringLiteral(
                    "Cannot prepare the legacy identity self-test schema."));
            }
        }
        legacy.close();
    }
    QSqlDatabase::removeDatabase(legacyConnection);
    IdentityStore migratedStore;
    if (!migratedStore.Open(legacyDatabaseFile, &error)) {
        return Fail(error);
    }
    remote::server_auth::UserInfoClaims legacyClaims;
    legacyClaims.subject = QStringLiteral("legacy-user");
    legacyClaims.username = QStringLiteral("legacy");
    legacyClaims.displayName = QStringLiteral("Legacy Updated");
    const auto legacyUser = migratedStore.UpsertUser(legacyClaims, &error);
    const auto migratedDevice = migratedStore.RegisterDevice(
        legacyUser.id, QStringLiteral("legacy-installation"),
        QStringLiteral("Migrated device"));
    if (!legacyUser.isValid() ||
        legacyUser.username != QStringLiteral("legacy") ||
        !migratedDevice.registered() ||
        !IsNineDigitPublicId(migratedDevice.publicDeviceId)) {
        return Fail(error.isEmpty()
                        ? QStringLiteral("Legacy device schema migration failed.")
                        : error);
    }

    QTextStream(stdout) << "USERINFO_CLASSIFICATION=PASS" << Qt::endl
                        << "IDENTITY_STORE=PASS" << Qt::endl
                        << "DEVICE_ACCOUNT_SCOPING=PASS" << Qt::endl
                        << "OWNED_DEVICE_ISOLATION=PASS" << Qt::endl
                        << "DEVICE_PUBLIC_ID_MIGRATION=PASS" << Qt::endl
                        << "ACCOUNT_DELETION=PASS" << Qt::endl
                        << "WEBHOOK_SIGNATURE=PASS" << Qt::endl
                        << "AUTH_STORAGE_SELF_TEST=PASS" << Qt::endl;
    return 0;
}

}  // namespace

int main(int argc, char* argv[])
{
    QCoreApplication application(argc, argv);
    application.setApplicationName(QStringLiteral("RemoteCSignalServer"));
    application.setApplicationVersion(QStringLiteral("0.1.0"));

    QCommandLineParser parser;
    parser.setApplicationDescription(
        QStringLiteral("RemoteC lightweight WSS signaling server"));
    parser.addHelpOption();
    parser.addVersionOption();

    const QCommandLineOption listenOption(
        QStringLiteral("listen"), QStringLiteral("IPv4 address to bind."),
        QStringLiteral("address"), QStringLiteral("127.0.0.1"));
    const QCommandLineOption portOption(
        QStringLiteral("port"), QStringLiteral("TCP port to bind."),
        QStringLiteral("port"), QStringLiteral("9443"));
    const QCommandLineOption certificateOption(
        QStringLiteral("cert"), QStringLiteral("PEM TLS certificate chain."),
        QStringLiteral("file"));
    const QCommandLineOption privateKeyOption(
        QStringLiteral("key"), QStringLiteral("PEM RSA private key."),
        QStringLiteral("file"));
    const QCommandLineOption secretFileOption(
        QStringLiteral("secret-file"),
        QStringLiteral("File containing at least 32 bytes of token secret."),
        QStringLiteral("file"));
    const QCommandLineOption issueTokenOption(
        QStringLiteral("issue-token"),
        QStringLiteral("Issue one short-lived token for a device and exit."),
        QStringLiteral("device-id"));
    const QCommandLineOption tokenLifetimeOption(
        QStringLiteral("ttl-seconds"),
        QStringLiteral("Issued token lifetime, maximum 604800 seconds."),
        QStringLiteral("seconds"), QStringLiteral("3600"));
    const QCommandLineOption iceUrlOption(
        QStringLiteral("ice-url"),
        QStringLiteral("STUN/TURN URL included in session_ready; repeatable."),
        QStringLiteral("url"));
    const QCommandLineOption maximumRoomMembersOption(
        QStringLiteral("max-room-members"),
        QStringLiteral("Deployment room-member limit, from 2 to 5."),
        QStringLiteral("count"), QStringLiteral("5"));
    const QCommandLineOption maximumConnectionsOption(
        QStringLiteral("max-connections"),
        QStringLiteral("Maximum number of active WebSocket connections."),
        QStringLiteral("count"), QStringLiteral("5000"));
    const QCommandLineOption maximumUnauthenticatedConnectionsOption(
        QStringLiteral("max-unauthenticated-connections"),
        QStringLiteral(
            "Maximum number of connected clients awaiting authentication."),
        QStringLiteral("count"), QStringLiteral("512"));
    const QCommandLineOption maximumConnectionsPerIpOption(
        QStringLiteral("max-connections-per-ip"),
        QStringLiteral(
            "Maximum active connections per source IP; zero disables this limit."),
        QStringLiteral("count"), QStringLiteral("0"));
    const QCommandLineOption diagnosticsIntervalOption(
        QStringLiteral("diagnostics-interval-seconds"),
        QStringLiteral(
            "Write compact server diagnostics at this interval; zero disables diagnostics."),
        QStringLiteral("seconds"), QStringLiteral("30"));
    const QCommandLineOption diagnosticsLogFileOption(
        QStringLiteral("diagnostics-log-file"),
        QStringLiteral(
            "Append server diagnostics to this UTF-8 log file in addition to stdout."),
        QStringLiteral("file"));
    const QCommandLineOption messageAuthTestOnlyOption(
        QStringLiteral("message-auth-test-only"),
        QStringLiteral(
            "Enable auth envelopes backed by legacy HMAC tokens for integration tests only."));
    const QCommandLineOption authenticationIpLimitOption(
        QStringLiteral("auth-ip-limit-per-minute"),
        QStringLiteral(
            "Authentication attempts allowed per source IP per minute. Values other than 20 require --message-auth-test-only."),
        QStringLiteral("count"), QStringLiteral("20"));
    const QCommandLineOption disableBusinessRateLimitsTestOnlyOption(
        QStringLiteral("disable-business-rate-limits-test-only"),
        QStringLiteral(
            "Disable room lookup, room join, and assistance rate limits for isolated load tests. Requires --message-auth-test-only."));
    const QCommandLineOption logtoIssuerOption(
        QStringLiteral("logto-issuer"),
        QStringLiteral(
            "Enable production Logto UserInfo authentication with an HTTPS issuer ending in /oidc."),
        QStringLiteral("url"));
    const QCommandLineOption identityDatabaseOption(
        QStringLiteral("identity-db"),
        QStringLiteral("SQLite database for RemoteC users and devices."),
        QStringLiteral("file"));
    const QCommandLineOption authStorageSelfTestOption(
        QStringLiteral("auth-storage-self-test"),
        QStringLiteral(
            "Test UserInfo classification and SQLite identity ownership, then exit."));
    const QCommandLineOption webhookListenOption(
        QStringLiteral("logto-webhook-listen"),
        QStringLiteral(
            "Loopback address for the internal Logto webhook HTTP listener."),
        QStringLiteral("address"), QStringLiteral("127.0.0.1"));
    const QCommandLineOption webhookPortOption(
        QStringLiteral("logto-webhook-port"),
        QStringLiteral(
            "Internal Logto webhook HTTP port; zero disables the listener."),
        QStringLiteral("port"), QStringLiteral("0"));
    const QCommandLineOption webhookSigningKeyFileOption(
        QStringLiteral("logto-webhook-signing-key-file"),
        QStringLiteral("File containing the Logto webhook signing key."),
        QStringLiteral("file"));
    const QCommandLineOption managementClientIdOption(
        QStringLiteral("logto-management-client-id"),
        QStringLiteral("Server-side Logto M2M application ID."),
        QStringLiteral("id"));
    const QCommandLineOption managementClientSecretFileOption(
        QStringLiteral("logto-management-client-secret-file"),
        QStringLiteral(
            "File containing the server-side Logto M2M application secret."),
        QStringLiteral("file"));

    parser.addOptions({listenOption, portOption, certificateOption,
                       privateKeyOption, secretFileOption,
                       issueTokenOption, tokenLifetimeOption, iceUrlOption,
                       maximumRoomMembersOption,
                       maximumConnectionsOption,
                       maximumUnauthenticatedConnectionsOption,
                       maximumConnectionsPerIpOption,
                       diagnosticsIntervalOption,
                       diagnosticsLogFileOption,
                       messageAuthTestOnlyOption, authenticationIpLimitOption,
                       disableBusinessRateLimitsTestOnlyOption,
                       logtoIssuerOption,
                       identityDatabaseOption, authStorageSelfTestOption,
                       webhookListenOption, webhookPortOption,
                       webhookSigningKeyFileOption,
                       managementClientIdOption,
                       managementClientSecretFileOption});
    parser.process(application);

    if (parser.isSet(authStorageSelfTestOption)) {
        return RunAuthStorageSelfTest();
    }
    const bool usesLogto = parser.isSet(logtoIssuerOption);
    if (usesLogto && parser.isSet(messageAuthTestOnlyOption)) {
        return Fail(QStringLiteral(
            "--logto-issuer and --message-auth-test-only are mutually exclusive."));
    }
    bool authenticationIpLimitOk = false;
    const int authenticationIpLimit =
        parser.value(authenticationIpLimitOption).toInt(
            &authenticationIpLimitOk);
    if (!authenticationIpLimitOk || authenticationIpLimit < 1 ||
        authenticationIpLimit > 100000) {
        return Fail(QStringLiteral(
            "--auth-ip-limit-per-minute must be between 1 and 100000."));
    }
    if (authenticationIpLimit != 20 &&
        !parser.isSet(messageAuthTestOnlyOption)) {
        return Fail(QStringLiteral(
            "A non-default --auth-ip-limit-per-minute requires --message-auth-test-only."));
    }
    if (parser.isSet(disableBusinessRateLimitsTestOnlyOption) &&
        !parser.isSet(messageAuthTestOnlyOption)) {
        return Fail(QStringLiteral(
            "--disable-business-rate-limits-test-only requires --message-auth-test-only."));
    }
    if (usesLogto && !parser.isSet(identityDatabaseOption)) {
        return Fail(QStringLiteral(
            "--identity-db is required with --logto-issuer."));
    }
    bool webhookPortOk = false;
    const uint parsedWebhookPort =
        parser.value(webhookPortOption).toUInt(&webhookPortOk);
    if (!webhookPortOk || parsedWebhookPort > 65535) {
        return Fail(QStringLiteral("--logto-webhook-port is invalid."));
    }
    if ((parsedWebhookPort != 0) !=
        parser.isSet(webhookSigningKeyFileOption)) {
        return Fail(QStringLiteral(
            "--logto-webhook-port and --logto-webhook-signing-key-file must be provided together."));
    }
    if (parsedWebhookPort != 0 && !usesLogto) {
        return Fail(QStringLiteral(
            "The Logto webhook listener requires --logto-issuer."));
    }
    const bool hasManagementClientId =
        parser.isSet(managementClientIdOption);
    const bool hasManagementClientSecret =
        parser.isSet(managementClientSecretFileOption);
    if (hasManagementClientId != hasManagementClientSecret) {
        return Fail(QStringLiteral(
            "--logto-management-client-id and --logto-management-client-secret-file must be provided together."));
    }
    if (hasManagementClientId && !usesLogto) {
        return Fail(QStringLiteral(
            "Logto Management API configuration requires --logto-issuer."));
    }
    const bool needsSecret = !usesLogto || parser.isSet(issueTokenOption);
    if (needsSecret && !parser.isSet(secretFileOption)) {
        return Fail(QStringLiteral("--secret-file is required in legacy mode."));
    }
    QString secretError;
    QByteArray secret;
    if (parser.isSet(secretFileOption)) {
        secret = ReadSecret(parser.value(secretFileOption), &secretError);
    }
    if (!secretError.isEmpty()) {
        return Fail(secretError);
    }
    QByteArray webhookSigningKey;
    if (parser.isSet(webhookSigningKeyFileOption)) {
        webhookSigningKey = ReadSecret(
            parser.value(webhookSigningKeyFileOption), &secretError);
        if (!secretError.isEmpty() || webhookSigningKey.size() < 16) {
            return Fail(secretError.isEmpty()
                            ? QStringLiteral(
                                  "The Logto webhook signing key is too short.")
                            : secretError);
        }
    }
    QByteArray managementClientSecret;
    if (hasManagementClientSecret) {
        managementClientSecret = ReadSecret(
            parser.value(managementClientSecretFileOption), &secretError);
        if (!secretError.isEmpty() || managementClientSecret.size() < 8) {
            return Fail(secretError.isEmpty()
                            ? QStringLiteral(
                                  "The Logto M2M client secret is invalid.")
                            : secretError);
        }
    }

    bool lifetimeOk = false;
    const qint64 lifetime =
        parser.value(tokenLifetimeOption).toLongLong(&lifetimeOk);
    if (!lifetimeOk) {
        return Fail(QStringLiteral("--ttl-seconds is invalid."));
    }

    if (parser.isSet(issueTokenOption)) {
        remote::signaling_server::AccessTokenService tokenService(secret);
        QString issueError;
        const QByteArray token = tokenService.Issue(
            parser.value(issueTokenOption), lifetime, &issueError);
        if (token.isEmpty()) {
            return Fail(issueError);
        }
        QTextStream(stdout) << token << Qt::endl;
        return 0;
    }

    if (!parser.isSet(certificateOption) ||
        !parser.isSet(privateKeyOption)) {
        return Fail(QStringLiteral("--cert and --key are required."));
    }

    QHostAddress listenAddress;
    if (!listenAddress.setAddress(parser.value(listenOption))) {
        return Fail(QStringLiteral("--listen must be a numeric IP address."));
    }
    bool portOk = false;
    const uint parsedPort = parser.value(portOption).toUInt(&portOk);
    if (!portOk || parsedPort > 65535) {
        return Fail(QStringLiteral("--port is invalid."));
    }
    bool maximumRoomMembersOk = false;
    const int maximumRoomMembers =
        parser.value(maximumRoomMembersOption).toInt(&maximumRoomMembersOk);
    if (!maximumRoomMembersOk || maximumRoomMembers < 2 ||
        maximumRoomMembers > 5) {
        return Fail(QStringLiteral(
            "--max-room-members must be between 2 and 5."));
    }
    bool maximumConnectionsOk = false;
    const int maximumConnections =
        parser.value(maximumConnectionsOption).toInt(&maximumConnectionsOk);
    if (!maximumConnectionsOk || maximumConnections < 1 ||
        maximumConnections > 100000) {
        return Fail(QStringLiteral(
            "--max-connections must be between 1 and 100000."));
    }
    bool maximumUnauthenticatedConnectionsOk = false;
    const int maximumUnauthenticatedConnections =
        parser.value(maximumUnauthenticatedConnectionsOption).toInt(
            &maximumUnauthenticatedConnectionsOk);
    if (!maximumUnauthenticatedConnectionsOk ||
        maximumUnauthenticatedConnections < 1 ||
        maximumUnauthenticatedConnections > maximumConnections) {
        return Fail(QStringLiteral(
            "--max-unauthenticated-connections must be between 1 and --max-connections."));
    }
    bool maximumConnectionsPerIpOk = false;
    const int maximumConnectionsPerIp =
        parser.value(maximumConnectionsPerIpOption).toInt(
            &maximumConnectionsPerIpOk);
    if (!maximumConnectionsPerIpOk || maximumConnectionsPerIp < 0 ||
        maximumConnectionsPerIp > maximumConnections) {
        return Fail(QStringLiteral(
            "--max-connections-per-ip must be zero or no greater than --max-connections."));
    }
    bool diagnosticsIntervalOk = false;
    const int diagnosticsIntervalSeconds =
        parser.value(diagnosticsIntervalOption).toInt(
            &diagnosticsIntervalOk);
    if (!diagnosticsIntervalOk || diagnosticsIntervalSeconds < 0 ||
        diagnosticsIntervalSeconds > 3600) {
        return Fail(QStringLiteral(
            "--diagnostics-interval-seconds must be between 0 and 3600."));
    }

    remote::signaling_server::SignalServerConfig config;
    config.listenAddress = listenAddress;
    config.port = static_cast<quint16>(parsedPort);
    config.certificateFile = parser.value(certificateOption);
    config.privateKeyFile = parser.value(privateKeyOption);
    config.tokenSecret = secret;
    config.maximumConnections = maximumConnections;
    config.maximumUnauthenticatedConnections =
        maximumUnauthenticatedConnections;
    config.maximumConnectionsPerIp = maximumConnectionsPerIp;
    config.diagnosticsIntervalMs = diagnosticsIntervalSeconds * 1000;
    config.diagnosticsLogFile =
        parser.value(diagnosticsLogFileOption).trimmed();
    config.authenticationIpLimitPerMinute = authenticationIpLimit;
    config.disableBusinessRateLimitsForTest =
        parser.isSet(disableBusinessRateLimitsTestOnlyOption);
    if (usesLogto) {
        config.authenticationMode =
            remote::signaling_server::SignalServerAuthenticationMode::
                kLogtoUserInfo;
        config.logtoIssuer = QUrl(parser.value(logtoIssuerOption));
        config.identityDatabaseFile = parser.value(identityDatabaseOption);
        if (hasManagementClientId) {
            config.logtoManagementClientId =
                parser.value(managementClientIdOption).trimmed();
            config.logtoManagementClientSecret = managementClientSecret;
        }
        if (parsedWebhookPort != 0) {
            QHostAddress webhookAddress;
            if (!webhookAddress.setAddress(
                    parser.value(webhookListenOption)) ||
                !webhookAddress.isLoopback()) {
                return Fail(QStringLiteral(
                    "--logto-webhook-listen must be a numeric loopback address."));
            }
            config.webhookListenAddress = webhookAddress;
            config.webhookPort = static_cast<quint16>(parsedWebhookPort);
            config.webhookSigningKey = webhookSigningKey;
        }
    } else if (parser.isSet(messageAuthTestOnlyOption)) {
        config.authenticationMode =
            remote::signaling_server::SignalServerAuthenticationMode::
                kLegacyUpgradeBearerWithMessageHmacTestOnly;
    }
    config.iceServerUrls = parser.values(iceUrlOption);
    config.maximumRoomMembers = maximumRoomMembers;

    remote::signaling_server::SignalServer server(std::move(config));
    managementClientSecret.fill('\0');
    managementClientSecret.clear();
    webhookSigningKey.fill('\0');
    webhookSigningKey.clear();
    QString startError;
    if (!server.Start(&startError)) {
        return Fail(startError);
    }
    QObject::connect(&application, &QCoreApplication::aboutToQuit,
                     &application, [&server] { server.Stop(); });

    QTextStream(stdout)
        << "SIGNAL_SERVER_READY=wss://"
        << listenAddress.toString() << ':' << server.ServerPort()
        << "/signaling" << Qt::endl;
    if (server.WebhookPort() != 0) {
        QTextStream(stdout)
            << "LOGTO_WEBHOOK_READY=http://"
            << parser.value(webhookListenOption) << ':'
            << server.WebhookPort() << "/webhooks/logto" << Qt::endl;
    }
    return application.exec();
}
