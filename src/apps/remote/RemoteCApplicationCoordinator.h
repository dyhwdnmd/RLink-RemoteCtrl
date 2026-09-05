// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#pragma once

#include <QObject>
#include <QString>

#include <functional>
#include <memory>

#include "src/auth/AuthConfig.h"
#include "src/auth/AuthTypes.h"

namespace remote::auth {
class AuthManager;
class TokenStore;
}

namespace remote::controller {
class ControllerMainWindow;
class LoginWindow;
class LoginStatusWindow;
}

namespace remote::app {

class RemoteCApplicationCoordinator final : public QObject {
public:
    struct AccountDeletionResult {
        bool deleted = false;
        QString code;
        QString message;
        bool retryable = false;
    };

    struct Options {
        bool authenticationRequired = false;
        bool startMainWindowInTray = false;
        auth::AuthConfig authConfig;
        QString configurationError;
        QString configurationSource;
    };

    struct MainWindowSession {
        std::unique_ptr<controller::ControllerMainWindow> window;
        // The coordinator invokes this after OAuth refresh. The callback only
        // replaces the in-memory signaling token used by a future reconnect.
        std::function<bool(const QString& accessToken)> updateAccessToken;
        std::function<bool(QString* errorMessage)> requestAccountDeletion;
        std::function<void(
            std::function<void(AccountDeletionResult)> callback)>
            setAccountDeletionResultCallback;
    };

    using MainWindowFactory = std::function<MainWindowSession(
        const QString& accessToken)>;

    RemoteCApplicationCoordinator(
        Options options,
        MainWindowFactory mainWindowFactory,
        std::unique_ptr<auth::TokenStore> tokenStore,
        QObject* parent = nullptr);
    ~RemoteCApplicationCoordinator() override;

    bool Start();
    void ActivateFromExternalLaunch();

private:
    void EnsureLoginWindow();
    void EnsureLoginStatusWindow();
    void ContinueAuthenticatedStartup();
    void ShowAccountWindow();
    void ShowMainWindowAfterStartup();
    void CompleteStartupSignedOut();
    bool CreateMainWindow(const QString& accessToken);
    void HandleAuthState(auth::AuthState state);
    void HandleAuthenticated(const auth::AuthTokenSnapshot& tokens);
    void HandleAuthFailure(const auth::AuthError& error);
    void HandleSignOut();
    void HandleSwitchAccount();
    void HandleDeleteAccount();
    void HandleAccountDeletionResult(AccountDeletionResult result);
    void ExitApplication();

    Options options_;
    MainWindowFactory mainWindowFactory_;
    std::unique_ptr<auth::AuthManager> authManager_;
    std::unique_ptr<controller::ControllerMainWindow> mainWindow_;
    std::unique_ptr<controller::LoginWindow> loginWindow_;
    std::unique_ptr<controller::LoginStatusWindow> loginStatusWindow_;
    std::function<bool(const QString& accessToken)> updateAccessToken_;
    std::function<bool(QString* errorMessage)> requestAccountDeletion_;
    std::function<void(
        std::function<void(AccountDeletionResult)> callback)>
        setAccountDeletionResultCallback_;
    bool interactiveLoginRequested_ = false;
    bool startupAuthenticationPending_ = false;
    bool authenticated_ = false;
    bool accountDeletionPending_ = false;
    QString accountLabel_;
    QString accountDetail_;
    QString accountSubject_;
};

}  // namespace remote::app
