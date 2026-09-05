// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#pragma once

#include <QNetworkAccessManager>
#include <QObject>
#include <QTimer>

#include <functional>
#include <memory>

#include "src/auth/AuthConfig.h"
#include "src/auth/AuthTypes.h"
#include "src/auth/OidcDiscovery.h"
#include "src/auth/TokenStore.h"

QT_BEGIN_NAMESPACE
class QOAuth2AuthorizationCodeFlow;
class QOAuthHttpServerReplyHandler;
QT_END_NAMESPACE

namespace remote::auth {

class AuthManager final : public QObject {
public:
    struct Callbacks {
        std::function<void(AuthState)> stateChanged;
        std::function<void(const AuthTokenSnapshot&)> authenticated;
        std::function<void(const AuthError&)> failed;
    };

    explicit AuthManager(
        std::unique_ptr<TokenStore> tokenStore,
        QObject* parent = nullptr);
    ~AuthManager() override;

    bool Configure(const AuthConfig& config, QString* errorMessage = nullptr);
    void SetCallbacks(Callbacks callbacks);

    void StartInteractiveLogin();
    void RestoreSession();
    void RefreshAccessToken();
    void SignOut();
    void Cancel();

    AuthState state() const { return state_; }
    const AuthTokenSnapshot& tokens() const { return tokens_; }
    const AuthConfig& config() const { return config_; }

private:
    enum class Operation {
        kNone,
        kInteractiveLogin,
        kRestoreSession,
        kRefresh,
    };

    void BeginDiscovery(Operation operation);
    void BuildFlow();
    void BeginInteractiveGrant();
    void BeginStoredTokenRefresh(const StoredRefreshToken& storedToken);
    void CompleteAuthentication();
    void SetState(AuthState state);
    void Fail(AuthErrorCode code, QString message, bool transient);
    void CloseCallbackListener();
    void AbortOutstandingNetworkReplies();
    static QString GenerateState();

    std::unique_ptr<TokenStore> tokenStore_;
    QNetworkAccessManager networkManager_;
    OidcDiscovery discovery_;
    std::unique_ptr<QOAuth2AuthorizationCodeFlow> flow_;
    std::unique_ptr<QOAuthHttpServerReplyHandler> replyHandler_;
    QTimer operationTimer_;
    QTimer refreshTimer_;
    AuthConfig config_;
    OidcMetadata metadata_;
    AuthTokenSnapshot tokens_;
    Callbacks callbacks_;
    AuthState state_ = AuthState::kUnconfigured;
    Operation operation_ = Operation::kNone;
    QString expectedState_;
};

}  // namespace remote::auth
