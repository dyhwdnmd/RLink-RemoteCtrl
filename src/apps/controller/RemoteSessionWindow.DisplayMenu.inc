// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

    void RemoteSessionWindow::ShowRemoteDisplayMenu()
    {
        if (!sessionEngine_ || !remoteDisplayButton_) {
            return;
        }
        const auto snapshot = sessionEngine_->Snapshot();
        std::vector<DisplayDescriptor> displays;
        std::string currentDisplayKey;
        bool catalogReported = false;
        if (binding_.IsDirect()) {
            displays = snapshot.directRemoteDisplays;
            currentDisplayKey =
                snapshot.directRemoteDisplay.stableDisplayKey;
            catalogReported =
                snapshot.directRemoteDisplayCatalogReported;
        } else {
            const auto pair = std::find_if(
                snapshot.roomPeerConnections.begin(),
                snapshot.roomPeerConnections.end(),
                [this](const RoomPeerConnectionSnapshot& current) {
                    return current.pairId ==
                           binding_.roomPairId.toStdString();
                });
            if (pair != snapshot.roomPeerConnections.end()) {
                displays = pair->remoteDisplays;
                currentDisplayKey = pair->sharedDisplayStableKey;
                catalogReported = pair->remoteDisplayCatalogReported;
            }
        }
        if (!catalogReported || displays.empty()) {
            RemoteCToast::ShowAbove(
                remoteDisplayButton_,
                QStringLiteral("远端显示器列表尚未就绪"),
                RemoteCToast::Tone::kInformation);
            return;
        }

        ShowSessionToolbar(false);
        mediaDeviceMenuOpen_ = true;
        if (toolbarHideTimer_) {
            toolbarHideTimer_->stop();
        }
        RoundedPopupMenu menu(this, 10.0);
        menu.setObjectName(
            QStringLiteral("remoteDisplayMenu"));
        menu.setStyleSheet(QStringLiteral(R"(
QMenu#remoteDisplayMenu {
    background:#111b2b;
    border:1px solid #34445c;
    border-radius:10px;
    padding:7px;
    color:#dce4ef;
    font-size:13px;
}
QMenu#remoteDisplayMenu::item {
    min-width:260px;
    min-height:24px;
    border-radius:8px;
    padding:8px 36px 8px 14px;
}
QMenu#remoteDisplayMenu::item:selected {
    background:#293650;
    color:#ffffff;
}
QMenu#remoteDisplayMenu::item:checked {
    background:#22345e;
    color:#82a3ff;
    font-weight:700;
}
)"));
        for (std::size_t index = 0;
             index < displays.size(); ++index) {
            const auto& display = displays[index];
            const QString displayName =
                display.friendlyName.empty()
                    ? QStringLiteral("显示器 %1").arg(index + 1)
                    : QString::fromStdString(
                          display.friendlyName);
            const QString label = QStringLiteral(
                "%1  ·  %2 × %3  ·  %4%5")
                .arg(displayName)
                .arg(display.width)
                .arg(display.height)
                .arg(display.scalePercent)
                .arg(display.primary
                         ? QStringLiteral("%（主显示器）")
                         : QStringLiteral("%"));
            QAction* action = menu.addAction(label);
            action->setCheckable(true);
            action->setChecked(
                display.stableDisplayKey ==
                currentDisplayKey);
            action->setData(QString::fromStdString(
                display.stableDisplayKey));
        }
        menu.ensurePolished();
        menu.adjustSize();
        const QSize popupSize = menu.sizeHint();
        const QPoint anchorBottom =
            remoteDisplayButton_->mapToGlobal(
                QPoint(remoteDisplayButton_->width(),
                       remoteDisplayButton_->height()));
        QPoint popupPosition(
            anchorBottom.x() - popupSize.width(),
            anchorBottom.y() + 5);
        if (QScreen* screen =
                QGuiApplication::screenAt(anchorBottom)) {
            const QRect available = screen->availableGeometry();
            popupPosition.setX(std::clamp(
                popupPosition.x(), available.left(),
                (std::max)(
                    available.left(),
                    available.right() -
                        popupSize.width() + 1)));
            popupPosition.setY(std::clamp(
                popupPosition.y(), available.top(),
                (std::max)(
                    available.top(),
                    available.bottom() -
                        popupSize.height() + 1)));
        }
        QAction* selected = menu.exec(popupPosition);
        mediaDeviceMenuOpen_ = false;
        ScheduleSessionToolbarHide();
        if (!selected) {
            return;
        }
        const QString displayKey =
            selected->data().toString();
        if (displayKey.isEmpty() ||
            displayKey.toStdString() == currentDisplayKey) {
            return;
        }
        const auto result = binding_.IsDirect()
            ? sessionEngine_->RequestDirectSharedDisplaySwitch(
                  displayKey.toStdString())
            : sessionEngine_->RequestRemoteSharedDisplaySwitch(
                  binding_.roomPairId.toStdString(),
                  displayKey.toStdString());
        if (!result.accepted) {
            RemoteCToast::ShowAbove(
                remoteDisplayButton_,
                QString::fromStdString(result.errorMessage),
                RemoteCToast::Tone::kError);
            return;
        }
        const auto pendingSnapshot = sessionEngine_->Snapshot();
        if (binding_.IsDirect()) {
            pendingRemoteDisplaySwitchSequence_ =
                pendingSnapshot.directRemoteDisplaySwitchSequence;
        } else {
            const auto pendingPair = std::find_if(
                pendingSnapshot.roomPeerConnections.begin(),
                pendingSnapshot.roomPeerConnections.end(),
                [this](const RoomPeerConnectionSnapshot& current) {
                    return current.pairId ==
                           binding_.roomPairId.toStdString();
                });
            if (pendingPair !=
                pendingSnapshot.roomPeerConnections.end()) {
            pendingRemoteDisplaySwitchSequence_ =
                pendingPair->remoteDisplaySwitchSequence;
            }
        }
        remoteDisplayButton_->setEnabled(false);
        remoteDisplayButton_->setToolTip(
            QStringLiteral("正在切换远端显示器…"));
        RemoteCToast::ShowAbove(
            remoteDisplayButton_,
            QStringLiteral("正在切换远端显示器"),
            RemoteCToast::Tone::kSuccess);
    }
