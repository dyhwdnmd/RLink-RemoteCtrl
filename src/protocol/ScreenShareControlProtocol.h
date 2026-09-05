// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#pragma once

#include <cstddef>
#include <cstdint>
#include <span>
#include <string>
#include <vector>

#include "src/core/DisplayTopology.h"

namespace remote {

inline constexpr std::uint8_t kScreenShareControlProtocolVersion = 1;
inline constexpr std::uint32_t kMinimumScreenFrameRate = 5;
inline constexpr std::uint32_t kDefaultScreenFrameRate = 60;
inline constexpr std::uint32_t kMaximumScreenFrameRate = 120;
inline constexpr std::uint32_t kMultiMemberMaximumScreenFrameRate = 60;
inline constexpr std::size_t kHighOccupancyRoomMemberThreshold = 2;
inline constexpr std::uint32_t kMaximumScreenStreamWidth = 7680;
inline constexpr std::uint32_t kMaximumScreenStreamHeight = 4320;

enum class ScreenQualityTier : std::uint8_t {
    kAutomatic = 0,
    k720p = 1,
    k1080p = 2,
    k1440p = 3,
    kOriginal = 4,
};

enum class ScreenScaleBackend : std::uint8_t {
    kUnknown = 0,
    kWebRtc = 1,
    kD3D11 = 2,
    kCpu = 3,
};

struct ScreenFrameRateRequest {
    std::string roomId;
    std::string senderDeviceId;
    std::uint64_t sequence = 0;
    std::uint32_t framesPerSecond = kDefaultScreenFrameRate;
};

// width/height are a bounding box. A zero pair requests the original source
// size. The sender preserves the desktop aspect ratio and uses even output
// dimensions required by H264/NV12.
struct ScreenStreamPreferenceRequest {
    std::string roomId;
    std::string senderDeviceId;
    std::uint64_t sequence = 0;
    std::uint32_t maxWidth = 0;
    std::uint32_t maxHeight = 0;
    std::uint32_t framesPerSecond = kDefaultScreenFrameRate;
    ScreenQualityTier quality = ScreenQualityTier::kAutomatic;
};

struct ScreenStreamPreferenceApplied {
    std::string roomId;
    std::string senderDeviceId;
    std::uint64_t requestSequence = 0;
    std::uint64_t screenShareGeneration = 0;
    std::uint32_t width = 0;
    std::uint32_t height = 0;
    std::uint32_t framesPerSecond = 0;
    std::uint32_t maxBitrateBps = 0;
    ScreenScaleBackend scaleBackend = ScreenScaleBackend::kUnknown;
    bool accepted = false;
    std::string error;
};

struct ScreenCaptureCapability {
    std::string roomId;
    std::string senderDeviceId;
    std::uint32_t maximumFrameRate = 60;
};

// The legacy type-4 capability above remains in use for compatibility. This
// detailed type-5 report explains why the active sender exposes that limit.
struct ScreenCaptureRuntimeCapability {
    std::string roomId;
    std::string senderDeviceId;
    std::uint32_t maximumFrameRate = 60;
    std::string configuredBackend;
    std::string activeBackend;
    std::string fallbackReason;
};

struct SharedDisplayLayout {
    std::string roomId;
    std::string senderDeviceId;
    std::uint64_t screenShareGeneration = 0;
    std::uint64_t layoutVersion = 0;
    DisplayDescriptor selectedDisplay;
};

struct SharedDisplayCatalog {
    std::string roomId;
    std::string senderDeviceId;
    std::uint64_t screenShareGeneration = 0;
    std::uint64_t layoutVersion = 0;
    std::vector<DisplayDescriptor> displays;
};

struct SharedDisplaySwitchRequest {
    std::string roomId;
    std::string senderDeviceId;
    std::uint64_t sequence = 0;
    std::uint64_t screenShareGeneration = 0;
    std::string stableDisplayKey;
};

struct SharedDisplaySwitchApplied {
    std::string roomId;
    std::string senderDeviceId;
    std::uint64_t requestSequence = 0;
    std::uint64_t screenShareGeneration = 0;
    std::string stableDisplayKey;
    bool accepted = false;
    std::string error;
};

// Sent by a viewer immediately after binding the screen sink. The sharer
// answers it by producing a bounded full-frame burst; the receiver separately
// requests a codec keyframe through WebRTC.
struct ScreenRefreshRequest {
    std::string roomId;
    std::string senderDeviceId;
    std::uint64_t sequence = 0;
    std::uint64_t screenShareGeneration = 0;
};

// Sent by a viewer only after the first frame of a screen-share generation
// has actually been presented. This is intentionally later than RTP receipt
// or decode completion, so the sharer can stop startup retries and release
// lower-priority clipboard warmup traffic only after the user sees the image.
struct ScreenFirstFramePresented {
    std::string roomId;
    std::string senderDeviceId;
    std::uint64_t sequence = 0;
    std::uint64_t screenShareGeneration = 0;
    std::uint32_t startupElapsedMs = 0;
};

bool EncodeScreenFrameRateRequest(
    const ScreenFrameRateRequest& request,
    std::vector<std::uint8_t>* encoded,
    std::string* error = nullptr);

bool DecodeScreenFrameRateRequest(
    std::span<const std::uint8_t> encoded,
    ScreenFrameRateRequest* request,
    std::string* error = nullptr);

bool EncodeScreenStreamPreferenceRequest(
    const ScreenStreamPreferenceRequest& request,
    std::vector<std::uint8_t>* encoded,
    std::string* error = nullptr);

bool DecodeScreenStreamPreferenceRequest(
    std::span<const std::uint8_t> encoded,
    ScreenStreamPreferenceRequest* request,
    std::string* error = nullptr);

bool EncodeScreenStreamPreferenceApplied(
    const ScreenStreamPreferenceApplied& applied,
    std::vector<std::uint8_t>* encoded,
    std::string* error = nullptr);

bool DecodeScreenStreamPreferenceApplied(
    std::span<const std::uint8_t> encoded,
    ScreenStreamPreferenceApplied* applied,
    std::string* error = nullptr);

// Type-11 extends the legacy type-3 result with the authoritative screen
// share generation. New peers send both messages (V2 first) so older peers
// continue to receive type-3 while current peers can reject stale results.
bool EncodeScreenStreamPreferenceAppliedV2(
    const ScreenStreamPreferenceApplied& applied,
    std::vector<std::uint8_t>* encoded,
    std::string* error = nullptr);

bool DecodeScreenStreamPreferenceAppliedV2(
    std::span<const std::uint8_t> encoded,
    ScreenStreamPreferenceApplied* applied,
    std::string* error = nullptr);

bool EncodeScreenCaptureCapability(
    const ScreenCaptureCapability& capability,
    std::vector<std::uint8_t>* encoded,
    std::string* error = nullptr);

bool DecodeScreenCaptureCapability(
    std::span<const std::uint8_t> encoded,
    ScreenCaptureCapability* capability,
    std::string* error = nullptr);

bool EncodeScreenCaptureRuntimeCapability(
    const ScreenCaptureRuntimeCapability& capability,
    std::vector<std::uint8_t>* encoded,
    std::string* error = nullptr);

bool DecodeScreenCaptureRuntimeCapability(
    std::span<const std::uint8_t> encoded,
    ScreenCaptureRuntimeCapability* capability,
    std::string* error = nullptr);

bool EncodeSharedDisplayLayout(
    const SharedDisplayLayout& layout,
    std::vector<std::uint8_t>* encoded,
    std::string* error = nullptr);

bool DecodeSharedDisplayLayout(
    std::span<const std::uint8_t> encoded,
    SharedDisplayLayout* layout,
    std::string* error = nullptr);

bool EncodeSharedDisplayCatalog(
    const SharedDisplayCatalog& catalog,
    std::vector<std::uint8_t>* encoded,
    std::string* error = nullptr);

bool DecodeSharedDisplayCatalog(
    std::span<const std::uint8_t> encoded,
    SharedDisplayCatalog* catalog,
    std::string* error = nullptr);

bool EncodeSharedDisplaySwitchRequest(
    const SharedDisplaySwitchRequest& request,
    std::vector<std::uint8_t>* encoded,
    std::string* error = nullptr);

bool DecodeSharedDisplaySwitchRequest(
    std::span<const std::uint8_t> encoded,
    SharedDisplaySwitchRequest* request,
    std::string* error = nullptr);

bool EncodeSharedDisplaySwitchApplied(
    const SharedDisplaySwitchApplied& applied,
    std::vector<std::uint8_t>* encoded,
    std::string* error = nullptr);

bool DecodeSharedDisplaySwitchApplied(
    std::span<const std::uint8_t> encoded,
    SharedDisplaySwitchApplied* applied,
    std::string* error = nullptr);

bool EncodeScreenRefreshRequest(
    const ScreenRefreshRequest& request,
    std::vector<std::uint8_t>* encoded,
    std::string* error = nullptr);

bool DecodeScreenRefreshRequest(
    std::span<const std::uint8_t> encoded,
    ScreenRefreshRequest* request,
    std::string* error = nullptr);

bool EncodeScreenFirstFramePresented(
    const ScreenFirstFramePresented& presented,
    std::vector<std::uint8_t>* encoded,
    std::string* error = nullptr);

bool DecodeScreenFirstFramePresented(
    std::span<const std::uint8_t> encoded,
    ScreenFirstFramePresented* presented,
    std::string* error = nullptr);

}  // namespace remote
