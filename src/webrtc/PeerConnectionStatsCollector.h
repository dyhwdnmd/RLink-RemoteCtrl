// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#pragma once

#include <functional>
#include <memory>

#include "api/peer_connection_interface.h"
#include "api/scoped_refptr.h"
#include "src/webrtc/IWebRtcSession.h"

namespace webrtc {
class RTCStatsReport;
}

namespace remote {

// Converts libwebrtc's asynchronous RTCStatsReport into a transport-neutral
// immutable snapshot. It owns all counter history needed to calculate rates;
// callers never need to retain or interpret WebRTC stats objects.
class PeerConnectionStatsCollector final {
public:
    using CompletionCallback = std::function<void()>;

    PeerConnectionStatsCollector();
    ~PeerConnectionStatsCollector();

    PeerConnectionStatsCollector(
        const PeerConnectionStatsCollector&) = delete;
    PeerConnectionStatsCollector& operator=(
        const PeerConnectionStatsCollector&) = delete;

    void Request(
        webrtc::scoped_refptr<webrtc::PeerConnectionInterface> peer,
        CompletionCallback completion = {});
    WebRtcSessionStatsSnapshot Snapshot() const;

private:
    struct State;
    class Callback;

    static void ProcessReport(
        const std::shared_ptr<State>& state,
        const webrtc::scoped_refptr<const webrtc::RTCStatsReport>& report);

    std::shared_ptr<State> state_;
};

}  // namespace remote
