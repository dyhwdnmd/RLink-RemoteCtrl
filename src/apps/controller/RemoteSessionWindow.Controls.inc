// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

    void RemoteSessionWindow::HandleDisconnectAction()
    {
        ReleaseRemoteInputs();
        closeWithoutConfirmation_ = true;
        if (disconnectHandler_) {
            disconnectHandler_();
        }
        close();
    }

    void RemoteSessionWindow::closeEvent(QCloseEvent* event)
    {
        if (!event) return;

        bool liveRemoteControl = false;
        if (!closeWithoutConfirmation_ && sessionEngine_) {
            const auto snapshot = sessionEngine_->Snapshot();
            if (binding_.IsDirect()) {
                liveRemoteControl =
                    snapshot.state == SessionEngineState::kConnecting ||
                    snapshot.state == SessionEngineState::kActive;
            } else if (binding_.IsRoom()) {
                liveRemoteControl =
                    snapshot.room.membership == RoomMembershipState::kActive &&
                    snapshot.room.screenShareState ==
                        RoomScreenShareState::kActive &&
                    !snapshot.room.screenSharerDeviceId.empty() &&
                    snapshot.room.screenSharerDeviceId !=
                        snapshot.localDeviceId;
            }
        }

        if (!liveRemoteControl) {
            FramelessMainWindow::closeEvent(event);
            return;
        }
        if (closeConfirmationVisible_) {
            event->ignore();
            return;
        }

        event->ignore();
        closeConfirmationVisible_ = true;
        const bool confirmed = RemoteCDialog::Confirm(
            this,
            QStringLiteral("结束本次远程控制？"),
            QStringLiteral(
                "关闭监控窗口将断开本次远程控制连接。\n"
                "如果还要继续操作，请选择“继续控制”。"),
            QStringLiteral("结束远控"),
            QStringLiteral("继续控制"),
            RemoteCDialog::Tone::kWarning,
            true);
        closeConfirmationVisible_ = false;
        if (!confirmed) return;

        closeWithoutConfirmation_ = true;
        ReleaseRemoteInputs();
        if (disconnectHandler_) disconnectHandler_();
        event->accept();
        FramelessMainWindow::closeEvent(event);
    }

    void RemoteSessionWindow::UpdateSessionDuration()
    {
        RefreshControlState();
        const qint64 totalSeconds = sessionElapsed_.elapsed() / 1000;
        const qint64 hours = totalSeconds / 3600;
        const qint64 minutes = (totalSeconds % 3600) / 60;
        const qint64 seconds = totalSeconds % 60;
        durationLabel_->setText(
            QStringLiteral("%1:%2:%3")
            .arg(hours, 2, 10, QLatin1Char('0'))
            .arg(minutes, 2, 10, QLatin1Char('0'))
            .arg(seconds, 2, 10, QLatin1Char('0')));
    }

    void RemoteSessionWindow::UpdateNetworkRecoveryPrompt(
        bool recovering,
        std::uint32_t attempt,
        bool waitingForSignaling,
        bool failed)
    {
        if (!networkRecoveryDialog_) {
            networkRecoveryDialog_ = RemoteCDialog::CreateStatus(
                this,
                QStringLiteral("正在恢复网络连接"),
                QStringLiteral(
                    "正在检测可用网络并等待 P2P 自恢复…\n当前显示的是断开前最后一帧，远程输入已暂停。"),
                QStringLiteral("隐藏提示"),
                RemoteCDialog::Tone::kWarning);
            connect(networkRecoveryDialog_, &QDialog::finished,
                    this, [this](int result) {
                        if (networkRecoveryShowingFailure_) {
                            networkRecoveryFailureDismissed_ = true;
                            if (result == QDialog::Accepted) {
                                auto* engine = sessionEngine_;
                                BindSessionVideo(nullptr, binding_);
                                close();
                                if (engine) {
                                    if (binding_.IsDirect()) {
                                        (void)engine->Disconnect();
                                    } else {
                                        engine->ExitRoomAfterRecoveryFailure();
                                    }
                                }
                            }
                        } else {
                            networkRecoveryPromptDismissed_ = true;
                        }
                    });
        }

        if (!recovering && !failed) {
            networkRecoveryPromptDismissed_ = false;
            networkRecoveryFailureDismissed_ = false;
            networkRecoveryShowingFailure_ = false;
            const QPoint recoveryDialogCenter =
                networkRecoveryDialog_->frameGeometry().center();
            if (networkRecoveryDialog_->isVisible()) {
                networkRecoveryDialog_->hide();
            }
            if (recoveryPromptWasVisible_) {
                recoveryPromptWasVisible_ = false;
                RemoteCToast::ShowAtGlobalCenter(
                    this,
                    recoveryDialogCenter,
                    QStringLiteral("网络连接已经恢复"),
                    RemoteCToast::Tone::kSuccess,
                    true);
            }
            return;
        }

        if (failed && networkRecoveryFailureDismissed_) {
            return;
        }

        if (failed) {
            if (!networkRecoveryShowingFailure_) {
                networkRecoveryFailureDismissed_ = false;
            }
            networkRecoveryShowingFailure_ = true;
            networkRecoveryDialog_->SetContent(
                QStringLiteral("连接恢复失败"),
                binding_.IsDirect()
                    ? QStringLiteral(
                          "连接已彻底中断。点击确定后将关闭监控窗口并结束当前远程会话。")
                    : QStringLiteral(
                          "连接已彻底中断。点击确定后将关闭监控窗口并退出当前房间。"),
                binding_.IsDirect()
                    ? QStringLiteral("确定并结束")
                    : QStringLiteral("确定并退出"),
                RemoteCDialog::Tone::kDanger);
        } else {
            networkRecoveryShowingFailure_ = false;
            networkRecoveryFailureDismissed_ = false;
            networkRecoveryDialog_->SetContent(
                waitingForSignaling
                    ? QStringLiteral("正在恢复信令连接")
                    : (attempt == 0
                        ? QStringLiteral("正在恢复网络连接")
                        : QStringLiteral("正在重新建立 P2P 连接")),
                waitingForSignaling
                    ? QStringLiteral(
                          "正在重新连接信令服务器…\n连接恢复后将自动重建 P2P，无需关闭远程窗口。")
                    : (attempt == 0
                        ? QStringLiteral(
                              "正在检测可用网络并等待 P2P 自恢复…\n当前显示的是断开前最后一帧，远程输入已暂停。")
                        : QStringLiteral(
                              "正在执行第 %1/3 次 ICE Restart…\n连接稳定后，画面和远程控制将自动继续。")
                              .arg(attempt)),
                QStringLiteral("隐藏提示"),
                RemoteCDialog::Tone::kWarning);
            if (networkRecoveryPromptDismissed_) {
                return;
            }
        }
        if (!networkRecoveryDialog_->isVisible()) {
            QApplication::beep();
            networkRecoveryDialog_->show();
        }
        networkRecoveryDialog_->raise();
        recoveryPromptWasVisible_ = recovering;
    }

    void RemoteSessionWindow::ShowRemotePasteProgress(
        const QString& transferId, const QString& title,
        const QString& message, double progress,
        std::uintptr_t localTargetWindow)
    {
        if (transferId.isEmpty()) return;
        const HWND localTarget = reinterpret_cast<HWND>(localTargetWindow);
        const bool localTargetValid = localTarget && IsWindow(localTarget);
        if (remotePasteTransferId_ != transferId) {
            if (remotePasteAnimation_) {
                remotePasteAnimation_->stop();
                remotePasteAnimation_->deleteLater();
                remotePasteAnimation_ = nullptr;
            }
            if (remotePasteAnimationOverlay_) {
                remotePasteAnimationOverlay_->deleteLater();
                remotePasteAnimationOverlay_ = nullptr;
            }
            if (remotePasteDialog_) {
                // close() runs the native dialog-finished/owner activation
                // path. Hide and retire the non-activating popup directly so
                // Explorer remains the foreground window.
                auto* oldDialog = remotePasteDialog_;
                remotePasteDialog_ = nullptr;
                oldDialog->hide();
                oldDialog->deleteLater();
            }
            remotePasteTransferId_ = transferId;
            remotePastePromptDismissed_ = false;
            remotePasteMinimized_ = false;
            remotePasteRestoreAnimating_ = false;
            remotePasteRestoreGeometry_ = {};
            remotePasteDialogSnapshot_ = {};
            SetRemotePasteStatusButtonVisible(false);
        }
        if (remotePastePromptDismissed_) return;
        if (!remotePasteDialog_) {
            auto* dialog = RemoteCDialog::CreateStatus(
                this,
                title,
                message,
                QStringLiteral("后台传输"),
                RemoteCDialog::Tone::kInformation);
            remotePasteDialog_ = dialog;
            if (localTargetValid) {
                dialog->SetNonActivatingWindow(true);
            }
            dialog->SetStatusActionHandler(
                [this] { MinimizeRemotePasteProgress(); });
            connect(dialog, &QDialog::finished, this,
                    [this, dialog](int) {
                        if (remotePasteDialog_ != dialog) return;
                        if (!remotePasteDialogClosing_) {
                            remotePastePromptDismissed_ = true;
                            if (remotePasteCancelHandler_) {
                                remotePasteCancelHandler_();
                            }
                        }
                        remotePasteDialog_ = nullptr;
                        remotePasteMinimized_ = false;
                        remotePasteRestoreAnimating_ = false;
                        remotePasteRestoreGeometry_ = {};
                        remotePasteDialogSnapshot_ = {};
                        SetRemotePasteStatusButtonVisible(false);
                        dialog->deleteLater();
                    });
        }
        remotePasteDialog_->SetProgress(progress);
        remotePasteDialog_->SetContent(
            title, message,
            QStringLiteral("后台传输"),
            RemoteCDialog::Tone::kInformation);
        if (remotePasteStatusButton_) {
            static_cast<LiquidTransferButton*>(remotePasteStatusButton_)
                ->SetProgress(progress);
            remotePasteStatusButton_->setToolTip(
                QStringLiteral("%1\n%2").arg(title, message));
        }
        if (remotePasteMinimized_ || remotePasteRestoreAnimating_) return;
        const bool firstShow = !remotePasteDialog_->isVisible();
        if (firstShow) {
            remotePasteDialog_->show();
            if (localTargetValid) {
                RECT nativeRect{};
                if (GetWindowRect(localTarget, &nativeRect)) {
                    const QSize dialogSize = remotePasteDialog_->size();
                    int x = nativeRect.left +
                        ((nativeRect.right - nativeRect.left) -
                         dialogSize.width()) / 2;
                    int y = nativeRect.top + 28;
                    if (QScreen* screen = QGuiApplication::screenAt(
                            QPoint((nativeRect.left + nativeRect.right) / 2,
                                   (nativeRect.top + nativeRect.bottom) / 2))) {
                        const QRect available = screen->availableGeometry();
                        x = std::clamp(x, available.left(),
                            (std::max)(available.left(),
                                available.right() - dialogSize.width() + 1));
                        y = std::clamp(y, available.top(),
                            (std::max)(available.top(),
                                available.bottom() - dialogSize.height() + 1));
                    }
                    remotePasteDialog_->move(x, y);
                    SetWindowPos(
                        reinterpret_cast<HWND>(remotePasteDialog_->winId()),
                        HWND_TOPMOST, x, y, 0, 0,
                        SWP_NOSIZE | SWP_NOACTIVATE | SWP_SHOWWINDOW);
                }
            } else {
                remotePasteDialog_->raise();
            }
        }
    }

    void RemoteSessionWindow::CompleteRemotePasteProgress(
        const QString& transferId)
    {
        if (transferId.isEmpty() ||
            transferId != remotePasteTransferId_) return;
        if (!remotePasteDialog_ || remotePasteMinimized_ ||
            !remotePasteDialog_->isVisible()) {
            CloseRemotePasteProgress(transferId);
            return;
        }

        // The worker publishes the final active snapshot and the following
        // ready snapshot back-to-back. The UI intentionally coalesces worker
        // notifications, so explicitly paint the completed value for one
        // frame before retiring the popup.
        remotePasteDialog_->SetProgress(1.0);
        if (remotePasteStatusButton_) {
            static_cast<LiquidTransferButton*>(remotePasteStatusButton_)
                ->SetProgress(1.0);
        }
        QTimer::singleShot(120, this, [this, transferId] {
            if (remotePasteTransferId_ == transferId) {
                CloseRemotePasteProgress(transferId);
            }
        });
    }

    void RemoteSessionWindow::CloseRemotePasteProgress(
        const QString& transferId)
    {
        if (!transferId.isEmpty() &&
            transferId != remotePasteTransferId_) return;
        if (remotePasteAnimation_) {
            remotePasteAnimation_->stop();
            remotePasteAnimation_->deleteLater();
            remotePasteAnimation_ = nullptr;
        }
        if (remotePasteAnimationOverlay_) {
            remotePasteAnimationOverlay_->deleteLater();
            remotePasteAnimationOverlay_ = nullptr;
        }
        if (remotePasteDialog_) {
            auto* finishedDialog = remotePasteDialog_;
            remotePasteDialog_ = nullptr;
            finishedDialog->hide();
            finishedDialog->deleteLater();
        }
        remotePasteTransferId_.clear();
        remotePastePromptDismissed_ = false;
        remotePasteMinimized_ = false;
        remotePasteRestoreAnimating_ = false;
        remotePasteRestoreGeometry_ = {};
        remotePasteDialogSnapshot_ = {};
        SetRemotePasteStatusButtonVisible(false);
    }

    void RemoteSessionWindow::ShowRemotePasteFailure(
        const QString& transferId, const QString& message)
    {
        if (!transferId.isEmpty()) {
            remotePasteTransferId_ = transferId;
        }
        if (remotePasteAnimation_) {
            remotePasteAnimation_->stop();
            remotePasteAnimation_->deleteLater();
            remotePasteAnimation_ = nullptr;
        }
        if (remotePasteAnimationOverlay_) {
            remotePasteAnimationOverlay_->deleteLater();
            remotePasteAnimationOverlay_ = nullptr;
        }
        remotePastePromptDismissed_ = false;
        remotePasteMinimized_ = false;
        remotePasteRestoreAnimating_ = false;
        SetRemotePasteStatusButtonVisible(false);
        if (!remotePasteDialog_) {
            auto* dialog = RemoteCDialog::CreateStatus(
                this,
                QStringLiteral("远程粘贴未完成"),
                message,
                QStringLiteral("知道了"),
                RemoteCDialog::Tone::kDanger);
            remotePasteDialog_ = dialog;
            connect(dialog, &QDialog::finished, this,
                    [this, dialog](int) {
                        if (remotePasteDialog_ != dialog) return;
                        remotePasteDialog_ = nullptr;
                        remotePasteTransferId_.clear();
                        remotePastePromptDismissed_ = false;
                        remotePasteRestoreGeometry_ = {};
                        remotePasteDialogSnapshot_ = {};
                        SetRemotePasteStatusButtonVisible(false);
                        dialog->deleteLater();
                    });
        }
        remotePasteDialog_->SetStatusActionHandler({});
        remotePasteDialog_->SetProgress(-1.0);
        remotePasteDialog_->SetContent(
            QStringLiteral("远程粘贴未完成"), message,
            QStringLiteral("知道了"),
            RemoteCDialog::Tone::kDanger);
        if (!remotePasteDialog_->isVisible()) {
            QApplication::beep();
            remotePasteDialog_->show();
        }
        remotePasteDialog_->raise();
    }

    void RemoteSessionWindow::SetRemotePasteStatusButtonVisible(bool visible)
    {
        if (!remotePasteStatusButton_ || !remotePasteStatusHost_ ||
            !sessionHud_) return;
        remotePasteStatusHost_->setVisible(visible);
        sessionHud_->setFixedHeight(visible ? 166 : 128);
        sessionHud_->updateGeometry();
        if (isMinimized()) {
            // mapToGlobal() is not meaningful while the native owner is
            // iconic; changeEvent() performs the layout after restoration.
            return;
        }
        LayoutSessionOverlays();
    }

    QRect RemoteSessionWindow::RemotePasteDialogRestoreGeometry() const
    {
        if (remotePasteRestoreGeometry_.isValid()) {
            return remotePasteRestoreGeometry_;
        }
        if (!remotePasteDialog_) return {};
        const QRect frame = frameGeometry();
        const QSize size = remotePasteDialog_->size();
        return QRect(
            frame.center().x() - size.width() / 2,
            frame.center().y() - size.height() / 2,
            size.width(), size.height());
    }

    QLabel* RemoteSessionWindow::CreateRemotePasteAnimationOverlay(
        const QPixmap& snapshot, const QRect& geometry, qreal opacity)
    {
        // The progress dialog may be displayed above Explorer or another local
        // application.  Keeping the animation owned by RemoteSessionWindow
        // causes Windows to raise the monitoring window before the animation
        // starts.  Use an ownerless, non-activating top-level overlay instead.
        auto* overlay = new QLabel(
            nullptr, Qt::Tool | Qt::FramelessWindowHint |
                Qt::WindowStaysOnTopHint |
                Qt::NoDropShadowWindowHint |
                Qt::WindowTransparentForInput |
                Qt::WindowDoesNotAcceptFocus);
        overlay->setAttribute(Qt::WA_TranslucentBackground, true);
        overlay->setAttribute(Qt::WA_ShowWithoutActivating, true);
        overlay->setScaledContents(true);
        overlay->setPixmap(snapshot);
        overlay->setGeometry(geometry);
        overlay->setWindowOpacity(opacity);

        const HWND overlayHwnd = reinterpret_cast<HWND>(overlay->winId());
        if (overlay->windowHandle()) {
            overlay->windowHandle()->setTransientParent(nullptr);
        }
        if (overlayHwnd) {
            LONG_PTR extendedStyle =
                ::GetWindowLongPtrW(overlayHwnd, GWL_EXSTYLE);
            extendedStyle |= WS_EX_NOACTIVATE | WS_EX_TOOLWINDOW |
                WS_EX_TRANSPARENT;
            ::SetWindowLongPtrW(
                overlayHwnd, GWL_EXSTYLE, extendedStyle);
            ::SetWindowLongPtrW(overlayHwnd, GWLP_HWNDPARENT, 0);
            // winId() creates the native tool window. Position that hidden
            // HWND before Qt makes it visible; otherwise DWM can compose one
            // frame at the default top-left coordinate.
            ::SetWindowPos(
                overlayHwnd, HWND_TOPMOST,
                geometry.x(), geometry.y(),
                geometry.width(), geometry.height(),
                SWP_NOACTIVATE | SWP_NOOWNERZORDER);
        }

        overlay->show();
        if (overlayHwnd) {
            ::SetWindowPos(
                overlayHwnd, HWND_TOPMOST,
                geometry.x(), geometry.y(),
                geometry.width(), geometry.height(),
                SWP_NOACTIVATE | SWP_SHOWWINDOW);
        }
        return overlay;
    }

    void RemoteSessionWindow::MinimizeRemotePasteProgress()
    {
        if (!remotePasteDialog_ || remotePasteMinimized_) return;
        if (remotePasteAnimation_) {
            remotePasteAnimation_->stop();
            remotePasteAnimation_->deleteLater();
            remotePasteAnimation_ = nullptr;
        }
        if (remotePasteAnimationOverlay_) {
            remotePasteAnimationOverlay_->deleteLater();
            remotePasteAnimationOverlay_ = nullptr;
        }
        // This is a top-level, frameless dialog. Keep its global frame
        // coordinate for the ownerless native animation overlay.
        remotePasteRestoreGeometry_ = remotePasteDialog_->frameGeometry();
        remotePasteDialogSnapshot_ = remotePasteDialog_->grab();
        remotePasteMinimized_ = true;
        remotePasteRestoreAnimating_ = false;
        SetRemotePasteStatusButtonVisible(true);

        const HWND sessionWindow = reinterpret_cast<HWND>(winId());
        const bool sessionWindowMinimized = isMinimized() ||
            (sessionWindow && ::IsIconic(sessionWindow));
        if (sessionWindowMinimized) {
            // There is no visible destination button while the monitoring
            // window is minimized. Hide immediately instead of flashing an
            // animation toward an off-screen native window coordinate.
            remotePasteDialog_->hide();
            remotePasteDialog_->setWindowOpacity(1.0);
            remotePasteDialog_->setGeometry(remotePasteRestoreGeometry_);
            return;
        }

        const QRect buttonRect(
            remotePasteStatusButton_->mapToGlobal(QPoint(0, 0)),
            remotePasteStatusButton_->size());
        const QRect target = buttonRect;
        if (CurrentUiAnimationLevel() <= 0) {
            remotePasteDialog_->hide();
            remotePasteDialog_->setWindowOpacity(1.0);
            remotePasteDialog_->setGeometry(remotePasteRestoreGeometry_);
            return;
        }

        auto* overlay = CreateRemotePasteAnimationOverlay(
            remotePasteDialogSnapshot_, remotePasteRestoreGeometry_, 0.0);
        remotePasteAnimationOverlay_ = overlay;
        remotePasteDialog_->hide();
        remotePasteDialog_->setWindowOpacity(1.0);

        auto* animation = new QVariantAnimation(this);
        remotePasteAnimation_ = animation;
        animation->setDuration(560);
        animation->setStartValue(0.0);
        animation->setEndValue(1.0);
        QEasingCurve curve(QEasingCurve::BezierSpline);
        curve.addCubicBezierSegment(
            QPointF(0.20, 0.58), QPointF(0.18, 1.0), QPointF(1.0, 1.0));
        animation->setEasingCurve(curve);
        const QRect start = remotePasteRestoreGeometry_;
        const QPointF startCenter = start.center();
        const QPointF endCenter = target.center();
        const QPointF control(
            endCenter.x(), startCenter.y() +
                (endCenter.y() - startCenter.y()) * 0.22);
        connect(animation, &QVariantAnimation::valueChanged, overlay,
                [overlay, start, target, startCenter, endCenter, control]
                (const QVariant& value) {
                    const qreal t = value.toReal();
                    const qreal inverse = 1.0 - t;
                    const QPointF center = inverse * inverse * startCenter +
                        2.0 * inverse * t * control + t * t * endCenter;
                    const qreal sizeT = std::pow(t, 1.18);
                    qreal width = start.width() +
                        (target.width() - start.width()) * sizeT;
                    qreal height = start.height() +
                        (target.height() - start.height()) *
                            std::pow(t, 0.96);
                    if (t < 0.16) {
                        const qreal settle = std::sin(
                            t / 0.16 * 3.14159265358979323846) * 0.018;
                        width *= 1.0 - settle;
                        height *= 1.0 - settle;
                    }
                    overlay->setGeometry(QRect(
                        qRound(center.x() - width / 2.0),
                        qRound(center.y() - height / 2.0),
                        (std::max)(1, qRound(width)),
                        (std::max)(1, qRound(height))));
                    const qreal opacity = t <= 0.84
                        ? 1.0 : (1.0 - (t - 0.84) / 0.16);
                    overlay->setWindowOpacity(std::clamp(opacity, 0.0, 1.0));
                });
        QPointer<RemoteCDialog> dialog = remotePasteDialog_;
        connect(animation, &QVariantAnimation::finished, this,
                [this, animation, overlay, dialog] {
                    if (remotePasteAnimation_ == animation) {
                        remotePasteAnimation_ = nullptr;
                    }
                    if (remotePasteAnimationOverlay_ == overlay) {
                        remotePasteAnimationOverlay_ = nullptr;
                    }
                    overlay->deleteLater();
                    if (dialog && remotePasteMinimized_) {
                        dialog->hide();
                        dialog->setWindowOpacity(1.0);
                        dialog->setGeometry(remotePasteRestoreGeometry_);
                    }
                });
        animation->start(QAbstractAnimation::DeleteWhenStopped);
    }

    void RemoteSessionWindow::RestoreRemotePasteProgress()
    {
        if (!remotePasteDialog_ || !remotePasteMinimized_) return;
        if (remotePasteAnimation_) {
            remotePasteAnimation_->stop();
            remotePasteAnimation_->deleteLater();
            remotePasteAnimation_ = nullptr;
        }
        if (remotePasteAnimationOverlay_) {
            remotePasteAnimationOverlay_->deleteLater();
            remotePasteAnimationOverlay_ = nullptr;
        }
        const QRect buttonRect(
            remotePasteStatusButton_->mapToGlobal(QPoint(0, 0)),
            remotePasteStatusButton_->size());
        const QRect start = buttonRect;
        const QRect target = RemotePasteDialogRestoreGeometry();
        remotePasteMinimized_ = false;
        remotePasteRestoreAnimating_ =
            CurrentUiAnimationLevel() > 0;
        SetRemotePasteStatusButtonVisible(false);

        if (CurrentUiAnimationLevel() <= 0) {
            remotePasteRestoreAnimating_ = false;
            remotePasteDialog_->setGeometry(target);
            remotePasteDialog_->setWindowOpacity(1.0);
            remotePasteDialog_->show();
            remotePasteDialog_->raise();
            return;
        }

        if (remotePasteDialog_->layout()) {
            remotePasteDialog_->layout()->activate();
        }
        remotePasteDialogSnapshot_ = remotePasteDialog_->grab();
        const QPixmap snapshot = remotePasteDialogSnapshot_.isNull()
            ? remotePasteDialog_->grab() : remotePasteDialogSnapshot_;
        auto* overlay = CreateRemotePasteAnimationOverlay(
            snapshot, start, 0.0);
        remotePasteAnimationOverlay_ = overlay;

        auto* animation = new QVariantAnimation(this);
        remotePasteAnimation_ = animation;
        animation->setDuration(470);
        animation->setStartValue(0.0);
        animation->setEndValue(1.0);
        QEasingCurve curve(QEasingCurve::BezierSpline);
        curve.addCubicBezierSegment(
            QPointF(0.18, 0.72), QPointF(0.20, 1.0), QPointF(1.0, 1.0));
        animation->setEasingCurve(curve);
        const QPointF startCenter = start.center();
        const QPointF endCenter = target.center();
        const QPointF control(
            startCenter.x(), endCenter.y() +
                (startCenter.y() - endCenter.y()) * 0.22);
        connect(animation, &QVariantAnimation::valueChanged, overlay,
                [overlay, start, target, startCenter, endCenter, control]
                (const QVariant& value) {
                    const qreal t = value.toReal();
                    const qreal inverse = 1.0 - t;
                    const QPointF center = inverse * inverse * startCenter +
                        2.0 * inverse * t * control + t * t * endCenter;
                    const qreal sizeT = std::pow(t, 0.78);
                    const qreal width = start.width() +
                        (target.width() - start.width()) * sizeT;
                    const qreal height = start.height() +
                        (target.height() - start.height()) *
                            std::pow(t, 0.86);
                    overlay->setGeometry(QRect(
                        qRound(center.x() - width / 2.0),
                        qRound(center.y() - height / 2.0),
                        (std::max)(1, qRound(width)),
                        (std::max)(1, qRound(height))));
                    overlay->setWindowOpacity(
                        std::clamp(0.15 + t * 0.85, 0.0, 1.0));
                });
        QPointer<RemoteCDialog> dialog = remotePasteDialog_;
        connect(animation, &QVariantAnimation::finished, this,
                [this, animation, overlay, dialog, target] {
                    if (remotePasteAnimation_ == animation) {
                        remotePasteAnimation_ = nullptr;
                    }
                    if (dialog) {
                        dialog->setGeometry(target);
                        dialog->setWindowOpacity(1.0);
                        dialog->show();
                        dialog->raise();
                    }
                    QPointer<QWidget> overlayGuard = overlay;
                    QTimer::singleShot(0, this,
                        [this, overlayGuard, dialog] {
                            remotePasteRestoreAnimating_ = false;
                            if (overlayGuard) {
                                if (remotePasteAnimationOverlay_ ==
                                    overlayGuard.data()) {
                                    remotePasteAnimationOverlay_ = nullptr;
                                }
                                overlayGuard->hide();
                                overlayGuard->deleteLater();
                            }
                            if (dialog) dialog->raise();
                        });
                });
        animation->start(QAbstractAnimation::DeleteWhenStopped);
    }

    void RemoteSessionWindow::HandleControlAction()
    {
        if (!sessionEngine_ || !binding_.IsRoom() || !controlButton_) {
            return;
        }

        const auto snapshot = sessionEngine_->Snapshot();
        const bool localControls =
            snapshot.room.activeControllerDeviceId == snapshot.localDeviceId;
        if (localControls) {
            ReleaseRemoteInputs();
        }

        controlButton_->setEnabled(false);
        controlButton_->SetIcon(QString());
        controlButton_->SetText(localControls
            ? QStringLiteral("正在释放…")
            : QStringLiteral("申请已发送…"));
        const auto result = localControls
            ? sessionEngine_->ReleaseRoomControl()
            : sessionEngine_->RequestRoomControl();
        if (!result.accepted) {
            RefreshControlState();
            controlButton_->setToolTip(
                QStringLiteral("控制操作失败：%1")
                .arg(QString::fromStdString(result.errorMessage)));
        }
    }

    void RemoteSessionWindow::RebuildFrameRateMenu()
    {
        if (!frameRateMenu_ || !frameRateGroup_) {
            return;
        }
        reportedRemoteMaximumFrameRate_ = std::clamp(
            reportedRemoteMaximumFrameRate_,
            kMinimumScreenFrameRate,
            kMaximumScreenFrameRate);
        remoteMaximumFrameRate_ = (std::min)(
            reportedRemoteMaximumFrameRate_, roomMaximumFrameRate_);
        selectedFrameRate_ = (std::min)(
            selectedFrameRate_, remoteMaximumFrameRate_);
        if (desktopCanvas_) {
            static_cast<RemoteDesktopCanvas*>(desktopCanvas_)
                ->SetTargetFrameRate(selectedFrameRate_);
        }
        frameRateMenu_->clear();
        constexpr std::array<std::uint32_t, 8> kFrameRates = {
            15u, 24u, 30u, 45u, 60u, 80u, 100u, 120u};
        for (const std::uint32_t framesPerSecond : kFrameRates) {
            if (framesPerSecond > remoteMaximumFrameRate_) {
                continue;
            }
            QAction* action = frameRateMenu_->addAction(
                QStringLiteral("%1 FPS").arg(framesPerSecond));
            action->setCheckable(true);
            action->setData(static_cast<qulonglong>(framesPerSecond));
            action->setChecked(framesPerSecond == selectedFrameRate_);
            frameRateGroup_->addAction(action);
        }
        if (frameRateButton_) {
            frameRateButton_->setText(
                QStringLiteral("目标帧率\n%1 FPS")
                    .arg(selectedFrameRate_));
            frameRateButton_->setToolTip(
                QStringLiteral(
                    "当前共享端最高支持 %1 FPS；点击选择目标帧率")
                    .arg(remoteMaximumFrameRate_));
        }
    }

    void RemoteSessionWindow::RebuildQualityMenu()
    {
        if (!qualityMenu_ || !qualityGroup_) {
            return;
        }
        qualityMenu_->clear();
        constexpr std::array<ScreenQualityTier, 4> kQualities = {
            ScreenQualityTier::k720p,
            ScreenQualityTier::k1080p,
            ScreenQualityTier::k1440p,
            ScreenQualityTier::kOriginal};
        const bool knownSource =
            remoteSourceWidth_ > 0 && remoteSourceHeight_ > 0;
        for (const ScreenQualityTier quality : kQualities) {
            const auto [width, height] = ScreenQualityBounds(quality);
            const bool presetExceedsSource =
                quality != ScreenQualityTier::kOriginal && knownSource &&
                width >= remoteSourceWidth_ &&
                height >= remoteSourceHeight_ &&
                (width > remoteSourceWidth_ ||
                 height > remoteSourceHeight_);
            if (presetExceedsSource) {
                continue;
            }
            const QString actionText =
                quality == ScreenQualityTier::kOriginal && knownSource
                ? QStringLiteral("原始画质（%1 × %2）")
                      .arg(remoteSourceWidth_)
                      .arg(remoteSourceHeight_)
                : ScreenQualityText(quality);
            QAction* action = qualityMenu_->addAction(actionText);
            action->setCheckable(true);
            action->setData(static_cast<int>(quality));
            action->setChecked(quality == selectedQuality_);
            qualityGroup_->addAction(action);
        }
        if (knownSource) {
            qualityButton_->setToolTip(
                QStringLiteral(
                    "共享端当前显示器为 %1 × %2；不会提供超过源分辨率的档位")
                    .arg(remoteSourceWidth_)
                    .arg(remoteSourceHeight_));
        }
    }

    void RemoteSessionWindow::HandleFrameRateSelection(
        std::uint32_t framesPerSecond)
    {
        if (!sessionEngine_ || !binding_.IsValid() || !frameRateButton_) {
            return;
        }
        if (framesPerSecond > remoteMaximumFrameRate_) {
            RemoteCToast::ShowAbove(
                frameRateButton_,
                QStringLiteral("当前共享端的目标帧率上限为 %1 FPS")
                    .arg(remoteMaximumFrameRate_),
                RemoteCToast::Tone::kInformation);
            return;
        }
        const std::uint32_t previous = selectedFrameRate_;
        selectedFrameRate_ = framesPerSecond;
        if (desktopCanvas_) {
            static_cast<RemoteDesktopCanvas*>(desktopCanvas_)
                ->SetTargetFrameRate(selectedFrameRate_);
        }
        if (!RequestStreamPreference()) {
            selectedFrameRate_ = previous;
            if (desktopCanvas_) {
                static_cast<RemoteDesktopCanvas*>(desktopCanvas_)
                    ->SetTargetFrameRate(selectedFrameRate_);
            }
            if (frameRateMenu_) {
                for (QAction* action : frameRateMenu_->actions()) {
                    action->setChecked(
                        action->data().toUInt() == selectedFrameRate_);
                }
            }
            return;
        }
        frameRateButton_->setText(
            QStringLiteral("目标帧率\n%1 FPS").arg(framesPerSecond));
        RemoteCToast::ShowAbove(
            frameRateButton_,
            QStringLiteral("已请求远端切换到 %1 FPS")
            .arg(framesPerSecond),
            RemoteCToast::Tone::kSuccess);
    }

    void RemoteSessionWindow::HandleQualitySelection(ScreenQualityTier quality)
    {
        if (!sessionEngine_ || !binding_.IsValid() || !qualityButton_) {
            return;
        }
        const ScreenQualityTier previous = selectedQuality_;
        selectedQuality_ = quality;
        if (!RequestStreamPreference()) {
            selectedQuality_ = previous;
            if (qualityMenu_) {
                for (QAction* action : qualityMenu_->actions()) {
                    action->setChecked(
                        action->data().toInt() ==
                        static_cast<int>(selectedQuality_));
                }
            }
            return;
        }
        const auto [width, height] = ScreenQualityBounds(quality);
        qualityButton_->setText(
            width > 0 && height > 0
            ? QStringLiteral("分辨率\n%1 × %2")
            .arg(width)
            .arg(height)
            : QStringLiteral("分辨率\n原始画质"));
        RemoteCToast::ShowAbove(
            qualityButton_,
            QStringLiteral("已请求远端切换到%1")
            .arg(ScreenQualityText(quality)),
            RemoteCToast::Tone::kSuccess);
    }

    bool RemoteSessionWindow::RequestStreamPreference(bool showError)
    {
        if (!sessionEngine_ || !binding_.IsValid()) {
            return false;
        }
        const auto [width, height] = ScreenQualityBounds(selectedQuality_);
        ScreenStreamPreferenceRequest request;
        request.maxWidth = width;
        request.maxHeight = height;
        request.framesPerSecond = selectedFrameRate_;
        request.quality = selectedQuality_;
        const auto result = binding_.IsDirect()
            ? sessionEngine_->SetDirectScreenStreamPreference(request)
            : sessionEngine_->SetRoomScreenStreamPreference(
                  binding_.roomPairId.toStdString(), request);
        if (result.accepted) {
            return true;
        }
        if (showError) {
            QWidget* anchor = qualityButton_
                ? static_cast<QWidget*>(qualityButton_)
                : static_cast<QWidget*>(frameRateButton_);
            RemoteCToast::ShowAbove(
                anchor, QString::fromStdString(result.errorMessage),
                RemoteCToast::Tone::kError);
        }
        return false;
    }

    void RemoteSessionWindow::ToggleRemoteSound()
    {
        if (!sessionEngine_ || !speakerButton_) {
            return;
        }
        const auto snapshot = sessionEngine_->Snapshot();
        const auto result = sessionEngine_->SetRemoteAudioPlaybackMuted(
            !snapshot.roomAudioPlaybackMuted);
        if (!result.accepted) {
            RemoteCToast::ShowAbove(
                speakerButton_, QString::fromStdString(result.errorMessage),
                RemoteCToast::Tone::kError);
            return;
        }
        RemoteCToast::ShowAbove(
            speakerButton_,
            snapshot.roomAudioPlaybackMuted
            ? QStringLiteral("远端声音已开启")
            : QStringLiteral("远端声音已关闭"),
            RemoteCToast::Tone::kSuccess);
        RefreshControlState();
    }

    void RemoteSessionWindow::ToggleLocalMicrophone()
    {
        if (!sessionEngine_ || !microphoneButton_) {
            return;
        }
        const auto snapshot = sessionEngine_->Snapshot();
        const bool enabled =
            snapshot.localMicrophone == LocalMicrophoneState::kPublishing ||
            snapshot.localMicrophone == LocalMicrophoneState::kStarting;
        const auto result = sessionEngine_->SetLocalMicrophoneEnabled(!enabled);
        if (!result.accepted) {
            RemoteCToast::ShowAbove(
                microphoneButton_, QString::fromStdString(result.errorMessage),
                RemoteCToast::Tone::kError);
            return;
        }
        RemoteCToast::ShowAbove(
            microphoneButton_,
            enabled ? QStringLiteral("正在关闭本机麦克风")
            : QStringLiteral("正在开启本机麦克风"),
            RemoteCToast::Tone::kSuccess);
        RefreshControlState();
    }
