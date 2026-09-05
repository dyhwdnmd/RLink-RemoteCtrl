# 应用层：会话引擎与业务控制器

> 自动生成于 2026-09-06，源码树 `1ae783c8db12-dirty`。请运行 `tools/Generate-SourceSymbolReference.ps1` 刷新。

direct/room 会话引擎、媒体路由、文件传输、远程粘贴与缓存状态机。

本册共收录 50 个源码文件。函数与变量的中文作用优先采用源码紧邻注释；无注释时根据符号命名生成阅读提示，最终语义仍以源码为准。

## `src/apps/remote/ClipboardCacheManager.cpp`

[打开源码](../src/apps/remote/ClipboardCacheManager.cpp) · **文件作用：** 实现 clipboard cache manager 相关函数与文件级辅助逻辑。

### 类型

| 行 | 类型 | 种类 | 作用 |
|---:|---|---|---|
| [L356](../src/apps/remote/ClipboardCacheManager.cpp#L356) | `Candidate` | struct | 定义 Candidate 的 struct 类型和相关状态。 |

### 成员与文件级变量

| 行 | 变量 | 声明 | 作用 |
|---:|---|---|---|
| [L20](../src/apps/remote/ClipboardCacheManager.cpp#L20) | `kGiB` | `constexpr std::uint64_t kGiB = 1024ull * 1024 * 1024;` | 定义 gi b 的编译期常量或产品边界。 |
| [L21](../src/apps/remote/ClipboardCacheManager.cpp#L21) | `kReserveBytes` | `constexpr std::uint64_t kReserveBytes = 256ull * 1024 * 1024;` | 定义 reserve bytes 的编译期常量或产品边界。 |
| [L24](../src/apps/remote/ClipboardCacheManager.cpp#L24) | `kMarkerName` | `constexpr wchar_t kMarkerName[] = L".remotec-cache";` | 定义 marker name 的编译期常量或产品边界。 |

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L26](../src/apps/remote/ClipboardCacheManager.cpp#L26) | `DefaultBaseDirectory` | 定义 | `std::filesystem::path DefaultBaseDirectory()` | 实现 default base directory 对应的业务或工具逻辑。 |
| [L42](../src/apps/remote/ClipboardCacheManager.cpp#L42) | `DirectoryBytes` | 定义 | `std::uint64_t DirectoryBytes(const std::filesystem::path& root)` | 实现 directory bytes 对应的业务或工具逻辑。 |
| [L64](../src/apps/remote/ClipboardCacheManager.cpp#L64) | `StartsWithManagedPrefix` | 定义 | `bool StartsWithManagedPrefix(const std::filesystem::path& path)` | 判断 starts with managed prefix 相关逻辑。 |
| [L73](../src/apps/remote/ClipboardCacheManager.cpp#L73) | `ClipboardCacheManager::ClipboardCacheManager` | 定义 | `ClipboardCacheManager::ClipboardCacheManager()` | 构造并初始化 ClipboardCacheManager 实例。 |
| [L81](../src/apps/remote/ClipboardCacheManager.cpp#L81) | `ClipboardCacheManager::~ClipboardCacheManager` | 定义 | `ClipboardCacheManager::~ClipboardCacheManager()` | 停止相关活动并释放 ClipboardCacheManager 实例拥有的资源。 |
| [L92](../src/apps/remote/ClipboardCacheManager.cpp#L92) | `ClipboardCacheManager::SetConfiguration` | 定义 | `void ClipboardCacheManager::SetConfiguration( ClipboardCacheConfiguration configuration)` | 更新或应用 set configuration 相关逻辑。 |
| [L115](../src/apps/remote/ClipboardCacheManager.cpp#L115) | `ClipboardCacheManager::Snapshot` | 定义 | `ClipboardCacheSnapshot ClipboardCacheManager::Snapshot() const` | 查询并返回 snapshot 相关逻辑。 |
| [L122](../src/apps/remote/ClipboardCacheManager.cpp#L122) | `ClipboardCacheManager::PrepareEntry` | 定义 | `bool ClipboardCacheManager::PrepareEntry( const std::string& itemId, std::uint64_t incomingBytes, std::filesystem::path* entryDirectory, std::string* error)` | 实现 prepare entry 对应的业务或工具逻辑。 |
| [L160](../src/apps/remote/ClipboardCacheManager.cpp#L160) | `ClipboardCacheManager::ProtectEntry` | 定义 | `void ClipboardCacheManager::ProtectEntry( const std::filesystem::path& entryDirectory)` | 实现 protect entry 对应的业务或工具逻辑。 |
| [L167](../src/apps/remote/ClipboardCacheManager.cpp#L167) | `ClipboardCacheManager::MarkCompleted` | 定义 | `void ClipboardCacheManager::MarkCompleted( const std::filesystem::path& entryDirectory)` | 实现 mark completed 对应的业务或工具逻辑。 |
| [L181](../src/apps/remote/ClipboardCacheManager.cpp#L181) | `ClipboardCacheManager::RetireCurrentEntry` | 定义 | `void ClipboardCacheManager::RetireCurrentEntry()` | 实现 retire current entry 对应的业务或工具逻辑。 |
| [L191](../src/apps/remote/ClipboardCacheManager.cpp#L191) | `ClipboardCacheManager::RemoveEntry` | 定义 | `void ClipboardCacheManager::RemoveEntry( const std::filesystem::path& entryDirectory)` | 重置或移除 remove entry 相关逻辑。 |
| [L207](../src/apps/remote/ClipboardCacheManager.cpp#L207) | `ClipboardCacheManager::RequestRemoveEntry` | 定义 | `void ClipboardCacheManager::RequestRemoveEntry( const std::filesystem::path& entryDirectory)` | 发起请求或查询 request remove entry 相关逻辑。 |
| [L229](../src/apps/remote/ClipboardCacheManager.cpp#L229) | `ClipboardCacheManager::RequestCleanup` | 定义 | `void ClipboardCacheManager::RequestCleanup( bool aggressive, std::function<void()> completion)` | 发起请求或查询 request cleanup 相关逻辑。 |
| [L243](../src/apps/remote/ClipboardCacheManager.cpp#L243) | `ClipboardCacheManager::MaintenanceMain` | 定义 | `void ClipboardCacheManager::MaintenanceMain(std::stop_token stopToken)` | 实现 maintenance main 对应的业务或工具逻辑。 |
| [L283](../src/apps/remote/ClipboardCacheManager.cpp#L283) | `ClipboardCacheManager::ScanLocked` | 定义 | `ClipboardCacheSnapshot ClipboardCacheManager::ScanLocked() const` | 实现 scan locked 对应的业务或工具逻辑。 |
| [L316](../src/apps/remote/ClipboardCacheManager.cpp#L316) | `ClipboardCacheManager::RootLocked` | 定义 | `std::filesystem::path ClipboardCacheManager::RootLocked() const` | 实现 root locked 对应的业务或工具逻辑。 |
| [L322](../src/apps/remote/ClipboardCacheManager.cpp#L322) | `ClipboardCacheManager::EffectiveLimitLocked` | 定义 | `std::uint64_t ClipboardCacheManager::EffectiveLimitLocked( std::uint64_t capacityPoolBytes) const` | 实现 effective limit locked 对应的业务或工具逻辑。 |
| [L331](../src/apps/remote/ClipboardCacheManager.cpp#L331) | `ClipboardCacheManager::CleanupLocked` | 定义 | `bool ClipboardCacheManager::CleanupLocked( std::uint64_t incomingBytes, bool aggressive, std::string* error)` | 实现 cleanup locked 对应的业务或工具逻辑。 |
| [L472](../src/apps/remote/ClipboardCacheManager.cpp#L472) | `ClipboardCacheManager::IsManagedEntryLocked` | 定义 | `bool ClipboardCacheManager::IsManagedEntryLocked( const std::filesystem::path& path) const` | 判断 is managed entry locked 相关逻辑。 |
| [L488](../src/apps/remote/ClipboardCacheManager.cpp#L488) | `ClipboardCacheManager::IsProtectedLocked` | 定义 | `bool ClipboardCacheManager::IsProtectedLocked( const std::filesystem::path& path) const` | 判断 is protected locked 相关逻辑。 |
| [L503](../src/apps/remote/ClipboardCacheManager.cpp#L503) | `ClipboardCacheManager::WriteMarkerLocked` | 定义 | `void ClipboardCacheManager::WriteMarkerLocked( const std::filesystem::path& entry, const char* state) const` | 保存或写入 write marker locked 相关逻辑。 |
| [L511](../src/apps/remote/ClipboardCacheManager.cpp#L511) | `ClipboardCacheManager::TouchRetiredLocked` | 定义 | `void ClipboardCacheManager::TouchRetiredLocked( const std::filesystem::path& entry) const` | 实现 touch retired locked 对应的业务或工具逻辑。 |

## `src/apps/remote/ClipboardCacheManager.h`

[打开源码](../src/apps/remote/ClipboardCacheManager.h) · **文件作用：** 声明 clipboard cache manager 相关类型、接口、配置和成员状态。

### 类型

| 行 | 类型 | 种类 | 作用 |
|---:|---|---|---|
| [L18](../src/apps/remote/ClipboardCacheManager.h#L18) | `ClipboardCacheConfiguration` | struct | 定义 ClipboardCacheConfiguration 的 struct 类型和相关状态。 |
| [L26](../src/apps/remote/ClipboardCacheManager.h#L26) | `ClipboardCacheSnapshot` | struct | 定义 ClipboardCacheSnapshot 的 struct 类型和相关状态。 |
| [L36](../src/apps/remote/ClipboardCacheManager.h#L36) | `ClipboardCacheManager` | class | 定义 ClipboardCacheManager 的 class 类型和相关状态。 |

### 成员与文件级变量

| 行 | 变量 | 声明 | 作用 |
|---:|---|---|---|
| [L21](../src/apps/remote/ClipboardCacheManager.h#L21) | `baseDirectory` | `std::filesystem::path baseDirectory;` | The user selects this parent. RemoteC always appends its own dedicated RemoteC/ClipboardCache directory and never removes arbitrary siblings. |
| [L22](../src/apps/remote/ClipboardCacheManager.h#L22) | `retention` | `std::chrono::minutes retention{60};` | 保存 retention 相关配置或运行状态。 |
| [L23](../src/apps/remote/ClipboardCacheManager.h#L23) | `requestedLimitBytes` | `std::uint64_t requestedLimitBytes = 2ull * 1024 * 1024 * 1024;` | 保存计数、尺寸或速率指标：requested limit bytes。 |
| [L27](../src/apps/remote/ClipboardCacheManager.h#L27) | `root` | `std::filesystem::path root;` | 保存 root 相关配置或运行状态。 |
| [L28](../src/apps/remote/ClipboardCacheManager.h#L28) | `totalBytes` | `std::uint64_t totalBytes = 0;` | 保存计数、尺寸或速率指标：total bytes。 |
| [L29](../src/apps/remote/ClipboardCacheManager.h#L29) | `entryCount` | `std::uint64_t entryCount = 0;` | 保存计数、尺寸或速率指标：entry count。 |
| [L30](../src/apps/remote/ClipboardCacheManager.h#L30) | `availableBytes` | `std::uint64_t availableBytes = 0;` | 保存计数、尺寸或速率指标：available bytes。 |
| [L31](../src/apps/remote/ClipboardCacheManager.h#L31) | `safeCapacityBytes` | `std::uint64_t safeCapacityBytes = 0;` | 保存计数、尺寸或速率指标：safe capacity bytes。 |
| [L32](../src/apps/remote/ClipboardCacheManager.h#L32) | `effectiveLimitBytes` | `std::uint64_t effectiveLimitBytes = 0;` | 保存计数、尺寸或速率指标：effective limit bytes。 |
| [L33](../src/apps/remote/ClipboardCacheManager.h#L33) | `lastError` | `std::string lastError;` | 保存最近错误或失败原因：last error。 |
| [L77](../src/apps/remote/ClipboardCacheManager.h#L77) | `mutex_` | `mutable std::mutex mutex_;` | 保护跨线程共享状态：mutex。 |
| [L78](../src/apps/remote/ClipboardCacheManager.h#L78) | `wake_` | `std::condition_variable_any wake_;` | 保存 wake 相关配置或运行状态。 |
| [L79](../src/apps/remote/ClipboardCacheManager.h#L79) | `configuration_` | `ClipboardCacheConfiguration configuration_;` | 保存 configuration 相关配置或运行状态。 |
| [L80](../src/apps/remote/ClipboardCacheManager.h#L80) | `retiredRoots_` | `std::vector<std::filesystem::path> retiredRoots_;` | 保存 retired roots 相关配置或运行状态。 |
| [L81](../src/apps/remote/ClipboardCacheManager.h#L81) | `pendingRemovals_` | `std::vector<std::filesystem::path> pendingRemovals_;` | 保存 pending removals 相关配置或运行状态。 |
| [L83](../src/apps/remote/ClipboardCacheManager.h#L83) | `receivingEntry_` | `std::filesystem::path receivingEntry_;` | 保存 receiving entry 相关配置或运行状态。 |
| [L84](../src/apps/remote/ClipboardCacheManager.h#L84) | `currentClipboardEntry_` | `std::filesystem::path currentClipboardEntry_;` | 保存 current clipboard entry 相关配置或运行状态。 |
| [L85](../src/apps/remote/ClipboardCacheManager.h#L85) | `lastSnapshot_` | `mutable ClipboardCacheSnapshot lastSnapshot_;` | 保存可跨层读取的状态快照：last snapshot。 |
| [L86](../src/apps/remote/ClipboardCacheManager.h#L86) | `cleanupRequested_` | `bool cleanupRequested_ = false;` | 保存 cleanup requested 相关配置或运行状态。 |
| [L87](../src/apps/remote/ClipboardCacheManager.h#L87) | `aggressiveCleanupRequested_` | `bool aggressiveCleanupRequested_ = false;` | 保存 aggressive cleanup requested 相关配置或运行状态。 |
| [L88](../src/apps/remote/ClipboardCacheManager.h#L88) | `stopping_` | `bool stopping_ = false;` | 保存 stopping 相关配置或运行状态。 |
| [L89](../src/apps/remote/ClipboardCacheManager.h#L89) | `maintenance_` | `std::jthread maintenance_;` | 保存 maintenance 相关配置或运行状态。 |

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L38](../src/apps/remote/ClipboardCacheManager.h#L38) | `ClipboardCacheManager` | 声明 | `ClipboardCacheManager()` | 实现 clipboard cache manager 对应的业务或工具逻辑。 |
| [L39](../src/apps/remote/ClipboardCacheManager.h#L39) | `~ClipboardCacheManager` | 声明 | `~ClipboardCacheManager()` | 停止相关活动并释放 ClipboardCacheManager 实例拥有的资源。 |
| [L41](../src/apps/remote/ClipboardCacheManager.h#L41) | `ClipboardCacheManager` | 声明 | `ClipboardCacheManager(const ClipboardCacheManager&) = delete` | 实现 clipboard cache manager 对应的业务或工具逻辑。 |
| [L44](../src/apps/remote/ClipboardCacheManager.h#L44) | `SetConfiguration` | 声明 | `void SetConfiguration(ClipboardCacheConfiguration configuration)` | 更新或应用 set configuration 相关逻辑。 |
| [L45](../src/apps/remote/ClipboardCacheManager.h#L45) | `Snapshot` | 声明 | `ClipboardCacheSnapshot Snapshot() const` | 查询并返回 snapshot 相关逻辑。 |
| [L47](../src/apps/remote/ClipboardCacheManager.h#L47) | `PrepareEntry` | 声明 | `bool PrepareEntry(const std::string& itemId, std::uint64_t incomingBytes, std::filesystem::path* entryDirectory, std::string* error)` | 实现 prepare entry 对应的业务或工具逻辑。 |
| [L51](../src/apps/remote/ClipboardCacheManager.h#L51) | `ProtectEntry` | 声明 | `void ProtectEntry(const std::filesystem::path& entryDirectory)` | 实现 protect entry 对应的业务或工具逻辑。 |
| [L52](../src/apps/remote/ClipboardCacheManager.h#L52) | `MarkCompleted` | 声明 | `void MarkCompleted(const std::filesystem::path& entryDirectory)` | 实现 mark completed 对应的业务或工具逻辑。 |
| [L53](../src/apps/remote/ClipboardCacheManager.h#L53) | `RetireCurrentEntry` | 声明 | `void RetireCurrentEntry()` | 实现 retire current entry 对应的业务或工具逻辑。 |
| [L54](../src/apps/remote/ClipboardCacheManager.h#L54) | `RemoveEntry` | 声明 | `void RemoveEntry(const std::filesystem::path& entryDirectory)` | 重置或移除 remove entry 相关逻辑。 |
| [L58](../src/apps/remote/ClipboardCacheManager.h#L58) | `RequestRemoveEntry` | 声明 | `void RequestRemoveEntry(const std::filesystem::path& entryDirectory)` | Detaches an incomplete entry synchronously, but performs recursive deletion only on the maintenance thread. Cancel must never make the clipboard/video worker walk hundreds of ca... |
| [L59](../src/apps/remote/ClipboardCacheManager.h#L59) | `RequestCleanup` | 定义 | `void RequestCleanup(bool aggressive = false, std::function<void()> completion = {})` | 发起请求或查询 request cleanup 相关逻辑。 |
| [L63](../src/apps/remote/ClipboardCacheManager.h#L63) | `MaintenanceMain` | 声明 | `void MaintenanceMain(std::stop_token stopToken)` | 实现 maintenance main 对应的业务或工具逻辑。 |
| [L64](../src/apps/remote/ClipboardCacheManager.h#L64) | `ScanLocked` | 声明 | `ClipboardCacheSnapshot ScanLocked() const` | 实现 scan locked 对应的业务或工具逻辑。 |
| [L65](../src/apps/remote/ClipboardCacheManager.h#L65) | `RootLocked` | 声明 | `std::filesystem::path RootLocked() const` | 实现 root locked 对应的业务或工具逻辑。 |
| [L66](../src/apps/remote/ClipboardCacheManager.h#L66) | `EffectiveLimitLocked` | 声明 | `std::uint64_t EffectiveLimitLocked( std::uint64_t availableBytes) const` | 实现 effective limit locked 对应的业务或工具逻辑。 |
| [L68](../src/apps/remote/ClipboardCacheManager.h#L68) | `CleanupLocked` | 声明 | `bool CleanupLocked(std::uint64_t incomingBytes, bool aggressive, std::string* error)` | 实现 cleanup locked 对应的业务或工具逻辑。 |
| [L71](../src/apps/remote/ClipboardCacheManager.h#L71) | `IsManagedEntryLocked` | 声明 | `bool IsManagedEntryLocked(const std::filesystem::path& path) const` | 判断 is managed entry locked 相关逻辑。 |
| [L72](../src/apps/remote/ClipboardCacheManager.h#L72) | `IsProtectedLocked` | 声明 | `bool IsProtectedLocked(const std::filesystem::path& path) const` | 判断 is protected locked 相关逻辑。 |
| [L73](../src/apps/remote/ClipboardCacheManager.h#L73) | `WriteMarkerLocked` | 声明 | `void WriteMarkerLocked(const std::filesystem::path& entry, const char* state) const` | 保存或写入 write marker locked 相关逻辑。 |
| [L75](../src/apps/remote/ClipboardCacheManager.h#L75) | `TouchRetiredLocked` | 声明 | `void TouchRetiredLocked(const std::filesystem::path& entry) const` | 实现 touch retired locked 对应的业务或工具逻辑。 |

## `src/apps/remote/ClipboardController.cpp`

[打开源码](../src/apps/remote/ClipboardController.cpp) · **文件作用：** 实现 clipboard controller 相关函数与文件级辅助逻辑。

### 类型

| 行 | 类型 | 种类 | 作用 |
|---:|---|---|---|
| [L232](../src/apps/remote/ClipboardController.cpp#L232) | `IncrementalSha256` | class | 定义 IncrementalSha256 的 class 类型和相关状态。 |
| [L548](../src/apps/remote/ClipboardController.cpp#L548) | `ClipboardController::TransferPart` | struct | 定义 ClipboardController::TransferPart 的 struct 类型和相关状态。 |
| [L563](../src/apps/remote/ClipboardController.cpp#L563) | `ClipboardController::OutgoingTransfer` | struct | 定义 ClipboardController::OutgoingTransfer 的 struct 类型和相关状态。 |
| [L564](../src/apps/remote/ClipboardController.cpp#L564) | `RateSample` | struct | 定义 RateSample 的 struct 类型和相关状态。 |
| [L588](../src/apps/remote/ClipboardController.cpp#L588) | `ClipboardController::IncomingTransfer` | struct | 定义 ClipboardController::IncomingTransfer 的 struct 类型和相关状态。 |
| [L589](../src/apps/remote/ClipboardController.cpp#L589) | `RateSample` | struct | 定义 RateSample 的 struct 类型和相关状态。 |

### 成员与文件级变量

| 行 | 变量 | 声明 | 作用 |
|---:|---|---|---|
| [L31](../src/apps/remote/ClipboardController.cpp#L31) | `kChunkBytes` | `constexpr std::size_t kChunkBytes = 56 * 1024;` | 定义 chunk bytes 的编译期常量或产品边界。 |
| [L32](../src/apps/remote/ClipboardController.cpp#L32) | `kClipboardSendBurstBytes` | `constexpr std::uint64_t kClipboardSendBurstBytes = 1024 * 1024;` | 定义 clipboard send burst bytes 的编译期常量或产品边界。 |
| [L38](../src/apps/remote/ClipboardController.cpp#L38) | `kClipboardProgressReportBytes` | `constexpr std::uint64_t kClipboardProgressReportBytes = 256 * 1024;` | 定义 clipboard progress report bytes 的编译期常量或产品边界。 |
| [L44](../src/apps/remote/ClipboardController.cpp#L44) | `kClipboardBackpressureRetryCount` | `constexpr int kClipboardBackpressureRetryCount = 1;` | Never spin inside the clipboard worker waiting for SCTP capacity. A single failed attempt is rescheduled below, leaving Cancel/Reject free to preempt the next chunk immediately. |
| [L47](../src/apps/remote/ClipboardController.cpp#L47) | `kMaximumTextBytes` | `constexpr std::uint64_t kMaximumTextBytes = 1024 * 1024;` | 定义 maximum text bytes 的编译期常量或产品边界。 |
| [L48](../src/apps/remote/ClipboardController.cpp#L48) | `kMaximumRichTextBytes` | `constexpr std::uint64_t kMaximumRichTextBytes = 4 * 1024 * 1024;` | 定义 maximum rich text bytes 的编译期常量或产品边界。 |
| [L49](../src/apps/remote/ClipboardController.cpp#L49) | `kMaximumPngBytes` | `constexpr std::uint64_t kMaximumPngBytes = 32 * 1024 * 1024;` | 定义 maximum png bytes 的编译期常量或产品边界。 |
| [L54](../src/apps/remote/ClipboardController.cpp#L54) | `kMaximumClipboardRetries` | `constexpr std::uint32_t kMaximumClipboardRetries = 3;` | 定义 maximum clipboard retries 的编译期常量或产品边界。 |

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L56](../src/apps/remote/ClipboardController.cpp#L56) | `CapabilityFlags` | 定义 | `std::uint32_t CapabilityFlags(const RemotePasteConfiguration& config)` | 实现 capability flags 对应的业务或工具逻辑。 |
| [L67](../src/apps/remote/ClipboardController.cpp#L67) | `ContentFlags` | 定义 | `std::uint32_t ContentFlags(const WindowsClipboardContent& content)` | 实现 content flags 对应的业务或工具逻辑。 |
| [L93](../src/apps/remote/ClipboardController.cpp#L93) | `FormatName` | 定义 | `std::string FormatName(ClipboardFormat format)` | 实现 format name 对应的业务或工具逻辑。 |
| [L106](../src/apps/remote/ClipboardController.cpp#L106) | `FormatEnabled` | 定义 | `bool FormatEnabled(ClipboardFormat format, const RemotePasteConfiguration& config)` | 实现 format enabled 对应的业务或工具逻辑。 |
| [L120](../src/apps/remote/ClipboardController.cpp#L120) | `FormatCapability` | 定义 | `std::uint32_t FormatCapability(ClipboardFormat format)` | 实现 format capability 对应的业务或工具逻辑。 |
| [L138](../src/apps/remote/ClipboardController.cpp#L138) | `FormatLimit` | 定义 | `std::uint64_t FormatLimit(ClipboardFormat format, const RemotePasteConfiguration& config)` | 实现 format limit 对应的业务或工具逻辑。 |
| [L153](../src/apps/remote/ClipboardController.cpp#L153) | `WideToUtf8` | 定义 | `std::string WideToUtf8(std::wstring_view text)` | 实现 wide to utf8 对应的业务或工具逻辑。 |
| [L169](../src/apps/remote/ClipboardController.cpp#L169) | `BuildClipboardAnnouncementDescriptors` | 定义 | `std::vector<ClipboardFormatDescriptor> BuildClipboardAnnouncementDescriptors( const WindowsClipboardContent& content)` | 创建或初始化 build clipboard announcement descriptors 相关逻辑。 |
| [L217](../src/apps/remote/ClipboardController.cpp#L217) | `Utf8ToWide` | 定义 | `std::wstring Utf8ToWide(std::string_view text)` | 实现 utf8 to wide 对应的业务或工具逻辑。 |
| [L234](../src/apps/remote/ClipboardController.cpp#L234) | `IncrementalSha256` | 定义 | `IncrementalSha256()` | 实现 incremental sha256 对应的业务或工具逻辑。 |
| [L250](../src/apps/remote/ClipboardController.cpp#L250) | `~IncrementalSha256` | 定义 | `~IncrementalSha256()` | 停止相关活动并释放 IncrementalSha256 实例拥有的资源。 |
| [L259](../src/apps/remote/ClipboardController.cpp#L259) | `Valid` | 定义 | `bool Valid() const { return valid_ && !finished_; }` | 判断 valid 相关逻辑。 |
| [L261](../src/apps/remote/ClipboardController.cpp#L261) | `Update` | 定义 | `bool Update(std::span<const std::uint8_t> bytes)` | 更新或应用 update 相关逻辑。 |
| [L269](../src/apps/remote/ClipboardController.cpp#L269) | `Finish` | 定义 | `bool Finish(std::array<std::uint8_t, 32>* digest)` | 停止 finish 相关逻辑。 |
| [L287](../src/apps/remote/ClipboardController.cpp#L287) | `SafeRelativePath` | 定义 | `bool SafeRelativePath(const std::string& utf8, std::filesystem::path* output)` | 实现 safe relative path 对应的业务或工具逻辑。 |
| [L319](../src/apps/remote/ClipboardController.cpp#L319) | `SameDescriptor` | 定义 | `bool SameDescriptor(const ClipboardFormatDescriptor& left, const ClipboardFormatDescriptor& right)` | 判断 same descriptor 相关逻辑。 |
| [L328](../src/apps/remote/ClipboardController.cpp#L328) | `SameDescriptorMetadata` | 定义 | `bool SameDescriptorMetadata(const ClipboardFormatDescriptor& left, const ClipboardFormatDescriptor& right)` | 判断 same descriptor metadata 相关逻辑。 |
| [L335](../src/apps/remote/ClipboardController.cpp#L335) | `WindowHasClass` | 定义 | `bool WindowHasClass(HWND window, const wchar_t* expected)` | 实现 window has class 对应的业务或工具逻辑。 |
| [L344](../src/apps/remote/ClipboardController.cpp#L344) | `IsDesktopPasteTarget` | 定义 | `bool IsDesktopPasteTarget(HWND target, HWND focused)` | 判断 is desktop paste target 相关逻辑。 |
| [L359](../src/apps/remote/ClipboardController.cpp#L359) | `DesktopDirectoryForWindow` | 定义 | `std::filesystem::path DesktopDirectoryForWindow(HWND target, HWND focused)` | 实现 desktop directory for window 对应的业务或工具逻辑。 |
| [L371](../src/apps/remote/ClipboardController.cpp#L371) | `ExplorerDirectoryForWindow` | 定义 | `std::filesystem::path ExplorerDirectoryForWindow( std::uintptr_t windowHandle, std::uintptr_t focusedWindowHandle)` | 实现 explorer directory for window 对应的业务或工具逻辑。 |
| [L464](../src/apps/remote/ClipboardController.cpp#L464) | `CopyPathReplacingConflicts` | 定义 | `bool CopyPathReplacingConflicts( const std::filesystem::path& source, const std::filesystem::path& target, bool isDirectory, bool replaceConflicts, std::error_code* error)` | 实现 copy path replacing conflicts 对应的业务或工具逻辑。 |
| [L610](../src/apps/remote/ClipboardController.cpp#L610) | `ClipboardController::ClipboardController` | 定义 | `ClipboardController::ClipboardController( SendCallback send, PasteCallback paste, std::unique_ptr<WindowsClipboardService> service) : send_(std::move(send)), paste_(std::move(paste)), service_(std::move(service)), cac...` | 构造并初始化 ClipboardController 实例。 |
| [L658](../src/apps/remote/ClipboardController.cpp#L658) | `ClipboardController::~ClipboardController` | 定义 | `ClipboardController::~ClipboardController()` | 停止相关活动并释放 ClipboardController 实例拥有的资源。 |
| [L677](../src/apps/remote/ClipboardController.cpp#L677) | `ClipboardController::SetObserver` | 定义 | `void ClipboardController::SetObserver(IClipboardControllerObserver* observer)` | 更新或应用 set observer 相关逻辑。 |
| [L686](../src/apps/remote/ClipboardController.cpp#L686) | `ClipboardController::SetConfiguration` | 定义 | `void ClipboardController::SetConfiguration( RemotePasteConfiguration configuration)` | 更新或应用 set configuration 相关逻辑。 |
| [L723](../src/apps/remote/ClipboardController.cpp#L723) | `ClipboardController::UpdateSession` | 定义 | `void ClipboardController::UpdateSession(ClipboardSessionContext context)` | 更新或应用 update session 相关逻辑。 |
| [L789](../src/apps/remote/ClipboardController.cpp#L789) | `ClipboardController::RequestPasteFromClipboard` | 定义 | `bool ClipboardController::RequestPasteFromClipboard()` | 发起请求或查询 request paste from clipboard 相关逻辑。 |
| [L834](../src/apps/remote/ClipboardController.cpp#L834) | `ClipboardController::ShouldPassThroughRemotePaste` | 定义 | `bool ClipboardController::ShouldPassThroughRemotePaste() const` | 判断 should pass through remote paste 相关逻辑。 |
| [L845](../src/apps/remote/ClipboardController.cpp#L845) | `ClipboardController::SetRemotePastePassthroughWindow` | 定义 | `void ClipboardController::SetRemotePastePassthroughWindow( std::uintptr_t windowHandle)` | 更新或应用 set remote paste passthrough window 相关逻辑。 |
| [L852](../src/apps/remote/ClipboardController.cpp#L852) | `ClipboardController::HandleLocalPasteHotkey` | 定义 | `bool ClipboardController::HandleLocalPasteHotkey(WindowsPasteTarget target)` | 接收并处理 handle local paste hotkey 相关逻辑。 |
| [L895](../src/apps/remote/ClipboardController.cpp#L895) | `ClipboardController::RequestAnnouncedRemoteClipboard` | 定义 | `void ClipboardController::RequestAnnouncedRemoteClipboard( WindowsPasteTarget target)` | 发起请求或查询 request announced remote clipboard 相关逻辑。 |
| [L967](../src/apps/remote/ClipboardController.cpp#L967) | `ClipboardController::SendAnnouncedRemoteClipboardRequest` | 定义 | `bool ClipboardController::SendAnnouncedRemoteClipboardRequest()` | 发送或发布 send announced remote clipboard request 相关逻辑。 |
| [L1022](../src/apps/remote/ClipboardController.cpp#L1022) | `ClipboardController::PasteIntoLocalTarget` | 定义 | `bool ClipboardController::PasteIntoLocalTarget( WindowsPasteTarget target, std::string* error)` | 实现 paste into local target 对应的业务或工具逻辑。 |
| [L1167](../src/apps/remote/ClipboardController.cpp#L1167) | `ClipboardController::CopyIncomingFilesToExplorerTarget` | 定义 | `bool ClipboardController::CopyIncomingFilesToExplorerTarget( const std::filesystem::path& destination, std::string* error)` | 实现 copy incoming files to explorer target 对应的业务或工具逻辑。 |
| [L1211](../src/apps/remote/ClipboardController.cpp#L1211) | `ClipboardController::RequestPasteFiles` | 定义 | `bool ClipboardController::RequestPasteFiles( std::vector<std::filesystem::path> paths)` | 发起请求或查询 request paste files 相关逻辑。 |
| [L1267](../src/apps/remote/ClipboardController.cpp#L1267) | `ClipboardController::CancelActiveTransfer` | 定义 | `bool ClipboardController::CancelActiveTransfer()` | 判断 cancel active transfer 相关逻辑。 |
| [L1301](../src/apps/remote/ClipboardController.cpp#L1301) | `ClipboardController::ResolveExplorerConflict` | 定义 | `bool ClipboardController::ResolveExplorerConflict(bool replaceAndContinue)` | 查询并返回 resolve explorer conflict 相关逻辑。 |
| [L1363](../src/apps/remote/ClipboardController.cpp#L1363) | `ClipboardController::RequestCacheCleanup` | 定义 | `bool ClipboardController::RequestCacheCleanup()` | 发起请求或查询 request cache cleanup 相关逻辑。 |
| [L1375](../src/apps/remote/ClipboardController.cpp#L1375) | `ClipboardController::RefreshCacheStatistics` | 定义 | `bool ClipboardController::RefreshCacheStatistics()` | 刷新 refresh cache statistics 相关逻辑。 |
| [L1383](../src/apps/remote/ClipboardController.cpp#L1383) | `ClipboardController::Snapshot` | 定义 | `ClipboardControllerSnapshot ClipboardController::Snapshot() const` | 查询并返回 snapshot 相关逻辑。 |
| [L1389](../src/apps/remote/ClipboardController.cpp#L1389) | `ClipboardController::OnClipboardMessage` | 定义 | `void ClipboardController::OnClipboardMessage( const ClipboardEnvelope& envelope)` | 接收并处理 on clipboard message 相关逻辑。 |
| [L1401](../src/apps/remote/ClipboardController.cpp#L1401) | `ClipboardController::Post` | 定义 | `bool ClipboardController::Post(std::function<void()> task)` | 实现 post 对应的业务或工具逻辑。 |
| [L1412](../src/apps/remote/ClipboardController.cpp#L1412) | `ClipboardController::PostPriority` | 定义 | `bool ClipboardController::PostPriority(std::function<void()> task)` | 实现 post priority 对应的业务或工具逻辑。 |
| [L1423](../src/apps/remote/ClipboardController.cpp#L1423) | `ClipboardController::WorkerMain` | 定义 | `void ClipboardController::WorkerMain(std::stop_token stopToken)` | 实现 worker main 对应的业务或工具逻辑。 |
| [L1493](../src/apps/remote/ClipboardController.cpp#L1493) | `ClipboardController::HandleLocalClipboard` | 定义 | `void ClipboardController::HandleLocalClipboard(WindowsClipboardContent content)` | 接收并处理 handle local clipboard 相关逻辑。 |
| [L1622](../src/apps/remote/ClipboardController.cpp#L1622) | `ClipboardController::ProcessDeferredLocalClipboard` | 定义 | `void ClipboardController::ProcessDeferredLocalClipboard()` | 接收并处理 process deferred local clipboard 相关逻辑。 |
| [L1630](../src/apps/remote/ClipboardController.cpp#L1630) | `ClipboardController::AnnounceLocalClipboard` | 定义 | `void ClipboardController::AnnounceLocalClipboard( WindowsClipboardContent content)` | 实现 announce local clipboard 对应的业务或工具逻辑。 |
| [L1654](../src/apps/remote/ClipboardController.cpp#L1654) | `ClipboardController::ProcessPendingLocalClipboard` | 定义 | `void ClipboardController::ProcessPendingLocalClipboard()` | 接收并处理 process pending local clipboard 相关逻辑。 |
| [L1738](../src/apps/remote/ClipboardController.cpp#L1738) | `ClipboardController::SendCapabilities` | 定义 | `void ClipboardController::SendCapabilities()` | 发送或发布 send capabilities 相关逻辑。 |
| [L1751](../src/apps/remote/ClipboardController.cpp#L1751) | `ClipboardController::ScheduleCapabilityRetry` | 定义 | `void ClipboardController::ScheduleCapabilityRetry()` | 执行后台循环或调度 schedule capability retry 相关逻辑。 |
| [L1762](../src/apps/remote/ClipboardController.cpp#L1762) | `ClipboardController::ProcessCapabilityRetry` | 定义 | `void ClipboardController::ProcessCapabilityRetry()` | 接收并处理 process capability retry 相关逻辑。 |
| [L1777](../src/apps/remote/ClipboardController.cpp#L1777) | `ClipboardController::ProcessPreparationWatchdog` | 定义 | `void ClipboardController::ProcessPreparationWatchdog()` | 接收并处理 process preparation watchdog 相关逻辑。 |
| [L1800](../src/apps/remote/ClipboardController.cpp#L1800) | `ClipboardController::ProcessRemoteRequestWatchdog` | 定义 | `void ClipboardController::ProcessRemoteRequestWatchdog()` | 接收并处理 process remote request watchdog 相关逻辑。 |
| [L1821](../src/apps/remote/ClipboardController.cpp#L1821) | `ClipboardController::ProcessExplorerConflictKeepalive` | 定义 | `void ClipboardController::ProcessExplorerConflictKeepalive()` | 接收并处理 process explorer conflict keepalive 相关逻辑。 |
| [L1843](../src/apps/remote/ClipboardController.cpp#L1843) | `ClipboardController::ScheduleTransferWatchdog` | 定义 | `void ClipboardController::ScheduleTransferWatchdog()` | 执行后台循环或调度 schedule transfer watchdog 相关逻辑。 |
| [L1859](../src/apps/remote/ClipboardController.cpp#L1859) | `ClipboardController::ScheduleBackpressureRetry` | 定义 | `void ClipboardController::ScheduleBackpressureRetry()` | 执行后台循环或调度 schedule backpressure retry 相关逻辑。 |
| [L1871](../src/apps/remote/ClipboardController.cpp#L1871) | `ClipboardController::ProcessTransferWatchdog` | 定义 | `void ClipboardController::ProcessTransferWatchdog()` | 接收并处理 process transfer watchdog 相关逻辑。 |
| [L1921](../src/apps/remote/ClipboardController.cpp#L1921) | `ClipboardController::SendSessionStart` | 定义 | `void ClipboardController::SendSessionStart()` | 发送或发布 send session start 相关逻辑。 |
| [L1930](../src/apps/remote/ClipboardController.cpp#L1930) | `ClipboardController::SendOutgoingOffer` | 定义 | `bool ClipboardController::SendOutgoingOffer()` | 发送或发布 send outgoing offer 相关逻辑。 |
| [L1949](../src/apps/remote/ClipboardController.cpp#L1949) | `ClipboardController::SendOutgoingCommit` | 定义 | `bool ClipboardController::SendOutgoingCommit()` | 发送或发布 send outgoing commit 相关逻辑。 |
| [L1970](../src/apps/remote/ClipboardController.cpp#L1970) | `ClipboardController::ResetOutgoingStreamingState` | 定义 | `bool ClipboardController::ResetOutgoingStreamingState(std::string* error)` | 重置或移除 reset outgoing streaming state 相关逻辑。 |
| [L1995](../src/apps/remote/ClipboardController.cpp#L1995) | `ClipboardController::FinalizeOutgoingPart` | 定义 | `bool ClipboardController::FinalizeOutgoingPart( TransferPart* part, std::string* error)` | 实现 finalize outgoing part 对应的业务或工具逻辑。 |
| [L2033](../src/apps/remote/ClipboardController.cpp#L2033) | `ClipboardController::ProcessEnvelope` | 定义 | `void ClipboardController::ProcessEnvelope(ClipboardEnvelope envelope)` | 接收并处理 process envelope 相关逻辑。 |
| [L2326](../src/apps/remote/ClipboardController.cpp#L2326) | `ClipboardController::ProcessOffer` | 定义 | `void ClipboardController::ProcessOffer(const ClipboardEnvelope& envelope)` | 接收并处理 process offer 相关逻辑。 |
| [L2494](../src/apps/remote/ClipboardController.cpp#L2494) | `ClipboardController::ProcessChunk` | 定义 | `void ClipboardController::ProcessChunk(const ClipboardEnvelope& envelope)` | 接收并处理 process chunk 相关逻辑。 |
| [L2584](../src/apps/remote/ClipboardController.cpp#L2584) | `ClipboardController::ProcessCommit` | 定义 | `void ClipboardController::ProcessCommit(const ClipboardEnvelope& envelope)` | 接收并处理 process commit 相关逻辑。 |
| [L2693](../src/apps/remote/ClipboardController.cpp#L2693) | `ClipboardController::SendNextChunk` | 定义 | `void ClipboardController::SendNextChunk()` | 发送或发布 send next chunk 相关逻辑。 |
| [L2814](../src/apps/remote/ClipboardController.cpp#L2814) | `ClipboardController::CancelTransfers` | 定义 | `void ClipboardController::CancelTransfers( const std::string& reasonCode, const std::string& reasonMessage, bool notifyPeer)` | 判断 cancel transfers 相关逻辑。 |
| [L2860](../src/apps/remote/ClipboardController.cpp#L2860) | `ClipboardController::CleanupIncomingTransferCache` | 定义 | `void ClipboardController::CleanupIncomingTransferCache()` | 实现 cleanup incoming transfer cache 对应的业务或工具逻辑。 |
| [L2874](../src/apps/remote/ClipboardController.cpp#L2874) | `ClipboardController::ClearRemoteAnnouncement` | 定义 | `void ClipboardController::ClearRemoteAnnouncement( bool clearWindowsAdvertisement)` | 重置或移除 clear remote announcement 相关逻辑。 |
| [L2892](../src/apps/remote/ClipboardController.cpp#L2892) | `ClipboardController::ResetTransferSnapshot` | 定义 | `void ClipboardController::ResetTransferSnapshot()` | 重置或移除 reset transfer snapshot 相关逻辑。 |
| [L2909](../src/apps/remote/ClipboardController.cpp#L2909) | `ClipboardController::UpdateOutgoingProgressSnapshot` | 定义 | `void ClipboardController::UpdateOutgoingProgressSnapshot()` | 更新或应用 update outgoing progress snapshot 相关逻辑。 |
| [L2984](../src/apps/remote/ClipboardController.cpp#L2984) | `ClipboardController::UpdateIncomingProgressSnapshot` | 定义 | `void ClipboardController::UpdateIncomingProgressSnapshot()` | 更新或应用 update incoming progress snapshot 相关逻辑。 |
| [L3068](../src/apps/remote/ClipboardController.cpp#L3068) | `ClipboardController::UpdateClipboardListenerState` | 定义 | `void ClipboardController::UpdateClipboardListenerState()` | 更新或应用 update clipboard listener state 相关逻辑。 |
| [L3075](../src/apps/remote/ClipboardController.cpp#L3075) | `ClipboardController::SendProtocolMessage` | 定义 | `bool ClipboardController::SendProtocolMessage( const ClipboardMessage& message)` | 发送或发布 send protocol message 相关逻辑。 |
| [L3116](../src/apps/remote/ClipboardController.cpp#L3116) | `ClipboardController::BuildOutgoingTransfer` | 定义 | `bool ClipboardController::BuildOutgoingTransfer( WindowsClipboardContent content, std::string* error)` | 创建或初始化 build outgoing transfer 相关逻辑。 |
| [L3252](../src/apps/remote/ClipboardController.cpp#L3252) | `ClipboardController::PrepareIncomingTransfer` | 定义 | `bool ClipboardController::PrepareIncomingTransfer( const ClipboardEnvelope& envelope, std::string* error)` | 实现 prepare incoming transfer 对应的业务或工具逻辑。 |
| [L3368](../src/apps/remote/ClipboardController.cpp#L3368) | `ClipboardController::FinalizeIncoming` | 定义 | `bool ClipboardController::FinalizeIncoming(std::string* error)` | 实现 finalize incoming 对应的业务或工具逻辑。 |
| [L3459](../src/apps/remote/ClipboardController.cpp#L3459) | `ClipboardController::GenerateId` | 定义 | `std::string ClipboardController::GenerateId(const char* prefix)` | 实现 generate id 对应的业务或工具逻辑。 |
| [L3468](../src/apps/remote/ClipboardController.cpp#L3468) | `ClipboardController::ClipboardCacheRoot` | 定义 | `std::filesystem::path ClipboardController::ClipboardCacheRoot() const` | 实现 clipboard cache root 对应的业务或工具逻辑。 |
| [L3474](../src/apps/remote/ClipboardController.cpp#L3474) | `ClipboardController::CleanupClipboardCache` | 定义 | `void ClipboardController::CleanupClipboardCache()` | 实现 cleanup clipboard cache 对应的业务或工具逻辑。 |
| [L3479](../src/apps/remote/ClipboardController.cpp#L3479) | `ClipboardController::UpdateCacheSnapshot` | 定义 | `void ClipboardController::UpdateCacheSnapshot()` | 更新或应用 update cache snapshot 相关逻辑。 |
| [L3491](../src/apps/remote/ClipboardController.cpp#L3491) | `ClipboardController::PublishSnapshot` | 定义 | `void ClipboardController::PublishSnapshot()` | 发送或发布 publish snapshot 相关逻辑。 |

## `src/apps/remote/ClipboardController.h`

[打开源码](../src/apps/remote/ClipboardController.h) · **文件作用：** 声明 clipboard controller 相关类型、接口、配置和成员状态。

### 类型

| 行 | 类型 | 种类 | 作用 |
|---:|---|---|---|
| [L27](../src/apps/remote/ClipboardController.h#L27) | `RemotePasteConfiguration` | struct | 定义 RemotePasteConfiguration 的 struct 类型和相关状态。 |
| [L40](../src/apps/remote/ClipboardController.h#L40) | `ClipboardSessionContext` | struct | 定义 ClipboardSessionContext 的 struct 类型和相关状态。 |
| [L49](../src/apps/remote/ClipboardController.h#L49) | `ClipboardControllerSnapshot` | struct | 定义 ClipboardControllerSnapshot 的 struct 类型和相关状态。 |
| [L88](../src/apps/remote/ClipboardController.h#L88) | `IClipboardControllerObserver` | class | 定义 IClipboardControllerObserver 的 class 类型和相关状态。 |
| [L95](../src/apps/remote/ClipboardController.h#L95) | `ClipboardController` | class | 定义 ClipboardController 的 class 类型和相关状态。 |
| [L129](../src/apps/remote/ClipboardController.h#L129) | `TransferPart` | struct | 定义 TransferPart 的 struct 类型和相关状态。 |
| [L130](../src/apps/remote/ClipboardController.h#L130) | `OutgoingTransfer` | struct | 定义 OutgoingTransfer 的 struct 类型和相关状态。 |
| [L131](../src/apps/remote/ClipboardController.h#L131) | `IncomingTransfer` | struct | 定义 IncomingTransfer 的 struct 类型和相关状态。 |

### 成员与文件级变量

| 行 | 变量 | 声明 | 作用 |
|---:|---|---|---|
| [L28](../src/apps/remote/ClipboardController.h#L28) | `enabled` | `bool enabled = true;` | 保存能力或开关状态：enabled。 |
| [L29](../src/apps/remote/ClipboardController.h#L29) | `unicodeText` | `bool unicodeText = true;` | 保存 unicode text 相关配置或运行状态。 |
| [L30](../src/apps/remote/ClipboardController.h#L30) | `html` | `bool html = true;` | 保存 html 相关配置或运行状态。 |
| [L31](../src/apps/remote/ClipboardController.h#L31) | `rtf` | `bool rtf = true;` | 保存 rtf 相关配置或运行状态。 |
| [L32](../src/apps/remote/ClipboardController.h#L32) | `png` | `bool png = true;` | 保存 png 相关配置或运行状态。 |
| [L33](../src/apps/remote/ClipboardController.h#L33) | `files` | `bool files = true;` | 保存 files 相关配置或运行状态。 |
| [L34](../src/apps/remote/ClipboardController.h#L34) | `automaticFileLimitBytes` | `std::uint64_t automaticFileLimitBytes = 100ull * 1024 * 1024;` | 保存计数、尺寸或速率指标：automatic file limit bytes。 |
| [L35](../src/apps/remote/ClipboardController.h#L35) | `cacheBaseDirectory` | `std::filesystem::path cacheBaseDirectory;` | 保存路径、地址或显示名称：cache base directory。 |
| [L36](../src/apps/remote/ClipboardController.h#L36) | `cacheRetention` | `std::chrono::minutes cacheRetention{60};` | 保存 cache retention 相关配置或运行状态。 |
| [L37](../src/apps/remote/ClipboardController.h#L37) | `cacheLimitBytes` | `std::uint64_t cacheLimitBytes = 2ull * 1024 * 1024 * 1024;` | 保存计数、尺寸或速率指标：cache limit bytes。 |
| [L41](../src/apps/remote/ClipboardController.h#L41) | `active` | `bool active = false;` | 保存能力或开关状态：active。 |
| [L42](../src/apps/remote/ClipboardController.h#L42) | `transportReady` | `bool transportReady = false;` | 保存能力或开关状态：transport ready。 |
| [L43](../src/apps/remote/ClipboardController.h#L43) | `localIsController` | `bool localIsController = false;` | 保存 local is controller 相关配置或运行状态。 |
| [L44](../src/apps/remote/ClipboardController.h#L44) | `roomId` | `std::string roomId;` | 保存身份或作用域标识：room id。 |
| [L45](../src/apps/remote/ClipboardController.h#L45) | `localDeviceId` | `std::string localDeviceId;` | 保存身份或作用域标识：local device id。 |
| [L46](../src/apps/remote/ClipboardController.h#L46) | `peerDeviceId` | `std::string peerDeviceId;` | 保存身份或作用域标识：peer device id。 |
| [L50](../src/apps/remote/ClipboardController.h#L50) | `enabled` | `bool enabled = false;` | 保存能力或开关状态：enabled。 |
| [L51](../src/apps/remote/ClipboardController.h#L51) | `sessionActive` | `bool sessionActive = false;` | 保存能力或开关状态：session active。 |
| [L52](../src/apps/remote/ClipboardController.h#L52) | `peerCapabilitiesSeen` | `bool peerCapabilitiesSeen = false;` | 保存 peer capabilities seen 相关配置或运行状态。 |
| [L53](../src/apps/remote/ClipboardController.h#L53) | `peerEnabled` | `bool peerEnabled = false;` | 保存能力或开关状态：peer enabled。 |
| [L54](../src/apps/remote/ClipboardController.h#L54) | `peerDeviceId` | `std::string peerDeviceId;` | 保存身份或作用域标识：peer device id。 |
| [L55](../src/apps/remote/ClipboardController.h#L55) | `state` | `std::string state = "disabled";` | 保存状态机当前状态：state。 |
| [L56](../src/apps/remote/ClipboardController.h#L56) | `lastFormat` | `std::string lastFormat;` | 保存 last format 相关配置或运行状态。 |
| [L57](../src/apps/remote/ClipboardController.h#L57) | `lastBytes` | `std::uint64_t lastBytes = 0;` | 保存计数、尺寸或速率指标：last bytes。 |
| [L58](../src/apps/remote/ClipboardController.h#L58) | `sentItems` | `std::uint64_t sentItems = 0;` | 保存 sent items 相关配置或运行状态。 |
| [L59](../src/apps/remote/ClipboardController.h#L59) | `receivedItems` | `std::uint64_t receivedItems = 0;` | 保存 received items 相关配置或运行状态。 |
| [L60](../src/apps/remote/ClipboardController.h#L60) | `suppressedEchoes` | `std::uint64_t suppressedEchoes = 0;` | 保存 suppressed echoes 相关配置或运行状态。 |
| [L61](../src/apps/remote/ClipboardController.h#L61) | `rejectedItems` | `std::uint64_t rejectedItems = 0;` | 保存 rejected items 相关配置或运行状态。 |
| [L62](../src/apps/remote/ClipboardController.h#L62) | `transferActive` | `bool transferActive = false;` | 保存能力或开关状态：transfer active。 |
| [L63](../src/apps/remote/ClipboardController.h#L63) | `transferOutgoing` | `bool transferOutgoing = false;` | 保存 transfer outgoing 相关配置或运行状态。 |
| [L64](../src/apps/remote/ClipboardController.h#L64) | `transferContainsFiles` | `bool transferContainsFiles = false;` | 保存 transfer contains files 相关配置或运行状态。 |
| [L65](../src/apps/remote/ClipboardController.h#L65) | `transferId` | `std::string transferId;` | 保存身份或作用域标识：transfer id。 |
| [L66](../src/apps/remote/ClipboardController.h#L66) | `transferItemCount` | `std::uint32_t transferItemCount = 0;` | 保存计数、尺寸或速率指标：transfer item count。 |
| [L67](../src/apps/remote/ClipboardController.h#L67) | `transferTotalBytes` | `std::uint64_t transferTotalBytes = 0;` | 保存计数、尺寸或速率指标：transfer total bytes。 |
| [L68](../src/apps/remote/ClipboardController.h#L68) | `transferCompletedBytes` | `std::uint64_t transferCompletedBytes = 0;` | 保存计数、尺寸或速率指标：transfer completed bytes。 |
| [L69](../src/apps/remote/ClipboardController.h#L69) | `transferBytesPerSecond` | `double transferBytesPerSecond = 0.0;` | 保存 transfer bytes per second 相关配置或运行状态。 |
| [L70](../src/apps/remote/ClipboardController.h#L70) | `transferEstimatedRemainingSeconds` | `std::uint64_t transferEstimatedRemainingSeconds = 0;` | 保存 transfer estimated remaining seconds 相关配置或运行状态。 |
| [L74](../src/apps/remote/ClipboardController.h#L74) | `localPasteTargetWindow` | `std::uintptr_t localPasteTargetWindow = 0;` | Local native window captured by the physical Ctrl+V hook. It is only populated for a controlled-to-controller lazy paste and is used to position the progress prompt without taki... |
| [L75](../src/apps/remote/ClipboardController.h#L75) | `localPasteDestinationPath` | `std::string localPasteDestinationPath;` | 保存路径、地址或显示名称：local paste destination path。 |
| [L76](../src/apps/remote/ClipboardController.h#L76) | `explorerConflictPending` | `bool explorerConflictPending = false;` | 保存待处理队列或请求：explorer conflict pending。 |
| [L77](../src/apps/remote/ClipboardController.h#L77) | `explorerConflictNames` | `std::vector<std::string> explorerConflictNames;` | 保存 explorer conflict names 相关配置或运行状态。 |
| [L78](../src/apps/remote/ClipboardController.h#L78) | `cacheRootPath` | `std::string cacheRootPath;` | 保存路径、地址或显示名称：cache root path。 |
| [L79](../src/apps/remote/ClipboardController.h#L79) | `cacheBytes` | `std::uint64_t cacheBytes = 0;` | 保存计数、尺寸或速率指标：cache bytes。 |
| [L80](../src/apps/remote/ClipboardController.h#L80) | `cacheEntryCount` | `std::uint64_t cacheEntryCount = 0;` | 保存计数、尺寸或速率指标：cache entry count。 |
| [L81](../src/apps/remote/ClipboardController.h#L81) | `cacheAvailableBytes` | `std::uint64_t cacheAvailableBytes = 0;` | 保存计数、尺寸或速率指标：cache available bytes。 |
| [L82](../src/apps/remote/ClipboardController.h#L82) | `cacheSafeCapacityBytes` | `std::uint64_t cacheSafeCapacityBytes = 0;` | 保存计数、尺寸或速率指标：cache safe capacity bytes。 |
| [L83](../src/apps/remote/ClipboardController.h#L83) | `cacheEffectiveLimitBytes` | `std::uint64_t cacheEffectiveLimitBytes = 0;` | 保存计数、尺寸或速率指标：cache effective limit bytes。 |
| [L84](../src/apps/remote/ClipboardController.h#L84) | `lastErrorCode` | `std::string lastErrorCode;` | 保存 last error code 相关配置或运行状态。 |
| [L85](../src/apps/remote/ClipboardController.h#L85) | `lastErrorMessage` | `std::string lastErrorMessage;` | 保存 last error message 相关配置或运行状态。 |
| [L129](../src/apps/remote/ClipboardController.h#L129) | `TransferPart` | `struct TransferPart;` | 保存 transfer part 相关配置或运行状态。 |
| [L130](../src/apps/remote/ClipboardController.h#L130) | `OutgoingTransfer` | `struct OutgoingTransfer;` | 保存 outgoing transfer 相关配置或运行状态。 |
| [L131](../src/apps/remote/ClipboardController.h#L131) | `IncomingTransfer` | `struct IncomingTransfer;` | 保存 incoming transfer 相关配置或运行状态。 |
| [L188](../src/apps/remote/ClipboardController.h#L188) | `send_` | `SendCallback send_;` | 保存 send 相关配置或运行状态。 |
| [L189](../src/apps/remote/ClipboardController.h#L189) | `paste_` | `PasteCallback paste_;` | 保存 paste 相关配置或运行状态。 |
| [L190](../src/apps/remote/ClipboardController.h#L190) | `service_` | `std::unique_ptr<WindowsClipboardService> service_;` | 保存 service 相关配置或运行状态。 |
| [L191](../src/apps/remote/ClipboardController.h#L191) | `cacheManager_` | `std::unique_ptr<ClipboardCacheManager> cacheManager_;` | 保存 cache manager 相关配置或运行状态。 |
| [L192](../src/apps/remote/ClipboardController.h#L192) | `mutex_` | `mutable std::mutex mutex_;` | 保护跨线程共享状态：mutex。 |
| [L193](../src/apps/remote/ClipboardController.h#L193) | `workAvailable_` | `std::condition_variable_any workAvailable_;` | 保存能力或开关状态：work available。 |
| [L195](../src/apps/remote/ClipboardController.h#L195) | `observer_` | `IClipboardControllerObserver* observer_ = nullptr;` | 保存回调或观察者入口：observer。 |
| [L196](../src/apps/remote/ClipboardController.h#L196) | `configuration_` | `RemotePasteConfiguration configuration_;` | 保存 configuration 相关配置或运行状态。 |
| [L197](../src/apps/remote/ClipboardController.h#L197) | `context_` | `ClipboardSessionContext context_;` | 保存 context 相关配置或运行状态。 |
| [L198](../src/apps/remote/ClipboardController.h#L198) | `snapshot_` | `ClipboardControllerSnapshot snapshot_;` | 保存可跨层读取的状态快照：snapshot。 |
| [L199](../src/apps/remote/ClipboardController.h#L199) | `publishedSnapshot_` | `ClipboardControllerSnapshot publishedSnapshot_;` | 保存可跨层读取的状态快照：published snapshot。 |
| [L200](../src/apps/remote/ClipboardController.h#L200) | `outgoing_` | `std::unique_ptr<OutgoingTransfer> outgoing_;` | 保存 outgoing 相关配置或运行状态。 |
| [L201](../src/apps/remote/ClipboardController.h#L201) | `incoming_` | `std::unique_ptr<IncomingTransfer> incoming_;` | 保存 incoming 相关配置或运行状态。 |
| [L202](../src/apps/remote/ClipboardController.h#L202) | `cachedLocalClipboard_` | `std::optional<WindowsClipboardContent> cachedLocalClipboard_;` | 保存 cached local clipboard 相关配置或运行状态。 |
| [L205](../src/apps/remote/ClipboardController.h#L205) | `deferredLocalClipboard_` | `std::optional<WindowsClipboardContent> deferredLocalClipboard_;` | A new Ctrl+C must not tear down bytes already being sent. Keep only the newest local item and publish it after the active send completes. |
| [L206](../src/apps/remote/ClipboardController.h#L206) | `pendingLocalClipboard_` | `std::optional<WindowsClipboardContent> pendingLocalClipboard_;` | 保存 pending local clipboard 相关配置或运行状态。 |
| [L207](../src/apps/remote/ClipboardController.h#L207) | `pendingLocalDue_` | `std::chrono::steady_clock::time_point pendingLocalDue_{};` | 保存 pending local due 相关配置或运行状态。 |
| [L209](../src/apps/remote/ClipboardController.h#L209) | `capabilityRetryDue_` | `capabilityRetryDue_;` | 保存 capability retry due 相关配置或运行状态。 |
| [L211](../src/apps/remote/ClipboardController.h#L211) | `transferWatchdogDue_` | `transferWatchdogDue_;` | 保存 transfer watchdog due 相关配置或运行状态。 |
| [L213](../src/apps/remote/ClipboardController.h#L213) | `preparationWatchdogDue_` | `preparationWatchdogDue_;` | 保存 preparation watchdog due 相关配置或运行状态。 |
| [L215](../src/apps/remote/ClipboardController.h#L215) | `remoteRequestWatchdogDue_` | `remoteRequestWatchdogDue_;` | 保存 remote request watchdog due 相关配置或运行状态。 |
| [L217](../src/apps/remote/ClipboardController.h#L217) | `explorerConflictKeepaliveDue_` | `explorerConflictKeepaliveDue_;` | 保存 explorer conflict keepalive due 相关配置或运行状态。 |
| [L218](../src/apps/remote/ClipboardController.h#L218) | `localSessionId_` | `std::string localSessionId_;` | 保存身份或作用域标识：local session id。 |
| [L219](../src/apps/remote/ClipboardController.h#L219) | `peerSessionId_` | `std::string peerSessionId_;` | 保存身份或作用域标识：peer session id。 |
| [L220](../src/apps/remote/ClipboardController.h#L220) | `peerCapabilities_` | `std::uint32_t peerCapabilities_ = 0;` | 保存 peer capabilities 相关配置或运行状态。 |
| [L221](../src/apps/remote/ClipboardController.h#L221) | `lastAppliedMarker_` | `std::string lastAppliedMarker_;` | 保存 last applied marker 相关配置或运行状态。 |
| [L222](../src/apps/remote/ClipboardController.h#L222) | `logicalClock_` | `std::uint64_t logicalClock_ = 0;` | 保护跨线程共享状态：logical clock。 |
| [L223](../src/apps/remote/ClipboardController.h#L223) | `lastAppliedVersion_` | `std::uint64_t lastAppliedVersion_ = 0;` | 保存 last applied version 相关配置或运行状态。 |
| [L224](../src/apps/remote/ClipboardController.h#L224) | `lastAppliedOrigin_` | `std::string lastAppliedOrigin_;` | 保存 last applied origin 相关配置或运行状态。 |
| [L225](../src/apps/remote/ClipboardController.h#L225) | `lastCompletedIncomingItemId_` | `std::string lastCompletedIncomingItemId_;` | 保存身份或作用域标识：last completed incoming item id。 |
| [L226](../src/apps/remote/ClipboardController.h#L226) | `lastCompletedIncomingVersion_` | `std::uint64_t lastCompletedIncomingVersion_ = 0;` | 保存 last completed incoming version 相关配置或运行状态。 |
| [L227](../src/apps/remote/ClipboardController.h#L227) | `lastCompletedIncomingAccepted_` | `bool lastCompletedIncomingAccepted_ = false;` | 保存 last completed incoming accepted 相关配置或运行状态。 |
| [L228](../src/apps/remote/ClipboardController.h#L228) | `lastCompletedIncomingErrorCode_` | `std::string lastCompletedIncomingErrorCode_;` | 保存 last completed incoming error code 相关配置或运行状态。 |
| [L229](../src/apps/remote/ClipboardController.h#L229) | `lastCompletedIncomingErrorMessage_` | `std::string lastCompletedIncomingErrorMessage_;` | 保存 last completed incoming error message 相关配置或运行状态。 |
| [L230](../src/apps/remote/ClipboardController.h#L230) | `announcedLocalItemId_` | `std::string announcedLocalItemId_;` | 保存身份或作用域标识：announced local item id。 |
| [L231](../src/apps/remote/ClipboardController.h#L231) | `announcedLocalVersion_` | `std::uint64_t announcedLocalVersion_ = 0;` | 保存 announced local version 相关配置或运行状态。 |
| [L232](../src/apps/remote/ClipboardController.h#L232) | `remoteAnnouncedItemId_` | `std::string remoteAnnouncedItemId_;` | 保存身份或作用域标识：remote announced item id。 |
| [L233](../src/apps/remote/ClipboardController.h#L233) | `remoteAnnouncedVersion_` | `std::uint64_t remoteAnnouncedVersion_ = 0;` | 保存 remote announced version 相关配置或运行状态。 |
| [L234](../src/apps/remote/ClipboardController.h#L234) | `remoteAnnouncedCapabilities_` | `std::uint32_t remoteAnnouncedCapabilities_ = 0;` | 保存 remote announced capabilities 相关配置或运行状态。 |
| [L235](../src/apps/remote/ClipboardController.h#L235) | `remoteAnnouncedDescriptors_` | `std::vector<ClipboardFormatDescriptor> remoteAnnouncedDescriptors_;` | 保存 remote announced descriptors 相关配置或运行状态。 |
| [L236](../src/apps/remote/ClipboardController.h#L236) | `pendingLocalPasteTarget_` | `WindowsPasteTarget pendingLocalPasteTarget_;` | 保存 pending local paste target 相关配置或运行状态。 |
| [L237](../src/apps/remote/ClipboardController.h#L237) | `pendingExplorerConflictOffer_` | `std::optional<ClipboardEnvelope> pendingExplorerConflictOffer_;` | 保存 pending explorer conflict offer 相关配置或运行状态。 |
| [L238](../src/apps/remote/ClipboardController.h#L238) | `pendingRemoteConflictDecision_` | `bool pendingRemoteConflictDecision_ = false;` | 保存 pending remote conflict decision 相关配置或运行状态。 |
| [L239](../src/apps/remote/ClipboardController.h#L239) | `fulfillingAnnouncedRequest_` | `bool fulfillingAnnouncedRequest_ = false;` | 保存 fulfilling announced request 相关配置或运行状态。 |
| [L240](../src/apps/remote/ClipboardController.h#L240) | `nextId_` | `std::uint64_t nextId_ = 0;` | 保存身份或作用域标识：next id。 |
| [L241](../src/apps/remote/ClipboardController.h#L241) | `captureRetryCount_` | `std::uint32_t captureRetryCount_ = 0;` | 保存计数、尺寸或速率指标：capture retry count。 |
| [L242](../src/apps/remote/ClipboardController.h#L242) | `localCapabilitiesSent_` | `bool localCapabilitiesSent_ = false;` | 保存 local capabilities sent 相关配置或运行状态。 |
| [L243](../src/apps/remote/ClipboardController.h#L243) | `lastSendBackpressured_` | `bool lastSendBackpressured_ = false;` | 保存 last send backpressured 相关配置或运行状态。 |
| [L244](../src/apps/remote/ClipboardController.h#L244) | `pasteRequestPending_` | `std::atomic_bool pasteRequestPending_{false};` | 保存待处理队列或请求：paste request pending。 |
| [L245](../src/apps/remote/ClipboardController.h#L245) | `remoteClipboardAvailable_` | `std::atomic_bool remoteClipboardAvailable_{false};` | 保存能力或开关状态：remote clipboard available。 |
| [L246](../src/apps/remote/ClipboardController.h#L246) | `localPasteHotkeyPending_` | `std::atomic_bool localPasteHotkeyPending_{false};` | 保存待处理队列或请求：local paste hotkey pending。 |
| [L247](../src/apps/remote/ClipboardController.h#L247) | `lastRemoteClipboardSequence_` | `std::atomic<std::uint32_t> lastRemoteClipboardSequence_{0};` | 保存单调序号，用于排序或去重：last remote clipboard sequence。 |
| [L248](../src/apps/remote/ClipboardController.h#L248) | `remoteAnnouncementLocalSequence_` | `std::atomic<std::uint32_t> remoteAnnouncementLocalSequence_{0};` | 保存单调序号，用于排序或去重：remote announcement local sequence。 |
| [L249](../src/apps/remote/ClipboardController.h#L249) | `remotePastePassthroughWindow_` | `std::atomic<std::uintptr_t> remotePastePassthroughWindow_{0};` | 保存 remote paste passthrough window 相关配置或运行状态。 |
| [L250](../src/apps/remote/ClipboardController.h#L250) | `localClipboardGeneration_` | `std::atomic<std::uint64_t> localClipboardGeneration_{0};` | 标记当前世代，用于拒绝过期异步结果：local clipboard generation。 |
| [L251](../src/apps/remote/ClipboardController.h#L251) | `stopping_` | `bool stopping_ = false;` | 保存 stopping 相关配置或运行状态。 |
| [L252](../src/apps/remote/ClipboardController.h#L252) | `worker_` | `std::jthread worker_;` | 拥有后台执行线程或工作器：worker。 |

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L90](../src/apps/remote/ClipboardController.h#L90) | `~IClipboardControllerObserver` | 声明 | `virtual ~IClipboardControllerObserver() = default` | 停止相关活动并释放 IClipboardControllerObserver 实例拥有的资源。 |
| [L91](../src/apps/remote/ClipboardController.h#L91) | `OnClipboardStateChanged` | 声明 | `virtual void OnClipboardStateChanged( const ClipboardControllerSnapshot& snapshot) = 0` | 接收并处理 on clipboard state changed 相关逻辑。 |
| [L103](../src/apps/remote/ClipboardController.h#L103) | `ClipboardController` | 声明 | `explicit ClipboardController( SendCallback send, PasteCallback paste, std::unique_ptr<WindowsClipboardService> service = std::make_unique<WindowsClipboardService>())` | 实现 clipboard controller 对应的业务或工具逻辑。 |
| [L108](../src/apps/remote/ClipboardController.h#L108) | `~ClipboardController` | 声明 | `~ClipboardController() override` | 停止相关活动并释放 ClipboardController 实例拥有的资源。 |
| [L110](../src/apps/remote/ClipboardController.h#L110) | `ClipboardController` | 声明 | `ClipboardController(const ClipboardController&) = delete` | 实现 clipboard controller 对应的业务或工具逻辑。 |
| [L113](../src/apps/remote/ClipboardController.h#L113) | `SetObserver` | 声明 | `void SetObserver(IClipboardControllerObserver* observer)` | 更新或应用 set observer 相关逻辑。 |
| [L114](../src/apps/remote/ClipboardController.h#L114) | `SetConfiguration` | 声明 | `void SetConfiguration(RemotePasteConfiguration configuration)` | 更新或应用 set configuration 相关逻辑。 |
| [L115](../src/apps/remote/ClipboardController.h#L115) | `UpdateSession` | 声明 | `void UpdateSession(ClipboardSessionContext context)` | 更新或应用 update session 相关逻辑。 |
| [L116](../src/apps/remote/ClipboardController.h#L116) | `RequestPasteFromClipboard` | 声明 | `bool RequestPasteFromClipboard()` | 发起请求或查询 request paste from clipboard 相关逻辑。 |
| [L117](../src/apps/remote/ClipboardController.h#L117) | `RequestPasteFiles` | 声明 | `bool RequestPasteFiles(std::vector<std::filesystem::path> paths)` | 发起请求或查询 request paste files 相关逻辑。 |
| [L118](../src/apps/remote/ClipboardController.h#L118) | `ShouldPassThroughRemotePaste` | 声明 | `bool ShouldPassThroughRemotePaste() const` | 判断 should pass through remote paste 相关逻辑。 |
| [L119](../src/apps/remote/ClipboardController.h#L119) | `SetRemotePastePassthroughWindow` | 声明 | `void SetRemotePastePassthroughWindow(std::uintptr_t windowHandle)` | 更新或应用 set remote paste passthrough window 相关逻辑。 |
| [L120](../src/apps/remote/ClipboardController.h#L120) | `CancelActiveTransfer` | 声明 | `bool CancelActiveTransfer()` | 判断 cancel active transfer 相关逻辑。 |
| [L121](../src/apps/remote/ClipboardController.h#L121) | `ResolveExplorerConflict` | 声明 | `bool ResolveExplorerConflict(bool replaceAndContinue)` | 查询并返回 resolve explorer conflict 相关逻辑。 |
| [L122](../src/apps/remote/ClipboardController.h#L122) | `RequestCacheCleanup` | 声明 | `bool RequestCacheCleanup()` | 发起请求或查询 request cache cleanup 相关逻辑。 |
| [L123](../src/apps/remote/ClipboardController.h#L123) | `RefreshCacheStatistics` | 声明 | `bool RefreshCacheStatistics()` | 刷新 refresh cache statistics 相关逻辑。 |
| [L124](../src/apps/remote/ClipboardController.h#L124) | `Snapshot` | 声明 | `ClipboardControllerSnapshot Snapshot() const` | 查询并返回 snapshot 相关逻辑。 |
| [L126](../src/apps/remote/ClipboardController.h#L126) | `OnClipboardMessage` | 声明 | `void OnClipboardMessage(const ClipboardEnvelope& envelope) override` | 接收并处理 on clipboard message 相关逻辑。 |
| [L133](../src/apps/remote/ClipboardController.h#L133) | `WorkerMain` | 声明 | `void WorkerMain(std::stop_token stopToken)` | 实现 worker main 对应的业务或工具逻辑。 |
| [L134](../src/apps/remote/ClipboardController.h#L134) | `Post` | 声明 | `bool Post(std::function<void()> task)` | 实现 post 对应的业务或工具逻辑。 |
| [L135](../src/apps/remote/ClipboardController.h#L135) | `PostPriority` | 声明 | `bool PostPriority(std::function<void()> task)` | 实现 post priority 对应的业务或工具逻辑。 |
| [L136](../src/apps/remote/ClipboardController.h#L136) | `PublishSnapshot` | 声明 | `void PublishSnapshot()` | 发送或发布 publish snapshot 相关逻辑。 |
| [L137](../src/apps/remote/ClipboardController.h#L137) | `HandleLocalClipboard` | 声明 | `void HandleLocalClipboard(WindowsClipboardContent content)` | 接收并处理 handle local clipboard 相关逻辑。 |
| [L138](../src/apps/remote/ClipboardController.h#L138) | `ProcessDeferredLocalClipboard` | 声明 | `void ProcessDeferredLocalClipboard()` | 接收并处理 process deferred local clipboard 相关逻辑。 |
| [L139](../src/apps/remote/ClipboardController.h#L139) | `ProcessPendingLocalClipboard` | 声明 | `void ProcessPendingLocalClipboard()` | 接收并处理 process pending local clipboard 相关逻辑。 |
| [L140](../src/apps/remote/ClipboardController.h#L140) | `SendCapabilities` | 声明 | `void SendCapabilities()` | 发送或发布 send capabilities 相关逻辑。 |
| [L141](../src/apps/remote/ClipboardController.h#L141) | `ScheduleCapabilityRetry` | 声明 | `void ScheduleCapabilityRetry()` | 执行后台循环或调度 schedule capability retry 相关逻辑。 |
| [L142](../src/apps/remote/ClipboardController.h#L142) | `ProcessCapabilityRetry` | 声明 | `void ProcessCapabilityRetry()` | 接收并处理 process capability retry 相关逻辑。 |
| [L143](../src/apps/remote/ClipboardController.h#L143) | `ProcessPreparationWatchdog` | 声明 | `void ProcessPreparationWatchdog()` | 接收并处理 process preparation watchdog 相关逻辑。 |
| [L144](../src/apps/remote/ClipboardController.h#L144) | `ProcessRemoteRequestWatchdog` | 声明 | `void ProcessRemoteRequestWatchdog()` | 接收并处理 process remote request watchdog 相关逻辑。 |
| [L145](../src/apps/remote/ClipboardController.h#L145) | `ProcessExplorerConflictKeepalive` | 声明 | `void ProcessExplorerConflictKeepalive()` | 接收并处理 process explorer conflict keepalive 相关逻辑。 |
| [L146](../src/apps/remote/ClipboardController.h#L146) | `ScheduleTransferWatchdog` | 声明 | `void ScheduleTransferWatchdog()` | 执行后台循环或调度 schedule transfer watchdog 相关逻辑。 |
| [L147](../src/apps/remote/ClipboardController.h#L147) | `ProcessTransferWatchdog` | 声明 | `void ProcessTransferWatchdog()` | 接收并处理 process transfer watchdog 相关逻辑。 |
| [L148](../src/apps/remote/ClipboardController.h#L148) | `SendSessionStart` | 声明 | `void SendSessionStart()` | 发送或发布 send session start 相关逻辑。 |
| [L149](../src/apps/remote/ClipboardController.h#L149) | `SendOutgoingOffer` | 声明 | `bool SendOutgoingOffer()` | 发送或发布 send outgoing offer 相关逻辑。 |
| [L150](../src/apps/remote/ClipboardController.h#L150) | `SendOutgoingCommit` | 声明 | `bool SendOutgoingCommit()` | 发送或发布 send outgoing commit 相关逻辑。 |
| [L151](../src/apps/remote/ClipboardController.h#L151) | `ResetOutgoingStreamingState` | 声明 | `bool ResetOutgoingStreamingState(std::string* error)` | 重置或移除 reset outgoing streaming state 相关逻辑。 |
| [L152](../src/apps/remote/ClipboardController.h#L152) | `FinalizeOutgoingPart` | 声明 | `bool FinalizeOutgoingPart(TransferPart* part, std::string* error)` | 实现 finalize outgoing part 对应的业务或工具逻辑。 |
| [L153](../src/apps/remote/ClipboardController.h#L153) | `ProcessEnvelope` | 声明 | `void ProcessEnvelope(ClipboardEnvelope envelope)` | 接收并处理 process envelope 相关逻辑。 |
| [L154](../src/apps/remote/ClipboardController.h#L154) | `ProcessOffer` | 声明 | `void ProcessOffer(const ClipboardEnvelope& envelope)` | 接收并处理 process offer 相关逻辑。 |
| [L155](../src/apps/remote/ClipboardController.h#L155) | `ProcessChunk` | 声明 | `void ProcessChunk(const ClipboardEnvelope& envelope)` | 接收并处理 process chunk 相关逻辑。 |
| [L156](../src/apps/remote/ClipboardController.h#L156) | `ProcessCommit` | 声明 | `void ProcessCommit(const ClipboardEnvelope& envelope)` | 接收并处理 process commit 相关逻辑。 |
| [L157](../src/apps/remote/ClipboardController.h#L157) | `SendNextChunk` | 声明 | `void SendNextChunk()` | 发送或发布 send next chunk 相关逻辑。 |
| [L158](../src/apps/remote/ClipboardController.h#L158) | `CancelTransfers` | 声明 | `void CancelTransfers(const std::string& reasonCode, const std::string& reasonMessage, bool notifyPeer)` | 判断 cancel transfers 相关逻辑。 |
| [L161](../src/apps/remote/ClipboardController.h#L161) | `CleanupIncomingTransferCache` | 声明 | `void CleanupIncomingTransferCache()` | 实现 cleanup incoming transfer cache 对应的业务或工具逻辑。 |
| [L162](../src/apps/remote/ClipboardController.h#L162) | `ClearRemoteAnnouncement` | 声明 | `void ClearRemoteAnnouncement(bool clearWindowsAdvertisement)` | 重置或移除 clear remote announcement 相关逻辑。 |
| [L163](../src/apps/remote/ClipboardController.h#L163) | `ResetTransferSnapshot` | 声明 | `void ResetTransferSnapshot()` | 重置或移除 reset transfer snapshot 相关逻辑。 |
| [L164](../src/apps/remote/ClipboardController.h#L164) | `UpdateOutgoingProgressSnapshot` | 声明 | `void UpdateOutgoingProgressSnapshot()` | 更新或应用 update outgoing progress snapshot 相关逻辑。 |
| [L165](../src/apps/remote/ClipboardController.h#L165) | `UpdateIncomingProgressSnapshot` | 声明 | `void UpdateIncomingProgressSnapshot()` | 更新或应用 update incoming progress snapshot 相关逻辑。 |
| [L166](../src/apps/remote/ClipboardController.h#L166) | `UpdateClipboardListenerState` | 声明 | `void UpdateClipboardListenerState()` | 更新或应用 update clipboard listener state 相关逻辑。 |
| [L167](../src/apps/remote/ClipboardController.h#L167) | `AnnounceLocalClipboard` | 声明 | `void AnnounceLocalClipboard(WindowsClipboardContent content)` | 实现 announce local clipboard 对应的业务或工具逻辑。 |
| [L168](../src/apps/remote/ClipboardController.h#L168) | `HandleLocalPasteHotkey` | 声明 | `bool HandleLocalPasteHotkey(WindowsPasteTarget target)` | 接收并处理 handle local paste hotkey 相关逻辑。 |
| [L169](../src/apps/remote/ClipboardController.h#L169) | `RequestAnnouncedRemoteClipboard` | 声明 | `void RequestAnnouncedRemoteClipboard(WindowsPasteTarget target)` | 发起请求或查询 request announced remote clipboard 相关逻辑。 |
| [L170](../src/apps/remote/ClipboardController.h#L170) | `SendAnnouncedRemoteClipboardRequest` | 声明 | `bool SendAnnouncedRemoteClipboardRequest()` | 发送或发布 send announced remote clipboard request 相关逻辑。 |
| [L171](../src/apps/remote/ClipboardController.h#L171) | `PasteIntoLocalTarget` | 声明 | `bool PasteIntoLocalTarget(WindowsPasteTarget target, std::string* error)` | 实现 paste into local target 对应的业务或工具逻辑。 |
| [L173](../src/apps/remote/ClipboardController.h#L173) | `CopyIncomingFilesToExplorerTarget` | 声明 | `bool CopyIncomingFilesToExplorerTarget( const std::filesystem::path& destination, std::string* error)` | 实现 copy incoming files to explorer target 对应的业务或工具逻辑。 |
| [L176](../src/apps/remote/ClipboardController.h#L176) | `SendProtocolMessage` | 声明 | `bool SendProtocolMessage(const ClipboardMessage& message)` | 发送或发布 send protocol message 相关逻辑。 |
| [L177](../src/apps/remote/ClipboardController.h#L177) | `BuildOutgoingTransfer` | 声明 | `bool BuildOutgoingTransfer(WindowsClipboardContent content, std::string* error)` | 创建或初始化 build outgoing transfer 相关逻辑。 |
| [L179](../src/apps/remote/ClipboardController.h#L179) | `ScheduleBackpressureRetry` | 声明 | `void ScheduleBackpressureRetry()` | 执行后台循环或调度 schedule backpressure retry 相关逻辑。 |
| [L180](../src/apps/remote/ClipboardController.h#L180) | `PrepareIncomingTransfer` | 声明 | `bool PrepareIncomingTransfer(const ClipboardEnvelope& envelope, std::string* error)` | 实现 prepare incoming transfer 对应的业务或工具逻辑。 |
| [L182](../src/apps/remote/ClipboardController.h#L182) | `FinalizeIncoming` | 声明 | `bool FinalizeIncoming(std::string* error)` | 实现 finalize incoming 对应的业务或工具逻辑。 |
| [L183](../src/apps/remote/ClipboardController.h#L183) | `CleanupClipboardCache` | 声明 | `void CleanupClipboardCache()` | 实现 cleanup clipboard cache 对应的业务或工具逻辑。 |
| [L184](../src/apps/remote/ClipboardController.h#L184) | `UpdateCacheSnapshot` | 声明 | `void UpdateCacheSnapshot()` | 更新或应用 update cache snapshot 相关逻辑。 |
| [L185](../src/apps/remote/ClipboardController.h#L185) | `GenerateId` | 声明 | `std::string GenerateId(const char* prefix)` | 实现 generate id 对应的业务或工具逻辑。 |
| [L186](../src/apps/remote/ClipboardController.h#L186) | `ClipboardCacheRoot` | 声明 | `std::filesystem::path ClipboardCacheRoot() const` | 实现 clipboard cache root 对应的业务或工具逻辑。 |

## `src/apps/remote/DirectSessionRuntimeState.h`

[打开源码](../src/apps/remote/DirectSessionRuntimeState.h) · **文件作用：** 声明 direct session runtime state 相关类型、接口、配置和成员状态。

### 类型

| 行 | 类型 | 种类 | 作用 |
|---:|---|---|---|
| [L20](../src/apps/remote/DirectSessionRuntimeState.h#L20) | `DirectSessionRuntimeState` | struct | Mutable transport/media state that belongs to one direct session. Keeping it together prevents owned-device and verification-code entry paths from growing separate state machine... |

### 成员与文件级变量

| 行 | 变量 | 声明 | 作用 |
|---:|---|---|---|
| [L21](../src/apps/remote/DirectSessionRuntimeState.h#L21) | `audioDevicesApplied` | `bool audioDevicesApplied = false;` | 保存 audio devices applied 相关配置或运行状态。 |
| [L22](../src/apps/remote/DirectSessionRuntimeState.h#L22) | `mediaSlotsPreparing` | `bool mediaSlotsPreparing = false;` | 保存 media slots preparing 相关配置或运行状态。 |
| [L23](../src/apps/remote/DirectSessionRuntimeState.h#L23) | `mediaSlotsPrepared` | `bool mediaSlotsPrepared = false;` | 保存 media slots prepared 相关配置或运行状态。 |
| [L24](../src/apps/remote/DirectSessionRuntimeState.h#L24) | `openDataChannels` | `std::unordered_map<std::string, bool> openDataChannels;` | 保存 open data channels 相关配置或运行状态。 |
| [L25](../src/apps/remote/DirectSessionRuntimeState.h#L25) | `screenTrack` | `webrtc::scoped_refptr<webrtc::MediaStreamTrackInterface> screenTrack;` | 保存 screen track 相关配置或运行状态。 |
| [L26](../src/apps/remote/DirectSessionRuntimeState.h#L26) | `nextInputSequence` | `std::uint64_t nextInputSequence = 0;` | 保存单调序号，用于排序或去重：next input sequence。 |
| [L27](../src/apps/remote/DirectSessionRuntimeState.h#L27) | `nextScreenControlSequence` | `std::uint64_t nextScreenControlSequence = 0;` | 保存单调序号，用于排序或去重：next screen control sequence。 |
| [L28](../src/apps/remote/DirectSessionRuntimeState.h#L28) | `lastScreenControlSequence` | `std::uint64_t lastScreenControlSequence = 0;` | 保存单调序号，用于排序或去重：last screen control sequence。 |
| [L29](../src/apps/remote/DirectSessionRuntimeState.h#L29) | `lastFastInputSequence` | `std::uint64_t lastFastInputSequence = 0;` | 保存单调序号，用于排序或去重：last fast input sequence。 |
| [L30](../src/apps/remote/DirectSessionRuntimeState.h#L30) | `lastReliableInputSequence` | `std::uint64_t lastReliableInputSequence = 0;` | 保存单调序号，用于排序或去重：last reliable input sequence。 |
| [L31](../src/apps/remote/DirectSessionRuntimeState.h#L31) | `lastPointerStateSequence` | `std::uint64_t lastPointerStateSequence = 0;` | 保存单调序号，用于排序或去重：last pointer state sequence。 |
| [L32](../src/apps/remote/DirectSessionRuntimeState.h#L32) | `lastCursorPositionSequence` | `std::uint64_t lastCursorPositionSequence = 0;` | 保存单调序号，用于排序或去重：last cursor position sequence。 |
| [L33](../src/apps/remote/DirectSessionRuntimeState.h#L33) | `lastCursorShapeSequence` | `std::uint64_t lastCursorShapeSequence = 0;` | 保存单调序号，用于排序或去重：last cursor shape sequence。 |
| [L34](../src/apps/remote/DirectSessionRuntimeState.h#L34) | `nextFileTransferSequence` | `std::uint64_t nextFileTransferSequence = 0;` | 保存单调序号，用于排序或去重：next file transfer sequence。 |
| [L35](../src/apps/remote/DirectSessionRuntimeState.h#L35) | `lastFileTransferSequence` | `std::uint64_t lastFileTransferSequence = 0;` | 保存单调序号，用于排序或去重：last file transfer sequence。 |
| [L36](../src/apps/remote/DirectSessionRuntimeState.h#L36) | `nextClipboardReliableSequence` | `std::uint64_t nextClipboardReliableSequence = 0;` | 保存单调序号，用于排序或去重：next clipboard reliable sequence。 |
| [L37](../src/apps/remote/DirectSessionRuntimeState.h#L37) | `lastClipboardReliableSequence` | `std::uint64_t lastClipboardReliableSequence = 0;` | 保存单调序号，用于排序或去重：last clipboard reliable sequence。 |
| [L38](../src/apps/remote/DirectSessionRuntimeState.h#L38) | `nextClipboardTransferSequence` | `std::uint64_t nextClipboardTransferSequence = 0;` | 保存单调序号，用于排序或去重：next clipboard transfer sequence。 |
| [L39](../src/apps/remote/DirectSessionRuntimeState.h#L39) | `lastClipboardTransferSequence` | `std::uint64_t lastClipboardTransferSequence = 0;` | 保存单调序号，用于排序或去重：last clipboard transfer sequence。 |
| [L40](../src/apps/remote/DirectSessionRuntimeState.h#L40) | `screenPreferenceApplied` | `bool screenPreferenceApplied = false;` | 保存 screen preference applied 相关配置或运行状态。 |
| [L41](../src/apps/remote/DirectSessionRuntimeState.h#L41) | `screenPreference` | `ScreenStreamPreferenceRequest screenPreference;` | 保存 screen preference 相关配置或运行状态。 |

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L49](../src/apps/remote/DirectSessionRuntimeState.h#L49) | `Reset` | 定义 | `void Reset()` | 重置或移除 reset 相关逻辑。 |

## `src/apps/remote/EncoderBenchmarkProfileCache.h`

[打开源码](../src/apps/remote/EncoderBenchmarkProfileCache.h) · **文件作用：** 声明 encoder benchmark profile cache 相关类型、接口、配置和成员状态。

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L21](../src/apps/remote/EncoderBenchmarkProfileCache.h#L21) | `EncoderBenchmarkProfileMatches` | 定义 | `inline bool EncoderBenchmarkProfileMatches( const QJsonObject& profile, const QString& hardwareFingerprint, const QString& captureBackend, const QString& quality, int policyVersion)` | 编码 encoder benchmark profile matches 相关逻辑。 |
| [L38](../src/apps/remote/EncoderBenchmarkProfileCache.h#L38) | `LoadEncoderBenchmarkProfile` | 定义 | `inline QJsonObject LoadEncoderBenchmarkProfile( const QSettings& settings, const QString& hardwareFingerprint, const QString& captureBackend, const QString& quality, int policyVersion)` | 读取或恢复 load encoder benchmark profile 相关逻辑。 |
| [L109](../src/apps/remote/EncoderBenchmarkProfileCache.h#L109) | `SaveEncoderBenchmarkProfile` | 定义 | `inline void SaveEncoderBenchmarkProfile( QSettings& settings, QJsonObject profile)` | 保存或写入 save encoder benchmark profile 相关逻辑。 |

## `src/apps/remote/FileTransferController.Chunking.inc`

[打开源码](../src/apps/remote/FileTransferController.Chunking.inc) · **文件作用：** `FileTransferController` 的实现切片，集中实现 chunking 相关逻辑；成员状态仍定义在所属头文件中。

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L4](../src/apps/remote/FileTransferController.Chunking.inc#L4) | `FileTransferController::SendAvailableChunks` | 定义 | `void FileTransferController::SendAvailableChunks( const std::string& transferId)` | SPDX-License-Identifier: GPL-3.0-only Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd) |

## `src/apps/remote/FileTransferController.Commands.inc`

[打开源码](../src/apps/remote/FileTransferController.Commands.inc) · **文件作用：** `FileTransferController` 的实现切片，集中实现 commands 相关逻辑；成员状态仍定义在所属头文件中。

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L4](../src/apps/remote/FileTransferController.Commands.inc#L4) | `FileTransferController::SendFile` | 定义 | `FileTransferCommandResult FileTransferController::SendFile( const std::string& peerDeviceId, const std::filesystem::path& sourcePath)` | SPDX-License-Identifier: GPL-3.0-only Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd) |
| [L37](../src/apps/remote/FileTransferController.Commands.inc#L37) | `FileTransferController::AcceptIncoming` | 定义 | `FileTransferCommandResult FileTransferController::AcceptIncoming( const std::string& transferId, const std::filesystem::path& destinationDirectory, bool replaceExisting)` | 处理并回复 accept incoming 相关逻辑。 |
| [L69](../src/apps/remote/FileTransferController.Commands.inc#L69) | `FileTransferController::RejectIncoming` | 定义 | `FileTransferCommandResult FileTransferController::RejectIncoming( const std::string& transferId)` | 处理并回复 reject incoming 相关逻辑。 |
| [L102](../src/apps/remote/FileTransferController.Commands.inc#L102) | `FileTransferController::Cancel` | 定义 | `FileTransferCommandResult FileTransferController::Cancel( const std::string& transferId)` | 判断 cancel 相关逻辑。 |
| [L154](../src/apps/remote/FileTransferController.Commands.inc#L154) | `FileTransferController::Resume` | 定义 | `FileTransferCommandResult FileTransferController::Resume( const std::string& transferId)` | 实现 resume 对应的业务或工具逻辑。 |

## `src/apps/remote/FileTransferController.cpp`

[打开源码](../src/apps/remote/FileTransferController.cpp) · **文件作用：** 实现 file transfer controller 相关函数与文件级辅助逻辑。

### 类型

| 行 | 类型 | 种类 | 作用 |
|---:|---|---|---|
| [L74](../src/apps/remote/FileTransferController.cpp#L74) | `FileTransferController::TransferRecord` | struct | 定义 FileTransferController::TransferRecord 的 struct 类型和相关状态。 |
| [L75](../src/apps/remote/FileTransferController.cpp#L75) | `RateSample` | struct | 定义 RateSample 的 struct 类型和相关状态。 |

### 成员与文件级变量

| 行 | 变量 | 声明 | 作用 |
|---:|---|---|---|
| [L19](../src/apps/remote/FileTransferController.cpp#L19) | `kControlBackpressureRetryCount` | `constexpr int kControlBackpressureRetryCount = 200;` | 定义 control backpressure retry count 的编译期常量或产品边界。 |
| [L23](../src/apps/remote/FileTransferController.cpp#L23) | `kFileTransferChunkBytes` | `constexpr std::size_t kFileTransferChunkBytes = 60 * 1024;` | 定义 file transfer chunk bytes 的编译期常量或产品边界。 |
| [L24](../src/apps/remote/FileTransferController.cpp#L24) | `kFileTransferSendBurstBytes` | `constexpr std::uint64_t kFileTransferSendBurstBytes = 1024 * 1024;` | 定义 file transfer send burst bytes 的编译期常量或产品边界。 |
| [L25](../src/apps/remote/FileTransferController.cpp#L25) | `kFileTransferMaximumInFlightBytes` | `constexpr std::uint64_t kFileTransferMaximumInFlightBytes = 2 * 1024 * 1024;` | 定义 file transfer maximum in flight bytes 的编译期常量或产品边界。 |
| [L26](../src/apps/remote/FileTransferController.cpp#L26) | `kFileTransferProgressAckBytes` | `constexpr std::uint64_t kFileTransferProgressAckBytes = 256 * 1024;` | 定义 file transfer progress ack bytes 的编译期常量或产品边界。 |
| [L33](../src/apps/remote/FileTransferController.cpp#L33) | `kMaximumFileResyncAttempts` | `constexpr std::uint32_t kMaximumFileResyncAttempts = 3;` | 定义 maximum file resync attempts 的编译期常量或产品边界。 |

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L40](../src/apps/remote/FileTransferController.cpp#L40) | `IsTerminal` | 定义 | `bool IsTerminal(FileTransferState state)` | 判断 is terminal 相关逻辑。 |
| [L49](../src/apps/remote/FileTransferController.cpp#L49) | `IsFileTransferBackpressure` | 定义 | `bool IsFileTransferBackpressure(const SessionCommandResult& result)` | 判断 is file transfer backpressure 相关逻辑。 |
| [L55](../src/apps/remote/FileTransferController.cpp#L55) | `Accepted` | 定义 | `FileTransferCommandResult Accepted(std::string transferId)` | 处理并回复 accepted 相关逻辑。 |
| [L63](../src/apps/remote/FileTransferController.cpp#L63) | `Rejected` | 定义 | `FileTransferCommandResult Rejected(std::string errorCode, std::string errorMessage)` | 处理并回复 rejected 相关逻辑。 |
| [L105](../src/apps/remote/FileTransferController.cpp#L105) | `FileTransferController::PublishSnapshots` | 定义 | `void FileTransferController::PublishSnapshots()` | 发送或发布 publish snapshots 相关逻辑。 |
| [L127](../src/apps/remote/FileTransferController.cpp#L127) | `FileTransferController::GenerateTransferId` | 定义 | `std::string FileTransferController::GenerateTransferId()` | 实现 generate transfer id 对应的业务或工具逻辑。 |

## `src/apps/remote/FileTransferController.h`

[打开源码](../src/apps/remote/FileTransferController.h) · **文件作用：** 声明 file transfer controller 相关类型、接口、配置和成员状态。

### 类型

| 行 | 类型 | 种类 | 作用 |
|---:|---|---|---|
| [L25](../src/apps/remote/FileTransferController.h#L25) | `FileTransferDirection` | enum class | 定义 FileTransferDirection 的 enum class 类型和相关状态。 |
| [L30](../src/apps/remote/FileTransferController.h#L30) | `FileTransferState` | enum class | 定义 FileTransferState 的 enum class 类型和相关状态。 |
| [L46](../src/apps/remote/FileTransferController.h#L46) | `FileTransferSnapshot` | struct | 定义 FileTransferSnapshot 的 struct 类型和相关状态。 |
| [L62](../src/apps/remote/FileTransferController.h#L62) | `FileTransferCommandResult` | struct | 定义 FileTransferCommandResult 的 struct 类型和相关状态。 |
| [L69](../src/apps/remote/FileTransferController.h#L69) | `IFileTransferControllerObserver` | class | 定义 IFileTransferControllerObserver 的 class 类型和相关状态。 |
| [L78](../src/apps/remote/FileTransferController.h#L78) | `FileTransferController` | class | Owns the file-transfer state machine and a single blocking file worker. The callback must route messages through the room member-pair engine. |
| [L115](../src/apps/remote/FileTransferController.h#L115) | `TransferRecord` | struct | 定义 TransferRecord 的 struct 类型和相关状态。 |
| [L116](../src/apps/remote/FileTransferController.h#L116) | `ScheduledTask` | struct | 定义 ScheduledTask 的 struct 类型和相关状态。 |

### 成员与文件级变量

| 行 | 变量 | 声明 | 作用 |
|---:|---|---|---|
| [L47](../src/apps/remote/FileTransferController.h#L47) | `transferId` | `std::string transferId;` | 保存身份或作用域标识：transfer id。 |
| [L48](../src/apps/remote/FileTransferController.h#L48) | `displayOrder` | `std::uint64_t displayOrder = 0;` | 保存 display order 相关配置或运行状态。 |
| [L49](../src/apps/remote/FileTransferController.h#L49) | `peerDeviceId` | `std::string peerDeviceId;` | 保存身份或作用域标识：peer device id。 |
| [L50](../src/apps/remote/FileTransferController.h#L50) | `fileName` | `std::string fileName;` | 保存路径、地址或显示名称：file name。 |
| [L51](../src/apps/remote/FileTransferController.h#L51) | `fileSize` | `std::uint64_t fileSize = 0;` | 保存计数、尺寸或速率指标：file size。 |
| [L52](../src/apps/remote/FileTransferController.h#L52) | `transferredBytes` | `std::uint64_t transferredBytes = 0;` | 保存计数、尺寸或速率指标：transferred bytes。 |
| [L53](../src/apps/remote/FileTransferController.h#L53) | `bytesPerSecond` | `double bytesPerSecond = 0.0;` | 保存 bytes per second 相关配置或运行状态。 |
| [L54](../src/apps/remote/FileTransferController.h#L54) | `estimatedRemainingSeconds` | `std::uint64_t estimatedRemainingSeconds = 0;` | 保存 estimated remaining seconds 相关配置或运行状态。 |
| [L55](../src/apps/remote/FileTransferController.h#L55) | `direction` | `FileTransferDirection direction = FileTransferDirection::kOutgoing;` | 保存 direction 相关配置或运行状态。 |
| [L56](../src/apps/remote/FileTransferController.h#L56) | `state` | `FileTransferState state = FileTransferState::kPreparing;` | 保存状态机当前状态：state。 |
| [L57](../src/apps/remote/FileTransferController.h#L57) | `localPath` | `std::filesystem::path localPath;` | 保存路径、地址或显示名称：local path。 |
| [L58](../src/apps/remote/FileTransferController.h#L58) | `errorCode` | `std::string errorCode;` | 保存 error code 相关配置或运行状态。 |
| [L59](../src/apps/remote/FileTransferController.h#L59) | `errorMessage` | `std::string errorMessage;` | 保存 error message 相关配置或运行状态。 |
| [L63](../src/apps/remote/FileTransferController.h#L63) | `accepted` | `bool accepted = false;` | 保存 accepted 相关配置或运行状态。 |
| [L64](../src/apps/remote/FileTransferController.h#L64) | `transferId` | `std::string transferId;` | 保存身份或作用域标识：transfer id。 |
| [L65](../src/apps/remote/FileTransferController.h#L65) | `errorCode` | `std::string errorCode;` | 保存 error code 相关配置或运行状态。 |
| [L66](../src/apps/remote/FileTransferController.h#L66) | `errorMessage` | `std::string errorMessage;` | 保存 error message 相关配置或运行状态。 |
| [L115](../src/apps/remote/FileTransferController.h#L115) | `TransferRecord` | `struct TransferRecord;` | 保存 transfer record 相关配置或运行状态。 |
| [L117](../src/apps/remote/FileTransferController.h#L117) | `due` | `std::chrono::steady_clock::time_point due;` | 保存 due 相关配置或运行状态。 |
| [L118](../src/apps/remote/FileTransferController.h#L118) | `order` | `std::uint64_t order = 0;` | 保存 order 相关配置或运行状态。 |
| [L155](../src/apps/remote/FileTransferController.h#L155) | `send_` | `SendCallback send_;` | 保存 send 相关配置或运行状态。 |
| [L156](../src/apps/remote/FileTransferController.h#L156) | `storage_` | `std::unique_ptr<FileTransferStorage> storage_;` | 保存 storage 相关配置或运行状态。 |
| [L157](../src/apps/remote/FileTransferController.h#L157) | `mutex_` | `mutable std::mutex mutex_;` | 保护跨线程共享状态：mutex。 |
| [L158](../src/apps/remote/FileTransferController.h#L158) | `workAvailable_` | `std::condition_variable_any workAvailable_;` | 保存能力或开关状态：work available。 |
| [L160](../src/apps/remote/FileTransferController.h#L160) | `scheduledTasks_` | `std::vector<ScheduledTask> scheduledTasks_;` | 保存待处理队列或请求：scheduled tasks。 |
| [L161](../src/apps/remote/FileTransferController.h#L161) | `transfers_` | `std::unordered_map<std::string, std::unique_ptr<TransferRecord>> transfers_;` | 保存 transfers 相关配置或运行状态。 |
| [L162](../src/apps/remote/FileTransferController.h#L162) | `availablePeers_` | `std::unordered_set<std::string> availablePeers_;` | 保存 available peers 相关配置或运行状态。 |
| [L163](../src/apps/remote/FileTransferController.h#L163) | `recoveringPeers_` | `std::unordered_set<std::string> recoveringPeers_;` | 保存 recovering peers 相关配置或运行状态。 |
| [L164](../src/apps/remote/FileTransferController.h#L164) | `observer_` | `IFileTransferControllerObserver* observer_ = nullptr;` | 保存回调或观察者入口：observer。 |
| [L165](../src/apps/remote/FileTransferController.h#L165) | `nextDisplayOrder_` | `std::uint64_t nextDisplayOrder_ = 0;` | 保存 next display order 相关配置或运行状态。 |
| [L166](../src/apps/remote/FileTransferController.h#L166) | `nextScheduledTaskOrder_` | `std::uint64_t nextScheduledTaskOrder_ = 0;` | 保存 next scheduled task order 相关配置或运行状态。 |
| [L167](../src/apps/remote/FileTransferController.h#L167) | `scheduledTasksRevision_` | `std::uint64_t scheduledTasksRevision_ = 0;` | 标记当前世代，用于拒绝过期异步结果：scheduled tasks revision。 |
| [L168](../src/apps/remote/FileTransferController.h#L168) | `stopping_` | `bool stopping_ = false;` | 保存 stopping 相关配置或运行状态。 |
| [L169](../src/apps/remote/FileTransferController.h#L169) | `worker_` | `std::jthread worker_;` | 拥有后台执行线程或工作器：worker。 |

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L71](../src/apps/remote/FileTransferController.h#L71) | `~IFileTransferControllerObserver` | 声明 | `virtual ~IFileTransferControllerObserver() = default` | 停止相关活动并释放 IFileTransferControllerObserver 实例拥有的资源。 |
| [L72](../src/apps/remote/FileTransferController.h#L72) | `OnFileTransfersChanged` | 声明 | `virtual void OnFileTransfersChanged( const std::vector<FileTransferSnapshot>& transfers) = 0` | 接收并处理 on file transfers changed 相关逻辑。 |
| [L84](../src/apps/remote/FileTransferController.h#L84) | `FileTransferController` | 声明 | `explicit FileTransferController( SendCallback send, std::unique_ptr<WindowsFileTransferService> fileService = std::make_unique<WindowsFileTransferService>())` | 实现 file transfer controller 对应的业务或工具逻辑。 |
| [L88](../src/apps/remote/FileTransferController.h#L88) | `~FileTransferController` | 声明 | `~FileTransferController() override` | 停止相关活动并释放 FileTransferController 实例拥有的资源。 |
| [L90](../src/apps/remote/FileTransferController.h#L90) | `FileTransferController` | 声明 | `FileTransferController(const FileTransferController&) = delete` | 实现 file transfer controller 对应的业务或工具逻辑。 |
| [L93](../src/apps/remote/FileTransferController.h#L93) | `SetObserver` | 声明 | `void SetObserver(IFileTransferControllerObserver* observer)` | 更新或应用 set observer 相关逻辑。 |
| [L94](../src/apps/remote/FileTransferController.h#L94) | `Transfers` | 声明 | `std::vector<FileTransferSnapshot> Transfers() const` | 实现 transfers 对应的业务或工具逻辑。 |
| [L95](../src/apps/remote/FileTransferController.h#L95) | `UpdatePeerConnectivity` | 声明 | `void UpdatePeerConnectivity( const std::vector<std::string>& activePeerDeviceIds, const std::vector<std::string>& recoveringPeerDeviceIds)` | 更新或应用 update peer connectivity 相关逻辑。 |
| [L99](../src/apps/remote/FileTransferController.h#L99) | `SendFile` | 声明 | `FileTransferCommandResult SendFile( const std::string& peerDeviceId, const std::filesystem::path& sourcePath)` | 发送或发布 send file 相关逻辑。 |
| [L102](../src/apps/remote/FileTransferController.h#L102) | `AcceptIncoming` | 声明 | `FileTransferCommandResult AcceptIncoming( const std::string& transferId, const std::filesystem::path& destinationDirectory, bool replaceExisting = false)` | 处理并回复 accept incoming 相关逻辑。 |
| [L106](../src/apps/remote/FileTransferController.h#L106) | `RejectIncoming` | 声明 | `FileTransferCommandResult RejectIncoming( const std::string& transferId)` | 处理并回复 reject incoming 相关逻辑。 |
| [L108](../src/apps/remote/FileTransferController.h#L108) | `Cancel` | 声明 | `FileTransferCommandResult Cancel(const std::string& transferId)` | 判断 cancel 相关逻辑。 |
| [L109](../src/apps/remote/FileTransferController.h#L109) | `Resume` | 声明 | `FileTransferCommandResult Resume(const std::string& transferId)` | 实现 resume 对应的业务或工具逻辑。 |
| [L111](../src/apps/remote/FileTransferController.h#L111) | `OnFileTransferMessage` | 声明 | `void OnFileTransferMessage( const FileTransferEnvelope& envelope) override` | 接收并处理 on file transfer message 相关逻辑。 |
| [L122](../src/apps/remote/FileTransferController.h#L122) | `WorkerMain` | 声明 | `void WorkerMain(std::stop_token stopToken)` | 实现 worker main 对应的业务或工具逻辑。 |
| [L123](../src/apps/remote/FileTransferController.h#L123) | `Post` | 声明 | `bool Post(std::function<void()> task)` | 实现 post 对应的业务或工具逻辑。 |
| [L124](../src/apps/remote/FileTransferController.h#L124) | `PostPriority` | 声明 | `bool PostPriority(std::function<void()> task)` | 实现 post priority 对应的业务或工具逻辑。 |
| [L125](../src/apps/remote/FileTransferController.h#L125) | `PostAfter` | 声明 | `bool PostAfter(std::chrono::milliseconds delay, std::function<void()> task)` | 实现 post after 对应的业务或工具逻辑。 |
| [L127](../src/apps/remote/FileTransferController.h#L127) | `PrepareOutgoing` | 声明 | `void PrepareOutgoing(const std::string& transferId)` | 实现 prepare outgoing 对应的业务或工具逻辑。 |
| [L128](../src/apps/remote/FileTransferController.h#L128) | `PrepareIncoming` | 声明 | `void PrepareIncoming(const std::string& transferId)` | 实现 prepare incoming 对应的业务或工具逻辑。 |
| [L129](../src/apps/remote/FileTransferController.h#L129) | `ProcessEnvelope` | 声明 | `void ProcessEnvelope(FileTransferEnvelope envelope)` | 接收并处理 process envelope 相关逻辑。 |
| [L130](../src/apps/remote/FileTransferController.h#L130) | `SendAvailableChunks` | 声明 | `void SendAvailableChunks(const std::string& transferId)` | 发送或发布 send available chunks 相关逻辑。 |
| [L131](../src/apps/remote/FileTransferController.h#L131) | `UpdateProgressLocked` | 声明 | `void UpdateProgressLocked(TransferRecord& transfer, std::uint64_t transferredBytes)` | 更新或应用 update progress locked 相关逻辑。 |
| [L133](../src/apps/remote/FileTransferController.h#L133) | `ResetRateLocked` | 声明 | `void ResetRateLocked(TransferRecord& transfer)` | 重置或移除 reset rate locked 相关逻辑。 |
| [L134](../src/apps/remote/FileTransferController.h#L134) | `RefreshRateLocked` | 声明 | `bool RefreshRateLocked( TransferRecord& transfer, std::chrono::steady_clock::time_point now)` | 刷新 refresh rate locked 相关逻辑。 |
| [L137](../src/apps/remote/FileTransferController.h#L137) | `CheckStalledTransfers` | 声明 | `void CheckStalledTransfers()` | 校验 check stalled transfers 相关逻辑。 |
| [L138](../src/apps/remote/FileTransferController.h#L138) | `SendOnce` | 声明 | `SessionCommandResult SendOnce( const std::string& peerDeviceId, const FileTransferMessage& message)` | 发送或发布 send once 相关逻辑。 |
| [L141](../src/apps/remote/FileTransferController.h#L141) | `SendWithBackpressureRetry` | 声明 | `SessionCommandResult SendWithBackpressureRetry( const std::string& peerDeviceId, const FileTransferMessage& message)` | 发送或发布 send with backpressure retry 相关逻辑。 |
| [L144](../src/apps/remote/FileTransferController.h#L144) | `FailTransfer` | 声明 | `void FailTransfer(const std::string& transferId, std::string errorCode, std::string errorMessage, bool notifyPeer)` | 实现 fail transfer 对应的业务或工具逻辑。 |
| [L148](../src/apps/remote/FileTransferController.h#L148) | `SendProtocolError` | 声明 | `void SendProtocolError(const std::string& peerDeviceId, const std::string& transferId, const std::string& errorCode, const std::string& errorMessage)` | 发送或发布 send protocol error 相关逻辑。 |
| [L152](../src/apps/remote/FileTransferController.h#L152) | `PublishSnapshots` | 声明 | `void PublishSnapshots()` | 发送或发布 publish snapshots 相关逻辑。 |
| [L153](../src/apps/remote/FileTransferController.h#L153) | `GenerateTransferId` | 声明 | `std::string GenerateTransferId()` | 实现 generate transfer id 对应的业务或工具逻辑。 |

## `src/apps/remote/FileTransferController.Lifecycle.inc`

[打开源码](../src/apps/remote/FileTransferController.Lifecycle.inc) · **文件作用：** `FileTransferController` 的实现切片，集中实现 lifecycle 相关逻辑；成员状态仍定义在所属头文件中。

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L4](../src/apps/remote/FileTransferController.Lifecycle.inc#L4) | `FileTransferController::FileTransferController` | 定义 | `FileTransferController::FileTransferController( SendCallback send, std::unique_ptr<WindowsFileTransferService> fileService) : send_(std::move(send)), storage_(std::make_unique<FileTransferStorage>( std::move(fileServi...` | SPDX-License-Identifier: GPL-3.0-only Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd) |
| [L15](../src/apps/remote/FileTransferController.Lifecycle.inc#L15) | `FileTransferController::~FileTransferController` | 定义 | `FileTransferController::~FileTransferController()` | 停止相关活动并释放 FileTransferController 实例拥有的资源。 |
| [L38](../src/apps/remote/FileTransferController.Lifecycle.inc#L38) | `FileTransferController::SetObserver` | 定义 | `void FileTransferController::SetObserver( IFileTransferControllerObserver* observer)` | 更新或应用 set observer 相关逻辑。 |
| [L48](../src/apps/remote/FileTransferController.Lifecycle.inc#L48) | `FileTransferController::Transfers` | 定义 | `std::vector<FileTransferSnapshot> FileTransferController::Transfers() const` | 实现 transfers 对应的业务或工具逻辑。 |
| [L63](../src/apps/remote/FileTransferController.Lifecycle.inc#L63) | `FileTransferController::UpdatePeerConnectivity` | 定义 | `void FileTransferController::UpdatePeerConnectivity( const std::vector<std::string>& activePeerDeviceIds, const std::vector<std::string>& recoveringPeerDeviceIds)` | 更新或应用 update peer connectivity 相关逻辑。 |

## `src/apps/remote/FileTransferController.Receive.inc`

[打开源码](../src/apps/remote/FileTransferController.Receive.inc) · **文件作用：** `FileTransferController` 的实现切片，集中实现 receive 相关逻辑；成员状态仍定义在所属头文件中。

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L4](../src/apps/remote/FileTransferController.Receive.inc#L4) | `FileTransferController::PrepareIncoming` | 定义 | `void FileTransferController::PrepareIncoming( const std::string& transferId)` | SPDX-License-Identifier: GPL-3.0-only Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd) |
| [L75](../src/apps/remote/FileTransferController.Receive.inc#L75) | `FileTransferController::ProcessEnvelope` | 定义 | `void FileTransferController::ProcessEnvelope(FileTransferEnvelope envelope)` | 接收并处理 process envelope 相关逻辑。 |

## `src/apps/remote/FileTransferController.Reliability.inc`

[打开源码](../src/apps/remote/FileTransferController.Reliability.inc) · **文件作用：** `FileTransferController` 的实现切片，集中实现 reliability 相关逻辑；成员状态仍定义在所属头文件中。

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L4](../src/apps/remote/FileTransferController.Reliability.inc#L4) | `FileTransferController::CheckStalledTransfers` | 定义 | `void FileTransferController::CheckStalledTransfers()` | SPDX-License-Identifier: GPL-3.0-only Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd) |
| [L97](../src/apps/remote/FileTransferController.Reliability.inc#L97) | `FileTransferController::SendWithBackpressureRetry` | 定义 | `SessionCommandResult FileTransferController::SendWithBackpressureRetry( const std::string& peerDeviceId, const FileTransferMessage& message)` | 发送或发布 send with backpressure retry 相关逻辑。 |
| [L117](../src/apps/remote/FileTransferController.Reliability.inc#L117) | `FileTransferController::SendOnce` | 定义 | `SessionCommandResult FileTransferController::SendOnce( const std::string& peerDeviceId, const FileTransferMessage& message)` | 发送或发布 send once 相关逻辑。 |
| [L130](../src/apps/remote/FileTransferController.Reliability.inc#L130) | `FileTransferController::FailTransfer` | 定义 | `void FileTransferController::FailTransfer( const std::string& transferId, std::string errorCode, std::string errorMessage, bool notifyPeer)` | 实现 fail transfer 对应的业务或工具逻辑。 |
| [L161](../src/apps/remote/FileTransferController.Reliability.inc#L161) | `FileTransferController::SendProtocolError` | 定义 | `void FileTransferController::SendProtocolError( const std::string& peerDeviceId, const std::string& transferId, const std::string& errorCode, const std::string& errorMessage)` | 发送或发布 send protocol error 相关逻辑。 |

## `src/apps/remote/FileTransferController.Send.inc`

[打开源码](../src/apps/remote/FileTransferController.Send.inc) · **文件作用：** `FileTransferController` 的实现切片，集中实现 send 相关逻辑；成员状态仍定义在所属头文件中。

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L4](../src/apps/remote/FileTransferController.Send.inc#L4) | `FileTransferController::PrepareOutgoing` | 定义 | `void FileTransferController::PrepareOutgoing( const std::string& transferId)` | SPDX-License-Identifier: GPL-3.0-only Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd) |

## `src/apps/remote/FileTransferController.Worker.inc`

[打开源码](../src/apps/remote/FileTransferController.Worker.inc) · **文件作用：** `FileTransferController` 的实现切片，集中实现 worker 相关逻辑；成员状态仍定义在所属头文件中。

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L4](../src/apps/remote/FileTransferController.Worker.inc#L4) | `FileTransferController::OnFileTransferMessage` | 定义 | `void FileTransferController::OnFileTransferMessage( const FileTransferEnvelope& envelope)` | SPDX-License-Identifier: GPL-3.0-only Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd) |
| [L10](../src/apps/remote/FileTransferController.Worker.inc#L10) | `FileTransferController::WorkerMain` | 定义 | `void FileTransferController::WorkerMain(std::stop_token stopToken)` | 实现 worker main 对应的业务或工具逻辑。 |
| [L78](../src/apps/remote/FileTransferController.Worker.inc#L78) | `FileTransferController::Post` | 定义 | `bool FileTransferController::Post(std::function<void()> task)` | 实现 post 对应的业务或工具逻辑。 |
| [L91](../src/apps/remote/FileTransferController.Worker.inc#L91) | `FileTransferController::PostPriority` | 定义 | `bool FileTransferController::PostPriority(std::function<void()> task)` | 实现 post priority 对应的业务或工具逻辑。 |
| [L104](../src/apps/remote/FileTransferController.Worker.inc#L104) | `FileTransferController::PostAfter` | 定义 | `bool FileTransferController::PostAfter( std::chrono::milliseconds delay, std::function<void()> task)` | 实现 post after 对应的业务或工具逻辑。 |
| [L124](../src/apps/remote/FileTransferController.Worker.inc#L124) | `FileTransferController::ResetRateLocked` | 定义 | `void FileTransferController::ResetRateLocked(TransferRecord& transfer)` | 重置或移除 reset rate locked 相关逻辑。 |
| [L137](../src/apps/remote/FileTransferController.Worker.inc#L137) | `FileTransferController::UpdateProgressLocked` | 定义 | `void FileTransferController::UpdateProgressLocked( TransferRecord& transfer, std::uint64_t transferredBytes)` | 更新或应用 update progress locked 相关逻辑。 |
| [L154](../src/apps/remote/FileTransferController.Worker.inc#L154) | `FileTransferController::RefreshRateLocked` | 定义 | `bool FileTransferController::RefreshRateLocked( TransferRecord& transfer, std::chrono::steady_clock::time_point now)` | 刷新 refresh rate locked 相关逻辑。 |

## `src/apps/remote/FileTransferStorage.cpp`

[打开源码](../src/apps/remote/FileTransferStorage.cpp) · **文件作用：** 实现 file transfer storage 相关函数与文件级辅助逻辑。

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L10](../src/apps/remote/FileTransferStorage.cpp#L10) | `FileTransferStorage::FileTransferStorage` | 定义 | `FileTransferStorage::FileTransferStorage( std::unique_ptr<WindowsFileTransferService> service) : service_(std::move(service)) {}` | 构造并初始化 FileTransferStorage 实例。 |
| [L15](../src/apps/remote/FileTransferStorage.cpp#L15) | `FileTransferStorage::InspectSource` | 定义 | `bool FileTransferStorage::InspectSource( const std::filesystem::path& path, FileTransferSourceInfo* output, std::string* error) const` | 实现 inspect source 对应的业务或工具逻辑。 |
| [L23](../src/apps/remote/FileTransferStorage.cpp#L23) | `FileTransferStorage::ReadChunk` | 定义 | `bool FileTransferStorage::ReadChunk( const std::filesystem::path& path, std::uint64_t offset, std::size_t maximumBytes, std::vector<std::uint8_t>* output, std::string* error)` | 读取或恢复 read chunk 相关逻辑。 |
| [L34](../src/apps/remote/FileTransferStorage.cpp#L34) | `FileTransferStorage::CloseSource` | 定义 | `void FileTransferStorage::CloseSource( const std::filesystem::path& path)` | 关闭并清理 close source 相关逻辑。 |
| [L40](../src/apps/remote/FileTransferStorage.cpp#L40) | `FileTransferStorage::PrepareDestination` | 定义 | `bool FileTransferStorage::PrepareDestination( const std::filesystem::path& directory, const std::string& fileName, const std::string& transferId, std::uint64_t expectedSize, bool replaceExisting, FileTransferDestinati...` | 实现 prepare destination 对应的业务或工具逻辑。 |
| [L54](../src/apps/remote/FileTransferStorage.cpp#L54) | `FileTransferStorage::AppendChunk` | 定义 | `bool FileTransferStorage::AppendChunk( const std::filesystem::path& temporaryPath, std::uint64_t expectedOffset, std::span<const std::uint8_t> bytes, std::string* error)` | 实现 append chunk 对应的业务或工具逻辑。 |
| [L64](../src/apps/remote/FileTransferStorage.cpp#L64) | `FileTransferStorage::VerifyAndCommit` | 定义 | `bool FileTransferStorage::VerifyAndCommit( const FileTransferDestination& destination, std::uint64_t expectedSize, const std::array<std::uint8_t, 32>& expectedSha256, std::string* error)` | 校验 verify and commit 相关逻辑。 |
| [L74](../src/apps/remote/FileTransferStorage.cpp#L74) | `FileTransferStorage::CloseDestination` | 定义 | `void FileTransferStorage::CloseDestination( const std::filesystem::path& temporaryPath)` | 关闭并清理 close destination 相关逻辑。 |
| [L80](../src/apps/remote/FileTransferStorage.cpp#L80) | `FileTransferStorage::RemovePartial` | 定义 | `void FileTransferStorage::RemovePartial( const std::filesystem::path& temporaryPath)` | 重置或移除 remove partial 相关逻辑。 |

## `src/apps/remote/FileTransferStorage.h`

[打开源码](../src/apps/remote/FileTransferStorage.h) · **文件作用：** 声明 file transfer storage 相关类型、接口、配置和成员状态。

### 类型

| 行 | 类型 | 种类 | 作用 |
|---:|---|---|---|
| [L21](../src/apps/remote/FileTransferStorage.h#L21) | `FileTransferStorage` | class | Blocking persistence boundary used only by the controller's file worker. It centralizes source validation, resumable partial files, and atomic commit. |

### 成员与文件级变量

| 行 | 变量 | 声明 | 作用 |
|---:|---|---|---|
| [L55](../src/apps/remote/FileTransferStorage.h#L55) | `service_` | `std::unique_ptr<WindowsFileTransferService> service_;` | 保存 service 相关配置或运行状态。 |

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L23](../src/apps/remote/FileTransferStorage.h#L23) | `FileTransferStorage` | 声明 | `explicit FileTransferStorage( std::unique_ptr<WindowsFileTransferService> service)` | 实现 file transfer storage 对应的业务或工具逻辑。 |
| [L26](../src/apps/remote/FileTransferStorage.h#L26) | `InspectSource` | 声明 | `bool InspectSource(const std::filesystem::path& path, FileTransferSourceInfo* output, std::string* error) const` | 实现 inspect source 对应的业务或工具逻辑。 |
| [L29](../src/apps/remote/FileTransferStorage.h#L29) | `ReadChunk` | 声明 | `bool ReadChunk(const std::filesystem::path& path, std::uint64_t offset, std::size_t maximumBytes, std::vector<std::uint8_t>* output, std::string* error)` | 读取或恢复 read chunk 相关逻辑。 |
| [L34](../src/apps/remote/FileTransferStorage.h#L34) | `CloseSource` | 声明 | `void CloseSource(const std::filesystem::path& path)` | 关闭并清理 close source 相关逻辑。 |
| [L35](../src/apps/remote/FileTransferStorage.h#L35) | `PrepareDestination` | 声明 | `bool PrepareDestination(const std::filesystem::path& directory, const std::string& fileName, const std::string& transferId, std::uint64_t expectedSize, bool replaceExisting, FileTransferDestination* output, std::strin...` | 实现 prepare destination 对应的业务或工具逻辑。 |
| [L42](../src/apps/remote/FileTransferStorage.h#L42) | `AppendChunk` | 声明 | `bool AppendChunk(const std::filesystem::path& temporaryPath, std::uint64_t expectedOffset, std::span<const std::uint8_t> bytes, std::string* error)` | 实现 append chunk 对应的业务或工具逻辑。 |
| [L46](../src/apps/remote/FileTransferStorage.h#L46) | `VerifyAndCommit` | 声明 | `bool VerifyAndCommit( const FileTransferDestination& destination, std::uint64_t expectedSize, const std::array<std::uint8_t, 32>& expectedSha256, std::string* error)` | 校验 verify and commit 相关逻辑。 |
| [L51](../src/apps/remote/FileTransferStorage.h#L51) | `CloseDestination` | 声明 | `void CloseDestination(const std::filesystem::path& temporaryPath)` | 关闭并清理 close destination 相关逻辑。 |
| [L52](../src/apps/remote/FileTransferStorage.h#L52) | `RemovePartial` | 声明 | `void RemovePartial(const std::filesystem::path& temporaryPath)` | 重置或移除 remove partial 相关逻辑。 |

## `src/apps/remote/InProcessSessionEngine.Audio.inc`

[打开源码](../src/apps/remote/InProcessSessionEngine.Audio.inc) · **文件作用：** `InProcessSessionEngine` 的实现切片，集中实现 audio 相关逻辑；成员状态仍定义在所属头文件中。

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L4](../src/apps/remote/InProcessSessionEngine.Audio.inc#L4) | `InProcessSessionEngine::SetLocalMicrophoneEnabled` | 定义 | `SessionCommandResult InProcessSessionEngine::SetLocalMicrophoneEnabled( bool enabled)` | SPDX-License-Identifier: GPL-3.0-only Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd) |

## `src/apps/remote/InProcessSessionEngine.Camera.inc`

[打开源码](../src/apps/remote/InProcessSessionEngine.Camera.inc) · **文件作用：** `InProcessSessionEngine` 的实现切片，集中实现 camera 相关逻辑；成员状态仍定义在所属头文件中。

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L4](../src/apps/remote/InProcessSessionEngine.Camera.inc#L4) | `InProcessSessionEngine::SetLocalCameraEnabled` | 定义 | `SessionCommandResult InProcessSessionEngine::SetLocalCameraEnabled(bool enabled)` | SPDX-License-Identifier: GPL-3.0-only Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd) |

## `src/apps/remote/InProcessSessionEngine.cpp`

[打开源码](../src/apps/remote/InProcessSessionEngine.cpp) · **文件作用：** 实现 in process session engine 相关函数与文件级辅助逻辑。

### 类型

| 行 | 类型 | 种类 | 作用 |
|---:|---|---|---|
| [L305](../src/apps/remote/InProcessSessionEngine.cpp#L305) | `IdleRoomVideoSource` | class | 定义 IdleRoomVideoSource 的 class 类型和相关状态。 |

### 成员与文件级变量

| 行 | 变量 | 声明 | 作用 |
|---:|---|---|---|
| [L105](../src/apps/remote/InProcessSessionEngine.cpp#L105) | `kMaximumFileTransferBufferedBytes` | `constexpr std::uint64_t kMaximumFileTransferBufferedBytes = 8 * 1024 * 1024;` | 定义 maximum file transfer buffered bytes 的编译期常量或产品边界。 |
| [L110](../src/apps/remote/InProcessSessionEngine.cpp#L110) | `kMaximumClipboardBufferedBytes` | `constexpr std::uint64_t kMaximumClipboardBufferedBytes = 512 * 1024;` | Clipboard paste shares the PeerConnection with the live desktop video. Keep its cancellable in-flight tail small: libwebrtc cannot retract bytes already accepted by an ordered r... |

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L36](../src/apps/remote/InProcessSessionEngine.cpp#L36) | `Success` | 定义 | `SessionCommandResult Success()` | 实现 success 对应的业务或工具逻辑。 |
| [L41](../src/apps/remote/InProcessSessionEngine.cpp#L41) | `Failure` | 定义 | `SessionCommandResult Failure(std::string code, std::string message)` | 实现 failure 对应的业务或工具逻辑。 |
| [L46](../src/apps/remote/InProcessSessionEngine.cpp#L46) | `GenerateRotatedVerificationCode` | 定义 | `std::string GenerateRotatedVerificationCode( const std::string& previous)` | 实现 generate rotated verification code 对应的业务或工具逻辑。 |
| [L61](../src/apps/remote/InProcessSessionEngine.cpp#L61) | `NormalizeMediaDeviceId` | 定义 | `std::string NormalizeMediaDeviceId(const std::string& deviceId)` | 实现 normalize media device id 对应的业务或工具逻辑。 |
| [L68](../src/apps/remote/InProcessSessionEngine.cpp#L68) | `ContainsMediaDevice` | 定义 | `bool ContainsMediaDevice( const std::vector<MediaDeviceDescriptor>& devices, const std::string& deviceId)` | 实现 contains media device 对应的业务或工具逻辑。 |
| [L79](../src/apps/remote/InProcessSessionEngine.cpp#L79) | `IsRoomVideoSlot` | 定义 | `bool IsRoomVideoSlot(const std::string& slot)` | 判断 is room video slot 相关逻辑。 |
| [L85](../src/apps/remote/InProcessSessionEngine.cpp#L85) | `ShouldBoostDesktopCaptureForInput` | 定义 | `bool ShouldBoostDesktopCaptureForInput(const RemoteInputEvent& event)` | 判断 should boost desktop capture for input 相关逻辑。 |
| [L114](../src/apps/remote/InProcessSessionEngine.cpp#L114) | `IsClipboardWarmupPayload` | 定义 | `bool IsClipboardWarmupPayload(std::span<const std::uint8_t> payload)` | 判断 is clipboard warmup payload 相关逻辑。 |
| [L121](../src/apps/remote/InProcessSessionEngine.cpp#L121) | `ResolveScreenPolicy` | 定义 | `ScreenStreamPolicyResult ResolveScreenPolicy( std::uint32_t sourceWidth, std::uint32_t sourceHeight, const ScreenStreamPreferenceRequest& request)` | 查询并返回 resolve screen policy 相关逻辑。 |
| [L132](../src/apps/remote/InProcessSessionEngine.cpp#L132) | `EncoderPreferenceName` | 定义 | `std::string EncoderPreferenceName(VideoEncoderPreference preference)` | 编码 encoder preference name 相关逻辑。 |
| [L149](../src/apps/remote/InProcessSessionEngine.cpp#L149) | `DecoderPreferenceName` | 定义 | `std::string DecoderPreferenceName(VideoDecoderPreference preference)` | 解码或解析 decoder preference name 相关逻辑。 |
| [L162](../src/apps/remote/InProcessSessionEngine.cpp#L162) | `DesktopCaptureImplementationName` | 定义 | `std::string DesktopCaptureImplementationName( DesktopCaptureImplementation implementation)` | 实现 desktop capture implementation name 对应的业务或工具逻辑。 |
| [L174](../src/apps/remote/InProcessSessionEngine.cpp#L174) | `DesktopCaptureBackendName` | 定义 | `std::string DesktopCaptureBackendName( WindowsDesktopCaptureSource::CaptureBackend backend)` | 实现 desktop capture backend name 对应的业务或工具逻辑。 |
| [L188](../src/apps/remote/InProcessSessionEngine.cpp#L188) | `MaximumDesktopCaptureFrameRate` | 定义 | `std::uint32_t MaximumDesktopCaptureFrameRate( DesktopCaptureImplementation implementation, const WindowsDesktopCaptureSource* source = nullptr)` | 实现 maximum desktop capture frame rate 对应的业务或工具逻辑。 |
| [L203](../src/apps/remote/InProcessSessionEngine.cpp#L203) | `DescribeEncoderRuntimeInstance` | 定义 | `std::string DescribeEncoderRuntimeInstance( const VideoEncoderInstanceRuntimeStatus& status)` | 实现 describe encoder runtime instance 对应的业务或工具逻辑。 |
| [L307](../src/apps/remote/InProcessSessionEngine.cpp#L307) | `state` | 定义 | `SourceState state() const override { return kLive; }` | 实现 state 对应的业务或工具逻辑。 |
| [L308](../src/apps/remote/InProcessSessionEngine.cpp#L308) | `remote` | 定义 | `bool remote() const override { return false; }` | 实现 remote 对应的业务或工具逻辑。 |
| [L309](../src/apps/remote/InProcessSessionEngine.cpp#L309) | `is_screencast` | 定义 | `bool is_screencast() const override { return true; }` | 判断 is screencast 相关逻辑。 |
| [L310](../src/apps/remote/InProcessSessionEngine.cpp#L310) | `needs_denoising` | 定义 | `std::optional<bool> needs_denoising() const override { return false; }` | 判断 needs denoising 相关逻辑。 |
| [L337](../src/apps/remote/InProcessSessionEngine.cpp#L337) | `InProcessSessionEngine::OnRoomPairDataMessage` | 定义 | `void InProcessSessionEngine::OnRoomPairDataMessage( const std::string& pairId, const std::string& label, std::span<const std::uint8_t> payload, bool binary)` | 接收并处理 on room pair data message 相关逻辑。 |

## `src/apps/remote/InProcessSessionEngine.DirectBridge.inc`

[打开源码](../src/apps/remote/InProcessSessionEngine.DirectBridge.inc) · **文件作用：** `InProcessSessionEngine` 的实现切片，集中实现 direct bridge 相关逻辑；成员状态仍定义在所属头文件中。

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L4](../src/apps/remote/InProcessSessionEngine.DirectBridge.inc#L4) | `InProcessSessionEngine::OnRoomPairRemoteTrackAdded` | 定义 | `void InProcessSessionEngine::OnRoomPairRemoteTrackAdded( const std::string& pairId, const RemoteTrackInfo& track)` | SPDX-License-Identifier: GPL-3.0-only Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd) |
| [L12](../src/apps/remote/InProcessSessionEngine.DirectBridge.inc#L12) | `InProcessSessionEngine::SendDescription` | 定义 | `bool InProcessSessionEngine::SendDescription( const SessionDescription& description)` | 发送或发布 send description 相关逻辑。 |
| [L36](../src/apps/remote/InProcessSessionEngine.DirectBridge.inc#L36) | `InProcessSessionEngine::SendIceCandidate` | 定义 | `bool InProcessSessionEngine::SendIceCandidate( const IceCandidate& candidate)` | 发送或发布 send ice candidate 相关逻辑。 |
| [L59](../src/apps/remote/InProcessSessionEngine.DirectBridge.inc#L59) | `InProcessSessionEngine::RequestIceRestart` | 定义 | `bool InProcessSessionEngine::RequestIceRestart( std::uint64_t observedGeneration, std::uint64_t requestSequence)` | 发起请求或查询 request ice restart 相关逻辑。 |
| [L79](../src/apps/remote/InProcessSessionEngine.DirectBridge.inc#L79) | `InProcessSessionEngine::CancelIceRestart` | 定义 | `bool InProcessSessionEngine::CancelIceRestart( std::uint64_t observedGeneration, std::uint64_t requestSequence)` | 判断 cancel ice restart 相关逻辑。 |
| [L99](../src/apps/remote/InProcessSessionEngine.DirectBridge.inc#L99) | `InProcessSessionEngine::OnControllerSnapshot` | 定义 | `void InProcessSessionEngine::OnControllerSnapshot( const SessionControllerSnapshot& controllerSnapshot)` | 接收并处理 on controller snapshot 相关逻辑。 |
| [L222](../src/apps/remote/InProcessSessionEngine.DirectBridge.inc#L222) | `InProcessSessionEngine::OnDataChannelStateChanged` | 定义 | `void InProcessSessionEngine::OnDataChannelStateChanged( const DataChannelInfo& channel)` | 接收并处理 on data channel state changed 相关逻辑。 |
| [L284](../src/apps/remote/InProcessSessionEngine.DirectBridge.inc#L284) | `InProcessSessionEngine::OnDataMessage` | 定义 | `void InProcessSessionEngine::OnDataMessage( const std::string& label, std::span<const std::uint8_t> payload, bool binary)` | 接收并处理 on data message 相关逻辑。 |
| [L410](../src/apps/remote/InProcessSessionEngine.DirectBridge.inc#L410) | `InProcessSessionEngine::OnRemoteTrackAdded` | 定义 | `void InProcessSessionEngine::OnRemoteTrackAdded( const RemoteTrackInfo& track)` | 接收并处理 on remote track added 相关逻辑。 |

## `src/apps/remote/InProcessSessionEngine.DirectCallbacks.inc`

[打开源码](../src/apps/remote/InProcessSessionEngine.DirectCallbacks.inc) · **文件作用：** `InProcessSessionEngine` 的实现切片，集中实现 direct callbacks 相关逻辑；成员状态仍定义在所属头文件中。

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L4](../src/apps/remote/InProcessSessionEngine.DirectCallbacks.inc#L4) | `InProcessSessionEngine::OnSignalingStateChanged` | 定义 | `void InProcessSessionEngine::OnSignalingStateChanged( SignalingConnectionState state)` | SPDX-License-Identifier: GPL-3.0-only Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd) |
| [L86](../src/apps/remote/InProcessSessionEngine.DirectCallbacks.inc#L86) | `InProcessSessionEngine::OnDeviceRegistered` | 定义 | `void InProcessSessionEngine::OnDeviceRegistered(const std::string& deviceId)` | 接收并处理 on device registered 相关逻辑。 |
| [L163](../src/apps/remote/InProcessSessionEngine.DirectCallbacks.inc#L163) | `InProcessSessionEngine::OnOwnedDevicesChanged` | 定义 | `void InProcessSessionEngine::OnOwnedDevicesChanged( const SignalingOwnedDevicesSnapshot& owned)` | 接收并处理 on owned devices changed 相关逻辑。 |
| [L189](../src/apps/remote/InProcessSessionEngine.DirectCallbacks.inc#L189) | `InProcessSessionEngine::OnIncomingSessionRequest` | 定义 | `void InProcessSessionEngine::OnIncomingSessionRequest( const IncomingSessionRequest& request)` | 接收并处理 on incoming session request 相关逻辑。 |
| [L278](../src/apps/remote/InProcessSessionEngine.DirectCallbacks.inc#L278) | `InProcessSessionEngine::OnSessionResponse` | 定义 | `void InProcessSessionEngine::OnSessionResponse( const SignalingSessionResponse& response)` | 接收并处理 on session response 相关逻辑。 |
| [L302](../src/apps/remote/InProcessSessionEngine.DirectCallbacks.inc#L302) | `InProcessSessionEngine::OnSessionPending` | 定义 | `void InProcessSessionEngine::OnSessionPending( const SignalingSessionPending& pending)` | 接收并处理 on session pending 相关逻辑。 |
| [L340](../src/apps/remote/InProcessSessionEngine.DirectCallbacks.inc#L340) | `InProcessSessionEngine::OnSessionReady` | 定义 | `void InProcessSessionEngine::OnSessionReady( const SignalingSessionReady& ready)` | 接收并处理 on session ready 相关逻辑。 |
| [L419](../src/apps/remote/InProcessSessionEngine.DirectCallbacks.inc#L419) | `InProcessSessionEngine::OnSessionSuspended` | 定义 | `void InProcessSessionEngine::OnSessionSuspended( const SignalingSessionSuspended& suspended)` | 接收并处理 on session suspended 相关逻辑。 |
| [L438](../src/apps/remote/InProcessSessionEngine.DirectCallbacks.inc#L438) | `InProcessSessionEngine::OnSessionResumed` | 定义 | `void InProcessSessionEngine::OnSessionResumed( const SignalingSessionResumed& resumed)` | 接收并处理 on session resumed 相关逻辑。 |
| [L469](../src/apps/remote/InProcessSessionEngine.DirectCallbacks.inc#L469) | `InProcessSessionEngine::OnSessionEnded` | 定义 | `void InProcessSessionEngine::OnSessionEnded( const SignalingSessionEnded& ended)` | 接收并处理 on session ended 相关逻辑。 |
| [L514](../src/apps/remote/InProcessSessionEngine.DirectCallbacks.inc#L514) | `InProcessSessionEngine::OnRemoteDescription` | 定义 | `void InProcessSessionEngine::OnRemoteDescription( const SignalingSessionDescription& description)` | 接收并处理 on remote description 相关逻辑。 |
| [L554](../src/apps/remote/InProcessSessionEngine.DirectCallbacks.inc#L554) | `InProcessSessionEngine::OnRemoteIceCandidate` | 定义 | `void InProcessSessionEngine::OnRemoteIceCandidate( const SignalingIceCandidate& candidate)` | 接收并处理 on remote ice candidate 相关逻辑。 |
| [L593](../src/apps/remote/InProcessSessionEngine.DirectCallbacks.inc#L593) | `InProcessSessionEngine::OnIceRestartRequested` | 定义 | `void InProcessSessionEngine::OnIceRestartRequested( const SignalingIceRestartRequest& request)` | 接收并处理 on ice restart requested 相关逻辑。 |
| [L616](../src/apps/remote/InProcessSessionEngine.DirectCallbacks.inc#L616) | `InProcessSessionEngine::OnIceRestartCancelled` | 定义 | `void InProcessSessionEngine::OnIceRestartCancelled( const SignalingIceRestartCancel& cancel)` | 接收并处理 on ice restart cancelled 相关逻辑。 |
| [L639](../src/apps/remote/InProcessSessionEngine.DirectCallbacks.inc#L639) | `InProcessSessionEngine::OnHeartbeatAcknowledged` | 定义 | `void InProcessSessionEngine::OnHeartbeatAcknowledged( std::uint32_t roundTripMs)` | 接收并处理 on heartbeat acknowledged 相关逻辑。 |
| [L645](../src/apps/remote/InProcessSessionEngine.DirectCallbacks.inc#L645) | `InProcessSessionEngine::OnSignalingError` | 定义 | `void InProcessSessionEngine::OnSignalingError( const std::string& code, const std::string& message)` | 接收并处理 on signaling error 相关逻辑。 |
| [L706](../src/apps/remote/InProcessSessionEngine.DirectCallbacks.inc#L706) | `InProcessSessionEngine::OnAccountDeletionResult` | 定义 | `void InProcessSessionEngine::OnAccountDeletionResult( const SignalingAccountDeletionResult& result)` | 接收并处理 on account deletion result 相关逻辑。 |

## `src/apps/remote/InProcessSessionEngine.DirectData.inc`

[打开源码](../src/apps/remote/InProcessSessionEngine.DirectData.inc) · **文件作用：** `InProcessSessionEngine` 的实现切片，集中实现 direct data 相关逻辑；成员状态仍定义在所属头文件中。

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L4](../src/apps/remote/InProcessSessionEngine.DirectData.inc#L4) | `InProcessSessionEngine::SendRemoteInput` | 定义 | `SessionCommandResult InProcessSessionEngine::SendRemoteInput( const RemoteInputEvent& event)` | SPDX-License-Identifier: GPL-3.0-only Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd) |
| [L16](../src/apps/remote/InProcessSessionEngine.DirectData.inc#L16) | `InProcessSessionEngine::SendRemoteFileMessage` | 定义 | `SessionCommandResult InProcessSessionEngine::SendRemoteFileMessage( const std::string& peerDeviceId, const FileTransferMessage& message)` | 发送或发布 send remote file message 相关逻辑。 |
| [L29](../src/apps/remote/InProcessSessionEngine.DirectData.inc#L29) | `InProcessSessionEngine::SendRemoteClipboardMessage` | 定义 | `SessionCommandResult InProcessSessionEngine::SendRemoteClipboardMessage( const std::string& peerDeviceId, const std::string& clipboardSessionId, const ClipboardMessage& message)` | 发送或发布 send remote clipboard message 相关逻辑。 |
| [L45](../src/apps/remote/InProcessSessionEngine.DirectData.inc#L45) | `InProcessSessionEngine::SendDirectFileMessage` | 定义 | `SessionCommandResult InProcessSessionEngine::SendDirectFileMessage( const std::string& peerDeviceId, const FileTransferMessage& message)` | 发送或发布 send direct file message 相关逻辑。 |
| [L117](../src/apps/remote/InProcessSessionEngine.DirectData.inc#L117) | `InProcessSessionEngine::SendDirectClipboardMessage` | 定义 | `SessionCommandResult InProcessSessionEngine::SendDirectClipboardMessage( const std::string& peerDeviceId, const std::string& clipboardSessionId, const ClipboardMessage& message)` | 发送或发布 send direct clipboard message 相关逻辑。 |
| [L198](../src/apps/remote/InProcessSessionEngine.DirectData.inc#L198) | `InProcessSessionEngine::DispatchDirectAuxiliaryData` | 定义 | `bool InProcessSessionEngine::DispatchDirectAuxiliaryData( const std::string& label, std::span<const std::uint8_t> payload)` | 接收并处理 dispatch direct auxiliary data 相关逻辑。 |

## `src/apps/remote/InProcessSessionEngine.DirectMedia.inc`

[打开源码](../src/apps/remote/InProcessSessionEngine.DirectMedia.inc) · **文件作用：** `InProcessSessionEngine` 的实现切片，集中实现 direct media 相关逻辑；成员状态仍定义在所属头文件中。

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L4](../src/apps/remote/InProcessSessionEngine.DirectMedia.inc#L4) | `InProcessSessionEngine::PrepareDirectMedia` | 定义 | `std::optional<OperationError> InProcessSessionEngine::PrepareDirectMedia( bool bindNegotiatedSlots)` | SPDX-License-Identifier: GPL-3.0-only Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd) |
| [L209](../src/apps/remote/InProcessSessionEngine.DirectMedia.inc#L209) | `InProcessSessionEngine::SetDirectRemoteVideoSink` | 定义 | `SessionCommandResult InProcessSessionEngine::SetDirectRemoteVideoSink( webrtc::VideoSinkInterface<webrtc::VideoFrame>* sink)` | 更新或应用 set direct remote video sink 相关逻辑。 |
| [L229](../src/apps/remote/InProcessSessionEngine.DirectMedia.inc#L229) | `InProcessSessionEngine::StopDirectDesktopCapture` | 定义 | `void InProcessSessionEngine::StopDirectDesktopCapture()` | 停止 stop direct desktop capture 相关逻辑。 |
| [L259](../src/apps/remote/InProcessSessionEngine.DirectMedia.inc#L259) | `InProcessSessionEngine::StopDirectMicrophoneCapture` | 定义 | `void InProcessSessionEngine::StopDirectMicrophoneCapture()` | 停止 stop direct microphone capture 相关逻辑。 |
| [L282](../src/apps/remote/InProcessSessionEngine.DirectMedia.inc#L282) | `InProcessSessionEngine::BroadcastDirectSharedDisplayLayout` | 定义 | `void InProcessSessionEngine::BroadcastDirectSharedDisplayLayout()` | 实现 broadcast direct shared display layout 对应的业务或工具逻辑。 |
| [L310](../src/apps/remote/InProcessSessionEngine.DirectMedia.inc#L310) | `InProcessSessionEngine::RequestDirectSharedDisplayLayout` | 定义 | `void InProcessSessionEngine::RequestDirectSharedDisplayLayout()` | 发起请求或查询 request direct shared display layout 相关逻辑。 |
| [L341](../src/apps/remote/InProcessSessionEngine.DirectMedia.inc#L341) | `InProcessSessionEngine::SendDirectInput` | 定义 | `SessionCommandResult InProcessSessionEngine::SendDirectInput( const RemoteInputEvent& event)` | 发送或发布 send direct input 相关逻辑。 |

## `src/apps/remote/InProcessSessionEngine.DirectScreen.inc`

[打开源码](../src/apps/remote/InProcessSessionEngine.DirectScreen.inc) · **文件作用：** `InProcessSessionEngine` 的实现切片，集中实现 direct screen 相关逻辑；成员状态仍定义在所属头文件中。

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L4](../src/apps/remote/InProcessSessionEngine.DirectScreen.inc#L4) | `InProcessSessionEngine::SetDirectScreenStreamPreference` | 定义 | `SessionCommandResult InProcessSessionEngine::SetDirectScreenStreamPreference( const ScreenStreamPreferenceRequest& preference)` | SPDX-License-Identifier: GPL-3.0-only Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd) |
| [L72](../src/apps/remote/InProcessSessionEngine.DirectScreen.inc#L72) | `InProcessSessionEngine::RequestDirectSharedDisplaySwitch` | 定义 | `SessionCommandResult InProcessSessionEngine::RequestDirectSharedDisplaySwitch( const std::string& stableDisplayKey)` | 发起请求或查询 request direct shared display switch 相关逻辑。 |
| [L140](../src/apps/remote/InProcessSessionEngine.DirectScreen.inc#L140) | `InProcessSessionEngine::BroadcastDirectSharedDisplayCatalog` | 定义 | `void InProcessSessionEngine::BroadcastDirectSharedDisplayCatalog()` | 实现 broadcast direct shared display catalog 对应的业务或工具逻辑。 |
| [L170](../src/apps/remote/InProcessSessionEngine.DirectScreen.inc#L170) | `InProcessSessionEngine::SwitchLocalDirectDisplay` | 定义 | `SessionCommandResult InProcessSessionEngine::SwitchLocalDirectDisplay( const std::string& stableDisplayKey)` | 实现 switch local direct display 对应的业务或工具逻辑。 |
| [L321](../src/apps/remote/InProcessSessionEngine.DirectScreen.inc#L321) | `InProcessSessionEngine::DispatchDirectScreenData` | 定义 | `bool InProcessSessionEngine::DispatchDirectScreenData( const std::string& label, std::span<const std::uint8_t> payload)` | 接收并处理 dispatch direct screen data 相关逻辑。 |

## `src/apps/remote/InProcessSessionEngine.DirectSession.inc`

[打开源码](../src/apps/remote/InProcessSessionEngine.DirectSession.inc) · **文件作用：** `InProcessSessionEngine` 的实现切片，集中实现 direct session 相关逻辑；成员状态仍定义在所属头文件中。

### 类型

| 行 | 类型 | 种类 | 作用 |
|---:|---|---|---|
| [L208](../src/apps/remote/InProcessSessionEngine.DirectSession.inc#L208) | `EndAction` | enum class | 定义 EndAction 的 enum class 类型和相关状态。 |

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L4](../src/apps/remote/InProcessSessionEngine.DirectSession.inc#L4) | `InProcessSessionEngine::ConnectDevice` | 定义 | `SessionCommandResult InProcessSessionEngine::ConnectDevice( const std::string& deviceId, SessionPurpose purpose)` | SPDX-License-Identifier: GPL-3.0-only Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd) |
| [L12](../src/apps/remote/InProcessSessionEngine.DirectSession.inc#L12) | `InProcessSessionEngine::ConnectDirectDevice` | 定义 | `SessionCommandResult InProcessSessionEngine::ConnectDirectDevice( const DirectSessionConnectRequest& request)` | 建立连接 connect direct device 相关逻辑。 |
| [L114](../src/apps/remote/InProcessSessionEngine.DirectSession.inc#L114) | `InProcessSessionEngine::ConnectOwnedDevice` | 定义 | `SessionCommandResult InProcessSessionEngine::ConnectOwnedDevice( const std::string& deviceId, SessionPurpose purpose)` | 建立连接 connect owned device 相关逻辑。 |
| [L122](../src/apps/remote/InProcessSessionEngine.DirectSession.inc#L122) | `InProcessSessionEngine::ConnectAssistedDevice` | 定义 | `SessionCommandResult InProcessSessionEngine::ConnectAssistedDevice( const std::string& deviceId, const std::string& verificationCode)` | 建立连接 connect assisted device 相关逻辑。 |
| [L133](../src/apps/remote/InProcessSessionEngine.DirectSession.inc#L133) | `InProcessSessionEngine::RefreshOwnedDevices` | 定义 | `SessionCommandResult InProcessSessionEngine::RefreshOwnedDevices()` | 刷新 refresh owned devices 相关逻辑。 |
| [L143](../src/apps/remote/InProcessSessionEngine.DirectSession.inc#L143) | `InProcessSessionEngine::AcceptIncomingSession` | 定义 | `SessionCommandResult InProcessSessionEngine::AcceptIncomingSession( const std::string& sessionId)` | 处理并回复 accept incoming session 相关逻辑。 |
| [L175](../src/apps/remote/InProcessSessionEngine.DirectSession.inc#L175) | `InProcessSessionEngine::RejectIncomingSession` | 定义 | `SessionCommandResult InProcessSessionEngine::RejectIncomingSession( const std::string& sessionId)` | 处理并回复 reject incoming session 相关逻辑。 |
| [L206](../src/apps/remote/InProcessSessionEngine.DirectSession.inc#L206) | `InProcessSessionEngine::Disconnect` | 定义 | `SessionCommandResult InProcessSessionEngine::Disconnect()` | 断开连接 disconnect 相关逻辑。 |

## `src/apps/remote/InProcessSessionEngine.h`

[打开源码](../src/apps/remote/InProcessSessionEngine.h) · **文件作用：** 声明 in process session engine 相关类型、接口、配置和成员状态。

### 类型

| 行 | 类型 | 种类 | 作用 |
|---:|---|---|---|
| [L33](../src/apps/remote/InProcessSessionEngine.h#L33) | `WebRtcRuntime` | class | 定义 WebRtcRuntime 的 class 类型和相关状态。 |
| [L34](../src/apps/remote/InProcessSessionEngine.h#L34) | `LibWebRtcSession` | class | 定义 LibWebRtcSession 的 class 类型和相关状态。 |
| [L35](../src/apps/remote/InProcessSessionEngine.h#L35) | `RoomMemberActionEnvelope` | struct | 定义 RoomMemberActionEnvelope 的 struct 类型和相关状态。 |
| [L38](../src/apps/remote/InProcessSessionEngine.h#L38) | `InProcessSessionEngineTestAccess` | class | 定义 InProcessSessionEngineTestAccess 的 class 类型和相关状态。 |
| [L43](../src/apps/remote/InProcessSessionEngine.h#L43) | `InProcessSessionEngineOptions` | struct | 定义 InProcessSessionEngineOptions 的 struct 类型和相关状态。 |
| [L78](../src/apps/remote/InProcessSessionEngine.h#L78) | `InProcessSessionEngine` | class | 定义 InProcessSessionEngine 的 class 类型和相关状态。 |
| [L412](../src/apps/remote/InProcessSessionEngine.h#L412) | `RoomPairBridge` | class | 定义 RoomPairBridge 的 class 类型和相关状态。 |
| [L413](../src/apps/remote/InProcessSessionEngine.h#L413) | `RoomPairRuntime` | struct | 定义 RoomPairRuntime 的 struct 类型和相关状态。 |

### 成员与文件级变量

| 行 | 变量 | 声明 | 作用 |
|---:|---|---|---|
| [L33](../src/apps/remote/InProcessSessionEngine.h#L33) | `WebRtcRuntime` | `class WebRtcRuntime;` | 保存 web rtc runtime 相关配置或运行状态。 |
| [L34](../src/apps/remote/InProcessSessionEngine.h#L34) | `LibWebRtcSession` | `class LibWebRtcSession;` | 保存 lib web rtc session 相关配置或运行状态。 |
| [L35](../src/apps/remote/InProcessSessionEngine.h#L35) | `RoomMemberActionEnvelope` | `struct RoomMemberActionEnvelope;` | 保存 room member action envelope 相关配置或运行状态。 |
| [L38](../src/apps/remote/InProcessSessionEngine.h#L38) | `InProcessSessionEngineTestAccess` | `class InProcessSessionEngineTestAccess;` | 保存 in process session engine test access 相关配置或运行状态。 |
| [L44](../src/apps/remote/InProcessSessionEngine.h#L44) | `includeLoopbackAdapter` | `bool includeLoopbackAdapter = false;` | 保存 include loopback adapter 相关配置或运行状态。 |
| [L45](../src/apps/remote/InProcessSessionEngine.h#L45) | `enableRealDesktopCapture` | `bool enableRealDesktopCapture = true;` | 保存 enable real desktop capture 相关配置或运行状态。 |
| [L46](../src/apps/remote/InProcessSessionEngine.h#L46) | `enableRealCameraCapture` | `bool enableRealCameraCapture = true;` | 保存 enable real camera capture 相关配置或运行状态。 |
| [L48](../src/apps/remote/InProcessSessionEngine.h#L48) | `kNativeDxgi` | `DesktopCaptureImplementation::kNativeDxgi;` | 定义 native dxgi 的编译期常量或产品边界。 |
| [L50](../src/apps/remote/InProcessSessionEngine.h#L50) | `kAutomatic` | `VideoEncoderPreference::kAutomatic;` | 定义 automatic 的编译期常量或产品边界。 |
| [L51](../src/apps/remote/InProcessSessionEngine.h#L51) | `ffmpegX264Preset` | `FfmpegX264Preset ffmpegX264Preset = FfmpegX264Preset::kMedium;` | 保存 ffmpeg x264 preset 相关配置或运行状态。 |
| [L53](../src/apps/remote/InProcessSessionEngine.h#L53) | `kAutomatic` | `FfmpegHardwareBackend::kAutomatic;` | 定义 automatic 的编译期常量或产品边界。 |
| [L54](../src/apps/remote/InProcessSessionEngine.h#L54) | `preferredAutomaticEncoderId` | `std::string preferredAutomaticEncoderId;` | 保存身份或作用域标识：preferred automatic encoder id。 |
| [L56](../src/apps/remote/InProcessSessionEngine.h#L56) | `kAutomatic` | `VideoDecoderPreference::kAutomatic;` | 定义 automatic 的编译期常量或产品边界。 |
| [L57](../src/apps/remote/InProcessSessionEngine.h#L57) | `preferredHardwareDecoderName` | `std::string preferredHardwareDecoderName;` | 保存路径、地址或显示名称：preferred hardware decoder name。 |
| [L58](../src/apps/remote/InProcessSessionEngine.h#L58) | `hardwareFingerprint` | `std::string hardwareFingerprint;` | 保存 hardware fingerprint 相关配置或运行状态。 |
| [L59](../src/apps/remote/InProcessSessionEngine.h#L59) | `operatingSystemDescription` | `std::string operatingSystemDescription;` | 保存 operating system description 相关配置或运行状态。 |
| [L60](../src/apps/remote/InProcessSessionEngine.h#L60) | `nativeArchitecture` | `std::string nativeArchitecture;` | 保存 native architecture 相关配置或运行状态。 |
| [L61](../src/apps/remote/InProcessSessionEngine.h#L61) | `remoteSession` | `bool remoteSession = false;` | 保存 remote session 相关配置或运行状态。 |
| [L62](../src/apps/remote/InProcessSessionEngine.h#L62) | `graphicsAdapterDescriptions` | `std::vector<std::string> graphicsAdapterDescriptions;` | 保存 graphics adapter descriptions 相关配置或运行状态。 |
| [L63](../src/apps/remote/InProcessSessionEngine.h#L63) | `graphicsEnumerationError` | `std::string graphicsEnumerationError;` | 保存最近错误或失败原因：graphics enumeration error。 |
| [L65](../src/apps/remote/InProcessSessionEngine.h#L65) | `encoderCapabilityCache` | `encoderCapabilityCache;` | 保存 encoder capability cache 相关配置或运行状态。 |
| [L67](../src/apps/remote/InProcessSessionEngine.h#L67) | `kSystemDefaultMediaDeviceId` | `kSystemDefaultMediaDeviceId;` | 定义 system default media device id 的编译期常量或产品边界。 |
| [L69](../src/apps/remote/InProcessSessionEngine.h#L69) | `kSystemDefaultMediaDeviceId` | `kSystemDefaultMediaDeviceId;` | 定义 system default media device id 的编译期常量或产品边界。 |
| [L71](../src/apps/remote/InProcessSessionEngine.h#L71) | `kSystemDefaultMediaDeviceId` | `kSystemDefaultMediaDeviceId;` | 定义 system default media device id 的编译期常量或产品边界。 |
| [L72](../src/apps/remote/InProcessSessionEngine.h#L72) | `iceMinPort` | `int iceMinPort = kDefaultIceMinPort;` | 保存 ice min port 相关配置或运行状态。 |
| [L73](../src/apps/remote/InProcessSessionEngine.h#L73) | `iceMaxPort` | `int iceMaxPort = kDefaultIceMaxPort;` | 保存 ice max port 相关配置或运行状态。 |
| [L74](../src/apps/remote/InProcessSessionEngine.h#L74) | `negotiationTimeout` | `std::chrono::milliseconds negotiationTimeout{15000};` | 保存定时、截止或超时状态：negotiation timeout。 |
| [L75](../src/apps/remote/InProcessSessionEngine.h#L75) | `reconnectTimeout` | `std::chrono::milliseconds reconnectTimeout{60000};` | 保存定时、截止或超时状态：reconnect timeout。 |
| [L412](../src/apps/remote/InProcessSessionEngine.h#L412) | `RoomPairBridge` | `class RoomPairBridge;` | 保存 room pair bridge 相关配置或运行状态。 |
| [L413](../src/apps/remote/InProcessSessionEngine.h#L413) | `RoomPairRuntime` | `struct RoomPairRuntime;` | 保存 room pair runtime 相关配置或运行状态。 |
| [L432](../src/apps/remote/InProcessSessionEngine.h#L432) | `runtime_` | `std::unique_ptr<WebRtcRuntime> runtime_;` | 保存 runtime 相关配置或运行状态。 |
| [L433](../src/apps/remote/InProcessSessionEngine.h#L433) | `signaling_` | `std::unique_ptr<ISignalingClient> signaling_;` | 保存 signaling 相关配置或运行状态。 |
| [L434](../src/apps/remote/InProcessSessionEngine.h#L434) | `signalingConfig_` | `SignalingClientConfig signalingConfig_;` | 保存 signaling config 相关配置或运行状态。 |
| [L435](../src/apps/remote/InProcessSessionEngine.h#L435) | `options_` | `InProcessSessionEngineOptions options_;` | 保存 options 相关配置或运行状态。 |
| [L436](../src/apps/remote/InProcessSessionEngine.h#L436) | `webRtcSession_` | `std::unique_ptr<LibWebRtcSession> webRtcSession_;` | 保存 web rtc session 相关配置或运行状态。 |
| [L437](../src/apps/remote/InProcessSessionEngine.h#L437) | `sessionController_` | `std::unique_ptr<SessionControllerBase> sessionController_;` | 保存 session controller 相关配置或运行状态。 |
| [L439](../src/apps/remote/InProcessSessionEngine.h#L439) | `roomPairs_` | `roomPairs_;` | 保存 room pairs 相关配置或运行状态。 |
| [L440](../src/apps/remote/InProcessSessionEngine.h#L440) | `retiredRoomPairThreads_` | `std::vector<std::jthread> retiredRoomPairThreads_;` | 保存 retired room pair threads 相关配置或运行状态。 |
| [L443](../src/apps/remote/InProcessSessionEngine.h#L443) | `retiredDesktopStopThreads_` | `std::vector<std::jthread> retiredDesktopStopThreads_;` | Device drivers may block while stopping a capture module. Keep those joins outside the Qt UI path, but retain ownership until engine stop. |
| [L444](../src/apps/remote/InProcessSessionEngine.h#L444) | `retiredCameraStopThreads_` | `std::vector<std::jthread> retiredCameraStopThreads_;` | 保存 retired camera stop threads 相关配置或运行状态。 |
| [L445](../src/apps/remote/InProcessSessionEngine.h#L445) | `mediaDeviceOperationThreads_` | `std::vector<std::jthread> mediaDeviceOperationThreads_;` | 保存 media device operation threads 相关配置或运行状态。 |
| [L446](../src/apps/remote/InProcessSessionEngine.h#L446) | `clipboardWarmupThreads_` | `std::vector<std::jthread> clipboardWarmupThreads_;` | 保存 clipboard warmup threads 相关配置或运行状态。 |
| [L447](../src/apps/remote/InProcessSessionEngine.h#L447) | `statsPollingThread_` | `std::jthread statsPollingThread_;` | 拥有后台执行线程或工作器：stats polling thread。 |
| [L448](../src/apps/remote/InProcessSessionEngine.h#L448) | `statsPollingCondition_` | `std::condition_variable_any statsPollingCondition_;` | 保存 stats polling condition 相关配置或运行状态。 |
| [L449](../src/apps/remote/InProcessSessionEngine.h#L449) | `statsPollingWaitMutex_` | `std::mutex statsPollingWaitMutex_;` | 保护跨线程共享状态：stats polling wait mutex。 |
| [L453](../src/apps/remote/InProcessSessionEngine.h#L453) | `localRoomVideoTracks_` | `localRoomVideoTracks_;` | 保存 local room video tracks 相关配置或运行状态。 |
| [L457](../src/apps/remote/InProcessSessionEngine.h#L457) | `idleRoomVideoTracks_` | `idleRoomVideoTracks_;` | 保存 idle room video tracks 相关配置或运行状态。 |
| [L459](../src/apps/remote/InProcessSessionEngine.h#L459) | `localDesktopCaptureSource_` | `localDesktopCaptureSource_;` | 保存 local desktop capture source 相关配置或运行状态。 |
| [L461](../src/apps/remote/InProcessSessionEngine.h#L461) | `localCameraCaptureSource_` | `localCameraCaptureSource_;` | 保存 local camera capture source 相关配置或运行状态。 |
| [L463](../src/apps/remote/InProcessSessionEngine.h#L463) | `localCameraPreviewSink_` | `localCameraPreviewSink_ = nullptr;` | 保存回调或观察者入口：local camera preview sink。 |
| [L465](../src/apps/remote/InProcessSessionEngine.h#L465) | `localCameraPreviewTrack_` | `localCameraPreviewTrack_;` | 保存 local camera preview track 相关配置或运行状态。 |
| [L467](../src/apps/remote/InProcessSessionEngine.h#L467) | `localMicrophoneAudioSource_` | `localMicrophoneAudioSource_;` | 保存 local microphone audio source 相关配置或运行状态。 |
| [L469](../src/apps/remote/InProcessSessionEngine.h#L469) | `localMicrophoneAudioTrack_` | `localMicrophoneAudioTrack_;` | 保存 local microphone audio track 相关配置或运行状态。 |
| [L470](../src/apps/remote/InProcessSessionEngine.h#L470) | `pendingRemoteDescription_` | `std::optional<SessionDescription> pendingRemoteDescription_;` | 保存 pending remote description 相关配置或运行状态。 |
| [L471](../src/apps/remote/InProcessSessionEngine.h#L471) | `pendingRemoteCandidates_` | `std::vector<IceCandidate> pendingRemoteCandidates_;` | 保存 pending remote candidates 相关配置或运行状态。 |
| [L472](../src/apps/remote/InProcessSessionEngine.h#L472) | `mutex_` | `mutable std::mutex mutex_;` | 保护跨线程共享状态：mutex。 |
| [L473](../src/apps/remote/InProcessSessionEngine.h#L473) | `observer_` | `ISessionEngineObserver* observer_ = nullptr;` | 保存回调或观察者入口：observer。 |
| [L475](../src/apps/remote/InProcessSessionEngine.h#L475) | `accountDeletionResultCallback_` | `accountDeletionResultCallback_;` | 保存回调或观察者入口：account deletion result callback。 |
| [L476](../src/apps/remote/InProcessSessionEngine.h#L476) | `snapshot_` | `SessionEngineSnapshot snapshot_;` | 保存可跨层读取的状态快照：snapshot。 |
| [L477](../src/apps/remote/InProcessSessionEngine.h#L477) | `capabilities_` | `SessionEngineCapabilities capabilities_;` | 保存 capabilities 相关配置或运行状态。 |
| [L478](../src/apps/remote/InProcessSessionEngine.h#L478) | `localIsOfferer_` | `bool localIsOfferer_ = false;` | 保存 local is offerer 相关配置或运行状态。 |
| [L479](../src/apps/remote/InProcessSessionEngine.h#L479) | `offerNegotiationStarted_` | `bool offerNegotiationStarted_ = false;` | 保存 offer negotiation started 相关配置或运行状态。 |
| [L480](../src/apps/remote/InProcessSessionEngine.h#L480) | `directSession_` | `DirectSessionRuntimeState directSession_;` | 保存 direct session 相关配置或运行状态。 |
| [L481](../src/apps/remote/InProcessSessionEngine.h#L481) | `roomAudioDevicesApplied_` | `bool roomAudioDevicesApplied_ = false;` | 保存 room audio devices applied 相关配置或运行状态。 |
| [L482](../src/apps/remote/InProcessSessionEngine.h#L482) | `sessionCloseRequested_` | `bool sessionCloseRequested_ = false;` | 保存 session close requested 相关配置或运行状态。 |
| [L483](../src/apps/remote/InProcessSessionEngine.h#L483) | `sessionEndSignalSent_` | `bool sessionEndSignalSent_ = false;` | 保存 session end signal sent 相关配置或运行状态。 |
| [L484](../src/apps/remote/InProcessSessionEngine.h#L484) | `cancelWhenSessionIdKnown_` | `bool cancelWhenSessionIdKnown_ = false;` | 保存 cancel when session id known 相关配置或运行状态。 |
| [L485](../src/apps/remote/InProcessSessionEngine.h#L485) | `serverSessionActive_` | `bool serverSessionActive_ = false;` | 保存能力或开关状态：server session active。 |
| [L486](../src/apps/remote/InProcessSessionEngine.h#L486) | `signalingRecoveryPending_` | `bool signalingRecoveryPending_ = false;` | 保存待处理队列或请求：signaling recovery pending。 |
| [L487](../src/apps/remote/InProcessSessionEngine.h#L487) | `peerSignalingSuspended_` | `bool peerSignalingSuspended_ = false;` | 保存 peer signaling suspended 相关配置或运行状态。 |
| [L488](../src/apps/remote/InProcessSessionEngine.h#L488) | `sessionRecoveryToken_` | `std::string sessionRecoveryToken_;` | 保存 session recovery token 相关配置或运行状态。 |
| [L489](../src/apps/remote/InProcessSessionEngine.h#L489) | `roomRecoveryToken_` | `std::string roomRecoveryToken_;` | 保存 room recovery token 相关配置或运行状态。 |
| [L490](../src/apps/remote/InProcessSessionEngine.h#L490) | `roomScreenShareGrantId_` | `std::string roomScreenShareGrantId_;` | 保存身份或作用域标识：room screen share grant id。 |
| [L491](../src/apps/remote/InProcessSessionEngine.h#L491) | `roomControlGrantId_` | `std::string roomControlGrantId_;` | 保存身份或作用域标识：room control grant id。 |
| [L492](../src/apps/remote/InProcessSessionEngine.h#L492) | `roomControlGrantScreenSharerDeviceId_` | `std::string roomControlGrantScreenSharerDeviceId_;` | 保存身份或作用域标识：room control grant screen sharer device id。 |
| [L493](../src/apps/remote/InProcessSessionEngine.h#L493) | `roomControlGrantControllerDeviceId_` | `std::string roomControlGrantControllerDeviceId_;` | 保存身份或作用域标识：room control grant controller device id。 |
| [L494](../src/apps/remote/InProcessSessionEngine.h#L494) | `remoteInputSink_` | `IRemoteInputSink* remoteInputSink_ = nullptr;` | 保存回调或观察者入口：remote input sink。 |
| [L495](../src/apps/remote/InProcessSessionEngine.h#L495) | `remoteFileTransferSink_` | `IFileTransferSink* remoteFileTransferSink_ = nullptr;` | 保存回调或观察者入口：remote file transfer sink。 |
| [L496](../src/apps/remote/InProcessSessionEngine.h#L496) | `remoteClipboardSink_` | `IClipboardSink* remoteClipboardSink_ = nullptr;` | 保存回调或观察者入口：remote clipboard sink。 |
| [L497](../src/apps/remote/InProcessSessionEngine.h#L497) | `remoteCursorCallback_` | `RemoteCursorCallback remoteCursorCallback_;` | 保存回调或观察者入口：remote cursor callback。 |
| [L498](../src/apps/remote/InProcessSessionEngine.h#L498) | `cursorMonitor_` | `WindowsCursorMonitor cursorMonitor_;` | 保存 cursor monitor 相关配置或运行状态。 |
| [L499](../src/apps/remote/InProcessSessionEngine.h#L499) | `latestLocalCursorPosition_` | `std::optional<RemoteCursorPosition> latestLocalCursorPosition_;` | 保存 latest local cursor position 相关配置或运行状态。 |
| [L500](../src/apps/remote/InProcessSessionEngine.h#L500) | `latestLocalCursorShape_` | `std::optional<RemoteCursorShape> latestLocalCursorShape_;` | 保存 latest local cursor shape 相关配置或运行状态。 |
| [L501](../src/apps/remote/InProcessSessionEngine.h#L501) | `nextCursorSequence_` | `std::uint64_t nextCursorSequence_ = 0;` | 保存单调序号，用于排序或去重：next cursor sequence。 |
| [L502](../src/apps/remote/InProcessSessionEngine.h#L502) | `cursorPositionsPublished_` | `std::uint64_t cursorPositionsPublished_ = 0;` | 保存 cursor positions published 相关配置或运行状态。 |
| [L503](../src/apps/remote/InProcessSessionEngine.h#L503) | `cursorShapesPublished_` | `std::uint64_t cursorShapesPublished_ = 0;` | 保存 cursor shapes published 相关配置或运行状态。 |
| [L504](../src/apps/remote/InProcessSessionEngine.h#L504) | `cursorPositionsReceived_` | `std::uint64_t cursorPositionsReceived_ = 0;` | 保存 cursor positions received 相关配置或运行状态。 |
| [L505](../src/apps/remote/InProcessSessionEngine.h#L505) | `cursorShapesReceived_` | `std::uint64_t cursorShapesReceived_ = 0;` | 保存 cursor shapes received 相关配置或运行状态。 |
| [L508](../src/apps/remote/InProcessSessionEngine.h#L508) | `nextRoomInputSequence_` | `std::uint64_t nextRoomInputSequence_ = 0;` | One sequence spans both input DataChannels so late reliable button transitions cannot overwrite newer fast pointer state. |
| [L509](../src/apps/remote/InProcessSessionEngine.h#L509) | `nextRoomScreenControlSequence_` | `std::uint64_t nextRoomScreenControlSequence_ = 0;` | 保存单调序号，用于排序或去重：next room screen control sequence。 |
| [L510](../src/apps/remote/InProcessSessionEngine.h#L510) | `localScreenFrameRate_` | `std::uint32_t localScreenFrameRate_ = kDefaultScreenFrameRate;` | 保存计数、尺寸或速率指标：local screen frame rate。 |
| [L511](../src/apps/remote/InProcessSessionEngine.h#L511) | `localScreenShareGeneration_` | `std::uint64_t localScreenShareGeneration_ = 0;` | 标记当前世代，用于拒绝过期异步结果：local screen share generation。 |
| [L512](../src/apps/remote/InProcessSessionEngine.h#L512) | `localCameraTrackGeneration_` | `std::uint64_t localCameraTrackGeneration_ = 0;` | 标记当前世代，用于拒绝过期异步结果：local camera track generation。 |
| [L514](../src/apps/remote/InProcessSessionEngine.h#L514) | `roomScreenStreamPreferences_` | `roomScreenStreamPreferences_;` | 保存 room screen stream preferences 相关配置或运行状态。 |
| [L515](../src/apps/remote/InProcessSessionEngine.h#L515) | `roomRecoveryPending_` | `bool roomRecoveryPending_ = false;` | 保存待处理队列或请求：room recovery pending。 |
| [L516](../src/apps/remote/InProcessSessionEngine.h#L516) | `roomLeaveRequested_` | `bool roomLeaveRequested_ = false;` | 保存 room leave requested 相关配置或运行状态。 |
| [L517](../src/apps/remote/InProcessSessionEngine.h#L517) | `deferredRoomLeaveId_` | `std::string deferredRoomLeaveId_;` | 保存身份或作用域标识：deferred room leave id。 |

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L83](../src/apps/remote/InProcessSessionEngine.h#L83) | `InProcessSessionEngine` | 声明 | `InProcessSessionEngine()` | 实现 in process session engine 对应的业务或工具逻辑。 |
| [L87](../src/apps/remote/InProcessSessionEngine.h#L87) | `~InProcessSessionEngine` | 声明 | `~InProcessSessionEngine() override` | 停止相关活动并释放 InProcessSessionEngine 实例拥有的资源。 |
| [L89](../src/apps/remote/InProcessSessionEngine.h#L89) | `InProcessSessionEngine` | 声明 | `InProcessSessionEngine(const InProcessSessionEngine&) = delete` | 实现 in process session engine 对应的业务或工具逻辑。 |
| [L92](../src/apps/remote/InProcessSessionEngine.h#L92) | `SetObserver` | 声明 | `void SetObserver(ISessionEngineObserver* observer) override` | 更新或应用 set observer 相关逻辑。 |
| [L93](../src/apps/remote/InProcessSessionEngine.h#L93) | `Start` | 声明 | `SessionCommandResult Start() override` | 启动 start 相关逻辑。 |
| [L97](../src/apps/remote/InProcessSessionEngine.h#L97) | `BeginStart` | 声明 | `SessionCommandResult BeginStart()` | Staged startup keeps the Qt-owned signaling transport on its owner thread while allowing expensive WebRTC/media initialization to run on a worker. Start() remains the synchronou... |
| [L98](../src/apps/remote/InProcessSessionEngine.h#L98) | `InitializeRuntimeForStart` | 声明 | `SessionCommandResult InitializeRuntimeForStart()` | 创建或初始化 initialize runtime for start 相关逻辑。 |
| [L99](../src/apps/remote/InProcessSessionEngine.h#L99) | `CompleteStart` | 声明 | `SessionCommandResult CompleteStart( const SessionCommandResult& runtimeResult)` | 实现 complete start 对应的业务或工具逻辑。 |
| [L101](../src/apps/remote/InProcessSessionEngine.h#L101) | `Stop` | 声明 | `void Stop() override` | 停止 stop 相关逻辑。 |
| [L105](../src/apps/remote/InProcessSessionEngine.h#L105) | `UpdateSignalingAccessToken` | 声明 | `SessionCommandResult UpdateSignalingAccessToken( std::string accessToken)` | Updates only the signaling credential retained for future authentication/reconnect. Active PeerConnections and DataChannels are intentionally left untouched. |
| [L107](../src/apps/remote/InProcessSessionEngine.h#L107) | `RequestAccountDeletion` | 声明 | `SessionCommandResult RequestAccountDeletion()` | 发起请求或查询 request account deletion 相关逻辑。 |
| [L108](../src/apps/remote/InProcessSessionEngine.h#L108) | `SetAccountDeletionResultCallback` | 声明 | `void SetAccountDeletionResultCallback( std::function<void(const SignalingAccountDeletionResult&)> callback)` | 更新或应用 set account deletion result callback 相关逻辑。 |
| [L111](../src/apps/remote/InProcessSessionEngine.h#L111) | `Snapshot` | 声明 | `SessionEngineSnapshot Snapshot() const override` | 查询并返回 snapshot 相关逻辑。 |
| [L112](../src/apps/remote/InProcessSessionEngine.h#L112) | `Capabilities` | 声明 | `SessionEngineCapabilities Capabilities() const override` | 查询并返回 capabilities 相关逻辑。 |
| [L113](../src/apps/remote/InProcessSessionEngine.h#L113) | `Diagnostics` | 声明 | `SessionDiagnosticsSnapshot Diagnostics() const override` | 查询并返回 diagnostics 相关逻辑。 |
| [L115](../src/apps/remote/InProcessSessionEngine.h#L115) | `ConnectDirectDevice` | 声明 | `SessionCommandResult ConnectDirectDevice( const DirectSessionConnectRequest& request) override` | 建立连接 connect direct device 相关逻辑。 |
| [L117](../src/apps/remote/InProcessSessionEngine.h#L117) | `ConnectDevice` | 声明 | `SessionCommandResult ConnectDevice( const std::string& deviceId, SessionPurpose purpose) override` | 建立连接 connect device 相关逻辑。 |
| [L120](../src/apps/remote/InProcessSessionEngine.h#L120) | `ConnectOwnedDevice` | 声明 | `SessionCommandResult ConnectOwnedDevice( const std::string& deviceId, SessionPurpose purpose) override` | 建立连接 connect owned device 相关逻辑。 |
| [L123](../src/apps/remote/InProcessSessionEngine.h#L123) | `ConnectAssistedDevice` | 声明 | `SessionCommandResult ConnectAssistedDevice( const std::string& deviceId, const std::string& verificationCode) override` | 建立连接 connect assisted device 相关逻辑。 |
| [L126](../src/apps/remote/InProcessSessionEngine.h#L126) | `RefreshOwnedDevices` | 声明 | `SessionCommandResult RefreshOwnedDevices() override` | 刷新 refresh owned devices 相关逻辑。 |
| [L127](../src/apps/remote/InProcessSessionEngine.h#L127) | `AcceptIncomingSession` | 声明 | `SessionCommandResult AcceptIncomingSession( const std::string& sessionId) override` | 处理并回复 accept incoming session 相关逻辑。 |
| [L129](../src/apps/remote/InProcessSessionEngine.h#L129) | `RejectIncomingSession` | 声明 | `SessionCommandResult RejectIncomingSession( const std::string& sessionId) override` | 处理并回复 reject incoming session 相关逻辑。 |
| [L131](../src/apps/remote/InProcessSessionEngine.h#L131) | `Disconnect` | 声明 | `SessionCommandResult Disconnect() override` | 断开连接 disconnect 相关逻辑。 |
| [L132](../src/apps/remote/InProcessSessionEngine.h#L132) | `CreateRoom` | 声明 | `SessionCommandResult CreateRoom(std::uint32_t capacity) override` | 创建或初始化 create room 相关逻辑。 |
| [L133](../src/apps/remote/InProcessSessionEngine.h#L133) | `JoinRoom` | 声明 | `SessionCommandResult JoinRoom(const std::string& roomId) override` | 实现 join room 对应的业务或工具逻辑。 |
| [L134](../src/apps/remote/InProcessSessionEngine.h#L134) | `QueryRoomAvailability` | 声明 | `SessionCommandResult QueryRoomAvailability( const std::vector<std::string>& roomIds) override` | 发起请求或查询 query room availability 相关逻辑。 |
| [L136](../src/apps/remote/InProcessSessionEngine.h#L136) | `RespondToRoomJoin` | 声明 | `SessionCommandResult RespondToRoomJoin( const std::string& requestId, bool accepted) override` | 处理并回复 respond to room join 相关逻辑。 |
| [L139](../src/apps/remote/InProcessSessionEngine.h#L139) | `SetRoomCapacity` | 声明 | `SessionCommandResult SetRoomCapacity( std::uint32_t capacity) override` | 更新或应用 set room capacity 相关逻辑。 |
| [L141](../src/apps/remote/InProcessSessionEngine.h#L141) | `LeaveRoom` | 声明 | `SessionCommandResult LeaveRoom() override` | 实现 leave room 对应的业务或工具逻辑。 |
| [L144](../src/apps/remote/InProcessSessionEngine.h#L144) | `ExitRoomAfterRecoveryFailure` | 声明 | `SessionCommandResult ExitRoomAfterRecoveryFailure()` | Used after an unrecoverable P2P failure. Local media/session state is torn down immediately; a server leave is sent now or after WSS returns. |
| [L145](../src/apps/remote/InProcessSessionEngine.h#L145) | `RefreshLocalDisplays` | 声明 | `SessionCommandResult RefreshLocalDisplays() override` | 刷新 refresh local displays 相关逻辑。 |
| [L146](../src/apps/remote/InProcessSessionEngine.h#L146) | `SelectRoomScreenShareDisplay` | 声明 | `SessionCommandResult SelectRoomScreenShareDisplay( const std::string& stableDisplayKey) override` | 查询并返回 select room screen share display 相关逻辑。 |
| [L148](../src/apps/remote/InProcessSessionEngine.h#L148) | `StartRoomScreenShare` | 声明 | `SessionCommandResult StartRoomScreenShare() override` | 启动 start room screen share 相关逻辑。 |
| [L149](../src/apps/remote/InProcessSessionEngine.h#L149) | `StopRoomScreenShare` | 声明 | `SessionCommandResult StopRoomScreenShare() override` | 停止 stop room screen share 相关逻辑。 |
| [L150](../src/apps/remote/InProcessSessionEngine.h#L150) | `RespondToRoomScreenShareSwitch` | 声明 | `SessionCommandResult RespondToRoomScreenShareSwitch( const std::string& requestId, bool accepted) override` | 处理并回复 respond to room screen share switch 相关逻辑。 |
| [L153](../src/apps/remote/InProcessSessionEngine.h#L153) | `CancelRoomScreenShareSwitch` | 声明 | `SessionCommandResult CancelRoomScreenShareSwitch() override` | 判断 cancel room screen share switch 相关逻辑。 |
| [L154](../src/apps/remote/InProcessSessionEngine.h#L154) | `RequestRoomControl` | 声明 | `SessionCommandResult RequestRoomControl() override` | 发起请求或查询 request room control 相关逻辑。 |
| [L155](../src/apps/remote/InProcessSessionEngine.h#L155) | `RespondToRoomControl` | 声明 | `SessionCommandResult RespondToRoomControl( const std::string& requestId, bool accepted) override` | 处理并回复 respond to room control 相关逻辑。 |
| [L158](../src/apps/remote/InProcessSessionEngine.h#L158) | `ReleaseRoomControl` | 声明 | `SessionCommandResult ReleaseRoomControl() override` | 释放或取消 release room control 相关逻辑。 |
| [L159](../src/apps/remote/InProcessSessionEngine.h#L159) | `RequestRoomMemberScreenShare` | 声明 | `SessionCommandResult RequestRoomMemberScreenShare( const std::string& peerDeviceId) override` | 发起请求或查询 request room member screen share 相关逻辑。 |
| [L161](../src/apps/remote/InProcessSessionEngine.h#L161) | `RespondToRoomMemberScreenShare` | 声明 | `SessionCommandResult RespondToRoomMemberScreenShare( const std::string& requesterDeviceId, std::uint64_t sequence, bool accepted) override` | 处理并回复 respond to room member screen share 相关逻辑。 |
| [L165](../src/apps/remote/InProcessSessionEngine.h#L165) | `RequestRoomMemberMicrophoneMute` | 声明 | `SessionCommandResult RequestRoomMemberMicrophoneMute( const std::string& peerDeviceId) override` | 发起请求或查询 request room member microphone mute 相关逻辑。 |
| [L167](../src/apps/remote/InProcessSessionEngine.h#L167) | `RequestRemoteRoomScreenShareStop` | 声明 | `SessionCommandResult RequestRemoteRoomScreenShareStop( const std::string& peerDeviceId, std::uint64_t screenShareEpoch) override` | 发起请求或查询 request remote room screen share stop 相关逻辑。 |
| [L170](../src/apps/remote/InProcessSessionEngine.h#L170) | `SendRoomInput` | 声明 | `SessionCommandResult SendRoomInput( const RemoteInputEvent& event) override` | 发送或发布 send room input 相关逻辑。 |
| [L172](../src/apps/remote/InProcessSessionEngine.h#L172) | `SendDirectInput` | 声明 | `SessionCommandResult SendDirectInput( const RemoteInputEvent& event)` | 发送或发布 send direct input 相关逻辑。 |
| [L174](../src/apps/remote/InProcessSessionEngine.h#L174) | `SendRemoteInput` | 声明 | `SessionCommandResult SendRemoteInput( const RemoteInputEvent& event)` | 发送或发布 send remote input 相关逻辑。 |
| [L176](../src/apps/remote/InProcessSessionEngine.h#L176) | `SetRoomScreenFrameRate` | 声明 | `SessionCommandResult SetRoomScreenFrameRate( const std::string& pairId, std::uint32_t framesPerSecond) override` | 更新或应用 set room screen frame rate 相关逻辑。 |
| [L179](../src/apps/remote/InProcessSessionEngine.h#L179) | `SetRoomScreenStreamPreference` | 声明 | `SessionCommandResult SetRoomScreenStreamPreference( const std::string& pairId, const ScreenStreamPreferenceRequest& preference) override` | 更新或应用 set room screen stream preference 相关逻辑。 |
| [L182](../src/apps/remote/InProcessSessionEngine.h#L182) | `RequestRemoteSharedDisplaySwitch` | 声明 | `SessionCommandResult RequestRemoteSharedDisplaySwitch( const std::string& pairId, const std::string& stableDisplayKey) override` | 发起请求或查询 request remote shared display switch 相关逻辑。 |
| [L185](../src/apps/remote/InProcessSessionEngine.h#L185) | `SendRoomFileMessage` | 声明 | `SessionCommandResult SendRoomFileMessage( const std::string& peerDeviceId, const FileTransferMessage& message) override` | 发送或发布 send room file message 相关逻辑。 |
| [L188](../src/apps/remote/InProcessSessionEngine.h#L188) | `SendRoomClipboardMessage` | 声明 | `SessionCommandResult SendRoomClipboardMessage( const std::string& peerDeviceId, const std::string& clipboardSessionId, const ClipboardMessage& message) override` | 发送或发布 send room clipboard message 相关逻辑。 |
| [L192](../src/apps/remote/InProcessSessionEngine.h#L192) | `SendRemoteFileMessage` | 声明 | `SessionCommandResult SendRemoteFileMessage( const std::string& peerDeviceId, const FileTransferMessage& message)` | 发送或发布 send remote file message 相关逻辑。 |
| [L195](../src/apps/remote/InProcessSessionEngine.h#L195) | `SendRemoteClipboardMessage` | 声明 | `SessionCommandResult SendRemoteClipboardMessage( const std::string& peerDeviceId, const std::string& clipboardSessionId, const ClipboardMessage& message)` | 发送或发布 send remote clipboard message 相关逻辑。 |
| [L199](../src/apps/remote/InProcessSessionEngine.h#L199) | `SetDirectScreenStreamPreference` | 声明 | `SessionCommandResult SetDirectScreenStreamPreference( const ScreenStreamPreferenceRequest& preference)` | 更新或应用 set direct screen stream preference 相关逻辑。 |
| [L201](../src/apps/remote/InProcessSessionEngine.h#L201) | `RequestDirectSharedDisplaySwitch` | 声明 | `SessionCommandResult RequestDirectSharedDisplaySwitch( const std::string& stableDisplayKey)` | 发起请求或查询 request direct shared display switch 相关逻辑。 |
| [L203](../src/apps/remote/InProcessSessionEngine.h#L203) | `SetLocalCameraEnabled` | 声明 | `SessionCommandResult SetLocalCameraEnabled(bool enabled) override` | 更新或应用 set local camera enabled 相关逻辑。 |
| [L204](../src/apps/remote/InProcessSessionEngine.h#L204) | `SetLocalMicrophoneEnabled` | 声明 | `SessionCommandResult SetLocalMicrophoneEnabled(bool enabled) override` | 更新或应用 set local microphone enabled 相关逻辑。 |
| [L205](../src/apps/remote/InProcessSessionEngine.h#L205) | `SetRemoteAudioPlaybackMuted` | 声明 | `SessionCommandResult SetRemoteAudioPlaybackMuted(bool muted)` | 更新或应用 set remote audio playback muted 相关逻辑。 |
| [L206](../src/apps/remote/InProcessSessionEngine.h#L206) | `SetRoomAudioPlaybackMuted` | 声明 | `SessionCommandResult SetRoomAudioPlaybackMuted(bool muted) override` | 更新或应用 set room audio playback muted 相关逻辑。 |
| [L207](../src/apps/remote/InProcessSessionEngine.h#L207) | `RefreshLocalMediaDevices` | 声明 | `SessionCommandResult RefreshLocalMediaDevices() override` | 刷新 refresh local media devices 相关逻辑。 |
| [L208](../src/apps/remote/InProcessSessionEngine.h#L208) | `SelectLocalCameraDevice` | 声明 | `SessionCommandResult SelectLocalCameraDevice( const std::string& deviceId) override` | 查询并返回 select local camera device 相关逻辑。 |
| [L210](../src/apps/remote/InProcessSessionEngine.h#L210) | `SelectLocalMicrophoneDevice` | 声明 | `SessionCommandResult SelectLocalMicrophoneDevice( const std::string& deviceId) override` | 查询并返回 select local microphone device 相关逻辑。 |
| [L212](../src/apps/remote/InProcessSessionEngine.h#L212) | `SelectLocalSpeakerDevice` | 声明 | `SessionCommandResult SelectLocalSpeakerDevice( const std::string& deviceId) override` | 查询并返回 select local speaker device 相关逻辑。 |
| [L218](../src/apps/remote/InProcessSessionEngine.h#L218) | `SetRoomVideoSource` | 声明 | `SessionCommandResult SetRoomVideoSource( const std::string& slot, webrtc::scoped_refptr<webrtc::VideoTrackSourceInterface> source, const std::string& trackId, std::optional<std::string> expectedRoomId = std::nullopt, ...` | Media adapters use these concrete-engine hooks. UI code continues to depend only on ISessionEngine; desktop/camera capture sources and Qt render sinks remain outside the control... |
| [L225](../src/apps/remote/InProcessSessionEngine.h#L225) | `ClearRoomVideoSource` | 声明 | `SessionCommandResult ClearRoomVideoSource( const std::string& slot)` | 重置或移除 clear room video source 相关逻辑。 |
| [L227](../src/apps/remote/InProcessSessionEngine.h#L227) | `SetRoomVideoSlotSendingActive` | 声明 | `SessionCommandResult SetRoomVideoSlotSendingActive( const std::string& slot, bool active)` | 更新或应用 set room video slot sending active 相关逻辑。 |
| [L230](../src/apps/remote/InProcessSessionEngine.h#L230) | `SetRoomRemoteVideoSink` | 声明 | `SessionCommandResult SetRoomRemoteVideoSink( const std::string& pairId, const std::string& slot, webrtc::VideoSinkInterface<webrtc::VideoFrame>* sink)` | 更新或应用 set room remote video sink 相关逻辑。 |
| [L234](../src/apps/remote/InProcessSessionEngine.h#L234) | `SetDirectRemoteVideoSink` | 声明 | `SessionCommandResult SetDirectRemoteVideoSink( webrtc::VideoSinkInterface<webrtc::VideoFrame>* sink)` | 更新或应用 set direct remote video sink 相关逻辑。 |
| [L236](../src/apps/remote/InProcessSessionEngine.h#L236) | `NotifyRoomScreenFirstFramePresented` | 声明 | `SessionCommandResult NotifyRoomScreenFirstFramePresented( const std::string& pairId, std::uint64_t screenShareGeneration, std::uint32_t startupElapsedMs)` | 通知或报告 notify room screen first frame presented 相关逻辑。 |
| [L240](../src/apps/remote/InProcessSessionEngine.h#L240) | `SetLocalCameraPreviewSink` | 声明 | `void SetLocalCameraPreviewSink( webrtc::VideoSinkInterface<webrtc::VideoFrame>* sink)` | 更新或应用 set local camera preview sink 相关逻辑。 |
| [L242](../src/apps/remote/InProcessSessionEngine.h#L242) | `SetRemoteInputSink` | 声明 | `void SetRemoteInputSink(IRemoteInputSink* sink)` | 更新或应用 set remote input sink 相关逻辑。 |
| [L243](../src/apps/remote/InProcessSessionEngine.h#L243) | `SetRemoteFileTransferSink` | 声明 | `void SetRemoteFileTransferSink(IFileTransferSink* sink)` | 更新或应用 set remote file transfer sink 相关逻辑。 |
| [L244](../src/apps/remote/InProcessSessionEngine.h#L244) | `SetRemoteClipboardSink` | 声明 | `void SetRemoteClipboardSink(IClipboardSink* sink)` | 更新或应用 set remote clipboard sink 相关逻辑。 |
| [L248](../src/apps/remote/InProcessSessionEngine.h#L248) | `SetRemoteCursorCallback` | 声明 | `void SetRemoteCursorCallback(RemoteCursorCallback callback)` | 更新或应用 set remote cursor callback 相关逻辑。 |
| [L249](../src/apps/remote/InProcessSessionEngine.h#L249) | `SetPreferredHardwareDecoderName` | 声明 | `void SetPreferredHardwareDecoderName(std::string name)` | 更新或应用 set preferred hardware decoder name 相关逻辑。 |
| [L250](../src/apps/remote/InProcessSessionEngine.h#L250) | `ApplyVideoPipelinePreferences` | 声明 | `SessionCommandResult ApplyVideoPipelinePreferences( DesktopCaptureImplementation desktopCaptureImplementation, VideoEncoderPreference videoEncoderPreference, FfmpegX264Preset quality, FfmpegHardwareBackend ffmpegHardw...` | 更新或应用 apply video pipeline preferences 相关逻辑。 |
| [L261](../src/apps/remote/InProcessSessionEngine.h#L261) | `OnSignalingStateChanged` | 声明 | `void OnSignalingStateChanged(SignalingConnectionState state) override` | 接收并处理 on signaling state changed 相关逻辑。 |
| [L262](../src/apps/remote/InProcessSessionEngine.h#L262) | `OnDeviceRegistered` | 声明 | `void OnDeviceRegistered(const std::string& deviceId) override` | 接收并处理 on device registered 相关逻辑。 |
| [L263](../src/apps/remote/InProcessSessionEngine.h#L263) | `OnIncomingSessionRequest` | 声明 | `void OnIncomingSessionRequest( const IncomingSessionRequest& request) override` | 接收并处理 on incoming session request 相关逻辑。 |
| [L265](../src/apps/remote/InProcessSessionEngine.h#L265) | `OnSessionResponse` | 声明 | `void OnSessionResponse( const SignalingSessionResponse& response) override` | 接收并处理 on session response 相关逻辑。 |
| [L267](../src/apps/remote/InProcessSessionEngine.h#L267) | `OnSessionPending` | 声明 | `void OnSessionPending( const SignalingSessionPending& pending) override` | 接收并处理 on session pending 相关逻辑。 |
| [L269](../src/apps/remote/InProcessSessionEngine.h#L269) | `OnSessionReady` | 声明 | `void OnSessionReady(const SignalingSessionReady& ready) override` | 接收并处理 on session ready 相关逻辑。 |
| [L270](../src/apps/remote/InProcessSessionEngine.h#L270) | `OnSessionSuspended` | 声明 | `void OnSessionSuspended( const SignalingSessionSuspended& suspended) override` | 接收并处理 on session suspended 相关逻辑。 |
| [L272](../src/apps/remote/InProcessSessionEngine.h#L272) | `OnSessionResumed` | 声明 | `void OnSessionResumed( const SignalingSessionResumed& resumed) override` | 接收并处理 on session resumed 相关逻辑。 |
| [L274](../src/apps/remote/InProcessSessionEngine.h#L274) | `OnSessionEnded` | 声明 | `void OnSessionEnded(const SignalingSessionEnded& ended) override` | 接收并处理 on session ended 相关逻辑。 |
| [L275](../src/apps/remote/InProcessSessionEngine.h#L275) | `OnRemoteDescription` | 声明 | `void OnRemoteDescription( const SignalingSessionDescription& description) override` | 接收并处理 on remote description 相关逻辑。 |
| [L277](../src/apps/remote/InProcessSessionEngine.h#L277) | `OnRemoteIceCandidate` | 声明 | `void OnRemoteIceCandidate( const SignalingIceCandidate& candidate) override` | 接收并处理 on remote ice candidate 相关逻辑。 |
| [L279](../src/apps/remote/InProcessSessionEngine.h#L279) | `OnIceRestartRequested` | 声明 | `void OnIceRestartRequested( const SignalingIceRestartRequest& request) override` | 接收并处理 on ice restart requested 相关逻辑。 |
| [L281](../src/apps/remote/InProcessSessionEngine.h#L281) | `OnIceRestartCancelled` | 声明 | `void OnIceRestartCancelled( const SignalingIceRestartCancel& cancel) override` | 接收并处理 on ice restart cancelled 相关逻辑。 |
| [L283](../src/apps/remote/InProcessSessionEngine.h#L283) | `OnHeartbeatAcknowledged` | 声明 | `void OnHeartbeatAcknowledged(std::uint32_t roundTripMs) override` | 接收并处理 on heartbeat acknowledged 相关逻辑。 |
| [L284](../src/apps/remote/InProcessSessionEngine.h#L284) | `OnSignalingError` | 声明 | `void OnSignalingError(const std::string& code, const std::string& message) override` | 接收并处理 on signaling error 相关逻辑。 |
| [L286](../src/apps/remote/InProcessSessionEngine.h#L286) | `OnAccountDeletionResult` | 声明 | `void OnAccountDeletionResult( const SignalingAccountDeletionResult& result) override` | 接收并处理 on account deletion result 相关逻辑。 |
| [L288](../src/apps/remote/InProcessSessionEngine.h#L288) | `OnOwnedDevicesChanged` | 声明 | `void OnOwnedDevicesChanged( const SignalingOwnedDevicesSnapshot& snapshot) override` | 接收并处理 on owned devices changed 相关逻辑。 |
| [L290](../src/apps/remote/InProcessSessionEngine.h#L290) | `OnRoomReady` | 声明 | `void OnRoomReady(const SignalingRoomReady& ready) override` | 接收并处理 on room ready 相关逻辑。 |
| [L291](../src/apps/remote/InProcessSessionEngine.h#L291) | `OnRoomState` | 声明 | `void OnRoomState(const RoomSnapshot& room) override` | 接收并处理 on room state 相关逻辑。 |
| [L292](../src/apps/remote/InProcessSessionEngine.h#L292) | `OnRoomJoinPending` | 声明 | `void OnRoomJoinPending( const SignalingRoomJoinPending& pending) override` | 接收并处理 on room join pending 相关逻辑。 |
| [L294](../src/apps/remote/InProcessSessionEngine.h#L294) | `OnRoomJoinRequested` | 声明 | `void OnRoomJoinRequested(const RoomJoinRequest& request) override` | 接收并处理 on room join requested 相关逻辑。 |
| [L295](../src/apps/remote/InProcessSessionEngine.h#L295) | `OnRoomJoinResult` | 声明 | `void OnRoomJoinResult( const SignalingRoomJoinResult& result) override` | 接收并处理 on room join result 相关逻辑。 |
| [L297](../src/apps/remote/InProcessSessionEngine.h#L297) | `OnRoomAvailabilityResult` | 声明 | `void OnRoomAvailabilityResult( const SignalingRoomAvailabilityResult& result) override` | 接收并处理 on room availability result 相关逻辑。 |
| [L299](../src/apps/remote/InProcessSessionEngine.h#L299) | `OnRoomClosed` | 声明 | `void OnRoomClosed(const SignalingRoomClosed& closed) override` | 接收并处理 on room closed 相关逻辑。 |
| [L300](../src/apps/remote/InProcessSessionEngine.h#L300) | `OnRoomPairReady` | 声明 | `void OnRoomPairReady(const SignalingRoomPairReady& ready) override` | 接收并处理 on room pair ready 相关逻辑。 |
| [L301](../src/apps/remote/InProcessSessionEngine.h#L301) | `OnRoomPairClosed` | 声明 | `void OnRoomPairClosed(const SignalingRoomPairClosed& closed) override` | 接收并处理 on room pair closed 相关逻辑。 |
| [L302](../src/apps/remote/InProcessSessionEngine.h#L302) | `OnRoomScreenShareGranted` | 声明 | `void OnRoomScreenShareGranted( const SignalingRoomScreenShareGranted& granted) override` | 接收并处理 on room screen share granted 相关逻辑。 |
| [L304](../src/apps/remote/InProcessSessionEngine.h#L304) | `OnRoomScreenShareSwitchPending` | 声明 | `void OnRoomScreenShareSwitchPending( const SignalingRoomScreenShareSwitchPending& pending) override` | 接收并处理 on room screen share switch pending 相关逻辑。 |
| [L306](../src/apps/remote/InProcessSessionEngine.h#L306) | `OnRoomScreenShareSwitchRequested` | 声明 | `void OnRoomScreenShareSwitchRequested( const RoomScreenShareSwitchRequest& request) override` | 接收并处理 on room screen share switch requested 相关逻辑。 |
| [L308](../src/apps/remote/InProcessSessionEngine.h#L308) | `OnRoomScreenShareSwitchResult` | 声明 | `void OnRoomScreenShareSwitchResult( const SignalingRoomScreenShareSwitchResult& result) override` | 接收并处理 on room screen share switch result 相关逻辑。 |
| [L310](../src/apps/remote/InProcessSessionEngine.h#L310) | `OnRoomControlRequested` | 声明 | `void OnRoomControlRequested( const RoomControlRequest& request) override` | 接收并处理 on room control requested 相关逻辑。 |
| [L312](../src/apps/remote/InProcessSessionEngine.h#L312) | `OnRoomControlResult` | 声明 | `void OnRoomControlResult( const SignalingRoomControlResult& result) override` | 接收并处理 on room control result 相关逻辑。 |
| [L314](../src/apps/remote/InProcessSessionEngine.h#L314) | `OnRoomControlGranted` | 声明 | `void OnRoomControlGranted( const SignalingRoomControlGranted& granted) override` | 接收并处理 on room control granted 相关逻辑。 |
| [L316](../src/apps/remote/InProcessSessionEngine.h#L316) | `OnRoomControlRevoked` | 声明 | `void OnRoomControlRevoked( const SignalingRoomControlRevoked& revoked) override` | 接收并处理 on room control revoked 相关逻辑。 |
| [L319](../src/apps/remote/InProcessSessionEngine.h#L319) | `SendDescription` | 声明 | `bool SendDescription( const SessionDescription& description) override` | 发送或发布 send description 相关逻辑。 |
| [L321](../src/apps/remote/InProcessSessionEngine.h#L321) | `SendIceCandidate` | 声明 | `bool SendIceCandidate(const IceCandidate& candidate) override` | 发送或发布 send ice candidate 相关逻辑。 |
| [L322](../src/apps/remote/InProcessSessionEngine.h#L322) | `RequestIceRestart` | 声明 | `bool RequestIceRestart(std::uint64_t observedGeneration, std::uint64_t requestSequence) override` | 发起请求或查询 request ice restart 相关逻辑。 |
| [L324](../src/apps/remote/InProcessSessionEngine.h#L324) | `CancelIceRestart` | 声明 | `bool CancelIceRestart(std::uint64_t observedGeneration, std::uint64_t requestSequence) override` | 判断 cancel ice restart 相关逻辑。 |
| [L326](../src/apps/remote/InProcessSessionEngine.h#L326) | `OnControllerSnapshot` | 声明 | `void OnControllerSnapshot( const SessionControllerSnapshot& snapshot) override` | 接收并处理 on controller snapshot 相关逻辑。 |
| [L328](../src/apps/remote/InProcessSessionEngine.h#L328) | `OnDataChannelStateChanged` | 声明 | `void OnDataChannelStateChanged( const DataChannelInfo& channel) override` | 接收并处理 on data channel state changed 相关逻辑。 |
| [L330](../src/apps/remote/InProcessSessionEngine.h#L330) | `OnDataMessage` | 声明 | `void OnDataMessage(const std::string& label, std::span<const std::uint8_t> payload, bool binary) override` | 接收并处理 on data message 相关逻辑。 |
| [L333](../src/apps/remote/InProcessSessionEngine.h#L333) | `OnRemoteTrackAdded` | 声明 | `void OnRemoteTrackAdded(const RemoteTrackInfo& track) override` | 接收并处理 on remote track added 相关逻辑。 |
| [L335](../src/apps/remote/InProcessSessionEngine.h#L335) | `RequireReady` | 声明 | `SessionCommandResult RequireReady(const char* operation) const` | 实现 require ready 对应的业务或工具逻辑。 |
| [L336](../src/apps/remote/InProcessSessionEngine.h#L336) | `SignalingIsOnline` | 声明 | `bool SignalingIsOnline() const` | 实现 signaling is online 对应的业务或工具逻辑。 |
| [L337](../src/apps/remote/InProcessSessionEngine.h#L337) | `DisposeClosedSession` | 声明 | `void DisposeClosedSession()` | 关闭并清理 dispose closed session 相关逻辑。 |
| [L338](../src/apps/remote/InProcessSessionEngine.h#L338) | `ResetSessionStateLocked` | 声明 | `void ResetSessionStateLocked()` | 重置或移除 reset session state locked 相关逻辑。 |
| [L339](../src/apps/remote/InProcessSessionEngine.h#L339) | `ResetRoomStateLocked` | 声明 | `void ResetRoomStateLocked()` | 重置或移除 reset room state locked 相关逻辑。 |
| [L340](../src/apps/remote/InProcessSessionEngine.h#L340) | `StopLocalDesktopCapture` | 声明 | `void StopLocalDesktopCapture()` | 停止 stop local desktop capture 相关逻辑。 |
| [L341](../src/apps/remote/InProcessSessionEngine.h#L341) | `StopDirectDesktopCapture` | 声明 | `void StopDirectDesktopCapture()` | 停止 stop direct desktop capture 相关逻辑。 |
| [L342](../src/apps/remote/InProcessSessionEngine.h#L342) | `StartRemoteCursorPublishing` | 声明 | `void StartRemoteCursorPublishing( const DisplayDescriptor& display, std::uint64_t layoutVersion)` | 启动 start remote cursor publishing 相关逻辑。 |
| [L345](../src/apps/remote/InProcessSessionEngine.h#L345) | `StopRemoteCursorPublishing` | 声明 | `void StopRemoteCursorPublishing()` | 停止 stop remote cursor publishing 相关逻辑。 |
| [L346](../src/apps/remote/InProcessSessionEngine.h#L346) | `OnLocalCursorObservation` | 声明 | `void OnLocalCursorObservation(WindowsCursorObservation observation)` | 接收并处理 on local cursor observation 相关逻辑。 |
| [L347](../src/apps/remote/InProcessSessionEngine.h#L347) | `RepublishRemoteCursor` | 声明 | `void RepublishRemoteCursor()` | 实现 republish remote cursor 对应的业务或工具逻辑。 |
| [L348](../src/apps/remote/InProcessSessionEngine.h#L348) | `DispatchRemoteCursorData` | 声明 | `bool DispatchRemoteCursorData( const std::string& pairId, const std::string& label, std::span<const std::uint8_t> payload)` | 接收并处理 dispatch remote cursor data 相关逻辑。 |
| [L352](../src/apps/remote/InProcessSessionEngine.h#L352) | `BroadcastDirectSharedDisplayLayout` | 声明 | `void BroadcastDirectSharedDisplayLayout()` | 实现 broadcast direct shared display layout 对应的业务或工具逻辑。 |
| [L353](../src/apps/remote/InProcessSessionEngine.h#L353) | `RequestDirectSharedDisplayLayout` | 声明 | `void RequestDirectSharedDisplayLayout()` | 发起请求或查询 request direct shared display layout 相关逻辑。 |
| [L354](../src/apps/remote/InProcessSessionEngine.h#L354) | `SendDirectFileMessage` | 声明 | `SessionCommandResult SendDirectFileMessage( const std::string& peerDeviceId, const FileTransferMessage& message)` | 发送或发布 send direct file message 相关逻辑。 |
| [L357](../src/apps/remote/InProcessSessionEngine.h#L357) | `SendDirectClipboardMessage` | 声明 | `SessionCommandResult SendDirectClipboardMessage( const std::string& peerDeviceId, const std::string& clipboardSessionId, const ClipboardMessage& message)` | 发送或发布 send direct clipboard message 相关逻辑。 |
| [L361](../src/apps/remote/InProcessSessionEngine.h#L361) | `DispatchDirectAuxiliaryData` | 声明 | `bool DispatchDirectAuxiliaryData( const std::string& label, std::span<const std::uint8_t> payload)` | 接收并处理 dispatch direct auxiliary data 相关逻辑。 |
| [L364](../src/apps/remote/InProcessSessionEngine.h#L364) | `DispatchDirectScreenData` | 声明 | `bool DispatchDirectScreenData( const std::string& label, std::span<const std::uint8_t> payload)` | 接收并处理 dispatch direct screen data 相关逻辑。 |
| [L367](../src/apps/remote/InProcessSessionEngine.h#L367) | `SwitchLocalDirectDisplay` | 声明 | `SessionCommandResult SwitchLocalDirectDisplay( const std::string& stableDisplayKey)` | 实现 switch local direct display 对应的业务或工具逻辑。 |
| [L369](../src/apps/remote/InProcessSessionEngine.h#L369) | `BroadcastDirectSharedDisplayCatalog` | 声明 | `void BroadcastDirectSharedDisplayCatalog()` | 实现 broadcast direct shared display catalog 对应的业务或工具逻辑。 |
| [L370](../src/apps/remote/InProcessSessionEngine.h#L370) | `BroadcastSharedDisplayLayout` | 声明 | `void BroadcastSharedDisplayLayout()` | 实现 broadcast shared display layout 对应的业务或工具逻辑。 |
| [L371](../src/apps/remote/InProcessSessionEngine.h#L371) | `SendRoomPairDescription` | 声明 | `bool SendRoomPairDescription( const std::string& pairId, const SessionDescription& description)` | 发送或发布 send room pair description 相关逻辑。 |
| [L374](../src/apps/remote/InProcessSessionEngine.h#L374) | `SendRoomPairIceCandidate` | 声明 | `bool SendRoomPairIceCandidate( const std::string& pairId, const IceCandidate& candidate)` | 发送或发布 send room pair ice candidate 相关逻辑。 |
| [L377](../src/apps/remote/InProcessSessionEngine.h#L377) | `SendRoomPairIceRestartRequest` | 声明 | `bool SendRoomPairIceRestartRequest( const std::string& pairId, std::uint64_t observedGeneration, std::uint64_t requestSequence)` | 发送或发布 send room pair ice restart request 相关逻辑。 |
| [L381](../src/apps/remote/InProcessSessionEngine.h#L381) | `SendRoomPairIceRestartCancel` | 声明 | `bool SendRoomPairIceRestartCancel( const std::string& pairId, std::uint64_t observedGeneration, std::uint64_t requestSequence)` | 发送或发布 send room pair ice restart cancel 相关逻辑。 |
| [L385](../src/apps/remote/InProcessSessionEngine.h#L385) | `OnRoomPairControllerSnapshot` | 声明 | `void OnRoomPairControllerSnapshot( const std::string& pairId, const SessionControllerSnapshot& snapshot)` | 接收并处理 on room pair controller snapshot 相关逻辑。 |
| [L388](../src/apps/remote/InProcessSessionEngine.h#L388) | `OnRoomPairDataChannelStateChanged` | 声明 | `void OnRoomPairDataChannelStateChanged( const std::string& pairId, const DataChannelInfo& channel)` | 接收并处理 on room pair data channel state changed 相关逻辑。 |
| [L391](../src/apps/remote/InProcessSessionEngine.h#L391) | `OnRoomPairDataMessage` | 声明 | `void OnRoomPairDataMessage( const std::string& pairId, const std::string& label, std::span<const std::uint8_t> payload, bool binary)` | 接收并处理 on room pair data message 相关逻辑。 |
| [L396](../src/apps/remote/InProcessSessionEngine.h#L396) | `OnRoomPairRemoteTrackAdded` | 声明 | `void OnRoomPairRemoteTrackAdded( const std::string& pairId, const RemoteTrackInfo& track)` | 接收并处理 on room pair remote track added 相关逻辑。 |
| [L399](../src/apps/remote/InProcessSessionEngine.h#L399) | `PrepareRoomPairAnswer` | 声明 | `std::optional<OperationError> PrepareRoomPairAnswer( const std::string& pairId)` | 实现 prepare room pair answer 对应的业务或工具逻辑。 |
| [L401](../src/apps/remote/InProcessSessionEngine.h#L401) | `PrepareRoomPairMedia` | 声明 | `std::optional<OperationError> PrepareRoomPairMedia( const std::string& pairId, bool bindNegotiatedSlots, bool preparationAlreadyClaimed = false)` | 实现 prepare room pair media 对应的业务或工具逻辑。 |
| [L405](../src/apps/remote/InProcessSessionEngine.h#L405) | `PrepareDirectMedia` | 声明 | `std::optional<OperationError> PrepareDirectMedia( bool bindNegotiatedSlots)` | 实现 prepare direct media 对应的业务或工具逻辑。 |
| [L407](../src/apps/remote/InProcessSessionEngine.h#L407) | `StopDirectMicrophoneCapture` | 声明 | `void StopDirectMicrophoneCapture()` | 停止 stop direct microphone capture 相关逻辑。 |
| [L408](../src/apps/remote/InProcessSessionEngine.h#L408) | `PublishSnapshot` | 声明 | `void PublishSnapshot()` | 发送或发布 publish snapshot 相关逻辑。 |
| [L409](../src/apps/remote/InProcessSessionEngine.h#L409) | `StartStatsPolling` | 声明 | `void StartStatsPolling()` | 启动 start stats polling 相关逻辑。 |
| [L410](../src/apps/remote/InProcessSessionEngine.h#L410) | `StopStatsPolling` | 声明 | `void StopStatsPolling()` | 停止 stop stats polling 相关逻辑。 |
| [L414](../src/apps/remote/InProcessSessionEngine.h#L414) | `RetireRoomPair` | 声明 | `void RetireRoomPair(std::shared_ptr<RoomPairRuntime> pair)` | 实现 retire room pair 对应的业务或工具逻辑。 |
| [L415](../src/apps/remote/InProcessSessionEngine.h#L415) | `ApplyLocalScreenFrameRate` | 声明 | `SessionCommandResult ApplyLocalScreenFrameRate( std::uint32_t framesPerSecond)` | 更新或应用 apply local screen frame rate 相关逻辑。 |
| [L417](../src/apps/remote/InProcessSessionEngine.h#L417) | `SwitchLocalSharedDisplay` | 声明 | `SessionCommandResult SwitchLocalSharedDisplay( const std::string& stableDisplayKey)` | 实现 switch local shared display 对应的业务或工具逻辑。 |
| [L419](../src/apps/remote/InProcessSessionEngine.h#L419) | `BroadcastSharedDisplayCatalog` | 声明 | `void BroadcastSharedDisplayCatalog()` | 实现 broadcast shared display catalog 对应的业务或工具逻辑。 |
| [L420](../src/apps/remote/InProcessSessionEngine.h#L420) | `StartClipboardWarmup` | 声明 | `void StartClipboardWarmup( const std::shared_ptr<RoomPairRuntime>& pair)` | 启动 start clipboard warmup 相关逻辑。 |
| [L422](../src/apps/remote/InProcessSessionEngine.h#L422) | `SendRoomMemberAction` | 声明 | `SessionCommandResult SendRoomMemberAction( const std::string& peerDeviceId, RoomMemberAction action, std::uint64_t screenShareEpoch = 0)` | 发送或发布 send room member action 相关逻辑。 |
| [L426](../src/apps/remote/InProcessSessionEngine.h#L426) | `SendRoomMemberActionResponse` | 声明 | `SendResult SendRoomMemberActionResponse( const std::shared_ptr<RoomPairRuntime>& pair, const RoomMemberActionEnvelope& request, bool accepted, const std::string& error)` | 发送或发布 send room member action response 相关逻辑。 |

## `src/apps/remote/InProcessSessionEngine.LeaseCallbacks.inc`

[打开源码](../src/apps/remote/InProcessSessionEngine.LeaseCallbacks.inc) · **文件作用：** `InProcessSessionEngine` 的实现切片，集中实现 lease callbacks 相关逻辑；成员状态仍定义在所属头文件中。

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L4](../src/apps/remote/InProcessSessionEngine.LeaseCallbacks.inc#L4) | `InProcessSessionEngine::OnRoomScreenShareGranted` | 定义 | `void InProcessSessionEngine::OnRoomScreenShareGranted( const SignalingRoomScreenShareGranted& granted)` | SPDX-License-Identifier: GPL-3.0-only Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd) |
| [L251](../src/apps/remote/InProcessSessionEngine.LeaseCallbacks.inc#L251) | `InProcessSessionEngine::OnRoomScreenShareSwitchPending` | 定义 | `void InProcessSessionEngine::OnRoomScreenShareSwitchPending( const SignalingRoomScreenShareSwitchPending& pending)` | 接收并处理 on room screen share switch pending 相关逻辑。 |
| [L272](../src/apps/remote/InProcessSessionEngine.LeaseCallbacks.inc#L272) | `InProcessSessionEngine::OnRoomScreenShareSwitchRequested` | 定义 | `void InProcessSessionEngine::OnRoomScreenShareSwitchRequested( const RoomScreenShareSwitchRequest& request)` | 接收并处理 on room screen share switch requested 相关逻辑。 |
| [L301](../src/apps/remote/InProcessSessionEngine.LeaseCallbacks.inc#L301) | `InProcessSessionEngine::OnRoomScreenShareSwitchResult` | 定义 | `void InProcessSessionEngine::OnRoomScreenShareSwitchResult( const SignalingRoomScreenShareSwitchResult& result)` | 接收并处理 on room screen share switch result 相关逻辑。 |
| [L345](../src/apps/remote/InProcessSessionEngine.LeaseCallbacks.inc#L345) | `InProcessSessionEngine::OnRoomControlRequested` | 定义 | `void InProcessSessionEngine::OnRoomControlRequested( const RoomControlRequest& request)` | 接收并处理 on room control requested 相关逻辑。 |
| [L371](../src/apps/remote/InProcessSessionEngine.LeaseCallbacks.inc#L371) | `InProcessSessionEngine::OnRoomControlResult` | 定义 | `void InProcessSessionEngine::OnRoomControlResult( const SignalingRoomControlResult& result)` | 接收并处理 on room control result 相关逻辑。 |
| [L405](../src/apps/remote/InProcessSessionEngine.LeaseCallbacks.inc#L405) | `InProcessSessionEngine::OnRoomControlGranted` | 定义 | `void InProcessSessionEngine::OnRoomControlGranted( const SignalingRoomControlGranted& granted)` | 接收并处理 on room control granted 相关逻辑。 |
| [L449](../src/apps/remote/InProcessSessionEngine.LeaseCallbacks.inc#L449) | `InProcessSessionEngine::OnRoomControlRevoked` | 定义 | `void InProcessSessionEngine::OnRoomControlRevoked( const SignalingRoomControlRevoked& revoked)` | 接收并处理 on room control revoked 相关逻辑。 |

## `src/apps/remote/InProcessSessionEngine.Lifecycle.inc`

[打开源码](../src/apps/remote/InProcessSessionEngine.Lifecycle.inc) · **文件作用：** `InProcessSessionEngine` 的实现切片，集中实现 lifecycle 相关逻辑；成员状态仍定义在所属头文件中。

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L4](../src/apps/remote/InProcessSessionEngine.Lifecycle.inc#L4) | `InProcessSessionEngine::InProcessSessionEngine` | 定义 | `InProcessSessionEngine::InProcessSessionEngine() : InProcessSessionEngine(nullptr, {})` | SPDX-License-Identifier: GPL-3.0-only Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd) |
| [L19](../src/apps/remote/InProcessSessionEngine.Lifecycle.inc#L19) | `NormalizeMediaDeviceId` | 定义 | `NormalizeMediaDeviceId( options.preferredMicrophoneDeviceId), NormalizeMediaDeviceId( options.preferredSpeakerDeviceId), options.ffmpegX264Preset, options.ffmpegHardwareBackend, options.preferredAutomaticEncoderId)) ,...` | 实现 normalize media device id 对应的业务或工具逻辑。 |
| [L34](../src/apps/remote/InProcessSessionEngine.Lifecycle.inc#L34) | `InProcessSessionEngine::~InProcessSessionEngine` | 定义 | `InProcessSessionEngine::~InProcessSessionEngine()` | 停止相关活动并释放 InProcessSessionEngine 实例拥有的资源。 |
| [L39](../src/apps/remote/InProcessSessionEngine.Lifecycle.inc#L39) | `InProcessSessionEngine::UpdateSignalingAccessToken` | 定义 | `SessionCommandResult InProcessSessionEngine::UpdateSignalingAccessToken( std::string accessToken)` | 更新或应用 update signaling access token 相关逻辑。 |
| [L54](../src/apps/remote/InProcessSessionEngine.Lifecycle.inc#L54) | `InProcessSessionEngine::RequestAccountDeletion` | 定义 | `SessionCommandResult InProcessSessionEngine::RequestAccountDeletion()` | 发起请求或查询 request account deletion 相关逻辑。 |
| [L66](../src/apps/remote/InProcessSessionEngine.Lifecycle.inc#L66) | `InProcessSessionEngine::SetAccountDeletionResultCallback` | 定义 | `void InProcessSessionEngine::SetAccountDeletionResultCallback( std::function<void(const SignalingAccountDeletionResult&)> callback)` | 更新或应用 set account deletion result callback 相关逻辑。 |
| [L73](../src/apps/remote/InProcessSessionEngine.Lifecycle.inc#L73) | `InProcessSessionEngine::SetObserver` | 定义 | `void InProcessSessionEngine::SetObserver(ISessionEngineObserver* observer)` | 更新或应用 set observer 相关逻辑。 |
| [L86](../src/apps/remote/InProcessSessionEngine.Lifecycle.inc#L86) | `InProcessSessionEngine::Start` | 定义 | `SessionCommandResult InProcessSessionEngine::Start()` | 启动 start 相关逻辑。 |
| [L95](../src/apps/remote/InProcessSessionEngine.Lifecycle.inc#L95) | `InProcessSessionEngine::BeginStart` | 定义 | `SessionCommandResult InProcessSessionEngine::BeginStart()` | 启动 begin start 相关逻辑。 |
| [L133](../src/apps/remote/InProcessSessionEngine.Lifecycle.inc#L133) | `InProcessSessionEngine::InitializeRuntimeForStart` | 定义 | `SessionCommandResult InProcessSessionEngine::InitializeRuntimeForStart()` | 创建或初始化 initialize runtime for start 相关逻辑。 |
| [L276](../src/apps/remote/InProcessSessionEngine.Lifecycle.inc#L276) | `InProcessSessionEngine::CompleteStart` | 定义 | `SessionCommandResult InProcessSessionEngine::CompleteStart( const SessionCommandResult& runtimeResult)` | 实现 complete start 对应的业务或工具逻辑。 |
| [L330](../src/apps/remote/InProcessSessionEngine.Lifecycle.inc#L330) | `InProcessSessionEngine::Stop` | 定义 | `void InProcessSessionEngine::Stop()` | 停止 stop 相关逻辑。 |
| [L507](../src/apps/remote/InProcessSessionEngine.Lifecycle.inc#L507) | `InProcessSessionEngine::Snapshot` | 定义 | `SessionEngineSnapshot InProcessSessionEngine::Snapshot() const` | 查询并返回 snapshot 相关逻辑。 |
| [L513](../src/apps/remote/InProcessSessionEngine.Lifecycle.inc#L513) | `InProcessSessionEngine::Capabilities` | 定义 | `SessionEngineCapabilities InProcessSessionEngine::Capabilities() const` | 查询并返回 capabilities 相关逻辑。 |
| [L535](../src/apps/remote/InProcessSessionEngine.Lifecycle.inc#L535) | `InProcessSessionEngine::Diagnostics` | 定义 | `SessionDiagnosticsSnapshot InProcessSessionEngine::Diagnostics() const` | 查询并返回 diagnostics 相关逻辑。 |
| [L784](../src/apps/remote/InProcessSessionEngine.Lifecycle.inc#L784) | `InProcessSessionEngine::StartStatsPolling` | 定义 | `void InProcessSessionEngine::StartStatsPolling()` | 启动 start stats polling 相关逻辑。 |
| [L848](../src/apps/remote/InProcessSessionEngine.Lifecycle.inc#L848) | `InProcessSessionEngine::StopStatsPolling` | 定义 | `void InProcessSessionEngine::StopStatsPolling()` | 停止 stop stats polling 相关逻辑。 |

## `src/apps/remote/InProcessSessionEngine.MediaDevices.inc`

[打开源码](../src/apps/remote/InProcessSessionEngine.MediaDevices.inc) · **文件作用：** `InProcessSessionEngine` 的实现切片，集中实现 media devices 相关逻辑；成员状态仍定义在所属头文件中。

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L4](../src/apps/remote/InProcessSessionEngine.MediaDevices.inc#L4) | `InProcessSessionEngine::SetRemoteAudioPlaybackMuted` | 定义 | `SessionCommandResult InProcessSessionEngine::SetRemoteAudioPlaybackMuted( bool muted)` | SPDX-License-Identifier: GPL-3.0-only Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd) |
| [L36](../src/apps/remote/InProcessSessionEngine.MediaDevices.inc#L36) | `InProcessSessionEngine::SetRoomAudioPlaybackMuted` | 定义 | `SessionCommandResult InProcessSessionEngine::SetRoomAudioPlaybackMuted( bool muted)` | 更新或应用 set room audio playback muted 相关逻辑。 |
| [L43](../src/apps/remote/InProcessSessionEngine.MediaDevices.inc#L43) | `InProcessSessionEngine::RefreshLocalMediaDevices` | 定义 | `InProcessSessionEngine::RefreshLocalMediaDevices()` | 刷新 refresh local media devices 相关逻辑。 |
| [L188](../src/apps/remote/InProcessSessionEngine.MediaDevices.inc#L188) | `InProcessSessionEngine::SelectLocalCameraDevice` | 定义 | `InProcessSessionEngine::SelectLocalCameraDevice( const std::string& deviceId)` | 查询并返回 select local camera device 相关逻辑。 |
| [L410](../src/apps/remote/InProcessSessionEngine.MediaDevices.inc#L410) | `InProcessSessionEngine::SelectLocalMicrophoneDevice` | 定义 | `InProcessSessionEngine::SelectLocalMicrophoneDevice( const std::string& deviceId)` | 查询并返回 select local microphone device 相关逻辑。 |
| [L499](../src/apps/remote/InProcessSessionEngine.MediaDevices.inc#L499) | `InProcessSessionEngine::SelectLocalSpeakerDevice` | 定义 | `InProcessSessionEngine::SelectLocalSpeakerDevice( const std::string& deviceId)` | 查询并返回 select local speaker device 相关逻辑。 |

## `src/apps/remote/InProcessSessionEngine.MediaHooks.inc`

[打开源码](../src/apps/remote/InProcessSessionEngine.MediaHooks.inc) · **文件作用：** `InProcessSessionEngine` 的实现切片，集中实现 media hooks 相关逻辑；成员状态仍定义在所属头文件中。

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L4](../src/apps/remote/InProcessSessionEngine.MediaHooks.inc#L4) | `InProcessSessionEngine::SetLocalCameraPreviewSink` | 定义 | `void InProcessSessionEngine::SetLocalCameraPreviewSink( webrtc::VideoSinkInterface<webrtc::VideoFrame>* sink)` | SPDX-License-Identifier: GPL-3.0-only Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd) |
| [L23](../src/apps/remote/InProcessSessionEngine.MediaHooks.inc#L23) | `InProcessSessionEngine::SetRemoteInputSink` | 定义 | `void InProcessSessionEngine::SetRemoteInputSink(IRemoteInputSink* sink)` | 更新或应用 set remote input sink 相关逻辑。 |
| [L29](../src/apps/remote/InProcessSessionEngine.MediaHooks.inc#L29) | `InProcessSessionEngine::SetRemoteFileTransferSink` | 定义 | `void InProcessSessionEngine::SetRemoteFileTransferSink( IFileTransferSink* sink)` | 更新或应用 set remote file transfer sink 相关逻辑。 |
| [L36](../src/apps/remote/InProcessSessionEngine.MediaHooks.inc#L36) | `InProcessSessionEngine::SetRemoteClipboardSink` | 定义 | `void InProcessSessionEngine::SetRemoteClipboardSink(IClipboardSink* sink)` | 更新或应用 set remote clipboard sink 相关逻辑。 |
| [L42](../src/apps/remote/InProcessSessionEngine.MediaHooks.inc#L42) | `InProcessSessionEngine::SetPreferredHardwareDecoderName` | 定义 | `void InProcessSessionEngine::SetPreferredHardwareDecoderName( std::string name)` | 更新或应用 set preferred hardware decoder name 相关逻辑。 |
| [L50](../src/apps/remote/InProcessSessionEngine.MediaHooks.inc#L50) | `InProcessSessionEngine::ApplyVideoPipelinePreferences` | 定义 | `SessionCommandResult InProcessSessionEngine::ApplyVideoPipelinePreferences( DesktopCaptureImplementation desktopCaptureImplementation, VideoEncoderPreference videoEncoderPreference, FfmpegX264Preset quality, FfmpegHar...` | 更新或应用 apply video pipeline preferences 相关逻辑。 |

## `src/apps/remote/InProcessSessionEngine.RemoteControl.inc`

[打开源码](../src/apps/remote/InProcessSessionEngine.RemoteControl.inc) · **文件作用：** `InProcessSessionEngine` 的实现切片，集中实现 remote control 相关逻辑；成员状态仍定义在所属头文件中。

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L4](../src/apps/remote/InProcessSessionEngine.RemoteControl.inc#L4) | `InProcessSessionEngine::RequestRoomControl` | 定义 | `SessionCommandResult InProcessSessionEngine::RequestRoomControl()` | SPDX-License-Identifier: GPL-3.0-only Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd) |
| [L41](../src/apps/remote/InProcessSessionEngine.RemoteControl.inc#L41) | `InProcessSessionEngine::RespondToRoomControl` | 定义 | `SessionCommandResult InProcessSessionEngine::RespondToRoomControl( const std::string& requestId, bool accepted)` | 处理并回复 respond to room control 相关逻辑。 |
| [L87](../src/apps/remote/InProcessSessionEngine.RemoteControl.inc#L87) | `InProcessSessionEngine::ReleaseRoomControl` | 定义 | `SessionCommandResult InProcessSessionEngine::ReleaseRoomControl()` | 释放或取消 release room control 相关逻辑。 |
| [L115](../src/apps/remote/InProcessSessionEngine.RemoteControl.inc#L115) | `InProcessSessionEngine::SendRoomMemberAction` | 定义 | `SessionCommandResult InProcessSessionEngine::SendRoomMemberAction( const std::string& peerDeviceId, RoomMemberAction action, std::uint64_t screenShareEpoch)` | 发送或发布 send room member action 相关逻辑。 |
| [L231](../src/apps/remote/InProcessSessionEngine.RemoteControl.inc#L231) | `InProcessSessionEngine::RequestRoomMemberScreenShare` | 定义 | `SessionCommandResult InProcessSessionEngine::RequestRoomMemberScreenShare( const std::string& peerDeviceId)` | 发起请求或查询 request room member screen share 相关逻辑。 |
| [L239](../src/apps/remote/InProcessSessionEngine.RemoteControl.inc#L239) | `InProcessSessionEngine::RequestRoomMemberMicrophoneMute` | 定义 | `InProcessSessionEngine::RequestRoomMemberMicrophoneMute( const std::string& peerDeviceId)` | 发起请求或查询 request room member microphone mute 相关逻辑。 |
| [L247](../src/apps/remote/InProcessSessionEngine.RemoteControl.inc#L247) | `InProcessSessionEngine::RequestRemoteRoomScreenShareStop` | 定义 | `InProcessSessionEngine::RequestRemoteRoomScreenShareStop( const std::string& peerDeviceId, std::uint64_t screenShareEpoch)` | 发起请求或查询 request remote room screen share stop 相关逻辑。 |
| [L256](../src/apps/remote/InProcessSessionEngine.RemoteControl.inc#L256) | `InProcessSessionEngine::SendRoomMemberActionResponse` | 定义 | `SendResult InProcessSessionEngine::SendRoomMemberActionResponse( const std::shared_ptr<RoomPairRuntime>& pair, const RoomMemberActionEnvelope& request, bool accepted, const std::string& error)` | 发送或发布 send room member action response 相关逻辑。 |
| [L284](../src/apps/remote/InProcessSessionEngine.RemoteControl.inc#L284) | `InProcessSessionEngine::RespondToRoomMemberScreenShare` | 定义 | `InProcessSessionEngine::RespondToRoomMemberScreenShare( const std::string& requesterDeviceId, std::uint64_t sequence, bool accepted)` | 处理并回复 respond to room member screen share 相关逻辑。 |
| [L365](../src/apps/remote/InProcessSessionEngine.RemoteControl.inc#L365) | `InProcessSessionEngine::SendRoomInput` | 定义 | `SessionCommandResult InProcessSessionEngine::SendRoomInput( const RemoteInputEvent& event)` | 发送或发布 send room input 相关逻辑。 |

## `src/apps/remote/InProcessSessionEngine.RemoteCursor.inc`

[打开源码](../src/apps/remote/InProcessSessionEngine.RemoteCursor.inc) · **文件作用：** `InProcessSessionEngine` 的实现切片，集中实现 remote cursor 相关逻辑；成员状态仍定义在所属头文件中。

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L4](../src/apps/remote/InProcessSessionEngine.RemoteCursor.inc#L4) | `InProcessSessionEngine::SetRemoteCursorCallback` | 定义 | `void InProcessSessionEngine::SetRemoteCursorCallback( RemoteCursorCallback callback)` | SPDX-License-Identifier: GPL-3.0-only Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd) |
| [L11](../src/apps/remote/InProcessSessionEngine.RemoteCursor.inc#L11) | `InProcessSessionEngine::StartRemoteCursorPublishing` | 定义 | `void InProcessSessionEngine::StartRemoteCursorPublishing( const DisplayDescriptor& display, std::uint64_t layoutVersion)` | 启动 start remote cursor publishing 相关逻辑。 |
| [L28](../src/apps/remote/InProcessSessionEngine.RemoteCursor.inc#L28) | `InProcessSessionEngine::StopRemoteCursorPublishing` | 定义 | `void InProcessSessionEngine::StopRemoteCursorPublishing()` | 停止 stop remote cursor publishing 相关逻辑。 |
| [L88](../src/apps/remote/InProcessSessionEngine.RemoteCursor.inc#L88) | `InProcessSessionEngine::OnLocalCursorObservation` | 定义 | `void InProcessSessionEngine::OnLocalCursorObservation( WindowsCursorObservation observation)` | 接收并处理 on local cursor observation 相关逻辑。 |
| [L192](../src/apps/remote/InProcessSessionEngine.RemoteCursor.inc#L192) | `InProcessSessionEngine::RepublishRemoteCursor` | 定义 | `void InProcessSessionEngine::RepublishRemoteCursor()` | 实现 republish remote cursor 对应的业务或工具逻辑。 |
| [L204](../src/apps/remote/InProcessSessionEngine.RemoteCursor.inc#L204) | `InProcessSessionEngine::DispatchRemoteCursorData` | 定义 | `bool InProcessSessionEngine::DispatchRemoteCursorData( const std::string& pairId, const std::string& label, std::span<const std::uint8_t> payload)` | 接收并处理 dispatch remote cursor data 相关逻辑。 |

## `src/apps/remote/InProcessSessionEngine.Room.inc`

[打开源码](../src/apps/remote/InProcessSessionEngine.Room.inc) · **文件作用：** `InProcessSessionEngine` 的实现切片，集中实现 room 相关逻辑；成员状态仍定义在所属头文件中。

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L4](../src/apps/remote/InProcessSessionEngine.Room.inc#L4) | `InProcessSessionEngine::CreateRoom` | 定义 | `SessionCommandResult InProcessSessionEngine::CreateRoom( std::uint32_t capacity)` | SPDX-License-Identifier: GPL-3.0-only Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd) |
| [L43](../src/apps/remote/InProcessSessionEngine.Room.inc#L43) | `InProcessSessionEngine::JoinRoom` | 定义 | `SessionCommandResult InProcessSessionEngine::JoinRoom( const std::string& roomId)` | 实现 join room 对应的业务或工具逻辑。 |
| [L80](../src/apps/remote/InProcessSessionEngine.Room.inc#L80) | `InProcessSessionEngine::QueryRoomAvailability` | 定义 | `SessionCommandResult InProcessSessionEngine::QueryRoomAvailability( const std::vector<std::string>& roomIds)` | 发起请求或查询 query room availability 相关逻辑。 |
| [L108](../src/apps/remote/InProcessSessionEngine.Room.inc#L108) | `InProcessSessionEngine::RespondToRoomJoin` | 定义 | `SessionCommandResult InProcessSessionEngine::RespondToRoomJoin( const std::string& requestId, bool accepted)` | 处理并回复 respond to room join 相关逻辑。 |
| [L154](../src/apps/remote/InProcessSessionEngine.Room.inc#L154) | `InProcessSessionEngine::SetRoomCapacity` | 定义 | `SessionCommandResult InProcessSessionEngine::SetRoomCapacity( std::uint32_t capacity)` | 更新或应用 set room capacity 相关逻辑。 |
| [L187](../src/apps/remote/InProcessSessionEngine.Room.inc#L187) | `InProcessSessionEngine::LeaveRoom` | 定义 | `SessionCommandResult InProcessSessionEngine::LeaveRoom()` | 实现 leave room 对应的业务或工具逻辑。 |
| [L223](../src/apps/remote/InProcessSessionEngine.Room.inc#L223) | `InProcessSessionEngine::ExitRoomAfterRecoveryFailure` | 定义 | `InProcessSessionEngine::ExitRoomAfterRecoveryFailure()` | 实现 exit room after recovery failure 对应的业务或工具逻辑。 |

## `src/apps/remote/InProcessSessionEngine.RoomCallbacks.inc`

[打开源码](../src/apps/remote/InProcessSessionEngine.RoomCallbacks.inc) · **文件作用：** `InProcessSessionEngine` 的实现切片，集中实现 room callbacks 相关逻辑；成员状态仍定义在所属头文件中。

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L4](../src/apps/remote/InProcessSessionEngine.RoomCallbacks.inc#L4) | `InProcessSessionEngine::OnRoomReady` | 定义 | `void InProcessSessionEngine::OnRoomReady( const SignalingRoomReady& ready)` | SPDX-License-Identifier: GPL-3.0-only Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd) |
| [L77](../src/apps/remote/InProcessSessionEngine.RoomCallbacks.inc#L77) | `InProcessSessionEngine::OnRoomState` | 定义 | `void InProcessSessionEngine::OnRoomState(const RoomSnapshot& room)` | 接收并处理 on room state 相关逻辑。 |
| [L290](../src/apps/remote/InProcessSessionEngine.RoomCallbacks.inc#L290) | `InProcessSessionEngine::OnRoomJoinPending` | 定义 | `void InProcessSessionEngine::OnRoomJoinPending( const SignalingRoomJoinPending& pending)` | 接收并处理 on room join pending 相关逻辑。 |
| [L305](../src/apps/remote/InProcessSessionEngine.RoomCallbacks.inc#L305) | `InProcessSessionEngine::OnRoomJoinRequested` | 定义 | `void InProcessSessionEngine::OnRoomJoinRequested( const RoomJoinRequest& request)` | 接收并处理 on room join requested 相关逻辑。 |
| [L328](../src/apps/remote/InProcessSessionEngine.RoomCallbacks.inc#L328) | `InProcessSessionEngine::OnRoomJoinResult` | 定义 | `void InProcessSessionEngine::OnRoomJoinResult( const SignalingRoomJoinResult& result)` | 接收并处理 on room join result 相关逻辑。 |
| [L369](../src/apps/remote/InProcessSessionEngine.RoomCallbacks.inc#L369) | `InProcessSessionEngine::OnRoomAvailabilityResult` | 定义 | `void InProcessSessionEngine::OnRoomAvailabilityResult( const SignalingRoomAvailabilityResult& result)` | 接收并处理 on room availability result 相关逻辑。 |
| [L388](../src/apps/remote/InProcessSessionEngine.RoomCallbacks.inc#L388) | `InProcessSessionEngine::OnRoomClosed` | 定义 | `void InProcessSessionEngine::OnRoomClosed( const SignalingRoomClosed& closed)` | 接收并处理 on room closed 相关逻辑。 |

## `src/apps/remote/InProcessSessionEngine.RoomPairChannels.inc`

[打开源码](../src/apps/remote/InProcessSessionEngine.RoomPairChannels.inc) · **文件作用：** `InProcessSessionEngine` 的实现切片，集中实现 room pair channels 相关逻辑；成员状态仍定义在所属头文件中。

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L4](../src/apps/remote/InProcessSessionEngine.RoomPairChannels.inc#L4) | `InProcessSessionEngine::OnRoomPairDataChannelStateChanged` | 定义 | `void InProcessSessionEngine::OnRoomPairDataChannelStateChanged( const std::string& pairId, const DataChannelInfo& channel)` | SPDX-License-Identifier: GPL-3.0-only Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd) |
| [L118](../src/apps/remote/InProcessSessionEngine.RoomPairChannels.inc#L118) | `InProcessSessionEngine::StartClipboardWarmup` | 定义 | `void InProcessSessionEngine::StartClipboardWarmup( const std::shared_ptr<RoomPairRuntime>& pair)` | 启动 start clipboard warmup 相关逻辑。 |

## `src/apps/remote/InProcessSessionEngine.RoomPairControlDispatch.inc`

[打开源码](../src/apps/remote/InProcessSessionEngine.RoomPairControlDispatch.inc) · **文件作用：** 所属主函数的连续实现片段；这里只包含该函数的局部流程，不单独声明成员变量或顶层函数。

当前规则未提取到公开类型、成员/文件级变量或函数；该文件可能只包含宏、模板片段、资源数据或条件编译内容，请直接打开源码查看。

## `src/apps/remote/InProcessSessionEngine.RoomPairDispatchPreamble.inc`

[打开源码](../src/apps/remote/InProcessSessionEngine.RoomPairDispatchPreamble.inc) · **文件作用：** 所属主函数的连续实现片段；这里只包含该函数的局部流程，不单独声明成员变量或顶层函数。

当前规则未提取到公开类型、成员/文件级变量或函数；该文件可能只包含宏、模板片段、资源数据或条件编译内容，请直接打开源码查看。

## `src/apps/remote/InProcessSessionEngine.RoomPairInputDispatch.inc`

[打开源码](../src/apps/remote/InProcessSessionEngine.RoomPairInputDispatch.inc) · **文件作用：** 所属主函数的连续实现片段；这里只包含该函数的局部流程，不单独声明成员变量或顶层函数。

当前规则未提取到公开类型、成员/文件级变量或函数；该文件可能只包含宏、模板片段、资源数据或条件编译内容，请直接打开源码查看。

## `src/apps/remote/InProcessSessionEngine.RoomPairLifecycle.inc`

[打开源码](../src/apps/remote/InProcessSessionEngine.RoomPairLifecycle.inc) · **文件作用：** `InProcessSessionEngine` 的实现切片，集中实现 room pair lifecycle 相关逻辑；成员状态仍定义在所属头文件中。

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L4](../src/apps/remote/InProcessSessionEngine.RoomPairLifecycle.inc#L4) | `InProcessSessionEngine::OnRoomPairReady` | 定义 | `void InProcessSessionEngine::OnRoomPairReady( const SignalingRoomPairReady& ready)` | SPDX-License-Identifier: GPL-3.0-only Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd) |
| [L114](../src/apps/remote/InProcessSessionEngine.RoomPairLifecycle.inc#L114) | `InProcessSessionEngine::OnRoomPairClosed` | 定义 | `void InProcessSessionEngine::OnRoomPairClosed( const SignalingRoomPairClosed& closed)` | 接收并处理 on room pair closed 相关逻辑。 |
| [L160](../src/apps/remote/InProcessSessionEngine.RoomPairLifecycle.inc#L160) | `InProcessSessionEngine::RetireRoomPair` | 定义 | `void InProcessSessionEngine::RetireRoomPair( std::shared_ptr<RoomPairRuntime> pair)` | 实现 retire room pair 对应的业务或工具逻辑。 |
| [L179](../src/apps/remote/InProcessSessionEngine.RoomPairLifecycle.inc#L179) | `InProcessSessionEngine::SendRoomPairDescription` | 定义 | `bool InProcessSessionEngine::SendRoomPairDescription( const std::string& pairId, const SessionDescription& description)` | 发送或发布 send room pair description 相关逻辑。 |
| [L204](../src/apps/remote/InProcessSessionEngine.RoomPairLifecycle.inc#L204) | `InProcessSessionEngine::SendRoomPairIceCandidate` | 定义 | `bool InProcessSessionEngine::SendRoomPairIceCandidate( const std::string& pairId, const IceCandidate& candidate)` | 发送或发布 send room pair ice candidate 相关逻辑。 |
| [L228](../src/apps/remote/InProcessSessionEngine.RoomPairLifecycle.inc#L228) | `InProcessSessionEngine::SendRoomPairIceRestartRequest` | 定义 | `bool InProcessSessionEngine::SendRoomPairIceRestartRequest( const std::string& pairId, std::uint64_t observedGeneration, std::uint64_t requestSequence)` | 发送或发布 send room pair ice restart request 相关逻辑。 |
| [L251](../src/apps/remote/InProcessSessionEngine.RoomPairLifecycle.inc#L251) | `InProcessSessionEngine::SendRoomPairIceRestartCancel` | 定义 | `bool InProcessSessionEngine::SendRoomPairIceRestartCancel( const std::string& pairId, std::uint64_t observedGeneration, std::uint64_t requestSequence)` | 发送或发布 send room pair ice restart cancel 相关逻辑。 |

## `src/apps/remote/InProcessSessionEngine.RoomPairScreenDispatch.inc`

[打开源码](../src/apps/remote/InProcessSessionEngine.RoomPairScreenDispatch.inc) · **文件作用：** 所属主函数的连续实现片段；这里只包含该函数的局部流程，不单独声明成员变量或顶层函数。

当前规则未提取到公开类型、成员/文件级变量或函数；该文件可能只包含宏、模板片段、资源数据或条件编译内容，请直接打开源码查看。

## `src/apps/remote/InProcessSessionEngine.RoomPairState.inc`

[打开源码](../src/apps/remote/InProcessSessionEngine.RoomPairState.inc) · **文件作用：** `InProcessSessionEngine` 的实现切片，集中实现 room pair state 相关逻辑；成员状态仍定义在所属头文件中。

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L4](../src/apps/remote/InProcessSessionEngine.RoomPairState.inc#L4) | `InProcessSessionEngine::OnRoomPairControllerSnapshot` | 定义 | `void InProcessSessionEngine::OnRoomPairControllerSnapshot( const std::string& pairId, const SessionControllerSnapshot& controllerSnapshot)` | SPDX-License-Identifier: GPL-3.0-only Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd) |
| [L191](../src/apps/remote/InProcessSessionEngine.RoomPairState.inc#L191) | `InProcessSessionEngine::PrepareRoomPairMedia` | 定义 | `InProcessSessionEngine::PrepareRoomPairMedia( const std::string& pairId, bool bindNegotiatedSlots, bool preparationAlreadyClaimed)` | 实现 prepare room pair media 对应的业务或工具逻辑。 |
| [L426](../src/apps/remote/InProcessSessionEngine.RoomPairState.inc#L426) | `InProcessSessionEngine::PrepareRoomPairAnswer` | 定义 | `InProcessSessionEngine::PrepareRoomPairAnswer( const std::string& pairId)` | 实现 prepare room pair answer 对应的业务或工具逻辑。 |

## `src/apps/remote/InProcessSessionEngine.RuntimeHelpers.inc`

[打开源码](../src/apps/remote/InProcessSessionEngine.RuntimeHelpers.inc) · **文件作用：** `InProcessSessionEngine` 的实现切片，集中实现 runtime helpers 相关逻辑；成员状态仍定义在所属头文件中。

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L4](../src/apps/remote/InProcessSessionEngine.RuntimeHelpers.inc#L4) | `InProcessSessionEngine::RequireReady` | 定义 | `SessionCommandResult InProcessSessionEngine::RequireReady( const char* operation) const` | SPDX-License-Identifier: GPL-3.0-only Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd) |
| [L16](../src/apps/remote/InProcessSessionEngine.RuntimeHelpers.inc#L16) | `InProcessSessionEngine::SignalingIsOnline` | 定义 | `bool InProcessSessionEngine::SignalingIsOnline() const` | 实现 signaling is online 对应的业务或工具逻辑。 |
| [L22](../src/apps/remote/InProcessSessionEngine.RuntimeHelpers.inc#L22) | `InProcessSessionEngine::DisposeClosedSession` | 定义 | `void InProcessSessionEngine::DisposeClosedSession()` | 关闭并清理 dispose closed session 相关逻辑。 |
| [L45](../src/apps/remote/InProcessSessionEngine.RuntimeHelpers.inc#L45) | `InProcessSessionEngine::ResetSessionStateLocked` | 定义 | `void InProcessSessionEngine::ResetSessionStateLocked()` | 重置或移除 reset session state locked 相关逻辑。 |
| [L96](../src/apps/remote/InProcessSessionEngine.RuntimeHelpers.inc#L96) | `InProcessSessionEngine::ResetRoomStateLocked` | 定义 | `void InProcessSessionEngine::ResetRoomStateLocked()` | 重置或移除 reset room state locked 相关逻辑。 |
| [L164](../src/apps/remote/InProcessSessionEngine.RuntimeHelpers.inc#L164) | `InProcessSessionEngine::StopLocalDesktopCapture` | 定义 | `void InProcessSessionEngine::StopLocalDesktopCapture()` | 停止 stop local desktop capture 相关逻辑。 |
| [L197](../src/apps/remote/InProcessSessionEngine.RuntimeHelpers.inc#L197) | `InProcessSessionEngine::BroadcastSharedDisplayLayout` | 定义 | `void InProcessSessionEngine::BroadcastSharedDisplayLayout()` | 实现 broadcast shared display layout 对应的业务或工具逻辑。 |
| [L241](../src/apps/remote/InProcessSessionEngine.RuntimeHelpers.inc#L241) | `InProcessSessionEngine::BroadcastSharedDisplayCatalog` | 定义 | `void InProcessSessionEngine::BroadcastSharedDisplayCatalog()` | 实现 broadcast shared display catalog 对应的业务或工具逻辑。 |

## `src/apps/remote/InProcessSessionEngine.ScreenPolicyFile.inc`

[打开源码](../src/apps/remote/InProcessSessionEngine.ScreenPolicyFile.inc) · **文件作用：** `InProcessSessionEngine` 的实现切片，集中实现 screen policy file 相关逻辑；成员状态仍定义在所属头文件中。

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L4](../src/apps/remote/InProcessSessionEngine.ScreenPolicyFile.inc#L4) | `InProcessSessionEngine::SetRoomScreenFrameRate` | 定义 | `SessionCommandResult InProcessSessionEngine::SetRoomScreenFrameRate( const std::string& pairId, std::uint32_t framesPerSecond)` | SPDX-License-Identifier: GPL-3.0-only Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd) |
| [L14](../src/apps/remote/InProcessSessionEngine.ScreenPolicyFile.inc#L14) | `InProcessSessionEngine::SetRoomScreenStreamPreference` | 定义 | `SessionCommandResult InProcessSessionEngine::SetRoomScreenStreamPreference( const std::string& pairId, const ScreenStreamPreferenceRequest& preference)` | 更新或应用 set room screen stream preference 相关逻辑。 |
| [L143](../src/apps/remote/InProcessSessionEngine.ScreenPolicyFile.inc#L143) | `InProcessSessionEngine::RequestRemoteSharedDisplaySwitch` | 定义 | `InProcessSessionEngine::RequestRemoteSharedDisplaySwitch( const std::string& pairId, const std::string& stableDisplayKey)` | 发起请求或查询 request remote shared display switch 相关逻辑。 |
| [L268](../src/apps/remote/InProcessSessionEngine.ScreenPolicyFile.inc#L268) | `InProcessSessionEngine::ApplyLocalScreenFrameRate` | 定义 | `SessionCommandResult InProcessSessionEngine::ApplyLocalScreenFrameRate( std::uint32_t framesPerSecond)` | 更新或应用 apply local screen frame rate 相关逻辑。 |
| [L335](../src/apps/remote/InProcessSessionEngine.ScreenPolicyFile.inc#L335) | `InProcessSessionEngine::SwitchLocalSharedDisplay` | 定义 | `SessionCommandResult InProcessSessionEngine::SwitchLocalSharedDisplay( const std::string& stableDisplayKey)` | 实现 switch local shared display 对应的业务或工具逻辑。 |
| [L549](../src/apps/remote/InProcessSessionEngine.ScreenPolicyFile.inc#L549) | `InProcessSessionEngine::SendRoomFileMessage` | 定义 | `SessionCommandResult InProcessSessionEngine::SendRoomFileMessage( const std::string& peerDeviceId, const FileTransferMessage& message)` | 发送或发布 send room file message 相关逻辑。 |
| [L640](../src/apps/remote/InProcessSessionEngine.ScreenPolicyFile.inc#L640) | `InProcessSessionEngine::SendRoomClipboardMessage` | 定义 | `SessionCommandResult InProcessSessionEngine::SendRoomClipboardMessage( const std::string& peerDeviceId, const std::string& clipboardSessionId, const ClipboardMessage& message)` | 发送或发布 send room clipboard message 相关逻辑。 |

## `src/apps/remote/InProcessSessionEngine.ScreenShare.inc`

[打开源码](../src/apps/remote/InProcessSessionEngine.ScreenShare.inc) · **文件作用：** `InProcessSessionEngine` 的实现切片，集中实现 screen share 相关逻辑；成员状态仍定义在所属头文件中。

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L4](../src/apps/remote/InProcessSessionEngine.ScreenShare.inc#L4) | `InProcessSessionEngine::StartRoomScreenShare` | 定义 | `SessionCommandResult InProcessSessionEngine::StartRoomScreenShare()` | SPDX-License-Identifier: GPL-3.0-only Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd) |
| [L37](../src/apps/remote/InProcessSessionEngine.ScreenShare.inc#L37) | `InProcessSessionEngine::RefreshLocalDisplays` | 定义 | `SessionCommandResult InProcessSessionEngine::RefreshLocalDisplays()` | 刷新 refresh local displays 相关逻辑。 |
| [L112](../src/apps/remote/InProcessSessionEngine.ScreenShare.inc#L112) | `InProcessSessionEngine::SelectRoomScreenShareDisplay` | 定义 | `InProcessSessionEngine::SelectRoomScreenShareDisplay( const std::string& stableDisplayKey)` | 查询并返回 select room screen share display 相关逻辑。 |
| [L140](../src/apps/remote/InProcessSessionEngine.ScreenShare.inc#L140) | `InProcessSessionEngine::StopRoomScreenShare` | 定义 | `SessionCommandResult InProcessSessionEngine::StopRoomScreenShare()` | 停止 stop room screen share 相关逻辑。 |
| [L170](../src/apps/remote/InProcessSessionEngine.ScreenShare.inc#L170) | `InProcessSessionEngine::RespondToRoomScreenShareSwitch` | 定义 | `InProcessSessionEngine::RespondToRoomScreenShareSwitch( const std::string& requestId, bool accepted)` | 处理并回复 respond to room screen share switch 相关逻辑。 |
| [L223](../src/apps/remote/InProcessSessionEngine.ScreenShare.inc#L223) | `InProcessSessionEngine::CancelRoomScreenShareSwitch` | 定义 | `InProcessSessionEngine::CancelRoomScreenShareSwitch()` | 判断 cancel room screen share switch 相关逻辑。 |

## `src/apps/remote/InProcessSessionEngine.VideoSlots.inc`

[打开源码](../src/apps/remote/InProcessSessionEngine.VideoSlots.inc) · **文件作用：** `InProcessSessionEngine` 的实现切片，集中实现 video slots 相关逻辑；成员状态仍定义在所属头文件中。

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L4](../src/apps/remote/InProcessSessionEngine.VideoSlots.inc#L4) | `InProcessSessionEngine::SetRoomVideoSource` | 定义 | `SessionCommandResult InProcessSessionEngine::SetRoomVideoSource( const std::string& slot, webrtc::scoped_refptr<webrtc::VideoTrackSourceInterface> source, const std::string& trackId, std::optional<std::string> expecte...` | SPDX-License-Identifier: GPL-3.0-only Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd) |
| [L121](../src/apps/remote/InProcessSessionEngine.VideoSlots.inc#L121) | `InProcessSessionEngine::ClearRoomVideoSource` | 定义 | `SessionCommandResult InProcessSessionEngine::ClearRoomVideoSource( const std::string& slot)` | 重置或移除 clear room video source 相关逻辑。 |
| [L180](../src/apps/remote/InProcessSessionEngine.VideoSlots.inc#L180) | `InProcessSessionEngine::SetRoomVideoSlotSendingActive` | 定义 | `SessionCommandResult InProcessSessionEngine::SetRoomVideoSlotSendingActive( const std::string& slot, bool active)` | 更新或应用 set room video slot sending active 相关逻辑。 |
| [L212](../src/apps/remote/InProcessSessionEngine.VideoSlots.inc#L212) | `InProcessSessionEngine::SetRoomRemoteVideoSink` | 定义 | `SessionCommandResult InProcessSessionEngine::SetRoomRemoteVideoSink( const std::string& pairId, const std::string& slot, webrtc::VideoSinkInterface<webrtc::VideoFrame>* sink)` | 更新或应用 set room remote video sink 相关逻辑。 |
| [L300](../src/apps/remote/InProcessSessionEngine.VideoSlots.inc#L300) | `InProcessSessionEngine::NotifyRoomScreenFirstFramePresented` | 定义 | `InProcessSessionEngine::NotifyRoomScreenFirstFramePresented( const std::string& pairId, std::uint64_t screenShareGeneration, std::uint32_t startupElapsedMs)` | 通知或报告 notify room screen first frame presented 相关逻辑。 |

## `src/apps/remote/InProcessSessionEngineInternal.h`

[打开源码](../src/apps/remote/InProcessSessionEngineInternal.h) · **文件作用：** 声明 in process session engine internal 相关类型、接口、配置和成员状态。

### 类型

| 行 | 类型 | 种类 | 作用 |
|---:|---|---|---|
| [L17](../src/apps/remote/InProcessSessionEngineInternal.h#L17) | `InProcessSessionEngine::RoomPairBridge` | class | 定义 InProcessSessionEngine::RoomPairBridge 的 class 类型和相关状态。 |
| [L80](../src/apps/remote/InProcessSessionEngineInternal.h#L80) | `InProcessSessionEngine::RoomPairRuntime` | struct | 定义 InProcessSessionEngine::RoomPairRuntime 的 struct 类型和相关状态。 |

### 成员与文件级变量

| 行 | 变量 | 声明 | 作用 |
|---:|---|---|---|
| [L76](../src/apps/remote/InProcessSessionEngineInternal.h#L76) | `engine_` | `InProcessSessionEngine& engine_;` | 保存 engine 相关配置或运行状态。 |
| [L77](../src/apps/remote/InProcessSessionEngineInternal.h#L77) | `pairId_` | `std::string pairId_;` | 保存身份或作用域标识：pair id。 |
| [L81](../src/apps/remote/InProcessSessionEngineInternal.h#L81) | `pairId` | `std::string pairId;` | 保存身份或作用域标识：pair id。 |
| [L82](../src/apps/remote/InProcessSessionEngineInternal.h#L82) | `roomId` | `std::string roomId;` | 保存身份或作用域标识：room id。 |
| [L83](../src/apps/remote/InProcessSessionEngineInternal.h#L83) | `peerDeviceId` | `std::string peerDeviceId;` | 保存身份或作用域标识：peer device id。 |
| [L84](../src/apps/remote/InProcessSessionEngineInternal.h#L84) | `localIsOfferer` | `bool localIsOfferer = false;` | 保存 local is offerer 相关配置或运行状态。 |
| [L85](../src/apps/remote/InProcessSessionEngineInternal.h#L85) | `offerNegotiationStarted` | `bool offerNegotiationStarted = false;` | 保存 offer negotiation started 相关配置或运行状态。 |
| [L86](../src/apps/remote/InProcessSessionEngineInternal.h#L86) | `mediaSlotsPreparing` | `bool mediaSlotsPreparing = false;` | 保存 media slots preparing 相关配置或运行状态。 |
| [L87](../src/apps/remote/InProcessSessionEngineInternal.h#L87) | `mediaSlotsPrepared` | `bool mediaSlotsPrepared = false;` | 保存 media slots prepared 相关配置或运行状态。 |
| [L88](../src/apps/remote/InProcessSessionEngineInternal.h#L88) | `clipboardTransferPrimed` | `bool clipboardTransferPrimed = false;` | 保存 clipboard transfer primed 相关配置或运行状态。 |
| [L89](../src/apps/remote/InProcessSessionEngineInternal.h#L89) | `clipboardWarmupPending` | `bool clipboardWarmupPending = false;` | 保存待处理队列或请求：clipboard warmup pending。 |
| [L90](../src/apps/remote/InProcessSessionEngineInternal.h#L90) | `clipboardWarmupRunning` | `bool clipboardWarmupRunning = false;` | 保存 clipboard warmup running 相关配置或运行状态。 |
| [L91](../src/apps/remote/InProcessSessionEngineInternal.h#L91) | `clipboardWarmupCancelled` | `bool clipboardWarmupCancelled = false;` | 保存 clipboard warmup cancelled 相关配置或运行状态。 |
| [L92](../src/apps/remote/InProcessSessionEngineInternal.h#L92) | `screenFirstFramePresentedGeneration` | `std::uint64_t screenFirstFramePresentedGeneration = 0;` | 标记当前世代，用于拒绝过期异步结果：screen first frame presented generation。 |
| [L93](../src/apps/remote/InProcessSessionEngineInternal.h#L93) | `screenFirstFrameStartupMs` | `std::uint32_t screenFirstFrameStartupMs = 0;` | 保存 screen first frame startup ms 相关配置或运行状态。 |
| [L94](../src/apps/remote/InProcessSessionEngineInternal.h#L94) | `screenStartupRefreshRequests` | `std::uint32_t screenStartupRefreshRequests = 0;` | 保存 screen startup refresh requests 相关配置或运行状态。 |
| [L95](../src/apps/remote/InProcessSessionEngineInternal.h#L95) | `screenStartupRefreshGeneration` | `std::uint64_t screenStartupRefreshGeneration = 0;` | 标记当前世代，用于拒绝过期异步结果：screen startup refresh generation。 |
| [L96](../src/apps/remote/InProcessSessionEngineInternal.h#L96) | `openDataChannels` | `std::unordered_map<std::string, bool> openDataChannels;` | 保存 open data channels 相关配置或运行状态。 |
| [L97](../src/apps/remote/InProcessSessionEngineInternal.h#L97) | `lastFastInputSequence` | `std::uint64_t lastFastInputSequence = 0;` | 保存单调序号，用于排序或去重：last fast input sequence。 |
| [L98](../src/apps/remote/InProcessSessionEngineInternal.h#L98) | `lastReliableInputSequence` | `std::uint64_t lastReliableInputSequence = 0;` | 保存单调序号，用于排序或去重：last reliable input sequence。 |
| [L103](../src/apps/remote/InProcessSessionEngineInternal.h#L103) | `lastPointerStateSequence` | `std::uint64_t lastPointerStateSequence = 0;` | Mouse-button state transitions can be duplicated onto the reliable channel. Track only state transitions here: comparing them with newer unordered move packets can incorrectly d... |
| [L104](../src/apps/remote/InProcessSessionEngineInternal.h#L104) | `lastCursorPositionSequence` | `std::uint64_t lastCursorPositionSequence = 0;` | 保存单调序号，用于排序或去重：last cursor position sequence。 |
| [L105](../src/apps/remote/InProcessSessionEngineInternal.h#L105) | `lastCursorShapeSequence` | `std::uint64_t lastCursorShapeSequence = 0;` | 保存单调序号，用于排序或去重：last cursor shape sequence。 |
| [L106](../src/apps/remote/InProcessSessionEngineInternal.h#L106) | `lastScreenControlSequence` | `std::uint64_t lastScreenControlSequence = 0;` | 保存单调序号，用于排序或去重：last screen control sequence。 |
| [L107](../src/apps/remote/InProcessSessionEngineInternal.h#L107) | `nextRoomMemberActionSequence` | `std::uint64_t nextRoomMemberActionSequence = 0;` | 保存单调序号，用于排序或去重：next room member action sequence。 |
| [L108](../src/apps/remote/InProcessSessionEngineInternal.h#L108) | `lastRoomMemberActionSequence` | `std::uint64_t lastRoomMemberActionSequence = 0;` | 保存单调序号，用于排序或去重：last room member action sequence。 |
| [L109](../src/apps/remote/InProcessSessionEngineInternal.h#L109) | `nextFileTransferSequence` | `std::uint64_t nextFileTransferSequence = 0;` | 保存单调序号，用于排序或去重：next file transfer sequence。 |
| [L110](../src/apps/remote/InProcessSessionEngineInternal.h#L110) | `lastFileTransferSequence` | `std::uint64_t lastFileTransferSequence = 0;` | 保存单调序号，用于排序或去重：last file transfer sequence。 |
| [L111](../src/apps/remote/InProcessSessionEngineInternal.h#L111) | `nextClipboardReliableSequence` | `std::uint64_t nextClipboardReliableSequence = 0;` | 保存单调序号，用于排序或去重：next clipboard reliable sequence。 |
| [L112](../src/apps/remote/InProcessSessionEngineInternal.h#L112) | `lastClipboardReliableSequence` | `std::uint64_t lastClipboardReliableSequence = 0;` | 保存单调序号，用于排序或去重：last clipboard reliable sequence。 |
| [L113](../src/apps/remote/InProcessSessionEngineInternal.h#L113) | `nextClipboardTransferSequence` | `std::uint64_t nextClipboardTransferSequence = 0;` | 保存单调序号，用于排序或去重：next clipboard transfer sequence。 |
| [L114](../src/apps/remote/InProcessSessionEngineInternal.h#L114) | `lastClipboardTransferSequence` | `std::uint64_t lastClipboardTransferSequence = 0;` | 保存单调序号，用于排序或去重：last clipboard transfer sequence。 |
| [L115](../src/apps/remote/InProcessSessionEngineInternal.h#L115) | `bridge` | `std::unique_ptr<RoomPairBridge> bridge;` | 保存 bridge 相关配置或运行状态。 |
| [L116](../src/apps/remote/InProcessSessionEngineInternal.h#L116) | `session` | `std::unique_ptr<LibWebRtcSession> session;` | 保存 session 相关配置或运行状态。 |
| [L117](../src/apps/remote/InProcessSessionEngineInternal.h#L117) | `controller` | `std::unique_ptr<SessionControllerBase> controller;` | 保存 controller 相关配置或运行状态。 |

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L21](../src/apps/remote/InProcessSessionEngineInternal.h#L21) | `RoomPairBridge` | 定义 | `RoomPairBridge(InProcessSessionEngine& engine, std::string pairId) : engine_(engine), pairId_(std::move(pairId)) {}` | 实现 room pair bridge 对应的业务或工具逻辑。 |
| [L25](../src/apps/remote/InProcessSessionEngineInternal.h#L25) | `SendDescription` | 定义 | `bool SendDescription( const SessionDescription& description) override` | 发送或发布 send description 相关逻辑。 |
| [L31](../src/apps/remote/InProcessSessionEngineInternal.h#L31) | `SendIceCandidate` | 定义 | `bool SendIceCandidate(const IceCandidate& candidate) override` | 发送或发布 send ice candidate 相关逻辑。 |
| [L36](../src/apps/remote/InProcessSessionEngineInternal.h#L36) | `RequestIceRestart` | 定义 | `bool RequestIceRestart(std::uint64_t observedGeneration, std::uint64_t requestSequence) override` | 发起请求或查询 request ice restart 相关逻辑。 |
| [L43](../src/apps/remote/InProcessSessionEngineInternal.h#L43) | `CancelIceRestart` | 定义 | `bool CancelIceRestart(std::uint64_t observedGeneration, std::uint64_t requestSequence) override` | 判断 cancel ice restart 相关逻辑。 |
| [L50](../src/apps/remote/InProcessSessionEngineInternal.h#L50) | `OnControllerSnapshot` | 定义 | `void OnControllerSnapshot( const SessionControllerSnapshot& snapshot) override` | 接收并处理 on controller snapshot 相关逻辑。 |
| [L56](../src/apps/remote/InProcessSessionEngineInternal.h#L56) | `OnDataChannelStateChanged` | 定义 | `void OnDataChannelStateChanged( const DataChannelInfo& channel) override` | 接收并处理 on data channel state changed 相关逻辑。 |
| [L62](../src/apps/remote/InProcessSessionEngineInternal.h#L62) | `OnDataMessage` | 定义 | `void OnDataMessage(const std::string& label, std::span<const std::uint8_t> payload, bool binary) override` | 接收并处理 on data message 相关逻辑。 |
| [L70](../src/apps/remote/InProcessSessionEngineInternal.h#L70) | `OnRemoteTrackAdded` | 定义 | `void OnRemoteTrackAdded(const RemoteTrackInfo& track) override` | 接收并处理 on remote track added 相关逻辑。 |

## `src/apps/remote/RemoteCApplicationCoordinator.cpp`

[打开源码](../src/apps/remote/RemoteCApplicationCoordinator.cpp) · **文件作用：** 实现 remote c application coordinator 相关函数与文件级辅助逻辑。

### 类型

| 行 | 类型 | 种类 | 作用 |
|---:|---|---|---|
| [L28](../src/apps/remote/RemoteCApplicationCoordinator.cpp#L28) | `AccountDisplayClaims` | struct | 定义 AccountDisplayClaims 的 struct 类型和相关状态。 |

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L34](../src/apps/remote/RemoteCApplicationCoordinator.cpp#L34) | `DecodeDisplayClaims` | 定义 | `AccountDisplayClaims DecodeDisplayClaims(const QString& idToken)` | 解码或解析 decode display claims 相关逻辑。 |
| [L64](../src/apps/remote/RemoteCApplicationCoordinator.cpp#L64) | `RemoteCApplicationCoordinator::RemoteCApplicationCoordinator` | 定义 | `RemoteCApplicationCoordinator::RemoteCApplicationCoordinator( Options options, MainWindowFactory mainWindowFactory, std::unique_ptr<auth::TokenStore> tokenStore, QObject* parent) : QObject(parent), options_(std::move(...` | 构造并初始化 RemoteCApplicationCoordinator 实例。 |
| [L85](../src/apps/remote/RemoteCApplicationCoordinator.cpp#L85) | `RemoteCApplicationCoordinator::Start` | 定义 | `bool RemoteCApplicationCoordinator::Start()` | 启动 start 相关逻辑。 |
| [L110](../src/apps/remote/RemoteCApplicationCoordinator.cpp#L110) | `RemoteCApplicationCoordinator::ContinueAuthenticatedStartup` | 定义 | `void RemoteCApplicationCoordinator::ContinueAuthenticatedStartup()` | 实现 continue authenticated startup 对应的业务或工具逻辑。 |
| [L190](../src/apps/remote/RemoteCApplicationCoordinator.cpp#L190) | `RemoteCApplicationCoordinator::ActivateFromExternalLaunch` | 定义 | `void RemoteCApplicationCoordinator::ActivateFromExternalLaunch()` | 实现 activate from external launch 对应的业务或工具逻辑。 |
| [L198](../src/apps/remote/RemoteCApplicationCoordinator.cpp#L198) | `RemoteCApplicationCoordinator::EnsureLoginWindow` | 定义 | `void RemoteCApplicationCoordinator::EnsureLoginWindow()` | 实现 ensure login window 对应的业务或工具逻辑。 |
| [L237](../src/apps/remote/RemoteCApplicationCoordinator.cpp#L237) | `RemoteCApplicationCoordinator::EnsureLoginStatusWindow` | 定义 | `void RemoteCApplicationCoordinator::EnsureLoginStatusWindow()` | 实现 ensure login status window 对应的业务或工具逻辑。 |
| [L244](../src/apps/remote/RemoteCApplicationCoordinator.cpp#L244) | `RemoteCApplicationCoordinator::ShowMainWindowAfterStartup` | 定义 | `void RemoteCApplicationCoordinator::ShowMainWindowAfterStartup()` | 实现 show main window after startup 对应的业务或工具逻辑。 |
| [L261](../src/apps/remote/RemoteCApplicationCoordinator.cpp#L261) | `RemoteCApplicationCoordinator::CompleteStartupSignedOut` | 定义 | `void RemoteCApplicationCoordinator::CompleteStartupSignedOut()` | 实现 complete startup signed out 对应的业务或工具逻辑。 |
| [L280](../src/apps/remote/RemoteCApplicationCoordinator.cpp#L280) | `RemoteCApplicationCoordinator::ShowAccountWindow` | 定义 | `void RemoteCApplicationCoordinator::ShowAccountWindow()` | 实现 show account window 对应的业务或工具逻辑。 |
| [L302](../src/apps/remote/RemoteCApplicationCoordinator.cpp#L302) | `RemoteCApplicationCoordinator::CreateMainWindow` | 定义 | `bool RemoteCApplicationCoordinator::CreateMainWindow( const QString& accessToken)` | 创建或初始化 create main window 相关逻辑。 |
| [L373](../src/apps/remote/RemoteCApplicationCoordinator.cpp#L373) | `RemoteCApplicationCoordinator::HandleAuthState` | 定义 | `void RemoteCApplicationCoordinator::HandleAuthState( auth::AuthState state)` | 接收并处理 handle auth state 相关逻辑。 |
| [L413](../src/apps/remote/RemoteCApplicationCoordinator.cpp#L413) | `RemoteCApplicationCoordinator::HandleAuthenticated` | 定义 | `void RemoteCApplicationCoordinator::HandleAuthenticated( const auth::AuthTokenSnapshot& tokens)` | 接收并处理 handle authenticated 相关逻辑。 |
| [L477](../src/apps/remote/RemoteCApplicationCoordinator.cpp#L477) | `RemoteCApplicationCoordinator::HandleAuthFailure` | 定义 | `void RemoteCApplicationCoordinator::HandleAuthFailure( const auth::AuthError& error)` | 接收并处理 handle auth failure 相关逻辑。 |
| [L525](../src/apps/remote/RemoteCApplicationCoordinator.cpp#L525) | `RemoteCApplicationCoordinator::HandleSignOut` | 定义 | `void RemoteCApplicationCoordinator::HandleSignOut()` | 接收并处理 handle sign out 相关逻辑。 |
| [L570](../src/apps/remote/RemoteCApplicationCoordinator.cpp#L570) | `RemoteCApplicationCoordinator::HandleSwitchAccount` | 定义 | `void RemoteCApplicationCoordinator::HandleSwitchAccount()` | 接收并处理 handle switch account 相关逻辑。 |
| [L614](../src/apps/remote/RemoteCApplicationCoordinator.cpp#L614) | `RemoteCApplicationCoordinator::HandleDeleteAccount` | 定义 | `void RemoteCApplicationCoordinator::HandleDeleteAccount()` | 接收并处理 handle delete account 相关逻辑。 |
| [L665](../src/apps/remote/RemoteCApplicationCoordinator.cpp#L665) | `RemoteCApplicationCoordinator::HandleAccountDeletionResult` | 定义 | `void RemoteCApplicationCoordinator::HandleAccountDeletionResult( AccountDeletionResult result)` | 接收并处理 handle account deletion result 相关逻辑。 |
| [L714](../src/apps/remote/RemoteCApplicationCoordinator.cpp#L714) | `RemoteCApplicationCoordinator::ExitApplication` | 定义 | `void RemoteCApplicationCoordinator::ExitApplication()` | 实现 exit application 对应的业务或工具逻辑。 |

## `src/apps/remote/RemoteCApplicationCoordinator.h`

[打开源码](../src/apps/remote/RemoteCApplicationCoordinator.h) · **文件作用：** 声明 remote c application coordinator 相关类型、接口、配置和成员状态。

### 类型

| 行 | 类型 | 种类 | 作用 |
|---:|---|---|---|
| [L16](../src/apps/remote/RemoteCApplicationCoordinator.h#L16) | `AuthManager` | class | 定义 AuthManager 的 class 类型和相关状态。 |
| [L17](../src/apps/remote/RemoteCApplicationCoordinator.h#L17) | `TokenStore` | class | 定义 TokenStore 的 class 类型和相关状态。 |
| [L21](../src/apps/remote/RemoteCApplicationCoordinator.h#L21) | `ControllerMainWindow` | class | 定义 ControllerMainWindow 的 class 类型和相关状态。 |
| [L22](../src/apps/remote/RemoteCApplicationCoordinator.h#L22) | `LoginWindow` | class | 定义 LoginWindow 的 class 类型和相关状态。 |
| [L23](../src/apps/remote/RemoteCApplicationCoordinator.h#L23) | `LoginStatusWindow` | class | 定义 LoginStatusWindow 的 class 类型和相关状态。 |
| [L28](../src/apps/remote/RemoteCApplicationCoordinator.h#L28) | `RemoteCApplicationCoordinator` | class | 定义 RemoteCApplicationCoordinator 的 class 类型和相关状态。 |
| [L30](../src/apps/remote/RemoteCApplicationCoordinator.h#L30) | `AccountDeletionResult` | struct | 定义 AccountDeletionResult 的 struct 类型和相关状态。 |
| [L37](../src/apps/remote/RemoteCApplicationCoordinator.h#L37) | `Options` | struct | 定义 Options 的 struct 类型和相关状态。 |
| [L45](../src/apps/remote/RemoteCApplicationCoordinator.h#L45) | `MainWindowSession` | struct | 定义 MainWindowSession 的 struct 类型和相关状态。 |

### 成员与文件级变量

| 行 | 变量 | 声明 | 作用 |
|---:|---|---|---|
| [L16](../src/apps/remote/RemoteCApplicationCoordinator.h#L16) | `AuthManager` | `class AuthManager;` | 保存 auth manager 相关配置或运行状态。 |
| [L17](../src/apps/remote/RemoteCApplicationCoordinator.h#L17) | `TokenStore` | `class TokenStore;` | 保存 token store 相关配置或运行状态。 |
| [L21](../src/apps/remote/RemoteCApplicationCoordinator.h#L21) | `ControllerMainWindow` | `class ControllerMainWindow;` | 保存 controller main window 相关配置或运行状态。 |
| [L22](../src/apps/remote/RemoteCApplicationCoordinator.h#L22) | `LoginWindow` | `class LoginWindow;` | 保存 login window 相关配置或运行状态。 |
| [L23](../src/apps/remote/RemoteCApplicationCoordinator.h#L23) | `LoginStatusWindow` | `class LoginStatusWindow;` | 保存 login status window 相关配置或运行状态。 |
| [L31](../src/apps/remote/RemoteCApplicationCoordinator.h#L31) | `deleted` | `bool deleted = false;` | 保存 deleted 相关配置或运行状态。 |
| [L32](../src/apps/remote/RemoteCApplicationCoordinator.h#L32) | `code` | `QString code;` | 保存 code 相关配置或运行状态。 |
| [L33](../src/apps/remote/RemoteCApplicationCoordinator.h#L33) | `message` | `QString message;` | 保存 message 相关配置或运行状态。 |
| [L34](../src/apps/remote/RemoteCApplicationCoordinator.h#L34) | `retryable` | `bool retryable = false;` | 保存 retryable 相关配置或运行状态。 |
| [L38](../src/apps/remote/RemoteCApplicationCoordinator.h#L38) | `authenticationRequired` | `bool authenticationRequired = false;` | 保存 authentication required 相关配置或运行状态。 |
| [L39](../src/apps/remote/RemoteCApplicationCoordinator.h#L39) | `startMainWindowInTray` | `bool startMainWindowInTray = false;` | 保存 start main window in tray 相关配置或运行状态。 |
| [L40](../src/apps/remote/RemoteCApplicationCoordinator.h#L40) | `authConfig` | `auth::AuthConfig authConfig;` | 保存 auth config 相关配置或运行状态。 |
| [L41](../src/apps/remote/RemoteCApplicationCoordinator.h#L41) | `configurationError` | `QString configurationError;` | 保存最近错误或失败原因：configuration error。 |
| [L42](../src/apps/remote/RemoteCApplicationCoordinator.h#L42) | `configurationSource` | `QString configurationSource;` | 保存 configuration source 相关配置或运行状态。 |
| [L46](../src/apps/remote/RemoteCApplicationCoordinator.h#L46) | `window` | `std::unique_ptr<controller::ControllerMainWindow> window;` | 保存 window 相关配置或运行状态。 |
| [L53](../src/apps/remote/RemoteCApplicationCoordinator.h#L53) | `setAccountDeletionResultCallback` | `setAccountDeletionResultCallback;` | 保存回调或观察者入口：set account deletion result callback。 |
| [L86](../src/apps/remote/RemoteCApplicationCoordinator.h#L86) | `options_` | `Options options_;` | 保存 options 相关配置或运行状态。 |
| [L87](../src/apps/remote/RemoteCApplicationCoordinator.h#L87) | `mainWindowFactory_` | `MainWindowFactory mainWindowFactory_;` | 保存 main window factory 相关配置或运行状态。 |
| [L88](../src/apps/remote/RemoteCApplicationCoordinator.h#L88) | `authManager_` | `std::unique_ptr<auth::AuthManager> authManager_;` | 保存 auth manager 相关配置或运行状态。 |
| [L89](../src/apps/remote/RemoteCApplicationCoordinator.h#L89) | `mainWindow_` | `std::unique_ptr<controller::ControllerMainWindow> mainWindow_;` | 保存 main window 相关配置或运行状态。 |
| [L90](../src/apps/remote/RemoteCApplicationCoordinator.h#L90) | `loginWindow_` | `std::unique_ptr<controller::LoginWindow> loginWindow_;` | 保存 login window 相关配置或运行状态。 |
| [L91](../src/apps/remote/RemoteCApplicationCoordinator.h#L91) | `loginStatusWindow_` | `std::unique_ptr<controller::LoginStatusWindow> loginStatusWindow_;` | 保存 login status window 相关配置或运行状态。 |
| [L96](../src/apps/remote/RemoteCApplicationCoordinator.h#L96) | `setAccountDeletionResultCallback_` | `setAccountDeletionResultCallback_;` | 保存回调或观察者入口：set account deletion result callback。 |
| [L97](../src/apps/remote/RemoteCApplicationCoordinator.h#L97) | `interactiveLoginRequested_` | `bool interactiveLoginRequested_ = false;` | 保存 interactive login requested 相关配置或运行状态。 |
| [L98](../src/apps/remote/RemoteCApplicationCoordinator.h#L98) | `startupAuthenticationPending_` | `bool startupAuthenticationPending_ = false;` | 保存待处理队列或请求：startup authentication pending。 |
| [L99](../src/apps/remote/RemoteCApplicationCoordinator.h#L99) | `authenticated_` | `bool authenticated_ = false;` | 保存 authenticated 相关配置或运行状态。 |
| [L100](../src/apps/remote/RemoteCApplicationCoordinator.h#L100) | `accountDeletionPending_` | `bool accountDeletionPending_ = false;` | 保存待处理队列或请求：account deletion pending。 |
| [L101](../src/apps/remote/RemoteCApplicationCoordinator.h#L101) | `accountLabel_` | `QString accountLabel_;` | 保存路径、地址或显示名称：account label。 |
| [L102](../src/apps/remote/RemoteCApplicationCoordinator.h#L102) | `accountDetail_` | `QString accountDetail_;` | 保存 account detail 相关配置或运行状态。 |
| [L103](../src/apps/remote/RemoteCApplicationCoordinator.h#L103) | `accountSubject_` | `QString accountSubject_;` | 保存 account subject 相关配置或运行状态。 |

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L59](../src/apps/remote/RemoteCApplicationCoordinator.h#L59) | `RemoteCApplicationCoordinator` | 声明 | `RemoteCApplicationCoordinator( Options options, MainWindowFactory mainWindowFactory, std::unique_ptr<auth::TokenStore> tokenStore, QObject* parent = nullptr)` | 实现 remote c application coordinator 对应的业务或工具逻辑。 |
| [L64](../src/apps/remote/RemoteCApplicationCoordinator.h#L64) | `~RemoteCApplicationCoordinator` | 声明 | `~RemoteCApplicationCoordinator() override` | 停止相关活动并释放 RemoteCApplicationCoordinator 实例拥有的资源。 |
| [L66](../src/apps/remote/RemoteCApplicationCoordinator.h#L66) | `Start` | 声明 | `bool Start()` | 启动 start 相关逻辑。 |
| [L67](../src/apps/remote/RemoteCApplicationCoordinator.h#L67) | `ActivateFromExternalLaunch` | 声明 | `void ActivateFromExternalLaunch()` | 实现 activate from external launch 对应的业务或工具逻辑。 |
| [L70](../src/apps/remote/RemoteCApplicationCoordinator.h#L70) | `EnsureLoginWindow` | 声明 | `void EnsureLoginWindow()` | 实现 ensure login window 对应的业务或工具逻辑。 |
| [L71](../src/apps/remote/RemoteCApplicationCoordinator.h#L71) | `EnsureLoginStatusWindow` | 声明 | `void EnsureLoginStatusWindow()` | 实现 ensure login status window 对应的业务或工具逻辑。 |
| [L72](../src/apps/remote/RemoteCApplicationCoordinator.h#L72) | `ContinueAuthenticatedStartup` | 声明 | `void ContinueAuthenticatedStartup()` | 实现 continue authenticated startup 对应的业务或工具逻辑。 |
| [L73](../src/apps/remote/RemoteCApplicationCoordinator.h#L73) | `ShowAccountWindow` | 声明 | `void ShowAccountWindow()` | 实现 show account window 对应的业务或工具逻辑。 |
| [L74](../src/apps/remote/RemoteCApplicationCoordinator.h#L74) | `ShowMainWindowAfterStartup` | 声明 | `void ShowMainWindowAfterStartup()` | 实现 show main window after startup 对应的业务或工具逻辑。 |
| [L75](../src/apps/remote/RemoteCApplicationCoordinator.h#L75) | `CompleteStartupSignedOut` | 声明 | `void CompleteStartupSignedOut()` | 实现 complete startup signed out 对应的业务或工具逻辑。 |
| [L76](../src/apps/remote/RemoteCApplicationCoordinator.h#L76) | `CreateMainWindow` | 声明 | `bool CreateMainWindow(const QString& accessToken)` | 创建或初始化 create main window 相关逻辑。 |
| [L77](../src/apps/remote/RemoteCApplicationCoordinator.h#L77) | `HandleAuthState` | 声明 | `void HandleAuthState(auth::AuthState state)` | 接收并处理 handle auth state 相关逻辑。 |
| [L78](../src/apps/remote/RemoteCApplicationCoordinator.h#L78) | `HandleAuthenticated` | 声明 | `void HandleAuthenticated(const auth::AuthTokenSnapshot& tokens)` | 接收并处理 handle authenticated 相关逻辑。 |
| [L79](../src/apps/remote/RemoteCApplicationCoordinator.h#L79) | `HandleAuthFailure` | 声明 | `void HandleAuthFailure(const auth::AuthError& error)` | 接收并处理 handle auth failure 相关逻辑。 |
| [L80](../src/apps/remote/RemoteCApplicationCoordinator.h#L80) | `HandleSignOut` | 声明 | `void HandleSignOut()` | 接收并处理 handle sign out 相关逻辑。 |
| [L81](../src/apps/remote/RemoteCApplicationCoordinator.h#L81) | `HandleSwitchAccount` | 声明 | `void HandleSwitchAccount()` | 接收并处理 handle switch account 相关逻辑。 |
| [L82](../src/apps/remote/RemoteCApplicationCoordinator.h#L82) | `HandleDeleteAccount` | 声明 | `void HandleDeleteAccount()` | 接收并处理 handle delete account 相关逻辑。 |
| [L83](../src/apps/remote/RemoteCApplicationCoordinator.h#L83) | `HandleAccountDeletionResult` | 声明 | `void HandleAccountDeletionResult(AccountDeletionResult result)` | 接收并处理 handle account deletion result 相关逻辑。 |
| [L84](../src/apps/remote/RemoteCApplicationCoordinator.h#L84) | `ExitApplication` | 声明 | `void ExitApplication()` | 实现 exit application 对应的业务或工具逻辑。 |

## `src/apps/remote/RemoteCProductMain.cpp`

[打开源码](../src/apps/remote/RemoteCProductMain.cpp) · **文件作用：** 实现 remote c product main 相关函数与文件级辅助逻辑。

### 类型

| 行 | 类型 | 种类 | 作用 |
|---:|---|---|---|
| [L112](../src/apps/remote/RemoteCProductMain.cpp#L112) | `RemoteCToolTipBubble` | class | 定义 RemoteCToolTipBubble 的 class 类型和相关状态。 |
| [L203](../src/apps/remote/RemoteCProductMain.cpp#L203) | `RemoteCToolTipController` | class | 定义 RemoteCToolTipController 的 class 类型和相关状态。 |
| [L263](../src/apps/remote/RemoteCProductMain.cpp#L263) | `StartupSignalingConfiguration` | struct | 定义 StartupSignalingConfiguration 的 struct 类型和相关状态。 |
| [L608](../src/apps/remote/RemoteCProductMain.cpp#L608) | `ScopedWinHandle` | class | 定义 ScopedWinHandle 的 class 类型和相关状态。 |

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L70](../src/apps/remote/RemoteCProductMain.cpp#L70) | `RegisterBundledFont` | 定义 | `QString RegisterBundledFont(const QString& resourcePath, const QString& fallbackFamily)` | 实现 register bundled font 对应的业务或工具逻辑。 |
| [L82](../src/apps/remote/RemoteCProductMain.cpp#L82) | `InstalledApplicationVersion` | 定义 | `QString InstalledApplicationVersion()` | 实现 installed application version 对应的业务或工具逻辑。 |
| [L114](../src/apps/remote/RemoteCProductMain.cpp#L114) | `RemoteCToolTipBubble` | 定义 | `RemoteCToolTipBubble() : QWidget(nullptr, Qt::ToolTip \| Qt::FramelessWindowHint \| Qt::NoDropShadowWindowHint)` | 实现 remote c tool tip bubble 对应的业务或工具逻辑。 |
| [L147](../src/apps/remote/RemoteCProductMain.cpp#L147) | `ShowText` | 定义 | `void ShowText(const QString& text, const QPoint& globalPosition)` | 实现 show text 对应的业务或工具逻辑。 |
| [L208](../src/apps/remote/RemoteCProductMain.cpp#L208) | `eventFilter` | 定义 | `bool eventFilter(QObject* watched, QEvent* event) override` | 实现 event filter 对应的业务或工具逻辑。 |
| [L257](../src/apps/remote/RemoteCProductMain.cpp#L257) | `ToUtf8` | 定义 | `std::string ToUtf8(const QString& value)` | 实现 to utf8 对应的业务或工具逻辑。 |
| [L278](../src/apps/remote/RemoteCProductMain.cpp#L278) | `InvalidStartupConfiguration` | 定义 | `StartupSignalingConfiguration InvalidStartupConfiguration( const QString& source, const QString& error, bool authenticationRequired = false)` | 实现 invalid startup configuration 对应的业务或工具逻辑。 |
| [L291](../src/apps/remote/RemoteCProductMain.cpp#L291) | `LoadOrCreateLocalDeviceId` | 定义 | `QString LoadOrCreateLocalDeviceId()` | 读取或恢复 load or create local device id 相关逻辑。 |
| [L307](../src/apps/remote/RemoteCProductMain.cpp#L307) | `GenerateSessionVerificationCode` | 定义 | `QString GenerateSessionVerificationCode()` | 实现 generate session verification code 对应的业务或工具逻辑。 |
| [L313](../src/apps/remote/RemoteCProductMain.cpp#L313) | `LoadStartupSignalingConfiguration` | 定义 | `StartupSignalingConfiguration LoadStartupSignalingConfiguration()` | 读取或恢复 load startup signaling configuration 相关逻辑。 |
| [L595](../src/apps/remote/RemoteCProductMain.cpp#L595) | `SingleInstanceServerName` | 定义 | `QString SingleInstanceServerName()` | 实现 single instance server name 对应的业务或工具逻辑。 |
| [L610](../src/apps/remote/RemoteCProductMain.cpp#L610) | `ScopedWinHandle` | 定义 | `explicit ScopedWinHandle(HANDLE handle = nullptr) : handle_(handle) {}` | 实现 scoped win handle 对应的业务或工具逻辑。 |
| [L611](../src/apps/remote/RemoteCProductMain.cpp#L611) | `~ScopedWinHandle` | 定义 | `~ScopedWinHandle()` | 停止相关活动并释放 ScopedWinHandle 实例拥有的资源。 |
| [L620](../src/apps/remote/RemoteCProductMain.cpp#L620) | `ScopedWinHandle` | 定义 | `ScopedWinHandle(ScopedWinHandle&& other) noexcept : handle_(other.handle_)` | 实现 scoped win handle 对应的业务或工具逻辑。 |
| [L637](../src/apps/remote/RemoteCProductMain.cpp#L637) | `get` | 定义 | `HANDLE get() const { return handle_; }` | 查询并返回 get 相关逻辑。 |
| [L643](../src/apps/remote/RemoteCProductMain.cpp#L643) | `IsUtilityInvocation` | 定义 | `bool IsUtilityInvocation(const QStringList& arguments)` | 判断 is utility invocation 相关逻辑。 |
| [L664](../src/apps/remote/RemoteCProductMain.cpp#L664) | `NotifyRunningInstance` | 定义 | `bool NotifyRunningInstance(const QString& serverName)` | 通知或报告 notify running instance 相关逻辑。 |
| [L679](../src/apps/remote/RemoteCProductMain.cpp#L679) | `WriteCurrentUserRegistryString` | 定义 | `bool WriteCurrentUserRegistryString( const wchar_t* subkey, const wchar_t* valueName, const QString& value)` | 保存或写入 write current user registry string 相关逻辑。 |
| [L700](../src/apps/remote/RemoteCProductMain.cpp#L700) | `EnsureRemoteCUrlProtocolRegistration` | 定义 | `bool EnsureRemoteCUrlProtocolRegistration()` | 实现 ensure remote c url protocol registration 对应的业务或工具逻辑。 |
| [L719](../src/apps/remote/RemoteCProductMain.cpp#L719) | `ConfiguredVideoEncoderPreference` | 定义 | `remote::VideoEncoderPreference ConfiguredVideoEncoderPreference()` | 更新或应用 configured video encoder preference 相关逻辑。 |
| [L739](../src/apps/remote/RemoteCProductMain.cpp#L739) | `ConfiguredFfmpegHardwareBackend` | 定义 | `remote::FfmpegHardwareBackend ConfiguredFfmpegHardwareBackend()` | 更新或应用 configured ffmpeg hardware backend 相关逻辑。 |
| [L756](../src/apps/remote/RemoteCProductMain.cpp#L756) | `ConfiguredFfmpegX264Preset` | 定义 | `remote::FfmpegX264Preset ConfiguredFfmpegX264Preset()` | 更新或应用 configured ffmpeg x264 preset 相关逻辑。 |
| [L807](../src/apps/remote/RemoteCProductMain.cpp#L807) | `ConfiguredVideoDecoderPreference` | 定义 | `remote::VideoDecoderPreference ConfiguredVideoDecoderPreference()` | 更新或应用 configured video decoder preference 相关逻辑。 |
| [L849](../src/apps/remote/RemoteCProductMain.cpp#L849) | `ConfiguredDesktopCaptureImplementation` | 定义 | `remote::DesktopCaptureImplementation ConfiguredDesktopCaptureImplementation()` | 更新或应用 configured desktop capture implementation 相关逻辑。 |
| [L860](../src/apps/remote/RemoteCProductMain.cpp#L860) | `CreateSessionEngine` | 定义 | `std::unique_ptr<remote::app::InProcessSessionEngine> CreateSessionEngine( const StartupSignalingConfiguration& startupConfiguration, const QString& deviceVerificationCode)` | 创建或初始化 create session engine 相关逻辑。 |
| [L1030](../src/apps/remote/RemoteCProductMain.cpp#L1030) | `RunSignalingPolicySelfTest` | 定义 | `int RunSignalingPolicySelfTest()` | 执行后台循环或调度 run signaling policy self test 相关逻辑。 |
| [L1067](../src/apps/remote/RemoteCProductMain.cpp#L1067) | `RunAuthCoordinatorSelfTest` | 定义 | `int RunAuthCoordinatorSelfTest()` | 执行后台循环或调度 run auth coordinator self test 相关逻辑。 |
| [L1137](../src/apps/remote/RemoteCProductMain.cpp#L1137) | `main` | 定义 | `int main(int argc, char* argv[])` | 实现 main 对应的业务或工具逻辑。 |
