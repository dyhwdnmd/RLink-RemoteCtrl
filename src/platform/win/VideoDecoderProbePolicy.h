// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#pragma once

namespace remote {

// Bump whenever the candidate set or the latency acceptance policy changes.
// Version 6 evaluates the real 1920x1080@60 desktop workload, including its
// sparse-frame phase. A cached 720p result must not select a decoder that
// queues excessively at the application's normal resolution.
inline constexpr int kVideoDecoderProbePolicyVersion = 6;

}  // namespace remote
