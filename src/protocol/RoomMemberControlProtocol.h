// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#pragma once

#include <cstdint>
#include <span>
#include <string>
#include <vector>

#include "src/core/RoomState.h"

namespace remote {

inline constexpr std::uint8_t kRoomMemberControlProtocolVersion = 1;

struct RoomMemberActionEnvelope {
    std::string roomId;
    std::string senderDeviceId;
    std::string receiverDeviceId;
    std::uint64_t sequence = 0;
    std::uint64_t screenShareEpoch = 0;
    RoomMemberAction action = RoomMemberAction::kRequestScreenShare;
    bool response = false;
    bool accepted = false;
    std::string error;
};

bool EncodeRoomMemberAction(
    const RoomMemberActionEnvelope& message,
    std::vector<std::uint8_t>* encoded,
    std::string* error = nullptr);

bool DecodeRoomMemberAction(
    std::span<const std::uint8_t> encoded,
    RoomMemberActionEnvelope* message,
    std::string* error = nullptr);

}  // namespace remote
