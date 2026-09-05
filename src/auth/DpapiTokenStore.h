// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#pragma once

#include <QString>

#include "src/auth/TokenStore.h"

namespace remote::auth {

class DpapiTokenStore final : public TokenStore {
public:
    explicit DpapiTokenStore(QString credentialsPath = {});

    TokenStoreLoadStatus Load(
        StoredRefreshToken* token,
        QString* errorMessage = nullptr) override;
    bool Save(
        const StoredRefreshToken& token,
        QString* errorMessage = nullptr) override;
    bool Clear(QString* errorMessage = nullptr) override;

    const QString& credentialsPath() const { return credentialsPath_; }

    static QString DefaultCredentialsPath();

private:
    QString credentialsPath_;
};

}  // namespace remote::auth
