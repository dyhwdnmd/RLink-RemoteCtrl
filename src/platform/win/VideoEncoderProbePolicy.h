// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#pragma once

namespace remote {

// Bump whenever the candidate set, the synthetic desktop workload, the
// acceptance limits, or the ranking policy changes. Cached results are never
// reused across policy versions.
// Version 10 treats any candidate that produces a decodable H264 stream as
// usable, ranks usable candidates by measured output FPS and then cost, and
// never rejects a useful encoder merely because it cannot sustain 60 FPS.
inline constexpr int kVideoEncoderProbePolicyVersion = 10;

}  // namespace remote
