# WebRTC 传输与媒体运行时

> 自动生成于 2026-09-06，源码树 `cfeee508b14c-dirty`。请运行 `tools/Generate-SourceSymbolReference.ps1` 刷新。

PeerConnection、RTP sender、DataChannel、统计、编解码工厂与运行时线程。

本册共收录 27 个源码文件。函数与变量的中文作用优先采用源码紧邻注释；无注释时根据符号命名生成阅读提示，最终语义仍以源码为准。

## `src/webrtc/IWebRtcSession.h`

[打开源码](../src/webrtc/IWebRtcSession.h) · **文件作用：** 声明 i web rtc session 相关类型、接口、配置和成员状态。

### 类型

| 行 | 类型 | 种类 | 作用 |
|---:|---|---|---|
| [L24](../src/webrtc/IWebRtcSession.h#L24) | `SessionDescriptionType` | enum class | 定义 SessionDescriptionType 的 enum class 类型和相关状态。 |
| [L29](../src/webrtc/IWebRtcSession.h#L29) | `WebRtcSessionState` | enum class | 定义 WebRtcSessionState 的 enum class 类型和相关状态。 |
| [L41](../src/webrtc/IWebRtcSession.h#L41) | `WebRtcIceGatheringState` | enum class | 定义 WebRtcIceGatheringState 的 enum class 类型和相关状态。 |
| [L47](../src/webrtc/IWebRtcSession.h#L47) | `DataChannelState` | enum class | 定义 DataChannelState 的 enum class 类型和相关状态。 |
| [L54](../src/webrtc/IWebRtcSession.h#L54) | `SendResult` | enum class | 定义 SendResult 的 enum class 类型和相关状态。 |
| [L62](../src/webrtc/IWebRtcSession.h#L62) | `SessionDescription` | struct | 定义 SessionDescription 的 struct 类型和相关状态。 |
| [L70](../src/webrtc/IWebRtcSession.h#L70) | `IceCandidate` | struct | 定义 IceCandidate 的 struct 类型和相关状态。 |
| [L77](../src/webrtc/IWebRtcSession.h#L77) | `IceServerConfig` | struct | 定义 IceServerConfig 的 struct 类型和相关状态。 |
| [L83](../src/webrtc/IWebRtcSession.h#L83) | `WebRtcSessionConfig` | struct | 定义 WebRtcSessionConfig 的 struct 类型和相关状态。 |
| [L92](../src/webrtc/IWebRtcSession.h#L92) | `DataChannelSpec` | struct | 定义 DataChannelSpec 的 struct 类型和相关状态。 |
| [L93](../src/webrtc/IWebRtcSession.h#L93) | `Priority` | enum class | 定义 Priority 的 enum class 类型和相关状态。 |
| [L107](../src/webrtc/IWebRtcSession.h#L107) | `DataChannelInfo` | struct | 定义 DataChannelInfo 的 struct 类型和相关状态。 |
| [L116](../src/webrtc/IWebRtcSession.h#L116) | `OperationError` | struct | 定义 OperationError 的 struct 类型和相关状态。 |
| [L121](../src/webrtc/IWebRtcSession.h#L121) | `RemoteTrackInfo` | struct | 定义 RemoteTrackInfo 的 struct 类型和相关状态。 |
| [L127](../src/webrtc/IWebRtcSession.h#L127) | `IWebRtcSessionObserver` | class | 定义 IWebRtcSessionObserver 的 class 类型和相关状态。 |
| [L153](../src/webrtc/IWebRtcSession.h#L153) | `IWebRtcSession` | class | 定义 IWebRtcSession 的 class 类型和相关状态。 |

### 成员与文件级变量

| 行 | 变量 | 声明 | 作用 |
|---:|---|---|---|
| [L18](../src/webrtc/IWebRtcSession.h#L18) | `kScreenMainVideoSlot` | `inline constexpr char kScreenMainVideoSlot[] = "screen-main";` | 定义 screen main video slot 的编译期常量或产品边界。 |
| [L19](../src/webrtc/IWebRtcSession.h#L19) | `kCameraMainVideoSlot` | `inline constexpr char kCameraMainVideoSlot[] = "camera-main";` | 定义 camera main video slot 的编译期常量或产品边界。 |
| [L20](../src/webrtc/IWebRtcSession.h#L20) | `kMicrophoneMainAudioSlot` | `inline constexpr char kMicrophoneMainAudioSlot[] = "microphone-main";` | 定义 microphone main audio slot 的编译期常量或产品边界。 |
| [L21](../src/webrtc/IWebRtcSession.h#L21) | `kDefaultIceMinPort` | `inline constexpr int kDefaultIceMinPort = 50000;` | 定义 default ice min port 的编译期常量或产品边界。 |
| [L22](../src/webrtc/IWebRtcSession.h#L22) | `kDefaultIceMaxPort` | `inline constexpr int kDefaultIceMaxPort = 50020;` | 定义 default ice max port 的编译期常量或产品边界。 |
| [L63](../src/webrtc/IWebRtcSession.h#L63) | `type` | `SessionDescriptionType type = SessionDescriptionType::kOffer;` | 保存 type 相关配置或运行状态。 |
| [L64](../src/webrtc/IWebRtcSession.h#L64) | `sdp` | `std::string sdp;` | 保存 sdp 相关配置或运行状态。 |
| [L67](../src/webrtc/IWebRtcSession.h#L67) | `negotiationGeneration` | `std::uint64_t negotiationGeneration = 1;` | Monotonically increasing per PeerConnection. Generation 1 is the initial negotiation; later generations are ICE restarts. |
| [L71](../src/webrtc/IWebRtcSession.h#L71) | `sdpMid` | `std::string sdpMid;` | 保存身份或作用域标识：sdp mid。 |
| [L72](../src/webrtc/IWebRtcSession.h#L72) | `sdpMLineIndex` | `int sdpMLineIndex = 0;` | 保存 sdp m line index 相关配置或运行状态。 |
| [L73](../src/webrtc/IWebRtcSession.h#L73) | `candidate` | `std::string candidate;` | 保存 candidate 相关配置或运行状态。 |
| [L74](../src/webrtc/IWebRtcSession.h#L74) | `negotiationGeneration` | `std::uint64_t negotiationGeneration = 1;` | 标记当前世代，用于拒绝过期异步结果：negotiation generation。 |
| [L78](../src/webrtc/IWebRtcSession.h#L78) | `urls` | `std::vector<std::string> urls;` | 保存 urls 相关配置或运行状态。 |
| [L79](../src/webrtc/IWebRtcSession.h#L79) | `username` | `std::string username;` | 保存路径、地址或显示名称：username。 |
| [L80](../src/webrtc/IWebRtcSession.h#L80) | `password` | `std::string password;` | 保存 password 相关配置或运行状态。 |
| [L84](../src/webrtc/IWebRtcSession.h#L84) | `iceServers` | `std::vector<IceServerConfig> iceServers;` | 保存 ice servers 相关配置或运行状态。 |
| [L85](../src/webrtc/IWebRtcSession.h#L85) | `includeLoopbackAdapter` | `bool includeLoopbackAdapter = false;` | 保存 include loopback adapter 相关配置或运行状态。 |
| [L86](../src/webrtc/IWebRtcSession.h#L86) | `fastDesktopBweStartup` | `bool fastDesktopBweStartup = false;` | 保存 fast desktop bwe startup 相关配置或运行状态。 |
| [L87](../src/webrtc/IWebRtcSession.h#L87) | `adaptiveDesktopNetworkFrameRate` | `bool adaptiveDesktopNetworkFrameRate = false;` | 保存计数、尺寸或速率指标：adaptive desktop network frame rate。 |
| [L88](../src/webrtc/IWebRtcSession.h#L88) | `iceMinPort` | `int iceMinPort = 0;` | 保存 ice min port 相关配置或运行状态。 |
| [L89](../src/webrtc/IWebRtcSession.h#L89) | `iceMaxPort` | `int iceMaxPort = 0;` | 保存 ice max port 相关配置或运行状态。 |
| [L99](../src/webrtc/IWebRtcSession.h#L99) | `label` | `std::string label;` | 保存路径、地址或显示名称：label。 |
| [L100](../src/webrtc/IWebRtcSession.h#L100) | `ordered` | `bool ordered = true;` | 保存 ordered 相关配置或运行状态。 |
| [L101](../src/webrtc/IWebRtcSession.h#L101) | `maxRetransmits` | `std::optional<int> maxRetransmits;` | 保存 max retransmits 相关配置或运行状态。 |
| [L102](../src/webrtc/IWebRtcSession.h#L102) | `maxPacketLifeTimeMs` | `std::optional<int> maxPacketLifeTimeMs;` | 保存 max packet life time ms 相关配置或运行状态。 |
| [L103](../src/webrtc/IWebRtcSession.h#L103) | `protocol` | `std::string protocol;` | 保存 protocol 相关配置或运行状态。 |
| [L104](../src/webrtc/IWebRtcSession.h#L104) | `priority` | `std::optional<Priority> priority;` | 保存 priority 相关配置或运行状态。 |
| [L108](../src/webrtc/IWebRtcSession.h#L108) | `label` | `std::string label;` | 保存路径、地址或显示名称：label。 |
| [L109](../src/webrtc/IWebRtcSession.h#L109) | `state` | `DataChannelState state = DataChannelState::kConnecting;` | 保存状态机当前状态：state。 |
| [L110](../src/webrtc/IWebRtcSession.h#L110) | `ordered` | `bool ordered = true;` | 保存 ordered 相关配置或运行状态。 |
| [L111](../src/webrtc/IWebRtcSession.h#L111) | `maxRetransmits` | `std::optional<int> maxRetransmits;` | 保存 max retransmits 相关配置或运行状态。 |
| [L112](../src/webrtc/IWebRtcSession.h#L112) | `maxPacketLifeTimeMs` | `std::optional<int> maxPacketLifeTimeMs;` | 保存 max packet life time ms 相关配置或运行状态。 |
| [L113](../src/webrtc/IWebRtcSession.h#L113) | `protocol` | `std::string protocol;` | 保存 protocol 相关配置或运行状态。 |
| [L117](../src/webrtc/IWebRtcSession.h#L117) | `code` | `std::string code;` | 保存 code 相关配置或运行状态。 |
| [L118](../src/webrtc/IWebRtcSession.h#L118) | `message` | `std::string message;` | 保存 message 相关配置或运行状态。 |
| [L122](../src/webrtc/IWebRtcSession.h#L122) | `id` | `std::string id;` | 保存身份或作用域标识：id。 |
| [L123](../src/webrtc/IWebRtcSession.h#L123) | `kind` | `std::string kind;` | 保存 kind 相关配置或运行状态。 |
| [L124](../src/webrtc/IWebRtcSession.h#L124) | `slot` | `std::string slot;` | 保存 slot 相关配置或运行状态。 |

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L129](../src/webrtc/IWebRtcSession.h#L129) | `~IWebRtcSessionObserver` | 声明 | `virtual ~IWebRtcSessionObserver() = default` | 停止相关活动并释放 IWebRtcSessionObserver 实例拥有的资源。 |
| [L136](../src/webrtc/IWebRtcSession.h#L136) | `OnSessionStateChanged` | 声明 | `virtual void OnSessionStateChanged(WebRtcSessionState state) = 0` | outlive the session or be cleared with SetObserver(nullptr). The payload span is valid only for the duration of OnDataMessage. Handlers must not call WebRTC session methods re-e... |
| [L137](../src/webrtc/IWebRtcSession.h#L137) | `OnIceGatheringStateChanged` | 声明 | `virtual void OnIceGatheringStateChanged( WebRtcIceGatheringState state) = 0` | 接收并处理 on ice gathering state changed 相关逻辑。 |
| [L139](../src/webrtc/IWebRtcSession.h#L139) | `OnLocalDescription` | 声明 | `virtual void OnLocalDescription( const SessionDescription& description) = 0` | 接收并处理 on local description 相关逻辑。 |
| [L141](../src/webrtc/IWebRtcSession.h#L141) | `OnLocalIceCandidate` | 声明 | `virtual void OnLocalIceCandidate(const IceCandidate& candidate) = 0` | 接收并处理 on local ice candidate 相关逻辑。 |
| [L142](../src/webrtc/IWebRtcSession.h#L142) | `OnDataChannelStateChanged` | 声明 | `virtual void OnDataChannelStateChanged( const DataChannelInfo& channel) = 0` | 接收并处理 on data channel state changed 相关逻辑。 |
| [L144](../src/webrtc/IWebRtcSession.h#L144) | `OnDataMessage` | 声明 | `virtual void OnDataMessage(const std::string& label, std::span<const std::uint8_t> payload, bool binary) = 0` | 接收并处理 on data message 相关逻辑。 |
| [L147](../src/webrtc/IWebRtcSession.h#L147) | `OnRemoteTrackAdded` | 声明 | `virtual void OnRemoteTrackAdded(const RemoteTrackInfo& track) = 0` | 接收并处理 on remote track added 相关逻辑。 |
| [L148](../src/webrtc/IWebRtcSession.h#L148) | `OnOperationCompleted` | 声明 | `virtual void OnOperationCompleted(OperationId operationId) = 0` | 接收并处理 on operation completed 相关逻辑。 |
| [L149](../src/webrtc/IWebRtcSession.h#L149) | `OnWebRtcError` | 声明 | `virtual void OnWebRtcError(OperationId operationId, const OperationError& error) = 0` | 接收并处理 on web rtc error 相关逻辑。 |
| [L155](../src/webrtc/IWebRtcSession.h#L155) | `~IWebRtcSession` | 声明 | `virtual ~IWebRtcSession() = default` | 停止相关活动并释放 IWebRtcSession 实例拥有的资源。 |
| [L157](../src/webrtc/IWebRtcSession.h#L157) | `SetObserver` | 声明 | `virtual void SetObserver(IWebRtcSessionObserver* observer) = 0` | 更新或应用 set observer 相关逻辑。 |
| [L158](../src/webrtc/IWebRtcSession.h#L158) | `Start` | 声明 | `virtual OperationId Start(const WebRtcSessionConfig& config) = 0` | 启动 start 相关逻辑。 |
| [L159](../src/webrtc/IWebRtcSession.h#L159) | `CreateOffer` | 声明 | `virtual OperationId CreateOffer() = 0` | 创建或初始化 create offer 相关逻辑。 |
| [L160](../src/webrtc/IWebRtcSession.h#L160) | `CreateIceRestartOffer` | 声明 | `virtual OperationId CreateIceRestartOffer() = 0` | 创建或初始化 create ice restart offer 相关逻辑。 |
| [L161](../src/webrtc/IWebRtcSession.h#L161) | `CreateAnswer` | 声明 | `virtual OperationId CreateAnswer() = 0` | 创建或初始化 create answer 相关逻辑。 |
| [L162](../src/webrtc/IWebRtcSession.h#L162) | `ApplyRemoteDescription` | 声明 | `virtual OperationId ApplyRemoteDescription( const SessionDescription& description) = 0` | 更新或应用 apply remote description 相关逻辑。 |
| [L164](../src/webrtc/IWebRtcSession.h#L164) | `AddRemoteIceCandidate` | 声明 | `virtual OperationId AddRemoteIceCandidate( const IceCandidate& candidate) = 0` | 实现 add remote ice candidate 对应的业务或工具逻辑。 |
| [L166](../src/webrtc/IWebRtcSession.h#L166) | `CreateDataChannels` | 声明 | `virtual OperationId CreateDataChannels( const std::vector<DataChannelSpec>& channels) = 0` | 创建或初始化 create data channels 相关逻辑。 |
| [L168](../src/webrtc/IWebRtcSession.h#L168) | `SendData` | 声明 | `virtual SendResult SendData(const std::string& channelName, std::span<const std::uint8_t> data, bool binary) = 0` | 发送或发布 send data 相关逻辑。 |
| [L171](../src/webrtc/IWebRtcSession.h#L171) | `DataChannelBufferedAmount` | 声明 | `virtual std::optional<std::uint64_t> DataChannelBufferedAmount( const std::string& channelName) const = 0` | 实现 data channel buffered amount 对应的业务或工具逻辑。 |
| [L173](../src/webrtc/IWebRtcSession.h#L173) | `RequestStats` | 定义 | `virtual void RequestStats() {}` | 发起请求或查询 request stats 相关逻辑。 |
| [L174](../src/webrtc/IWebRtcSession.h#L174) | `StatsSnapshot` | 定义 | `virtual WebRtcSessionStatsSnapshot StatsSnapshot() const { return {}; }` | 实现 stats snapshot 对应的业务或工具逻辑。 |
| [L175](../src/webrtc/IWebRtcSession.h#L175) | `Close` | 声明 | `virtual void Close() = 0` | 关闭并清理 close 相关逻辑。 |

## `src/webrtc/LibWebRtcSession.AudioSlot.inc`

[打开源码](../src/webrtc/LibWebRtcSession.AudioSlot.inc) · **文件作用：** `LibWebRtcSession` 的实现切片，集中实现 audio slot 相关逻辑；成员状态仍定义在所属头文件中。

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L4](../src/webrtc/LibWebRtcSession.AudioSlot.inc#L4) | `LibWebRtcSession::PrepareAudioTransceiverSlot` | 定义 | `webrtc::RTCError LibWebRtcSession::PrepareAudioTransceiverSlot( const std::string& slot)` | SPDX-License-Identifier: GPL-3.0-only Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd) |
| [L45](../src/webrtc/LibWebRtcSession.AudioSlot.inc#L45) | `LibWebRtcSession::BindNegotiatedAudioTransceiverSlot` | 定义 | `webrtc::RTCError LibWebRtcSession::BindNegotiatedAudioTransceiverSlot( const std::string& slot)` | 实现 bind negotiated audio transceiver slot 对应的业务或工具逻辑。 |
| [L85](../src/webrtc/LibWebRtcSession.AudioSlot.inc#L85) | `LibWebRtcSession::SetAudioSlotTrack` | 定义 | `webrtc::RTCError LibWebRtcSession::SetAudioSlotTrack( const std::string& slot, webrtc::scoped_refptr<webrtc::MediaStreamTrackInterface> track)` | 更新或应用 set audio slot track 相关逻辑。 |
| [L123](../src/webrtc/LibWebRtcSession.AudioSlot.inc#L123) | `LibWebRtcSession::SetRemoteAudioSlotEnabled` | 定义 | `void LibWebRtcSession::SetRemoteAudioSlotEnabled( const std::string& slot, bool enabled)` | 更新或应用 set remote audio slot enabled 相关逻辑。 |
| [L140](../src/webrtc/LibWebRtcSession.AudioSlot.inc#L140) | `LibWebRtcSession::AudioSlotPrepared` | 定义 | `bool LibWebRtcSession::AudioSlotPrepared(const std::string& slot) const` | 实现 audio slot prepared 对应的业务或工具逻辑。 |
| [L146](../src/webrtc/LibWebRtcSession.AudioSlot.inc#L146) | `LibWebRtcSession::PreparedVideoSlotCount` | 定义 | `std::size_t LibWebRtcSession::PreparedVideoSlotCount() const` | 实现 prepared video slot count 对应的业务或工具逻辑。 |
| [L152](../src/webrtc/LibWebRtcSession.AudioSlot.inc#L152) | `LibWebRtcSession::SetRemoteVideoSink` | 定义 | `void LibWebRtcSession::SetRemoteVideoSink( webrtc::VideoSinkInterface<webrtc::VideoFrame>* sink)` | 更新或应用 set remote video sink 相关逻辑。 |
| [L178](../src/webrtc/LibWebRtcSession.AudioSlot.inc#L178) | `LibWebRtcSession::SetRemoteVideoSlotSink` | 定义 | `void LibWebRtcSession::SetRemoteVideoSlotSink( const std::string& slot, webrtc::VideoSinkInterface<webrtc::VideoFrame>* sink)` | 更新或应用 set remote video slot sink 相关逻辑。 |

## `src/webrtc/LibWebRtcSession.cpp`

[打开源码](../src/webrtc/LibWebRtcSession.cpp) · **文件作用：** 实现 lib web rtc session 相关函数与文件级辅助逻辑。

### 类型

| 行 | 类型 | 种类 | 作用 |
|---:|---|---|---|
| [L81](../src/webrtc/LibWebRtcSession.cpp#L81) | `CreateDescriptionCallback` | class | 定义 CreateDescriptionCallback 的 class 类型和相关状态。 |
| [L109](../src/webrtc/LibWebRtcSession.cpp#L109) | `SetDescriptionCallback` | class | 定义 SetDescriptionCallback 的 class 类型和相关状态。 |

### 成员与文件级变量

| 行 | 变量 | 声明 | 作用 |
|---:|---|---|---|
| [L26](../src/webrtc/LibWebRtcSession.cpp#L26) | `kMaximumScreenBitrateBps` | `constexpr int kMaximumScreenBitrateBps = 100'000'000;` | 定义 maximum screen bitrate bps 的编译期常量或产品边界。 |
| [L27](../src/webrtc/LibWebRtcSession.cpp#L27) | `kDesktopStartupProbeFloorBps` | `constexpr int kDesktopStartupProbeFloorBps = 2'000'000;` | 定义 desktop startup probe floor bps 的编译期常量或产品边界。 |
| [L28](../src/webrtc/LibWebRtcSession.cpp#L28) | `kDefaultWebRtcMinimumBitrateBps` | `constexpr int kDefaultWebRtcMinimumBitrateBps = 30'000;` | 定义 default web rtc minimum bitrate bps 的编译期常量或产品边界。 |

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L30](../src/webrtc/LibWebRtcSession.cpp#L30) | `SteadyNowMs` | 定义 | `std::uint64_t SteadyNowMs()` | 实现 steady now ms 对应的业务或工具逻辑。 |
| [L37](../src/webrtc/LibWebRtcSession.cpp#L37) | `ProgressiveBitrateCeilingStatusName` | 定义 | `const char* ProgressiveBitrateCeilingStatusName( ProgressiveBitrateCeilingStatus status)` | 实现 progressive bitrate ceiling status name 对应的业务或工具逻辑。 |
| [L57](../src/webrtc/LibWebRtcSession.cpp#L57) | `AdaptiveScreenFrameRateStatusName` | 定义 | `const char* AdaptiveScreenFrameRateStatusName( AdaptiveScreenFrameRateStatus status)` | 实现 adaptive screen frame rate status name 对应的业务或工具逻辑。 |
| [L88](../src/webrtc/LibWebRtcSession.cpp#L88) | `CreateDescriptionCallback` | 定义 | `CreateDescriptionCallback(SuccessCallback success, FailureCallback failure) : success_(std::move(success)), failure_(std::move(failure)) {}` | 创建或初始化 create description callback 相关逻辑。 |
| [L93](../src/webrtc/LibWebRtcSession.cpp#L93) | `OnSuccess` | 定义 | `void OnSuccess(webrtc::SessionDescriptionInterface* description) override` | 接收并处理 on success 相关逻辑。 |
| [L99](../src/webrtc/LibWebRtcSession.cpp#L99) | `OnFailure` | 定义 | `void OnFailure(webrtc::RTCError error) override` | 接收并处理 on failure 相关逻辑。 |
| [L115](../src/webrtc/LibWebRtcSession.cpp#L115) | `SetDescriptionCallback` | 定义 | `SetDescriptionCallback(SuccessCallback success, FailureCallback failure) : success_(std::move(success)), failure_(std::move(failure)) {}` | 更新或应用 set description callback 相关逻辑。 |
| [L119](../src/webrtc/LibWebRtcSession.cpp#L119) | `OnSuccess` | 定义 | `void OnSuccess() override { success_(); }` | 接收并处理 on success 相关逻辑。 |
| [L121](../src/webrtc/LibWebRtcSession.cpp#L121) | `OnFailure` | 定义 | `void OnFailure(webrtc::RTCError error) override` | 接收并处理 on failure 相关逻辑。 |
| [L131](../src/webrtc/LibWebRtcSession.cpp#L131) | `ToNativeSdpType` | 定义 | `webrtc::SdpType ToNativeSdpType(SessionDescriptionType type)` | 实现 to native sdp type 对应的业务或工具逻辑。 |
| [L137](../src/webrtc/LibWebRtcSession.cpp#L137) | `ToPublicDataChannelState` | 定义 | `DataChannelState ToPublicDataChannelState( webrtc::DataChannelInterface::DataState state)` | 实现 to public data channel state 对应的业务或工具逻辑。 |
| [L153](../src/webrtc/LibWebRtcSession.cpp#L153) | `ToPublicIceGatheringState` | 定义 | `WebRtcIceGatheringState ToPublicIceGatheringState( webrtc::PeerConnectionInterface::IceGatheringState state)` | 实现 to public ice gathering state 对应的业务或工具逻辑。 |
| [L167](../src/webrtc/LibWebRtcSession.cpp#L167) | `EqualsIgnoreCase` | 定义 | `bool EqualsIgnoreCase(const std::string& left, const char* right)` | 判断 equals ignore case 相关逻辑。 |
| [L182](../src/webrtc/LibWebRtcSession.cpp#L182) | `H264CodecPreferences` | 定义 | `std::vector<webrtc::RtpCodecCapability> H264CodecPreferences( webrtc::PeerConnectionFactoryInterface* factory)` | 实现 h264 codec preferences 对应的业务或工具逻辑。 |
| [L208](../src/webrtc/LibWebRtcSession.cpp#L208) | `LibWebRtcSession::DetachRemoteVideoSink` | 定义 | `void LibWebRtcSession::DetachRemoteVideoSink()` | 实现 detach remote video sink 对应的业务或工具逻辑。 |

## `src/webrtc/LibWebRtcSession.DataChannelCallbacks.inc`

[打开源码](../src/webrtc/LibWebRtcSession.DataChannelCallbacks.inc) · **文件作用：** `LibWebRtcSession` 的实现切片，集中实现 data channel callbacks 相关逻辑；成员状态仍定义在所属头文件中。

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L4](../src/webrtc/LibWebRtcSession.DataChannelCallbacks.inc#L4) | `LibWebRtcSession::AttachDataChannel` | 定义 | `void LibWebRtcSession::AttachDataChannel( webrtc::scoped_refptr<webrtc::DataChannelInterface> channel)` | SPDX-License-Identifier: GPL-3.0-only Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd) |
| [L27](../src/webrtc/LibWebRtcSession.DataChannelCallbacks.inc#L27) | `LibWebRtcSession::HandleDataChannelState` | 定义 | `void LibWebRtcSession::HandleDataChannelState( webrtc::scoped_refptr<webrtc::DataChannelInterface> channel)` | 接收并处理 handle data channel state 相关逻辑。 |
| [L45](../src/webrtc/LibWebRtcSession.DataChannelCallbacks.inc#L45) | `LibWebRtcSession::HandleDataMessage` | 定义 | `void LibWebRtcSession::HandleDataMessage( const std::string& label, const webrtc::DataBuffer& buffer)` | 接收并处理 handle data message 相关逻辑。 |

## `src/webrtc/LibWebRtcSession.DataChannels.inc`

[打开源码](../src/webrtc/LibWebRtcSession.DataChannels.inc) · **文件作用：** `LibWebRtcSession` 的实现切片，集中实现 data channels 相关逻辑；成员状态仍定义在所属头文件中。

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L4](../src/webrtc/LibWebRtcSession.DataChannels.inc#L4) | `LibWebRtcSession::CreateDataChannels` | 定义 | `OperationId LibWebRtcSession::CreateDataChannels( const std::vector<DataChannelSpec>& channels)` | SPDX-License-Identifier: GPL-3.0-only Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd) |
| [L72](../src/webrtc/LibWebRtcSession.DataChannels.inc#L72) | `LibWebRtcSession::SendData` | 定义 | `SendResult LibWebRtcSession::SendData(const std::string& channelName, std::span<const std::uint8_t> data, bool binary)` | 发送或发布 send data 相关逻辑。 |
| [L100](../src/webrtc/LibWebRtcSession.DataChannels.inc#L100) | `LibWebRtcSession::DataChannelBufferedAmount` | 定义 | `LibWebRtcSession::DataChannelBufferedAmount( const std::string& channelName) const` | 实现 data channel buffered amount 对应的业务或工具逻辑。 |

## `src/webrtc/LibWebRtcSession.h`

[打开源码](../src/webrtc/LibWebRtcSession.h) · **文件作用：** 声明 lib web rtc session 相关类型、接口、配置和成员状态。

### 类型

| 行 | 类型 | 种类 | 作用 |
|---:|---|---|---|
| [L26](../src/webrtc/LibWebRtcSession.h#L26) | `PeerConnectionStatsCollector` | class | 定义 PeerConnectionStatsCollector 的 class 类型和相关状态。 |
| [L28](../src/webrtc/LibWebRtcSession.h#L28) | `LibWebRtcSession` | class | 定义 LibWebRtcSession 的 class 类型和相关状态。 |
| [L120](../src/webrtc/LibWebRtcSession.h#L120) | `CallbackGate` | class | 定义 CallbackGate 的 class 类型和相关状态。 |
| [L121](../src/webrtc/LibWebRtcSession.h#L121) | `DataChannelBinding` | class | 定义 DataChannelBinding 的 class 类型和相关状态。 |
| [L122](../src/webrtc/LibWebRtcSession.h#L122) | `VideoSlotBinding` | struct | 定义 VideoSlotBinding 的 struct 类型和相关状态。 |
| [L144](../src/webrtc/LibWebRtcSession.h#L144) | `AudioSlotBinding` | struct | 定义 AudioSlotBinding 的 struct 类型和相关状态。 |

### 成员与文件级变量

| 行 | 变量 | 声明 | 作用 |
|---:|---|---|---|
| [L26](../src/webrtc/LibWebRtcSession.h#L26) | `PeerConnectionStatsCollector` | `class PeerConnectionStatsCollector;` | 保存 peer connection stats collector 相关配置或运行状态。 |
| [L117](../src/webrtc/LibWebRtcSession.h#L117) | `override` | `override;` | 保存 override 相关配置或运行状态。 |
| [L120](../src/webrtc/LibWebRtcSession.h#L120) | `CallbackGate` | `class CallbackGate;` | 保存 callback gate 相关配置或运行状态。 |
| [L121](../src/webrtc/LibWebRtcSession.h#L121) | `DataChannelBinding` | `class DataChannelBinding;` | 保存 data channel binding 相关配置或运行状态。 |
| [L123](../src/webrtc/LibWebRtcSession.h#L123) | `transceiver` | `webrtc::scoped_refptr<webrtc::RtpTransceiverInterface> transceiver;` | 保存 transceiver 相关配置或运行状态。 |
| [L124](../src/webrtc/LibWebRtcSession.h#L124) | `remoteTrack` | `webrtc::scoped_refptr<webrtc::VideoTrackInterface> remoteTrack;` | 保存 remote track 相关配置或运行状态。 |
| [L125](../src/webrtc/LibWebRtcSession.h#L125) | `remoteSink` | `webrtc::VideoSinkInterface<webrtc::VideoFrame>* remoteSink = nullptr;` | 保存回调或观察者入口：remote sink。 |
| [L126](../src/webrtc/LibWebRtcSession.h#L126) | `configuredMaxFrameRate` | `std::uint32_t configuredMaxFrameRate = 0;` | 保存计数、尺寸或速率指标：configured max frame rate。 |
| [L127](../src/webrtc/LibWebRtcSession.h#L127) | `configuredOutputWidth` | `std::uint32_t configuredOutputWidth = 0;` | 保存计数、尺寸或速率指标：configured output width。 |
| [L128](../src/webrtc/LibWebRtcSession.h#L128) | `configuredOutputHeight` | `std::uint32_t configuredOutputHeight = 0;` | 保存计数、尺寸或速率指标：configured output height。 |
| [L129](../src/webrtc/LibWebRtcSession.h#L129) | `configuredStartBitrateBps` | `std::uint64_t configuredStartBitrateBps = 0;` | 保存计数、尺寸或速率指标：configured start bitrate bps。 |
| [L130](../src/webrtc/LibWebRtcSession.h#L130) | `configuredMaxBitrateBps` | `std::uint64_t configuredMaxBitrateBps = 0;` | 保存计数、尺寸或速率指标：configured max bitrate bps。 |
| [L131](../src/webrtc/LibWebRtcSession.h#L131) | `adaptiveFrameRate` | `AdaptiveScreenFrameRateState adaptiveFrameRate;` | 保存计数、尺寸或速率指标：adaptive frame rate。 |
| [L132](../src/webrtc/LibWebRtcSession.h#L132) | `adaptiveFrameRateRevision` | `std::uint64_t adaptiveFrameRateRevision = 0;` | 标记当前世代，用于拒绝过期异步结果：adaptive frame rate revision。 |
| [L133](../src/webrtc/LibWebRtcSession.h#L133) | `adaptiveFrameRateError` | `std::string adaptiveFrameRateError;` | 保存最近错误或失败原因：adaptive frame rate error。 |
| [L134](../src/webrtc/LibWebRtcSession.h#L134) | `sendingActive` | `bool sendingActive = false;` | 保存能力或开关状态：sending active。 |
| [L135](../src/webrtc/LibWebRtcSession.h#L135) | `startBitrateBootstrapPending` | `bool startBitrateBootstrapPending = true;` | 保存待处理队列或请求：start bitrate bootstrap pending。 |
| [L136](../src/webrtc/LibWebRtcSession.h#L136) | `bitrateBootstrapAttempts` | `std::uint32_t bitrateBootstrapAttempts = 0;` | 保存 bitrate bootstrap attempts 相关配置或运行状态。 |
| [L137](../src/webrtc/LibWebRtcSession.h#L137) | `bitrateBootstrapSuccesses` | `std::uint32_t bitrateBootstrapSuccesses = 0;` | 保存 bitrate bootstrap successes 相关配置或运行状态。 |
| [L138](../src/webrtc/LibWebRtcSession.h#L138) | `mediaReadyBitrateRestarts` | `std::uint32_t mediaReadyBitrateRestarts = 0;` | 保存 media ready bitrate restarts 相关配置或运行状态。 |
| [L139](../src/webrtc/LibWebRtcSession.h#L139) | `allocationProbePulses` | `std::uint32_t allocationProbePulses = 0;` | 保存 allocation probe pulses 相关配置或运行状态。 |
| [L140](../src/webrtc/LibWebRtcSession.h#L140) | `bitrateProbeFloorReleases` | `std::uint32_t bitrateProbeFloorReleases = 0;` | 保存 bitrate probe floor releases 相关配置或运行状态。 |
| [L141](../src/webrtc/LibWebRtcSession.h#L141) | `bitrateProbeFloorActive` | `bool bitrateProbeFloorActive = false;` | 保存能力或开关状态：bitrate probe floor active。 |
| [L142](../src/webrtc/LibWebRtcSession.h#L142) | `bitrateBootstrapError` | `std::string bitrateBootstrapError;` | 保存最近错误或失败原因：bitrate bootstrap error。 |
| [L145](../src/webrtc/LibWebRtcSession.h#L145) | `name` | `std::string name;` | 保存路径、地址或显示名称：name。 |
| [L146](../src/webrtc/LibWebRtcSession.h#L146) | `transceiver` | `webrtc::scoped_refptr<webrtc::RtpTransceiverInterface> transceiver;` | 保存 transceiver 相关配置或运行状态。 |
| [L147](../src/webrtc/LibWebRtcSession.h#L147) | `remoteTrack` | `webrtc::scoped_refptr<webrtc::AudioTrackInterface> remoteTrack;` | 保存 remote track 相关配置或运行状态。 |
| [L148](../src/webrtc/LibWebRtcSession.h#L148) | `remotePlaybackEnabled` | `bool remotePlaybackEnabled = true;` | 保存能力或开关状态：remote playback enabled。 |
| [L196](../src/webrtc/LibWebRtcSession.h#L196) | `mutex_` | `mutable std::mutex mutex_;` | 保护跨线程共享状态：mutex。 |
| [L200](../src/webrtc/LibWebRtcSession.h#L200) | `videoSenderParametersMutex_` | `std::mutex videoSenderParametersMutex_;` | Serializes read-modify-write updates to RtpSender parameters. User preference changes, share activation and stats-driven per-viewer FPS adaptation may arrive on different threads. |
| [L201](../src/webrtc/LibWebRtcSession.h#L201) | `nextOperationId_` | `std::atomic<OperationId> nextOperationId_{1};` | 保存身份或作用域标识：next operation id。 |
| [L202](../src/webrtc/LibWebRtcSession.h#L202) | `callbackGate_` | `std::shared_ptr<CallbackGate> callbackGate_;` | 保存 callback gate 相关配置或运行状态。 |
| [L203](../src/webrtc/LibWebRtcSession.h#L203) | `factory_` | `webrtc::scoped_refptr<webrtc::PeerConnectionFactoryInterface> factory_;` | 保存 factory 相关配置或运行状态。 |
| [L204](../src/webrtc/LibWebRtcSession.h#L204) | `statsCollector_` | `std::unique_ptr<PeerConnectionStatsCollector> statsCollector_;` | 保存 stats collector 相关配置或运行状态。 |
| [L205](../src/webrtc/LibWebRtcSession.h#L205) | `peerConnection_` | `webrtc::scoped_refptr<webrtc::PeerConnectionInterface> peerConnection_;` | 保存 peer connection 相关配置或运行状态。 |
| [L206](../src/webrtc/LibWebRtcSession.h#L206) | `observer_` | `IWebRtcSessionObserver* observer_ = nullptr;` | 保存回调或观察者入口：observer。 |
| [L207](../src/webrtc/LibWebRtcSession.h#L207) | `fastDesktopBweStartup_` | `bool fastDesktopBweStartup_ = false;` | 保存 fast desktop bwe startup 相关配置或运行状态。 |
| [L208](../src/webrtc/LibWebRtcSession.h#L208) | `adaptiveDesktopNetworkFrameRate_` | `bool adaptiveDesktopNetworkFrameRate_ = false;` | 保存计数、尺寸或速率指标：adaptive desktop network frame rate。 |
| [L209](../src/webrtc/LibWebRtcSession.h#L209) | `progressiveBitrateCeiling_` | `ProgressiveBitrateCeilingState progressiveBitrateCeiling_;` | 保存 progressive bitrate ceiling 相关配置或运行状态。 |
| [L210](../src/webrtc/LibWebRtcSession.h#L210) | `progressiveBitrateCeilingRevision_` | `std::uint64_t progressiveBitrateCeilingRevision_ = 0;` | 标记当前世代，用于拒绝过期异步结果：progressive bitrate ceiling revision。 |
| [L211](../src/webrtc/LibWebRtcSession.h#L211) | `progressiveBitrateCeilingError_` | `std::string progressiveBitrateCeilingError_;` | 保存最近错误或失败原因：progressive bitrate ceiling error。 |
| [L213](../src/webrtc/LibWebRtcSession.h#L213) | `kUnknown` | `ScreenContentActivity::kUnknown;` | 定义 unknown 的编译期常量或产品边界。 |
| [L214](../src/webrtc/LibWebRtcSession.h#L214) | `state_` | `WebRtcSessionState state_ = WebRtcSessionState::kNew;` | 保存状态机当前状态：state。 |
| [L217](../src/webrtc/LibWebRtcSession.h#L217) | `kNew` | `webrtc::PeerConnectionInterface::PeerConnectionState::kNew;` | 定义 new 的编译期常量或产品边界。 |
| [L219](../src/webrtc/LibWebRtcSession.h#L219) | `kIceConnectionNew` | `webrtc::PeerConnectionInterface::kIceConnectionNew;` | 定义 ice connection new 的编译期常量或产品边界。 |
| [L221](../src/webrtc/LibWebRtcSession.h#L221) | `dataChannels_` | `dataChannels_;` | 保存 data channels 相关配置或运行状态。 |
| [L222](../src/webrtc/LibWebRtcSession.h#L222) | `videoSlots_` | `std::unordered_map<std::string, VideoSlotBinding> videoSlots_;` | 保存 video slots 相关配置或运行状态。 |
| [L223](../src/webrtc/LibWebRtcSession.h#L223) | `videoSlotOrder_` | `std::vector<std::string> videoSlotOrder_;` | 保存 video slot order 相关配置或运行状态。 |
| [L224](../src/webrtc/LibWebRtcSession.h#L224) | `audioSlot_` | `AudioSlotBinding audioSlot_;` | 保存 audio slot 相关配置或运行状态。 |
| [L225](../src/webrtc/LibWebRtcSession.h#L225) | `remoteVideoSink_` | `webrtc::VideoSinkInterface<webrtc::VideoFrame>* remoteVideoSink_ = nullptr;` | 保存回调或观察者入口：remote video sink。 |
| [L226](../src/webrtc/LibWebRtcSession.h#L226) | `remoteVideoTrack_` | `webrtc::scoped_refptr<webrtc::VideoTrackInterface> remoteVideoTrack_;` | 保存 remote video track 相关配置或运行状态。 |

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L31](../src/webrtc/LibWebRtcSession.h#L31) | `LibWebRtcSession` | 声明 | `explicit LibWebRtcSession( webrtc::scoped_refptr<webrtc::PeerConnectionFactoryInterface> factory)` | 实现 lib web rtc session 对应的业务或工具逻辑。 |
| [L33](../src/webrtc/LibWebRtcSession.h#L33) | `~LibWebRtcSession` | 声明 | `~LibWebRtcSession() override` | 停止相关活动并释放 LibWebRtcSession 实例拥有的资源。 |
| [L35](../src/webrtc/LibWebRtcSession.h#L35) | `LibWebRtcSession` | 声明 | `LibWebRtcSession(const LibWebRtcSession&) = delete` | 实现 lib web rtc session 对应的业务或工具逻辑。 |
| [L38](../src/webrtc/LibWebRtcSession.h#L38) | `SetObserver` | 声明 | `void SetObserver(IWebRtcSessionObserver* observer) override` | 更新或应用 set observer 相关逻辑。 |
| [L39](../src/webrtc/LibWebRtcSession.h#L39) | `Start` | 声明 | `OperationId Start(const WebRtcSessionConfig& config) override` | 启动 start 相关逻辑。 |
| [L40](../src/webrtc/LibWebRtcSession.h#L40) | `CreateOffer` | 声明 | `OperationId CreateOffer() override` | 创建或初始化 create offer 相关逻辑。 |
| [L41](../src/webrtc/LibWebRtcSession.h#L41) | `CreateIceRestartOffer` | 声明 | `OperationId CreateIceRestartOffer() override` | 创建或初始化 create ice restart offer 相关逻辑。 |
| [L42](../src/webrtc/LibWebRtcSession.h#L42) | `CreateAnswer` | 声明 | `OperationId CreateAnswer() override` | 创建或初始化 create answer 相关逻辑。 |
| [L43](../src/webrtc/LibWebRtcSession.h#L43) | `ApplyRemoteDescription` | 声明 | `OperationId ApplyRemoteDescription( const SessionDescription& description) override` | 更新或应用 apply remote description 相关逻辑。 |
| [L45](../src/webrtc/LibWebRtcSession.h#L45) | `AddRemoteIceCandidate` | 声明 | `OperationId AddRemoteIceCandidate( const IceCandidate& candidate) override` | 实现 add remote ice candidate 对应的业务或工具逻辑。 |
| [L47](../src/webrtc/LibWebRtcSession.h#L47) | `CreateDataChannels` | 声明 | `OperationId CreateDataChannels( const std::vector<DataChannelSpec>& channels) override` | 创建或初始化 create data channels 相关逻辑。 |
| [L49](../src/webrtc/LibWebRtcSession.h#L49) | `SendData` | 声明 | `SendResult SendData(const std::string& channelName, std::span<const std::uint8_t> data, bool binary) override` | 发送或发布 send data 相关逻辑。 |
| [L52](../src/webrtc/LibWebRtcSession.h#L52) | `DataChannelBufferedAmount` | 声明 | `std::optional<std::uint64_t> DataChannelBufferedAmount( const std::string& channelName) const override` | 实现 data channel buffered amount 对应的业务或工具逻辑。 |
| [L54](../src/webrtc/LibWebRtcSession.h#L54) | `RequestStats` | 声明 | `void RequestStats() override` | 发起请求或查询 request stats 相关逻辑。 |
| [L55](../src/webrtc/LibWebRtcSession.h#L55) | `StatsSnapshot` | 声明 | `WebRtcSessionStatsSnapshot StatsSnapshot() const override` | 实现 stats snapshot 对应的业务或工具逻辑。 |
| [L56](../src/webrtc/LibWebRtcSession.h#L56) | `Close` | 声明 | `void Close() override` | 关闭并清理 close 相关逻辑。 |
| [L63](../src/webrtc/LibWebRtcSession.h#L63) | `AddVideoTrack` | 声明 | `AddVideoTrack( webrtc::scoped_refptr<webrtc::MediaStreamTrackInterface> track)` | 实现 add video track 对应的业务或工具逻辑。 |
| [L67](../src/webrtc/LibWebRtcSession.h#L67) | `AddVideoReceiveTransceiver` | 声明 | `AddVideoReceiveTransceiver()` | 实现 add video receive transceiver 对应的业务或工具逻辑。 |
| [L68](../src/webrtc/LibWebRtcSession.h#L68) | `PrepareVideoTransceiverSlot` | 声明 | `webrtc::RTCError PrepareVideoTransceiverSlot( const std::string& slot)` | 实现 prepare video transceiver slot 对应的业务或工具逻辑。 |
| [L70](../src/webrtc/LibWebRtcSession.h#L70) | `BindNegotiatedVideoTransceiverSlots` | 声明 | `webrtc::RTCError BindNegotiatedVideoTransceiverSlots( const std::vector<std::string>& slots)` | 实现 bind negotiated video transceiver slots 对应的业务或工具逻辑。 |
| [L72](../src/webrtc/LibWebRtcSession.h#L72) | `SetVideoSlotTrack` | 声明 | `webrtc::RTCError SetVideoSlotTrack( const std::string& slot, webrtc::scoped_refptr<webrtc::MediaStreamTrackInterface> track)` | 更新或应用 set video slot track 相关逻辑。 |
| [L75](../src/webrtc/LibWebRtcSession.h#L75) | `SetVideoSlotSendingActive` | 声明 | `webrtc::RTCError SetVideoSlotSendingActive( const std::string& slot, bool active)` | 更新或应用 set video slot sending active 相关逻辑。 |
| [L78](../src/webrtc/LibWebRtcSession.h#L78) | `SetFastDesktopBweStartupEnabled` | 声明 | `void SetFastDesktopBweStartupEnabled(bool enabled)` | 更新或应用 set fast desktop bwe startup enabled 相关逻辑。 |
| [L79](../src/webrtc/LibWebRtcSession.h#L79) | `SetAdaptiveDesktopNetworkFrameRateEnabled` | 声明 | `void SetAdaptiveDesktopNetworkFrameRateEnabled(bool enabled)` | 更新或应用 set adaptive desktop network frame rate enabled 相关逻辑。 |
| [L80](../src/webrtc/LibWebRtcSession.h#L80) | `SetScreenContentActivity` | 声明 | `void SetScreenContentActivity(ScreenContentActivity activity)` | 更新或应用 set screen content activity 相关逻辑。 |
| [L81](../src/webrtc/LibWebRtcSession.h#L81) | `RestartVideoSlotBandwidthEstimation` | 声明 | `void RestartVideoSlotBandwidthEstimation(const std::string& slot)` | 实现 restart video slot bandwidth estimation 对应的业务或工具逻辑。 |
| [L82](../src/webrtc/LibWebRtcSession.h#L82) | `FinishVideoSlotBandwidthBootstrap` | 声明 | `void FinishVideoSlotBandwidthBootstrap(const std::string& slot)` | 停止 finish video slot bandwidth bootstrap 相关逻辑。 |
| [L83](../src/webrtc/LibWebRtcSession.h#L83) | `SetVideoSlotEncodingPolicy` | 声明 | `webrtc::RTCError SetVideoSlotEncodingPolicy( const std::string& slot, std::uint32_t framesPerSecond, std::uint32_t width, std::uint32_t height)` | 更新或应用 set video slot encoding policy 相关逻辑。 |
| [L88](../src/webrtc/LibWebRtcSession.h#L88) | `PrepareAudioTransceiverSlot` | 声明 | `webrtc::RTCError PrepareAudioTransceiverSlot( const std::string& slot)` | 实现 prepare audio transceiver slot 对应的业务或工具逻辑。 |
| [L90](../src/webrtc/LibWebRtcSession.h#L90) | `BindNegotiatedAudioTransceiverSlot` | 声明 | `webrtc::RTCError BindNegotiatedAudioTransceiverSlot( const std::string& slot)` | 实现 bind negotiated audio transceiver slot 对应的业务或工具逻辑。 |
| [L92](../src/webrtc/LibWebRtcSession.h#L92) | `SetAudioSlotTrack` | 声明 | `webrtc::RTCError SetAudioSlotTrack( const std::string& slot, webrtc::scoped_refptr<webrtc::MediaStreamTrackInterface> track)` | 更新或应用 set audio slot track 相关逻辑。 |
| [L95](../src/webrtc/LibWebRtcSession.h#L95) | `SetRemoteAudioSlotEnabled` | 声明 | `void SetRemoteAudioSlotEnabled(const std::string& slot, bool enabled)` | 更新或应用 set remote audio slot enabled 相关逻辑。 |
| [L96](../src/webrtc/LibWebRtcSession.h#L96) | `AudioSlotPrepared` | 声明 | `bool AudioSlotPrepared(const std::string& slot) const` | 实现 audio slot prepared 对应的业务或工具逻辑。 |
| [L97](../src/webrtc/LibWebRtcSession.h#L97) | `PreparedVideoSlotCount` | 声明 | `std::size_t PreparedVideoSlotCount() const` | 实现 prepared video slot count 对应的业务或工具逻辑。 |
| [L98](../src/webrtc/LibWebRtcSession.h#L98) | `SetRemoteVideoSink` | 声明 | `void SetRemoteVideoSink( webrtc::VideoSinkInterface<webrtc::VideoFrame>* sink)` | 更新或应用 set remote video sink 相关逻辑。 |
| [L100](../src/webrtc/LibWebRtcSession.h#L100) | `SetRemoteVideoSlotSink` | 声明 | `void SetRemoteVideoSlotSink( const std::string& slot, webrtc::VideoSinkInterface<webrtc::VideoFrame>* sink)` | 更新或应用 set remote video slot sink 相关逻辑。 |
| [L104](../src/webrtc/LibWebRtcSession.h#L104) | `OnSignalingChange` | 声明 | `void OnSignalingChange( webrtc::PeerConnectionInterface::SignalingState state) override` | 接收并处理 on signaling change 相关逻辑。 |
| [L106](../src/webrtc/LibWebRtcSession.h#L106) | `OnDataChannel` | 声明 | `void OnDataChannel( webrtc::scoped_refptr<webrtc::DataChannelInterface> channel) override` | 接收并处理 on data channel 相关逻辑。 |
| [L108](../src/webrtc/LibWebRtcSession.h#L108) | `OnIceGatheringChange` | 声明 | `void OnIceGatheringChange( webrtc::PeerConnectionInterface::IceGatheringState state) override` | 接收并处理 on ice gathering change 相关逻辑。 |
| [L110](../src/webrtc/LibWebRtcSession.h#L110) | `OnIceCandidate` | 声明 | `void OnIceCandidate(const webrtc::IceCandidate* candidate) override` | 接收并处理 on ice candidate 相关逻辑。 |
| [L111](../src/webrtc/LibWebRtcSession.h#L111) | `OnConnectionChange` | 声明 | `void OnConnectionChange( webrtc::PeerConnectionInterface::PeerConnectionState state) override` | 接收并处理 on connection change 相关逻辑。 |
| [L113](../src/webrtc/LibWebRtcSession.h#L113) | `OnIceConnectionChange` | 声明 | `void OnIceConnectionChange( webrtc::PeerConnectionInterface::IceConnectionState state) override` | 接收并处理 on ice connection change 相关逻辑。 |
| [L115](../src/webrtc/LibWebRtcSession.h#L115) | `OnTrack` | 声明 | `void OnTrack( webrtc::scoped_refptr<webrtc::RtpTransceiverInterface> transceiver) override` | 接收并处理 on track 相关逻辑。 |
| [L151](../src/webrtc/LibWebRtcSession.h#L151) | `NextOperationId` | 声明 | `OperationId NextOperationId()` | 实现 next operation id 对应的业务或工具逻辑。 |
| [L153](../src/webrtc/LibWebRtcSession.h#L153) | `PeerConnection` | 声明 | `PeerConnection() const` | 实现 peer connection 对应的业务或工具逻辑。 |
| [L154](../src/webrtc/LibWebRtcSession.h#L154) | `CreateLocalDescription` | 声明 | `void CreateLocalDescription(OperationId operationId, SessionDescriptionType type, bool iceRestart = false)` | 创建或初始化 create local description 相关逻辑。 |
| [L157](../src/webrtc/LibWebRtcSession.h#L157) | `SetLocalDescription` | 声明 | `void SetLocalDescription( OperationId operationId, SessionDescription description, std::unique_ptr<webrtc::SessionDescriptionInterface> nativeDescription)` | 更新或应用 set local description 相关逻辑。 |
| [L161](../src/webrtc/LibWebRtcSession.h#L161) | `AttachDataChannel` | 声明 | `void AttachDataChannel( webrtc::scoped_refptr<webrtc::DataChannelInterface> channel)` | 实现 attach data channel 对应的业务或工具逻辑。 |
| [L163](../src/webrtc/LibWebRtcSession.h#L163) | `HandleDataChannelState` | 声明 | `void HandleDataChannelState( webrtc::scoped_refptr<webrtc::DataChannelInterface> channel)` | 接收并处理 handle data channel state 相关逻辑。 |
| [L165](../src/webrtc/LibWebRtcSession.h#L165) | `HandleDataMessage` | 声明 | `void HandleDataMessage(const std::string& label, const webrtc::DataBuffer& buffer)` | 接收并处理 handle data message 相关逻辑。 |
| [L167](../src/webrtc/LibWebRtcSession.h#L167) | `UpdatePeerConnectionState` | 声明 | `void UpdatePeerConnectionState( webrtc::PeerConnectionInterface::PeerConnectionState state)` | 更新或应用 update peer connection state 相关逻辑。 |
| [L169](../src/webrtc/LibWebRtcSession.h#L169) | `UpdateIceConnectionState` | 声明 | `void UpdateIceConnectionState( webrtc::PeerConnectionInterface::IceConnectionState state)` | 更新或应用 update ice connection state 相关逻辑。 |
| [L171](../src/webrtc/LibWebRtcSession.h#L171) | `ApplyPendingVideoStartBitrateBootstrap` | 声明 | `void ApplyPendingVideoStartBitrateBootstrap()` | 更新或应用 apply pending video start bitrate bootstrap 相关逻辑。 |
| [L172](../src/webrtc/LibWebRtcSession.h#L172) | `HandleCompletedStatsSample` | 声明 | `void HandleCompletedStatsSample()` | 接收并处理 handle completed stats sample 相关逻辑。 |
| [L173](../src/webrtc/LibWebRtcSession.h#L173) | `ApplyProgressiveBitrateCeilingDecision` | 声明 | `webrtc::RTCError ApplyProgressiveBitrateCeilingDecision( const ProgressiveBitrateCeilingDecision& decision, std::uint64_t decisionRevision, const ProgressiveBitrateCeilingState& previousState)` | 更新或应用 apply progressive bitrate ceiling decision 相关逻辑。 |
| [L177](../src/webrtc/LibWebRtcSession.h#L177) | `ApplyAdaptiveScreenFrameRateDecision` | 声明 | `webrtc::RTCError ApplyAdaptiveScreenFrameRateDecision( const AdaptiveScreenFrameRateDecision& decision, std::uint64_t decisionRevision, const AdaptiveScreenFrameRateState& previousState, webrtc::scoped_refptr<webrtc::...` | 更新或应用 apply adaptive screen frame rate decision 相关逻辑。 |
| [L182](../src/webrtc/LibWebRtcSession.h#L182) | `PulseVideoSlotAllocationProbe` | 声明 | `bool PulseVideoSlotAllocationProbe( webrtc::scoped_refptr<webrtc::RtpTransceiverInterface> transceiver, std::uint64_t startBitrate, std::uint64_t maxBitrate, std::string* error)` | 实现 pulse video slot allocation probe 对应的业务或工具逻辑。 |
| [L187](../src/webrtc/LibWebRtcSession.h#L187) | `CombinedConnectionStateLocked` | 声明 | `WebRtcSessionState CombinedConnectionStateLocked() const` | 实现 combined connection state locked 对应的业务或工具逻辑。 |
| [L188](../src/webrtc/LibWebRtcSession.h#L188) | `ChangeState` | 声明 | `void ChangeState(WebRtcSessionState state)` | 实现 change state 对应的业务或工具逻辑。 |
| [L189](../src/webrtc/LibWebRtcSession.h#L189) | `CompleteOperation` | 声明 | `void CompleteOperation(OperationId operationId)` | 实现 complete operation 对应的业务或工具逻辑。 |
| [L190](../src/webrtc/LibWebRtcSession.h#L190) | `FailOperation` | 声明 | `void FailOperation(OperationId operationId, std::string code, std::string message)` | 实现 fail operation 对应的业务或工具逻辑。 |
| [L193](../src/webrtc/LibWebRtcSession.h#L193) | `Observer` | 声明 | `IWebRtcSessionObserver* Observer() const` | 实现 observer 对应的业务或工具逻辑。 |
| [L194](../src/webrtc/LibWebRtcSession.h#L194) | `DetachRemoteVideoSink` | 声明 | `void DetachRemoteVideoSink()` | 实现 detach remote video sink 对应的业务或工具逻辑。 |

## `src/webrtc/LibWebRtcSession.Lifecycle.inc`

[打开源码](../src/webrtc/LibWebRtcSession.Lifecycle.inc) · **文件作用：** `LibWebRtcSession` 的实现切片，集中实现 lifecycle 相关逻辑；成员状态仍定义在所属头文件中。

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L4](../src/webrtc/LibWebRtcSession.Lifecycle.inc#L4) | `LibWebRtcSession::LibWebRtcSession` | 定义 | `LibWebRtcSession::LibWebRtcSession( webrtc::scoped_refptr<webrtc::PeerConnectionFactoryInterface> factory) : callbackGate_(std::make_shared<CallbackGate>(this)), factory_(std::move(factory)), statsCollector_( std::mak...` | SPDX-License-Identifier: GPL-3.0-only Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd) |
| [L12](../src/webrtc/LibWebRtcSession.Lifecycle.inc#L12) | `LibWebRtcSession::~LibWebRtcSession` | 定义 | `LibWebRtcSession::~LibWebRtcSession()` | 停止相关活动并释放 LibWebRtcSession 实例拥有的资源。 |
| [L19](../src/webrtc/LibWebRtcSession.Lifecycle.inc#L19) | `LibWebRtcSession::SetObserver` | 定义 | `void LibWebRtcSession::SetObserver(IWebRtcSessionObserver* observer)` | 更新或应用 set observer 相关逻辑。 |
| [L25](../src/webrtc/LibWebRtcSession.Lifecycle.inc#L25) | `LibWebRtcSession::Start` | 定义 | `OperationId LibWebRtcSession::Start(const WebRtcSessionConfig& config)` | 启动 start 相关逻辑。 |

## `src/webrtc/LibWebRtcSession.Negotiation.inc`

[打开源码](../src/webrtc/LibWebRtcSession.Negotiation.inc) · **文件作用：** `LibWebRtcSession` 的实现切片，集中实现 negotiation 相关逻辑；成员状态仍定义在所属头文件中。

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L4](../src/webrtc/LibWebRtcSession.Negotiation.inc#L4) | `LibWebRtcSession::CreateOffer` | 定义 | `OperationId LibWebRtcSession::CreateOffer()` | SPDX-License-Identifier: GPL-3.0-only Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd) |
| [L11](../src/webrtc/LibWebRtcSession.Negotiation.inc#L11) | `LibWebRtcSession::CreateIceRestartOffer` | 定义 | `OperationId LibWebRtcSession::CreateIceRestartOffer()` | 创建或初始化 create ice restart offer 相关逻辑。 |
| [L19](../src/webrtc/LibWebRtcSession.Negotiation.inc#L19) | `LibWebRtcSession::CreateAnswer` | 定义 | `OperationId LibWebRtcSession::CreateAnswer()` | 创建或初始化 create answer 相关逻辑。 |
| [L26](../src/webrtc/LibWebRtcSession.Negotiation.inc#L26) | `LibWebRtcSession::ApplyRemoteDescription` | 定义 | `OperationId LibWebRtcSession::ApplyRemoteDescription( const SessionDescription& description)` | 更新或应用 apply remote description 相关逻辑。 |
| [L66](../src/webrtc/LibWebRtcSession.Negotiation.inc#L66) | `LibWebRtcSession::AddRemoteIceCandidate` | 定义 | `OperationId LibWebRtcSession::AddRemoteIceCandidate( const IceCandidate& candidate)` | 实现 add remote ice candidate 对应的业务或工具逻辑。 |

## `src/webrtc/LibWebRtcSession.NegotiationInternals.inc`

[打开源码](../src/webrtc/LibWebRtcSession.NegotiationInternals.inc) · **文件作用：** `LibWebRtcSession` 的实现切片，集中实现 negotiation internals 相关逻辑；成员状态仍定义在所属头文件中。

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L4](../src/webrtc/LibWebRtcSession.NegotiationInternals.inc#L4) | `LibWebRtcSession::NextOperationId` | 定义 | `OperationId LibWebRtcSession::NextOperationId()` | SPDX-License-Identifier: GPL-3.0-only Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd) |
| [L10](../src/webrtc/LibWebRtcSession.NegotiationInternals.inc#L10) | `LibWebRtcSession::PeerConnection` | 定义 | `LibWebRtcSession::PeerConnection() const` | 实现 peer connection 对应的业务或工具逻辑。 |
| [L16](../src/webrtc/LibWebRtcSession.NegotiationInternals.inc#L16) | `LibWebRtcSession::CreateLocalDescription` | 定义 | `void LibWebRtcSession::CreateLocalDescription( OperationId operationId, SessionDescriptionType type, bool iceRestart)` | 创建或初始化 create local description 相关逻辑。 |
| [L67](../src/webrtc/LibWebRtcSession.NegotiationInternals.inc#L67) | `LibWebRtcSession::SetLocalDescription` | 定义 | `void LibWebRtcSession::SetLocalDescription( OperationId operationId, SessionDescription description, std::unique_ptr<webrtc::SessionDescriptionInterface> nativeDescription)` | 更新或应用 set local description 相关逻辑。 |

## `src/webrtc/LibWebRtcSession.Observers.inc`

[打开源码](../src/webrtc/LibWebRtcSession.Observers.inc) · **文件作用：** `LibWebRtcSession` 的实现切片，集中实现 observers 相关逻辑；成员状态仍定义在所属头文件中。

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L4](../src/webrtc/LibWebRtcSession.Observers.inc#L4) | `LibWebRtcSession::OnSignalingChange` | 定义 | `void LibWebRtcSession::OnSignalingChange( webrtc::PeerConnectionInterface::SignalingState) {}` | SPDX-License-Identifier: GPL-3.0-only Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd) |
| [L8](../src/webrtc/LibWebRtcSession.Observers.inc#L8) | `LibWebRtcSession::OnDataChannel` | 定义 | `void LibWebRtcSession::OnDataChannel( webrtc::scoped_refptr<webrtc::DataChannelInterface> channel)` | 接收并处理 on data channel 相关逻辑。 |
| [L14](../src/webrtc/LibWebRtcSession.Observers.inc#L14) | `LibWebRtcSession::OnIceGatheringChange` | 定义 | `void LibWebRtcSession::OnIceGatheringChange( webrtc::PeerConnectionInterface::IceGatheringState state)` | 接收并处理 on ice gathering change 相关逻辑。 |
| [L23](../src/webrtc/LibWebRtcSession.Observers.inc#L23) | `LibWebRtcSession::OnIceCandidate` | 定义 | `void LibWebRtcSession::OnIceCandidate(const webrtc::IceCandidate* candidate)` | 接收并处理 on ice candidate 相关逻辑。 |
| [L37](../src/webrtc/LibWebRtcSession.Observers.inc#L37) | `LibWebRtcSession::OnConnectionChange` | 定义 | `void LibWebRtcSession::OnConnectionChange( webrtc::PeerConnectionInterface::PeerConnectionState state)` | 接收并处理 on connection change 相关逻辑。 |
| [L43](../src/webrtc/LibWebRtcSession.Observers.inc#L43) | `LibWebRtcSession::OnIceConnectionChange` | 定义 | `void LibWebRtcSession::OnIceConnectionChange( webrtc::PeerConnectionInterface::IceConnectionState state)` | 接收并处理 on ice connection change 相关逻辑。 |
| [L49](../src/webrtc/LibWebRtcSession.Observers.inc#L49) | `LibWebRtcSession::OnTrack` | 定义 | `void LibWebRtcSession::OnTrack( webrtc::scoped_refptr<webrtc::RtpTransceiverInterface> transceiver)` | 接收并处理 on track 相关逻辑。 |

## `src/webrtc/LibWebRtcSession.State.inc`

[打开源码](../src/webrtc/LibWebRtcSession.State.inc) · **文件作用：** `LibWebRtcSession` 的实现切片，集中实现 state 相关逻辑；成员状态仍定义在所属头文件中。

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L4](../src/webrtc/LibWebRtcSession.State.inc#L4) | `LibWebRtcSession::UpdatePeerConnectionState` | 定义 | `void LibWebRtcSession::UpdatePeerConnectionState( webrtc::PeerConnectionInterface::PeerConnectionState state)` | SPDX-License-Identifier: GPL-3.0-only Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd) |
| [L29](../src/webrtc/LibWebRtcSession.State.inc#L29) | `LibWebRtcSession::UpdateIceConnectionState` | 定义 | `void LibWebRtcSession::UpdateIceConnectionState( webrtc::PeerConnectionInterface::IceConnectionState state)` | 更新或应用 update ice connection state 相关逻辑。 |
| [L57](../src/webrtc/LibWebRtcSession.State.inc#L57) | `LibWebRtcSession::CombinedConnectionStateLocked` | 定义 | `WebRtcSessionState LibWebRtcSession::CombinedConnectionStateLocked() const` | 实现 combined connection state locked 对应的业务或工具逻辑。 |
| [L111](../src/webrtc/LibWebRtcSession.State.inc#L111) | `LibWebRtcSession::ChangeState` | 定义 | `void LibWebRtcSession::ChangeState(WebRtcSessionState state)` | 实现 change state 对应的业务或工具逻辑。 |
| [L127](../src/webrtc/LibWebRtcSession.State.inc#L127) | `LibWebRtcSession::CompleteOperation` | 定义 | `void LibWebRtcSession::CompleteOperation(OperationId operationId)` | 实现 complete operation 对应的业务或工具逻辑。 |
| [L134](../src/webrtc/LibWebRtcSession.State.inc#L134) | `LibWebRtcSession::FailOperation` | 定义 | `void LibWebRtcSession::FailOperation(OperationId operationId, std::string code, std::string message)` | 实现 fail operation 对应的业务或工具逻辑。 |
| [L144](../src/webrtc/LibWebRtcSession.State.inc#L144) | `LibWebRtcSession::Observer` | 定义 | `IWebRtcSessionObserver* LibWebRtcSession::Observer() const` | 实现 observer 对应的业务或工具逻辑。 |

## `src/webrtc/LibWebRtcSession.StatsClose.inc`

[打开源码](../src/webrtc/LibWebRtcSession.StatsClose.inc) · **文件作用：** `LibWebRtcSession` 的实现切片，集中实现 stats close 相关逻辑；成员状态仍定义在所属头文件中。

### 类型

| 行 | 类型 | 种类 | 作用 |
|---:|---|---|---|
| [L216](../src/webrtc/LibWebRtcSession.StatsClose.inc#L216) | `EncodingPolicy` | struct | 定义 EncodingPolicy 的 struct 类型和相关状态。 |

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L4](../src/webrtc/LibWebRtcSession.StatsClose.inc#L4) | `LibWebRtcSession::RequestStats` | 定义 | `void LibWebRtcSession::RequestStats()` | SPDX-License-Identifier: GPL-3.0-only Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd) |
| [L20](../src/webrtc/LibWebRtcSession.StatsClose.inc#L20) | `LibWebRtcSession::HandleCompletedStatsSample` | 定义 | `void LibWebRtcSession::HandleCompletedStatsSample()` | 接收并处理 handle completed stats sample 相关逻辑。 |
| [L103](../src/webrtc/LibWebRtcSession.StatsClose.inc#L103) | `LibWebRtcSession::ApplyProgressiveBitrateCeilingDecision` | 定义 | `webrtc::RTCError LibWebRtcSession::ApplyProgressiveBitrateCeilingDecision( const ProgressiveBitrateCeilingDecision& decision, std::uint64_t decisionRevision, const ProgressiveBitrateCeilingState& previousState)` | 更新或应用 apply progressive bitrate ceiling decision 相关逻辑。 |
| [L152](../src/webrtc/LibWebRtcSession.StatsClose.inc#L152) | `LibWebRtcSession::ApplyAdaptiveScreenFrameRateDecision` | 定义 | `webrtc::RTCError LibWebRtcSession::ApplyAdaptiveScreenFrameRateDecision( const AdaptiveScreenFrameRateDecision& decision, std::uint64_t decisionRevision, const AdaptiveScreenFrameRateState& previousState, webrtc::scop...` | 更新或应用 apply adaptive screen frame rate decision 相关逻辑。 |
| [L208](../src/webrtc/LibWebRtcSession.StatsClose.inc#L208) | `LibWebRtcSession::StatsSnapshot` | 定义 | `WebRtcSessionStatsSnapshot LibWebRtcSession::StatsSnapshot() const` | 实现 stats snapshot 对应的业务或工具逻辑。 |
| [L379](../src/webrtc/LibWebRtcSession.StatsClose.inc#L379) | `LibWebRtcSession::Close` | 定义 | `void LibWebRtcSession::Close()` | 关闭并清理 close 相关逻辑。 |
| [L432](../src/webrtc/LibWebRtcSession.StatsClose.inc#L432) | `LibWebRtcSession::AddVideoTrack` | 定义 | `LibWebRtcSession::AddVideoTrack( webrtc::scoped_refptr<webrtc::MediaStreamTrackInterface> track)` | 实现 add video track 对应的业务或工具逻辑。 |
| [L458](../src/webrtc/LibWebRtcSession.StatsClose.inc#L458) | `LibWebRtcSession::AddVideoReceiveTransceiver` | 定义 | `LibWebRtcSession::AddVideoReceiveTransceiver()` | 实现 add video receive transceiver 对应的业务或工具逻辑。 |

## `src/webrtc/LibWebRtcSession.VideoSlots.inc`

[打开源码](../src/webrtc/LibWebRtcSession.VideoSlots.inc) · **文件作用：** `LibWebRtcSession` 的实现切片，集中实现 video slots 相关逻辑；成员状态仍定义在所属头文件中。

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L4](../src/webrtc/LibWebRtcSession.VideoSlots.inc#L4) | `LibWebRtcSession::PrepareVideoTransceiverSlot` | 定义 | `webrtc::RTCError LibWebRtcSession::PrepareVideoTransceiverSlot( const std::string& slot)` | SPDX-License-Identifier: GPL-3.0-only Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd) |
| [L60](../src/webrtc/LibWebRtcSession.VideoSlots.inc#L60) | `LibWebRtcSession::BindNegotiatedVideoTransceiverSlots` | 定义 | `webrtc::RTCError LibWebRtcSession::BindNegotiatedVideoTransceiverSlots( const std::vector<std::string>& slots)` | 实现 bind negotiated video transceiver slots 对应的业务或工具逻辑。 |
| [L146](../src/webrtc/LibWebRtcSession.VideoSlots.inc#L146) | `LibWebRtcSession::SetVideoSlotTrack` | 定义 | `webrtc::RTCError LibWebRtcSession::SetVideoSlotTrack( const std::string& slot, webrtc::scoped_refptr<webrtc::MediaStreamTrackInterface> track)` | 更新或应用 set video slot track 相关逻辑。 |
| [L173](../src/webrtc/LibWebRtcSession.VideoSlots.inc#L173) | `LibWebRtcSession::SetVideoSlotSendingActive` | 定义 | `webrtc::RTCError LibWebRtcSession::SetVideoSlotSendingActive( const std::string& slot, bool active)` | 更新或应用 set video slot sending active 相关逻辑。 |
| [L254](../src/webrtc/LibWebRtcSession.VideoSlots.inc#L254) | `LibWebRtcSession::SetFastDesktopBweStartupEnabled` | 定义 | `void LibWebRtcSession::SetFastDesktopBweStartupEnabled(bool enabled)` | 更新或应用 set fast desktop bwe startup enabled 相关逻辑。 |
| [L265](../src/webrtc/LibWebRtcSession.VideoSlots.inc#L265) | `LibWebRtcSession::SetAdaptiveDesktopNetworkFrameRateEnabled` | 定义 | `void LibWebRtcSession::SetAdaptiveDesktopNetworkFrameRateEnabled( bool enabled)` | 更新或应用 set adaptive desktop network frame rate enabled 相关逻辑。 |
| [L284](../src/webrtc/LibWebRtcSession.VideoSlots.inc#L284) | `LibWebRtcSession::SetScreenContentActivity` | 定义 | `void LibWebRtcSession::SetScreenContentActivity( ScreenContentActivity activity)` | 更新或应用 set screen content activity 相关逻辑。 |
| [L291](../src/webrtc/LibWebRtcSession.VideoSlots.inc#L291) | `LibWebRtcSession::RestartVideoSlotBandwidthEstimation` | 定义 | `void LibWebRtcSession::RestartVideoSlotBandwidthEstimation( const std::string& slot)` | 实现 restart video slot bandwidth estimation 对应的业务或工具逻辑。 |
| [L380](../src/webrtc/LibWebRtcSession.VideoSlots.inc#L380) | `LibWebRtcSession::FinishVideoSlotBandwidthBootstrap` | 定义 | `void LibWebRtcSession::FinishVideoSlotBandwidthBootstrap( const std::string& slot)` | 停止 finish video slot bandwidth bootstrap 相关逻辑。 |
| [L443](../src/webrtc/LibWebRtcSession.VideoSlots.inc#L443) | `LibWebRtcSession::PulseVideoSlotAllocationProbe` | 定义 | `bool LibWebRtcSession::PulseVideoSlotAllocationProbe( webrtc::scoped_refptr<webrtc::RtpTransceiverInterface> transceiver, std::uint64_t startBitrate, std::uint64_t maxBitrate, std::string* error)` | 实现 pulse video slot allocation probe 对应的业务或工具逻辑。 |
| [L490](../src/webrtc/LibWebRtcSession.VideoSlots.inc#L490) | `LibWebRtcSession::SetVideoSlotEncodingPolicy` | 定义 | `webrtc::RTCError LibWebRtcSession::SetVideoSlotEncodingPolicy( const std::string& slot, std::uint32_t framesPerSecond, std::uint32_t width, std::uint32_t height)` | 更新或应用 set video slot encoding policy 相关逻辑。 |
| [L690](../src/webrtc/LibWebRtcSession.VideoSlots.inc#L690) | `LibWebRtcSession::ApplyPendingVideoStartBitrateBootstrap` | 定义 | `void LibWebRtcSession::ApplyPendingVideoStartBitrateBootstrap()` | 更新或应用 apply pending video start bitrate bootstrap 相关逻辑。 |

## `src/webrtc/LibWebRtcSessionInternal.inc`

[打开源码](../src/webrtc/LibWebRtcSessionInternal.inc) · **文件作用：** `LibWebRtcSessionInternal` 的实现切片，集中实现 lib web rtc session internal 相关逻辑；成员状态仍定义在所属头文件中。

### 类型

| 行 | 类型 | 种类 | 作用 |
|---:|---|---|---|
| [L4](../src/webrtc/LibWebRtcSessionInternal.inc#L4) | `LibWebRtcSession::CallbackGate` | class | SPDX-License-Identifier: GPL-3.0-only Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd) |
| [L6](../src/webrtc/LibWebRtcSessionInternal.inc#L6) | `Lease` | class | 定义 Lease 的 class 类型和相关状态。 |
| [L67](../src/webrtc/LibWebRtcSessionInternal.inc#L67) | `LibWebRtcSession::DataChannelBinding` | class | 定义 LibWebRtcSession::DataChannelBinding 的 class 类型和相关状态。 |

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L9](../src/webrtc/LibWebRtcSessionInternal.inc#L9) | `Lease` | 定义 | `Lease(CallbackGate* gate, LibWebRtcSession* owner) : gate_(gate), owner_(owner) {}` | 实现 lease 对应的业务或工具逻辑。 |
| [L14](../src/webrtc/LibWebRtcSessionInternal.inc#L14) | `Lease` | 定义 | `Lease(Lease&& other) noexcept : gate_(std::exchange(other.gate_, nullptr)), owner_(std::exchange(other.owner_, nullptr)) {}` | 实现 lease 对应的业务或工具逻辑。 |
| [L18](../src/webrtc/LibWebRtcSessionInternal.inc#L18) | `~Lease` | 定义 | `~Lease()` | 停止相关活动并释放 Lease 实例拥有的资源。 |
| [L25](../src/webrtc/LibWebRtcSessionInternal.inc#L25) | `Owner` | 定义 | `LibWebRtcSession* Owner() const { return owner_; }` | 实现 owner 对应的业务或工具逻辑。 |
| [L32](../src/webrtc/LibWebRtcSessionInternal.inc#L32) | `CallbackGate` | 定义 | `explicit CallbackGate(LibWebRtcSession* owner) : owner_(owner) {}` | 实现 callback gate 对应的业务或工具逻辑。 |
| [L34](../src/webrtc/LibWebRtcSessionInternal.inc#L34) | `Enter` | 定义 | `Lease Enter()` | 实现 enter 对应的业务或工具逻辑。 |
| [L44](../src/webrtc/LibWebRtcSessionInternal.inc#L44) | `DetachAndWait` | 定义 | `void DetachAndWait()` | 实现 detach and wait 对应的业务或工具逻辑。 |
| [L52](../src/webrtc/LibWebRtcSessionInternal.inc#L52) | `Leave` | 定义 | `void Leave()` | 实现 leave 对应的业务或工具逻辑。 |
| [L70](../src/webrtc/LibWebRtcSessionInternal.inc#L70) | `DataChannelBinding` | 定义 | `DataChannelBinding( LibWebRtcSession* owner, webrtc::scoped_refptr<webrtc::DataChannelInterface> channel) : owner_(owner), channel_(std::move(channel))` | 实现 data channel binding 对应的业务或工具逻辑。 |
| [L78](../src/webrtc/LibWebRtcSessionInternal.inc#L78) | `~DataChannelBinding` | 定义 | `~DataChannelBinding() override { channel_->UnregisterObserver(); }` | 停止相关活动并释放 DataChannelBinding 实例拥有的资源。 |
| [L80](../src/webrtc/LibWebRtcSessionInternal.inc#L80) | `Channel` | 定义 | `webrtc::scoped_refptr<webrtc::DataChannelInterface> Channel() const` | 实现 channel 对应的业务或工具逻辑。 |
| [L85](../src/webrtc/LibWebRtcSessionInternal.inc#L85) | `Close` | 定义 | `void Close() { channel_->Close(); }` | 关闭并清理 close 相关逻辑。 |
| [L87](../src/webrtc/LibWebRtcSessionInternal.inc#L87) | `OnStateChange` | 定义 | `void OnStateChange() override` | 接收并处理 on state change 相关逻辑。 |
| [L92](../src/webrtc/LibWebRtcSessionInternal.inc#L92) | `OnMessage` | 定义 | `void OnMessage(const webrtc::DataBuffer& buffer) override` | 接收并处理 on message 相关逻辑。 |

## `src/webrtc/LocalPeerConnectionH264SelfTest.cpp`

[打开源码](../src/webrtc/LocalPeerConnectionH264SelfTest.cpp) · **文件作用：** 实现 local peer connection h264 self test 相关函数与文件级辅助逻辑。

### 类型

| 行 | 类型 | 种类 | 作用 |
|---:|---|---|---|
| [L64](../src/webrtc/LocalPeerConnectionH264SelfTest.cpp#L64) | `SyntheticVideoSource` | class | 定义 SyntheticVideoSource 的 class 类型和相关状态。 |
| [L74](../src/webrtc/LocalPeerConnectionH264SelfTest.cpp#L74) | `NativeFrameSink` | class | 定义 NativeFrameSink 的 class 类型和相关状态。 |
| [L133](../src/webrtc/LocalPeerConnectionH264SelfTest.cpp#L133) | `ControllerSelfTestObserver` | class | 定义 ControllerSelfTestObserver 的 class 类型和相关状态。 |
| [L248](../src/webrtc/LocalPeerConnectionH264SelfTest.cpp#L248) | `LocalSignalingEndpoint` | class | 定义 LocalSignalingEndpoint 的 class 类型和相关状态。 |
| [L339](../src/webrtc/LocalPeerConnectionH264SelfTest.cpp#L339) | `DiscardSignalingSender` | class | 定义 DiscardSignalingSender 的 class 类型和相关状态。 |

### 成员与文件级变量

| 行 | 变量 | 声明 | 作用 |
|---:|---|---|---|
| [L42](../src/webrtc/LocalPeerConnectionH264SelfTest.cpp#L42) | `kFrameWidth` | `constexpr int kFrameWidth = 320;` | 定义 frame width 的编译期常量或产品边界。 |
| [L43](../src/webrtc/LocalPeerConnectionH264SelfTest.cpp#L43) | `kFrameHeight` | `constexpr int kFrameHeight = 180;` | 定义 frame height 的编译期常量或产品边界。 |
| [L44](../src/webrtc/LocalPeerConnectionH264SelfTest.cpp#L44) | `kFrameRate` | `constexpr int kFrameRate = 30;` | 定义 frame rate 的编译期常量或产品边界。 |
| [L45](../src/webrtc/LocalPeerConnectionH264SelfTest.cpp#L45) | `kMaximumFrames` | `constexpr int kMaximumFrames = 180;` | 定义 maximum frames 的编译期常量或产品边界。 |
| [L46](../src/webrtc/LocalPeerConnectionH264SelfTest.cpp#L46) | `kOperationTimeout` | `constexpr auto kOperationTimeout = 10s;` | 定义 operation timeout 的编译期常量或产品边界。 |

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L48](../src/webrtc/LocalPeerConnectionH264SelfTest.cpp#L48) | `EqualsIgnoreCase` | 定义 | `bool EqualsIgnoreCase(const std::string& left, const char* right)` | 判断 equals ignore case 相关逻辑。 |
| [L66](../src/webrtc/LocalPeerConnectionH264SelfTest.cpp#L66) | `state` | 定义 | `SourceState state() const override { return kLive; }` | 实现 state 对应的业务或工具逻辑。 |
| [L67](../src/webrtc/LocalPeerConnectionH264SelfTest.cpp#L67) | `remote` | 定义 | `bool remote() const override { return false; }` | 实现 remote 对应的业务或工具逻辑。 |
| [L68](../src/webrtc/LocalPeerConnectionH264SelfTest.cpp#L68) | `is_screencast` | 定义 | `bool is_screencast() const override { return true; }` | 判断 is screencast 相关逻辑。 |
| [L69](../src/webrtc/LocalPeerConnectionH264SelfTest.cpp#L69) | `needs_denoising` | 定义 | `std::optional<bool> needs_denoising() const override { return false; }` | 判断 needs denoising 相关逻辑。 |
| [L71](../src/webrtc/LocalPeerConnectionH264SelfTest.cpp#L71) | `PushFrame` | 定义 | `void PushFrame(const webrtc::VideoFrame& frame) { OnFrame(frame); }` | 实现 push frame 对应的业务或工具逻辑。 |
| [L77](../src/webrtc/LocalPeerConnectionH264SelfTest.cpp#L77) | `OnFrame` | 定义 | `void OnFrame(const webrtc::VideoFrame& frame) override` | 接收并处理 on frame 相关逻辑。 |
| [L93](../src/webrtc/LocalPeerConnectionH264SelfTest.cpp#L93) | `WaitForNativeFrame` | 定义 | `bool WaitForNativeFrame(std::chrono::milliseconds timeout)` | 实现 wait for native frame 对应的业务或工具逻辑。 |
| [L100](../src/webrtc/LocalPeerConnectionH264SelfTest.cpp#L100) | `FrameCount` | 定义 | `int FrameCount() const` | 实现 frame count 对应的业务或工具逻辑。 |
| [L106](../src/webrtc/LocalPeerConnectionH264SelfTest.cpp#L106) | `NativeFrameCount` | 定义 | `int NativeFrameCount() const` | 实现 native frame count 对应的业务或工具逻辑。 |
| [L112](../src/webrtc/LocalPeerConnectionH264SelfTest.cpp#L112) | `Width` | 定义 | `int Width() const` | 实现 width 对应的业务或工具逻辑。 |
| [L118](../src/webrtc/LocalPeerConnectionH264SelfTest.cpp#L118) | `Height` | 定义 | `int Height() const` | 实现 height 对应的业务或工具逻辑。 |
| [L135](../src/webrtc/LocalPeerConnectionH264SelfTest.cpp#L135) | `OnControllerSnapshot` | 定义 | `void OnControllerSnapshot( const SessionControllerSnapshot& snapshot) override` | 接收并处理 on controller snapshot 相关逻辑。 |
| [L143](../src/webrtc/LocalPeerConnectionH264SelfTest.cpp#L143) | `OnDataChannelStateChanged` | 定义 | `void OnDataChannelStateChanged( const DataChannelInfo& channel) override` | 接收并处理 on data channel state changed 相关逻辑。 |
| [L151](../src/webrtc/LocalPeerConnectionH264SelfTest.cpp#L151) | `OnDataMessage` | 定义 | `void OnDataMessage(const std::string& label, std::span<const std::uint8_t> payload, bool) override` | 接收并处理 on data message 相关逻辑。 |
| [L166](../src/webrtc/LocalPeerConnectionH264SelfTest.cpp#L166) | `OnRemoteTrackAdded` | 定义 | `void OnRemoteTrackAdded(const RemoteTrackInfo&) override` | 接收并处理 on remote track added 相关逻辑。 |
| [L173](../src/webrtc/LocalPeerConnectionH264SelfTest.cpp#L173) | `WaitForState` | 定义 | `bool WaitForState(SessionControllerState expected, std::chrono::milliseconds timeout)` | 实现 wait for state 对应的业务或工具逻辑。 |
| [L185](../src/webrtc/LocalPeerConnectionH264SelfTest.cpp#L185) | `WaitForChannelOpen` | 定义 | `bool WaitForChannelOpen(const std::string& label, std::chrono::milliseconds timeout)` | 实现 wait for channel open 对应的业务或工具逻辑。 |
| [L196](../src/webrtc/LocalPeerConnectionH264SelfTest.cpp#L196) | `ChannelInfo` | 定义 | `std::optional<DataChannelInfo> ChannelInfo( const std::string& label) const` | 实现 channel info 对应的业务或工具逻辑。 |
| [L207](../src/webrtc/LocalPeerConnectionH264SelfTest.cpp#L207) | `WaitForMessage` | 定义 | `bool WaitForMessage(const std::string& label, const std::string& expected, std::chrono::milliseconds timeout)` | 实现 wait for message 对应的业务或工具逻辑。 |
| [L221](../src/webrtc/LocalPeerConnectionH264SelfTest.cpp#L221) | `ErrorText` | 定义 | `std::string ErrorText() const` | 实现 error text 对应的业务或工具逻辑。 |
| [L230](../src/webrtc/LocalPeerConnectionH264SelfTest.cpp#L230) | `WaitForErrorCode` | 定义 | `bool WaitForErrorCode(const std::string& expected, std::chrono::milliseconds timeout)` | 实现 wait for error code 对应的业务或工具逻辑。 |
| [L250](../src/webrtc/LocalPeerConnectionH264SelfTest.cpp#L250) | `SetPeer` | 定义 | `void SetPeer(SessionControllerBase* peer)` | 更新或应用 set peer 相关逻辑。 |
| [L256](../src/webrtc/LocalPeerConnectionH264SelfTest.cpp#L256) | `SendDescription` | 定义 | `bool SendDescription(const SessionDescription& description) override` | 发送或发布 send description 相关逻辑。 |
| [L279](../src/webrtc/LocalPeerConnectionH264SelfTest.cpp#L279) | `SendIceCandidate` | 定义 | `bool SendIceCandidate(const IceCandidate& candidate) override` | 发送或发布 send ice candidate 相关逻辑。 |
| [L303](../src/webrtc/LocalPeerConnectionH264SelfTest.cpp#L303) | `Description` | 定义 | `std::optional<SessionDescription> Description( SessionDescriptionType type) const` | 实现 description 对应的业务或工具逻辑。 |
| [L318](../src/webrtc/LocalPeerConnectionH264SelfTest.cpp#L318) | `CandidateCount` | 定义 | `int CandidateCount() const` | 判断 candidate count 相关逻辑。 |
| [L324](../src/webrtc/LocalPeerConnectionH264SelfTest.cpp#L324) | `CandidateBeforeDescriptionExercised` | 定义 | `bool CandidateBeforeDescriptionExercised() const` | 判断 candidate before description exercised 相关逻辑。 |
| [L341](../src/webrtc/LocalPeerConnectionH264SelfTest.cpp#L341) | `SendDescription` | 定义 | `bool SendDescription(const SessionDescription&) override { return true; }` | 发送或发布 send description 相关逻辑。 |
| [L342](../src/webrtc/LocalPeerConnectionH264SelfTest.cpp#L342) | `SendIceCandidate` | 定义 | `bool SendIceCandidate(const IceCandidate&) override { return true; }` | 发送或发布 send ice candidate 相关逻辑。 |
| [L345](../src/webrtc/LocalPeerConnectionH264SelfTest.cpp#L345) | `H264CodecPreferences` | 定义 | `std::vector<webrtc::RtpCodecCapability> H264CodecPreferences( webrtc::PeerConnectionFactoryInterface* factory)` | 实现 h264 codec preferences 对应的业务或工具逻辑。 |
| [L358](../src/webrtc/LocalPeerConnectionH264SelfTest.cpp#L358) | `CreateSyntheticFrame` | 定义 | `webrtc::VideoFrame CreateSyntheticFrame(int frameIndex)` | 创建或初始化 create synthetic frame 相关逻辑。 |
| [L386](../src/webrtc/LocalPeerConnectionH264SelfTest.cpp#L386) | `AppendResult` | 定义 | `void AppendResult(std::ostringstream& report, std::string_view label, bool value)` | 实现 append result 对应的业务或工具逻辑。 |
| [L395](../src/webrtc/LocalPeerConnectionH264SelfTest.cpp#L395) | `RunLocalPeerConnectionH264SelfTest` | 定义 | `LocalPeerConnectionH264SelfTestResult RunLocalPeerConnectionH264SelfTest( webrtc::scoped_refptr<webrtc::PeerConnectionFactoryInterface> factory, std::function<bool()> reconfigureEncoder)` | 执行后台循环或调度 run local peer connection h264 self test 相关逻辑。 |

## `src/webrtc/LocalPeerConnectionH264SelfTest.h`

[打开源码](../src/webrtc/LocalPeerConnectionH264SelfTest.h) · **文件作用：** 声明 local peer connection h264 self test 相关类型、接口、配置和成员状态。

### 类型

| 行 | 类型 | 种类 | 作用 |
|---:|---|---|---|
| [L14](../src/webrtc/LocalPeerConnectionH264SelfTest.h#L14) | `LocalPeerConnectionH264SelfTestResult` | struct | 定义 LocalPeerConnectionH264SelfTestResult 的 struct 类型和相关状态。 |

### 成员与文件级变量

| 行 | 变量 | 声明 | 作用 |
|---:|---|---|---|
| [L15](../src/webrtc/LocalPeerConnectionH264SelfTest.h#L15) | `passed` | `bool passed = false;` | 保存 passed 相关配置或运行状态。 |
| [L16](../src/webrtc/LocalPeerConnectionH264SelfTest.h#L16) | `report` | `std::string report;` | 保存 report 相关配置或运行状态。 |

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L22](../src/webrtc/LocalPeerConnectionH264SelfTest.h#L22) | `RunLocalPeerConnectionH264SelfTest` | 定义 | `LocalPeerConnectionH264SelfTestResult RunLocalPeerConnectionH264SelfTest( webrtc::scoped_refptr<webrtc::PeerConnectionFactoryInterface> factory, std::function<bool()> reconfigureEncoder = {})` | Builds two PeerConnections in one process and sends synthetic I420 frames through the complete H264/RTP/ICE/decode pipeline. The receiver must observe D3D11NativeFrameBuffer out... |

## `src/webrtc/PeerConnectionStatsCollector.cpp`

[打开源码](../src/webrtc/PeerConnectionStatsCollector.cpp) · **文件作用：** 实现 peer connection stats collector 相关函数与文件级辅助逻辑。

### 类型

| 行 | 类型 | 种类 | 作用 |
|---:|---|---|---|
| [L24](../src/webrtc/PeerConnectionStatsCollector.cpp#L24) | `CounterSample` | struct | 定义 CounterSample 的 struct 类型和相关状态。 |
| [L29](../src/webrtc/PeerConnectionStatsCollector.cpp#L29) | `AggregateSample` | struct | 定义 AggregateSample 的 struct 类型和相关状态。 |
| [L35](../src/webrtc/PeerConnectionStatsCollector.cpp#L35) | `WindowAverage` | struct | 定义 WindowAverage 的 struct 类型和相关状态。 |
| [L264](../src/webrtc/PeerConnectionStatsCollector.cpp#L264) | `PeerConnectionStatsCollector::State` | struct | 定义 PeerConnectionStatsCollector::State 的 struct 类型和相关状态。 |
| [L272](../src/webrtc/PeerConnectionStatsCollector.cpp#L272) | `PeerConnectionStatsCollector::Callback` | class | 定义 PeerConnectionStatsCollector::Callback 的 class 类型和相关状态。 |

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L41](../src/webrtc/PeerConnectionStatsCollector.cpp#L41) | `CalculateRate` | 定义 | `std::uint64_t CalculateRate( std::unordered_map<std::string, CounterSample>& history, const std::string& key, std::uint64_t value, std::int64_t timestampUs, std::uint32_t* windowMs = nullptr)` | 计算或采集 calculate rate 相关逻辑。 |
| [L71](../src/webrtc/PeerConnectionStatsCollector.cpp#L71) | `CalculateCounterPerSecond` | 定义 | `double CalculateCounterPerSecond( std::unordered_map<std::string, CounterSample>& history, const std::string& key, std::uint64_t value, std::int64_t timestampUs)` | 计算或采集 calculate counter per second 相关逻辑。 |
| [L94](../src/webrtc/PeerConnectionStatsCollector.cpp#L94) | `CalculateWindowAverage` | 定义 | `WindowAverage CalculateWindowAverage( std::unordered_map<std::string, AggregateSample>& history, const std::string& key, double total, std::uint64_t count, std::int64_t timestampUs, double scale)` | 计算或采集 calculate window average 相关逻辑。 |
| [L123](../src/webrtc/PeerConnectionStatsCollector.cpp#L123) | `ApplyLatestFrameTiming` | 定义 | `void ApplyLatestFrameTiming( RtpStreamStatsSnapshot& stream, std::string& implementation, VideoCodecTimingDirection expectedDirection)` | 更新或应用 apply latest frame timing 相关逻辑。 |
| [L192](../src/webrtc/PeerConnectionStatsCollector.cpp#L192) | `CodecName` | 定义 | `std::string CodecName( const webrtc::RTCStatsReport& report, const std::optional<std::string>& codecId)` | 实现 codec name 对应的业务或工具逻辑。 |
| [L206](../src/webrtc/PeerConnectionStatsCollector.cpp#L206) | `CandidateSnapshot` | 定义 | `IceCandidateStatsSnapshot CandidateSnapshot( const webrtc::RTCIceCandidateStats* candidate)` | 判断 candidate snapshot 相关逻辑。 |
| [L227](../src/webrtc/PeerConnectionStatsCollector.cpp#L227) | `RouteType` | 定义 | `std::string RouteType( const IceCandidateStatsSnapshot& local, const IceCandidateStatsSnapshot& remote)` | 实现 route type 对应的业务或工具逻辑。 |
| [L248](../src/webrtc/PeerConnectionStatsCollector.cpp#L248) | `LossPercent` | 定义 | `double LossPercent(std::int64_t lost, std::uint64_t received)` | 实现 loss percent 对应的业务或工具逻辑。 |
| [L275](../src/webrtc/PeerConnectionStatsCollector.cpp#L275) | `Callback` | 定义 | `Callback( std::shared_ptr<State> state, CompletionCallback completion) : state_(std::move(state)), completion_(std::move(completion)) {}` | 实现 callback 对应的业务或工具逻辑。 |
| [L281](../src/webrtc/PeerConnectionStatsCollector.cpp#L281) | `OnStatsDelivered` | 定义 | `void OnStatsDelivered( const webrtc::scoped_refptr<const webrtc::RTCStatsReport>& report) override` | 接收并处理 on stats delivered 相关逻辑。 |
| [L296](../src/webrtc/PeerConnectionStatsCollector.cpp#L296) | `PeerConnectionStatsCollector::PeerConnectionStatsCollector` | 定义 | `PeerConnectionStatsCollector::PeerConnectionStatsCollector() : state_(std::make_shared<State>()) {}` | 构造并初始化 PeerConnectionStatsCollector 实例。 |
| [L302](../src/webrtc/PeerConnectionStatsCollector.cpp#L302) | `PeerConnectionStatsCollector::Request` | 定义 | `void PeerConnectionStatsCollector::Request( webrtc::scoped_refptr<webrtc::PeerConnectionInterface> peer, CompletionCallback completion)` | 发起请求或查询 request 相关逻辑。 |
| [L322](../src/webrtc/PeerConnectionStatsCollector.cpp#L322) | `PeerConnectionStatsCollector::Snapshot` | 定义 | `PeerConnectionStatsCollector::Snapshot() const` | 查询并返回 snapshot 相关逻辑。 |
| [L328](../src/webrtc/PeerConnectionStatsCollector.cpp#L328) | `PeerConnectionStatsCollector::ProcessReport` | 定义 | `void PeerConnectionStatsCollector::ProcessReport( const std::shared_ptr<State>& state, const webrtc::scoped_refptr<const webrtc::RTCStatsReport>& report)` | 接收并处理 process report 相关逻辑。 |

## `src/webrtc/PeerConnectionStatsCollector.h`

[打开源码](../src/webrtc/PeerConnectionStatsCollector.h) · **文件作用：** 声明 peer connection stats collector 相关类型、接口、配置和成员状态。

### 类型

| 行 | 类型 | 种类 | 作用 |
|---:|---|---|---|
| [L14](../src/webrtc/PeerConnectionStatsCollector.h#L14) | `RTCStatsReport` | class | 定义 RTCStatsReport 的 class 类型和相关状态。 |
| [L22](../src/webrtc/PeerConnectionStatsCollector.h#L22) | `PeerConnectionStatsCollector` | class | Converts libwebrtc's asynchronous RTCStatsReport into a transport-neutral immutable snapshot. It owns all counter history needed to calculate rates; callers never need to retain... |
| [L40](../src/webrtc/PeerConnectionStatsCollector.h#L40) | `State` | struct | 定义 State 的 struct 类型和相关状态。 |
| [L41](../src/webrtc/PeerConnectionStatsCollector.h#L41) | `Callback` | class | 定义 Callback 的 class 类型和相关状态。 |

### 成员与文件级变量

| 行 | 变量 | 声明 | 作用 |
|---:|---|---|---|
| [L14](../src/webrtc/PeerConnectionStatsCollector.h#L14) | `RTCStatsReport` | `class RTCStatsReport;` | 保存 rtc stats report 相关配置或运行状态。 |
| [L40](../src/webrtc/PeerConnectionStatsCollector.h#L40) | `State` | `struct State;` | 保存状态机当前状态：state。 |
| [L41](../src/webrtc/PeerConnectionStatsCollector.h#L41) | `Callback` | `class Callback;` | 保存回调或观察者入口：callback。 |
| [L47](../src/webrtc/PeerConnectionStatsCollector.h#L47) | `state_` | `std::shared_ptr<State> state_;` | 保存状态机当前状态：state。 |

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L26](../src/webrtc/PeerConnectionStatsCollector.h#L26) | `PeerConnectionStatsCollector` | 声明 | `PeerConnectionStatsCollector()` | 实现 peer connection stats collector 对应的业务或工具逻辑。 |
| [L27](../src/webrtc/PeerConnectionStatsCollector.h#L27) | `~PeerConnectionStatsCollector` | 声明 | `~PeerConnectionStatsCollector()` | 停止相关活动并释放 PeerConnectionStatsCollector 实例拥有的资源。 |
| [L29](../src/webrtc/PeerConnectionStatsCollector.h#L29) | `PeerConnectionStatsCollector` | 声明 | `PeerConnectionStatsCollector( const PeerConnectionStatsCollector&) = delete` | 实现 peer connection stats collector 对应的业务或工具逻辑。 |
| [L37](../src/webrtc/PeerConnectionStatsCollector.h#L37) | `Snapshot` | 声明 | `WebRtcSessionStatsSnapshot Snapshot() const` | 查询并返回 snapshot 相关逻辑。 |
| [L43](../src/webrtc/PeerConnectionStatsCollector.h#L43) | `ProcessReport` | 声明 | `static void ProcessReport( const std::shared_ptr<State>& state, const webrtc::scoped_refptr<const webrtc::RTCStatsReport>& report)` | 接收并处理 process report 相关逻辑。 |

## `src/webrtc/VideoCodecTimingTelemetry.h`

[打开源码](../src/webrtc/VideoCodecTimingTelemetry.h) · **文件作用：** 声明 video codec timing telemetry 相关类型、接口、配置和成员状态。

### 类型

| 行 | 类型 | 种类 | 作用 |
|---:|---|---|---|
| [L20](../src/webrtc/VideoCodecTimingTelemetry.h#L20) | `VideoCodecTimingDirection` | enum class | 定义 VideoCodecTimingDirection 的 enum class 类型和相关状态。 |
| [L25](../src/webrtc/VideoCodecTimingTelemetry.h#L25) | `VideoDecodePipelineTimingSnapshot` | struct | 定义 VideoDecodePipelineTimingSnapshot 的 struct 类型和相关状态。 |
| [L41](../src/webrtc/VideoCodecTimingTelemetry.h#L41) | `VideoCodecFrameTimingSnapshot` | struct | 定义 VideoCodecFrameTimingSnapshot 的 struct 类型和相关状态。 |
| [L63](../src/webrtc/VideoCodecTimingTelemetry.h#L63) | `VideoCodecTimingRegistry` | class | 定义 VideoCodecTimingRegistry 的 class 类型和相关状态。 |
| [L211](../src/webrtc/VideoCodecTimingTelemetry.h#L211) | `TimingWindows` | struct | 定义 TimingWindows 的 struct 类型和相关状态。 |

### 成员与文件级变量

| 行 | 变量 | 声明 | 作用 |
|---:|---|---|---|
| [L26](../src/webrtc/VideoCodecTimingTelemetry.h#L26) | `available` | `bool available = false;` | 保存能力或开关状态：available。 |
| [L27](../src/webrtc/VideoCodecTimingTelemetry.h#L27) | `asynchronous` | `bool asynchronous = false;` | 保存 asynchronous 相关配置或运行状态。 |
| [L28](../src/webrtc/VideoCodecTimingTelemetry.h#L28) | `inputPreparationUs` | `std::uint64_t inputPreparationUs = 0;` | 保存 input preparation us 相关配置或运行状态。 |
| [L29](../src/webrtc/VideoCodecTimingTelemetry.h#L29) | `inputQueueWaitUs` | `std::uint64_t inputQueueWaitUs = 0;` | 保存 input queue wait us 相关配置或运行状态。 |
| [L30](../src/webrtc/VideoCodecTimingTelemetry.h#L30) | `transformWaitUs` | `std::uint64_t transformWaitUs = 0;` | 保存 transform wait us 相关配置或运行状态。 |
| [L31](../src/webrtc/VideoCodecTimingTelemetry.h#L31) | `outputDeliveryUs` | `std::uint64_t outputDeliveryUs = 0;` | 保存 output delivery us 相关配置或运行状态。 |
| [L32](../src/webrtc/VideoCodecTimingTelemetry.h#L32) | `averageTransformWaitUs` | `std::uint64_t averageTransformWaitUs = 0;` | 保存 average transform wait us 相关配置或运行状态。 |
| [L33](../src/webrtc/VideoCodecTimingTelemetry.h#L33) | `p95TransformWaitUs` | `std::uint64_t p95TransformWaitUs = 0;` | 保存 p95 transform wait us 相关配置或运行状态。 |
| [L34](../src/webrtc/VideoCodecTimingTelemetry.h#L34) | `maximumTransformWaitUs` | `std::uint64_t maximumTransformWaitUs = 0;` | 保存 maximum transform wait us 相关配置或运行状态。 |
| [L35](../src/webrtc/VideoCodecTimingTelemetry.h#L35) | `queuedInputFrames` | `std::uint32_t queuedInputFrames = 0;` | 保存 queued input frames 相关配置或运行状态。 |
| [L36](../src/webrtc/VideoCodecTimingTelemetry.h#L36) | `inFlightFrames` | `std::uint32_t inFlightFrames = 0;` | 保存 in flight frames 相关配置或运行状态。 |
| [L37](../src/webrtc/VideoCodecTimingTelemetry.h#L37) | `peakBacklogFrames` | `std::uint32_t peakBacklogFrames = 0;` | 保存 peak backlog frames 相关配置或运行状态。 |
| [L38](../src/webrtc/VideoCodecTimingTelemetry.h#L38) | `backpressureFallbacks` | `std::uint64_t backpressureFallbacks = 0;` | 保存 backpressure fallbacks 相关配置或运行状态。 |
| [L42](../src/webrtc/VideoCodecTimingTelemetry.h#L42) | `valid` | `bool valid = false;` | 保存身份或作用域标识：valid。 |
| [L43](../src/webrtc/VideoCodecTimingTelemetry.h#L43) | `instanceId` | `std::uint64_t instanceId = 0;` | 保存身份或作用域标识：instance id。 |
| [L45](../src/webrtc/VideoCodecTimingTelemetry.h#L45) | `kEncode` | `VideoCodecTimingDirection::kEncode;` | 定义 encode 的编译期常量或产品边界。 |
| [L46](../src/webrtc/VideoCodecTimingTelemetry.h#L46) | `implementation` | `std::string implementation;` | 保存 implementation 相关配置或运行状态。 |
| [L47](../src/webrtc/VideoCodecTimingTelemetry.h#L47) | `rtpTimestamp` | `std::uint32_t rtpTimestamp = 0;` | 保存 rtp timestamp 相关配置或运行状态。 |
| [L48](../src/webrtc/VideoCodecTimingTelemetry.h#L48) | `latestFrameDurationUs` | `std::uint64_t latestFrameDurationUs = 0;` | 保存 latest frame duration us 相关配置或运行状态。 |
| [L49](../src/webrtc/VideoCodecTimingTelemetry.h#L49) | `averageFrameDurationUs` | `std::uint64_t averageFrameDurationUs = 0;` | 保存 average frame duration us 相关配置或运行状态。 |
| [L50](../src/webrtc/VideoCodecTimingTelemetry.h#L50) | `p95FrameDurationUs` | `std::uint64_t p95FrameDurationUs = 0;` | 保存 p95 frame duration us 相关配置或运行状态。 |
| [L51](../src/webrtc/VideoCodecTimingTelemetry.h#L51) | `maximumFrameDurationUs` | `std::uint64_t maximumFrameDurationUs = 0;` | 保存 maximum frame duration us 相关配置或运行状态。 |
| [L52](../src/webrtc/VideoCodecTimingTelemetry.h#L52) | `frameWidth` | `std::uint32_t frameWidth = 0;` | 保存计数、尺寸或速率指标：frame width。 |
| [L53](../src/webrtc/VideoCodecTimingTelemetry.h#L53) | `frameHeight` | `std::uint32_t frameHeight = 0;` | 保存计数、尺寸或速率指标：frame height。 |
| [L54](../src/webrtc/VideoCodecTimingTelemetry.h#L54) | `encodedBytes` | `std::uint64_t encodedBytes = 0;` | 保存计数、尺寸或速率指标：encoded bytes。 |
| [L55](../src/webrtc/VideoCodecTimingTelemetry.h#L55) | `qpAvailable` | `bool qpAvailable = false;` | 保存能力或开关状态：qp available。 |
| [L56](../src/webrtc/VideoCodecTimingTelemetry.h#L56) | `qp` | `std::int32_t qp = 0;` | 保存 qp 相关配置或运行状态。 |
| [L57](../src/webrtc/VideoCodecTimingTelemetry.h#L57) | `completedAtSteadyUs` | `std::int64_t completedAtSteadyUs = 0;` | 保存 completed at steady us 相关配置或运行状态。 |
| [L58](../src/webrtc/VideoCodecTimingTelemetry.h#L58) | `completedFrames` | `std::uint64_t completedFrames = 0;` | 保存 completed frames 相关配置或运行状态。 |
| [L59](../src/webrtc/VideoCodecTimingTelemetry.h#L59) | `droppedFrames` | `std::uint64_t droppedFrames = 0;` | 保存 dropped frames 相关配置或运行状态。 |
| [L60](../src/webrtc/VideoCodecTimingTelemetry.h#L60) | `decodePipeline` | `VideoDecodePipelineTimingSnapshot decodePipeline;` | 保存 decode pipeline 相关配置或运行状态。 |
| [L209](../src/webrtc/VideoCodecTimingTelemetry.h#L209) | `kTimingWindowSize` | `static constexpr std::size_t kTimingWindowSize = 240;` | 定义 timing window size 的编译期常量或产品边界。 |
| [L212](../src/webrtc/VideoCodecTimingTelemetry.h#L212) | `frameDurationsUs` | `std::deque<std::uint64_t> frameDurationsUs;` | 保存 frame durations us 相关配置或运行状态。 |
| [L213](../src/webrtc/VideoCodecTimingTelemetry.h#L213) | `transformWaitDurationsUs` | `std::deque<std::uint64_t> transformWaitDurationsUs;` | 保存 transform wait durations us 相关配置或运行状态。 |
| [L293](../src/webrtc/VideoCodecTimingTelemetry.h#L293) | `mutex_` | `mutable std::mutex mutex_;` | 保护跨线程共享状态：mutex。 |
| [L294](../src/webrtc/VideoCodecTimingTelemetry.h#L294) | `nextInstanceId_` | `std::uint64_t nextInstanceId_ = 1;` | 保存身份或作用域标识：next instance id。 |
| [L297](../src/webrtc/VideoCodecTimingTelemetry.h#L297) | `instances_` | `VideoCodecFrameTimingSnapshot> instances_;` | 保存 instances 相关配置或运行状态。 |
| [L298](../src/webrtc/VideoCodecTimingTelemetry.h#L298) | `timingWindows_` | `std::unordered_map<std::uint64_t, TimingWindows> timingWindows_;` | 保存 timing windows 相关配置或运行状态。 |

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L65](../src/webrtc/VideoCodecTimingTelemetry.h#L65) | `Instance` | 定义 | `static VideoCodecTimingRegistry& Instance()` | 实现 instance 对应的业务或工具逻辑。 |
| [L71](../src/webrtc/VideoCodecTimingTelemetry.h#L71) | `Register` | 定义 | `std::uint64_t Register(VideoCodecTimingDirection direction)` | 实现 register 对应的业务或工具逻辑。 |
| [L81](../src/webrtc/VideoCodecTimingTelemetry.h#L81) | `Unregister` | 定义 | `void Unregister(std::uint64_t id)` | 实现 unregister 对应的业务或工具逻辑。 |
| [L88](../src/webrtc/VideoCodecTimingTelemetry.h#L88) | `RecordCompleted` | 定义 | `void RecordCompleted( std::uint64_t id, std::string implementation, std::uint32_t rtpTimestamp, std::uint64_t durationUs, std::uint32_t frameWidth, std::uint32_t frameHeight, std::uint64_t encodedBytes, std::optional<...` | 实现 record completed 对应的业务或工具逻辑。 |
| [L147](../src/webrtc/VideoCodecTimingTelemetry.h#L147) | `StageDecodePipelineTiming` | 定义 | `static void StageDecodePipelineTiming( std::uint32_t rtpTimestamp, VideoDecodePipelineTimingSnapshot timing)` | A hardware decoder calls this immediately before invoking WebRTC's decoded-frame callback. The timing wrapper consumes it synchronously on the same thread, so no decoder/factory... |
| [L156](../src/webrtc/VideoCodecTimingTelemetry.h#L156) | `RecordDropped` | 定义 | `void RecordDropped(std::uint64_t id)` | 实现 record dropped 对应的业务或工具逻辑。 |
| [L166](../src/webrtc/VideoCodecTimingTelemetry.h#L166) | `SnapshotForImplementation` | 定义 | `SnapshotForImplementation(const std::string& implementation) const` | 查询并返回 snapshot for implementation 相关逻辑。 |
| [L180](../src/webrtc/VideoCodecTimingTelemetry.h#L180) | `TaggedImplementation` | 定义 | `static std::string TaggedImplementation( std::string implementation, std::uint64_t id)` | 实现 tagged implementation 对应的业务或工具逻辑。 |
| [L190](../src/webrtc/VideoCodecTimingTelemetry.h#L190) | `UntaggedImplementation` | 定义 | `static std::string UntaggedImplementation( const std::string& implementation)` | 实现 untagged implementation 对应的业务或工具逻辑。 |
| [L199](../src/webrtc/VideoCodecTimingTelemetry.h#L199) | `SteadyNowUs` | 定义 | `static std::int64_t SteadyNowUs()` | 实现 steady now us 对应的业务或工具逻辑。 |
| [L216](../src/webrtc/VideoCodecTimingTelemetry.h#L216) | `PushDuration` | 定义 | `static void PushDuration( std::deque<std::uint64_t>& values, std::uint64_t value)` | 实现 push duration 对应的业务或工具逻辑。 |
| [L226](../src/webrtc/VideoCodecTimingTelemetry.h#L226) | `Average` | 定义 | `static std::uint64_t Average( const std::deque<std::uint64_t>& values)` | 实现 average 对应的业务或工具逻辑。 |
| [L240](../src/webrtc/VideoCodecTimingTelemetry.h#L240) | `Percentile` | 定义 | `static std::uint64_t Percentile( const std::deque<std::uint64_t>& values, double percentile)` | 实现 percentile 对应的业务或工具逻辑。 |
| [L254](../src/webrtc/VideoCodecTimingTelemetry.h#L254) | `Maximum` | 定义 | `static std::uint64_t Maximum( const std::deque<std::uint64_t>& values)` | 实现 maximum 对应的业务或工具逻辑。 |
| [L262](../src/webrtc/VideoCodecTimingTelemetry.h#L262) | `ExtractInstanceId` | 定义 | `static std::optional<std::uint64_t> ExtractInstanceId( const std::string& implementation)` | 实现 extract instance id 对应的业务或工具逻辑。 |
| [L285](../src/webrtc/VideoCodecTimingTelemetry.h#L285) | `PendingDecodePipelineTiming` | 定义 | `PendingDecodePipelineTiming()` | 实现 pending decode pipeline timing 对应的业务或工具逻辑。 |

## `src/webrtc/VideoDecoderRuntimeStatus.h`

[打开源码](../src/webrtc/VideoDecoderRuntimeStatus.h) · **文件作用：** 声明 video decoder runtime status 相关类型、接口、配置和成员状态。

### 类型

| 行 | 类型 | 种类 | 作用 |
|---:|---|---|---|
| [L8](../src/webrtc/VideoDecoderRuntimeStatus.h#L8) | `VideoDecoderPreference` | enum class | 定义 VideoDecoderPreference 的 enum class 类型和相关状态。 |

## `src/webrtc/VideoEncoderRuntimeStatus.h`

[打开源码](../src/webrtc/VideoEncoderRuntimeStatus.h) · **文件作用：** 声明 video encoder runtime status 相关类型、接口、配置和成员状态。

### 类型

| 行 | 类型 | 种类 | 作用 |
|---:|---|---|---|
| [L16](../src/webrtc/VideoEncoderRuntimeStatus.h#L16) | `VideoEncoderPreference` | enum class | 定义 VideoEncoderPreference 的 enum class 类型和相关状态。 |
| [L24](../src/webrtc/VideoEncoderRuntimeStatus.h#L24) | `FfmpegHardwareBackend` | enum class | 定义 FfmpegHardwareBackend 的 enum class 类型和相关状态。 |
| [L47](../src/webrtc/VideoEncoderRuntimeStatus.h#L47) | `FfmpegX264Preset` | enum class | 定义 FfmpegX264Preset 的 enum class 类型和相关状态。 |
| [L84](../src/webrtc/VideoEncoderRuntimeStatus.h#L84) | `OpenH264QualityComplexity` | enum class | 定义 OpenH264QualityComplexity 的 enum class 类型和相关状态。 |
| [L90](../src/webrtc/VideoEncoderRuntimeStatus.h#L90) | `VideoEncoderQualityProfile` | struct | 定义 VideoEncoderQualityProfile 的 struct 类型和相关状态。 |
| [L131](../src/webrtc/VideoEncoderRuntimeStatus.h#L131) | `VideoEncoderInstanceRuntimeStatus` | struct | 定义 VideoEncoderInstanceRuntimeStatus 的 struct 类型和相关状态。 |
| [L158](../src/webrtc/VideoEncoderRuntimeStatus.h#L158) | `VideoEncoderRuntimeSnapshot` | struct | 定义 VideoEncoderRuntimeSnapshot 的 struct 类型和相关状态。 |
| [L168](../src/webrtc/VideoEncoderRuntimeStatus.h#L168) | `VideoEncoderRuntimeState` | class | Shared by the runtime, encoder factory and Media Foundation encoders. WebRTC creates and drives encoders on worker threads, so the UI must only consume immutable snapshots from ... |

### 成员与文件级变量

| 行 | 变量 | 声明 | 作用 |
|---:|---|---|---|
| [L91](../src/webrtc/VideoEncoderRuntimeStatus.h#L91) | `displayName` | `const char* displayName;` | 保存路径、地址或显示名称：display name。 |
| [L92](../src/webrtc/VideoEncoderRuntimeStatus.h#L92) | `x264Preset` | `const char* x264Preset;` | 保存 x264 preset 相关配置或运行状态。 |
| [L93](../src/webrtc/VideoEncoderRuntimeStatus.h#L93) | `nvencPreset` | `const char* nvencPreset;` | 保存 nvenc preset 相关配置或运行状态。 |
| [L94](../src/webrtc/VideoEncoderRuntimeStatus.h#L94) | `qsvPreset` | `const char* qsvPreset;` | 保存 qsv preset 相关配置或运行状态。 |
| [L95](../src/webrtc/VideoEncoderRuntimeStatus.h#L95) | `amfQuality` | `const char* amfQuality;` | 保存 amf quality 相关配置或运行状态。 |
| [L96](../src/webrtc/VideoEncoderRuntimeStatus.h#L96) | `mfQualityVsSpeed` | `std::uint32_t mfQualityVsSpeed;` | 保存 mf quality vs speed 相关配置或运行状态。 |
| [L97](../src/webrtc/VideoEncoderRuntimeStatus.h#L97) | `openH264Complexity` | `OpenH264QualityComplexity openH264Complexity;` | 保存 open h264 complexity 相关配置或运行状态。 |
| [L132](../src/webrtc/VideoEncoderRuntimeStatus.h#L132) | `instanceId` | `std::uint64_t instanceId = 0;` | 保存身份或作用域标识：instance id。 |
| [L133](../src/webrtc/VideoEncoderRuntimeStatus.h#L133) | `implementation` | `std::string implementation;` | 保存 implementation 相关配置或运行状态。 |
| [L134](../src/webrtc/VideoEncoderRuntimeStatus.h#L134) | `inputFormat` | `std::string inputFormat;` | 保存 input format 相关配置或运行状态。 |
| [L135](../src/webrtc/VideoEncoderRuntimeStatus.h#L135) | `state` | `std::string state;` | 保存状态机当前状态：state。 |
| [L136](../src/webrtc/VideoEncoderRuntimeStatus.h#L136) | `fallbackReason` | `std::string fallbackReason;` | 保存 fallback reason 相关配置或运行状态。 |
| [L137](../src/webrtc/VideoEncoderRuntimeStatus.h#L137) | `width` | `std::uint32_t width = 0;` | 保存计数、尺寸或速率指标：width。 |
| [L138](../src/webrtc/VideoEncoderRuntimeStatus.h#L138) | `height` | `std::uint32_t height = 0;` | 保存计数、尺寸或速率指标：height。 |
| [L139](../src/webrtc/VideoEncoderRuntimeStatus.h#L139) | `frameRate` | `std::uint32_t frameRate = 0;` | 保存计数、尺寸或速率指标：frame rate。 |
| [L140](../src/webrtc/VideoEncoderRuntimeStatus.h#L140) | `configuredFrameRate` | `std::uint32_t configuredFrameRate = 0;` | 保存计数、尺寸或速率指标：configured frame rate。 |
| [L141](../src/webrtc/VideoEncoderRuntimeStatus.h#L141) | `observedInputFrameRate` | `std::uint32_t observedInputFrameRate = 0;` | 保存计数、尺寸或速率指标：observed input frame rate。 |
| [L142](../src/webrtc/VideoEncoderRuntimeStatus.h#L142) | `observedOutputFrameRate` | `std::uint32_t observedOutputFrameRate = 0;` | 保存计数、尺寸或速率指标：observed output frame rate。 |
| [L143](../src/webrtc/VideoEncoderRuntimeStatus.h#L143) | `totalInputFrames` | `std::uint64_t totalInputFrames = 0;` | 保存 total input frames 相关配置或运行状态。 |
| [L144](../src/webrtc/VideoEncoderRuntimeStatus.h#L144) | `totalOutputFrames` | `std::uint64_t totalOutputFrames = 0;` | 保存 total output frames 相关配置或运行状态。 |
| [L145](../src/webrtc/VideoEncoderRuntimeStatus.h#L145) | `totalDroppedFrames` | `std::uint64_t totalDroppedFrames = 0;` | 保存 total dropped frames 相关配置或运行状态。 |
| [L146](../src/webrtc/VideoEncoderRuntimeStatus.h#L146) | `targetBitrateBps` | `std::uint32_t targetBitrateBps = 0;` | 保存计数、尺寸或速率指标：target bitrate bps。 |
| [L147](../src/webrtc/VideoEncoderRuntimeStatus.h#L147) | `configuredBitrateBps` | `std::uint32_t configuredBitrateBps = 0;` | 保存计数、尺寸或速率指标：configured bitrate bps。 |
| [L148](../src/webrtc/VideoEncoderRuntimeStatus.h#L148) | `initMinBitrateBps` | `std::uint32_t initMinBitrateBps = 0;` | 保存计数、尺寸或速率指标：init min bitrate bps。 |
| [L149](../src/webrtc/VideoEncoderRuntimeStatus.h#L149) | `initStartBitrateBps` | `std::uint32_t initStartBitrateBps = 0;` | 保存计数、尺寸或速率指标：init start bitrate bps。 |
| [L150](../src/webrtc/VideoEncoderRuntimeStatus.h#L150) | `initMaxBitrateBps` | `std::uint32_t initMaxBitrateBps = 0;` | 保存计数、尺寸或速率指标：init max bitrate bps。 |
| [L151](../src/webrtc/VideoEncoderRuntimeStatus.h#L151) | `initMaxFrameRate` | `std::uint32_t initMaxFrameRate = 0;` | 保存计数、尺寸或速率指标：init max frame rate。 |
| [L152](../src/webrtc/VideoEncoderRuntimeStatus.h#L152) | `rateUpdateSequence` | `std::uint64_t rateUpdateSequence = 0;` | 保存单调序号，用于排序或去重：rate update sequence。 |
| [L153](../src/webrtc/VideoEncoderRuntimeStatus.h#L153) | `lastRateUpdateUnixMs` | `std::uint64_t lastRateUpdateUnixMs = 0;` | 保存 last rate update unix ms 相关配置或运行状态。 |
| [L154](../src/webrtc/VideoEncoderRuntimeStatus.h#L154) | `bitrateConfigurationAccepted` | `bool bitrateConfigurationAccepted = false;` | 保存 bitrate configuration accepted 相关配置或运行状态。 |
| [L155](../src/webrtc/VideoEncoderRuntimeStatus.h#L155) | `bitrateReadbackAvailable` | `bool bitrateReadbackAvailable = false;` | 保存能力或开关状态：bitrate readback available。 |
| [L160](../src/webrtc/VideoEncoderRuntimeStatus.h#L160) | `kAutomatic` | `VideoEncoderPreference::kAutomatic;` | 定义 automatic 的编译期常量或产品边界。 |
| [L161](../src/webrtc/VideoEncoderRuntimeStatus.h#L161) | `instances` | `std::vector<VideoEncoderInstanceRuntimeStatus> instances;` | 保存 instances 相关配置或运行状态。 |
| [L162](../src/webrtc/VideoEncoderRuntimeStatus.h#L162) | `lastFallbackReason` | `std::string lastFallbackReason;` | 保存 last fallback reason 相关配置或运行状态。 |
| [L417](../src/webrtc/VideoEncoderRuntimeStatus.h#L417) | `mutex_` | `mutable std::mutex mutex_;` | 保护跨线程共享状态：mutex。 |
| [L418](../src/webrtc/VideoEncoderRuntimeStatus.h#L418) | `preference_` | `VideoEncoderPreference preference_;` | 保存 preference 相关配置或运行状态。 |
| [L419](../src/webrtc/VideoEncoderRuntimeStatus.h#L419) | `nextInstanceId_` | `std::uint64_t nextInstanceId_ = 1;` | 保存身份或作用域标识：next instance id。 |
| [L420](../src/webrtc/VideoEncoderRuntimeStatus.h#L420) | `desktopShareGeneration_` | `std::uint64_t desktopShareGeneration_ = 0;` | 标记当前世代，用于拒绝过期异步结果：desktop share generation。 |
| [L421](../src/webrtc/VideoEncoderRuntimeStatus.h#L421) | `desktopCaptureAdapterLuid_` | `std::uint64_t desktopCaptureAdapterLuid_ = 0;` | 保存身份或作用域标识：desktop capture adapter luid。 |
| [L424](../src/webrtc/VideoEncoderRuntimeStatus.h#L424) | `instances_` | `VideoEncoderInstanceRuntimeStatus> instances_;` | 保存 instances 相关配置或运行状态。 |
| [L425](../src/webrtc/VideoEncoderRuntimeStatus.h#L425) | `lastFallbackReason_` | `std::string lastFallbackReason_;` | 保存 last fallback reason 相关配置或运行状态。 |

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L31](../src/webrtc/VideoEncoderRuntimeStatus.h#L31) | `FfmpegHardwareBackendName` | 定义 | `inline const char* FfmpegHardwareBackendName( FfmpegHardwareBackend backend)` | 实现 ffmpeg hardware backend name 对应的业务或工具逻辑。 |
| [L59](../src/webrtc/VideoEncoderRuntimeStatus.h#L59) | `FfmpegX264PresetName` | 定义 | `inline const char* FfmpegX264PresetName(FfmpegX264Preset preset)` | 实现 ffmpeg x264 preset name 对应的业务或工具逻辑。 |
| [L103](../src/webrtc/VideoEncoderRuntimeStatus.h#L103) | `VideoEncoderQualityProfileForPreset` | 定义 | `inline VideoEncoderQualityProfile VideoEncoderQualityProfileForPreset( FfmpegX264Preset preset)` | The public setting has five levels. Values from the former nine-level libx264-only selector are deliberately folded into the nearest shared level so every backend observes one a... |
| [L170](../src/webrtc/VideoEncoderRuntimeStatus.h#L170) | `VideoEncoderRuntimeState` | 定义 | `explicit VideoEncoderRuntimeState(VideoEncoderPreference preference) : preference_(preference) {}` | 实现 video encoder runtime state 对应的业务或工具逻辑。 |
| [L174](../src/webrtc/VideoEncoderRuntimeStatus.h#L174) | `RegisterHardwareEncoder` | 定义 | `std::uint64_t RegisterHardwareEncoder( std::string implementation = "MediaFoundationD3D11H264")` | 实现 register hardware encoder 对应的业务或工具逻辑。 |
| [L189](../src/webrtc/VideoEncoderRuntimeStatus.h#L189) | `RegisterSoftwareEncoder` | 定义 | `std::uint64_t RegisterSoftwareEncoder(std::string implementation)` | 实现 register software encoder 对应的业务或工具逻辑。 |
| [L205](../src/webrtc/VideoEncoderRuntimeStatus.h#L205) | `UnregisterEncoder` | 定义 | `void UnregisterEncoder(std::uint64_t id)` | 实现 unregister encoder 对应的业务或工具逻辑。 |
| [L211](../src/webrtc/VideoEncoderRuntimeStatus.h#L211) | `MarkHardwareInitialized` | 定义 | `void MarkHardwareInitialized( std::uint64_t id, std::uint32_t width, std::uint32_t height, std::uint32_t frameRate, std::uint32_t minBitrateBps, std::uint32_t startBitrateBps, std::uint32_t maxBitrateBps)` | 实现 mark hardware initialized 对应的业务或工具逻辑。 |
| [L239](../src/webrtc/VideoEncoderRuntimeStatus.h#L239) | `MarkInputFormat` | 定义 | `void MarkInputFormat(std::uint64_t id, std::string inputFormat)` | 实现 mark input format 对应的业务或工具逻辑。 |
| [L248](../src/webrtc/VideoEncoderRuntimeStatus.h#L248) | `MarkRates` | 定义 | `void MarkRates( std::uint64_t id, std::uint32_t frameRate, std::uint32_t targetBitrateBps, std::uint32_t configuredFrameRate = 0, std::uint32_t observedInputFrameRate = 0, std::uint32_t configuredBitrateBps = 0, bool ...` | 实现 mark rates 对应的业务或工具逻辑。 |
| [L286](../src/webrtc/VideoEncoderRuntimeStatus.h#L286) | `MarkObservedFrameRate` | 定义 | `void MarkObservedFrameRate( std::uint64_t id, std::uint32_t observedInputFrameRate, std::uint32_t configuredFrameRate)` | 实现 mark observed frame rate 对应的业务或工具逻辑。 |
| [L301](../src/webrtc/VideoEncoderRuntimeStatus.h#L301) | `MarkFramePipeline` | 定义 | `void MarkFramePipeline( std::uint64_t id, std::uint32_t observedInputFrameRate, std::uint32_t observedOutputFrameRate, std::uint64_t totalInputFrames, std::uint64_t totalOutputFrames, std::uint64_t totalDroppedFrames)` | 实现 mark frame pipeline 对应的业务或工具逻辑。 |
| [L323](../src/webrtc/VideoEncoderRuntimeStatus.h#L323) | `MarkSoftwareInitialized` | 定义 | `void MarkSoftwareInitialized( std::uint64_t id, std::string implementation, std::uint32_t width, std::uint32_t height, std::uint32_t frameRate, std::uint32_t minBitrateBps, std::uint32_t startBitrateBps, std::uint32_t...` | 实现 mark software initialized 对应的业务或工具逻辑。 |
| [L351](../src/webrtc/VideoEncoderRuntimeStatus.h#L351) | `MarkHardwareStopped` | 定义 | `void MarkHardwareStopped(std::uint64_t id)` | 实现 mark hardware stopped 对应的业务或工具逻辑。 |
| [L361](../src/webrtc/VideoEncoderRuntimeStatus.h#L361) | `MarkSoftwareFallback` | 定义 | `void MarkSoftwareFallback(std::uint64_t id, std::string reason)` | 实现 mark software fallback 对应的业务或工具逻辑。 |
| [L372](../src/webrtc/VideoEncoderRuntimeStatus.h#L372) | `Snapshot` | 定义 | `VideoEncoderRuntimeSnapshot Snapshot() const` | 查询并返回 snapshot 相关逻辑。 |
| [L386](../src/webrtc/VideoEncoderRuntimeStatus.h#L386) | `SetPreference` | 定义 | `void SetPreference(VideoEncoderPreference preference)` | 更新或应用 set preference 相关逻辑。 |
| [L392](../src/webrtc/VideoEncoderRuntimeStatus.h#L392) | `SetDesktopShareGeneration` | 定义 | `void SetDesktopShareGeneration(std::uint64_t generation)` | 更新或应用 set desktop share generation 相关逻辑。 |
| [L398](../src/webrtc/VideoEncoderRuntimeStatus.h#L398) | `DesktopShareGeneration` | 定义 | `std::uint64_t DesktopShareGeneration() const` | 实现 desktop share generation 对应的业务或工具逻辑。 |
| [L404](../src/webrtc/VideoEncoderRuntimeStatus.h#L404) | `SetDesktopCaptureAdapterLuid` | 定义 | `void SetDesktopCaptureAdapterLuid(std::uint64_t adapterLuid)` | 更新或应用 set desktop capture adapter luid 相关逻辑。 |
| [L410](../src/webrtc/VideoEncoderRuntimeStatus.h#L410) | `DesktopCaptureAdapterLuid` | 定义 | `std::uint64_t DesktopCaptureAdapterLuid() const` | 实现 desktop capture adapter luid 对应的业务或工具逻辑。 |

## `src/webrtc/WebRtcRuntime.cpp`

[打开源码](../src/webrtc/WebRtcRuntime.cpp) · **文件作用：** 实现 web rtc runtime 相关函数与文件级辅助逻辑。

### 类型

| 行 | 类型 | 种类 | 作用 |
|---:|---|---|---|
| [L259](../src/webrtc/WebRtcRuntime.cpp#L259) | `VideoEncoderFactorySelection` | struct | 定义 VideoEncoderFactorySelection 的 struct 类型和相关状态。 |

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L48](../src/webrtc/WebRtcRuntime.cpp#L48) | `DescribeFormats` | 定义 | `std::vector<std::string> DescribeFormats( const std::vector<webrtc::SdpVideoFormat>& formats)` | 实现 describe formats 对应的业务或工具逻辑。 |
| [L59](../src/webrtc/WebRtcRuntime.cpp#L59) | `ContainsCodec` | 定义 | `bool ContainsCodec(const std::vector<webrtc::SdpVideoFormat>& formats, const std::string& codecName)` | 实现 contains codec 对应的业务或工具逻辑。 |
| [L78](../src/webrtc/WebRtcRuntime.cpp#L78) | `IsSystemDefaultDeviceId` | 定义 | `bool IsSystemDefaultDeviceId(const std::string& deviceId)` | 判断 is system default device id 相关逻辑。 |
| [L84](../src/webrtc/WebRtcRuntime.cpp#L84) | `WideToUtf8` | 定义 | `std::string WideToUtf8(const wchar_t* value)` | 实现 wide to utf8 对应的业务或工具逻辑。 |
| [L102](../src/webrtc/WebRtcRuntime.cpp#L102) | `DefaultWindowsAudioEndpointId` | 定义 | `std::string DefaultWindowsAudioEndpointId(EDataFlow flow)` | 实现 default windows audio endpoint id 对应的业务或工具逻辑。 |
| [L135](../src/webrtc/WebRtcRuntime.cpp#L135) | `ResolveActiveDeviceName` | 定义 | `std::string ResolveActiveDeviceName( const std::vector<MediaDeviceDescriptor>& devices, const std::string& activeDeviceId, bool recording)` | 查询并返回 resolve active device name 相关逻辑。 |
| [L153](../src/webrtc/WebRtcRuntime.cpp#L153) | `EnumerateAudioDevices` | 定义 | `std::vector<MediaDeviceDescriptor> EnumerateAudioDevices( webrtc::AudioDeviceModule* module, bool recording)` | 实现 enumerate audio devices 对应的业务或工具逻辑。 |
| [L189](../src/webrtc/WebRtcRuntime.cpp#L189) | `SelectAudioDevice` | 定义 | `bool SelectAudioDevice(webrtc::AudioDeviceModule* module, bool recording, const std::string& deviceId)` | 查询并返回 select audio device 相关逻辑。 |
| [L227](../src/webrtc/WebRtcRuntime.cpp#L227) | `DescribeHardwareEncoder` | 定义 | `std::string DescribeHardwareEncoder( const MfH264EncoderCapability& capability)` | 实现 describe hardware encoder 对应的业务或工具逻辑。 |
| [L270](../src/webrtc/WebRtcRuntime.cpp#L270) | `BuildVideoEncoderFactorySelection` | 定义 | `VideoEncoderFactorySelection BuildVideoEncoderFactorySelection( VideoEncoderPreference preference, FfmpegX264Preset quality, FfmpegHardwareBackend ffmpegHardwareBackend, const std::string& preferredAutomaticEncoderId,...` | 创建或初始化 build video encoder factory selection 相关逻辑。 |
| [L425](../src/webrtc/WebRtcRuntime.cpp#L425) | `hardwareFingerprint_` | 定义 | `std::make_shared<MfD3D11H264DecoderPreferenceState>()), hardwareFingerprint_(std::move(hardwareFingerprint)), encoderCapabilityCache_(std::move(encoderCapabilityCache)), preferredRecordingDeviceId_( preferredRecording...` | 实现 hardware fingerprint 对应的业务或工具逻辑。 |
| [L441](../src/webrtc/WebRtcRuntime.cpp#L441) | `WebRtcRuntime::~WebRtcRuntime` | 定义 | `WebRtcRuntime::~WebRtcRuntime()` | 停止相关活动并释放 WebRtcRuntime 实例拥有的资源。 |
| [L446](../src/webrtc/WebRtcRuntime.cpp#L446) | `WebRtcRuntime::Initialize` | 定义 | `bool WebRtcRuntime::Initialize()` | 创建或初始化 initialize 相关逻辑。 |
| [L847](../src/webrtc/WebRtcRuntime.cpp#L847) | `WebRtcRuntime::Shutdown` | 定义 | `void WebRtcRuntime::Shutdown()` | 关闭并清理 shutdown 相关逻辑。 |
| [L918](../src/webrtc/WebRtcRuntime.cpp#L918) | `WebRtcRuntime::CapabilityReport` | 定义 | `const WebRtcCapabilityReport& WebRtcRuntime::CapabilityReport() const noexcept` | 实现 capability report 对应的业务或工具逻辑。 |
| [L923](../src/webrtc/WebRtcRuntime.cpp#L923) | `WebRtcRuntime::EncoderRuntimeStatus` | 定义 | `VideoEncoderRuntimeSnapshot WebRtcRuntime::EncoderRuntimeStatus() const` | 编码 encoder runtime status 相关逻辑。 |
| [L933](../src/webrtc/WebRtcRuntime.cpp#L933) | `WebRtcRuntime::SetDesktopShareGeneration` | 定义 | `void WebRtcRuntime::SetDesktopShareGeneration(std::uint64_t generation)` | 更新或应用 set desktop share generation 相关逻辑。 |
| [L940](../src/webrtc/WebRtcRuntime.cpp#L940) | `WebRtcRuntime::SetDesktopCaptureAdapterLuid` | 定义 | `void WebRtcRuntime::SetDesktopCaptureAdapterLuid(std::uint64_t adapterLuid)` | 更新或应用 set desktop capture adapter luid 相关逻辑。 |
| [L947](../src/webrtc/WebRtcRuntime.cpp#L947) | `WebRtcRuntime::SetPreferredHardwareDecoderName` | 定义 | `void WebRtcRuntime::SetPreferredHardwareDecoderName(std::string name)` | 更新或应用 set preferred hardware decoder name 相关逻辑。 |
| [L957](../src/webrtc/WebRtcRuntime.cpp#L957) | `WebRtcRuntime::ApplyVideoCodecPreferences` | 定义 | `bool WebRtcRuntime::ApplyVideoCodecPreferences( VideoEncoderPreference videoEncoderPreference, FfmpegX264Preset quality, FfmpegHardwareBackend ffmpegHardwareBackend, std::string preferredAutomaticEncoderId, VideoDecod...` | 更新或应用 apply video codec preferences 相关逻辑。 |
| [L1032](../src/webrtc/WebRtcRuntime.cpp#L1032) | `WebRtcRuntime::EnumerateAudioDevices` | 定义 | `AudioDeviceEnumeration WebRtcRuntime::EnumerateAudioDevices() const` | 实现 enumerate audio devices 对应的业务或工具逻辑。 |
| [L1073](../src/webrtc/WebRtcRuntime.cpp#L1073) | `WebRtcRuntime::SelectRecordingDevice` | 定义 | `AudioDeviceOperationResult WebRtcRuntime::SelectRecordingDevice( const std::string& deviceId)` | 查询并返回 select recording device 相关逻辑。 |
| [L1181](../src/webrtc/WebRtcRuntime.cpp#L1181) | `WebRtcRuntime::SetRecordingEnabled` | 定义 | `AudioDeviceOperationResult WebRtcRuntime::SetRecordingEnabled(bool enabled)` | 更新或应用 set recording enabled 相关逻辑。 |
| [L1234](../src/webrtc/WebRtcRuntime.cpp#L1234) | `WebRtcRuntime::SelectPlayoutDevice` | 定义 | `AudioDeviceOperationResult WebRtcRuntime::SelectPlayoutDevice( const std::string& deviceId)` | 查询并返回 select playout device 相关逻辑。 |
| [L1342](../src/webrtc/WebRtcRuntime.cpp#L1342) | `WebRtcRuntime::ReapplyPreferredAudioDevices` | 定义 | `WebRtcRuntime::ReapplyPreferredAudioDevices()` | 实现 reapply preferred audio devices 对应的业务或工具逻辑。 |
| [L1391](../src/webrtc/WebRtcRuntime.cpp#L1391) | `WebRtcRuntime::PeerConnectionFactory` | 定义 | `WebRtcRuntime::PeerConnectionFactory() const` | 实现 peer connection factory 对应的业务或工具逻辑。 |

## `src/webrtc/WebRtcRuntime.h`

[打开源码](../src/webrtc/WebRtcRuntime.h) · **文件作用：** 声明 web rtc runtime 相关类型、接口、配置和成员状态。

### 类型

| 行 | 类型 | 种类 | 作用 |
|---:|---|---|---|
| [L20](../src/webrtc/WebRtcRuntime.h#L20) | `Thread` | class | 定义 Thread 的 class 类型和相关状态。 |
| [L21](../src/webrtc/WebRtcRuntime.h#L21) | `WinsockInitializer` | class | 定义 WinsockInitializer 的 class 类型和相关状态。 |
| [L26](../src/webrtc/WebRtcRuntime.h#L26) | `MfD3D11H264DecoderPreferenceState` | class | 定义 MfD3D11H264DecoderPreferenceState 的 class 类型和相关状态。 |
| [L27](../src/webrtc/WebRtcRuntime.h#L27) | `WindowsPreferredVideoEncoderFactory` | class | 定义 WindowsPreferredVideoEncoderFactory 的 class 类型和相关状态。 |
| [L28](../src/webrtc/WebRtcRuntime.h#L28) | `WindowsPreferredVideoDecoderFactory` | class | 定义 WindowsPreferredVideoDecoderFactory 的 class 类型和相关状态。 |
| [L30](../src/webrtc/WebRtcRuntime.h#L30) | `WebRtcCapabilityReport` | struct | 定义 WebRtcCapabilityReport 的 struct 类型和相关状态。 |
| [L74](../src/webrtc/WebRtcRuntime.h#L74) | `AudioDeviceEnumeration` | struct | 定义 AudioDeviceEnumeration 的 struct 类型和相关状态。 |
| [L84](../src/webrtc/WebRtcRuntime.h#L84) | `AudioDeviceOperationResult` | struct | 定义 AudioDeviceOperationResult 的 struct 类型和相关状态。 |
| [L91](../src/webrtc/WebRtcRuntime.h#L91) | `WebRtcRuntime` | class | 定义 WebRtcRuntime 的 class 类型和相关状态。 |

### 成员与文件级变量

| 行 | 变量 | 声明 | 作用 |
|---:|---|---|---|
| [L20](../src/webrtc/WebRtcRuntime.h#L20) | `Thread` | `class Thread;` | 拥有后台执行线程或工作器：thread。 |
| [L21](../src/webrtc/WebRtcRuntime.h#L21) | `WinsockInitializer` | `class WinsockInitializer;` | 保存 winsock initializer 相关配置或运行状态。 |
| [L26](../src/webrtc/WebRtcRuntime.h#L26) | `MfD3D11H264DecoderPreferenceState` | `class MfD3D11H264DecoderPreferenceState;` | 保存状态机当前状态：mf d3 d11 h264 decoder preference state。 |
| [L27](../src/webrtc/WebRtcRuntime.h#L27) | `WindowsPreferredVideoEncoderFactory` | `class WindowsPreferredVideoEncoderFactory;` | 保存 windows preferred video encoder factory 相关配置或运行状态。 |
| [L28](../src/webrtc/WebRtcRuntime.h#L28) | `WindowsPreferredVideoDecoderFactory` | `class WindowsPreferredVideoDecoderFactory;` | 保存 windows preferred video decoder factory 相关配置或运行状态。 |
| [L31](../src/webrtc/WebRtcRuntime.h#L31) | `factoryCreated` | `bool factoryCreated = false;` | 保存 factory created 相关配置或运行状态。 |
| [L32](../src/webrtc/WebRtcRuntime.h#L32) | `windowsPreferredEncoderFactoryCreated` | `bool windowsPreferredEncoderFactoryCreated = false;` | 保存 windows preferred encoder factory created 相关配置或运行状态。 |
| [L33](../src/webrtc/WebRtcRuntime.h#L33) | `windowsPreferredDecoderFactoryCreated` | `bool windowsPreferredDecoderFactoryCreated = false;` | 保存 windows preferred decoder factory created 相关配置或运行状态。 |
| [L34](../src/webrtc/WebRtcRuntime.h#L34) | `hasH264Encoder` | `bool hasH264Encoder = false;` | 保存 has h264 encoder 相关配置或运行状态。 |
| [L35](../src/webrtc/WebRtcRuntime.h#L35) | `hasH264Decoder` | `bool hasH264Decoder = false;` | 保存 has h264 decoder 相关配置或运行状态。 |
| [L36](../src/webrtc/WebRtcRuntime.h#L36) | `h264HardwareEncoderAvailable` | `bool h264HardwareEncoderAvailable = false;` | 保存能力或开关状态：h264 hardware encoder available。 |
| [L37](../src/webrtc/WebRtcRuntime.h#L37) | `h264HardwareEncoderCpuNv12InputSupported` | `bool h264HardwareEncoderCpuNv12InputSupported = false;` | 保存 h264 hardware encoder cpu nv12 input supported 相关配置或运行状态。 |
| [L38](../src/webrtc/WebRtcRuntime.h#L38) | `h264HardwareEncoderD3D11InputCandidate` | `bool h264HardwareEncoderD3D11InputCandidate = false;` | 保存 h264 hardware encoder d3 d11 input candidate 相关配置或运行状态。 |
| [L39](../src/webrtc/WebRtcRuntime.h#L39) | `h264HardwareEncoderCount` | `uint32_t h264HardwareEncoderCount = 0;` | 保存计数、尺寸或速率指标：h264 hardware encoder count。 |
| [L40](../src/webrtc/WebRtcRuntime.h#L40) | `h264HardwareEncoderWired` | `bool h264HardwareEncoderWired = false;` | 保存 h264 hardware encoder wired 相关配置或运行状态。 |
| [L41](../src/webrtc/WebRtcRuntime.h#L41) | `h264SoftwareEncoderWired` | `bool h264SoftwareEncoderWired = false;` | 保存 h264 software encoder wired 相关配置或运行状态。 |
| [L42](../src/webrtc/WebRtcRuntime.h#L42) | `h264FfmpegX264EncoderWired` | `bool h264FfmpegX264EncoderWired = false;` | 保存 h264 ffmpeg x264 encoder wired 相关配置或运行状态。 |
| [L43](../src/webrtc/WebRtcRuntime.h#L43) | `h264FfmpegX264EncoderError` | `std::string h264FfmpegX264EncoderError;` | 保存最近错误或失败原因：h264 ffmpeg x264 encoder error。 |
| [L44](../src/webrtc/WebRtcRuntime.h#L44) | `h264FfmpegHardwareEncoderAvailable` | `bool h264FfmpegHardwareEncoderAvailable = false;` | 保存能力或开关状态：h264 ffmpeg hardware encoder available。 |
| [L45](../src/webrtc/WebRtcRuntime.h#L45) | `h264FfmpegHardwareEncoderWired` | `bool h264FfmpegHardwareEncoderWired = false;` | 保存 h264 ffmpeg hardware encoder wired 相关配置或运行状态。 |
| [L46](../src/webrtc/WebRtcRuntime.h#L46) | `h264FfmpegHardwareEncoderError` | `std::string h264FfmpegHardwareEncoderError;` | 保存最近错误或失败原因：h264 ffmpeg hardware encoder error。 |
| [L47](../src/webrtc/WebRtcRuntime.h#L47) | `h264FfmpegHardwareEncoderDescriptions` | `std::vector<std::string> h264FfmpegHardwareEncoderDescriptions;` | 保存 h264 ffmpeg hardware encoder descriptions 相关配置或运行状态。 |
| [L48](../src/webrtc/WebRtcRuntime.h#L48) | `h264SoftwareEncoderFallbackWired` | `bool h264SoftwareEncoderFallbackWired = false;` | 保存 h264 software encoder fallback wired 相关配置或运行状态。 |
| [L49](../src/webrtc/WebRtcRuntime.h#L49) | `h264MfDecoderWired` | `bool h264MfDecoderWired = false;` | 保存 h264 mf decoder wired 相关配置或运行状态。 |
| [L50](../src/webrtc/WebRtcRuntime.h#L50) | `h264MfDecoderConfigured` | `bool h264MfDecoderConfigured = false;` | 保存 h264 mf decoder configured 相关配置或运行状态。 |
| [L51](../src/webrtc/WebRtcRuntime.h#L51) | `h264MfDecoderHardware` | `bool h264MfDecoderHardware = false;` | 保存 h264 mf decoder hardware 相关配置或运行状态。 |
| [L52](../src/webrtc/WebRtcRuntime.h#L52) | `h264MfDecoderSoftware` | `bool h264MfDecoderSoftware = false;` | 保存 h264 mf decoder software 相关配置或运行状态。 |
| [L53](../src/webrtc/WebRtcRuntime.h#L53) | `h264MfDecoderNativeOutputSupported` | `bool h264MfDecoderNativeOutputSupported = false;` | 保存 h264 mf decoder native output supported 相关配置或运行状态。 |
| [L54](../src/webrtc/WebRtcRuntime.h#L54) | `h264MfDecoderAsynchronous` | `bool h264MfDecoderAsynchronous = false;` | 保存 h264 mf decoder asynchronous 相关配置或运行状态。 |
| [L55](../src/webrtc/WebRtcRuntime.h#L55) | `h264FfmpegSoftwareDecoderWired` | `bool h264FfmpegSoftwareDecoderWired = false;` | 保存 h264 ffmpeg software decoder wired 相关配置或运行状态。 |
| [L56](../src/webrtc/WebRtcRuntime.h#L56) | `h264MfDecoderName` | `std::string h264MfDecoderName;` | 保存路径、地址或显示名称：h264 mf decoder name。 |
| [L57](../src/webrtc/WebRtcRuntime.h#L57) | `h264MfDecoderError` | `std::string h264MfDecoderError;` | 保存最近错误或失败原因：h264 mf decoder error。 |
| [L58](../src/webrtc/WebRtcRuntime.h#L58) | `h264HardwareEncoderDescriptions` | `std::vector<std::string> h264HardwareEncoderDescriptions;` | 保存 h264 hardware encoder descriptions 相关配置或运行状态。 |
| [L59](../src/webrtc/WebRtcRuntime.h#L59) | `h264HardwareEncoderWarnings` | `std::vector<std::string> h264HardwareEncoderWarnings;` | 保存 h264 hardware encoder warnings 相关配置或运行状态。 |
| [L60](../src/webrtc/WebRtcRuntime.h#L60) | `videoEncoderFormats` | `std::vector<std::string> videoEncoderFormats;` | 保存 video encoder formats 相关配置或运行状态。 |
| [L61](../src/webrtc/WebRtcRuntime.h#L61) | `videoDecoderFormats` | `std::vector<std::string> videoDecoderFormats;` | 保存 video decoder formats 相关配置或运行状态。 |
| [L62](../src/webrtc/WebRtcRuntime.h#L62) | `error` | `std::string error;` | 保存最近错误或失败原因：error。 |
| [L63](../src/webrtc/WebRtcRuntime.h#L63) | `hardwareFingerprint` | `std::string hardwareFingerprint;` | 保存 hardware fingerprint 相关配置或运行状态。 |
| [L64](../src/webrtc/WebRtcRuntime.h#L64) | `h264HardwareEncoderProbeSucceeded` | `bool h264HardwareEncoderProbeSucceeded = false;` | 保存 h264 hardware encoder probe succeeded 相关配置或运行状态。 |
| [L65](../src/webrtc/WebRtcRuntime.h#L65) | `h264HardwareEncoderProbeFromCache` | `bool h264HardwareEncoderProbeFromCache = false;` | 保存 h264 hardware encoder probe from cache 相关配置或运行状态。 |
| [L66](../src/webrtc/WebRtcRuntime.h#L66) | `audioDeviceModuleCreated` | `bool audioDeviceModuleCreated = false;` | 保存 audio device module created 相关配置或运行状态。 |
| [L67](../src/webrtc/WebRtcRuntime.h#L67) | `audioDeviceError` | `std::string audioDeviceError;` | 保存最近错误或失败原因：audio device error。 |
| [L69](../src/webrtc/WebRtcRuntime.h#L69) | `kAutomatic` | `VideoEncoderPreference::kAutomatic;` | 定义 automatic 的编译期常量或产品边界。 |
| [L71](../src/webrtc/WebRtcRuntime.h#L71) | `kAutomatic` | `VideoDecoderPreference::kAutomatic;` | 定义 automatic 的编译期常量或产品边界。 |
| [L75](../src/webrtc/WebRtcRuntime.h#L75) | `microphones` | `std::vector<MediaDeviceDescriptor> microphones;` | 保存 microphones 相关配置或运行状态。 |
| [L76](../src/webrtc/WebRtcRuntime.h#L76) | `speakers` | `std::vector<MediaDeviceDescriptor> speakers;` | 保存 speakers 相关配置或运行状态。 |
| [L77](../src/webrtc/WebRtcRuntime.h#L77) | `activeMicrophoneId` | `std::string activeMicrophoneId;` | 保存身份或作用域标识：active microphone id。 |
| [L78](../src/webrtc/WebRtcRuntime.h#L78) | `activeMicrophoneName` | `std::string activeMicrophoneName;` | 保存路径、地址或显示名称：active microphone name。 |
| [L79](../src/webrtc/WebRtcRuntime.h#L79) | `activeSpeakerId` | `std::string activeSpeakerId;` | 保存身份或作用域标识：active speaker id。 |
| [L80](../src/webrtc/WebRtcRuntime.h#L80) | `activeSpeakerName` | `std::string activeSpeakerName;` | 保存路径、地址或显示名称：active speaker name。 |
| [L81](../src/webrtc/WebRtcRuntime.h#L81) | `error` | `std::string error;` | 保存最近错误或失败原因：error。 |
| [L85](../src/webrtc/WebRtcRuntime.h#L85) | `succeeded` | `bool succeeded = false;` | 保存 succeeded 相关配置或运行状态。 |
| [L86](../src/webrtc/WebRtcRuntime.h#L86) | `activeDeviceId` | `std::string activeDeviceId;` | 保存身份或作用域标识：active device id。 |
| [L87](../src/webrtc/WebRtcRuntime.h#L87) | `errorCode` | `std::string errorCode;` | 保存 error code 相关配置或运行状态。 |
| [L88](../src/webrtc/WebRtcRuntime.h#L88) | `errorMessage` | `std::string errorMessage;` | 保存 error message 相关配置或运行状态。 |
| [L142](../src/webrtc/WebRtcRuntime.h#L142) | `networkThread_` | `std::unique_ptr<webrtc::Thread> networkThread_;` | 拥有后台执行线程或工作器：network thread。 |
| [L143](../src/webrtc/WebRtcRuntime.h#L143) | `workerThread_` | `std::unique_ptr<webrtc::Thread> workerThread_;` | 拥有后台执行线程或工作器：worker thread。 |
| [L144](../src/webrtc/WebRtcRuntime.h#L144) | `signalingThread_` | `std::unique_ptr<webrtc::Thread> signalingThread_;` | 拥有后台执行线程或工作器：signaling thread。 |
| [L145](../src/webrtc/WebRtcRuntime.h#L145) | `winsockInitializer_` | `std::unique_ptr<webrtc::WinsockInitializer> winsockInitializer_;` | 保存 winsock initializer 相关配置或运行状态。 |
| [L147](../src/webrtc/WebRtcRuntime.h#L147) | `networkThreadStarted_` | `bool networkThreadStarted_ = false;` | 保存 network thread started 相关配置或运行状态。 |
| [L148](../src/webrtc/WebRtcRuntime.h#L148) | `workerThreadStarted_` | `bool workerThreadStarted_ = false;` | 保存 worker thread started 相关配置或运行状态。 |
| [L149](../src/webrtc/WebRtcRuntime.h#L149) | `signalingThreadStarted_` | `bool signalingThreadStarted_ = false;` | 保存 signaling thread started 相关配置或运行状态。 |
| [L150](../src/webrtc/WebRtcRuntime.h#L150) | `sslInitialized_` | `bool sslInitialized_ = false;` | 保存 ssl initialized 相关配置或运行状态。 |
| [L152](../src/webrtc/WebRtcRuntime.h#L152) | `factory_` | `webrtc::scoped_refptr<webrtc::PeerConnectionFactoryInterface> factory_;` | 保存 factory 相关配置或运行状态。 |
| [L155](../src/webrtc/WebRtcRuntime.h#L155) | `videoEncoderFactory_` | `WindowsPreferredVideoEncoderFactory* videoEncoderFactory_ = nullptr;` | Owned by factory_. These non-owning pointers remain valid until factory_ is released in Shutdown(). |
| [L156](../src/webrtc/WebRtcRuntime.h#L156) | `videoDecoderFactory_` | `WindowsPreferredVideoDecoderFactory* videoDecoderFactory_ = nullptr;` | 保存 video decoder factory 相关配置或运行状态。 |
| [L157](../src/webrtc/WebRtcRuntime.h#L157) | `audioDeviceModule_` | `webrtc::scoped_refptr<webrtc::AudioDeviceModule> audioDeviceModule_;` | 保存 audio device module 相关配置或运行状态。 |
| [L158](../src/webrtc/WebRtcRuntime.h#L158) | `report_` | `WebRtcCapabilityReport report_;` | 保存 report 相关配置或运行状态。 |
| [L159](../src/webrtc/WebRtcRuntime.h#L159) | `videoEncoderPreference_` | `VideoEncoderPreference videoEncoderPreference_;` | 保存 video encoder preference 相关配置或运行状态。 |
| [L160](../src/webrtc/WebRtcRuntime.h#L160) | `ffmpegX264Preset_` | `FfmpegX264Preset ffmpegX264Preset_ = FfmpegX264Preset::kMedium;` | 保存 ffmpeg x264 preset 相关配置或运行状态。 |
| [L162](../src/webrtc/WebRtcRuntime.h#L162) | `kAutomatic` | `FfmpegHardwareBackend::kAutomatic;` | 定义 automatic 的编译期常量或产品边界。 |
| [L163](../src/webrtc/WebRtcRuntime.h#L163) | `preferredAutomaticEncoderId_` | `std::string preferredAutomaticEncoderId_;` | 保存身份或作用域标识：preferred automatic encoder id。 |
| [L164](../src/webrtc/WebRtcRuntime.h#L164) | `videoDecoderPreference_` | `VideoDecoderPreference videoDecoderPreference_;` | 保存 video decoder preference 相关配置或运行状态。 |
| [L165](../src/webrtc/WebRtcRuntime.h#L165) | `encoderRuntimeState_` | `std::shared_ptr<VideoEncoderRuntimeState> encoderRuntimeState_;` | 保存状态机当前状态：encoder runtime state。 |
| [L167](../src/webrtc/WebRtcRuntime.h#L167) | `decoderPreferenceState_` | `decoderPreferenceState_;` | 保存状态机当前状态：decoder preference state。 |
| [L168](../src/webrtc/WebRtcRuntime.h#L168) | `hardwareFingerprint_` | `std::string hardwareFingerprint_;` | 保存 hardware fingerprint 相关配置或运行状态。 |
| [L170](../src/webrtc/WebRtcRuntime.h#L170) | `encoderCapabilityCache_` | `encoderCapabilityCache_;` | 保存 encoder capability cache 相关配置或运行状态。 |
| [L171](../src/webrtc/WebRtcRuntime.h#L171) | `preferredRecordingDeviceId_` | `std::string preferredRecordingDeviceId_;` | 保存身份或作用域标识：preferred recording device id。 |
| [L172](../src/webrtc/WebRtcRuntime.h#L172) | `preferredPlayoutDeviceId_` | `std::string preferredPlayoutDeviceId_;` | 保存身份或作用域标识：preferred playout device id。 |
| [L173](../src/webrtc/WebRtcRuntime.h#L173) | `activeRecordingDeviceId_` | `std::string activeRecordingDeviceId_;` | 保存身份或作用域标识：active recording device id。 |
| [L174](../src/webrtc/WebRtcRuntime.h#L174) | `activePlayoutDeviceId_` | `std::string activePlayoutDeviceId_;` | 保存身份或作用域标识：active playout device id。 |

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L111](../src/webrtc/WebRtcRuntime.h#L111) | `~WebRtcRuntime` | 声明 | `~WebRtcRuntime()` | 停止相关活动并释放 WebRtcRuntime 实例拥有的资源。 |
| [L113](../src/webrtc/WebRtcRuntime.h#L113) | `WebRtcRuntime` | 声明 | `WebRtcRuntime(const WebRtcRuntime&) = delete` | 实现 web rtc runtime 对应的业务或工具逻辑。 |
| [L116](../src/webrtc/WebRtcRuntime.h#L116) | `Initialize` | 声明 | `bool Initialize()` | 创建或初始化 initialize 相关逻辑。 |
| [L117](../src/webrtc/WebRtcRuntime.h#L117) | `Shutdown` | 声明 | `void Shutdown()` | 关闭并清理 shutdown 相关逻辑。 |
| [L119](../src/webrtc/WebRtcRuntime.h#L119) | `CapabilityReport` | 声明 | `const WebRtcCapabilityReport& CapabilityReport() const noexcept` | 实现 capability report 对应的业务或工具逻辑。 |
| [L120](../src/webrtc/WebRtcRuntime.h#L120) | `EncoderRuntimeStatus` | 声明 | `VideoEncoderRuntimeSnapshot EncoderRuntimeStatus() const` | 编码 encoder runtime status 相关逻辑。 |
| [L121](../src/webrtc/WebRtcRuntime.h#L121) | `SetDesktopShareGeneration` | 声明 | `void SetDesktopShareGeneration(std::uint64_t generation)` | 更新或应用 set desktop share generation 相关逻辑。 |
| [L122](../src/webrtc/WebRtcRuntime.h#L122) | `SetDesktopCaptureAdapterLuid` | 声明 | `void SetDesktopCaptureAdapterLuid(std::uint64_t adapterLuid)` | 更新或应用 set desktop capture adapter luid 相关逻辑。 |
| [L123](../src/webrtc/WebRtcRuntime.h#L123) | `SetPreferredHardwareDecoderName` | 声明 | `void SetPreferredHardwareDecoderName(std::string name)` | 更新或应用 set preferred hardware decoder name 相关逻辑。 |
| [L124](../src/webrtc/WebRtcRuntime.h#L124) | `ApplyVideoCodecPreferences` | 声明 | `bool ApplyVideoCodecPreferences( VideoEncoderPreference videoEncoderPreference, FfmpegX264Preset quality, FfmpegHardwareBackend ffmpegHardwareBackend, std::string preferredAutomaticEncoderId, VideoDecoderPreference vi...` | 更新或应用 apply video codec preferences 相关逻辑。 |
| [L131](../src/webrtc/WebRtcRuntime.h#L131) | `EnumerateAudioDevices` | 声明 | `AudioDeviceEnumeration EnumerateAudioDevices() const` | 实现 enumerate audio devices 对应的业务或工具逻辑。 |
| [L132](../src/webrtc/WebRtcRuntime.h#L132) | `SelectRecordingDevice` | 声明 | `AudioDeviceOperationResult SelectRecordingDevice( const std::string& deviceId)` | 查询并返回 select recording device 相关逻辑。 |
| [L134](../src/webrtc/WebRtcRuntime.h#L134) | `SetRecordingEnabled` | 声明 | `AudioDeviceOperationResult SetRecordingEnabled(bool enabled)` | 更新或应用 set recording enabled 相关逻辑。 |
| [L135](../src/webrtc/WebRtcRuntime.h#L135) | `SelectPlayoutDevice` | 声明 | `AudioDeviceOperationResult SelectPlayoutDevice( const std::string& deviceId)` | 查询并返回 select playout device 相关逻辑。 |
| [L137](../src/webrtc/WebRtcRuntime.h#L137) | `ReapplyPreferredAudioDevices` | 声明 | `AudioDeviceOperationResult ReapplyPreferredAudioDevices()` | 实现 reapply preferred audio devices 对应的业务或工具逻辑。 |
| [L139](../src/webrtc/WebRtcRuntime.h#L139) | `PeerConnectionFactory` | 声明 | `PeerConnectionFactory() const` | 实现 peer connection factory 对应的业务或工具逻辑。 |

## `src/webrtc/WindowsPreferredVideoDecoderFactory.cpp`

[打开源码](../src/webrtc/WindowsPreferredVideoDecoderFactory.cpp) · **文件作用：** 实现 windows preferred video decoder factory 相关函数与文件级辅助逻辑。

### 类型

| 行 | 类型 | 种类 | 作用 |
|---:|---|---|---|
| [L19](../src/webrtc/WindowsPreferredVideoDecoderFactory.cpp#L19) | `FrameTimedVideoDecoder` | class | 定义 FrameTimedVideoDecoder 的 class 类型和相关状态。 |
| [L207](../src/webrtc/WindowsPreferredVideoDecoderFactory.cpp#L207) | `WindowsPreferredVideoDecoderFactoryState` | class | 定义 WindowsPreferredVideoDecoderFactoryState 的 class 类型和相关状态。 |
| [L337](../src/webrtc/WindowsPreferredVideoDecoderFactory.cpp#L337) | `ReconfigurableVideoDecoder` | class | 定义 ReconfigurableVideoDecoder 的 class 类型和相关状态。 |

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L23](../src/webrtc/WindowsPreferredVideoDecoderFactory.cpp#L23) | `FrameTimedVideoDecoder` | 定义 | `explicit FrameTimedVideoDecoder( std::unique_ptr<webrtc::VideoDecoder> decoder) : decoder_(std::move(decoder)), timingInstanceId_( VideoCodecTimingRegistry::Instance().Register( VideoCodecTimingDirection::kDecode)) {}` | 实现 frame timed video decoder 对应的业务或工具逻辑。 |
| [L31](../src/webrtc/WindowsPreferredVideoDecoderFactory.cpp#L31) | `~FrameTimedVideoDecoder` | 定义 | `~FrameTimedVideoDecoder() override` | 停止相关活动并释放 FrameTimedVideoDecoder 实例拥有的资源。 |
| [L37](../src/webrtc/WindowsPreferredVideoDecoderFactory.cpp#L37) | `Configure` | 定义 | `bool Configure(const Settings& settings) override` | 更新或应用 configure 相关逻辑。 |
| [L43](../src/webrtc/WindowsPreferredVideoDecoderFactory.cpp#L43) | `Decode` | 定义 | `int32_t Decode( const webrtc::EncodedImage& inputImage, int64_t renderTimeMs) override` | 解码或解析 decode 相关逻辑。 |
| [L68](../src/webrtc/WindowsPreferredVideoDecoderFactory.cpp#L68) | `RegisterDecodeCompleteCallback` | 定义 | `int32_t RegisterDecodeCompleteCallback( webrtc::DecodedImageCallback* callback) override` | 实现 register decode complete callback 对应的业务或工具逻辑。 |
| [L79](../src/webrtc/WindowsPreferredVideoDecoderFactory.cpp#L79) | `Release` | 定义 | `int32_t Release() override` | 释放或取消 release 相关逻辑。 |
| [L89](../src/webrtc/WindowsPreferredVideoDecoderFactory.cpp#L89) | `GetDecoderInfo` | 定义 | `DecoderInfo GetDecoderInfo() const override` | 查询并返回 get decoder info 相关逻辑。 |
| [L99](../src/webrtc/WindowsPreferredVideoDecoderFactory.cpp#L99) | `ImplementationName` | 定义 | `const char* ImplementationName() const override` | 实现 implementation name 对应的业务或工具逻辑。 |
| [L106](../src/webrtc/WindowsPreferredVideoDecoderFactory.cpp#L106) | `Decoded` | 定义 | `int32_t Decoded(webrtc::VideoFrame& decodedImage) override` | 解码或解析 decoded 相关逻辑。 |
| [L115](../src/webrtc/WindowsPreferredVideoDecoderFactory.cpp#L115) | `Decoded` | 定义 | `int32_t Decoded( webrtc::VideoFrame& decodedImage, int64_t decodeTimeMs) override` | 解码或解析 decoded 相关逻辑。 |
| [L126](../src/webrtc/WindowsPreferredVideoDecoderFactory.cpp#L126) | `Decoded` | 定义 | `void Decoded( webrtc::VideoFrame& decodedImage, std::optional<int32_t> decodeTimeMs, std::optional<uint8_t> qp) override` | 解码或解析 decoded 相关逻辑。 |
| [L141](../src/webrtc/WindowsPreferredVideoDecoderFactory.cpp#L141) | `Callback` | 定义 | `webrtc::DecodedImageCallback* Callback() const` | 实现 callback 对应的业务或工具逻辑。 |
| [L147](../src/webrtc/WindowsPreferredVideoDecoderFactory.cpp#L147) | `ClearPending` | 定义 | `void ClearPending()` | 重置或移除 clear pending 相关逻辑。 |
| [L153](../src/webrtc/WindowsPreferredVideoDecoderFactory.cpp#L153) | `RecordCompleted` | 定义 | `void RecordCompleted( const webrtc::VideoFrame& decodedImage, std::optional<std::int32_t> qp)` | 实现 record completed 对应的业务或工具逻辑。 |
| [L194](../src/webrtc/WindowsPreferredVideoDecoderFactory.cpp#L194) | `AppendUniqueFormats` | 定义 | `void AppendUniqueFormats( std::vector<webrtc::SdpVideoFormat>& destination, const std::vector<webrtc::SdpVideoFormat>& source)` | 实现 append unique formats 对应的业务或工具逻辑。 |
| [L209](../src/webrtc/WindowsPreferredVideoDecoderFactory.cpp#L209) | `WindowsPreferredVideoDecoderFactoryState` | 定义 | `WindowsPreferredVideoDecoderFactoryState( std::unique_ptr<webrtc::VideoDecoderFactory> softwareFactory, std::unique_ptr<webrtc::VideoDecoderFactory> hardwareFactory, bool preferSoftware) : softwareFactory_(std::move(s...` | 实现 windows preferred video decoder factory state 对应的业务或工具逻辑。 |
| [L218](../src/webrtc/WindowsPreferredVideoDecoderFactory.cpp#L218) | `GetSupportedFormats` | 定义 | `std::vector<webrtc::SdpVideoFormat> GetSupportedFormats() const` | 查询并返回 get supported formats 相关逻辑。 |
| [L237](../src/webrtc/WindowsPreferredVideoDecoderFactory.cpp#L237) | `QueryCodecSupport` | 定义 | `webrtc::VideoDecoderFactory::CodecSupport QueryCodecSupport( const webrtc::SdpVideoFormat& format, bool referenceScaling, std::optional<webrtc::Resolution> resolution) const` | 发起请求或查询 query codec support 相关逻辑。 |
| [L263](../src/webrtc/WindowsPreferredVideoDecoderFactory.cpp#L263) | `CreateSelected` | 定义 | `std::unique_ptr<webrtc::VideoDecoder> CreateSelected( const webrtc::Environment& environment, const webrtc::SdpVideoFormat& format, std::uint64_t* revision) const` | 创建或初始化 create selected 相关逻辑。 |
| [L290](../src/webrtc/WindowsPreferredVideoDecoderFactory.cpp#L290) | `Revision` | 定义 | `std::uint64_t Revision() const` | 实现 revision 对应的业务或工具逻辑。 |
| [L296](../src/webrtc/WindowsPreferredVideoDecoderFactory.cpp#L296) | `HasHardwareDecoderFor` | 定义 | `bool HasHardwareDecoderFor( const webrtc::SdpVideoFormat& format) const` | 判断 has hardware decoder for 相关逻辑。 |
| [L304](../src/webrtc/WindowsPreferredVideoDecoderFactory.cpp#L304) | `HasSoftwareDecoderFor` | 定义 | `bool HasSoftwareDecoderFor( const webrtc::SdpVideoFormat& format) const` | 判断 has software decoder for 相关逻辑。 |
| [L312](../src/webrtc/WindowsPreferredVideoDecoderFactory.cpp#L312) | `SetPreferSoftware` | 定义 | `void SetPreferSoftware(bool preferSoftware)` | 更新或应用 set prefer software 相关逻辑。 |
| [L321](../src/webrtc/WindowsPreferredVideoDecoderFactory.cpp#L321) | `NotifyConfigurationChanged` | 定义 | `void NotifyConfigurationChanged()` | 通知或报告 notify configuration changed 相关逻辑。 |
| [L339](../src/webrtc/WindowsPreferredVideoDecoderFactory.cpp#L339) | `ReconfigurableVideoDecoder` | 定义 | `ReconfigurableVideoDecoder( std::shared_ptr<WindowsPreferredVideoDecoderFactoryState> state, webrtc::Environment environment, webrtc::SdpVideoFormat format) : state_(std::move(state)), environment_(std::move(environme...` | 实现 reconfigurable video decoder 对应的业务或工具逻辑。 |
| [L350](../src/webrtc/WindowsPreferredVideoDecoderFactory.cpp#L350) | `IsValid` | 定义 | `bool IsValid() const noexcept { return decoder_ != nullptr; }` | 判断 is valid 相关逻辑。 |
| [L352](../src/webrtc/WindowsPreferredVideoDecoderFactory.cpp#L352) | `Configure` | 定义 | `bool Configure(const Settings& settings) override` | 更新或应用 configure 相关逻辑。 |
| [L362](../src/webrtc/WindowsPreferredVideoDecoderFactory.cpp#L362) | `Decode` | 定义 | `int32_t Decode( const webrtc::EncodedImage& inputImage, int64_t renderTimeMs) override` | 解码或解析 decode 相关逻辑。 |
| [L371](../src/webrtc/WindowsPreferredVideoDecoderFactory.cpp#L371) | `RegisterDecodeCompleteCallback` | 定义 | `int32_t RegisterDecodeCompleteCallback( webrtc::DecodedImageCallback* callback) override` | 实现 register decode complete callback 对应的业务或工具逻辑。 |
| [L380](../src/webrtc/WindowsPreferredVideoDecoderFactory.cpp#L380) | `Release` | 定义 | `int32_t Release() override` | 释放或取消 release 相关逻辑。 |
| [L387](../src/webrtc/WindowsPreferredVideoDecoderFactory.cpp#L387) | `GetDecoderInfo` | 定义 | `DecoderInfo GetDecoderInfo() const override` | 查询并返回 get decoder info 相关逻辑。 |
| [L392](../src/webrtc/WindowsPreferredVideoDecoderFactory.cpp#L392) | `ImplementationName` | 定义 | `const char* ImplementationName() const override` | 实现 implementation name 对应的业务或工具逻辑。 |
| [L399](../src/webrtc/WindowsPreferredVideoDecoderFactory.cpp#L399) | `EnsureCurrent` | 定义 | `bool EnsureCurrent(bool configure)` | 实现 ensure current 对应的业务或工具逻辑。 |
| [L447](../src/webrtc/WindowsPreferredVideoDecoderFactory.cpp#L447) | `WindowsPreferredVideoDecoderFactory::WindowsPreferredVideoDecoderFactory` | 定义 | `WindowsPreferredVideoDecoderFactory::WindowsPreferredVideoDecoderFactory( std::unique_ptr<webrtc::VideoDecoderFactory> softwareFactory, std::unique_ptr<webrtc::VideoDecoderFactory> hardwareFactory, bool preferSoftware...` | 构造并初始化 WindowsPreferredVideoDecoderFactory 实例。 |
| [L460](../src/webrtc/WindowsPreferredVideoDecoderFactory.cpp#L460) | `WindowsPreferredVideoDecoderFactory::GetSupportedFormats` | 定义 | `WindowsPreferredVideoDecoderFactory::GetSupportedFormats() const` | 查询并返回 get supported formats 相关逻辑。 |
| [L466](../src/webrtc/WindowsPreferredVideoDecoderFactory.cpp#L466) | `WindowsPreferredVideoDecoderFactory::QueryCodecSupport` | 定义 | `WindowsPreferredVideoDecoderFactory::QueryCodecSupport( const webrtc::SdpVideoFormat& format, bool referenceScaling, std::optional<webrtc::Resolution> resolution) const` | 发起请求或查询 query codec support 相关逻辑。 |
| [L476](../src/webrtc/WindowsPreferredVideoDecoderFactory.cpp#L476) | `WindowsPreferredVideoDecoderFactory::Create` | 定义 | `WindowsPreferredVideoDecoderFactory::Create( const webrtc::Environment& environment, const webrtc::SdpVideoFormat& format)` | 创建或初始化 create 相关逻辑。 |
| [L488](../src/webrtc/WindowsPreferredVideoDecoderFactory.cpp#L488) | `WindowsPreferredVideoDecoderFactory::SetPreferSoftware` | 定义 | `void WindowsPreferredVideoDecoderFactory::SetPreferSoftware( bool preferSoftware) noexcept` | 更新或应用 set prefer software 相关逻辑。 |
| [L494](../src/webrtc/WindowsPreferredVideoDecoderFactory.cpp#L494) | `WindowsPreferredVideoDecoderFactory::NotifyConfigurationChanged` | 定义 | `void WindowsPreferredVideoDecoderFactory::NotifyConfigurationChanged() noexcept` | 通知或报告 notify configuration changed 相关逻辑。 |
| [L500](../src/webrtc/WindowsPreferredVideoDecoderFactory.cpp#L500) | `WindowsPreferredVideoDecoderFactory::HasHardwareDecoderFor` | 定义 | `bool WindowsPreferredVideoDecoderFactory::HasHardwareDecoderFor( const webrtc::SdpVideoFormat& format) const` | 判断 has hardware decoder for 相关逻辑。 |
| [L506](../src/webrtc/WindowsPreferredVideoDecoderFactory.cpp#L506) | `WindowsPreferredVideoDecoderFactory::HasSoftwareDecoderFor` | 定义 | `bool WindowsPreferredVideoDecoderFactory::HasSoftwareDecoderFor( const webrtc::SdpVideoFormat& format) const` | 判断 has software decoder for 相关逻辑。 |

## `src/webrtc/WindowsPreferredVideoDecoderFactory.h`

[打开源码](../src/webrtc/WindowsPreferredVideoDecoderFactory.h) · **文件作用：** 声明 windows preferred video decoder factory 相关类型、接口、配置和成员状态。

### 类型

| 行 | 类型 | 种类 | 作用 |
|---:|---|---|---|
| [L16](../src/webrtc/WindowsPreferredVideoDecoderFactory.h#L16) | `WindowsPreferredVideoDecoderFactoryState` | class | 定义 WindowsPreferredVideoDecoderFactoryState 的 class 类型和相关状态。 |
| [L21](../src/webrtc/WindowsPreferredVideoDecoderFactory.h#L21) | `WindowsPreferredVideoDecoderFactory` | class | Composes a Windows hardware decoder factory with WebRTC's built-in software decoder factory. Automatic mode prefers the proven low-latency software decoder; users can still expl... |

### 成员与文件级变量

| 行 | 变量 | 声明 | 作用 |
|---:|---|---|---|
| [L16](../src/webrtc/WindowsPreferredVideoDecoderFactory.h#L16) | `WindowsPreferredVideoDecoderFactoryState` | `class WindowsPreferredVideoDecoderFactoryState;` | 保存状态机当前状态：windows preferred video decoder factory state。 |
| [L55](../src/webrtc/WindowsPreferredVideoDecoderFactory.h#L55) | `state_` | `std::shared_ptr<WindowsPreferredVideoDecoderFactoryState> state_;` | 保存状态机当前状态：state。 |

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L24](../src/webrtc/WindowsPreferredVideoDecoderFactory.h#L24) | `WindowsPreferredVideoDecoderFactory` | 声明 | `WindowsPreferredVideoDecoderFactory( std::unique_ptr<webrtc::VideoDecoderFactory> softwareFactory, std::unique_ptr<webrtc::VideoDecoderFactory> hardwareFactory, bool preferSoftware)` | 实现 windows preferred video decoder factory 对应的业务或工具逻辑。 |
| [L28](../src/webrtc/WindowsPreferredVideoDecoderFactory.h#L28) | `~WindowsPreferredVideoDecoderFactory` | 声明 | `~WindowsPreferredVideoDecoderFactory() override` | 停止相关活动并释放 WindowsPreferredVideoDecoderFactory 实例拥有的资源。 |
| [L30](../src/webrtc/WindowsPreferredVideoDecoderFactory.h#L30) | `WindowsPreferredVideoDecoderFactory` | 声明 | `WindowsPreferredVideoDecoderFactory( const WindowsPreferredVideoDecoderFactory&) = delete` | 实现 windows preferred video decoder factory 对应的业务或工具逻辑。 |
| [L35](../src/webrtc/WindowsPreferredVideoDecoderFactory.h#L35) | `GetSupportedFormats` | 声明 | `std::vector<webrtc::SdpVideoFormat> GetSupportedFormats() const override` | 查询并返回 get supported formats 相关逻辑。 |
| [L37](../src/webrtc/WindowsPreferredVideoDecoderFactory.h#L37) | `QueryCodecSupport` | 声明 | `CodecSupport QueryCodecSupport( const webrtc::SdpVideoFormat& format, bool referenceScaling, std::optional<webrtc::Resolution> resolution) const override` | 发起请求或查询 query codec support 相关逻辑。 |
| [L42](../src/webrtc/WindowsPreferredVideoDecoderFactory.h#L42) | `Create` | 声明 | `std::unique_ptr<webrtc::VideoDecoder> Create( const webrtc::Environment& environment, const webrtc::SdpVideoFormat& format) override` | 创建或初始化 create 相关逻辑。 |
| [L46](../src/webrtc/WindowsPreferredVideoDecoderFactory.h#L46) | `HasHardwareDecoderFor` | 声明 | `bool HasHardwareDecoderFor( const webrtc::SdpVideoFormat& format) const` | 判断 has hardware decoder for 相关逻辑。 |
| [L48](../src/webrtc/WindowsPreferredVideoDecoderFactory.h#L48) | `HasSoftwareDecoderFor` | 声明 | `bool HasSoftwareDecoderFor( const webrtc::SdpVideoFormat& format) const` | 判断 has software decoder for 相关逻辑。 |
| [L51](../src/webrtc/WindowsPreferredVideoDecoderFactory.h#L51) | `SetPreferSoftware` | 声明 | `void SetPreferSoftware(bool preferSoftware) noexcept` | 更新或应用 set prefer software 相关逻辑。 |
| [L52](../src/webrtc/WindowsPreferredVideoDecoderFactory.h#L52) | `NotifyConfigurationChanged` | 声明 | `void NotifyConfigurationChanged() noexcept` | 通知或报告 notify configuration changed 相关逻辑。 |

## `src/webrtc/WindowsPreferredVideoEncoderFactory.cpp`

[打开源码](../src/webrtc/WindowsPreferredVideoEncoderFactory.cpp) · **文件作用：** 实现 windows preferred video encoder factory 相关函数与文件级辅助逻辑。

### 类型

| 行 | 类型 | 种类 | 作用 |
|---:|---|---|---|
| [L34](../src/webrtc/WindowsPreferredVideoEncoderFactory.cpp#L34) | `HardwareFailureOnlyFieldTrials` | class | a healthy NVENC/QSV/AMF encoder with libx264. Preserve every other field trial and disable only the resolution-based forced fallback for the wrapper we construct here. Real Init... |
| [L62](../src/webrtc/WindowsPreferredVideoEncoderFactory.cpp#L62) | `TransientFailureRetryVideoEncoder` | class | fail even though an immediate retry succeeds. libwebrtc's stock fallback wrapper permanently selects software after that first failure. Absorb only this short transient here; re... |
| [L216](../src/webrtc/WindowsPreferredVideoEncoderFactory.cpp#L216) | `RuntimeTrackedSoftwareEncoder` | class | 定义 RuntimeTrackedSoftwareEncoder 的 class 类型和相关状态。 |
| [L536](../src/webrtc/WindowsPreferredVideoEncoderFactory.cpp#L536) | `FrameTimedVideoEncoder` | class | 定义 FrameTimedVideoEncoder 的 class 类型和相关状态。 |
| [L768](../src/webrtc/WindowsPreferredVideoEncoderFactory.cpp#L768) | `WindowsPreferredVideoEncoderFactoryState` | class | 定义 WindowsPreferredVideoEncoderFactoryState 的 class 类型和相关状态。 |
| [L896](../src/webrtc/WindowsPreferredVideoEncoderFactory.cpp#L896) | `ReconfigurableVideoEncoder` | class | 定义 ReconfigurableVideoEncoder 的 class 类型和相关状态。 |

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L37](../src/webrtc/WindowsPreferredVideoEncoderFactory.cpp#L37) | `HardwareFailureOnlyFieldTrials` | 定义 | `explicit HardwareFailureOnlyFieldTrials( const webrtc::FieldTrialsView& base) : base_(base) {}` | 实现 hardware failure only field trials 对应的业务或工具逻辑。 |
| [L42](../src/webrtc/WindowsPreferredVideoEncoderFactory.cpp#L42) | `Lookup` | 定义 | `std::string Lookup(absl::string_view key) const override` | 查询并返回 lookup 相关逻辑。 |
| [L65](../src/webrtc/WindowsPreferredVideoEncoderFactory.cpp#L65) | `TransientFailureRetryVideoEncoder` | 定义 | `explicit TransientFailureRetryVideoEncoder( std::unique_ptr<webrtc::VideoEncoder> encoder) : encoder_(std::move(encoder)) {}` | 实现 transient failure retry video encoder 对应的业务或工具逻辑。 |
| [L70](../src/webrtc/WindowsPreferredVideoEncoderFactory.cpp#L70) | `SetFecControllerOverride` | 定义 | `void SetFecControllerOverride( webrtc::FecControllerOverride* fecControllerOverride) override` | 更新或应用 set fec controller override 相关逻辑。 |
| [L77](../src/webrtc/WindowsPreferredVideoEncoderFactory.cpp#L77) | `InitEncode` | 定义 | `int InitEncode( const webrtc::VideoCodec* codecSettings, const Settings& settings) override` | 创建或初始化 init encode 相关逻辑。 |
| [L93](../src/webrtc/WindowsPreferredVideoEncoderFactory.cpp#L93) | `RegisterEncodeCompleteCallback` | 定义 | `int32_t RegisterEncodeCompleteCallback( webrtc::EncodedImageCallback* callback) override` | 实现 register encode complete callback 对应的业务或工具逻辑。 |
| [L100](../src/webrtc/WindowsPreferredVideoEncoderFactory.cpp#L100) | `Release` | 定义 | `int32_t Release() override` | 释放或取消 release 相关逻辑。 |
| [L108](../src/webrtc/WindowsPreferredVideoEncoderFactory.cpp#L108) | `Encode` | 定义 | `int32_t Encode( const webrtc::VideoFrame& frame, const std::vector<webrtc::VideoFrameType>* frameTypes) override` | 编码 encode 相关逻辑。 |
| [L133](../src/webrtc/WindowsPreferredVideoEncoderFactory.cpp#L133) | `OnPacketLossRateUpdate` | 定义 | `void OnPacketLossRateUpdate(float packetLossRate) override` | 接收并处理 on packet loss rate update 相关逻辑。 |
| [L139](../src/webrtc/WindowsPreferredVideoEncoderFactory.cpp#L139) | `OnRttUpdate` | 定义 | `void OnRttUpdate(int64_t rttMs) override` | 接收并处理 on rtt update 相关逻辑。 |
| [L145](../src/webrtc/WindowsPreferredVideoEncoderFactory.cpp#L145) | `OnLossNotification` | 定义 | `void OnLossNotification( const LossNotification& lossNotification) override` | 接收并处理 on loss notification 相关逻辑。 |
| [L152](../src/webrtc/WindowsPreferredVideoEncoderFactory.cpp#L152) | `SetRates` | 定义 | `void SetRates(const RateControlParameters& parameters) override` | 更新或应用 set rates 相关逻辑。 |
| [L158](../src/webrtc/WindowsPreferredVideoEncoderFactory.cpp#L158) | `GetEncoderInfo` | 定义 | `EncoderInfo GetEncoderInfo() const override` | 查询并返回 get encoder info 相关逻辑。 |
| [L164](../src/webrtc/WindowsPreferredVideoEncoderFactory.cpp#L164) | `InitializeWithRetry` | 定义 | `int InitializeWithRetry(int attempts)` | 创建或初始化 initialize with retry 相关逻辑。 |
| [L184](../src/webrtc/WindowsPreferredVideoEncoderFactory.cpp#L184) | `PrimeEncoder` | 定义 | `void PrimeEncoder()` | 实现 prime encoder 对应的业务或工具逻辑。 |
| [L220](../src/webrtc/WindowsPreferredVideoEncoderFactory.cpp#L220) | `RuntimeTrackedSoftwareEncoder` | 定义 | `RuntimeTrackedSoftwareEncoder( std::unique_ptr<webrtc::VideoEncoder> encoder, std::shared_ptr<VideoEncoderRuntimeState> runtimeState) : encoder_(std::move(encoder)) , runtimeState_(std::move(runtimeState)) {}` | 执行后台循环或调度 runtime tracked software encoder 相关逻辑。 |
| [L227](../src/webrtc/WindowsPreferredVideoEncoderFactory.cpp#L227) | `~RuntimeTrackedSoftwareEncoder` | 定义 | `~RuntimeTrackedSoftwareEncoder() override` | 停止相关活动并释放 RuntimeTrackedSoftwareEncoder 实例拥有的资源。 |
| [L232](../src/webrtc/WindowsPreferredVideoEncoderFactory.cpp#L232) | `InitEncode` | 定义 | `int InitEncode( const webrtc::VideoCodec* codecSettings, const Settings& settings) override` | 创建或初始化 init encode 相关逻辑。 |
| [L289](../src/webrtc/WindowsPreferredVideoEncoderFactory.cpp#L289) | `RegisterEncodeCompleteCallback` | 定义 | `int32_t RegisterEncodeCompleteCallback( webrtc::EncodedImageCallback* callback) override` | 实现 register encode complete callback 对应的业务或工具逻辑。 |
| [L300](../src/webrtc/WindowsPreferredVideoEncoderFactory.cpp#L300) | `Release` | 定义 | `int32_t Release() override` | 释放或取消 release 相关逻辑。 |
| [L311](../src/webrtc/WindowsPreferredVideoEncoderFactory.cpp#L311) | `Encode` | 定义 | `int32_t Encode( const webrtc::VideoFrame& frame, const std::vector<webrtc::VideoFrameType>* frameTypes) override` | 编码 encode 相关逻辑。 |
| [L324](../src/webrtc/WindowsPreferredVideoEncoderFactory.cpp#L324) | `SetRates` | 定义 | `void SetRates( const RateControlParameters& parameters) override` | 更新或应用 set rates 相关逻辑。 |
| [L339](../src/webrtc/WindowsPreferredVideoEncoderFactory.cpp#L339) | `GetEncoderInfo` | 定义 | `EncoderInfo GetEncoderInfo() const override` | 查询并返回 get encoder info 相关逻辑。 |
| [L344](../src/webrtc/WindowsPreferredVideoEncoderFactory.cpp#L344) | `OnEncodedImage` | 定义 | `Result OnEncodedImage( const webrtc::EncodedImage& encodedImage, const webrtc::CodecSpecificInfo* codecSpecificInfo) override` | 接收并处理 on encoded image 相关逻辑。 |
| [L360](../src/webrtc/WindowsPreferredVideoEncoderFactory.cpp#L360) | `OnFrameDropped` | 定义 | `void OnFrameDropped( uint32_t rtpTimestamp, int spatialId, bool isEndOfTemporalUnit) override` | 接收并处理 on frame dropped 相关逻辑。 |
| [L378](../src/webrtc/WindowsPreferredVideoEncoderFactory.cpp#L378) | `PublishFramePipeline` | 定义 | `void PublishFramePipeline()` | 发送或发布 publish frame pipeline 相关逻辑。 |
| [L401](../src/webrtc/WindowsPreferredVideoEncoderFactory.cpp#L401) | `RecordInputFrame` | 定义 | `void RecordInputFrame()` | 实现 record input frame 对应的业务或工具逻辑。 |
| [L434](../src/webrtc/WindowsPreferredVideoEncoderFactory.cpp#L434) | `RecordOutputFrame` | 定义 | `void RecordOutputFrame()` | 实现 record output frame 对应的业务或工具逻辑。 |
| [L467](../src/webrtc/WindowsPreferredVideoEncoderFactory.cpp#L467) | `RecordDroppedFrame` | 定义 | `void RecordDroppedFrame()` | 实现 record dropped frame 对应的业务或工具逻辑。 |
| [L476](../src/webrtc/WindowsPreferredVideoEncoderFactory.cpp#L476) | `Unregister` | 定义 | `void Unregister()` | 实现 unregister 对应的业务或工具逻辑。 |
| [L484](../src/webrtc/WindowsPreferredVideoEncoderFactory.cpp#L484) | `ReportInputFormat` | 定义 | `void ReportInputFormat( const webrtc::scoped_refptr<webrtc::VideoFrameBuffer>& buffer)` | 通知或报告 report input format 相关逻辑。 |
| [L540](../src/webrtc/WindowsPreferredVideoEncoderFactory.cpp#L540) | `FrameTimedVideoEncoder` | 定义 | `explicit FrameTimedVideoEncoder( std::unique_ptr<webrtc::VideoEncoder> encoder) : encoder_(std::move(encoder)), timingInstanceId_( VideoCodecTimingRegistry::Instance().Register( VideoCodecTimingDirection::kEncode)) {}` | 实现 frame timed video encoder 对应的业务或工具逻辑。 |
| [L548](../src/webrtc/WindowsPreferredVideoEncoderFactory.cpp#L548) | `~FrameTimedVideoEncoder` | 定义 | `~FrameTimedVideoEncoder() override` | 停止相关活动并释放 FrameTimedVideoEncoder 实例拥有的资源。 |
| [L554](../src/webrtc/WindowsPreferredVideoEncoderFactory.cpp#L554) | `SetFecControllerOverride` | 定义 | `void SetFecControllerOverride( webrtc::FecControllerOverride* fecControllerOverride) override` | 更新或应用 set fec controller override 相关逻辑。 |
| [L560](../src/webrtc/WindowsPreferredVideoEncoderFactory.cpp#L560) | `InitEncode` | 定义 | `int InitEncode( const webrtc::VideoCodec* codecSettings, const Settings& settings) override` | 创建或初始化 init encode 相关逻辑。 |
| [L573](../src/webrtc/WindowsPreferredVideoEncoderFactory.cpp#L573) | `RegisterEncodeCompleteCallback` | 定义 | `int32_t RegisterEncodeCompleteCallback( webrtc::EncodedImageCallback* callback) override` | 实现 register encode complete callback 对应的业务或工具逻辑。 |
| [L584](../src/webrtc/WindowsPreferredVideoEncoderFactory.cpp#L584) | `Release` | 定义 | `int32_t Release() override` | 释放或取消 release 相关逻辑。 |
| [L595](../src/webrtc/WindowsPreferredVideoEncoderFactory.cpp#L595) | `Encode` | 定义 | `int32_t Encode( const webrtc::VideoFrame& frame, const std::vector<webrtc::VideoFrameType>* frameTypes) override` | 编码 encode 相关逻辑。 |
| [L616](../src/webrtc/WindowsPreferredVideoEncoderFactory.cpp#L616) | `SetRates` | 定义 | `void SetRates( const RateControlParameters& parameters) override` | 更新或应用 set rates 相关逻辑。 |
| [L622](../src/webrtc/WindowsPreferredVideoEncoderFactory.cpp#L622) | `OnPacketLossRateUpdate` | 定义 | `void OnPacketLossRateUpdate(float packetLossRate) override` | 接收并处理 on packet loss rate update 相关逻辑。 |
| [L627](../src/webrtc/WindowsPreferredVideoEncoderFactory.cpp#L627) | `OnRttUpdate` | 定义 | `void OnRttUpdate(int64_t rttMs) override` | 接收并处理 on rtt update 相关逻辑。 |
| [L632](../src/webrtc/WindowsPreferredVideoEncoderFactory.cpp#L632) | `OnLossNotification` | 定义 | `void OnLossNotification( const LossNotification& lossNotification) override` | 接收并处理 on loss notification 相关逻辑。 |
| [L638](../src/webrtc/WindowsPreferredVideoEncoderFactory.cpp#L638) | `GetEncoderInfo` | 定义 | `EncoderInfo GetEncoderInfo() const override` | 查询并返回 get encoder info 相关逻辑。 |
| [L648](../src/webrtc/WindowsPreferredVideoEncoderFactory.cpp#L648) | `OnEncodedImage` | 定义 | `Result OnEncodedImage( const webrtc::EncodedImage& encodedImage, const webrtc::CodecSpecificInfo* codecSpecificInfo) override` | 接收并处理 on encoded image 相关逻辑。 |
| [L683](../src/webrtc/WindowsPreferredVideoEncoderFactory.cpp#L683) | `OnFrameDropped` | 定义 | `void OnFrameDropped( uint32_t rtpTimestamp, int spatialId, bool isEndOfTemporalUnit) override` | 接收并处理 on frame dropped 相关逻辑。 |
| [L707](../src/webrtc/WindowsPreferredVideoEncoderFactory.cpp#L707) | `ClearPending` | 定义 | `void ClearPending()` | 重置或移除 clear pending 相关逻辑。 |
| [L713](../src/webrtc/WindowsPreferredVideoEncoderFactory.cpp#L713) | `RecordCompleted` | 定义 | `void RecordCompleted(const webrtc::EncodedImage& encodedImage)` | 实现 record completed 对应的业务或工具逻辑。 |
| [L755](../src/webrtc/WindowsPreferredVideoEncoderFactory.cpp#L755) | `AppendUniqueFormats` | 定义 | `void AppendUniqueFormats( std::vector<webrtc::SdpVideoFormat>& destination, const std::vector<webrtc::SdpVideoFormat>& source)` | 实现 append unique formats 对应的业务或工具逻辑。 |
| [L770](../src/webrtc/WindowsPreferredVideoEncoderFactory.cpp#L770) | `WindowsPreferredVideoEncoderFactoryState` | 定义 | `WindowsPreferredVideoEncoderFactoryState( std::unique_ptr<webrtc::VideoEncoderFactory> softwareFactory, std::unique_ptr<webrtc::VideoEncoderFactory> hardwareFactory, std::shared_ptr<VideoEncoderRuntimeState> runtimeSt...` | 实现 windows preferred video encoder factory state 对应的业务或工具逻辑。 |
| [L779](../src/webrtc/WindowsPreferredVideoEncoderFactory.cpp#L779) | `GetSupportedFormats` | 定义 | `std::vector<webrtc::SdpVideoFormat> GetSupportedFormats() const` | 查询并返回 get supported formats 相关逻辑。 |
| [L794](../src/webrtc/WindowsPreferredVideoEncoderFactory.cpp#L794) | `QueryCodecSupport` | 定义 | `webrtc::VideoEncoderFactory::CodecSupport QueryCodecSupport( const webrtc::SdpVideoFormat& format, std::optional<std::string> scalabilityMode, std::optional<webrtc::Resolution> resolution) const` | 发起请求或查询 query codec support 相关逻辑。 |
| [L813](../src/webrtc/WindowsPreferredVideoEncoderFactory.cpp#L813) | `CreateSelected` | 定义 | `std::unique_ptr<webrtc::VideoEncoder> CreateSelected( const webrtc::Environment& environment, const webrtc::SdpVideoFormat& format, std::uint64_t* revision) const` | 创建或初始化 create selected 相关逻辑。 |
| [L854](../src/webrtc/WindowsPreferredVideoEncoderFactory.cpp#L854) | `Revision` | 定义 | `std::uint64_t Revision() const` | 实现 revision 对应的业务或工具逻辑。 |
| [L860](../src/webrtc/WindowsPreferredVideoEncoderFactory.cpp#L860) | `HasHardwareEncoderFor` | 定义 | `bool HasHardwareEncoderFor( const webrtc::SdpVideoFormat& format) const` | 判断 has hardware encoder for 相关逻辑。 |
| [L868](../src/webrtc/WindowsPreferredVideoEncoderFactory.cpp#L868) | `HasSoftwareEncoderFor` | 定义 | `bool HasSoftwareEncoderFor( const webrtc::SdpVideoFormat& format) const` | 判断 has software encoder for 相关逻辑。 |
| [L876](../src/webrtc/WindowsPreferredVideoEncoderFactory.cpp#L876) | `ReplaceFactories` | 定义 | `void ReplaceFactories( std::unique_ptr<webrtc::VideoEncoderFactory> softwareFactory, std::unique_ptr<webrtc::VideoEncoderFactory> hardwareFactory)` | 实现 replace factories 对应的业务或工具逻辑。 |
| [L898](../src/webrtc/WindowsPreferredVideoEncoderFactory.cpp#L898) | `ReconfigurableVideoEncoder` | 定义 | `ReconfigurableVideoEncoder( std::shared_ptr<WindowsPreferredVideoEncoderFactoryState> state, webrtc::Environment environment, webrtc::SdpVideoFormat format) : state_(std::move(state)), environment_(std::move(environme...` | 实现 reconfigurable video encoder 对应的业务或工具逻辑。 |
| [L909](../src/webrtc/WindowsPreferredVideoEncoderFactory.cpp#L909) | `SetFecControllerOverride` | 定义 | `void SetFecControllerOverride( webrtc::FecControllerOverride* fecControllerOverride) override` | 更新或应用 set fec controller override 相关逻辑。 |
| [L918](../src/webrtc/WindowsPreferredVideoEncoderFactory.cpp#L918) | `InitEncode` | 定义 | `int InitEncode( const webrtc::VideoCodec* codecSettings, const Settings& settings) override` | 创建或初始化 init encode 相关逻辑。 |
| [L932](../src/webrtc/WindowsPreferredVideoEncoderFactory.cpp#L932) | `RegisterEncodeCompleteCallback` | 定义 | `int32_t RegisterEncodeCompleteCallback( webrtc::EncodedImageCallback* callback) override` | 实现 register encode complete callback 对应的业务或工具逻辑。 |
| [L941](../src/webrtc/WindowsPreferredVideoEncoderFactory.cpp#L941) | `Release` | 定义 | `int32_t Release() override` | 释放或取消 release 相关逻辑。 |
| [L949](../src/webrtc/WindowsPreferredVideoEncoderFactory.cpp#L949) | `Encode` | 定义 | `int32_t Encode( const webrtc::VideoFrame& frame, const std::vector<webrtc::VideoFrameType>* frameTypes) override` | 编码 encode 相关逻辑。 |
| [L958](../src/webrtc/WindowsPreferredVideoEncoderFactory.cpp#L958) | `SetRates` | 定义 | `void SetRates(const RateControlParameters& parameters) override` | 更新或应用 set rates 相关逻辑。 |
| [L966](../src/webrtc/WindowsPreferredVideoEncoderFactory.cpp#L966) | `OnPacketLossRateUpdate` | 定义 | `void OnPacketLossRateUpdate(float packetLossRate) override` | 接收并处理 on packet loss rate update 相关逻辑。 |
| [L973](../src/webrtc/WindowsPreferredVideoEncoderFactory.cpp#L973) | `OnRttUpdate` | 定义 | `void OnRttUpdate(int64_t rttMs) override` | 接收并处理 on rtt update 相关逻辑。 |
| [L980](../src/webrtc/WindowsPreferredVideoEncoderFactory.cpp#L980) | `OnLossNotification` | 定义 | `void OnLossNotification( const LossNotification& lossNotification) override` | 接收并处理 on loss notification 相关逻辑。 |
| [L988](../src/webrtc/WindowsPreferredVideoEncoderFactory.cpp#L988) | `GetEncoderInfo` | 定义 | `EncoderInfo GetEncoderInfo() const override` | 查询并返回 get encoder info 相关逻辑。 |
| [L994](../src/webrtc/WindowsPreferredVideoEncoderFactory.cpp#L994) | `EnsureCurrent` | 定义 | `bool EnsureCurrent(bool initialize)` | 实现 ensure current 对应的业务或工具逻辑。 |
| [L1051](../src/webrtc/WindowsPreferredVideoEncoderFactory.cpp#L1051) | `WindowsPreferredVideoEncoderFactory` | 定义 | `WindowsPreferredVideoEncoderFactory( std::unique_ptr<webrtc::VideoEncoderFactory> softwareFactory, std::unique_ptr<webrtc::VideoEncoderFactory> hardwareFactory, std::shared_ptr<VideoEncoderRuntimeState> runtimeState) ...` | 实现 windows preferred video encoder factory 对应的业务或工具逻辑。 |
| [L1064](../src/webrtc/WindowsPreferredVideoEncoderFactory.cpp#L1064) | `WindowsPreferredVideoEncoderFactory::GetSupportedFormats` | 定义 | `WindowsPreferredVideoEncoderFactory::GetSupportedFormats() const` | 查询并返回 get supported formats 相关逻辑。 |
| [L1070](../src/webrtc/WindowsPreferredVideoEncoderFactory.cpp#L1070) | `WindowsPreferredVideoEncoderFactory::QueryCodecSupport` | 定义 | `WindowsPreferredVideoEncoderFactory::QueryCodecSupport( const webrtc::SdpVideoFormat& format, std::optional<std::string> scalabilityMode, std::optional<webrtc::Resolution> resolution) const` | 发起请求或查询 query codec support 相关逻辑。 |
| [L1080](../src/webrtc/WindowsPreferredVideoEncoderFactory.cpp#L1080) | `WindowsPreferredVideoEncoderFactory::Create` | 定义 | `WindowsPreferredVideoEncoderFactory::Create( const webrtc::Environment& environment, const webrtc::SdpVideoFormat& format)` | 创建或初始化 create 相关逻辑。 |
| [L1093](../src/webrtc/WindowsPreferredVideoEncoderFactory.cpp#L1093) | `WindowsPreferredVideoEncoderFactory::HasHardwareEncoderFor` | 定义 | `bool WindowsPreferredVideoEncoderFactory::HasHardwareEncoderFor( const webrtc::SdpVideoFormat& format) const` | 判断 has hardware encoder for 相关逻辑。 |
| [L1099](../src/webrtc/WindowsPreferredVideoEncoderFactory.cpp#L1099) | `WindowsPreferredVideoEncoderFactory::HasSoftwareEncoderFor` | 定义 | `bool WindowsPreferredVideoEncoderFactory::HasSoftwareEncoderFor( const webrtc::SdpVideoFormat& format) const` | 判断 has software encoder for 相关逻辑。 |
| [L1105](../src/webrtc/WindowsPreferredVideoEncoderFactory.cpp#L1105) | `WindowsPreferredVideoEncoderFactory::ReplaceFactories` | 定义 | `void WindowsPreferredVideoEncoderFactory::ReplaceFactories( std::unique_ptr<webrtc::VideoEncoderFactory> softwareFactory, std::unique_ptr<webrtc::VideoEncoderFactory> hardwareFactory)` | 实现 replace factories 对应的业务或工具逻辑。 |

## `src/webrtc/WindowsPreferredVideoEncoderFactory.h`

[打开源码](../src/webrtc/WindowsPreferredVideoEncoderFactory.h) · **文件作用：** 声明 windows preferred video encoder factory 相关类型、接口、配置和成员状态。

### 类型

| 行 | 类型 | 种类 | 作用 |
|---:|---|---|---|
| [L16](../src/webrtc/WindowsPreferredVideoEncoderFactory.h#L16) | `WindowsPreferredVideoEncoderFactoryState` | class | 定义 WindowsPreferredVideoEncoderFactoryState 的 class 类型和相关状态。 |
| [L20](../src/webrtc/WindowsPreferredVideoEncoderFactory.h#L20) | `WindowsPreferredVideoEncoderFactory` | class | Prefers the Windows Media Foundation/D3D11 H264 encoder and delegates runtime failure to WebRTC's official software-fallback wrapper. |

### 成员与文件级变量

| 行 | 变量 | 声明 | 作用 |
|---:|---|---|---|
| [L16](../src/webrtc/WindowsPreferredVideoEncoderFactory.h#L16) | `WindowsPreferredVideoEncoderFactoryState` | `class WindowsPreferredVideoEncoderFactoryState;` | 保存状态机当前状态：windows preferred video encoder factory state。 |
| [L53](../src/webrtc/WindowsPreferredVideoEncoderFactory.h#L53) | `state_` | `std::shared_ptr<WindowsPreferredVideoEncoderFactoryState> state_;` | 保存状态机当前状态：state。 |

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L23](../src/webrtc/WindowsPreferredVideoEncoderFactory.h#L23) | `WindowsPreferredVideoEncoderFactory` | 声明 | `WindowsPreferredVideoEncoderFactory( std::unique_ptr<webrtc::VideoEncoderFactory> softwareFactory, std::unique_ptr<webrtc::VideoEncoderFactory> hardwareFactory, std::shared_ptr<VideoEncoderRuntimeState> runtimeState =...` | 实现 windows preferred video encoder factory 对应的业务或工具逻辑。 |
| [L27](../src/webrtc/WindowsPreferredVideoEncoderFactory.h#L27) | `~WindowsPreferredVideoEncoderFactory` | 声明 | `~WindowsPreferredVideoEncoderFactory() override` | 停止相关活动并释放 WindowsPreferredVideoEncoderFactory 实例拥有的资源。 |
| [L30](../src/webrtc/WindowsPreferredVideoEncoderFactory.h#L30) | `GetSupportedFormats` | 声明 | `GetSupportedFormats() const override` | 查询并返回 get supported formats 相关逻辑。 |
| [L31](../src/webrtc/WindowsPreferredVideoEncoderFactory.h#L31) | `QueryCodecSupport` | 声明 | `CodecSupport QueryCodecSupport( const webrtc::SdpVideoFormat& format, std::optional<std::string> scalabilityMode, std::optional<webrtc::Resolution> resolution) const override` | 发起请求或查询 query codec support 相关逻辑。 |
| [L35](../src/webrtc/WindowsPreferredVideoEncoderFactory.h#L35) | `Create` | 声明 | `std::unique_ptr<webrtc::VideoEncoder> Create( const webrtc::Environment& environment, const webrtc::SdpVideoFormat& format) override` | 创建或初始化 create 相关逻辑。 |
| [L39](../src/webrtc/WindowsPreferredVideoEncoderFactory.h#L39) | `HasHardwareEncoderFor` | 声明 | `bool HasHardwareEncoderFor( const webrtc::SdpVideoFormat& format) const` | 判断 has hardware encoder for 相关逻辑。 |
| [L41](../src/webrtc/WindowsPreferredVideoEncoderFactory.h#L41) | `HasSoftwareEncoderFor` | 声明 | `bool HasSoftwareEncoderFor( const webrtc::SdpVideoFormat& format) const` | 判断 has software encoder for 相关逻辑。 |
| [L48](../src/webrtc/WindowsPreferredVideoEncoderFactory.h#L48) | `ReplaceFactories` | 声明 | `void ReplaceFactories( std::unique_ptr<webrtc::VideoEncoderFactory> softwareFactory, std::unique_ptr<webrtc::VideoEncoderFactory> hardwareFactory)` | Existing PeerConnections keep this factory for their whole lifetime. Replacing the child factories while no video is being published lets the next track create an encoder from t... |
