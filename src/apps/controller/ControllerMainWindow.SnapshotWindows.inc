// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

    RememberRecentRoom(snapshot);
    RememberRecentDevice(snapshot);
    UpdateRoomUi(snapshot);
    UpdateDirectFileTransferSession(snapshot);
    UpdateVideoPipelineSettingsAvailability(snapshot);

    // UpdateRoomUi may replace the general signaling text with the final
    // room-specific state. Compare and animate only that final value; doing
    // this earlier made the sidebar status briefly animate an intermediate
    // string on every media-state snapshot.
    SetBusyStatusAnimation(connectivityPill_, connectivityBusy);
    SetBusyStatusAnimation(serviceStatus_, connectivityBusy);
    if (!connectivityBusy &&
        previousConnectivityText != connectivityPill_->text()) {
        AnimateSmallUiChange(connectivityPill_);
    }
    if (!connectivityBusy && previousServiceText != serviceStatus_->text()) {
        AnimateSmallUiChange(serviceStatus_);
    }

    if (snapshot.localCamera == LocalCameraState::kOff ||
        snapshot.localCamera == LocalCameraState::kFailed) {
        localCameraStopRequested_ = false;
    }
    const bool roomActiveForCamera =
        snapshot.room.membership == RoomMembershipState::kActive;
    const bool remoteCameraPublished = roomActiveForCamera && std::any_of(
        snapshot.room.members.begin(), snapshot.room.members.end(),
        [&snapshot](const RoomMemberSnapshot& member) {
            return member.deviceId != snapshot.localDeviceId &&
                member.online && member.cameraPublishing;
        });
    const bool localCameraPublished = roomActiveForCamera &&
        !localCameraStopRequested_ &&
        (snapshot.localCamera == LocalCameraState::kStarting ||
         snapshot.localCamera == LocalCameraState::kPublishing);
    const bool anyPublishedCamera =
        remoteCameraPublished || localCameraPublished;
    auto* roomEngine = dynamic_cast<app::InProcessSessionEngine*>(
        engine_.get());
    if (anyPublishedCamera && roomEngine) {
        if (!roomCameraWindow_) {
            roomCameraWindow_ = new RoomCameraWindow(roomEngine, nullptr);
            roomCameraWindow_->SetHiddenByUserCallback([this] {
                cameraGalleryManuallyHidden_ = true;
                if (engine_) UpdateRoomUi(engine_->Snapshot());
            });
        }
        roomCameraWindow_->SyncSnapshot(snapshot);
        const bool autoOpenCameraGallery = QSettings().value(
            QString::fromLatin1(kAutoOpenCameraGallerySetting),
            true).toBool();
        const bool mayOpenCameraGallery =
            localCameraPublished || autoOpenCameraGallery;
        if (mayOpenCameraGallery && !cameraGalleryManuallyHidden_ &&
            !roomCameraWindow_->isVisible()) {
            roomCameraWindow_->OpenBesideMainWindow(frameGeometry());
        }
    } else if (roomCameraWindow_) {
        roomCameraWindow_->SyncSnapshot(snapshot);
        roomCameraWindow_->hide();
        cameraGalleryManuallyHidden_ = false;
    }

    const bool roomMembershipActive =
        snapshot.room.membership == RoomMembershipState::kActive;
    const bool remoteRoomShareActive =
        roomMembershipActive &&
        snapshot.room.screenShareState == RoomScreenShareState::kActive &&
        !snapshot.room.screenSharerDeviceId.empty() &&
        snapshot.room.screenSharerDeviceId != snapshot.localDeviceId;
    if (remoteRoomShareActive) {
        const QString activeSharer = QString::fromStdString(
            snapshot.room.screenSharerDeviceId);
        const bool manuallyDismissed =
            dismissedRemoteScreenSharerDeviceId_ == activeSharer &&
            dismissedRemoteScreenShareEpoch_ ==
                snapshot.room.screenShareEpoch;
        if (!manuallyDismissed &&
            dismissedRemoteScreenShareEpoch_ != 0) {
            dismissedRemoteScreenSharerDeviceId_.clear();
            dismissedRemoteScreenShareEpoch_ = 0;
        }
        const auto pair = std::find_if(
            snapshot.roomPeerConnections.begin(),
            snapshot.roomPeerConnections.end(),
            [&snapshot](const RoomPeerConnectionSnapshot& candidate) {
                return candidate.peerDeviceId ==
                           snapshot.room.screenSharerDeviceId &&
                       candidate.state == RoomPeerConnectionState::kActive;
            });
        if (pair != snapshot.roomPeerConnections.end() &&
            !manuallyDismissed) {
            const auto member = std::find_if(
                snapshot.room.members.begin(), snapshot.room.members.end(),
                [&snapshot](const RoomMemberSnapshot& candidate) {
                    return candidate.deviceId ==
                           snapshot.room.screenSharerDeviceId;
                });
            const QString deviceId = QString::fromStdString(
                snapshot.room.screenSharerDeviceId);
            const QString deviceName =
                member != snapshot.room.members.end() &&
                        !member->deviceName.empty()
                    ? QString::fromStdString(member->deviceName)
                    : deviceId;
            OpenRemoteSession(RemoteSessionBinding::Room(
                deviceId, deviceName,
                QString::fromStdString(pair->pairId)));
        }
    } else {
        const bool localRoomShareActive =
            roomMembershipActive &&
            snapshot.room.screenShareState ==
                RoomScreenShareState::kActive &&
            snapshot.room.screenSharerDeviceId == snapshot.localDeviceId;
        const bool roomShareDefinitivelyEnded =
            !roomMembershipActive ||
            snapshot.room.screenShareState ==
                RoomScreenShareState::kIdle ||
            localRoomShareActive;
        if (remoteSessionBinding_ && remoteSessionBinding_->IsRoom() &&
            roomShareDefinitivelyEnded) {
            if (remoteSessionWindow_) {
                remoteSessionWindow_->close();
            }
            remoteSessionBinding_.reset();
            dismissedRemoteScreenSharerDeviceId_.clear();
            dismissedRemoteScreenShareEpoch_ = 0;
        } else if (snapshot.room.screenShareState ==
                       RoomScreenShareState::kIdle) {
            dismissedRemoteScreenSharerDeviceId_.clear();
            dismissedRemoteScreenShareEpoch_ = 0;
        }
    }

    // The direct 1V1 path remains during migration so existing tests and the
    // independent media windows continue to compile and behave as before.
    if (snapshot.state == SessionEngineState::kConnecting) {
        if (assistedSessionPending_) {
            connectButton_->setEnabled(
                !assistedSessionCancellationPending_ &&
                !assistedSessionTimedOut_);
            connectButton_->setText(
                assistedSessionCancellationPending_
                    ? (assistedSessionTimedOut_
                           ? QStringLiteral("连接已超时")
                           : QStringLiteral("正在取消…"))
                    : QStringLiteral("正在连接…（点击取消）"));
            if (!assistedSessionCancellationPending_) {
                connectionHint_->setProperty("error", false);
                connectionHint_->setText(QStringLiteral(
                    "正在校验验证码并建立远程桌面…"));
                connectionHint_->setStyleSheet(
                    QStringLiteral("color:#a66b12;"));
            }
        } else {
            connectButton_->setEnabled(false);
            connectButton_->setText(QStringLiteral("正在建立会话…"));
        }
    } else if (snapshot.state == SessionEngineState::kActive &&
               (assistedSessionPending_ || assistedSessionActive_)) {
        connectButton_->setEnabled(false);
        connectButton_->setText(QStringLiteral("远程桌面已连接"));
    } else {
        connectButton_->setText(QStringLiteral("发起连接"));
        connectButton_->setEnabled(
            authenticationAvailable_ &&
            snapshot.connectivity == SessionConnectivityState::kOnline &&
            snapshot.state == SessionEngineState::kReady &&
            IsNineDigitPublicId(deviceIdEdit_->text().trimmed()) &&
            verificationCodeEdit_->text().trimmed().size() == 6 &&
            deviceIdEdit_->text().trimmed().toStdString() !=
                snapshot.localDeviceId);
    }

    const QString directSessionError =
        QString::fromStdString(snapshot.errorCode);
    const bool directRequestWasPending =
        assistedSessionPending_ || ownedDeviceSessionPending_;
    const bool initialPeerConnectionFailed =
        directSessionError == QStringLiteral("negotiation_timeout") ||
        directSessionError == QStringLiteral("peer_connection_create_failed") ||
        directSessionError == QStringLiteral("p2p_connection_failed");
    if (snapshot.state == SessionEngineState::kConnecting &&
        directSessionError.isEmpty()) {
        // The controlled side does not go through StartAssistedSession() or
        // StartOwnedDeviceSession(), so reset the one-shot alert guard when a
        // fresh incoming direct session starts as well.
        lastDirectSessionToastError_.clear();
    }
    QString directSessionErrorText;
    if (directSessionError == QStringLiteral("verification_code_invalid")) {
        directSessionErrorText =
            QStringLiteral("验证码错误，请向对方核对当前显示的 6 位一次性验证码。");
    } else if (directSessionError == QStringLiteral("rate_limited")) {
        directSessionErrorText =
            QStringLiteral("尝试过于频繁，请稍后再试。");
    } else if (directSessionError == QStringLiteral("target_offline")) {
        directSessionErrorText =
            QStringLiteral("对方设备不在线，请确认 RLink 已运行且信令已连接。");
    } else if (directSessionError == QStringLiteral("device_busy")) {
        directSessionErrorText =
            QStringLiteral("对方设备已在协助房间或其他远程会话中，请结束后再试。");
    } else if (directSessionError ==
               QStringLiteral("session_request_timeout")) {
        directSessionErrorText =
            QStringLiteral("连接请求超时，请确认对方在线并使用最新验证码后重试。");
    } else if (directSessionError ==
                   QStringLiteral("invalid_session_request") ||
               directSessionError ==
                   QStringLiteral("invalid_session_permissions")) {
        directSessionErrorText =
            QStringLiteral("连接请求无效，请刷新验证码后重新发起连接。");
    } else if (directSessionError == QStringLiteral("session_rejected")) {
        directSessionErrorText =
            QStringLiteral("对方设备未接受本次连接，请重新获取验证码后再试。");
    } else if (initialPeerConnectionFailed) {
        directSessionErrorText =
            QStringLiteral("设备间的 P2P 连接未能建立，请更换网络后重试。");
    }
    if (assistedSessionPending_ &&
        snapshot.state == SessionEngineState::kReady) {
        assistedSessionTimeoutTimer_->stop();
        if (assistedSessionTimedOut_) {
            directSessionErrorText = QStringLiteral(
                "连接请求超时，请确认对方在线并使用最新验证码后重试。");
        } else if (assistedSessionCancellationPending_) {
            connectionHint_->setProperty("error", false);
            connectionHint_->setText(
                QStringLiteral("连接已取消，可以重新发起。"));
            connectionHint_->setStyleSheet({});
        } else if (directSessionErrorText.isEmpty() &&
                   !directSessionError.isEmpty()) {
            directSessionErrorText = QStringLiteral(
                "远程桌面未能建立（错误代码：%1），请稍后重试。")
                .arg(directSessionError);
        } else if (directSessionErrorText.isEmpty()) {
            connectionHint_->setProperty("error", false);
            connectionHint_->setText(QStringLiteral(
                "本次连接未建立，可以检查设备 ID 和验证码后重试。"));
            connectionHint_->setStyleSheet({});
        }
        assistedSessionPending_ = false;
        assistedSessionCancellationPending_ = false;
        assistedSessionTimedOut_ = false;
    } else if (assistedSessionPending_ &&
               snapshot.state == SessionEngineState::kActive) {
        assistedSessionTimeoutTimer_->stop();
        assistedSessionPending_ = false;
        assistedSessionActive_ = true;
        assistedSessionCancellationPending_ = false;
        assistedSessionTimedOut_ = false;
        connectionHint_->setProperty("error", false);
        connectionHint_->setText(QStringLiteral(
            "远程桌面已建立，断开后可以重新发起连接。"));
        connectionHint_->setStyleSheet(QStringLiteral("color:#15945f;"));
    } else if (assistedSessionActive_ &&
               snapshot.state == SessionEngineState::kReady) {
        assistedSessionActive_ = false;
        connectionHint_->setProperty("error", false);
        connectionHint_->setText(QStringLiteral(
            "远程桌面已断开，可以使用对方当前验证码重新连接。"));
        connectionHint_->setStyleSheet({});
    }
    if (!directSessionErrorText.isEmpty() &&
        snapshot.state == SessionEngineState::kReady) {
        connectionHint_->setProperty("error", true);
        connectionHint_->setText(directSessionErrorText);
        connectionHint_->setStyleSheet(QStringLiteral("color:#d14343;"));

        if ((directRequestWasPending || initialPeerConnectionFailed) &&
            lastDirectSessionToastError_ != directSessionError) {
            lastDirectSessionToastError_ = directSessionError;
            if (initialPeerConnectionFailed) {
                // Let this snapshot finish closing the failed monitor window
                // before entering a modal dialog. Otherwise the old session
                // window can remain above the warning until it is dismissed.
                QTimer::singleShot(0, this, [this] {
                    RemoteCDialog::Alert(
                        this,
                        QStringLiteral("P2P 连接失败"),
                        QStringLiteral(
                            "未能建立设备之间的 P2P 连接，请更换本机或对方设备所使用的网络后再试。"
                            "<br><br><b>提示：使用手机热点连接成功率更高哦。</b>"),
                        QStringLiteral("知道了"),
                        RemoteCDialog::Tone::kWarning,
                        true);
                });
            } else {
                RemoteCToast::Show(
                    this, directSessionErrorText,
                    RemoteCToast::Tone::kError);
            }
        }
    }

    if (ownedDeviceSessionPending_) {
        if (snapshot.state == SessionEngineState::kReady) {
            if (directSessionErrorText.isEmpty()) {
                const QString message = QStringLiteral(
                    "未能连接到该设备，请确认对方在线且未处于其他会话中。");
                RemoteCToast::Show(
                    this, message, RemoteCToast::Tone::kError);
            }
            ownedDeviceSessionPending_ = false;
        } else if (snapshot.directSignalingSessionReady ||
                   snapshot.state == SessionEngineState::kActive) {
            ownedDeviceSessionPending_ = false;
        }
    }

    if (snapshot.state == SessionEngineState::kAwaitingLocalApproval &&
        !snapshot.sessionId.empty()) {
        const QString sessionId = QString::fromStdString(snapshot.sessionId);
        if (promptedSessionId_ != sessionId) {
            promptedSessionId_ = sessionId;
            const QString peer = QString::fromStdString(snapshot.peerDeviceId);
            const QString purpose =
                snapshot.purpose == SessionPurpose::kRemoteControl
                    ? QStringLiteral("远程控制")
                    : QStringLiteral("摄像头画面");
            const bool accepted = RemoteCDialog::Confirm(
                this, QStringLiteral("收到会话请求"),
                QStringLiteral("设备 %1 请求建立%2会话，是否允许？")
                    .arg(peer, purpose),
                QStringLiteral("允许"), QStringLiteral("拒绝"),
                RemoteCDialog::Tone::kQuestion);
            if (accepted) {
                engine_->AcceptIncomingSession(snapshot.sessionId);
            } else {
                engine_->RejectIncomingSession(snapshot.sessionId);
            }
        }
    } else if (snapshot.state == SessionEngineState::kReady) {
        promptedSessionId_.clear();
        dismissedDirectSessionId_.clear();
    }

    if (!assistedSessionCancellationPending_ &&
        !assistedSessionTimedOut_ &&
        snapshot.state == SessionEngineState::kConnecting &&
        (snapshot.origin == SessionOrigin::kRemoteAssistance ||
         snapshot.origin == SessionOrigin::kOwnedDevice) &&
        snapshot.remoteControlRole == RemoteControlRole::kController &&
        snapshot.directSignalingSessionReady) {
        const QString peer = QString::fromStdString(snapshot.peerDeviceId);
        dismissedDirectSessionId_.clear();
        OpenRemoteSession(RemoteSessionBinding::Direct(
            peer,
            pendingDeviceName_.isEmpty()
                ? QStringLiteral("远程协助设备")
                : pendingDeviceName_,
            SessionOrigin::kRemoteAssistance));
    }

    if (snapshot.state == SessionEngineState::kActive) {
        const QString peer = QString::fromStdString(snapshot.peerDeviceId);
        if (snapshot.purpose == SessionPurpose::kRemoteControl &&
            snapshot.remoteControlRole == RemoteControlRole::kController &&
            QString::fromStdString(snapshot.sessionId) !=
                dismissedDirectSessionId_) {
            OpenRemoteSession(RemoteSessionBinding::Direct(
                peer,
                pendingDeviceName_.isEmpty()
                    ? QStringLiteral("远程设备")
                    : pendingDeviceName_,
                snapshot.origin));
        } else if (snapshot.purpose == SessionPurpose::kCameraOnly) {
            OpenCameraWindow(peer, pendingDeviceName_.isEmpty()
                                       ? QStringLiteral("远程设备")
                                       : pendingDeviceName_);
        }
    }

    if (remoteSessionBinding_ && remoteSessionBinding_->IsDirect() &&
        snapshot.state != SessionEngineState::kActive &&
        snapshot.state != SessionEngineState::kConnecting &&
        !IsDirectRecoveryFailureCode(snapshot.errorCode) &&
        remoteSessionWindow_) {
        remoteSessionWindow_->close();
    }

    if (remoteSessionWindow_) {
        const std::size_t onlineMemberCount =
            static_cast<std::size_t>(std::count_if(
                snapshot.room.members.begin(),
                snapshot.room.members.end(),
                [](const RoomMemberSnapshot& member) {
                    return member.online;
                }));
        remoteSessionWindow_->SetRoomOnlineMemberCount(
            onlineMemberCount);
        remoteSessionWindow_->RefreshControlState();
    }

    QueueRoomJoinApproval(snapshot);
    QueueRoomScreenShareSwitchApproval(snapshot);
    QueueRoomControlApproval(snapshot);
    QueueRoomScreenShareViewApproval(snapshot);
    HandleRoomMemberActionResults(snapshot);
}
