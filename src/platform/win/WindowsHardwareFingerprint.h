// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#pragma once

#include <string>
#include <vector>

namespace remote {

struct WindowsCompatibilityProfile {
    std::string hardwareFingerprint;
    std::string operatingSystem;
    std::string nativeArchitecture;
    bool remoteSession = false;
    std::vector<std::string> graphicsAdapters;
    std::string graphicsEnumerationError;
};

// Describes the current Windows/GPU environment using stable, copyable text.
// This is diagnostic metadata only; it never owns D3D/MFT/COM objects.
WindowsCompatibilityProfile QueryWindowsCompatibilityProfile();

// Returns a stable, inexpensive identity for the current Windows media
// environment. It intentionally contains descriptors only; D3D/MFT/COM
// objects are never cached across application runs.
std::string BuildWindowsHardwareFingerprint();

}  // namespace remote
