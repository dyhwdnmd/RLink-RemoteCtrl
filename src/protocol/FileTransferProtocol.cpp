// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#include "FileTransferProtocol.h"

#include <algorithm>
#include <array>
#include <limits>
#include <utility>

#include "BinaryProtocol.h"

namespace remote {
namespace {

constexpr std::array<std::uint8_t, 4> kMagic = {'R', 'C', 'F', '1'};
constexpr std::size_t kFixedHeaderBytes = 36;
constexpr std::size_t kMaximumRoomIdBytes = 128;
constexpr std::size_t kMaximumDeviceIdBytes = 64;
constexpr std::size_t kMaximumTransferIdBytes = 64;
constexpr std::size_t kMaximumFileNameBytes = 255;
constexpr std::size_t kMaximumErrorCodeBytes = 64;
constexpr std::size_t kMaximumErrorMessageBytes = 512;

void SetError(std::string* error, std::string message)
{
    if (error) {
        *error = std::move(message);
    }
}

void AppendU16(std::vector<std::uint8_t>* output, std::uint16_t value)
{
    BinaryProtocolWriter(output).WriteU16(value);
}

void AppendU32(std::vector<std::uint8_t>* output, std::uint32_t value)
{
    BinaryProtocolWriter(output).WriteU32(value);
}

void AppendU64(std::vector<std::uint8_t>* output, std::uint64_t value)
{
    BinaryProtocolWriter(output).WriteU64(value);
}

void AppendString(std::vector<std::uint8_t>* output,
                  const std::string& value)
{
    BinaryProtocolWriter(output).WriteString(value);
}

using Reader = BinaryProtocolReader;

bool ValidType(std::uint8_t type)
{
    return type >= static_cast<std::uint8_t>(
                       FileTransferMessageType::kOffer) &&
           type <= static_cast<std::uint8_t>(
                       FileTransferMessageType::kProgress);
}

bool ValidTransferId(const std::string& value)
{
    return !value.empty() && value.size() <= kMaximumTransferIdBytes &&
           std::all_of(value.begin(), value.end(), [](unsigned char byte) {
               return (byte >= 'a' && byte <= 'z') ||
                      (byte >= 'A' && byte <= 'Z') ||
                      (byte >= '0' && byte <= '9') ||
                      byte == '-' || byte == '_';
           });
}

bool ValidFileName(const std::string& value)
{
    if (value.empty() || value.size() > kMaximumFileNameBytes ||
        value.back() == ' ' || value.back() == '.') {
        return false;
    }
    return std::none_of(value.begin(), value.end(), [](unsigned char byte) {
        return byte < 0x20 || byte == 0x7f || byte == '/' || byte == '\\' ||
               byte == ':' || byte == '*' || byte == '?' || byte == '"' ||
               byte == '<' || byte == '>' || byte == '|';
    });
}

bool ValidErrorCode(const std::string& value, bool required)
{
    if (value.size() > kMaximumErrorCodeBytes ||
        (required && value.empty())) {
        return false;
    }
    return std::all_of(value.begin(), value.end(), [](unsigned char byte) {
        return (byte >= 'a' && byte <= 'z') ||
               (byte >= '0' && byte <= '9') || byte == '_';
    });
}

bool ValidatePayload(const FileTransferMessage& message, std::string* error)
{
    if (!ValidTransferId(message.transferId)) {
        SetError(error, "The file transfer ID is invalid.");
        return false;
    }
    if (message.errorMessage.size() > kMaximumErrorMessageBytes) {
        SetError(error, "The file transfer error message is too long.");
        return false;
    }

    switch (message.type) {
    case FileTransferMessageType::kOffer:
        if (!ValidFileName(message.fileName) ||
            message.fileSize > kMaximumFileSizeBytes) {
            SetError(error, "The file offer metadata is invalid.");
            return false;
        }
        return true;
    case FileTransferMessageType::kResponse:
        if (message.resumeOffset > kMaximumFileSizeBytes ||
            (!message.accepted && message.resumeOffset != 0) ||
            !ValidErrorCode(message.errorCode, !message.accepted)) {
            SetError(error, "The file response is invalid.");
            return false;
        }
        return true;
    case FileTransferMessageType::kChunk:
        if (message.chunk.empty() ||
            message.chunk.size() > kMaximumFileChunkBytes ||
            message.chunkOffset > kMaximumFileSizeBytes ||
            message.chunk.size() >
                kMaximumFileSizeBytes - message.chunkOffset) {
            SetError(error, "The file chunk range is invalid.");
            return false;
        }
        return true;
    case FileTransferMessageType::kComplete:
        if (message.fileSize > kMaximumFileSizeBytes ||
            !message.hasSha256) {
            SetError(error, "The file completion metadata is invalid.");
            return false;
        }
        return true;
    case FileTransferMessageType::kCancel:
    case FileTransferMessageType::kError:
        if (!ValidErrorCode(message.errorCode, true)) {
            SetError(error, "The file transfer reason is invalid.");
            return false;
        }
        return true;
    case FileTransferMessageType::kResume:
        return true;
    case FileTransferMessageType::kProgress:
        if (message.resumeOffset > kMaximumFileSizeBytes) {
            SetError(error, "The file progress offset is invalid.");
            return false;
        }
        return true;
    }
    SetError(error, "The file transfer message type is invalid.");
    return false;
}

bool ValidateEnvelope(const FileTransferEnvelope& envelope,
                      std::string* error)
{
    if (envelope.roomId.empty() ||
        envelope.roomId.size() > kMaximumRoomIdBytes ||
        envelope.senderDeviceId.empty() ||
        envelope.senderDeviceId.size() > kMaximumDeviceIdBytes ||
        envelope.receiverDeviceId.empty() ||
        envelope.receiverDeviceId.size() > kMaximumDeviceIdBytes ||
        envelope.senderDeviceId == envelope.receiverDeviceId) {
        SetError(error, "The file transfer identity fields are invalid.");
        return false;
    }
    if (envelope.sequence == 0 || envelope.timestampUs == 0) {
        SetError(error, "The file transfer sequence and timestamp are required.");
        return false;
    }
    return ValidatePayload(envelope.message, error);
}

void AppendReason(const FileTransferMessage& message,
                  std::vector<std::uint8_t>* payload)
{
    AppendU16(payload,
              static_cast<std::uint16_t>(message.errorCode.size()));
    AppendU16(payload,
              static_cast<std::uint16_t>(message.errorMessage.size()));
    AppendString(payload, message.errorCode);
    AppendString(payload, message.errorMessage);
}

bool EncodePayload(const FileTransferMessage& message,
                   std::vector<std::uint8_t>* payload)
{
    payload->clear();
    switch (message.type) {
    case FileTransferMessageType::kOffer:
        AppendU64(payload, message.fileSize);
        AppendU16(payload,
                  static_cast<std::uint16_t>(message.fileName.size()));
        payload->push_back(message.hasSha256 ? 1 : 0);
        payload->push_back(0);
        AppendString(payload, message.fileName);
        if (message.hasSha256) {
            payload->insert(payload->end(), message.sha256.begin(),
                            message.sha256.end());
        }
        return true;
    case FileTransferMessageType::kResponse:
        payload->push_back(message.accepted ? 1 : 0);
        payload->push_back(0);
        AppendU16(payload,
                  static_cast<std::uint16_t>(message.errorCode.size()));
        AppendU16(payload,
                  static_cast<std::uint16_t>(message.errorMessage.size()));
        AppendU16(payload, 0);
        AppendU64(payload, message.resumeOffset);
        AppendString(payload, message.errorCode);
        AppendString(payload, message.errorMessage);
        return true;
    case FileTransferMessageType::kChunk:
        AppendU64(payload, message.chunkOffset);
        AppendU32(payload,
                  static_cast<std::uint32_t>(message.chunk.size()));
        payload->insert(payload->end(), message.chunk.begin(),
                        message.chunk.end());
        return true;
    case FileTransferMessageType::kComplete:
        AppendU64(payload, message.fileSize);
        payload->insert(payload->end(), message.sha256.begin(),
                        message.sha256.end());
        return true;
    case FileTransferMessageType::kCancel:
    case FileTransferMessageType::kError:
        AppendReason(message, payload);
        return true;
    case FileTransferMessageType::kResume:
        return true;
    case FileTransferMessageType::kProgress:
        AppendU64(payload, message.resumeOffset);
        return true;
    }
    return false;
}

bool ReadReason(Reader* reader, FileTransferMessage* message)
{
    std::uint16_t codeLength = 0;
    std::uint16_t messageLength = 0;
    return reader->ReadU16(&codeLength) &&
           reader->ReadU16(&messageLength) &&
           codeLength <= kMaximumErrorCodeBytes &&
           messageLength <= kMaximumErrorMessageBytes &&
           reader->ReadString(codeLength, &message->errorCode) &&
           reader->ReadString(messageLength, &message->errorMessage);
}

bool DecodePayload(std::span<const std::uint8_t> bytes,
                   FileTransferMessage* message)
{
    Reader reader(bytes);
    std::uint8_t byte = 0;
    std::uint16_t value16 = 0;
    std::uint32_t value32 = 0;
    switch (message->type) {
    case FileTransferMessageType::kOffer:
        if (!reader.ReadU64(&message->fileSize) ||
            !reader.ReadU16(&value16) ||
            value16 == 0 || value16 > kMaximumFileNameBytes ||
            !reader.ReadU8(&byte) || byte > 1) {
            return false;
        }
        message->hasSha256 = byte != 0;
        if (!reader.ReadU8(&byte) || byte != 0 ||
            !reader.ReadString(value16, &message->fileName)) {
            return false;
        }
        if (message->hasSha256 && !reader.ReadBytes(&message->sha256)) {
            return false;
        }
        break;
    case FileTransferMessageType::kResponse: {
        std::uint16_t codeLength = 0;
        std::uint16_t messageLength = 0;
        std::uint16_t reserved = 0;
        if (!reader.ReadU8(&byte) || byte > 1) {
            return false;
        }
        message->accepted = byte != 0;
        if (!reader.ReadU8(&byte) || byte != 0 ||
            !reader.ReadU16(&codeLength) ||
            !reader.ReadU16(&messageLength) ||
            !reader.ReadU16(&reserved) || reserved != 0 ||
            codeLength > kMaximumErrorCodeBytes ||
            messageLength > kMaximumErrorMessageBytes ||
            !reader.ReadU64(&message->resumeOffset) ||
            !reader.ReadString(codeLength, &message->errorCode) ||
            !reader.ReadString(messageLength, &message->errorMessage)) {
            return false;
        }
        break;
    }
    case FileTransferMessageType::kChunk:
        if (!reader.ReadU64(&message->chunkOffset) ||
            !reader.ReadU32(&value32) || value32 == 0 ||
            value32 > kMaximumFileChunkBytes ||
            !reader.ReadBytes(value32, &message->chunk)) {
            return false;
        }
        break;
    case FileTransferMessageType::kComplete:
        if (!reader.ReadU64(&message->fileSize) ||
            !reader.ReadBytes(&message->sha256)) {
            return false;
        }
        message->hasSha256 = true;
        break;
    case FileTransferMessageType::kCancel:
    case FileTransferMessageType::kError:
        if (!ReadReason(&reader, message)) {
            return false;
        }
        break;
    case FileTransferMessageType::kResume:
        break;
    case FileTransferMessageType::kProgress:
        if (!reader.ReadU64(&message->resumeOffset)) {
            return false;
        }
        break;
    }
    return reader.Remaining() == 0;
}

}  // namespace

bool EncodeFileTransferMessage(const FileTransferEnvelope& input,
                               std::vector<std::uint8_t>* encoded,
                               std::string* error)
{
    if (!encoded) {
        SetError(error, "A file transfer output buffer is required.");
        return false;
    }
    if (!ValidateEnvelope(input, error)) {
        return false;
    }

    std::vector<std::uint8_t> payload;
    if (!EncodePayload(input.message, &payload)) {
        SetError(error, "The file transfer payload could not be encoded.");
        return false;
    }
    const std::size_t totalSize = kFixedHeaderBytes + input.roomId.size() +
        input.senderDeviceId.size() + input.receiverDeviceId.size() +
        input.message.transferId.size() + payload.size();
    if (totalSize > kMaximumFileTransferMessageBytes ||
        payload.size() > std::numeric_limits<std::uint32_t>::max()) {
        SetError(error, "The file transfer message exceeds the size limit.");
        return false;
    }

    encoded->clear();
    encoded->reserve(totalSize);
    encoded->insert(encoded->end(), kMagic.begin(), kMagic.end());
    encoded->push_back(kFileTransferProtocolVersion);
    encoded->push_back(static_cast<std::uint8_t>(input.message.type));
    AppendU16(encoded, 0);
    AppendU64(encoded, input.sequence);
    AppendU64(encoded, input.timestampUs);
    AppendU16(encoded, static_cast<std::uint16_t>(input.roomId.size()));
    AppendU16(encoded,
              static_cast<std::uint16_t>(input.senderDeviceId.size()));
    AppendU16(encoded,
              static_cast<std::uint16_t>(input.receiverDeviceId.size()));
    AppendU16(encoded,
              static_cast<std::uint16_t>(input.message.transferId.size()));
    AppendU32(encoded, static_cast<std::uint32_t>(payload.size()));
    AppendString(encoded, input.roomId);
    AppendString(encoded, input.senderDeviceId);
    AppendString(encoded, input.receiverDeviceId);
    AppendString(encoded, input.message.transferId);
    encoded->insert(encoded->end(), payload.begin(), payload.end());
    return true;
}

bool DecodeFileTransferMessage(std::span<const std::uint8_t> encoded,
                               FileTransferEnvelope* output,
                               std::string* error)
{
    if (!output) {
        SetError(error, "A file transfer destination is required.");
        return false;
    }
    if (encoded.size() < kFixedHeaderBytes ||
        encoded.size() > kMaximumFileTransferMessageBytes ||
        !std::equal(kMagic.begin(), kMagic.end(), encoded.begin())) {
        SetError(error, "The file transfer message header is invalid.");
        return false;
    }

    Reader reader(encoded.subspan(kMagic.size()));
    std::uint8_t version = 0;
    std::uint8_t type = 0;
    std::uint16_t flags = 0;
    std::uint16_t roomLength = 0;
    std::uint16_t senderLength = 0;
    std::uint16_t receiverLength = 0;
    std::uint16_t transferLength = 0;
    std::uint32_t payloadLength = 0;
    FileTransferEnvelope decoded;
    if (!reader.ReadU8(&version) ||
        version != kFileTransferProtocolVersion ||
        !reader.ReadU8(&type) || !ValidType(type) ||
        !reader.ReadU16(&flags) || flags != 0 ||
        !reader.ReadU64(&decoded.sequence) ||
        !reader.ReadU64(&decoded.timestampUs) ||
        !reader.ReadU16(&roomLength) ||
        !reader.ReadU16(&senderLength) ||
        !reader.ReadU16(&receiverLength) ||
        !reader.ReadU16(&transferLength) ||
        !reader.ReadU32(&payloadLength) ||
        roomLength == 0 || roomLength > kMaximumRoomIdBytes ||
        senderLength == 0 || senderLength > kMaximumDeviceIdBytes ||
        receiverLength == 0 || receiverLength > kMaximumDeviceIdBytes ||
        transferLength == 0 ||
        transferLength > kMaximumTransferIdBytes ||
        reader.Remaining() != static_cast<std::size_t>(roomLength) +
                                  senderLength + receiverLength +
                                  transferLength + payloadLength ||
        !reader.ReadString(roomLength, &decoded.roomId) ||
        !reader.ReadString(senderLength, &decoded.senderDeviceId) ||
        !reader.ReadString(receiverLength, &decoded.receiverDeviceId) ||
        !reader.ReadString(transferLength,
                           &decoded.message.transferId)) {
        SetError(error, "The file transfer identity lengths are invalid.");
        return false;
    }
    decoded.message.type = static_cast<FileTransferMessageType>(type);
    const std::span<const std::uint8_t> payload = encoded.last(payloadLength);
    if (!DecodePayload(payload, &decoded.message) ||
        !ValidateEnvelope(decoded, error)) {
        if (error && error->empty()) {
            *error = "The file transfer payload is invalid.";
        }
        return false;
    }

    *output = std::move(decoded);
    return true;
}

}  // namespace remote
