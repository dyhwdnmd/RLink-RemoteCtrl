// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

                inboundCards.push_back(std::move(streamCard));
            }

            const QString line =
                QStringLiteral("%1\n%2")
                    .arg(peerHeader, details.join(QStringLiteral(" · ")));
            if (stream.direction == RtpStreamDirection::kOutbound) {
                outboundLines << line;
                hasOutbound = true;
            } else {
                inboundLines << line;
                hasInbound = true;
            }
        }
        if (!hasOutbound) {
            outboundLines
                << QStringLiteral("%1\n当前没有发送中的 RTP 媒体流。")
                       .arg(peerHeader);
            outboundCards.push_back(DiagnosticsCard{
                peerKey + QStringLiteral("/outbound-empty"),
                QStringLiteral("暂无发送媒体"),
                peerName,
                {{QStringLiteral("state"), QStringLiteral("状态"),
                  QStringLiteral("当前没有发送中的 RTP 媒体流"),
                  "normal", true}}});
        }
        if (!hasInbound) {
            inboundLines
                << QStringLiteral("%1\n当前没有接收中的 RTP 媒体流。")
                       .arg(peerHeader);
            inboundCards.push_back(DiagnosticsCard{
                peerKey + QStringLiteral("/inbound-empty"),
                QStringLiteral("暂无接收媒体"),
                peerName,
                {{QStringLiteral("state"), QStringLiteral("状态"),
                  QStringLiteral("当前没有接收中的 RTP 媒体流"),
                  "normal", true}}});
        }

        if (stats.dataChannels.empty()) {
            dataChannelLines
                << QStringLiteral("%1\n当前没有 DataChannel Stats。")
                       .arg(peerHeader);
            dataChannelCards.push_back(DiagnosticsCard{
                peerKey + QStringLiteral("/data-empty"),
                QStringLiteral("暂无数据通道"),
                peerName,
                {{QStringLiteral("state"), QStringLiteral("状态"),
                  QStringLiteral("当前没有 DataChannel Stats"),
                  "normal", true}}});
        } else {
            for (const auto& channel : stats.dataChannels) {
                dataChannelLines
                    << QStringLiteral(
                           "%1\n%2 · %3 · ↑ %4 / ↓ %5 · 缓冲 %6\n"
                           "累计 ↑ %7（%8 条）/ ↓ %9（%10 条）")
                           .arg(peerHeader)
                           .arg(QString::fromStdString(channel.label))
                           .arg(QString::fromStdString(channel.state))
                           .arg(FormatBitrate(channel.sendBitrateBps))
                           .arg(FormatBitrate(channel.receiveBitrateBps))
                           .arg(FormatByteCount(
                               channel.bufferedAmountBytes))
                           .arg(FormatByteCount(channel.bytesSent))
                           .arg(channel.messagesSent)
                           .arg(FormatByteCount(channel.bytesReceived))
                           .arg(channel.messagesReceived);

                DiagnosticsCard channelCard;
                channelCard.key =
                    peerKey + QLatin1Char('/') +
                    QString::fromStdString(channel.label);
                channelCard.title =
                    QString::fromStdString(channel.label);
                channelCard.subtitle = peerName;
                channelCard.chips = {
                    {QStringLiteral("state"), QStringLiteral("状态"),
                     QString::fromStdString(channel.state),
                     channel.state == "open" ? QByteArray("good")
                                             : QByteArray("warning"),
                     false},
                    {QStringLiteral("protocol"), QStringLiteral("协议"),
                     channel.protocol.empty()
                         ? QStringLiteral("SCTP")
                         : QString::fromStdString(channel.protocol),
                     "normal", false},
                    {QStringLiteral("sendRate"),
                     QStringLiteral("采样窗上行"),
                     FormatBitrate(channel.sendBitrateBps) +
                         SampleWindowSuffix(channel.sampleWindowMs),
                     "normal",
                     false},
                    {QStringLiteral("receiveRate"),
                     QStringLiteral("采样窗下行"),
                     FormatBitrate(channel.receiveBitrateBps) +
                         SampleWindowSuffix(channel.sampleWindowMs),
                     "normal",
                     false},
                    {QStringLiteral("buffer"), QStringLiteral("发送缓冲"),
                     FormatByteCount(channel.bufferedAmountBytes),
                     channel.bufferedAmountBytes >= 4 * 1024 * 1024
                         ? QByteArray("warning")
                         : QByteArray("good"),
                     false},
                    {QStringLiteral("messages"),
                     QStringLiteral("累计消息"),
                     QStringLiteral("↑ %1 · ↓ %2")
                         .arg(channel.messagesSent)
                         .arg(channel.messagesReceived),
                     "normal", false},
                    {QStringLiteral("bytes"), QStringLiteral("累计流量"),
                     QStringLiteral("↑ %1 · ↓ %2")
                         .arg(FormatByteCount(channel.bytesSent),
                              FormatByteCount(channel.bytesReceived)),
                     "normal", true}};
                dataChannelCards.push_back(std::move(channelCard));
            }
        }
    }

    if (diagnostics.peerConnections.empty()) {
        overviewLines
            << QStringLiteral("当前没有成员对 P2P 连接。");
    }
    if (iceLines.isEmpty()) {
        iceLines << QStringLiteral("尚未采集到 ICE 与传输层状态。");
    }
    if (outboundLines.isEmpty()) {
        outboundLines << QStringLiteral("尚未采集到发送媒体流。");
    }
    if (inboundLines.isEmpty()) {
        inboundLines << QStringLiteral("尚未采集到接收媒体流。");
    }
    if (dataChannelLines.isEmpty()) {
        dataChannelLines << QStringLiteral("尚未采集到 DataChannel 状态。");
    }

    const QString overviewText =
        overviewLines.join(QStringLiteral("\n\n"));
    const QString iceText = iceLines.join(QStringLiteral("\n\n"));
    const QString outboundText =
        outboundLines.join(QStringLiteral("\n\n"));
    const QString inboundText =
        inboundLines.join(QStringLiteral("\n\n"));
    const QString dataChannelText =
        dataChannelLines.join(QStringLiteral("\n\n"));

    QVector<DiagnosticsSection> sections;
    if (!connectionCards.isEmpty()) {
        sections.push_back(DiagnosticsSection{
            QStringLiteral("connection"),
            QStringLiteral("连接概览"),
            QStringLiteral("传输路径、候选地址、带宽与安全协商"),
            std::move(connectionCards)});
    }
    if (!outboundCards.isEmpty()) {
        sections.push_back(DiagnosticsSection{
            QStringLiteral("outbound"),
            QStringLiteral("发送媒体"),
            QStringLiteral("本机发往房间成员的音视频流"),
            std::move(outboundCards)});
    }
    if (!inboundCards.isEmpty()) {
        sections.push_back(DiagnosticsSection{
            QStringLiteral("inbound"),
            QStringLiteral("接收媒体"),
            QStringLiteral("从房间成员接收并解码的音视频流"),
            std::move(inboundCards)});
    }
    if (!dataChannelCards.isEmpty()) {
        sections.push_back(DiagnosticsSection{
            QStringLiteral("data"),
            QStringLiteral("数据通道"),
            QStringLiteral("输入、控制、文件与遥测通道"),
            std::move(dataChannelCards)});
    }
    if (statsCardsWidget_) {
        static_cast<DiagnosticsCardsWidget*>(statsCardsWidget_)
            ->SetSections(
                sections,
                QStringLiteral("当前没有成员对 P2P 连接"));
    }

    if (diagnosticsCopyTextRequested_) {
        statsDebugCopyText_ =
            QStringList{
                QStringLiteral("【WebRTC 采样与逐帧遥测】"),
                QStringLiteral("传输快照：\n%1").arg(overviewText),
                QStringLiteral("ICE 与安全：\n%1").arg(iceText),
                QStringLiteral("发送媒体流：\n%1").arg(outboundText),
                QStringLiteral("接收媒体流：\n%1").arg(inboundText),
                QStringLiteral("DataChannel：\n%1").arg(dataChannelText),
                QStringLiteral("鼠标与键盘：\n%1").arg(inputDebugText)}
                .join(QStringLiteral("\n\n"));
    }
}
