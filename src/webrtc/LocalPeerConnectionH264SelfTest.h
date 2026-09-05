// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#pragma once

#include <functional>
#include <string>

#include "api/peer_connection_interface.h"
#include "api/scoped_refptr.h"

namespace remote {

struct LocalPeerConnectionH264SelfTestResult {
    bool passed = false;
    std::string report;
};

// Builds two PeerConnections in one process and sends synthetic I420 frames
// through the complete H264/RTP/ICE/decode pipeline. The receiver must observe
// D3D11NativeFrameBuffer output for the hardware path to pass.
LocalPeerConnectionH264SelfTestResult RunLocalPeerConnectionH264SelfTest(
    webrtc::scoped_refptr<webrtc::PeerConnectionFactoryInterface> factory,
    std::function<bool()> reconfigureEncoder = {});

}  // namespace remote
