// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

void FileTransferController::PrepareOutgoing(
    const std::string& transferId)
{
    std::filesystem::path sourcePath;
    {
        std::lock_guard lock(mutex_);
        const auto found = transfers_.find(transferId);
        if (found == transfers_.end() ||
            found->second->snapshot.state != FileTransferState::kPreparing) {
            return;
        }
        sourcePath = found->second->sourcePath;
    }
    FileTransferSourceInfo source;
    std::string error;
    if (!storage_->InspectSource(sourcePath, &source, &error)) {
        FailTransfer(transferId, "source_unavailable", std::move(error),
                     false);
        return;
    }
    std::string peer;
    {
        std::lock_guard lock(mutex_);
        const auto found = transfers_.find(transferId);
        if (found == transfers_.end() ||
            found->second->snapshot.state != FileTransferState::kPreparing) {
            return;
        }
        auto& transfer = *found->second;
        transfer.snapshot.fileName = source.fileName;
        transfer.snapshot.fileSize = source.fileSize;
        transfer.sha256 = source.sha256;
        transfer.snapshot.state = FileTransferState::kAwaitingPeerAcceptance;
        peer = transfer.snapshot.peerDeviceId;
    }
    FileTransferMessage offer;
    offer.type = FileTransferMessageType::kOffer;
    offer.transferId = transferId;
    offer.fileName = source.fileName;
    offer.fileSize = source.fileSize;
    offer.sha256 = source.sha256;
    offer.hasSha256 = true;
    const auto result = SendWithBackpressureRetry(peer, offer);
    if (!result.accepted) {
        FailTransfer(transferId, result.errorCode, result.errorMessage, false);
        return;
    }
    PublishSnapshots();
}
