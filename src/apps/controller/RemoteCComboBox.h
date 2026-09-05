// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#pragma once

#include <QComboBox>

class QVariantAnimation;
class QFrame;
class QListWidget;
class QWheelEvent;

namespace remote::controller {

class RemoteCComboBox final : public QComboBox {
public:
    explicit RemoteCComboBox(QWidget* parent = nullptr);
    void RefreshThemeStyle();
    void SetWheelSelectionEnabled(bool enabled);

protected:
    bool eventFilter(QObject* watched, QEvent* event) override;
    void paintEvent(QPaintEvent* event) override;
    void wheelEvent(QWheelEvent* event) override;
    void showPopup() override;
    void hidePopup() override;

private:
    void EnsurePopup();
    void SetPopupOpen(bool open);

    QVariantAnimation* arrowAnimation_ = nullptr;
    QFrame* popup_ = nullptr;
    QListWidget* popupList_ = nullptr;
    QString lightComboStyleSheet_;
    QString lightPopupStyleSheet_;
    bool darkTheme_ = false;
    bool wheelSelectionEnabled_ = true;
    qreal arrowRotation_ = 0.0;
};

}  // namespace remote::controller
