// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#pragma once

#include <QByteArray>
#include <QHash>
#include <QNetworkReply>
#include <QObject>
#include <QUrl>

#include <functional>
#include <memory>

#include "src/server/auth/AuthTypes.h"

QT_BEGIN_NAMESPACE
class QNetworkAccessManager;
QT_END_NAMESPACE

namespace remote::server_auth {

class LogtoUserInfoClient final : public QObject {
public:
    using Completion = std::function<void(UserInfoResult)>;

    explicit LogtoUserInfoClient(QObject* parent = nullptr);
    ~LogtoUserInfoClient() override;

    bool Configure(const QUrl& issuer,
                   int timeoutMs,
                   QString* errorMessage = nullptr);
    QNetworkReply* Fetch(const QByteArray& accessToken,
                         Completion completion);
    void Cancel(QNetworkReply* reply);
    void CancelAll();

    QUrl userInfoEndpoint() const { return userInfoEndpoint_; }

    static UserInfoResult ParseResponseForTesting(
        int httpStatus,
        const QByteArray& body,
        QNetworkReply::NetworkError networkError =
            QNetworkReply::NoError,
        bool timedOut = false,
        bool tooLarge = false);

private:
    struct PendingRequest;

    static UserInfoResult ParseResponse(
        int httpStatus,
        const QByteArray& body,
        QNetworkReply::NetworkError networkError,
        bool timedOut,
        bool tooLarge);
    void Finish(QNetworkReply* reply);

    std::unique_ptr<QNetworkAccessManager> networkManager_;
    QHash<QNetworkReply*, std::shared_ptr<PendingRequest>> pending_;
    QUrl userInfoEndpoint_;
    int timeoutMs_ = 10000;
};

}  // namespace remote::server_auth
