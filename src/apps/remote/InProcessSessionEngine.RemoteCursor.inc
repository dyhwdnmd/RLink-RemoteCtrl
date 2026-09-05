// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

void InProcessSessionEngine::SetRemoteCursorCallback(
    RemoteCursorCallback callback)
{
    std::lock_guard lock(mutex_);
    remoteCursorCallback_ = std::move(callback);
}

void InProcessSessionEngine::StartRemoteCursorPublishing(
    const DisplayDescriptor& display,
    std::uint64_t layoutVersion)
{
    StopRemoteCursorPublishing();
    {
        std::lock_guard lock(mutex_);
        latestLocalCursorPosition_.reset();
        latestLocalCursorShape_.reset();
    }
    (void)cursorMonitor_.Start(
        display, layoutVersion,
        [this](WindowsCursorObservation observation) {
            OnLocalCursorObservation(std::move(observation));
        });
}

void InProcessSessionEngine::StopRemoteCursorPublishing()
{
    cursorMonitor_.Stop();
    RemoteCursorEnvelope reset;
    SessionControllerBase* directController = nullptr;
    std::vector<std::shared_ptr<RoomPairRuntime>> roomRecipients;
    {
        std::lock_guard lock(mutex_);
        if (latestLocalCursorPosition_) {
            reset.type = RemoteCursorMessageType::kReset;
            reset.senderDeviceId = snapshot_.localDeviceId;
            reset.sequence = ++nextCursorSequence_;
            reset.timestampUs = static_cast<std::uint64_t>(
                std::chrono::duration_cast<std::chrono::microseconds>(
                    std::chrono::steady_clock::now().time_since_epoch())
                    .count());
            if (snapshot_.remoteControlRole ==
                    RemoteControlRole::kControlled &&
                !snapshot_.sessionId.empty() && sessionController_ &&
                directSession_.IsChannelOpen(kControlReliableChannel)) {
                reset.contextId = snapshot_.sessionId;
                reset.screenShareGeneration =
                    snapshot_.screenShareGeneration;
                directController = sessionController_.get();
            } else if (snapshot_.room.membership ==
                           RoomMembershipState::kActive &&
                       snapshot_.room.screenSharerDeviceId ==
                           snapshot_.localDeviceId &&
                       snapshot_.room.screenShareEpoch != 0) {
                reset.contextId = snapshot_.room.roomId;
                reset.screenShareGeneration =
                    snapshot_.room.screenShareEpoch;
                for (const auto& [pairId, pair] : roomPairs_) {
                    (void)pairId;
                    const auto reliable = pair->openDataChannels.find(
                        std::string(kControlReliableChannel));
                    if (pair->controller && reliable !=
                            pair->openDataChannels.end() &&
                        reliable->second) {
                        roomRecipients.push_back(pair);
                    }
                }
            }
        }
        latestLocalCursorPosition_.reset();
        latestLocalCursorShape_.reset();
    }
    if (reset.contextId.empty()) return;
    std::vector<std::uint8_t> encoded;
    if (!EncodeRemoteCursorMessage(reset, &encoded)) return;
    if (directController) {
        (void)directController->QueueData(
            std::string(kControlReliableChannel), encoded, true);
    }
    for (const auto& pair : roomRecipients) {
        (void)pair->controller->QueueData(
            std::string(kControlReliableChannel), encoded, true);
    }
}

void InProcessSessionEngine::OnLocalCursorObservation(
    WindowsCursorObservation observation)
{
    std::string contextId;
    std::string senderDeviceId;
    std::uint64_t generation = 0;
    SessionControllerBase* directController = nullptr;
    std::vector<std::shared_ptr<RoomPairRuntime>> roomRecipients;
    RemoteCursorEnvelope position;
    std::optional<RemoteCursorEnvelope> shape;
    {
        std::lock_guard lock(mutex_);
        if (snapshot_.activeSharedDisplay.sessionDisplayId == 0 ||
            observation.position.displayId !=
                snapshot_.activeSharedDisplay.sessionDisplayId ||
            observation.position.displayLayoutVersion !=
                snapshot_.activeSharedDisplayLayoutVersion) {
            return;
        }
        senderDeviceId = snapshot_.localDeviceId;
        if (snapshot_.remoteControlRole == RemoteControlRole::kControlled &&
            !snapshot_.sessionId.empty() && sessionController_) {
            contextId = snapshot_.sessionId;
            generation = snapshot_.screenShareGeneration;
            if (directSession_.IsChannelOpen(kTelemetryChannel)) {
                directController = sessionController_.get();
            }
        } else if (snapshot_.room.membership ==
                       RoomMembershipState::kActive &&
                   snapshot_.room.screenSharerDeviceId ==
                       snapshot_.localDeviceId &&
                   snapshot_.room.screenShareEpoch != 0) {
            contextId = snapshot_.room.roomId;
            generation = snapshot_.room.screenShareEpoch;
            for (const auto& [pairId, pair] : roomPairs_) {
                (void)pairId;
                const auto telemetry = pair->openDataChannels.find(
                    std::string(kTelemetryChannel));
                if (pair->controller && telemetry !=
                        pair->openDataChannels.end() && telemetry->second) {
                    roomRecipients.push_back(pair);
                }
            }
        } else {
            return;
        }

        latestLocalCursorPosition_ = observation.position;
        ++cursorPositionsPublished_;
        if (observation.changedShape) {
            latestLocalCursorShape_ = *observation.changedShape;
            ++cursorShapesPublished_;
        }
        position.type = RemoteCursorMessageType::kPosition;
        position.contextId = contextId;
        position.senderDeviceId = senderDeviceId;
        position.sequence = ++nextCursorSequence_;
        position.timestampUs = static_cast<std::uint64_t>(
            std::chrono::duration_cast<std::chrono::microseconds>(
                std::chrono::steady_clock::now().time_since_epoch()).count());
        position.screenShareGeneration = generation;
        position.position = observation.position;
        if (observation.changedShape) {
            shape.emplace();
            shape->type = RemoteCursorMessageType::kShape;
            shape->contextId = contextId;
            shape->senderDeviceId = senderDeviceId;
            shape->sequence = ++nextCursorSequence_;
            shape->timestampUs = position.timestampUs;
            shape->screenShareGeneration = generation;
            shape->shape = std::move(*observation.changedShape);
        }
    }

    if (shape) {
        std::vector<std::uint8_t> encoded;
        if (EncodeRemoteCursorMessage(*shape, &encoded)) {
            if (directController) {
                (void)directController->QueueData(
                    std::string(kControlReliableChannel), encoded, true);
            }
            for (const auto& pair : roomRecipients) {
                const auto reliable = pair->openDataChannels.find(
                    std::string(kControlReliableChannel));
                if (reliable != pair->openDataChannels.end() &&
                    reliable->second) {
                    (void)pair->controller->QueueData(
                        std::string(kControlReliableChannel), encoded, true);
                }
            }
        }
    }
    std::vector<std::uint8_t> encoded;
    if (!EncodeRemoteCursorMessage(position, &encoded)) return;
    if (directController) {
        (void)directController->QueueData(
            std::string(kTelemetryChannel), encoded, true);
    }
    for (const auto& pair : roomRecipients) {
        (void)pair->controller->QueueData(
            std::string(kTelemetryChannel), encoded, true);
    }
}

void InProcessSessionEngine::RepublishRemoteCursor()
{
    WindowsCursorObservation observation;
    {
        std::lock_guard lock(mutex_);
        if (!latestLocalCursorPosition_) return;
        observation.position = *latestLocalCursorPosition_;
        observation.changedShape = latestLocalCursorShape_;
    }
    OnLocalCursorObservation(std::move(observation));
}

bool InProcessSessionEngine::DispatchRemoteCursorData(
    const std::string& pairId,
    const std::string& label,
    std::span<const std::uint8_t> payload)
{
    if (label != kTelemetryChannel && label != kControlReliableChannel) {
        return false;
    }
    RemoteCursorEnvelope envelope;
    if (!DecodeRemoteCursorMessage(payload, &envelope)) return false;
    const bool expectedChannel =
        envelope.type == RemoteCursorMessageType::kPosition
            ? label == kTelemetryChannel
            : label == kControlReliableChannel;
    if (!expectedChannel) return true;

    RemoteCursorCallback callback;
    {
        std::lock_guard lock(mutex_);
        if (pairId.empty()) {
            if (snapshot_.remoteControlRole != RemoteControlRole::kController ||
                envelope.contextId != snapshot_.sessionId ||
                envelope.senderDeviceId != snapshot_.peerDeviceId ||
                envelope.screenShareGeneration == 0 ||
                (snapshot_.directRemoteScreenShareGeneration != 0 &&
                 envelope.screenShareGeneration !=
                     snapshot_.directRemoteScreenShareGeneration)) {
                return true;
            }
            auto& lastSequence =
                envelope.type == RemoteCursorMessageType::kPosition
                    ? directSession_.lastCursorPositionSequence
                    : directSession_.lastCursorShapeSequence;
            if (envelope.sequence <= lastSequence) return true;
            lastSequence = envelope.sequence;
        } else {
            const auto pairIt = roomPairs_.find(pairId);
            if (pairIt == roomPairs_.end() || !pairIt->second ||
                snapshot_.room.membership != RoomMembershipState::kActive ||
                snapshot_.room.screenSharerDeviceId !=
                    pairIt->second->peerDeviceId ||
                envelope.contextId != snapshot_.room.roomId ||
                envelope.contextId != pairIt->second->roomId ||
                envelope.senderDeviceId != pairIt->second->peerDeviceId ||
                envelope.screenShareGeneration !=
                    snapshot_.room.screenShareEpoch) {
                return true;
            }
            auto& lastSequence =
                envelope.type == RemoteCursorMessageType::kPosition
                    ? pairIt->second->lastCursorPositionSequence
                    : pairIt->second->lastCursorShapeSequence;
            if (envelope.sequence <= lastSequence) return true;
            lastSequence = envelope.sequence;
        }
        if (envelope.type == RemoteCursorMessageType::kPosition) {
            ++cursorPositionsReceived_;
        } else if (envelope.type == RemoteCursorMessageType::kShape) {
            ++cursorShapesReceived_;
        }
        callback = remoteCursorCallback_;
    }
    if (callback) callback(pairId, envelope);
    return true;
}
