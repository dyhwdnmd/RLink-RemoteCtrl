// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#pragma once

#include <utility>

#include <QString>

#include "src/core/ISessionEngine.h"

namespace remote::controller {

enum class RemoteSessionMode {
    kDirect,
    kRoom,
};

// Immutable-by-convention UI routing context. Authorization origin explains
// how a session was admitted; mode determines how media and control are bound.
// Owned-device and verification-code sessions are both direct after admission.
struct RemoteSessionBinding {
    RemoteSessionMode mode = RemoteSessionMode::kDirect;
    SessionOrigin origin = SessionOrigin::kManualDeviceId;
    QString peerDeviceId;
    QString peerDeviceName;
    QString roomPairId;

    static RemoteSessionBinding Direct(
        QString deviceId,
        QString deviceName,
        SessionOrigin sessionOrigin)
    {
        return {RemoteSessionMode::kDirect,
                sessionOrigin,
                std::move(deviceId),
                std::move(deviceName),
                {}};
    }

    static RemoteSessionBinding Room(
        QString deviceId,
        QString deviceName,
        QString pairId)
    {
        return {RemoteSessionMode::kRoom,
                SessionOrigin::kCollaborationRoom,
                std::move(deviceId),
                std::move(deviceName),
                std::move(pairId)};
    }

    bool IsDirect() const { return mode == RemoteSessionMode::kDirect; }
    bool IsRoom() const { return mode == RemoteSessionMode::kRoom; }
    bool IsValid() const
    {
        return !peerDeviceId.isEmpty() &&
            ((IsDirect() && roomPairId.isEmpty()) ||
             (IsRoom() && !roomPairId.isEmpty()));
    }

    bool SameTransport(const RemoteSessionBinding& other) const
    {
        return mode == other.mode &&
            peerDeviceId == other.peerDeviceId &&
            roomPairId == other.roomPairId;
    }

    QString SourceText() const
    {
        if (IsRoom()) {
            return QStringLiteral("协作房间");
        }
        switch (origin) {
        case SessionOrigin::kOwnedDevice:
            return QStringLiteral("我的设备");
        case SessionOrigin::kRemoteAssistance:
            return QStringLiteral("远程协助");
        default:
            return QStringLiteral("手动设备 ID");
        }
    }
};

}  // namespace remote::controller
