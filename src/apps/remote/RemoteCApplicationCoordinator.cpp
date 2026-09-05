// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#include "src/apps/remote/RemoteCApplicationCoordinator.h"

#include <QApplication>
#include <QByteArray>
#include <QCoreApplication>
#include <QEventLoop>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSettings>
#include <QSystemTrayIcon>
#include <QTimer>
#include <QWindow>

#include <utility>

#include "src/apps/controller/ControllerMainWindow.h"
#include "src/apps/controller/LoginWindow.h"
#include "src/apps/controller/RemoteCDialog.h"
#include "src/auth/AuthManager.h"
#include "src/auth/TokenStore.h"

namespace remote::app {
namespace {

struct AccountDisplayClaims {
    QString label = QStringLiteral("RLink 用户");
    QString detail;
    QString subject;
};

AccountDisplayClaims DecodeDisplayClaims(const QString& idToken) {
    AccountDisplayClaims result;
    const QList<QByteArray> parts = idToken.toUtf8().split('.');
    if (parts.size() != 3) {
        return result;
    }
    const QJsonObject claims = QJsonDocument::fromJson(
        QByteArray::fromBase64(parts.at(1),
                               QByteArray::Base64UrlEncoding))
                                   .object();
    const QString email = claims.value(QStringLiteral("email")).toString();
    const QString name = claims.value(QStringLiteral("name")).toString();
    const QString username =
        claims.value(QStringLiteral("username")).toString();
    const QString subject = claims.value(
        QStringLiteral("sub")).toString().trimmed();
    result.subject = subject;
    if (!name.isEmpty()) {
        result.label = name;
    } else if (!username.isEmpty()) {
        result.label = username;
    } else if (!email.isEmpty()) {
        result.label = email;
    }
    result.detail = !email.isEmpty() ? email : subject;
    return result;
}

}  // namespace

RemoteCApplicationCoordinator::RemoteCApplicationCoordinator(
    Options options,
    MainWindowFactory mainWindowFactory,
    std::unique_ptr<auth::TokenStore> tokenStore,
    QObject* parent)
    : QObject(parent),
      options_(std::move(options)),
      mainWindowFactory_(std::move(mainWindowFactory)) {
    connect(qApp, &QCoreApplication::aboutToQuit, this, [this] {
        if (mainWindow_) {
            mainWindow_->PrepareForApplicationExit();
        }
    });
    if (options_.authenticationRequired) {
        authManager_ = std::make_unique<auth::AuthManager>(
            std::move(tokenStore), this);
    }
}

RemoteCApplicationCoordinator::~RemoteCApplicationCoordinator() = default;

bool RemoteCApplicationCoordinator::Start() {
    if (!mainWindowFactory_) {
        return false;
    }
    if (!options_.authenticationRequired) {
        return CreateMainWindow({});
    }

    // Give Windows a real top-level window immediately. ControllerMainWindow
    // contains a large QWidget tree and must be constructed on the Qt thread;
    // deferring that work briefly lets this lightweight startup window paint
    // before the heavier construction begins instead of showing only the
    // Windows application-starting cursor.
    EnsureLoginStatusWindow();
    QSettings settings;
    loginStatusWindow_->SetAccountLabel(settings.value(
        QStringLiteral("auth/accountLabel"),
        QStringLiteral("RLink 账户")).toString());
    loginStatusWindow_->ShowAndActivate();
    QTimer::singleShot(50, this, [this] {
        ContinueAuthenticatedStartup();
    });
    return true;
}

void RemoteCApplicationCoordinator::ContinueAuthenticatedStartup() {
    if (!CreateMainWindow({})) {
        if (loginStatusWindow_) {
            loginStatusWindow_->HideAndReleaseTopmost();
        }
        EnsureLoginWindow();
        loginWindow_->ShowError(
            QStringLiteral("RLink 主窗口初始化失败。"), false);
        loginWindow_->ShowAndActivate();
        return;
    }
    EnsureLoginWindow();
    if (!options_.configurationError.isEmpty()) {
        if (loginStatusWindow_) {
            loginStatusWindow_->HideAndReleaseTopmost();
        }
        mainWindow_->SetAccountSignedOut(options_.configurationError);
        loginWindow_->ShowError(
            QStringLiteral("%1\n\n配置来源：%2")
                .arg(options_.configurationError,
                     options_.configurationSource),
            false);
        ShowMainWindowAfterStartup();
        loginWindow_->ShowAndActivate();
        return;
    }

    QString errorMessage;
    if (!authManager_ ||
        !authManager_->Configure(options_.authConfig, &errorMessage)) {
        if (loginStatusWindow_) {
            loginStatusWindow_->HideAndReleaseTopmost();
        }
        loginWindow_->ShowError(
            errorMessage.isEmpty()
                ? QStringLiteral("Logto 登录配置不可用。")
                : errorMessage,
            false);
        mainWindow_->SetAccountSignedOut(
            errorMessage.isEmpty()
                ? QStringLiteral("Logto 登录配置不可用。")
                : errorMessage);
        ShowMainWindowAfterStartup();
        loginWindow_->ShowAndActivate();
        return;
    }
    authManager_->SetCallbacks({
        [this](auth::AuthState state) { HandleAuthState(state); },
        [this](const auth::AuthTokenSnapshot& tokens) {
            HandleAuthenticated(tokens);
        },
        [this](const auth::AuthError& error) {
            HandleAuthFailure(error);
        },
    });

    startupAuthenticationPending_ = true;

    // First inspect the encrypted credential store through RestoreSession().
    // The status window is only useful when a stored session really exists;
    // signed-out users should go straight to the main/login UI without a
    // misleading "signing in" flash.
    QTimer::singleShot(0, authManager_.get(), [this] {
        if (!authManager_) {
            return;
        }
        authManager_->RestoreSession();
        if (!startupAuthenticationPending_) {
            return;
        }
        if (authManager_->state() == auth::AuthState::kSignedOut &&
            startupAuthenticationPending_) {
            CompleteStartupSignedOut();
            return;
        }
        // The startup window is already visible. Keep it in place while the
        // stored refresh token is being validated over the network.
    });
}

void RemoteCApplicationCoordinator::ActivateFromExternalLaunch() {
    if (mainWindow_) {
        mainWindow_->ActivateFromExternalLaunch();
    } else if (loginWindow_) {
        loginWindow_->ShowAndActivate();
    }
}

void RemoteCApplicationCoordinator::EnsureLoginWindow() {
    if (loginWindow_) {
        return;
    }
    loginWindow_ = std::make_unique<controller::LoginWindow>();
    if (mainWindow_) {
        loginWindow_->SetOwnerWindow(mainWindow_.get());
    }
    loginWindow_->hide();
    loginWindow_->SetLoginRequestedCallback([this] {
        if (authManager_) {
            interactiveLoginRequested_ = true;
            if (mainWindow_) {
                mainWindow_->SetAccountBusy(QStringLiteral(
                    "请在系统浏览器中完成 Logto 登录。"));
            }
            authManager_->StartInteractiveLogin();
        }
    });
    loginWindow_->SetCancelRequestedCallback([this] {
        if (authManager_) {
            authManager_->Cancel();
        }
        if (loginWindow_) {
            loginWindow_->ShowReady(
                QStringLiteral("登录操作已取消。"));
        }
    });
    loginWindow_->SetExitRequestedCallback([this] {
        if (loginWindow_) {
            loginWindow_->HideWithAnimation();
        }
    });
    loginWindow_->SetSignOutRequestedCallback(
        [this] { HandleSignOut(); });
    loginWindow_->SetDeleteAccountRequestedCallback(
        [this] { HandleDeleteAccount(); });
}

void RemoteCApplicationCoordinator::EnsureLoginStatusWindow() {
    if (!loginStatusWindow_) {
        loginStatusWindow_ =
            std::make_unique<controller::LoginStatusWindow>();
    }
}

void RemoteCApplicationCoordinator::ShowMainWindowAfterStartup() {
    if (!mainWindow_) {
        return;
    }
    if (!options_.startMainWindowInTray ||
        !QSystemTrayIcon::isSystemTrayAvailable()) {
        mainWindow_->ActivateFromExternalLaunch();
        // Hiding a native topmost restore window and activating another
        // top-level window in the same stack frame is occasionally ignored by
        // Windows. Repeat after the hide/activation messages have drained.
        QTimer::singleShot(0, mainWindow_.get(), [this] {
            if (!mainWindow_ || !mainWindow_->isVisible()) return;
            mainWindow_->ActivateFromExternalLaunch();
        });
    }
}

void RemoteCApplicationCoordinator::CompleteStartupSignedOut() {
    if (!startupAuthenticationPending_) {
        return;
    }
    startupAuthenticationPending_ = false;
    if (loginStatusWindow_) {
        loginStatusWindow_->HideAndReleaseTopmost();
    }
    if (mainWindow_) {
        mainWindow_->show();
        mainWindow_->raise();
        mainWindow_->activateWindow();
        mainWindow_->SetAccountSignedOut();
    }
    EnsureLoginWindow();
    loginWindow_->ShowReady();
    loginWindow_->ShowAndActivate();
}

void RemoteCApplicationCoordinator::ShowAccountWindow() {
    EnsureLoginWindow();
    if (loginWindow_->isVisible()) {
        if (!loginWindow_->IsHiding()) {
            loginWindow_->HideWithAnimation();
        }
        return;
    }
    if (authenticated_) {
        loginWindow_->ShowAuthenticated(accountLabel_, accountDetail_);
    } else if (authManager_ &&
               (authManager_->state() == auth::AuthState::kAuthorizing ||
                authManager_->state() == auth::AuthState::kExchangingCode ||
                (interactiveLoginRequested_ &&
                 authManager_->state() == auth::AuthState::kDiscovering))) {
        loginWindow_->ShowBusy(
            QStringLiteral("请在浏览器中完成登录"),
            QStringLiteral("RLink 正在等待安全登录结果。"));
    }
    loginWindow_->ShowAndActivate();
}

bool RemoteCApplicationCoordinator::CreateMainWindow(
    const QString& accessToken) {
    if (mainWindow_) {
        return true;
    }
    MainWindowSession session = mainWindowFactory_(accessToken);
    mainWindow_ = std::move(session.window);
    updateAccessToken_ = std::move(session.updateAccessToken);
    requestAccountDeletion_ = std::move(session.requestAccountDeletion);
    setAccountDeletionResultCallback_ =
        std::move(session.setAccountDeletionResultCallback);
    if (!mainWindow_) {
        updateAccessToken_ = {};
        requestAccountDeletion_ = {};
        setAccountDeletionResultCallback_ = {};
        if (options_.authenticationRequired) {
            EnsureLoginWindow();
            loginWindow_->ShowError(
                QStringLiteral("RLink 主窗口初始化失败。"), true);
            loginWindow_->ShowAndActivate();
        }
        return false;
    }
    if (setAccountDeletionResultCallback_) {
        setAccountDeletionResultCallback_(
            [this](AccountDeletionResult result) {
                QMetaObject::invokeMethod(
                    this,
                    [this, result = std::move(result)]() mutable {
                        HandleAccountDeletionResult(std::move(result));
                    },
                    Qt::QueuedConnection);
            });
    }
    if (options_.authenticationRequired) {
        mainWindow_->SetAccountInteractionCallback(
            [this] { ShowAccountWindow(); });
        mainWindow_->SetAccountSwitchCallback(
            [this] { HandleSwitchAccount(); });
        mainWindow_->SetAccountDeletionCallback(
            [this] { HandleDeleteAccount(); });
        mainWindow_->SetAccountSignedOut();
    }
    if (loginWindow_) {
        loginWindow_->hide();
    }
    if (!options_.authenticationRequired &&
        (!options_.startMainWindowInTray ||
         !QSystemTrayIcon::isSystemTrayAvailable())) {
        mainWindow_->show();
    }
    if (!options_.authenticationRequired &&
        !options_.configurationError.isEmpty()) {
        QTimer::singleShot(0, mainWindow_.get(), [this] {
            if (!mainWindow_) {
                return;
            }
            controller::RemoteCDialog::Alert(
                mainWindow_.get(),
                QStringLiteral("RLink 启动配置不可用"),
                QStringLiteral("%1\n\n配置来源：%2")
                    .arg(options_.configurationError,
                         options_.configurationSource),
                QStringLiteral("知道了"),
                controller::RemoteCDialog::Tone::kDanger,
                true);
        });
    }
    return true;
}

void RemoteCApplicationCoordinator::HandleAuthState(
    auth::AuthState state) {
    EnsureLoginWindow();
    switch (state) {
        case auth::AuthState::kSignedOut:
            if (startupAuthenticationPending_) {
                CompleteStartupSignedOut();
                break;
            }
            if (!authenticated_ && mainWindow_) {
                mainWindow_->SetAccountSignedOut();
            }
            loginWindow_->ShowReady();
            break;
        case auth::AuthState::kDiscovering:
            if (interactiveLoginRequested_) {
                loginWindow_->ShowBusy(
                    QStringLiteral("正在连接 Logto"),
                    QStringLiteral("正在读取安全登录服务配置…"));
            }
            break;
        case auth::AuthState::kAuthorizing:
            loginWindow_->ShowBusy(
                QStringLiteral("请在浏览器中完成登录"),
                QStringLiteral(
                    "RLink 正在等待 127.0.0.1 安全回调。"));
            break;
        case auth::AuthState::kExchangingCode:
            loginWindow_->ShowBusy(
                QStringLiteral("正在完成登录"),
                QStringLiteral("正在安全交换授权码…"));
            break;
        case auth::AuthState::kRefreshing:
            // Initial restoration and routine refresh are deliberately silent.
            break;
        default:
            break;
    }
}

void RemoteCApplicationCoordinator::HandleAuthenticated(
    const auth::AuthTokenSnapshot& tokens) {
    if (!tokens.HasUsableAccessToken()) {
        HandleAuthFailure(auth::AuthError{
            auth::AuthErrorCode::kTokenRequestFailed,
            QStringLiteral("Logto 没有返回可用的 Access Token。"),
            false});
        return;
    }
    if (!mainWindow_) {
        HandleAuthFailure(auth::AuthError{
            auth::AuthErrorCode::kTokenRequestFailed,
            QStringLiteral("登录成功，但 RLink 主窗口不可用。"),
            false});
        return;
    }
    if (!updateAccessToken_ ||
        !updateAccessToken_(tokens.accessToken)) {
        HandleAuthFailure(auth::AuthError{
            auth::AuthErrorCode::kTokenRequestFailed,
            QStringLiteral(
                "新的 Access Token 无法更新到 signaling 客户端。"),
            false});
        return;
    }
    if (!tokens.idToken.isEmpty() || accountLabel_.isEmpty()) {
        const AccountDisplayClaims claims =
            DecodeDisplayClaims(tokens.idToken);
        accountLabel_ = claims.label;
        accountDetail_ = claims.detail;
        if (!claims.subject.isEmpty()) {
            accountSubject_ = claims.subject;
        }
    }
    const bool completedInteractiveLogin = interactiveLoginRequested_;
    const bool completedStartupLogin = startupAuthenticationPending_;
    authenticated_ = true;
    interactiveLoginRequested_ = false;
    startupAuthenticationPending_ = false;
    QSettings settings;
    if (accountSubject_.isEmpty()) {
        accountSubject_ = settings.value(
            QStringLiteral("auth/accountSubject")).toString();
    }
    settings.setValue(QStringLiteral("auth/accountLabel"), accountLabel_);
    settings.setValue(QStringLiteral("auth/accountDetail"), accountDetail_);
    settings.setValue(QStringLiteral("auth/accountSubject"), accountSubject_);
    mainWindow_->SetAccountSession(
        accountSubject_, accountLabel_, accountDetail_,
        [this] { HandleSignOut(); });
    (void)mainWindow_->StartSessionEngine();
    if (loginStatusWindow_) {
        loginStatusWindow_->HideAndReleaseTopmost();
    }
    if (completedStartupLogin) {
        ShowMainWindowAfterStartup();
    }
    EnsureLoginWindow();
    loginWindow_->ShowAuthenticated(accountLabel_, accountDetail_);
    if (completedInteractiveLogin && loginWindow_->isVisible()) {
        loginWindow_->HideWithAnimation();
    }
}

void RemoteCApplicationCoordinator::HandleAuthFailure(
    const auth::AuthError& error) {
    interactiveLoginRequested_ = false;
    if (authenticated_ && error.transient && authManager_) {
        QTimer::singleShot(5000, authManager_.get(), [this] {
            if (authManager_ && authenticated_) {
                authManager_->RefreshAccessToken();
            }
        });
        return;
    }
    const bool failedDuringStartup = startupAuthenticationPending_;
    startupAuthenticationPending_ = false;
    if (loginStatusWindow_) {
        loginStatusWindow_->HideAndReleaseTopmost();
    }
    authenticated_ = false;
    accountDeletionPending_ = false;
    accountLabel_.clear();
    accountDetail_.clear();
    accountSubject_.clear();
    QSettings settings;
    settings.remove(QStringLiteral("auth/accountLabel"));
    settings.remove(QStringLiteral("auth/accountDetail"));
    settings.remove(QStringLiteral("auth/accountSubject"));
    if (mainWindow_) {
        mainWindow_->StopSessionEngine();
        mainWindow_->SetAccountSignedOut(
            error.message.isEmpty()
                ? QStringLiteral("登录未完成，请重试。")
                : error.message);
    }
    EnsureLoginWindow();
    loginWindow_->ShowError(
        error.message.isEmpty()
            ? QStringLiteral("登录过程中发生未知错误。")
            : error.message,
        true);
    if (failedDuringStartup && mainWindow_) {
        mainWindow_->show();
        mainWindow_->raise();
        mainWindow_->activateWindow();
    }
    if (failedDuringStartup || loginWindow_->isVisible()) {
        loginWindow_->ShowAndActivate();
    }
}

void RemoteCApplicationCoordinator::HandleSignOut() {
    QWidget* confirmationParent =
        loginWindow_ && loginWindow_->isVisible()
            ? static_cast<QWidget*>(loginWindow_.get())
            : static_cast<QWidget*>(mainWindow_.get());
    if (!controller::RemoteCDialog::Confirm(
            confirmationParent,
            QStringLiteral("退出当前账户？"),
            QStringLiteral(
                "退出后将清除这台电脑上保存的登录凭据，并断开当前房间、"
                "远程控制和 signaling 连接。"),
            QStringLiteral("退出登录"),
            QStringLiteral("取消"),
            controller::RemoteCDialog::Tone::kDanger,
            true)) {
        return;
    }
    QTimer::singleShot(0, this, [this] {
        accountDeletionPending_ = false;
        authenticated_ = false;
        interactiveLoginRequested_ = false;
        accountLabel_.clear();
        accountDetail_.clear();
        accountSubject_.clear();
        QSettings settings;
        settings.remove(QStringLiteral("auth/accountLabel"));
        settings.remove(QStringLiteral("auth/accountDetail"));
        settings.remove(QStringLiteral("auth/accountSubject"));
        if (mainWindow_) {
            mainWindow_->StopSessionEngine();
            mainWindow_->SetAccountSignedOut(
                QStringLiteral("你已安全退出登录。"));
        }
        EnsureLoginWindow();
        if (authManager_) {
            authManager_->SignOut();
        }
        loginWindow_->ShowReady(
            QStringLiteral("你已安全退出登录。"));
        if (loginWindow_->isVisible()) {
            loginWindow_->ShowAndActivate();
        }
    });
}

void RemoteCApplicationCoordinator::HandleSwitchAccount() {
    if (!authenticated_ || !authManager_ || !mainWindow_) {
        ShowAccountWindow();
        return;
    }
    if (!controller::RemoteCDialog::Confirm(
            mainWindow_.get(),
            QStringLiteral("切换 RLink 用户？"),
            QStringLiteral(
                "当前房间、远程控制和 signaling 连接将断开，"
                "本机保存的当前用户凭据也会清除。随后将立即打开浏览器登录其他用户。"),
            QStringLiteral("切换用户"),
            QStringLiteral("取消"),
            controller::RemoteCDialog::Tone::kWarning,
            true)) {
        return;
    }
    QTimer::singleShot(0, this, [this] {
        accountDeletionPending_ = false;
        authenticated_ = false;
        interactiveLoginRequested_ = false;
        accountLabel_.clear();
        accountDetail_.clear();
        accountSubject_.clear();
        QSettings settings;
        settings.remove(QStringLiteral("auth/accountLabel"));
        settings.remove(QStringLiteral("auth/accountDetail"));
        settings.remove(QStringLiteral("auth/accountSubject"));
        mainWindow_->StopSessionEngine();
        mainWindow_->SetAccountSignedOut(
            QStringLiteral("正在切换用户…"));
        EnsureLoginWindow();
        authManager_->SignOut();
        interactiveLoginRequested_ = true;
        mainWindow_->SetAccountBusy(
            QStringLiteral("请在系统浏览器中登录其他 RLink 用户。"));
        loginWindow_->ShowBusy(
            QStringLiteral("正在切换用户"),
            QStringLiteral("即将在系统浏览器中打开安全登录页面…"));
        loginWindow_->ShowAndActivate();
        authManager_->StartInteractiveLogin();
    });
}

void RemoteCApplicationCoordinator::HandleDeleteAccount() {
    if (!authenticated_ || accountDeletionPending_) {
        return;
    }
    QWidget* confirmationParent =
        loginWindow_ && loginWindow_->isVisible()
            ? static_cast<QWidget*>(loginWindow_.get())
            : static_cast<QWidget*>(mainWindow_.get());
    if (!controller::RemoteCDialog::Confirm(
            confirmationParent,
            QStringLiteral("永久注销 RLink 账号？"),
            QStringLiteral(
                "此操作不可撤销。将永久删除当前 Logto 账号，以及 RLink "
                "服务器中属于该账号的设备与身份记录；当前房间和远程连接也会立即结束。\n\n"
                "“退出登录”不会删除账号，如只想换账号请取消并使用退出登录。"),
            QStringLiteral("永久注销账号"),
            QStringLiteral("取消"),
            controller::RemoteCDialog::Tone::kDanger,
            true)) {
        return;
    }

    if (!requestAccountDeletion_) {
        controller::RemoteCDialog::Alert(
            confirmationParent,
            QStringLiteral("暂时无法注销账号"),
            QStringLiteral("当前版本没有可用的账号注销通道。"),
            QStringLiteral("知道了"),
            controller::RemoteCDialog::Tone::kWarning,
            true);
        return;
    }
    QString requestError;
    if (!requestAccountDeletion_(&requestError)) {
        controller::RemoteCDialog::Alert(
            confirmationParent,
            QStringLiteral("暂时无法注销账号"),
            requestError.isEmpty()
                ? QStringLiteral("请先等待 signaling 连接完成后重试。")
                : requestError,
            QStringLiteral("知道了"),
            controller::RemoteCDialog::Tone::kWarning,
            true);
        return;
    }
    accountDeletionPending_ = true;
    EnsureLoginWindow();
    loginWindow_->ShowAccountDeletionBusy();
    loginWindow_->ShowAndActivate();
}

void RemoteCApplicationCoordinator::HandleAccountDeletionResult(
    AccountDeletionResult result) {
    if (result.deleted) {
        if (!authenticated_ && !accountDeletionPending_) {
            return;
        }
        accountDeletionPending_ = false;
        authenticated_ = false;
        interactiveLoginRequested_ = false;
        accountLabel_.clear();
        accountDetail_.clear();
        accountSubject_.clear();
        QSettings settings;
        settings.remove(QStringLiteral("auth/accountLabel"));
        settings.remove(QStringLiteral("auth/accountDetail"));
        settings.remove(QStringLiteral("auth/accountSubject"));
        if (mainWindow_) {
            mainWindow_->StopSessionEngine();
            mainWindow_->SetAccountSignedOut(
                QStringLiteral("RLink 账号已注销。"));
        }
        if (authManager_) {
            authManager_->SignOut();
        }
        EnsureLoginWindow();
        loginWindow_->ShowReady(
            QStringLiteral("RLink 账号已注销，本机登录凭据已清除。"));
        loginWindow_->ShowAndActivate();
        return;
    }

    if (!accountDeletionPending_) {
        return;
    }
    accountDeletionPending_ = false;
    EnsureLoginWindow();
    loginWindow_->ShowAuthenticated(accountLabel_, accountDetail_);
    loginWindow_->ShowAndActivate();
    controller::RemoteCDialog::Alert(
        loginWindow_.get(),
        QStringLiteral("账号未注销"),
        result.message.isEmpty()
            ? QStringLiteral("服务器未能完成账号注销，请稍后重试。")
            : result.message,
        QStringLiteral("知道了"),
        controller::RemoteCDialog::Tone::kWarning,
        true);
}

void RemoteCApplicationCoordinator::ExitApplication() {
    if (authManager_) {
        authManager_->Cancel();
    }
    if (mainWindow_) {
        mainWindow_->PrepareForApplicationExit();
    }
    QApplication::setQuitOnLastWindowClosed(true);
    QTimer::singleShot(0, qApp, [] { QApplication::quit(); });
}

}  // namespace remote::app
