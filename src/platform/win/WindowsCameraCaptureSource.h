// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#pragma once

#include <chrono>
#include <condition_variable>
#include <mutex>
#include <optional>
#include <string>
#include <vector>

#include "api/video/adapted_video_track_source.h"
#include "api/video/video_sink_interface.h"
#include "modules/video_capture/video_capture.h"

namespace remote {

struct CameraDeviceInfo {
    std::string id;
    std::string name;
};

class WindowsCameraCaptureSource
    : public webrtc::AdaptedVideoTrackSource,
      public webrtc::VideoSinkInterface<webrtc::VideoFrame> {
public:
    WindowsCameraCaptureSource();
    ~WindowsCameraCaptureSource() override;

    static std::vector<CameraDeviceInfo> EnumerateDevices();

    bool StartCapture(
        const std::string& preferredDeviceId = {},
        std::chrono::milliseconds firstFrameTimeout =
            std::chrono::seconds(5));
    void StopCapture();

    std::string ActiveDeviceId() const;
    std::string ActiveDeviceName() const;
    std::string LastError() const;

    SourceState state() const override;
    bool remote() const override;
    bool is_screencast() const override;
    std::optional<bool> needs_denoising() const override;
    void OnFrame(const webrtc::VideoFrame& frame) override;

private:
    mutable std::mutex mutex_;
    std::condition_variable firstFrameCondition_;
    webrtc::scoped_refptr<webrtc::VideoCaptureModule> captureModule_;
    std::string activeDeviceId_;
    std::string activeDeviceName_;
    std::string lastError_;
    bool running_ = false;
    bool firstFrameReady_ = false;
};

}  // namespace remote
