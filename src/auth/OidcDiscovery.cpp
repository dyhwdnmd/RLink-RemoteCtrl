// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#include "src/auth/OidcDiscovery.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>

#include <chrono>
#include <utility>

namespace remote::auth {
namespace {

constexpr qint64 kMaximumDiscoveryBytes = 1024 * 1024;

QUrl NormalizeIssuer(QUrl issuer) {
    QString path = issuer.path();
    while (path.size() > 1 && path.endsWith(QLatin1Char('/'))) {
        path.chop(1);
    }
    issuer.setPath(path);
    return issuer;
}

bool IsSecureEndpoint(const QUrl& url) {
    return url.isValid() && url.scheme() == QStringLiteral("https") &&
           !url.host().isEmpty() && !url.hasFragment();
}

bool StringArrayContains(const QJsonValue& value, const QString& expected) {
    if (!value.isArray()) {
        return false;
    }
    for (const QJsonValue& item : value.toArray()) {
        if (item.toString() == expected) {
            return true;
        }
    }
    return false;
}

}  // namespace

OidcDiscovery::OidcDiscovery(
    QNetworkAccessManager* networkManager,
    QObject* parent)
    : QObject(parent), networkManager_(networkManager) {}

OidcDiscovery::~OidcDiscovery() {
    Cancel();
}

void OidcDiscovery::Discover(
    const QUrl& issuer,
    int timeoutMs,
    Completion completion) {
    Cancel();
    completion_ = std::move(completion);
    expectedIssuer_ = NormalizeIssuer(issuer);
    if (!networkManager_) {
        Finish(false, {},
               QStringLiteral("OIDC discovery has no network manager."),
               false);
        return;
    }

    QString issuerText = expectedIssuer_.toString(QUrl::FullyEncoded);
    while (issuerText.endsWith(QLatin1Char('/'))) {
        issuerText.chop(1);
    }
    const QUrl discoveryUrl(
        issuerText + QStringLiteral("/.well-known/openid-configuration"));
    QNetworkRequest request(discoveryUrl);
    request.setRawHeader("Accept", "application/json");
    request.setAttribute(
        QNetworkRequest::RedirectPolicyAttribute,
        QNetworkRequest::NoLessSafeRedirectPolicy);
    request.setTransferTimeout(std::chrono::milliseconds(timeoutMs));

    QNetworkReply* reply = networkManager_->get(request);
    activeReply_ = reply;
    connect(reply, &QNetworkReply::downloadProgress, this,
            [reply](qint64 received, qint64) {
                if (received > kMaximumDiscoveryBytes) {
                    reply->abort();
                }
            });
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        if (activeReply_ != reply) {
            reply->deleteLater();
            return;
        }
        activeReply_.clear();
        const auto networkError = reply->error();
        const int status = reply->attribute(
            QNetworkRequest::HttpStatusCodeAttribute).toInt();
        QByteArray body = reply->readAll();
        const QString networkMessage = reply->errorString();
        reply->deleteLater();

        if (networkError != QNetworkReply::NoError) {
            Finish(false, {},
                   QStringLiteral("OIDC discovery request failed: %1")
                       .arg(networkMessage),
                   true);
            return;
        }
        if (status != 200 || body.size() > kMaximumDiscoveryBytes) {
            Finish(false, {},
                   QStringLiteral("OIDC discovery returned HTTP %1 or an "
                                  "oversized response.")
                       .arg(status),
                   status == 429 || status >= 500);
            return;
        }

        QJsonParseError parseError{};
        const QJsonDocument document =
            QJsonDocument::fromJson(body, &parseError);
        body.fill('\0');
        body.clear();
        if (parseError.error != QJsonParseError::NoError ||
            !document.isObject()) {
            Finish(false, {},
                   QStringLiteral("OIDC discovery returned invalid JSON."),
                   false);
            return;
        }

        const QJsonObject object = document.object();
        OidcMetadata metadata;
        metadata.issuer = NormalizeIssuer(QUrl(
            object.value(QStringLiteral("issuer")).toString()));
        metadata.authorizationEndpoint = QUrl(object.value(
            QStringLiteral("authorization_endpoint")).toString());
        metadata.tokenEndpoint = QUrl(object.value(
            QStringLiteral("token_endpoint")).toString());
        metadata.userInfoEndpoint = QUrl(object.value(
            QStringLiteral("userinfo_endpoint")).toString());

        if (metadata.issuer != expectedIssuer_) {
            Finish(false, {},
                   QStringLiteral("OIDC discovery issuer does not match the "
                                  "configured issuer."),
                   false);
            return;
        }
        if (!IsSecureEndpoint(metadata.authorizationEndpoint) ||
            !IsSecureEndpoint(metadata.tokenEndpoint) ||
            (!metadata.userInfoEndpoint.isEmpty() &&
             !IsSecureEndpoint(metadata.userInfoEndpoint))) {
            Finish(false, {},
                   QStringLiteral("OIDC discovery contains an insecure or "
                                  "invalid endpoint."),
                   false);
            return;
        }
        const QJsonValue challengeMethods = object.value(
            QStringLiteral("code_challenge_methods_supported"));
        if (challengeMethods.isArray() &&
            !StringArrayContains(challengeMethods, QStringLiteral("S256"))) {
            Finish(false, {},
                   QStringLiteral("OIDC provider does not advertise PKCE "
                                  "S256 support."),
                   false);
            return;
        }
        const QJsonValue responseTypes = object.value(
            QStringLiteral("response_types_supported"));
        if (responseTypes.isArray() &&
            !StringArrayContains(responseTypes, QStringLiteral("code"))) {
            Finish(false, {},
                   QStringLiteral("OIDC provider does not support the "
                                  "authorization code response type."),
                   false);
            return;
        }
        const QJsonValue authMethods = object.value(
            QStringLiteral("token_endpoint_auth_methods_supported"));
        if (authMethods.isArray() &&
            !StringArrayContains(authMethods, QStringLiteral("none"))) {
            Finish(false, {},
                   QStringLiteral("OIDC provider does not allow public native "
                                  "clients at the token endpoint."),
                   false);
            return;
        }
        Finish(true, std::move(metadata), {}, false);
    });
}

void OidcDiscovery::Cancel() {
    completion_ = {};
    expectedIssuer_.clear();
    if (activeReply_) {
        disconnect(activeReply_, nullptr, this, nullptr);
        activeReply_->abort();
        activeReply_->deleteLater();
        activeReply_.clear();
    }
}

void OidcDiscovery::Finish(
    bool success,
    OidcMetadata metadata,
    QString errorMessage,
    bool transient) {
    Completion completion = std::move(completion_);
    completion_ = {};
    if (completion) {
        completion(success, std::move(metadata), std::move(errorMessage),
                   transient);
    }
}

}  // namespace remote::auth
