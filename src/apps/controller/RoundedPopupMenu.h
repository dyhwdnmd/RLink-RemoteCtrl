// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#pragma once

#include <QMenu>
#include <QPointer>

#include <functional>

class QEvent;
class QHideEvent;
class QMouseEvent;
class QShowEvent;

namespace remote::controller {

class RoundedPopupMenu final : public QMenu {
public:
    explicit RoundedPopupMenu(
        QWidget* parent = nullptr,
        qreal cornerRadius = 12.0);

    void SetToggleAnchor(
        QWidget* anchor,
        std::function<void()> anchorPressed = {});

protected:
    bool eventFilter(QObject* watched, QEvent* event) override;
    void hideEvent(QHideEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;
    void showEvent(QShowEvent* event) override;

private:
    bool HandleToggleAnchorPress(QMouseEvent* event);

    qreal cornerRadius_ = 12.0;
    QPointer<QWidget> toggleAnchor_;
    std::function<void()> toggleAnchorPressed_;
    bool applicationFilterInstalled_ = false;
};

}  // namespace remote::controller
