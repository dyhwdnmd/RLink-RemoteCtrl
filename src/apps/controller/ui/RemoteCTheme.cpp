// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#include "RemoteCTheme.h"

#include <QFile>
#include <QAbstractButton>
#include <QAction>
#include <QGuiApplication>
#include <QLabel>
#include <QPainter>
#include <QPalette>
#include <QPixmap>
#include <QSettings>
#include <QStyleHints>
#include <QWidget>

namespace remote::controller::ui {

namespace {
constexpr auto kThemeIconPath = "remoteCThemeIconPath";
constexpr auto kThemeIconTone = "remoteCThemeIconTone";
constexpr auto kThemeIconSize = "remoteCThemeIconSize";
}

QColor RemoteCTheme::Color(ThemeColor color)
{
    switch (color) {
    case ThemeColor::kPrimary: return QColor(QStringLiteral("#5365F5"));
    case ThemeColor::kPrimaryHover: return QColor(QStringLiteral("#4657E8"));
    case ThemeColor::kPrimaryPressed: return QColor(QStringLiteral("#3949CF"));
    case ThemeColor::kPrimarySubtle: return QColor(QStringLiteral("#EEF0FF"));
    case ThemeColor::kAppBackground: return QColor(QStringLiteral("#F5F4F0"));
    case ThemeColor::kSurface: return QColor(QStringLiteral("#FFFEFB"));
    case ThemeColor::kSurfaceSubtle: return QColor(QStringLiteral("#F8F7F3"));
    case ThemeColor::kBorder: return QColor(QStringLiteral("#DDE0E4"));
    case ThemeColor::kBorderStrong: return QColor(QStringLiteral("#C9CED6"));
    case ThemeColor::kTextPrimary: return QColor(QStringLiteral("#172033"));
    case ThemeColor::kTextSecondary: return QColor(QStringLiteral("#536176"));
    case ThemeColor::kTextTertiary: return QColor(QStringLiteral("#667085"));
    case ThemeColor::kDisabled: return QColor(QStringLiteral("#667085"));
    case ThemeColor::kSuccess: return QColor(QStringLiteral("#159A63"));
    case ThemeColor::kWarning: return QColor(QStringLiteral("#B7791F"));
    case ThemeColor::kError: return QColor(QStringLiteral("#C63D48"));
    case ThemeColor::kInfo: return QColor(QStringLiteral("#3B82F6"));
    case ThemeColor::kRemoteBackground: return QColor(QStringLiteral("#0B111B"));
    case ThemeColor::kRemoteSurface: return QColor(QStringLiteral("#121C2B"));
    case ThemeColor::kRemoteElevated: return QColor(QStringLiteral("#182538"));
    case ThemeColor::kRemoteBorder: return QColor(QStringLiteral("#2A394F"));
    case ThemeColor::kRemoteText: return QColor(QStringLiteral("#F4F7FB"));
    case ThemeColor::kRemoteTextSecondary: return QColor(QStringLiteral("#9AA8BB"));
    case ThemeColor::kRemotePrimary: return QColor(QStringLiteral("#7180FF"));
    case ThemeColor::kRemoteSuccess: return QColor(QStringLiteral("#4ED89A"));
    case ThemeColor::kRemoteError: return QColor(QStringLiteral("#FF7383"));
    }
    return {};
}

QString RemoteCTheme::LoadStyleSheet(const QString& resourcePath)
{
    QFile file(resourcePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return {};
    }
    return QString::fromUtf8(file.readAll());
}

QString RemoteCTheme::LightStyleSheet()
{
    return LoadStyleSheet(QStringLiteral(":/ui/theme/base.qss")) +
           LoadStyleSheet(QStringLiteral(":/ui/theme/light.qss"));
}

QString RemoteCTheme::DarkStyleSheet()
{
    return LoadStyleSheet(QStringLiteral(":/ui/theme/base.qss")) +
           LoadStyleSheet(QStringLiteral(":/ui/theme/dark.qss"));
}

QString RemoteCTheme::MainWindowColorOverrides(bool dark)
{
    return dark
        ? LoadStyleSheet(QStringLiteral(":/ui/theme/dark.qss"))
        : QString();
}

QString RemoteCTheme::RemoteDarkStyleSheet()
{
    return LoadStyleSheet(QStringLiteral(":/ui/theme/base.qss")) +
           LoadStyleSheet(QStringLiteral(":/ui/theme/remote-dark.qss"));
}

QString RemoteCTheme::PageStyleSheet(const QString& resourcePath, bool dark)
{
    QString result = LoadStyleSheet(resourcePath);
    if (dark) {
        result += LoadStyleSheet(QStringLiteral(":/ui/theme/dark.qss"));
    }
    return result;
}

ThemePreference RemoteCTheme::LoadPreference()
{
    return PreferenceFromValue(
        QSettings().value(QStringLiteral("ui/themeMode"),
                          QStringLiteral("system")).toString());
}

void RemoteCTheme::SavePreference(ThemePreference preference)
{
    QSettings settings;
    settings.setValue(QStringLiteral("ui/themeMode"),
                      PreferenceValue(preference));
}

QString RemoteCTheme::PreferenceValue(ThemePreference preference)
{
    switch (preference) {
    case ThemePreference::kLight: return QStringLiteral("light");
    case ThemePreference::kDark: return QStringLiteral("dark");
    case ThemePreference::kSystem: return QStringLiteral("system");
    }
    return QStringLiteral("system");
}

ThemePreference RemoteCTheme::PreferenceFromValue(const QString& value)
{
    const QString normalized = value.trimmed().toLower();
    if (normalized == QStringLiteral("light")) {
        return ThemePreference::kLight;
    }
    if (normalized == QStringLiteral("dark")) {
        return ThemePreference::kDark;
    }
    return ThemePreference::kSystem;
}

bool RemoteCTheme::IsDark(ThemePreference preference)
{
    if (preference == ThemePreference::kDark) return true;
    if (preference == ThemePreference::kLight) return false;
    if (const auto* hints = QGuiApplication::styleHints()) {
        if (hints->colorScheme() == Qt::ColorScheme::Dark) return true;
        if (hints->colorScheme() == Qt::ColorScheme::Light) return false;
    }
    return QGuiApplication::palette().color(QPalette::Window).lightness() < 128;
}

QColor RemoteCTheme::IconColor(ThemeIconTone tone, bool dark)
{
    switch (tone) {
    case ThemeIconTone::kPrimary:
        return QColor(dark ? QStringLiteral("#8EA5FF")
                           : QStringLiteral("#315EFB"));
    case ThemeIconTone::kSuccess:
        return QColor(dark ? QStringLiteral("#4FF0B5")
                           : QStringLiteral("#168A5B"));
    case ThemeIconTone::kWarning:
        return QColor(dark ? QStringLiteral("#E5B85E")
                           : QStringLiteral("#B7791F"));
    case ThemeIconTone::kDanger:
        return QColor(dark ? QStringLiteral("#FF98A5")
                           : QStringLiteral("#C63D48"));
    case ThemeIconTone::kOnDark:
        return QColor(dark ? QStringLiteral("#F7F9FF")
                           : QStringLiteral("#FFFFFF"));
    case ThemeIconTone::kNeutral:
        return QColor(dark ? QStringLiteral("#AEBBD0")
                           : QStringLiteral("#64748B"));
    }
    return {};
}

QIcon RemoteCTheme::Icon(const QString& resourcePath,
                         ThemeIconTone tone, QSize logicalSize)
{
    if (resourcePath.isEmpty() || !logicalSize.isValid()) return {};
    constexpr qreal kScale = 2.0;
    QPixmap pixmap(qRound(logicalSize.width() * kScale),
                   qRound(logicalSize.height() * kScale));
    pixmap.setDevicePixelRatio(kScale);
    pixmap.fill(Qt::transparent);
    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing, true);
    QIcon(resourcePath).paint(&painter, QRect(QPoint(), logicalSize));
    painter.setCompositionMode(QPainter::CompositionMode_SourceIn);
    painter.fillRect(QRect(QPoint(), logicalSize),
                     IconColor(tone, IsDark(LoadPreference())));
    painter.end();
    return QIcon(pixmap);
}

void RemoteCTheme::SetIcon(QAbstractButton* button,
                           const QString& resourcePath,
                           ThemeIconTone tone)
{
    if (!button) return;
    button->setProperty(kThemeIconPath, resourcePath);
    button->setProperty(kThemeIconTone, static_cast<int>(tone));
    button->setIcon(Icon(resourcePath, tone, button->iconSize()));
}

void RemoteCTheme::SetIcon(QAction* action, const QString& resourcePath,
                           ThemeIconTone tone)
{
    if (!action) return;
    action->setProperty(kThemeIconPath, resourcePath);
    action->setProperty(kThemeIconTone, static_cast<int>(tone));
    action->setIcon(Icon(resourcePath, tone));
}

void RemoteCTheme::SetPixmap(QLabel* label, const QString& resourcePath,
                             QSize logicalSize, ThemeIconTone tone)
{
    if (!label) return;
    label->setProperty(kThemeIconPath, resourcePath);
    label->setProperty(kThemeIconTone, static_cast<int>(tone));
    label->setProperty(kThemeIconSize, logicalSize);
    label->setPixmap(Icon(resourcePath, tone, logicalSize)
                         .pixmap(logicalSize));
}

void RemoteCTheme::RefreshIcons(QWidget* root)
{
    if (!root) return;
    const auto buttons = root->findChildren<QAbstractButton*>();
    for (auto* button : buttons) {
        const QString path = button->property(kThemeIconPath).toString();
        if (path.isEmpty()) continue;
        SetIcon(button, path, static_cast<ThemeIconTone>(
            button->property(kThemeIconTone).toInt()));
    }
    const auto actions = root->findChildren<QAction*>();
    for (auto* action : actions) {
        const QString path = action->property(kThemeIconPath).toString();
        if (path.isEmpty()) continue;
        SetIcon(action, path, static_cast<ThemeIconTone>(
            action->property(kThemeIconTone).toInt()));
    }
    const auto labels = root->findChildren<QLabel*>();
    for (auto* label : labels) {
        const QString path = label->property(kThemeIconPath).toString();
        if (path.isEmpty()) continue;
        SetPixmap(label, path, label->property(kThemeIconSize).toSize(),
                  static_cast<ThemeIconTone>(
                      label->property(kThemeIconTone).toInt()));
    }
}

void RemoteCTheme::ApplyLight(QWidget* widget)
{
    if (widget) {
        widget->setStyleSheet(LightStyleSheet());
    }
}

void RemoteCTheme::ApplyDark(QWidget* widget)
{
    if (widget) {
        widget->setStyleSheet(DarkStyleSheet());
    }
}

void RemoteCTheme::ApplyRemoteDark(QWidget* widget)
{
    if (widget) {
        widget->setStyleSheet(RemoteDarkStyleSheet());
    }
}

}  // namespace remote::controller::ui
