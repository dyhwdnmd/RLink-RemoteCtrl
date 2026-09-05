// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#include "RoomMemberControlProtocol.h"

#include <algorithm>
#include <array>
#include <utility>

#include "BinaryProtocol.h"

namespace remote {
namespace {

constexpr std::array<std::uint8_t, 4> kMagic = {'R', 'C', 'M', '1'};
constexpr std::size_t kHeaderBytes = 32;
constexpr std::size_t kMaximumMessageBytes = 512;
constexpr std::size_t kMaximumRoomIdBytes = 128;
constexpr std::size_t kMaximumDeviceIdBytes = 64;
constexpr std::size_t kMaximumErrorBytes = 192;

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

void AppendU64(std::vector<std::uint8_t>* output, std::uint64_t value)
{
    BinaryProtocolWriter(output).WriteU64(value);
}

bool ReadU16(std::span<const std::uint8_t> bytes,
             std::size_t& offset,
             std::uint16_t* value)
{
    if (offset > bytes.size()) {
        return false;
    }
    BinaryProtocolReader reader(bytes.subspan(offset));
    const bool result = reader.ReadU16(value);
    offset += reader.offset();
    return result;
}

bool ReadU64(std::span<const std::uint8_t> bytes,
             std::size_t& offset,
             std::uint64_t* value)
{
    if (offset > bytes.size()) {
        return false;
    }
    BinaryProtocolReader reader(bytes.subspan(offset));
    const bool result = reader.ReadU64(value);
    offset += reader.offset();
    return result;
}

bool IsValidAction(RoomMemberAction action)
{
    switch (action) {
    case RoomMemberAction::kRequestScreenShare:
    case RoomMemberAction::kStopScreenShare:
    case RoomMemberAction::kDisableMicrophone:
        return true;
    }
    return false;
}

}  // namespace

bool EncodeRoomMemberAction(
    const RoomMemberActionEnvelope& message,
    std::vector<std::uint8_t>* encoded,
    std::string* error)
{
    const bool epochValid =
        message.action != RoomMemberAction::kStopScreenShare ||
        message.screenShareEpoch != 0;
    if (!encoded || message.roomId.empty() ||
        message.roomId.size() > kMaximumRoomIdBytes ||
        message.senderDeviceId.empty() ||
        message.senderDeviceId.size() > kMaximumDeviceIdBytes ||
        message.receiverDeviceId.empty() ||
        message.receiverDeviceId.size() > kMaximumDeviceIdBytes ||
        message.senderDeviceId == message.receiverDeviceId ||
        message.sequence == 0 || !IsValidAction(message.action) ||
        !epochValid || message.error.size() > kMaximumErrorBytes ||
        (!message.response && (!message.error.empty() || message.accepted))) {
        SetError(error, "Room member action is invalid.");
        return false;
    }

    const auto roomLength =
        static_cast<std::uint16_t>(message.roomId.size());
    const auto senderLength =
        static_cast<std::uint16_t>(message.senderDeviceId.size());
    const auto receiverLength =
        static_cast<std::uint16_t>(message.receiverDeviceId.size());
    const auto errorLength =
        static_cast<std::uint16_t>(message.error.size());
    const std::size_t totalSize =
        kHeaderBytes + roomLength + senderLength +
        receiverLength + errorLength;
    if (totalSize > kMaximumMessageBytes) {
        SetError(error, "Room member action is too large.");
        return false;
    }

    encoded->clear();
    encoded->reserve(totalSize);
    encoded->insert(encoded->end(), kMagic.begin(), kMagic.end());
    encoded->push_back(kRoomMemberControlProtocolVersion);
    encoded->push_back(message.response ? 2 : 1);
    encoded->push_back(static_cast<std::uint8_t>(message.action));
    encoded->push_back(message.accepted ? 1 : 0);
    AppendU64(encoded, message.sequence);
    AppendU64(encoded, message.screenShareEpoch);
    AppendU16(encoded, roomLength);
    AppendU16(encoded, senderLength);
    AppendU16(encoded, receiverLength);
    AppendU16(encoded, errorLength);
    encoded->insert(
        encoded->end(), message.roomId.begin(), message.roomId.end());
    encoded->insert(encoded->end(), message.senderDeviceId.begin(),
                    message.senderDeviceId.end());
    encoded->insert(encoded->end(), message.receiverDeviceId.begin(),
                    message.receiverDeviceId.end());
    encoded->insert(
        encoded->end(), message.error.begin(), message.error.end());
    return true;
}

bool DecodeRoomMemberAction(
    std::span<const std::uint8_t> encoded,
    RoomMemberActionEnvelope* message,
    std::string* error)
{
    if (!message || encoded.size() < kHeaderBytes ||
        encoded.size() > kMaximumMessageBytes ||
        !std::equal(kMagic.begin(), kMagic.end(), encoded.begin())) {
        SetError(error, "Room member action header is invalid.");
        return false;
    }

    std::size_t offset = kMagic.size();
    const std::uint8_t version = encoded[offset++];
    const std::uint8_t type = encoded[offset++];
    const std::uint8_t actionValue = encoded[offset++];
    const std::uint8_t acceptedValue = encoded[offset++];
    std::uint16_t roomLength = 0;
    std::uint16_t senderLength = 0;
    std::uint16_t receiverLength = 0;
    std::uint16_t errorLength = 0;
    RoomMemberActionEnvelope decoded;
    decoded.response = type == 2;
    decoded.accepted = acceptedValue == 1;
    decoded.action = static_cast<RoomMemberAction>(actionValue);
    if (version != kRoomMemberControlProtocolVersion ||
        (type != 1 && type != 2) || acceptedValue > 1 ||
        !IsValidAction(decoded.action) ||
        !ReadU64(encoded, offset, &decoded.sequence) ||
        !ReadU64(encoded, offset, &decoded.screenShareEpoch) ||
        !ReadU16(encoded, offset, &roomLength) ||
        !ReadU16(encoded, offset, &senderLength) ||
        !ReadU16(encoded, offset, &receiverLength) ||
        !ReadU16(encoded, offset, &errorLength) ||
        decoded.sequence == 0 ||
        (decoded.action == RoomMemberAction::kStopScreenShare &&
         decoded.screenShareEpoch == 0) ||
        roomLength == 0 || roomLength > kMaximumRoomIdBytes ||
        senderLength == 0 || senderLength > kMaximumDeviceIdBytes ||
        receiverLength == 0 ||
        receiverLength > kMaximumDeviceIdBytes ||
        errorLength > kMaximumErrorBytes ||
        (!decoded.response && (decoded.accepted || errorLength != 0)) ||
        encoded.size() - offset !=
            static_cast<std::size_t>(roomLength) + senderLength +
                receiverLength + errorLength) {
        SetError(error, "Room member action payload is invalid.");
        return false;
    }

    auto readString = [&encoded, &offset](std::uint16_t length) {
        std::string value(
            reinterpret_cast<const char*>(encoded.data() + offset), length);
        offset += length;
        return value;
    };
    decoded.roomId = readString(roomLength);
    decoded.senderDeviceId = readString(senderLength);
    decoded.receiverDeviceId = readString(receiverLength);
    decoded.error = readString(errorLength);
    if (decoded.senderDeviceId == decoded.receiverDeviceId) {
        SetError(error, "Room member action cannot target its sender.");
        return false;
    }
    *message = std::move(decoded);
    return true;
}

}  // namespace remote
