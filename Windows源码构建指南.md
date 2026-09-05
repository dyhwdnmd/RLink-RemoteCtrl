# RLink Windows 源码构建指南

本文面向第一次在全新 Windows 电脑上编译 RLink 的开发者。仓库包含
RLink 的正式源码、Visual Studio 解决方案、界面资源以及运行时使用的
FFmpeg DLL，但不包含 Qt SDK 和 libwebrtc 的预编译产物。

## 1. 构建目标

正式解决方案为：

```text
build\RLink.sln
```

使用 `Release | x64` 构建后，主要产物位于：

```text
x64\Release\RLinkAPP.exe
x64\Release\RLinkUpdater.exe
x64\Release\RemoteCSignalServer.exe
```

`RLinkAPP.exe` 是控制端与被控端共用的客户端，
`RLinkUpdater.exe` 是客户端确认更新后使用的独立更新程序，
`RemoteCSignalServer.exe` 是 WSS 信令服务。

## 2. 所需环境

建议使用与当前已验证环境一致的版本：

| 组件 | 版本或要求 |
| --- | --- |
| Windows | Windows 10/11 x64 |
| Visual Studio | Visual Studio 2022 17.14 |
| MSVC | v143，安装“使用 C++ 的桌面开发” |
| Windows SDK | 10.0.26100.0，或兼容的 Windows 10/11 SDK |
| Qt | Qt 6.11.1，MSVC 2022 64-bit |
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

项目直接调用 Qt 自带的 `rcc.exe` 生成资源源码，不强制要求安装
Visual Studio 的 Qt 扩展；但 Qt 安装目录必须完整。

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

先安装并将 `depot_tools` 加入 `PATH`，然后按照 WebRTC 官方 Windows
流程取得源码。以下示例把源码放在 `E:\webrtc_src\src`：

```powershell
New-Item -ItemType Directory -Force E:\webrtc_src
Set-Location E:\webrtc_src
fetch --nohooks webrtc
Set-Location .\src
git checkout 1e2bd46a33bc0a95ff4e032e380f9fcfa2505808
gclient sync -D
```

在 `E:\webrtc_src\src\out\ReleaseMD\args.gn` 写入：

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

其中 `use_dynamic_crt_for_webrtc = true` 不能省略。Qt/MSVC 工程使用
`/MD`，如果 WebRTC 使用 `/MT`，链接时会出现 `LNK2038 RuntimeLibrary`
不匹配。

生成并编译 WebRTC：

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

## 5. 配置本机路径

RLink 不会把本机路径或凭证提交到 Git。复制路径模板：

```powershell
Copy-Item .\build\LocalBuild.props.example `
  .\build\LocalBuild.props
```

编辑 `build\LocalBuild.props`，例如：

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

按实际安装位置修改三个目录。`LocalBuild.props` 已被 `.gitignore` 排除，
不要在其中写入 Logto 密钥、服务器证书或其他凭证。

构建前可以先检查关键文件：

```powershell
$required = @(
  'E:\Qt6\6.11.1\msvc2022_64\bin\rcc.exe',
  'E:\webrtc_src\src\out\ReleaseMD\obj\webrtc.lib',
  '.\third_party\ffmpeg_d3d11va\prefix\bin\avcodec-62.dll',
  '.\third_party\ffmpeg_d3d11va\prefix\bin\avutil-60.dll'
)
$required | ForEach-Object {
  if (-not (Test-Path -LiteralPath $_)) {
    Write-Error "缺少构建依赖：$_"
  }
}
```

## 6. 编译正式解决方案

打开 **Developer PowerShell for VS 2022** 或
**x64 Native Tools Command Prompt for VS 2022**，然后在仓库根目录执行：

```powershell
MSBuild.exe .\build\RLink.sln /t:Build `
  /p:Configuration=Release /p:Platform=x64 /m
```

如果当前终端找不到 `MSBuild.exe`，请使用上述 Visual Studio 开发者终端，
或改为本机实际的 `MSBuild.exe` 路径。也可以使用 Visual Studio 打开
`build\RLink.sln`，选择 `Release | x64` 后生成解决方案。

构建过程会把 Qt、FFmpeg 和平台插件复制到 `x64\Release`。成功后检查：

```powershell
Test-Path .\x64\Release\RLinkAPP.exe
Test-Path .\x64\Release\RLinkUpdater.exe
Test-Path .\x64\Release\RemoteCSignalServer.exe
Test-Path .\x64\Release\platforms\qwindows.dll
Test-Path .\x64\Release\avcodec-62.dll
```

全部返回 `True` 表示主要程序和运行时文件已经生成。

## 7. 常见问题

### 找不到 Qt 头文件或 Qt6*.lib

检查 `QtInstallDir` 是否指向 `msvc2022_64` 根目录，而不是 Qt 安装器根目录。
该目录下必须存在 `include`、`lib`、`bin` 和 `plugins`。

### LNK1104：无法打开 webrtc.lib

检查 `WebRtcOutputDir`，并确认 `obj\webrtc.lib` 已生成。不要把路径指向
`out\ReleaseMD\obj\obj`，项目会自动追加 `obj`。

### LNK2038：RuntimeLibrary 不匹配

当前项目固定使用 `/MD`。重新生成 WebRTC，并确保 GN 参数包含：

```gn
use_dynamic_crt_for_webrtc = true
```

### 找不到 builtin_video_* 或 adapted_video_track_source

这些实现不保证全部包含在聚合 `webrtc.lib` 中。重新执行第 4 节列出的
`autoninja` 命令，不要只构建单个 `webrtc` 目标。

### 构建后程序启动时提示缺少 DLL

先确认整个解决方案构建成功，不要只复制 EXE。程序旁边还需要 Qt DLL、
Qt 插件以及 `third_party\ffmpeg_d3d11va\prefix\bin` 中的运行时 DLL。

## 8. 编译成功与可连接运行的区别

完成以上步骤即可生成客户端和信令服务，但实际登录、WSS 信令和远程连接
还依赖部署环境中的 Logto 配置、TLS 证书、服务端密钥和可访问的信令地址。
这些运行凭证不会提交到源码仓库。部署信令服务前请检查
`scripts\New-PublicSignalingDeployment.ps1`、
`scripts\Set-LogtoM2MSecret.ps1` 和
`scripts\Start-PublicSignalingServer.ps1`，不要把生产密钥写入 Git。

首次修改依赖或工具链后，建议至少执行一次双机冒烟测试：登录、创建/加入
房间、验证码连接、我的设备连接、屏幕共享、键鼠控制、剪贴板和文件传输。
