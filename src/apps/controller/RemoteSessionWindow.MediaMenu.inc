// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

    void RemoteSessionWindow::ShowMediaDeviceMenu()
    {
        if (!sessionEngine_ || !mediaDeviceButton_) {
            return;
        }
        ShowSessionToolbar(false);
        mediaDeviceMenuOpen_ = true;
        if (toolbarHideTimer_) {
            toolbarHideTimer_->stop();
        }
        RoundedPopupMenu menu(this, 10.0);
        const auto positionMenu =
            [this, &menu] {
                menu.ensurePolished();
                menu.adjustSize();
                const QSize popupSize = menu.sizeHint();
                menu.resize(popupSize);
                const QPoint anchorBottom =
                    mediaDeviceButton_->mapToGlobal(
                        QPoint(
                            mediaDeviceButton_->width(),
                            mediaDeviceButton_->height()));
                QPoint popupPosition(
                    anchorBottom.x() - popupSize.width(),
                    anchorBottom.y() + 5);
                QScreen* screen =
                    QGuiApplication::screenAt(anchorBottom);
                if (!screen) {
                    screen = QApplication::primaryScreen();
                }
                if (screen) {
                    const QRect available =
                        screen->availableGeometry();
                    popupPosition.setX(std::clamp(
                        popupPosition.x(),
                        available.left(),
                        (std::max)(
                            available.left(),
                            available.right() -
                                popupSize.width() + 1)));
                    popupPosition.setY(std::clamp(
                        popupPosition.y(),
                        available.top(),
                        (std::max)(
                            available.top(),
                            available.bottom() -
                                popupSize.height() + 1)));
                }
                menu.move(popupPosition);
                return popupPosition;
            };
        std::uint64_t refreshRevision = 0;
        QString renderedDeviceSignature;
        QPointer<QPushButton> activeRefreshButton;
        QTimer refreshPoll(&menu);
        refreshPoll.setInterval(80);
        const auto mediaSignature =
            [](const MediaDeviceSnapshot& media) {
                QStringList parts;
                const auto appendCategory =
                    [&parts](
                        const MediaDeviceCategorySnapshot&
                            category) {
                        parts << QString::fromStdString(
                                     category.preferredDeviceId)
                              << QString::fromStdString(
                                     category.activeDeviceId)
                              << QString::fromStdString(
                                     category.activeDeviceName)
                              << QString::number(
                                     static_cast<int>(
                                         category.state));
                        for (const auto& device :
                             category.devices) {
                            parts << QString::fromStdString(
                                         device.id)
                                  << QString::fromStdString(
                                         device.name)
                                  << QString::number(
                                         device.available
                                             ? 1
                                             : 0);
                        }
                    };
                appendCategory(media.camera);
                appendCategory(media.microphone);
                appendCategory(media.speaker);
                return parts.join(QChar(0x001f));
            };
        std::function<void()> rebuildMenu;
        rebuildMenu =
            [this, &menu, &refreshPoll,
             &refreshRevision,
             &renderedDeviceSignature,
             &activeRefreshButton,
             &positionMenu, mediaSignature] {
                menu.setUpdatesEnabled(false);
                menu.clear();
                const auto media =
                    sessionEngine_->Snapshot()
                        .localMediaDevices;
                renderedDeviceSignature =
                    mediaSignature(media);
                auto* displayModeMenu =
                    new RoundedPopupMenu(&menu, 10.0);
                displayModeMenu->setTitle(
                    QStringLiteral("画面显示"));
                menu.addMenu(displayModeMenu);
                auto* displayModeGroup =
                    new QActionGroup(displayModeMenu);
                displayModeGroup->setExclusive(true);
                const bool actualPixels = desktopCanvas_ &&
                    static_cast<RemoteDesktopCanvas*>(desktopCanvas_)
                        ->ActualPixelDisplayMode();
                const auto addDisplayMode =
                    [this, displayModeMenu, displayModeGroup,
                     actualPixels](const QString& label,
                                   bool useActualPixels) {
                        QAction* action =
                            displayModeMenu->addAction(label);
                        displayModeGroup->addAction(action);
                        action->setCheckable(true);
                        action->setChecked(
                            actualPixels == useActualPixels);
                        connect(action, &QAction::triggered, this,
                            [this, useActualPixels] {
                                if (!desktopCanvas_) return;
                                static_cast<RemoteDesktopCanvas*>(
                                    desktopCanvas_)
                                    ->SetActualPixelDisplayMode(
                                        useActualPixels);
                            });
                        return action;
                    };
                addDisplayMode(
                    QStringLiteral("适应窗口（保持比例）"), false);
                QAction* actualPixelAction = addDisplayMode(
                    QStringLiteral("100% 原始像素"), true);
                auto* actualPixelHint = new QLabel(
                    QStringLiteral("按住 Alt + 左键拖动画面"),
                    displayModeMenu,
                    Qt::ToolTip | Qt::FramelessWindowHint |
                        Qt::NoDropShadowWindowHint);
                actualPixelHint->setAttribute(
                    Qt::WA_ShowWithoutActivating, true);
                actualPixelHint->setAttribute(
                    Qt::WA_TransparentForMouseEvents, true);
                actualPixelHint->setStyleSheet(QStringLiteral(
                    "QLabel {"
                    "background:#202632;"
                    "border:1px solid #4a566a;"
                    "border-radius:9px;"
                    "color:#f7f9fc;"
                    "font-size:12px;"
                    "padding:7px 10px;"
                    "}"));
                actualPixelHint->adjustSize();
                actualPixelHint->hide();
                connect(displayModeMenu, &QMenu::hovered,
                    displayModeMenu,
                    [displayModeMenu, actualPixelAction,
                     actualPixelHint](QAction* hovered) {
                        if (hovered != actualPixelAction) {
                            actualPixelHint->hide();
                            return;
                        }
                        actualPixelHint->adjustSize();
                        const QRect actionRect =
                            displayModeMenu->actionGeometry(
                                actualPixelAction);
                        QPoint position =
                            displayModeMenu->mapToGlobal(QPoint(
                                actionRect.center().x() -
                                    actualPixelHint->width() / 2,
                                actionRect.bottom() + 7));
                        QScreen* screen =
                            QGuiApplication::screenAt(position);
                        if (!screen) {
                            screen = QApplication::primaryScreen();
                        }
                        if (screen) {
                            const QRect available =
                                screen->availableGeometry();
                            position.setX(std::clamp(
                                position.x(), available.left() + 4,
                                (std::max)(available.left() + 4,
                                    available.right() -
                                        actualPixelHint->width() - 3)));
                            if (position.y() +
                                    actualPixelHint->height() >
                                available.bottom()) {
                                position.setY(
                                    displayModeMenu->mapToGlobal(
                                        QPoint(0, actionRect.top())).y() -
                                    actualPixelHint->height() - 7);
                            }
                        }
                        actualPixelHint->move(position);
                        actualPixelHint->show();
                        actualPixelHint->raise();
                    });
                connect(displayModeMenu, &QMenu::aboutToHide,
                    actualPixelHint, &QWidget::hide);
                menu.addSeparator();
                const auto addCategory =
                    [this, &menu](
                        const QString& title,
                        MediaDeviceKind kind,
                        const MediaDeviceCategorySnapshot&
                            category) {
                        auto* categoryMenu =
                            new RoundedPopupMenu(&menu, 10.0);
                        categoryMenu->setTitle(title);
                        menu.addMenu(categoryMenu);
                        const std::string preferredId =
                            category.preferredDeviceId.empty()
                                ? std::string(
                                      kSystemDefaultMediaDeviceId)
                                : category.preferredDeviceId;
                        const auto addDevice =
                            [this, categoryMenu, kind,
                             &preferredId](
                                const QString& name,
                                const std::string& deviceId,
                                bool available) {
                                auto* action =
                                    categoryMenu->addAction(
                                        name);
                                action->setCheckable(true);
                                action->setChecked(
                                    deviceId ==
                                    preferredId);
                                action->setEnabled(available);
                                connect(
                                    action,
                                    &QAction::triggered,
                                    this,
                                    [this, kind, deviceId] {
                                        BeginMediaDeviceSelection(
                                            kind,
                                            QString::fromStdString(
                                                deviceId));
                                    });
                            };
                        addDevice(
                            category.activeDeviceName.empty()
                                ? QStringLiteral(
                                      "跟随系统默认")
                                : QStringLiteral(
                                      "跟随系统默认（当前：%1）")
                                      .arg(QString::fromStdString(
                                          category.activeDeviceName)),
                            kSystemDefaultMediaDeviceId,
                            !category.devices.empty());
                        for (const auto& device :
                             category.devices) {
                            addDevice(
                                QString::fromStdString(
                                    device.name),
                                device.id,
                                device.available);
                        }
                        if (category.devices.empty()) {
                            categoryMenu->addSeparator();
                            auto* unavailable =
                                categoryMenu->addAction(
                                    QStringLiteral(
                                        "没有检测到可用设备"));
                            unavailable->setEnabled(false);
                        }
                        categoryMenu->setEnabled(
                            category.state !=
                                MediaDeviceSelectionState::
                                    kSwitching);
                    };
                addCategory(
                    QStringLiteral("摄像头"),
                    MediaDeviceKind::kCamera,
                    media.camera);
                addCategory(
                    QStringLiteral("麦克风"),
                    MediaDeviceKind::kMicrophone,
                    media.microphone);
                addCategory(
                    QStringLiteral("扬声器"),
                    MediaDeviceKind::kSpeaker,
                    media.speaker);
                menu.addSeparator();
                auto* refreshAction =
                    new QWidgetAction(&menu);
                auto* refreshButton =
                    new QPushButton(
                        media.refreshing
                            ? QStringLiteral(
                                  "正在刷新设备…")
                            : QStringLiteral(
                                  "刷新设备列表"),
                        &menu);
                refreshButton->setObjectName(
                    QStringLiteral(
                        "sessionMediaRefreshButton"));
                refreshButton->setCursor(
                    Qt::PointingHandCursor);
                refreshButton->setFocusPolicy(
                    Qt::NoFocus);
                refreshButton->setEnabled(
                    !media.refreshing);
                activeRefreshButton = refreshButton;
                refreshAction->setDefaultWidget(
                    refreshButton);
                menu.addAction(refreshAction);
                connect(
                    refreshButton,
                    &QPushButton::clicked,
                    &menu,
                    [this, &menu, refreshButton,
                     &refreshPoll,
                     &refreshRevision] {
                        menu.setActiveAction(nullptr);
                        refreshButton->clearFocus();
                        refreshRevision =
                            sessionEngine_->Snapshot()
                                .localMediaDevices
                                .revision;
                        refreshButton->setText(
                            QStringLiteral(
                                "正在刷新设备…"));
                        refreshButton->setEnabled(false);
                        const auto result =
                            sessionEngine_->
                                RefreshLocalMediaDevices();
                        if (!result.accepted) {
                            refreshButton->setText(
                                QStringLiteral(
                                    "刷新设备列表"));
                            refreshButton->setEnabled(true);
                            RemoteCToast::ShowAbove(
                                mediaDeviceButton_,
                                QString::fromStdString(
                                    result.errorMessage),
                                RemoteCToast::Tone::kError);
                            return;
                        }
                        refreshPoll.start();
                    });
                positionMenu();
                menu.setActiveAction(nullptr);
                menu.setUpdatesEnabled(true);
                menu.update();
            };
        connect(
            &refreshPoll, &QTimer::timeout, &menu,
            [this, &menu, &refreshPoll, &refreshRevision,
             &rebuildMenu, &renderedDeviceSignature,
             &activeRefreshButton, mediaSignature] {
                const auto media =
                    sessionEngine_->Snapshot()
                        .localMediaDevices;
                if (!media.refreshing &&
                    media.revision != refreshRevision) {
                    refreshPoll.stop();
                    const QString newSignature =
                        mediaSignature(media);
                    if (newSignature ==
                            renderedDeviceSignature &&
                        activeRefreshButton) {
                        activeRefreshButton->setText(
                            QStringLiteral(
                                "刷新设备列表"));
                        activeRefreshButton->setEnabled(true);
                        activeRefreshButton->clearFocus();
                        menu.setActiveAction(nullptr);
                        activeRefreshButton->update();
                    } else {
                        rebuildMenu();
                    }
                }
            });
        rebuildMenu();
        const QPoint popupPosition = positionMenu();
        menu.exec(popupPosition);
        mediaDeviceMenuOpen_ = false;
        ScheduleSessionToolbarHide();
    }
