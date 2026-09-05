// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#include "RoundedPopupMenu.h"

#include <QApplication>
#include <QHideEvent>
#include <QMouseEvent>
#include <QPainterPath>
#include <QRegion>
#include <QResizeEvent>
#include <QShowEvent>

#include <utility>

namespace remote::controller {

RoundedPopupMenu::RoundedPopupMenu(
    QWidget* parent,
    qreal cornerRadius)
    : QMenu(parent), cornerRadius_(cornerRadius)
{
    setWindowFlag(Qt::NoDropShadowWindowHint, true);
    setAttribute(Qt::WA_TranslucentBackground, false);
}

void RoundedPopupMenu::SetToggleAnchor(
    QWidget* anchor,
    std::function<void()> anchorPressed)
{
    toggleAnchor_ = anchor;
    toggleAnchorPressed_ = std::move(anchorPressed);
}

bool RoundedPopupMenu::HandleToggleAnchorPress(QMouseEvent* event)
{
    if (!event || event->button() != Qt::LeftButton ||
        !toggleAnchor_ ||
        !QRect(toggleAnchor_->mapToGlobal(QPoint(0, 0)),
               toggleAnchor_->size())
             .contains(event->globalPosition().toPoint())) {
        return false;
    }

    // Catch the press before QMenu processes an outside-popup click.  Without
    // WA_NoMouseReplay Qt posts the same press back to the anchor and opens
    // the menu again.
    setAttribute(Qt::WA_NoMouseReplay);
    if (toggleAnchorPressed_) {
        toggleAnchorPressed_();
    }
    close();
    event->accept();
    return true;
}

bool RoundedPopupMenu::eventFilter(QObject* watched, QEvent* event)
{
    if (isVisible() && event &&
        event->type() == QEvent::MouseButtonPress &&
        HandleToggleAnchorPress(
            static_cast<QMouseEvent*>(event))) {
        return true;
    }
    return QMenu::eventFilter(watched, event);
}

void RoundedPopupMenu::hideEvent(QHideEvent* event)
{
    if (applicationFilterInstalled_) {
        qApp->removeEventFilter(this);
        applicationFilterInstalled_ = false;
    }
    QMenu::hideEvent(event);
}

void RoundedPopupMenu::mousePressEvent(QMouseEvent* event)
{
    if (HandleToggleAnchorPress(event)) {
        return;
    }
    QMenu::mousePressEvent(event);
}

void RoundedPopupMenu::resizeEvent(QResizeEvent* event)
{
    QMenu::resizeEvent(event);
    QPainterPath clip;
    clip.addRoundedRect(
        QRectF(rect()).adjusted(0, 0, -1, -1),
        cornerRadius_, cornerRadius_);
    setMask(QRegion(clip.toFillPolygon().toPolygon()));
}

void RoundedPopupMenu::showEvent(QShowEvent* event)
{
    QMenu::showEvent(event);
    if (toggleAnchor_ && !applicationFilterInstalled_) {
        qApp->installEventFilter(this);
        applicationFilterInstalled_ = true;
    }
}

}  // namespace remote::controller
