// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#pragma once

#include <chrono>
#include <condition_variable>
#include <cstdint>
#include <filesystem>
#include <functional>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

namespace remote::app {

struct ClipboardCacheConfiguration {
    // The user selects this parent. RemoteC always appends its own dedicated
    // RemoteC/ClipboardCache directory and never removes arbitrary siblings.
    std::filesystem::path baseDirectory;
    std::chrono::minutes retention{60};
    std::uint64_t requestedLimitBytes = 2ull * 1024 * 1024 * 1024;
};

struct ClipboardCacheSnapshot {
    std::filesystem::path root;
    std::uint64_t totalBytes = 0;
    std::uint64_t entryCount = 0;
    std::uint64_t availableBytes = 0;
    std::uint64_t safeCapacityBytes = 0;
    std::uint64_t effectiveLimitBytes = 0;
    std::string lastError;
};

class ClipboardCacheManager final {
public:
    ClipboardCacheManager();
    ~ClipboardCacheManager();

    ClipboardCacheManager(const ClipboardCacheManager&) = delete;
    ClipboardCacheManager& operator=(const ClipboardCacheManager&) = delete;

    void SetConfiguration(ClipboardCacheConfiguration configuration);
    ClipboardCacheSnapshot Snapshot() const;

    bool PrepareEntry(const std::string& itemId,
                      std::uint64_t incomingBytes,
                      std::filesystem::path* entryDirectory,
                      std::string* error);
    void ProtectEntry(const std::filesystem::path& entryDirectory);
    void MarkCompleted(const std::filesystem::path& entryDirectory);
    void RetireCurrentEntry();
    void RemoveEntry(const std::filesystem::path& entryDirectory);
    // Detaches an incomplete entry synchronously, but performs recursive
    // deletion only on the maintenance thread. Cancel must never make the
    // clipboard/video worker walk hundreds of cached files.
    void RequestRemoveEntry(const std::filesystem::path& entryDirectory);
    void RequestCleanup(bool aggressive = false,
                        std::function<void()> completion = {});

private:
    void MaintenanceMain(std::stop_token stopToken);
    ClipboardCacheSnapshot ScanLocked() const;
    std::filesystem::path RootLocked() const;
    std::uint64_t EffectiveLimitLocked(
        std::uint64_t availableBytes) const;
    bool CleanupLocked(std::uint64_t incomingBytes,
                       bool aggressive,
                       std::string* error);
    bool IsManagedEntryLocked(const std::filesystem::path& path) const;
    bool IsProtectedLocked(const std::filesystem::path& path) const;
    void WriteMarkerLocked(const std::filesystem::path& entry,
                           const char* state) const;
    void TouchRetiredLocked(const std::filesystem::path& entry) const;

    mutable std::mutex mutex_;
    std::condition_variable_any wake_;
    ClipboardCacheConfiguration configuration_;
    std::vector<std::filesystem::path> retiredRoots_;
    std::vector<std::filesystem::path> pendingRemovals_;
    std::vector<std::function<void()>> cleanupCallbacks_;
    std::filesystem::path receivingEntry_;
    std::filesystem::path currentClipboardEntry_;
    mutable ClipboardCacheSnapshot lastSnapshot_;
    bool cleanupRequested_ = false;
    bool aggressiveCleanupRequested_ = false;
    bool stopping_ = false;
    std::jthread maintenance_;
};

}  // namespace remote::app
