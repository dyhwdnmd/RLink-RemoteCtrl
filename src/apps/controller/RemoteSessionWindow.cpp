// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#include "RemoteSessionWindow.h"

#include <Windows.h>
#include <d3d10_1.h>
#include <d3d11.h>
#include <d3dcompiler.h>
#include <dxgi1_2.h>
#include <wrl/client.h>

#include <QAction>
#include <QActionGroup>
#include <QCloseEvent>
#include <QClipboard>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QEasingCurve>
#include <QEvent>
#include <QEnterEvent>
#include <QFrame>
#include <QFocusEvent>
#include <QFont>
#include <QGridLayout>
#include <QGraphicsOpacityEffect>
#include <QHBoxLayout>
#include <QLabel>
#include <QMenu>
#include <QImage>
#include <QElapsedTimer>
#include <QKeyEvent>
#include <QLinearGradient>
#include <QMouseEvent>
#include <QMimeData>
#include <QMoveEvent>
#include <QPainter>
#include <QPainterPath>
#include <QPushButton>
#include <QPropertyAnimation>
#include <QRegion>
#include <QResizeEvent>
#include <QScreen>
#include <QSettings>
#include <QShortcut>
#include <QStyle>
#include <QTimer>
#include <QToolButton>
#include <QUrl>
#include <QVariantAnimation>
#include <QVBoxLayout>
#include <QWheelEvent>
#include <QWindow>
#include <QWidget>
#include <QWidgetAction>
#include <QApplication>

#include <algorithm>
#include <array>
#include <atomic>
#include <chrono>
#include <cmath>
#include <condition_variable>
#include <cstring>
#include <functional>
#include <iterator>
#include <limits>
#include <mutex>
#include <optional>
#include <thread>
#include <utility>

#include "api/video/video_frame.h"
#include "api/video/i420_buffer.h"
#include "api/video/video_sink_interface.h"
#include "libyuv/convert_argb.h"
#include "libyuv/convert_from.h"
#include "libyuv/scale.h"
#include "rtc_base/time_utils.h"
#include "src/apps/remote/InProcessSessionEngine.h"
#include "src/apps/controller/ui/morph/MorphIconCore.h"
#include "src/core/RemoteInputTelemetry.h"
#include "src/core/VideoPresentationTelemetry.h"
#include "src/platform/win/D3D11NativeFrameBuffer.h"
#include "RemoteCDialog.h"
#include "RemoteCToast.h"
#include "RemoteInputDispatcher.h"
#include "RoundedPopupMenu.h"
#include "src/core/RoomState.h"
#include "src/protocol/RemoteCursorProtocol.h"

namespace remote::controller {

    namespace {
        constexpr auto kRemoteScreenQualitySetting =
            "remoteSession/qualityTier";
        constexpr auto kDragPointerSampleRateSetting =
            "remoteSession/dragPointerSampleRateHz";
        constexpr std::array<std::uint32_t, 5>
            kSupportedDragPointerSampleRates = {
                60u, 80u, 120u, 170u, 240u };

        bool IsDirectRecoveryFailureCode(
            const std::string& errorCode)
        {
            return errorCode == "peer_reconnect_timeout" ||
                   errorCode == "ice_restart_exhausted";
        }
    }

#include "RemoteSessionActionTile.inc"

    class MorphIconToolButton final : public QToolButton {
    public:
        MorphIconToolButton(const QString& sourceResource,
                            const QString& targetResource,
                            QWidget* parent = nullptr)
            : QToolButton(parent), sourceResource_(sourceResource),
              targetResource_(targetResource)
        {
            setCursor(Qt::PointingHandCursor);
            setFocusPolicy(Qt::NoFocus);
            setToolButtonStyle(Qt::ToolButtonIconOnly);
            QString error;
            if (!morphIcon_.configure(sourceResource, targetResource, 64,
                                      &error)) {
                setIcon(QIcon(sourceResource));
                setIconSize(QSize(18, 18));
            }
            timer_.setTimerType(Qt::PreciseTimer);
            timer_.setInterval(16);
            connect(&timer_, &QTimer::timeout, this, [this] {
                const double elapsedSeconds =
                    std::clamp(elapsed_.restart() / 1000.0,
                               1.0 / 1000.0, 0.05);
                const bool settled = spring_.step(elapsedSeconds);
                progress_ = start_ + (end_ - start_) * spring_.value();
                update();
                if (settled) {
                    progress_ = end_;
                    timer_.stop();
                    update();
                }
            });
        }

        void SetTarget(bool target)
        {
            if (!morphIcon_.isValid()) {
                setIcon(QIcon(target ? targetResource_ : sourceResource_));
                return;
            }
            start_ = progress_;
            end_ = target ? 1.0 : 0.0;
            if (CurrentUiAnimationLevel() <= 0) {
                progress_ = end_;
                timer_.stop();
                update();
                return;
            }
            spring_.configure(
                CurrentUiAnimationLevel() == 1 ? 170.0 : 420.0,
                CurrentUiAnimationLevel() == 1 ? 26.0 : 30.0);
            spring_.start();
            elapsed_.restart();
            timer_.start();
        }

    protected:
        void paintEvent(QPaintEvent* event) override
        {
            QToolButton::paintEvent(event);
            if (!morphIcon_.isValid()) return;
            QPainter painter(this);
            QColor color(QStringLiteral("#D8D1C3"));
            if (!isEnabled()) {
                color = QColor(QStringLiteral("#738096"));
            }
            else if (progress_ > 0.5) {
                color = QColor(QStringLiteral("#FFC75A"));
            }
            const QRectF iconRect((width() - 18.0) / 2.0,
                                  (height() - 18.0) / 2.0,
                                  18.0, 18.0);
            morphIcon_.paint(painter, iconRect, color, progress_, 2.0);
            // The locked endpoint follows the high-contrast product icon:
            // yellow lock body with a white shackle. Fade it over the generic
            // morph near the endpoint so the transition remains continuous.
            const qreal lockedOverlay = std::clamp(
                (progress_ - 0.55) / 0.45, 0.0, 1.0);
            if (lockedOverlay > 0.0) {
                painter.save();
                painter.setRenderHint(QPainter::Antialiasing, true);
                painter.setOpacity(lockedOverlay);
                painter.translate(iconRect.left(), iconRect.top());
                painter.scale(iconRect.width() / 24.0,
                              iconRect.height() / 24.0);
                painter.setPen(Qt::NoPen);
                painter.setBrush(QColor(QStringLiteral("#FFC857")));
                painter.drawRoundedRect(QRectF(3.0, 10.0, 18.0, 12.0),
                                        2.0, 2.0);
                QPainterPath shackle;
                shackle.moveTo(7.0, 10.0);
                shackle.lineTo(7.0, 7.0);
                shackle.cubicTo(7.0, 4.24, 9.24, 2.0, 12.0, 2.0);
                shackle.cubicTo(14.76, 2.0, 17.0, 4.24, 17.0, 7.0);
                shackle.lineTo(17.0, 10.0);
                painter.setBrush(Qt::NoBrush);
                painter.setPen(QPen(QColor(QStringLiteral("#F4F7FB")),
                                    2.0, Qt::SolidLine, Qt::RoundCap,
                                    Qt::RoundJoin));
                painter.drawPath(shackle);
                painter.setPen(Qt::NoPen);
                painter.setBrush(QColor(QStringLiteral("#5B4310")));
                painter.drawEllipse(QPointF(12.0, 16.0), 1.25, 1.25);
                painter.restore();
            }
        }

    private:
        QString sourceResource_;
        QString targetResource_;
        remotec::ui::morph::MorphIconCore morphIcon_;
        remotec::ui::morph::Spring spring_;
        QTimer timer_;
        QElapsedTimer elapsed_;
        double progress_ = 0.0;
        double start_ = 0.0;
        double end_ = 0.0;
    };

    namespace {

        class LiquidTransferButton final : public QToolButton {
        public:
            explicit LiquidTransferButton(QWidget* parent = nullptr)
                : QToolButton(parent)
            {
                setCursor(Qt::PointingHandCursor);
                setToolTip(QStringLiteral(
                    "远程粘贴正在后台传输，点击查看"));
                setFixedSize(24, 28);
                waveTimer_.setInterval(34);
                connect(&waveTimer_, &QTimer::timeout, this, [this] {
                    if (!isVisible()) return;
                    wavePhase_ += 0.22;
                    update();
                });
                waveTimer_.start();
            }

            void SetProgress(double progress)
            {
                progress = std::clamp(progress, 0.0, 1.0);
                if (std::abs(progress - targetProgress_) < 0.0005) return;
                targetProgress_ = progress;
                if (progressAnimation_) {
                    progressAnimation_->stop();
                    progressAnimation_->deleteLater();
                }
                auto* animation = new QVariantAnimation(this);
                progressAnimation_ = animation;
                animation->setDuration(260);
                animation->setStartValue(progress_);
                animation->setEndValue(targetProgress_);
                animation->setEasingCurve(QEasingCurve::OutCubic);
                connect(animation, &QVariantAnimation::valueChanged, this,
                        [this](const QVariant& value) {
                            progress_ = value.toDouble();
                            update();
                        });
                connect(animation, &QVariantAnimation::finished, this,
                        [this, animation] {
                            if (progressAnimation_ == animation) {
                                progressAnimation_ = nullptr;
                            }
                        });
                animation->start(QAbstractAnimation::DeleteWhenStopped);
            }

        protected:
            void paintEvent(QPaintEvent*) override
            {
                QPainter painter(this);
                painter.setRenderHint(QPainter::Antialiasing, true);
                const QRectF bounds = QRectF(rect()).adjusted(
                    0.75, 0.75, -0.75, -0.75);
                QPainterPath clip;
                clip.addRoundedRect(bounds, 7.0, 7.0);
                painter.fillPath(
                    clip, underMouse() ? QColor(22, 38, 54, 242)
                                       : QColor(15, 27, 41, 228));

                painter.save();
                painter.setClipPath(clip);
                const qreal liquidTop = bounds.bottom() -
                    bounds.height() * progress_;
                QPainterPath liquid;
                liquid.moveTo(bounds.left(), bounds.bottom());
                liquid.lineTo(bounds.left(), liquidTop);
                for (qreal x = bounds.left(); x <= bounds.right() + 1.0;
                     x += 1.5) {
                    const qreal wave = std::sin(
                        (x - bounds.left()) * 0.34 + wavePhase_) * 1.35;
                    liquid.lineTo(x, liquidTop + wave);
                }
                liquid.lineTo(bounds.right(), bounds.bottom());
                liquid.closeSubpath();
                QLinearGradient water(bounds.topLeft(), bounds.bottomLeft());
                water.setColorAt(0.0, QColor(92, 238, 174, 235));
                water.setColorAt(1.0, QColor(20, 155, 112, 245));
                painter.fillPath(liquid, water);
                painter.restore();

                painter.setPen(QPen(
                    underMouse() ? QColor(112, 240, 180, 205)
                                 : QColor(77, 201, 148, 120), 1.1));
                painter.drawPath(clip);
                const QColor glyph = progress_ > 0.43
                    ? QColor(246, 255, 251)
                    : QColor(105, 229, 174);
                painter.setPen(QPen(glyph, 2.0, Qt::SolidLine,
                                    Qt::RoundCap, Qt::RoundJoin));
                // Keep the arrow one logical pixel left of the common HUD
                // center line, matching the three glyphs above it.
                const qreal centerX = bounds.center().x();
                painter.drawLine(QPointF(centerX, 7.0),
                                 QPointF(centerX, 17.0));
                painter.drawLine(QPointF(centerX - 3.5, 13.5),
                                 QPointF(centerX, 17.0));
                painter.drawLine(QPointF(centerX + 3.5, 13.5),
                                 QPointF(centerX, 17.0));
                painter.drawLine(QPointF(centerX - 5.0, 20.5),
                                 QPointF(centerX + 5.0, 20.5));
            }

        private:
            QTimer waveTimer_{this};
            QVariantAnimation* progressAnimation_ = nullptr;
            double progress_ = 0.0;
            double targetProgress_ = 0.0;
            double wavePhase_ = 0.0;
        };

        QString SessionBitrateText(std::uint64_t bitsPerSecond)
        {
            if (bitsPerSecond >= 1'000'000) {
                return QStringLiteral("%1 Mbps")
                    .arg(static_cast<double>(bitsPerSecond) / 1'000'000.0,
                        0, 'f', 1);
            }
            return QStringLiteral("%1 Kbps")
                .arg(static_cast<double>(bitsPerSecond) / 1'000.0,
                    0, 'f', 0);
        }

        QString SessionRouteText(const std::string& route)
        {
            if (route == "host") {
                return QStringLiteral("局域网直连");
            }
            if (route == "stun") {
                return QStringLiteral("公网直连");
            }
            if (route == "turn") {
                return QStringLiteral("TURN 中继");
            }
            return QStringLiteral("连接中");
        }

        void AnimateSessionStatusWidget(QWidget* widget)
        {
            const int level = CurrentUiAnimationLevel();
            if (!widget || level <= 0) {
                return;
            }
            auto* effect = new QGraphicsOpacityEffect(widget);
            widget->setGraphicsEffect(effect);
            auto* animation = new QPropertyAnimation(effect, "opacity", effect);
            animation->setDuration(level == 1 ? 90 : 145);
            animation->setStartValue(0.4);
            animation->setEndValue(1.0);
            QObject::connect(animation, &QPropertyAnimation::finished, widget,
                [widget, effect] {
                    if (widget->graphicsEffect() == effect) {
                        widget->setGraphicsEffect(nullptr);
                    }
                });
            animation->start(QAbstractAnimation::DeleteWhenStopped);
        }

        constexpr auto kSessionStyle = R"(
QMainWindow, QWidget#sessionRoot {
    background: #0c111b;
    color: #eef2f8;
    font-size: 12px;
}
QWidget#customTitleBar {
    background: #0b111c;
    border-bottom: 1px solid #222d40;
}
QLabel#titleBarAppName {
    color: #f7f9fc;
    font-size: 13px;
    font-weight: 700;
}
QLabel#titleBarDivider, QLabel#titleBarTitle {
    color: #8190a6;
    font-size: 12px;
}
QToolButton#titleBarButton, QToolButton#titleBarCloseButton {
    background: transparent;
    border: none;
    color: #b8c1d0;
    font-family: "Segoe UI Symbol";
    font-size: 15px;
}
QToolButton#titleBarButton:hover {
    background: #263247;
    color: white;
}
QToolButton#titleBarCloseButton:hover {
    background: #d84a55;
    color: white;
}
QFrame#sessionToolbar {
    background: rgba(15, 23, 36, 247);
    border: 1px solid #2a3850;
    border-radius: 18px;
}
QFrame#sessionToolbar[fullscreen="true"] {
    border-top: none;
    border-top-left-radius: 0;
    border-top-right-radius: 0;
}
QFrame#sessionDeviceCard {
    background: rgba(21, 32, 49, 220);
    border: 1px solid #293850;
    border-radius: 12px;
}
QLabel#deviceIcon {
    color: #8b72ff;
    font-family: "Segoe UI Emoji", "Segoe UI Symbol";
    font-size: 23px;
}
QFrame#sessionMetricBlock {
    background: transparent;
    border: none;
}
QLabel#metricIcon {
    color: #627dff;
    font-family: "Segoe UI Symbol";
    font-size: 23px;
}
QLabel#metricTitle {
    color: #8794a9;
    font-size: 10px;
}
QLabel#metricValue {
    color: #f0f4fa;
    font-size: 12px;
    font-weight: 700;
}
QLabel#metricGoodValue {
    color: #54dc9a;
    font-size: 12px;
    font-weight: 700;
}
QFrame#toolbarSeparator {
    background: #2b374a;
    border: none;
    min-width: 1px;
    max-width: 1px;
    margin-top: 11px;
    margin-bottom: 11px;
}
QFrame#sessionActionCluster {
    background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                                stop:0 #16243a, stop:1 #101a2b);
    border: 1px solid #31435d;
    border-radius: 15px;
}
QFrame#actionInnerSeparator {
    background: #2a3a50;
    border: none;
    min-width: 1px;
    max-width: 1px;
    margin-top: 10px;
    margin-bottom: 10px;
}
QLabel#sessionName {
    color: white;
    font-size: 14px;
    font-weight: 700;
}
QLabel#sessionMeta {
    color: #8e9aad;
    font-size: 11px;
}
QPushButton#sessionActionTile {
    background: rgba(24, 37, 56, 210);
    border: 1px solid #344760;
    border-radius: 11px;
    padding: 0;
    margin: 0;
    min-width: 0;
    min-height: 0;
}
QPushButton#sessionActionTile[interactive="true"]:hover {
    background: rgba(39, 56, 80, 185);
    border-color: #405878;
}
QPushButton#sessionActionTile[interactive="true"]:pressed {
    background: rgba(25, 38, 57, 230);
    border-color: #4d6688;
}
QPushButton#sessionActionTile[tone="muted"] {
    background: transparent;
    border-color: transparent;
}
QPushButton#sessionActionTile[tone="positive"] {
    background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                                stop:0 #205444, stop:1 #173b31);
    border-color: #37816b;
}
QPushButton#sessionActionTile[tone="positive"]:hover {
    background: #245f4d;
    border-color: #43a083;
}
QPushButton#sessionActionTile[tone="positive"]:pressed {
    background: #14342a;
    border-color: #32735f;
}
QPushButton#sessionActionTile[tone="primary"] {
    background: #182f62;
    border-color: #3159a5;
}
QPushButton#sessionActionTile[tone="primary"]:hover {
    background: #1d2b3f;
    border-color: #39506f;
}
QPushButton#sessionActionTile[tone="primary"]:pressed {
    background: #172234;
    border-color: #466182;
}
QPushButton#sessionActionTile[tone="primary"]:disabled {
    background: transparent;
    border-color: transparent;
}
QPushButton#sessionActionTile[tone="danger"] {
    background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                                stop:0 #40232a, stop:1 #351c22);
    border-color: #69303a;
    border-radius: 18px;
}
QPushButton#sessionActionTile[tone="danger"]:hover {
    background: #512830;
    border-color: #853c49;
}
QPushButton#sessionActionTile[tone="danger"]:pressed {
    background: #2f171c;
    border-color: #5f2932;
}
QToolButton {
    background: transparent;
    border: none;
    border-radius: 7px;
    color: #b8c1d0;
    min-width: 52px;
    padding: 6px 8px;
}
QToolButton#sessionDisplayTile {
    background: rgba(21, 32, 49, 210);
    border: 1px solid #2d3d56;
    border-radius: 10px;
    color: #eef3fa;
    font-size: 11px;
    font-weight: 700;
    min-width: 104px;
    min-height: 50px;
    padding: 3px 7px;
}
QToolButton#sessionDisplayTile:hover {
    background: #202d40;
    border-color: #45618b;
    color: white;
}
QToolButton#sessionFrameRateTile {
    background: transparent;
    border: 1px solid transparent;
    border-radius: 10px;
    color: #eef3fa;
    font-size: 12px;
    font-weight: 700;
    min-width: 82px;
    min-height: 52px;
    padding: 3px 6px;
}
QToolButton#sessionFrameRateTile:hover {
    background: #202d40;
    border-color: #30415b;
}
QToolButton:hover {
    background: #242e40;
    color: white;
}
QToolButton:pressed {
    background: #303c50;
}
QToolButton::menu-indicator {
    image: none;
}
QMenu {
    background: #182130;
    border: 1px solid #344158;
    border-radius: 10px;
    color: #dce4ef;
    padding: 6px;
}
QMenu::item {
    border-radius: 7px;
    min-width: 112px;
    padding: 8px 22px 8px 12px;
}
QMenu::item:selected {
    background: #293650;
    color: white;
}
QMenu::item:checked {
    background: #22345e;
    color: #82a3ff;
    font-weight: 700;
}
QPushButton#sessionMediaRefreshButton {
    background: transparent;
    border: none;
    border-radius: 7px;
    color: #dce4ef;
    min-height: 36px;
    padding: 0 12px;
    text-align: left;
}
QPushButton#sessionMediaRefreshButton:hover {
    background: #293650;
    color: white;
}
QPushButton#sessionMediaRefreshButton:disabled {
    color: #77859a;
}
QWidget#sessionHud {
    background: rgba(8, 15, 25, 120);
    border: 1px solid rgba(174, 181, 182, 88);
    border-radius: 12px;
}
QLabel#hudFrameRateLabel {
    background: transparent;
    border: none;
    color: #66dda0;
    font-size: 12px;
    font-weight: 800;
    padding: 0;
}
QToolButton#sessionHudButton {
    background: rgba(19, 30, 46, 142);
    border: 1px solid rgba(190, 188, 178, 96);
    border-radius: 8px;
    color: #dce5f3;
    font-size: 16px;
    font-weight: 700;
    min-width: 0;
    padding: 0;
}
QToolButton#sessionHudButton:hover {
    background: rgba(39, 57, 82, 188);
    border-color: rgba(220, 216, 204, 170);
    color: white;
}
QToolButton#sessionHudButton[locked="true"] {
    background: rgba(43, 34, 18, 158);
    border-color: rgba(255, 200, 87, 185);
    color: #ffc857;
}
QToolButton#sessionHudButton[transferActive="true"] {
    background: rgba(22, 91, 74, 205);
    border-radius: 14px;
    color: #70f0b4;
}
QToolButton#sessionHudButton[transferActive="true"]:hover {
    background: rgba(30, 119, 94, 225);
    color: white;
}
QLabel#goodStatus {
    color: #5fd59b;
    font-weight: 600;
}
QLabel#statusText {
    color: #8996aa;
    font-size: 11px;
}
)";

        using Microsoft::WRL::ComPtr;

#include "RemoteSessionNetworkIndicator.inc"
#include "RemoteCursorOverlay.inc"
#include "D3D11VideoSurface.inc"
#include "RemoteDesktopCanvas.Public.inc"
#include "RemoteDesktopCanvas.Rendering.inc"
#include "RemoteDesktopCanvas.Input.inc"
#include "RemoteDesktopCanvas.State.inc"
#include "RemoteSessionWindow.Helpers.inc"
    }  // namespace

#include "RemoteSessionWindow.Lifecycle.inc"
#include "RemoteSessionWindow.SessionState.inc"
#include "RemoteSessionWindow.Layout.inc"
#include "RemoteSessionWindow.Diagnostics.inc"
#include "RemoteSessionWindow.Ui.inc"
#include "RemoteSessionWindow.Controls.inc"
#include "RemoteSessionWindow.MediaMenu.inc"
#include "RemoteSessionWindow.DisplayMenu.inc"
#include "RemoteSessionWindow.MediaSelection.inc"
#include "RemoteSessionWindow.WindowActions.inc"
}  // namespace remote::controller
