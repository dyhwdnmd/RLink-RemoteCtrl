// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#include "DataChannelCatalog.h"

#include <string>
#include <utility>

namespace remote {

std::vector<DataChannelSpec> DefaultRemoteControlDataChannels()
{
    DataChannelSpec inputFast;
    inputFast.label = std::string(kInputFastChannel);
    inputFast.ordered = false;
    inputFast.maxRetransmits = 0;
    inputFast.protocol = "remote-control.input.v1";

    DataChannelSpec controlReliable;
    controlReliable.label = std::string(kControlReliableChannel);
    controlReliable.ordered = true;
    controlReliable.protocol = "remote-control.control.v2";

    DataChannelSpec fileTransfer;
    fileTransfer.label = std::string(kFileTransferChannel);
    fileTransfer.ordered = true;
    fileTransfer.protocol = "remote-control.file.v4";
    // File bytes are reliable but bulk. Keep them below interactive control
    // traffic while the application-level receive window maintains
    // throughput without filling SCTP memory.
    fileTransfer.priority = DataChannelSpec::Priority::kLow;

    DataChannelSpec clipboardReliable;
    clipboardReliable.label = std::string(kClipboardReliableChannel);
    clipboardReliable.ordered = true;
    clipboardReliable.protocol = "remote-control.clipboard.v8";

    DataChannelSpec clipboardTransfer;
    clipboardTransfer.label = std::string(kClipboardTransferChannel);
    clipboardTransfer.ordered = true;
    clipboardTransfer.protocol = "remote-control.clipboard-transfer.v8";
    // Screen video is the latency-sensitive product path. Clipboard file
    // bytes remain reliable, but must yield whenever the shared SCTP/ICE
    // transport is carrying live desktop media.
    clipboardTransfer.priority = DataChannelSpec::Priority::kLow;

    DataChannelSpec telemetry;
    telemetry.label = std::string(kTelemetryChannel);
    telemetry.ordered = false;
    telemetry.maxRetransmits = 0;
    telemetry.protocol = "remote-control.telemetry.v2";

    return {std::move(inputFast), std::move(controlReliable),
            std::move(fileTransfer), std::move(clipboardReliable),
            std::move(clipboardTransfer), std::move(telemetry)};
}

std::vector<DataChannelSpec> DefaultCameraSessionDataChannels()
{
    DataChannelSpec controlReliable;
    controlReliable.label = std::string(kControlReliableChannel);
    controlReliable.ordered = true;
    controlReliable.protocol = "remote-control.control.v1";

    DataChannelSpec telemetry;
    telemetry.label = std::string(kTelemetryChannel);
    telemetry.ordered = false;
    telemetry.maxRetransmits = 0;
    telemetry.protocol = "remote-control.telemetry.v1";

    return {std::move(controlReliable), std::move(telemetry)};
}

}  // namespace remote
