// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#pragma once

#include <cstddef>
#include <cstdint>
#include <functional>
#include <memory>

#include <QElapsedTimer>
#include <QPixmap>
#include <QPointer>
#include <QRect>
#include <QStringList>

#include "FramelessWindow.h"
#include "RemoteSessionBinding.h"
#include "src/core/MediaDevice.h"
#include "src/core/SessionDiagnostics.h"
#include "src/protocol/RemoteInputProtocol.h"
#include "src/protocol/RemoteCursorProtocol.h"
#include "src/protocol/ScreenShareControlProtocol.h"

class QLabel;
class QMenu;
class QActionGroup;
class QCloseEvent;
class QEvent;
class QMoveEvent;
class QAbstractAnimation;
class QPropertyAnimation;
class QResizeEvent;
class QTimer;
class QToolButton;
class QWidget;

namespace remote::app {
    class InProcessSessionEngine;
}

namespace remote::controller {

    class ActionTile;
    class RemoteCDialog;
    class RemoteInputDispatcher;

    class RemoteSessionWindow final : public FramelessMainWindow {
    public:
        RemoteSessionWindow(RemoteSessionBinding binding,
            app::InProcessSessionEngine* sessionEngine = nullptr,
            QWidget* parent = nullptr);
        ~RemoteSessionWindow() override;

        RemoteSessionWindow(const RemoteSessionWindow&) = delete;
        RemoteSessionWindow& operator=(const RemoteSessionWindow&) = delete;

        void BindSessionVideo(app::InProcessSessionEngine* engine,
            RemoteSessionBinding binding);
        void RefreshControlState();
        void UpdateDiagnostics(
            const PeerConnectionDiagnosticsSnapshot& diagnostics);
        void SetDisconnectHandler(std::function<void()> handler);
        void SetRemotePasteHandler(
            std::function<bool(const QStringList& localFiles,
                               bool keyboardPaste)> handler);
        void SetRemotePasteCancelHandler(std::function<void()> handler);
        void SetFileTransferHandlers(
            std::function<void()> openHandler,
            std::function<void()> releaseHostHandler);
        void ShowRemotePasteProgress(
            const QString& transferId, const QString& title,
            const QString& message, double progress,
            std::uintptr_t localTargetWindow = 0);
        void CompleteRemotePasteProgress(const QString& transferId);
        void CloseRemotePasteProgress(const QString& transferId);
        void ShowRemotePasteFailure(
            const QString& transferId, const QString& message);
        void SetDragPointerSampleRate(std::uint32_t hertz);
        void SetRoomOnlineMemberCount(std::size_t onlineMemberCount);

    protected:
        bool eventFilter(QObject* watched, QEvent* event) override;
        void changeEvent(QEvent* event) override;
        void closeEvent(QCloseEvent* event) override;
        void moveEvent(QMoveEvent* event) override;
        void resizeEvent(QResizeEvent* event) override;

    private:
        void BuildUi();
        void LayoutSessionOverlays();
        void ShowSessionToolbar(bool animated = true);
        void HideSessionToolbar(bool animated = true);
        void ScheduleSessionToolbarHide();
        void UpdateSessionDuration();
        void UpdateNetworkRecoveryPrompt(
            bool recovering,
            std::uint32_t attempt,
            bool waitingForSignaling,
            bool failed = false);
        void HandleControlAction();
        void RebuildQualityMenu();
        void RebuildFrameRateMenu();
        void HandleFrameRateSelection(std::uint32_t framesPerSecond);
        void HandleQualitySelection(ScreenQualityTier quality);
        bool RequestStreamPreference(bool showError = true);
        void ToggleRemoteSound();
        void ToggleLocalMicrophone();
        void ShowMediaDeviceMenu();
        void ShowRemoteDisplayMenu();
        void BeginMediaDeviceSelection(
            MediaDeviceKind kind, const QString& deviceId);
        void CompleteMediaDeviceSelections(
            const MediaDeviceSnapshot& media);
        void ToggleToolbarLock();
        void ToggleFullScreenMode();
        void HandleDisconnectAction();
        void ReleaseRemoteInputs();
        void HandleRemoteCursorMessage(
            const std::string& pairId,
            const RemoteCursorEnvelope& envelope);
        void BeginScreenStartup(std::uint64_t screenShareGeneration);
        void RequestScreenStartupRefresh(
            std::uint64_t screenShareGeneration,
            std::uint32_t retryStage);
        void HandleFirstScreenPresentation(
            std::uint64_t screenShareGeneration);
        void MinimizeRemotePasteProgress();
        void RestoreRemotePasteProgress();
        void SetRemotePasteStatusButtonVisible(bool visible);
        QRect RemotePasteDialogRestoreGeometry() const;
        QLabel* CreateRemotePasteAnimationOverlay(
            const QPixmap& snapshot, const QRect& geometry,
            qreal opacity = 1.0);

        std::shared_ptr<RemoteInputDispatcher>
            remoteInputDispatcher_;
        RemoteSessionBinding binding_;
        QLabel* sessionSourceLabel_ = nullptr;
        QLabel* durationLabel_ = nullptr;
        QLabel* previewBadge_ = nullptr;
        ActionTile* controlBadge_ = nullptr;
        QLabel* connectionStatusLabel_ = nullptr;
        QWidget* networkSignalIndicator_ = nullptr;
        QLabel* latencyLabel_ = nullptr;
        QLabel* codecLabel_ = nullptr;
        QLabel* resolutionLabel_ = nullptr;
        ActionTile* controlButton_ = nullptr;
        QToolButton* qualityButton_ = nullptr;
        QMenu* qualityMenu_ = nullptr;
        QActionGroup* qualityGroup_ = nullptr;
        QToolButton* frameRateButton_ = nullptr;
        QMenu* frameRateMenu_ = nullptr;
        QActionGroup* frameRateGroup_ = nullptr;
        ActionTile* speakerButton_ = nullptr;
        ActionTile* microphoneButton_ = nullptr;
        ActionTile* fileTransferButton_ = nullptr;
        QToolButton* mediaDeviceButton_ = nullptr;
        QToolButton* remoteDisplayButton_ = nullptr;
        ActionTile* fullScreenButton_ = nullptr;
        QToolButton* toolbarLockButton_ = nullptr;
        QToolButton* remotePasteStatusButton_ = nullptr;
        QLabel* hudFrameRateLabel_ = nullptr;
        CustomTitleBar* sessionTitleBar_ = nullptr;
        QWidget* contentHost_ = nullptr;
        QWidget* toolbarRevealZone_ = nullptr;
        QWidget* sessionToolbar_ = nullptr;
        QWidget* sessionHud_ = nullptr;
        QWidget* remotePasteStatusHost_ = nullptr;
        QWidget* desktopCanvas_ = nullptr;
        QTimer* toolbarHideTimer_ = nullptr;
        QPropertyAnimation* toolbarAnimation_ = nullptr;
        QElapsedTimer sessionElapsed_;
        QElapsedTimer screenStartupElapsed_;
        QTimer* durationTimer_ = nullptr;
        app::InProcessSessionEngine* sessionEngine_ = nullptr;
        bool sessionVideoSinkBound_ = false;
        bool sessionVideoSinkRetryScheduled_ = false;
        bool roomScreenPreferenceRetryScheduled_ = false;
        std::uint64_t screenStartupGeneration_ = 0;
        std::uint32_t screenStartupRefreshAttempts_ = 0;
        bool screenFirstFramePresented_ = false;
        std::uint64_t preferenceRequestedScreenShareEpoch_ = 0;
        std::uint64_t preferenceSentScreenShareEpoch_ = 0;
        std::uint64_t pendingRemoteDisplaySwitchSequence_ = 0;
        QString lastControlActionText_;
        bool inputControlEnabled_ = false;
        bool closeWithoutConfirmation_ = false;
        bool closeConfirmationVisible_ = false;
        RemoteCDialog* networkRecoveryDialog_ = nullptr;
        RemoteCDialog* remotePasteDialog_ = nullptr;
        QString remotePasteTransferId_;
        bool remotePastePromptDismissed_ = false;
        bool remotePasteDialogClosing_ = false;
        bool remotePasteMinimized_ = false;
        bool remotePasteRestoreAnimating_ = false;
        QRect remotePasteRestoreGeometry_;
        QAbstractAnimation* remotePasteAnimation_ = nullptr;
        QWidget* remotePasteAnimationOverlay_ = nullptr;
        QPixmap remotePasteDialogSnapshot_;
        bool pairWasActive_ = false;
        bool recoveryPromptWasVisible_ = false;
        bool networkRecoveryPromptDismissed_ = false;
        bool networkRecoveryFailureDismissed_ = false;
        bool networkRecoveryShowingFailure_ = false;
        std::uint32_t selectedFrameRate_ = kDefaultScreenFrameRate;
        std::uint32_t dragPointerSampleRateHz_ = 240;
        std::uint32_t remoteMaximumFrameRate_ = 120;
        std::uint32_t reportedRemoteMaximumFrameRate_ = 120;
        std::uint32_t roomMaximumFrameRate_ = 120;
        std::uint32_t remoteSourceWidth_ = 0;
        std::uint32_t remoteSourceHeight_ = 0;
        ScreenQualityTier selectedQuality_ = ScreenQualityTier::kOriginal;
        bool toolbarShown_ = true;
        bool toolbarLocked_ = false;
        bool fullScreenMode_ = false;
        bool mediaDeviceMenuOpen_ = false;
        QString pendingCameraDeviceId_;
        QString pendingMicrophoneDeviceId_;
        QString pendingSpeakerDeviceId_;
        std::function<void()> disconnectHandler_;
        std::function<bool(const QStringList&, bool)> remotePasteHandler_;
        std::function<void()> remotePasteCancelHandler_;
        std::function<void()> fileTransferHandler_;
        std::function<void()> releaseFileTransferHostHandler_;
    };

}  // namespace remote::controller
