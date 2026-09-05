// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#pragma once

#include <QObject>
#include <QPointer>
#include <QString>
#include <QUrl>

#include <functional>

QT_BEGIN_NAMESPACE
class QNetworkAccessManager;
class QNetworkReply;
QT_END_NAMESPACE

namespace remote::auth {

struct OidcMetadata {
    QUrl issuer;
    QUrl authorizationEndpoint;
    QUrl tokenEndpoint;
    QUrl userInfoEndpoint;
};

class OidcDiscovery final : public QObject {
public:
    using Completion =
        std::function<void(bool, OidcMetadata, QString, bool)>;

    explicit OidcDiscovery(
        QNetworkAccessManager* networkManager,
        QObject* parent = nullptr);
    ~OidcDiscovery() override;

    void Discover(
        const QUrl& issuer,
        int timeoutMs,
        Completion completion);
    void Cancel();

private:
    void Finish(
        bool success,
        OidcMetadata metadata,
        QString errorMessage,
        bool transient);

    QNetworkAccessManager* networkManager_ = nullptr;
    QPointer<QNetworkReply> activeReply_;
    Completion completion_;
    QUrl expectedIssuer_;
};

}  // namespace remote::auth
