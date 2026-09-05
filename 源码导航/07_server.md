# 信令服务器

> 自动生成于 2026-09-06，源码树 `1ae783c8db12-dirty`。请运行 `tools/Generate-SourceSymbolReference.ps1` 刷新。

WSS 认证、设备注册、direct session、协作房间、持久化、限流和诊断。

本册共收录 29 个源码文件。函数与变量的中文作用优先采用源码紧邻注释；无注释时根据符号命名生成阅读提示，最终语义仍以源码为准。

## `src/server/auth/AuthTypes.h`

[打开源码](../src/server/auth/AuthTypes.h) · **文件作用：** 声明 auth types 相关类型、接口、配置和成员状态。

### 类型

| 行 | 类型 | 种类 | 作用 |
|---:|---|---|---|
| [L10](../src/server/auth/AuthTypes.h#L10) | `UserInfoClaims` | struct | 定义 UserInfoClaims 的 struct 类型和相关状态。 |
| [L17](../src/server/auth/AuthTypes.h#L17) | `UserInfoStatus` | enum class | 定义 UserInfoStatus 的 enum class 类型和相关状态。 |
| [L25](../src/server/auth/AuthTypes.h#L25) | `UserInfoResult` | struct | 定义 UserInfoResult 的 struct 类型和相关状态。 |

### 成员与文件级变量

| 行 | 变量 | 声明 | 作用 |
|---:|---|---|---|
| [L11](../src/server/auth/AuthTypes.h#L11) | `subject` | `QString subject;` | 保存 subject 相关配置或运行状态。 |
| [L12](../src/server/auth/AuthTypes.h#L12) | `username` | `QString username;` | 保存路径、地址或显示名称：username。 |
| [L13](../src/server/auth/AuthTypes.h#L13) | `displayName` | `QString displayName;` | 保存路径、地址或显示名称：display name。 |
| [L14](../src/server/auth/AuthTypes.h#L14) | `email` | `QString email;` | 保存 email 相关配置或运行状态。 |
| [L26](../src/server/auth/AuthTypes.h#L26) | `status` | `UserInfoStatus status = UserInfoStatus::kInvalidResponse;` | 保存状态机当前状态：status。 |
| [L27](../src/server/auth/AuthTypes.h#L27) | `claims` | `UserInfoClaims claims;` | 保存 claims 相关配置或运行状态。 |
| [L28](../src/server/auth/AuthTypes.h#L28) | `code` | `QString code;` | 保存 code 相关配置或运行状态。 |
| [L29](../src/server/auth/AuthTypes.h#L29) | `message` | `QString message;` | 保存 message 相关配置或运行状态。 |
| [L30](../src/server/auth/AuthTypes.h#L30) | `retryable` | `bool retryable = false;` | 保存 retryable 相关配置或运行状态。 |

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L32](../src/server/auth/AuthTypes.h#L32) | `authenticated` | 定义 | `bool authenticated() const` | 实现 authenticated 对应的业务或工具逻辑。 |

## `src/server/auth/LogtoManagementClient.cpp`

[打开源码](../src/server/auth/LogtoManagementClient.cpp) · **文件作用：** 实现 logto management client 相关函数与文件级辅助逻辑。

### 成员与文件级变量

| 行 | 变量 | 声明 | 作用 |
|---:|---|---|---|
| [L18](../src/server/auth/LogtoManagementClient.cpp#L18) | `kMaximumResponseBytes` | `constexpr qsizetype kMaximumResponseBytes = 64 * 1024;` | 定义 maximum response bytes 的编译期常量或产品边界。 |

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L20](../src/server/auth/LogtoManagementClient.cpp#L20) | `SetError` | 定义 | `void SetError(QString* errorMessage, const QString& message)` | 更新或应用 set error 相关逻辑。 |
| [L26](../src/server/auth/LogtoManagementClient.cpp#L26) | `Failure` | 定义 | `UserDeletionResult Failure(UserDeletionStatus status, QString code, QString message, bool retryable)` | 实现 failure 对应的业务或工具逻辑。 |
| [L40](../src/server/auth/LogtoManagementClient.cpp#L40) | `LogtoManagementClient::LogtoManagementClient` | 定义 | `LogtoManagementClient::LogtoManagementClient(QObject* parent) : QObject(parent), networkManager_(std::make_unique<QNetworkAccessManager>(this)) {}` | 构造并初始化 LogtoManagementClient 实例。 |
| [L44](../src/server/auth/LogtoManagementClient.cpp#L44) | `LogtoManagementClient::~LogtoManagementClient` | 定义 | `LogtoManagementClient::~LogtoManagementClient()` | 停止相关活动并释放 LogtoManagementClient 实例拥有的资源。 |
| [L49](../src/server/auth/LogtoManagementClient.cpp#L49) | `LogtoManagementClient::Configure` | 定义 | `bool LogtoManagementClient::Configure( const QUrl& issuer, QString clientId, QByteArray clientSecret, int timeoutMs, QString* errorMessage)` | 更新或应用 configure 相关逻辑。 |
| [L83](../src/server/auth/LogtoManagementClient.cpp#L83) | `LogtoManagementClient::IsConfigured` | 定义 | `bool LogtoManagementClient::IsConfigured() const` | 判断 is configured 相关逻辑。 |
| [L88](../src/server/auth/LogtoManagementClient.cpp#L88) | `LogtoManagementClient::DeleteUser` | 定义 | `void LogtoManagementClient::DeleteUser( const QString& subject, Completion completion)` | 实现 delete user 对应的业务或工具逻辑。 |
| [L107](../src/server/auth/LogtoManagementClient.cpp#L107) | `LogtoManagementClient::CancelAll` | 定义 | `void LogtoManagementClient::CancelAll()` | 判断 cancel all 相关逻辑。 |
| [L132](../src/server/auth/LogtoManagementClient.cpp#L132) | `LogtoManagementClient::StartNext` | 定义 | `void LogtoManagementClient::StartNext()` | 启动 start next 相关逻辑。 |
| [L145](../src/server/auth/LogtoManagementClient.cpp#L145) | `LogtoManagementClient::RequestAccessToken` | 定义 | `void LogtoManagementClient::RequestAccessToken()` | 发起请求或查询 request access token 相关逻辑。 |
| [L170](../src/server/auth/LogtoManagementClient.cpp#L170) | `LogtoManagementClient::FinishTokenRequest` | 定义 | `void LogtoManagementClient::FinishTokenRequest(QNetworkReply* reply)` | 停止 finish token request 相关逻辑。 |
| [L215](../src/server/auth/LogtoManagementClient.cpp#L215) | `LogtoManagementClient::StartDeleteRequest` | 定义 | `void LogtoManagementClient::StartDeleteRequest()` | 启动 start delete request 相关逻辑。 |
| [L233](../src/server/auth/LogtoManagementClient.cpp#L233) | `LogtoManagementClient::FinishDeleteRequest` | 定义 | `void LogtoManagementClient::FinishDeleteRequest(QNetworkReply* reply)` | 停止 finish delete request 相关逻辑。 |
| [L279](../src/server/auth/LogtoManagementClient.cpp#L279) | `LogtoManagementClient::CompleteCurrent` | 定义 | `void LogtoManagementClient::CompleteCurrent(UserDeletionResult result)` | 实现 complete current 对应的业务或工具逻辑。 |

## `src/server/auth/LogtoManagementClient.h`

[打开源码](../src/server/auth/LogtoManagementClient.h) · **文件作用：** 声明 logto management client 相关类型、接口、配置和成员状态。

### 类型

| 行 | 类型 | 种类 | 作用 |
|---:|---|---|---|
| [L17](../src/server/auth/LogtoManagementClient.h#L17) | `QNetworkAccessManager` | class | 定义 QNetworkAccessManager 的 class 类型和相关状态。 |
| [L22](../src/server/auth/LogtoManagementClient.h#L22) | `UserDeletionStatus` | enum class | 定义 UserDeletionStatus 的 enum class 类型和相关状态。 |
| [L31](../src/server/auth/LogtoManagementClient.h#L31) | `UserDeletionResult` | struct | 定义 UserDeletionResult 的 struct 类型和相关状态。 |
| [L43](../src/server/auth/LogtoManagementClient.h#L43) | `LogtoManagementClient` | class | 定义 LogtoManagementClient 的 class 类型和相关状态。 |
| [L60](../src/server/auth/LogtoManagementClient.h#L60) | `PendingDeletion` | struct | 定义 PendingDeletion 的 struct 类型和相关状态。 |

### 成员与文件级变量

| 行 | 变量 | 声明 | 作用 |
|---:|---|---|---|
| [L17](../src/server/auth/LogtoManagementClient.h#L17) | `QNetworkAccessManager` | `class QNetworkAccessManager;` | 保存 q network access manager 相关配置或运行状态。 |
| [L32](../src/server/auth/LogtoManagementClient.h#L32) | `status` | `UserDeletionStatus status = UserDeletionStatus::kRejected;` | 保存状态机当前状态：status。 |
| [L33](../src/server/auth/LogtoManagementClient.h#L33) | `code` | `QString code;` | 保存 code 相关配置或运行状态。 |
| [L34](../src/server/auth/LogtoManagementClient.h#L34) | `message` | `QString message;` | 保存 message 相关配置或运行状态。 |
| [L35](../src/server/auth/LogtoManagementClient.h#L35) | `retryable` | `bool retryable = false;` | 保存 retryable 相关配置或运行状态。 |
| [L61](../src/server/auth/LogtoManagementClient.h#L61) | `subject` | `QString subject;` | 保存 subject 相关配置或运行状态。 |
| [L62](../src/server/auth/LogtoManagementClient.h#L62) | `completion` | `Completion completion;` | 保存 completion 相关配置或运行状态。 |
| [L72](../src/server/auth/LogtoManagementClient.h#L72) | `networkManager_` | `std::unique_ptr<QNetworkAccessManager> networkManager_;` | 保存 network manager 相关配置或运行状态。 |
| [L73](../src/server/auth/LogtoManagementClient.h#L73) | `queue_` | `QQueue<PendingDeletion> queue_;` | 保存待处理队列或请求：queue。 |
| [L74](../src/server/auth/LogtoManagementClient.h#L74) | `current_` | `std::unique_ptr<PendingDeletion> current_;` | 保存 current 相关配置或运行状态。 |
| [L75](../src/server/auth/LogtoManagementClient.h#L75) | `activeReply_` | `QNetworkReply* activeReply_ = nullptr;` | 保存 active reply 相关配置或运行状态。 |
| [L76](../src/server/auth/LogtoManagementClient.h#L76) | `tokenEndpoint_` | `QUrl tokenEndpoint_;` | 保存 token endpoint 相关配置或运行状态。 |
| [L77](../src/server/auth/LogtoManagementClient.h#L77) | `managementEndpoint_` | `QUrl managementEndpoint_;` | 保存 management endpoint 相关配置或运行状态。 |
| [L78](../src/server/auth/LogtoManagementClient.h#L78) | `clientId_` | `QString clientId_;` | 保存身份或作用域标识：client id。 |
| [L79](../src/server/auth/LogtoManagementClient.h#L79) | `clientSecret_` | `QByteArray clientSecret_;` | 保存 client secret 相关配置或运行状态。 |
| [L80](../src/server/auth/LogtoManagementClient.h#L80) | `accessToken_` | `QByteArray accessToken_;` | 保存 access token 相关配置或运行状态。 |
| [L81](../src/server/auth/LogtoManagementClient.h#L81) | `accessTokenExpiresAtSeconds_` | `qint64 accessTokenExpiresAtSeconds_ = 0;` | 保存 access token expires at seconds 相关配置或运行状态。 |
| [L82](../src/server/auth/LogtoManagementClient.h#L82) | `timeoutMs_` | `int timeoutMs_ = 10000;` | 保存 timeout ms 相关配置或运行状态。 |
| [L83](../src/server/auth/LogtoManagementClient.h#L83) | `cancelling_` | `bool cancelling_ = false;` | 保存 cancelling 相关配置或运行状态。 |

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L37](../src/server/auth/LogtoManagementClient.h#L37) | `deleted` | 定义 | `bool deleted() const` | 实现 deleted 对应的业务或工具逻辑。 |
| [L47](../src/server/auth/LogtoManagementClient.h#L47) | `LogtoManagementClient` | 声明 | `explicit LogtoManagementClient(QObject* parent = nullptr)` | 实现 logto management client 对应的业务或工具逻辑。 |
| [L48](../src/server/auth/LogtoManagementClient.h#L48) | `~LogtoManagementClient` | 声明 | `~LogtoManagementClient() override` | 停止相关活动并释放 LogtoManagementClient 实例拥有的资源。 |
| [L50](../src/server/auth/LogtoManagementClient.h#L50) | `Configure` | 声明 | `bool Configure(const QUrl& issuer, QString clientId, QByteArray clientSecret, int timeoutMs, QString* errorMessage = nullptr)` | 更新或应用 configure 相关逻辑。 |
| [L55](../src/server/auth/LogtoManagementClient.h#L55) | `IsConfigured` | 声明 | `bool IsConfigured() const` | 判断 is configured 相关逻辑。 |
| [L56](../src/server/auth/LogtoManagementClient.h#L56) | `DeleteUser` | 声明 | `void DeleteUser(const QString& subject, Completion completion)` | 实现 delete user 对应的业务或工具逻辑。 |
| [L57](../src/server/auth/LogtoManagementClient.h#L57) | `CancelAll` | 声明 | `void CancelAll()` | 判断 cancel all 相关逻辑。 |
| [L65](../src/server/auth/LogtoManagementClient.h#L65) | `StartNext` | 声明 | `void StartNext()` | 启动 start next 相关逻辑。 |
| [L66](../src/server/auth/LogtoManagementClient.h#L66) | `RequestAccessToken` | 声明 | `void RequestAccessToken()` | 发起请求或查询 request access token 相关逻辑。 |
| [L67](../src/server/auth/LogtoManagementClient.h#L67) | `FinishTokenRequest` | 声明 | `void FinishTokenRequest(QNetworkReply* reply)` | 停止 finish token request 相关逻辑。 |
| [L68](../src/server/auth/LogtoManagementClient.h#L68) | `StartDeleteRequest` | 声明 | `void StartDeleteRequest()` | 启动 start delete request 相关逻辑。 |
| [L69](../src/server/auth/LogtoManagementClient.h#L69) | `FinishDeleteRequest` | 声明 | `void FinishDeleteRequest(QNetworkReply* reply)` | 停止 finish delete request 相关逻辑。 |
| [L70](../src/server/auth/LogtoManagementClient.h#L70) | `CompleteCurrent` | 声明 | `void CompleteCurrent(UserDeletionResult result)` | 实现 complete current 对应的业务或工具逻辑。 |

## `src/server/auth/LogtoUserInfoClient.cpp`

[打开源码](../src/server/auth/LogtoUserInfoClient.cpp) · **文件作用：** 实现 logto user info client 相关函数与文件级辅助逻辑。

### 类型

| 行 | 类型 | 种类 | 作用 |
|---:|---|---|---|
| [L39](../src/server/auth/LogtoUserInfoClient.cpp#L39) | `LogtoUserInfoClient::PendingRequest` | struct | 定义 LogtoUserInfoClient::PendingRequest 的 struct 类型和相关状态。 |

### 成员与文件级变量

| 行 | 变量 | 声明 | 作用 |
|---:|---|---|---|
| [L17](../src/server/auth/LogtoUserInfoClient.cpp#L17) | `kMaximumUserInfoBytes` | `constexpr qsizetype kMaximumUserInfoBytes = 64 * 1024;` | 定义 maximum user info bytes 的编译期常量或产品边界。 |

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L19](../src/server/auth/LogtoUserInfoClient.cpp#L19) | `SetError` | 定义 | `void SetError(QString* errorMessage, const QString& message)` | 更新或应用 set error 相关逻辑。 |
| [L25](../src/server/auth/LogtoUserInfoClient.cpp#L25) | `Failure` | 定义 | `UserInfoResult Failure(UserInfoStatus status, QString code, QString message, bool retryable)` | 实现 failure 对应的业务或工具逻辑。 |
| [L47](../src/server/auth/LogtoUserInfoClient.cpp#L47) | `LogtoUserInfoClient::LogtoUserInfoClient` | 定义 | `LogtoUserInfoClient::LogtoUserInfoClient(QObject* parent) : QObject(parent), networkManager_(std::make_unique<QNetworkAccessManager>(this)) {}` | 构造并初始化 LogtoUserInfoClient 实例。 |
| [L51](../src/server/auth/LogtoUserInfoClient.cpp#L51) | `LogtoUserInfoClient::~LogtoUserInfoClient` | 定义 | `LogtoUserInfoClient::~LogtoUserInfoClient()` | 停止相关活动并释放 LogtoUserInfoClient 实例拥有的资源。 |
| [L55](../src/server/auth/LogtoUserInfoClient.cpp#L55) | `LogtoUserInfoClient::Configure` | 定义 | `bool LogtoUserInfoClient::Configure( const QUrl& issuer, int timeoutMs, QString* errorMessage)` | 更新或应用 configure 相关逻辑。 |
| [L96](../src/server/auth/LogtoUserInfoClient.cpp#L96) | `LogtoUserInfoClient::Fetch` | 定义 | `QNetworkReply* LogtoUserInfoClient::Fetch( const QByteArray& accessToken, Completion completion)` | 实现 fetch 对应的业务或工具逻辑。 |
| [L153](../src/server/auth/LogtoUserInfoClient.cpp#L153) | `LogtoUserInfoClient::Cancel` | 定义 | `void LogtoUserInfoClient::Cancel(QNetworkReply* reply)` | 判断 cancel 相关逻辑。 |
| [L163](../src/server/auth/LogtoUserInfoClient.cpp#L163) | `LogtoUserInfoClient::CancelAll` | 定义 | `void LogtoUserInfoClient::CancelAll()` | 判断 cancel all 相关逻辑。 |
| [L170](../src/server/auth/LogtoUserInfoClient.cpp#L170) | `LogtoUserInfoClient::ParseResponseForTesting` | 定义 | `UserInfoResult LogtoUserInfoClient::ParseResponseForTesting( int httpStatus, const QByteArray& body, QNetworkReply::NetworkError networkError, bool timedOut, bool tooLarge)` | 解码或解析 parse response for testing 相关逻辑。 |
| [L180](../src/server/auth/LogtoUserInfoClient.cpp#L180) | `LogtoUserInfoClient::ParseResponse` | 定义 | `UserInfoResult LogtoUserInfoClient::ParseResponse( int httpStatus, const QByteArray& body, QNetworkReply::NetworkError networkError, bool timedOut, bool tooLarge)` | 解码或解析 parse response 相关逻辑。 |
| [L262](../src/server/auth/LogtoUserInfoClient.cpp#L262) | `LogtoUserInfoClient::Finish` | 定义 | `void LogtoUserInfoClient::Finish(QNetworkReply* reply)` | 停止 finish 相关逻辑。 |

## `src/server/auth/LogtoUserInfoClient.h`

[打开源码](../src/server/auth/LogtoUserInfoClient.h) · **文件作用：** 声明 logto user info client 相关类型、接口、配置和成员状态。

### 类型

| 行 | 类型 | 种类 | 作用 |
|---:|---|---|---|
| [L18](../src/server/auth/LogtoUserInfoClient.h#L18) | `QNetworkAccessManager` | class | 定义 QNetworkAccessManager 的 class 类型和相关状态。 |
| [L23](../src/server/auth/LogtoUserInfoClient.h#L23) | `LogtoUserInfoClient` | class | 定义 LogtoUserInfoClient 的 class 类型和相关状态。 |
| [L49](../src/server/auth/LogtoUserInfoClient.h#L49) | `PendingRequest` | struct | 定义 PendingRequest 的 struct 类型和相关状态。 |

### 成员与文件级变量

| 行 | 变量 | 声明 | 作用 |
|---:|---|---|---|
| [L18](../src/server/auth/LogtoUserInfoClient.h#L18) | `QNetworkAccessManager` | `class QNetworkAccessManager;` | 保存 q network access manager 相关配置或运行状态。 |
| [L49](../src/server/auth/LogtoUserInfoClient.h#L49) | `PendingRequest` | `struct PendingRequest;` | 保存 pending request 相关配置或运行状态。 |
| [L59](../src/server/auth/LogtoUserInfoClient.h#L59) | `networkManager_` | `std::unique_ptr<QNetworkAccessManager> networkManager_;` | 保存 network manager 相关配置或运行状态。 |
| [L60](../src/server/auth/LogtoUserInfoClient.h#L60) | `pending_` | `QHash<QNetworkReply*, std::shared_ptr<PendingRequest>> pending_;` | 保存待处理队列或请求：pending。 |
| [L61](../src/server/auth/LogtoUserInfoClient.h#L61) | `userInfoEndpoint_` | `QUrl userInfoEndpoint_;` | 保存 user info endpoint 相关配置或运行状态。 |
| [L62](../src/server/auth/LogtoUserInfoClient.h#L62) | `timeoutMs_` | `int timeoutMs_ = 10000;` | 保存 timeout ms 相关配置或运行状态。 |

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L27](../src/server/auth/LogtoUserInfoClient.h#L27) | `LogtoUserInfoClient` | 声明 | `explicit LogtoUserInfoClient(QObject* parent = nullptr)` | 实现 logto user info client 对应的业务或工具逻辑。 |
| [L28](../src/server/auth/LogtoUserInfoClient.h#L28) | `~LogtoUserInfoClient` | 声明 | `~LogtoUserInfoClient() override` | 停止相关活动并释放 LogtoUserInfoClient 实例拥有的资源。 |
| [L30](../src/server/auth/LogtoUserInfoClient.h#L30) | `Configure` | 声明 | `bool Configure(const QUrl& issuer, int timeoutMs, QString* errorMessage = nullptr)` | 更新或应用 configure 相关逻辑。 |
| [L33](../src/server/auth/LogtoUserInfoClient.h#L33) | `Fetch` | 声明 | `QNetworkReply* Fetch(const QByteArray& accessToken, Completion completion)` | 实现 fetch 对应的业务或工具逻辑。 |
| [L35](../src/server/auth/LogtoUserInfoClient.h#L35) | `Cancel` | 声明 | `void Cancel(QNetworkReply* reply)` | 判断 cancel 相关逻辑。 |
| [L36](../src/server/auth/LogtoUserInfoClient.h#L36) | `CancelAll` | 声明 | `void CancelAll()` | 判断 cancel all 相关逻辑。 |
| [L38](../src/server/auth/LogtoUserInfoClient.h#L38) | `userInfoEndpoint` | 定义 | `QUrl userInfoEndpoint() const { return userInfoEndpoint_; }` | 实现 user info endpoint 对应的业务或工具逻辑。 |
| [L40](../src/server/auth/LogtoUserInfoClient.h#L40) | `ParseResponseForTesting` | 声明 | `static UserInfoResult ParseResponseForTesting( int httpStatus, const QByteArray& body, QNetworkReply::NetworkError networkError = QNetworkReply::NoError, bool timedOut = false, bool tooLarge = false)` | 解码或解析 parse response for testing 相关逻辑。 |
| [L51](../src/server/auth/LogtoUserInfoClient.h#L51) | `ParseResponse` | 声明 | `static UserInfoResult ParseResponse( int httpStatus, const QByteArray& body, QNetworkReply::NetworkError networkError, bool timedOut, bool tooLarge)` | 解码或解析 parse response 相关逻辑。 |
| [L57](../src/server/auth/LogtoUserInfoClient.h#L57) | `Finish` | 声明 | `void Finish(QNetworkReply* reply)` | 停止 finish 相关逻辑。 |

## `src/server/auth/LogtoWebhookServer.cpp`

[打开源码](../src/server/auth/LogtoWebhookServer.cpp) · **文件作用：** 实现 logto webhook server 相关函数与文件级辅助逻辑。

### 成员与文件级变量

| 行 | 变量 | 声明 | 作用 |
|---:|---|---|---|
| [L14](../src/server/auth/LogtoWebhookServer.cpp#L14) | `kMaximumWebhookBytes` | `constexpr qsizetype kMaximumWebhookBytes = 64 * 1024;` | 定义 maximum webhook bytes 的编译期常量或产品边界。 |

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L16](../src/server/auth/LogtoWebhookServer.cpp#L16) | `SetError` | 定义 | `void SetError(QString* errorMessage, const QString& message)` | 更新或应用 set error 相关逻辑。 |
| [L22](../src/server/auth/LogtoWebhookServer.cpp#L22) | `JsonResponse` | 定义 | `QHttpServerResponse JsonResponse( QHttpServerResponse::StatusCode status, const QString& code)` | 实现 json response 对应的业务或工具逻辑。 |
| [L34](../src/server/auth/LogtoWebhookServer.cpp#L34) | `LogtoWebhookServer::LogtoWebhookServer` | 定义 | `LogtoWebhookServer::LogtoWebhookServer()` | 构造并初始化 LogtoWebhookServer 实例。 |
| [L44](../src/server/auth/LogtoWebhookServer.cpp#L44) | `LogtoWebhookServer::~LogtoWebhookServer` | 定义 | `LogtoWebhookServer::~LogtoWebhookServer()` | 停止相关活动并释放 LogtoWebhookServer 实例拥有的资源。 |
| [L48](../src/server/auth/LogtoWebhookServer.cpp#L48) | `LogtoWebhookServer::Start` | 定义 | `bool LogtoWebhookServer::Start( const QHostAddress& listenAddress, quint16 port, QByteArray signingKey, UserDeletedHandler handler, QString* errorMessage)` | 启动 start 相关逻辑。 |
| [L85](../src/server/auth/LogtoWebhookServer.cpp#L85) | `LogtoWebhookServer::Stop` | 定义 | `void LogtoWebhookServer::Stop()` | 停止 stop 相关逻辑。 |
| [L92](../src/server/auth/LogtoWebhookServer.cpp#L92) | `LogtoWebhookServer::IsListening` | 定义 | `bool LogtoWebhookServer::IsListening() const` | 判断 is listening 相关逻辑。 |
| [L96](../src/server/auth/LogtoWebhookServer.cpp#L96) | `LogtoWebhookServer::ServerPort` | 定义 | `quint16 LogtoWebhookServer::ServerPort() const` | 实现 server port 对应的业务或工具逻辑。 |
| [L100](../src/server/auth/LogtoWebhookServer.cpp#L100) | `LogtoWebhookServer::ConstantTimeEquals` | 定义 | `bool LogtoWebhookServer::ConstantTimeEquals( const QByteArray& first, const QByteArray& second)` | 实现 constant time equals 对应的业务或工具逻辑。 |
| [L114](../src/server/auth/LogtoWebhookServer.cpp#L114) | `LogtoWebhookServer::VerifySignatureForTesting` | 定义 | `bool LogtoWebhookServer::VerifySignatureForTesting( const QByteArray& body, const QByteArray& signingKey, const QByteArray& signature)` | 校验 verify signature for testing 相关逻辑。 |
| [L127](../src/server/auth/LogtoWebhookServer.cpp#L127) | `LogtoWebhookServer::HandleRequest` | 定义 | `QHttpServerResponse LogtoWebhookServer::HandleRequest( const QHttpServerRequest& request)` | 接收并处理 handle request 相关逻辑。 |

## `src/server/auth/LogtoWebhookServer.h`

[打开源码](../src/server/auth/LogtoWebhookServer.h) · **文件作用：** 声明 logto webhook server 相关类型、接口、配置和成员状态。

### 类型

| 行 | 类型 | 种类 | 作用 |
|---:|---|---|---|
| [L15](../src/server/auth/LogtoWebhookServer.h#L15) | `LogtoWebhookServer` | class | 定义 LogtoWebhookServer 的 class 类型和相关状态。 |

### 成员与文件级变量

| 行 | 变量 | 声明 | 作用 |
|---:|---|---|---|
| [L44](../src/server/auth/LogtoWebhookServer.h#L44) | `tcpServer_` | `QTcpServer tcpServer_;` | 保存 tcp server 相关配置或运行状态。 |
| [L45](../src/server/auth/LogtoWebhookServer.h#L45) | `httpServer_` | `QHttpServer httpServer_;` | 保存 http server 相关配置或运行状态。 |
| [L46](../src/server/auth/LogtoWebhookServer.h#L46) | `signingKey_` | `QByteArray signingKey_;` | 保存 signing key 相关配置或运行状态。 |
| [L47](../src/server/auth/LogtoWebhookServer.h#L47) | `userDeletedHandler_` | `UserDeletedHandler userDeletedHandler_;` | 保存回调或观察者入口：user deleted handler。 |

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L20](../src/server/auth/LogtoWebhookServer.h#L20) | `LogtoWebhookServer` | 声明 | `LogtoWebhookServer()` | 实现 logto webhook server 对应的业务或工具逻辑。 |
| [L21](../src/server/auth/LogtoWebhookServer.h#L21) | `~LogtoWebhookServer` | 声明 | `~LogtoWebhookServer()` | 停止相关活动并释放 LogtoWebhookServer 实例拥有的资源。 |
| [L23](../src/server/auth/LogtoWebhookServer.h#L23) | `LogtoWebhookServer` | 声明 | `LogtoWebhookServer(const LogtoWebhookServer&) = delete` | 实现 logto webhook server 对应的业务或工具逻辑。 |
| [L26](../src/server/auth/LogtoWebhookServer.h#L26) | `Start` | 声明 | `bool Start(const QHostAddress& listenAddress, quint16 port, QByteArray signingKey, UserDeletedHandler handler, QString* errorMessage = nullptr)` | 启动 start 相关逻辑。 |
| [L31](../src/server/auth/LogtoWebhookServer.h#L31) | `Stop` | 声明 | `void Stop()` | 停止 stop 相关逻辑。 |
| [L32](../src/server/auth/LogtoWebhookServer.h#L32) | `IsListening` | 声明 | `bool IsListening() const` | 判断 is listening 相关逻辑。 |
| [L33](../src/server/auth/LogtoWebhookServer.h#L33) | `ServerPort` | 声明 | `quint16 ServerPort() const` | 实现 server port 对应的业务或工具逻辑。 |
| [L35](../src/server/auth/LogtoWebhookServer.h#L35) | `VerifySignatureForTesting` | 声明 | `static bool VerifySignatureForTesting(const QByteArray& body, const QByteArray& signingKey, const QByteArray& signature)` | 校验 verify signature for testing 相关逻辑。 |
| [L40](../src/server/auth/LogtoWebhookServer.h#L40) | `HandleRequest` | 声明 | `QHttpServerResponse HandleRequest(const QHttpServerRequest& request)` | 接收并处理 handle request 相关逻辑。 |
| [L41](../src/server/auth/LogtoWebhookServer.h#L41) | `ConstantTimeEquals` | 声明 | `static bool ConstantTimeEquals(const QByteArray& first, const QByteArray& second)` | 实现 constant time equals 对应的业务或工具逻辑。 |

## `src/server/persistence/IdentityStore.cpp`

[打开源码](../src/server/persistence/IdentityStore.cpp) · **文件作用：** 实现 identity store 相关函数与文件级辅助逻辑。

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L24](../src/server/persistence/IdentityStore.cpp#L24) | `SetError` | 定义 | `void SetError(QString* errorMessage, const QString& message)` | 更新或应用 set error 相关逻辑。 |
| [L30](../src/server/persistence/IdentityStore.cpp#L30) | `QueryError` | 定义 | `QString QueryError(const QString& operation, const QSqlQuery& query)` | 发起请求或查询 query error 相关逻辑。 |
| [L35](../src/server/persistence/IdentityStore.cpp#L35) | `IsNineDigitPublicCode` | 定义 | `bool IsNineDigitPublicCode(const QString& value)` | 判断 is nine digit public code 相关逻辑。 |
| [L41](../src/server/persistence/IdentityStore.cpp#L41) | `AllocatePublicCode` | 定义 | `QString AllocatePublicCode(QSqlDatabase database, QString* errorMessage)` | 实现 allocate public code 对应的业务或工具逻辑。 |
| [L69](../src/server/persistence/IdentityStore.cpp#L69) | `DeviceTableSql` | 定义 | `QString DeviceTableSql(const QString& tableName)` | 实现 device table sql 对应的业务或工具逻辑。 |
| [L85](../src/server/persistence/IdentityStore.cpp#L85) | `HasUniqueDeviceIndex` | 定义 | `bool HasUniqueDeviceIndex(QSqlDatabase database, const QStringList& expectedColumns, bool* found, QString* errorMessage)` | 判断 has unique device index 相关逻辑。 |
| [L126](../src/server/persistence/IdentityStore.cpp#L126) | `RebuildDeviceTableForAccountScope` | 定义 | `bool RebuildDeviceTableForAccountScope(QSqlDatabase database, QString* errorMessage)` | 更新或应用 rebuild device table for account scope 相关逻辑。 |
| [L156](../src/server/persistence/IdentityStore.cpp#L156) | `IdentityStore::IdentityStore` | 定义 | `IdentityStore::IdentityStore() : connectionName_(QStringLiteral("remotec-identity-%1").arg( QUuid::createUuid().toString(QUuid::WithoutBraces))) {}` | 构造并初始化 IdentityStore 实例。 |
| [L160](../src/server/persistence/IdentityStore.cpp#L160) | `IdentityStore::~IdentityStore` | 定义 | `IdentityStore::~IdentityStore()` | 停止相关活动并释放 IdentityStore 实例拥有的资源。 |
| [L164](../src/server/persistence/IdentityStore.cpp#L164) | `IdentityStore::Open` | 定义 | `bool IdentityStore::Open(const QString& databaseFile, QString* errorMessage)` | 启动 open 相关逻辑。 |
| [L218](../src/server/persistence/IdentityStore.cpp#L218) | `IdentityStore::Close` | 定义 | `void IdentityStore::Close()` | 关闭并清理 close 相关逻辑。 |
| [L231](../src/server/persistence/IdentityStore.cpp#L231) | `IdentityStore::IsOpen` | 定义 | `bool IdentityStore::IsOpen() const` | 判断 is open 相关逻辑。 |
| [L238](../src/server/persistence/IdentityStore.cpp#L238) | `IdentityStore::EnsureSchema` | 定义 | `bool IdentityStore::EnsureSchema(QString* errorMessage)` | 实现 ensure schema 对应的业务或工具逻辑。 |
| [L413](../src/server/persistence/IdentityStore.cpp#L413) | `IdentityStore::UpsertUser` | 定义 | `RemoteUser IdentityStore::UpsertUser( const remote::server_auth::UserInfoClaims& claims, QString* errorMessage)` | 实现 upsert user 对应的业务或工具逻辑。 |
| [L467](../src/server/persistence/IdentityStore.cpp#L467) | `IdentityStore::RegisterDevice` | 定义 | `DeviceRegistrationResult IdentityStore::RegisterDevice( qint64 ownerUserId, const QString& installationId, const QString& deviceName)` | 实现 register device 对应的业务或工具逻辑。 |
| [L549](../src/server/persistence/IdentityStore.cpp#L549) | `IdentityStore::ListDevicesForUser` | 定义 | `QList<OwnedDeviceRecord> IdentityStore::ListDevicesForUser( qint64 ownerUserId, QString* errorMessage) const` | 实现 list devices for user 对应的业务或工具逻辑。 |
| [L588](../src/server/persistence/IdentityStore.cpp#L588) | `IdentityStore::DeleteUserByLogtoSubject` | 定义 | `bool IdentityStore::DeleteUserByLogtoSubject( const QString& logtoSubject, bool* existed, QString* errorMessage)` | 实现 delete user by logto subject 对应的业务或工具逻辑。 |

## `src/server/persistence/IdentityStore.h`

[打开源码](../src/server/persistence/IdentityStore.h) · **文件作用：** 声明 identity store 相关类型、接口、配置和成员状态。

### 类型

| 行 | 类型 | 种类 | 作用 |
|---:|---|---|---|
| [L12](../src/server/persistence/IdentityStore.h#L12) | `QSqlDatabase` | class | 定义 QSqlDatabase 的 class 类型和相关状态。 |
| [L17](../src/server/persistence/IdentityStore.h#L17) | `RemoteUser` | struct | 定义 RemoteUser 的 struct 类型和相关状态。 |
| [L27](../src/server/persistence/IdentityStore.h#L27) | `DeviceRegistrationStatus` | enum class | 定义 DeviceRegistrationStatus 的 enum class 类型和相关状态。 |
| [L33](../src/server/persistence/IdentityStore.h#L33) | `DeviceRegistrationResult` | struct | 定义 DeviceRegistrationResult 的 struct 类型和相关状态。 |
| [L43](../src/server/persistence/IdentityStore.h#L43) | `OwnedDeviceRecord` | struct | 定义 OwnedDeviceRecord 的 struct 类型和相关状态。 |
| [L53](../src/server/persistence/IdentityStore.h#L53) | `IdentityStore` | class | 定义 IdentityStore 的 class 类型和相关状态。 |

### 成员与文件级变量

| 行 | 变量 | 声明 | 作用 |
|---:|---|---|---|
| [L12](../src/server/persistence/IdentityStore.h#L12) | `QSqlDatabase` | `class QSqlDatabase;` | 保存 q sql database 相关配置或运行状态。 |
| [L18](../src/server/persistence/IdentityStore.h#L18) | `id` | `qint64 id = 0;` | 保存身份或作用域标识：id。 |
| [L19](../src/server/persistence/IdentityStore.h#L19) | `logtoSubject` | `QString logtoSubject;` | 保存 logto subject 相关配置或运行状态。 |
| [L20](../src/server/persistence/IdentityStore.h#L20) | `username` | `QString username;` | 保存路径、地址或显示名称：username。 |
| [L21](../src/server/persistence/IdentityStore.h#L21) | `displayName` | `QString displayName;` | 保存路径、地址或显示名称：display name。 |
| [L22](../src/server/persistence/IdentityStore.h#L22) | `email` | `QString email;` | 保存 email 相关配置或运行状态。 |
| [L34](../src/server/persistence/IdentityStore.h#L34) | `status` | `DeviceRegistrationStatus status = DeviceRegistrationStatus::kStoreError;` | 保存状态机当前状态：status。 |
| [L35](../src/server/persistence/IdentityStore.h#L35) | `publicDeviceId` | `QString publicDeviceId;` | 保存身份或作用域标识：public device id。 |
| [L36](../src/server/persistence/IdentityStore.h#L36) | `errorMessage` | `QString errorMessage;` | 保存 error message 相关配置或运行状态。 |
| [L44](../src/server/persistence/IdentityStore.h#L44) | `publicDeviceId` | `QString publicDeviceId;` | 保存身份或作用域标识：public device id。 |
| [L45](../src/server/persistence/IdentityStore.h#L45) | `installationId` | `QString installationId;` | 保存身份或作用域标识：installation id。 |
| [L46](../src/server/persistence/IdentityStore.h#L46) | `ownerUserId` | `qint64 ownerUserId = 0;` | 保存身份或作用域标识：owner user id。 |
| [L47](../src/server/persistence/IdentityStore.h#L47) | `deviceName` | `QString deviceName;` | 保存路径、地址或显示名称：device name。 |
| [L48](../src/server/persistence/IdentityStore.h#L48) | `createdAt` | `qint64 createdAt = 0;` | 保存 created at 相关配置或运行状态。 |
| [L49](../src/server/persistence/IdentityStore.h#L49) | `lastSeenAt` | `qint64 lastSeenAt = 0;` | 保存 last seen at 相关配置或运行状态。 |
| [L50](../src/server/persistence/IdentityStore.h#L50) | `revoked` | `bool revoked = false;` | 保存 revoked 相关配置或运行状态。 |
| [L83](../src/server/persistence/IdentityStore.h#L83) | `connectionName_` | `QString connectionName_;` | 保存路径、地址或显示名称：connection name。 |

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L24](../src/server/persistence/IdentityStore.h#L24) | `isValid` | 定义 | `bool isValid() const { return id > 0 && !logtoSubject.isEmpty(); }` | 判断 is valid 相关逻辑。 |
| [L38](../src/server/persistence/IdentityStore.h#L38) | `registered` | 定义 | `bool registered() const` | 实现 registered 对应的业务或工具逻辑。 |
| [L55](../src/server/persistence/IdentityStore.h#L55) | `IdentityStore` | 声明 | `IdentityStore()` | 实现 identity store 对应的业务或工具逻辑。 |
| [L56](../src/server/persistence/IdentityStore.h#L56) | `~IdentityStore` | 声明 | `~IdentityStore()` | 停止相关活动并释放 IdentityStore 实例拥有的资源。 |
| [L58](../src/server/persistence/IdentityStore.h#L58) | `IdentityStore` | 声明 | `IdentityStore(const IdentityStore&) = delete` | 实现 identity store 对应的业务或工具逻辑。 |
| [L61](../src/server/persistence/IdentityStore.h#L61) | `Open` | 声明 | `bool Open(const QString& databaseFile, QString* errorMessage = nullptr)` | 启动 open 相关逻辑。 |
| [L62](../src/server/persistence/IdentityStore.h#L62) | `Close` | 声明 | `void Close()` | 关闭并清理 close 相关逻辑。 |
| [L63](../src/server/persistence/IdentityStore.h#L63) | `IsOpen` | 声明 | `bool IsOpen() const` | 判断 is open 相关逻辑。 |
| [L65](../src/server/persistence/IdentityStore.h#L65) | `UpsertUser` | 声明 | `RemoteUser UpsertUser( const remote::server_auth::UserInfoClaims& claims, QString* errorMessage = nullptr)` | 实现 upsert user 对应的业务或工具逻辑。 |
| [L68](../src/server/persistence/IdentityStore.h#L68) | `RegisterDevice` | 声明 | `DeviceRegistrationResult RegisterDevice( qint64 ownerUserId, const QString& installationId, const QString& deviceName)` | 实现 register device 对应的业务或工具逻辑。 |
| [L72](../src/server/persistence/IdentityStore.h#L72) | `ListDevicesForUser` | 声明 | `QList<OwnedDeviceRecord> ListDevicesForUser( qint64 ownerUserId, QString* errorMessage = nullptr) const` | 实现 list devices for user 对应的业务或工具逻辑。 |
| [L75](../src/server/persistence/IdentityStore.h#L75) | `DeleteUserByLogtoSubject` | 声明 | `bool DeleteUserByLogtoSubject( const QString& logtoSubject, bool* existed = nullptr, QString* errorMessage = nullptr)` | 实现 delete user by logto subject 对应的业务或工具逻辑。 |
| [L81](../src/server/persistence/IdentityStore.h#L81) | `EnsureSchema` | 声明 | `bool EnsureSchema(QString* errorMessage)` | 实现 ensure schema 对应的业务或工具逻辑。 |

## `src/server/signaling/AccessTokenService.cpp`

[打开源码](../src/server/signaling/AccessTokenService.cpp) · **文件作用：** 实现 access token service 相关函数与文件级辅助逻辑。

### 成员与文件级变量

| 行 | 变量 | 声明 | 作用 |
|---:|---|---|---|
| [L18](../src/server/signaling/AccessTokenService.cpp#L18) | `kMaximumTokenLifetimeSeconds` | `constexpr qint64 kMaximumTokenLifetimeSeconds = 7 * 24 * 60 * 60;` | 定义 maximum token lifetime seconds 的编译期常量或产品边界。 |
| [L19](../src/server/signaling/AccessTokenService.cpp#L19) | `kClockSkewSeconds` | `constexpr qint64 kClockSkewSeconds = 60;` | 定义 clock skew seconds 的编译期常量或产品边界。 |
| [L20](../src/server/signaling/AccessTokenService.cpp#L20) | `kTokenAudience` | `constexpr auto kTokenAudience = "remotec-signaling";` | 定义 token audience 的编译期常量或产品边界。 |

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L22](../src/server/signaling/AccessTokenService.cpp#L22) | `Base64UrlEncode` | 定义 | `QByteArray Base64UrlEncode(const QByteArray& value)` | 实现 base64 url encode 对应的业务或工具逻辑。 |
| [L28](../src/server/signaling/AccessTokenService.cpp#L28) | `Base64UrlDecode` | 定义 | `QByteArray Base64UrlDecode(const QByteArray& value)` | 实现 base64 url decode 对应的业务或工具逻辑。 |
| [L35](../src/server/signaling/AccessTokenService.cpp#L35) | `ConstantTimeEqual` | 定义 | `bool ConstantTimeEqual(const QByteArray& left, const QByteArray& right)` | 实现 constant time equal 对应的业务或工具逻辑。 |
| [L48](../src/server/signaling/AccessTokenService.cpp#L48) | `SetError` | 定义 | `void SetError(QString* error, const QString& message)` | 更新或应用 set error 相关逻辑。 |
| [L57](../src/server/signaling/AccessTokenService.cpp#L57) | `AccessTokenService::AccessTokenService` | 定义 | `AccessTokenService::AccessTokenService(QByteArray secret) : secret_(std::move(secret)) {}` | 构造并初始化 AccessTokenService 实例。 |
| [L61](../src/server/signaling/AccessTokenService.cpp#L61) | `AccessTokenService::IsConfigured` | 定义 | `bool AccessTokenService::IsConfigured() const` | 判断 is configured 相关逻辑。 |
| [L66](../src/server/signaling/AccessTokenService.cpp#L66) | `AccessTokenService::Issue` | 定义 | `QByteArray AccessTokenService::Issue(const QString& deviceId, qint64 lifetimeSeconds, QString* error) const` | 判断 issue 相关逻辑。 |
| [L103](../src/server/signaling/AccessTokenService.cpp#L103) | `AccessTokenService::Verify` | 定义 | `bool AccessTokenService::Verify(const QByteArray& token, AccessTokenClaims* claims, QString* error) const` | 校验 verify 相关逻辑。 |
| [L167](../src/server/signaling/AccessTokenService.cpp#L167) | `AccessTokenService::IsValidDeviceId` | 定义 | `bool AccessTokenService::IsValidDeviceId(const QString& deviceId)` | 判断 is valid device id 相关逻辑。 |
| [L174](../src/server/signaling/AccessTokenService.cpp#L174) | `AccessTokenService::Sign` | 定义 | `QByteArray AccessTokenService::Sign(const QByteArray& encodedPayload) const` | 实现 sign 对应的业务或工具逻辑。 |

## `src/server/signaling/AccessTokenService.h`

[打开源码](../src/server/signaling/AccessTokenService.h) · **文件作用：** 声明 access token service 相关类型、接口、配置和成员状态。

### 类型

| 行 | 类型 | 种类 | 作用 |
|---:|---|---|---|
| [L11](../src/server/signaling/AccessTokenService.h#L11) | `AccessTokenClaims` | struct | 定义 AccessTokenClaims 的 struct 类型和相关状态。 |
| [L18](../src/server/signaling/AccessTokenService.h#L18) | `AccessTokenService` | class | 定义 AccessTokenService 的 class 类型和相关状态。 |

### 成员与文件级变量

| 行 | 变量 | 声明 | 作用 |
|---:|---|---|---|
| [L12](../src/server/signaling/AccessTokenService.h#L12) | `deviceId` | `QString deviceId;` | 保存身份或作用域标识：device id。 |
| [L13](../src/server/signaling/AccessTokenService.h#L13) | `tokenId` | `QString tokenId;` | 保存身份或作用域标识：token id。 |
| [L14](../src/server/signaling/AccessTokenService.h#L14) | `issuedAtSeconds` | `qint64 issuedAtSeconds = 0;` | 保存 issued at seconds 相关配置或运行状态。 |
| [L15](../src/server/signaling/AccessTokenService.h#L15) | `expiresAtSeconds` | `qint64 expiresAtSeconds = 0;` | 保存 expires at seconds 相关配置或运行状态。 |
| [L36](../src/server/signaling/AccessTokenService.h#L36) | `secret_` | `QByteArray secret_;` | 保存 secret 相关配置或运行状态。 |

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L20](../src/server/signaling/AccessTokenService.h#L20) | `AccessTokenService` | 声明 | `explicit AccessTokenService(QByteArray secret)` | 实现 access token service 对应的业务或工具逻辑。 |
| [L22](../src/server/signaling/AccessTokenService.h#L22) | `IsConfigured` | 声明 | `bool IsConfigured() const` | 判断 is configured 相关逻辑。 |
| [L24](../src/server/signaling/AccessTokenService.h#L24) | `Issue` | 声明 | `QByteArray Issue(const QString& deviceId, qint64 lifetimeSeconds, QString* error) const` | 判断 issue 相关逻辑。 |
| [L27](../src/server/signaling/AccessTokenService.h#L27) | `Verify` | 声明 | `bool Verify(const QByteArray& token, AccessTokenClaims* claims, QString* error) const` | 校验 verify 相关逻辑。 |
| [L31](../src/server/signaling/AccessTokenService.h#L31) | `IsValidDeviceId` | 声明 | `static bool IsValidDeviceId(const QString& deviceId)` | 判断 is valid device id 相关逻辑。 |
| [L34](../src/server/signaling/AccessTokenService.h#L34) | `Sign` | 声明 | `QByteArray Sign(const QByteArray& encodedPayload) const` | 实现 sign 对应的业务或工具逻辑。 |

## `src/server/signaling/SignalServer.AccountManagement.inc`

[打开源码](../src/server/signaling/SignalServer.AccountManagement.inc) · **文件作用：** `SignalServer` 的实现切片，集中实现 account management 相关逻辑；成员状态仍定义在所属头文件中。

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L4](../src/server/signaling/SignalServer.AccountManagement.inc#L4) | `SendAccountDeletionResult` | 定义 | `void SendAccountDeletionResult( QWebSocket* socket, bool deleted, const QString& code, const QString& message, bool retryable)` | SPDX-License-Identifier: GPL-3.0-only Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd) |
| [L20](../src/server/signaling/SignalServer.AccountManagement.inc#L20) | `HandleAccountDelete` | 定义 | `void HandleAccountDelete(ClientState* client)` | 接收并处理 handle account delete 相关逻辑。 |

## `src/server/signaling/SignalServer.Authentication.inc`

[打开源码](../src/server/signaling/SignalServer.Authentication.inc) · **文件作用：** `SignalServer` 的实现切片，集中实现 authentication 相关逻辑；成员状态仍定义在所属头文件中。

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L4](../src/server/signaling/SignalServer.Authentication.inc#L4) | `HandleMessageAuthentication` | 定义 | `void HandleMessageAuthentication( ClientState* client, const QJsonObject& payload)` | SPDX-License-Identifier: GPL-3.0-only Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd) |
| [L161](../src/server/signaling/SignalServer.Authentication.inc#L161) | `SendAuthenticationError` | 定义 | `void SendAuthenticationError( QWebSocket* socket, const QString& reason, const QString& message, bool retryable)` | 发送或发布 send authentication error 相关逻辑。 |

## `src/server/signaling/SignalServer.Connection.inc`

[打开源码](../src/server/signaling/SignalServer.Connection.inc) · **文件作用：** `SignalServer` 的实现切片，集中实现 connection 相关逻辑；成员状态仍定义在所属头文件中。

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L4](../src/server/signaling/SignalServer.Connection.inc#L4) | `AcceptConnections` | 定义 | `void AcceptConnections()` | SPDX-License-Identifier: GPL-3.0-only Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd) |
| [L121](../src/server/signaling/SignalServer.Connection.inc#L121) | `RejectConnection` | 定义 | `void RejectConnection(QWebSocket* socket, const QString& reason)` | 处理并回复 reject connection 相关逻辑。 |
| [L129](../src/server/signaling/SignalServer.Connection.inc#L129) | `OnTextMessage` | 定义 | `void OnTextMessage(QWebSocket* socket, const QString& message)` | 接收并处理 on text message 相关逻辑。 |

## `src/server/signaling/SignalServer.cpp`

[打开源码](../src/server/signaling/SignalServer.cpp) · **文件作用：** 实现 signal server 相关函数与文件级辅助逻辑。

### 类型

| 行 | 类型 | 种类 | 作用 |
|---:|---|---|---|
| [L206](../src/server/signaling/SignalServer.cpp#L206) | `SignalServer::Impl` | class | 定义 SignalServer::Impl 的 class 类型和相关状态。 |

### 成员与文件级变量

| 行 | 变量 | 声明 | 作用 |
|---:|---|---|---|
| [L57](../src/server/signaling/SignalServer.cpp#L57) | `kProtocolVersion` | `constexpr int kProtocolVersion = 5;` | 定义 protocol version 的编译期常量或产品边界。 |
| [L58](../src/server/signaling/SignalServer.cpp#L58) | `kMaximumMessageBytes` | `constexpr quint64 kMaximumMessageBytes = 1024 * 1024;` | 定义 maximum message bytes 的编译期常量或产品边界。 |
| [L59](../src/server/signaling/SignalServer.cpp#L59) | `kMaximumRememberedMessageIds` | `constexpr int kMaximumRememberedMessageIds = 4096;` | 定义 maximum remembered message ids 的编译期常量或产品边界。 |
| [L60](../src/server/signaling/SignalServer.cpp#L60) | `kMaximumSessions` | `constexpr int kMaximumSessions = 4096;` | 定义 maximum sessions 的编译期常量或产品边界。 |
| [L61](../src/server/signaling/SignalServer.cpp#L61) | `kMaximumRooms` | `constexpr int kMaximumRooms = 4096;` | 定义 maximum rooms 的编译期常量或产品边界。 |
| [L62](../src/server/signaling/SignalServer.cpp#L62) | `kInitialStateReserve` | `constexpr qsizetype kInitialStateReserve = 2048;` | 定义 initial state reserve 的编译期常量或产品边界。 |
| [L63](../src/server/signaling/SignalServer.cpp#L63) | `kMaximumPermissions` | `constexpr int kMaximumPermissions = 16;` | 定义 maximum permissions 的编译期常量或产品边界。 |
| [L64](../src/server/signaling/SignalServer.cpp#L64) | `kProtocolMaximumRoomMembers` | `constexpr int kProtocolMaximumRoomMembers = 5;` | 定义 protocol maximum room members 的编译期常量或产品边界。 |
| [L65](../src/server/signaling/SignalServer.cpp#L65) | `kRoomJoinRequestTimeoutMs` | `constexpr int kRoomJoinRequestTimeoutMs = 30000;` | 定义 room join request timeout ms 的编译期常量或产品边界。 |
| [L66](../src/server/signaling/SignalServer.cpp#L66) | `kRoomControlRequestTimeoutMs` | `constexpr int kRoomControlRequestTimeoutMs = 30000;` | 定义 room control request timeout ms 的编译期常量或产品边界。 |
| [L67](../src/server/signaling/SignalServer.cpp#L67) | `kRoomScreenShareSwitchRequestTimeoutMs` | `constexpr int kRoomScreenShareSwitchRequestTimeoutMs = 30000;` | 定义 room screen share switch request timeout ms 的编译期常量或产品边界。 |
| [L68](../src/server/signaling/SignalServer.cpp#L68) | `kRateLimitWindowMs` | `constexpr qint64 kRateLimitWindowMs = 60000;` | 定义 rate limit window ms 的编译期常量或产品边界。 |
| [L69](../src/server/signaling/SignalServer.cpp#L69) | `kRateLimitMaximumKeys` | `constexpr int kRateLimitMaximumKeys = 4096;` | 定义 rate limit maximum keys 的编译期常量或产品边界。 |

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L83](../src/server/signaling/SignalServer.cpp#L83) | `StdoutIsTerminal` | 定义 | `bool StdoutIsTerminal()` | 实现 stdout is terminal 对应的业务或工具逻辑。 |
| [L92](../src/server/signaling/SignalServer.cpp#L92) | `ClearStdoutTerminal` | 定义 | `void ClearStdoutTerminal()` | 重置或移除 clear stdout terminal 相关逻辑。 |
| [L114](../src/server/signaling/SignalServer.cpp#L114) | `FormatBytes` | 定义 | `QString FormatBytes(qint64 bytes)` | 实现 format bytes 对应的业务或工具逻辑。 |
| [L130](../src/server/signaling/SignalServer.cpp#L130) | `FormatBitRate` | 定义 | `QString FormatBitRate(double bitsPerSecond)` | 实现 format bit rate 对应的业务或工具逻辑。 |
| [L146](../src/server/signaling/SignalServer.cpp#L146) | `FormatDuration` | 定义 | `QString FormatDuration(qint64 milliseconds)` | 实现 format duration 对应的业务或工具逻辑。 |
| [L172](../src/server/signaling/SignalServer.cpp#L172) | `SetError` | 定义 | `void SetError(QString* error, const QString& message)` | 更新或应用 set error 相关逻辑。 |
| [L179](../src/server/signaling/SignalServer.cpp#L179) | `IsValidPurpose` | 定义 | `bool IsValidPurpose(const QString& purpose)` | 判断 is valid purpose 相关逻辑。 |
| [L185](../src/server/signaling/SignalServer.cpp#L185) | `IsNineDigitPublicId` | 定义 | `bool IsNineDigitPublicId(const QString& value)` | 判断 is nine digit public id 相关逻辑。 |
| [L192](../src/server/signaling/SignalServer.cpp#L192) | `GenerateNineDigitPublicId` | 定义 | `QString GenerateNineDigitPublicId()` | 实现 generate nine digit public id 对应的业务或工具逻辑。 |
| [L220](../src/server/signaling/SignalServer.cpp#L220) | `RememberMessageId` | 定义 | `bool RememberMessageId(ClientState* client, const QByteArray& messageId)` | 实现 remember message id 对应的业务或工具逻辑。 |
| [L238](../src/server/signaling/SignalServer.cpp#L238) | `CompleteClientAuthentication` | 定义 | `void CompleteClientAuthentication(ClientState* client)` | 实现 complete client authentication 对应的业务或工具逻辑。 |
| [L249](../src/server/signaling/SignalServer.cpp#L249) | `DeviceHasSession` | 定义 | `bool DeviceHasSession(const QString& deviceId) const` | 实现 device has session 对应的业务或工具逻辑。 |
| [L254](../src/server/signaling/SignalServer.cpp#L254) | `InsertSession` | 定义 | `bool InsertSession(SessionState session)` | 实现 insert session 对应的业务或工具逻辑。 |
| [L273](../src/server/signaling/SignalServer.cpp#L273) | `RemoveSession` | 定义 | `void RemoveSession(const QString& sessionId)` | 重置或移除 remove session 相关逻辑。 |
| [L290](../src/server/signaling/SignalServer.cpp#L290) | `ClearSessions` | 定义 | `void ClearSessions()` | 重置或移除 clear sessions 相关逻辑。 |
| [L296](../src/server/signaling/SignalServer.cpp#L296) | `AddAccountConnection` | 定义 | `void AddAccountConnection(ClientState* client)` | 实现 add account connection 对应的业务或工具逻辑。 |
| [L304](../src/server/signaling/SignalServer.cpp#L304) | `RemoveAccountConnection` | 定义 | `void RemoveAccountConnection(const ClientState* client)` | 重置或移除 remove account connection 相关逻辑。 |
| [L319](../src/server/signaling/SignalServer.cpp#L319) | `DeviceHasRoom` | 定义 | `bool DeviceHasRoom(const QString& deviceId) const` | 实现 device has room 对应的业务或工具逻辑。 |
| [L324](../src/server/signaling/SignalServer.cpp#L324) | `ReleaseDisconnectedRoomForFreshStart` | 定义 | `bool ReleaseDisconnectedRoomForFreshStart(const QString& deviceId)` | 释放或取消 release disconnected room for fresh start 相关逻辑。 |
| [L366](../src/server/signaling/SignalServer.cpp#L366) | `DeleteLocalAccount` | 定义 | `bool DeleteLocalAccount(const QString& subject, QString* error)` | 实现 delete local account 对应的业务或工具逻辑。 |
| [L452](../src/server/signaling/SignalServer.cpp#L452) | `SendError` | 定义 | `void SendError(QWebSocket* socket, const QString& sessionId, const QString& code, const QString& message)` | 发送或发布 send error 相关逻辑。 |
| [L463](../src/server/signaling/SignalServer.cpp#L463) | `PeerRateLimitKey` | 定义 | `QString PeerRateLimitKey(const QWebSocket* socket) const` | 实现 peer rate limit key 对应的业务或工具逻辑。 |
| [L478](../src/server/signaling/SignalServer.cpp#L478) | `AllowAuthenticationAttempt` | 定义 | `bool AllowAuthenticationAttempt(QWebSocket* socket, qint64 now)` | 实现 allow authentication attempt 对应的业务或工具逻辑。 |
| [L484](../src/server/signaling/SignalServer.cpp#L484) | `AllowRoomAvailabilityQuery` | 定义 | `bool AllowRoomAvailabilityQuery(ClientState* client, int requestedRoomCount, qint64 now)` | 实现 allow room availability query 对应的业务或工具逻辑。 |
| [L520](../src/server/signaling/SignalServer.cpp#L520) | `AllowRoomJoinAttempt` | 定义 | `bool AllowRoomJoinAttempt(ClientState* client, qint64 now)` | 实现 allow room join attempt 对应的业务或工具逻辑。 |
| [L549](../src/server/signaling/SignalServer.cpp#L549) | `AllowAccountDeletion` | 定义 | `bool AllowAccountDeletion(ClientState* client, qint64 now)` | 实现 allow account deletion 对应的业务或工具逻辑。 |
| [L556](../src/server/signaling/SignalServer.cpp#L556) | `AllowAssistanceAttempt` | 定义 | `bool AllowAssistanceAttempt(ClientState* client, const QString& targetDeviceId, qint64 now)` | 实现 allow assistance attempt 对应的业务或工具逻辑。 |
| [L582](../src/server/signaling/SignalServer.cpp#L582) | `PruneRateLimiters` | 定义 | `void PruneRateLimiters(qint64 now)` | 实现 prune rate limiters 对应的业务或工具逻辑。 |
| [L597](../src/server/signaling/SignalServer.cpp#L597) | `ClearRateLimiters` | 定义 | `void ClearRateLimiters()` | 重置或移除 clear rate limiters 相关逻辑。 |
| [L612](../src/server/signaling/SignalServer.cpp#L612) | `RateLimiterKeyCount` | 定义 | `qsizetype RateLimiterKeyCount() const` | 实现 rate limiter key count 对应的业务或工具逻辑。 |
| [L627](../src/server/signaling/SignalServer.cpp#L627) | `ProbeEventLoopLag` | 定义 | `void ProbeEventLoopLag()` | 实现 probe event loop lag 对应的业务或工具逻辑。 |
| [L641](../src/server/signaling/SignalServer.cpp#L641) | `WriteDiagnostics` | 定义 | `void WriteDiagnostics()` | 保存或写入 write diagnostics 相关逻辑。 |
| [L801](../src/server/signaling/SignalServer.cpp#L801) | `SendEnvelope` | 定义 | `void SendEnvelope(QWebSocket* socket, const QString& type, const QString& sessionId, const QJsonObject& payload)` | 发送或发布 send envelope 相关逻辑。 |
| [L884](../src/server/signaling/SignalServer.cpp#L884) | `SignalServer::SignalServer` | 定义 | `SignalServer::SignalServer(SignalServerConfig config) : impl_(std::make_unique<Impl>(std::move(config))) {}` | 构造并初始化 SignalServer 实例。 |
| [L888](../src/server/signaling/SignalServer.cpp#L888) | `SignalServer::~SignalServer` | 定义 | `SignalServer::~SignalServer()` | 停止相关活动并释放 SignalServer 实例拥有的资源。 |
| [L893](../src/server/signaling/SignalServer.cpp#L893) | `SignalServer::Start` | 定义 | `bool SignalServer::Start(QString* error)` | 启动 start 相关逻辑。 |
| [L898](../src/server/signaling/SignalServer.cpp#L898) | `SignalServer::Stop` | 定义 | `void SignalServer::Stop()` | 停止 stop 相关逻辑。 |
| [L903](../src/server/signaling/SignalServer.cpp#L903) | `SignalServer::IsListening` | 定义 | `bool SignalServer::IsListening() const` | 判断 is listening 相关逻辑。 |
| [L908](../src/server/signaling/SignalServer.cpp#L908) | `SignalServer::ServerPort` | 定义 | `quint16 SignalServer::ServerPort() const` | 实现 server port 对应的业务或工具逻辑。 |
| [L913](../src/server/signaling/SignalServer.cpp#L913) | `SignalServer::WebhookPort` | 定义 | `quint16 SignalServer::WebhookPort() const` | 实现 webhook port 对应的业务或工具逻辑。 |

## `src/server/signaling/SignalServer.h`

[打开源码](../src/server/signaling/SignalServer.h) · **文件作用：** 声明 signal server 相关类型、接口、配置和成员状态。

### 类型

| 行 | 类型 | 种类 | 作用 |
|---:|---|---|---|
| [L16](../src/server/signaling/SignalServer.h#L16) | `SignalServerAuthenticationMode` | enum class | 定义 SignalServerAuthenticationMode 的 enum class 类型和相关状态。 |
| [L27](../src/server/signaling/SignalServer.h#L27) | `SignalServerConfig` | struct | 定义 SignalServerConfig 的 struct 类型和相关状态。 |
| [L60](../src/server/signaling/SignalServer.h#L60) | `SignalServer` | class | 定义 SignalServer 的 class 类型和相关状态。 |
| [L75](../src/server/signaling/SignalServer.h#L75) | `Impl` | class | 定义 Impl 的 class 类型和相关状态。 |

### 成员与文件级变量

| 行 | 变量 | 声明 | 作用 |
|---:|---|---|---|
| [L28](../src/server/signaling/SignalServer.h#L28) | `listenAddress` | `QHostAddress listenAddress = QHostAddress::LocalHost;` | 保存 listen address 相关配置或运行状态。 |
| [L29](../src/server/signaling/SignalServer.h#L29) | `port` | `quint16 port = 9443;` | 保存 port 相关配置或运行状态。 |
| [L30](../src/server/signaling/SignalServer.h#L30) | `certificateFile` | `QString certificateFile;` | 保存 certificate file 相关配置或运行状态。 |
| [L31](../src/server/signaling/SignalServer.h#L31) | `privateKeyFile` | `QString privateKeyFile;` | 保存 private key file 相关配置或运行状态。 |
| [L32](../src/server/signaling/SignalServer.h#L32) | `tokenSecret` | `QByteArray tokenSecret;` | 保存 token secret 相关配置或运行状态。 |
| [L34](../src/server/signaling/SignalServer.h#L34) | `kLegacyUpgradeBearer` | `SignalServerAuthenticationMode::kLegacyUpgradeBearer;` | 定义 legacy upgrade bearer 的编译期常量或产品边界。 |
| [L35](../src/server/signaling/SignalServer.h#L35) | `iceServerUrls` | `QStringList iceServerUrls;` | 保存 ice server urls 相关配置或运行状态。 |
| [L36](../src/server/signaling/SignalServer.h#L36) | `maximumPendingConnections` | `int maximumPendingConnections = 128;` | 保存 maximum pending connections 相关配置或运行状态。 |
| [L37](../src/server/signaling/SignalServer.h#L37) | `maximumConnections` | `int maximumConnections = 5000;` | 保存 maximum connections 相关配置或运行状态。 |
| [L38](../src/server/signaling/SignalServer.h#L38) | `maximumUnauthenticatedConnections` | `int maximumUnauthenticatedConnections = 512;` | 保存 maximum unauthenticated connections 相关配置或运行状态。 |
| [L39](../src/server/signaling/SignalServer.h#L39) | `maximumConnectionsPerIp` | `int maximumConnectionsPerIp = 0;` | 保存 maximum connections per ip 相关配置或运行状态。 |
| [L40](../src/server/signaling/SignalServer.h#L40) | `authenticationIpLimitPerMinute` | `int authenticationIpLimitPerMinute = 20;` | 保存 authentication ip limit per minute 相关配置或运行状态。 |
| [L41](../src/server/signaling/SignalServer.h#L41) | `disableBusinessRateLimitsForTest` | `bool disableBusinessRateLimitsForTest = false;` | 保存 disable business rate limits for test 相关配置或运行状态。 |
| [L42](../src/server/signaling/SignalServer.h#L42) | `diagnosticsIntervalMs` | `int diagnosticsIntervalMs = 0;` | 保存 diagnostics interval ms 相关配置或运行状态。 |
| [L43](../src/server/signaling/SignalServer.h#L43) | `diagnosticsLogFile` | `QString diagnosticsLogFile;` | 保存 diagnostics log file 相关配置或运行状态。 |
| [L44](../src/server/signaling/SignalServer.h#L44) | `authenticationTimeoutMs` | `int authenticationTimeoutMs = 10000;` | 保存 authentication timeout ms 相关配置或运行状态。 |
| [L45](../src/server/signaling/SignalServer.h#L45) | `logtoIssuer` | `QUrl logtoIssuer;` | 保存 logto issuer 相关配置或运行状态。 |
| [L46](../src/server/signaling/SignalServer.h#L46) | `identityDatabaseFile` | `QString identityDatabaseFile;` | 保存 identity database file 相关配置或运行状态。 |
| [L47](../src/server/signaling/SignalServer.h#L47) | `userInfoTimeoutMs` | `int userInfoTimeoutMs = 10000;` | 保存 user info timeout ms 相关配置或运行状态。 |
| [L48](../src/server/signaling/SignalServer.h#L48) | `logtoManagementClientId` | `QString logtoManagementClientId;` | 保存身份或作用域标识：logto management client id。 |
| [L49](../src/server/signaling/SignalServer.h#L49) | `logtoManagementClientSecret` | `QByteArray logtoManagementClientSecret;` | 保存 logto management client secret 相关配置或运行状态。 |
| [L50](../src/server/signaling/SignalServer.h#L50) | `managementTimeoutMs` | `int managementTimeoutMs = 10000;` | 保存 management timeout ms 相关配置或运行状态。 |
| [L51](../src/server/signaling/SignalServer.h#L51) | `webhookListenAddress` | `QHostAddress webhookListenAddress = QHostAddress::LocalHost;` | 保存 webhook listen address 相关配置或运行状态。 |
| [L52](../src/server/signaling/SignalServer.h#L52) | `webhookPort` | `quint16 webhookPort = 0;` | 保存 webhook port 相关配置或运行状态。 |
| [L53](../src/server/signaling/SignalServer.h#L53) | `webhookSigningKey` | `QByteArray webhookSigningKey;` | 保存 webhook signing key 相关配置或运行状态。 |
| [L54](../src/server/signaling/SignalServer.h#L54) | `clientIdleTimeoutMs` | `int clientIdleTimeoutMs = 45000;` | 保存 client idle timeout ms 相关配置或运行状态。 |
| [L55](../src/server/signaling/SignalServer.h#L55) | `pendingSessionTimeoutMs` | `int pendingSessionTimeoutMs = 20000;` | 保存 pending session timeout ms 相关配置或运行状态。 |
| [L56](../src/server/signaling/SignalServer.h#L56) | `sessionRecoveryWindowMs` | `int sessionRecoveryWindowMs = 60000;` | 保存 session recovery window ms 相关配置或运行状态。 |
| [L57](../src/server/signaling/SignalServer.h#L57) | `maximumRoomMembers` | `int maximumRoomMembers = 5;` | 保存 maximum room members 相关配置或运行状态。 |
| [L75](../src/server/signaling/SignalServer.h#L75) | `Impl` | `class Impl;` | 保存 impl 相关配置或运行状态。 |
| [L76](../src/server/signaling/SignalServer.h#L76) | `impl_` | `std::unique_ptr<Impl> impl_;` | 保存 impl 相关配置或运行状态。 |

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L62](../src/server/signaling/SignalServer.h#L62) | `SignalServer` | 声明 | `explicit SignalServer(SignalServerConfig config)` | 实现 signal server 对应的业务或工具逻辑。 |
| [L63](../src/server/signaling/SignalServer.h#L63) | `~SignalServer` | 声明 | `~SignalServer()` | 停止相关活动并释放 SignalServer 实例拥有的资源。 |
| [L65](../src/server/signaling/SignalServer.h#L65) | `SignalServer` | 声明 | `SignalServer(const SignalServer&) = delete` | 实现 signal server 对应的业务或工具逻辑。 |
| [L68](../src/server/signaling/SignalServer.h#L68) | `Start` | 声明 | `bool Start(QString* error)` | 启动 start 相关逻辑。 |
| [L69](../src/server/signaling/SignalServer.h#L69) | `Stop` | 声明 | `void Stop()` | 停止 stop 相关逻辑。 |
| [L70](../src/server/signaling/SignalServer.h#L70) | `IsListening` | 声明 | `bool IsListening() const` | 判断 is listening 相关逻辑。 |
| [L71](../src/server/signaling/SignalServer.h#L71) | `ServerPort` | 声明 | `quint16 ServerPort() const` | 实现 server port 对应的业务或工具逻辑。 |
| [L72](../src/server/signaling/SignalServer.h#L72) | `WebhookPort` | 声明 | `quint16 WebhookPort() const` | 实现 webhook port 对应的业务或工具逻辑。 |

## `src/server/signaling/SignalServer.LegacySession.inc`

[打开源码](../src/server/signaling/SignalServer.LegacySession.inc) · **文件作用：** `SignalServer` 的实现切片，集中实现 legacy session 相关逻辑；成员状态仍定义在所属头文件中。

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L4](../src/server/signaling/SignalServer.LegacySession.inc#L4) | `HandleSessionRequest` | 定义 | `void HandleSessionRequest(ClientState* requester, const QJsonObject& payload)` | SPDX-License-Identifier: GPL-3.0-only Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd) |
| [L125](../src/server/signaling/SignalServer.LegacySession.inc#L125) | `HandleSessionResponse` | 定义 | `void HandleSessionResponse(ClientState* target, const QString& sessionId, const QJsonObject& payload)` | 接收并处理 handle session response 相关逻辑。 |
| [L185](../src/server/signaling/SignalServer.LegacySession.inc#L185) | `SendSessionReady` | 定义 | `void SendSessionReady(QWebSocket* socket, const QString& sessionId, const QString& peerDeviceId, const QString& recoveryToken)` | 发送或发布 send session ready 相关逻辑。 |
| [L198](../src/server/signaling/SignalServer.LegacySession.inc#L198) | `HandleSessionResume` | 定义 | `void HandleSessionResume(ClientState* client, const QString& sessionId, const QJsonObject& payload)` | 接收并处理 handle session resume 相关逻辑。 |
| [L254](../src/server/signaling/SignalServer.LegacySession.inc#L254) | `SendSessionResumed` | 定义 | `void SendSessionResumed(QWebSocket* socket, const QString& sessionId, const QString& peerDeviceId, const QString& resumedDeviceId)` | 发送或发布 send session resumed 相关逻辑。 |

## `src/server/signaling/SignalServer.Lifecycle.inc`

[打开源码](../src/server/signaling/SignalServer.Lifecycle.inc) · **文件作用：** `SignalServer` 的实现切片，集中实现 lifecycle 相关逻辑；成员状态仍定义在所属头文件中。

### 类型

| 行 | 类型 | 种类 | 作用 |
|---:|---|---|---|
| [L300](../src/server/signaling/SignalServer.Lifecycle.inc#L300) | `ClientState` | struct | 定义 ClientState 的 struct 类型和相关状态。 |
| [L319](../src/server/signaling/SignalServer.Lifecycle.inc#L319) | `SessionState` | struct | 定义 SessionState 的 struct 类型和相关状态。 |
| [L320](../src/server/signaling/SignalServer.Lifecycle.inc#L320) | `Phase` | enum class | 定义 Phase 的 enum class 类型和相关状态。 |
| [L338](../src/server/signaling/SignalServer.Lifecycle.inc#L338) | `RoomMemberState` | struct | 定义 RoomMemberState 的 struct 类型和相关状态。 |
| [L347](../src/server/signaling/SignalServer.Lifecycle.inc#L347) | `RoomPairState` | struct | 定义 RoomPairState 的 struct 类型和相关状态。 |
| [L354](../src/server/signaling/SignalServer.Lifecycle.inc#L354) | `RoomState` | struct | 定义 RoomState 的 struct 类型和相关状态。 |
| [L375](../src/server/signaling/SignalServer.Lifecycle.inc#L375) | `RoomJoinRequestState` | struct | 定义 RoomJoinRequestState 的 struct 类型和相关状态。 |

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L4](../src/server/signaling/SignalServer.Lifecycle.inc#L4) | `Impl` | 定义 | `explicit Impl(SignalServerConfig config) : config_(std::move(config)) , tokenService_(config_.tokenSecret) , authenticationIpRateLimiter_(SlidingWindowRateLimitPolicy` | SPDX-License-Identifier: GPL-3.0-only Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd) |
| [L11](../src/server/signaling/SignalServer.Lifecycle.inc#L11) | `availabilityIpRateLimiter_` | 定义 | `, availabilityIpRateLimiter_(kAvailabilityIpRateLimit) , availabilityUserRateLimiter_(kAvailabilityUserRateLimit) , availabilityDeviceRateLimiter_(kAvailabilityDeviceRateLimit) , roomJoinIpRateLimiter_(kRoomJoinIpRate...` | 实现 availability ip rate limiter 对应的业务或工具逻辑。 |
| [L46](../src/server/signaling/SignalServer.Lifecycle.inc#L46) | `Start` | 定义 | `bool Start(QString* error)` | 启动 start 相关逻辑。 |
| [L240](../src/server/signaling/SignalServer.Lifecycle.inc#L240) | `Stop` | 定义 | `void Stop()` | 停止 stop 相关逻辑。 |
| [L284](../src/server/signaling/SignalServer.Lifecycle.inc#L284) | `IsListening` | 定义 | `bool IsListening() const` | 判断 is listening 相关逻辑。 |
| [L289](../src/server/signaling/SignalServer.Lifecycle.inc#L289) | `ServerPort` | 定义 | `quint16 ServerPort() const` | 实现 server port 对应的业务或工具逻辑。 |
| [L294](../src/server/signaling/SignalServer.Lifecycle.inc#L294) | `WebhookPort` | 定义 | `quint16 WebhookPort() const` | 实现 webhook port 对应的业务或工具逻辑。 |

## `src/server/signaling/SignalServer.OwnedDevices.inc`

[打开源码](../src/server/signaling/SignalServer.OwnedDevices.inc) · **文件作用：** `SignalServer` 的实现切片，集中实现 owned devices 相关逻辑；成员状态仍定义在所属头文件中。

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L4](../src/server/signaling/SignalServer.OwnedDevices.inc#L4) | `BuildOwnedDevicesPayloadFromRecords` | 定义 | `QJsonObject BuildOwnedDevicesPayloadFromRecords( ClientState* recipient, quint64 revision, const QList<remote::server_persistence::OwnedDeviceRecord>& owned, const QString& storeError)` | SPDX-License-Identifier: GPL-3.0-only Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd) |
| [L47](../src/server/signaling/SignalServer.OwnedDevices.inc#L47) | `BuildOwnedDevicesPayload` | 定义 | `QJsonObject BuildOwnedDevicesPayload(ClientState* recipient, quint64 revision)` | 创建或初始化 build owned devices payload 相关逻辑。 |
| [L57](../src/server/signaling/SignalServer.OwnedDevices.inc#L57) | `HandleMyDevicesRequest` | 定义 | `void HandleMyDevicesRequest(ClientState* client)` | 接收并处理 handle my devices request 相关逻辑。 |
| [L71](../src/server/signaling/SignalServer.OwnedDevices.inc#L71) | `BroadcastOwnedDevices` | 定义 | `void BroadcastOwnedDevices(qint64 trustedUserId)` | 实现 broadcast owned devices 对应的业务或工具逻辑。 |

## `src/server/signaling/SignalServer.Recovery.inc`

[打开源码](../src/server/signaling/SignalServer.Recovery.inc) · **文件作用：** `SignalServer` 的实现切片，集中实现 recovery 相关逻辑；成员状态仍定义在所属头文件中。

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L4](../src/server/signaling/SignalServer.Recovery.inc#L4) | `OnDisconnected` | 定义 | `void OnDisconnected(QWebSocket* socket)` | SPDX-License-Identifier: GPL-3.0-only Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd) |
| [L49](../src/server/signaling/SignalServer.Recovery.inc#L49) | `MarkDeviceDisconnected` | 定义 | `void MarkDeviceDisconnected(const QString& deviceId)` | 实现 mark device disconnected 对应的业务或工具逻辑。 |
| [L131](../src/server/signaling/SignalServer.Recovery.inc#L131) | `SweepIdleClients` | 定义 | `void SweepIdleClients()` | 实现 sweep idle clients 对应的业务或工具逻辑。 |
| [L161](../src/server/signaling/SignalServer.Recovery.inc#L161) | `SweepPendingSessions` | 定义 | `void SweepPendingSessions(qint64 now)` | 实现 sweep pending sessions 对应的业务或工具逻辑。 |
| [L198](../src/server/signaling/SignalServer.Recovery.inc#L198) | `SweepRecoverySessions` | 定义 | `void SweepRecoverySessions(qint64 now)` | 实现 sweep recovery sessions 对应的业务或工具逻辑。 |
| [L238](../src/server/signaling/SignalServer.Recovery.inc#L238) | `SweepRoomJoinRequests` | 定义 | `void SweepRoomJoinRequests(qint64 now)` | 实现 sweep room join requests 对应的业务或工具逻辑。 |
| [L290](../src/server/signaling/SignalServer.Recovery.inc#L290) | `SweepRoomControlRequests` | 定义 | `void SweepRoomControlRequests(qint64 now)` | 实现 sweep room control requests 对应的业务或工具逻辑。 |
| [L304](../src/server/signaling/SignalServer.Recovery.inc#L304) | `SweepRoomScreenShareSwitchRequests` | 定义 | `void SweepRoomScreenShareSwitchRequests(qint64 now)` | 实现 sweep room screen share switch requests 对应的业务或工具逻辑。 |
| [L319](../src/server/signaling/SignalServer.Recovery.inc#L319) | `SweepRoomRecoveries` | 定义 | `void SweepRoomRecoveries(qint64 now)` | 实现 sweep room recoveries 对应的业务或工具逻辑。 |
| [L389](../src/server/signaling/SignalServer.Recovery.inc#L389) | `FindClient` | 定义 | `ClientState* FindClient(QWebSocket* socket)` | 查询并返回 find client 相关逻辑。 |

## `src/server/signaling/SignalServer.Relay.inc`

[打开源码](../src/server/signaling/SignalServer.Relay.inc) · **文件作用：** `SignalServer` 的实现切片，集中实现 relay 相关逻辑；成员状态仍定义在所属头文件中。

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L4](../src/server/signaling/SignalServer.Relay.inc#L4) | `RelayRoomPairMessage` | 定义 | `bool RelayRoomPairMessage(ClientState* sender, const QString& pairId, const QString& type, const QJsonObject& payload)` | SPDX-License-Identifier: GPL-3.0-only Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd) |
| [L93](../src/server/signaling/SignalServer.Relay.inc#L93) | `RelaySessionMessage` | 定义 | `void RelaySessionMessage(ClientState* sender, const QString& sessionId, const QString& type, const QJsonObject& payload)` | 发送或发布 relay session message 相关逻辑。 |
| [L149](../src/server/signaling/SignalServer.Relay.inc#L149) | `CloseSession` | 定义 | `void CloseSession(ClientState* sender, const QString& sessionId, const QString& type, const QJsonObject& payload)` | 关闭并清理 close session 相关逻辑。 |
| [L201](../src/server/signaling/SignalServer.Relay.inc#L201) | `SendSessionEnded` | 定义 | `void SendSessionEnded(QWebSocket* socket, const QString& sessionId, const QString& initiatorDeviceId, const QString& reasonCode, const QString& disposition)` | 发送或发布 send session ended 相关逻辑。 |

## `src/server/signaling/SignalServer.RoomLeases.inc`

[打开源码](../src/server/signaling/SignalServer.RoomLeases.inc) · **文件作用：** `SignalServer` 的实现切片，集中实现 room leases 相关逻辑；成员状态仍定义在所属头文件中。

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L4](../src/server/signaling/SignalServer.RoomLeases.inc#L4) | `IsOnlineRoomMember` | 定义 | `bool IsOnlineRoomMember(const RoomState& room, const QString& deviceId) const` | SPDX-License-Identifier: GPL-3.0-only Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd) |
| [L13](../src/server/signaling/SignalServer.RoomLeases.inc#L13) | `SendRoomScreenShareGranted` | 定义 | `void SendRoomScreenShareGranted(QWebSocket* socket, const RoomState& room)` | 发送或发布 send room screen share granted 相关逻辑。 |
| [L29](../src/server/signaling/SignalServer.RoomLeases.inc#L29) | `SendRoomScreenShareSwitchPending` | 定义 | `void SendRoomScreenShareSwitchPending(QWebSocket* socket, const RoomState& room)` | 发送或发布 send room screen share switch pending 相关逻辑。 |
| [L47](../src/server/signaling/SignalServer.RoomLeases.inc#L47) | `SendRoomScreenShareSwitchRequested` | 定义 | `void SendRoomScreenShareSwitchRequested(QWebSocket* socket, const RoomState& room)` | 发送或发布 send room screen share switch requested 相关逻辑。 |
| [L70](../src/server/signaling/SignalServer.RoomLeases.inc#L70) | `SendRoomScreenShareSwitchResult` | 定义 | `void SendRoomScreenShareSwitchResult( QWebSocket* socket, const RoomState& room, const QString& requestId, bool accepted, const QString& reasonCode, const QString& reasonMessage)` | 发送或发布 send room screen share switch result 相关逻辑。 |
| [L92](../src/server/signaling/SignalServer.RoomLeases.inc#L92) | `CancelPendingRoomScreenShareSwitch` | 定义 | `void CancelPendingRoomScreenShareSwitch( RoomState& room, const QString& reasonCode, const QString& reasonMessage)` | 判断 cancel pending room screen share switch 相关逻辑。 |
| [L118](../src/server/signaling/SignalServer.RoomLeases.inc#L118) | `SendRoomControlResult` | 定义 | `void SendRoomControlResult(QWebSocket* socket, const RoomState& room, const QString& requestId, bool accepted, const QString& reasonCode, const QString& reasonMessage)` | 发送或发布 send room control result 相关逻辑。 |
| [L138](../src/server/signaling/SignalServer.RoomLeases.inc#L138) | `SendRoomControlGranted` | 定义 | `void SendRoomControlGranted(QWebSocket* socket, const RoomState& room)` | 发送或发布 send room control granted 相关逻辑。 |
| [L157](../src/server/signaling/SignalServer.RoomLeases.inc#L157) | `SendRoomControlRevoked` | 定义 | `void SendRoomControlRevoked(QWebSocket* socket, const RoomState& room, const QString& initiatorDeviceId, const QString& reasonCode)` | 发送或发布 send room control revoked 相关逻辑。 |
| [L174](../src/server/signaling/SignalServer.RoomLeases.inc#L174) | `CancelPendingRoomControl` | 定义 | `void CancelPendingRoomControl(RoomState& room, const QString& reasonCode, const QString& reasonMessage)` | 判断 cancel pending room control 相关逻辑。 |
| [L200](../src/server/signaling/SignalServer.RoomLeases.inc#L200) | `RevokeRoomControl` | 定义 | `void RevokeRoomControl(RoomState& room, const QString& initiatorDeviceId, const QString& reasonCode)` | 实现 revoke room control 对应的业务或工具逻辑。 |
| [L222](../src/server/signaling/SignalServer.RoomLeases.inc#L222) | `ResetRoomScreenShare` | 定义 | `void ResetRoomScreenShare(RoomState& room, const QString& initiatorDeviceId, const QString& reasonCode)` | 重置或移除 reset room screen share 相关逻辑。 |
| [L244](../src/server/signaling/SignalServer.RoomLeases.inc#L244) | `HandleRoomScreenShareRequest` | 定义 | `void HandleRoomScreenShareRequest(ClientState* client, const QJsonObject& payload)` | 接收并处理 handle room screen share request 相关逻辑。 |
| [L334](../src/server/signaling/SignalServer.RoomLeases.inc#L334) | `HandleRoomScreenShareReady` | 定义 | `void HandleRoomScreenShareReady(ClientState* client, const QJsonObject& payload)` | 接收并处理 handle room screen share ready 相关逻辑。 |
| [L366](../src/server/signaling/SignalServer.RoomLeases.inc#L366) | `HandleRoomScreenShareSwitchResponse` | 定义 | `void HandleRoomScreenShareSwitchResponse( ClientState* client, const QJsonObject& payload)` | 接收并处理 handle room screen share switch response 相关逻辑。 |
| [L451](../src/server/signaling/SignalServer.RoomLeases.inc#L451) | `HandleRoomScreenShareSwitchCancel` | 定义 | `void HandleRoomScreenShareSwitchCancel( ClientState* client, const QJsonObject& payload)` | 接收并处理 handle room screen share switch cancel 相关逻辑。 |
| [L489](../src/server/signaling/SignalServer.RoomLeases.inc#L489) | `HandleRoomScreenShareStop` | 定义 | `void HandleRoomScreenShareStop(ClientState* client, const QJsonObject& payload)` | 接收并处理 handle room screen share stop 相关逻辑。 |
| [L525](../src/server/signaling/SignalServer.RoomLeases.inc#L525) | `HandleRoomControlRequest` | 定义 | `void HandleRoomControlRequest(ClientState* client, const QJsonObject& payload)` | 接收并处理 handle room control request 相关逻辑。 |
| [L596](../src/server/signaling/SignalServer.RoomLeases.inc#L596) | `HandleRoomControlResponse` | 定义 | `void HandleRoomControlResponse(ClientState* client, const QJsonObject& payload)` | 接收并处理 handle room control response 相关逻辑。 |
| [L670](../src/server/signaling/SignalServer.RoomLeases.inc#L670) | `HandleRoomControlRelease` | 定义 | `void HandleRoomControlRelease(ClientState* client, const QJsonObject& payload)` | 接收并处理 handle room control release 相关逻辑。 |
| [L703](../src/server/signaling/SignalServer.RoomLeases.inc#L703) | `SerializeIceServers` | 定义 | `QJsonArray SerializeIceServers() const` | 编码 serialize ice servers 相关逻辑。 |

## `src/server/signaling/SignalServer.RoomMembership.inc`

[打开源码](../src/server/signaling/SignalServer.RoomMembership.inc) · **文件作用：** `SignalServer` 的实现切片，集中实现 room membership 相关逻辑；成员状态仍定义在所属头文件中。

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L4](../src/server/signaling/SignalServer.RoomMembership.inc#L4) | `HandleRegister` | 定义 | `void HandleRegister(ClientState* client, const QJsonObject& payload)` | SPDX-License-Identifier: GPL-3.0-only Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd) |
| [L77](../src/server/signaling/SignalServer.RoomMembership.inc#L77) | `HandleRoomCreate` | 定义 | `void HandleRoomCreate(ClientState* creator, const QJsonObject& payload)` | 接收并处理 handle room create 相关逻辑。 |
| [L148](../src/server/signaling/SignalServer.RoomMembership.inc#L148) | `HandleRoomAvailabilityQuery` | 定义 | `void HandleRoomAvailabilityQuery(ClientState* client, const QJsonObject& payload)` | 接收并处理 handle room availability query 相关逻辑。 |
| [L208](../src/server/signaling/SignalServer.RoomMembership.inc#L208) | `HandleRoomJoinRequest` | 定义 | `void HandleRoomJoinRequest(ClientState* requester, const QJsonObject& payload)` | 接收并处理 handle room join request 相关逻辑。 |
| [L310](../src/server/signaling/SignalServer.RoomMembership.inc#L310) | `HandleRoomJoinResponse` | 定义 | `void HandleRoomJoinResponse(ClientState* owner, const QJsonObject& payload)` | 接收并处理 handle room join response 相关逻辑。 |
| [L440](../src/server/signaling/SignalServer.RoomMembership.inc#L440) | `HandleRoomSetCapacity` | 定义 | `void HandleRoomSetCapacity(ClientState* owner, const QJsonObject& payload)` | 接收并处理 handle room set capacity 相关逻辑。 |
| [L492](../src/server/signaling/SignalServer.RoomMembership.inc#L492) | `HandleRoomLeave` | 定义 | `void HandleRoomLeave(ClientState* memberClient, const QJsonObject& payload)` | 接收并处理 handle room leave 相关逻辑。 |
| [L568](../src/server/signaling/SignalServer.RoomMembership.inc#L568) | `HandleRoomMediaState` | 定义 | `void HandleRoomMediaState(ClientState* memberClient, const QJsonObject& payload)` | 接收并处理 handle room media state 相关逻辑。 |
| [L611](../src/server/signaling/SignalServer.RoomMembership.inc#L611) | `HandleRoomResume` | 定义 | `void HandleRoomResume(ClientState* client, const QJsonObject& payload)` | 接收并处理 handle room resume 相关逻辑。 |

## `src/server/signaling/SignalServer.RoomPairs.inc`

[打开源码](../src/server/signaling/SignalServer.RoomPairs.inc) · **文件作用：** `SignalServer` 的实现切片，集中实现 room pairs 相关逻辑；成员状态仍定义在所属头文件中。

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L4](../src/server/signaling/SignalServer.RoomPairs.inc#L4) | `CreateRoomPair` | 定义 | `void CreateRoomPair(RoomState& room, const QString& firstDeviceId, const QString& secondDeviceId)` | SPDX-License-Identifier: GPL-3.0-only Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd) |
| [L34](../src/server/signaling/SignalServer.RoomPairs.inc#L34) | `PairPeerDeviceId` | 定义 | `QString PairPeerDeviceId(const RoomPairState& pair, const QString& deviceId) const` | 实现 pair peer device id 对应的业务或工具逻辑。 |
| [L46](../src/server/signaling/SignalServer.RoomPairs.inc#L46) | `RoomPairMembersAreOnline` | 定义 | `bool RoomPairMembersAreOnline(const RoomState& room, const RoomPairState& pair) const` | 实现 room pair members are online 对应的业务或工具逻辑。 |
| [L59](../src/server/signaling/SignalServer.RoomPairs.inc#L59) | `SendRoomPairReadyToMember` | 定义 | `void SendRoomPairReadyToMember(const RoomState& room, const RoomPairState& pair, const QString& deviceId)` | 发送或发布 send room pair ready to member 相关逻辑。 |
| [L79](../src/server/signaling/SignalServer.RoomPairs.inc#L79) | `SendRoomPairReady` | 定义 | `void SendRoomPairReady(const RoomState& room, const RoomPairState& pair)` | 发送或发布 send room pair ready 相关逻辑。 |
| [L96](../src/server/signaling/SignalServer.RoomPairs.inc#L96) | `SendRoomPairsReadyForMember` | 定义 | `void SendRoomPairsReadyForMember(const RoomState& room, const QString& deviceId)` | 发送或发布 send room pairs ready for member 相关逻辑。 |
| [L110](../src/server/signaling/SignalServer.RoomPairs.inc#L110) | `SendRoomPairClosedToMember` | 定义 | `void SendRoomPairClosedToMember(const RoomState& room, const RoomPairState& pair, const QString& deviceId, const QString& initiatorDeviceId, const QString& reasonCode)` | 发送或发布 send room pair closed to member 相关逻辑。 |
| [L131](../src/server/signaling/SignalServer.RoomPairs.inc#L131) | `CloseRoomPair` | 定义 | `void CloseRoomPair(RoomState& room, const QString& pairId, const QString& initiatorDeviceId, const QString& reasonCode)` | 关闭并清理 close room pair 相关逻辑。 |
| [L152](../src/server/signaling/SignalServer.RoomPairs.inc#L152) | `CloseRoomPairsForMember` | 定义 | `void CloseRoomPairsForMember(RoomState& room, const QString& deviceId, const QString& initiatorDeviceId, const QString& reasonCode)` | 关闭并清理 close room pairs for member 相关逻辑。 |
| [L170](../src/server/signaling/SignalServer.RoomPairs.inc#L170) | `CloseAllRoomPairs` | 定义 | `void CloseAllRoomPairs(RoomState& room, const QString& initiatorDeviceId, const QString& reasonCode)` | 关闭并清理 close all room pairs 相关逻辑。 |

## `src/server/signaling/SignalServer.RoomState.inc`

[打开源码](../src/server/signaling/SignalServer.RoomState.inc) · **文件作用：** `SignalServer` 的实现切片，集中实现 room state 相关逻辑；成员状态仍定义在所属头文件中。

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L4](../src/server/signaling/SignalServer.RoomState.inc#L4) | `SerializeRoom` | 定义 | `QJsonObject SerializeRoom(const RoomState& room) const` | SPDX-License-Identifier: GPL-3.0-only Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd) |
| [L44](../src/server/signaling/SignalServer.RoomState.inc#L44) | `SendRoomReady` | 定义 | `void SendRoomReady(QWebSocket* socket, const RoomState& room, const QString& recoveryToken)` | 发送或发布 send room ready 相关逻辑。 |
| [L54](../src/server/signaling/SignalServer.RoomState.inc#L54) | `BroadcastRoomState` | 定义 | `void BroadcastRoomState(const RoomState& room)` | 实现 broadcast room state 对应的业务或工具逻辑。 |
| [L71](../src/server/signaling/SignalServer.RoomState.inc#L71) | `SendRoomJoinPending` | 定义 | `void SendRoomJoinPending(QWebSocket* socket, const QString& roomId, const QString& requestId)` | 发送或发布 send room join pending 相关逻辑。 |
| [L82](../src/server/signaling/SignalServer.RoomState.inc#L82) | `SendRoomJoinResult` | 定义 | `void SendRoomJoinResult(QWebSocket* socket, const QString& roomId, const QString& requestId, bool accepted, const QString& reasonCode, const QString& reasonMessage)` | 发送或发布 send room join result 相关逻辑。 |
| [L99](../src/server/signaling/SignalServer.RoomState.inc#L99) | `SendRoomClosed` | 定义 | `void SendRoomClosed(QWebSocket* socket, const QString& roomId, const QString& initiatorDeviceId, const QString& reasonCode)` | 发送或发布 send room closed 相关逻辑。 |
| [L112](../src/server/signaling/SignalServer.RoomState.inc#L112) | `CloseRoom` | 定义 | `void CloseRoom(const QString& roomId, const QString& initiatorDeviceId, const QString& reasonCode)` | 关闭并清理 close room 相关逻辑。 |
| [L169](../src/server/signaling/SignalServer.RoomState.inc#L169) | `CancelPendingRoomJoinForDevice` | 定义 | `void CancelPendingRoomJoinForDevice(const QString& deviceId)` | 判断 cancel pending room join for device 相关逻辑。 |

## `src/server/signaling/SignalServerMain.cpp`

[打开源码](../src/server/signaling/SignalServerMain.cpp) · **文件作用：** 实现 signal server main 相关函数与文件级辅助逻辑。

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L27](../src/server/signaling/SignalServerMain.cpp#L27) | `ReadSecret` | 定义 | `QByteArray ReadSecret(const QString& path, QString* error)` | 读取或恢复 read secret 相关逻辑。 |
| [L40](../src/server/signaling/SignalServerMain.cpp#L40) | `Fail` | 定义 | `int Fail(const QString& message)` | 实现 fail 对应的业务或工具逻辑。 |
| [L46](../src/server/signaling/SignalServerMain.cpp#L46) | `IsNineDigitPublicId` | 定义 | `bool IsNineDigitPublicId(const QString& value)` | 判断 is nine digit public id 相关逻辑。 |
| [L53](../src/server/signaling/SignalServerMain.cpp#L53) | `RunAuthStorageSelfTest` | 定义 | `int RunAuthStorageSelfTest()` | 执行后台循环或调度 run auth storage self test 相关逻辑。 |
| [L247](../src/server/signaling/SignalServerMain.cpp#L247) | `main` | 定义 | `int main(int argc, char* argv[])` | 实现 main 对应的业务或工具逻辑。 |

## `src/server/signaling/SignalServerSupport.cpp`

[打开源码](../src/server/signaling/SignalServerSupport.cpp) · **文件作用：** 实现 signal server support 相关函数与文件级辅助逻辑。

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L14](../src/server/signaling/SignalServerSupport.cpp#L14) | `SetError` | 定义 | `void SetError(QString* error, const QString& message)` | 更新或应用 set error 相关逻辑。 |
| [L23](../src/server/signaling/SignalServerSupport.cpp#L23) | `ReadFile` | 定义 | `bool ReadFile(const QString& path, QByteArray* contents, QString* error)` | 读取或恢复 read file 相关逻辑。 |
| [L37](../src/server/signaling/SignalServerSupport.cpp#L37) | `CreateRecoveryToken` | 定义 | `QString CreateRecoveryToken()` | 创建或初始化 create recovery token 相关逻辑。 |

## `src/server/signaling/SignalServerSupport.h`

[打开源码](../src/server/signaling/SignalServerSupport.h) · **文件作用：** 声明 signal server support 相关类型、接口、配置和成员状态。

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L11](../src/server/signaling/SignalServerSupport.h#L11) | `ReadFile` | 声明 | `bool ReadFile(const QString& path, QByteArray* contents, QString* error)` | 读取或恢复 read file 相关逻辑。 |
| [L14](../src/server/signaling/SignalServerSupport.h#L14) | `CreateRecoveryToken` | 声明 | `QString CreateRecoveryToken()` | 创建或初始化 create recovery token 相关逻辑。 |

## `src/server/signaling/SlidingWindowRateLimiter.h`

[打开源码](../src/server/signaling/SlidingWindowRateLimiter.h) · **文件作用：** 声明 sliding window rate limiter 相关类型、接口、配置和成员状态。

### 类型

| 行 | 类型 | 种类 | 作用 |
|---:|---|---|---|
| [L15](../src/server/signaling/SlidingWindowRateLimiter.h#L15) | `SlidingWindowRateLimitPolicy` | struct | 定义 SlidingWindowRateLimitPolicy 的 struct 类型和相关状态。 |
| [L25](../src/server/signaling/SlidingWindowRateLimiter.h#L25) | `SlidingWindowRateLimiter` | class | In-memory, single-threaded rate limiter for the signaling event loop. Each key keeps only the accepted events inside the active window, while the number of keys is capped to pre... |
| [L120](../src/server/signaling/SlidingWindowRateLimiter.h#L120) | `Bucket` | struct | 定义 Bucket 的 struct 类型和相关状态。 |

### 成员与文件级变量

| 行 | 变量 | 声明 | 作用 |
|---:|---|---|---|
| [L16](../src/server/signaling/SlidingWindowRateLimiter.h#L16) | `maximumCost` | `int maximumCost = 1;` | 保存 maximum cost 相关配置或运行状态。 |
| [L17](../src/server/signaling/SlidingWindowRateLimiter.h#L17) | `windowMs` | `qint64 windowMs = 60000;` | 保存 window ms 相关配置或运行状态。 |
| [L18](../src/server/signaling/SlidingWindowRateLimiter.h#L18) | `maximumKeys` | `int maximumKeys = 4096;` | 保存 maximum keys 相关配置或运行状态。 |
| [L121](../src/server/signaling/SlidingWindowRateLimiter.h#L121) | `events` | `QQueue<qint64> events;` | 保存 events 相关配置或运行状态。 |
| [L122](../src/server/signaling/SlidingWindowRateLimiter.h#L122) | `lastSeenMs` | `qint64 lastSeenMs = 0;` | 保存 last seen ms 相关配置或运行状态。 |
| [L172](../src/server/signaling/SlidingWindowRateLimiter.h#L172) | `policy_` | `SlidingWindowRateLimitPolicy policy_;` | 保存 policy 相关配置或运行状态。 |
| [L173](../src/server/signaling/SlidingWindowRateLimiter.h#L173) | `buckets_` | `QHash<QString, Bucket> buckets_;` | 保存 buckets 相关配置或运行状态。 |

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L27](../src/server/signaling/SlidingWindowRateLimiter.h#L27) | `SlidingWindowRateLimiter` | 定义 | `explicit SlidingWindowRateLimiter( SlidingWindowRateLimitPolicy policy) : policy_(policy) {}` | 实现 sliding window rate limiter 对应的业务或工具逻辑。 |
| [L32](../src/server/signaling/SlidingWindowRateLimiter.h#L32) | `CanAcquire` | 定义 | `bool CanAcquire(const QString& key, int cost, qint64 nowMs, qint64* retryAfterMs = nullptr)` | 判断 can acquire 相关逻辑。 |
| [L66](../src/server/signaling/SlidingWindowRateLimiter.h#L66) | `Record` | 定义 | `void Record(const QString& key, int cost, qint64 nowMs)` | 实现 record 对应的业务或工具逻辑。 |
| [L82](../src/server/signaling/SlidingWindowRateLimiter.h#L82) | `TryAcquire` | 定义 | `bool TryAcquire(const QString& key, int cost, qint64 nowMs, qint64* retryAfterMs = nullptr)` | 实现 try acquire 对应的业务或工具逻辑。 |
| [L94](../src/server/signaling/SlidingWindowRateLimiter.h#L94) | `Prune` | 定义 | `void Prune(qint64 nowMs)` | 实现 prune 对应的业务或工具逻辑。 |
| [L109](../src/server/signaling/SlidingWindowRateLimiter.h#L109) | `Clear` | 定义 | `void Clear()` | 重置或移除 clear 相关逻辑。 |
| [L114](../src/server/signaling/SlidingWindowRateLimiter.h#L114) | `keyCount` | 定义 | `qsizetype keyCount() const` | 实现 key count 对应的业务或工具逻辑。 |
| [L125](../src/server/signaling/SlidingWindowRateLimiter.h#L125) | `SetRetryAfter` | 定义 | `static void SetRetryAfter(qint64* destination, qint64 value)` | 更新或应用 set retry after 相关逻辑。 |
| [L132](../src/server/signaling/SlidingWindowRateLimiter.h#L132) | `Expire` | 定义 | `void Expire(Bucket& bucket, qint64 nowMs) const` | 实现 expire 对应的业务或工具逻辑。 |
| [L141](../src/server/signaling/SlidingWindowRateLimiter.h#L141) | `EnsureCapacityFor` | 定义 | `void EnsureCapacityFor(const QString& key, qint64 nowMs)` | 实现 ensure capacity for 对应的业务或工具逻辑。 |
