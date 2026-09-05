// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#pragma once

#include <QByteArray>
#include <QNetworkReply>
#include <QObject>
#include <QQueue>
#include <QString>
#include <QUrl>

#include <functional>
#include <memory>

QT_BEGIN_NAMESPACE
class QNetworkAccessManager;
QT_END_NAMESPACE

namespace remote::server_auth {

enum class UserDeletionStatus {
    kDeleted,
    kNotFound,
    kUnauthorized,
    kServiceUnavailable,
    kRejected,
    kCancelled,
};

struct UserDeletionResult {
    UserDeletionStatus status = UserDeletionStatus::kRejected;
    QString code;
    QString message;
    bool retryable = false;

    bool deleted() const {
        return status == UserDeletionStatus::kDeleted ||
               status == UserDeletionStatus::kNotFound;
    }
};

class LogtoManagementClient final : public QObject {
public:
    using Completion = std::function<void(UserDeletionResult)>;

    explicit LogtoManagementClient(QObject* parent = nullptr);
    ~LogtoManagementClient() override;

    bool Configure(const QUrl& issuer,
                   QString clientId,
                   QByteArray clientSecret,
                   int timeoutMs,
                   QString* errorMessage = nullptr);
    bool IsConfigured() const;
    void DeleteUser(const QString& subject, Completion completion);
    void CancelAll();

private:
    struct PendingDeletion {
        QString subject;
        Completion completion;
    };

    void StartNext();
    void RequestAccessToken();
    void FinishTokenRequest(QNetworkReply* reply);
    void StartDeleteRequest();
    void FinishDeleteRequest(QNetworkReply* reply);
    void CompleteCurrent(UserDeletionResult result);

    std::unique_ptr<QNetworkAccessManager> networkManager_;
    QQueue<PendingDeletion> queue_;
    std::unique_ptr<PendingDeletion> current_;
    QNetworkReply* activeReply_ = nullptr;
    QUrl tokenEndpoint_;
    QUrl managementEndpoint_;
    QString clientId_;
    QByteArray clientSecret_;
    QByteArray accessToken_;
    qint64 accessTokenExpiresAtSeconds_ = 0;
    int timeoutMs_ = 10000;
    bool cancelling_ = false;
};

}  // namespace remote::server_auth
