// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <span>
#include <string>
#include <vector>

namespace remote {

inline constexpr std::uint8_t kClipboardProtocolVersion = 9;
inline constexpr std::size_t kMaximumClipboardMessageBytes = 64 * 1024;
inline constexpr std::size_t kMaximumClipboardChunkBytes = 60 * 1024;
inline constexpr std::size_t kMaximumClipboardDescriptors = 256;
inline constexpr std::uint64_t kMaximumClipboardInlineBytes = 48ull * 1024 * 1024;
inline constexpr std::uint64_t kMaximumClipboardFileBytes =
    512ull * 1024 * 1024;

enum class ClipboardMessageType : std::uint8_t {
    kCapabilities = 1,
    kSessionStart = 2,
    kSessionStop = 3,
    kOffer = 4,
    kAccept = 5,
    kReject = 6,
    kChunk = 7,
    kCommit = 8,
    kApplyResult = 9,
    kCancel = 10,
    kRequestCurrent = 11,
    kClear = 12,
    kResync = 13,
    kProgress = 14,
    // Announces a new user-originated current clipboard item without sending
    // its body. The peer asks for it with kRequestCurrent only when a local
    // application actually pastes.
    kClipboardChanged = 15,
};

enum class ClipboardFormat : std::uint8_t {
    kUnicodeText = 1,
    kHtml = 2,
    kRtf = 3,
    kPng = 4,
    kFile = 5,
    kDirectory = 6,
};

enum ClipboardCapability : std::uint32_t {
    kClipboardCapabilityUnicodeText = 1u << 0,
    kClipboardCapabilityHtml = 1u << 1,
    kClipboardCapabilityRtf = 1u << 2,
    kClipboardCapabilityPng = 1u << 3,
    kClipboardCapabilityFiles = 1u << 4,
    kClipboardCapabilityClear = 1u << 5,
};

struct ClipboardFormatDescriptor {
    ClipboardFormat format = ClipboardFormat::kUnicodeText;
    std::string name;
    std::uint64_t size = 0;
    std::array<std::uint8_t, 32> sha256{};
    bool hasSha256 = false;
    bool topLevel = false;
};

struct ClipboardMessage {
    ClipboardMessageType type = ClipboardMessageType::kCapabilities;
    std::uint32_t capabilities = 0;
    bool enabled = false;
    bool accepted = false;
    std::string itemId;
    std::uint64_t lamportVersion = 0;
    std::uint32_t descriptorIndex = 0;
    std::uint64_t offset = 0;
    std::uint64_t totalSize = 0;
    std::vector<ClipboardFormatDescriptor> descriptors;
    std::vector<std::uint8_t> data;
    std::string errorCode;
    std::string errorMessage;
};

struct ClipboardEnvelope {
    std::string roomId;
    std::string senderDeviceId;
    std::string receiverDeviceId;
    std::string controlGrantId;
    std::string clipboardSessionId;
    std::uint64_t sequence = 0;
    std::uint64_t timestampUs = 0;
    ClipboardMessage message;
};

class IClipboardSink {
public:
    virtual ~IClipboardSink() = default;
    virtual void OnClipboardMessage(const ClipboardEnvelope& envelope) = 0;
};

bool EncodeClipboardMessage(const ClipboardEnvelope& input,
                            std::vector<std::uint8_t>* encoded,
                            std::string* error = nullptr);

bool DecodeClipboardMessage(std::span<const std::uint8_t> encoded,
                            ClipboardEnvelope* output,
                            std::string* error = nullptr);

bool IsClipboardTransferMessage(ClipboardMessageType type);

}  // namespace remote
