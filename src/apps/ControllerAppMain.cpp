// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#include <chrono>
#include <cstdint>
#include <iostream>
#include <string>
#include <thread>

#include "src/core/SessionController.h"
#include "src/protocol/DataChannelCatalog.h"
#include "src/signaling/LocalTcpSignaling.h"
#include "src/testing/ProcessSelfTestSupport.h"
#include "src/webrtc/LibWebRtcSession.h"
#include "src/webrtc/WebRtcRuntime.h"

namespace {

using namespace std::chrono_literals;

std::uint16_t ParsePort(int argc, char* argv[])
{
    constexpr std::uint16_t defaultPort = 38473;
    for (int index = 1; index < argc; ++index) {
        const std::string argument = argv[index];
        std::string value;
        if (argument == "--port" && index + 1 < argc) {
            value = argv[++index];
        } else if (argument.starts_with("--port=")) {
            value = argument.substr(7);
        } else {
            continue;
        }
        try {
            const int parsed = std::stoi(value);
            if (parsed > 0 && parsed <= 65535) {
                return static_cast<std::uint16_t>(parsed);
            }
        } catch (...) {
        }
    }
    return defaultPort;
}

void PrintFailure(const std::string& stage, const std::string& detail)
{
    std::cout << "Controller stage " << stage << ": NO\n";
    if (!detail.empty()) {
        std::cout << "Controller error: " << detail << '\n';
    }
    std::cout << "CONTROLLER_EXIT_CODE=1\n" << std::flush;
}

}  // namespace

int main(int argc, char* argv[])
{
    const auto port = ParsePort(argc, argv);
    constexpr auto operationTimeout = 15s;

    remote::WebRtcRuntime runtime;
    if (!runtime.Initialize()) {
        PrintFailure("runtime initialization",
                     runtime.CapabilityReport().error);
        return 1;
    }

    auto factory = runtime.PeerConnectionFactory();
    remote::LocalTcpSignaling signaling;
    remote::testing::ProcessSessionObserver observer;
    remote::testing::NativeFrameSink videoSink;
    remote::LibWebRtcSession session(factory);
    remote::ControllerSessionController controller(session, signaling);
    observer.SetController(&controller);
    controller.SetObserver(&observer);
    signaling.SetReceiver(&controller);
    session.SetRemoteVideoSink(&videoSink);

    remote::SessionControllerConfig config;
    config.webRtc.includeLoopbackAdapter = true;
    config.negotiationTimeout = operationTimeout;
    controller.Start(config);
    if (!observer.WaitForState(remote::SessionControllerState::kReady,
                               operationTimeout)) {
        PrintFailure("PeerConnection creation", observer.ErrorText());
        return 1;
    }

    auto transceiver = session.AddVideoReceiveTransceiver();
    if (!transceiver.ok()) {
        PrintFailure("receive transceiver creation",
                     transceiver.error().message());
        return 1;
    }

    std::size_t codecCount = 0;
    std::string codecError;
    if (!remote::testing::ApplyH264CodecPreferences(
            factory.get(), transceiver.value(), &codecCount, &codecError)) {
        PrintFailure("H264 codec preference", codecError);
        return 1;
    }

    if (!signaling.Connect("127.0.0.1", port, operationTimeout)) {
        PrintFailure("local signaling connect", signaling.LastError());
        return 1;
    }

    const auto channelSpecs = remote::DefaultRemoteControlDataChannels();
    controller.Connect(channelSpecs);
    const bool peerConnected = observer.WaitForState(
        remote::SessionControllerState::kConnected, operationTimeout);
    const bool channelsOpen = peerConnected &&
        observer.WaitForAllChannelsOpen(channelSpecs, operationTimeout);
    const bool channelParametersMatch = channelsOpen &&
        observer.AllChannelParametersMatch(channelSpecs);

    bool allMessagesPassed = channelsOpen;
    for (const auto& spec : channelSpecs) {
        const std::string request = spec.label + "-probe";
        const std::string response = spec.label + "-ack";
        bool received = false;
        for (int attempt = 0; attempt < 5 && !received; ++attempt) {
            const bool sent = controller.SendData(
                spec.label,
                remote::testing::AsBytes(request),
                false) == remote::SendResult::kSent;
            received = sent && observer.WaitForMessage(
                spec.label, response, 1s);
        }
        allMessagesPassed = allMessagesPassed && received;
        std::cout << "Controller channel " << spec.label
                  << " bidirectional message: "
                  << (received ? "YES" : "NO") << '\n';
    }

    const bool nativeFramesReceived =
        videoSink.WaitForNativeFrames(3, operationTimeout);
    const bool decodedSizeMatches =
        videoSink.Width() == remote::testing::kTestFrameWidth &&
        videoSink.Height() == remote::testing::kTestFrameHeight;

    const std::string shutdown = "shutdown";
    const bool shutdownSent = channelsOpen &&
        controller.SendData(
            std::string(remote::kControlReliableChannel),
            remote::testing::AsBytes(shutdown),
            false) == remote::SendResult::kSent;
    if (shutdownSent) {
        std::this_thread::sleep_for(200ms);
    }

    std::cout << "Controller PeerConnection ready: YES\n";
    std::cout << "Controller receive transceiver ready: YES\n";
    std::cout << "Controller H264 codec preferences: " << codecCount << '\n';
    std::cout << "Controller local signaling connected: YES\n";
    std::cout << "Controller WebRTC connected: "
              << (peerConnected ? "YES" : "NO") << '\n';
    const auto controllerDirection =
        transceiver.value()->current_direction();
    std::cout << "Controller negotiated video direction: "
              << (controllerDirection
                      ? std::to_string(static_cast<int>(*controllerDirection))
                      : "none")
              << '\n';
    std::cout << "Controller remote tracks observed: "
              << observer.RemoteTrackCount() << '\n';
    std::cout << "Controller six DataChannels open: "
              << (channelsOpen ? "YES" : "NO") << '\n';
    std::cout << "Controller DataChannel parameters verified: "
              << (channelParametersMatch ? "YES" : "NO") << '\n';
    std::cout << "Controller six DataChannels exchanged messages: "
              << (allMessagesPassed ? "YES" : "NO") << '\n';
    std::cout << "Controller decoded frames received: "
              << videoSink.FrameCount() << '\n';
    std::cout << "Controller D3D11 native frames received: "
              << videoSink.NativeFrameCount() << '\n';
    std::cout << "Controller decoded size: " << videoSink.Width() << 'x'
              << videoSink.Height() << '\n';
    std::cout << "Controller MF/D3D11 decode confirmed: "
              << (nativeFramesReceived ? "YES" : "NO") << '\n';
    std::cout << "Controller decoded size matches source: "
              << (decodedSizeMatches ? "YES" : "NO") << '\n';
    std::cout << "Controller shutdown command sent: "
              << (shutdownSent ? "YES" : "NO") << '\n';
    if (!observer.ErrorText().empty()) {
        std::cout << "Controller session error: " << observer.ErrorText()
                  << '\n';
    }

    controller.Close();
    const bool closed = observer.WaitForState(
        remote::SessionControllerState::kClosed, 3s);
    signaling.SetReceiver(nullptr);
    signaling.Stop();
    controller.SetObserver(nullptr);
    observer.SetController(nullptr);

    const bool passed = peerConnected && channelsOpen &&
                        channelParametersMatch && allMessagesPassed &&
                        nativeFramesReceived && decodedSizeMatches &&
                        shutdownSent && closed;
    std::cout << "Controller session closed: "
              << (closed ? "YES" : "NO") << '\n';
    std::cout << "CONTROLLER_SELF_TEST_PASSED: "
              << (passed ? "YES" : "NO") << '\n';
    std::cout << "CONTROLLER_EXIT_CODE=" << (passed ? 0 : 1) << '\n'
              << std::flush;
    return passed ? 0 : 1;
}
