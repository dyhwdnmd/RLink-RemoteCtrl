// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

    void RemoteSessionWindow::BeginMediaDeviceSelection(
        MediaDeviceKind kind, const QString& deviceId)
    {
        if (!sessionEngine_ || deviceId.isEmpty()) {
            return;
        }
        const auto media =
            sessionEngine_->Snapshot().localMediaDevices;
        const MediaDeviceCategorySnapshot* category = nullptr;
        QString* pendingId = nullptr;
        QString label;
        SessionCommandResult
            (app::InProcessSessionEngine::*operation)(
                const std::string&) = nullptr;
        switch (kind) {
        case MediaDeviceKind::kCamera:
            category = &media.camera;
            pendingId = &pendingCameraDeviceId_;
            label = QStringLiteral("摄像头");
            operation =
                &app::InProcessSessionEngine::
                    SelectLocalCameraDevice;
            break;
        case MediaDeviceKind::kMicrophone:
            category = &media.microphone;
            pendingId = &pendingMicrophoneDeviceId_;
            label = QStringLiteral("麦克风");
            operation =
                &app::InProcessSessionEngine::
                    SelectLocalMicrophoneDevice;
            break;
        case MediaDeviceKind::kSpeaker:
            category = &media.speaker;
            pendingId = &pendingSpeakerDeviceId_;
            label = QStringLiteral("扬声器");
            operation =
                &app::InProcessSessionEngine::
                    SelectLocalSpeakerDevice;
            break;
        }
        if (!category || !pendingId || !operation) {
            return;
        }
        if (category->state ==
                MediaDeviceSelectionState::kReady &&
            QString::fromStdString(
                category->preferredDeviceId) == deviceId) {
            RemoteCToast::ShowAbove(
                mediaDeviceButton_,
                QStringLiteral("当前已经在使用该%1").arg(label),
                RemoteCToast::Tone::kInformation);
            return;
        }

        *pendingId = deviceId;
        const auto result =
            (sessionEngine_->*operation)(deviceId.toStdString());
        if (!result.accepted) {
            pendingId->clear();
            RemoteCToast::ShowAbove(
                mediaDeviceButton_,
                QStringLiteral("%1切换失败：%2")
                    .arg(label,
                         QString::fromStdString(
                             result.errorMessage)),
                RemoteCToast::Tone::kError);
            return;
        }
        RemoteCToast::ShowAbove(
            mediaDeviceButton_,
            QStringLiteral("正在切换%1…").arg(label),
            RemoteCToast::Tone::kInformation);
    }

    void RemoteSessionWindow::CompleteMediaDeviceSelections(
        const MediaDeviceSnapshot& media)
    {
        const auto complete =
            [this](
                const MediaDeviceCategorySnapshot& category,
                QString& pendingId,
                const char* settingKey,
                const QString& label) {
                if (pendingId.isEmpty()) {
                    return;
                }
                if (category.state ==
                        MediaDeviceSelectionState::kReady &&
                    QString::fromStdString(
                        category.preferredDeviceId) ==
                        pendingId) {
                    QSettings settings;
                    settings.setValue(
                        QString::fromLatin1(settingKey),
                        pendingId);
                    RemoteCToast::ShowAbove(
                        mediaDeviceButton_,
                        QStringLiteral("%1已切换").arg(label),
                        RemoteCToast::Tone::kSuccess);
                    pendingId.clear();
                    return;
                }
                if (category.state ==
                        MediaDeviceSelectionState::kFailed ||
                    category.state ==
                        MediaDeviceSelectionState::kUnavailable) {
                    const QString error =
                        category.errorMessage.empty()
                            ? QStringLiteral("设备不可用")
                            : QString::fromStdString(
                                  category.errorMessage);
                    RemoteCToast::ShowAbove(
                        mediaDeviceButton_,
                        QStringLiteral("%1切换失败：%2")
                            .arg(label, error),
                        RemoteCToast::Tone::kError);
                    pendingId.clear();
                }
            };
        complete(
            media.camera, pendingCameraDeviceId_,
            "media/cameraDeviceId", QStringLiteral("摄像头"));
        complete(
            media.microphone, pendingMicrophoneDeviceId_,
            "media/microphoneDeviceId", QStringLiteral("麦克风"));
        complete(
            media.speaker, pendingSpeakerDeviceId_,
            "media/speakerDeviceId", QStringLiteral("扬声器"));
    }
