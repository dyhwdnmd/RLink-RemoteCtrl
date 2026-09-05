// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

void ControllerMainWindow::ApplyClipboardConfigurationFromUi(
    bool showFeedback)
{
    if (!remotePasteEnabledSelector_ || !clipboardFormatsSelector_ ||
        !clipboardLargeFileLimitSelector_ ||
        !clipboardCacheRetentionSelector_ ||
        !clipboardCacheCapacitySelector_) {
        return;
    }
    const bool enabled =
        remotePasteEnabledSelector_->currentData().toBool();
    const QString formats =
        clipboardFormatsSelector_->currentData().toString();
    const int fileLimitMiB = (std::min)(
        clipboardLargeFileLimitSelector_->currentData().toInt(), 512);
    const int retentionMinutes =
        clipboardCacheRetentionSelector_->currentData().toInt();
    const qulonglong cacheLimitGiB =
        clipboardCacheCapacitySelector_->currentData().toULongLong();

    QSettings settings;
    settings.setValue(QString::fromLatin1(kRemotePasteEnabledSetting),
                      enabled);
    settings.setValue(QString::fromLatin1(kClipboardFormatsSetting),
                      formats);
    settings.setValue(QString::fromLatin1(kClipboardFileLimitSetting),
                      fileLimitMiB);
    settings.setValue(
        QString::fromLatin1(kClipboardCacheRetentionSetting),
        retentionMinutes);
    settings.setValue(
        QString::fromLatin1(kClipboardCacheCapacitySetting),
        cacheLimitGiB);
    if (clipboardController_) {
        app::RemotePasteConfiguration configuration;
        configuration.enabled = enabled;
        configuration.unicodeText = true;
        configuration.html = formats != QStringLiteral("text");
        configuration.rtf = formats != QStringLiteral("text");
        configuration.png =
            formats == QStringLiteral("all") ||
            formats == QStringLiteral("no_files");
        configuration.files = formats == QStringLiteral("all");
        configuration.automaticFileLimitBytes =
            static_cast<std::uint64_t>((std::max)(1, fileLimitMiB)) *
            1024ull * 1024ull;
        configuration.cacheBaseDirectory = std::filesystem::path(
            settings.value(
                QString::fromLatin1(
                    kClipboardCacheBaseDirectorySetting),
                InitialClipboardCacheBaseDirectory())
                .toString().toStdWString());
        configuration.cacheRetention = std::chrono::minutes(
            (std::max)(10, retentionMinutes));
        configuration.cacheLimitBytes =
            static_cast<std::uint64_t>(cacheLimitGiB) *
            1024ull * 1024ull * 1024ull;
        clipboardController_->SetConfiguration(configuration);
    }
    if (showFeedback) {
        RemoteCToast::Show(
            this,
            enabled
                ? QStringLiteral("远程粘贴已开启")
                : QStringLiteral("远程粘贴已关闭"),
            RemoteCToast::Tone::kSuccess);
    }
}

void ControllerMainWindow::UpdateClipboardSession(
    const SessionEngineSnapshot& snapshot)
{
    if (!clipboardController_) {
        return;
    }
    app::ClipboardSessionContext context;
    context.localDeviceId = snapshot.localDeviceId;
    const bool directActive =
        snapshot.state == SessionEngineState::kActive &&
        snapshot.purpose == SessionPurpose::kRemoteControl &&
        snapshot.remoteControlRole != RemoteControlRole::kNone &&
        !snapshot.sessionId.empty() && !snapshot.peerDeviceId.empty();
    if (directActive) {
        context.roomId = snapshot.sessionId;
        context.active = true;
        context.localIsController =
            snapshot.remoteControlRole == RemoteControlRole::kController;
        context.peerDeviceId = snapshot.peerDeviceId;
        context.transportReady =
            snapshot.directClipboardReliableChannelOpen &&
            snapshot.directClipboardTransferChannelOpen;
        clipboardController_->UpdateSession(std::move(context));
        return;
    }

    context.roomId = snapshot.room.roomId;
    const bool roomAndControlActive =
        snapshot.room.membership == RoomMembershipState::kActive &&
        snapshot.roomControlGrantActive &&
        (snapshot.room.screenShareState == RoomScreenShareState::kActive ||
         snapshot.room.screenShareState ==
             RoomScreenShareState::kRecovering) &&
        !snapshot.room.screenSharerDeviceId.empty() &&
        !snapshot.room.activeControllerDeviceId.empty();
    if (roomAndControlActive &&
        snapshot.room.screenSharerDeviceId == snapshot.localDeviceId) {
        context.active = clipboardAllowedForCurrentControl_;
        context.localIsController = false;
        context.peerDeviceId = snapshot.room.activeControllerDeviceId;
    } else if (roomAndControlActive &&
               snapshot.room.activeControllerDeviceId ==
                   snapshot.localDeviceId) {
        context.active = true;
        context.localIsController = true;
        context.peerDeviceId = snapshot.room.screenSharerDeviceId;
    }
    if (context.active) {
        const auto pair = std::find_if(
            snapshot.roomPeerConnections.begin(),
            snapshot.roomPeerConnections.end(),
            [&context](const RoomPeerConnectionSnapshot& candidate) {
                return candidate.peerDeviceId == context.peerDeviceId;
            });
        context.transportReady =
            pair != snapshot.roomPeerConnections.end() &&
            pair->state == RoomPeerConnectionState::kActive &&
            pair->clipboardReliableChannelOpen &&
            pair->clipboardTransferChannelOpen;
    }
    clipboardController_->UpdateSession(std::move(context));
}

void ControllerMainWindow::UpdateDirectFileTransferSession(
    const SessionEngineSnapshot& snapshot)
{
    const bool directSession =
        snapshot.state == SessionEngineState::kActive &&
        snapshot.purpose == SessionPurpose::kRemoteControl &&
        !snapshot.peerDeviceId.empty();
    if (!directSession) {
        return;
    }

    std::vector<FileTransferPeer> peers;
    std::vector<std::string> availablePeerIds;
    std::vector<std::string> recoveringPeerIds;
    if (snapshot.directFileTransferChannelOpen) {
        FileTransferPeer peer;
        peer.deviceId = snapshot.peerDeviceId;
        peer.displayName = remoteSessionBinding_ &&
                remoteSessionBinding_->peerDeviceId ==
                    QString::fromStdString(snapshot.peerDeviceId) &&
                !remoteSessionBinding_->peerDeviceName.isEmpty()
            ? remoteSessionBinding_->peerDeviceName
            : QString::fromStdString(snapshot.peerDeviceId);
        peers.push_back(std::move(peer));
        availablePeerIds.push_back(snapshot.peerDeviceId);
    } else {
        recoveringPeerIds.push_back(snapshot.peerDeviceId);
    }
    if (fileTransferController_) {
        fileTransferController_->UpdatePeerConnectivity(
            availablePeerIds, recoveringPeerIds);
    }
    if (fileTransferWindow_) {
        fileTransferWindow_->SyncPeers(peers, frameGeometry());
    }
}

void ControllerMainWindow::OnClipboardStateChanged(
    const app::ClipboardControllerSnapshot& snapshot)
{
    if (QThread::currentThread() != thread()) {
        bool expected = false;
        if (clipboardUiUpdatePending_.compare_exchange_strong(
                expected, true, std::memory_order_acq_rel)) {
            QMetaObject::invokeMethod(this, [this] {
                clipboardUiUpdatePending_.store(
                    false, std::memory_order_release);
                if (clipboardController_) {
                    OnClipboardStateChanged(
                        clipboardController_->Snapshot());
                }
            }, Qt::QueuedConnection);
        }
        return;
    }
    if (clipboardCacheUsageLabel_) {
        const auto formatCacheBytes = [](std::uint64_t bytes) {
            constexpr double kMiB = 1024.0 * 1024.0;
            constexpr double kGiB = 1024.0 * 1024.0 * 1024.0;
            return bytes >= static_cast<std::uint64_t>(kGiB)
                ? QStringLiteral("%1 GiB").arg(bytes / kGiB, 0, 'f', 2)
                : QStringLiteral("%1 MiB").arg(bytes / kMiB, 0, 'f', 1);
        };
        clipboardCacheUsageLabel_->setText(QStringLiteral(
            "%1 个项目 · %2 / 上限 %3")
            .arg(static_cast<qulonglong>(snapshot.cacheEntryCount))
            .arg(formatCacheBytes(snapshot.cacheBytes))
            .arg(formatCacheBytes(snapshot.cacheEffectiveLimitBytes)));
        clipboardCacheUsageLabel_->setToolTip(
            QStringLiteral("缓存路径：%1\n磁盘可用：%2\n安全容量：%3")
                .arg(QString::fromUtf8(snapshot.cacheRootPath))
                .arg(formatCacheBytes(snapshot.cacheAvailableBytes))
                .arg(formatCacheBytes(snapshot.cacheSafeCapacityBytes)));
    }
    if (clearClipboardCacheButton_) {
        clearClipboardCacheButton_->setEnabled(
            snapshot.cacheEntryCount > 0);
    }
    const bool receivedAdvanced =
        snapshot.receivedItems > displayedClipboardReceivedItems_;
    const bool rejectedAdvanced =
        snapshot.rejectedItems > displayedClipboardRejectedItems_;
    const QString transferId =
        QString::fromStdString(snapshot.transferId);
    if (snapshot.explorerConflictPending && !transferId.isEmpty()) {
        if (remoteSessionWindow_) {
            // Metadata preflight has not started a transfer. Keep exactly one
            // window visible: the decision prompt. This also closes a legacy
            // requesting popup if the peer did not provide V8 metadata.
            remoteSessionWindow_->CloseRemotePasteProgress(transferId);
        }
        if (promptedClipboardConflictId_ != transferId) {
            promptedClipboardConflictId_ = transferId;
            QStringList names;
            for (const auto& name : snapshot.explorerConflictNames) {
                if (names.size() >= 5) break;
                names.push_back(QString::fromUtf8(name));
            }
            QString conflictList = names.join(QStringLiteral("\n"));
            if (snapshot.explorerConflictNames.size() >
                static_cast<std::size_t>(names.size())) {
                conflictList += QStringLiteral("\n等 %1 个同名项目")
                    .arg(snapshot.explorerConflictNames.size());
            }
            const QString destination = QString::fromUtf8(
                snapshot.localPasteDestinationPath);
            QTimer::singleShot(0, this,
                [this, transferId, destination, conflictList] {
                    if (!clipboardController_) return;
                    const auto latest = clipboardController_->Snapshot();
                    if (!latest.explorerConflictPending ||
                        QString::fromStdString(latest.transferId) !=
                            transferId) {
                        return;
                    }
                    const bool replace = RemoteCDialog::Confirm(
                        nullptr,
                        QStringLiteral("目标位置已有同名项目"),
                        QStringLiteral(
                            "粘贴位置：%1\n\n"
                            "以下项目已经存在：\n%2\n\n"
                            "继续后将合并同名文件夹，并替换其中的同名文件。")
                            .arg(QDir::toNativeSeparators(destination),
                                 conflictList),
                        QStringLiteral("替换并粘贴"),
                        QStringLiteral("取消粘贴"),
                        RemoteCDialog::Tone::kDanger, true, true,
                        latest.localPasteTargetWindow, true);
                    (void)clipboardController_->ResolveExplorerConflict(
                        replace);
                });
        }
        return;
    }
    if (!snapshot.explorerConflictPending &&
        promptedClipboardConflictId_ == transferId) {
        promptedClipboardConflictId_.clear();
    }
    const auto formatTransferBytes = [](std::uint64_t bytes) {
        constexpr double kKiB = 1024.0;
        constexpr double kMiB = 1024.0 * 1024.0;
        constexpr double kGiB = 1024.0 * 1024.0 * 1024.0;
        if (bytes >= static_cast<std::uint64_t>(kGiB)) {
            return QStringLiteral("%1 GB").arg(bytes / kGiB, 0, 'f', 2);
        }
        if (bytes >= static_cast<std::uint64_t>(kMiB)) {
            return QStringLiteral("%1 MB").arg(bytes / kMiB, 0, 'f', 1);
        }
        if (bytes >= static_cast<std::uint64_t>(kKiB)) {
            return QStringLiteral("%1 KB").arg(bytes / kKiB, 0, 'f', 1);
        }
        return QStringLiteral("%1 B").arg(
            static_cast<qulonglong>(bytes));
    };
    const bool needsProgressDialog =
        snapshot.transferActive &&
        snapshot.transferContainsFiles &&
        (snapshot.state == "requesting_remote" ||
         snapshot.transferTotalBytes > kRemotePastePopupThresholdBytes);

    if (needsProgressDialog && remoteSessionWindow_) {
        QString progressText;
        const QString progressTitle = snapshot.transferOutgoing
            ? QStringLiteral("正在粘贴到远端")
            : QStringLiteral("正在从远端粘贴到本机");
        const QString totalText =
            formatTransferBytes(snapshot.transferTotalBytes);
        const QString completedText =
            formatTransferBytes(snapshot.transferCompletedBytes);
        const QString speedText = snapshot.transferBytesPerSecond > 1.0
            ? QStringLiteral("%1/s").arg(formatTransferBytes(
                  static_cast<std::uint64_t>(
                      snapshot.transferBytesPerSecond)))
            : QStringLiteral("正在估算速度");
        const QString remainingText =
            snapshot.transferEstimatedRemainingSeconds > 0
                ? QStringLiteral("预计剩余 %1 秒").arg(
                      static_cast<qulonglong>(
                          snapshot.transferEstimatedRemainingSeconds))
                : QStringLiteral("正在估算剩余时间");
        if (snapshot.state == "requesting_remote") {
            progressText = snapshot.transferTotalBytes > 0
                ? QStringLiteral(
                    "正在与远端建立文件传输 · %1 个项目 · %2\n"
                    "连接确认后将立即开始传输")
                    .arg(snapshot.transferItemCount)
                    .arg(totalText)
                : QStringLiteral(
                    "正在与远端建立文件传输…\n"
                    "连接确认后将立即开始传输");
        } else if (snapshot.state == "offering") {
            progressText = QStringLiteral(
                "正在与远端确认文件传输…\n"
                "%1 个文件项目 · 总大小 %2")
                .arg(snapshot.transferItemCount)
                .arg(totalText);
        } else if (snapshot.state == "applying") {
            progressText = (snapshot.transferOutgoing
                ? QStringLiteral(
                    "文件已经传输完成，正在粘贴到远端目标位置…\n"
                    "%1 / %2 · %3")
                : QStringLiteral(
                    "文件已经传输完成，正在粘贴到本机目标位置…\n"
                    "%1 / %2 · %3"))
                .arg(completedText, totalText, speedText);
        } else {
            const int percent = snapshot.transferTotalBytes == 0
                ? 0
                : static_cast<int>((std::min)(
                      100ull,
                      snapshot.transferCompletedBytes * 100ull /
                          snapshot.transferTotalBytes));
            progressText = (snapshot.transferOutgoing
                ? QStringLiteral(
                    "正在向远端传输 %1 个文件项目 · %2%\n"
                    "%3 / %4 · %5 · %6")
                : QStringLiteral(
                    "正在从远端接收 %1 个文件项目 · %2%\n"
                    "%3 / %4 · %5 · %6"))
                .arg(snapshot.transferItemCount)
                .arg(percent)
                .arg(completedText, totalText, speedText, remainingText);
        }
        if (!snapshot.transferOutgoing &&
            !snapshot.localPasteDestinationPath.empty()) {
            const QString nativeDestination = QDir::toNativeSeparators(
                QString::fromUtf8(snapshot.localPasteDestinationPath));
            const QString visibleDestination = QFontMetrics(
                QApplication::font()).elidedText(
                    nativeDestination, Qt::ElideMiddle, 315);
            progressText.prepend(
                QStringLiteral("粘贴位置：%1\n")
                    .arg(visibleDestination));
        }

        if (pendingRemotePasteDialogId_ != transferId) {
            pendingRemotePasteDialogId_ = transferId;
            // Show the prompt on the initial RequestCurrent snapshot instead
            // of waiting for hashing and Offer construction, so the very
            // first Ctrl+V gives immediate feedback.
            visibleRemotePasteDialogId_ = transferId;
        }
        if (visibleRemotePasteDialogId_ == transferId) {
            const double progress = snapshot.transferTotalBytes == 0
                ? 0.0
                : static_cast<double>(snapshot.transferCompletedBytes) /
                      static_cast<double>(snapshot.transferTotalBytes);
            remoteSessionWindow_->ShowRemotePasteProgress(
                transferId, progressTitle, progressText, progress,
                snapshot.localPasteTargetWindow);
        }
    } else if (!snapshot.transferActive) {
        const QString completedTransferId =
            !visibleRemotePasteDialogId_.isEmpty()
                ? visibleRemotePasteDialogId_
                : pendingRemotePasteDialogId_;
        if (rejectedAdvanced && !snapshot.lastErrorMessage.empty() &&
            remoteSessionWindow_) {
            QString message =
                QString::fromStdString(snapshot.lastErrorMessage);
            if (snapshot.lastErrorCode == "clipboard_transfer_timeout") {
                message = QStringLiteral(
                    "远端没有及时确认本次传输。当前粘贴状态已经自动释放，"
                    "请确认 P2P 连接正常后重新粘贴。");
            } else if (snapshot.lastErrorCode ==
                       "clipboard_capture_timeout") {
                message = QStringLiteral(
                    "本机剪贴板没有及时返回内容。当前粘贴状态已经自动释放，"
                    "请重新复制文件后再按 Ctrl+V。");
            } else if (snapshot.lastErrorCode ==
                       "clipboard_capture_rejected") {
                message = QStringLiteral(
                    "文件无法读取、超过设置中的大小上限，或包含当前不支持的内容。");
            } else if (snapshot.lastErrorCode ==
                       "remote_paste_clipboard_unsupported") {
                message = QStringLiteral(
                    "本机剪贴板中没有可读取的文件或文本。请重新复制文件后再试。");
            } else if (snapshot.lastErrorCode ==
                       "clipboard_apply_failed") {
                message = QStringLiteral(
                    "文件已经传到远端，但远端系统剪贴板未能接收该文件。请重试。");
            } else if (snapshot.lastErrorCode ==
                       "remote_paste_input_unavailable") {
                message = QStringLiteral(
                    "远端文件已准备好，但 Ctrl+V 输入没有成功送达。请确认控制权仍然有效。");
            } else if (snapshot.lastErrorCode ==
                       "clipboard_remote_request_timeout") {
                message = QStringLiteral(
                    "被控端没有及时响应剪贴板请求，请确认连接正常后重新粘贴。");
            } else if (snapshot.lastErrorCode ==
                       "clipboard_local_paste_failed") {
                message = QStringLiteral(
                    "文件已经接收到本机剪贴板，但未能自动送入刚才的目标窗口。"
                    "请回到目标应用再按一次 Ctrl+V。");
            } else if (snapshot.lastErrorCode ==
                       "clipboard_current_unavailable") {
                message = QStringLiteral(
                    "被控端的剪贴板内容已经变化，请重新复制后再粘贴。");
            } else if (snapshot.lastErrorCode ==
                       "clipboard_source_changed") {
                message = QStringLiteral(
                    "源文件在传输过程中发生了变化，本次粘贴已安全取消。"
                    "请等待文件写入完成后重新复制并粘贴。");
            } else if (snapshot.lastErrorCode ==
                       "clipboard_hash_update_failed" ||
                       snapshot.lastErrorCode ==
                           "clipboard_hash_reset_failed") {
                message = QStringLiteral(
                    "文件完整性校验未能继续，本次粘贴已取消。请重新粘贴；"
                    "若问题持续出现，请检查系统加密服务状态。");
            }
            remoteSessionWindow_->ShowRemotePasteFailure(
                completedTransferId, message);
        } else if (!completedTransferId.isEmpty() &&
                   remoteSessionWindow_) {
            remoteSessionWindow_->CompleteRemotePasteProgress(
                completedTransferId);
        }
        pendingRemotePasteDialogId_.clear();
        visibleRemotePasteDialogId_.clear();
    }
    if (receivedAdvanced) {
        RemoteCToast::Show(
            this,
            snapshot.lastFormat == "file" ||
                    snapshot.lastFormat == "directory"
                ? QStringLiteral("已接收远程粘贴文件")
                : QStringLiteral("已接收远程粘贴内容"),
            RemoteCToast::Tone::kSuccess);
    } else if (rejectedAdvanced && !snapshot.lastErrorMessage.empty() &&
               !remoteSessionWindow_) {
        RemoteCToast::Show(
            this,
            QStringLiteral("远程粘贴未完成：%1")
                .arg(QString::fromStdString(snapshot.lastErrorMessage)),
            RemoteCToast::Tone::kError);
    }
    displayedClipboardSentItems_ = snapshot.sentItems;
    displayedClipboardReceivedItems_ = snapshot.receivedItems;
    displayedClipboardRejectedItems_ = snapshot.rejectedItems;
    displayedClipboardErrorCode_ =
        QString::fromStdString(snapshot.lastErrorCode);
}
