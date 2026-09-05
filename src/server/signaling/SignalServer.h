// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#pragma once

#include <memory>

#include <QByteArray>
#include <QHostAddress>
#include <QString>
#include <QStringList>
#include <QUrl>

namespace remote::signaling_server {

enum class SignalServerAuthenticationMode {
    kLegacyUpgradeBearer,
    // Enables connection-level auth envelopes backed by the legacy HMAC
    // verifier strictly for integration tests. Production Logto UserInfo
    // authentication is added as a separate mode in phase 5.
    kLegacyUpgradeBearerWithMessageHmacTestOnly,
    // Production mode: accepts an opaque Logto Access Token in the first
    // signaling auth envelope and validates it once through /oidc/me.
    kLogtoUserInfo,
};

struct SignalServerConfig {
    QHostAddress listenAddress = QHostAddress::LocalHost;
    quint16 port = 9443;
    QString certificateFile;
    QString privateKeyFile;
    QByteArray tokenSecret;
    SignalServerAuthenticationMode authenticationMode =
        SignalServerAuthenticationMode::kLegacyUpgradeBearer;
    QStringList iceServerUrls;
    int maximumPendingConnections = 128;
    int maximumConnections = 5000;
    int maximumUnauthenticatedConnections = 512;
    int maximumConnectionsPerIp = 0;
    int authenticationIpLimitPerMinute = 20;
    bool disableBusinessRateLimitsForTest = false;
    int diagnosticsIntervalMs = 0;
    QString diagnosticsLogFile;
    int authenticationTimeoutMs = 10000;
    QUrl logtoIssuer;
    QString identityDatabaseFile;
    int userInfoTimeoutMs = 10000;
    QString logtoManagementClientId;
    QByteArray logtoManagementClientSecret;
    int managementTimeoutMs = 10000;
    QHostAddress webhookListenAddress = QHostAddress::LocalHost;
    quint16 webhookPort = 0;
    QByteArray webhookSigningKey;
    int clientIdleTimeoutMs = 45000;
    int pendingSessionTimeoutMs = 20000;
    int sessionRecoveryWindowMs = 60000;
    int maximumRoomMembers = 5;
};

class SignalServer final {
public:
    explicit SignalServer(SignalServerConfig config);
    ~SignalServer();

    SignalServer(const SignalServer&) = delete;
    SignalServer& operator=(const SignalServer&) = delete;

    bool Start(QString* error);
    void Stop();
    bool IsListening() const;
    quint16 ServerPort() const;
    quint16 WebhookPort() const;

private:
    class Impl;
    std::unique_ptr<Impl> impl_;
};

}  // namespace remote::signaling_server
