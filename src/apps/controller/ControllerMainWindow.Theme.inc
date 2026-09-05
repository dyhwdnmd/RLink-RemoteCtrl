// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

void ControllerMainWindow::ApplyInterfaceTheme(bool showFeedback)
{
    // Apply atomically: setStyleSheet() already performs the required Qt
    // repolish. A manual root-only unpolish/polish leaves descendants (most
    // visibly custom combo boxes and category buttons) unpolished after a
    // dark -> light round trip and makes them fall back to native geometry.
    setUpdatesEnabled(false);
    struct ScrollPosition {
        QPointer<QAbstractScrollArea> area;
        int horizontal = 0;
        int vertical = 0;
    };
    QVector<ScrollPosition> scrollPositions;
    const auto scrollAreas = findChildren<QAbstractScrollArea*>();
    scrollPositions.reserve(scrollAreas.size());
    for (auto* area : scrollAreas) {
        scrollPositions.push_back({area,
                                   area->horizontalScrollBar()->value(),
                                   area->verticalScrollBar()->value()});
    }
    const ui::ThemePreference preference =
        ui::RemoteCTheme::LoadPreference();
    const bool dark = ui::RemoteCTheme::IsDark(preference);
    darkInterfaceTheme_ = dark;

    setProperty("themeRoot", dark ? QStringLiteral("dark")
                                   : QStringLiteral("light"));
    ApplyUiStyleSheet(
        QString::fromUtf8(kMainStyle) +
        ui::RemoteCTheme::MainWindowColorOverrides(dark));

    const auto applyPage = [dark](QWidget* page,
                                  const QString& resourcePath) {
        if (!page) return;
        page->setProperty("themeRoot", dark ? QStringLiteral("dark")
                                             : QStringLiteral("light"));
        page->setStyleSheet(
            ui::RemoteCTheme::PageStyleSheet(resourcePath, dark));
    };
    applyPage(localDevicePage_,
              QStringLiteral(":/ui/theme/local-device.qss"));
    applyPage(ownedDevicesPage_,
              QStringLiteral(":/ui/theme/owned-devices.qss"));
    applyPage(recentConnectionsPage_,
              QStringLiteral(":/ui/theme/recent-connections.qss"));
    if (ownedDevicesEmptyArtwork_) {
        const QString artwork = dark
            ? QStringLiteral(
                  ":/ui/illustrations/devices/no-devices-dark.svg")
            : QStringLiteral(
                  ":/ui/illustrations/devices/no-devices.svg");
        ownedDevicesEmptyArtwork_->setPixmap(
            QIcon(artwork).pixmap(210, 122));
    }

    if (titleBar_) titleBar_->RefreshThemeStyle(dark);
    const QColor navigationNormal(
        dark ? QStringLiteral("#AEBBD0") : QStringLiteral("#64748B"));
    const QColor navigationActive(
        dark ? QStringLiteral("#8EA5FF") : QStringLiteral("#315EFB"));
    const auto navigationButtons = findChildren<QPushButton*>();
    for (auto* button : navigationButtons) {
        if (!button->property("nav").toBool()) continue;
        const auto icon = static_cast<NavigationIcon>(
            button->property("navigationIcon").toInt());
        button->setIcon(MakeNavigationIcon(icon, dark));
        button->setProperty("remoteCMorphCheckedColor",
                            navigationActive.name());
        const QString source =
            button->property("navigationMorphSource").toString();
        const QString target =
            button->property("navigationMorphTarget").toString();
        if (!source.isEmpty() && !target.isEmpty()) {
            remotec::ui::morph::MorphIconButtonBinding::attach(
                button,
                QStringLiteral(":/ui/icons/lucide/base/%1.svg").arg(source),
                QStringLiteral(":/ui/icons/lucide/base/%1.svg").arg(target),
                remotec::ui::morph::MorphIconButtonBinding::Interaction::Hover,
                QSize(18, 18), navigationNormal, navigationActive);
        }
    }
    const auto selectors = findChildren<QComboBox*>();
    for (auto* selector : selectors) {
        if (auto* remoteSelector =
                dynamic_cast<RemoteCComboBox*>(selector)) {
            remoteSelector->RefreshThemeStyle();
        }
    }
    ui::RemoteCTheme::RefreshIcons(this);
    const auto featureIcons = findChildren<QLabel*>(
        QStringLiteral("localDeviceFeatureIcon"));
    for (auto* icon : featureIcons) {
        const QString resource =
            icon->property("localFeatureIconResource").toString();
        if (resource.isEmpty()) continue;
        const auto tone = resource.contains(QStringLiteral("shield-check"))
            ? ui::ThemeIconTone::kSuccess
            : (dark ? ui::ThemeIconTone::kOnDark
                    : ui::ThemeIconTone::kPrimary);
        ui::RemoteCTheme::SetPixmap(icon, resource, QSize(20, 20), tone);
    }
    if (ownedDevicesRefreshButton_) {
        const QColor refreshColor(
            dark ? QStringLiteral("#F7F9FF")
                 : QStringLiteral("#2563EB"));
        ui::RemoteCTheme::SetIcon(
            ownedDevicesRefreshButton_,
            QStringLiteral(":/ui/icons/actions/refresh.svg"),
            dark ? ui::ThemeIconTone::kOnDark
                 : ui::ThemeIconTone::kPrimary);
        remotec::ui::morph::MorphIconButtonBinding::attach(
            ownedDevicesRefreshButton_,
            QStringLiteral(":/ui/icons/lucide/base/refresh-cw.svg"),
            QStringLiteral(":/ui/icons/lucide/base/refresh-ccw.svg"),
            remotec::ui::morph::MorphIconButtonBinding::Interaction::Feedback,
            QSize(18, 18), refreshColor, refreshColor);
    }
    // Benchmark summaries use rich text, so QSS cannot recolor their inline
    // spans. Rebuild them whenever the theme changes.
    RefreshEncoderBenchmarkSummary();
    RefreshDecoderBenchmarkSummary();
    for (auto* widget : QApplication::topLevelWidgets()) {
        if (auto* login = dynamic_cast<LoginWindow*>(widget)) {
            login->RefreshThemeStyle();
        }
        if (auto* status = dynamic_cast<LoginStatusWindow*>(widget)) {
            status->RefreshThemeStyle();
        }
        if (auto* transfer = dynamic_cast<FileTransferWindow*>(widget)) {
            transfer->RefreshThemeStyle();
        }
        for (auto* descendant : widget->findChildren<QWidget*>()) {
            if (auto* login = dynamic_cast<LoginWindow*>(descendant)) {
                login->RefreshThemeStyle();
            }
            if (auto* status =
                    dynamic_cast<LoginStatusWindow*>(descendant)) {
                status->RefreshThemeStyle();
            }
            if (auto* transfer =
                    dynamic_cast<FileTransferWindow*>(descendant)) {
                transfer->RefreshThemeStyle();
            }
        }
    }

    if (themeModeSelector_) {
        const QSignalBlocker blocker(themeModeSelector_);
        const int index = themeModeSelector_->findData(
            ui::RemoteCTheme::PreferenceValue(preference));
        themeModeSelector_->setCurrentIndex(index >= 0 ? index : 0);
    }

    // QStyleSheetStyle recalculates scroll ranges while selectors are being
    // repolished. Preserve the user's exact viewport so a color switch cannot
    // shift page content by a few pixels.
    for (const auto& position : scrollPositions) {
        if (!position.area) continue;
        position.area->horizontalScrollBar()->setValue(position.horizontal);
        position.area->verticalScrollBar()->setValue(position.vertical);
    }
    QTimer::singleShot(0, this, [this, scrollPositions] {
        for (const auto& position : scrollPositions) {
            if (!position.area) continue;
            position.area->horizontalScrollBar()->setValue(
                position.horizontal);
            position.area->verticalScrollBar()->setValue(position.vertical);
        }
        update();
    });

    // Room-member cards contain inline colors which are not recolored by the
    // root style sheet. Force their render key to refresh in the same frame.
    renderedRoomMemberKey_.clear();
    if (engine_ && roomMemberList_) {
        UpdateRoomUi(engine_->Snapshot());
    }

    setUpdatesEnabled(true);
    update();
    if (showFeedback) {
        const QString modeName =
            preference == ui::ThemePreference::kSystem
                ? QStringLiteral("跟随系统")
                : (dark ? QStringLiteral("深色")
                        : QStringLiteral("浅色"));
        RemoteCToast::Show(
            this, QStringLiteral("界面主题已切换为%1").arg(modeName),
            RemoteCToast::Tone::kSuccess);
    }
}

void ControllerMainWindow::SetInterfaceThemePreference(
    const QString& value)
{
    ui::RemoteCTheme::SavePreference(
        ui::RemoteCTheme::PreferenceFromValue(value));
    ApplyInterfaceTheme(true);
}

bool ControllerMainWindow::RunThemeRoundTripSelfTest(QString* errorMessage)
{
    QSettings settings;
    const QString key = QStringLiteral("ui/themeMode");
    const bool hadOriginal = settings.contains(key);
    const QVariant original = settings.value(key);
    const int originalPage = pageStack_ ? pageStack_->currentIndex() : -1;
    if (pageStack_) pageStack_->setCurrentIndex(5);

    const auto settle = [this] {
        QCoreApplication::processEvents(QEventLoop::AllEvents);
        const auto widgets = findChildren<QWidget*>();
        for (auto* widget : widgets) {
            if (widget->layout()) widget->layout()->activate();
        }
        if (layout()) layout()->activate();
        QCoreApplication::processEvents(QEventLoop::AllEvents);
    };
    const auto fingerprint = [this] {
        QStringList result;
        const auto widgets = findChildren<QWidget*>();
        int ordinal = 0;
        for (auto* widget : widgets) {
            if (!widget->isVisibleTo(this)) continue;
            const bool tracked =
                widget->objectName() == QStringLiteral("settingRow") ||
                widget->objectName() == QStringLiteral("customTitleBar") ||
                widget->objectName() == QStringLiteral("sidebar") ||
                widget->objectName() == QStringLiteral("profileCard") ||
                widget->property("nav").toBool() ||
                widget->objectName() == QStringLiteral("settingsCategoryPanel") ||
                widget->objectName() == QStringLiteral("settingsCategoryButton") ||
                widget->objectName() == QStringLiteral("pageTitle") ||
                widget->objectName() == QStringLiteral("pageSubtitle") ||
                widget->objectName() == QStringLiteral("settingsDetailTitle") ||
                dynamic_cast<RemoteCComboBox*>(widget) != nullptr;
            if (!tracked) continue;
            const QRect geometry = widget->geometry();
            const QSize hint = widget->sizeHint();
            const QStringList values = {
                QString::number(ordinal++),
                QString::fromLatin1(widget->metaObject()->className()),
                widget->objectName(),
                QStringLiteral("g=%1,%2,%3,%4")
                    .arg(geometry.x()).arg(geometry.y())
                    .arg(geometry.width()).arg(geometry.height()),
                QStringLiteral("hint=%1,%2")
                    .arg(hint.width()).arg(hint.height()),
                QStringLiteral("min=%1,%2")
                    .arg(widget->minimumWidth()).arg(widget->minimumHeight()),
                QStringLiteral("max=%1,%2")
                    .arg(widget->maximumWidth()).arg(widget->maximumHeight())};
            result.push_back(values.join(QLatin1Char('|')));
        }
        return result;
    };
    const auto apply = [this, &settle](ui::ThemePreference preference) {
        ui::RemoteCTheme::SavePreference(preference);
        ApplyInterfaceTheme(false);
        settle();
    };

    apply(ui::ThemePreference::kLight);
    const QStringList lightBefore = fingerprint();
    apply(ui::ThemePreference::kDark);
    const QStringList dark = fingerprint();
    apply(ui::ThemePreference::kLight);
    const QStringList lightAfter = fingerprint();

    const bool passed = !lightBefore.isEmpty() &&
                        lightBefore == dark &&
                        lightBefore == lightAfter;
    if (!passed && errorMessage) {
        const int count = std::max({lightBefore.size(), dark.size(),
                                    lightAfter.size()});
        for (int index = 0; index < count; ++index) {
            const QString before = index < lightBefore.size()
                ? lightBefore[index] : QStringLiteral("<missing>");
            const QString during = index < dark.size()
                ? dark[index] : QStringLiteral("<missing>");
            const QString after = index < lightAfter.size()
                ? lightAfter[index] : QStringLiteral("<missing>");
            if (before != during || before != after) {
                *errorMessage = QStringLiteral(
                    "theme geometry mismatch at %1\nLIGHT_1=%2\nDARK=%3\nLIGHT_2=%4")
                    .arg(index).arg(before, during, after);
                break;
            }
        }
        if (errorMessage->isEmpty()) {
            *errorMessage = QStringLiteral("theme geometry fingerprint is empty");
        }
    }

    if (hadOriginal) settings.setValue(key, original);
    else settings.remove(key);
    settings.sync();
    ApplyInterfaceTheme(false);
    if (pageStack_ && originalPage >= 0) {
        pageStack_->setCurrentIndex(originalPage);
    }
    settle();
    return passed;
}
