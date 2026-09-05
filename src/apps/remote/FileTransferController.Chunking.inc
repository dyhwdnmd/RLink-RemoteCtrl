// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

void FileTransferController::SendAvailableChunks(
    const std::string& transferId)
{
    std::uint64_t burstBytes = 0;
    for (;;) {
        std::filesystem::path sourcePath;
        std::string peer;
        std::uint64_t offset = 0;
        std::uint64_t receiverCommittedOffset = 0;
        std::uint64_t fileSize = 0;
        std::array<std::uint8_t, 32> digest{};
        {
            std::lock_guard lock(mutex_);
            const auto found = transfers_.find(transferId);
            if (found == transfers_.end() ||
                found->second->snapshot.direction !=
                    FileTransferDirection::kOutgoing ||
                (found->second->snapshot.state !=
                     FileTransferState::kTransferring &&
                 found->second->snapshot.state !=
                     FileTransferState::kAwaitingCompletion)) {
                return;
            }
            auto& transfer = *found->second;
            sourcePath = transfer.sourcePath;
            peer = transfer.snapshot.peerDeviceId;
            offset = transfer.nextSendOffset;
            receiverCommittedOffset = transfer.receiverCommittedOffset;
            fileSize = transfer.snapshot.fileSize;
            digest = transfer.sha256;
            if (transfer.snapshot.state ==
                    FileTransferState::kTransferring &&
                offset >= receiverCommittedOffset &&
                offset - receiverCommittedOffset >=
                    kFileTransferMaximumInFlightBytes) {
                // Wait for the receiver's durable .part offset. The incoming
                // Progress message will queue the next burst.
                return;
            }
            if (offset == fileSize) {
                transfer.snapshot.state =
                    FileTransferState::kAwaitingCompletion;
                transfer.lastProgressAt = std::chrono::steady_clock::now();
            }
        }
        if (offset == fileSize) {
            FileTransferMessage complete;
            complete.type = FileTransferMessageType::kComplete;
            complete.transferId = transferId;
            complete.fileSize = fileSize;
            complete.sha256 = digest;
            complete.hasSha256 = true;
            const auto result = SendOnce(peer, complete);
            if (IsFileTransferBackpressure(result)) {
                {
                    std::lock_guard lock(mutex_);
                    const auto found = transfers_.find(transferId);
                    if (found == transfers_.end() ||
                        found->second->snapshot.state !=
                            FileTransferState::kAwaitingCompletion) {
                        return;
                    }
                    found->second->sendBackpressured = true;
                    found->second->lastProgressAt =
                        std::chrono::steady_clock::now();
                }
                PostAfter(kFilePayloadBackpressureRetryDelay,
                          [this, transferId] {
                              SendAvailableChunks(transferId);
                          });
                return;
            }
            if (!result.accepted) {
                FailTransfer(transferId, result.errorCode,
                             result.errorMessage, true);
            } else {
                {
                    std::lock_guard lock(mutex_);
                    const auto found = transfers_.find(transferId);
                    if (found != transfers_.end()) {
                        found->second->sendBackpressured = false;
                    }
                }
                PublishSnapshots();
            }
            return;
        }

        std::vector<std::uint8_t> bytes;
        std::string error;
        if (!storage_->ReadChunk(sourcePath, offset,
                                 kFileTransferChunkBytes, &bytes,
                                 &error)) {
            FailTransfer(transferId, "source_read_failed", std::move(error),
                         true);
            return;
        }
        const auto nextOffset = offset + bytes.size();
        FileTransferMessage chunk;
        chunk.type = FileTransferMessageType::kChunk;
        chunk.transferId = transferId;
        chunk.chunkOffset = offset;
        chunk.chunk = std::move(bytes);
        const auto result = SendOnce(peer, chunk);
        if (IsFileTransferBackpressure(result)) {
            {
                std::lock_guard lock(mutex_);
                const auto found = transfers_.find(transferId);
                if (found == transfers_.end() ||
                    found->second->snapshot.state !=
                        FileTransferState::kTransferring ||
                    found->second->nextSendOffset != offset) {
                    return;
                }
                // Keep the sender offset fixed until WebRTC accepts this
                // exact chunk. Backpressure is flow control, not failure.
                found->second->sendBackpressured = true;
                found->second->lastProgressAt =
                    std::chrono::steady_clock::now();
            }
            PostAfter(kFilePayloadBackpressureRetryDelay,
                      [this, transferId] {
                          SendAvailableChunks(transferId);
                      });
            return;
        }
        if (!result.accepted) {
            FailTransfer(transferId, result.errorCode, result.errorMessage,
                         true);
            return;
        }

        bool publishProgress = false;
        {
            std::lock_guard lock(mutex_);
            const auto found = transfers_.find(transferId);
            if (found == transfers_.end() ||
                found->second->snapshot.state !=
                    FileTransferState::kTransferring ||
                found->second->nextSendOffset != offset) {
                return;
            }
            auto& transfer = *found->second;
            transfer.sendBackpressured = false;
            transfer.nextSendOffset = nextOffset;
            // Sender progress is advanced only by a receiver Progress ACK.
            // This keeps speed, ETA and resume state tied to bytes already
            // written to the peer's .part file rather than local enqueue.
            transfer.lastProgressAt = std::chrono::steady_clock::now();
            const auto now = std::chrono::steady_clock::now();
            publishProgress = nextOffset == fileSize ||
                now - transfer.progressPublishedAt >=
                    kFileTransferProgressPublishInterval;
            if (publishProgress) {
                transfer.progressPublishedAt = now;
            }
        }
        burstBytes += chunk.chunk.size();
        if (publishProgress) {
            PublishSnapshots();
        }
        if (burstBytes >= kFileTransferSendBurstBytes) {
            Post([this, transferId] { SendAvailableChunks(transferId); });
            return;
        }
    }
}
