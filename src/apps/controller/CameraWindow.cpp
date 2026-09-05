// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#include "CameraWindow.h"
#include "src/apps/controller/ui/morph/MorphIconButtonBinding.h"

#include <array>
#include <utility>

#include <QApplication>
#include <QComboBox>
#include <QEvent>
#include <QHBoxLayout>
#include <QIcon>
#include <QLabel>
#include <QLinearGradient>
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>
#include <QPushButton>
#include <QToolButton>
#include <QVBoxLayout>

#include "RemoteCComboBox.h"

namespace remote::controller {
namespace {

constexpr auto kCameraWindowStyle = R"(
QMainWindow, QWidget#cameraRoot {
    background: #0c111b;
    color: #edf2f8;
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
QFrame#cameraToolbar {
    background: #151c29;
    border-bottom: 1px solid #283244;
}
QLabel#cameraName {
    color: white;
    font-size: 14px;
    font-weight: 700;
}
QLabel#cameraMeta, QLabel#cameraStatusText {
    color: #8e9aad;
    font-size: 11px;
}
QLabel#previewBadge {
    background: #3b3322;
    border: 1px solid #67562c;
    border-radius: 8px;
    color: #f0c96b;
    font-size: 10px;
    font-weight: 700;
    padding: 3px 8px;
}
QComboBox {
    background: #202a3a;
    border: 1px solid #313d51;
    border-radius: 7px;
    color: #dfe6f1;
    min-width: 108px;
    padding: 7px 32px 7px 10px;
}
QComboBox:hover {
    border-color: #526078;
}
QComboBox::drop-down {
    border: none;
    width: 30px;
}
QComboBox::down-arrow { image: none; }
QComboBox QAbstractItemView {
    background: #202a3a;
    border: 1px solid #3b475b;
    border-radius: 8px;
    padding: 4px;
    color: white;
    selection-background-color: #5b6cf9;
}
QPushButton#cameraToolButton {
    background: transparent;
    border: none;
    border-radius: 7px;
    color: #b8c1d0;
    padding: 8px 11px;
}
QPushButton#cameraToolButton:hover {
    background: #242e40;
    color: white;
}
QFrame#cameraStatusBar {
    background: #111824;
    border-top: 1px solid #263044;
}
QLabel#cameraGoodStatus {
    color: #5fd59b;
    font-weight: 600;
}
)";

}  // namespace

CameraPreviewWidget::CameraPreviewWidget(QWidget* parent) : QWidget(parent)
{
    setMinimumSize(420, 260);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
}

void CameraPreviewWidget::paintEvent(QPaintEvent*)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    QLinearGradient background(0, 0, width(), height());
    background.setColorAt(0.0, QColor(21, 34, 55));
    background.setColorAt(0.55, QColor(35, 51, 76));
    background.setColorAt(1.0, QColor(15, 24, 39));
    painter.fillRect(rect(), background);

    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor(91, 108, 249, 45));
    painter.drawEllipse(QPointF(width() * 0.72, height() * 0.28),
                        width() * 0.3, height() * 0.52);
    painter.setBrush(QColor(88, 198, 155, 25));
    painter.drawEllipse(QPointF(width() * 0.24, height() * 0.76),
                        width() * 0.24, height() * 0.38);

    const int iconSize = qBound(44, qMin(width(), height()) / 5, 82);
    const QPointF center(width() / 2.0, height() / 2.0 - 25);
    painter.setBrush(QColor(222, 231, 244, 34));
    painter.setPen(QPen(QColor(222, 231, 244, 135), 2));
    painter.drawRoundedRect(
        QRectF(center.x() - iconSize * 0.55, center.y() - iconSize * 0.34,
               iconSize * 0.82, iconSize * 0.68),
        8, 8);
    QPainterPath lens;
    lens.moveTo(center.x() + iconSize * 0.27, center.y() - iconSize * 0.19);
    lens.lineTo(center.x() + iconSize * 0.57, center.y() - iconSize * 0.34);
    lens.lineTo(center.x() + iconSize * 0.57, center.y() + iconSize * 0.34);
    lens.lineTo(center.x() + iconSize * 0.27, center.y() + iconSize * 0.19);
    lens.closeSubpath();
    painter.drawPath(lens);

    painter.setPen(QColor(239, 244, 251));
    QFont titleFont = QApplication::font();
    titleFont.setPixelSize(17);
    titleFont.setWeight(QFont::DemiBold);
    painter.setFont(titleFont);
    painter.drawText(QRectF(20, center.y() + iconSize * 0.55, width() - 40, 28),
                     Qt::AlignCenter, QStringLiteral("等待远程摄像头画面"));
    painter.setPen(QColor(153, 168, 190));
    QFont hintFont = QApplication::font();
    hintFont.setPixelSize(12);
    painter.setFont(hintFont);
    painter.drawText(QRectF(20, center.y() + iconSize * 0.55 + 30,
                            width() - 40, 24),
                     Qt::AlignCenter,
                     QStringLiteral("Camera VideoTrack 建立后将在这里显示"));
}

CameraOverlayWidget::CameraOverlayWidget(
    QString deviceName,
    std::function<void()> detachAction,
    std::function<void()> closeAction,
    QWidget* parent)
    : QWidget(parent)
{
    setObjectName(QStringLiteral("cameraOverlay"));
    setFixedSize(330, 220);
    setStyleSheet(QStringLiteral(R"(
QWidget#cameraOverlay {
    background: #121a27;
    border: 1px solid #3a4659;
    border-radius: 11px;
}
QWidget#cameraOverlayHeader {
    background: #192333;
    border: none;
    border-top-left-radius: 10px;
    border-top-right-radius: 10px;
}
QLabel#overlayTitle {
    color: #f3f6fb;
    font-size: 12px;
    font-weight: 700;
}
QToolButton {
    background: transparent;
    border: none;
    border-radius: 5px;
    color: #aeb9ca;
    padding: 3px 6px;
}
QToolButton:hover {
    background: #2a3649;
    color: white;
}
)"));

    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(1, 1, 1, 1);
    layout->setSpacing(0);
    dragHandle_ = new QWidget(this);
    dragHandle_->setObjectName(QStringLiteral("cameraOverlayHeader"));
    dragHandle_->setFixedHeight(34);
    dragHandle_->installEventFilter(this);
    auto* header = new QHBoxLayout(dragHandle_);
    header->setContentsMargins(10, 0, 5, 0);
    header->setSpacing(4);
    auto* state = new QLabel(QStringLiteral("●"), dragHandle_);
    state->setStyleSheet(QStringLiteral("color:#4FF0B5;"));
    state->setAttribute(Qt::WA_TransparentForMouseEvents);
    header->addWidget(state);
    auto* title = new QLabel(
        QStringLiteral("%1 · 摄像头").arg(std::move(deviceName)), dragHandle_);
    title->setObjectName(QStringLiteral("overlayTitle"));
    title->setAttribute(Qt::WA_TransparentForMouseEvents);
    header->addWidget(title);
    header->addStretch(1);
    auto* detach = new QToolButton(dragHandle_);
    detach->setIcon(QIcon(
        QStringLiteral(":/ui/icons/actions/fullscreen.svg")));
    detach->setToolTip(QStringLiteral("分离摄像头窗口"));
    detach->setIconSize(QSize(16, 16));
    detach->setCursor(Qt::PointingHandCursor);
    remotec::ui::morph::MorphIconButtonBinding::attach(
        detach,
        QStringLiteral(":/ui/icons/lucide/base/maximize.svg"),
        QStringLiteral(":/ui/icons/lucide/base/monitor.svg"),
        remotec::ui::morph::MorphIconButtonBinding::Interaction::Hover,
        QSize(16, 16), QColor(QStringLiteral("#C7D2E3")),
        QColor(QStringLiteral("#FFFFFF")));
    auto* close = new QToolButton(dragHandle_);
    close->setIcon(QIcon(QStringLiteral(":/ui/icons/window/close.svg")));
    close->setToolTip(QStringLiteral("关闭摄像头画面"));
    close->setIconSize(QSize(16, 16));
    close->setCursor(Qt::PointingHandCursor);
    remotec::ui::morph::MorphIconButtonBinding::attach(
        close,
        QStringLiteral(":/ui/icons/lucide/base/x.svg"),
        QStringLiteral(":/ui/icons/lucide/base/circle-x.svg"),
        remotec::ui::morph::MorphIconButtonBinding::Interaction::Hover,
        QSize(16, 16), QColor(QStringLiteral("#C7D2E3")),
        QColor(QStringLiteral("#FF8795")));
    header->addWidget(detach);
    header->addWidget(close);
    layout->addWidget(dragHandle_);
    auto* preview = new CameraPreviewWidget(this);
    preview->setMinimumSize(0, 0);
    preview->setAttribute(Qt::WA_TransparentForMouseEvents);
    layout->addWidget(preview, 1);

    connect(detach, &QToolButton::clicked, this,
            [action = std::move(detachAction)] { action(); });
    connect(close, &QToolButton::clicked, this,
            [action = std::move(closeAction)] { action(); });
}

bool CameraOverlayWidget::eventFilter(QObject* watched, QEvent* event)
{
    if (watched == dragHandle_) {
        if (event->type() == QEvent::MouseButtonPress) {
            const auto* mouseEvent = static_cast<QMouseEvent*>(event);
            if (mouseEvent->button() == Qt::LeftButton) {
                dragging_ = true;
                dragOffset_ = mapFromGlobal(mouseEvent->globalPosition().toPoint());
                return true;
            }
        } else if (event->type() == QEvent::MouseMove && dragging_) {
            const auto* mouseEvent = static_cast<QMouseEvent*>(event);
            QPoint target = parentWidget()->mapFromGlobal(
                                mouseEvent->globalPosition().toPoint()) -
                            dragOffset_;
            target.setX(qBound(0, target.x(),
                               parentWidget()->width() - width()));
            target.setY(qBound(0, target.y(),
                               parentWidget()->height() - height()));
            move(target);
            raise();
            return true;
        } else if (event->type() == QEvent::MouseButtonRelease) {
            dragging_ = false;
            return true;
        }
    }
    return QWidget::eventFilter(watched, event);
}

CameraWindow::CameraWindow(QString deviceId,
                           QString deviceName,
                           std::function<void()> reattachAction,
                           QWidget* parent)
    : FramelessMainWindow(parent),
      deviceId_(std::move(deviceId)),
      deviceName_(std::move(deviceName)),
      reattachAction_(std::move(reattachAction))
{
    BuildUi();
}

void CameraWindow::BuildUi()
{
    setWindowTitle(QStringLiteral("%1 - RLink 摄像头").arg(deviceName_));
    setMinimumSize(620, 420);
    resize(1080, 690);
    ApplyUiStyleSheet(QString::fromUtf8(kCameraWindowStyle));

    auto* rootWidget = new QWidget(this);
    rootWidget->setObjectName(QStringLiteral("cameraRoot"));
    auto* root = new QVBoxLayout(rootWidget);
    root->setContentsMargins(0, 0, 0, 0);
    root->setSpacing(0);
    auto* titleBar = new CustomTitleBar(
        this, QStringLiteral("%1 · 摄像头").arg(deviceName_), rootWidget);
    if (reattachAction_) {
        titleBar->SetMinimizeAction([this] { ReturnToOverlay(); });
    }
    root->addWidget(titleBar);

    auto* toolbar = new QFrame(rootWidget);
    toolbar->setObjectName(QStringLiteral("cameraToolbar"));
    toolbar->setFixedHeight(64);
    auto* toolbarLayout = new QHBoxLayout(toolbar);
    toolbarLayout->setContentsMargins(17, 9, 14, 9);
    toolbarLayout->setSpacing(8);
    auto* nameGroup = new QVBoxLayout();
    nameGroup->setSpacing(0);
    auto* cameraName = new QLabel(QStringLiteral("远程摄像头"), toolbar);
    cameraName->setObjectName(QStringLiteral("cameraName"));
    auto* meta = new QLabel(
        QStringLiteral("%1  ·  %2").arg(deviceName_, deviceId_), toolbar);
    meta->setObjectName(QStringLiteral("cameraMeta"));
    nameGroup->addWidget(cameraName);
    nameGroup->addWidget(meta);
    toolbarLayout->addLayout(nameGroup);
    auto* previewBadge = new QLabel(QStringLiteral("UI 预览"), toolbar);
    previewBadge->setObjectName(QStringLiteral("previewBadge"));
    toolbarLayout->addWidget(previewBadge);
    toolbarLayout->addStretch(1);

    auto* cameraSelect = new RemoteCComboBox(toolbar);
    cameraSelect->addItems(
        {QStringLiteral("Camera 1"), QStringLiteral("USB Camera")});
    cameraSelect->setToolTip(QStringLiteral("选择远程摄像头"));
    toolbarLayout->addWidget(cameraSelect);
    auto* qualitySelect = new RemoteCComboBox(toolbar);
    qualitySelect->addItems({QStringLiteral("1080p"), QStringLiteral("720p"),
                             QStringLiteral("480p")});
    qualitySelect->setToolTip(QStringLiteral("清晰度"));
    toolbarLayout->addWidget(qualitySelect);
    auto* fpsSelect = new RemoteCComboBox(toolbar);
    fpsSelect->addItems({QStringLiteral("30 FPS"), QStringLiteral("15 FPS")});
    fpsSelect->setToolTip(QStringLiteral("帧率"));
    toolbarLayout->addWidget(fpsSelect);

    const std::array cameraActions = {
        std::pair{QStringLiteral("声音"),
                  QStringLiteral(":/ui/icons/actions/speaker.svg")},
        std::pair{QStringLiteral("麦克风"),
                  QStringLiteral(":/ui/icons/actions/microphone.svg")},
        std::pair{QStringLiteral("截图"),
                  QStringLiteral(":/ui/icons/lucide/base/camera.svg")}};
    for (const auto& [text, iconPath] : cameraActions) {
        auto* button = new QPushButton(text, toolbar);
        button->setIcon(QIcon(iconPath));
        button->setIconSize(QSize(17, 17));
        button->setObjectName(QStringLiteral("cameraToolButton"));
        button->setCursor(Qt::PointingHandCursor);
        QString source;
        QString target;
        if (text == QStringLiteral("声音")) {
            source = QStringLiteral("volume-2");
            target = QStringLiteral("volume-x");
        }
        else if (text == QStringLiteral("麦克风")) {
            source = QStringLiteral("mic");
            target = QStringLiteral("mic-off");
        }
        else {
            source = QStringLiteral("camera");
            target = QStringLiteral("scan");
        }
        remotec::ui::morph::MorphIconButtonBinding::attach(
            button,
            QStringLiteral(":/ui/icons/lucide/base/%1.svg").arg(source),
            QStringLiteral(":/ui/icons/lucide/base/%1.svg").arg(target),
            remotec::ui::morph::MorphIconButtonBinding::Interaction::Hover,
            QSize(17, 17), QColor(QStringLiteral("#E3EAF5")),
            QColor(QStringLiteral("#FFFFFF")));
        toolbarLayout->addWidget(button);
    }
    auto* fullScreen = new QPushButton(QStringLiteral("全屏"), toolbar);
    fullScreen->setIcon(
        QIcon(QStringLiteral(":/ui/icons/actions/fullscreen.svg")));
    fullScreen->setIconSize(QSize(17, 17));
    fullScreen->setObjectName(QStringLiteral("cameraToolButton"));
    fullScreen->setCursor(Qt::PointingHandCursor);
    remotec::ui::morph::MorphIconButtonBinding::attach(
        fullScreen,
        QStringLiteral(":/ui/icons/lucide/base/maximize.svg"),
        QStringLiteral(":/ui/icons/lucide/base/minimize.svg"),
        remotec::ui::morph::MorphIconButtonBinding::Interaction::Hover,
        QSize(17, 17), QColor(QStringLiteral("#E3EAF5")),
        QColor(QStringLiteral("#FFFFFF")));
    toolbarLayout->addWidget(fullScreen);
    QPushButton* reattachButton = nullptr;
    if (reattachAction_) {
        reattachButton = new QPushButton(QStringLiteral("收回画中画"), toolbar);
        reattachButton->setIcon(
            QIcon(QStringLiteral(":/ui/icons/actions/display.svg")));
        reattachButton->setIconSize(QSize(17, 17));
        reattachButton->setObjectName(QStringLiteral("cameraToolButton"));
        reattachButton->setCursor(Qt::PointingHandCursor);
        reattachButton->setToolTip(QStringLiteral("返回远程桌面画中画"));
        remotec::ui::morph::MorphIconButtonBinding::attach(
            reattachButton,
            QStringLiteral(":/ui/icons/lucide/base/monitor-smartphone.svg"),
            QStringLiteral(":/ui/icons/lucide/base/monitor.svg"),
            remotec::ui::morph::MorphIconButtonBinding::Interaction::Hover,
            QSize(17, 17), QColor(QStringLiteral("#E3EAF5")),
            QColor(QStringLiteral("#FFFFFF")));
        toolbarLayout->addWidget(reattachButton);
    }
    root->addWidget(toolbar);

    auto* previewContainer = new QWidget(rootWidget);
    auto* previewLayout = new QVBoxLayout(previewContainer);
    previewLayout->setContentsMargins(14, 14, 14, 12);
    previewLayout->addWidget(new CameraPreviewWidget(previewContainer));
    root->addWidget(previewContainer, 1);

    auto* statusBar = new QFrame(rootWidget);
    statusBar->setObjectName(QStringLiteral("cameraStatusBar"));
    statusBar->setFixedHeight(35);
    auto* statusLayout = new QHBoxLayout(statusBar);
    statusLayout->setContentsMargins(16, 0, 16, 0);
    statusLayout->setSpacing(18);
    auto* trackState = new QLabel(QStringLiteral("●  Camera Track 待接入"), statusBar);
    trackState->setObjectName(QStringLiteral("cameraGoodStatus"));
    statusLayout->addWidget(trackState);
    auto* codec = new QLabel(QStringLiteral("H264 · D3D11 硬解优先"), statusBar);
    codec->setObjectName(QStringLiteral("cameraStatusText"));
    statusLayout->addWidget(codec);
    auto* permission = new QLabel(QStringLiteral("需要 viewCamera 权限"), statusBar);
    permission->setObjectName(QStringLiteral("cameraStatusText"));
    statusLayout->addWidget(permission);
    statusLayout->addStretch(1);
    auto* state = new QLabel(QStringLiteral("UI 预览"), statusBar);
    state->setObjectName(QStringLiteral("cameraStatusText"));
    statusLayout->addWidget(state);
    root->addWidget(statusBar);

    setCentralWidget(rootWidget);
    connect(fullScreen, &QPushButton::clicked, this, [this] {
        isFullScreen() ? showNormal() : showFullScreen();
    });
    if (reattachButton) {
        connect(reattachButton, &QPushButton::clicked, this,
                &CameraWindow::ReturnToOverlay);
    }
}

void CameraWindow::ReturnToOverlay()
{
    if (!reattachAction_) {
        showMinimized();
        return;
    }
    reattachAction_();
    close();
}

}  // namespace remote::controller
