// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#pragma once

#include "MorphIconCore.h"

#include <QColor>
#include <QElapsedTimer>
#include <QObject>
#include <QSize>
#include <QTimer>

class QAbstractButton;
class QEvent;

namespace remotec::ui::morph {

class MorphIconButtonBinding final : public QObject {
public:
    enum class Interaction {
        State,
        Hover,
        Feedback,
    };

    static MorphIconButtonBinding* attach(
        QAbstractButton* button,
        const QString& sourceResource,
        const QString& targetResource,
        Interaction interaction,
        const QSize& iconSize = QSize(18, 18),
        const QColor& sourceColor = QColor(QStringLiteral("#667085")),
        const QColor& targetColor = QColor(QStringLiteral("#2563EB")));

    void setTarget(bool target, bool animated = true);
    void pulse(int holdMilliseconds = 1100);

    [[nodiscard]] bool isValid() const noexcept { return core_.isValid(); }
    [[nodiscard]] bool target() const noexcept { return target_; }

protected:
    bool eventFilter(QObject* watched, QEvent* event) override;

private:
    MorphIconButtonBinding(QAbstractButton* button,
                           QString sourceResource,
                           QString targetResource,
                           Interaction interaction,
                           QSize iconSize,
                           QColor sourceColor,
                           QColor targetColor);

    void startTransition(double targetProgress, bool animated);
    void render();
    QColor currentColor() const;

    QAbstractButton* button_ = nullptr;
    QString sourceResource_;
    QString targetResource_;
    Interaction interaction_ = Interaction::State;
    QSize iconSize_{18, 18};
    QColor sourceColor_;
    QColor targetColor_;
    MorphIconCore core_;
    Spring spring_;
    QTimer timer_;
    QElapsedTimer elapsed_;
    double progress_ = 0.0;
    double start_ = 0.0;
    double end_ = 0.0;
    bool target_ = false;
    bool initialized_ = false;
};

} // namespace remotec::ui::morph
