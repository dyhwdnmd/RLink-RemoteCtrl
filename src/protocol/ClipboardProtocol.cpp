// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#include "ClipboardProtocol.h"

#include <algorithm>
#include <array>
#include <limits>
#include <utility>

#include "BinaryProtocol.h"

namespace remote {
namespace {

constexpr std::array<std::uint8_t, 4> kMagic = {'R', 'C', 'C', '2'};
constexpr std::size_t kMaximumRoomIdBytes = 128;
constexpr std::size_t kMaximumDeviceIdBytes = 64;
constexpr std::size_t kMaximumGrantIdBytes = 192;
constexpr std::size_t kMaximumSessionIdBytes = 64;
constexpr std::size_t kMaximumItemIdBytes = 64;
constexpr std::size_t kMaximumDescriptorNameBytes = 1024;
constexpr std::size_t kMaximumErrorCodeBytes = 64;
constexpr std::size_t kMaximumErrorMessageBytes = 512;

void SetError(std::string* error, std::string message)
{
    if (error) {
        *error = std::move(message);
    }
}

bool ValidType(std::uint8_t value)
{
    return value >= static_cast<std::uint8_t>(ClipboardMessageType::kCapabilities) &&
           value <= static_cast<std::uint8_t>(ClipboardMessageType::kClipboardChanged);
}

bool ValidFormat(std::uint8_t value)
{
    return value >= static_cast<std::uint8_t>(ClipboardFormat::kUnicodeText) &&
           value <= static_cast<std::uint8_t>(ClipboardFormat::kDirectory);
}

bool SafeIdentifier(const std::string& value, std::size_t maximum, bool required)
{
    if ((required && value.empty()) || value.size() > maximum) {
        return false;
    }
    return std::all_of(value.begin(), value.end(), [](unsigned char byte) {
        return (byte >= 'a' && byte <= 'z') ||
               (byte >= 'A' && byte <= 'Z') ||
               (byte >= '0' && byte <= '9') || byte == '-' || byte == '_' ||
               byte == '.' || byte == ':';
    });
}

bool Validate(const ClipboardEnvelope& envelope, std::string* error)
{
    if (envelope.roomId.empty() || envelope.roomId.size() > kMaximumRoomIdBytes ||
        envelope.senderDeviceId.empty() ||
        envelope.senderDeviceId.size() > kMaximumDeviceIdBytes ||
        envelope.receiverDeviceId.empty() ||
        envelope.receiverDeviceId.size() > kMaximumDeviceIdBytes ||
        envelope.senderDeviceId == envelope.receiverDeviceId ||
        envelope.controlGrantId.empty() ||
        envelope.controlGrantId.size() > kMaximumGrantIdBytes ||
        !SafeIdentifier(envelope.clipboardSessionId,
                        kMaximumSessionIdBytes, true) ||
        envelope.sequence == 0 || envelope.timestampUs == 0) {
        SetError(error, "The clipboard envelope identity is invalid.");
        return false;
    }
    const auto& message = envelope.message;
    if (message.errorCode.size() > kMaximumErrorCodeBytes ||
        message.errorMessage.size() > kMaximumErrorMessageBytes ||
        message.descriptors.size() > kMaximumClipboardDescriptors ||
        message.data.size() > kMaximumClipboardChunkBytes ||
        (!message.itemId.empty() &&
         !SafeIdentifier(message.itemId, kMaximumItemIdBytes, false))) {
        SetError(error, "The clipboard message bounds are invalid.");
        return false;
    }
    if ((message.type == ClipboardMessageType::kOffer ||
         message.type == ClipboardMessageType::kChunk ||
         message.type == ClipboardMessageType::kCommit ||
         message.type == ClipboardMessageType::kApplyResult ||
         message.type == ClipboardMessageType::kProgress ||
         message.type == ClipboardMessageType::kRequestCurrent ||
         message.type == ClipboardMessageType::kClipboardChanged ||
         message.type == ClipboardMessageType::kCancel ||
         message.type == ClipboardMessageType::kClear) &&
        (message.itemId.empty() || message.lamportVersion == 0)) {
        SetError(error, "The clipboard item identity is missing.");
        return false;
    }
    if ((message.type == ClipboardMessageType::kOffer ||
         message.type == ClipboardMessageType::kCommit) &&
        message.descriptors.empty()) {
        SetError(error,
                 "A clipboard offer or commit requires format metadata.");
        return false;
    }
    if (message.type == ClipboardMessageType::kChunk &&
        (message.data.empty() ||
         message.offset > message.totalSize ||
         message.data.size() > message.totalSize - message.offset)) {
        SetError(error, "The clipboard chunk range is invalid.");
        return false;
    }
    if (message.type == ClipboardMessageType::kProgress &&
        (message.offset > message.totalSize || !message.data.empty())) {
        SetError(error, "The clipboard progress range is invalid.");
        return false;
    }
    const bool metadataAnnouncement =
        message.type == ClipboardMessageType::kClipboardChanged;
    const bool deferredDigestOffer =
        message.type == ClipboardMessageType::kOffer;
    const bool finalDigestCommit =
        message.type == ClipboardMessageType::kCommit;
    if (!message.descriptors.empty() && !metadataAnnouncement &&
        !deferredDigestOffer && !finalDigestCommit) {
        SetError(error,
                 "Clipboard descriptors are not valid for this message.");
        return false;
    }
    std::uint64_t aggregate = 0;
    for (const auto& descriptor : message.descriptors) {
        const auto format = static_cast<std::uint8_t>(descriptor.format);
        if (!ValidFormat(format) ||
            descriptor.name.size() > kMaximumDescriptorNameBytes ||
            descriptor.size > kMaximumClipboardFileBytes ||
            aggregate > kMaximumClipboardFileBytes - descriptor.size) {
            SetError(error, "A clipboard format descriptor is invalid.");
            return false;
        }
        if ((descriptor.format == ClipboardFormat::kDirectory &&
             (descriptor.size != 0 || descriptor.hasSha256)) ||
            (!descriptor.hasSha256 &&
             std::any_of(descriptor.sha256.begin(),
                         descriptor.sha256.end(),
                         [](std::uint8_t byte) { return byte != 0; })) ||
            ((descriptor.format == ClipboardFormat::kFile ||
              descriptor.format == ClipboardFormat::kDirectory) &&
             descriptor.name.empty())) {
            SetError(error, "A clipboard format descriptor is inconsistent.");
            return false;
        }
        if (metadataAnnouncement &&
            ((descriptor.format != ClipboardFormat::kFile &&
              descriptor.format != ClipboardFormat::kDirectory) ||
             !descriptor.topLevel || descriptor.hasSha256)) {
            SetError(error, "A clipboard metadata announcement is inconsistent.");
            return false;
        }
        if (deferredDigestOffer && descriptor.hasSha256) {
            SetError(error,
                     "A clipboard offer must defer its digest to commit.");
            return false;
        }
        if (finalDigestCommit &&
            descriptor.format != ClipboardFormat::kDirectory &&
            !descriptor.hasSha256) {
            SetError(error,
                     "A clipboard commit requires every final digest.");
            return false;
        }
        aggregate += descriptor.size;
    }
    if ((message.type == ClipboardMessageType::kOffer ||
         message.type == ClipboardMessageType::kCommit) &&
        message.totalSize != aggregate) {
        SetError(error,
                 "The clipboard transfer total size is inconsistent.");
        return false;
    }
    return true;
}

}  // namespace

bool IsClipboardTransferMessage(ClipboardMessageType type)
{
    // Keep the complete transfer handshake on one ordered SCTP stream. Apart
    // from preserving Commit ordering, this primes the stream with the small
    // Offer/Accept exchange before the first large clipboard payload.
    return type == ClipboardMessageType::kOffer ||
           type == ClipboardMessageType::kAccept ||
           type == ClipboardMessageType::kChunk ||
           type == ClipboardMessageType::kCommit;
}

bool EncodeClipboardMessage(const ClipboardEnvelope& input,
                            std::vector<std::uint8_t>* encoded,
                            std::string* error)
{
    if (!encoded || !Validate(input, error)) {
        return false;
    }
    if (input.roomId.size() > std::numeric_limits<std::uint16_t>::max() ||
        input.senderDeviceId.size() > std::numeric_limits<std::uint16_t>::max() ||
        input.receiverDeviceId.size() > std::numeric_limits<std::uint16_t>::max() ||
        input.controlGrantId.size() > std::numeric_limits<std::uint16_t>::max() ||
        input.clipboardSessionId.size() > std::numeric_limits<std::uint16_t>::max() ||
        input.message.itemId.size() > std::numeric_limits<std::uint16_t>::max()) {
        SetError(error, "A clipboard string is too long.");
        return false;
    }

    encoded->clear();
    BinaryProtocolWriter writer(encoded);
    writer.WriteBytes(kMagic);
    writer.WriteU8(kClipboardProtocolVersion);
    writer.WriteU8(static_cast<std::uint8_t>(input.message.type));
    std::uint16_t flags = 0;
    flags |= input.message.enabled ? 1u : 0u;
    flags |= input.message.accepted ? 2u : 0u;
    writer.WriteU16(flags);
    writer.WriteU16(static_cast<std::uint16_t>(input.roomId.size()));
    writer.WriteU16(static_cast<std::uint16_t>(input.senderDeviceId.size()));
    writer.WriteU16(static_cast<std::uint16_t>(input.receiverDeviceId.size()));
    writer.WriteU16(static_cast<std::uint16_t>(input.controlGrantId.size()));
    writer.WriteU16(static_cast<std::uint16_t>(input.clipboardSessionId.size()));
    writer.WriteU16(static_cast<std::uint16_t>(input.message.itemId.size()));
    writer.WriteU16(static_cast<std::uint16_t>(input.message.errorCode.size()));
    writer.WriteU16(static_cast<std::uint16_t>(input.message.errorMessage.size()));
    writer.WriteU16(static_cast<std::uint16_t>(input.message.descriptors.size()));
    writer.WriteU16(0);
    writer.WriteU64(input.sequence);
    writer.WriteU64(input.timestampUs);
    writer.WriteU64(input.message.lamportVersion);
    writer.WriteU32(input.message.capabilities);
    writer.WriteU32(input.message.descriptorIndex);
    writer.WriteU64(input.message.offset);
    writer.WriteU64(input.message.totalSize);
    writer.WriteU32(static_cast<std::uint32_t>(input.message.data.size()));
    writer.WriteString(input.roomId);
    writer.WriteString(input.senderDeviceId);
    writer.WriteString(input.receiverDeviceId);
    writer.WriteString(input.controlGrantId);
    writer.WriteString(input.clipboardSessionId);
    writer.WriteString(input.message.itemId);
    writer.WriteString(input.message.errorCode);
    writer.WriteString(input.message.errorMessage);
    for (const auto& descriptor : input.message.descriptors) {
        writer.WriteU8(static_cast<std::uint8_t>(descriptor.format));
        writer.WriteU8((descriptor.hasSha256 ? 1u : 0u) |
                       (descriptor.topLevel ? 2u : 0u));
        writer.WriteU16(static_cast<std::uint16_t>(descriptor.name.size()));
        writer.WriteU64(descriptor.size);
        writer.WriteBytes(descriptor.sha256);
        writer.WriteString(descriptor.name);
    }
    writer.WriteBytes(input.message.data);
    if (encoded->size() > kMaximumClipboardMessageBytes) {
        encoded->clear();
        SetError(error, "The encoded clipboard message is too large.");
        return false;
    }
    return true;
}

bool DecodeClipboardMessage(std::span<const std::uint8_t> encoded,
                            ClipboardEnvelope* output,
                            std::string* error)
{
    if (!output || encoded.empty() ||
        encoded.size() > kMaximumClipboardMessageBytes) {
        SetError(error, "The clipboard message length is invalid.");
        return false;
    }
    BinaryProtocolReader reader(encoded);
    std::array<std::uint8_t, 4> magic{};
    std::uint8_t version = 0;
    std::uint8_t type = 0;
    std::uint16_t flags = 0;
    std::array<std::uint16_t, 10> lengths{};
    std::uint32_t dataLength = 0;
    ClipboardEnvelope decoded;
    if (!reader.ReadBytes(&magic) || magic != kMagic ||
        !reader.ReadU8(&version) || version != kClipboardProtocolVersion ||
        !reader.ReadU8(&type) || !ValidType(type) ||
        !reader.ReadU16(&flags) || (flags & ~3u) != 0) {
        SetError(error, "The clipboard protocol header is invalid.");
        return false;
    }
    for (auto& length : lengths) {
        if (!reader.ReadU16(&length)) {
            SetError(error, "The clipboard length table is truncated.");
            return false;
        }
    }
    if (lengths[9] != 0) {
        SetError(error, "The clipboard reserved header is invalid.");
        return false;
    }
    decoded.message.type = static_cast<ClipboardMessageType>(type);
    decoded.message.enabled = (flags & 1u) != 0;
    decoded.message.accepted = (flags & 2u) != 0;
    if (!reader.ReadU64(&decoded.sequence) ||
        !reader.ReadU64(&decoded.timestampUs) ||
        !reader.ReadU64(&decoded.message.lamportVersion) ||
        !reader.ReadU32(&decoded.message.capabilities) ||
        !reader.ReadU32(&decoded.message.descriptorIndex) ||
        !reader.ReadU64(&decoded.message.offset) ||
        !reader.ReadU64(&decoded.message.totalSize) ||
        !reader.ReadU32(&dataLength)) {
        SetError(error, "The clipboard message header is truncated.");
        return false;
    }
    if (dataLength > kMaximumClipboardChunkBytes ||
        lengths[8] > kMaximumClipboardDescriptors ||
        !reader.ReadString(lengths[0], &decoded.roomId) ||
        !reader.ReadString(lengths[1], &decoded.senderDeviceId) ||
        !reader.ReadString(lengths[2], &decoded.receiverDeviceId) ||
        !reader.ReadString(lengths[3], &decoded.controlGrantId) ||
        !reader.ReadString(lengths[4], &decoded.clipboardSessionId) ||
        !reader.ReadString(lengths[5], &decoded.message.itemId) ||
        !reader.ReadString(lengths[6], &decoded.message.errorCode) ||
        !reader.ReadString(lengths[7], &decoded.message.errorMessage)) {
        SetError(error, "The clipboard message strings are invalid.");
        return false;
    }
    decoded.message.descriptors.reserve(lengths[8]);
    for (std::uint16_t index = 0; index < lengths[8]; ++index) {
        std::uint8_t format = 0;
        std::uint8_t descriptorFlags = 0;
        std::uint16_t nameLength = 0;
        ClipboardFormatDescriptor descriptor;
        if (!reader.ReadU8(&format) || !ValidFormat(format) ||
            !reader.ReadU8(&descriptorFlags) ||
            (descriptorFlags & ~3u) != 0 ||
            !reader.ReadU16(&nameLength) ||
            nameLength > kMaximumDescriptorNameBytes ||
            !reader.ReadU64(&descriptor.size) ||
            !reader.ReadBytes(&descriptor.sha256) ||
            !reader.ReadString(nameLength, &descriptor.name)) {
            SetError(error, "A clipboard descriptor is invalid.");
            return false;
        }
        descriptor.format = static_cast<ClipboardFormat>(format);
        descriptor.hasSha256 = (descriptorFlags & 1u) != 0;
        descriptor.topLevel = (descriptorFlags & 2u) != 0;
        decoded.message.descriptors.push_back(std::move(descriptor));
    }
    if (!reader.ReadBytes(dataLength, &decoded.message.data) ||
        reader.remaining() != 0 || !Validate(decoded, error)) {
        if (error && error->empty()) {
            *error = "The clipboard message payload is invalid.";
        }
        return false;
    }
    *output = std::move(decoded);
    return true;
}

}  // namespace remote
