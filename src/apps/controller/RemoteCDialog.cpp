// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#include "RemoteCDialog.h"
#include "src/apps/controller/ui/morph/MorphIconButtonBinding.h"
#include "src/apps/controller/ui/RemoteCTheme.h"
#include "FramelessWindow.h"

#include <algorithm>
#include <cmath>

#include <QAbstractButton>
#include <QAbstractAnimation>
#include <QApplication>
#include <QColor>
#include <QCheckBox>
#include <QEasingCurve>
#include <QElapsedTimer>
#include <QEvent>
#include <QFrame>
#include <QGraphicsDropShadowEffect>
#include <QGuiApplication>
#include <QHBoxLayout>
#include <QIcon>
#include <QLabel>
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>
#include <QPropertyAnimation>
#include <QPushButton>
#include <QProgressBar>
#include <QScreen>
#include <QSettings>
#include <QShowEvent>
#include <QSizePolicy>
#include <QStyle>
#include <QTimer>
#include <QToolButton>
#include <QVBoxLayout>
#include <QWindow>

#ifdef Q_OS_WIN
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#endif

namespace remote::controller {
namespace {

constexpr auto kDialogStyle = R"(
QDialog#remoteCDialog {
    background: transparent;
}
QFrame#dialogCard {
    background: #ffffff;
    border: 1px solid #e5e5ea;
    border-radius: 16px;
}
QLabel#dialogTitle {
    color: #1d1d1f;
    font-size: 17px;
    font-weight: 700;
}
QLabel#dialogMessage {
    color: #5f6368;
    font-size: 13px;
}
QLabel#dialogBadge {
    border-radius: 18px;
    font-family: "Segoe UI";
    font-size: 18px;
    font-weight: 800;
}
QToolButton#dialogClose {
    background: transparent;
    border: none;
    border-radius: 7px;
    color: #8e8e93;
    font-family: "Segoe UI Symbol";
    font-size: 17px;
}
QToolButton#dialogClose:hover {
    background: #f2f2f7;
    color: #1d1d1f;
}
QPushButton#dialogCancel, QPushButton#dialogConfirm,
QPushButton#dialogDanger {
    border: none;
    border-radius: 9px;
    font-size: 13px;
    font-weight: 650;
    min-height: 38px;
    padding: 0 20px;
}
QPushButton#dialogCancel {
    background: #f2f2f7;
    color: #3a3a3c;
}
QPushButton#dialogCancel:hover {
    background: #e5e5ea;
}
QPushButton#dialogConfirm {
    background: #007aff;
    color: white;
}
QPushButton#dialogConfirm:hover {
    background: #006ee6;
}
QPushButton#dialogConfirm:pressed {
    background: #005dcc;
}
QPushButton#dialogDanger {
    background: #ff3b30;
    color: white;
}
QPushButton#dialogDanger:hover {
    background: #e8332a;
}
QPushButton#dialogDanger:pressed {
    background: #cf2d25;
}
QProgressBar#dialogProgress {
    background: #e9eef7;
    border: 1px solid #dfe6f1;
    border-radius: 5px;
    min-height: 10px;
    max-height: 10px;
}
QProgressBar#dialogProgress::chunk {
    background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
        stop:0 #28c989, stop:0.55 #20bff2, stop:1 #1677ff);
    border-radius: 4px;
}
QCheckBox#dialogOption {
    color: #3a3a3c;
    font-size: 13px;
    spacing: 9px;
}
QCheckBox#dialogOption::indicator {
    width: 18px;
    height: 18px;
}
)";

constexpr auto kDialogDarkStyle = R"(
QFrame#dialogCard {
    background: #151F2E;
    border-color: #34445B;
}
QLabel#dialogTitle { color: #F1F5FA; }
QLabel#dialogMessage { color: #A9B6C8; }
QToolButton#dialogClose { color: #AAB6C8; }
QToolButton#dialogClose:hover {
    background: #243247;
    color: #FFFFFF;
}
QPushButton#dialogCancel {
    background: #253246;
    color: #DCE5F2;
}
QPushButton#dialogCancel:hover { background: #304058; }
QProgressBar#dialogProgress {
    background: #101824;
    border-color: #2A394E;
}
QCheckBox#dialogOption { color: #DCE5F2; }
)";

QString BadgeText(RemoteCDialog::Tone tone)
{
    switch (tone) {
    case RemoteCDialog::Tone::kQuestion:
        return QStringLiteral("?");
    case RemoteCDialog::Tone::kWarning:
    case RemoteCDialog::Tone::kDanger:
        return QStringLiteral("!");
    case RemoteCDialog::Tone::kInformation:
        return QStringLiteral("i");
    }
    return QStringLiteral("i");
}

QString BadgeStyle(RemoteCDialog::Tone tone)
{
    switch (tone) {
    case RemoteCDialog::Tone::kQuestion:
        return QStringLiteral("background:#eaf3ff;color:#007aff;");
    case RemoteCDialog::Tone::kWarning:
        return QStringLiteral("background:#fff5df;color:#b66a00;");
    case RemoteCDialog::Tone::kDanger:
        return QStringLiteral("background:#fff0ef;color:#ff3b30;");
    case RemoteCDialog::Tone::kInformation:
        return QStringLiteral("background:#edf8f1;color:#248a3d;");
    }
    return {};
}

class DialogDragHeader final : public QWidget {
public:
    explicit DialogDragHeader(QDialog* dialog, QWidget* parent)
        : QWidget(parent), dialog_(dialog)
    {
        setCursor(Qt::ArrowCursor);
    }

protected:
    void mousePressEvent(QMouseEvent* event) override
    {
        if (event->button() == Qt::LeftButton &&
            dialog_ && dialog_->windowHandle()) {
            dialog_->windowHandle()->startSystemMove();
            event->accept();
            return;
        }
        QWidget::mousePressEvent(event);
    }

private:
    QDialog* dialog_ = nullptr;
};

class DialogActivityIndicator final : public QWidget {
public:
    explicit DialogActivityIndicator(QWidget* parent)
        : QWidget(parent)
    {
        setFixedHeight(18);
        setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
        setAttribute(Qt::WA_TransparentForMouseEvents);
        timer_.setInterval(16);
        timer_.setTimerType(Qt::PreciseTimer);
        connect(&timer_, &QTimer::timeout, this,
                qOverload<>(&DialogActivityIndicator::update));
    }

protected:
    void showEvent(QShowEvent* event) override
    {
        QWidget::showEvent(event);
        elapsed_.restart();
        timer_.start();
    }

    void hideEvent(QHideEvent* event) override
    {
        timer_.stop();
        QWidget::hideEvent(event);
    }

    void paintEvent(QPaintEvent*) override
    {
        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing, true);
        painter.setPen(Qt::NoPen);

        constexpr double kCycleMs = 1050.0;
        constexpr double kTwoPi = 6.283185307179586;
        const double time = static_cast<double>(elapsed_.elapsed()) /
                            kCycleMs;
        const double centerY = height() / 2.0;
        constexpr double kHorizontalPadding = 8.0;
        constexpr double kPreferredDotSpacing = 16.0;
        const double availableWidth =
            std::max(0.0, width() - kHorizontalPadding * 2.0);
        const int kDotCount = std::max(
            2, static_cast<int>(availableWidth / kPreferredDotSpacing) + 1);
        const double kDotSpacing = availableWidth / (kDotCount - 1);
        const double startX = kHorizontalPadding;
        for (int index = 0; index < kDotCount; ++index) {
            const double phase = kTwoPi * time - index * 0.38;
            const double wave = 0.5 + 0.5 * std::sin(phase);
            const double radius = 3.1 + wave * 1.35;
            QColor color(77, 107, 255);
            color.setAlpha(90 + static_cast<int>(wave * 165.0));
            painter.setBrush(color);
            painter.drawEllipse(
                QPointF(startX + index * kDotSpacing, centerY),
                radius, radius);
        }
    }

private:
    QTimer timer_;
    QElapsedTimer elapsed_;
};

// QProgressBar may delegate value transitions to the native Windows style.
// For a transfer dialog that makes the painted chunk visibly trail the byte
// counter when updates arrive quickly. Paint the current value directly so
// the bar and the text always describe the same snapshot.
class DialogProgressBar final : public QProgressBar {
public:
    explicit DialogProgressBar(QWidget* parent = nullptr)
        : QProgressBar(parent)
    {
        setAttribute(Qt::WA_OpaquePaintEvent, false);
    }

protected:
    void paintEvent(QPaintEvent*) override
    {
        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing, true);

        const QRectF track = QRectF(rect()).adjusted(0.5, 0.5, -0.5, -0.5);
        painter.setPen(QPen(QColor(223, 230, 241), 1.0));
        painter.setBrush(QColor(233, 238, 247));
        painter.drawRoundedRect(track, 5.0, 5.0);

        const int span = maximum() - minimum();
        const qreal progress = span > 0
            ? std::clamp(static_cast<qreal>(value() - minimum()) /
                             static_cast<qreal>(span),
                         0.0, 1.0)
            : 0.0;
        if (progress <= 0.0) return;

        QPainterPath trackClip;
        trackClip.addRoundedRect(track.adjusted(1.0, 1.0, -1.0, -1.0),
                                 4.0, 4.0);
        painter.save();
        painter.setClipPath(trackClip);
        const QRectF fill = track.adjusted(1.0, 1.0, -1.0, -1.0);
        const QRectF filled(fill.left(), fill.top(),
                            fill.width() * progress, fill.height());
        QLinearGradient gradient(fill.topLeft(), fill.topRight());
        gradient.setColorAt(0.0, QColor(40, 201, 137));
        gradient.setColorAt(0.55, QColor(32, 191, 242));
        gradient.setColorAt(1.0, QColor(22, 119, 255));
        painter.setPen(Qt::NoPen);
        painter.setBrush(gradient);
        painter.drawRect(filled);
        painter.restore();
    }
};

class DialogOptionCheckBox final : public QCheckBox {
public:
    explicit DialogOptionCheckBox(
        const QString& text, QWidget* parent = nullptr)
        : QCheckBox(text, parent)
    {
        setMinimumHeight(28);
        setCursor(Qt::PointingHandCursor);
    }

protected:
    void paintEvent(QPaintEvent*) override
    {
        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing, true);

        constexpr qreal kBoxSize = 20.0;
        const QRectF box(0.5, (height() - kBoxSize) / 2.0,
                         kBoxSize, kBoxSize);
        const bool active = isEnabled();
        const bool hovered = underMouse();
        const bool dark = ui::RemoteCTheme::IsDark(
            ui::RemoteCTheme::LoadPreference());
        if (isChecked()) {
            QLinearGradient fill(box.topLeft(), box.bottomRight());
            fill.setColorAt(0.0, QColor(48, 139, 255));
            fill.setColorAt(1.0, QColor(20, 105, 238));
            painter.setPen(QPen(QColor(18, 98, 224), 1.0));
            painter.setBrush(fill);
        } else {
            painter.setPen(QPen(
                hovered ? QColor(79, 139, 230)
                        : QColor(dark ? QStringLiteral("#66758A")
                                      : QStringLiteral("#B7C2D3")),
                hovered ? 1.5 : 1.0));
            painter.setBrush(hovered
                ? QColor(dark ? QStringLiteral("#243247")
                              : QStringLiteral("#F3F8FF"))
                : QColor(dark ? QStringLiteral("#1B283A")
                              : QStringLiteral("#FAFBFD")));
        }
        painter.drawRoundedRect(box, 5.0, 5.0);

        if (isChecked()) {
            QPainterPath check;
            check.moveTo(box.left() + 4.7, box.center().y() + 0.2);
            check.lineTo(box.left() + 8.4, box.bottom() - 5.0);
            check.lineTo(box.right() - 4.0, box.top() + 5.3);
            painter.setPen(QPen(QColor(255, 255, 255), 2.2,
                                Qt::SolidLine, Qt::RoundCap,
                                Qt::RoundJoin));
            painter.setBrush(Qt::NoBrush);
            painter.drawPath(check);
        }

        painter.setPen(active
            ? QColor(dark ? QStringLiteral("#DCE5F2")
                          : QStringLiteral("#3A3A3C"))
            : QColor(dark ? QStringLiteral("#7F8BA0")
                          : QStringLiteral("#8E8E93")));
        painter.setFont(font());
        painter.drawText(
            QRectF(31.0, 0.0, width() - 31.0, height()),
            Qt::AlignLeft | Qt::AlignVCenter, text());
    }
};

}  // namespace

bool RemoteCDialog::Confirm(QWidget* parent,
                            const QString& title,
                            const QString& message,
                            const QString& confirmText,
                            const QString& cancelText,
                            Tone tone,
                            bool requestAttention,
                            bool nonActivating,
                            std::uintptr_t nativeAnchorWindow,
                            bool centerOnAnchorScreen)
{
    RemoteCDialog dialog(parent, title, message, confirmText, cancelText,
                         tone, true, requestAttention);
    dialog.SetNativeAnchorWindow(nativeAnchorWindow);
    dialog.centerOnAnchorScreen_ = centerOnAnchorScreen;
    if (nonActivating) dialog.SetNonActivatingWindow(true);
    return dialog.exec() == QDialog::Accepted;
}

bool RemoteCDialog::ConfirmWithOption(
    QWidget* parent,
    const QString& title,
    const QString& message,
    const QString& confirmText,
    const QString& cancelText,
    const QString& optionText,
    bool optionInitiallyChecked,
    bool* optionChecked,
    Tone tone,
    bool requestAttention)
{
    RemoteCDialog dialog(parent, title, message, confirmText, cancelText,
                         tone, true, requestAttention);
    auto* option = new DialogOptionCheckBox(optionText, dialog.card_);
    option->setObjectName(QStringLiteral("dialogOption"));
    option->setChecked(optionInitiallyChecked);
    option->setCursor(Qt::PointingHandCursor);
    if (auto* layout = qobject_cast<QVBoxLayout*>(dialog.card_->layout())) {
        layout->insertWidget((std::max)(0, layout->count() - 1), option);
    }
    const bool accepted = dialog.exec() == QDialog::Accepted;
    if (optionChecked) {
        *optionChecked = accepted && option->isChecked();
    }
    return accepted;
}

void RemoteCDialog::Alert(QWidget* parent,
                          const QString& title,
                          const QString& message,
                          const QString& buttonText,
                          Tone tone,
                          bool requestAttention)
{
    RemoteCDialog dialog(parent, title, message, buttonText, {}, tone, false,
                         requestAttention);
    dialog.exec();
}

RemoteCDialog* RemoteCDialog::CreateStatus(
    QWidget* parent,
    const QString& title,
    const QString& message,
    const QString& buttonText,
    Tone tone)
{
    auto* dialog = new RemoteCDialog(
        parent, title, message, buttonText, {}, tone, false, false);
    dialog->statusDialog_ = true;
    if (dialog->activityIndicator_) {
        dialog->activityIndicator_->setVisible(tone != Tone::kDanger);
    }
    // Status text changes as recovery advances. Keep the outer window stable
    // so an extra wrapped line never makes a user-moved dialog jump in size.
    dialog->setFixedHeight(280);
    if (dialog->card_) {
        dialog->card_->setFixedHeight(244);
        if (auto* statusLayout = qobject_cast<QVBoxLayout*>(
                dialog->card_->layout())) {
            statusLayout->setSpacing(13);
        }
    }
    if (dialog->messageLabel_) {
        // Remote-to-local transfers include a one-line destination followed
        // by two progress lines. Keep a stable three-line viewport so no text
        // is clipped or painted into the progress bar while values refresh.
        dialog->messageLabel_->setFixedHeight(66);
        dialog->messageLabel_->setAlignment(Qt::AlignLeft | Qt::AlignTop);
    }
    dialog->setWindowModality(Qt::NonModal);
    dialog->setModal(false);
    return dialog;
}

void RemoteCDialog::SetContent(const QString& title,
                               const QString& message,
                               const QString& buttonText,
                               Tone tone)
{
    if (badgeLabel_) {
        const QString badgeText = BadgeText(tone);
        const QString badgeStyle = BadgeStyle(tone);
        if (badgeLabel_->text() != badgeText) {
            badgeLabel_->setText(badgeText);
        }
        if (badgeLabel_->styleSheet() != badgeStyle) {
            badgeLabel_->setStyleSheet(badgeStyle);
        }
    }
    if (titleLabel_ && titleLabel_->text() != title) {
        titleLabel_->setText(title);
    }
    if (messageLabel_ && messageLabel_->text() != message) {
        messageLabel_->setText(message);
    }
    if (activityIndicator_) {
        // isVisible() becomes false whenever the whole dialog is minimized
        // into the transfer button, even though the progress bar itself is
        // still enabled. Using it here briefly re-enabled the old activity
        // dots in the hidden dialog and baked them into the restore snapshot.
        const bool progressEnabled =
            progressBar_ && !progressBar_->isHidden();
        activityIndicator_->setVisible(
            statusDialog_ && tone != Tone::kDanger &&
            !progressEnabled);
    }
    if (confirmButton_) {
        if (confirmButton_->text() != buttonText) {
            confirmButton_->setText(buttonText);
        }
        const QString objectName = tone == Tone::kDanger
            ? QStringLiteral("dialogDanger")
            : QStringLiteral("dialogConfirm");
        if (confirmButton_->objectName() != objectName) {
            confirmButton_->setObjectName(objectName);
            confirmButton_->style()->unpolish(confirmButton_);
            confirmButton_->style()->polish(confirmButton_);
        }
    }
    if (!statusDialog_) {
        adjustSize();
    }
}

void RemoteCDialog::SetStatusActionHandler(std::function<void()> handler)
{
    statusActionHandler_ = std::move(handler);
}

void RemoteCDialog::SetProgress(double progress)
{
    if (!progressBar_) return;
    if (progress < 0.0) {
        progressBar_->hide();
        return;
    }
    progressBar_->setValue(qRound(std::clamp(progress, 0.0, 1.0) * 1000.0));
    progressBar_->show();
    if (activityIndicator_) activityIndicator_->hide();
}

void RemoteCDialog::SetNonActivatingWindow(bool enabled)
{
    nonActivatingWindow_ = enabled;
    setAttribute(Qt::WA_ShowWithoutActivating, enabled);
    setWindowFlag(Qt::WindowDoesNotAcceptFocus, enabled);
#ifdef Q_OS_WIN
    if (!enabled) return;
    ApplyNonActivatingNativeStyle();
#endif
}

void RemoteCDialog::SetNativeAnchorWindow(std::uintptr_t windowHandle)
{
    nativeAnchorWindow_ = windowHandle;
}

void RemoteCDialog::ApplyNonActivatingNativeStyle()
{
#ifdef Q_OS_WIN
    // Force creation now so the native owner/style are fixed before show().
    // Clearing only the native owner preserves QObject ownership/lifetime but
    // prevents a click or drag from raising the RemoteSessionWindow.
    const HWND window = reinterpret_cast<HWND>(winId());
    if (!window) return;
    if (windowHandle()) {
        windowHandle()->setTransientParent(nullptr);
    }
    const LONG_PTR extendedStyle = GetWindowLongPtrW(window, GWL_EXSTYLE);
    SetWindowLongPtrW(window, GWL_EXSTYLE,
        extendedStyle | WS_EX_NOACTIVATE | WS_EX_TOOLWINDOW);
    SetWindowLongPtrW(window, GWLP_HWNDPARENT, 0);
    SetWindowPos(window, nullptr, 0, 0, 0, 0,
        SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE |
        SWP_FRAMECHANGED);
#endif
}

RemoteCDialog::RemoteCDialog(QWidget* parent,
                             const QString& title,
                             const QString& message,
                             const QString& confirmText,
                             const QString& cancelText,
                             Tone tone,
                             bool showCancel,
                             bool requestAttention)
    : QDialog(parent ? parent->window() : nullptr),
      requestAttention_(requestAttention)
{
    setObjectName(QStringLiteral("remoteCDialog"));
    Qt::WindowFlags flags = Qt::Dialog | Qt::FramelessWindowHint;
    if (requestAttention_) {
        flags |= Qt::WindowStaysOnTopHint;
    }
    setWindowFlags(flags);
    setWindowModality(Qt::WindowModal);
    setAttribute(Qt::WA_TranslucentBackground);
    QString dialogStyle = QString::fromUtf8(kDialogStyle);
    if (ui::RemoteCTheme::IsDark(
            ui::RemoteCTheme::LoadPreference())) {
        dialogStyle += QString::fromUtf8(kDialogDarkStyle);
    }
    setStyleSheet(ScaleUiStyleSheet(dialogStyle));

    auto* overlayLayout = new QVBoxLayout(this);
    overlayLayout->setContentsMargins(18, 18, 18, 18);
    overlayLayout->setAlignment(Qt::AlignCenter);

    auto* card = new QFrame(this);
    card_ = card;
    card->setObjectName(QStringLiteral("dialogCard"));
    card->setFixedWidth(410);
    auto* shadow = new QGraphicsDropShadowEffect(card);
    shadow->setBlurRadius(34);
    shadow->setOffset(0, 10);
    shadow->setColor(QColor(0, 0, 0, 75));
    card->setGraphicsEffect(shadow);

    auto* cardLayout = new QVBoxLayout(card);
    cardLayout->setContentsMargins(22, 20, 22, 20);
    cardLayout->setSpacing(16);

    auto* headerWidget = new DialogDragHeader(this, card);
    auto* header = new QHBoxLayout(headerWidget);
    header->setContentsMargins(0, 0, 0, 0);
    header->setSpacing(12);
    badgeLabel_ = new QLabel(BadgeText(tone), card);
    badgeLabel_->setObjectName(QStringLiteral("dialogBadge"));
    badgeLabel_->setStyleSheet(BadgeStyle(tone));
    badgeLabel_->setAlignment(Qt::AlignCenter);
    badgeLabel_->setFixedSize(36, 36);
    badgeLabel_->setAttribute(Qt::WA_TransparentForMouseEvents);
    header->addWidget(badgeLabel_, 0, Qt::AlignTop);
    titleLabel_ = new QLabel(title, card);
    titleLabel_->setObjectName(QStringLiteral("dialogTitle"));
    titleLabel_->setWordWrap(true);
    titleLabel_->setAttribute(Qt::WA_TransparentForMouseEvents);
    header->addWidget(titleLabel_, 1, Qt::AlignVCenter);
    auto* closeButton = new QToolButton(card);
    closeButton->setObjectName(QStringLiteral("dialogClose"));
    closeButton->setIcon(
        QIcon(QStringLiteral(":/ui/icons/window/close.svg")));
    remotec::ui::morph::MorphIconButtonBinding::attach(
        closeButton,
        QStringLiteral(":/ui/icons/lucide/base/x.svg"),
        QStringLiteral(":/ui/icons/lucide/base/circle-x.svg"),
        remotec::ui::morph::MorphIconButtonBinding::Interaction::Hover,
        QSize(16, 16), QColor(QStringLiteral("#667085")),
        QColor(QStringLiteral("#D14343")));
    closeButton->setIconSize(QSize(16, 16));
    closeButton->setCursor(Qt::PointingHandCursor);
    closeButton->setFixedSize(30, 30);
    header->addWidget(closeButton, 0, Qt::AlignTop);
    cardLayout->addWidget(headerWidget);

    messageLabel_ = new QLabel(message, card);
    messageLabel_->setObjectName(QStringLiteral("dialogMessage"));
    messageLabel_->setWordWrap(true);
    messageLabel_->setTextInteractionFlags(Qt::TextSelectableByMouse);
    cardLayout->addWidget(messageLabel_);

    activityIndicator_ = new DialogActivityIndicator(card);
    activityIndicator_->hide();
    cardLayout->addWidget(activityIndicator_);

    progressBar_ = new DialogProgressBar(card);
    progressBar_->setObjectName(QStringLiteral("dialogProgress"));
    progressBar_->setRange(0, 1000);
    progressBar_->setValue(0);
    progressBar_->setTextVisible(false);
    progressBar_->setFixedHeight(10);
    progressBar_->hide();
    cardLayout->addWidget(progressBar_);

    auto* actions = new QHBoxLayout();
    actions->setSpacing(10);
    actions->addStretch(1);
    QPushButton* cancelButton = nullptr;
    if (showCancel) {
        cancelButton = new QPushButton(cancelText, card);
        cancelButton->setObjectName(QStringLiteral("dialogCancel"));
        cancelButton->setCursor(Qt::PointingHandCursor);
        actions->addWidget(cancelButton);
    }
    confirmButton_ = new QPushButton(confirmText, card);
    confirmButton_->setObjectName(
        tone == Tone::kDanger ? QStringLiteral("dialogDanger")
                              : QStringLiteral("dialogConfirm"));
    confirmButton_->setCursor(Qt::PointingHandCursor);
    actions->addWidget(confirmButton_);
    cardLayout->addLayout(actions);
    overlayLayout->addWidget(card, 0, Qt::AlignCenter);

    connect(closeButton, &QToolButton::clicked, this, &QDialog::reject);
    connect(confirmButton_, &QPushButton::clicked, this, [this] {
        if (statusDialog_ && statusActionHandler_) {
            statusActionHandler_();
            return;
        }
        accept();
    });
    if (cancelButton) {
        connect(cancelButton, &QPushButton::clicked, this, &QDialog::reject);
        cancelButton->setDefault(true);
        QTimer::singleShot(0, cancelButton,
                           [cancelButton] { cancelButton->setFocus(); });
    } else {
        confirmButton_->setDefault(true);
        QTimer::singleShot(0, confirmButton_, [this] {
            if (!nonActivatingWindow_) confirmButton_->setFocus();
        });
    }

    installEventFilter(this);
    card->installEventFilter(this);
    const auto dragSurfaces = card->findChildren<QWidget*>();
    for (auto* surface : dragSurfaces) {
        if (!dynamic_cast<QAbstractButton*>(surface)) {
            surface->installEventFilter(this);
        }
    }
}

bool RemoteCDialog::eventFilter(QObject* watched, QEvent* event)
{
    if (nonActivatingWindow_) {
        if (event->type() == QEvent::MouseButtonPress &&
            !dynamic_cast<QAbstractButton*>(watched)) {
            auto* mouseEvent = static_cast<QMouseEvent*>(event);
            if (mouseEvent->button() == Qt::LeftButton) {
                nonActivatingDragActive_ = true;
                nonActivatingDragStartGlobal_ =
                    mouseEvent->globalPosition().toPoint();
                nonActivatingDragStartTopLeft_ = frameGeometry().topLeft();
                mouseEvent->accept();
                return true;
            }
        }
        if (event->type() == QEvent::MouseMove &&
            nonActivatingDragActive_) {
            auto* mouseEvent = static_cast<QMouseEvent*>(event);
            if ((mouseEvent->buttons() & Qt::LeftButton) != 0) {
                move(nonActivatingDragStartTopLeft_ +
                    mouseEvent->globalPosition().toPoint() -
                    nonActivatingDragStartGlobal_);
                mouseEvent->accept();
                return true;
            }
            nonActivatingDragActive_ = false;
        }
        if (event->type() == QEvent::MouseButtonRelease &&
            nonActivatingDragActive_) {
            auto* mouseEvent = static_cast<QMouseEvent*>(event);
            if (mouseEvent->button() == Qt::LeftButton) {
                nonActivatingDragActive_ = false;
                mouseEvent->accept();
                return true;
            }
        }
    }
    if (event->type() == QEvent::MouseButtonPress &&
        !dynamic_cast<QAbstractButton*>(watched)) {
        auto* mouseEvent = static_cast<QMouseEvent*>(event);
        if (mouseEvent->button() == Qt::LeftButton && windowHandle()) {
            windowHandle()->startSystemMove();
            mouseEvent->accept();
            return true;
        }
    }
    return QDialog::eventFilter(watched, event);
}

void RemoteCDialog::showEvent(QShowEvent* event)
{
    QDialog::showEvent(event);
    if (nonActivatingWindow_) {
        // QPA may recreate the transient native owner during show(). Reapply
        // the no-activate/tool-window contract after the native show event.
        ApplyNonActivatingNativeStyle();
    }
    if (requestAttention_ && !nonActivatingWindow_) {
        QApplication::beep();
        QApplication::alert(this, 0);
        raise();
        activateWindow();
        if (windowHandle()) {
            windowHandle()->requestActivate();
        }
    } else if (requestAttention_) {
        QApplication::beep();
#ifdef Q_OS_WIN
        SetWindowPos(reinterpret_cast<HWND>(winId()), HWND_TOPMOST,
            0, 0, 0, 0,
            SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE | SWP_SHOWWINDOW);
#endif
    }
    if (shown_) {
        return;
    }
    shown_ = true;

    adjustSize();
    QRect parentGeometry = parentWidget()
        ? parentWidget()->window()->frameGeometry()
        : QRect();
#ifdef Q_OS_WIN
    const HWND nativeAnchor = reinterpret_cast<HWND>(nativeAnchorWindow_);
    if (nativeAnchor && IsWindow(nativeAnchor)) {
        RECT nativeRect{};
        if (GetWindowRect(nativeAnchor, &nativeRect)) {
            parentGeometry = QRect(
                nativeRect.left, nativeRect.top,
                nativeRect.right - nativeRect.left,
                nativeRect.bottom - nativeRect.top);
        }
    }
#endif
    QScreen* screen = !parentGeometry.isNull()
        ? QGuiApplication::screenAt(parentGeometry.center())
        : QGuiApplication::primaryScreen();
    if (screen) {
        const QRect available = screen->availableGeometry();
        const QPoint center = !parentGeometry.isNull() &&
                !centerOnAnchorScreen_
            ? parentGeometry.center()
            : available.center();
        int x = center.x() - width() / 2;
        int y = center.y() - height() / 2;
        x = std::clamp(x, available.left(),
                       std::max(available.left(), available.right() - width() + 1));
        y = std::clamp(y, available.top(),
                       std::max(available.top(), available.bottom() - height() + 1));
        move(x, y);
    }

    const QSettings settings;
    const int legacyAnimationLevel = settings.value(
        QStringLiteral("ui/animationsEnabled"), true).toBool() ? 2 : 0;
    const int animationLevel = std::clamp(
        settings.value(QStringLiteral("ui/animationLevel"),
                       legacyAnimationLevel).toInt(), 0, 2);
    if (animationLevel == 0) {
        setWindowOpacity(1.0);
        return;
    }
    setWindowOpacity(0.0);
    auto* animation = new QPropertyAnimation(this, "windowOpacity", this);
    animation->setDuration(animationLevel == 1 ? 105 : 150);
    animation->setStartValue(0.0);
    animation->setEndValue(1.0);
    animation->setEasingCurve(QEasingCurve::OutCubic);
    animation->start(QAbstractAnimation::DeleteWhenStopped);
}

}  // namespace remote::controller
