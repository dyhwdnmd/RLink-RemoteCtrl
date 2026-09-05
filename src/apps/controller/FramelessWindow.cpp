// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#include "FramelessWindow.h"

#include <algorithm>
#include <cmath>

#include <QEvent>
#include <QApplication>
#include <QAbstractScrollArea>
#include <QEasingCurve>
#include <QElapsedTimer>
#include <QEnterEvent>
#include <QHBoxLayout>
#include <QLabel>
#include <QMouseEvent>
#include <QPainter>
#include <QParallelAnimationGroup>
#include <QPixmap>
#include <QPropertyAnimation>
#include <QScrollBar>
#include <QWheelEvent>
#include <QRegularExpression>
#include <QSettings>
#include <QTimer>
#include <QToolButton>
#include <QWindow>

#include "src/apps/controller/ui/morph/MorphIconButtonBinding.h"
#include "src/apps/controller/ui/RemoteCTheme.h"

#ifdef Q_OS_WIN
#include <Windows.h>
#include <dwmapi.h>
#include <windowsx.h>
#pragma comment(lib, "dwmapi.lib")
#endif

namespace remote::controller {
namespace {

QToolButton* MakeCaptionButton(const QString& iconResource,
                               const QString& objectName,
                               QWidget* parent)
{
    auto* button = new QToolButton(parent);
    button->setObjectName(objectName);
    button->setIcon(QIcon(iconResource));
    button->setIconSize(QSize(16, 16));
    button->setToolButtonStyle(Qt::ToolButtonIconOnly);
    button->setAutoRaise(true);
    button->setCursor(Qt::ArrowCursor);
    button->setFixedSize(46, 40);
    return button;
}

#ifdef Q_OS_WIN
void SetNativeRoundedCorners(HWND window, bool enabled)
{
    if (!window) {
        return;
    }
    const DWM_WINDOW_CORNER_PREFERENCE preference =
        enabled ? DWMWCP_ROUND : DWMWCP_DONOTROUND;
    DwmSetWindowAttribute(
        window, DWMWA_WINDOW_CORNER_PREFERENCE,
        &preference, sizeof(preference));
}

void EnableNativeWindowTransitions(HWND window)
{
    if (!window) return;

    // DWM only provides the standard taskbar minimize/restore transition for
    // windows that retain the normal caption/resizable-frame capabilities.
    // WM_NCCALCSIZE below removes their visible non-client area, so these
    // styles do not bring back the standard Windows title bar or border.
    LONG_PTR style = GetWindowLongPtrW(window, GWL_STYLE);
    style |= WS_CAPTION | WS_THICKFRAME | WS_SYSMENU |
             WS_MINIMIZEBOX | WS_MAXIMIZEBOX;
    SetWindowLongPtrW(window, GWL_STYLE, style);

    const BOOL transitionsForcedDisabled = FALSE;
    (void)DwmSetWindowAttribute(
        window, DWMWA_TRANSITIONS_FORCEDISABLED,
        &transitionsForcedDisabled, sizeof(transitionsForcedDisabled));
    SetWindowPos(window, nullptr, 0, 0, 0, 0,
                 SWP_FRAMECHANGED | SWP_NOMOVE | SWP_NOSIZE |
                     SWP_NOACTIVATE | SWP_NOZORDER);
}

struct ResizeBorderMetrics {
    int horizontal = 8;
    int vertical = 8;
};

ResizeBorderMetrics ResizeBordersForWindow(HWND window)
{
    const UINT dpi = GetDpiForWindow(window);
    const int paddedBorder = GetSystemMetricsForDpi(
        SM_CXPADDEDBORDER, dpi);
    const int horizontalFrame = GetSystemMetricsForDpi(
        SM_CXSIZEFRAME, dpi);
    const int verticalFrame = GetSystemMetricsForDpi(
        SM_CYSIZEFRAME, dpi);
    const int minimumGrip = MulDiv(8, static_cast<int>(dpi), 96);
    return {
        std::max(horizontalFrame + paddedBorder, minimumGrip),
        std::max(verticalFrame + paddedBorder, minimumGrip)};
}

Qt::Edges ResizeEdgesForHitTest(LRESULT hitTest)
{
    switch (hitTest) {
    case HTLEFT:
        return Qt::LeftEdge;
    case HTRIGHT:
        return Qt::RightEdge;
    case HTTOP:
        return Qt::TopEdge;
    case HTBOTTOM:
        return Qt::BottomEdge;
    case HTTOPLEFT:
        return Qt::TopEdge | Qt::LeftEdge;
    case HTTOPRIGHT:
        return Qt::TopEdge | Qt::RightEdge;
    case HTBOTTOMLEFT:
        return Qt::BottomEdge | Qt::LeftEdge;
    case HTBOTTOMRIGHT:
        return Qt::BottomEdge | Qt::RightEdge;
    default:
        return {};
    }
}

HCURSOR ResizeCursorForHitTest(LRESULT hitTest)
{
    switch (hitTest) {
    case HTLEFT:
    case HTRIGHT:
        return LoadCursor(nullptr, IDC_SIZEWE);
    case HTTOP:
    case HTBOTTOM:
        return LoadCursor(nullptr, IDC_SIZENS);
    case HTTOPLEFT:
    case HTBOTTOMRIGHT:
        return LoadCursor(nullptr, IDC_SIZENWSE);
    case HTTOPRIGHT:
    case HTBOTTOMLEFT:
        return LoadCursor(nullptr, IDC_SIZENESW);
    default:
        return nullptr;
    }
}
#endif

class FramelessResizeOverlay final : public QWidget {
public:
    explicit FramelessResizeOverlay(FramelessMainWindow* owner)
        : QWidget(owner,
                  Qt::Tool | Qt::FramelessWindowHint |
                      Qt::NoDropShadowWindowHint |
                      Qt::WindowDoesNotAcceptFocus),
          owner_(owner)
    {
        setObjectName(QStringLiteral("framelessResizeOverlay"));
        setAttribute(Qt::WA_TranslucentBackground, true);
        setAttribute(Qt::WA_ShowWithoutActivating, true);
        setMouseTracking(true);
        if (owner_) {
            owner_->installEventFilter(this);
        }
        hide();
    }

protected:
    bool eventFilter(QObject* watched, QEvent* event) override
    {
        if (watched != owner_) {
            return QWidget::eventFilter(watched, event);
        }
        switch (event->type()) {
        case QEvent::Show:
        case QEvent::Move:
        case QEvent::Resize:
        case QEvent::WindowStateChange:
        case QEvent::WindowActivate:
            QTimer::singleShot(0, this, [this] { SyncToOwner(); });
            break;
        case QEvent::Hide:
        case QEvent::Close:
            hide();
            break;
        default:
            break;
        }
        return QWidget::eventFilter(watched, event);
    }

    void enterEvent(QEnterEvent* event) override
    {
        UpdateCursorForPosition(event->position().toPoint());
        QWidget::enterEvent(event);
    }

    void mouseMoveEvent(QMouseEvent* event) override
    {
        UpdateCursorForPosition(event->position().toPoint());
        event->accept();
    }

    void mousePressEvent(QMouseEvent* event) override
    {
        if (event->button() == Qt::LeftButton && owner_ &&
            owner_->windowHandle()) {
            const Qt::Edges edges = EdgesAt(event->position().toPoint());
            if (edges != Qt::Edges{} &&
                owner_->windowHandle()->startSystemResize(edges)) {
                event->accept();
                return;
            }
#ifdef Q_OS_WIN
            const WPARAM sizingEdge = NativeSizingEdge(edges);
            const HWND ownerWindow = reinterpret_cast<HWND>(owner_->winId());
            if (sizingEdge != 0 && ownerWindow) {
                ReleaseCapture();
                SendMessage(ownerWindow, WM_SYSCOMMAND,
                            SC_SIZE | sizingEdge, 0);
                event->accept();
                return;
            }
#endif
        }
        QWidget::mousePressEvent(event);
    }

    void paintEvent(QPaintEvent*) override
    {
        // A fully zero-alpha layered window is click-through on Windows.
        // Alpha 1 is visually imperceptible but keeps the masked resize ring
        // available for stable hit testing above native D3D child HWNDs.
        QPainter painter(this);
        painter.fillRect(rect(), QColor(0, 0, 0, 1));
    }

private:
    static constexpr int kGrip = 8;

#ifdef Q_OS_WIN
    static WPARAM NativeSizingEdge(Qt::Edges edges)
    {
        if (edges == (Qt::LeftEdge | Qt::TopEdge)) {
            return WMSZ_TOPLEFT;
        }
        if (edges == (Qt::RightEdge | Qt::TopEdge)) {
            return WMSZ_TOPRIGHT;
        }
        if (edges == (Qt::LeftEdge | Qt::BottomEdge)) {
            return WMSZ_BOTTOMLEFT;
        }
        if (edges == (Qt::RightEdge | Qt::BottomEdge)) {
            return WMSZ_BOTTOMRIGHT;
        }
        if (edges == Qt::LeftEdge) {
            return WMSZ_LEFT;
        }
        if (edges == Qt::RightEdge) {
            return WMSZ_RIGHT;
        }
        if (edges == Qt::TopEdge) {
            return WMSZ_TOP;
        }
        if (edges == Qt::BottomEdge) {
            return WMSZ_BOTTOM;
        }
        return 0;
    }
#endif

    Qt::Edges EdgesAt(const QPoint& position) const
    {
        Qt::Edges edges;
        if (position.x() < kGrip) {
            edges |= Qt::LeftEdge;
        }
        else if (position.x() >= width() - kGrip) {
            edges |= Qt::RightEdge;
        }
        if (position.y() < kGrip) {
            edges |= Qt::TopEdge;
        }
        else if (position.y() >= height() - kGrip) {
            edges |= Qt::BottomEdge;
        }
        return edges;
    }

    void UpdateCursorForPosition(const QPoint& position)
    {
        const Qt::Edges edges = EdgesAt(position);
        const bool horizontal = edges.testFlag(Qt::LeftEdge) ||
                                edges.testFlag(Qt::RightEdge);
        const bool vertical = edges.testFlag(Qt::TopEdge) ||
                              edges.testFlag(Qt::BottomEdge);
        if (horizontal && vertical) {
            const bool northwestSoutheast =
                (edges.testFlag(Qt::LeftEdge) &&
                 edges.testFlag(Qt::TopEdge)) ||
                (edges.testFlag(Qt::RightEdge) &&
                 edges.testFlag(Qt::BottomEdge));
            setCursor(northwestSoutheast
                          ? Qt::SizeFDiagCursor
                          : Qt::SizeBDiagCursor);
        }
        else if (horizontal) {
            setCursor(Qt::SizeHorCursor);
        }
        else if (vertical) {
            setCursor(Qt::SizeVerCursor);
        }
    }

    void SyncToOwner()
    {
        if (!owner_ || !owner_->isVisible() || owner_->isMinimized() ||
            owner_->isMaximized() || owner_->isFullScreen()) {
            hide();
            return;
        }
        setGeometry(QRect(owner_->mapToGlobal(QPoint(0, 0)),
                          owner_->size()));
        const QRegion inner(rect().adjusted(
            kGrip, kGrip, -kGrip, -kGrip));
        setMask(QRegion(rect()).subtracted(inner));
        if (!isVisible()) {
            show();
        }
        raise();
    }

    FramelessMainWindow* owner_ = nullptr;
};

}  // namespace

QIcon CreateRemoteCIcon()
{
    const QIcon icon(QStringLiteral(":/ui/branding/remotec-logo.png"));
    if (!icon.isNull()) {
        return icon;
    }
    return QIcon(QStringLiteral(":/ui/icons/navigation/local-device.svg"));
}

QString ScaleUiStyleSheet(const QString& styleSheet)
{
    const QSettings settings;
    const int baseSize = std::clamp(
        settings.value(QStringLiteral("ui/fontPixelSize"), 13).toInt(),
        12, 17);
    if (baseSize == 13) {
        return styleSheet;
    }
    QString scaled = styleSheet;
    const QRegularExpression expression(
        QStringLiteral("font-size\\s*:\\s*(\\d+)px"));
    const auto matches = expression.globalMatch(styleSheet);
    QList<QRegularExpressionMatch> found;
    auto iterator = matches;
    while (iterator.hasNext()) {
        found.push_back(iterator.next());
    }
    for (auto match = found.crbegin(); match != found.crend(); ++match) {
        const int original = match->captured(1).toInt();
        const int adjusted = std::max(
            9, qRound(static_cast<qreal>(original) * baseSize / 13.0));
        scaled.replace(
            match->capturedStart(0), match->capturedLength(0),
            QStringLiteral("font-size: %1px").arg(adjusted));
    }
    return scaled;
}

int CurrentUiAnimationLevel()
{
    const QSettings settings;
    const int legacyLevel = settings.value(
        QStringLiteral("ui/animationsEnabled"), true).toBool() ? 2 : 0;
    return std::clamp(settings.value(
        QStringLiteral("ui/animationLevel"), legacyLevel).toInt(), 0, 2);
}

namespace {

class SmoothWheelFilter final : public QObject {
public:
    explicit SmoothWheelFilter(QAbstractScrollArea* area)
        : QObject(area), area_(area)
    {
        timer_.setTimerType(Qt::PreciseTimer);
        timer_.setInterval(8);
        connect(&timer_, &QTimer::timeout, this,
                [this] { AdvanceInertia(); });
    }

protected:
    bool eventFilter(QObject* watched, QEvent* event) override
    {
        if (!area_ || watched != area_->viewport() ||
            event->type() != QEvent::Wheel) {
            return QObject::eventFilter(watched, event);
        }
        auto* wheel = static_cast<QWheelEvent*>(event);
        if (!wheel->pixelDelta().isNull()) {
            StopInertia();
            return false;
        }
        if (CurrentUiAnimationLevel() <= 0) {
            StopInertia();
            return false;
        }

        QScrollBar* bar = area_->verticalScrollBar();
        if (!bar || bar->maximum() <= bar->minimum()) {
            bar = area_->horizontalScrollBar();
        }
        if (!bar || bar->maximum() <= bar->minimum()) {
            return false;
        }
        const int angle = !wheel->angleDelta().y()
                              ? wheel->angleDelta().x()
                              : wheel->angleDelta().y();
        if (angle == 0) {
            return false;
        }

        if (activeBar_ != bar) {
            StopInertia();
            activeBar_ = bar;
            position_ = static_cast<qreal>(bar->value());
        }
        if (!timer_.isActive()) {
            position_ = static_cast<qreal>(bar->value());
            elapsed_.restart();
            timer_.start();
        }

        // One conventional wheel notch yields roughly 80 px of travel after
        // friction. Further notches add velocity instead of restarting an
        // easing curve, so a continuous wheel gesture remains continuous.
        const qreal impulse =
            -(static_cast<qreal>(angle) / 120.0) * 0.66;
        if (velocity_ * impulse < 0.0) {
            velocity_ *= 0.32;
        }
        velocity_ = std::clamp(velocity_ + impulse, -3.2, 3.2);
        wheel->accept();
        return true;
    }

private:
    void StopInertia()
    {
        timer_.stop();
        velocity_ = 0.0;
        activeBar_ = nullptr;
    }

    void AdvanceInertia()
    {
        if (!activeBar_ || CurrentUiAnimationLevel() <= 0) {
            StopInertia();
            return;
        }
        const qint64 elapsedMs = elapsed_.restart();
        const qreal deltaMs = std::clamp(
            static_cast<qreal>(elapsedMs), 1.0, 32.0);
        position_ += velocity_ * deltaMs;
        const qreal bounded = std::clamp(
            position_, static_cast<qreal>(activeBar_->minimum()),
            static_cast<qreal>(activeBar_->maximum()));
        const bool hitBoundary = !qFuzzyCompare(position_ + 1.0,
                                                bounded + 1.0);
        position_ = bounded;
        activeBar_->setValue(qRound(position_));

        // Approximately 200 ms of perceptible coast after the final notch.
        velocity_ *= std::pow(0.88, deltaMs / 16.0);
        if (hitBoundary || std::abs(velocity_) < 0.025) {
            StopInertia();
        }
    }

    QAbstractScrollArea* area_ = nullptr;
    QScrollBar* activeBar_ = nullptr;
    QTimer timer_;
    QElapsedTimer elapsed_;
    qreal position_ = 0.0;
    qreal velocity_ = 0.0;
};

}  // namespace

void EnableSmoothWheelScrolling(QAbstractScrollArea* scrollArea)
{
    if (!scrollArea ||
        scrollArea->property("remoteCSmoothWheelEnabled").toBool()) {
        return;
    }
    scrollArea->setProperty("remoteCSmoothWheelEnabled", true);
    auto* filter = new SmoothWheelFilter(scrollArea);
    scrollArea->viewport()->installEventFilter(filter);
}

FramelessMainWindow::FramelessMainWindow(QWidget* parent, bool embedded)
    : QMainWindow(parent)
{
    if (embedded) {
        setWindowFlags(Qt::Widget);
        return;
    }
    setWindowFlags(Qt::Window | Qt::FramelessWindowHint |
                   Qt::WindowSystemMenuHint |
                   Qt::WindowMinimizeButtonHint |
                   Qt::WindowMaximizeButtonHint);
    setWindowIcon(CreateRemoteCIcon());
#ifdef Q_OS_WIN
    const HWND window = reinterpret_cast<HWND>(winId());
    EnableNativeWindowTransitions(window);
    SetNativeRoundedCorners(window, true);
#endif
    resizeOverlay_ = new FramelessResizeOverlay(this);
}

void FramelessMainWindow::SetNativeRoundedCornersEnabled(bool enabled)
{
#ifdef Q_OS_WIN
    SetNativeRoundedCorners(
        reinterpret_cast<HWND>(winId()), enabled);
#else
    Q_UNUSED(enabled);
#endif
}

void FramelessMainWindow::changeEvent(QEvent* event)
{
    QMainWindow::changeEvent(event);
    if (event->type() != QEvent::WindowStateChange) {
        return;
    }
    const auto applyWindowShape = [this] {
        const bool flushToScreen = isMaximized() || isFullScreen();
        SetNativeRoundedCornersEnabled(!flushToScreen);
        if (flushToScreen) {
            clearMask();
        }
    };
    applyWindowShape();
    // Qt and Windows can finish the native state transition after the Qt
    // WindowStateChange notification. Reapply once the new HWND state and
    // geometry have settled so DWM cannot restore rounded corners.
    QTimer::singleShot(0, this, applyWindowShape);
}

void FramelessMainWindow::ApplyUiStyleSheet(const QString& styleSheet)
{
    baseStyleSheet_ = styleSheet;
    RefreshWindowStyle();
}

void FramelessMainWindow::RefreshWindowStyle()
{
    if (!baseStyleSheet_.isEmpty()) {
        setStyleSheet(ScaleUiStyleSheet(baseStyleSheet_));
    }
}

void FramelessMainWindow::RefreshAllWindowStyles()
{
    for (auto* widget : QApplication::topLevelWidgets()) {
        if (auto* window = dynamic_cast<FramelessMainWindow*>(widget)) {
            window->RefreshWindowStyle();
        }
        const auto descendants = widget->findChildren<QWidget*>();
        for (auto* descendant : descendants) {
            if (auto* window =
                    dynamic_cast<FramelessMainWindow*>(descendant)) {
                window->RefreshWindowStyle();
            }
        }
    }
}

void FramelessMainWindow::MinimizeWithSystemAnimation()
{
#ifdef Q_OS_WIN
    const HWND window = reinterpret_cast<HWND>(winId());
    if (window) {
        // Route the action through the native system command. Windows then
        // owns the taskbar transition, focus transfer and restore animation.
        PostMessage(window, WM_SYSCOMMAND, SC_MINIMIZE, 0);
        return;
    }
#endif
    showMinimized();
}

void FramelessMainWindow::AnimateWindowEntrance(
    const QPoint& fullAnimationOffset,
    const QRect& globalClipRect,
    int durationOverrideMs)
{
    const int level = CurrentUiAnimationLevel();
    if (auto* exitAnimation = findChild<QParallelAnimationGroup*>(
            QStringLiteral("remoteCWindowExitAnimation"))) {
        exitAnimation->stop();
        exitAnimation->deleteLater();
        setProperty("remoteCExitRunning", false);
        setWindowOpacity(1.0);
        clearMask();
    }
    if (level <= 0) {
        clearMask();
        setWindowOpacity(1.0);
        return;
    }
    if (property("remoteCEntranceRunning").toBool()) {
        return;
    }
    setProperty("remoteCEntranceRunning", true);
    const QPoint destination = pos();
    setWindowOpacity(0.0);
    if (level >= 2) {
        move(destination + fullAnimationOffset);
    }
    const auto applyClip = [this, globalClipRect] {
        if (globalClipRect.isNull()) {
            return;
        }
        const QRect windowRect = frameGeometry();
        const QRect visible = windowRect.intersected(globalClipRect);
        setMask(visible.isEmpty()
                    ? QRegion()
                    : QRegion(visible.translated(-windowRect.topLeft())));
    };
    applyClip();
    auto* group = new QParallelAnimationGroup(this);
    group->setObjectName(QStringLiteral("remoteCWindowEntranceAnimation"));
    auto* opacity = new QPropertyAnimation(this, "windowOpacity", group);
    opacity->setDuration(durationOverrideMs > 0
                             ? durationOverrideMs
                             : (level == 1 ? 105 : 160));
    opacity->setStartValue(0.0);
    opacity->setEndValue(1.0);
    opacity->setEasingCurve(QEasingCurve::OutCubic);
    group->addAnimation(opacity);
    if (level >= 2) {
        auto* position = new QPropertyAnimation(this, "pos", group);
        position->setDuration(durationOverrideMs > 0
                                  ? durationOverrideMs
                                  : 170);
        position->setStartValue(destination + fullAnimationOffset);
        position->setEndValue(destination);
        position->setEasingCurve(QEasingCurve::OutCubic);
        connect(position, &QPropertyAnimation::valueChanged, this,
                [applyClip](const QVariant&) { applyClip(); });
        group->addAnimation(position);
    }
    connect(group, &QParallelAnimationGroup::finished, this,
            [this, destination] {
                setWindowOpacity(1.0);
                move(destination);
                clearMask();
                setProperty("remoteCEntranceRunning", false);
            });
    group->start(QAbstractAnimation::DeleteWhenStopped);
}

void FramelessMainWindow::AnimateWindowExit(
    const QPoint& fullAnimationOffset,
    const QRect& globalClipRect,
    int durationOverrideMs)
{
    if (!isVisible() || property("remoteCExitRunning").toBool()) {
        return;
    }
    if (auto* entranceAnimation = findChild<QParallelAnimationGroup*>(
            QStringLiteral("remoteCWindowEntranceAnimation"))) {
        entranceAnimation->stop();
        entranceAnimation->deleteLater();
        setProperty("remoteCEntranceRunning", false);
        setWindowOpacity(1.0);
        clearMask();
    }
    const int level = CurrentUiAnimationLevel();
    if (level <= 0) {
        hide();
        clearMask();
        setWindowOpacity(1.0);
        return;
    }
    setProperty("remoteCExitRunning", true);
    const QPoint origin = pos();
    auto* group = new QParallelAnimationGroup(this);
    group->setObjectName(QStringLiteral("remoteCWindowExitAnimation"));
    const auto applyClip = [this, globalClipRect] {
        if (globalClipRect.isNull()) {
            return;
        }
        const QRect windowRect = frameGeometry();
        const QRect visible = windowRect.intersected(globalClipRect);
        setMask(visible.isEmpty()
                    ? QRegion()
                    : QRegion(visible.translated(-windowRect.topLeft())));
    };
    applyClip();
    if (fullAnimationOffset.isNull()) {
        auto* opacity = new QPropertyAnimation(
            this, "windowOpacity", group);
        opacity->setDuration(durationOverrideMs > 0
                                 ? durationOverrideMs
                                 : (level == 1 ? 85 : 120));
        opacity->setStartValue(windowOpacity());
        opacity->setEndValue(0.0);
        opacity->setEasingCurve(QEasingCurve::InCubic);
        group->addAnimation(opacity);
    } else {
        auto* position = new QPropertyAnimation(this, "pos", group);
        position->setDuration(durationOverrideMs > 0
                                  ? durationOverrideMs
                                  : (level == 1 ? 120 : 170));
        position->setStartValue(origin);
        position->setEndValue(origin + fullAnimationOffset);
        position->setEasingCurve(QEasingCurve::InCubic);
        connect(position, &QPropertyAnimation::valueChanged, this,
                [applyClip](const QVariant&) { applyClip(); });
        group->addAnimation(position);
    }
    connect(group, &QParallelAnimationGroup::finished, this,
            [this, origin] {
        hide();
        move(origin);
        clearMask();
        setWindowOpacity(1.0);
        setProperty("remoteCExitRunning", false);
    });
    group->start(QAbstractAnimation::DeleteWhenStopped);
}

bool FramelessMainWindow::nativeEvent(const QByteArray& eventType,
                                      void* message,
                                      qintptr* result)
{
#ifdef Q_OS_WIN
    Q_UNUSED(eventType);
    const auto* nativeMessage = static_cast<MSG*>(message);
    if (nativeMessage && nativeMessage->message == WM_NCCALCSIZE) {
        if (nativeMessage->wParam != FALSE &&
            IsZoomed(nativeMessage->hwnd)) {
            auto* parameters = reinterpret_cast<NCCALCSIZE_PARAMS*>(
                nativeMessage->lParam);
            MONITORINFO monitorInfo{};
            monitorInfo.cbSize = sizeof(monitorInfo);
            const HMONITOR monitor = MonitorFromWindow(
                nativeMessage->hwnd, MONITOR_DEFAULTTONEAREST);
            if (parameters && monitor &&
                GetMonitorInfoW(monitor, &monitorInfo)) {
                // Keep maximized content inside the monitor work area rather
                // than extending under the taskbar after removing the frame.
                parameters->rgrc[0] = monitorInfo.rcWork;
            }
        }
        *result = 0;
        return true;
    }
    if (nativeMessage && resizeEdges_ != Qt::Edges{} &&
        nativeMessage->message == WM_MOUSEMOVE) {
        POINT cursor{};
        if (GetCursorPos(&cursor)) {
            const int deltaX = cursor.x - resizeStartCursor_.x();
            const int deltaY = cursor.y - resizeStartCursor_.y();
            int left = resizeStartWindowRect_.left();
            int top = resizeStartWindowRect_.top();
            int right = left + resizeStartWindowRect_.width();
            int bottom = top + resizeStartWindowRect_.height();

            if (resizeEdges_.testFlag(Qt::LeftEdge)) {
                left += deltaX;
            }
            if (resizeEdges_.testFlag(Qt::RightEdge)) {
                right += deltaX;
            }
            if (resizeEdges_.testFlag(Qt::TopEdge)) {
                top += deltaY;
            }
            if (resizeEdges_.testFlag(Qt::BottomEdge)) {
                bottom += deltaY;
            }

            const UINT dpi = GetDpiForWindow(nativeMessage->hwnd);
            const int minimumPhysicalWidth =
                MulDiv(minimumWidth(), static_cast<int>(dpi), 96);
            const int minimumPhysicalHeight =
                MulDiv(minimumHeight(), static_cast<int>(dpi), 96);
            if (right - left < minimumPhysicalWidth) {
                if (resizeEdges_.testFlag(Qt::LeftEdge)) {
                    left = right - minimumPhysicalWidth;
                } else {
                    right = left + minimumPhysicalWidth;
                }
            }
            if (bottom - top < minimumPhysicalHeight) {
                if (resizeEdges_.testFlag(Qt::TopEdge)) {
                    top = bottom - minimumPhysicalHeight;
                } else {
                    bottom = top + minimumPhysicalHeight;
                }
            }

            const qreal devicePixelRatio =
                static_cast<qreal>(dpi) / 96.0;
            const QRect constrained = ConstrainResizeGeometry(
                QRect(left, top, right - left, bottom - top),
                resizeEdges_, devicePixelRatio);
            SetWindowPos(nativeMessage->hwnd, nullptr,
                         constrained.left(), constrained.top(),
                         constrained.width(), constrained.height(),
                         SWP_NOACTIVATE | SWP_NOZORDER);
        }
        *result = 0;
        return true;
    }
    if (nativeMessage && resizeEdges_ != Qt::Edges{} &&
        (nativeMessage->message == WM_LBUTTONUP ||
         nativeMessage->message == WM_NCLBUTTONUP ||
         nativeMessage->message == WM_CANCELMODE)) {
        resizeEdges_ = {};
        if (GetCapture() == nativeMessage->hwnd) {
            ReleaseCapture();
        }
        *result = 0;
        return true;
    }
    if (nativeMessage && resizeEdges_ != Qt::Edges{} &&
        nativeMessage->message == WM_CAPTURECHANGED) {
        resizeEdges_ = {};
    }
    if (nativeMessage && nativeMessage->message == WM_SETCURSOR &&
        !isMaximized() && !isFullScreen()) {
        const LRESULT hitTest = LOWORD(nativeMessage->lParam);
        if (HCURSOR cursor = ResizeCursorForHitTest(hitTest)) {
            SetCursor(cursor);
            *result = TRUE;
            return true;
        }
    }
    if (nativeMessage && nativeMessage->message == WM_NCLBUTTONDOWN &&
        nativeMessage->wParam != HTCLIENT && !isMaximized() &&
        !isFullScreen()) {
        const Qt::Edges edges =
            ResizeEdgesForHitTest(static_cast<LRESULT>(nativeMessage->wParam));
        if (edges != Qt::Edges{}) {
            if (windowHandle() && windowHandle()->startSystemResize(edges)) {
                *result = 0;
                return true;
            }
            POINT cursor{};
            RECT windowRect{};
            if (!GetCursorPos(&cursor) ||
                !GetWindowRect(nativeMessage->hwnd, &windowRect)) {
                return QMainWindow::nativeEvent(eventType, message, result);
            }
            resizeEdges_ = edges;
            resizeStartCursor_ = QPoint(cursor.x, cursor.y);
            resizeStartWindowRect_ =
                QRect(windowRect.left, windowRect.top,
                      windowRect.right - windowRect.left,
                      windowRect.bottom - windowRect.top);
            SetCapture(nativeMessage->hwnd);
            *result = 0;
            return true;
        }
    }
    if (nativeMessage && nativeMessage->message == WM_NCHITTEST &&
        !isMaximized() && !isFullScreen()) {
        RECT windowRect{};
        if (!GetWindowRect(nativeMessage->hwnd, &windowRect)) {
            return QMainWindow::nativeEvent(eventType, message, result);
        }
        const int cursorX = GET_X_LPARAM(nativeMessage->lParam);
        const int cursorY = GET_Y_LPARAM(nativeMessage->lParam);
        const ResizeBorderMetrics border =
            ResizeBordersForWindow(nativeMessage->hwnd);

        // WM_NCHITTEST and GetWindowRect both use physical screen pixels.
        // Keeping the calculation entirely in that coordinate space avoids
        // edge drift under Per-Monitor V2 and VM/RDP fractional scaling.
        const bool onLeft =
            cursorX >= windowRect.left &&
            cursorX < windowRect.left + border.horizontal;
        const bool onRight =
            cursorX < windowRect.right &&
            cursorX >= windowRect.right - border.horizontal;
        const bool onTop =
            cursorY >= windowRect.top &&
            cursorY < windowRect.top + border.vertical;
        const bool onBottom =
            cursorY < windowRect.bottom &&
            cursorY >= windowRect.bottom - border.vertical;

        if (onTop && onLeft) {
            *result = HTTOPLEFT;
            return true;
        }
        if (onTop && onRight) {
            *result = HTTOPRIGHT;
            return true;
        }
        if (onBottom && onLeft) {
            *result = HTBOTTOMLEFT;
            return true;
        }
        if (onBottom && onRight) {
            *result = HTBOTTOMRIGHT;
            return true;
        }
        if (onLeft) {
            *result = HTLEFT;
            return true;
        }
        if (onRight) {
            *result = HTRIGHT;
            return true;
        }
        if (onTop) {
            *result = HTTOP;
            return true;
        }
        if (onBottom) {
            *result = HTBOTTOM;
            return true;
        }
    }
#else
    Q_UNUSED(eventType);
    Q_UNUSED(message);
    Q_UNUSED(result);
#endif
    return QMainWindow::nativeEvent(eventType, message, result);
}

QRect FramelessMainWindow::ConstrainResizeGeometry(
    const QRect& proposedGeometry,
    Qt::Edges resizeEdges,
    qreal devicePixelRatio) const
{
    Q_UNUSED(resizeEdges);
    Q_UNUSED(devicePixelRatio);
    return proposedGeometry;
}

CustomTitleBar::CustomTitleBar(FramelessMainWindow* window,
                               QString title,
                               QWidget* parent)
    : QWidget(parent), window_(window)
{
    setObjectName(QStringLiteral("customTitleBar"));
    setFixedHeight(40);

    auto* layout = new QHBoxLayout(this);
    layout->setContentsMargins(12, 0, 0, 0);
    layout->setSpacing(8);

    auto* icon = new QLabel(this);
    icon->setObjectName(QStringLiteral("titleBarIcon"));
    icon->setPixmap(CreateRemoteCIcon().pixmap(22, 22));
    icon->setFixedSize(24, 24);
    icon->setAttribute(Qt::WA_TransparentForMouseEvents);
    layout->addWidget(icon);

    auto* applicationName = new QLabel(QStringLiteral("RLink"), this);
    applicationName->setObjectName(QStringLiteral("titleBarAppName"));
    applicationName->setAttribute(Qt::WA_TransparentForMouseEvents);
    layout->addWidget(applicationName);

    auto* divider = new QLabel(QStringLiteral("·"), this);
    divider->setObjectName(QStringLiteral("titleBarDivider"));
    divider->setAttribute(Qt::WA_TransparentForMouseEvents);
    layout->addWidget(divider);

    titleLabel_ = new QLabel(std::move(title), this);
    titleLabel_->setObjectName(QStringLiteral("titleBarTitle"));
    titleLabel_->setAttribute(Qt::WA_TransparentForMouseEvents);
    layout->addWidget(titleLabel_);
    layout->addStretch(1);

    minimizeButton_ = MakeCaptionButton(
        QStringLiteral(":/ui/icons/window/minimize.svg"),
        QStringLiteral("titleBarButton"), this);
    minimizeButton_->setToolTip(QStringLiteral("最小化"));
    maximizeButton_ = MakeCaptionButton(
        QStringLiteral(":/ui/icons/window/maximize.svg"),
        QStringLiteral("titleBarButton"), this);
    maximizeButton_->setToolTip(QStringLiteral("最大化"));
    remotec::ui::morph::MorphIconButtonBinding::attach(
        maximizeButton_,
        QStringLiteral(":/ui/icons/lucide/base/maximize.svg"),
        QStringLiteral(":/ui/icons/lucide/base/minimize.svg"),
        remotec::ui::morph::MorphIconButtonBinding::Interaction::State,
        QSize(16, 16), QColor(QStringLiteral("#B9C3D2")),
        QColor(QStringLiteral("#FFFFFF")));
    closeButton_ = MakeCaptionButton(
        QStringLiteral(":/ui/icons/window/close.svg"),
        QStringLiteral("titleBarCloseButton"), this);
    closeButton_->setToolTip(QStringLiteral("关闭"));
    remotec::ui::morph::MorphIconButtonBinding::attach(
        closeButton_,
        QStringLiteral(":/ui/icons/lucide/base/x.svg"),
        QStringLiteral(":/ui/icons/lucide/base/circle-x.svg"),
        remotec::ui::morph::MorphIconButtonBinding::Interaction::Hover,
        QSize(16, 16), QColor(QStringLiteral("#B9C3D2")),
        QColor(QStringLiteral("#FFFFFF")));
    layout->addWidget(minimizeButton_);
    layout->addWidget(maximizeButton_);
    layout->addWidget(closeButton_);

    connect(minimizeButton_, &QToolButton::clicked, this, [this] {
        if (minimizeAction_) {
            minimizeAction_();
        } else {
            window_->MinimizeWithSystemAnimation();
        }
    });
    connect(maximizeButton_, &QToolButton::clicked, this,
            &CustomTitleBar::ToggleMaximized);
    connect(closeButton_, &QToolButton::clicked, window_, &QWidget::close);
    window_->installEventFilter(this);
    RefreshThemeStyle(ui::RemoteCTheme::IsDark(
        ui::RemoteCTheme::LoadPreference()));
    UpdateMaximizeButton();
}

void CustomTitleBar::SetMinimizeAction(std::function<void()> action)
{
    minimizeAction_ = std::move(action);
}

void CustomTitleBar::SetEmbeddedMode(bool embedded)
{
    embedded_ = embedded;
    minimizeButton_->setVisible(!embedded);
    maximizeButton_->setVisible(!embedded);
}

void CustomTitleBar::SetTitle(const QString& title)
{
    titleLabel_->setText(title);
}

void CustomTitleBar::RefreshThemeStyle(bool dark)
{
    const QColor neutral(dark ? QStringLiteral("#AEBBD0")
                              : QStringLiteral("#637087"));
    const QColor active(dark ? QStringLiteral("#F1F5FB")
                             : QStringLiteral("#172033"));
    const QColor danger(dark ? QStringLiteral("#FF98A5")
                             : QStringLiteral("#FFFFFF"));
    ui::RemoteCTheme::SetIcon(
        minimizeButton_, QStringLiteral(":/ui/icons/window/minimize.svg"),
        ui::ThemeIconTone::kNeutral);
    remotec::ui::morph::MorphIconButtonBinding::attach(
        maximizeButton_,
        QStringLiteral(":/ui/icons/lucide/base/maximize.svg"),
        QStringLiteral(":/ui/icons/lucide/base/minimize.svg"),
        remotec::ui::morph::MorphIconButtonBinding::Interaction::State,
        QSize(16, 16), neutral, active);
    remotec::ui::morph::MorphIconButtonBinding::attach(
        closeButton_,
        QStringLiteral(":/ui/icons/lucide/base/x.svg"),
        QStringLiteral(":/ui/icons/lucide/base/circle-x.svg"),
        remotec::ui::morph::MorphIconButtonBinding::Interaction::Hover,
        QSize(16, 16), neutral, danger);
    UpdateMaximizeButton();
}

bool CustomTitleBar::eventFilter(QObject* watched, QEvent* event)
{
    if (watched == window_ && event->type() == QEvent::WindowStateChange) {
        UpdateMaximizeButton();
    }
    return QWidget::eventFilter(watched, event);
}

void CustomTitleBar::mouseDoubleClickEvent(QMouseEvent* event)
{
    if (embedded_) {
        QWidget::mouseDoubleClickEvent(event);
        return;
    }
    if (event->button() == Qt::LeftButton) {
        ToggleMaximized();
        event->accept();
        return;
    }
    QWidget::mouseDoubleClickEvent(event);
}

void CustomTitleBar::mousePressEvent(QMouseEvent* event)
{
    if (embedded_) {
        QWidget::mousePressEvent(event);
        return;
    }
    if (event->button() == Qt::LeftButton && window_->windowHandle()) {
        window_->windowHandle()->startSystemMove();
        event->accept();
        return;
    }
    QWidget::mousePressEvent(event);
}

void CustomTitleBar::ToggleMaximized()
{
#ifdef Q_OS_WIN
    if (const HWND nativeWindow =
            reinterpret_cast<HWND>(window_->winId())) {
        // Use the native system command so DWM owns the maximize/restore
        // transition. Direct QWidget::showMaximized() changes state without
        // the standard Windows zoom animation on this frameless HWND.
        PostMessageW(nativeWindow, WM_SYSCOMMAND,
                     window_->isMaximized() ? SC_RESTORE : SC_MAXIMIZE, 0);
        return;
    }
#endif
    if (window_->isMaximized()) {
        window_->showNormal();
    } else {
        window_->showMaximized();
    }
}

void CustomTitleBar::UpdateMaximizeButton()
{
    const bool maximized = window_->isMaximized();
    maximizeButton_->setIcon(QIcon(
        maximized ? QStringLiteral(":/ui/icons/window/restore.svg")
                  : QStringLiteral(":/ui/icons/window/maximize.svg")));
    maximizeButton_->setToolTip(maximized ? QStringLiteral("还原")
                                          : QStringLiteral("最大化"));
    if (auto* morph =
            remotec::ui::morph::MorphIconButtonBinding::attach(
                maximizeButton_,
                QStringLiteral(":/ui/icons/lucide/base/maximize.svg"),
                QStringLiteral(":/ui/icons/lucide/base/minimize.svg"),
                remotec::ui::morph::MorphIconButtonBinding::Interaction::State,
                QSize(16, 16),
                QColor(ui::RemoteCTheme::IsDark(
                           ui::RemoteCTheme::LoadPreference())
                           ? QStringLiteral("#AEBBD0")
                           : QStringLiteral("#637087")),
                QColor(ui::RemoteCTheme::IsDark(
                           ui::RemoteCTheme::LoadPreference())
                           ? QStringLiteral("#F1F5FB")
                           : QStringLiteral("#172033")))) {
        morph->setTarget(maximized);
    }
}

}  // namespace remote::controller
