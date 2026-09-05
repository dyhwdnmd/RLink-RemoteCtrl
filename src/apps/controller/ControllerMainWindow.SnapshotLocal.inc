// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

    if (!snapshot.localDeviceId.empty()) {
        localDeviceId_->setText(
            QString::fromStdString(snapshot.localDeviceId));
        copyDeviceIdButton_->setEnabled(true);
    } else {
        localDeviceId_->setText(QStringLiteral("未注册"));
        copyDeviceIdButton_->setEnabled(false);
    }
    if (localVerificationCode_) {
        const QString code = QString::fromStdString(
            snapshot.localVerificationCode);
        localVerificationCode_->setText(
            code.size() == 6 ? code : QStringLiteral("------"));
        copyVerificationCodeButton_->setEnabled(code.size() == 6);
    }
    if (shareLocalCredentialsButton_) {
        shareLocalCredentialsButton_->setEnabled(
            IsNineDigitPublicId(localDeviceId_->text().trimmed()) &&
            localVerificationCode_->text().trimmed().size() == 6);
    }

    const bool debugPageVisible =
        debugPage_ && debugPage_->isVisibleTo(this);
    if (!debugValueLabels_.isEmpty() &&
        (debugPageVisible || diagnosticsCopyTextRequested_)) {
        const auto capabilities = engine_->Capabilities();
        const auto onlineMembers = std::count_if(
            snapshot.room.members.begin(), snapshot.room.members.end(),
            [](const RoomMemberSnapshot& member) { return member.online; });
        const auto activePeers = std::count_if(
            snapshot.roomPeerConnections.begin(),
            snapshot.roomPeerConnections.end(),
            [](const RoomPeerConnectionSnapshot& peer) {
                return peer.state == RoomPeerConnectionState::kActive;
            });
        const auto setDebugValue =
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
