// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#include "ClipboardCacheManager.h"

#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <shlobj.h>

#include <algorithm>
#include <fstream>
#include <limits>
#include <vector>

namespace remote::app {
namespace {

constexpr std::uint64_t kGiB = 1024ull * 1024 * 1024;
constexpr std::uint64_t kReserveBytes = 256ull * 1024 * 1024;
constexpr auto kMinimumRetiredGrace = std::chrono::minutes(10);
constexpr auto kMaintenanceInterval = std::chrono::minutes(10);
constexpr wchar_t kMarkerName[] = L".remotec-cache";

std::filesystem::path DefaultBaseDirectory()
{
    PWSTR localAppData = nullptr;
    std::filesystem::path result;
    if (SUCCEEDED(SHGetKnownFolderPath(FOLDERID_LocalAppData,
            KF_FLAG_CREATE, nullptr, &localAppData)) && localAppData) {
        result = localAppData;
        CoTaskMemFree(localAppData);
    }
    if (result.empty()) {
        std::error_code error;
        result = std::filesystem::temp_directory_path(error);
    }
    return result;
}

std::uint64_t DirectoryBytes(const std::filesystem::path& root)
{
    std::uint64_t bytes = 0;
    std::error_code error;
    for (std::filesystem::recursive_directory_iterator iterator(
             root, std::filesystem::directory_options::skip_permission_denied,
             error), end;
         !error && iterator != end; iterator.increment(error)) {
        if (!iterator->is_regular_file(error) || error) {
            error.clear();
            continue;
        }
        const auto size = iterator->file_size(error);
        if (!error && bytes <= (std::numeric_limits<std::uint64_t>::max)() -
                size) {
            bytes += size;
        }
        error.clear();
    }
    return bytes;
}

bool StartsWithManagedPrefix(const std::filesystem::path& path)
{
    const auto name = path.filename().wstring();
    return name.starts_with(L"item-") ||
        name.starts_with(L"current-");
}

}  // namespace

ClipboardCacheManager::ClipboardCacheManager()
{
    configuration_.baseDirectory = DefaultBaseDirectory();
    maintenance_ = std::jthread([this](std::stop_token stopToken) {
        MaintenanceMain(stopToken);
    });
}

ClipboardCacheManager::~ClipboardCacheManager()
{
    {
        std::lock_guard lock(mutex_);
        stopping_ = true;
    }
    wake_.notify_all();
    maintenance_.request_stop();
    if (maintenance_.joinable()) maintenance_.join();
}

void ClipboardCacheManager::SetConfiguration(
    ClipboardCacheConfiguration configuration)
{
    if (configuration.baseDirectory.empty()) {
        configuration.baseDirectory = DefaultBaseDirectory();
    }
    configuration.retention = (std::max)(
        configuration.retention, kMinimumRetiredGrace);
    {
        std::lock_guard lock(mutex_);
        const auto oldRoot = RootLocked().lexically_normal();
        configuration_ = std::move(configuration);
        const auto newRoot = RootLocked().lexically_normal();
        if (!oldRoot.empty() && oldRoot != newRoot &&
            std::find(retiredRoots_.begin(), retiredRoots_.end(), oldRoot) ==
                retiredRoots_.end()) {
            retiredRoots_.push_back(oldRoot);
        }
        cleanupRequested_ = true;
    }
    wake_.notify_all();
}

ClipboardCacheSnapshot ClipboardCacheManager::Snapshot() const
{
    std::lock_guard lock(mutex_);
    lastSnapshot_ = ScanLocked();
    return lastSnapshot_;
}

bool ClipboardCacheManager::PrepareEntry(
    const std::string& itemId, std::uint64_t incomingBytes,
    std::filesystem::path* entryDirectory, std::string* error)
{
    if (!entryDirectory || itemId.empty() ||
        itemId.find_first_of("/\\:") != std::string::npos ||
        itemId.find("..") != std::string::npos) {
        if (error) *error = "The clipboard cache item id is invalid.";
        return false;
    }
    std::lock_guard lock(mutex_);
    std::string cleanupError;
    if (!CleanupLocked(incomingBytes, false, &cleanupError)) {
        if (error) *error = cleanupError;
        return false;
    }
    const auto root = RootLocked();
    std::error_code filesystemError;
    std::filesystem::create_directories(root, filesystemError);
    if (filesystemError) {
        if (error) *error = "The clipboard cache directory is unavailable.";
        return false;
    }
    const auto entry = root / std::filesystem::path(itemId);
    std::filesystem::remove_all(entry, filesystemError);
    filesystemError.clear();
    std::filesystem::create_directories(entry, filesystemError);
    if (filesystemError) {
        if (error) *error = "The clipboard cache entry could not be created.";
        return false;
    }
    receivingEntry_ = entry;
    WriteMarkerLocked(entry, "receiving");
    *entryDirectory = entry;
    lastSnapshot_ = ScanLocked();
    return true;
}

void ClipboardCacheManager::ProtectEntry(
    const std::filesystem::path& entryDirectory)
{
    std::lock_guard lock(mutex_);
    currentClipboardEntry_ = entryDirectory;
}

void ClipboardCacheManager::MarkCompleted(
    const std::filesystem::path& entryDirectory)
{
    std::lock_guard lock(mutex_);
    if (!currentClipboardEntry_.empty() &&
        currentClipboardEntry_ != entryDirectory) {
        TouchRetiredLocked(currentClipboardEntry_);
    }
    receivingEntry_.clear();
    currentClipboardEntry_ = entryDirectory;
    WriteMarkerLocked(entryDirectory, "completed");
    lastSnapshot_ = ScanLocked();
}

void ClipboardCacheManager::RetireCurrentEntry()
{
    std::lock_guard lock(mutex_);
    if (currentClipboardEntry_.empty()) return;
    TouchRetiredLocked(currentClipboardEntry_);
    currentClipboardEntry_.clear();
    cleanupRequested_ = true;
    wake_.notify_all();
}

void ClipboardCacheManager::RemoveEntry(
    const std::filesystem::path& entryDirectory)
{
    std::lock_guard lock(mutex_);
    if (entryDirectory.empty() || !IsManagedEntryLocked(entryDirectory)) {
        return;
    }
    if (receivingEntry_ == entryDirectory) receivingEntry_.clear();
    if (currentClipboardEntry_ == entryDirectory) {
        currentClipboardEntry_.clear();
    }
    std::error_code ignored;
    std::filesystem::remove_all(entryDirectory, ignored);
    lastSnapshot_ = ScanLocked();
}

void ClipboardCacheManager::RequestRemoveEntry(
    const std::filesystem::path& entryDirectory)
{
    {
        std::lock_guard lock(mutex_);
        if (entryDirectory.empty() ||
            !IsManagedEntryLocked(entryDirectory)) {
            return;
        }
        if (receivingEntry_ == entryDirectory) receivingEntry_.clear();
        if (currentClipboardEntry_ == entryDirectory) {
            currentClipboardEntry_.clear();
        }
        if (std::find(pendingRemovals_.begin(), pendingRemovals_.end(),
                entryDirectory) == pendingRemovals_.end()) {
            pendingRemovals_.push_back(entryDirectory);
        }
        cleanupRequested_ = true;
    }
    wake_.notify_all();
}

void ClipboardCacheManager::RequestCleanup(
    bool aggressive, std::function<void()> completion)
{
    {
        std::lock_guard lock(mutex_);
        cleanupRequested_ = true;
        aggressiveCleanupRequested_ |= aggressive;
        if (completion) {
            cleanupCallbacks_.push_back(std::move(completion));
        }
    }
    wake_.notify_all();
}

void ClipboardCacheManager::MaintenanceMain(std::stop_token stopToken)
{
    while (!stopToken.stop_requested()) {
        bool aggressive = false;
        std::vector<std::filesystem::path> removals;
        std::vector<std::function<void()>> callbacks;
        {
            std::unique_lock lock(mutex_);
            wake_.wait_for(lock, stopToken, kMaintenanceInterval, [this] {
                return stopping_ || cleanupRequested_;
            });
            if (stopping_) return;
            aggressive = aggressiveCleanupRequested_;
            cleanupRequested_ = false;
            aggressiveCleanupRequested_ = false;
            // Keep these entries marked protected until recursive deletion
            // finishes. A new paste may call PrepareEntry concurrently; it
            // must not steal this cleanup and block its clipboard worker.
            removals = pendingRemovals_;
            callbacks.swap(cleanupCallbacks_);
        }
        for (const auto& path : removals) {
            std::error_code ignored;
            std::filesystem::remove_all(path, ignored);
        }
        {
            std::lock_guard lock(mutex_);
            for (const auto& removed : removals) {
                std::erase(pendingRemovals_, removed);
            }
            std::string ignored;
            (void)CleanupLocked(0, aggressive, &ignored);
            lastSnapshot_ = ScanLocked();
        }
        for (auto& callback : callbacks) {
            if (callback) callback();
        }
    }
}

ClipboardCacheSnapshot ClipboardCacheManager::ScanLocked() const
{
    ClipboardCacheSnapshot snapshot;
    snapshot.root = RootLocked();
    std::error_code error;
    std::filesystem::create_directories(snapshot.root, error);
    if (error) {
        snapshot.lastError = "The clipboard cache directory is unavailable.";
        return snapshot;
    }
    const auto space = std::filesystem::space(snapshot.root, error);
    if (!error) snapshot.availableBytes = space.available;
    error.clear();
    for (std::filesystem::directory_iterator iterator(
             snapshot.root,
             std::filesystem::directory_options::skip_permission_denied,
             error), end;
         !error && iterator != end; iterator.increment(error)) {
        if (!iterator->is_directory(error) || error ||
            !IsManagedEntryLocked(iterator->path())) {
            error.clear();
            continue;
        }
        ++snapshot.entryCount;
        snapshot.totalBytes += DirectoryBytes(iterator->path());
    }
    snapshot.safeCapacityBytes =
        (snapshot.availableBytes / 2 / kGiB) * kGiB;
    snapshot.effectiveLimitBytes =
        EffectiveLimitLocked(snapshot.availableBytes);
    return snapshot;
}

std::filesystem::path ClipboardCacheManager::RootLocked() const
{
    return configuration_.baseDirectory / L"RemoteC" /
        L"ClipboardCache";
}

std::uint64_t ClipboardCacheManager::EffectiveLimitLocked(
    std::uint64_t capacityPoolBytes) const
{
    const std::uint64_t safe =
        (capacityPoolBytes / 2 / kGiB) * kGiB;
    if (configuration_.requestedLimitBytes == 0) return safe;
    return (std::min)(safe, configuration_.requestedLimitBytes);
}

bool ClipboardCacheManager::CleanupLocked(
    std::uint64_t incomingBytes, bool aggressive, std::string* error)
{
    const auto root = RootLocked();
    std::error_code filesystemError;
    std::filesystem::create_directories(root, filesystemError);
    if (filesystemError) {
        if (error) *error = "The clipboard cache directory is unavailable.";
        return false;
    }
    auto snapshot = ScanLocked();
    const std::uint64_t effectiveLimit = snapshot.effectiveLimitBytes;
    if (effectiveLimit < kGiB && incomingBytes != 0) {
        if (error) {
            *error = "The selected clipboard cache disk has less than 2 GiB of safe free space.";
        }
        return false;
    }
    if (incomingBytes > effectiveLimit) {
        if (error) {
            *error = "The incoming clipboard item exceeds the safe cache capacity of the selected disk.";
        }
        return false;
    }

    struct Candidate {
        std::filesystem::path path;
        std::filesystem::file_time_type modified;
        std::uint64_t bytes = 0;
        bool incomplete = false;
    };
    std::vector<Candidate> candidates;
    const auto now = std::filesystem::file_time_type::clock::now();

    // A path change applies only to new transfers. Old roots are retained
    // until their entries expire so Windows/QQ can finish reading paths that
    // were already published before the switch.
    for (const auto& retiredRoot : retiredRoots_) {
        std::error_code retiredError;
        for (std::filesystem::directory_iterator iterator(
                 retiredRoot,
                 std::filesystem::directory_options::skip_permission_denied,
                 retiredError), end;
             !retiredError && iterator != end;
             iterator.increment(retiredError)) {
            if (!iterator->is_directory(retiredError) || retiredError ||
                !IsManagedEntryLocked(iterator->path()) ||
                IsProtectedLocked(iterator->path())) {
                retiredError.clear();
                continue;
            }
            const auto modified = iterator->last_write_time(retiredError);
            if (retiredError) {
                retiredError.clear();
                continue;
            }
            if (aggressive ||
                now - modified >= configuration_.retention) {
                std::error_code ignored;
                std::filesystem::remove_all(iterator->path(), ignored);
            }
        }
    }
    for (std::filesystem::directory_iterator iterator(
             root, std::filesystem::directory_options::skip_permission_denied,
             filesystemError), end;
         !filesystemError && iterator != end;
         iterator.increment(filesystemError)) {
        if (!iterator->is_directory(filesystemError) || filesystemError ||
            !IsManagedEntryLocked(iterator->path()) ||
            IsProtectedLocked(iterator->path())) {
            filesystemError.clear();
            continue;
        }
        Candidate candidate;
        candidate.path = iterator->path();
        candidate.modified = iterator->last_write_time(filesystemError);
        if (filesystemError) {
            filesystemError.clear();
            continue;
        }
        candidate.bytes = DirectoryBytes(candidate.path);
        std::ifstream marker(candidate.path / kMarkerName);
        std::string state;
        if (marker) std::getline(marker, state);
        candidate.incomplete = state == "receiving";
        const auto age = now - candidate.modified;
        const bool expired = age >= configuration_.retention;
        const bool manualEligible = aggressive;
        if (candidate.incomplete || expired || manualEligible ||
            incomingBytes != 0) {
            candidates.push_back(std::move(candidate));
        }
    }
    std::sort(candidates.begin(), candidates.end(),
        [](const Candidate& left, const Candidate& right) {
            if (left.incomplete != right.incomplete) {
                return left.incomplete;
            }
            return left.modified < right.modified;
        });

    const std::uint64_t projected = snapshot.totalBytes + incomingBytes;
    const std::uint64_t targetAfterIncoming =
        effectiveLimit * 8 / 10;
    const std::uint64_t capacityNeed = projected > effectiveLimit
        ? projected - targetAfterIncoming : 0;
    const std::uint64_t diskNeed = incomingBytes + kReserveBytes >
            snapshot.availableBytes
        ? incomingBytes + kReserveBytes - snapshot.availableBytes : 0;
    const std::uint64_t required = aggressive
        ? snapshot.totalBytes
        : (std::max)(capacityNeed, diskNeed);

    std::uint64_t freed = 0;
    for (const auto& candidate : candidates) {
        const auto age = now - candidate.modified;
        const bool normalExpiry = candidate.incomplete ||
            age >= configuration_.retention;
        const bool canDeleteForSpace = incomingBytes != 0 &&
            age >= kMinimumRetiredGrace;
        if (!normalExpiry && !aggressive && !canDeleteForSpace) continue;
        if (!aggressive && required != 0 && freed >= required &&
            !normalExpiry) break;
        std::error_code removeError;
        std::filesystem::remove_all(candidate.path, removeError);
        if (!removeError) freed += candidate.bytes;
    }

    snapshot = ScanLocked();
    if (incomingBytes != 0 &&
        (snapshot.totalBytes + incomingBytes > effectiveLimit ||
         snapshot.availableBytes < incomingBytes + kReserveBytes)) {
        if (error) {
            *error = "There is not enough eligible cache space for this paste. The current clipboard item and recently used files were kept protected.";
        }
        return false;
    }
    return true;
}

bool ClipboardCacheManager::IsManagedEntryLocked(
    const std::filesystem::path& path) const
{
    const auto parent = path.parent_path().lexically_normal();
    const bool currentRoot = parent == RootLocked().lexically_normal();
    const bool retiredRoot = std::find(
        retiredRoots_.begin(), retiredRoots_.end(), parent) !=
            retiredRoots_.end();
    if ((!currentRoot && !retiredRoot) || !StartsWithManagedPrefix(path)) {
        return false;
    }
    // Legacy item-* directories created before the marker was introduced are
    // accepted only as direct children of RemoteC's dedicated cache root.
    return true;
}

bool ClipboardCacheManager::IsProtectedLocked(
    const std::filesystem::path& path) const
{
    const auto normalized = path.lexically_normal();
    const bool pendingRemoval = std::any_of(
        pendingRemovals_.begin(), pendingRemovals_.end(),
        [&normalized](const std::filesystem::path& candidate) {
            return normalized == candidate.lexically_normal();
        });
    return pendingRemoval || (!receivingEntry_.empty() &&
            normalized == receivingEntry_.lexically_normal()) ||
        (!currentClipboardEntry_.empty() &&
            normalized == currentClipboardEntry_.lexically_normal());
}

void ClipboardCacheManager::WriteMarkerLocked(
    const std::filesystem::path& entry, const char* state) const
{
    std::ofstream marker(entry / kMarkerName,
                         std::ios::binary | std::ios::trunc);
    if (marker) marker << state << '\n';
}

void ClipboardCacheManager::TouchRetiredLocked(
    const std::filesystem::path& entry) const
{
    if (!IsManagedEntryLocked(entry)) return;
    WriteMarkerLocked(entry, "retired");
    std::error_code ignored;
    std::filesystem::last_write_time(
        entry, std::filesystem::file_time_type::clock::now(), ignored);
}

}  // namespace remote::app
