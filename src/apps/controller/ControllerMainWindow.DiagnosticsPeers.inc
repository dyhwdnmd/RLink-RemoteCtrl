// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

    QStringList overviewLines;
    QStringList iceLines;
    QStringList outboundLines;
    QStringList inboundLines;
    QStringList dataChannelLines;
    QVector<DiagnosticsCard> connectionCards;
    QVector<DiagnosticsCard> outboundCards;
    QVector<DiagnosticsCard> inboundCards;
    QVector<DiagnosticsCard> dataChannelCards;

    for (const auto& peer : diagnostics.peerConnections) {
        const QString peerName =
            peer.peerDeviceId.empty()
                ? QString::fromStdString(peer.pairId)
                : QString::fromStdString(peer.peerDeviceId);
        const QString peerHeader =
            QStringLiteral("对端：%1").arg(
                peerName.isEmpty() ? QStringLiteral("未知成员") : peerName);
        const QString peerKey =
            peer.pairId.empty()
                ? (peerName.isEmpty() ? QStringLiteral("unknown")
                                      : peerName)
                : QString::fromStdString(peer.pairId);
        const auto& stats = peer.stats;
        const auto& transport = stats.transport;
        if (!transport.collected) {
            overviewLines
                << QStringLiteral("%1\n正在等待第一份 WebRTC Stats…")
                       .arg(peerHeader);
            connectionCards.push_back(DiagnosticsCard{
                peerKey,
                QStringLiteral("连接正在建立"),
                peerName,
                {{QStringLiteral("state"),
                  QStringLiteral("状态"),
                  QStringLiteral("正在等待第一份 WebRTC Stats"),
                  "warning",
                  true}}});
            continue;
        }
        QString collectedTime = QStringLiteral("未知");
        if (transport.timestampMs > 0) {
            collectedTime =
                QDateTime::fromMSecsSinceEpoch(transport.timestampMs)
                    .toString(QStringLiteral("HH:mm:ss"));
        }
        overviewLines
            << QStringLiteral(
                   "%1\n↑ %2 · ↓ %3 · 累计发送 %4 · 累计接收 %5 · RTT %6 ms · %7")
                   .arg(peerHeader)
                   .arg(FormatBitrate(transport.sendBitrateBps))
                   .arg(FormatBitrate(transport.receiveBitrateBps))
                   .arg(FormatByteCount(transport.bytesSent))
                   .arg(FormatByteCount(transport.bytesReceived))
                   .arg(transport.currentRoundTripTimeMs, 0, 'f', 1)
                   .arg(collectedTime);

        QString availableBandwidth =
            QStringLiteral("可用上行：%1 · 可用下行：%2")
                .arg(transport.availableOutgoingBitrateBps == 0
                         ? QStringLiteral("未报告")
                         : FormatBitrate(
                               transport.availableOutgoingBitrateBps))
                .arg(transport.availableIncomingBitrateBps == 0
                         ? QStringLiteral("未报告")
                         : FormatBitrate(
                               transport.availableIncomingBitrateBps));
        iceLines
            << QStringLiteral(
                   "%1\n%2 · 候选对 %3 · ICE %4 · DTLS %5 · 角色 %6\n"
                   "本地：%7\n远端：%8\n%9\n"
                   "TLS %10 · DTLS %11 · SRTP %12 · 候选对切换 %13 次")
                   .arg(peerHeader)
                   .arg(RouteDisplayName(transport.routeType))
                   .arg(QString::fromStdString(
                       transport.candidatePairState))
                   .arg(QString::fromStdString(transport.iceState))
                   .arg(QString::fromStdString(transport.dtlsState))
                   .arg(QString::fromStdString(transport.iceRole))
                   .arg(CandidateDisplayText(transport.localCandidate))
                   .arg(CandidateDisplayText(transport.remoteCandidate))
                   .arg(availableBandwidth)
                   .arg(QString::fromStdString(transport.tlsVersion))
                   .arg(QString::fromStdString(transport.dtlsCipher))
                   .arg(QString::fromStdString(transport.srtpCipher))
                   .arg(transport.selectedCandidatePairChanges);

        DiagnosticsCard connectionCard;
        connectionCard.key = peerKey;
        connectionCard.title = QStringLiteral("连接与安全");
        connectionCard.subtitle = peerName;
        connectionCard.chips = {
            {QStringLiteral("route"), QStringLiteral("连接路径"),
             RouteDisplayName(transport.routeType), "good", false},
            {QStringLiteral("rtt"), QStringLiteral("当前 RTT"),
             QStringLiteral("%1 ms").arg(
                 transport.currentRoundTripTimeMs, 0, 'f', 1),
             transport.currentRoundTripTimeMs >= 250.0
                 ? QByteArray("warning")
                 : QByteArray("good"),
             false},
            {QStringLiteral("sendRate"), QStringLiteral("采样窗上行"),
             FormatBitrate(transport.sendBitrateBps) +
                 SampleWindowSuffix(transport.sampleWindowMs),
             "normal", false},
            {QStringLiteral("receiveRate"), QStringLiteral("采样窗下行"),
             FormatBitrate(transport.receiveBitrateBps) +
                 SampleWindowSuffix(transport.sampleWindowMs),
             "normal", false},
            {QStringLiteral("ice"), QStringLiteral("ICE"),
             QString::fromStdString(transport.iceState), "good", false},
            {QStringLiteral("dtls"), QStringLiteral("DTLS"),
             QString::fromStdString(transport.dtlsState), "good", false},
            {QStringLiteral("role"), QStringLiteral("连接角色"),
             QString::fromStdString(transport.iceRole), "normal", false},
            {QStringLiteral("pair"), QStringLiteral("候选对"),
             QString::fromStdString(transport.candidatePairState),
             "good", false},
            {QStringLiteral("local"), QStringLiteral("本地候选"),
             CandidateDisplayText(transport.localCandidate), "normal",
             true},
            {QStringLiteral("remote"), QStringLiteral("远端候选"),
             CandidateDisplayText(transport.remoteCandidate), "normal",
             true},
            {QStringLiteral("availableUp"),
             QStringLiteral("估计可用上行"),
             transport.availableOutgoingBitrateBps == 0
                 ? QStringLiteral("未报告")
                 : FormatBitrate(
                       transport.availableOutgoingBitrateBps),
             "normal", false},
            {QStringLiteral("availableDown"),
             QStringLiteral("估计可用下行"),
             transport.availableIncomingBitrateBps == 0
                 ? QStringLiteral("未报告")
                 : FormatBitrate(
                       transport.availableIncomingBitrateBps),
             "normal", false},
            {QStringLiteral("security"), QStringLiteral("安全套件"),
             QStringLiteral("TLS %1 · DTLS %2 · SRTP %3")
                 .arg(QString::fromStdString(transport.tlsVersion),
                      QString::fromStdString(transport.dtlsCipher),
                      QString::fromStdString(transport.srtpCipher)),
             "normal", true},
            {QStringLiteral("pairChanges"),
             QStringLiteral("累计路径切换"),
             QStringLiteral("%1 次").arg(
                 transport.selectedCandidatePairChanges),
             transport.selectedCandidatePairChanges > 2
                 ? QByteArray("warning")
                 : QByteArray("normal"),
             false},
            {QStringLiteral("discarded"),
             QStringLiteral("累计发送丢弃"),
             QStringLiteral("%1 包 / %2")
                 .arg(transport.packetsDiscardedOnSend)
                 .arg(FormatByteCount(
                     transport.bytesDiscardedOnSend)),
             transport.packetsDiscardedOnSend > 0
                 ? QByteArray("warning")
                 : QByteArray("normal"),
             false}};
        connectionCards.push_back(std::move(connectionCard));

        bool hasOutbound = false;
        bool hasInbound = false;
        for (const auto& stream : stats.rtpStreams) {
            QStringList details;
            details
                << QStringLiteral("[%1] %2")
                       .arg(SlotDisplayName(stream.slot, stream.kind),
                            stream.codec.empty()
                                ? QStringLiteral("未知编码")
                                : QString::fromStdString(stream.codec));
            if (stream.frameWidth > 0 && stream.frameHeight > 0) {
                details
                    << QStringLiteral("%1×%2")
                           .arg(stream.frameWidth)
                           .arg(stream.frameHeight);
            }
            if (stream.framesPerSecond > 0.0) {
                details
                    << QStringLiteral("%1 fps")
                           .arg(stream.framesPerSecond, 0, 'f', 1);
            }
            if (stream.direction == RtpStreamDirection::kOutbound &&
                stream.sourceFramesPerSecond > 0.0) {
                details
                    << QStringLiteral("采集 %1×%2@%3")
                           .arg(stream.sourceWidth)
                           .arg(stream.sourceHeight)
                           .arg(stream.sourceFramesPerSecond, 0, 'f', 1);
            }
            details << QStringLiteral("码率 %1")
                           .arg(FormatBitrate(stream.bitrateBps));
            if (stream.direction == RtpStreamDirection::kOutbound &&
                stream.targetBitrateBps > 0) {
                details
                    << QStringLiteral("目标 %1")
                           .arg(FormatBitrate(
                               stream.targetBitrateBps));
            }
            const std::string& implementation =
                stream.direction == RtpStreamDirection::kOutbound
                    ? stream.encoderImplementation
                    : stream.decoderImplementation;
            if (!implementation.empty()) {
                details
                    << QStringLiteral("%1：%2")
                           .arg(stream.direction ==
                                        RtpStreamDirection::kOutbound
                                    ? QStringLiteral("编码器")
                                    : QStringLiteral("解码器"),
                                QString::fromStdString(implementation));
            }
            details
                << QStringLiteral("累计包 %1 · 累计丢包 %2（%3%）")
                       .arg(stream.packets)
                       .arg(stream.packetsLost)
                       .arg(stream.lossPercent, 0, 'f', 2);
            if (stream.direction == RtpStreamDirection::kOutbound &&
                stream.roundTripTimeMs > 0.0) {
                details
                    << QStringLiteral("RTT %1 ms")
                           .arg(stream.roundTripTimeMs, 0, 'f', 1);
            }
            if (stream.direction == RtpStreamDirection::kInbound) {
                details
                    << QStringLiteral("抖动 %1 ms")
                           .arg(stream.jitterMs, 0, 'f', 1);
                if (stream.kind == "video") {
                    details
                        << QStringLiteral(
                               "已解码 %1 · 关键帧 %2 · 丢帧 %3")
                               .arg(stream.framesDecoded)
                               .arg(stream.keyFrames)
                               .arg(stream.framesDropped);
                    details
                        << QStringLiteral(
                               "最近帧解码 %1 · 采样窗平均 %2 ms · 会话平均 %3 ms")
                               .arg(
                                   LatestFrameTimingText(
                                       stream,
                                       QStringLiteral("解码")))
                               .arg(
                                   stream.windowDecodeTimeMs,
                                   0, 'f', 2)
                               .arg(
                                   stream.averageDecodeTimeMs,
                                   0, 'f', 2);
                    if (stream.freezeCount > 0 ||
                        stream.pauseCount > 0) {
                        details
                            << QStringLiteral(
                                   "冻结 %1 次/%2 ms · 暂停 %3 次/%4 ms")
                                   .arg(stream.freezeCount)
                                   .arg(stream.totalFreezeDurationMs,
                                        0, 'f', 0)
                                   .arg(stream.pauseCount)
                                   .arg(stream.totalPauseDurationMs,
                                        0, 'f', 0);
                    }
                } else if (stream.totalAudioSamples > 0) {
                    details
                        << QStringLiteral(
                               "音频电平 %1 · 隐藏样本 %2/%3 · 隐藏事件 %4")
                               .arg(stream.audioLevel, 0, 'f', 3)
                               .arg(stream.concealedAudioSamples)
                               .arg(stream.totalAudioSamples)
                               .arg(stream.concealmentEvents);
                }
            } else if (stream.kind == "video") {
                details
                    << QStringLiteral(
                           "已编码 %1 · 关键帧 %2 · 最近帧编码 %3 · 采样窗平均 %4 ms · 会话平均 %5 ms")
                           .arg(stream.framesEncoded)
                           .arg(stream.keyFrames)
                           .arg(
                               LatestFrameTimingText(
                                   stream,
                                   QStringLiteral("编码")))
                           .arg(stream.windowEncodeTimeMs, 0, 'f', 2)
                           .arg(stream.averageEncodeTimeMs, 0, 'f', 2);
            }
            if (stream.averageQp > 0.0) {
                details
                    << QStringLiteral("会话平均 QP %1")
                           .arg(stream.averageQp, 0, 'f', 1);
            }
            if (stream.retransmittedPackets > 0 ||
                stream.retransmittedBytes > 0) {
                details
                    << QStringLiteral("累计重传 %1 包/%2")
                           .arg(stream.retransmittedPackets)
                           .arg(FormatByteCount(
                               stream.retransmittedBytes));
            }
            if (stream.nackCount > 0 || stream.pliCount > 0 ||
                stream.firCount > 0) {
                details
                    << QStringLiteral("NACK/PLI/FIR %1/%2/%3")
                           .arg(stream.nackCount)
                           .arg(stream.pliCount)
                           .arg(stream.firCount);
            }
            if (!stream.qualityLimitationReason.empty() &&
                stream.qualityLimitationReason != "none") {
                details
                    << QStringLiteral("质量受限：%1")
                           .arg(QString::fromStdString(
                               stream.qualityLimitationReason));
            }
            if (stream.powerEfficient) {
                details << QStringLiteral("节能硬件路径");
            }
