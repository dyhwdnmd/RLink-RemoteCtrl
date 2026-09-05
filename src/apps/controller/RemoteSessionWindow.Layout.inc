// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

    bool RemoteSessionWindow::eventFilter(QObject* watched, QEvent* event)
    {
        if (watched == contentHost_ && event->type() == QEvent::Resize) {
            LayoutSessionOverlays();
        }
        else if (watched == desktopCanvas_ &&
                 event->type() == QEvent::MouseMove) {
            const auto* mouseEvent = static_cast<QMouseEvent*>(event);
            const int canvasWidth = desktopCanvas_->width();
            const int revealWidth = std::clamp(
                canvasWidth / 3, 260, 620);
            const qreal revealLeft =
                static_cast<qreal>(canvasWidth - revealWidth) / 2.0;
            const qreal pointerX = mouseEvent->position().x();
            if (!toolbarLocked_ && mouseEvent->position().y() <= 7.0 &&
                pointerX >= revealLeft &&
                pointerX <= revealLeft + revealWidth) {
                ShowSessionToolbar();
                if (toolbarHideTimer_) {
                    toolbarHideTimer_->stop();
                }
            }
        }
        else if (watched == sessionToolbar_) {
            if (event->type() == QEvent::Enter) {
                if (!toolbarShown_) {
                    ShowSessionToolbar(true);
                }
                if (toolbarHideTimer_) {
                    toolbarHideTimer_->stop();
                }
            }
            else if (event->type() == QEvent::Leave) {
                ScheduleSessionToolbarHide();
            }
        }
        return FramelessMainWindow::eventFilter(watched, event);
    }

    void RemoteSessionWindow::resizeEvent(QResizeEvent* event)
    {
        FramelessMainWindow::resizeEvent(event);
        LayoutSessionOverlays();
    }

    void RemoteSessionWindow::changeEvent(QEvent* event)
    {
        FramelessMainWindow::changeEvent(event);
        if (!event || event->type() != QEvent::WindowStateChange) {
            return;
        }
        if (isMinimized()) {
            if (sessionHud_) sessionHud_->hide();
            return;
        }

        const auto restoreSessionHud = [this] {
            if (!isVisible() || isMinimized() || !sessionHud_) return;
            LayoutSessionOverlays();
            sessionHud_->show();
            sessionHud_->raise();
            sessionHud_->updateGeometry();
            sessionHud_->update();
        };
        restoreSessionHud();
        QTimer::singleShot(0, this, restoreSessionHud);
        QTimer::singleShot(50, this, restoreSessionHud);
    }

    void RemoteSessionWindow::moveEvent(QMoveEvent* event)
    {
        FramelessMainWindow::moveEvent(event);
        LayoutSessionOverlays();
    }

    void RemoteSessionWindow::LayoutSessionOverlays()
    {
        if (!contentHost_) {
            return;
        }
        const int width = contentHost_->width();
        const int height = contentHost_->height();
        if (desktopCanvas_) {
            desktopCanvas_->setGeometry(0, 0, width, height);
        }
        if (sessionToolbar_) {
            const int toolbarWidth = (std::max)(
                320, (std::min)(width - 96, 1100));
            const int toolbarX = (width - toolbarWidth) / 2;
            const int y = toolbarShown_ ? 0 : -sessionToolbar_->height();
            if (!toolbarAnimation_ ||
                toolbarAnimation_->state() != QAbstractAnimation::Running) {
                sessionToolbar_->setGeometry(
                    toolbarX, y, toolbarWidth, sessionToolbar_->height());
            }
            else {
                sessionToolbar_->resize(toolbarWidth, sessionToolbar_->height());
            }
            QPainterPath toolbarClip;
            if (fullScreenMode_) {
                const QRectF bounds(sessionToolbar_->rect());
                constexpr qreal radius = 18.0;
                toolbarClip.moveTo(bounds.left(), bounds.top());
                toolbarClip.lineTo(bounds.right(), bounds.top());
                toolbarClip.lineTo(bounds.right(),
                                   bounds.bottom() - radius);
                toolbarClip.quadTo(bounds.right(), bounds.bottom(),
                                   bounds.right() - radius,
                                   bounds.bottom());
                toolbarClip.lineTo(bounds.left() + radius,
                                   bounds.bottom());
                toolbarClip.quadTo(bounds.left(), bounds.bottom(),
                                   bounds.left(),
                                   bounds.bottom() - radius);
                toolbarClip.closeSubpath();
            }
            else {
                toolbarClip.addRoundedRect(
                    QRectF(sessionToolbar_->rect()), 18.0, 18.0);
            }
            sessionToolbar_->setMask(QRegion(
                toolbarClip.toFillPolygon().toPolygon()));
            sessionToolbar_->raise();
        }
        if (toolbarRevealZone_) {
            const int revealWidth = std::clamp(width / 3, 260, 620);
            toolbarRevealZone_->setGeometry(
                (width - revealWidth) / 2, 0, revealWidth, 7);
            toolbarRevealZone_->raise();
        }
        if (sessionHud_) {
            // Keep the HUD deliberately compact: FPS plus three semantic
            // icon-only controls. The previous 84x222 layout reintroduced
            // Chinese labels on every layout pass and no longer matched the
            // original remote-session interaction model.
            const bool transferVisible =
                remotePasteStatusHost_ && remotePasteStatusHost_->isVisible();
            sessionHud_->setFixedSize(58, transferVisible ? 166 : 128);
            if (sessionHud_->layout()) {
                sessionHud_->layout()->setContentsMargins(0, 0, 0, 0);
                sessionHud_->layout()->setSpacing(4);
            }
            if (!sessionHud_->property("compactHudStyled").toBool()) {
                sessionHud_->setProperty("compactHudStyled", true);
                sessionHud_->setStyleSheet(QStringLiteral(R"(
QWidget#sessionHud {
    background: rgba(8, 15, 25, 120);
    border: 1px solid rgba(174, 181, 182, 88);
    border-radius: 12px;
}
QLabel#hudFrameRateLabel {
    background: transparent;
    border: none;
    color: #58E3A5;
    font-size: 11px;
    font-weight: 800;
    padding: 0;
}
QToolButton#sessionHudButton {
    background: rgba(19, 30, 46, 142);
    border: 1px solid rgba(190, 188, 178, 96);
    border-radius: 8px;
    color: #DCE7F6;
    padding: 0;
}
QToolButton#sessionHudButton:hover {
    background: rgba(39, 57, 82, 188);
    border-color: rgba(220, 216, 204, 170);
}
QToolButton#sessionHudButton:pressed {
    background: rgba(12, 25, 43, 230);
}
QToolButton#sessionHudButton:disabled {
    background: rgba(15, 24, 38, 116);
    border-color: rgba(150, 149, 143, 72);
}
QToolButton#sessionHudButton[locked="true"] {
    background: rgba(43, 34, 18, 158);
    border-color: rgba(255, 200, 87, 185);
}
)"));
            }
            const auto configureHudButton = [](QToolButton* button,
                                                const QString& resource) {
                if (!button) return;
                button->setText(QString());
                button->setIcon(QIcon(resource));
                button->setIconSize(QSize(18, 18));
                button->setToolButtonStyle(Qt::ToolButtonIconOnly);
                button->setFixedSize(36, 28);
                button->setStyleSheet(QStringLiteral(
                    "QToolButton { image: url(%1); }").arg(resource));
            };
            configureHudButton(
                mediaDeviceButton_,
                QStringLiteral(":/ui/icons/actions/settings.svg"));
            configureHudButton(
                remoteDisplayButton_,
                QStringLiteral(":/ui/icons/actions/display.svg"));
            configureHudButton(
                toolbarLockButton_,
                toolbarLocked_
                    ? QStringLiteral(":/ui/icons/actions/lock.svg")
                    : QStringLiteral(":/ui/icons/actions/unlock.svg"));
            if (remotePasteStatusHost_) {
                remotePasteStatusHost_->setFixedSize(58, 34);
            }
            if (remotePasteStatusButton_) {
                remotePasteStatusButton_->move(15, 6);
            }

            const int hudY = 18;
            sessionHud_->move(contentHost_->mapToGlobal(QPoint(
                (std::max)(12, width - sessionHud_->width() - 14),
                hudY)));
            sessionHud_->raise();
        }
    }

    void RemoteSessionWindow::ShowSessionToolbar(bool animated)
    {
        if (!sessionToolbar_ || !contentHost_ || toolbarLocked_) {
            return;
        }
        toolbarShown_ = true;
        sessionToolbar_->show();
        sessionToolbar_->raise();
        if (sessionHud_) {
            sessionHud_->raise();
        }
        const int toolbarWidth = (std::max)(
            320, (std::min)(contentHost_->width() - 96, 1100));
        const int toolbarX = (contentHost_->width() - toolbarWidth) / 2;
        const QPoint target(toolbarX, 0);
        if (!animated || CurrentUiAnimationLevel() <= 0) {
            if (toolbarAnimation_) {
                toolbarAnimation_->stop();
            }
            sessionToolbar_->move(target);
            return;
        }
        toolbarAnimation_->stop();
        toolbarAnimation_->setEasingCurve(QEasingCurve::OutCubic);
        toolbarAnimation_->setDuration(
            CurrentUiAnimationLevel() == 1 ? 200 : 280);
        const QPoint current = sessionToolbar_->pos();
        const QPoint startPoint(
            target.x(),
            current.y() < 0 ? current.y() : -sessionToolbar_->height());
        sessionToolbar_->move(startPoint);
        toolbarAnimation_->setStartValue(startPoint);
        toolbarAnimation_->setEndValue(target);
        toolbarAnimation_->start();
    }

    void RemoteSessionWindow::HideSessionToolbar(bool animated)
    {
        if (!sessionToolbar_) {
            return;
        }
        toolbarShown_ = false;
        const int toolbarWidth = contentHost_
            ? (std::max)(320,
                (std::min)(contentHost_->width() - 96, 1100))
            : 320;
        const int toolbarX = contentHost_
            ? (contentHost_->width() - toolbarWidth) / 2
            : 0;
        const QPoint target(toolbarX, -sessionToolbar_->height());
        if (!animated || CurrentUiAnimationLevel() <= 0) {
            if (toolbarAnimation_) {
                toolbarAnimation_->stop();
            }
            sessionToolbar_->move(target);
            return;
        }
        toolbarAnimation_->stop();
        toolbarAnimation_->setEasingCurve(QEasingCurve::InCubic);
        toolbarAnimation_->setDuration(
            CurrentUiAnimationLevel() == 1 ? 180 : 250);
        const QPoint start(toolbarX, sessionToolbar_->y());
        sessionToolbar_->move(start);
        toolbarAnimation_->setStartValue(start);
        toolbarAnimation_->setEndValue(target);
        toolbarAnimation_->start();
    }

    void RemoteSessionWindow::ScheduleSessionToolbarHide()
    {
        if (toolbarHideTimer_ && !toolbarLocked_ &&
            !mediaDeviceMenuOpen_) {
            toolbarHideTimer_->start(900);
        }
    }
