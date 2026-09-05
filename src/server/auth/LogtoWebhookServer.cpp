// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#include "src/server/auth/LogtoWebhookServer.h"

#include <QCryptographicHash>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMessageAuthenticationCode>

namespace remote::server_auth {
namespace {

constexpr qsizetype kMaximumWebhookBytes = 64 * 1024;

void SetError(QString* errorMessage, const QString& message) {
    if (errorMessage) {
        *errorMessage = message;
    }
}

QHttpServerResponse JsonResponse(
    QHttpServerResponse::StatusCode status,
    const QString& code) {
    return QHttpServerResponse(
        QJsonObject{{QStringLiteral("ok"),
                     status == QHttpServerResponse::StatusCode::Ok},
                    {QStringLiteral("code"), code}},
        status);
}

}  // namespace

LogtoWebhookServer::LogtoWebhookServer() {
    httpServer_.route(
        QStringLiteral("/webhooks/logto"),
        QHttpServerRequest::Method::Post,
        &httpServer_,
        [this](const QHttpServerRequest& request) {
            return HandleRequest(request);
        });
}

LogtoWebhookServer::~LogtoWebhookServer() {
    Stop();
}

bool LogtoWebhookServer::Start(
    const QHostAddress& listenAddress,
    quint16 port,
    QByteArray signingKey,
    UserDeletedHandler handler,
    QString* errorMessage) {
    Stop();
    signingKey = signingKey.trimmed();
    if (listenAddress.isNull() || port == 0 || signingKey.size() < 16 ||
        !handler) {
        SetError(errorMessage,
                 QStringLiteral("The Logto webhook configuration is invalid."));
        return false;
    }
    if (!listenAddress.isLoopback()) {
        SetError(errorMessage,
                 QStringLiteral(
                     "The Logto webhook listener must bind to a loopback address."));
        return false;
    }

    signingKey_ = std::move(signingKey);
    userDeletedHandler_ = std::move(handler);
    if (!tcpServer_.listen(listenAddress, port)) {
        SetError(errorMessage, tcpServer_.errorString());
        Stop();
        return false;
    }
    if (!httpServer_.bind(&tcpServer_)) {
        SetError(errorMessage,
                 QStringLiteral("Qt HttpServer could not bind the webhook socket."));
        Stop();
        return false;
    }
    return true;
}

void LogtoWebhookServer::Stop() {
    tcpServer_.close();
    signingKey_.fill('\0');
    signingKey_.clear();
    userDeletedHandler_ = {};
}

bool LogtoWebhookServer::IsListening() const {
    return tcpServer_.isListening();
}

quint16 LogtoWebhookServer::ServerPort() const {
    return tcpServer_.serverPort();
}

bool LogtoWebhookServer::ConstantTimeEquals(
    const QByteArray& first,
    const QByteArray& second) {
    if (first.size() != second.size()) {
        return false;
    }
    unsigned char difference = 0;
    for (qsizetype index = 0; index < first.size(); ++index) {
        difference |= static_cast<unsigned char>(first.at(index)) ^
                      static_cast<unsigned char>(second.at(index));
    }
    return difference == 0;
}

bool LogtoWebhookServer::VerifySignatureForTesting(
    const QByteArray& body,
    const QByteArray& signingKey,
    const QByteArray& signature) {
    const QByteArray supplied = signature.trimmed().toLower();
    if (supplied.size() != 64) {
        return false;
    }
    const QByteArray expected = QMessageAuthenticationCode::hash(
        body, signingKey, QCryptographicHash::Sha256).toHex();
    return ConstantTimeEquals(expected, supplied);
}

QHttpServerResponse LogtoWebhookServer::HandleRequest(
    const QHttpServerRequest& request) {
    const QByteArray body = request.body();
    if (body.size() > kMaximumWebhookBytes) {
        return JsonResponse(QHttpServerResponse::StatusCode::PayloadTooLarge,
                            QStringLiteral("payload_too_large"));
    }
    if (!VerifySignatureForTesting(
            body, signingKey_,
            request.value(QByteArrayLiteral("logto-signature-sha-256")))) {
        return JsonResponse(QHttpServerResponse::StatusCode::Unauthorized,
                            QStringLiteral("invalid_signature"));
    }

    QJsonParseError parseError;
    const QJsonDocument document = QJsonDocument::fromJson(body, &parseError);
    if (parseError.error != QJsonParseError::NoError ||
        !document.isObject()) {
        return JsonResponse(QHttpServerResponse::StatusCode::BadRequest,
                            QStringLiteral("invalid_json"));
    }
    const QJsonObject payload = document.object();
    if (payload.value(QStringLiteral("event")).toString() !=
        QStringLiteral("User.Deleted")) {
        return JsonResponse(QHttpServerResponse::StatusCode::Ok,
                            QStringLiteral("event_ignored"));
    }
    const QString subject =
        payload.value(QStringLiteral("params"))
            .toObject()
            .value(QStringLiteral("userId"))
            .toString()
            .trimmed();
    if (subject.isEmpty() || subject.size() > 256) {
        return JsonResponse(QHttpServerResponse::StatusCode::BadRequest,
                            QStringLiteral("missing_user_id"));
    }
    QString deletionError;
    if (!userDeletedHandler_ ||
        !userDeletedHandler_(subject, &deletionError)) {
        return JsonResponse(QHttpServerResponse::StatusCode::ServiceUnavailable,
                            QStringLiteral("cleanup_failed"));
    }
    return JsonResponse(QHttpServerResponse::StatusCode::Ok,
                        QStringLiteral("user_deleted"));
}

}  // namespace remote::server_auth
