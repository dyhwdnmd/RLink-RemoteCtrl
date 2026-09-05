// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

void ControllerMainWindow::RefreshDecoderBenchmarkSummary()
{
    if (!decoderBenchmarkSummary_) {
        return;
    }
    const QSettings settings;
    const QString currentFingerprint =
        QString::fromStdString(
            BuildWindowsHardwareFingerprint());
    const bool resultMatchesCurrentHardware =
        settings.value(
            QString::fromLatin1(
                kDecoderBenchmarkCompletedSetting),
            false).toBool() &&
        settings.value(
            QString::fromLatin1(
                kDecoderBenchmarkPolicyVersionSetting),
            0).toInt() == kDecoderBenchmarkPolicyVersion &&
        settings.value(
            QString::fromLatin1(
                kDecoderHardwareFingerprintSetting))
            .toString() == currentFingerprint;
    const QString name = resultMatchesCurrentHardware &&
            settings.value(
                QString::fromLatin1(
                    kDecoderBenchmarkPassedSetting),
                false).toBool()
        ? settings.value(
              QString::fromLatin1(kBestDecoderNameSetting))
              .toString()
        : QString{};
    if (name.isEmpty()) {
        const bool currentNegativeResult =
            resultMatchesCurrentHardware &&
            !settings.value(
                QString::fromLatin1(
                    kDecoderBenchmarkPassedSetting),
                false).toBool();
        decoderBenchmarkSummary_->setText(AdaptBenchmarkHtmlForTheme(QStringLiteral(
            "<table width=\"100%\" cellspacing=\"0\" cellpadding=\"0\">"
            "<tr><td><b style=\"color:#24324b\">%1</b></td>"
            "<td align=\"right\"><span style=\"color:%2\"><b>%3</b></span></td></tr>"
            "<tr><td colspan=\"2\" style=\"padding-top:5px;color:#667085\">%4</td></tr>"
            "</table>")
            .arg(currentNegativeResult
                     ? QStringLiteral("当前使用软件解码")
                     : QStringLiteral("等待首次性能检测"),
                 currentNegativeResult
                     ? QStringLiteral("#a76508")
                     : QStringLiteral("#1769e8"),
                 currentNegativeResult
                     ? QStringLiteral("无合格硬解")
                     : QStringLiteral("待检测"),
                 currentNegativeResult
                     ? QStringLiteral(
                           "本机硬件解码器未达到低延迟要求，自动模式已安全使用 WebRTC FFmpeg。")
                     : QStringLiteral(
                           "程序会在当前硬件环境空闲时自动检测，不影响正常使用。"))));
        return;
    }
    const QDateTime testedAt = QDateTime::fromString(
        settings.value(
            QString::fromLatin1(kBestDecoderTestTimeSetting)).toString(),
        Qt::ISODateWithMs).toLocalTime();
    const QString configuredMode = settings.value(
        QStringLiteral("media/videoDecoderPreference"),
        QStringLiteral("auto")).toString();
    const QString compactBestName = name.startsWith(
            QStringLiteral("FFmpeg D3D11VA"))
        ? QStringLiteral("FFmpeg D3D11VA")
        : name.contains(QStringLiteral("Microsoft H264"))
        ? QStringLiteral("Microsoft H264 MFT")
        : name;
    const QString strategyText =
        configuredMode == QStringLiteral("software")
            ? QStringLiteral("仅软件 · WebRTC FFmpeg")
            : configuredMode == QStringLiteral("hardware")
            ? QStringLiteral("仅硬件 · %1").arg(compactBestName)
            : QStringLiteral("自动 · 优先 %1").arg(compactBestName);
    QString html = QStringLiteral(
        "<table width=\"100%\" cellspacing=\"0\" cellpadding=\"0\">"
        "<tr><td><span style=\"color:#667085\">当前策略</span>"
        "&nbsp;&nbsp;<b style=\"color:#172033\">%1</b></td>"
        "<td align=\"right\"><span style=\"color:#138b57\"><b>检测通过</b></span></td></tr>"
        "<tr><td colspan=\"2\" style=\"padding-top:5px;color:#667085\">"
        "硬解异常时自动回退软件解码；已建立的画面连接不会中途切换。</td></tr>"
        "</table>"
        "<hr style=\"color:#e3e8f1\"/>")
        .arg(strategyText.toHtmlEscaped());
    const QJsonArray candidates = QJsonDocument::fromJson(
        settings.value(QString::fromLatin1(kDecoderCandidatesSetting))
            .toByteArray()).array();
    QList<QJsonObject> orderedCandidates;
    orderedCandidates.reserve(candidates.size());
    for (const auto& value : candidates) {
        orderedCandidates.push_back(value.toObject());
    }
    std::stable_sort(
        orderedCandidates.begin(), orderedCandidates.end(),
        [&name](const QJsonObject& left, const QJsonObject& right) {
            const auto rank = [&name](const QJsonObject& candidate) {
                const QString candidateName =
                    candidate.value(QStringLiteral("name")).toString();
                if (candidateName == name) {
                    return 0;
                }
                if (!candidate.value(QStringLiteral("hardware")).toBool(true)) {
                    return 1;
                }
                return candidate.value(QStringLiteral("passed")).toBool()
                    ? 2 : 3;
            };
            return rank(left) < rank(right);
        });
    bool firstCandidate = true;
    for (const auto& candidate : orderedCandidates) {
        const QString candidateName =
            candidate.value(QStringLiteral("name")).toString();
        const bool hardware =
            candidate.value(QStringLiteral("hardware")).toBool(true);
        const bool passed =
            candidate.value(QStringLiteral("passed")).toBool();
        QString displayName = candidateName;
        if (candidateName.startsWith(QStringLiteral("FFmpeg D3D11VA"))) {
            displayName = QStringLiteral("FFmpeg D3D11VA");
        } else if (candidateName == QStringLiteral("FFmpeg (WebRTC Builtin)")) {
            displayName = QStringLiteral("WebRTC FFmpeg");
        } else if (candidateName.contains(QStringLiteral("Microsoft H264"))) {
            displayName = QStringLiteral("Microsoft H264 MFT");
        }
        const QString kind = hardware
            ? (candidateName.startsWith(QStringLiteral("FFmpeg D3D11VA"))
                   ? QStringLiteral("硬件 · D3D11VA")
                   : candidate.value(QStringLiteral("asynchronous")).toBool()
                   ? QStringLiteral("硬件 · 异步 MFT")
                   : QStringLiteral("硬件 · 同步 MFT"))
            : QStringLiteral("软件 · CPU");
        const bool selected = hardware && candidateName == name;
        const QString status = selected
            ? QStringLiteral("最优硬解")
            : !hardware
            ? QStringLiteral("软件备用")
            : passed ? QStringLiteral("可用") : QStringLiteral("未通过");
        const QString statusColor = selected || passed
            ? QStringLiteral("#138b57")
            : hardware ? QStringLiteral("#c12b37")
                       : QStringLiteral("#1769e8");
        const double realtimeAverage = candidate.value(
            QStringLiteral("realtimeAverageLatencyMs")).toDouble();
        const double realtimeP95 = candidate.value(
            QStringLiteral("realtimeP95LatencyMs")).toDouble();
        const double sparseAverage = candidate.value(
            QStringLiteral("sparseAverageLatencyMs")).toDouble();
        const double sparseP95 = candidate.value(
            QStringLiteral("sparseP95LatencyMs")).toDouble();
        if (!firstCandidate) {
            html += QStringLiteral("<hr style=\"color:#e8ecf3\"/>");
        }
        firstCandidate = false;
        html += QStringLiteral(
            "<table width=\"100%\" cellspacing=\"0\" cellpadding=\"0\">"
            "<tr><td><b style=\"color:#24324b\">%1</b>"
            "&nbsp;&nbsp;<span style=\"color:#7a8496\">%2</span></td>"
            "<td align=\"right\"><span style=\"color:%3\"><b>%4</b></span></td></tr>"
            "<tr><td colspan=\"2\" style=\"padding-top:5px;color:#667085\">"
            "连续 60 FPS&nbsp;&nbsp;平均 %5 ms&nbsp;·&nbsp;P95 %6 ms"
            "&nbsp;&nbsp;&nbsp;&nbsp;稀疏 5 FPS&nbsp;&nbsp;平均 %7 ms&nbsp;·&nbsp;P95 %8 ms"
            "</td></tr>")
            .arg(displayName.toHtmlEscaped(), kind, statusColor, status)
            .arg(realtimeAverage, 0, 'f', 2)
            .arg(realtimeP95, 0, 'f', 2)
            .arg(sparseAverage, 0, 'f', 2)
            .arg(sparseP95, 0, 'f', 2);
        if (!passed && hardware) {
            QString reason = QStringLiteral("未达到远程控制低延迟要求");
            if (sparseP95 > 180.0) {
                reason = QStringLiteral("稀疏画面响应过慢（P95 %1 ms）")
                             .arg(sparseP95, 0, 'f', 2);
            } else if (realtimeP95 > 80.0) {
                reason = QStringLiteral("连续画面响应过慢（P95 %1 ms）")
                             .arg(realtimeP95, 0, 'f', 2);
            }
            html += QStringLiteral(
                "<tr><td colspan=\"2\" style=\"padding-top:4px;color:#a76508\">"
                "原因：%1</td></tr>")
                .arg(reason);
        }
        html += QStringLiteral("</table>");
    }
    if (candidates.isEmpty()) {
        const double average = settings.value(
            QString::fromLatin1(kBestDecoderAverageSetting)).toDouble();
        const double p95 = settings.value(
            QString::fromLatin1(kBestDecoderP95Setting)).toDouble();
        html += QStringLiteral(
            "<b style=\"color:#24324b\">%1</b>"
            "&nbsp;&nbsp;<span style=\"color:#138b57\"><b>最优硬解</b></span><br/>"
            "<span style=\"color:#667085\">平均 %2 ms · P95 %3 ms</span>")
            .arg(compactBestName.toHtmlEscaped())
            .arg(average, 0, 'f', 2)
            .arg(p95, 0, 'f', 2);
    }
    if (testedAt.isValid()) {
        html += QStringLiteral(
            "<div style=\"margin-top:10px;color:#98a2b3\">最近检测：%1</div>")
            .arg(testedAt.toString(
                QStringLiteral("yyyy-MM-dd HH:mm")));
    }
    decoderBenchmarkSummary_->setText(AdaptBenchmarkHtmlForTheme(html));
    decoderBenchmarkSummary_->setToolTip(QStringLiteral(
        "自动模式优先使用通过连续与稀疏帧低延迟检测的硬件解码器；运行失败后永久切换到软件解码，直到下次会话。"));
}

void ControllerMainWindow::RefreshDecoderHardwareSelectionAvailability()
{
    if (!videoDecoderSelector_) {
        return;
    }
    const QSettings settings;
    const QString currentFingerprint = QString::fromStdString(
        BuildWindowsHardwareFingerprint());
    const bool available =
        settings.value(
            QString::fromLatin1(kDecoderBenchmarkCompletedSetting),
            false).toBool() &&
        settings.value(
            QString::fromLatin1(kDecoderBenchmarkPassedSetting),
            false).toBool() &&
        settings.value(
            QString::fromLatin1(
                kDecoderBenchmarkPolicyVersionSetting),
            0).toInt() == kDecoderBenchmarkPolicyVersion &&
        settings.value(
            QString::fromLatin1(kDecoderHardwareFingerprintSetting))
                .toString() == currentFingerprint;
    const int hardwareIndex = videoDecoderSelector_->findData(
        QStringLiteral("hardware"));
    if (hardwareIndex < 0) {
        return;
    }
    if (auto* model = qobject_cast<QStandardItemModel*>(
            videoDecoderSelector_->model())) {
        if (auto* item = model->item(hardwareIndex)) {
            item->setEnabled(available);
        }
    }
    videoDecoderSelector_->setItemData(
        hardwareIndex,
        available
            ? QStringLiteral(
                  "当前硬件已通过原生 D3D11 输出和低延迟检测。")
            : QStringLiteral(
                  "当前机器尚未通过硬件解码低延迟检测，不能选择仅硬件。"),
        Qt::ToolTipRole);
    if (!available &&
        videoDecoderSelector_->currentData().toString() ==
            QStringLiteral("hardware")) {
        const QSignalBlocker blocker(videoDecoderSelector_);
        videoDecoderSelector_->setCurrentIndex(std::max(
            0, videoDecoderSelector_->findData(
                   QStringLiteral("auto"))));
        QSettings writableSettings;
        writableSettings.setValue(
            QString::fromLatin1(kVideoDecoderPreferenceSetting),
            QStringLiteral("auto"));
        writableSettings.sync();
    }
}

void ControllerMainWindow::StartDecoderBenchmark(bool manualRequest)
{
    if (decoderBenchmarkProcess_) {
        if (manualRequest) {
            RemoteCToast::Show(
                this, QStringLiteral("最优解码器检测正在进行"),
                RemoteCToast::Tone::kInformation);
        }
        return;
    }
    if (encoderBenchmarkProcess_) {
        if (manualRequest) {
            RemoteCToast::Show(
                this, QStringLiteral("请等待编码器检测完成"),
                RemoteCToast::Tone::kInformation);
        } else {
            QTimer::singleShot(
                std::chrono::seconds(10), this,
                [this] { StartDecoderBenchmark(false); });
        }
        return;
    }
    if (engine_) {
        const auto snapshot = engine_->Snapshot();
        if (snapshot.room.screenShareState !=
            RoomScreenShareState::kIdle) {
            if (manualRequest) {
                RemoteCToast::Show(
                    this,
                    QStringLiteral("请结束当前屏幕共享后再检测解码器"),
                    RemoteCToast::Tone::kInformation);
            } else {
                QTimer::singleShot(
                    std::chrono::seconds(30), this,
                    [this] { StartDecoderBenchmark(false); });
            }
            return;
        }
    }

    decoderBenchmarkManualRequest_ = manualRequest;
    decoderBenchmarkHardwareFingerprint_ =
        QString::fromStdString(
            BuildWindowsHardwareFingerprint());
    decoderBenchmarkProcess_ = new QProcess(this);
    decoderBenchmarkProcess_->setProcessChannelMode(
        QProcess::SeparateChannels);
    if (decoderBenchmarkButton_) {
        decoderBenchmarkButton_->setEnabled(false);
        decoderBenchmarkButton_->setText(QStringLiteral("正在检测…"));
    }
    if (decoderBenchmarkSummary_) {
        decoderBenchmarkSummary_->setText(
            QStringLiteral("正在后台对比软件与硬件解码器，请稍候…"));
    }
    connect(
        decoderBenchmarkProcess_,
        qOverload<int, QProcess::ExitStatus>(&QProcess::finished),
        this,
        [this](int exitCode, QProcess::ExitStatus) {
            FinishDecoderBenchmark(exitCode);
        });
    connect(decoderBenchmarkProcess_, &QProcess::errorOccurred,
            this, [this](QProcess::ProcessError error) {
                if (error == QProcess::FailedToStart) {
                    FinishDecoderBenchmark(-1);
                }
            });
    decoderBenchmarkProcess_->start(
        QCoreApplication::applicationFilePath(),
        {QStringLiteral("--decoder-optimal-probe")});

    QPointer<QProcess> guardedProcess(decoderBenchmarkProcess_);
    QTimer::singleShot(
        std::chrono::seconds(60), this,
        [guardedProcess] {
            if (guardedProcess &&
                guardedProcess->state() != QProcess::NotRunning) {
                guardedProcess->kill();
            }
        });
}

void ControllerMainWindow::FinishDecoderBenchmark(int exitCode)
{
    auto* process = decoderBenchmarkProcess_;
    if (!process) {
        return;
    }
    decoderBenchmarkProcess_ = nullptr;
    const QByteArray output = process->readAllStandardOutput().trimmed();
    const QString processError = process->errorString();
    process->deleteLater();

    QJsonDocument document;
    const QList<QByteArray> lines = output.split('\n');
    for (auto it = lines.crbegin(); it != lines.crend(); ++it) {
        QJsonParseError parseError;
        const auto candidate = QJsonDocument::fromJson(
            it->trimmed(), &parseError);
        if (parseError.error == QJsonParseError::NoError &&
            candidate.isObject()) {
            document = candidate;
            break;
        }
    }
    const QJsonObject result = document.object();
    const bool resultDocumentComplete =
        !result.isEmpty() &&
        result.contains(QStringLiteral("passed"));
    const QString currentHardwareFingerprint =
        QString::fromStdString(
            BuildWindowsHardwareFingerprint());
    const QString benchmarkHardwareFingerprint =
        std::exchange(
            decoderBenchmarkHardwareFingerprint_, QString{});
    const bool hardwareEnvironmentChanged =
        resultDocumentComplete &&
        (!benchmarkHardwareFingerprint.isEmpty() &&
         benchmarkHardwareFingerprint !=
             currentHardwareFingerprint);
    const bool completedNormally =
        resultDocumentComplete &&
        !hardwareEnvironmentChanged;
    const bool passed = completedNormally &&
        result.value(QStringLiteral("passed")).toBool();
    const QString bestName =
        result.value(QStringLiteral("bestDecoderName")).toString();
    if (completedNormally) {
        QSettings settings;
        settings.setValue(
            QString::fromLatin1(
                kDecoderHardwareFingerprintSetting),
            benchmarkHardwareFingerprint);
        settings.setValue(
            QString::fromLatin1(
                kDecoderBenchmarkCompletedSetting),
            true);
        settings.setValue(
            QString::fromLatin1(
                kDecoderBenchmarkPolicyVersionSetting),
            kDecoderBenchmarkPolicyVersion);
        settings.setValue(
            QString::fromLatin1(
                kDecoderBenchmarkPassedSetting),
            passed);
        settings.setValue(
            QString::fromLatin1(kBestDecoderTestTimeSetting),
            QDateTime::currentDateTimeUtc().toString(
                Qt::ISODateWithMs));
        settings.setValue(
            QString::fromLatin1(kDecoderCandidatesSetting),
            QJsonDocument(
                result.value(QStringLiteral("candidates"))
                    .toArray())
                .toJson(QJsonDocument::Compact));
        if (!passed) {
            settings.remove(
                QString::fromLatin1(kBestDecoderNameSetting));
            settings.remove(
                QString::fromLatin1(
                    kBestDecoderAverageSetting));
            settings.remove(
                QString::fromLatin1(kBestDecoderP95Setting));
        }
        settings.sync();
    }
    if (passed && !bestName.isEmpty()) {
        QSettings settings;
        settings.setValue(
            QString::fromLatin1(kBestDecoderNameSetting), bestName);
        settings.setValue(
            QString::fromLatin1(kBestDecoderAverageSetting),
            result.value(
                QStringLiteral("bestAverageLatencyMs")).toDouble());
        settings.setValue(
            QString::fromLatin1(kBestDecoderP95Setting),
            result.value(QStringLiteral("bestP95LatencyMs")).toDouble());
        settings.sync();
        if (auto* roomEngine =
                dynamic_cast<app::InProcessSessionEngine*>(engine_.get())) {
            roomEngine->SetPreferredHardwareDecoderName(
                bestName.toStdString());
        }
        RefreshDecoderBenchmarkSummary();
        if (decoderBenchmarkManualRequest_) {
            RemoteCToast::Show(
                this, QStringLiteral("解码器性能检测完成"),
                RemoteCToast::Tone::kSuccess);
        }
    } else {
        const QString reportedError =
            result.value(QStringLiteral("error")).toString();
        if (decoderBenchmarkSummary_) {
            decoderBenchmarkSummary_->setText(
                hardwareEnvironmentChanged
                    ? QStringLiteral(
                          "检测期间硬件、驱动或远程桌面环境发生变化，结果已丢弃。")
                    : completedNormally
                    ? QStringLiteral(
                          "当前硬件环境没有通过检测的硬件解码器，自动模式继续使用 FFmpeg。")
                    : QStringLiteral("检测失败：%1")
                          .arg(!reportedError.isEmpty()
                                   ? reportedError
                                   : processError));
        }
        if (decoderBenchmarkManualRequest_) {
            RemoteCToast::Show(
                this,
                hardwareEnvironmentChanged
                    ? QStringLiteral(
                          "硬件环境已变化，本次检测结果未保存")
                    : completedNormally
                    ? QStringLiteral(
                          "未找到可用硬件解码器，继续使用 FFmpeg")
                    : QStringLiteral("最优解码器检测失败"),
                completedNormally
                    ? RemoteCToast::Tone::kInformation
                    : RemoteCToast::Tone::kError);
        }
        if (completedNormally) {
            RefreshDecoderBenchmarkSummary();
        }
    }
    decoderBenchmarkManualRequest_ = false;
    RefreshDecoderHardwareSelectionAvailability();
    if (decoderBenchmarkButton_) {
        decoderBenchmarkButton_->setEnabled(true);
        decoderBenchmarkButton_->setText(
            QStringLiteral("重新检测"));
    }
    if (hardwareEnvironmentChanged) {
        QTimer::singleShot(
            std::chrono::seconds(5), this,
            [this] { StartDecoderBenchmark(false); });
    }
}
