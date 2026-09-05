// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

            DiagnosticsCard streamCard;
            streamCard.key =
                peerKey + QLatin1Char('/') +
                QString::fromStdString(
                    stream.statsId.empty() ? stream.mid
                                           : stream.statsId);
            streamCard.title =
                QStringLiteral("%1%2")
                    .arg(SlotDisplayName(stream.slot, stream.kind),
                         stream.direction ==
                                  RtpStreamDirection::kOutbound
                              ? QStringLiteral("发送")
                              : QStringLiteral("接收"));
            streamCard.subtitle = peerName;
            streamCard.chips = {
                {QStringLiteral("codec"), QStringLiteral("编码格式"),
                 stream.codec.empty()
                     ? QStringLiteral("未知")
                     : QString::fromStdString(stream.codec),
                 "normal", false},
                {QStringLiteral("rate"), QStringLiteral("采样窗码率"),
                 FormatBitrate(stream.bitrateBps) +
                     SampleWindowSuffix(stream.sampleWindowMs),
                 stream.bitrateBps > 0 ? QByteArray("good")
                                       : QByteArray("normal"),
                 false},
                {QStringLiteral("implementation"),
                 stream.direction == RtpStreamDirection::kOutbound
                     ? QStringLiteral("编码器")
                     : QStringLiteral("解码器"),
                 implementation.empty()
                     ? QStringLiteral("未报告")
                     : QString::fromStdString(implementation),
                 stream.powerEfficient ? QByteArray("good")
                                       : QByteArray("normal"),
                 false},
                {QStringLiteral("packets"), QStringLiteral("累计数据包"),
                 QStringLiteral("%1").arg(stream.packets), "normal",
                 false},
                {QStringLiteral("loss"), QStringLiteral("累计丢包"),
                 QStringLiteral("%1 包 · %2%")
                     .arg(stream.packetsLost)
                     .arg(stream.lossPercent, 0, 'f', 2),
                 stream.lossPercent >= 3.0
                     ? QByteArray("warning")
                     : QByteArray("normal"),
                 false},
                {QStringLiteral("retransmit"), QStringLiteral("累计重传"),
                 QStringLiteral("%1 包 · %2")
                     .arg(stream.retransmittedPackets)
                     .arg(FormatByteCount(
                         stream.retransmittedBytes)),
                 stream.retransmittedPackets > 0
                     ? QByteArray("warning")
                     : QByteArray("normal"),
                 false}};

            if (stream.kind == "video") {
                streamCard.chips
                    << DiagnosticsChip{
                           QStringLiteral("resolution"),
                           QStringLiteral("分辨率"),
                           stream.frameWidth > 0 &&
                                   stream.frameHeight > 0
                               ? QStringLiteral("%1 × %2")
                                     .arg(stream.frameWidth)
                                     .arg(stream.frameHeight)
                               : QStringLiteral("未报告"),
                           "normal", false}
                    << DiagnosticsChip{
                           QStringLiteral("fps"),
                           QStringLiteral("WebRTC当前帧率"),
                           QStringLiteral("%1 FPS").arg(
                               stream.framesPerSecond, 0, 'f', 1),
                           stream.framesPerSecond > 0.0
                               ? QByteArray("good")
                               : QByteArray("normal"),
                           false}
                    << DiagnosticsChip{
                           QStringLiteral("frames"),
                           stream.direction ==
                                   RtpStreamDirection::kOutbound
                               ? QStringLiteral("累计编码帧")
                               : QStringLiteral("累计解码帧"),
                           stream.direction ==
                                   RtpStreamDirection::kOutbound
                               ? QStringLiteral(
                                     "编码 %1 · 发送 %2 · 关键帧 %3")
                                     .arg(stream.framesEncoded)
                                     .arg(stream.framesSent)
                                     .arg(stream.keyFrames)
                               : QStringLiteral(
                                     "%1 · 关键帧 %2 · 丢帧 %3")
                                     .arg(stream.framesDecoded)
                                     .arg(stream.keyFrames)
                                     .arg(stream.framesDropped),
                           "normal", false}
                    << DiagnosticsChip{
                           QStringLiteral("qp"),
                           QStringLiteral("最近一帧 QP"),
                           stream.latestFrameQpAvailable
                               ? QStringLiteral("%1").arg(
                                     stream.latestFrameQp)
                               : QStringLiteral("未报告"),
                           "normal", false}
                    << DiagnosticsChip{
                           QStringLiteral("feedback"),
                           QStringLiteral("累计反馈"),
                           QStringLiteral("NACK %1 · PLI %2 · FIR %3")
                               .arg(stream.nackCount)
                               .arg(stream.pliCount)
                               .arg(stream.firCount),
                           stream.pliCount > 10
                               ? QByteArray("warning")
                               : QByteArray("normal"),
                           true};
            }
            if (stream.direction == RtpStreamDirection::kOutbound) {
                streamCard.chips
                    << DiagnosticsChip{
                           QStringLiteral("target"),
                           QStringLiteral("WebRTC目标码率"),
                           stream.targetBitrateBps > 0
                               ? FormatBitrate(
                                     stream.targetBitrateBps)
                               : QStringLiteral("未报告"),
                           "normal", false}
                    << DiagnosticsChip{
                           QStringLiteral("senderPolicy"),
                           QStringLiteral("发送策略上限"),
                           stream.configuredOutputWidth > 0 &&
                                   stream.configuredOutputHeight > 0
                               ? QStringLiteral("%1 × %2 · %3 FPS")
                                     .arg(stream.configuredOutputWidth)
                                     .arg(stream.configuredOutputHeight)
                                     .arg(stream.configuredMaxFrameRate)
                               : QStringLiteral("未配置"),
                           "normal", false}
                    << DiagnosticsChip{
                           QStringLiteral("senderStartBitrate"),
                           QStringLiteral("WebRTC启动码率"),
                           stream.configuredStartBitrateBps > 0
                               ? FormatBitrate(
                                     stream.configuredStartBitrateBps)
                               : QStringLiteral("未配置"),
                           "normal", false}
                    << DiagnosticsChip{
                           QStringLiteral("senderBitrateCeiling"),
                           QStringLiteral("应用码率上限"),
                           stream.configuredMaxBitrateBps > 0
                               ? FormatBitrate(
                                     stream.configuredMaxBitrateBps)
                               : QStringLiteral("未配置"),
                           "normal", false}
                    << DiagnosticsChip{
                           QStringLiteral("senderBitrateBootstrap"),
                           QStringLiteral("启动码率引导"),
                           QStringLiteral(
                               "重启 %1 · SetBitrate %2/%3 · 分配探测 %4 · "
                               "2 Mbps探测底座 %5%6")
                               .arg(stream.mediaReadyBitrateRestarts)
                               .arg(stream.bitrateBootstrapSuccesses)
                               .arg(stream.bitrateBootstrapAttempts)
                               .arg(stream.allocationProbePulses)
                               .arg(stream.bitrateProbeFloorActive
                                        ? QStringLiteral("活动")
                                        : QStringLiteral("已释放%1次").arg(
                                              stream.bitrateProbeFloorReleases))
                               .arg(stream.bitrateBootstrapError.empty()
                                        ? QString()
                                        : QStringLiteral(" · %1").arg(
                                              QString::fromStdString(
                                                  stream.bitrateBootstrapError))),
                           stream.bitrateBootstrapError.empty()
                               ? QByteArray("good")
                               : QByteArray("warning"),
                           true}
                    << DiagnosticsChip{
                           QStringLiteral("rtt"),
                           QStringLiteral("当前媒体 RTT"),
                           QStringLiteral("%1 ms").arg(
                               stream.roundTripTimeMs, 0, 'f', 1),
                           stream.roundTripTimeMs >= 250.0
                                ? QByteArray("warning")
                                : QByteArray("normal"),
                           false};
                if (stream.progressiveBitrateCeilingEnabled) {
                    QString ceilingState = QStringLiteral("稳定中");
                    if (stream.bitrateCeilingStatus == "waiting_for_stats") {
                        ceilingState = QStringLiteral("等待带宽统计");
                    } else if (stream.bitrateCeilingStatus == "stable") {
                        ceilingState = QStringLiteral("已稳定");
                    } else if (stream.bitrateCeilingStatus ==
                               "probe_pending") {
                        ceilingState = QStringLiteral("等待探测结果");
                    } else if (stream.bitrateCeilingStatus == "cooldown") {
                        ceilingState = QStringLiteral("失败冷却");
                    }
                    QString ceilingDetail = QStringLiteral(
                        "当前 %1 · 目标 %2 · 平滑估计 %3 · 稳定样本 %4/3 · %5")
                        .arg(FormatBitrate(
                            stream.appliedPeerConnectionMaxBitrateBps))
                        .arg(FormatBitrate(
                            stream.desiredPeerConnectionMaxBitrateBps))
                        .arg(stream.smoothedOutgoingCapacityBps > 0
                                 ? FormatBitrate(
                                       stream.smoothedOutgoingCapacityBps)
                                 : QStringLiteral("未报告"))
                        .arg(stream.bitrateCeilingStableSamples)
                        .arg(ceilingState);
                    if (stream.bitrateCeilingCooldownRemainingMs > 0) {
                        ceilingDetail += QStringLiteral(" · 剩余 %1 秒")
                            .arg(
                                stream.bitrateCeilingCooldownRemainingMs /
                                1000.0,
                                0,
                                'f',
                                1);
                    }
                    if (!stream.bitrateCeilingError.empty()) {
                        ceilingDetail += QStringLiteral("\n错误：%1").arg(
                            QString::fromStdString(
                                stream.bitrateCeilingError));
                    }
                    streamCard.chips << DiagnosticsChip{
                        QStringLiteral("progressiveBitrateCeiling"),
                        QStringLiteral("渐进码率上限"),
                        ceilingDetail,
                        stream.bitrateCeilingLastProbeFailed ||
                                !stream.bitrateCeilingError.empty()
                            ? QByteArray("warning")
                            : QByteArray("good"),
                        true};
                }
                if (stream.adaptiveNetworkFrameRateEnabled) {
                    QString frameRateState = QStringLiteral("稳定");
                    if (stream.adaptiveNetworkFrameRateStatus ==
                        "waiting_for_activity") {
                        frameRateState = QStringLiteral("等待桌面活动");
                    } else if (stream.adaptiveNetworkFrameRateStatus ==
                               "idle_suspended") {
                        frameRateState = QStringLiteral("静止暂停判断");
                    } else if (stream.adaptiveNetworkFrameRateStatus ==
                               "startup_grace") {
                        frameRateState = QStringLiteral("启动保护期");
                    } else if (stream.adaptiveNetworkFrameRateStatus ==
                               "waiting_for_capacity") {
                        frameRateState = QStringLiteral("等待带宽估计");
                    } else if (stream.adaptiveNetworkFrameRateStatus ==
                               "reducing") {
                        frameRateState = QStringLiteral("弱网降帧判断");
                    } else if (stream.adaptiveNetworkFrameRateStatus ==
                               "recovering") {
                        frameRateState = QStringLiteral("带宽恢复判断");
                    }
                    QString frameRateDetail = QStringLiteral(
                        "用户请求 %1 FPS · 当前连接有效 %2 FPS · %3 · "
                        "平滑容量 %4 · 降帧样本 %5/2 · 恢复样本 %6/5")
                        .arg(stream.configuredMaxFrameRate)
                        .arg(stream.effectiveNetworkFrameRate)
                        .arg(frameRateState)
                        .arg(stream.adaptiveNetworkFrameRateCapacityBps > 0
                                 ? FormatBitrate(
                                       stream.adaptiveNetworkFrameRateCapacityBps)
                                 : QStringLiteral("未报告"))
                        .arg(stream.adaptiveNetworkFrameRateReductionSamples)
                        .arg(stream.adaptiveNetworkFrameRateRecoverySamples);
                    if (!stream.adaptiveNetworkFrameRateError.empty()) {
                        frameRateDetail += QStringLiteral("\n错误：%1").arg(
                            QString::fromStdString(
                                stream.adaptiveNetworkFrameRateError));
                    }
                    streamCard.chips << DiagnosticsChip{
                        QStringLiteral("adaptiveNetworkFrameRate"),
                        QStringLiteral("网络有效帧率"),
                        frameRateDetail,
                        stream.adaptiveNetworkFrameRateError.empty()
                            ? QByteArray("good")
                            : QByteArray("warning"),
                        true};
                }
                if (stream.kind == "video") {
                    if (!diagnostics.videoEncoderLastFallbackReason.empty()) {
                        const bool hardwareRecovered =
                            implementation.find("FFmpeg/QSV") !=
                                std::string::npos ||
                            implementation.find("FFmpeg/NVENC") !=
                                std::string::npos ||
                            implementation.find("FFmpeg/AMF") !=
                                std::string::npos ||
                            implementation.find("MediaFoundation") !=
                                std::string::npos;
                        streamCard.chips << DiagnosticsChip{
                            QStringLiteral("encoderFallbackReason"),
                            hardwareRecovered
                                ? QStringLiteral(
                                      "最近硬件编码异常（已自动恢复）")
                                : QStringLiteral("最近硬件编码异常"),
                            QString::fromStdString(
                                diagnostics.videoEncoderLastFallbackReason),
                            hardwareRecovered ? QByteArray("good")
                                              : QByteArray("warning"),
                            true};
                    }
                    const QString configuredCaptureBackend =
                        stream.captureConfiguredBackend == "libwebrtc"
                            ? QStringLiteral("libwebrtc")
                            : stream.captureConfiguredBackend == "native_dxgi"
                                  ? QStringLiteral("自研 DXGI")
                                  : QStringLiteral("未报告");
                    QString activeCaptureBackend;
                    if (stream.captureActiveBackend ==
                        "native_dxgi_texture") {
                        activeCaptureBackend =
                            QStringLiteral("自研 DXGI 纹理");
                    } else if (stream.captureActiveBackend ==
                               "libwebrtc_dxgi_with_gdi_fallback") {
                        activeCaptureBackend =
                            QStringLiteral("libwebrtc DXGI/GDI");
                    } else if (stream.captureActiveBackend ==
                               "libwebrtc_gdi") {
                        activeCaptureBackend =
                            QStringLiteral("libwebrtc GDI");
                    } else {
                        activeCaptureBackend = QStringLiteral("未报告");
                    }
                    QString captureBackendText =
                        QStringLiteral("配置 %1 · 实际 %2")
                            .arg(configuredCaptureBackend,
                                 activeCaptureBackend);
                    if (!stream.captureFallbackReason.empty()) {
                        captureBackendText += QStringLiteral("\n回退：%1")
                            .arg(QString::fromStdString(
                                stream.captureFallbackReason));
                    }
                    streamCard.chips
                        << DiagnosticsChip{
                               QStringLiteral("captureBackend"),
                               QStringLiteral("屏幕采集器"),
                               captureBackendText,
                               stream.captureFallbackReason.empty()
                                   ? QByteArray("good")
                                   : QByteArray("warning"),
                               true}
                        << DiagnosticsChip{
                               QStringLiteral("capture"),
                               QStringLiteral("桌面采集调度"),
                               stream.captureTargetFrameRate > 0
                                    ? QStringLiteral(
                                          "目标 %1 · 调用 %2 · 交付 %3 FPS")
                                         .arg(
                                             stream.captureTargetFrameRate)
                                         .arg(
                                             stream.captureAttemptsPerSecond,
                                             0, 'f', 1)
                                         .arg(
                                             stream.captureDeliveredFramesPerSecond,
                                             0, 'f', 1)
                                    : QStringLiteral("未报告"),
                               !stream.captureAdaptiveFrameDeliveryEnabled &&
                                       stream.captureDeliveredFramesPerSecond +
                                               1.0 <
                                           static_cast<double>(
                                               stream.captureTargetFrameRate)
                                    ? QByteArray("warning")
                                    : QByteArray("good"),
                               true}
                        << DiagnosticsChip{
                               QStringLiteral("captureActivity"),
                               QStringLiteral("静态桌面降帧"),
                               stream.captureAdaptiveFrameDeliveryEnabled
                                   ? QStringLiteral(
                                         "%1 · 变化 %2 · 心跳 %3 FPS")
                                         .arg(
                                             stream.captureActivityState ==
                                                     "idle"
                                                 ? QStringLiteral("静止")
                                                 : (stream.captureActivityState ==
                                                            "active"
                                                        ? QStringLiteral("活动")
                                                        : QStringLiteral("启动")))
                                         .arg(
                                             stream.captureChangedFramesPerSecond,
                                             0, 'f', 1)
                                         .arg(
                                             stream.captureIdleHeartbeatFramesPerSecond,
                                             0, 'f', 1)
                                   : QStringLiteral("回退路径未启用"),
                               stream.captureAdaptiveFrameDeliveryEnabled
                                   ? QByteArray("good")
                                   : QByteArray("normal"),
                               true}
                        << DiagnosticsChip{
                               QStringLiteral("captureSuppression"),
                               QStringLiteral("累计抑制重复帧"),
                               stream.captureAdaptiveFrameDeliveryEnabled
                                   ? QStringLiteral("%1 · 状态切换 %2")
                                         .arg(
                                             stream.captureSuppressedUnchangedFrames)
                                         .arg(
                                             stream.captureActivityTransitions)
                                   : QStringLiteral("不适用"),
                               "normal", false}
                        << DiagnosticsChip{
                               QStringLiteral("captureInteraction"),
                               QStringLiteral("交互加速 / 强制刷新"),
                               stream.captureConfiguredBackend == "libwebrtc"
                                   ? QStringLiteral(
                                         "%1 · 输入加速 %2 次 · 全帧 %3 次")
                                         .arg(
                                             stream.captureInputBoostActive
                                                 ? QStringLiteral("加速中")
                                                 : QStringLiteral("常规"))
                                         .arg(stream.captureInputBoosts)
                                         .arg(
                                             stream.captureForcedRefreshFrames)
                                   : QStringLiteral("仅 libwebrtc 路径启用"),
                               stream.captureInputBoostActive
                                   ? QByteArray("good")
                                   : QByteArray("normal"),
                               false}
                        << DiagnosticsChip{
                               QStringLiteral("captureCall"),
                               QStringLiteral("采集调用耗时"),
                               stream.captureTargetFrameRate > 0
                                   ? QStringLiteral(
                                         "%1 ms · 失败 %2/%3")
                                         .arg(
                                             stream.latestCaptureCallMs,
                                             0, 'f', 3)
                                         .arg(stream.captureFailures)
                                         .arg(stream.captureAttempts)
                                   : QStringLiteral("未报告"),
                               stream.latestCaptureCallMs >= 15.0
                                   ? QByteArray("warning")
                                   : QByteArray("normal"),
                               false}
                        << DiagnosticsChip{
                               QStringLiteral("source"),
                               QStringLiteral("WebRTC源输出"),
                               stream.sourceWidth > 0 &&
                                       stream.sourceHeight > 0
                                   ? QStringLiteral("%1 × %2 · %3 FPS")
                                         .arg(stream.sourceWidth)
                                         .arg(stream.sourceHeight)
                                         .arg(
                                             stream.sourceFramesPerSecond,
                                             0, 'f', 1)
                                   : QStringLiteral("未报告"),
                               "normal", true}
                        << DiagnosticsChip{
                               QStringLiteral("encodedFps"),
                               QStringLiteral("编码完成帧率"),
                               QStringLiteral("%1 FPS")
                                   .arg(
                                       stream.encodedFramesPerSecond,
                                       0, 'f', 1),
                               stream.encodedFramesPerSecond > 0.0
                                   ? QByteArray("good")
                                   : QByteArray("normal"),
                               false}
                        << DiagnosticsChip{
                               QStringLiteral("sentFps"),
                               QStringLiteral("RTP发送帧率"),
                               QStringLiteral("%1 FPS")
                                   .arg(
                                       stream.sentFramesPerSecond,
                                       0, 'f', 1),
                               stream.sentFramesPerSecond > 0.0
                                   ? QByteArray("good")
                                   : QByteArray("normal"),
                               false}
                        << DiagnosticsChip{
                               QStringLiteral("encodeTime"),
                               QStringLiteral("最近一帧编码"),
                               LatestFrameTimingText(
                                   stream,
                                   QStringLiteral("编码")),
                               stream.latestFrameTimeMs >= 30.0
                                   ? QByteArray("warning")
                                   : QByteArray("normal"),
                               true}
                        << DiagnosticsChip{
                               QStringLiteral("windowEncodeTime"),
                               QStringLiteral("采样窗平均编码"),
                               stream.windowEncodeTimeAvailable
                                   ? QStringLiteral("%1 ms%2")
                                         .arg(
                                             stream.windowEncodeTimeMs,
                                             0, 'f', 3)
                                         .arg(SampleWindowSuffix(
                                             stream.sampleWindowMs))
                                   : QStringLiteral("等待第二次采样"),
                               stream.windowEncodeTimeMs >= 30.0
                                   ? QByteArray("warning")
                                   : QByteArray("normal"),
                               false}
                        << DiagnosticsChip{
                               QStringLiteral("sessionEncodeTime"),
                               QStringLiteral("会话平均编码"),
                               QStringLiteral("%1 ms").arg(
                                   stream.averageEncodeTimeMs,
                                   0, 'f', 3),
                               "normal", false}
                        << DiagnosticsChip{
                               QStringLiteral("windowQp"),
                               QStringLiteral("采样窗平均 QP"),
                               stream.windowQpAvailable
                                   ? QStringLiteral("%1").arg(
                                         stream.windowQp, 0, 'f', 2)
                                   : QStringLiteral("未报告"),
                               "normal", false}
                        << DiagnosticsChip{
                               QStringLiteral("limitation"),
                               QStringLiteral("当前质量限制"),
                               stream.qualityLimitationReason.empty() ||
                                       stream.qualityLimitationReason ==
                                           "none"
                                   ? QStringLiteral("无")
                                   : QString::fromStdString(
                                         stream
                                             .qualityLimitationReason),
                               stream.qualityLimitationReason.empty() ||
                                       stream.qualityLimitationReason ==
                                           "none"
                                   ? QByteArray("good")
                                   : QByteArray("warning"),
                               false};
                }
