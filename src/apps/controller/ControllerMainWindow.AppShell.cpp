// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#include "ControllerMainWindow.h"
#include "ControllerMainWindowSupport.h"

#include <QAction>
#include <QApplication>
#include <QCursor>
#include <QEasingCurve>
#include <QHBoxLayout>
#include <QLabel>
#include <QMenu>
#include <QPushButton>
#include <QScreen>
#include <QStyle>
#include <QSystemTrayIcon>
#include <QTimer>
#include <QToolButton>
#include <QWidgetAction>
#include <QWindow>

#include <algorithm>

#include "CameraWindow.h"
#include "ControlledSessionIndicator.h"
#include "FileTransferWindow.h"
#include "RemoteCDialog.h"
#include "RemoteCToast.h"
#include "RemoteSessionWindow.h"
#include "RoomCameraWindow.h"
#include "pages/SettingsPage.h"

#ifdef Q_OS_WIN
#include <qt_windows.h>
#endif

namespace remote::controller {
using namespace detail;

void ControllerMainWindow::ToggleAccountMenu()
{
    if (!accountMenu_ || !authenticationAvailable_) return;
    if (accountMenu_->isVisible()) HideAccountMenu();
    else ShowAccountMenu();
}

void ControllerMainWindow::UpdateAccountMenuGeometry()
{
    if (!accountMenu_ || !profileCard_ || !accountMenu_->parentWidget()) {
        return;
    }
    QWidget* host = accountMenu_->parentWidget();
    const QPoint profileTopLeft = host->mapFromGlobal(
        profileCard_->mapToGlobal(QPoint(0, 0)));
    const int gap = 9;
    const int margin = 8;
    int x = profileTopLeft.x();
    int y = profileTopLeft.y() - accountMenu_->height() - gap;
    x = std::clamp(x, margin,
                   std::max(margin, host->width() - accountMenu_->width() - margin));
    y = std::clamp(y, margin,
                   std::max(margin, host->height() - accountMenu_->height() - margin));
    accountMenu_->move(x, y);
}

void ControllerMainWindow::UpdateAccountMenuHoverFromCursor()
{
    if (!accountMenu_ || !accountMenu_->isVisible()) return;

    const QPoint cursorPosition = QCursor::pos();
    const auto actions = accountMenu_->findChildren<QPushButton*>(
        QStringLiteral("accountMenuAction"));
    for (QPushButton* action : actions) {
        const bool hovered = action->isVisible() &&
            action->rect().contains(action->mapFromGlobal(cursorPosition));
        if (action->property("accountHover").toBool() == hovered) continue;

        action->setProperty("accountHover", hovered);
        action->style()->unpolish(action);
        action->style()->polish(action);
        action->update();
    }
}

void ControllerMainWindow::ShowAccountMenu()
{
    if (!accountMenu_ || !authenticationAvailable_) return;
#ifdef Q_OS_WIN
    // The restore-status window is a separate native top-level window. If
    // Windows has not completed the foreground hand-off yet, the profile-card
    // release can still open this embedded menu while the controller HWND is
    // inactive. Promote it before showing the menu so hover tracking starts on
    // the first pointer move rather than after an extra click.
    const HWND mainHwnd = reinterpret_cast<HWND>(winId());
    if (mainHwnd && GetForegroundWindow() != mainHwnd) {
        (void)BringWindowToTop(mainHwnd);
        (void)SetForegroundWindow(mainHwnd);
        (void)SetActiveWindow(mainHwnd);
    }
#endif
    activateWindow();
    if (windowHandle()) windowHandle()->requestActivate();
    StopAccountMenuMotion();
    UpdateAccountMenuGeometry();
    const QRect target = accountMenu_->geometry();
    const QPoint targetPosition = target.topLeft();
    const QPoint startPosition = targetPosition + QPoint(0, 20);
    accountMenu_->setGeometry(target);
    accountMenu_->move(startPosition);
    // Do not animate this embedded popup through QGraphicsOpacityEffect.
    // Its off-screen cache can keep the first painted frame alive and suppress
    // visible QPushButton hover updates until another top-level window forces
    // a full repaint. A short geometry-only reveal stays smooth and lets every
    // action paint directly into the main window backing store from frame one.
    accountMenu_->setGraphicsEffect(nullptr);
    accountMenuOpacity_ = nullptr;
    const auto actions = accountMenu_->findChildren<QPushButton*>(
        QStringLiteral("accountMenuAction"));
    for (QPushButton* action : actions) {
        action->setProperty("accountHover", false);
        action->style()->unpolish(action);
        action->style()->polish(action);
    }
    accountMenu_->ensurePolished();
    accountMenu_->show();
    accountMenu_->raise();
    accountMenu_->update();
    if (!accountMenuHoverTimer_) {
        accountMenuHoverTimer_ = new QTimer(this);
        accountMenuHoverTimer_->setInterval(16);
        connect(accountMenuHoverTimer_, &QTimer::timeout, this,
                &ControllerMainWindow::UpdateAccountMenuHoverFromCursor);
    }
    UpdateAccountMenuHoverFromCursor();
    accountMenuHoverTimer_->start();
    const int level = CurrentUiAnimationLevel();
    if (level <= 0) {
        accountMenu_->move(targetPosition);
        return;
    }
    StartAccountMenuMotion(
        targetPosition, level == 1 ? 120 : 175, false);
}

void ControllerMainWindow::HideAccountMenu(bool animated)
{
    if (!accountMenu_ || !accountMenu_->isVisible()) return;
    if (accountMenuHoverTimer_) accountMenuHoverTimer_->stop();
    const auto actions = accountMenu_->findChildren<QPushButton*>(
        QStringLiteral("accountMenuAction"));
    for (QPushButton* action : actions) {
        if (!action->property("accountHover").toBool()) continue;
        action->setProperty("accountHover", false);
        action->style()->unpolish(action);
        action->style()->polish(action);
        action->update();
    }
    StopAccountMenuMotion();
    const QPoint startPosition = accountMenu_->pos();
    const int level = CurrentUiAnimationLevel();
    if (!animated || level <= 0) {
        accountMenu_->hide();
        return;
    }
    StartAccountMenuMotion(
        startPosition + QPoint(0, 18), level == 1 ? 95 : 140, true);
}

void ControllerMainWindow::StartAccountMenuMotion(
    const QPoint& targetPosition, int durationMs, bool hideWhenFinished)
{
    accountMenuMotionStart_ = accountMenu_->pos();
    accountMenuMotionTarget_ = targetPosition;
    accountMenuMotionDurationMs_ = std::max(1, durationMs);
    accountMenuMotionHiding_ = hideWhenFinished;
    accountMenuMotionClock_.restart();

    if (!accountMenuMotionTimer_) {
        accountMenuMotionTimer_ = new QTimer(this);
        accountMenuMotionTimer_->setTimerType(Qt::PreciseTimer);
        accountMenuMotionTimer_->setInterval(8);
        connect(accountMenuMotionTimer_, &QTimer::timeout, this, [this] {
            if (accountMenuMotionDurationMs_ <= 0) {
                accountMenuMotionTimer_->stop();
                return;
            }
            const qreal progress = std::min<qreal>(
                1.0,
                static_cast<qreal>(accountMenuMotionClock_.elapsed()) /
                    accountMenuMotionDurationMs_);
            static const QEasingCurve reveal(QEasingCurve::OutQuart);
            static const QEasingCurve dismiss(QEasingCurve::InCubic);
            const qreal eased = (accountMenuMotionHiding_
                ? dismiss : reveal).valueForProgress(progress);
            const QPoint delta =
                accountMenuMotionTarget_ - accountMenuMotionStart_;
            accountMenu_->move(accountMenuMotionStart_ + QPoint(
                qRound(delta.x() * eased), qRound(delta.y() * eased)));
            if (progress < 1.0) return;

            accountMenuMotionTimer_->stop();
            accountMenu_->move(accountMenuMotionTarget_);
            accountMenuMotionDurationMs_ = 0;
            if (accountMenuMotionHiding_) {
                accountMenu_->hide();
            } else {
                accountMenu_->update();
            }
            accountMenuMotionHiding_ = false;
        });
    }
    accountMenuMotionTimer_->start();
}

void ControllerMainWindow::StopAccountMenuMotion()
{
    if (accountMenuMotionTimer_) accountMenuMotionTimer_->stop();
    accountMenuMotionDurationMs_ = 0;
    accountMenuMotionHiding_ = false;
}

void ControllerMainWindow::BuildSystemTray()
{
    // At Windows logon Explorer may not have created the tray yet. Keep a
    // visible tray object for background startup; Qt adds it when the tray
    // becomes available, without needing to expose the main window.
    if (!QSystemTrayIcon::isSystemTrayAvailable() &&
        !qApp->property("remoteCBackgroundStartup").toBool()) {
        return;
    }
    trayIcon_ = new QSystemTrayIcon(CreateRemoteCIcon(), this);
    trayIcon_->setToolTip(QStringLiteral("RLink - 远程控制"));

    auto* menu = new QMenu(this);
    menu->setObjectName(QStringLiteral("systemTrayMenu"));
    menu->setAttribute(Qt::WA_TranslucentBackground);
    menu->setMinimumWidth(224);
    menu->setStyleSheet(ScaleUiStyleSheet(QStringLiteral(R"(
QMenu#systemTrayMenu {
    background: #151e2e;
    color: #edf2fa;
    border: 1px solid #2b3950;
    border-radius: 12px;
    padding: 7px;
    font-size: 13px;
}
QMenu#systemTrayMenu::item {
    min-height: 36px;
    padding: 0 14px;
    margin: 2px 0;
    border-radius: 8px;
}
QMenu#systemTrayMenu::item:selected {
    background: #273652;
    color: #ffffff;
}
QMenu#systemTrayMenu::item:disabled {
    background: transparent;
    color: #718096;
}
QMenu#systemTrayMenu::separator {
    height: 1px;
    margin: 6px 10px;
    background: #2a374b;
}
)")));

    auto* headerAction = new QWidgetAction(menu);
    auto* header = new QWidget(menu);
    auto* headerLayout = new QHBoxLayout(header);
    headerLayout->setContentsMargins(12, 8, 12, 9);
    headerLayout->setSpacing(10);
    auto* logo = new QLabel(header);
    logo->setAlignment(Qt::AlignCenter);
    logo->setFixedSize(32, 32);
    logo->setPixmap(CreateRemoteCIcon().pixmap(30, 30));
    trayIdentityLabel_ = new QLabel(
        QStringLiteral("<b>RLink</b><br><span style='color:#8290a6;'>安全远程工作台</span>"),
        header);
    trayIdentityLabel_->setStyleSheet(
        QStringLiteral("color:#f4f7fb;font-size:12px;"));
    headerLayout->addWidget(logo);
    headerLayout->addWidget(trayIdentityLabel_, 1);
    headerAction->setDefaultWidget(header);
    menu->addAction(headerAction);
    menu->addSeparator();

    auto* openAction = menu->addAction(QStringLiteral("打开主窗口"));
    traySignOutAction_ = menu->addAction(QStringLiteral("退出登录"));
    traySignOutAction_->setVisible(false);
    menu->addSeparator();
    auto* quitAction = menu->addAction(QStringLiteral("退出 RLink"));
    trayIcon_->setContextMenu(menu);

    connect(openAction, &QAction::triggered,
            this, &ControllerMainWindow::ShowFromSystemTray);
    connect(traySignOutAction_, &QAction::triggered, this, [this] {
        if (signOutCallback_) {
            signOutCallback_();
        }
    });
    connect(quitAction, &QAction::triggered,
            this, &ControllerMainWindow::QuitFromSystemTray);
    connect(trayIcon_, &QSystemTrayIcon::activated, this,
            [this](QSystemTrayIcon::ActivationReason reason) {
                if (reason == QSystemTrayIcon::Trigger ||
                    reason == QSystemTrayIcon::DoubleClick) {
                    ShowFromSystemTray();
                }
            });
    trayIcon_->show();
}

void ControllerMainWindow::ShowFromSystemTray()
{
    if (isMinimized()) {
        showNormal();
    } else {
        show();
    }
    raise();
    activateWindow();
    if (windowHandle()) windowHandle()->requestActivate();
#ifdef Q_OS_WIN
    const HWND mainHwnd = reinterpret_cast<HWND>(winId());
    if (mainHwnd) {
        if (IsIconic(mainHwnd)) ShowWindow(mainHwnd, SW_RESTORE);
        // Temporarily enter the topmost band, activate, then immediately
        // return to the normal band. This puts RLink above the current desktop
        // windows without turning the main window into permanent always-on-top.
        (void)SetWindowPos(mainHwnd, HWND_TOPMOST, 0, 0, 0, 0,
                           SWP_NOMOVE | SWP_NOSIZE | SWP_SHOWWINDOW);
        (void)BringWindowToTop(mainHwnd);
        (void)SetForegroundWindow(mainHwnd);
        (void)SetActiveWindow(mainHwnd);
        (void)SetWindowPos(mainHwnd, HWND_NOTOPMOST, 0, 0, 0, 0,
                           SWP_NOMOVE | SWP_NOSIZE | SWP_SHOWWINDOW);
    }
#endif
}

void ControllerMainWindow::QuitFromSystemTray()
{
    if (quitting_) {
        return;
    }
    quitting_ = true;
    PrepareForApplicationExit();
    if (trayIcon_) {
        trayIcon_->hide();
    }
    // QApplication::quit() first asks every top-level window to close. The
    // auxiliary windows normally ignore close events so that their X button
    // only hides them; destroy them explicitly on the real tray-exit path or
    // they can veto application shutdown and leave RLinkAPP.exe alive.
    DestroyAuxiliaryWindowsForExit();
    QApplication::setQuitOnLastWindowClosed(true);
    close();
    QTimer::singleShot(0, qApp, [] { QApplication::quit(); });
}

void ControllerMainWindow::DestroyAuxiliaryWindowsForExit()
{
    if (controlledSessionWindow_) delete controlledSessionWindow_.data();
    if (fileTransferWindow_) {
        fileTransferWindow_->DetachController();
        delete fileTransferWindow_.data();
    }
    if (roomCameraWindow_) {
        delete roomCameraWindow_.data();
    }
    if (cameraWindow_) {
        delete cameraWindow_.data();
    }
    if (remoteSessionWindow_) {
        delete remoteSessionWindow_.data();
    }
}
void ControllerMainWindow::CheckForSoftwareUpdates(bool manualRequest)
{
    if (!softwareUpdateController_) {
        if (manualRequest) {
            RemoteCDialog::Alert(
                this, QStringLiteral("暂时无法检查更新"),
                QStringLiteral("更新组件尚未准备好，请稍后重试。"));
        }
        return;
    }
    softwareUpdateController_->CheckForUpdates(manualRequest);
}

void ControllerMainWindow::HandleSoftwareUpdateState(
    const update::SoftwareUpdateController::Snapshot& snapshot)
{
    using UpdateState = update::SoftwareUpdateController::State;
    const bool checking = snapshot.state == UpdateState::kChecking;
    const bool available =
        snapshot.state == UpdateState::kUpdateAvailable;
    const bool launching =
        snapshot.state == UpdateState::kLaunchingUpdater;
    const auto* settingsControls =
        settingsPage_ ? &settingsPage_->Controls() : nullptr;

    if (profileUpdateButton_) {
        profileUpdateButton_->setVisible(available);
        profileUpdateButton_->setToolTip(
            available
                ? QStringLiteral("发现新版本 v%1")
                      .arg(snapshot.latestVersion)
                : QStringLiteral("有新版本可用"));
    }
    if (softwareUpdateAction_) {
        softwareUpdateAction_->setEnabled(!checking && !launching);
        softwareUpdateAction_->setText(
            available
                ? QStringLiteral("软件更新 · v%1")
                      .arg(snapshot.latestVersion)
                : checking
                    ? QStringLiteral("正在检查更新…")
                    : QStringLiteral("软件更新"));
    }
    if (settingsControls && settingsControls->softwareUpdateCheckButton) {
        settingsControls->softwareUpdateCheckButton->setEnabled(
            !checking && !launching);
        settingsControls->softwareUpdateCheckButton->setText(
            available ? QStringLiteral("立即更新")
                      : checking ? QStringLiteral("检查中…")
                                 : QStringLiteral("检查更新"));
    }
    if (settingsControls && settingsControls->softwareUpdateStatusLabel) {
        switch (snapshot.state) {
        case UpdateState::kIdle:
            settingsControls->softwareUpdateStatusLabel->setText(
                QStringLiteral("当前安装版本由安装程序统一管理"));
            break;
        case UpdateState::kChecking:
            settingsControls->softwareUpdateStatusLabel->setText(
                QStringLiteral("正在从更新镜像检查新版本"));
            break;
        case UpdateState::kUpToDate:
            settingsControls->softwareUpdateStatusLabel->setText(
                QStringLiteral("当前已是最新版本"));
            break;
        case UpdateState::kUpdateAvailable:
            settingsControls->softwareUpdateStatusLabel->setText(
                QStringLiteral("发现新版本 v%1")
                    .arg(snapshot.latestVersion));
            break;
        case UpdateState::kFailed:
            settingsControls->softwareUpdateStatusLabel->setText(
                snapshot.errorMessage);
            break;
        case UpdateState::kLaunchingUpdater:
            settingsControls->softwareUpdateStatusLabel->setText(
                QStringLiteral("正在启动独立更新程序"));
            break;
        }
    }

    if (snapshot.state == UpdateState::kUpToDate &&
        snapshot.manualRequest) {
        RemoteCToast::Show(
            this, QStringLiteral("当前已是最新版本"),
            RemoteCToast::Tone::kSuccess);
    } else if (snapshot.state == UpdateState::kFailed &&
               snapshot.manualRequest) {
        RemoteCDialog::Alert(
            this, QStringLiteral("检查更新失败"),
            snapshot.errorMessage, QStringLiteral("知道了"),
            RemoteCDialog::Tone::kWarning);
    } else if (available && snapshot.mandatory &&
               !softwareUpdatePromptOpen_) {
        QTimer::singleShot(0, this, [this] { OpenSoftwareUpdate(); });
    }
}

void ControllerMainWindow::OpenSoftwareUpdate()
{
    if (!softwareUpdateController_) {
        CheckForSoftwareUpdates(true);
        return;
    }
    const auto snapshot = softwareUpdateController_->CurrentSnapshot();
    using UpdateState = update::SoftwareUpdateController::State;
    if (snapshot.state != UpdateState::kUpdateAvailable) {
        CheckForSoftwareUpdates(true);
        return;
    }

    QString details = snapshot.summary.isEmpty()
        ? QStringLiteral("新版本已准备好。")
        : snapshot.summary;
    if (!snapshot.releaseNotes.isEmpty()) {
        details += QStringLiteral("\n\n本次更新：\n• ") +
            snapshot.releaseNotes.join(QStringLiteral("\n• "));
    }
    QString message = QStringLiteral(
        "当前版本：v%1\n可用版本：v%2\n\n%3\n\n"
        "开始后会结束当前远程会话，RLink 将自动退出、安装更新并重新启动。")
        .arg(snapshot.installedVersion,
             snapshot.latestVersion,
             details);
    softwareUpdatePromptOpen_ = true;
    const bool accepted = RemoteCDialog::Confirm(
        this,
        snapshot.mandatory ? QStringLiteral("必须更新 RLink")
                           : QStringLiteral("安装 RLink 更新"),
        message, QStringLiteral("立即更新"),
        snapshot.mandatory ? QStringLiteral("退出 RLink")
                           : QStringLiteral("稍后"),
        snapshot.mandatory ? RemoteCDialog::Tone::kWarning
                           : RemoteCDialog::Tone::kInformation,
        snapshot.mandatory);
    softwareUpdatePromptOpen_ = false;
    if (!accepted) {
        if (snapshot.mandatory) {
            PrepareForApplicationExit();
            QApplication::quit();
        }
        return;
    }

    QString launchError;
    if (!softwareUpdateController_->LaunchUpdater(&launchError)) {
        RemoteCDialog::Alert(
            this, QStringLiteral("无法开始更新"), launchError,
            QStringLiteral("知道了"), RemoteCDialog::Tone::kDanger);
        if (snapshot.mandatory) {
            QTimer::singleShot(0, this, [this] { OpenSoftwareUpdate(); });
        }
        return;
    }
    QuitForSoftwareUpdate();
}

void ControllerMainWindow::QuitForSoftwareUpdate()
{
    if (quitting_) {
        return;
    }
    quitting_ = true;
    PrepareForApplicationExit();
    if (trayIcon_) {
        trayIcon_->hide();
    }
    DestroyAuxiliaryWindowsForExit();
    for (QWidget* widget : QApplication::topLevelWidgets()) {
        if (widget) {
            widget->hide();
        }
    }
    QApplication::setQuitOnLastWindowClosed(true);
    QTimer::singleShot(0, qApp, [] { QApplication::quit(); });
}

}  // namespace remote::controller
