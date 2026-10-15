<h1 align="center">
  <img src="assets/ui/branding/remotec-logo.png" alt="RLink Logo" width="36" height="36" align="absmiddle"> RLink
</h1>

<p align="center"><a href="README.md">简体中文</a> · English</p>

<p align="center">Your computers, connected. Your team, together.</p>

<p align="center">
  Native Windows remote desktop · One-time-code remote assistance · Group calls and screen sharing
</p>

<!-- These buttons link to a specific release. Update both the tag and asset names when publishing a new version; do not combine a latest URL with a version-specific filename. -->
<p align="center">
  <a href="https://github.com/dyhwdnmd/RLink-RemoteCtrl/releases/download/v0.1.13/RLink-Setup-0.1.13.exe"><img src="https://img.shields.io/badge/Windows_x64-Download_installer-2563EB?style=for-the-badge" alt="Download Windows x64 installer"></a>
  <a href="https://github.com/dyhwdnmd/RLink-RemoteCtrl/releases/download/v0.1.13/RLink-Portable-Windows-x64-0.1.13.zip"><img src="https://img.shields.io/badge/Windows_x64-Download_portable-475569?style=for-the-badge" alt="Download Windows x64 portable ZIP"></a>
</p>

<p align="center">
  <a href="#download">Download</a> ·
  <a href="#features">Features</a> ·
  <a href="#screenshots">Screenshots</a> ·
  <a href="#build-from-source">Build from source</a> ·
  <a href="https://github.com/dyhwdnmd/RLink-RemoteCtrl/releases">Releases</a>
</p>

---

RLink is a high-performance, open-source remote desktop and collaboration app for Windows, with low-latency desktop sharing at up to 4K and 120 FPS. Start a remote assistance session with a device ID and a one-time code, or sign in to connect directly to your own online devices. RLink also brings group voice and video calls, screen sharing, and remote control into a single client.

## Download

The download buttons above link to **v0.1.13**. The ready-to-run packages do not require Qt, WebRTC, or a development environment.

| Package | Download | Getting started |
| --- | --- | --- |
| Windows x64 installer (recommended) | [Download installer](https://github.com/dyhwdnmd/RLink-RemoteCtrl/releases/download/v0.1.13/RLink-Setup-0.1.13.exe) | Run the installer; recommended for regular use and future updates |
| Windows x64 portable | [Download ZIP](https://github.com/dyhwdnmd/RLink-RemoteCtrl/releases/download/v0.1.13/RLink-Portable-Windows-x64-0.1.13.zip) | Extract the entire archive, then run `RLinkAPP.exe`; useful for testing or occasional use |

[Latest release and other versions](https://github.com/dyhwdnmd/RLink-RemoteCtrl/releases) ·
[Installer SHA-256](https://github.com/dyhwdnmd/RLink-RemoteCtrl/releases/download/v0.1.13/RLink-Setup-0.1.13.exe.sha256.txt) ·
[Portable ZIP SHA-256](https://github.com/dyhwdnmd/RLink-RemoteCtrl/releases/download/v0.1.13/RLink-Portable-Windows-x64-0.1.13.zip.sha256.txt)

Both packages include the required runtime files. For the portable version, keep the extracted DLLs, plugins, and other files together—do not move the EXE on its own.
`RLink-Windows-x64-0.1.13.zip` is for automatic updates of existing installations, not portable use. The `Source code` downloads contain source archives, and `RLink-update.json` is the client's update manifest; neither is an installer.

### Get started

1. Install or extract RLink on the Windows computers you want to connect, then sign in.
2. Choose the workflow that fits your session:
   - **Remote assistance:** enter the other computer's 9-digit device ID and 6-digit one-time code to start a session.
   - **My devices:** sign in to the same account on both computers, then open a remote desktop from the online device list.
   - **Collaboration rooms:** create or join a room to use voice, video, or screen sharing. Request control from the person sharing their screen when you need to interact with it.

Remote sessions use WebRTC P2P connections. Connectivity depends on the networks at both ends. If a connection cannot be established, check that your firewall allows RLink to access the network, or try another network or a mobile hotspot.
Actual resolution and frame rate depend on the host's capture hardware and encoder, available bandwidth, and the receiving computer's decoding capabilities.

## Features

The list below describes the current source code. For changes included in a particular downloadable build, see its release notes.

### Remote desktop

- [x] AI scene-adaptive streaming
  - Automatically balances image clarity and frame rate during network fluctuations based on on-screen content, such as code, documents, spreadsheets, videos, and games, with smooth transitions between scenes
  - Requires enabling the feature and configuring a model API; the local model is still in development
- [x] View and control another Windows computer
  - Keyboard input, mouse movement, clicks, scrolling, and dragging
  - Remote cursor position and shape synchronization
- [x] High-resolution streaming and selectable frame rates
  - Up to 4K, with a target frame rate of up to 120 FPS
  - 15 / 24 / 30 / 45 / 60 / 80 / 100 / 120 FPS options, subject to capture capabilities and room size
  - 720p / 1080p / 1440p / original-resolution options, subject to the source display's resolution
- [x] Multiple-monitor selection and switching
- [x] Windowed, maximized, and full-screen viewing
- [x] Fit-to-window mode with preserved aspect ratio, plus 100% pixel-scale viewing with panning
- [x] Selectable desktop capture, hardware/software codec, and rendering options

### Connections and device management

- [x] Remote assistance using a device ID and one-time code, with automatic code renewal after a successful assistance session
- [x] Online status and direct, code-free connections to devices on the same account
- [x] Recent devices and rooms, including connection type and last-used time
- [x] Automatic reconnection attempts after a network interruption, with the last frame retained and input paused

### Collaboration, voice, and video

- [x] Create or join collaboration rooms for up to 5 participants
- [x] Group voice calls, video calls, and screen sharing
- [x] Screen-sharing handover and remote control requests
- [x] Approve, decline, or revoke control as the screen owner, with view-only access available
- [x] Camera gallery and separate camera viewing windows
- [x] Camera, microphone, and speaker selection, with on/off controls, muting, and device switching

### File transfer and clipboard

- [x] Transfer files through an established P2P connection, without passing them through the signaling server
  - Accept, save, or decline incoming files
  - Track progress, pause, resume, cancel, and verify file integrity
  - Open the destination folder
- [x] Bidirectional text and file clipboard sharing
- [x] Configurable clipboard content types and per-transfer file size limits
- [x] Clipboard cache location, retention, and capacity settings, plus manual cleanup

### Appearance and everyday use

- [x] Light, dark, and system-following themes
- [x] Animation, font, and text-size settings
- [x] Startup, initial window state, and close-button behavior settings
- [x] Keyboard shortcut reference
- [x] Automatic and manual update checks, with version details, summaries, and release notes
  - CNB as the primary update source, with GitHub fallback and mirror switching if a download or verification fails
  - Required-update prompts for versions below the minimum supported version

### Diagnostics and self-hosting

- [x] Connection, network path, and recovery status
- [x] Send/receive frame rates, bitrate, latency, and device capability diagnostics
- [x] File transfer, clipboard, and input-control status
- [x] Client and C++ signaling server source code for building and self-hosting

### Platform support

| Platform | Status |
| --- | --- |
| Windows x64 | Client available |
| Linux | TODO |
| Android | TODO |
| macOS | TODO |

TODO indicates that a client is not yet available.

## Screenshots

### Collaboration rooms

Create or join a room where participants can request screen access, request control, and take over screen sharing.

![RLink collaboration room](assets/readme/collaboration-room.png)

### Remote assistance

The host provides a 9-digit device ID and a one-time code. Enter both on the other computer to start a remote assistance session.

![RLink remote assistance](assets/readme/remote-assistance.png)

### My devices

Online devices linked to the same RLink account appear automatically. Open a remote desktop directly from the device list.

![RLink device list](assets/readme/my-devices.png)

### Recent connections

Return to recently used devices and rooms, with information about connection type and availability.

![RLink recent connections](assets/readme/recent-connections.png)

### File transfer

Files travel directly over the established P2P connection, not through the signaling server.

<p align="center">
  <img src="assets/readme/file-transfer.png" alt="RLink file transfer window" width="360">
</p>

### Remote desktop settings

Choose a screen capture method, video encoder, hardware encoding backend, and encoding quality.

![RLink remote desktop settings](assets/readme/remote-desktop-settings.png)

## Technology and self-hosting

RLink is built with **C++20, Qt 6, libwebrtc, and FFmpeg**. Qt powers the native desktop interface and WSS signaling client. libwebrtc handles audio/video, DataChannels, congestion control, and connection recovery. Windows capture, encoding, decoding, and rendering use **Direct3D 11, DXGI, and Media Foundation**. Account sign-in and authentication use **Logto**.

The repository includes a signaling server that can be deployed independently. A self-hosted deployment needs a reachable signaling endpoint, TLS certificates, and the appropriate authentication configuration. Production account sign-in requires Logto. Keep secrets, tokens, and local deployment configuration out of the public repository.

## Build from source

Requirements: CMake 3.24+, Visual Studio 2022 with the v143 toolset, the Windows SDK, Qt 6.11.x for MSVC 2022 x64, and WebRTC built with `/MD`.

Set the dependency paths through `RLINK_QT_DIR`, `RLINK_WEBRTC_SRC`, and `RLINK_WEBRTC_OUT`, or copy `cmake\local.bat.example` to `cmake\local.bat` and fill in your local paths. This file is ignored by Git. Do not store signaling tokens, certificate private keys, or other credentials in it.

For a fresh Windows setup, start with [Building RLink on Windows](BUILDING.md). The detailed [Chinese build guide](Windows源码构建指南.md) also covers Qt components, the pinned libwebrtc commit, GN arguments, dependency checks, and common linker errors.

```powershell
.\cmake_configure.bat
cmake --build --preset windows-msvc-x64-v143-release -j
```

Alternatively, use the CMake presets directly: run `cmake --preset windows-msvc-x64-v143`, followed by `cmake --build --preset windows-msvc-x64-v143-release -j`.

To browse or build the project in Visual Studio, configure it first, then open `build\RLinkRemoteCtrl.sln`. `build\RLink.sln` and `ControllerApp.vcxproj` are legacy build artifacts, not the current entry points.

For a Debug build, use `cmake --build --preset windows-msvc-x64-v143-debug -j` after preparing a Debug WebRTC build as described in the build guide.

The main outputs are in `x64\Release`:

- `RLinkAPP.exe`: the RLink client, used for both controlling and sharing a desktop.
- `RLinkUpdater.exe`: a separate updater launched by the client after an update is confirmed.
- `RemoteCSignalServer.exe`: the RLink WSS signaling server.

The public repository includes what is needed to build the source, self-host the signaling service, and build the FFmpeg dependencies. Installer packaging, official release signing, and GitHub/CNB release publishing are handled locally by the maintainer and are not part of the public source-build workflow.

Desktop capture and hardware codecs depend on the active Windows session, displays, and drivers. Before shipping a build, run two-computer smoke tests covering room creation/joining, screen sharing/handover, control and `ReleaseAll`, 30/60/120 FPS, both capture methods, file transfer, cameras/microphones, and ICE restart.

## Repository layout

- Repository root: build entry points (`CMakeLists.txt`, `CMakePresets.json`, and `cmake_configure.bat`) and documentation.
- `cmake`: dependency configuration, target definitions, and local build configuration templates.
- `src`: client, server, and feature modules.
- `assets`: icons, SVGs, themes, fonts, and Windows resources.
- `third_party`: third-party dependencies used to build or run the project.
- `scripts`: helpers for building FFmpeg dependencies and deploying or starting the public signaling service.
- `tools`: source navigation and public-asset maintenance utilities.

## Explore the source

Use the [source navigation index](源码导航/README.md) to find classes, member variables, and functions by file. It covers the C/C++ files under `src`, with clickable source line links and an automatic update script.

## Report an issue

Bug reports and suggestions are welcome on [GitHub Issues](https://github.com/dyhwdnmd/RLink-RemoteCtrl/issues). Include your client version, Windows version, connection type, steps to reproduce, and relevant diagnostics. Before uploading screenshots or logs, redact email addresses, device IDs, one-time codes, access tokens, and other sensitive information.

## License

RLink's own source code is licensed under the [GNU GPL v3.0](LICENSE), with `Copyright (c) 2026 dyhwdnmd`. When modifying or redistributing RLink, provide the corresponding source code and retain the license and copyright notices as required by GPL-3.0.

Third-party components retain their respective licenses; see [Third-party software and licenses](THIRD_PARTY_NOTICES.md). WebRTC uses a BSD 3-Clause-style license. The FFmpeg/libx264 runtime distributed with this project includes GPL components licensed under GPL v2 or later; RLink distributes them under terms compatible with GPL-3.0.
