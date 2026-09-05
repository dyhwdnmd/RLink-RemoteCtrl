// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#include "FileTransferStorage.h"

#include <utility>

namespace remote::app {

FileTransferStorage::FileTransferStorage(
    std::unique_ptr<WindowsFileTransferService> service)
    : service_(std::move(service))
{}

bool FileTransferStorage::InspectSource(
    const std::filesystem::path& path,
    FileTransferSourceInfo* output,
    std::string* error) const
{
    return service_->InspectSource(path, output, error);
}

bool FileTransferStorage::ReadChunk(
    const std::filesystem::path& path,
    std::uint64_t offset,
    std::size_t maximumBytes,
    std::vector<std::uint8_t>* output,
    std::string* error)
{
    return service_->ReadChunk(
        path, offset, maximumBytes, output, error);
}

void FileTransferStorage::CloseSource(
    const std::filesystem::path& path)
{
    service_->CloseSource(path);
}

bool FileTransferStorage::PrepareDestination(
    const std::filesystem::path& directory,
    const std::string& fileName,
    const std::string& transferId,
    std::uint64_t expectedSize,
    bool replaceExisting,
    FileTransferDestination* output,
    std::string* error) const
{
    return service_->PrepareDestination(
        directory, fileName, transferId, expectedSize,
        replaceExisting, output, error);
}

bool FileTransferStorage::AppendChunk(
    const std::filesystem::path& temporaryPath,
    std::uint64_t expectedOffset,
    std::span<const std::uint8_t> bytes,
    std::string* error)
{
    return service_->AppendChunk(
        temporaryPath, expectedOffset, bytes, error);
}

bool FileTransferStorage::VerifyAndCommit(
    const FileTransferDestination& destination,
    std::uint64_t expectedSize,
    const std::array<std::uint8_t, 32>& expectedSha256,
    std::string* error)
{
    return service_->VerifyAndCommit(
        destination, expectedSize, expectedSha256, error);
}

void FileTransferStorage::CloseDestination(
    const std::filesystem::path& temporaryPath)
{
    service_->CloseDestination(temporaryPath);
}

void FileTransferStorage::RemovePartial(
    const std::filesystem::path& temporaryPath)
{
    service_->RemovePartial(temporaryPath);
}

}  // namespace remote::app
