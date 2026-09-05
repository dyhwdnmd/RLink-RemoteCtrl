// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#pragma once

#include <string>
#include <mutex>
#include <vector>

#include <QRect>
#include <QElapsedTimer>
#include <QHash>
#include <QPointer>
#include <QSet>
#include <QString>

#include "FramelessWindow.h"
#include "src/apps/remote/FileTransferController.h"

class QCloseEvent;
class QComboBox;
class QEvent;
class QLabel;
class QResizeEvent;
class QPushButton;
class QScrollArea;
class QTimer;
class QVBoxLayout;
class QWidget;

namespace remote::controller {

class FileTransferCard;

struct FileTransferPeer {
    std::string deviceId;
    QString displayName;
};

class FileTransferWindow final
    : public FramelessMainWindow,
      private app::IFileTransferControllerObserver {
public:
    explicit FileTransferWindow(app::FileTransferController* controller,
                                QWidget* parent = nullptr);
    ~FileTransferWindow() override;

    void SyncPeers(const std::vector<FileTransferPeer>& peers,
                   const QRect& anchorGeometry);
    void AttachAsDrawer(QWidget* host);
    void OpenBesideMainWindow(const QRect& mainWindowGeometry);
    void HideWithAnimation();
    void HideImmediately();
    void UpdateDrawerGeometry();
    bool IsHiding() const;
    void DetachController();
    void RefreshThemeStyle();

protected:
    void closeEvent(QCloseEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;
    bool eventFilter(QObject* watched, QEvent* event) override;

private:
    void OnFileTransfersChanged(
        const std::vector<app::FileTransferSnapshot>& transfers) override;
    void BuildUi();
    void ApplyTransfers(
        std::vector<app::FileTransferSnapshot> transfers);
    void ScheduleTransferApply();
    void RebuildTransferCards();
    void UpdateTransferCardsIncrementally();
    void SetDrawerWidth(int requestedWidth);
    int EffectiveDrawerWidth() const;
    void StartDrawerMotion(const QPoint& targetPosition, int durationMs);
    void StopDrawerAnimation();
    void ChooseAndSendFile();
    void AcceptTransfer(const std::string& transferId);
    void SaveTransferAs(const std::string& transferId);
    void AcceptTransferToDirectory(const std::string& transferId,
                                   const QString& directory);
    void RejectTransfer(const std::string& transferId);
    void CancelTransfer(const std::string& transferId);
    void ResumeTransfer(const std::string& transferId);
    void OpenTransferFolder(const std::filesystem::path& path);

    app::FileTransferController* controller_ = nullptr;
    QComboBox* peerSelector_ = nullptr;
    CustomTitleBar* titleBar_ = nullptr;
    QPushButton* sendButton_ = nullptr;
    QLabel* peerHint_ = nullptr;
    QLabel* emptyState_ = nullptr;
    QWidget* transferContainer_ = nullptr;
    QScrollArea* transferScroll_ = nullptr;
    QVBoxLayout* transferLayout_ = nullptr;
    QHash<QString, FileTransferCard*> transferCards_;
    std::vector<app::FileTransferSnapshot> transfers_;
    std::mutex pendingTransfersMutex_;
    std::vector<app::FileTransferSnapshot> pendingTransfers_;
    bool transferApplyScheduled_ = false;
    QRect anchorGeometry_;
    QPointer<QWidget> drawerHost_;
    QTimer* drawerMotionTimer_ = nullptr;
    QElapsedTimer drawerMotionClock_;
    QPoint drawerMotionStart_;
    QPoint drawerMotionTarget_;
    int drawerMotionDurationMs_ = 0;
    bool drawerHiding_ = false;
    QWidget* drawerResizeHandle_ = nullptr;
    int drawerWidth_ = 390;
    bool drawerResizeDragging_ = false;
    int drawerResizeStartGlobalX_ = 0;
    int drawerResizeStartWidth_ = 0;
    QSet<QString> announcedIncomingTransfers_;
};

}  // namespace remote::controller
