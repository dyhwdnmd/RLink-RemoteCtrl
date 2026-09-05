// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

    RemoteInputEnvelope input;
    std::string inputDecodeError;
    if (!DecodeRemoteInput(payload, &input, &inputDecodeError)) {
        return;
    }
    auto& inputTelemetry = RemoteInputTelemetry::Instance();
    inputTelemetry.RecordPacketReceived();
    const auto recordInputDrop = [&inputTelemetry] {
        inputTelemetry.RecordPacketDropped();
    };
    const bool buttonTransition =
        input.event.type == RemoteInputMessageType::kMouseButton;
    if (!buttonTransition &&
        UsesFastInputChannel(input.event.type) != fastChannel) {
        recordInputDrop();
        return;
    }

    IRemoteInputSink* sink = nullptr;
    webrtc::scoped_refptr<WindowsDesktopCaptureSource>
        sourceToBoost;
    {
        std::lock_guard lock(mutex_);
        const auto pairIt = roomPairs_.find(pairId);
        if (pairIt == roomPairs_.end() ||
            snapshot_.room.membership != RoomMembershipState::kActive ||
            snapshot_.room.screenShareState !=
                RoomScreenShareState::kActive ||
            snapshot_.room.screenSharerDeviceId != snapshot_.localDeviceId ||
            snapshot_.room.activeControllerDeviceId !=
                pairIt->second->peerDeviceId ||
            input.roomId != snapshot_.room.roomId ||
            input.roomId != pairIt->second->roomId ||
            input.senderDeviceId != pairIt->second->peerDeviceId ||
            roomControlGrantId_.empty() ||
            input.controlGrantId != roomControlGrantId_) {
            recordInputDrop();
            return;
        }
        if (IsPointerInput(input.event.type) &&
            (snapshot_.activeSharedDisplay.sessionDisplayId == 0 ||
             snapshot_.activeSharedDisplayLayoutVersion == 0 ||
             input.event.displayId !=
                 snapshot_.activeSharedDisplay.sessionDisplayId ||
             input.event.displayLayoutVersion !=
                 snapshot_.activeSharedDisplayLayoutVersion)) {
            recordInputDrop();
            return;
        }

        std::uint64_t& lastSequence = fastChannel
            ? pairIt->second->lastFastInputSequence
            : pairIt->second->lastReliableInputSequence;
        if (input.sequence <= lastSequence) {
            recordInputDrop();
            return;
        }
        lastSequence = input.sequence;
        input.event.deliverySequence = input.sequence;
        if (buttonTransition ||
            input.event.type == RemoteInputMessageType::kReleaseAll) {
            if (input.sequence <=
                pairIt->second->lastPointerStateSequence) {
                recordInputDrop();
                return;
            }
            pairIt->second->lastPointerStateSequence = input.sequence;
        }
        sink = remoteInputSink_;
        if (sink && ShouldBoostDesktopCaptureForInput(input.event)) {
            sourceToBoost = localDesktopCaptureSource_;
        }
    }
    if (sink) {
        sink->OnRemoteInput(input.event);
        cursorMonitor_.SetLastAppliedInputSequence(input.sequence);
        // Wake capture only after SendInput has run, matching Chrome Remote
        // Desktop's input-to-frame ordering instead of racing the injection.
        if (sourceToBoost) {
            sourceToBoost->NotifyRemoteInputActivity();
        }
    } else {
        recordInputDrop();
    }
}
