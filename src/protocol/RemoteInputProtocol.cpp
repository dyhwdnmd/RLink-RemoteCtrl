// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#include "RemoteInputProtocol.h"

#include <algorithm>
#include <array>
#include <limits>
#include <utility>

#include "BinaryProtocol.h"

namespace remote {
namespace {

constexpr std::array<std::uint8_t, 4> kMagic = {'R', 'C', 'I', '1'};
constexpr std::size_t kFixedHeaderBytes = 44;
constexpr std::size_t kMaximumRoomIdBytes = 128;
constexpr std::size_t kMaximumDeviceIdBytes = 64;
constexpr std::size_t kMaximumGrantIdBytes = 128;

void SetError(std::string* error, std::string message)
{
    if (error) {
        *error = std::move(message);
    }
}

bool ValidType(std::uint8_t type)
{
    return type >= static_cast<std::uint8_t>(
                       RemoteInputMessageType::kMouseMove) &&
           type <= static_cast<std::uint8_t>(
                       RemoteInputMessageType::kReleaseAll);
}

bool ValidMouseButton(RemoteMouseButton button)
{
    return button >= RemoteMouseButton::kLeft &&
           button <= RemoteMouseButton::kX2;
}

std::uint16_t PayloadSize(RemoteInputMessageType type)
{
    switch (type) {
    case RemoteInputMessageType::kMouseMove:
        return 5;
    case RemoteInputMessageType::kMouseButton:
        return 7;
    case RemoteInputMessageType::kMouseWheel:
        return 9;
    case RemoteInputMessageType::kKey:
        return 8;
    case RemoteInputMessageType::kReleaseAll:
        return 0;
    }
    return std::numeric_limits<std::uint16_t>::max();
}

bool ValidateEnvelope(const RemoteInputEnvelope& input, std::string* error)
{
    if (input.roomId.empty() ||
        input.roomId.size() > kMaximumRoomIdBytes ||
        input.senderDeviceId.empty() ||
        input.senderDeviceId.size() > kMaximumDeviceIdBytes ||
        input.controlGrantId.empty() ||
        input.controlGrantId.size() > kMaximumGrantIdBytes) {
        SetError(error, "Remote input identity fields are invalid.");
        return false;
    }
    if (input.sequence == 0 || input.timestampUs == 0) {
        SetError(error, "Remote input sequence and timestamp are required.");
        return false;
    }
    if (PayloadSize(input.event.type) ==
        std::numeric_limits<std::uint16_t>::max()) {
        SetError(error, "Remote input message type is invalid.");
        return false;
    }
    if (input.event.type == RemoteInputMessageType::kMouseButton &&
        !ValidMouseButton(input.event.mouseButton)) {
        SetError(error, "Remote mouse button is invalid.");
        return false;
    }
    if (IsPointerInput(input.event.type) &&
        (input.event.pressedMouseButtons &
            ~kRemoteMouseButtonsMask) != 0) {
        SetError(error, "Remote mouse button-state mask is invalid.");
        return false;
    }
    if (IsPointerInput(input.event.type) &&
        (input.event.displayId == 0 ||
         input.event.displayLayoutVersion == 0)) {
        SetError(error,
                 "Remote pointer display identity is missing.");
        return false;
    }
    if (input.event.type == RemoteInputMessageType::kMouseButton) {
        const auto bit = static_cast<std::uint8_t>(
            1u << (static_cast<std::uint8_t>(input.event.mouseButton) - 1));
        if (((input.event.pressedMouseButtons & bit) != 0) !=
            input.event.pressed) {
            SetError(error,
                     "Remote mouse transition does not match its state mask.");
            return false;
        }
    }
    if (input.event.type == RemoteInputMessageType::kKey &&
        input.event.virtualKey == 0 && input.event.scanCode == 0) {
        SetError(error, "Remote key identity is missing.");
        return false;
    }
    return true;
}

void EncodePayload(const RemoteInputEvent& event,
                   BinaryProtocolWriter* writer)
{
    switch (event.type) {
    case RemoteInputMessageType::kMouseMove:
        writer->WriteU16(event.normalizedX);
        writer->WriteU16(event.normalizedY);
        writer->WriteU8(event.pressedMouseButtons);
        break;
    case RemoteInputMessageType::kMouseButton:
        writer->WriteU16(event.normalizedX);
        writer->WriteU16(event.normalizedY);
        writer->WriteU8(static_cast<std::uint8_t>(event.mouseButton));
        writer->WriteU8(event.pressed ? 1 : 0);
        writer->WriteU8(event.pressedMouseButtons);
        break;
    case RemoteInputMessageType::kMouseWheel:
        writer->WriteU16(event.normalizedX);
        writer->WriteU16(event.normalizedY);
        writer->WriteU16(
            static_cast<std::uint16_t>(event.wheelDeltaX));
        writer->WriteU16(
            static_cast<std::uint16_t>(event.wheelDeltaY));
        writer->WriteU8(event.pressedMouseButtons);
        break;
    case RemoteInputMessageType::kKey:
        writer->WriteU16(event.virtualKey);
        writer->WriteU16(event.scanCode);
        writer->WriteU8(event.pressed ? 1 : 0);
        writer->WriteU8(event.extendedKey ? 1 : 0);
        writer->WriteU8(event.repeat ? 1 : 0);
        writer->WriteU8(0);
        break;
    case RemoteInputMessageType::kReleaseAll:
        break;
    }
}

bool DecodePayload(BinaryProtocolReader* reader,
                   std::uint16_t payloadSize,
                   RemoteInputEvent* event,
                   std::string* error)
{
    if (payloadSize != PayloadSize(event->type)) {
        SetError(error, "Remote input payload size does not match its type.");
        return false;
    }

    std::uint8_t byte = 0;
    std::uint16_t value = 0;
    switch (event->type) {
    case RemoteInputMessageType::kMouseMove:
        return reader->ReadU16(&event->normalizedX) &&
               reader->ReadU16(&event->normalizedY) &&
               reader->ReadU8(&event->pressedMouseButtons);
    case RemoteInputMessageType::kMouseButton:
        if (!reader->ReadU16(&event->normalizedX) ||
            !reader->ReadU16(&event->normalizedY) ||
            !reader->ReadU8(&byte)) {
            return false;
        }
        event->mouseButton = static_cast<RemoteMouseButton>(byte);
        if (!ValidMouseButton(event->mouseButton) ||
            !reader->ReadU8(&byte) || byte > 1) {
            SetError(error, "Remote mouse button payload is invalid.");
            return false;
        }
        event->pressed = byte != 0;
        return reader->ReadU8(&event->pressedMouseButtons);
    case RemoteInputMessageType::kMouseWheel:
        if (!reader->ReadU16(&event->normalizedX) ||
            !reader->ReadU16(&event->normalizedY) ||
            !reader->ReadU16(&value)) {
            return false;
        }
        event->wheelDeltaX = static_cast<std::int16_t>(value);
        if (!reader->ReadU16(&value)) {
            return false;
        }
        event->wheelDeltaY = static_cast<std::int16_t>(value);
        return reader->ReadU8(&event->pressedMouseButtons);
    case RemoteInputMessageType::kKey:
        if (!reader->ReadU16(&event->virtualKey) ||
            !reader->ReadU16(&event->scanCode) ||
            !reader->ReadU8(&byte) || byte > 1) {
            return false;
        }
        event->pressed = byte != 0;
        if (!reader->ReadU8(&byte) || byte > 1) {
            return false;
        }
        event->extendedKey = byte != 0;
        if (!reader->ReadU8(&byte) || byte > 1) {
            return false;
        }
        event->repeat = byte != 0;
        if (!reader->ReadU8(&byte) || byte != 0 ||
            (event->virtualKey == 0 && event->scanCode == 0)) {
            SetError(error, "Remote key payload is invalid.");
            return false;
        }
        return true;
    case RemoteInputMessageType::kReleaseAll:
        return true;
    }
    return false;
}

}  // namespace

bool UsesFastInputChannel(RemoteInputMessageType type)
{
    return type == RemoteInputMessageType::kMouseMove ||
           type == RemoteInputMessageType::kMouseButton ||
           type == RemoteInputMessageType::kMouseWheel;
}

bool IsPointerInput(RemoteInputMessageType type)
{
    return type == RemoteInputMessageType::kMouseMove ||
           type == RemoteInputMessageType::kMouseButton ||
           type == RemoteInputMessageType::kMouseWheel;
}

bool EncodeRemoteInput(const RemoteInputEnvelope& input,
                       std::vector<std::uint8_t>* encoded,
                       std::string* error)
{
    if (!encoded) {
        SetError(error, "Remote input output buffer is required.");
        return false;
    }
    if (!ValidateEnvelope(input, error)) {
        return false;
    }

    const auto roomLength = static_cast<std::uint16_t>(input.roomId.size());
    const auto senderLength =
        static_cast<std::uint16_t>(input.senderDeviceId.size());
    const auto grantLength =
        static_cast<std::uint16_t>(input.controlGrantId.size());
    const std::uint16_t payloadLength = PayloadSize(input.event.type);
    const std::size_t totalSize = kFixedHeaderBytes + roomLength +
        senderLength + grantLength + payloadLength;
    if (totalSize > kMaximumRemoteInputMessageBytes) {
        SetError(error, "Remote input message exceeds the size limit.");
        return false;
    }

    encoded->clear();
    encoded->reserve(totalSize);
    encoded->insert(encoded->end(), kMagic.begin(), kMagic.end());
    BinaryProtocolWriter writer(encoded);
    writer.WriteU8(kRemoteInputProtocolVersion);
    writer.WriteU8(static_cast<std::uint8_t>(input.event.type));
    writer.WriteU16(0);
    writer.WriteU64(input.sequence);
    writer.WriteU64(input.timestampUs);
    writer.WriteU32(input.event.displayId);
    writer.WriteU64(input.event.displayLayoutVersion);
    writer.WriteU16(roomLength);
    writer.WriteU16(senderLength);
    writer.WriteU16(grantLength);
    writer.WriteU16(payloadLength);
    writer.WriteString(input.roomId);
    writer.WriteString(input.senderDeviceId);
    writer.WriteString(input.controlGrantId);
    EncodePayload(input.event, &writer);
    return true;
}

bool DecodeRemoteInput(std::span<const std::uint8_t> encoded,
                       RemoteInputEnvelope* input,
                       std::string* error)
{
    if (!input) {
        SetError(error, "Remote input destination is required.");
        return false;
    }
    if (encoded.size() < kFixedHeaderBytes ||
        encoded.size() > kMaximumRemoteInputMessageBytes) {
        SetError(error, "Remote input message size is invalid.");
        return false;
    }
    if (!std::equal(kMagic.begin(), kMagic.end(), encoded.begin())) {
        SetError(error, "Remote input magic is invalid.");
        return false;
    }

    BinaryProtocolReader reader(encoded.subspan(kMagic.size()));
    std::uint8_t version = 0;
    std::uint8_t type = 0;
    std::uint16_t flags = 0;
    RemoteInputEnvelope decoded;
    if (!reader.ReadU8(&version) || version != kRemoteInputProtocolVersion ||
        !reader.ReadU8(&type) || !ValidType(type) ||
        !reader.ReadU16(&flags) || flags != 0 ||
        !reader.ReadU64(&decoded.sequence) ||
        !reader.ReadU64(&decoded.timestampUs) ||
        !reader.ReadU32(&decoded.event.displayId) ||
        !reader.ReadU64(&decoded.event.displayLayoutVersion)) {
        SetError(error, "Remote input header is invalid.");
        return false;
    }
    decoded.event.type = static_cast<RemoteInputMessageType>(type);

    std::uint16_t roomLength = 0;
    std::uint16_t senderLength = 0;
    std::uint16_t grantLength = 0;
    std::uint16_t payloadLength = 0;
    if (!reader.ReadU16(&roomLength) ||
        !reader.ReadU16(&senderLength) ||
        !reader.ReadU16(&grantLength) ||
        !reader.ReadU16(&payloadLength) ||
        roomLength == 0 || roomLength > kMaximumRoomIdBytes ||
        senderLength == 0 || senderLength > kMaximumDeviceIdBytes ||
        grantLength == 0 || grantLength > kMaximumGrantIdBytes ||
        reader.remaining() != static_cast<std::size_t>(roomLength) +
                                  senderLength + grantLength +
                                  payloadLength ||
        !reader.ReadString(roomLength, &decoded.roomId) ||
        !reader.ReadString(senderLength, &decoded.senderDeviceId) ||
        !reader.ReadString(grantLength, &decoded.controlGrantId)) {
        SetError(error, "Remote input identity lengths are invalid.");
        return false;
    }
    if (!DecodePayload(&reader, payloadLength, &decoded.event, error) ||
        reader.remaining() != 0 || !ValidateEnvelope(decoded, error)) {
        if (error && error->empty()) {
            *error = "Remote input payload is truncated.";
        }
        return false;
    }

    decoded.event.deliverySequence = decoded.sequence;
    *input = std::move(decoded);
    return true;
}

}  // namespace remote
