// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

bool ControllerMainWindow::eventFilter(QObject* watched, QEvent* event)
{
    if (profileUpdateButton_ &&
        (watched == profileUpdateButton_ ||
         profileUpdateButton_->isAncestorOf(
             qobject_cast<QWidget*>(watched)))) {
        return FramelessMainWindow::eventFilter(watched, event);
    }
    if (event) {
        auto* accountAction = qobject_cast<QPushButton*>(watched);
        if (accountAction &&
            accountAction->objectName() == QStringLiteral("accountMenuAction") &&
            (event->type() == QEvent::Enter ||
             event->type() == QEvent::HoverEnter ||
             event->type() == QEvent::Leave ||
             event->type() == QEvent::HoverLeave)) {
            const bool hovered = event->type() == QEvent::Enter ||
                event->type() == QEvent::HoverEnter;
            if (accountAction->property("accountHover").toBool() != hovered) {
                accountAction->setProperty("accountHover", hovered);
                accountAction->style()->unpolish(accountAction);
                accountAction->style()->polish(accountAction);
                accountAction->update();
            }
        }
    }
    if (event && event->type() == QEvent::MouseButtonRelease &&
        profileCard_) {
        const auto* mouseEvent = static_cast<QMouseEvent*>(event);
        const QPoint globalPosition =
            mouseEvent->globalPosition().toPoint();
        const QRect profileGeometry(
            profileCard_->mapToGlobal(QPoint(0, 0)),
            profileCard_->size());
        if (mouseEvent->button() == Qt::LeftButton &&
            profileGeometry.contains(globalPosition)) {
            if (authenticationAvailable_) {
                ToggleAccountMenu();
            } else if (accountInteractionCallback_) {
                accountInteractionCallback_();
            }
            return true;
        }
    }
    if (event && event->type() == QEvent::MouseButtonPress &&
        accountMenu_ && accountMenu_->isVisible()) {
        const auto* mouseEvent = static_cast<QMouseEvent*>(event);
        const QPoint globalPosition =
            mouseEvent->globalPosition().toPoint();
        const QRect menuGeometry(
            accountMenu_->mapToGlobal(QPoint(0, 0)), accountMenu_->size());
        const QRect profileGeometry(
            profileCard_->mapToGlobal(QPoint(0, 0)), profileCard_->size());
        if (!menuGeometry.contains(globalPosition) &&
            !profileGeometry.contains(globalPosition)) {
            HideAccountMenu();
        }
    }
    if (event && event->type() == QEvent::MouseButtonPress &&
        fileTransferWindow_ && fileTransferWindow_->isVisible() &&
        !fileTransferWindow_->IsHiding()) {
        auto* clickedWidget = qobject_cast<QWidget*>(watched);
        const auto* mouseEvent = static_cast<QMouseEvent*>(event);
        const QPoint globalPosition =
            mouseEvent->globalPosition().toPoint();
        const QRect drawerGeometry(
            fileTransferWindow_->mapToGlobal(QPoint(0, 0)),
            fileTransferWindow_->size());
        const bool insideDrawer =
            drawerGeometry.contains(globalPosition);
        const bool onDrawerToggle = fileTransferNavButton_ &&
            QRect(fileTransferNavButton_->mapToGlobal(QPoint(0, 0)),
                  fileTransferNavButton_->size())
                .contains(globalPosition);
        const bool insideMainWindow =
            clickedWidget &&
            (clickedWidget == this || isAncestorOf(clickedWidget));
        const bool insidePopup =
            clickedWidget && clickedWidget->window() &&
            clickedWidget->window()->windowFlags().testFlag(Qt::Popup);
        if (insideMainWindow && !insideDrawer && !onDrawerToggle &&
            !insidePopup) {
            fileTransferWindow_->HideWithAnimation();
        }
    }
    return FramelessMainWindow::eventFilter(watched, event);
}

bool ControllerMainWindow::nativeEvent(
    const QByteArray& eventType,
    void* message,
    qintptr* result)
{
#ifdef Q_OS_WIN
    const auto* nativeMessage = static_cast<MSG*>(message);
    if (nativeMessage &&
        nativeMessage->message == WM_DEVICECHANGE &&
        (nativeMessage->wParam == DBT_DEVICEARRIVAL ||
         nativeMessage->wParam == DBT_DEVICEREMOVECOMPLETE ||
         nativeMessage->wParam == DBT_DEVNODES_CHANGED ||
         nativeMessage->wParam == DBT_CONFIGCHANGED)) {
        RequestMediaDeviceRefresh(false);
    }
#endif
    return FramelessMainWindow::nativeEvent(
        eventType, message, result);
}

void ControllerMainWindow::closeEvent(QCloseEvent* event)
{
    if (quitting_ || !trayIcon_ || !trayIcon_->isVisible()) {
        FramelessMainWindow::closeEvent(event);
        return;
    }
    const QString closeBehavior = QSettings().value(
        QString::fromLatin1(kCloseButtonBehaviorSetting),
        QStringLiteral("tray")).toString();
    if (closeBehavior == QStringLiteral("exit")) {
        event->ignore();
        QTimer::singleShot(0, this,
                           &ControllerMainWindow::QuitFromSystemTray);
        return;
    }
    if (fileTransferWindow_) {
        fileTransferWindow_->HideImmediately();
    }
    if (roomCameraWindow_) {
        roomCameraWindow_->hide();
    }
    if (cameraWindow_) {
        cameraWindow_->hide();
    }
    if (remoteSessionWindow_) {
        remoteSessionWindow_->hide();
    }
    hide();
    event->ignore();
}

void ControllerMainWindow::changeEvent(QEvent* event)
{
    FramelessMainWindow::changeEvent(event);
    if (!event || event->type() != QEvent::WindowStateChange ||
        !fileTransferWindow_) {
        return;
    }
    if (isMinimized()) {
        fileTransferWindow_->HideImmediately();
    }
}

void ControllerMainWindow::resizeEvent(QResizeEvent* event)
{
    FramelessMainWindow::resizeEvent(event);
    UpdateAccountMenuGeometry();
    if (fileTransferWindow_) {
        fileTransferWindow_->UpdateDrawerGeometry();
    }
}

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
    if (!QSystemTrayIcon::isSystemTrayAvailable()) {
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
