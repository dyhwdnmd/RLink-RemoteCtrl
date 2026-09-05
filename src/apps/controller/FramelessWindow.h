// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#pragma once

#include <functional>

#include <QIcon>
#include <QMainWindow>
#include <QPoint>
#include <QRect>
#include <QWidget>

class QLabel;
class QAbstractScrollArea;
class QToolButton;

namespace remote::controller {

QIcon CreateRemoteCIcon();
QString ScaleUiStyleSheet(const QString& styleSheet);
int CurrentUiAnimationLevel();
void EnableSmoothWheelScrolling(QAbstractScrollArea* scrollArea);

class FramelessMainWindow : public QMainWindow {
public:
    explicit FramelessMainWindow(QWidget* parent = nullptr,
                                 bool embedded = false);
    ~FramelessMainWindow() override = default;

    static void RefreshAllWindowStyles();
    void MinimizeWithSystemAnimation();
    void AnimateWindowEntrance(const QPoint& fullAnimationOffset =
                                   QPoint(10, 0),
                               const QRect& globalClipRect = QRect(),
                               int durationOverrideMs = 0);
    void AnimateWindowExit(const QPoint& fullAnimationOffset = QPoint(),
                           const QRect& globalClipRect = QRect(),
                           int durationOverrideMs = 0);

protected:
    void ApplyUiStyleSheet(const QString& styleSheet);
    void SetNativeRoundedCornersEnabled(bool enabled);
    void changeEvent(QEvent* event) override;
    bool nativeEvent(const QByteArray& eventType,
                     void* message,
                     qintptr* result) override;
    virtual QRect ConstrainResizeGeometry(const QRect& proposedGeometry,
                                          Qt::Edges resizeEdges,
                                          qreal devicePixelRatio) const;

private:
    void RefreshWindowStyle();

    QString baseStyleSheet_;
    QWidget* resizeOverlay_ = nullptr;
#ifdef Q_OS_WIN
    Qt::Edges resizeEdges_{};
    QPoint resizeStartCursor_;
    QRect resizeStartWindowRect_;
#endif
};

class CustomTitleBar final : public QWidget {
public:
    CustomTitleBar(FramelessMainWindow* window,
                   QString title,
                   QWidget* parent = nullptr);

    void SetMinimizeAction(std::function<void()> action);
    void SetEmbeddedMode(bool embedded);
    void SetTitle(const QString& title);
    void RefreshThemeStyle(bool dark);

protected:
    bool eventFilter(QObject* watched, QEvent* event) override;
    void mouseDoubleClickEvent(QMouseEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;

private:
    void ToggleMaximized();
    void UpdateMaximizeButton();

    FramelessMainWindow* window_ = nullptr;
    QLabel* titleLabel_ = nullptr;
    std::function<void()> minimizeAction_;
    QToolButton* minimizeButton_ = nullptr;
    QToolButton* maximizeButton_ = nullptr;
    QToolButton* closeButton_ = nullptr;
    bool embedded_ = false;
};

}  // namespace remote::controller
