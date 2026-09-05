// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#pragma once

#include <functional>

#include <QHash>
#include <QRect>
#include <QString>
#include <QStringList>

#include "FramelessWindow.h"
#include "src/core/ISessionEngine.h"

class QCloseEvent;
class QGridLayout;
class QHBoxLayout;
class QLabel;
class QPushButton;
class QScrollArea;
class QStackedWidget;
class QVBoxLayout;

namespace remote::app {
class InProcessSessionEngine;
}

namespace remote::controller {

class RoomCameraTile;

class RoomCameraWindow final : public FramelessMainWindow {
public:
    explicit RoomCameraWindow(app::InProcessSessionEngine* engine,
                              QWidget* parent = nullptr);
    ~RoomCameraWindow() override;

    void SyncSnapshot(const SessionEngineSnapshot& snapshot);
    void OpenBesideMainWindow(const QRect& mainWindowGeometry);
    void SetHiddenByUserCallback(std::function<void()> callback);

protected:
    void closeEvent(QCloseEvent* event) override;
    QRect ConstrainResizeGeometry(const QRect& proposedGeometry,
                                  Qt::Edges resizeEdges,
                                  qreal devicePixelRatio) const override;

private:
    void RebuildLayout();
    void RebuildFocusLayout();
    void RebuildOverviewLayout();
    void SetFocusedDevice(const QString& deviceId, bool maximize);
    void SetOverviewMode(bool overview);
    void UpdateSingleParticipantMode(int participantCount);
    int SingleWindowHeightForWidth(int windowWidth) const;
    int SingleWindowWidthForHeight(int windowHeight) const;
    void PlaceBesideMainWindow(const QRect& mainWindowGeometry);
    void RemoveTile(const QString& deviceId);
    void DetachAllSinks();

    app::InProcessSessionEngine* engine_ = nullptr;
    QStackedWidget* viewStack_ = nullptr;
    QWidget* galleryHeader_ = nullptr;
    QWidget* focusPage_ = nullptr;
    QVBoxLayout* focusLayout_ = nullptr;
    QWidget* thumbnailHost_ = nullptr;
    QHBoxLayout* thumbnailLayout_ = nullptr;
    QScrollArea* thumbnailScroll_ = nullptr;
    QWidget* overviewPage_ = nullptr;
    QGridLayout* overviewGrid_ = nullptr;
    QLabel* countLabel_ = nullptr;
    QLabel* emptyLabel_ = nullptr;
    QPushButton* viewModeButton_ = nullptr;
    QHash<QString, RoomCameraTile*> tiles_;
    QHash<QString, QWidget*> tileHosts_;
    QHash<QString, QString> pairBindings_;
    QStringList orderedDeviceIds_;
    QString focusedDeviceId_;
    bool overviewMode_ = false;
    bool singleParticipantMode_ = false;
    bool initialPlacementDone_ = false;
    std::function<void()> hiddenByUserCallback_;
};

}  // namespace remote::controller
