// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#pragma once

#include <QString>

namespace remote::server_auth {

struct UserInfoClaims {
    QString subject;
    QString username;
    QString displayName;
    QString email;
};

enum class UserInfoStatus {
    kAuthenticated,
    kInvalidToken,
    kServiceUnavailable,
    kInvalidResponse,
    kCancelled,
};

struct UserInfoResult {
    UserInfoStatus status = UserInfoStatus::kInvalidResponse;
    UserInfoClaims claims;
    QString code;
    QString message;
    bool retryable = false;

    bool authenticated() const {
        return status == UserInfoStatus::kAuthenticated;
    }
};

}  // namespace remote::server_auth
