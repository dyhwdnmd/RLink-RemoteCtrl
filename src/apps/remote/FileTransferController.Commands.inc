// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

FileTransferCommandResult FileTransferController::SendFile(
    const std::string& peerDeviceId,
    const std::filesystem::path& sourcePath)
{
    if (peerDeviceId.empty() || sourcePath.empty()) {
        return Rejected("file_send_invalid",
                        "A peer and source file are required.");
    }
    const std::string transferId = GenerateTransferId();
    auto transfer = std::make_unique<TransferRecord>();
    transfer->snapshot.transferId = transferId;
    transfer->snapshot.peerDeviceId = peerDeviceId;
    transfer->snapshot.direction = FileTransferDirection::kOutgoing;
    transfer->snapshot.state = FileTransferState::kPreparing;
    transfer->snapshot.localPath = sourcePath;
    transfer->sourcePath = sourcePath;
    {
        std::lock_guard lock(mutex_);
        if (stopping_) {
            return Rejected("file_transfer_stopping",
                            "The file transfer controller is stopping.");
        }
        transfer->snapshot.displayOrder = ++nextDisplayOrder_;
        transfers_.emplace(transferId, std::move(transfer));
    }
    if (!Post([this, transferId] { PrepareOutgoing(transferId); })) {
        return Rejected("file_transfer_stopping",
                        "The file transfer controller is stopping.");
    }
    PublishSnapshots();
    return Accepted(transferId);
}

FileTransferCommandResult FileTransferController::AcceptIncoming(
    const std::string& transferId,
    const std::filesystem::path& destinationDirectory,
    bool replaceExisting)
{
    if (destinationDirectory.empty()) {
        return Rejected("file_destination_invalid",
                        "A destination directory is required.");
    }
    {
        std::lock_guard lock(mutex_);
        const auto found = transfers_.find(transferId);
        if (found == transfers_.end() ||
            found->second->snapshot.direction !=
                FileTransferDirection::kIncoming ||
            found->second->snapshot.state !=
                FileTransferState::kAwaitingLocalAcceptance) {
            return Rejected("file_offer_not_pending",
                            "The incoming file offer is not pending.");
        }
        found->second->snapshot.state = FileTransferState::kPreparing;
        found->second->snapshot.localPath = destinationDirectory;
        found->second->replaceExisting = replaceExisting;
    }
    if (!Post([this, transferId] { PrepareIncoming(transferId); })) {
        return Rejected("file_transfer_stopping",
                        "The file transfer controller is stopping.");
    }
    PublishSnapshots();
    return Accepted(transferId);
}

FileTransferCommandResult FileTransferController::RejectIncoming(
    const std::string& transferId)
{
    std::string peer;
    {
        std::lock_guard lock(mutex_);
        const auto found = transfers_.find(transferId);
        if (found == transfers_.end() ||
            found->second->snapshot.direction !=
                FileTransferDirection::kIncoming ||
            found->second->snapshot.state !=
                FileTransferState::kAwaitingLocalAcceptance) {
            return Rejected("file_offer_not_pending",
                            "The incoming file offer is not pending.");
        }
        peer = found->second->snapshot.peerDeviceId;
        found->second->snapshot.state = FileTransferState::kRejected;
        found->second->snapshot.errorCode = "user_rejected";
        found->second->snapshot.errorMessage =
            "The receiver rejected the file offer.";
    }
    Post([this, transferId, peer] {
        FileTransferMessage response;
        response.type = FileTransferMessageType::kResponse;
        response.transferId = transferId;
        response.errorCode = "user_rejected";
        response.errorMessage = "The receiver rejected the file offer.";
        SendWithBackpressureRetry(peer, response);
    });
    PublishSnapshots();
    return Accepted(transferId);
}

FileTransferCommandResult FileTransferController::Cancel(
    const std::string& transferId)
{
    std::string peer;
    std::filesystem::path partial;
    std::filesystem::path source;
    bool resumable = false;
    {
        std::lock_guard lock(mutex_);
        const auto found = transfers_.find(transferId);
        if (found == transfers_.end() ||
            IsTerminal(found->second->snapshot.state)) {
            return Rejected("file_transfer_not_active",
                            "The file transfer is not active.");
        }
        peer = found->second->snapshot.peerDeviceId;
        partial = found->second->destination.temporaryPath;
        source = found->second->sourcePath;
        resumable = found->second->snapshot.state ==
                    FileTransferState::kTransferring;
        found->second->snapshot.state = resumable
            ? FileTransferState::kPaused
            : FileTransferState::kCanceled;
        found->second->snapshot.bytesPerSecond = 0.0;
        found->second->snapshot.estimatedRemainingSeconds = 0;
        found->second->snapshot.errorCode = resumable
            ? "user_paused"
            : "user_canceled";
        found->second->snapshot.errorMessage =
            resumable ? "The file transfer was paused locally."
                      : "The file transfer was canceled locally.";
    }
    PostPriority([this, transferId, peer, partial, source, resumable] {
        storage_->CloseSource(source);
        if (resumable) {
            storage_->CloseDestination(partial);
        } else {
            storage_->RemovePartial(partial);
        }
        FileTransferMessage cancel;
        cancel.type = FileTransferMessageType::kCancel;
        cancel.transferId = transferId;
        cancel.errorCode = resumable ? "user_paused" : "user_canceled";
        cancel.errorMessage = resumable
            ? "The peer paused the file transfer."
            : "The peer canceled the file transfer.";
        SendWithBackpressureRetry(peer, cancel);
    });
    PublishSnapshots();
    return Accepted(transferId);
}

FileTransferCommandResult FileTransferController::Resume(
    const std::string& transferId)
{
    std::string peer;
    FileTransferDirection direction = FileTransferDirection::kOutgoing;
    std::uint64_t resumeOffset = 0;
    {
        std::lock_guard lock(mutex_);
        const auto found = transfers_.find(transferId);
        if (found == transfers_.end() ||
            found->second->snapshot.state != FileTransferState::kPaused) {
            return Rejected("file_transfer_not_paused",
                            "The file transfer is not paused.");
        }
        auto& transfer = *found->second;
        peer = transfer.snapshot.peerDeviceId;
        direction = transfer.snapshot.direction;
        resumeOffset = transfer.snapshot.transferredBytes;
        transfer.snapshot.state = direction == FileTransferDirection::kIncoming
            ? FileTransferState::kTransferring
            : FileTransferState::kResuming;
        transfer.snapshot.errorCode.clear();
        transfer.snapshot.errorMessage.clear();
        ResetRateLocked(transfer);
    }
    // Resume is deliberately queued behind a pending priority pause. This
    // preserves DataChannel ordering when the user clicks Continue quickly.
    Post([this, transferId, peer, direction, resumeOffset] {
        FileTransferMessage message;
        message.transferId = transferId;
        if (direction == FileTransferDirection::kIncoming) {
            message.type = FileTransferMessageType::kResponse;
            message.accepted = true;
            message.resumeOffset = resumeOffset;
        } else {
            message.type = FileTransferMessageType::kResume;
        }
        const auto result = SendWithBackpressureRetry(peer, message);
        if (!result.accepted) {
            {
                std::lock_guard lock(mutex_);
                const auto found = transfers_.find(transferId);
                if (found != transfers_.end()) {
                    found->second->snapshot.state = FileTransferState::kPaused;
                    found->second->snapshot.errorCode = result.errorCode;
                    found->second->snapshot.errorMessage = result.errorMessage;
                }
            }
            PublishSnapshots();
        }
    });
    PublishSnapshots();
    return Accepted(transferId);
}
