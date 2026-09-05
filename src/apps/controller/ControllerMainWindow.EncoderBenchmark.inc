// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

void ControllerMainWindow::RefreshEncoderBenchmarkSummary()
{
    if (!encoderBenchmarkSummary_) {
        return;
    }
    const QSettings settings;
    const QString fingerprint = QString::fromStdString(
        BuildWindowsHardwareFingerprint());
    const QString captureBackend = settings.value(
        QString::fromLatin1(kDesktopCaptureBackendSetting),
        QStringLiteral("native_dxgi")).toString();
    const QString ffmpegX264Preset = settings.value(
        QString::fromLatin1(kFfmpegX264PresetSetting),
        QStringLiteral("medium")).toString();
    const QJsonObject profile = app::LoadEncoderBenchmarkProfile(
        settings, fingerprint, captureBackend, ffmpegX264Preset,
        kEncoderBenchmarkPolicyVersion);
    if (profile.isEmpty()) {
        encoderBenchmarkSummary_->setText(AdaptBenchmarkHtmlForTheme(QStringLiteral(
            "<b style=\"color:#1769e8\">等待首次性能检测</b><br/>"
            "<span style=\"color:#667085\">检测结果按硬件环境、采集方式和编码质量分别缓存；自动模式在下次共享时应用。</span>")));
        return;
    }
    const bool passed = profile.value(QStringLiteral("passed")).toBool();
    const QString bestId =
        profile.value(QStringLiteral("bestEncoderId")).toString();
    const QString bestName =
        profile.value(QStringLiteral("bestEncoderName")).toString();
    const QString configuredMode = settings.value(
        QString::fromLatin1(kVideoEncoderPreferenceSetting),
        QStringLiteral("auto")).toString();
    const QString inputDescription =
        captureBackend == QStringLiteral("native_dxgi")
            ? QStringLiteral("自研 DXGI 输入")
            : QStringLiteral("libwebrtc CPU 输入");
    const QString testedAt = QDateTime::fromString(
        profile.value(QStringLiteral("testedAtUtc")).toString(),
        Qt::ISODateWithMs).toLocalTime().toString(
            QStringLiteral("yyyy-MM-dd HH:mm"));
    QString html;
    if (passed) {
        html = QStringLiteral(
            "<table width=\"100%\" cellspacing=\"0\" cellpadding=\"0\">"
            "<tr><td><span style=\"color:#667085\">实测最优</span>&nbsp;&nbsp;"
            "<b style=\"color:#172033\">%1</b></td>"
            "<td align=\"right\"><span style=\"color:#138b57\"><b>%2</b></span></td></tr>"
            "<tr><td colspan=\"2\" style=\"padding-top:5px;color:#667085\">"
            "%3 · %4；仅自动模式采用排名，手动选择不受影响。</td></tr></table>")
            .arg(bestName.toHtmlEscaped(),
                 configuredMode == QStringLiteral("auto")
                     ? QStringLiteral("自动采用")
                     : QStringLiteral("检测通过"),
                 inputDescription, testedAt);
    } else {
        // A failed overall result is still useful diagnostic evidence. Keep
        // every candidate and its partial measurements visible instead of
        // replacing the page with a single generic failure sentence.
        html = QStringLiteral(
            "<table width=\"100%\" cellspacing=\"0\" cellpadding=\"0\">"
            "<tr><td><b style=\"color:#a76508\">本次未产生自动选择结果</b></td>"
            "<td align=\"right\"><span style=\"color:#667085\"><b>已保留全部数据</b></span></td></tr>"
            "<tr><td colspan=\"2\" style=\"padding-top:5px;color:#667085\">"
            "%1 · %2；下方仍显示所有候选的实测数据和失败原因。</td></tr></table>")
            .arg(inputDescription, testedAt);
    }
    const QJsonArray candidates =
        profile.value(QStringLiteral("candidates")).toArray();
    QList<QJsonObject> ordered;
    for (const auto& item : candidates) {
        ordered.push_back(item.toObject());
    }
    std::stable_sort(ordered.begin(), ordered.end(),
        [&bestId](const auto& left, const auto& right) {
            const auto rank = [&bestId](const auto& item) {
                if (item.value(QStringLiteral("id")).toString() == bestId) {
                    return 0;
                }
                return item.value(QStringLiteral("passed")).toBool()
                    ? 1 : 2;
            };
            if (rank(left) != rank(right)) {
                return rank(left) < rank(right);
            }
            const double leftFps = left.value(
                QStringLiteral("inputFramesPerSecond")).toDouble();
            const double rightFps = right.value(
                QStringLiteral("inputFramesPerSecond")).toDouble();
            if (std::abs(leftFps - rightFps) > 0.25) {
                return leftFps > rightFps;
            }
            return left.value(QStringLiteral("score")).toDouble(1e9) <
                   right.value(QStringLiteral("score")).toDouble(1e9);
        });
    for (const auto& candidate : ordered) {
        const bool selected =
            candidate.value(QStringLiteral("id")).toString() == bestId;
        const bool candidatePassed =
            candidate.value(QStringLiteral("passed")).toBool();
        html += QStringLiteral(
            "<hr style=\"color:#e8ecf3\"/>"
            "<table width=\"100%\" cellspacing=\"0\" cellpadding=\"0\">"
            "<tr><td><b style=\"color:#24324b\">%1</b></td>"
            "<td align=\"right\"><span style=\"color:%2\"><b>%3</b></span></td></tr>"
            "<tr><td colspan=\"2\" style=\"padding-top:5px;color:#667085\">"
            "平均 %4 ms · P95 %5 ms · CPU %6 ms/帧 · PSNR %7 dB · 输出 %8/%9 帧 · 实测 %10 FPS%11</td></tr></table>")
            .arg(candidate.value(QStringLiteral("name")).toString()
                     .toHtmlEscaped(),
                 selected || candidatePassed
                     ? QStringLiteral("#138b57")
                     : QStringLiteral("#c12b37"),
                 selected ? QStringLiteral("最优")
                          : candidatePassed ? QStringLiteral("可用")
                                            : QStringLiteral("不可用"),
                 QString::number(candidate.value(
                     QStringLiteral("averageLatencyMs")).toDouble(), 'f', 2),
                 QString::number(candidate.value(
                     QStringLiteral("p95LatencyMs")).toDouble(), 'f', 2),
                 QString::number(candidate.value(
                     QStringLiteral("cpuTimePerFrameMs")).toDouble(), 'f', 2),
                 QString::number(candidate.value(
                     QStringLiteral("averageLumaPsnrDb")).toDouble(), 'f', 1),
                 QString::number(candidate.value(
                     QStringLiteral("encodedFrames")).toInt()),
                  QString::number(candidate.value(
                      QStringLiteral("submittedFrames")).toInt()),
                  QString::number(candidate.value(
                      QStringLiteral("inputFramesPerSecond")).toDouble(),
                      'f', 1),
                  candidate.value(QStringLiteral("error")).toString().isEmpty()
                     ? QString{}
                     : QStringLiteral("<br/><span style=\"color:#a76508\">%1</span>")
                           .arg(candidate.value(QStringLiteral("error"))
                               .toString().toHtmlEscaped()));
        const QString rateWarning =
            candidate.value(QStringLiteral("warning")).toString();
        if (!rateWarning.isEmpty()) {
            html.chop(QStringLiteral("</td></tr></table>").size());
            html += QStringLiteral(
                "<br/><span style=\"color:#a76508\">"
                "动态码率兼容性提示：%1</span></td></tr></table>")
                .arg(rateWarning.toHtmlEscaped());
        }
    }
    encoderBenchmarkSummary_->setText(AdaptBenchmarkHtmlForTheme(html));
}

void ControllerMainWindow::StartEncoderBenchmark(bool manualRequest)
{
    if (encoderBenchmarkProcess_) {
        if (manualRequest) {
            RemoteCToast::Show(
                this, QStringLiteral("编码器性能检测正在进行"),
                RemoteCToast::Tone::kInformation);
        }
        return;
    }
    if (decoderBenchmarkProcess_) {
        if (manualRequest) {
            RemoteCToast::Show(
                this, QStringLiteral("请等待解码器检测完成"),
                RemoteCToast::Tone::kInformation);
        } else {
            QTimer::singleShot(std::chrono::seconds(10), this,
                [this] { StartEncoderBenchmark(false); });
        }
        return;
    }
    if (engine_ && engine_->Snapshot().room.screenShareState !=
                       RoomScreenShareState::kIdle) {
        if (manualRequest) {
            RemoteCToast::Show(
                this, QStringLiteral("请结束当前屏幕共享后再检测编码器"),
                RemoteCToast::Tone::kInformation);
        } else {
            QTimer::singleShot(std::chrono::seconds(30), this,
                [this] { StartEncoderBenchmark(false); });
        }
        return;
    }

    const QSettings settings;
    encoderBenchmarkManualRequest_ = manualRequest;
    encoderBenchmarkHardwareFingerprint_ = QString::fromStdString(
        BuildWindowsHardwareFingerprint());
    encoderBenchmarkCaptureBackend_ = settings.value(
        QString::fromLatin1(kDesktopCaptureBackendSetting),
        QStringLiteral("native_dxgi")).toString();
    encoderBenchmarkX264Preset_ = settings.value(
        QString::fromLatin1(kFfmpegX264PresetSetting),
        QStringLiteral("medium")).toString();
    encoderBenchmarkProcess_ = new QProcess(this);
    encoderBenchmarkProcess_->setProcessChannelMode(
        QProcess::SeparateChannels);
    encoderBenchmarkButton_->setEnabled(false);
    encoderBenchmarkButton_->setText(QStringLiteral("正在检测…"));
    encoderBenchmarkSummary_->setText(QStringLiteral(
        "正在独立进程中测试完整输入、转换和编码链路，请稍候…"));
    connect(encoderBenchmarkProcess_,
        qOverload<int, QProcess::ExitStatus>(&QProcess::finished),
        this, [this](int exitCode, QProcess::ExitStatus) {
            FinishEncoderBenchmark(exitCode);
        });
    connect(encoderBenchmarkProcess_, &QProcess::errorOccurred,
        this, [this](QProcess::ProcessError error) {
            if (error == QProcess::FailedToStart) {
                FinishEncoderBenchmark(-1);
            }
        });
    encoderBenchmarkProcess_->start(
        QCoreApplication::applicationFilePath(),
        {QStringLiteral("--encoder-optimal-probe=%1")
             .arg(encoderBenchmarkCaptureBackend_)});
    QPointer<QProcess> guarded(encoderBenchmarkProcess_);
    QTimer::singleShot(std::chrono::seconds(120), this, [guarded] {
        if (guarded && guarded->state() != QProcess::NotRunning) {
            guarded->kill();
        }
    });
}

void ControllerMainWindow::FinishEncoderBenchmark(int exitCode)
{
    Q_UNUSED(exitCode);
    auto* process = encoderBenchmarkProcess_;
    if (!process) {
        return;
    }
    encoderBenchmarkProcess_ = nullptr;
    const QByteArray output = process->readAllStandardOutput().trimmed();
    const QString processError = process->errorString();
    process->deleteLater();
    QJsonDocument document;
    const QList<QByteArray> lines = output.split('\n');
    for (auto it = lines.crbegin(); it != lines.crend(); ++it) {
        QJsonParseError error;
        const auto candidate = QJsonDocument::fromJson(it->trimmed(), &error);
        if (error.error == QJsonParseError::NoError &&
            candidate.isObject()) {
            document = candidate;
            break;
        }
    }
    const QJsonObject result = document.object();
    const QString currentFingerprint = QString::fromStdString(
        BuildWindowsHardwareFingerprint());
    const QSettings currentSettings;
    const QString currentCaptureBackend = currentSettings.value(
        QString::fromLatin1(kDesktopCaptureBackendSetting),
        QStringLiteral("native_dxgi")).toString();
    const QString testedFingerprint = std::exchange(
        encoderBenchmarkHardwareFingerprint_, QString{});
    const QString testedCaptureBackend = std::exchange(
        encoderBenchmarkCaptureBackend_, QString{});
    const QString testedX264Preset = std::exchange(
        encoderBenchmarkX264Preset_, QString{});
    const bool hardwareEnvironmentChanged =
        testedFingerprint != currentFingerprint;
    const bool completed = !result.isEmpty() &&
        result.contains(QStringLiteral("passed")) &&
        !hardwareEnvironmentChanged;
    const bool passed = completed &&
        result.value(QStringLiteral("passed")).toBool();
    if (completed) {
        QSettings settings;
        QJsonObject profile;
        profile.insert(QStringLiteral("hardwareFingerprint"),
                       testedFingerprint);
        profile.insert(QStringLiteral("captureBackend"),
                       testedCaptureBackend);
        profile.insert(QStringLiteral("quality"), testedX264Preset);
        profile.insert(QStringLiteral("policyVersion"),
                       kEncoderBenchmarkPolicyVersion);
        profile.insert(QStringLiteral("completed"), true);
        profile.insert(QStringLiteral("passed"), passed);
        profile.insert(QStringLiteral("testedAtUtc"),
                       QDateTime::currentDateTimeUtc().toString(
                           Qt::ISODateWithMs));
        profile.insert(QStringLiteral("candidates"),
                       result.value(QStringLiteral("candidates")).toArray());
        if (passed) {
            profile.insert(
                QStringLiteral("bestEncoderId"),
                result.value(QStringLiteral("bestEncoderId")).toString());
            profile.insert(
                QStringLiteral("bestEncoderName"),
                result.value(QStringLiteral("bestEncoderName")).toString());
        }
        app::SaveEncoderBenchmarkProfile(settings, std::move(profile));
        // Keep the last profile in the former keys so an older executable can
        // still read its most recent benchmark after a temporary downgrade.
        settings.setValue(
            QString::fromLatin1(kEncoderHardwareFingerprintSetting),
            testedFingerprint);
        settings.setValue(
            QString::fromLatin1(kEncoderCaptureBackendSetting),
            testedCaptureBackend);
        settings.setValue(
            QString::fromLatin1(kEncoderX264PresetSetting),
            testedX264Preset);
        settings.setValue(
            QString::fromLatin1(kEncoderBenchmarkCompletedSetting), true);
        settings.setValue(
            QString::fromLatin1(kEncoderBenchmarkPassedSetting), passed);
        settings.setValue(
            QString::fromLatin1(kEncoderBenchmarkPolicyVersionSetting),
            kEncoderBenchmarkPolicyVersion);
        settings.setValue(
            QString::fromLatin1(kBestEncoderTestTimeSetting),
            QDateTime::currentDateTimeUtc());
        settings.setValue(
            QString::fromLatin1(kEncoderCandidatesSetting),
            QJsonDocument(result.value(QStringLiteral("candidates"))
                              .toArray())
                .toJson(QJsonDocument::Compact));
        if (passed) {
            settings.setValue(
                QString::fromLatin1(kBestEncoderIdSetting),
                result.value(QStringLiteral("bestEncoderId")).toString());
            settings.setValue(
                QString::fromLatin1(kBestEncoderNameSetting),
                result.value(QStringLiteral("bestEncoderName")).toString());
        } else {
            settings.remove(QString::fromLatin1(kBestEncoderIdSetting));
            settings.remove(QString::fromLatin1(kBestEncoderNameSetting));
        }
        settings.sync();
    }
    RefreshEncoderBenchmarkSummary();
    const bool testedProfileIsCurrent =
        testedCaptureBackend == currentCaptureBackend &&
        testedX264Preset == currentSettings.value(
            QString::fromLatin1(kFfmpegX264PresetSetting),
            QStringLiteral("medium")).toString();
    if (passed && testedProfileIsCurrent && videoEncoderSelector_ &&
        videoEncoderSelector_->currentData().toString() ==
            QStringLiteral("auto")) {
        ApplyVideoPipelineSettingsFromUi(false);
    }
    if (encoderBenchmarkManualRequest_) {
        RemoteCToast::Show(
            this,
            hardwareEnvironmentChanged
                ? QStringLiteral("环境已变化，本次编码检测结果未保存")
                : passed
                ? QStringLiteral("编码器检测完成，自动模式将在下次共享采用最优结果")
                : completed
                ? QStringLiteral("编码器检测完成，已保留所有候选结果")
                : QStringLiteral("编码器性能检测失败：%1")
                      .arg(processError),
            passed ? RemoteCToast::Tone::kSuccess
                   : RemoteCToast::Tone::kInformation);
    }
    encoderBenchmarkManualRequest_ = false;
    encoderBenchmarkButton_->setEnabled(true);
    encoderBenchmarkButton_->setText(QStringLiteral("重新检测"));
    if (hardwareEnvironmentChanged) {
        QTimer::singleShot(std::chrono::seconds(5), this,
            [this] { StartEncoderBenchmark(false); });
    } else if (!completed && encoderBenchmarkSummary_) {
        encoderBenchmarkSummary_->setText(QStringLiteral("检测失败：%1")
            .arg(processError.toHtmlEscaped()));
    }
}
