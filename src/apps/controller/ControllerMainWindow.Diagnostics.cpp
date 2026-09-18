// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#include "ControllerMainWindow.Internal.h"

namespace remote::controller {
using namespace detail;

#include "ControllerMainWindow.DiagnosticsInput.inc"
#include "ControllerMainWindow.DiagnosticsPeers.inc"
#include "ControllerMainWindow.DiagnosticsOutbound.inc"
#include "ControllerMainWindow.DiagnosticsInbound.inc"
#include "ControllerMainWindow.DiagnosticsPublish.inc"
#include "ControllerMainWindow.SnapshotPreflight.inc"
#include "ControllerMainWindow.SnapshotConnectivity.inc"
#include "ControllerMainWindow.SnapshotLocal.inc"
#include "ControllerMainWindow.SnapshotDebug.inc"
#include "ControllerMainWindow.SnapshotCopy.inc"
#include "ControllerMainWindow.SnapshotWindows.inc"
}  // namespace remote::controller
