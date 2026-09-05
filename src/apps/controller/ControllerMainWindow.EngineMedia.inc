// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

bool ControllerMainWindow::InitializeEngine()
{
    if (!engine_) {
        return false;
    }
    engine_->SetObserver(this);
    auto* inProcessEngine = dynamic_cast<app::InProcessSessionEngine*>(
        engine_.get());
    if (!inProcessEngine) {
        const auto startResult = engine_->Start();
        CompleteEngineInitialization(startResult);
        return startResult.accepted;
    }

    const auto beginResult = inProcessEngine->BeginStart();
    if (!beginResult.accepted) {
        CompleteEngineInitialization(beginResult);
        return false;
    }

    SetRuntimeStatus(QStringLiteral("● WebRTC 正在初始化"),
                     QStringLiteral("#a66b12"));
    decoderStatus_->setText(QStringLiteral("● 正在检测视频能力"));
    decoderStatus_->setStyleSheet(QStringLiteral("color:#a66b12;"));
    if (connectButton_) {
        connectButton_->setEnabled(false);
    }

    engineStartThread_ = std::jthread(
        [this, inProcessEngine](std::stop_token stopToken) {
            const auto runtimeResult =
                inProcessEngine->InitializeRuntimeForStart();
            if (stopToken.stop_requested()) {
                return;
            }
            QMetaObject::invokeMethod(
                this,
                [this, inProcessEngine, runtimeResult] {
                    if (!sessionEngineStarted_ ||
                        engine_.get() != inProcessEngine) {
                        return;
                    }
                    const auto startResult =
                        inProcessEngine->CompleteStart(runtimeResult);
                    CompleteEngineInitialization(startResult);
                },
                Qt::QueuedConnection);
        });
    return true;
}

void ControllerMainWindow::CompleteEngineInitialization(
    const SessionCommandResult& startResult)
{
    if (!startResult.accepted) {
        // A valid negative hardware-encoder enumeration is still useful.
        // Persist it even when an explicit hardware-only preference makes
        // the runtime reject startup.
        PersistHardwareCapabilityCache();
        connectButton_->setEnabled(false);
        SetRuntimeStatus(QStringLiteral("● WebRTC 初始化失败"),
                         QStringLiteral("#d14343"));
        decoderStatus_->setText(QStringLiteral("● 视频能力不可用"));
        decoderStatus_->setStyleSheet(QStringLiteral("color:#d14343;"));
        return;
    }

    const auto report = engine_->Capabilities();
    PersistHardwareCapabilityCache();
    SetRuntimeStatus(QStringLiteral("● WebRTC 运行时就绪"),
                     QStringLiteral("#138b57"));
    if (report.mfD3D11DecoderHardware) {
        decoderStatus_->setText(
            QStringLiteral("● MF/D3D11 H264 硬件解码可用"));
        decoderStatus_->setStyleSheet(QStringLiteral("color:#138b57;"));
    } else if (report.mfD3D11DecoderSoftware) {
        decoderStatus_->setText(
            QStringLiteral("● MF H264 软件解码（D3D11 输出）"));
        decoderStatus_->setStyleSheet(QStringLiteral("color:#a66b12;"));
    } else if (report.ffmpegSoftwareH264Decoder) {
        decoderStatus_->setText(
            QStringLiteral("● FFmpeg H264 软件解码可用"));
        decoderStatus_->setStyleSheet(QStringLiteral("color:#a66b12;"));
    } else if (report.hasH264Decoder) {
        decoderStatus_->setText(QStringLiteral("● H264 解码可用"));
        decoderStatus_->setStyleSheet(QStringLiteral("color:#a66b12;"));
    } else {
        decoderStatus_->setText(QStringLiteral("● H264 解码不可用"));
        decoderStatus_->setStyleSheet(QStringLiteral("color:#d14343;"));
    }
}

void ControllerMainWindow::PersistHardwareCapabilityCache()
{
    if (!engine_) {
        return;
    }
    const auto report = engine_->Capabilities();
    if (report.hardwareFingerprint.empty() ||
        !report.h264HardwareEncoderProbeSucceeded ||
        ((report.videoEncoderPreference == "software" ||
          report.videoEncoderPreference == "ffmpeg") &&
         !report.h264HardwareEncoderProbeFromCache)) {
        return;
    }

    QStringList descriptions;
    for (const auto& value :
         report.h264HardwareEncoderDescriptions) {
        descriptions.push_back(QString::fromStdString(value));
    }
    QStringList warnings;
    for (const auto& value :
         report.h264HardwareEncoderWarnings) {
        warnings.push_back(QString::fromStdString(value));
    }

    QSettings settings;
    settings.setValue(
        QStringLiteral("media/hardwareProbe/fingerprint"),
        QString::fromStdString(report.hardwareFingerprint));
    settings.setValue(
        QStringLiteral("media/hardwareProbe/valid"), true);
    settings.setValue(
        QStringLiteral(
            "media/hardwareProbe/encoderAvailable"),
        report.h264HardwareEncoderAvailable);
    settings.setValue(
        QStringLiteral(
            "media/hardwareProbe/cpuNv12InputSupported"),
        report.h264HardwareEncoderCpuNv12InputSupported);
    settings.setValue(
        QStringLiteral(
            "media/hardwareProbe/d3d11InputCandidate"),
        report.h264HardwareEncoderD3D11InputCandidate);
    settings.setValue(
        QStringLiteral("media/hardwareProbe/encoderCount"),
        report.h264HardwareEncoderCount);
    settings.setValue(
        QStringLiteral("media/hardwareProbe/descriptions"),
        descriptions);
    settings.setValue(
        QStringLiteral("media/hardwareProbe/warnings"),
        warnings);
}

void ControllerMainWindow::ApplyVideoPipelineSettingsFromUi(
    bool showFeedback,
    const QString& changedSettingName)
{
    if (videoPipelineSettingsBusy_) {
        if (!showFeedback) {
            videoPipelineSettingsApplyPending_ = true;
        }
        return;
    }
    if (!desktopCaptureSelector_ || !videoEncoderSelector_ ||
        !ffmpegHardwareBackendSelector_ ||
        !ffmpegX264PresetSelector_ || !videoDecoderSelector_) {
        return;
    }
    auto* roomEngine = dynamic_cast<app::InProcessSessionEngine*>(
        engine_.get());
    if (!roomEngine) {
        return;
    }

    const QString captureValue =
        desktopCaptureSelector_->currentData().toString();
    const QString encoderValue =
        videoEncoderSelector_->currentData().toString();
    const QString backendValue =
        ffmpegHardwareBackendSelector_->currentData().toString();
    const QString qualityValue =
        ffmpegX264PresetSelector_->currentData().toString();
    const QString decoderValue =
        videoDecoderSelector_->currentData().toString();
    const QSettings currentSettings;
    QString automaticEncoderId;
    const QString captureBackend = captureValue;
    const QJsonObject encoderProfile = app::LoadEncoderBenchmarkProfile(
        currentSettings,
        QString::fromStdString(BuildWindowsHardwareFingerprint()),
        captureBackend, qualityValue, kEncoderBenchmarkPolicyVersion);
    if (encoderProfile.value(QStringLiteral("passed")).toBool()) {
        automaticEncoderId = encoderProfile.value(
            QStringLiteral("bestEncoderId")).toString();
    } else if (encoderValue == QStringLiteral("auto")) {
        automaticEncoderId = QString::fromLatin1(
            kAutomaticEncoderFfmpegHardwareDefault);
    }
    const auto result = roomEngine->ApplyVideoPipelinePreferences(
        DesktopCaptureImplementationFromSetting(captureValue),
        VideoEncoderPreferenceFromSetting(encoderValue),
        EncoderQualityFromSetting(qualityValue),
        FfmpegHardwareBackendFromSetting(backendValue),
        automaticEncoderId.toStdString(),
        VideoDecoderPreferenceFromSetting(decoderValue));
    if (!result.accepted) {
        videoPipelineSettingsApplyPending_ = false;
        QSettings saved;
        const QSignalBlocker captureBlocker(desktopCaptureSelector_);
        const QSignalBlocker encoderBlocker(videoEncoderSelector_);
        const QSignalBlocker backendBlocker(
            ffmpegHardwareBackendSelector_);
        const QSignalBlocker qualityBlocker(
            ffmpegX264PresetSelector_);
        const QSignalBlocker decoderBlocker(videoDecoderSelector_);
        const auto restore = [&saved](QComboBox* selector,
                                     const char* key,
                                     const QString& fallback) {
            const int index = selector->findData(saved.value(
                QString::fromLatin1(key), fallback).toString());
            if (index >= 0) {
                selector->setCurrentIndex(index);
            }
        };
        restore(desktopCaptureSelector_, kDesktopCaptureBackendSetting,
                QStringLiteral("native_dxgi"));
        restore(videoEncoderSelector_, kVideoEncoderPreferenceSetting,
                QStringLiteral("auto"));
        restore(ffmpegHardwareBackendSelector_,
                kFfmpegHardwareBackendSetting,
                QStringLiteral("auto"));
        restore(ffmpegX264PresetSelector_,
                kFfmpegX264PresetSetting,
                QStringLiteral("medium"));
        restore(videoDecoderSelector_, kVideoDecoderPreferenceSetting,
                QStringLiteral("auto"));
        UpdateVideoPipelineSettingsAvailability(engine_->Snapshot());
        if (showFeedback) {
            RemoteCToast::Show(
                this,
                QStringLiteral("无法应用媒体设置：%1")
                    .arg(QString::fromStdString(result.errorMessage)),
                RemoteCToast::Tone::kError);
        }
        return;
    }

    QSettings settings;
    settings.setValue(QString::fromLatin1(
                          kDesktopCaptureBackendSetting),
                      captureValue);
    settings.setValue(QString::fromLatin1(
                          kVideoEncoderPreferenceSetting),
                      encoderValue);
    settings.setValue(QString::fromLatin1(
                          kFfmpegHardwareBackendSetting),
                      backendValue);
    settings.setValue(QString::fromLatin1(kFfmpegX264PresetSetting),
                      qualityValue);
    settings.setValue(QString::fromLatin1(
                          kVideoDecoderPreferenceSetting),
                      decoderValue);
    videoPipelineSettingsApplyPending_ = false;
    if (showFeedback) {
        RemoteCToast::Show(
            this,
            changedSettingName.isEmpty()
                ? QStringLiteral(
                      "远程桌面媒体设置已更新，将从下一次共享开始生效")
                : QStringLiteral(
                      "%1已更新，将从下一次共享开始生效")
                      .arg(changedSettingName),
            RemoteCToast::Tone::kSuccess);
    }
}

void ControllerMainWindow::UpdateVideoPipelineSettingsAvailability(
    const SessionEngineSnapshot& snapshot)
{
    if (!desktopCaptureSelector_ || !videoEncoderSelector_ ||
        !ffmpegHardwareBackendSelector_ ||
        !ffmpegX264PresetSelector_ || !videoDecoderSelector_) {
        return;
    }
    const bool cameraVideoActive =
        snapshot.localCamera != LocalCameraState::kOff ||
        std::any_of(
            snapshot.room.members.begin(),
            snapshot.room.members.end(),
            [](const RoomMemberSnapshot& member) {
                return member.cameraPublishing;
            });
    const bool directVideoActive =
        snapshot.state == SessionEngineState::kStarting ||
        snapshot.state == SessionEngineState::kConnecting ||
        snapshot.state == SessionEngineState::kAwaitingLocalApproval ||
        snapshot.state == SessionEngineState::kActive ||
        snapshot.state == SessionEngineState::kStopping;
    const bool wasBusy = videoPipelineSettingsBusy_;
    videoPipelineSettingsBusy_ =
        snapshot.room.screenShareState != RoomScreenShareState::kIdle ||
        cameraVideoActive || directVideoActive;
    const bool enabled = !videoPipelineSettingsBusy_;
    desktopCaptureSelector_->setEnabled(enabled);
    videoEncoderSelector_->setEnabled(enabled);
    ffmpegX264PresetSelector_->setEnabled(enabled);
    videoDecoderSelector_->setEnabled(enabled);
    ffmpegHardwareBackendSelector_->setEnabled(
        enabled &&
        ffmpegHardwareBackendSelector_->count() > 1 &&
        videoEncoderSelector_->currentData().toString() ==
            QStringLiteral("ffmpeg_hardware"));
    if (wasBusy && !videoPipelineSettingsBusy_ &&
        videoPipelineSettingsApplyPending_) {
        QTimer::singleShot(0, this, [this] {
            ApplyVideoPipelineSettingsFromUi(false);
        });
    }
}

void ControllerMainWindow::UpdateLocalMediaDevicesUi(
    const SessionEngineSnapshot& snapshot)
{
    if (!cameraDeviceSelector_ ||
        !microphoneDeviceSelector_ ||
        !speakerDeviceSelector_) {
        return;
    }
    const auto& media = snapshot.localMediaDevices;
    const bool activityChanged =
        !mediaActivitySnapshotSeen_ ||
        displayedMicrophoneState_ != snapshot.localMicrophone ||
        displayedRoomAudioPlaybackMuted_ !=
            snapshot.roomAudioPlaybackMuted;
    if (media.revision == mediaDeviceRevision_ &&
        !activityChanged) {
        return;
    }
    mediaActivitySnapshotSeen_ = true;
    displayedMicrophoneState_ = snapshot.localMicrophone;
    displayedRoomAudioPlaybackMuted_ =
        snapshot.roomAudioPlaybackMuted;
    const bool hadPreviousMediaSnapshot =
        mediaDeviceSnapshotSeen_;
    mediaDeviceSnapshotSeen_ = true;
    mediaDeviceRevision_ = media.revision;

    const auto deviceName =
        [](const MediaDeviceCategorySnapshot& category,
           const std::string& deviceId,
           const QString& emptyName) {
            if (deviceId.empty()) {
                return emptyName;
            }
            if (deviceId == category.activeDeviceId &&
                !category.activeDeviceName.empty()) {
                const QString resolved = QString::fromStdString(
                    category.activeDeviceName);
                return deviceId == kSystemDefaultMediaDeviceId
                    ? QStringLiteral("%1（系统默认）").arg(resolved)
                    : resolved;
            }
            if (deviceId == kSystemDefaultMediaDeviceId) {
                return category.activeDeviceName.empty()
                    ? QStringLiteral("跟随系统默认")
                    : QStringLiteral("跟随系统默认（当前：%1）")
                          .arg(QString::fromStdString(
                              category.activeDeviceName));
            }
            const auto found = std::find_if(
                category.devices.begin(),
                category.devices.end(),
                [&deviceId](const MediaDeviceDescriptor& device) {
                    return device.id == deviceId;
                });
            return found == category.devices.end()
                ? QStringLiteral("已断开的设备")
                : QString::fromStdString(found->name);
        };
    const auto rebuild =
        [&deviceName, &media](
            QComboBox* selector,
            const MediaDeviceCategorySnapshot& category) {
            const QSignalBlocker blocker(selector);
            selector->clear();
            const bool defaultActive =
                category.activeDeviceId ==
                kSystemDefaultMediaDeviceId;
            selector->addItem(
                defaultActive
                    ? (category.activeDeviceName.empty()
                           ? QStringLiteral(
                                 "跟随系统默认（当前）")
                           : QStringLiteral(
                                 "跟随系统默认（当前：%1）")
                                 .arg(QString::fromStdString(
                                     category.activeDeviceName)))
                    : QStringLiteral("跟随系统默认"),
                QStringLiteral("default"));
            for (const auto& device : category.devices) {
                QString label =
                    QString::fromStdString(device.name);
                if (device.id == category.activeDeviceId) {
                    label += QStringLiteral("（当前）");
                }
                selector->addItem(
                    label,
                    QString::fromStdString(device.id));
            }
            const QString preferredId =
                QString::fromStdString(
                    category.preferredDeviceId.empty()
                        ? std::string(
                              kSystemDefaultMediaDeviceId)
                        : category.preferredDeviceId);
            int preferredIndex =
                selector->findData(preferredId);
            if (preferredIndex < 0) {
                selector->addItem(
                    QStringLiteral("首选设备已断开"),
                    preferredId);
                preferredIndex = selector->count() - 1;
            }
            selector->setCurrentIndex(preferredIndex);
            selector->setEnabled(
                !media.refreshing &&
                category.state !=
                MediaDeviceSelectionState::kSwitching);
            selector->setToolTip(deviceName(
                category, category.activeDeviceId,
                QStringLiteral("当前未启用")));
        };

    rebuild(cameraDeviceSelector_, media.camera);
    rebuild(microphoneDeviceSelector_, media.microphone);
    rebuild(speakerDeviceSelector_, media.speaker);

    const auto completePending =
        [this](
            const MediaDeviceCategorySnapshot& category,
            QString& pendingId,
            const char* settingKey,
            const QString& deviceLabel) {
            if (pendingId.isEmpty()) {
                return;
            }
            const QString preferred =
                QString::fromStdString(
                    category.preferredDeviceId);
            if (category.state ==
                    MediaDeviceSelectionState::kReady &&
                preferred == pendingId) {
                QSettings settings;
                settings.setValue(
                    QString::fromLatin1(settingKey),
                    pendingId);
                RemoteCToast::Show(
                    this,
                    QStringLiteral("%1已切换")
                        .arg(deviceLabel),
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
                RemoteCToast::Show(
                    this,
                    QStringLiteral("%1切换失败：%2")
                        .arg(deviceLabel, error),
                    RemoteCToast::Tone::kError);
                pendingId.clear();
                return;
            }
            if (category.state !=
                    MediaDeviceSelectionState::kSwitching &&
                preferred != pendingId) {
                RemoteCToast::Show(
                    this,
                    QStringLiteral("%1切换已取消")
                        .arg(deviceLabel),
                    RemoteCToast::Tone::kInformation);
                pendingId.clear();
            }
        };
    const bool cameraHadPending =
        !pendingCameraDeviceId_.isEmpty();
    const bool microphoneHadPending =
        !pendingMicrophoneDeviceId_.isEmpty();
    const bool speakerHadPending =
        !pendingSpeakerDeviceId_.isEmpty();
    completePending(
        media.camera, pendingCameraDeviceId_,
        kCameraDeviceSetting, QStringLiteral("摄像头"));
    completePending(
        media.microphone, pendingMicrophoneDeviceId_,
        kMicrophoneDeviceSetting, QStringLiteral("麦克风"));
    completePending(
        media.speaker, pendingSpeakerDeviceId_,
        kSpeakerDeviceSetting, QStringLiteral("扬声器"));

    if (refreshMediaDevicesButton_) {
        const bool switching =
            media.camera.state ==
                MediaDeviceSelectionState::kSwitching ||
            media.microphone.state ==
                MediaDeviceSelectionState::kSwitching ||
            media.speaker.state ==
                MediaDeviceSelectionState::kSwitching;
        refreshMediaDevicesButton_->setEnabled(
            !media.refreshing && !switching);
    }
    if (mediaDeviceStatusLabel_) {
        QStringList operations;
        if (media.refreshing) {
            operations << QStringLiteral("正在刷新设备");
        }
        if (media.camera.state ==
            MediaDeviceSelectionState::kSwitching) {
            operations << QStringLiteral("正在切换摄像头");
        }
        if (media.microphone.state ==
            MediaDeviceSelectionState::kSwitching) {
            operations << QStringLiteral("正在切换麦克风");
        }
        if (media.speaker.state ==
            MediaDeviceSelectionState::kSwitching) {
            operations << QStringLiteral("正在切换扬声器");
        }
        if (!operations.isEmpty()) {
            mediaDeviceStatusLabel_->setText(
                operations.join(QStringLiteral("   ·   ")) +
                QStringLiteral("，请稍候…"));
        } else {
            QStringList lines;
            lines << QStringLiteral("摄像头：%1")
                         .arg(deviceName(
                             media.camera,
                             media.camera.activeDeviceId,
                             QStringLiteral("尚未开启")));
            const QString microphoneName = deviceName(
                media.microphone,
                media.microphone.activeDeviceId,
                QStringLiteral("未找到可用设备"));
            switch (snapshot.localMicrophone) {
            case LocalMicrophoneState::kStarting:
                lines << QStringLiteral("正在开启麦克风：%1")
                             .arg(microphoneName);
                break;
            case LocalMicrophoneState::kPublishing:
                lines << QStringLiteral("麦克风正在使用：%1")
                             .arg(microphoneName);
                break;
            case LocalMicrophoneState::kStopping:
                lines << QStringLiteral("正在关闭麦克风：%1")
                             .arg(microphoneName);
                break;
            case LocalMicrophoneState::kFailed:
                lines << QStringLiteral(
                             "麦克风不可用（选择：%1）")
                             .arg(microphoneName);
                break;
            default:
                lines << QStringLiteral(
                             "麦克风已关闭（开启后使用：%1）")
                             .arg(microphoneName);
                break;
            }
            const QString speakerName = deviceName(
                media.speaker,
                media.speaker.activeDeviceId,
                QStringLiteral("未找到可用设备"));
            lines << (snapshot.roomAudioPlaybackMuted
                ? QStringLiteral("扬声器已静音（恢复后输出到：%1）")
                      .arg(speakerName)
                : QStringLiteral("声音输出到：%1")
                      .arg(speakerName));
            mediaDeviceStatusLabel_->setText(
                lines.join(QStringLiteral("   ·   ")));
        }
    }

    const auto unavailableText =
        [](const MediaDeviceCategorySnapshot& category,
           const QString& label) {
            if (category.state !=
                    MediaDeviceSelectionState::kUnavailable &&
                category.state !=
                    MediaDeviceSelectionState::kFailed) {
                return QString{};
            }
            return QStringLiteral("%1不可用，请重新选择设备")
                .arg(label);
        };
    QStringList deviceErrors;
    if (!cameraHadPending) {
        const QString error =
            unavailableText(media.camera, QStringLiteral("摄像头"));
        if (!error.isEmpty()) {
            deviceErrors << error;
        }
    }
    if (!microphoneHadPending) {
        const QString error = unavailableText(
            media.microphone, QStringLiteral("麦克风"));
        if (!error.isEmpty()) {
            deviceErrors << error;
        }
    }
    if (!speakerHadPending) {
        const QString error =
            unavailableText(media.speaker, QStringLiteral("扬声器"));
        if (!error.isEmpty()) {
            deviceErrors << error;
        }
    }
    const QString errorKey = deviceErrors.join(QLatin1Char('|'));
    if (errorKey.isEmpty()) {
        lastMediaDeviceErrorKey_.clear();
    } else if (!hadPreviousMediaSnapshot) {
        lastMediaDeviceErrorKey_ = errorKey;
    } else if (hadPreviousMediaSnapshot &&
               errorKey != lastMediaDeviceErrorKey_) {
        lastMediaDeviceErrorKey_ = errorKey;
        RemoteCToast::Show(
            this, deviceErrors.join(QStringLiteral("；")),
            RemoteCToast::Tone::kError);
    }
}
