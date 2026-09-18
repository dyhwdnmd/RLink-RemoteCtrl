# RLink Windows 源码构建指南

本文面向第一次在全新 Windows 电脑上编译 RLink 的开发者。仓库包含
RLink 的正式源码、CMake 构建系统、界面资源以及运行时使用的 FFmpeg DLL，
但不包含 Qt SDK 和 libwebrtc 的预编译产物。

## 1. 构建目标

构建系统为 CMake + Visual Studio 2022（v143）生成器。`Release` 构建后，
主要产物位于：

```text
x64\Release\RLinkAPP.exe
x64\Release\RLinkUpdater.exe
x64\Release\RemoteCSignalServer.exe
```

`RLinkAPP.exe` 是控制端与被控端共用的客户端，
`RLinkUpdater.exe` 是客户端确认更新后使用的独立更新程序，
`RemoteCSignalServer.exe` 是 WSS 信令服务。

CMake 目标与源码的对应关系：

| 目标 | 类型 | 说明 |
| --- | --- | --- |
| `rlink_core` | 静态库 | 协议 + 会话核心策略 |
| `rlink_auth` | 静态库 | OIDC/OAuth（Qt NetworkAuthorization） |
| `rlink_signaling` | 静态库 | Qt WebSocket 信令客户端 |
| `rlink_webrtc_transport` | 静态库 | libwebrtc 会话 + Windows 采集/编码 |
| `rlink_session_engine` | 静态库 | 进程内会话引擎 |
| `RLinkAPP` | 可执行 | Qt Widgets 客户端 |
| `RemoteCSignalServer` | 可执行 | Qt HTTP/WS 信令服务 |
| `RLinkUpdater` | 可执行 | 独立更新器（不依赖 Qt） |

## 2. 所需环境

建议使用与当前已验证环境一致的版本：

| 组件 | 版本或要求 |
| --- | --- |
| Windows | Windows 10/11 x64 |
| CMake | 3.24 或更高 |
| Visual Studio | Visual Studio 2022 生成工具（或 IDE），含 v143 工具集、MSVC 14.44.x |
| Windows SDK | 10.0.26100.0，或兼容的 Windows 10/11 SDK |
| Qt | Qt 6.11.x，MSVC 2022 64-bit（`msvc2022_64`） |
| WebRTC | 固定 commit `1e2bd46a33bc0a95ff4e032e380f9fcfa2505808` |
| depot_tools | 已验证 revision `3799a497b1e483ab3625b91f9540155e8d311985` |

Qt 安装至少需要能够提供以下模块：

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

CMake 通过 `find_package(Qt6 6.11 REQUIRED ...)` 查找 Qt，MOC/UIC/RCC 由
CMake 的 `AUTOMOC`/`AUTOUIC`/`AUTORCC` 处理，因此不依赖 Visual Studio 的
Qt 扩展。

### 与 WebRTC 的工具链一致性

WebRTC 由 GN 单独编译，其静态库会被链接进 `RLinkAPP.exe`。WebRTC 使用的
MSVC STL 必须与 CMake 构建一致：若 WebRTC 用 14.51、主程序用 14.44 之类的
不同工具集，链接时会出现无法解析的 `__std_*` 符号（`LNK2001`）。这与第 4 节
要处理的 `/MT` 与 `/MD` 不匹配是两类不同的问题。

让两侧保持同一工具集。生成 WebRTC 时把 depot_tools 指向你本机的 VS2022
安装（生成工具或 IDE，按实际路径调整），使其选用同一 MSVC，例如
`14.44.35207`：

```powershell
$env:DEPOT_TOOLS_WIN_TOOLCHAIN = '0'
$env:GYP_MSVS_OVERRIDE_PATH = 'C:\Program Files\Microsoft Visual Studio\2022\Community'
```

`scripts\Prepare-LibWebRtc.ps1` 会自行设置 `GYP_MSVS_OVERRIDE_PATH`：可用
`-MsvsPath` 传入，或读取 `$env:GYP_MSVS_OVERRIDE_PATH`，或通过 `vswhere`
自动探测。只有手工执行 `gn gen`/`ninja` 时才需要按上面的方式自行导出。

### WebRTC 检出的 Git 配置

WebRTC 需要逐字节精确的文件内容，检出过程中 Git 不能改写行尾。Git for
Windows 在**系统级**默认 `core.autocrlf=true`，这正是问题的来源。WebRTC
的目录层级也很深，因此还需要长路径支持。

`scripts\Prepare-LibWebRtc.ps1` 通过 `GIT_CONFIG_COUNT` 环境变量只对自身
进程树注入以下设置，不改动你的全局 Git 配置：

```text
core.autocrlf    = false
core.filemode    = false
core.fscache     = true
core.preloadindex = true
core.longpaths   = true
```

仓库里的 `.gitattributes` 只约束**该仓库自身**跟踪的文件，覆盖不到 WebRTC
检出、depot_tools 或任何 DEPS 子仓库。若你手工同步 WebRTC，请自行设置上述
项（例如用 `GIT_CONFIG_GLOBAL` 指向一份专用配置文件）。开启 Windows 长路径
（`HKLM\SYSTEM\CurrentControlSet\Control\FileSystem\LongPathsEnabled = 1`）
需要管理员权限；未开启时 `core.longpaths=true` 可作为替代。

## 3. 克隆源码

```powershell
git clone --branch release --single-branch `
  https://github.com/dyhwdnmd/RLink-RemoteCtrl.git
cd .\RLink-RemoteCtrl
```

仓库没有必须初始化的 Git submodule。正常编译所需的 FFmpeg 头文件、
运行时 DLL 和许可证已包含在 `third_party\ffmpeg_d3d11va` 中。

## 4. 准备 libwebrtc

libwebrtc 是整个构建中体积最大、耗时最长的外部依赖。不要使用任意版本
的预编译库；WebRTC 的源码、生成头文件、静态库、编译器 ABI 和 CRT
配置必须互相匹配。

### 自动方式：`scripts\Prepare-LibWebRtc.ps1`

脚本会完成本节全部工作：

- 解析 depot_tools：已有的检出（`-DepotTools`、`RLINK_DEPOT_TOOLS` 或 `PATH`
  上）直接复用，并把它的 `HEAD` 与固定 revision 比对（不一致只告警）；
  完全找不到时才克隆固定 revision；
- 在 `git.bat` 缺失时引导 depot_tools 包装脚本；
- 写入或改写 `.gclient` 的 solution url 以固定 WebRTC commit；
- 一遍执行 `gclient sync -D`，并断言同步后的 `HEAD` 等于固定 commit；
- 把 `default_crt` 修补为动态 CRT（见下文）；
- 写入两个 `args.gn`，并编译 `out\ReleaseMD` 与 `out\DebugMD`。

```powershell
scripts\Prepare-LibWebRtc.ps1 `
  -Root D:\dev\webrtc_src `
  -MsvsPath "C:\Program Files\Microsoft Visual Studio\2022\Community"
```

`-Root` 是包含（或将要包含）WebRTC `src` 检出的目录。它没有提交到任何地方：
WebRTC 检出位于本仓库之外、路径因机器而异，脚本对它没有默认值，在此也无法
推断——`RLINK_WEBRTC_SRC` 要到第 5 节才设置。因此需要显式传 `-Root`，或先
按第 5 节设置 `RLINK_WEBRTC_SRC`。

常用开关：

- `-DepotTools <路径>` 复用指定的 depot_tools 检出；`-DepotToolsRoot <路径>`
  改变新建克隆的落点。新克隆默认落在 `-Root` 的同级目录，因此
  `-Root E:\webrtc_src` 会克隆到 `E:\depot_tools`。
- `-Configurations Release` 只构建 CMake 始终需要的 Release 树；不加则同时
  构建 Debug 树。
- `-SkipBuild` 只写 `args.gn`、不执行 `gn gen`/`ninja`；`-SkipFetch` 复用已有
  检出而不做同步。使用 `-SkipFetch` 时 depot_tools 必须已经可以解析到。

### 手工方式

逐步完成同样的工作。`E:\webrtc_src` 与 `E:\depot_tools` 只是示例，请替换为
你自己的路径。

**1. 安装 depot_tools** 并固定到第 2 节的 revision：

```powershell
git clone https://chromium.googlesource.com/chromium/tools/depot_tools.git E:\depot_tools
git -C E:\depot_tools checkout 3799a497b1e483ab3625b91f9540155e8d311985
```

先引导一次包装脚本。全新克隆在引导完成前不会生成 `git.bat`，此时 `gclient`
会抛出令人困惑的 `git.bat` `FileNotFoundError`：

```powershell
E:\depot_tools\bootstrap\win_tools.bat
```

下面的命令需要把 `E:\depot_tools` 放到 `PATH` 最前面（或直接写全路径调用），
并设置 `DEPOT_TOOLS_UPDATE=0`，避免它自更新后偏离固定 revision。

**2. 设置 WebRTC 需要的 Git 配置**——`core.autocrlf=false` 与
`core.longpaths=true`——见第 2 节；例如只对当前 shell 生效：

```powershell
$env:GIT_CONFIG_COUNT = '2'
$env:GIT_CONFIG_KEY_0   = 'core.autocrlf';  $env:GIT_CONFIG_VALUE_0 = 'false'
$env:GIT_CONFIG_KEY_1   = 'core.longpaths'; $env:GIT_CONFIG_VALUE_1 = 'true'
```

**3. 写入固定的 `.gclient`** 并一遍完成同步。revision 必须写入 solution url：
这一版 gclient 会忽略单独的 `"revision"` 键，而 solution url 不带 revision 时
会跟踪 `origin/main`，下一次 `gclient sync` 就会把手工 `git checkout` 覆盖掉。

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

`gclient sync` 会把 `src` 拉取/检出到固定 revision，并按该 revision 的 DEPS
一次性同步全部依赖，不需要再单独 `git checkout`。用下面的命令校验结果：

```powershell
git -C E:\webrtc_src\src rev-parse HEAD
# 必须输出 1e2bd46a33bc0a95ff4e032e380f9fcfa2505808
```

不要用 `fetch --nohooks webrtc` 起步：它会写出不带 revision 的 `.gclient`、
先按 `origin/main` 同步一遍，而且在 `.gclient` 已存在时会拒绝运行；此时应直接
执行 `gclient sync`。如果要固定的是分支头（`refs/branch-heads/...`）而不是
提交，需要额外加 `gclient sync --with_branch_heads`；固定提交则不需要。

手工操作时需要注意 depot_tools 的两个坑：

- 全新克隆的 depot_tools 在引导完成前不会生成 `git.bat`，此时 `gclient` 会
  抛出令人困惑的 `git.bat` `FileNotFoundError`。先运行一次
  `<depot_tools>\bootstrap\win_tools.bat`。
- 若在已存在 `.gclient` 的目录改用 `fetch`（而非上面的流程），它会拒绝继续
  （提示 “already contain ... a checkout”）。此时应直接执行 `gclient sync`；
  `fetch` 只是「写 `.gclient` + 跑一次 `gclient sync`」的包装。

**4. 写入 `args.gn`、应用动态 CRT 修补、然后运行 `gn gen`/`ninja`**——即下面
三节，按此顺序，并使用第 2 节的 v143 工具链覆盖。

#### Release 的 args.gn

在 `E:\webrtc_src\src\out\ReleaseMD\args.gn` 写入：

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

#### 选择动态 CRT（`/MD`）

这个固定的 WebRTC revision **没有任何 GN 参数**可以选择动态 CRT。旧版本文档
提到的 `use_dynamic_crt_for_webrtc = true` 在该 revision 中根本未声明，GN 会
静默忽略它，构建出的仍是**静态** CRT。对非 component 的桌面构建，
`build/config/win/BUILD.gn` 的 `default_crt` 选择 `:static_crt`；也不能改用
`is_component_build`，因为 `webrtc.gni` 明确断言不支持 component build。把
`/MT` 的 WebRTC 链接进使用 `/MD` 的 `RLinkAPP`（与 Qt 一致）时，会出现
`LNK2038`/`LNK1319` RuntimeLibrary 不匹配。

因此需要修改 WebRTC 检出中的 `build/config/win/BUILD.gn`，让
`config("default_crt")` 在桌面 Windows 下使用 `:dynamic_crt`：

```gn
    } else {
      # Desktop Windows: dynamic CRT (/MD; /MDd when is_debug = true) to match
      # the Qt/CMake RLink build.
      configs = [ ":dynamic_crt" ]
    }
```

`scripts\Prepare-LibWebRtc.ps1` 会在 `gn gen` 之前自动应用该修补。修补位于
WebRTC 检出（本仓库之外），若某次 `gclient sync` 更新了 `build` 依赖就会
被覆盖；链接再次出现 CRT 不匹配时，重跑脚本或重新应用即可。

#### 生成并编译（Release）

生成并编译 WebRTC（同时应用第 2 节的 v143 工具链覆盖）：

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

如果 `autoninja`（siso）卡住，可改用普通 ninja：

```powershell
third_party\ninja\ninja.exe -C out\ReleaseMD `
  webrtc `
  builtin_video_decoder_factory `
  builtin_video_encoder_factory `
  api/video:adapted_video_track_source
```

编译完成后，至少确认以下文件存在：

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

如果修改了 WebRTC commit 或 GN 参数，应删除旧输出目录后重新生成，不能
把不同版本产生的头文件与 `.lib` 混用。

### Debug 版 WebRTC（Debug 构建必需）

Debug 主程序要链接 Debug 版 WebRTC，因此需要第二份 GN 输出目录。它使用
动态调试 CRT（`/MDd`）；与上游 Chromium 的默认不同，这里必须**保持开启**
迭代器调试，使 WebRTC 使用 `_ITERATOR_DEBUG_LEVEL=2`。Qt 的调试运行库用的是
MSVC 默认的 IDL 2 且不可更改，所以主程序与 WebRTC 都必须对齐到 2：
`std::string`/`std::wstring` 在 IDL 0 与 2 下的布局不同，按值跨 Qt DLL 边界
返回时会把调用方的栈写坏。

自动方式已经覆盖这份输出：加 `-Configurations Debug` 只构建它，不加则两套
都构建。手工步骤如下。

创建 `E:\webrtc_src\src\out\DebugMD\args.gn`（参数相同，`is_debug = true`）：

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

Release 节中的同一处 `default_crt` 修补会让 Debug 树使用 `/MDd`；否则 Debug
版 WebRTC 是 `/MTd`，会以完全相同的方式链接失败。

用相同的工具链覆盖生成并编译：

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

Debug 输出目录是可选的：只做 Release 构建时无需生成。若同时构建两种配置
（见第 6 节），请保持 `out\ReleaseMD` 与 `out\DebugMD` 相互独立。

## 5. 配置本机路径

RLink 不会把本机路径提交到 Git。CMake 构建读取以下环境变量：

| 变量 | 含义 |
| --- | --- |
| `RLINK_QT_DIR` | Qt 6.11+ `msvc2022_64` 套件（需含 `lib\cmake\Qt6`） |
| `RLINK_WEBRTC_SRC` | WebRTC 源码目录 |
| `RLINK_WEBRTC_OUT` | Release 用的 GN 输出目录（`out\ReleaseMD`） |
| `RLINK_WEBRTC_OUT_DEBUG` | Debug 用的 GN 输出目录（`out\DebugMD`）；可选，缺省为 `<RLINK_WEBRTC_SRC>\out\DebugMD` |

在当前终端临时设置，或设为用户级持久变量：

```powershell
$env:RLINK_QT_DIR           = 'E:\Qt6\6.11.1\msvc2022_64'
$env:RLINK_WEBRTC_SRC       = 'E:\webrtc_src\src'
$env:RLINK_WEBRTC_OUT       = 'E:\webrtc_src\src\out\ReleaseMD'
$env:RLINK_WEBRTC_OUT_DEBUG = 'E:\webrtc_src\src\out\DebugMD'   # 仅 Debug 构建需要
```

在 Windows 上也可以放进一个被 Git 忽略的本地文件。复制模板：

```bat
copy cmake\local.bat.example cmake\local.bat
```

然后编辑 `cmake\local.bat`：

```bat
set "RLINK_QT_DIR=E:\Qt6\6.11.1\msvc2022_64"
set "RLINK_WEBRTC_SRC=E:\webrtc_src\src"
set "RLINK_WEBRTC_OUT=E:\webrtc_src\src\out\ReleaseMD"
set "RLINK_WEBRTC_OUT_DEBUG=E:\webrtc_src\src\out\DebugMD"
```

`cmake_configure.bat` 会在该文件存在时自动加载它。`cmake\local.bat` 已被
`.gitignore` 排除，不要在其中写入 Logto 密钥、服务器证书或其他凭证。

## 6. 配置与编译

在仓库根目录，可以运行辅助脚本：

```powershell
.\cmake_configure.bat
cmake --build --preset windows-msvc-x64-v143-release -j
```

也可以直接使用 CMake 预设：

```powershell
cmake --preset windows-msvc-x64-v143
cmake --build --preset windows-msvc-x64-v143-release -j
```

`cmake --preset windows-msvc-x64-v143` 会在仓库根目录的 `build\` 下生成
`Visual Studio 17 2022`、x64、工具集 `v143` 的工程（解决方案文件为
`build\RLinkRemoteCtrl.sln`）。该生成器是多配置的，一次 configure 即可构建两种
配置：

```powershell
cmake --build --preset windows-msvc-x64-v143-release -j   # -> x64\Release
cmake --build --preset windows-msvc-x64-v143-debug   -j   # -> x64\Debug
```

Debug 构建需要第 4 节的 Debug 版 WebRTC。两种配置都会把 Qt、FFmpeg 和平台
插件复制到 `x64\<Config>`（Debug 使用 `windeployqt --debug`）。

即使只构建 Debug，CMake 也要求存在 Release 版 WebRTC 树：`RLINK_WEBRTC_OUT`
必须已设置且其中含 `obj\webrtc.lib`（见 `cmake\RlinkWebRtc.cmake`）。做 Debug
时请同时保留两棵输出树。

如果修改了环境变量而 CMake 缓存仍是旧值，用 `--fresh` 重新配置：

```powershell
cmake --preset windows-msvc-x64-v143 --fresh
```

成功后检查：

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

全部返回 `True` 表示主要程序和运行时文件已经生成。Debug 的平台插件是带调试
后缀的 `qwindowsd.dll`，不是 `qwindows.dll`。

## 7. 常见问题

### CMake 报 “Missing dependency locations”

`RLINK_QT_DIR`、`RLINK_WEBRTC_SRC` 或 `RLINK_WEBRTC_OUT` 为空。按第 5 节设置
环境变量或创建 `cmake\local.bat` 后重新配置。

### 找不到 Qt6Config.cmake

`RLINK_QT_DIR` 必须指向 `msvc2022_64` 根目录，而不是 Qt 安装器根目录。该
目录下必须存在 `lib\cmake\Qt6`、`include`、`lib`、`bin` 和 `plugins`。
Qt 6.8 及更早版本不满足要求：代码使用了 Qt 6.11 引入的 NetworkAuth API。

### WebRTC static libraries were not found

`RLINK_WEBRTC_OUT`（Release）或 `RLINK_WEBRTC_OUT_DEBUG`（Debug）必须指向
相应的 GN 输出目录（`out\ReleaseMD` / `out\DebugMD`），CMake 会在其下查找
`obj\webrtc.lib`。缺少 Debug 目录在 configure 阶段只是警告，Debug 链接时才会失败。

### LNK2038 / LNK1319：RuntimeLibrary 不匹配

WebRTC 使用了静态 CRT（`/MT`/`/MTd`）而非 `/MD`/`/MDd`。部分文档提到的 GN
参数 `use_dynamic_crt_for_webrtc` 在该固定 revision 中并不存在、会被忽略，
详见第 4 节。应用 `default_crt` 修补（或运行
`scripts\Prepare-LibWebRtc.ps1`），再为对应配置重新编译 WebRTC。

### LNK2038：`_ITERATOR_DEBUG_LEVEL` 不匹配（Debug）

Qt 的调试运行库使用 `_ITERATOR_DEBUG_LEVEL=2`，因此 Debug 主程序也必须使用 2
（不要定义 `_HAS_ITERATOR_DEBUGGING=0`）。Debug 版 WebRTC 必须用
`enable_iterator_debugging = true` 构建，否则 `webrtc.lib` 带的是
`_ITERATOR_DEBUG_LEVEL=0`，链接时会报 `0` 与 `2` 不匹配。

### Debug 版 RLinkAPP 报 “Run-Time Check Failure #2”

`/RTC1` 会捕获栈破坏。若被点名的变量是处在 Qt 调用边界上的 `std::string` /
`std::wstring`（例如 `WriteCurrentUserRegistryString` 里由
`QString::toStdWString()` 得到的 `encoded`），根因就是
`_ITERATOR_DEBUG_LEVEL` 不一致：用 `enable_iterator_debugging = true` 重建
Debug 版 WebRTC，并且不要在应用中定义 `_HAS_ITERATOR_DEBUGGING=0`（见第 4 节）。
当然，真实的代码缓冲区越界也会产生该错误。

### 链接 RLinkAPP 时出现无法解析的 `__std_*` 符号（LNK2001）

这与上面的 RuntimeLibrary 不匹配是两类不同的问题：WebRTC 与主程序使用了
不同的 MSVC STL 版本。用与主程序相同的工具集重新生成 WebRTC
（`GYP_MSVS_OVERRIDE_PATH` 指向你本机的 VS2022 安装，即 MSVC 14.44.35207），
然后重新构建。

### 找不到 builtin_video_* 或 adapted_video_track_source

这些实现不保证全部包含在聚合 `webrtc.lib` 中。重新执行第 4 节列出的
`autoninja` 命令，不要只构建单个 `webrtc` 目标。

### 构建后程序启动时提示缺少 DLL

请构建完整的 CMake 工程（所有目标），不要只构建单个目标；构建后的部署步骤
会复制 Qt 运行时与插件，并拷贝
`third_party\ffmpeg_d3d11va\prefix\bin` 中的运行时 DLL。

## 8. 编译成功与可连接运行的区别

完成以上步骤即可生成客户端和信令服务，但实际登录、WSS 信令和远程连接
还依赖部署环境中的 Logto 配置、TLS 证书、服务端密钥和可访问的信令地址。
这些运行凭证不会提交到源码仓库。部署信令服务前请检查
`scripts\New-PublicSignalingDeployment.ps1`、
`scripts\Set-LogtoM2MSecret.ps1` 和
`scripts\Start-PublicSignalingServer.ps1`，不要把生产密钥写入 Git。

首次修改依赖或工具链后，建议至少执行一次双机冒烟测试：登录、创建/加入
房间、验证码连接、我的设备连接、屏幕共享、键鼠控制、剪贴板和文件传输。
