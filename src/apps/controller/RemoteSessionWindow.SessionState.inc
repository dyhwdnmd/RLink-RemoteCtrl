// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

    void RemoteSessionWindow::RefreshControlState()
    {
        bool enabled = false;
        std::uint32_t remoteDisplayId = 0;
        std::uint64_t remoteDisplayLayoutVersion = 0;
        bool showRecoveryPrompt = false;
        bool waitingForSignaling = false;
        bool recoveryFailed = false;
        std::uint32_t recoveryAttempt = 0;
        bool showControlAction = false;
        bool controlLeaseAssigned = false;
        bool controlActionEnabled = false;
        QString controlActionText = QStringLiteral("申请控制");
        QString controlActionToolTip;
        bool remoteDisplaySwitchEnabled = false;
        QString remoteDisplaySwitchToolTip =
            QStringLiteral("等待远端上报显示器列表");
        QString completedRemoteDisplaySwitchMessage;
        bool completedRemoteDisplaySwitchSucceeded = false;
        QString connectionStageText =
            QStringLiteral("正在建立远程会话");
        if (sessionEngine_ && binding_.IsDirect()) {
            const auto snapshot = sessionEngine_->Snapshot();
            CompleteMediaDeviceSelections(snapshot.localMediaDevices);
            const bool directActive =
                snapshot.state == SessionEngineState::kActive &&
                snapshot.purpose == SessionPurpose::kRemoteControl &&
                snapshot.remoteControlRole ==
                    RemoteControlRole::kController;
            remoteDisplayId =
                snapshot.directRemoteDisplay.sessionDisplayId;
            remoteDisplayLayoutVersion =
                snapshot.directRemoteDisplayLayoutVersion;
            enabled = directActive &&
                snapshot.directMediaSlotsPrepared &&
                snapshot.directControlReliableChannelOpen &&
                snapshot.directInputFastChannelOpen &&
                remoteDisplayId != 0 &&
                remoteDisplayLayoutVersion != 0;
            remoteSourceWidth_ = snapshot.directRemoteDisplay.width;
            remoteSourceHeight_ = snapshot.directRemoteDisplay.height;
            if (directActive || snapshot.directSessionEverActive) {
                pairWasActive_ = true;
            }
            const bool establishedDirectSession =
                pairWasActive_ || snapshot.directSessionEverActive;
            const bool directSignalingRecovering =
                establishedDirectSession &&
                (snapshot.connectivity != SessionConnectivityState::kOnline ||
                 snapshot.errorCode == "signaling_session_recovering" ||
                 snapshot.errorCode == "peer_signaling_recovering" ||
                 snapshot.errorCode == "websocket_error" ||
                 snapshot.errorCode == "heartbeat_timeout");
            const bool directPeerRecovering =
                establishedDirectSession &&
                snapshot.state == SessionEngineState::kConnecting;
            recoveryFailed =
                establishedDirectSession &&
                IsDirectRecoveryFailureCode(snapshot.errorCode);
            showRecoveryPrompt = directSignalingRecovering ||
                                 directPeerRecovering || recoveryFailed;
            waitingForSignaling = directSignalingRecovering;
            recoveryAttempt = snapshot.directIceRestartAttempt;
            if (showRecoveryPrompt) {
                enabled = false;
            }
            inputControlEnabled_ = enabled;
            if (!directActive) {
                connectionStageText =
                    snapshot.origin ==
                            SessionOrigin::kRemoteAssistance
                        ? (snapshot.directSignalingSessionReady
                               ? QStringLiteral(
                                     "验证码已通过，正在建立 WebRTC 连接")
                               : QStringLiteral(
                                     "正在校验验证码并连接设备"))
                        : QStringLiteral("正在建立 WebRTC 连接");
            } else if (!snapshot.directMediaSlotsPrepared) {
                connectionStageText =
                    QStringLiteral("正在准备视频通道");
            } else if (remoteDisplayId == 0 ||
                       remoteDisplayLayoutVersion == 0) {
                connectionStageText =
                    QStringLiteral("正在同步显示信息");
            } else {
                connectionStageText =
                    QStringLiteral("正在等待远端画面");
            }
            if (connectionStatusLabel_) {
                if (recoveryFailed) {
                    connectionStatusLabel_->setText(
                        QStringLiteral("连接恢复失败"));
                } else if (showRecoveryPrompt) {
                    connectionStatusLabel_->setText(
                        waitingForSignaling
                            ? QStringLiteral("等待信令恢复")
                            : (recoveryAttempt == 0
                                   ? QStringLiteral("网络波动，等待恢复")
                                   : QStringLiteral("正在恢复 %1/3")
                                         .arg(recoveryAttempt)));
                } else if (!directActive) {
                    connectionStatusLabel_->setText(
                        QStringLiteral("正在连接"));
                } else if (!snapshot.directControlReliableChannelOpen ||
                           !snapshot.directInputFastChannelOpen) {
                    connectionStatusLabel_->setText(
                        QStringLiteral("等待控制通道"));
                } else if (remoteDisplayId == 0 ||
                           remoteDisplayLayoutVersion == 0) {
                    connectionStatusLabel_->setText(
                        QStringLiteral("正在同步显示信息"));
                }
            }
            if (qualityButton_) {
                const auto [requestedWidth, requestedHeight] =
                    ScreenQualityBounds(selectedQuality_);
                const bool showRequested =
                    snapshot.directScreenPreferencePending &&
                    requestedWidth > 0 && requestedHeight > 0;
                const int shownWidth = showRequested
                    ? static_cast<int>(requestedWidth)
                    : (snapshot.directScreenWidth > 0
                        ? static_cast<int>(snapshot.directScreenWidth)
                        : static_cast<int>(remoteSourceWidth_));
                const int shownHeight = showRequested
                    ? static_cast<int>(requestedHeight)
                    : (snapshot.directScreenHeight > 0
                        ? static_cast<int>(snapshot.directScreenHeight)
                        : static_cast<int>(remoteSourceHeight_));
                qualityButton_->setText(
                    shownWidth > 0 && shownHeight > 0
                        ? QStringLiteral("分辨率\n%1 × %2")
                              .arg(shownWidth)
                              .arg(shownHeight)
                        : QStringLiteral("分辨率\n等待画面"));
                qualityButton_->setEnabled(
                    directActive &&
                    snapshot.directControlReliableChannelOpen &&
                    !snapshot.directScreenPreferencePending);
                qualityButton_->setToolTip(
                    snapshot.directScreenPreferencePending
                        ? QStringLiteral("正在请求远端调整输出画质")
                        : QStringLiteral("调整当前直连会话的远端输出分辨率"));
            }
            const std::uint32_t directMaximumFrameRate = std::clamp(
                snapshot.directScreenMaximumFrameRate,
                kMinimumScreenFrameRate,
                kMaximumScreenFrameRate);
            if (reportedRemoteMaximumFrameRate_ !=
                directMaximumFrameRate) {
                reportedRemoteMaximumFrameRate_ = directMaximumFrameRate;
                RebuildFrameRateMenu();
            }
            if (!snapshot.directScreenPreferencePending &&
                snapshot.directScreenFramesPerSecond > 0 &&
                selectedFrameRate_ !=
                    snapshot.directScreenFramesPerSecond) {
                selectedFrameRate_ =
                    snapshot.directScreenFramesPerSecond;
                if (desktopCanvas_) {
                    static_cast<RemoteDesktopCanvas*>(desktopCanvas_)
                        ->SetTargetFrameRate(selectedFrameRate_);
                }
            }
            if (frameRateButton_) {
                frameRateButton_->setEnabled(
                    directActive &&
                    snapshot.directControlReliableChannelOpen &&
                    !snapshot.directScreenPreferencePending);
                frameRateButton_->setText(
                    QStringLiteral("目标帧率\n%1 FPS")
                        .arg(selectedFrameRate_));
                frameRateButton_->setToolTip(
                    snapshot.directScreenPreferencePending
                        ? QStringLiteral("正在请求远端调整帧率")
                        : QStringLiteral("当前共享端最高支持 %1 FPS")
                              .arg(directMaximumFrameRate));
            }
            if (desktopCanvas_) {
                auto* canvas =
                    static_cast<RemoteDesktopCanvas*>(desktopCanvas_);
                canvas->SetRemoteDisplayIdentity(
                    remoteDisplayId, remoteDisplayLayoutVersion);
                canvas->SetConnectionStage(connectionStageText);
                remoteInputDispatcher_->SetEnabled(enabled);
                canvas->SetControlEnabled(enabled);
            } else {
                remoteInputDispatcher_->SetEnabled(enabled);
            }
            if (controlBadge_) {
                controlBadge_->hide();
            }
            if (controlButton_) {
                controlButton_->hide();
            }
            if (speakerButton_) {
                const QString speakerName = ActiveMediaDeviceText(
                    snapshot.localMediaDevices.speaker);
                speakerButton_->SetIcon(QString());
                speakerButton_->SetText(QStringLiteral("声音"));
                speakerButton_->SetSlashVisible(
                    snapshot.roomAudioPlaybackMuted);
                speakerButton_->setToolTip(
                    snapshot.roomAudioPlaybackMuted
                        ? QStringLiteral("开启远端声音\n恢复后输出到：%1")
                              .arg(speakerName)
                        : QStringLiteral("关闭远端声音\n正在输出到：%1")
                              .arg(speakerName));
            }
            if (microphoneButton_) {
                const QString microphoneName = ActiveMediaDeviceText(
                    snapshot.localMediaDevices.microphone);
                const bool microphoneEnabled =
                    snapshot.localMicrophone ==
                        LocalMicrophoneState::kPublishing ||
                    snapshot.localMicrophone ==
                        LocalMicrophoneState::kStarting;
                microphoneButton_->SetIcon(QString());
                microphoneButton_->SetSlashVisible(!microphoneEnabled);
                microphoneButton_->SetText(QStringLiteral("麦克风"));
                microphoneButton_->setToolTip(
                    microphoneEnabled
                        ? QStringLiteral("关闭本机麦克风\n正在使用：%1")
                              .arg(microphoneName)
                        : QStringLiteral("开启本机麦克风\n开启后使用：%1")
                              .arg(microphoneName));
            }
            if (remoteDisplayButton_) {
                const bool maySwitch = directActive &&
                    snapshot.directRemoteDisplayCatalogReported &&
                    snapshot.directRemoteDisplays.size() > 1 &&
                    !snapshot.directRemoteDisplaySwitchPending;
                remoteDisplayButton_->setEnabled(maySwitch);
                if (snapshot.directRemoteDisplaySwitchPending) {
                    remoteDisplayButton_->setToolTip(
                        QStringLiteral("正在切换远端显示器…"));
                } else if (!snapshot.directRemoteDisplayCatalogReported) {
                    remoteDisplayButton_->setToolTip(
                        QStringLiteral("等待远端上报显示器列表"));
                } else if (snapshot.directRemoteDisplays.size() <= 1) {
                    remoteDisplayButton_->setToolTip(
                        QStringLiteral("远端当前只有一个可用显示器"));
                } else {
                    remoteDisplayButton_->setToolTip(
                        QStringLiteral("切换远端正在共享的显示器"));
                }
                if (pendingRemoteDisplaySwitchSequence_ != 0 &&
                    snapshot.directRemoteDisplaySwitchSequence ==
                        pendingRemoteDisplaySwitchSequence_ &&
                    !snapshot.directRemoteDisplaySwitchPending) {
                    const bool succeeded =
                        snapshot.directRemoteDisplaySwitchError.empty();
                    RemoteCToast::ShowAbove(
                        remoteDisplayButton_,
                        succeeded
                            ? QStringLiteral("远端显示器已切换")
                            : QStringLiteral("切换显示器失败：%1")
                                  .arg(QString::fromStdString(
                                      snapshot.directRemoteDisplaySwitchError)),
                        succeeded ? RemoteCToast::Tone::kSuccess
                                  : RemoteCToast::Tone::kError);
                    pendingRemoteDisplaySwitchSequence_ = 0;
                }
            }
            UpdateNetworkRecoveryPrompt(
                showRecoveryPrompt,
                recoveryAttempt,
                waitingForSignaling,
                recoveryFailed);
            return;
        }
        if (sessionEngine_ && binding_.IsRoom()) {
            const auto snapshot = sessionEngine_->Snapshot();
            CompleteMediaDeviceSelections(
                snapshot.localMediaDevices);
            showControlAction =
                snapshot.room.membership == RoomMembershipState::kActive &&
                snapshot.room.screenShareState ==
                RoomScreenShareState::kActive &&
                !snapshot.room.screenSharerDeviceId.empty() &&
                snapshot.room.screenSharerDeviceId != snapshot.localDeviceId;
            controlLeaseAssigned = showControlAction &&
                snapshot.room.activeControllerDeviceId ==
                    snapshot.localDeviceId;
            enabled = controlLeaseAssigned &&
                snapshot.roomControlGrantActive;

            const bool signalingReady =
                snapshot.connectivity == SessionConnectivityState::kOnline &&
                snapshot.room.errorCode != "room_signaling_recovering";
            if (enabled) {
                controlActionText = QStringLiteral("释放控制");
                controlActionEnabled = signalingReady;
                controlActionToolTip = QStringLiteral(
                    "停止向这台机器发送鼠标和键盘输入");
            }
            else if (snapshot.room.pendingControllerDeviceId ==
                snapshot.localDeviceId) {
                controlActionText = QStringLiteral("等待对方批准…");
                controlActionToolTip = QStringLiteral(
                    "屏幕分享者批准后即可控制");
            }
            else if (!snapshot.room.activeControllerDeviceId.empty()) {
                controlActionText = QStringLiteral("其他成员正在控制");
                controlActionToolTip = QStringLiteral(
                    "同一时间只能有一名控制者");
            }
            else if (!snapshot.room.pendingControllerDeviceId.empty()) {
                controlActionText = QStringLiteral("其他成员申请中");
                controlActionToolTip = QStringLiteral(
                    "正在等待屏幕分享者处理另一项控制申请");
            }
            else {
                controlActionText = QStringLiteral("申请控制");
                controlActionEnabled = signalingReady;
                controlActionToolTip = signalingReady
                    ? QStringLiteral("请求控制当前共享的屏幕")
                    : QStringLiteral("信令恢复后才能申请控制");
            }

            if (speakerButton_) {
                const QString speakerName = ActiveMediaDeviceText(
                    snapshot.localMediaDevices.speaker);
                speakerButton_->SetIcon(QString());
                speakerButton_->SetText(QStringLiteral("声音"));
                speakerButton_->SetSlashVisible(
                    snapshot.roomAudioPlaybackMuted);
                speakerButton_->setToolTip(
                    snapshot.roomAudioPlaybackMuted
                    ? QStringLiteral(
                          "开启远端声音\n恢复后输出到：%1")
                          .arg(speakerName)
                    : QStringLiteral(
                          "关闭远端声音\n正在输出到：%1")
                          .arg(speakerName));
            }
            if (microphoneButton_) {
                const QString microphoneName =
                    ActiveMediaDeviceText(
                        snapshot.localMediaDevices.microphone);
                const bool microphoneEnabled =
                    snapshot.localMicrophone ==
                    LocalMicrophoneState::kPublishing ||
                    snapshot.localMicrophone ==
                    LocalMicrophoneState::kStarting;
                microphoneButton_->SetIcon(QString());
                microphoneButton_->SetSlashVisible(!microphoneEnabled);
                microphoneButton_->SetText(QStringLiteral("麦克风"));
                microphoneButton_->setToolTip(
                    microphoneEnabled
                        ? QStringLiteral(
                              "关闭本机麦克风\n正在使用：%1")
                              .arg(microphoneName)
                        : QStringLiteral(
                              "开启本机麦克风\n开启后使用：%1")
                              .arg(microphoneName));
            }
            const auto pair = std::find_if(
                snapshot.roomPeerConnections.begin(),
                snapshot.roomPeerConnections.end(),
                [this](const RoomPeerConnectionSnapshot& current) {
                    return current.pairId ==
                           binding_.roomPairId.toStdString();
                });
            if (pair != snapshot.roomPeerConnections.end()) {
                remoteDisplayId = pair->sharedDisplayId;
                remoteDisplayLayoutVersion =
                    pair->sharedDisplayLayoutVersion;
                if (remoteSourceWidth_ != pair->sharedDisplayWidth ||
                    remoteSourceHeight_ != pair->sharedDisplayHeight) {
                    remoteSourceWidth_ = pair->sharedDisplayWidth;
                    remoteSourceHeight_ = pair->sharedDisplayHeight;
                    const auto [selectedWidth, selectedHeight] =
                        ScreenQualityBounds(selectedQuality_);
                    const bool selectedPresetExceedsSource =
                        selectedQuality_ != ScreenQualityTier::kOriginal &&
                        remoteSourceWidth_ > 0 &&
                        remoteSourceHeight_ > 0 &&
                        selectedWidth >= remoteSourceWidth_ &&
                        selectedHeight >= remoteSourceHeight_ &&
                        (selectedWidth > remoteSourceWidth_ ||
                         selectedHeight > remoteSourceHeight_);
                    if (selectedPresetExceedsSource) {
                        selectedQuality_ = ScreenQualityTier::kOriginal;
                        (void)RequestStreamPreference(false);
                    }
                    RebuildQualityMenu();
                }
                const bool pairActive =
                    pair->state == RoomPeerConnectionState::kActive;
                if (pair->state ==
                        RoomPeerConnectionState::kStarting ||
                    pair->state ==
                        RoomPeerConnectionState::kNegotiating) {
                    connectionStageText =
                        QStringLiteral("正在协商视频连接");
                } else if (pair->state ==
                           RoomPeerConnectionState::kConnecting) {
                    connectionStageText =
                        QStringLiteral("正在连接 WebRTC");
                } else if (pair->state ==
                           RoomPeerConnectionState::kDisconnected) {
                    connectionStageText =
                        QStringLiteral("网络波动，等待恢复");
                } else if (pair->state ==
                           RoomPeerConnectionState::kRecovering) {
                    connectionStageText =
                        QStringLiteral("正在恢复视频连接");
                } else if (pairActive &&
                           (remoteDisplayId == 0 ||
                            remoteDisplayLayoutVersion == 0)) {
                    connectionStageText =
                        QStringLiteral("正在同步显示信息");
                } else if (pairActive) {
                    connectionStageText =
                        QStringLiteral("正在等待远端画面");
                } else {
                    connectionStageText =
                        QStringLiteral("正在准备视频连接");
                }
                if (pairActive && showControlAction &&
                    snapshot.room.screenShareEpoch != 0 &&
                    screenStartupGeneration_ !=
                        snapshot.room.screenShareEpoch) {
                    BeginScreenStartup(
                        snapshot.room.screenShareEpoch);
                }
                const bool maySwitchRemoteDisplay =
                    snapshot.room.ownerDeviceId ==
                        snapshot.localDeviceId ||
                    snapshot.room.activeControllerDeviceId ==
                        snapshot.localDeviceId;
                remoteDisplaySwitchEnabled =
                    pairActive && maySwitchRemoteDisplay &&
                    pair->remoteDisplayCatalogReported &&
                    pair->remoteDisplays.size() > 1 &&
                    !pair->remoteDisplaySwitchPending;
                if (pendingRemoteDisplaySwitchSequence_ != 0 &&
                    pair->remoteDisplaySwitchSequence ==
                        pendingRemoteDisplaySwitchSequence_ &&
                    !pair->remoteDisplaySwitchPending) {
                    completedRemoteDisplaySwitchSucceeded =
                        pair->remoteDisplaySwitchError.empty();
                    completedRemoteDisplaySwitchMessage =
                        completedRemoteDisplaySwitchSucceeded
                            ? QStringLiteral("远端显示器已切换")
                            : QStringLiteral("切换显示器失败：%1")
                                  .arg(QString::fromStdString(
                                      pair->remoteDisplaySwitchError));
                    pendingRemoteDisplaySwitchSequence_ = 0;
                }
                if (pair->remoteDisplaySwitchPending) {
                    remoteDisplaySwitchToolTip =
                        QStringLiteral("正在切换远端显示器…");
                } else if (!maySwitchRemoteDisplay) {
                    remoteDisplaySwitchToolTip =
                        QStringLiteral(
                            "只有房主或当前控制者可以切换远端显示器");
                } else if (!pair->remoteDisplayCatalogReported) {
                    remoteDisplaySwitchToolTip =
                        QStringLiteral("等待远端上报显示器列表");
                } else if (pair->remoteDisplays.size() <= 1) {
                    remoteDisplaySwitchToolTip =
                        QStringLiteral("远端当前只有一个可用显示器");
                } else {
                    remoteDisplaySwitchToolTip =
                        QStringLiteral("切换远端正在共享的显示器");
                }
                if (pairActive) {
                    pairWasActive_ = true;
                } else if (pairWasActive_) {
                    showRecoveryPrompt =
                        pair->state == RoomPeerConnectionState::kStarting ||
                        pair->state == RoomPeerConnectionState::kNegotiating ||
                        pair->state == RoomPeerConnectionState::kConnecting ||
                        pair->state == RoomPeerConnectionState::kDisconnected ||
                        pair->state == RoomPeerConnectionState::kRecovering;
                    recoveryFailed =
                        pair->state == RoomPeerConnectionState::kFailed ||
                        pair->state == RoomPeerConnectionState::kClosed;
                    waitingForSignaling =
                        snapshot.connectivity != SessionConnectivityState::kOnline;
                    recoveryAttempt = pair->iceRestartAttempt;
                }
                const bool inputChannelsReady =
                    pair->controlReliableChannelOpen &&
                    pair->inputFastChannelOpen;
                enabled = enabled && pairActive &&
                    inputChannelsReady &&
                    remoteDisplayId != 0 &&
                    remoteDisplayLayoutVersion != 0;
                if (connectionStatusLabel_ && !pairActive) {
                    if (pair->state == RoomPeerConnectionState::kDisconnected) {
                        connectionStatusLabel_->setText(
                            QStringLiteral("网络波动，等待恢复"));
                    } else if (pair->state ==
                               RoomPeerConnectionState::kRecovering) {
                        connectionStatusLabel_->setText(
                            QStringLiteral("正在恢复 %1/3")
                                .arg(pair->iceRestartAttempt));
                    }
                }
                bool newShareNeedsPreference =
                    showControlAction &&
                    snapshot.room.screenShareEpoch != 0 &&
                    preferenceSentScreenShareEpoch_ !=
                        snapshot.room.screenShareEpoch;
                if (newShareNeedsPreference &&
                    preferenceRequestedScreenShareEpoch_ ==
                        snapshot.room.screenShareEpoch &&
                    !pair->screenPreferencePending) {
                    if (pair->errorCode ==
                        "screen_stream_preference_rejected") {
                        // The sharer can acknowledge the new lease before its
                        // first captured frame has established a source size.
                        // Leave the epoch unconfirmed so the next UI refresh
                        // retries instead of silently retaining the old rate.
                        preferenceRequestedScreenShareEpoch_ = 0;
                    }
                    else {
                        preferenceSentScreenShareEpoch_ =
                            snapshot.room.screenShareEpoch;
                    }
                }
                // The menu selection is only a request until the sharer
                // acknowledges it. Once the response arrives, show the value
                // that was actually applied instead of retaining a rejected
                // request as a false active setting.
                const std::uint32_t reportedMaximumFrameRate = std::clamp(
                    pair->screenMaximumFrameRate,
                    kMinimumScreenFrameRate,
                    kMaximumScreenFrameRate);
                if (reportedRemoteMaximumFrameRate_ !=
                    reportedMaximumFrameRate) {
                    reportedRemoteMaximumFrameRate_ =
                        reportedMaximumFrameRate;
                    RebuildFrameRateMenu();
                }
                const auto [savedWidth, savedHeight] =
                    ScreenQualityBounds(selectedQuality_);
                const bool savedQualityAlreadyApplied =
                    pair->screenPreferenceGeneration ==
                        snapshot.room.screenShareEpoch &&
                    pair->screenWidth > 0 &&
                    pair->screenHeight > 0 &&
                    ((savedWidth == 0 && savedHeight == 0) ||
                     (pair->screenWidth <= savedWidth &&
                      pair->screenHeight <= savedHeight));
                if (newShareNeedsPreference &&
                    !pair->screenPreferencePending &&
                    pair->screenFramesPerSecond ==
                        selectedFrameRate_ &&
                    savedQualityAlreadyApplied &&
                    pair->screenCaptureCapabilityReported) {
                    // ControllerMainWindow preflighted this policy while the
                    // share was still switching. Skip the post-activation
                    // request only after the runtime capture capability has
                    // also arrived. A preflight capability is intentionally
                    // ignored until the pending sharer becomes active.
                    preferenceRequestedScreenShareEpoch_ =
                        snapshot.room.screenShareEpoch;
                    preferenceSentScreenShareEpoch_ =
                        snapshot.room.screenShareEpoch;
                    newShareNeedsPreference = false;
                }
                if (!pair->screenPreferencePending &&
                    pair->screenPreferenceGeneration ==
                        snapshot.room.screenShareEpoch &&
                    pair->errorCode !=
                        "screen_stream_preference_rejected" &&
                    pair->screenFramesPerSecond > 0) {
                    // Persist quality only. FPS is intentionally scoped to a
                    // share generation and resets to 60 for the next share.
                    QSettings settings;
                    settings.setValue(
                        QString::fromLatin1(
                            kRemoteScreenQualitySetting),
                        static_cast<int>(selectedQuality_));
                }
                if (!newShareNeedsPreference &&
                    !pair->screenPreferencePending &&
                    pair->screenFramesPerSecond > 0 &&
                    selectedFrameRate_ !=
                        pair->screenFramesPerSecond) {
                    selectedFrameRate_ =
                        pair->screenFramesPerSecond;
                    if (frameRateMenu_) {
                        for (QAction* action :
                             frameRateMenu_->actions()) {
                            action->setChecked(
                                action->data().toUInt() ==
                                selectedFrameRate_);
                        }
                    }
                    if (frameRateButton_) {
                        frameRateButton_->setText(
                            QStringLiteral("目标帧率\n%1 FPS")
                            .arg(selectedFrameRate_));
                    }
                }
                if (qualityButton_) {
                    const auto [requestedWidth, requestedHeight] =
                        ScreenQualityBounds(selectedQuality_);
                    const bool showRequested = pair->screenPreferencePending &&
                        requestedWidth > 0 && requestedHeight > 0;
                    const int shownWidth = showRequested
                        ? static_cast<int>(requestedWidth)
                        : (pair->screenWidth > 0
                            ? pair->screenWidth
                            : static_cast<int>(requestedWidth));
                    const int shownHeight = showRequested
                        ? static_cast<int>(requestedHeight)
                        : (pair->screenHeight > 0
                            ? pair->screenHeight
                            : static_cast<int>(requestedHeight));
                    qualityButton_->setText(
                        shownWidth > 0 && shownHeight > 0
                        ? QStringLiteral("分辨率\n%1 × %2")
                        .arg(shownWidth)
                        .arg(shownHeight)
                        : QStringLiteral("分辨率\n原始画质"));
                    qualityButton_->setToolTip(
                        QStringLiteral("当前%1；点击调整远端输出分辨率")
                        .arg(ScreenQualityText(selectedQuality_)));
                    if (pair->screenWidth > 0 && pair->screenHeight > 0) {
                        qualityButton_->setToolTip(
                            QStringLiteral(
                                "%1；远端已应用 %2 × %3，仅对当前观看端生效")
                            .arg(ScreenQualityText(selectedQuality_))
                            .arg(pair->screenWidth)
                            .arg(pair->screenHeight));
                    }
                }
                if (frameRateButton_) {
                    QString frameRateTip;
                    if (pair->screenPreferencePending) {
                        frameRateTip =
                            QStringLiteral("正在请求远端调整帧率");
                    } else if (pair->screenCaptureCapabilityReported) {
                        frameRateTip = QStringLiteral(
                            "当前共享端最高支持 %1 FPS\n配置：%2；实际：%3")
                            .arg(remoteMaximumFrameRate_)
                            .arg(CaptureBackendText(
                                pair->screenCaptureConfiguredBackend))
                            .arg(CaptureBackendText(
                                pair->screenCaptureActiveBackend));
                        if (!pair->screenCaptureFallbackReason.empty()) {
                            frameRateTip += QStringLiteral("\n回退原因：%1")
                                .arg(QString::fromStdString(
                                    pair->screenCaptureFallbackReason));
                        }
                    } else {
                        frameRateTip = QStringLiteral(
                            "等待当前共享端上报实际采集后端，暂按 %1 FPS 显示")
                            .arg(remoteMaximumFrameRate_);
                    }
                    frameRateButton_->setToolTip(frameRateTip);
                }
                // A stopped share clears the sender-side preference cache.
                // Apply this generation's 60-FPS default exactly once unless
                // the user changes it after the share becomes active.
                if (newShareNeedsPreference &&
                    !pair->screenPreferencePending &&
                    preferenceRequestedScreenShareEpoch_ !=
                        snapshot.room.screenShareEpoch &&
                    !roomScreenPreferenceRetryScheduled_) {
                    roomScreenPreferenceRetryScheduled_ = true;
                    const std::uint64_t expectedEpoch =
                        snapshot.room.screenShareEpoch;
                    QTimer::singleShot(0, this,
                        [this, expectedEpoch] {
                            roomScreenPreferenceRetryScheduled_ = false;
                            if (!sessionEngine_ || !binding_.IsRoom()) {
                                return;
                            }
                            const auto current = sessionEngine_->Snapshot();
                            if (current.room.screenShareState !=
                                    RoomScreenShareState::kActive ||
                                current.room.screenShareEpoch !=
                                    expectedEpoch ||
                                current.room.screenSharerDeviceId.empty() ||
                                current.room.screenSharerDeviceId ==
                                    current.localDeviceId) {
                                return;
                            }
                            if (RequestStreamPreference(false)) {
                                preferenceRequestedScreenShareEpoch_ =
                                    expectedEpoch;
                            }
                        });
                }
            }
            else if (pairWasActive_) {
                recoveryFailed = true;
            }
        }

        if (controlLeaseAssigned && !enabled) {
            controlActionText = QStringLiteral("正在启用控制…");
            controlActionEnabled = false;
            controlActionToolTip = QStringLiteral(
                "控制申请已通过，正在等待输入通道和远端显示信息就绪");
        }

        if (remoteDisplayButton_) {
            remoteDisplayButton_->setEnabled(
                remoteDisplaySwitchEnabled);
            remoteDisplayButton_->setToolTip(
                remoteDisplaySwitchToolTip);
            if (!completedRemoteDisplaySwitchMessage.isEmpty()) {
                RemoteCToast::ShowAbove(
                    remoteDisplayButton_,
                    completedRemoteDisplaySwitchMessage,
                    completedRemoteDisplaySwitchSucceeded
                        ? RemoteCToast::Tone::kSuccess
                        : RemoteCToast::Tone::kError);
            }
        }

        UpdateNetworkRecoveryPrompt(
            showRecoveryPrompt,
            recoveryAttempt,
            waitingForSignaling,
            recoveryFailed);

        inputControlEnabled_ = enabled;
        if (desktopCanvas_) {
            auto* canvas =
                static_cast<RemoteDesktopCanvas*>(desktopCanvas_);
            canvas->SetRemoteDisplayIdentity(
                remoteDisplayId, remoteDisplayLayoutVersion);
            canvas->SetConnectionStage(connectionStageText);
            if (enabled) {
                remoteInputDispatcher_->SetEnabled(true);
                canvas->SetControlEnabled(true);
            } else {
                // Stop and drain the high-resolution scheduler before
                // disabling its engine bridge, so no worker callback can
                // outlive the active control lease.
                canvas->SetControlEnabled(false);
                remoteInputDispatcher_->SetEnabled(false);
            }
        } else {
            remoteInputDispatcher_->SetEnabled(enabled);
        }
        if (controlBadge_) {
            controlBadge_->hide();
        }
        if (controlButton_) {
            const bool statusChanged =
                lastControlActionText_ != controlActionText;
            const bool desiredControlButtonVisible = showControlAction || enabled;
            const bool visibilityChanged =
                controlButton_->isVisible() != desiredControlButtonVisible;
            if (visibilityChanged) {
                controlButton_->setVisible(desiredControlButtonVisible);
            }

            if (desiredControlButtonVisible && statusChanged) {
                QString compactControlAction = controlActionText;
                if (compactControlAction == QStringLiteral("等待对方批准…")) {
                    compactControlAction = QStringLiteral("等待批准");
                }
                else if (compactControlAction ==
                    QStringLiteral("其他成员正在控制")) {
                    compactControlAction = QStringLiteral("他人控制");
                }
                else if (compactControlAction ==
                    QStringLiteral("其他成员申请中")) {
                    compactControlAction = QStringLiteral("他人申请");
                }
                controlButton_->SetIcon(QString());
                controlButton_->SetText(compactControlAction);
                controlButton_->SetTone(
                    enabled ? QStringLiteral("positive")
                    : QStringLiteral("primary"));
                controlButton_->setToolTip(controlActionToolTip);
                lastControlActionText_ = controlActionText;
            }
            if (controlButton_->isEnabled() != controlActionEnabled) {
                controlButton_->setEnabled(controlActionEnabled);
            }
        }
    }
