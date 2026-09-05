# RLink 源码导航与符号参考

> 自动生成于 2026-09-06，源码树 `1ae783c8db12-dirty`。

这套文档按源码文件回答四个问题：这个文件负责什么、定义哪些类型、保存哪些长期状态、提供哪些函数。每个符号都链接到当前源码行。

建议先读人工整理的 [核心源码符号说明](00_核心符号说明.md)，建立三条会话路径、对象所有权和线程关系；再用下面的自动分册查全量符号。

## 阅读边界

- 收录 `src` 下全部 `.h/.hpp/.cpp/.inc` 文件。
- “变量”只收录成员字段和文件级常量/状态，不罗列函数内部临时变量、循环下标和 lambda 捕获。
- “函数”同时收录头文件声明和实现文件定义，因此同一函数可能在两处出现，方便从接口或实现双向查找。
- 紧邻源码注释优先作为作用说明；没有注释时按命名生成中文提示。自动提示是导航，不替代对函数体和调用方的阅读。
- `.inc` 属于其主类的实现切片，成员变量通常只会在主 `.h` 中列出。

## 规模

- 源码文件：309
- 类型：654
- 函数声明/定义：4017
- 成员与文件级变量：2821

## 分册

| 分册 | 文件数 | 内容 |
|---|---:|---|
| [应用层：主界面与远程会话窗口](01_apps_controller.md) | 84 | Qt 主窗口、远程会话窗口、画布、文件窗口、主题与交互控件。 |
| [应用层：会话引擎与业务控制器](02_apps_remote.md) | 50 | direct/room 会话引擎、媒体路由、文件传输、远程粘贴与缓存状态机。 |
| [认证与客户端信令](03_auth_signaling.md) | 26 | Logto/DPAPI 身份、启动配置、WSS 客户端、JSON 消息与恢复。 |
| [核心接口与二进制协议](04_core_protocol.md) | 30 | 引擎抽象、快照、策略、控制器执行器以及输入、光标、屏幕、文件和剪贴板协议。 |
| [Windows 平台能力](05_platform_windows.md) | 58 | DXGI/D3D11、Windows 输入、剪贴板、文件、摄像头、MFT/FFmpeg 编解码和硬件探测。 |
| [WebRTC 传输与媒体运行时](06_webrtc.md) | 27 | PeerConnection、RTP sender、DataChannel、统计、编解码工厂与运行时线程。 |
| [信令服务器](07_server.md) | 29 | WSS 认证、设备注册、direct session、协作房间、持久化、限流和诊断。 |
| [其他源码](08_other.md) | 5 | 未归入现有模块的源码文件。 |

## 怎样查一个功能

1. 先在本页选择所属模块。
2. 在分册中搜索类名、函数名、变量名或文件名。
3. 点击行号进入源码，检查调用者、锁、generation/sequence 和失败路径。
4. 结合人工整理的 [核心源码符号说明](00_核心符号说明.md) 理解跨模块调用和对象所有权。

常见入口：

- 我的设备/验证码：搜索 `DirectSessionRuntimeState`、`ConnectDirectDevice`、`OnIncomingSessionRequest`。
- 协作房间：搜索 `RoomSnapshot`、`RoomPairRuntime`、`CreateRoom`、`RequestRoomControl`。
- 屏幕收发：搜索 `WindowsDesktopCaptureSource`、`SetScreenStreamPreference`、`OnFrame`、`Present`。
- 输入/光标：搜索 `RemoteInputEvent`、`SendRemoteInput`、`WindowsInputExecutor`、`WindowsCursorMonitor`。
- 文件/剪贴板：搜索 `FileTransferController`、`ClipboardController`、`bufferedAmount`。

## 更新方式

在仓库根目录执行：

```powershell
.\tools\Generate-SourceSymbolReference.ps1
```

生成后检查 `git diff -- 源码导航`。若函数改名或职责变化，应同时补充源码注释；这样下一次生成的作用说明也会随源码更新。
