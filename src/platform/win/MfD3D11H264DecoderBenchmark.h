// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace remote {

struct MfD3D11H264DecoderBenchmarkCandidate {
    std::string name;
    bool hardware = true;
    bool passed = false;
    bool asynchronous = false;
    bool nativeD3D11Output = false;
    std::uint32_t decodedFrames = 0;
    double averageLatencyMs = 0.0;
    double p95LatencyMs = 0.0;
    std::uint32_t realtimeDecodedFrames = 0;
    double realtimeAverageLatencyMs = 0.0;
    double realtimeP95LatencyMs = 0.0;
    std::uint32_t sparseDecodedFrames = 0;
    double sparseAverageLatencyMs = 0.0;
    double sparseP95LatencyMs = 0.0;
    std::string error;
};

struct MfD3D11H264DecoderBenchmarkResult {
    bool passed = false;
    std::string bestDecoderName;
    double bestAverageLatencyMs = 0.0;
    double bestP95LatencyMs = 0.0;
    std::string adapterName;
    std::string error;
    std::vector<MfD3D11H264DecoderBenchmarkCandidate> candidates;
};

// Runs a short 1920x1080 H264 decode benchmark with a continuous 60 FPS phase
// followed by a sparse 5 FPS phase. Call this only in the isolated probe
// process; an unstable third-party MFT must never be able to freeze or
// terminate the main RemoteC process.
MfD3D11H264DecoderBenchmarkResult
RunMfD3D11H264DecoderBenchmark();

}  // namespace remote
