# 应用层：主界面与远程会话窗口

> 自动生成于 2026-09-06，源码树 `1ae783c8db12-dirty`。请运行 `tools/Generate-SourceSymbolReference.ps1` 刷新。

Qt 主窗口、远程会话窗口、画布、文件窗口、主题与交互控件。

本册共收录 84 个源码文件。函数与变量的中文作用优先采用源码紧邻注释；无注释时根据符号命名生成阅读提示，最终语义仍以源码为准。

## `src/apps/controller/CameraWindow.cpp`

[打开源码](../src/apps/controller/CameraWindow.cpp) · **文件作用：** 实现 camera window 相关函数与文件级辅助逻辑。

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L132](../src/apps/controller/CameraWindow.cpp#L132) | `CameraPreviewWidget::CameraPreviewWidget` | 定义 | `CameraPreviewWidget::CameraPreviewWidget(QWidget* parent) : QWidget(parent)` | 构造并初始化 CameraPreviewWidget 实例。 |
| [L138](../src/apps/controller/CameraWindow.cpp#L138) | `CameraPreviewWidget::paintEvent` | 定义 | `void CameraPreviewWidget::paintEvent(QPaintEvent*)` | 准备或呈现 paint event 相关逻辑。 |
| [L190](../src/apps/controller/CameraWindow.cpp#L190) | `CameraOverlayWidget::CameraOverlayWidget` | 定义 | `CameraOverlayWidget::CameraOverlayWidget( QString deviceName, std::function<void()> detachAction, std::function<void()> closeAction, QWidget* parent) : QWidget(parent)` | 构造并初始化 CameraOverlayWidget 实例。 |
| [L288](../src/apps/controller/CameraWindow.cpp#L288) | `CameraOverlayWidget::eventFilter` | 定义 | `bool CameraOverlayWidget::eventFilter(QObject* watched, QEvent* event)` | 实现 event filter 对应的业务或工具逻辑。 |
| [L318](../src/apps/controller/CameraWindow.cpp#L318) | `CameraWindow::CameraWindow` | 定义 | `CameraWindow::CameraWindow(QString deviceId, QString deviceName, std::function<void()> reattachAction, QWidget* parent) : FramelessMainWindow(parent), deviceId_(std::move(deviceId)), deviceName_(std::move(deviceName))...` | 构造并初始化 CameraWindow 实例。 |
| [L330](../src/apps/controller/CameraWindow.cpp#L330) | `CameraWindow::BuildUi` | 定义 | `void CameraWindow::BuildUi()` | 创建或初始化 build ui 相关逻辑。 |
| [L492](../src/apps/controller/CameraWindow.cpp#L492) | `CameraWindow::ReturnToOverlay` | 定义 | `void CameraWindow::ReturnToOverlay()` | 实现 return to overlay 对应的业务或工具逻辑。 |

## `src/apps/controller/CameraWindow.h`

[打开源码](../src/apps/controller/CameraWindow.h) · **文件作用：** 声明 camera window 相关类型、接口、配置和成员状态。

### 类型

| 行 | 类型 | 种类 | 作用 |
|---:|---|---|---|
| [L15](../src/apps/controller/CameraWindow.h#L15) | `CameraPreviewWidget` | class | 定义 CameraPreviewWidget 的 class 类型和相关状态。 |
| [L23](../src/apps/controller/CameraWindow.h#L23) | `CameraOverlayWidget` | class | 定义 CameraOverlayWidget 的 class 类型和相关状态。 |
| [L39](../src/apps/controller/CameraWindow.h#L39) | `CameraWindow` | class | 定义 CameraWindow 的 class 类型和相关状态。 |

### 成员与文件级变量

| 行 | 变量 | 声明 | 作用 |
|---:|---|---|---|
| [L34](../src/apps/controller/CameraWindow.h#L34) | `dragHandle_` | `QWidget* dragHandle_ = nullptr;` | 保存 drag handle 相关配置或运行状态。 |
| [L35](../src/apps/controller/CameraWindow.h#L35) | `dragOffset_` | `QPoint dragOffset_;` | 保存 drag offset 相关配置或运行状态。 |
| [L36](../src/apps/controller/CameraWindow.h#L36) | `dragging_` | `bool dragging_ = false;` | 保存 dragging 相关配置或运行状态。 |
| [L54](../src/apps/controller/CameraWindow.h#L54) | `deviceId_` | `QString deviceId_;` | 保存身份或作用域标识：device id。 |
| [L55](../src/apps/controller/CameraWindow.h#L55) | `deviceName_` | `QString deviceName_;` | 保存路径、地址或显示名称：device name。 |

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L17](../src/apps/controller/CameraWindow.h#L17) | `CameraPreviewWidget` | 声明 | `explicit CameraPreviewWidget(QWidget* parent = nullptr)` | 实现 camera preview widget 对应的业务或工具逻辑。 |
| [L20](../src/apps/controller/CameraWindow.h#L20) | `paintEvent` | 声明 | `void paintEvent(QPaintEvent* event) override` | 准备或呈现 paint event 相关逻辑。 |
| [L25](../src/apps/controller/CameraWindow.h#L25) | `CameraOverlayWidget` | 声明 | `CameraOverlayWidget(QString deviceName, std::function<void()> detachAction, std::function<void()> closeAction, QWidget* parent = nullptr)` | 实现 camera overlay widget 对应的业务或工具逻辑。 |
| [L31](../src/apps/controller/CameraWindow.h#L31) | `eventFilter` | 声明 | `bool eventFilter(QObject* watched, QEvent* event) override` | 实现 event filter 对应的业务或工具逻辑。 |
| [L41](../src/apps/controller/CameraWindow.h#L41) | `CameraWindow` | 定义 | `CameraWindow(QString deviceId, QString deviceName, std::function<void()> reattachAction = {},` | 实现 camera window 对应的业务或工具逻辑。 |
| [L45](../src/apps/controller/CameraWindow.h#L45) | `~CameraWindow` | 声明 | `~CameraWindow() override = default` | 停止相关活动并释放 CameraWindow 实例拥有的资源。 |
| [L47](../src/apps/controller/CameraWindow.h#L47) | `CameraWindow` | 声明 | `CameraWindow(const CameraWindow&) = delete` | 实现 camera window 对应的业务或工具逻辑。 |
| [L51](../src/apps/controller/CameraWindow.h#L51) | `BuildUi` | 声明 | `void BuildUi()` | 创建或初始化 build ui 相关逻辑。 |
| [L52](../src/apps/controller/CameraWindow.h#L52) | `ReturnToOverlay` | 声明 | `void ReturnToOverlay()` | 实现 return to overlay 对应的业务或工具逻辑。 |

## `src/apps/controller/ControllerMainWindow.AuxiliaryWindows.inc`

[打开源码](../src/apps/controller/ControllerMainWindow.AuxiliaryWindows.inc) · **文件作用：** `ControllerMainWindow` 的实现切片，集中实现 auxiliary windows 相关逻辑；成员状态仍定义在所属头文件中。

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L4](../src/apps/controller/ControllerMainWindow.AuxiliaryWindows.inc#L4) | `ControllerMainWindow::HandleRemoteSessionDisconnect` | 定义 | `void ControllerMainWindow::HandleRemoteSessionDisconnect()` | SPDX-License-Identifier: GPL-3.0-only Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd) |
| [L52](../src/apps/controller/ControllerMainWindow.AuxiliaryWindows.inc#L52) | `ControllerMainWindow::SetRoomActionHint` | 定义 | `void ControllerMainWindow::SetRoomActionHint(const QString& text, bool error)` | 更新或应用 set room action hint 相关逻辑。 |
| [L64](../src/apps/controller/ControllerMainWindow.AuxiliaryWindows.inc#L64) | `ControllerMainWindow::OpenCameraWindow` | 定义 | `void ControllerMainWindow::OpenCameraWindow(const QString& deviceId, const QString& deviceName)` | 启动 open camera window 相关逻辑。 |
| [L82](../src/apps/controller/ControllerMainWindow.AuxiliaryWindows.inc#L82) | `ControllerMainWindow::OpenRemoteSession` | 定义 | `void ControllerMainWindow::OpenRemoteSession( RemoteSessionBinding binding)` | 启动 open remote session 相关逻辑。 |
| [L248](../src/apps/controller/ControllerMainWindow.AuxiliaryWindows.inc#L248) | `ControllerMainWindow::SetRuntimeStatus` | 定义 | `void ControllerMainWindow::SetRuntimeStatus(const QString& status, const QString& color)` | 更新或应用 set runtime status 相关逻辑。 |

## `src/apps/controller/ControllerMainWindow.Clipboard.inc`

[打开源码](../src/apps/controller/ControllerMainWindow.Clipboard.inc) · **文件作用：** `ControllerMainWindow` 的实现切片，集中实现 clipboard 相关逻辑；成员状态仍定义在所属头文件中。

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L4](../src/apps/controller/ControllerMainWindow.Clipboard.inc#L4) | `ControllerMainWindow::ApplyClipboardConfigurationFromUi` | 定义 | `void ControllerMainWindow::ApplyClipboardConfigurationFromUi( bool showFeedback)` | SPDX-License-Identifier: GPL-3.0-only Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd) |
| [L73](../src/apps/controller/ControllerMainWindow.Clipboard.inc#L73) | `ControllerMainWindow::UpdateClipboardSession` | 定义 | `void ControllerMainWindow::UpdateClipboardSession( const SessionEngineSnapshot& snapshot)` | 更新或应用 update clipboard session 相关逻辑。 |
| [L136](../src/apps/controller/ControllerMainWindow.Clipboard.inc#L136) | `ControllerMainWindow::UpdateDirectFileTransferSession` | 定义 | `void ControllerMainWindow::UpdateDirectFileTransferSession( const SessionEngineSnapshot& snapshot)` | 更新或应用 update direct file transfer session 相关逻辑。 |
| [L173](../src/apps/controller/ControllerMainWindow.Clipboard.inc#L173) | `ControllerMainWindow::OnClipboardStateChanged` | 定义 | `void ControllerMainWindow::OnClipboardStateChanged( const app::ClipboardControllerSnapshot& snapshot)` | 接收并处理 on clipboard state changed 相关逻辑。 |

## `src/apps/controller/ControllerMainWindow.cpp`

[打开源码](../src/apps/controller/ControllerMainWindow.cpp) · **文件作用：** 实现 controller main window 相关函数与文件级辅助逻辑。

### 类型

| 行 | 类型 | 种类 | 作用 |
|---:|---|---|---|
| [L518](../src/apps/controller/ControllerMainWindow.cpp#L518) | `ScreenFrameRateLogRecord` | struct | 定义 ScreenFrameRateLogRecord 的 struct 类型和相关状态。 |
| [L544](../src/apps/controller/ControllerMainWindow.cpp#L544) | `AsyncScreenFrameRateLogger` | class | 定义 AsyncScreenFrameRateLogger 的 class 类型和相关状态。 |
| [L1006](../src/apps/controller/ControllerMainWindow.cpp#L1006) | `DiagnosticsChip` | struct | 定义 DiagnosticsChip 的 struct 类型和相关状态。 |
| [L1021](../src/apps/controller/ControllerMainWindow.cpp#L1021) | `DiagnosticsCard` | struct | 定义 DiagnosticsCard 的 struct 类型和相关状态。 |
| [L1028](../src/apps/controller/ControllerMainWindow.cpp#L1028) | `DiagnosticsSection` | struct | 定义 DiagnosticsSection 的 struct 类型和相关状态。 |
| [L1035](../src/apps/controller/ControllerMainWindow.cpp#L1035) | `DiagnosticsCardsWidget` | class | 定义 DiagnosticsCardsWidget 的 class 类型和相关状态。 |
| [L1387](../src/apps/controller/ControllerMainWindow.cpp#L1387) | `CurrentPageStack` | class | 定义 CurrentPageStack 的 class 类型和相关状态。 |
| [L1447](../src/apps/controller/ControllerMainWindow.cpp#L1447) | `MediaStateIcon` | enum class | 定义 MediaStateIcon 的 enum class 类型和相关状态。 |
| [L1454](../src/apps/controller/ControllerMainWindow.cpp#L1454) | `MediaDeviceButton` | class | 定义 MediaDeviceButton 的 class 类型和相关状态。 |
| [L1729](../src/apps/controller/ControllerMainWindow.cpp#L1729) | `RoomStatusIcon` | enum class | 定义 RoomStatusIcon 的 enum class 类型和相关状态。 |
| [L2578](../src/apps/controller/ControllerMainWindow.cpp#L2578) | `NavigationIcon` | enum class | 定义 NavigationIcon 的 enum class 类型和相关状态。 |

### 成员与文件级变量

| 行 | 变量 | 声明 | 作用 |
|---:|---|---|---|
| [L273](../src/apps/controller/ControllerMainWindow.cpp#L273) | `kWindowsAutoStartValueName` | `constexpr auto kWindowsAutoStartValueName = "RemoteC";` | 定义 windows auto start value name 的编译期常量或产品边界。 |
| [L276](../src/apps/controller/ControllerMainWindow.cpp#L276) | `kInterfaceFontFamilySetting` | `constexpr auto kInterfaceFontFamilySetting = "ui/systemFontFamily";` | 定义 interface font family setting 的编译期常量或产品边界。 |
| [L426](../src/apps/controller/ControllerMainWindow.cpp#L426) | `kRemotePastePopupThresholdBytes` | `constexpr std::uint64_t kRemotePastePopupThresholdBytes = 64ull * 1024ull;` | 定义 remote paste popup threshold bytes 的编译期常量或产品边界。 |
| [L427](../src/apps/controller/ControllerMainWindow.cpp#L427) | `kSettingsControlWidth` | `constexpr int kSettingsControlWidth = 270;` | 定义 settings control width 的编译期常量或产品边界。 |
| [L570](../src/apps/controller/ControllerMainWindow.cpp#L570) | `kMaximumQueuedRecords` | `static constexpr std::size_t kMaximumQueuedRecords = 4096;` | 定义 maximum queued records 的编译期常量或产品边界。 |
| [L1534](../src/apps/controller/ControllerMainWindow.cpp#L1534) | `kMenuAreaWidth` | `static constexpr int kMenuAreaWidth = 30;` | 定义 menu area width 的编译期常量或产品边界。 |

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L129](../src/apps/controller/ControllerMainWindow.cpp#L129) | `IsNineDigitPublicId` | 定义 | `bool IsNineDigitPublicId(const QString& value)` | 判断 is nine digit public id 相关逻辑。 |
| [L136](../src/apps/controller/ControllerMainWindow.cpp#L136) | `LocalizedDirectSessionError` | 定义 | `QString LocalizedDirectSessionError( const std::string& errorCode, const std::string& /*fallbackMessage*/, const SessionEngineSnapshot* snapshot = nullptr)` | 实现 localized direct session error 对应的业务或工具逻辑。 |
| [L198](../src/apps/controller/ControllerMainWindow.cpp#L198) | `IsDirectRecoveryFailureCode` | 定义 | `bool IsDirectRecoveryFailureCode(const std::string& errorCode)` | 判断 is direct recovery failure code 相关逻辑。 |
| [L278](../src/apps/controller/ControllerMainWindow.cpp#L278) | `InstalledChineseInterfaceFonts` | 定义 | `QStringList InstalledChineseInterfaceFonts()` | 实现 installed chinese interface fonts 对应的业务或工具逻辑。 |
| [L296](../src/apps/controller/ControllerMainWindow.cpp#L296) | `ApplyInterfaceFontPreference` | 定义 | `void ApplyInterfaceFontPreference(const QString& systemFamily, int pixelSize)` | 更新或应用 apply interface font preference 相关逻辑。 |
| [L315](../src/apps/controller/ControllerMainWindow.cpp#L315) | `AdaptBenchmarkHtmlForTheme` | 定义 | `QString AdaptBenchmarkHtmlForTheme(QString html)` | 实现 adapt benchmark html for theme 对应的业务或工具逻辑。 |
| [L341](../src/apps/controller/ControllerMainWindow.cpp#L341) | `VideoEncoderPreferenceFromSetting` | 定义 | `VideoEncoderPreference VideoEncoderPreferenceFromSetting( const QString& value)` | 实现 video encoder preference from setting 对应的业务或工具逻辑。 |
| [L359](../src/apps/controller/ControllerMainWindow.cpp#L359) | `DesktopCaptureImplementationFromSetting` | 定义 | `DesktopCaptureImplementation DesktopCaptureImplementationFromSetting( const QString& value)` | 实现 desktop capture implementation from setting 对应的业务或工具逻辑。 |
| [L367](../src/apps/controller/ControllerMainWindow.cpp#L367) | `FfmpegHardwareBackendFromSetting` | 定义 | `FfmpegHardwareBackend FfmpegHardwareBackendFromSetting( const QString& value)` | 实现 ffmpeg hardware backend from setting 对应的业务或工具逻辑。 |
| [L382](../src/apps/controller/ControllerMainWindow.cpp#L382) | `EncoderQualityFromSetting` | 定义 | `FfmpegX264Preset EncoderQualityFromSetting(const QString& value)` | 编码 encoder quality from setting 相关逻辑。 |
| [L399](../src/apps/controller/ControllerMainWindow.cpp#L399) | `VideoDecoderPreferenceFromSetting` | 定义 | `VideoDecoderPreference VideoDecoderPreferenceFromSetting( const QString& value)` | 实现 video decoder preference from setting 对应的业务或工具逻辑。 |
| [L429](../src/apps/controller/ControllerMainWindow.cpp#L429) | `InitialClipboardCacheBaseDirectory` | 定义 | `QString InitialClipboardCacheBaseDirectory()` | 创建或初始化 initial clipboard cache base directory 相关逻辑。 |
| [L436](../src/apps/controller/ControllerMainWindow.cpp#L436) | `ClipboardCacheRootForBase` | 定义 | `QString ClipboardCacheRootForBase(const QString& baseDirectory)` | 实现 clipboard cache root for base 对应的业务或工具逻辑。 |
| [L442](../src/apps/controller/ControllerMainWindow.cpp#L442) | `SafeClipboardCacheCapacityGiB` | 定义 | `qulonglong SafeClipboardCacheCapacityGiB(const QString& baseDirectory)` | 实现 safe clipboard cache capacity gi b 对应的业务或工具逻辑。 |
| [L451](../src/apps/controller/ControllerMainWindow.cpp#L451) | `WindowsAutoStartCommand` | 定义 | `QString WindowsAutoStartCommand()` | 实现 windows auto start command 对应的业务或工具逻辑。 |
| [L457](../src/apps/controller/ControllerMainWindow.cpp#L457) | `WindowsAutoStartEnabled` | 定义 | `bool WindowsAutoStartEnabled()` | 实现 windows auto start enabled 对应的业务或工具逻辑。 |
| [L471](../src/apps/controller/ControllerMainWindow.cpp#L471) | `SetWindowsAutoStartEnabled` | 定义 | `bool SetWindowsAutoStartEnabled(bool enabled)` | 更新或应用 set windows auto start enabled 相关逻辑。 |
| [L494](../src/apps/controller/ControllerMainWindow.cpp#L494) | `SavedScreenQualityBounds` | 定义 | `std::pair<std::uint32_t, std::uint32_t> SavedScreenQualityBounds( ScreenQualityTier quality)` | 保存或写入 saved screen quality bounds 相关逻辑。 |
| [L511](../src/apps/controller/ControllerMainWindow.cpp#L511) | `CsvCell` | 定义 | `QString CsvCell(const std::string& value)` | 实现 csv cell 对应的业务或工具逻辑。 |
| [L546](../src/apps/controller/ControllerMainWindow.cpp#L546) | `Instance` | 定义 | `static AsyncScreenFrameRateLogger& Instance()` | 实现 instance 对应的业务或工具逻辑。 |
| [L552](../src/apps/controller/ControllerMainWindow.cpp#L552) | `Enqueue` | 定义 | `void Enqueue(std::vector<ScreenFrameRateLogRecord> records)` | 实现 enqueue 对应的业务或工具逻辑。 |
| [L572](../src/apps/controller/ControllerMainWindow.cpp#L572) | `AsyncScreenFrameRateLogger` | 定义 | `AsyncScreenFrameRateLogger() : worker_([this](std::stop_token stopToken) { Run(stopToken); })` | 实现 async screen frame rate logger 对应的业务或工具逻辑。 |
| [L576](../src/apps/controller/ControllerMainWindow.cpp#L576) | `~AsyncScreenFrameRateLogger` | 定义 | `~AsyncScreenFrameRateLogger()` | 停止相关活动并释放 AsyncScreenFrameRateLogger 实例拥有的资源。 |
| [L585](../src/apps/controller/ControllerMainWindow.cpp#L585) | `Run` | 定义 | `void Run(std::stop_token stopToken)` | 执行后台循环或调度 run 相关逻辑。 |
| [L672](../src/apps/controller/ControllerMainWindow.cpp#L672) | `AppendScreenFrameRateLog` | 定义 | `void AppendScreenFrameRateLog( const SessionDiagnosticsSnapshot& diagnostics)` | 实现 append screen frame rate log 对应的业务或工具逻辑。 |
| [L722](../src/apps/controller/ControllerMainWindow.cpp#L722) | `FormatBitrate` | 定义 | `QString FormatBitrate(std::uint64_t bitsPerSecond)` | 实现 format bitrate 对应的业务或工具逻辑。 |
| [L737](../src/apps/controller/ControllerMainWindow.cpp#L737) | `FormatByteCount` | 定义 | `QString FormatByteCount(std::uint64_t bytes)` | 实现 format byte count 对应的业务或工具逻辑。 |
| [L758](../src/apps/controller/ControllerMainWindow.cpp#L758) | `SampleWindowSuffix` | 定义 | `QString SampleWindowSuffix(std::uint32_t windowMs)` | 实现 sample window suffix 对应的业务或工具逻辑。 |
| [L765](../src/apps/controller/ControllerMainWindow.cpp#L765) | `LatestFrameTimingText` | 定义 | `QString LatestFrameTimingText( const RtpStreamStatsSnapshot& stream, const QString& action)` | 实现 latest frame timing text 对应的业务或工具逻辑。 |
| [L791](../src/apps/controller/ControllerMainWindow.cpp#L791) | `RouteDisplayName` | 定义 | `QString RouteDisplayName(const std::string& route)` | 实现 route display name 对应的业务或工具逻辑。 |
| [L805](../src/apps/controller/ControllerMainWindow.cpp#L805) | `SlotDisplayName` | 定义 | `QString SlotDisplayName(const std::string& slot, const std::string& kind)` | 实现 slot display name 对应的业务或工具逻辑。 |
| [L821](../src/apps/controller/ControllerMainWindow.cpp#L821) | `CandidateDisplayText` | 定义 | `QString CandidateDisplayText( const IceCandidateStatsSnapshot& candidate)` | 判断 candidate display text 相关逻辑。 |
| [L845](../src/apps/controller/ControllerMainWindow.cpp#L845) | `DiagnosticsMetricExplanation` | 定义 | `QString DiagnosticsMetricExplanation(const QString& label)` | 查询并返回 diagnostics metric explanation 相关逻辑。 |
| [L1014](../src/apps/controller/ControllerMainWindow.cpp#L1014) | `DiagnosticsMetricToolTip` | 定义 | `QString DiagnosticsMetricToolTip(const DiagnosticsChip& chip)` | 查询并返回 diagnostics metric tool tip 相关逻辑。 |
| [L1037](../src/apps/controller/ControllerMainWindow.cpp#L1037) | `DiagnosticsCardsWidget` | 定义 | `explicit DiagnosticsCardsWidget(QWidget* parent = nullptr) : QWidget(parent), layout_(new QVBoxLayout(this))` | 查询并返回 diagnostics cards widget 相关逻辑。 |
| [L1053](../src/apps/controller/ControllerMainWindow.cpp#L1053) | `SetSections` | 定义 | `void SetSections(const QVector<DiagnosticsSection>& sections, const QString& emptyText)` | 更新或应用 set sections 相关逻辑。 |
| [L1131](../src/apps/controller/ControllerMainWindow.cpp#L1131) | `Rebuild` | 定义 | `void Rebuild(const QVector<DiagnosticsSection>& sections, const QString& emptyText)` | 更新或应用 rebuild 相关逻辑。 |
| [L1377](../src/apps/controller/ControllerMainWindow.cpp#L1377) | `InitialFileSaveDirectory` | 定义 | `QString InitialFileSaveDirectory()` | 创建或初始化 initial file save directory 相关逻辑。 |
| [L1391](../src/apps/controller/ControllerMainWindow.cpp#L1391) | `PageHeightForWidth` | 定义 | `int PageHeightForWidth(QWidget* page) const` | 实现 page height for width 对应的业务或工具逻辑。 |
| [L1411](../src/apps/controller/ControllerMainWindow.cpp#L1411) | `RefreshCurrentHeight` | 定义 | `void RefreshCurrentHeight()` | 刷新 refresh current height 相关逻辑。 |
| [L1424](../src/apps/controller/ControllerMainWindow.cpp#L1424) | `sizeHint` | 定义 | `QSize sizeHint() const override` | 实现 size hint 对应的业务或工具逻辑。 |
| [L1430](../src/apps/controller/ControllerMainWindow.cpp#L1430) | `minimumSizeHint` | 定义 | `QSize minimumSizeHint() const override` | 实现 minimum size hint 对应的业务或工具逻辑。 |
| [L1437](../src/apps/controller/ControllerMainWindow.cpp#L1437) | `resizeEvent` | 定义 | `void resizeEvent(QResizeEvent* event) override` | 实现 resize event 对应的业务或工具逻辑。 |
| [L1456](../src/apps/controller/ControllerMainWindow.cpp#L1456) | `MediaDeviceButton` | 定义 | `explicit MediaDeviceButton(QWidget* parent = nullptr) : QPushButton(parent), arrowAnimation_(new QVariantAnimation(this))` | 实现 media device button 对应的业务或工具逻辑。 |
| [L1469](../src/apps/controller/ControllerMainWindow.cpp#L1469) | `SetDeviceMenuHandler` | 定义 | `void SetDeviceMenuHandler(std::function<void()> handler)` | 更新或应用 set device menu handler 相关逻辑。 |
| [L1477](../src/apps/controller/ControllerMainWindow.cpp#L1477) | `mousePressEvent` | 定义 | `void mousePressEvent(QMouseEvent* event) override` | 实现 mouse press event 对应的业务或工具逻辑。 |
| [L1492](../src/apps/controller/ControllerMainWindow.cpp#L1492) | `paintEvent` | 定义 | `void paintEvent(QPaintEvent* event) override` | 准备或呈现 paint event 相关逻辑。 |
| [L1518](../src/apps/controller/ControllerMainWindow.cpp#L1518) | `SetDeviceMenuOpen` | 定义 | `void SetDeviceMenuOpen(bool open)` | 更新或应用 set device menu open 相关逻辑。 |
| [L1540](../src/apps/controller/ControllerMainWindow.cpp#L1540) | `CreateMediaStateIcon` | 定义 | `QIcon CreateMediaStateIcon(MediaStateIcon type, bool active)` | 创建或初始化 create media state icon 相关逻辑。 |
| [L1590](../src/apps/controller/ControllerMainWindow.cpp#L1590) | `SetMediaStateButton` | 定义 | `void SetMediaStateButton(QPushButton* button, MediaStateIcon type, bool active, const QString& toolTip)` | 更新或应用 set media state button 相关逻辑。 |
| [L1641](../src/apps/controller/ControllerMainWindow.cpp#L1641) | `SetCameraGalleryStateButton` | 定义 | `void SetCameraGalleryStateButton(QPushButton* button, bool camerasAvailable, bool galleryVisible, const QString& toolTip)` | 更新或应用 set camera gallery state button 相关逻辑。 |
| [L1669](../src/apps/controller/ControllerMainWindow.cpp#L1669) | `CreateMemberMediaIndicator` | 定义 | `QToolButton* CreateMemberMediaIndicator( QWidget* parent, MediaStateIcon type, bool active, const QString& toolTip, bool hadPreviousState = false, bool previousActive = false)` | 创建或初始化 create member media indicator 相关逻辑。 |
| [L1737](../src/apps/controller/ControllerMainWindow.cpp#L1737) | `CreateRoomStatusIndicator` | 定义 | `QLabel* CreateRoomStatusIndicator(QWidget* parent, RoomStatusIcon type)` | 创建或初始化 create room status indicator 相关逻辑。 |
| [L1784](../src/apps/controller/ControllerMainWindow.cpp#L1784) | `SetBusyStatusAnimation` | 定义 | `void SetBusyStatusAnimation(QLabel* label, bool busy)` | 更新或应用 set busy status animation 相关逻辑。 |
| [L1816](../src/apps/controller/ControllerMainWindow.cpp#L1816) | `AnimateSmallUiChange` | 定义 | `void AnimateSmallUiChange(QWidget* widget)` | 实现 animate small ui change 对应的业务或工具逻辑。 |
| [L2590](../src/apps/controller/ControllerMainWindow.cpp#L2590) | `DrawNavigationIcon` | 定义 | `QPixmap DrawNavigationIcon(NavigationIcon icon, const QColor& color)` | 准备或呈现 draw navigation icon 相关逻辑。 |
| [L2634](../src/apps/controller/ControllerMainWindow.cpp#L2634) | `MakeNavigationIcon` | 定义 | `QIcon MakeNavigationIcon(NavigationIcon icon, bool dark)` | 创建或初始化 make navigation icon 相关逻辑。 |
| [L2652](../src/apps/controller/ControllerMainWindow.cpp#L2652) | `MakePageSurface` | 定义 | `QScrollArea* MakePageSurface(const QString& title, const QString& subtitle, QWidget* parent, QVBoxLayout** pageLayout)` | 创建或初始化 make page surface 相关逻辑。 |
| [L2681](../src/apps/controller/ControllerMainWindow.cpp#L2681) | `MakeNavigationButton` | 定义 | `QPushButton* MakeNavigationButton(const QString& text, NavigationIcon icon, bool active, QWidget* parent)` | 创建或初始化 make navigation button 相关逻辑。 |
| [L2751](../src/apps/controller/ControllerMainWindow.cpp#L2751) | `MakeDivider` | 定义 | `QFrame* MakeDivider(QWidget* parent)` | 创建或初始化 make divider 相关逻辑。 |
| [L2759](../src/apps/controller/ControllerMainWindow.cpp#L2759) | `AddRoomCapacityItems` | 定义 | `void AddRoomCapacityItems(QComboBox* comboBox)` | 实现 add room capacity items 对应的业务或工具逻辑。 |
| [L2775](../src/apps/controller/ControllerMainWindow.cpp#L2775) | `MemberDisplayName` | 定义 | `QString MemberDisplayName(const RoomSnapshot& room, const std::string& deviceId, const std::string& localDeviceId)` | 实现 member display name 对应的业务或工具逻辑。 |
| [L2799](../src/apps/controller/ControllerMainWindow.cpp#L2799) | `ConnectivityDebugName` | 定义 | `QString ConnectivityDebugName(SessionConnectivityState state)` | 建立连接 connectivity debug name 相关逻辑。 |
| [L2816](../src/apps/controller/ControllerMainWindow.cpp#L2816) | `RoomMembershipDebugName` | 定义 | `QString RoomMembershipDebugName(RoomMembershipState state)` | 实现 room membership debug name 对应的业务或工具逻辑。 |
| [L2835](../src/apps/controller/ControllerMainWindow.cpp#L2835) | `PeerConnectionDebugName` | 定义 | `QString PeerConnectionDebugName(RoomPeerConnectionState state)` | 实现 peer connection debug name 对应的业务或工具逻辑。 |
| [L2858](../src/apps/controller/ControllerMainWindow.cpp#L2858) | `MediaDeviceSelectionDebugName` | 定义 | `QString MediaDeviceSelectionDebugName( MediaDeviceSelectionState state)` | 实现 media device selection debug name 对应的业务或工具逻辑。 |
| [L2874](../src/apps/controller/ControllerMainWindow.cpp#L2874) | `MediaDeviceDebugName` | 定义 | `QString MediaDeviceDebugName( const MediaDeviceCategorySnapshot& category, const std::string& deviceId)` | 实现 media device debug name 对应的业务或工具逻辑。 |
| [L2898](../src/apps/controller/ControllerMainWindow.cpp#L2898) | `MediaDeviceCategoryDebugText` | 定义 | `QString MediaDeviceCategoryDebugText( const MediaDeviceCategorySnapshot& category)` | 实现 media device category debug text 对应的业务或工具逻辑。 |

## `src/apps/controller/ControllerMainWindow.DecoderBenchmark.inc`

[打开源码](../src/apps/controller/ControllerMainWindow.DecoderBenchmark.inc) · **文件作用：** `ControllerMainWindow` 的实现切片，集中实现 decoder benchmark 相关逻辑；成员状态仍定义在所属头文件中。

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L4](../src/apps/controller/ControllerMainWindow.DecoderBenchmark.inc#L4) | `ControllerMainWindow::RefreshDecoderBenchmarkSummary` | 定义 | `void ControllerMainWindow::RefreshDecoderBenchmarkSummary()` | SPDX-License-Identifier: GPL-3.0-only Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd) |
| [L217](../src/apps/controller/ControllerMainWindow.DecoderBenchmark.inc#L217) | `ControllerMainWindow::RefreshDecoderHardwareSelectionAvailability` | 定义 | `void ControllerMainWindow::RefreshDecoderHardwareSelectionAvailability()` | 刷新 refresh decoder hardware selection availability 相关逻辑。 |
| [L273](../src/apps/controller/ControllerMainWindow.DecoderBenchmark.inc#L273) | `ControllerMainWindow::StartDecoderBenchmark` | 定义 | `void ControllerMainWindow::StartDecoderBenchmark(bool manualRequest)` | 启动 start decoder benchmark 相关逻辑。 |
| [L356](../src/apps/controller/ControllerMainWindow.DecoderBenchmark.inc#L356) | `ControllerMainWindow::FinishDecoderBenchmark` | 定义 | `void ControllerMainWindow::FinishDecoderBenchmark(int exitCode)` | 停止 finish decoder benchmark 相关逻辑。 |

## `src/apps/controller/ControllerMainWindow.DiagnosticsInbound.inc`

[打开源码](../src/apps/controller/ControllerMainWindow.DiagnosticsInbound.inc) · **文件作用：** 所属主函数的连续实现片段；这里只包含该函数的局部流程，不单独声明成员变量或顶层函数。

当前规则未提取到公开类型、成员/文件级变量或函数；该文件可能只包含宏、模板片段、资源数据或条件编译内容，请直接打开源码查看。

## `src/apps/controller/ControllerMainWindow.DiagnosticsInput.inc`

[打开源码](../src/apps/controller/ControllerMainWindow.DiagnosticsInput.inc) · **文件作用：** `ControllerMainWindow` 的实现切片，集中实现 diagnostics input 相关逻辑；成员状态仍定义在所属头文件中。

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L4](../src/apps/controller/ControllerMainWindow.DiagnosticsInput.inc#L4) | `ControllerMainWindow::RefreshDiagnosticsUi` | 定义 | `void ControllerMainWindow::RefreshDiagnosticsUi()` | SPDX-License-Identifier: GPL-3.0-only Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd) |

## `src/apps/controller/ControllerMainWindow.DiagnosticsOutbound.inc`

[打开源码](../src/apps/controller/ControllerMainWindow.DiagnosticsOutbound.inc) · **文件作用：** 所属主函数的连续实现片段；这里只包含该函数的局部流程，不单独声明成员变量或顶层函数。

当前规则未提取到公开类型、成员/文件级变量或函数；该文件可能只包含宏、模板片段、资源数据或条件编译内容，请直接打开源码查看。

## `src/apps/controller/ControllerMainWindow.DiagnosticsPeers.inc`

[打开源码](../src/apps/controller/ControllerMainWindow.DiagnosticsPeers.inc) · **文件作用：** 所属主函数的连续实现片段；这里只包含该函数的局部流程，不单独声明成员变量或顶层函数。

当前规则未提取到公开类型、成员/文件级变量或函数；该文件可能只包含宏、模板片段、资源数据或条件编译内容，请直接打开源码查看。

## `src/apps/controller/ControllerMainWindow.DiagnosticsPublish.inc`

[打开源码](../src/apps/controller/ControllerMainWindow.DiagnosticsPublish.inc) · **文件作用：** 所属主函数的连续实现片段；这里只包含该函数的局部流程，不单独声明成员变量或顶层函数。

当前规则未提取到公开类型、成员/文件级变量或函数；该文件可能只包含宏、模板片段、资源数据或条件编译内容，请直接打开源码查看。

## `src/apps/controller/ControllerMainWindow.EncoderBenchmark.inc`

[打开源码](../src/apps/controller/ControllerMainWindow.EncoderBenchmark.inc) · **文件作用：** `ControllerMainWindow` 的实现切片，集中实现 encoder benchmark 相关逻辑；成员状态仍定义在所属头文件中。

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L4](../src/apps/controller/ControllerMainWindow.EncoderBenchmark.inc#L4) | `ControllerMainWindow::RefreshEncoderBenchmarkSummary` | 定义 | `void ControllerMainWindow::RefreshEncoderBenchmarkSummary()` | SPDX-License-Identifier: GPL-3.0-only Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd) |
| [L150](../src/apps/controller/ControllerMainWindow.EncoderBenchmark.inc#L150) | `ControllerMainWindow::StartEncoderBenchmark` | 定义 | `void ControllerMainWindow::StartEncoderBenchmark(bool manualRequest)` | 启动 start encoder benchmark 相关逻辑。 |
| [L224](../src/apps/controller/ControllerMainWindow.EncoderBenchmark.inc#L224) | `ControllerMainWindow::FinishEncoderBenchmark` | 定义 | `void ControllerMainWindow::FinishEncoderBenchmark(int exitCode)` | 停止 finish encoder benchmark 相关逻辑。 |

## `src/apps/controller/ControllerMainWindow.EngineMedia.inc`

[打开源码](../src/apps/controller/ControllerMainWindow.EngineMedia.inc) · **文件作用：** `ControllerMainWindow` 的实现切片，集中实现 engine media 相关逻辑；成员状态仍定义在所属头文件中。

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L4](../src/apps/controller/ControllerMainWindow.EngineMedia.inc#L4) | `ControllerMainWindow::InitializeEngine` | 定义 | `bool ControllerMainWindow::InitializeEngine()` | SPDX-License-Identifier: GPL-3.0-only Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd) |
| [L55](../src/apps/controller/ControllerMainWindow.EngineMedia.inc#L55) | `ControllerMainWindow::CompleteEngineInitialization` | 定义 | `void ControllerMainWindow::CompleteEngineInitialization( const SessionCommandResult& startResult)` | 实现 complete engine initialization 对应的业务或工具逻辑。 |
| [L96](../src/apps/controller/ControllerMainWindow.EngineMedia.inc#L96) | `ControllerMainWindow::PersistHardwareCapabilityCache` | 定义 | `void ControllerMainWindow::PersistHardwareCapabilityCache()` | 保存或写入 persist hardware capability cache 相关逻辑。 |
| [L150](../src/apps/controller/ControllerMainWindow.EngineMedia.inc#L150) | `ControllerMainWindow::ApplyVideoPipelineSettingsFromUi` | 定义 | `void ControllerMainWindow::ApplyVideoPipelineSettingsFromUi( bool showFeedback, const QString& changedSettingName)` | 更新或应用 apply video pipeline settings from ui 相关逻辑。 |
| [L273](../src/apps/controller/ControllerMainWindow.EngineMedia.inc#L273) | `ControllerMainWindow::UpdateVideoPipelineSettingsAvailability` | 定义 | `void ControllerMainWindow::UpdateVideoPipelineSettingsAvailability( const SessionEngineSnapshot& snapshot)` | 更新或应用 update video pipeline settings availability 相关逻辑。 |
| [L317](../src/apps/controller/ControllerMainWindow.EngineMedia.inc#L317) | `ControllerMainWindow::UpdateLocalMediaDevicesUi` | 定义 | `void ControllerMainWindow::UpdateLocalMediaDevicesUi( const SessionEngineSnapshot& snapshot)` | 更新或应用 update local media devices ui 相关逻辑。 |

## `src/apps/controller/ControllerMainWindow.h`

[打开源码](../src/apps/controller/ControllerMainWindow.h) · **文件作用：** 声明 controller main window 相关类型、接口、配置和成员状态。

### 类型

| 行 | 类型 | 种类 | 作用 |
|---:|---|---|---|
| [L26](../src/apps/controller/ControllerMainWindow.h#L26) | `WindowsInputExecutor` | class | 定义 WindowsInputExecutor 的 class 类型和相关状态。 |
| [L28](../src/apps/controller/ControllerMainWindow.h#L28) | `FileTransferController` | class | 定义 FileTransferController 的 class 类型和相关状态。 |
| [L32](../src/apps/controller/ControllerMainWindow.h#L32) | `QComboBox` | class | 定义 QComboBox 的 class 类型和相关状态。 |
| [L33](../src/apps/controller/ControllerMainWindow.h#L33) | `QAction` | class | 定义 QAction 的 class 类型和相关状态。 |
| [L34](../src/apps/controller/ControllerMainWindow.h#L34) | `QCloseEvent` | class | 定义 QCloseEvent 的 class 类型和相关状态。 |
| [L35](../src/apps/controller/ControllerMainWindow.h#L35) | `QEvent` | class | 定义 QEvent 的 class 类型和相关状态。 |
| [L36](../src/apps/controller/ControllerMainWindow.h#L36) | `QFrame` | class | 定义 QFrame 的 class 类型和相关状态。 |
| [L37](../src/apps/controller/ControllerMainWindow.h#L37) | `QGraphicsOpacityEffect` | class | 定义 QGraphicsOpacityEffect 的 class 类型和相关状态。 |
| [L38](../src/apps/controller/ControllerMainWindow.h#L38) | `QLabel` | class | 定义 QLabel 的 class 类型和相关状态。 |
| [L39](../src/apps/controller/ControllerMainWindow.h#L39) | `QLineEdit` | class | 定义 QLineEdit 的 class 类型和相关状态。 |
| [L40](../src/apps/controller/ControllerMainWindow.h#L40) | `QListWidget` | class | 定义 QListWidget 的 class 类型和相关状态。 |
| [L41](../src/apps/controller/ControllerMainWindow.h#L41) | `QParallelAnimationGroup` | class | 定义 QParallelAnimationGroup 的 class 类型和相关状态。 |
| [L42](../src/apps/controller/ControllerMainWindow.h#L42) | `QPropertyAnimation` | class | 定义 QPropertyAnimation 的 class 类型和相关状态。 |
| [L43](../src/apps/controller/ControllerMainWindow.h#L43) | `QProcess` | class | 定义 QProcess 的 class 类型和相关状态。 |
| [L44](../src/apps/controller/ControllerMainWindow.h#L44) | `QPoint` | class | 定义 QPoint 的 class 类型和相关状态。 |
| [L45](../src/apps/controller/ControllerMainWindow.h#L45) | `QPushButton` | class | 定义 QPushButton 的 class 类型和相关状态。 |
| [L46](../src/apps/controller/ControllerMainWindow.h#L46) | `QResizeEvent` | class | 定义 QResizeEvent 的 class 类型和相关状态。 |
| [L47](../src/apps/controller/ControllerMainWindow.h#L47) | `QStackedWidget` | class | 定义 QStackedWidget 的 class 类型和相关状态。 |
| [L48](../src/apps/controller/ControllerMainWindow.h#L48) | `QSystemTrayIcon` | class | 定义 QSystemTrayIcon 的 class 类型和相关状态。 |
| [L49](../src/apps/controller/ControllerMainWindow.h#L49) | `QTimer` | class | 定义 QTimer 的 class 类型和相关状态。 |
| [L50](../src/apps/controller/ControllerMainWindow.h#L50) | `QToolButton` | class | 定义 QToolButton 的 class 类型和相关状态。 |
| [L51](../src/apps/controller/ControllerMainWindow.h#L51) | `QVBoxLayout` | class | 定义 QVBoxLayout 的 class 类型和相关状态。 |
| [L55](../src/apps/controller/ControllerMainWindow.h#L55) | `CameraWindow` | class | 定义 CameraWindow 的 class 类型和相关状态。 |
| [L56](../src/apps/controller/ControllerMainWindow.h#L56) | `FileTransferWindow` | class | 定义 FileTransferWindow 的 class 类型和相关状态。 |
| [L57](../src/apps/controller/ControllerMainWindow.h#L57) | `RemoteSessionWindow` | class | 定义 RemoteSessionWindow 的 class 类型和相关状态。 |
| [L58](../src/apps/controller/ControllerMainWindow.h#L58) | `RoomCameraWindow` | class | 定义 RoomCameraWindow 的 class 类型和相关状态。 |
| [L60](../src/apps/controller/ControllerMainWindow.h#L60) | `ControllerMainWindow` | class | 定义 ControllerMainWindow 的 class 类型和相关状态。 |

### 成员与文件级变量

| 行 | 变量 | 声明 | 作用 |
|---:|---|---|---|
| [L26](../src/apps/controller/ControllerMainWindow.h#L26) | `WindowsInputExecutor` | `class WindowsInputExecutor;` | 保存 windows input executor 相关配置或运行状态。 |
| [L28](../src/apps/controller/ControllerMainWindow.h#L28) | `FileTransferController` | `class FileTransferController;` | 保存 file transfer controller 相关配置或运行状态。 |
| [L32](../src/apps/controller/ControllerMainWindow.h#L32) | `QComboBox` | `class QComboBox;` | 保存 q combo box 相关配置或运行状态。 |
| [L33](../src/apps/controller/ControllerMainWindow.h#L33) | `QAction` | `class QAction;` | 保存 q action 相关配置或运行状态。 |
| [L34](../src/apps/controller/ControllerMainWindow.h#L34) | `QCloseEvent` | `class QCloseEvent;` | 保存 q close event 相关配置或运行状态。 |
| [L35](../src/apps/controller/ControllerMainWindow.h#L35) | `QEvent` | `class QEvent;` | 保存 q event 相关配置或运行状态。 |
| [L36](../src/apps/controller/ControllerMainWindow.h#L36) | `QFrame` | `class QFrame;` | 保存媒体帧、图像或缓冲资源：q frame。 |
| [L37](../src/apps/controller/ControllerMainWindow.h#L37) | `QGraphicsOpacityEffect` | `class QGraphicsOpacityEffect;` | 保存 q graphics opacity effect 相关配置或运行状态。 |
| [L38](../src/apps/controller/ControllerMainWindow.h#L38) | `QLabel` | `class QLabel;` | 保存路径、地址或显示名称：q label。 |
| [L39](../src/apps/controller/ControllerMainWindow.h#L39) | `QLineEdit` | `class QLineEdit;` | 保存 q line edit 相关配置或运行状态。 |
| [L40](../src/apps/controller/ControllerMainWindow.h#L40) | `QListWidget` | `class QListWidget;` | 保存 q list widget 相关配置或运行状态。 |
| [L41](../src/apps/controller/ControllerMainWindow.h#L41) | `QParallelAnimationGroup` | `class QParallelAnimationGroup;` | 保存 q parallel animation group 相关配置或运行状态。 |
| [L42](../src/apps/controller/ControllerMainWindow.h#L42) | `QPropertyAnimation` | `class QPropertyAnimation;` | 保存 q property animation 相关配置或运行状态。 |
| [L43](../src/apps/controller/ControllerMainWindow.h#L43) | `QProcess` | `class QProcess;` | 保存 q process 相关配置或运行状态。 |
| [L44](../src/apps/controller/ControllerMainWindow.h#L44) | `QPoint` | `class QPoint;` | 保存 q point 相关配置或运行状态。 |
| [L45](../src/apps/controller/ControllerMainWindow.h#L45) | `QPushButton` | `class QPushButton;` | 保存 q push button 相关配置或运行状态。 |
| [L46](../src/apps/controller/ControllerMainWindow.h#L46) | `QResizeEvent` | `class QResizeEvent;` | 保存 q resize event 相关配置或运行状态。 |
| [L47](../src/apps/controller/ControllerMainWindow.h#L47) | `QStackedWidget` | `class QStackedWidget;` | 保存 q stacked widget 相关配置或运行状态。 |
| [L48](../src/apps/controller/ControllerMainWindow.h#L48) | `QSystemTrayIcon` | `class QSystemTrayIcon;` | 保存 q system tray icon 相关配置或运行状态。 |
| [L49](../src/apps/controller/ControllerMainWindow.h#L49) | `QTimer` | `class QTimer;` | 保存定时、截止或超时状态：q timer。 |
| [L50](../src/apps/controller/ControllerMainWindow.h#L50) | `QToolButton` | `class QToolButton;` | 保存 q tool button 相关配置或运行状态。 |
| [L51](../src/apps/controller/ControllerMainWindow.h#L51) | `QVBoxLayout` | `class QVBoxLayout;` | 保存 qv box layout 相关配置或运行状态。 |
| [L55](../src/apps/controller/ControllerMainWindow.h#L55) | `CameraWindow` | `class CameraWindow;` | 保存 camera window 相关配置或运行状态。 |
| [L56](../src/apps/controller/ControllerMainWindow.h#L56) | `FileTransferWindow` | `class FileTransferWindow;` | 保存 file transfer window 相关配置或运行状态。 |
| [L57](../src/apps/controller/ControllerMainWindow.h#L57) | `RemoteSessionWindow` | `class RemoteSessionWindow;` | 保存 remote session window 相关配置或运行状态。 |
| [L58](../src/apps/controller/ControllerMainWindow.h#L58) | `RoomCameraWindow` | `class RoomCameraWindow;` | 保存 room camera window 相关配置或运行状态。 |
| [L196](../src/apps/controller/ControllerMainWindow.h#L196) | `inputExecutor_` | `std::unique_ptr<WindowsInputExecutor> inputExecutor_;` | 保存 input executor 相关配置或运行状态。 |
| [L197](../src/apps/controller/ControllerMainWindow.h#L197) | `fileTransferController_` | `std::unique_ptr<app::FileTransferController> fileTransferController_;` | 保存 file transfer controller 相关配置或运行状态。 |
| [L198](../src/apps/controller/ControllerMainWindow.h#L198) | `clipboardController_` | `std::unique_ptr<app::ClipboardController> clipboardController_;` | 保存 clipboard controller 相关配置或运行状态。 |
| [L200](../src/apps/controller/ControllerMainWindow.h#L200) | `softwareUpdateController_` | `softwareUpdateController_;` | 保存 software update controller 相关配置或运行状态。 |
| [L201](../src/apps/controller/ControllerMainWindow.h#L201) | `softwareUpdatePromptOpen_` | `bool softwareUpdatePromptOpen_ = false;` | 保存能力或开关状态：software update prompt open。 |
| [L202](../src/apps/controller/ControllerMainWindow.h#L202) | `engine_` | `std::unique_ptr<ISessionEngine> engine_;` | 保存 engine 相关配置或运行状态。 |
| [L203](../src/apps/controller/ControllerMainWindow.h#L203) | `engineStartThread_` | `std::jthread engineStartThread_;` | 拥有后台执行线程或工作器：engine start thread。 |
| [L204](../src/apps/controller/ControllerMainWindow.h#L204) | `deviceIdEdit_` | `QLineEdit* deviceIdEdit_ = nullptr;` | 保存 device id edit 相关配置或运行状态。 |
| [L205](../src/apps/controller/ControllerMainWindow.h#L205) | `verificationCodeEdit_` | `QLineEdit* verificationCodeEdit_ = nullptr;` | 保存 verification code edit 相关配置或运行状态。 |
| [L206](../src/apps/controller/ControllerMainWindow.h#L206) | `connectButton_` | `QPushButton* connectButton_ = nullptr;` | 保存 connect button 相关配置或运行状态。 |
| [L207](../src/apps/controller/ControllerMainWindow.h#L207) | `assistedSessionTimeoutTimer_` | `QTimer* assistedSessionTimeoutTimer_ = nullptr;` | 保存定时、截止或超时状态：assisted session timeout timer。 |
| [L208](../src/apps/controller/ControllerMainWindow.h#L208) | `assistedSessionPending_` | `bool assistedSessionPending_ = false;` | 保存待处理队列或请求：assisted session pending。 |
| [L209](../src/apps/controller/ControllerMainWindow.h#L209) | `assistedSessionActive_` | `bool assistedSessionActive_ = false;` | 保存能力或开关状态：assisted session active。 |
| [L210](../src/apps/controller/ControllerMainWindow.h#L210) | `assistedSessionCancellationPending_` | `bool assistedSessionCancellationPending_ = false;` | 保存待处理队列或请求：assisted session cancellation pending。 |
| [L211](../src/apps/controller/ControllerMainWindow.h#L211) | `assistedSessionTimedOut_` | `bool assistedSessionTimedOut_ = false;` | 保存 assisted session timed out 相关配置或运行状态。 |
| [L212](../src/apps/controller/ControllerMainWindow.h#L212) | `ownedDeviceSessionPending_` | `bool ownedDeviceSessionPending_ = false;` | 保存待处理队列或请求：owned device session pending。 |
| [L213](../src/apps/controller/ControllerMainWindow.h#L213) | `lastDirectSessionToastError_` | `QString lastDirectSessionToastError_;` | 保存最近错误或失败原因：last direct session toast error。 |
| [L214](../src/apps/controller/ControllerMainWindow.h#L214) | `runtimeStatus_` | `QLabel* runtimeStatus_ = nullptr;` | 保存状态机当前状态：runtime status。 |
| [L215](../src/apps/controller/ControllerMainWindow.h#L215) | `decoderStatus_` | `QLabel* decoderStatus_ = nullptr;` | 保存状态机当前状态：decoder status。 |
| [L216](../src/apps/controller/ControllerMainWindow.h#L216) | `connectivityPill_` | `QLabel* connectivityPill_ = nullptr;` | 保存 connectivity pill 相关配置或运行状态。 |
| [L217](../src/apps/controller/ControllerMainWindow.h#L217) | `profileCard_` | `QFrame* profileCard_ = nullptr;` | 保存 profile card 相关配置或运行状态。 |
| [L218](../src/apps/controller/ControllerMainWindow.h#L218) | `profileAvatar_` | `QLabel* profileAvatar_ = nullptr;` | 保存 profile avatar 相关配置或运行状态。 |
| [L219](../src/apps/controller/ControllerMainWindow.h#L219) | `profileName_` | `QLabel* profileName_ = nullptr;` | 保存路径、地址或显示名称：profile name。 |
| [L220](../src/apps/controller/ControllerMainWindow.h#L220) | `serviceStatus_` | `QLabel* serviceStatus_ = nullptr;` | 保存状态机当前状态：service status。 |
| [L221](../src/apps/controller/ControllerMainWindow.h#L221) | `profileUpdateButton_` | `QToolButton* profileUpdateButton_ = nullptr;` | 保存 profile update button 相关配置或运行状态。 |
| [L222](../src/apps/controller/ControllerMainWindow.h#L222) | `accountMenu_` | `QFrame* accountMenu_ = nullptr;` | 保存 account menu 相关配置或运行状态。 |
| [L223](../src/apps/controller/ControllerMainWindow.h#L223) | `accountMenuAvatar_` | `QLabel* accountMenuAvatar_ = nullptr;` | 保存 account menu avatar 相关配置或运行状态。 |
| [L224](../src/apps/controller/ControllerMainWindow.h#L224) | `accountMenuName_` | `QLabel* accountMenuName_ = nullptr;` | 保存路径、地址或显示名称：account menu name。 |
| [L225](../src/apps/controller/ControllerMainWindow.h#L225) | `accountMenuDetail_` | `QLabel* accountMenuDetail_ = nullptr;` | 保存 account menu detail 相关配置或运行状态。 |
| [L226](../src/apps/controller/ControllerMainWindow.h#L226) | `softwareUpdateAction_` | `QPushButton* softwareUpdateAction_ = nullptr;` | 保存 software update action 相关配置或运行状态。 |
| [L227](../src/apps/controller/ControllerMainWindow.h#L227) | `softwareUpdateStatusLabel_` | `QLabel* softwareUpdateStatusLabel_ = nullptr;` | 保存路径、地址或显示名称：software update status label。 |
| [L228](../src/apps/controller/ControllerMainWindow.h#L228) | `softwareUpdateCheckButton_` | `QPushButton* softwareUpdateCheckButton_ = nullptr;` | 保存 software update check button 相关配置或运行状态。 |
| [L229](../src/apps/controller/ControllerMainWindow.h#L229) | `accountMenuOpacity_` | `QGraphicsOpacityEffect* accountMenuOpacity_ = nullptr;` | 保存 account menu opacity 相关配置或运行状态。 |
| [L230](../src/apps/controller/ControllerMainWindow.h#L230) | `accountMenuMotionTimer_` | `QTimer* accountMenuMotionTimer_ = nullptr;` | 保存定时、截止或超时状态：account menu motion timer。 |
| [L231](../src/apps/controller/ControllerMainWindow.h#L231) | `accountMenuMotionClock_` | `QElapsedTimer accountMenuMotionClock_;` | 保护跨线程共享状态：account menu motion clock。 |
| [L232](../src/apps/controller/ControllerMainWindow.h#L232) | `accountMenuMotionStart_` | `QPoint accountMenuMotionStart_;` | 保存 account menu motion start 相关配置或运行状态。 |
| [L233](../src/apps/controller/ControllerMainWindow.h#L233) | `accountMenuMotionTarget_` | `QPoint accountMenuMotionTarget_;` | 保存 account menu motion target 相关配置或运行状态。 |
| [L234](../src/apps/controller/ControllerMainWindow.h#L234) | `accountMenuMotionDurationMs_` | `int accountMenuMotionDurationMs_ = 0;` | 保存 account menu motion duration ms 相关配置或运行状态。 |
| [L235](../src/apps/controller/ControllerMainWindow.h#L235) | `accountMenuMotionHiding_` | `bool accountMenuMotionHiding_ = false;` | 保存 account menu motion hiding 相关配置或运行状态。 |
| [L236](../src/apps/controller/ControllerMainWindow.h#L236) | `accountMenuHoverTimer_` | `QTimer* accountMenuHoverTimer_ = nullptr;` | 保存定时、截止或超时状态：account menu hover timer。 |
| [L237](../src/apps/controller/ControllerMainWindow.h#L237) | `deviceLoginPrompt_` | `QFrame* deviceLoginPrompt_ = nullptr;` | 保存 device login prompt 相关配置或运行状态。 |
| [L238](../src/apps/controller/ControllerMainWindow.h#L238) | `deviceLoginStatus_` | `QLabel* deviceLoginStatus_ = nullptr;` | 保存状态机当前状态：device login status。 |
| [L239](../src/apps/controller/ControllerMainWindow.h#L239) | `deviceLoginButton_` | `QPushButton* deviceLoginButton_ = nullptr;` | 保存 device login button 相关配置或运行状态。 |
| [L240](../src/apps/controller/ControllerMainWindow.h#L240) | `deviceIdentityCard_` | `QFrame* deviceIdentityCard_ = nullptr;` | 保存 device identity card 相关配置或运行状态。 |
| [L241](../src/apps/controller/ControllerMainWindow.h#L241) | `deviceDirectAssistCard_` | `QFrame* deviceDirectAssistCard_ = nullptr;` | 保存 device direct assist card 相关配置或运行状态。 |
| [L242](../src/apps/controller/ControllerMainWindow.h#L242) | `deviceStatusCard_` | `QFrame* deviceStatusCard_ = nullptr;` | 保存 device status card 相关配置或运行状态。 |
| [L243](../src/apps/controller/ControllerMainWindow.h#L243) | `deviceAccountLabel_` | `QLabel* deviceAccountLabel_ = nullptr;` | 保存路径、地址或显示名称：device account label。 |
| [L244](../src/apps/controller/ControllerMainWindow.h#L244) | `localDeviceId_` | `QLabel* localDeviceId_ = nullptr;` | 保存身份或作用域标识：local device id。 |
| [L245](../src/apps/controller/ControllerMainWindow.h#L245) | `copyDeviceIdButton_` | `QPushButton* copyDeviceIdButton_ = nullptr;` | 保存 copy device id button 相关配置或运行状态。 |
| [L246](../src/apps/controller/ControllerMainWindow.h#L246) | `localVerificationCode_` | `QLabel* localVerificationCode_ = nullptr;` | 保存 local verification code 相关配置或运行状态。 |
| [L247](../src/apps/controller/ControllerMainWindow.h#L247) | `copyVerificationCodeButton_` | `QPushButton* copyVerificationCodeButton_ = nullptr;` | 保存 copy verification code button 相关配置或运行状态。 |
| [L248](../src/apps/controller/ControllerMainWindow.h#L248) | `shareLocalCredentialsButton_` | `QPushButton* shareLocalCredentialsButton_ = nullptr;` | 保存 share local credentials button 相关配置或运行状态。 |
| [L249](../src/apps/controller/ControllerMainWindow.h#L249) | `connectionHint_` | `QLabel* connectionHint_ = nullptr;` | 保存 connection hint 相关配置或运行状态。 |
| [L250](../src/apps/controller/ControllerMainWindow.h#L250) | `createRoomCapacity_` | `QComboBox* createRoomCapacity_ = nullptr;` | 保存 create room capacity 相关配置或运行状态。 |
| [L251](../src/apps/controller/ControllerMainWindow.h#L251) | `createRoomButton_` | `QPushButton* createRoomButton_ = nullptr;` | 保存 create room button 相关配置或运行状态。 |
| [L252](../src/apps/controller/ControllerMainWindow.h#L252) | `roomIdEdit_` | `QLineEdit* roomIdEdit_ = nullptr;` | 保存 room id edit 相关配置或运行状态。 |
| [L253](../src/apps/controller/ControllerMainWindow.h#L253) | `joinRoomButton_` | `QPushButton* joinRoomButton_ = nullptr;` | 保存 join room button 相关配置或运行状态。 |
| [L254](../src/apps/controller/ControllerMainWindow.h#L254) | `roomActionHint_` | `QLabel* roomActionHint_ = nullptr;` | 保存 room action hint 相关配置或运行状态。 |
| [L255](../src/apps/controller/ControllerMainWindow.h#L255) | `roomEntryPanel_` | `QFrame* roomEntryPanel_ = nullptr;` | 保存 room entry panel 相关配置或运行状态。 |
| [L256](../src/apps/controller/ControllerMainWindow.h#L256) | `roomPanel_` | `QFrame* roomPanel_ = nullptr;` | 保存 room panel 相关配置或运行状态。 |
| [L257](../src/apps/controller/ControllerMainWindow.h#L257) | `roomWorkspaceStack_` | `QStackedWidget* roomWorkspaceStack_ = nullptr;` | 保存 room workspace stack 相关配置或运行状态。 |
| [L258](../src/apps/controller/ControllerMainWindow.h#L258) | `roomWorkspaceAnimation_` | `QParallelAnimationGroup* roomWorkspaceAnimation_ = nullptr;` | 保存 room workspace animation 相关配置或运行状态。 |
| [L259](../src/apps/controller/ControllerMainWindow.h#L259) | `roomWorkspaceTransitionLayer_` | `QWidget* roomWorkspaceTransitionLayer_ = nullptr;` | 保存 room workspace transition layer 相关配置或运行状态。 |
| [L260](../src/apps/controller/ControllerMainWindow.h#L260) | `roomWorkspaceTargetActive_` | `bool roomWorkspaceTargetActive_ = false;` | 保存能力或开关状态：room workspace target active。 |
| [L261](../src/apps/controller/ControllerMainWindow.h#L261) | `roomWorkspaceTransitionPending_` | `bool roomWorkspaceTransitionPending_ = false;` | 保存待处理队列或请求：room workspace transition pending。 |
| [L262](../src/apps/controller/ControllerMainWindow.h#L262) | `roomWorkspaceTransitionRequest_` | `quint64 roomWorkspaceTransitionRequest_ = 0;` | 保存 room workspace transition request 相关配置或运行状态。 |
| [L263](../src/apps/controller/ControllerMainWindow.h#L263) | `roomIdLabel_` | `QLabel* roomIdLabel_ = nullptr;` | 保存路径、地址或显示名称：room id label。 |
| [L264](../src/apps/controller/ControllerMainWindow.h#L264) | `copyRoomIdButton_` | `QPushButton* copyRoomIdButton_ = nullptr;` | 保存 copy room id button 相关配置或运行状态。 |
| [L265](../src/apps/controller/ControllerMainWindow.h#L265) | `roomOccupancyLabel_` | `QLabel* roomOccupancyLabel_ = nullptr;` | 保存路径、地址或显示名称：room occupancy label。 |
| [L266](../src/apps/controller/ControllerMainWindow.h#L266) | `roomOwnerLabel_` | `QLabel* roomOwnerLabel_ = nullptr;` | 保存路径、地址或显示名称：room owner label。 |
| [L267](../src/apps/controller/ControllerMainWindow.h#L267) | `roomScreenSharerLabel_` | `QLabel* roomScreenSharerLabel_ = nullptr;` | 保存路径、地址或显示名称：room screen sharer label。 |
| [L268](../src/apps/controller/ControllerMainWindow.h#L268) | `roomControllerLabel_` | `QLabel* roomControllerLabel_ = nullptr;` | 保存路径、地址或显示名称：room controller label。 |
| [L269](../src/apps/controller/ControllerMainWindow.h#L269) | `roomPeerConnectivityLabel_` | `QLabel* roomPeerConnectivityLabel_ = nullptr;` | 保存路径、地址或显示名称：room peer connectivity label。 |
| [L270](../src/apps/controller/ControllerMainWindow.h#L270) | `roomSeatUsageLabel_` | `QLabel* roomSeatUsageLabel_ = nullptr;` | 保存路径、地址或显示名称：room seat usage label。 |
| [L271](../src/apps/controller/ControllerMainWindow.h#L271) | `roomMemberSummaryLabel_` | `QLabel* roomMemberSummaryLabel_ = nullptr;` | 保存路径、地址或显示名称：room member summary label。 |
| [L272](../src/apps/controller/ControllerMainWindow.h#L272) | `roomMemberFooterLabel_` | `QLabel* roomMemberFooterLabel_ = nullptr;` | 保存路径、地址或显示名称：room member footer label。 |
| [L273](../src/apps/controller/ControllerMainWindow.h#L273) | `roomMemberList_` | `QListWidget* roomMemberList_ = nullptr;` | 保存 room member list 相关配置或运行状态。 |
| [L274](../src/apps/controller/ControllerMainWindow.h#L274) | `renderedRoomMemberKey_` | `QString renderedRoomMemberKey_;` | 保存 rendered room member key 相关配置或运行状态。 |
| [L275](../src/apps/controller/ControllerMainWindow.h#L275) | `activeRoomCapacity_` | `QComboBox* activeRoomCapacity_ = nullptr;` | 保存 active room capacity 相关配置或运行状态。 |
| [L276](../src/apps/controller/ControllerMainWindow.h#L276) | `applyRoomCapacityButton_` | `QPushButton* applyRoomCapacityButton_ = nullptr;` | 保存 apply room capacity button 相关配置或运行状态。 |
| [L277](../src/apps/controller/ControllerMainWindow.h#L277) | `roomScreenShareButton_` | `QPushButton* roomScreenShareButton_ = nullptr;` | 保存 room screen share button 相关配置或运行状态。 |
| [L278](../src/apps/controller/ControllerMainWindow.h#L278) | `roomCameraButton_` | `QPushButton* roomCameraButton_ = nullptr;` | 保存 room camera button 相关配置或运行状态。 |
| [L279](../src/apps/controller/ControllerMainWindow.h#L279) | `roomMicrophoneButton_` | `QPushButton* roomMicrophoneButton_ = nullptr;` | 保存 room microphone button 相关配置或运行状态。 |
| [L280](../src/apps/controller/ControllerMainWindow.h#L280) | `roomSpeakerButton_` | `QPushButton* roomSpeakerButton_ = nullptr;` | 保存 room speaker button 相关配置或运行状态。 |
| [L281](../src/apps/controller/ControllerMainWindow.h#L281) | `roomCameraGalleryButton_` | `QPushButton* roomCameraGalleryButton_ = nullptr;` | 保存 room camera gallery button 相关配置或运行状态。 |
| [L282](../src/apps/controller/ControllerMainWindow.h#L282) | `roomFileTransferButton_` | `QPushButton* roomFileTransferButton_ = nullptr;` | 保存 room file transfer button 相关配置或运行状态。 |
| [L283](../src/apps/controller/ControllerMainWindow.h#L283) | `fileTransferNavButton_` | `QPushButton* fileTransferNavButton_ = nullptr;` | 保存 file transfer nav button 相关配置或运行状态。 |
| [L284](../src/apps/controller/ControllerMainWindow.h#L284) | `leaveRoomButton_` | `QPushButton* leaveRoomButton_ = nullptr;` | 保存 leave room button 相关配置或运行状态。 |
| [L285](../src/apps/controller/ControllerMainWindow.h#L285) | `roomStageLabel_` | `QLabel* roomStageLabel_ = nullptr;` | 保存路径、地址或显示名称：room stage label。 |
| [L286](../src/apps/controller/ControllerMainWindow.h#L286) | `titleBar_` | `CustomTitleBar* titleBar_ = nullptr;` | 保存 title bar 相关配置或运行状态。 |
| [L287](../src/apps/controller/ControllerMainWindow.h#L287) | `pageStack_` | `QStackedWidget* pageStack_ = nullptr;` | 保存 page stack 相关配置或运行状态。 |
| [L288](../src/apps/controller/ControllerMainWindow.h#L288) | `roomNavButton_` | `QPushButton* roomNavButton_ = nullptr;` | 保存 room nav button 相关配置或运行状态。 |
| [L289](../src/apps/controller/ControllerMainWindow.h#L289) | `deviceNavButton_` | `QPushButton* deviceNavButton_ = nullptr;` | 保存 device nav button 相关配置或运行状态。 |
| [L290](../src/apps/controller/ControllerMainWindow.h#L290) | `myDevicesNavButton_` | `QPushButton* myDevicesNavButton_ = nullptr;` | 保存 my devices nav button 相关配置或运行状态。 |
| [L291](../src/apps/controller/ControllerMainWindow.h#L291) | `recentNavButton_` | `QPushButton* recentNavButton_ = nullptr;` | 保存 recent nav button 相关配置或运行状态。 |
| [L292](../src/apps/controller/ControllerMainWindow.h#L292) | `debugNavButton_` | `QPushButton* debugNavButton_ = nullptr;` | 保存 debug nav button 相关配置或运行状态。 |
| [L293](../src/apps/controller/ControllerMainWindow.h#L293) | `settingsNavButton_` | `QPushButton* settingsNavButton_ = nullptr;` | 保存 settings nav button 相关配置或运行状态。 |
| [L294](../src/apps/controller/ControllerMainWindow.h#L294) | `helpNavButton_` | `QPushButton* helpNavButton_ = nullptr;` | 保存 help nav button 相关配置或运行状态。 |
| [L295](../src/apps/controller/ControllerMainWindow.h#L295) | `authorNavButton_` | `QPushButton* authorNavButton_ = nullptr;` | 保存 author nav button 相关配置或运行状态。 |
| [L296](../src/apps/controller/ControllerMainWindow.h#L296) | `pageNavigationButtons_` | `QVector<QPushButton*> pageNavigationButtons_;` | 保存 page navigation buttons 相关配置或运行状态。 |
| [L297](../src/apps/controller/ControllerMainWindow.h#L297) | `navigationIndicator_` | `QFrame* navigationIndicator_ = nullptr;` | 保存 navigation indicator 相关配置或运行状态。 |
| [L298](../src/apps/controller/ControllerMainWindow.h#L298) | `navigationAnimation_` | `QPropertyAnimation* navigationAnimation_ = nullptr;` | 保存 navigation animation 相关配置或运行状态。 |
| [L299](../src/apps/controller/ControllerMainWindow.h#L299) | `trayIcon_` | `QSystemTrayIcon* trayIcon_ = nullptr;` | 保存 tray icon 相关配置或运行状态。 |
| [L300](../src/apps/controller/ControllerMainWindow.h#L300) | `trayIdentityLabel_` | `QLabel* trayIdentityLabel_ = nullptr;` | 保存路径、地址或显示名称：tray identity label。 |
| [L301](../src/apps/controller/ControllerMainWindow.h#L301) | `traySignOutAction_` | `QAction* traySignOutAction_ = nullptr;` | 保存 tray sign out action 相关配置或运行状态。 |
| [L306](../src/apps/controller/ControllerMainWindow.h#L306) | `sessionEngineStarted_` | `bool sessionEngineStarted_ = false;` | 保存 session engine started 相关配置或运行状态。 |
| [L307](../src/apps/controller/ControllerMainWindow.h#L307) | `applicationExitPrepared_` | `bool applicationExitPrepared_ = false;` | 保存 application exit prepared 相关配置或运行状态。 |
| [L308](../src/apps/controller/ControllerMainWindow.h#L308) | `authenticationAvailable_` | `bool authenticationAvailable_ = true;` | 保存能力或开关状态：authentication available。 |
| [L309](../src/apps/controller/ControllerMainWindow.h#L309) | `animationLevelSelector_` | `QComboBox* animationLevelSelector_ = nullptr;` | 保存 animation level selector 相关配置或运行状态。 |
| [L310](../src/apps/controller/ControllerMainWindow.h#L310) | `themeModeSelector_` | `QComboBox* themeModeSelector_ = nullptr;` | 保存 theme mode selector 相关配置或运行状态。 |
| [L311](../src/apps/controller/ControllerMainWindow.h#L311) | `fontFamilySelector_` | `QComboBox* fontFamilySelector_ = nullptr;` | 保存 font family selector 相关配置或运行状态。 |
| [L312](../src/apps/controller/ControllerMainWindow.h#L312) | `fontSizeSelector_` | `QComboBox* fontSizeSelector_ = nullptr;` | 保存 font size selector 相关配置或运行状态。 |
| [L313](../src/apps/controller/ControllerMainWindow.h#L313) | `autoStartSelector_` | `QComboBox* autoStartSelector_ = nullptr;` | 保存 auto start selector 相关配置或运行状态。 |
| [L314](../src/apps/controller/ControllerMainWindow.h#L314) | `startupVisibilitySelector_` | `QComboBox* startupVisibilitySelector_ = nullptr;` | 保存 startup visibility selector 相关配置或运行状态。 |
| [L315](../src/apps/controller/ControllerMainWindow.h#L315) | `closeButtonBehaviorSelector_` | `QComboBox* closeButtonBehaviorSelector_ = nullptr;` | 保存 close button behavior selector 相关配置或运行状态。 |
| [L316](../src/apps/controller/ControllerMainWindow.h#L316) | `defaultRoomCapacitySelector_` | `QComboBox* defaultRoomCapacitySelector_ = nullptr;` | 保存 default room capacity selector 相关配置或运行状态。 |
| [L317](../src/apps/controller/ControllerMainWindow.h#L317) | `desktopCaptureSelector_` | `QComboBox* desktopCaptureSelector_ = nullptr;` | 保存 desktop capture selector 相关配置或运行状态。 |
| [L318](../src/apps/controller/ControllerMainWindow.h#L318) | `videoEncoderSelector_` | `QComboBox* videoEncoderSelector_ = nullptr;` | 保存 video encoder selector 相关配置或运行状态。 |
| [L319](../src/apps/controller/ControllerMainWindow.h#L319) | `ffmpegHardwareBackendSelector_` | `QComboBox* ffmpegHardwareBackendSelector_ = nullptr;` | 保存 ffmpeg hardware backend selector 相关配置或运行状态。 |
| [L320](../src/apps/controller/ControllerMainWindow.h#L320) | `ffmpegX264PresetSelector_` | `QComboBox* ffmpegX264PresetSelector_ = nullptr;` | 保存 ffmpeg x264 preset selector 相关配置或运行状态。 |
| [L321](../src/apps/controller/ControllerMainWindow.h#L321) | `videoDecoderSelector_` | `QComboBox* videoDecoderSelector_ = nullptr;` | 保存 video decoder selector 相关配置或运行状态。 |
| [L322](../src/apps/controller/ControllerMainWindow.h#L322) | `videoRendererSelector_` | `QComboBox* videoRendererSelector_ = nullptr;` | 保存 video renderer selector 相关配置或运行状态。 |
| [L323](../src/apps/controller/ControllerMainWindow.h#L323) | `videoPipelineSettingsBusy_` | `bool videoPipelineSettingsBusy_ = false;` | 保存 video pipeline settings busy 相关配置或运行状态。 |
| [L324](../src/apps/controller/ControllerMainWindow.h#L324) | `videoPipelineSettingsApplyPending_` | `bool videoPipelineSettingsApplyPending_ = false;` | 保存待处理队列或请求：video pipeline settings apply pending。 |
| [L325](../src/apps/controller/ControllerMainWindow.h#L325) | `dragPointerSampleRateSelector_` | `QComboBox* dragPointerSampleRateSelector_ = nullptr;` | 保存 drag pointer sample rate selector 相关配置或运行状态。 |
| [L326](../src/apps/controller/ControllerMainWindow.h#L326) | `remotePasteEnabledSelector_` | `QComboBox* remotePasteEnabledSelector_ = nullptr;` | 保存 remote paste enabled selector 相关配置或运行状态。 |
| [L327](../src/apps/controller/ControllerMainWindow.h#L327) | `clipboardFormatsSelector_` | `QComboBox* clipboardFormatsSelector_ = nullptr;` | 保存 clipboard formats selector 相关配置或运行状态。 |
| [L328](../src/apps/controller/ControllerMainWindow.h#L328) | `clipboardLargeFileLimitSelector_` | `QComboBox* clipboardLargeFileLimitSelector_ = nullptr;` | 保存 clipboard large file limit selector 相关配置或运行状态。 |
| [L329](../src/apps/controller/ControllerMainWindow.h#L329) | `clipboardCacheRetentionSelector_` | `QComboBox* clipboardCacheRetentionSelector_ = nullptr;` | 保存 clipboard cache retention selector 相关配置或运行状态。 |
| [L330](../src/apps/controller/ControllerMainWindow.h#L330) | `clipboardCacheCapacitySelector_` | `QComboBox* clipboardCacheCapacitySelector_ = nullptr;` | 保存 clipboard cache capacity selector 相关配置或运行状态。 |
| [L331](../src/apps/controller/ControllerMainWindow.h#L331) | `clipboardCachePathLabel_` | `QLabel* clipboardCachePathLabel_ = nullptr;` | 保存路径、地址或显示名称：clipboard cache path label。 |
| [L332](../src/apps/controller/ControllerMainWindow.h#L332) | `clipboardCacheUsageLabel_` | `QLabel* clipboardCacheUsageLabel_ = nullptr;` | 保存路径、地址或显示名称：clipboard cache usage label。 |
| [L333](../src/apps/controller/ControllerMainWindow.h#L333) | `clearClipboardCacheButton_` | `QPushButton* clearClipboardCacheButton_ = nullptr;` | 保存 clear clipboard cache button 相关配置或运行状态。 |
| [L334](../src/apps/controller/ControllerMainWindow.h#L334) | `decoderBenchmarkSummary_` | `QLabel* decoderBenchmarkSummary_ = nullptr;` | 保存 decoder benchmark summary 相关配置或运行状态。 |
| [L335](../src/apps/controller/ControllerMainWindow.h#L335) | `decoderBenchmarkButton_` | `QPushButton* decoderBenchmarkButton_ = nullptr;` | 保存 decoder benchmark button 相关配置或运行状态。 |
| [L336](../src/apps/controller/ControllerMainWindow.h#L336) | `decoderBenchmarkProcess_` | `QProcess* decoderBenchmarkProcess_ = nullptr;` | 保存 decoder benchmark process 相关配置或运行状态。 |
| [L337](../src/apps/controller/ControllerMainWindow.h#L337) | `decoderBenchmarkManualRequest_` | `bool decoderBenchmarkManualRequest_ = false;` | 保存 decoder benchmark manual request 相关配置或运行状态。 |
| [L338](../src/apps/controller/ControllerMainWindow.h#L338) | `decoderBenchmarkHardwareFingerprint_` | `QString decoderBenchmarkHardwareFingerprint_;` | 保存 decoder benchmark hardware fingerprint 相关配置或运行状态。 |
| [L339](../src/apps/controller/ControllerMainWindow.h#L339) | `encoderBenchmarkSummary_` | `QLabel* encoderBenchmarkSummary_ = nullptr;` | 保存 encoder benchmark summary 相关配置或运行状态。 |
| [L340](../src/apps/controller/ControllerMainWindow.h#L340) | `encoderBenchmarkButton_` | `QPushButton* encoderBenchmarkButton_ = nullptr;` | 保存 encoder benchmark button 相关配置或运行状态。 |
| [L341](../src/apps/controller/ControllerMainWindow.h#L341) | `encoderBenchmarkProcess_` | `QProcess* encoderBenchmarkProcess_ = nullptr;` | 保存 encoder benchmark process 相关配置或运行状态。 |
| [L342](../src/apps/controller/ControllerMainWindow.h#L342) | `encoderBenchmarkManualRequest_` | `bool encoderBenchmarkManualRequest_ = false;` | 保存 encoder benchmark manual request 相关配置或运行状态。 |
| [L343](../src/apps/controller/ControllerMainWindow.h#L343) | `encoderBenchmarkHardwareFingerprint_` | `QString encoderBenchmarkHardwareFingerprint_;` | 保存 encoder benchmark hardware fingerprint 相关配置或运行状态。 |
| [L344](../src/apps/controller/ControllerMainWindow.h#L344) | `encoderBenchmarkCaptureBackend_` | `QString encoderBenchmarkCaptureBackend_;` | 保存 encoder benchmark capture backend 相关配置或运行状态。 |
| [L345](../src/apps/controller/ControllerMainWindow.h#L345) | `encoderBenchmarkX264Preset_` | `QString encoderBenchmarkX264Preset_;` | 保存 encoder benchmark x264 preset 相关配置或运行状态。 |
| [L346](../src/apps/controller/ControllerMainWindow.h#L346) | `cameraGalleryBehaviorSelector_` | `QComboBox* cameraGalleryBehaviorSelector_ = nullptr;` | 保存 camera gallery behavior selector 相关配置或运行状态。 |
| [L347](../src/apps/controller/ControllerMainWindow.h#L347) | `cameraDeviceSelector_` | `QComboBox* cameraDeviceSelector_ = nullptr;` | 保存 camera device selector 相关配置或运行状态。 |
| [L348](../src/apps/controller/ControllerMainWindow.h#L348) | `microphoneDeviceSelector_` | `QComboBox* microphoneDeviceSelector_ = nullptr;` | 保存 microphone device selector 相关配置或运行状态。 |
| [L349](../src/apps/controller/ControllerMainWindow.h#L349) | `speakerDeviceSelector_` | `QComboBox* speakerDeviceSelector_ = nullptr;` | 保存 speaker device selector 相关配置或运行状态。 |
| [L350](../src/apps/controller/ControllerMainWindow.h#L350) | `mediaDeviceStatusLabel_` | `QLabel* mediaDeviceStatusLabel_ = nullptr;` | 保存路径、地址或显示名称：media device status label。 |
| [L351](../src/apps/controller/ControllerMainWindow.h#L351) | `refreshMediaDevicesButton_` | `QPushButton* refreshMediaDevicesButton_ = nullptr;` | 保存 refresh media devices button 相关配置或运行状态。 |
| [L352](../src/apps/controller/ControllerMainWindow.h#L352) | `mediaDeviceRefreshDebounceTimer_` | `QTimer* mediaDeviceRefreshDebounceTimer_ = nullptr;` | 保存定时、截止或超时状态：media device refresh debounce timer。 |
| [L353](../src/apps/controller/ControllerMainWindow.h#L353) | `devicePageSignalStatus_` | `QLabel* devicePageSignalStatus_ = nullptr;` | 保存状态机当前状态：device page signal status。 |
| [L354](../src/apps/controller/ControllerMainWindow.h#L354) | `recentRoomsLayout_` | `QVBoxLayout* recentRoomsLayout_ = nullptr;` | 保存 recent rooms layout 相关配置或运行状态。 |
| [L355](../src/apps/controller/ControllerMainWindow.h#L355) | `recentDevicesLayout_` | `QVBoxLayout* recentDevicesLayout_ = nullptr;` | 保存 recent devices layout 相关配置或运行状态。 |
| [L356](../src/apps/controller/ControllerMainWindow.h#L356) | `ownedDevicesCardsLayout_` | `QVBoxLayout* ownedDevicesCardsLayout_ = nullptr;` | 保存 owned devices cards layout 相关配置或运行状态。 |
| [L357](../src/apps/controller/ControllerMainWindow.h#L357) | `ownedDevicesSummaryLabel_` | `QLabel* ownedDevicesSummaryLabel_ = nullptr;` | 保存路径、地址或显示名称：owned devices summary label。 |
| [L358](../src/apps/controller/ControllerMainWindow.h#L358) | `ownedDevicesEmptyState_` | `QFrame* ownedDevicesEmptyState_ = nullptr;` | 保存状态机当前状态：owned devices empty state。 |
| [L359](../src/apps/controller/ControllerMainWindow.h#L359) | `ownedDevicesEmptyArtwork_` | `QLabel* ownedDevicesEmptyArtwork_ = nullptr;` | 保存 owned devices empty artwork 相关配置或运行状态。 |
| [L360](../src/apps/controller/ControllerMainWindow.h#L360) | `ownedDevicesRefreshButton_` | `QPushButton* ownedDevicesRefreshButton_ = nullptr;` | 保存 owned devices refresh button 相关配置或运行状态。 |
| [L361](../src/apps/controller/ControllerMainWindow.h#L361) | `localDevicePage_` | `QWidget* localDevicePage_ = nullptr;` | 保存 local device page 相关配置或运行状态。 |
| [L362](../src/apps/controller/ControllerMainWindow.h#L362) | `ownedDevicesPage_` | `QWidget* ownedDevicesPage_ = nullptr;` | 保存 owned devices page 相关配置或运行状态。 |
| [L363](../src/apps/controller/ControllerMainWindow.h#L363) | `recentConnectionsPage_` | `QWidget* recentConnectionsPage_ = nullptr;` | 保存 recent connections page 相关配置或运行状态。 |
| [L364](../src/apps/controller/ControllerMainWindow.h#L364) | `debugPage_` | `QWidget* debugPage_ = nullptr;` | 保存 debug page 相关配置或运行状态。 |
| [L365](../src/apps/controller/ControllerMainWindow.h#L365) | `darkInterfaceTheme_` | `bool darkInterfaceTheme_ = false;` | 保存 dark interface theme 相关配置或运行状态。 |
| [L366](../src/apps/controller/ControllerMainWindow.h#L366) | `renderedOwnedDevicesRevision_` | `quint64 renderedOwnedDevicesRevision_ = 0;` | 标记当前世代，用于拒绝过期异步结果：rendered owned devices revision。 |
| [L368](../src/apps/controller/ControllerMainWindow.h#L368) | `kNotConfigured` | `SessionConnectivityState::kNotConfigured;` | 定义 not configured 的编译期常量或产品边界。 |
| [L369](../src/apps/controller/ControllerMainWindow.h#L369) | `recentEmptyState_` | `QFrame* recentEmptyState_ = nullptr;` | 保存状态机当前状态：recent empty state。 |
| [L370](../src/apps/controller/ControllerMainWindow.h#L370) | `recentDevicesEmptyState_` | `QFrame* recentDevicesEmptyState_ = nullptr;` | 保存状态机当前状态：recent devices empty state。 |
| [L371](../src/apps/controller/ControllerMainWindow.h#L371) | `debugValueLabels_` | `QHash<QString, QLabel*> debugValueLabels_;` | 保存 debug value labels 相关配置或运行状态。 |
| [L372](../src/apps/controller/ControllerMainWindow.h#L372) | `statsCardsWidget_` | `QWidget* statsCardsWidget_ = nullptr;` | 保存 stats cards widget 相关配置或运行状态。 |
| [L373](../src/apps/controller/ControllerMainWindow.h#L373) | `screenFrameRateLogButton_` | `QPushButton* screenFrameRateLogButton_ = nullptr;` | 保存 screen frame rate log button 相关配置或运行状态。 |
| [L374](../src/apps/controller/ControllerMainWindow.h#L374) | `screenFrameRateLogEnabled_` | `bool screenFrameRateLogEnabled_ = false;` | 保存能力或开关状态：screen frame rate log enabled。 |
| [L375](../src/apps/controller/ControllerMainWindow.h#L375) | `inputEventStatsButton_` | `QPushButton* inputEventStatsButton_ = nullptr;` | 保存 input event stats button 相关配置或运行状态。 |
| [L376](../src/apps/controller/ControllerMainWindow.h#L376) | `inputEventStatsEnabled_` | `bool inputEventStatsEnabled_ = false;` | 保存能力或开关状态：input event stats enabled。 |
| [L377](../src/apps/controller/ControllerMainWindow.h#L377) | `copyDebugButton_` | `QPushButton* copyDebugButton_ = nullptr;` | 保存 copy debug button 相关配置或运行状态。 |
| [L378](../src/apps/controller/ControllerMainWindow.h#L378) | `copyMediaDebugButton_` | `QPushButton* copyMediaDebugButton_ = nullptr;` | 保存 copy media debug button 相关配置或运行状态。 |
| [L379](../src/apps/controller/ControllerMainWindow.h#L379) | `debugCopyText_` | `QString debugCopyText_;` | 保存 debug copy text 相关配置或运行状态。 |
| [L380](../src/apps/controller/ControllerMainWindow.h#L380) | `mediaDebugCopyText_` | `QString mediaDebugCopyText_;` | 保存 media debug copy text 相关配置或运行状态。 |
| [L381](../src/apps/controller/ControllerMainWindow.h#L381) | `statsDebugCopyText_` | `QString statsDebugCopyText_;` | 保存 stats debug copy text 相关配置或运行状态。 |
| [L382](../src/apps/controller/ControllerMainWindow.h#L382) | `diagnosticsCopyTextRequested_` | `bool diagnosticsCopyTextRequested_ = false;` | 保存 diagnostics copy text requested 相关配置或运行状态。 |
| [L383](../src/apps/controller/ControllerMainWindow.h#L383) | `diagnosticsRefreshTimer_` | `QTimer* diagnosticsRefreshTimer_ = nullptr;` | 保存定时、截止或超时状态：diagnostics refresh timer。 |
| [L384](../src/apps/controller/ControllerMainWindow.h#L384) | `lastRememberedRoomId_` | `QString lastRememberedRoomId_;` | 保存身份或作用域标识：last remembered room id。 |
| [L385](../src/apps/controller/ControllerMainWindow.h#L385) | `lastRememberedDirectSessionId_` | `QString lastRememberedDirectSessionId_;` | 保存身份或作用域标识：last remembered direct session id。 |
| [L386](../src/apps/controller/ControllerMainWindow.h#L386) | `recentHistoryAccountKey_` | `QString recentHistoryAccountKey_;` | 保存 recent history account key 相关配置或运行状态。 |
| [L387](../src/apps/controller/ControllerMainWindow.h#L387) | `recentRoomAvailabilityRequested_` | `bool recentRoomAvailabilityRequested_ = false;` | 保存 recent room availability requested 相关配置或运行状态。 |
| [L388](../src/apps/controller/ControllerMainWindow.h#L388) | `quitting_` | `bool quitting_ = false;` | 保存 quitting 相关配置或运行状态。 |
| [L389](../src/apps/controller/ControllerMainWindow.h#L389) | `pendingDeviceName_` | `QString pendingDeviceName_;` | 保存路径、地址或显示名称：pending device name。 |
| [L390](../src/apps/controller/ControllerMainWindow.h#L390) | `promptedSessionId_` | `QString promptedSessionId_;` | 保存身份或作用域标识：prompted session id。 |
| [L391](../src/apps/controller/ControllerMainWindow.h#L391) | `approvalRoomId_` | `QString approvalRoomId_;` | 保存身份或作用域标识：approval room id。 |
| [L392](../src/apps/controller/ControllerMainWindow.h#L392) | `promptedRoomJoinRequestIds_` | `QSet<QString> promptedRoomJoinRequestIds_;` | 保存 prompted room join request ids 相关配置或运行状态。 |
| [L393](../src/apps/controller/ControllerMainWindow.h#L393) | `promptedRoomScreenShareSwitchRequestIds_` | `QSet<QString> promptedRoomScreenShareSwitchRequestIds_;` | 保存 prompted room screen share switch request ids 相关配置或运行状态。 |
| [L394](../src/apps/controller/ControllerMainWindow.h#L394) | `promptedRoomControlRequestIds_` | `QSet<QString> promptedRoomControlRequestIds_;` | 保存 prompted room control request ids 相关配置或运行状态。 |
| [L395](../src/apps/controller/ControllerMainWindow.h#L395) | `promptedRoomScreenShareViewRequestIds_` | `QSet<QString> promptedRoomScreenShareViewRequestIds_;` | 保存 prompted room screen share view request ids 相关配置或运行状态。 |
| [L396](../src/apps/controller/ControllerMainWindow.h#L396) | `handledRoomMemberActionResultKeys_` | `QSet<QString> handledRoomMemberActionResultKeys_;` | 保存 handled room member action result keys 相关配置或运行状态。 |
| [L397](../src/apps/controller/ControllerMainWindow.h#L397) | `roomJoinApprovalPromptPending_` | `bool roomJoinApprovalPromptPending_ = false;` | 保存待处理队列或请求：room join approval prompt pending。 |
| [L398](../src/apps/controller/ControllerMainWindow.h#L398) | `roomScreenShareSwitchApprovalPromptPending_` | `bool roomScreenShareSwitchApprovalPromptPending_ = false;` | 保存待处理队列或请求：room screen share switch approval prompt pending。 |
| [L399](../src/apps/controller/ControllerMainWindow.h#L399) | `roomControlApprovalPromptPending_` | `bool roomControlApprovalPromptPending_ = false;` | 保存待处理队列或请求：room control approval prompt pending。 |
| [L400](../src/apps/controller/ControllerMainWindow.h#L400) | `roomScreenShareViewApprovalPromptPending_` | `bool roomScreenShareViewApprovalPromptPending_ = false;` | 保存待处理队列或请求：room screen share view approval prompt pending。 |
| [L401](../src/apps/controller/ControllerMainWindow.h#L401) | `lastRoomDecisionAlertKey_` | `QString lastRoomDecisionAlertKey_;` | 保存 last room decision alert key 相关配置或运行状态。 |
| [L402](../src/apps/controller/ControllerMainWindow.h#L402) | `cameraWindow_` | `QPointer<CameraWindow> cameraWindow_;` | 保存 camera window 相关配置或运行状态。 |
| [L403](../src/apps/controller/ControllerMainWindow.h#L403) | `fileTransferWindow_` | `QPointer<FileTransferWindow> fileTransferWindow_;` | 保存 file transfer window 相关配置或运行状态。 |
| [L404](../src/apps/controller/ControllerMainWindow.h#L404) | `remoteSessionWindow_` | `QPointer<RemoteSessionWindow> remoteSessionWindow_;` | 保存 remote session window 相关配置或运行状态。 |
| [L405](../src/apps/controller/ControllerMainWindow.h#L405) | `roomCameraWindow_` | `QPointer<RoomCameraWindow> roomCameraWindow_;` | 保存 room camera window 相关配置或运行状态。 |
| [L406](../src/apps/controller/ControllerMainWindow.h#L406) | `remoteSessionBinding_` | `std::optional<RemoteSessionBinding> remoteSessionBinding_;` | 保存 remote session binding 相关配置或运行状态。 |
| [L407](../src/apps/controller/ControllerMainWindow.h#L407) | `preflightScreenPreferenceEpoch_` | `quint64 preflightScreenPreferenceEpoch_ = 0;` | 标记当前世代，用于拒绝过期异步结果：preflight screen preference epoch。 |
| [L408](../src/apps/controller/ControllerMainWindow.h#L408) | `preflightScreenPreferenceAttemptEpoch_` | `quint64 preflightScreenPreferenceAttemptEpoch_ = 0;` | 标记当前世代，用于拒绝过期异步结果：preflight screen preference attempt epoch。 |
| [L409](../src/apps/controller/ControllerMainWindow.h#L409) | `preflightScreenPreferenceAttempts_` | `int preflightScreenPreferenceAttempts_ = 0;` | 保存 preflight screen preference attempts 相关配置或运行状态。 |
| [L410](../src/apps/controller/ControllerMainWindow.h#L410) | `preflightScreenPreferenceScheduled_` | `bool preflightScreenPreferenceScheduled_ = false;` | 保存 preflight screen preference scheduled 相关配置或运行状态。 |
| [L411](../src/apps/controller/ControllerMainWindow.h#L411) | `dismissedRemoteScreenSharerDeviceId_` | `QString dismissedRemoteScreenSharerDeviceId_;` | 保存身份或作用域标识：dismissed remote screen sharer device id。 |
| [L412](../src/apps/controller/ControllerMainWindow.h#L412) | `dismissedRemoteScreenShareEpoch_` | `quint64 dismissedRemoteScreenShareEpoch_ = 0;` | 标记当前世代，用于拒绝过期异步结果：dismissed remote screen share epoch。 |
| [L413](../src/apps/controller/ControllerMainWindow.h#L413) | `dismissedDirectSessionId_` | `QString dismissedDirectSessionId_;` | 保存身份或作用域标识：dismissed direct session id。 |
| [L414](../src/apps/controller/ControllerMainWindow.h#L414) | `localCameraStopRequested_` | `bool localCameraStopRequested_ = false;` | 保存 local camera stop requested 相关配置或运行状态。 |
| [L415](../src/apps/controller/ControllerMainWindow.h#L415) | `cameraGalleryManuallyHidden_` | `bool cameraGalleryManuallyHidden_ = false;` | 保存 camera gallery manually hidden 相关配置或运行状态。 |
| [L416](../src/apps/controller/ControllerMainWindow.h#L416) | `mediaDeviceRevision_` | `quint64 mediaDeviceRevision_ = 0;` | 标记当前世代，用于拒绝过期异步结果：media device revision。 |
| [L417](../src/apps/controller/ControllerMainWindow.h#L417) | `mediaDeviceSnapshotSeen_` | `bool mediaDeviceSnapshotSeen_ = false;` | 保存 media device snapshot seen 相关配置或运行状态。 |
| [L418](../src/apps/controller/ControllerMainWindow.h#L418) | `mediaActivitySnapshotSeen_` | `bool mediaActivitySnapshotSeen_ = false;` | 保存 media activity snapshot seen 相关配置或运行状态。 |
| [L420](../src/apps/controller/ControllerMainWindow.h#L420) | `kOff` | `LocalMicrophoneState::kOff;` | 定义 off 的编译期常量或产品边界。 |
| [L421](../src/apps/controller/ControllerMainWindow.h#L421) | `displayedRoomAudioPlaybackMuted_` | `bool displayedRoomAudioPlaybackMuted_ = false;` | 保存 displayed room audio playback muted 相关配置或运行状态。 |
| [L422](../src/apps/controller/ControllerMainWindow.h#L422) | `mediaDeviceRefreshUserRequested_` | `bool mediaDeviceRefreshUserRequested_ = false;` | 保存 media device refresh user requested 相关配置或运行状态。 |
| [L423](../src/apps/controller/ControllerMainWindow.h#L423) | `pendingCameraDeviceId_` | `QString pendingCameraDeviceId_;` | 保存身份或作用域标识：pending camera device id。 |
| [L424](../src/apps/controller/ControllerMainWindow.h#L424) | `pendingMicrophoneDeviceId_` | `QString pendingMicrophoneDeviceId_;` | 保存身份或作用域标识：pending microphone device id。 |
| [L425](../src/apps/controller/ControllerMainWindow.h#L425) | `pendingSpeakerDeviceId_` | `QString pendingSpeakerDeviceId_;` | 保存身份或作用域标识：pending speaker device id。 |
| [L426](../src/apps/controller/ControllerMainWindow.h#L426) | `lastMediaDeviceErrorKey_` | `QString lastMediaDeviceErrorKey_;` | 保存 last media device error key 相关配置或运行状态。 |
| [L427](../src/apps/controller/ControllerMainWindow.h#L427) | `displayedClipboardSentItems_` | `std::uint64_t displayedClipboardSentItems_ = 0;` | 保存 displayed clipboard sent items 相关配置或运行状态。 |
| [L428](../src/apps/controller/ControllerMainWindow.h#L428) | `displayedClipboardReceivedItems_` | `std::uint64_t displayedClipboardReceivedItems_ = 0;` | 保存 displayed clipboard received items 相关配置或运行状态。 |
| [L429](../src/apps/controller/ControllerMainWindow.h#L429) | `displayedClipboardRejectedItems_` | `std::uint64_t displayedClipboardRejectedItems_ = 0;` | 保存 displayed clipboard rejected items 相关配置或运行状态。 |
| [L430](../src/apps/controller/ControllerMainWindow.h#L430) | `displayedClipboardErrorCode_` | `QString displayedClipboardErrorCode_;` | 保存 displayed clipboard error code 相关配置或运行状态。 |
| [L431](../src/apps/controller/ControllerMainWindow.h#L431) | `pendingRemotePasteDialogId_` | `QString pendingRemotePasteDialogId_;` | 保存身份或作用域标识：pending remote paste dialog id。 |
| [L432](../src/apps/controller/ControllerMainWindow.h#L432) | `visibleRemotePasteDialogId_` | `QString visibleRemotePasteDialogId_;` | 保存身份或作用域标识：visible remote paste dialog id。 |
| [L433](../src/apps/controller/ControllerMainWindow.h#L433) | `promptedClipboardConflictId_` | `QString promptedClipboardConflictId_;` | 保存身份或作用域标识：prompted clipboard conflict id。 |
| [L434](../src/apps/controller/ControllerMainWindow.h#L434) | `clipboardUiUpdatePending_` | `std::atomic_bool clipboardUiUpdatePending_{false};` | 保存待处理队列或请求：clipboard ui update pending。 |
| [L435](../src/apps/controller/ControllerMainWindow.h#L435) | `clipboardAllowedForCurrentControl_` | `bool clipboardAllowedForCurrentControl_ = false;` | 保存 clipboard allowed for current control 相关配置或运行状态。 |

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L64](../src/apps/controller/ControllerMainWindow.h#L64) | `ControllerMainWindow` | 声明 | `explicit ControllerMainWindow(std::unique_ptr<ISessionEngine> engine, bool startEngineImmediately = true, QWidget* parent = nullptr)` | 实现 controller main window 对应的业务或工具逻辑。 |
| [L67](../src/apps/controller/ControllerMainWindow.h#L67) | `~ControllerMainWindow` | 声明 | `~ControllerMainWindow() override` | 停止相关活动并释放 ControllerMainWindow 实例拥有的资源。 |
| [L69](../src/apps/controller/ControllerMainWindow.h#L69) | `ActivateFromExternalLaunch` | 声明 | `void ActivateFromExternalLaunch()` | 实现 activate from external launch 对应的业务或工具逻辑。 |
| [L70](../src/apps/controller/ControllerMainWindow.h#L70) | `SetAccountInteractionCallback` | 声明 | `void SetAccountInteractionCallback( std::function<void()> callback)` | 更新或应用 set account interaction callback 相关逻辑。 |
| [L72](../src/apps/controller/ControllerMainWindow.h#L72) | `SetAccountSwitchCallback` | 声明 | `void SetAccountSwitchCallback(std::function<void()> callback)` | 更新或应用 set account switch callback 相关逻辑。 |
| [L73](../src/apps/controller/ControllerMainWindow.h#L73) | `SetAccountDeletionCallback` | 声明 | `void SetAccountDeletionCallback(std::function<void()> callback)` | 更新或应用 set account deletion callback 相关逻辑。 |
| [L76](../src/apps/controller/ControllerMainWindow.h#L76) | `SetAccountBusy` | 声明 | `void SetAccountBusy(const QString& message)` | 更新或应用 set account busy 相关逻辑。 |
| [L77](../src/apps/controller/ControllerMainWindow.h#L77) | `SetAccountSession` | 声明 | `void SetAccountSession( const QString& accountId, const QString& accountLabel, const QString& accountDetail, std::function<void()> signOutCallback)` | 更新或应用 set account session 相关逻辑。 |
| [L82](../src/apps/controller/ControllerMainWindow.h#L82) | `StartSessionEngine` | 声明 | `bool StartSessionEngine()` | 启动 start session engine 相关逻辑。 |
| [L83](../src/apps/controller/ControllerMainWindow.h#L83) | `StopSessionEngine` | 声明 | `void StopSessionEngine()` | 停止 stop session engine 相关逻辑。 |
| [L86](../src/apps/controller/ControllerMainWindow.h#L86) | `PrepareForApplicationExit` | 声明 | `void PrepareForApplicationExit()` | Executes the protocol-aware shutdown while Qt can still flush WSS messages. Safe to call repeatedly from every application-exit path. |
| [L87](../src/apps/controller/ControllerMainWindow.h#L87) | `RunThemeRoundTripSelfTest` | 声明 | `bool RunThemeRoundTripSelfTest(QString* errorMessage = nullptr)` | 执行后台循环或调度 run theme round trip self test 相关逻辑。 |
| [L89](../src/apps/controller/ControllerMainWindow.h#L89) | `ControllerMainWindow` | 声明 | `ControllerMainWindow(const ControllerMainWindow&) = delete` | 实现 controller main window 对应的业务或工具逻辑。 |
| [L93](../src/apps/controller/ControllerMainWindow.h#L93) | `OnSessionEngineSnapshot` | 声明 | `void OnSessionEngineSnapshot( const SessionEngineSnapshot& snapshot) override` | 接收并处理 on session engine snapshot 相关逻辑。 |
| [L95](../src/apps/controller/ControllerMainWindow.h#L95) | `OnClipboardStateChanged` | 声明 | `void OnClipboardStateChanged( const app::ClipboardControllerSnapshot& snapshot) override` | 接收并处理 on clipboard state changed 相关逻辑。 |
| [L97](../src/apps/controller/ControllerMainWindow.h#L97) | `BuildUi` | 声明 | `void BuildUi()` | 创建或初始化 build ui 相关逻辑。 |
| [L98](../src/apps/controller/ControllerMainWindow.h#L98) | `BuildSystemTray` | 声明 | `void BuildSystemTray()` | 创建或初始化 build system tray 相关逻辑。 |
| [L99](../src/apps/controller/ControllerMainWindow.h#L99) | `ShowFromSystemTray` | 声明 | `void ShowFromSystemTray()` | 实现 show from system tray 对应的业务或工具逻辑。 |
| [L100](../src/apps/controller/ControllerMainWindow.h#L100) | `QuitFromSystemTray` | 声明 | `void QuitFromSystemTray()` | 实现 quit from system tray 对应的业务或工具逻辑。 |
| [L101](../src/apps/controller/ControllerMainWindow.h#L101) | `DestroyAuxiliaryWindowsForExit` | 声明 | `void DestroyAuxiliaryWindowsForExit()` | 关闭并清理 destroy auxiliary windows for exit 相关逻辑。 |
| [L102](../src/apps/controller/ControllerMainWindow.h#L102) | `CheckForSoftwareUpdates` | 声明 | `void CheckForSoftwareUpdates(bool manualRequest)` | 校验 check for software updates 相关逻辑。 |
| [L103](../src/apps/controller/ControllerMainWindow.h#L103) | `HandleSoftwareUpdateState` | 声明 | `void HandleSoftwareUpdateState( const update::SoftwareUpdateController::Snapshot& snapshot)` | 接收并处理 handle software update state 相关逻辑。 |
| [L105](../src/apps/controller/ControllerMainWindow.h#L105) | `OpenSoftwareUpdate` | 声明 | `void OpenSoftwareUpdate()` | 启动 open software update 相关逻辑。 |
| [L106](../src/apps/controller/ControllerMainWindow.h#L106) | `QuitForSoftwareUpdate` | 声明 | `void QuitForSoftwareUpdate()` | 实现 quit for software update 对应的业务或工具逻辑。 |
| [L107](../src/apps/controller/ControllerMainWindow.h#L107) | `InitializeEngine` | 声明 | `bool InitializeEngine()` | 创建或初始化 initialize engine 相关逻辑。 |
| [L108](../src/apps/controller/ControllerMainWindow.h#L108) | `CompleteEngineInitialization` | 声明 | `void CompleteEngineInitialization( const SessionCommandResult& startResult)` | 实现 complete engine initialization 对应的业务或工具逻辑。 |
| [L110](../src/apps/controller/ControllerMainWindow.h#L110) | `ApplyAuthenticationAvailability` | 声明 | `void ApplyAuthenticationAvailability(bool authenticated)` | 更新或应用 apply authentication availability 相关逻辑。 |
| [L111](../src/apps/controller/ControllerMainWindow.h#L111) | `StartSession` | 声明 | `void StartSession(const QString& deviceId, const QString& deviceName, SessionPurpose purpose)` | 启动 start session 相关逻辑。 |
| [L114](../src/apps/controller/ControllerMainWindow.h#L114) | `StartOwnedDeviceSession` | 声明 | `void StartOwnedDeviceSession(const QString& deviceId, const QString& deviceName)` | 启动 start owned device session 相关逻辑。 |
| [L116](../src/apps/controller/ControllerMainWindow.h#L116) | `StartAssistedSession` | 声明 | `void StartAssistedSession(const QString& deviceId, const QString& verificationCode)` | 启动 start assisted session 相关逻辑。 |
| [L118](../src/apps/controller/ControllerMainWindow.h#L118) | `OpenCameraWindow` | 声明 | `void OpenCameraWindow(const QString& deviceId, const QString& deviceName)` | 启动 open camera window 相关逻辑。 |
| [L119](../src/apps/controller/ControllerMainWindow.h#L119) | `OpenRemoteSession` | 声明 | `void OpenRemoteSession(RemoteSessionBinding binding)` | 启动 open remote session 相关逻辑。 |
| [L120](../src/apps/controller/ControllerMainWindow.h#L120) | `RefreshOwnedDevicesUi` | 声明 | `void RefreshOwnedDevicesUi( const SessionEngineSnapshot& snapshot)` | 刷新 refresh owned devices ui 相关逻辑。 |
| [L122](../src/apps/controller/ControllerMainWindow.h#L122) | `UpdateRoomUi` | 声明 | `void UpdateRoomUi(const SessionEngineSnapshot& snapshot)` | 更新或应用 update room ui 相关逻辑。 |
| [L123](../src/apps/controller/ControllerMainWindow.h#L123) | `QueueRoomJoinApproval` | 声明 | `void QueueRoomJoinApproval(const SessionEngineSnapshot& snapshot)` | 实现 queue room join approval 对应的业务或工具逻辑。 |
| [L124](../src/apps/controller/ControllerMainWindow.h#L124) | `QueueRoomScreenShareSwitchApproval` | 声明 | `void QueueRoomScreenShareSwitchApproval( const SessionEngineSnapshot& snapshot)` | 实现 queue room screen share switch approval 对应的业务或工具逻辑。 |
| [L126](../src/apps/controller/ControllerMainWindow.h#L126) | `QueueRoomControlApproval` | 声明 | `void QueueRoomControlApproval(const SessionEngineSnapshot& snapshot)` | 实现 queue room control approval 对应的业务或工具逻辑。 |
| [L127](../src/apps/controller/ControllerMainWindow.h#L127) | `QueueRoomScreenShareViewApproval` | 声明 | `void QueueRoomScreenShareViewApproval( const SessionEngineSnapshot& snapshot)` | 实现 queue room screen share view approval 对应的业务或工具逻辑。 |
| [L129](../src/apps/controller/ControllerMainWindow.h#L129) | `HandleRoomMemberActionResults` | 声明 | `void HandleRoomMemberActionResults( const SessionEngineSnapshot& snapshot)` | 接收并处理 handle room member action results 相关逻辑。 |
| [L131](../src/apps/controller/ControllerMainWindow.h#L131) | `ShowRoomMemberContextMenu` | 声明 | `void ShowRoomMemberContextMenu(const QPoint& position)` | 实现 show room member context menu 对应的业务或工具逻辑。 |
| [L132](../src/apps/controller/ControllerMainWindow.h#L132) | `HandleRemoteSessionDisconnect` | 声明 | `void HandleRemoteSessionDisconnect()` | 接收并处理 handle remote session disconnect 相关逻辑。 |
| [L133](../src/apps/controller/ControllerMainWindow.h#L133) | `SetRoomActionHint` | 声明 | `void SetRoomActionHint(const QString& text, bool error = false)` | 更新或应用 set room action hint 相关逻辑。 |
| [L134](../src/apps/controller/ControllerMainWindow.h#L134) | `SetRuntimeStatus` | 声明 | `void SetRuntimeStatus(const QString& status, const QString& color)` | 更新或应用 set runtime status 相关逻辑。 |
| [L135](../src/apps/controller/ControllerMainWindow.h#L135) | `SelectMainPage` | 声明 | `void SelectMainPage(int pageIndex, QPushButton* navigationButton, const QString& title)` | 查询并返回 select main page 相关逻辑。 |
| [L138](../src/apps/controller/ControllerMainWindow.h#L138) | `AnimateNavigationIndicator` | 声明 | `void AnimateNavigationIndicator(QPushButton* navigationButton)` | 实现 animate navigation indicator 对应的业务或工具逻辑。 |
| [L139](../src/apps/controller/ControllerMainWindow.h#L139) | `QueueRoomWorkspaceActive` | 声明 | `void QueueRoomWorkspaceActive(bool active)` | 实现 queue room workspace active 对应的业务或工具逻辑。 |
| [L140](../src/apps/controller/ControllerMainWindow.h#L140) | `SetRoomWorkspaceActive` | 声明 | `void SetRoomWorkspaceActive(bool active)` | 更新或应用 set room workspace active 相关逻辑。 |
| [L141](../src/apps/controller/ControllerMainWindow.h#L141) | `SetAnimationLevel` | 声明 | `void SetAnimationLevel(int level)` | 更新或应用 set animation level 相关逻辑。 |
| [L142](../src/apps/controller/ControllerMainWindow.h#L142) | `ToggleAccountMenu` | 声明 | `void ToggleAccountMenu()` | 实现 toggle account menu 对应的业务或工具逻辑。 |
| [L143](../src/apps/controller/ControllerMainWindow.h#L143) | `ShowAccountMenu` | 声明 | `void ShowAccountMenu()` | 实现 show account menu 对应的业务或工具逻辑。 |
| [L144](../src/apps/controller/ControllerMainWindow.h#L144) | `HideAccountMenu` | 声明 | `void HideAccountMenu(bool animated = true)` | 实现 hide account menu 对应的业务或工具逻辑。 |
| [L145](../src/apps/controller/ControllerMainWindow.h#L145) | `UpdateAccountMenuGeometry` | 声明 | `void UpdateAccountMenuGeometry()` | 更新或应用 update account menu geometry 相关逻辑。 |
| [L146](../src/apps/controller/ControllerMainWindow.h#L146) | `StartAccountMenuMotion` | 声明 | `void StartAccountMenuMotion(const QPoint& targetPosition, int durationMs, bool hideWhenFinished)` | 启动 start account menu motion 相关逻辑。 |
| [L148](../src/apps/controller/ControllerMainWindow.h#L148) | `StopAccountMenuMotion` | 声明 | `void StopAccountMenuMotion()` | 停止 stop account menu motion 相关逻辑。 |
| [L149](../src/apps/controller/ControllerMainWindow.h#L149) | `UpdateAccountMenuHoverFromCursor` | 声明 | `void UpdateAccountMenuHoverFromCursor()` | 更新或应用 update account menu hover from cursor 相关逻辑。 |
| [L150](../src/apps/controller/ControllerMainWindow.h#L150) | `ApplyInterfaceTheme` | 声明 | `void ApplyInterfaceTheme(bool showFeedback = false)` | 更新或应用 apply interface theme 相关逻辑。 |
| [L151](../src/apps/controller/ControllerMainWindow.h#L151) | `SetInterfaceThemePreference` | 声明 | `void SetInterfaceThemePreference(const QString& value)` | 更新或应用 set interface theme preference 相关逻辑。 |
| [L152](../src/apps/controller/ControllerMainWindow.h#L152) | `RememberRecentRoom` | 声明 | `void RememberRecentRoom(const SessionEngineSnapshot& snapshot)` | 实现 remember recent room 对应的业务或工具逻辑。 |
| [L153](../src/apps/controller/ControllerMainWindow.h#L153) | `RememberRecentDevice` | 声明 | `void RememberRecentDevice(const SessionEngineSnapshot& snapshot)` | 实现 remember recent device 对应的业务或工具逻辑。 |
| [L154](../src/apps/controller/ControllerMainWindow.h#L154) | `RefreshRecentRooms` | 声明 | `void RefreshRecentRooms()` | 刷新 refresh recent rooms 相关逻辑。 |
| [L155](../src/apps/controller/ControllerMainWindow.h#L155) | `RefreshRecentDevices` | 声明 | `void RefreshRecentDevices()` | 刷新 refresh recent devices 相关逻辑。 |
| [L156](../src/apps/controller/ControllerMainWindow.h#L156) | `RecentSettingsKey` | 声明 | `QString RecentSettingsKey(const QString& listName) const` | 实现 recent settings key 对应的业务或工具逻辑。 |
| [L157](../src/apps/controller/ControllerMainWindow.h#L157) | `MigrateLegacyRecentHistory` | 声明 | `void MigrateLegacyRecentHistory()` | 实现 migrate legacy recent history 对应的业务或工具逻辑。 |
| [L158](../src/apps/controller/ControllerMainWindow.h#L158) | `RequestRecentRoomAvailability` | 声明 | `void RequestRecentRoomAvailability()` | 发起请求或查询 request recent room availability 相关逻辑。 |
| [L159](../src/apps/controller/ControllerMainWindow.h#L159) | `RefreshDiagnosticsUi` | 声明 | `void RefreshDiagnosticsUi()` | 刷新 refresh diagnostics ui 相关逻辑。 |
| [L160](../src/apps/controller/ControllerMainWindow.h#L160) | `StartDecoderBenchmark` | 声明 | `void StartDecoderBenchmark(bool manualRequest)` | 启动 start decoder benchmark 相关逻辑。 |
| [L161](../src/apps/controller/ControllerMainWindow.h#L161) | `FinishDecoderBenchmark` | 声明 | `void FinishDecoderBenchmark(int exitCode)` | 停止 finish decoder benchmark 相关逻辑。 |
| [L162](../src/apps/controller/ControllerMainWindow.h#L162) | `RefreshDecoderBenchmarkSummary` | 声明 | `void RefreshDecoderBenchmarkSummary()` | 刷新 refresh decoder benchmark summary 相关逻辑。 |
| [L163](../src/apps/controller/ControllerMainWindow.h#L163) | `RefreshDecoderHardwareSelectionAvailability` | 声明 | `void RefreshDecoderHardwareSelectionAvailability()` | 刷新 refresh decoder hardware selection availability 相关逻辑。 |
| [L164](../src/apps/controller/ControllerMainWindow.h#L164) | `StartEncoderBenchmark` | 声明 | `void StartEncoderBenchmark(bool manualRequest)` | 启动 start encoder benchmark 相关逻辑。 |
| [L165](../src/apps/controller/ControllerMainWindow.h#L165) | `FinishEncoderBenchmark` | 声明 | `void FinishEncoderBenchmark(int exitCode)` | 停止 finish encoder benchmark 相关逻辑。 |
| [L166](../src/apps/controller/ControllerMainWindow.h#L166) | `RefreshEncoderBenchmarkSummary` | 声明 | `void RefreshEncoderBenchmarkSummary()` | 刷新 refresh encoder benchmark summary 相关逻辑。 |
| [L167](../src/apps/controller/ControllerMainWindow.h#L167) | `UpdateLocalMediaDevicesUi` | 声明 | `void UpdateLocalMediaDevicesUi( const SessionEngineSnapshot& snapshot)` | 更新或应用 update local media devices ui 相关逻辑。 |
| [L169](../src/apps/controller/ControllerMainWindow.h#L169) | `PersistHardwareCapabilityCache` | 声明 | `void PersistHardwareCapabilityCache()` | 保存或写入 persist hardware capability cache 相关逻辑。 |
| [L170](../src/apps/controller/ControllerMainWindow.h#L170) | `RequestMediaDeviceRefresh` | 声明 | `void RequestMediaDeviceRefresh(bool userInitiated = false)` | 发起请求或查询 request media device refresh 相关逻辑。 |
| [L174](../src/apps/controller/ControllerMainWindow.h#L174) | `UpdateVideoPipelineSettingsAvailability` | 声明 | `void UpdateVideoPipelineSettingsAvailability( const SessionEngineSnapshot& snapshot)` | 更新或应用 update video pipeline settings availability 相关逻辑。 |
| [L176](../src/apps/controller/ControllerMainWindow.h#L176) | `ShowMediaDeviceMenu` | 声明 | `void ShowMediaDeviceMenu( MediaDeviceKind kind, QWidget* anchor)` | 实现 show media device menu 对应的业务或工具逻辑。 |
| [L178](../src/apps/controller/ControllerMainWindow.h#L178) | `BeginMediaDeviceSelection` | 声明 | `void BeginMediaDeviceSelection( MediaDeviceKind kind, const QString& deviceId)` | 启动 begin media device selection 相关逻辑。 |
| [L180](../src/apps/controller/ControllerMainWindow.h#L180) | `ApplyClipboardConfigurationFromUi` | 声明 | `void ApplyClipboardConfigurationFromUi(bool showFeedback = false)` | 更新或应用 apply clipboard configuration from ui 相关逻辑。 |
| [L181](../src/apps/controller/ControllerMainWindow.h#L181) | `UpdateClipboardSession` | 声明 | `void UpdateClipboardSession(const SessionEngineSnapshot& snapshot)` | 更新或应用 update clipboard session 相关逻辑。 |
| [L182](../src/apps/controller/ControllerMainWindow.h#L182) | `UpdateDirectFileTransferSession` | 声明 | `void UpdateDirectFileTransferSession( const SessionEngineSnapshot& snapshot)` | 更新或应用 update direct file transfer session 相关逻辑。 |
| [L186](../src/apps/controller/ControllerMainWindow.h#L186) | `eventFilter` | 声明 | `bool eventFilter(QObject* watched, QEvent* event) override` | 实现 event filter 对应的业务或工具逻辑。 |
| [L187](../src/apps/controller/ControllerMainWindow.h#L187) | `nativeEvent` | 声明 | `bool nativeEvent(const QByteArray& eventType, void* message, qintptr* result) override` | 实现 native event 对应的业务或工具逻辑。 |
| [L190](../src/apps/controller/ControllerMainWindow.h#L190) | `changeEvent` | 声明 | `void changeEvent(QEvent* event) override` | 实现 change event 对应的业务或工具逻辑。 |
| [L191](../src/apps/controller/ControllerMainWindow.h#L191) | `closeEvent` | 声明 | `void closeEvent(QCloseEvent* event) override` | 关闭并清理 close event 相关逻辑。 |
| [L192](../src/apps/controller/ControllerMainWindow.h#L192) | `resizeEvent` | 声明 | `void resizeEvent(QResizeEvent* event) override` | 实现 resize event 对应的业务或工具逻辑。 |

## `src/apps/controller/ControllerMainWindow.Lifecycle.inc`

[打开源码](../src/apps/controller/ControllerMainWindow.Lifecycle.inc) · **文件作用：** `ControllerMainWindow` 的实现切片，集中实现 lifecycle 相关逻辑；成员状态仍定义在所属头文件中。

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L4](../src/apps/controller/ControllerMainWindow.Lifecycle.inc#L4) | `ControllerMainWindow::ControllerMainWindow` | 定义 | `ControllerMainWindow::ControllerMainWindow( std::unique_ptr<ISessionEngine> engine, bool startEngineImmediately, QWidget* parent) : FramelessMainWindow(parent), inputExecutor_(std::make_unique<WindowsInputExecutor>())...` | SPDX-License-Identifier: GPL-3.0-only Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd) |
| [L236](../src/apps/controller/ControllerMainWindow.Lifecycle.inc#L236) | `ControllerMainWindow::~ControllerMainWindow` | 定义 | `ControllerMainWindow::~ControllerMainWindow()` | 停止相关活动并释放 ControllerMainWindow 实例拥有的资源。 |
| [L281](../src/apps/controller/ControllerMainWindow.Lifecycle.inc#L281) | `ControllerMainWindow::ActivateFromExternalLaunch` | 定义 | `void ControllerMainWindow::ActivateFromExternalLaunch()` | 实现 activate from external launch 对应的业务或工具逻辑。 |
| [L286](../src/apps/controller/ControllerMainWindow.Lifecycle.inc#L286) | `ControllerMainWindow::SetAccountInteractionCallback` | 定义 | `void ControllerMainWindow::SetAccountInteractionCallback( std::function<void()> callback)` | 更新或应用 set account interaction callback 相关逻辑。 |
| [L292](../src/apps/controller/ControllerMainWindow.Lifecycle.inc#L292) | `ControllerMainWindow::SetAccountSwitchCallback` | 定义 | `void ControllerMainWindow::SetAccountSwitchCallback( std::function<void()> callback)` | 更新或应用 set account switch callback 相关逻辑。 |
| [L298](../src/apps/controller/ControllerMainWindow.Lifecycle.inc#L298) | `ControllerMainWindow::SetAccountDeletionCallback` | 定义 | `void ControllerMainWindow::SetAccountDeletionCallback( std::function<void()> callback)` | 更新或应用 set account deletion callback 相关逻辑。 |
| [L304](../src/apps/controller/ControllerMainWindow.Lifecycle.inc#L304) | `ControllerMainWindow::SetAccountSignedOut` | 定义 | `void ControllerMainWindow::SetAccountSignedOut(const QString& message)` | 更新或应用 set account signed out 相关逻辑。 |
| [L360](../src/apps/controller/ControllerMainWindow.Lifecycle.inc#L360) | `ControllerMainWindow::SetAccountBusy` | 定义 | `void ControllerMainWindow::SetAccountBusy(const QString& message)` | 更新或应用 set account busy 相关逻辑。 |
| [L381](../src/apps/controller/ControllerMainWindow.Lifecycle.inc#L381) | `ControllerMainWindow::SetAccountSession` | 定义 | `void ControllerMainWindow::SetAccountSession( const QString& accountId, const QString& accountLabel, const QString& accountDetail, std::function<void()> signOutCallback)` | 更新或应用 set account session 相关逻辑。 |
| [L461](../src/apps/controller/ControllerMainWindow.Lifecycle.inc#L461) | `ControllerMainWindow::StartSessionEngine` | 定义 | `bool ControllerMainWindow::StartSessionEngine()` | 启动 start session engine 相关逻辑。 |
| [L475](../src/apps/controller/ControllerMainWindow.Lifecycle.inc#L475) | `ControllerMainWindow::StopSessionEngine` | 定义 | `void ControllerMainWindow::StopSessionEngine()` | 停止 stop session engine 相关逻辑。 |
| [L489](../src/apps/controller/ControllerMainWindow.Lifecycle.inc#L489) | `ControllerMainWindow::PrepareForApplicationExit` | 定义 | `void ControllerMainWindow::PrepareForApplicationExit()` | 实现 prepare for application exit 对应的业务或工具逻辑。 |
| [L498](../src/apps/controller/ControllerMainWindow.Lifecycle.inc#L498) | `ControllerMainWindow::ApplyAuthenticationAvailability` | 定义 | `void ControllerMainWindow::ApplyAuthenticationAvailability(bool authenticated)` | 更新或应用 apply authentication availability 相关逻辑。 |
| [L516](../src/apps/controller/ControllerMainWindow.Lifecycle.inc#L516) | `ControllerMainWindow::RequestMediaDeviceRefresh` | 定义 | `void ControllerMainWindow::RequestMediaDeviceRefresh( bool userInitiated)` | 发起请求或查询 request media device refresh 相关逻辑。 |

## `src/apps/controller/ControllerMainWindow.LocalDeviceV2Patch.inc`

[打开源码](../src/apps/controller/ControllerMainWindow.LocalDeviceV2Patch.inc) · **文件作用：** 所属主函数的连续实现片段；这里只包含该函数的局部流程，不单独声明成员变量或顶层函数。

当前规则未提取到公开类型、成员/文件级变量或函数；该文件可能只包含宏、模板片段、资源数据或条件编译内容，请直接打开源码查看。

## `src/apps/controller/ControllerMainWindow.MediaMenu.inc`

[打开源码](../src/apps/controller/ControllerMainWindow.MediaMenu.inc) · **文件作用：** `ControllerMainWindow` 的实现切片，集中实现 media menu 相关逻辑；成员状态仍定义在所属头文件中。

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L4](../src/apps/controller/ControllerMainWindow.MediaMenu.inc#L4) | `ControllerMainWindow::ShowMediaDeviceMenu` | 定义 | `void ControllerMainWindow::ShowMediaDeviceMenu( MediaDeviceKind kind, QWidget* anchor)` | SPDX-License-Identifier: GPL-3.0-only Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd) |
| [L298](../src/apps/controller/ControllerMainWindow.MediaMenu.inc#L298) | `ControllerMainWindow::BeginMediaDeviceSelection` | 定义 | `void ControllerMainWindow::BeginMediaDeviceSelection( MediaDeviceKind kind, const QString& deviceId)` | 启动 begin media device selection 相关逻辑。 |

## `src/apps/controller/ControllerMainWindow.NavigationRecent.inc`

[打开源码](../src/apps/controller/ControllerMainWindow.NavigationRecent.inc) · **文件作用：** `ControllerMainWindow` 的实现切片，集中实现 navigation recent 相关逻辑；成员状态仍定义在所属头文件中。

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L4](../src/apps/controller/ControllerMainWindow.NavigationRecent.inc#L4) | `ControllerMainWindow::RecentSettingsKey` | 定义 | `QString ControllerMainWindow::RecentSettingsKey( const QString& listName) const` | SPDX-License-Identifier: GPL-3.0-only Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd) |
| [L14](../src/apps/controller/ControllerMainWindow.NavigationRecent.inc#L14) | `ControllerMainWindow::MigrateLegacyRecentHistory` | 定义 | `void ControllerMainWindow::MigrateLegacyRecentHistory()` | 实现 migrate legacy recent history 对应的业务或工具逻辑。 |
| [L39](../src/apps/controller/ControllerMainWindow.NavigationRecent.inc#L39) | `ControllerMainWindow::RememberRecentRoom` | 定义 | `void ControllerMainWindow::RememberRecentRoom( const SessionEngineSnapshot& snapshot)` | 实现 remember recent room 对应的业务或工具逻辑。 |
| [L99](../src/apps/controller/ControllerMainWindow.NavigationRecent.inc#L99) | `ControllerMainWindow::RefreshRecentRooms` | 定义 | `void ControllerMainWindow::RefreshRecentRooms()` | 刷新 refresh recent rooms 相关逻辑。 |
| [L267](../src/apps/controller/ControllerMainWindow.NavigationRecent.inc#L267) | `ControllerMainWindow::RememberRecentDevice` | 定义 | `void ControllerMainWindow::RememberRecentDevice( const SessionEngineSnapshot& snapshot)` | 实现 remember recent device 对应的业务或工具逻辑。 |
| [L329](../src/apps/controller/ControllerMainWindow.NavigationRecent.inc#L329) | `ControllerMainWindow::RefreshRecentDevices` | 定义 | `void ControllerMainWindow::RefreshRecentDevices()` | 刷新 refresh recent devices 相关逻辑。 |
| [L479](../src/apps/controller/ControllerMainWindow.NavigationRecent.inc#L479) | `ControllerMainWindow::RequestRecentRoomAvailability` | 定义 | `void ControllerMainWindow::RequestRecentRoomAvailability()` | 发起请求或查询 request recent room availability 相关逻辑。 |
| [L505](../src/apps/controller/ControllerMainWindow.NavigationRecent.inc#L505) | `ControllerMainWindow::SelectMainPage` | 定义 | `void ControllerMainWindow::SelectMainPage( int pageIndex, QPushButton* navigationButton, const QString& title)` | 查询并返回 select main page 相关逻辑。 |
| [L537](../src/apps/controller/ControllerMainWindow.NavigationRecent.inc#L537) | `ControllerMainWindow::AnimateNavigationIndicator` | 定义 | `void ControllerMainWindow::AnimateNavigationIndicator( QPushButton* navigationButton)` | 实现 animate navigation indicator 对应的业务或工具逻辑。 |
| [L562](../src/apps/controller/ControllerMainWindow.NavigationRecent.inc#L562) | `ControllerMainWindow::QueueRoomWorkspaceActive` | 定义 | `void ControllerMainWindow::QueueRoomWorkspaceActive(bool active)` | 实现 queue room workspace active 对应的业务或工具逻辑。 |
| [L593](../src/apps/controller/ControllerMainWindow.NavigationRecent.inc#L593) | `ControllerMainWindow::SetRoomWorkspaceActive` | 定义 | `void ControllerMainWindow::SetRoomWorkspaceActive(bool active)` | 更新或应用 set room workspace active 相关逻辑。 |
| [L743](../src/apps/controller/ControllerMainWindow.NavigationRecent.inc#L743) | `ControllerMainWindow::SetAnimationLevel` | 定义 | `void ControllerMainWindow::SetAnimationLevel(int level)` | 更新或应用 set animation level 相关逻辑。 |

## `src/apps/controller/ControllerMainWindow.OwnedDevices.inc`

[打开源码](../src/apps/controller/ControllerMainWindow.OwnedDevices.inc) · **文件作用：** `ControllerMainWindow` 的实现切片，集中实现 owned devices 相关逻辑；成员状态仍定义在所属头文件中。

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L4](../src/apps/controller/ControllerMainWindow.OwnedDevices.inc#L4) | `ControllerMainWindow::RefreshOwnedDevicesUi` | 定义 | `void ControllerMainWindow::RefreshOwnedDevicesUi( const SessionEngineSnapshot& snapshot)` | SPDX-License-Identifier: GPL-3.0-only Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd) |
| [L139](../src/apps/controller/ControllerMainWindow.OwnedDevices.inc#L139) | `ControllerMainWindow::StartOwnedDeviceSession` | 定义 | `void ControllerMainWindow::StartOwnedDeviceSession( const QString& deviceId, const QString& deviceName)` | 启动 start owned device session 相关逻辑。 |

## `src/apps/controller/ControllerMainWindow.RoomApprovals.inc`

[打开源码](../src/apps/controller/ControllerMainWindow.RoomApprovals.inc) · **文件作用：** `ControllerMainWindow` 的实现切片，集中实现 room approvals 相关逻辑；成员状态仍定义在所属头文件中。

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L4](../src/apps/controller/ControllerMainWindow.RoomApprovals.inc#L4) | `ControllerMainWindow::QueueRoomJoinApproval` | 定义 | `void ControllerMainWindow::QueueRoomJoinApproval( const SessionEngineSnapshot& snapshot)` | SPDX-License-Identifier: GPL-3.0-only Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd) |
| [L105](../src/apps/controller/ControllerMainWindow.RoomApprovals.inc#L105) | `ControllerMainWindow::QueueRoomScreenShareSwitchApproval` | 定义 | `void ControllerMainWindow::QueueRoomScreenShareSwitchApproval( const SessionEngineSnapshot& snapshot)` | 实现 queue room screen share switch approval 对应的业务或工具逻辑。 |
| [L218](../src/apps/controller/ControllerMainWindow.RoomApprovals.inc#L218) | `ControllerMainWindow::QueueRoomControlApproval` | 定义 | `void ControllerMainWindow::QueueRoomControlApproval( const SessionEngineSnapshot& snapshot)` | 实现 queue room control approval 对应的业务或工具逻辑。 |
| [L319](../src/apps/controller/ControllerMainWindow.RoomApprovals.inc#L319) | `ControllerMainWindow::QueueRoomScreenShareViewApproval` | 定义 | `void ControllerMainWindow::QueueRoomScreenShareViewApproval( const SessionEngineSnapshot& snapshot)` | 实现 queue room screen share view approval 对应的业务或工具逻辑。 |

## `src/apps/controller/ControllerMainWindow.RoomMemberActions.inc`

[打开源码](../src/apps/controller/ControllerMainWindow.RoomMemberActions.inc) · **文件作用：** `ControllerMainWindow` 的实现切片，集中实现 room member actions 相关逻辑；成员状态仍定义在所属头文件中。

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L4](../src/apps/controller/ControllerMainWindow.RoomMemberActions.inc#L4) | `ControllerMainWindow::HandleRoomMemberActionResults` | 定义 | `void ControllerMainWindow::HandleRoomMemberActionResults( const SessionEngineSnapshot& snapshot)` | SPDX-License-Identifier: GPL-3.0-only Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd) |
| [L71](../src/apps/controller/ControllerMainWindow.RoomMemberActions.inc#L71) | `ControllerMainWindow::ShowRoomMemberContextMenu` | 定义 | `void ControllerMainWindow::ShowRoomMemberContextMenu( const QPoint& position)` | 实现 show room member context menu 对应的业务或工具逻辑。 |

## `src/apps/controller/ControllerMainWindow.RoomUi.inc`

[打开源码](../src/apps/controller/ControllerMainWindow.RoomUi.inc) · **文件作用：** `ControllerMainWindow` 的实现切片，集中实现 room ui 相关逻辑；成员状态仍定义在所属头文件中。

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L4](../src/apps/controller/ControllerMainWindow.RoomUi.inc#L4) | `ControllerMainWindow::UpdateRoomUi` | 定义 | `void ControllerMainWindow::UpdateRoomUi( const SessionEngineSnapshot& snapshot)` | SPDX-License-Identifier: GPL-3.0-only Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd) |

## `src/apps/controller/ControllerMainWindow.Session.inc`

[打开源码](../src/apps/controller/ControllerMainWindow.Session.inc) · **文件作用：** `ControllerMainWindow` 的实现切片，集中实现 session 相关逻辑；成员状态仍定义在所属头文件中。

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L4](../src/apps/controller/ControllerMainWindow.Session.inc#L4) | `ControllerMainWindow::StartSession` | 定义 | `void ControllerMainWindow::StartSession(const QString& deviceId, const QString& deviceName, SessionPurpose purpose)` | SPDX-License-Identifier: GPL-3.0-only Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd) |
| [L24](../src/apps/controller/ControllerMainWindow.Session.inc#L24) | `ControllerMainWindow::StartAssistedSession` | 定义 | `void ControllerMainWindow::StartAssistedSession( const QString& deviceId, const QString& verificationCode)` | 启动 start assisted session 相关逻辑。 |

## `src/apps/controller/ControllerMainWindow.SnapshotConnectivity.inc`

[打开源码](../src/apps/controller/ControllerMainWindow.SnapshotConnectivity.inc) · **文件作用：** 所属主函数的连续实现片段；这里只包含该函数的局部流程，不单独声明成员变量或顶层函数。

当前规则未提取到公开类型、成员/文件级变量或函数；该文件可能只包含宏、模板片段、资源数据或条件编译内容，请直接打开源码查看。

## `src/apps/controller/ControllerMainWindow.SnapshotCopy.inc`

[打开源码](../src/apps/controller/ControllerMainWindow.SnapshotCopy.inc) · **文件作用：** 所属主函数的连续实现片段；这里只包含该函数的局部流程，不单独声明成员变量或顶层函数。

当前规则未提取到公开类型、成员/文件级变量或函数；该文件可能只包含宏、模板片段、资源数据或条件编译内容，请直接打开源码查看。

## `src/apps/controller/ControllerMainWindow.SnapshotDebug.inc`

[打开源码](../src/apps/controller/ControllerMainWindow.SnapshotDebug.inc) · **文件作用：** 所属主函数的连续实现片段；这里只包含该函数的局部流程，不单独声明成员变量或顶层函数。

当前规则未提取到公开类型、成员/文件级变量或函数；该文件可能只包含宏、模板片段、资源数据或条件编译内容，请直接打开源码查看。

## `src/apps/controller/ControllerMainWindow.SnapshotLocal.inc`

[打开源码](../src/apps/controller/ControllerMainWindow.SnapshotLocal.inc) · **文件作用：** 所属主函数的连续实现片段；这里只包含该函数的局部流程，不单独声明成员变量或顶层函数。

当前规则未提取到公开类型、成员/文件级变量或函数；该文件可能只包含宏、模板片段、资源数据或条件编译内容，请直接打开源码查看。

## `src/apps/controller/ControllerMainWindow.SnapshotPreflight.inc`

[打开源码](../src/apps/controller/ControllerMainWindow.SnapshotPreflight.inc) · **文件作用：** `ControllerMainWindow` 的实现切片，集中实现 snapshot preflight 相关逻辑；成员状态仍定义在所属头文件中。

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L4](../src/apps/controller/ControllerMainWindow.SnapshotPreflight.inc#L4) | `ControllerMainWindow::OnSessionEngineSnapshot` | 定义 | `void ControllerMainWindow::OnSessionEngineSnapshot( const SessionEngineSnapshot& snapshot)` | SPDX-License-Identifier: GPL-3.0-only Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd) |

## `src/apps/controller/ControllerMainWindow.SnapshotWindows.inc`

[打开源码](../src/apps/controller/ControllerMainWindow.SnapshotWindows.inc) · **文件作用：** 所属主函数的连续实现片段；这里只包含该函数的局部流程，不单独声明成员变量或顶层函数。

当前规则未提取到公开类型、成员/文件级变量或函数；该文件可能只包含宏、模板片段、资源数据或条件编译内容，请直接打开源码查看。

## `src/apps/controller/ControllerMainWindow.SoftwareUpdate.inc`

[打开源码](../src/apps/controller/ControllerMainWindow.SoftwareUpdate.inc) · **文件作用：** `ControllerMainWindow` 的实现切片，集中实现 software update 相关逻辑；成员状态仍定义在所属头文件中。

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L4](../src/apps/controller/ControllerMainWindow.SoftwareUpdate.inc#L4) | `ControllerMainWindow::CheckForSoftwareUpdates` | 定义 | `void ControllerMainWindow::CheckForSoftwareUpdates(bool manualRequest)` | SPDX-License-Identifier: GPL-3.0-only Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd) |
| [L17](../src/apps/controller/ControllerMainWindow.SoftwareUpdate.inc#L17) | `ControllerMainWindow::HandleSoftwareUpdateState` | 定义 | `void ControllerMainWindow::HandleSoftwareUpdateState( const update::SoftwareUpdateController::Snapshot& snapshot)` | 接收并处理 handle software update state 相关逻辑。 |
| [L98](../src/apps/controller/ControllerMainWindow.SoftwareUpdate.inc#L98) | `ControllerMainWindow::OpenSoftwareUpdate` | 定义 | `void ControllerMainWindow::OpenSoftwareUpdate()` | 启动 open software update 相关逻辑。 |
| [L157](../src/apps/controller/ControllerMainWindow.SoftwareUpdate.inc#L157) | `ControllerMainWindow::QuitForSoftwareUpdate` | 定义 | `void ControllerMainWindow::QuitForSoftwareUpdate()` | 实现 quit for software update 对应的业务或工具逻辑。 |

## `src/apps/controller/ControllerMainWindow.Theme.inc`

[打开源码](../src/apps/controller/ControllerMainWindow.Theme.inc) · **文件作用：** `ControllerMainWindow` 的实现切片，集中实现 theme 相关逻辑；成员状态仍定义在所属头文件中。

### 类型

| 行 | 类型 | 种类 | 作用 |
|---:|---|---|---|
| [L11](../src/apps/controller/ControllerMainWindow.Theme.inc#L11) | `ScrollPosition` | struct | 定义 ScrollPosition 的 struct 类型和相关状态。 |

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L4](../src/apps/controller/ControllerMainWindow.Theme.inc#L4) | `ControllerMainWindow::ApplyInterfaceTheme` | 定义 | `void ControllerMainWindow::ApplyInterfaceTheme(bool showFeedback)` | SPDX-License-Identifier: GPL-3.0-only Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd) |
| [L196](../src/apps/controller/ControllerMainWindow.Theme.inc#L196) | `ControllerMainWindow::SetInterfaceThemePreference` | 定义 | `void ControllerMainWindow::SetInterfaceThemePreference( const QString& value)` | 更新或应用 set interface theme preference 相关逻辑。 |
| [L204](../src/apps/controller/ControllerMainWindow.Theme.inc#L204) | `ControllerMainWindow::RunThemeRoundTripSelfTest` | 定义 | `bool ControllerMainWindow::RunThemeRoundTripSelfTest(QString* errorMessage)` | 执行后台循环或调度 run theme round trip self test 相关逻辑。 |

## `src/apps/controller/ControllerMainWindow.UiConnections.inc`

[打开源码](../src/apps/controller/ControllerMainWindow.UiConnections.inc) · **文件作用：** 所属主函数的连续实现片段；这里只包含该函数的局部流程，不单独声明成员变量或顶层函数。

当前规则未提取到公开类型、成员/文件级变量或函数；该文件可能只包含宏、模板片段、资源数据或条件编译内容，请直接打开源码查看。

## `src/apps/controller/ControllerMainWindow.UiDeviceRecent.inc`

[打开源码](../src/apps/controller/ControllerMainWindow.UiDeviceRecent.inc) · **文件作用：** 所属主函数的连续实现片段；这里只包含该函数的局部流程，不单独声明成员变量或顶层函数。

当前规则未提取到公开类型、成员/文件级变量或函数；该文件可能只包含宏、模板片段、资源数据或条件编译内容，请直接打开源码查看。

## `src/apps/controller/ControllerMainWindow.UiDiagnostics.inc`

[打开源码](../src/apps/controller/ControllerMainWindow.UiDiagnostics.inc) · **文件作用：** 所属主函数的连续实现片段；这里只包含该函数的局部流程，不单独声明成员变量或顶层函数。

当前规则未提取到公开类型、成员/文件级变量或函数；该文件可能只包含宏、模板片段、资源数据或条件编译内容，请直接打开源码查看。

## `src/apps/controller/ControllerMainWindow.UiOwnedDevices.inc`

[打开源码](../src/apps/controller/ControllerMainWindow.UiOwnedDevices.inc) · **文件作用：** 所属主函数的连续实现片段；这里只包含该函数的局部流程，不单独声明成员变量或顶层函数。

当前规则未提取到公开类型、成员/文件级变量或函数；该文件可能只包含宏、模板片段、资源数据或条件编译内容，请直接打开源码查看。

## `src/apps/controller/ControllerMainWindow.UiSettings.inc`

[打开源码](../src/apps/controller/ControllerMainWindow.UiSettings.inc) · **文件作用：** 所属主函数的连续实现片段；这里只包含该函数的局部流程，不单独声明成员变量或顶层函数。

当前规则未提取到公开类型、成员/文件级变量或函数；该文件可能只包含宏、模板片段、资源数据或条件编译内容，请直接打开源码查看。

## `src/apps/controller/ControllerMainWindow.UiShellRoom.inc`

[打开源码](../src/apps/controller/ControllerMainWindow.UiShellRoom.inc) · **文件作用：** `ControllerMainWindow` 的实现切片，集中实现 ui shell room 相关逻辑；成员状态仍定义在所属头文件中。

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L4](../src/apps/controller/ControllerMainWindow.UiShellRoom.inc#L4) | `ControllerMainWindow::BuildUi` | 定义 | `void ControllerMainWindow::BuildUi()` | SPDX-License-Identifier: GPL-3.0-only Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd) |

## `src/apps/controller/ControllerMainWindow.WindowLifecycle.inc`

[打开源码](../src/apps/controller/ControllerMainWindow.WindowLifecycle.inc) · **文件作用：** `ControllerMainWindow` 的实现切片，集中实现 window lifecycle 相关逻辑；成员状态仍定义在所属头文件中。

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L4](../src/apps/controller/ControllerMainWindow.WindowLifecycle.inc#L4) | `ControllerMainWindow::eventFilter` | 定义 | `bool ControllerMainWindow::eventFilter(QObject* watched, QEvent* event)` | SPDX-License-Identifier: GPL-3.0-only Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd) |
| [L92](../src/apps/controller/ControllerMainWindow.WindowLifecycle.inc#L92) | `ControllerMainWindow::nativeEvent` | 定义 | `bool ControllerMainWindow::nativeEvent( const QByteArray& eventType, void* message, qintptr* result)` | 实现 native event 对应的业务或工具逻辑。 |
| [L112](../src/apps/controller/ControllerMainWindow.WindowLifecycle.inc#L112) | `ControllerMainWindow::closeEvent` | 定义 | `void ControllerMainWindow::closeEvent(QCloseEvent* event)` | 关闭并清理 close event 相关逻辑。 |
| [L143](../src/apps/controller/ControllerMainWindow.WindowLifecycle.inc#L143) | `ControllerMainWindow::changeEvent` | 定义 | `void ControllerMainWindow::changeEvent(QEvent* event)` | 实现 change event 对应的业务或工具逻辑。 |
| [L155](../src/apps/controller/ControllerMainWindow.WindowLifecycle.inc#L155) | `ControllerMainWindow::resizeEvent` | 定义 | `void ControllerMainWindow::resizeEvent(QResizeEvent* event)` | 实现 resize event 对应的业务或工具逻辑。 |
| [L164](../src/apps/controller/ControllerMainWindow.WindowLifecycle.inc#L164) | `ControllerMainWindow::ToggleAccountMenu` | 定义 | `void ControllerMainWindow::ToggleAccountMenu()` | 实现 toggle account menu 对应的业务或工具逻辑。 |
| [L171](../src/apps/controller/ControllerMainWindow.WindowLifecycle.inc#L171) | `ControllerMainWindow::UpdateAccountMenuGeometry` | 定义 | `void ControllerMainWindow::UpdateAccountMenuGeometry()` | 更新或应用 update account menu geometry 相关逻辑。 |
| [L190](../src/apps/controller/ControllerMainWindow.WindowLifecycle.inc#L190) | `ControllerMainWindow::UpdateAccountMenuHoverFromCursor` | 定义 | `void ControllerMainWindow::UpdateAccountMenuHoverFromCursor()` | 更新或应用 update account menu hover from cursor 相关逻辑。 |
| [L209](../src/apps/controller/ControllerMainWindow.WindowLifecycle.inc#L209) | `ControllerMainWindow::ShowAccountMenu` | 定义 | `void ControllerMainWindow::ShowAccountMenu()` | 实现 show account menu 对应的业务或工具逻辑。 |
| [L269](../src/apps/controller/ControllerMainWindow.WindowLifecycle.inc#L269) | `ControllerMainWindow::HideAccountMenu` | 定义 | `void ControllerMainWindow::HideAccountMenu(bool animated)` | 实现 hide account menu 对应的业务或工具逻辑。 |
| [L293](../src/apps/controller/ControllerMainWindow.WindowLifecycle.inc#L293) | `ControllerMainWindow::StartAccountMenuMotion` | 定义 | `void ControllerMainWindow::StartAccountMenuMotion( const QPoint& targetPosition, int durationMs, bool hideWhenFinished)` | 启动 start account menu motion 相关逻辑。 |
| [L339](../src/apps/controller/ControllerMainWindow.WindowLifecycle.inc#L339) | `ControllerMainWindow::StopAccountMenuMotion` | 定义 | `void ControllerMainWindow::StopAccountMenuMotion()` | 停止 stop account menu motion 相关逻辑。 |
| [L346](../src/apps/controller/ControllerMainWindow.WindowLifecycle.inc#L346) | `ControllerMainWindow::BuildSystemTray` | 定义 | `void ControllerMainWindow::BuildSystemTray()` | 创建或初始化 build system tray 相关逻辑。 |
| [L434](../src/apps/controller/ControllerMainWindow.WindowLifecycle.inc#L434) | `ControllerMainWindow::ShowFromSystemTray` | 定义 | `void ControllerMainWindow::ShowFromSystemTray()` | 实现 show from system tray 对应的业务或工具逻辑。 |
| [L462](../src/apps/controller/ControllerMainWindow.WindowLifecycle.inc#L462) | `ControllerMainWindow::QuitFromSystemTray` | 定义 | `void ControllerMainWindow::QuitFromSystemTray()` | 实现 quit from system tray 对应的业务或工具逻辑。 |
| [L482](../src/apps/controller/ControllerMainWindow.WindowLifecycle.inc#L482) | `ControllerMainWindow::DestroyAuxiliaryWindowsForExit` | 定义 | `void ControllerMainWindow::DestroyAuxiliaryWindowsForExit()` | 关闭并清理 destroy auxiliary windows for exit 相关逻辑。 |

## `src/apps/controller/ControllerProductMain.cpp`

[打开源码](../src/apps/controller/ControllerProductMain.cpp) · **文件作用：** 实现 controller product main 相关函数与文件级辅助逻辑。

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L10](../src/apps/controller/ControllerProductMain.cpp#L10) | `main` | 定义 | `int main(int argc, char* argv[])` | 实现 main 对应的业务或工具逻辑。 |

## `src/apps/controller/D3D11VideoSurface.inc`

[打开源码](../src/apps/controller/D3D11VideoSurface.inc) · **文件作用：** `D3D11VideoSurface` 的实现切片，集中实现 d3 d11 video surface 相关逻辑；成员状态仍定义在所属头文件中。

### 类型

| 行 | 类型 | 种类 | 作用 |
|---:|---|---|---|
| [L7](../src/apps/controller/D3D11VideoSurface.inc#L7) | `D3D11VideoSurface` | class | Presents a decoder-owned NV12 texture through a swap chain created on the decoder's D3D11 device. The canvas keeps the owning WebRTC frame alive while Present() runs, so the act... |
| [L9](../src/apps/controller/D3D11VideoSurface.inc#L9) | `PresentTiming` | struct | 定义 PresentTiming 的 struct 类型和相关状态。 |
| [L247](../src/apps/controller/D3D11VideoSurface.inc#L247) | `I420PlaneTexture` | struct | 定义 I420PlaneTexture 的 struct 类型和相关状态。 |
| [L252](../src/apps/controller/D3D11VideoSurface.inc#L252) | `I420UploadSlot` | struct | 定义 I420UploadSlot 的 struct 类型和相关状态。 |
| [L258](../src/apps/controller/D3D11VideoSurface.inc#L258) | `CursorConstants` | struct | 定义 CursorConstants 的 struct 类型和相关状态。 |
| [L279](../src/apps/controller/D3D11VideoSurface.inc#L279) | `VertexOutput` | struct | 定义 VertexOutput 的 struct 类型和相关状态。 |
| [L493](../src/apps/controller/D3D11VideoSurface.inc#L493) | `VertexOutput` | struct | 定义 VertexOutput 的 struct 类型和相关状态。 |

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L16](../src/apps/controller/D3D11VideoSurface.inc#L16) | `SetCursorFrame` | 定义 | `void SetCursorFrame( const RemoteCursorRenderState::Snapshot& snapshot, int sourceWidth, int sourceHeight)` | 更新或应用 set cursor frame 相关逻辑。 |
| [L29](../src/apps/controller/D3D11VideoSurface.inc#L29) | `D3D11VideoSurface` | 定义 | `explicit D3D11VideoSurface(QWidget* parent = nullptr) : QWidget(parent)` | 实现 d3 d11 video surface 对应的业务或工具逻辑。 |
| [L44](../src/apps/controller/D3D11VideoSurface.inc#L44) | `nativeEvent` | 定义 | `bool nativeEvent(const QByteArray& eventType, void* message, qintptr* result) override` | 实现 native event 对应的业务或工具逻辑。 |
| [L71](../src/apps/controller/D3D11VideoSurface.inc#L71) | `Present` | 定义 | `PresentTiming Present(D3D11NativeFrameBuffer* frame)` | 准备或呈现 present 相关逻辑。 |
| [L83](../src/apps/controller/D3D11VideoSurface.inc#L83) | `PresentCpuBgra` | 定义 | `PresentTiming PresentCpuBgra(const QImage& image)` | 准备或呈现 present cpu bgra 相关逻辑。 |
| [L105](../src/apps/controller/D3D11VideoSurface.inc#L105) | `PresentCpuNv12` | 定义 | `PresentTiming PresentCpuNv12(const QImage& image)` | 准备或呈现 present cpu nv12 相关逻辑。 |
| [L139](../src/apps/controller/D3D11VideoSurface.inc#L139) | `PresentCpuI420` | 定义 | `PresentTiming PresentCpuI420( webrtc::I420BufferInterface* frame)` | 准备或呈现 present cpu i420 相关逻辑。 |
| [L243](../src/apps/controller/D3D11VideoSurface.inc#L243) | `paintEngine` | 定义 | `QPaintEngine* paintEngine() const override { return nullptr; }` | 准备或呈现 paint engine 相关逻辑。 |
| [L244](../src/apps/controller/D3D11VideoSurface.inc#L244) | `paintEvent` | 定义 | `void paintEvent(QPaintEvent*) override {}` | 准备或呈现 paint event 相关逻辑。 |
| [L265](../src/apps/controller/D3D11VideoSurface.inc#L265) | `EnsureCursorPipeline` | 定义 | `bool EnsureCursorPipeline()` | 实现 ensure cursor pipeline 对应的业务或工具逻辑。 |
| [L363](../src/apps/controller/D3D11VideoSurface.inc#L363) | `UpdateCursorTexture` | 定义 | `bool UpdateCursorTexture()` | 更新或应用 update cursor texture 相关逻辑。 |
| [L408](../src/apps/controller/D3D11VideoSurface.inc#L408) | `DrawCursorOverlay` | 定义 | `void DrawCursorOverlay(ID3D11RenderTargetView* renderTarget, UINT outputWidth, UINT outputHeight)` | 准备或呈现 draw cursor overlay 相关逻辑。 |
| [L466](../src/apps/controller/D3D11VideoSurface.inc#L466) | `ResetCursorResources` | 定义 | `void ResetCursorResources()` | 重置或移除 reset cursor resources 相关逻辑。 |
| [L481](../src/apps/controller/D3D11VideoSurface.inc#L481) | `EnsureI420ShaderPipeline` | 定义 | `bool EnsureI420ShaderPipeline()` | 实现 ensure i420 shader pipeline 对应的业务或工具逻辑。 |
| [L567](../src/apps/controller/D3D11VideoSurface.inc#L567) | `CreateI420PlaneTexture` | 定义 | `bool CreateI420PlaneTexture( UINT width, UINT height, I420PlaneTexture* plane)` | 创建或初始化 create i420 plane texture 相关逻辑。 |
| [L589](../src/apps/controller/D3D11VideoSurface.inc#L589) | `EnsureI420UploadTextures` | 定义 | `bool EnsureI420UploadTextures(UINT width, UINT height)` | 实现 ensure i420 upload textures 对应的业务或工具逻辑。 |
| [L613](../src/apps/controller/D3D11VideoSurface.inc#L613) | `UploadI420Plane` | 定义 | `bool UploadI420Plane(I420PlaneTexture& plane, const std::uint8_t* source, int sourceStride, UINT width, UINT height)` | 准备或呈现 upload i420 plane 相关逻辑。 |
| [L636](../src/apps/controller/D3D11VideoSurface.inc#L636) | `EnsureI420BackBufferView` | 定义 | `bool EnsureI420BackBufferView()` | 实现 ensure i420 back buffer view 对应的业务或工具逻辑。 |
| [L648](../src/apps/controller/D3D11VideoSurface.inc#L648) | `ResetI420UploadTextures` | 定义 | `void ResetI420UploadTextures()` | 重置或移除 reset i420 upload textures 相关逻辑。 |
| [L660](../src/apps/controller/D3D11VideoSurface.inc#L660) | `ResetI420ShaderResources` | 定义 | `void ResetI420ShaderResources()` | 重置或移除 reset i420 shader resources 相关逻辑。 |
| [L670](../src/apps/controller/D3D11VideoSurface.inc#L670) | `PresentTexture` | 定义 | `PresentTiming PresentTexture( ID3D11Device* device, ID3D11Texture2D* texture, UINT subresourceIndex, int sourceWidth, int sourceHeight, DXGI_FORMAT expectedFormat)` | 准备或呈现 present texture 相关逻辑。 |
| [L795](../src/apps/controller/D3D11VideoSurface.inc#L795) | `EnsureCpuDevice` | 定义 | `bool EnsureCpuDevice()` | 实现 ensure cpu device 对应的业务或工具逻辑。 |
| [L828](../src/apps/controller/D3D11VideoSurface.inc#L828) | `EnsureCpuUploadTexture` | 定义 | `bool EnsureCpuUploadTexture( UINT width, UINT height, DXGI_FORMAT format)` | 实现 ensure cpu upload texture 对应的业务或工具逻辑。 |
| [L857](../src/apps/controller/D3D11VideoSurface.inc#L857) | `EnsureDevice` | 定义 | `bool EnsureDevice(ID3D11Device* device)` | 实现 ensure device 对应的业务或工具逻辑。 |
| [L898](../src/apps/controller/D3D11VideoSurface.inc#L898) | `EnsureSwapChain` | 定义 | `bool EnsureSwapChain(HWND window, UINT width, UINT height)` | 实现 ensure swap chain 对应的业务或工具逻辑。 |
| [L940](../src/apps/controller/D3D11VideoSurface.inc#L940) | `EnsureVideoProcessor` | 定义 | `bool EnsureVideoProcessor(UINT sourceWidth, UINT sourceHeight, UINT outputWidth, UINT outputHeight)` | 实现 ensure video processor 对应的业务或工具逻辑。 |
| [L978](../src/apps/controller/D3D11VideoSurface.inc#L978) | `ResetDevice` | 定义 | `void ResetDevice()` | 重置或移除 reset device 相关逻辑。 |

## `src/apps/controller/FileTransferWindow.cpp`

[打开源码](../src/apps/controller/FileTransferWindow.cpp) · **文件作用：** 实现 file transfer window 相关函数与文件级辅助逻辑。

### 类型

| 行 | 类型 | 种类 | 作用 |
|---:|---|---|---|
| [L451](../src/apps/controller/FileTransferWindow.cpp#L451) | `FileTransferCard` | class | 定义 FileTransferCard 的 class 类型和相关状态。 |
| [L453](../src/apps/controller/FileTransferWindow.cpp#L453) | `Actions` | struct | 定义 Actions 的 struct 类型和相关状态。 |

### 成员与文件级变量

| 行 | 变量 | 声明 | 作用 |
|---:|---|---|---|
| [L51](../src/apps/controller/FileTransferWindow.cpp#L51) | `kDefaultFileTransferDrawerWidth` | `constexpr int kDefaultFileTransferDrawerWidth = 390;` | 定义 default file transfer drawer width 的编译期常量或产品边界。 |
| [L52](../src/apps/controller/FileTransferWindow.cpp#L52) | `kMinimumFileTransferDrawerWidth` | `constexpr int kMinimumFileTransferDrawerWidth = 340;` | 定义 minimum file transfer drawer width 的编译期常量或产品边界。 |
| [L53](../src/apps/controller/FileTransferWindow.cpp#L53) | `kDrawerResizeHandleWidth` | `constexpr int kDrawerResizeHandleWidth = 8;` | 定义 drawer resize handle width 的编译期常量或产品边界。 |

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L55](../src/apps/controller/FileTransferWindow.cpp#L55) | `DrawerRevealEasing` | 定义 | `const QEasingCurve& DrawerRevealEasing()` | 准备或呈现 drawer reveal easing 相关逻辑。 |
| [L66](../src/apps/controller/FileTransferWindow.cpp#L66) | `DrawerDismissEasing` | 定义 | `const QEasingCurve& DrawerDismissEasing()` | 准备或呈现 drawer dismiss easing 相关逻辑。 |
| [L77](../src/apps/controller/FileTransferWindow.cpp#L77) | `InitialFileSaveDirectory` | 定义 | `QString InitialFileSaveDirectory()` | 创建或初始化 initial file save directory 相关逻辑。 |
| [L278](../src/apps/controller/FileTransferWindow.cpp#L278) | `FormatBytes` | 定义 | `QString FormatBytes(std::uint64_t bytes)` | 实现 format bytes 对应的业务或工具逻辑。 |
| [L295](../src/apps/controller/FileTransferWindow.cpp#L295) | `FormatRemainingTime` | 定义 | `QString FormatRemainingTime(std::uint64_t seconds)` | 实现 format remaining time 对应的业务或工具逻辑。 |
| [L310](../src/apps/controller/FileTransferWindow.cpp#L310) | `StateText` | 定义 | `QString StateText(app::FileTransferState state)` | 实现 state text 对应的业务或工具逻辑。 |
| [L343](../src/apps/controller/FileTransferWindow.cpp#L343) | `FileTransferErrorText` | 定义 | `QString FileTransferErrorText(const std::string& errorCode, const std::string& errorMessage)` | 实现 file transfer error text 对应的业务或工具逻辑。 |
| [L444](../src/apps/controller/FileTransferWindow.cpp#L444) | `FileTransferErrorText` | 定义 | `QString FileTransferErrorText(const app::FileTransferSnapshot& transfer)` | 实现 file transfer error text 对应的业务或工具逻辑。 |
| [L462](../src/apps/controller/FileTransferWindow.cpp#L462) | `FileTransferCard` | 定义 | `FileTransferCard(Actions actions, QWidget* parent) : QFrame(parent), actions_(std::move(actions))` | 实现 file transfer card 对应的业务或工具逻辑。 |
| [L518](../src/apps/controller/FileTransferWindow.cpp#L518) | `Update` | 定义 | `void Update(const app::FileTransferSnapshot& transfer, bool animate)` | 更新或应用 update 相关逻辑。 |
| [L617](../src/apps/controller/FileTransferWindow.cpp#L617) | `AddAction` | 定义 | `QPushButton* AddAction(const QString& text, const char* tone, std::function<void()> callback)` | 实现 add action 对应的业务或工具逻辑。 |
| [L706](../src/apps/controller/FileTransferWindow.cpp#L706) | `RebuildActions` | 定义 | `void RebuildActions(const app::FileTransferSnapshot& transfer)` | 更新或应用 rebuild actions 相关逻辑。 |
| [L742](../src/apps/controller/FileTransferWindow.cpp#L742) | `AnimateStateLabel` | 定义 | `void AnimateStateLabel()` | 实现 animate state label 对应的业务或工具逻辑。 |
| [L777](../src/apps/controller/FileTransferWindow.cpp#L777) | `FileTransferWindow::FileTransferWindow` | 定义 | `FileTransferWindow::FileTransferWindow( app::FileTransferController* controller, QWidget* parent) : FramelessMainWindow(parent, true), controller_(controller)` | 构造并初始化 FileTransferWindow 实例。 |
| [L788](../src/apps/controller/FileTransferWindow.cpp#L788) | `FileTransferWindow::~FileTransferWindow` | 定义 | `FileTransferWindow::~FileTransferWindow()` | 停止相关活动并释放 FileTransferWindow 实例拥有的资源。 |
| [L793](../src/apps/controller/FileTransferWindow.cpp#L793) | `FileTransferWindow::DetachController` | 定义 | `void FileTransferWindow::DetachController()` | 实现 detach controller 对应的业务或工具逻辑。 |
| [L802](../src/apps/controller/FileTransferWindow.cpp#L802) | `FileTransferWindow::SyncPeers` | 定义 | `void FileTransferWindow::SyncPeers( const std::vector<FileTransferPeer>& peers, const QRect& mainWindowGeometry)` | 实现 sync peers 对应的业务或工具逻辑。 |
| [L825](../src/apps/controller/FileTransferWindow.cpp#L825) | `FileTransferWindow::AttachAsDrawer` | 定义 | `void FileTransferWindow::AttachAsDrawer(QWidget* host)` | 实现 attach as drawer 对应的业务或工具逻辑。 |
| [L839](../src/apps/controller/FileTransferWindow.cpp#L839) | `FileTransferWindow::EffectiveDrawerWidth` | 定义 | `int FileTransferWindow::EffectiveDrawerWidth() const` | 实现 effective drawer width 对应的业务或工具逻辑。 |
| [L852](../src/apps/controller/FileTransferWindow.cpp#L852) | `FileTransferWindow::SetDrawerWidth` | 定义 | `void FileTransferWindow::SetDrawerWidth(int requestedWidth)` | 更新或应用 set drawer width 相关逻辑。 |
| [L867](../src/apps/controller/FileTransferWindow.cpp#L867) | `FileTransferWindow::OpenBesideMainWindow` | 定义 | `void FileTransferWindow::OpenBesideMainWindow( const QRect& mainWindowGeometry)` | 启动 open beside main window 相关逻辑。 |
| [L894](../src/apps/controller/FileTransferWindow.cpp#L894) | `FileTransferWindow::HideWithAnimation` | 定义 | `void FileTransferWindow::HideWithAnimation()` | 实现 hide with animation 对应的业务或工具逻辑。 |
| [L910](../src/apps/controller/FileTransferWindow.cpp#L910) | `FileTransferWindow::HideImmediately` | 定义 | `void FileTransferWindow::HideImmediately()` | 实现 hide immediately 对应的业务或工具逻辑。 |
| [L920](../src/apps/controller/FileTransferWindow.cpp#L920) | `FileTransferWindow::UpdateDrawerGeometry` | 定义 | `void FileTransferWindow::UpdateDrawerGeometry()` | 更新或应用 update drawer geometry 相关逻辑。 |
| [L939](../src/apps/controller/FileTransferWindow.cpp#L939) | `FileTransferWindow::IsHiding` | 定义 | `bool FileTransferWindow::IsHiding() const` | 判断 is hiding 相关逻辑。 |
| [L944](../src/apps/controller/FileTransferWindow.cpp#L944) | `FileTransferWindow::StartDrawerMotion` | 定义 | `void FileTransferWindow::StartDrawerMotion(const QPoint& targetPosition, int durationMs)` | 启动 start drawer motion 相关逻辑。 |
| [L985](../src/apps/controller/FileTransferWindow.cpp#L985) | `FileTransferWindow::StopDrawerAnimation` | 定义 | `void FileTransferWindow::StopDrawerAnimation()` | 停止 stop drawer animation 相关逻辑。 |
| [L991](../src/apps/controller/FileTransferWindow.cpp#L991) | `FileTransferWindow::closeEvent` | 定义 | `void FileTransferWindow::closeEvent(QCloseEvent* event)` | 关闭并清理 close event 相关逻辑。 |
| [L997](../src/apps/controller/FileTransferWindow.cpp#L997) | `FileTransferWindow::resizeEvent` | 定义 | `void FileTransferWindow::resizeEvent(QResizeEvent* event)` | 实现 resize event 对应的业务或工具逻辑。 |
| [L1007](../src/apps/controller/FileTransferWindow.cpp#L1007) | `FileTransferWindow::eventFilter` | 定义 | `bool FileTransferWindow::eventFilter(QObject* watched, QEvent* event)` | 实现 event filter 对应的业务或工具逻辑。 |
| [L1046](../src/apps/controller/FileTransferWindow.cpp#L1046) | `FileTransferWindow::OnFileTransfersChanged` | 定义 | `void FileTransferWindow::OnFileTransfersChanged( const std::vector<app::FileTransferSnapshot>& transfers)` | 接收并处理 on file transfers changed 相关逻辑。 |
| [L1070](../src/apps/controller/FileTransferWindow.cpp#L1070) | `FileTransferWindow::ScheduleTransferApply` | 定义 | `void FileTransferWindow::ScheduleTransferApply()` | 执行后台循环或调度 schedule transfer apply 相关逻辑。 |
| [L1087](../src/apps/controller/FileTransferWindow.cpp#L1087) | `FileTransferWindow::BuildUi` | 定义 | `void FileTransferWindow::BuildUi()` | 创建或初始化 build ui 相关逻辑。 |
| [L1198](../src/apps/controller/FileTransferWindow.cpp#L1198) | `FileTransferWindow::RefreshThemeStyle` | 定义 | `void FileTransferWindow::RefreshThemeStyle()` | 刷新 refresh theme style 相关逻辑。 |
| [L1213](../src/apps/controller/FileTransferWindow.cpp#L1213) | `FileTransferWindow::ApplyTransfers` | 定义 | `void FileTransferWindow::ApplyTransfers( std::vector<app::FileTransferSnapshot> transfers)` | 更新或应用 apply transfers 相关逻辑。 |
| [L1246](../src/apps/controller/FileTransferWindow.cpp#L1246) | `FileTransferWindow::UpdateTransferCardsIncrementally` | 定义 | `void FileTransferWindow::UpdateTransferCardsIncrementally()` | 更新或应用 update transfer cards incrementally 相关逻辑。 |
| [L1373](../src/apps/controller/FileTransferWindow.cpp#L1373) | `FileTransferWindow::RebuildTransferCards` | 定义 | `void FileTransferWindow::RebuildTransferCards()` | 更新或应用 rebuild transfer cards 相关逻辑。 |
| [L1587](../src/apps/controller/FileTransferWindow.cpp#L1587) | `FileTransferWindow::ChooseAndSendFile` | 定义 | `void FileTransferWindow::ChooseAndSendFile()` | 实现 choose and send file 对应的业务或工具逻辑。 |
| [L1607](../src/apps/controller/FileTransferWindow.cpp#L1607) | `FileTransferWindow::AcceptTransfer` | 定义 | `void FileTransferWindow::AcceptTransfer(const std::string& transferId)` | 处理并回复 accept transfer 相关逻辑。 |
| [L1623](../src/apps/controller/FileTransferWindow.cpp#L1623) | `FileTransferWindow::SaveTransferAs` | 定义 | `void FileTransferWindow::SaveTransferAs(const std::string& transferId)` | 保存或写入 save transfer as 相关逻辑。 |
| [L1637](../src/apps/controller/FileTransferWindow.cpp#L1637) | `FileTransferWindow::AcceptTransferToDirectory` | 定义 | `void FileTransferWindow::AcceptTransferToDirectory( const std::string& transferId, const QString& directory)` | 处理并回复 accept transfer to directory 相关逻辑。 |
| [L1677](../src/apps/controller/FileTransferWindow.cpp#L1677) | `FileTransferWindow::RejectTransfer` | 定义 | `void FileTransferWindow::RejectTransfer(const std::string& transferId)` | 处理并回复 reject transfer 相关逻辑。 |
| [L1690](../src/apps/controller/FileTransferWindow.cpp#L1690) | `FileTransferWindow::CancelTransfer` | 定义 | `void FileTransferWindow::CancelTransfer(const std::string& transferId)` | 判断 cancel transfer 相关逻辑。 |
| [L1703](../src/apps/controller/FileTransferWindow.cpp#L1703) | `FileTransferWindow::ResumeTransfer` | 定义 | `void FileTransferWindow::ResumeTransfer(const std::string& transferId)` | 实现 resume transfer 对应的业务或工具逻辑。 |
| [L1716](../src/apps/controller/FileTransferWindow.cpp#L1716) | `FileTransferWindow::OpenTransferFolder` | 定义 | `void FileTransferWindow::OpenTransferFolder( const std::filesystem::path& path)` | 启动 open transfer folder 相关逻辑。 |

## `src/apps/controller/FileTransferWindow.h`

[打开源码](../src/apps/controller/FileTransferWindow.h) · **文件作用：** 声明 file transfer window 相关类型、接口、配置和成员状态。

### 类型

| 行 | 类型 | 种类 | 作用 |
|---:|---|---|---|
| [L20](../src/apps/controller/FileTransferWindow.h#L20) | `QCloseEvent` | class | 定义 QCloseEvent 的 class 类型和相关状态。 |
| [L21](../src/apps/controller/FileTransferWindow.h#L21) | `QComboBox` | class | 定义 QComboBox 的 class 类型和相关状态。 |
| [L22](../src/apps/controller/FileTransferWindow.h#L22) | `QEvent` | class | 定义 QEvent 的 class 类型和相关状态。 |
| [L23](../src/apps/controller/FileTransferWindow.h#L23) | `QLabel` | class | 定义 QLabel 的 class 类型和相关状态。 |
| [L24](../src/apps/controller/FileTransferWindow.h#L24) | `QResizeEvent` | class | 定义 QResizeEvent 的 class 类型和相关状态。 |
| [L25](../src/apps/controller/FileTransferWindow.h#L25) | `QPushButton` | class | 定义 QPushButton 的 class 类型和相关状态。 |
| [L26](../src/apps/controller/FileTransferWindow.h#L26) | `QScrollArea` | class | 定义 QScrollArea 的 class 类型和相关状态。 |
| [L27](../src/apps/controller/FileTransferWindow.h#L27) | `QTimer` | class | 定义 QTimer 的 class 类型和相关状态。 |
| [L28](../src/apps/controller/FileTransferWindow.h#L28) | `QVBoxLayout` | class | 定义 QVBoxLayout 的 class 类型和相关状态。 |
| [L29](../src/apps/controller/FileTransferWindow.h#L29) | `QWidget` | class | 定义 QWidget 的 class 类型和相关状态。 |
| [L33](../src/apps/controller/FileTransferWindow.h#L33) | `FileTransferCard` | class | 定义 FileTransferCard 的 class 类型和相关状态。 |
| [L35](../src/apps/controller/FileTransferWindow.h#L35) | `FileTransferPeer` | struct | 定义 FileTransferPeer 的 struct 类型和相关状态。 |
| [L40](../src/apps/controller/FileTransferWindow.h#L40) | `FileTransferWindow` | class | 定义 FileTransferWindow 的 class 类型和相关状态。 |

### 成员与文件级变量

| 行 | 变量 | 声明 | 作用 |
|---:|---|---|---|
| [L20](../src/apps/controller/FileTransferWindow.h#L20) | `QCloseEvent` | `class QCloseEvent;` | 保存 q close event 相关配置或运行状态。 |
| [L21](../src/apps/controller/FileTransferWindow.h#L21) | `QComboBox` | `class QComboBox;` | 保存 q combo box 相关配置或运行状态。 |
| [L22](../src/apps/controller/FileTransferWindow.h#L22) | `QEvent` | `class QEvent;` | 保存 q event 相关配置或运行状态。 |
| [L23](../src/apps/controller/FileTransferWindow.h#L23) | `QLabel` | `class QLabel;` | 保存路径、地址或显示名称：q label。 |
| [L24](../src/apps/controller/FileTransferWindow.h#L24) | `QResizeEvent` | `class QResizeEvent;` | 保存 q resize event 相关配置或运行状态。 |
| [L25](../src/apps/controller/FileTransferWindow.h#L25) | `QPushButton` | `class QPushButton;` | 保存 q push button 相关配置或运行状态。 |
| [L26](../src/apps/controller/FileTransferWindow.h#L26) | `QScrollArea` | `class QScrollArea;` | 保存 q scroll area 相关配置或运行状态。 |
| [L27](../src/apps/controller/FileTransferWindow.h#L27) | `QTimer` | `class QTimer;` | 保存定时、截止或超时状态：q timer。 |
| [L28](../src/apps/controller/FileTransferWindow.h#L28) | `QVBoxLayout` | `class QVBoxLayout;` | 保存 qv box layout 相关配置或运行状态。 |
| [L29](../src/apps/controller/FileTransferWindow.h#L29) | `QWidget` | `class QWidget;` | 保存 q widget 相关配置或运行状态。 |
| [L33](../src/apps/controller/FileTransferWindow.h#L33) | `FileTransferCard` | `class FileTransferCard;` | 保存 file transfer card 相关配置或运行状态。 |
| [L36](../src/apps/controller/FileTransferWindow.h#L36) | `deviceId` | `std::string deviceId;` | 保存身份或作用域标识：device id。 |
| [L37](../src/apps/controller/FileTransferWindow.h#L37) | `displayName` | `QString displayName;` | 保存路径、地址或显示名称：display name。 |
| [L87](../src/apps/controller/FileTransferWindow.h#L87) | `controller_` | `app::FileTransferController* controller_ = nullptr;` | 保存 controller 相关配置或运行状态。 |
| [L88](../src/apps/controller/FileTransferWindow.h#L88) | `peerSelector_` | `QComboBox* peerSelector_ = nullptr;` | 保存 peer selector 相关配置或运行状态。 |
| [L89](../src/apps/controller/FileTransferWindow.h#L89) | `titleBar_` | `CustomTitleBar* titleBar_ = nullptr;` | 保存 title bar 相关配置或运行状态。 |
| [L90](../src/apps/controller/FileTransferWindow.h#L90) | `sendButton_` | `QPushButton* sendButton_ = nullptr;` | 保存 send button 相关配置或运行状态。 |
| [L91](../src/apps/controller/FileTransferWindow.h#L91) | `peerHint_` | `QLabel* peerHint_ = nullptr;` | 保存 peer hint 相关配置或运行状态。 |
| [L92](../src/apps/controller/FileTransferWindow.h#L92) | `emptyState_` | `QLabel* emptyState_ = nullptr;` | 保存状态机当前状态：empty state。 |
| [L93](../src/apps/controller/FileTransferWindow.h#L93) | `transferContainer_` | `QWidget* transferContainer_ = nullptr;` | 保存 transfer container 相关配置或运行状态。 |
| [L94](../src/apps/controller/FileTransferWindow.h#L94) | `transferScroll_` | `QScrollArea* transferScroll_ = nullptr;` | 保存 transfer scroll 相关配置或运行状态。 |
| [L95](../src/apps/controller/FileTransferWindow.h#L95) | `transferLayout_` | `QVBoxLayout* transferLayout_ = nullptr;` | 保存 transfer layout 相关配置或运行状态。 |
| [L96](../src/apps/controller/FileTransferWindow.h#L96) | `transferCards_` | `QHash<QString, FileTransferCard*> transferCards_;` | 保存 transfer cards 相关配置或运行状态。 |
| [L97](../src/apps/controller/FileTransferWindow.h#L97) | `transfers_` | `std::vector<app::FileTransferSnapshot> transfers_;` | 保存 transfers 相关配置或运行状态。 |
| [L98](../src/apps/controller/FileTransferWindow.h#L98) | `pendingTransfersMutex_` | `std::mutex pendingTransfersMutex_;` | 保护跨线程共享状态：pending transfers mutex。 |
| [L99](../src/apps/controller/FileTransferWindow.h#L99) | `pendingTransfers_` | `std::vector<app::FileTransferSnapshot> pendingTransfers_;` | 保存 pending transfers 相关配置或运行状态。 |
| [L100](../src/apps/controller/FileTransferWindow.h#L100) | `transferApplyScheduled_` | `bool transferApplyScheduled_ = false;` | 保存 transfer apply scheduled 相关配置或运行状态。 |
| [L101](../src/apps/controller/FileTransferWindow.h#L101) | `anchorGeometry_` | `QRect anchorGeometry_;` | 保存 anchor geometry 相关配置或运行状态。 |
| [L102](../src/apps/controller/FileTransferWindow.h#L102) | `drawerHost_` | `QPointer<QWidget> drawerHost_;` | 保存 drawer host 相关配置或运行状态。 |
| [L103](../src/apps/controller/FileTransferWindow.h#L103) | `drawerMotionTimer_` | `QTimer* drawerMotionTimer_ = nullptr;` | 保存定时、截止或超时状态：drawer motion timer。 |
| [L104](../src/apps/controller/FileTransferWindow.h#L104) | `drawerMotionClock_` | `QElapsedTimer drawerMotionClock_;` | 保护跨线程共享状态：drawer motion clock。 |
| [L105](../src/apps/controller/FileTransferWindow.h#L105) | `drawerMotionStart_` | `QPoint drawerMotionStart_;` | 保存 drawer motion start 相关配置或运行状态。 |
| [L106](../src/apps/controller/FileTransferWindow.h#L106) | `drawerMotionTarget_` | `QPoint drawerMotionTarget_;` | 保存 drawer motion target 相关配置或运行状态。 |
| [L107](../src/apps/controller/FileTransferWindow.h#L107) | `drawerMotionDurationMs_` | `int drawerMotionDurationMs_ = 0;` | 保存 drawer motion duration ms 相关配置或运行状态。 |
| [L108](../src/apps/controller/FileTransferWindow.h#L108) | `drawerHiding_` | `bool drawerHiding_ = false;` | 保存 drawer hiding 相关配置或运行状态。 |
| [L109](../src/apps/controller/FileTransferWindow.h#L109) | `drawerResizeHandle_` | `QWidget* drawerResizeHandle_ = nullptr;` | 保存 drawer resize handle 相关配置或运行状态。 |
| [L110](../src/apps/controller/FileTransferWindow.h#L110) | `drawerWidth_` | `int drawerWidth_ = 390;` | 保存计数、尺寸或速率指标：drawer width。 |
| [L111](../src/apps/controller/FileTransferWindow.h#L111) | `drawerResizeDragging_` | `bool drawerResizeDragging_ = false;` | 保存 drawer resize dragging 相关配置或运行状态。 |
| [L112](../src/apps/controller/FileTransferWindow.h#L112) | `drawerResizeStartGlobalX_` | `int drawerResizeStartGlobalX_ = 0;` | 保存 drawer resize start global x 相关配置或运行状态。 |
| [L113](../src/apps/controller/FileTransferWindow.h#L113) | `drawerResizeStartWidth_` | `int drawerResizeStartWidth_ = 0;` | 保存计数、尺寸或速率指标：drawer resize start width。 |
| [L114](../src/apps/controller/FileTransferWindow.h#L114) | `announcedIncomingTransfers_` | `QSet<QString> announcedIncomingTransfers_;` | 保存 announced incoming transfers 相关配置或运行状态。 |

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L44](../src/apps/controller/FileTransferWindow.h#L44) | `FileTransferWindow` | 声明 | `explicit FileTransferWindow(app::FileTransferController* controller, QWidget* parent = nullptr)` | 实现 file transfer window 对应的业务或工具逻辑。 |
| [L46](../src/apps/controller/FileTransferWindow.h#L46) | `~FileTransferWindow` | 声明 | `~FileTransferWindow() override` | 停止相关活动并释放 FileTransferWindow 实例拥有的资源。 |
| [L48](../src/apps/controller/FileTransferWindow.h#L48) | `SyncPeers` | 声明 | `void SyncPeers(const std::vector<FileTransferPeer>& peers, const QRect& anchorGeometry)` | 实现 sync peers 对应的业务或工具逻辑。 |
| [L50](../src/apps/controller/FileTransferWindow.h#L50) | `AttachAsDrawer` | 声明 | `void AttachAsDrawer(QWidget* host)` | 实现 attach as drawer 对应的业务或工具逻辑。 |
| [L51](../src/apps/controller/FileTransferWindow.h#L51) | `OpenBesideMainWindow` | 声明 | `void OpenBesideMainWindow(const QRect& mainWindowGeometry)` | 启动 open beside main window 相关逻辑。 |
| [L52](../src/apps/controller/FileTransferWindow.h#L52) | `HideWithAnimation` | 声明 | `void HideWithAnimation()` | 实现 hide with animation 对应的业务或工具逻辑。 |
| [L53](../src/apps/controller/FileTransferWindow.h#L53) | `HideImmediately` | 声明 | `void HideImmediately()` | 实现 hide immediately 对应的业务或工具逻辑。 |
| [L54](../src/apps/controller/FileTransferWindow.h#L54) | `UpdateDrawerGeometry` | 声明 | `void UpdateDrawerGeometry()` | 更新或应用 update drawer geometry 相关逻辑。 |
| [L55](../src/apps/controller/FileTransferWindow.h#L55) | `IsHiding` | 声明 | `bool IsHiding() const` | 判断 is hiding 相关逻辑。 |
| [L56](../src/apps/controller/FileTransferWindow.h#L56) | `DetachController` | 声明 | `void DetachController()` | 实现 detach controller 对应的业务或工具逻辑。 |
| [L57](../src/apps/controller/FileTransferWindow.h#L57) | `RefreshThemeStyle` | 声明 | `void RefreshThemeStyle()` | 刷新 refresh theme style 相关逻辑。 |
| [L60](../src/apps/controller/FileTransferWindow.h#L60) | `closeEvent` | 声明 | `void closeEvent(QCloseEvent* event) override` | 关闭并清理 close event 相关逻辑。 |
| [L61](../src/apps/controller/FileTransferWindow.h#L61) | `resizeEvent` | 声明 | `void resizeEvent(QResizeEvent* event) override` | 实现 resize event 对应的业务或工具逻辑。 |
| [L62](../src/apps/controller/FileTransferWindow.h#L62) | `eventFilter` | 声明 | `bool eventFilter(QObject* watched, QEvent* event) override` | 实现 event filter 对应的业务或工具逻辑。 |
| [L65](../src/apps/controller/FileTransferWindow.h#L65) | `OnFileTransfersChanged` | 声明 | `void OnFileTransfersChanged( const std::vector<app::FileTransferSnapshot>& transfers) override` | 接收并处理 on file transfers changed 相关逻辑。 |
| [L67](../src/apps/controller/FileTransferWindow.h#L67) | `BuildUi` | 声明 | `void BuildUi()` | 创建或初始化 build ui 相关逻辑。 |
| [L68](../src/apps/controller/FileTransferWindow.h#L68) | `ApplyTransfers` | 声明 | `void ApplyTransfers( std::vector<app::FileTransferSnapshot> transfers)` | 更新或应用 apply transfers 相关逻辑。 |
| [L70](../src/apps/controller/FileTransferWindow.h#L70) | `ScheduleTransferApply` | 声明 | `void ScheduleTransferApply()` | 执行后台循环或调度 schedule transfer apply 相关逻辑。 |
| [L71](../src/apps/controller/FileTransferWindow.h#L71) | `RebuildTransferCards` | 声明 | `void RebuildTransferCards()` | 更新或应用 rebuild transfer cards 相关逻辑。 |
| [L72](../src/apps/controller/FileTransferWindow.h#L72) | `UpdateTransferCardsIncrementally` | 声明 | `void UpdateTransferCardsIncrementally()` | 更新或应用 update transfer cards incrementally 相关逻辑。 |
| [L73](../src/apps/controller/FileTransferWindow.h#L73) | `SetDrawerWidth` | 声明 | `void SetDrawerWidth(int requestedWidth)` | 更新或应用 set drawer width 相关逻辑。 |
| [L74](../src/apps/controller/FileTransferWindow.h#L74) | `EffectiveDrawerWidth` | 声明 | `int EffectiveDrawerWidth() const` | 实现 effective drawer width 对应的业务或工具逻辑。 |
| [L75](../src/apps/controller/FileTransferWindow.h#L75) | `StartDrawerMotion` | 声明 | `void StartDrawerMotion(const QPoint& targetPosition, int durationMs)` | 启动 start drawer motion 相关逻辑。 |
| [L76](../src/apps/controller/FileTransferWindow.h#L76) | `StopDrawerAnimation` | 声明 | `void StopDrawerAnimation()` | 停止 stop drawer animation 相关逻辑。 |
| [L77](../src/apps/controller/FileTransferWindow.h#L77) | `ChooseAndSendFile` | 声明 | `void ChooseAndSendFile()` | 实现 choose and send file 对应的业务或工具逻辑。 |
| [L78](../src/apps/controller/FileTransferWindow.h#L78) | `AcceptTransfer` | 声明 | `void AcceptTransfer(const std::string& transferId)` | 处理并回复 accept transfer 相关逻辑。 |
| [L79](../src/apps/controller/FileTransferWindow.h#L79) | `SaveTransferAs` | 声明 | `void SaveTransferAs(const std::string& transferId)` | 保存或写入 save transfer as 相关逻辑。 |
| [L80](../src/apps/controller/FileTransferWindow.h#L80) | `AcceptTransferToDirectory` | 声明 | `void AcceptTransferToDirectory(const std::string& transferId, const QString& directory)` | 处理并回复 accept transfer to directory 相关逻辑。 |
| [L82](../src/apps/controller/FileTransferWindow.h#L82) | `RejectTransfer` | 声明 | `void RejectTransfer(const std::string& transferId)` | 处理并回复 reject transfer 相关逻辑。 |
| [L83](../src/apps/controller/FileTransferWindow.h#L83) | `CancelTransfer` | 声明 | `void CancelTransfer(const std::string& transferId)` | 判断 cancel transfer 相关逻辑。 |
| [L84](../src/apps/controller/FileTransferWindow.h#L84) | `ResumeTransfer` | 声明 | `void ResumeTransfer(const std::string& transferId)` | 实现 resume transfer 对应的业务或工具逻辑。 |
| [L85](../src/apps/controller/FileTransferWindow.h#L85) | `OpenTransferFolder` | 声明 | `void OpenTransferFolder(const std::filesystem::path& path)` | 启动 open transfer folder 相关逻辑。 |

## `src/apps/controller/FramelessWindow.cpp`

[打开源码](../src/apps/controller/FramelessWindow.cpp) · **文件作用：** 实现 frameless window 相关函数与文件级辅助逻辑。

### 类型

| 行 | 类型 | 种类 | 作用 |
|---:|---|---|---|
| [L93](../src/apps/controller/FramelessWindow.cpp#L93) | `ResizeBorderMetrics` | struct | 定义 ResizeBorderMetrics 的 struct 类型和相关状态。 |
| [L158](../src/apps/controller/FramelessWindow.cpp#L158) | `FramelessResizeOverlay` | class | 定义 FramelessResizeOverlay 的 class 类型和相关状态。 |
| [L396](../src/apps/controller/FramelessWindow.cpp#L396) | `SmoothWheelFilter` | class | 定义 SmoothWheelFilter 的 class 类型和相关状态。 |

### 成员与文件级变量

| 行 | 变量 | 声明 | 作用 |
|---:|---|---|---|
| [L248](../src/apps/controller/FramelessWindow.cpp#L248) | `kGrip` | `static constexpr int kGrip = 8;` | 定义 grip 的编译期常量或产品边界。 |

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L43](../src/apps/controller/FramelessWindow.cpp#L43) | `MakeCaptionButton` | 定义 | `QToolButton* MakeCaptionButton(const QString& iconResource, const QString& objectName, QWidget* parent)` | 创建或初始化 make caption button 相关逻辑。 |
| [L59](../src/apps/controller/FramelessWindow.cpp#L59) | `SetNativeRoundedCorners` | 定义 | `void SetNativeRoundedCorners(HWND window, bool enabled)` | 更新或应用 set native rounded corners 相关逻辑。 |
| [L71](../src/apps/controller/FramelessWindow.cpp#L71) | `EnableNativeWindowTransitions` | 定义 | `void EnableNativeWindowTransitions(HWND window)` | 实现 enable native window transitions 对应的业务或工具逻辑。 |
| [L98](../src/apps/controller/FramelessWindow.cpp#L98) | `ResizeBordersForWindow` | 定义 | `ResizeBorderMetrics ResizeBordersForWindow(HWND window)` | 实现 resize borders for window 对应的业务或工具逻辑。 |
| [L113](../src/apps/controller/FramelessWindow.cpp#L113) | `ResizeEdgesForHitTest` | 定义 | `Qt::Edges ResizeEdgesForHitTest(LRESULT hitTest)` | 实现 resize edges for hit test 对应的业务或工具逻辑。 |
| [L137](../src/apps/controller/FramelessWindow.cpp#L137) | `ResizeCursorForHitTest` | 定义 | `HCURSOR ResizeCursorForHitTest(LRESULT hitTest)` | 实现 resize cursor for hit test 对应的业务或工具逻辑。 |
| [L160](../src/apps/controller/FramelessWindow.cpp#L160) | `FramelessResizeOverlay` | 定义 | `explicit FramelessResizeOverlay(FramelessMainWindow* owner) : QWidget(owner, Qt::Tool \| Qt::FramelessWindowHint \| Qt::NoDropShadowWindowHint \| Qt::WindowDoesNotAcceptFocus), owner_(owner)` | 实现 frameless resize overlay 对应的业务或工具逻辑。 |
| [L178](../src/apps/controller/FramelessWindow.cpp#L178) | `eventFilter` | 定义 | `bool eventFilter(QObject* watched, QEvent* event) override` | 实现 event filter 对应的业务或工具逻辑。 |
| [L201](../src/apps/controller/FramelessWindow.cpp#L201) | `enterEvent` | 定义 | `void enterEvent(QEnterEvent* event) override` | 实现 enter event 对应的业务或工具逻辑。 |
| [L207](../src/apps/controller/FramelessWindow.cpp#L207) | `mouseMoveEvent` | 定义 | `void mouseMoveEvent(QMouseEvent* event) override` | 实现 mouse move event 对应的业务或工具逻辑。 |
| [L213](../src/apps/controller/FramelessWindow.cpp#L213) | `mousePressEvent` | 定义 | `void mousePressEvent(QMouseEvent* event) override` | 实现 mouse press event 对应的业务或工具逻辑。 |
| [L238](../src/apps/controller/FramelessWindow.cpp#L238) | `paintEvent` | 定义 | `void paintEvent(QPaintEvent*) override` | 准备或呈现 paint event 相关逻辑。 |
| [L251](../src/apps/controller/FramelessWindow.cpp#L251) | `NativeSizingEdge` | 定义 | `static WPARAM NativeSizingEdge(Qt::Edges edges)` | 实现 native sizing edge 对应的业务或工具逻辑。 |
| [L281](../src/apps/controller/FramelessWindow.cpp#L281) | `EdgesAt` | 定义 | `Qt::Edges EdgesAt(const QPoint& position) const` | 实现 edges at 对应的业务或工具逻辑。 |
| [L299](../src/apps/controller/FramelessWindow.cpp#L299) | `UpdateCursorForPosition` | 定义 | `void UpdateCursorForPosition(const QPoint& position)` | 更新或应用 update cursor for position 相关逻辑。 |
| [L324](../src/apps/controller/FramelessWindow.cpp#L324) | `SyncToOwner` | 定义 | `void SyncToOwner()` | 实现 sync to owner 对应的业务或工具逻辑。 |
| [L347](../src/apps/controller/FramelessWindow.cpp#L347) | `CreateRemoteCIcon` | 定义 | `QIcon CreateRemoteCIcon()` | 创建或初始化 create remote c icon 相关逻辑。 |
| [L356](../src/apps/controller/FramelessWindow.cpp#L356) | `ScaleUiStyleSheet` | 定义 | `QString ScaleUiStyleSheet(const QString& styleSheet)` | 转换或缩放 scale ui style sheet 相关逻辑。 |
| [L385](../src/apps/controller/FramelessWindow.cpp#L385) | `CurrentUiAnimationLevel` | 定义 | `int CurrentUiAnimationLevel()` | 实现 current ui animation level 对应的业务或工具逻辑。 |
| [L398](../src/apps/controller/FramelessWindow.cpp#L398) | `SmoothWheelFilter` | 定义 | `explicit SmoothWheelFilter(QAbstractScrollArea* area) : QObject(area), area_(area)` | 实现 smooth wheel filter 对应的业务或工具逻辑。 |
| [L408](../src/apps/controller/FramelessWindow.cpp#L408) | `eventFilter` | 定义 | `bool eventFilter(QObject* watched, QEvent* event) override` | 实现 event filter 对应的业务或工具逻辑。 |
| [L463](../src/apps/controller/FramelessWindow.cpp#L463) | `StopInertia` | 定义 | `void StopInertia()` | 停止 stop inertia 相关逻辑。 |
| [L470](../src/apps/controller/FramelessWindow.cpp#L470) | `AdvanceInertia` | 定义 | `void AdvanceInertia()` | 实现 advance inertia 对应的业务或工具逻辑。 |
| [L505](../src/apps/controller/FramelessWindow.cpp#L505) | `EnableSmoothWheelScrolling` | 定义 | `void EnableSmoothWheelScrolling(QAbstractScrollArea* scrollArea)` | 实现 enable smooth wheel scrolling 对应的业务或工具逻辑。 |
| [L516](../src/apps/controller/FramelessWindow.cpp#L516) | `FramelessMainWindow::FramelessMainWindow` | 定义 | `FramelessMainWindow::FramelessMainWindow(QWidget* parent, bool embedded) : QMainWindow(parent)` | 构造并初始化 FramelessMainWindow 实例。 |
| [L536](../src/apps/controller/FramelessWindow.cpp#L536) | `FramelessMainWindow::SetNativeRoundedCornersEnabled` | 定义 | `void FramelessMainWindow::SetNativeRoundedCornersEnabled(bool enabled)` | 更新或应用 set native rounded corners enabled 相关逻辑。 |
| [L546](../src/apps/controller/FramelessWindow.cpp#L546) | `FramelessMainWindow::changeEvent` | 定义 | `void FramelessMainWindow::changeEvent(QEvent* event)` | 实现 change event 对应的业务或工具逻辑。 |
| [L566](../src/apps/controller/FramelessWindow.cpp#L566) | `FramelessMainWindow::ApplyUiStyleSheet` | 定义 | `void FramelessMainWindow::ApplyUiStyleSheet(const QString& styleSheet)` | 更新或应用 apply ui style sheet 相关逻辑。 |
| [L572](../src/apps/controller/FramelessWindow.cpp#L572) | `FramelessMainWindow::RefreshWindowStyle` | 定义 | `void FramelessMainWindow::RefreshWindowStyle()` | 刷新 refresh window style 相关逻辑。 |
| [L579](../src/apps/controller/FramelessWindow.cpp#L579) | `FramelessMainWindow::RefreshAllWindowStyles` | 定义 | `void FramelessMainWindow::RefreshAllWindowStyles()` | 刷新 refresh all window styles 相关逻辑。 |
| [L595](../src/apps/controller/FramelessWindow.cpp#L595) | `FramelessMainWindow::MinimizeWithSystemAnimation` | 定义 | `void FramelessMainWindow::MinimizeWithSystemAnimation()` | 实现 minimize with system animation 对应的业务或工具逻辑。 |
| [L609](../src/apps/controller/FramelessWindow.cpp#L609) | `FramelessMainWindow::AnimateWindowEntrance` | 定义 | `void FramelessMainWindow::AnimateWindowEntrance( const QPoint& fullAnimationOffset, const QRect& globalClipRect, int durationOverrideMs)` | 实现 animate window entrance 对应的业务或工具逻辑。 |
| [L680](../src/apps/controller/FramelessWindow.cpp#L680) | `FramelessMainWindow::AnimateWindowExit` | 定义 | `void FramelessMainWindow::AnimateWindowExit( const QPoint& fullAnimationOffset, const QRect& globalClipRect, int durationOverrideMs)` | 实现 animate window exit 对应的业务或工具逻辑。 |
| [L751](../src/apps/controller/FramelessWindow.cpp#L751) | `FramelessMainWindow::nativeEvent` | 定义 | `bool FramelessMainWindow::nativeEvent(const QByteArray& eventType, void* message, qintptr* result)` | 实现 native event 对应的业务或工具逻辑。 |
| [L953](../src/apps/controller/FramelessWindow.cpp#L953) | `FramelessMainWindow::ConstrainResizeGeometry` | 定义 | `QRect FramelessMainWindow::ConstrainResizeGeometry( const QRect& proposedGeometry, Qt::Edges resizeEdges, qreal devicePixelRatio) const` | 实现 constrain resize geometry 对应的业务或工具逻辑。 |
| [L963](../src/apps/controller/FramelessWindow.cpp#L963) | `CustomTitleBar::CustomTitleBar` | 定义 | `CustomTitleBar::CustomTitleBar(FramelessMainWindow* window, QString title, QWidget* parent) : QWidget(parent), window_(window)` | 构造并初始化 CustomTitleBar 实例。 |
| [L1044](../src/apps/controller/FramelessWindow.cpp#L1044) | `CustomTitleBar::SetMinimizeAction` | 定义 | `void CustomTitleBar::SetMinimizeAction(std::function<void()> action)` | 更新或应用 set minimize action 相关逻辑。 |
| [L1049](../src/apps/controller/FramelessWindow.cpp#L1049) | `CustomTitleBar::SetEmbeddedMode` | 定义 | `void CustomTitleBar::SetEmbeddedMode(bool embedded)` | 更新或应用 set embedded mode 相关逻辑。 |
| [L1056](../src/apps/controller/FramelessWindow.cpp#L1056) | `CustomTitleBar::SetTitle` | 定义 | `void CustomTitleBar::SetTitle(const QString& title)` | 更新或应用 set title 相关逻辑。 |
| [L1061](../src/apps/controller/FramelessWindow.cpp#L1061) | `CustomTitleBar::RefreshThemeStyle` | 定义 | `void CustomTitleBar::RefreshThemeStyle(bool dark)` | 刷新 refresh theme style 相关逻辑。 |
| [L1087](../src/apps/controller/FramelessWindow.cpp#L1087) | `CustomTitleBar::eventFilter` | 定义 | `bool CustomTitleBar::eventFilter(QObject* watched, QEvent* event)` | 实现 event filter 对应的业务或工具逻辑。 |
| [L1095](../src/apps/controller/FramelessWindow.cpp#L1095) | `CustomTitleBar::mouseDoubleClickEvent` | 定义 | `void CustomTitleBar::mouseDoubleClickEvent(QMouseEvent* event)` | 实现 mouse double click event 对应的业务或工具逻辑。 |
| [L1109](../src/apps/controller/FramelessWindow.cpp#L1109) | `CustomTitleBar::mousePressEvent` | 定义 | `void CustomTitleBar::mousePressEvent(QMouseEvent* event)` | 实现 mouse press event 对应的业务或工具逻辑。 |
| [L1123](../src/apps/controller/FramelessWindow.cpp#L1123) | `CustomTitleBar::ToggleMaximized` | 定义 | `void CustomTitleBar::ToggleMaximized()` | 实现 toggle maximized 对应的业务或工具逻辑。 |
| [L1143](../src/apps/controller/FramelessWindow.cpp#L1143) | `CustomTitleBar::UpdateMaximizeButton` | 定义 | `void CustomTitleBar::UpdateMaximizeButton()` | 更新或应用 update maximize button 相关逻辑。 |

## `src/apps/controller/FramelessWindow.h`

[打开源码](../src/apps/controller/FramelessWindow.h) · **文件作用：** 声明 frameless window 相关类型、接口、配置和成员状态。

### 类型

| 行 | 类型 | 种类 | 作用 |
|---:|---|---|---|
| [L14](../src/apps/controller/FramelessWindow.h#L14) | `QLabel` | class | 定义 QLabel 的 class 类型和相关状态。 |
| [L15](../src/apps/controller/FramelessWindow.h#L15) | `QAbstractScrollArea` | class | 定义 QAbstractScrollArea 的 class 类型和相关状态。 |
| [L16](../src/apps/controller/FramelessWindow.h#L16) | `QToolButton` | class | 定义 QToolButton 的 class 类型和相关状态。 |
| [L25](../src/apps/controller/FramelessWindow.h#L25) | `FramelessMainWindow` | class | 定义 FramelessMainWindow 的 class 类型和相关状态。 |
| [L64](../src/apps/controller/FramelessWindow.h#L64) | `CustomTitleBar` | class | 定义 CustomTitleBar 的 class 类型和相关状态。 |

### 成员与文件级变量

| 行 | 变量 | 声明 | 作用 |
|---:|---|---|---|
| [L14](../src/apps/controller/FramelessWindow.h#L14) | `QLabel` | `class QLabel;` | 保存路径、地址或显示名称：q label。 |
| [L15](../src/apps/controller/FramelessWindow.h#L15) | `QAbstractScrollArea` | `class QAbstractScrollArea;` | 保存 q abstract scroll area 相关配置或运行状态。 |
| [L16](../src/apps/controller/FramelessWindow.h#L16) | `QToolButton` | `class QToolButton;` | 保存 q tool button 相关配置或运行状态。 |
| [L55](../src/apps/controller/FramelessWindow.h#L55) | `baseStyleSheet_` | `QString baseStyleSheet_;` | 保存 base style sheet 相关配置或运行状态。 |
| [L56](../src/apps/controller/FramelessWindow.h#L56) | `resizeOverlay_` | `QWidget* resizeOverlay_ = nullptr;` | 保存 resize overlay 相关配置或运行状态。 |
| [L58](../src/apps/controller/FramelessWindow.h#L58) | `resizeEdges_` | `Qt::Edges resizeEdges_{};` | 保存 resize edges 相关配置或运行状态。 |
| [L59](../src/apps/controller/FramelessWindow.h#L59) | `resizeStartCursor_` | `QPoint resizeStartCursor_;` | 保存 resize start cursor 相关配置或运行状态。 |
| [L60](../src/apps/controller/FramelessWindow.h#L60) | `resizeStartWindowRect_` | `QRect resizeStartWindowRect_;` | 保存 resize start window rect 相关配置或运行状态。 |
| [L84](../src/apps/controller/FramelessWindow.h#L84) | `window_` | `FramelessMainWindow* window_ = nullptr;` | 保存 window 相关配置或运行状态。 |
| [L85](../src/apps/controller/FramelessWindow.h#L85) | `titleLabel_` | `QLabel* titleLabel_ = nullptr;` | 保存路径、地址或显示名称：title label。 |
| [L87](../src/apps/controller/FramelessWindow.h#L87) | `minimizeButton_` | `QToolButton* minimizeButton_ = nullptr;` | 保存 minimize button 相关配置或运行状态。 |
| [L88](../src/apps/controller/FramelessWindow.h#L88) | `maximizeButton_` | `QToolButton* maximizeButton_ = nullptr;` | 保存 maximize button 相关配置或运行状态。 |
| [L89](../src/apps/controller/FramelessWindow.h#L89) | `closeButton_` | `QToolButton* closeButton_ = nullptr;` | 保存 close button 相关配置或运行状态。 |
| [L90](../src/apps/controller/FramelessWindow.h#L90) | `embedded_` | `bool embedded_ = false;` | 保存 embedded 相关配置或运行状态。 |

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L20](../src/apps/controller/FramelessWindow.h#L20) | `CreateRemoteCIcon` | 声明 | `QIcon CreateRemoteCIcon()` | 创建或初始化 create remote c icon 相关逻辑。 |
| [L21](../src/apps/controller/FramelessWindow.h#L21) | `ScaleUiStyleSheet` | 声明 | `QString ScaleUiStyleSheet(const QString& styleSheet)` | 转换或缩放 scale ui style sheet 相关逻辑。 |
| [L22](../src/apps/controller/FramelessWindow.h#L22) | `CurrentUiAnimationLevel` | 声明 | `int CurrentUiAnimationLevel()` | 实现 current ui animation level 对应的业务或工具逻辑。 |
| [L23](../src/apps/controller/FramelessWindow.h#L23) | `EnableSmoothWheelScrolling` | 声明 | `void EnableSmoothWheelScrolling(QAbstractScrollArea* scrollArea)` | 实现 enable smooth wheel scrolling 对应的业务或工具逻辑。 |
| [L27](../src/apps/controller/FramelessWindow.h#L27) | `FramelessMainWindow` | 声明 | `explicit FramelessMainWindow(QWidget* parent = nullptr, bool embedded = false)` | 实现 frameless main window 对应的业务或工具逻辑。 |
| [L29](../src/apps/controller/FramelessWindow.h#L29) | `~FramelessMainWindow` | 声明 | `~FramelessMainWindow() override = default` | 停止相关活动并释放 FramelessMainWindow 实例拥有的资源。 |
| [L31](../src/apps/controller/FramelessWindow.h#L31) | `RefreshAllWindowStyles` | 声明 | `static void RefreshAllWindowStyles()` | 刷新 refresh all window styles 相关逻辑。 |
| [L32](../src/apps/controller/FramelessWindow.h#L32) | `MinimizeWithSystemAnimation` | 声明 | `void MinimizeWithSystemAnimation()` | 实现 minimize with system animation 对应的业务或工具逻辑。 |
| [L33](../src/apps/controller/FramelessWindow.h#L33) | `AnimateWindowEntrance` | 声明 | `void AnimateWindowEntrance(const QPoint& fullAnimationOffset = QPoint(10, 0), const QRect& globalClipRect = QRect(), int durationOverrideMs = 0)` | 实现 animate window entrance 对应的业务或工具逻辑。 |
| [L34](../src/apps/controller/FramelessWindow.h#L34) | `QPoint` | 声明 | `QPoint(10, 0), const QRect& globalClipRect = QRect(), int durationOverrideMs = 0)` | 实现 q point 对应的业务或工具逻辑。 |
| [L37](../src/apps/controller/FramelessWindow.h#L37) | `AnimateWindowExit` | 声明 | `void AnimateWindowExit(const QPoint& fullAnimationOffset = QPoint(), const QRect& globalClipRect = QRect(), int durationOverrideMs = 0)` | 实现 animate window exit 对应的业务或工具逻辑。 |
| [L42](../src/apps/controller/FramelessWindow.h#L42) | `ApplyUiStyleSheet` | 声明 | `void ApplyUiStyleSheet(const QString& styleSheet)` | 更新或应用 apply ui style sheet 相关逻辑。 |
| [L43](../src/apps/controller/FramelessWindow.h#L43) | `SetNativeRoundedCornersEnabled` | 声明 | `void SetNativeRoundedCornersEnabled(bool enabled)` | 更新或应用 set native rounded corners enabled 相关逻辑。 |
| [L44](../src/apps/controller/FramelessWindow.h#L44) | `changeEvent` | 声明 | `void changeEvent(QEvent* event) override` | 实现 change event 对应的业务或工具逻辑。 |
| [L45](../src/apps/controller/FramelessWindow.h#L45) | `nativeEvent` | 声明 | `bool nativeEvent(const QByteArray& eventType, void* message, qintptr* result) override` | 实现 native event 对应的业务或工具逻辑。 |
| [L48](../src/apps/controller/FramelessWindow.h#L48) | `ConstrainResizeGeometry` | 声明 | `virtual QRect ConstrainResizeGeometry(const QRect& proposedGeometry, Qt::Edges resizeEdges, qreal devicePixelRatio) const` | 实现 constrain resize geometry 对应的业务或工具逻辑。 |
| [L53](../src/apps/controller/FramelessWindow.h#L53) | `RefreshWindowStyle` | 声明 | `void RefreshWindowStyle()` | 刷新 refresh window style 相关逻辑。 |
| [L66](../src/apps/controller/FramelessWindow.h#L66) | `CustomTitleBar` | 声明 | `CustomTitleBar(FramelessMainWindow* window, QString title, QWidget* parent = nullptr)` | 实现 custom title bar 对应的业务或工具逻辑。 |
| [L70](../src/apps/controller/FramelessWindow.h#L70) | `SetMinimizeAction` | 声明 | `void SetMinimizeAction(std::function<void()> action)` | 更新或应用 set minimize action 相关逻辑。 |
| [L71](../src/apps/controller/FramelessWindow.h#L71) | `SetEmbeddedMode` | 声明 | `void SetEmbeddedMode(bool embedded)` | 更新或应用 set embedded mode 相关逻辑。 |
| [L72](../src/apps/controller/FramelessWindow.h#L72) | `SetTitle` | 声明 | `void SetTitle(const QString& title)` | 更新或应用 set title 相关逻辑。 |
| [L73](../src/apps/controller/FramelessWindow.h#L73) | `RefreshThemeStyle` | 声明 | `void RefreshThemeStyle(bool dark)` | 刷新 refresh theme style 相关逻辑。 |
| [L76](../src/apps/controller/FramelessWindow.h#L76) | `eventFilter` | 声明 | `bool eventFilter(QObject* watched, QEvent* event) override` | 实现 event filter 对应的业务或工具逻辑。 |
| [L77](../src/apps/controller/FramelessWindow.h#L77) | `mouseDoubleClickEvent` | 声明 | `void mouseDoubleClickEvent(QMouseEvent* event) override` | 实现 mouse double click event 对应的业务或工具逻辑。 |
| [L78](../src/apps/controller/FramelessWindow.h#L78) | `mousePressEvent` | 声明 | `void mousePressEvent(QMouseEvent* event) override` | 实现 mouse press event 对应的业务或工具逻辑。 |
| [L81](../src/apps/controller/FramelessWindow.h#L81) | `ToggleMaximized` | 声明 | `void ToggleMaximized()` | 实现 toggle maximized 对应的业务或工具逻辑。 |
| [L82](../src/apps/controller/FramelessWindow.h#L82) | `UpdateMaximizeButton` | 声明 | `void UpdateMaximizeButton()` | 更新或应用 update maximize button 相关逻辑。 |

## `src/apps/controller/LoginWindow.cpp`

[打开源码](../src/apps/controller/LoginWindow.cpp) · **文件作用：** 实现 login window 相关函数与文件级辅助逻辑。

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L41](../src/apps/controller/LoginWindow.cpp#L41) | `AccountRevealEasing` | 定义 | `const QEasingCurve& AccountRevealEasing()` | 实现 account reveal easing 对应的业务或工具逻辑。 |
| [L52](../src/apps/controller/LoginWindow.cpp#L52) | `AccountDismissEasing` | 定义 | `const QEasingCurve& AccountDismissEasing()` | 实现 account dismiss easing 对应的业务或工具逻辑。 |
| [L65](../src/apps/controller/LoginWindow.cpp#L65) | `LoginStatusWindow::LoginStatusWindow` | 定义 | `LoginStatusWindow::LoginStatusWindow(QWidget* parent) : QDialog(parent)` | 构造并初始化 LoginStatusWindow 实例。 |
| [L140](../src/apps/controller/LoginWindow.cpp#L140) | `LoginStatusWindow::SetAccountLabel` | 定义 | `void LoginStatusWindow::SetAccountLabel(const QString& accountLabel)` | 更新或应用 set account label 相关逻辑。 |
| [L146](../src/apps/controller/LoginWindow.cpp#L146) | `LoginStatusWindow::RefreshThemeStyle` | 定义 | `void LoginStatusWindow::RefreshThemeStyle()` | 刷新 refresh theme style 相关逻辑。 |
| [L166](../src/apps/controller/LoginWindow.cpp#L166) | `LoginStatusWindow::ShowAndActivate` | 定义 | `void LoginStatusWindow::ShowAndActivate()` | 实现 show and activate 对应的业务或工具逻辑。 |
| [L187](../src/apps/controller/LoginWindow.cpp#L187) | `LoginStatusWindow::HideAndReleaseTopmost` | 定义 | `void LoginStatusWindow::HideAndReleaseTopmost()` | 实现 hide and release topmost 对应的业务或工具逻辑。 |
| [L198](../src/apps/controller/LoginWindow.cpp#L198) | `LoginStatusWindow::mousePressEvent` | 定义 | `void LoginStatusWindow::mousePressEvent(QMouseEvent* event)` | 实现 mouse press event 对应的业务或工具逻辑。 |
| [L207](../src/apps/controller/LoginWindow.cpp#L207) | `LoginWindow::LoginWindow` | 定义 | `LoginWindow::LoginWindow(QWidget* parent) : QDialog(parent)` | 构造并初始化 LoginWindow 实例。 |
| [L218](../src/apps/controller/LoginWindow.cpp#L218) | `LoginWindow::SetLoginRequestedCallback` | 定义 | `void LoginWindow::SetLoginRequestedCallback( std::function<void()> callback)` | 更新或应用 set login requested callback 相关逻辑。 |
| [L223](../src/apps/controller/LoginWindow.cpp#L223) | `LoginWindow::SetCancelRequestedCallback` | 定义 | `void LoginWindow::SetCancelRequestedCallback( std::function<void()> callback)` | 更新或应用 set cancel requested callback 相关逻辑。 |
| [L228](../src/apps/controller/LoginWindow.cpp#L228) | `LoginWindow::SetExitRequestedCallback` | 定义 | `void LoginWindow::SetExitRequestedCallback( std::function<void()> callback)` | 更新或应用 set exit requested callback 相关逻辑。 |
| [L233](../src/apps/controller/LoginWindow.cpp#L233) | `LoginWindow::SetSignOutRequestedCallback` | 定义 | `void LoginWindow::SetSignOutRequestedCallback( std::function<void()> callback)` | 更新或应用 set sign out requested callback 相关逻辑。 |
| [L238](../src/apps/controller/LoginWindow.cpp#L238) | `LoginWindow::SetDeleteAccountRequestedCallback` | 定义 | `void LoginWindow::SetDeleteAccountRequestedCallback( std::function<void()> callback)` | 更新或应用 set delete account requested callback 相关逻辑。 |
| [L243](../src/apps/controller/LoginWindow.cpp#L243) | `LoginWindow::SetOwnerWindow` | 定义 | `void LoginWindow::SetOwnerWindow(QWidget* owner)` | 更新或应用 set owner window 相关逻辑。 |
| [L256](../src/apps/controller/LoginWindow.cpp#L256) | `LoginWindow::ShowReady` | 定义 | `void LoginWindow::ShowReady(const QString& message)` | 实现 show ready 对应的业务或工具逻辑。 |
| [L276](../src/apps/controller/LoginWindow.cpp#L276) | `LoginWindow::ShowBusy` | 定义 | `void LoginWindow::ShowBusy( const QString& title, const QString& message)` | 实现 show busy 对应的业务或工具逻辑。 |
| [L294](../src/apps/controller/LoginWindow.cpp#L294) | `LoginWindow::ShowAccountDeletionBusy` | 定义 | `void LoginWindow::ShowAccountDeletionBusy()` | 实现 show account deletion busy 对应的业务或工具逻辑。 |
| [L303](../src/apps/controller/LoginWindow.cpp#L303) | `LoginWindow::ShowError` | 定义 | `void LoginWindow::ShowError( const QString& message, bool retryable)` | 实现 show error 对应的业务或工具逻辑。 |
| [L322](../src/apps/controller/LoginWindow.cpp#L322) | `LoginWindow::ShowAuthenticated` | 定义 | `void LoginWindow::ShowAuthenticated( const QString& accountLabel, const QString& accountDetail)` | 实现 show authenticated 对应的业务或工具逻辑。 |
| [L345](../src/apps/controller/LoginWindow.cpp#L345) | `LoginWindow::ShowAndActivate` | 定义 | `void LoginWindow::ShowAndActivate()` | 实现 show and activate 对应的业务或工具逻辑。 |
| [L380](../src/apps/controller/LoginWindow.cpp#L380) | `LoginWindow::HideWithAnimation` | 定义 | `void LoginWindow::HideWithAnimation()` | 实现 hide with animation 对应的业务或工具逻辑。 |
| [L408](../src/apps/controller/LoginWindow.cpp#L408) | `LoginWindow::IsHiding` | 定义 | `bool LoginWindow::IsHiding() const` | 判断 is hiding 相关逻辑。 |
| [L412](../src/apps/controller/LoginWindow.cpp#L412) | `LoginWindow::CardTargetGeometry` | 定义 | `QRect LoginWindow::CardTargetGeometry() const` | 实现 card target geometry 对应的业务或工具逻辑。 |
| [L436](../src/apps/controller/LoginWindow.cpp#L436) | `LoginWindow::LayoutCardLayer` | 定义 | `void LoginWindow::LayoutCardLayer()` | 实现 layout card layer 对应的业务或工具逻辑。 |
| [L443](../src/apps/controller/LoginWindow.cpp#L443) | `LoginWindow::StartVisibilityMotion` | 定义 | `void LoginWindow::StartVisibilityMotion(const QPoint& targetPosition, int durationMs)` | 启动 start visibility motion 相关逻辑。 |
| [L486](../src/apps/controller/LoginWindow.cpp#L486) | `LoginWindow::StopVisibilityMotion` | 定义 | `void LoginWindow::StopVisibilityMotion()` | 停止 stop visibility motion 相关逻辑。 |
| [L492](../src/apps/controller/LoginWindow.cpp#L492) | `LoginWindow::IsVisibilityMotionActive` | 定义 | `bool LoginWindow::IsVisibilityMotionActive() const` | 判断 is visibility motion active 相关逻辑。 |
| [L497](../src/apps/controller/LoginWindow.cpp#L497) | `LoginWindow::closeEvent` | 定义 | `void LoginWindow::closeEvent(QCloseEvent* event)` | 关闭并清理 close event 相关逻辑。 |
| [L513](../src/apps/controller/LoginWindow.cpp#L513) | `LoginWindow::eventFilter` | 定义 | `bool LoginWindow::eventFilter(QObject* watched, QEvent* event)` | 实现 event filter 对应的业务或工具逻辑。 |
| [L558](../src/apps/controller/LoginWindow.cpp#L558) | `LoginWindow::SetStatePanelTone` | 定义 | `void LoginWindow::SetStatePanelTone(const QString& tone)` | 更新或应用 set state panel tone 相关逻辑。 |
| [L570](../src/apps/controller/LoginWindow.cpp#L570) | `LoginWindow::mousePressEvent` | 定义 | `void LoginWindow::mousePressEvent(QMouseEvent* event)` | 实现 mouse press event 对应的业务或工具逻辑。 |
| [L580](../src/apps/controller/LoginWindow.cpp#L580) | `LoginWindow::BuildUi` | 定义 | `void LoginWindow::BuildUi()` | 创建或初始化 build ui 相关逻辑。 |
| [L874](../src/apps/controller/LoginWindow.cpp#L874) | `LoginWindow::RefreshThemeStyle` | 定义 | `void LoginWindow::RefreshThemeStyle()` | 刷新 refresh theme style 相关逻辑。 |
| [L936](../src/apps/controller/LoginWindow.cpp#L936) | `LoginWindow::UpdateAuthenticatedAccountText` | 定义 | `void LoginWindow::UpdateAuthenticatedAccountText()` | 更新或应用 update authenticated account text 相关逻辑。 |

## `src/apps/controller/LoginWindow.h`

[打开源码](../src/apps/controller/LoginWindow.h) · **文件作用：** 声明 login window 相关类型、接口、配置和成员状态。

### 类型

| 行 | 类型 | 种类 | 作用 |
|---:|---|---|---|
| [L15](../src/apps/controller/LoginWindow.h#L15) | `QCloseEvent` | class | 定义 QCloseEvent 的 class 类型和相关状态。 |
| [L16](../src/apps/controller/LoginWindow.h#L16) | `QLabel` | class | 定义 QLabel 的 class 类型和相关状态。 |
| [L17](../src/apps/controller/LoginWindow.h#L17) | `QMouseEvent` | class | 定义 QMouseEvent 的 class 类型和相关状态。 |
| [L18](../src/apps/controller/LoginWindow.h#L18) | `QProgressBar` | class | 定义 QProgressBar 的 class 类型和相关状态。 |
| [L19](../src/apps/controller/LoginWindow.h#L19) | `QPushButton` | class | 定义 QPushButton 的 class 类型和相关状态。 |
| [L20](../src/apps/controller/LoginWindow.h#L20) | `QFrame` | class | 定义 QFrame 的 class 类型和相关状态。 |
| [L21](../src/apps/controller/LoginWindow.h#L21) | `QTimer` | class | 定义 QTimer 的 class 类型和相关状态。 |
| [L25](../src/apps/controller/LoginWindow.h#L25) | `LoginStatusWindow` | class | 定义 LoginStatusWindow 的 class 类型和相关状态。 |
| [L40](../src/apps/controller/LoginWindow.h#L40) | `LoginWindow` | class | 定义 LoginWindow 的 class 类型和相关状态。 |

### 成员与文件级变量

| 行 | 变量 | 声明 | 作用 |
|---:|---|---|---|
| [L15](../src/apps/controller/LoginWindow.h#L15) | `QCloseEvent` | `class QCloseEvent;` | 保存 q close event 相关配置或运行状态。 |
| [L16](../src/apps/controller/LoginWindow.h#L16) | `QLabel` | `class QLabel;` | 保存路径、地址或显示名称：q label。 |
| [L17](../src/apps/controller/LoginWindow.h#L17) | `QMouseEvent` | `class QMouseEvent;` | 保存 q mouse event 相关配置或运行状态。 |
| [L18](../src/apps/controller/LoginWindow.h#L18) | `QProgressBar` | `class QProgressBar;` | 保存 q progress bar 相关配置或运行状态。 |
| [L19](../src/apps/controller/LoginWindow.h#L19) | `QPushButton` | `class QPushButton;` | 保存 q push button 相关配置或运行状态。 |
| [L20](../src/apps/controller/LoginWindow.h#L20) | `QFrame` | `class QFrame;` | 保存媒体帧、图像或缓冲资源：q frame。 |
| [L21](../src/apps/controller/LoginWindow.h#L21) | `QTimer` | `class QTimer;` | 保存定时、截止或超时状态：q timer。 |
| [L37](../src/apps/controller/LoginWindow.h#L37) | `accountLabel_` | `QLabel* accountLabel_ = nullptr;` | 保存路径、地址或显示名称：account label。 |
| [L77](../src/apps/controller/LoginWindow.h#L77) | `titleLabel_` | `QLabel* titleLabel_ = nullptr;` | 保存路径、地址或显示名称：title label。 |
| [L78](../src/apps/controller/LoginWindow.h#L78) | `stateIcon_` | `QLabel* stateIcon_ = nullptr;` | 保存 state icon 相关配置或运行状态。 |
| [L79](../src/apps/controller/LoginWindow.h#L79) | `statusLabel_` | `QLabel* statusLabel_ = nullptr;` | 保存路径、地址或显示名称：status label。 |
| [L80](../src/apps/controller/LoginWindow.h#L80) | `securityLabel_` | `QLabel* securityLabel_ = nullptr;` | 保存路径、地址或显示名称：security label。 |
| [L81](../src/apps/controller/LoginWindow.h#L81) | `progressBar_` | `QProgressBar* progressBar_ = nullptr;` | 保存 progress bar 相关配置或运行状态。 |
| [L82](../src/apps/controller/LoginWindow.h#L82) | `loginButton_` | `QPushButton* loginButton_ = nullptr;` | 保存 login button 相关配置或运行状态。 |
| [L83](../src/apps/controller/LoginWindow.h#L83) | `cancelButton_` | `QPushButton* cancelButton_ = nullptr;` | 保存 cancel button 相关配置或运行状态。 |
| [L84](../src/apps/controller/LoginWindow.h#L84) | `exitButton_` | `QPushButton* exitButton_ = nullptr;` | 保存 exit button 相关配置或运行状态。 |
| [L85](../src/apps/controller/LoginWindow.h#L85) | `signOutButton_` | `QPushButton* signOutButton_ = nullptr;` | 保存 sign out button 相关配置或运行状态。 |
| [L86](../src/apps/controller/LoginWindow.h#L86) | `deleteAccountButton_` | `QPushButton* deleteAccountButton_ = nullptr;` | 保存 delete account button 相关配置或运行状态。 |
| [L87](../src/apps/controller/LoginWindow.h#L87) | `statePanel_` | `QFrame* statePanel_ = nullptr;` | 保存 state panel 相关配置或运行状态。 |
| [L88](../src/apps/controller/LoginWindow.h#L88) | `cardLayer_` | `QWidget* cardLayer_ = nullptr;` | 保存 card layer 相关配置或运行状态。 |
| [L89](../src/apps/controller/LoginWindow.h#L89) | `card_` | `QFrame* card_ = nullptr;` | 保存 card 相关配置或运行状态。 |
| [L90](../src/apps/controller/LoginWindow.h#L90) | `dragHandle_` | `QWidget* dragHandle_ = nullptr;` | 保存 drag handle 相关配置或运行状态。 |
| [L91](../src/apps/controller/LoginWindow.h#L91) | `visibilityMotionTimer_` | `QTimer* visibilityMotionTimer_ = nullptr;` | 保存定时、截止或超时状态：visibility motion timer。 |
| [L92](../src/apps/controller/LoginWindow.h#L92) | `visibilityMotionClock_` | `QElapsedTimer visibilityMotionClock_;` | 保护跨线程共享状态：visibility motion clock。 |
| [L93](../src/apps/controller/LoginWindow.h#L93) | `visibilityMotionStart_` | `QPoint visibilityMotionStart_;` | 保存 visibility motion start 相关配置或运行状态。 |
| [L94](../src/apps/controller/LoginWindow.h#L94) | `visibilityMotionTarget_` | `QPoint visibilityMotionTarget_;` | 保存 visibility motion target 相关配置或运行状态。 |
| [L95](../src/apps/controller/LoginWindow.h#L95) | `visibilityMotionDurationMs_` | `int visibilityMotionDurationMs_ = 0;` | 保存 visibility motion duration ms 相关配置或运行状态。 |
| [L96](../src/apps/controller/LoginWindow.h#L96) | `ownerWindow_` | `QPointer<QWidget> ownerWindow_;` | 保存 owner window 相关配置或运行状态。 |
| [L97](../src/apps/controller/LoginWindow.h#L97) | `preferredCardSize_` | `QSize preferredCardSize_{620, 520};` | 保存计数、尺寸或速率指标：preferred card size。 |
| [L98](../src/apps/controller/LoginWindow.h#L98) | `dragOffset_` | `QPoint dragOffset_;` | 保存 drag offset 相关配置或运行状态。 |
| [L99](../src/apps/controller/LoginWindow.h#L99) | `lastCardPosition_` | `QPoint lastCardPosition_;` | 保存 last card position 相关配置或运行状态。 |
| [L100](../src/apps/controller/LoginWindow.h#L100) | `dragging_` | `bool dragging_ = false;` | 保存 dragging 相关配置或运行状态。 |
| [L101](../src/apps/controller/LoginWindow.h#L101) | `hasLastCardPosition_` | `bool hasLastCardPosition_ = false;` | 保存 has last card position 相关配置或运行状态。 |
| [L102](../src/apps/controller/LoginWindow.h#L102) | `hiding_` | `bool hiding_ = false;` | 保存 hiding 相关配置或运行状态。 |
| [L103](../src/apps/controller/LoginWindow.h#L103) | `authenticatedAccountLabel_` | `QString authenticatedAccountLabel_;` | 保存路径、地址或显示名称：authenticated account label。 |
| [L104](../src/apps/controller/LoginWindow.h#L104) | `authenticatedAccountDetail_` | `QString authenticatedAccountDetail_;` | 保存 authenticated account detail 相关配置或运行状态。 |

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L27](../src/apps/controller/LoginWindow.h#L27) | `LoginStatusWindow` | 声明 | `explicit LoginStatusWindow(QWidget* parent = nullptr)` | 实现 login status window 对应的业务或工具逻辑。 |
| [L28](../src/apps/controller/LoginWindow.h#L28) | `SetAccountLabel` | 声明 | `void SetAccountLabel(const QString& accountLabel)` | 更新或应用 set account label 相关逻辑。 |
| [L29](../src/apps/controller/LoginWindow.h#L29) | `ShowAndActivate` | 声明 | `void ShowAndActivate()` | 实现 show and activate 对应的业务或工具逻辑。 |
| [L30](../src/apps/controller/LoginWindow.h#L30) | `HideAndReleaseTopmost` | 声明 | `void HideAndReleaseTopmost()` | 实现 hide and release topmost 对应的业务或工具逻辑。 |
| [L31](../src/apps/controller/LoginWindow.h#L31) | `RefreshThemeStyle` | 声明 | `void RefreshThemeStyle()` | 刷新 refresh theme style 相关逻辑。 |
| [L34](../src/apps/controller/LoginWindow.h#L34) | `mousePressEvent` | 声明 | `void mousePressEvent(QMouseEvent* event) override` | 实现 mouse press event 对应的业务或工具逻辑。 |
| [L42](../src/apps/controller/LoginWindow.h#L42) | `LoginWindow` | 声明 | `explicit LoginWindow(QWidget* parent = nullptr)` | 实现 login window 对应的业务或工具逻辑。 |
| [L44](../src/apps/controller/LoginWindow.h#L44) | `SetLoginRequestedCallback` | 声明 | `void SetLoginRequestedCallback(std::function<void()> callback)` | 更新或应用 set login requested callback 相关逻辑。 |
| [L45](../src/apps/controller/LoginWindow.h#L45) | `SetCancelRequestedCallback` | 声明 | `void SetCancelRequestedCallback(std::function<void()> callback)` | 更新或应用 set cancel requested callback 相关逻辑。 |
| [L46](../src/apps/controller/LoginWindow.h#L46) | `SetExitRequestedCallback` | 声明 | `void SetExitRequestedCallback(std::function<void()> callback)` | 更新或应用 set exit requested callback 相关逻辑。 |
| [L47](../src/apps/controller/LoginWindow.h#L47) | `SetSignOutRequestedCallback` | 声明 | `void SetSignOutRequestedCallback(std::function<void()> callback)` | 更新或应用 set sign out requested callback 相关逻辑。 |
| [L48](../src/apps/controller/LoginWindow.h#L48) | `SetDeleteAccountRequestedCallback` | 声明 | `void SetDeleteAccountRequestedCallback(std::function<void()> callback)` | 更新或应用 set delete account requested callback 相关逻辑。 |
| [L49](../src/apps/controller/LoginWindow.h#L49) | `SetOwnerWindow` | 声明 | `void SetOwnerWindow(QWidget* owner)` | 更新或应用 set owner window 相关逻辑。 |
| [L52](../src/apps/controller/LoginWindow.h#L52) | `ShowBusy` | 声明 | `void ShowBusy(const QString& title, const QString& message)` | 实现 show busy 对应的业务或工具逻辑。 |
| [L53](../src/apps/controller/LoginWindow.h#L53) | `ShowAccountDeletionBusy` | 声明 | `void ShowAccountDeletionBusy()` | 实现 show account deletion busy 对应的业务或工具逻辑。 |
| [L54](../src/apps/controller/LoginWindow.h#L54) | `ShowError` | 声明 | `void ShowError(const QString& message, bool retryable = true)` | 实现 show error 对应的业务或工具逻辑。 |
| [L55](../src/apps/controller/LoginWindow.h#L55) | `ShowAuthenticated` | 声明 | `void ShowAuthenticated(const QString& accountLabel, const QString& accountDetail)` | 实现 show authenticated 对应的业务或工具逻辑。 |
| [L57](../src/apps/controller/LoginWindow.h#L57) | `ShowAndActivate` | 声明 | `void ShowAndActivate()` | 实现 show and activate 对应的业务或工具逻辑。 |
| [L58](../src/apps/controller/LoginWindow.h#L58) | `HideWithAnimation` | 声明 | `void HideWithAnimation()` | 实现 hide with animation 对应的业务或工具逻辑。 |
| [L59](../src/apps/controller/LoginWindow.h#L59) | `IsHiding` | 声明 | `bool IsHiding() const` | 判断 is hiding 相关逻辑。 |
| [L60](../src/apps/controller/LoginWindow.h#L60) | `RefreshThemeStyle` | 声明 | `void RefreshThemeStyle()` | 刷新 refresh theme style 相关逻辑。 |
| [L63](../src/apps/controller/LoginWindow.h#L63) | `closeEvent` | 声明 | `void closeEvent(QCloseEvent* event) override` | 关闭并清理 close event 相关逻辑。 |
| [L64](../src/apps/controller/LoginWindow.h#L64) | `eventFilter` | 声明 | `bool eventFilter(QObject* watched, QEvent* event) override` | 实现 event filter 对应的业务或工具逻辑。 |
| [L65](../src/apps/controller/LoginWindow.h#L65) | `mousePressEvent` | 声明 | `void mousePressEvent(QMouseEvent* event) override` | 实现 mouse press event 对应的业务或工具逻辑。 |
| [L68](../src/apps/controller/LoginWindow.h#L68) | `BuildUi` | 声明 | `void BuildUi()` | 创建或初始化 build ui 相关逻辑。 |
| [L69](../src/apps/controller/LoginWindow.h#L69) | `SetStatePanelTone` | 声明 | `void SetStatePanelTone(const QString& tone)` | 更新或应用 set state panel tone 相关逻辑。 |
| [L70](../src/apps/controller/LoginWindow.h#L70) | `UpdateAuthenticatedAccountText` | 声明 | `void UpdateAuthenticatedAccountText()` | 更新或应用 update authenticated account text 相关逻辑。 |
| [L71](../src/apps/controller/LoginWindow.h#L71) | `CardTargetGeometry` | 声明 | `QRect CardTargetGeometry() const` | 实现 card target geometry 对应的业务或工具逻辑。 |
| [L72](../src/apps/controller/LoginWindow.h#L72) | `LayoutCardLayer` | 声明 | `void LayoutCardLayer()` | 实现 layout card layer 对应的业务或工具逻辑。 |
| [L73](../src/apps/controller/LoginWindow.h#L73) | `StartVisibilityMotion` | 声明 | `void StartVisibilityMotion(const QPoint& targetPosition, int durationMs)` | 启动 start visibility motion 相关逻辑。 |
| [L74](../src/apps/controller/LoginWindow.h#L74) | `StopVisibilityMotion` | 声明 | `void StopVisibilityMotion()` | 停止 stop visibility motion 相关逻辑。 |
| [L75](../src/apps/controller/LoginWindow.h#L75) | `IsVisibilityMotionActive` | 声明 | `bool IsVisibilityMotionActive() const` | 判断 is visibility motion active 相关逻辑。 |

## `src/apps/controller/RemoteCComboBox.cpp`

[打开源码](../src/apps/controller/RemoteCComboBox.cpp) · **文件作用：** 实现 remote c combo box 相关函数与文件级辅助逻辑。

### 类型

| 行 | 类型 | 种类 | 作用 |
|---:|---|---|---|
| [L33](../src/apps/controller/RemoteCComboBox.cpp#L33) | `RemoteCComboItemDelegate` | class | 定义 RemoteCComboItemDelegate 的 class 类型和相关状态。 |

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L37](../src/apps/controller/RemoteCComboBox.cpp#L37) | `sizeHint` | 定义 | `QSize sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const override` | 实现 size hint 对应的业务或工具逻辑。 |
| [L45](../src/apps/controller/RemoteCComboBox.cpp#L45) | `paint` | 定义 | `void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const override` | 准备或呈现 paint 相关逻辑。 |
| [L99](../src/apps/controller/RemoteCComboBox.cpp#L99) | `RemoteCComboBox::RemoteCComboBox` | 定义 | `RemoteCComboBox::RemoteCComboBox(QWidget* parent) : QComboBox(parent), arrowAnimation_(new QVariantAnimation(this))` | 构造并初始化 RemoteCComboBox 实例。 |
| [L186](../src/apps/controller/RemoteCComboBox.cpp#L186) | `RemoteCComboBox::RefreshThemeStyle` | 定义 | `void RemoteCComboBox::RefreshThemeStyle()` | 刷新 refresh theme style 相关逻辑。 |
| [L254](../src/apps/controller/RemoteCComboBox.cpp#L254) | `RemoteCComboBox::SetWheelSelectionEnabled` | 定义 | `void RemoteCComboBox::SetWheelSelectionEnabled(bool enabled)` | 更新或应用 set wheel selection enabled 相关逻辑。 |
| [L259](../src/apps/controller/RemoteCComboBox.cpp#L259) | `RemoteCComboBox::wheelEvent` | 定义 | `void RemoteCComboBox::wheelEvent(QWheelEvent* event)` | 实现 wheel event 对应的业务或工具逻辑。 |
| [L268](../src/apps/controller/RemoteCComboBox.cpp#L268) | `RemoteCComboBox::paintEvent` | 定义 | `void RemoteCComboBox::paintEvent(QPaintEvent*)` | 准备或呈现 paint event 相关逻辑。 |
| [L295](../src/apps/controller/RemoteCComboBox.cpp#L295) | `RemoteCComboBox::showPopup` | 定义 | `void RemoteCComboBox::showPopup()` | 实现 show popup 对应的业务或工具逻辑。 |
| [L341](../src/apps/controller/RemoteCComboBox.cpp#L341) | `RemoteCComboBox::hidePopup` | 定义 | `void RemoteCComboBox::hidePopup()` | 实现 hide popup 对应的业务或工具逻辑。 |
| [L349](../src/apps/controller/RemoteCComboBox.cpp#L349) | `RemoteCComboBox::eventFilter` | 定义 | `bool RemoteCComboBox::eventFilter(QObject* watched, QEvent* event)` | 实现 event filter 对应的业务或工具逻辑。 |
| [L374](../src/apps/controller/RemoteCComboBox.cpp#L374) | `RemoteCComboBox::EnsurePopup` | 定义 | `void RemoteCComboBox::EnsurePopup()` | 实现 ensure popup 对应的业务或工具逻辑。 |
| [L446](../src/apps/controller/RemoteCComboBox.cpp#L446) | `RemoteCComboBox::SetPopupOpen` | 定义 | `void RemoteCComboBox::SetPopupOpen(bool open)` | 更新或应用 set popup open 相关逻辑。 |

## `src/apps/controller/RemoteCComboBox.h`

[打开源码](../src/apps/controller/RemoteCComboBox.h) · **文件作用：** 声明 remote c combo box 相关类型、接口、配置和成员状态。

### 类型

| 行 | 类型 | 种类 | 作用 |
|---:|---|---|---|
| [L8](../src/apps/controller/RemoteCComboBox.h#L8) | `QVariantAnimation` | class | 定义 QVariantAnimation 的 class 类型和相关状态。 |
| [L9](../src/apps/controller/RemoteCComboBox.h#L9) | `QFrame` | class | 定义 QFrame 的 class 类型和相关状态。 |
| [L10](../src/apps/controller/RemoteCComboBox.h#L10) | `QListWidget` | class | 定义 QListWidget 的 class 类型和相关状态。 |
| [L11](../src/apps/controller/RemoteCComboBox.h#L11) | `QWheelEvent` | class | 定义 QWheelEvent 的 class 类型和相关状态。 |
| [L15](../src/apps/controller/RemoteCComboBox.h#L15) | `RemoteCComboBox` | class | 定义 RemoteCComboBox 的 class 类型和相关状态。 |

### 成员与文件级变量

| 行 | 变量 | 声明 | 作用 |
|---:|---|---|---|
| [L8](../src/apps/controller/RemoteCComboBox.h#L8) | `QVariantAnimation` | `class QVariantAnimation;` | 保存 q variant animation 相关配置或运行状态。 |
| [L9](../src/apps/controller/RemoteCComboBox.h#L9) | `QFrame` | `class QFrame;` | 保存媒体帧、图像或缓冲资源：q frame。 |
| [L10](../src/apps/controller/RemoteCComboBox.h#L10) | `QListWidget` | `class QListWidget;` | 保存 q list widget 相关配置或运行状态。 |
| [L11](../src/apps/controller/RemoteCComboBox.h#L11) | `QWheelEvent` | `class QWheelEvent;` | 保存 q wheel event 相关配置或运行状态。 |
| [L32](../src/apps/controller/RemoteCComboBox.h#L32) | `arrowAnimation_` | `QVariantAnimation* arrowAnimation_ = nullptr;` | 保存 arrow animation 相关配置或运行状态。 |
| [L33](../src/apps/controller/RemoteCComboBox.h#L33) | `popup_` | `QFrame* popup_ = nullptr;` | 保存 popup 相关配置或运行状态。 |
| [L34](../src/apps/controller/RemoteCComboBox.h#L34) | `popupList_` | `QListWidget* popupList_ = nullptr;` | 保存 popup list 相关配置或运行状态。 |
| [L35](../src/apps/controller/RemoteCComboBox.h#L35) | `lightComboStyleSheet_` | `QString lightComboStyleSheet_;` | 保存 light combo style sheet 相关配置或运行状态。 |
| [L36](../src/apps/controller/RemoteCComboBox.h#L36) | `lightPopupStyleSheet_` | `QString lightPopupStyleSheet_;` | 保存 light popup style sheet 相关配置或运行状态。 |
| [L37](../src/apps/controller/RemoteCComboBox.h#L37) | `darkTheme_` | `bool darkTheme_ = false;` | 保存 dark theme 相关配置或运行状态。 |
| [L38](../src/apps/controller/RemoteCComboBox.h#L38) | `wheelSelectionEnabled_` | `bool wheelSelectionEnabled_ = true;` | 保存能力或开关状态：wheel selection enabled。 |
| [L39](../src/apps/controller/RemoteCComboBox.h#L39) | `arrowRotation_` | `qreal arrowRotation_ = 0.0;` | 保存 arrow rotation 相关配置或运行状态。 |

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L17](../src/apps/controller/RemoteCComboBox.h#L17) | `RemoteCComboBox` | 声明 | `explicit RemoteCComboBox(QWidget* parent = nullptr)` | 实现 remote c combo box 对应的业务或工具逻辑。 |
| [L18](../src/apps/controller/RemoteCComboBox.h#L18) | `RefreshThemeStyle` | 声明 | `void RefreshThemeStyle()` | 刷新 refresh theme style 相关逻辑。 |
| [L19](../src/apps/controller/RemoteCComboBox.h#L19) | `SetWheelSelectionEnabled` | 声明 | `void SetWheelSelectionEnabled(bool enabled)` | 更新或应用 set wheel selection enabled 相关逻辑。 |
| [L22](../src/apps/controller/RemoteCComboBox.h#L22) | `eventFilter` | 声明 | `bool eventFilter(QObject* watched, QEvent* event) override` | 实现 event filter 对应的业务或工具逻辑。 |
| [L23](../src/apps/controller/RemoteCComboBox.h#L23) | `paintEvent` | 声明 | `void paintEvent(QPaintEvent* event) override` | 准备或呈现 paint event 相关逻辑。 |
| [L24](../src/apps/controller/RemoteCComboBox.h#L24) | `wheelEvent` | 声明 | `void wheelEvent(QWheelEvent* event) override` | 实现 wheel event 对应的业务或工具逻辑。 |
| [L25](../src/apps/controller/RemoteCComboBox.h#L25) | `showPopup` | 声明 | `void showPopup() override` | 实现 show popup 对应的业务或工具逻辑。 |
| [L26](../src/apps/controller/RemoteCComboBox.h#L26) | `hidePopup` | 声明 | `void hidePopup() override` | 实现 hide popup 对应的业务或工具逻辑。 |
| [L29](../src/apps/controller/RemoteCComboBox.h#L29) | `EnsurePopup` | 声明 | `void EnsurePopup()` | 实现 ensure popup 对应的业务或工具逻辑。 |
| [L30](../src/apps/controller/RemoteCComboBox.h#L30) | `SetPopupOpen` | 声明 | `void SetPopupOpen(bool open)` | 更新或应用 set popup open 相关逻辑。 |

## `src/apps/controller/RemoteCDialog.cpp`

[打开源码](../src/apps/controller/RemoteCDialog.cpp) · **文件作用：** 实现 remote c dialog 相关函数与文件级辅助逻辑。

### 类型

| 行 | 类型 | 种类 | 作用 |
|---:|---|---|---|
| [L200](../src/apps/controller/RemoteCDialog.cpp#L200) | `DialogDragHeader` | class | 定义 DialogDragHeader 的 class 类型和相关状态。 |
| [L224](../src/apps/controller/RemoteCDialog.cpp#L224) | `DialogActivityIndicator` | class | 定义 DialogActivityIndicator 的 class 类型和相关状态。 |
| [L293](../src/apps/controller/RemoteCDialog.cpp#L293) | `DialogProgressBar` | class | QProgressBar may delegate value transitions to the native Windows style. For a transfer dialog that makes the painted chunk visibly trail the byte counter when updates arrive qu... |
| [L339](../src/apps/controller/RemoteCDialog.cpp#L339) | `DialogOptionCheckBox` | class | 定义 DialogOptionCheckBox 的 class 类型和相关状态。 |

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L171](../src/apps/controller/RemoteCDialog.cpp#L171) | `BadgeText` | 定义 | `QString BadgeText(RemoteCDialog::Tone tone)` | 实现 badge text 对应的业务或工具逻辑。 |
| [L185](../src/apps/controller/RemoteCDialog.cpp#L185) | `BadgeStyle` | 定义 | `QString BadgeStyle(RemoteCDialog::Tone tone)` | 实现 badge style 对应的业务或工具逻辑。 |
| [L202](../src/apps/controller/RemoteCDialog.cpp#L202) | `DialogDragHeader` | 定义 | `explicit DialogDragHeader(QDialog* dialog, QWidget* parent) : QWidget(parent), dialog_(dialog)` | 实现 dialog drag header 对应的业务或工具逻辑。 |
| [L209](../src/apps/controller/RemoteCDialog.cpp#L209) | `mousePressEvent` | 定义 | `void mousePressEvent(QMouseEvent* event) override` | 实现 mouse press event 对应的业务或工具逻辑。 |
| [L226](../src/apps/controller/RemoteCDialog.cpp#L226) | `DialogActivityIndicator` | 定义 | `explicit DialogActivityIndicator(QWidget* parent) : QWidget(parent)` | 实现 dialog activity indicator 对应的业务或工具逻辑。 |
| [L239](../src/apps/controller/RemoteCDialog.cpp#L239) | `showEvent` | 定义 | `void showEvent(QShowEvent* event) override` | 实现 show event 对应的业务或工具逻辑。 |
| [L246](../src/apps/controller/RemoteCDialog.cpp#L246) | `hideEvent` | 定义 | `void hideEvent(QHideEvent* event) override` | 实现 hide event 对应的业务或工具逻辑。 |
| [L252](../src/apps/controller/RemoteCDialog.cpp#L252) | `paintEvent` | 定义 | `void paintEvent(QPaintEvent*) override` | 准备或呈现 paint event 相关逻辑。 |
| [L295](../src/apps/controller/RemoteCDialog.cpp#L295) | `DialogProgressBar` | 定义 | `explicit DialogProgressBar(QWidget* parent = nullptr) : QProgressBar(parent)` | 实现 dialog progress bar 对应的业务或工具逻辑。 |
| [L302](../src/apps/controller/RemoteCDialog.cpp#L302) | `paintEvent` | 定义 | `void paintEvent(QPaintEvent*) override` | 准备或呈现 paint event 相关逻辑。 |
| [L341](../src/apps/controller/RemoteCDialog.cpp#L341) | `DialogOptionCheckBox` | 定义 | `explicit DialogOptionCheckBox( const QString& text, QWidget* parent = nullptr) : QCheckBox(text, parent)` | 实现 dialog option check box 对应的业务或工具逻辑。 |
| [L350](../src/apps/controller/RemoteCDialog.cpp#L350) | `paintEvent` | 定义 | `void paintEvent(QPaintEvent*) override` | 准备或呈现 paint event 相关逻辑。 |
| [L408](../src/apps/controller/RemoteCDialog.cpp#L408) | `RemoteCDialog::Confirm` | 定义 | `bool RemoteCDialog::Confirm(QWidget* parent, const QString& title, const QString& message, const QString& confirmText, const QString& cancelText, Tone tone, bool requestAttention, bool nonActivating, std::uintptr_t na...` | 实现 confirm 对应的业务或工具逻辑。 |
| [L427](../src/apps/controller/RemoteCDialog.cpp#L427) | `RemoteCDialog::ConfirmWithOption` | 定义 | `bool RemoteCDialog::ConfirmWithOption( QWidget* parent, const QString& title, const QString& message, const QString& confirmText, const QString& cancelText, const QString& optionText, bool optionInitiallyChecked, bool...` | 实现 confirm with option 对应的业务或工具逻辑。 |
| [L455](../src/apps/controller/RemoteCDialog.cpp#L455) | `RemoteCDialog::Alert` | 定义 | `void RemoteCDialog::Alert(QWidget* parent, const QString& title, const QString& message, const QString& buttonText, Tone tone, bool requestAttention)` | 实现 alert 对应的业务或工具逻辑。 |
| [L467](../src/apps/controller/RemoteCDialog.cpp#L467) | `RemoteCDialog::CreateStatus` | 定义 | `RemoteCDialog* RemoteCDialog::CreateStatus( QWidget* parent, const QString& title, const QString& message, const QString& buttonText, Tone tone)` | 创建或初始化 create status 相关逻辑。 |
| [L502](../src/apps/controller/RemoteCDialog.cpp#L502) | `RemoteCDialog::SetContent` | 定义 | `void RemoteCDialog::SetContent(const QString& title, const QString& message, const QString& buttonText, Tone tone)` | 更新或应用 set content 相关逻辑。 |
| [L552](../src/apps/controller/RemoteCDialog.cpp#L552) | `RemoteCDialog::SetStatusActionHandler` | 定义 | `void RemoteCDialog::SetStatusActionHandler(std::function<void()> handler)` | 更新或应用 set status action handler 相关逻辑。 |
| [L557](../src/apps/controller/RemoteCDialog.cpp#L557) | `RemoteCDialog::SetProgress` | 定义 | `void RemoteCDialog::SetProgress(double progress)` | 更新或应用 set progress 相关逻辑。 |
| [L569](../src/apps/controller/RemoteCDialog.cpp#L569) | `RemoteCDialog::SetNonActivatingWindow` | 定义 | `void RemoteCDialog::SetNonActivatingWindow(bool enabled)` | 更新或应用 set non activating window 相关逻辑。 |
| [L580](../src/apps/controller/RemoteCDialog.cpp#L580) | `RemoteCDialog::SetNativeAnchorWindow` | 定义 | `void RemoteCDialog::SetNativeAnchorWindow(std::uintptr_t windowHandle)` | 更新或应用 set native anchor window 相关逻辑。 |
| [L585](../src/apps/controller/RemoteCDialog.cpp#L585) | `RemoteCDialog::ApplyNonActivatingNativeStyle` | 定义 | `void RemoteCDialog::ApplyNonActivatingNativeStyle()` | 更新或应用 apply non activating native style 相关逻辑。 |
| [L606](../src/apps/controller/RemoteCDialog.cpp#L606) | `RemoteCDialog::RemoteCDialog` | 定义 | `RemoteCDialog::RemoteCDialog(QWidget* parent, const QString& title, const QString& message, const QString& confirmText, const QString& cancelText, Tone tone, bool showCancel, bool requestAttention) : QDialog(parent ? ...` | 构造并初始化 RemoteCDialog 实例。 |
| [L751](../src/apps/controller/RemoteCDialog.cpp#L751) | `RemoteCDialog::eventFilter` | 定义 | `bool RemoteCDialog::eventFilter(QObject* watched, QEvent* event)` | 实现 event filter 对应的业务或工具逻辑。 |
| [L800](../src/apps/controller/RemoteCDialog.cpp#L800) | `RemoteCDialog::showEvent` | 定义 | `void RemoteCDialog::showEvent(QShowEvent* event)` | 实现 show event 对应的业务或工具逻辑。 |

## `src/apps/controller/RemoteCDialog.h`

[打开源码](../src/apps/controller/RemoteCDialog.h) · **文件作用：** 声明 remote c dialog 相关类型、接口、配置和成员状态。

### 类型

| 行 | 类型 | 种类 | 作用 |
|---:|---|---|---|
| [L13](../src/apps/controller/RemoteCDialog.h#L13) | `QLabel` | class | 定义 QLabel 的 class 类型和相关状态。 |
| [L14](../src/apps/controller/RemoteCDialog.h#L14) | `QFrame` | class | 定义 QFrame 的 class 类型和相关状态。 |
| [L15](../src/apps/controller/RemoteCDialog.h#L15) | `QPushButton` | class | 定义 QPushButton 的 class 类型和相关状态。 |
| [L16](../src/apps/controller/RemoteCDialog.h#L16) | `QProgressBar` | class | 定义 QProgressBar 的 class 类型和相关状态。 |
| [L20](../src/apps/controller/RemoteCDialog.h#L20) | `RemoteCDialog` | class | 定义 RemoteCDialog 的 class 类型和相关状态。 |
| [L22](../src/apps/controller/RemoteCDialog.h#L22) | `Tone` | enum class | 定义 Tone 的 enum class 类型和相关状态。 |

### 成员与文件级变量

| 行 | 变量 | 声明 | 作用 |
|---:|---|---|---|
| [L13](../src/apps/controller/RemoteCDialog.h#L13) | `QLabel` | `class QLabel;` | 保存路径、地址或显示名称：q label。 |
| [L14](../src/apps/controller/RemoteCDialog.h#L14) | `QFrame` | `class QFrame;` | 保存媒体帧、图像或缓冲资源：q frame。 |
| [L15](../src/apps/controller/RemoteCDialog.h#L15) | `QPushButton` | `class QPushButton;` | 保存 q push button 相关配置或运行状态。 |
| [L16](../src/apps/controller/RemoteCDialog.h#L16) | `QProgressBar` | `class QProgressBar;` | 保存 q progress bar 相关配置或运行状态。 |
| [L96](../src/apps/controller/RemoteCDialog.h#L96) | `shown_` | `bool shown_ = false;` | 保存 shown 相关配置或运行状态。 |
| [L97](../src/apps/controller/RemoteCDialog.h#L97) | `statusDialog_` | `bool statusDialog_ = false;` | 保存 status dialog 相关配置或运行状态。 |
| [L98](../src/apps/controller/RemoteCDialog.h#L98) | `requestAttention_` | `bool requestAttention_ = false;` | 保存 request attention 相关配置或运行状态。 |
| [L99](../src/apps/controller/RemoteCDialog.h#L99) | `nonActivatingWindow_` | `bool nonActivatingWindow_ = false;` | 保存 non activating window 相关配置或运行状态。 |
| [L100](../src/apps/controller/RemoteCDialog.h#L100) | `nativeAnchorWindow_` | `std::uintptr_t nativeAnchorWindow_ = 0;` | 保存 native anchor window 相关配置或运行状态。 |
| [L101](../src/apps/controller/RemoteCDialog.h#L101) | `centerOnAnchorScreen_` | `bool centerOnAnchorScreen_ = false;` | 保存 center on anchor screen 相关配置或运行状态。 |
| [L102](../src/apps/controller/RemoteCDialog.h#L102) | `nonActivatingDragActive_` | `bool nonActivatingDragActive_ = false;` | 保存能力或开关状态：non activating drag active。 |
| [L103](../src/apps/controller/RemoteCDialog.h#L103) | `nonActivatingDragStartGlobal_` | `QPoint nonActivatingDragStartGlobal_;` | 保存 non activating drag start global 相关配置或运行状态。 |
| [L104](../src/apps/controller/RemoteCDialog.h#L104) | `nonActivatingDragStartTopLeft_` | `QPoint nonActivatingDragStartTopLeft_;` | 保存 non activating drag start top left 相关配置或运行状态。 |
| [L105](../src/apps/controller/RemoteCDialog.h#L105) | `card_` | `QFrame* card_ = nullptr;` | 保存 card 相关配置或运行状态。 |
| [L106](../src/apps/controller/RemoteCDialog.h#L106) | `badgeLabel_` | `QLabel* badgeLabel_ = nullptr;` | 保存路径、地址或显示名称：badge label。 |
| [L107](../src/apps/controller/RemoteCDialog.h#L107) | `titleLabel_` | `QLabel* titleLabel_ = nullptr;` | 保存路径、地址或显示名称：title label。 |
| [L108](../src/apps/controller/RemoteCDialog.h#L108) | `messageLabel_` | `QLabel* messageLabel_ = nullptr;` | 保存路径、地址或显示名称：message label。 |
| [L109](../src/apps/controller/RemoteCDialog.h#L109) | `activityIndicator_` | `QWidget* activityIndicator_ = nullptr;` | 保存 activity indicator 相关配置或运行状态。 |
| [L110](../src/apps/controller/RemoteCDialog.h#L110) | `progressBar_` | `QProgressBar* progressBar_ = nullptr;` | 保存 progress bar 相关配置或运行状态。 |
| [L111](../src/apps/controller/RemoteCDialog.h#L111) | `confirmButton_` | `QPushButton* confirmButton_ = nullptr;` | 保存 confirm button 相关配置或运行状态。 |

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L29](../src/apps/controller/RemoteCDialog.h#L29) | `Confirm` | 声明 | `static bool Confirm(QWidget* parent, const QString& title, const QString& message, const QString& confirmText, const QString& cancelText = QStringLiteral("取消"), Tone tone = Tone::kQuestion, bool requestAttention = fal...` | 实现 confirm 对应的业务或工具逻辑。 |
| [L40](../src/apps/controller/RemoteCDialog.h#L40) | `ConfirmWithOption` | 声明 | `static bool ConfirmWithOption( QWidget* parent, const QString& title, const QString& message, const QString& confirmText, const QString& cancelText, const QString& optionText, bool optionInitiallyChecked, bool* option...` | 实现 confirm with option 对应的业务或工具逻辑。 |
| [L52](../src/apps/controller/RemoteCDialog.h#L52) | `Alert` | 声明 | `static void Alert(QWidget* parent, const QString& title, const QString& message, const QString& buttonText = QStringLiteral("知道了"), Tone tone = Tone::kWarning, bool requestAttention = false)` | 实现 alert 对应的业务或工具逻辑。 |
| [L61](../src/apps/controller/RemoteCDialog.h#L61) | `CreateStatus` | 声明 | `static RemoteCDialog* CreateStatus( QWidget* parent, const QString& title, const QString& message, const QString& buttonText = QStringLiteral("隐藏提示"), Tone tone = Tone::kWarning)` | Creates a movable, non-modal status dialog using the same visual language as room-join and control approval prompts. The parent owns it. |
| [L68](../src/apps/controller/RemoteCDialog.h#L68) | `SetContent` | 声明 | `void SetContent(const QString& title, const QString& message, const QString& buttonText, Tone tone)` | 更新或应用 set content 相关逻辑。 |
| [L72](../src/apps/controller/RemoteCDialog.h#L72) | `SetStatusActionHandler` | 声明 | `void SetStatusActionHandler(std::function<void()> handler)` | 更新或应用 set status action handler 相关逻辑。 |
| [L73](../src/apps/controller/RemoteCDialog.h#L73) | `SetProgress` | 声明 | `void SetProgress(double progress)` | 更新或应用 set progress 相关逻辑。 |
| [L77](../src/apps/controller/RemoteCDialog.h#L77) | `SetNonActivatingWindow` | 声明 | `void SetNonActivatingWindow(bool enabled)` | Keeps a status window above a foreign target (for example Explorer) without activating this dialog or bringing its Qt owner to the front. The dialog remains mouse-draggable thro... |
| [L78](../src/apps/controller/RemoteCDialog.h#L78) | `SetNativeAnchorWindow` | 声明 | `void SetNativeAnchorWindow(std::uintptr_t windowHandle)` | 更新或应用 set native anchor window 相关逻辑。 |
| [L81](../src/apps/controller/RemoteCDialog.h#L81) | `eventFilter` | 声明 | `bool eventFilter(QObject* watched, QEvent* event) override` | 实现 event filter 对应的业务或工具逻辑。 |
| [L82](../src/apps/controller/RemoteCDialog.h#L82) | `showEvent` | 声明 | `void showEvent(QShowEvent* event) override` | 实现 show event 对应的业务或工具逻辑。 |
| [L85](../src/apps/controller/RemoteCDialog.h#L85) | `ApplyNonActivatingNativeStyle` | 声明 | `void ApplyNonActivatingNativeStyle()` | 更新或应用 apply non activating native style 相关逻辑。 |
| [L87](../src/apps/controller/RemoteCDialog.h#L87) | `RemoteCDialog` | 声明 | `RemoteCDialog(QWidget* parent, const QString& title, const QString& message, const QString& confirmText, const QString& cancelText, Tone tone, bool showCancel, bool requestAttention)` | 实现 remote c dialog 对应的业务或工具逻辑。 |

## `src/apps/controller/RemoteCToast.cpp`

[打开源码](../src/apps/controller/RemoteCToast.cpp) · **文件作用：** 实现 remote c toast 相关函数与文件级辅助逻辑。

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L23](../src/apps/controller/RemoteCToast.cpp#L23) | `ToastHost` | 定义 | `QWidget* ToastHost(QWidget* parent)` | 实现 toast host 对应的业务或工具逻辑。 |
| [L37](../src/apps/controller/RemoteCToast.cpp#L37) | `AnimationLevel` | 定义 | `int AnimationLevel()` | 实现 animation level 对应的业务或工具逻辑。 |
| [L48](../src/apps/controller/RemoteCToast.cpp#L48) | `RemoteCToast::Show` | 定义 | `void RemoteCToast::Show(QWidget* parent, const QString& message, Tone tone)` | 实现 show 对应的业务或工具逻辑。 |
| [L71](../src/apps/controller/RemoteCToast.cpp#L71) | `RemoteCToast::ShowAbove` | 定义 | `void RemoteCToast::ShowAbove(QWidget* anchor, const QString& message, Tone tone)` | 实现 show above 对应的业务或工具逻辑。 |
| [L94](../src/apps/controller/RemoteCToast.cpp#L94) | `RemoteCToast::ShowAtGlobalCenter` | 定义 | `void RemoteCToast::ShowAtGlobalCenter( QWidget* parent, const QPoint& globalCenter, const QString& message, Tone tone, bool prominent)` | 实现 show at global center 对应的业务或工具逻辑。 |
| [L120](../src/apps/controller/RemoteCToast.cpp#L120) | `RemoteCToast::RemoteCToast` | 定义 | `RemoteCToast::RemoteCToast(QWidget* host) : QFrame(host), host_(host)` | 构造并初始化 RemoteCToast 实例。 |
| [L151](../src/apps/controller/RemoteCToast.cpp#L151) | `RemoteCToast::ShowMessage` | 定义 | `void RemoteCToast::ShowMessage(const QString& message, Tone tone, QWidget* anchor, const QPoint* globalCenter, bool prominent)` | 实现 show message 对应的业务或工具逻辑。 |
| [L247](../src/apps/controller/RemoteCToast.cpp#L247) | `RemoteCToast::FadeOut` | 定义 | `void RemoteCToast::FadeOut()` | 实现 fade out 对应的业务或工具逻辑。 |
| [L267](../src/apps/controller/RemoteCToast.cpp#L267) | `RemoteCToast::Reposition` | 定义 | `void RemoteCToast::Reposition()` | 实现 reposition 对应的业务或工具逻辑。 |
| [L300](../src/apps/controller/RemoteCToast.cpp#L300) | `RemoteCToast::eventFilter` | 定义 | `bool RemoteCToast::eventFilter(QObject* watched, QEvent* event)` | 实现 event filter 对应的业务或工具逻辑。 |

## `src/apps/controller/RemoteCToast.h`

[打开源码](../src/apps/controller/RemoteCToast.h) · **文件作用：** 声明 remote c toast 相关类型、接口、配置和成员状态。

### 类型

| 行 | 类型 | 种类 | 作用 |
|---:|---|---|---|
| [L11](../src/apps/controller/RemoteCToast.h#L11) | `QEvent` | class | 定义 QEvent 的 class 类型和相关状态。 |
| [L12](../src/apps/controller/RemoteCToast.h#L12) | `QGraphicsOpacityEffect` | class | 定义 QGraphicsOpacityEffect 的 class 类型和相关状态。 |
| [L13](../src/apps/controller/RemoteCToast.h#L13) | `QLabel` | class | 定义 QLabel 的 class 类型和相关状态。 |
| [L14](../src/apps/controller/RemoteCToast.h#L14) | `QPropertyAnimation` | class | 定义 QPropertyAnimation 的 class 类型和相关状态。 |
| [L15](../src/apps/controller/RemoteCToast.h#L15) | `QTimer` | class | 定义 QTimer 的 class 类型和相关状态。 |
| [L19](../src/apps/controller/RemoteCToast.h#L19) | `RemoteCToast` | class | 定义 RemoteCToast 的 class 类型和相关状态。 |
| [L21](../src/apps/controller/RemoteCToast.h#L21) | `Tone` | enum class | 定义 Tone 的 enum class 类型和相关状态。 |

### 成员与文件级变量

| 行 | 变量 | 声明 | 作用 |
|---:|---|---|---|
| [L11](../src/apps/controller/RemoteCToast.h#L11) | `QEvent` | `class QEvent;` | 保存 q event 相关配置或运行状态。 |
| [L12](../src/apps/controller/RemoteCToast.h#L12) | `QGraphicsOpacityEffect` | `class QGraphicsOpacityEffect;` | 保存 q graphics opacity effect 相关配置或运行状态。 |
| [L13](../src/apps/controller/RemoteCToast.h#L13) | `QLabel` | `class QLabel;` | 保存路径、地址或显示名称：q label。 |
| [L14](../src/apps/controller/RemoteCToast.h#L14) | `QPropertyAnimation` | `class QPropertyAnimation;` | 保存 q property animation 相关配置或运行状态。 |
| [L15](../src/apps/controller/RemoteCToast.h#L15) | `QTimer` | `class QTimer;` | 保存定时、截止或超时状态：q timer。 |
| [L53](../src/apps/controller/RemoteCToast.h#L53) | `host_` | `QWidget* host_ = nullptr;` | 保存 host 相关配置或运行状态。 |
| [L54](../src/apps/controller/RemoteCToast.h#L54) | `anchor_` | `QPointer<QWidget> anchor_;` | 保存 anchor 相关配置或运行状态。 |
| [L55](../src/apps/controller/RemoteCToast.h#L55) | `globalCenter_` | `QPoint globalCenter_;` | 保存 global center 相关配置或运行状态。 |
| [L56](../src/apps/controller/RemoteCToast.h#L56) | `useGlobalCenter_` | `bool useGlobalCenter_ = false;` | 保存 use global center 相关配置或运行状态。 |
| [L57](../src/apps/controller/RemoteCToast.h#L57) | `marker_` | `QLabel* marker_ = nullptr;` | 保存 marker 相关配置或运行状态。 |
| [L58](../src/apps/controller/RemoteCToast.h#L58) | `message_` | `QLabel* message_ = nullptr;` | 保存 message 相关配置或运行状态。 |
| [L59](../src/apps/controller/RemoteCToast.h#L59) | `timer_` | `QTimer* timer_ = nullptr;` | 保存定时、截止或超时状态：timer。 |
| [L60](../src/apps/controller/RemoteCToast.h#L60) | `opacity_` | `QGraphicsOpacityEffect* opacity_ = nullptr;` | 保存 opacity 相关配置或运行状态。 |
| [L61](../src/apps/controller/RemoteCToast.h#L61) | `animation_` | `QPropertyAnimation* animation_ = nullptr;` | 保存 animation 相关配置或运行状态。 |

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L27](../src/apps/controller/RemoteCToast.h#L27) | `Show` | 声明 | `static void Show(QWidget* parent, const QString& message, Tone tone = Tone::kInformation)` | 实现 show 对应的业务或工具逻辑。 |
| [L30](../src/apps/controller/RemoteCToast.h#L30) | `ShowAbove` | 声明 | `static void ShowAbove(QWidget* anchor, const QString& message, Tone tone = Tone::kInformation)` | 实现 show above 对应的业务或工具逻辑。 |
| [L33](../src/apps/controller/RemoteCToast.h#L33) | `ShowAtGlobalCenter` | 声明 | `static void ShowAtGlobalCenter( QWidget* parent, const QPoint& globalCenter, const QString& message, Tone tone = Tone::kInformation, bool prominent = false)` | 实现 show at global center 对应的业务或工具逻辑。 |
| [L41](../src/apps/controller/RemoteCToast.h#L41) | `eventFilter` | 声明 | `bool eventFilter(QObject* watched, QEvent* event) override` | 实现 event filter 对应的业务或工具逻辑。 |
| [L44](../src/apps/controller/RemoteCToast.h#L44) | `RemoteCToast` | 声明 | `explicit RemoteCToast(QWidget* host)` | 实现 remote c toast 对应的业务或工具逻辑。 |
| [L45](../src/apps/controller/RemoteCToast.h#L45) | `ShowMessage` | 声明 | `void ShowMessage(const QString& message, Tone tone, QWidget* anchor, const QPoint* globalCenter = nullptr, bool prominent = false)` | 实现 show message 对应的业务或工具逻辑。 |
| [L50](../src/apps/controller/RemoteCToast.h#L50) | `Reposition` | 声明 | `void Reposition()` | 实现 reposition 对应的业务或工具逻辑。 |
| [L51](../src/apps/controller/RemoteCToast.h#L51) | `FadeOut` | 声明 | `void FadeOut()` | 实现 fade out 对应的业务或工具逻辑。 |

## `src/apps/controller/RemoteCursorOverlay.inc`

[打开源码](../src/apps/controller/RemoteCursorOverlay.inc) · **文件作用：** `RemoteCursorOverlay` 的实现切片，集中实现 remote cursor overlay 相关逻辑；成员状态仍定义在所属头文件中。

### 类型

| 行 | 类型 | 种类 | 作用 |
|---:|---|---|---|
| [L8](../src/apps/controller/RemoteCursorOverlay.inc#L8) | `RemoteCursorRenderState` | class | Thread-safe cursor state shared by the Qt paint path and the dedicated D3D11 presentation thread. This is deliberately not a QWidget: moving a translucent native window at point... |
| [L10](../src/apps/controller/RemoteCursorOverlay.inc#L10) | `Snapshot` | struct | 定义 Snapshot 的 struct 类型和相关状态。 |

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L18](../src/apps/controller/RemoteCursorOverlay.inc#L18) | `RemoteCursorRenderState` | 定义 | `RemoteCursorRenderState() { BuildFallbackArrow(); }` | 实现 remote cursor render state 对应的业务或工具逻辑。 |
| [L20](../src/apps/controller/RemoteCursorOverlay.inc#L20) | `SetShape` | 定义 | `void SetShape(const RemoteCursorShape& shape)` | 更新或应用 set shape 相关逻辑。 |
| [L51](../src/apps/controller/RemoteCursorOverlay.inc#L51) | `SetPosition` | 定义 | `void SetPosition(const RemoteCursorPosition& position, bool predicted)` | 更新或应用 set position 相关逻辑。 |
| [L65](../src/apps/controller/RemoteCursorOverlay.inc#L65) | `ApplyRemotePosition` | 定义 | `void ApplyRemotePosition(const RemoteCursorPosition& position)` | 更新或应用 apply remote position 相关逻辑。 |
| [L83](../src/apps/controller/RemoteCursorOverlay.inc#L83) | `Reset` | 定义 | `void Reset()` | 重置或移除 reset 相关逻辑。 |
| [L91](../src/apps/controller/RemoteCursorOverlay.inc#L91) | `SetRenderingEnabled` | 定义 | `void SetRenderingEnabled(bool enabled)` | 更新或应用 set rendering enabled 相关逻辑。 |
| [L97](../src/apps/controller/RemoteCursorOverlay.inc#L97) | `GetSnapshot` | 定义 | `Snapshot GetSnapshot() const` | 查询并返回 get snapshot 相关逻辑。 |
| [L104](../src/apps/controller/RemoteCursorOverlay.inc#L104) | `Paint` | 定义 | `void Paint(QPainter& painter, const QRect& content, const QSize& sourceSize) const` | 准备或呈现 paint 相关逻辑。 |
| [L119](../src/apps/controller/RemoteCursorOverlay.inc#L119) | `TargetRect` | 定义 | `static QRect TargetRect(const Snapshot& snapshot, const QRect& content, const QSize& sourceSize)` | 实现 target rect 对应的业务或工具逻辑。 |
| [L144](../src/apps/controller/RemoteCursorOverlay.inc#L144) | `BuildFallbackArrow` | 定义 | `void BuildFallbackArrow()` | 创建或初始化 build fallback arrow 相关逻辑。 |

## `src/apps/controller/RemoteDesktopCanvas.Input.inc`

[打开源码](../src/apps/controller/RemoteDesktopCanvas.Input.inc) · **文件作用：** `RemoteDesktopCanvas` 的实现切片，集中实现 input 相关逻辑；成员状态仍定义在所属头文件中。

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L4](../src/apps/controller/RemoteDesktopCanvas.Input.inc#L4) | `UpdateNativeSurfaceGeometry` | 定义 | `void UpdateNativeSurfaceGeometry()` | SPDX-License-Identifier: GPL-3.0-only Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd) |
| [L11](../src/apps/controller/RemoteDesktopCanvas.Input.inc#L11) | `VideoContentRect` | 定义 | `QRect VideoContentRect() const` | 实现 video content rect 对应的业务或工具逻辑。 |
| [L48](../src/apps/controller/RemoteDesktopCanvas.Input.inc#L48) | `VideoSourceSize` | 定义 | `QSize VideoSourceSize() const` | 实现 video source size 对应的业务或工具逻辑。 |
| [L62](../src/apps/controller/RemoteDesktopCanvas.Input.inc#L62) | `SampleCurrentDragPointer` | 定义 | `SampleCurrentDragPointer( const RemoteInputEvent& fallback) const` | 实现 sample current drag pointer 对应的业务或工具逻辑。 |
| [L141](../src/apps/controller/RemoteDesktopCanvas.Input.inc#L141) | `MapPoint` | 定义 | `std::optional<std::pair<std::uint16_t, std::uint16_t>> MapPoint( const QPointF& point, bool clampToContent) const` | 实现 map point 对应的业务或工具逻辑。 |
| [L181](../src/apps/controller/RemoteDesktopCanvas.Input.inc#L181) | `MapMouseButton` | 定义 | `static std::optional<RemoteMouseButton> MapMouseButton( Qt::MouseButton button)` | 实现 map mouse button 对应的业务或工具逻辑。 |
| [L200](../src/apps/controller/RemoteDesktopCanvas.Input.inc#L200) | `MapMouseButtons` | 定义 | `static std::uint8_t MapMouseButtons(Qt::MouseButtons buttons)` | 实现 map mouse buttons 对应的业务或工具逻辑。 |
| [L221](../src/apps/controller/RemoteDesktopCanvas.Input.inc#L221) | `ClampWheelDelta` | 定义 | `static std::int16_t ClampWheelDelta(int value)` | 实现 clamp wheel delta 对应的业务或工具逻辑。 |
| [L229](../src/apps/controller/RemoteDesktopCanvas.Input.inc#L229) | `IsExtendedVirtualKey` | 定义 | `static bool IsExtendedVirtualKey(std::uint32_t virtualKey)` | 判断 is extended virtual key 相关逻辑。 |
| [L256](../src/apps/controller/RemoteDesktopCanvas.Input.inc#L256) | `QueuePointerMove` | 定义 | `bool QueuePointerMove( const std::pair<std::uint16_t, std::uint16_t>& point, std::uint8_t pressedMouseButtons)` | 实现 queue pointer move 对应的业务或工具逻辑。 |
| [L293](../src/apps/controller/RemoteDesktopCanvas.Input.inc#L293) | `FlushPendingPointerMove` | 定义 | `void FlushPendingPointerMove()` | 实现 flush pending pointer move 对应的业务或工具逻辑。 |
| [L300](../src/apps/controller/RemoteDesktopCanvas.Input.inc#L300) | `CancelPendingPointerMove` | 定义 | `void CancelPendingPointerMove()` | 判断 cancel pending pointer move 相关逻辑。 |
| [L307](../src/apps/controller/RemoteDesktopCanvas.Input.inc#L307) | `SendKeyEvent` | 定义 | `bool SendKeyEvent(QKeyEvent* event, bool pressed)` | 发送或发布 send key event 相关逻辑。 |
| [L332](../src/apps/controller/RemoteDesktopCanvas.Input.inc#L332) | `FocusRemotePasteTarget` | 定义 | `void FocusRemotePasteTarget(const QPointF& position)` | 实现 focus remote paste target 对应的业务或工具逻辑。 |

## `src/apps/controller/RemoteDesktopCanvas.Public.inc`

[打开源码](../src/apps/controller/RemoteDesktopCanvas.Public.inc) · **文件作用：** `RemoteDesktopCanvas` 的实现切片，集中实现 public 相关逻辑；成员状态仍定义在所属头文件中。

### 类型

| 行 | 类型 | 种类 | 作用 |
|---:|---|---|---|
| [L4](../src/apps/controller/RemoteDesktopCanvas.Public.inc#L4) | `RemoteDesktopCanvas` | class | SPDX-License-Identifier: GPL-3.0-only Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd) |

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L24](../src/apps/controller/RemoteDesktopCanvas.Public.inc#L24) | `pasteSender_` | 定义 | `pasteSender_(std::move(pasteSender)), firstPresentationCallback_( std::move(firstPresentationCallback)), presentationTelemetryId_( VideoPresentationTelemetryRegistry::Instance().Register( std::move(telemetryPeerDevice...` | 实现 paste sender 对应的业务或工具逻辑。 |
| [L84](../src/apps/controller/RemoteDesktopCanvas.Public.inc#L84) | `~RemoteDesktopCanvas` | 定义 | `~RemoteDesktopCanvas() override` | 停止相关活动并释放 RemoteDesktopCanvas 实例拥有的资源。 |
| [L101](../src/apps/controller/RemoteDesktopCanvas.Public.inc#L101) | `SetControlEnabled` | 定义 | `void SetControlEnabled(bool enabled)` | 更新或应用 set control enabled 相关逻辑。 |
| [L123](../src/apps/controller/RemoteDesktopCanvas.Public.inc#L123) | `SetActualPixelDisplayMode` | 定义 | `void SetActualPixelDisplayMode(bool enabled)` | 更新或应用 set actual pixel display mode 相关逻辑。 |
| [L142](../src/apps/controller/RemoteDesktopCanvas.Public.inc#L142) | `ActualPixelDisplayMode` | 定义 | `bool ActualPixelDisplayMode() const` | 实现 actual pixel display mode 对应的业务或工具逻辑。 |
| [L148](../src/apps/controller/RemoteDesktopCanvas.Public.inc#L148) | `ApplyRemoteCursor` | 定义 | `void ApplyRemoteCursor(const RemoteCursorEnvelope& envelope)` | 更新或应用 apply remote cursor 相关逻辑。 |
| [L170](../src/apps/controller/RemoteDesktopCanvas.Public.inc#L170) | `ResetRemoteCursor` | 定义 | `void ResetRemoteCursor()` | 重置或移除 reset remote cursor 相关逻辑。 |
| [L177](../src/apps/controller/RemoteDesktopCanvas.Public.inc#L177) | `SetConnectionStage` | 定义 | `void SetConnectionStage(QString stage)` | 更新或应用 set connection stage 相关逻辑。 |
| [L190](../src/apps/controller/RemoteDesktopCanvas.Public.inc#L190) | `BeginPresentationGeneration` | 定义 | `void BeginPresentationGeneration(std::uint64_t generation)` | 启动 begin presentation generation 相关逻辑。 |
| [L198](../src/apps/controller/RemoteDesktopCanvas.Public.inc#L198) | `NotifyFirstPresentation` | 定义 | `void NotifyFirstPresentation()` | 通知或报告 notify first presentation 相关逻辑。 |
| [L213](../src/apps/controller/RemoteDesktopCanvas.Public.inc#L213) | `SetTargetFrameRate` | 定义 | `void SetTargetFrameRate(std::uint32_t framesPerSecond)` | 更新或应用 set target frame rate 相关逻辑。 |
| [L224](../src/apps/controller/RemoteDesktopCanvas.Public.inc#L224) | `SetDragPointerSampleRate` | 定义 | `void SetDragPointerSampleRate(std::uint32_t hertz)` | 更新或应用 set drag pointer sample rate 相关逻辑。 |
| [L240](../src/apps/controller/RemoteDesktopCanvas.Public.inc#L240) | `ShutdownInputScheduler` | 定义 | `void ShutdownInputScheduler()` | 关闭并清理 shutdown input scheduler 相关逻辑。 |
| [L247](../src/apps/controller/RemoteDesktopCanvas.Public.inc#L247) | `SetRemoteDisplayIdentity` | 定义 | `void SetRemoteDisplayIdentity( std::uint32_t displayId, std::uint64_t layoutVersion)` | 更新或应用 set remote display identity 相关逻辑。 |
| [L261](../src/apps/controller/RemoteDesktopCanvas.Public.inc#L261) | `ReleaseRemoteInputs` | 定义 | `void ReleaseRemoteInputs()` | 释放或取消 release remote inputs 相关逻辑。 |
| [L272](../src/apps/controller/RemoteDesktopCanvas.Public.inc#L272) | `OnFrame` | 定义 | `void OnFrame(const webrtc::VideoFrame& frame) override` | 接收并处理 on frame 相关逻辑。 |
| [L321](../src/apps/controller/RemoteDesktopCanvas.Public.inc#L321) | `mouseMoveEvent` | 定义 | `void mouseMoveEvent(QMouseEvent* event) override` | 实现 mouse move event 对应的业务或工具逻辑。 |
| [L345](../src/apps/controller/RemoteDesktopCanvas.Public.inc#L345) | `mousePressEvent` | 定义 | `void mousePressEvent(QMouseEvent* event) override` | 实现 mouse press event 对应的业务或工具逻辑。 |
| [L392](../src/apps/controller/RemoteDesktopCanvas.Public.inc#L392) | `mouseReleaseEvent` | 定义 | `void mouseReleaseEvent(QMouseEvent* event) override` | 实现 mouse release event 对应的业务或工具逻辑。 |
| [L440](../src/apps/controller/RemoteDesktopCanvas.Public.inc#L440) | `wheelEvent` | 定义 | `void wheelEvent(QWheelEvent* event) override` | 实现 wheel event 对应的业务或工具逻辑。 |
| [L470](../src/apps/controller/RemoteDesktopCanvas.Public.inc#L470) | `keyPressEvent` | 定义 | `void keyPressEvent(QKeyEvent* event) override` | 实现 key press event 对应的业务或工具逻辑。 |
| [L516](../src/apps/controller/RemoteDesktopCanvas.Public.inc#L516) | `keyReleaseEvent` | 定义 | `void keyReleaseEvent(QKeyEvent* event) override` | 实现 key release event 对应的业务或工具逻辑。 |
| [L538](../src/apps/controller/RemoteDesktopCanvas.Public.inc#L538) | `dragEnterEvent` | 定义 | `void dragEnterEvent(QDragEnterEvent* event) override` | 实现 drag enter event 对应的业务或工具逻辑。 |
| [L555](../src/apps/controller/RemoteDesktopCanvas.Public.inc#L555) | `dropEvent` | 定义 | `void dropEvent(QDropEvent* event) override` | 实现 drop event 对应的业务或工具逻辑。 |
| [L577](../src/apps/controller/RemoteDesktopCanvas.Public.inc#L577) | `focusOutEvent` | 定义 | `void focusOutEvent(QFocusEvent* event) override` | 实现 focus out event 对应的业务或工具逻辑。 |
| [L587](../src/apps/controller/RemoteDesktopCanvas.Public.inc#L587) | `resizeEvent` | 定义 | `void resizeEvent(QResizeEvent* event) override` | 实现 resize event 对应的业务或工具逻辑。 |
| [L599](../src/apps/controller/RemoteDesktopCanvas.Public.inc#L599) | `event` | 定义 | `bool event(QEvent* event) override` | 实现 event 对应的业务或工具逻辑。 |
| [L613](../src/apps/controller/RemoteDesktopCanvas.Public.inc#L613) | `paintEvent` | 定义 | `void paintEvent(QPaintEvent*) override` | 准备或呈现 paint event 相关逻辑。 |

## `src/apps/controller/RemoteDesktopCanvas.Rendering.inc`

[打开源码](../src/apps/controller/RemoteDesktopCanvas.Rendering.inc) · **文件作用：** `RemoteDesktopCanvas` 的实现切片，集中实现 rendering 相关逻辑；成员状态仍定义在所属头文件中。

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L4](../src/apps/controller/RemoteDesktopCanvas.Rendering.inc#L4) | `IsCpuNv12Image` | 定义 | `static bool IsCpuNv12Image(const QImage& image)` | SPDX-License-Identifier: GPL-3.0-only Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd) |
| [L11](../src/apps/controller/RemoteDesktopCanvas.Rendering.inc#L11) | `UpdateCpuCanvasMetrics` | 定义 | `void UpdateCpuCanvasMetrics()` | 更新或应用 update cpu canvas metrics 相关逻辑。 |
| [L21](../src/apps/controller/RemoteDesktopCanvas.Rendering.inc#L21) | `CpuQtOutputSize` | 定义 | `QSize CpuQtOutputSize(int sourceWidth, int sourceHeight) const` | 实现 cpu qt output size 对应的业务或工具逻辑。 |
| [L74](../src/apps/controller/RemoteDesktopCanvas.Rendering.inc#L74) | `QueueCpuFrame` | 定义 | `void QueueCpuFrame( webrtc::scoped_refptr<webrtc::VideoFrameBuffer> frame, bool recordArrival = true)` | 实现 queue cpu frame 对应的业务或工具逻辑。 |
| [L108](../src/apps/controller/RemoteDesktopCanvas.Rendering.inc#L108) | `CpuConversionLoop` | 定义 | `void CpuConversionLoop(std::stop_token stopToken)` | 实现 cpu conversion loop 对应的业务或工具逻辑。 |
| [L309](../src/apps/controller/RemoteDesktopCanvas.Rendering.inc#L309) | `RecycleCpuImage` | 定义 | `void RecycleCpuImage(QImage image)` | 实现 recycle cpu image 对应的业务或工具逻辑。 |
| [L318](../src/apps/controller/RemoteDesktopCanvas.Rendering.inc#L318) | `QueueConvertedCpuImage` | 定义 | `void QueueConvertedCpuImage(QImage image, std::uint64_t sequence)` | 实现 queue converted cpu image 对应的业务或工具逻辑。 |
| [L329](../src/apps/controller/RemoteDesktopCanvas.Rendering.inc#L329) | `QueueD3D11CpuImage` | 定义 | `void QueueD3D11CpuImage(QImage image, std::uint64_t sequence)` | 实现 queue d3 d11 cpu image 对应的业务或工具逻辑。 |
| [L372](../src/apps/controller/RemoteDesktopCanvas.Rendering.inc#L372) | `QueueQtCpuImage` | 定义 | `void QueueQtCpuImage(QImage image, std::uint64_t sequence)` | 实现 queue qt cpu image 对应的业务或工具逻辑。 |
| [L396](../src/apps/controller/RemoteDesktopCanvas.Rendering.inc#L396) | `ApplyPendingCpuImage` | 定义 | `void ApplyPendingCpuImage()` | 更新或应用 apply pending cpu image 相关逻辑。 |
| [L437](../src/apps/controller/RemoteDesktopCanvas.Rendering.inc#L437) | `QueueNativeFrame` | 定义 | `void QueueNativeFrame( webrtc::scoped_refptr<webrtc::VideoFrameBuffer> frame)` | 实现 queue native frame 对应的业务或工具逻辑。 |
| [L484](../src/apps/controller/RemoteDesktopCanvas.Rendering.inc#L484) | `QueueI420Frame` | 定义 | `void QueueI420Frame( webrtc::scoped_refptr<webrtc::VideoFrameBuffer> frame)` | 实现 queue i420 frame 对应的业务或工具逻辑。 |
| [L530](../src/apps/controller/RemoteDesktopCanvas.Rendering.inc#L530) | `ScheduleNativeUiUpdate` | 定义 | `void ScheduleNativeUiUpdate()` | 执行后台循环或调度 schedule native ui update 相关逻辑。 |
| [L568](../src/apps/controller/RemoteDesktopCanvas.Rendering.inc#L568) | `UpdateCpuConversionInterval` | 定义 | `void UpdateCpuConversionInterval()` | 更新或应用 update cpu conversion interval 相关逻辑。 |
| [L589](../src/apps/controller/RemoteDesktopCanvas.Rendering.inc#L589) | `RequestNativeRedraw` | 定义 | `void RequestNativeRedraw()` | 发起请求或查询 request native redraw 相关逻辑。 |
| [L602](../src/apps/controller/RemoteDesktopCanvas.Rendering.inc#L602) | `CursorVisualChanged` | 定义 | `void CursorVisualChanged()` | 实现 cursor visual changed 对应的业务或工具逻辑。 |
| [L611](../src/apps/controller/RemoteDesktopCanvas.Rendering.inc#L611) | `DeactivateNativePresentation` | 定义 | `bool DeactivateNativePresentation(std::uint64_t cpuSequence)` | 实现 deactivate native presentation 对应的业务或工具逻辑。 |
| [L637](../src/apps/controller/RemoteDesktopCanvas.Rendering.inc#L637) | `NativePresentationLoop` | 定义 | `void NativePresentationLoop(std::stop_token stopToken)` | 实现 native presentation loop 对应的业务或工具逻辑。 |

## `src/apps/controller/RemoteDesktopCanvas.State.inc`

[打开源码](../src/apps/controller/RemoteDesktopCanvas.State.inc) · **文件作用：** `RemoteDesktopCanvas` 的实现切片，集中实现 state 相关逻辑；成员状态仍定义在所属头文件中。

当前规则未提取到公开类型、成员/文件级变量或函数；该文件可能只包含宏、模板片段、资源数据或条件编译内容，请直接打开源码查看。

## `src/apps/controller/RemoteInputDispatcher.h`

[打开源码](../src/apps/controller/RemoteInputDispatcher.h) · **文件作用：** 声明 remote input dispatcher 相关类型、接口、配置和成员状态。

### 类型

| 行 | 类型 | 种类 | 作用 |
|---:|---|---|---|
| [L20](../src/apps/controller/RemoteInputDispatcher.h#L20) | `RemoteInputDispatcher` | class | 定义 RemoteInputDispatcher 的 class 类型和相关状态。 |
| [L59](../src/apps/controller/RemoteInputDispatcher.h#L59) | `HighResolutionPointerMoveScheduler` | class | 定义 HighResolutionPointerMoveScheduler 的 class 类型和相关状态。 |

### 成员与文件级变量

| 行 | 变量 | 声明 | 作用 |
|---:|---|---|---|
| [L53](../src/apps/controller/RemoteInputDispatcher.h#L53) | `mutex_` | `std::mutex mutex_;` | 保护跨线程共享状态：mutex。 |
| [L54](../src/apps/controller/RemoteInputDispatcher.h#L54) | `engine_` | `app::InProcessSessionEngine* engine_ = nullptr;` | 保存 engine 相关配置或运行状态。 |
| [L55](../src/apps/controller/RemoteInputDispatcher.h#L55) | `enabled_` | `bool enabled_ = false;` | 保存能力或开关状态：enabled。 |
| [L56](../src/apps/controller/RemoteInputDispatcher.h#L56) | `directSession_` | `bool directSession_ = false;` | 保存 direct session 相关配置或运行状态。 |
| [L476](../src/apps/controller/RemoteInputDispatcher.h#L476) | `sender_` | `InputSender sender_;` | 保存 sender 相关配置或运行状态。 |
| [L477](../src/apps/controller/RemoteInputDispatcher.h#L477) | `dragSampleProvider_` | `DragSampleProvider dragSampleProvider_;` | 保存 drag sample provider 相关配置或运行状态。 |
| [L478](../src/apps/controller/RemoteInputDispatcher.h#L478) | `wakeEvent_` | `HANDLE wakeEvent_ = nullptr;` | 保存 wake event 相关配置或运行状态。 |
| [L479](../src/apps/controller/RemoteInputDispatcher.h#L479) | `waitableTimer_` | `HANDLE waitableTimer_ = nullptr;` | 保存定时、截止或超时状态：waitable timer。 |
| [L480](../src/apps/controller/RemoteInputDispatcher.h#L480) | `worker_` | `std::jthread worker_;` | 拥有后台执行线程或工作器：worker。 |
| [L481](../src/apps/controller/RemoteInputDispatcher.h#L481) | `stateMutex_` | `std::mutex stateMutex_;` | 保护跨线程共享状态：state mutex。 |
| [L482](../src/apps/controller/RemoteInputDispatcher.h#L482) | `dispatchMutex_` | `std::mutex dispatchMutex_;` | 保护跨线程共享状态：dispatch mutex。 |
| [L483](../src/apps/controller/RemoteInputDispatcher.h#L483) | `pendingInput_` | `std::optional<RemoteInputEvent> pendingInput_;` | 保存 pending input 相关配置或运行状态。 |
| [L484](../src/apps/controller/RemoteInputDispatcher.h#L484) | `dragInput_` | `RemoteInputEvent dragInput_;` | 保存 drag input 相关配置或运行状态。 |
| [L485](../src/apps/controller/RemoteInputDispatcher.h#L485) | `pendingInterval_` | `std::chrono::microseconds pendingInterval_{8'333};` | 保存 pending interval 相关配置或运行状态。 |
| [L486](../src/apps/controller/RemoteInputDispatcher.h#L486) | `dragInterval_` | `std::chrono::microseconds dragInterval_{4'166};` | 保存 drag interval 相关配置或运行状态。 |
| [L487](../src/apps/controller/RemoteInputDispatcher.h#L487) | `lastDispatchAt_` | `std::chrono::steady_clock::time_point lastDispatchAt_;` | 保存 last dispatch at 相关配置或运行状态。 |
| [L488](../src/apps/controller/RemoteInputDispatcher.h#L488) | `nextDispatchAt_` | `std::chrono::steady_clock::time_point nextDispatchAt_;` | 保存 next dispatch at 相关配置或运行状态。 |
| [L489](../src/apps/controller/RemoteInputDispatcher.h#L489) | `lastDragX_` | `std::uint16_t lastDragX_ = 0;` | 保存 last drag x 相关配置或运行状态。 |
| [L490](../src/apps/controller/RemoteInputDispatcher.h#L490) | `lastDragY_` | `std::uint16_t lastDragY_ = 0;` | 保存 last drag y 相关配置或运行状态。 |
| [L491](../src/apps/controller/RemoteInputDispatcher.h#L491) | `hasCadence_` | `bool hasCadence_ = false;` | 保存 has cadence 相关配置或运行状态。 |
| [L492](../src/apps/controller/RemoteInputDispatcher.h#L492) | `dragActive_` | `bool dragActive_ = false;` | 保存能力或开关状态：drag active。 |
| [L493](../src/apps/controller/RemoteInputDispatcher.h#L493) | `lastDragPointValid_` | `bool lastDragPointValid_ = false;` | 保存身份或作用域标识：last drag point valid。 |
| [L494](../src/apps/controller/RemoteInputDispatcher.h#L494) | `stopping_` | `bool stopping_ = false;` | 保存 stopping 相关配置或运行状态。 |

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L22](../src/apps/controller/RemoteInputDispatcher.h#L22) | `SetEngine` | 定义 | `void SetEngine(app::InProcessSessionEngine* engine, bool directSession = false)` | 更新或应用 set engine 相关逻辑。 |
| [L30](../src/apps/controller/RemoteInputDispatcher.h#L30) | `SetEnabled` | 定义 | `void SetEnabled(bool enabled)` | 更新或应用 set enabled 相关逻辑。 |
| [L36](../src/apps/controller/RemoteInputDispatcher.h#L36) | `Clear` | 定义 | `void Clear()` | 重置或移除 clear 相关逻辑。 |
| [L44](../src/apps/controller/RemoteInputDispatcher.h#L44) | `Send` | 定义 | `bool Send(const RemoteInputEvent& event)` | 发送或发布 send 相关逻辑。 |
| [L67](../src/apps/controller/RemoteInputDispatcher.h#L67) | `HighResolutionPointerMoveScheduler` | 定义 | `explicit HighResolutionPointerMoveScheduler( InputSender sender, DragSampleProvider dragSampleProvider) : sender_(std::move(sender)), dragSampleProvider_( std::move(dragSampleProvider))` | 实现 high resolution pointer move scheduler 对应的业务或工具逻辑。 |
| [L95](../src/apps/controller/RemoteInputDispatcher.h#L95) | `~HighResolutionPointerMoveScheduler` | 定义 | `~HighResolutionPointerMoveScheduler()` | 停止相关活动并释放 HighResolutionPointerMoveScheduler 实例拥有的资源。 |
| [L100](../src/apps/controller/RemoteInputDispatcher.h#L100) | `HighResolutionPointerMoveScheduler` | 声明 | `HighResolutionPointerMoveScheduler( const HighResolutionPointerMoveScheduler&) = delete` | 实现 high resolution pointer move scheduler 对应的业务或工具逻辑。 |
| [L105](../src/apps/controller/RemoteInputDispatcher.h#L105) | `Queue` | 定义 | `bool Queue( const RemoteInputEvent& input, std::uint32_t rateLimitHz)` | 实现 queue 对应的业务或工具逻辑。 |
| [L166](../src/apps/controller/RemoteInputDispatcher.h#L166) | `BeginDrag` | 定义 | `void BeginDrag( const RemoteInputEvent& input, std::uint32_t sampleRateHz)` | 启动 begin drag 相关逻辑。 |
| [L199](../src/apps/controller/RemoteInputDispatcher.h#L199) | `SetActiveDragSampleRate` | 定义 | `void SetActiveDragSampleRate( std::uint32_t sampleRateHz)` | 更新或应用 set active drag sample rate 相关逻辑。 |
| [L224](../src/apps/controller/RemoteInputDispatcher.h#L224) | `EndDrag` | 定义 | `void EndDrag()` | 停止 end drag 相关逻辑。 |
| [L241](../src/apps/controller/RemoteInputDispatcher.h#L241) | `Flush` | 定义 | `bool Flush()` | 实现 flush 对应的业务或工具逻辑。 |
| [L282](../src/apps/controller/RemoteInputDispatcher.h#L282) | `Cancel` | 定义 | `void Cancel()` | 判断 cancel 相关逻辑。 |
| [L299](../src/apps/controller/RemoteInputDispatcher.h#L299) | `Shutdown` | 定义 | `void Shutdown()` | 关闭并清理 shutdown 相关逻辑。 |
| [L324](../src/apps/controller/RemoteInputDispatcher.h#L324) | `Dispatch` | 定义 | `bool Dispatch(const RemoteInputEvent& input)` | 接收并处理 dispatch 相关逻辑。 |
| [L331](../src/apps/controller/RemoteInputDispatcher.h#L331) | `SampleDragInput` | 定义 | `std::optional<RemoteInputEvent> SampleDragInput( const RemoteInputEvent& fallback) const` | 实现 sample drag input 对应的业务或工具逻辑。 |
| [L344](../src/apps/controller/RemoteInputDispatcher.h#L344) | `AcceptDragPoint` | 定义 | `bool AcceptDragPoint(const RemoteInputEvent& input)` | 处理并回复 accept drag point 相关逻辑。 |
| [L363](../src/apps/controller/RemoteInputDispatcher.h#L363) | `DispatchDueInput` | 定义 | `void DispatchDueInput()` | 接收并处理 dispatch due input 相关逻辑。 |
| [L408](../src/apps/controller/RemoteInputDispatcher.h#L408) | `ArmTimer` | 定义 | `bool ArmTimer( std::chrono::steady_clock::time_point deadline)` | 实现 arm timer 对应的业务或工具逻辑。 |
| [L435](../src/apps/controller/RemoteInputDispatcher.h#L435) | `Run` | 定义 | `void Run(std::stop_token stopToken)` | 执行后台循环或调度 run 相关逻辑。 |

## `src/apps/controller/RemoteSessionActionTile.inc`

[打开源码](../src/apps/controller/RemoteSessionActionTile.inc) · **文件作用：** `RemoteSessionActionTile` 的实现切片，集中实现 remote session action tile 相关逻辑；成员状态仍定义在所属头文件中。

### 类型

| 行 | 类型 | 种类 | 作用 |
|---:|---|---|---|
| [L4](../src/apps/controller/RemoteSessionActionTile.inc#L4) | `ActionTile` | class | SPDX-License-Identifier: GPL-3.0-only Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd) |

### 成员与文件级变量

| 行 | 变量 | 声明 | 作用 |
|---:|---|---|---|
| [L201](../src/apps/controller/RemoteSessionActionTile.inc#L201) | `kTileHeight` | `static constexpr int kTileHeight = 56;` | 定义 tile height 的编译期常量或产品边界。 |

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L6](../src/apps/controller/RemoteSessionActionTile.inc#L6) | `ActionTile` | 定义 | `ActionTile(const QString& icon, const QString& text, int fixedWidth, QWidget* parent = nullptr) : QPushButton(parent)` | 实现 action tile 对应的业务或工具逻辑。 |
| [L50](../src/apps/controller/RemoteSessionActionTile.inc#L50) | `SetIcon` | 定义 | `void SetIcon(const QString& icon)` | 更新或应用 set icon 相关逻辑。 |
| [L64](../src/apps/controller/RemoteSessionActionTile.inc#L64) | `SetText` | 定义 | `void SetText(const QString& text)` | 更新或应用 set text 相关逻辑。 |
| [L72](../src/apps/controller/RemoteSessionActionTile.inc#L72) | `SetTone` | 定义 | `void SetTone(const QString& tone)` | 更新或应用 set tone 相关逻辑。 |
| [L81](../src/apps/controller/RemoteSessionActionTile.inc#L81) | `SetInteractive` | 定义 | `void SetInteractive(bool interactive)` | 更新或应用 set interactive 相关逻辑。 |
| [L92](../src/apps/controller/RemoteSessionActionTile.inc#L92) | `SetSlashVisible` | 定义 | `void SetSlashVisible(bool visible)` | 更新或应用 set slash visible 相关逻辑。 |
| [L105](../src/apps/controller/RemoteSessionActionTile.inc#L105) | `enterEvent` | 定义 | `void enterEvent(QEnterEvent* event) override` | 实现 enter event 对应的业务或工具逻辑。 |
| [L114](../src/apps/controller/RemoteSessionActionTile.inc#L114) | `leaveEvent` | 定义 | `void leaveEvent(QEvent* event) override` | 实现 leave event 对应的业务或工具逻辑。 |
| [L122](../src/apps/controller/RemoteSessionActionTile.inc#L122) | `paintEvent` | 定义 | `void paintEvent(QPaintEvent* event) override` | 准备或呈现 paint event 相关逻辑。 |
| [L203](../src/apps/controller/RemoteSessionActionTile.inc#L203) | `ConfigureMorphPair` | 定义 | `void ConfigureMorphPair()` | 更新或应用 configure morph pair 相关逻辑。 |
| [L251](../src/apps/controller/RemoteSessionActionTile.inc#L251) | `StartMorphTransition` | 定义 | `void StartMorphTransition(bool target)` | 启动 start morph transition 相关逻辑。 |
| [L271](../src/apps/controller/RemoteSessionActionTile.inc#L271) | `StartIconTransition` | 定义 | `void StartIconTransition()` | 启动 start icon transition 相关逻辑。 |
| [L288](../src/apps/controller/RemoteSessionActionTile.inc#L288) | `ResolveIconResource` | 定义 | `QString ResolveIconResource() const` | 查询并返回 resolve icon resource 相关逻辑。 |
| [L328](../src/apps/controller/RemoteSessionActionTile.inc#L328) | `Repolish` | 定义 | `static void Repolish(QWidget* widget)` | 实现 repolish 对应的业务或工具逻辑。 |

## `src/apps/controller/RemoteSessionBinding.h`

[打开源码](../src/apps/controller/RemoteSessionBinding.h) · **文件作用：** 声明 remote session binding 相关类型、接口、配置和成员状态。

### 类型

| 行 | 类型 | 种类 | 作用 |
|---:|---|---|---|
| [L14](../src/apps/controller/RemoteSessionBinding.h#L14) | `RemoteSessionMode` | enum class | 定义 RemoteSessionMode 的 enum class 类型和相关状态。 |
| [L22](../src/apps/controller/RemoteSessionBinding.h#L22) | `RemoteSessionBinding` | struct | Immutable-by-convention UI routing context. Authorization origin explains how a session was admitted; mode determines how media and control are bound. Owned-device and verificat... |

### 成员与文件级变量

| 行 | 变量 | 声明 | 作用 |
|---:|---|---|---|
| [L23](../src/apps/controller/RemoteSessionBinding.h#L23) | `mode` | `RemoteSessionMode mode = RemoteSessionMode::kDirect;` | 保存 mode 相关配置或运行状态。 |
| [L24](../src/apps/controller/RemoteSessionBinding.h#L24) | `origin` | `SessionOrigin origin = SessionOrigin::kManualDeviceId;` | 保存 origin 相关配置或运行状态。 |
| [L25](../src/apps/controller/RemoteSessionBinding.h#L25) | `peerDeviceId` | `QString peerDeviceId;` | 保存身份或作用域标识：peer device id。 |
| [L26](../src/apps/controller/RemoteSessionBinding.h#L26) | `peerDeviceName` | `QString peerDeviceName;` | 保存路径、地址或显示名称：peer device name。 |
| [L27](../src/apps/controller/RemoteSessionBinding.h#L27) | `roomPairId` | `QString roomPairId;` | 保存身份或作用域标识：room pair id。 |

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L29](../src/apps/controller/RemoteSessionBinding.h#L29) | `Direct` | 定义 | `static RemoteSessionBinding Direct( QString deviceId, QString deviceName, SessionOrigin sessionOrigin)` | 实现 direct 对应的业务或工具逻辑。 |
| [L41](../src/apps/controller/RemoteSessionBinding.h#L41) | `Room` | 定义 | `static RemoteSessionBinding Room( QString deviceId, QString deviceName, QString pairId)` | 实现 room 对应的业务或工具逻辑。 |
| [L53](../src/apps/controller/RemoteSessionBinding.h#L53) | `IsDirect` | 定义 | `bool IsDirect() const { return mode == RemoteSessionMode::kDirect; }` | 判断 is direct 相关逻辑。 |
| [L54](../src/apps/controller/RemoteSessionBinding.h#L54) | `IsRoom` | 定义 | `bool IsRoom() const { return mode == RemoteSessionMode::kRoom; }` | 判断 is room 相关逻辑。 |
| [L55](../src/apps/controller/RemoteSessionBinding.h#L55) | `IsValid` | 定义 | `bool IsValid() const` | 判断 is valid 相关逻辑。 |
| [L62](../src/apps/controller/RemoteSessionBinding.h#L62) | `SameTransport` | 定义 | `bool SameTransport(const RemoteSessionBinding& other) const` | 判断 same transport 相关逻辑。 |
| [L69](../src/apps/controller/RemoteSessionBinding.h#L69) | `SourceText` | 定义 | `QString SourceText() const` | 实现 source text 对应的业务或工具逻辑。 |

## `src/apps/controller/RemoteSessionNetworkIndicator.inc`

[打开源码](../src/apps/controller/RemoteSessionNetworkIndicator.inc) · **文件作用：** `RemoteSessionNetworkIndicator` 的实现切片，集中实现 remote session network indicator 相关逻辑；成员状态仍定义在所属头文件中。

### 类型

| 行 | 类型 | 种类 | 作用 |
|---:|---|---|---|
| [L4](../src/apps/controller/RemoteSessionNetworkIndicator.inc#L4) | `NetworkSignalIndicator` | class | SPDX-License-Identifier: GPL-3.0-only Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd) |

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L6](../src/apps/controller/RemoteSessionNetworkIndicator.inc#L6) | `NetworkSignalIndicator` | 定义 | `explicit NetworkSignalIndicator(QWidget* parent = nullptr) : QWidget(parent)` | 实现 network signal indicator 对应的业务或工具逻辑。 |
| [L14](../src/apps/controller/RemoteSessionNetworkIndicator.inc#L14) | `paintEvent` | 定义 | `void paintEvent(QPaintEvent*) override` | 准备或呈现 paint event 相关逻辑。 |

## `src/apps/controller/RemoteSessionWindow.Controls.inc`

[打开源码](../src/apps/controller/RemoteSessionWindow.Controls.inc) · **文件作用：** `RemoteSessionWindow` 的实现切片，集中实现 controls 相关逻辑；成员状态仍定义在所属头文件中。

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L4](../src/apps/controller/RemoteSessionWindow.Controls.inc#L4) | `RemoteSessionWindow::HandleDisconnectAction` | 定义 | `void RemoteSessionWindow::HandleDisconnectAction()` | SPDX-License-Identifier: GPL-3.0-only Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd) |
| [L14](../src/apps/controller/RemoteSessionWindow.Controls.inc#L14) | `RemoteSessionWindow::closeEvent` | 定义 | `void RemoteSessionWindow::closeEvent(QCloseEvent* event)` | 关闭并清理 close event 相关逻辑。 |
| [L67](../src/apps/controller/RemoteSessionWindow.Controls.inc#L67) | `RemoteSessionWindow::UpdateSessionDuration` | 定义 | `void RemoteSessionWindow::UpdateSessionDuration()` | 更新或应用 update session duration 相关逻辑。 |
| [L81](../src/apps/controller/RemoteSessionWindow.Controls.inc#L81) | `RemoteSessionWindow::UpdateNetworkRecoveryPrompt` | 定义 | `void RemoteSessionWindow::UpdateNetworkRecoveryPrompt( bool recovering, std::uint32_t attempt, bool waitingForSignaling, bool failed)` | 更新或应用 update network recovery prompt 相关逻辑。 |
| [L190](../src/apps/controller/RemoteSessionWindow.Controls.inc#L190) | `RemoteSessionWindow::ShowRemotePasteProgress` | 定义 | `void RemoteSessionWindow::ShowRemotePasteProgress( const QString& transferId, const QString& title, const QString& message, double progress, std::uintptr_t localTargetWindow)` | 实现 show remote paste progress 对应的业务或工具逻辑。 |
| [L303](../src/apps/controller/RemoteSessionWindow.Controls.inc#L303) | `RemoteSessionWindow::CompleteRemotePasteProgress` | 定义 | `void RemoteSessionWindow::CompleteRemotePasteProgress( const QString& transferId)` | 实现 complete remote paste progress 对应的业务或工具逻辑。 |
| [L330](../src/apps/controller/RemoteSessionWindow.Controls.inc#L330) | `RemoteSessionWindow::CloseRemotePasteProgress` | 定义 | `void RemoteSessionWindow::CloseRemotePasteProgress( const QString& transferId)` | 关闭并清理 close remote paste progress 相关逻辑。 |
| [L359](../src/apps/controller/RemoteSessionWindow.Controls.inc#L359) | `RemoteSessionWindow::ShowRemotePasteFailure` | 定义 | `void RemoteSessionWindow::ShowRemotePasteFailure( const QString& transferId, const QString& message)` | 实现 show remote paste failure 对应的业务或工具逻辑。 |
| [L411](../src/apps/controller/RemoteSessionWindow.Controls.inc#L411) | `RemoteSessionWindow::SetRemotePasteStatusButtonVisible` | 定义 | `void RemoteSessionWindow::SetRemotePasteStatusButtonVisible(bool visible)` | 更新或应用 set remote paste status button visible 相关逻辑。 |
| [L426](../src/apps/controller/RemoteSessionWindow.Controls.inc#L426) | `RemoteSessionWindow::RemotePasteDialogRestoreGeometry` | 定义 | `QRect RemoteSessionWindow::RemotePasteDialogRestoreGeometry() const` | 实现 remote paste dialog restore geometry 对应的业务或工具逻辑。 |
| [L440](../src/apps/controller/RemoteSessionWindow.Controls.inc#L440) | `RemoteSessionWindow::CreateRemotePasteAnimationOverlay` | 定义 | `QLabel* RemoteSessionWindow::CreateRemotePasteAnimationOverlay( const QPixmap& snapshot, const QRect& geometry, qreal opacity)` | 创建或初始化 create remote paste animation overlay 相关逻辑。 |
| [L493](../src/apps/controller/RemoteSessionWindow.Controls.inc#L493) | `RemoteSessionWindow::MinimizeRemotePasteProgress` | 定义 | `void RemoteSessionWindow::MinimizeRemotePasteProgress()` | 实现 minimize remote paste progress 对应的业务或工具逻辑。 |
| [L605](../src/apps/controller/RemoteSessionWindow.Controls.inc#L605) | `RemoteSessionWindow::RestoreRemotePasteProgress` | 定义 | `void RemoteSessionWindow::RestoreRemotePasteProgress()` | 读取或恢复 restore remote paste progress 相关逻辑。 |
| [L711](../src/apps/controller/RemoteSessionWindow.Controls.inc#L711) | `RemoteSessionWindow::HandleControlAction` | 定义 | `void RemoteSessionWindow::HandleControlAction()` | 接收并处理 handle control action 相关逻辑。 |
| [L740](../src/apps/controller/RemoteSessionWindow.Controls.inc#L740) | `RemoteSessionWindow::RebuildFrameRateMenu` | 定义 | `void RemoteSessionWindow::RebuildFrameRateMenu()` | 更新或应用 rebuild frame rate menu 相关逻辑。 |
| [L782](../src/apps/controller/RemoteSessionWindow.Controls.inc#L782) | `RemoteSessionWindow::RebuildQualityMenu` | 定义 | `void RemoteSessionWindow::RebuildQualityMenu()` | 更新或应用 rebuild quality menu 相关逻辑。 |
| [L827](../src/apps/controller/RemoteSessionWindow.Controls.inc#L827) | `RemoteSessionWindow::HandleFrameRateSelection` | 定义 | `void RemoteSessionWindow::HandleFrameRateSelection( std::uint32_t framesPerSecond)` | 接收并处理 handle frame rate selection 相关逻辑。 |
| [L870](../src/apps/controller/RemoteSessionWindow.Controls.inc#L870) | `RemoteSessionWindow::HandleQualitySelection` | 定义 | `void RemoteSessionWindow::HandleQualitySelection(ScreenQualityTier quality)` | 接收并处理 handle quality selection 相关逻辑。 |
| [L902](../src/apps/controller/RemoteSessionWindow.Controls.inc#L902) | `RemoteSessionWindow::RequestStreamPreference` | 定义 | `bool RemoteSessionWindow::RequestStreamPreference(bool showError)` | 发起请求或查询 request stream preference 相关逻辑。 |
| [L931](../src/apps/controller/RemoteSessionWindow.Controls.inc#L931) | `RemoteSessionWindow::ToggleRemoteSound` | 定义 | `void RemoteSessionWindow::ToggleRemoteSound()` | 实现 toggle remote sound 对应的业务或工具逻辑。 |
| [L954](../src/apps/controller/RemoteSessionWindow.Controls.inc#L954) | `RemoteSessionWindow::ToggleLocalMicrophone` | 定义 | `void RemoteSessionWindow::ToggleLocalMicrophone()` | 实现 toggle local microphone 对应的业务或工具逻辑。 |

## `src/apps/controller/RemoteSessionWindow.cpp`

[打开源码](../src/apps/controller/RemoteSessionWindow.cpp) · **文件作用：** 实现 remote session window 相关函数与文件级辅助逻辑。

### 类型

| 行 | 类型 | 种类 | 作用 |
|---:|---|---|---|
| [L113](../src/apps/controller/RemoteSessionWindow.cpp#L113) | `MorphIconToolButton` | class | 定义 MorphIconToolButton 的 class 类型和相关状态。 |
| [L234](../src/apps/controller/RemoteSessionWindow.cpp#L234) | `LiquidTransferButton` | class | 定义 LiquidTransferButton 的 class 类型和相关状态。 |

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L103](../src/apps/controller/RemoteSessionWindow.cpp#L103) | `IsDirectRecoveryFailureCode` | 定义 | `bool IsDirectRecoveryFailureCode( const std::string& errorCode)` | 判断 is direct recovery failure code 相关逻辑。 |
| [L115](../src/apps/controller/RemoteSessionWindow.cpp#L115) | `MorphIconToolButton` | 定义 | `MorphIconToolButton(const QString& sourceResource, const QString& targetResource, QWidget* parent = nullptr) : QToolButton(parent), sourceResource_(sourceResource), targetResource_(targetResource)` | 实现 morph icon tool button 对应的业务或工具逻辑。 |
| [L147](../src/apps/controller/RemoteSessionWindow.cpp#L147) | `SetTarget` | 定义 | `void SetTarget(bool target)` | 更新或应用 set target 相关逻辑。 |
| [L170](../src/apps/controller/RemoteSessionWindow.cpp#L170) | `paintEvent` | 定义 | `void paintEvent(QPaintEvent* event) override` | 准备或呈现 paint event 相关逻辑。 |
| [L236](../src/apps/controller/RemoteSessionWindow.cpp#L236) | `LiquidTransferButton` | 定义 | `explicit LiquidTransferButton(QWidget* parent = nullptr) : QToolButton(parent)` | 实现 liquid transfer button 对应的业务或工具逻辑。 |
| [L252](../src/apps/controller/RemoteSessionWindow.cpp#L252) | `SetProgress` | 定义 | `void SetProgress(double progress)` | 更新或应用 set progress 相关逻辑。 |
| [L282](../src/apps/controller/RemoteSessionWindow.cpp#L282) | `paintEvent` | 定义 | `void paintEvent(QPaintEvent*) override` | 准备或呈现 paint event 相关逻辑。 |
| [L345](../src/apps/controller/RemoteSessionWindow.cpp#L345) | `SessionBitrateText` | 定义 | `QString SessionBitrateText(std::uint64_t bitsPerSecond)` | 实现 session bitrate text 对应的业务或工具逻辑。 |
| [L357](../src/apps/controller/RemoteSessionWindow.cpp#L357) | `SessionRouteText` | 定义 | `QString SessionRouteText(const std::string& route)` | 实现 session route text 对应的业务或工具逻辑。 |
| [L371](../src/apps/controller/RemoteSessionWindow.cpp#L371) | `AnimateSessionStatusWidget` | 定义 | `void AnimateSessionStatusWidget(QWidget* widget)` | 实现 animate session status widget 对应的业务或工具逻辑。 |

## `src/apps/controller/RemoteSessionWindow.Diagnostics.inc`

[打开源码](../src/apps/controller/RemoteSessionWindow.Diagnostics.inc) · **文件作用：** `RemoteSessionWindow` 的实现切片，集中实现 diagnostics 相关逻辑；成员状态仍定义在所属头文件中。

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L4](../src/apps/controller/RemoteSessionWindow.Diagnostics.inc#L4) | `RemoteSessionWindow::UpdateDiagnostics` | 定义 | `void RemoteSessionWindow::UpdateDiagnostics( const PeerConnectionDiagnosticsSnapshot& diagnostics)` | SPDX-License-Identifier: GPL-3.0-only Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd) |

## `src/apps/controller/RemoteSessionWindow.DisplayMenu.inc`

[打开源码](../src/apps/controller/RemoteSessionWindow.DisplayMenu.inc) · **文件作用：** `RemoteSessionWindow` 的实现切片，集中实现 display menu 相关逻辑；成员状态仍定义在所属头文件中。

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L4](../src/apps/controller/RemoteSessionWindow.DisplayMenu.inc#L4) | `RemoteSessionWindow::ShowRemoteDisplayMenu` | 定义 | `void RemoteSessionWindow::ShowRemoteDisplayMenu()` | SPDX-License-Identifier: GPL-3.0-only Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd) |

## `src/apps/controller/RemoteSessionWindow.h`

[打开源码](../src/apps/controller/RemoteSessionWindow.h) · **文件作用：** 声明 remote session window 相关类型、接口、配置和成员状态。

### 类型

| 行 | 类型 | 种类 | 作用 |
|---:|---|---|---|
| [L25](../src/apps/controller/RemoteSessionWindow.h#L25) | `QLabel` | class | 定义 QLabel 的 class 类型和相关状态。 |
| [L26](../src/apps/controller/RemoteSessionWindow.h#L26) | `QMenu` | class | 定义 QMenu 的 class 类型和相关状态。 |
| [L27](../src/apps/controller/RemoteSessionWindow.h#L27) | `QActionGroup` | class | 定义 QActionGroup 的 class 类型和相关状态。 |
| [L28](../src/apps/controller/RemoteSessionWindow.h#L28) | `QCloseEvent` | class | 定义 QCloseEvent 的 class 类型和相关状态。 |
| [L29](../src/apps/controller/RemoteSessionWindow.h#L29) | `QEvent` | class | 定义 QEvent 的 class 类型和相关状态。 |
| [L30](../src/apps/controller/RemoteSessionWindow.h#L30) | `QMoveEvent` | class | 定义 QMoveEvent 的 class 类型和相关状态。 |
| [L31](../src/apps/controller/RemoteSessionWindow.h#L31) | `QAbstractAnimation` | class | 定义 QAbstractAnimation 的 class 类型和相关状态。 |
| [L32](../src/apps/controller/RemoteSessionWindow.h#L32) | `QPropertyAnimation` | class | 定义 QPropertyAnimation 的 class 类型和相关状态。 |
| [L33](../src/apps/controller/RemoteSessionWindow.h#L33) | `QResizeEvent` | class | 定义 QResizeEvent 的 class 类型和相关状态。 |
| [L34](../src/apps/controller/RemoteSessionWindow.h#L34) | `QTimer` | class | 定义 QTimer 的 class 类型和相关状态。 |
| [L35](../src/apps/controller/RemoteSessionWindow.h#L35) | `QToolButton` | class | 定义 QToolButton 的 class 类型和相关状态。 |
| [L36](../src/apps/controller/RemoteSessionWindow.h#L36) | `QWidget` | class | 定义 QWidget 的 class 类型和相关状态。 |
| [L39](../src/apps/controller/RemoteSessionWindow.h#L39) | `InProcessSessionEngine` | class | 定义 InProcessSessionEngine 的 class 类型和相关状态。 |
| [L44](../src/apps/controller/RemoteSessionWindow.h#L44) | `ActionTile` | class | 定义 ActionTile 的 class 类型和相关状态。 |
| [L45](../src/apps/controller/RemoteSessionWindow.h#L45) | `RemoteCDialog` | class | 定义 RemoteCDialog 的 class 类型和相关状态。 |
| [L46](../src/apps/controller/RemoteSessionWindow.h#L46) | `RemoteInputDispatcher` | class | 定义 RemoteInputDispatcher 的 class 类型和相关状态。 |
| [L48](../src/apps/controller/RemoteSessionWindow.h#L48) | `RemoteSessionWindow` | class | 定义 RemoteSessionWindow 的 class 类型和相关状态。 |

### 成员与文件级变量

| 行 | 变量 | 声明 | 作用 |
|---:|---|---|---|
| [L25](../src/apps/controller/RemoteSessionWindow.h#L25) | `QLabel` | `class QLabel;` | 保存路径、地址或显示名称：q label。 |
| [L26](../src/apps/controller/RemoteSessionWindow.h#L26) | `QMenu` | `class QMenu;` | 保存 q menu 相关配置或运行状态。 |
| [L27](../src/apps/controller/RemoteSessionWindow.h#L27) | `QActionGroup` | `class QActionGroup;` | 保存 q action group 相关配置或运行状态。 |
| [L28](../src/apps/controller/RemoteSessionWindow.h#L28) | `QCloseEvent` | `class QCloseEvent;` | 保存 q close event 相关配置或运行状态。 |
| [L29](../src/apps/controller/RemoteSessionWindow.h#L29) | `QEvent` | `class QEvent;` | 保存 q event 相关配置或运行状态。 |
| [L30](../src/apps/controller/RemoteSessionWindow.h#L30) | `QMoveEvent` | `class QMoveEvent;` | 保存 q move event 相关配置或运行状态。 |
| [L31](../src/apps/controller/RemoteSessionWindow.h#L31) | `QAbstractAnimation` | `class QAbstractAnimation;` | 保存 q abstract animation 相关配置或运行状态。 |
| [L32](../src/apps/controller/RemoteSessionWindow.h#L32) | `QPropertyAnimation` | `class QPropertyAnimation;` | 保存 q property animation 相关配置或运行状态。 |
| [L33](../src/apps/controller/RemoteSessionWindow.h#L33) | `QResizeEvent` | `class QResizeEvent;` | 保存 q resize event 相关配置或运行状态。 |
| [L34](../src/apps/controller/RemoteSessionWindow.h#L34) | `QTimer` | `class QTimer;` | 保存定时、截止或超时状态：q timer。 |
| [L35](../src/apps/controller/RemoteSessionWindow.h#L35) | `QToolButton` | `class QToolButton;` | 保存 q tool button 相关配置或运行状态。 |
| [L36](../src/apps/controller/RemoteSessionWindow.h#L36) | `QWidget` | `class QWidget;` | 保存 q widget 相关配置或运行状态。 |
| [L39](../src/apps/controller/RemoteSessionWindow.h#L39) | `InProcessSessionEngine` | `class InProcessSessionEngine;` | 保存 in process session engine 相关配置或运行状态。 |
| [L44](../src/apps/controller/RemoteSessionWindow.h#L44) | `ActionTile` | `class ActionTile;` | 保存 action tile 相关配置或运行状态。 |
| [L45](../src/apps/controller/RemoteSessionWindow.h#L45) | `RemoteCDialog` | `class RemoteCDialog;` | 保存 remote c dialog 相关配置或运行状态。 |
| [L46](../src/apps/controller/RemoteSessionWindow.h#L46) | `RemoteInputDispatcher` | `class RemoteInputDispatcher;` | 保存 remote input dispatcher 相关配置或运行状态。 |
| [L137](../src/apps/controller/RemoteSessionWindow.h#L137) | `remoteInputDispatcher_` | `remoteInputDispatcher_;` | 保存 remote input dispatcher 相关配置或运行状态。 |
| [L138](../src/apps/controller/RemoteSessionWindow.h#L138) | `binding_` | `RemoteSessionBinding binding_;` | 保存 binding 相关配置或运行状态。 |
| [L139](../src/apps/controller/RemoteSessionWindow.h#L139) | `sessionSourceLabel_` | `QLabel* sessionSourceLabel_ = nullptr;` | 保存路径、地址或显示名称：session source label。 |
| [L140](../src/apps/controller/RemoteSessionWindow.h#L140) | `durationLabel_` | `QLabel* durationLabel_ = nullptr;` | 保存路径、地址或显示名称：duration label。 |
| [L141](../src/apps/controller/RemoteSessionWindow.h#L141) | `previewBadge_` | `QLabel* previewBadge_ = nullptr;` | 保存 preview badge 相关配置或运行状态。 |
| [L142](../src/apps/controller/RemoteSessionWindow.h#L142) | `controlBadge_` | `ActionTile* controlBadge_ = nullptr;` | 保存 control badge 相关配置或运行状态。 |
| [L143](../src/apps/controller/RemoteSessionWindow.h#L143) | `connectionStatusLabel_` | `QLabel* connectionStatusLabel_ = nullptr;` | 保存路径、地址或显示名称：connection status label。 |
| [L144](../src/apps/controller/RemoteSessionWindow.h#L144) | `networkSignalIndicator_` | `QWidget* networkSignalIndicator_ = nullptr;` | 保存 network signal indicator 相关配置或运行状态。 |
| [L145](../src/apps/controller/RemoteSessionWindow.h#L145) | `latencyLabel_` | `QLabel* latencyLabel_ = nullptr;` | 保存路径、地址或显示名称：latency label。 |
| [L146](../src/apps/controller/RemoteSessionWindow.h#L146) | `codecLabel_` | `QLabel* codecLabel_ = nullptr;` | 保存路径、地址或显示名称：codec label。 |
| [L147](../src/apps/controller/RemoteSessionWindow.h#L147) | `resolutionLabel_` | `QLabel* resolutionLabel_ = nullptr;` | 保存路径、地址或显示名称：resolution label。 |
| [L148](../src/apps/controller/RemoteSessionWindow.h#L148) | `controlButton_` | `ActionTile* controlButton_ = nullptr;` | 保存 control button 相关配置或运行状态。 |
| [L149](../src/apps/controller/RemoteSessionWindow.h#L149) | `qualityButton_` | `QToolButton* qualityButton_ = nullptr;` | 保存 quality button 相关配置或运行状态。 |
| [L150](../src/apps/controller/RemoteSessionWindow.h#L150) | `qualityMenu_` | `QMenu* qualityMenu_ = nullptr;` | 保存 quality menu 相关配置或运行状态。 |
| [L151](../src/apps/controller/RemoteSessionWindow.h#L151) | `qualityGroup_` | `QActionGroup* qualityGroup_ = nullptr;` | 保存 quality group 相关配置或运行状态。 |
| [L152](../src/apps/controller/RemoteSessionWindow.h#L152) | `frameRateButton_` | `QToolButton* frameRateButton_ = nullptr;` | 保存 frame rate button 相关配置或运行状态。 |
| [L153](../src/apps/controller/RemoteSessionWindow.h#L153) | `frameRateMenu_` | `QMenu* frameRateMenu_ = nullptr;` | 保存 frame rate menu 相关配置或运行状态。 |
| [L154](../src/apps/controller/RemoteSessionWindow.h#L154) | `frameRateGroup_` | `QActionGroup* frameRateGroup_ = nullptr;` | 保存 frame rate group 相关配置或运行状态。 |
| [L155](../src/apps/controller/RemoteSessionWindow.h#L155) | `speakerButton_` | `ActionTile* speakerButton_ = nullptr;` | 保存 speaker button 相关配置或运行状态。 |
| [L156](../src/apps/controller/RemoteSessionWindow.h#L156) | `microphoneButton_` | `ActionTile* microphoneButton_ = nullptr;` | 保存 microphone button 相关配置或运行状态。 |
| [L157](../src/apps/controller/RemoteSessionWindow.h#L157) | `fileTransferButton_` | `ActionTile* fileTransferButton_ = nullptr;` | 保存 file transfer button 相关配置或运行状态。 |
| [L158](../src/apps/controller/RemoteSessionWindow.h#L158) | `mediaDeviceButton_` | `QToolButton* mediaDeviceButton_ = nullptr;` | 保存 media device button 相关配置或运行状态。 |
| [L159](../src/apps/controller/RemoteSessionWindow.h#L159) | `remoteDisplayButton_` | `QToolButton* remoteDisplayButton_ = nullptr;` | 保存 remote display button 相关配置或运行状态。 |
| [L160](../src/apps/controller/RemoteSessionWindow.h#L160) | `fullScreenButton_` | `ActionTile* fullScreenButton_ = nullptr;` | 保存 full screen button 相关配置或运行状态。 |
| [L161](../src/apps/controller/RemoteSessionWindow.h#L161) | `toolbarLockButton_` | `QToolButton* toolbarLockButton_ = nullptr;` | 保存 toolbar lock button 相关配置或运行状态。 |
| [L162](../src/apps/controller/RemoteSessionWindow.h#L162) | `remotePasteStatusButton_` | `QToolButton* remotePasteStatusButton_ = nullptr;` | 保存 remote paste status button 相关配置或运行状态。 |
| [L163](../src/apps/controller/RemoteSessionWindow.h#L163) | `hudFrameRateLabel_` | `QLabel* hudFrameRateLabel_ = nullptr;` | 保存路径、地址或显示名称：hud frame rate label。 |
| [L164](../src/apps/controller/RemoteSessionWindow.h#L164) | `sessionTitleBar_` | `CustomTitleBar* sessionTitleBar_ = nullptr;` | 保存 session title bar 相关配置或运行状态。 |
| [L165](../src/apps/controller/RemoteSessionWindow.h#L165) | `contentHost_` | `QWidget* contentHost_ = nullptr;` | 保存 content host 相关配置或运行状态。 |
| [L166](../src/apps/controller/RemoteSessionWindow.h#L166) | `toolbarRevealZone_` | `QWidget* toolbarRevealZone_ = nullptr;` | 保存 toolbar reveal zone 相关配置或运行状态。 |
| [L167](../src/apps/controller/RemoteSessionWindow.h#L167) | `sessionToolbar_` | `QWidget* sessionToolbar_ = nullptr;` | 保存 session toolbar 相关配置或运行状态。 |
| [L168](../src/apps/controller/RemoteSessionWindow.h#L168) | `sessionHud_` | `QWidget* sessionHud_ = nullptr;` | 保存 session hud 相关配置或运行状态。 |
| [L169](../src/apps/controller/RemoteSessionWindow.h#L169) | `remotePasteStatusHost_` | `QWidget* remotePasteStatusHost_ = nullptr;` | 保存 remote paste status host 相关配置或运行状态。 |
| [L170](../src/apps/controller/RemoteSessionWindow.h#L170) | `desktopCanvas_` | `QWidget* desktopCanvas_ = nullptr;` | 保存 desktop canvas 相关配置或运行状态。 |
| [L171](../src/apps/controller/RemoteSessionWindow.h#L171) | `toolbarHideTimer_` | `QTimer* toolbarHideTimer_ = nullptr;` | 保存定时、截止或超时状态：toolbar hide timer。 |
| [L172](../src/apps/controller/RemoteSessionWindow.h#L172) | `toolbarAnimation_` | `QPropertyAnimation* toolbarAnimation_ = nullptr;` | 保存 toolbar animation 相关配置或运行状态。 |
| [L173](../src/apps/controller/RemoteSessionWindow.h#L173) | `sessionElapsed_` | `QElapsedTimer sessionElapsed_;` | 保存 session elapsed 相关配置或运行状态。 |
| [L174](../src/apps/controller/RemoteSessionWindow.h#L174) | `screenStartupElapsed_` | `QElapsedTimer screenStartupElapsed_;` | 保存 screen startup elapsed 相关配置或运行状态。 |
| [L175](../src/apps/controller/RemoteSessionWindow.h#L175) | `durationTimer_` | `QTimer* durationTimer_ = nullptr;` | 保存定时、截止或超时状态：duration timer。 |
| [L176](../src/apps/controller/RemoteSessionWindow.h#L176) | `sessionEngine_` | `app::InProcessSessionEngine* sessionEngine_ = nullptr;` | 保存 session engine 相关配置或运行状态。 |
| [L177](../src/apps/controller/RemoteSessionWindow.h#L177) | `sessionVideoSinkBound_` | `bool sessionVideoSinkBound_ = false;` | 保存 session video sink bound 相关配置或运行状态。 |
| [L178](../src/apps/controller/RemoteSessionWindow.h#L178) | `sessionVideoSinkRetryScheduled_` | `bool sessionVideoSinkRetryScheduled_ = false;` | 保存 session video sink retry scheduled 相关配置或运行状态。 |
| [L179](../src/apps/controller/RemoteSessionWindow.h#L179) | `roomScreenPreferenceRetryScheduled_` | `bool roomScreenPreferenceRetryScheduled_ = false;` | 保存 room screen preference retry scheduled 相关配置或运行状态。 |
| [L180](../src/apps/controller/RemoteSessionWindow.h#L180) | `screenStartupGeneration_` | `std::uint64_t screenStartupGeneration_ = 0;` | 标记当前世代，用于拒绝过期异步结果：screen startup generation。 |
| [L181](../src/apps/controller/RemoteSessionWindow.h#L181) | `screenStartupRefreshAttempts_` | `std::uint32_t screenStartupRefreshAttempts_ = 0;` | 保存 screen startup refresh attempts 相关配置或运行状态。 |
| [L182](../src/apps/controller/RemoteSessionWindow.h#L182) | `screenFirstFramePresented_` | `bool screenFirstFramePresented_ = false;` | 保存 screen first frame presented 相关配置或运行状态。 |
| [L183](../src/apps/controller/RemoteSessionWindow.h#L183) | `preferenceRequestedScreenShareEpoch_` | `std::uint64_t preferenceRequestedScreenShareEpoch_ = 0;` | 标记当前世代，用于拒绝过期异步结果：preference requested screen share epoch。 |
| [L184](../src/apps/controller/RemoteSessionWindow.h#L184) | `preferenceSentScreenShareEpoch_` | `std::uint64_t preferenceSentScreenShareEpoch_ = 0;` | 标记当前世代，用于拒绝过期异步结果：preference sent screen share epoch。 |
| [L185](../src/apps/controller/RemoteSessionWindow.h#L185) | `pendingRemoteDisplaySwitchSequence_` | `std::uint64_t pendingRemoteDisplaySwitchSequence_ = 0;` | 保存单调序号，用于排序或去重：pending remote display switch sequence。 |
| [L186](../src/apps/controller/RemoteSessionWindow.h#L186) | `lastControlActionText_` | `QString lastControlActionText_;` | 保存 last control action text 相关配置或运行状态。 |
| [L187](../src/apps/controller/RemoteSessionWindow.h#L187) | `inputControlEnabled_` | `bool inputControlEnabled_ = false;` | 保存能力或开关状态：input control enabled。 |
| [L188](../src/apps/controller/RemoteSessionWindow.h#L188) | `closeWithoutConfirmation_` | `bool closeWithoutConfirmation_ = false;` | 保存 close without confirmation 相关配置或运行状态。 |
| [L189](../src/apps/controller/RemoteSessionWindow.h#L189) | `closeConfirmationVisible_` | `bool closeConfirmationVisible_ = false;` | 保存 close confirmation visible 相关配置或运行状态。 |
| [L190](../src/apps/controller/RemoteSessionWindow.h#L190) | `networkRecoveryDialog_` | `RemoteCDialog* networkRecoveryDialog_ = nullptr;` | 保存 network recovery dialog 相关配置或运行状态。 |
| [L191](../src/apps/controller/RemoteSessionWindow.h#L191) | `remotePasteDialog_` | `RemoteCDialog* remotePasteDialog_ = nullptr;` | 保存 remote paste dialog 相关配置或运行状态。 |
| [L192](../src/apps/controller/RemoteSessionWindow.h#L192) | `remotePasteTransferId_` | `QString remotePasteTransferId_;` | 保存身份或作用域标识：remote paste transfer id。 |
| [L193](../src/apps/controller/RemoteSessionWindow.h#L193) | `remotePastePromptDismissed_` | `bool remotePastePromptDismissed_ = false;` | 保存 remote paste prompt dismissed 相关配置或运行状态。 |
| [L194](../src/apps/controller/RemoteSessionWindow.h#L194) | `remotePasteDialogClosing_` | `bool remotePasteDialogClosing_ = false;` | 保存 remote paste dialog closing 相关配置或运行状态。 |
| [L195](../src/apps/controller/RemoteSessionWindow.h#L195) | `remotePasteMinimized_` | `bool remotePasteMinimized_ = false;` | 保存 remote paste minimized 相关配置或运行状态。 |
| [L196](../src/apps/controller/RemoteSessionWindow.h#L196) | `remotePasteRestoreAnimating_` | `bool remotePasteRestoreAnimating_ = false;` | 保存 remote paste restore animating 相关配置或运行状态。 |
| [L197](../src/apps/controller/RemoteSessionWindow.h#L197) | `remotePasteRestoreGeometry_` | `QRect remotePasteRestoreGeometry_;` | 保存 remote paste restore geometry 相关配置或运行状态。 |
| [L198](../src/apps/controller/RemoteSessionWindow.h#L198) | `remotePasteAnimation_` | `QAbstractAnimation* remotePasteAnimation_ = nullptr;` | 保存 remote paste animation 相关配置或运行状态。 |
| [L199](../src/apps/controller/RemoteSessionWindow.h#L199) | `remotePasteAnimationOverlay_` | `QWidget* remotePasteAnimationOverlay_ = nullptr;` | 保存 remote paste animation overlay 相关配置或运行状态。 |
| [L200](../src/apps/controller/RemoteSessionWindow.h#L200) | `remotePasteDialogSnapshot_` | `QPixmap remotePasteDialogSnapshot_;` | 保存可跨层读取的状态快照：remote paste dialog snapshot。 |
| [L201](../src/apps/controller/RemoteSessionWindow.h#L201) | `pairWasActive_` | `bool pairWasActive_ = false;` | 保存能力或开关状态：pair was active。 |
| [L202](../src/apps/controller/RemoteSessionWindow.h#L202) | `recoveryPromptWasVisible_` | `bool recoveryPromptWasVisible_ = false;` | 保存 recovery prompt was visible 相关配置或运行状态。 |
| [L203](../src/apps/controller/RemoteSessionWindow.h#L203) | `networkRecoveryPromptDismissed_` | `bool networkRecoveryPromptDismissed_ = false;` | 保存 network recovery prompt dismissed 相关配置或运行状态。 |
| [L204](../src/apps/controller/RemoteSessionWindow.h#L204) | `networkRecoveryFailureDismissed_` | `bool networkRecoveryFailureDismissed_ = false;` | 保存 network recovery failure dismissed 相关配置或运行状态。 |
| [L205](../src/apps/controller/RemoteSessionWindow.h#L205) | `networkRecoveryShowingFailure_` | `bool networkRecoveryShowingFailure_ = false;` | 保存最近错误或失败原因：network recovery showing failure。 |
| [L206](../src/apps/controller/RemoteSessionWindow.h#L206) | `selectedFrameRate_` | `std::uint32_t selectedFrameRate_ = kDefaultScreenFrameRate;` | 保存计数、尺寸或速率指标：selected frame rate。 |
| [L207](../src/apps/controller/RemoteSessionWindow.h#L207) | `dragPointerSampleRateHz_` | `std::uint32_t dragPointerSampleRateHz_ = 240;` | 保存 drag pointer sample rate hz 相关配置或运行状态。 |
| [L208](../src/apps/controller/RemoteSessionWindow.h#L208) | `remoteMaximumFrameRate_` | `std::uint32_t remoteMaximumFrameRate_ = 120;` | 保存计数、尺寸或速率指标：remote maximum frame rate。 |
| [L209](../src/apps/controller/RemoteSessionWindow.h#L209) | `reportedRemoteMaximumFrameRate_` | `std::uint32_t reportedRemoteMaximumFrameRate_ = 120;` | 保存计数、尺寸或速率指标：reported remote maximum frame rate。 |
| [L210](../src/apps/controller/RemoteSessionWindow.h#L210) | `roomMaximumFrameRate_` | `std::uint32_t roomMaximumFrameRate_ = 120;` | 保存计数、尺寸或速率指标：room maximum frame rate。 |
| [L211](../src/apps/controller/RemoteSessionWindow.h#L211) | `remoteSourceWidth_` | `std::uint32_t remoteSourceWidth_ = 0;` | 保存计数、尺寸或速率指标：remote source width。 |
| [L212](../src/apps/controller/RemoteSessionWindow.h#L212) | `remoteSourceHeight_` | `std::uint32_t remoteSourceHeight_ = 0;` | 保存计数、尺寸或速率指标：remote source height。 |
| [L213](../src/apps/controller/RemoteSessionWindow.h#L213) | `selectedQuality_` | `ScreenQualityTier selectedQuality_ = ScreenQualityTier::kOriginal;` | 保存 selected quality 相关配置或运行状态。 |
| [L214](../src/apps/controller/RemoteSessionWindow.h#L214) | `toolbarShown_` | `bool toolbarShown_ = true;` | 保存 toolbar shown 相关配置或运行状态。 |
| [L215](../src/apps/controller/RemoteSessionWindow.h#L215) | `toolbarLocked_` | `bool toolbarLocked_ = false;` | 保存 toolbar locked 相关配置或运行状态。 |
| [L216](../src/apps/controller/RemoteSessionWindow.h#L216) | `fullScreenMode_` | `bool fullScreenMode_ = false;` | 保存 full screen mode 相关配置或运行状态。 |
| [L217](../src/apps/controller/RemoteSessionWindow.h#L217) | `mediaDeviceMenuOpen_` | `bool mediaDeviceMenuOpen_ = false;` | 保存能力或开关状态：media device menu open。 |
| [L218](../src/apps/controller/RemoteSessionWindow.h#L218) | `pendingCameraDeviceId_` | `QString pendingCameraDeviceId_;` | 保存身份或作用域标识：pending camera device id。 |
| [L219](../src/apps/controller/RemoteSessionWindow.h#L219) | `pendingMicrophoneDeviceId_` | `QString pendingMicrophoneDeviceId_;` | 保存身份或作用域标识：pending microphone device id。 |
| [L220](../src/apps/controller/RemoteSessionWindow.h#L220) | `pendingSpeakerDeviceId_` | `QString pendingSpeakerDeviceId_;` | 保存身份或作用域标识：pending speaker device id。 |

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L50](../src/apps/controller/RemoteSessionWindow.h#L50) | `RemoteSessionWindow` | 声明 | `RemoteSessionWindow(RemoteSessionBinding binding, app::InProcessSessionEngine* sessionEngine = nullptr, QWidget* parent = nullptr)` | 实现 remote session window 对应的业务或工具逻辑。 |
| [L53](../src/apps/controller/RemoteSessionWindow.h#L53) | `~RemoteSessionWindow` | 声明 | `~RemoteSessionWindow() override` | 停止相关活动并释放 RemoteSessionWindow 实例拥有的资源。 |
| [L55](../src/apps/controller/RemoteSessionWindow.h#L55) | `RemoteSessionWindow` | 声明 | `RemoteSessionWindow(const RemoteSessionWindow&) = delete` | 实现 remote session window 对应的业务或工具逻辑。 |
| [L58](../src/apps/controller/RemoteSessionWindow.h#L58) | `BindSessionVideo` | 声明 | `void BindSessionVideo(app::InProcessSessionEngine* engine, RemoteSessionBinding binding)` | 实现 bind session video 对应的业务或工具逻辑。 |
| [L60](../src/apps/controller/RemoteSessionWindow.h#L60) | `RefreshControlState` | 声明 | `void RefreshControlState()` | 刷新 refresh control state 相关逻辑。 |
| [L61](../src/apps/controller/RemoteSessionWindow.h#L61) | `UpdateDiagnostics` | 声明 | `void UpdateDiagnostics( const PeerConnectionDiagnosticsSnapshot& diagnostics)` | 更新或应用 update diagnostics 相关逻辑。 |
| [L63](../src/apps/controller/RemoteSessionWindow.h#L63) | `SetDisconnectHandler` | 声明 | `void SetDisconnectHandler(std::function<void()> handler)` | 更新或应用 set disconnect handler 相关逻辑。 |
| [L64](../src/apps/controller/RemoteSessionWindow.h#L64) | `SetRemotePasteHandler` | 声明 | `void SetRemotePasteHandler( std::function<bool(const QStringList& localFiles, bool keyboardPaste)> handler)` | 更新或应用 set remote paste handler 相关逻辑。 |
| [L67](../src/apps/controller/RemoteSessionWindow.h#L67) | `SetRemotePasteCancelHandler` | 声明 | `void SetRemotePasteCancelHandler(std::function<void()> handler)` | 更新或应用 set remote paste cancel handler 相关逻辑。 |
| [L68](../src/apps/controller/RemoteSessionWindow.h#L68) | `SetFileTransferHandlers` | 声明 | `void SetFileTransferHandlers( std::function<void()> openHandler, std::function<void()> releaseHostHandler)` | 更新或应用 set file transfer handlers 相关逻辑。 |
| [L71](../src/apps/controller/RemoteSessionWindow.h#L71) | `ShowRemotePasteProgress` | 声明 | `void ShowRemotePasteProgress( const QString& transferId, const QString& title, const QString& message, double progress, std::uintptr_t localTargetWindow = 0)` | 实现 show remote paste progress 对应的业务或工具逻辑。 |
| [L75](../src/apps/controller/RemoteSessionWindow.h#L75) | `CompleteRemotePasteProgress` | 声明 | `void CompleteRemotePasteProgress(const QString& transferId)` | 实现 complete remote paste progress 对应的业务或工具逻辑。 |
| [L76](../src/apps/controller/RemoteSessionWindow.h#L76) | `CloseRemotePasteProgress` | 声明 | `void CloseRemotePasteProgress(const QString& transferId)` | 关闭并清理 close remote paste progress 相关逻辑。 |
| [L77](../src/apps/controller/RemoteSessionWindow.h#L77) | `ShowRemotePasteFailure` | 声明 | `void ShowRemotePasteFailure( const QString& transferId, const QString& message)` | 实现 show remote paste failure 对应的业务或工具逻辑。 |
| [L79](../src/apps/controller/RemoteSessionWindow.h#L79) | `SetDragPointerSampleRate` | 声明 | `void SetDragPointerSampleRate(std::uint32_t hertz)` | 更新或应用 set drag pointer sample rate 相关逻辑。 |
| [L80](../src/apps/controller/RemoteSessionWindow.h#L80) | `SetRoomOnlineMemberCount` | 声明 | `void SetRoomOnlineMemberCount(std::size_t onlineMemberCount)` | 更新或应用 set room online member count 相关逻辑。 |
| [L83](../src/apps/controller/RemoteSessionWindow.h#L83) | `eventFilter` | 声明 | `bool eventFilter(QObject* watched, QEvent* event) override` | 实现 event filter 对应的业务或工具逻辑。 |
| [L84](../src/apps/controller/RemoteSessionWindow.h#L84) | `changeEvent` | 声明 | `void changeEvent(QEvent* event) override` | 实现 change event 对应的业务或工具逻辑。 |
| [L85](../src/apps/controller/RemoteSessionWindow.h#L85) | `closeEvent` | 声明 | `void closeEvent(QCloseEvent* event) override` | 关闭并清理 close event 相关逻辑。 |
| [L86](../src/apps/controller/RemoteSessionWindow.h#L86) | `moveEvent` | 声明 | `void moveEvent(QMoveEvent* event) override` | 实现 move event 对应的业务或工具逻辑。 |
| [L87](../src/apps/controller/RemoteSessionWindow.h#L87) | `resizeEvent` | 声明 | `void resizeEvent(QResizeEvent* event) override` | 实现 resize event 对应的业务或工具逻辑。 |
| [L90](../src/apps/controller/RemoteSessionWindow.h#L90) | `BuildUi` | 声明 | `void BuildUi()` | 创建或初始化 build ui 相关逻辑。 |
| [L91](../src/apps/controller/RemoteSessionWindow.h#L91) | `LayoutSessionOverlays` | 声明 | `void LayoutSessionOverlays()` | 实现 layout session overlays 对应的业务或工具逻辑。 |
| [L92](../src/apps/controller/RemoteSessionWindow.h#L92) | `ShowSessionToolbar` | 声明 | `void ShowSessionToolbar(bool animated = true)` | 实现 show session toolbar 对应的业务或工具逻辑。 |
| [L93](../src/apps/controller/RemoteSessionWindow.h#L93) | `HideSessionToolbar` | 声明 | `void HideSessionToolbar(bool animated = true)` | 实现 hide session toolbar 对应的业务或工具逻辑。 |
| [L94](../src/apps/controller/RemoteSessionWindow.h#L94) | `ScheduleSessionToolbarHide` | 声明 | `void ScheduleSessionToolbarHide()` | 执行后台循环或调度 schedule session toolbar hide 相关逻辑。 |
| [L95](../src/apps/controller/RemoteSessionWindow.h#L95) | `UpdateSessionDuration` | 声明 | `void UpdateSessionDuration()` | 更新或应用 update session duration 相关逻辑。 |
| [L96](../src/apps/controller/RemoteSessionWindow.h#L96) | `UpdateNetworkRecoveryPrompt` | 声明 | `void UpdateNetworkRecoveryPrompt( bool recovering, std::uint32_t attempt, bool waitingForSignaling, bool failed = false)` | 更新或应用 update network recovery prompt 相关逻辑。 |
| [L101](../src/apps/controller/RemoteSessionWindow.h#L101) | `HandleControlAction` | 声明 | `void HandleControlAction()` | 接收并处理 handle control action 相关逻辑。 |
| [L102](../src/apps/controller/RemoteSessionWindow.h#L102) | `RebuildQualityMenu` | 声明 | `void RebuildQualityMenu()` | 更新或应用 rebuild quality menu 相关逻辑。 |
| [L103](../src/apps/controller/RemoteSessionWindow.h#L103) | `RebuildFrameRateMenu` | 声明 | `void RebuildFrameRateMenu()` | 更新或应用 rebuild frame rate menu 相关逻辑。 |
| [L104](../src/apps/controller/RemoteSessionWindow.h#L104) | `HandleFrameRateSelection` | 声明 | `void HandleFrameRateSelection(std::uint32_t framesPerSecond)` | 接收并处理 handle frame rate selection 相关逻辑。 |
| [L105](../src/apps/controller/RemoteSessionWindow.h#L105) | `HandleQualitySelection` | 声明 | `void HandleQualitySelection(ScreenQualityTier quality)` | 接收并处理 handle quality selection 相关逻辑。 |
| [L106](../src/apps/controller/RemoteSessionWindow.h#L106) | `RequestStreamPreference` | 声明 | `bool RequestStreamPreference(bool showError = true)` | 发起请求或查询 request stream preference 相关逻辑。 |
| [L107](../src/apps/controller/RemoteSessionWindow.h#L107) | `ToggleRemoteSound` | 声明 | `void ToggleRemoteSound()` | 实现 toggle remote sound 对应的业务或工具逻辑。 |
| [L108](../src/apps/controller/RemoteSessionWindow.h#L108) | `ToggleLocalMicrophone` | 声明 | `void ToggleLocalMicrophone()` | 实现 toggle local microphone 对应的业务或工具逻辑。 |
| [L109](../src/apps/controller/RemoteSessionWindow.h#L109) | `ShowMediaDeviceMenu` | 声明 | `void ShowMediaDeviceMenu()` | 实现 show media device menu 对应的业务或工具逻辑。 |
| [L110](../src/apps/controller/RemoteSessionWindow.h#L110) | `ShowRemoteDisplayMenu` | 声明 | `void ShowRemoteDisplayMenu()` | 实现 show remote display menu 对应的业务或工具逻辑。 |
| [L111](../src/apps/controller/RemoteSessionWindow.h#L111) | `BeginMediaDeviceSelection` | 声明 | `void BeginMediaDeviceSelection( MediaDeviceKind kind, const QString& deviceId)` | 启动 begin media device selection 相关逻辑。 |
| [L113](../src/apps/controller/RemoteSessionWindow.h#L113) | `CompleteMediaDeviceSelections` | 声明 | `void CompleteMediaDeviceSelections( const MediaDeviceSnapshot& media)` | 实现 complete media device selections 对应的业务或工具逻辑。 |
| [L115](../src/apps/controller/RemoteSessionWindow.h#L115) | `ToggleToolbarLock` | 声明 | `void ToggleToolbarLock()` | 实现 toggle toolbar lock 对应的业务或工具逻辑。 |
| [L116](../src/apps/controller/RemoteSessionWindow.h#L116) | `ToggleFullScreenMode` | 声明 | `void ToggleFullScreenMode()` | 实现 toggle full screen mode 对应的业务或工具逻辑。 |
| [L117](../src/apps/controller/RemoteSessionWindow.h#L117) | `HandleDisconnectAction` | 声明 | `void HandleDisconnectAction()` | 接收并处理 handle disconnect action 相关逻辑。 |
| [L118](../src/apps/controller/RemoteSessionWindow.h#L118) | `ReleaseRemoteInputs` | 声明 | `void ReleaseRemoteInputs()` | 释放或取消 release remote inputs 相关逻辑。 |
| [L119](../src/apps/controller/RemoteSessionWindow.h#L119) | `HandleRemoteCursorMessage` | 声明 | `void HandleRemoteCursorMessage( const std::string& pairId, const RemoteCursorEnvelope& envelope)` | 接收并处理 handle remote cursor message 相关逻辑。 |
| [L122](../src/apps/controller/RemoteSessionWindow.h#L122) | `BeginScreenStartup` | 声明 | `void BeginScreenStartup(std::uint64_t screenShareGeneration)` | 启动 begin screen startup 相关逻辑。 |
| [L123](../src/apps/controller/RemoteSessionWindow.h#L123) | `RequestScreenStartupRefresh` | 声明 | `void RequestScreenStartupRefresh( std::uint64_t screenShareGeneration, std::uint32_t retryStage)` | 发起请求或查询 request screen startup refresh 相关逻辑。 |
| [L126](../src/apps/controller/RemoteSessionWindow.h#L126) | `HandleFirstScreenPresentation` | 声明 | `void HandleFirstScreenPresentation( std::uint64_t screenShareGeneration)` | 接收并处理 handle first screen presentation 相关逻辑。 |
| [L128](../src/apps/controller/RemoteSessionWindow.h#L128) | `MinimizeRemotePasteProgress` | 声明 | `void MinimizeRemotePasteProgress()` | 实现 minimize remote paste progress 对应的业务或工具逻辑。 |
| [L129](../src/apps/controller/RemoteSessionWindow.h#L129) | `RestoreRemotePasteProgress` | 声明 | `void RestoreRemotePasteProgress()` | 读取或恢复 restore remote paste progress 相关逻辑。 |
| [L130](../src/apps/controller/RemoteSessionWindow.h#L130) | `SetRemotePasteStatusButtonVisible` | 声明 | `void SetRemotePasteStatusButtonVisible(bool visible)` | 更新或应用 set remote paste status button visible 相关逻辑。 |
| [L131](../src/apps/controller/RemoteSessionWindow.h#L131) | `RemotePasteDialogRestoreGeometry` | 声明 | `QRect RemotePasteDialogRestoreGeometry() const` | 实现 remote paste dialog restore geometry 对应的业务或工具逻辑。 |
| [L132](../src/apps/controller/RemoteSessionWindow.h#L132) | `CreateRemotePasteAnimationOverlay` | 声明 | `QLabel* CreateRemotePasteAnimationOverlay( const QPixmap& snapshot, const QRect& geometry, qreal opacity = 1.0)` | 创建或初始化 create remote paste animation overlay 相关逻辑。 |

## `src/apps/controller/RemoteSessionWindow.Helpers.inc`

[打开源码](../src/apps/controller/RemoteSessionWindow.Helpers.inc) · **文件作用：** `RemoteSessionWindow` 的实现切片，集中实现 helpers 相关逻辑；成员状态仍定义在所属头文件中。

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L4](../src/apps/controller/RemoteSessionWindow.Helpers.inc#L4) | `MakeToolButton` | 定义 | `QToolButton* MakeToolButton(const QString& text, const QString& tooltip, QWidget* parent)` | SPDX-License-Identifier: GPL-3.0-only Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd) |
| [L52](../src/apps/controller/RemoteSessionWindow.Helpers.inc#L52) | `ScreenQualityText` | 定义 | `QString ScreenQualityText(ScreenQualityTier quality)` | 实现 screen quality text 对应的业务或工具逻辑。 |
| [L69](../src/apps/controller/RemoteSessionWindow.Helpers.inc#L69) | `CaptureBackendText` | 定义 | `QString CaptureBackendText(const std::string& backend)` | 采集 capture backend text 相关逻辑。 |
| [L92](../src/apps/controller/RemoteSessionWindow.Helpers.inc#L92) | `ActiveMediaDeviceText` | 定义 | `QString ActiveMediaDeviceText( const MediaDeviceCategorySnapshot& category)` | 实现 active media device text 对应的业务或工具逻辑。 |
| [L117](../src/apps/controller/RemoteSessionWindow.Helpers.inc#L117) | `ScreenQualityBounds` | 定义 | `std::pair<std::uint32_t, std::uint32_t> ScreenQualityBounds( ScreenQualityTier quality)` | 实现 screen quality bounds 对应的业务或工具逻辑。 |

## `src/apps/controller/RemoteSessionWindow.Layout.inc`

[打开源码](../src/apps/controller/RemoteSessionWindow.Layout.inc) · **文件作用：** `RemoteSessionWindow` 的实现切片，集中实现 layout 相关逻辑；成员状态仍定义在所属头文件中。

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L4](../src/apps/controller/RemoteSessionWindow.Layout.inc#L4) | `RemoteSessionWindow::eventFilter` | 定义 | `bool RemoteSessionWindow::eventFilter(QObject* watched, QEvent* event)` | SPDX-License-Identifier: GPL-3.0-only Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd) |
| [L43](../src/apps/controller/RemoteSessionWindow.Layout.inc#L43) | `RemoteSessionWindow::resizeEvent` | 定义 | `void RemoteSessionWindow::resizeEvent(QResizeEvent* event)` | 实现 resize event 对应的业务或工具逻辑。 |
| [L49](../src/apps/controller/RemoteSessionWindow.Layout.inc#L49) | `RemoteSessionWindow::changeEvent` | 定义 | `void RemoteSessionWindow::changeEvent(QEvent* event)` | 实现 change event 对应的业务或工具逻辑。 |
| [L73](../src/apps/controller/RemoteSessionWindow.Layout.inc#L73) | `RemoteSessionWindow::moveEvent` | 定义 | `void RemoteSessionWindow::moveEvent(QMoveEvent* event)` | 实现 move event 对应的业务或工具逻辑。 |
| [L79](../src/apps/controller/RemoteSessionWindow.Layout.inc#L79) | `RemoteSessionWindow::LayoutSessionOverlays` | 定义 | `void RemoteSessionWindow::LayoutSessionOverlays()` | 实现 layout session overlays 对应的业务或工具逻辑。 |
| [L223](../src/apps/controller/RemoteSessionWindow.Layout.inc#L223) | `RemoteSessionWindow::ShowSessionToolbar` | 定义 | `void RemoteSessionWindow::ShowSessionToolbar(bool animated)` | 实现 show session toolbar 对应的业务或工具逻辑。 |
| [L259](../src/apps/controller/RemoteSessionWindow.Layout.inc#L259) | `RemoteSessionWindow::HideSessionToolbar` | 定义 | `void RemoteSessionWindow::HideSessionToolbar(bool animated)` | 实现 hide session toolbar 对应的业务或工具逻辑。 |
| [L291](../src/apps/controller/RemoteSessionWindow.Layout.inc#L291) | `RemoteSessionWindow::ScheduleSessionToolbarHide` | 定义 | `void RemoteSessionWindow::ScheduleSessionToolbarHide()` | 执行后台循环或调度 schedule session toolbar hide 相关逻辑。 |

## `src/apps/controller/RemoteSessionWindow.Lifecycle.inc`

[打开源码](../src/apps/controller/RemoteSessionWindow.Lifecycle.inc) · **文件作用：** `RemoteSessionWindow` 的实现切片，集中实现 lifecycle 相关逻辑；成员状态仍定义在所属头文件中。

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L4](../src/apps/controller/RemoteSessionWindow.Lifecycle.inc#L4) | `RemoteSessionWindow::RemoteSessionWindow` | 定义 | `RemoteSessionWindow::RemoteSessionWindow(RemoteSessionBinding binding, app::InProcessSessionEngine* sessionEngine, QWidget* parent) : FramelessMainWindow(parent), remoteInputDispatcher_( std::make_shared<RemoteInputDi...` | SPDX-License-Identifier: GPL-3.0-only Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd) |
| [L43](../src/apps/controller/RemoteSessionWindow.Lifecycle.inc#L43) | `RemoteSessionWindow::~RemoteSessionWindow` | 定义 | `RemoteSessionWindow::~RemoteSessionWindow()` | 停止相关活动并释放 RemoteSessionWindow 实例拥有的资源。 |
| [L62](../src/apps/controller/RemoteSessionWindow.Lifecycle.inc#L62) | `RemoteSessionWindow::SetDisconnectHandler` | 定义 | `void RemoteSessionWindow::SetDisconnectHandler( std::function<void()> handler)` | 更新或应用 set disconnect handler 相关逻辑。 |
| [L68](../src/apps/controller/RemoteSessionWindow.Lifecycle.inc#L68) | `RemoteSessionWindow::SetRemotePasteHandler` | 定义 | `void RemoteSessionWindow::SetRemotePasteHandler( std::function<bool(const QStringList& localFiles, bool keyboardPaste)> handler)` | 更新或应用 set remote paste handler 相关逻辑。 |
| [L75](../src/apps/controller/RemoteSessionWindow.Lifecycle.inc#L75) | `RemoteSessionWindow::SetRemotePasteCancelHandler` | 定义 | `void RemoteSessionWindow::SetRemotePasteCancelHandler( std::function<void()> handler)` | 更新或应用 set remote paste cancel handler 相关逻辑。 |
| [L81](../src/apps/controller/RemoteSessionWindow.Lifecycle.inc#L81) | `RemoteSessionWindow::SetFileTransferHandlers` | 定义 | `void RemoteSessionWindow::SetFileTransferHandlers( std::function<void()> openHandler, std::function<void()> releaseHostHandler)` | 更新或应用 set file transfer handlers 相关逻辑。 |
| [L90](../src/apps/controller/RemoteSessionWindow.Lifecycle.inc#L90) | `RemoteSessionWindow::SetDragPointerSampleRate` | 定义 | `void RemoteSessionWindow::SetDragPointerSampleRate( std::uint32_t hertz)` | 更新或应用 set drag pointer sample rate 相关逻辑。 |
| [L107](../src/apps/controller/RemoteSessionWindow.Lifecycle.inc#L107) | `RemoteSessionWindow::SetRoomOnlineMemberCount` | 定义 | `void RemoteSessionWindow::SetRoomOnlineMemberCount( std::size_t onlineMemberCount)` | 更新或应用 set room online member count 相关逻辑。 |
| [L126](../src/apps/controller/RemoteSessionWindow.Lifecycle.inc#L126) | `RemoteSessionWindow::BindSessionVideo` | 定义 | `void RemoteSessionWindow::BindSessionVideo( app::InProcessSessionEngine* engine, RemoteSessionBinding binding)` | 实现 bind session video 对应的业务或工具逻辑。 |
| [L283](../src/apps/controller/RemoteSessionWindow.Lifecycle.inc#L283) | `RemoteSessionWindow::HandleRemoteCursorMessage` | 定义 | `void RemoteSessionWindow::HandleRemoteCursorMessage( const std::string& pairId, const RemoteCursorEnvelope& envelope)` | 接收并处理 handle remote cursor message 相关逻辑。 |
| [L301](../src/apps/controller/RemoteSessionWindow.Lifecycle.inc#L301) | `RemoteSessionWindow::BeginScreenStartup` | 定义 | `void RemoteSessionWindow::BeginScreenStartup( std::uint64_t screenShareGeneration)` | 启动 begin screen startup 相关逻辑。 |
| [L336](../src/apps/controller/RemoteSessionWindow.Lifecycle.inc#L336) | `RemoteSessionWindow::RequestScreenStartupRefresh` | 定义 | `void RemoteSessionWindow::RequestScreenStartupRefresh( std::uint64_t screenShareGeneration, std::uint32_t retryStage)` | 发起请求或查询 request screen startup refresh 相关逻辑。 |
| [L367](../src/apps/controller/RemoteSessionWindow.Lifecycle.inc#L367) | `RemoteSessionWindow::HandleFirstScreenPresentation` | 定义 | `void RemoteSessionWindow::HandleFirstScreenPresentation( std::uint64_t screenShareGeneration)` | 接收并处理 handle first screen presentation 相关逻辑。 |

## `src/apps/controller/RemoteSessionWindow.MediaMenu.inc`

[打开源码](../src/apps/controller/RemoteSessionWindow.MediaMenu.inc) · **文件作用：** `RemoteSessionWindow` 的实现切片，集中实现 media menu 相关逻辑；成员状态仍定义在所属头文件中。

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L4](../src/apps/controller/RemoteSessionWindow.MediaMenu.inc#L4) | `RemoteSessionWindow::ShowMediaDeviceMenu` | 定义 | `void RemoteSessionWindow::ShowMediaDeviceMenu()` | SPDX-License-Identifier: GPL-3.0-only Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd) |

## `src/apps/controller/RemoteSessionWindow.MediaSelection.inc`

[打开源码](../src/apps/controller/RemoteSessionWindow.MediaSelection.inc) · **文件作用：** `RemoteSessionWindow` 的实现切片，集中实现 media selection 相关逻辑；成员状态仍定义在所属头文件中。

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L4](../src/apps/controller/RemoteSessionWindow.MediaSelection.inc#L4) | `RemoteSessionWindow::BeginMediaDeviceSelection` | 定义 | `void RemoteSessionWindow::BeginMediaDeviceSelection( MediaDeviceKind kind, const QString& deviceId)` | SPDX-License-Identifier: GPL-3.0-only Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd) |
| [L78](../src/apps/controller/RemoteSessionWindow.MediaSelection.inc#L78) | `RemoteSessionWindow::CompleteMediaDeviceSelections` | 定义 | `void RemoteSessionWindow::CompleteMediaDeviceSelections( const MediaDeviceSnapshot& media)` | 实现 complete media device selections 对应的业务或工具逻辑。 |

## `src/apps/controller/RemoteSessionWindow.SessionState.inc`

[打开源码](../src/apps/controller/RemoteSessionWindow.SessionState.inc) · **文件作用：** `RemoteSessionWindow` 的实现切片，集中实现 session state 相关逻辑；成员状态仍定义在所属头文件中。

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L4](../src/apps/controller/RemoteSessionWindow.SessionState.inc#L4) | `RemoteSessionWindow::RefreshControlState` | 定义 | `void RemoteSessionWindow::RefreshControlState()` | SPDX-License-Identifier: GPL-3.0-only Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd) |

## `src/apps/controller/RemoteSessionWindow.Ui.inc`

[打开源码](../src/apps/controller/RemoteSessionWindow.Ui.inc) · **文件作用：** `RemoteSessionWindow` 的实现切片，集中实现 ui 相关逻辑；成员状态仍定义在所属头文件中。

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L4](../src/apps/controller/RemoteSessionWindow.Ui.inc#L4) | `RemoteSessionWindow::BuildUi` | 定义 | `void RemoteSessionWindow::BuildUi()` | SPDX-License-Identifier: GPL-3.0-only Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd) |

## `src/apps/controller/RemoteSessionWindow.WindowActions.inc`

[打开源码](../src/apps/controller/RemoteSessionWindow.WindowActions.inc) · **文件作用：** `RemoteSessionWindow` 的实现切片，集中实现 window actions 相关逻辑；成员状态仍定义在所属头文件中。

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L4](../src/apps/controller/RemoteSessionWindow.WindowActions.inc#L4) | `RemoteSessionWindow::ToggleToolbarLock` | 定义 | `void RemoteSessionWindow::ToggleToolbarLock()` | SPDX-License-Identifier: GPL-3.0-only Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd) |
| [L69](../src/apps/controller/RemoteSessionWindow.WindowActions.inc#L69) | `RemoteSessionWindow::ToggleFullScreenMode` | 定义 | `void RemoteSessionWindow::ToggleFullScreenMode()` | 实现 toggle full screen mode 对应的业务或工具逻辑。 |
| [L106](../src/apps/controller/RemoteSessionWindow.WindowActions.inc#L106) | `RemoteSessionWindow::ReleaseRemoteInputs` | 定义 | `void RemoteSessionWindow::ReleaseRemoteInputs()` | 释放或取消 release remote inputs 相关逻辑。 |

## `src/apps/controller/RoomCameraWindow.cpp`

[打开源码](../src/apps/controller/RoomCameraWindow.cpp) · **文件作用：** 实现 room camera window 相关函数与文件级辅助逻辑。

### 类型

| 行 | 类型 | 种类 | 作用 |
|---:|---|---|---|
| [L143](../src/apps/controller/RoomCameraWindow.cpp#L143) | `RoomCameraTile` | class | 定义 RoomCameraTile 的 class 类型和相关状态。 |
| [L301](../src/apps/controller/RoomCameraWindow.cpp#L301) | `CameraAspectRatioHost` | class | 定义 CameraAspectRatioHost 的 class 类型和相关状态。 |

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L106](../src/apps/controller/RoomCameraWindow.cpp#L106) | `MemberName` | 定义 | `QString MemberName(const RoomMemberSnapshot& member)` | 实现 member name 对应的业务或工具逻辑。 |
| [L113](../src/apps/controller/RoomCameraWindow.cpp#L113) | `ClearLayout` | 定义 | `void ClearLayout(QLayout* layout)` | 重置或移除 clear layout 相关逻辑。 |
| [L120](../src/apps/controller/RoomCameraWindow.cpp#L120) | `AnimateCameraTileEntrance` | 定义 | `void AnimateCameraTileEntrance(QWidget* widget)` | 实现 animate camera tile entrance 对应的业务或工具逻辑。 |
| [L147](../src/apps/controller/RoomCameraWindow.cpp#L147) | `RoomCameraTile` | 定义 | `RoomCameraTile(QString name, bool local, std::function<void(bool)> activate, QWidget* parent = nullptr) : QFrame(parent), name_(std::move(name)), local_(local), activate_(std::move(activate))` | 实现 room camera tile 对应的业务或工具逻辑。 |
| [L172](../src/apps/controller/RoomCameraWindow.cpp#L172) | `SetName` | 定义 | `void SetName(const QString& name)` | 更新或应用 set name 相关逻辑。 |
| [L180](../src/apps/controller/RoomCameraWindow.cpp#L180) | `IsLocal` | 定义 | `bool IsLocal() const { return local_; }` | 判断 is local 相关逻辑。 |
| [L182](../src/apps/controller/RoomCameraWindow.cpp#L182) | `SetPresentation` | 定义 | `void SetPresentation(bool thumbnail, bool overview)` | 更新或应用 set presentation 相关逻辑。 |
| [L204](../src/apps/controller/RoomCameraWindow.cpp#L204) | `OnFrame` | 定义 | `void OnFrame(const webrtc::VideoFrame& frame) override` | 接收并处理 on frame 相关逻辑。 |
| [L231](../src/apps/controller/RoomCameraWindow.cpp#L231) | `mousePressEvent` | 定义 | `void mousePressEvent(QMouseEvent* event) override` | 实现 mouse press event 对应的业务或工具逻辑。 |
| [L239](../src/apps/controller/RoomCameraWindow.cpp#L239) | `mouseDoubleClickEvent` | 定义 | `void mouseDoubleClickEvent(QMouseEvent* event) override` | 实现 mouse double click event 对应的业务或工具逻辑。 |
| [L249](../src/apps/controller/RoomCameraWindow.cpp#L249) | `resizeEvent` | 定义 | `void resizeEvent(QResizeEvent* event) override` | 实现 resize event 对应的业务或工具逻辑。 |
| [L260](../src/apps/controller/RoomCameraWindow.cpp#L260) | `paintEvent` | 定义 | `void paintEvent(QPaintEvent* event) override` | 准备或呈现 paint event 相关逻辑。 |
| [L303](../src/apps/controller/RoomCameraWindow.cpp#L303) | `CameraAspectRatioHost` | 定义 | `explicit CameraAspectRatioHost(RoomCameraTile* tile, QWidget* parent = nullptr) : QWidget(parent), tile_(tile)` | 实现 camera aspect ratio host 对应的业务或工具逻辑。 |
| [L311](../src/apps/controller/RoomCameraWindow.cpp#L311) | `SetPresentation` | 定义 | `void SetPresentation(bool thumbnail, bool overview)` | 更新或应用 set presentation 相关逻辑。 |
| [L326](../src/apps/controller/RoomCameraWindow.cpp#L326) | `resizeEvent` | 定义 | `void resizeEvent(QResizeEvent* event) override` | 实现 resize event 对应的业务或工具逻辑。 |
| [L333](../src/apps/controller/RoomCameraWindow.cpp#L333) | `UpdateTileGeometry` | 定义 | `void UpdateTileGeometry()` | 更新或应用 update tile geometry 相关逻辑。 |
| [L353](../src/apps/controller/RoomCameraWindow.cpp#L353) | `RoomCameraWindow::RoomCameraWindow` | 定义 | `RoomCameraWindow::RoomCameraWindow(app::InProcessSessionEngine* engine, QWidget* parent) : FramelessMainWindow(parent), engine_(engine)` | 构造并初始化 RoomCameraWindow 实例。 |
| [L434](../src/apps/controller/RoomCameraWindow.cpp#L434) | `RoomCameraWindow::~RoomCameraWindow` | 定义 | `RoomCameraWindow::~RoomCameraWindow()` | 停止相关活动并释放 RoomCameraWindow 实例拥有的资源。 |
| [L439](../src/apps/controller/RoomCameraWindow.cpp#L439) | `RoomCameraWindow::SyncSnapshot` | 定义 | `void RoomCameraWindow::SyncSnapshot(const SessionEngineSnapshot& snapshot)` | 实现 sync snapshot 对应的业务或工具逻辑。 |
| [L544](../src/apps/controller/RoomCameraWindow.cpp#L544) | `RoomCameraWindow::OpenBesideMainWindow` | 定义 | `void RoomCameraWindow::OpenBesideMainWindow( const QRect& mainWindowGeometry)` | 启动 open beside main window 相关逻辑。 |
| [L564](../src/apps/controller/RoomCameraWindow.cpp#L564) | `RoomCameraWindow::SetHiddenByUserCallback` | 定义 | `void RoomCameraWindow::SetHiddenByUserCallback( std::function<void()> callback)` | 更新或应用 set hidden by user callback 相关逻辑。 |
| [L570](../src/apps/controller/RoomCameraWindow.cpp#L570) | `RoomCameraWindow::closeEvent` | 定义 | `void RoomCameraWindow::closeEvent(QCloseEvent* event)` | 关闭并清理 close event 相关逻辑。 |
| [L579](../src/apps/controller/RoomCameraWindow.cpp#L579) | `RoomCameraWindow::ConstrainResizeGeometry` | 定义 | `QRect RoomCameraWindow::ConstrainResizeGeometry( const QRect& proposedGeometry, Qt::Edges resizeEdges, qreal devicePixelRatio) const` | 实现 constrain resize geometry 对应的业务或工具逻辑。 |
| [L619](../src/apps/controller/RoomCameraWindow.cpp#L619) | `RoomCameraWindow::RebuildLayout` | 定义 | `void RoomCameraWindow::RebuildLayout()` | 更新或应用 rebuild layout 相关逻辑。 |
| [L634](../src/apps/controller/RoomCameraWindow.cpp#L634) | `RoomCameraWindow::RebuildFocusLayout` | 定义 | `void RoomCameraWindow::RebuildFocusLayout()` | 更新或应用 rebuild focus layout 相关逻辑。 |
| [L690](../src/apps/controller/RoomCameraWindow.cpp#L690) | `RoomCameraWindow::RebuildOverviewLayout` | 定义 | `void RoomCameraWindow::RebuildOverviewLayout()` | 更新或应用 rebuild overview layout 相关逻辑。 |
| [L741](../src/apps/controller/RoomCameraWindow.cpp#L741) | `RoomCameraWindow::SetFocusedDevice` | 定义 | `void RoomCameraWindow::SetFocusedDevice(const QString& deviceId, bool maximize)` | 更新或应用 set focused device 相关逻辑。 |
| [L756](../src/apps/controller/RoomCameraWindow.cpp#L756) | `RoomCameraWindow::SetOverviewMode` | 定义 | `void RoomCameraWindow::SetOverviewMode(bool overview)` | 更新或应用 set overview mode 相关逻辑。 |
| [L762](../src/apps/controller/RoomCameraWindow.cpp#L762) | `RoomCameraWindow::UpdateSingleParticipantMode` | 定义 | `void RoomCameraWindow::UpdateSingleParticipantMode(int participantCount)` | 更新或应用 update single participant mode 相关逻辑。 |
| [L790](../src/apps/controller/RoomCameraWindow.cpp#L790) | `RoomCameraWindow::SingleWindowHeightForWidth` | 定义 | `int RoomCameraWindow::SingleWindowHeightForWidth(int windowWidth) const` | 实现 single window height for width 对应的业务或工具逻辑。 |
| [L804](../src/apps/controller/RoomCameraWindow.cpp#L804) | `RoomCameraWindow::SingleWindowWidthForHeight` | 定义 | `int RoomCameraWindow::SingleWindowWidthForHeight(int windowHeight) const` | 实现 single window width for height 对应的业务或工具逻辑。 |
| [L818](../src/apps/controller/RoomCameraWindow.cpp#L818) | `RoomCameraWindow::PlaceBesideMainWindow` | 定义 | `void RoomCameraWindow::PlaceBesideMainWindow( const QRect& mainWindowGeometry)` | 实现 place beside main window 对应的业务或工具逻辑。 |
| [L850](../src/apps/controller/RoomCameraWindow.cpp#L850) | `RoomCameraWindow::RemoveTile` | 定义 | `void RoomCameraWindow::RemoveTile(const QString& deviceId)` | 重置或移除 remove tile 相关逻辑。 |
| [L872](../src/apps/controller/RoomCameraWindow.cpp#L872) | `RoomCameraWindow::DetachAllSinks` | 定义 | `void RoomCameraWindow::DetachAllSinks()` | 实现 detach all sinks 对应的业务或工具逻辑。 |

## `src/apps/controller/RoomCameraWindow.h`

[打开源码](../src/apps/controller/RoomCameraWindow.h) · **文件作用：** 声明 room camera window 相关类型、接口、配置和成员状态。

### 类型

| 行 | 类型 | 种类 | 作用 |
|---:|---|---|---|
| [L16](../src/apps/controller/RoomCameraWindow.h#L16) | `QCloseEvent` | class | 定义 QCloseEvent 的 class 类型和相关状态。 |
| [L17](../src/apps/controller/RoomCameraWindow.h#L17) | `QGridLayout` | class | 定义 QGridLayout 的 class 类型和相关状态。 |
| [L18](../src/apps/controller/RoomCameraWindow.h#L18) | `QHBoxLayout` | class | 定义 QHBoxLayout 的 class 类型和相关状态。 |
| [L19](../src/apps/controller/RoomCameraWindow.h#L19) | `QLabel` | class | 定义 QLabel 的 class 类型和相关状态。 |
| [L20](../src/apps/controller/RoomCameraWindow.h#L20) | `QPushButton` | class | 定义 QPushButton 的 class 类型和相关状态。 |
| [L21](../src/apps/controller/RoomCameraWindow.h#L21) | `QScrollArea` | class | 定义 QScrollArea 的 class 类型和相关状态。 |
| [L22](../src/apps/controller/RoomCameraWindow.h#L22) | `QStackedWidget` | class | 定义 QStackedWidget 的 class 类型和相关状态。 |
| [L23](../src/apps/controller/RoomCameraWindow.h#L23) | `QVBoxLayout` | class | 定义 QVBoxLayout 的 class 类型和相关状态。 |
| [L26](../src/apps/controller/RoomCameraWindow.h#L26) | `InProcessSessionEngine` | class | 定义 InProcessSessionEngine 的 class 类型和相关状态。 |
| [L31](../src/apps/controller/RoomCameraWindow.h#L31) | `RoomCameraTile` | class | 定义 RoomCameraTile 的 class 类型和相关状态。 |
| [L33](../src/apps/controller/RoomCameraWindow.h#L33) | `RoomCameraWindow` | class | 定义 RoomCameraWindow 的 class 类型和相关状态。 |

### 成员与文件级变量

| 行 | 变量 | 声明 | 作用 |
|---:|---|---|---|
| [L16](../src/apps/controller/RoomCameraWindow.h#L16) | `QCloseEvent` | `class QCloseEvent;` | 保存 q close event 相关配置或运行状态。 |
| [L17](../src/apps/controller/RoomCameraWindow.h#L17) | `QGridLayout` | `class QGridLayout;` | 保存 q grid layout 相关配置或运行状态。 |
| [L18](../src/apps/controller/RoomCameraWindow.h#L18) | `QHBoxLayout` | `class QHBoxLayout;` | 保存 qh box layout 相关配置或运行状态。 |
| [L19](../src/apps/controller/RoomCameraWindow.h#L19) | `QLabel` | `class QLabel;` | 保存路径、地址或显示名称：q label。 |
| [L20](../src/apps/controller/RoomCameraWindow.h#L20) | `QPushButton` | `class QPushButton;` | 保存 q push button 相关配置或运行状态。 |
| [L21](../src/apps/controller/RoomCameraWindow.h#L21) | `QScrollArea` | `class QScrollArea;` | 保存 q scroll area 相关配置或运行状态。 |
| [L22](../src/apps/controller/RoomCameraWindow.h#L22) | `QStackedWidget` | `class QStackedWidget;` | 保存 q stacked widget 相关配置或运行状态。 |
| [L23](../src/apps/controller/RoomCameraWindow.h#L23) | `QVBoxLayout` | `class QVBoxLayout;` | 保存 qv box layout 相关配置或运行状态。 |
| [L26](../src/apps/controller/RoomCameraWindow.h#L26) | `InProcessSessionEngine` | `class InProcessSessionEngine;` | 保存 in process session engine 相关配置或运行状态。 |
| [L31](../src/apps/controller/RoomCameraWindow.h#L31) | `RoomCameraTile` | `class RoomCameraTile;` | 保存 room camera tile 相关配置或运行状态。 |
| [L62](../src/apps/controller/RoomCameraWindow.h#L62) | `engine_` | `app::InProcessSessionEngine* engine_ = nullptr;` | 保存 engine 相关配置或运行状态。 |
| [L63](../src/apps/controller/RoomCameraWindow.h#L63) | `viewStack_` | `QStackedWidget* viewStack_ = nullptr;` | 保存 view stack 相关配置或运行状态。 |
| [L64](../src/apps/controller/RoomCameraWindow.h#L64) | `galleryHeader_` | `QWidget* galleryHeader_ = nullptr;` | 保存 gallery header 相关配置或运行状态。 |
| [L65](../src/apps/controller/RoomCameraWindow.h#L65) | `focusPage_` | `QWidget* focusPage_ = nullptr;` | 保存 focus page 相关配置或运行状态。 |
| [L66](../src/apps/controller/RoomCameraWindow.h#L66) | `focusLayout_` | `QVBoxLayout* focusLayout_ = nullptr;` | 保存 focus layout 相关配置或运行状态。 |
| [L67](../src/apps/controller/RoomCameraWindow.h#L67) | `thumbnailHost_` | `QWidget* thumbnailHost_ = nullptr;` | 保存 thumbnail host 相关配置或运行状态。 |
| [L68](../src/apps/controller/RoomCameraWindow.h#L68) | `thumbnailLayout_` | `QHBoxLayout* thumbnailLayout_ = nullptr;` | 保存 thumbnail layout 相关配置或运行状态。 |
| [L69](../src/apps/controller/RoomCameraWindow.h#L69) | `thumbnailScroll_` | `QScrollArea* thumbnailScroll_ = nullptr;` | 保存 thumbnail scroll 相关配置或运行状态。 |
| [L70](../src/apps/controller/RoomCameraWindow.h#L70) | `overviewPage_` | `QWidget* overviewPage_ = nullptr;` | 保存 overview page 相关配置或运行状态。 |
| [L71](../src/apps/controller/RoomCameraWindow.h#L71) | `overviewGrid_` | `QGridLayout* overviewGrid_ = nullptr;` | 保存身份或作用域标识：overview grid。 |
| [L72](../src/apps/controller/RoomCameraWindow.h#L72) | `countLabel_` | `QLabel* countLabel_ = nullptr;` | 保存路径、地址或显示名称：count label。 |
| [L73](../src/apps/controller/RoomCameraWindow.h#L73) | `emptyLabel_` | `QLabel* emptyLabel_ = nullptr;` | 保存路径、地址或显示名称：empty label。 |
| [L74](../src/apps/controller/RoomCameraWindow.h#L74) | `viewModeButton_` | `QPushButton* viewModeButton_ = nullptr;` | 保存 view mode button 相关配置或运行状态。 |
| [L75](../src/apps/controller/RoomCameraWindow.h#L75) | `tiles_` | `QHash<QString, RoomCameraTile*> tiles_;` | 保存 tiles 相关配置或运行状态。 |
| [L76](../src/apps/controller/RoomCameraWindow.h#L76) | `tileHosts_` | `QHash<QString, QWidget*> tileHosts_;` | 保存 tile hosts 相关配置或运行状态。 |
| [L77](../src/apps/controller/RoomCameraWindow.h#L77) | `pairBindings_` | `QHash<QString, QString> pairBindings_;` | 保存 pair bindings 相关配置或运行状态。 |
| [L78](../src/apps/controller/RoomCameraWindow.h#L78) | `orderedDeviceIds_` | `QStringList orderedDeviceIds_;` | 保存 ordered device ids 相关配置或运行状态。 |
| [L79](../src/apps/controller/RoomCameraWindow.h#L79) | `focusedDeviceId_` | `QString focusedDeviceId_;` | 保存身份或作用域标识：focused device id。 |
| [L80](../src/apps/controller/RoomCameraWindow.h#L80) | `overviewMode_` | `bool overviewMode_ = false;` | 保存 overview mode 相关配置或运行状态。 |
| [L81](../src/apps/controller/RoomCameraWindow.h#L81) | `singleParticipantMode_` | `bool singleParticipantMode_ = false;` | 保存 single participant mode 相关配置或运行状态。 |
| [L82](../src/apps/controller/RoomCameraWindow.h#L82) | `initialPlacementDone_` | `bool initialPlacementDone_ = false;` | 保存 initial placement done 相关配置或运行状态。 |

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L35](../src/apps/controller/RoomCameraWindow.h#L35) | `RoomCameraWindow` | 声明 | `explicit RoomCameraWindow(app::InProcessSessionEngine* engine, QWidget* parent = nullptr)` | 实现 room camera window 对应的业务或工具逻辑。 |
| [L37](../src/apps/controller/RoomCameraWindow.h#L37) | `~RoomCameraWindow` | 声明 | `~RoomCameraWindow() override` | 停止相关活动并释放 RoomCameraWindow 实例拥有的资源。 |
| [L39](../src/apps/controller/RoomCameraWindow.h#L39) | `SyncSnapshot` | 声明 | `void SyncSnapshot(const SessionEngineSnapshot& snapshot)` | 实现 sync snapshot 对应的业务或工具逻辑。 |
| [L40](../src/apps/controller/RoomCameraWindow.h#L40) | `OpenBesideMainWindow` | 声明 | `void OpenBesideMainWindow(const QRect& mainWindowGeometry)` | 启动 open beside main window 相关逻辑。 |
| [L41](../src/apps/controller/RoomCameraWindow.h#L41) | `SetHiddenByUserCallback` | 声明 | `void SetHiddenByUserCallback(std::function<void()> callback)` | 更新或应用 set hidden by user callback 相关逻辑。 |
| [L44](../src/apps/controller/RoomCameraWindow.h#L44) | `closeEvent` | 声明 | `void closeEvent(QCloseEvent* event) override` | 关闭并清理 close event 相关逻辑。 |
| [L45](../src/apps/controller/RoomCameraWindow.h#L45) | `ConstrainResizeGeometry` | 声明 | `QRect ConstrainResizeGeometry(const QRect& proposedGeometry, Qt::Edges resizeEdges, qreal devicePixelRatio) const override` | 实现 constrain resize geometry 对应的业务或工具逻辑。 |
| [L50](../src/apps/controller/RoomCameraWindow.h#L50) | `RebuildLayout` | 声明 | `void RebuildLayout()` | 更新或应用 rebuild layout 相关逻辑。 |
| [L51](../src/apps/controller/RoomCameraWindow.h#L51) | `RebuildFocusLayout` | 声明 | `void RebuildFocusLayout()` | 更新或应用 rebuild focus layout 相关逻辑。 |
| [L52](../src/apps/controller/RoomCameraWindow.h#L52) | `RebuildOverviewLayout` | 声明 | `void RebuildOverviewLayout()` | 更新或应用 rebuild overview layout 相关逻辑。 |
| [L53](../src/apps/controller/RoomCameraWindow.h#L53) | `SetFocusedDevice` | 声明 | `void SetFocusedDevice(const QString& deviceId, bool maximize)` | 更新或应用 set focused device 相关逻辑。 |
| [L54](../src/apps/controller/RoomCameraWindow.h#L54) | `SetOverviewMode` | 声明 | `void SetOverviewMode(bool overview)` | 更新或应用 set overview mode 相关逻辑。 |
| [L55](../src/apps/controller/RoomCameraWindow.h#L55) | `UpdateSingleParticipantMode` | 声明 | `void UpdateSingleParticipantMode(int participantCount)` | 更新或应用 update single participant mode 相关逻辑。 |
| [L56](../src/apps/controller/RoomCameraWindow.h#L56) | `SingleWindowHeightForWidth` | 声明 | `int SingleWindowHeightForWidth(int windowWidth) const` | 实现 single window height for width 对应的业务或工具逻辑。 |
| [L57](../src/apps/controller/RoomCameraWindow.h#L57) | `SingleWindowWidthForHeight` | 声明 | `int SingleWindowWidthForHeight(int windowHeight) const` | 实现 single window width for height 对应的业务或工具逻辑。 |
| [L58](../src/apps/controller/RoomCameraWindow.h#L58) | `PlaceBesideMainWindow` | 声明 | `void PlaceBesideMainWindow(const QRect& mainWindowGeometry)` | 实现 place beside main window 对应的业务或工具逻辑。 |
| [L59](../src/apps/controller/RoomCameraWindow.h#L59) | `RemoveTile` | 声明 | `void RemoveTile(const QString& deviceId)` | 重置或移除 remove tile 相关逻辑。 |
| [L60](../src/apps/controller/RoomCameraWindow.h#L60) | `DetachAllSinks` | 声明 | `void DetachAllSinks()` | 实现 detach all sinks 对应的业务或工具逻辑。 |

## `src/apps/controller/RoundedPopupMenu.cpp`

[打开源码](../src/apps/controller/RoundedPopupMenu.cpp) · **文件作用：** 实现 rounded popup menu 相关函数与文件级辅助逻辑。

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L18](../src/apps/controller/RoundedPopupMenu.cpp#L18) | `RoundedPopupMenu::RoundedPopupMenu` | 定义 | `RoundedPopupMenu::RoundedPopupMenu( QWidget* parent, qreal cornerRadius) : QMenu(parent), cornerRadius_(cornerRadius)` | 构造并初始化 RoundedPopupMenu 实例。 |
| [L27](../src/apps/controller/RoundedPopupMenu.cpp#L27) | `RoundedPopupMenu::SetToggleAnchor` | 定义 | `void RoundedPopupMenu::SetToggleAnchor( QWidget* anchor, std::function<void()> anchorPressed)` | 更新或应用 set toggle anchor 相关逻辑。 |
| [L35](../src/apps/controller/RoundedPopupMenu.cpp#L35) | `RoundedPopupMenu::HandleToggleAnchorPress` | 定义 | `bool RoundedPopupMenu::HandleToggleAnchorPress(QMouseEvent* event)` | 接收并处理 handle toggle anchor press 相关逻辑。 |
| [L57](../src/apps/controller/RoundedPopupMenu.cpp#L57) | `RoundedPopupMenu::eventFilter` | 定义 | `bool RoundedPopupMenu::eventFilter(QObject* watched, QEvent* event)` | 实现 event filter 对应的业务或工具逻辑。 |
| [L68](../src/apps/controller/RoundedPopupMenu.cpp#L68) | `RoundedPopupMenu::hideEvent` | 定义 | `void RoundedPopupMenu::hideEvent(QHideEvent* event)` | 实现 hide event 对应的业务或工具逻辑。 |
| [L77](../src/apps/controller/RoundedPopupMenu.cpp#L77) | `RoundedPopupMenu::mousePressEvent` | 定义 | `void RoundedPopupMenu::mousePressEvent(QMouseEvent* event)` | 实现 mouse press event 对应的业务或工具逻辑。 |
| [L85](../src/apps/controller/RoundedPopupMenu.cpp#L85) | `RoundedPopupMenu::resizeEvent` | 定义 | `void RoundedPopupMenu::resizeEvent(QResizeEvent* event)` | 实现 resize event 对应的业务或工具逻辑。 |
| [L95](../src/apps/controller/RoundedPopupMenu.cpp#L95) | `RoundedPopupMenu::showEvent` | 定义 | `void RoundedPopupMenu::showEvent(QShowEvent* event)` | 实现 show event 对应的业务或工具逻辑。 |

## `src/apps/controller/RoundedPopupMenu.h`

[打开源码](../src/apps/controller/RoundedPopupMenu.h) · **文件作用：** 声明 rounded popup menu 相关类型、接口、配置和成员状态。

### 类型

| 行 | 类型 | 种类 | 作用 |
|---:|---|---|---|
| [L11](../src/apps/controller/RoundedPopupMenu.h#L11) | `QEvent` | class | 定义 QEvent 的 class 类型和相关状态。 |
| [L12](../src/apps/controller/RoundedPopupMenu.h#L12) | `QHideEvent` | class | 定义 QHideEvent 的 class 类型和相关状态。 |
| [L13](../src/apps/controller/RoundedPopupMenu.h#L13) | `QMouseEvent` | class | 定义 QMouseEvent 的 class 类型和相关状态。 |
| [L14](../src/apps/controller/RoundedPopupMenu.h#L14) | `QShowEvent` | class | 定义 QShowEvent 的 class 类型和相关状态。 |
| [L18](../src/apps/controller/RoundedPopupMenu.h#L18) | `RoundedPopupMenu` | class | 定义 RoundedPopupMenu 的 class 类型和相关状态。 |

### 成员与文件级变量

| 行 | 变量 | 声明 | 作用 |
|---:|---|---|---|
| [L11](../src/apps/controller/RoundedPopupMenu.h#L11) | `QEvent` | `class QEvent;` | 保存 q event 相关配置或运行状态。 |
| [L12](../src/apps/controller/RoundedPopupMenu.h#L12) | `QHideEvent` | `class QHideEvent;` | 保存 q hide event 相关配置或运行状态。 |
| [L13](../src/apps/controller/RoundedPopupMenu.h#L13) | `QMouseEvent` | `class QMouseEvent;` | 保存 q mouse event 相关配置或运行状态。 |
| [L14](../src/apps/controller/RoundedPopupMenu.h#L14) | `QShowEvent` | `class QShowEvent;` | 保存 q show event 相关配置或运行状态。 |
| [L38](../src/apps/controller/RoundedPopupMenu.h#L38) | `cornerRadius_` | `qreal cornerRadius_ = 12.0;` | 保存 corner radius 相关配置或运行状态。 |
| [L39](../src/apps/controller/RoundedPopupMenu.h#L39) | `toggleAnchor_` | `QPointer<QWidget> toggleAnchor_;` | 保存 toggle anchor 相关配置或运行状态。 |
| [L41](../src/apps/controller/RoundedPopupMenu.h#L41) | `applicationFilterInstalled_` | `bool applicationFilterInstalled_ = false;` | 保存 application filter installed 相关配置或运行状态。 |

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L20](../src/apps/controller/RoundedPopupMenu.h#L20) | `RoundedPopupMenu` | 声明 | `explicit RoundedPopupMenu( QWidget* parent = nullptr, qreal cornerRadius = 12.0)` | 实现 rounded popup menu 对应的业务或工具逻辑。 |
| [L24](../src/apps/controller/RoundedPopupMenu.h#L24) | `SetToggleAnchor` | 定义 | `void SetToggleAnchor( QWidget* anchor, std::function<void()> anchorPressed = {})` | 更新或应用 set toggle anchor 相关逻辑。 |
| [L29](../src/apps/controller/RoundedPopupMenu.h#L29) | `eventFilter` | 声明 | `bool eventFilter(QObject* watched, QEvent* event) override` | 实现 event filter 对应的业务或工具逻辑。 |
| [L30](../src/apps/controller/RoundedPopupMenu.h#L30) | `hideEvent` | 声明 | `void hideEvent(QHideEvent* event) override` | 实现 hide event 对应的业务或工具逻辑。 |
| [L31](../src/apps/controller/RoundedPopupMenu.h#L31) | `mousePressEvent` | 声明 | `void mousePressEvent(QMouseEvent* event) override` | 实现 mouse press event 对应的业务或工具逻辑。 |
| [L32](../src/apps/controller/RoundedPopupMenu.h#L32) | `resizeEvent` | 声明 | `void resizeEvent(QResizeEvent* event) override` | 实现 resize event 对应的业务或工具逻辑。 |
| [L33](../src/apps/controller/RoundedPopupMenu.h#L33) | `showEvent` | 声明 | `void showEvent(QShowEvent* event) override` | 实现 show event 对应的业务或工具逻辑。 |
| [L36](../src/apps/controller/RoundedPopupMenu.h#L36) | `HandleToggleAnchorPress` | 声明 | `bool HandleToggleAnchorPress(QMouseEvent* event)` | 接收并处理 handle toggle anchor press 相关逻辑。 |

## `src/apps/controller/ui/morph/MorphIconButtonBinding.cpp`

[打开源码](../src/apps/controller/ui/morph/MorphIconButtonBinding.cpp) · **文件作用：** 实现 morph icon button binding 相关函数与文件级辅助逻辑。

### 成员与文件级变量

| 行 | 变量 | 声明 | 作用 |
|---:|---|---|---|
| [L22](../src/apps/controller/ui/morph/MorphIconButtonBinding.cpp#L22) | `kBindingName` | `constexpr auto kBindingName = "remoteCMorphIconBinding";` | 定义 binding name 的编译期常量或产品边界。 |

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L24](../src/apps/controller/ui/morph/MorphIconButtonBinding.cpp#L24) | `animationLevel` | 定义 | `int animationLevel()` | 实现 animation level 对应的业务或工具逻辑。 |
| [L29](../src/apps/controller/ui/morph/MorphIconButtonBinding.cpp#L29) | `mixColor` | 定义 | `QColor mixColor(const QColor& source, const QColor& target, double progress)` | 实现 mix color 对应的业务或工具逻辑。 |
| [L41](../src/apps/controller/ui/morph/MorphIconButtonBinding.cpp#L41) | `MorphIconButtonBinding::attach` | 定义 | `MorphIconButtonBinding* MorphIconButtonBinding::attach( QAbstractButton* button, const QString& sourceResource, const QString& targetResource, Interaction interaction, const QSize& iconSize, const QColor& sourceColor,...` | 实现 attach 对应的业务或工具逻辑。 |
| [L78](../src/apps/controller/ui/morph/MorphIconButtonBinding.cpp#L78) | `QObject` | 定义 | `: QObject(button), button_(button), sourceResource_(std::move(sourceResource)), targetResource_(std::move(targetResource)), interaction_(interaction), iconSize_(iconSize), sourceColor_(sourceColor), targetColor_(targe...` | 实现 q object 对应的业务或工具逻辑。 |
| [L113](../src/apps/controller/ui/morph/MorphIconButtonBinding.cpp#L113) | `MorphIconButtonBinding::setTarget` | 定义 | `void MorphIconButtonBinding::setTarget(bool target, bool animated)` | 更新或应用 set target 相关逻辑。 |
| [L127](../src/apps/controller/ui/morph/MorphIconButtonBinding.cpp#L127) | `MorphIconButtonBinding::pulse` | 定义 | `void MorphIconButtonBinding::pulse(int holdMilliseconds)` | 实现 pulse 对应的业务或工具逻辑。 |
| [L135](../src/apps/controller/ui/morph/MorphIconButtonBinding.cpp#L135) | `MorphIconButtonBinding::eventFilter` | 定义 | `bool MorphIconButtonBinding::eventFilter(QObject* watched, QEvent* event)` | 实现 event filter 对应的业务或工具逻辑。 |
| [L154](../src/apps/controller/ui/morph/MorphIconButtonBinding.cpp#L154) | `MorphIconButtonBinding::startTransition` | 定义 | `void MorphIconButtonBinding::startTransition(double targetProgress, bool animated)` | 启动 start transition 相关逻辑。 |
| [L178](../src/apps/controller/ui/morph/MorphIconButtonBinding.cpp#L178) | `MorphIconButtonBinding::currentColor` | 定义 | `QColor MorphIconButtonBinding::currentColor() const` | 实现 current color 对应的业务或工具逻辑。 |
| [L194](../src/apps/controller/ui/morph/MorphIconButtonBinding.cpp#L194) | `MorphIconButtonBinding::render` | 定义 | `void MorphIconButtonBinding::render()` | 准备或呈现 render 相关逻辑。 |

## `src/apps/controller/ui/morph/MorphIconButtonBinding.h`

[打开源码](../src/apps/controller/ui/morph/MorphIconButtonBinding.h) · **文件作用：** 声明 morph icon button binding 相关类型、接口、配置和成员状态。

### 类型

| 行 | 类型 | 种类 | 作用 |
|---:|---|---|---|
| [L14](../src/apps/controller/ui/morph/MorphIconButtonBinding.h#L14) | `QAbstractButton` | class | 定义 QAbstractButton 的 class 类型和相关状态。 |
| [L15](../src/apps/controller/ui/morph/MorphIconButtonBinding.h#L15) | `QEvent` | class | 定义 QEvent 的 class 类型和相关状态。 |
| [L19](../src/apps/controller/ui/morph/MorphIconButtonBinding.h#L19) | `MorphIconButtonBinding` | class | 定义 MorphIconButtonBinding 的 class 类型和相关状态。 |
| [L21](../src/apps/controller/ui/morph/MorphIconButtonBinding.h#L21) | `Interaction` | enum class | 定义 Interaction 的 enum class 类型和相关状态。 |

### 成员与文件级变量

| 行 | 变量 | 声明 | 作用 |
|---:|---|---|---|
| [L14](../src/apps/controller/ui/morph/MorphIconButtonBinding.h#L14) | `QAbstractButton` | `class QAbstractButton;` | 保存 q abstract button 相关配置或运行状态。 |
| [L15](../src/apps/controller/ui/morph/MorphIconButtonBinding.h#L15) | `QEvent` | `class QEvent;` | 保存 q event 相关配置或运行状态。 |
| [L58](../src/apps/controller/ui/morph/MorphIconButtonBinding.h#L58) | `button_` | `QAbstractButton* button_ = nullptr;` | 保存 button 相关配置或运行状态。 |
| [L59](../src/apps/controller/ui/morph/MorphIconButtonBinding.h#L59) | `sourceResource_` | `QString sourceResource_;` | 保存 source resource 相关配置或运行状态。 |
| [L60](../src/apps/controller/ui/morph/MorphIconButtonBinding.h#L60) | `targetResource_` | `QString targetResource_;` | 保存 target resource 相关配置或运行状态。 |
| [L61](../src/apps/controller/ui/morph/MorphIconButtonBinding.h#L61) | `interaction_` | `Interaction interaction_ = Interaction::State;` | 保存 interaction 相关配置或运行状态。 |
| [L62](../src/apps/controller/ui/morph/MorphIconButtonBinding.h#L62) | `iconSize_` | `QSize iconSize_{18, 18};` | 保存计数、尺寸或速率指标：icon size。 |
| [L63](../src/apps/controller/ui/morph/MorphIconButtonBinding.h#L63) | `sourceColor_` | `QColor sourceColor_;` | 保存 source color 相关配置或运行状态。 |
| [L64](../src/apps/controller/ui/morph/MorphIconButtonBinding.h#L64) | `targetColor_` | `QColor targetColor_;` | 保存 target color 相关配置或运行状态。 |
| [L65](../src/apps/controller/ui/morph/MorphIconButtonBinding.h#L65) | `core_` | `MorphIconCore core_;` | 保存 core 相关配置或运行状态。 |
| [L66](../src/apps/controller/ui/morph/MorphIconButtonBinding.h#L66) | `spring_` | `Spring spring_;` | 保存 spring 相关配置或运行状态。 |
| [L67](../src/apps/controller/ui/morph/MorphIconButtonBinding.h#L67) | `timer_` | `QTimer timer_;` | 保存定时、截止或超时状态：timer。 |
| [L68](../src/apps/controller/ui/morph/MorphIconButtonBinding.h#L68) | `elapsed_` | `QElapsedTimer elapsed_;` | 保存 elapsed 相关配置或运行状态。 |
| [L69](../src/apps/controller/ui/morph/MorphIconButtonBinding.h#L69) | `progress_` | `double progress_ = 0.0;` | 保存 progress 相关配置或运行状态。 |
| [L70](../src/apps/controller/ui/morph/MorphIconButtonBinding.h#L70) | `start_` | `double start_ = 0.0;` | 保存 start 相关配置或运行状态。 |
| [L71](../src/apps/controller/ui/morph/MorphIconButtonBinding.h#L71) | `end_` | `double end_ = 0.0;` | 保存 end 相关配置或运行状态。 |
| [L72](../src/apps/controller/ui/morph/MorphIconButtonBinding.h#L72) | `target_` | `bool target_ = false;` | 保存 target 相关配置或运行状态。 |
| [L73](../src/apps/controller/ui/morph/MorphIconButtonBinding.h#L73) | `initialized_` | `bool initialized_ = false;` | 保存 initialized 相关配置或运行状态。 |

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L27](../src/apps/controller/ui/morph/MorphIconButtonBinding.h#L27) | `attach` | 声明 | `static MorphIconButtonBinding* attach( QAbstractButton* button, const QString& sourceResource, const QString& targetResource, Interaction interaction, const QSize& iconSize = QSize(18, 18), const QColor& sourceColor =...` | 实现 attach 对应的业务或工具逻辑。 |
| [L36](../src/apps/controller/ui/morph/MorphIconButtonBinding.h#L36) | `setTarget` | 声明 | `void setTarget(bool target, bool animated = true)` | 更新或应用 set target 相关逻辑。 |
| [L37](../src/apps/controller/ui/morph/MorphIconButtonBinding.h#L37) | `pulse` | 声明 | `void pulse(int holdMilliseconds = 1100)` | 实现 pulse 对应的业务或工具逻辑。 |
| [L43](../src/apps/controller/ui/morph/MorphIconButtonBinding.h#L43) | `eventFilter` | 声明 | `bool eventFilter(QObject* watched, QEvent* event) override` | 实现 event filter 对应的业务或工具逻辑。 |
| [L46](../src/apps/controller/ui/morph/MorphIconButtonBinding.h#L46) | `MorphIconButtonBinding` | 声明 | `MorphIconButtonBinding(QAbstractButton* button, QString sourceResource, QString targetResource, Interaction interaction, QSize iconSize, QColor sourceColor, QColor targetColor)` | 实现 morph icon button binding 对应的业务或工具逻辑。 |
| [L54](../src/apps/controller/ui/morph/MorphIconButtonBinding.h#L54) | `startTransition` | 声明 | `void startTransition(double targetProgress, bool animated)` | 启动 start transition 相关逻辑。 |
| [L55](../src/apps/controller/ui/morph/MorphIconButtonBinding.h#L55) | `render` | 声明 | `void render()` | 准备或呈现 render 相关逻辑。 |
| [L56](../src/apps/controller/ui/morph/MorphIconButtonBinding.h#L56) | `currentColor` | 声明 | `QColor currentColor() const` | 实现 current color 对应的业务或工具逻辑。 |

## `src/apps/controller/ui/morph/MorphIconCore.cpp`

[打开源码](../src/apps/controller/ui/morph/MorphIconCore.cpp) · **文件作用：** 实现 morph icon core 相关函数与文件级辅助逻辑。

### 类型

| 行 | 类型 | 种类 | 作用 |
|---:|---|---|---|
| [L33](../src/apps/controller/ui/morph/MorphIconCore.cpp#L33) | `CachedMorphPlan` | struct | 定义 CachedMorphPlan 的 struct 类型和相关状态。 |
| [L98](../src/apps/controller/ui/morph/MorphIconCore.cpp#L98) | `RawSegment` | struct | 定义 RawSegment 的 struct 类型和相关状态。 |
| [L103](../src/apps/controller/ui/morph/MorphIconCore.cpp#L103) | `RawSubpath` | struct | 定义 RawSubpath 的 struct 类型和相关状态。 |
| [L109](../src/apps/controller/ui/morph/MorphIconCore.cpp#L109) | `PathParser` | class | 定义 PathParser 的 class 类型和相关状态。 |
| [L370](../src/apps/controller/ui/morph/MorphIconCore.cpp#L370) | `CubicBuilder` | class | 定义 CubicBuilder 的 class 类型和相关状态。 |
| [L818](../src/apps/controller/ui/morph/MorphIconCore.cpp#L818) | `Alignment` | struct | 定义 Alignment 的 struct 类型和相关状态。 |

### 成员与文件级变量

| 行 | 变量 | 声明 | 作用 |
|---:|---|---|---|
| [L25](../src/apps/controller/ui/morph/MorphIconCore.cpp#L25) | `kPi` | `constexpr double kPi = 3.1415926535897932384626433832795;` | 定义 pi 的编译期常量或产品边界。 |
| [L26](../src/apps/controller/ui/morph/MorphIconCore.cpp#L26) | `kTau` | `constexpr double kTau = 2.0 * kPi;` | 定义 tau 的编译期常量或产品边界。 |
| [L27](../src/apps/controller/ui/morph/MorphIconCore.cpp#L27) | `kCornerThreshold` | `constexpr double kCornerThreshold = kPi / 8.0;` | 定义 corner threshold 的编译期常量或产品边界。 |
| [L28](../src/apps/controller/ui/morph/MorphIconCore.cpp#L28) | `kLengthWeight` | `constexpr double kLengthWeight = 0.35;` | 定义 length weight 的编译期常量或产品边界。 |
| [L29](../src/apps/controller/ui/morph/MorphIconCore.cpp#L29) | `kRotationTieBreak` | `constexpr double kRotationTieBreak = 0.05;` | 定义 rotation tie break 的编译期常量或产品边界。 |
| [L30](../src/apps/controller/ui/morph/MorphIconCore.cpp#L30) | `kGlobalResidualThreshold` | `constexpr double kGlobalResidualThreshold = 5e-3;` | 定义 global residual threshold 的编译期常量或产品边界。 |
| [L31](../src/apps/controller/ui/morph/MorphIconCore.cpp#L31) | `kEpsilon` | `constexpr double kEpsilon = 1e-12;` | 定义 epsilon 的编译期常量或产品边界。 |

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L38](../src/apps/controller/ui/morph/MorphIconCore.cpp#L38) | `morphPlanCacheMutex` | 定义 | `std::mutex& morphPlanCacheMutex()` | 实现 morph plan cache mutex 对应的业务或工具逻辑。 |
| [L44](../src/apps/controller/ui/morph/MorphIconCore.cpp#L44) | `morphPlanCache` | 定义 | `std::unordered_map<std::string, CachedMorphPlan>& morphPlanCache()` | 实现 morph plan cache 对应的业务或工具逻辑。 |
| [L50](../src/apps/controller/ui/morph/MorphIconCore.cpp#L50) | `cross` | 定义 | `double cross(const QPointF& a, const QPointF& b)` | 实现 cross 对应的业务或工具逻辑。 |
| [L55](../src/apps/controller/ui/morph/MorphIconCore.cpp#L55) | `dot` | 定义 | `double dot(const QPointF& a, const QPointF& b)` | 实现 dot 对应的业务或工具逻辑。 |
| [L60](../src/apps/controller/ui/morph/MorphIconCore.cpp#L60) | `norm` | 定义 | `double norm(const QPointF& p)` | 实现 norm 对应的业务或工具逻辑。 |
| [L65](../src/apps/controller/ui/morph/MorphIconCore.cpp#L65) | `centroid` | 定义 | `QPointF centroid(const std::vector<QPointF>& points)` | 实现 centroid 对应的业务或工具逻辑。 |
| [L74](../src/apps/controller/ui/morph/MorphIconCore.cpp#L74) | `polylineLength` | 定义 | `double polylineLength(const std::vector<QPointF>& points)` | 实现 polyline length 对应的业务或工具逻辑。 |
| [L83](../src/apps/controller/ui/morph/MorphIconCore.cpp#L83) | `reversePoints` | 定义 | `std::vector<QPointF> reversePoints(const std::vector<QPointF>& points)` | 实现 reverse points 对应的业务或工具逻辑。 |
| [L88](../src/apps/controller/ui/morph/MorphIconCore.cpp#L88) | `rotatePoints` | 定义 | `std::vector<QPointF> rotatePoints(const std::vector<QPointF>& points, int offset)` | 实现 rotate points 对应的业务或工具逻辑。 |
| [L111](../src/apps/controller/ui/morph/MorphIconCore.cpp#L111) | `PathParser` | 定义 | `explicit PathParser(QString data) : data_(std::move(data)) {}` | 实现 path parser 对应的业务或工具逻辑。 |
| [L113](../src/apps/controller/ui/morph/MorphIconCore.cpp#L113) | `parse` | 定义 | `std::vector<RawSubpath> parse()` | 解码或解析 parse 相关逻辑。 |
| [L244](../src/apps/controller/ui/morph/MorphIconCore.cpp#L244) | `atEnd` | 定义 | `bool atEnd() const { return index_ >= data_.size(); }` | 实现 at end 对应的业务或工具逻辑。 |
| [L245](../src/apps/controller/ui/morph/MorphIconCore.cpp#L245) | `peek` | 定义 | `QChar peek() const { return atEnd() ? QChar() : data_[index_]; }` | 实现 peek 对应的业务或工具逻辑。 |
| [L246](../src/apps/controller/ui/morph/MorphIconCore.cpp#L246) | `take` | 定义 | `QChar take() { return data_[index_++]; }` | 实现 take 对应的业务或工具逻辑。 |
| [L248](../src/apps/controller/ui/morph/MorphIconCore.cpp#L248) | `isCommand` | 定义 | `static bool isCommand(QChar value)` | 判断 is command 相关逻辑。 |
| [L253](../src/apps/controller/ui/morph/MorphIconCore.cpp#L253) | `skipSeparators` | 定义 | `void skipSeparators()` | 实现 skip separators 对应的业务或工具逻辑。 |
| [L269](../src/apps/controller/ui/morph/MorphIconCore.cpp#L269) | `number` | 定义 | `double number()` | 实现 number 对应的业务或工具逻辑。 |
| [L313](../src/apps/controller/ui/morph/MorphIconCore.cpp#L313) | `flag` | 定义 | `int flag()` | 实现 flag 对应的业务或工具逻辑。 |
| [L322](../src/apps/controller/ui/morph/MorphIconCore.cpp#L322) | `point` | 定义 | `QPointF point(bool relative)` | 实现 point 对应的业务或工具逻辑。 |
| [L327](../src/apps/controller/ui/morph/MorphIconCore.cpp#L327) | `pointFromOrigin` | 定义 | `QPointF pointFromOrigin(bool relative, const QPointF& origin)` | 实现 point from origin 对应的业务或工具逻辑。 |
| [L338](../src/apps/controller/ui/morph/MorphIconCore.cpp#L338) | `ensureOpen` | 定义 | `void ensureOpen()` | 实现 ensure open 对应的业务或工具逻辑。 |
| [L349](../src/apps/controller/ui/morph/MorphIconCore.cpp#L349) | `append` | 定义 | `void append(QChar kind, std::initializer_list<double> values)` | 实现 append 对应的业务或工具逻辑。 |
| [L372](../src/apps/controller/ui/morph/MorphIconCore.cpp#L372) | `CubicBuilder` | 定义 | `explicit CubicBuilder(QPointF start) : current_(start)` | 实现 cubic builder 对应的业务或工具逻辑。 |
| [L377](../src/apps/controller/ui/morph/MorphIconCore.cpp#L377) | `cubic` | 定义 | `void cubic(const QPointF& c1, const QPointF& c2, const QPointF& end)` | 实现 cubic 对应的业务或工具逻辑。 |
| [L385](../src/apps/controller/ui/morph/MorphIconCore.cpp#L385) | `line` | 定义 | `void line(const QPointF& end)` | 实现 line 对应的业务或工具逻辑。 |
| [L394](../src/apps/controller/ui/morph/MorphIconCore.cpp#L394) | `quadratic` | 定义 | `void quadratic(const QPointF& control, const QPointF& end)` | 实现 quadratic 对应的业务或工具逻辑。 |
| [L400](../src/apps/controller/ui/morph/MorphIconCore.cpp#L400) | `arc` | 定义 | `void arc(double rx0, double ry0, double rotationDegrees, int large, int sweep, const QPointF& end)` | 实现 arc 对应的业务或工具逻辑。 |
| [L475](../src/apps/controller/ui/morph/MorphIconCore.cpp#L475) | `finish` | 定义 | `CubicPath finish(bool closed)` | 停止 finish 相关逻辑。 |
| [L489](../src/apps/controller/ui/morph/MorphIconCore.cpp#L489) | `lowerPath` | 定义 | `CubicPath lowerPath(const RawSubpath& raw)` | 实现 lower path 对应的业务或工具逻辑。 |
| [L506](../src/apps/controller/ui/morph/MorphIconCore.cpp#L506) | `attributeNumber` | 定义 | `double attributeNumber(const QXmlStreamAttributes& attributes, const QString& name, double fallback = 0.0)` | 实现 attribute number 对应的业务或工具逻辑。 |
| [L518](../src/apps/controller/ui/morph/MorphIconCore.cpp#L518) | `parsePointList` | 定义 | `std::vector<double> parsePointList(QString text)` | 解码或解析 parse point list 相关逻辑。 |
| [L533](../src/apps/controller/ui/morph/MorphIconCore.cpp#L533) | `polyPath` | 定义 | `CubicPath polyPath(const std::vector<double>& values, bool closed)` | 实现 poly path 对应的业务或工具逻辑。 |
| [L545](../src/apps/controller/ui/morph/MorphIconCore.cpp#L545) | `ellipsePath` | 定义 | `CubicPath ellipsePath(double cx, double cy, double rx, double ry)` | 实现 ellipse path 对应的业务或工具逻辑。 |
| [L561](../src/apps/controller/ui/morph/MorphIconCore.cpp#L561) | `rectPath` | 定义 | `CubicPath rectPath(const QXmlStreamAttributes& attributes)` | 实现 rect path 对应的业务或工具逻辑。 |
| [L605](../src/apps/controller/ui/morph/MorphIconCore.cpp#L605) | `segmentCount` | 定义 | `int segmentCount(const CubicPath& path)` | 实现 segment count 对应的业务或工具逻辑。 |
| [L610](../src/apps/controller/ui/morph/MorphIconCore.cpp#L610) | `cubicPoint` | 定义 | `QPointF cubicPoint(const CubicPath& path, int segment, double t)` | 实现 cubic point 对应的业务或工具逻辑。 |
| [L622](../src/apps/controller/ui/morph/MorphIconCore.cpp#L622) | `cubicSpeed` | 定义 | `double cubicSpeed(const CubicPath& path, int segment, double t)` | 实现 cubic speed 对应的业务或工具逻辑。 |
| [L633](../src/apps/controller/ui/morph/MorphIconCore.cpp#L633) | `segmentLength` | 定义 | `double segmentLength(const CubicPath& path, int segment, double end = 1.0)` | 实现 segment length 对应的业务或工具逻辑。 |
| [L651](../src/apps/controller/ui/morph/MorphIconCore.cpp#L651) | `endpointTangent` | 定义 | `QPointF endpointTangent(const CubicPath& path, int segment, bool atEnd)` | 停止 endpoint tangent 相关逻辑。 |
| [L669](../src/apps/controller/ui/morph/MorphIconCore.cpp#L669) | `detectCorners` | 定义 | `std::vector<int> detectCorners(const CubicPath& path)` | 实现 detect corners 对应的业务或工具逻辑。 |
| [L696](../src/apps/controller/ui/morph/MorphIconCore.cpp#L696) | `invertLength` | 定义 | `double invertLength(const CubicPath& path, int segment, double target, double length)` | 实现 invert length 对应的业务或工具逻辑。 |
| [L715](../src/apps/controller/ui/morph/MorphIconCore.cpp#L715) | `resamplePath` | 定义 | `std::vector<QPointF> resamplePath(const CubicPath& path, int sampleCount)` | 实现 resample path 对应的业务或工具逻辑。 |
| [L826](../src/apps/controller/ui/morph/MorphIconCore.cpp#L826) | `alignPair` | 定义 | `Alignment alignPair(const SampledPath& source, const SampledPath& target)` | 实现 align pair 对应的业务或工具逻辑。 |
| [L859](../src/apps/controller/ui/morph/MorphIconCore.cpp#L859) | `costMatrix` | 定义 | `std::vector<std::vector<double>> costMatrix(const std::vector<SampledPath>& source, const std::vector<SampledPath>& target)` | 实现 cost matrix 对应的业务或工具逻辑。 |
| [L875](../src/apps/controller/ui/morph/MorphIconCore.cpp#L875) | `bestPermutation` | 定义 | `std::vector<int> bestPermutation(const std::vector<std::vector<double>>& costs)` | 实现 best permutation 对应的业务或工具逻辑。 |
| [L907](../src/apps/controller/ui/morph/MorphIconCore.cpp#L907) | `bestSurjection` | 定义 | `std::vector<int> bestSurjection(const std::vector<std::vector<double>>& costs)` | 实现 best surjection 对应的业务或工具逻辑。 |
| [L956](../src/apps/controller/ui/morph/MorphIconCore.cpp#L956) | `applyGlobalAlignment` | 定义 | `void applyGlobalAlignment(MorphPlan& plan)` | 更新或应用 apply global alignment 相关逻辑。 |
| [L1004](../src/apps/controller/ui/morph/MorphIconCore.cpp#L1004) | `setError` | 定义 | `void setError(QString* output, const QString& value)` | 更新或应用 set error 相关逻辑。 |
| [L1011](../src/apps/controller/ui/morph/MorphIconCore.cpp#L1011) | `MorphIconCore::loadSvg` | 定义 | `bool MorphIconCore::loadSvg(const QString& resource, std::vector<CubicPath>& paths, QRectF& viewBox, QString* errorMessage)` | 读取或恢复 load svg 相关逻辑。 |
| [L1091](../src/apps/controller/ui/morph/MorphIconCore.cpp#L1091) | `MorphIconCore::resample` | 定义 | `std::vector<SampledPath> MorphIconCore::resample( const std::vector<CubicPath>& paths, int sampleCount)` | 实现 resample 对应的业务或工具逻辑。 |
| [L1102](../src/apps/controller/ui/morph/MorphIconCore.cpp#L1102) | `MorphIconCore::procrustes` | 定义 | `Similarity MorphIconCore::procrustes(const std::vector<QPointF>& source, const std::vector<QPointF>& target, const QPointF& sourceCentroid, const QPointF& targetCentroid)` | 实现 procrustes 对应的业务或工具逻辑。 |
| [L1134](../src/apps/controller/ui/morph/MorphIconCore.cpp#L1134) | `MorphIconCore::buildPlan` | 定义 | `MorphPlan MorphIconCore::buildPlan(const std::vector<SampledPath>& source, const std::vector<SampledPath>& target)` | 创建或初始化 build plan 相关逻辑。 |
| [L1190](../src/apps/controller/ui/morph/MorphIconCore.cpp#L1190) | `MorphIconCore::configure` | 定义 | `bool MorphIconCore::configure(const QString& sourceSvgResource, const QString& targetSvgResource, int sampleCount, QString* errorMessage)` | 更新或应用 configure 相关逻辑。 |
| [L1249](../src/apps/controller/ui/morph/MorphIconCore.cpp#L1249) | `MorphIconCore::interpolate` | 定义 | `void MorphIconCore::interpolate( double progress, std::vector<std::vector<QPointF>>& output) const` | 实现 interpolate 对应的业务或工具逻辑。 |
| [L1281](../src/apps/controller/ui/morph/MorphIconCore.cpp#L1281) | `MorphIconCore::painterPath` | 定义 | `QPainterPath MorphIconCore::painterPath(double progress) const` | 准备或呈现 painter path 相关逻辑。 |
| [L1296](../src/apps/controller/ui/morph/MorphIconCore.cpp#L1296) | `MorphIconCore::paint` | 定义 | `void MorphIconCore::paint(QPainter& painter, const QRectF& targetRect, const QColor& color, double progress, qreal strokeWidth) const` | 准备或呈现 paint 相关逻辑。 |
| [L1319](../src/apps/controller/ui/morph/MorphIconCore.cpp#L1319) | `Spring::configure` | 定义 | `void Spring::configure(double stiffness, double damping) noexcept` | 更新或应用 configure 相关逻辑。 |
| [L1325](../src/apps/controller/ui/morph/MorphIconCore.cpp#L1325) | `Spring::start` | 定义 | `void Spring::start() noexcept` | 启动 start 相关逻辑。 |
| [L1331](../src/apps/controller/ui/morph/MorphIconCore.cpp#L1331) | `Spring::step` | 定义 | `bool Spring::step(double seconds) noexcept` | 实现 step 对应的业务或工具逻辑。 |

## `src/apps/controller/ui/morph/MorphIconCore.h`

[打开源码](../src/apps/controller/ui/morph/MorphIconCore.h) · **文件作用：** 声明 morph icon core 相关类型、接口、配置和成员状态。

### 类型

| 行 | 类型 | 种类 | 作用 |
|---:|---|---|---|
| [L14](../src/apps/controller/ui/morph/MorphIconCore.h#L14) | `QPainter` | class | 定义 QPainter 的 class 类型和相关状态。 |
| [L18](../src/apps/controller/ui/morph/MorphIconCore.h#L18) | `CubicPath` | struct | 定义 CubicPath 的 struct 类型和相关状态。 |
| [L24](../src/apps/controller/ui/morph/MorphIconCore.h#L24) | `SampledPath` | struct | 定义 SampledPath 的 struct 类型和相关状态。 |
| [L29](../src/apps/controller/ui/morph/MorphIconCore.h#L29) | `Similarity` | struct | 定义 Similarity 的 struct 类型和相关状态。 |
| [L35](../src/apps/controller/ui/morph/MorphIconCore.h#L35) | `PlanItem` | struct | 定义 PlanItem 的 struct 类型和相关状态。 |
| [L51](../src/apps/controller/ui/morph/MorphIconCore.h#L51) | `MorphPlan` | struct | 定义 MorphPlan 的 struct 类型和相关状态。 |
| [L64](../src/apps/controller/ui/morph/MorphIconCore.h#L64) | `MorphIconCore` | class | MIT-licensed C++ port of Morphicons 1.7.1 core geometry. It intentionally has no DOM/web dependency: SVG resources are parsed once, then only sampled point clouds and a cached p... |
| [L102](../src/apps/controller/ui/morph/MorphIconCore.h#L102) | `Spring` | class | 定义 Spring 的 class 类型和相关状态。 |

### 成员与文件级变量

| 行 | 变量 | 声明 | 作用 |
|---:|---|---|---|
| [L14](../src/apps/controller/ui/morph/MorphIconCore.h#L14) | `QPainter` | `class QPainter;` | 保存 q painter 相关配置或运行状态。 |
| [L20](../src/apps/controller/ui/morph/MorphIconCore.h#L20) | `points` | `std::vector<QPointF> points;` | p0, c1, c2, p1, c1, c2, p2, ...; adjacent cubics share an endpoint. |
| [L21](../src/apps/controller/ui/morph/MorphIconCore.h#L21) | `closed` | `bool closed = false;` | 保存 closed 相关配置或运行状态。 |
| [L25](../src/apps/controller/ui/morph/MorphIconCore.h#L25) | `points` | `std::vector<QPointF> points;` | 保存 points 相关配置或运行状态。 |
| [L26](../src/apps/controller/ui/morph/MorphIconCore.h#L26) | `closed` | `bool closed = false;` | 保存 closed 相关配置或运行状态。 |
| [L30](../src/apps/controller/ui/morph/MorphIconCore.h#L30) | `theta` | `double theta = 0.0;` | 保存 theta 相关配置或运行状态。 |
| [L31](../src/apps/controller/ui/morph/MorphIconCore.h#L31) | `sigma` | `double sigma = 1.0;` | 保存 sigma 相关配置或运行状态。 |
| [L32](../src/apps/controller/ui/morph/MorphIconCore.h#L32) | `residual` | `double residual = 0.0;` | 保存 residual 相关配置或运行状态。 |
| [L36](../src/apps/controller/ui/morph/MorphIconCore.h#L36) | `source` | `std::vector<QPointF> source;` | 保存 source 相关配置或运行状态。 |
| [L37](../src/apps/controller/ui/morph/MorphIconCore.h#L37) | `sourceCentered` | `std::vector<QPointF> sourceCentered;` | 保存 source centered 相关配置或运行状态。 |
| [L38](../src/apps/controller/ui/morph/MorphIconCore.h#L38) | `targetAligned` | `std::vector<QPointF> targetAligned;` | 保存 target aligned 相关配置或运行状态。 |
| [L39](../src/apps/controller/ui/morph/MorphIconCore.h#L39) | `targetOriented` | `std::vector<QPointF> targetOriented;` | 保存 target oriented 相关配置或运行状态。 |
| [L40](../src/apps/controller/ui/morph/MorphIconCore.h#L40) | `sourceCentroid` | `QPointF sourceCentroid;` | 保存身份或作用域标识：source centroid。 |
| [L41](../src/apps/controller/ui/morph/MorphIconCore.h#L41) | `targetCentroid` | `QPointF targetCentroid;` | 保存身份或作用域标识：target centroid。 |
| [L42](../src/apps/controller/ui/morph/MorphIconCore.h#L42) | `theta` | `double theta = 0.0;` | 保存 theta 相关配置或运行状态。 |
| [L43](../src/apps/controller/ui/morph/MorphIconCore.h#L43) | `logSigma` | `double logSigma = 0.0;` | 保存 log sigma 相关配置或运行状态。 |
| [L44](../src/apps/controller/ui/morph/MorphIconCore.h#L44) | `residual` | `double residual = 0.0;` | 保存 residual 相关配置或运行状态。 |
| [L45](../src/apps/controller/ui/morph/MorphIconCore.h#L45) | `closed` | `bool closed = false;` | 保存 closed 相关配置或运行状态。 |
| [L46](../src/apps/controller/ui/morph/MorphIconCore.h#L46) | `hasBlockTransport` | `bool hasBlockTransport = false;` | 保存 has block transport 相关配置或运行状态。 |
| [L47](../src/apps/controller/ui/morph/MorphIconCore.h#L47) | `blockOffset` | `QPointF blockOffset;` | 保存 block offset 相关配置或运行状态。 |
| [L48](../src/apps/controller/ui/morph/MorphIconCore.h#L48) | `blockDrift` | `QPointF blockDrift;` | 保存 block drift 相关配置或运行状态。 |
| [L52](../src/apps/controller/ui/morph/MorphIconCore.h#L52) | `items` | `std::vector<PlanItem> items;` | 保存 items 相关配置或运行状态。 |
| [L53](../src/apps/controller/ui/morph/MorphIconCore.h#L53) | `sampleCount` | `int sampleCount = 0;` | 保存计数、尺寸或速率指标：sample count。 |
| [L98](../src/apps/controller/ui/morph/MorphIconCore.h#L98) | `plan_` | `MorphPlan plan_;` | 保存 plan 相关配置或运行状态。 |
| [L99](../src/apps/controller/ui/morph/MorphIconCore.h#L99) | `sharedViewBox_` | `QRectF sharedViewBox_{0.0, 0.0, 24.0, 24.0};` | 保存 shared view box 相关配置或运行状态。 |
| [L112](../src/apps/controller/ui/morph/MorphIconCore.h#L112) | `value_` | `double value_ = 1.0;` | 保存 value 相关配置或运行状态。 |
| [L113](../src/apps/controller/ui/morph/MorphIconCore.h#L113) | `velocity_` | `double velocity_ = 0.0;` | 保存 velocity 相关配置或运行状态。 |
| [L114](../src/apps/controller/ui/morph/MorphIconCore.h#L114) | `stiffness_` | `double stiffness_ = 250.0;` | 保存 stiffness 相关配置或运行状态。 |
| [L115](../src/apps/controller/ui/morph/MorphIconCore.h#L115) | `damping_` | `double damping_ = 24.0;` | 保存 damping 相关配置或运行状态。 |

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L66](../src/apps/controller/ui/morph/MorphIconCore.h#L66) | `configure` | 声明 | `bool configure(const QString& sourceSvgResource, const QString& targetSvgResource, int sampleCount = 64, QString* errorMessage = nullptr)` | 更新或应用 configure 相关逻辑。 |
| [L74](../src/apps/controller/ui/morph/MorphIconCore.h#L74) | `interpolate` | 声明 | `void interpolate(double progress, std::vector<std::vector<QPointF>>& output) const` | 实现 interpolate 对应的业务或工具逻辑。 |
| [L76](../src/apps/controller/ui/morph/MorphIconCore.h#L76) | `painterPath` | 声明 | `[[nodiscard]] QPainterPath painterPath(double progress) const` | 准备或呈现 painter path 相关逻辑。 |
| [L77](../src/apps/controller/ui/morph/MorphIconCore.h#L77) | `paint` | 声明 | `void paint(QPainter& painter, const QRectF& targetRect, const QColor& color, double progress, qreal strokeWidth = 2.0) const` | 准备或呈现 paint 相关逻辑。 |
| [L83](../src/apps/controller/ui/morph/MorphIconCore.h#L83) | `loadSvg` | 声明 | `static bool loadSvg(const QString& resource, std::vector<CubicPath>& paths, QRectF& viewBox, QString* errorMessage = nullptr)` | 读取或恢复 load svg 相关逻辑。 |
| [L87](../src/apps/controller/ui/morph/MorphIconCore.h#L87) | `resample` | 声明 | `static std::vector<SampledPath> resample( const std::vector<CubicPath>& paths, int sampleCount = 64)` | 实现 resample 对应的业务或工具逻辑。 |
| [L90](../src/apps/controller/ui/morph/MorphIconCore.h#L90) | `buildPlan` | 声明 | `static MorphPlan buildPlan(const std::vector<SampledPath>& source, const std::vector<SampledPath>& target)` | 创建或初始化 build plan 相关逻辑。 |
| [L92](../src/apps/controller/ui/morph/MorphIconCore.h#L92) | `procrustes` | 声明 | `static Similarity procrustes(const std::vector<QPointF>& source, const std::vector<QPointF>& target, const QPointF& sourceCentroid, const QPointF& targetCentroid)` | 实现 procrustes 对应的业务或工具逻辑。 |
| [L104](../src/apps/controller/ui/morph/MorphIconCore.h#L104) | `configure` | 声明 | `void configure(double stiffness, double damping) noexcept` | 更新或应用 configure 相关逻辑。 |
| [L105](../src/apps/controller/ui/morph/MorphIconCore.h#L105) | `start` | 声明 | `void start() noexcept` | 启动 start 相关逻辑。 |
| [L106](../src/apps/controller/ui/morph/MorphIconCore.h#L106) | `step` | 声明 | `bool step(double seconds) noexcept` | 实现 step 对应的业务或工具逻辑。 |

## `src/apps/controller/ui/RemoteCTheme.cpp`

[打开源码](../src/apps/controller/ui/RemoteCTheme.cpp) · **文件作用：** 实现 remote c theme 相关函数与文件级辅助逻辑。

### 成员与文件级变量

| 行 | 变量 | 声明 | 作用 |
|---:|---|---|---|
| [L21](../src/apps/controller/ui/RemoteCTheme.cpp#L21) | `kThemeIconPath` | `constexpr auto kThemeIconPath = "remoteCThemeIconPath";` | 定义 theme icon path 的编译期常量或产品边界。 |
| [L22](../src/apps/controller/ui/RemoteCTheme.cpp#L22) | `kThemeIconTone` | `constexpr auto kThemeIconTone = "remoteCThemeIconTone";` | 定义 theme icon tone 的编译期常量或产品边界。 |
| [L23](../src/apps/controller/ui/RemoteCTheme.cpp#L23) | `kThemeIconSize` | `constexpr auto kThemeIconSize = "remoteCThemeIconSize";` | 定义 theme icon size 的编译期常量或产品边界。 |

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L26](../src/apps/controller/ui/RemoteCTheme.cpp#L26) | `RemoteCTheme::Color` | 定义 | `QColor RemoteCTheme::Color(ThemeColor color)` | 实现 color 对应的业务或工具逻辑。 |
| [L59](../src/apps/controller/ui/RemoteCTheme.cpp#L59) | `RemoteCTheme::LoadStyleSheet` | 定义 | `QString RemoteCTheme::LoadStyleSheet(const QString& resourcePath)` | 读取或恢复 load style sheet 相关逻辑。 |
| [L68](../src/apps/controller/ui/RemoteCTheme.cpp#L68) | `RemoteCTheme::LightStyleSheet` | 定义 | `QString RemoteCTheme::LightStyleSheet()` | 实现 light style sheet 对应的业务或工具逻辑。 |
| [L74](../src/apps/controller/ui/RemoteCTheme.cpp#L74) | `RemoteCTheme::DarkStyleSheet` | 定义 | `QString RemoteCTheme::DarkStyleSheet()` | 实现 dark style sheet 对应的业务或工具逻辑。 |
| [L80](../src/apps/controller/ui/RemoteCTheme.cpp#L80) | `RemoteCTheme::MainWindowColorOverrides` | 定义 | `QString RemoteCTheme::MainWindowColorOverrides(bool dark)` | 实现 main window color overrides 对应的业务或工具逻辑。 |
| [L87](../src/apps/controller/ui/RemoteCTheme.cpp#L87) | `RemoteCTheme::RemoteDarkStyleSheet` | 定义 | `QString RemoteCTheme::RemoteDarkStyleSheet()` | 实现 remote dark style sheet 对应的业务或工具逻辑。 |
| [L93](../src/apps/controller/ui/RemoteCTheme.cpp#L93) | `RemoteCTheme::PageStyleSheet` | 定义 | `QString RemoteCTheme::PageStyleSheet(const QString& resourcePath, bool dark)` | 实现 page style sheet 对应的业务或工具逻辑。 |
| [L102](../src/apps/controller/ui/RemoteCTheme.cpp#L102) | `RemoteCTheme::LoadPreference` | 定义 | `ThemePreference RemoteCTheme::LoadPreference()` | 读取或恢复 load preference 相关逻辑。 |
| [L109](../src/apps/controller/ui/RemoteCTheme.cpp#L109) | `RemoteCTheme::SavePreference` | 定义 | `void RemoteCTheme::SavePreference(ThemePreference preference)` | 保存或写入 save preference 相关逻辑。 |
| [L116](../src/apps/controller/ui/RemoteCTheme.cpp#L116) | `RemoteCTheme::PreferenceValue` | 定义 | `QString RemoteCTheme::PreferenceValue(ThemePreference preference)` | 实现 preference value 对应的业务或工具逻辑。 |
| [L126](../src/apps/controller/ui/RemoteCTheme.cpp#L126) | `RemoteCTheme::PreferenceFromValue` | 定义 | `ThemePreference RemoteCTheme::PreferenceFromValue(const QString& value)` | 实现 preference from value 对应的业务或工具逻辑。 |
| [L138](../src/apps/controller/ui/RemoteCTheme.cpp#L138) | `RemoteCTheme::IsDark` | 定义 | `bool RemoteCTheme::IsDark(ThemePreference preference)` | 判断 is dark 相关逻辑。 |
| [L149](../src/apps/controller/ui/RemoteCTheme.cpp#L149) | `RemoteCTheme::IconColor` | 定义 | `QColor RemoteCTheme::IconColor(ThemeIconTone tone, bool dark)` | 实现 icon color 对应的业务或工具逻辑。 |
| [L174](../src/apps/controller/ui/RemoteCTheme.cpp#L174) | `RemoteCTheme::Icon` | 定义 | `QIcon RemoteCTheme::Icon(const QString& resourcePath, ThemeIconTone tone, QSize logicalSize)` | 实现 icon 对应的业务或工具逻辑。 |
| [L193](../src/apps/controller/ui/RemoteCTheme.cpp#L193) | `RemoteCTheme::SetIcon` | 定义 | `void RemoteCTheme::SetIcon(QAbstractButton* button, const QString& resourcePath, ThemeIconTone tone)` | 更新或应用 set icon 相关逻辑。 |
| [L203](../src/apps/controller/ui/RemoteCTheme.cpp#L203) | `RemoteCTheme::SetIcon` | 定义 | `void RemoteCTheme::SetIcon(QAction* action, const QString& resourcePath, ThemeIconTone tone)` | 更新或应用 set icon 相关逻辑。 |
| [L212](../src/apps/controller/ui/RemoteCTheme.cpp#L212) | `RemoteCTheme::SetPixmap` | 定义 | `void RemoteCTheme::SetPixmap(QLabel* label, const QString& resourcePath, QSize logicalSize, ThemeIconTone tone)` | 更新或应用 set pixmap 相关逻辑。 |
| [L223](../src/apps/controller/ui/RemoteCTheme.cpp#L223) | `RemoteCTheme::RefreshIcons` | 定义 | `void RemoteCTheme::RefreshIcons(QWidget* root)` | 刷新 refresh icons 相关逻辑。 |
| [L250](../src/apps/controller/ui/RemoteCTheme.cpp#L250) | `RemoteCTheme::ApplyLight` | 定义 | `void RemoteCTheme::ApplyLight(QWidget* widget)` | 更新或应用 apply light 相关逻辑。 |
| [L257](../src/apps/controller/ui/RemoteCTheme.cpp#L257) | `RemoteCTheme::ApplyDark` | 定义 | `void RemoteCTheme::ApplyDark(QWidget* widget)` | 更新或应用 apply dark 相关逻辑。 |
| [L264](../src/apps/controller/ui/RemoteCTheme.cpp#L264) | `RemoteCTheme::ApplyRemoteDark` | 定义 | `void RemoteCTheme::ApplyRemoteDark(QWidget* widget)` | 更新或应用 apply remote dark 相关逻辑。 |

## `src/apps/controller/ui/RemoteCTheme.h`

[打开源码](../src/apps/controller/ui/RemoteCTheme.h) · **文件作用：** 声明 remote c theme 相关类型、接口、配置和成员状态。

### 类型

| 行 | 类型 | 种类 | 作用 |
|---:|---|---|---|
| [L11](../src/apps/controller/ui/RemoteCTheme.h#L11) | `QAbstractButton` | class | 定义 QAbstractButton 的 class 类型和相关状态。 |
| [L12](../src/apps/controller/ui/RemoteCTheme.h#L12) | `QAction` | class | 定义 QAction 的 class 类型和相关状态。 |
| [L13](../src/apps/controller/ui/RemoteCTheme.h#L13) | `QLabel` | class | 定义 QLabel 的 class 类型和相关状态。 |
| [L14](../src/apps/controller/ui/RemoteCTheme.h#L14) | `QWidget` | class | 定义 QWidget 的 class 类型和相关状态。 |
| [L18](../src/apps/controller/ui/RemoteCTheme.h#L18) | `ThemePreference` | enum class | 定义 ThemePreference 的 enum class 类型和相关状态。 |
| [L24](../src/apps/controller/ui/RemoteCTheme.h#L24) | `ThemeColor` | enum class | 定义 ThemeColor 的 enum class 类型和相关状态。 |
| [L53](../src/apps/controller/ui/RemoteCTheme.h#L53) | `ThemeIconTone` | enum class | 定义 ThemeIconTone 的 enum class 类型和相关状态。 |
| [L62](../src/apps/controller/ui/RemoteCTheme.h#L62) | `RemoteCTheme` | class | 定义 RemoteCTheme 的 class 类型和相关状态。 |

### 成员与文件级变量

| 行 | 变量 | 声明 | 作用 |
|---:|---|---|---|
| [L11](../src/apps/controller/ui/RemoteCTheme.h#L11) | `QAbstractButton` | `class QAbstractButton;` | 保存 q abstract button 相关配置或运行状态。 |
| [L12](../src/apps/controller/ui/RemoteCTheme.h#L12) | `QAction` | `class QAction;` | 保存 q action 相关配置或运行状态。 |
| [L13](../src/apps/controller/ui/RemoteCTheme.h#L13) | `QLabel` | `class QLabel;` | 保存路径、地址或显示名称：q label。 |
| [L14](../src/apps/controller/ui/RemoteCTheme.h#L14) | `QWidget` | `class QWidget;` | 保存 q widget 相关配置或运行状态。 |

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L64](../src/apps/controller/ui/RemoteCTheme.h#L64) | `Color` | 声明 | `static QColor Color(ThemeColor color)` | 实现 color 对应的业务或工具逻辑。 |
| [L65](../src/apps/controller/ui/RemoteCTheme.h#L65) | `LightStyleSheet` | 声明 | `static QString LightStyleSheet()` | 实现 light style sheet 对应的业务或工具逻辑。 |
| [L66](../src/apps/controller/ui/RemoteCTheme.h#L66) | `DarkStyleSheet` | 声明 | `static QString DarkStyleSheet()` | 实现 dark style sheet 对应的业务或工具逻辑。 |
| [L69](../src/apps/controller/ui/RemoteCTheme.h#L69) | `MainWindowColorOverrides` | 声明 | `static QString MainWindowColorOverrides(bool dark)` | Main-window geometry lives in kMainStyle. Theme switching may append only color overrides so a round trip can never change layout metrics. |
| [L70](../src/apps/controller/ui/RemoteCTheme.h#L70) | `RemoteDarkStyleSheet` | 声明 | `static QString RemoteDarkStyleSheet()` | 实现 remote dark style sheet 对应的业务或工具逻辑。 |
| [L71](../src/apps/controller/ui/RemoteCTheme.h#L71) | `PageStyleSheet` | 声明 | `static QString PageStyleSheet(const QString& resourcePath, bool dark)` | 实现 page style sheet 对应的业务或工具逻辑。 |
| [L72](../src/apps/controller/ui/RemoteCTheme.h#L72) | `LoadPreference` | 声明 | `static ThemePreference LoadPreference()` | 读取或恢复 load preference 相关逻辑。 |
| [L73](../src/apps/controller/ui/RemoteCTheme.h#L73) | `SavePreference` | 声明 | `static void SavePreference(ThemePreference preference)` | 保存或写入 save preference 相关逻辑。 |
| [L74](../src/apps/controller/ui/RemoteCTheme.h#L74) | `PreferenceValue` | 声明 | `static QString PreferenceValue(ThemePreference preference)` | 实现 preference value 对应的业务或工具逻辑。 |
| [L75](../src/apps/controller/ui/RemoteCTheme.h#L75) | `PreferenceFromValue` | 声明 | `static ThemePreference PreferenceFromValue(const QString& value)` | 实现 preference from value 对应的业务或工具逻辑。 |
| [L76](../src/apps/controller/ui/RemoteCTheme.h#L76) | `IsDark` | 声明 | `static bool IsDark(ThemePreference preference)` | 判断 is dark 相关逻辑。 |
| [L77](../src/apps/controller/ui/RemoteCTheme.h#L77) | `Icon` | 声明 | `static QIcon Icon(const QString& resourcePath, ThemeIconTone tone = ThemeIconTone::kNeutral, QSize logicalSize = QSize(24, 24))` | 实现 icon 对应的业务或工具逻辑。 |
| [L80](../src/apps/controller/ui/RemoteCTheme.h#L80) | `SetIcon` | 声明 | `static void SetIcon(QAbstractButton* button, const QString& resourcePath, ThemeIconTone tone = ThemeIconTone::kNeutral)` | 更新或应用 set icon 相关逻辑。 |
| [L82](../src/apps/controller/ui/RemoteCTheme.h#L82) | `SetIcon` | 声明 | `static void SetIcon(QAction* action, const QString& resourcePath, ThemeIconTone tone = ThemeIconTone::kNeutral)` | 更新或应用 set icon 相关逻辑。 |
| [L84](../src/apps/controller/ui/RemoteCTheme.h#L84) | `SetPixmap` | 声明 | `static void SetPixmap(QLabel* label, const QString& resourcePath, QSize logicalSize, ThemeIconTone tone = ThemeIconTone::kNeutral)` | 更新或应用 set pixmap 相关逻辑。 |
| [L87](../src/apps/controller/ui/RemoteCTheme.h#L87) | `RefreshIcons` | 声明 | `static void RefreshIcons(QWidget* root)` | 刷新 refresh icons 相关逻辑。 |
| [L88](../src/apps/controller/ui/RemoteCTheme.h#L88) | `ApplyLight` | 声明 | `static void ApplyLight(QWidget* widget)` | 更新或应用 apply light 相关逻辑。 |
| [L89](../src/apps/controller/ui/RemoteCTheme.h#L89) | `ApplyDark` | 声明 | `static void ApplyDark(QWidget* widget)` | 更新或应用 apply dark 相关逻辑。 |
| [L90](../src/apps/controller/ui/RemoteCTheme.h#L90) | `ApplyRemoteDark` | 声明 | `static void ApplyRemoteDark(QWidget* widget)` | 更新或应用 apply remote dark 相关逻辑。 |
| [L93](../src/apps/controller/ui/RemoteCTheme.h#L93) | `LoadStyleSheet` | 声明 | `static QString LoadStyleSheet(const QString& resourcePath)` | 读取或恢复 load style sheet 相关逻辑。 |
| [L94](../src/apps/controller/ui/RemoteCTheme.h#L94) | `IconColor` | 声明 | `static QColor IconColor(ThemeIconTone tone, bool dark)` | 实现 icon color 对应的业务或工具逻辑。 |
