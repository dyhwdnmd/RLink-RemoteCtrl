// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

        QToolButton* MakeToolButton(const QString& text,
            const QString& tooltip,
            QWidget* parent)
        {
            auto* button = new QToolButton(parent);
            button->setToolTip(tooltip);
            button->setCursor(Qt::PointingHandCursor);
            button->setFocusPolicy(Qt::NoFocus);
            button->setFixedHeight(38);

            QString iconResource;
            if (text.startsWith(QStringLiteral(":/"))) {
                iconResource = text;
            }
            else if (text == QStringLiteral("⚙") ||
                     text == QStringLiteral("设置") ||
                     tooltip.contains(QStringLiteral("摄像头"))) {
                iconResource = QStringLiteral(":/ui/icons/actions/settings.svg");
            }
            else if (text == QStringLiteral("▣") ||
                     text == QStringLiteral("显示") ||
                     tooltip.contains(QStringLiteral("远端显示器"))) {
                iconResource = QStringLiteral(":/ui/icons/actions/display.svg");
            }
            else if (text == QStringLiteral("🔓") ||
                     text == QStringLiteral("锁定") ||
                     tooltip.contains(QStringLiteral("锁定顶部"))) {
                iconResource = QStringLiteral(":/ui/icons/actions/unlock.svg");
            }
            else if (text == QStringLiteral("🔒") ||
                     text == QStringLiteral("解锁") ||
                     tooltip.contains(QStringLiteral("解锁顶部"))) {
                iconResource = QStringLiteral(":/ui/icons/actions/lock.svg");
            }

            if (!iconResource.isEmpty()) {
                button->setText(QString());
                button->setIcon(QIcon(iconResource));
                button->setIconSize(QSize(20, 20));
                button->setToolButtonStyle(Qt::ToolButtonIconOnly);
            }
            else {
                button->setText(text);
                button->setToolButtonStyle(Qt::ToolButtonTextOnly);
            }
            return button;
        }

        QString ScreenQualityText(ScreenQualityTier quality)
        {
            switch (quality) {
            case ScreenQualityTier::k720p:
                return QStringLiteral("标清 720p");
            case ScreenQualityTier::k1080p:
                return QStringLiteral("高清 1080p");
            case ScreenQualityTier::k1440p:
                return QStringLiteral("超清 2K");
            case ScreenQualityTier::kOriginal:
                return QStringLiteral("原始画质");
            case ScreenQualityTier::kAutomatic:
                return QStringLiteral("自动画质");
            }
            return QStringLiteral("画质");
        }

        QString CaptureBackendText(const std::string& backend)
        {
            if (backend == "native_dxgi") {
                return QStringLiteral("自研 DXGI");
            }
            if (backend == "native_dxgi_texture") {
                return QStringLiteral("自研 DXGI GPU 纹理");
            }
            if (backend == "libwebrtc") {
                return QStringLiteral("libwebrtc");
            }
            if (backend == "libwebrtc_dxgi_with_gdi_fallback") {
                return QStringLiteral("libwebrtc DXGI（可回退 GDI）");
            }
            if (backend == "libwebrtc_gdi") {
                return QStringLiteral("libwebrtc GDI");
            }
            if (backend == "not_initialized") {
                return QStringLiteral("尚未初始化");
            }
            return QString::fromStdString(backend);
        }

        QString ActiveMediaDeviceText(
            const MediaDeviceCategorySnapshot& category)
        {
            if (!category.activeDeviceName.empty()) {
                return QString::fromStdString(
                    category.activeDeviceName);
            }
            const std::string deviceId =
                category.activeDeviceId.empty()
                    ? category.preferredDeviceId
                    : category.activeDeviceId;
            if (deviceId.empty() ||
                deviceId == kSystemDefaultMediaDeviceId) {
                return QStringLiteral("系统默认设备");
            }
            const auto found = std::find_if(
                category.devices.begin(), category.devices.end(),
                [&deviceId](const MediaDeviceDescriptor& device) {
                    return device.id == deviceId;
                });
            return found == category.devices.end()
                ? QStringLiteral("设备不可用")
                : QString::fromStdString(found->name);
        }

        std::pair<std::uint32_t, std::uint32_t> ScreenQualityBounds(
            ScreenQualityTier quality)
        {
            switch (quality) {
            case ScreenQualityTier::k720p:
                return { 1280, 720 };
            case ScreenQualityTier::k1080p:
                return { 1920, 1080 };
            case ScreenQualityTier::k1440p:
                return { 2560, 1440 };
            case ScreenQualityTier::kAutomatic:
                return { 1920, 1080 };
            case ScreenQualityTier::kOriginal:
                return { 0, 0 };
            }
            return { 1920, 1080 };
        }
