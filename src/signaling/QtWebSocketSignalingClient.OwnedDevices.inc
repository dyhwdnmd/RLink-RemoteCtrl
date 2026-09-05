// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

    SignalingOperationResult RequestOwnedDevices()
    {
        if (auto ready = RequireRegistered(); !ready.accepted) {
            return ready;
        }
        return SendEnvelope(QStringLiteral("my_devices_request"), {}, {});
    }
