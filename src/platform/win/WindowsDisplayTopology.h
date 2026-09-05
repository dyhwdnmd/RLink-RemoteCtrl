// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#pragma once

#include "src/core/DisplayTopology.h"

namespace remote {

// Enumerates the physical-pixel Windows desktop. The returned layoutVersion
// is a deterministic fingerprint of display identity, position, mode, DPI
// and rotation, and therefore changes whenever old pointer coordinates become
// unsafe.
DisplayTopologySnapshot EnumerateWindowsDisplayTopology();

}  // namespace remote
