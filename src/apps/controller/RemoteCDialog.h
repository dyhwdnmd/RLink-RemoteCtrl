// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#pragma once

#include <cstdint>
#include <functional>

#include <QDialog>
#include <QPoint>
#include <QString>

class QLabel;
class QFrame;
class QPushButton;
class QProgressBar;

namespace remote::controller {

class RemoteCDialog final : public QDialog {
public:
    enum class Tone {
        kQuestion,
        kWarning,
        kDanger,
        kInformation,
    };

    static bool Confirm(QWidget* parent,
                        const QString& title,
                        const QString& message,
                        const QString& confirmText,
                        const QString& cancelText = QStringLiteral("取消"),
                        Tone tone = Tone::kQuestion,
                        bool requestAttention = false,
                        bool nonActivating = false,
                        std::uintptr_t nativeAnchorWindow = 0,
                        bool centerOnAnchorScreen = false);

    static bool ConfirmWithOption(
        QWidget* parent,
        const QString& title,
        const QString& message,
        const QString& confirmText,
        const QString& cancelText,
        const QString& optionText,
        bool optionInitiallyChecked,
        bool* optionChecked,
        Tone tone = Tone::kQuestion,
        bool requestAttention = false);

    static void Alert(QWidget* parent,
                      const QString& title,
                      const QString& message,
                      const QString& buttonText = QStringLiteral("知道了"),
                      Tone tone = Tone::kWarning,
                      bool requestAttention = false);

    // Creates a movable, non-modal status dialog using the same visual
    // language as room-join and control approval prompts. The parent owns it.
    static RemoteCDialog* CreateStatus(
        QWidget* parent,
        const QString& title,
        const QString& message,
        const QString& buttonText = QStringLiteral("隐藏提示"),
        Tone tone = Tone::kWarning);

    void SetContent(const QString& title,
                    const QString& message,
                    const QString& buttonText,
                    Tone tone);
    void SetStatusActionHandler(std::function<void()> handler);
    void SetProgress(double progress);
    // Keeps a status window above a foreign target (for example Explorer)
    // without activating this dialog or bringing its Qt owner to the front.
    // The dialog remains mouse-draggable through our manual drag path.
    void SetNonActivatingWindow(bool enabled);
    void SetNativeAnchorWindow(std::uintptr_t windowHandle);

protected:
    bool eventFilter(QObject* watched, QEvent* event) override;
    void showEvent(QShowEvent* event) override;

private:
    void ApplyNonActivatingNativeStyle();

    RemoteCDialog(QWidget* parent,
                  const QString& title,
                  const QString& message,
                  const QString& confirmText,
                  const QString& cancelText,
                  Tone tone,
                  bool showCancel,
                  bool requestAttention);

    bool shown_ = false;
    bool statusDialog_ = false;
    bool requestAttention_ = false;
    bool nonActivatingWindow_ = false;
    std::uintptr_t nativeAnchorWindow_ = 0;
    bool centerOnAnchorScreen_ = false;
    bool nonActivatingDragActive_ = false;
    QPoint nonActivatingDragStartGlobal_;
    QPoint nonActivatingDragStartTopLeft_;
    QFrame* card_ = nullptr;
    QLabel* badgeLabel_ = nullptr;
    QLabel* titleLabel_ = nullptr;
    QLabel* messageLabel_ = nullptr;
    QWidget* activityIndicator_ = nullptr;
    QProgressBar* progressBar_ = nullptr;
    QPushButton* confirmButton_ = nullptr;
    std::function<void()> statusActionHandler_;
};

}  // namespace remote::controller
