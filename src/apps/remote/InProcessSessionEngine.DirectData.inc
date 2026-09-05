// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

SessionCommandResult InProcessSessionEngine::SendRemoteInput(
    const RemoteInputEvent& event)
{
    const auto snapshot = Snapshot();
    if (snapshot.state == SessionEngineState::kActive &&
        snapshot.purpose == SessionPurpose::kRemoteControl &&
        snapshot.remoteControlRole == RemoteControlRole::kController) {
        return SendDirectInput(event);
    }
    return SendRoomInput(event);
}

SessionCommandResult InProcessSessionEngine::SendRemoteFileMessage(
    const std::string& peerDeviceId,
    const FileTransferMessage& message)
{
    const auto snapshot = Snapshot();
    if (snapshot.state == SessionEngineState::kActive &&
        snapshot.purpose == SessionPurpose::kRemoteControl &&
        snapshot.peerDeviceId == peerDeviceId) {
        return SendDirectFileMessage(peerDeviceId, message);
    }
    return SendRoomFileMessage(peerDeviceId, message);
}

SessionCommandResult InProcessSessionEngine::SendRemoteClipboardMessage(
    const std::string& peerDeviceId,
    const std::string& clipboardSessionId,
    const ClipboardMessage& message)
{
    const auto snapshot = Snapshot();
    if (snapshot.state == SessionEngineState::kActive &&
        snapshot.purpose == SessionPurpose::kRemoteControl &&
        snapshot.peerDeviceId == peerDeviceId) {
        return SendDirectClipboardMessage(
            peerDeviceId, clipboardSessionId, message);
    }
    return SendRoomClipboardMessage(
        peerDeviceId, clipboardSessionId, message);
}

SessionCommandResult InProcessSessionEngine::SendDirectFileMessage(
    const std::string& peerDeviceId,
    const FileTransferMessage& message)
{
    const std::string channel(kFileTransferChannel);
    SessionControllerBase* controller = nullptr;
    LibWebRtcSession* session = nullptr;
    FileTransferEnvelope envelope;
    {
        std::lock_guard lock(mutex_);
        if (!sessionController_ || !webRtcSession_ ||
            snapshot_.state != SessionEngineState::kActive ||
            snapshot_.purpose != SessionPurpose::kRemoteControl ||
            snapshot_.peerDeviceId != peerDeviceId ||
            peerDeviceId.empty()) {
            return Failure("direct_file_transfer_not_active",
                           "The direct peer session is not active.");
        }
        if (!directSession_.IsChannelOpen(channel)) {
            return Failure("direct_file_transfer_channel_not_open",
                           "The file-transfer DataChannel is not open.");
        }
        if (directSession_.nextFileTransferSequence ==
            std::numeric_limits<std::uint64_t>::max()) {
            return Failure("direct_file_transfer_sequence_exhausted",
                           "The file-transfer sequence is exhausted.");
        }
        controller = sessionController_.get();
        session = webRtcSession_.get();
        envelope.roomId = snapshot_.sessionId;
        envelope.senderDeviceId = snapshot_.localDeviceId;
        envelope.receiverDeviceId = peerDeviceId;
        envelope.sequence = ++directSession_.nextFileTransferSequence;
        envelope.timestampUs = static_cast<std::uint64_t>(
            std::chrono::duration_cast<std::chrono::microseconds>(
                std::chrono::steady_clock::now().time_since_epoch()).count());
        envelope.message = message;
    }
    const auto buffered = session->DataChannelBufferedAmount(channel);
    if (!buffered) {
        return Failure("direct_file_transfer_channel_not_open",
                       "The file-transfer DataChannel is unavailable.");
    }
    if (*buffered > kMaximumFileTransferBufferedBytes) {
        return Failure("direct_file_transfer_backpressure",
                       "The file-transfer send buffer is full; retry later.");
    }
    std::vector<std::uint8_t> encoded;
    std::string error;
    if (!EncodeFileTransferMessage(envelope, &encoded, &error)) {
        return Failure("direct_file_transfer_encode_failed", error);
    }
    switch (controller->SendData(channel, encoded, true)) {
    case SendResult::kSent:
        return Success();
    case SendResult::kChannelNotFound:
        return Failure("direct_file_transfer_channel_not_found",
                       "The file-transfer DataChannel was not negotiated.");
    case SendResult::kChannelNotOpen:
        return Failure("direct_file_transfer_channel_not_open",
                       "The file-transfer DataChannel is not open.");
    case SendResult::kSessionNotStarted:
        return Failure("direct_file_transfer_session_not_started",
                       "The direct P2P session is not active.");
    case SendResult::kSendFailed:
        return Failure("direct_file_transfer_send_failed",
                       "WebRTC rejected the file-transfer message.");
    }
    return Failure("direct_file_transfer_send_failed",
                   "The file-transfer send result is unknown.");
}

SessionCommandResult InProcessSessionEngine::SendDirectClipboardMessage(
    const std::string& peerDeviceId,
    const std::string& clipboardSessionId,
    const ClipboardMessage& message)
{
    const std::string channel = IsClipboardTransferMessage(message.type)
        ? std::string(kClipboardTransferChannel)
        : std::string(kClipboardReliableChannel);
    SessionControllerBase* controller = nullptr;
    LibWebRtcSession* session = nullptr;
    ClipboardEnvelope envelope;
    {
        std::lock_guard lock(mutex_);
        if (!sessionController_ || !webRtcSession_ ||
            snapshot_.state != SessionEngineState::kActive ||
            snapshot_.purpose != SessionPurpose::kRemoteControl ||
            snapshot_.peerDeviceId != peerDeviceId ||
            peerDeviceId.empty()) {
            return Failure("direct_clipboard_not_active",
                           "The direct peer session is not active.");
        }
        if (!directSession_.IsChannelOpen(channel)) {
            return Failure("direct_clipboard_channel_not_open",
                           "The clipboard DataChannel is not open.");
        }
        auto& nextSequence = channel == kClipboardTransferChannel
            ? directSession_.nextClipboardTransferSequence
            : directSession_.nextClipboardReliableSequence;
        if (nextSequence == std::numeric_limits<std::uint64_t>::max()) {
            return Failure("direct_clipboard_sequence_exhausted",
                           "The clipboard sequence is exhausted.");
        }
        controller = sessionController_.get();
        session = webRtcSession_.get();
        envelope.roomId = snapshot_.sessionId;
        envelope.senderDeviceId = snapshot_.localDeviceId;
        envelope.receiverDeviceId = peerDeviceId;
        // Direct authorization itself is the control grant. It is bound to
        // the authenticated session ID and cannot be supplied by the caller.
        envelope.controlGrantId = snapshot_.sessionId;
        envelope.clipboardSessionId = clipboardSessionId;
        envelope.sequence = ++nextSequence;
        envelope.timestampUs = static_cast<std::uint64_t>(
            std::chrono::duration_cast<std::chrono::microseconds>(
                std::chrono::steady_clock::now().time_since_epoch()).count());
        envelope.message = message;
    }
    const auto buffered = session->DataChannelBufferedAmount(channel);
    if (!buffered) {
        return Failure("direct_clipboard_channel_not_open",
                       "The clipboard DataChannel is unavailable.");
    }
    if (*buffered > kMaximumClipboardBufferedBytes) {
        return Failure("direct_clipboard_backpressure",
                       "The clipboard send buffer is full; retry later.");
    }
    std::vector<std::uint8_t> encoded;
    std::string error;
    if (!EncodeClipboardMessage(envelope, &encoded, &error)) {
        return Failure("direct_clipboard_encode_failed", error);
    }
    switch (controller->SendData(channel, encoded, true)) {
    case SendResult::kSent:
        return Success();
    case SendResult::kChannelNotFound:
        return Failure("direct_clipboard_channel_not_found",
                       "The clipboard DataChannel was not negotiated.");
    case SendResult::kChannelNotOpen:
        return Failure("direct_clipboard_channel_not_open",
                       "The clipboard DataChannel is not open.");
    case SendResult::kSessionNotStarted:
        return Failure("direct_clipboard_session_not_started",
                       "The direct P2P session is not active.");
    case SendResult::kSendFailed:
        return Failure("direct_clipboard_send_failed",
                       "WebRTC rejected the clipboard message.");
    }
    return Failure("direct_clipboard_send_failed",
                   "The clipboard send result is unknown.");
}

bool InProcessSessionEngine::DispatchDirectAuxiliaryData(
    const std::string& label,
    std::span<const std::uint8_t> payload)
{
    if (label == kFileTransferChannel) {
        FileTransferEnvelope envelope;
        if (!DecodeFileTransferMessage(payload, &envelope)) {
            return true;
        }
        IFileTransferSink* sink = nullptr;
        {
            std::lock_guard lock(mutex_);
            if (snapshot_.state != SessionEngineState::kActive ||
                snapshot_.purpose != SessionPurpose::kRemoteControl ||
                envelope.roomId != snapshot_.sessionId ||
                envelope.senderDeviceId != snapshot_.peerDeviceId ||
                envelope.receiverDeviceId != snapshot_.localDeviceId ||
                envelope.sequence <=
                    directSession_.lastFileTransferSequence) {
                return true;
            }
            directSession_.lastFileTransferSequence = envelope.sequence;
            sink = remoteFileTransferSink_;
        }
        if (sink) {
            sink->OnFileTransferMessage(envelope);
        }
        return true;
    }

    if (label == kClipboardReliableChannel ||
        label == kClipboardTransferChannel) {
        if (label == kClipboardTransferChannel &&
            IsClipboardWarmupPayload(payload)) {
            return true;
        }
        ClipboardEnvelope envelope;
        if (!DecodeClipboardMessage(payload, &envelope)) {
            return true;
        }
        IClipboardSink* sink = nullptr;
        {
            std::lock_guard lock(mutex_);
            if (snapshot_.state != SessionEngineState::kActive ||
                snapshot_.purpose != SessionPurpose::kRemoteControl ||
                envelope.roomId != snapshot_.sessionId ||
                envelope.senderDeviceId != snapshot_.peerDeviceId ||
                envelope.receiverDeviceId != snapshot_.localDeviceId ||
                envelope.controlGrantId != snapshot_.sessionId ||
                IsClipboardTransferMessage(envelope.message.type) !=
                    (label == kClipboardTransferChannel)) {
                return true;
            }
            auto& lastSequence = label == kClipboardTransferChannel
                ? directSession_.lastClipboardTransferSequence
                : directSession_.lastClipboardReliableSequence;
            if (envelope.sequence <= lastSequence) {
                return true;
            }
            lastSequence = envelope.sequence;
            sink = remoteClipboardSink_;
        }
        if (sink) {
            sink->OnClipboardMessage(envelope);
        }
        return true;
    }
    return false;
}
