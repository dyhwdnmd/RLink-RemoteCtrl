// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#include "RemoteCToast.h"

#include <algorithm>

#include <QAbstractAnimation>
#include <QEasingCurve>
#include <QEvent>
#include <QFontMetrics>
#include <QGraphicsOpacityEffect>
#include <QHBoxLayout>
#include <QLabel>
#include <QMainWindow>
#include <QPropertyAnimation>
#include <QSettings>
#include <QTimer>

namespace remote::controller {
namespace {

QWidget* ToastHost(QWidget* parent)
{
    if (!parent) {
        return nullptr;
    }
    QWidget* window = parent->window();
    if (auto* mainWindow = dynamic_cast<QMainWindow*>(window)) {
        if (mainWindow->centralWidget()) {
            return mainWindow->centralWidget();
        }
    }
    return window;
}

int AnimationLevel()
{
    const QSettings settings;
    const int legacyLevel = settings.value(
        QStringLiteral("ui/animationsEnabled"), true).toBool() ? 2 : 0;
    return std::clamp(settings.value(
        QStringLiteral("ui/animationLevel"), legacyLevel).toInt(), 0, 2);
}

}  // namespace

void RemoteCToast::Show(QWidget* parent,
                        const QString& message,
                        Tone tone)
{
    QWidget* host = ToastHost(parent);
    if (!host || message.trimmed().isEmpty()) {
        return;
    }
    RemoteCToast* toast = nullptr;
    const auto children = host->findChildren<QFrame*>(
        QStringLiteral("remoteCToast"), Qt::FindDirectChildrenOnly);
    for (auto* child : children) {
        toast = dynamic_cast<RemoteCToast*>(child);
        if (toast) {
            break;
        }
    }
    if (!toast) {
        toast = new RemoteCToast(host);
    }
    toast->ShowMessage(message, tone, nullptr);
}

void RemoteCToast::ShowAbove(QWidget* anchor,
                             const QString& message,
                             Tone tone)
{
    QWidget* host = ToastHost(anchor);
    if (!host || !anchor || message.trimmed().isEmpty()) {
        return;
    }
    RemoteCToast* toast = nullptr;
    const auto children = host->findChildren<QFrame*>(
        QStringLiteral("remoteCToast"), Qt::FindDirectChildrenOnly);
    for (auto* child : children) {
        toast = dynamic_cast<RemoteCToast*>(child);
        if (toast) {
            break;
        }
    }
    if (!toast) {
        toast = new RemoteCToast(host);
    }
    toast->ShowMessage(message, tone, anchor);
}

void RemoteCToast::ShowAtGlobalCenter(
    QWidget* parent,
    const QPoint& globalCenter,
    const QString& message,
    Tone tone,
    bool prominent)
{
    QWidget* host = ToastHost(parent);
    if (!host || message.trimmed().isEmpty()) {
        return;
    }
    RemoteCToast* toast = nullptr;
    const auto children = host->findChildren<QFrame*>(
        QStringLiteral("remoteCToast"), Qt::FindDirectChildrenOnly);
    for (auto* child : children) {
        toast = dynamic_cast<RemoteCToast*>(child);
        if (toast) {
            break;
        }
    }
    if (!toast) {
        toast = new RemoteCToast(host);
    }
    toast->ShowMessage(message, tone, nullptr, &globalCenter, prominent);
}

RemoteCToast::RemoteCToast(QWidget* host)
    : QFrame(host), host_(host)
{
    setObjectName(QStringLiteral("remoteCToast"));
    setAttribute(Qt::WA_TransparentForMouseEvents);
    setAttribute(Qt::WA_StyledBackground, true);
    setAutoFillBackground(false);
    setMaximumWidth(380);

    auto* layout = new QHBoxLayout(this);
    layout->setContentsMargins(14, 11, 16, 11);
    layout->setSpacing(9);
    marker_ = new QLabel(this);
    marker_->setAlignment(Qt::AlignCenter);
    marker_->setFixedSize(20, 20);
    layout->addWidget(marker_, 0, Qt::AlignVCenter);
    message_ = new QLabel(this);
    message_->setMaximumWidth(320);
    layout->addWidget(message_, 1);

    opacity_ = new QGraphicsOpacityEffect(this);
    setGraphicsEffect(opacity_);
    animation_ = new QPropertyAnimation(opacity_, "opacity", this);
    animation_->setEasingCurve(QEasingCurve::OutCubic);
    timer_ = new QTimer(this);
    timer_->setSingleShot(true);
    connect(timer_, &QTimer::timeout, this, &RemoteCToast::FadeOut);
    host_->installEventFilter(this);
    hide();
}

void RemoteCToast::ShowMessage(const QString& message,
                               Tone tone,
                               QWidget* anchor,
                               const QPoint* globalCenter,
                               bool prominent)
{
    anchor_ = anchor;
    useGlobalCenter_ = globalCenter != nullptr;
    if (globalCenter) {
        globalCenter_ = *globalCenter;
    }
    const int maximumTextWidth = prominent ? 380 : 320;
    setMaximumWidth(prominent ? 460 : 380);
    if (auto* toastLayout = qobject_cast<QHBoxLayout*>(layout())) {
        toastLayout->setContentsMargins(
            prominent ? 20 : 14,
            prominent ? 15 : 11,
            prominent ? 22 : 16,
            prominent ? 15 : 11);
        toastLayout->setSpacing(prominent ? 12 : 9);
    }
    marker_->setFixedSize(prominent ? 26 : 20,
                          prominent ? 26 : 20);
    message_->setMaximumWidth(maximumTextWidth);
    QString markerText;
    QString accent;
    switch (tone) {
    case Tone::kSuccess:
        markerText = QStringLiteral("✓");
        accent = QStringLiteral("#30d158");
        break;
    case Tone::kInformation:
        markerText = QStringLiteral("i");
        accent = QStringLiteral("#0a84ff");
        break;
    case Tone::kError:
        markerText = QStringLiteral("!");
        accent = QStringLiteral("#ff453a");
        break;
    }
    setStyleSheet(QStringLiteral(
        "QFrame#remoteCToast{"
        "background:%3;border:1px solid %4;border-radius:%5px;}"
        "QLabel{color:#f7f9fc;font-size:%2px;}"
        "QLabel:first-child{color:%1;font-weight:800;}")
        .arg(accent)
        .arg(prominent ? 14 : 12)
        .arg(prominent ? QStringLiteral("rgba(24,31,43,246)")
                       : QStringLiteral("#202632"))
        .arg(prominent ? QStringLiteral("#4a566a")
                       : QStringLiteral("#343d4d"))
        .arg(prominent ? 18 : 12));
    marker_->setText(markerText);
    marker_->setStyleSheet(
        prominent
            ? QStringLiteral(
                  "color:%1;background:rgba(48,209,88,30);"
                  "border:1px solid rgba(48,209,88,105);"
                  "border-radius:13px;font-weight:800;")
                  .arg(accent)
            : QStringLiteral("color:%1;font-weight:800;").arg(accent));
    message_->setText(message);
    const int naturalTextWidth =
        QFontMetrics(message_->font()).horizontalAdvance(message) + 4;
    const bool needsWrapping = naturalTextWidth > maximumTextWidth;
    message_->setWordWrap(needsWrapping);
    message_->setMinimumWidth(
        needsWrapping
            ? (prominent ? 330 : 280)
            : std::min(maximumTextWidth, naturalTextWidth));
    message_->setMaximumWidth(maximumTextWidth);
    adjustSize();
    Reposition();
    raise();
    show();
    timer_->start(prominent ? 2800 : 2200);

    animation_->stop();
    const int animationLevel = AnimationLevel();
    if (animationLevel == 0) {
        opacity_->setOpacity(1.0);
        return;
    }
    opacity_->setOpacity(0.0);
    // A control may begin media work immediately after requesting a toast.
    // Do not make the acknowledgement wait for a fully-transparent fade-in:
    // paint a clearly visible first frame now, then ease to full opacity.
    constexpr qreal kImmediateOpacity = 0.84;
    opacity_->setOpacity(kImmediateOpacity);
    repaint();
    animation_->setDuration(animationLevel == 1 ? 90 : 120);
    animation_->setStartValue(kImmediateOpacity);
    animation_->setEndValue(1.0);
    animation_->start();
}

void RemoteCToast::FadeOut()
{
    animation_->stop();
    const int animationLevel = AnimationLevel();
    if (animationLevel == 0) {
        hide();
        return;
    }
    animation_->setDuration(animationLevel == 1 ? 125 : 170);
    animation_->setStartValue(opacity_->opacity());
    animation_->setEndValue(0.0);
    connect(animation_, &QPropertyAnimation::finished, this,
            [this] {
                if (!timer_->isActive() && opacity_->opacity() <= 0.01) {
                    hide();
                }
            }, Qt::SingleShotConnection);
    animation_->start();
}

void RemoteCToast::Reposition()
{
    if (!host_) {
        return;
    }
    const int margin = 20;
    if (useGlobalCenter_) {
        const QPoint center = host_->mapFromGlobal(globalCenter_);
        const int maximumX =
            std::max(margin, host_->width() - width() - margin);
        const int maximumY =
            std::max(margin, host_->height() - height() - margin);
        move(std::clamp(center.x() - width() / 2, margin, maximumX),
             std::clamp(center.y() - height() / 2, margin, maximumY));
        return;
    }
    if (anchor_) {
        const QPoint anchorTopLeft = anchor_->mapTo(host_, QPoint(0, 0));
        const int centeredX = anchorTopLeft.x() +
            (anchor_->width() - width()) / 2;
        const int maximumX = std::max(margin, host_->width() - width() - margin);
        const int x = std::clamp(centeredX, margin, maximumX);
        int y = anchorTopLeft.y() - height() - 9;
        if (y < margin) {
            y = anchorTopLeft.y() + anchor_->height() + 9;
        }
        move(x, y);
        return;
    }
    move(std::max(margin, host_->width() - width() - margin),
         std::max(margin, host_->height() - height() - margin));
}

bool RemoteCToast::eventFilter(QObject* watched, QEvent* event)
{
    if (watched == host_ && event->type() == QEvent::Resize) {
        Reposition();
    }
    return QFrame::eventFilter(watched, event);
}

}  // namespace remote::controller
