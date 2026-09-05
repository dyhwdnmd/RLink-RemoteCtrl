// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#include "AccessTokenService.h"

#include <QCryptographicHash>
#include <QDateTime>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <QMessageAuthenticationCode>
#include <QRegularExpression>
#include <QUuid>

namespace remote::signaling_server {
namespace {

constexpr qint64 kMaximumTokenLifetimeSeconds = 7 * 24 * 60 * 60;
constexpr qint64 kClockSkewSeconds = 60;
constexpr auto kTokenAudience = "remotec-signaling";

QByteArray Base64UrlEncode(const QByteArray& value)
{
    return value.toBase64(QByteArray::Base64UrlEncoding |
                          QByteArray::OmitTrailingEquals);
}

QByteArray Base64UrlDecode(const QByteArray& value)
{
    return QByteArray::fromBase64(
        value, QByteArray::Base64UrlEncoding |
                   QByteArray::AbortOnBase64DecodingErrors);
}

bool ConstantTimeEqual(const QByteArray& left, const QByteArray& right)
{
    if (left.size() != right.size()) {
        return false;
    }
    unsigned char difference = 0;
    for (qsizetype index = 0; index < left.size(); ++index) {
        difference |= static_cast<unsigned char>(left[index]) ^
                      static_cast<unsigned char>(right[index]);
    }
    return difference == 0;
}

void SetError(QString* error, const QString& message)
{
    if (error) {
        *error = message;
    }
}

}  // namespace

AccessTokenService::AccessTokenService(QByteArray secret)
    : secret_(std::move(secret))
{}

bool AccessTokenService::IsConfigured() const
{
    return secret_.size() >= 32;
}

QByteArray AccessTokenService::Issue(const QString& deviceId,
                                     qint64 lifetimeSeconds,
                                     QString* error) const
{
    if (!IsConfigured()) {
        SetError(error, QStringLiteral(
                            "The token secret must contain at least 32 bytes."));
        return {};
    }
    if (!IsValidDeviceId(deviceId)) {
        SetError(error, QStringLiteral(
                            "Device ID must use 3-64 letters, digits, '.', '_' or '-'."));
        return {};
    }
    if (lifetimeSeconds <= 0 ||
        lifetimeSeconds > kMaximumTokenLifetimeSeconds) {
        SetError(error, QStringLiteral(
                            "Token lifetime must be between 1 and 604800 seconds."));
        return {};
    }

    const qint64 issuedAt = QDateTime::currentSecsSinceEpoch();
    QJsonObject payload;
    payload.insert(QStringLiteral("v"), 1);
    payload.insert(QStringLiteral("aud"),
                   QString::fromLatin1(kTokenAudience));
    payload.insert(QStringLiteral("deviceId"), deviceId);
    payload.insert(QStringLiteral("jti"),
                   QUuid::createUuid().toString(QUuid::WithoutBraces));
    payload.insert(QStringLiteral("iat"), issuedAt);
    payload.insert(QStringLiteral("exp"), issuedAt + lifetimeSeconds);

    const QByteArray encodedPayload = Base64UrlEncode(
        QJsonDocument(payload).toJson(QJsonDocument::Compact));
    return encodedPayload + '.' + Base64UrlEncode(Sign(encodedPayload));
}

bool AccessTokenService::Verify(const QByteArray& token,
                                AccessTokenClaims* claims,
                                QString* error) const
{
    if (!IsConfigured()) {
        SetError(error, QStringLiteral("Token verification is not configured."));
        return false;
    }

    const qsizetype separator = token.indexOf('.');
    if (separator <= 0 || separator != token.lastIndexOf('.') ||
        separator == token.size() - 1) {
        SetError(error, QStringLiteral("Access token format is invalid."));
        return false;
    }
    const QByteArray encodedPayload = token.left(separator);
    const QByteArray suppliedSignature =
        Base64UrlDecode(token.mid(separator + 1));
    const QByteArray expectedSignature = Sign(encodedPayload);
    if (suppliedSignature.isEmpty() ||
        !ConstantTimeEqual(suppliedSignature, expectedSignature)) {
        SetError(error, QStringLiteral("Access token signature is invalid."));
        return false;
    }

    QJsonParseError parseError;
    const QJsonDocument document = QJsonDocument::fromJson(
        Base64UrlDecode(encodedPayload), &parseError);
    if (parseError.error != QJsonParseError::NoError ||
        !document.isObject()) {
        SetError(error, QStringLiteral("Access token payload is invalid."));
        return false;
    }

    const QJsonObject payload = document.object();
    const QString deviceId =
        payload.value(QStringLiteral("deviceId")).toString();
    const QString tokenId = payload.value(QStringLiteral("jti")).toString();
    const qint64 issuedAt =
        payload.value(QStringLiteral("iat")).toInteger(-1);
    const qint64 expiresAt =
        payload.value(QStringLiteral("exp")).toInteger(-1);
    const qint64 now = QDateTime::currentSecsSinceEpoch();

    if (payload.value(QStringLiteral("v")).toInt(-1) != 1 ||
        payload.value(QStringLiteral("aud")).toString() !=
            QString::fromLatin1(kTokenAudience) ||
        !IsValidDeviceId(deviceId) || tokenId.isEmpty() || issuedAt < 0 ||
        expiresAt <= issuedAt ||
        expiresAt - issuedAt > kMaximumTokenLifetimeSeconds ||
        issuedAt > now + kClockSkewSeconds || expiresAt <= now) {
        SetError(error, QStringLiteral("Access token claims are invalid or expired."));
        return false;
    }

    if (claims) {
        claims->deviceId = deviceId;
        claims->tokenId = tokenId;
        claims->issuedAtSeconds = issuedAt;
        claims->expiresAtSeconds = expiresAt;
    }
    return true;
}

bool AccessTokenService::IsValidDeviceId(const QString& deviceId)
{
    static const QRegularExpression pattern(
        QStringLiteral("^[A-Za-z0-9][A-Za-z0-9._-]{2,63}$"));
    return pattern.match(deviceId).hasMatch();
}

QByteArray AccessTokenService::Sign(const QByteArray& encodedPayload) const
{
    return QMessageAuthenticationCode::hash(
        encodedPayload, secret_, QCryptographicHash::Sha256);
}

}  // namespace remote::signaling_server
