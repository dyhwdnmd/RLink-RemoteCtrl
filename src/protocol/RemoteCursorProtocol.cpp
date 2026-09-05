// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#include "RemoteCursorProtocol.h"

#include <array>
#include <limits>

#include "BinaryProtocol.h"

namespace remote {
namespace {

constexpr std::array<std::uint8_t, 4> kMagic = {'R', 'C', 'C', '1'};
constexpr std::uint8_t kVersion = 1;
constexpr std::size_t kMaximumIdentityBytes = 512;

bool Fail(std::string* error, const char* message)
{
    if (error) {
        *error = message;
    }
    return false;
}

bool IsKnownType(RemoteCursorMessageType type)
{
    return type == RemoteCursorMessageType::kPosition ||
           type == RemoteCursorMessageType::kShape ||
           type == RemoteCursorMessageType::kReset;
}

bool ValidateIdentity(const RemoteCursorEnvelope& envelope,
                      std::string* error)
{
    if (envelope.contextId.empty() || envelope.senderDeviceId.empty()) {
        return Fail(error, "Cursor context and sender must not be empty.");
    }
    if (envelope.contextId.size() > kMaximumIdentityBytes ||
        envelope.senderDeviceId.size() > kMaximumIdentityBytes ||
        envelope.contextId.size() >
            (std::numeric_limits<std::uint16_t>::max)() ||
        envelope.senderDeviceId.size() >
            (std::numeric_limits<std::uint16_t>::max)()) {
        return Fail(error, "Cursor identity is too long.");
    }
    return true;
}

bool ValidateShape(const RemoteCursorShape& shape, std::string* error)
{
    if (shape.shapeId == 0 || shape.width == 0 || shape.height == 0 ||
        shape.width > kMaximumRemoteCursorDimension ||
        shape.height > kMaximumRemoteCursorDimension ||
        shape.hotspotX >= shape.width || shape.hotspotY >= shape.height) {
        return Fail(error, "Cursor shape geometry is invalid.");
    }
    const auto pixels = static_cast<std::size_t>(shape.width) * shape.height;
    if (pixels > kMaximumRemoteCursorPixelBytes / 4 ||
        shape.premultipliedBgra.size() != pixels * 4 ||
        (!shape.invertMask.empty() && shape.invertMask.size() != pixels)) {
        return Fail(error, "Cursor shape payload size is invalid.");
    }
    return true;
}

}  // namespace

bool EncodeRemoteCursorMessage(
    const RemoteCursorEnvelope& envelope,
    std::vector<std::uint8_t>* output,
    std::string* error)
{
    if (!output) {
        return Fail(error, "Cursor output buffer is null.");
    }
    output->clear();
    if (!IsKnownType(envelope.type) ||
        !ValidateIdentity(envelope, error)) {
        return false;
    }
    if (envelope.type == RemoteCursorMessageType::kShape &&
        !ValidateShape(envelope.shape, error)) {
        return false;
    }

    BinaryProtocolWriter writer(output);
    writer.WriteBytes(kMagic);
    writer.WriteU8(kVersion);
    writer.WriteU8(static_cast<std::uint8_t>(envelope.type));
    writer.WriteU16(0);
    writer.WriteU64(envelope.sequence);
    writer.WriteU64(envelope.timestampUs);
    writer.WriteU64(envelope.screenShareGeneration);
    writer.WriteU16(static_cast<std::uint16_t>(envelope.contextId.size()));
    writer.WriteU16(
        static_cast<std::uint16_t>(envelope.senderDeviceId.size()));
    writer.WriteString(envelope.contextId);
    writer.WriteString(envelope.senderDeviceId);

    switch (envelope.type) {
    case RemoteCursorMessageType::kPosition:
        writer.WriteU32(envelope.position.displayId);
        writer.WriteU64(envelope.position.displayLayoutVersion);
        writer.WriteU16(envelope.position.normalizedX);
        writer.WriteU16(envelope.position.normalizedY);
        writer.WriteU8(envelope.position.visible ? 1 : 0);
        writer.WriteU8(0);
        writer.WriteU16(0);
        writer.WriteU64(envelope.position.shapeId);
        writer.WriteU64(envelope.position.lastAppliedInputSequence);
        break;
    case RemoteCursorMessageType::kShape:
        writer.WriteU64(envelope.shape.shapeId);
        writer.WriteU16(envelope.shape.width);
        writer.WriteU16(envelope.shape.height);
        writer.WriteU16(envelope.shape.hotspotX);
        writer.WriteU16(envelope.shape.hotspotY);
        writer.WriteU32(static_cast<std::uint32_t>(
            envelope.shape.premultipliedBgra.size()));
        writer.WriteU32(static_cast<std::uint32_t>(
            envelope.shape.invertMask.size()));
        writer.WriteBytes(envelope.shape.premultipliedBgra);
        writer.WriteBytes(envelope.shape.invertMask);
        break;
    case RemoteCursorMessageType::kReset:
        break;
    }
    if (error) {
        error->clear();
    }
    return true;
}

bool DecodeRemoteCursorMessage(
    std::span<const std::uint8_t> payload,
    RemoteCursorEnvelope* envelope,
    std::string* error)
{
    if (!envelope) {
        return Fail(error, "Cursor output envelope is null.");
    }
    BinaryProtocolReader reader(payload);
    std::array<std::uint8_t, 4> magic{};
    std::uint8_t version = 0;
    std::uint8_t rawType = 0;
    std::uint16_t reserved = 0;
    std::uint16_t contextLength = 0;
    std::uint16_t senderLength = 0;
    RemoteCursorEnvelope decoded;
    if (!reader.ReadBytes(&magic) || magic != kMagic ||
        !reader.ReadU8(&version) || version != kVersion ||
        !reader.ReadU8(&rawType) || !reader.ReadU16(&reserved) ||
        reserved != 0 || !reader.ReadU64(&decoded.sequence) ||
        !reader.ReadU64(&decoded.timestampUs) ||
        !reader.ReadU64(&decoded.screenShareGeneration) ||
        !reader.ReadU16(&contextLength) || !reader.ReadU16(&senderLength)) {
        return Fail(error, "Cursor header is invalid or truncated.");
    }
    decoded.type = static_cast<RemoteCursorMessageType>(rawType);
    if (!IsKnownType(decoded.type) || contextLength == 0 ||
        senderLength == 0 || contextLength > kMaximumIdentityBytes ||
        senderLength > kMaximumIdentityBytes ||
        !reader.ReadString(contextLength, &decoded.contextId) ||
        !reader.ReadString(senderLength, &decoded.senderDeviceId)) {
        return Fail(error, "Cursor identity or message type is invalid.");
    }

    switch (decoded.type) {
    case RemoteCursorMessageType::kPosition: {
        std::uint8_t visible = 0;
        std::uint8_t reserved8 = 0;
        std::uint16_t reserved16 = 0;
        if (!reader.ReadU32(&decoded.position.displayId) ||
            !reader.ReadU64(&decoded.position.displayLayoutVersion) ||
            !reader.ReadU16(&decoded.position.normalizedX) ||
            !reader.ReadU16(&decoded.position.normalizedY) ||
            !reader.ReadU8(&visible) || visible > 1 ||
            !reader.ReadU8(&reserved8) || reserved8 != 0 ||
            !reader.ReadU16(&reserved16) || reserved16 != 0 ||
            !reader.ReadU64(&decoded.position.shapeId) ||
            !reader.ReadU64(&decoded.position.lastAppliedInputSequence)) {
            return Fail(error, "Cursor position is invalid or truncated.");
        }
        decoded.position.visible = visible != 0;
        break;
    }
    case RemoteCursorMessageType::kShape: {
        std::uint32_t pixelBytes = 0;
        std::uint32_t invertBytes = 0;
        if (!reader.ReadU64(&decoded.shape.shapeId) ||
            !reader.ReadU16(&decoded.shape.width) ||
            !reader.ReadU16(&decoded.shape.height) ||
            !reader.ReadU16(&decoded.shape.hotspotX) ||
            !reader.ReadU16(&decoded.shape.hotspotY) ||
            !reader.ReadU32(&pixelBytes) ||
            !reader.ReadU32(&invertBytes) ||
            pixelBytes > kMaximumRemoteCursorPixelBytes ||
            invertBytes > kMaximumRemoteCursorPixelBytes / 4 ||
            !reader.ReadBytes(pixelBytes,
                              &decoded.shape.premultipliedBgra) ||
            !reader.ReadBytes(invertBytes, &decoded.shape.invertMask) ||
            !ValidateShape(decoded.shape, error)) {
            if (error && error->empty()) {
                *error = "Cursor shape is invalid or truncated.";
            }
            return false;
        }
        break;
    }
    case RemoteCursorMessageType::kReset:
        break;
    }
    if (reader.remaining() != 0) {
        return Fail(error, "Cursor message has trailing bytes.");
    }
    *envelope = std::move(decoded);
    if (error) {
        error->clear();
    }
    return true;
}

}  // namespace remote
