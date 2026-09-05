// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

    void RemoteSessionWindow::ToggleToolbarLock()
    {
        toolbarLocked_ = !toolbarLocked_;
        if (toolbarHideTimer_) {
            toolbarHideTimer_->stop();
        }
        if (toolbarRevealZone_) {
            toolbarRevealZone_->setEnabled(!toolbarLocked_);
        }
        if (toolbarLockButton_) {
            // The compact HUD is icon-only. Do not re-introduce the temporary
            // “锁定 / 解锁” labels when the state changes.
            toolbarLockButton_->setText(QString());
            const QString lockResource = toolbarLocked_
                ? QStringLiteral(":/ui/icons/actions/lock.svg")
                : QStringLiteral(":/ui/icons/actions/unlock.svg");
            if (auto* morphButton =
                    dynamic_cast<MorphIconToolButton*>(toolbarLockButton_)) {
                morphButton->setStyleSheet(QString());
                morphButton->SetTarget(toolbarLocked_);
            }
            else {
                toolbarLockButton_->setIcon(QIcon(lockResource));
                toolbarLockButton_->setStyleSheet(QStringLiteral(
                    "QToolButton { image: url(%1); }").arg(lockResource));
            }
            toolbarLockButton_->setIconSize(QSize(18, 18));
            toolbarLockButton_->setToolButtonStyle(Qt::ToolButtonIconOnly);
            toolbarLockButton_->setFixedSize(36, 28);
            toolbarLockButton_->setProperty("locked", toolbarLocked_);
            toolbarLockButton_->style()->unpolish(toolbarLockButton_);
            toolbarLockButton_->style()->polish(toolbarLockButton_);
            toolbarLockButton_->setToolTip(
                toolbarLocked_
                ? QStringLiteral("顶部控制面板已锁定；点击解锁")
                : QStringLiteral("锁定顶部控制面板"));
            if (CurrentUiAnimationLevel() > 0) {
                auto* effect =
                    new QGraphicsOpacityEffect(toolbarLockButton_);
                toolbarLockButton_->setGraphicsEffect(effect);
                auto* animation = new QPropertyAnimation(
                    effect, "opacity", effect);
                animation->setDuration(
                    CurrentUiAnimationLevel() == 1 ? 100 : 155);
                animation->setStartValue(0.3);
                animation->setEndValue(1.0);
                animation->setEasingCurve(QEasingCurve::OutCubic);
                connect(animation, &QPropertyAnimation::finished,
                        toolbarLockButton_,
                        [button = toolbarLockButton_, effect] {
                            if (button && button->graphicsEffect() == effect) {
                                button->setGraphicsEffect(nullptr);
                            }
                        });
                animation->start(QAbstractAnimation::DeleteWhenStopped);
            }
        }
        if (toolbarLocked_) {
            HideSessionToolbar();
        }
        else {
            toolbarShown_ = false;
        }
    }

    void RemoteSessionWindow::ToggleFullScreenMode()
    {
        fullScreenMode_ = !fullScreenMode_;
        if (fullScreenMode_ && toolbarLocked_) {
            ToggleToolbarLock();
        }
        if (sessionTitleBar_) {
            sessionTitleBar_->setVisible(!fullScreenMode_);
        }
        SetNativeRoundedCornersEnabled(!fullScreenMode_);
        clearMask();
        if (sessionToolbar_) {
            sessionToolbar_->setProperty("fullscreen", fullScreenMode_);
            sessionToolbar_->style()->unpolish(sessionToolbar_);
            sessionToolbar_->style()->polish(sessionToolbar_);
        }
        if (fullScreenMode_) {
            showFullScreen();
        }
        else {
            showNormal();
        }
        SetNativeRoundedCornersEnabled(!fullScreenMode_);
        clearMask();
        LayoutSessionOverlays();
        if (fullScreenButton_) {
            fullScreenButton_->SetIcon(
                fullScreenMode_
                    ? QStringLiteral(":/ui/icons/actions/fullscreen-exit.svg")
                    : QStringLiteral(":/ui/icons/actions/fullscreen.svg"));
            fullScreenButton_->SetText(
                QStringLiteral("全屏"));
        }
        ShowSessionToolbar(false);
        ScheduleSessionToolbarHide();
    }

    void RemoteSessionWindow::ReleaseRemoteInputs()
    {
        if (!desktopCanvas_) {
            return;
        }
        static_cast<RemoteDesktopCanvas*>(desktopCanvas_)
            ->ReleaseRemoteInputs();
    }
