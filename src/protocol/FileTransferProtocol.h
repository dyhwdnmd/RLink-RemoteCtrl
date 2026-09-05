// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#pragma once

#include <array>
#include <cstdint>
#include <span>
#include <string>
#include <vector>

namespace remote {

inline constexpr std::uint8_t kFileTransferProtocolVersion = 4;
inline constexpr std::size_t kMaximumFileTransferMessageBytes = 64 * 1024;
inline constexpr std::size_t kMaximumFileChunkBytes = 60 * 1024;
inline constexpr std::uint64_t kMaximumFileSizeBytes =
    std::uint64_t{1} << 50;

enum class FileTransferMessageType : std::uint8_t {
    kOffer = 1,
    kResponse = 2,
    kChunk = 3,
    kComplete = 4,
    kCancel = 5,
    kError = 6,
    kResume = 7,
    kProgress = 8,
};

struct FileTransferMessage {
    FileTransferMessageType type = FileTransferMessageType::kOffer;
    std::string transferId;
    std::string fileName;
    std::uint64_t fileSize = 0;
    std::array<std::uint8_t, 32> sha256{};
    bool hasSha256 = false;
    bool accepted = false;
    std::uint64_t resumeOffset = 0;
    std::uint64_t chunkOffset = 0;
    std::vector<std::uint8_t> chunk;
    std::string errorCode;
    std::string errorMessage;
};

struct FileTransferEnvelope {
    std::string roomId;
    std::string senderDeviceId;
    std::string receiverDeviceId;
    std::uint64_t sequence = 0;
    std::uint64_t timestampUs = 0;
    FileTransferMessage message;
};

class IFileTransferSink {
public:
    virtual ~IFileTransferSink() = default;

    // The engine invokes this only after validating room membership, the Pair
    // identity, sender/receiver IDs and the per-Pair monotonic sequence.
    virtual void OnFileTransferMessage(
        const FileTransferEnvelope& envelope) = 0;
};

bool EncodeFileTransferMessage(const FileTransferEnvelope& input,
                               std::vector<std::uint8_t>* encoded,
                               std::string* error = nullptr);

bool DecodeFileTransferMessage(std::span<const std::uint8_t> encoded,
                               FileTransferEnvelope* output,
                               std::string* error = nullptr);

}  // namespace remote
