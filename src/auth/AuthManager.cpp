// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#include "src/auth/AuthManager.h"

#include <QAbstractOAuth>
#include <QAbstractOAuth2>
#include <QDesktopServices>
#include <QHostAddress>
#include <QNetworkReply>
#include <QOAuth2AuthorizationCodeFlow>
#include <QOAuthHttpServerReplyHandler>
#include <QRandomGenerator>
#include <QVariantMap>

#include <limits>
#include <utility>

namespace remote::auth {
namespace {

QString OAuthErrorMessage(QAbstractOAuth::Error error) {
    switch (error) {
        case QAbstractOAuth::Error::NetworkError:
            return QStringLiteral("OAuth token request failed because of a "
                                  "network error.");
        case QAbstractOAuth::Error::ServerError:
            return QStringLiteral("OAuth provider returned a server error.");
        case QAbstractOAuth::Error::OAuthCallbackNotVerified:
            return QStringLiteral("OAuth callback state verification failed.");
        case QAbstractOAuth::Error::ClientError:
            return QStringLiteral("OAuth provider rejected the native client "
                                  "request.");
        case QAbstractOAuth::Error::ExpiredError:
            return QStringLiteral("OAuth credentials have expired.");
        default:
            return QStringLiteral("OAuth request failed.");
    }
}

bool IsTransientOAuthError(QAbstractOAuth::Error error) {
    return error == QAbstractOAuth::Error::NetworkError ||
           error == QAbstractOAuth::Error::ServerError;
}

}  // namespace

AuthManager::AuthManager(
    std::unique_ptr<TokenStore> tokenStore,
    QObject* parent)
    : QObject(parent),
      tokenStore_(std::move(tokenStore)),
      discovery_(&networkManager_, this) {
    operationTimer_.setSingleShot(true);
    refreshTimer_.setSingleShot(true);
    connect(&operationTimer_, &QTimer::timeout, this, [this]() {
        if (operation_ == Operation::kNone) {
            return;
        }
        const bool interactive =
            operation_ == Operation::kInteractiveLogin;
        discovery_.Cancel();
        CloseCallbackListener();
        Fail(AuthErrorCode::kAuthorizationTimedOut,
             interactive
                 ? QStringLiteral(
                       "登录等待时间过长，本次操作已取消。RemoteC 仍在运行，"
                       "你可以重新登录。")
                 : QStringLiteral(
                       "认证服务响应超时，RemoteC 将保留当前界面并允许重试。"),
             true);
    });
    connect(&refreshTimer_, &QTimer::timeout, this, [this]() {
        if (state_ == AuthState::kAuthenticated) {
            RefreshAccessToken();
        }
    });
}

AuthManager::~AuthManager() {
    callbacks_ = {};
    Cancel();
}

bool AuthManager::Configure(
    const AuthConfig& config,
    QString* errorMessage) {
    if (!tokenStore_) {
        if (errorMessage) {
            *errorMessage = QStringLiteral("AuthManager requires a token store.");
        }
        return false;
    }
    if (!config.Validate(errorMessage)) {
        return false;
    }
    Cancel();
    flow_.reset();
    metadata_ = {};
    tokens_ = {};
    config_ = config;
    SetState(AuthState::kSignedOut);
    return true;
}

void AuthManager::SetCallbacks(Callbacks callbacks) {
    callbacks_ = std::move(callbacks);
}

void AuthManager::StartInteractiveLogin() {
    if (state_ == AuthState::kUnconfigured) {
        Fail(AuthErrorCode::kInvalidConfiguration,
             QStringLiteral("OAuth is not configured."), false);
        return;
    }
    Cancel();
    BeginDiscovery(Operation::kInteractiveLogin);
}

void AuthManager::RestoreSession() {
    if (state_ == AuthState::kUnconfigured) {
        Fail(AuthErrorCode::kInvalidConfiguration,
             QStringLiteral("OAuth is not configured."), false);
        return;
    }
    Cancel();
    StoredRefreshToken stored;
    QString errorMessage;
    const TokenStoreLoadStatus loadStatus =
        tokenStore_->Load(&stored, &errorMessage);
    if (loadStatus == TokenStoreLoadStatus::kNotFound) {
        SetState(AuthState::kSignedOut);
        return;
    }
    if (loadStatus == TokenStoreLoadStatus::kError) {
        Fail(AuthErrorCode::kStoredTokenUnavailable,
             errorMessage.isEmpty()
                 ? QStringLiteral("Encrypted credentials could not be read.")
                 : errorMessage,
             false);
        return;
    }
    if (QUrl(stored.issuer) != config_.issuer ||
        stored.clientId != config_.clientId) {
        Fail(AuthErrorCode::kStoredTokenUnavailable,
             QStringLiteral("Stored credentials belong to a different Logto "
                            "application."),
             false);
        return;
    }
    operation_ = Operation::kRestoreSession;
    operationTimer_.start(config_.networkTimeoutMs * 3);
    SetState(AuthState::kDiscovering);
    discovery_.Discover(
        config_.issuer, config_.networkTimeoutMs,
        [this, stored = std::move(stored)](
            bool success,
            OidcMetadata metadata,
            QString errorMessage,
            bool transient) mutable {
            if (operation_ != Operation::kRestoreSession) {
                return;
            }
            if (!success) {
                Fail(AuthErrorCode::kDiscoveryFailed,
                     std::move(errorMessage), transient);
                return;
            }
            metadata_ = std::move(metadata);
            BuildFlow();
            BeginStoredTokenRefresh(stored);
        });
}

void AuthManager::RefreshAccessToken() {
    if (!flow_ || flow_->refreshToken().isEmpty() ||
        state_ != AuthState::kAuthenticated) {
        Fail(AuthErrorCode::kStoredTokenUnavailable,
             QStringLiteral("No refreshable authenticated session exists."),
             false);
        return;
    }
    operation_ = Operation::kRefresh;
    operationTimer_.start(config_.networkTimeoutMs * 2);
    SetState(AuthState::kRefreshing);
    flow_->refreshTokens();
}

void AuthManager::SignOut() {
    Cancel();
    if (flow_) {
        flow_->setToken({});
        flow_->setRefreshToken({});
    }
    tokens_ = {};
    QString errorMessage;
    if (tokenStore_ && !tokenStore_->Clear(&errorMessage)) {
        Fail(AuthErrorCode::kTokenStorageFailed,
             errorMessage.isEmpty()
                 ? QStringLiteral("Encrypted credentials could not be removed.")
                 : errorMessage,
             false);
        return;
    }
    SetState(AuthState::kSignedOut);
}

void AuthManager::Cancel() {
    const bool hadOperation = operation_ != Operation::kNone;
    operation_ = Operation::kNone;
    operationTimer_.stop();
    refreshTimer_.stop();
    discovery_.Cancel();
    AbortOutstandingNetworkReplies();
    CloseCallbackListener();
    expectedState_.clear();
    if (hadOperation && state_ != AuthState::kUnconfigured) {
        SetState(tokens_.HasUsableAccessToken()
                     ? AuthState::kAuthenticated
                     : AuthState::kSignedOut);
    }
}

void AuthManager::BeginDiscovery(Operation operation) {
    operation_ = operation;
    operationTimer_.start(operation == Operation::kInteractiveLogin
                              ? config_.interactiveTimeoutMs
                              : config_.networkTimeoutMs * 2);
    SetState(AuthState::kDiscovering);
    discovery_.Discover(
        config_.issuer, config_.networkTimeoutMs,
        [this, operation](bool success,
                          OidcMetadata metadata,
                          QString errorMessage,
                          bool transient) {
            if (operation_ != operation) {
                return;
            }
            if (!success) {
                Fail(AuthErrorCode::kDiscoveryFailed,
                     std::move(errorMessage), transient);
                return;
            }
            metadata_ = std::move(metadata);
            BuildFlow();
            if (operation == Operation::kInteractiveLogin) {
                BeginInteractiveGrant();
            }
        });
}

void AuthManager::BuildFlow() {
    flow_ = std::make_unique<QOAuth2AuthorizationCodeFlow>(
        &networkManager_);
    flow_->setClientIdentifier(config_.clientId);
    flow_->setAuthorizationUrl(metadata_.authorizationEndpoint);
    flow_->setTokenUrl(metadata_.tokenEndpoint);
    flow_->setRequestedScopeTokens(config_.scopes);
    flow_->setModifyParametersFunction(
        [](QAbstractOAuth::Stage stage,
           QMultiMap<QString, QVariant>* parameters) {
            if (stage == QAbstractOAuth::Stage::RequestingAuthorization &&
                parameters) {
                // Explicit user-initiated login must bypass the persistent
                // system-browser SSO cookie. Consent remains required with
                // offline_access so Logto returns a refresh token.
                parameters->replace(QStringLiteral("prompt"),
                                    QStringLiteral("login consent"));
            }
        });
    flow_->setPkceMethod(
        QOAuth2AuthorizationCodeFlow::PkceMethod::S256, 64);
    flow_->setNonceMode(QAbstractOAuth2::NonceMode::Automatic);
    flow_->setAutoRefresh(false);

    connect(flow_.get(), &QAbstractOAuth::authorizeWithBrowser, this,
            [this](const QUrl& url) {
                if (operation_ != Operation::kInteractiveLogin) {
                    return;
                }
                if (!QDesktopServices::openUrl(url)) {
                    Fail(AuthErrorCode::kBrowserLaunchFailed,
                         QStringLiteral("The system browser could not be "
                                        "opened for Logto login."),
                         false);
                }
            });
    connect(flow_.get(), &QAbstractOAuth2::authorizationCallbackReceived,
            this, [this](const QVariantMap& data) {
                if (operation_ != Operation::kInteractiveLogin) {
                    return;
                }
                const QString callbackState =
                    data.value(QStringLiteral("state")).toString();
                // QAbstractOAuth2 also verifies its state property internally.
                // Keep this explicit comparison so the RemoteC state machine
                // independently rejects missing or mismatched callbacks.
                if (callbackState.isEmpty() ||
                    callbackState != expectedState_) {
                    Fail(AuthErrorCode::kStateMismatch,
                         QStringLiteral("OAuth callback state did not match "
                                        "the authorization request."),
                         false);
                    return;
                }
                SetState(AuthState::kExchangingCode);
            });
    connect(flow_.get(), &QAbstractOAuth::statusChanged, this,
            [this](QAbstractOAuth::Status status) {
                if (operation_ == Operation::kNone) {
                    return;
                }
                if (status == QAbstractOAuth::Status::RefreshingToken) {
                    SetState(AuthState::kRefreshing);
                } else if (status == QAbstractOAuth::Status::Granted) {
                    CompleteAuthentication();
                }
            });
    connect(flow_.get(), &QAbstractOAuth::requestFailed, this,
            [this](QAbstractOAuth::Error error) {
                if (operation_ == Operation::kNone) {
                    return;
                }
                if (error == QAbstractOAuth::Error::OAuthCallbackNotVerified) {
                    Fail(AuthErrorCode::kStateMismatch,
                         OAuthErrorMessage(error), false);
                    return;
                }
                const AuthErrorCode code =
                    operation_ == Operation::kRestoreSession ||
                            operation_ == Operation::kRefresh
                        ? AuthErrorCode::kRefreshRejected
                        : AuthErrorCode::kTokenRequestFailed;
                Fail(code, OAuthErrorMessage(error),
                     IsTransientOAuthError(error));
            });
    connect(flow_.get(), &QAbstractOAuth2::serverReportedErrorOccurred,
            this, [this](const QString& error,
                         const QString& description,
                         const QUrl&) {
                if (operation_ == Operation::kNone) {
                    return;
                }
                const bool invalidGrant =
                    error.compare(QStringLiteral("invalid_grant"),
                                  Qt::CaseInsensitive) == 0;
                if (invalidGrant && tokenStore_) {
                    tokenStore_->Clear(nullptr);
                }
                const AuthErrorCode code = invalidGrant
                    ? AuthErrorCode::kRefreshRejected
                    : AuthErrorCode::kAuthorizationDenied;
                Fail(code,
                     description.isEmpty()
                         ? QStringLiteral("Logto rejected the OAuth request: %1")
                               .arg(error)
                         : description,
                     false);
            });
}

void AuthManager::BeginInteractiveGrant() {
    const quint16 port = static_cast<quint16>(config_.callbackUrl.port());
    replyHandler_ = std::make_unique<QOAuthHttpServerReplyHandler>(
        QHostAddress::LocalHost, port);
    replyHandler_->setCallbackPath(config_.callbackUrl.path());
    replyHandler_->setCallbackText(QStringLiteral(R"HTML(
<!doctype html>
<html lang="zh-CN">
<head>
  <meta charset="utf-8">
  <meta name="viewport" content="width=device-width,initial-scale=1">
  <title>RemoteC 授权已完成</title>
  <style>
    *{box-sizing:border-box}html,body{height:100%;margin:0}
    body{display:grid;place-items:center;padding:32px;overflow-x:hidden;background:
      radial-gradient(circle at 7% 5%,#e8e7ff 0 9%,transparent 9.2%),
      radial-gradient(circle at 96% 91%,#d9f8e9 0 10%,transparent 10.2%),
      linear-gradient(125deg,#f7f8ff 0%,#ffffff 48%,#f2fcf8 100%);
      color:#172033;font-family:"Segoe UI","Microsoft YaHei",sans-serif}
    .card{position:relative;width:min(650px,100%);overflow:hidden;text-align:center;
      background:rgba(255,255,255,.94);border:1px solid #e1e6ef;
      border-radius:28px;box-shadow:0 28px 80px rgba(36,50,78,.14)}
    .content{padding:44px 64px 38px}
    .brand{display:flex;align-items:center;justify-content:center;gap:14px;margin-bottom:42px}
    .brand-text{text-align:left}.brand-name{font-size:24px;font-weight:780;letter-spacing:.1px}
    .tagline{margin-top:2px;color:#7a8495;font-size:12px;letter-spacing:.6px}
    .logo{display:grid;place-items:center;width:54px;height:54px;border-radius:15px;
      background:linear-gradient(145deg,#687bff,#4357e8);box-shadow:0 8px 20px #d8ddff;
      color:#fff;font-size:27px;font-weight:750}
    .success{position:relative;width:132px;height:132px;margin:0 auto 27px;display:grid;place-items:center}
    .ring{position:absolute;inset:0;border-radius:50%;background:#e8f8ef;animation:ring 1.8s ease-out infinite}
    .ring.two{inset:14px;background:#d8f4e5;animation-delay:.2s}
    .check-circle{position:relative;display:grid;place-items:center;width:88px;height:88px;
      border-radius:50%;background:linear-gradient(145deg,#35c979,#19aa5d);
      box-shadow:0 12px 28px rgba(28,174,96,.27);animation:pop .62s cubic-bezier(.2,.9,.25,1.25) both}
    .check-circle svg{width:48px;height:48px;overflow:visible}
    .check-path{fill:none;stroke:#fff;stroke-width:7;stroke-linecap:round;stroke-linejoin:round;
      stroke-dasharray:70;stroke-dashoffset:70;animation:draw .55s .38s ease-out forwards}
    .spark{position:absolute;width:7px;height:7px;border-radius:2px;opacity:0;animation:spark 1.3s .45s ease-out both}
    .s1{left:-16px;top:45px;background:#6380ff}.s2{right:-20px;top:28px;background:#55d490}
    .s3{left:4px;bottom:-2px;background:#9565ef}.s4{right:0;bottom:9px;background:#71c9ff}
    h1{margin:0 0 10px;font-size:32px;line-height:1.25;letter-spacing:.3px}
    .returned{margin:0 0 17px;color:#1aa762;font-size:17px;font-weight:700}
    p{margin:0;color:#667085;font-size:14px;line-height:1.85}
    .open{display:flex;align-items:center;justify-content:center;gap:10px;width:min(330px,100%);
      min-height:52px;margin:28px auto 0;border-radius:14px;background:linear-gradient(100deg,#2878ef,#7b3ff2);
      box-shadow:0 12px 25px rgba(78,75,230,.22);color:#fff;text-decoration:none;
      font-size:16px;font-weight:700;transition:transform .18s,box-shadow .18s}
    .open:hover{transform:translateY(-2px);box-shadow:0 16px 31px rgba(78,75,230,.29)}
    .open:active{transform:translateY(0)}
    .open svg{width:21px;height:21px;fill:none;stroke:currentColor;stroke-width:1.8}
    .open.expired{pointer-events:none;cursor:default;filter:brightness(.58) saturate(.5);opacity:.58;
      animation:returnDim 2.4s cubic-bezier(.22,1,.36,1) forwards}
    .open.expired:hover{transform:none;box-shadow:0 5px 12px rgba(55,58,116,.08)}
    .manual{margin-top:22px;color:#8a93a3;font-size:12px}
    .countdown{display:inline-block;min-width:20px;color:#1aa762;font-weight:800;font-size:13px}
    .footer{display:flex;align-items:center;justify-content:center;gap:12px;padding:24px 34px;
      border-top:1px solid #edf0f5;background:#fbfcfe;color:#687386;font-size:12px;text-align:left}
    .shield{display:grid;place-items:center;width:34px;height:34px;border-radius:50%;background:#e8f8ef;color:#169454;font-size:17px}
    @keyframes pop{0%{transform:scale(.25);opacity:0}70%{transform:scale(1.08);opacity:1}100%{transform:scale(1)}}
    @keyframes draw{to{stroke-dashoffset:0}}
    @keyframes ring{0%{transform:scale(.78);opacity:.9}75%,100%{transform:scale(1.18);opacity:0}}
    @keyframes spark{0%{transform:scale(.2) rotate(0);opacity:0}25%{opacity:1}100%{transform:scale(1.2) rotate(110deg);opacity:0}}
    @keyframes returnDim{
      0%{filter:brightness(1) saturate(1);opacity:1;box-shadow:0 12px 25px rgba(78,75,230,.22)}
      45%{filter:brightness(.82) saturate(.8);opacity:.86;box-shadow:0 9px 20px rgba(65,64,175,.15)}
      100%{filter:brightness(.58) saturate(.5);opacity:.58;box-shadow:0 5px 12px rgba(55,58,116,.08)}
    }
    @media(max-width:560px){body{padding:15px}.content{padding:34px 24px 30px}.card{border-radius:22px}h1{font-size:27px}}
    @media(prefers-reduced-motion:reduce){*{animation:none!important;transition:none!important}.check-path{stroke-dashoffset:0}.spark{display:none}}
  </style>
</head>
<body>
  <main class="card">
    <section class="content">
      <div class="brand"><span class="logo">R</span><span class="brand-text"><span class="brand-name">RemoteC</span><br><span class="tagline">安全 · 可靠 · 高效</span></span></div>
      <div class="success">
        <span class="ring"></span><span class="ring two"></span>
        <span class="spark s1"></span><span class="spark s2"></span><span class="spark s3"></span><span class="spark s4"></span>
        <span class="check-circle"><svg viewBox="0 0 56 56" aria-hidden="true"><path class="check-path" d="M13 29l10 10 21-23"/></svg></span>
      </div>
      <h1>登录成功</h1>
      <div class="returned">可安全返回 RemoteC</div>
      <p>你的身份验证已经完成，RemoteC 正在安全接管登录流程。<br>你可以返回应用继续使用。</p>
      <a class="open" href="remotec://auth-complete" aria-label="返回 RemoteC">
        <svg viewBox="0 0 24 24" aria-hidden="true"><rect x="3" y="4" width="18" height="13" rx="2"/><path d="M8 21h8M12 17v4"/></svg>
        返回 RemoteC
      </a>
      <div class="manual" id="close-status">点击按钮回到已经运行的 RemoteC，本页面将在 <span class="countdown" id="countdown">15</span> 秒后自动关闭</div>
    </section>
    <footer class="footer"><span class="shield">&#10003;</span><span>由 Logto 提供安全身份认证<br>密码和登录凭据始终由 Logto 安全处理</span></footer>
  </main>
  <script>
    (() => {
      const status = document.getElementById('close-status');
      const countdown = document.getElementById('countdown');
      const returnButton = document.querySelector('.open');
      let seconds = 15;
      const updateStatus = () => {
        countdown.textContent = seconds;
      };
      const closePage = () => {
        window.close();
        window.setTimeout(() => {
          if (!window.closed) {
            status.textContent = '浏览器阻止了自动关闭，你现在可以安全地手动关闭此页面';
          }
        }, 400);
      };
      returnButton.addEventListener('click', () => window.setTimeout(closePage, 500));
      updateStatus();
      const timer = window.setInterval(() => {
        seconds -= 1;
        if (seconds <= 0) {
          window.clearInterval(timer);
          returnButton.removeAttribute('href');
          returnButton.setAttribute('aria-disabled', 'true');
          returnButton.tabIndex = -1;
          returnButton.classList.add('expired');
          status.textContent = '正在关闭此页面…';
          closePage();
          return;
        }
        updateStatus();
      }, 1000);
    })();
  </script>
</body>
</html>)HTML"));
    if (!replyHandler_->isListening() &&
        !replyHandler_->listen(QHostAddress::LocalHost, port)) {
        Fail(AuthErrorCode::kCallbackListenerFailed,
             QStringLiteral("Unable to listen on 127.0.0.1:18765 for the "
                            "OAuth callback."),
             false);
        return;
    }
    if (QUrl(replyHandler_->callback()) != config_.callbackUrl) {
        Fail(AuthErrorCode::kCallbackListenerFailed,
             QStringLiteral("OAuth callback listener did not bind the exact "
                            "configured redirect URI."),
             false);
        return;
    }
    flow_->setReplyHandler(replyHandler_.get());
    expectedState_ = GenerateState();
    flow_->setState(expectedState_);
    SetState(AuthState::kAuthorizing);
    flow_->grant();
}

void AuthManager::BeginStoredTokenRefresh(
    const StoredRefreshToken& storedToken) {
    flow_->setRefreshToken(storedToken.refreshToken);
    SetState(AuthState::kRefreshing);
    flow_->refreshTokens();
}

void AuthManager::CompleteAuthentication() {
    if (!flow_ || operation_ == Operation::kNone || flow_->token().isEmpty()) {
        return;
    }
    const QString refreshToken = flow_->refreshToken();
    if (refreshToken.isEmpty()) {
        Fail(AuthErrorCode::kTokenStorageFailed,
             QStringLiteral("Logto did not return a refresh token. Ensure "
                            "offline_access is enabled for the native app."),
             false);
        return;
    }

    StoredRefreshToken stored;
    stored.issuer = config_.issuer.toString(QUrl::FullyEncoded);
    stored.clientId = config_.clientId;
    stored.refreshToken = refreshToken;
    stored.savedAtUtc = QDateTime::currentDateTimeUtc();
    QString errorMessage;
    if (!tokenStore_->Save(stored, &errorMessage)) {
        Fail(AuthErrorCode::kTokenStorageFailed,
             errorMessage.isEmpty()
                 ? QStringLiteral("Refresh token could not be encrypted.")
                 : errorMessage,
             false);
        return;
    }

    tokens_.accessToken = flow_->token();
    tokens_.idToken = flow_->idToken();
    tokens_.tokenType = flow_->extraTokens()
                            .value(QStringLiteral("token_type"),
                                   QStringLiteral("Bearer"))
                            .toString();
    tokens_.grantedScopes = flow_->grantedScopeTokens();
    tokens_.expiresAtUtc = flow_->expirationAt().toUTC();
    operationTimer_.stop();
    CloseCallbackListener();
    expectedState_.clear();
    operation_ = Operation::kNone;
    SetState(AuthState::kAuthenticated);
    refreshTimer_.stop();
    if (tokens_.expiresAtUtc.isValid()) {
        const QDateTime refreshAt = tokens_.expiresAtUtc.addSecs(-60);
        const qint64 delayMs = qMax<qint64>(
            0, QDateTime::currentDateTimeUtc().msecsTo(refreshAt));
        refreshTimer_.start(static_cast<int>(qMin<qint64>(
            delayMs, std::numeric_limits<int>::max())));
    }
    if (callbacks_.authenticated) {
        callbacks_.authenticated(tokens_);
    }
}

void AuthManager::SetState(AuthState state) {
    if (state_ == state) {
        return;
    }
    state_ = state;
    if (callbacks_.stateChanged) {
        callbacks_.stateChanged(state_);
    }
}

void AuthManager::Fail(
    AuthErrorCode code,
    QString message,
    bool transient) {
    operationTimer_.stop();
    refreshTimer_.stop();
    operation_ = Operation::kNone;
    discovery_.Cancel();
    AbortOutstandingNetworkReplies();
    CloseCallbackListener();
    expectedState_.clear();
    if (!transient &&
        (code == AuthErrorCode::kRefreshRejected ||
         code == AuthErrorCode::kTokenStorageFailed)) {
        if (tokenStore_) {
            tokenStore_->Clear(nullptr);
        }
        if (flow_) {
            flow_->setToken({});
            flow_->setRefreshToken({});
        }
        tokens_ = {};
    }
    SetState(AuthState::kError);
    if (callbacks_.failed) {
        callbacks_.failed(AuthError{code, std::move(message), transient});
    }
}

void AuthManager::CloseCallbackListener() {
    if (replyHandler_) {
        replyHandler_->close();
    }
}

void AuthManager::AbortOutstandingNetworkReplies() {
    const auto replies = networkManager_.findChildren<QNetworkReply*>(
        QString(), Qt::FindDirectChildrenOnly);
    for (QNetworkReply* reply : replies) {
        if (reply && reply->isRunning()) {
            reply->abort();
        }
    }
}

QString AuthManager::GenerateState() {
    QByteArray randomBytes(32, Qt::Uninitialized);
    QRandomGenerator* generator = QRandomGenerator::system();
    for (qsizetype index = 0; index < randomBytes.size(); ++index) {
        randomBytes[index] = static_cast<char>(generator->generate() & 0xffu);
    }
    return QString::fromLatin1(randomBytes.toBase64(
        QByteArray::Base64UrlEncoding | QByteArray::OmitTrailingEquals));
}

}  // namespace remote::auth
