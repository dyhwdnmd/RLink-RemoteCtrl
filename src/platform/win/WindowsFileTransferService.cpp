// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#include "WindowsFileTransferService.h"

#include <Windows.h>
#include <bcrypt.h>

#include <algorithm>
#include <chrono>
#include <cwctype>
#include <limits>
#include <memory>
#include <thread>
#include <system_error>
#include <unordered_map>

#include "src/protocol/FileTransferProtocol.h"

namespace remote {
namespace {

struct HandleCloser {
    void operator()(void* handle) const
    {
        if (handle && handle != INVALID_HANDLE_VALUE) {
            CloseHandle(handle);
        }
    }
};

using UniqueHandle = std::unique_ptr<void, HandleCloser>;

constexpr int kDestinationOpenRetryCount = 5;
constexpr auto kDestinationOpenRetryDelay = std::chrono::milliseconds(20);

void SetError(std::string* error, std::string message)
{
    if (error) {
        *error = std::move(message);
    }
}

std::string WindowsError(const char* operation, DWORD code)
{
    return std::string(operation) + " failed with Windows error " +
           std::to_string(code) + ".";
}

std::string WideToUtf8(const std::wstring& value)
{
    if (value.empty()) {
        return {};
    }
    const int size = WideCharToMultiByte(
        CP_UTF8, WC_ERR_INVALID_CHARS, value.data(),
        static_cast<int>(value.size()), nullptr, 0, nullptr, nullptr);
    if (size <= 0) {
        return {};
    }
    std::string result(static_cast<std::size_t>(size), '\0');
    if (WideCharToMultiByte(
            CP_UTF8, WC_ERR_INVALID_CHARS, value.data(),
            static_cast<int>(value.size()), result.data(), size,
            nullptr, nullptr) != size) {
        return {};
    }
    return result;
}

std::wstring Utf8ToWide(const std::string& value)
{
    if (value.empty()) {
        return {};
    }
    const int size = MultiByteToWideChar(
        CP_UTF8, MB_ERR_INVALID_CHARS, value.data(),
        static_cast<int>(value.size()), nullptr, 0);
    if (size <= 0) {
        return {};
    }
    std::wstring result(static_cast<std::size_t>(size), L'\0');
    if (MultiByteToWideChar(
            CP_UTF8, MB_ERR_INVALID_CHARS, value.data(),
            static_cast<int>(value.size()), result.data(), size) != size) {
        return {};
    }
    return result;
}

bool IsReservedWindowsFileName(const std::wstring& fileName)
{
    std::wstring base = fileName.substr(0, fileName.find(L'.'));
    std::transform(base.begin(), base.end(), base.begin(),
                   [](wchar_t value) {
                       return static_cast<wchar_t>(std::towupper(value));
                   });
    if (base == L"CON" || base == L"PRN" || base == L"AUX" ||
        base == L"NUL") {
        return true;
    }
    if (base.size() == 4 &&
        (base.starts_with(L"COM") || base.starts_with(L"LPT")) &&
        base[3] >= L'1' && base[3] <= L'9') {
        return true;
    }
    return false;
}

bool GetFileSize(HANDLE file, std::uint64_t* size)
{
    LARGE_INTEGER value{};
    if (!size || !GetFileSizeEx(file, &value) || value.QuadPart < 0) {
        return false;
    }
    *size = static_cast<std::uint64_t>(value.QuadPart);
    return true;
}

bool SeekFile(HANDLE file, std::uint64_t offset)
{
    if (offset > static_cast<std::uint64_t>(
                     std::numeric_limits<LONGLONG>::max())) {
        return false;
    }
    LARGE_INTEGER position{};
    position.QuadPart = static_cast<LONGLONG>(offset);
    return SetFilePointerEx(file, position, nullptr, FILE_BEGIN) != FALSE;
}

}  // namespace

struct WindowsFileTransferService::Impl {
    struct OpenSource {
        UniqueHandle file;
        std::uint64_t size = 0;
        std::uint64_t offset = 0;
    };

    struct OpenDestination {
        UniqueHandle file;
        std::uint64_t offset = 0;
    };

    std::unordered_map<std::wstring, OpenSource> sources;
    std::unordered_map<std::wstring, OpenDestination> destinations;
};

WindowsFileTransferService::WindowsFileTransferService()
    : impl_(std::make_unique<Impl>())
{}

WindowsFileTransferService::~WindowsFileTransferService() = default;

bool WindowsFileTransferService::InspectSource(
    const std::filesystem::path& path,
    FileTransferSourceInfo* output,
    std::string* error) const
{
    if (!output || path.empty()) {
        SetError(error, "A source file is required.");
        return false;
    }
    UniqueHandle file(CreateFileW(
        path.c_str(), GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, nullptr));
    if (file.get() == INVALID_HANDLE_VALUE) {
        SetError(error, "The source file could not be opened.");
        return false;
    }
    const DWORD attributes = GetFileAttributesW(path.c_str());
    std::uint64_t size = 0;
    if (attributes == INVALID_FILE_ATTRIBUTES ||
        (attributes & FILE_ATTRIBUTE_DIRECTORY) != 0 ||
        !GetFileSize(file.get(), &size) ||
        size > kMaximumFileSizeBytes) {
        SetError(error, "The source file is not a supported regular file.");
        return false;
    }
    const std::string fileName = WideToUtf8(path.filename().wstring());
    if (fileName.empty() || fileName.size() > 255) {
        SetError(error, "The source file name is not valid UTF-8 or is too long.");
        return false;
    }
    FileTransferSourceInfo inspected;
    inspected.path = path;
    inspected.fileName = fileName;
    inspected.fileSize = size;
    if (!CalculateSha256(path, &inspected.sha256, error)) {
        return false;
    }
    *output = std::move(inspected);
    return true;
}

bool WindowsFileTransferService::ReadChunk(
    const std::filesystem::path& path,
    std::uint64_t offset,
    std::size_t maximumBytes,
    std::vector<std::uint8_t>* output,
    std::string* error)
{
    if (!output || maximumBytes == 0 ||
        maximumBytes > kMaximumFileChunkBytes) {
        SetError(error, "The requested file chunk size is invalid.");
        return false;
    }
    const auto key = path.wstring();
    auto found = impl_->sources.find(key);
    if (found == impl_->sources.end()) {
        UniqueHandle file(CreateFileW(
            path.c_str(), GENERIC_READ,
            FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, nullptr,
            OPEN_EXISTING,
            FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, nullptr));
        if (file.get() == INVALID_HANDLE_VALUE) {
            SetError(error, WindowsError("Opening the source file",
                                         GetLastError()));
            return false;
        }
        std::uint64_t size = 0;
        if (!GetFileSize(file.get(), &size)) {
            SetError(error, WindowsError("Reading the source file size",
                                         GetLastError()));
            return false;
        }
        Impl::OpenSource source;
        source.file = std::move(file);
        source.size = size;
        found = impl_->sources.emplace(key, std::move(source)).first;
    }
    auto& source = found->second;
    if (offset >= source.size) {
        SetError(error, "The source file offset is outside the file.");
        return false;
    }
    if (source.offset != offset) {
        if (!SeekFile(source.file.get(), offset)) {
            SetError(error, WindowsError("Seeking the source file",
                                         GetLastError()));
            return false;
        }
        source.offset = offset;
    }
    const std::size_t wanted = static_cast<std::size_t>(
        std::min<std::uint64_t>(maximumBytes, source.size - offset));
    output->assign(wanted, 0);
    DWORD read = 0;
    if (!ReadFile(source.file.get(), output->data(), static_cast<DWORD>(wanted),
                  &read, nullptr) || read != wanted) {
        output->clear();
        SetError(error, "The source file chunk could not be read.");
        return false;
    }
    source.offset += read;
    return true;
}

void WindowsFileTransferService::CloseSource(
    const std::filesystem::path& path)
{
    if (!path.empty()) {
        impl_->sources.erase(path.wstring());
    }
}

bool WindowsFileTransferService::PrepareDestination(
    const std::filesystem::path& directory,
    const std::string& fileName,
    const std::string& transferId,
    std::uint64_t expectedSize,
    bool replaceExisting,
    FileTransferDestination* output,
    std::string* error) const
{
    if (!output || directory.empty() || transferId.empty() ||
        expectedSize > kMaximumFileSizeBytes) {
        SetError(error, "The destination request is invalid.");
        return false;
    }
    const std::wstring wideName = Utf8ToWide(fileName);
    const std::filesystem::path requested(wideName);
    if (wideName.empty() || requested.filename() != requested ||
        wideName.back() == L' ' || wideName.back() == L'.' ||
        IsReservedWindowsFileName(wideName)) {
        SetError(error, "The destination file name is not allowed on Windows.");
        return false;
    }

    std::error_code ec;
    const auto root = std::filesystem::weakly_canonical(directory, ec);
    if (ec || !std::filesystem::is_directory(root, ec) || ec) {
        SetError(error, "The destination directory does not exist.");
        return false;
    }

    FileTransferDestination prepared;
    prepared.finalPath = root / requested;
    const bool finalExists = std::filesystem::exists(prepared.finalPath, ec);
    if (ec) {
        SetError(error, "The destination file could not be inspected.");
        return false;
    }
    if (finalExists && !replaceExisting) {
        SetError(error, "A file with the same name already exists in the "
                        "destination directory.");
        return false;
    }
    prepared.replaceExisting = finalExists && replaceExisting;
    prepared.temporaryPath = prepared.finalPath.wstring() + L".remotec-" +
        Utf8ToWide(transferId) + L".part";
    const bool partExists =
        std::filesystem::exists(prepared.temporaryPath, ec);
    if (ec) {
        SetError(error, "The partial destination file could not be inspected.");
        return false;
    }
    if (partExists) {
        prepared.resumeOffset =
            std::filesystem::file_size(prepared.temporaryPath, ec);
        if (ec || prepared.resumeOffset > expectedSize) {
            SetError(error, "The existing partial file is invalid.");
            return false;
        }
    } else {
        UniqueHandle created(CreateFileW(
            prepared.temporaryPath.c_str(), GENERIC_WRITE, 0, nullptr,
            CREATE_NEW, FILE_ATTRIBUTE_NORMAL, nullptr));
        if (created.get() == INVALID_HANDLE_VALUE) {
            SetError(error, "The partial destination file could not be created.");
            return false;
        }
    }
    *output = std::move(prepared);
    return true;
}

bool WindowsFileTransferService::AppendChunk(
    const std::filesystem::path& temporaryPath,
    std::uint64_t expectedOffset,
    std::span<const std::uint8_t> bytes,
    std::string* error)
{
    if (bytes.empty() || bytes.size() > kMaximumFileChunkBytes) {
        SetError(error, "The destination file chunk is invalid.");
        return false;
    }
    const auto key = temporaryPath.wstring();
    auto found = impl_->destinations.find(key);
    if (found == impl_->destinations.end()) {
        UniqueHandle file;
        DWORD openError = ERROR_SUCCESS;
        for (int attempt = 0; attempt < kDestinationOpenRetryCount; ++attempt) {
            file.reset(CreateFileW(
                temporaryPath.c_str(), GENERIC_READ | GENERIC_WRITE,
                FILE_SHARE_READ | FILE_SHARE_DELETE, nullptr, OPEN_EXISTING,
                FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, nullptr));
            if (file.get() != INVALID_HANDLE_VALUE) {
                break;
            }
            openError = GetLastError();
            if (openError != ERROR_SHARING_VIOLATION &&
                openError != ERROR_LOCK_VIOLATION) {
                break;
            }
            std::this_thread::sleep_for(kDestinationOpenRetryDelay);
        }
        if (file.get() == INVALID_HANDLE_VALUE) {
            SetError(error, WindowsError("Opening the partial file", openError));
            return false;
        }
        std::uint64_t actualSize = 0;
        if (!GetFileSize(file.get(), &actualSize)) {
            SetError(error, WindowsError("Reading the partial file size",
                                         GetLastError()));
            return false;
        }
        if (actualSize != expectedOffset) {
            SetError(error, "The partial file size is " +
                                std::to_string(actualSize) +
                                " bytes, but offset " +
                                std::to_string(expectedOffset) +
                                " was expected.");
            return false;
        }
        if (!SeekFile(file.get(), expectedOffset)) {
            SetError(error, WindowsError("Seeking the partial file",
                                         GetLastError()));
            return false;
        }
        Impl::OpenDestination destination;
        destination.file = std::move(file);
        destination.offset = expectedOffset;
        found = impl_->destinations.emplace(key, std::move(destination)).first;
    }

    auto& destination = found->second;
    if (destination.offset != expectedOffset) {
        SetError(error, "The open partial file is at offset " +
                            std::to_string(destination.offset) +
                            ", but offset " + std::to_string(expectedOffset) +
                            " was expected.");
        return false;
    }
    DWORD written = 0;
    if (!WriteFile(destination.file.get(), bytes.data(),
                   static_cast<DWORD>(bytes.size()), &written, nullptr) ||
        written != bytes.size()) {
        SetError(error, WindowsError("Writing the partial file", GetLastError()));
        return false;
    }
    destination.offset += written;
    return true;
}

bool WindowsFileTransferService::VerifyAndCommit(
    const FileTransferDestination& destination,
    std::uint64_t expectedSize,
    const std::array<std::uint8_t, 32>& expectedSha256,
    std::string* error)
{
    const auto open = impl_->destinations.find(
        destination.temporaryPath.wstring());
    if (open != impl_->destinations.end()) {
        // Closing the handle makes all cached writes visible to the following
        // size and SHA-256 reads. Windows remains responsible for physical
        // disk-cache writeback.
        impl_->destinations.erase(open);
    }
    std::error_code ec;
    const auto actualSize =
        std::filesystem::file_size(destination.temporaryPath, ec);
    if (ec || actualSize != expectedSize) {
        SetError(error, "The received file size does not match.");
        return false;
    }
    std::array<std::uint8_t, 32> actualSha256{};
    if (!CalculateSha256(destination.temporaryPath, &actualSha256, error)) {
        return false;
    }
    if (actualSha256 != expectedSha256) {
        SetError(error, "The received file SHA-256 does not match.");
        return false;
    }
    const DWORD moveFlags = destination.replaceExisting
                                ? MOVEFILE_REPLACE_EXISTING
                                : 0;
    if (!MoveFileExW(destination.temporaryPath.c_str(),
                     destination.finalPath.c_str(), moveFlags)) {
        SetError(error, "The verified file could not be committed.");
        return false;
    }
    return true;
}

void WindowsFileTransferService::RemovePartial(
    const std::filesystem::path& temporaryPath)
{
    if (temporaryPath.empty()) {
        return;
    }
    impl_->destinations.erase(temporaryPath.wstring());
    std::error_code ignored;
    std::filesystem::remove(temporaryPath, ignored);
}

void WindowsFileTransferService::CloseDestination(
    const std::filesystem::path& temporaryPath)
{
    if (!temporaryPath.empty()) {
        impl_->destinations.erase(temporaryPath.wstring());
    }
}

bool WindowsFileTransferService::CalculateSha256(
    const std::filesystem::path& path,
    std::array<std::uint8_t, 32>* digest,
    std::string* error) const
{
    if (!digest) {
        SetError(error, "A SHA-256 output buffer is required.");
        return false;
    }
    BCRYPT_ALG_HANDLE algorithm = nullptr;
    BCRYPT_HASH_HANDLE hash = nullptr;
    DWORD objectLength = 0;
    DWORD resultLength = 0;
    std::vector<std::uint8_t> hashObject;
    auto cleanup = [&] {
        if (hash) {
            BCryptDestroyHash(hash);
        }
        if (algorithm) {
            BCryptCloseAlgorithmProvider(algorithm, 0);
        }
    };
    if (BCryptOpenAlgorithmProvider(
            &algorithm, BCRYPT_SHA256_ALGORITHM, nullptr, 0) < 0 ||
        BCryptGetProperty(
            algorithm, BCRYPT_OBJECT_LENGTH,
            reinterpret_cast<PUCHAR>(&objectLength), sizeof(objectLength),
            &resultLength, 0) < 0) {
        cleanup();
        SetError(error, "Windows SHA-256 initialization failed.");
        return false;
    }
    hashObject.resize(objectLength);
    if (BCryptCreateHash(algorithm, &hash, hashObject.data(),
                         static_cast<ULONG>(hashObject.size()), nullptr, 0,
                         0) < 0) {
        cleanup();
        SetError(error, "Windows SHA-256 state creation failed.");
        return false;
    }
    UniqueHandle file(CreateFileW(
        path.c_str(), GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, nullptr));
    if (file.get() == INVALID_HANDLE_VALUE) {
        cleanup();
        SetError(error, "The file could not be opened for SHA-256.");
        return false;
    }
    std::vector<std::uint8_t> buffer(1024 * 1024);
    bool succeeded = true;
    for (;;) {
        DWORD read = 0;
        if (!ReadFile(file.get(), buffer.data(),
                      static_cast<DWORD>(buffer.size()), &read, nullptr)) {
            succeeded = false;
            break;
        }
        if (read == 0) {
            break;
        }
        if (BCryptHashData(hash, buffer.data(), read, 0) < 0) {
            succeeded = false;
            break;
        }
    }
    if (succeeded &&
        BCryptFinishHash(hash, digest->data(),
                         static_cast<ULONG>(digest->size()), 0) < 0) {
        succeeded = false;
    }
    cleanup();
    if (!succeeded) {
        SetError(error, "The file SHA-256 calculation failed.");
    }
    return succeeded;
}

}  // namespace remote
