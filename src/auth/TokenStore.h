// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#pragma once

#include <QDateTime>
#include <QString>

namespace remote::auth {

struct StoredRefreshToken {
    QString issuer;
    QString clientId;
    QString refreshToken;
    QDateTime savedAtUtc;
};

enum class TokenStoreLoadStatus {
    kLoaded,
    kNotFound,
    kError,
};

class TokenStore {
public:
    virtual ~TokenStore() = default;

    virtual TokenStoreLoadStatus Load(
        StoredRefreshToken* token,
        QString* errorMessage = nullptr) = 0;
    virtual bool Save(
        const StoredRefreshToken& token,
        QString* errorMessage = nullptr) = 0;
    virtual bool Clear(QString* errorMessage = nullptr) = 0;
};

}  // namespace remote::auth
