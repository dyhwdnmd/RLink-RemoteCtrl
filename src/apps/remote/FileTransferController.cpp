// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#include "FileTransferController.h"

#include <algorithm>
#include <array>
#include <chrono>
#include <cmath>
#include <iomanip>
#include <random>
#include <sstream>
#include <unordered_set>
#include <utility>

namespace remote::app {
namespace {

constexpr int kControlBackpressureRetryCount = 200;
constexpr auto kBackpressureRetryDelay = std::chrono::milliseconds(10);
constexpr auto kFilePayloadBackpressureRetryDelay =
    std::chrono::milliseconds(20);
constexpr std::size_t kFileTransferChunkBytes = 60 * 1024;
constexpr std::uint64_t kFileTransferSendBurstBytes = 1024 * 1024;
constexpr std::uint64_t kFileTransferMaximumInFlightBytes = 2 * 1024 * 1024;
constexpr std::uint64_t kFileTransferProgressAckBytes = 256 * 1024;
constexpr auto kFileTransferProgressAckInterval =
    std::chrono::milliseconds(100);
constexpr auto kFileTransferProgressPublishInterval =
    std::chrono::milliseconds(100);
constexpr auto kFileTransferStallTimeout = std::chrono::seconds(15);
constexpr auto kIncomingFileTransferStallTimeout = std::chrono::seconds(60);
constexpr std::uint32_t kMaximumFileResyncAttempts = 3;
constexpr auto kFileTransferStallCheckInterval =
    std::chrono::milliseconds(500);
constexpr auto kFileTransferRateWindow = std::chrono::seconds(3);
constexpr auto kFileTransferRateRefreshInterval =
    std::chrono::milliseconds(500);

bool IsTerminal(FileTransferState state)
{
    return state == FileTransferState::kCompleted ||
           state == FileTransferState::kRejected ||
           state == FileTransferState::kPaused ||
           state == FileTransferState::kCanceled ||
           state == FileTransferState::kFailed;
}

bool IsFileTransferBackpressure(const SessionCommandResult& result)
{
    return result.errorCode == "room_file_transfer_backpressure" ||
           result.errorCode == "direct_file_transfer_backpressure";
}

FileTransferCommandResult Accepted(std::string transferId)
{
    FileTransferCommandResult result;
    result.accepted = true;
    result.transferId = std::move(transferId);
    return result;
}

FileTransferCommandResult Rejected(std::string errorCode,
                                   std::string errorMessage)
{
    FileTransferCommandResult result;
    result.errorCode = std::move(errorCode);
    result.errorMessage = std::move(errorMessage);
    return result;
}

}  // namespace

struct FileTransferController::TransferRecord {
    struct RateSample {
        std::chrono::steady_clock::time_point at;
        std::uint64_t bytes = 0;
    };

    FileTransferSnapshot snapshot;
    std::filesystem::path sourcePath;
    FileTransferDestination destination;
    std::array<std::uint8_t, 32> sha256{};
    std::uint64_t nextSendOffset = 0;
    std::uint64_t receiverCommittedOffset = 0;
    std::uint64_t lastAcknowledgedOffset = 0;
    std::chrono::steady_clock::time_point progressPublishedAt{};
    std::chrono::steady_clock::time_point lastAcknowledgedAt{};
    std::chrono::steady_clock::time_point lastProgressAt{};
    std::deque<RateSample> rateSamples;
    std::chrono::steady_clock::time_point lastRatePublishedAt{};
    bool replaceExisting = false;
    bool interruptedByNetwork = false;
    bool sendBackpressured = false;
    std::uint32_t resyncAttempts = 0;
};

#include "FileTransferController.Lifecycle.inc"
#include "FileTransferController.Commands.inc"
#include "FileTransferController.Worker.inc"
#include "FileTransferController.Send.inc"
#include "FileTransferController.Receive.inc"
#include "FileTransferController.Chunking.inc"
#include "FileTransferController.Reliability.inc"
void FileTransferController::PublishSnapshots()
{
    IFileTransferControllerObserver* observer = nullptr;
    std::vector<FileTransferSnapshot> snapshots;
    {
        std::lock_guard lock(mutex_);
        observer = observer_;
        if (!observer) {
            return;
        }
        snapshots.reserve(transfers_.size());
        for (const auto& [id, transfer] : transfers_) {
            snapshots.push_back(transfer->snapshot);
        }
    }
    std::sort(snapshots.begin(), snapshots.end(),
              [](const auto& left, const auto& right) {
                  return left.displayOrder > right.displayOrder;
              });
    observer->OnFileTransfersChanged(snapshots);
}

std::string FileTransferController::GenerateTransferId()
{
    std::random_device random;
    for (;;) {
        std::array<std::uint32_t, 4> words{};
        for (auto& word : words) {
            word = random();
        }
        std::ostringstream stream;
        stream << 'f' << std::hex << std::setfill('0');
        for (const auto word : words) {
            stream << std::setw(8) << word;
        }
        const std::string value = stream.str();
        std::lock_guard lock(mutex_);
        if (!transfers_.contains(value)) {
            return value;
        }
    }
}

}  // namespace remote::app
