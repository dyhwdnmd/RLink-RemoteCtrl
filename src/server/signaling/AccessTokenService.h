// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#pragma once

#include <QByteArray>
#include <QString>

namespace remote::signaling_server {

struct AccessTokenClaims {
    QString deviceId;
    QString tokenId;
    qint64 issuedAtSeconds = 0;
    qint64 expiresAtSeconds = 0;
};

class AccessTokenService final {
public:
    explicit AccessTokenService(QByteArray secret);

    bool IsConfigured() const;

    QByteArray Issue(const QString& deviceId,
                     qint64 lifetimeSeconds,
                     QString* error) const;
    bool Verify(const QByteArray& token,
                AccessTokenClaims* claims,
                QString* error) const;

    static bool IsValidDeviceId(const QString& deviceId);

private:
    QByteArray Sign(const QByteArray& encodedPayload) const;

    QByteArray secret_;
};

}  // namespace remote::signaling_server
