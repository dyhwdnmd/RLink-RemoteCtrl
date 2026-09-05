// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

    if (!binary) {
        return;
    }

    if (DispatchRemoteCursorData(pairId, label, payload)) {
        return;
    }

    if (label == kFileTransferChannel) {
        FileTransferEnvelope envelope;
        if (!DecodeFileTransferMessage(payload, &envelope)) {
            return;
        }

        IFileTransferSink* sink = nullptr;
        {
            std::lock_guard lock(mutex_);
            const auto pairIt = roomPairs_.find(pairId);
            if (pairIt == roomPairs_.end() ||
                snapshot_.room.membership !=
                    RoomMembershipState::kActive ||
                envelope.roomId != snapshot_.room.roomId ||
                envelope.roomId != pairIt->second->roomId ||
                envelope.senderDeviceId !=
                    pairIt->second->peerDeviceId ||
                envelope.receiverDeviceId != snapshot_.localDeviceId ||
                envelope.sequence <=
                    pairIt->second->lastFileTransferSequence) {
                return;
            }
            pairIt->second->lastFileTransferSequence = envelope.sequence;
            sink = remoteFileTransferSink_;
        }
        if (sink) {
            sink->OnFileTransferMessage(envelope);
        }
        return;
    }

    if (label == kClipboardReliableChannel ||
        label == kClipboardTransferChannel) {
        if (label == kClipboardTransferChannel &&
            IsClipboardWarmupPayload(payload)) {
            return;
        }
        ClipboardEnvelope envelope;
        if (!DecodeClipboardMessage(payload, &envelope)) {
            return;
        }
        IClipboardSink* sink = nullptr;
        {
            std::lock_guard lock(mutex_);
            const auto pairIt = roomPairs_.find(pairId);
            if (pairIt == roomPairs_.end() || !pairIt->second ||
                snapshot_.room.membership !=
                    RoomMembershipState::kActive ||
                envelope.roomId != snapshot_.room.roomId ||
                envelope.roomId != pairIt->second->roomId ||
                envelope.senderDeviceId != pairIt->second->peerDeviceId ||
                envelope.receiverDeviceId != snapshot_.localDeviceId ||
                roomControlGrantId_.empty() ||
                envelope.controlGrantId != roomControlGrantId_) {
                return;
            }
            const bool localIsSharer =
                snapshot_.room.screenSharerDeviceId ==
                    snapshot_.localDeviceId &&
                snapshot_.room.activeControllerDeviceId ==
                    envelope.senderDeviceId;
            const bool localIsController =
                snapshot_.room.activeControllerDeviceId ==
                    snapshot_.localDeviceId &&
                snapshot_.room.screenSharerDeviceId ==
                    envelope.senderDeviceId;
            if (!localIsSharer && !localIsController) {
                return;
            }
            auto& lastSequence = label == kClipboardTransferChannel
                ? pairIt->second->lastClipboardTransferSequence
                : pairIt->second->lastClipboardReliableSequence;
            if (envelope.sequence <= lastSequence ||
                IsClipboardTransferMessage(envelope.message.type) !=
                    (label == kClipboardTransferChannel)) {
                return;
            }
            lastSequence = envelope.sequence;
            sink = remoteClipboardSink_;
        }
        if (sink) {
            sink->OnClipboardMessage(envelope);
        }
        return;
    }

    const bool fastChannel = label == kInputFastChannel;
    const bool reliableChannel = label == kControlReliableChannel;
    if (!fastChannel && !reliableChannel) {
        return;
    }
