// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

void ControllerMainWindow::UpdateRoomUi(
    const SessionEngineSnapshot& snapshot)
{
    const auto& room = snapshot.room;
    const bool signalingOnline =
        snapshot.connectivity == SessionConnectivityState::kOnline;
    const bool engineReady =
        snapshot.state == SessionEngineState::kReady;
    const bool legacySessionActive =
        snapshot.state == SessionEngineState::kConnecting ||
        snapshot.state == SessionEngineState::kAwaitingLocalApproval ||
        snapshot.state == SessionEngineState::kActive ||
        snapshot.state == SessionEngineState::kStopping;
    const bool canStartRoom =
        authenticationAvailable_ && signalingOnline && engineReady &&
        (room.membership == RoomMembershipState::kNone ||
         room.membership == RoomMembershipState::kFailed);
    const bool roomActive =
        room.membership == RoomMembershipState::kActive;
    const bool roomLeaving =
        room.membership == RoomMembershipState::kLeaving;
    const bool roomRecovering =
        roomActive &&
        (!signalingOnline || room.errorCode == "room_signaling_recovering");

    createRoomCapacity_->setEnabled(canStartRoom);
    createRoomButton_->setEnabled(canStartRoom);
    roomIdEdit_->setEnabled(canStartRoom);
    joinRoomButton_->setEnabled(canStartRoom);
    createRoomButton_->setText(QStringLiteral("创建房间"));
    joinRoomButton_->setText(QStringLiteral("申请加入  →"));

    switch (room.membership) {
    case RoomMembershipState::kNone:
        if (signalingOnline && legacySessionActive) {
            SetRoomActionHint(QStringLiteral(
                "当前已有远程会话正在进行；请先结束该会话，再创建或加入房间。"));
        } else if (snapshot.state == SessionEngineState::kFailed) {
            SetRoomActionHint(
                snapshot.errorMessage.empty()
                    ? QStringLiteral(
                          "WebRTC 运行时初始化失败；请打开调试信息查看原因。")
                    : QStringLiteral("WebRTC 初始化失败：%1")
                          .arg(QString::fromStdString(
                              snapshot.errorMessage)),
                true);
        } else if (snapshot.state == SessionEngineState::kStarting) {
            SetRoomActionHint(QStringLiteral(
                "WebRTC 运行时正在初始化，请稍候。"));
        } else if (signalingOnline && engineReady) {
            SetRoomActionHint(QStringLiteral(
                "房间控制面已就绪；创建房间或输入房间 ID 申请加入。"));
        }
        break;
    case RoomMembershipState::kCreating:
        createRoomButton_->setText(QStringLiteral("正在创建…"));
        SetRoomActionHint(QStringLiteral(
            "正在创建房间并等待服务端返回权威成员状态。"));
        serviceStatus_->setText(QStringLiteral("● 正在创建房间"));
        break;
    case RoomMembershipState::kJoinPending:
        joinRoomButton_->setText(QStringLiteral("等待房主审批…"));
        SetRoomActionHint(
            QStringLiteral("已申请加入房间 %1，正在等待房主审批。")
                .arg(QString::fromStdString(room.roomId)));
        serviceStatus_->setText(QStringLiteral("● 等待入房审批"));
        break;
    case RoomMembershipState::kActive:
        if (roomRecovering) {
            SetRoomActionHint(
                room.errorMessage.empty()
                    ? QStringLiteral(
                          "房间信令正在恢复，席位仍保留；恢复前不能审批、调容量或离开。")
                    : QString::fromStdString(room.errorMessage));
            serviceStatus_->setText(QStringLiteral("● 房间控制面恢复中"));
        } else if (!room.errorMessage.empty()) {
            SetRoomActionHint(
                QStringLiteral("房间操作失败：%1")
                    .arg(QString::fromStdString(room.errorMessage)),
                true);
            serviceStatus_->setText(QStringLiteral("● 房间控制面在线"));
        } else {
            SetRoomActionHint(QStringLiteral(
                "房间控制面已连接；成员加入后会自动建立 P2P。"));
            serviceStatus_->setText(QStringLiteral("● 房间 P2P 在线"));
        }
        break;
    case RoomMembershipState::kLeaving:
        leaveRoomButton_->setText(QStringLiteral("正在离开…"));
        SetRoomActionHint(QStringLiteral("正在离开房间，请稍候。"));
        serviceStatus_->setText(QStringLiteral("● 正在离开房间"));
        break;
    case RoomMembershipState::kFailed:
        SetRoomActionHint(
            room.errorMessage.empty()
                ? QStringLiteral("房间操作失败，可以重新创建或申请加入。")
                : QStringLiteral("房间操作失败：%1")
                      .arg(QString::fromStdString(room.errorMessage)),
            true);
        serviceStatus_->setText(signalingOnline
                                    ? QStringLiteral("● 可重试房间操作")
                                    : QStringLiteral("● 等待信令恢复"));
        break;

    }

    if (!roomActive && !roomLeaving) {
        activeRoomCapacity_->setEnabled(false);
        applyRoomCapacityButton_->setEnabled(false);
        leaveRoomButton_->setEnabled(false);
        roomScreenShareButton_->setEnabled(false);
        roomCameraButton_->setEnabled(false);
        roomMicrophoneButton_->setEnabled(false);
        roomSpeakerButton_->setEnabled(false);
        roomCameraGalleryButton_->setEnabled(false);
        roomFileTransferButton_->setEnabled(false);
        SetMediaStateButton(roomScreenShareButton_, MediaStateIcon::kScreen,
                            false, QStringLiteral("共享本机屏幕"));
        SetMediaStateButton(roomCameraButton_, MediaStateIcon::kCamera,
                            false, QStringLiteral("开启摄像头"));
        SetMediaStateButton(roomMicrophoneButton_,
                            MediaStateIcon::kMicrophone, false,
                            QStringLiteral("开启麦克风"));
        SetMediaStateButton(roomSpeakerButton_, MediaStateIcon::kSpeaker,
                            true, QStringLiteral("关闭远端声音"));
        SetCameraGalleryStateButton(
            roomCameraGalleryButton_, false, false,
            QStringLiteral("摄像头画廊 · 0 人开启"));
        if (fileTransferWindow_) {
            fileTransferWindow_->SyncPeers({}, frameGeometry());
        }
        leaveRoomButton_->setText(QStringLiteral("离开房间"));
        QueueRoomWorkspaceActive(false);
        return;
    }

    roomIdLabel_->setText(room.roomId.empty()
                              ? QStringLiteral("—")
                              : QString::fromStdString(room.roomId));
    copyRoomIdButton_->setEnabled(!room.roomId.empty());
    const auto onlineMemberCount = std::count_if(
        room.members.begin(), room.members.end(),
        [](const RoomMemberSnapshot& member) { return member.online; });
    roomOccupancyLabel_->setText(
        QStringLiteral("%1 在线 · %2/%3 席位")
            .arg(static_cast<qulonglong>(onlineMemberCount))
            .arg(static_cast<qulonglong>(room.members.size()))
            .arg(room.capacity));
    roomMemberSummaryLabel_->setText(
        QStringLiteral("房间成员  (%1/%2)")
            .arg(static_cast<qulonglong>(room.members.size()))
            .arg(room.capacity));
    roomMemberFooterLabel_->setText(
        QStringLiteral("人数上限：%1 人        %2 在线")
            .arg(room.capacity)
            .arg(static_cast<qulonglong>(onlineMemberCount)));
    roomSeatUsageLabel_->setText(
        QStringLiteral("%1 / %2")
            .arg(static_cast<qulonglong>(room.members.size()))
            .arg(room.capacity));

    const QString owner =
        MemberDisplayName(room, room.ownerDeviceId, snapshot.localDeviceId);
    roomOwnerLabel_->setText(owner.isEmpty() ? QStringLiteral("未知") : owner);
    const QString screenSharer = MemberDisplayName(
        room, room.screenSharerDeviceId, snapshot.localDeviceId);
    const QString pendingScreenSharer = MemberDisplayName(
        room, room.pendingScreenSharerDeviceId, snapshot.localDeviceId);
    const QString displayedScreenSharer = pendingScreenSharer.isEmpty()
        ? screenSharer
        : pendingScreenSharer;
    roomScreenSharerLabel_->setText(
        displayedScreenSharer.isEmpty() ? QStringLiteral("无人共享")
                                        : displayedScreenSharer);
    const QString controller = MemberDisplayName(
        room, room.activeControllerDeviceId, snapshot.localDeviceId);
    const QString pendingController = MemberDisplayName(
        room, room.pendingControllerDeviceId, snapshot.localDeviceId);
    roomControllerLabel_->setText(
        !controller.isEmpty()
            ? controller
            : (!pendingController.isEmpty()
                   ? QStringLiteral("%1（申请中）").arg(pendingController)
                   : QStringLiteral("无人控制")));

    const std::size_t expectedPeerCount = static_cast<std::size_t>(
        std::count_if(room.members.begin(), room.members.end(),
                      [&snapshot](const RoomMemberSnapshot& member) {
                          return member.deviceId != snapshot.localDeviceId;
                      }));
    const std::size_t activePairCount = static_cast<std::size_t>(
        std::count_if(
            snapshot.roomPeerConnections.begin(),
            snapshot.roomPeerConnections.end(),
            [](const RoomPeerConnectionSnapshot& pair) {
                return pair.state == RoomPeerConnectionState::kActive;
            }));
    const std::size_t failedPairCount = static_cast<std::size_t>(
        std::count_if(
            snapshot.roomPeerConnections.begin(),
            snapshot.roomPeerConnections.end(),
            [](const RoomPeerConnectionSnapshot& pair) {
                return pair.state == RoomPeerConnectionState::kFailed;
            }));
    const std::size_t recoveringPairCount = static_cast<std::size_t>(
        std::count_if(
            snapshot.roomPeerConnections.begin(),
            snapshot.roomPeerConnections.end(),
            [](const RoomPeerConnectionSnapshot& pair) {
                return pair.state == RoomPeerConnectionState::kDisconnected ||
                       pair.state == RoomPeerConnectionState::kRecovering;
            }));
    roomPeerConnectivityLabel_->setText(
        expectedPeerCount == 0
            ? QStringLiteral("等待成员加入")
            : (failedPairCount > 0
                ? QStringLiteral("%1 条 P2P 恢复失败")
                      .arg(static_cast<qulonglong>(failedPairCount))
            : (recoveringPairCount > 0
                ? QStringLiteral("%1 条 P2P 正在恢复")
                      .arg(static_cast<qulonglong>(recoveringPairCount))
            : QStringLiteral("%1 / %2 已连接")
                  .arg(static_cast<qulonglong>(activePairCount))
                  .arg(static_cast<qulonglong>(expectedPeerCount)))));

    {
        const QSignalBlocker blocker(activeRoomCapacity_);
        const int capacityIndex = activeRoomCapacity_->findData(
            QVariant::fromValue(room.capacity));
        if (capacityIndex >= 0) {
            activeRoomCapacity_->setCurrentIndex(capacityIndex);
        }
    }

    QStringList memberRenderKeyParts;
    memberRenderKeyParts.reserve(
        8 + static_cast<int>(room.members.size()) * 8);
    const auto appendKeyString = [&memberRenderKeyParts](
                                     const std::string& value) {
        const QString text = QString::fromStdString(value);
        memberRenderKeyParts.push_back(
            QStringLiteral("%1:%2").arg(text.size()).arg(text));
    };
    appendKeyString(room.roomId);
    appendKeyString(room.ownerDeviceId);
    appendKeyString(room.screenSharerDeviceId);
    appendKeyString(room.pendingScreenSharerDeviceId);
    appendKeyString(room.activeControllerDeviceId);
    appendKeyString(room.pendingControllerDeviceId);
    appendKeyString(snapshot.localDeviceId);
    memberRenderKeyParts.push_back(
        darkInterfaceTheme_ ? QStringLiteral("dark")
                            : QStringLiteral("light"));
    for (const auto& member : room.members) {
        appendKeyString(member.deviceId);
        appendKeyString(member.deviceName);
        memberRenderKeyParts.push_back(QString::number(member.online));
        memberRenderKeyParts.push_back(
            QString::number(member.cameraPublishing));
        memberRenderKeyParts.push_back(
            QString::number(member.microphonePublishing));
        const auto pair = std::find_if(
            snapshot.roomPeerConnections.begin(),
            snapshot.roomPeerConnections.end(),
            [&member](const RoomPeerConnectionSnapshot& candidate) {
                return candidate.peerDeviceId == member.deviceId;
            });
        memberRenderKeyParts.push_back(QString::number(
            pair == snapshot.roomPeerConnections.end()
                ? -1
                : static_cast<int>(pair->state)));
        memberRenderKeyParts.push_back(QString::number(
            pair == snapshot.roomPeerConnections.end()
                ? 0u
                : pair->iceRestartAttempt));
    }
    const QString memberRenderKey =
        memberRenderKeyParts.join(QChar(0x1f));
    if (memberRenderKey != renderedRoomMemberKey_) {
        renderedRoomMemberKey_ = memberRenderKey;
        QHash<QString, bool> previousMemberMediaStates;
        const auto previousIndicators =
            roomMemberList_->findChildren<QToolButton*>();
        for (auto* indicator : previousIndicators) {
            const QString stateKey = indicator->property(
                "roomMemberMediaStateKey").toString();
            if (!stateKey.isEmpty()) {
                previousMemberMediaStates.insert(
                    stateKey,
                    indicator->property("roomMemberMediaActive").toBool());
            }
        }
        const int previousMemberCount =
            roomMemberList_->property("remoteCMemberCount").toInt();
        roomMemberList_->clear();
        for (const auto& member : room.members) {
        QString name = member.deviceName.empty()
                           ? QString::fromStdString(member.deviceId)
                           : QString::fromStdString(member.deviceName);
        const bool isLocal = member.deviceId == snapshot.localDeviceId;
        QStringList roles;
        if (member.deviceId == room.ownerDeviceId) {
            roles.push_back(QStringLiteral("房主"));
        }
        if (member.deviceId == room.activeControllerDeviceId) {
            roles.push_back(QStringLiteral("控制者"));
        }
        if (member.deviceId == room.pendingControllerDeviceId) {
            roles.push_back(QStringLiteral("申请控制中"));
        }

        QString connectionStatus = isLocal
            ? QStringLiteral("[本机]")
            : QStringLiteral("P2P 等待建立");
        if (!isLocal) {
            const auto pair = std::find_if(
                snapshot.roomPeerConnections.begin(),
                snapshot.roomPeerConnections.end(),
                [&member](const RoomPeerConnectionSnapshot& candidate) {
                    return candidate.peerDeviceId == member.deviceId;
                });
            if (pair == snapshot.roomPeerConnections.end()) {
                connectionStatus = member.online
                    ? QStringLiteral("P2P 等待建立")
                    : QStringLiteral("P2P 未建立");
            } else {
                switch (pair->state) {
                case RoomPeerConnectionState::kActive:
                    connectionStatus = QStringLiteral("P2P 已连接");
                    break;
                case RoomPeerConnectionState::kDisconnected:
                    connectionStatus = QStringLiteral("P2P 网络波动");
                    break;
                case RoomPeerConnectionState::kRecovering:
                    connectionStatus = QStringLiteral("P2P 恢复中 %1/3")
                        .arg(pair->iceRestartAttempt);
                    break;
                case RoomPeerConnectionState::kFailed:
                    connectionStatus = QStringLiteral("P2P 失败");
                    break;
                case RoomPeerConnectionState::kClosed:
                    connectionStatus = QStringLiteral("P2P 已关闭");
                    break;
                default:
                    connectionStatus = QStringLiteral("P2P 连接中");
                    break;
                }
            }
        }

        auto* item = new QListWidgetItem(roomMemberList_);
        item->setData(
            Qt::UserRole, QString::fromStdString(member.deviceId));
        item->setData(Qt::UserRole + 1, name);
        item->setSizeHint(QSize(0, 78));
        auto* memberRow = new QWidget(roomMemberList_);
        auto* memberRowLayout = new QVBoxLayout(memberRow);
        memberRowLayout->setContentsMargins(8, 6, 8, 5);
        memberRowLayout->setSpacing(3);

        auto* identityRow = new QHBoxLayout();
        identityRow->setSpacing(7);
        auto* avatar = new QLabel(
            name.trimmed().left(1).toUpper(), memberRow);
        avatar->setFixedSize(30, 30);
        avatar->setAlignment(Qt::AlignCenter);
        avatar->setStyleSheet(QStringLiteral(
            "background:#147df5;color:white;border-radius:15px;"
            "font-weight:700;"));
        identityRow->addWidget(avatar);

        auto* memberName = new QLabel(name, memberRow);
        memberName->setStyleSheet(QStringLiteral(
            "color:%1;font-weight:700;")
            .arg(darkInterfaceTheme_ ? QStringLiteral("#E4EBF5")
                                     : QStringLiteral("#1b263b")));
        memberName->setSizePolicy(
            QSizePolicy::Ignored, QSizePolicy::Fixed);
        identityRow->addWidget(memberName, 1, Qt::AlignVCenter);
        if (!roles.isEmpty()) {
            auto* roleLabel = new QLabel(roles.join(QStringLiteral(" · ")),
                                         memberRow);
            roleLabel->setAlignment(Qt::AlignCenter);
            roleLabel->setSizePolicy(
                QSizePolicy::Maximum, QSizePolicy::Fixed);
            roleLabel->setFixedHeight(24);
            roleLabel->setStyleSheet(QStringLiteral(
                "background:%1;color:%2;border-radius:7px;"
                "padding:0px 7px;font-size:11px;font-weight:650;")
                .arg(darkInterfaceTheme_ ? QStringLiteral("#263A61")
                                         : QStringLiteral("#e9f2ff"),
                     darkInterfaceTheme_ ? QStringLiteral("#9BB4FF")
                                         : QStringLiteral("#0b7cff")));
            roleLabel->setToolTip(roles.join(QStringLiteral("、")));
            identityRow->addWidget(roleLabel, 0, Qt::AlignVCenter);
        }

        auto* onlinePill = new QLabel(
            member.online ? QStringLiteral("在线") : QStringLiteral("离线"),
            memberRow);
        onlinePill->setAlignment(Qt::AlignCenter);
        onlinePill->setSizePolicy(
            QSizePolicy::Maximum, QSizePolicy::Fixed);
        onlinePill->setFixedHeight(24);
        onlinePill->setStyleSheet(
            member.online
                ? QStringLiteral(
                      "background:%1;color:%2;border-radius:7px;"
                      "padding:0px 8px;font-size:11px;font-weight:650;")
                      .arg(darkInterfaceTheme_ ? QStringLiteral("#162336")
                                               : QStringLiteral("#e8f8f0"),
                           darkInterfaceTheme_ ? QStringLiteral("#4FF0B5")
                                               : QStringLiteral("#15945c"))
                : QStringLiteral(
                      "background:%1;color:%2;border-radius:7px;"
                      "padding:0px 8px;font-size:11px;font-weight:650;")
                      .arg(darkInterfaceTheme_ ? QStringLiteral("#202A38")
                                               : QStringLiteral("#f0f2f5"),
                           darkInterfaceTheme_ ? QStringLiteral("#8F9DB1")
                                               : QStringLiteral("#8b95a5")));
        identityRow->addWidget(onlinePill, 0, Qt::AlignVCenter);
        memberRowLayout->addLayout(identityRow);

        auto* stateRow = new QHBoxLayout();
        stateRow->setContentsMargins(0, 0, 0, 0);
        stateRow->setSpacing(4);
        const QString memberDetail =
            QString::fromStdString(member.deviceId);
        auto* connectionLabel = new QLabel(memberDetail, memberRow);
        connectionLabel->setProperty("muted", true);
        connectionLabel->setAlignment(Qt::AlignCenter);
        connectionLabel->setSizePolicy(
            QSizePolicy::Ignored, QSizePolicy::Fixed);
        connectionLabel->setToolTip(memberDetail);
        stateRow->addWidget(connectionLabel, 1);

        const bool cameraActive =
            member.online && member.cameraPublishing;
        const bool screenActive =
            member.online &&
            (member.deviceId == room.screenSharerDeviceId ||
             member.deviceId == room.pendingScreenSharerDeviceId);
        const bool microphoneActive =
            member.online && member.microphonePublishing;
        const auto createMemberIndicator =
            [memberRow, &member, &previousMemberMediaStates](
                MediaStateIcon type, bool active,
                const QString& toolTip) {
                const QString stateKey = QStringLiteral("%1:%2")
                    .arg(QString::fromStdString(member.deviceId))
                    .arg(static_cast<int>(type));
                const bool hadPreviousState =
                    previousMemberMediaStates.contains(stateKey);
                auto* indicator = CreateMemberMediaIndicator(
                    memberRow, type, active, toolTip, hadPreviousState,
                    previousMemberMediaStates.value(stateKey));
                indicator->setProperty(
                    "roomMemberMediaStateKey", stateKey);
                indicator->setProperty(
                    "roomMemberMediaActive", active);
                return indicator;
            };
        stateRow->addWidget(createMemberIndicator(
            MediaStateIcon::kCamera, cameraActive,
            cameraActive ? QStringLiteral("摄像头已开启")
                         : QStringLiteral("摄像头未开启")));
        stateRow->addWidget(createMemberIndicator(
            MediaStateIcon::kScreen, screenActive,
            screenActive ? QStringLiteral("正在共享屏幕")
                         : QStringLiteral("未共享屏幕")));
        const bool canMuteRemoteMicrophone =
            !isLocal && microphoneActive &&
            room.ownerDeviceId == snapshot.localDeviceId &&
            std::any_of(
                snapshot.roomPeerConnections.begin(),
                snapshot.roomPeerConnections.end(),
                [&member](const RoomPeerConnectionSnapshot& current) {
                    return current.peerDeviceId == member.deviceId &&
                           current.state ==
                               RoomPeerConnectionState::kActive;
                });
        if (canMuteRemoteMicrophone) {
            auto* microphoneAction = new QToolButton(memberRow);
            microphoneAction->setFixedSize(28, 28);
            microphoneAction->setAutoRaise(true);
            microphoneAction->setCursor(Qt::PointingHandCursor);
            microphoneAction->setIcon(CreateMediaStateIcon(
                MediaStateIcon::kMicrophone, true));
            microphoneAction->setIconSize(QSize(21, 21));
            microphoneAction->setToolTip(
                QStringLiteral("点击关闭对方麦克风"));
            microphoneAction->setAccessibleName(
                QStringLiteral("关闭 %1 的麦克风").arg(name));
            microphoneAction->setStyleSheet(QStringLiteral(
                "QToolButton{background:transparent;border:none;"
                "border-radius:7px;padding:0;}"
                "QToolButton:hover{background:%1;}"
                "QToolButton:pressed{background:%2;}")
                .arg(darkInterfaceTheme_ ? QStringLiteral("#3A2027")
                                         : QStringLiteral("#ffe9eb"),
                     darkInterfaceTheme_ ? QStringLiteral("#4A2730")
                                         : QStringLiteral("#ffd8dc")));
            const QString targetDeviceId =
                QString::fromStdString(member.deviceId);
            const QString microphoneStateKey = QStringLiteral("%1:%2")
                .arg(targetDeviceId)
                .arg(static_cast<int>(MediaStateIcon::kMicrophone));
            microphoneAction->setProperty(
                "roomMemberMediaStateKey", microphoneStateKey);
            microphoneAction->setProperty(
                "roomMemberMediaActive", microphoneActive);
            const bool hadPreviousMicrophoneState =
                previousMemberMediaStates.contains(microphoneStateKey);
            const bool dark = ui::RemoteCTheme::IsDark(
                ui::RemoteCTheme::LoadPreference());
            auto* microphoneMorph =
                remotec::ui::morph::MorphIconButtonBinding::attach(
                    microphoneAction,
                    QStringLiteral(":/ui/icons/lucide/base/mic.svg"),
                    QStringLiteral(":/ui/icons/lucide/base/mic-off.svg"),
                    remotec::ui::morph::MorphIconButtonBinding::
                        Interaction::State,
                    QSize(21, 21),
                    QColor(dark ? QStringLiteral("#4FF0B5")
                                : QStringLiteral("#168A5B")),
                    QColor(dark ? QStringLiteral("#7F8DA3")
                                : QStringLiteral("#667085")));
            if (microphoneMorph) {
                const bool previousMicrophoneActive =
                    previousMemberMediaStates.value(microphoneStateKey);
                microphoneMorph->setTarget(
                    !(hadPreviousMicrophoneState
                          ? previousMicrophoneActive
                          : microphoneActive),
                    false);
                if (hadPreviousMicrophoneState &&
                    previousMicrophoneActive != microphoneActive) {
                    QTimer::singleShot(
                        0, microphoneAction,
                        [microphoneMorph, microphoneActive] {
                            microphoneMorph->setTarget(
                                !microphoneActive, true);
                        });
                }
            }
            connect(
                microphoneAction, &QToolButton::clicked, this,
                [this, microphoneAction, targetDeviceId, name] {
                    const auto result =
                        engine_->RequestRoomMemberMicrophoneMute(
                            targetDeviceId.toStdString());
                    if (!result.accepted) {
                        RemoteCToast::ShowAbove(
                            microphoneAction,
                            QString::fromStdString(
                                result.errorMessage),
                            RemoteCToast::Tone::kError);
                        return;
                    }
                    RemoteCToast::ShowAbove(
                        microphoneAction,
                        QStringLiteral(
                            "正在关闭 %1 的麦克风").arg(name),
                        RemoteCToast::Tone::kSuccess);
                });
            stateRow->addWidget(microphoneAction);
        } else {
            stateRow->addWidget(createMemberIndicator(
                MediaStateIcon::kMicrophone,
                microphoneActive,
                microphoneActive
                    ? (isLocal
                           ? QStringLiteral("本机麦克风已开启")
                           : QStringLiteral(
                                 "对方麦克风已开启，仅房主可以关闭"))
                    : QStringLiteral("麦克风未开启")));
        }
        memberRowLayout->addLayout(stateRow);
        const QString fullIdentity = QStringLiteral("%1 · %2 · %3")
            .arg(name,
                 QString::fromStdString(member.deviceId),
                 connectionStatus);
        memberName->setToolTip(fullIdentity);
            roomMemberList_->setItemWidget(item, memberRow);
        }
        if (room.members.empty()) {
            auto* item = new QListWidgetItem(
                QStringLiteral("正在同步房间成员状态…"), roomMemberList_);
            item->setForeground(QColor(QStringLiteral("#929cab")));
        }
        roomMemberList_->setProperty(
            "remoteCMemberCount", static_cast<int>(room.members.size()));
        if (previousMemberCount != static_cast<int>(room.members.size())) {
            AnimateSmallUiChange(roomMemberList_);
        }
    }

    const bool localOwner =
        room.ownerDeviceId == snapshot.localDeviceId;
    const bool ownerCanEdit =
        roomActive && localOwner && signalingOnline && !roomRecovering;
    activeRoomCapacity_->setEnabled(ownerCanEdit);
    applyRoomCapacityButton_->setEnabled(false);
    activeRoomCapacity_->setToolTip(
        localOwner ? QStringLiteral("房主可以在 2～5 人之间调整上限")
                   : QStringLiteral("只有房主可以调整人数上限"));
    leaveRoomButton_->setEnabled(
        roomActive && signalingOnline && !roomRecovering);
    leaveRoomButton_->setText(roomLeaving ? QStringLiteral("正在离开…")
                                          : QStringLiteral("离开房间"));

    const bool localOwnsScreen =
        room.screenSharerDeviceId == snapshot.localDeviceId ||
        room.pendingScreenSharerDeviceId == snapshot.localDeviceId;
    const bool screenShareTakeoverPending =
        !snapshot.outgoingRoomScreenShareSwitchRequestId.empty();
    SetMediaStateButton(
        roomScreenShareButton_, MediaStateIcon::kScreen, localOwnsScreen,
        screenShareTakeoverPending
            ? QStringLiteral("取消接替主机器申请")
            : (localOwnsScreen ? QStringLiteral("停止共享本机屏幕")
                               : QStringLiteral("共享本机屏幕")));
    if (localOwnsScreen &&
        snapshot.activeSharedDisplay.sessionDisplayId != 0) {
        const QString displayName =
            snapshot.activeSharedDisplay.friendlyName.empty()
                ? QStringLiteral("当前显示器")
                : QString::fromStdString(
                      snapshot.activeSharedDisplay.friendlyName);
        roomScreenShareButton_->setToolTip(
            QStringLiteral("正在共享：%1 · %2 × %3\n点击停止共享")
                .arg(displayName)
                .arg(snapshot.activeSharedDisplay.width)
                .arg(snapshot.activeSharedDisplay.height));
    }
    roomScreenShareButton_->setEnabled(
        roomActive && signalingOnline && !roomRecovering &&
        (localOwnsScreen ||
         room.screenShareState != RoomScreenShareState::kRecovering));

    const bool canChangeLocalMedia =
        roomActive && signalingOnline && !roomRecovering;
    const auto activeDeviceName =
        [](const MediaDeviceCategorySnapshot& category) {
            if (!category.activeDeviceName.empty()) {
                return QString::fromStdString(
                    category.activeDeviceName);
            }
            const std::string deviceId =
                category.activeDeviceId.empty()
                    ? category.preferredDeviceId
                    : category.activeDeviceId;
            if (deviceId.empty() ||
                deviceId == kSystemDefaultMediaDeviceId) {
                return QStringLiteral("系统默认设备");
            }
            const auto device = std::find_if(
                category.devices.begin(), category.devices.end(),
                [&deviceId](const MediaDeviceDescriptor& candidate) {
                    return candidate.id == deviceId;
                });
            return device == category.devices.end()
                ? QStringLiteral("设备不可用")
                : QString::fromStdString(device->name);
        };
    const QString cameraDeviceName =
        activeDeviceName(snapshot.localMediaDevices.camera);
    const QString microphoneDeviceName =
        activeDeviceName(snapshot.localMediaDevices.microphone);
    const QString speakerDeviceName =
        activeDeviceName(snapshot.localMediaDevices.speaker);
    QString cameraToolTip;
    switch (snapshot.localCamera) {
    case LocalCameraState::kStarting:
        cameraToolTip = QStringLiteral("摄像头启动中…");
        break;
    case LocalCameraState::kPublishing:
        cameraToolTip = QStringLiteral("关闭摄像头");
        break;
    case LocalCameraState::kStopping:
        cameraToolTip = QStringLiteral("摄像头关闭中…");
        break;
    case LocalCameraState::kFailed:
        cameraToolTip = QStringLiteral("重试摄像头");
        break;
    default:
        cameraToolTip = QStringLiteral("开启摄像头");
        break;
    }
    cameraToolTip += snapshot.localCamera ==
            LocalCameraState::kPublishing
        ? QStringLiteral("\n正在使用：%1").arg(cameraDeviceName)
        : QStringLiteral("\n开启后使用：%1").arg(cameraDeviceName);
    SetMediaStateButton(
        roomCameraButton_, MediaStateIcon::kCamera,
        snapshot.localCamera == LocalCameraState::kPublishing,
        cameraToolTip);
    roomCameraButton_->setEnabled(
        canChangeLocalMedia &&
        snapshot.localCamera != LocalCameraState::kStarting &&
        snapshot.localCamera != LocalCameraState::kStopping);

    QString microphoneToolTip;
    switch (snapshot.localMicrophone) {
    case LocalMicrophoneState::kStarting:
        microphoneToolTip = QStringLiteral("麦克风启动中…");
        break;
    case LocalMicrophoneState::kPublishing:
        microphoneToolTip = QStringLiteral("关闭麦克风");
        break;
    case LocalMicrophoneState::kStopping:
        microphoneToolTip = QStringLiteral("麦克风关闭中…");
        break;
    case LocalMicrophoneState::kFailed:
        microphoneToolTip = QStringLiteral("重试麦克风");
        break;
    default:
        microphoneToolTip = QStringLiteral("开启麦克风");
        break;
    }
    microphoneToolTip += snapshot.localMicrophone ==
            LocalMicrophoneState::kPublishing
        ? QStringLiteral("\n正在使用：%1").arg(microphoneDeviceName)
        : QStringLiteral("\n开启后使用：%1")
              .arg(microphoneDeviceName);
    SetMediaStateButton(
        roomMicrophoneButton_, MediaStateIcon::kMicrophone,
        snapshot.localMicrophone == LocalMicrophoneState::kPublishing,
        microphoneToolTip);
    roomMicrophoneButton_->setEnabled(
        canChangeLocalMedia &&
        snapshot.localMicrophone != LocalMicrophoneState::kStarting &&
        snapshot.localMicrophone != LocalMicrophoneState::kStopping);
    SetMediaStateButton(
        roomSpeakerButton_, MediaStateIcon::kSpeaker,
        !snapshot.roomAudioPlaybackMuted,
        snapshot.roomAudioPlaybackMuted
            ? QStringLiteral("开启远端声音\n恢复后输出到：%1")
                  .arg(speakerDeviceName)
            : QStringLiteral("关闭远端声音\n正在输出到：%1")
                  .arg(speakerDeviceName));
    roomSpeakerButton_->setEnabled(roomActive);

    const auto publishedCameraCount = std::count_if(
        room.members.begin(), room.members.end(),
        [](const RoomMemberSnapshot& member) {
            return member.online && member.cameraPublishing;
        });
    const bool camerasAvailable = publishedCameraCount > 0;
    const bool galleryVisible = camerasAvailable && roomCameraWindow_ &&
        roomCameraWindow_->isVisible() && !cameraGalleryManuallyHidden_;
    SetCameraGalleryStateButton(
        roomCameraGalleryButton_, camerasAvailable, galleryVisible,
        QStringLiteral("%1摄像头画廊 · %2 人开启")
            .arg(galleryVisible ? QStringLiteral("关闭")
                                : QStringLiteral("打开"))
            .arg(static_cast<qulonglong>(publishedCameraCount)));
    roomCameraGalleryButton_->setEnabled(
        roomActive && (publishedCameraCount > 0 ||
                       snapshot.localCamera == LocalCameraState::kStarting));

    std::vector<FileTransferPeer> filePeers;
    std::vector<std::string> availableFilePeerIds;
    std::vector<std::string> recoveringFilePeerIds;
    for (const auto& member : room.members) {
        if (!member.online || member.deviceId == snapshot.localDeviceId) {
            continue;
        }
        const auto pair = std::find_if(
            snapshot.roomPeerConnections.begin(),
            snapshot.roomPeerConnections.end(),
            [&member](const RoomPeerConnectionSnapshot& candidate) {
                return candidate.peerDeviceId == member.deviceId;
            });
        if (pair == snapshot.roomPeerConnections.end()) {
            continue;
        }
        const bool fileChannelReady =
            pair->state == RoomPeerConnectionState::kActive &&
            pair->openDataChannelCount >= 4;
        const bool pairRecovering =
            pair->state == RoomPeerConnectionState::kStarting ||
            pair->state == RoomPeerConnectionState::kNegotiating ||
            pair->state == RoomPeerConnectionState::kConnecting ||
            pair->state == RoomPeerConnectionState::kDisconnected ||
            pair->state == RoomPeerConnectionState::kRecovering ||
            (pair->state == RoomPeerConnectionState::kActive &&
             pair->openDataChannelCount < 4);
        if (pairRecovering) {
            recoveringFilePeerIds.push_back(member.deviceId);
        }
        if (!fileChannelReady) {
            continue;
        }
        FileTransferPeer peer;
        peer.deviceId = member.deviceId;
        peer.displayName = member.deviceName.empty()
                               ? QString::fromStdString(member.deviceId)
                               : QString::fromStdString(member.deviceName);
        filePeers.push_back(std::move(peer));
        availableFilePeerIds.push_back(member.deviceId);
    }
    if (fileTransferController_) {
        fileTransferController_->UpdatePeerConnectivity(
            availableFilePeerIds, recoveringFilePeerIds);
    }
    roomFileTransferButton_->setEnabled(!filePeers.empty());
    if (fileTransferWindow_) {
        fileTransferWindow_->SyncPeers(filePeers, frameGeometry());
    }

    if (roomRecovering) {
        roomStageLabel_->setText(QStringLiteral(
            "房间控制面正在恢复；已建立的 P2P 暂时保留，不发起新连接。"));
    } else if (roomLeaving) {
        roomStageLabel_->setText(QStringLiteral("正在关闭本机房间状态。"));
    } else if (
        !snapshot.outgoingRoomScreenShareSwitchRequestId.empty()) {
        roomStageLabel_->setText(QStringLiteral(
            "已申请接替主机器，正在等待当前分享者确认；原画面和控制保持不变。"));
    } else if (room.screenShareState ==
               RoomScreenShareState::kSwitching) {
        roomStageLabel_->setText(QStringLiteral(
            "正在切换主机器；旧控制授权已撤销，等待新分享端画面就绪。"));
    } else if (room.screenShareState ==
               RoomScreenShareState::kRecovering) {
        roomStageLabel_->setText(QStringLiteral(
            "主机器信令正在恢复；保留现有分享和控制租约，暂不允许切换。"));
    } else if (expectedPeerCount == 0) {
        roomStageLabel_->setText(QStringLiteral(
            "等待其他成员加入；加入后将自动建立成员对 P2P。"));
    } else if (failedPairCount > 0) {
        roomStageLabel_->setText(
            QStringLiteral("有 %1 条 P2P 连接失败，请检查网络和 ICE 状态。")
                .arg(static_cast<qulonglong>(failedPairCount)));
    } else if (recoveringPairCount > 0) {
        roomStageLabel_->setText(
            QStringLiteral("网络已中断，正在恢复 %1 条成员对 P2P，请稍候。")
                .arg(static_cast<qulonglong>(recoveringPairCount)));
    } else if (activePairCount == expectedPeerCount) {
        roomStageLabel_->setText(
            QStringLiteral("本机到其他 %1 个成员的 P2P 已全部连接。")
                .arg(static_cast<qulonglong>(expectedPeerCount)));
    } else {
        roomStageLabel_->setText(QStringLiteral(
            "正在建立成员对 P2P：%1 / %2 已连接。")
                .arg(static_cast<qulonglong>(activePairCount))
                .arg(static_cast<qulonglong>(expectedPeerCount)));
    }
    QueueRoomWorkspaceActive(true);
}
