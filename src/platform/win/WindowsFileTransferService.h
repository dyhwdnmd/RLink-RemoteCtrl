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

namespace remote {

struct FileTransferSourceInfo {
    std::filesystem::path path;
    std::string fileName;
    std::uint64_t fileSize = 0;
    std::array<std::uint8_t, 32> sha256{};
};

struct FileTransferDestination {
    std::filesystem::path finalPath;
    std::filesystem::path temporaryPath;
    std::uint64_t resumeOffset = 0;
    bool replaceExisting = false;
};

// Performs blocking file operations. Callers must run these methods on a file
// worker thread, never on a Qt or WebRTC callback thread.
class WindowsFileTransferService final {
public:
    WindowsFileTransferService();
    ~WindowsFileTransferService();

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
    struct Impl;
    bool CalculateSha256(const std::filesystem::path& path,
                         std::array<std::uint8_t, 32>* digest,
                         std::string* error) const;
    std::unique_ptr<Impl> impl_;
};

}  // namespace remote
