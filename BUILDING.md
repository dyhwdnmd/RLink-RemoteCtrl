# Building RLink on Windows

This guide is intended for developers building RLink for the first time on a
clean Windows machine. The repository contains the production source code, the
CMake build system, UI assets, FFmpeg headers, and runtime DLLs. It does not
include the Qt SDK or prebuilt libwebrtc artifacts.

## 1. Build targets

The build system is CMake with a Visual Studio 2022 (v143) generator. A
`Release` build produces the main executables under:

```text
x64\Release\RLinkAPP.exe
x64\Release\RLinkUpdater.exe
x64\Release\RemoteCSignalServer.exe
```

`RLinkAPP.exe` is the shared controller/controlled-side client.
`RLinkUpdater.exe` is the standalone updater used by installed clients.
`RemoteCSignalServer.exe` is the WSS signaling server.

CMake targets map to the sources as follows:

| Target | Kind | Notes |
| --- | --- | --- |
| `rlink_core` | static lib | protocols + core session policy |
| `rlink_auth` | static lib | OIDC/OAuth (Qt NetworkAuthorization) |
| `rlink_signaling` | static lib | Qt WebSocket signaling client |
| `rlink_webrtc_transport` | static lib | libwebrtc session + Windows capture/encode |
| `rlink_session_engine` | static lib | in-process session engine |
| `RLinkAPP` | exe | Qt Widgets client |
| `RemoteCSignalServer` | exe | Qt HTTP/WS signaling server |
| `RLinkUpdater` | exe | standalone updater (no Qt) |

## 2. Prerequisites

Use the versions from the currently verified environment when possible:

| Component | Version or requirement |
| --- | --- |
| Windows | Windows 10/11 x64 |
| CMake | 3.24 or newer |
| Visual Studio | Visual Studio 2022 Build Tools (or IDE) with the v143 toolset, MSVC 14.44.x |
| Windows SDK | 10.0.26100.0 or a compatible Windows 10/11 SDK |
| Qt | Qt 6.11.x, MSVC 2022 64-bit (`msvc2022_64`) |
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

CMake locates Qt through `find_package(Qt6 6.11 REQUIRED ...)`. MOC, UIC, and
RCC are handled by CMake (`AUTOMOC`/`AUTOUIC`/`AUTORCC`), so the Qt Visual
Studio extension is not required.

### Toolchain consistency with WebRTC

WebRTC is compiled separately with GN, and its static libraries are linked into
`RLinkAPP.exe`. The MSVC STL used by WebRTC must match the one used by the CMake
build. If WebRTC is built with a different toolset (for example MSVC 14.51 while
the app uses 14.44), linking fails with unresolved `__std_*` symbols.

Keep both sides on the same toolset. Point depot_tools at your VS2022
installation (Build Tools or IDE — adjust the path for your machine) so that
WebRTC picks the same MSVC, for example `14.44.35207`:

```powershell
$env:DEPOT_TOOLS_WIN_TOOLCHAIN = '0'
$env:GYP_MSVS_OVERRIDE_PATH = 'C:\Program Files\Microsoft Visual Studio\2022\Community'
```

A mismatch here surfaces as unresolved `__std_*` symbols (`LNK2001`), which is
a different failure from the `/MT`-vs-`/MD` RuntimeLibrary mismatch covered in
section 4.

`scripts\Prepare-LibWebRtc.ps1` sets `GYP_MSVS_OVERRIDE_PATH` itself: pass
`-MsvsPath`, or let it use `$env:GYP_MSVS_OVERRIDE_PATH`, or let it fall back to
`vswhere`. The manual export above is only needed when you run `gn gen`/`ninja`
by hand.

### Git configuration for the WebRTC checkout

WebRTC requires exact file contents, so Git must not rewrite line endings while
checking it out. Git for Windows installs with `core.autocrlf=true` at *system*
scope, which is where the surprise comes from. The WebRTC checkout also has deep
paths, so long path support is needed.

`scripts\Prepare-LibWebRtc.ps1` injects these settings for its own process tree
through the `GIT_CONFIG_COUNT` environment mechanism, which leaves your global
git configuration untouched:

```text
core.autocrlf    = false
core.filemode    = false
core.fscache     = true
core.preloadindex = true
core.longpaths   = true
```

A repository `.gitattributes` only governs the files of *that* repository, so
this repository's `.gitattributes` does not cover the WebRTC checkout, depot_tools,
or any DEPS dependency. If you sync WebRTC by hand instead of using the script,
set the values above yourself (for example in a dedicated config file selected
with `GIT_CONFIG_GLOBAL`). Enabling Windows long path support
(`HKLM\SYSTEM\CurrentControlSet\Control\FileSystem\LongPathsEnabled = 1`) also
requires an elevated shell; without it, `core.longpaths=true` is the workaround.

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

Both workflows below produce the same artifacts. Prefer the automatic one; it
runs the manual steps and adds the revision checks described in section 2.

### Automatic: `scripts\Prepare-LibWebRtc.ps1`

The script performs this whole section:

- resolves depot_tools: an existing checkout (`-DepotTools`,
  `RLINK_DEPOT_TOOLS`, or on `PATH`) is reused and its `HEAD` compared against
  the pinned revision, which only warns on a mismatch; when none is found it
  clones the pinned revision,
- bootstraps the depot_tools wrappers when `git.bat` is missing,
- writes or re-pins the `.gclient` solution URL to the pinned WebRTC commit,
- runs `gclient sync -D` in one pass and asserts that the resulting `HEAD`
  equals the pinned commit,
- patches `default_crt` to the dynamic CRT (see below),
- writes both `args.gn` files and builds `out\ReleaseMD` and `out\DebugMD`.

```powershell
scripts\Prepare-LibWebRtc.ps1 `
  -Root D:\dev\webrtc_src `
  -MsvsPath "C:\Program Files\Microsoft Visual Studio\2022\Community"
```

`-Root` is the directory that contains (or will contain) the WebRTC `src`
checkout. It is not committed anywhere: the WebRTC checkout lives outside this
repository and its location is machine-specific, so the script has no default
for it and cannot infer it here — `RLINK_WEBRTC_SRC` is only set later, in
section 5. Pass `-Root`, or set `RLINK_WEBRTC_SRC` first (section 5).

Useful switches:

- `-DepotTools <path>` reuses a specific depot_tools checkout, and
  `-DepotToolsRoot <path>` changes where a fresh clone goes. A fresh clone
  defaults to the sibling of `-Root`, so `-Root E:\webrtc_src` clones into
  `E:\depot_tools`.
- `-Configurations Release` builds only the tree CMake always requires; omit it
  to build `Debug` as well.
- `-SkipBuild` writes `args.gn` without running `gn gen`/`ninja`; `-SkipFetch`
  reuses an existing checkout instead of syncing. With `-SkipFetch`, depot_tools
  must already be resolvable.

### Manual

The same work step by step. `E:\webrtc_src` and `E:\depot_tools` are examples;
substitute your own paths.

**1. Install depot_tools** at the revision pinned in section 2:

```powershell
git clone https://chromium.googlesource.com/chromium/tools/depot_tools.git E:\depot_tools
git -C E:\depot_tools checkout 3799a497b1e483ab3625b91f9540155e8d311985
```

Bootstrap the wrappers once. A fresh checkout does not create `git.bat` until
this runs, and `gclient` fails with a confusing `FileNotFoundError` for
`git.bat` until then:

```powershell
E:\depot_tools\bootstrap\win_tools.bat
```

For the commands below, add `E:\depot_tools` to the front of `PATH` (or call its
scripts by full path). Set `DEPOT_TOOLS_UPDATE=0` so it does not update itself
off the pinned revision.

**2. Set the Git settings WebRTC requires** — `core.autocrlf=false` and
`core.longpaths=true` — as described in section 2, for example for the current
shell only:

```powershell
$env:GIT_CONFIG_COUNT = '2'
$env:GIT_CONFIG_KEY_0   = 'core.autocrlf';  $env:GIT_CONFIG_VALUE_0 = 'false'
$env:GIT_CONFIG_KEY_1   = 'core.longpaths'; $env:GIT_CONFIG_VALUE_1 = 'true'
```

**3. Write the pinned `.gclient`** and sync it in one pass. The revision has to
be part of the solution URL: this `gclient` ignores a separate `"revision"` key,
and a solution URL without a revision tracks `origin/main`, so a bare
`git checkout` is undone by the next `gclient sync`.

```powershell
New-Item -ItemType Directory -Force E:\webrtc_src
Set-Content E:\webrtc_src\.gclient -Encoding ASCII -Value @'
solutions = [
  {
    "name": "src",
    "url": "https://webrtc.googlesource.com/src.git@1e2bd46a33bc0a95ff4e032e380f9fcfa2505808",
    "deps_file": "DEPS",
  },
]
'@
Set-Location E:\webrtc_src
gclient sync -D
```

`gclient sync` clones or checks out `src` at the pinned revision and syncs every
DEPS dependency to that same revision in one pass; no separate `git checkout` is
needed. Verify the result with:

```powershell
git -C E:\webrtc_src\src rev-parse HEAD
# must print 1e2bd46a33bc0a95ff4e032e380f9fcfa2505808
```

Do not start with `fetch --nohooks webrtc`: it writes an unpinned `.gclient`,
does an initial sync at `origin/main`, and refuses to run once a `.gclient`
exists. If you run `fetch` on a directory that already has one, it refuses to
continue ("already contain ... a checkout"); use `gclient sync` there instead.
Pinning a branch head (`refs/branch-heads/...`) instead of a commit additionally
needs `gclient sync --with_branch_heads`; a commit pin does not.

**4. Write `args.gn`, apply the dynamic-CRT edit, then run `gn gen` and
`ninja`** — the three subsections below, in that order, with the v143 toolchain
override from section 2.

#### Release args.gn

Create `E:\webrtc_src\src\out\ReleaseMD\args.gn` with:

```gn
is_debug = false
enable_iterator_debugging = false
target_cpu = "x64"
rtc_include_tests = false
use_custom_libcxx = false
use_lld = false
proprietary_codecs = true
ffmpeg_branding = "Chrome"
```

#### Match the dynamic CRT (`/MD`)

The pinned WebRTC revision has **no GN argument** that selects the dynamic CRT.
Older revisions of this guide mentioned `use_dynamic_crt_for_webrtc = true`, but
that argument is not declared anywhere in this revision, so GN silently ignores
it and the build still uses the **static** CRT. For a non-component desktop
build, `build/config/win/BUILD.gn`'s `default_crt` selects `:static_crt`, and
`is_component_build` cannot be used as a workaround because WebRTC asserts it is
unsupported (`webrtc.gni`). Linking a `/MT` WebRTC into `RLinkAPP` — which, like
Qt, uses `/MD` — fails with `LNK2038`/`LNK1319` RuntimeLibrary mismatches.

Force the dynamic CRT by editing `build/config/win/BUILD.gn` in the WebRTC
checkout so that `config("default_crt")` uses `:dynamic_crt` for desktop
Windows:

```gn
    } else {
      # Desktop Windows: dynamic CRT (/MD; /MDd when is_debug = true) to match
      # the Qt/CMake RLink build.
      configs = [ ":dynamic_crt" ]
    }
```

`scripts\Prepare-LibWebRtc.ps1` applies this edit automatically before
`gn gen`. The edit lives in the WebRTC checkout (outside this repository) and a
`gclient sync` that updates the `build` dependency can overwrite it; re-run the
script (or re-apply the edit) if linking later fails with CRT mismatches again.

#### Generate and build (Release)

Generate and build WebRTC (using the v143 toolchain override from section 2):

```powershell
Set-Location E:\webrtc_src\src
$env:DEPOT_TOOLS_WIN_TOOLCHAIN = '0'
$env:GYP_MSVS_OVERRIDE_PATH = 'C:\Program Files\Microsoft Visual Studio\2022\Community'

gn gen out\ReleaseMD
autoninja -C out\ReleaseMD `
  webrtc `
  builtin_video_decoder_factory `
  builtin_video_encoder_factory `
  api/video:adapted_video_track_source
```

If `autoninja` (siso) stalls, run plain `ninja` instead:

```powershell
third_party\ninja\ninja.exe -C out\ReleaseMD `
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

### Debug WebRTC (required for Debug builds)

A Debug application links a Debug build of WebRTC, so a second GN output tree is
required. It uses the dynamic debug CRT (`/MDd`) and, unlike upstream Chromium's
default, iterator debugging must stay **enabled** so that WebRTC uses
`_ITERATOR_DEBUG_LEVEL=2`. Qt's debug DLLs are built with the MSVC default
(IDL 2) and cannot be changed, so the application and WebRTC must both match
them: `std::string`/`std::wstring` have a different layout at IDL 0 versus 2,
and returning one by value across the Qt DLL boundary corrupts the caller's
stack.

The automatic workflow already covers this: `-Configurations Debug` builds only
this tree, and omitting `-Configurations` builds both. The manual steps follow.

Create `E:\webrtc_src\src\out\DebugMD\args.gn` — same arguments, `is_debug = true`:

```gn
is_debug = true
enable_iterator_debugging = true
target_cpu = "x64"
rtc_include_tests = false
use_custom_libcxx = false
use_lld = false
proprietary_codecs = true
ffmpeg_branding = "Chrome"
```

The same `default_crt` edit from the Release section makes the Debug tree use
`/MDd`; without it, Debug WebRTC is built with `/MTd` and fails identically.

Generate and build it with the same toolchain override:

```powershell
Set-Location E:\webrtc_src\src
$env:DEPOT_TOOLS_WIN_TOOLCHAIN = '0'
$env:GYP_MSVS_OVERRIDE_PATH = 'C:\Program Files\Microsoft Visual Studio\2022\Community'

gn gen out\DebugMD
third_party\ninja\ninja.exe -C out\DebugMD `
  webrtc `
  builtin_video_decoder_factory `
  builtin_video_encoder_factory `
  api/video:adapted_video_track_source
```

The Debug tree is optional: skip it if you only need Release. If you build both
configurations (section 6), keep `out\ReleaseMD` and `out\DebugMD` separate.

## 5. Configure local paths

RLink does not commit machine-specific paths. The CMake build reads these
environment variables:

| Variable | Meaning |
| --- | --- |
| `RLINK_QT_DIR` | Qt 6.11+ `msvc2022_64` kit (must contain `lib\cmake\Qt6`) |
| `RLINK_WEBRTC_SRC` | WebRTC source checkout |
| `RLINK_WEBRTC_OUT` | WebRTC GN output directory for Release (`out\ReleaseMD`) |
| `RLINK_WEBRTC_OUT_DEBUG` | WebRTC GN output directory for Debug (`out\DebugMD`); optional, defaults to `<RLINK_WEBRTC_SRC>\out\DebugMD` |

Set them for the current shell, or persistently for the user:

```powershell
$env:RLINK_QT_DIR           = 'E:\Qt6\6.11.1\msvc2022_64'
$env:RLINK_WEBRTC_SRC       = 'E:\webrtc_src\src'
$env:RLINK_WEBRTC_OUT       = 'E:\webrtc_src\src\out\ReleaseMD'
$env:RLINK_WEBRTC_OUT_DEBUG = 'E:\webrtc_src\src\out\DebugMD'   # Debug builds only
```

On Windows you can instead keep them in a Git-ignored file. Copy the template:

```bat
copy cmake\local.bat.example cmake\local.bat
```

and edit `cmake\local.bat`:

```bat
set "RLINK_QT_DIR=E:\Qt6\6.11.1\msvc2022_64"
set "RLINK_WEBRTC_SRC=E:\webrtc_src\src"
set "RLINK_WEBRTC_OUT=E:\webrtc_src\src\out\ReleaseMD"
set "RLINK_WEBRTC_OUT_DEBUG=E:\webrtc_src\src\out\DebugMD"
```

`cmake_configure.bat` loads `cmake\local.bat` automatically when present.
`cmake\local.bat` is ignored by Git. Do not store Logto secrets, server
certificates, or other credentials in it.

## 6. Configure and build

From the repository root, either run the helper script:

```powershell
.\cmake_configure.bat
cmake --build --preset windows-msvc-x64-v143-release -j
```

or use the CMake preset directly:

```powershell
cmake --preset windows-msvc-x64-v143
cmake --build --preset windows-msvc-x64-v143-release -j
```

`cmake --preset windows-msvc-x64-v143` configures a `Visual Studio 17 2022` x64
project with toolset `v143` into the repository root's `build\` directory (the
generated solution is `build\RLinkRemoteCtrl.sln`). The generator is
multi-configuration, so one configure supports both configurations:

```powershell
cmake --build --preset windows-msvc-x64-v143-release -j   # -> x64\Release
cmake --build --preset windows-msvc-x64-v143-debug   -j   # -> x64\Debug
```

The Debug build requires the Debug WebRTC tree from section 4. Each build copies
Qt, FFmpeg, and platform plugins into `x64\<Config>` (Debug uses
`windeployqt --debug`).

CMake also requires a Release WebRTC tree even when you only build Debug:
`RLINK_WEBRTC_OUT` must be set and contain `obj\webrtc.lib` (see
`cmake\RlinkWebRtc.cmake`). Keep both trees if you work on Debug.

If you change an environment variable and the CMake cache is stale, reconfigure
with `--fresh`:

```powershell
cmake --preset windows-msvc-x64-v143 --fresh
```

Verify the main outputs after a successful build:

```powershell
Test-Path .\x64\Release\RLinkAPP.exe
Test-Path .\x64\Release\RLinkUpdater.exe
Test-Path .\x64\Release\RemoteCSignalServer.exe
Test-Path .\x64\Release\platforms\qwindows.dll
Test-Path .\x64\Release\avcodec-62.dll
Test-Path .\x64\Debug\RLinkAPP.exe
Test-Path .\x64\Debug\RLinkUpdater.exe
Test-Path .\x64\Debug\RemoteCSignalServer.exe
Test-Path .\x64\Debug\platforms\qwindowsd.dll
Test-Path .\x64\Debug\avcodec-62.dll
```

Every command should return `True`. Debug uses the debug-flavoured platform
plugin `qwindowsd.dll`, not `qwindows.dll`.

## 7. Troubleshooting

### CMake reports "Missing dependency locations"

`RLINK_QT_DIR`, `RLINK_WEBRTC_SRC`, or `RLINK_WEBRTC_OUT` is empty. Set the
environment variables (section 5) or create `cmake\local.bat`, then reconfigure.

### Qt6Config.cmake not found

`RLINK_QT_DIR` must point to the `msvc2022_64` kit root, not the Qt installer
root. The directory must contain `lib\cmake\Qt6`, `include`, `lib`, `bin`, and
`plugins`. Qt 6.8 or older is not sufficient: the code uses NetworkAuth APIs
introduced in Qt 6.11.

### WebRTC static libraries were not found

`RLINK_WEBRTC_OUT` (Release) or `RLINK_WEBRTC_OUT_DEBUG` (Debug) must point at
the GN output directory (for example `out\ReleaseMD` / `out\DebugMD`). CMake
looks for `obj\webrtc.lib` inside each. A missing Debug tree only produces a
configure-time warning; the Debug link then fails.

### LNK2038 / LNK1319: RuntimeLibrary mismatch

WebRTC was built with the static CRT (`/MT`/`/MTd`) instead of `/MD`/`/MDd`.
The GN argument some guides mention (`use_dynamic_crt_for_webrtc`) does not
exist in the pinned revision and is ignored — see section 4. Apply the
`default_crt` edit (or run `scripts\Prepare-LibWebRtc.ps1`) and rebuild WebRTC
for the matching configuration.

### LNK2038: _ITERATOR_DEBUG_LEVEL mismatch (Debug)

Qt's debug DLLs use `_ITERATOR_DEBUG_LEVEL=2`, so the Debug application must too
(do not define `_HAS_ITERATOR_DEBUGGING=0`). The Debug WebRTC tree must be built
with `enable_iterator_debugging = true`; otherwise `webrtc.lib` carries
`_ITERATOR_DEBUG_LEVEL=0` and the linker reports `0` vs `2` against the app.

### Debug RLinkAPP reports "Run-Time Check Failure #2"

`/RTC1` traps stack corruption. If the reported variable is a `std::string` /
`std::wstring` at a Qt call boundary (for example `encoded` in
`WriteCurrentUserRegistryString`, from `QString::toStdWString()`), the cause is
an `_ITERATOR_DEBUG_LEVEL` mismatch: rebuild the Debug WebRTC tree with
`enable_iterator_debugging = true` and do not define
`_HAS_ITERATOR_DEBUGGING=0` in the application (see section 4). A genuine code
buffer overflow can also produce this error.

### LNK2001: unresolved `__std_*` symbols when linking RLinkAPP

This is a different failure from the RuntimeLibrary mismatch above: WebRTC and
the application were built with different MSVC STL versions. Regenerate WebRTC
with the same toolset as the app (`GYP_MSVS_OVERRIDE_PATH` pointing at your
VS2022 install, i.e. MSVC 14.44.35207), then rebuild.

### builtin_video_* or adapted_video_track_source is missing

These implementations are not guaranteed to be contained in the aggregate
`webrtc.lib`. Run the complete `autoninja` command from section 4 instead of
building only the `webrtc` target.

### The built application reports missing DLLs

Build the full CMake build (all targets) rather than a single target; the
post-build steps deploy the Qt runtime and plugins and copy the runtime DLLs
from `third_party\ffmpeg_d3d11va\prefix\bin`.

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
