// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#include "src/server/auth/LogtoManagementClient.h"

#include <QDateTime>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QUrlQuery>

#include <utility>

namespace remote::server_auth {
namespace {

constexpr qsizetype kMaximumResponseBytes = 64 * 1024;

void SetError(QString* errorMessage, const QString& message) {
    if (errorMessage) {
        *errorMessage = message;
    }
}

UserDeletionResult Failure(UserDeletionStatus status,
                           QString code,
                           QString message,
                           bool retryable) {
    UserDeletionResult result;
    result.status = status;
    result.code = std::move(code);
    result.message = std::move(message);
    result.retryable = retryable;
    return result;
}

}  // namespace

LogtoManagementClient::LogtoManagementClient(QObject* parent)
    : QObject(parent),
      networkManager_(std::make_unique<QNetworkAccessManager>(this)) {}

LogtoManagementClient::~LogtoManagementClient() {
    CancelAll();
    clientSecret_.fill('\0');
}

bool LogtoManagementClient::Configure(
    const QUrl& issuer,
    QString clientId,
    QByteArray clientSecret,
    int timeoutMs,
    QString* errorMessage) {
    QString issuerPath = issuer.path();
    while (issuerPath.endsWith(QLatin1Char('/'))) {
        issuerPath.chop(1);
    }
    if (!issuer.isValid() || issuer.scheme() != QStringLiteral("https") ||
        issuer.host().isEmpty() || !issuer.userInfo().isEmpty() ||
        !issuer.query().isEmpty() || !issuer.fragment().isEmpty() ||
        !issuerPath.endsWith(QStringLiteral("/oidc")) ||
        clientId.trimmed().isEmpty() || clientId.size() > 256 ||
        clientSecret.size() < 8 || clientSecret.size() > 4096 ||
        timeoutMs < 1000 || timeoutMs > 60000) {
        SetError(errorMessage,
                 QStringLiteral("The Logto Management API configuration is invalid."));
        return false;
    }

    CancelAll();
    clientSecret_.fill('\0');
    tokenEndpoint_ = issuer;
    tokenEndpoint_.setPath(issuerPath + QStringLiteral("/token"));
    managementEndpoint_ = issuer;
    managementEndpoint_.setPath(QStringLiteral("/api"));
    clientId_ = clientId.trimmed();
    clientSecret_ = std::move(clientSecret);
    timeoutMs_ = timeoutMs;
    return true;
}

bool LogtoManagementClient::IsConfigured() const {
    return !tokenEndpoint_.isEmpty() && !managementEndpoint_.isEmpty() &&
           !clientId_.isEmpty() && !clientSecret_.isEmpty();
}

void LogtoManagementClient::DeleteUser(
    const QString& subject,
    Completion completion) {
    const QString cleanSubject = subject.trimmed();
    if (!IsConfigured() || cleanSubject.isEmpty() ||
        cleanSubject.size() > 256 || !completion) {
        if (completion) {
            completion(Failure(
                UserDeletionStatus::kRejected,
                QStringLiteral("account_deletion_unavailable"),
                QStringLiteral("Logto account deletion is not configured."),
                false));
        }
        return;
    }
    queue_.enqueue(PendingDeletion{cleanSubject, std::move(completion)});
    StartNext();
}

void LogtoManagementClient::CancelAll() {
    cancelling_ = true;
    if (activeReply_) {
        activeReply_->abort();
    }
    if (current_ && current_->completion) {
        current_->completion(Failure(
            UserDeletionStatus::kCancelled,
            QStringLiteral("account_deletion_cancelled"),
            QStringLiteral("Account deletion was cancelled."), false));
    }
    current_.reset();
    while (!queue_.isEmpty()) {
        PendingDeletion pending = queue_.dequeue();
        if (pending.completion) {
            pending.completion(Failure(
                UserDeletionStatus::kCancelled,
                QStringLiteral("account_deletion_cancelled"),
                QStringLiteral("Account deletion was cancelled."), false));
        }
    }
    activeReply_ = nullptr;
    cancelling_ = false;
}

void LogtoManagementClient::StartNext() {
    if (current_ || queue_.isEmpty() || activeReply_) {
        return;
    }
    current_ = std::make_unique<PendingDeletion>(queue_.dequeue());
    const qint64 now = QDateTime::currentSecsSinceEpoch();
    if (!accessToken_.isEmpty() && accessTokenExpiresAtSeconds_ > now + 30) {
        StartDeleteRequest();
    } else {
        RequestAccessToken();
    }
}

void LogtoManagementClient::RequestAccessToken() {
    QNetworkRequest request(tokenEndpoint_);
    request.setRawHeader("Accept", "application/json");
    request.setRawHeader("Content-Type", "application/x-www-form-urlencoded");
    request.setRawHeader(
        "Authorization",
        QByteArrayLiteral("Basic ") +
            (clientId_.toUtf8() + ':' + clientSecret_).toBase64());
    request.setAttribute(QNetworkRequest::RedirectPolicyAttribute,
                         QNetworkRequest::ManualRedirectPolicy);
    request.setTransferTimeout(timeoutMs_);

    QUrlQuery form;
    form.addQueryItem(QStringLiteral("grant_type"),
                      QStringLiteral("client_credentials"));
    form.addQueryItem(QStringLiteral("resource"),
                      managementEndpoint_.toString());
    form.addQueryItem(QStringLiteral("scope"), QStringLiteral("all"));
    activeReply_ = networkManager_->post(
        request, form.query(QUrl::FullyEncoded).toUtf8());
    QNetworkReply* reply = activeReply_;
    QObject::connect(reply, &QNetworkReply::finished, this,
                     [this, reply] { FinishTokenRequest(reply); });
}

void LogtoManagementClient::FinishTokenRequest(QNetworkReply* reply) {
    if (reply != activeReply_) {
        reply->deleteLater();
        return;
    }
    activeReply_ = nullptr;
    const int status = reply->attribute(
        QNetworkRequest::HttpStatusCodeAttribute).toInt();
    const QNetworkReply::NetworkError networkError = reply->error();
    const QByteArray body = reply->read(kMaximumResponseBytes + 1);
    reply->deleteLater();
    if (cancelling_ || !current_) {
        return;
    }
    if (status != 200 || networkError != QNetworkReply::NoError ||
        body.size() > kMaximumResponseBytes) {
        CompleteCurrent(Failure(
            (status == 401 || status == 403)
                ? UserDeletionStatus::kUnauthorized
                : UserDeletionStatus::kServiceUnavailable,
            QStringLiteral("management_token_failed"),
            QStringLiteral("Logto Management API authorization failed."),
            status == 429 || status >= 500 || status <= 0));
        return;
    }
    const QJsonObject object = QJsonDocument::fromJson(body).object();
    accessToken_ =
        object.value(QStringLiteral("access_token")).toString().toUtf8();
    const int expiresIn =
        object.value(QStringLiteral("expires_in")).toInt(3600);
    if (accessToken_.isEmpty() || accessToken_.size() > 16 * 1024 ||
        expiresIn <= 0) {
        accessToken_.clear();
        CompleteCurrent(Failure(
            UserDeletionStatus::kRejected,
            QStringLiteral("management_token_invalid"),
            QStringLiteral("Logto returned an invalid management token."),
            false));
        return;
    }
    accessTokenExpiresAtSeconds_ =
        QDateTime::currentSecsSinceEpoch() + expiresIn;
    StartDeleteRequest();
}

void LogtoManagementClient::StartDeleteRequest() {
    QUrl endpoint = managementEndpoint_;
    endpoint.setPath(
        managementEndpoint_.path() + QStringLiteral("/users/") +
        QString::fromLatin1(QUrl::toPercentEncoding(current_->subject)));
    QNetworkRequest request(endpoint);
    request.setRawHeader("Accept", "application/json");
    request.setRawHeader("Authorization",
                         QByteArrayLiteral("Bearer ") + accessToken_);
    request.setAttribute(QNetworkRequest::RedirectPolicyAttribute,
                         QNetworkRequest::ManualRedirectPolicy);
    request.setTransferTimeout(timeoutMs_);
    activeReply_ = networkManager_->deleteResource(request);
    QNetworkReply* reply = activeReply_;
    QObject::connect(reply, &QNetworkReply::finished, this,
                     [this, reply] { FinishDeleteRequest(reply); });
}

void LogtoManagementClient::FinishDeleteRequest(QNetworkReply* reply) {
    if (reply != activeReply_) {
        reply->deleteLater();
        return;
    }
    activeReply_ = nullptr;
    const int status = reply->attribute(
        QNetworkRequest::HttpStatusCodeAttribute).toInt();
    const QNetworkReply::NetworkError networkError = reply->error();
    reply->read(kMaximumResponseBytes + 1);
    reply->deleteLater();
    if (cancelling_ || !current_) {
        return;
    }
    if (status == 200 || status == 204) {
        UserDeletionResult result;
        result.status = UserDeletionStatus::kDeleted;
        CompleteCurrent(std::move(result));
    } else if (status == 404) {
        UserDeletionResult result;
        result.status = UserDeletionStatus::kNotFound;
        CompleteCurrent(std::move(result));
    } else if (status == 401 || status == 403) {
        accessToken_.clear();
        accessTokenExpiresAtSeconds_ = 0;
        CompleteCurrent(Failure(
            UserDeletionStatus::kUnauthorized,
            QStringLiteral("management_access_denied"),
            QStringLiteral("Logto denied the account deletion request."),
            false));
    } else if (networkError != QNetworkReply::NoError || status == 429 ||
               status >= 500 || status <= 0) {
        CompleteCurrent(Failure(
            UserDeletionStatus::kServiceUnavailable,
            QStringLiteral("management_service_unavailable"),
            QStringLiteral("Logto account deletion is temporarily unavailable."),
            true));
    } else {
        CompleteCurrent(Failure(
            UserDeletionStatus::kRejected,
            QStringLiteral("management_request_rejected"),
            QStringLiteral("Logto rejected the account deletion request."),
            false));
    }
}

void LogtoManagementClient::CompleteCurrent(UserDeletionResult result) {
    Completion completion;
    if (current_) {
        completion = std::move(current_->completion);
        current_.reset();
    }
    if (completion) {
        completion(std::move(result));
    }
    StartNext();
}

}  // namespace remote::server_auth
