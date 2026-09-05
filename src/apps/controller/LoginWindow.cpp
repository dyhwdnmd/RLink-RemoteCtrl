// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#include "src/apps/controller/LoginWindow.h"
#include "src/apps/controller/FramelessWindow.h"
#include "src/apps/controller/ui/RemoteCTheme.h"

#include <algorithm>

#include <QCloseEvent>
#include <QApplication>
#include <QEvent>
#include <QFrame>
#include <QHBoxLayout>
#include <QIcon>
#include <QLabel>
#include <QMouseEvent>
#include <QPainter>
#include <QPixmap>
#include <QPointF>
#include <QProgressBar>
#include <QPushButton>
#include <QEasingCurve>
#include <QScreen>
#include <QStyle>
#include <QTimer>
#include <QGuiApplication>
#include <QVBoxLayout>
#include <QWindow>

#ifdef Q_OS_WIN
#include <qt_windows.h>
#endif

#include <utility>

namespace remote::controller {

namespace {

const QEasingCurve& AccountRevealEasing()
{
    static const QEasingCurve curve = [] {
        QEasingCurve value(QEasingCurve::BezierSpline);
        value.addCubicBezierSegment(
            QPointF(0.25, 0.10), QPointF(0.25, 1.0), QPointF(1.0, 1.0));
        return value;
    }();
    return curve;
}

const QEasingCurve& AccountDismissEasing()
{
    static const QEasingCurve curve = [] {
        QEasingCurve value(QEasingCurve::BezierSpline);
        value.addCubicBezierSegment(
            QPointF(0.42, 0.0), QPointF(1.0, 1.0), QPointF(1.0, 1.0));
        return value;
    }();
    return curve;
}

}  // namespace

LoginStatusWindow::LoginStatusWindow(QWidget* parent)
    : QDialog(parent) {
    setObjectName(QStringLiteral("loginStatusWindow"));
    setWindowTitle(QStringLiteral("RLink 正在登录"));
    setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint |
                   Qt::WindowStaysOnTopHint);
    setModal(false);
    setFixedSize(390, 190);
    setStyleSheet(QStringLiteral(R"(
QDialog#loginStatusWindow {
    background:#FFFEFB;
    border:1px solid #DDE0E4;
    border-radius:20px;
    color:#172033;
}
QLabel#statusLogo {
    background:transparent;border:none;
}
QLabel#statusAccount { color:#172033;font-size:18px;font-weight:750; }
QLabel#statusText { color:#536176;font-size:13px; }
QProgressBar {
    min-height:4px;max-height:4px;border:0;border-radius:2px;
    background:#E7E1D8;
}
QProgressBar::chunk { background:#5264f5; }
)"));
    setProperty("remoteCLightStyleSheet", styleSheet());
    if (ui::RemoteCTheme::IsDark(ui::RemoteCTheme::LoadPreference())) {
        setStyleSheet(styleSheet() + QStringLiteral(R"(
QDialog#loginStatusWindow {
    background:#151F2E; border-color:#34445B; color:#EEF3FA;
}
QLabel#statusAccount { color:#EEF3FA; }
QLabel#statusText { color:#9FACBE; }
QProgressBar { background:#263246; }
)"));
    }
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(30, 26, 30, 25);
    layout->setSpacing(13);
    auto* row = new QHBoxLayout();
    row->setSpacing(14);
    auto* logo = new QLabel(this);
    logo->setObjectName(QStringLiteral("statusLogo"));
    logo->setFixedSize(48, 48);
    logo->setAlignment(Qt::AlignCenter);
    logo->setPixmap(CreateRemoteCIcon().pixmap(42, 42));
    logo->setAttribute(Qt::WA_TransparentForMouseEvents);
    auto* labels = new QVBoxLayout();
    labels->setSpacing(3);
    accountLabel_ = new QLabel(QStringLiteral("RLink 账户"), this);
    accountLabel_->setObjectName(QStringLiteral("statusAccount"));
    accountLabel_->setAttribute(Qt::WA_TransparentForMouseEvents);
    auto* status = new QLabel(QStringLiteral("正在登录中…"), this);
    status->setObjectName(QStringLiteral("statusText"));
    status->setAttribute(Qt::WA_TransparentForMouseEvents);
    labels->addWidget(accountLabel_);
    labels->addWidget(status);
    row->addWidget(logo);
    row->addLayout(labels, 1);
    auto* progress = new QProgressBar(this);
    progress->setRange(0, 0);
    progress->setTextVisible(false);
    progress->setAttribute(Qt::WA_TransparentForMouseEvents);
    auto* hint = new QLabel(
        QStringLiteral("正在安全验证账户并恢复 RLink 会话"), this);
    hint->setObjectName(QStringLiteral("statusText"));
    hint->setAlignment(Qt::AlignCenter);
    hint->setAttribute(Qt::WA_TransparentForMouseEvents);
    layout->addLayout(row);
    layout->addWidget(progress);
    layout->addWidget(hint);
    RefreshThemeStyle();
}

void LoginStatusWindow::SetAccountLabel(const QString& accountLabel) {
    accountLabel_->setText(accountLabel.trimmed().isEmpty()
                               ? QStringLiteral("RLink 账户")
                               : accountLabel.trimmed());
}

void LoginStatusWindow::RefreshThemeStyle()
{
    QString style = property("remoteCLightStyleSheet").toString();
    if (ui::RemoteCTheme::IsDark(ui::RemoteCTheme::LoadPreference())) {
        style += QStringLiteral(R"(
QDialog#loginStatusWindow {
    background:#141E2D; border-color:#2B3A50; color:#F1F5FB;
}
QLabel#statusAccount { color:#F1F5FB; }
QLabel#statusText { color:#A8B5C7; }
QProgressBar { background:#202B3B; }
QProgressBar::chunk { background:#6F86FF; }
)");
    }
    setProperty("themeRoot",
                ui::RemoteCTheme::IsDark(ui::RemoteCTheme::LoadPreference())
                    ? QStringLiteral("dark") : QStringLiteral("light"));
    setStyleSheet(style);
}

void LoginStatusWindow::ShowAndActivate() {
    QRect available;
    if (const QScreen* screen = QGuiApplication::primaryScreen()) {
        available = screen->availableGeometry();
    }
    if (available.isValid()) {
        move(available.center() - rect().center());
    }
    show();
    raise();
    activateWindow();
#ifdef Q_OS_WIN
    // WindowStaysOnTopHint maps to a topmost window, but explicitly promote
    // the native HWND after creation as well. This avoids a startup race in
    // which another foreground application can cover the restore-status card.
    SetWindowPos(reinterpret_cast<HWND>(winId()), HWND_TOPMOST,
                 0, 0, 0, 0,
                 SWP_NOMOVE | SWP_NOSIZE | SWP_SHOWWINDOW);
#endif
}

void LoginStatusWindow::HideAndReleaseTopmost() {
#ifdef Q_OS_WIN
    if (winId()) {
        SetWindowPos(reinterpret_cast<HWND>(winId()), HWND_NOTOPMOST,
                     0, 0, 0, 0,
                     SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
    }
#endif
    hide();
}

void LoginStatusWindow::mousePressEvent(QMouseEvent* event) {
    if (event && event->button() == Qt::LeftButton && windowHandle() &&
        windowHandle()->startSystemMove()) {
        event->accept();
        return;
    }
    QDialog::mousePressEvent(event);
}

LoginWindow::LoginWindow(QWidget* parent)
    : QDialog(parent) {
    setObjectName(QStringLiteral("loginWindow"));
    BuildUi();
    RefreshThemeStyle();
    if (parent) {
        SetOwnerWindow(parent);
    }
    ShowReady();
}

void LoginWindow::SetLoginRequestedCallback(
    std::function<void()> callback) {
    loginRequested_ = std::move(callback);
}

void LoginWindow::SetCancelRequestedCallback(
    std::function<void()> callback) {
    cancelRequested_ = std::move(callback);
}

void LoginWindow::SetExitRequestedCallback(
    std::function<void()> callback) {
    exitRequested_ = std::move(callback);
}

void LoginWindow::SetSignOutRequestedCallback(
    std::function<void()> callback) {
    signOutRequested_ = std::move(callback);
}

void LoginWindow::SetDeleteAccountRequestedCallback(
    std::function<void()> callback) {
    deleteAccountRequested_ = std::move(callback);
}

void LoginWindow::SetOwnerWindow(QWidget* owner) {
    if (ownerWindow_) {
        ownerWindow_->removeEventFilter(this);
    }
    ownerWindow_ = owner;
    if (ownerWindow_) {
        setParent(ownerWindow_);
        setWindowFlags(Qt::Widget | Qt::FramelessWindowHint);
        setGeometry(ownerWindow_->rect());
        ownerWindow_->installEventFilter(this);
    }
}

void LoginWindow::ShowReady(const QString& message) {
    preferredCardSize_ = QSize(620, 520);
    titleLabel_->setText(QStringLiteral("登录你的 RLink 账户"));
    stateIcon_->setText(QStringLiteral("i"));
    SetStatePanelTone(QStringLiteral("ready"));
    statusLabel_->setText(message.isEmpty()
                              ? QStringLiteral(
                                    "登录将在系统浏览器中安全完成")
                              : message);
    securityLabel_->show();
    progressBar_->hide();
    loginButton_->setText(QStringLiteral("使用浏览器登录"));
    loginButton_->setEnabled(true);
    cancelButton_->show();
    cancelButton_->setEnabled(false);
    signOutButton_->hide();
    deleteAccountButton_->hide();
    loginButton_->show();
}

void LoginWindow::ShowBusy(
    const QString& title,
    const QString& message) {
    preferredCardSize_ = QSize(620, 520);
    titleLabel_->setText(title);
    stateIcon_->setText(QStringLiteral("…"));
    SetStatePanelTone(QStringLiteral("busy"));
    statusLabel_->setText(message);
    securityLabel_->show();
    progressBar_->show();
    loginButton_->setEnabled(false);
    cancelButton_->show();
    cancelButton_->setEnabled(true);
    signOutButton_->hide();
    deleteAccountButton_->hide();
    loginButton_->show();
}

void LoginWindow::ShowAccountDeletionBusy() {
    ShowBusy(
        QStringLiteral("正在注销 RLink 账号"),
        QStringLiteral(
            "正在安全删除 Logto 身份和 RLink 服务器数据，请勿关闭软件。"));
    cancelButton_->hide();
    loginButton_->hide();
}

void LoginWindow::ShowError(
    const QString& message,
    bool retryable) {
    preferredCardSize_ = QSize(620, 520);
    titleLabel_->setText(QStringLiteral("暂时无法完成登录"));
    stateIcon_->setText(QStringLiteral("!"));
    SetStatePanelTone(QStringLiteral("error"));
    statusLabel_->setText(message);
    securityLabel_->show();
    progressBar_->hide();
    loginButton_->setText(QStringLiteral("重试登录"));
    loginButton_->setEnabled(retryable);
    cancelButton_->show();
    cancelButton_->setEnabled(false);
    signOutButton_->hide();
    deleteAccountButton_->hide();
    loginButton_->show();
}

void LoginWindow::ShowAuthenticated(
    const QString& accountLabel,
    const QString& accountDetail) {
    preferredCardSize_ = QSize(600, 470);
    titleLabel_->setText(QStringLiteral("你的 RLink 账户"));
    const QString trimmedLabel = accountLabel.trimmed();
    stateIcon_->setText(trimmedLabel.isEmpty()
                            ? QStringLiteral("R")
                            : trimmedLabel.left(1).toUpper());
    SetStatePanelTone(QStringLiteral("authenticated"));
    const QString safeLabel = accountLabel.toHtmlEscaped();
    const QString safeDetail = accountDetail.toHtmlEscaped();
    authenticatedAccountLabel_ = safeLabel;
    authenticatedAccountDetail_ = safeDetail;
    UpdateAuthenticatedAccountText();
    securityLabel_->hide();
    progressBar_->hide();
    loginButton_->hide();
    cancelButton_->hide();
    signOutButton_->show();
    deleteAccountButton_->show();
}

void LoginWindow::ShowAndActivate() {
    const bool reversingHide = IsVisibilityMotionActive() && hiding_;
    const QPoint interruptedPosition = cardLayer_
        ? cardLayer_->pos()
        : QPoint();
    StopVisibilityMotion();
    hiding_ = false;
    if (ownerWindow_) {
        setGeometry(ownerWindow_->rect());
    }
    LayoutCardLayer();
    const QRect targetGeometry = CardTargetGeometry();
    const QPoint targetPosition = targetGeometry.topLeft();
    const QPoint startPosition = reversingHide
        ? interruptedPosition
        : targetPosition + QPoint(0, 30);
    cardLayer_->setGeometry(targetGeometry);
    cardLayer_->move(startPosition);
    card_->setGraphicsEffect(nullptr);
    show();
    raise();
    cardLayer_->raise();
    if (ownerWindow_) {
        ownerWindow_->activateWindow();
    }

    const int animationLevel = CurrentUiAnimationLevel();
    if (animationLevel <= 0 || startPosition == targetPosition) {
        cardLayer_->move(targetPosition);
        return;
    }
    StartVisibilityMotion(
        targetPosition, animationLevel == 1 ? 170 : 235);
}

void LoginWindow::HideWithAnimation() {
    if (!isVisible() || hiding_) {
        return;
    }
    dragging_ = false;
    if (dragHandle_) {
        dragHandle_->setCursor(Qt::OpenHandCursor);
    }
    raise();
    cardLayer_->raise();
    StopVisibilityMotion();
    hiding_ = true;
    const QPoint startPosition = cardLayer_->pos();
    lastCardPosition_ = startPosition;
    hasLastCardPosition_ = true;
    const QPoint endPosition = startPosition + QPoint(0, 46);
    card_->setGraphicsEffect(nullptr);
    const int animationLevel = CurrentUiAnimationLevel();
    if (animationLevel <= 0) {
        hide();
        cardLayer_->setGeometry(CardTargetGeometry());
        hiding_ = false;
        return;
    }
    StartVisibilityMotion(
        endPosition, animationLevel == 1 ? 140 : 180);
}

bool LoginWindow::IsHiding() const {
    return hiding_;
}

QRect LoginWindow::CardTargetGeometry() const {
    const QSize layerSize = preferredCardSize_.boundedTo(
        size() - QSize(36, 36));
    QRect target(QPoint(), layerSize);
    constexpr int margin = 8;
    if (hasLastCardPosition_) {
        target.moveTopLeft(QPoint(
            std::clamp(lastCardPosition_.x(), margin,
                       std::max(margin, width() - layerSize.width() - margin)),
            std::clamp(lastCardPosition_.y(), margin,
                       std::max(margin, height() - layerSize.height() - margin))));
    } else {
        // The account sheet looks top-heavy when it is mathematically
        // centered because its header carries most of the visual weight.
        // Place the initial card slightly below center while keeping it fully
        // inside compact displays.
        target.moveCenter(rect().center() + QPoint(0, 28));
        target.moveTop(std::clamp(
            target.top(), margin,
            std::max(margin, height() - layerSize.height() - margin)));
    }
    return target;
}

void LoginWindow::LayoutCardLayer() {
    const QSize cardSize = preferredCardSize_.boundedTo(
        size() - QSize(36, 36));
    cardLayer_->resize(cardSize);
    card_->setGeometry(QRect(QPoint(0, 0), cardSize));
}

void LoginWindow::StartVisibilityMotion(const QPoint& targetPosition,
                                        int durationMs)
{
    visibilityMotionStart_ = cardLayer_->pos();
    visibilityMotionTarget_ = targetPosition;
    visibilityMotionDurationMs_ = std::max(1, durationMs);
    visibilityMotionClock_.restart();

    if (!visibilityMotionTimer_) {
        visibilityMotionTimer_ = new QTimer(this);
        visibilityMotionTimer_->setTimerType(Qt::PreciseTimer);
        visibilityMotionTimer_->setInterval(8);
        connect(visibilityMotionTimer_, &QTimer::timeout, this, [this] {
            if (visibilityMotionDurationMs_ <= 0) {
                visibilityMotionTimer_->stop();
                return;
            }
            const qreal progress = std::min<qreal>(
                1.0,
                static_cast<qreal>(visibilityMotionClock_.elapsed()) /
                    visibilityMotionDurationMs_);
            const qreal eased = (hiding_
                ? AccountDismissEasing()
                : AccountRevealEasing()).valueForProgress(progress);
            const QPoint delta =
                visibilityMotionTarget_ - visibilityMotionStart_;
            cardLayer_->move(visibilityMotionStart_ + QPoint(
                qRound(delta.x() * eased), qRound(delta.y() * eased)));
            if (progress < 1.0) return;

            visibilityMotionTimer_->stop();
            cardLayer_->move(visibilityMotionTarget_);
            visibilityMotionDurationMs_ = 0;
            if (hiding_) {
                hide();
                cardLayer_->setGeometry(CardTargetGeometry());
                hiding_ = false;
            }
        });
    }
    visibilityMotionTimer_->start();
}

void LoginWindow::StopVisibilityMotion()
{
    if (visibilityMotionTimer_) visibilityMotionTimer_->stop();
    visibilityMotionDurationMs_ = 0;
}

bool LoginWindow::IsVisibilityMotionActive() const
{
    return visibilityMotionTimer_ && visibilityMotionTimer_->isActive();
}

void LoginWindow::closeEvent(QCloseEvent* event) {
    // The tray-exit path enables quit-on-last-window-closed immediately
    // before QApplication::quit(). Do not let the cosmetic hide animation
    // veto that real application shutdown and leave a headless process.
    if (QApplication::quitOnLastWindowClosed()) {
        event->accept();
        return;
    }
    if (isVisible() && !hiding_) {
        event->ignore();
        HideWithAnimation();
        return;
    }
    event->accept();
}

bool LoginWindow::eventFilter(QObject* watched, QEvent* event) {
    if (watched == dragHandle_ && event && card_) {
        auto* mouseEvent = dynamic_cast<QMouseEvent*>(event);
        if (mouseEvent && event->type() == QEvent::MouseButtonPress &&
            mouseEvent->button() == Qt::LeftButton &&
            !IsVisibilityMotionActive() && !hiding_) {
            dragging_ = true;
            dragOffset_ = mouseEvent->globalPosition().toPoint() -
                cardLayer_->mapToGlobal(QPoint(0, 0));
            dragHandle_->setCursor(Qt::ClosedHandCursor);
            return true;
        }
        if (mouseEvent && event->type() == QEvent::MouseMove &&
            dragging_) {
            QPoint target = mapFromGlobal(
                mouseEvent->globalPosition().toPoint()) - dragOffset_;
            constexpr int margin = 12;
            target.setX(std::clamp(
                target.x(), margin,
                std::max(margin, width() - cardLayer_->width() - margin)));
            target.setY(std::clamp(
                target.y(), margin,
                std::max(margin, height() - cardLayer_->height() - margin)));
            cardLayer_->move(target);
            lastCardPosition_ = target;
            hasLastCardPosition_ = true;
            return true;
        }
        if (mouseEvent && event->type() == QEvent::MouseButtonRelease &&
            mouseEvent->button() == Qt::LeftButton && dragging_) {
            dragging_ = false;
            dragHandle_->setCursor(Qt::OpenHandCursor);
            return true;
        }
    }
    if (watched == ownerWindow_.data() && event &&
        event->type() == QEvent::Resize) {
        setGeometry(ownerWindow_->rect());
        if (!IsVisibilityMotionActive()) {
            cardLayer_->setGeometry(CardTargetGeometry());
        }
    }
    return QDialog::eventFilter(watched, event);
}

void LoginWindow::SetStatePanelTone(const QString& tone) {
    statePanel_->setProperty("tone", tone);
    stateIcon_->setProperty("tone", tone);
    statusLabel_->setProperty("tone", tone);
    statePanel_->style()->unpolish(statePanel_);
    statePanel_->style()->polish(statePanel_);
    stateIcon_->style()->unpolish(stateIcon_);
    stateIcon_->style()->polish(stateIcon_);
    statusLabel_->style()->unpolish(statusLabel_);
    statusLabel_->style()->polish(statusLabel_);
}

void LoginWindow::mousePressEvent(QMouseEvent* event) {
    if (event && event->button() == Qt::LeftButton && cardLayer_ &&
        !cardLayer_->geometry().contains(event->position().toPoint())) {
        HideWithAnimation();
        event->accept();
        return;
    }
    QDialog::mousePressEvent(event);
}

void LoginWindow::BuildUi() {
    setWindowTitle(QStringLiteral("RLink 账户"));
    setModal(false);
    setWindowFlags(Qt::Widget | Qt::FramelessWindowHint);
    setAttribute(Qt::WA_TranslucentBackground, true);
    setWindowIcon(CreateRemoteCIcon());
    setStyleSheet(QStringLiteral(R"(
QDialog#loginWindow {
    background:transparent;
    color:#172033;
}
QFrame#loginCard {
    background:#FFFEFB;
    border:1px solid #DDE0E4;
    border-radius:28px;
}
QLabel#brandMark {
    background:transparent;
    border:none;
}
QLabel#brandName {
    color:#172033;
    font-size:21px;
    font-weight:750;
}
QLabel#secureBadge {
    background:#edf8f2;
    border:1px solid #ccebd9;
    border-radius:13px;
    color:#14804a;
    font-size:13px;
    font-weight:650;
    padding:7px 13px;
}
QLabel#loginTitle {
    color:#172033;
    font-size:29px;
    font-weight:750;
}
QLabel#securityNote {
    color:#536176;
    font-size:13px;
}
QFrame#loginStatePanel {
    background:#F5F2EC;
    border:1px solid #DDE0E4;
    border-radius:18px;
}
QFrame#loginStatePanel[tone="error"] {
    background:#fff7f7;
    border-color:#ffdadd;
}
QLabel#loginStateIcon {
    background:#e8ebff;
    border:7px solid #f3f5ff;
    border-radius:38px;
    color:#4357e8;
    font-size:27px;
    font-weight:800;
}
QLabel#loginStateIcon[tone="authenticated"] {
    background:#5b6df8;
    color:white;
}
QLabel#loginStateIcon[tone="error"] {
    background:#fff0f1;
    color:#c92a38;
}
QLabel#loginStatus {
    color:#4F5D72;
    font-size:16px;
}
QLabel#loginStatus[tone="error"] {
    color:#b4232f;
}
QPushButton {
    min-height:48px;
    border-radius:12px;
    padding:0 25px;
    font-size:14px;
}
QPushButton#loginButton {
    background:#5264f5;
    color:white;
    border:1px solid #5264f5;
    font-weight:650;
}
QPushButton#loginButton:hover { background:#4054e9; }
QPushButton#loginButton:disabled {
    background:#DCD9D3;
    color:#5F6B7D;
    border-color:#D2CCC1;
}
QPushButton#secondaryButton {
    background:#FFFEFB;
    color:#344054;
    border:1px solid #d9dfe9;
}
QPushButton#secondaryButton:hover { background:#F0ECE5; }
QPushButton#secondaryButton:disabled { color:#667085; }
QPushButton#signOutButton {
    background:#fff5f5;
    color:#b4232f;
    border:1px solid #f2c9cd;
}
QPushButton#signOutButton:hover { background:#ffe9eb; }
QPushButton#deleteAccountButton {
    min-height:42px; padding:0 18px; border-radius:10px;
    border:1px solid #ef4444; color:#ffffff; background:#dc2626;
}
QPushButton#deleteAccountButton:hover { background:#b91c1c; }
QPushButton#deleteAccountButton:disabled {
    color:#fca5a5; background:#7f1d1d; border-color:#991b1b;
}
QProgressBar {
    min-height:4px;
    max-height:4px;
    border:0;
    border-radius:2px;
    background:#E7E1D8;
}
QProgressBar::chunk { background:#5064f6; }
)"));
    setProperty("remoteCLightStyleSheet", styleSheet());
    if (ui::RemoteCTheme::IsDark(ui::RemoteCTheme::LoadPreference())) {
        setStyleSheet(styleSheet() + QStringLiteral(R"(
QFrame#loginCard { background:#141E2D; border-color:#2B3A50; }
QLabel#brandName, QLabel#loginTitle { color:#F1F5FB; }
QLabel#secureBadge {
    background:#162336; border-color:#354861; color:#4FF0B5;
}
QFrame#loginStatePanel { background:#101927; border-color:#2B3A50; }
QFrame#loginStatePanel[tone="error"] {
    background:#281920; border-color:#6C3541;
}
QLabel#loginStateIcon {
    background:#1D2A4D; border-color:#18243A; color:#8EA5FF;
}
QLabel#loginStateIcon[tone="authenticated"] {
    background:#6F86FF; border-color:#23304B; color:#F7F9FF;
}
QLabel#loginStateIcon[tone="error"] {
    background:#382027; border-color:#2A1B22; color:#FF98A5;
}
QLabel#loginStatus, QLabel#securityNote { color:#A8B5C7; }
QLabel#loginStatus[tone="error"] { color:#FF98A5; }
QPushButton#loginButton {
    background:#4962D7; border-color:#4962D7; color:#F7F9FF;
}
QPushButton#loginButton:hover { background:#526BDE; border-color:#526BDE; }
QPushButton#loginButton:disabled {
    background:#202B3B; border-color:#2A374A; color:#68778C;
}
QPushButton#secondaryButton {
    background:#1A2738; border-color:#354861; color:#DCE7F5;
}
QPushButton#secondaryButton:hover {
    background:#223249; border-color:#49617E;
}
QPushButton#signOutButton {
    background:#382027; border-color:#6C3541; color:#FF98A5;
}
QPushButton#signOutButton:hover { background:#472731; }
QPushButton#deleteAccountButton {
    background:#B83A49; border-color:#D65261; color:#FFFFFF;
}
QPushButton#deleteAccountButton:hover { background:#CA4656; }
QProgressBar { background:#202B3B; }
QProgressBar::chunk { background:#6F86FF; }
)"));
    }

    cardLayer_ = new QWidget(this);
    card_ = new QFrame(cardLayer_);
    auto* card = card_;
    card->setObjectName(QStringLiteral("loginCard"));
    auto* cardLayout = new QVBoxLayout(card);
    cardLayout->setContentsMargins(38, 32, 38, 30);
    cardLayout->setSpacing(15);

    dragHandle_ = new QWidget(card);
    dragHandle_->setCursor(Qt::OpenHandCursor);
    dragHandle_->installEventFilter(this);
    auto* brandMark = new QLabel(dragHandle_);
    brandMark->setObjectName(QStringLiteral("brandMark"));
    brandMark->setAttribute(Qt::WA_TransparentForMouseEvents, true);
    brandMark->setAlignment(Qt::AlignCenter);
    brandMark->setFixedSize(50, 50);
    brandMark->setPixmap(CreateRemoteCIcon().pixmap(46, 46));
    auto* brandName = new QLabel(QStringLiteral("RLink"), dragHandle_);
    brandName->setObjectName(QStringLiteral("brandName"));
    brandName->setAttribute(Qt::WA_TransparentForMouseEvents, true);
    auto* secureBadge = new QLabel(
        QStringLiteral("安全账户"), dragHandle_);
    secureBadge->setObjectName(QStringLiteral("secureBadge"));
    secureBadge->setAttribute(Qt::WA_TransparentForMouseEvents, true);
    auto* brandRow = new QHBoxLayout(dragHandle_);
    brandRow->setContentsMargins(0, 0, 0, 0);
    brandRow->setSpacing(11);
    brandRow->addWidget(brandMark);
    brandRow->addWidget(brandName);
    brandRow->addStretch(1);
    brandRow->addWidget(secureBadge);
    titleLabel_ = new QLabel(card);
    titleLabel_->setObjectName(QStringLiteral("loginTitle"));
    titleLabel_->setAlignment(Qt::AlignCenter);
    statePanel_ = new QFrame(card);
    statePanel_->setObjectName(QStringLiteral("loginStatePanel"));
    statePanel_->setFixedHeight(155);
    auto* stateLayout = new QVBoxLayout(statePanel_);
    stateLayout->setContentsMargins(24, 17, 24, 17);
    stateLayout->setSpacing(10);
    stateLayout->setAlignment(Qt::AlignCenter);
    stateIcon_ = new QLabel(QStringLiteral("i"), statePanel_);
    stateIcon_->setObjectName(QStringLiteral("loginStateIcon"));
    stateIcon_->setFixedSize(64, 64);
    stateIcon_->setAlignment(Qt::AlignCenter);
    statusLabel_ = new QLabel(statePanel_);
    statusLabel_->setObjectName(QStringLiteral("loginStatus"));
    statusLabel_->setWordWrap(true);
    statusLabel_->setAlignment(Qt::AlignCenter);
    statusLabel_->setTextFormat(Qt::AutoText);
    statusLabel_->setMinimumHeight(34);
    stateLayout->addWidget(stateIcon_, 0, Qt::AlignHCenter);
    stateLayout->addWidget(statusLabel_);
    progressBar_ = new QProgressBar(card);
    progressBar_->setRange(0, 0);
    progressBar_->setTextVisible(false);
    securityLabel_ = new QLabel(
        QStringLiteral(
            "登录将在系统浏览器中完成。RLink 不会读取或保存你的密码。"),
        card);
    securityLabel_->setObjectName(QStringLiteral("securityNote"));
    securityLabel_->setWordWrap(true);

    loginButton_ = new QPushButton(card);
    loginButton_->setObjectName(QStringLiteral("loginButton"));
    cancelButton_ = new QPushButton(QStringLiteral("取消当前操作"), card);
    cancelButton_->setObjectName(QStringLiteral("secondaryButton"));
    signOutButton_ = new QPushButton(QStringLiteral("退出登录"), card);
    signOutButton_->setObjectName(QStringLiteral("signOutButton"));
    deleteAccountButton_ =
        new QPushButton(QStringLiteral("注销账号"), card);
    deleteAccountButton_->setObjectName(
        QStringLiteral("deleteAccountButton"));
    exitButton_ = new QPushButton(QStringLiteral("关闭"), card);
    exitButton_->setObjectName(QStringLiteral("secondaryButton"));

    auto* buttonLayout = new QHBoxLayout();
    buttonLayout->setSpacing(10);
    buttonLayout->addWidget(exitButton_);
    buttonLayout->addWidget(signOutButton_);
    buttonLayout->addWidget(deleteAccountButton_);
    buttonLayout->addWidget(cancelButton_);
    buttonLayout->addStretch(1);
    buttonLayout->addWidget(loginButton_);

    cardLayout->addWidget(dragHandle_);
    cardLayout->addSpacing(3);
    cardLayout->addWidget(titleLabel_, 0, Qt::AlignHCenter);
    cardLayout->addWidget(statePanel_);
    cardLayout->addWidget(progressBar_);
    cardLayout->addWidget(securityLabel_);
    cardLayout->addSpacing(3);
    cardLayout->addLayout(buttonLayout);
    connect(loginButton_, &QPushButton::clicked, this, [this] {
        if (loginRequested_) {
            loginRequested_();
        }
    });
    connect(cancelButton_, &QPushButton::clicked, this, [this] {
        if (cancelRequested_) {
            cancelRequested_();
        }
    });
    connect(exitButton_, &QPushButton::clicked, this, [this] {
        if (exitRequested_) {
            exitRequested_();
        } else {
            close();
        }
    });
    connect(signOutButton_, &QPushButton::clicked, this, [this] {
        if (signOutRequested_) {
            signOutRequested_();
        }
    });
    connect(deleteAccountButton_, &QPushButton::clicked, this, [this] {
        if (deleteAccountRequested_) {
            deleteAccountRequested_();
        }
    });
}

void LoginWindow::RefreshThemeStyle()
{
    QString style = property("remoteCLightStyleSheet").toString();
    const bool dark = ui::RemoteCTheme::IsDark(
        ui::RemoteCTheme::LoadPreference());
    if (dark) {
        style += QStringLiteral(R"(
QFrame#loginCard { background:#141E2D; border-color:#2B3A50; }
QLabel#brandName, QLabel#loginTitle { color:#F1F5FB; }
QLabel#secureBadge {
    background:#162336; border-color:#354861; color:#4FF0B5;
}
QFrame#loginStatePanel { background:#101927; border-color:#2B3A50; }
QFrame#loginStatePanel[tone="error"] {
    background:#281920; border-color:#6C3541;
}
QLabel#loginStateIcon {
    background:#1D2A4D; border-color:#18243A; color:#8EA5FF;
}
QLabel#loginStateIcon[tone="authenticated"] {
    background:#6F86FF; border-color:#23304B; color:#F7F9FF;
}
QLabel#loginStateIcon[tone="error"] {
    background:#382027; border-color:#2A1B22; color:#FF98A5;
}
QLabel#loginStatus, QLabel#securityNote { color:#A8B5C7; }
QLabel#loginStatus[tone="error"] { color:#FF98A5; }
QPushButton#loginButton {
    background:#4962D7; border-color:#4962D7; color:#F7F9FF;
}
QPushButton#loginButton:hover { background:#526BDE; border-color:#526BDE; }
QPushButton#loginButton:disabled {
    background:#202B3B; border-color:#2A374A; color:#68778C;
}
QPushButton#secondaryButton {
    background:#1A2738; border-color:#354861; color:#DCE7F5;
}
QPushButton#secondaryButton:hover {
    background:#223249; border-color:#49617E;
}
QPushButton#signOutButton {
    background:#382027; border-color:#6C3541; color:#FF98A5;
}
QPushButton#signOutButton:hover { background:#472731; }
QPushButton#deleteAccountButton {
    background:#B83A49; border-color:#D65261; color:#FFFFFF;
}
QPushButton#deleteAccountButton:hover { background:#CA4656; }
QProgressBar { background:#202B3B; }
QProgressBar::chunk { background:#6F86FF; }
)");
    }
    setProperty("themeRoot", dark ? QStringLiteral("dark")
                                   : QStringLiteral("light"));
    setStyleSheet(style);
    if (statePanel_ &&
        statePanel_->property("tone").toString() ==
            QStringLiteral("authenticated")) {
        UpdateAuthenticatedAccountText();
    }
}

void LoginWindow::UpdateAuthenticatedAccountText()
{
    if (!statusLabel_) return;
    const bool dark = ui::RemoteCTheme::IsDark(
        ui::RemoteCTheme::LoadPreference());
    const QString primary = dark ? QStringLiteral("#F1F5FB")
                                 : QStringLiteral("#172033");
    const QString secondary = dark ? QStringLiteral("#AEBBD0")
                                   : QStringLiteral("#536176");
    statusLabel_->setText(
        authenticatedAccountDetail_.isEmpty()
            ? QStringLiteral(
                  "<div style='font-size:18px;font-weight:700;color:%1;'>%2</div>")
                  .arg(primary, authenticatedAccountLabel_)
            : QStringLiteral(
                  "<div style='font-size:18px;font-weight:700;color:%1;'>%2</div>"
                  "<div style='margin-top:7px;color:%3;'>%4</div>")
                  .arg(primary, authenticatedAccountLabel_, secondary,
                       authenticatedAccountDetail_));
}

}  // namespace remote::controller
