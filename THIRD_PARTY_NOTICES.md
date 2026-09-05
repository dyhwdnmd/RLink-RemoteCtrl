# RLink third-party software and licenses

RLink's GNU GPL v3.0 license covers RLink's own source code. Third-party
software remains copyrighted by its respective authors and is provided under
the licenses named below. This notice is a practical inventory, not a
replacement for the complete license texts distributed by each project.

## WebRTC

- Project: The WebRTC project
- Source used by this repository: commit
  `1e2bd46a33bc0a95ff4e032e380f9fcfa2505808`
- License: BSD 3-Clause style license
- License: <https://webrtc.googlesource.com/src/+/refs/heads/main/LICENSE>
- Patent grant: <https://webrtc.googlesource.com/src/+/refs/heads/main/PATENTS>

WebRTC and its bundled third-party components are not relicensed as GPL.
Their copyright, license, patent and third-party notice files must accompany
binary distributions as required by the pinned WebRTC checkout.

## FFmpeg and x264

- FFmpeg: <https://ffmpeg.org/>
- FFmpeg license information: <https://ffmpeg.org/legal.html>
- FFmpeg revision used by the bundled DLLs: `ad41607`
- x264: <https://www.videolan.org/developers/x264.html>
- x264 revision used by `libx264-165.dll`: `570f6c7`
- Effective license for the bundled combination: GNU GPL version 2 or later

The runtime in `third_party/ffmpeg_d3d11va` is built with
`--enable-gpl --enable-libx264`. Enabling libx264 changes the resulting FFmpeg
build from its default LGPL terms to GPL v2 or later. RLink uses the GPL v3
option for this combined distribution. The corresponding GPL v3 text is in
`third_party/ffmpeg_d3d11va/COPYING.GPLv3`.

The exact FFmpeg build switches are recorded in
`scripts/Build-FfmpegD3D11Va.ps1`. Distributors must make the complete
corresponding source for the exact FFmpeg and x264 binaries they ship
available under the applicable GPL terms; distributing only the DLL files and
headers is not a substitute for that obligation. A public binary release
should therefore attach the matching FFmpeg and x264 source archives, build
instructions, and applicable license files alongside the RLink installer.

## Other dependencies

Qt, oneVPL, NV Codec SDK headers, AMD AMF headers, Morphicons and other
dependencies retain their own licenses. Before publishing a binary release,
include the license and notice files from the exact dependency versions used
to build that release.
