// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#include "InProcessSessionEngine.h"

#include <algorithm>
#include <array>
#include <chrono>
#include <limits>
#include <optional>
#include <random>
#include <sstream>
#include <utility>
#include <vector>

#include "api/make_ref_counted.h"
#include "api/audio_options.h"
#include "api/video/adapted_video_track_source.h"
#include "src/protocol/DataChannelCatalog.h"
#include "src/protocol/ClipboardProtocol.h"
#include "src/protocol/FileTransferProtocol.h"
#include "src/protocol/RemoteInputProtocol.h"
#include "src/protocol/RemoteCursorProtocol.h"
#include "src/protocol/RoomMemberControlProtocol.h"
#include "src/protocol/ScreenShareControlProtocol.h"
#include "src/core/ScreenStreamPolicy.h"
#include "src/core/VideoPresentationTelemetry.h"
#include "src/platform/win/WindowsDisplayTopology.h"
#include "src/webrtc/LibWebRtcSession.h"
#include "src/webrtc/WebRtcRuntime.h"
#include "InProcessSessionEngineInternal.h"

namespace remote::app {
namespace {

SessionCommandResult Success()
{
    return {true, {}, {}};
}

SessionCommandResult Failure(std::string code, std::string message)
{
    return {false, std::move(code), std::move(message)};
}

std::string GenerateRotatedVerificationCode(
    const std::string& previous)
{
    std::string code;
    static thread_local std::mt19937 generator{
        std::random_device{}()};
    static thread_local std::uniform_int_distribution<std::uint32_t>
        distribution(0, 999'999);
    do {
        const auto value = distribution(generator);
        code = std::to_string(1'000'000u + value).substr(1);
    } while (code == previous);
    return code;
}

std::string NormalizeMediaDeviceId(const std::string& deviceId)
{
    return deviceId.empty()
        ? std::string(kSystemDefaultMediaDeviceId)
        : deviceId;
}

bool ContainsMediaDevice(
    const std::vector<MediaDeviceDescriptor>& devices,
    const std::string& deviceId)
{
    return std::any_of(
        devices.begin(), devices.end(),
        [&deviceId](const MediaDeviceDescriptor& device) {
            return device.id == deviceId && device.available;
        });
}

bool IsRoomVideoSlot(const std::string& slot)
{
    return slot == kScreenMainVideoSlot ||
           slot == kCameraMainVideoSlot;
}

bool ShouldBoostDesktopCaptureForInput(const RemoteInputEvent& event)
{
    switch (event.type) {
    case RemoteInputMessageType::kMouseMove:
        // Plain hover movement should not keep the desktop capturer boosted.
        // During a drag, every move extends the short 50 ms capture window.
        return event.pressedMouseButtons != 0;
    case RemoteInputMessageType::kMouseButton:
    case RemoteInputMessageType::kMouseWheel:
    case RemoteInputMessageType::kKey:
    case RemoteInputMessageType::kReleaseAll:
        return true;
    default:
        return false;
    }
}

constexpr std::array<const char*, 2> kRoomVideoSlots = {
    kScreenMainVideoSlot, kCameraMainVideoSlot};

constexpr std::uint64_t kMaximumFileTransferBufferedBytes = 8 * 1024 * 1024;
// Clipboard paste shares the PeerConnection with the live desktop video.
// Keep its cancellable in-flight tail small: libwebrtc cannot retract bytes
// already accepted by an ordered reliable SCTP stream, so a large buffer
// would continue competing with video for seconds after the user cancels.
constexpr std::uint64_t kMaximumClipboardBufferedBytes = 512 * 1024;
constexpr std::array<std::uint8_t, 4> kClipboardWarmupMagic = {
    'R', 'C', 'W', '1'};

bool IsClipboardWarmupPayload(std::span<const std::uint8_t> payload)
{
    return payload.size() >= kClipboardWarmupMagic.size() &&
        std::equal(kClipboardWarmupMagic.begin(),
                   kClipboardWarmupMagic.end(), payload.begin());
}

ScreenStreamPolicyResult ResolveScreenPolicy(
    std::uint32_t sourceWidth,
    std::uint32_t sourceHeight,
    const ScreenStreamPreferenceRequest& request)
{
    return ResolveScreenStreamPolicy(
        sourceWidth,
        sourceHeight,
        {request.maxWidth, request.maxHeight, request.framesPerSecond});
}

std::string EncoderPreferenceName(VideoEncoderPreference preference)
{
    switch (preference) {
    case VideoEncoderPreference::kHardwareOnly:
        return "hardware";
    case VideoEncoderPreference::kFfmpegHardware:
        return "ffmpeg_hardware";
    case VideoEncoderPreference::kSoftwareOnly:
        return "software";
    case VideoEncoderPreference::kFfmpegX264Only:
        return "ffmpeg";
    case VideoEncoderPreference::kAutomatic:
    default:
        return "auto";
    }
}

std::string DecoderPreferenceName(VideoDecoderPreference preference)
{
    switch (preference) {
    case VideoDecoderPreference::kHardwareOnly:
        return "hardware";
    case VideoDecoderPreference::kSoftwareOnly:
        return "software";
    case VideoDecoderPreference::kAutomatic:
    default:
        return "auto";
    }
}

std::string DesktopCaptureImplementationName(
    DesktopCaptureImplementation implementation)
{
    switch (implementation) {
    case DesktopCaptureImplementation::kLibWebRtc:
        return "libwebrtc";
    case DesktopCaptureImplementation::kNativeDxgi:
    default:
        return "native_dxgi";
    }
}

std::string DesktopCaptureBackendName(
    WindowsDesktopCaptureSource::CaptureBackend backend)
{
    switch (backend) {
    case WindowsDesktopCaptureSource::CaptureBackend::kDxgiNativeTexture:
        return "native_dxgi_texture";
    case WindowsDesktopCaptureSource::CaptureBackend::kDxgiPreferred:
        return "libwebrtc_dxgi_with_gdi_fallback";
    case WindowsDesktopCaptureSource::CaptureBackend::kGdi:
    default:
        return "libwebrtc_gdi";
    }
}

std::uint32_t MaximumDesktopCaptureFrameRate(
    DesktopCaptureImplementation implementation,
    const WindowsDesktopCaptureSource* source = nullptr)
{
    if (implementation == DesktopCaptureImplementation::kLibWebRtc) {
        return kMaximumScreenFrameRate;
    }
    if (source && source->Backend() !=
            WindowsDesktopCaptureSource::CaptureBackend::
                kDxgiNativeTexture) {
        return 60u;
    }
    return kMaximumScreenFrameRate;
}

std::string DescribeEncoderRuntimeInstance(
    const VideoEncoderInstanceRuntimeStatus& status)
{
    std::string state = status.state;
    if (state == "hardware_created") {
        state = "硬件编码器已创建";
    } else if (state == "hardware_active") {
        state = "硬件编码中";
    } else if (state == "hardware_stopped") {
        state = "硬件编码已停止";
    } else if (state == "software_fallback") {
        state = "已回退软件编码";
    } else if (state == "software_active") {
        state = "软件编码中";
    }
    std::string input = status.inputFormat;
    if (input == "D3D11 BGRA desktop texture") {
        input = "桌面 D3D11 BGRA 原生纹理";
    } else if (input == "CPU BGRA desktop") {
        input = "桌面 CPU BGRA";
    } else if (input == "CPU I420") {
        input = "摄像头/CPU I420";
    } else if (input == "CPU NV12") {
        input = "CPU NV12";
    }
    std::ostringstream stream;
    stream << "实例 #" << status.instanceId << " · "
           << (status.implementation.empty()
                   ? "未知编码器"
                   : status.implementation)
           << " | 状态：" << state;
    if (!input.empty()) {
        stream << " | 输入：" << input;
    }
    if (status.width != 0 && status.height != 0) {
        stream << " | " << status.width << 'x' << status.height;
    }
    if (status.initMaxFrameRate != 0) {
        stream << " | 初始化参数（历史）："
               << status.initMinBitrateBps << '/'
               << status.initStartBitrateBps << '/'
               << status.initMaxBitrateBps
               << " bps · 最大 "
               << status.initMaxFrameRate << " FPS";
    }
    if (status.frameRate != 0) {
        stream << " | WebRTC 参考帧率："
               << status.frameRate << " FPS";
    }
    if (status.observedInputFrameRate != 0) {
        stream << " | Encode 输入："
               << status.observedInputFrameRate << " FPS";
    }
    if (status.observedOutputFrameRate != 0) {
        stream << " | 编码回调输出："
               << status.observedOutputFrameRate << " FPS";
    }
    if (status.totalInputFrames != 0 ||
        status.totalOutputFrames != 0) {
        stream << " | 帧计数：输入 "
               << status.totalInputFrames
               << " / 输出 " << status.totalOutputFrames
               << " / 编码器拒绝 " << status.totalDroppedFrames;
    }
    if (status.configuredFrameRate != 0) {
        stream << " | MFT 声明帧率："
               << status.configuredFrameRate << " FPS";
    }
    if (status.targetBitrateBps != 0) {
        stream << " | SetRates #" << status.rateUpdateSequence
               << " 请求码率："
               << status.targetBitrateBps << " bps";
        if (status.lastRateUpdateUnixMs != 0) {
            const auto nowMs = static_cast<std::uint64_t>(
                std::chrono::duration_cast<std::chrono::milliseconds>(
                    std::chrono::system_clock::now().time_since_epoch())
                    .count());
            stream << "（"
                   << (nowMs >= status.lastRateUpdateUnixMs
                           ? nowMs - status.lastRateUpdateUnixMs
                           : 0)
                   << " ms 前）";
        }
    }
    if (status.implementation == "MediaFoundationD3D11H264") {
        stream << " | MFT 动态码率设置："
               << (status.bitrateConfigurationAccepted
                       ? "成功"
                       : "失败");
        if (status.bitrateReadbackAvailable) {
            stream << " | MFT 读回码率："
                   << status.configuredBitrateBps << " bps";
        } else {
            stream << " | MFT 读回码率：驱动未提供";
        }
    }
    if (!status.fallbackReason.empty()) {
        stream << " | 回退原因：" << status.fallbackReason;
    }
    return stream.str();
}

class IdleRoomVideoSource : public webrtc::AdaptedVideoTrackSource {
public:
    SourceState state() const override { return kLive; }
    bool remote() const override { return false; }
    bool is_screencast() const override { return true; }
    std::optional<bool> needs_denoising() const override { return false; }
};

}  // namespace

#include "InProcessSessionEngine.Lifecycle.inc"
#include "InProcessSessionEngine.DirectSession.inc"
#include "InProcessSessionEngine.Room.inc"
#include "InProcessSessionEngine.ScreenShare.inc"
#include "InProcessSessionEngine.RemoteControl.inc"
#include "InProcessSessionEngine.ScreenPolicyFile.inc"
#include "InProcessSessionEngine.Camera.inc"
#include "InProcessSessionEngine.Audio.inc"
#include "InProcessSessionEngine.MediaDevices.inc"
#include "InProcessSessionEngine.MediaHooks.inc"
#include "InProcessSessionEngine.VideoSlots.inc"
#include "InProcessSessionEngine.RemoteCursor.inc"
#include "InProcessSessionEngine.DirectMedia.inc"
#include "InProcessSessionEngine.DirectData.inc"
#include "InProcessSessionEngine.DirectScreen.inc"
#include "InProcessSessionEngine.RuntimeHelpers.inc"
#include "InProcessSessionEngine.DirectCallbacks.inc"
#include "InProcessSessionEngine.RoomCallbacks.inc"
#include "InProcessSessionEngine.LeaseCallbacks.inc"
#include "InProcessSessionEngine.RoomPairLifecycle.inc"
#include "InProcessSessionEngine.RoomPairState.inc"
#include "InProcessSessionEngine.RoomPairChannels.inc"
void InProcessSessionEngine::OnRoomPairDataMessage(
    const std::string& pairId,
    const std::string& label,
    std::span<const std::uint8_t> payload,
    bool binary)
{
#include "InProcessSessionEngine.RoomPairDispatchPreamble.inc"
#include "InProcessSessionEngine.RoomPairControlDispatch.inc"
#include "InProcessSessionEngine.RoomPairScreenDispatch.inc"
#include "InProcessSessionEngine.RoomPairInputDispatch.inc"
#include "InProcessSessionEngine.DirectBridge.inc"
void InProcessSessionEngine::PublishSnapshot()
{
    ISessionEngineObserver* observer = nullptr;
    SessionEngineSnapshot snapshot;
    {
        std::lock_guard lock(mutex_);
        observer = observer_;
        snapshot = snapshot_;
    }
    if (observer) {
        observer->OnSessionEngineSnapshot(snapshot);
    }
}

}  // namespace remote::app
