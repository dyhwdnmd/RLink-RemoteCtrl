// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#include <chrono>
#include <atomic>
#include <cstdint>
#include <iostream>
#include <string>
#include <thread>

#include "api/make_ref_counted.h"
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
    std::cout << "Agent stage " << stage << ": NO\n";
    if (!detail.empty()) {
        std::cout << "Agent error: " << detail << '\n';
    }
    std::cout << "AGENT_EXIT_CODE=1\n" << std::flush;
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
    remote::testing::ProcessSessionObserver observer(true);
    remote::LibWebRtcSession session(factory);
    remote::AgentSessionController controller(session, signaling);
    observer.SetController(&controller);
    controller.SetObserver(&observer);
    signaling.SetReceiver(&controller);

    auto source = webrtc::make_ref_counted<
        remote::testing::SyntheticVideoSource>();
    auto videoTrack = factory->CreateVideoTrack(source, "agent-screen");
    std::atomic<std::size_t> codecCount{0};
    controller.SetAnswerPreparation(
        [&session, factory, videoTrack, &codecCount]()
            -> std::optional<remote::OperationError> {
            auto transceiver = session.AddVideoTrack(videoTrack);
            if (!transceiver.ok()) {
                return remote::OperationError{
                    "agent_video_track_attach_failed",
                    transceiver.error().message()};
            }

            std::size_t negotiatedCodecCount = 0;
            std::string codecError;
            if (!remote::testing::ApplyH264CodecPreferences(
                    factory.get(), transceiver.value(),
                    &negotiatedCodecCount, &codecError)) {
                return remote::OperationError{
                    "agent_h264_preference_failed", codecError};
            }
            codecCount.store(negotiatedCodecCount,
                             std::memory_order_release);
            return std::nullopt;
        });

    remote::SessionControllerConfig config;
    config.webRtc.includeLoopbackAdapter = true;
    config.negotiationTimeout = operationTimeout;
    controller.Start(config);
    if (!observer.WaitForState(remote::SessionControllerState::kReady,
                               operationTimeout)) {
        PrintFailure("PeerConnection creation", observer.ErrorText());
        return 1;
    }

    if (!signaling.StartListening(port)) {
        PrintFailure("local signaling listen", signaling.LastError());
        return 1;
    }

    std::cout << "Agent PeerConnection ready: YES\n";
    std::cout << "Agent video answer preparation ready: YES\n";
    std::cout << "AGENT_READY port=" << port << '\n' << std::flush;

    const bool signalingConnected =
        signaling.WaitUntilConnected(operationTimeout);
    const bool peerConnected = signalingConnected &&
        observer.WaitForState(remote::SessionControllerState::kConnected,
                              operationTimeout);
    const auto channelSpecs = remote::DefaultRemoteControlDataChannels();
    const bool channelsOpen = peerConnected &&
        observer.WaitForAllChannelsOpen(channelSpecs, operationTimeout);
    const bool channelParametersMatch = channelsOpen &&
        observer.AllChannelParametersMatch(channelSpecs);

    int framesPushed = 0;
    const auto videoDeadline = std::chrono::steady_clock::now() + 30s;
    while (channelsOpen &&
           std::chrono::steady_clock::now() < videoDeadline &&
           !observer.ShutdownRequested()) {
        source->PushFrame(remote::testing::CreateSyntheticFrame(framesPushed));
        ++framesPushed;
        std::this_thread::sleep_for(1000ms /
                                    remote::testing::kTestFrameRate);
    }
    const bool shutdownReceived = observer.ShutdownRequested() ||
        observer.WaitForShutdown(500ms);

    std::cout << "Agent signaling connected: "
              << (signalingConnected ? "YES" : "NO") << '\n';
    std::cout << "Agent WebRTC connected: "
              << (peerConnected ? "YES" : "NO") << '\n';
    std::cout << "Agent video transceiver attached: "
              << (codecCount.load(std::memory_order_acquire) > 0
                      ? "YES" : "NO")
              << '\n';
    std::cout << "Agent H264 codec preferences: "
              << codecCount.load(std::memory_order_acquire) << '\n';
    std::cout << "Agent six DataChannels open: "
              << (channelsOpen ? "YES" : "NO") << '\n';
    std::cout << "Agent DataChannel parameters verified: "
              << (channelParametersMatch ? "YES" : "NO") << '\n';
    std::cout << "Agent synthetic frames pushed: " << framesPushed << '\n';
    std::cout << "Agent shutdown command received: "
              << (shutdownReceived ? "YES" : "NO") << '\n';
    if (!observer.ErrorText().empty()) {
        std::cout << "Agent controller error: " << observer.ErrorText()
                  << '\n';
    }

    controller.Close();
    const bool closed = observer.WaitForState(
        remote::SessionControllerState::kClosed, 3s);
    signaling.SetReceiver(nullptr);
    signaling.Stop();
    controller.SetObserver(nullptr);
    observer.SetController(nullptr);

    const bool passed = signalingConnected && peerConnected && channelsOpen &&
                        channelParametersMatch && framesPushed > 0 &&
                        codecCount.load(std::memory_order_acquire) > 0 &&
                        shutdownReceived && closed;
    std::cout << "Agent session closed: " << (closed ? "YES" : "NO")
              << '\n';
    std::cout << "AGENT_SELF_TEST_PASSED: "
              << (passed ? "YES" : "NO") << '\n';
    std::cout << "AGENT_EXIT_CODE=" << (passed ? 0 : 1) << '\n'
              << std::flush;
    return passed ? 0 : 1;
}
