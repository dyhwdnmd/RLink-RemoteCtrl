// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#pragma once

#include <chrono>
#include <condition_variable>
#include <cstdint>
#include <deque>
#include <filesystem>
#include <functional>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "src/core/ISessionEngine.h"
#include "src/apps/remote/FileTransferStorage.h"

namespace remote::app {

enum class FileTransferDirection {
    kOutgoing,
    kIncoming,
};

enum class FileTransferState {
    kPreparing,
    kAwaitingPeerAcceptance,
    kAwaitingLocalAcceptance,
    kTransferring,
    kVerifying,
    kAwaitingCompletion,
    kCompleted,
    kRejected,
    kRecovering,
    kPaused,
    kResuming,
    kCanceled,
    kFailed,
};

struct FileTransferSnapshot {
    std::string transferId;
    std::uint64_t displayOrder = 0;
    std::string peerDeviceId;
    std::string fileName;
    std::uint64_t fileSize = 0;
    std::uint64_t transferredBytes = 0;
    double bytesPerSecond = 0.0;
    std::uint64_t estimatedRemainingSeconds = 0;
    FileTransferDirection direction = FileTransferDirection::kOutgoing;
    FileTransferState state = FileTransferState::kPreparing;
    std::filesystem::path localPath;
    std::string errorCode;
    std::string errorMessage;
};

struct FileTransferCommandResult {
    bool accepted = false;
    std::string transferId;
    std::string errorCode;
    std::string errorMessage;
};

class IFileTransferControllerObserver {
public:
    virtual ~IFileTransferControllerObserver() = default;
    virtual void OnFileTransfersChanged(
        const std::vector<FileTransferSnapshot>& transfers) = 0;
};

// Owns the file-transfer state machine and a single blocking file worker.
// The callback must route messages through the room member-pair engine.
class FileTransferController final : public IFileTransferSink {
public:
    using SendCallback = std::function<SessionCommandResult(
        const std::string& peerDeviceId,
        const FileTransferMessage& message)>;

    explicit FileTransferController(
        SendCallback send,
        std::unique_ptr<WindowsFileTransferService> fileService =
            std::make_unique<WindowsFileTransferService>());
    ~FileTransferController() override;

    FileTransferController(const FileTransferController&) = delete;
    FileTransferController& operator=(const FileTransferController&) = delete;

    void SetObserver(IFileTransferControllerObserver* observer);
    std::vector<FileTransferSnapshot> Transfers() const;
    void UpdatePeerConnectivity(
        const std::vector<std::string>& activePeerDeviceIds,
        const std::vector<std::string>& recoveringPeerDeviceIds);

    FileTransferCommandResult SendFile(
        const std::string& peerDeviceId,
        const std::filesystem::path& sourcePath);
    FileTransferCommandResult AcceptIncoming(
        const std::string& transferId,
        const std::filesystem::path& destinationDirectory,
        bool replaceExisting = false);
    FileTransferCommandResult RejectIncoming(
        const std::string& transferId);
    FileTransferCommandResult Cancel(const std::string& transferId);
    FileTransferCommandResult Resume(const std::string& transferId);

    void OnFileTransferMessage(
        const FileTransferEnvelope& envelope) override;

private:
    struct TransferRecord;
    struct ScheduledTask {
        std::chrono::steady_clock::time_point due;
        std::uint64_t order = 0;
        std::function<void()> task;
    };

    void WorkerMain(std::stop_token stopToken);
    bool Post(std::function<void()> task);
    bool PostPriority(std::function<void()> task);
    bool PostAfter(std::chrono::milliseconds delay,
                   std::function<void()> task);
    void PrepareOutgoing(const std::string& transferId);
    void PrepareIncoming(const std::string& transferId);
    void ProcessEnvelope(FileTransferEnvelope envelope);
    void SendAvailableChunks(const std::string& transferId);
    void UpdateProgressLocked(TransferRecord& transfer,
                              std::uint64_t transferredBytes);
    void ResetRateLocked(TransferRecord& transfer);
    bool RefreshRateLocked(
        TransferRecord& transfer,
        std::chrono::steady_clock::time_point now);
    void CheckStalledTransfers();
    SessionCommandResult SendOnce(
        const std::string& peerDeviceId,
        const FileTransferMessage& message);
    SessionCommandResult SendWithBackpressureRetry(
        const std::string& peerDeviceId,
        const FileTransferMessage& message);
    void FailTransfer(const std::string& transferId,
                      std::string errorCode,
                      std::string errorMessage,
                      bool notifyPeer);
    void SendProtocolError(const std::string& peerDeviceId,
                           const std::string& transferId,
                           const std::string& errorCode,
                           const std::string& errorMessage);
    void PublishSnapshots();
    std::string GenerateTransferId();

    SendCallback send_;
    std::unique_ptr<FileTransferStorage> storage_;
    mutable std::mutex mutex_;
    std::condition_variable_any workAvailable_;
    std::deque<std::function<void()>> workQueue_;
    std::vector<ScheduledTask> scheduledTasks_;
    std::unordered_map<std::string, std::unique_ptr<TransferRecord>> transfers_;
    std::unordered_set<std::string> availablePeers_;
    std::unordered_set<std::string> recoveringPeers_;
    IFileTransferControllerObserver* observer_ = nullptr;
    std::uint64_t nextDisplayOrder_ = 0;
    std::uint64_t nextScheduledTaskOrder_ = 0;
    std::uint64_t scheduledTasksRevision_ = 0;
    bool stopping_ = false;
    std::jthread worker_;
};

}  // namespace remote::app
