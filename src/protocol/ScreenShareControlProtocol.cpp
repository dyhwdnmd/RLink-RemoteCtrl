// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#include "ScreenShareControlProtocol.h"

#include <algorithm>
#include <array>
#include <utility>

#include "BinaryProtocol.h"

namespace remote {
namespace {

constexpr std::array<std::uint8_t, 4> kMagic = {'R', 'C', 'S', '1'};
constexpr std::size_t kFixedHeaderBytes = 24;
constexpr std::size_t kMaximumMessageBytes = 512;
constexpr std::size_t kMaximumRoomIdBytes = 128;
constexpr std::size_t kMaximumDeviceIdBytes = 64;
constexpr std::size_t kMaximumErrorBytes = 192;
constexpr std::size_t kMaximumBackendNameBytes = 64;
constexpr std::size_t kMaximumDisplayKeyBytes = 192;
constexpr std::size_t kMaximumDisplayNameBytes = 128;
constexpr std::size_t kMaximumDisplayCatalogBytes = 8192;
constexpr std::size_t kMaximumDisplayCount = 16;

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

bool ReadU32(std::span<const std::uint8_t> bytes,
             std::size_t& offset,
             std::uint32_t* value)
{
    if (offset > bytes.size()) {
        return false;
    }
    BinaryProtocolReader reader(bytes.subspan(offset));
    const bool result = reader.ReadU32(value);
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

}  // namespace

bool EncodeScreenFrameRateRequest(
    const ScreenFrameRateRequest& request,
    std::vector<std::uint8_t>* encoded,
    std::string* error)
{
    if (!encoded) {
        SetError(error, "Screen frame-rate output buffer is required.");
        return false;
    }
    if (request.roomId.empty() ||
        request.roomId.size() > kMaximumRoomIdBytes ||
        request.senderDeviceId.empty() ||
        request.senderDeviceId.size() > kMaximumDeviceIdBytes ||
        request.sequence == 0 ||
        request.framesPerSecond < kMinimumScreenFrameRate ||
        request.framesPerSecond > kMaximumScreenFrameRate) {
        SetError(error, "Screen frame-rate request is invalid.");
        return false;
    }

    const auto roomLength =
        static_cast<std::uint16_t>(request.roomId.size());
    const auto senderLength =
        static_cast<std::uint16_t>(request.senderDeviceId.size());
    const std::size_t totalSize =
        kFixedHeaderBytes + roomLength + senderLength;
    if (totalSize > kMaximumMessageBytes) {
        SetError(error, "Screen frame-rate request is too large.");
        return false;
    }

    encoded->clear();
    encoded->reserve(totalSize);
    encoded->insert(encoded->end(), kMagic.begin(), kMagic.end());
    encoded->push_back(kScreenShareControlProtocolVersion);
    encoded->push_back(1);  // Set target frame rate.
    AppendU16(encoded, 0);
    AppendU64(encoded, request.sequence);
    AppendU32(encoded, request.framesPerSecond);
    AppendU16(encoded, roomLength);
    AppendU16(encoded, senderLength);
    encoded->insert(
        encoded->end(), request.roomId.begin(), request.roomId.end());
    encoded->insert(encoded->end(), request.senderDeviceId.begin(),
                    request.senderDeviceId.end());
    return true;
}

bool DecodeScreenFrameRateRequest(
    std::span<const std::uint8_t> encoded,
    ScreenFrameRateRequest* request,
    std::string* error)
{
    if (!request || encoded.size() < kFixedHeaderBytes ||
        encoded.size() > kMaximumMessageBytes ||
        !std::equal(kMagic.begin(), kMagic.end(), encoded.begin())) {
        SetError(error, "Screen frame-rate message header is invalid.");
        return false;
    }

    std::size_t offset = kMagic.size();
    const std::uint8_t version = encoded[offset++];
    const std::uint8_t type = encoded[offset++];
    std::uint16_t flags = 0;
    std::uint16_t roomLength = 0;
    std::uint16_t senderLength = 0;
    ScreenFrameRateRequest decoded;
    if (version != kScreenShareControlProtocolVersion || type != 1 ||
        !ReadU16(encoded, offset, &flags) || flags != 0 ||
        !ReadU64(encoded, offset, &decoded.sequence) ||
        !ReadU32(encoded, offset, &decoded.framesPerSecond) ||
        !ReadU16(encoded, offset, &roomLength) ||
        !ReadU16(encoded, offset, &senderLength) ||
        decoded.sequence == 0 || roomLength == 0 ||
        roomLength > kMaximumRoomIdBytes || senderLength == 0 ||
        senderLength > kMaximumDeviceIdBytes ||
        decoded.framesPerSecond < kMinimumScreenFrameRate ||
        decoded.framesPerSecond > kMaximumScreenFrameRate ||
        encoded.size() - offset !=
            static_cast<std::size_t>(roomLength) + senderLength) {
        SetError(error, "Screen frame-rate message payload is invalid.");
        return false;
    }

    decoded.roomId.assign(
        reinterpret_cast<const char*>(encoded.data() + offset), roomLength);
    offset += roomLength;
    decoded.senderDeviceId.assign(
        reinterpret_cast<const char*>(encoded.data() + offset),
        senderLength);
    *request = std::move(decoded);
    return true;
}

bool EncodeScreenStreamPreferenceRequest(
    const ScreenStreamPreferenceRequest& request,
    std::vector<std::uint8_t>* encoded,
    std::string* error)
{
    const bool originalSize = request.maxWidth == 0 && request.maxHeight == 0;
    const bool boundedSize = request.maxWidth > 0 && request.maxHeight > 0 &&
        request.maxWidth <= kMaximumScreenStreamWidth &&
        request.maxHeight <= kMaximumScreenStreamHeight;
    if (!encoded || request.roomId.empty() ||
        request.roomId.size() > kMaximumRoomIdBytes ||
        request.senderDeviceId.empty() ||
        request.senderDeviceId.size() > kMaximumDeviceIdBytes ||
        request.sequence == 0 || (!originalSize && !boundedSize) ||
        request.framesPerSecond < kMinimumScreenFrameRate ||
        request.framesPerSecond > kMaximumScreenFrameRate ||
        static_cast<std::uint8_t>(request.quality) >
            static_cast<std::uint8_t>(ScreenQualityTier::kOriginal)) {
        SetError(error, "Screen stream preference request is invalid.");
        return false;
    }

    constexpr std::size_t kHeaderBytes = 36;
    const auto roomLength = static_cast<std::uint16_t>(request.roomId.size());
    const auto senderLength =
        static_cast<std::uint16_t>(request.senderDeviceId.size());
    encoded->clear();
    encoded->reserve(kHeaderBytes + roomLength + senderLength);
    encoded->insert(encoded->end(), kMagic.begin(), kMagic.end());
    encoded->push_back(kScreenShareControlProtocolVersion);
    encoded->push_back(2);  // Set per-member stream preference.
    AppendU16(encoded, 0);
    AppendU64(encoded, request.sequence);
    AppendU32(encoded, request.maxWidth);
    AppendU32(encoded, request.maxHeight);
    AppendU32(encoded, request.framesPerSecond);
    encoded->push_back(static_cast<std::uint8_t>(request.quality));
    encoded->insert(encoded->end(), 3, 0);
    AppendU16(encoded, roomLength);
    AppendU16(encoded, senderLength);
    encoded->insert(encoded->end(), request.roomId.begin(), request.roomId.end());
    encoded->insert(encoded->end(), request.senderDeviceId.begin(),
                    request.senderDeviceId.end());
    return encoded->size() <= kMaximumMessageBytes;
}

bool DecodeScreenStreamPreferenceRequest(
    std::span<const std::uint8_t> encoded,
    ScreenStreamPreferenceRequest* request,
    std::string* error)
{
    constexpr std::size_t kHeaderBytes = 36;
    if (!request || encoded.size() < kHeaderBytes ||
        encoded.size() > kMaximumMessageBytes ||
        !std::equal(kMagic.begin(), kMagic.end(), encoded.begin())) {
        SetError(error, "Screen stream preference header is invalid.");
        return false;
    }
    std::size_t offset = kMagic.size();
    const std::uint8_t version = encoded[offset++];
    const std::uint8_t type = encoded[offset++];
    std::uint16_t flags = 0;
    std::uint16_t roomLength = 0;
    std::uint16_t senderLength = 0;
    ScreenStreamPreferenceRequest decoded;
    std::uint32_t quality = 0;
    if (version != kScreenShareControlProtocolVersion || type != 2 ||
        !ReadU16(encoded, offset, &flags) || flags != 0 ||
        !ReadU64(encoded, offset, &decoded.sequence) ||
        !ReadU32(encoded, offset, &decoded.maxWidth) ||
        !ReadU32(encoded, offset, &decoded.maxHeight) ||
        !ReadU32(encoded, offset, &decoded.framesPerSecond) ||
        encoded.size() - offset < 4) {
        SetError(error, "Screen stream preference payload is invalid.");
        return false;
    }
    quality = encoded[offset++];
    offset += 3;
    if (!ReadU16(encoded, offset, &roomLength) ||
        !ReadU16(encoded, offset, &senderLength)) {
        SetError(error, "Screen stream preference lengths are invalid.");
        return false;
    }
    const bool originalSize = decoded.maxWidth == 0 && decoded.maxHeight == 0;
    const bool boundedSize = decoded.maxWidth > 0 && decoded.maxHeight > 0 &&
        decoded.maxWidth <= kMaximumScreenStreamWidth &&
        decoded.maxHeight <= kMaximumScreenStreamHeight;
    if (decoded.sequence == 0 || (!originalSize && !boundedSize) ||
        decoded.framesPerSecond < kMinimumScreenFrameRate ||
        decoded.framesPerSecond > kMaximumScreenFrameRate ||
        quality > static_cast<std::uint8_t>(ScreenQualityTier::kOriginal) ||
        roomLength == 0 || roomLength > kMaximumRoomIdBytes ||
        senderLength == 0 || senderLength > kMaximumDeviceIdBytes ||
        encoded.size() - offset !=
            static_cast<std::size_t>(roomLength) + senderLength) {
        SetError(error, "Screen stream preference values are invalid.");
        return false;
    }
    decoded.quality = static_cast<ScreenQualityTier>(quality);
    decoded.roomId.assign(
        reinterpret_cast<const char*>(encoded.data() + offset), roomLength);
    offset += roomLength;
    decoded.senderDeviceId.assign(
        reinterpret_cast<const char*>(encoded.data() + offset), senderLength);
    *request = std::move(decoded);
    return true;
}

bool EncodeScreenStreamPreferenceApplied(
    const ScreenStreamPreferenceApplied& applied,
    std::vector<std::uint8_t>* encoded,
    std::string* error)
{
    if (!encoded || applied.roomId.empty() ||
        applied.roomId.size() > kMaximumRoomIdBytes ||
        applied.senderDeviceId.empty() ||
        applied.senderDeviceId.size() > kMaximumDeviceIdBytes ||
        applied.requestSequence == 0 ||
        applied.error.size() > kMaximumErrorBytes) {
        SetError(error, "Screen stream preference result is invalid.");
        return false;
    }
    constexpr std::size_t kHeaderBytes = 42;
    const auto roomLength = static_cast<std::uint16_t>(applied.roomId.size());
    const auto senderLength =
        static_cast<std::uint16_t>(applied.senderDeviceId.size());
    const auto errorLength = static_cast<std::uint16_t>(applied.error.size());
    encoded->clear();
    encoded->reserve(kHeaderBytes + roomLength + senderLength + errorLength);
    encoded->insert(encoded->end(), kMagic.begin(), kMagic.end());
    encoded->push_back(kScreenShareControlProtocolVersion);
    encoded->push_back(3);  // Applied per-member stream preference.
    AppendU16(encoded, 0);
    AppendU64(encoded, applied.requestSequence);
    AppendU32(encoded, applied.width);
    AppendU32(encoded, applied.height);
    AppendU32(encoded, applied.framesPerSecond);
    AppendU32(encoded, applied.maxBitrateBps);
    encoded->push_back(static_cast<std::uint8_t>(applied.scaleBackend));
    encoded->push_back(applied.accepted ? 1 : 0);
    AppendU16(encoded, 0);
    AppendU16(encoded, roomLength);
    AppendU16(encoded, senderLength);
    AppendU16(encoded, errorLength);
    encoded->insert(encoded->end(), applied.roomId.begin(), applied.roomId.end());
    encoded->insert(encoded->end(), applied.senderDeviceId.begin(),
                    applied.senderDeviceId.end());
    encoded->insert(encoded->end(), applied.error.begin(), applied.error.end());
    return encoded->size() <= kMaximumMessageBytes;
}

bool DecodeScreenStreamPreferenceApplied(
    std::span<const std::uint8_t> encoded,
    ScreenStreamPreferenceApplied* applied,
    std::string* error)
{
    constexpr std::size_t kHeaderBytes = 42;
    if (!applied || encoded.size() < kHeaderBytes ||
        encoded.size() > kMaximumMessageBytes ||
        !std::equal(kMagic.begin(), kMagic.end(), encoded.begin())) {
        SetError(error, "Screen stream preference result header is invalid.");
        return false;
    }
    std::size_t offset = kMagic.size();
    const std::uint8_t version = encoded[offset++];
    const std::uint8_t type = encoded[offset++];
    std::uint16_t flags = 0;
    std::uint16_t reserved = 0;
    std::uint16_t roomLength = 0;
    std::uint16_t senderLength = 0;
    std::uint16_t errorLength = 0;
    ScreenStreamPreferenceApplied decoded;
    if (version != kScreenShareControlProtocolVersion || type != 3 ||
        !ReadU16(encoded, offset, &flags) || flags != 0 ||
        !ReadU64(encoded, offset, &decoded.requestSequence) ||
        !ReadU32(encoded, offset, &decoded.width) ||
        !ReadU32(encoded, offset, &decoded.height) ||
        !ReadU32(encoded, offset, &decoded.framesPerSecond) ||
        !ReadU32(encoded, offset, &decoded.maxBitrateBps) ||
        encoded.size() - offset < 2) {
        SetError(error, "Screen stream preference result payload is invalid.");
        return false;
    }
    decoded.scaleBackend = static_cast<ScreenScaleBackend>(encoded[offset++]);
    decoded.accepted = encoded[offset++] != 0;
    if (!ReadU16(encoded, offset, &reserved) || reserved != 0 ||
        !ReadU16(encoded, offset, &roomLength) ||
        !ReadU16(encoded, offset, &senderLength) ||
        !ReadU16(encoded, offset, &errorLength) ||
        decoded.requestSequence == 0 || roomLength == 0 ||
        roomLength > kMaximumRoomIdBytes || senderLength == 0 ||
        senderLength > kMaximumDeviceIdBytes ||
        errorLength > kMaximumErrorBytes ||
        encoded.size() - offset != static_cast<std::size_t>(roomLength) +
            senderLength + errorLength) {
        SetError(error, "Screen stream preference result values are invalid.");
        return false;
    }
    decoded.roomId.assign(
        reinterpret_cast<const char*>(encoded.data() + offset), roomLength);
    offset += roomLength;
    decoded.senderDeviceId.assign(
        reinterpret_cast<const char*>(encoded.data() + offset), senderLength);
    offset += senderLength;
    decoded.error.assign(
        reinterpret_cast<const char*>(encoded.data() + offset), errorLength);
    *applied = std::move(decoded);
    return true;
}

bool EncodeScreenStreamPreferenceAppliedV2(
    const ScreenStreamPreferenceApplied& applied,
    std::vector<std::uint8_t>* encoded,
    std::string* error)
{
    constexpr std::size_t kHeaderBytes = 50;
    if (!encoded || applied.roomId.empty() ||
        applied.roomId.size() > kMaximumRoomIdBytes ||
        applied.senderDeviceId.empty() ||
        applied.senderDeviceId.size() > kMaximumDeviceIdBytes ||
        applied.requestSequence == 0 ||
        applied.screenShareGeneration == 0 ||
        applied.error.size() > kMaximumErrorBytes) {
        SetError(error, "Screen stream preference V2 result is invalid.");
        return false;
    }
    const auto roomLength =
        static_cast<std::uint16_t>(applied.roomId.size());
    const auto senderLength =
        static_cast<std::uint16_t>(applied.senderDeviceId.size());
    const auto errorLength =
        static_cast<std::uint16_t>(applied.error.size());
    encoded->clear();
    encoded->reserve(
        kHeaderBytes + roomLength + senderLength + errorLength);
    encoded->insert(encoded->end(), kMagic.begin(), kMagic.end());
    encoded->push_back(kScreenShareControlProtocolVersion);
    encoded->push_back(11);  // Generation-bound preference result.
    AppendU16(encoded, 0);
    AppendU64(encoded, applied.requestSequence);
    AppendU64(encoded, applied.screenShareGeneration);
    AppendU32(encoded, applied.width);
    AppendU32(encoded, applied.height);
    AppendU32(encoded, applied.framesPerSecond);
    AppendU32(encoded, applied.maxBitrateBps);
    encoded->push_back(
        static_cast<std::uint8_t>(applied.scaleBackend));
    encoded->push_back(applied.accepted ? 1 : 0);
    AppendU16(encoded, 0);
    AppendU16(encoded, roomLength);
    AppendU16(encoded, senderLength);
    AppendU16(encoded, errorLength);
    encoded->insert(
        encoded->end(), applied.roomId.begin(), applied.roomId.end());
    encoded->insert(
        encoded->end(),
        applied.senderDeviceId.begin(),
        applied.senderDeviceId.end());
    encoded->insert(
        encoded->end(), applied.error.begin(), applied.error.end());
    return encoded->size() <= kMaximumMessageBytes;
}

bool DecodeScreenStreamPreferenceAppliedV2(
    std::span<const std::uint8_t> encoded,
    ScreenStreamPreferenceApplied* applied,
    std::string* error)
{
    constexpr std::size_t kHeaderBytes = 50;
    if (!applied || encoded.size() < kHeaderBytes ||
        encoded.size() > kMaximumMessageBytes ||
        !std::equal(kMagic.begin(), kMagic.end(), encoded.begin())) {
        SetError(error, "Screen stream preference V2 header is invalid.");
        return false;
    }
    std::size_t offset = kMagic.size();
    const std::uint8_t version = encoded[offset++];
    const std::uint8_t type = encoded[offset++];
    std::uint16_t flags = 0;
    std::uint16_t reserved = 0;
    std::uint16_t roomLength = 0;
    std::uint16_t senderLength = 0;
    std::uint16_t errorLength = 0;
    ScreenStreamPreferenceApplied decoded;
    if (version != kScreenShareControlProtocolVersion || type != 11 ||
        !ReadU16(encoded, offset, &flags) || flags != 0 ||
        !ReadU64(encoded, offset, &decoded.requestSequence) ||
        !ReadU64(
            encoded, offset, &decoded.screenShareGeneration) ||
        !ReadU32(encoded, offset, &decoded.width) ||
        !ReadU32(encoded, offset, &decoded.height) ||
        !ReadU32(encoded, offset, &decoded.framesPerSecond) ||
        !ReadU32(encoded, offset, &decoded.maxBitrateBps) ||
        encoded.size() - offset < 2) {
        SetError(error, "Screen stream preference V2 payload is invalid.");
        return false;
    }
    decoded.scaleBackend =
        static_cast<ScreenScaleBackend>(encoded[offset++]);
    decoded.accepted = encoded[offset++] != 0;
    if (!ReadU16(encoded, offset, &reserved) || reserved != 0 ||
        !ReadU16(encoded, offset, &roomLength) ||
        !ReadU16(encoded, offset, &senderLength) ||
        !ReadU16(encoded, offset, &errorLength) ||
        decoded.requestSequence == 0 ||
        decoded.screenShareGeneration == 0 ||
        roomLength == 0 || roomLength > kMaximumRoomIdBytes ||
        senderLength == 0 ||
        senderLength > kMaximumDeviceIdBytes ||
        errorLength > kMaximumErrorBytes ||
        encoded.size() - offset !=
            static_cast<std::size_t>(roomLength) +
                senderLength + errorLength) {
        SetError(error, "Screen stream preference V2 values are invalid.");
        return false;
    }
    decoded.roomId.assign(
        reinterpret_cast<const char*>(encoded.data() + offset),
        roomLength);
    offset += roomLength;
    decoded.senderDeviceId.assign(
        reinterpret_cast<const char*>(encoded.data() + offset),
        senderLength);
    offset += senderLength;
    decoded.error.assign(
        reinterpret_cast<const char*>(encoded.data() + offset),
        errorLength);
    *applied = std::move(decoded);
    return true;
}

bool EncodeScreenCaptureCapability(
    const ScreenCaptureCapability& capability,
    std::vector<std::uint8_t>* encoded,
    std::string* error)
{
    constexpr std::size_t kHeaderBytes = 16;
    if (!encoded || capability.roomId.empty() ||
        capability.roomId.size() > kMaximumRoomIdBytes ||
        capability.senderDeviceId.empty() ||
        capability.senderDeviceId.size() > kMaximumDeviceIdBytes ||
        capability.maximumFrameRate < kMinimumScreenFrameRate ||
        capability.maximumFrameRate > kMaximumScreenFrameRate) {
        SetError(error, "Screen capture capability is invalid.");
        return false;
    }
    const auto roomLength =
        static_cast<std::uint16_t>(capability.roomId.size());
    const auto senderLength =
        static_cast<std::uint16_t>(capability.senderDeviceId.size());
    encoded->clear();
    encoded->reserve(kHeaderBytes + roomLength + senderLength);
    encoded->insert(encoded->end(), kMagic.begin(), kMagic.end());
    encoded->push_back(kScreenShareControlProtocolVersion);
    encoded->push_back(4);  // Active desktop capture capability.
    AppendU16(encoded, 0);
    AppendU32(encoded, capability.maximumFrameRate);
    AppendU16(encoded, roomLength);
    AppendU16(encoded, senderLength);
    encoded->insert(
        encoded->end(), capability.roomId.begin(), capability.roomId.end());
    encoded->insert(encoded->end(), capability.senderDeviceId.begin(),
                    capability.senderDeviceId.end());
    return encoded->size() <= kMaximumMessageBytes;
}

bool DecodeScreenCaptureCapability(
    std::span<const std::uint8_t> encoded,
    ScreenCaptureCapability* capability,
    std::string* error)
{
    constexpr std::size_t kHeaderBytes = 16;
    if (!capability || encoded.size() < kHeaderBytes ||
        encoded.size() > kMaximumMessageBytes ||
        !std::equal(kMagic.begin(), kMagic.end(), encoded.begin())) {
        SetError(error, "Screen capture capability header is invalid.");
        return false;
    }
    std::size_t offset = kMagic.size();
    const std::uint8_t version = encoded[offset++];
    const std::uint8_t type = encoded[offset++];
    std::uint16_t flags = 0;
    std::uint16_t roomLength = 0;
    std::uint16_t senderLength = 0;
    ScreenCaptureCapability decoded;
    if (version != kScreenShareControlProtocolVersion || type != 4 ||
        !ReadU16(encoded, offset, &flags) || flags != 0 ||
        !ReadU32(encoded, offset, &decoded.maximumFrameRate) ||
        !ReadU16(encoded, offset, &roomLength) ||
        !ReadU16(encoded, offset, &senderLength) ||
        decoded.maximumFrameRate < kMinimumScreenFrameRate ||
        decoded.maximumFrameRate > kMaximumScreenFrameRate ||
        roomLength == 0 || roomLength > kMaximumRoomIdBytes ||
        senderLength == 0 || senderLength > kMaximumDeviceIdBytes ||
        encoded.size() - offset !=
            static_cast<std::size_t>(roomLength) + senderLength) {
        SetError(error, "Screen capture capability payload is invalid.");
        return false;
    }
    decoded.roomId.assign(
        reinterpret_cast<const char*>(encoded.data() + offset), roomLength);
    offset += roomLength;
    decoded.senderDeviceId.assign(
        reinterpret_cast<const char*>(encoded.data() + offset),
        senderLength);
    *capability = std::move(decoded);
    return true;
}

bool EncodeScreenCaptureRuntimeCapability(
    const ScreenCaptureRuntimeCapability& capability,
    std::vector<std::uint8_t>* encoded,
    std::string* error)
{
    constexpr std::size_t kHeaderBytes = 22;
    if (!encoded || capability.roomId.empty() ||
        capability.roomId.size() > kMaximumRoomIdBytes ||
        capability.senderDeviceId.empty() ||
        capability.senderDeviceId.size() > kMaximumDeviceIdBytes ||
        capability.maximumFrameRate < kMinimumScreenFrameRate ||
        capability.maximumFrameRate > kMaximumScreenFrameRate ||
        capability.configuredBackend.empty() ||
        capability.configuredBackend.size() > kMaximumBackendNameBytes ||
        capability.activeBackend.empty() ||
        capability.activeBackend.size() > kMaximumBackendNameBytes ||
        capability.fallbackReason.size() > kMaximumErrorBytes) {
        SetError(error, "Screen capture runtime capability is invalid.");
        return false;
    }

    const auto roomLength =
        static_cast<std::uint16_t>(capability.roomId.size());
    const auto senderLength =
        static_cast<std::uint16_t>(capability.senderDeviceId.size());
    const auto configuredLength =
        static_cast<std::uint16_t>(capability.configuredBackend.size());
    const auto activeLength =
        static_cast<std::uint16_t>(capability.activeBackend.size());
    const auto fallbackLength =
        static_cast<std::uint16_t>(capability.fallbackReason.size());
    encoded->clear();
    encoded->reserve(kHeaderBytes + roomLength + senderLength +
                     configuredLength + activeLength + fallbackLength);
    encoded->insert(encoded->end(), kMagic.begin(), kMagic.end());
    encoded->push_back(kScreenShareControlProtocolVersion);
    encoded->push_back(5);  // Detailed active capture backend report.
    AppendU16(encoded, 0);
    AppendU32(encoded, capability.maximumFrameRate);
    AppendU16(encoded, roomLength);
    AppendU16(encoded, senderLength);
    AppendU16(encoded, configuredLength);
    AppendU16(encoded, activeLength);
    AppendU16(encoded, fallbackLength);
    encoded->insert(encoded->end(), capability.roomId.begin(),
                    capability.roomId.end());
    encoded->insert(encoded->end(), capability.senderDeviceId.begin(),
                    capability.senderDeviceId.end());
    encoded->insert(encoded->end(), capability.configuredBackend.begin(),
                    capability.configuredBackend.end());
    encoded->insert(encoded->end(), capability.activeBackend.begin(),
                    capability.activeBackend.end());
    encoded->insert(encoded->end(), capability.fallbackReason.begin(),
                    capability.fallbackReason.end());
    if (encoded->size() > kMaximumMessageBytes) {
        encoded->clear();
        SetError(error, "Screen capture runtime capability is too large.");
        return false;
    }
    return true;
}

bool DecodeScreenCaptureRuntimeCapability(
    std::span<const std::uint8_t> encoded,
    ScreenCaptureRuntimeCapability* capability,
    std::string* error)
{
    constexpr std::size_t kHeaderBytes = 22;
    if (!capability || encoded.size() < kHeaderBytes ||
        encoded.size() > kMaximumMessageBytes ||
        !std::equal(kMagic.begin(), kMagic.end(), encoded.begin())) {
        SetError(error, "Screen capture runtime capability header is invalid.");
        return false;
    }

    std::size_t offset = kMagic.size();
    const std::uint8_t version = encoded[offset++];
    const std::uint8_t type = encoded[offset++];
    std::uint16_t flags = 0;
    std::uint16_t roomLength = 0;
    std::uint16_t senderLength = 0;
    std::uint16_t configuredLength = 0;
    std::uint16_t activeLength = 0;
    std::uint16_t fallbackLength = 0;
    ScreenCaptureRuntimeCapability decoded;
    if (version != kScreenShareControlProtocolVersion || type != 5 ||
        !ReadU16(encoded, offset, &flags) || flags != 0 ||
        !ReadU32(encoded, offset, &decoded.maximumFrameRate) ||
        !ReadU16(encoded, offset, &roomLength) ||
        !ReadU16(encoded, offset, &senderLength) ||
        !ReadU16(encoded, offset, &configuredLength) ||
        !ReadU16(encoded, offset, &activeLength) ||
        !ReadU16(encoded, offset, &fallbackLength) ||
        decoded.maximumFrameRate < kMinimumScreenFrameRate ||
        decoded.maximumFrameRate > kMaximumScreenFrameRate ||
        roomLength == 0 || roomLength > kMaximumRoomIdBytes ||
        senderLength == 0 || senderLength > kMaximumDeviceIdBytes ||
        configuredLength == 0 ||
        configuredLength > kMaximumBackendNameBytes ||
        activeLength == 0 || activeLength > kMaximumBackendNameBytes ||
        fallbackLength > kMaximumErrorBytes ||
        encoded.size() - offset != static_cast<std::size_t>(roomLength) +
            senderLength + configuredLength + activeLength + fallbackLength) {
        SetError(error, "Screen capture runtime capability payload is invalid.");
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
    decoded.configuredBackend = readString(configuredLength);
    decoded.activeBackend = readString(activeLength);
    decoded.fallbackReason = readString(fallbackLength);
    *capability = std::move(decoded);
    return true;
}

bool EncodeSharedDisplayLayout(
    const SharedDisplayLayout& layout,
    std::vector<std::uint8_t>* encoded,
    std::string* error)
{
    constexpr std::size_t kHeaderBytes = 64;
    const auto& display = layout.selectedDisplay;
    if (!encoded || layout.roomId.empty() ||
        layout.roomId.size() > kMaximumRoomIdBytes ||
        layout.senderDeviceId.empty() ||
        layout.senderDeviceId.size() > kMaximumDeviceIdBytes ||
        layout.screenShareGeneration == 0 ||
        layout.layoutVersion == 0 ||
        display.sessionDisplayId == 0 ||
        display.stableDisplayKey.empty() ||
        display.stableDisplayKey.size() > kMaximumDisplayKeyBytes ||
        display.friendlyName.size() > kMaximumDisplayNameBytes ||
        display.width == 0 || display.height == 0 ||
        display.dpiX == 0 || display.dpiY == 0 ||
        display.rotationDegrees > 270 ||
        display.rotationDegrees % 90 != 0) {
        SetError(error, "Shared display layout is invalid.");
        return false;
    }
    const auto roomLength =
        static_cast<std::uint16_t>(layout.roomId.size());
    const auto senderLength =
        static_cast<std::uint16_t>(layout.senderDeviceId.size());
    const auto keyLength =
        static_cast<std::uint16_t>(display.stableDisplayKey.size());
    const auto nameLength =
        static_cast<std::uint16_t>(display.friendlyName.size());
    encoded->clear();
    encoded->reserve(kHeaderBytes + roomLength + senderLength +
                     keyLength + nameLength);
    encoded->insert(encoded->end(), kMagic.begin(), kMagic.end());
    encoded->push_back(kScreenShareControlProtocolVersion);
    encoded->push_back(6);  // Selected display identity and physical layout.
    AppendU16(encoded, display.primary ? 1 : 0);
    AppendU64(encoded, layout.screenShareGeneration);
    AppendU64(encoded, layout.layoutVersion);
    AppendU32(encoded, display.sessionDisplayId);
    AppendU32(encoded, static_cast<std::uint32_t>(display.left));
    AppendU32(encoded, static_cast<std::uint32_t>(display.top));
    AppendU32(encoded, display.width);
    AppendU32(encoded, display.height);
    AppendU32(encoded, display.dpiX);
    AppendU32(encoded, display.dpiY);
    AppendU32(encoded, display.rotationDegrees);
    AppendU16(encoded, roomLength);
    AppendU16(encoded, senderLength);
    AppendU16(encoded, keyLength);
    AppendU16(encoded, nameLength);
    encoded->insert(encoded->end(), layout.roomId.begin(),
                    layout.roomId.end());
    encoded->insert(encoded->end(), layout.senderDeviceId.begin(),
                    layout.senderDeviceId.end());
    encoded->insert(encoded->end(), display.stableDisplayKey.begin(),
                    display.stableDisplayKey.end());
    encoded->insert(encoded->end(), display.friendlyName.begin(),
                    display.friendlyName.end());
    if (encoded->size() > kMaximumMessageBytes) {
        encoded->clear();
        SetError(error, "Shared display layout is too large.");
        return false;
    }
    return true;
}

bool DecodeSharedDisplayLayout(
    std::span<const std::uint8_t> encoded,
    SharedDisplayLayout* layout,
    std::string* error)
{
    constexpr std::size_t kHeaderBytes = 64;
    if (!layout || encoded.size() < kHeaderBytes ||
        encoded.size() > kMaximumMessageBytes ||
        !std::equal(kMagic.begin(), kMagic.end(), encoded.begin())) {
        SetError(error, "Shared display layout header is invalid.");
        return false;
    }
    std::size_t offset = kMagic.size();
    const std::uint8_t version = encoded[offset++];
    const std::uint8_t type = encoded[offset++];
    std::uint16_t flags = 0;
    std::uint16_t roomLength = 0;
    std::uint16_t senderLength = 0;
    std::uint16_t keyLength = 0;
    std::uint16_t nameLength = 0;
    std::uint32_t left = 0;
    std::uint32_t top = 0;
    SharedDisplayLayout decoded;
    auto& display = decoded.selectedDisplay;
    if (version != kScreenShareControlProtocolVersion || type != 6 ||
        !ReadU16(encoded, offset, &flags) || flags > 1 ||
        !ReadU64(encoded, offset, &decoded.screenShareGeneration) ||
        !ReadU64(encoded, offset, &decoded.layoutVersion) ||
        !ReadU32(encoded, offset, &display.sessionDisplayId) ||
        !ReadU32(encoded, offset, &left) ||
        !ReadU32(encoded, offset, &top) ||
        !ReadU32(encoded, offset, &display.width) ||
        !ReadU32(encoded, offset, &display.height) ||
        !ReadU32(encoded, offset, &display.dpiX) ||
        !ReadU32(encoded, offset, &display.dpiY) ||
        !ReadU32(encoded, offset, &display.rotationDegrees) ||
        !ReadU16(encoded, offset, &roomLength) ||
        !ReadU16(encoded, offset, &senderLength) ||
        !ReadU16(encoded, offset, &keyLength) ||
        !ReadU16(encoded, offset, &nameLength) ||
        decoded.screenShareGeneration == 0 ||
        decoded.layoutVersion == 0 ||
        display.sessionDisplayId == 0 ||
        display.width == 0 || display.height == 0 ||
        display.dpiX == 0 || display.dpiY == 0 ||
        display.rotationDegrees > 270 ||
        display.rotationDegrees % 90 != 0 ||
        roomLength == 0 || roomLength > kMaximumRoomIdBytes ||
        senderLength == 0 || senderLength > kMaximumDeviceIdBytes ||
        keyLength == 0 || keyLength > kMaximumDisplayKeyBytes ||
        nameLength > kMaximumDisplayNameBytes ||
        encoded.size() - offset !=
            static_cast<std::size_t>(roomLength) + senderLength +
                keyLength + nameLength) {
        SetError(error, "Shared display layout payload is invalid.");
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
    display.stableDisplayKey = readString(keyLength);
    display.friendlyName = readString(nameLength);
    display.left = static_cast<std::int32_t>(left);
    display.top = static_cast<std::int32_t>(top);
    display.primary = (flags & 1) != 0;
    display.scalePercent = static_cast<std::uint32_t>(
        (static_cast<std::uint64_t>(display.dpiX) * 100 + 48) / 96);
    *layout = std::move(decoded);
    return true;
}

bool EncodeSharedDisplayCatalog(
    const SharedDisplayCatalog& catalog,
    std::vector<std::uint8_t>* encoded,
    std::string* error)
{
    constexpr std::size_t kHeaderBytes = 32;
    constexpr std::size_t kEntryHeaderBytes = 24;
    if (!encoded || catalog.roomId.empty() ||
        catalog.roomId.size() > kMaximumRoomIdBytes ||
        catalog.senderDeviceId.empty() ||
        catalog.senderDeviceId.size() > kMaximumDeviceIdBytes ||
        catalog.screenShareGeneration == 0 ||
        catalog.layoutVersion == 0 || catalog.displays.empty() ||
        catalog.displays.size() > kMaximumDisplayCount) {
        SetError(error, "Shared display catalog is invalid.");
        return false;
    }
    std::size_t totalSize =
        kHeaderBytes + catalog.roomId.size() +
        catalog.senderDeviceId.size();
    for (const auto& display : catalog.displays) {
        if (display.sessionDisplayId == 0 ||
            display.stableDisplayKey.empty() ||
            display.stableDisplayKey.size() >
                kMaximumDisplayKeyBytes ||
            display.friendlyName.size() >
                kMaximumDisplayNameBytes ||
            display.width == 0 || display.height == 0 ||
            display.scalePercent == 0) {
            SetError(error, "Shared display catalog entry is invalid.");
            return false;
        }
        totalSize += kEntryHeaderBytes +
            display.stableDisplayKey.size() +
            display.friendlyName.size();
    }
    if (totalSize > kMaximumDisplayCatalogBytes) {
        SetError(error, "Shared display catalog is too large.");
        return false;
    }

    encoded->clear();
    encoded->reserve(totalSize);
    encoded->insert(encoded->end(), kMagic.begin(), kMagic.end());
    encoded->push_back(kScreenShareControlProtocolVersion);
    encoded->push_back(7);  // Available displays on the active sharer.
    AppendU16(encoded, 0);
    AppendU64(encoded, catalog.screenShareGeneration);
    AppendU64(encoded, catalog.layoutVersion);
    AppendU16(
        encoded,
        static_cast<std::uint16_t>(catalog.displays.size()));
    AppendU16(encoded,
              static_cast<std::uint16_t>(catalog.roomId.size()));
    AppendU16(
        encoded,
        static_cast<std::uint16_t>(
            catalog.senderDeviceId.size()));
    AppendU16(encoded, 0);
    encoded->insert(
        encoded->end(), catalog.roomId.begin(), catalog.roomId.end());
    encoded->insert(
        encoded->end(), catalog.senderDeviceId.begin(),
        catalog.senderDeviceId.end());
    for (const auto& display : catalog.displays) {
        AppendU32(encoded, display.sessionDisplayId);
        AppendU32(encoded, display.width);
        AppendU32(encoded, display.height);
        AppendU32(encoded, display.scalePercent);
        encoded->push_back(display.primary ? 1 : 0);
        encoded->push_back(0);
        AppendU16(
            encoded,
            static_cast<std::uint16_t>(
                display.stableDisplayKey.size()));
        AppendU16(
            encoded,
            static_cast<std::uint16_t>(
                display.friendlyName.size()));
        AppendU16(encoded, 0);
        encoded->insert(
            encoded->end(), display.stableDisplayKey.begin(),
            display.stableDisplayKey.end());
        encoded->insert(
            encoded->end(), display.friendlyName.begin(),
            display.friendlyName.end());
    }
    return true;
}

bool DecodeSharedDisplayCatalog(
    std::span<const std::uint8_t> encoded,
    SharedDisplayCatalog* catalog,
    std::string* error)
{
    constexpr std::size_t kHeaderBytes = 32;
    if (!catalog || encoded.size() < kHeaderBytes ||
        encoded.size() > kMaximumDisplayCatalogBytes ||
        !std::equal(kMagic.begin(), kMagic.end(), encoded.begin())) {
        SetError(error, "Shared display catalog header is invalid.");
        return false;
    }
    std::size_t offset = kMagic.size();
    const std::uint8_t version = encoded[offset++];
    const std::uint8_t type = encoded[offset++];
    std::uint16_t flags = 0;
    std::uint16_t displayCount = 0;
    std::uint16_t roomLength = 0;
    std::uint16_t senderLength = 0;
    std::uint16_t reserved = 0;
    SharedDisplayCatalog decoded;
    if (version != kScreenShareControlProtocolVersion || type != 7 ||
        !ReadU16(encoded, offset, &flags) || flags != 0 ||
        !ReadU64(
            encoded, offset, &decoded.screenShareGeneration) ||
        !ReadU64(encoded, offset, &decoded.layoutVersion) ||
        !ReadU16(encoded, offset, &displayCount) ||
        !ReadU16(encoded, offset, &roomLength) ||
        !ReadU16(encoded, offset, &senderLength) ||
        !ReadU16(encoded, offset, &reserved) || reserved != 0 ||
        decoded.screenShareGeneration == 0 ||
        decoded.layoutVersion == 0 || displayCount == 0 ||
        displayCount > kMaximumDisplayCount ||
        roomLength == 0 || roomLength > kMaximumRoomIdBytes ||
        senderLength == 0 ||
        senderLength > kMaximumDeviceIdBytes ||
        encoded.size() - offset <
            static_cast<std::size_t>(roomLength) + senderLength) {
        SetError(error, "Shared display catalog payload is invalid.");
        return false;
    }
    auto readString = [&encoded, &offset](std::uint16_t length) {
        std::string value(
            reinterpret_cast<const char*>(encoded.data() + offset),
            length);
        offset += length;
        return value;
    };
    decoded.roomId = readString(roomLength);
    decoded.senderDeviceId = readString(senderLength);
    decoded.displays.reserve(displayCount);
    for (std::uint16_t index = 0; index < displayCount; ++index) {
        constexpr std::size_t kEntryHeaderBytes = 24;
        if (offset > encoded.size() ||
            encoded.size() - offset < kEntryHeaderBytes) {
            SetError(error, "Shared display catalog entry is truncated.");
            return false;
        }
        DisplayDescriptor display;
        if (!ReadU32(encoded, offset, &display.sessionDisplayId) ||
            !ReadU32(encoded, offset, &display.width) ||
            !ReadU32(encoded, offset, &display.height) ||
            !ReadU32(encoded, offset, &display.scalePercent)) {
            SetError(error, "Shared display catalog entry is invalid.");
            return false;
        }
        const std::uint8_t primary = encoded[offset++];
        const std::uint8_t entryReservedByte = encoded[offset++];
        std::uint16_t keyLength = 0;
        std::uint16_t nameLength = 0;
        std::uint16_t entryReserved = 0;
        if (primary > 1 || entryReservedByte != 0 ||
            !ReadU16(encoded, offset, &keyLength) ||
            !ReadU16(encoded, offset, &nameLength) ||
            !ReadU16(encoded, offset, &entryReserved) ||
            entryReserved != 0 ||
            display.sessionDisplayId == 0 ||
            display.width == 0 || display.height == 0 ||
            display.scalePercent == 0 || keyLength == 0 ||
            keyLength > kMaximumDisplayKeyBytes ||
            nameLength > kMaximumDisplayNameBytes ||
            offset > encoded.size() ||
            encoded.size() - offset <
                static_cast<std::size_t>(keyLength) + nameLength) {
            SetError(error, "Shared display catalog entry values are invalid.");
            return false;
        }
        display.primary = primary != 0;
        display.stableDisplayKey = readString(keyLength);
        display.friendlyName = readString(nameLength);
        if (std::any_of(
                decoded.displays.begin(), decoded.displays.end(),
                [&display](const DisplayDescriptor& current) {
                    return current.sessionDisplayId ==
                               display.sessionDisplayId ||
                           current.stableDisplayKey ==
                               display.stableDisplayKey;
                })) {
            SetError(error, "Shared display catalog contains duplicates.");
            return false;
        }
        decoded.displays.push_back(std::move(display));
    }
    if (offset != encoded.size()) {
        SetError(error, "Shared display catalog has trailing bytes.");
        return false;
    }
    *catalog = std::move(decoded);
    return true;
}

bool EncodeSharedDisplaySwitchRequest(
    const SharedDisplaySwitchRequest& request,
    std::vector<std::uint8_t>* encoded,
    std::string* error)
{
    constexpr std::size_t kHeaderBytes = 32;
    if (!encoded || request.roomId.empty() ||
        request.roomId.size() > kMaximumRoomIdBytes ||
        request.senderDeviceId.empty() ||
        request.senderDeviceId.size() > kMaximumDeviceIdBytes ||
        request.sequence == 0 ||
        request.screenShareGeneration == 0 ||
        request.stableDisplayKey.empty() ||
        request.stableDisplayKey.size() >
            kMaximumDisplayKeyBytes) {
        SetError(error, "Shared display switch request is invalid.");
        return false;
    }
    encoded->clear();
    encoded->reserve(
        kHeaderBytes + request.roomId.size() +
        request.senderDeviceId.size() +
        request.stableDisplayKey.size());
    encoded->insert(encoded->end(), kMagic.begin(), kMagic.end());
    encoded->push_back(kScreenShareControlProtocolVersion);
    encoded->push_back(8);  // Switch the active shared display.
    AppendU16(encoded, 0);
    AppendU64(encoded, request.sequence);
    AppendU64(encoded, request.screenShareGeneration);
    AppendU16(
        encoded,
        static_cast<std::uint16_t>(request.roomId.size()));
    AppendU16(
        encoded,
        static_cast<std::uint16_t>(
            request.senderDeviceId.size()));
    AppendU16(
        encoded,
        static_cast<std::uint16_t>(
            request.stableDisplayKey.size()));
    AppendU16(encoded, 0);
    encoded->insert(
        encoded->end(), request.roomId.begin(), request.roomId.end());
    encoded->insert(
        encoded->end(), request.senderDeviceId.begin(),
        request.senderDeviceId.end());
    encoded->insert(
        encoded->end(), request.stableDisplayKey.begin(),
        request.stableDisplayKey.end());
    return encoded->size() <= kMaximumMessageBytes;
}

bool DecodeSharedDisplaySwitchRequest(
    std::span<const std::uint8_t> encoded,
    SharedDisplaySwitchRequest* request,
    std::string* error)
{
    constexpr std::size_t kHeaderBytes = 32;
    if (!request || encoded.size() < kHeaderBytes ||
        encoded.size() > kMaximumMessageBytes ||
        !std::equal(kMagic.begin(), kMagic.end(), encoded.begin())) {
        SetError(error, "Shared display switch request header is invalid.");
        return false;
    }
    std::size_t offset = kMagic.size();
    const std::uint8_t version = encoded[offset++];
    const std::uint8_t type = encoded[offset++];
    std::uint16_t flags = 0;
    std::uint16_t roomLength = 0;
    std::uint16_t senderLength = 0;
    std::uint16_t keyLength = 0;
    std::uint16_t reserved = 0;
    SharedDisplaySwitchRequest decoded;
    if (version != kScreenShareControlProtocolVersion || type != 8 ||
        !ReadU16(encoded, offset, &flags) || flags != 0 ||
        !ReadU64(encoded, offset, &decoded.sequence) ||
        !ReadU64(
            encoded, offset, &decoded.screenShareGeneration) ||
        !ReadU16(encoded, offset, &roomLength) ||
        !ReadU16(encoded, offset, &senderLength) ||
        !ReadU16(encoded, offset, &keyLength) ||
        !ReadU16(encoded, offset, &reserved) || reserved != 0 ||
        decoded.sequence == 0 ||
        decoded.screenShareGeneration == 0 ||
        roomLength == 0 || roomLength > kMaximumRoomIdBytes ||
        senderLength == 0 ||
        senderLength > kMaximumDeviceIdBytes ||
        keyLength == 0 || keyLength > kMaximumDisplayKeyBytes ||
        encoded.size() - offset !=
            static_cast<std::size_t>(roomLength) + senderLength +
                keyLength) {
        SetError(error, "Shared display switch request payload is invalid.");
        return false;
    }
    auto readString = [&encoded, &offset](std::uint16_t length) {
        std::string value(
            reinterpret_cast<const char*>(encoded.data() + offset),
            length);
        offset += length;
        return value;
    };
    decoded.roomId = readString(roomLength);
    decoded.senderDeviceId = readString(senderLength);
    decoded.stableDisplayKey = readString(keyLength);
    *request = std::move(decoded);
    return true;
}

bool EncodeSharedDisplaySwitchApplied(
    const SharedDisplaySwitchApplied& applied,
    std::vector<std::uint8_t>* encoded,
    std::string* error)
{
    constexpr std::size_t kHeaderBytes = 32;
    if (!encoded || applied.roomId.empty() ||
        applied.roomId.size() > kMaximumRoomIdBytes ||
        applied.senderDeviceId.empty() ||
        applied.senderDeviceId.size() > kMaximumDeviceIdBytes ||
        applied.requestSequence == 0 ||
        applied.screenShareGeneration == 0 ||
        applied.stableDisplayKey.size() >
            kMaximumDisplayKeyBytes ||
        applied.error.size() > kMaximumErrorBytes ||
        (applied.accepted && applied.stableDisplayKey.empty())) {
        SetError(error, "Shared display switch result is invalid.");
        return false;
    }
    encoded->clear();
    encoded->reserve(
        kHeaderBytes + applied.roomId.size() +
        applied.senderDeviceId.size() +
        applied.stableDisplayKey.size() + applied.error.size());
    encoded->insert(encoded->end(), kMagic.begin(), kMagic.end());
    encoded->push_back(kScreenShareControlProtocolVersion);
    encoded->push_back(9);  // Shared display switch result.
    encoded->push_back(applied.accepted ? 1 : 0);
    encoded->push_back(0);
    AppendU64(encoded, applied.requestSequence);
    AppendU64(encoded, applied.screenShareGeneration);
    AppendU16(
        encoded,
        static_cast<std::uint16_t>(applied.roomId.size()));
    AppendU16(
        encoded,
        static_cast<std::uint16_t>(
            applied.senderDeviceId.size()));
    AppendU16(
        encoded,
        static_cast<std::uint16_t>(
            applied.stableDisplayKey.size()));
    AppendU16(
        encoded,
        static_cast<std::uint16_t>(applied.error.size()));
    encoded->insert(
        encoded->end(), applied.roomId.begin(), applied.roomId.end());
    encoded->insert(
        encoded->end(), applied.senderDeviceId.begin(),
        applied.senderDeviceId.end());
    encoded->insert(
        encoded->end(), applied.stableDisplayKey.begin(),
        applied.stableDisplayKey.end());
    encoded->insert(
        encoded->end(), applied.error.begin(), applied.error.end());
    return encoded->size() <= kMaximumMessageBytes;
}

bool DecodeSharedDisplaySwitchApplied(
    std::span<const std::uint8_t> encoded,
    SharedDisplaySwitchApplied* applied,
    std::string* error)
{
    constexpr std::size_t kHeaderBytes = 32;
    if (!applied || encoded.size() < kHeaderBytes ||
        encoded.size() > kMaximumMessageBytes ||
        !std::equal(kMagic.begin(), kMagic.end(), encoded.begin())) {
        SetError(error, "Shared display switch result header is invalid.");
        return false;
    }
    std::size_t offset = kMagic.size();
    const std::uint8_t version = encoded[offset++];
    const std::uint8_t type = encoded[offset++];
    const std::uint8_t accepted = encoded[offset++];
    const std::uint8_t reserved = encoded[offset++];
    std::uint16_t roomLength = 0;
    std::uint16_t senderLength = 0;
    std::uint16_t keyLength = 0;
    std::uint16_t errorLength = 0;
    SharedDisplaySwitchApplied decoded;
    decoded.accepted = accepted == 1;
    if (version != kScreenShareControlProtocolVersion || type != 9 ||
        accepted > 1 || reserved != 0 ||
        !ReadU64(encoded, offset, &decoded.requestSequence) ||
        !ReadU64(
            encoded, offset, &decoded.screenShareGeneration) ||
        !ReadU16(encoded, offset, &roomLength) ||
        !ReadU16(encoded, offset, &senderLength) ||
        !ReadU16(encoded, offset, &keyLength) ||
        !ReadU16(encoded, offset, &errorLength) ||
        decoded.requestSequence == 0 ||
        decoded.screenShareGeneration == 0 ||
        roomLength == 0 || roomLength > kMaximumRoomIdBytes ||
        senderLength == 0 ||
        senderLength > kMaximumDeviceIdBytes ||
        keyLength > kMaximumDisplayKeyBytes ||
        errorLength > kMaximumErrorBytes ||
        (decoded.accepted && keyLength == 0) ||
        encoded.size() - offset !=
            static_cast<std::size_t>(roomLength) + senderLength +
                keyLength + errorLength) {
        SetError(error, "Shared display switch result payload is invalid.");
        return false;
    }
    auto readString = [&encoded, &offset](std::uint16_t length) {
        std::string value(
            reinterpret_cast<const char*>(encoded.data() + offset),
            length);
        offset += length;
        return value;
    };
    decoded.roomId = readString(roomLength);
    decoded.senderDeviceId = readString(senderLength);
    decoded.stableDisplayKey = readString(keyLength);
    decoded.error = readString(errorLength);
    *applied = std::move(decoded);
    return true;
}

bool EncodeScreenRefreshRequest(
    const ScreenRefreshRequest& request,
    std::vector<std::uint8_t>* encoded,
    std::string* error)
{
    constexpr std::size_t kHeaderBytes = 32;
    if (!encoded || request.roomId.empty() ||
        request.roomId.size() > kMaximumRoomIdBytes ||
        request.senderDeviceId.empty() ||
        request.senderDeviceId.size() > kMaximumDeviceIdBytes ||
        request.sequence == 0 || request.screenShareGeneration == 0) {
        SetError(error, "Screen refresh request is invalid.");
        return false;
    }
    const auto roomLength =
        static_cast<std::uint16_t>(request.roomId.size());
    const auto senderLength =
        static_cast<std::uint16_t>(request.senderDeviceId.size());
    const std::size_t totalSize =
        kHeaderBytes + roomLength + senderLength;
    if (totalSize > kMaximumMessageBytes) {
        SetError(error, "Screen refresh request is too large.");
        return false;
    }

    encoded->clear();
    encoded->reserve(totalSize);
    encoded->insert(encoded->end(), kMagic.begin(), kMagic.end());
    encoded->push_back(kScreenShareControlProtocolVersion);
    encoded->push_back(10);  // Viewer sink attached; refresh screen stream.
    AppendU16(encoded, 0);
    AppendU64(encoded, request.sequence);
    AppendU64(encoded, request.screenShareGeneration);
    AppendU16(encoded, roomLength);
    AppendU16(encoded, senderLength);
    AppendU32(encoded, 0);
    encoded->insert(
        encoded->end(), request.roomId.begin(), request.roomId.end());
    encoded->insert(
        encoded->end(),
        request.senderDeviceId.begin(),
        request.senderDeviceId.end());
    return true;
}

bool DecodeScreenRefreshRequest(
    std::span<const std::uint8_t> encoded,
    ScreenRefreshRequest* request,
    std::string* error)
{
    constexpr std::size_t kHeaderBytes = 32;
    if (!request || encoded.size() < kHeaderBytes ||
        encoded.size() > kMaximumMessageBytes ||
        !std::equal(kMagic.begin(), kMagic.end(), encoded.begin())) {
        SetError(error, "Screen refresh request header is invalid.");
        return false;
    }

    std::size_t offset = kMagic.size();
    const std::uint8_t version = encoded[offset++];
    const std::uint8_t type = encoded[offset++];
    std::uint16_t flags = 0;
    std::uint16_t roomLength = 0;
    std::uint16_t senderLength = 0;
    std::uint32_t reserved = 0;
    ScreenRefreshRequest decoded;
    if (version != kScreenShareControlProtocolVersion || type != 10 ||
        !ReadU16(encoded, offset, &flags) || flags != 0 ||
        !ReadU64(encoded, offset, &decoded.sequence) ||
        !ReadU64(
            encoded, offset, &decoded.screenShareGeneration) ||
        !ReadU16(encoded, offset, &roomLength) ||
        !ReadU16(encoded, offset, &senderLength) ||
        !ReadU32(encoded, offset, &reserved) || reserved != 0 ||
        decoded.sequence == 0 ||
        decoded.screenShareGeneration == 0 ||
        roomLength == 0 || roomLength > kMaximumRoomIdBytes ||
        senderLength == 0 || senderLength > kMaximumDeviceIdBytes ||
        encoded.size() - offset !=
            static_cast<std::size_t>(roomLength) + senderLength) {
        SetError(error, "Screen refresh request payload is invalid.");
        return false;
    }

    decoded.roomId.assign(
        reinterpret_cast<const char*>(encoded.data() + offset),
        roomLength);
    offset += roomLength;
    decoded.senderDeviceId.assign(
        reinterpret_cast<const char*>(encoded.data() + offset),
        senderLength);
    *request = std::move(decoded);
    return true;
}

bool EncodeScreenFirstFramePresented(
    const ScreenFirstFramePresented& presented,
    std::vector<std::uint8_t>* encoded,
    std::string* error)
{
    constexpr std::size_t kHeaderBytes = 32;
    if (!encoded || presented.roomId.empty() ||
        presented.roomId.size() > kMaximumRoomIdBytes ||
        presented.senderDeviceId.empty() ||
        presented.senderDeviceId.size() > kMaximumDeviceIdBytes ||
        presented.sequence == 0 ||
        presented.screenShareGeneration == 0) {
        SetError(error, "Screen first-frame presentation is invalid.");
        return false;
    }
    const auto roomLength =
        static_cast<std::uint16_t>(presented.roomId.size());
    const auto senderLength =
        static_cast<std::uint16_t>(presented.senderDeviceId.size());
    const std::size_t totalSize =
        kHeaderBytes + roomLength + senderLength;
    if (totalSize > kMaximumMessageBytes) {
        SetError(error, "Screen first-frame presentation is too large.");
        return false;
    }

    encoded->clear();
    encoded->reserve(totalSize);
    encoded->insert(encoded->end(), kMagic.begin(), kMagic.end());
    encoded->push_back(kScreenShareControlProtocolVersion);
    encoded->push_back(12);  // First screen frame actually presented.
    AppendU16(encoded, 0);
    AppendU64(encoded, presented.sequence);
    AppendU64(encoded, presented.screenShareGeneration);
    AppendU16(encoded, roomLength);
    AppendU16(encoded, senderLength);
    AppendU32(encoded, presented.startupElapsedMs);
    encoded->insert(
        encoded->end(), presented.roomId.begin(), presented.roomId.end());
    encoded->insert(encoded->end(),
                    presented.senderDeviceId.begin(),
                    presented.senderDeviceId.end());
    return true;
}

bool DecodeScreenFirstFramePresented(
    std::span<const std::uint8_t> encoded,
    ScreenFirstFramePresented* presented,
    std::string* error)
{
    constexpr std::size_t kHeaderBytes = 32;
    if (!presented || encoded.size() < kHeaderBytes ||
        encoded.size() > kMaximumMessageBytes ||
        !std::equal(kMagic.begin(), kMagic.end(), encoded.begin())) {
        SetError(error, "Screen first-frame presentation header is invalid.");
        return false;
    }

    std::size_t offset = kMagic.size();
    const std::uint8_t version = encoded[offset++];
    const std::uint8_t type = encoded[offset++];
    std::uint16_t flags = 0;
    std::uint16_t roomLength = 0;
    std::uint16_t senderLength = 0;
    ScreenFirstFramePresented decoded;
    if (version != kScreenShareControlProtocolVersion || type != 12 ||
        !ReadU16(encoded, offset, &flags) || flags != 0 ||
        !ReadU64(encoded, offset, &decoded.sequence) ||
        !ReadU64(encoded, offset, &decoded.screenShareGeneration) ||
        !ReadU16(encoded, offset, &roomLength) ||
        !ReadU16(encoded, offset, &senderLength) ||
        !ReadU32(encoded, offset, &decoded.startupElapsedMs) ||
        decoded.sequence == 0 || decoded.screenShareGeneration == 0 ||
        roomLength == 0 || roomLength > kMaximumRoomIdBytes ||
        senderLength == 0 || senderLength > kMaximumDeviceIdBytes ||
        encoded.size() - offset !=
            static_cast<std::size_t>(roomLength) + senderLength) {
        SetError(error, "Screen first-frame presentation payload is invalid.");
        return false;
    }

    decoded.roomId.assign(
        reinterpret_cast<const char*>(encoded.data() + offset), roomLength);
    offset += roomLength;
    decoded.senderDeviceId.assign(
        reinterpret_cast<const char*>(encoded.data() + offset), senderLength);
    *presented = std::move(decoded);
    return true;
}

}  // namespace remote
