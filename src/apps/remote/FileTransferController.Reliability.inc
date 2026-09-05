// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

void FileTransferController::CheckStalledTransfers()
{
    std::vector<std::pair<std::string, FileTransferDirection>> stalled;
    std::vector<std::pair<std::string, std::string>> resyncs;
    const auto now = std::chrono::steady_clock::now();
    bool rateChanged = false;
    {
        std::lock_guard lock(mutex_);
        for (const auto& [id, transferPointer] : transfers_) {
            auto& transfer = *transferPointer;
            rateChanged = RefreshRateLocked(transfer, now) || rateChanged;
            const auto state = transfer.snapshot.state;
            if (transfer.lastProgressAt.time_since_epoch().count() == 0) {
                continue;
            }
            if (transfer.sendBackpressured) {
                // The local SCTP queue still contains accepted data. Its
                // high-water condition is normal flow control, not evidence
                // that the receiver's committed offset is stale.
                continue;
            }
            if (transfer.snapshot.direction ==
                    FileTransferDirection::kOutgoing &&
                (state == FileTransferState::kTransferring ||
                 state == FileTransferState::kResuming) &&
                (state == FileTransferState::kResuming ||
                 transfer.nextSendOffset <
                     transfer.snapshot.fileSize) &&
                now - transfer.lastProgressAt >=
                    kFileTransferStallTimeout) {
                if (transfer.resyncAttempts <
                    kMaximumFileResyncAttempts) {
                    ++transfer.resyncAttempts;
                    transfer.snapshot.state = FileTransferState::kResuming;
                    transfer.snapshot.bytesPerSecond = 0.0;
                    transfer.snapshot.estimatedRemainingSeconds = 0;
                    transfer.snapshot.errorCode =
                        "file_transfer_resynchronizing";
                    transfer.snapshot.errorMessage =
                        "No file-transfer progress was observed; RemoteC "
                        "is asking the receiver for its committed offset.";
                    transfer.lastProgressAt = now;
                    resyncs.emplace_back(
                        id, transfer.snapshot.peerDeviceId);
                } else {
                    stalled.emplace_back(
                        id, FileTransferDirection::kOutgoing);
                }
                continue;
            }
            if (transfer.snapshot.direction ==
                    FileTransferDirection::kIncoming &&
                state == FileTransferState::kTransferring &&
                transfer.snapshot.transferredBytes <
                    transfer.snapshot.fileSize &&
                now - transfer.lastProgressAt >=
                    kIncomingFileTransferStallTimeout) {
                stalled.emplace_back(
                    id, FileTransferDirection::kIncoming);
            }
        }
    }
    for (const auto& [id, peer] : resyncs) {
        FileTransferMessage resume;
        resume.type = FileTransferMessageType::kResume;
        resume.transferId = id;
        const auto result = SendWithBackpressureRetry(peer, resume);
        if (!result.accepted) {
            std::lock_guard lock(mutex_);
            const auto found = transfers_.find(id);
            if (found != transfers_.end() &&
                found->second->snapshot.state ==
                    FileTransferState::kResuming) {
                found->second->snapshot.errorCode = result.errorCode;
                found->second->snapshot.errorMessage =
                    "File transfer resynchronization is waiting for the "
                    "P2P connection.";
            }
        }
    }
    if (rateChanged || !resyncs.empty()) {
        PublishSnapshots();
    }
    for (const auto& [id, direction] : stalled) {
        FailTransfer(id, "file_transfer_stalled",
                     direction == FileTransferDirection::kOutgoing
                         ? "The transfer did not recover after three "
                           "resynchronization attempts."
                         : "No file data was received for 60 seconds.",
                     true);
    }
}

SessionCommandResult FileTransferController::SendWithBackpressureRetry(
    const std::string& peerDeviceId,
    const FileTransferMessage& message)
{
    SessionCommandResult result;
    for (int attempt = 0; attempt < kControlBackpressureRetryCount;
         ++attempt) {
        result = SendOnce(peerDeviceId, message);
        if (result.accepted || !IsFileTransferBackpressure(result)) {
            return result;
        }
        std::this_thread::sleep_for(kBackpressureRetryDelay);
    }
    result.errorCode = "file_transfer_control_backpressure_timeout";
    result.errorMessage =
        "The file control message could not be queued while the transfer "
        "channel was congested.";
    return result;
}

SessionCommandResult FileTransferController::SendOnce(
    const std::string& peerDeviceId,
    const FileTransferMessage& message)
{
    SessionCommandResult result;
    if (!send_) {
        result.errorCode = "file_transport_unavailable";
        result.errorMessage = "The file transfer transport is unavailable.";
        return result;
    }
    return send_(peerDeviceId, message);
}

void FileTransferController::FailTransfer(
    const std::string& transferId,
    std::string errorCode,
    std::string errorMessage,
    bool notifyPeer)
{
    std::string peer;
    std::filesystem::path partial;
    std::filesystem::path source;
    {
        std::lock_guard lock(mutex_);
        const auto found = transfers_.find(transferId);
        if (found == transfers_.end() ||
            IsTerminal(found->second->snapshot.state)) {
            return;
        }
        peer = found->second->snapshot.peerDeviceId;
        partial = found->second->destination.temporaryPath;
        source = found->second->sourcePath;
        found->second->snapshot.state = FileTransferState::kFailed;
        found->second->snapshot.errorCode = errorCode;
        found->second->snapshot.errorMessage = errorMessage;
    }
    storage_->CloseSource(source);
    storage_->RemovePartial(partial);
    if (notifyPeer) {
        SendProtocolError(peer, transferId, errorCode, errorMessage);
    }
    PublishSnapshots();
}

void FileTransferController::SendProtocolError(
    const std::string& peerDeviceId,
    const std::string& transferId,
    const std::string& errorCode,
    const std::string& errorMessage)
{
    FileTransferMessage message;
    message.type = FileTransferMessageType::kError;
    message.transferId = transferId;
    message.errorCode = errorCode;
    message.errorMessage = errorMessage.substr(0, 512);
    SendWithBackpressureRetry(peerDeviceId, message);
}
