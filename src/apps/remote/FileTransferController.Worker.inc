// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

void FileTransferController::OnFileTransferMessage(
    const FileTransferEnvelope& envelope)
{
    Post([this, envelope] { ProcessEnvelope(envelope); });
}

void FileTransferController::WorkerMain(std::stop_token stopToken)
{
    while (!stopToken.stop_requested()) {
        std::function<void()> task;
        bool shouldStop = false;
        {
            std::unique_lock lock(mutex_);
            for (;;) {
                if (stopping_ || stopToken.stop_requested()) {
                    shouldStop = true;
                    break;
                }
                if (!workQueue_.empty()) {
                    task = std::move(workQueue_.front());
                    workQueue_.pop_front();
                    break;
                }

                const auto now = std::chrono::steady_clock::now();
                const auto scheduled = std::min_element(
                    scheduledTasks_.begin(), scheduledTasks_.end(),
                    [](const ScheduledTask& left,
                       const ScheduledTask& right) {
                        return left.due < right.due ||
                               (left.due == right.due &&
                                left.order < right.order);
                    });
                if (scheduled != scheduledTasks_.end() &&
                    scheduled->due <= now) {
                    task = std::move(scheduled->task);
                    scheduledTasks_.erase(scheduled);
                    ++scheduledTasksRevision_;
                    break;
                }

                auto waitDuration = kFileTransferStallCheckInterval;
                if (scheduled != scheduledTasks_.end()) {
                    waitDuration = std::min(
                        waitDuration,
                        std::chrono::duration_cast<std::chrono::milliseconds>(
                            scheduled->due - now));
                    if (waitDuration <= std::chrono::milliseconds::zero()) {
                        waitDuration = std::chrono::milliseconds(1);
                    }
                }
                const auto revision = scheduledTasksRevision_;
                workAvailable_.wait_for(
                    lock, stopToken, waitDuration,
                    [this, revision] {
                        return stopping_ || !workQueue_.empty() ||
                               scheduledTasksRevision_ != revision;
                    });
                if (scheduledTasksRevision_ == revision &&
                    workQueue_.empty()) {
                    break;
                }
            }
        }
        if (shouldStop || stopToken.stop_requested()) {
            break;
        }
        if (task) {
            task();
        }
        CheckStalledTransfers();
    }
}

bool FileTransferController::Post(std::function<void()> task)
{
    {
        std::lock_guard lock(mutex_);
        if (stopping_) {
            return false;
        }
        workQueue_.push_back(std::move(task));
    }
    workAvailable_.notify_one();
    return true;
}

bool FileTransferController::PostPriority(std::function<void()> task)
{
    {
        std::lock_guard lock(mutex_);
        if (stopping_) {
            return false;
        }
        workQueue_.push_front(std::move(task));
    }
    workAvailable_.notify_one();
    return true;
}

bool FileTransferController::PostAfter(
    std::chrono::milliseconds delay,
    std::function<void()> task)
{
    {
        std::lock_guard lock(mutex_);
        if (stopping_) {
            return false;
        }
        ScheduledTask scheduled;
        scheduled.due = std::chrono::steady_clock::now() + delay;
        scheduled.order = ++nextScheduledTaskOrder_;
        scheduled.task = std::move(task);
        scheduledTasks_.push_back(std::move(scheduled));
        ++scheduledTasksRevision_;
    }
    workAvailable_.notify_one();
    return true;
}

void FileTransferController::ResetRateLocked(TransferRecord& transfer)
{
    const auto now = std::chrono::steady_clock::now();
    transfer.rateSamples.clear();
    transfer.rateSamples.push_back(
        TransferRecord::RateSample{now, transfer.snapshot.transferredBytes});
    transfer.lastRatePublishedAt = now;
    transfer.snapshot.bytesPerSecond = 0.0;
    transfer.snapshot.estimatedRemainingSeconds = 0;
    transfer.lastProgressAt = now;
    transfer.sendBackpressured = false;
}

void FileTransferController::UpdateProgressLocked(
    TransferRecord& transfer,
    std::uint64_t transferredBytes)
{
    const auto now = std::chrono::steady_clock::now();
    transfer.snapshot.transferredBytes = transferredBytes;
    transfer.lastProgressAt = now;
    transfer.sendBackpressured = false;
    transfer.resyncAttempts = 0;
    if (transfer.rateSamples.empty() ||
        transfer.rateSamples.back().bytes != transferredBytes) {
        transfer.rateSamples.push_back(
            TransferRecord::RateSample{now, transferredBytes});
    }
    RefreshRateLocked(transfer, now);
}

bool FileTransferController::RefreshRateLocked(
    TransferRecord& transfer,
    std::chrono::steady_clock::time_point now)
{
    if (transfer.snapshot.state != FileTransferState::kTransferring) {
        return false;
    }
    if (transfer.rateSamples.empty()) {
        transfer.rateSamples.push_back(TransferRecord::RateSample{
            now, transfer.snapshot.transferredBytes});
        transfer.lastRatePublishedAt = now;
        return false;
    }
    if (now - transfer.rateSamples.back().at >=
        kFileTransferRateRefreshInterval) {
        transfer.rateSamples.push_back(TransferRecord::RateSample{
            now, transfer.snapshot.transferredBytes});
    }
    const auto cutoff = now - kFileTransferRateWindow;
    while (transfer.rateSamples.size() > 1 &&
           transfer.rateSamples[1].at <= cutoff) {
        transfer.rateSamples.pop_front();
    }
    if (now - transfer.lastRatePublishedAt <
        kFileTransferRateRefreshInterval) {
        return false;
    }
    transfer.lastRatePublishedAt = now;
    const auto& baseline = transfer.rateSamples.front();
    const double elapsedSeconds =
        std::chrono::duration<double>(now - baseline.at).count();
    const std::uint64_t currentBytes = transfer.snapshot.transferredBytes;
    const double bytesPerSecond =
        elapsedSeconds > 0.0 && currentBytes >= baseline.bytes
            ? static_cast<double>(currentBytes - baseline.bytes) /
                  elapsedSeconds
            : 0.0;
    transfer.snapshot.bytesPerSecond = bytesPerSecond;
    if (bytesPerSecond > 0.0 &&
        currentBytes < transfer.snapshot.fileSize) {
        const double remaining = static_cast<double>(
            transfer.snapshot.fileSize - currentBytes);
        transfer.snapshot.estimatedRemainingSeconds =
            static_cast<std::uint64_t>(
                std::ceil(remaining / bytesPerSecond));
    } else {
        transfer.snapshot.estimatedRemainingSeconds = 0;
    }
    return true;
}
