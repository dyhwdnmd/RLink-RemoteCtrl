// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#pragma once

#include <QJsonObject>
#include <QSet>
#include <QString>
#include <QUrl>

#include <optional>

namespace remote::auth {

struct AuthConfig {
    QUrl issuer;
    QString clientId;
    QUrl callbackUrl = QUrl(QStringLiteral(
        "http://127.0.0.1:18765/callback"));
    QSet<QByteArray> scopes = {
        QByteArrayLiteral("openid"),
        QByteArrayLiteral("profile"),
        QByteArrayLiteral("email"),
        QByteArrayLiteral("offline_access"),
    };
    int networkTimeoutMs = 10000;
    // Interactive sign-up can require email verification and profile setup.
    // Keep the loopback callback alive without affecting silent refreshes.
    int interactiveTimeoutMs = 900000;

    bool Validate(QString* errorMessage = nullptr) const;

    static std::optional<AuthConfig> FromJson(
        const QJsonObject& object,
        QString* errorMessage = nullptr);
};

}  // namespace remote::auth
