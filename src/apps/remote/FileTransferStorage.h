// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <memory>
#include <span>
#include <string>
#include <vector>

#include "src/platform/win/WindowsFileTransferService.h"

namespace remote::app {

// Blocking persistence boundary used only by the controller's file worker.
// It centralizes source validation, resumable partial files, and atomic commit.
class FileTransferStorage final {
public:
    explicit FileTransferStorage(
        std::unique_ptr<WindowsFileTransferService> service);

    bool InspectSource(const std::filesystem::path& path,
                       FileTransferSourceInfo* output,
                       std::string* error) const;
    bool ReadChunk(const std::filesystem::path& path,
                   std::uint64_t offset,
                   std::size_t maximumBytes,
                   std::vector<std::uint8_t>* output,
                   std::string* error);
    void CloseSource(const std::filesystem::path& path);
    bool PrepareDestination(const std::filesystem::path& directory,
                            const std::string& fileName,
                            const std::string& transferId,
                            std::uint64_t expectedSize,
                            bool replaceExisting,
                            FileTransferDestination* output,
                            std::string* error) const;
    bool AppendChunk(const std::filesystem::path& temporaryPath,
                     std::uint64_t expectedOffset,
                     std::span<const std::uint8_t> bytes,
                     std::string* error);
    bool VerifyAndCommit(
        const FileTransferDestination& destination,
        std::uint64_t expectedSize,
        const std::array<std::uint8_t, 32>& expectedSha256,
        std::string* error);
    void CloseDestination(const std::filesystem::path& temporaryPath);
    void RemovePartial(const std::filesystem::path& temporaryPath);

private:
    std::unique_ptr<WindowsFileTransferService> service_;
};

}  // namespace remote::app
