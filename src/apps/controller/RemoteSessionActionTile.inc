// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

    class ActionTile final : public QPushButton {
    public:
        ActionTile(const QString& icon,
            const QString& text,
            int fixedWidth,
            QWidget* parent = nullptr)
            : QPushButton(parent)
        {
            setObjectName(QStringLiteral("sessionActionTile"));
            setCursor(Qt::PointingHandCursor);
            setFocusPolicy(Qt::NoFocus);
            setAutoDefault(false);
            setDefault(false);
            setFixedSize(fixedWidth, kTileHeight);
            setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

            icon_ = icon;
            text_ = text;
            ConfigureMorphPair();
            transitionAnimation_ = new QVariantAnimation(this);
            connect(transitionAnimation_, &QVariantAnimation::valueChanged,
                    this, [this](const QVariant& value) {
                        iconScale_ = value.toReal();
                        update();
                    });
            morphTimer_ = new QTimer(this);
            morphTimer_->setTimerType(Qt::PreciseTimer);
            morphTimer_->setInterval(16);
            connect(morphTimer_, &QTimer::timeout, this, [this] {
                const double elapsedSeconds =
                    std::clamp(morphElapsed_.restart() / 1000.0,
                               1.0 / 1000.0, 0.05);
                const bool settled = morphSpring_.step(elapsedSeconds);
                morphProgress_ = morphStart_ +
                    (morphEnd_ - morphStart_) * morphSpring_.value();
                update();
                if (settled) {
                    morphProgress_ = morphEnd_;
                    morphTimer_->stop();
                    update();
                }
            });
            setProperty("tone", QStringLiteral("neutral"));
            setProperty("interactive", true);
        }

        void SetIcon(const QString& icon)
        {
            if (icon_ != icon) {
                icon_ = icon;
                const bool wantsTarget =
                    icon.contains(QStringLiteral("fullscreen-exit"));
                if (morphIcon_.isValid()) {
                    StartMorphTransition(wantsTarget);
                    return;
                }
                StartIconTransition();
            }
        }

        void SetText(const QString& text)
        {
            if (text_ != text) {
                text_ = text;
                update();
            }
        }

        void SetTone(const QString& tone)
        {
            if (property("tone").toString() == tone) {
                return;
            }
            setProperty("tone", tone);
            Repolish(this);
        }

        void SetInteractive(bool interactive)
        {
            if (property("interactive").toBool() == interactive) {
                return;
            }
            setProperty("interactive", interactive);
            setCursor(interactive ? Qt::PointingHandCursor : Qt::ArrowCursor);
            setAttribute(Qt::WA_TransparentForMouseEvents, !interactive);
            Repolish(this);
        }

        void SetSlashVisible(bool visible)
        {
            if (slashVisible_ != visible) {
                slashVisible_ = visible;
                if (morphIcon_.isValid()) {
                    StartMorphTransition(visible);
                    return;
                }
                StartIconTransition();
            }
        }

    protected:
        void enterEvent(QEnterEvent* event) override
        {
            QPushButton::enterEvent(event);
            if (hoverMorph_ && isEnabled() &&
                property("interactive").toBool()) {
                StartMorphTransition(true);
            }
        }

        void leaveEvent(QEvent* event) override
        {
            QPushButton::leaveEvent(event);
            if (hoverMorph_) {
                StartMorphTransition(false);
            }
        }

        void paintEvent(QPaintEvent* event) override
        {
            QPushButton::paintEvent(event);

            QPainter painter(this);
            painter.setRenderHint(QPainter::Antialiasing, true);
            painter.setRenderHint(QPainter::SmoothPixmapTransform, true);

            QColor textColor(QStringLiteral("#E3EAF5"));
            const QString tone = property("tone").toString();
            if (!isEnabled()) {
                textColor = QColor(QStringLiteral("#7F8BA0"));
            }
            else if (tone == QStringLiteral("positive")) {
                textColor = QColor(QStringLiteral("#73E0B1"));
            }
            else if (tone == QStringLiteral("danger")) {
                textColor = QColor(QStringLiteral("#FF8795"));
            }
            else if (tone == QStringLiteral("muted")) {
                textColor = QColor(QStringLiteral("#94A1B5"));
            }

            const QString iconResource = ResolveIconResource();
            const bool hasVectorIcon = !iconResource.isEmpty();
            const bool hasFallbackGlyph = !hasVectorIcon && !icon_.isEmpty();
            const bool hasIcon = morphIcon_.isValid() ||
                hasVectorIcon || hasFallbackGlyph;

            // Keep the semantic icon and label well inside the rounded tile.
            // This leaves an 11 px bottom safety margin even at scaled DPI.
            const QRect iconRect(6, 4, width() - 12, 18);
            const QRect textRect = hasIcon
                ? QRect(4, 25, width() - 8, 18)
                : QRect(6, 4, width() - 12, height() - 12);

            if (morphIcon_.isValid()) {
                painter.save();
                const QPointF center = iconRect.center();
                painter.translate(center);
                painter.scale(iconScale_, iconScale_);
                painter.translate(-center);
                morphIcon_.paint(painter, iconRect, textColor,
                                 morphProgress_, 2.0);
                painter.restore();
            }
            else if (hasVectorIcon) {
                painter.save();
                const QPointF center = iconRect.center();
                painter.translate(center);
                painter.scale(iconScale_, iconScale_);
                painter.translate(-center);
                const QIcon vectorIcon(iconResource);
                vectorIcon.paint(
                    &painter, iconRect, Qt::AlignCenter,
                    isEnabled() ? QIcon::Normal : QIcon::Disabled,
                    QIcon::Off);
                painter.restore();
            }
            else if (hasFallbackGlyph) {
                QFont iconFont(QStringLiteral("Segoe UI Symbol"));
                iconFont.setPixelSize(15);
                painter.setFont(iconFont);
                painter.setPen(textColor);
                painter.drawText(iconRect, Qt::AlignCenter, icon_);
            }

            QFont textFont = QApplication::font();
            textFont.setPixelSize(10);
            textFont.setWeight(QFont::DemiBold);
            painter.setFont(textFont);
            painter.setPen(textColor);
            painter.drawText(
                textRect,
                Qt::AlignHCenter | Qt::AlignVCenter | Qt::TextSingleLine,
                text_);
        }

    private:
        static constexpr int kTileHeight = 56;

        void ConfigureMorphPair()
        {
            QString source;
            QString target;
            if (text_.startsWith(QStringLiteral("声音"))) {
                source = QStringLiteral(
                    ":/ui/icons/lucide/base/volume-2.svg");
                target = QStringLiteral(
                    ":/ui/icons/lucide/base/volume-x.svg");
            }
            else if (text_.startsWith(QStringLiteral("麦克风"))) {
                source = QStringLiteral(
                    ":/ui/icons/lucide/base/mic.svg");
                target = QStringLiteral(
                    ":/ui/icons/lucide/base/mic-off.svg");
            }
            else if (text_ == QStringLiteral("全屏")) {
                source = QStringLiteral(
                    ":/ui/icons/lucide/base/maximize.svg");
                target = QStringLiteral(
                    ":/ui/icons/lucide/base/minimize.svg");
            }
            else if (text_ == QStringLiteral("文件")) {
                source = QStringLiteral(
                    ":/ui/icons/lucide/base/file.svg");
                target = QStringLiteral(
                    ":/ui/icons/lucide/base/folder-open.svg");
                hoverMorph_ = true;
            }
            else if (text_.contains(QStringLiteral("控制"))) {
                source = QStringLiteral(
                    ":/ui/icons/lucide/base/mouse-pointer-2.svg");
                target = QStringLiteral(
                    ":/ui/icons/lucide/base/screen-share.svg");
                hoverMorph_ = true;
            }
            else if (text_ == QStringLiteral("断开")) {
                source = QStringLiteral(
                    ":/ui/icons/lucide/base/log-out.svg");
                target = QStringLiteral(
                    ":/ui/icons/lucide/base/x.svg");
                hoverMorph_ = true;
            }
            if (!source.isEmpty()) {
                morphIcon_.configure(source, target);
            }
        }

        void StartMorphTransition(bool target)
        {
            transitionAnimation_->stop();
            iconScale_ = 1.0;
            morphStart_ = morphProgress_;
            morphEnd_ = target ? 1.0 : 0.0;
            if (CurrentUiAnimationLevel() <= 0) {
                morphProgress_ = morphEnd_;
                morphTimer_->stop();
                update();
                return;
            }
            morphSpring_.configure(
                CurrentUiAnimationLevel() == 1 ? 170.0 : 420.0,
                CurrentUiAnimationLevel() == 1 ? 26.0 : 30.0);
            morphSpring_.start();
            morphElapsed_.restart();
            morphTimer_->start();
        }

        void StartIconTransition()
        {
            transitionAnimation_->stop();
            if (CurrentUiAnimationLevel() <= 0) {
                iconScale_ = 1.0;
                update();
                return;
            }
            transitionAnimation_->setDuration(
                CurrentUiAnimationLevel() == 1 ? 105 : 165);
            transitionAnimation_->setStartValue(0.76);
            transitionAnimation_->setKeyValueAt(0.68, 1.08);
            transitionAnimation_->setEndValue(1.0);
            transitionAnimation_->setEasingCurve(QEasingCurve::OutCubic);
            transitionAnimation_->start();
        }

        QString ResolveIconResource() const
        {
            if (icon_.startsWith(QStringLiteral(":/"))) {
                return icon_;
            }
            if (icon_ == QStringLiteral("🎮")) {
                return QStringLiteral(":/ui/icons/actions/view.svg");
            }
            if (text_.contains(QStringLiteral("申请控制")) ||
                text_.contains(QStringLiteral("控制中"))) {
                return QStringLiteral(":/ui/icons/actions/control.svg");
            }
            if (text_.startsWith(QStringLiteral("声音"))) {
                return slashVisible_
                    ? QStringLiteral(":/ui/icons/actions/speaker-off.svg")
                    : QStringLiteral(":/ui/icons/actions/speaker.svg");
            }
            if (text_.startsWith(QStringLiteral("麦克风"))) {
                return slashVisible_
                    ? QStringLiteral(":/ui/icons/actions/microphone-off.svg")
                    : QStringLiteral(":/ui/icons/actions/microphone.svg");
            }
            if (text_ == QStringLiteral("文件")) {
                return QStringLiteral(":/ui/icons/actions/file.svg");
            }
            if (text_ == QStringLiteral("全屏")) {
                return QStringLiteral(":/ui/icons/actions/fullscreen.svg");
            }
            if (text_ == QStringLiteral("退出全屏")) {
                return QStringLiteral(":/ui/icons/actions/fullscreen-exit.svg");
            }
            if (text_ == QStringLiteral("断开")) {
                return QStringLiteral(":/ui/icons/actions/disconnect.svg");
            }
            if (text_.contains(QStringLiteral("观看"))) {
                return QStringLiteral(":/ui/icons/actions/view.svg");
            }
            return {};
        }

        static void Repolish(QWidget* widget)
        {
            if (!widget) {
                return;
            }
            widget->style()->unpolish(widget);
            widget->style()->polish(widget);
            widget->update();
        }

        QString icon_;
        QString text_;
        bool slashVisible_ = false;
        QVariantAnimation* transitionAnimation_ = nullptr;
        qreal iconScale_ = 1.0;
        remotec::ui::morph::MorphIconCore morphIcon_;
        remotec::ui::morph::Spring morphSpring_;
        QTimer* morphTimer_ = nullptr;
        QElapsedTimer morphElapsed_;
        double morphProgress_ = 0.0;
        double morphStart_ = 0.0;
        double morphEnd_ = 0.0;
        bool hoverMorph_ = false;
    };
