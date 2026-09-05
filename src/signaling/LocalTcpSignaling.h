// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#pragma once

#include <chrono>
#include <cstdint>
#include <memory>
#include <string>

#include "src/core/SessionController.h"

namespace remote {

// Local, process-to-process signaling transport used by the first vertical
// slice. Only SDP and trickle ICE travel over this connection; WebRTC media
// and DataChannels continue to use the PeerConnection transport directly.
class LocalTcpSignaling final : public ISessionSignalingSender {
public:
    LocalTcpSignaling();
    ~LocalTcpSignaling() override;

    LocalTcpSignaling(const LocalTcpSignaling&) = delete;
    LocalTcpSignaling& operator=(const LocalTcpSignaling&) = delete;

    void SetReceiver(SessionControllerBase* receiver);

    bool StartListening(std::uint16_t port);
    bool Connect(const std::string& ipv4Address,
                 std::uint16_t port,
                 std::chrono::milliseconds timeout);
    bool WaitUntilConnected(std::chrono::milliseconds timeout);

    bool SendDescription(
        const SessionDescription& description) override;
    bool SendIceCandidate(const IceCandidate& candidate) override;

    void Stop();
    std::string LastError() const;

private:
    class Impl;
    std::unique_ptr<Impl> impl_;
};

}  // namespace remote
