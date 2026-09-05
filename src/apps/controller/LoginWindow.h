// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#pragma once

#include <QDialog>
#include <QElapsedTimer>
#include <QPointer>
#include <QPoint>
#include <QRect>
#include <QSize>

#include <functional>

class QCloseEvent;
class QLabel;
class QMouseEvent;
class QProgressBar;
class QPushButton;
class QFrame;
class QTimer;

namespace remote::controller {

class LoginStatusWindow final : public QDialog {
public:
    explicit LoginStatusWindow(QWidget* parent = nullptr);
    void SetAccountLabel(const QString& accountLabel);
    void ShowAndActivate();
    void HideAndReleaseTopmost();
    void RefreshThemeStyle();

protected:
    void mousePressEvent(QMouseEvent* event) override;

private:
    QLabel* accountLabel_ = nullptr;
};

class LoginWindow final : public QDialog {
public:
    explicit LoginWindow(QWidget* parent = nullptr);

    void SetLoginRequestedCallback(std::function<void()> callback);
    void SetCancelRequestedCallback(std::function<void()> callback);
    void SetExitRequestedCallback(std::function<void()> callback);
    void SetSignOutRequestedCallback(std::function<void()> callback);
    void SetDeleteAccountRequestedCallback(std::function<void()> callback);
    void SetOwnerWindow(QWidget* owner);

    void ShowReady(const QString& message = {});
    void ShowBusy(const QString& title, const QString& message);
    void ShowAccountDeletionBusy();
    void ShowError(const QString& message, bool retryable = true);
    void ShowAuthenticated(const QString& accountLabel,
                           const QString& accountDetail);
    void ShowAndActivate();
    void HideWithAnimation();
    bool IsHiding() const;
    void RefreshThemeStyle();

protected:
    void closeEvent(QCloseEvent* event) override;
    bool eventFilter(QObject* watched, QEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;

private:
    void BuildUi();
    void SetStatePanelTone(const QString& tone);
    void UpdateAuthenticatedAccountText();
    QRect CardTargetGeometry() const;
    void LayoutCardLayer();
    void StartVisibilityMotion(const QPoint& targetPosition, int durationMs);
    void StopVisibilityMotion();
    bool IsVisibilityMotionActive() const;

    QLabel* titleLabel_ = nullptr;
    QLabel* stateIcon_ = nullptr;
    QLabel* statusLabel_ = nullptr;
    QLabel* securityLabel_ = nullptr;
    QProgressBar* progressBar_ = nullptr;
    QPushButton* loginButton_ = nullptr;
    QPushButton* cancelButton_ = nullptr;
    QPushButton* exitButton_ = nullptr;
    QPushButton* signOutButton_ = nullptr;
    QPushButton* deleteAccountButton_ = nullptr;
    QFrame* statePanel_ = nullptr;
    QWidget* cardLayer_ = nullptr;
    QFrame* card_ = nullptr;
    QWidget* dragHandle_ = nullptr;
    QTimer* visibilityMotionTimer_ = nullptr;
    QElapsedTimer visibilityMotionClock_;
    QPoint visibilityMotionStart_;
    QPoint visibilityMotionTarget_;
    int visibilityMotionDurationMs_ = 0;
    QPointer<QWidget> ownerWindow_;
    QSize preferredCardSize_{620, 520};
    QPoint dragOffset_;
    QPoint lastCardPosition_;
    bool dragging_ = false;
    bool hasLastCardPosition_ = false;
    bool hiding_ = false;
    QString authenticatedAccountLabel_;
    QString authenticatedAccountDetail_;
    std::function<void()> loginRequested_;
    std::function<void()> cancelRequested_;
    std::function<void()> exitRequested_;
    std::function<void()> signOutRequested_;
    std::function<void()> deleteAccountRequested_;
};

}  // namespace remote::controller
