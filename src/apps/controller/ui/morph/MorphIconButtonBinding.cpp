// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#include "MorphIconButtonBinding.h"
#include "src/apps/controller/FramelessWindow.h"
#include "src/apps/controller/ui/RemoteCTheme.h"

#include <QAbstractButton>
#include <QApplication>
#include <QEvent>
#include <QIcon>
#include <QPainter>
#include <QPixmap>

#include <algorithm>
#include <cmath>
#include <utility>

namespace remotec::ui::morph {
namespace {

constexpr auto kBindingName = "remoteCMorphIconBinding";

int animationLevel()
{
    return remote::controller::CurrentUiAnimationLevel();
}

QColor mixColor(const QColor& source, const QColor& target, double progress)
{
    progress = std::clamp(progress, 0.0, 1.0);
    return QColor::fromRgbF(
        source.redF() + (target.redF() - source.redF()) * progress,
        source.greenF() + (target.greenF() - source.greenF()) * progress,
        source.blueF() + (target.blueF() - source.blueF()) * progress,
        source.alphaF() + (target.alphaF() - source.alphaF()) * progress);
}

} // namespace

MorphIconButtonBinding* MorphIconButtonBinding::attach(
    QAbstractButton* button,
    const QString& sourceResource,
    const QString& targetResource,
    Interaction interaction,
    const QSize& iconSize,
    const QColor& sourceColor,
    const QColor& targetColor)
{
    if (!button) return nullptr;
    for (QObject* child : button->children()) {
        if (child->objectName() == QString::fromLatin1(kBindingName)) {
            if (auto* binding = dynamic_cast<MorphIconButtonBinding*>(child);
                binding && binding->sourceResource_ == sourceResource &&
                binding->targetResource_ == targetResource) {
                binding->iconSize_ = iconSize;
                binding->sourceColor_ = sourceColor;
                binding->targetColor_ = targetColor;
                binding->button_->setIconSize(iconSize);
                binding->render();
                return binding;
            }
        }
    }
    return new MorphIconButtonBinding(button, sourceResource, targetResource,
                                      interaction, iconSize,
                                      sourceColor, targetColor);
}

MorphIconButtonBinding::MorphIconButtonBinding(
    QAbstractButton* button,
    QString sourceResource,
    QString targetResource,
    Interaction interaction,
    QSize iconSize,
    QColor sourceColor,
    QColor targetColor)
    : QObject(button), button_(button),
      sourceResource_(std::move(sourceResource)),
      targetResource_(std::move(targetResource)), interaction_(interaction),
      iconSize_(iconSize), sourceColor_(sourceColor),
      targetColor_(targetColor)
{
    setObjectName(QString::fromLatin1(kBindingName));
    QString error;
    core_.configure(sourceResource_, targetResource_, 64, &error);
    button_->setIconSize(iconSize_);
    button_->installEventFilter(this);
    if (button_->isCheckable()) {
        connect(button_, &QAbstractButton::toggled, this,
                [this](bool) { render(); });
    }
    timer_.setTimerType(Qt::PreciseTimer);
    timer_.setInterval(16);
    connect(&timer_, &QTimer::timeout, this, [this] {
        const double seconds = std::clamp(
            elapsed_.restart() / 1000.0, 1.0 / 1000.0, 0.05);
        const bool settled = spring_.step(seconds);
        progress_ = start_ + (end_ - start_) * spring_.value();
        render();
        if (settled) {
            progress_ = end_;
            timer_.stop();
            render();
        }
    });
    render();
    if (interaction_ != Interaction::State) {
        setTarget(false, false);
    }
}

void MorphIconButtonBinding::setTarget(bool target, bool animated)
{
    if (initialized_ && target_ == target) {
        render();
        return;
    }
    target_ = target;
    if (!initialized_) {
        initialized_ = true;
        animated = false;
    }
    startTransition(target ? 1.0 : 0.0, animated);
}

void MorphIconButtonBinding::pulse(int holdMilliseconds)
{
    setTarget(true, true);
    QTimer::singleShot(holdMilliseconds, this, [this] {
        setTarget(false, true);
    });
}

bool MorphIconButtonBinding::eventFilter(QObject* watched, QEvent* event)
{
    if (watched == button_ && interaction_ == Interaction::Hover &&
        button_->isEnabled()) {
        if (event->type() == QEvent::Enter) {
            setTarget(true, true);
        }
        else if (event->type() == QEvent::Leave) {
            setTarget(false, true);
        }
    }
    if (watched == button_ &&
        (event->type() == QEvent::EnabledChange ||
         event->type() == QEvent::PaletteChange)) {
        render();
    }
    return QObject::eventFilter(watched, event);
}

void MorphIconButtonBinding::startTransition(double targetProgress,
                                             bool animated)
{
    if (!core_.isValid()) {
        progress_ = targetProgress;
        render();
        return;
    }
    start_ = progress_;
    end_ = targetProgress;
    const int level = animationLevel();
    if (!animated || level <= 0 || std::abs(start_ - end_) < 1e-5) {
        progress_ = end_;
        timer_.stop();
        render();
        return;
    }
    spring_.configure(level == 1 ? 170.0 : 420.0,
                      level == 1 ? 26.0 : 30.0);
    spring_.start();
    elapsed_.restart();
    timer_.start();
}

QColor MorphIconButtonBinding::currentColor() const
{
    if (!button_->isEnabled()) {
        const bool dark = remote::controller::ui::RemoteCTheme::IsDark(
            remote::controller::ui::RemoteCTheme::LoadPreference());
        return QColor(dark ? QStringLiteral("#AEBBD0")
                           : QStringLiteral("#526079"));
    }
    if (button_->isCheckable() && button_->isChecked()) {
        const QColor checked(
            button_->property("remoteCMorphCheckedColor").toString());
        return checked.isValid() ? checked : targetColor_;
    }
    return mixColor(sourceColor_, targetColor_, progress_);
}

void MorphIconButtonBinding::render()
{
    if (!button_) return;
    if (!core_.isValid()) {
        const QSize logicalSize = iconSize_.isValid()
            ? iconSize_ : QSize(18, 18);
        constexpr qreal backingScale = 2.0;
        QPixmap pixmap(qRound(logicalSize.width() * backingScale),
                       qRound(logicalSize.height() * backingScale));
        pixmap.setDevicePixelRatio(backingScale);
        pixmap.fill(Qt::transparent);
        QPainter painter(&pixmap);
        painter.setRenderHint(QPainter::Antialiasing, true);
        QIcon(progress_ > 0.5 ? targetResource_ : sourceResource_)
            .paint(&painter, QRect(QPoint(), logicalSize));
        painter.setCompositionMode(QPainter::CompositionMode_SourceIn);
        painter.fillRect(QRect(QPoint(), logicalSize), currentColor());
        painter.end();
        button_->setIcon(QIcon(pixmap));
        return;
    }
    constexpr qreal backingScale = 2.0;
    QPixmap pixmap(qRound(iconSize_.width() * backingScale),
                   qRound(iconSize_.height() * backingScale));
    pixmap.setDevicePixelRatio(backingScale);
    pixmap.fill(Qt::transparent);
    QPainter painter(&pixmap);
    core_.paint(painter,
                QRectF(0.0, 0.0, iconSize_.width(), iconSize_.height()),
                currentColor(), progress_, 2.0);
    painter.end();
    button_->setIcon(QIcon(pixmap));
}

} // namespace remotec::ui::morph
