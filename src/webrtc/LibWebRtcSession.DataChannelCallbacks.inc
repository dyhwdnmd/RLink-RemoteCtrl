// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

void LibWebRtcSession::AttachDataChannel(
    webrtc::scoped_refptr<webrtc::DataChannelInterface> channel)
{
    if (!channel) {
        return;
    }
    const std::string label = channel->label();
    auto binding = std::make_unique<DataChannelBinding>(this, channel);
    bool inserted = false;
    {
        std::lock_guard lock(mutex_);
        if (!dataChannels_.contains(label)) {
            dataChannels_.emplace(label, std::move(binding));
            inserted = true;
        }
    }
    if (!inserted) {
        channel->Close();
        return;
    }
    HandleDataChannelState(channel);
}

void LibWebRtcSession::HandleDataChannelState(
    webrtc::scoped_refptr<webrtc::DataChannelInterface> channel)
{
    if (!channel) {
        return;
    }
    DataChannelInfo info;
    info.label = channel->label();
    info.state = ToPublicDataChannelState(channel->state());
    info.ordered = channel->ordered();
    info.maxRetransmits = channel->maxRetransmitsOpt();
    info.maxPacketLifeTimeMs = channel->maxPacketLifeTime();
    info.protocol = channel->protocol();
    if (auto* observer = Observer()) {
        observer->OnDataChannelStateChanged(info);
    }
}

void LibWebRtcSession::HandleDataMessage(
    const std::string& label,
    const webrtc::DataBuffer& buffer)
{
    std::vector<std::uint8_t> payload(buffer.data.size());
    if (!payload.empty()) {
        std::copy_n(buffer.data.cdata(), payload.size(), payload.data());
    }
    if (auto* observer = Observer()) {
        observer->OnDataMessage(label, payload, buffer.binary);
    }
}
