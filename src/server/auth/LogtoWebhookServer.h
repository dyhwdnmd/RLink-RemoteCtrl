// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#pragma once

#include <QByteArray>
#include <QHostAddress>
#include <QHttpServer>
#include <QTcpServer>

#include <functional>

namespace remote::server_auth {

class LogtoWebhookServer final {
public:
    using UserDeletedHandler =
        std::function<bool(const QString& subject, QString* errorMessage)>;

    LogtoWebhookServer();
    ~LogtoWebhookServer();

    LogtoWebhookServer(const LogtoWebhookServer&) = delete;
    LogtoWebhookServer& operator=(const LogtoWebhookServer&) = delete;

    bool Start(const QHostAddress& listenAddress,
               quint16 port,
               QByteArray signingKey,
               UserDeletedHandler handler,
               QString* errorMessage = nullptr);
    void Stop();
    bool IsListening() const;
    quint16 ServerPort() const;

    static bool VerifySignatureForTesting(const QByteArray& body,
                                          const QByteArray& signingKey,
                                          const QByteArray& signature);

private:
    QHttpServerResponse HandleRequest(const QHttpServerRequest& request);
    static bool ConstantTimeEquals(const QByteArray& first,
                                   const QByteArray& second);

    QTcpServer tcpServer_;
    QHttpServer httpServer_;
    QByteArray signingKey_;
    UserDeletedHandler userDeletedHandler_;
};

}  // namespace remote::server_auth
