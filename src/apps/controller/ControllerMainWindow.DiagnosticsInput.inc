// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

void ControllerMainWindow::RefreshDiagnosticsUi()
{
    if (QThread::currentThread() != thread()) {
        QMetaObject::invokeMethod(
            this, [this] { RefreshDiagnosticsUi(); },
            Qt::QueuedConnection);
        return;
    }
    if (!engine_) {
        return;
    }

    const bool diagnosticsPageVisible =
        debugPage_ && debugPage_->isVisibleTo(this);
    const bool sessionHudNeeded =
        remoteSessionWindow_ && remoteSessionBinding_;
    if (!diagnosticsPageVisible && !sessionHudNeeded &&
        !screenFrameRateLogEnabled_ &&
        !diagnosticsCopyTextRequested_) {
        return;
    }

    const auto diagnostics = engine_->Diagnostics();
    if (screenFrameRateLogEnabled_) {
        AppendScreenFrameRateLog(diagnostics);
    }
    // The session HUD is independent from the optional diagnostics page.
    // Feed the explicitly bound transport before the optional diagnostics
    // page early return so latency, resolution and FPS always remain live.
    if (remoteSessionWindow_ && remoteSessionBinding_) {
        for (const auto& peer : diagnostics.peerConnections) {
            const bool directPeer =
                remoteSessionBinding_->IsDirect() && peer.pairId.empty();
            const bool roomPeer =
                remoteSessionBinding_->IsRoom() &&
                remoteSessionBinding_->roomPairId ==
                    QString::fromStdString(peer.pairId);
            if (directPeer || roomPeer) {
                remoteSessionWindow_->UpdateDiagnostics(peer);
                break;
            }
        }
    }
    if ((!diagnosticsPageVisible && !diagnosticsCopyTextRequested_) ||
        debugValueLabels_.isEmpty()) {
        return;
    }

    const auto setInputDebugValue =
        [this](const QString& key, const QString& value,
               const char* tone = "normal") {
            auto* label = debugValueLabels_.value(key, nullptr);
            if (!label) {
                return;
            }
            label->setText(value);
            label->setToolTip(value);
            if (label->property("tone").toByteArray() != tone) {
                label->setProperty("tone", tone);
                label->style()->unpolish(label);
                label->style()->polish(label);
            }
        };
    const auto& input = diagnostics.remoteInput;
    const std::uint32_t dragSampleRateHz =
        QSettings().value(
            QString::fromLatin1(
                kDragPointerSampleRateSetting),
            QVariant::fromValue(240u)).toUInt();
    QString inputDebugText;
    if (!input.enabled) {
        const QString disabled =
            QStringLiteral("统计已关闭，远控输入功能仍正常运行。");
        for (const QString& key : {
                 QStringLiteral("inputMoveGenerated"),
                 QStringLiteral("inputMoveDispatched"),
                 QStringLiteral("inputImmediateEvents"),
                 QStringLiteral("inputPacketsSent"),
                 QStringLiteral("inputPacketsReceived"),
                 QStringLiteral("inputInjected")}) {
            setInputDebugValue(key, disabled, "muted");
        }
        setInputDebugValue(
            QStringLiteral("inputMovePolicy"),
            QStringLiteral(
                "高精度独立调度 · 普通移动 120–240 Hz · 按住拖动主动采样 %1 Hz")
                .arg(dragSampleRateHz),
            "good");
        inputDebugText =
            QStringLiteral("鼠标与键盘统计：已关闭");
    } else {
        const auto rateAndTotal =
            [](std::uint64_t rate, std::uint64_t total) {
                return QStringLiteral("%1 次/秒 · 累计 %2")
                    .arg(static_cast<qulonglong>(rate))
                    .arg(static_cast<qulonglong>(total));
            };
        const auto& rate = input.perSecond;
        const auto& total = input.totals;
        const QString movePolicy =
            QStringLiteral(
                "高精度独立调度 · 当前上限 %1 Hz · 采样窗 %2 ms\n"
                "普通移动按画面帧率两倍调度；按住拖动以 %3 Hz 主动读取 Windows 真实光标；按下、松开、滚轮和键盘立即发送。")
                .arg(input.moveDispatchRateLimitHz)
                .arg(input.sampleWindowMs)
                .arg(dragSampleRateHz);
        const QString generatedMoves = rateAndTotal(
            rate.generatedMouseMoves,
            total.generatedMouseMoves);
        const QString dispatchedMoves =
            QStringLiteral(
                "%1\n合并覆盖 %2\n"
                "发送间隔 平均 %3 ms · P95 %4 ms · 最大 %5 ms")
                .arg(
                    rateAndTotal(
                        rate.dispatchedMouseMoves,
                        total.dispatchedMouseMoves),
                    rateAndTotal(
                        rate.coalescedMouseMoves,
                        total.coalescedMouseMoves))
                .arg(
                    static_cast<double>(
                        input.moveDispatchIntervalAverageUs) /
                        1000.0,
                    0, 'f', 3)
                .arg(
                    static_cast<double>(
                        input.moveDispatchIntervalP95Us) /
                        1000.0,
                    0, 'f', 3)
                .arg(
                    static_cast<double>(
                        input.moveDispatchIntervalMaximumUs) /
                        1000.0,
                    0, 'f', 3);
        const QString immediateEvents =
            QStringLiteral(
                "鼠标按键 %1/%2 · 滚轮 %3/%4 · 键盘 %5/%6（每秒/累计）")
                .arg(
                    static_cast<qulonglong>(
                        rate.generatedMouseButtons))
                .arg(
                    static_cast<qulonglong>(
                        total.generatedMouseButtons))
                .arg(
                    static_cast<qulonglong>(
                        rate.generatedMouseWheels))
                .arg(
                    static_cast<qulonglong>(
                        total.generatedMouseWheels))
                .arg(
                    static_cast<qulonglong>(rate.generatedKeys))
                .arg(
                    static_cast<qulonglong>(total.generatedKeys));
        const QString packetsSent =
            QStringLiteral(
                "快速队列 %1/%2 · 可靠队列 %3/%4\n"
                "发送完成 %5/%6 · 失败 %7/%8（每秒/累计）")
                .arg(
                    static_cast<qulonglong>(rate.fastPacketsQueued))
                .arg(
                    static_cast<qulonglong>(total.fastPacketsQueued))
                .arg(
                    static_cast<qulonglong>(
                        rate.reliablePacketsQueued))
                .arg(
                    static_cast<qulonglong>(
                        total.reliablePacketsQueued))
                .arg(
                    static_cast<qulonglong>(rate.packetsSent))
                .arg(
                    static_cast<qulonglong>(total.packetsSent))
                .arg(
                    static_cast<qulonglong>(
                        rate.packetsSendFailed))
                .arg(
                    static_cast<qulonglong>(
                        total.packetsSendFailed));
        const QString packetsReceived =
            QStringLiteral(
                "收到 %1/%2 · 去重或拒绝 %3/%4（每秒/累计）")
                .arg(
                    static_cast<qulonglong>(rate.packetsReceived))
                .arg(
                    static_cast<qulonglong>(total.packetsReceived))
                .arg(
                    static_cast<qulonglong>(rate.packetsDropped))
                .arg(
                    static_cast<qulonglong>(total.packetsDropped));
        const QString injected =
            QStringLiteral(
                "移动 %1/%2 · 按键 %3/%4 · 滚轮 %5/%6 · 键盘 %7/%8\n"
                "注入失败 %9/%10（每秒/累计）")
                .arg(
                    static_cast<qulonglong>(
                        rate.injectedMouseMoves))
                .arg(
                    static_cast<qulonglong>(
                        total.injectedMouseMoves))
                .arg(
                    static_cast<qulonglong>(
                        rate.injectedMouseButtons))
                .arg(
                    static_cast<qulonglong>(
                        total.injectedMouseButtons))
                .arg(
                    static_cast<qulonglong>(
                        rate.injectedMouseWheels))
                .arg(
                    static_cast<qulonglong>(
                        total.injectedMouseWheels))
                .arg(
                    static_cast<qulonglong>(rate.injectedKeys))
                .arg(
                    static_cast<qulonglong>(total.injectedKeys))
                .arg(
                    static_cast<qulonglong>(
                        rate.injectionFailures))
                .arg(
                    static_cast<qulonglong>(
                        total.injectionFailures));
        setInputDebugValue(
            QStringLiteral("inputMovePolicy"), movePolicy, "good");
        setInputDebugValue(
            QStringLiteral("inputMoveGenerated"), generatedMoves);
        setInputDebugValue(
            QStringLiteral("inputMoveDispatched"), dispatchedMoves);
        setInputDebugValue(
            QStringLiteral("inputImmediateEvents"), immediateEvents);
        setInputDebugValue(
            QStringLiteral("inputPacketsSent"), packetsSent,
            total.packetsSendFailed == 0 ? "good" : "warning");
        setInputDebugValue(
            QStringLiteral("inputPacketsReceived"), packetsReceived);
        setInputDebugValue(
            QStringLiteral("inputInjected"), injected,
            total.injectionFailures == 0 ? "good" : "warning");
        inputDebugText =
            QStringList{
                QStringLiteral("调度：%1").arg(movePolicy),
                QStringLiteral("Qt 移动：%1").arg(generatedMoves),
                QStringLiteral("移动调度：%1").arg(dispatchedMoves),
                QStringLiteral("即时事件：%1").arg(immediateEvents),
                QStringLiteral("发送：%1").arg(packetsSent),
                QStringLiteral("接收：%1").arg(packetsReceived),
                QStringLiteral("注入：%1").arg(injected)}
                .join(QStringLiteral("\n"));
    }
    const auto& cursor = diagnostics.remoteCursor;
    inputDebugText += QStringLiteral(
        "\n\n独立远程光标：%1 · 形状 %2 · 显示器 %3 / 布局 %4\n"
        "发布 位置 %5 / 形状 %6 · 接收 位置 %7 / 形状 %8 · 远端已执行输入 #%9")
        .arg(cursor.publishing ? QStringLiteral("正在采集")
                               : QStringLiteral("未采集"),
             cursor.shapeAvailable ? QStringLiteral("已缓存")
                                   : QStringLiteral("等待"))
        .arg(cursor.displayId)
        .arg(static_cast<qulonglong>(cursor.displayLayoutVersion))
        .arg(static_cast<qulonglong>(cursor.positionMessagesPublished))
        .arg(static_cast<qulonglong>(cursor.shapeMessagesPublished))
        .arg(static_cast<qulonglong>(cursor.positionMessagesReceived))
        .arg(static_cast<qulonglong>(cursor.shapeMessagesReceived))
        .arg(static_cast<qulonglong>(cursor.lastAppliedInputSequence));
