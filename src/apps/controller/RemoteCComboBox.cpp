// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#include "RemoteCComboBox.h"

#include <QAbstractItemView>
#include <QApplication>
#include <QEasingCurve>
#include <QEvent>
#include <QFrame>
#include <QFontMetrics>
#include <QHBoxLayout>
#include <QListWidget>
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>
#include <QScreen>
#include <QStyledItemDelegate>
#include <QStyleOptionComboBox>
#include <QStyleOptionViewItem>
#include <QStylePainter>
#include <QVariantAnimation>
#include <QWheelEvent>

#include <algorithm>

#include "FramelessWindow.h"
#include "src/apps/controller/ui/RemoteCTheme.h"

namespace remote::controller {
namespace {

class RemoteCComboItemDelegate final : public QStyledItemDelegate {
public:
    using QStyledItemDelegate::QStyledItemDelegate;

    QSize sizeHint(const QStyleOptionViewItem& option,
                   const QModelIndex& index) const override
    {
        QSize size = QStyledItemDelegate::sizeHint(option, index);
        size.setHeight(40);
        return size;
    }

    void paint(QPainter* painter, const QStyleOptionViewItem& option,
               const QModelIndex& index) const override
    {
        painter->save();
        painter->setRenderHint(QPainter::Antialiasing);
        const QRectF itemRect = QRectF(option.rect).adjusted(5, 2, -5, -2);
        const bool selected = option.state & QStyle::State_Selected;
        const bool hovered = option.state & QStyle::State_MouseOver;
        const bool dark = option.widget &&
            option.widget->property("darkTheme").toBool();
        if (selected || hovered) {
            painter->setPen(Qt::NoPen);
            painter->setBrush(
                dark
                    ? (selected ? QColor(QStringLiteral("#263A61"))
                                : QColor(QStringLiteral("#202D40")))
                    : (selected ? QColor(QStringLiteral("#e9f1ff"))
                                : QColor(QStringLiteral("#f3f6fb"))));
            painter->drawRoundedRect(itemRect, 8.0, 8.0);
        }

        const QColor textColor = dark
            ? (selected ? QColor(QStringLiteral("#9BB4FF"))
                        : QColor(QStringLiteral("#E4EBF5")))
            : (selected ? QColor(QStringLiteral("#1769e8"))
                        : QColor(QStringLiteral("#263248")));
        painter->setPen(textColor);
        const QRect textRect = option.rect.adjusted(16, 0, -38, 0);
        const QString text = index.data(Qt::DisplayRole).toString();
        painter->drawText(
            textRect, Qt::AlignLeft | Qt::AlignVCenter,
            option.fontMetrics.elidedText(text, Qt::ElideRight,
                                          textRect.width()));

        if (selected) {
            painter->setPen(QPen(
                QColor(dark ? QStringLiteral("#9BB4FF")
                            : QStringLiteral("#1769e8")), 1.8,
                                 Qt::SolidLine, Qt::RoundCap,
                                 Qt::RoundJoin));
            const qreal centerX = option.rect.right() - 20.0;
            const qreal centerY = option.rect.center().y();
            QPainterPath check;
            check.moveTo(centerX - 5.0, centerY);
            check.lineTo(centerX - 1.5, centerY + 3.5);
            check.lineTo(centerX + 5.5, centerY - 4.0);
            painter->drawPath(check);
        }
        painter->restore();
    }
};

}  // namespace

RemoteCComboBox::RemoteCComboBox(QWidget* parent)
    : QComboBox(parent), arrowAnimation_(new QVariantAnimation(this))
{
    setCursor(Qt::PointingHandCursor);
    setMaxVisibleItems(9);
    // Device names are filled asynchronously and can be extremely long.
    // Do not let newly inserted item text change this widget's size hint and
    // consequently widen the whole settings page beyond its viewport.
    setSizeAdjustPolicy(QComboBox::AdjustToMinimumContentsLengthWithIcon);
    setMinimumContentsLength(1);
    setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Fixed);
    // Geometry is deliberately theme-independent.  Letting QSS padding and
    // min-height participate in the size hint made the light selector render
    // as a short pill while the dark selector stretched its parent row.
    setFixedHeight(50);
    setStyleSheet(QStringLiteral(R"(
QComboBox {
    background: #F8F7F3;
    border: 1px solid #D4CDC1;
    border-radius: 12px;
    color: #1f2a3d;
    padding: 0 38px 0 13px;
    selection-background-color: transparent;
    outline: none;
}
QComboBox:hover {
    background: #F0ECE5;
    border-color: #BEB5A7;
}
QComboBox:focus, QComboBox:on {
    background: #FFFEFB;
    border-color: #7180FF;
}
QComboBox:disabled {
    background: #ECE8E1;
    border-color: #DDE0E4;
    color: #667085;
}
QComboBox::drop-down {
    border: none;
    width: 36px;
}
QComboBox::down-arrow {
    image: none;
}
QComboBox QAbstractItemView {
    background: #FFFEFB;
    border: 1px solid #D9D2C7;
    border-radius: 12px;
    padding: 6px;
    color: #263248;
    outline: none;
    selection-background-color: transparent;
}
QComboBox QScrollBar:vertical {
    background: transparent;
    border: none;
    width: 8px;
    margin: 7px 2px 7px 0;
}
QComboBox QScrollBar::handle:vertical {
    background: #c5cbd6;
    border-radius: 3px;
    min-height: 28px;
}
QComboBox QScrollBar::handle:vertical:hover {
    background: #aeb7c6;
}
QComboBox QScrollBar::add-line:vertical,
QComboBox QScrollBar::sub-line:vertical,
QComboBox QScrollBar::add-page:vertical,
QComboBox QScrollBar::sub-page:vertical {
    background: transparent;
    border: none;
    height: 0;
}
)"));
    lightComboStyleSheet_ = styleSheet();
    RefreshThemeStyle();
    arrowAnimation_->setEasingCurve(QEasingCurve::OutCubic);
    connect(arrowAnimation_, &QVariantAnimation::valueChanged,
            this, [this](const QVariant& value) {
                arrowRotation_ = value.toReal();
                update();
            });
}

void RemoteCComboBox::RefreshThemeStyle()
{
    darkTheme_ = ui::RemoteCTheme::IsDark(
        ui::RemoteCTheme::LoadPreference());
    setProperty("darkTheme", darkTheme_);
    if (darkTheme_) {
        setStyleSheet(QStringLiteral(R"(
QComboBox {
    background: #101927;
    border: 1px solid #354861;
    border-radius: 12px;
    color: #E8EEF7;
    padding: 0 38px 0 13px;
    selection-background-color: transparent;
    outline: none;
}
QComboBox:hover { background: #172438; border-color: #49617E; }
QComboBox:focus, QComboBox:on {
    background: #192638;
    border-color: #8EA5FF;
}
QComboBox:disabled {
    background: #111A27;
    border-color: #2A374A;
    color: #7F8DA3;
}
QComboBox::drop-down { border: none; width: 36px; }
QComboBox::down-arrow { image: none; }
)"));
    }
    else if (!lightComboStyleSheet_.isEmpty()) {
        setStyleSheet(lightComboStyleSheet_);
    }
    if (popupList_) {
        popupList_->setProperty("darkTheme", darkTheme_);
        if (darkTheme_) {
            popupList_->setStyleSheet(QStringLiteral(R"(
QListWidget#remoteCComboPopupList {
    background: #141E2D;
    border: 1px solid #354861;
    border-radius: 12px;
    padding: 6px;
    color: #E6EDF7;
    outline: none;
}
QListWidget#remoteCComboPopupList QScrollBar:vertical {
    background: transparent; border: none; width: 8px;
    margin: 7px 2px 7px 0;
}
QListWidget#remoteCComboPopupList QScrollBar::handle:vertical {
    background: #4A586D; border-radius: 3px; min-height: 28px;
}
QListWidget#remoteCComboPopupList QScrollBar::add-line:vertical,
QListWidget#remoteCComboPopupList QScrollBar::sub-line:vertical,
QListWidget#remoteCComboPopupList QScrollBar::add-page:vertical,
QListWidget#remoteCComboPopupList QScrollBar::sub-page:vertical {
    background: transparent; border: none; height: 0;
}
)"));
        }
        else if (!lightPopupStyleSheet_.isEmpty()) {
            popupList_->setStyleSheet(lightPopupStyleSheet_);
        }
        popupList_->viewport()->update();
    }
    update();
}

void RemoteCComboBox::SetWheelSelectionEnabled(bool enabled)
{
    wheelSelectionEnabled_ = enabled;
}

void RemoteCComboBox::wheelEvent(QWheelEvent* event)
{
    if (!wheelSelectionEnabled_ && (!popup_ || !popup_->isVisible())) {
        event->ignore();
        return;
    }
    QComboBox::wheelEvent(event);
}

void RemoteCComboBox::paintEvent(QPaintEvent*)
{
    QStylePainter stylePainter(this);
    QStyleOptionComboBox option;
    initStyleOption(&option);
    option.subControls = QStyle::SC_ComboBoxFrame |
                         QStyle::SC_ComboBoxEditField;
    stylePainter.drawComplexControl(QStyle::CC_ComboBox, option);
    stylePainter.drawControl(QStyle::CE_ComboBoxLabel, option);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    const QColor color = darkTheme_
        ? (isEnabled() ? QColor(QStringLiteral("#AAB6C8"))
                       : QColor(QStringLiteral("#667489")))
        : (isEnabled() ? QColor(QStringLiteral("#536176"))
                       : QColor(QStringLiteral("#667085")));
    painter.setPen(QPen(color, 1.6, Qt::SolidLine,
                        Qt::RoundCap, Qt::RoundJoin));
    const qreal centerX = width() - 17.0;
    const qreal centerY = height() / 2.0;
    painter.translate(centerX, centerY);
    painter.rotate(arrowRotation_);
    painter.drawLine(QPointF(-3.5, -1.5), QPointF(0.0, 2.0));
    painter.drawLine(QPointF(0.0, 2.0), QPointF(3.5, -1.5));
}

void RemoteCComboBox::showPopup()
{
    if (popup_ && popup_->isVisible()) {
        hidePopup();
        return;
    }
    if (!isEnabled() || count() == 0) {
        return;
    }
    EnsurePopup();
    popupList_->clear();
    int popupWidth = width();
    const QFontMetrics metrics(font());
    for (int index = 0; index < count(); ++index) {
        popupWidth = std::max(
            popupWidth, metrics.horizontalAdvance(itemText(index)) + 66);
        auto* item = new QListWidgetItem(itemText(index), popupList_);
        item->setData(
            Qt::ToolTipRole, itemData(index, Qt::ToolTipRole));
    }
    popupList_->setCurrentRow(currentIndex());
    popupWidth = std::min(popupWidth, 460);
    const int visibleRows = std::min(count(), maxVisibleItems());
    const int popupHeight = visibleRows * 40 + 14;
    QPoint popupPosition = mapToGlobal(QPoint(0, height() + 5));
    QScreen* screen = QGuiApplication::screenAt(popupPosition);
    if (!screen) {
        screen = QApplication::primaryScreen();
    }
    if (screen) {
        const QRect available = screen->availableGeometry();
        if (popupPosition.y() + popupHeight > available.bottom()) {
            popupPosition = mapToGlobal(QPoint(0, -popupHeight - 5));
        }
        popupPosition.setX(std::clamp(
            popupPosition.x(), available.left(),
            std::max(available.left(), available.right() - popupWidth + 1)));
    }
    popup_->setGeometry(
        QRect(popupPosition, QSize(popupWidth, popupHeight)));
    SetPopupOpen(true);
    popup_->show();
    popup_->raise();
    popupList_->setFocus(Qt::PopupFocusReason);
}

void RemoteCComboBox::hidePopup()
{
    if (popup_) {
        popup_->hide();
    }
    SetPopupOpen(false);
}

bool RemoteCComboBox::eventFilter(QObject* watched, QEvent* event)
{
    if (watched == popup_ && event) {
        if (event->type() == QEvent::MouseButtonPress) {
            const auto* mouseEvent =
                static_cast<const QMouseEvent*>(event);
            if (mouseEvent->button() == Qt::LeftButton &&
                rect().contains(mapFromGlobal(
                    mouseEvent->globalPosition().toPoint()))) {
                // Match QComboBoxPrivateContainer: the popup receives an
                // outside press first.  Disable Qt's replay before hiding it,
                // otherwise the same press immediately opens the combo again.
                popup_->setAttribute(Qt::WA_NoMouseReplay);
                hidePopup();
                event->accept();
                return true;
            }
        } else if (event->type() == QEvent::Hide ||
                   event->type() == QEvent::Close) {
            SetPopupOpen(false);
        }
    }
    return QComboBox::eventFilter(watched, event);
}

void RemoteCComboBox::EnsurePopup()
{
    if (popup_) {
        return;
    }
    popup_ = new QFrame(
        this, Qt::Popup | Qt::FramelessWindowHint |
                  Qt::NoDropShadowWindowHint);
    popup_->setObjectName(QStringLiteral("remoteCComboPopup"));
    popup_->setAttribute(Qt::WA_TranslucentBackground, true);
    popup_->installEventFilter(this);
    auto* layout = new QHBoxLayout(popup_);
    layout->setContentsMargins(0, 0, 0, 0);
    popupList_ = new QListWidget(popup_);
    popupList_->setObjectName(QStringLiteral("remoteCComboPopupList"));
    popupList_->setFrameShape(QFrame::NoFrame);
    popupList_->setMouseTracking(true);
    popupList_->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    popupList_->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    popupList_->setItemDelegate(new RemoteCComboItemDelegate(popupList_));
    popupList_->setStyleSheet(QStringLiteral(R"(
QListWidget#remoteCComboPopupList {
    background: #FFFEFB;
    border: 1px solid #D9D2C7;
    border-radius: 12px;
    padding: 6px;
    outline: none;
}
QListWidget#remoteCComboPopupList QScrollBar:vertical {
    background: transparent;
    border: none;
    width: 8px;
    margin: 7px 2px 7px 0;
}
QListWidget#remoteCComboPopupList QScrollBar::handle:vertical {
    background: #c5cbd6;
    border-radius: 3px;
    min-height: 28px;
}
QListWidget#remoteCComboPopupList QScrollBar::handle:vertical:hover {
    background: #aeb7c6;
}
QListWidget#remoteCComboPopupList QScrollBar::add-line:vertical,
QListWidget#remoteCComboPopupList QScrollBar::sub-line:vertical,
QListWidget#remoteCComboPopupList QScrollBar::add-page:vertical,
QListWidget#remoteCComboPopupList QScrollBar::sub-page:vertical {
    background: transparent;
    border: none;
    height: 0;
}
)"));
    lightPopupStyleSheet_ = popupList_->styleSheet();
    RefreshThemeStyle();
    layout->addWidget(popupList_);
    const auto activateItem = [this](QListWidgetItem* item) {
        if (!item || !popupList_) {
            return;
        }
        const int index = popupList_->row(item);
        if (index < 0 || index >= count()) {
            return;
        }
        setCurrentIndex(index);
        emit activated(index);
        hidePopup();
    };
    connect(popupList_, &QListWidget::itemClicked,
            this, activateItem);
    connect(popupList_, &QListWidget::itemActivated,
            this, activateItem);
}

void RemoteCComboBox::SetPopupOpen(bool open)
{
    const qreal target = open ? 180.0 : 0.0;
    const int level = CurrentUiAnimationLevel();
    arrowAnimation_->stop();
    if (level <= 0) {
        arrowRotation_ = target;
        update();
        return;
    }
    arrowAnimation_->setDuration(level == 1 ? 80 : 135);
    arrowAnimation_->setStartValue(arrowRotation_);
    arrowAnimation_->setEndValue(target);
    arrowAnimation_->start();
}

}  // namespace remote::controller
