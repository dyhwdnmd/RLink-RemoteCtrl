// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#include "LocalTcpSignaling.h"

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <winsock2.h>
#include <ws2tcpip.h>

#include <algorithm>
#include <array>
#include <condition_variable>
#include <cstring>
#include <limits>
#include <mutex>
#include <span>
#include <thread>
#include <utility>
#include <vector>

namespace remote {
namespace {

constexpr std::uint32_t kFrameMagic = 0x52544353;  // "RTCS"
constexpr std::uint16_t kProtocolVersion = 1;
constexpr std::uint32_t kMaxPayloadBytes = 1024 * 1024;

enum class MessageType : std::uint16_t {
    kDescription = 1,
    kIceCandidate = 2,
};

void AppendU32(std::vector<std::uint8_t>& output, std::uint32_t value)
{
    value = htonl(value);
    const auto* bytes = reinterpret_cast<const std::uint8_t*>(&value);
    output.insert(output.end(), bytes, bytes + sizeof(value));
}

bool AppendString(std::vector<std::uint8_t>& output,
                  const std::string& value)
{
    if (value.size() > std::numeric_limits<std::uint32_t>::max()) {
        return false;
    }
    AppendU32(output, static_cast<std::uint32_t>(value.size()));
    output.insert(output.end(), value.begin(), value.end());
    return true;
}

bool ReadU32(std::span<const std::uint8_t> input,
             std::size_t& offset,
             std::uint32_t& value)
{
    if (input.size() - std::min(input.size(), offset) < sizeof(value)) {
        return false;
    }
    std::memcpy(&value, input.data() + offset, sizeof(value));
    value = ntohl(value);
    offset += sizeof(value);
    return true;
}

bool ReadString(std::span<const std::uint8_t> input,
                std::size_t& offset,
                std::string& value)
{
    std::uint32_t length = 0;
    if (!ReadU32(input, offset, length) ||
        length > input.size() - std::min(input.size(), offset)) {
        return false;
    }
    value.assign(reinterpret_cast<const char*>(input.data() + offset),
                 length);
    offset += length;
    return true;
}

bool SendAll(SOCKET socket, std::span<const std::uint8_t> bytes)
{
    std::size_t sent = 0;
    while (sent < bytes.size()) {
        const auto remaining = bytes.size() - sent;
        const int chunk = static_cast<int>(std::min<std::size_t>(
            remaining, static_cast<std::size_t>(std::numeric_limits<int>::max())));
        const int result = send(
            socket,
            reinterpret_cast<const char*>(bytes.data() + sent),
            chunk,
            0);
        if (result == SOCKET_ERROR || result == 0) {
            return false;
        }
        sent += static_cast<std::size_t>(result);
    }
    return true;
}

bool ReceiveAll(SOCKET socket, std::span<std::uint8_t> bytes)
{
    std::size_t received = 0;
    while (received < bytes.size()) {
        const auto remaining = bytes.size() - received;
        const int chunk = static_cast<int>(std::min<std::size_t>(
            remaining, static_cast<std::size_t>(std::numeric_limits<int>::max())));
        const int result = recv(
            socket,
            reinterpret_cast<char*>(bytes.data() + received),
            chunk,
            0);
        if (result == SOCKET_ERROR || result == 0) {
            return false;
        }
        received += static_cast<std::size_t>(result);
    }
    return true;
}

std::string SocketErrorText(const char* operation)
{
    return std::string(operation) + " failed, WSA error " +
           std::to_string(WSAGetLastError());
}

}  // namespace

class LocalTcpSignaling::Impl final {
public:
    Impl()
    {
        WSADATA data{};
        if (WSAStartup(MAKEWORD(2, 2), &data) == 0) {
            winsockReady_ = true;
        } else {
            SetError("WSAStartup failed");
        }
    }

    ~Impl()
    {
        Stop();
        if (winsockReady_) {
            WSACleanup();
        }
    }

    void SetReceiver(SessionControllerBase* receiver)
    {
        std::lock_guard lock(stateMutex_);
        receiver_ = receiver;
    }

    bool StartListening(std::uint16_t port)
    {
        if (!winsockReady_) {
            return false;
        }

        SOCKET listener = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (listener == INVALID_SOCKET) {
            SetError(SocketErrorText("socket"));
            return false;
        }

        const BOOL reuse = TRUE;
        setsockopt(listener,
                   SOL_SOCKET,
                   SO_REUSEADDR,
                   reinterpret_cast<const char*>(&reuse),
                   sizeof(reuse));

        sockaddr_in address{};
        address.sin_family = AF_INET;
        address.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
        address.sin_port = htons(port);
        if (bind(listener,
                 reinterpret_cast<const sockaddr*>(&address),
                 sizeof(address)) == SOCKET_ERROR) {
            const auto error = SocketErrorText("bind");
            closesocket(listener);
            SetError(error);
            return false;
        }
        if (listen(listener, 1) == SOCKET_ERROR) {
            const auto error = SocketErrorText("listen");
            closesocket(listener);
            SetError(error);
            return false;
        }

        {
            std::lock_guard lock(stateMutex_);
            if (worker_.joinable() || stopping_) {
                closesocket(listener);
                SetErrorLocked("signaling transport already started");
                return false;
            }
            listenSocket_ = listener;
            worker_ = std::thread([this, listener] { AcceptAndRead(listener); });
        }
        return true;
    }

    bool Connect(const std::string& ipv4Address,
                 std::uint16_t port,
                 std::chrono::milliseconds timeout)
    {
        if (!winsockReady_) {
            return false;
        }

        sockaddr_in address{};
        address.sin_family = AF_INET;
        address.sin_port = htons(port);
        if (inet_pton(AF_INET, ipv4Address.c_str(), &address.sin_addr) != 1) {
            SetError("invalid IPv4 signaling address: " + ipv4Address);
            return false;
        }

        const auto deadline = std::chrono::steady_clock::now() + timeout;
        for (;;) {
            SOCKET peer = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
            if (peer == INVALID_SOCKET) {
                SetError(SocketErrorText("socket"));
                return false;
            }

            if (connect(peer,
                        reinterpret_cast<const sockaddr*>(&address),
                        sizeof(address)) == 0) {
                {
                    std::lock_guard lock(stateMutex_);
                    if (worker_.joinable() || stopping_) {
                        closesocket(peer);
                        SetErrorLocked("signaling transport already started");
                        return false;
                    }
                    peerSocket_ = peer;
                    connected_ = true;
                    worker_ = std::thread([this, peer] { ReadLoop(peer); });
                }
                stateCondition_.notify_all();
                return true;
            }

            const int error = WSAGetLastError();
            closesocket(peer);
            if (std::chrono::steady_clock::now() >= deadline) {
                SetError("connect failed, WSA error " +
                         std::to_string(error));
                return false;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
        }
    }

    bool WaitUntilConnected(std::chrono::milliseconds timeout)
    {
        std::unique_lock lock(stateMutex_);
        stateCondition_.wait_for(lock, timeout, [this] {
            return connected_ || stopping_ || !lastError_.empty();
        });
        return connected_;
    }

    bool SendDescription(const SessionDescription& description)
    {
        std::vector<std::uint8_t> payload;
        payload.reserve(1 + sizeof(std::uint32_t) + description.sdp.size());
        payload.push_back(description.type == SessionDescriptionType::kOffer
                              ? 0
                              : 1);
        if (!AppendString(payload, description.sdp)) {
            SetError("SDP is too large for local signaling frame");
            return false;
        }
        return SendFrame(MessageType::kDescription, payload);
    }

    bool SendIceCandidate(const IceCandidate& candidate)
    {
        std::vector<std::uint8_t> payload;
        payload.reserve(12 + candidate.sdpMid.size() +
                        candidate.candidate.size());
        AppendU32(payload,
                  static_cast<std::uint32_t>(candidate.sdpMLineIndex));
        if (!AppendString(payload, candidate.sdpMid) ||
            !AppendString(payload, candidate.candidate)) {
            SetError("ICE candidate is too large for local signaling frame");
            return false;
        }
        return SendFrame(MessageType::kIceCandidate, payload);
    }

    void Stop()
    {
        SOCKET peer = INVALID_SOCKET;
        SOCKET listener = INVALID_SOCKET;
        {
            std::lock_guard lock(stateMutex_);
            if (!stopping_) {
                stopping_ = true;
            }
            peer = std::exchange(peerSocket_, INVALID_SOCKET);
            listener = std::exchange(listenSocket_, INVALID_SOCKET);
            connected_ = false;
            receiver_ = nullptr;
        }
        stateCondition_.notify_all();

        if (peer != INVALID_SOCKET) {
            shutdown(peer, SD_BOTH);
            closesocket(peer);
        }
        if (listener != INVALID_SOCKET) {
            closesocket(listener);
        }
        if (worker_.joinable() &&
            worker_.get_id() != std::this_thread::get_id()) {
            worker_.join();
        }
    }

    std::string LastError() const
    {
        std::lock_guard lock(stateMutex_);
        return lastError_;
    }

private:
    bool SendFrame(MessageType type,
                   std::span<const std::uint8_t> payload)
    {
        if (payload.size() > kMaxPayloadBytes) {
            SetError("local signaling payload exceeds size limit");
            return false;
        }

        std::array<std::uint32_t, 3> header{};
        header[0] = htonl(kFrameMagic);
        const std::uint32_t versionAndType =
            (static_cast<std::uint32_t>(kProtocolVersion) << 16) |
            static_cast<std::uint16_t>(type);
        header[1] = htonl(versionAndType);
        header[2] = htonl(static_cast<std::uint32_t>(payload.size()));

        std::lock_guard sendLock(sendMutex_);
        SOCKET peer = INVALID_SOCKET;
        {
            std::lock_guard lock(stateMutex_);
            if (!connected_ || peerSocket_ == INVALID_SOCKET || stopping_) {
                SetErrorLocked("local signaling peer is not connected");
                return false;
            }
            peer = peerSocket_;
        }

        const auto headerBytes = std::span<const std::uint8_t>(
            reinterpret_cast<const std::uint8_t*>(header.data()),
            sizeof(header));
        if (!SendAll(peer, headerBytes) || !SendAll(peer, payload)) {
            SetError(SocketErrorText("send"));
            return false;
        }
        return true;
    }

    void AcceptAndRead(SOCKET listener)
    {
        SOCKET peer = accept(listener, nullptr, nullptr);

        bool closeListener = false;
        {
            std::lock_guard lock(stateMutex_);
            if (listenSocket_ == listener) {
                listenSocket_ = INVALID_SOCKET;
                closeListener = true;
            }
        }
        if (closeListener) {
            closesocket(listener);
        }

        if (peer == INVALID_SOCKET) {
            std::lock_guard lock(stateMutex_);
            if (!stopping_) {
                SetErrorLocked(SocketErrorText("accept"));
            }
            stateCondition_.notify_all();
            return;
        }

        {
            std::lock_guard lock(stateMutex_);
            if (stopping_) {
                closesocket(peer);
                return;
            }
            peerSocket_ = peer;
            connected_ = true;
        }
        stateCondition_.notify_all();
        ReadLoop(peer);
    }

    void ReadLoop(SOCKET peer)
    {
        for (;;) {
            std::array<std::uint32_t, 3> header{};
            auto headerBytes = std::span<std::uint8_t>(
                reinterpret_cast<std::uint8_t*>(header.data()),
                sizeof(header));
            if (!ReceiveAll(peer, headerBytes)) {
                break;
            }

            const std::uint32_t magic = ntohl(header[0]);
            const std::uint32_t versionAndType = ntohl(header[1]);
            const auto version = static_cast<std::uint16_t>(
                versionAndType >> 16);
            const auto type = static_cast<MessageType>(
                versionAndType & 0xffff);
            const std::uint32_t payloadSize = ntohl(header[2]);
            if (magic != kFrameMagic || version != kProtocolVersion ||
                payloadSize > kMaxPayloadBytes) {
                SetError("invalid local signaling frame header");
                break;
            }

            std::vector<std::uint8_t> payload(payloadSize);
            if (!ReceiveAll(peer, payload) ||
                !Dispatch(type, payload)) {
                break;
            }
        }

        bool closePeer = false;
        {
            std::lock_guard lock(stateMutex_);
            if (peerSocket_ == peer) {
                peerSocket_ = INVALID_SOCKET;
                connected_ = false;
                closePeer = true;
            }
        }
        if (closePeer) {
            closesocket(peer);
        }
        stateCondition_.notify_all();
    }

    bool Dispatch(MessageType type,
                  std::span<const std::uint8_t> payload)
    {
        SessionControllerBase* receiver = nullptr;
        {
            std::lock_guard lock(stateMutex_);
            receiver = receiver_;
        }
        if (receiver == nullptr) {
            SetError("local signaling receiver is not configured");
            return false;
        }

        std::size_t offset = 0;
        if (type == MessageType::kDescription) {
            if (payload.empty()) {
                SetError("invalid description signaling payload");
                return false;
            }
            SessionDescription description;
            description.type = payload[0] == 0
                                   ? SessionDescriptionType::kOffer
                                   : SessionDescriptionType::kAnswer;
            offset = 1;
            if (!ReadString(payload, offset, description.sdp) ||
                offset != payload.size()) {
                SetError("invalid description signaling payload");
                return false;
            }
            receiver->HandleRemoteDescription(description);
            return true;
        }

        if (type == MessageType::kIceCandidate) {
            IceCandidate candidate;
            std::uint32_t mLineIndex = 0;
            if (!ReadU32(payload, offset, mLineIndex) ||
                !ReadString(payload, offset, candidate.sdpMid) ||
                !ReadString(payload, offset, candidate.candidate) ||
                offset != payload.size()) {
                SetError("invalid ICE candidate signaling payload");
                return false;
            }
            candidate.sdpMLineIndex = static_cast<int>(mLineIndex);
            receiver->HandleRemoteIceCandidate(candidate);
            return true;
        }

        SetError("unknown local signaling message type");
        return false;
    }

    void SetError(std::string error)
    {
        {
            std::lock_guard lock(stateMutex_);
            SetErrorLocked(std::move(error));
        }
        stateCondition_.notify_all();
    }

    void SetErrorLocked(std::string error)
    {
        if (lastError_.empty()) {
            lastError_ = std::move(error);
        }
    }

    mutable std::mutex stateMutex_;
    std::condition_variable stateCondition_;
    std::mutex sendMutex_;
    std::thread worker_;
    SOCKET listenSocket_ = INVALID_SOCKET;
    SOCKET peerSocket_ = INVALID_SOCKET;
    SessionControllerBase* receiver_ = nullptr;
    std::string lastError_;
    bool winsockReady_ = false;
    bool connected_ = false;
    bool stopping_ = false;
};

LocalTcpSignaling::LocalTcpSignaling() : impl_(std::make_unique<Impl>()) {}

LocalTcpSignaling::~LocalTcpSignaling() = default;

void LocalTcpSignaling::SetReceiver(SessionControllerBase* receiver)
{
    impl_->SetReceiver(receiver);
}

bool LocalTcpSignaling::StartListening(std::uint16_t port)
{
    return impl_->StartListening(port);
}

bool LocalTcpSignaling::Connect(const std::string& ipv4Address,
                                std::uint16_t port,
                                std::chrono::milliseconds timeout)
{
    return impl_->Connect(ipv4Address, port, timeout);
}

bool LocalTcpSignaling::WaitUntilConnected(std::chrono::milliseconds timeout)
{
    return impl_->WaitUntilConnected(timeout);
}

bool LocalTcpSignaling::SendDescription(
    const SessionDescription& description)
{
    return impl_->SendDescription(description);
}

bool LocalTcpSignaling::SendIceCandidate(const IceCandidate& candidate)
{
    return impl_->SendIceCandidate(candidate);
}

void LocalTcpSignaling::Stop()
{
    impl_->Stop();
}

std::string LocalTcpSignaling::LastError() const
{
    return impl_->LastError();
}

}  // namespace remote
