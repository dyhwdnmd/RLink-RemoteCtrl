// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

void ControllerMainWindow::StartSession(const QString& deviceId,
                                        const QString& deviceName,
                                        SessionPurpose purpose)
{
    pendingDeviceName_ = deviceName;
    const auto result = engine_->ConnectDevice(deviceId.toStdString(), purpose);
    if (!result.accepted) {
        connectionHint_->setText(
            QStringLiteral("无法发起会话：%1")
                .arg(QString::fromStdString(result.errorMessage)));
        connectionHint_->setProperty("error", true);
        connectionHint_->setStyleSheet(QStringLiteral("color:#d14343;"));
    } else {
        connectionHint_->setProperty("error", false);
        connectionHint_->setText(
            QStringLiteral("请求已发送，正在等待对方确认…"));
        connectionHint_->setStyleSheet(QStringLiteral("color:#a66b12;"));
    }
}

void ControllerMainWindow::StartAssistedSession(
    const QString& deviceId,
    const QString& verificationCode)
{
    pendingDeviceName_ = QStringLiteral("远程协助设备");
    assistedSessionPending_ = true;
    assistedSessionActive_ = false;
    assistedSessionCancellationPending_ = false;
    assistedSessionTimedOut_ = false;
    lastDirectSessionToastError_.clear();
    const auto beforeConnect = engine_->Snapshot();
    const auto result = engine_->ConnectAssistedDevice(
        deviceId.toStdString(), verificationCode.toStdString());
    if (!result.accepted) {
        assistedSessionPending_ = false;
        const QString errorText = LocalizedDirectSessionError(
            result.errorCode, result.errorMessage, &beforeConnect);
        connectionHint_->setText(
            errorText);
        connectionHint_->setProperty("error", true);
        connectionHint_->setStyleSheet(QStringLiteral("color:#d14343;"));
        RemoteCToast::Show(this, errorText, RemoteCToast::Tone::kError);
        return;
    }
    connectionHint_->setProperty("error", false);
    connectionHint_->setText(
        QStringLiteral("正在校验验证码并建立远程桌面…"));
    connectionHint_->setStyleSheet(QStringLiteral("color:#a66b12;"));
    connectButton_->setEnabled(true);
    connectButton_->setText(
        QStringLiteral("正在连接…（点击取消）"));
    assistedSessionTimeoutTimer_->start();
}
