// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#pragma once

#include <string>

namespace remote {

enum class SessionPurpose {
    kNone,
    kRemoteControl,
    kCameraOnly,
};

// Describes only the authorization gate used before a direct P2P session is
// established. Once signaling returns SessionReady, media and input behavior
// is intentionally independent from this value.
enum class DirectAuthorizationMethod {
    kManualApproval,
    kOwnedAccount,
    kVerificationCode,
};

struct DirectSessionConnectRequest {
    std::string targetDeviceId;
    SessionPurpose purpose = SessionPurpose::kNone;
    DirectAuthorizationMethod authorization =
        DirectAuthorizationMethod::kManualApproval;
    std::string verificationCode;
};

}  // namespace remote
