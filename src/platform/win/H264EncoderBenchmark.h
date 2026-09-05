// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "src/webrtc/VideoEncoderRuntimeStatus.h"

namespace remote {

enum class H264EncoderBenchmarkInput {
    kCpuBgra,
    kD3D11Bgra,
};

inline constexpr char kAutomaticEncoderMf[] = "mf_d3d11";
inline constexpr char kAutomaticEncoderOpenH264[] = "openh264";
inline constexpr char kAutomaticEncoderFfmpegX264[] = "ffmpeg_x264";
inline constexpr char kAutomaticEncoderFfmpegQsv[] = "ffmpeg_qsv";
inline constexpr char kAutomaticEncoderFfmpegNvenc[] = "ffmpeg_nvenc";
inline constexpr char kAutomaticEncoderFfmpegAmf[] = "ffmpeg_amf";
inline constexpr char kAutomaticEncoderFfmpegHardwareDefault[] =
    "ffmpeg_hardware_default";

struct H264EncoderBenchmarkCandidate {
    std::string id;
    std::string name;
    std::string inputPath;
    bool hardware = false;
    bool passed = false;
    std::uint32_t submittedFrames = 0;
    std::uint32_t encodedFrames = 0;
    std::uint32_t keyFrames = 0;
    std::uint64_t encodedBytes = 0;
    double averageLatencyMs = 0.0;
    double p95LatencyMs = 0.0;
    double cpuTimePerFrameMs = 0.0;
    double inputFramesPerSecond = 0.0;
    double averageLumaPsnrDb = 0.0;
    double score = 0.0;
    bool dynamicRateControlTested = false;
    bool dynamicRateControlPassed = false;
    std::string warning;
    std::string error;
};

struct H264EncoderBenchmarkResult {
    bool passed = false;
    std::string bestEncoderId;
    std::string bestEncoderName;
    H264EncoderBenchmarkInput input =
        H264EncoderBenchmarkInput::kCpuBgra;
    std::string error;
    std::vector<H264EncoderBenchmarkCandidate> candidates;
};

// Executes an end-to-end 1920x1080@60 moving-desktop workload. kCpuBgra models
// libwebrtc DesktopCapturer output; kD3D11Bgra models the native DXGI texture
// path, including any conversion/readback/upload performed by the encoder.
// Run only in the isolated probe process.
H264EncoderBenchmarkResult RunH264EncoderBenchmark(
    H264EncoderBenchmarkInput input,
    FfmpegX264Preset ffmpegX264Preset);

}  // namespace remote
