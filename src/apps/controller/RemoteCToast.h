// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#pragma once

#include <QFrame>
#include <QPoint>
#include <QPointer>
#include <QString>

class QEvent;
class QGraphicsOpacityEffect;
class QLabel;
class QPropertyAnimation;
class QTimer;

namespace remote::controller {

class RemoteCToast final : public QFrame {
public:
    enum class Tone {
        kSuccess,
        kInformation,
        kError,
    };

    static void Show(QWidget* parent,
                     const QString& message,
                     Tone tone = Tone::kInformation);
    static void ShowAbove(QWidget* anchor,
                          const QString& message,
                          Tone tone = Tone::kInformation);
    static void ShowAtGlobalCenter(
        QWidget* parent,
        const QPoint& globalCenter,
        const QString& message,
        Tone tone = Tone::kInformation,
        bool prominent = false);

protected:
    bool eventFilter(QObject* watched, QEvent* event) override;

private:
    explicit RemoteCToast(QWidget* host);
    void ShowMessage(const QString& message,
                     Tone tone,
                     QWidget* anchor,
                     const QPoint* globalCenter = nullptr,
                     bool prominent = false);
    void Reposition();
    void FadeOut();

    QWidget* host_ = nullptr;
    QPointer<QWidget> anchor_;
    QPoint globalCenter_;
    bool useGlobalCenter_ = false;
    QLabel* marker_ = nullptr;
    QLabel* message_ = nullptr;
    QTimer* timer_ = nullptr;
    QGraphicsOpacityEffect* opacity_ = nullptr;
    QPropertyAnimation* animation_ = nullptr;
};

}  // namespace remote::controller
