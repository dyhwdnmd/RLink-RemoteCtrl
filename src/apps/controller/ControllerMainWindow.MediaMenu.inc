// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

void ControllerMainWindow::ShowMediaDeviceMenu(
    MediaDeviceKind kind, QWidget* anchor)
{
    if (!engine_ || !anchor) {
        return;
    }
    RoundedPopupMenu menu(this);
    menu.SetToggleAnchor(anchor);
    menu.setObjectName(QStringLiteral("mediaDeviceMenu"));
    menu.setAttribute(Qt::WA_TranslucentBackground, false);
    menu.setWindowFlag(Qt::NoDropShadowWindowHint, true);
    menu.setStyleSheet(QStringLiteral(R"(
QMenu#mediaDeviceMenu {
    background: #ffffff;
    border: 1px solid #dce2eb;
    border-radius: 12px;
    padding: 7px;
    color: #263248;
    font-size: 13px;
}
QMenu#mediaDeviceMenu::item {
    min-width: 210px;
    min-height: 22px;
    border-radius: 8px;
    padding: 8px 36px 8px 14px;
}
QMenu#mediaDeviceMenu::item:selected {
    background: #f3f6fb;
    color: #1769e8;
}
QMenu#mediaDeviceMenu::item:checked {
    background: #e9f1ff;
    color: #1769e8;
    font-weight: 700;
}
QMenu#mediaDeviceMenu::item:disabled {
    color: #9aa3b2;
}
QMenu#mediaDeviceMenu::separator {
    height: 1px;
    background: #edf0f4;
    margin: 6px 8px;
}
QPushButton#mediaDeviceRefreshButton {
    background: transparent;
    border: none;
    border-radius: 8px;
    color: #263248;
    min-height: 38px;
    padding: 0 14px;
    text-align: left;
}
QPushButton#mediaDeviceRefreshButton:hover {
    background: #f3f6fb;
    color: #1769e8;
}
QPushButton#mediaDeviceRefreshButton:disabled {
    color: #9aa3b2;
}
)"));
    if (darkInterfaceTheme_) {
        menu.setStyleSheet(menu.styleSheet() + QStringLiteral(R"(
QMenu#mediaDeviceMenu {
    background:#151F2E; border-color:#34445B; color:#E4EBF5;
}
QMenu#mediaDeviceMenu::item:selected {
    background:#202D40; color:#9BB4FF;
}
QMenu#mediaDeviceMenu::item:checked {
    background:#263A61; color:#9BB4FF;
}
QMenu#mediaDeviceMenu::item:disabled { color:#69788E; }
QMenu#mediaDeviceMenu::separator { background:#2A394E; }
QPushButton#mediaDeviceRefreshButton { color:#E4EBF5; }
QPushButton#mediaDeviceRefreshButton:hover {
    background:#202D40; color:#9BB4FF;
}
QPushButton#mediaDeviceRefreshButton:disabled { color:#69788E; }
)"));
    }

    const auto categoryForKind =
        [kind](const MediaDeviceSnapshot& media)
            -> const MediaDeviceCategorySnapshot& {
            switch (kind) {
            case MediaDeviceKind::kCamera:
                return media.camera;
            case MediaDeviceKind::kMicrophone:
                return media.microphone;
            case MediaDeviceKind::kSpeaker:
                return media.speaker;
            }
            return media.camera;
        };
    const auto positionMenu = [&menu, anchor] {
        menu.ensurePolished();
        menu.adjustSize();
        const QSize popupSize = menu.sizeHint();
        menu.resize(popupSize);
        const QPoint anchorTop =
            anchor->mapToGlobal(QPoint(0, 0));
        QPoint popupPosition(
            anchorTop.x() +
                (anchor->width() - popupSize.width()) / 2,
            anchorTop.y() - popupSize.height() - 6);
        QScreen* screen =
            QGuiApplication::screenAt(anchorTop);
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
            popupPosition.setY((std::max)(
                available.top(), popupPosition.y()));
        }
        menu.move(popupPosition);
        return popupPosition;
    };

    std::uint64_t refreshRevision = 0;
    QString renderedDeviceSignature;
    QPointer<QPushButton> activeRefreshButton;
    QTimer refreshPoll(&menu);
    refreshPoll.setInterval(80);
    const auto deviceSignature =
        [](const MediaDeviceCategorySnapshot& category) {
            QStringList parts;
            parts << QString::fromStdString(
                         category.preferredDeviceId)
                  << QString::fromStdString(
                         category.activeDeviceId)
                  << QString::fromStdString(
                         category.activeDeviceName)
                  << QString::number(
                         static_cast<int>(category.state));
            for (const auto& device : category.devices) {
                parts << QString::fromStdString(device.id)
                      << QString::fromStdString(device.name)
                      << QString::number(device.available ? 1 : 0);
            }
            return parts.join(QChar(0x001f));
        };
    std::function<void()> rebuildMenu;
    rebuildMenu = [this, &menu, &refreshPoll,
                   &refreshRevision,
                   &renderedDeviceSignature,
                   &activeRefreshButton,
                   &positionMenu, kind,
                   categoryForKind,
                   deviceSignature] {
        menu.setUpdatesEnabled(false);
        menu.clear();
        const auto media =
            engine_->Snapshot().localMediaDevices;
        const auto& category = categoryForKind(media);
        renderedDeviceSignature =
            deviceSignature(category);
        const std::string preferredId =
            category.preferredDeviceId.empty()
                ? std::string(
                      kSystemDefaultMediaDeviceId)
                : category.preferredDeviceId;
        const auto addDeviceAction =
            [this, &menu, kind, &preferredId](
                const QString& name,
                const std::string& deviceId,
                bool available) {
                auto* action = menu.addAction(name);
                action->setCheckable(true);
                action->setChecked(
                    deviceId == preferredId);
                action->setEnabled(available);
                connect(
                    action, &QAction::triggered, this,
                    [this, kind, deviceId] {
                        BeginMediaDeviceSelection(
                            kind,
                            QString::fromStdString(
                                deviceId));
                    });
            };
        addDeviceAction(
            category.activeDeviceName.empty()
                ? QStringLiteral("跟随系统默认")
                : QStringLiteral("跟随系统默认（当前：%1）")
                      .arg(QString::fromStdString(
                          category.activeDeviceName)),
            kSystemDefaultMediaDeviceId,
            !category.devices.empty());
        for (const auto& device : category.devices) {
            addDeviceAction(
                QString::fromStdString(device.name),
                device.id, device.available);
        }
        if (category.devices.empty()) {
            menu.addSeparator();
            auto* unavailable = menu.addAction(
                QStringLiteral("没有检测到可用设备"));
            unavailable->setEnabled(false);
        }
        menu.addSeparator();
        auto* refreshAction =
            new QWidgetAction(&menu);
        auto* refreshButton =
            new QPushButton(
                media.refreshing
                    ? QStringLiteral("正在刷新设备…")
                    : QStringLiteral("刷新设备列表"),
                &menu);
        refreshButton->setObjectName(
            QStringLiteral("mediaDeviceRefreshButton"));
        refreshButton->setCursor(
            Qt::PointingHandCursor);
        refreshButton->setFocusPolicy(Qt::NoFocus);
        refreshButton->setEnabled(!media.refreshing);
        activeRefreshButton = refreshButton;
        refreshAction->setDefaultWidget(refreshButton);
        menu.addAction(refreshAction);
        connect(
            refreshButton, &QPushButton::clicked,
            &menu,
            [this, &menu, refreshButton, &refreshPoll,
             &refreshRevision] {
                menu.setActiveAction(nullptr);
                refreshButton->clearFocus();
                refreshRevision =
                    engine_->Snapshot()
                        .localMediaDevices.revision;
                refreshButton->setText(
                    QStringLiteral("正在刷新设备…"));
                refreshButton->setEnabled(false);
                const auto result =
                    engine_->RefreshLocalMediaDevices();
                if (!result.accepted) {
                    refreshButton->setText(
                        QStringLiteral("刷新设备列表"));
                    refreshButton->setEnabled(true);
                    RemoteCToast::Show(
                        this,
                        QStringLiteral(
                            "刷新音视频设备失败：%1")
                            .arg(QString::fromStdString(
                                result.errorMessage)),
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
         &activeRefreshButton, categoryForKind,
         deviceSignature] {
            const auto media =
                engine_->Snapshot().localMediaDevices;
            if (!media.refreshing &&
                media.revision != refreshRevision) {
                refreshPoll.stop();
                const QString newSignature =
                    deviceSignature(
                        categoryForKind(media));
                if (newSignature ==
                        renderedDeviceSignature &&
                    activeRefreshButton) {
                    activeRefreshButton->setText(
                        QStringLiteral("刷新设备列表"));
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
}

void ControllerMainWindow::BeginMediaDeviceSelection(
    MediaDeviceKind kind, const QString& deviceId)
{
    if (!engine_ || deviceId.isEmpty()) {
        return;
    }
    const auto snapshot = engine_->Snapshot();
    const MediaDeviceCategorySnapshot* category = nullptr;
    QString* pendingId = nullptr;
    QComboBox* selector = nullptr;
    QString label;
    SessionCommandResult (ISessionEngine::*operation)(
        const std::string&) = nullptr;
    switch (kind) {
    case MediaDeviceKind::kCamera:
        category = &snapshot.localMediaDevices.camera;
        pendingId = &pendingCameraDeviceId_;
        selector = cameraDeviceSelector_;
        label = QStringLiteral("摄像头");
        operation = &ISessionEngine::SelectLocalCameraDevice;
        break;
    case MediaDeviceKind::kMicrophone:
        category = &snapshot.localMediaDevices.microphone;
        pendingId = &pendingMicrophoneDeviceId_;
        selector = microphoneDeviceSelector_;
        label = QStringLiteral("麦克风");
        operation = &ISessionEngine::SelectLocalMicrophoneDevice;
        break;
    case MediaDeviceKind::kSpeaker:
        category = &snapshot.localMediaDevices.speaker;
        pendingId = &pendingSpeakerDeviceId_;
        selector = speakerDeviceSelector_;
        label = QStringLiteral("扬声器");
        operation = &ISessionEngine::SelectLocalSpeakerDevice;
        break;
    }
    if (!category || !pendingId || !operation) {
        return;
    }
    if (category->state == MediaDeviceSelectionState::kReady &&
        QString::fromStdString(category->preferredDeviceId) ==
            deviceId) {
        RemoteCToast::ShowAbove(
            selector ? static_cast<QWidget*>(selector) : this,
            QStringLiteral("当前已经在使用该%1").arg(label),
            RemoteCToast::Tone::kInformation);
        return;
    }

    *pendingId = deviceId;
    if (selector) {
        selector->setEnabled(false);
    }
    if (mediaDeviceStatusLabel_) {
        mediaDeviceStatusLabel_->setText(
            QStringLiteral("正在切换%1，请稍候…").arg(label));
    }
    const auto result =
        (engine_.get()->*operation)(deviceId.toStdString());
    if (!result.accepted) {
        pendingId->clear();
        mediaDeviceRevision_ = 0;
        RemoteCToast::Show(
            this,
            QStringLiteral("%1切换失败：%2")
                .arg(label,
                     QString::fromStdString(result.errorMessage)),
            RemoteCToast::Tone::kError);
        UpdateLocalMediaDevicesUi(engine_->Snapshot());
    }
}
