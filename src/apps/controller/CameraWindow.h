// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#pragma once

#include <functional>

#include <QPoint>
#include <QWidget>

#include "FramelessWindow.h"

namespace remote::controller {

class CameraPreviewWidget final : public QWidget {
public:
    explicit CameraPreviewWidget(QWidget* parent = nullptr);

protected:
    void paintEvent(QPaintEvent* event) override;
};

class CameraOverlayWidget final : public QWidget {
public:
    CameraOverlayWidget(QString deviceName,
                        std::function<void()> detachAction,
                        std::function<void()> closeAction,
                        QWidget* parent = nullptr);

protected:
    bool eventFilter(QObject* watched, QEvent* event) override;

private:
    QWidget* dragHandle_ = nullptr;
    QPoint dragOffset_;
    bool dragging_ = false;
};

class CameraWindow final : public FramelessMainWindow {
public:
    CameraWindow(QString deviceId,
                 QString deviceName,
                 std::function<void()> reattachAction = {},
                 QWidget* parent = nullptr);
    ~CameraWindow() override = default;

    CameraWindow(const CameraWindow&) = delete;
    CameraWindow& operator=(const CameraWindow&) = delete;

private:
    void BuildUi();
    void ReturnToOverlay();

    QString deviceId_;
    QString deviceName_;
    std::function<void()> reattachAction_;
};

}  // namespace remote::controller
