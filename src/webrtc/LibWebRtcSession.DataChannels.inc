// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

OperationId LibWebRtcSession::CreateDataChannels(
    const std::vector<DataChannelSpec>& channels)
{
    const OperationId operationId = NextOperationId();
    auto peer = PeerConnection();
    if (!peer) {
        FailOperation(operationId, "session_not_started",
                      "Cannot create DataChannels before Start.");
        return operationId;
    }

    for (const auto& spec : channels) {
        if (spec.label.empty()) {
            FailOperation(operationId, "invalid_channel_label",
                          "A DataChannel label cannot be empty.");
            return operationId;
        }
        if (spec.maxRetransmits && spec.maxPacketLifeTimeMs) {
            FailOperation(operationId, "invalid_channel_reliability",
                          "maxRetransmits and maxPacketLifeTimeMs are mutually exclusive.");
            return operationId;
        }
        bool duplicateLabel = false;
        {
            std::lock_guard lock(mutex_);
            duplicateLabel = dataChannels_.contains(spec.label);
        }
        if (duplicateLabel) {
            FailOperation(operationId, "duplicate_channel_label",
                          "A DataChannel with this label already exists.");
            return operationId;
        }

        webrtc::DataChannelInit nativeConfig;
        nativeConfig.ordered = spec.ordered;
        nativeConfig.maxRetransmits = spec.maxRetransmits;
        nativeConfig.maxRetransmitTime = spec.maxPacketLifeTimeMs;
        nativeConfig.protocol = spec.protocol;
        if (spec.priority) {
            switch (*spec.priority) {
            case DataChannelSpec::Priority::kLow:
                nativeConfig.priority = webrtc::PriorityValue(
                    webrtc::Priority::kLow);
                break;
            case DataChannelSpec::Priority::kMedium:
                nativeConfig.priority = webrtc::PriorityValue(
                    webrtc::Priority::kMedium);
                break;
            case DataChannelSpec::Priority::kHigh:
                nativeConfig.priority = webrtc::PriorityValue(
                    webrtc::Priority::kHigh);
                break;
            }
        }
        auto channelOrError =
            peer->CreateDataChannelOrError(spec.label, &nativeConfig);
        if (!channelOrError.ok()) {
            FailOperation(operationId, "data_channel_create_failed",
                          channelOrError.error().message());
            return operationId;
        }
        AttachDataChannel(channelOrError.MoveValue());
    }

    CompleteOperation(operationId);
    return operationId;
}

SendResult LibWebRtcSession::SendData(const std::string& channelName,
                                      std::span<const std::uint8_t> data,
                                      bool binary)
{
    if (!PeerConnection()) {
        return SendResult::kSessionNotStarted;
    }

    webrtc::scoped_refptr<webrtc::DataChannelInterface> channel;
    {
        std::lock_guard lock(mutex_);
        const auto found = dataChannels_.find(channelName);
        if (found == dataChannels_.end()) {
            return SendResult::kChannelNotFound;
        }
        channel = found->second->Channel();
    }
    if (channel->state() != webrtc::DataChannelInterface::kOpen) {
        return SendResult::kChannelNotOpen;
    }

    const webrtc::CopyOnWriteBuffer payload(data.data(), data.size());
    return channel->Send(webrtc::DataBuffer(payload, binary))
               ? SendResult::kSent
               : SendResult::kSendFailed;
}

std::optional<std::uint64_t>
LibWebRtcSession::DataChannelBufferedAmount(
    const std::string& channelName) const
{
    if (!PeerConnection()) {
        return std::nullopt;
    }

    webrtc::scoped_refptr<webrtc::DataChannelInterface> channel;
    {
        std::lock_guard lock(mutex_);
        const auto found = dataChannels_.find(channelName);
        if (found == dataChannels_.end()) {
            return std::nullopt;
        }
        channel = found->second->Channel();
    }
    if (!channel ||
        channel->state() != webrtc::DataChannelInterface::kOpen) {
        return std::nullopt;
    }
    return channel->buffered_amount();
}
