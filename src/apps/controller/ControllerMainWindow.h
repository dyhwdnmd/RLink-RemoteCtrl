// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#pragma once

#include <atomic>
#include <functional>
#include <memory>
#include <optional>
#include <thread>

#include <QHash>
#include <QElapsedTimer>
#include <QPointer>
#include <QPoint>
#include <QSet>
#include <QVector>

#include "FramelessWindow.h"
#include "RemoteSessionBinding.h"
#include "src/apps/remote/ClipboardController.h"
#include "src/apps/update/SoftwareUpdateController.h"
#include "src/core/ISessionEngine.h"

namespace remote {
class WindowsInputExecutor;
namespace app {
class FileTransferController;
}
}

class QComboBox;
class QAction;
class QCloseEvent;
class QEvent;
class QFrame;
class QGraphicsOpacityEffect;
class QLabel;
class QLineEdit;
class QListWidget;
class QParallelAnimationGroup;
class QPropertyAnimation;
class QProcess;
class QPoint;
class QPushButton;
class QResizeEvent;
class QStackedWidget;
class QSystemTrayIcon;
class QTimer;
class QToolButton;
class QVBoxLayout;

namespace remote::controller {

class CameraWindow;
class FileTransferWindow;
class RemoteSessionWindow;
class RoomCameraWindow;

class ControllerMainWindow final : public FramelessMainWindow,
                                   private ISessionEngineObserver,
                                   private app::IClipboardControllerObserver {
public:
    explicit ControllerMainWindow(std::unique_ptr<ISessionEngine> engine,
                                  bool startEngineImmediately = true,
                                  QWidget* parent = nullptr);
    ~ControllerMainWindow() override;

    void ActivateFromExternalLaunch();
    void SetAccountInteractionCallback(
        std::function<void()> callback);
    void SetAccountSwitchCallback(std::function<void()> callback);
    void SetAccountDeletionCallback(std::function<void()> callback);
    void SetAccountSignedOut(
        const QString& message = {});
    void SetAccountBusy(const QString& message);
    void SetAccountSession(
        const QString& accountId,
        const QString& accountLabel,
        const QString& accountDetail,
        std::function<void()> signOutCallback);
    bool StartSessionEngine();
    void StopSessionEngine();
    // Executes the protocol-aware shutdown while Qt can still flush WSS
    // messages. Safe to call repeatedly from every application-exit path.
    void PrepareForApplicationExit();
    bool RunThemeRoundTripSelfTest(QString* errorMessage = nullptr);

    ControllerMainWindow(const ControllerMainWindow&) = delete;
    ControllerMainWindow& operator=(const ControllerMainWindow&) = delete;

private:
    void OnSessionEngineSnapshot(
        const SessionEngineSnapshot& snapshot) override;
    void OnClipboardStateChanged(
        const app::ClipboardControllerSnapshot& snapshot) override;
    void BuildUi();
    void BuildSystemTray();
    void ShowFromSystemTray();
    void QuitFromSystemTray();
    void DestroyAuxiliaryWindowsForExit();
    void CheckForSoftwareUpdates(bool manualRequest);
    void HandleSoftwareUpdateState(
        const update::SoftwareUpdateController::Snapshot& snapshot);
    void OpenSoftwareUpdate();
    void QuitForSoftwareUpdate();
    bool InitializeEngine();
    void CompleteEngineInitialization(
        const SessionCommandResult& startResult);
    void ApplyAuthenticationAvailability(bool authenticated);
    void StartSession(const QString& deviceId,
                      const QString& deviceName,
                      SessionPurpose purpose);
    void StartOwnedDeviceSession(const QString& deviceId,
                                 const QString& deviceName);
    void StartAssistedSession(const QString& deviceId,
                              const QString& verificationCode);
    void OpenCameraWindow(const QString& deviceId, const QString& deviceName);
    void OpenRemoteSession(RemoteSessionBinding binding);
    void RefreshOwnedDevicesUi(
        const SessionEngineSnapshot& snapshot);
    void UpdateRoomUi(const SessionEngineSnapshot& snapshot);
    void QueueRoomJoinApproval(const SessionEngineSnapshot& snapshot);
    void QueueRoomScreenShareSwitchApproval(
        const SessionEngineSnapshot& snapshot);
    void QueueRoomControlApproval(const SessionEngineSnapshot& snapshot);
    void QueueRoomScreenShareViewApproval(
        const SessionEngineSnapshot& snapshot);
    void HandleRoomMemberActionResults(
        const SessionEngineSnapshot& snapshot);
    void ShowRoomMemberContextMenu(const QPoint& position);
    void HandleRemoteSessionDisconnect();
    void SetRoomActionHint(const QString& text, bool error = false);
    void SetRuntimeStatus(const QString& status, const QString& color);
    void SelectMainPage(int pageIndex,
                        QPushButton* navigationButton,
                        const QString& title);
    void AnimateNavigationIndicator(QPushButton* navigationButton);
    void QueueRoomWorkspaceActive(bool active);
    void SetRoomWorkspaceActive(bool active);
    void SetAnimationLevel(int level);
    void ToggleAccountMenu();
    void ShowAccountMenu();
    void HideAccountMenu(bool animated = true);
    void UpdateAccountMenuGeometry();
    void StartAccountMenuMotion(const QPoint& targetPosition,
                                int durationMs, bool hideWhenFinished);
    void StopAccountMenuMotion();
    void UpdateAccountMenuHoverFromCursor();
    void ApplyInterfaceTheme(bool showFeedback = false);
    void SetInterfaceThemePreference(const QString& value);
    void RememberRecentRoom(const SessionEngineSnapshot& snapshot);
    void RememberRecentDevice(const SessionEngineSnapshot& snapshot);
    void RefreshRecentRooms();
    void RefreshRecentDevices();
    QString RecentSettingsKey(const QString& listName) const;
    void MigrateLegacyRecentHistory();
    void RequestRecentRoomAvailability();
    void RefreshDiagnosticsUi();
    void StartDecoderBenchmark(bool manualRequest);
    void FinishDecoderBenchmark(int exitCode);
    void RefreshDecoderBenchmarkSummary();
    void RefreshDecoderHardwareSelectionAvailability();
    void StartEncoderBenchmark(bool manualRequest);
    void FinishEncoderBenchmark(int exitCode);
    void RefreshEncoderBenchmarkSummary();
    void UpdateLocalMediaDevicesUi(
        const SessionEngineSnapshot& snapshot);
    void PersistHardwareCapabilityCache();
    void RequestMediaDeviceRefresh(bool userInitiated = false);
    void ApplyVideoPipelineSettingsFromUi(
        bool showFeedback = true,
        const QString& changedSettingName = {});
    void UpdateVideoPipelineSettingsAvailability(
        const SessionEngineSnapshot& snapshot);
    void ShowMediaDeviceMenu(
        MediaDeviceKind kind, QWidget* anchor);
    void BeginMediaDeviceSelection(
        MediaDeviceKind kind, const QString& deviceId);
    void ApplyClipboardConfigurationFromUi(bool showFeedback = false);
    void UpdateClipboardSession(const SessionEngineSnapshot& snapshot);
    void UpdateDirectFileTransferSession(
        const SessionEngineSnapshot& snapshot);

protected:
    bool eventFilter(QObject* watched, QEvent* event) override;
    bool nativeEvent(const QByteArray& eventType,
                     void* message,
                     qintptr* result) override;
    void changeEvent(QEvent* event) override;
    void closeEvent(QCloseEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;

private:

    std::unique_ptr<WindowsInputExecutor> inputExecutor_;
    std::unique_ptr<app::FileTransferController> fileTransferController_;
    std::unique_ptr<app::ClipboardController> clipboardController_;
    std::unique_ptr<update::SoftwareUpdateController>
        softwareUpdateController_;
    bool softwareUpdatePromptOpen_ = false;
    std::unique_ptr<ISessionEngine> engine_;
    std::jthread engineStartThread_;
    QLineEdit* deviceIdEdit_ = nullptr;
    QLineEdit* verificationCodeEdit_ = nullptr;
    QPushButton* connectButton_ = nullptr;
    QTimer* assistedSessionTimeoutTimer_ = nullptr;
    bool assistedSessionPending_ = false;
    bool assistedSessionActive_ = false;
    bool assistedSessionCancellationPending_ = false;
    bool assistedSessionTimedOut_ = false;
    bool ownedDeviceSessionPending_ = false;
    QString lastDirectSessionToastError_;
    QLabel* runtimeStatus_ = nullptr;
    QLabel* decoderStatus_ = nullptr;
    QLabel* connectivityPill_ = nullptr;
    QFrame* profileCard_ = nullptr;
    QLabel* profileAvatar_ = nullptr;
    QLabel* profileName_ = nullptr;
    QLabel* serviceStatus_ = nullptr;
    QToolButton* profileUpdateButton_ = nullptr;
    QFrame* accountMenu_ = nullptr;
    QLabel* accountMenuAvatar_ = nullptr;
    QLabel* accountMenuName_ = nullptr;
    QLabel* accountMenuDetail_ = nullptr;
    QPushButton* softwareUpdateAction_ = nullptr;
    QLabel* softwareUpdateStatusLabel_ = nullptr;
    QPushButton* softwareUpdateCheckButton_ = nullptr;
    QGraphicsOpacityEffect* accountMenuOpacity_ = nullptr;
    QTimer* accountMenuMotionTimer_ = nullptr;
    QElapsedTimer accountMenuMotionClock_;
    QPoint accountMenuMotionStart_;
    QPoint accountMenuMotionTarget_;
    int accountMenuMotionDurationMs_ = 0;
    bool accountMenuMotionHiding_ = false;
    QTimer* accountMenuHoverTimer_ = nullptr;
    QFrame* deviceLoginPrompt_ = nullptr;
    QLabel* deviceLoginStatus_ = nullptr;
    QPushButton* deviceLoginButton_ = nullptr;
    QFrame* deviceIdentityCard_ = nullptr;
    QFrame* deviceDirectAssistCard_ = nullptr;
    QFrame* deviceStatusCard_ = nullptr;
    QLabel* deviceAccountLabel_ = nullptr;
    QLabel* localDeviceId_ = nullptr;
    QPushButton* copyDeviceIdButton_ = nullptr;
    QLabel* localVerificationCode_ = nullptr;
    QPushButton* copyVerificationCodeButton_ = nullptr;
    QPushButton* shareLocalCredentialsButton_ = nullptr;
    QLabel* connectionHint_ = nullptr;
    QComboBox* createRoomCapacity_ = nullptr;
    QPushButton* createRoomButton_ = nullptr;
    QLineEdit* roomIdEdit_ = nullptr;
    QPushButton* joinRoomButton_ = nullptr;
    QLabel* roomActionHint_ = nullptr;
    QFrame* roomEntryPanel_ = nullptr;
    QFrame* roomPanel_ = nullptr;
    QStackedWidget* roomWorkspaceStack_ = nullptr;
    QParallelAnimationGroup* roomWorkspaceAnimation_ = nullptr;
    QWidget* roomWorkspaceTransitionLayer_ = nullptr;
    bool roomWorkspaceTargetActive_ = false;
    bool roomWorkspaceTransitionPending_ = false;
    quint64 roomWorkspaceTransitionRequest_ = 0;
    QLabel* roomIdLabel_ = nullptr;
    QPushButton* copyRoomIdButton_ = nullptr;
    QLabel* roomOccupancyLabel_ = nullptr;
    QLabel* roomOwnerLabel_ = nullptr;
    QLabel* roomScreenSharerLabel_ = nullptr;
    QLabel* roomControllerLabel_ = nullptr;
    QLabel* roomPeerConnectivityLabel_ = nullptr;
    QLabel* roomSeatUsageLabel_ = nullptr;
    QLabel* roomMemberSummaryLabel_ = nullptr;
    QLabel* roomMemberFooterLabel_ = nullptr;
    QListWidget* roomMemberList_ = nullptr;
    QString renderedRoomMemberKey_;
    QComboBox* activeRoomCapacity_ = nullptr;
    QPushButton* applyRoomCapacityButton_ = nullptr;
    QPushButton* roomScreenShareButton_ = nullptr;
    QPushButton* roomCameraButton_ = nullptr;
    QPushButton* roomMicrophoneButton_ = nullptr;
    QPushButton* roomSpeakerButton_ = nullptr;
    QPushButton* roomCameraGalleryButton_ = nullptr;
    QPushButton* roomFileTransferButton_ = nullptr;
    QPushButton* fileTransferNavButton_ = nullptr;
    QPushButton* leaveRoomButton_ = nullptr;
    QLabel* roomStageLabel_ = nullptr;
    CustomTitleBar* titleBar_ = nullptr;
    QStackedWidget* pageStack_ = nullptr;
    QPushButton* roomNavButton_ = nullptr;
    QPushButton* deviceNavButton_ = nullptr;
    QPushButton* myDevicesNavButton_ = nullptr;
    QPushButton* recentNavButton_ = nullptr;
    QPushButton* debugNavButton_ = nullptr;
    QPushButton* settingsNavButton_ = nullptr;
    QPushButton* helpNavButton_ = nullptr;
    QPushButton* authorNavButton_ = nullptr;
    QVector<QPushButton*> pageNavigationButtons_;
    QFrame* navigationIndicator_ = nullptr;
    QPropertyAnimation* navigationAnimation_ = nullptr;
    QSystemTrayIcon* trayIcon_ = nullptr;
    QLabel* trayIdentityLabel_ = nullptr;
    QAction* traySignOutAction_ = nullptr;
    std::function<void()> accountInteractionCallback_;
    std::function<void()> accountSwitchCallback_;
    std::function<void()> accountDeletionCallback_;
    std::function<void()> signOutCallback_;
    bool sessionEngineStarted_ = false;
    bool applicationExitPrepared_ = false;
    bool authenticationAvailable_ = true;
    QComboBox* animationLevelSelector_ = nullptr;
    QComboBox* themeModeSelector_ = nullptr;
    QComboBox* fontFamilySelector_ = nullptr;
    QComboBox* fontSizeSelector_ = nullptr;
    QComboBox* autoStartSelector_ = nullptr;
    QComboBox* startupVisibilitySelector_ = nullptr;
    QComboBox* closeButtonBehaviorSelector_ = nullptr;
    QComboBox* defaultRoomCapacitySelector_ = nullptr;
    QComboBox* desktopCaptureSelector_ = nullptr;
    QComboBox* videoEncoderSelector_ = nullptr;
    QComboBox* ffmpegHardwareBackendSelector_ = nullptr;
    QComboBox* ffmpegX264PresetSelector_ = nullptr;
    QComboBox* videoDecoderSelector_ = nullptr;
    QComboBox* videoRendererSelector_ = nullptr;
    bool videoPipelineSettingsBusy_ = false;
    bool videoPipelineSettingsApplyPending_ = false;
    QComboBox* dragPointerSampleRateSelector_ = nullptr;
    QComboBox* remotePasteEnabledSelector_ = nullptr;
    QComboBox* clipboardFormatsSelector_ = nullptr;
    QComboBox* clipboardLargeFileLimitSelector_ = nullptr;
    QComboBox* clipboardCacheRetentionSelector_ = nullptr;
    QComboBox* clipboardCacheCapacitySelector_ = nullptr;
    QLabel* clipboardCachePathLabel_ = nullptr;
    QLabel* clipboardCacheUsageLabel_ = nullptr;
    QPushButton* clearClipboardCacheButton_ = nullptr;
    QLabel* decoderBenchmarkSummary_ = nullptr;
    QPushButton* decoderBenchmarkButton_ = nullptr;
    QProcess* decoderBenchmarkProcess_ = nullptr;
    bool decoderBenchmarkManualRequest_ = false;
    QString decoderBenchmarkHardwareFingerprint_;
    QLabel* encoderBenchmarkSummary_ = nullptr;
    QPushButton* encoderBenchmarkButton_ = nullptr;
    QProcess* encoderBenchmarkProcess_ = nullptr;
    bool encoderBenchmarkManualRequest_ = false;
    QString encoderBenchmarkHardwareFingerprint_;
    QString encoderBenchmarkCaptureBackend_;
    QString encoderBenchmarkX264Preset_;
    QComboBox* cameraGalleryBehaviorSelector_ = nullptr;
    QComboBox* cameraDeviceSelector_ = nullptr;
    QComboBox* microphoneDeviceSelector_ = nullptr;
    QComboBox* speakerDeviceSelector_ = nullptr;
    QLabel* mediaDeviceStatusLabel_ = nullptr;
    QPushButton* refreshMediaDevicesButton_ = nullptr;
    QTimer* mediaDeviceRefreshDebounceTimer_ = nullptr;
    QLabel* devicePageSignalStatus_ = nullptr;
    QVBoxLayout* recentRoomsLayout_ = nullptr;
    QVBoxLayout* recentDevicesLayout_ = nullptr;
    QVBoxLayout* ownedDevicesCardsLayout_ = nullptr;
    QLabel* ownedDevicesSummaryLabel_ = nullptr;
    QFrame* ownedDevicesEmptyState_ = nullptr;
    QLabel* ownedDevicesEmptyArtwork_ = nullptr;
    QPushButton* ownedDevicesRefreshButton_ = nullptr;
    QWidget* localDevicePage_ = nullptr;
    QWidget* ownedDevicesPage_ = nullptr;
    QWidget* recentConnectionsPage_ = nullptr;
    QWidget* debugPage_ = nullptr;
    bool darkInterfaceTheme_ = false;
    quint64 renderedOwnedDevicesRevision_ = 0;
    SessionConnectivityState renderedOwnedDevicesConnectivity_ =
        SessionConnectivityState::kNotConfigured;
    QFrame* recentEmptyState_ = nullptr;
    QFrame* recentDevicesEmptyState_ = nullptr;
    QHash<QString, QLabel*> debugValueLabels_;
    QWidget* statsCardsWidget_ = nullptr;
    QPushButton* screenFrameRateLogButton_ = nullptr;
    bool screenFrameRateLogEnabled_ = false;
    QPushButton* inputEventStatsButton_ = nullptr;
    bool inputEventStatsEnabled_ = false;
    QPushButton* copyDebugButton_ = nullptr;
    QPushButton* copyMediaDebugButton_ = nullptr;
    QString debugCopyText_;
    QString mediaDebugCopyText_;
    QString statsDebugCopyText_;
    bool diagnosticsCopyTextRequested_ = false;
    QTimer* diagnosticsRefreshTimer_ = nullptr;
    QString lastRememberedRoomId_;
    QString lastRememberedDirectSessionId_;
    QString recentHistoryAccountKey_;
    bool recentRoomAvailabilityRequested_ = false;
    bool quitting_ = false;
    QString pendingDeviceName_;
    QString promptedSessionId_;
    QString approvalRoomId_;
    QSet<QString> promptedRoomJoinRequestIds_;
    QSet<QString> promptedRoomScreenShareSwitchRequestIds_;
    QSet<QString> promptedRoomControlRequestIds_;
    QSet<QString> promptedRoomScreenShareViewRequestIds_;
    QSet<QString> handledRoomMemberActionResultKeys_;
    bool roomJoinApprovalPromptPending_ = false;
    bool roomScreenShareSwitchApprovalPromptPending_ = false;
    bool roomControlApprovalPromptPending_ = false;
    bool roomScreenShareViewApprovalPromptPending_ = false;
    QString lastRoomDecisionAlertKey_;
    QPointer<CameraWindow> cameraWindow_;
    QPointer<FileTransferWindow> fileTransferWindow_;
    QPointer<RemoteSessionWindow> remoteSessionWindow_;
    QPointer<RoomCameraWindow> roomCameraWindow_;
    std::optional<RemoteSessionBinding> remoteSessionBinding_;
    quint64 preflightScreenPreferenceEpoch_ = 0;
    quint64 preflightScreenPreferenceAttemptEpoch_ = 0;
    int preflightScreenPreferenceAttempts_ = 0;
    bool preflightScreenPreferenceScheduled_ = false;
    QString dismissedRemoteScreenSharerDeviceId_;
    quint64 dismissedRemoteScreenShareEpoch_ = 0;
    QString dismissedDirectSessionId_;
    bool localCameraStopRequested_ = false;
    bool cameraGalleryManuallyHidden_ = false;
    quint64 mediaDeviceRevision_ = 0;
    bool mediaDeviceSnapshotSeen_ = false;
    bool mediaActivitySnapshotSeen_ = false;
    LocalMicrophoneState displayedMicrophoneState_ =
        LocalMicrophoneState::kOff;
    bool displayedRoomAudioPlaybackMuted_ = false;
    bool mediaDeviceRefreshUserRequested_ = false;
    QString pendingCameraDeviceId_;
    QString pendingMicrophoneDeviceId_;
    QString pendingSpeakerDeviceId_;
    QString lastMediaDeviceErrorKey_;
    std::uint64_t displayedClipboardSentItems_ = 0;
    std::uint64_t displayedClipboardReceivedItems_ = 0;
    std::uint64_t displayedClipboardRejectedItems_ = 0;
    QString displayedClipboardErrorCode_;
    QString pendingRemotePasteDialogId_;
    QString visibleRemotePasteDialogId_;
    QString promptedClipboardConflictId_;
    std::atomic_bool clipboardUiUpdatePending_{false};
    bool clipboardAllowedForCurrentControl_ = false;
};

}  // namespace remote::controller
