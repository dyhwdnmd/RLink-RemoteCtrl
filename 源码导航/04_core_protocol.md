# 核心接口与二进制协议

> 自动生成于 2026-09-06，源码树 `1ae783c8db12-dirty`。请运行 `tools/Generate-SourceSymbolReference.ps1` 刷新。

引擎抽象、快照、策略、控制器执行器以及输入、光标、屏幕、文件和剪贴板协议。

本册共收录 30 个源码文件。函数与变量的中文作用优先采用源码紧邻注释；无注释时根据符号命名生成阅读提示，最终语义仍以源码为准。

## `src/core/DirectSessionRequest.h`

[打开源码](../src/core/DirectSessionRequest.h) · **文件作用：** 声明 direct session request 相关类型、接口、配置和成员状态。

### 类型

| 行 | 类型 | 种类 | 作用 |
|---:|---|---|---|
| [L10](../src/core/DirectSessionRequest.h#L10) | `SessionPurpose` | enum class | 定义 SessionPurpose 的 enum class 类型和相关状态。 |
| [L19](../src/core/DirectSessionRequest.h#L19) | `DirectAuthorizationMethod` | enum class | Describes only the authorization gate used before a direct P2P session is established. Once signaling returns SessionReady, media and input behavior is intentionally independent... |
| [L25](../src/core/DirectSessionRequest.h#L25) | `DirectSessionConnectRequest` | struct | 定义 DirectSessionConnectRequest 的 struct 类型和相关状态。 |

### 成员与文件级变量

| 行 | 变量 | 声明 | 作用 |
|---:|---|---|---|
| [L26](../src/core/DirectSessionRequest.h#L26) | `targetDeviceId` | `std::string targetDeviceId;` | 保存身份或作用域标识：target device id。 |
| [L27](../src/core/DirectSessionRequest.h#L27) | `purpose` | `SessionPurpose purpose = SessionPurpose::kNone;` | 保存 purpose 相关配置或运行状态。 |
| [L29](../src/core/DirectSessionRequest.h#L29) | `kManualApproval` | `DirectAuthorizationMethod::kManualApproval;` | 定义 manual approval 的编译期常量或产品边界。 |
| [L30](../src/core/DirectSessionRequest.h#L30) | `verificationCode` | `std::string verificationCode;` | 保存 verification code 相关配置或运行状态。 |

## `src/core/DisplayTopology.h`

[打开源码](../src/core/DisplayTopology.h) · **文件作用：** 声明 display topology 相关类型、接口、配置和成员状态。

### 类型

| 行 | 类型 | 种类 | 作用 |
|---:|---|---|---|
| [L16](../src/core/DisplayTopology.h#L16) | `DisplayDescriptor` | struct | A process-local snapshot of one attached Windows display. stableDisplayKey is persisted in settings; sessionDisplayId and layoutVersion travel in the remote-input protocol so st... |
| [L37](../src/core/DisplayTopology.h#L37) | `DisplayTopologySnapshot` | struct | 定义 DisplayTopologySnapshot 的 struct 类型和相关状态。 |

### 成员与文件级变量

| 行 | 变量 | 声明 | 作用 |
|---:|---|---|---|
| [L17](../src/core/DisplayTopology.h#L17) | `sessionDisplayId` | `std::uint32_t sessionDisplayId = 0;` | 保存身份或作用域标识：session display id。 |
| [L18](../src/core/DisplayTopology.h#L18) | `stableDisplayKey` | `std::string stableDisplayKey;` | 保存 stable display key 相关配置或运行状态。 |
| [L19](../src/core/DisplayTopology.h#L19) | `deviceName` | `std::string deviceName;` | 保存路径、地址或显示名称：device name。 |
| [L20](../src/core/DisplayTopology.h#L20) | `friendlyName` | `std::string friendlyName;` | 保存路径、地址或显示名称：friendly name。 |
| [L21](../src/core/DisplayTopology.h#L21) | `left` | `std::int32_t left = 0;` | 保存 left 相关配置或运行状态。 |
| [L22](../src/core/DisplayTopology.h#L22) | `top` | `std::int32_t top = 0;` | 保存 top 相关配置或运行状态。 |
| [L23](../src/core/DisplayTopology.h#L23) | `width` | `std::uint32_t width = 0;` | 保存计数、尺寸或速率指标：width。 |
| [L24](../src/core/DisplayTopology.h#L24) | `height` | `std::uint32_t height = 0;` | 保存计数、尺寸或速率指标：height。 |
| [L25](../src/core/DisplayTopology.h#L25) | `dpiX` | `std::uint32_t dpiX = 96;` | 保存 dpi x 相关配置或运行状态。 |
| [L26](../src/core/DisplayTopology.h#L26) | `dpiY` | `std::uint32_t dpiY = 96;` | 保存 dpi y 相关配置或运行状态。 |
| [L27](../src/core/DisplayTopology.h#L27) | `scalePercent` | `std::uint32_t scalePercent = 100;` | 保存 scale percent 相关配置或运行状态。 |
| [L28](../src/core/DisplayTopology.h#L28) | `rotationDegrees` | `std::uint32_t rotationDegrees = 0;` | 保存 rotation degrees 相关配置或运行状态。 |
| [L29](../src/core/DisplayTopology.h#L29) | `primary` | `bool primary = false;` | 保存 primary 相关配置或运行状态。 |
| [L30](../src/core/DisplayTopology.h#L30) | `adapterLuid` | `std::uint64_t adapterLuid = 0;` | 保存身份或作用域标识：adapter luid。 |
| [L31](../src/core/DisplayTopology.h#L31) | `outputIndex` | `std::uint32_t outputIndex = 0;` | 保存 output index 相关配置或运行状态。 |
| [L34](../src/core/DisplayTopology.h#L34) | `nativeMonitorHandle` | `std::uint64_t nativeMonitorHandle = 0;` | HMONITOR is deliberately stored as an integer so this DTO remains free of Windows headers. It is valid only for the current process/topology. |
| [L38](../src/core/DisplayTopology.h#L38) | `layoutVersion` | `std::uint64_t layoutVersion = 0;` | 保存 layout version 相关配置或运行状态。 |
| [L39](../src/core/DisplayTopology.h#L39) | `displays` | `std::vector<DisplayDescriptor> displays;` | 保存 displays 相关配置或运行状态。 |

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L42](../src/core/DisplayTopology.h#L42) | `FindDisplayByStableKey` | 定义 | `inline const DisplayDescriptor* FindDisplayByStableKey( const DisplayTopologySnapshot& topology, const std::string& stableDisplayKey)` | 查询并返回 find display by stable key 相关逻辑。 |
| [L54](../src/core/DisplayTopology.h#L54) | `FindDisplayBySessionId` | 定义 | `inline const DisplayDescriptor* FindDisplayBySessionId( const DisplayTopologySnapshot& topology, std::uint32_t sessionDisplayId)` | 查询并返回 find display by session id 相关逻辑。 |
| [L66](../src/core/DisplayTopology.h#L66) | `FindPrimaryDisplay` | 定义 | `inline const DisplayDescriptor* FindPrimaryDisplay( const DisplayTopologySnapshot& topology)` | 查询并返回 find primary display 相关逻辑。 |

## `src/core/ISessionEngine.h`

[打开源码](../src/core/ISessionEngine.h) · **文件作用：** 声明 i session engine 相关类型、接口、配置和成员状态。

### 类型

| 行 | 类型 | 种类 | 作用 |
|---:|---|---|---|
| [L22](../src/core/ISessionEngine.h#L22) | `RemoteControlRole` | enum class | 定义 RemoteControlRole 的 enum class 类型和相关状态。 |
| [L28](../src/core/ISessionEngine.h#L28) | `SessionOrigin` | enum class | 定义 SessionOrigin 的 enum class 类型和相关状态。 |
| [L36](../src/core/ISessionEngine.h#L36) | `OwnedDeviceSnapshot` | struct | 定义 OwnedDeviceSnapshot 的 struct 类型和相关状态。 |
| [L45](../src/core/ISessionEngine.h#L45) | `SessionEngineState` | enum class | 定义 SessionEngineState 的 enum class 类型和相关状态。 |
| [L56](../src/core/ISessionEngine.h#L56) | `SessionConnectivityState` | enum class | 定义 SessionConnectivityState 的 enum class 类型和相关状态。 |
| [L64](../src/core/ISessionEngine.h#L64) | `LocalCameraState` | enum class | 定义 LocalCameraState 的 enum class 类型和相关状态。 |
| [L72](../src/core/ISessionEngine.h#L72) | `LocalMicrophoneState` | enum class | 定义 LocalMicrophoneState 的 enum class 类型和相关状态。 |
| [L80](../src/core/ISessionEngine.h#L80) | `SessionEngineCapabilities` | struct | 定义 SessionEngineCapabilities 的 struct 类型和相关状态。 |
| [L125](../src/core/ISessionEngine.h#L125) | `SessionEngineSnapshot` | struct | 定义 SessionEngineSnapshot 的 struct 类型和相关状态。 |
| [L199](../src/core/ISessionEngine.h#L199) | `SessionCommandResult` | struct | 定义 SessionCommandResult 的 struct 类型和相关状态。 |
| [L205](../src/core/ISessionEngine.h#L205) | `ISessionEngineObserver` | class | 定义 ISessionEngineObserver 的 class 类型和相关状态。 |
| [L217](../src/core/ISessionEngine.h#L217) | `ISessionEngine` | class | 定义 ISessionEngine 的 class 类型和相关状态。 |

### 成员与文件级变量

| 行 | 变量 | 声明 | 作用 |
|---:|---|---|---|
| [L37](../src/core/ISessionEngine.h#L37) | `deviceId` | `std::string deviceId;` | 保存身份或作用域标识：device id。 |
| [L38](../src/core/ISessionEngine.h#L38) | `deviceName` | `std::string deviceName;` | 保存路径、地址或显示名称：device name。 |
| [L39](../src/core/ISessionEngine.h#L39) | `online` | `bool online = false;` | 保存 online 相关配置或运行状态。 |
| [L40](../src/core/ISessionEngine.h#L40) | `current` | `bool current = false;` | 保存 current 相关配置或运行状态。 |
| [L41](../src/core/ISessionEngine.h#L41) | `createdAt` | `std::int64_t createdAt = 0;` | 保存 created at 相关配置或运行状态。 |
| [L42](../src/core/ISessionEngine.h#L42) | `lastSeenAt` | `std::int64_t lastSeenAt = 0;` | 保存 last seen at 相关配置或运行状态。 |
| [L81](../src/core/ISessionEngine.h#L81) | `webRtcReady` | `bool webRtcReady = false;` | 保存能力或开关状态：web rtc ready。 |
| [L82](../src/core/ISessionEngine.h#L82) | `hasH264Encoder` | `bool hasH264Encoder = false;` | 保存 has h264 encoder 相关配置或运行状态。 |
| [L83](../src/core/ISessionEngine.h#L83) | `hasH264Decoder` | `bool hasH264Decoder = false;` | 保存 has h264 decoder 相关配置或运行状态。 |
| [L84](../src/core/ISessionEngine.h#L84) | `h264HardwareEncoderAvailable` | `bool h264HardwareEncoderAvailable = false;` | 保存能力或开关状态：h264 hardware encoder available。 |
| [L85](../src/core/ISessionEngine.h#L85) | `h264HardwareEncoderCpuNv12InputSupported` | `bool h264HardwareEncoderCpuNv12InputSupported = false;` | 保存 h264 hardware encoder cpu nv12 input supported 相关配置或运行状态。 |
| [L86](../src/core/ISessionEngine.h#L86) | `h264HardwareEncoderD3D11InputCandidate` | `bool h264HardwareEncoderD3D11InputCandidate = false;` | 保存 h264 hardware encoder d3 d11 input candidate 相关配置或运行状态。 |
| [L87](../src/core/ISessionEngine.h#L87) | `h264HardwareEncoderCount` | `uint32_t h264HardwareEncoderCount = 0;` | 保存计数、尺寸或速率指标：h264 hardware encoder count。 |
| [L88](../src/core/ISessionEngine.h#L88) | `h264HardwareEncoderWired` | `bool h264HardwareEncoderWired = false;` | 保存 h264 hardware encoder wired 相关配置或运行状态。 |
| [L89](../src/core/ISessionEngine.h#L89) | `h264SoftwareEncoderWired` | `bool h264SoftwareEncoderWired = false;` | 保存 h264 software encoder wired 相关配置或运行状态。 |
| [L90](../src/core/ISessionEngine.h#L90) | `ffmpegX264EncoderWired` | `bool ffmpegX264EncoderWired = false;` | 保存 ffmpeg x264 encoder wired 相关配置或运行状态。 |
| [L91](../src/core/ISessionEngine.h#L91) | `ffmpegX264EncoderError` | `std::string ffmpegX264EncoderError;` | 保存最近错误或失败原因：ffmpeg x264 encoder error。 |
| [L92](../src/core/ISessionEngine.h#L92) | `ffmpegHardwareEncoderAvailable` | `bool ffmpegHardwareEncoderAvailable = false;` | 保存能力或开关状态：ffmpeg hardware encoder available。 |
| [L93](../src/core/ISessionEngine.h#L93) | `ffmpegHardwareEncoderWired` | `bool ffmpegHardwareEncoderWired = false;` | 保存 ffmpeg hardware encoder wired 相关配置或运行状态。 |
| [L94](../src/core/ISessionEngine.h#L94) | `ffmpegHardwareEncoderError` | `std::string ffmpegHardwareEncoderError;` | 保存最近错误或失败原因：ffmpeg hardware encoder error。 |
| [L95](../src/core/ISessionEngine.h#L95) | `ffmpegHardwareEncoderDescriptions` | `std::vector<std::string> ffmpegHardwareEncoderDescriptions;` | 保存 ffmpeg hardware encoder descriptions 相关配置或运行状态。 |
| [L96](../src/core/ISessionEngine.h#L96) | `h264SoftwareEncoderFallback` | `bool h264SoftwareEncoderFallback = false;` | 保存 h264 software encoder fallback 相关配置或运行状态。 |
| [L97](../src/core/ISessionEngine.h#L97) | `desktopCapturePreference` | `std::string desktopCapturePreference;` | 保存 desktop capture preference 相关配置或运行状态。 |
| [L98](../src/core/ISessionEngine.h#L98) | `videoEncoderPreference` | `std::string videoEncoderPreference;` | 保存 video encoder preference 相关配置或运行状态。 |
| [L99](../src/core/ISessionEngine.h#L99) | `videoDecoderPreference` | `std::string videoDecoderPreference;` | 保存 video decoder preference 相关配置或运行状态。 |
| [L100](../src/core/ISessionEngine.h#L100) | `videoEncoderRuntimeDetails` | `std::vector<std::string> videoEncoderRuntimeDetails;` | 保存 video encoder runtime details 相关配置或运行状态。 |
| [L101](../src/core/ISessionEngine.h#L101) | `videoEncoderLastFallbackReason` | `std::string videoEncoderLastFallbackReason;` | 保存 video encoder last fallback reason 相关配置或运行状态。 |
| [L102](../src/core/ISessionEngine.h#L102) | `h264HardwareEncoderDescriptions` | `std::vector<std::string> h264HardwareEncoderDescriptions;` | 保存 h264 hardware encoder descriptions 相关配置或运行状态。 |
| [L103](../src/core/ISessionEngine.h#L103) | `h264HardwareEncoderWarnings` | `std::vector<std::string> h264HardwareEncoderWarnings;` | 保存 h264 hardware encoder warnings 相关配置或运行状态。 |
| [L104](../src/core/ISessionEngine.h#L104) | `mfD3D11DecoderConfigured` | `bool mfD3D11DecoderConfigured = false;` | 保存 mf d3 d11 decoder configured 相关配置或运行状态。 |
| [L105](../src/core/ISessionEngine.h#L105) | `mfD3D11DecoderHardware` | `bool mfD3D11DecoderHardware = false;` | 保存 mf d3 d11 decoder hardware 相关配置或运行状态。 |
| [L106](../src/core/ISessionEngine.h#L106) | `mfD3D11DecoderSoftware` | `bool mfD3D11DecoderSoftware = false;` | 保存 mf d3 d11 decoder software 相关配置或运行状态。 |
| [L107](../src/core/ISessionEngine.h#L107) | `d3d11NativeDecoderOutput` | `bool d3d11NativeDecoderOutput = false;` | 保存 d3d11 native decoder output 相关配置或运行状态。 |
| [L108](../src/core/ISessionEngine.h#L108) | `mfD3D11DecoderAsynchronous` | `bool mfD3D11DecoderAsynchronous = false;` | 保存 mf d3 d11 decoder asynchronous 相关配置或运行状态。 |
| [L109](../src/core/ISessionEngine.h#L109) | `ffmpegSoftwareH264Decoder` | `bool ffmpegSoftwareH264Decoder = false;` | 保存 ffmpeg software h264 decoder 相关配置或运行状态。 |
| [L110](../src/core/ISessionEngine.h#L110) | `mfD3D11DecoderName` | `std::string mfD3D11DecoderName;` | 保存路径、地址或显示名称：mf d3 d11 decoder name。 |
| [L111](../src/core/ISessionEngine.h#L111) | `mfD3D11DecoderError` | `std::string mfD3D11DecoderError;` | 保存最近错误或失败原因：mf d3 d11 decoder error。 |
| [L112](../src/core/ISessionEngine.h#L112) | `hardwareFingerprint` | `std::string hardwareFingerprint;` | 保存 hardware fingerprint 相关配置或运行状态。 |
| [L113](../src/core/ISessionEngine.h#L113) | `operatingSystemDescription` | `std::string operatingSystemDescription;` | 保存 operating system description 相关配置或运行状态。 |
| [L114](../src/core/ISessionEngine.h#L114) | `nativeArchitecture` | `std::string nativeArchitecture;` | 保存 native architecture 相关配置或运行状态。 |
| [L115](../src/core/ISessionEngine.h#L115) | `remoteSession` | `bool remoteSession = false;` | 保存 remote session 相关配置或运行状态。 |
| [L116](../src/core/ISessionEngine.h#L116) | `graphicsAdapterDescriptions` | `std::vector<std::string> graphicsAdapterDescriptions;` | 保存 graphics adapter descriptions 相关配置或运行状态。 |
| [L117](../src/core/ISessionEngine.h#L117) | `graphicsEnumerationError` | `std::string graphicsEnumerationError;` | 保存最近错误或失败原因：graphics enumeration error。 |
| [L118](../src/core/ISessionEngine.h#L118) | `h264HardwareEncoderProbeSucceeded` | `bool h264HardwareEncoderProbeSucceeded = false;` | 保存 h264 hardware encoder probe succeeded 相关配置或运行状态。 |
| [L119](../src/core/ISessionEngine.h#L119) | `h264HardwareEncoderProbeFromCache` | `bool h264HardwareEncoderProbeFromCache = false;` | 保存 h264 hardware encoder probe from cache 相关配置或运行状态。 |
| [L120](../src/core/ISessionEngine.h#L120) | `audioDeviceModuleCreated` | `bool audioDeviceModuleCreated = false;` | 保存 audio device module created 相关配置或运行状态。 |
| [L121](../src/core/ISessionEngine.h#L121) | `audioDeviceError` | `std::string audioDeviceError;` | 保存最近错误或失败原因：audio device error。 |
| [L122](../src/core/ISessionEngine.h#L122) | `error` | `std::string error;` | 保存最近错误或失败原因：error。 |
| [L126](../src/core/ISessionEngine.h#L126) | `state` | `SessionEngineState state = SessionEngineState::kStopped;` | 保存状态机当前状态：state。 |
| [L128](../src/core/ISessionEngine.h#L128) | `kNotConfigured` | `SessionConnectivityState::kNotConfigured;` | 定义 not configured 的编译期常量或产品边界。 |
| [L129](../src/core/ISessionEngine.h#L129) | `purpose` | `SessionPurpose purpose = SessionPurpose::kNone;` | 保存 purpose 相关配置或运行状态。 |
| [L130](../src/core/ISessionEngine.h#L130) | `origin` | `SessionOrigin origin = SessionOrigin::kNone;` | 保存 origin 相关配置或运行状态。 |
| [L131](../src/core/ISessionEngine.h#L131) | `remoteControlRole` | `RemoteControlRole remoteControlRole = RemoteControlRole::kNone;` | 保存 remote control role 相关配置或运行状态。 |
| [L132](../src/core/ISessionEngine.h#L132) | `localCamera` | `LocalCameraState localCamera = LocalCameraState::kOff;` | 保存 local camera 相关配置或运行状态。 |
| [L133](../src/core/ISessionEngine.h#L133) | `localMicrophone` | `LocalMicrophoneState localMicrophone = LocalMicrophoneState::kOff;` | 保存 local microphone 相关配置或运行状态。 |
| [L134](../src/core/ISessionEngine.h#L134) | `roomAudioPlaybackMuted` | `bool roomAudioPlaybackMuted = false;` | 保存 room audio playback muted 相关配置或运行状态。 |
| [L137](../src/core/ISessionEngine.h#L137) | `roomControlGrantActive` | `bool roomControlGrantActive = false;` | True only after the signaling server's control-grant event has supplied the lease token used to authenticate input and clipboard packets. |
| [L138](../src/core/ISessionEngine.h#L138) | `remoteCameraPublishing` | `bool remoteCameraPublishing = false;` | 保存 remote camera publishing 相关配置或运行状态。 |
| [L139](../src/core/ISessionEngine.h#L139) | `localDeviceId` | `std::string localDeviceId;` | 保存身份或作用域标识：local device id。 |
| [L140](../src/core/ISessionEngine.h#L140) | `localVerificationCode` | `std::string localVerificationCode;` | 保存 local verification code 相关配置或运行状态。 |
| [L141](../src/core/ISessionEngine.h#L141) | `sessionId` | `std::string sessionId;` | 保存身份或作用域标识：session id。 |
| [L142](../src/core/ISessionEngine.h#L142) | `peerDeviceId` | `std::string peerDeviceId;` | 保存身份或作用域标识：peer device id。 |
| [L145](../src/core/ISessionEngine.h#L145) | `directSignalingSessionReady` | `bool directSignalingSessionReady = false;` | True after signaling has accepted the direct-session authorization and issued session-ready data. WebRTC may still be negotiating. |
| [L146](../src/core/ISessionEngine.h#L146) | `directMediaSlotsPrepared` | `bool directMediaSlotsPrepared = false;` | 保存 direct media slots prepared 相关配置或运行状态。 |
| [L147](../src/core/ISessionEngine.h#L147) | `directControlReliableChannelOpen` | `bool directControlReliableChannelOpen = false;` | 保存能力或开关状态：direct control reliable channel open。 |
| [L148](../src/core/ISessionEngine.h#L148) | `directInputFastChannelOpen` | `bool directInputFastChannelOpen = false;` | 保存能力或开关状态：direct input fast channel open。 |
| [L149](../src/core/ISessionEngine.h#L149) | `directFileTransferChannelOpen` | `bool directFileTransferChannelOpen = false;` | 保存能力或开关状态：direct file transfer channel open。 |
| [L150](../src/core/ISessionEngine.h#L150) | `directClipboardReliableChannelOpen` | `bool directClipboardReliableChannelOpen = false;` | 保存能力或开关状态：direct clipboard reliable channel open。 |
| [L151](../src/core/ISessionEngine.h#L151) | `directClipboardTransferChannelOpen` | `bool directClipboardTransferChannelOpen = false;` | 保存能力或开关状态：direct clipboard transfer channel open。 |
| [L154](../src/core/ISessionEngine.h#L154) | `directSessionEverActive` | `bool directSessionEverActive = false;` | Remains true across temporary signaling/P2P interruptions after this direct session has reached Connected at least once. |
| [L158](../src/core/ISessionEngine.h#L158) | `directIceRestartAttempt` | `std::uint32_t directIceRestartAttempt = 0;` | Recovery telemetry for owned-device and verification-code sessions. Initial negotiation also uses kConnecting, so the UI combines this attempt count with its own "was active" la... |
| [L159](../src/core/ISessionEngine.h#L159) | `directRemoteDisplay` | `DisplayDescriptor directRemoteDisplay;` | 保存 direct remote display 相关配置或运行状态。 |
| [L160](../src/core/ISessionEngine.h#L160) | `directRemoteDisplayLayoutVersion` | `std::uint64_t directRemoteDisplayLayoutVersion = 0;` | 保存 direct remote display layout version 相关配置或运行状态。 |
| [L161](../src/core/ISessionEngine.h#L161) | `directRemoteScreenShareGeneration` | `std::uint64_t directRemoteScreenShareGeneration = 0;` | 标记当前世代，用于拒绝过期异步结果：direct remote screen share generation。 |
| [L162](../src/core/ISessionEngine.h#L162) | `directScreenPreferencePending` | `bool directScreenPreferencePending = false;` | 保存待处理队列或请求：direct screen preference pending。 |
| [L163](../src/core/ISessionEngine.h#L163) | `directScreenPreferenceSequence` | `std::uint64_t directScreenPreferenceSequence = 0;` | 保存单调序号，用于排序或去重：direct screen preference sequence。 |
| [L164](../src/core/ISessionEngine.h#L164) | `directScreenWidth` | `std::uint32_t directScreenWidth = 0;` | 保存计数、尺寸或速率指标：direct screen width。 |
| [L165](../src/core/ISessionEngine.h#L165) | `directScreenHeight` | `std::uint32_t directScreenHeight = 0;` | 保存计数、尺寸或速率指标：direct screen height。 |
| [L166](../src/core/ISessionEngine.h#L166) | `directScreenFramesPerSecond` | `std::uint32_t directScreenFramesPerSecond = kDefaultScreenFrameRate;` | 保存 direct screen frames per second 相关配置或运行状态。 |
| [L167](../src/core/ISessionEngine.h#L167) | `directScreenMaximumFrameRate` | `std::uint32_t directScreenMaximumFrameRate = kMaximumScreenFrameRate;` | 保存计数、尺寸或速率指标：direct screen maximum frame rate。 |
| [L168](../src/core/ISessionEngine.h#L168) | `directScreenMaxBitrateBps` | `std::uint32_t directScreenMaxBitrateBps = 0;` | 保存计数、尺寸或速率指标：direct screen max bitrate bps。 |
| [L169](../src/core/ISessionEngine.h#L169) | `directRemoteDisplayCatalogReported` | `bool directRemoteDisplayCatalogReported = false;` | 保存 direct remote display catalog reported 相关配置或运行状态。 |
| [L170](../src/core/ISessionEngine.h#L170) | `directRemoteDisplayCatalogLayoutVersion` | `std::uint64_t directRemoteDisplayCatalogLayoutVersion = 0;` | 保存 direct remote display catalog layout version 相关配置或运行状态。 |
| [L171](../src/core/ISessionEngine.h#L171) | `directRemoteDisplays` | `std::vector<DisplayDescriptor> directRemoteDisplays;` | 保存 direct remote displays 相关配置或运行状态。 |
| [L172](../src/core/ISessionEngine.h#L172) | `directRemoteDisplaySwitchPending` | `bool directRemoteDisplaySwitchPending = false;` | 保存待处理队列或请求：direct remote display switch pending。 |
| [L173](../src/core/ISessionEngine.h#L173) | `directRemoteDisplaySwitchSequence` | `std::uint64_t directRemoteDisplaySwitchSequence = 0;` | 保存单调序号，用于排序或去重：direct remote display switch sequence。 |
| [L174](../src/core/ISessionEngine.h#L174) | `directRemoteDisplaySwitchError` | `std::string directRemoteDisplaySwitchError;` | 保存最近错误或失败原因：direct remote display switch error。 |
| [L175](../src/core/ISessionEngine.h#L175) | `errorCode` | `std::string errorCode;` | 保存 error code 相关配置或运行状态。 |
| [L176](../src/core/ISessionEngine.h#L176) | `errorMessage` | `std::string errorMessage;` | 保存 error message 相关配置或运行状态。 |
| [L177](../src/core/ISessionEngine.h#L177) | `screenShareGeneration` | `std::uint64_t screenShareGeneration = 0;` | 标记当前世代，用于拒绝过期异步结果：screen share generation。 |
| [L178](../src/core/ISessionEngine.h#L178) | `localDisplayTopology` | `DisplayTopologySnapshot localDisplayTopology;` | 保存 local display topology 相关配置或运行状态。 |
| [L179](../src/core/ISessionEngine.h#L179) | `localMediaDevices` | `MediaDeviceSnapshot localMediaDevices;` | 保存 local media devices 相关配置或运行状态。 |
| [L180](../src/core/ISessionEngine.h#L180) | `selectedLocalDisplayKey` | `std::string selectedLocalDisplayKey;` | 保存 selected local display key 相关配置或运行状态。 |
| [L181](../src/core/ISessionEngine.h#L181) | `activeSharedDisplay` | `DisplayDescriptor activeSharedDisplay;` | 保存 active shared display 相关配置或运行状态。 |
| [L182](../src/core/ISessionEngine.h#L182) | `activeSharedDisplayLayoutVersion` | `std::uint64_t activeSharedDisplayLayoutVersion = 0;` | 保存 active shared display layout version 相关配置或运行状态。 |
| [L183](../src/core/ISessionEngine.h#L183) | `room` | `RoomSnapshot room;` | 保存 room 相关配置或运行状态。 |
| [L184](../src/core/ISessionEngine.h#L184) | `incomingRoomJoinRequests` | `std::vector<RoomJoinRequest> incomingRoomJoinRequests;` | 保存 incoming room join requests 相关配置或运行状态。 |
| [L186](../src/core/ISessionEngine.h#L186) | `incomingRoomScreenShareSwitchRequests` | `incomingRoomScreenShareSwitchRequests;` | 保存 incoming room screen share switch requests 相关配置或运行状态。 |
| [L187](../src/core/ISessionEngine.h#L187) | `incomingRoomControlRequests` | `std::vector<RoomControlRequest> incomingRoomControlRequests;` | 保存 incoming room control requests 相关配置或运行状态。 |
| [L189](../src/core/ISessionEngine.h#L189) | `incomingRoomScreenShareViewRequests` | `incomingRoomScreenShareViewRequests;` | 保存 incoming room screen share view requests 相关配置或运行状态。 |
| [L190](../src/core/ISessionEngine.h#L190) | `roomMemberActionResults` | `std::vector<RoomMemberActionResult> roomMemberActionResults;` | 保存 room member action results 相关配置或运行状态。 |
| [L191](../src/core/ISessionEngine.h#L191) | `outgoingRoomScreenShareSwitchRequestId` | `std::string outgoingRoomScreenShareSwitchRequestId;` | 保存身份或作用域标识：outgoing room screen share switch request id。 |
| [L192](../src/core/ISessionEngine.h#L192) | `roomPeerConnections` | `std::vector<RoomPeerConnectionSnapshot> roomPeerConnections;` | 保存 room peer connections 相关配置或运行状态。 |
| [L193](../src/core/ISessionEngine.h#L193) | `roomAvailabilities` | `std::vector<RoomAvailabilitySnapshot> roomAvailabilities;` | 保存 room availabilities 相关配置或运行状态。 |
| [L194](../src/core/ISessionEngine.h#L194) | `ownedDevicesRevision` | `std::uint64_t ownedDevicesRevision = 0;` | 标记当前世代，用于拒绝过期异步结果：owned devices revision。 |
| [L195](../src/core/ISessionEngine.h#L195) | `ownedDevicesLoaded` | `bool ownedDevicesLoaded = false;` | 保存 owned devices loaded 相关配置或运行状态。 |
| [L196](../src/core/ISessionEngine.h#L196) | `ownedDevices` | `std::vector<OwnedDeviceSnapshot> ownedDevices;` | 保存 owned devices 相关配置或运行状态。 |
| [L200](../src/core/ISessionEngine.h#L200) | `accepted` | `bool accepted = false;` | 保存 accepted 相关配置或运行状态。 |
| [L201](../src/core/ISessionEngine.h#L201) | `errorCode` | `std::string errorCode;` | 保存 error code 相关配置或运行状态。 |
| [L202](../src/core/ISessionEngine.h#L202) | `errorMessage` | `std::string errorMessage;` | 保存 error message 相关配置或运行状态。 |

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L207](../src/core/ISessionEngine.h#L207) | `~ISessionEngineObserver` | 声明 | `virtual ~ISessionEngineObserver() = default` | 停止相关活动并释放 ISessionEngineObserver 实例拥有的资源。 |
| [L213](../src/core/ISessionEngine.h#L213) | `OnSessionEngineSnapshot` | 声明 | `virtual void OnSessionEngineSnapshot( const SessionEngineSnapshot& snapshot) = 0` | V1 callbacks are synchronous on the thread that changes engine state. That may be the Qt signaling thread or the session controller executor; UI observers must marshal the immut... |
| [L219](../src/core/ISessionEngine.h#L219) | `~ISessionEngine` | 声明 | `virtual ~ISessionEngine() = default` | 停止相关活动并释放 ISessionEngine 实例拥有的资源。 |
| [L221](../src/core/ISessionEngine.h#L221) | `SetObserver` | 声明 | `virtual void SetObserver(ISessionEngineObserver* observer) = 0` | 更新或应用 set observer 相关逻辑。 |
| [L222](../src/core/ISessionEngine.h#L222) | `Start` | 声明 | `virtual SessionCommandResult Start() = 0` | 启动 start 相关逻辑。 |
| [L223](../src/core/ISessionEngine.h#L223) | `Stop` | 声明 | `virtual void Stop() = 0` | 停止 stop 相关逻辑。 |
| [L225](../src/core/ISessionEngine.h#L225) | `Snapshot` | 声明 | `virtual SessionEngineSnapshot Snapshot() const = 0` | 查询并返回 snapshot 相关逻辑。 |
| [L226](../src/core/ISessionEngine.h#L226) | `Capabilities` | 声明 | `virtual SessionEngineCapabilities Capabilities() const = 0` | 查询并返回 capabilities 相关逻辑。 |
| [L227](../src/core/ISessionEngine.h#L227) | `Diagnostics` | 定义 | `virtual SessionDiagnosticsSnapshot Diagnostics() const { return {}; }` | 查询并返回 diagnostics 相关逻辑。 |
| [L229](../src/core/ISessionEngine.h#L229) | `ConnectDirectDevice` | 声明 | `virtual SessionCommandResult ConnectDirectDevice( const DirectSessionConnectRequest& request) = 0` | 建立连接 connect direct device 相关逻辑。 |
| [L234](../src/core/ISessionEngine.h#L234) | `ConnectDevice` | 声明 | `virtual SessionCommandResult ConnectDevice( const std::string& deviceId, SessionPurpose purpose) = 0` | Compatibility entry points. Implementations translate these to one validated DirectSessionConnectRequest so media setup has one path. |
| [L237](../src/core/ISessionEngine.h#L237) | `ConnectOwnedDevice` | 声明 | `virtual SessionCommandResult ConnectOwnedDevice( const std::string& deviceId, SessionPurpose purpose) = 0` | 建立连接 connect owned device 相关逻辑。 |
| [L240](../src/core/ISessionEngine.h#L240) | `ConnectAssistedDevice` | 声明 | `virtual SessionCommandResult ConnectAssistedDevice( const std::string& deviceId, const std::string& verificationCode) = 0` | 建立连接 connect assisted device 相关逻辑。 |
| [L243](../src/core/ISessionEngine.h#L243) | `RefreshOwnedDevices` | 声明 | `virtual SessionCommandResult RefreshOwnedDevices() = 0` | 刷新 refresh owned devices 相关逻辑。 |
| [L244](../src/core/ISessionEngine.h#L244) | `AcceptIncomingSession` | 声明 | `virtual SessionCommandResult AcceptIncomingSession( const std::string& sessionId) = 0` | 处理并回复 accept incoming session 相关逻辑。 |
| [L246](../src/core/ISessionEngine.h#L246) | `RejectIncomingSession` | 声明 | `virtual SessionCommandResult RejectIncomingSession( const std::string& sessionId) = 0` | 处理并回复 reject incoming session 相关逻辑。 |
| [L248](../src/core/ISessionEngine.h#L248) | `Disconnect` | 声明 | `virtual SessionCommandResult Disconnect() = 0` | 断开连接 disconnect 相关逻辑。 |
| [L254](../src/core/ISessionEngine.h#L254) | `CreateRoom` | 声明 | `virtual SessionCommandResult CreateRoom(std::uint32_t capacity) = 0` | Room control-plane operations are intentionally independent from the legacy direct-session API during migration. Room membership and member-pair WebRTC connections use the same ... |
| [L255](../src/core/ISessionEngine.h#L255) | `JoinRoom` | 声明 | `virtual SessionCommandResult JoinRoom(const std::string& roomId) = 0` | 实现 join room 对应的业务或工具逻辑。 |
| [L256](../src/core/ISessionEngine.h#L256) | `QueryRoomAvailability` | 定义 | `virtual SessionCommandResult QueryRoomAvailability( const std::vector<std::string>& roomIds)` | 发起请求或查询 query room availability 相关逻辑。 |
| [L263](../src/core/ISessionEngine.h#L263) | `RespondToRoomJoin` | 声明 | `virtual SessionCommandResult RespondToRoomJoin( const std::string& requestId, bool accepted) = 0` | 处理并回复 respond to room join 相关逻辑。 |
| [L266](../src/core/ISessionEngine.h#L266) | `SetRoomCapacity` | 声明 | `virtual SessionCommandResult SetRoomCapacity( std::uint32_t capacity) = 0` | 更新或应用 set room capacity 相关逻辑。 |
| [L268](../src/core/ISessionEngine.h#L268) | `LeaveRoom` | 声明 | `virtual SessionCommandResult LeaveRoom() = 0` | 实现 leave room 对应的业务或工具逻辑。 |
| [L269](../src/core/ISessionEngine.h#L269) | `RefreshLocalDisplays` | 声明 | `virtual SessionCommandResult RefreshLocalDisplays() = 0` | 刷新 refresh local displays 相关逻辑。 |
| [L270](../src/core/ISessionEngine.h#L270) | `SelectRoomScreenShareDisplay` | 声明 | `virtual SessionCommandResult SelectRoomScreenShareDisplay( const std::string& stableDisplayKey) = 0` | 查询并返回 select room screen share display 相关逻辑。 |
| [L272](../src/core/ISessionEngine.h#L272) | `StartRoomScreenShare` | 声明 | `virtual SessionCommandResult StartRoomScreenShare() = 0` | 启动 start room screen share 相关逻辑。 |
| [L273](../src/core/ISessionEngine.h#L273) | `StopRoomScreenShare` | 声明 | `virtual SessionCommandResult StopRoomScreenShare() = 0` | 停止 stop room screen share 相关逻辑。 |
| [L274](../src/core/ISessionEngine.h#L274) | `RespondToRoomScreenShareSwitch` | 声明 | `virtual SessionCommandResult RespondToRoomScreenShareSwitch( const std::string& requestId, bool accepted) = 0` | 处理并回复 respond to room screen share switch 相关逻辑。 |
| [L277](../src/core/ISessionEngine.h#L277) | `CancelRoomScreenShareSwitch` | 声明 | `virtual SessionCommandResult CancelRoomScreenShareSwitch() = 0` | 判断 cancel room screen share switch 相关逻辑。 |
| [L278](../src/core/ISessionEngine.h#L278) | `RequestRoomControl` | 声明 | `virtual SessionCommandResult RequestRoomControl() = 0` | 发起请求或查询 request room control 相关逻辑。 |
| [L279](../src/core/ISessionEngine.h#L279) | `RespondToRoomControl` | 声明 | `virtual SessionCommandResult RespondToRoomControl( const std::string& requestId, bool accepted) = 0` | 处理并回复 respond to room control 相关逻辑。 |
| [L282](../src/core/ISessionEngine.h#L282) | `ReleaseRoomControl` | 声明 | `virtual SessionCommandResult ReleaseRoomControl() = 0` | 释放或取消 release room control 相关逻辑。 |
| [L283](../src/core/ISessionEngine.h#L283) | `RequestRoomMemberScreenShare` | 声明 | `virtual SessionCommandResult RequestRoomMemberScreenShare( const std::string& peerDeviceId) = 0` | 发起请求或查询 request room member screen share 相关逻辑。 |
| [L285](../src/core/ISessionEngine.h#L285) | `RespondToRoomMemberScreenShare` | 声明 | `virtual SessionCommandResult RespondToRoomMemberScreenShare( const std::string& requesterDeviceId, std::uint64_t sequence, bool accepted) = 0` | 处理并回复 respond to room member screen share 相关逻辑。 |
| [L289](../src/core/ISessionEngine.h#L289) | `RequestRoomMemberMicrophoneMute` | 声明 | `virtual SessionCommandResult RequestRoomMemberMicrophoneMute( const std::string& peerDeviceId) = 0` | 发起请求或查询 request room member microphone mute 相关逻辑。 |
| [L291](../src/core/ISessionEngine.h#L291) | `RequestRemoteRoomScreenShareStop` | 声明 | `virtual SessionCommandResult RequestRemoteRoomScreenShareStop( const std::string& peerDeviceId, std::uint64_t screenShareEpoch) = 0` | 发起请求或查询 request remote room screen share stop 相关逻辑。 |
| [L294](../src/core/ISessionEngine.h#L294) | `SendRoomInput` | 声明 | `virtual SessionCommandResult SendRoomInput( const RemoteInputEvent& event) = 0` | 发送或发布 send room input 相关逻辑。 |
| [L296](../src/core/ISessionEngine.h#L296) | `SetRoomScreenFrameRate` | 声明 | `virtual SessionCommandResult SetRoomScreenFrameRate( const std::string& pairId, std::uint32_t framesPerSecond) = 0` | 更新或应用 set room screen frame rate 相关逻辑。 |
| [L299](../src/core/ISessionEngine.h#L299) | `SetRoomScreenStreamPreference` | 声明 | `virtual SessionCommandResult SetRoomScreenStreamPreference( const std::string& pairId, const ScreenStreamPreferenceRequest& preference) = 0` | 更新或应用 set room screen stream preference 相关逻辑。 |
| [L302](../src/core/ISessionEngine.h#L302) | `RequestRemoteSharedDisplaySwitch` | 声明 | `virtual SessionCommandResult RequestRemoteSharedDisplaySwitch( const std::string& pairId, const std::string& stableDisplayKey) = 0` | 发起请求或查询 request remote shared display switch 相关逻辑。 |
| [L305](../src/core/ISessionEngine.h#L305) | `SendRoomFileMessage` | 声明 | `virtual SessionCommandResult SendRoomFileMessage( const std::string& peerDeviceId, const FileTransferMessage& message) = 0` | 发送或发布 send room file message 相关逻辑。 |
| [L308](../src/core/ISessionEngine.h#L308) | `SendRoomClipboardMessage` | 声明 | `virtual SessionCommandResult SendRoomClipboardMessage( const std::string& peerDeviceId, const std::string& clipboardSessionId, const ClipboardMessage& message) = 0` | 发送或发布 send room clipboard message 相关逻辑。 |
| [L315](../src/core/ISessionEngine.h#L315) | `SetLocalCameraEnabled` | 声明 | `virtual SessionCommandResult SetLocalCameraEnabled(bool enabled) = 0` | A peer may only enable its own camera. Enabling starts local preview and publishes the local Camera Track; disabling stops capture and sending. |
| [L316](../src/core/ISessionEngine.h#L316) | `SetLocalMicrophoneEnabled` | 声明 | `virtual SessionCommandResult SetLocalMicrophoneEnabled(bool enabled) = 0` | 更新或应用 set local microphone enabled 相关逻辑。 |
| [L317](../src/core/ISessionEngine.h#L317) | `SetRoomAudioPlaybackMuted` | 声明 | `virtual SessionCommandResult SetRoomAudioPlaybackMuted(bool muted) = 0` | 更新或应用 set room audio playback muted 相关逻辑。 |
| [L318](../src/core/ISessionEngine.h#L318) | `RefreshLocalMediaDevices` | 声明 | `virtual SessionCommandResult RefreshLocalMediaDevices() = 0` | 刷新 refresh local media devices 相关逻辑。 |
| [L319](../src/core/ISessionEngine.h#L319) | `SelectLocalCameraDevice` | 声明 | `virtual SessionCommandResult SelectLocalCameraDevice( const std::string& deviceId) = 0` | 查询并返回 select local camera device 相关逻辑。 |
| [L321](../src/core/ISessionEngine.h#L321) | `SelectLocalMicrophoneDevice` | 声明 | `virtual SessionCommandResult SelectLocalMicrophoneDevice( const std::string& deviceId) = 0` | 查询并返回 select local microphone device 相关逻辑。 |
| [L323](../src/core/ISessionEngine.h#L323) | `SelectLocalSpeakerDevice` | 声明 | `virtual SessionCommandResult SelectLocalSpeakerDevice( const std::string& deviceId) = 0` | 查询并返回 select local speaker device 相关逻辑。 |

## `src/core/MediaDevice.h`

[打开源码](../src/core/MediaDevice.h) · **文件作用：** 声明 media device 相关类型、接口、配置和成员状态。

### 类型

| 行 | 类型 | 种类 | 作用 |
|---:|---|---|---|
| [L14](../src/core/MediaDevice.h#L14) | `MediaDeviceKind` | enum class | 定义 MediaDeviceKind 的 enum class 类型和相关状态。 |
| [L20](../src/core/MediaDevice.h#L20) | `MediaDeviceDescriptor` | struct | 定义 MediaDeviceDescriptor 的 struct 类型和相关状态。 |
| [L26](../src/core/MediaDevice.h#L26) | `MediaDeviceSelectionState` | enum class | 定义 MediaDeviceSelectionState 的 enum class 类型和相关状态。 |
| [L33](../src/core/MediaDevice.h#L33) | `MediaDeviceCategorySnapshot` | struct | 定义 MediaDeviceCategorySnapshot 的 struct 类型和相关状态。 |
| [L46](../src/core/MediaDevice.h#L46) | `MediaDeviceSnapshot` | struct | 定义 MediaDeviceSnapshot 的 struct 类型和相关状态。 |

### 成员与文件级变量

| 行 | 变量 | 声明 | 作用 |
|---:|---|---|---|
| [L12](../src/core/MediaDevice.h#L12) | `kSystemDefaultMediaDeviceId` | `inline constexpr char kSystemDefaultMediaDeviceId[] = "default";` | 定义 system default media device id 的编译期常量或产品边界。 |
| [L21](../src/core/MediaDevice.h#L21) | `id` | `std::string id;` | 保存身份或作用域标识：id。 |
| [L22](../src/core/MediaDevice.h#L22) | `name` | `std::string name;` | 保存路径、地址或显示名称：name。 |
| [L23](../src/core/MediaDevice.h#L23) | `available` | `bool available = true;` | 保存能力或开关状态：available。 |
| [L34](../src/core/MediaDevice.h#L34) | `devices` | `std::vector<MediaDeviceDescriptor> devices;` | 保存 devices 相关配置或运行状态。 |
| [L35](../src/core/MediaDevice.h#L35) | `preferredDeviceId` | `std::string preferredDeviceId = kSystemDefaultMediaDeviceId;` | 保存身份或作用域标识：preferred device id。 |
| [L36](../src/core/MediaDevice.h#L36) | `activeDeviceId` | `std::string activeDeviceId;` | 保存身份或作用域标识：active device id。 |
| [L39](../src/core/MediaDevice.h#L39) | `activeDeviceName` | `std::string activeDeviceName;` | Resolved physical endpoint name. When activeDeviceId is "default", this identifies the Windows default device that is actually in use. |
| [L41](../src/core/MediaDevice.h#L41) | `kReady` | `MediaDeviceSelectionState::kReady;` | 定义 ready 的编译期常量或产品边界。 |
| [L42](../src/core/MediaDevice.h#L42) | `errorCode` | `std::string errorCode;` | 保存 error code 相关配置或运行状态。 |
| [L43](../src/core/MediaDevice.h#L43) | `errorMessage` | `std::string errorMessage;` | 保存 error message 相关配置或运行状态。 |
| [L47](../src/core/MediaDevice.h#L47) | `camera` | `MediaDeviceCategorySnapshot camera;` | 保存 camera 相关配置或运行状态。 |
| [L48](../src/core/MediaDevice.h#L48) | `microphone` | `MediaDeviceCategorySnapshot microphone;` | 保存 microphone 相关配置或运行状态。 |
| [L49](../src/core/MediaDevice.h#L49) | `speaker` | `MediaDeviceCategorySnapshot speaker;` | 保存 speaker 相关配置或运行状态。 |
| [L50](../src/core/MediaDevice.h#L50) | `refreshing` | `bool refreshing = false;` | 保存 refreshing 相关配置或运行状态。 |
| [L51](../src/core/MediaDevice.h#L51) | `revision` | `std::uint64_t revision = 0;` | 标记当前世代，用于拒绝过期异步结果：revision。 |

## `src/core/RemoteInputTelemetry.h`

[打开源码](../src/core/RemoteInputTelemetry.h) · **文件作用：** 声明 remote input telemetry 相关类型、接口、配置和成员状态。

### 类型

| 行 | 类型 | 种类 | 作用 |
|---:|---|---|---|
| [L16](../src/core/RemoteInputTelemetry.h#L16) | `RemoteInputTelemetryCounters` | struct | 定义 RemoteInputTelemetryCounters 的 struct 类型和相关状态。 |
| [L36](../src/core/RemoteInputTelemetry.h#L36) | `RemoteInputTelemetrySnapshot` | struct | 定义 RemoteInputTelemetrySnapshot 的 struct 类型和相关状态。 |
| [L51](../src/core/RemoteInputTelemetry.h#L51) | `RemoteInputTelemetry` | class | Process-wide, opt-in counters for the controller, DataChannel and Windows SendInput stages. Keeping this independent from a particular room pair lets the controlled and controll... |
| [L170](../src/core/RemoteInputTelemetry.h#L170) | `InjectionType` | enum class | 定义 InjectionType 的 enum class 类型和相关状态。 |

### 成员与文件级变量

| 行 | 变量 | 声明 | 作用 |
|---:|---|---|---|
| [L17](../src/core/RemoteInputTelemetry.h#L17) | `generatedMouseMoves` | `std::uint64_t generatedMouseMoves = 0;` | 保存 generated mouse moves 相关配置或运行状态。 |
| [L18](../src/core/RemoteInputTelemetry.h#L18) | `dispatchedMouseMoves` | `std::uint64_t dispatchedMouseMoves = 0;` | 保存 dispatched mouse moves 相关配置或运行状态。 |
| [L19](../src/core/RemoteInputTelemetry.h#L19) | `coalescedMouseMoves` | `std::uint64_t coalescedMouseMoves = 0;` | 保存 coalesced mouse moves 相关配置或运行状态。 |
| [L20](../src/core/RemoteInputTelemetry.h#L20) | `generatedMouseButtons` | `std::uint64_t generatedMouseButtons = 0;` | 保存 generated mouse buttons 相关配置或运行状态。 |
| [L21](../src/core/RemoteInputTelemetry.h#L21) | `generatedMouseWheels` | `std::uint64_t generatedMouseWheels = 0;` | 保存 generated mouse wheels 相关配置或运行状态。 |
| [L22](../src/core/RemoteInputTelemetry.h#L22) | `generatedKeys` | `std::uint64_t generatedKeys = 0;` | 保存 generated keys 相关配置或运行状态。 |
| [L23](../src/core/RemoteInputTelemetry.h#L23) | `fastPacketsQueued` | `std::uint64_t fastPacketsQueued = 0;` | 保存 fast packets queued 相关配置或运行状态。 |
| [L24](../src/core/RemoteInputTelemetry.h#L24) | `reliablePacketsQueued` | `std::uint64_t reliablePacketsQueued = 0;` | 保存 reliable packets queued 相关配置或运行状态。 |
| [L25](../src/core/RemoteInputTelemetry.h#L25) | `packetsSent` | `std::uint64_t packetsSent = 0;` | 保存 packets sent 相关配置或运行状态。 |
| [L26](../src/core/RemoteInputTelemetry.h#L26) | `packetsSendFailed` | `std::uint64_t packetsSendFailed = 0;` | 保存 packets send failed 相关配置或运行状态。 |
| [L27](../src/core/RemoteInputTelemetry.h#L27) | `packetsReceived` | `std::uint64_t packetsReceived = 0;` | 保存 packets received 相关配置或运行状态。 |
| [L28](../src/core/RemoteInputTelemetry.h#L28) | `packetsDropped` | `std::uint64_t packetsDropped = 0;` | 保存 packets dropped 相关配置或运行状态。 |
| [L29](../src/core/RemoteInputTelemetry.h#L29) | `injectedMouseMoves` | `std::uint64_t injectedMouseMoves = 0;` | 保存 injected mouse moves 相关配置或运行状态。 |
| [L30](../src/core/RemoteInputTelemetry.h#L30) | `injectedMouseButtons` | `std::uint64_t injectedMouseButtons = 0;` | 保存 injected mouse buttons 相关配置或运行状态。 |
| [L31](../src/core/RemoteInputTelemetry.h#L31) | `injectedMouseWheels` | `std::uint64_t injectedMouseWheels = 0;` | 保存 injected mouse wheels 相关配置或运行状态。 |
| [L32](../src/core/RemoteInputTelemetry.h#L32) | `injectedKeys` | `std::uint64_t injectedKeys = 0;` | 保存 injected keys 相关配置或运行状态。 |
| [L33](../src/core/RemoteInputTelemetry.h#L33) | `injectionFailures` | `std::uint64_t injectionFailures = 0;` | 保存 injection failures 相关配置或运行状态。 |
| [L37](../src/core/RemoteInputTelemetry.h#L37) | `enabled` | `bool enabled = false;` | 保存能力或开关状态：enabled。 |
| [L38](../src/core/RemoteInputTelemetry.h#L38) | `moveDispatchRateLimitHz` | `std::uint32_t moveDispatchRateLimitHz = 0;` | 保存 move dispatch rate limit hz 相关配置或运行状态。 |
| [L39](../src/core/RemoteInputTelemetry.h#L39) | `sampleWindowMs` | `std::uint32_t sampleWindowMs = 0;` | 保存 sample window ms 相关配置或运行状态。 |
| [L40](../src/core/RemoteInputTelemetry.h#L40) | `moveDispatchIntervalSamples` | `std::uint32_t moveDispatchIntervalSamples = 0;` | 保存 move dispatch interval samples 相关配置或运行状态。 |
| [L41](../src/core/RemoteInputTelemetry.h#L41) | `moveDispatchIntervalAverageUs` | `std::uint32_t moveDispatchIntervalAverageUs = 0;` | 保存 move dispatch interval average us 相关配置或运行状态。 |
| [L42](../src/core/RemoteInputTelemetry.h#L42) | `moveDispatchIntervalP95Us` | `std::uint32_t moveDispatchIntervalP95Us = 0;` | 保存 move dispatch interval p95 us 相关配置或运行状态。 |
| [L43](../src/core/RemoteInputTelemetry.h#L43) | `moveDispatchIntervalMaximumUs` | `std::uint32_t moveDispatchIntervalMaximumUs = 0;` | 保存 move dispatch interval maximum us 相关配置或运行状态。 |
| [L44](../src/core/RemoteInputTelemetry.h#L44) | `totals` | `RemoteInputTelemetryCounters totals;` | 保存 totals 相关配置或运行状态。 |
| [L45](../src/core/RemoteInputTelemetry.h#L45) | `perSecond` | `RemoteInputTelemetryCounters perSecond;` | 保存 per second 相关配置或运行状态。 |
| [L268](../src/core/RemoteInputTelemetry.h#L268) | `kMaximumMeasuredMoveIntervalUs` | `kMaximumMeasuredMoveIntervalUs = 250'000;` | 定义 maximum measured move interval us 的编译期常量或产品边界。 |
| [L270](../src/core/RemoteInputTelemetry.h#L270) | `kMoveIntervalHistogramBucketUs` | `kMoveIntervalHistogramBucketUs = 500;` | 定义 move interval histogram bucket us 的编译期常量或产品边界。 |
| [L272](../src/core/RemoteInputTelemetry.h#L272) | `kMoveIntervalHistogramBuckets` | `kMoveIntervalHistogramBuckets = 128;` | 定义 move interval histogram buckets 的编译期常量或产品边界。 |
| [L393](../src/core/RemoteInputTelemetry.h#L393) | `enabled_` | `std::atomic_bool enabled_{false};` | 保存能力或开关状态：enabled。 |
| [L394](../src/core/RemoteInputTelemetry.h#L394) | `moveDispatchRateLimitHz_` | `std::atomic<std::uint32_t> moveDispatchRateLimitHz_{0};` | 保存 move dispatch rate limit hz 相关配置或运行状态。 |
| [L395](../src/core/RemoteInputTelemetry.h#L395) | `generatedMouseMoves_` | `std::atomic<std::uint64_t> generatedMouseMoves_{0};` | 保存 generated mouse moves 相关配置或运行状态。 |
| [L396](../src/core/RemoteInputTelemetry.h#L396) | `dispatchedMouseMoves_` | `std::atomic<std::uint64_t> dispatchedMouseMoves_{0};` | 保存 dispatched mouse moves 相关配置或运行状态。 |
| [L397](../src/core/RemoteInputTelemetry.h#L397) | `coalescedMouseMoves_` | `std::atomic<std::uint64_t> coalescedMouseMoves_{0};` | 保存 coalesced mouse moves 相关配置或运行状态。 |
| [L398](../src/core/RemoteInputTelemetry.h#L398) | `generatedMouseButtons_` | `std::atomic<std::uint64_t> generatedMouseButtons_{0};` | 保存 generated mouse buttons 相关配置或运行状态。 |
| [L399](../src/core/RemoteInputTelemetry.h#L399) | `generatedMouseWheels_` | `std::atomic<std::uint64_t> generatedMouseWheels_{0};` | 保存 generated mouse wheels 相关配置或运行状态。 |
| [L400](../src/core/RemoteInputTelemetry.h#L400) | `generatedKeys_` | `std::atomic<std::uint64_t> generatedKeys_{0};` | 保存 generated keys 相关配置或运行状态。 |
| [L401](../src/core/RemoteInputTelemetry.h#L401) | `fastPacketsQueued_` | `std::atomic<std::uint64_t> fastPacketsQueued_{0};` | 保存 fast packets queued 相关配置或运行状态。 |
| [L402](../src/core/RemoteInputTelemetry.h#L402) | `reliablePacketsQueued_` | `std::atomic<std::uint64_t> reliablePacketsQueued_{0};` | 保存 reliable packets queued 相关配置或运行状态。 |
| [L403](../src/core/RemoteInputTelemetry.h#L403) | `packetsSent_` | `std::atomic<std::uint64_t> packetsSent_{0};` | 保存 packets sent 相关配置或运行状态。 |
| [L404](../src/core/RemoteInputTelemetry.h#L404) | `packetsSendFailed_` | `std::atomic<std::uint64_t> packetsSendFailed_{0};` | 保存 packets send failed 相关配置或运行状态。 |
| [L405](../src/core/RemoteInputTelemetry.h#L405) | `packetsReceived_` | `std::atomic<std::uint64_t> packetsReceived_{0};` | 保存 packets received 相关配置或运行状态。 |
| [L406](../src/core/RemoteInputTelemetry.h#L406) | `packetsDropped_` | `std::atomic<std::uint64_t> packetsDropped_{0};` | 保存 packets dropped 相关配置或运行状态。 |
| [L407](../src/core/RemoteInputTelemetry.h#L407) | `injectedMouseMoves_` | `std::atomic<std::uint64_t> injectedMouseMoves_{0};` | 保存 injected mouse moves 相关配置或运行状态。 |
| [L408](../src/core/RemoteInputTelemetry.h#L408) | `injectedMouseButtons_` | `std::atomic<std::uint64_t> injectedMouseButtons_{0};` | 保存 injected mouse buttons 相关配置或运行状态。 |
| [L409](../src/core/RemoteInputTelemetry.h#L409) | `injectedMouseWheels_` | `std::atomic<std::uint64_t> injectedMouseWheels_{0};` | 保存 injected mouse wheels 相关配置或运行状态。 |
| [L410](../src/core/RemoteInputTelemetry.h#L410) | `injectedKeys_` | `std::atomic<std::uint64_t> injectedKeys_{0};` | 保存 injected keys 相关配置或运行状态。 |
| [L411](../src/core/RemoteInputTelemetry.h#L411) | `injectionFailures_` | `std::atomic<std::uint64_t> injectionFailures_{0};` | 保存 injection failures 相关配置或运行状态。 |
| [L412](../src/core/RemoteInputTelemetry.h#L412) | `lastMoveDispatchAtUs_` | `std::atomic<std::uint64_t> lastMoveDispatchAtUs_{0};` | 保存 last move dispatch at us 相关配置或运行状态。 |
| [L413](../src/core/RemoteInputTelemetry.h#L413) | `moveDispatchIntervalSamples_` | `std::atomic<std::uint64_t> moveDispatchIntervalSamples_{0};` | 保存 move dispatch interval samples 相关配置或运行状态。 |
| [L414](../src/core/RemoteInputTelemetry.h#L414) | `moveDispatchIntervalSumUs_` | `std::atomic<std::uint64_t> moveDispatchIntervalSumUs_{0};` | 保存 move dispatch interval sum us 相关配置或运行状态。 |
| [L415](../src/core/RemoteInputTelemetry.h#L415) | `moveDispatchIntervalMaximumUs_` | `std::atomic<std::uint64_t> moveDispatchIntervalMaximumUs_{0};` | 保存 move dispatch interval maximum us 相关配置或运行状态。 |
| [L419](../src/core/RemoteInputTelemetry.h#L419) | `moveDispatchIntervalHistogram_` | `moveDispatchIntervalHistogram_{};` | 保存 move dispatch interval histogram 相关配置或运行状态。 |
| [L420](../src/core/RemoteInputTelemetry.h#L420) | `sampleMutex_` | `std::mutex sampleMutex_;` | 保护跨线程共享状态：sample mutex。 |
| [L421](../src/core/RemoteInputTelemetry.h#L421) | `previousSample_` | `RemoteInputTelemetryCounters previousSample_;` | 保存 previous sample 相关配置或运行状态。 |

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L53](../src/core/RemoteInputTelemetry.h#L53) | `Instance` | 定义 | `static RemoteInputTelemetry& Instance()` | 实现 instance 对应的业务或工具逻辑。 |
| [L59](../src/core/RemoteInputTelemetry.h#L59) | `SetEnabled` | 定义 | `void SetEnabled(bool enabled)` | 更新或应用 set enabled 相关逻辑。 |
| [L71](../src/core/RemoteInputTelemetry.h#L71) | `Enabled` | 定义 | `bool Enabled() const` | 实现 enabled 对应的业务或工具逻辑。 |
| [L76](../src/core/RemoteInputTelemetry.h#L76) | `SetMoveDispatchRateLimit` | 定义 | `void SetMoveDispatchRateLimit(std::uint32_t hertz)` | 更新或应用 set move dispatch rate limit 相关逻辑。 |
| [L81](../src/core/RemoteInputTelemetry.h#L81) | `RecordGeneratedMouseMove` | 定义 | `void RecordGeneratedMouseMove()` | 实现 record generated mouse move 对应的业务或工具逻辑。 |
| [L86](../src/core/RemoteInputTelemetry.h#L86) | `RecordDispatchedMouseMove` | 定义 | `void RecordDispatchedMouseMove()` | 实现 record dispatched mouse move 对应的业务或工具逻辑。 |
| [L129](../src/core/RemoteInputTelemetry.h#L129) | `RecordCoalescedMouseMove` | 定义 | `void RecordCoalescedMouseMove()` | 实现 record coalesced mouse move 对应的业务或工具逻辑。 |
| [L134](../src/core/RemoteInputTelemetry.h#L134) | `RecordGeneratedMouseButton` | 定义 | `void RecordGeneratedMouseButton()` | 实现 record generated mouse button 对应的业务或工具逻辑。 |
| [L139](../src/core/RemoteInputTelemetry.h#L139) | `RecordGeneratedMouseWheel` | 定义 | `void RecordGeneratedMouseWheel()` | 实现 record generated mouse wheel 对应的业务或工具逻辑。 |
| [L144](../src/core/RemoteInputTelemetry.h#L144) | `RecordGeneratedKey` | 定义 | `void RecordGeneratedKey()` | 实现 record generated key 对应的业务或工具逻辑。 |
| [L149](../src/core/RemoteInputTelemetry.h#L149) | `RecordPacketQueued` | 定义 | `void RecordPacketQueued(bool fastChannel)` | 实现 record packet queued 对应的业务或工具逻辑。 |
| [L155](../src/core/RemoteInputTelemetry.h#L155) | `RecordPacketSendResult` | 定义 | `void RecordPacketSendResult(bool succeeded)` | 实现 record packet send result 对应的业务或工具逻辑。 |
| [L160](../src/core/RemoteInputTelemetry.h#L160) | `RecordPacketReceived` | 定义 | `void RecordPacketReceived()` | 实现 record packet received 对应的业务或工具逻辑。 |
| [L165](../src/core/RemoteInputTelemetry.h#L165) | `RecordPacketDropped` | 定义 | `void RecordPacketDropped()` | 实现 record packet dropped 对应的业务或工具逻辑。 |
| [L177](../src/core/RemoteInputTelemetry.h#L177) | `RecordInjection` | 定义 | `void RecordInjection(InjectionType type, bool succeeded)` | 实现 record injection 对应的业务或工具逻辑。 |
| [L201](../src/core/RemoteInputTelemetry.h#L201) | `TakeSnapshot` | 定义 | `RemoteInputTelemetrySnapshot TakeSnapshot()` | 实现 take snapshot 对应的业务或工具逻辑。 |
| [L265](../src/core/RemoteInputTelemetry.h#L265) | `RemoteInputTelemetry` | 声明 | `RemoteInputTelemetry() = default` | 实现 remote input telemetry 对应的业务或工具逻辑。 |
| [L277](../src/core/RemoteInputTelemetry.h#L277) | `fetch_add` | 声明 | `counter.fetch_add(1, std::memory_order_relaxed)` | 实现 fetch add 对应的业务或工具逻辑。 |
| [L281](../src/core/RemoteInputTelemetry.h#L281) | `LoadCounters` | 定义 | `RemoteInputTelemetryCounters LoadCounters() const` | 读取或恢复 load counters 相关逻辑。 |
| [L321](../src/core/RemoteInputTelemetry.h#L321) | `PerSecond` | 定义 | `static std::uint64_t PerSecond(std::uint64_t current, std::uint64_t previous, std::int64_t elapsedMs)` | 实现 per second 对应的业务或工具逻辑。 |
| [L332](../src/core/RemoteInputTelemetry.h#L332) | `Rate` | 定义 | `static RemoteInputTelemetryCounters Rate( const RemoteInputTelemetryCounters& current, const RemoteInputTelemetryCounters& previous, std::int64_t elapsedMs)` | 实现 rate 对应的业务或工具逻辑。 |
| [L361](../src/core/RemoteInputTelemetry.h#L361) | `ResetCounters` | 定义 | `void ResetCounters()` | 重置或移除 reset counters 相关逻辑。 |

## `src/core/RoomState.h`

[打开源码](../src/core/RoomState.h) · **文件作用：** 声明 room state 相关类型、接口、配置和成员状态。

### 类型

| 行 | 类型 | 种类 | 作用 |
|---:|---|---|---|
| [L18](../src/core/RoomState.h#L18) | `RoomMembershipState` | enum class | 定义 RoomMembershipState 的 enum class 类型和相关状态。 |
| [L27](../src/core/RoomState.h#L27) | `RoomScreenShareState` | enum class | 定义 RoomScreenShareState 的 enum class 类型和相关状态。 |
| [L34](../src/core/RoomState.h#L34) | `RoomPeerConnectionState` | enum class | 定义 RoomPeerConnectionState 的 enum class 类型和相关状态。 |
| [L45](../src/core/RoomState.h#L45) | `RoomAvailabilityState` | enum class | 定义 RoomAvailabilityState 的 enum class 类型和相关状态。 |
| [L52](../src/core/RoomState.h#L52) | `RoomMemberAction` | enum class | 定义 RoomMemberAction 的 enum class 类型和相关状态。 |
| [L58](../src/core/RoomState.h#L58) | `RoomAvailabilitySnapshot` | struct | 定义 RoomAvailabilitySnapshot 的 struct 类型和相关状态。 |
| [L63](../src/core/RoomState.h#L63) | `RoomMemberSnapshot` | struct | 定义 RoomMemberSnapshot 的 struct 类型和相关状态。 |
| [L71](../src/core/RoomState.h#L71) | `RoomSnapshot` | struct | 定义 RoomSnapshot 的 struct 类型和相关状态。 |
| [L87](../src/core/RoomState.h#L87) | `RoomJoinRequest` | struct | 定义 RoomJoinRequest 的 struct 类型和相关状态。 |
| [L94](../src/core/RoomState.h#L94) | `RoomControlRequest` | struct | 定义 RoomControlRequest 的 struct 类型和相关状态。 |
| [L101](../src/core/RoomState.h#L101) | `RoomScreenShareSwitchRequest` | struct | 定义 RoomScreenShareSwitchRequest 的 struct 类型和相关状态。 |
| [L108](../src/core/RoomState.h#L108) | `RoomScreenShareViewRequest` | struct | 定义 RoomScreenShareViewRequest 的 struct 类型和相关状态。 |
| [L115](../src/core/RoomState.h#L115) | `RoomMemberActionResult` | struct | 定义 RoomMemberActionResult 的 struct 类型和相关状态。 |
| [L124](../src/core/RoomState.h#L124) | `RoomPeerConnectionSnapshot` | struct | 定义 RoomPeerConnectionSnapshot 的 struct 类型和相关状态。 |

### 成员与文件级变量

| 行 | 变量 | 声明 | 作用 |
|---:|---|---|---|
| [L14](../src/core/RoomState.h#L14) | `kMinimumRoomMembers` | `inline constexpr std::uint32_t kMinimumRoomMembers = 2;` | 定义 minimum room members 的编译期常量或产品边界。 |
| [L15](../src/core/RoomState.h#L15) | `kDefaultRoomCapacity` | `inline constexpr std::uint32_t kDefaultRoomCapacity = 2;` | 定义 default room capacity 的编译期常量或产品边界。 |
| [L16](../src/core/RoomState.h#L16) | `kProtocolMaximumRoomMembers` | `inline constexpr std::uint32_t kProtocolMaximumRoomMembers = 5;` | 定义 protocol maximum room members 的编译期常量或产品边界。 |
| [L59](../src/core/RoomState.h#L59) | `roomId` | `std::string roomId;` | 保存身份或作用域标识：room id。 |
| [L60](../src/core/RoomState.h#L60) | `state` | `RoomAvailabilityState state = RoomAvailabilityState::kChecking;` | 保存状态机当前状态：state。 |
| [L64](../src/core/RoomState.h#L64) | `deviceId` | `std::string deviceId;` | 保存身份或作用域标识：device id。 |
| [L65](../src/core/RoomState.h#L65) | `deviceName` | `std::string deviceName;` | 保存路径、地址或显示名称：device name。 |
| [L66](../src/core/RoomState.h#L66) | `online` | `bool online = false;` | 保存 online 相关配置或运行状态。 |
| [L67](../src/core/RoomState.h#L67) | `cameraPublishing` | `bool cameraPublishing = false;` | 保存 camera publishing 相关配置或运行状态。 |
| [L68](../src/core/RoomState.h#L68) | `microphonePublishing` | `bool microphonePublishing = false;` | 保存 microphone publishing 相关配置或运行状态。 |
| [L72](../src/core/RoomState.h#L72) | `membership` | `RoomMembershipState membership = RoomMembershipState::kNone;` | 保存 membership 相关配置或运行状态。 |
| [L73](../src/core/RoomState.h#L73) | `roomId` | `std::string roomId;` | 保存身份或作用域标识：room id。 |
| [L74](../src/core/RoomState.h#L74) | `ownerDeviceId` | `std::string ownerDeviceId;` | 保存身份或作用域标识：owner device id。 |
| [L75](../src/core/RoomState.h#L75) | `capacity` | `std::uint32_t capacity = kDefaultRoomCapacity;` | 保存 capacity 相关配置或运行状态。 |
| [L76](../src/core/RoomState.h#L76) | `screenShareState` | `RoomScreenShareState screenShareState = RoomScreenShareState::kIdle;` | 保存状态机当前状态：screen share state。 |
| [L77](../src/core/RoomState.h#L77) | `screenSharerDeviceId` | `std::string screenSharerDeviceId;` | 保存身份或作用域标识：screen sharer device id。 |
| [L78](../src/core/RoomState.h#L78) | `pendingScreenSharerDeviceId` | `std::string pendingScreenSharerDeviceId;` | 保存身份或作用域标识：pending screen sharer device id。 |
| [L79](../src/core/RoomState.h#L79) | `screenShareEpoch` | `std::uint64_t screenShareEpoch = 0;` | 标记当前世代，用于拒绝过期异步结果：screen share epoch。 |
| [L80](../src/core/RoomState.h#L80) | `pendingControllerDeviceId` | `std::string pendingControllerDeviceId;` | 保存身份或作用域标识：pending controller device id。 |
| [L81](../src/core/RoomState.h#L81) | `activeControllerDeviceId` | `std::string activeControllerDeviceId;` | 保存身份或作用域标识：active controller device id。 |
| [L82](../src/core/RoomState.h#L82) | `members` | `std::vector<RoomMemberSnapshot> members;` | 保存 members 相关配置或运行状态。 |
| [L83](../src/core/RoomState.h#L83) | `errorCode` | `std::string errorCode;` | 保存 error code 相关配置或运行状态。 |
| [L84](../src/core/RoomState.h#L84) | `errorMessage` | `std::string errorMessage;` | 保存 error message 相关配置或运行状态。 |
| [L88](../src/core/RoomState.h#L88) | `roomId` | `std::string roomId;` | 保存身份或作用域标识：room id。 |
| [L89](../src/core/RoomState.h#L89) | `requestId` | `std::string requestId;` | 保存身份或作用域标识：request id。 |
| [L90](../src/core/RoomState.h#L90) | `requesterDeviceId` | `std::string requesterDeviceId;` | 保存身份或作用域标识：requester device id。 |
| [L91](../src/core/RoomState.h#L91) | `requesterDeviceName` | `std::string requesterDeviceName;` | 保存路径、地址或显示名称：requester device name。 |
| [L95](../src/core/RoomState.h#L95) | `roomId` | `std::string roomId;` | 保存身份或作用域标识：room id。 |
| [L96](../src/core/RoomState.h#L96) | `requestId` | `std::string requestId;` | 保存身份或作用域标识：request id。 |
| [L97](../src/core/RoomState.h#L97) | `requesterDeviceId` | `std::string requesterDeviceId;` | 保存身份或作用域标识：requester device id。 |
| [L98](../src/core/RoomState.h#L98) | `requesterDeviceName` | `std::string requesterDeviceName;` | 保存路径、地址或显示名称：requester device name。 |
| [L102](../src/core/RoomState.h#L102) | `roomId` | `std::string roomId;` | 保存身份或作用域标识：room id。 |
| [L103](../src/core/RoomState.h#L103) | `requestId` | `std::string requestId;` | 保存身份或作用域标识：request id。 |
| [L104](../src/core/RoomState.h#L104) | `requesterDeviceId` | `std::string requesterDeviceId;` | 保存身份或作用域标识：requester device id。 |
| [L105](../src/core/RoomState.h#L105) | `requesterDeviceName` | `std::string requesterDeviceName;` | 保存路径、地址或显示名称：requester device name。 |
| [L109](../src/core/RoomState.h#L109) | `roomId` | `std::string roomId;` | 保存身份或作用域标识：room id。 |
| [L110](../src/core/RoomState.h#L110) | `sequence` | `std::uint64_t sequence = 0;` | 保存单调序号，用于排序或去重：sequence。 |
| [L111](../src/core/RoomState.h#L111) | `requesterDeviceId` | `std::string requesterDeviceId;` | 保存身份或作用域标识：requester device id。 |
| [L112](../src/core/RoomState.h#L112) | `requesterDeviceName` | `std::string requesterDeviceName;` | 保存路径、地址或显示名称：requester device name。 |
| [L116](../src/core/RoomState.h#L116) | `roomId` | `std::string roomId;` | 保存身份或作用域标识：room id。 |
| [L117](../src/core/RoomState.h#L117) | `sequence` | `std::uint64_t sequence = 0;` | 保存单调序号，用于排序或去重：sequence。 |
| [L118](../src/core/RoomState.h#L118) | `peerDeviceId` | `std::string peerDeviceId;` | 保存身份或作用域标识：peer device id。 |
| [L119](../src/core/RoomState.h#L119) | `action` | `RoomMemberAction action = RoomMemberAction::kRequestScreenShare;` | 保存 action 相关配置或运行状态。 |
| [L120](../src/core/RoomState.h#L120) | `accepted` | `bool accepted = false;` | 保存 accepted 相关配置或运行状态。 |
| [L121](../src/core/RoomState.h#L121) | `error` | `std::string error;` | 保存最近错误或失败原因：error。 |
| [L125](../src/core/RoomState.h#L125) | `pairId` | `std::string pairId;` | 保存身份或作用域标识：pair id。 |
| [L126](../src/core/RoomState.h#L126) | `peerDeviceId` | `std::string peerDeviceId;` | 保存身份或作用域标识：peer device id。 |
| [L127](../src/core/RoomState.h#L127) | `localIsOfferer` | `bool localIsOfferer = false;` | 保存 local is offerer 相关配置或运行状态。 |
| [L128](../src/core/RoomState.h#L128) | `state` | `RoomPeerConnectionState state = RoomPeerConnectionState::kStarting;` | 保存状态机当前状态：state。 |
| [L129](../src/core/RoomState.h#L129) | `openDataChannelCount` | `std::uint32_t openDataChannelCount = 0;` | 保存计数、尺寸或速率指标：open data channel count。 |
| [L133](../src/core/RoomState.h#L133) | `controlReliableChannelOpen` | `bool controlReliableChannelOpen = false;` | Input readiness is tracked per channel instead of inferred from the aggregate count. Clipboard channels can already be open while the low-latency pointer channel is still negoti... |
| [L134](../src/core/RoomState.h#L134) | `inputFastChannelOpen` | `bool inputFastChannelOpen = false;` | 保存能力或开关状态：input fast channel open。 |
| [L135](../src/core/RoomState.h#L135) | `clipboardReliableChannelOpen` | `bool clipboardReliableChannelOpen = false;` | 保存能力或开关状态：clipboard reliable channel open。 |
| [L136](../src/core/RoomState.h#L136) | `clipboardTransferChannelOpen` | `bool clipboardTransferChannelOpen = false;` | 保存能力或开关状态：clipboard transfer channel open。 |
| [L137](../src/core/RoomState.h#L137) | `preparedVideoSlotCount` | `std::uint32_t preparedVideoSlotCount = 0;` | 保存计数、尺寸或速率指标：prepared video slot count。 |
| [L138](../src/core/RoomState.h#L138) | `screenPreferencePending` | `bool screenPreferencePending = false;` | 保存待处理队列或请求：screen preference pending。 |
| [L139](../src/core/RoomState.h#L139) | `screenPreferenceSequence` | `std::uint64_t screenPreferenceSequence = 0;` | 保存单调序号，用于排序或去重：screen preference sequence。 |
| [L142](../src/core/RoomState.h#L142) | `screenPreferenceGeneration` | `std::uint64_t screenPreferenceGeneration = 0;` | Epoch for which screenPreferenceSequence was sent and acknowledged. Width/FPS values from an older share must never satisfy a new share. |
| [L146](../src/core/RoomState.h#L146) | `screenFirstFramePresentedGeneration` | `std::uint64_t screenFirstFramePresentedGeneration = 0;` | Startup telemetry is generation-bound. A non-zero presented generation means that the first frame reached the viewer's actual presentation path, not merely the decoder callback. |
| [L147](../src/core/RoomState.h#L147) | `screenFirstFrameStartupMs` | `std::uint32_t screenFirstFrameStartupMs = 0;` | 保存 screen first frame startup ms 相关配置或运行状态。 |
| [L148](../src/core/RoomState.h#L148) | `screenStartupRefreshRequests` | `std::uint32_t screenStartupRefreshRequests = 0;` | 保存 screen startup refresh requests 相关配置或运行状态。 |
| [L149](../src/core/RoomState.h#L149) | `screenWidth` | `std::uint32_t screenWidth = 0;` | 保存计数、尺寸或速率指标：screen width。 |
| [L150](../src/core/RoomState.h#L150) | `screenHeight` | `std::uint32_t screenHeight = 0;` | 保存计数、尺寸或速率指标：screen height。 |
| [L153](../src/core/RoomState.h#L153) | `screenFramesPerSecond` | `std::uint32_t screenFramesPerSecond = 60;` | A prepared screen sender starts at the product's 60 FPS default until a viewer preference is acknowledged. |
| [L156](../src/core/RoomState.h#L156) | `screenMaximumFrameRate` | `std::uint32_t screenMaximumFrameRate = 120;` | Both normal capture implementations support up to 120 FPS. A runtime fallback may report a lower ceiling after capture starts. |
| [L157](../src/core/RoomState.h#L157) | `screenCaptureCapabilityReported` | `bool screenCaptureCapabilityReported = false;` | 保存 screen capture capability reported 相关配置或运行状态。 |
| [L158](../src/core/RoomState.h#L158) | `screenCaptureConfiguredBackend` | `std::string screenCaptureConfiguredBackend;` | 保存 screen capture configured backend 相关配置或运行状态。 |
| [L159](../src/core/RoomState.h#L159) | `screenCaptureActiveBackend` | `std::string screenCaptureActiveBackend;` | 保存 screen capture active backend 相关配置或运行状态。 |
| [L160](../src/core/RoomState.h#L160) | `screenCaptureFallbackReason` | `std::string screenCaptureFallbackReason;` | 保存 screen capture fallback reason 相关配置或运行状态。 |
| [L161](../src/core/RoomState.h#L161) | `screenMaxBitrateBps` | `std::uint32_t screenMaxBitrateBps = 0;` | 保存计数、尺寸或速率指标：screen max bitrate bps。 |
| [L162](../src/core/RoomState.h#L162) | `screenScaleBackend` | `std::uint8_t screenScaleBackend = 0;` | 保存 screen scale backend 相关配置或运行状态。 |
| [L165](../src/core/RoomState.h#L165) | `sharedDisplayLayoutVersion` | `std::uint64_t sharedDisplayLayoutVersion = 0;` | Identity of the exact display currently carried by screen-main. Remote pointer input stays disabled until this reliable metadata arrives. |
| [L166](../src/core/RoomState.h#L166) | `sharedDisplayId` | `std::uint32_t sharedDisplayId = 0;` | 保存身份或作用域标识：shared display id。 |
| [L167](../src/core/RoomState.h#L167) | `sharedDisplayName` | `std::string sharedDisplayName;` | 保存路径、地址或显示名称：shared display name。 |
| [L168](../src/core/RoomState.h#L168) | `sharedDisplayStableKey` | `std::string sharedDisplayStableKey;` | 保存 shared display stable key 相关配置或运行状态。 |
| [L169](../src/core/RoomState.h#L169) | `sharedDisplayWidth` | `std::uint32_t sharedDisplayWidth = 0;` | 保存计数、尺寸或速率指标：shared display width。 |
| [L170](../src/core/RoomState.h#L170) | `sharedDisplayHeight` | `std::uint32_t sharedDisplayHeight = 0;` | 保存计数、尺寸或速率指标：shared display height。 |
| [L171](../src/core/RoomState.h#L171) | `remoteDisplayCatalogLayoutVersion` | `std::uint64_t remoteDisplayCatalogLayoutVersion = 0;` | 保存 remote display catalog layout version 相关配置或运行状态。 |
| [L172](../src/core/RoomState.h#L172) | `remoteDisplayCatalogReported` | `bool remoteDisplayCatalogReported = false;` | 保存 remote display catalog reported 相关配置或运行状态。 |
| [L173](../src/core/RoomState.h#L173) | `remoteDisplays` | `std::vector<DisplayDescriptor> remoteDisplays;` | 保存 remote displays 相关配置或运行状态。 |
| [L174](../src/core/RoomState.h#L174) | `remoteDisplaySwitchPending` | `bool remoteDisplaySwitchPending = false;` | 保存待处理队列或请求：remote display switch pending。 |
| [L175](../src/core/RoomState.h#L175) | `remoteDisplaySwitchSequence` | `std::uint64_t remoteDisplaySwitchSequence = 0;` | 保存单调序号，用于排序或去重：remote display switch sequence。 |
| [L176](../src/core/RoomState.h#L176) | `remoteDisplaySwitchError` | `std::string remoteDisplaySwitchError;` | 保存最近错误或失败原因：remote display switch error。 |
| [L177](../src/core/RoomState.h#L177) | `negotiationGeneration` | `std::uint64_t negotiationGeneration = 1;` | 标记当前世代，用于拒绝过期异步结果：negotiation generation。 |
| [L178](../src/core/RoomState.h#L178) | `iceRestartAttempt` | `std::uint32_t iceRestartAttempt = 0;` | 保存 ice restart attempt 相关配置或运行状态。 |
| [L179](../src/core/RoomState.h#L179) | `errorCode` | `std::string errorCode;` | 保存 error code 相关配置或运行状态。 |
| [L180](../src/core/RoomState.h#L180) | `errorMessage` | `std::string errorMessage;` | 保存 error message 相关配置或运行状态。 |

## `src/core/ScreenNetworkPolicy.cpp`

[打开源码](../src/core/ScreenNetworkPolicy.cpp) · **文件作用：** 实现 screen network policy 相关函数与文件级辅助逻辑。

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L16](../src/core/ScreenNetworkPolicy.cpp#L16) | `BlendUnsigned` | 定义 | `std::uint64_t BlendUnsigned( std::uint64_t previous, std::uint64_t current, double alpha)` | 实现 blend unsigned 对应的业务或工具逻辑。 |
| [L34](../src/core/ScreenNetworkPolicy.cpp#L34) | `BlendDouble` | 定义 | `double BlendDouble(double previous, double current, double alpha)` | 实现 blend double 对应的业务或工具逻辑。 |
| [L44](../src/core/ScreenNetworkPolicy.cpp#L44) | `HasNewDiscardedPackets` | 定义 | `bool HasNewDiscardedPackets( ProgressiveBitrateCeilingState* state, std::uint64_t packetsDiscardedOnSend)` | 判断 has new discarded packets 相关逻辑。 |
| [L59](../src/core/ScreenNetworkPolicy.cpp#L59) | `RttRegressed` | 定义 | `bool RttRegressed( double currentRttMs, double baselineRttMs, const ProgressiveBitrateCeilingConfig& config)` | 实现 rtt regressed 对应的业务或工具逻辑。 |
| [L76](../src/core/ScreenNetworkPolicy.cpp#L76) | `RequiredStartBitrate` | 定义 | `std::uint64_t RequiredStartBitrate( std::uint32_t width, std::uint32_t height, std::uint32_t framesPerSecond)` | 实现 required start bitrate 对应的业务或工具逻辑。 |
| [L87](../src/core/ScreenNetworkPolicy.cpp#L87) | `HighestSupportedFrameRate` | 定义 | `std::uint32_t HighestSupportedFrameRate( const AdaptiveScreenFrameRateState& state, std::uint64_t usableCapacityBps)` | 实现 highest supported frame rate 对应的业务或工具逻辑。 |
| [L113](../src/core/ScreenNetworkPolicy.cpp#L113) | `NextRecoveryFrameRate` | 定义 | `std::uint32_t NextRecoveryFrameRate( const AdaptiveScreenFrameRateState& state, std::uint32_t supportedFrameRate)` | 实现 next recovery frame rate 对应的业务或工具逻辑。 |
| [L134](../src/core/ScreenNetworkPolicy.cpp#L134) | `ResetProgressiveBitrateCeiling` | 定义 | `void ResetProgressiveBitrateCeiling( ProgressiveBitrateCeilingState* state, bool enabled, std::uint64_t desiredMaxBitrateBps, std::uint64_t appliedMaxBitrateBps, std::uint64_t timestampMs)` | 重置或移除 reset progressive bitrate ceiling 相关逻辑。 |
| [L155](../src/core/ScreenNetworkPolicy.cpp#L155) | `RetargetProgressiveBitrateCeiling` | 定义 | `ProgressiveBitrateCeilingDecision RetargetProgressiveBitrateCeiling( ProgressiveBitrateCeilingState* state, std::uint64_t desiredMaxBitrateBps, std::uint64_t timestampMs)` | 实现 retarget progressive bitrate ceiling 对应的业务或工具逻辑。 |
| [L193](../src/core/ScreenNetworkPolicy.cpp#L193) | `EvaluateProgressiveBitrateCeiling` | 定义 | `ProgressiveBitrateCeilingDecision EvaluateProgressiveBitrateCeiling( ProgressiveBitrateCeilingState* state, const ProgressiveBitrateCeilingSample& sample, const ProgressiveBitrateCeilingConfig& config)` | 实现 evaluate progressive bitrate ceiling 对应的业务或工具逻辑。 |
| [L334](../src/core/ScreenNetworkPolicy.cpp#L334) | `ResetAdaptiveScreenFrameRate` | 定义 | `void ResetAdaptiveScreenFrameRate( AdaptiveScreenFrameRateState* state, bool enabled, std::uint32_t requestedFrameRate, std::uint32_t outputWidth, std::uint32_t outputHeight, std::uint64_t timestampMs)` | 重置或移除 reset adaptive screen frame rate 相关逻辑。 |
| [L358](../src/core/ScreenNetworkPolicy.cpp#L358) | `RetargetAdaptiveScreenFrameRate` | 定义 | `AdaptiveScreenFrameRateDecision RetargetAdaptiveScreenFrameRate( AdaptiveScreenFrameRateState* state, std::uint32_t requestedFrameRate, std::uint32_t outputWidth, std::uint32_t outputHeight, std::uint64_t timestampMs)` | 实现 retarget adaptive screen frame rate 对应的业务或工具逻辑。 |
| [L392](../src/core/ScreenNetworkPolicy.cpp#L392) | `EvaluateAdaptiveScreenFrameRate` | 定义 | `AdaptiveScreenFrameRateDecision EvaluateAdaptiveScreenFrameRate( AdaptiveScreenFrameRateState* state, const AdaptiveScreenFrameRateSample& sample, const AdaptiveScreenFrameRateConfig& config)` | 实现 evaluate adaptive screen frame rate 对应的业务或工具逻辑。 |

## `src/core/ScreenNetworkPolicy.h`

[打开源码](../src/core/ScreenNetworkPolicy.h) · **文件作用：** 声明 screen network policy 相关类型、接口、配置和成员状态。

### 类型

| 行 | 类型 | 种类 | 作用 |
|---:|---|---|---|
| [L10](../src/core/ScreenNetworkPolicy.h#L10) | `ProgressiveBitrateCeilingStatus` | enum class | 定义 ProgressiveBitrateCeilingStatus 的 enum class 类型和相关状态。 |
| [L19](../src/core/ScreenNetworkPolicy.h#L19) | `ProgressiveBitrateCeilingConfig` | struct | 定义 ProgressiveBitrateCeilingConfig 的 struct 类型和相关状态。 |
| [L36](../src/core/ScreenNetworkPolicy.h#L36) | `ProgressiveBitrateCeilingSample` | struct | 定义 ProgressiveBitrateCeilingSample 的 struct 类型和相关状态。 |
| [L45](../src/core/ScreenNetworkPolicy.h#L45) | `ProgressiveBitrateCeilingState` | struct | 定义 ProgressiveBitrateCeilingState 的 struct 类型和相关状态。 |
| [L68](../src/core/ScreenNetworkPolicy.h#L68) | `ProgressiveBitrateCeilingDecision` | struct | 定义 ProgressiveBitrateCeilingDecision 的 struct 类型和相关状态。 |
| [L99](../src/core/ScreenNetworkPolicy.h#L99) | `ScreenContentActivity` | enum class | 定义 ScreenContentActivity 的 enum class 类型和相关状态。 |
| [L106](../src/core/ScreenNetworkPolicy.h#L106) | `AdaptiveScreenFrameRateStatus` | enum class | 定义 AdaptiveScreenFrameRateStatus 的 enum class 类型和相关状态。 |
| [L117](../src/core/ScreenNetworkPolicy.h#L117) | `AdaptiveScreenFrameRateConfig` | struct | 定义 AdaptiveScreenFrameRateConfig 的 struct 类型和相关状态。 |
| [L128](../src/core/ScreenNetworkPolicy.h#L128) | `AdaptiveScreenFrameRateSample` | struct | 定义 AdaptiveScreenFrameRateSample 的 struct 类型和相关状态。 |
| [L134](../src/core/ScreenNetworkPolicy.h#L134) | `AdaptiveScreenFrameRateState` | struct | 定义 AdaptiveScreenFrameRateState 的 struct 类型和相关状态。 |
| [L150](../src/core/ScreenNetworkPolicy.h#L150) | `AdaptiveScreenFrameRateDecision` | struct | 定义 AdaptiveScreenFrameRateDecision 的 struct 类型和相关状态。 |

### 成员与文件级变量

| 行 | 变量 | 声明 | 作用 |
|---:|---|---|---|
| [L20](../src/core/ScreenNetworkPolicy.h#L20) | `capacityEmaAlpha` | `double capacityEmaAlpha = 0.25;` | 保存 capacity ema alpha 相关配置或运行状态。 |
| [L21](../src/core/ScreenNetworkPolicy.h#L21) | `growthFactor` | `double growthFactor = 1.5;` | 保存 growth factor 相关配置或运行状态。 |
| [L22](../src/core/ScreenNetworkPolicy.h#L22) | `capacityUtilizationThreshold` | `double capacityUtilizationThreshold = 0.75;` | 保存 capacity utilization threshold 相关配置或运行状态。 |
| [L23](../src/core/ScreenNetworkPolicy.h#L23) | `maximumLossPercent` | `double maximumLossPercent = 2.0;` | 保存 maximum loss percent 相关配置或运行状态。 |
| [L24](../src/core/ScreenNetworkPolicy.h#L24) | `probeFailureCapacityRatio` | `double probeFailureCapacityRatio = 0.65;` | 保存 probe failure capacity ratio 相关配置或运行状态。 |
| [L25](../src/core/ScreenNetworkPolicy.h#L25) | `probeFailureRttRatio` | `double probeFailureRttRatio = 1.5;` | 保存 probe failure rtt ratio 相关配置或运行状态。 |
| [L26](../src/core/ScreenNetworkPolicy.h#L26) | `probeFailureRttIncreaseMs` | `double probeFailureRttIncreaseMs = 50.0;` | 保存 probe failure rtt increase ms 相关配置或运行状态。 |
| [L27](../src/core/ScreenNetworkPolicy.h#L27) | `requiredStableSamples` | `std::uint32_t requiredStableSamples = 3;` | 保存 required stable samples 相关配置或运行状态。 |
| [L28](../src/core/ScreenNetworkPolicy.h#L28) | `requiredFailureSamples` | `std::uint32_t requiredFailureSamples = 2;` | 保存 required failure samples 相关配置或运行状态。 |
| [L29](../src/core/ScreenNetworkPolicy.h#L29) | `minimumIncreaseIntervalMs` | `std::uint64_t minimumIncreaseIntervalMs = 5'000;` | 保存 minimum increase interval ms 相关配置或运行状态。 |
| [L30](../src/core/ScreenNetworkPolicy.h#L30) | `probeEvaluationDelayMs` | `std::uint64_t probeEvaluationDelayMs = 2'000;` | 保存 probe evaluation delay ms 相关配置或运行状态。 |
| [L31](../src/core/ScreenNetworkPolicy.h#L31) | `probeSuccessDelayMs` | `std::uint64_t probeSuccessDelayMs = 3'000;` | 保存 probe success delay ms 相关配置或运行状态。 |
| [L32](../src/core/ScreenNetworkPolicy.h#L32) | `failureCooldownMs` | `std::uint64_t failureCooldownMs = 10'000;` | 保存 failure cooldown ms 相关配置或运行状态。 |
| [L33](../src/core/ScreenNetworkPolicy.h#L33) | `minimumIncreaseBps` | `std::uint64_t minimumIncreaseBps = 250'000;` | 保存计数、尺寸或速率指标：minimum increase bps。 |
| [L37](../src/core/ScreenNetworkPolicy.h#L37) | `timestampMs` | `std::uint64_t timestampMs = 0;` | 保存 timestamp ms 相关配置或运行状态。 |
| [L38](../src/core/ScreenNetworkPolicy.h#L38) | `availableOutgoingBitrateBps` | `std::uint64_t availableOutgoingBitrateBps = 0;` | 保存计数、尺寸或速率指标：available outgoing bitrate bps。 |
| [L39](../src/core/ScreenNetworkPolicy.h#L39) | `targetBitrateBps` | `std::uint64_t targetBitrateBps = 0;` | 保存计数、尺寸或速率指标：target bitrate bps。 |
| [L40](../src/core/ScreenNetworkPolicy.h#L40) | `roundTripTimeMs` | `double roundTripTimeMs = 0.0;` | 保存 round trip time ms 相关配置或运行状态。 |
| [L41](../src/core/ScreenNetworkPolicy.h#L41) | `lossPercent` | `double lossPercent = 0.0;` | 保存 loss percent 相关配置或运行状态。 |
| [L42](../src/core/ScreenNetworkPolicy.h#L42) | `packetsDiscardedOnSend` | `std::uint64_t packetsDiscardedOnSend = 0;` | 保存 packets discarded on send 相关配置或运行状态。 |
| [L46](../src/core/ScreenNetworkPolicy.h#L46) | `enabled` | `bool enabled = false;` | 保存能力或开关状态：enabled。 |
| [L47](../src/core/ScreenNetworkPolicy.h#L47) | `desiredMaxBitrateBps` | `std::uint64_t desiredMaxBitrateBps = 0;` | 保存计数、尺寸或速率指标：desired max bitrate bps。 |
| [L48](../src/core/ScreenNetworkPolicy.h#L48) | `appliedMaxBitrateBps` | `std::uint64_t appliedMaxBitrateBps = 0;` | 保存计数、尺寸或速率指标：applied max bitrate bps。 |
| [L49](../src/core/ScreenNetworkPolicy.h#L49) | `previousValidatedMaxBitrateBps` | `std::uint64_t previousValidatedMaxBitrateBps = 0;` | 保存计数、尺寸或速率指标：previous validated max bitrate bps。 |
| [L50](../src/core/ScreenNetworkPolicy.h#L50) | `smoothedCapacityBps` | `std::uint64_t smoothedCapacityBps = 0;` | 保存计数、尺寸或速率指标：smoothed capacity bps。 |
| [L51](../src/core/ScreenNetworkPolicy.h#L51) | `smoothedRoundTripTimeMs` | `double smoothedRoundTripTimeMs = 0.0;` | 保存 smoothed round trip time ms 相关配置或运行状态。 |
| [L52](../src/core/ScreenNetworkPolicy.h#L52) | `stableSampleCount` | `std::uint32_t stableSampleCount = 0;` | 保存计数、尺寸或速率指标：stable sample count。 |
| [L53](../src/core/ScreenNetworkPolicy.h#L53) | `failureSampleCount` | `std::uint32_t failureSampleCount = 0;` | 保存计数、尺寸或速率指标：failure sample count。 |
| [L54](../src/core/ScreenNetworkPolicy.h#L54) | `lastSampleTimestampMs` | `std::uint64_t lastSampleTimestampMs = 0;` | 保存 last sample timestamp ms 相关配置或运行状态。 |
| [L55](../src/core/ScreenNetworkPolicy.h#L55) | `lastIncreaseTimestampMs` | `std::uint64_t lastIncreaseTimestampMs = 0;` | 保存 last increase timestamp ms 相关配置或运行状态。 |
| [L56](../src/core/ScreenNetworkPolicy.h#L56) | `cooldownUntilMs` | `std::uint64_t cooldownUntilMs = 0;` | 保存 cooldown until ms 相关配置或运行状态。 |
| [L57](../src/core/ScreenNetworkPolicy.h#L57) | `probeStartedTimestampMs` | `std::uint64_t probeStartedTimestampMs = 0;` | 保存 probe started timestamp ms 相关配置或运行状态。 |
| [L58](../src/core/ScreenNetworkPolicy.h#L58) | `probeBaselineCapacityBps` | `std::uint64_t probeBaselineCapacityBps = 0;` | 保存计数、尺寸或速率指标：probe baseline capacity bps。 |
| [L59](../src/core/ScreenNetworkPolicy.h#L59) | `probeBaselineRoundTripTimeMs` | `double probeBaselineRoundTripTimeMs = 0.0;` | 保存 probe baseline round trip time ms 相关配置或运行状态。 |
| [L60](../src/core/ScreenNetworkPolicy.h#L60) | `lastPacketsDiscardedOnSend` | `std::uint64_t lastPacketsDiscardedOnSend = 0;` | 保存 last packets discarded on send 相关配置或运行状态。 |
| [L61](../src/core/ScreenNetworkPolicy.h#L61) | `hasDiscardCounter` | `bool hasDiscardCounter = false;` | 保存 has discard counter 相关配置或运行状态。 |
| [L62](../src/core/ScreenNetworkPolicy.h#L62) | `probePending` | `bool probePending = false;` | 保存待处理队列或请求：probe pending。 |
| [L63](../src/core/ScreenNetworkPolicy.h#L63) | `lastProbeFailed` | `bool lastProbeFailed = false;` | 保存 last probe failed 相关配置或运行状态。 |
| [L65](../src/core/ScreenNetworkPolicy.h#L65) | `kDisabled` | `ProgressiveBitrateCeilingStatus::kDisabled;` | 定义 disabled 的编译期常量或产品边界。 |
| [L69](../src/core/ScreenNetworkPolicy.h#L69) | `applyPeerConnectionMax` | `bool applyPeerConnectionMax = false;` | 保存 apply peer connection max 相关配置或运行状态。 |
| [L70](../src/core/ScreenNetworkPolicy.h#L70) | `peerConnectionMaxBitrateBps` | `std::uint64_t peerConnectionMaxBitrateBps = 0;` | 保存计数、尺寸或速率指标：peer connection max bitrate bps。 |
| [L118](../src/core/ScreenNetworkPolicy.h#L118) | `capacityEmaAlpha` | `double capacityEmaAlpha = 0.25;` | 保存 capacity ema alpha 相关配置或运行状态。 |
| [L119](../src/core/ScreenNetworkPolicy.h#L119) | `capacitySafetyRatio` | `double capacitySafetyRatio = 0.85;` | 保存 capacity safety ratio 相关配置或运行状态。 |
| [L120](../src/core/ScreenNetworkPolicy.h#L120) | `recoveryHeadroomRatio` | `double recoveryHeadroomRatio = 1.15;` | 保存 recovery headroom ratio 相关配置或运行状态。 |
| [L121](../src/core/ScreenNetworkPolicy.h#L121) | `requiredReductionSamples` | `std::uint32_t requiredReductionSamples = 2;` | 保存 required reduction samples 相关配置或运行状态。 |
| [L122](../src/core/ScreenNetworkPolicy.h#L122) | `requiredRecoverySamples` | `std::uint32_t requiredRecoverySamples = 5;` | 保存 required recovery samples 相关配置或运行状态。 |
| [L123](../src/core/ScreenNetworkPolicy.h#L123) | `startupGraceMs` | `std::uint64_t startupGraceMs = 8'000;` | 保存 startup grace ms 相关配置或运行状态。 |
| [L124](../src/core/ScreenNetworkPolicy.h#L124) | `minimumReductionIntervalMs` | `std::uint64_t minimumReductionIntervalMs = 2'000;` | 保存 minimum reduction interval ms 相关配置或运行状态。 |
| [L125](../src/core/ScreenNetworkPolicy.h#L125) | `minimumRecoveryIntervalMs` | `std::uint64_t minimumRecoveryIntervalMs = 5'000;` | 保存 minimum recovery interval ms 相关配置或运行状态。 |
| [L129](../src/core/ScreenNetworkPolicy.h#L129) | `timestampMs` | `std::uint64_t timestampMs = 0;` | 保存 timestamp ms 相关配置或运行状态。 |
| [L130](../src/core/ScreenNetworkPolicy.h#L130) | `activity` | `ScreenContentActivity activity = ScreenContentActivity::kUnknown;` | 保存 activity 相关配置或运行状态。 |
| [L131](../src/core/ScreenNetworkPolicy.h#L131) | `capacityBps` | `std::uint64_t capacityBps = 0;` | 保存计数、尺寸或速率指标：capacity bps。 |
| [L135](../src/core/ScreenNetworkPolicy.h#L135) | `enabled` | `bool enabled = false;` | 保存能力或开关状态：enabled。 |
| [L136](../src/core/ScreenNetworkPolicy.h#L136) | `requestedFrameRate` | `std::uint32_t requestedFrameRate = 0;` | 保存计数、尺寸或速率指标：requested frame rate。 |
| [L137](../src/core/ScreenNetworkPolicy.h#L137) | `effectiveFrameRate` | `std::uint32_t effectiveFrameRate = 0;` | 保存计数、尺寸或速率指标：effective frame rate。 |
| [L138](../src/core/ScreenNetworkPolicy.h#L138) | `outputWidth` | `std::uint32_t outputWidth = 0;` | 保存计数、尺寸或速率指标：output width。 |
| [L139](../src/core/ScreenNetworkPolicy.h#L139) | `outputHeight` | `std::uint32_t outputHeight = 0;` | 保存计数、尺寸或速率指标：output height。 |
| [L140](../src/core/ScreenNetworkPolicy.h#L140) | `smoothedCapacityBps` | `std::uint64_t smoothedCapacityBps = 0;` | 保存计数、尺寸或速率指标：smoothed capacity bps。 |
| [L141](../src/core/ScreenNetworkPolicy.h#L141) | `reductionSampleCount` | `std::uint32_t reductionSampleCount = 0;` | 保存计数、尺寸或速率指标：reduction sample count。 |
| [L142](../src/core/ScreenNetworkPolicy.h#L142) | `recoverySampleCount` | `std::uint32_t recoverySampleCount = 0;` | 保存计数、尺寸或速率指标：recovery sample count。 |
| [L143](../src/core/ScreenNetworkPolicy.h#L143) | `lastSampleTimestampMs` | `std::uint64_t lastSampleTimestampMs = 0;` | 保存 last sample timestamp ms 相关配置或运行状态。 |
| [L144](../src/core/ScreenNetworkPolicy.h#L144) | `lastRetargetTimestampMs` | `std::uint64_t lastRetargetTimestampMs = 0;` | 保存 last retarget timestamp ms 相关配置或运行状态。 |
| [L145](../src/core/ScreenNetworkPolicy.h#L145) | `lastChangeTimestampMs` | `std::uint64_t lastChangeTimestampMs = 0;` | 保存 last change timestamp ms 相关配置或运行状态。 |
| [L147](../src/core/ScreenNetworkPolicy.h#L147) | `kDisabled` | `AdaptiveScreenFrameRateStatus::kDisabled;` | 定义 disabled 的编译期常量或产品边界。 |
| [L151](../src/core/ScreenNetworkPolicy.h#L151) | `applyEffectiveFrameRate` | `bool applyEffectiveFrameRate = false;` | 保存计数、尺寸或速率指标：apply effective frame rate。 |
| [L152](../src/core/ScreenNetworkPolicy.h#L152) | `effectiveFrameRate` | `std::uint32_t effectiveFrameRate = 0;` | 保存计数、尺寸或速率指标：effective frame rate。 |

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L76](../src/core/ScreenNetworkPolicy.h#L76) | `ResetProgressiveBitrateCeiling` | 声明 | `void ResetProgressiveBitrateCeiling( ProgressiveBitrateCeilingState* state, bool enabled, std::uint64_t desiredMaxBitrateBps, std::uint64_t appliedMaxBitrateBps, std::uint64_t timestampMs)` | Initializes the controller after the screen-share startup bitrate bootstrap has installed its first PeerConnection-wide maximum. The controller only raises that ceiling; it neve... |
| [L86](../src/core/ScreenNetworkPolicy.h#L86) | `RetargetProgressiveBitrateCeiling` | 声明 | `ProgressiveBitrateCeilingDecision RetargetProgressiveBitrateCeiling( ProgressiveBitrateCeilingState* state, std::uint64_t desiredMaxBitrateBps, std::uint64_t timestampMs)` | Records a new theoretical screen maximum. A lower target is applied immediately, while a higher target remains pending until stable BWE samples allow a bounded upward step. |
| [L158](../src/core/ScreenNetworkPolicy.h#L158) | `ResetAdaptiveScreenFrameRate` | 声明 | `void ResetAdaptiveScreenFrameRate( AdaptiveScreenFrameRateState* state, bool enabled, std::uint32_t requestedFrameRate, std::uint32_t outputWidth, std::uint32_t outputHeight, std::uint64_t timestampMs)` | Starts one per-PeerConnection network frame-rate controller. It changes only that connection's RTP sender maximum; the shared desktop capture scheduler and the user's requested ... |
| [L168](../src/core/ScreenNetworkPolicy.h#L168) | `RetargetAdaptiveScreenFrameRate` | 声明 | `AdaptiveScreenFrameRateDecision RetargetAdaptiveScreenFrameRate( AdaptiveScreenFrameRateState* state, std::uint32_t requestedFrameRate, std::uint32_t outputWidth, std::uint32_t outputHeight, std::uint64_t timestampMs)` | A user FPS or resolution change takes effect immediately. Later stats may temporarily reduce the effective sender FPS without overwriting the request. |

## `src/core/ScreenStreamPolicy.cpp`

[打开源码](../src/core/ScreenStreamPolicy.cpp) · **文件作用：** 实现 screen stream policy 相关函数与文件级辅助逻辑。

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L11](../src/core/ScreenStreamPolicy.cpp#L11) | `ResolveScreenStreamPolicy` | 定义 | `ScreenStreamPolicyResult ResolveScreenStreamPolicy( std::uint32_t sourceWidth, std::uint32_t sourceHeight, const ScreenStreamPolicyRequest& request)` | 查询并返回 resolve screen stream policy 相关逻辑。 |

## `src/core/ScreenStreamPolicy.h`

[打开源码](../src/core/ScreenStreamPolicy.h) · **文件作用：** 声明 screen stream policy 相关类型、接口、配置和成员状态。

### 类型

| 行 | 类型 | 种类 | 作用 |
|---:|---|---|---|
| [L10](../src/core/ScreenStreamPolicy.h#L10) | `ScreenStreamPolicyRequest` | struct | 定义 ScreenStreamPolicyRequest 的 struct 类型和相关状态。 |
| [L16](../src/core/ScreenStreamPolicy.h#L16) | `ScreenStreamPolicyResult` | struct | 定义 ScreenStreamPolicyResult 的 struct 类型和相关状态。 |

### 成员与文件级变量

| 行 | 变量 | 声明 | 作用 |
|---:|---|---|---|
| [L11](../src/core/ScreenStreamPolicy.h#L11) | `maxWidth` | `std::uint32_t maxWidth = 0;` | 保存计数、尺寸或速率指标：max width。 |
| [L12](../src/core/ScreenStreamPolicy.h#L12) | `maxHeight` | `std::uint32_t maxHeight = 0;` | 保存计数、尺寸或速率指标：max height。 |
| [L13](../src/core/ScreenStreamPolicy.h#L13) | `framesPerSecond` | `std::uint32_t framesPerSecond = 60;` | 保存 frames per second 相关配置或运行状态。 |
| [L17](../src/core/ScreenStreamPolicy.h#L17) | `width` | `std::uint32_t width = 0;` | 保存计数、尺寸或速率指标：width。 |
| [L18](../src/core/ScreenStreamPolicy.h#L18) | `height` | `std::uint32_t height = 0;` | 保存计数、尺寸或速率指标：height。 |
| [L19](../src/core/ScreenStreamPolicy.h#L19) | `framesPerSecond` | `std::uint32_t framesPerSecond = 60;` | 保存 frames per second 相关配置或运行状态。 |
| [L20](../src/core/ScreenStreamPolicy.h#L20) | `startBitrateBps` | `std::uint32_t startBitrateBps = 0;` | 保存计数、尺寸或速率指标：start bitrate bps。 |
| [L21](../src/core/ScreenStreamPolicy.h#L21) | `maxBitrateBps` | `std::uint32_t maxBitrateBps = 0;` | 保存计数、尺寸或速率指标：max bitrate bps。 |

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L26](../src/core/ScreenStreamPolicy.h#L26) | `ResolveScreenStreamPolicy` | 声明 | `ScreenStreamPolicyResult ResolveScreenStreamPolicy( std::uint32_t sourceWidth, std::uint32_t sourceHeight, const ScreenStreamPolicyRequest& request)` | Resolves the encoded desktop dimensions and bitrate without depending on capture, signaling, Qt, or WebRTC state. |

## `src/core/SessionController.cpp`

[打开源码](../src/core/SessionController.cpp) · **文件作用：** 实现 session controller 相关函数与文件级辅助逻辑。

### 类型

| 行 | 类型 | 种类 | 作用 |
|---:|---|---|---|
| [L21](../src/core/SessionController.cpp#L21) | `SerialExecutor` | class | 定义 SerialExecutor 的 class 类型和相关状态。 |
| [L79](../src/core/SessionController.cpp#L79) | `WorkItem` | struct | 定义 WorkItem 的 struct 类型和相关状态。 |
| [L85](../src/core/SessionController.cpp#L85) | `WorkItemLater` | struct | 定义 WorkItemLater 的 struct 类型和相关状态。 |
| [L156](../src/core/SessionController.cpp#L156) | `SessionControllerBase::Impl` | class | 定义 SessionControllerBase::Impl 的 class 类型和相关状态。 |

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L23](../src/core/SessionController.cpp#L23) | `SerialExecutor` | 定义 | `SerialExecutor() : thread_([this] { Run(); }) {}` | 实现 serial executor 对应的业务或工具逻辑。 |
| [L24](../src/core/SessionController.cpp#L24) | `~SerialExecutor` | 定义 | `~SerialExecutor() { Stop(); }` | 停止相关活动并释放 SerialExecutor 实例拥有的资源。 |
| [L29](../src/core/SessionController.cpp#L29) | `Post` | 定义 | `bool Post(std::function<void()> task)` | 实现 post 对应的业务或工具逻辑。 |
| [L34](../src/core/SessionController.cpp#L34) | `PostAfter` | 定义 | `bool PostAfter(std::chrono::milliseconds delay, std::function<void()> task)` | 实现 post after 对应的业务或工具逻辑。 |
| [L41](../src/core/SessionController.cpp#L41) | `Invoke` | 定义 | `void Invoke(std::function<void()> task)` | 实现 invoke 对应的业务或工具逻辑。 |
| [L59](../src/core/SessionController.cpp#L59) | `Stop` | 定义 | `void Stop()` | 停止 stop 相关逻辑。 |
| [L86](../src/core/SessionController.cpp#L86) | `operator` | 定义 | `bool operator()(const WorkItem& left, const WorkItem& right) const` | 实现 operator 对应的业务或工具逻辑。 |
| [L95](../src/core/SessionController.cpp#L95) | `PostAt` | 定义 | `bool PostAt(std::chrono::steady_clock::time_point due, std::function<void()> task)` | 实现 post at 对应的业务或工具逻辑。 |
| [L109](../src/core/SessionController.cpp#L109) | `Run` | 定义 | `void Run()` | 执行后台循环或调度 run 相关逻辑。 |
| [L147](../src/core/SessionController.cpp#L147) | `CandidateKey` | 定义 | `std::string CandidateKey(const IceCandidate& candidate)` | 判断 candidate key 相关逻辑。 |
| [L158](../src/core/SessionController.cpp#L158) | `Impl` | 定义 | `Impl(SessionControllerBase* owner, Role role, IWebRtcSession& session, ISessionSignalingSender& signaling) : owner_(owner), role_(role), session_(session), signaling_(signaling) {}` | 实现 impl 对应的业务或工具逻辑。 |
| [L168](../src/core/SessionController.cpp#L168) | `~Impl` | 定义 | `~Impl() { Shutdown(); }` | 停止相关活动并释放 Impl 实例拥有的资源。 |
| [L170](../src/core/SessionController.cpp#L170) | `SetObserver` | 定义 | `void SetObserver(ISessionControllerObserver* observer)` | 更新或应用 set observer 相关逻辑。 |
| [L176](../src/core/SessionController.cpp#L176) | `Start` | 定义 | `void Start(SessionControllerConfig config)` | 启动 start 相关逻辑。 |
| [L183](../src/core/SessionController.cpp#L183) | `BeginNegotiation` | 定义 | `void BeginNegotiation(std::vector<DataChannelSpec> dataChannels)` | 启动 begin negotiation 相关逻辑。 |
| [L191](../src/core/SessionController.cpp#L191) | `SetAgentAnswerPreparation` | 定义 | `void SetAgentAnswerPreparation( std::function<std::optional<OperationError>()> preparation)` | 更新或应用 set agent answer preparation 相关逻辑。 |
| [L202](../src/core/SessionController.cpp#L202) | `HandleRemoteDescription` | 定义 | `void HandleRemoteDescription(SessionDescription description)` | 接收并处理 handle remote description 相关逻辑。 |
| [L209](../src/core/SessionController.cpp#L209) | `HandleRemoteIceCandidate` | 定义 | `void HandleRemoteIceCandidate(IceCandidate candidate)` | 接收并处理 handle remote ice candidate 相关逻辑。 |
| [L216](../src/core/SessionController.cpp#L216) | `HandleRemoteIceRestartRequest` | 定义 | `void HandleRemoteIceRestartRequest(std::uint64_t observedGeneration, std::uint64_t requestSequence)` | 接收并处理 handle remote ice restart request 相关逻辑。 |
| [L252](../src/core/SessionController.cpp#L252) | `HandleRemoteIceRestartCancel` | 定义 | `void HandleRemoteIceRestartCancel(std::uint64_t observedGeneration, std::uint64_t requestSequence)` | 接收并处理 handle remote ice restart cancel 相关逻辑。 |
| [L265](../src/core/SessionController.cpp#L265) | `SetSignalingAvailable` | 定义 | `void SetSignalingAvailable(bool available)` | 更新或应用 set signaling available 相关逻辑。 |
| [L287](../src/core/SessionController.cpp#L287) | `Close` | 定义 | `void Close()` | 关闭并清理 close 相关逻辑。 |
| [L292](../src/core/SessionController.cpp#L292) | `SendData` | 定义 | `SendResult SendData(std::string channelName, std::vector<std::uint8_t> payload, bool binary)` | 发送或发布 send data 相关逻辑。 |
| [L305](../src/core/SessionController.cpp#L305) | `QueueData` | 定义 | `bool QueueData(std::string channelName, std::vector<std::uint8_t> payload, bool binary, std::function<void(SendResult)> completion)` | 实现 queue data 对应的业务或工具逻辑。 |
| [L324](../src/core/SessionController.cpp#L324) | `Snapshot` | 定义 | `SessionControllerSnapshot Snapshot() const` | 查询并返回 snapshot 相关逻辑。 |
| [L330](../src/core/SessionController.cpp#L330) | `OnSessionStateChanged` | 定义 | `void OnSessionStateChanged(WebRtcSessionState state)` | 接收并处理 on session state changed 相关逻辑。 |
| [L390](../src/core/SessionController.cpp#L390) | `OnIceGatheringStateChanged` | 定义 | `void OnIceGatheringStateChanged(WebRtcIceGatheringState) {}` | 接收并处理 on ice gathering state changed 相关逻辑。 |
| [L393](../src/core/SessionController.cpp#L393) | `OnLocalDescription` | 定义 | `void OnLocalDescription(SessionDescription description)` | 接收并处理 on local description 相关逻辑。 |
| [L411](../src/core/SessionController.cpp#L411) | `OnLocalIceCandidate` | 定义 | `void OnLocalIceCandidate(IceCandidate candidate)` | 接收并处理 on local ice candidate 相关逻辑。 |
| [L429](../src/core/SessionController.cpp#L429) | `OnDataChannelStateChanged` | 定义 | `void OnDataChannelStateChanged(DataChannelInfo channel)` | 接收并处理 on data channel state changed 相关逻辑。 |
| [L438](../src/core/SessionController.cpp#L438) | `OnDataMessage` | 定义 | `void OnDataMessage(std::string label, std::vector<std::uint8_t> payload, bool binary)` | 接收并处理 on data message 相关逻辑。 |
| [L450](../src/core/SessionController.cpp#L450) | `OnRemoteTrackAdded` | 定义 | `void OnRemoteTrackAdded(RemoteTrackInfo track)` | 接收并处理 on remote track added 相关逻辑。 |
| [L459](../src/core/SessionController.cpp#L459) | `OnOperationCompleted` | 定义 | `void OnOperationCompleted(OperationId operationId)` | 接收并处理 on operation completed 相关逻辑。 |
| [L465](../src/core/SessionController.cpp#L465) | `OnWebRtcError` | 定义 | `void OnWebRtcError(OperationId operationId, OperationError error)` | 接收并处理 on web rtc error 相关逻辑。 |
| [L518](../src/core/SessionController.cpp#L518) | `Shutdown` | 定义 | `void Shutdown()` | 关闭并清理 shutdown 相关逻辑。 |
| [L536](../src/core/SessionController.cpp#L536) | `StartOnExecutor` | 定义 | `void StartOnExecutor(SessionControllerConfig config)` | 启动 start on executor 相关逻辑。 |
| [L549](../src/core/SessionController.cpp#L549) | `BeginNegotiationOnExecutor` | 定义 | `void BeginNegotiationOnExecutor( std::vector<DataChannelSpec> dataChannels)` | 启动 begin negotiation on executor 相关逻辑。 |
| [L568](../src/core/SessionController.cpp#L568) | `HandleRemoteDescriptionOnExecutor` | 定义 | `void HandleRemoteDescriptionOnExecutor(SessionDescription description)` | 接收并处理 handle remote description on executor 相关逻辑。 |
| [L644](../src/core/SessionController.cpp#L644) | `HandleRemoteIceCandidateOnExecutor` | 定义 | `void HandleRemoteIceCandidateOnExecutor(IceCandidate candidate)` | 接收并处理 handle remote ice candidate on executor 相关逻辑。 |
| [L664](../src/core/SessionController.cpp#L664) | `AddRemoteIceCandidateOnExecutor` | 定义 | `void AddRemoteIceCandidateOnExecutor(const IceCandidate& candidate)` | 实现 add remote ice candidate on executor 对应的业务或工具逻辑。 |
| [L673](../src/core/SessionController.cpp#L673) | `OnOperationCompletedOnExecutor` | 定义 | `void OnOperationCompletedOnExecutor(OperationId operationId)` | 接收并处理 on operation completed on executor 相关逻辑。 |
| [L717](../src/core/SessionController.cpp#L717) | `ResetRemoteNegotiationContext` | 定义 | `void ResetRemoteNegotiationContext()` | 重置或移除 reset remote negotiation context 相关逻辑。 |
| [L738](../src/core/SessionController.cpp#L738) | `CompleteConnectedOnExecutor` | 定义 | `void CompleteConnectedOnExecutor()` | 实现 complete connected on executor 对应的业务或工具逻辑。 |
| [L759](../src/core/SessionController.cpp#L759) | `UpdateRecoverySnapshot` | 定义 | `void UpdateRecoverySnapshot()` | 更新或应用 update recovery snapshot 相关逻辑。 |
| [L773](../src/core/SessionController.cpp#L773) | `BeginOrRequestIceRestartOnExecutor` | 定义 | `void BeginOrRequestIceRestartOnExecutor()` | 启动 begin or request ice restart on executor 相关逻辑。 |
| [L819](../src/core/SessionController.cpp#L819) | `BeginIceRestartOnExecutor` | 定义 | `void BeginIceRestartOnExecutor()` | 启动 begin ice restart on executor 相关逻辑。 |
| [L850](../src/core/SessionController.cpp#L850) | `ArmDisconnectedGracePeriod` | 定义 | `void ArmDisconnectedGracePeriod()` | 实现 arm disconnected grace period 对应的业务或工具逻辑。 |
| [L864](../src/core/SessionController.cpp#L864) | `ArmIceRestartTimeout` | 定义 | `void ArmIceRestartTimeout()` | 实现 arm ice restart timeout 对应的业务或工具逻辑。 |
| [L890](../src/core/SessionController.cpp#L890) | `VerifyTransportBeforeIceRestartTimeoutOnExecutor` | 定义 | `void VerifyTransportBeforeIceRestartTimeoutOnExecutor( std::uint64_t generation, std::uint32_t attempt, std::int64_t previousStatsTimestamp, std::chrono::steady_clock::time_point deadline)` | 校验 verify transport before ice restart timeout on executor 相关逻辑。 |
| [L937](../src/core/SessionController.cpp#L937) | `HandleIceRestartTimeoutOnExecutor` | 定义 | `void HandleIceRestartTimeoutOnExecutor()` | 接收并处理 handle ice restart timeout on executor 相关逻辑。 |
| [L959](../src/core/SessionController.cpp#L959) | `CancelOutstandingIceRestartRequestOnExecutor` | 定义 | `void CancelOutstandingIceRestartRequestOnExecutor()` | 判断 cancel outstanding ice restart request on executor 相关逻辑。 |
| [L978](../src/core/SessionController.cpp#L978) | `ClearPendingRemoteIceRestartRequestOnExecutor` | 定义 | `void ClearPendingRemoteIceRestartRequestOnExecutor()` | 重置或移除 clear pending remote ice restart request on executor 相关逻辑。 |
| [L985](../src/core/SessionController.cpp#L985) | `ArmNegotiationTimeout` | 定义 | `void ArmNegotiationTimeout()` | 实现 arm negotiation timeout 对应的业务或工具逻辑。 |
| [L1000](../src/core/SessionController.cpp#L1000) | `ArmReconnectTimeout` | 定义 | `void ArmReconnectTimeout()` | 实现 arm reconnect timeout 对应的业务或工具逻辑。 |
| [L1017](../src/core/SessionController.cpp#L1017) | `Fail` | 定义 | `void Fail(std::string code, std::string message)` | 实现 fail 对应的业务或工具逻辑。 |
| [L1031](../src/core/SessionController.cpp#L1031) | `CloseOnExecutor` | 定义 | `void CloseOnExecutor()` | 关闭并清理 close on executor 相关逻辑。 |
| [L1043](../src/core/SessionController.cpp#L1043) | `PublishState` | 定义 | `void PublishState(SessionControllerState state)` | 发送或发布 publish state 相关逻辑。 |
| [L1059](../src/core/SessionController.cpp#L1059) | `Observer` | 定义 | `ISessionControllerObserver* Observer() const` | 实现 observer 对应的业务或工具逻辑。 |
| [L1112](../src/core/SessionController.cpp#L1112) | `SessionControllerBase::SessionControllerBase` | 定义 | `SessionControllerBase::SessionControllerBase( Role role, IWebRtcSession& session, ISessionSignalingSender& signaling) : impl_(std::make_unique<Impl>(this, role, session, signaling)) {}` | 构造并初始化 SessionControllerBase 实例。 |
| [L1121](../src/core/SessionController.cpp#L1121) | `SessionControllerBase::SetObserver` | 定义 | `void SessionControllerBase::SetObserver( ISessionControllerObserver* observer)` | 更新或应用 set observer 相关逻辑。 |
| [L1127](../src/core/SessionController.cpp#L1127) | `SessionControllerBase::Start` | 定义 | `void SessionControllerBase::Start(const SessionControllerConfig& config)` | 启动 start 相关逻辑。 |
| [L1132](../src/core/SessionController.cpp#L1132) | `SessionControllerBase::HandleRemoteDescription` | 定义 | `void SessionControllerBase::HandleRemoteDescription( const SessionDescription& description)` | 接收并处理 handle remote description 相关逻辑。 |
| [L1138](../src/core/SessionController.cpp#L1138) | `SessionControllerBase::HandleRemoteIceCandidate` | 定义 | `void SessionControllerBase::HandleRemoteIceCandidate( const IceCandidate& candidate)` | 接收并处理 handle remote ice candidate 相关逻辑。 |
| [L1144](../src/core/SessionController.cpp#L1144) | `SessionControllerBase::HandleRemoteIceRestartRequest` | 定义 | `void SessionControllerBase::HandleRemoteIceRestartRequest( std::uint64_t observedGeneration, std::uint64_t requestSequence)` | 接收并处理 handle remote ice restart request 相关逻辑。 |
| [L1151](../src/core/SessionController.cpp#L1151) | `SessionControllerBase::HandleRemoteIceRestartCancel` | 定义 | `void SessionControllerBase::HandleRemoteIceRestartCancel( std::uint64_t observedGeneration, std::uint64_t requestSequence)` | 接收并处理 handle remote ice restart cancel 相关逻辑。 |
| [L1158](../src/core/SessionController.cpp#L1158) | `SessionControllerBase::SetSignalingAvailable` | 定义 | `void SessionControllerBase::SetSignalingAvailable(bool available)` | 更新或应用 set signaling available 相关逻辑。 |
| [L1163](../src/core/SessionController.cpp#L1163) | `SessionControllerBase::SendData` | 定义 | `SendResult SessionControllerBase::SendData( const std::string& channelName, std::span<const std::uint8_t> data, bool binary)` | 发送或发布 send data 相关逻辑。 |
| [L1173](../src/core/SessionController.cpp#L1173) | `SessionControllerBase::QueueData` | 定义 | `bool SessionControllerBase::QueueData( const std::string& channelName, std::span<const std::uint8_t> data, bool binary, std::function<void(SendResult)> completion)` | 实现 queue data 对应的业务或工具逻辑。 |
| [L1186](../src/core/SessionController.cpp#L1186) | `SessionControllerBase::Close` | 定义 | `void SessionControllerBase::Close()` | 关闭并清理 close 相关逻辑。 |
| [L1191](../src/core/SessionController.cpp#L1191) | `SessionControllerBase::Snapshot` | 定义 | `SessionControllerSnapshot SessionControllerBase::Snapshot() const` | 查询并返回 snapshot 相关逻辑。 |
| [L1196](../src/core/SessionController.cpp#L1196) | `SessionControllerBase::BeginNegotiation` | 定义 | `void SessionControllerBase::BeginNegotiation( const std::vector<DataChannelSpec>& dataChannels)` | 启动 begin negotiation 相关逻辑。 |
| [L1202](../src/core/SessionController.cpp#L1202) | `SessionControllerBase::SetAgentAnswerPreparation` | 定义 | `void SessionControllerBase::SetAgentAnswerPreparation( std::function<std::optional<OperationError>()> preparation)` | 更新或应用 set agent answer preparation 相关逻辑。 |
| [L1208](../src/core/SessionController.cpp#L1208) | `SessionControllerBase::OnSessionStateChanged` | 定义 | `void SessionControllerBase::OnSessionStateChanged(WebRtcSessionState state)` | 接收并处理 on session state changed 相关逻辑。 |
| [L1213](../src/core/SessionController.cpp#L1213) | `SessionControllerBase::OnIceGatheringStateChanged` | 定义 | `void SessionControllerBase::OnIceGatheringStateChanged( WebRtcIceGatheringState state)` | 接收并处理 on ice gathering state changed 相关逻辑。 |
| [L1219](../src/core/SessionController.cpp#L1219) | `SessionControllerBase::OnLocalDescription` | 定义 | `void SessionControllerBase::OnLocalDescription( const SessionDescription& description)` | 接收并处理 on local description 相关逻辑。 |
| [L1225](../src/core/SessionController.cpp#L1225) | `SessionControllerBase::OnLocalIceCandidate` | 定义 | `void SessionControllerBase::OnLocalIceCandidate( const IceCandidate& candidate)` | 接收并处理 on local ice candidate 相关逻辑。 |
| [L1231](../src/core/SessionController.cpp#L1231) | `SessionControllerBase::OnDataChannelStateChanged` | 定义 | `void SessionControllerBase::OnDataChannelStateChanged( const DataChannelInfo& channel)` | 接收并处理 on data channel state changed 相关逻辑。 |
| [L1237](../src/core/SessionController.cpp#L1237) | `SessionControllerBase::OnDataMessage` | 定义 | `void SessionControllerBase::OnDataMessage( const std::string& label, std::span<const std::uint8_t> payload, bool binary)` | 接收并处理 on data message 相关逻辑。 |
| [L1248](../src/core/SessionController.cpp#L1248) | `SessionControllerBase::OnRemoteTrackAdded` | 定义 | `void SessionControllerBase::OnRemoteTrackAdded(const RemoteTrackInfo& track)` | 接收并处理 on remote track added 相关逻辑。 |
| [L1253](../src/core/SessionController.cpp#L1253) | `SessionControllerBase::OnOperationCompleted` | 定义 | `void SessionControllerBase::OnOperationCompleted(OperationId operationId)` | 接收并处理 on operation completed 相关逻辑。 |
| [L1258](../src/core/SessionController.cpp#L1258) | `SessionControllerBase::OnWebRtcError` | 定义 | `void SessionControllerBase::OnWebRtcError( OperationId operationId, const OperationError& error)` | 接收并处理 on web rtc error 相关逻辑。 |
| [L1265](../src/core/SessionController.cpp#L1265) | `ControllerSessionController::ControllerSessionController` | 定义 | `ControllerSessionController::ControllerSessionController( IWebRtcSession& session, ISessionSignalingSender& signaling) : SessionControllerBase(Role::kController, session, signaling) {}` | 构造并初始化 ControllerSessionController 实例。 |
| [L1271](../src/core/SessionController.cpp#L1271) | `ControllerSessionController::Connect` | 定义 | `void ControllerSessionController::Connect( const std::vector<DataChannelSpec>& dataChannels)` | 建立连接 connect 相关逻辑。 |
| [L1277](../src/core/SessionController.cpp#L1277) | `AgentSessionController::AgentSessionController` | 定义 | `AgentSessionController::AgentSessionController( IWebRtcSession& session, ISessionSignalingSender& signaling) : SessionControllerBase(Role::kAgent, session, signaling) {}` | 构造并初始化 AgentSessionController 实例。 |
| [L1283](../src/core/SessionController.cpp#L1283) | `AgentSessionController::SetAnswerPreparation` | 定义 | `void AgentSessionController::SetAnswerPreparation( std::function<std::optional<OperationError>()> preparation)` | 更新或应用 set answer preparation 相关逻辑。 |

## `src/core/SessionController.h`

[打开源码](../src/core/SessionController.h) · **文件作用：** 声明 session controller 相关类型、接口、配置和成员状态。

### 类型

| 行 | 类型 | 种类 | 作用 |
|---:|---|---|---|
| [L19](../src/core/SessionController.h#L19) | `SessionControllerState` | enum class | 定义 SessionControllerState 的 enum class 类型和相关状态。 |
| [L33](../src/core/SessionController.h#L33) | `SessionControllerConfig` | struct | 定义 SessionControllerConfig 的 struct 类型和相关状态。 |
| [L46](../src/core/SessionController.h#L46) | `SessionControllerSnapshot` | struct | 定义 SessionControllerSnapshot 的 struct 类型和相关状态。 |
| [L55](../src/core/SessionController.h#L55) | `ISessionSignalingSender` | class | 定义 ISessionSignalingSender 的 class 类型和相关状态。 |
| [L77](../src/core/SessionController.h#L77) | `ISessionControllerObserver` | class | 定义 ISessionControllerObserver 的 class 类型和相关状态。 |
| [L93](../src/core/SessionController.h#L93) | `SessionControllerBase` | class | 定义 SessionControllerBase 的 class 类型和相关状态。 |
| [L137](../src/core/SessionController.h#L137) | `Role` | enum class | 定义 Role 的 enum class 类型和相关状态。 |
| [L151](../src/core/SessionController.h#L151) | `Impl` | class | 定义 Impl 的 class 类型和相关状态。 |
| [L155](../src/core/SessionController.h#L155) | `ControllerSessionController` | class | 定义 ControllerSessionController 的 class 类型和相关状态。 |
| [L163](../src/core/SessionController.h#L163) | `AgentSessionController` | class | 定义 AgentSessionController 的 class 类型和相关状态。 |

### 成员与文件级变量

| 行 | 变量 | 声明 | 作用 |
|---:|---|---|---|
| [L34](../src/core/SessionController.h#L34) | `webRtc` | `WebRtcSessionConfig webRtc;` | 保存 web rtc 相关配置或运行状态。 |
| [L35](../src/core/SessionController.h#L35) | `negotiationTimeout` | `std::chrono::milliseconds negotiationTimeout{10000};` | 保存定时、截止或超时状态：negotiation timeout。 |
| [L36](../src/core/SessionController.h#L36) | `reconnectTimeout` | `std::chrono::milliseconds reconnectTimeout{60000};` | 保存定时、截止或超时状态：reconnect timeout。 |
| [L37](../src/core/SessionController.h#L37) | `disconnectedGracePeriod` | `std::chrono::milliseconds disconnectedGracePeriod{3000};` | 保存 disconnected grace period 相关配置或运行状态。 |
| [L38](../src/core/SessionController.h#L38) | `iceRestartTimeout` | `std::chrono::milliseconds iceRestartTimeout{10000};` | 保存定时、截止或超时状态：ice restart timeout。 |
| [L39](../src/core/SessionController.h#L39) | `transportVerificationDelay` | `std::chrono::milliseconds transportVerificationDelay{350};` | 保存 transport verification delay 相关配置或运行状态。 |
| [L40](../src/core/SessionController.h#L40) | `transportVerificationTimeout` | `std::chrono::milliseconds transportVerificationTimeout{2000};` | 保存定时、截止或超时状态：transport verification timeout。 |
| [L41](../src/core/SessionController.h#L41) | `remoteIceRestartRequestDebounce` | `std::chrono::milliseconds remoteIceRestartRequestDebounce{1000};` | 保存 remote ice restart request debounce 相关配置或运行状态。 |
| [L42](../src/core/SessionController.h#L42) | `connectedStabilityPeriod` | `std::chrono::milliseconds connectedStabilityPeriod{2000};` | 保存 connected stability period 相关配置或运行状态。 |
| [L43](../src/core/SessionController.h#L43) | `maximumIceRestartAttempts` | `std::uint32_t maximumIceRestartAttempts = 3;` | 保存 maximum ice restart attempts 相关配置或运行状态。 |
| [L47](../src/core/SessionController.h#L47) | `state` | `SessionControllerState state = SessionControllerState::kIdle;` | 保存状态机当前状态：state。 |
| [L48](../src/core/SessionController.h#L48) | `webRtcState` | `WebRtcSessionState webRtcState = WebRtcSessionState::kNew;` | 保存状态机当前状态：web rtc state。 |
| [L49](../src/core/SessionController.h#L49) | `errorCode` | `std::string errorCode;` | 保存 error code 相关配置或运行状态。 |
| [L50](../src/core/SessionController.h#L50) | `errorMessage` | `std::string errorMessage;` | 保存 error message 相关配置或运行状态。 |
| [L51](../src/core/SessionController.h#L51) | `negotiationGeneration` | `std::uint64_t negotiationGeneration = 1;` | 标记当前世代，用于拒绝过期异步结果：negotiation generation。 |
| [L52](../src/core/SessionController.h#L52) | `iceRestartAttempt` | `std::uint32_t iceRestartAttempt = 0;` | 保存 ice restart attempt 相关配置或运行状态。 |
| [L151](../src/core/SessionController.h#L151) | `Impl` | `class Impl;` | 保存 impl 相关配置或运行状态。 |
| [L152](../src/core/SessionController.h#L152) | `impl_` | `std::unique_ptr<Impl> impl_;` | 保存 impl 相关配置或运行状态。 |

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L57](../src/core/SessionController.h#L57) | `~ISessionSignalingSender` | 声明 | `virtual ~ISessionSignalingSender() = default` | 停止相关活动并释放 ISessionSignalingSender 实例拥有的资源。 |
| [L58](../src/core/SessionController.h#L58) | `SendDescription` | 声明 | `virtual bool SendDescription( const SessionDescription& description) = 0` | 发送或发布 send description 相关逻辑。 |
| [L60](../src/core/SessionController.h#L60) | `SendIceCandidate` | 声明 | `virtual bool SendIceCandidate(const IceCandidate& candidate) = 0` | 发送或发布 send ice candidate 相关逻辑。 |
| [L61](../src/core/SessionController.h#L61) | `RequestIceRestart` | 定义 | `virtual bool RequestIceRestart(std::uint64_t observedGeneration, std::uint64_t requestSequence)` | 发起请求或查询 request ice restart 相关逻辑。 |
| [L68](../src/core/SessionController.h#L68) | `CancelIceRestart` | 定义 | `virtual bool CancelIceRestart(std::uint64_t observedGeneration, std::uint64_t requestSequence)` | 判断 cancel ice restart 相关逻辑。 |
| [L79](../src/core/SessionController.h#L79) | `~ISessionControllerObserver` | 声明 | `virtual ~ISessionControllerObserver() = default` | 停止相关活动并释放 ISessionControllerObserver 实例拥有的资源。 |
| [L83](../src/core/SessionController.h#L83) | `OnControllerSnapshot` | 声明 | `virtual void OnControllerSnapshot( const SessionControllerSnapshot& snapshot) = 0` | Callbacks run on the controller's serial executor. Implementations must return quickly and post UI or business work to the appropriate thread. |
| [L85](../src/core/SessionController.h#L85) | `OnDataChannelStateChanged` | 声明 | `virtual void OnDataChannelStateChanged( const DataChannelInfo& channel) = 0` | 接收并处理 on data channel state changed 相关逻辑。 |
| [L87](../src/core/SessionController.h#L87) | `OnDataMessage` | 声明 | `virtual void OnDataMessage(const std::string& label, std::span<const std::uint8_t> payload, bool binary) = 0` | 接收并处理 on data message 相关逻辑。 |
| [L90](../src/core/SessionController.h#L90) | `OnRemoteTrackAdded` | 声明 | `virtual void OnRemoteTrackAdded(const RemoteTrackInfo& track) = 0` | 接收并处理 on remote track added 相关逻辑。 |
| [L95](../src/core/SessionController.h#L95) | `~SessionControllerBase` | 声明 | `~SessionControllerBase() override` | 停止相关活动并释放 SessionControllerBase 实例拥有的资源。 |
| [L97](../src/core/SessionController.h#L97) | `SessionControllerBase` | 声明 | `SessionControllerBase(const SessionControllerBase&) = delete` | 实现 session controller base 对应的业务或工具逻辑。 |
| [L100](../src/core/SessionController.h#L100) | `SetObserver` | 声明 | `void SetObserver(ISessionControllerObserver* observer)` | 更新或应用 set observer 相关逻辑。 |
| [L101](../src/core/SessionController.h#L101) | `Start` | 声明 | `void Start(const SessionControllerConfig& config)` | 启动 start 相关逻辑。 |
| [L102](../src/core/SessionController.h#L102) | `HandleRemoteDescription` | 声明 | `void HandleRemoteDescription(const SessionDescription& description)` | 接收并处理 handle remote description 相关逻辑。 |
| [L103](../src/core/SessionController.h#L103) | `HandleRemoteIceCandidate` | 声明 | `void HandleRemoteIceCandidate(const IceCandidate& candidate)` | 接收并处理 handle remote ice candidate 相关逻辑。 |
| [L104](../src/core/SessionController.h#L104) | `HandleRemoteIceRestartRequest` | 声明 | `void HandleRemoteIceRestartRequest(std::uint64_t observedGeneration, std::uint64_t requestSequence)` | 接收并处理 handle remote ice restart request 相关逻辑。 |
| [L106](../src/core/SessionController.h#L106) | `HandleRemoteIceRestartCancel` | 声明 | `void HandleRemoteIceRestartCancel(std::uint64_t observedGeneration, std::uint64_t requestSequence)` | 接收并处理 handle remote ice restart cancel 相关逻辑。 |
| [L108](../src/core/SessionController.h#L108) | `SetSignalingAvailable` | 声明 | `void SetSignalingAvailable(bool available)` | 更新或应用 set signaling available 相关逻辑。 |
| [L109](../src/core/SessionController.h#L109) | `SendData` | 声明 | `SendResult SendData(const std::string& channelName, std::span<const std::uint8_t> data, bool binary)` | 发送或发布 send data 相关逻辑。 |
| [L112](../src/core/SessionController.h#L112) | `QueueData` | 定义 | `bool QueueData( const std::string& channelName, std::span<const std::uint8_t> data, bool binary, std::function<void(SendResult)> completion = {})` | 实现 queue data 对应的业务或工具逻辑。 |
| [L117](../src/core/SessionController.h#L117) | `Close` | 声明 | `void Close()` | 关闭并清理 close 相关逻辑。 |
| [L118](../src/core/SessionController.h#L118) | `Snapshot` | 声明 | `SessionControllerSnapshot Snapshot() const` | 查询并返回 snapshot 相关逻辑。 |
| [L120](../src/core/SessionController.h#L120) | `OnSessionStateChanged` | 声明 | `void OnSessionStateChanged(WebRtcSessionState state) final` | 接收并处理 on session state changed 相关逻辑。 |
| [L121](../src/core/SessionController.h#L121) | `OnIceGatheringStateChanged` | 声明 | `void OnIceGatheringStateChanged( WebRtcIceGatheringState state) final` | 接收并处理 on ice gathering state changed 相关逻辑。 |
| [L123](../src/core/SessionController.h#L123) | `OnLocalDescription` | 声明 | `void OnLocalDescription( const SessionDescription& description) final` | 接收并处理 on local description 相关逻辑。 |
| [L125](../src/core/SessionController.h#L125) | `OnLocalIceCandidate` | 声明 | `void OnLocalIceCandidate(const IceCandidate& candidate) final` | 接收并处理 on local ice candidate 相关逻辑。 |
| [L126](../src/core/SessionController.h#L126) | `OnDataChannelStateChanged` | 声明 | `void OnDataChannelStateChanged( const DataChannelInfo& channel) final` | 接收并处理 on data channel state changed 相关逻辑。 |
| [L128](../src/core/SessionController.h#L128) | `OnDataMessage` | 声明 | `void OnDataMessage(const std::string& label, std::span<const std::uint8_t> payload, bool binary) final` | 接收并处理 on data message 相关逻辑。 |
| [L131](../src/core/SessionController.h#L131) | `OnRemoteTrackAdded` | 声明 | `void OnRemoteTrackAdded(const RemoteTrackInfo& track) final` | 接收并处理 on remote track added 相关逻辑。 |
| [L132](../src/core/SessionController.h#L132) | `OnOperationCompleted` | 声明 | `void OnOperationCompleted(OperationId operationId) final` | 接收并处理 on operation completed 相关逻辑。 |
| [L133](../src/core/SessionController.h#L133) | `OnWebRtcError` | 声明 | `void OnWebRtcError(OperationId operationId, const OperationError& error) final` | 接收并处理 on web rtc error 相关逻辑。 |
| [L142](../src/core/SessionController.h#L142) | `SessionControllerBase` | 声明 | `SessionControllerBase(Role role, IWebRtcSession& session, ISessionSignalingSender& signaling)` | 实现 session controller base 对应的业务或工具逻辑。 |
| [L145](../src/core/SessionController.h#L145) | `BeginNegotiation` | 声明 | `void BeginNegotiation( const std::vector<DataChannelSpec>& dataChannels)` | 启动 begin negotiation 相关逻辑。 |
| [L147](../src/core/SessionController.h#L147) | `SetAgentAnswerPreparation` | 声明 | `void SetAgentAnswerPreparation( std::function<std::optional<OperationError>()> preparation)` | 更新或应用 set agent answer preparation 相关逻辑。 |
| [L157](../src/core/SessionController.h#L157) | `ControllerSessionController` | 声明 | `ControllerSessionController(IWebRtcSession& session, ISessionSignalingSender& signaling)` | 实现 controller session controller 对应的业务或工具逻辑。 |
| [L160](../src/core/SessionController.h#L160) | `Connect` | 声明 | `void Connect(const std::vector<DataChannelSpec>& dataChannels)` | 建立连接 connect 相关逻辑。 |
| [L165](../src/core/SessionController.h#L165) | `AgentSessionController` | 声明 | `AgentSessionController(IWebRtcSession& session, ISessionSignalingSender& signaling)` | 实现 agent session controller 对应的业务或工具逻辑。 |
| [L171](../src/core/SessionController.h#L171) | `SetAnswerPreparation` | 声明 | `void SetAnswerPreparation( std::function<std::optional<OperationError>()> preparation)` | Runs after the remote Offer has been applied and before CreateAnswer(). Agent media tracks should be attached here so they bind to the offered transceiver instead of creating an... |

## `src/core/SessionDiagnostics.h`

[打开源码](../src/core/SessionDiagnostics.h) · **文件作用：** 声明 session diagnostics 相关类型、接口、配置和成员状态。

### 类型

| 行 | 类型 | 种类 | 作用 |
|---:|---|---|---|
| [L14](../src/core/SessionDiagnostics.h#L14) | `RtpStreamDirection` | enum class | 定义 RtpStreamDirection 的 enum class 类型和相关状态。 |
| [L19](../src/core/SessionDiagnostics.h#L19) | `RtpStreamStatsSnapshot` | struct | 定义 RtpStreamStatsSnapshot 的 struct 类型和相关状态。 |
| [L183](../src/core/SessionDiagnostics.h#L183) | `DataChannelStatsSnapshot` | struct | 定义 DataChannelStatsSnapshot 的 struct 类型和相关状态。 |
| [L197](../src/core/SessionDiagnostics.h#L197) | `IceCandidateStatsSnapshot` | struct | 定义 IceCandidateStatsSnapshot 的 struct 类型和相关状态。 |
| [L207](../src/core/SessionDiagnostics.h#L207) | `WebRtcTransportStatsSnapshot` | struct | 定义 WebRtcTransportStatsSnapshot 的 struct 类型和相关状态。 |
| [L235](../src/core/SessionDiagnostics.h#L235) | `WebRtcSessionStatsSnapshot` | struct | 定义 WebRtcSessionStatsSnapshot 的 struct 类型和相关状态。 |
| [L241](../src/core/SessionDiagnostics.h#L241) | `PeerConnectionDiagnosticsSnapshot` | struct | 定义 PeerConnectionDiagnosticsSnapshot 的 struct 类型和相关状态。 |
| [L247](../src/core/SessionDiagnostics.h#L247) | `SessionDiagnosticsSnapshot` | struct | 定义 SessionDiagnosticsSnapshot 的 struct 类型和相关状态。 |
| [L248](../src/core/SessionDiagnostics.h#L248) | `RemoteCursorDiagnosticsSnapshot` | struct | 定义 RemoteCursorDiagnosticsSnapshot 的 struct 类型和相关状态。 |

### 成员与文件级变量

| 行 | 变量 | 声明 | 作用 |
|---:|---|---|---|
| [L20](../src/core/SessionDiagnostics.h#L20) | `statsId` | `std::string statsId;` | 保存身份或作用域标识：stats id。 |
| [L21](../src/core/SessionDiagnostics.h#L21) | `direction` | `RtpStreamDirection direction = RtpStreamDirection::kOutbound;` | 保存 direction 相关配置或运行状态。 |
| [L22](../src/core/SessionDiagnostics.h#L22) | `kind` | `std::string kind;` | 保存 kind 相关配置或运行状态。 |
| [L23](../src/core/SessionDiagnostics.h#L23) | `slot` | `std::string slot;` | 保存 slot 相关配置或运行状态。 |
| [L24](../src/core/SessionDiagnostics.h#L24) | `mid` | `std::string mid;` | 保存身份或作用域标识：mid。 |
| [L25](../src/core/SessionDiagnostics.h#L25) | `codec` | `std::string codec;` | 保存 codec 相关配置或运行状态。 |
| [L26](../src/core/SessionDiagnostics.h#L26) | `encoderImplementation` | `std::string encoderImplementation;` | 保存 encoder implementation 相关配置或运行状态。 |
| [L27](../src/core/SessionDiagnostics.h#L27) | `decoderImplementation` | `std::string decoderImplementation;` | 保存 decoder implementation 相关配置或运行状态。 |
| [L28](../src/core/SessionDiagnostics.h#L28) | `qualityLimitationReason` | `std::string qualityLimitationReason;` | 保存 quality limitation reason 相关配置或运行状态。 |
| [L29](../src/core/SessionDiagnostics.h#L29) | `bytes` | `std::uint64_t bytes = 0;` | 保存计数、尺寸或速率指标：bytes。 |
| [L30](../src/core/SessionDiagnostics.h#L30) | `bitrateBps` | `std::uint64_t bitrateBps = 0;` | 保存计数、尺寸或速率指标：bitrate bps。 |
| [L31](../src/core/SessionDiagnostics.h#L31) | `sampleWindowMs` | `std::uint32_t sampleWindowMs = 0;` | 保存 sample window ms 相关配置或运行状态。 |
| [L32](../src/core/SessionDiagnostics.h#L32) | `packets` | `std::uint64_t packets = 0;` | 保存 packets 相关配置或运行状态。 |
| [L33](../src/core/SessionDiagnostics.h#L33) | `packetsLost` | `std::int64_t packetsLost = 0;` | 保存 packets lost 相关配置或运行状态。 |
| [L34](../src/core/SessionDiagnostics.h#L34) | `retransmittedPackets` | `std::uint64_t retransmittedPackets = 0;` | 保存 retransmitted packets 相关配置或运行状态。 |
| [L35](../src/core/SessionDiagnostics.h#L35) | `retransmittedBytes` | `std::uint64_t retransmittedBytes = 0;` | 保存计数、尺寸或速率指标：retransmitted bytes。 |
| [L36](../src/core/SessionDiagnostics.h#L36) | `lossPercent` | `double lossPercent = 0.0;` | 保存 loss percent 相关配置或运行状态。 |
| [L37](../src/core/SessionDiagnostics.h#L37) | `jitterMs` | `double jitterMs = 0.0;` | 保存 jitter ms 相关配置或运行状态。 |
| [L38](../src/core/SessionDiagnostics.h#L38) | `roundTripTimeMs` | `double roundTripTimeMs = 0.0;` | 保存 round trip time ms 相关配置或运行状态。 |
| [L39](../src/core/SessionDiagnostics.h#L39) | `targetBitrateBps` | `std::uint64_t targetBitrateBps = 0;` | 保存计数、尺寸或速率指标：target bitrate bps。 |
| [L40](../src/core/SessionDiagnostics.h#L40) | `configuredStartBitrateBps` | `std::uint64_t configuredStartBitrateBps = 0;` | 保存计数、尺寸或速率指标：configured start bitrate bps。 |
| [L41](../src/core/SessionDiagnostics.h#L41) | `configuredMaxBitrateBps` | `std::uint64_t configuredMaxBitrateBps = 0;` | 保存计数、尺寸或速率指标：configured max bitrate bps。 |
| [L42](../src/core/SessionDiagnostics.h#L42) | `bitrateBootstrapAttempts` | `std::uint32_t bitrateBootstrapAttempts = 0;` | 保存 bitrate bootstrap attempts 相关配置或运行状态。 |
| [L43](../src/core/SessionDiagnostics.h#L43) | `bitrateBootstrapSuccesses` | `std::uint32_t bitrateBootstrapSuccesses = 0;` | 保存 bitrate bootstrap successes 相关配置或运行状态。 |
| [L44](../src/core/SessionDiagnostics.h#L44) | `mediaReadyBitrateRestarts` | `std::uint32_t mediaReadyBitrateRestarts = 0;` | 保存 media ready bitrate restarts 相关配置或运行状态。 |
| [L45](../src/core/SessionDiagnostics.h#L45) | `allocationProbePulses` | `std::uint32_t allocationProbePulses = 0;` | 保存 allocation probe pulses 相关配置或运行状态。 |
| [L46](../src/core/SessionDiagnostics.h#L46) | `bitrateProbeFloorReleases` | `std::uint32_t bitrateProbeFloorReleases = 0;` | 保存 bitrate probe floor releases 相关配置或运行状态。 |
| [L47](../src/core/SessionDiagnostics.h#L47) | `bitrateProbeFloorActive` | `bool bitrateProbeFloorActive = false;` | 保存能力或开关状态：bitrate probe floor active。 |
| [L48](../src/core/SessionDiagnostics.h#L48) | `bitrateBootstrapError` | `std::string bitrateBootstrapError;` | 保存最近错误或失败原因：bitrate bootstrap error。 |
| [L49](../src/core/SessionDiagnostics.h#L49) | `progressiveBitrateCeilingEnabled` | `bool progressiveBitrateCeilingEnabled = false;` | 保存能力或开关状态：progressive bitrate ceiling enabled。 |
| [L50](../src/core/SessionDiagnostics.h#L50) | `desiredPeerConnectionMaxBitrateBps` | `std::uint64_t desiredPeerConnectionMaxBitrateBps = 0;` | 保存计数、尺寸或速率指标：desired peer connection max bitrate bps。 |
| [L51](../src/core/SessionDiagnostics.h#L51) | `appliedPeerConnectionMaxBitrateBps` | `std::uint64_t appliedPeerConnectionMaxBitrateBps = 0;` | 保存计数、尺寸或速率指标：applied peer connection max bitrate bps。 |
| [L52](../src/core/SessionDiagnostics.h#L52) | `smoothedOutgoingCapacityBps` | `std::uint64_t smoothedOutgoingCapacityBps = 0;` | 保存计数、尺寸或速率指标：smoothed outgoing capacity bps。 |
| [L53](../src/core/SessionDiagnostics.h#L53) | `bitrateCeilingStableSamples` | `std::uint32_t bitrateCeilingStableSamples = 0;` | 保存 bitrate ceiling stable samples 相关配置或运行状态。 |
| [L54](../src/core/SessionDiagnostics.h#L54) | `bitrateCeilingCooldownRemainingMs` | `std::uint32_t bitrateCeilingCooldownRemainingMs = 0;` | 保存 bitrate ceiling cooldown remaining ms 相关配置或运行状态。 |
| [L55](../src/core/SessionDiagnostics.h#L55) | `bitrateCeilingProbePending` | `bool bitrateCeilingProbePending = false;` | 保存待处理队列或请求：bitrate ceiling probe pending。 |
| [L56](../src/core/SessionDiagnostics.h#L56) | `bitrateCeilingLastProbeFailed` | `bool bitrateCeilingLastProbeFailed = false;` | 保存 bitrate ceiling last probe failed 相关配置或运行状态。 |
| [L57](../src/core/SessionDiagnostics.h#L57) | `bitrateCeilingStatus` | `std::string bitrateCeilingStatus;` | 保存状态机当前状态：bitrate ceiling status。 |
| [L58](../src/core/SessionDiagnostics.h#L58) | `bitrateCeilingError` | `std::string bitrateCeilingError;` | 保存最近错误或失败原因：bitrate ceiling error。 |
| [L59](../src/core/SessionDiagnostics.h#L59) | `configuredMaxFrameRate` | `std::uint32_t configuredMaxFrameRate = 0;` | 保存计数、尺寸或速率指标：configured max frame rate。 |
| [L60](../src/core/SessionDiagnostics.h#L60) | `effectiveNetworkFrameRate` | `std::uint32_t effectiveNetworkFrameRate = 0;` | 保存计数、尺寸或速率指标：effective network frame rate。 |
| [L61](../src/core/SessionDiagnostics.h#L61) | `adaptiveNetworkFrameRateEnabled` | `bool adaptiveNetworkFrameRateEnabled = false;` | 保存能力或开关状态：adaptive network frame rate enabled。 |
| [L62](../src/core/SessionDiagnostics.h#L62) | `adaptiveNetworkFrameRateReductionSamples` | `std::uint32_t adaptiveNetworkFrameRateReductionSamples = 0;` | 保存 adaptive network frame rate reduction samples 相关配置或运行状态。 |
| [L63](../src/core/SessionDiagnostics.h#L63) | `adaptiveNetworkFrameRateRecoverySamples` | `std::uint32_t adaptiveNetworkFrameRateRecoverySamples = 0;` | 保存 adaptive network frame rate recovery samples 相关配置或运行状态。 |
| [L64](../src/core/SessionDiagnostics.h#L64) | `adaptiveNetworkFrameRateCapacityBps` | `std::uint64_t adaptiveNetworkFrameRateCapacityBps = 0;` | 保存计数、尺寸或速率指标：adaptive network frame rate capacity bps。 |
| [L65](../src/core/SessionDiagnostics.h#L65) | `adaptiveNetworkFrameRateStatus` | `std::string adaptiveNetworkFrameRateStatus;` | 保存状态机当前状态：adaptive network frame rate status。 |
| [L66](../src/core/SessionDiagnostics.h#L66) | `adaptiveNetworkFrameRateError` | `std::string adaptiveNetworkFrameRateError;` | 保存最近错误或失败原因：adaptive network frame rate error。 |
| [L67](../src/core/SessionDiagnostics.h#L67) | `configuredOutputWidth` | `std::uint32_t configuredOutputWidth = 0;` | 保存计数、尺寸或速率指标：configured output width。 |
| [L68](../src/core/SessionDiagnostics.h#L68) | `configuredOutputHeight` | `std::uint32_t configuredOutputHeight = 0;` | 保存计数、尺寸或速率指标：configured output height。 |
| [L69](../src/core/SessionDiagnostics.h#L69) | `frameWidth` | `std::uint32_t frameWidth = 0;` | 保存计数、尺寸或速率指标：frame width。 |
| [L70](../src/core/SessionDiagnostics.h#L70) | `frameHeight` | `std::uint32_t frameHeight = 0;` | 保存计数、尺寸或速率指标：frame height。 |
| [L71](../src/core/SessionDiagnostics.h#L71) | `framesPerSecond` | `double framesPerSecond = 0.0;` | 保存 frames per second 相关配置或运行状态。 |
| [L72](../src/core/SessionDiagnostics.h#L72) | `encodedFramesPerSecond` | `double encodedFramesPerSecond = 0.0;` | 保存 encoded frames per second 相关配置或运行状态。 |
| [L73](../src/core/SessionDiagnostics.h#L73) | `sentFramesPerSecond` | `double sentFramesPerSecond = 0.0;` | 保存 sent frames per second 相关配置或运行状态。 |
| [L74](../src/core/SessionDiagnostics.h#L74) | `framesEncoded` | `std::uint32_t framesEncoded = 0;` | 保存 frames encoded 相关配置或运行状态。 |
| [L75](../src/core/SessionDiagnostics.h#L75) | `framesSent` | `std::uint32_t framesSent = 0;` | 保存 frames sent 相关配置或运行状态。 |
| [L76](../src/core/SessionDiagnostics.h#L76) | `framesDecoded` | `std::uint32_t framesDecoded = 0;` | 保存 frames decoded 相关配置或运行状态。 |
| [L77](../src/core/SessionDiagnostics.h#L77) | `framesDropped` | `std::uint32_t framesDropped = 0;` | 保存 frames dropped 相关配置或运行状态。 |
| [L78](../src/core/SessionDiagnostics.h#L78) | `keyFrames` | `std::uint32_t keyFrames = 0;` | 保存 key frames 相关配置或运行状态。 |
| [L79](../src/core/SessionDiagnostics.h#L79) | `sourceWidth` | `std::uint32_t sourceWidth = 0;` | 保存计数、尺寸或速率指标：source width。 |
| [L80](../src/core/SessionDiagnostics.h#L80) | `sourceHeight` | `std::uint32_t sourceHeight = 0;` | 保存计数、尺寸或速率指标：source height。 |
| [L81](../src/core/SessionDiagnostics.h#L81) | `sourceFramesPerSecond` | `double sourceFramesPerSecond = 0.0;` | 保存 source frames per second 相关配置或运行状态。 |
| [L82](../src/core/SessionDiagnostics.h#L82) | `captureTargetFrameRate` | `std::uint32_t captureTargetFrameRate = 0;` | 保存计数、尺寸或速率指标：capture target frame rate。 |
| [L83](../src/core/SessionDiagnostics.h#L83) | `captureConfiguredBackend` | `std::string captureConfiguredBackend;` | 保存 capture configured backend 相关配置或运行状态。 |
| [L84](../src/core/SessionDiagnostics.h#L84) | `captureActiveBackend` | `std::string captureActiveBackend;` | 保存 capture active backend 相关配置或运行状态。 |
| [L85](../src/core/SessionDiagnostics.h#L85) | `captureFallbackReason` | `std::string captureFallbackReason;` | 保存 capture fallback reason 相关配置或运行状态。 |
| [L86](../src/core/SessionDiagnostics.h#L86) | `captureActivityState` | `std::string captureActivityState;` | 保存状态机当前状态：capture activity state。 |
| [L87](../src/core/SessionDiagnostics.h#L87) | `captureAdaptiveFrameDeliveryEnabled` | `bool captureAdaptiveFrameDeliveryEnabled = false;` | 保存能力或开关状态：capture adaptive frame delivery enabled。 |
| [L88](../src/core/SessionDiagnostics.h#L88) | `captureAttemptsPerSecond` | `double captureAttemptsPerSecond = 0.0;` | 保存 capture attempts per second 相关配置或运行状态。 |
| [L89](../src/core/SessionDiagnostics.h#L89) | `captureDeliveredFramesPerSecond` | `double captureDeliveredFramesPerSecond = 0.0;` | 保存 capture delivered frames per second 相关配置或运行状态。 |
| [L90](../src/core/SessionDiagnostics.h#L90) | `captureChangedFramesPerSecond` | `double captureChangedFramesPerSecond = 0.0;` | 保存 capture changed frames per second 相关配置或运行状态。 |
| [L91](../src/core/SessionDiagnostics.h#L91) | `captureIdleHeartbeatFramesPerSecond` | `double captureIdleHeartbeatFramesPerSecond = 0.0;` | 保存 capture idle heartbeat frames per second 相关配置或运行状态。 |
| [L92](../src/core/SessionDiagnostics.h#L92) | `captureAttempts` | `std::uint64_t captureAttempts = 0;` | 保存 capture attempts 相关配置或运行状态。 |
| [L93](../src/core/SessionDiagnostics.h#L93) | `captureDeliveredFrames` | `std::uint64_t captureDeliveredFrames = 0;` | 保存 capture delivered frames 相关配置或运行状态。 |
| [L94](../src/core/SessionDiagnostics.h#L94) | `captureChangedFrames` | `std::uint64_t captureChangedFrames = 0;` | 保存 capture changed frames 相关配置或运行状态。 |
| [L95](../src/core/SessionDiagnostics.h#L95) | `captureIdleHeartbeatFrames` | `std::uint64_t captureIdleHeartbeatFrames = 0;` | 保存 capture idle heartbeat frames 相关配置或运行状态。 |
| [L96](../src/core/SessionDiagnostics.h#L96) | `captureSuppressedUnchangedFrames` | `std::uint64_t captureSuppressedUnchangedFrames = 0;` | 保存 capture suppressed unchanged frames 相关配置或运行状态。 |
| [L97](../src/core/SessionDiagnostics.h#L97) | `captureActivityTransitions` | `std::uint64_t captureActivityTransitions = 0;` | 保存 capture activity transitions 相关配置或运行状态。 |
| [L98](../src/core/SessionDiagnostics.h#L98) | `captureFailures` | `std::uint64_t captureFailures = 0;` | 保存 capture failures 相关配置或运行状态。 |
| [L99](../src/core/SessionDiagnostics.h#L99) | `captureInputBoostActive` | `bool captureInputBoostActive = false;` | 保存能力或开关状态：capture input boost active。 |
| [L100](../src/core/SessionDiagnostics.h#L100) | `captureInputBoosts` | `std::uint64_t captureInputBoosts = 0;` | 保存 capture input boosts 相关配置或运行状态。 |
| [L101](../src/core/SessionDiagnostics.h#L101) | `captureForcedRefreshFrames` | `std::uint64_t captureForcedRefreshFrames = 0;` | 保存 capture forced refresh frames 相关配置或运行状态。 |
| [L102](../src/core/SessionDiagnostics.h#L102) | `latestCaptureCallMs` | `double latestCaptureCallMs = 0.0;` | 保存 latest capture call ms 相关配置或运行状态。 |
| [L103](../src/core/SessionDiagnostics.h#L103) | `averageEncodeTimeMs` | `double averageEncodeTimeMs = 0.0;` | 保存 average encode time ms 相关配置或运行状态。 |
| [L104](../src/core/SessionDiagnostics.h#L104) | `averageDecodeTimeMs` | `double averageDecodeTimeMs = 0.0;` | 保存 average decode time ms 相关配置或运行状态。 |
| [L105](../src/core/SessionDiagnostics.h#L105) | `averageProcessingDelayMs` | `double averageProcessingDelayMs = 0.0;` | 保存 average processing delay ms 相关配置或运行状态。 |
| [L106](../src/core/SessionDiagnostics.h#L106) | `averageJitterBufferDelayMs` | `double averageJitterBufferDelayMs = 0.0;` | 保存 average jitter buffer delay ms 相关配置或运行状态。 |
| [L107](../src/core/SessionDiagnostics.h#L107) | `averageQp` | `double averageQp = 0.0;` | 保存 average qp 相关配置或运行状态。 |
| [L108](../src/core/SessionDiagnostics.h#L108) | `latestFrameTimingAvailable` | `bool latestFrameTimingAvailable = false;` | 保存能力或开关状态：latest frame timing available。 |
| [L109](../src/core/SessionDiagnostics.h#L109) | `latestFrameTimeMs` | `double latestFrameTimeMs = 0.0;` | 保存 latest frame time ms 相关配置或运行状态。 |
| [L110](../src/core/SessionDiagnostics.h#L110) | `averageFrameTimingMs` | `double averageFrameTimingMs = 0.0;` | 保存 average frame timing ms 相关配置或运行状态。 |
| [L111](../src/core/SessionDiagnostics.h#L111) | `p95FrameTimingMs` | `double p95FrameTimingMs = 0.0;` | 保存 p95 frame timing ms 相关配置或运行状态。 |
| [L112](../src/core/SessionDiagnostics.h#L112) | `maximumFrameTimingMs` | `double maximumFrameTimingMs = 0.0;` | 保存 maximum frame timing ms 相关配置或运行状态。 |
| [L113](../src/core/SessionDiagnostics.h#L113) | `latestFrameRtpTimestamp` | `std::uint32_t latestFrameRtpTimestamp = 0;` | 保存 latest frame rtp timestamp 相关配置或运行状态。 |
| [L114](../src/core/SessionDiagnostics.h#L114) | `latestFrameTimingAgeMs` | `std::uint32_t latestFrameTimingAgeMs = 0;` | 保存 latest frame timing age ms 相关配置或运行状态。 |
| [L115](../src/core/SessionDiagnostics.h#L115) | `latestFrameWidth` | `std::uint32_t latestFrameWidth = 0;` | 保存计数、尺寸或速率指标：latest frame width。 |
| [L116](../src/core/SessionDiagnostics.h#L116) | `latestFrameHeight` | `std::uint32_t latestFrameHeight = 0;` | 保存计数、尺寸或速率指标：latest frame height。 |
| [L117](../src/core/SessionDiagnostics.h#L117) | `latestEncodedBytes` | `std::uint64_t latestEncodedBytes = 0;` | 保存计数、尺寸或速率指标：latest encoded bytes。 |
| [L118](../src/core/SessionDiagnostics.h#L118) | `latestFrameQpAvailable` | `bool latestFrameQpAvailable = false;` | 保存能力或开关状态：latest frame qp available。 |
| [L119](../src/core/SessionDiagnostics.h#L119) | `latestFrameQp` | `std::int32_t latestFrameQp = 0;` | 保存 latest frame qp 相关配置或运行状态。 |
| [L120](../src/core/SessionDiagnostics.h#L120) | `timingCompletedFrames` | `std::uint64_t timingCompletedFrames = 0;` | 保存 timing completed frames 相关配置或运行状态。 |
| [L121](../src/core/SessionDiagnostics.h#L121) | `timingDroppedFrames` | `std::uint64_t timingDroppedFrames = 0;` | 保存 timing dropped frames 相关配置或运行状态。 |
| [L122](../src/core/SessionDiagnostics.h#L122) | `decodePipelineTimingAvailable` | `bool decodePipelineTimingAvailable = false;` | 保存能力或开关状态：decode pipeline timing available。 |
| [L123](../src/core/SessionDiagnostics.h#L123) | `decodePipelineAsynchronous` | `bool decodePipelineAsynchronous = false;` | 保存 decode pipeline asynchronous 相关配置或运行状态。 |
| [L124](../src/core/SessionDiagnostics.h#L124) | `decodeInputPreparationMs` | `double decodeInputPreparationMs = 0.0;` | 保存 decode input preparation ms 相关配置或运行状态。 |
| [L125](../src/core/SessionDiagnostics.h#L125) | `decodeInputQueueWaitMs` | `double decodeInputQueueWaitMs = 0.0;` | 保存 decode input queue wait ms 相关配置或运行状态。 |
| [L126](../src/core/SessionDiagnostics.h#L126) | `decodeTransformWaitMs` | `double decodeTransformWaitMs = 0.0;` | 保存 decode transform wait ms 相关配置或运行状态。 |
| [L127](../src/core/SessionDiagnostics.h#L127) | `averageDecodeTransformWaitMs` | `double averageDecodeTransformWaitMs = 0.0;` | 保存 average decode transform wait ms 相关配置或运行状态。 |
| [L128](../src/core/SessionDiagnostics.h#L128) | `p95DecodeTransformWaitMs` | `double p95DecodeTransformWaitMs = 0.0;` | 保存 p95 decode transform wait ms 相关配置或运行状态。 |
| [L129](../src/core/SessionDiagnostics.h#L129) | `maximumDecodeTransformWaitMs` | `double maximumDecodeTransformWaitMs = 0.0;` | 保存 maximum decode transform wait ms 相关配置或运行状态。 |
| [L130](../src/core/SessionDiagnostics.h#L130) | `decodeOutputDeliveryMs` | `double decodeOutputDeliveryMs = 0.0;` | 保存 decode output delivery ms 相关配置或运行状态。 |
| [L131](../src/core/SessionDiagnostics.h#L131) | `decodeQueuedInputFrames` | `std::uint32_t decodeQueuedInputFrames = 0;` | 保存 decode queued input frames 相关配置或运行状态。 |
| [L132](../src/core/SessionDiagnostics.h#L132) | `decodeInFlightFrames` | `std::uint32_t decodeInFlightFrames = 0;` | 保存 decode in flight frames 相关配置或运行状态。 |
| [L133](../src/core/SessionDiagnostics.h#L133) | `decodePeakBacklogFrames` | `std::uint32_t decodePeakBacklogFrames = 0;` | 保存 decode peak backlog frames 相关配置或运行状态。 |
| [L134](../src/core/SessionDiagnostics.h#L134) | `decodeBackpressureFallbacks` | `std::uint64_t decodeBackpressureFallbacks = 0;` | 保存 decode backpressure fallbacks 相关配置或运行状态。 |
| [L135](../src/core/SessionDiagnostics.h#L135) | `windowEncodeTimeAvailable` | `bool windowEncodeTimeAvailable = false;` | 保存能力或开关状态：window encode time available。 |
| [L136](../src/core/SessionDiagnostics.h#L136) | `windowEncodeTimeMs` | `double windowEncodeTimeMs = 0.0;` | 保存 window encode time ms 相关配置或运行状态。 |
| [L137](../src/core/SessionDiagnostics.h#L137) | `windowDecodeTimeAvailable` | `bool windowDecodeTimeAvailable = false;` | 保存能力或开关状态：window decode time available。 |
| [L138](../src/core/SessionDiagnostics.h#L138) | `windowDecodeTimeMs` | `double windowDecodeTimeMs = 0.0;` | 保存 window decode time ms 相关配置或运行状态。 |
| [L139](../src/core/SessionDiagnostics.h#L139) | `windowProcessingDelayAvailable` | `bool windowProcessingDelayAvailable = false;` | 保存能力或开关状态：window processing delay available。 |
| [L140](../src/core/SessionDiagnostics.h#L140) | `windowProcessingDelayMs` | `double windowProcessingDelayMs = 0.0;` | 保存 window processing delay ms 相关配置或运行状态。 |
| [L141](../src/core/SessionDiagnostics.h#L141) | `windowJitterBufferDelayAvailable` | `bool windowJitterBufferDelayAvailable = false;` | 保存能力或开关状态：window jitter buffer delay available。 |
| [L142](../src/core/SessionDiagnostics.h#L142) | `windowJitterBufferDelayMs` | `double windowJitterBufferDelayMs = 0.0;` | 保存 window jitter buffer delay ms 相关配置或运行状态。 |
| [L143](../src/core/SessionDiagnostics.h#L143) | `windowQpAvailable` | `bool windowQpAvailable = false;` | 保存能力或开关状态：window qp available。 |
| [L144](../src/core/SessionDiagnostics.h#L144) | `windowQp` | `double windowQp = 0.0;` | 保存 window qp 相关配置或运行状态。 |
| [L145](../src/core/SessionDiagnostics.h#L145) | `audioLevel` | `double audioLevel = 0.0;` | 保存 audio level 相关配置或运行状态。 |
| [L146](../src/core/SessionDiagnostics.h#L146) | `totalAudioSamples` | `std::uint64_t totalAudioSamples = 0;` | 保存 total audio samples 相关配置或运行状态。 |
| [L147](../src/core/SessionDiagnostics.h#L147) | `concealedAudioSamples` | `std::uint64_t concealedAudioSamples = 0;` | 保存 concealed audio samples 相关配置或运行状态。 |
| [L148](../src/core/SessionDiagnostics.h#L148) | `concealmentEvents` | `std::uint64_t concealmentEvents = 0;` | 保存 concealment events 相关配置或运行状态。 |
| [L149](../src/core/SessionDiagnostics.h#L149) | `freezeCount` | `std::uint32_t freezeCount = 0;` | 保存计数、尺寸或速率指标：freeze count。 |
| [L150](../src/core/SessionDiagnostics.h#L150) | `totalFreezeDurationMs` | `double totalFreezeDurationMs = 0.0;` | 保存 total freeze duration ms 相关配置或运行状态。 |
| [L151](../src/core/SessionDiagnostics.h#L151) | `pauseCount` | `std::uint32_t pauseCount = 0;` | 保存计数、尺寸或速率指标：pause count。 |
| [L152](../src/core/SessionDiagnostics.h#L152) | `totalPauseDurationMs` | `double totalPauseDurationMs = 0.0;` | 保存 total pause duration ms 相关配置或运行状态。 |
| [L153](../src/core/SessionDiagnostics.h#L153) | `nackCount` | `std::uint32_t nackCount = 0;` | 保存计数、尺寸或速率指标：nack count。 |
| [L154](../src/core/SessionDiagnostics.h#L154) | `pliCount` | `std::uint32_t pliCount = 0;` | 保存计数、尺寸或速率指标：pli count。 |
| [L155](../src/core/SessionDiagnostics.h#L155) | `firCount` | `std::uint32_t firCount = 0;` | 保存计数、尺寸或速率指标：fir count。 |
| [L156](../src/core/SessionDiagnostics.h#L156) | `presentationTimingAvailable` | `bool presentationTimingAvailable = false;` | 保存能力或开关状态：presentation timing available。 |
| [L157](../src/core/SessionDiagnostics.h#L157) | `presentationPath` | `std::string presentationPath;` | 保存路径、地址或显示名称：presentation path。 |
| [L158](../src/core/SessionDiagnostics.h#L158) | `localRefreshRateHz` | `double localRefreshRateHz = 0.0;` | 保存 local refresh rate hz 相关配置或运行状态。 |
| [L159](../src/core/SessionDiagnostics.h#L159) | `presentationArrivalFramesPerSecond` | `double presentationArrivalFramesPerSecond = 0.0;` | 保存 presentation arrival frames per second 相关配置或运行状态。 |
| [L160](../src/core/SessionDiagnostics.h#L160) | `presentedFramesPerSecond` | `double presentedFramesPerSecond = 0.0;` | 保存 presented frames per second 相关配置或运行状态。 |
| [L161](../src/core/SessionDiagnostics.h#L161) | `presentationArrivedFrames` | `std::uint64_t presentationArrivedFrames = 0;` | 保存 presentation arrived frames 相关配置或运行状态。 |
| [L162](../src/core/SessionDiagnostics.h#L162) | `presentedFrames` | `std::uint64_t presentedFrames = 0;` | 保存 presented frames 相关配置或运行状态。 |
| [L163](../src/core/SessionDiagnostics.h#L163) | `presentationSupersededFrames` | `std::uint64_t presentationSupersededFrames = 0;` | 保存 presentation superseded frames 相关配置或运行状态。 |
| [L164](../src/core/SessionDiagnostics.h#L164) | `presentationConvertedFrames` | `std::uint64_t presentationConvertedFrames = 0;` | 保存 presentation converted frames 相关配置或运行状态。 |
| [L165](../src/core/SessionDiagnostics.h#L165) | `presentationFailures` | `std::uint64_t presentationFailures = 0;` | 保存 presentation failures 相关配置或运行状态。 |
| [L166](../src/core/SessionDiagnostics.h#L166) | `latestPresentationConversionMs` | `double latestPresentationConversionMs = 0.0;` | 保存 latest presentation conversion ms 相关配置或运行状态。 |
| [L167](../src/core/SessionDiagnostics.h#L167) | `averagePresentationConversionMs` | `double averagePresentationConversionMs = 0.0;` | 保存 average presentation conversion ms 相关配置或运行状态。 |
| [L168](../src/core/SessionDiagnostics.h#L168) | `latestPresentationRenderSubmitMs` | `double latestPresentationRenderSubmitMs = 0.0;` | 保存 latest presentation render submit ms 相关配置或运行状态。 |
| [L169](../src/core/SessionDiagnostics.h#L169) | `averagePresentationRenderSubmitMs` | `double averagePresentationRenderSubmitMs = 0.0;` | 保存 average presentation render submit ms 相关配置或运行状态。 |
| [L170](../src/core/SessionDiagnostics.h#L170) | `latestPresentCallMs` | `double latestPresentCallMs = 0.0;` | 保存 latest present call ms 相关配置或运行状态。 |
| [L171](../src/core/SessionDiagnostics.h#L171) | `averagePresentCallMs` | `double averagePresentCallMs = 0.0;` | 保存 average present call ms 相关配置或运行状态。 |
| [L172](../src/core/SessionDiagnostics.h#L172) | `averagePresentedIntervalMs` | `double averagePresentedIntervalMs = 0.0;` | 保存 average presented interval ms 相关配置或运行状态。 |
| [L173](../src/core/SessionDiagnostics.h#L173) | `p95PresentedIntervalMs` | `double p95PresentedIntervalMs = 0.0;` | 保存 p95 presented interval ms 相关配置或运行状态。 |
| [L174](../src/core/SessionDiagnostics.h#L174) | `maximumPresentedIntervalMs` | `double maximumPresentedIntervalMs = 0.0;` | 保存 maximum presented interval ms 相关配置或运行状态。 |
| [L175](../src/core/SessionDiagnostics.h#L175) | `latestReceiverPipelineMs` | `double latestReceiverPipelineMs = 0.0;` | 保存 latest receiver pipeline ms 相关配置或运行状态。 |
| [L176](../src/core/SessionDiagnostics.h#L176) | `averageReceiverPipelineMs` | `double averageReceiverPipelineMs = 0.0;` | 保存 average receiver pipeline ms 相关配置或运行状态。 |
| [L177](../src/core/SessionDiagnostics.h#L177) | `p95ReceiverPipelineMs` | `double p95ReceiverPipelineMs = 0.0;` | 保存 p95 receiver pipeline ms 相关配置或运行状态。 |
| [L178](../src/core/SessionDiagnostics.h#L178) | `maximumReceiverPipelineMs` | `double maximumReceiverPipelineMs = 0.0;` | 保存 maximum receiver pipeline ms 相关配置或运行状态。 |
| [L179](../src/core/SessionDiagnostics.h#L179) | `active` | `bool active = false;` | 保存能力或开关状态：active。 |
| [L180](../src/core/SessionDiagnostics.h#L180) | `powerEfficient` | `bool powerEfficient = false;` | 保存 power efficient 相关配置或运行状态。 |
| [L184](../src/core/SessionDiagnostics.h#L184) | `label` | `std::string label;` | 保存路径、地址或显示名称：label。 |
| [L185](../src/core/SessionDiagnostics.h#L185) | `protocol` | `std::string protocol;` | 保存 protocol 相关配置或运行状态。 |
| [L186](../src/core/SessionDiagnostics.h#L186) | `state` | `std::string state;` | 保存状态机当前状态：state。 |
| [L187](../src/core/SessionDiagnostics.h#L187) | `bytesSent` | `std::uint64_t bytesSent = 0;` | 保存 bytes sent 相关配置或运行状态。 |
| [L188](../src/core/SessionDiagnostics.h#L188) | `bytesReceived` | `std::uint64_t bytesReceived = 0;` | 保存 bytes received 相关配置或运行状态。 |
| [L189](../src/core/SessionDiagnostics.h#L189) | `sendBitrateBps` | `std::uint64_t sendBitrateBps = 0;` | 保存计数、尺寸或速率指标：send bitrate bps。 |
| [L190](../src/core/SessionDiagnostics.h#L190) | `receiveBitrateBps` | `std::uint64_t receiveBitrateBps = 0;` | 保存计数、尺寸或速率指标：receive bitrate bps。 |
| [L191](../src/core/SessionDiagnostics.h#L191) | `sampleWindowMs` | `std::uint32_t sampleWindowMs = 0;` | 保存 sample window ms 相关配置或运行状态。 |
| [L192](../src/core/SessionDiagnostics.h#L192) | `bufferedAmountBytes` | `std::uint64_t bufferedAmountBytes = 0;` | 保存计数、尺寸或速率指标：buffered amount bytes。 |
| [L193](../src/core/SessionDiagnostics.h#L193) | `messagesSent` | `std::uint32_t messagesSent = 0;` | 保存 messages sent 相关配置或运行状态。 |
| [L194](../src/core/SessionDiagnostics.h#L194) | `messagesReceived` | `std::uint32_t messagesReceived = 0;` | 保存 messages received 相关配置或运行状态。 |
| [L198](../src/core/SessionDiagnostics.h#L198) | `candidateType` | `std::string candidateType;` | 保存 candidate type 相关配置或运行状态。 |
| [L199](../src/core/SessionDiagnostics.h#L199) | `protocol` | `std::string protocol;` | 保存 protocol 相关配置或运行状态。 |
| [L200](../src/core/SessionDiagnostics.h#L200) | `address` | `std::string address;` | 保存 address 相关配置或运行状态。 |
| [L201](../src/core/SessionDiagnostics.h#L201) | `port` | `std::int32_t port = 0;` | 保存 port 相关配置或运行状态。 |
| [L202](../src/core/SessionDiagnostics.h#L202) | `networkType` | `std::string networkType;` | 保存 network type 相关配置或运行状态。 |
| [L203](../src/core/SessionDiagnostics.h#L203) | `adapterType` | `std::string adapterType;` | 保存 adapter type 相关配置或运行状态。 |
| [L204](../src/core/SessionDiagnostics.h#L204) | `relayProtocol` | `std::string relayProtocol;` | 保存 relay protocol 相关配置或运行状态。 |
| [L208](../src/core/SessionDiagnostics.h#L208) | `collected` | `bool collected = false;` | 保存 collected 相关配置或运行状态。 |
| [L209](../src/core/SessionDiagnostics.h#L209) | `timestampMs` | `std::int64_t timestampMs = 0;` | 保存 timestamp ms 相关配置或运行状态。 |
| [L210](../src/core/SessionDiagnostics.h#L210) | `iceState` | `std::string iceState;` | 保存状态机当前状态：ice state。 |
| [L211](../src/core/SessionDiagnostics.h#L211) | `dtlsState` | `std::string dtlsState;` | 保存状态机当前状态：dtls state。 |
| [L212](../src/core/SessionDiagnostics.h#L212) | `iceRole` | `std::string iceRole;` | 保存 ice role 相关配置或运行状态。 |
| [L213](../src/core/SessionDiagnostics.h#L213) | `candidatePairState` | `std::string candidatePairState;` | 保存状态机当前状态：candidate pair state。 |
| [L214](../src/core/SessionDiagnostics.h#L214) | `routeType` | `std::string routeType;` | 保存 route type 相关配置或运行状态。 |
| [L215](../src/core/SessionDiagnostics.h#L215) | `tlsVersion` | `std::string tlsVersion;` | 保存 tls version 相关配置或运行状态。 |
| [L216](../src/core/SessionDiagnostics.h#L216) | `dtlsCipher` | `std::string dtlsCipher;` | 保存 dtls cipher 相关配置或运行状态。 |
| [L217](../src/core/SessionDiagnostics.h#L217) | `srtpCipher` | `std::string srtpCipher;` | 保存 srtp cipher 相关配置或运行状态。 |
| [L218](../src/core/SessionDiagnostics.h#L218) | `localCandidate` | `IceCandidateStatsSnapshot localCandidate;` | 保存 local candidate 相关配置或运行状态。 |
| [L219](../src/core/SessionDiagnostics.h#L219) | `remoteCandidate` | `IceCandidateStatsSnapshot remoteCandidate;` | 保存 remote candidate 相关配置或运行状态。 |
| [L220](../src/core/SessionDiagnostics.h#L220) | `nominated` | `bool nominated = false;` | 保存 nominated 相关配置或运行状态。 |
| [L221](../src/core/SessionDiagnostics.h#L221) | `writable` | `bool writable = false;` | 保存 writable 相关配置或运行状态。 |
| [L222](../src/core/SessionDiagnostics.h#L222) | `selectedCandidatePairChanges` | `std::uint32_t selectedCandidatePairChanges = 0;` | 保存 selected candidate pair changes 相关配置或运行状态。 |
| [L223](../src/core/SessionDiagnostics.h#L223) | `packetsDiscardedOnSend` | `std::uint64_t packetsDiscardedOnSend = 0;` | 保存 packets discarded on send 相关配置或运行状态。 |
| [L224](../src/core/SessionDiagnostics.h#L224) | `bytesDiscardedOnSend` | `std::uint64_t bytesDiscardedOnSend = 0;` | 保存 bytes discarded on send 相关配置或运行状态。 |
| [L225](../src/core/SessionDiagnostics.h#L225) | `currentRoundTripTimeMs` | `double currentRoundTripTimeMs = 0.0;` | 保存 current round trip time ms 相关配置或运行状态。 |
| [L226](../src/core/SessionDiagnostics.h#L226) | `availableOutgoingBitrateBps` | `std::uint64_t availableOutgoingBitrateBps = 0;` | 保存计数、尺寸或速率指标：available outgoing bitrate bps。 |
| [L227](../src/core/SessionDiagnostics.h#L227) | `availableIncomingBitrateBps` | `std::uint64_t availableIncomingBitrateBps = 0;` | 保存计数、尺寸或速率指标：available incoming bitrate bps。 |
| [L228](../src/core/SessionDiagnostics.h#L228) | `sendBitrateBps` | `std::uint64_t sendBitrateBps = 0;` | 保存计数、尺寸或速率指标：send bitrate bps。 |
| [L229](../src/core/SessionDiagnostics.h#L229) | `receiveBitrateBps` | `std::uint64_t receiveBitrateBps = 0;` | 保存计数、尺寸或速率指标：receive bitrate bps。 |
| [L230](../src/core/SessionDiagnostics.h#L230) | `sampleWindowMs` | `std::uint32_t sampleWindowMs = 0;` | 保存 sample window ms 相关配置或运行状态。 |
| [L231](../src/core/SessionDiagnostics.h#L231) | `bytesSent` | `std::uint64_t bytesSent = 0;` | 保存 bytes sent 相关配置或运行状态。 |
| [L232](../src/core/SessionDiagnostics.h#L232) | `bytesReceived` | `std::uint64_t bytesReceived = 0;` | 保存 bytes received 相关配置或运行状态。 |
| [L236](../src/core/SessionDiagnostics.h#L236) | `transport` | `WebRtcTransportStatsSnapshot transport;` | 保存 transport 相关配置或运行状态。 |
| [L237](../src/core/SessionDiagnostics.h#L237) | `rtpStreams` | `std::vector<RtpStreamStatsSnapshot> rtpStreams;` | 保存 rtp streams 相关配置或运行状态。 |
| [L238](../src/core/SessionDiagnostics.h#L238) | `dataChannels` | `std::vector<DataChannelStatsSnapshot> dataChannels;` | 保存 data channels 相关配置或运行状态。 |
| [L242](../src/core/SessionDiagnostics.h#L242) | `pairId` | `std::string pairId;` | 保存身份或作用域标识：pair id。 |
| [L243](../src/core/SessionDiagnostics.h#L243) | `peerDeviceId` | `std::string peerDeviceId;` | 保存身份或作用域标识：peer device id。 |
| [L244](../src/core/SessionDiagnostics.h#L244) | `stats` | `WebRtcSessionStatsSnapshot stats;` | 保存 stats 相关配置或运行状态。 |
| [L249](../src/core/SessionDiagnostics.h#L249) | `publishing` | `bool publishing = false;` | 保存 publishing 相关配置或运行状态。 |
| [L250](../src/core/SessionDiagnostics.h#L250) | `shapeAvailable` | `bool shapeAvailable = false;` | 保存能力或开关状态：shape available。 |
| [L251](../src/core/SessionDiagnostics.h#L251) | `displayId` | `std::uint32_t displayId = 0;` | 保存身份或作用域标识：display id。 |
| [L252](../src/core/SessionDiagnostics.h#L252) | `displayLayoutVersion` | `std::uint64_t displayLayoutVersion = 0;` | 保存 display layout version 相关配置或运行状态。 |
| [L253](../src/core/SessionDiagnostics.h#L253) | `shapeId` | `std::uint64_t shapeId = 0;` | 保存身份或作用域标识：shape id。 |
| [L254](../src/core/SessionDiagnostics.h#L254) | `positionMessagesPublished` | `std::uint64_t positionMessagesPublished = 0;` | 保存 position messages published 相关配置或运行状态。 |
| [L255](../src/core/SessionDiagnostics.h#L255) | `shapeMessagesPublished` | `std::uint64_t shapeMessagesPublished = 0;` | 保存 shape messages published 相关配置或运行状态。 |
| [L256](../src/core/SessionDiagnostics.h#L256) | `positionMessagesReceived` | `std::uint64_t positionMessagesReceived = 0;` | 保存 position messages received 相关配置或运行状态。 |
| [L257](../src/core/SessionDiagnostics.h#L257) | `shapeMessagesReceived` | `std::uint64_t shapeMessagesReceived = 0;` | 保存 shape messages received 相关配置或运行状态。 |
| [L258](../src/core/SessionDiagnostics.h#L258) | `lastAppliedInputSequence` | `std::uint64_t lastAppliedInputSequence = 0;` | 保存单调序号，用于排序或去重：last applied input sequence。 |
| [L259](../src/core/SessionDiagnostics.h#L259) | `remoteCursor` | `} remoteCursor;` | 保存 remote cursor 相关配置或运行状态。 |
| [L260](../src/core/SessionDiagnostics.h#L260) | `collectedAtMs` | `std::int64_t collectedAtMs = 0;` | 保存 collected at ms 相关配置或运行状态。 |
| [L265](../src/core/SessionDiagnostics.h#L265) | `videoEncoderLastFallbackReason` | `std::string videoEncoderLastFallbackReason;` | Most recent reason reported by a hardware video encoder before asking libwebrtc to use its software fallback. Keep this in the session diagnostics as well as the global capabili... |
| [L266](../src/core/SessionDiagnostics.h#L266) | `peerConnections` | `std::vector<PeerConnectionDiagnosticsSnapshot> peerConnections;` | 保存 peer connections 相关配置或运行状态。 |
| [L267](../src/core/SessionDiagnostics.h#L267) | `remoteInput` | `RemoteInputTelemetrySnapshot remoteInput;` | 保存 remote input 相关配置或运行状态。 |

## `src/core/VideoPresentationTelemetry.h`

[打开源码](../src/core/VideoPresentationTelemetry.h) · **文件作用：** 声明 video presentation telemetry 相关类型、接口、配置和成员状态。

### 类型

| 行 | 类型 | 种类 | 作用 |
|---:|---|---|---|
| [L19](../src/core/VideoPresentationTelemetry.h#L19) | `VideoPresentationPath` | enum class | 定义 VideoPresentationPath 的 enum class 类型和相关状态。 |
| [L28](../src/core/VideoPresentationTelemetry.h#L28) | `VideoPresentationTelemetrySnapshot` | struct | 定义 VideoPresentationTelemetrySnapshot 的 struct 类型和相关状态。 |
| [L57](../src/core/VideoPresentationTelemetry.h#L57) | `VideoPresentationTelemetryRegistry` | class | Application-side video display telemetry. Renderers only publish narrowly defined timing events; the diagnostics layer consumes immutable snapshots. This keeps Qt/D3D implementa... |
| [L230](../src/core/VideoPresentationTelemetry.h#L230) | `Entry` | struct | 定义 Entry 的 struct 类型和相关状态。 |

### 成员与文件级变量

| 行 | 变量 | 声明 | 作用 |
|---:|---|---|---|
| [L29](../src/core/VideoPresentationTelemetry.h#L29) | `available` | `bool available = false;` | 保存能力或开关状态：available。 |
| [L30](../src/core/VideoPresentationTelemetry.h#L30) | `path` | `VideoPresentationPath path = VideoPresentationPath::kUnknown;` | 保存路径、地址或显示名称：path。 |
| [L31](../src/core/VideoPresentationTelemetry.h#L31) | `localRefreshRateHz` | `double localRefreshRateHz = 0.0;` | 保存 local refresh rate hz 相关配置或运行状态。 |
| [L32](../src/core/VideoPresentationTelemetry.h#L32) | `arrivalFramesPerSecond` | `double arrivalFramesPerSecond = 0.0;` | 保存 arrival frames per second 相关配置或运行状态。 |
| [L33](../src/core/VideoPresentationTelemetry.h#L33) | `presentedFramesPerSecond` | `double presentedFramesPerSecond = 0.0;` | 保存 presented frames per second 相关配置或运行状态。 |
| [L34](../src/core/VideoPresentationTelemetry.h#L34) | `arrivedFrames` | `std::uint64_t arrivedFrames = 0;` | 保存 arrived frames 相关配置或运行状态。 |
| [L35](../src/core/VideoPresentationTelemetry.h#L35) | `presentedFrames` | `std::uint64_t presentedFrames = 0;` | 保存 presented frames 相关配置或运行状态。 |
| [L36](../src/core/VideoPresentationTelemetry.h#L36) | `supersededFrames` | `std::uint64_t supersededFrames = 0;` | 保存 superseded frames 相关配置或运行状态。 |
| [L37](../src/core/VideoPresentationTelemetry.h#L37) | `conversionFrames` | `std::uint64_t conversionFrames = 0;` | 保存 conversion frames 相关配置或运行状态。 |
| [L38](../src/core/VideoPresentationTelemetry.h#L38) | `presentFailures` | `std::uint64_t presentFailures = 0;` | 保存 present failures 相关配置或运行状态。 |
| [L39](../src/core/VideoPresentationTelemetry.h#L39) | `latestConversionMs` | `double latestConversionMs = 0.0;` | 保存 latest conversion ms 相关配置或运行状态。 |
| [L40](../src/core/VideoPresentationTelemetry.h#L40) | `averageConversionMs` | `double averageConversionMs = 0.0;` | 保存 average conversion ms 相关配置或运行状态。 |
| [L41](../src/core/VideoPresentationTelemetry.h#L41) | `latestRenderSubmitMs` | `double latestRenderSubmitMs = 0.0;` | 保存 latest render submit ms 相关配置或运行状态。 |
| [L42](../src/core/VideoPresentationTelemetry.h#L42) | `averageRenderSubmitMs` | `double averageRenderSubmitMs = 0.0;` | 保存 average render submit ms 相关配置或运行状态。 |
| [L43](../src/core/VideoPresentationTelemetry.h#L43) | `latestPresentCallMs` | `double latestPresentCallMs = 0.0;` | 保存 latest present call ms 相关配置或运行状态。 |
| [L44](../src/core/VideoPresentationTelemetry.h#L44) | `averagePresentCallMs` | `double averagePresentCallMs = 0.0;` | 保存 average present call ms 相关配置或运行状态。 |
| [L45](../src/core/VideoPresentationTelemetry.h#L45) | `averagePresentedIntervalMs` | `double averagePresentedIntervalMs = 0.0;` | 保存 average presented interval ms 相关配置或运行状态。 |
| [L46](../src/core/VideoPresentationTelemetry.h#L46) | `p95PresentedIntervalMs` | `double p95PresentedIntervalMs = 0.0;` | 保存 p95 presented interval ms 相关配置或运行状态。 |
| [L47](../src/core/VideoPresentationTelemetry.h#L47) | `maximumPresentedIntervalMs` | `double maximumPresentedIntervalMs = 0.0;` | 保存 maximum presented interval ms 相关配置或运行状态。 |
| [L48](../src/core/VideoPresentationTelemetry.h#L48) | `latestReceiverPipelineMs` | `double latestReceiverPipelineMs = 0.0;` | 保存 latest receiver pipeline ms 相关配置或运行状态。 |
| [L49](../src/core/VideoPresentationTelemetry.h#L49) | `averageReceiverPipelineMs` | `double averageReceiverPipelineMs = 0.0;` | 保存 average receiver pipeline ms 相关配置或运行状态。 |
| [L50](../src/core/VideoPresentationTelemetry.h#L50) | `p95ReceiverPipelineMs` | `double p95ReceiverPipelineMs = 0.0;` | 保存 p95 receiver pipeline ms 相关配置或运行状态。 |
| [L51](../src/core/VideoPresentationTelemetry.h#L51) | `maximumReceiverPipelineMs` | `double maximumReceiverPipelineMs = 0.0;` | 保存 maximum receiver pipeline ms 相关配置或运行状态。 |
| [L228](../src/core/VideoPresentationTelemetry.h#L228) | `kTimingWindowSize` | `static constexpr std::size_t kTimingWindowSize = 240;` | 定义 timing window size 的编译期常量或产品边界。 |
| [L231](../src/core/VideoPresentationTelemetry.h#L231) | `peerDeviceId` | `std::string peerDeviceId;` | 保存身份或作用域标识：peer device id。 |
| [L232](../src/core/VideoPresentationTelemetry.h#L232) | `path` | `VideoPresentationPath path = VideoPresentationPath::kUnknown;` | 保存路径、地址或显示名称：path。 |
| [L233](../src/core/VideoPresentationTelemetry.h#L233) | `localRefreshRateHz` | `double localRefreshRateHz = 0.0;` | 保存 local refresh rate hz 相关配置或运行状态。 |
| [L234](../src/core/VideoPresentationTelemetry.h#L234) | `arrivedFrames` | `std::uint64_t arrivedFrames = 0;` | 保存 arrived frames 相关配置或运行状态。 |
| [L235](../src/core/VideoPresentationTelemetry.h#L235) | `presentedFrames` | `std::uint64_t presentedFrames = 0;` | 保存 presented frames 相关配置或运行状态。 |
| [L236](../src/core/VideoPresentationTelemetry.h#L236) | `supersededFrames` | `std::uint64_t supersededFrames = 0;` | 保存 superseded frames 相关配置或运行状态。 |
| [L237](../src/core/VideoPresentationTelemetry.h#L237) | `conversionFrames` | `std::uint64_t conversionFrames = 0;` | 保存 conversion frames 相关配置或运行状态。 |
| [L238](../src/core/VideoPresentationTelemetry.h#L238) | `presentFailures` | `std::uint64_t presentFailures = 0;` | 保存 present failures 相关配置或运行状态。 |
| [L239](../src/core/VideoPresentationTelemetry.h#L239) | `registeredAt` | `Clock::time_point registeredAt{};` | 保存 registered at 相关配置或运行状态。 |
| [L240](../src/core/VideoPresentationTelemetry.h#L240) | `lastPresentedAt` | `Clock::time_point lastPresentedAt{};` | 保存 last presented at 相关配置或运行状态。 |
| [L241](../src/core/VideoPresentationTelemetry.h#L241) | `arrivalTimes` | `std::deque<Clock::time_point> arrivalTimes;` | 保存 arrival times 相关配置或运行状态。 |
| [L242](../src/core/VideoPresentationTelemetry.h#L242) | `presentedTimes` | `std::deque<Clock::time_point> presentedTimes;` | 保存 presented times 相关配置或运行状态。 |
| [L243](../src/core/VideoPresentationTelemetry.h#L243) | `conversionTimesUs` | `std::deque<std::uint64_t> conversionTimesUs;` | 保存 conversion times us 相关配置或运行状态。 |
| [L244](../src/core/VideoPresentationTelemetry.h#L244) | `videoProcessorSubmitTimesUs` | `std::deque<std::uint64_t> videoProcessorSubmitTimesUs;` | 保存 video processor submit times us 相关配置或运行状态。 |
| [L245](../src/core/VideoPresentationTelemetry.h#L245) | `presentCallTimesUs` | `std::deque<std::uint64_t> presentCallTimesUs;` | 保存 present call times us 相关配置或运行状态。 |
| [L246](../src/core/VideoPresentationTelemetry.h#L246) | `presentedIntervalsUs` | `std::deque<std::uint64_t> presentedIntervalsUs;` | 保存 presented intervals us 相关配置或运行状态。 |
| [L247](../src/core/VideoPresentationTelemetry.h#L247) | `receiverPipelineTimesUs` | `std::deque<std::uint64_t> receiverPipelineTimesUs;` | 保存 receiver pipeline times us 相关配置或运行状态。 |
| [L328](../src/core/VideoPresentationTelemetry.h#L328) | `mutex_` | `mutable std::mutex mutex_;` | 保护跨线程共享状态：mutex。 |
| [L329](../src/core/VideoPresentationTelemetry.h#L329) | `nextInstanceId_` | `std::uint64_t nextInstanceId_ = 1;` | 保存身份或作用域标识：next instance id。 |
| [L330](../src/core/VideoPresentationTelemetry.h#L330) | `entries_` | `std::unordered_map<std::uint64_t, Entry> entries_;` | 保存 entries 相关配置或运行状态。 |

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L59](../src/core/VideoPresentationTelemetry.h#L59) | `Instance` | 定义 | `static VideoPresentationTelemetryRegistry& Instance()` | 实现 instance 对应的业务或工具逻辑。 |
| [L65](../src/core/VideoPresentationTelemetry.h#L65) | `Register` | 定义 | `std::uint64_t Register(std::string peerDeviceId)` | 实现 register 对应的业务或工具逻辑。 |
| [L75](../src/core/VideoPresentationTelemetry.h#L75) | `Unregister` | 定义 | `void Unregister(std::uint64_t id)` | 实现 unregister 对应的业务或工具逻辑。 |
| [L81](../src/core/VideoPresentationTelemetry.h#L81) | `SetLocalRefreshRate` | 定义 | `void SetLocalRefreshRate(std::uint64_t id, double refreshRateHz)` | 更新或应用 set local refresh rate 相关逻辑。 |
| [L89](../src/core/VideoPresentationTelemetry.h#L89) | `RecordArrival` | 定义 | `void RecordArrival(std::uint64_t id, VideoPresentationPath path)` | 实现 record arrival 对应的业务或工具逻辑。 |
| [L100](../src/core/VideoPresentationTelemetry.h#L100) | `RecordSuperseded` | 定义 | `void RecordSuperseded(std::uint64_t id)` | 实现 record superseded 对应的业务或工具逻辑。 |
| [L112](../src/core/VideoPresentationTelemetry.h#L112) | `RecordReceiverPipeline` | 定义 | `void RecordReceiverPipeline(std::uint64_t id, std::uint64_t durationUs)` | Time from the final RTP packet of a frame reaching this receiver to the decoded frame reaching the application sink. Both timestamps use libwebrtc's local monotonic clock, so th... |
| [L120](../src/core/VideoPresentationTelemetry.h#L120) | `RecordConversion` | 定义 | `void RecordConversion(std::uint64_t id, std::uint64_t durationUs)` | 实现 record conversion 对应的业务或工具逻辑。 |
| [L129](../src/core/VideoPresentationTelemetry.h#L129) | `RecordPresented` | 定义 | `void RecordPresented(std::uint64_t id, VideoPresentationPath path, std::uint64_t videoProcessorSubmitUs, std::uint64_t presentCallUs)` | 实现 record presented 对应的业务或工具逻辑。 |
| [L157](../src/core/VideoPresentationTelemetry.h#L157) | `RecordPresentFailure` | 定义 | `void RecordPresentFailure(std::uint64_t id)` | 实现 record present failure 对应的业务或工具逻辑。 |
| [L165](../src/core/VideoPresentationTelemetry.h#L165) | `SnapshotForPeer` | 定义 | `std::optional<VideoPresentationTelemetrySnapshot> SnapshotForPeer( const std::string& peerDeviceId) const` | 查询并返回 snapshot for peer 相关逻辑。 |
| [L250](../src/core/VideoPresentationTelemetry.h#L250) | `Find` | 定义 | `Entry* Find(std::uint64_t id)` | 查询并返回 find 相关逻辑。 |
| [L256](../src/core/VideoPresentationTelemetry.h#L256) | `PushTimestamp` | 定义 | `static void PushTimestamp( std::deque<Clock::time_point>& values, Clock::time_point value)` | 实现 push timestamp 对应的业务或工具逻辑。 |
| [L266](../src/core/VideoPresentationTelemetry.h#L266) | `PushDuration` | 定义 | `static void PushDuration( std::deque<std::uint64_t>& values, std::uint64_t value)` | 实现 push duration 对应的业务或工具逻辑。 |
| [L276](../src/core/VideoPresentationTelemetry.h#L276) | `TrailingRate` | 定义 | `static double TrailingRate( const std::deque<Clock::time_point>& values, Clock::time_point now)` | 实现 trailing rate 对应的业务或工具逻辑。 |
| [L286](../src/core/VideoPresentationTelemetry.h#L286) | `LatestMs` | 定义 | `static double LatestMs(const std::deque<std::uint64_t>& values)` | 实现 latest ms 对应的业务或工具逻辑。 |
| [L292](../src/core/VideoPresentationTelemetry.h#L292) | `AverageMs` | 定义 | `static double AverageMs(const std::deque<std::uint64_t>& values)` | 实现 average ms 对应的业务或工具逻辑。 |
| [L304](../src/core/VideoPresentationTelemetry.h#L304) | `MaximumMs` | 定义 | `static double MaximumMs(const std::deque<std::uint64_t>& values)` | 实现 maximum ms 对应的业务或工具逻辑。 |
| [L313](../src/core/VideoPresentationTelemetry.h#L313) | `PercentileMs` | 定义 | `static double PercentileMs( const std::deque<std::uint64_t>& values, double percentile)` | 实现 percentile ms 对应的业务或工具逻辑。 |

## `src/protocol/BinaryProtocol.cpp`

[打开源码](../src/protocol/BinaryProtocol.cpp) · **文件作用：** 实现 binary protocol 相关函数与文件级辅助逻辑。

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L10](../src/protocol/BinaryProtocol.cpp#L10) | `BinaryProtocolWriter::WriteU8` | 定义 | `void BinaryProtocolWriter::WriteU8(std::uint8_t value)` | 保存或写入 write u8 相关逻辑。 |
| [L17](../src/protocol/BinaryProtocol.cpp#L17) | `BinaryProtocolWriter::WriteU16` | 定义 | `void BinaryProtocolWriter::WriteU16(std::uint16_t value)` | 保存或写入 write u16 相关逻辑。 |
| [L26](../src/protocol/BinaryProtocol.cpp#L26) | `BinaryProtocolWriter::WriteU32` | 定义 | `void BinaryProtocolWriter::WriteU32(std::uint32_t value)` | 保存或写入 write u32 相关逻辑。 |
| [L37](../src/protocol/BinaryProtocol.cpp#L37) | `BinaryProtocolWriter::WriteU64` | 定义 | `void BinaryProtocolWriter::WriteU64(std::uint64_t value)` | 保存或写入 write u64 相关逻辑。 |
| [L48](../src/protocol/BinaryProtocol.cpp#L48) | `BinaryProtocolWriter::WriteString` | 定义 | `void BinaryProtocolWriter::WriteString(const std::string& value)` | 保存或写入 write string 相关逻辑。 |
| [L55](../src/protocol/BinaryProtocol.cpp#L55) | `BinaryProtocolWriter::WriteBytes` | 定义 | `void BinaryProtocolWriter::WriteBytes( std::span<const std::uint8_t> value)` | 保存或写入 write bytes 相关逻辑。 |
| [L63](../src/protocol/BinaryProtocol.cpp#L63) | `BinaryProtocolReader::ReadU8` | 定义 | `bool BinaryProtocolReader::ReadU8(std::uint8_t* value)` | 读取或恢复 read u8 相关逻辑。 |
| [L72](../src/protocol/BinaryProtocol.cpp#L72) | `BinaryProtocolReader::ReadU16` | 定义 | `bool BinaryProtocolReader::ReadU16(std::uint16_t* value)` | 读取或恢复 read u16 相关逻辑。 |
| [L84](../src/protocol/BinaryProtocol.cpp#L84) | `BinaryProtocolReader::ReadU32` | 定义 | `bool BinaryProtocolReader::ReadU32(std::uint32_t* value)` | 读取或恢复 read u32 相关逻辑。 |
| [L97](../src/protocol/BinaryProtocol.cpp#L97) | `BinaryProtocolReader::ReadU64` | 定义 | `bool BinaryProtocolReader::ReadU64(std::uint64_t* value)` | 读取或恢复 read u64 相关逻辑。 |
| [L110](../src/protocol/BinaryProtocol.cpp#L110) | `BinaryProtocolReader::ReadString` | 定义 | `bool BinaryProtocolReader::ReadString( std::size_t size, std::string* value)` | 读取或恢复 read string 相关逻辑。 |
| [L123](../src/protocol/BinaryProtocol.cpp#L123) | `BinaryProtocolReader::ReadBytes` | 定义 | `bool BinaryProtocolReader::ReadBytes( std::size_t size, std::vector<std::uint8_t>* value)` | 读取或恢复 read bytes 相关逻辑。 |
| [L136](../src/protocol/BinaryProtocol.cpp#L136) | `BinaryProtocolReader::ReadBytes` | 定义 | `bool BinaryProtocolReader::ReadBytes(std::span<std::uint8_t> value)` | 读取或恢复 read bytes 相关逻辑。 |
| [L146](../src/protocol/BinaryProtocol.cpp#L146) | `BinaryProtocolReader::Skip` | 定义 | `bool BinaryProtocolReader::Skip(std::size_t size)` | 实现 skip 对应的业务或工具逻辑。 |

## `src/protocol/BinaryProtocol.h`

[打开源码](../src/protocol/BinaryProtocol.h) · **文件作用：** 声明 binary protocol 相关类型、接口、配置和成员状态。

### 类型

| 行 | 类型 | 种类 | 作用 |
|---:|---|---|---|
| [L15](../src/protocol/BinaryProtocol.h#L15) | `BinaryProtocolWriter` | class | 定义 BinaryProtocolWriter 的 class 类型和相关状态。 |
| [L34](../src/protocol/BinaryProtocol.h#L34) | `BinaryProtocolReader` | class | 定义 BinaryProtocolReader 的 class 类型和相关状态。 |

### 成员与文件级变量

| 行 | 变量 | 声明 | 作用 |
|---:|---|---|---|
| [L31](../src/protocol/BinaryProtocol.h#L31) | `output_` | `std::vector<std::uint8_t>* output_ = nullptr;` | 保存 output 相关配置或运行状态。 |
| [L59](../src/protocol/BinaryProtocol.h#L59) | `bytes_` | `std::span<const std::uint8_t> bytes_;` | 保存计数、尺寸或速率指标：bytes。 |
| [L60](../src/protocol/BinaryProtocol.h#L60) | `offset_` | `std::size_t offset_ = 0;` | 保存 offset 相关配置或运行状态。 |

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L17](../src/protocol/BinaryProtocol.h#L17) | `BinaryProtocolWriter` | 定义 | `explicit BinaryProtocolWriter(std::vector<std::uint8_t>* output) : output_(output) {}` | 实现 binary protocol writer 对应的业务或工具逻辑。 |
| [L21](../src/protocol/BinaryProtocol.h#L21) | `valid` | 定义 | `bool valid() const { return output_ != nullptr; }` | 判断 valid 相关逻辑。 |
| [L23](../src/protocol/BinaryProtocol.h#L23) | `WriteU8` | 声明 | `void WriteU8(std::uint8_t value)` | 保存或写入 write u8 相关逻辑。 |
| [L24](../src/protocol/BinaryProtocol.h#L24) | `WriteU16` | 声明 | `void WriteU16(std::uint16_t value)` | 保存或写入 write u16 相关逻辑。 |
| [L25](../src/protocol/BinaryProtocol.h#L25) | `WriteU32` | 声明 | `void WriteU32(std::uint32_t value)` | 保存或写入 write u32 相关逻辑。 |
| [L26](../src/protocol/BinaryProtocol.h#L26) | `WriteU64` | 声明 | `void WriteU64(std::uint64_t value)` | 保存或写入 write u64 相关逻辑。 |
| [L27](../src/protocol/BinaryProtocol.h#L27) | `WriteString` | 声明 | `void WriteString(const std::string& value)` | 保存或写入 write string 相关逻辑。 |
| [L28](../src/protocol/BinaryProtocol.h#L28) | `WriteBytes` | 声明 | `void WriteBytes(std::span<const std::uint8_t> value)` | 保存或写入 write bytes 相关逻辑。 |
| [L36](../src/protocol/BinaryProtocol.h#L36) | `BinaryProtocolReader` | 定义 | `explicit BinaryProtocolReader(std::span<const std::uint8_t> bytes) : bytes_(bytes) {}` | 实现 binary protocol reader 对应的业务或工具逻辑。 |
| [L40](../src/protocol/BinaryProtocol.h#L40) | `ReadU8` | 声明 | `bool ReadU8(std::uint8_t* value)` | 读取或恢复 read u8 相关逻辑。 |
| [L41](../src/protocol/BinaryProtocol.h#L41) | `ReadU16` | 声明 | `bool ReadU16(std::uint16_t* value)` | 读取或恢复 read u16 相关逻辑。 |
| [L42](../src/protocol/BinaryProtocol.h#L42) | `ReadU32` | 声明 | `bool ReadU32(std::uint32_t* value)` | 读取或恢复 read u32 相关逻辑。 |
| [L43](../src/protocol/BinaryProtocol.h#L43) | `ReadU64` | 声明 | `bool ReadU64(std::uint64_t* value)` | 读取或恢复 read u64 相关逻辑。 |
| [L44](../src/protocol/BinaryProtocol.h#L44) | `ReadString` | 声明 | `bool ReadString(std::size_t size, std::string* value)` | 读取或恢复 read string 相关逻辑。 |
| [L45](../src/protocol/BinaryProtocol.h#L45) | `ReadBytes` | 声明 | `bool ReadBytes(std::size_t size, std::vector<std::uint8_t>* value)` | 读取或恢复 read bytes 相关逻辑。 |
| [L46](../src/protocol/BinaryProtocol.h#L46) | `ReadBytes` | 声明 | `bool ReadBytes(std::span<std::uint8_t> value)` | 读取或恢复 read bytes 相关逻辑。 |
| [L48](../src/protocol/BinaryProtocol.h#L48) | `ReadBytes` | 定义 | `bool ReadBytes(std::array<std::uint8_t, Size>* value)` | 读取或恢复 read bytes 相关逻辑。 |
| [L52](../src/protocol/BinaryProtocol.h#L52) | `Skip` | 声明 | `bool Skip(std::size_t size)` | 实现 skip 对应的业务或工具逻辑。 |
| [L54](../src/protocol/BinaryProtocol.h#L54) | `remaining` | 定义 | `std::size_t remaining() const { return bytes_.size() - offset_; }` | 实现 remaining 对应的业务或工具逻辑。 |
| [L55](../src/protocol/BinaryProtocol.h#L55) | `Remaining` | 定义 | `std::size_t Remaining() const { return remaining(); }` | 实现 remaining 对应的业务或工具逻辑。 |
| [L56](../src/protocol/BinaryProtocol.h#L56) | `offset` | 定义 | `std::size_t offset() const { return offset_; }` | 实现 offset 对应的业务或工具逻辑。 |

## `src/protocol/ClipboardProtocol.cpp`

[打开源码](../src/protocol/ClipboardProtocol.cpp) · **文件作用：** 实现 clipboard protocol 相关函数与文件级辅助逻辑。

### 成员与文件级变量

| 行 | 变量 | 声明 | 作用 |
|---:|---|---|---|
| [L16](../src/protocol/ClipboardProtocol.cpp#L16) | `kMagic` | `constexpr std::array<std::uint8_t, 4> kMagic = {'R', 'C', 'C', '2'};` | 定义 magic 的编译期常量或产品边界。 |
| [L17](../src/protocol/ClipboardProtocol.cpp#L17) | `kMaximumRoomIdBytes` | `constexpr std::size_t kMaximumRoomIdBytes = 128;` | 定义 maximum room id bytes 的编译期常量或产品边界。 |
| [L18](../src/protocol/ClipboardProtocol.cpp#L18) | `kMaximumDeviceIdBytes` | `constexpr std::size_t kMaximumDeviceIdBytes = 64;` | 定义 maximum device id bytes 的编译期常量或产品边界。 |
| [L19](../src/protocol/ClipboardProtocol.cpp#L19) | `kMaximumGrantIdBytes` | `constexpr std::size_t kMaximumGrantIdBytes = 192;` | 定义 maximum grant id bytes 的编译期常量或产品边界。 |
| [L20](../src/protocol/ClipboardProtocol.cpp#L20) | `kMaximumSessionIdBytes` | `constexpr std::size_t kMaximumSessionIdBytes = 64;` | 定义 maximum session id bytes 的编译期常量或产品边界。 |
| [L21](../src/protocol/ClipboardProtocol.cpp#L21) | `kMaximumItemIdBytes` | `constexpr std::size_t kMaximumItemIdBytes = 64;` | 定义 maximum item id bytes 的编译期常量或产品边界。 |
| [L22](../src/protocol/ClipboardProtocol.cpp#L22) | `kMaximumDescriptorNameBytes` | `constexpr std::size_t kMaximumDescriptorNameBytes = 1024;` | 定义 maximum descriptor name bytes 的编译期常量或产品边界。 |
| [L23](../src/protocol/ClipboardProtocol.cpp#L23) | `kMaximumErrorCodeBytes` | `constexpr std::size_t kMaximumErrorCodeBytes = 64;` | 定义 maximum error code bytes 的编译期常量或产品边界。 |
| [L24](../src/protocol/ClipboardProtocol.cpp#L24) | `kMaximumErrorMessageBytes` | `constexpr std::size_t kMaximumErrorMessageBytes = 512;` | 定义 maximum error message bytes 的编译期常量或产品边界。 |

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L26](../src/protocol/ClipboardProtocol.cpp#L26) | `SetError` | 定义 | `void SetError(std::string* error, std::string message)` | 更新或应用 set error 相关逻辑。 |
| [L33](../src/protocol/ClipboardProtocol.cpp#L33) | `ValidType` | 定义 | `bool ValidType(std::uint8_t value)` | 判断 valid type 相关逻辑。 |
| [L39](../src/protocol/ClipboardProtocol.cpp#L39) | `ValidFormat` | 定义 | `bool ValidFormat(std::uint8_t value)` | 判断 valid format 相关逻辑。 |
| [L45](../src/protocol/ClipboardProtocol.cpp#L45) | `SafeIdentifier` | 定义 | `bool SafeIdentifier(const std::string& value, std::size_t maximum, bool required)` | 实现 safe identifier 对应的业务或工具逻辑。 |
| [L58](../src/protocol/ClipboardProtocol.cpp#L58) | `Validate` | 定义 | `bool Validate(const ClipboardEnvelope& envelope, std::string* error)` | 判断 validate 相关逻辑。 |
| [L183](../src/protocol/ClipboardProtocol.cpp#L183) | `IsClipboardTransferMessage` | 定义 | `bool IsClipboardTransferMessage(ClipboardMessageType type)` | 判断 is clipboard transfer message 相关逻辑。 |
| [L194](../src/protocol/ClipboardProtocol.cpp#L194) | `EncodeClipboardMessage` | 定义 | `bool EncodeClipboardMessage(const ClipboardEnvelope& input, std::vector<std::uint8_t>* encoded, std::string* error)` | 编码 encode clipboard message 相关逻辑。 |
| [L264](../src/protocol/ClipboardProtocol.cpp#L264) | `DecodeClipboardMessage` | 定义 | `bool DecodeClipboardMessage(std::span<const std::uint8_t> encoded, ClipboardEnvelope* output, std::string* error)` | 解码或解析 decode clipboard message 相关逻辑。 |

## `src/protocol/ClipboardProtocol.h`

[打开源码](../src/protocol/ClipboardProtocol.h) · **文件作用：** 声明 clipboard protocol 相关类型、接口、配置和成员状态。

### 类型

| 行 | 类型 | 种类 | 作用 |
|---:|---|---|---|
| [L23](../src/protocol/ClipboardProtocol.h#L23) | `ClipboardMessageType` | enum class | 定义 ClipboardMessageType 的 enum class 类型和相关状态。 |
| [L44](../src/protocol/ClipboardProtocol.h#L44) | `ClipboardFormat` | enum class | 定义 ClipboardFormat 的 enum class 类型和相关状态。 |
| [L53](../src/protocol/ClipboardProtocol.h#L53) | `ClipboardCapability` | enum | 定义 ClipboardCapability 的 enum 类型和相关状态。 |
| [L62](../src/protocol/ClipboardProtocol.h#L62) | `ClipboardFormatDescriptor` | struct | 定义 ClipboardFormatDescriptor 的 struct 类型和相关状态。 |
| [L71](../src/protocol/ClipboardProtocol.h#L71) | `ClipboardMessage` | struct | 定义 ClipboardMessage 的 struct 类型和相关状态。 |
| [L87](../src/protocol/ClipboardProtocol.h#L87) | `ClipboardEnvelope` | struct | 定义 ClipboardEnvelope 的 struct 类型和相关状态。 |
| [L98](../src/protocol/ClipboardProtocol.h#L98) | `IClipboardSink` | class | 定义 IClipboardSink 的 class 类型和相关状态。 |

### 成员与文件级变量

| 行 | 变量 | 声明 | 作用 |
|---:|---|---|---|
| [L15](../src/protocol/ClipboardProtocol.h#L15) | `kClipboardProtocolVersion` | `inline constexpr std::uint8_t kClipboardProtocolVersion = 9;` | 定义 clipboard protocol version 的编译期常量或产品边界。 |
| [L16](../src/protocol/ClipboardProtocol.h#L16) | `kMaximumClipboardMessageBytes` | `inline constexpr std::size_t kMaximumClipboardMessageBytes = 64 * 1024;` | 定义 maximum clipboard message bytes 的编译期常量或产品边界。 |
| [L17](../src/protocol/ClipboardProtocol.h#L17) | `kMaximumClipboardChunkBytes` | `inline constexpr std::size_t kMaximumClipboardChunkBytes = 60 * 1024;` | 定义 maximum clipboard chunk bytes 的编译期常量或产品边界。 |
| [L18](../src/protocol/ClipboardProtocol.h#L18) | `kMaximumClipboardDescriptors` | `inline constexpr std::size_t kMaximumClipboardDescriptors = 256;` | 定义 maximum clipboard descriptors 的编译期常量或产品边界。 |
| [L19](../src/protocol/ClipboardProtocol.h#L19) | `kMaximumClipboardInlineBytes` | `inline constexpr std::uint64_t kMaximumClipboardInlineBytes = 48ull * 1024 * 1024;` | 定义 maximum clipboard inline bytes 的编译期常量或产品边界。 |
| [L63](../src/protocol/ClipboardProtocol.h#L63) | `format` | `ClipboardFormat format = ClipboardFormat::kUnicodeText;` | 保存 format 相关配置或运行状态。 |
| [L64](../src/protocol/ClipboardProtocol.h#L64) | `name` | `std::string name;` | 保存路径、地址或显示名称：name。 |
| [L65](../src/protocol/ClipboardProtocol.h#L65) | `size` | `std::uint64_t size = 0;` | 保存计数、尺寸或速率指标：size。 |
| [L66](../src/protocol/ClipboardProtocol.h#L66) | `sha256` | `std::array<std::uint8_t, 32> sha256{};` | 保存 sha256 相关配置或运行状态。 |
| [L67](../src/protocol/ClipboardProtocol.h#L67) | `hasSha256` | `bool hasSha256 = false;` | 保存 has sha256 相关配置或运行状态。 |
| [L68](../src/protocol/ClipboardProtocol.h#L68) | `topLevel` | `bool topLevel = false;` | 保存 top level 相关配置或运行状态。 |
| [L72](../src/protocol/ClipboardProtocol.h#L72) | `type` | `ClipboardMessageType type = ClipboardMessageType::kCapabilities;` | 保存 type 相关配置或运行状态。 |
| [L73](../src/protocol/ClipboardProtocol.h#L73) | `capabilities` | `std::uint32_t capabilities = 0;` | 保存 capabilities 相关配置或运行状态。 |
| [L74](../src/protocol/ClipboardProtocol.h#L74) | `enabled` | `bool enabled = false;` | 保存能力或开关状态：enabled。 |
| [L75](../src/protocol/ClipboardProtocol.h#L75) | `accepted` | `bool accepted = false;` | 保存 accepted 相关配置或运行状态。 |
| [L76](../src/protocol/ClipboardProtocol.h#L76) | `itemId` | `std::string itemId;` | 保存身份或作用域标识：item id。 |
| [L77](../src/protocol/ClipboardProtocol.h#L77) | `lamportVersion` | `std::uint64_t lamportVersion = 0;` | 保存 lamport version 相关配置或运行状态。 |
| [L78](../src/protocol/ClipboardProtocol.h#L78) | `descriptorIndex` | `std::uint32_t descriptorIndex = 0;` | 保存 descriptor index 相关配置或运行状态。 |
| [L79](../src/protocol/ClipboardProtocol.h#L79) | `offset` | `std::uint64_t offset = 0;` | 保存 offset 相关配置或运行状态。 |
| [L80](../src/protocol/ClipboardProtocol.h#L80) | `totalSize` | `std::uint64_t totalSize = 0;` | 保存计数、尺寸或速率指标：total size。 |
| [L81](../src/protocol/ClipboardProtocol.h#L81) | `descriptors` | `std::vector<ClipboardFormatDescriptor> descriptors;` | 保存 descriptors 相关配置或运行状态。 |
| [L82](../src/protocol/ClipboardProtocol.h#L82) | `data` | `std::vector<std::uint8_t> data;` | 保存 data 相关配置或运行状态。 |
| [L83](../src/protocol/ClipboardProtocol.h#L83) | `errorCode` | `std::string errorCode;` | 保存 error code 相关配置或运行状态。 |
| [L84](../src/protocol/ClipboardProtocol.h#L84) | `errorMessage` | `std::string errorMessage;` | 保存 error message 相关配置或运行状态。 |
| [L88](../src/protocol/ClipboardProtocol.h#L88) | `roomId` | `std::string roomId;` | 保存身份或作用域标识：room id。 |
| [L89](../src/protocol/ClipboardProtocol.h#L89) | `senderDeviceId` | `std::string senderDeviceId;` | 保存身份或作用域标识：sender device id。 |
| [L90](../src/protocol/ClipboardProtocol.h#L90) | `receiverDeviceId` | `std::string receiverDeviceId;` | 保存身份或作用域标识：receiver device id。 |
| [L91](../src/protocol/ClipboardProtocol.h#L91) | `controlGrantId` | `std::string controlGrantId;` | 保存身份或作用域标识：control grant id。 |
| [L92](../src/protocol/ClipboardProtocol.h#L92) | `clipboardSessionId` | `std::string clipboardSessionId;` | 保存身份或作用域标识：clipboard session id。 |
| [L93](../src/protocol/ClipboardProtocol.h#L93) | `sequence` | `std::uint64_t sequence = 0;` | 保存单调序号，用于排序或去重：sequence。 |
| [L94](../src/protocol/ClipboardProtocol.h#L94) | `timestampUs` | `std::uint64_t timestampUs = 0;` | 保存 timestamp us 相关配置或运行状态。 |
| [L95](../src/protocol/ClipboardProtocol.h#L95) | `message` | `ClipboardMessage message;` | 保存 message 相关配置或运行状态。 |

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L100](../src/protocol/ClipboardProtocol.h#L100) | `~IClipboardSink` | 声明 | `virtual ~IClipboardSink() = default` | 停止相关活动并释放 IClipboardSink 实例拥有的资源。 |
| [L101](../src/protocol/ClipboardProtocol.h#L101) | `OnClipboardMessage` | 声明 | `virtual void OnClipboardMessage(const ClipboardEnvelope& envelope) = 0` | 接收并处理 on clipboard message 相关逻辑。 |
| [L104](../src/protocol/ClipboardProtocol.h#L104) | `EncodeClipboardMessage` | 声明 | `bool EncodeClipboardMessage(const ClipboardEnvelope& input, std::vector<std::uint8_t>* encoded, std::string* error = nullptr)` | 编码 encode clipboard message 相关逻辑。 |
| [L108](../src/protocol/ClipboardProtocol.h#L108) | `DecodeClipboardMessage` | 声明 | `bool DecodeClipboardMessage(std::span<const std::uint8_t> encoded, ClipboardEnvelope* output, std::string* error = nullptr)` | 解码或解析 decode clipboard message 相关逻辑。 |
| [L112](../src/protocol/ClipboardProtocol.h#L112) | `IsClipboardTransferMessage` | 声明 | `bool IsClipboardTransferMessage(ClipboardMessageType type)` | 判断 is clipboard transfer message 相关逻辑。 |

## `src/protocol/DataChannelCatalog.cpp`

[打开源码](../src/protocol/DataChannelCatalog.cpp) · **文件作用：** 实现 data channel catalog 相关函数与文件级辅助逻辑。

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L11](../src/protocol/DataChannelCatalog.cpp#L11) | `DefaultRemoteControlDataChannels` | 定义 | `std::vector<DataChannelSpec> DefaultRemoteControlDataChannels()` | 实现 default remote control data channels 对应的业务或工具逻辑。 |
| [L58](../src/protocol/DataChannelCatalog.cpp#L58) | `DefaultCameraSessionDataChannels` | 定义 | `std::vector<DataChannelSpec> DefaultCameraSessionDataChannels()` | 实现 default camera session data channels 对应的业务或工具逻辑。 |

## `src/protocol/DataChannelCatalog.h`

[打开源码](../src/protocol/DataChannelCatalog.h) · **文件作用：** 声明 data channel catalog 相关类型、接口、配置和成员状态。

### 成员与文件级变量

| 行 | 变量 | 声明 | 作用 |
|---:|---|---|---|
| [L13](../src/protocol/DataChannelCatalog.h#L13) | `kInputFastChannel` | `inline constexpr std::string_view kInputFastChannel = "input-fast";` | 定义 input fast channel 的编译期常量或产品边界。 |
| [L16](../src/protocol/DataChannelCatalog.h#L16) | `kFileTransferChannel` | `inline constexpr std::string_view kFileTransferChannel = "file-transfer";` | 定义 file transfer channel 的编译期常量或产品边界。 |
| [L21](../src/protocol/DataChannelCatalog.h#L21) | `kTelemetryChannel` | `inline constexpr std::string_view kTelemetryChannel = "telemetry";` | 定义 telemetry channel 的编译期常量或产品边界。 |

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L23](../src/protocol/DataChannelCatalog.h#L23) | `DefaultRemoteControlDataChannels` | 声明 | `std::vector<DataChannelSpec> DefaultRemoteControlDataChannels()` | 实现 default remote control data channels 对应的业务或工具逻辑。 |
| [L24](../src/protocol/DataChannelCatalog.h#L24) | `DefaultCameraSessionDataChannels` | 声明 | `std::vector<DataChannelSpec> DefaultCameraSessionDataChannels()` | 实现 default camera session data channels 对应的业务或工具逻辑。 |

## `src/protocol/FileTransferProtocol.cpp`

[打开源码](../src/protocol/FileTransferProtocol.cpp) · **文件作用：** 实现 file transfer protocol 相关函数与文件级辅助逻辑。

### 成员与文件级变量

| 行 | 变量 | 声明 | 作用 |
|---:|---|---|---|
| [L16](../src/protocol/FileTransferProtocol.cpp#L16) | `kMagic` | `constexpr std::array<std::uint8_t, 4> kMagic = {'R', 'C', 'F', '1'};` | 定义 magic 的编译期常量或产品边界。 |
| [L17](../src/protocol/FileTransferProtocol.cpp#L17) | `kFixedHeaderBytes` | `constexpr std::size_t kFixedHeaderBytes = 36;` | 定义 fixed header bytes 的编译期常量或产品边界。 |
| [L18](../src/protocol/FileTransferProtocol.cpp#L18) | `kMaximumRoomIdBytes` | `constexpr std::size_t kMaximumRoomIdBytes = 128;` | 定义 maximum room id bytes 的编译期常量或产品边界。 |
| [L19](../src/protocol/FileTransferProtocol.cpp#L19) | `kMaximumDeviceIdBytes` | `constexpr std::size_t kMaximumDeviceIdBytes = 64;` | 定义 maximum device id bytes 的编译期常量或产品边界。 |
| [L20](../src/protocol/FileTransferProtocol.cpp#L20) | `kMaximumTransferIdBytes` | `constexpr std::size_t kMaximumTransferIdBytes = 64;` | 定义 maximum transfer id bytes 的编译期常量或产品边界。 |
| [L21](../src/protocol/FileTransferProtocol.cpp#L21) | `kMaximumFileNameBytes` | `constexpr std::size_t kMaximumFileNameBytes = 255;` | 定义 maximum file name bytes 的编译期常量或产品边界。 |
| [L22](../src/protocol/FileTransferProtocol.cpp#L22) | `kMaximumErrorCodeBytes` | `constexpr std::size_t kMaximumErrorCodeBytes = 64;` | 定义 maximum error code bytes 的编译期常量或产品边界。 |
| [L23](../src/protocol/FileTransferProtocol.cpp#L23) | `kMaximumErrorMessageBytes` | `constexpr std::size_t kMaximumErrorMessageBytes = 512;` | 定义 maximum error message bytes 的编译期常量或产品边界。 |

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L25](../src/protocol/FileTransferProtocol.cpp#L25) | `SetError` | 定义 | `void SetError(std::string* error, std::string message)` | 更新或应用 set error 相关逻辑。 |
| [L32](../src/protocol/FileTransferProtocol.cpp#L32) | `AppendU16` | 定义 | `void AppendU16(std::vector<std::uint8_t>* output, std::uint16_t value)` | 实现 append u16 对应的业务或工具逻辑。 |
| [L37](../src/protocol/FileTransferProtocol.cpp#L37) | `AppendU32` | 定义 | `void AppendU32(std::vector<std::uint8_t>* output, std::uint32_t value)` | 实现 append u32 对应的业务或工具逻辑。 |
| [L42](../src/protocol/FileTransferProtocol.cpp#L42) | `AppendU64` | 定义 | `void AppendU64(std::vector<std::uint8_t>* output, std::uint64_t value)` | 实现 append u64 对应的业务或工具逻辑。 |
| [L47](../src/protocol/FileTransferProtocol.cpp#L47) | `AppendString` | 定义 | `void AppendString(std::vector<std::uint8_t>* output, const std::string& value)` | 实现 append string 对应的业务或工具逻辑。 |
| [L55](../src/protocol/FileTransferProtocol.cpp#L55) | `ValidType` | 定义 | `bool ValidType(std::uint8_t type)` | 判断 valid type 相关逻辑。 |
| [L63](../src/protocol/FileTransferProtocol.cpp#L63) | `ValidTransferId` | 定义 | `bool ValidTransferId(const std::string& value)` | 判断 valid transfer id 相关逻辑。 |
| [L74](../src/protocol/FileTransferProtocol.cpp#L74) | `ValidFileName` | 定义 | `bool ValidFileName(const std::string& value)` | 判断 valid file name 相关逻辑。 |
| [L87](../src/protocol/FileTransferProtocol.cpp#L87) | `ValidErrorCode` | 定义 | `bool ValidErrorCode(const std::string& value, bool required)` | 判断 valid error code 相关逻辑。 |
| [L99](../src/protocol/FileTransferProtocol.cpp#L99) | `ValidatePayload` | 定义 | `bool ValidatePayload(const FileTransferMessage& message, std::string* error)` | 判断 validate payload 相关逻辑。 |
| [L163](../src/protocol/FileTransferProtocol.cpp#L163) | `ValidateEnvelope` | 定义 | `bool ValidateEnvelope(const FileTransferEnvelope& envelope, std::string* error)` | 判断 validate envelope 相关逻辑。 |
| [L183](../src/protocol/FileTransferProtocol.cpp#L183) | `AppendReason` | 定义 | `void AppendReason(const FileTransferMessage& message, std::vector<std::uint8_t>* payload)` | 实现 append reason 对应的业务或工具逻辑。 |
| [L194](../src/protocol/FileTransferProtocol.cpp#L194) | `EncodePayload` | 定义 | `bool EncodePayload(const FileTransferMessage& message, std::vector<std::uint8_t>* payload)` | 编码 encode payload 相关逻辑。 |
| [L248](../src/protocol/FileTransferProtocol.cpp#L248) | `ReadReason` | 定义 | `bool ReadReason(Reader* reader, FileTransferMessage* message)` | 读取或恢复 read reason 相关逻辑。 |
| [L260](../src/protocol/FileTransferProtocol.cpp#L260) | `DecodePayload` | 定义 | `bool DecodePayload(std::span<const std::uint8_t> bytes, FileTransferMessage* message)` | 解码或解析 decode payload 相关逻辑。 |
| [L339](../src/protocol/FileTransferProtocol.cpp#L339) | `EncodeFileTransferMessage` | 定义 | `bool EncodeFileTransferMessage(const FileTransferEnvelope& input, std::vector<std::uint8_t>* encoded, std::string* error)` | 编码 encode file transfer message 相关逻辑。 |
| [L389](../src/protocol/FileTransferProtocol.cpp#L389) | `DecodeFileTransferMessage` | 定义 | `bool DecodeFileTransferMessage(std::span<const std::uint8_t> encoded, FileTransferEnvelope* output, std::string* error)` | 解码或解析 decode file transfer message 相关逻辑。 |

## `src/protocol/FileTransferProtocol.h`

[打开源码](../src/protocol/FileTransferProtocol.h) · **文件作用：** 声明 file transfer protocol 相关类型、接口、配置和成员状态。

### 类型

| 行 | 类型 | 种类 | 作用 |
|---:|---|---|---|
| [L20](../src/protocol/FileTransferProtocol.h#L20) | `FileTransferMessageType` | enum class | 定义 FileTransferMessageType 的 enum class 类型和相关状态。 |
| [L31](../src/protocol/FileTransferProtocol.h#L31) | `FileTransferMessage` | struct | 定义 FileTransferMessage 的 struct 类型和相关状态。 |
| [L46](../src/protocol/FileTransferProtocol.h#L46) | `FileTransferEnvelope` | struct | 定义 FileTransferEnvelope 的 struct 类型和相关状态。 |
| [L55](../src/protocol/FileTransferProtocol.h#L55) | `IFileTransferSink` | class | 定义 IFileTransferSink 的 class 类型和相关状态。 |

### 成员与文件级变量

| 行 | 变量 | 声明 | 作用 |
|---:|---|---|---|
| [L14](../src/protocol/FileTransferProtocol.h#L14) | `kFileTransferProtocolVersion` | `inline constexpr std::uint8_t kFileTransferProtocolVersion = 4;` | 定义 file transfer protocol version 的编译期常量或产品边界。 |
| [L15](../src/protocol/FileTransferProtocol.h#L15) | `kMaximumFileTransferMessageBytes` | `inline constexpr std::size_t kMaximumFileTransferMessageBytes = 64 * 1024;` | 定义 maximum file transfer message bytes 的编译期常量或产品边界。 |
| [L16](../src/protocol/FileTransferProtocol.h#L16) | `kMaximumFileChunkBytes` | `inline constexpr std::size_t kMaximumFileChunkBytes = 60 * 1024;` | 定义 maximum file chunk bytes 的编译期常量或产品边界。 |
| [L18](../src/protocol/FileTransferProtocol.h#L18) | `uint64_t` | `std::uint64_t{1} << 50;` | 保存 uint64 t 相关配置或运行状态。 |
| [L32](../src/protocol/FileTransferProtocol.h#L32) | `type` | `FileTransferMessageType type = FileTransferMessageType::kOffer;` | 保存 type 相关配置或运行状态。 |
| [L33](../src/protocol/FileTransferProtocol.h#L33) | `transferId` | `std::string transferId;` | 保存身份或作用域标识：transfer id。 |
| [L34](../src/protocol/FileTransferProtocol.h#L34) | `fileName` | `std::string fileName;` | 保存路径、地址或显示名称：file name。 |
| [L35](../src/protocol/FileTransferProtocol.h#L35) | `fileSize` | `std::uint64_t fileSize = 0;` | 保存计数、尺寸或速率指标：file size。 |
| [L36](../src/protocol/FileTransferProtocol.h#L36) | `sha256` | `std::array<std::uint8_t, 32> sha256{};` | 保存 sha256 相关配置或运行状态。 |
| [L37](../src/protocol/FileTransferProtocol.h#L37) | `hasSha256` | `bool hasSha256 = false;` | 保存 has sha256 相关配置或运行状态。 |
| [L38](../src/protocol/FileTransferProtocol.h#L38) | `accepted` | `bool accepted = false;` | 保存 accepted 相关配置或运行状态。 |
| [L39](../src/protocol/FileTransferProtocol.h#L39) | `resumeOffset` | `std::uint64_t resumeOffset = 0;` | 保存 resume offset 相关配置或运行状态。 |
| [L40](../src/protocol/FileTransferProtocol.h#L40) | `chunkOffset` | `std::uint64_t chunkOffset = 0;` | 保存 chunk offset 相关配置或运行状态。 |
| [L41](../src/protocol/FileTransferProtocol.h#L41) | `chunk` | `std::vector<std::uint8_t> chunk;` | 保存 chunk 相关配置或运行状态。 |
| [L42](../src/protocol/FileTransferProtocol.h#L42) | `errorCode` | `std::string errorCode;` | 保存 error code 相关配置或运行状态。 |
| [L43](../src/protocol/FileTransferProtocol.h#L43) | `errorMessage` | `std::string errorMessage;` | 保存 error message 相关配置或运行状态。 |
| [L47](../src/protocol/FileTransferProtocol.h#L47) | `roomId` | `std::string roomId;` | 保存身份或作用域标识：room id。 |
| [L48](../src/protocol/FileTransferProtocol.h#L48) | `senderDeviceId` | `std::string senderDeviceId;` | 保存身份或作用域标识：sender device id。 |
| [L49](../src/protocol/FileTransferProtocol.h#L49) | `receiverDeviceId` | `std::string receiverDeviceId;` | 保存身份或作用域标识：receiver device id。 |
| [L50](../src/protocol/FileTransferProtocol.h#L50) | `sequence` | `std::uint64_t sequence = 0;` | 保存单调序号，用于排序或去重：sequence。 |
| [L51](../src/protocol/FileTransferProtocol.h#L51) | `timestampUs` | `std::uint64_t timestampUs = 0;` | 保存 timestamp us 相关配置或运行状态。 |
| [L52](../src/protocol/FileTransferProtocol.h#L52) | `message` | `FileTransferMessage message;` | 保存 message 相关配置或运行状态。 |

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L57](../src/protocol/FileTransferProtocol.h#L57) | `~IFileTransferSink` | 声明 | `virtual ~IFileTransferSink() = default` | 停止相关活动并释放 IFileTransferSink 实例拥有的资源。 |
| [L61](../src/protocol/FileTransferProtocol.h#L61) | `OnFileTransferMessage` | 声明 | `virtual void OnFileTransferMessage( const FileTransferEnvelope& envelope) = 0` | The engine invokes this only after validating room membership, the Pair identity, sender/receiver IDs and the per-Pair monotonic sequence. |
| [L65](../src/protocol/FileTransferProtocol.h#L65) | `EncodeFileTransferMessage` | 声明 | `bool EncodeFileTransferMessage(const FileTransferEnvelope& input, std::vector<std::uint8_t>* encoded, std::string* error = nullptr)` | 编码 encode file transfer message 相关逻辑。 |
| [L69](../src/protocol/FileTransferProtocol.h#L69) | `DecodeFileTransferMessage` | 声明 | `bool DecodeFileTransferMessage(std::span<const std::uint8_t> encoded, FileTransferEnvelope* output, std::string* error = nullptr)` | 解码或解析 decode file transfer message 相关逻辑。 |

## `src/protocol/RemoteCursorProtocol.cpp`

[打开源码](../src/protocol/RemoteCursorProtocol.cpp) · **文件作用：** 实现 remote cursor protocol 相关函数与文件级辅助逻辑。

### 成员与文件级变量

| 行 | 变量 | 声明 | 作用 |
|---:|---|---|---|
| [L14](../src/protocol/RemoteCursorProtocol.cpp#L14) | `kMagic` | `constexpr std::array<std::uint8_t, 4> kMagic = {'R', 'C', 'C', '1'};` | 定义 magic 的编译期常量或产品边界。 |
| [L15](../src/protocol/RemoteCursorProtocol.cpp#L15) | `kVersion` | `constexpr std::uint8_t kVersion = 1;` | 定义 version 的编译期常量或产品边界。 |
| [L16](../src/protocol/RemoteCursorProtocol.cpp#L16) | `kMaximumIdentityBytes` | `constexpr std::size_t kMaximumIdentityBytes = 512;` | 定义 maximum identity bytes 的编译期常量或产品边界。 |

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L18](../src/protocol/RemoteCursorProtocol.cpp#L18) | `Fail` | 定义 | `bool Fail(std::string* error, const char* message)` | 实现 fail 对应的业务或工具逻辑。 |
| [L26](../src/protocol/RemoteCursorProtocol.cpp#L26) | `IsKnownType` | 定义 | `bool IsKnownType(RemoteCursorMessageType type)` | 判断 is known type 相关逻辑。 |
| [L33](../src/protocol/RemoteCursorProtocol.cpp#L33) | `ValidateIdentity` | 定义 | `bool ValidateIdentity(const RemoteCursorEnvelope& envelope, std::string* error)` | 判断 validate identity 相关逻辑。 |
| [L50](../src/protocol/RemoteCursorProtocol.cpp#L50) | `ValidateShape` | 定义 | `bool ValidateShape(const RemoteCursorShape& shape, std::string* error)` | 判断 validate shape 相关逻辑。 |
| [L69](../src/protocol/RemoteCursorProtocol.cpp#L69) | `EncodeRemoteCursorMessage` | 定义 | `bool EncodeRemoteCursorMessage( const RemoteCursorEnvelope& envelope, std::vector<std::uint8_t>* output, std::string* error)` | 编码 encode remote cursor message 相关逻辑。 |
| [L135](../src/protocol/RemoteCursorProtocol.cpp#L135) | `DecodeRemoteCursorMessage` | 定义 | `bool DecodeRemoteCursorMessage( std::span<const std::uint8_t> payload, RemoteCursorEnvelope* envelope, std::string* error)` | 解码或解析 decode remote cursor message 相关逻辑。 |

## `src/protocol/RemoteCursorProtocol.h`

[打开源码](../src/protocol/RemoteCursorProtocol.h) · **文件作用：** 声明 remote cursor protocol 相关类型、接口、配置和成员状态。

### 类型

| 行 | 类型 | 种类 | 作用 |
|---:|---|---|---|
| [L18](../src/protocol/RemoteCursorProtocol.h#L18) | `RemoteCursorMessageType` | enum class | 定义 RemoteCursorMessageType 的 enum class 类型和相关状态。 |
| [L24](../src/protocol/RemoteCursorProtocol.h#L24) | `RemoteCursorPosition` | struct | 定义 RemoteCursorPosition 的 struct 类型和相关状态。 |
| [L37](../src/protocol/RemoteCursorProtocol.h#L37) | `RemoteCursorShape` | struct | 定义 RemoteCursorShape 的 struct 类型和相关状态。 |
| [L50](../src/protocol/RemoteCursorProtocol.h#L50) | `RemoteCursorEnvelope` | struct | 定义 RemoteCursorEnvelope 的 struct 类型和相关状态。 |

### 成员与文件级变量

| 行 | 变量 | 声明 | 作用 |
|---:|---|---|---|
| [L13](../src/protocol/RemoteCursorProtocol.h#L13) | `kMaximumRemoteCursorDimension` | `inline constexpr std::uint32_t kMaximumRemoteCursorDimension = 256;` | 定义 maximum remote cursor dimension 的编译期常量或产品边界。 |
| [L25](../src/protocol/RemoteCursorProtocol.h#L25) | `displayId` | `std::uint32_t displayId = 0;` | 保存身份或作用域标识：display id。 |
| [L26](../src/protocol/RemoteCursorProtocol.h#L26) | `displayLayoutVersion` | `std::uint64_t displayLayoutVersion = 0;` | 保存 display layout version 相关配置或运行状态。 |
| [L27](../src/protocol/RemoteCursorProtocol.h#L27) | `normalizedX` | `std::uint16_t normalizedX = 0;` | 保存 normalized x 相关配置或运行状态。 |
| [L28](../src/protocol/RemoteCursorProtocol.h#L28) | `normalizedY` | `std::uint16_t normalizedY = 0;` | 保存 normalized y 相关配置或运行状态。 |
| [L29](../src/protocol/RemoteCursorProtocol.h#L29) | `visible` | `bool visible = false;` | 保存 visible 相关配置或运行状态。 |
| [L30](../src/protocol/RemoteCursorProtocol.h#L30) | `shapeId` | `std::uint64_t shapeId = 0;` | 保存身份或作用域标识：shape id。 |
| [L34](../src/protocol/RemoteCursorProtocol.h#L34) | `lastAppliedInputSequence` | `std::uint64_t lastAppliedInputSequence = 0;` | Highest controller input sequence known to have reached the controlled machine. It lets viewers and diagnostics correlate the authoritative cursor echo with recently transmitted... |
| [L38](../src/protocol/RemoteCursorProtocol.h#L38) | `shapeId` | `std::uint64_t shapeId = 0;` | 保存身份或作用域标识：shape id。 |
| [L39](../src/protocol/RemoteCursorProtocol.h#L39) | `width` | `std::uint16_t width = 0;` | 保存计数、尺寸或速率指标：width。 |
| [L40](../src/protocol/RemoteCursorProtocol.h#L40) | `height` | `std::uint16_t height = 0;` | 保存计数、尺寸或速率指标：height。 |
| [L41](../src/protocol/RemoteCursorProtocol.h#L41) | `hotspotX` | `std::uint16_t hotspotX = 0;` | 保存 hotspot x 相关配置或运行状态。 |
| [L42](../src/protocol/RemoteCursorProtocol.h#L42) | `hotspotY` | `std::uint16_t hotspotY = 0;` | 保存 hotspot y 相关配置或运行状态。 |
| [L44](../src/protocol/RemoteCursorProtocol.h#L44) | `premultipliedBgra` | `std::vector<std::uint8_t> premultipliedBgra;` | Premultiplied BGRA pixels in top-to-bottom row order. |
| [L47](../src/protocol/RemoteCursorProtocol.h#L47) | `invertMask` | `std::vector<std::uint8_t> invertMask;` | One byte per pixel. Zero uses premultipliedBgra; non-zero requests the legacy Windows XOR/invert cursor operation. Empty means no invert pixels. |
| [L51](../src/protocol/RemoteCursorProtocol.h#L51) | `type` | `RemoteCursorMessageType type = RemoteCursorMessageType::kPosition;` | 保存 type 相关配置或运行状态。 |
| [L53](../src/protocol/RemoteCursorProtocol.h#L53) | `contextId` | `std::string contextId;` | Direct sessions use the signaling session id; rooms use the room id. |
| [L54](../src/protocol/RemoteCursorProtocol.h#L54) | `senderDeviceId` | `std::string senderDeviceId;` | 保存身份或作用域标识：sender device id。 |
| [L55](../src/protocol/RemoteCursorProtocol.h#L55) | `sequence` | `std::uint64_t sequence = 0;` | 保存单调序号，用于排序或去重：sequence。 |
| [L56](../src/protocol/RemoteCursorProtocol.h#L56) | `timestampUs` | `std::uint64_t timestampUs = 0;` | 保存 timestamp us 相关配置或运行状态。 |
| [L57](../src/protocol/RemoteCursorProtocol.h#L57) | `screenShareGeneration` | `std::uint64_t screenShareGeneration = 0;` | 标记当前世代，用于拒绝过期异步结果：screen share generation。 |
| [L58](../src/protocol/RemoteCursorProtocol.h#L58) | `position` | `RemoteCursorPosition position;` | 保存 position 相关配置或运行状态。 |
| [L59](../src/protocol/RemoteCursorProtocol.h#L59) | `shape` | `RemoteCursorShape shape;` | 保存 shape 相关配置或运行状态。 |

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L62](../src/protocol/RemoteCursorProtocol.h#L62) | `EncodeRemoteCursorMessage` | 声明 | `bool EncodeRemoteCursorMessage( const RemoteCursorEnvelope& envelope, std::vector<std::uint8_t>* output, std::string* error = nullptr)` | 编码 encode remote cursor message 相关逻辑。 |
| [L67](../src/protocol/RemoteCursorProtocol.h#L67) | `DecodeRemoteCursorMessage` | 声明 | `bool DecodeRemoteCursorMessage( std::span<const std::uint8_t> payload, RemoteCursorEnvelope* envelope, std::string* error = nullptr)` | 解码或解析 decode remote cursor message 相关逻辑。 |

## `src/protocol/RemoteInputProtocol.cpp`

[打开源码](../src/protocol/RemoteInputProtocol.cpp) · **文件作用：** 实现 remote input protocol 相关函数与文件级辅助逻辑。

### 成员与文件级变量

| 行 | 变量 | 声明 | 作用 |
|---:|---|---|---|
| [L16](../src/protocol/RemoteInputProtocol.cpp#L16) | `kMagic` | `constexpr std::array<std::uint8_t, 4> kMagic = {'R', 'C', 'I', '1'};` | 定义 magic 的编译期常量或产品边界。 |
| [L17](../src/protocol/RemoteInputProtocol.cpp#L17) | `kFixedHeaderBytes` | `constexpr std::size_t kFixedHeaderBytes = 44;` | 定义 fixed header bytes 的编译期常量或产品边界。 |
| [L18](../src/protocol/RemoteInputProtocol.cpp#L18) | `kMaximumRoomIdBytes` | `constexpr std::size_t kMaximumRoomIdBytes = 128;` | 定义 maximum room id bytes 的编译期常量或产品边界。 |
| [L19](../src/protocol/RemoteInputProtocol.cpp#L19) | `kMaximumDeviceIdBytes` | `constexpr std::size_t kMaximumDeviceIdBytes = 64;` | 定义 maximum device id bytes 的编译期常量或产品边界。 |
| [L20](../src/protocol/RemoteInputProtocol.cpp#L20) | `kMaximumGrantIdBytes` | `constexpr std::size_t kMaximumGrantIdBytes = 128;` | 定义 maximum grant id bytes 的编译期常量或产品边界。 |

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L22](../src/protocol/RemoteInputProtocol.cpp#L22) | `SetError` | 定义 | `void SetError(std::string* error, std::string message)` | 更新或应用 set error 相关逻辑。 |
| [L29](../src/protocol/RemoteInputProtocol.cpp#L29) | `ValidType` | 定义 | `bool ValidType(std::uint8_t type)` | 判断 valid type 相关逻辑。 |
| [L37](../src/protocol/RemoteInputProtocol.cpp#L37) | `ValidMouseButton` | 定义 | `bool ValidMouseButton(RemoteMouseButton button)` | 判断 valid mouse button 相关逻辑。 |
| [L43](../src/protocol/RemoteInputProtocol.cpp#L43) | `PayloadSize` | 定义 | `std::uint16_t PayloadSize(RemoteInputMessageType type)` | 实现 payload size 对应的业务或工具逻辑。 |
| [L60](../src/protocol/RemoteInputProtocol.cpp#L60) | `ValidateEnvelope` | 定义 | `bool ValidateEnvelope(const RemoteInputEnvelope& input, std::string* error)` | 判断 validate envelope 相关逻辑。 |
| [L116](../src/protocol/RemoteInputProtocol.cpp#L116) | `EncodePayload` | 定义 | `void EncodePayload(const RemoteInputEvent& event, BinaryProtocolWriter* writer)` | 编码 encode payload 相关逻辑。 |
| [L154](../src/protocol/RemoteInputProtocol.cpp#L154) | `DecodePayload` | 定义 | `bool DecodePayload(BinaryProtocolReader* reader, std::uint16_t payloadSize, RemoteInputEvent* event, std::string* error)` | 解码或解析 decode payload 相关逻辑。 |
| [L226](../src/protocol/RemoteInputProtocol.cpp#L226) | `UsesFastInputChannel` | 定义 | `bool UsesFastInputChannel(RemoteInputMessageType type)` | 实现 uses fast input channel 对应的业务或工具逻辑。 |
| [L233](../src/protocol/RemoteInputProtocol.cpp#L233) | `IsPointerInput` | 定义 | `bool IsPointerInput(RemoteInputMessageType type)` | 判断 is pointer input 相关逻辑。 |
| [L240](../src/protocol/RemoteInputProtocol.cpp#L240) | `EncodeRemoteInput` | 定义 | `bool EncodeRemoteInput(const RemoteInputEnvelope& input, std::vector<std::uint8_t>* encoded, std::string* error)` | 编码 encode remote input 相关逻辑。 |
| [L287](../src/protocol/RemoteInputProtocol.cpp#L287) | `DecodeRemoteInput` | 定义 | `bool DecodeRemoteInput(std::span<const std::uint8_t> encoded, RemoteInputEnvelope* input, std::string* error)` | 解码或解析 decode remote input 相关逻辑。 |

## `src/protocol/RemoteInputProtocol.h`

[打开源码](../src/protocol/RemoteInputProtocol.h) · **文件作用：** 声明 remote input protocol 相关类型、接口、配置和成员状态。

### 类型

| 行 | 类型 | 种类 | 作用 |
|---:|---|---|---|
| [L17](../src/protocol/RemoteInputProtocol.h#L17) | `RemoteInputMessageType` | enum class | 定义 RemoteInputMessageType 的 enum class 类型和相关状态。 |
| [L25](../src/protocol/RemoteInputProtocol.h#L25) | `RemoteMouseButton` | enum class | 定义 RemoteMouseButton 的 enum class 类型和相关状态。 |
| [L37](../src/protocol/RemoteInputProtocol.h#L37) | `RemoteInputEvent` | struct | Coordinates are normalized to [0, 65535] so the transport is independent from the controller window size and the controlled display resolution. The UI renderer performs letterbo... |
| [L60](../src/protocol/RemoteInputProtocol.h#L60) | `RemoteInputEnvelope` | struct | 定义 RemoteInputEnvelope 的 struct 类型和相关状态。 |
| [L69](../src/protocol/RemoteInputProtocol.h#L69) | `IRemoteInputSink` | class | 定义 IRemoteInputSink 的 class 类型和相关状态。 |

### 成员与文件级变量

| 行 | 变量 | 声明 | 作用 |
|---:|---|---|---|
| [L13](../src/protocol/RemoteInputProtocol.h#L13) | `kRemoteInputProtocolVersion` | `inline constexpr std::uint8_t kRemoteInputProtocolVersion = 3;` | 定义 remote input protocol version 的编译期常量或产品边界。 |
| [L14](../src/protocol/RemoteInputProtocol.h#L14) | `kMaximumRemoteInputMessageBytes` | `inline constexpr std::size_t kMaximumRemoteInputMessageBytes = 512;` | 定义 maximum remote input message bytes 的编译期常量或产品边界。 |
| [L15](../src/protocol/RemoteInputProtocol.h#L15) | `kRemoteMouseButtonsMask` | `inline constexpr std::uint8_t kRemoteMouseButtonsMask = 0x1f;` | 定义 remote mouse buttons mask 的编译期常量或产品边界。 |
| [L38](../src/protocol/RemoteInputProtocol.h#L38) | `type` | `RemoteInputMessageType type = RemoteInputMessageType::kMouseMove;` | 保存 type 相关配置或运行状态。 |
| [L39](../src/protocol/RemoteInputProtocol.h#L39) | `displayId` | `std::uint32_t displayId = 0;` | 保存身份或作用域标识：display id。 |
| [L40](../src/protocol/RemoteInputProtocol.h#L40) | `displayLayoutVersion` | `std::uint64_t displayLayoutVersion = 0;` | 保存 display layout version 相关配置或运行状态。 |
| [L41](../src/protocol/RemoteInputProtocol.h#L41) | `normalizedX` | `std::uint16_t normalizedX = 0;` | 保存 normalized x 相关配置或运行状态。 |
| [L42](../src/protocol/RemoteInputProtocol.h#L42) | `normalizedY` | `std::uint16_t normalizedY = 0;` | 保存 normalized y 相关配置或运行状态。 |
| [L43](../src/protocol/RemoteInputProtocol.h#L43) | `mouseButton` | `RemoteMouseButton mouseButton = RemoteMouseButton::kNone;` | 保存 mouse button 相关配置或运行状态。 |
| [L44](../src/protocol/RemoteInputProtocol.h#L44) | `pressed` | `bool pressed = false;` | 保存 pressed 相关配置或运行状态。 |
| [L48](../src/protocol/RemoteInputProtocol.h#L48) | `pressedMouseButtons` | `std::uint8_t pressedMouseButtons = 0;` | Bit 0..4 represent left, right, middle, X1 and X2. Every pointer message carries the complete post-event state so an unreliable move can repair a delayed or lost button transition. |
| [L49](../src/protocol/RemoteInputProtocol.h#L49) | `wheelDeltaX` | `std::int16_t wheelDeltaX = 0;` | 保存 wheel delta x 相关配置或运行状态。 |
| [L50](../src/protocol/RemoteInputProtocol.h#L50) | `wheelDeltaY` | `std::int16_t wheelDeltaY = 0;` | 保存 wheel delta y 相关配置或运行状态。 |
| [L51](../src/protocol/RemoteInputProtocol.h#L51) | `virtualKey` | `std::uint16_t virtualKey = 0;` | 保存 virtual key 相关配置或运行状态。 |
| [L52](../src/protocol/RemoteInputProtocol.h#L52) | `scanCode` | `std::uint16_t scanCode = 0;` | 保存 scan code 相关配置或运行状态。 |
| [L53](../src/protocol/RemoteInputProtocol.h#L53) | `extendedKey` | `bool extendedKey = false;` | 保存 extended key 相关配置或运行状态。 |
| [L54](../src/protocol/RemoteInputProtocol.h#L54) | `repeat` | `bool repeat = false;` | 保存 repeat 相关配置或运行状态。 |
| [L57](../src/protocol/RemoteInputProtocol.h#L57) | `deliverySequence` | `std::uint64_t deliverySequence = 0;` | Receiver-local metadata copied from the envelope after decoding. It is not serialized inside the event payload. |
| [L61](../src/protocol/RemoteInputProtocol.h#L61) | `roomId` | `std::string roomId;` | 保存身份或作用域标识：room id。 |
| [L62](../src/protocol/RemoteInputProtocol.h#L62) | `senderDeviceId` | `std::string senderDeviceId;` | 保存身份或作用域标识：sender device id。 |
| [L63](../src/protocol/RemoteInputProtocol.h#L63) | `controlGrantId` | `std::string controlGrantId;` | 保存身份或作用域标识：control grant id。 |
| [L64](../src/protocol/RemoteInputProtocol.h#L64) | `sequence` | `std::uint64_t sequence = 0;` | 保存单调序号，用于排序或去重：sequence。 |
| [L65](../src/protocol/RemoteInputProtocol.h#L65) | `timestampUs` | `std::uint64_t timestampUs = 0;` | 保存 timestamp us 相关配置或运行状态。 |
| [L66](../src/protocol/RemoteInputProtocol.h#L66) | `event` | `RemoteInputEvent event;` | 保存 event 相关配置或运行状态。 |

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L71](../src/protocol/RemoteInputProtocol.h#L71) | `~IRemoteInputSink` | 声明 | `virtual ~IRemoteInputSink() = default` | 停止相关活动并释放 IRemoteInputSink 实例拥有的资源。 |
| [L75](../src/protocol/RemoteInputProtocol.h#L75) | `OnRemoteInput` | 声明 | `virtual void OnRemoteInput(const RemoteInputEvent& event) = 0` | Called only after the engine validates the room, pair identity, control grant, channel and monotonically increasing sequence. |
| [L76](../src/protocol/RemoteInputProtocol.h#L76) | `ReleaseAllRemoteInputs` | 声明 | `virtual void ReleaseAllRemoteInputs() = 0` | 释放或取消 release all remote inputs 相关逻辑。 |
| [L79](../src/protocol/RemoteInputProtocol.h#L79) | `UsesFastInputChannel` | 声明 | `bool UsesFastInputChannel(RemoteInputMessageType type)` | 实现 uses fast input channel 对应的业务或工具逻辑。 |
| [L80](../src/protocol/RemoteInputProtocol.h#L80) | `IsPointerInput` | 声明 | `bool IsPointerInput(RemoteInputMessageType type)` | 判断 is pointer input 相关逻辑。 |
| [L82](../src/protocol/RemoteInputProtocol.h#L82) | `EncodeRemoteInput` | 声明 | `bool EncodeRemoteInput(const RemoteInputEnvelope& input, std::vector<std::uint8_t>* encoded, std::string* error = nullptr)` | 编码 encode remote input 相关逻辑。 |
| [L86](../src/protocol/RemoteInputProtocol.h#L86) | `DecodeRemoteInput` | 声明 | `bool DecodeRemoteInput(std::span<const std::uint8_t> encoded, RemoteInputEnvelope* input, std::string* error = nullptr)` | 解码或解析 decode remote input 相关逻辑。 |

## `src/protocol/RoomMemberControlProtocol.cpp`

[打开源码](../src/protocol/RoomMemberControlProtocol.cpp) · **文件作用：** 实现 room member control protocol 相关函数与文件级辅助逻辑。

### 成员与文件级变量

| 行 | 变量 | 声明 | 作用 |
|---:|---|---|---|
| [L15](../src/protocol/RoomMemberControlProtocol.cpp#L15) | `kMagic` | `constexpr std::array<std::uint8_t, 4> kMagic = {'R', 'C', 'M', '1'};` | 定义 magic 的编译期常量或产品边界。 |
| [L16](../src/protocol/RoomMemberControlProtocol.cpp#L16) | `kHeaderBytes` | `constexpr std::size_t kHeaderBytes = 32;` | 定义 header bytes 的编译期常量或产品边界。 |
| [L17](../src/protocol/RoomMemberControlProtocol.cpp#L17) | `kMaximumMessageBytes` | `constexpr std::size_t kMaximumMessageBytes = 512;` | 定义 maximum message bytes 的编译期常量或产品边界。 |
| [L18](../src/protocol/RoomMemberControlProtocol.cpp#L18) | `kMaximumRoomIdBytes` | `constexpr std::size_t kMaximumRoomIdBytes = 128;` | 定义 maximum room id bytes 的编译期常量或产品边界。 |
| [L19](../src/protocol/RoomMemberControlProtocol.cpp#L19) | `kMaximumDeviceIdBytes` | `constexpr std::size_t kMaximumDeviceIdBytes = 64;` | 定义 maximum device id bytes 的编译期常量或产品边界。 |
| [L20](../src/protocol/RoomMemberControlProtocol.cpp#L20) | `kMaximumErrorBytes` | `constexpr std::size_t kMaximumErrorBytes = 192;` | 定义 maximum error bytes 的编译期常量或产品边界。 |

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L22](../src/protocol/RoomMemberControlProtocol.cpp#L22) | `SetError` | 定义 | `void SetError(std::string* error, std::string message)` | 更新或应用 set error 相关逻辑。 |
| [L29](../src/protocol/RoomMemberControlProtocol.cpp#L29) | `AppendU16` | 定义 | `void AppendU16(std::vector<std::uint8_t>* output, std::uint16_t value)` | 实现 append u16 对应的业务或工具逻辑。 |
| [L34](../src/protocol/RoomMemberControlProtocol.cpp#L34) | `AppendU64` | 定义 | `void AppendU64(std::vector<std::uint8_t>* output, std::uint64_t value)` | 实现 append u64 对应的业务或工具逻辑。 |
| [L39](../src/protocol/RoomMemberControlProtocol.cpp#L39) | `ReadU16` | 定义 | `bool ReadU16(std::span<const std::uint8_t> bytes, std::size_t& offset, std::uint16_t* value)` | 读取或恢复 read u16 相关逻辑。 |
| [L52](../src/protocol/RoomMemberControlProtocol.cpp#L52) | `ReadU64` | 定义 | `bool ReadU64(std::span<const std::uint8_t> bytes, std::size_t& offset, std::uint64_t* value)` | 读取或恢复 read u64 相关逻辑。 |
| [L65](../src/protocol/RoomMemberControlProtocol.cpp#L65) | `IsValidAction` | 定义 | `bool IsValidAction(RoomMemberAction action)` | 判断 is valid action 相关逻辑。 |
| [L78](../src/protocol/RoomMemberControlProtocol.cpp#L78) | `EncodeRoomMemberAction` | 定义 | `bool EncodeRoomMemberAction( const RoomMemberActionEnvelope& message, std::vector<std::uint8_t>* encoded, std::string* error)` | 编码 encode room member action 相关逻辑。 |
| [L140](../src/protocol/RoomMemberControlProtocol.cpp#L140) | `DecodeRoomMemberAction` | 定义 | `bool DecodeRoomMemberAction( std::span<const std::uint8_t> encoded, RoomMemberActionEnvelope* message, std::string* error)` | 解码或解析 decode room member action 相关逻辑。 |

## `src/protocol/RoomMemberControlProtocol.h`

[打开源码](../src/protocol/RoomMemberControlProtocol.h) · **文件作用：** 声明 room member control protocol 相关类型、接口、配置和成员状态。

### 类型

| 行 | 类型 | 种类 | 作用 |
|---:|---|---|---|
| [L17](../src/protocol/RoomMemberControlProtocol.h#L17) | `RoomMemberActionEnvelope` | struct | 定义 RoomMemberActionEnvelope 的 struct 类型和相关状态。 |

### 成员与文件级变量

| 行 | 变量 | 声明 | 作用 |
|---:|---|---|---|
| [L15](../src/protocol/RoomMemberControlProtocol.h#L15) | `kRoomMemberControlProtocolVersion` | `inline constexpr std::uint8_t kRoomMemberControlProtocolVersion = 1;` | 定义 room member control protocol version 的编译期常量或产品边界。 |
| [L18](../src/protocol/RoomMemberControlProtocol.h#L18) | `roomId` | `std::string roomId;` | 保存身份或作用域标识：room id。 |
| [L19](../src/protocol/RoomMemberControlProtocol.h#L19) | `senderDeviceId` | `std::string senderDeviceId;` | 保存身份或作用域标识：sender device id。 |
| [L20](../src/protocol/RoomMemberControlProtocol.h#L20) | `receiverDeviceId` | `std::string receiverDeviceId;` | 保存身份或作用域标识：receiver device id。 |
| [L21](../src/protocol/RoomMemberControlProtocol.h#L21) | `sequence` | `std::uint64_t sequence = 0;` | 保存单调序号，用于排序或去重：sequence。 |
| [L22](../src/protocol/RoomMemberControlProtocol.h#L22) | `screenShareEpoch` | `std::uint64_t screenShareEpoch = 0;` | 标记当前世代，用于拒绝过期异步结果：screen share epoch。 |
| [L23](../src/protocol/RoomMemberControlProtocol.h#L23) | `action` | `RoomMemberAction action = RoomMemberAction::kRequestScreenShare;` | 保存 action 相关配置或运行状态。 |
| [L24](../src/protocol/RoomMemberControlProtocol.h#L24) | `response` | `bool response = false;` | 保存 response 相关配置或运行状态。 |
| [L25](../src/protocol/RoomMemberControlProtocol.h#L25) | `accepted` | `bool accepted = false;` | 保存 accepted 相关配置或运行状态。 |
| [L26](../src/protocol/RoomMemberControlProtocol.h#L26) | `error` | `std::string error;` | 保存最近错误或失败原因：error。 |

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L29](../src/protocol/RoomMemberControlProtocol.h#L29) | `EncodeRoomMemberAction` | 声明 | `bool EncodeRoomMemberAction( const RoomMemberActionEnvelope& message, std::vector<std::uint8_t>* encoded, std::string* error = nullptr)` | 编码 encode room member action 相关逻辑。 |
| [L34](../src/protocol/RoomMemberControlProtocol.h#L34) | `DecodeRoomMemberAction` | 声明 | `bool DecodeRoomMemberAction( std::span<const std::uint8_t> encoded, RoomMemberActionEnvelope* message, std::string* error = nullptr)` | 解码或解析 decode room member action 相关逻辑。 |

## `src/protocol/ScreenShareControlProtocol.cpp`

[打开源码](../src/protocol/ScreenShareControlProtocol.cpp) · **文件作用：** 实现 screen share control protocol 相关函数与文件级辅助逻辑。

### 成员与文件级变量

| 行 | 变量 | 声明 | 作用 |
|---:|---|---|---|
| [L15](../src/protocol/ScreenShareControlProtocol.cpp#L15) | `kMagic` | `constexpr std::array<std::uint8_t, 4> kMagic = {'R', 'C', 'S', '1'};` | 定义 magic 的编译期常量或产品边界。 |
| [L16](../src/protocol/ScreenShareControlProtocol.cpp#L16) | `kFixedHeaderBytes` | `constexpr std::size_t kFixedHeaderBytes = 24;` | 定义 fixed header bytes 的编译期常量或产品边界。 |
| [L17](../src/protocol/ScreenShareControlProtocol.cpp#L17) | `kMaximumMessageBytes` | `constexpr std::size_t kMaximumMessageBytes = 512;` | 定义 maximum message bytes 的编译期常量或产品边界。 |
| [L18](../src/protocol/ScreenShareControlProtocol.cpp#L18) | `kMaximumRoomIdBytes` | `constexpr std::size_t kMaximumRoomIdBytes = 128;` | 定义 maximum room id bytes 的编译期常量或产品边界。 |
| [L19](../src/protocol/ScreenShareControlProtocol.cpp#L19) | `kMaximumDeviceIdBytes` | `constexpr std::size_t kMaximumDeviceIdBytes = 64;` | 定义 maximum device id bytes 的编译期常量或产品边界。 |
| [L20](../src/protocol/ScreenShareControlProtocol.cpp#L20) | `kMaximumErrorBytes` | `constexpr std::size_t kMaximumErrorBytes = 192;` | 定义 maximum error bytes 的编译期常量或产品边界。 |
| [L21](../src/protocol/ScreenShareControlProtocol.cpp#L21) | `kMaximumBackendNameBytes` | `constexpr std::size_t kMaximumBackendNameBytes = 64;` | 定义 maximum backend name bytes 的编译期常量或产品边界。 |
| [L22](../src/protocol/ScreenShareControlProtocol.cpp#L22) | `kMaximumDisplayKeyBytes` | `constexpr std::size_t kMaximumDisplayKeyBytes = 192;` | 定义 maximum display key bytes 的编译期常量或产品边界。 |
| [L23](../src/protocol/ScreenShareControlProtocol.cpp#L23) | `kMaximumDisplayNameBytes` | `constexpr std::size_t kMaximumDisplayNameBytes = 128;` | 定义 maximum display name bytes 的编译期常量或产品边界。 |
| [L24](../src/protocol/ScreenShareControlProtocol.cpp#L24) | `kMaximumDisplayCatalogBytes` | `constexpr std::size_t kMaximumDisplayCatalogBytes = 8192;` | 定义 maximum display catalog bytes 的编译期常量或产品边界。 |
| [L25](../src/protocol/ScreenShareControlProtocol.cpp#L25) | `kMaximumDisplayCount` | `constexpr std::size_t kMaximumDisplayCount = 16;` | 定义 maximum display count 的编译期常量或产品边界。 |

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L27](../src/protocol/ScreenShareControlProtocol.cpp#L27) | `SetError` | 定义 | `void SetError(std::string* error, std::string message)` | 更新或应用 set error 相关逻辑。 |
| [L34](../src/protocol/ScreenShareControlProtocol.cpp#L34) | `AppendU16` | 定义 | `void AppendU16(std::vector<std::uint8_t>* output, std::uint16_t value)` | 实现 append u16 对应的业务或工具逻辑。 |
| [L39](../src/protocol/ScreenShareControlProtocol.cpp#L39) | `AppendU32` | 定义 | `void AppendU32(std::vector<std::uint8_t>* output, std::uint32_t value)` | 实现 append u32 对应的业务或工具逻辑。 |
| [L44](../src/protocol/ScreenShareControlProtocol.cpp#L44) | `AppendU64` | 定义 | `void AppendU64(std::vector<std::uint8_t>* output, std::uint64_t value)` | 实现 append u64 对应的业务或工具逻辑。 |
| [L49](../src/protocol/ScreenShareControlProtocol.cpp#L49) | `ReadU16` | 定义 | `bool ReadU16(std::span<const std::uint8_t> bytes, std::size_t& offset, std::uint16_t* value)` | 读取或恢复 read u16 相关逻辑。 |
| [L62](../src/protocol/ScreenShareControlProtocol.cpp#L62) | `ReadU32` | 定义 | `bool ReadU32(std::span<const std::uint8_t> bytes, std::size_t& offset, std::uint32_t* value)` | 读取或恢复 read u32 相关逻辑。 |
| [L75](../src/protocol/ScreenShareControlProtocol.cpp#L75) | `ReadU64` | 定义 | `bool ReadU64(std::span<const std::uint8_t> bytes, std::size_t& offset, std::uint64_t* value)` | 读取或恢复 read u64 相关逻辑。 |
| [L90](../src/protocol/ScreenShareControlProtocol.cpp#L90) | `EncodeScreenFrameRateRequest` | 定义 | `bool EncodeScreenFrameRateRequest( const ScreenFrameRateRequest& request, std::vector<std::uint8_t>* encoded, std::string* error)` | 编码 encode screen frame rate request 相关逻辑。 |
| [L138](../src/protocol/ScreenShareControlProtocol.cpp#L138) | `DecodeScreenFrameRateRequest` | 定义 | `bool DecodeScreenFrameRateRequest( std::span<const std::uint8_t> encoded, ScreenFrameRateRequest* request, std::string* error)` | 解码或解析 decode screen frame rate request 相关逻辑。 |
| [L184](../src/protocol/ScreenShareControlProtocol.cpp#L184) | `EncodeScreenStreamPreferenceRequest` | 定义 | `bool EncodeScreenStreamPreferenceRequest( const ScreenStreamPreferenceRequest& request, std::vector<std::uint8_t>* encoded, std::string* error)` | 编码 encode screen stream preference request 相关逻辑。 |
| [L230](../src/protocol/ScreenShareControlProtocol.cpp#L230) | `DecodeScreenStreamPreferenceRequest` | 定义 | `bool DecodeScreenStreamPreferenceRequest( std::span<const std::uint8_t> encoded, ScreenStreamPreferenceRequest* request, std::string* error)` | 解码或解析 decode screen stream preference request 相关逻辑。 |
| [L292](../src/protocol/ScreenShareControlProtocol.cpp#L292) | `EncodeScreenStreamPreferenceApplied` | 定义 | `bool EncodeScreenStreamPreferenceApplied( const ScreenStreamPreferenceApplied& applied, std::vector<std::uint8_t>* encoded, std::string* error)` | 编码 encode screen stream preference applied 相关逻辑。 |
| [L335](../src/protocol/ScreenShareControlProtocol.cpp#L335) | `DecodeScreenStreamPreferenceApplied` | 定义 | `bool DecodeScreenStreamPreferenceApplied( std::span<const std::uint8_t> encoded, ScreenStreamPreferenceApplied* applied, std::string* error)` | 解码或解析 decode screen stream preference applied 相关逻辑。 |
| [L394](../src/protocol/ScreenShareControlProtocol.cpp#L394) | `EncodeScreenStreamPreferenceAppliedV2` | 定义 | `bool EncodeScreenStreamPreferenceAppliedV2( const ScreenStreamPreferenceApplied& applied, std::vector<std::uint8_t>* encoded, std::string* error)` | 编码 encode screen stream preference applied v2 相关逻辑。 |
| [L447](../src/protocol/ScreenShareControlProtocol.cpp#L447) | `DecodeScreenStreamPreferenceAppliedV2` | 定义 | `bool DecodeScreenStreamPreferenceAppliedV2( std::span<const std::uint8_t> encoded, ScreenStreamPreferenceApplied* applied, std::string* error)` | 解码或解析 decode screen stream preference applied v2 相关逻辑。 |
| [L515](../src/protocol/ScreenShareControlProtocol.cpp#L515) | `EncodeScreenCaptureCapability` | 定义 | `bool EncodeScreenCaptureCapability( const ScreenCaptureCapability& capability, std::vector<std::uint8_t>* encoded, std::string* error)` | 编码 encode screen capture capability 相关逻辑。 |
| [L550](../src/protocol/ScreenShareControlProtocol.cpp#L550) | `DecodeScreenCaptureCapability` | 定义 | `bool DecodeScreenCaptureCapability( std::span<const std::uint8_t> encoded, ScreenCaptureCapability* capability, std::string* error)` | 解码或解析 decode screen capture capability 相关逻辑。 |
| [L593](../src/protocol/ScreenShareControlProtocol.cpp#L593) | `EncodeScreenCaptureRuntimeCapability` | 定义 | `bool EncodeScreenCaptureRuntimeCapability( const ScreenCaptureRuntimeCapability& capability, std::vector<std::uint8_t>* encoded, std::string* error)` | 编码 encode screen capture runtime capability 相关逻辑。 |
| [L655](../src/protocol/ScreenShareControlProtocol.cpp#L655) | `DecodeScreenCaptureRuntimeCapability` | 定义 | `bool DecodeScreenCaptureRuntimeCapability( std::span<const std::uint8_t> encoded, ScreenCaptureRuntimeCapability* capability, std::string* error)` | 解码或解析 decode screen capture runtime capability 相关逻辑。 |
| [L715](../src/protocol/ScreenShareControlProtocol.cpp#L715) | `EncodeSharedDisplayLayout` | 定义 | `bool EncodeSharedDisplayLayout( const SharedDisplayLayout& layout, std::vector<std::uint8_t>* encoded, std::string* error)` | 编码 encode shared display layout 相关逻辑。 |
| [L784](../src/protocol/ScreenShareControlProtocol.cpp#L784) | `DecodeSharedDisplayLayout` | 定义 | `bool DecodeSharedDisplayLayout( std::span<const std::uint8_t> encoded, SharedDisplayLayout* layout, std::string* error)` | 解码或解析 decode shared display layout 相关逻辑。 |
| [L860](../src/protocol/ScreenShareControlProtocol.cpp#L860) | `EncodeSharedDisplayCatalog` | 定义 | `bool EncodeSharedDisplayCatalog( const SharedDisplayCatalog& catalog, std::vector<std::uint8_t>* encoded, std::string* error)` | 编码 encode shared display catalog 相关逻辑。 |
| [L950](../src/protocol/ScreenShareControlProtocol.cpp#L950) | `DecodeSharedDisplayCatalog` | 定义 | `bool DecodeSharedDisplayCatalog( std::span<const std::uint8_t> encoded, SharedDisplayCatalog* catalog, std::string* error)` | 解码或解析 decode shared display catalog 相关逻辑。 |
| [L1061](../src/protocol/ScreenShareControlProtocol.cpp#L1061) | `EncodeSharedDisplaySwitchRequest` | 定义 | `bool EncodeSharedDisplaySwitchRequest( const SharedDisplaySwitchRequest& request, std::vector<std::uint8_t>* encoded, std::string* error)` | 编码 encode shared display switch request 相关逻辑。 |
| [L1113](../src/protocol/ScreenShareControlProtocol.cpp#L1113) | `DecodeSharedDisplaySwitchRequest` | 定义 | `bool DecodeSharedDisplaySwitchRequest( std::span<const std::uint8_t> encoded, SharedDisplaySwitchRequest* request, std::string* error)` | 解码或解析 decode shared display switch request 相关逻辑。 |
| [L1169](../src/protocol/ScreenShareControlProtocol.cpp#L1169) | `EncodeSharedDisplaySwitchApplied` | 定义 | `bool EncodeSharedDisplaySwitchApplied( const SharedDisplaySwitchApplied& applied, std::vector<std::uint8_t>* encoded, std::string* error)` | 编码 encode shared display switch applied 相关逻辑。 |
| [L1227](../src/protocol/ScreenShareControlProtocol.cpp#L1227) | `DecodeSharedDisplaySwitchApplied` | 定义 | `bool DecodeSharedDisplaySwitchApplied( std::span<const std::uint8_t> encoded, SharedDisplaySwitchApplied* applied, std::string* error)` | 解码或解析 decode shared display switch applied 相关逻辑。 |
| [L1288](../src/protocol/ScreenShareControlProtocol.cpp#L1288) | `EncodeScreenRefreshRequest` | 定义 | `bool EncodeScreenRefreshRequest( const ScreenRefreshRequest& request, std::vector<std::uint8_t>* encoded, std::string* error)` | 编码 encode screen refresh request 相关逻辑。 |
| [L1333](../src/protocol/ScreenShareControlProtocol.cpp#L1333) | `DecodeScreenRefreshRequest` | 定义 | `bool DecodeScreenRefreshRequest( std::span<const std::uint8_t> encoded, ScreenRefreshRequest* request, std::string* error)` | 解码或解析 decode screen refresh request 相关逻辑。 |
| [L1383](../src/protocol/ScreenShareControlProtocol.cpp#L1383) | `EncodeScreenFirstFramePresented` | 定义 | `bool EncodeScreenFirstFramePresented( const ScreenFirstFramePresented& presented, std::vector<std::uint8_t>* encoded, std::string* error)` | 编码 encode screen first frame presented 相关逻辑。 |
| [L1428](../src/protocol/ScreenShareControlProtocol.cpp#L1428) | `DecodeScreenFirstFramePresented` | 定义 | `bool DecodeScreenFirstFramePresented( std::span<const std::uint8_t> encoded, ScreenFirstFramePresented* presented, std::string* error)` | 解码或解析 decode screen first frame presented 相关逻辑。 |

## `src/protocol/ScreenShareControlProtocol.h`

[打开源码](../src/protocol/ScreenShareControlProtocol.h) · **文件作用：** 声明 screen share control protocol 相关类型、接口、配置和成员状态。

### 类型

| 行 | 类型 | 种类 | 作用 |
|---:|---|---|---|
| [L25](../src/protocol/ScreenShareControlProtocol.h#L25) | `ScreenQualityTier` | enum class | 定义 ScreenQualityTier 的 enum class 类型和相关状态。 |
| [L33](../src/protocol/ScreenShareControlProtocol.h#L33) | `ScreenScaleBackend` | enum class | 定义 ScreenScaleBackend 的 enum class 类型和相关状态。 |
| [L40](../src/protocol/ScreenShareControlProtocol.h#L40) | `ScreenFrameRateRequest` | struct | 定义 ScreenFrameRateRequest 的 struct 类型和相关状态。 |
| [L50](../src/protocol/ScreenShareControlProtocol.h#L50) | `ScreenStreamPreferenceRequest` | struct | width/height are a bounding box. A zero pair requests the original source size. The sender preserves the desktop aspect ratio and uses even output dimensions required by H264/NV12. |
| [L60](../src/protocol/ScreenShareControlProtocol.h#L60) | `ScreenStreamPreferenceApplied` | struct | 定义 ScreenStreamPreferenceApplied 的 struct 类型和相关状态。 |
| [L74](../src/protocol/ScreenShareControlProtocol.h#L74) | `ScreenCaptureCapability` | struct | 定义 ScreenCaptureCapability 的 struct 类型和相关状态。 |
| [L82](../src/protocol/ScreenShareControlProtocol.h#L82) | `ScreenCaptureRuntimeCapability` | struct | The legacy type-4 capability above remains in use for compatibility. This detailed type-5 report explains why the active sender exposes that limit. |
| [L91](../src/protocol/ScreenShareControlProtocol.h#L91) | `SharedDisplayLayout` | struct | 定义 SharedDisplayLayout 的 struct 类型和相关状态。 |
| [L99](../src/protocol/ScreenShareControlProtocol.h#L99) | `SharedDisplayCatalog` | struct | 定义 SharedDisplayCatalog 的 struct 类型和相关状态。 |
| [L107](../src/protocol/ScreenShareControlProtocol.h#L107) | `SharedDisplaySwitchRequest` | struct | 定义 SharedDisplaySwitchRequest 的 struct 类型和相关状态。 |
| [L115](../src/protocol/ScreenShareControlProtocol.h#L115) | `SharedDisplaySwitchApplied` | struct | 定义 SharedDisplaySwitchApplied 的 struct 类型和相关状态。 |
| [L128](../src/protocol/ScreenShareControlProtocol.h#L128) | `ScreenRefreshRequest` | struct | Sent by a viewer immediately after binding the screen sink. The sharer answers it by producing a bounded full-frame burst; the receiver separately requests a codec keyframe thro... |
| [L139](../src/protocol/ScreenShareControlProtocol.h#L139) | `ScreenFirstFramePresented` | struct | Sent by a viewer only after the first frame of a screen-share generation has actually been presented. This is intentionally later than RTP receipt or decode completion, so the s... |

### 成员与文件级变量

| 行 | 变量 | 声明 | 作用 |
|---:|---|---|---|
| [L16](../src/protocol/ScreenShareControlProtocol.h#L16) | `kScreenShareControlProtocolVersion` | `inline constexpr std::uint8_t kScreenShareControlProtocolVersion = 1;` | 定义 screen share control protocol version 的编译期常量或产品边界。 |
| [L17](../src/protocol/ScreenShareControlProtocol.h#L17) | `kMinimumScreenFrameRate` | `inline constexpr std::uint32_t kMinimumScreenFrameRate = 5;` | 定义 minimum screen frame rate 的编译期常量或产品边界。 |
| [L18](../src/protocol/ScreenShareControlProtocol.h#L18) | `kDefaultScreenFrameRate` | `inline constexpr std::uint32_t kDefaultScreenFrameRate = 60;` | 定义 default screen frame rate 的编译期常量或产品边界。 |
| [L19](../src/protocol/ScreenShareControlProtocol.h#L19) | `kMaximumScreenFrameRate` | `inline constexpr std::uint32_t kMaximumScreenFrameRate = 120;` | 定义 maximum screen frame rate 的编译期常量或产品边界。 |
| [L20](../src/protocol/ScreenShareControlProtocol.h#L20) | `kMultiMemberMaximumScreenFrameRate` | `inline constexpr std::uint32_t kMultiMemberMaximumScreenFrameRate = 60;` | 定义 multi member maximum screen frame rate 的编译期常量或产品边界。 |
| [L21](../src/protocol/ScreenShareControlProtocol.h#L21) | `kHighOccupancyRoomMemberThreshold` | `inline constexpr std::size_t kHighOccupancyRoomMemberThreshold = 2;` | 定义 high occupancy room member threshold 的编译期常量或产品边界。 |
| [L22](../src/protocol/ScreenShareControlProtocol.h#L22) | `kMaximumScreenStreamWidth` | `inline constexpr std::uint32_t kMaximumScreenStreamWidth = 7680;` | 定义 maximum screen stream width 的编译期常量或产品边界。 |
| [L23](../src/protocol/ScreenShareControlProtocol.h#L23) | `kMaximumScreenStreamHeight` | `inline constexpr std::uint32_t kMaximumScreenStreamHeight = 4320;` | 定义 maximum screen stream height 的编译期常量或产品边界。 |
| [L41](../src/protocol/ScreenShareControlProtocol.h#L41) | `roomId` | `std::string roomId;` | 保存身份或作用域标识：room id。 |
| [L42](../src/protocol/ScreenShareControlProtocol.h#L42) | `senderDeviceId` | `std::string senderDeviceId;` | 保存身份或作用域标识：sender device id。 |
| [L43](../src/protocol/ScreenShareControlProtocol.h#L43) | `sequence` | `std::uint64_t sequence = 0;` | 保存单调序号，用于排序或去重：sequence。 |
| [L44](../src/protocol/ScreenShareControlProtocol.h#L44) | `framesPerSecond` | `std::uint32_t framesPerSecond = kDefaultScreenFrameRate;` | 保存 frames per second 相关配置或运行状态。 |
| [L51](../src/protocol/ScreenShareControlProtocol.h#L51) | `roomId` | `std::string roomId;` | 保存身份或作用域标识：room id。 |
| [L52](../src/protocol/ScreenShareControlProtocol.h#L52) | `senderDeviceId` | `std::string senderDeviceId;` | 保存身份或作用域标识：sender device id。 |
| [L53](../src/protocol/ScreenShareControlProtocol.h#L53) | `sequence` | `std::uint64_t sequence = 0;` | 保存单调序号，用于排序或去重：sequence。 |
| [L54](../src/protocol/ScreenShareControlProtocol.h#L54) | `maxWidth` | `std::uint32_t maxWidth = 0;` | 保存计数、尺寸或速率指标：max width。 |
| [L55](../src/protocol/ScreenShareControlProtocol.h#L55) | `maxHeight` | `std::uint32_t maxHeight = 0;` | 保存计数、尺寸或速率指标：max height。 |
| [L56](../src/protocol/ScreenShareControlProtocol.h#L56) | `framesPerSecond` | `std::uint32_t framesPerSecond = kDefaultScreenFrameRate;` | 保存 frames per second 相关配置或运行状态。 |
| [L57](../src/protocol/ScreenShareControlProtocol.h#L57) | `quality` | `ScreenQualityTier quality = ScreenQualityTier::kAutomatic;` | 保存 quality 相关配置或运行状态。 |
| [L61](../src/protocol/ScreenShareControlProtocol.h#L61) | `roomId` | `std::string roomId;` | 保存身份或作用域标识：room id。 |
| [L62](../src/protocol/ScreenShareControlProtocol.h#L62) | `senderDeviceId` | `std::string senderDeviceId;` | 保存身份或作用域标识：sender device id。 |
| [L63](../src/protocol/ScreenShareControlProtocol.h#L63) | `requestSequence` | `std::uint64_t requestSequence = 0;` | 保存单调序号，用于排序或去重：request sequence。 |
| [L64](../src/protocol/ScreenShareControlProtocol.h#L64) | `screenShareGeneration` | `std::uint64_t screenShareGeneration = 0;` | 标记当前世代，用于拒绝过期异步结果：screen share generation。 |
| [L65](../src/protocol/ScreenShareControlProtocol.h#L65) | `width` | `std::uint32_t width = 0;` | 保存计数、尺寸或速率指标：width。 |
| [L66](../src/protocol/ScreenShareControlProtocol.h#L66) | `height` | `std::uint32_t height = 0;` | 保存计数、尺寸或速率指标：height。 |
| [L67](../src/protocol/ScreenShareControlProtocol.h#L67) | `framesPerSecond` | `std::uint32_t framesPerSecond = 0;` | 保存 frames per second 相关配置或运行状态。 |
| [L68](../src/protocol/ScreenShareControlProtocol.h#L68) | `maxBitrateBps` | `std::uint32_t maxBitrateBps = 0;` | 保存计数、尺寸或速率指标：max bitrate bps。 |
| [L69](../src/protocol/ScreenShareControlProtocol.h#L69) | `scaleBackend` | `ScreenScaleBackend scaleBackend = ScreenScaleBackend::kUnknown;` | 保存 scale backend 相关配置或运行状态。 |
| [L70](../src/protocol/ScreenShareControlProtocol.h#L70) | `accepted` | `bool accepted = false;` | 保存 accepted 相关配置或运行状态。 |
| [L71](../src/protocol/ScreenShareControlProtocol.h#L71) | `error` | `std::string error;` | 保存最近错误或失败原因：error。 |
| [L75](../src/protocol/ScreenShareControlProtocol.h#L75) | `roomId` | `std::string roomId;` | 保存身份或作用域标识：room id。 |
| [L76](../src/protocol/ScreenShareControlProtocol.h#L76) | `senderDeviceId` | `std::string senderDeviceId;` | 保存身份或作用域标识：sender device id。 |
| [L77](../src/protocol/ScreenShareControlProtocol.h#L77) | `maximumFrameRate` | `std::uint32_t maximumFrameRate = 60;` | 保存计数、尺寸或速率指标：maximum frame rate。 |
| [L83](../src/protocol/ScreenShareControlProtocol.h#L83) | `roomId` | `std::string roomId;` | 保存身份或作用域标识：room id。 |
| [L84](../src/protocol/ScreenShareControlProtocol.h#L84) | `senderDeviceId` | `std::string senderDeviceId;` | 保存身份或作用域标识：sender device id。 |
| [L85](../src/protocol/ScreenShareControlProtocol.h#L85) | `maximumFrameRate` | `std::uint32_t maximumFrameRate = 60;` | 保存计数、尺寸或速率指标：maximum frame rate。 |
| [L86](../src/protocol/ScreenShareControlProtocol.h#L86) | `configuredBackend` | `std::string configuredBackend;` | 保存 configured backend 相关配置或运行状态。 |
| [L87](../src/protocol/ScreenShareControlProtocol.h#L87) | `activeBackend` | `std::string activeBackend;` | 保存 active backend 相关配置或运行状态。 |
| [L88](../src/protocol/ScreenShareControlProtocol.h#L88) | `fallbackReason` | `std::string fallbackReason;` | 保存 fallback reason 相关配置或运行状态。 |
| [L92](../src/protocol/ScreenShareControlProtocol.h#L92) | `roomId` | `std::string roomId;` | 保存身份或作用域标识：room id。 |
| [L93](../src/protocol/ScreenShareControlProtocol.h#L93) | `senderDeviceId` | `std::string senderDeviceId;` | 保存身份或作用域标识：sender device id。 |
| [L94](../src/protocol/ScreenShareControlProtocol.h#L94) | `screenShareGeneration` | `std::uint64_t screenShareGeneration = 0;` | 标记当前世代，用于拒绝过期异步结果：screen share generation。 |
| [L95](../src/protocol/ScreenShareControlProtocol.h#L95) | `layoutVersion` | `std::uint64_t layoutVersion = 0;` | 保存 layout version 相关配置或运行状态。 |
| [L96](../src/protocol/ScreenShareControlProtocol.h#L96) | `selectedDisplay` | `DisplayDescriptor selectedDisplay;` | 保存 selected display 相关配置或运行状态。 |
| [L100](../src/protocol/ScreenShareControlProtocol.h#L100) | `roomId` | `std::string roomId;` | 保存身份或作用域标识：room id。 |
| [L101](../src/protocol/ScreenShareControlProtocol.h#L101) | `senderDeviceId` | `std::string senderDeviceId;` | 保存身份或作用域标识：sender device id。 |
| [L102](../src/protocol/ScreenShareControlProtocol.h#L102) | `screenShareGeneration` | `std::uint64_t screenShareGeneration = 0;` | 标记当前世代，用于拒绝过期异步结果：screen share generation。 |
| [L103](../src/protocol/ScreenShareControlProtocol.h#L103) | `layoutVersion` | `std::uint64_t layoutVersion = 0;` | 保存 layout version 相关配置或运行状态。 |
| [L104](../src/protocol/ScreenShareControlProtocol.h#L104) | `displays` | `std::vector<DisplayDescriptor> displays;` | 保存 displays 相关配置或运行状态。 |
| [L108](../src/protocol/ScreenShareControlProtocol.h#L108) | `roomId` | `std::string roomId;` | 保存身份或作用域标识：room id。 |
| [L109](../src/protocol/ScreenShareControlProtocol.h#L109) | `senderDeviceId` | `std::string senderDeviceId;` | 保存身份或作用域标识：sender device id。 |
| [L110](../src/protocol/ScreenShareControlProtocol.h#L110) | `sequence` | `std::uint64_t sequence = 0;` | 保存单调序号，用于排序或去重：sequence。 |
| [L111](../src/protocol/ScreenShareControlProtocol.h#L111) | `screenShareGeneration` | `std::uint64_t screenShareGeneration = 0;` | 标记当前世代，用于拒绝过期异步结果：screen share generation。 |
| [L112](../src/protocol/ScreenShareControlProtocol.h#L112) | `stableDisplayKey` | `std::string stableDisplayKey;` | 保存 stable display key 相关配置或运行状态。 |
| [L116](../src/protocol/ScreenShareControlProtocol.h#L116) | `roomId` | `std::string roomId;` | 保存身份或作用域标识：room id。 |
| [L117](../src/protocol/ScreenShareControlProtocol.h#L117) | `senderDeviceId` | `std::string senderDeviceId;` | 保存身份或作用域标识：sender device id。 |
| [L118](../src/protocol/ScreenShareControlProtocol.h#L118) | `requestSequence` | `std::uint64_t requestSequence = 0;` | 保存单调序号，用于排序或去重：request sequence。 |
| [L119](../src/protocol/ScreenShareControlProtocol.h#L119) | `screenShareGeneration` | `std::uint64_t screenShareGeneration = 0;` | 标记当前世代，用于拒绝过期异步结果：screen share generation。 |
| [L120](../src/protocol/ScreenShareControlProtocol.h#L120) | `stableDisplayKey` | `std::string stableDisplayKey;` | 保存 stable display key 相关配置或运行状态。 |
| [L121](../src/protocol/ScreenShareControlProtocol.h#L121) | `accepted` | `bool accepted = false;` | 保存 accepted 相关配置或运行状态。 |
| [L122](../src/protocol/ScreenShareControlProtocol.h#L122) | `error` | `std::string error;` | 保存最近错误或失败原因：error。 |
| [L129](../src/protocol/ScreenShareControlProtocol.h#L129) | `roomId` | `std::string roomId;` | 保存身份或作用域标识：room id。 |
| [L130](../src/protocol/ScreenShareControlProtocol.h#L130) | `senderDeviceId` | `std::string senderDeviceId;` | 保存身份或作用域标识：sender device id。 |
| [L131](../src/protocol/ScreenShareControlProtocol.h#L131) | `sequence` | `std::uint64_t sequence = 0;` | 保存单调序号，用于排序或去重：sequence。 |
| [L132](../src/protocol/ScreenShareControlProtocol.h#L132) | `screenShareGeneration` | `std::uint64_t screenShareGeneration = 0;` | 标记当前世代，用于拒绝过期异步结果：screen share generation。 |
| [L140](../src/protocol/ScreenShareControlProtocol.h#L140) | `roomId` | `std::string roomId;` | 保存身份或作用域标识：room id。 |
| [L141](../src/protocol/ScreenShareControlProtocol.h#L141) | `senderDeviceId` | `std::string senderDeviceId;` | 保存身份或作用域标识：sender device id。 |
| [L142](../src/protocol/ScreenShareControlProtocol.h#L142) | `sequence` | `std::uint64_t sequence = 0;` | 保存单调序号，用于排序或去重：sequence。 |
| [L143](../src/protocol/ScreenShareControlProtocol.h#L143) | `screenShareGeneration` | `std::uint64_t screenShareGeneration = 0;` | 标记当前世代，用于拒绝过期异步结果：screen share generation。 |
| [L144](../src/protocol/ScreenShareControlProtocol.h#L144) | `startupElapsedMs` | `std::uint32_t startupElapsedMs = 0;` | 保存 startup elapsed ms 相关配置或运行状态。 |

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L147](../src/protocol/ScreenShareControlProtocol.h#L147) | `EncodeScreenFrameRateRequest` | 声明 | `bool EncodeScreenFrameRateRequest( const ScreenFrameRateRequest& request, std::vector<std::uint8_t>* encoded, std::string* error = nullptr)` | 编码 encode screen frame rate request 相关逻辑。 |
| [L152](../src/protocol/ScreenShareControlProtocol.h#L152) | `DecodeScreenFrameRateRequest` | 声明 | `bool DecodeScreenFrameRateRequest( std::span<const std::uint8_t> encoded, ScreenFrameRateRequest* request, std::string* error = nullptr)` | 解码或解析 decode screen frame rate request 相关逻辑。 |
| [L157](../src/protocol/ScreenShareControlProtocol.h#L157) | `EncodeScreenStreamPreferenceRequest` | 声明 | `bool EncodeScreenStreamPreferenceRequest( const ScreenStreamPreferenceRequest& request, std::vector<std::uint8_t>* encoded, std::string* error = nullptr)` | 编码 encode screen stream preference request 相关逻辑。 |
| [L162](../src/protocol/ScreenShareControlProtocol.h#L162) | `DecodeScreenStreamPreferenceRequest` | 声明 | `bool DecodeScreenStreamPreferenceRequest( std::span<const std::uint8_t> encoded, ScreenStreamPreferenceRequest* request, std::string* error = nullptr)` | 解码或解析 decode screen stream preference request 相关逻辑。 |
| [L167](../src/protocol/ScreenShareControlProtocol.h#L167) | `EncodeScreenStreamPreferenceApplied` | 声明 | `bool EncodeScreenStreamPreferenceApplied( const ScreenStreamPreferenceApplied& applied, std::vector<std::uint8_t>* encoded, std::string* error = nullptr)` | 编码 encode screen stream preference applied 相关逻辑。 |
| [L172](../src/protocol/ScreenShareControlProtocol.h#L172) | `DecodeScreenStreamPreferenceApplied` | 声明 | `bool DecodeScreenStreamPreferenceApplied( std::span<const std::uint8_t> encoded, ScreenStreamPreferenceApplied* applied, std::string* error = nullptr)` | 解码或解析 decode screen stream preference applied 相关逻辑。 |
| [L180](../src/protocol/ScreenShareControlProtocol.h#L180) | `EncodeScreenStreamPreferenceAppliedV2` | 声明 | `bool EncodeScreenStreamPreferenceAppliedV2( const ScreenStreamPreferenceApplied& applied, std::vector<std::uint8_t>* encoded, std::string* error = nullptr)` | Type-11 extends the legacy type-3 result with the authoritative screen share generation. New peers send both messages (V2 first) so older peers continue to receive type-3 while ... |
| [L185](../src/protocol/ScreenShareControlProtocol.h#L185) | `DecodeScreenStreamPreferenceAppliedV2` | 声明 | `bool DecodeScreenStreamPreferenceAppliedV2( std::span<const std::uint8_t> encoded, ScreenStreamPreferenceApplied* applied, std::string* error = nullptr)` | 解码或解析 decode screen stream preference applied v2 相关逻辑。 |
| [L190](../src/protocol/ScreenShareControlProtocol.h#L190) | `EncodeScreenCaptureCapability` | 声明 | `bool EncodeScreenCaptureCapability( const ScreenCaptureCapability& capability, std::vector<std::uint8_t>* encoded, std::string* error = nullptr)` | 编码 encode screen capture capability 相关逻辑。 |
| [L195](../src/protocol/ScreenShareControlProtocol.h#L195) | `DecodeScreenCaptureCapability` | 声明 | `bool DecodeScreenCaptureCapability( std::span<const std::uint8_t> encoded, ScreenCaptureCapability* capability, std::string* error = nullptr)` | 解码或解析 decode screen capture capability 相关逻辑。 |
| [L200](../src/protocol/ScreenShareControlProtocol.h#L200) | `EncodeScreenCaptureRuntimeCapability` | 声明 | `bool EncodeScreenCaptureRuntimeCapability( const ScreenCaptureRuntimeCapability& capability, std::vector<std::uint8_t>* encoded, std::string* error = nullptr)` | 编码 encode screen capture runtime capability 相关逻辑。 |
| [L205](../src/protocol/ScreenShareControlProtocol.h#L205) | `DecodeScreenCaptureRuntimeCapability` | 声明 | `bool DecodeScreenCaptureRuntimeCapability( std::span<const std::uint8_t> encoded, ScreenCaptureRuntimeCapability* capability, std::string* error = nullptr)` | 解码或解析 decode screen capture runtime capability 相关逻辑。 |
| [L210](../src/protocol/ScreenShareControlProtocol.h#L210) | `EncodeSharedDisplayLayout` | 声明 | `bool EncodeSharedDisplayLayout( const SharedDisplayLayout& layout, std::vector<std::uint8_t>* encoded, std::string* error = nullptr)` | 编码 encode shared display layout 相关逻辑。 |
| [L215](../src/protocol/ScreenShareControlProtocol.h#L215) | `DecodeSharedDisplayLayout` | 声明 | `bool DecodeSharedDisplayLayout( std::span<const std::uint8_t> encoded, SharedDisplayLayout* layout, std::string* error = nullptr)` | 解码或解析 decode shared display layout 相关逻辑。 |
| [L220](../src/protocol/ScreenShareControlProtocol.h#L220) | `EncodeSharedDisplayCatalog` | 声明 | `bool EncodeSharedDisplayCatalog( const SharedDisplayCatalog& catalog, std::vector<std::uint8_t>* encoded, std::string* error = nullptr)` | 编码 encode shared display catalog 相关逻辑。 |
| [L225](../src/protocol/ScreenShareControlProtocol.h#L225) | `DecodeSharedDisplayCatalog` | 声明 | `bool DecodeSharedDisplayCatalog( std::span<const std::uint8_t> encoded, SharedDisplayCatalog* catalog, std::string* error = nullptr)` | 解码或解析 decode shared display catalog 相关逻辑。 |
| [L230](../src/protocol/ScreenShareControlProtocol.h#L230) | `EncodeSharedDisplaySwitchRequest` | 声明 | `bool EncodeSharedDisplaySwitchRequest( const SharedDisplaySwitchRequest& request, std::vector<std::uint8_t>* encoded, std::string* error = nullptr)` | 编码 encode shared display switch request 相关逻辑。 |
| [L235](../src/protocol/ScreenShareControlProtocol.h#L235) | `DecodeSharedDisplaySwitchRequest` | 声明 | `bool DecodeSharedDisplaySwitchRequest( std::span<const std::uint8_t> encoded, SharedDisplaySwitchRequest* request, std::string* error = nullptr)` | 解码或解析 decode shared display switch request 相关逻辑。 |
| [L240](../src/protocol/ScreenShareControlProtocol.h#L240) | `EncodeSharedDisplaySwitchApplied` | 声明 | `bool EncodeSharedDisplaySwitchApplied( const SharedDisplaySwitchApplied& applied, std::vector<std::uint8_t>* encoded, std::string* error = nullptr)` | 编码 encode shared display switch applied 相关逻辑。 |
| [L245](../src/protocol/ScreenShareControlProtocol.h#L245) | `DecodeSharedDisplaySwitchApplied` | 声明 | `bool DecodeSharedDisplaySwitchApplied( std::span<const std::uint8_t> encoded, SharedDisplaySwitchApplied* applied, std::string* error = nullptr)` | 解码或解析 decode shared display switch applied 相关逻辑。 |
| [L250](../src/protocol/ScreenShareControlProtocol.h#L250) | `EncodeScreenRefreshRequest` | 声明 | `bool EncodeScreenRefreshRequest( const ScreenRefreshRequest& request, std::vector<std::uint8_t>* encoded, std::string* error = nullptr)` | 编码 encode screen refresh request 相关逻辑。 |
| [L255](../src/protocol/ScreenShareControlProtocol.h#L255) | `DecodeScreenRefreshRequest` | 声明 | `bool DecodeScreenRefreshRequest( std::span<const std::uint8_t> encoded, ScreenRefreshRequest* request, std::string* error = nullptr)` | 解码或解析 decode screen refresh request 相关逻辑。 |
| [L260](../src/protocol/ScreenShareControlProtocol.h#L260) | `EncodeScreenFirstFramePresented` | 声明 | `bool EncodeScreenFirstFramePresented( const ScreenFirstFramePresented& presented, std::vector<std::uint8_t>* encoded, std::string* error = nullptr)` | 编码 encode screen first frame presented 相关逻辑。 |
| [L265](../src/protocol/ScreenShareControlProtocol.h#L265) | `DecodeScreenFirstFramePresented` | 声明 | `bool DecodeScreenFirstFramePresented( std::span<const std::uint8_t> encoded, ScreenFirstFramePresented* presented, std::string* error = nullptr)` | 解码或解析 decode screen first frame presented 相关逻辑。 |
