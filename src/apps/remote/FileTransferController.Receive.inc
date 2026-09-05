// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

void FileTransferController::PrepareIncoming(
    const std::string& transferId)
{
    std::filesystem::path directory;
    std::string fileName;
    std::string peer;
    std::uint64_t fileSize = 0;
    bool replaceExisting = false;
    {
        std::lock_guard lock(mutex_);
        const auto found = transfers_.find(transferId);
        if (found == transfers_.end() ||
            found->second->snapshot.state != FileTransferState::kPreparing ||
            found->second->snapshot.direction !=
                FileTransferDirection::kIncoming) {
            return;
        }
        directory = found->second->snapshot.localPath;
        fileName = found->second->snapshot.fileName;
        fileSize = found->second->snapshot.fileSize;
        peer = found->second->snapshot.peerDeviceId;
        replaceExisting = found->second->replaceExisting;
    }
    FileTransferDestination destination;
    std::string error;
    if (!storage_->PrepareDestination(
            directory, fileName, transferId, fileSize, replaceExisting,
            &destination,
            &error)) {
        FileTransferMessage response;
        response.type = FileTransferMessageType::kResponse;
        response.transferId = transferId;
        response.errorCode = "destination_unavailable";
        response.errorMessage = error;
        SendWithBackpressureRetry(peer, response);
        FailTransfer(transferId, response.errorCode, std::move(error), false);
        return;
    }
    {
        std::lock_guard lock(mutex_);
        const auto found = transfers_.find(transferId);
        if (found == transfers_.end() ||
            found->second->snapshot.state != FileTransferState::kPreparing) {
            storage_->RemovePartial(destination.temporaryPath);
            return;
        }
        found->second->destination = destination;
        found->second->snapshot.localPath = destination.finalPath;
        found->second->snapshot.transferredBytes = destination.resumeOffset;
        found->second->receiverCommittedOffset = destination.resumeOffset;
        found->second->lastAcknowledgedOffset = destination.resumeOffset;
        found->second->lastAcknowledgedAt =
            std::chrono::steady_clock::now();
        found->second->progressPublishedAt =
            std::chrono::steady_clock::now();
        found->second->snapshot.state = FileTransferState::kTransferring;
        ResetRateLocked(*found->second);
    }
    FileTransferMessage response;
    response.type = FileTransferMessageType::kResponse;
    response.transferId = transferId;
    response.accepted = true;
    response.resumeOffset = destination.resumeOffset;
    const auto result = SendWithBackpressureRetry(peer, response);
    if (!result.accepted) {
        FailTransfer(transferId, result.errorCode, result.errorMessage, false);
        return;
    }
    PublishSnapshots();
}

void FileTransferController::ProcessEnvelope(FileTransferEnvelope envelope)
{
    const auto& message = envelope.message;
    if (message.type == FileTransferMessageType::kOffer) {
        bool duplicate = false;
        {
            std::lock_guard lock(mutex_);
            duplicate = transfers_.contains(message.transferId);
            if (!duplicate && message.hasSha256) {
                auto transfer = std::make_unique<TransferRecord>();
                transfer->snapshot.transferId = message.transferId;
                transfer->snapshot.peerDeviceId = envelope.senderDeviceId;
                transfer->snapshot.fileName = message.fileName;
                transfer->snapshot.fileSize = message.fileSize;
                transfer->snapshot.direction =
                    FileTransferDirection::kIncoming;
                transfer->snapshot.state =
                    FileTransferState::kAwaitingLocalAcceptance;
                transfer->snapshot.displayOrder = ++nextDisplayOrder_;
                transfer->sha256 = message.sha256;
                transfers_.emplace(message.transferId, std::move(transfer));
            }
        }
        if (duplicate) {
            SendProtocolError(envelope.senderDeviceId, message.transferId,
                              "duplicate_transfer_id",
                              "The transfer ID is already in use.");
        } else if (!message.hasSha256) {
            SendProtocolError(envelope.senderDeviceId, message.transferId,
                              "sha256_required",
                              "A SHA-256 digest is required.");
        } else {
            PublishSnapshots();
        }
        return;
    }

    std::unique_lock lock(mutex_);
    const auto found = transfers_.find(message.transferId);
    if (found == transfers_.end() ||
        found->second->snapshot.peerDeviceId != envelope.senderDeviceId) {
        lock.unlock();
        SendProtocolError(envelope.senderDeviceId, message.transferId,
                          "unknown_transfer",
                          "The file transfer is not active.");
        return;
    }
    auto& transfer = *found->second;
    const auto direction = transfer.snapshot.direction;
    const auto state = transfer.snapshot.state;

    if (message.type == FileTransferMessageType::kResponse) {
        if (direction != FileTransferDirection::kOutgoing ||
            (state != FileTransferState::kAwaitingPeerAcceptance &&
             state != FileTransferState::kPaused &&
             state != FileTransferState::kResuming)) {
            lock.unlock();
            SendProtocolError(envelope.senderDeviceId, message.transferId,
                              "unexpected_response",
                              "The file response is not expected.");
            return;
        }
        if (!message.accepted) {
            transfer.snapshot.state = state == FileTransferState::kResuming
                ? FileTransferState::kPaused
                : FileTransferState::kRejected;
            transfer.snapshot.errorCode = message.errorCode;
            transfer.snapshot.errorMessage = message.errorMessage;
            lock.unlock();
            PublishSnapshots();
            return;
        }
        if (message.resumeOffset > transfer.snapshot.fileSize) {
            lock.unlock();
            FailTransfer(message.transferId, "invalid_resume_offset",
                         "The receiver resume offset is outside the file.",
                         true);
            return;
        }
        transfer.snapshot.transferredBytes = message.resumeOffset;
        transfer.nextSendOffset = message.resumeOffset;
        transfer.receiverCommittedOffset = message.resumeOffset;
        transfer.lastAcknowledgedOffset = message.resumeOffset;
        transfer.lastAcknowledgedAt =
            std::chrono::steady_clock::now();
        transfer.snapshot.state = FileTransferState::kTransferring;
        transfer.snapshot.errorCode.clear();
        transfer.snapshot.errorMessage.clear();
        transfer.interruptedByNetwork = false;
        transfer.resyncAttempts = 0;
        ResetRateLocked(transfer);
        lock.unlock();
        PublishSnapshots();
        SendAvailableChunks(message.transferId);
        return;
    }

    if (message.type == FileTransferMessageType::kResume) {
        if (direction == FileTransferDirection::kIncoming &&
            state == FileTransferState::kCompleted) {
            const auto peer = transfer.snapshot.peerDeviceId;
            const auto fileSize = transfer.snapshot.fileSize;
            const auto digest = transfer.sha256;
            lock.unlock();
            FileTransferMessage receipt;
            receipt.type = FileTransferMessageType::kComplete;
            receipt.transferId = message.transferId;
            receipt.fileSize = fileSize;
            receipt.sha256 = digest;
            receipt.hasSha256 = true;
            SendWithBackpressureRetry(peer, receipt);
            return;
        }
        if (direction != FileTransferDirection::kIncoming ||
            (state != FileTransferState::kPaused &&
             state != FileTransferState::kTransferring &&
             state != FileTransferState::kRecovering) ||
            transfer.destination.temporaryPath.empty()) {
            lock.unlock();
            SendProtocolError(envelope.senderDeviceId, message.transferId,
                              "resume_not_available",
                              "The file transfer cannot be resumed.");
            return;
        }
        const auto peer = transfer.snapshot.peerDeviceId;
        const auto resumeOffset = transfer.snapshot.transferredBytes;
        transfer.snapshot.state = FileTransferState::kTransferring;
        transfer.snapshot.errorCode.clear();
        transfer.snapshot.errorMessage.clear();
        transfer.interruptedByNetwork = false;
        transfer.resyncAttempts = 0;
        ResetRateLocked(transfer);
        lock.unlock();
        FileTransferMessage response;
        response.type = FileTransferMessageType::kResponse;
        response.transferId = message.transferId;
        response.accepted = true;
        response.resumeOffset = resumeOffset;
        const auto result = SendWithBackpressureRetry(peer, response);
        if (!result.accepted) {
            std::lock_guard updateLock(mutex_);
            const auto current = transfers_.find(message.transferId);
            if (current != transfers_.end()) {
                current->second->snapshot.state =
                    state == FileTransferState::kRecovering
                        ? FileTransferState::kRecovering
                        : FileTransferState::kPaused;
                current->second->snapshot.errorCode = result.errorCode;
                current->second->snapshot.errorMessage = result.errorMessage;
            }
        }
        PublishSnapshots();
        return;
    }

    if (message.type == FileTransferMessageType::kProgress) {
        if (direction != FileTransferDirection::kOutgoing) {
            lock.unlock();
            SendProtocolError(envelope.senderDeviceId, message.transferId,
                              "invalid_progress_offset",
                              "The receiver progress offset is invalid.");
            return;
        }

        // Progress acknowledgements are sent over the reliable DataChannel,
        // but one that was already queued can legitimately arrive after the
        // local user pauses the sender or while a resume/recovery handshake is
        // in progress. Treat duplicate and terminal-state acknowledgements as
        // idempotent instead of poisoning the peer's resumable transfer.
        const bool permanentlyFinished =
            state == FileTransferState::kCompleted ||
            state == FileTransferState::kRejected ||
            state == FileTransferState::kCanceled ||
            state == FileTransferState::kFailed;
        if (message.resumeOffset <= transfer.receiverCommittedOffset ||
            permanentlyFinished) {
            return;
        }

        const bool acceptsProgress =
            state == FileTransferState::kTransferring ||
            state == FileTransferState::kAwaitingCompletion ||
            state == FileTransferState::kPaused ||
            state == FileTransferState::kResuming ||
            state == FileTransferState::kRecovering;
        if (!acceptsProgress ||
            message.resumeOffset > transfer.nextSendOffset ||
            message.resumeOffset > transfer.snapshot.fileSize) {
            lock.unlock();
            SendProtocolError(envelope.senderDeviceId, message.transferId,
                              "invalid_progress_offset",
                              "The receiver progress offset is invalid.");
            return;
        }
        const bool advanced =
            message.resumeOffset > transfer.receiverCommittedOffset;
        transfer.receiverCommittedOffset = message.resumeOffset;
        if (advanced) {
            UpdateProgressLocked(transfer, message.resumeOffset);
        }
        const bool continueSending =
            state == FileTransferState::kTransferring &&
            transfer.nextSendOffset < transfer.snapshot.fileSize;
        lock.unlock();
        if (advanced) {
            PublishSnapshots();
        }
        if (continueSending) {
            Post([this, transferId = message.transferId] {
                SendAvailableChunks(transferId);
            });
        }
        return;
    }

    if ((state == FileTransferState::kPaused ||
         state == FileTransferState::kRecovering) &&
        (message.type == FileTransferMessageType::kChunk ||
         message.type == FileTransferMessageType::kComplete)) {
        return;
    }
    if (state == FileTransferState::kResuming &&
        message.type == FileTransferMessageType::kChunk) {
        return;
    }

    if (IsTerminal(state) && state != FileTransferState::kPaused) {
        return;
    }

    if (message.type == FileTransferMessageType::kChunk) {
        if (direction != FileTransferDirection::kIncoming ||
            state != FileTransferState::kTransferring ||
            message.chunkOffset > transfer.snapshot.transferredBytes) {
            lock.unlock();
            FailTransfer(message.transferId, "invalid_chunk_offset",
                         "The received file chunk is out of sequence.", true);
            return;
        }
        if (message.chunkOffset < transfer.snapshot.transferredBytes) {
            const auto duplicateEnd =
                message.chunkOffset + message.chunk.size();
            if (duplicateEnd < message.chunkOffset ||
                duplicateEnd > transfer.snapshot.transferredBytes) {
                lock.unlock();
                FailTransfer(message.transferId, "overlapping_chunk_offset",
                             "The received file chunk partially overlaps "
                             "committed data.", true);
                return;
            }
            lock.unlock();
            return;
        }
        if (message.chunk.size() >
            transfer.snapshot.fileSize - transfer.snapshot.transferredBytes) {
            lock.unlock();
            FailTransfer(message.transferId, "invalid_chunk_offset",
                         "The received file chunk exceeds the expected file "
                         "size.", true);
            return;
        }
        const auto temporaryPath = transfer.destination.temporaryPath;
        const auto expectedOffset = transfer.snapshot.transferredBytes;
        lock.unlock();
        std::string error;
        if (!storage_->AppendChunk(temporaryPath, expectedOffset,
                                       message.chunk, &error)) {
            FailTransfer(message.transferId, "destination_write_failed",
                         std::move(error), true);
            return;
        }
        const std::uint64_t acknowledged =
            expectedOffset + message.chunk.size();
        bool publishProgress = false;
        bool sendProgress = false;
        std::string progressPeer;
        {
            std::lock_guard updateLock(mutex_);
            const auto current = transfers_.find(message.transferId);
            if (current == transfers_.end() ||
                IsTerminal(current->second->snapshot.state)) {
                return;
            }
            auto& currentTransfer = *current->second;
            UpdateProgressLocked(currentTransfer, acknowledged);
            const auto now = std::chrono::steady_clock::now();
            sendProgress =
                acknowledged == currentTransfer.snapshot.fileSize ||
                acknowledged - currentTransfer.lastAcknowledgedOffset >=
                    kFileTransferProgressAckBytes ||
                currentTransfer.lastAcknowledgedAt.time_since_epoch().count() ==
                    0 ||
                now - currentTransfer.lastAcknowledgedAt >=
                    kFileTransferProgressAckInterval;
            if (sendProgress) {
                currentTransfer.lastAcknowledgedOffset = acknowledged;
                currentTransfer.lastAcknowledgedAt = now;
                progressPeer = currentTransfer.snapshot.peerDeviceId;
            }
            publishProgress =
                acknowledged == currentTransfer.snapshot.fileSize ||
                now - currentTransfer.progressPublishedAt >=
                    kFileTransferProgressPublishInterval;
            if (publishProgress) {
                currentTransfer.progressPublishedAt = now;
            }
        }
        if (publishProgress) {
            PublishSnapshots();
        }
        if (sendProgress) {
            FileTransferMessage progress;
            progress.type = FileTransferMessageType::kProgress;
            progress.transferId = message.transferId;
            progress.resumeOffset = acknowledged;
            const auto progressResult =
                SendWithBackpressureRetry(progressPeer, progress);
            if (!progressResult.accepted) {
                // The reliable file channel will preserve already received
                // chunks. If this ACK cannot be sent now, the sender's stall
                // recovery asks for the same durable offset via Resume.
            }
        }
        return;
    }

    if (message.type == FileTransferMessageType::kComplete) {
        if (message.fileSize != transfer.snapshot.fileSize ||
            message.sha256 != transfer.sha256) {
            lock.unlock();
            FailTransfer(message.transferId, "completion_mismatch",
                         "The completion metadata does not match the offer.",
                         true);
            return;
        }
        if (direction == FileTransferDirection::kOutgoing) {
            if (state != FileTransferState::kAwaitingCompletion &&
                state != FileTransferState::kResuming) {
                lock.unlock();
                SendProtocolError(envelope.senderDeviceId,
                                  message.transferId,
                                  "unexpected_complete",
                                  "The completion receipt is not expected.");
                return;
            }
            UpdateProgressLocked(transfer, transfer.snapshot.fileSize);
            transfer.snapshot.state = FileTransferState::kCompleted;
            const auto source = transfer.sourcePath;
            lock.unlock();
            storage_->CloseSource(source);
            PublishSnapshots();
            return;
        }
        if (state != FileTransferState::kTransferring ||
            transfer.snapshot.transferredBytes != transfer.snapshot.fileSize) {
            lock.unlock();
            FailTransfer(message.transferId, "incomplete_file",
                         "The file completion arrived before all chunks.",
                         true);
            return;
        }
        transfer.snapshot.state = FileTransferState::kVerifying;
        const auto destination = transfer.destination;
        const auto peer = transfer.snapshot.peerDeviceId;
        const auto fileSize = transfer.snapshot.fileSize;
        const auto digest = transfer.sha256;
        lock.unlock();
        PublishSnapshots();
        std::string error;
        if (!storage_->VerifyAndCommit(
                destination, fileSize, digest, &error)) {
            storage_->RemovePartial(destination.temporaryPath);
            FailTransfer(message.transferId, "integrity_check_failed",
                         std::move(error), true);
            return;
        }
        {
            std::lock_guard updateLock(mutex_);
            const auto current = transfers_.find(message.transferId);
            if (current == transfers_.end()) {
                return;
            }
            current->second->snapshot.localPath = destination.finalPath;
            current->second->snapshot.state = FileTransferState::kCompleted;
        }
        FileTransferMessage receipt;
        receipt.type = FileTransferMessageType::kComplete;
        receipt.transferId = message.transferId;
        receipt.fileSize = fileSize;
        receipt.sha256 = digest;
        receipt.hasSha256 = true;
        SendWithBackpressureRetry(peer, receipt);
        PublishSnapshots();
        return;
    }

    if (message.type == FileTransferMessageType::kCancel ||
        message.type == FileTransferMessageType::kError) {
        const auto partial = transfer.destination.temporaryPath;
        const auto source = transfer.sourcePath;
        const bool paused = message.type == FileTransferMessageType::kCancel &&
                            message.errorCode == "user_paused" &&
                            (!partial.empty() || !source.empty());
        transfer.snapshot.state = message.type == FileTransferMessageType::kError
            ? FileTransferState::kFailed
            : (paused ? FileTransferState::kPaused
                      : FileTransferState::kCanceled);
        transfer.snapshot.bytesPerSecond = 0.0;
        transfer.snapshot.estimatedRemainingSeconds = 0;
        transfer.snapshot.errorCode = message.errorCode;
        transfer.snapshot.errorMessage = message.errorMessage;
        lock.unlock();
        storage_->CloseSource(source);
        if (paused) {
            storage_->CloseDestination(partial);
        } else {
            storage_->RemovePartial(partial);
        }
        PublishSnapshots();
        return;
    }

    lock.unlock();
    SendProtocolError(envelope.senderDeviceId, message.transferId,
                      "unexpected_message",
                      "The file transfer message is not expected.");
}
