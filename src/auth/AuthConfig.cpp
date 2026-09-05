// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#include "src/auth/AuthConfig.h"

#include <QJsonArray>
#include <QStringList>

namespace remote::auth {
namespace {

void SetError(QString* errorMessage, const QString& message) {
    if (errorMessage) {
        *errorMessage = message;
    }
}

bool HasForbiddenSecret(const QJsonObject& object) {
    for (const QString& key : object.keys()) {
        QString normalized = key.toLower();
        normalized.remove(QLatin1Char('_'));
        normalized.remove(QLatin1Char('-'));
        if (normalized == QStringLiteral("clientsecret")) {
            return true;
        }
        const QJsonValue value = object.value(key);
        if (value.isObject() && HasForbiddenSecret(value.toObject())) {
            return true;
        }
        if (value.isArray()) {
            for (const QJsonValue& item : value.toArray()) {
                if (item.isObject() &&
                    HasForbiddenSecret(item.toObject())) {
                    return true;
                }
            }
        }
    }
    return false;
}

}  // namespace

bool AuthConfig::Validate(QString* errorMessage) const {
    if (!issuer.isValid() || issuer.scheme() != QStringLiteral("https") ||
        issuer.host().isEmpty() || !issuer.query().isEmpty() ||
        !issuer.fragment().isEmpty()) {
        SetError(errorMessage,
                 QStringLiteral("Logto issuer must be an HTTPS URL without "
                                "a query or fragment."));
        return false;
    }
    if (clientId.trimmed().isEmpty() || clientId.size() > 256) {
        SetError(errorMessage,
                 QStringLiteral("Logto client ID is missing or too long."));
        return false;
    }

    const QUrl requiredCallback(QStringLiteral(
        "http://127.0.0.1:18765/callback"));
    if (callbackUrl != requiredCallback) {
        SetError(errorMessage,
                 QStringLiteral("OAuth callback must be exactly "
                                "http://127.0.0.1:18765/callback."));
        return false;
    }

    static const QSet<QByteArray> requiredScopes = {
        QByteArrayLiteral("openid"),
        QByteArrayLiteral("profile"),
        QByteArrayLiteral("email"),
        QByteArrayLiteral("offline_access"),
    };
    if (!scopes.contains(requiredScopes)) {
        SetError(errorMessage,
                 QStringLiteral("OAuth scopes must include openid, profile, "
                                "email, and offline_access."));
        return false;
    }
    if (networkTimeoutMs < 1000 || networkTimeoutMs > 60000 ||
        interactiveTimeoutMs < 10000 || interactiveTimeoutMs > 1800000) {
        SetError(errorMessage,
                 QStringLiteral("OAuth timeout configuration is outside the "
                                "supported range."));
        return false;
    }
    return true;
}

std::optional<AuthConfig> AuthConfig::FromJson(
    const QJsonObject& object,
    QString* errorMessage) {
    if (HasForbiddenSecret(object)) {
        SetError(errorMessage,
                 QStringLiteral("A native OAuth client must not contain a "
                                "client secret."));
        return std::nullopt;
    }

    AuthConfig config;
    config.issuer = QUrl(object.value(QStringLiteral("issuer")).toString());
    config.clientId =
        object.value(QStringLiteral("clientId")).toString().trimmed();
    if (object.contains(QStringLiteral("callbackUrl"))) {
        config.callbackUrl = QUrl(
            object.value(QStringLiteral("callbackUrl")).toString());
    }
    if (object.contains(QStringLiteral("networkTimeoutMs"))) {
        config.networkTimeoutMs =
            object.value(QStringLiteral("networkTimeoutMs")).toInt();
    }
    if (object.contains(QStringLiteral("interactiveTimeoutMs"))) {
        config.interactiveTimeoutMs =
            object.value(QStringLiteral("interactiveTimeoutMs")).toInt();
    }
    if (const QJsonValue value = object.value(QStringLiteral("scopes"));
        value.isArray()) {
        config.scopes.clear();
        for (const QJsonValue& scope : value.toArray()) {
            const QByteArray encoded = scope.toString().trimmed().toUtf8();
            if (!encoded.isEmpty()) {
                config.scopes.insert(encoded);
            }
        }
    }

    if (!config.Validate(errorMessage)) {
        return std::nullopt;
    }
    return config;
}

}  // namespace remote::auth
