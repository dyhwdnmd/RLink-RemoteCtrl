// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#pragma once

#include <QColor>
#include <QIcon>
#include <QSize>
#include <QString>

class QAbstractButton;
class QAction;
class QLabel;
class QWidget;

namespace remote::controller::ui {

enum class ThemePreference {
    kSystem,
    kLight,
    kDark,
};

enum class ThemeColor {
    kPrimary,
    kPrimaryHover,
    kPrimaryPressed,
    kPrimarySubtle,
    kAppBackground,
    kSurface,
    kSurfaceSubtle,
    kBorder,
    kBorderStrong,
    kTextPrimary,
    kTextSecondary,
    kTextTertiary,
    kDisabled,
    kSuccess,
    kWarning,
    kError,
    kInfo,
    kRemoteBackground,
    kRemoteSurface,
    kRemoteElevated,
    kRemoteBorder,
    kRemoteText,
    kRemoteTextSecondary,
    kRemotePrimary,
    kRemoteSuccess,
    kRemoteError,
};

enum class ThemeIconTone {
    kNeutral,
    kPrimary,
    kSuccess,
    kWarning,
    kDanger,
    kOnDark,
};

class RemoteCTheme final {
public:
    static QColor Color(ThemeColor color);
    static QString LightStyleSheet();
    static QString DarkStyleSheet();
    // Main-window geometry lives in kMainStyle. Theme switching may append
    // only color overrides so a round trip can never change layout metrics.
    static QString MainWindowColorOverrides(bool dark);
    static QString RemoteDarkStyleSheet();
    static QString PageStyleSheet(const QString& resourcePath, bool dark);
    static ThemePreference LoadPreference();
    static void SavePreference(ThemePreference preference);
    static QString PreferenceValue(ThemePreference preference);
    static ThemePreference PreferenceFromValue(const QString& value);
    static bool IsDark(ThemePreference preference);
    static QIcon Icon(const QString& resourcePath,
                      ThemeIconTone tone = ThemeIconTone::kNeutral,
                      QSize logicalSize = QSize(24, 24));
    static void SetIcon(QAbstractButton* button, const QString& resourcePath,
                        ThemeIconTone tone = ThemeIconTone::kNeutral);
    static void SetIcon(QAction* action, const QString& resourcePath,
                        ThemeIconTone tone = ThemeIconTone::kNeutral);
    static void SetPixmap(QLabel* label, const QString& resourcePath,
                          QSize logicalSize,
                          ThemeIconTone tone = ThemeIconTone::kNeutral);
    static void RefreshIcons(QWidget* root);
    static void ApplyLight(QWidget* widget);
    static void ApplyDark(QWidget* widget);
    static void ApplyRemoteDark(QWidget* widget);

private:
    static QString LoadStyleSheet(const QString& resourcePath);
    static QColor IconColor(ThemeIconTone tone, bool dark);
};

}  // namespace remote::controller::ui
