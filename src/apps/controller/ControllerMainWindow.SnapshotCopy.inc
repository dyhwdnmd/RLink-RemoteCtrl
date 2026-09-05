// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

        if (diagnosticsCopyTextRequested_) {
        const QSettings encoderSelectionSettings;
        const QString encoderSelectionCaptureBackend =
            encoderSelectionSettings.value(
                QString::fromLatin1(kDesktopCaptureBackendSetting),
                QStringLiteral("native_dxgi")).toString();
        const QString encoderSelectionX264Preset =
            encoderSelectionSettings.value(
                QString::fromLatin1(kFfmpegX264PresetSetting),
                QStringLiteral("medium")).toString();
        const bool encoderSelectionCurrent =
            encoderSelectionSettings.value(
                QString::fromLatin1(kEncoderBenchmarkCompletedSetting),
                false).toBool() &&
            encoderSelectionSettings.value(
                QString::fromLatin1(kEncoderBenchmarkPassedSetting),
                false).toBool() &&
            encoderSelectionSettings.value(
                QString::fromLatin1(kEncoderBenchmarkPolicyVersionSetting),
                0).toInt() == kEncoderBenchmarkPolicyVersion &&
            encoderSelectionSettings.value(
                QString::fromLatin1(kEncoderHardwareFingerprintSetting))
                .toString() == hardwareFingerprint &&
            encoderSelectionSettings.value(
                QString::fromLatin1(kEncoderCaptureBackendSetting))
                .toString() == encoderSelectionCaptureBackend &&
            encoderSelectionSettings.value(
                QString::fromLatin1(kEncoderX264PresetSetting))
                .toString() == encoderSelectionX264Preset;
        const QString automaticEncoderSelection = encoderSelectionCurrent
            ? QStringLiteral("%1 [%2]")
                  .arg(encoderSelectionSettings.value(
                           QString::fromLatin1(kBestEncoderNameSetting))
                           .toString(),
                       encoderSelectionSettings.value(
                           QString::fromLatin1(kBestEncoderIdSetting))
                           .toString())
            : QStringLiteral("等待当前采集方式的性能检测");
        QStringList copyLines;
        copyLines
            << QStringLiteral("RLink 调试信息")
            << QStringLiteral("================")
            << QString()
            << QStringLiteral("【设备与信令】")
            << QStringLiteral("本机设备 ID：%1").arg(deviceId)
            << QStringLiteral("信令连接：%1").arg(signaling)
            << QStringLiteral("WebRTC 运行时：%1").arg(webRtc)
            << QString()
            << QStringLiteral("【兼容性环境】")
            << QStringLiteral("系统与架构：%1")
                   .arg(systemEnvironment)
            << QStringLiteral("会话类型：%1")
                   .arg(sessionEnvironment)
            << QStringLiteral("图形适配器：%1")
                   .arg(graphicsAdapters.join(
                       QStringLiteral("；")))
            << QStringLiteral("硬件能力指纹：%1")
                   .arg(hardwareFingerprint)
            << QStringLiteral("编码能力探测：%1")
                   .arg(encoderProbeSource)
            << QStringLiteral("WebRTC 音频模块：%1")
                   .arg(audioDeviceModule.replace(
                       QLatin1Char('\n'), QStringLiteral("；")))
            << QStringLiteral("摄像头：%1")
                   .arg(QString(cameraDevice).replace(
                       QLatin1Char('\n'), QStringLiteral("；")))
            << QStringLiteral("麦克风：%1")
                   .arg(QString(microphoneDevice).replace(
                       QLatin1Char('\n'), QStringLiteral("；")))
            << QStringLiteral("扬声器：%1")
                   .arg(QString(speakerDevice).replace(
                       QLatin1Char('\n'), QStringLiteral("；")))
            << QString()
            << QStringLiteral("【媒体能力】")
            << QStringLiteral("H264 编码链路：%1")
                   .arg(builtinEncoder)
            << QStringLiteral("屏幕采集模式：%1")
                   .arg(desktopCapture)
            << QStringLiteral("视频编码模式：%1")
                   .arg(encoderPreference)
            << QStringLiteral("编码质量：%1")
                   .arg(encoderQuality)
            << QStringLiteral("自动编码实测选择：%1")
                   .arg(automaticEncoderSelection)
            << QStringLiteral("硬件编码器：%1")
                   .arg(hardwareEncoderCount)
            << QStringLiteral("CPU NV12 输入：%1").arg(cpuNv12)
            << QStringLiteral("D3D11 硬件编码：%1")
                   .arg(d3d11Encoder)
            << QStringLiteral("FFmpeg 硬件编码：%1")
                   .arg(ffmpegHardwareEncoder)
            << QStringLiteral("软件编码链路：%1")
                   .arg(encoderFallback)
            << QStringLiteral("当前编码实例：%1")
                   .arg(encoderRuntimeDetails.join(
                       QStringLiteral("；")))
            << QStringLiteral("最近回退原因：%1")
                   .arg(encoderFallbackReason)
            << QStringLiteral("H264 解码：%1").arg(decoder)
            << QStringLiteral("视频解码模式：%1")
                   .arg(decoderPreference)
            << QStringLiteral("MF D3D11 解码类型：%1")
                   .arg(mfDecoderType)
            << QStringLiteral("MF 解码器实例：%1")
                   .arg(mfDecoderName)
            << QStringLiteral("D3D11 原生输出：%1")
                   .arg(d3d11Output)
            << QStringLiteral("硬件 MFT 驱动：%1")
                   .arg(mfDecoderAsync)
            << QStringLiteral("FFmpeg 软件解码：%1")
                   .arg(softwareFallback)
            << QStringLiteral("MF 解码检测说明：%1")
                   .arg(mfDecoderError)
            << encoderDetails
            << QString()
            << QStringLiteral("【房间状态】")
            << QStringLiteral("状态：%1").arg(membership)
            << QStringLiteral("房间 ID：%1").arg(roomId)
            << QStringLiteral("成员与席位：%1").arg(members)
            << QStringLiteral("当前主机器：%1").arg(screenSharer)
            << QStringLiteral("当前控制者：%1").arg(controller)
            << QStringLiteral("控制授权令牌：%1").arg(controlGrant)
            << QString()
            << QStringLiteral("【成员连接】")
            << peerSummary
            << peerDetails
            << QString()
            << QStringLiteral("【远程粘贴】")
            << QStringLiteral("状态：%1").arg(clipboardState)
            << QStringLiteral("成员对：%1").arg(clipboardPeer)
            << QStringLiteral("当前传输：%1").arg(clipboardTransfer)
            << QStringLiteral("最近项目：%1").arg(clipboardLast)
            << QStringLiteral("累计统计：%1").arg(clipboardCounters)
            << QStringLiteral("最近错误：%1").arg(clipboardError)
            << QString()
            << QStringLiteral("【最近错误】")
            << QStringLiteral("会话错误：%1").arg(sessionError)
            << QStringLiteral("房间错误：%1").arg(roomError);
        debugCopyText_ = copyLines.join(QLatin1Char('\n'));

        QStringList mediaCopyLines;
        mediaCopyLines
            << QStringLiteral("RLink 媒体能力")
            << QStringLiteral("================")
            << QStringLiteral("H264 编码链路：%1")
                   .arg(builtinEncoder)
            << QStringLiteral("屏幕采集模式：%1")
                   .arg(desktopCapture)
            << QStringLiteral("视频编码模式：%1")
                   .arg(encoderPreference)
            << QStringLiteral("编码质量：%1")
                   .arg(encoderQuality)
            << QStringLiteral("自动编码实测选择：%1")
                   .arg(automaticEncoderSelection)
            << QStringLiteral("硬件编码器：%1")
                   .arg(hardwareEncoderCount)
            << QStringLiteral("CPU NV12 输入：%1").arg(cpuNv12)
            << QStringLiteral("D3D11 硬件编码：%1")
                   .arg(d3d11Encoder)
            << QStringLiteral("FFmpeg 硬件编码：%1")
                   .arg(ffmpegHardwareEncoder)
            << QStringLiteral("软件编码链路：%1")
                   .arg(encoderFallback)
            << QStringLiteral("当前编码实例：%1")
                   .arg(encoderRuntimeDetails.join(
                       QStringLiteral("；")))
            << QStringLiteral("最近回退原因：%1")
                   .arg(encoderFallbackReason)
            << QStringLiteral("H264 解码：%1").arg(decoder)
            << QStringLiteral("视频解码模式：%1")
                   .arg(decoderPreference)
            << QStringLiteral("MF D3D11 解码类型：%1")
                   .arg(mfDecoderType)
            << QStringLiteral("MF 解码器实例：%1")
                   .arg(mfDecoderName)
            << QStringLiteral("D3D11 原生输出：%1")
                   .arg(d3d11Output)
            << QStringLiteral("硬件 MFT 驱动：%1")
                   .arg(mfDecoderAsync)
            << QStringLiteral("FFmpeg 软件解码：%1")
                   .arg(softwareFallback)
            << QStringLiteral("MF 解码检测说明：%1")
                   .arg(mfDecoderError)
            << encoderDetails;
        mediaDebugCopyText_ =
            mediaCopyLines.join(QLatin1Char('\n'));
        }
    }
