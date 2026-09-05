// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#pragma once

#include <QDateTime>
#include <QSet>
#include <QString>

namespace remote::auth {

enum class AuthState {
    kUnconfigured,
    kSignedOut,
    kDiscovering,
    kAuthorizing,
    kExchangingCode,
    kRefreshing,
    kAuthenticated,
    kError,
};

enum class AuthErrorCode {
    kNone,
    kInvalidConfiguration,
    kDiscoveryFailed,
    kCallbackListenerFailed,
    kBrowserLaunchFailed,
    kStateMismatch,
    kAuthorizationTimedOut,
    kAuthorizationDenied,
    kTokenRequestFailed,
    kRefreshRejected,
    kTokenStorageFailed,
    kStoredTokenUnavailable,
};

struct AuthError {
    AuthErrorCode code = AuthErrorCode::kNone;
    QString message;
    bool transient = false;
};

struct AuthTokenSnapshot {
    QString accessToken;
    QString idToken;
    QString tokenType;
    QSet<QByteArray> grantedScopes;
    QDateTime expiresAtUtc;

    bool HasUsableAccessToken() const {
        return !accessToken.isEmpty() &&
               (!expiresAtUtc.isValid() ||
                expiresAtUtc > QDateTime::currentDateTimeUtc());
    }
};

}  // namespace remote::auth
