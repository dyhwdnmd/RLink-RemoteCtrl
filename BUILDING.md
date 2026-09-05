# Building RLink on Windows

This guide is intended for developers building RLink for the first time on a
clean Windows machine. The repository contains the production source code,
Visual Studio solution, UI assets, FFmpeg headers, and runtime DLLs. It does
not include the Qt SDK or prebuilt libwebrtc artifacts.

## 1. Build targets

The production solution is:

```text
build\RLink.sln
```

A `Release | x64` build produces the main executables under:

```text
x64\Release\RLinkAPP.exe
x64\Release\RLinkUpdater.exe
x64\Release\RemoteCSignalServer.exe
```

`RLinkAPP.exe` is the shared controller/controlled-side client.
`RLinkUpdater.exe` is the standalone updater used by installed clients.
`RemoteCSignalServer.exe` is the WSS signaling server.

## 2. Prerequisites

Use the versions from the currently verified environment when possible:

| Component | Version or requirement |
| --- | --- |
| Windows | Windows 10/11 x64 |
| Visual Studio | Visual Studio 2022 17.14 |
| MSVC | v143 with the Desktop development with C++ workload |
| Windows SDK | 10.0.26100.0 or a compatible Windows 10/11 SDK |
| Qt | Qt 6.11.1, MSVC 2022 64-bit |
| WebRTC | Pinned commit `1e2bd46a33bc0a95ff4e032e380f9fcfa2505808` |
| depot_tools | Verified revision `3799a497b1e483ab3625b91f9540155e8d311985` |

The Qt installation must provide at least these modules:

```text
Qt Core
Qt GUI
Qt Widgets
Qt Network
Qt WebSockets
Qt Network Authorization
Qt HTTP Server
Qt SQL
Qt SVG
```

The project invokes Qt's `rcc.exe` directly to generate the resource source
file. The Qt Visual Studio extension is therefore optional, but the Qt
installation directory must be complete.

## 3. Clone the source code

```powershell
git clone --branch release --single-branch `
  https://github.com/dyhwdnmd/RLink-RemoteCtrl.git
cd .\RLink-RemoteCtrl
```

There are no required Git submodules. The FFmpeg headers, runtime DLLs, and
licenses required by a normal build are already stored under
`third_party\ffmpeg_d3d11va`.

## 4. Prepare libwebrtc

libwebrtc is the largest and most time-consuming external dependency. Do not
use an arbitrary prebuilt version. The WebRTC source, generated headers,
static libraries, compiler ABI, and CRT configuration must match.

Install `depot_tools`, add it to `PATH`, and obtain the WebRTC source through
the official Windows workflow. The following example uses
`E:\webrtc_src\src`:

```powershell
New-Item -ItemType Directory -Force E:\webrtc_src
Set-Location E:\webrtc_src
fetch --nohooks webrtc
Set-Location .\src
git checkout 1e2bd46a33bc0a95ff4e032e380f9fcfa2505808
gclient sync -D
```

Create `E:\webrtc_src\src\out\ReleaseMD\args.gn` with:

```gn
is_debug = false
target_cpu = "x64"
rtc_include_tests = false
use_custom_libcxx = false
use_lld = false
use_dynamic_crt_for_webrtc = true
proprietary_codecs = true
ffmpeg_branding = "Chrome"
```

Do not omit `use_dynamic_crt_for_webrtc = true`. The Qt/MSVC projects use
`/MD`. A WebRTC library built with `/MT` produces an
`LNK2038 RuntimeLibrary` mismatch at link time.

Generate and build WebRTC:

```powershell
Set-Location E:\webrtc_src\src
$env:DEPOT_TOOLS_WIN_TOOLCHAIN = '0'

gn gen out\ReleaseMD
autoninja -C out\ReleaseMD `
  webrtc `
  builtin_video_decoder_factory `
  builtin_video_encoder_factory `
  api/video:adapted_video_track_source
```

Verify that at least the following outputs exist:

```text
out\ReleaseMD\obj\webrtc.lib
out\ReleaseMD\obj\api\video\adapted_video_track_source.lib
out\ReleaseMD\obj\api\video_codecs\builtin_video_decoder_factory.lib
out\ReleaseMD\obj\api\video_codecs\builtin_video_encoder_factory.lib
out\ReleaseMD\obj\api\video_codecs\rtc_software_fallback_wrappers.lib
out\ReleaseMD\obj\media\rtc_internal_video_codecs.lib
out\ReleaseMD\obj\media\rtc_simulcast_encoder_adapter.lib
out\ReleaseMD\gen
```

If you change the WebRTC commit or GN arguments, remove the old output
directory and regenerate it. Never mix headers and libraries generated from
different revisions.

## 5. Configure local paths

RLink does not commit machine-specific paths or credentials. Copy the local
path template:

```powershell
Copy-Item .\build\LocalBuild.props.example `
  .\build\LocalBuild.props
```

Edit `build\LocalBuild.props`, for example:

```xml
<?xml version="1.0" encoding="utf-8"?>
<Project ToolsVersion="Current"
         xmlns="http://schemas.microsoft.com/developer/msbuild/2003">
  <PropertyGroup>
    <WebRtcSourceDir>E:\webrtc_src\src</WebRtcSourceDir>
    <WebRtcOutputDir>$(WebRtcSourceDir)\out\ReleaseMD</WebRtcOutputDir>
    <QtInstallDir>E:\Qt6\6.11.1\msvc2022_64</QtInstallDir>
  </PropertyGroup>
</Project>
```

Adjust all three directories for your machine. `LocalBuild.props` is ignored
by Git. Do not store Logto secrets, server certificates, or other credentials
in it.

You can check the critical inputs before building:

```powershell
$required = @(
  'E:\Qt6\6.11.1\msvc2022_64\bin\rcc.exe',
  'E:\webrtc_src\src\out\ReleaseMD\obj\webrtc.lib',
  '.\third_party\ffmpeg_d3d11va\prefix\bin\avcodec-62.dll',
  '.\third_party\ffmpeg_d3d11va\prefix\bin\avutil-60.dll'
)
$required | ForEach-Object {
  if (-not (Test-Path -LiteralPath $_)) {
    Write-Error "Missing build dependency: $_"
  }
}
```

## 6. Build the production solution

Open **Developer PowerShell for VS 2022** or
**x64 Native Tools Command Prompt for VS 2022**, then run this command from
the repository root:

```powershell
MSBuild.exe .\build\RLink.sln /t:Build `
  /p:Configuration=Release /p:Platform=x64 /m
```

If `MSBuild.exe` is not available in `PATH`, launch one of the Visual Studio
developer shells above or use the actual `MSBuild.exe` path on your machine.
You can also open `build\RLink.sln` in Visual Studio, select `Release | x64`,
and build the solution.

The build copies Qt, FFmpeg, and platform plugins into `x64\Release`. Verify
the main outputs after a successful build:

```powershell
Test-Path .\x64\Release\RLinkAPP.exe
Test-Path .\x64\Release\RLinkUpdater.exe
Test-Path .\x64\Release\RemoteCSignalServer.exe
Test-Path .\x64\Release\platforms\qwindows.dll
Test-Path .\x64\Release\avcodec-62.dll
```

All five commands should return `True`.

## 7. Troubleshooting

### Qt headers or Qt6*.lib cannot be found

Make sure `QtInstallDir` points to the `msvc2022_64` root, not the Qt installer
root. The directory must contain `include`, `lib`, `bin`, and `plugins`.

### LNK1104: cannot open webrtc.lib

Check `WebRtcOutputDir` and confirm that `obj\webrtc.lib` exists. Do not point
the property to `out\ReleaseMD\obj`; the project appends `obj` itself.

### LNK2038: RuntimeLibrary mismatch

The project uses `/MD`. Regenerate WebRTC and make sure the GN arguments
contain:

```gn
use_dynamic_crt_for_webrtc = true
```

### builtin_video_* or adapted_video_track_source is missing

These implementations are not guaranteed to be contained in the aggregate
`webrtc.lib`. Run the complete `autoninja` command from section 4 instead of
building only the `webrtc` target.

### The built application reports missing DLLs

Build the complete solution and do not copy the EXE alone. The application
also needs the Qt DLLs, Qt plugins, and runtime DLLs from
`third_party\ffmpeg_d3d11va\prefix\bin`.

## 8. Building versus connecting to deployed services

The steps above produce the client and signaling-server executables. Actual
login, WSS signaling, and remote-control sessions also require deployment-side
Logto configuration, TLS certificates, server secrets, and a reachable
signaling endpoint. These runtime credentials are intentionally excluded from
the source repository. Review `scripts\New-PublicSignalingDeployment.ps1`,
`scripts\Set-LogtoM2MSecret.ps1`, and
`scripts\Start-PublicSignalingServer.ps1` before deploying the signaling
server, and never commit production secrets.

After changing dependencies or the toolchain, perform at least one two-machine
smoke test covering login, room creation/joining, verification-code sessions,
My Devices sessions, screen sharing, keyboard and mouse control, clipboard,
and file transfer.
