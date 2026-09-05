// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#include "src/server/auth/LogtoUserInfoClient.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QTimer>

#include <utility>

namespace remote::server_auth {
namespace {

constexpr qsizetype kMaximumUserInfoBytes = 64 * 1024;

void SetError(QString* errorMessage, const QString& message) {
    if (errorMessage) {
        *errorMessage = message;
    }
}

UserInfoResult Failure(UserInfoStatus status,
                       QString code,
                       QString message,
                       bool retryable) {
    UserInfoResult result;
    result.status = status;
    result.code = std::move(code);
    result.message = std::move(message);
    result.retryable = retryable;
    return result;
}

}  // namespace

struct LogtoUserInfoClient::PendingRequest {
    Completion completion;
    QByteArray body;
    bool timedOut = false;
    bool tooLarge = false;
    bool cancelled = false;
};

LogtoUserInfoClient::LogtoUserInfoClient(QObject* parent)
    : QObject(parent),
      networkManager_(std::make_unique<QNetworkAccessManager>(this)) {}

LogtoUserInfoClient::~LogtoUserInfoClient() {
    CancelAll();
}

bool LogtoUserInfoClient::Configure(
    const QUrl& issuer,
    int timeoutMs,
    QString* errorMessage) {
    if (!issuer.isValid() || issuer.scheme() != QStringLiteral("https") ||
        issuer.host().isEmpty() || !issuer.userInfo().isEmpty() ||
        !issuer.query().isEmpty() || !issuer.fragment().isEmpty()) {
        SetError(errorMessage,
                 QStringLiteral("Logto issuer must be a clean HTTPS URL."));
        return false;
    }
    if (timeoutMs < 1000 || timeoutMs > 60000) {
        SetError(errorMessage,
                 QStringLiteral("Logto UserInfo timeout is invalid."));
        return false;
    }

    QString path = issuer.path();
    while (path.endsWith(QLatin1Char('/'))) {
        path.chop(1);
    }
    if (!path.endsWith(QStringLiteral("/oidc"))) {
        SetError(errorMessage,
                 QStringLiteral(
                     "Logto issuer path must end with /oidc."));
        return false;
    }
    QUrl endpoint = issuer;
    endpoint.setPath(path + QStringLiteral("/me"));
    if (!endpoint.isValid() || endpoint.scheme() != QStringLiteral("https")) {
        SetError(errorMessage,
                 QStringLiteral("Logto UserInfo endpoint is invalid."));
        return false;
    }

    CancelAll();
    userInfoEndpoint_ = std::move(endpoint);
    timeoutMs_ = timeoutMs;
    return true;
}

QNetworkReply* LogtoUserInfoClient::Fetch(
    const QByteArray& accessToken,
    Completion completion) {
    if (userInfoEndpoint_.isEmpty() || accessToken.isEmpty() ||
        accessToken.size() > 16 * 1024 || !completion) {
        if (completion) {
            completion(Failure(
                UserInfoStatus::kInvalidResponse,
                QStringLiteral("userinfo_request_invalid"),
                QStringLiteral("The UserInfo request is not configured."),
                false));
        }
        return nullptr;
    }

    QNetworkRequest request(userInfoEndpoint_);
    request.setRawHeader("Accept", "application/json");
    request.setRawHeader("Authorization", QByteArray("Bearer ") + accessToken);
    request.setAttribute(
        QNetworkRequest::RedirectPolicyAttribute,
        QNetworkRequest::ManualRedirectPolicy);
    request.setTransferTimeout(timeoutMs_);

    QNetworkReply* reply = networkManager_->get(request);
    auto pending = std::make_shared<PendingRequest>();
    pending->completion = std::move(completion);
    pending_.insert(reply, pending);

    auto* timeout = new QTimer(reply);
    timeout->setSingleShot(true);
    QObject::connect(timeout, &QTimer::timeout, reply,
                     [reply, pending] {
                         pending->timedOut = true;
                         reply->abort();
                     });
    timeout->start(timeoutMs_);

    QObject::connect(reply, &QIODevice::readyRead, reply,
                     [reply, pending] {
                         if (pending->tooLarge || pending->cancelled) {
                             reply->readAll();
                             return;
                         }
                         const qsizetype remaining =
                             kMaximumUserInfoBytes - pending->body.size();
                         pending->body.append(reply->read(remaining + 1));
                         if (pending->body.size() >
                             kMaximumUserInfoBytes) {
                             pending->tooLarge = true;
                             reply->abort();
                         }
                     });
    QObject::connect(reply, &QNetworkReply::finished, this,
                     [this, reply] { Finish(reply); });
    return reply;
}

void LogtoUserInfoClient::Cancel(QNetworkReply* reply) {
    const auto it = pending_.find(reply);
    if (it == pending_.end()) {
        return;
    }
    (*it)->cancelled = true;
    (*it)->completion = {};
    reply->abort();
}

void LogtoUserInfoClient::CancelAll() {
    const auto replies = pending_.keys();
    for (QNetworkReply* reply : replies) {
        Cancel(reply);
    }
}

UserInfoResult LogtoUserInfoClient::ParseResponseForTesting(
    int httpStatus,
    const QByteArray& body,
    QNetworkReply::NetworkError networkError,
    bool timedOut,
    bool tooLarge) {
    return ParseResponse(
        httpStatus, body, networkError, timedOut, tooLarge);
}

UserInfoResult LogtoUserInfoClient::ParseResponse(
    int httpStatus,
    const QByteArray& body,
    QNetworkReply::NetworkError networkError,
    bool timedOut,
    bool tooLarge) {
    if (tooLarge) {
        return Failure(
            UserInfoStatus::kInvalidResponse,
            QStringLiteral("userinfo_response_too_large"),
            QStringLiteral("Logto returned an oversized UserInfo response."),
            false);
    }
    if (httpStatus == 401 || httpStatus == 403) {
        return Failure(
            UserInfoStatus::kInvalidToken,
            QStringLiteral("invalid_token"),
            QStringLiteral("Logto rejected the Access Token."), false);
    }
    if (timedOut || httpStatus == 429 ||
        (httpStatus >= 500 && httpStatus <= 599) ||
        (networkError != QNetworkReply::NoError && httpStatus <= 0)) {
        return Failure(
            UserInfoStatus::kServiceUnavailable,
            QStringLiteral("authentication_service_unavailable"),
            QStringLiteral("Logto UserInfo is temporarily unavailable."),
            true);
    }
    if (httpStatus != 200 || networkError != QNetworkReply::NoError) {
        return Failure(
            UserInfoStatus::kInvalidResponse,
            QStringLiteral("userinfo_request_rejected"),
            QStringLiteral("Logto returned an unexpected UserInfo response."),
            false);
    }
    if (body.isEmpty() || body.size() > kMaximumUserInfoBytes) {
        return Failure(
            UserInfoStatus::kInvalidResponse,
            QStringLiteral("userinfo_response_invalid"),
            QStringLiteral("Logto returned an invalid UserInfo response."),
            false);
    }

    QJsonParseError parseError;
    const QJsonDocument document =
        QJsonDocument::fromJson(body, &parseError);
    if (parseError.error != QJsonParseError::NoError ||
        !document.isObject()) {
        return Failure(
            UserInfoStatus::kInvalidResponse,
            QStringLiteral("userinfo_response_invalid"),
            QStringLiteral("Logto returned malformed UserInfo JSON."),
            false);
    }
    const QJsonObject object = document.object();
    UserInfoClaims claims;
    claims.subject =
        object.value(QStringLiteral("sub")).toString().trimmed();
    claims.username =
        object.value(QStringLiteral("username")).toString().trimmed();
    claims.displayName =
        object.value(QStringLiteral("name")).toString().trimmed();
    if (claims.displayName.isEmpty()) {
        claims.displayName = claims.username;
    }
    claims.email =
        object.value(QStringLiteral("email")).toString().trimmed();
    if (claims.subject.isEmpty() || claims.subject.size() > 256 ||
        claims.username.size() > 256 || claims.displayName.size() > 256 ||
        claims.email.size() > 320) {
        return Failure(
            UserInfoStatus::kInvalidResponse,
            QStringLiteral("userinfo_claims_invalid"),
            QStringLiteral("Logto UserInfo claims are invalid."), false);
    }

    UserInfoResult result;
    result.status = UserInfoStatus::kAuthenticated;
    result.claims = std::move(claims);
    return result;
}

void LogtoUserInfoClient::Finish(QNetworkReply* reply) {
    const auto it = pending_.find(reply);
    if (it == pending_.end()) {
        reply->deleteLater();
        return;
    }
    const std::shared_ptr<PendingRequest> pending = *it;
    pending_.erase(it);
    if (!pending->tooLarge && !pending->cancelled) {
        const qsizetype remaining =
            kMaximumUserInfoBytes - pending->body.size();
        pending->body.append(reply->read(remaining + 1));
        if (pending->body.size() > kMaximumUserInfoBytes) {
            pending->tooLarge = true;
        }
    }
    const int httpStatus = reply->attribute(
        QNetworkRequest::HttpStatusCodeAttribute).toInt();
    const QNetworkReply::NetworkError networkError = reply->error();
    Completion completion = std::move(pending->completion);
    const bool cancelled = pending->cancelled;
    UserInfoResult result;
    if (cancelled) {
        result.status = UserInfoStatus::kCancelled;
    } else {
        result = ParseResponse(httpStatus, pending->body, networkError,
                               pending->timedOut, pending->tooLarge);
    }
    reply->deleteLater();
    if (completion) {
        completion(std::move(result));
    }
}

}  // namespace remote::server_auth
