// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#include "RoomCameraWindow.h"

#include <QCloseEvent>
#include <QFrame>
#include <QGraphicsOpacityEffect>
#include <QGuiApplication>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QImage>
#include <QLabel>
#include <QMetaObject>
#include <QMouseEvent>
#include <QPainter>
#include <QPushButton>
#include <QPropertyAnimation>
#include <QResizeEvent>
#include <QScreen>
#include <QScrollArea>
#include <QSet>
#include <QStackedWidget>
#include <QStyle>
#include <QTimer>
#include <QToolButton>
#include <QVBoxLayout>
#include <QWidget>

#include <algorithm>
#include <functional>

#include "api/video/video_frame.h"
#include "api/video/video_sink_interface.h"
#include "libyuv/convert_argb.h"
#include "src/apps/remote/InProcessSessionEngine.h"
#include "src/core/RoomState.h"

namespace remote::controller {
namespace {

constexpr auto kCameraGalleryStyle = R"(
QMainWindow, QWidget#cameraGalleryRoot {
    background: #0b1019;
    color: #eef2f8;
    font-size: 12px;
}
QWidget#customTitleBar {
    background: #0b1019;
    border-bottom: 1px solid #222b3b;
}
QLabel#titleBarAppName { color: #f7f9fc; font-weight: 700; }
QLabel#titleBarDivider, QLabel#titleBarTitle { color: #8290a4; }
QToolButton#titleBarButton, QToolButton#titleBarCloseButton {
    background: transparent; border: none; color: #b8c1d0;
    font-family: "Segoe UI Symbol"; font-size: 15px;
}
QToolButton#titleBarButton:hover { background: #263247; color: white; }
QToolButton#titleBarCloseButton:hover { background: #d84a55; color: white; }
QFrame#galleryHeader {
    background: #111823;
    border-bottom: 1px solid #222c3c;
}
QLabel#galleryTitle { color: white; font-size: 15px; font-weight: 700; }
QLabel#galleryHint { color: #8491a5; font-size: 11px; }
QLabel#galleryCount {
    background: #202b40; border: 1px solid #33415a; border-radius: 10px;
    color: #aebcff; font-weight: 700; padding: 4px 10px;
}
QPushButton#galleryModeButton {
    background: #202b40; border: 1px solid #33415a; border-radius: 8px;
    color: #dce3f1; font-weight: 650; padding: 5px 10px;
}
QPushButton#galleryModeButton:hover { background: #2a3750; color: white; }
QPushButton#galleryModeButton:disabled { color: #67758a; }
QFrame#cameraTile {
    background: #111824; border: 1px solid #283246; border-radius: 12px;
}
QFrame#cameraTile[thumbnail="true"] {
    border-radius: 9px;
}
QLabel#emptyGallery { color: #7f8ca0; font-size: 14px; }
QLabel#cameraName {
    background: rgba(12, 17, 27, 205); border-radius: 8px;
    color: white; font-weight: 650; padding: 5px 9px;
}
QLabel#cameraBadge {
    background: rgba(78, 94, 221, 220); border-radius: 8px;
    color: white; font-size: 10px; font-weight: 700; padding: 4px 8px;
}
QScrollArea#cameraThumbnailStrip {
    background: #0e151f; border: 1px solid #252f40; border-radius: 10px;
}
QScrollArea#cameraThumbnailStrip QWidget#qt_scrollarea_viewport {
    background: transparent;
}
QScrollBar:horizontal {
    background: transparent; height: 6px; margin: 0 4px;
}
QScrollBar::handle:horizontal {
    background: #3a465a; border-radius: 3px; min-width: 28px;
}
QScrollBar::add-line:horizontal, QScrollBar::sub-line:horizontal { width: 0; }
)";

QString MemberName(const RoomMemberSnapshot& member)
{
    return member.deviceName.empty()
               ? QString::fromStdString(member.deviceId)
               : QString::fromStdString(member.deviceName);
}

void ClearLayout(QLayout* layout)
{
    while (auto* item = layout->takeAt(0)) {
        delete item;
    }
}

void AnimateCameraTileEntrance(QWidget* widget)
{
    const int level = CurrentUiAnimationLevel();
    if (!widget || level <= 0) {
        return;
    }
    auto* effect = new QGraphicsOpacityEffect(widget);
    widget->setGraphicsEffect(effect);
    auto* animation = new QPropertyAnimation(effect, "opacity", effect);
    animation->setDuration(level == 1 ? 100 : 165);
    animation->setStartValue(0.15);
    animation->setEndValue(1.0);
    QObject::connect(animation, &QPropertyAnimation::finished, widget,
                     [widget, effect] {
                if (widget->graphicsEffect() == effect) {
                    widget->setGraphicsEffect(nullptr);
                }
            });
    animation->start(QAbstractAnimation::DeleteWhenStopped);
}

}  // namespace

class RoomCameraTile final
    : public QFrame,
      public webrtc::VideoSinkInterface<webrtc::VideoFrame> {
public:
    RoomCameraTile(QString name,
                   bool local,
                   std::function<void(bool)> activate,
                   QWidget* parent = nullptr)
        : QFrame(parent),
          name_(std::move(name)),
          local_(local),
          activate_(std::move(activate))
    {
        setObjectName(QStringLiteral("cameraTile"));
        setMinimumSize(300, 169);
        setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

        nameLabel_ = new QLabel(name_, this);
        nameLabel_->setObjectName(QStringLiteral("cameraName"));
        nameLabel_->setAttribute(Qt::WA_TransparentForMouseEvents);
        nameLabel_->adjustSize();
        if (local_) {
            badgeLabel_ = new QLabel(QStringLiteral("本机 · 镜像预览"), this);
            badgeLabel_->setObjectName(QStringLiteral("cameraBadge"));
            badgeLabel_->setAttribute(Qt::WA_TransparentForMouseEvents);
            badgeLabel_->adjustSize();
        }
    }

    void SetName(const QString& name)
    {
        name_ = name;
        nameLabel_->setText(name);
        nameLabel_->adjustSize();
        update();
    }

    bool IsLocal() const { return local_; }

    void SetPresentation(bool thumbnail, bool overview)
    {
        thumbnail_ = thumbnail;
        setProperty("thumbnail", thumbnail);
        if (thumbnail) {
            setMinimumSize(160, 90);
        } else {
            setMinimumSize(overview ? QSize(128, 72) : QSize(280, 158));
        }
        setMaximumSize(QWIDGETSIZE_MAX, QWIDGETSIZE_MAX);
        setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        if (badgeLabel_) {
            badgeLabel_->setText(
                thumbnail ? QStringLiteral("本机")
                          : QStringLiteral("本机 · 镜像预览"));
            badgeLabel_->adjustSize();
        }
        style()->unpolish(this);
        style()->polish(this);
        updateGeometry();
    }

    void OnFrame(const webrtc::VideoFrame& frame) override
    {
        const auto source = frame.video_frame_buffer();
        const auto i420 = source ? source->ToI420() : nullptr;
        if (!i420) {
            return;
        }
        QImage image(i420->width(), i420->height(), QImage::Format_ARGB32);
        if (image.isNull() ||
            libyuv::I420ToARGB(
                i420->DataY(), i420->StrideY(),
                i420->DataU(), i420->StrideU(),
                i420->DataV(), i420->StrideV(),
                image.bits(), image.bytesPerLine(),
                i420->width(), i420->height()) != 0) {
            return;
        }
        QMetaObject::invokeMethod(
            this,
            [this, image = std::move(image)]() mutable {
                frame_ = std::move(image);
                update();
            },
            Qt::QueuedConnection);
    }

protected:
    void mousePressEvent(QMouseEvent* event) override
    {
        if (event->button() == Qt::LeftButton && activate_) {
            activate_(false);
        }
        QFrame::mousePressEvent(event);
    }

    void mouseDoubleClickEvent(QMouseEvent* event) override
    {
        if (event->button() == Qt::LeftButton && activate_) {
            activate_(true);
            event->accept();
            return;
        }
        QFrame::mouseDoubleClickEvent(event);
    }

    void resizeEvent(QResizeEvent* event) override
    {
        QFrame::resizeEvent(event);
        const int margin = thumbnail_ ? 8 : 14;
        nameLabel_->move(margin, height() - nameLabel_->height() - margin);
        if (badgeLabel_) {
            badgeLabel_->move(width() - badgeLabel_->width() - margin,
                              margin);
        }
    }

    void paintEvent(QPaintEvent* event) override
    {
        QFrame::paintEvent(event);
        QPainter painter(this);
        painter.setRenderHint(QPainter::SmoothPixmapTransform);
        const QRect viewport = rect().adjusted(1, 1, -1, -1);
        painter.setClipRect(viewport);
        painter.fillRect(viewport, QColor(QStringLiteral("#090d14")));
        if (!frame_.isNull()) {
            QSize scaled = frame_.size().scaled(
                viewport.size(), Qt::KeepAspectRatio);
            QRect target(QPoint(), scaled);
            target.moveCenter(viewport.center());
            if (local_) {
                painter.translate(width(), 0);
                painter.scale(-1, 1);
                target.moveCenter(QPoint(width() - viewport.center().x(),
                                         viewport.center().y()));
            }
            painter.drawImage(target, frame_);
        } else {
            painter.setPen(QColor(QStringLiteral("#66758c")));
            painter.drawText(viewport, Qt::AlignCenter,
                             QStringLiteral("正在等待摄像头画面…"));
        }
        nameLabel_->raise();
        if (badgeLabel_) {
            badgeLabel_->raise();
        }
    }

private:
    QString name_;
    bool local_ = false;
    bool thumbnail_ = false;
    QImage frame_;
    QLabel* nameLabel_ = nullptr;
    QLabel* badgeLabel_ = nullptr;
    std::function<void(bool)> activate_;
};

class CameraAspectRatioHost final : public QWidget {
public:
    explicit CameraAspectRatioHost(RoomCameraTile* tile,
                                   QWidget* parent = nullptr)
        : QWidget(parent), tile_(tile)
    {
        tile_->setParent(this);
        setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    }

    void SetPresentation(bool thumbnail, bool overview)
    {
        thumbnail_ = thumbnail;
        tile_->SetPresentation(thumbnail, overview);
        if (thumbnail_) {
            setFixedSize(168, 98);
        } else {
            setMinimumSize(overview ? QSize(128, 72) : QSize(280, 158));
            setMaximumSize(QWIDGETSIZE_MAX, QWIDGETSIZE_MAX);
            setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        }
        UpdateTileGeometry();
    }

protected:
    void resizeEvent(QResizeEvent* event) override
    {
        QWidget::resizeEvent(event);
        UpdateTileGeometry();
    }

private:
    void UpdateTileGeometry()
    {
        QRect available = rect();
        if (thumbnail_) {
            available.adjust(4, 4, -4, -4);
        }
        if (available.isEmpty()) {
            return;
        }
        QSize videoSize = QSize(16, 9).scaled(
            available.size(), Qt::KeepAspectRatio);
        QRect videoRect(QPoint(), videoSize);
        videoRect.moveCenter(available.center());
        tile_->setGeometry(videoRect);
    }

    RoomCameraTile* tile_ = nullptr;
    bool thumbnail_ = false;
};

RoomCameraWindow::RoomCameraWindow(app::InProcessSessionEngine* engine,
                                   QWidget* parent)
    : FramelessMainWindow(parent), engine_(engine)
{
    setWindowTitle(QStringLiteral("RLink - 房间摄像头"));
    setMinimumSize(340, 420);
    resize(400, 560);
    ApplyUiStyleSheet(QString::fromUtf8(kCameraGalleryStyle));

    auto* root = new QWidget(this);
    root->setObjectName(QStringLiteral("cameraGalleryRoot"));
    auto* layout = new QVBoxLayout(root);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    layout->addWidget(new CustomTitleBar(
        this, QStringLiteral("房间摄像头"), root));

    galleryHeader_ = new QFrame(root);
    galleryHeader_->setObjectName(QStringLiteral("galleryHeader"));
    auto* headerLayout = new QHBoxLayout(galleryHeader_);
    headerLayout->setContentsMargins(20, 13, 20, 13);
    auto* labels = new QVBoxLayout();
    labels->setSpacing(2);
    auto* title = new QLabel(QStringLiteral("摄像头画廊"), galleryHeader_);
    title->setObjectName(QStringLiteral("galleryTitle"));
    auto* hint = new QLabel(QStringLiteral("开启后对全房间可见"), galleryHeader_);
    hint->setObjectName(QStringLiteral("galleryHint"));
    labels->addWidget(title);
    labels->addWidget(hint);
    headerLayout->addLayout(labels, 1);
    countLabel_ = new QLabel(QStringLiteral("0 人开启"), galleryHeader_);
    countLabel_->setObjectName(QStringLiteral("galleryCount"));
    headerLayout->addWidget(countLabel_);
    viewModeButton_ = new QPushButton(QStringLiteral("全览"), galleryHeader_);
    viewModeButton_->setObjectName(QStringLiteral("galleryModeButton"));
    viewModeButton_->setCursor(Qt::PointingHandCursor);
    viewModeButton_->setEnabled(false);
    headerLayout->addWidget(viewModeButton_);
    layout->addWidget(galleryHeader_);

    viewStack_ = new QStackedWidget(root);

    focusPage_ = new QWidget(viewStack_);
    focusLayout_ = new QVBoxLayout(focusPage_);
    focusLayout_->setContentsMargins(12, 12, 12, 12);
    focusLayout_->setSpacing(10);

    thumbnailScroll_ = new QScrollArea(focusPage_);
    thumbnailScroll_->setObjectName(QStringLiteral("cameraThumbnailStrip"));
    thumbnailScroll_->setWidgetResizable(true);
    thumbnailScroll_->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    thumbnailScroll_->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    EnableSmoothWheelScrolling(thumbnailScroll_);
    thumbnailScroll_->setFixedHeight(122);
    thumbnailHost_ = new QWidget(thumbnailScroll_);
    thumbnailLayout_ = new QHBoxLayout(thumbnailHost_);
    thumbnailLayout_->setContentsMargins(8, 8, 8, 8);
    thumbnailLayout_->setSpacing(8);
    thumbnailScroll_->setWidget(thumbnailHost_);

    overviewPage_ = new QWidget(viewStack_);
    overviewGrid_ = new QGridLayout(overviewPage_);
    overviewGrid_->setContentsMargins(12, 12, 12, 12);
    overviewGrid_->setHorizontalSpacing(10);
    overviewGrid_->setVerticalSpacing(10);

    emptyLabel_ = new QLabel(
        QStringLiteral("当前没有成员开启摄像头"), focusPage_);
    emptyLabel_->setObjectName(QStringLiteral("emptyGallery"));
    emptyLabel_->setAlignment(Qt::AlignCenter);
    focusLayout_->addWidget(emptyLabel_, 1);
    viewStack_->addWidget(focusPage_);
    viewStack_->addWidget(overviewPage_);
    layout->addWidget(viewStack_, 1);
    setCentralWidget(root);

    connect(viewModeButton_, &QPushButton::clicked, this, [this] {
        SetOverviewMode(!overviewMode_);
    });
}

RoomCameraWindow::~RoomCameraWindow()
{
    DetachAllSinks();
}

void RoomCameraWindow::SyncSnapshot(const SessionEngineSnapshot& snapshot)
{
    if (!engine_) {
        return;
    }
    QSet<QString> desired;
    QList<QWidget*> newHosts;
    QStringList newOrder;
    QString localDeviceId;
    for (const auto& member : snapshot.room.members) {
        const bool local = member.deviceId == snapshot.localDeviceId;
        const bool localStarting = local &&
            (snapshot.localCamera == LocalCameraState::kStarting ||
             snapshot.localCamera == LocalCameraState::kPublishing ||
             snapshot.localCamera == LocalCameraState::kStopping);
        if (local) {
            if (!member.cameraPublishing && !localStarting) {
                continue;
            }
        } else if (!member.online || !member.cameraPublishing) {
            continue;
        }
        const QString deviceId = QString::fromStdString(member.deviceId);
        desired.insert(deviceId);
        if (local) {
            localDeviceId = deviceId;
        } else {
            newOrder.push_back(deviceId);
        }
        RoomCameraTile* tile = tiles_.value(deviceId, nullptr);
        if (!tile) {
            tile = new RoomCameraTile(
                MemberName(member), local,
                [this, deviceId](bool maximize) {
                    SetFocusedDevice(deviceId, maximize);
                },
                nullptr);
            tiles_.insert(deviceId, tile);
            auto* host = new CameraAspectRatioHost(tile, focusPage_);
            tileHosts_.insert(deviceId, host);
            newHosts.push_back(host);
            if (local) {
                engine_->SetLocalCameraPreviewSink(tile);
            }
        } else {
            tile->SetName(MemberName(member));
        }

        if (local) {
            continue;
        }
        const auto pair = std::find_if(
            snapshot.roomPeerConnections.begin(),
            snapshot.roomPeerConnections.end(),
            [&member](const RoomPeerConnectionSnapshot& candidate) {
                return candidate.peerDeviceId == member.deviceId &&
                       candidate.state == RoomPeerConnectionState::kActive;
            });
        const QString newPair = pair == snapshot.roomPeerConnections.end()
                                    ? QString()
                                    : QString::fromStdString(pair->pairId);
        const QString oldPair = pairBindings_.value(deviceId);
        if (oldPair == newPair) {
            continue;
        }
        if (!oldPair.isEmpty()) {
            engine_->SetRoomRemoteVideoSink(
                oldPair.toStdString(), kCameraMainVideoSlot, nullptr);
        }
        if (!newPair.isEmpty()) {
            engine_->SetRoomRemoteVideoSink(
                newPair.toStdString(), kCameraMainVideoSlot, tile);
            pairBindings_.insert(deviceId, newPair);
        } else {
            pairBindings_.remove(deviceId);
        }
    }

    const auto currentIds = tiles_.keys();
    for (const auto& deviceId : currentIds) {
        if (!desired.contains(deviceId)) {
            RemoveTile(deviceId);
        }
    }
    if (!localDeviceId.isEmpty()) {
        newOrder.push_back(localDeviceId);
    }
    orderedDeviceIds_ = newOrder;
    if (!orderedDeviceIds_.contains(focusedDeviceId_)) {
        focusedDeviceId_ = orderedDeviceIds_.isEmpty()
                               ? QString()
                               : orderedDeviceIds_.front();
    }
    if (orderedDeviceIds_.size() <= 1) {
        overviewMode_ = false;
    }
    if (orderedDeviceIds_.isEmpty()) {
        initialPlacementDone_ = false;
    }
    RebuildLayout();
    for (auto* host : newHosts) {
        AnimateCameraTileEntrance(host);
    }
}

void RoomCameraWindow::OpenBesideMainWindow(
    const QRect& mainWindowGeometry)
{
    const bool animateEntrance = !isVisible() || isMinimized();
    if (!initialPlacementDone_) {
        PlaceBesideMainWindow(mainWindowGeometry);
        initialPlacementDone_ = true;
    }
    if (isMinimized()) {
        showNormal();
    } else {
        show();
    }
    if (animateEntrance) {
        AnimateWindowEntrance(QPoint(12, 0));
    }
    raise();
    activateWindow();
}

void RoomCameraWindow::SetHiddenByUserCallback(
    std::function<void()> callback)
{
    hiddenByUserCallback_ = std::move(callback);
}

void RoomCameraWindow::closeEvent(QCloseEvent* event)
{
    if (hiddenByUserCallback_) {
        hiddenByUserCallback_();
    }
    hide();
    event->ignore();
}

QRect RoomCameraWindow::ConstrainResizeGeometry(
    const QRect& proposedGeometry,
    Qt::Edges resizeEdges,
    qreal devicePixelRatio) const
{
    if (!singleParticipantMode_ || isMaximized() || isFullScreen() ||
        devicePixelRatio <= 0.0) {
        return proposedGeometry;
    }

    QRect constrained = proposedGeometry;
    const bool horizontalResize =
        resizeEdges.testFlag(Qt::LeftEdge) ||
        resizeEdges.testFlag(Qt::RightEdge);
    if (horizontalResize) {
        const int logicalWidth = qRound(
            proposedGeometry.width() / devicePixelRatio);
        const int physicalHeight = qMax(
            qRound(minimumHeight() * devicePixelRatio),
            qRound(SingleWindowHeightForWidth(logicalWidth) *
                   devicePixelRatio));
        if (resizeEdges.testFlag(Qt::TopEdge)) {
            constrained.setY(
                proposedGeometry.y() + proposedGeometry.height() -
                physicalHeight);
        }
        constrained.setHeight(physicalHeight);
        return constrained;
    }

    const int logicalHeight = qRound(
        proposedGeometry.height() / devicePixelRatio);
    const int physicalWidth = qMax(
        qRound(minimumWidth() * devicePixelRatio),
        qRound(SingleWindowWidthForHeight(logicalHeight) *
               devicePixelRatio));
    constrained.setWidth(physicalWidth);
    return constrained;
}

void RoomCameraWindow::RebuildLayout()
{
    UpdateSingleParticipantMode(orderedDeviceIds_.size());
    countLabel_->setText(
        QStringLiteral("%1 人开启").arg(orderedDeviceIds_.size()));
    viewModeButton_->setEnabled(orderedDeviceIds_.size() > 1);
    viewModeButton_->setText(
        overviewMode_ ? QStringLiteral("聚焦") : QStringLiteral("全览"));
    if (overviewMode_) {
        RebuildOverviewLayout();
    } else {
        RebuildFocusLayout();
    }
}

void RoomCameraWindow::RebuildFocusLayout()
{
    ClearLayout(focusLayout_);
    ClearLayout(thumbnailLayout_);
    if (orderedDeviceIds_.isEmpty()) {
        emptyLabel_->setParent(focusPage_);
        emptyLabel_->show();
        focusLayout_->addWidget(emptyLabel_, 1);
        viewStack_->setCurrentWidget(focusPage_);
        return;
    }

    emptyLabel_->hide();
    auto* focusedTile = tiles_.value(focusedDeviceId_, nullptr);
    if (!focusedTile) {
        focusedDeviceId_ = orderedDeviceIds_.front();
        focusedTile = tiles_.value(focusedDeviceId_, nullptr);
    }
    auto* focusedHost = static_cast<CameraAspectRatioHost*>(
        tileHosts_.value(focusedDeviceId_, nullptr));
    if (!focusedHost) {
        return;
    }
    focusedHost->setParent(focusPage_);
    focusedHost->SetPresentation(false, false);
    focusedTile->show();
    focusedHost->show();
    focusLayout_->addWidget(focusedHost, 1);

    for (const auto& deviceId : orderedDeviceIds_) {
        if (deviceId == focusedDeviceId_) {
            continue;
        }
        auto* tile = tiles_.value(deviceId, nullptr);
        if (!tile) {
            continue;
        }
        auto* host = static_cast<CameraAspectRatioHost*>(
            tileHosts_.value(deviceId, nullptr));
        if (!host) {
            continue;
        }
        host->setParent(thumbnailHost_);
        host->SetPresentation(true, false);
        tile->show();
        host->show();
        thumbnailLayout_->addWidget(host);
    }
    thumbnailLayout_->addStretch(1);
    thumbnailScroll_->setVisible(orderedDeviceIds_.size() > 1);
    if (thumbnailScroll_->isVisible()) {
        focusLayout_->addWidget(thumbnailScroll_);
    }
    viewStack_->setCurrentWidget(focusPage_);
}

void RoomCameraWindow::RebuildOverviewLayout()
{
    ClearLayout(overviewGrid_);
    for (int index = 0; index < 6; ++index) {
        overviewGrid_->setColumnStretch(index, 0);
        overviewGrid_->setRowStretch(index, 0);
    }
    const int count = orderedDeviceIds_.size();
    for (int index = 0; index < count; ++index) {
        auto* tile = tiles_.value(orderedDeviceIds_.at(index), nullptr);
        auto* host = static_cast<CameraAspectRatioHost*>(
            tileHosts_.value(orderedDeviceIds_.at(index), nullptr));
        if (!tile || !host) {
            continue;
        }
        host->setParent(overviewPage_);
        host->SetPresentation(false, true);
        tile->show();
        host->show();
        if (count == 1) {
            overviewGrid_->addWidget(host, 0, 0);
        } else if (count == 2) {
            overviewGrid_->addWidget(host, 0, index);
        } else if (count == 3) {
            if (index == 0) {
                overviewGrid_->addWidget(host, 0, 0, 1, 2);
            } else {
                overviewGrid_->addWidget(host, 1, index - 1);
            }
        } else if (count == 4) {
            overviewGrid_->addWidget(host, index / 2, index % 2);
        } else {
            if (index < 3) {
                overviewGrid_->addWidget(host, 0, index * 2, 1, 2);
            } else {
                overviewGrid_->addWidget(host, 1,
                                         1 + (index - 3) * 2, 1, 2);
            }
        }
    }
    const int columns = count == 5 ? 6 : (count > 1 ? 2 : 1);
    const int rows = count <= 2 ? 1 : 2;
    for (int column = 0; column < columns; ++column) {
        overviewGrid_->setColumnStretch(column, 1);
    }
    for (int row = 0; row < rows; ++row) {
        overviewGrid_->setRowStretch(row, 1);
    }
    viewStack_->setCurrentWidget(overviewPage_);
}

void RoomCameraWindow::SetFocusedDevice(const QString& deviceId,
                                        bool maximize)
{
    if (!tiles_.contains(deviceId)) {
        return;
    }
    focusedDeviceId_ = deviceId;
    overviewMode_ = false;
    RebuildLayout();
    AnimateCameraTileEntrance(tileHosts_.value(deviceId, nullptr));
    if (maximize) {
        isMaximized() ? showNormal() : showMaximized();
    }
}

void RoomCameraWindow::SetOverviewMode(bool overview)
{
    overviewMode_ = overview && orderedDeviceIds_.size() > 1;
    RebuildLayout();
}

void RoomCameraWindow::UpdateSingleParticipantMode(int participantCount)
{
    const bool single = participantCount == 1;
    viewModeButton_->setVisible(!single);
    if (singleParticipantMode_ == single) {
        return;
    }

    singleParticipantMode_ = single;
    if (singleParticipantMode_) {
        setMinimumHeight(300);
        if (!isMaximized() && !isFullScreen()) {
            QTimer::singleShot(0, this, [this] {
                if (singleParticipantMode_ &&
                    !isMaximized() && !isFullScreen()) {
                    resize(width(), SingleWindowHeightForWidth(width()));
                }
            });
        }
        return;
    }

    setMinimumHeight(420);
    if (!isMaximized() && !isFullScreen() && height() < minimumHeight()) {
        resize(width(), minimumHeight());
    }
}

int RoomCameraWindow::SingleWindowHeightForWidth(int windowWidth) const
{
    const QMargins margins = focusLayout_->contentsMargins();
    const int videoWidth = qMax(
        1, windowWidth - margins.left() - margins.right());
    const int headerHeight = galleryHeader_
        ? qMax(galleryHeader_->height(), galleryHeader_->sizeHint().height())
        : 60;
    const int chromeHeight = 40 + headerHeight +
        margins.top() + margins.bottom();
    return qMax(minimumHeight(),
                chromeHeight + qRound(videoWidth * 9.0 / 16.0));
}

int RoomCameraWindow::SingleWindowWidthForHeight(int windowHeight) const
{
    const QMargins margins = focusLayout_->contentsMargins();
    const int headerHeight = galleryHeader_
        ? qMax(galleryHeader_->height(), galleryHeader_->sizeHint().height())
        : 60;
    const int chromeHeight = 40 + headerHeight +
        margins.top() + margins.bottom();
    const int videoHeight = qMax(1, windowHeight - chromeHeight);
    return qMax(minimumWidth(),
                margins.left() + margins.right() +
                qRound(videoHeight * 16.0 / 9.0));
}

void RoomCameraWindow::PlaceBesideMainWindow(
    const QRect& mainWindowGeometry)
{
    QScreen* screen = QGuiApplication::screenAt(mainWindowGeometry.center());
    if (!screen) {
        screen = QGuiApplication::primaryScreen();
    }
    if (!screen) {
        return;
    }
    const QRect available = screen->availableGeometry();
    const int desiredWidth = 400;
    const int desiredHeight = std::min(
        std::clamp(mainWindowGeometry.height(), 460, 600),
        available.height());
    resize(desiredWidth, desiredHeight);

    const int gap = 10;
    int x = mainWindowGeometry.right() + gap;
    if (x + desiredWidth > available.right() + 1) {
        const int leftX = mainWindowGeometry.left() - gap - desiredWidth;
        x = leftX >= available.left()
                ? leftX
                : available.right() - desiredWidth + 1;
    }
    const int maximumY = std::max(
        available.top(), available.bottom() - desiredHeight + 1);
    const int y = std::clamp(mainWindowGeometry.top(), available.top(),
                             maximumY);
    move(x, y);
}

void RoomCameraWindow::RemoveTile(const QString& deviceId)
{
    auto* tile = tiles_.take(deviceId);
    if (!tile) {
        return;
    }
    const QString pairId = pairBindings_.take(deviceId);
    if (!pairId.isEmpty() && engine_) {
        engine_->SetRoomRemoteVideoSink(
            pairId.toStdString(), kCameraMainVideoSlot, nullptr);
    }
    if (engine_ && tile->IsLocal()) {
        engine_->SetLocalCameraPreviewSink(nullptr);
    }
    auto* host = tileHosts_.take(deviceId);
    if (host) {
        delete host;
    } else {
        delete tile;
    }
}

void RoomCameraWindow::DetachAllSinks()
{
    if (!engine_) {
        return;
    }
    engine_->SetLocalCameraPreviewSink(nullptr);
    for (auto it = pairBindings_.cbegin(); it != pairBindings_.cend(); ++it) {
        engine_->SetRoomRemoteVideoSink(
            it.value().toStdString(), kCameraMainVideoSlot, nullptr);
    }
    pairBindings_.clear();
}

}  // namespace remote::controller
