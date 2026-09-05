// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

FileTransferController::FileTransferController(
    SendCallback send,
    std::unique_ptr<WindowsFileTransferService> fileService)
    : send_(std::move(send)),
      storage_(std::make_unique<FileTransferStorage>(
          std::move(fileService))),
      worker_([this](std::stop_token stopToken) {
          WorkerMain(stopToken);
      })
{}

FileTransferController::~FileTransferController()
{
    {
        std::lock_guard lock(mutex_);
        stopping_ = true;
        observer_ = nullptr;
        workQueue_.clear();
        scheduledTasks_.clear();
        ++scheduledTasksRevision_;
    }
    worker_.request_stop();
    workAvailable_.notify_all();
    if (worker_.joinable()) {
        worker_.join();
    }
    for (const auto& [id, transfer] : transfers_) {
        if (transfer->snapshot.direction == FileTransferDirection::kIncoming) {
            storage_->RemovePartial(
                transfer->destination.temporaryPath);
        }
    }
}

void FileTransferController::SetObserver(
    IFileTransferControllerObserver* observer)
{
    {
        std::lock_guard lock(mutex_);
        observer_ = observer;
    }
    PublishSnapshots();
}

std::vector<FileTransferSnapshot> FileTransferController::Transfers() const
{
    std::vector<FileTransferSnapshot> snapshots;
    std::lock_guard lock(mutex_);
    snapshots.reserve(transfers_.size());
    for (const auto& [id, transfer] : transfers_) {
        snapshots.push_back(transfer->snapshot);
    }
    std::sort(snapshots.begin(), snapshots.end(),
              [](const auto& left, const auto& right) {
                  return left.displayOrder > right.displayOrder;
              });
    return snapshots;
}

void FileTransferController::UpdatePeerConnectivity(
    const std::vector<std::string>& activePeerDeviceIds,
    const std::vector<std::string>& recoveringPeerDeviceIds)
{
    std::unordered_set<std::string> available(activePeerDeviceIds.begin(),
                                              activePeerDeviceIds.end());
    std::unordered_set<std::string> recovering(
        recoveringPeerDeviceIds.begin(), recoveringPeerDeviceIds.end());
    std::vector<std::filesystem::path> sources;
    std::vector<std::filesystem::path> partialsToClose;
    std::vector<std::filesystem::path> partialsToRemove;
    std::vector<std::pair<std::string, std::string>> resumes;
    bool changed = false;
    {
        std::lock_guard lock(mutex_);
        if (availablePeers_ == available && recoveringPeers_ == recovering) {
            return;
        }
        availablePeers_ = available;
        recoveringPeers_ = recovering;
        for (auto& [id, record] : transfers_) {
            if (IsTerminal(record->snapshot.state)) {
                continue;
            }
            const auto& peer = record->snapshot.peerDeviceId;
            if (available.contains(peer)) {
                if (record->snapshot.state ==
                        FileTransferState::kRecovering &&
                    record->interruptedByNetwork &&
                    record->snapshot.direction ==
                        FileTransferDirection::kOutgoing) {
                    record->snapshot.state = FileTransferState::kResuming;
                    record->snapshot.errorCode.clear();
                    record->snapshot.errorMessage.clear();
                    record->interruptedByNetwork = false;
                    record->lastProgressAt =
                        std::chrono::steady_clock::now();
                    resumes.emplace_back(id, peer);
                    changed = true;
                }
                continue;
            }
            if (recovering.contains(peer)) {
                const auto state = record->snapshot.state;
                if (state == FileTransferState::kTransferring ||
                    state == FileTransferState::kResuming ||
                    state == FileTransferState::kAwaitingCompletion) {
                    record->snapshot.state =
                        FileTransferState::kRecovering;
                    record->snapshot.bytesPerSecond = 0.0;
                    record->snapshot.estimatedRemainingSeconds = 0;
                    record->snapshot.errorCode =
                        "file_transfer_network_recovering";
                    record->snapshot.errorMessage =
                        "The P2P connection is temporarily interrupted; "
                        "RemoteC is waiting for it to recover.";
                    record->interruptedByNetwork = true;
                    sources.push_back(record->sourcePath);
                    partialsToClose.push_back(
                        record->destination.temporaryPath);
                    changed = true;
                }
                continue;
            }
            record->snapshot.state = FileTransferState::kFailed;
            record->snapshot.bytesPerSecond = 0.0;
            record->snapshot.estimatedRemainingSeconds = 0;
            record->snapshot.errorCode = "file_transfer_peer_unavailable";
            record->snapshot.errorMessage =
                "The peer P2P connection is no longer available.";
            sources.push_back(record->sourcePath);
            partialsToRemove.push_back(record->destination.temporaryPath);
            changed = true;
        }
    }
    if (!sources.empty() || !partialsToClose.empty() ||
        !partialsToRemove.empty()) {
        PostPriority([this, sources = std::move(sources),
                      partialsToClose = std::move(partialsToClose),
                      partialsToRemove = std::move(partialsToRemove)] {
            for (const auto& source : sources) {
                storage_->CloseSource(source);
            }
            for (const auto& partial : partialsToClose) {
                storage_->CloseDestination(partial);
            }
            for (const auto& partial : partialsToRemove) {
                storage_->RemovePartial(partial);
            }
        });
    }
    for (const auto& [id, peer] : resumes) {
        Post([this, id, peer] {
            FileTransferMessage resume;
            resume.type = FileTransferMessageType::kResume;
            resume.transferId = id;
            const auto result = SendWithBackpressureRetry(peer, resume);
            if (!result.accepted) {
                {
                    std::lock_guard lock(mutex_);
                    const auto found = transfers_.find(id);
                    if (found != transfers_.end() &&
                        found->second->snapshot.state ==
                            FileTransferState::kResuming) {
                        found->second->snapshot.errorCode = result.errorCode;
                        found->second->snapshot.errorMessage =
                            "The P2P connection recovered, but file transfer "
                            "resynchronization is still pending.";
                        found->second->interruptedByNetwork = true;
                        found->second->lastProgressAt =
                            std::chrono::steady_clock::now();
                    }
                }
                PublishSnapshots();
            }
        });
    }
    if (changed) {
        PublishSnapshots();
    }
}
