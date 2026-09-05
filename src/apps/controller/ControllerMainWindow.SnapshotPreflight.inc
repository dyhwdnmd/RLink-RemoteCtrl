// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

void ControllerMainWindow::OnSessionEngineSnapshot(
    const SessionEngineSnapshot& snapshot)
{
    if (QThread::currentThread() != thread()) {
        QMetaObject::invokeMethod(
            this, [this, snapshot] { OnSessionEngineSnapshot(snapshot); },
            Qt::QueuedConnection);
        return;
    }

    UpdateLocalMediaDevicesUi(snapshot);
    UpdateClipboardSession(snapshot);

    // During the signaling "switching" phase the future sharer's P2P control
    // channel already exists, while capture has not started yet. Send the
    // viewer's persisted final policy now so the sharer creates its first
    // track/encoder at the requested FPS and resolution instead of starting
    // at 30 FPS and being retuned after the monitor window opens.
    const bool canPreflightRemoteScreen =
        snapshot.room.membership == RoomMembershipState::kActive &&
        snapshot.room.screenShareState ==
            RoomScreenShareState::kSwitching &&
        snapshot.room.screenShareEpoch != 0 &&
        !snapshot.room.pendingScreenSharerDeviceId.empty() &&
        snapshot.room.pendingScreenSharerDeviceId !=
            snapshot.localDeviceId;
    if (canPreflightRemoteScreen &&
        preflightScreenPreferenceAttemptEpoch_ !=
            snapshot.room.screenShareEpoch) {
        preflightScreenPreferenceAttemptEpoch_ =
            snapshot.room.screenShareEpoch;
        preflightScreenPreferenceAttempts_ = 0;
    }
    if (canPreflightRemoteScreen &&
        preflightScreenPreferenceEpoch_ !=
            snapshot.room.screenShareEpoch &&
        preflightScreenPreferenceAttempts_ < 5 &&
        !preflightScreenPreferenceScheduled_) {
        const auto pair = std::find_if(
            snapshot.roomPeerConnections.begin(),
            snapshot.roomPeerConnections.end(),
            [&snapshot](const RoomPeerConnectionSnapshot& current) {
                return current.peerDeviceId ==
                    snapshot.room.pendingScreenSharerDeviceId;
            });
        if (pair != snapshot.roomPeerConnections.end()) {
            preflightScreenPreferenceScheduled_ = true;
            ++preflightScreenPreferenceAttempts_;
            const std::string pairId = pair->pairId;
            const std::uint64_t expectedEpoch =
                snapshot.room.screenShareEpoch;
            QTimer::singleShot(0, this, [this, pairId, expectedEpoch] {
                if (!engine_) {
                    preflightScreenPreferenceScheduled_ = false;
                    return;
                }
                const auto current = engine_->Snapshot();
                if (current.room.screenShareState !=
                        RoomScreenShareState::kSwitching ||
                    current.room.screenShareEpoch != expectedEpoch ||
                    current.room.pendingScreenSharerDeviceId.empty() ||
                    current.room.pendingScreenSharerDeviceId ==
                        current.localDeviceId) {
                    preflightScreenPreferenceScheduled_ = false;
                    return;
                }
                ScreenStreamPreferenceRequest request;
                request.framesPerSecond = kDefaultScreenFrameRate;
                const QSettings settings;
                const int qualityValue = settings.value(
                    QString::fromLatin1(
                        kRemoteScreenQualitySetting),
                    static_cast<int>(
                        ScreenQualityTier::kOriginal)).toInt();
                request.quality =
                    qualityValue >= static_cast<int>(
                        ScreenQualityTier::kAutomatic) &&
                    qualityValue <= static_cast<int>(
                        ScreenQualityTier::kOriginal)
                    ? static_cast<ScreenQualityTier>(qualityValue)
                    : ScreenQualityTier::kOriginal;
                const auto [width, height] =
                    SavedScreenQualityBounds(request.quality);
                request.maxWidth = width;
                request.maxHeight = height;
                const auto result =
                    engine_->SetRoomScreenStreamPreference(
                        pairId, request);
                if (result.accepted) {
                    preflightScreenPreferenceEpoch_ = expectedEpoch;
                }
                preflightScreenPreferenceScheduled_ = false;
                if (!result.accepted &&
                    preflightScreenPreferenceAttempts_ < 5) {
                    QTimer::singleShot(
                        50, this, [this, expectedEpoch] {
                            if (!engine_) {
                                return;
                            }
                            const auto retry = engine_->Snapshot();
                            if (retry.room.screenShareState ==
                                    RoomScreenShareState::kSwitching &&
                                retry.room.screenShareEpoch ==
                                    expectedEpoch) {
                                OnSessionEngineSnapshot(retry);
                            }
                        });
                }
            });
        }
    } else if (!canPreflightRemoteScreen &&
               snapshot.room.screenShareState !=
                   RoomScreenShareState::kActive) {
        preflightScreenPreferenceEpoch_ = 0;
        preflightScreenPreferenceAttemptEpoch_ = 0;
        preflightScreenPreferenceAttempts_ = 0;
    }

    if (inputExecutor_) {
        const bool localIsRoomSharedDisplayOwner =
            snapshot.room.screenSharerDeviceId ==
                snapshot.localDeviceId &&
            snapshot.activeSharedDisplay.sessionDisplayId != 0 &&
            snapshot.activeSharedDisplayLayoutVersion != 0;
        const bool localIsDirectControlledDisplayOwner =
            snapshot.state == SessionEngineState::kActive &&
            snapshot.purpose == SessionPurpose::kRemoteControl &&
            snapshot.remoteControlRole == RemoteControlRole::kControlled &&
            snapshot.activeSharedDisplay.sessionDisplayId != 0 &&
            snapshot.activeSharedDisplayLayoutVersion != 0;
        if (localIsRoomSharedDisplayOwner ||
            localIsDirectControlledDisplayOwner) {
            inputExecutor_->SetActiveDisplay(
                snapshot.localDisplayTopology,
                snapshot.activeSharedDisplay.stableDisplayKey);
        } else {
            inputExecutor_->ClearActiveDisplay();
        }
    }

    const QString roomErrorCode =
        QString::fromStdString(snapshot.room.errorCode);
    const bool joinRejected =
        roomErrorCode == QStringLiteral("rejected_by_owner") ||
        roomErrorCode == QStringLiteral("room_join_rejected");
    const bool controlRejected =
        roomErrorCode == QStringLiteral("rejected_by_screen_sharer") ||
        roomErrorCode == QStringLiteral("room_control_rejected");
    const bool screenShareSwitchFailed =
        roomErrorCode ==
            QStringLiteral("screen_share_switch_rejected_by_sharer") ||
        roomErrorCode == QStringLiteral("screen_share_switch_timeout") ||
        roomErrorCode ==
            QStringLiteral("screen_share_switch_sharer_offline");
    const bool screenShareStartFailed =
        roomErrorCode == QStringLiteral("display_not_available") ||
        roomErrorCode ==
            QStringLiteral("desktop_capture_start_failed") ||
        roomErrorCode ==
            QStringLiteral("desktop_capture_frame_rate_rejected") ||
        roomErrorCode ==
            QStringLiteral("screen_encoding_policy_failed") ||
        roomErrorCode ==
            QStringLiteral("screen_sender_activate_failed");
    if (joinRejected || controlRejected || screenShareSwitchFailed ||
        screenShareStartFailed) {
        const QString decisionKey =
            QString::fromStdString(snapshot.room.roomId) +
            QLatin1Char('|') + roomErrorCode;
        if (lastRoomDecisionAlertKey_ != decisionKey) {
            lastRoomDecisionAlertKey_ = decisionKey;
            const QString title =
                joinRejected
                    ? QStringLiteral("加入房间被拒绝")
                    : (controlRejected
                           ? QStringLiteral("控制申请被拒绝")
                           : (screenShareStartFailed
                                  ? QStringLiteral("屏幕共享启动失败")
                                  : QStringLiteral("接替主机器未完成")));
            const QString message =
                joinRejected
                    ? QStringLiteral("对方已拒绝你的加入房间申请。")
                    : (controlRejected
                           ? QStringLiteral(
                                 "对方已拒绝你的远程控制申请，你仍可继续观看共享画面。")
                           : (screenShareStartFailed
                                  ? (snapshot.room.errorMessage.empty()
                                         ? QStringLiteral(
                                               "当前显示器无法启动采集，请检查调试信息中的采集错误。")
                                         : QString::fromStdString(
                                               snapshot.room.errorMessage))
                                  : (roomErrorCode ==
                                             QStringLiteral(
                                                 "screen_share_switch_rejected_by_sharer")
                                         ? QStringLiteral(
                                               "当前主机器拒绝了你的接替申请，原共享画面不受影响。")
                                         : QStringLiteral(
                                               "接替申请已超时或当前主机器暂时离线，原共享状态未被改变。"))));
            const QPointer<QWidget> alertParent =
                controlRejected && remoteSessionWindow_
                    ? static_cast<QWidget*>(remoteSessionWindow_.data())
                    : static_cast<QWidget*>(this);
            QTimer::singleShot(0, this,
                               [this, alertParent, title, message] {
                RemoteCDialog::Alert(
                    alertParent ? alertParent.data() : this,
                    title, message, QStringLiteral("知道了"),
                    RemoteCDialog::Tone::kDanger, true);
            });
        }
    } else {
        lastRoomDecisionAlertKey_.clear();
    }
