// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

void ControllerMainWindow::CheckForSoftwareUpdates(bool manualRequest)
{
    if (!softwareUpdateController_) {
        if (manualRequest) {
            RemoteCDialog::Alert(
                this, QStringLiteral("暂时无法检查更新"),
                QStringLiteral("更新组件尚未准备好，请稍后重试。"));
        }
        return;
    }
    softwareUpdateController_->CheckForUpdates(manualRequest);
}

void ControllerMainWindow::HandleSoftwareUpdateState(
    const update::SoftwareUpdateController::Snapshot& snapshot)
{
    using UpdateState = update::SoftwareUpdateController::State;
    const bool checking = snapshot.state == UpdateState::kChecking;
    const bool available =
        snapshot.state == UpdateState::kUpdateAvailable;
    const bool launching =
        snapshot.state == UpdateState::kLaunchingUpdater;

    if (profileUpdateButton_) {
        profileUpdateButton_->setVisible(available);
        profileUpdateButton_->setToolTip(
            available
                ? QStringLiteral("发现新版本 v%1")
                      .arg(snapshot.latestVersion)
                : QStringLiteral("有新版本可用"));
    }
    if (softwareUpdateAction_) {
        softwareUpdateAction_->setEnabled(!checking && !launching);
        softwareUpdateAction_->setText(
            available
                ? QStringLiteral("软件更新 · v%1")
                      .arg(snapshot.latestVersion)
                : checking
                    ? QStringLiteral("正在检查更新…")
                    : QStringLiteral("软件更新"));
    }
    if (softwareUpdateCheckButton_) {
        softwareUpdateCheckButton_->setEnabled(!checking && !launching);
        softwareUpdateCheckButton_->setText(
            available ? QStringLiteral("立即更新")
                      : checking ? QStringLiteral("检查中…")
                                 : QStringLiteral("检查更新"));
    }
    if (softwareUpdateStatusLabel_) {
        switch (snapshot.state) {
        case UpdateState::kIdle:
            softwareUpdateStatusLabel_->setText(
                QStringLiteral("当前安装版本由安装程序统一管理"));
            break;
        case UpdateState::kChecking:
            softwareUpdateStatusLabel_->setText(
                QStringLiteral("正在从 GitHub Releases 检查新版本"));
            break;
        case UpdateState::kUpToDate:
            softwareUpdateStatusLabel_->setText(
                QStringLiteral("当前已是最新版本"));
            break;
        case UpdateState::kUpdateAvailable:
            softwareUpdateStatusLabel_->setText(
                QStringLiteral("发现新版本 v%1")
                    .arg(snapshot.latestVersion));
            break;
        case UpdateState::kFailed:
            softwareUpdateStatusLabel_->setText(snapshot.errorMessage);
            break;
        case UpdateState::kLaunchingUpdater:
            softwareUpdateStatusLabel_->setText(
                QStringLiteral("正在启动独立更新程序"));
            break;
        }
    }

    if (snapshot.state == UpdateState::kUpToDate &&
        snapshot.manualRequest) {
        RemoteCToast::Show(
            this, QStringLiteral("当前已是最新版本"),
            RemoteCToast::Tone::kSuccess);
    } else if (snapshot.state == UpdateState::kFailed &&
               snapshot.manualRequest) {
        RemoteCDialog::Alert(
            this, QStringLiteral("检查更新失败"),
            snapshot.errorMessage, QStringLiteral("知道了"),
            RemoteCDialog::Tone::kWarning);
    } else if (available && snapshot.mandatory &&
               !softwareUpdatePromptOpen_) {
        QTimer::singleShot(0, this, [this] { OpenSoftwareUpdate(); });
    }
}

void ControllerMainWindow::OpenSoftwareUpdate()
{
    if (!softwareUpdateController_) {
        CheckForSoftwareUpdates(true);
        return;
    }
    const auto snapshot = softwareUpdateController_->CurrentSnapshot();
    using UpdateState = update::SoftwareUpdateController::State;
    if (snapshot.state != UpdateState::kUpdateAvailable) {
        CheckForSoftwareUpdates(true);
        return;
    }

    QString details = snapshot.summary.isEmpty()
        ? QStringLiteral("新版本已准备好。")
        : snapshot.summary;
    if (!snapshot.releaseNotes.isEmpty()) {
        details += QStringLiteral("\n\n本次更新：\n• ") +
            snapshot.releaseNotes.join(QStringLiteral("\n• "));
    }
    QString message = QStringLiteral(
        "当前版本：v%1\n可用版本：v%2\n\n%3\n\n"
        "开始后会结束当前远程会话，RLink 将自动退出、安装更新并重新启动。")
        .arg(snapshot.installedVersion,
             snapshot.latestVersion,
             details);
    softwareUpdatePromptOpen_ = true;
    const bool accepted = RemoteCDialog::Confirm(
        this,
        snapshot.mandatory ? QStringLiteral("必须更新 RLink")
                           : QStringLiteral("安装 RLink 更新"),
        message, QStringLiteral("立即更新"),
        snapshot.mandatory ? QStringLiteral("退出 RLink")
                           : QStringLiteral("稍后"),
        snapshot.mandatory ? RemoteCDialog::Tone::kWarning
                           : RemoteCDialog::Tone::kInformation,
        snapshot.mandatory);
    softwareUpdatePromptOpen_ = false;
    if (!accepted) {
        if (snapshot.mandatory) {
            PrepareForApplicationExit();
            QApplication::quit();
        }
        return;
    }

    QString launchError;
    if (!softwareUpdateController_->LaunchUpdater(&launchError)) {
        RemoteCDialog::Alert(
            this, QStringLiteral("无法开始更新"), launchError,
            QStringLiteral("知道了"), RemoteCDialog::Tone::kDanger);
        if (snapshot.mandatory) {
            QTimer::singleShot(0, this, [this] { OpenSoftwareUpdate(); });
        }
        return;
    }
    QuitForSoftwareUpdate();
}

void ControllerMainWindow::QuitForSoftwareUpdate()
{
    if (quitting_) {
        return;
    }
    quitting_ = true;
    PrepareForApplicationExit();
    if (trayIcon_) {
        trayIcon_->hide();
    }
    DestroyAuxiliaryWindowsForExit();
    for (QWidget* widget : QApplication::topLevelWidgets()) {
        if (widget) {
            widget->hide();
        }
    }
    QApplication::setQuitOnLastWindowClosed(true);
    QTimer::singleShot(0, qApp, [] { QApplication::quit(); });
}
