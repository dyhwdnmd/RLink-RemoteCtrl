// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

    // Explicit runtime polish for the signed-in local-device workspace.
    // Do not rely on zero-height QSS tricks here: these widgets are hidden
    // directly so the layout is deterministic across Qt/Windows DPI modes.
    heroBandTitle->hide();
    heroBandSubtitle->hide();
    devicePageSignalStatus_->hide();
    runtimeStatus_->hide();
    decoderStatus_->hide();
    identitySubtitle->hide();
    approvalBadge->hide();
    connectionHint_->hide();
    trustStrip->hide();

    // All contents in the signed-in hero are intentionally hidden. Remove
    // the surface itself as well; keeping its old fixed height left a large
    // invisible band between the page heading and the two useful panels.
    heroBand->hide();
    heroBand->setFixedHeight(0);

    loggedInWorkspace->setAttribute(Qt::WA_StyledBackground, true);
    heroBand->setAttribute(Qt::WA_StyledBackground, true);
    identityPanel->setAttribute(Qt::WA_StyledBackground, true);
    directAssistCard->setAttribute(Qt::WA_StyledBackground, true);
    deviceIdBox->setAttribute(Qt::WA_StyledBackground, true);
    verificationBox->setAttribute(Qt::WA_StyledBackground, true);
    accountBox->setAttribute(Qt::WA_StyledBackground, true);

    workspaceLayout->setSpacing(12);
    heroBandLayout->setContentsMargins(0, 0, 18, 0);
    heroArtwork->setFixedSize(330, 138);
    heroArtwork->setPixmap(
        QIcon(QStringLiteral(":/ui/illustrations/devices/local-device-hero.svg"))
            .pixmap(QSize(320, 132)));

    mainColumns->setSpacing(22);
    identityPanel->setMinimumHeight(330);
    directAssistCard->setMinimumHeight(330);
    identityLayout->setContentsMargins(24, 22, 24, 22);
    identityLayout->setSpacing(12);
    directAssistLayout->setContentsMargins(24, 22, 24, 22);
    directAssistLayout->setSpacing(12);

    // Visual colors live in local-device.qss so light/dark themes can be
    // switched at runtime without rebuilding or replacing these widgets.
    identityPanel->setStyleSheet({});
    directAssistCard->setStyleSheet({});
