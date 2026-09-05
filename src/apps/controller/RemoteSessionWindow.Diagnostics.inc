// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

    void RemoteSessionWindow::UpdateDiagnostics(
        const PeerConnectionDiagnosticsSnapshot& diagnostics)
    {
        const auto updateSignalBars = [this](int activeBars) {
            if (!networkSignalIndicator_) {
                return;
            }
            networkSignalIndicator_->setProperty(
                "activeBars", std::clamp(activeBars, 0, 3));
            networkSignalIndicator_->update();
            };
        if (sessionEngine_ && binding_.IsRoom()) {
            const auto snapshot = sessionEngine_->Snapshot();
            const auto pair = std::find_if(
                snapshot.roomPeerConnections.begin(),
                snapshot.roomPeerConnections.end(),
                [this](const RoomPeerConnectionSnapshot& current) {
                    return current.pairId ==
                           binding_.roomPairId.toStdString();
                });
            if (pair != snapshot.roomPeerConnections.end() &&
                (pair->state == RoomPeerConnectionState::kDisconnected ||
                 pair->state == RoomPeerConnectionState::kRecovering)) {
                if (connectionStatusLabel_) {
                    connectionStatusLabel_->setText(
                        pair->state == RoomPeerConnectionState::kDisconnected
                            ? QStringLiteral("网络波动，等待恢复")
                            : QStringLiteral("正在恢复 %1/3")
                                  .arg(pair->iceRestartAttempt));
                    connectionStatusLabel_->setToolTip(
                        QStringLiteral("ICE 正在重新协商，画面和控制将在连接恢复后继续"));
                }
                updateSignalBars(0);
                return;
            }
        }
        const auto& transport = diagnostics.stats.transport;
        if (!transport.collected) {
            if (connectionStatusLabel_) {
                connectionStatusLabel_->setText(
                    QStringLiteral("正在采集"));
            }
            updateSignalBars(0);
            return;
        }

        int signalBars = transport.currentRoundTripTimeMs <= 80.0
            ? 3
            : (transport.currentRoundTripTimeMs <= 180.0 ? 2 : 1);
        updateSignalBars(signalBars);

        if (connectionStatusLabel_) {
            connectionStatusLabel_->setText(
                SessionRouteText(transport.routeType));
            connectionStatusLabel_->setToolTip(
                QStringLiteral(
                    "ICE %1 · DTLS %2 · ↑ %3 · ↓ %4")
                .arg(QString::fromStdString(transport.iceState),
                    QString::fromStdString(transport.dtlsState),
                    SessionBitrateText(transport.sendBitrateBps),
                    SessionBitrateText(transport.receiveBitrateBps)));
        }
        if (latencyLabel_) {
            latencyLabel_->setText(
                QStringLiteral("延迟  %1 ms")
                .arg(transport.currentRoundTripTimeMs, 0, 'f', 0));
        }

        const RtpStreamStatsSnapshot* inboundScreen = nullptr;
        for (const auto& stream : diagnostics.stats.rtpStreams) {
            if (stream.direction == RtpStreamDirection::kInbound &&
                stream.kind == "video" &&
                (stream.slot == kScreenMainVideoSlot ||
                    inboundScreen == nullptr)) {
                inboundScreen = &stream;
                if (stream.slot == kScreenMainVideoSlot) {
                    break;
                }
            }
        }
        if (!inboundScreen) {
            if (codecLabel_) {
                codecLabel_->setText(QStringLiteral("等待视频流"));
            }
            if (resolutionLabel_) {
                resolutionLabel_->setText(
                    QStringLiteral("等待画面"));
            }
            if (hudFrameRateLabel_) {
                hudFrameRateLabel_->setText(QStringLiteral("-- FPS"));
            }
            return;
        }

        if (inboundScreen->lossPercent > 5.0) {
            signalBars = (std::min)(signalBars, 1);
        }
        else if (inboundScreen->lossPercent > 1.0) {
            signalBars = (std::min)(signalBars, 2);
        }
        updateSignalBars(signalBars);

        if (codecLabel_) {
            const QString codec = inboundScreen->codec.empty()
                ? QStringLiteral("未知编码")
                : QString::fromStdString(
                    inboundScreen->codec);
            const QString decoder =
                inboundScreen->decoderImplementation.empty()
                ? QStringLiteral("解码器待报告")
                : QString::fromStdString(
                    inboundScreen->decoderImplementation);
            QString compactDecoder = decoder;
            if (decoder.contains(QStringLiteral("FFmpeg"),
                Qt::CaseInsensitive)) {
                compactDecoder = QStringLiteral("FFmpeg");
            }
            else if (decoder.contains(QStringLiteral("Hardware"),
                Qt::CaseInsensitive) ||
                decoder.contains(QStringLiteral("DXVA"),
                    Qt::CaseInsensitive)) {
                compactDecoder = QStringLiteral("硬件解码");
            }
            else if (decoder.contains(QStringLiteral("Software"),
                Qt::CaseInsensitive)) {
                compactDecoder = QStringLiteral("软件解码");
            }
            codecLabel_->setText(
                QStringLiteral("%1 · %2").arg(codec, compactDecoder));
            codecLabel_->setToolTip(
                QStringLiteral(
                    "解码器 %1 · 采样窗接收 %2 · 累计丢包 %3% · 当前抖动估计 %4 ms · "
                    "最近一帧解码 %5 ms · 会话平均解码 %6 ms")
                .arg(decoder)
                .arg(SessionBitrateText(
                    inboundScreen->bitrateBps))
                .arg(inboundScreen->lossPercent, 0, 'f', 2)
                .arg(inboundScreen->jitterMs, 0, 'f', 1)
                .arg(inboundScreen->latestFrameTimeMs, 0, 'f', 3)
                .arg(inboundScreen->averageDecodeTimeMs, 0, 'f', 3));
        }
        if (resolutionLabel_) {
            resolutionLabel_->setText(
                inboundScreen->frameWidth > 0 &&
                inboundScreen->frameHeight > 0
                ? QStringLiteral("%1 × %2")
                .arg(inboundScreen->frameWidth)
                .arg(inboundScreen->frameHeight)
                : QStringLiteral("分辨率待报告"));
        }
        if (hudFrameRateLabel_) {
            hudFrameRateLabel_->setText(
                QStringLiteral("%1 FPS")
                .arg(inboundScreen->framesPerSecond, 0, 'f', 0));
            hudFrameRateLabel_->setToolTip(
                QStringLiteral("当前实际接收并解码的屏幕帧率"));
        }
    }
