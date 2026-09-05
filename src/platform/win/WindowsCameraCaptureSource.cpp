// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#include "WindowsCameraCaptureSource.h"

#include <algorithm>
#include <array>
#include <cstdint>
#include <memory>

#include "modules/video_capture/video_capture_defines.h"
#include "modules/video_capture/video_capture_factory.h"

namespace remote {
namespace {

constexpr std::size_t kDeviceTextCapacity = 512;

}  // namespace

WindowsCameraCaptureSource::WindowsCameraCaptureSource() = default;

WindowsCameraCaptureSource::~WindowsCameraCaptureSource()
{
    StopCapture();
}

std::vector<CameraDeviceInfo>
WindowsCameraCaptureSource::EnumerateDevices()
{
    std::vector<CameraDeviceInfo> devices;
    std::unique_ptr<webrtc::VideoCaptureModule::DeviceInfo> deviceInfo(
        webrtc::VideoCaptureFactory::CreateDeviceInfo());
    if (!deviceInfo) {
        return devices;
    }
    const std::uint32_t count = deviceInfo->NumberOfDevices();
    devices.reserve(count);
    for (std::uint32_t index = 0; index < count; ++index) {
        std::array<char, kDeviceTextCapacity> name{};
        std::array<char, kDeviceTextCapacity> id{};
        if (deviceInfo->GetDeviceName(
                index, name.data(), static_cast<std::uint32_t>(name.size()),
                id.data(), static_cast<std::uint32_t>(id.size())) != 0 ||
            id.front() == '\0') {
            continue;
        }
        devices.push_back({id.data(), name.data()});
    }
    return devices;
}

bool WindowsCameraCaptureSource::StartCapture(
    const std::string& preferredDeviceId,
    std::chrono::milliseconds firstFrameTimeout)
{
    {
        std::lock_guard lock(mutex_);
        if (running_) {
            return firstFrameReady_;
        }
        lastError_.clear();
        firstFrameReady_ = false;
    }

    const auto devices = EnumerateDevices();
    if (devices.empty()) {
        std::lock_guard lock(mutex_);
        lastError_ = "No Windows camera device is available.";
        return false;
    }
    auto selected = devices.begin();
    if (!preferredDeviceId.empty()) {
        const auto found = std::find_if(
            devices.begin(), devices.end(),
            [&preferredDeviceId](const CameraDeviceInfo& device) {
                return device.id == preferredDeviceId;
            });
        if (found == devices.end()) {
            std::lock_guard lock(mutex_);
            lastError_ =
                "The selected Windows camera is no longer available.";
            return false;
        }
        selected = found;
    }

    auto module = webrtc::VideoCaptureFactory::Create(selected->id.c_str());
    if (!module) {
        std::lock_guard lock(mutex_);
        lastError_ = "WebRTC could not open the selected camera device.";
        return false;
    }

    webrtc::VideoCaptureCapability requested;
    requested.width = 1280;
    requested.height = 720;
    requested.maxFPS = 30;
    webrtc::VideoCaptureCapability selectedCapability = requested;
    std::unique_ptr<webrtc::VideoCaptureModule::DeviceInfo> deviceInfo(
        webrtc::VideoCaptureFactory::CreateDeviceInfo());
    if (deviceInfo) {
        deviceInfo->GetBestMatchedCapability(
            selected->id.c_str(), requested, selectedCapability);
    }

    module->RegisterCaptureDataCallback(this);
    if (module->StartCapture(selectedCapability) != 0) {
        module->DeRegisterCaptureDataCallback();
        std::lock_guard lock(mutex_);
        lastError_ = "The selected camera rejected the capture format.";
        return false;
    }

    {
        std::lock_guard lock(mutex_);
        captureModule_ = module;
        activeDeviceId_ = selected->id;
        activeDeviceName_ = selected->name;
        running_ = true;
    }

    std::unique_lock lock(mutex_);
    const bool received = firstFrameCondition_.wait_for(
        lock, firstFrameTimeout, [this] { return firstFrameReady_; });
    if (received) {
        return true;
    }
    lastError_ = "Timed out waiting for the first camera frame.";
    lock.unlock();
    StopCapture();
    return false;
}

void WindowsCameraCaptureSource::StopCapture()
{
    webrtc::scoped_refptr<webrtc::VideoCaptureModule> module;
    {
        std::lock_guard lock(mutex_);
        module = std::move(captureModule_);
        running_ = false;
        firstFrameReady_ = false;
        activeDeviceId_.clear();
        activeDeviceName_.clear();
    }
    if (module) {
        module->DeRegisterCaptureDataCallback();
        if (module->CaptureStarted()) {
            module->StopCapture();
        }
    }
}

std::string WindowsCameraCaptureSource::ActiveDeviceId() const
{
    std::lock_guard lock(mutex_);
    return activeDeviceId_;
}

std::string WindowsCameraCaptureSource::ActiveDeviceName() const
{
    std::lock_guard lock(mutex_);
    return activeDeviceName_;
}

std::string WindowsCameraCaptureSource::LastError() const
{
    std::lock_guard lock(mutex_);
    return lastError_;
}

webrtc::MediaSourceInterface::SourceState
WindowsCameraCaptureSource::state() const
{
    std::lock_guard lock(mutex_);
    return running_ ? kLive : kEnded;
}

bool WindowsCameraCaptureSource::remote() const
{
    return false;
}

bool WindowsCameraCaptureSource::is_screencast() const
{
    return false;
}

std::optional<bool> WindowsCameraCaptureSource::needs_denoising() const
{
    return true;
}

void WindowsCameraCaptureSource::OnFrame(const webrtc::VideoFrame& frame)
{
    webrtc::AdaptedVideoTrackSource::OnFrame(frame);
    {
        std::lock_guard lock(mutex_);
        firstFrameReady_ = true;
        lastError_.clear();
    }
    firstFrameCondition_.notify_all();
}

}  // namespace remote
