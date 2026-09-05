// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

                outboundCards.push_back(std::move(streamCard));
            } else {
                streamCard.chips
                    << DiagnosticsChip{
                           QStringLiteral("jitter"),
                           QStringLiteral("网络抖动"),
                           QStringLiteral("%1 ms").arg(
                               stream.jitterMs, 0, 'f', 1),
                           stream.jitterMs >= 30.0
                               ? QByteArray("warning")
                               : QByteArray("normal"),
                           false}
                    << DiagnosticsChip{
                           QStringLiteral("buffer"),
                           QStringLiteral("采样窗抖动缓冲"),
                           stream.windowJitterBufferDelayAvailable
                               ? QStringLiteral("%1 ms").arg(
                                     stream
                                         .windowJitterBufferDelayMs,
                                     0, 'f', 3)
                               : QStringLiteral("等待第二次采样"),
                           "normal", false};
                if (stream.kind == "video") {
                    streamCard.chips
                        << DiagnosticsChip{
                               QStringLiteral("decodeTime"),
                               QStringLiteral("最近一帧解码"),
                               LatestFrameTimingText(
                                   stream,
                                   QStringLiteral("解码")),
                               stream.latestFrameTimeMs >= 30.0
                                   ? QByteArray("warning")
                                   : QByteArray("normal"),
                               true}
                        << DiagnosticsChip{
                               QStringLiteral("windowDecodeTime"),
                               QStringLiteral("采样窗平均解码"),
                               stream.windowDecodeTimeAvailable
                                   ? QStringLiteral("%1 ms").arg(
                                         stream.windowDecodeTimeMs,
                                         0, 'f', 3)
                                   : QStringLiteral("等待第二次采样"),
                               stream.windowDecodeTimeMs >= 30.0
                                   ? QByteArray("warning")
                                   : QByteArray("normal"),
                               false}
                        << DiagnosticsChip{
                               QStringLiteral("decoderTimingWindow"),
                               QStringLiteral("逐帧解码耗时窗口"),
                               QStringLiteral(
                                   "平均 %1 · P95 %2 · 最大 %3 ms")
                                   .arg(stream.averageFrameTimingMs,
                                       0, 'f', 3)
                                   .arg(stream.p95FrameTimingMs,
                                       0, 'f', 3)
                                   .arg(stream.maximumFrameTimingMs,
                                       0, 'f', 3),
                               stream.p95FrameTimingMs >= 30.0
                                   ? QByteArray("warning")
                                   : QByteArray("normal"),
                               true}
                        << DiagnosticsChip{
                               QStringLiteral("sessionDecodeTime"),
                               QStringLiteral("会话平均解码"),
                               QStringLiteral("%1 ms").arg(
                                   stream.averageDecodeTimeMs,
                                   0, 'f', 3),
                               "normal", false}
                        << DiagnosticsChip{
                               QStringLiteral("processing"),
                               QStringLiteral("采样窗处理延迟"),
                               stream.windowProcessingDelayAvailable
                                   ? QStringLiteral("%1 ms").arg(
                                         stream
                                             .windowProcessingDelayMs,
                                         0, 'f', 3)
                                   : QStringLiteral("等待第二次采样"),
                               "normal", false}
                        << DiagnosticsChip{
                               QStringLiteral("sessionProcessing"),
                               QStringLiteral("会话平均处理延迟"),
                               QStringLiteral("%1 ms").arg(
                                   stream.averageProcessingDelayMs,
                                   0, 'f', 3),
                               "normal", false}
                        << DiagnosticsChip{
                               QStringLiteral("sessionBuffer"),
                               QStringLiteral("会话平均抖动缓冲"),
                               QStringLiteral("%1 ms").arg(
                                   stream
                                       .averageJitterBufferDelayMs,
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
                               QStringLiteral("freeze"),
                               QStringLiteral("累计冻结/暂停"),
                               QStringLiteral(
                                   "%1 次/%2 ms · %3 次/%4 ms")
                                   .arg(stream.freezeCount)
                                   .arg(
                                       stream.totalFreezeDurationMs,
                                       0, 'f', 0)
                                   .arg(stream.pauseCount)
                                   .arg(
                                       stream.totalPauseDurationMs,
                                       0, 'f', 0),
                               stream.freezeCount > 0
                                   ? QByteArray("warning")
                                   : QByteArray("normal"),
                               true};
                    if (stream.presentationTimingAvailable) {
                        const double supersededPercent =
                            stream.presentationArrivedFrames > 0
                                ? static_cast<double>(
                                      stream.presentationSupersededFrames) *
                                      100.0 /
                                      static_cast<double>(
                                          stream.presentationArrivedFrames)
                                : 0.0;
                        const bool nativePresentation =
                            stream.presentationPath == "D3D11" ||
                            stream.presentationPath == "CPU/D3D11" ||
                            stream.presentationPath == "CPU NV12/D3D11" ||
                            stream.presentationPath == "CPU I420/D3D11";
                        const bool i420ShaderPresentation =
                            stream.presentationPath == "CPU I420/D3D11";
                        streamCard.chips
                            << DiagnosticsChip{
                                   QStringLiteral("presentationPath"),
                                   QStringLiteral("本机显示路径"),
                                   QStringLiteral("%1 · %2 Hz")
                                       .arg(QString::fromStdString(
                                           stream.presentationPath))
                                       .arg(stream.localRefreshRateHz,
                                           0, 'f', 1),
                                   nativePresentation
                                       ? QByteArray("good")
                                       : QByteArray("normal"),
                                   false}
                            << DiagnosticsChip{
                                   QStringLiteral("presentationArrival"),
                                   QStringLiteral("应用帧到达"),
                                   QStringLiteral("%1 FPS · 累计 %2")
                                       .arg(
                                           stream
                                               .presentationArrivalFramesPerSecond,
                                           0, 'f', 1)
                                       .arg(
                                           stream.presentationArrivedFrames),
                                   stream.presentationArrivalFramesPerSecond >
                                           0.0
                                       ? QByteArray("good")
                                       : QByteArray("normal"),
                                   false}
                            << DiagnosticsChip{
                                   QStringLiteral("presentationFps"),
                                   QStringLiteral("显示提交帧率"),
                                   QStringLiteral("%1 FPS · 累计 %2")
                                       .arg(stream.presentedFramesPerSecond,
                                           0, 'f', 1)
                                       .arg(stream.presentedFrames),
                                   stream.presentedFramesPerSecond > 0.0
                                       ? QByteArray("good")
                                       : QByteArray("normal"),
                                   false}
                            << DiagnosticsChip{
                                   QStringLiteral("presentationSuperseded"),
                                   QStringLiteral("应用覆盖丢帧"),
                                   QStringLiteral("%1 帧 · %2%")
                                       .arg(
                                           stream.presentationSupersededFrames)
                                       .arg(supersededPercent, 0, 'f', 2),
                                   supersededPercent >= 3.0
                                       ? QByteArray("warning")
                                       : QByteArray("normal"),
                                   false}
                            << DiagnosticsChip{
                                   QStringLiteral("presentationInterval"),
                                   QStringLiteral("呈现帧间隔"),
                                   QStringLiteral(
                                       "平均 %1 · P95 %2 · 最大 %3 ms")
                                       .arg(
                                           stream.averagePresentedIntervalMs,
                                           0, 'f', 3)
                                       .arg(stream.p95PresentedIntervalMs,
                                           0, 'f', 3)
                                       .arg(stream.maximumPresentedIntervalMs,
                                           0, 'f', 3),
                                   stream.p95PresentedIntervalMs >= 30.0
                                       ? QByteArray("warning")
                                       : QByteArray("normal"),
                                   true}
                            << DiagnosticsChip{
                                   QStringLiteral("receiverPipeline"),
                                   QStringLiteral("末包到应用画面"),
                                   QStringLiteral(
                                       "最近 %1 · 平均 %2 · P95 %3 · 最大 %4 ms")
                                       .arg(stream.latestReceiverPipelineMs,
                                           0, 'f', 3)
                                       .arg(stream.averageReceiverPipelineMs,
                                           0, 'f', 3)
                                       .arg(stream.p95ReceiverPipelineMs,
                                           0, 'f', 3)
                                       .arg(stream.maximumReceiverPipelineMs,
                                           0, 'f', 3),
                                   stream.p95ReceiverPipelineMs >= 150.0
                                       ? QByteArray("warning")
                                       : QByteArray("normal"),
                                   true};
                        if (stream.presentationConvertedFrames > 0) {
                            streamCard.chips
                                << DiagnosticsChip{
                                       QStringLiteral(
                                           "presentationConversion"),
                                       i420ShaderPresentation
                                           ? QStringLiteral("I420三平面上传")
                                           : QStringLiteral("CPU帧格式整理"),
                                       QStringLiteral(
                                           "最近 %1 · 平均 %2 ms · %3 帧")
                                           .arg(
                                               stream
                                                   .latestPresentationConversionMs,
                                               0, 'f', 3)
                                           .arg(
                                               stream
                                                   .averagePresentationConversionMs,
                                               0, 'f', 3)
                                           .arg(
                                               stream
                                                   .presentationConvertedFrames),
                                       stream
                                                   .averagePresentationConversionMs >=
                                               8.0
                                           ? QByteArray("warning")
                                           : QByteArray("normal"),
                                       true};
                        }
                        streamCard.chips
                            << DiagnosticsChip{
                                   QStringLiteral("presentationRenderSubmit"),
                                   i420ShaderPresentation
                                       ? QStringLiteral("Pixel Shader提交")
                                       : (nativePresentation
                                       ? QStringLiteral(
                                             "VideoProcessor提交")
                                       : QStringLiteral("Qt绘制提交")),
                                   QStringLiteral("最近 %1 · 平均 %2 ms")
                                       .arg(
                                           stream
                                               .latestPresentationRenderSubmitMs,
                                           0, 'f', 3)
                                       .arg(
                                           stream
                                               .averagePresentationRenderSubmitMs,
                                           0, 'f', 3),
                                   stream
                                               .averagePresentationRenderSubmitMs >=
                                           8.0
                                       ? QByteArray("warning")
                                       : QByteArray("normal"),
                                   false};
                        if (nativePresentation) {
                            streamCard.chips
                                << DiagnosticsChip{
                                       QStringLiteral("presentationCall"),
                                       QStringLiteral("Present调用"),
                                       QStringLiteral(
                                           "最近 %1 · 平均 %2 ms")
                                           .arg(
                                               stream.latestPresentCallMs,
                                               0, 'f', 3)
                                           .arg(
                                               stream.averagePresentCallMs,
                                               0, 'f', 3),
                                       stream.averagePresentCallMs >= 8.0
                                           ? QByteArray("warning")
                                           : QByteArray("normal"),
                                       false};
                        }
                        streamCard.chips
                            << DiagnosticsChip{
                                   QStringLiteral("presentationFailures"),
                                   QStringLiteral("显示失败"),
                                   QStringLiteral("%1 次").arg(
                                       stream.presentationFailures),
                                   stream.presentationFailures > 0
                                       ? QByteArray("warning")
                                       : QByteArray("normal"),
                                   false};
                    }
                    if (stream.decodePipelineTimingAvailable) {
                        streamCard.chips
                            << DiagnosticsChip{
                                   QStringLiteral("decodePreparation"),
                                   QStringLiteral("输入准备耗时"),
                                   QStringLiteral("%1 ms").arg(
                                       stream.decodeInputPreparationMs,
                                       0, 'f', 3),
                                   "normal", false}
                            << DiagnosticsChip{
                                   QStringLiteral("decodeQueue"),
                                   QStringLiteral("输入排队耗时"),
                                   QStringLiteral("%1 ms").arg(
                                       stream.decodeInputQueueWaitMs,
                                       0, 'f', 3),
                                   stream.decodeInputQueueWaitMs >= 20.0
                                       ? QByteArray("warning")
                                       : QByteArray("normal"),
                                   false}
                            << DiagnosticsChip{
                                   QStringLiteral("decodeTransform"),
                                   QStringLiteral("MFT输出等待"),
                                   QStringLiteral(
                                       "最近 %1 · 平均 %2 · P95 %3 · 最大 %4 ms")
                                       .arg(stream.decodeTransformWaitMs,
                                           0, 'f', 3)
                                       .arg(
                                           stream.averageDecodeTransformWaitMs,
                                           0, 'f', 3)
                                       .arg(stream.p95DecodeTransformWaitMs,
                                           0, 'f', 3)
                                       .arg(
                                           stream.maximumDecodeTransformWaitMs,
                                           0, 'f', 3),
                                   stream.p95DecodeTransformWaitMs >= 30.0
                                       ? QByteArray("warning")
                                       : QByteArray("normal"),
                                   true}
                            << DiagnosticsChip{
                                   QStringLiteral("decodeDelivery"),
                                   QStringLiteral("纹理交付耗时"),
                                   QStringLiteral("%1 ms").arg(
                                       stream.decodeOutputDeliveryMs,
                                       0, 'f', 3),
                                   stream.decodeOutputDeliveryMs >= 10.0
                                       ? QByteArray("warning")
                                       : QByteArray("normal"),
                                   false}
                            << DiagnosticsChip{
                                   QStringLiteral("decodeBacklog"),
                                   QStringLiteral("硬解积压"),
                                   QStringLiteral("排队 %1 · MFT内 %2 · 峰值 %3%4")
                                       .arg(stream.decodeQueuedInputFrames)
                                       .arg(stream.decodeInFlightFrames)
                                       .arg(stream.decodePeakBacklogFrames)
                                       .arg(
                                           stream.decodePipelineAsynchronous
                                               ? QStringLiteral(" · 异步")
                                               : QStringLiteral(" · 同步")),
                                   stream.decodePeakBacklogFrames >= 6
                                       ? QByteArray("warning")
                                       : QByteArray("normal"),
                                   true};
                    }
                } else {
                    streamCard.chips
                        << DiagnosticsChip{
                               QStringLiteral("audio"),
                               QStringLiteral("当前电平/累计隐藏"),
                               QStringLiteral(
                                   "电平 %1 · 隐藏样本 %2/%3 · %4 次")
                                   .arg(stream.audioLevel, 0,
                                        'f', 3)
                                   .arg(
                                       stream.concealedAudioSamples)
                                   .arg(stream.totalAudioSamples)
                                   .arg(stream.concealmentEvents),
                               stream.concealmentEvents > 0
                                   ? QByteArray("warning")
                                   : QByteArray("normal"),
                               true};
                }
