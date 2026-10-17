// SPDX-License-Identifier: GPL-3.0-only
#pragma once
#include <QString>
#include <functional>
class QObject;

namespace remote::controller::detail {
// Includes --autostart, which the product entry point treats as tray-only startup.
QString WindowsAutoStartCommand();
bool WindowsAutoStartEnabled();
bool SetWindowsAutoStartEnabled(bool enabled, QString* error = nullptr);
void QueryWindowsAutoStartAsync(QObject* context,
    std::function<void(bool enabled)> completed);
// Applies the first-run default or migrates an enabled legacy task for this EXE.
// An explicit off preference, disabled task, or another installation is preserved.
void InitializeWindowsAutoStartAsync(QObject* context,
    std::function<void(bool actualEnabled, QString error)> completed);
void SetWindowsAutoStartAsync(bool enabled, QObject* context,
    std::function<void(bool success, bool actualEnabled, QString error)> completed);
}
