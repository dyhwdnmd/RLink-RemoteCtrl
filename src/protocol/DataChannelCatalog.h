// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#pragma once

#include <string_view>
#include <vector>

#include "src/webrtc/IWebRtcSession.h"

namespace remote {

inline constexpr std::string_view kInputFastChannel = "input-fast";
inline constexpr std::string_view kControlReliableChannel =
    "control-reliable";
inline constexpr std::string_view kFileTransferChannel = "file-transfer";
inline constexpr std::string_view kClipboardReliableChannel =
    "clipboard-reliable";
inline constexpr std::string_view kClipboardTransferChannel =
    "clipboard-transfer";
inline constexpr std::string_view kTelemetryChannel = "telemetry";

std::vector<DataChannelSpec> DefaultRemoteControlDataChannels();
std::vector<DataChannelSpec> DefaultCameraSessionDataChannels();

}  // namespace remote
