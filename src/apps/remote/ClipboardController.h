// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#pragma once

#include <chrono>
#include <atomic>
#include <condition_variable>
#include <cstdint>
#include <deque>
#include <filesystem>
#include <functional>
#include <memory>
#include <mutex>
#include <optional>
#include <string>
#include <thread>
#include <vector>

#include "src/core/ISessionEngine.h"
#include "src/apps/remote/ClipboardCacheManager.h"
#include "src/platform/win/WindowsClipboardService.h"
#include "src/protocol/ClipboardProtocol.h"

namespace remote::app {

struct RemotePasteConfiguration {
    bool enabled = true;
    bool unicodeText = true;
    bool html = true;
    bool rtf = true;
    bool png = true;
    bool files = true;
    std::uint64_t automaticFileLimitBytes = 100ull * 1024 * 1024;
    std::filesystem::path cacheBaseDirectory;
    std::chrono::minutes cacheRetention{60};
    std::uint64_t cacheLimitBytes = 2ull * 1024 * 1024 * 1024;
};

struct ClipboardSessionContext {
    bool active = false;
    bool transportReady = false;
    bool localIsController = false;
    std::string roomId;
    std::string localDeviceId;
    std::string peerDeviceId;
};

struct ClipboardControllerSnapshot {
    bool enabled = false;
    bool sessionActive = false;
    bool peerCapabilitiesSeen = false;
    bool peerEnabled = false;
    std::string peerDeviceId;
    std::string state = "disabled";
    std::string lastFormat;
    std::uint64_t lastBytes = 0;
    std::uint64_t sentItems = 0;
    std::uint64_t receivedItems = 0;
    std::uint64_t suppressedEchoes = 0;
    std::uint64_t rejectedItems = 0;
    bool transferActive = false;
    bool transferOutgoing = false;
    bool transferContainsFiles = false;
    std::string transferId;
    std::uint32_t transferItemCount = 0;
    std::uint64_t transferTotalBytes = 0;
    std::uint64_t transferCompletedBytes = 0;
    double transferBytesPerSecond = 0.0;
    std::uint64_t transferEstimatedRemainingSeconds = 0;
    // Local native window captured by the physical Ctrl+V hook. It is only
    // populated for a controlled-to-controller lazy paste and is used to
    // position the progress prompt without taking focus from the target.
    std::uintptr_t localPasteTargetWindow = 0;
    std::string localPasteDestinationPath;
    bool explorerConflictPending = false;
    std::vector<std::string> explorerConflictNames;
    std::string cacheRootPath;
    std::uint64_t cacheBytes = 0;
    std::uint64_t cacheEntryCount = 0;
    std::uint64_t cacheAvailableBytes = 0;
    std::uint64_t cacheSafeCapacityBytes = 0;
    std::uint64_t cacheEffectiveLimitBytes = 0;
    std::string lastErrorCode;
    std::string lastErrorMessage;
};

class IClipboardControllerObserver {
public:
    virtual ~IClipboardControllerObserver() = default;
    virtual void OnClipboardStateChanged(
        const ClipboardControllerSnapshot& snapshot) = 0;
};

class ClipboardController final : public IClipboardSink {
public:
    using SendCallback = std::function<SessionCommandResult(
        const std::string& peerDeviceId,
        const std::string& clipboardSessionId,
        const ClipboardMessage& message)>;
    using PasteCallback = std::function<SessionCommandResult()>;

    explicit ClipboardController(
        SendCallback send,
        PasteCallback paste,
        std::unique_ptr<WindowsClipboardService> service =
            std::make_unique<WindowsClipboardService>());
    ~ClipboardController() override;

    ClipboardController(const ClipboardController&) = delete;
    ClipboardController& operator=(const ClipboardController&) = delete;

    void SetObserver(IClipboardControllerObserver* observer);
    void SetConfiguration(RemotePasteConfiguration configuration);
    void UpdateSession(ClipboardSessionContext context);
    bool RequestPasteFromClipboard();
    bool RequestPasteFiles(std::vector<std::filesystem::path> paths);
    bool ShouldPassThroughRemotePaste() const;
    void SetRemotePastePassthroughWindow(std::uintptr_t windowHandle);
    bool CancelActiveTransfer();
    bool ResolveExplorerConflict(bool replaceAndContinue);
    bool RequestCacheCleanup();
    bool RefreshCacheStatistics();
    ClipboardControllerSnapshot Snapshot() const;

    void OnClipboardMessage(const ClipboardEnvelope& envelope) override;

private:
    struct TransferPart;
    struct OutgoingTransfer;
    struct IncomingTransfer;

    void WorkerMain(std::stop_token stopToken);
    bool Post(std::function<void()> task);
    bool PostPriority(std::function<void()> task);
    void PublishSnapshot();
    void HandleLocalClipboard(WindowsClipboardContent content);
    void ProcessDeferredLocalClipboard();
    void ProcessPendingLocalClipboard();
    void SendCapabilities();
    void ScheduleCapabilityRetry();
    void ProcessCapabilityRetry();
    void ProcessPreparationWatchdog();
    void ProcessRemoteRequestWatchdog();
    void ProcessExplorerConflictKeepalive();
    void ScheduleTransferWatchdog();
    void ProcessTransferWatchdog();
    void SendSessionStart();
    bool SendOutgoingOffer();
    bool SendOutgoingCommit();
    bool ResetOutgoingStreamingState(std::string* error);
    bool FinalizeOutgoingPart(TransferPart* part, std::string* error);
    void ProcessEnvelope(ClipboardEnvelope envelope);
    void ProcessOffer(const ClipboardEnvelope& envelope);
    void ProcessChunk(const ClipboardEnvelope& envelope);
    void ProcessCommit(const ClipboardEnvelope& envelope);
    void SendNextChunk();
    void CancelTransfers(const std::string& reasonCode,
                         const std::string& reasonMessage,
                         bool notifyPeer);
    void CleanupIncomingTransferCache();
    void ClearRemoteAnnouncement(bool clearWindowsAdvertisement);
    void ResetTransferSnapshot();
    void UpdateOutgoingProgressSnapshot();
    void UpdateIncomingProgressSnapshot();
    void UpdateClipboardListenerState();
    void AnnounceLocalClipboard(WindowsClipboardContent content);
    bool HandleLocalPasteHotkey(WindowsPasteTarget target);
    void RequestAnnouncedRemoteClipboard(WindowsPasteTarget target);
    bool SendAnnouncedRemoteClipboardRequest();
    bool PasteIntoLocalTarget(WindowsPasteTarget target,
                              std::string* error);
    bool CopyIncomingFilesToExplorerTarget(
        const std::filesystem::path& destination,
        std::string* error);
    bool SendProtocolMessage(const ClipboardMessage& message);
    bool BuildOutgoingTransfer(WindowsClipboardContent content,
                               std::string* error);
    void ScheduleBackpressureRetry();
    bool PrepareIncomingTransfer(const ClipboardEnvelope& envelope,
                                 std::string* error);
    bool FinalizeIncoming(std::string* error);
    void CleanupClipboardCache();
    void UpdateCacheSnapshot();
    std::string GenerateId(const char* prefix);
    std::filesystem::path ClipboardCacheRoot() const;

    SendCallback send_;
    PasteCallback paste_;
    std::unique_ptr<WindowsClipboardService> service_;
    std::unique_ptr<ClipboardCacheManager> cacheManager_;
    mutable std::mutex mutex_;
    std::condition_variable_any workAvailable_;
    std::deque<std::function<void()>> workQueue_;
    IClipboardControllerObserver* observer_ = nullptr;
    RemotePasteConfiguration configuration_;
    ClipboardSessionContext context_;
    ClipboardControllerSnapshot snapshot_;
    ClipboardControllerSnapshot publishedSnapshot_;
    std::unique_ptr<OutgoingTransfer> outgoing_;
    std::unique_ptr<IncomingTransfer> incoming_;
    std::optional<WindowsClipboardContent> cachedLocalClipboard_;
    // A new Ctrl+C must not tear down bytes already being sent. Keep only
    // the newest local item and publish it after the active send completes.
    std::optional<WindowsClipboardContent> deferredLocalClipboard_;
    std::optional<WindowsClipboardContent> pendingLocalClipboard_;
    std::chrono::steady_clock::time_point pendingLocalDue_{};
    std::optional<std::chrono::steady_clock::time_point>
        capabilityRetryDue_;
    std::optional<std::chrono::steady_clock::time_point>
        transferWatchdogDue_;
    std::optional<std::chrono::steady_clock::time_point>
        preparationWatchdogDue_;
    std::optional<std::chrono::steady_clock::time_point>
        remoteRequestWatchdogDue_;
    std::optional<std::chrono::steady_clock::time_point>
        explorerConflictKeepaliveDue_;
    std::string localSessionId_;
    std::string peerSessionId_;
    std::uint32_t peerCapabilities_ = 0;
    std::string lastAppliedMarker_;
    std::uint64_t logicalClock_ = 0;
    std::uint64_t lastAppliedVersion_ = 0;
    std::string lastAppliedOrigin_;
    std::string lastCompletedIncomingItemId_;
    std::uint64_t lastCompletedIncomingVersion_ = 0;
    bool lastCompletedIncomingAccepted_ = false;
    std::string lastCompletedIncomingErrorCode_;
    std::string lastCompletedIncomingErrorMessage_;
    std::string announcedLocalItemId_;
    std::uint64_t announcedLocalVersion_ = 0;
    std::string remoteAnnouncedItemId_;
    std::uint64_t remoteAnnouncedVersion_ = 0;
    std::uint32_t remoteAnnouncedCapabilities_ = 0;
    std::vector<ClipboardFormatDescriptor> remoteAnnouncedDescriptors_;
    WindowsPasteTarget pendingLocalPasteTarget_;
    std::optional<ClipboardEnvelope> pendingExplorerConflictOffer_;
    bool pendingRemoteConflictDecision_ = false;
    bool fulfillingAnnouncedRequest_ = false;
    std::uint64_t nextId_ = 0;
    std::uint32_t captureRetryCount_ = 0;
    bool localCapabilitiesSent_ = false;
    bool lastSendBackpressured_ = false;
    std::atomic_bool pasteRequestPending_{false};
    std::atomic_bool remoteClipboardAvailable_{false};
    std::atomic_bool localPasteHotkeyPending_{false};
    std::atomic<std::uint32_t> lastRemoteClipboardSequence_{0};
    std::atomic<std::uint32_t> remoteAnnouncementLocalSequence_{0};
    std::atomic<std::uintptr_t> remotePastePassthroughWindow_{0};
    std::atomic<std::uint64_t> localClipboardGeneration_{0};
    bool stopping_ = false;
    std::jthread worker_;
};

}  // namespace remote::app
