# Windows 平台能力

> 自动生成于 2026-09-06，源码树 `cfeee508b14c-dirty`。请运行 `tools/Generate-SourceSymbolReference.ps1` 刷新。

DXGI/D3D11、Windows 输入、剪贴板、文件、摄像头、MFT/FFmpeg 编解码和硬件探测。

本册共收录 58 个源码文件。函数与变量的中文作用优先采用源码紧邻注释；无注释时根据符号命名生成阅读提示，最终语义仍以源码为准。

## `src/platform/win/D3D11DesktopFrameBuffer.cpp`

[打开源码](../src/platform/win/D3D11DesktopFrameBuffer.cpp) · **文件作用：** 实现 d3 d11 desktop frame buffer 相关函数与文件级辅助逻辑。

### 类型

| 行 | 类型 | 种类 | 作用 |
|---:|---|---|---|
| [L32](../src/platform/win/D3D11DesktopFrameBuffer.cpp#L32) | `D3D11DesktopFrameBuffer::Impl` | class | 定义 D3D11DesktopFrameBuffer::Impl 的 class 类型和相关状态。 |

### 成员与文件级变量

| 行 | 变量 | 声明 | 作用 |
|---:|---|---|---|
| [L25](../src/platform/win/D3D11DesktopFrameBuffer.cpp#L25) | `kStorageRepresentation` | `constexpr char kStorageRepresentation[] = "D3D11_BGRA_DESKTOP";` | 定义 storage representation 的编译期常量或产品边界。 |

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L43](../src/platform/win/D3D11DesktopFrameBuffer.cpp#L43) | `texture` | 定义 | `: texture(texture), sourceWidth(sourceWidth), sourceHeight(sourceHeight), cropX(cropX), cropY(cropY), cropWidth(cropWidth), cropHeight(cropHeight), outputWidth(outputWidth), outputHeight(outputHeight)` | 实现 texture 对应的业务或工具逻辑。 |
| [L70](../src/platform/win/D3D11DesktopFrameBuffer.cpp#L70) | `D3D11DesktopFrameBuffer::D3D11DesktopFrameBuffer` | 定义 | `D3D11DesktopFrameBuffer::D3D11DesktopFrameBuffer( ID3D11Texture2D* texture, int sourceWidth, int sourceHeight) : D3D11DesktopFrameBuffer( texture, sourceWidth, sourceHeight, 0, 0, sourceWidth, sourceHeight, sourceWidt...` | 构造并初始化 D3D11DesktopFrameBuffer 实例。 |
| [L90](../src/platform/win/D3D11DesktopFrameBuffer.cpp#L90) | `impl_` | 定义 | `: impl_(std::make_unique<Impl>( texture, sourceWidth, sourceHeight, cropX, cropY, cropWidth, cropHeight, outputWidth, outputHeight)) {}` | 实现 impl 对应的业务或工具逻辑。 |
| [L98](../src/platform/win/D3D11DesktopFrameBuffer.cpp#L98) | `D3D11DesktopFrameBuffer::type` | 定义 | `webrtc::VideoFrameBuffer::Type D3D11DesktopFrameBuffer::type() const` | 实现 type 对应的业务或工具逻辑。 |
| [L103](../src/platform/win/D3D11DesktopFrameBuffer.cpp#L103) | `D3D11DesktopFrameBuffer::width` | 定义 | `int D3D11DesktopFrameBuffer::width() const` | 实现 width 对应的业务或工具逻辑。 |
| [L108](../src/platform/win/D3D11DesktopFrameBuffer.cpp#L108) | `D3D11DesktopFrameBuffer::height` | 定义 | `int D3D11DesktopFrameBuffer::height() const` | 实现 height 对应的业务或工具逻辑。 |
| [L114](../src/platform/win/D3D11DesktopFrameBuffer.cpp#L114) | `D3D11DesktopFrameBuffer::ToI420` | 定义 | `D3D11DesktopFrameBuffer::ToI420()` | 实现 to i420 对应的业务或工具逻辑。 |
| [L189](../src/platform/win/D3D11DesktopFrameBuffer.cpp#L189) | `D3D11DesktopFrameBuffer::CropAndScale` | 定义 | `D3D11DesktopFrameBuffer::CropAndScale( int offsetX, int offsetY, int cropWidth, int cropHeight, int scaledWidth, int scaledHeight)` | 转换或缩放 crop and scale 相关逻辑。 |
| [L231](../src/platform/win/D3D11DesktopFrameBuffer.cpp#L231) | `D3D11DesktopFrameBuffer::storage_representation` | 定义 | `std::string D3D11DesktopFrameBuffer::storage_representation() const` | 实现 storage representation 对应的业务或工具逻辑。 |
| [L236](../src/platform/win/D3D11DesktopFrameBuffer.cpp#L236) | `D3D11DesktopFrameBuffer::Texture` | 定义 | `ID3D11Texture2D* D3D11DesktopFrameBuffer::Texture() const noexcept` | 实现 texture 对应的业务或工具逻辑。 |
| [L241](../src/platform/win/D3D11DesktopFrameBuffer.cpp#L241) | `D3D11DesktopFrameBuffer::Device` | 定义 | `ID3D11Device* D3D11DesktopFrameBuffer::Device() const noexcept` | 实现 device 对应的业务或工具逻辑。 |
| [L246](../src/platform/win/D3D11DesktopFrameBuffer.cpp#L246) | `D3D11DesktopFrameBuffer::SourceWidth` | 定义 | `int D3D11DesktopFrameBuffer::SourceWidth() const noexcept` | 实现 source width 对应的业务或工具逻辑。 |
| [L251](../src/platform/win/D3D11DesktopFrameBuffer.cpp#L251) | `D3D11DesktopFrameBuffer::SourceHeight` | 定义 | `int D3D11DesktopFrameBuffer::SourceHeight() const noexcept` | 实现 source height 对应的业务或工具逻辑。 |
| [L256](../src/platform/win/D3D11DesktopFrameBuffer.cpp#L256) | `D3D11DesktopFrameBuffer::CropX` | 定义 | `int D3D11DesktopFrameBuffer::CropX() const noexcept` | 转换或缩放 crop x 相关逻辑。 |
| [L261](../src/platform/win/D3D11DesktopFrameBuffer.cpp#L261) | `D3D11DesktopFrameBuffer::CropY` | 定义 | `int D3D11DesktopFrameBuffer::CropY() const noexcept` | 转换或缩放 crop y 相关逻辑。 |
| [L266](../src/platform/win/D3D11DesktopFrameBuffer.cpp#L266) | `D3D11DesktopFrameBuffer::CropWidth` | 定义 | `int D3D11DesktopFrameBuffer::CropWidth() const noexcept` | 转换或缩放 crop width 相关逻辑。 |
| [L271](../src/platform/win/D3D11DesktopFrameBuffer.cpp#L271) | `D3D11DesktopFrameBuffer::CropHeight` | 定义 | `int D3D11DesktopFrameBuffer::CropHeight() const noexcept` | 转换或缩放 crop height 相关逻辑。 |
| [L276](../src/platform/win/D3D11DesktopFrameBuffer.cpp#L276) | `D3D11DesktopFrameBuffer::From` | 定义 | `D3D11DesktopFrameBuffer* D3D11DesktopFrameBuffer::From( webrtc::VideoFrameBuffer* buffer)` | 实现 from 对应的业务或工具逻辑。 |
| [L287](../src/platform/win/D3D11DesktopFrameBuffer.cpp#L287) | `D3D11DesktopFrameBuffer::From` | 定义 | `const D3D11DesktopFrameBuffer* D3D11DesktopFrameBuffer::From( const webrtc::VideoFrameBuffer* buffer)` | 实现 from 对应的业务或工具逻辑。 |
| [L293](../src/platform/win/D3D11DesktopFrameBuffer.cpp#L293) | `PublishDesktopCaptureD3D11Device` | 定义 | `void PublishDesktopCaptureD3D11Device(ID3D11Device* device)` | 发送或发布 publish desktop capture d3 d11 device 相关逻辑。 |
| [L299](../src/platform/win/D3D11DesktopFrameBuffer.cpp#L299) | `AcquireDesktopCaptureD3D11Device` | 定义 | `ID3D11Device* AcquireDesktopCaptureD3D11Device()` | 采集 acquire desktop capture d3 d11 device 相关逻辑。 |

## `src/platform/win/D3D11DesktopFrameBuffer.h`

[打开源码](../src/platform/win/D3D11DesktopFrameBuffer.h) · **文件作用：** 声明 d3 d11 desktop frame buffer 相关类型、接口、配置和成员状态。

### 类型

| 行 | 类型 | 种类 | 作用 |
|---:|---|---|---|
| [L12](../src/platform/win/D3D11DesktopFrameBuffer.h#L12) | `ID3D11Device` | struct | 定义 ID3D11Device 的 struct 类型和相关状态。 |
| [L13](../src/platform/win/D3D11DesktopFrameBuffer.h#L13) | `ID3D11Texture2D` | struct | 定义 ID3D11Texture2D 的 struct 类型和相关状态。 |
| [L21](../src/platform/win/D3D11DesktopFrameBuffer.h#L21) | `D3D11DesktopFrameBuffer` | class | Immutable BGRA desktop texture captured through DXGI Desktop Duplication. CropAndScale keeps a lazy description so each sender can use the same GPU texture while applying its ow... |
| [L67](../src/platform/win/D3D11DesktopFrameBuffer.h#L67) | `Impl` | class | 定义 Impl 的 class 类型和相关状态。 |

### 成员与文件级变量

| 行 | 变量 | 声明 | 作用 |
|---:|---|---|---|
| [L12](../src/platform/win/D3D11DesktopFrameBuffer.h#L12) | `ID3D11Device` | `struct ID3D11Device;` | 保存 id3 d11 device 相关配置或运行状态。 |
| [L13](../src/platform/win/D3D11DesktopFrameBuffer.h#L13) | `ID3D11Texture2D` | `struct ID3D11Texture2D;` | 保存 id3 d11 texture2 d 相关配置或运行状态。 |
| [L67](../src/platform/win/D3D11DesktopFrameBuffer.h#L67) | `Impl` | `class Impl;` | 保存 impl 相关配置或运行状态。 |
| [L68](../src/platform/win/D3D11DesktopFrameBuffer.h#L68) | `impl_` | `std::unique_ptr<Impl> impl_;` | 保存 impl 相关配置或运行状态。 |

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L23](../src/platform/win/D3D11DesktopFrameBuffer.h#L23) | `D3D11DesktopFrameBuffer` | 声明 | `D3D11DesktopFrameBuffer( ID3D11Texture2D* texture, int sourceWidth, int sourceHeight)` | 实现 d3 d11 desktop frame buffer 对应的业务或工具逻辑。 |
| [L27](../src/platform/win/D3D11DesktopFrameBuffer.h#L27) | `D3D11DesktopFrameBuffer` | 声明 | `D3D11DesktopFrameBuffer( ID3D11Texture2D* texture, int sourceWidth, int sourceHeight, int cropX, int cropY, int cropWidth, int cropHeight, int outputWidth, int outputHeight)` | 实现 d3 d11 desktop frame buffer 对应的业务或工具逻辑。 |
| [L37](../src/platform/win/D3D11DesktopFrameBuffer.h#L37) | `~D3D11DesktopFrameBuffer` | 声明 | `~D3D11DesktopFrameBuffer() override` | 停止相关活动并释放 D3D11DesktopFrameBuffer 实例拥有的资源。 |
| [L39](../src/platform/win/D3D11DesktopFrameBuffer.h#L39) | `type` | 声明 | `Type type() const override` | 实现 type 对应的业务或工具逻辑。 |
| [L40](../src/platform/win/D3D11DesktopFrameBuffer.h#L40) | `width` | 声明 | `int width() const override` | 实现 width 对应的业务或工具逻辑。 |
| [L41](../src/platform/win/D3D11DesktopFrameBuffer.h#L41) | `height` | 声明 | `int height() const override` | 实现 height 对应的业务或工具逻辑。 |
| [L42](../src/platform/win/D3D11DesktopFrameBuffer.h#L42) | `ToI420` | 声明 | `webrtc::scoped_refptr<webrtc::I420BufferInterface> ToI420() override` | 实现 to i420 对应的业务或工具逻辑。 |
| [L43](../src/platform/win/D3D11DesktopFrameBuffer.h#L43) | `CropAndScale` | 声明 | `webrtc::scoped_refptr<webrtc::VideoFrameBuffer> CropAndScale( int offsetX, int offsetY, int cropWidth, int cropHeight, int scaledWidth, int scaledHeight) override` | 转换或缩放 crop and scale 相关逻辑。 |
| [L50](../src/platform/win/D3D11DesktopFrameBuffer.h#L50) | `storage_representation` | 声明 | `std::string storage_representation() const override` | 实现 storage representation 对应的业务或工具逻辑。 |
| [L52](../src/platform/win/D3D11DesktopFrameBuffer.h#L52) | `Texture` | 声明 | `ID3D11Texture2D* Texture() const noexcept` | 实现 texture 对应的业务或工具逻辑。 |
| [L53](../src/platform/win/D3D11DesktopFrameBuffer.h#L53) | `Device` | 声明 | `ID3D11Device* Device() const noexcept` | 实现 device 对应的业务或工具逻辑。 |
| [L54](../src/platform/win/D3D11DesktopFrameBuffer.h#L54) | `SourceWidth` | 声明 | `int SourceWidth() const noexcept` | 实现 source width 对应的业务或工具逻辑。 |
| [L55](../src/platform/win/D3D11DesktopFrameBuffer.h#L55) | `SourceHeight` | 声明 | `int SourceHeight() const noexcept` | 实现 source height 对应的业务或工具逻辑。 |
| [L56](../src/platform/win/D3D11DesktopFrameBuffer.h#L56) | `CropX` | 声明 | `int CropX() const noexcept` | 转换或缩放 crop x 相关逻辑。 |
| [L57](../src/platform/win/D3D11DesktopFrameBuffer.h#L57) | `CropY` | 声明 | `int CropY() const noexcept` | 转换或缩放 crop y 相关逻辑。 |
| [L58](../src/platform/win/D3D11DesktopFrameBuffer.h#L58) | `CropWidth` | 声明 | `int CropWidth() const noexcept` | 转换或缩放 crop width 相关逻辑。 |
| [L59](../src/platform/win/D3D11DesktopFrameBuffer.h#L59) | `CropHeight` | 声明 | `int CropHeight() const noexcept` | 转换或缩放 crop height 相关逻辑。 |
| [L61](../src/platform/win/D3D11DesktopFrameBuffer.h#L61) | `From` | 声明 | `static D3D11DesktopFrameBuffer* From( webrtc::VideoFrameBuffer* buffer)` | 实现 from 对应的业务或工具逻辑。 |
| [L63](../src/platform/win/D3D11DesktopFrameBuffer.h#L63) | `From` | 声明 | `static const D3D11DesktopFrameBuffer* From( const webrtc::VideoFrameBuffer* buffer)` | 实现 from 对应的业务或工具逻辑。 |
| [L75](../src/platform/win/D3D11DesktopFrameBuffer.h#L75) | `PublishDesktopCaptureD3D11Device` | 声明 | `void PublishDesktopCaptureD3D11Device(ID3D11Device* device)` | The desktop capturer publishes its D3D11 device before the WebRTC encoder is created. The MFT encoder acquires a reference to the same device, eliminating cross-device texture s... |
| [L76](../src/platform/win/D3D11DesktopFrameBuffer.h#L76) | `AcquireDesktopCaptureD3D11Device` | 声明 | `ID3D11Device* AcquireDesktopCaptureD3D11Device()` | 采集 acquire desktop capture d3 d11 device 相关逻辑。 |

## `src/platform/win/D3D11NativeFrameBuffer.cpp`

[打开源码](../src/platform/win/D3D11NativeFrameBuffer.cpp) · **文件作用：** 实现 d3 d11 native frame buffer 相关函数与文件级辅助逻辑。

### 类型

| 行 | 类型 | 种类 | 作用 |
|---:|---|---|---|
| [L27](../src/platform/win/D3D11NativeFrameBuffer.cpp#L27) | `D3D11NativeFrameBuffer::Impl` | class | 定义 D3D11NativeFrameBuffer::Impl 的 class 类型和相关状态。 |

### 成员与文件级变量

| 行 | 变量 | 声明 | 作用 |
|---:|---|---|---|
| [L21](../src/platform/win/D3D11NativeFrameBuffer.cpp#L21) | `kD3D11StorageRepresentation` | `constexpr char kD3D11StorageRepresentation[] = "D3D11 NV12 texture";` | 定义 d3 d11 storage representation 的编译期常量或产品边界。 |

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L29](../src/platform/win/D3D11NativeFrameBuffer.cpp#L29) | `Impl` | 定义 | `Impl(IMFSample* sample, ID3D11Texture2D* texture, uint32_t subresourceIndex, int visibleWidth, int visibleHeight) : sample_(sample), texture_(texture), subresourceIndex_(subresourceIndex), visibleWidth_(visibleWidth),...` | 实现 impl 对应的业务或工具逻辑。 |
| [L45](../src/platform/win/D3D11NativeFrameBuffer.cpp#L45) | `Impl` | 定义 | `Impl(std::shared_ptr<void> lifetimeOwner, ID3D11Texture2D* texture, uint32_t subresourceIndex, int visibleWidth, int visibleHeight) : lifetimeOwner_(std::move(lifetimeOwner)), texture_(texture), subresourceIndex_(subr...` | 实现 impl 对应的业务或工具逻辑。 |
| [L61](../src/platform/win/D3D11NativeFrameBuffer.cpp#L61) | `ToI420` | 定义 | `webrtc::scoped_refptr<webrtc::I420BufferInterface> ToI420()` | 实现 to i420 对应的业务或工具逻辑。 |
| [L162](../src/platform/win/D3D11NativeFrameBuffer.cpp#L162) | `impl_` | 定义 | `: impl_(std::make_unique<Impl>( sample, texture, subresourceIndex, visibleWidth, visibleHeight)) {}` | 实现 impl 对应的业务或工具逻辑。 |
| [L176](../src/platform/win/D3D11NativeFrameBuffer.cpp#L176) | `impl_` | 定义 | `: impl_(std::make_unique<Impl>( std::move(lifetimeOwner), texture, subresourceIndex, visibleWidth, visibleHeight)) {}` | 实现 impl 对应的业务或工具逻辑。 |
| [L186](../src/platform/win/D3D11NativeFrameBuffer.cpp#L186) | `D3D11NativeFrameBuffer::type` | 定义 | `webrtc::VideoFrameBuffer::Type D3D11NativeFrameBuffer::type() const` | 实现 type 对应的业务或工具逻辑。 |
| [L191](../src/platform/win/D3D11NativeFrameBuffer.cpp#L191) | `D3D11NativeFrameBuffer::width` | 定义 | `int D3D11NativeFrameBuffer::width() const` | 实现 width 对应的业务或工具逻辑。 |
| [L196](../src/platform/win/D3D11NativeFrameBuffer.cpp#L196) | `D3D11NativeFrameBuffer::height` | 定义 | `int D3D11NativeFrameBuffer::height() const` | 实现 height 对应的业务或工具逻辑。 |
| [L202](../src/platform/win/D3D11NativeFrameBuffer.cpp#L202) | `D3D11NativeFrameBuffer::ToI420` | 定义 | `D3D11NativeFrameBuffer::ToI420()` | 实现 to i420 对应的业务或工具逻辑。 |
| [L207](../src/platform/win/D3D11NativeFrameBuffer.cpp#L207) | `D3D11NativeFrameBuffer::storage_representation` | 定义 | `std::string D3D11NativeFrameBuffer::storage_representation() const` | 实现 storage representation 对应的业务或工具逻辑。 |
| [L212](../src/platform/win/D3D11NativeFrameBuffer.cpp#L212) | `D3D11NativeFrameBuffer::Texture` | 定义 | `ID3D11Texture2D* D3D11NativeFrameBuffer::Texture() const noexcept` | 实现 texture 对应的业务或工具逻辑。 |
| [L217](../src/platform/win/D3D11NativeFrameBuffer.cpp#L217) | `D3D11NativeFrameBuffer::Device` | 定义 | `ID3D11Device* D3D11NativeFrameBuffer::Device() const noexcept` | 实现 device 对应的业务或工具逻辑。 |
| [L222](../src/platform/win/D3D11NativeFrameBuffer.cpp#L222) | `D3D11NativeFrameBuffer::SubresourceIndex` | 定义 | `uint32_t D3D11NativeFrameBuffer::SubresourceIndex() const noexcept` | 实现 subresource index 对应的业务或工具逻辑。 |
| [L227](../src/platform/win/D3D11NativeFrameBuffer.cpp#L227) | `D3D11NativeFrameBuffer::From` | 定义 | `D3D11NativeFrameBuffer* D3D11NativeFrameBuffer::From( webrtc::VideoFrameBuffer* buffer)` | 实现 from 对应的业务或工具逻辑。 |
| [L239](../src/platform/win/D3D11NativeFrameBuffer.cpp#L239) | `D3D11NativeFrameBuffer::From` | 定义 | `const D3D11NativeFrameBuffer* D3D11NativeFrameBuffer::From( const webrtc::VideoFrameBuffer* buffer)` | 实现 from 对应的业务或工具逻辑。 |

## `src/platform/win/D3D11NativeFrameBuffer.h`

[打开源码](../src/platform/win/D3D11NativeFrameBuffer.h) · **文件作用：** 声明 d3 d11 native frame buffer 相关类型、接口、配置和成员状态。

### 类型

| 行 | 类型 | 种类 | 作用 |
|---:|---|---|---|
| [L12](../src/platform/win/D3D11NativeFrameBuffer.h#L12) | `ID3D11Device` | struct | 定义 ID3D11Device 的 struct 类型和相关状态。 |
| [L13](../src/platform/win/D3D11NativeFrameBuffer.h#L13) | `ID3D11Texture2D` | struct | 定义 ID3D11Texture2D 的 struct 类型和相关状态。 |
| [L14](../src/platform/win/D3D11NativeFrameBuffer.h#L14) | `IMFSample` | struct | 定义 IMFSample 的 struct 类型和相关状态。 |
| [L21](../src/platform/win/D3D11NativeFrameBuffer.h#L21) | `D3D11NativeFrameBuffer` | class | Owns a decoder-specific lifetime token and its D3D11 NV12 texture. Keeping the token alive prevents either Media Foundation or FFmpeg from recycling the decode surface before th... |
| [L55](../src/platform/win/D3D11NativeFrameBuffer.h#L55) | `Impl` | class | 定义 Impl 的 class 类型和相关状态。 |

### 成员与文件级变量

| 行 | 变量 | 声明 | 作用 |
|---:|---|---|---|
| [L12](../src/platform/win/D3D11NativeFrameBuffer.h#L12) | `ID3D11Device` | `struct ID3D11Device;` | 保存 id3 d11 device 相关配置或运行状态。 |
| [L13](../src/platform/win/D3D11NativeFrameBuffer.h#L13) | `ID3D11Texture2D` | `struct ID3D11Texture2D;` | 保存 id3 d11 texture2 d 相关配置或运行状态。 |
| [L14](../src/platform/win/D3D11NativeFrameBuffer.h#L14) | `IMFSample` | `struct IMFSample;` | 保存 imf sample 相关配置或运行状态。 |
| [L55](../src/platform/win/D3D11NativeFrameBuffer.h#L55) | `Impl` | `class Impl;` | 保存 impl 相关配置或运行状态。 |
| [L56](../src/platform/win/D3D11NativeFrameBuffer.h#L56) | `impl_` | `std::unique_ptr<Impl> impl_;` | 保存 impl 相关配置或运行状态。 |

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L23](../src/platform/win/D3D11NativeFrameBuffer.h#L23) | `D3D11NativeFrameBuffer` | 声明 | `D3D11NativeFrameBuffer(IMFSample* sample, ID3D11Texture2D* texture, uint32_t subresourceIndex, int visibleWidth, int visibleHeight)` | 实现 d3 d11 native frame buffer 对应的业务或工具逻辑。 |
| [L28](../src/platform/win/D3D11NativeFrameBuffer.h#L28) | `D3D11NativeFrameBuffer` | 声明 | `D3D11NativeFrameBuffer(std::shared_ptr<void> lifetimeOwner, ID3D11Texture2D* texture, uint32_t subresourceIndex, int visibleWidth, int visibleHeight)` | 实现 d3 d11 native frame buffer 对应的业务或工具逻辑。 |
| [L33](../src/platform/win/D3D11NativeFrameBuffer.h#L33) | `~D3D11NativeFrameBuffer` | 声明 | `~D3D11NativeFrameBuffer() override` | 停止相关活动并释放 D3D11NativeFrameBuffer 实例拥有的资源。 |
| [L35](../src/platform/win/D3D11NativeFrameBuffer.h#L35) | `type` | 声明 | `Type type() const override` | 实现 type 对应的业务或工具逻辑。 |
| [L36](../src/platform/win/D3D11NativeFrameBuffer.h#L36) | `width` | 声明 | `int width() const override` | 实现 width 对应的业务或工具逻辑。 |
| [L37](../src/platform/win/D3D11NativeFrameBuffer.h#L37) | `height` | 声明 | `int height() const override` | 实现 height 对应的业务或工具逻辑。 |
| [L38](../src/platform/win/D3D11NativeFrameBuffer.h#L38) | `ToI420` | 声明 | `webrtc::scoped_refptr<webrtc::I420BufferInterface> ToI420() override` | 实现 to i420 对应的业务或工具逻辑。 |
| [L39](../src/platform/win/D3D11NativeFrameBuffer.h#L39) | `storage_representation` | 声明 | `std::string storage_representation() const override` | 实现 storage representation 对应的业务或工具逻辑。 |
| [L41](../src/platform/win/D3D11NativeFrameBuffer.h#L41) | `Texture` | 声明 | `ID3D11Texture2D* Texture() const noexcept` | 实现 texture 对应的业务或工具逻辑。 |
| [L42](../src/platform/win/D3D11NativeFrameBuffer.h#L42) | `Device` | 声明 | `ID3D11Device* Device() const noexcept` | 实现 device 对应的业务或工具逻辑。 |
| [L43](../src/platform/win/D3D11NativeFrameBuffer.h#L43) | `SubresourceIndex` | 声明 | `uint32_t SubresourceIndex() const noexcept` | 实现 subresource index 对应的业务或工具逻辑。 |
| [L49](../src/platform/win/D3D11NativeFrameBuffer.h#L49) | `From` | 声明 | `static D3D11NativeFrameBuffer* From( webrtc::VideoFrameBuffer* buffer)` | libwebrtc is built without C++ RTTI, so dynamic_cast must never be used on VideoFrameBuffer instances produced by its software decoder. Identify our native buffer through WebRTC... |
| [L51](../src/platform/win/D3D11NativeFrameBuffer.h#L51) | `From` | 声明 | `static const D3D11NativeFrameBuffer* From( const webrtc::VideoFrameBuffer* buffer)` | 实现 from 对应的业务或工具逻辑。 |

## `src/platform/win/DesktopBgraFrameBuffer.cpp`

[打开源码](../src/platform/win/DesktopBgraFrameBuffer.cpp) · **文件作用：** 实现 desktop bgra frame buffer 相关函数与文件级辅助逻辑。

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L20](../src/platform/win/DesktopBgraFrameBuffer.cpp#L20) | `width_` | 定义 | `: width_(width), height_(height), sourceWidth_(width), sourceHeight_(height), sourceStride_(width * 4), cropWidth_(width), cropHeight_(height), pixels_(std::make_shared<std::vector<uint8_t>>( static_cast<size_t>(sourc...` | 实现 width 对应的业务或工具逻辑。 |
| [L49](../src/platform/win/DesktopBgraFrameBuffer.cpp#L49) | `width_` | 定义 | `: width_(outputWidth), height_(outputHeight), sourceWidth_(sourceWidth), sourceHeight_(sourceHeight), sourceStride_(sourceStride), cropX_(cropX), cropY_(cropY), cropWidth_(cropWidth), cropHeight_(cropHeight), pixels_(...` | 实现 width 对应的业务或工具逻辑。 |
| [L61](../src/platform/win/DesktopBgraFrameBuffer.cpp#L61) | `DesktopBgraFrameBuffer::type` | 定义 | `webrtc::VideoFrameBuffer::Type DesktopBgraFrameBuffer::type() const` | 实现 type 对应的业务或工具逻辑。 |
| [L66](../src/platform/win/DesktopBgraFrameBuffer.cpp#L66) | `DesktopBgraFrameBuffer::width` | 定义 | `int DesktopBgraFrameBuffer::width() const` | 实现 width 对应的业务或工具逻辑。 |
| [L71](../src/platform/win/DesktopBgraFrameBuffer.cpp#L71) | `DesktopBgraFrameBuffer::height` | 定义 | `int DesktopBgraFrameBuffer::height() const` | 实现 height 对应的业务或工具逻辑。 |
| [L77](../src/platform/win/DesktopBgraFrameBuffer.cpp#L77) | `DesktopBgraFrameBuffer::ToI420` | 定义 | `DesktopBgraFrameBuffer::ToI420()` | 实现 to i420 对应的业务或工具逻辑。 |
| [L112](../src/platform/win/DesktopBgraFrameBuffer.cpp#L112) | `DesktopBgraFrameBuffer::CropAndScale` | 定义 | `DesktopBgraFrameBuffer::CropAndScale( int offsetX, int offsetY, int cropWidth, int cropHeight, int scaledWidth, int scaledHeight)` | 转换或缩放 crop and scale 相关逻辑。 |
| [L151](../src/platform/win/DesktopBgraFrameBuffer.cpp#L151) | `DesktopBgraFrameBuffer::storage_representation` | 定义 | `std::string DesktopBgraFrameBuffer::storage_representation() const` | 实现 storage representation 对应的业务或工具逻辑。 |
| [L156](../src/platform/win/DesktopBgraFrameBuffer.cpp#L156) | `DesktopBgraFrameBuffer::Data` | 定义 | `const uint8_t* DesktopBgraFrameBuffer::Data() const noexcept` | 实现 data 对应的业务或工具逻辑。 |
| [L161](../src/platform/win/DesktopBgraFrameBuffer.cpp#L161) | `DesktopBgraFrameBuffer::Stride` | 定义 | `int DesktopBgraFrameBuffer::Stride() const noexcept` | 实现 stride 对应的业务或工具逻辑。 |
| [L166](../src/platform/win/DesktopBgraFrameBuffer.cpp#L166) | `DesktopBgraFrameBuffer::SourceWidth` | 定义 | `int DesktopBgraFrameBuffer::SourceWidth() const noexcept { return sourceWidth_; }` | 实现 source width 对应的业务或工具逻辑。 |
| [L167](../src/platform/win/DesktopBgraFrameBuffer.cpp#L167) | `DesktopBgraFrameBuffer::SourceHeight` | 定义 | `int DesktopBgraFrameBuffer::SourceHeight() const noexcept { return sourceHeight_; }` | 实现 source height 对应的业务或工具逻辑。 |
| [L168](../src/platform/win/DesktopBgraFrameBuffer.cpp#L168) | `DesktopBgraFrameBuffer::CropX` | 定义 | `int DesktopBgraFrameBuffer::CropX() const noexcept { return cropX_; }` | 转换或缩放 crop x 相关逻辑。 |
| [L169](../src/platform/win/DesktopBgraFrameBuffer.cpp#L169) | `DesktopBgraFrameBuffer::CropY` | 定义 | `int DesktopBgraFrameBuffer::CropY() const noexcept { return cropY_; }` | 转换或缩放 crop y 相关逻辑。 |
| [L170](../src/platform/win/DesktopBgraFrameBuffer.cpp#L170) | `DesktopBgraFrameBuffer::CropWidth` | 定义 | `int DesktopBgraFrameBuffer::CropWidth() const noexcept { return cropWidth_; }` | 转换或缩放 crop width 相关逻辑。 |
| [L171](../src/platform/win/DesktopBgraFrameBuffer.cpp#L171) | `DesktopBgraFrameBuffer::CropHeight` | 定义 | `int DesktopBgraFrameBuffer::CropHeight() const noexcept { return cropHeight_; }` | 转换或缩放 crop height 相关逻辑。 |

## `src/platform/win/DesktopBgraFrameBuffer.h`

[打开源码](../src/platform/win/DesktopBgraFrameBuffer.h) · **文件作用：** 声明 desktop bgra frame buffer 相关类型、接口、配置和成员状态。

### 类型

| 行 | 类型 | 种类 | 作用 |
|---:|---|---|---|
| [L17](../src/platform/win/DesktopBgraFrameBuffer.h#L17) | `DesktopBgraFrameBuffer` | class | A desktop frame that keeps the capturer's native BGRA pixels until the selected encoder is known. The Media Foundation encoder uploads these pixels directly to D3D11; software e... |

### 成员与文件级变量

| 行 | 变量 | 声明 | 作用 |
|---:|---|---|---|
| [L57](../src/platform/win/DesktopBgraFrameBuffer.h#L57) | `width_` | `int width_ = 0;` | 保存计数、尺寸或速率指标：width。 |
| [L58](../src/platform/win/DesktopBgraFrameBuffer.h#L58) | `height_` | `int height_ = 0;` | 保存计数、尺寸或速率指标：height。 |
| [L59](../src/platform/win/DesktopBgraFrameBuffer.h#L59) | `sourceWidth_` | `int sourceWidth_ = 0;` | 保存计数、尺寸或速率指标：source width。 |
| [L60](../src/platform/win/DesktopBgraFrameBuffer.h#L60) | `sourceHeight_` | `int sourceHeight_ = 0;` | 保存计数、尺寸或速率指标：source height。 |
| [L61](../src/platform/win/DesktopBgraFrameBuffer.h#L61) | `sourceStride_` | `int sourceStride_ = 0;` | 保存 source stride 相关配置或运行状态。 |
| [L62](../src/platform/win/DesktopBgraFrameBuffer.h#L62) | `cropX_` | `int cropX_ = 0;` | 保存 crop x 相关配置或运行状态。 |
| [L63](../src/platform/win/DesktopBgraFrameBuffer.h#L63) | `cropY_` | `int cropY_ = 0;` | 保存 crop y 相关配置或运行状态。 |
| [L64](../src/platform/win/DesktopBgraFrameBuffer.h#L64) | `cropWidth_` | `int cropWidth_ = 0;` | 保存计数、尺寸或速率指标：crop width。 |
| [L65](../src/platform/win/DesktopBgraFrameBuffer.h#L65) | `cropHeight_` | `int cropHeight_ = 0;` | 保存计数、尺寸或速率指标：crop height。 |
| [L66](../src/platform/win/DesktopBgraFrameBuffer.h#L66) | `pixels_` | `std::shared_ptr<std::vector<uint8_t>> pixels_;` | 保存 pixels 相关配置或运行状态。 |

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L19](../src/platform/win/DesktopBgraFrameBuffer.h#L19) | `DesktopBgraFrameBuffer` | 声明 | `DesktopBgraFrameBuffer(const uint8_t* data, int stride, int width, int height)` | 实现 desktop bgra frame buffer 对应的业务或工具逻辑。 |
| [L23](../src/platform/win/DesktopBgraFrameBuffer.h#L23) | `DesktopBgraFrameBuffer` | 声明 | `DesktopBgraFrameBuffer(std::shared_ptr<std::vector<uint8_t>> pixels, int sourceStride, int sourceWidth, int sourceHeight, int cropX, int cropY, int cropWidth, int cropHeight, int outputWidth, int outputHeight)` | 实现 desktop bgra frame buffer 对应的业务或工具逻辑。 |
| [L34](../src/platform/win/DesktopBgraFrameBuffer.h#L34) | `type` | 声明 | `Type type() const override` | 实现 type 对应的业务或工具逻辑。 |
| [L35](../src/platform/win/DesktopBgraFrameBuffer.h#L35) | `width` | 声明 | `int width() const override` | 实现 width 对应的业务或工具逻辑。 |
| [L36](../src/platform/win/DesktopBgraFrameBuffer.h#L36) | `height` | 声明 | `int height() const override` | 实现 height 对应的业务或工具逻辑。 |
| [L37](../src/platform/win/DesktopBgraFrameBuffer.h#L37) | `ToI420` | 声明 | `webrtc::scoped_refptr<webrtc::I420BufferInterface> ToI420() override` | 实现 to i420 对应的业务或工具逻辑。 |
| [L38](../src/platform/win/DesktopBgraFrameBuffer.h#L38) | `CropAndScale` | 声明 | `webrtc::scoped_refptr<webrtc::VideoFrameBuffer> CropAndScale( int offsetX, int offsetY, int cropWidth, int cropHeight, int scaledWidth, int scaledHeight) override` | 转换或缩放 crop and scale 相关逻辑。 |
| [L45](../src/platform/win/DesktopBgraFrameBuffer.h#L45) | `storage_representation` | 声明 | `std::string storage_representation() const override` | 实现 storage representation 对应的业务或工具逻辑。 |
| [L47](../src/platform/win/DesktopBgraFrameBuffer.h#L47) | `Data` | 声明 | `const uint8_t* Data() const noexcept` | 实现 data 对应的业务或工具逻辑。 |
| [L48](../src/platform/win/DesktopBgraFrameBuffer.h#L48) | `Stride` | 声明 | `int Stride() const noexcept` | 实现 stride 对应的业务或工具逻辑。 |
| [L49](../src/platform/win/DesktopBgraFrameBuffer.h#L49) | `SourceWidth` | 声明 | `int SourceWidth() const noexcept` | 实现 source width 对应的业务或工具逻辑。 |
| [L50](../src/platform/win/DesktopBgraFrameBuffer.h#L50) | `SourceHeight` | 声明 | `int SourceHeight() const noexcept` | 实现 source height 对应的业务或工具逻辑。 |
| [L51](../src/platform/win/DesktopBgraFrameBuffer.h#L51) | `CropX` | 声明 | `int CropX() const noexcept` | 转换或缩放 crop x 相关逻辑。 |
| [L52](../src/platform/win/DesktopBgraFrameBuffer.h#L52) | `CropY` | 声明 | `int CropY() const noexcept` | 转换或缩放 crop y 相关逻辑。 |
| [L53](../src/platform/win/DesktopBgraFrameBuffer.h#L53) | `CropWidth` | 声明 | `int CropWidth() const noexcept` | 转换或缩放 crop width 相关逻辑。 |
| [L54](../src/platform/win/DesktopBgraFrameBuffer.h#L54) | `CropHeight` | 声明 | `int CropHeight() const noexcept` | 转换或缩放 crop height 相关逻辑。 |

## `src/platform/win/DxgiNativeDesktopCapturer.cpp`

[打开源码](../src/platform/win/DxgiNativeDesktopCapturer.cpp) · **文件作用：** 实现 dxgi native desktop capturer 相关函数与文件级辅助逻辑。

### 类型

| 行 | 类型 | 种类 | 作用 |
|---:|---|---|---|
| [L41](../src/platform/win/DxgiNativeDesktopCapturer.cpp#L41) | `DxgiNativeDesktopCapturer::Impl` | class | 定义 DxgiNativeDesktopCapturer::Impl 的 class 类型和相关状态。 |

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L23](../src/platform/win/DxgiNativeDesktopCapturer.cpp#L23) | `HResultText` | 定义 | `std::string HResultText(const char* operation, HRESULT result)` | 实现 h result text 对应的业务或工具逻辑。 |
| [L32](../src/platform/win/DxgiNativeDesktopCapturer.cpp#L32) | `IsPrimaryOutput` | 定义 | `bool IsPrimaryOutput(const DXGI_OUTPUT_DESC& description)` | 判断 is primary output 相关逻辑。 |
| [L43](../src/platform/win/DxgiNativeDesktopCapturer.cpp#L43) | `~Impl` | 定义 | `~Impl()` | 停止相关活动并释放 Impl 实例拥有的资源。 |
| [L48](../src/platform/win/DxgiNativeDesktopCapturer.cpp#L48) | `Reset` | 定义 | `void Reset()` | 重置或移除 reset 相关逻辑。 |
| [L59](../src/platform/win/DxgiNativeDesktopCapturer.cpp#L59) | `Initialize` | 定义 | `bool Initialize(const DisplayDescriptor& target)` | 创建或初始化 initialize 相关逻辑。 |
| [L194](../src/platform/win/DxgiNativeDesktopCapturer.cpp#L194) | `Capture` | 定义 | `Result Capture( std::uint32_t timeoutMs, webrtc::scoped_refptr<D3D11DesktopFrameBuffer>* resultFrame)` | 采集 capture 相关逻辑。 |
| [L272](../src/platform/win/DxgiNativeDesktopCapturer.cpp#L272) | `DxgiNativeDesktopCapturer::DxgiNativeDesktopCapturer` | 定义 | `DxgiNativeDesktopCapturer::DxgiNativeDesktopCapturer() : impl_(std::make_unique<Impl>()) {}` | 构造并初始化 DxgiNativeDesktopCapturer 实例。 |
| [L278](../src/platform/win/DxgiNativeDesktopCapturer.cpp#L278) | `DxgiNativeDesktopCapturer::Initialize` | 定义 | `bool DxgiNativeDesktopCapturer::Initialize( const DisplayDescriptor& target)` | 创建或初始化 initialize 相关逻辑。 |
| [L285](../src/platform/win/DxgiNativeDesktopCapturer.cpp#L285) | `DxgiNativeDesktopCapturer::Capture` | 定义 | `DxgiNativeDesktopCapturer::Capture( std::uint32_t timeoutMs, webrtc::scoped_refptr<D3D11DesktopFrameBuffer>* frame)` | 采集 capture 相关逻辑。 |
| [L292](../src/platform/win/DxgiNativeDesktopCapturer.cpp#L292) | `DxgiNativeDesktopCapturer::LastError` | 定义 | `std::string DxgiNativeDesktopCapturer::LastError() const` | 实现 last error 对应的业务或工具逻辑。 |

## `src/platform/win/DxgiNativeDesktopCapturer.h`

[打开源码](../src/platform/win/DxgiNativeDesktopCapturer.h) · **文件作用：** 声明 dxgi native desktop capturer 相关类型、接口、配置和成员状态。

### 类型

| 行 | 类型 | 种类 | 作用 |
|---:|---|---|---|
| [L15](../src/platform/win/DxgiNativeDesktopCapturer.h#L15) | `D3D11DesktopFrameBuffer` | class | 定义 D3D11DesktopFrameBuffer 的 class 类型和相关状态。 |
| [L21](../src/platform/win/DxgiNativeDesktopCapturer.h#L21) | `DxgiNativeDesktopCapturer` | class | Captures one selected Windows display as an immutable D3D11 BGRA texture. It deliberately performs a GPU CopyResource into an application-owned texture before releasing the Desk... |
| [L23](../src/platform/win/DxgiNativeDesktopCapturer.h#L23) | `Result` | enum class | 定义 Result 的 enum class 类型和相关状态。 |
| [L39](../src/platform/win/DxgiNativeDesktopCapturer.h#L39) | `Impl` | class | 定义 Impl 的 class 类型和相关状态。 |

### 成员与文件级变量

| 行 | 变量 | 声明 | 作用 |
|---:|---|---|---|
| [L15](../src/platform/win/DxgiNativeDesktopCapturer.h#L15) | `D3D11DesktopFrameBuffer` | `class D3D11DesktopFrameBuffer;` | 保存媒体帧、图像或缓冲资源：d3 d11 desktop frame buffer。 |
| [L39](../src/platform/win/DxgiNativeDesktopCapturer.h#L39) | `Impl` | `class Impl;` | 保存 impl 相关配置或运行状态。 |
| [L40](../src/platform/win/DxgiNativeDesktopCapturer.h#L40) | `impl_` | `std::unique_ptr<Impl> impl_;` | 保存 impl 相关配置或运行状态。 |

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L29](../src/platform/win/DxgiNativeDesktopCapturer.h#L29) | `DxgiNativeDesktopCapturer` | 声明 | `DxgiNativeDesktopCapturer()` | 实现 dxgi native desktop capturer 对应的业务或工具逻辑。 |
| [L30](../src/platform/win/DxgiNativeDesktopCapturer.h#L30) | `~DxgiNativeDesktopCapturer` | 声明 | `~DxgiNativeDesktopCapturer()` | 停止相关活动并释放 DxgiNativeDesktopCapturer 实例拥有的资源。 |
| [L32](../src/platform/win/DxgiNativeDesktopCapturer.h#L32) | `Initialize` | 声明 | `bool Initialize(const DisplayDescriptor& target)` | 创建或初始化 initialize 相关逻辑。 |
| [L33](../src/platform/win/DxgiNativeDesktopCapturer.h#L33) | `Capture` | 声明 | `Result Capture( std::uint32_t timeoutMs, webrtc::scoped_refptr<D3D11DesktopFrameBuffer>* frame)` | 采集 capture 相关逻辑。 |
| [L36](../src/platform/win/DxgiNativeDesktopCapturer.h#L36) | `LastError` | 声明 | `std::string LastError() const` | 实现 last error 对应的业务或工具逻辑。 |

## `src/platform/win/FfmpegD3D11H264Decoder.cpp`

[打开源码](../src/platform/win/FfmpegD3D11H264Decoder.cpp) · **文件作用：** 实现 ffmpeg d3 d11 h264 decoder 相关函数与文件级辅助逻辑。

### 类型

| 行 | 类型 | 种类 | 作用 |
|---:|---|---|---|
| [L66](../src/platform/win/FfmpegD3D11H264Decoder.cpp#L66) | `FfmpegD3D11Api` | class | 定义 FfmpegD3D11Api 的 class 类型和相关状态。 |
| [L193](../src/platform/win/FfmpegD3D11H264Decoder.cpp#L193) | `FfmpegD3D11H264Decoder::Impl` | class | 定义 FfmpegD3D11H264Decoder::Impl 的 class 类型和相关状态。 |
| [L362](../src/platform/win/FfmpegD3D11H264Decoder.cpp#L362) | `FrameMetadata` | struct | 定义 FrameMetadata 的 struct 类型和相关状态。 |

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L40](../src/platform/win/FfmpegD3D11H264Decoder.cpp#L40) | `ResolveFunction` | 定义 | `bool ResolveFunction(HMODULE module, const char* name, Function* target)` | 查询并返回 resolve function 相关逻辑。 |
| [L49](../src/platform/win/FfmpegD3D11H264Decoder.cpp#L49) | `ExecutableDirectory` | 定义 | `std::filesystem::path ExecutableDirectory()` | 实现 executable directory 对应的业务或工具逻辑。 |
| [L68](../src/platform/win/FfmpegD3D11H264Decoder.cpp#L68) | `Instance` | 定义 | `static FfmpegD3D11Api& Instance()` | 实现 instance 对应的业务或工具逻辑。 |
| [L78](../src/platform/win/FfmpegD3D11H264Decoder.cpp#L78) | `EnsureLoaded` | 定义 | `bool EnsureLoaded()` | 实现 ensure loaded 对应的业务或工具逻辑。 |
| [L84](../src/platform/win/FfmpegD3D11H264Decoder.cpp#L84) | `Error` | 定义 | `const std::string& Error() const noexcept { return error_; }` | 实现 error 对应的业务或工具逻辑。 |
| [L107](../src/platform/win/FfmpegD3D11H264Decoder.cpp#L107) | `Load` | 定义 | `void Load()` | 读取或恢复 load 相关逻辑。 |
| [L178](../src/platform/win/FfmpegD3D11H264Decoder.cpp#L178) | `FfmpegError` | 定义 | `std::string FfmpegError(FfmpegD3D11Api& api, const char* operation, int result)` | 实现 ffmpeg error 对应的业务或工具逻辑。 |
| [L195](../src/platform/win/FfmpegD3D11H264Decoder.cpp#L195) | `Configure` | 定义 | `bool Configure(const Settings& settings)` | 更新或应用 configure 相关逻辑。 |
| [L256](../src/platform/win/FfmpegD3D11H264Decoder.cpp#L256) | `Decode` | 定义 | `int32_t Decode(const webrtc::EncodedImage& inputImage, int64_t renderTimeMs)` | 解码或解析 decode 相关逻辑。 |
| [L321](../src/platform/win/FfmpegD3D11H264Decoder.cpp#L321) | `RegisterDecodeCompleteCallback` | 定义 | `int32_t RegisterDecodeCompleteCallback( webrtc::DecodedImageCallback* callback)` | 实现 register decode complete callback 对应的业务或工具逻辑。 |
| [L328](../src/platform/win/FfmpegD3D11H264Decoder.cpp#L328) | `Release` | 定义 | `int32_t Release()` | 释放或取消 release 相关逻辑。 |
| [L352](../src/platform/win/FfmpegD3D11H264Decoder.cpp#L352) | `IsConfigured` | 定义 | `bool IsConfigured() const noexcept { return configured_; }` | 判断 is configured 相关逻辑。 |
| [L353](../src/platform/win/FfmpegD3D11H264Decoder.cpp#L353) | `SupportsNativeOutputSamples` | 定义 | `bool SupportsNativeOutputSamples() const noexcept` | 判断 supports native output samples 相关逻辑。 |
| [L357](../src/platform/win/FfmpegD3D11H264Decoder.cpp#L357) | `IsHardwareAccelerated` | 定义 | `bool IsHardwareAccelerated() const noexcept { return configured_; }` | 判断 is hardware accelerated 相关逻辑。 |
| [L358](../src/platform/win/FfmpegD3D11H264Decoder.cpp#L358) | `IsAsynchronous` | 定义 | `bool IsAsynchronous() const noexcept { return false; }` | 判断 is asynchronous 相关逻辑。 |
| [L359](../src/platform/win/FfmpegD3D11H264Decoder.cpp#L359) | `LastError` | 定义 | `const std::string& LastError() const noexcept { return lastError_; }` | 实现 last error 对应的业务或工具逻辑。 |
| [L375](../src/platform/win/FfmpegD3D11H264Decoder.cpp#L375) | `SelectHardwareFormat` | 定义 | `static AVPixelFormat SelectHardwareFormat( AVCodecContext* context, const AVPixelFormat* formats)` | 查询并返回 select hardware format 相关逻辑。 |
| [L392](../src/platform/win/FfmpegD3D11H264Decoder.cpp#L392) | `TakeMetadata` | 定义 | `FrameMetadata TakeMetadata(std::int64_t presentationId)` | 实现 take metadata 对应的业务或工具逻辑。 |
| [L413](../src/platform/win/FfmpegD3D11H264Decoder.cpp#L413) | `DrainOutput` | 定义 | `bool DrainOutput()` | 实现 drain output 对应的业务或工具逻辑。 |
| [L435](../src/platform/win/FfmpegD3D11H264Decoder.cpp#L435) | `DeliverOutputFrame` | 定义 | `bool DeliverOutputFrame()` | 实现 deliver output frame 对应的业务或工具逻辑。 |
| [L531](../src/platform/win/FfmpegD3D11H264Decoder.cpp#L531) | `FfmpegD3D11H264Decoder::FfmpegD3D11H264Decoder` | 定义 | `FfmpegD3D11H264Decoder::FfmpegD3D11H264Decoder() : impl_(std::make_unique<Impl>()) {}` | 构造并初始化 FfmpegD3D11H264Decoder 实例。 |
| [L535](../src/platform/win/FfmpegD3D11H264Decoder.cpp#L535) | `FfmpegD3D11H264Decoder::~FfmpegD3D11H264Decoder` | 定义 | `FfmpegD3D11H264Decoder::~FfmpegD3D11H264Decoder()` | 停止相关活动并释放 FfmpegD3D11H264Decoder 实例拥有的资源。 |
| [L540](../src/platform/win/FfmpegD3D11H264Decoder.cpp#L540) | `FfmpegD3D11H264Decoder::Configure` | 定义 | `bool FfmpegD3D11H264Decoder::Configure(const Settings& settings)` | 更新或应用 configure 相关逻辑。 |
| [L545](../src/platform/win/FfmpegD3D11H264Decoder.cpp#L545) | `FfmpegD3D11H264Decoder::Decode` | 定义 | `int32_t FfmpegD3D11H264Decoder::Decode( const webrtc::EncodedImage& inputImage, int64_t renderTimeMs)` | 解码或解析 decode 相关逻辑。 |
| [L552](../src/platform/win/FfmpegD3D11H264Decoder.cpp#L552) | `FfmpegD3D11H264Decoder::RegisterDecodeCompleteCallback` | 定义 | `int32_t FfmpegD3D11H264Decoder::RegisterDecodeCompleteCallback( webrtc::DecodedImageCallback* callback)` | 实现 register decode complete callback 对应的业务或工具逻辑。 |
| [L558](../src/platform/win/FfmpegD3D11H264Decoder.cpp#L558) | `FfmpegD3D11H264Decoder::Release` | 定义 | `int32_t FfmpegD3D11H264Decoder::Release()` | 释放或取消 release 相关逻辑。 |
| [L564](../src/platform/win/FfmpegD3D11H264Decoder.cpp#L564) | `FfmpegD3D11H264Decoder::GetDecoderInfo` | 定义 | `FfmpegD3D11H264Decoder::GetDecoderInfo() const` | 查询并返回 get decoder info 相关逻辑。 |
| [L572](../src/platform/win/FfmpegD3D11H264Decoder.cpp#L572) | `FfmpegD3D11H264Decoder::ImplementationName` | 定义 | `const char* FfmpegD3D11H264Decoder::ImplementationName() const` | 实现 implementation name 对应的业务或工具逻辑。 |
| [L577](../src/platform/win/FfmpegD3D11H264Decoder.cpp#L577) | `FfmpegD3D11H264Decoder::IsConfigured` | 定义 | `bool FfmpegD3D11H264Decoder::IsConfigured() const noexcept` | 判断 is configured 相关逻辑。 |
| [L582](../src/platform/win/FfmpegD3D11H264Decoder.cpp#L582) | `FfmpegD3D11H264Decoder::SupportsNativeOutputSamples` | 定义 | `bool FfmpegD3D11H264Decoder::SupportsNativeOutputSamples() const noexcept` | 判断 supports native output samples 相关逻辑。 |
| [L587](../src/platform/win/FfmpegD3D11H264Decoder.cpp#L587) | `FfmpegD3D11H264Decoder::IsHardwareAccelerated` | 定义 | `bool FfmpegD3D11H264Decoder::IsHardwareAccelerated() const noexcept` | 判断 is hardware accelerated 相关逻辑。 |
| [L592](../src/platform/win/FfmpegD3D11H264Decoder.cpp#L592) | `FfmpegD3D11H264Decoder::IsAsynchronous` | 定义 | `bool FfmpegD3D11H264Decoder::IsAsynchronous() const noexcept` | 判断 is asynchronous 相关逻辑。 |
| [L597](../src/platform/win/FfmpegD3D11H264Decoder.cpp#L597) | `FfmpegD3D11H264Decoder::LastError` | 定义 | `const std::string& FfmpegD3D11H264Decoder::LastError() const noexcept` | 实现 last error 对应的业务或工具逻辑。 |

## `src/platform/win/FfmpegD3D11H264Decoder.h`

[打开源码](../src/platform/win/FfmpegD3D11H264Decoder.h) · **文件作用：** 声明 ffmpeg d3 d11 h264 decoder 相关类型、接口、配置和成员状态。

### 类型

| 行 | 类型 | 种类 | 作用 |
|---:|---|---|---|
| [L16](../src/platform/win/FfmpegD3D11H264Decoder.h#L16) | `FfmpegD3D11H264Decoder` | class | 定义 FfmpegD3D11H264Decoder 的 class 类型和相关状态。 |
| [L41](../src/platform/win/FfmpegD3D11H264Decoder.h#L41) | `Impl` | class | 定义 Impl 的 class 类型和相关状态。 |

### 成员与文件级变量

| 行 | 变量 | 声明 | 作用 |
|---:|---|---|---|
| [L41](../src/platform/win/FfmpegD3D11H264Decoder.h#L41) | `Impl` | `class Impl;` | 保存 impl 相关配置或运行状态。 |
| [L42](../src/platform/win/FfmpegD3D11H264Decoder.h#L42) | `impl_` | `std::unique_ptr<Impl> impl_;` | 保存 impl 相关配置或运行状态。 |

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L18](../src/platform/win/FfmpegD3D11H264Decoder.h#L18) | `FfmpegD3D11H264Decoder` | 声明 | `FfmpegD3D11H264Decoder()` | 实现 ffmpeg d3 d11 h264 decoder 对应的业务或工具逻辑。 |
| [L19](../src/platform/win/FfmpegD3D11H264Decoder.h#L19) | `~FfmpegD3D11H264Decoder` | 声明 | `~FfmpegD3D11H264Decoder() override` | 停止相关活动并释放 FfmpegD3D11H264Decoder 实例拥有的资源。 |
| [L21](../src/platform/win/FfmpegD3D11H264Decoder.h#L21) | `FfmpegD3D11H264Decoder` | 声明 | `FfmpegD3D11H264Decoder(const FfmpegD3D11H264Decoder&) = delete` | 实现 ffmpeg d3 d11 h264 decoder 对应的业务或工具逻辑。 |
| [L25](../src/platform/win/FfmpegD3D11H264Decoder.h#L25) | `Configure` | 声明 | `bool Configure(const Settings& settings) override` | 更新或应用 configure 相关逻辑。 |
| [L26](../src/platform/win/FfmpegD3D11H264Decoder.h#L26) | `Decode` | 声明 | `int32_t Decode(const webrtc::EncodedImage& inputImage, int64_t renderTimeMs) override` | 解码或解析 decode 相关逻辑。 |
| [L28](../src/platform/win/FfmpegD3D11H264Decoder.h#L28) | `RegisterDecodeCompleteCallback` | 声明 | `int32_t RegisterDecodeCompleteCallback( webrtc::DecodedImageCallback* callback) override` | 实现 register decode complete callback 对应的业务或工具逻辑。 |
| [L30](../src/platform/win/FfmpegD3D11H264Decoder.h#L30) | `Release` | 声明 | `int32_t Release() override` | 释放或取消 release 相关逻辑。 |
| [L31](../src/platform/win/FfmpegD3D11H264Decoder.h#L31) | `GetDecoderInfo` | 声明 | `DecoderInfo GetDecoderInfo() const override` | 查询并返回 get decoder info 相关逻辑。 |
| [L32](../src/platform/win/FfmpegD3D11H264Decoder.h#L32) | `ImplementationName` | 声明 | `const char* ImplementationName() const override` | 实现 implementation name 对应的业务或工具逻辑。 |
| [L34](../src/platform/win/FfmpegD3D11H264Decoder.h#L34) | `IsConfigured` | 声明 | `bool IsConfigured() const noexcept` | 判断 is configured 相关逻辑。 |
| [L35](../src/platform/win/FfmpegD3D11H264Decoder.h#L35) | `SupportsNativeOutputSamples` | 声明 | `bool SupportsNativeOutputSamples() const noexcept` | 判断 supports native output samples 相关逻辑。 |
| [L36](../src/platform/win/FfmpegD3D11H264Decoder.h#L36) | `IsHardwareAccelerated` | 声明 | `bool IsHardwareAccelerated() const noexcept` | 判断 is hardware accelerated 相关逻辑。 |
| [L37](../src/platform/win/FfmpegD3D11H264Decoder.h#L37) | `IsAsynchronous` | 声明 | `bool IsAsynchronous() const noexcept` | 判断 is asynchronous 相关逻辑。 |
| [L38](../src/platform/win/FfmpegD3D11H264Decoder.h#L38) | `LastError` | 声明 | `const std::string& LastError() const noexcept` | 实现 last error 对应的业务或工具逻辑。 |

## `src/platform/win/FfmpegHardwareH264Encoder.cpp`

[打开源码](../src/platform/win/FfmpegHardwareH264Encoder.cpp) · **文件作用：** 实现 ffmpeg hardware h264 encoder 相关函数与文件级辅助逻辑。

### 类型

| 行 | 类型 | 种类 | 作用 |
|---:|---|---|---|
| [L51](../src/platform/win/FfmpegHardwareH264Encoder.cpp#L51) | `BackendDescriptor` | struct | 定义 BackendDescriptor 的 struct 类型和相关状态。 |
| [L119](../src/platform/win/FfmpegHardwareH264Encoder.cpp#L119) | `HardwareAdapterDescriptor` | struct | 定义 HardwareAdapterDescriptor 的 struct 类型和相关状态。 |
| [L176](../src/platform/win/FfmpegHardwareH264Encoder.cpp#L176) | `FfmpegHardwareApi` | class | 定义 FfmpegHardwareApi 的 class 类型和相关状态。 |
| [L412](../src/platform/win/FfmpegHardwareH264Encoder.cpp#L412) | `FfmpegHardwareH264Encoder::Impl` | class | 定义 FfmpegHardwareH264Encoder::Impl 的 class 类型和相关状态。 |
| [L741](../src/platform/win/FfmpegHardwareH264Encoder.cpp#L741) | `FrameMetadata` | struct | 定义 FrameMetadata 的 struct 类型和相关状态。 |
| [L1516](../src/platform/win/FfmpegHardwareH264Encoder.cpp#L1516) | `Callback` | class | 定义 Callback 的 class 类型和相关状态。 |

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L68](../src/platform/win/FfmpegHardwareH264Encoder.cpp#L68) | `DescriptorFor` | 定义 | `const BackendDescriptor* DescriptorFor(FfmpegHardwareBackend backend)` | 实现 descriptor for 对应的业务或工具逻辑。 |
| [L79](../src/platform/win/FfmpegHardwareH264Encoder.cpp#L79) | `ResolveFunction` | 定义 | `bool ResolveFunction(HMODULE module, const char* name, Function* target)` | 查询并返回 resolve function 相关逻辑。 |
| [L88](../src/platform/win/FfmpegHardwareH264Encoder.cpp#L88) | `ExecutableDirectory` | 定义 | `std::filesystem::path ExecutableDirectory()` | 实现 executable directory 对应的业务或工具逻辑。 |
| [L105](../src/platform/win/FfmpegHardwareH264Encoder.cpp#L105) | `SystemModuleAvailable` | 定义 | `bool SystemModuleAvailable(const wchar_t* moduleName)` | 实现 system module available 对应的业务或工具逻辑。 |
| [L125](../src/platform/win/FfmpegHardwareH264Encoder.cpp#L125) | `PackAdapterLuid` | 定义 | `std::uint64_t PackAdapterLuid(const LUID& luid)` | 编码 pack adapter luid 相关逻辑。 |
| [L133](../src/platform/win/FfmpegHardwareH264Encoder.cpp#L133) | `HardwareAdapters` | 定义 | `std::vector<HardwareAdapterDescriptor> HardwareAdapters()` | 实现 hardware adapters 对应的业务或工具逻辑。 |
| [L166](../src/platform/win/FfmpegHardwareH264Encoder.cpp#L166) | `HasVendor` | 定义 | `bool HasVendor(UINT vendorId)` | 判断 has vendor 相关逻辑。 |
| [L178](../src/platform/win/FfmpegHardwareH264Encoder.cpp#L178) | `Instance` | 定义 | `static FfmpegHardwareApi& Instance()` | 实现 instance 对应的业务或工具逻辑。 |
| [L184](../src/platform/win/FfmpegHardwareH264Encoder.cpp#L184) | `EnsureLoaded` | 定义 | `bool EnsureLoaded()` | 实现 ensure loaded 对应的业务或工具逻辑。 |
| [L190](../src/platform/win/FfmpegHardwareH264Encoder.cpp#L190) | `Error` | 定义 | `const std::string& Error() const noexcept { return error_; }` | 实现 error 对应的业务或工具逻辑。 |
| [L220](../src/platform/win/FfmpegHardwareH264Encoder.cpp#L220) | `Load` | 定义 | `void Load()` | 读取或恢复 load 相关逻辑。 |
| [L313](../src/platform/win/FfmpegHardwareH264Encoder.cpp#L313) | `FfmpegError` | 定义 | `std::string FfmpegError(FfmpegHardwareApi& api, const char* operation, int result)` | 实现 ffmpeg error 对应的业务或工具逻辑。 |
| [L327](../src/platform/win/FfmpegHardwareH264Encoder.cpp#L327) | `CandidateBackends` | 定义 | `std::vector<const BackendDescriptor*> CandidateBackends( FfmpegHardwareBackend requested, std::uint64_t preferredAdapterLuid)` | 判断 candidate backends 相关逻辑。 |
| [L364](../src/platform/win/FfmpegHardwareH264Encoder.cpp#L364) | `AnnexBStartCode` | 定义 | `bool AnnexBStartCode(const uint8_t* data, std::size_t size)` | 实现 annex b start code 对应的业务或工具逻辑。 |
| [L379](../src/platform/win/FfmpegHardwareH264Encoder.cpp#L379) | `NvencPresetForQuality` | 定义 | `const char* NvencPresetForQuality(FfmpegX264Preset quality)` | 实现 nvenc preset for quality 对应的业务或工具逻辑。 |
| [L384](../src/platform/win/FfmpegHardwareH264Encoder.cpp#L384) | `QsvPresetForQuality` | 定义 | `const char* QsvPresetForQuality(FfmpegX264Preset quality)` | 实现 qsv preset for quality 对应的业务或工具逻辑。 |
| [L389](../src/platform/win/FfmpegHardwareH264Encoder.cpp#L389) | `AmfQualityForPreset` | 定义 | `const char* AmfQualityForPreset(FfmpegX264Preset quality)` | 实现 amf quality for preset 对应的业务或工具逻辑。 |
| [L394](../src/platform/win/FfmpegHardwareH264Encoder.cpp#L394) | `HardwareQualityOption` | 定义 | `const char* HardwareQualityOption( FfmpegHardwareBackend backend, FfmpegX264Preset quality)` | 实现 hardware quality option 对应的业务或工具逻辑。 |
| [L414](../src/platform/win/FfmpegHardwareH264Encoder.cpp#L414) | `Impl` | 定义 | `Impl(FfmpegHardwareBackend requestedBackend, std::shared_ptr<VideoEncoderRuntimeState> runtimeState, FfmpegX264Preset quality) : requestedBackend_(requestedBackend), runtimeState_(std::move(runtimeState)), quality_(qu...` | 实现 impl 对应的业务或工具逻辑。 |
| [L422](../src/platform/win/FfmpegHardwareH264Encoder.cpp#L422) | `~Impl` | 定义 | `~Impl() { Release(); }` | 停止相关活动并释放 Impl 实例拥有的资源。 |
| [L424](../src/platform/win/FfmpegHardwareH264Encoder.cpp#L424) | `ImplementationName` | 定义 | `const std::string& ImplementationName() const noexcept` | 实现 implementation name 对应的业务或工具逻辑。 |
| [L429](../src/platform/win/FfmpegHardwareH264Encoder.cpp#L429) | `LastError` | 定义 | `std::string LastError() const` | 实现 last error 对应的业务或工具逻辑。 |
| [L435](../src/platform/win/FfmpegHardwareH264Encoder.cpp#L435) | `Init` | 定义 | `int Init(const webrtc::VideoCodec* codecSettings)` | 创建或初始化 init 相关逻辑。 |
| [L602](../src/platform/win/FfmpegHardwareH264Encoder.cpp#L602) | `RegisterCallback` | 定义 | `int RegisterCallback(webrtc::EncodedImageCallback* callback)` | 实现 register callback 对应的业务或工具逻辑。 |
| [L609](../src/platform/win/FfmpegHardwareH264Encoder.cpp#L609) | `Release` | 定义 | `int Release()` | 释放或取消 release 相关逻辑。 |
| [L616](../src/platform/win/FfmpegHardwareH264Encoder.cpp#L616) | `Encode` | 定义 | `int Encode(const webrtc::VideoFrame& frame, const std::vector<webrtc::VideoFrameType>* frameTypes)` | 编码 encode 相关逻辑。 |
| [L721](../src/platform/win/FfmpegHardwareH264Encoder.cpp#L721) | `SetRates` | 定义 | `void SetRates(const webrtc::VideoEncoder::RateControlParameters& parameters)` | 更新或应用 set rates 相关逻辑。 |
| [L746](../src/platform/win/FfmpegHardwareH264Encoder.cpp#L746) | `EncoderSupportsD3D11` | 定义 | `bool EncoderSupportsD3D11(FfmpegHardwareApi& api, const AVCodec* encoder) const` | 编码 encoder supports d3 d11 相关逻辑。 |
| [L766](../src/platform/win/FfmpegHardwareH264Encoder.cpp#L766) | `InitializeD3D11Input` | 定义 | `bool InitializeD3D11Input(FfmpegHardwareApi& api, const AVCodec* encoder, UINT selectedVendorId)` | 创建或初始化 initialize d3 d11 input 相关逻辑。 |
| [L846](../src/platform/win/FfmpegHardwareH264Encoder.cpp#L846) | `EnsureVideoProcessor` | 定义 | `bool EnsureVideoProcessor(UINT sourceWidth, UINT sourceHeight)` | 实现 ensure video processor 对应的业务或工具逻辑。 |
| [L910](../src/platform/win/FfmpegHardwareH264Encoder.cpp#L910) | `PrepareD3D11Frame` | 定义 | `bool PrepareD3D11Frame( FfmpegHardwareApi& api, const webrtc::scoped_refptr<webrtc::VideoFrameBuffer>& source)` | 实现 prepare d3 d11 frame 对应的业务或工具逻辑。 |
| [L1013](../src/platform/win/FfmpegHardwareH264Encoder.cpp#L1013) | `ConfigureBackendOptions` | 定义 | `bool ConfigureBackendOptions(FfmpegHardwareApi& api, FfmpegHardwareBackend backend)` | 更新或应用 configure backend options 相关逻辑。 |
| [L1077](../src/platform/win/FfmpegHardwareH264Encoder.cpp#L1077) | `ApplyBitrate` | 定义 | `void ApplyBitrate(std::uint32_t bitrate)` | 更新或应用 apply bitrate 相关逻辑。 |
| [L1104](../src/platform/win/FfmpegHardwareH264Encoder.cpp#L1104) | `ScheduleBitrate` | 定义 | `void ScheduleBitrate(std::uint32_t bitrate)` | 执行后台循环或调度 schedule bitrate 相关逻辑。 |
| [L1114](../src/platform/win/FfmpegHardwareH264Encoder.cpp#L1114) | `MaybeApplyPendingBitrate` | 定义 | `void MaybeApplyPendingBitrate()` | 实现 maybe apply pending bitrate 对应的业务或工具逻辑。 |
| [L1168](../src/platform/win/FfmpegHardwareH264Encoder.cpp#L1168) | `ReportInputFormat` | 定义 | `void ReportInputFormat( const webrtc::scoped_refptr<webrtc::VideoFrameBuffer>& buffer)` | 通知或报告 report input format 相关逻辑。 |
| [L1200](../src/platform/win/FfmpegHardwareH264Encoder.cpp#L1200) | `RequestSoftwareFallback` | 定义 | `int RequestSoftwareFallback(std::string reason)` | 发起请求或查询 request software fallback 相关逻辑。 |
| [L1221](../src/platform/win/FfmpegHardwareH264Encoder.cpp#L1221) | `DrainOutput` | 定义 | `bool DrainOutput()` | 实现 drain output 对应的业务或工具逻辑。 |
| [L1286](../src/platform/win/FfmpegHardwareH264Encoder.cpp#L1286) | `ReleaseUnlocked` | 定义 | `void ReleaseUnlocked()` | 释放或取消 release unlocked 相关逻辑。 |
| [L1383](../src/platform/win/FfmpegHardwareH264Encoder.cpp#L1383) | `FfmpegHardwareH264Encoder::FfmpegHardwareH264Encoder` | 定义 | `FfmpegHardwareH264Encoder::FfmpegHardwareH264Encoder( FfmpegHardwareBackend backend, std::shared_ptr<VideoEncoderRuntimeState> runtimeState, FfmpegX264Preset quality) : impl_(std::make_unique<Impl>( backend, std::move...` | 构造并初始化 FfmpegHardwareH264Encoder 实例。 |
| [L1394](../src/platform/win/FfmpegHardwareH264Encoder.cpp#L1394) | `FfmpegHardwareH264Encoder::EnumerateAvailability` | 定义 | `FfmpegHardwareH264Encoder::EnumerateAvailability()` | 实现 enumerate availability 对应的业务或工具逻辑。 |
| [L1425](../src/platform/win/FfmpegHardwareH264Encoder.cpp#L1425) | `FfmpegHardwareH264Encoder::RuntimeAvailable` | 定义 | `bool FfmpegHardwareH264Encoder::RuntimeAvailable( FfmpegHardwareBackend backend, std::string* error)` | 执行后台循环或调度 runtime available 相关逻辑。 |
| [L1457](../src/platform/win/FfmpegHardwareH264Encoder.cpp#L1457) | `FfmpegHardwareH264Encoder::InitEncode` | 定义 | `int FfmpegHardwareH264Encoder::InitEncode( const webrtc::VideoCodec* codecSettings, const Settings& /* settings */)` | 创建或初始化 init encode 相关逻辑。 |
| [L1464](../src/platform/win/FfmpegHardwareH264Encoder.cpp#L1464) | `FfmpegHardwareH264Encoder::RegisterEncodeCompleteCallback` | 定义 | `int32_t FfmpegHardwareH264Encoder::RegisterEncodeCompleteCallback( webrtc::EncodedImageCallback* callback)` | 实现 register encode complete callback 对应的业务或工具逻辑。 |
| [L1470](../src/platform/win/FfmpegHardwareH264Encoder.cpp#L1470) | `FfmpegHardwareH264Encoder::Release` | 定义 | `int32_t FfmpegHardwareH264Encoder::Release()` | 释放或取消 release 相关逻辑。 |
| [L1475](../src/platform/win/FfmpegHardwareH264Encoder.cpp#L1475) | `FfmpegHardwareH264Encoder::Encode` | 定义 | `int32_t FfmpegHardwareH264Encoder::Encode( const webrtc::VideoFrame& frame, const std::vector<webrtc::VideoFrameType>* frameTypes)` | 编码 encode 相关逻辑。 |
| [L1482](../src/platform/win/FfmpegHardwareH264Encoder.cpp#L1482) | `FfmpegHardwareH264Encoder::SetRates` | 定义 | `void FfmpegHardwareH264Encoder::SetRates( const RateControlParameters& parameters)` | 更新或应用 set rates 相关逻辑。 |
| [L1489](../src/platform/win/FfmpegHardwareH264Encoder.cpp#L1489) | `FfmpegHardwareH264Encoder::GetEncoderInfo` | 定义 | `FfmpegHardwareH264Encoder::GetEncoderInfo() const` | 查询并返回 get encoder info 相关逻辑。 |
| [L1508](../src/platform/win/FfmpegHardwareH264Encoder.cpp#L1508) | `FfmpegHardwareH264Encoder::LastError` | 定义 | `std::string FfmpegHardwareH264Encoder::LastError() const` | 实现 last error 对应的业务或工具逻辑。 |
| [L1513](../src/platform/win/FfmpegHardwareH264Encoder.cpp#L1513) | `RunFfmpegHardwareEncoderSelfTest` | 定义 | `FfmpegHardwareEncoderSelfTestResult RunFfmpegHardwareEncoderSelfTest( FfmpegHardwareBackend backend)` | 执行后台循环或调度 run ffmpeg hardware encoder self test 相关逻辑。 |

## `src/platform/win/FfmpegHardwareH264Encoder.h`

[打开源码](../src/platform/win/FfmpegHardwareH264Encoder.h) · **文件作用：** 声明 ffmpeg hardware h264 encoder 相关类型、接口、配置和成员状态。

### 类型

| 行 | 类型 | 种类 | 作用 |
|---:|---|---|---|
| [L15](../src/platform/win/FfmpegHardwareH264Encoder.h#L15) | `FfmpegHardwareEncoderAvailability` | struct | 定义 FfmpegHardwareEncoderAvailability 的 struct 类型和相关状态。 |
| [L23](../src/platform/win/FfmpegHardwareH264Encoder.h#L23) | `FfmpegHardwareEncoderSelfTestResult` | struct | 定义 FfmpegHardwareEncoderSelfTestResult 的 struct 类型和相关状态。 |
| [L39](../src/platform/win/FfmpegHardwareH264Encoder.h#L39) | `FfmpegHardwareH264Encoder` | class | D3D11 NV12 hardware frames when the selected FFmpeg backend exposes the D3D11 hw-frames interface. CPU/libwebrtc inputs and unsupported backends keep the deterministic CPU NV12 ... |
| [L66](../src/platform/win/FfmpegHardwareH264Encoder.h#L66) | `Impl` | class | 定义 Impl 的 class 类型和相关状态。 |

### 成员与文件级变量

| 行 | 变量 | 声明 | 作用 |
|---:|---|---|---|
| [L16](../src/platform/win/FfmpegHardwareH264Encoder.h#L16) | `backend` | `FfmpegHardwareBackend backend = FfmpegHardwareBackend::kAutomatic;` | 保存 backend 相关配置或运行状态。 |
| [L17](../src/platform/win/FfmpegHardwareH264Encoder.h#L17) | `compiled` | `bool compiled = false;` | 保存 compiled 相关配置或运行状态。 |
| [L18](../src/platform/win/FfmpegHardwareH264Encoder.h#L18) | `driverRuntimePresent` | `bool driverRuntimePresent = false;` | 保存 driver runtime present 相关配置或运行状态。 |
| [L19](../src/platform/win/FfmpegHardwareH264Encoder.h#L19) | `implementation` | `std::string implementation;` | 保存 implementation 相关配置或运行状态。 |
| [L20](../src/platform/win/FfmpegHardwareH264Encoder.h#L20) | `detail` | `std::string detail;` | 保存 detail 相关配置或运行状态。 |
| [L24](../src/platform/win/FfmpegHardwareH264Encoder.h#L24) | `succeeded` | `bool succeeded = false;` | 保存 succeeded 相关配置或运行状态。 |
| [L25](../src/platform/win/FfmpegHardwareH264Encoder.h#L25) | `backend` | `FfmpegHardwareBackend backend = FfmpegHardwareBackend::kAutomatic;` | 保存 backend 相关配置或运行状态。 |
| [L26](../src/platform/win/FfmpegHardwareH264Encoder.h#L26) | `implementation` | `std::string implementation;` | 保存 implementation 相关配置或运行状态。 |
| [L27](../src/platform/win/FfmpegHardwareH264Encoder.h#L27) | `error` | `std::string error;` | 保存最近错误或失败原因：error。 |
| [L28](../src/platform/win/FfmpegHardwareH264Encoder.h#L28) | `frames` | `std::uint32_t frames = 0;` | 保存 frames 相关配置或运行状态。 |
| [L29](../src/platform/win/FfmpegHardwareH264Encoder.h#L29) | `keyFrames` | `std::uint32_t keyFrames = 0;` | 保存 key frames 相关配置或运行状态。 |
| [L30](../src/platform/win/FfmpegHardwareH264Encoder.h#L30) | `encodedBytes` | `std::size_t encodedBytes = 0;` | 保存计数、尺寸或速率指标：encoded bytes。 |
| [L66](../src/platform/win/FfmpegHardwareH264Encoder.h#L66) | `Impl` | `class Impl;` | 保存 impl 相关配置或运行状态。 |
| [L67](../src/platform/win/FfmpegHardwareH264Encoder.h#L67) | `impl_` | `std::unique_ptr<Impl> impl_;` | 保存 impl 相关配置或运行状态。 |

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L45](../src/platform/win/FfmpegHardwareH264Encoder.h#L45) | `~FfmpegHardwareH264Encoder` | 声明 | `~FfmpegHardwareH264Encoder() override` | 停止相关活动并释放 FfmpegHardwareH264Encoder 实例拥有的资源。 |
| [L48](../src/platform/win/FfmpegHardwareH264Encoder.h#L48) | `EnumerateAvailability` | 声明 | `EnumerateAvailability()` | 实现 enumerate availability 对应的业务或工具逻辑。 |
| [L49](../src/platform/win/FfmpegHardwareH264Encoder.h#L49) | `RuntimeAvailable` | 声明 | `static bool RuntimeAvailable( FfmpegHardwareBackend backend = FfmpegHardwareBackend::kAutomatic, std::string* error = nullptr)` | 执行后台循环或调度 runtime available 相关逻辑。 |
| [L53](../src/platform/win/FfmpegHardwareH264Encoder.h#L53) | `InitEncode` | 声明 | `int InitEncode(const webrtc::VideoCodec* codecSettings, const Settings& settings) override` | 创建或初始化 init encode 相关逻辑。 |
| [L55](../src/platform/win/FfmpegHardwareH264Encoder.h#L55) | `RegisterEncodeCompleteCallback` | 声明 | `int32_t RegisterEncodeCompleteCallback( webrtc::EncodedImageCallback* callback) override` | 实现 register encode complete callback 对应的业务或工具逻辑。 |
| [L57](../src/platform/win/FfmpegHardwareH264Encoder.h#L57) | `Release` | 声明 | `int32_t Release() override` | 释放或取消 release 相关逻辑。 |
| [L58](../src/platform/win/FfmpegHardwareH264Encoder.h#L58) | `Encode` | 声明 | `int32_t Encode( const webrtc::VideoFrame& frame, const std::vector<webrtc::VideoFrameType>* frameTypes) override` | 编码 encode 相关逻辑。 |
| [L61](../src/platform/win/FfmpegHardwareH264Encoder.h#L61) | `SetRates` | 声明 | `void SetRates(const RateControlParameters& parameters) override` | 更新或应用 set rates 相关逻辑。 |
| [L62](../src/platform/win/FfmpegHardwareH264Encoder.h#L62) | `GetEncoderInfo` | 声明 | `EncoderInfo GetEncoderInfo() const override` | 查询并返回 get encoder info 相关逻辑。 |
| [L63](../src/platform/win/FfmpegHardwareH264Encoder.h#L63) | `LastError` | 声明 | `std::string LastError() const` | 实现 last error 对应的业务或工具逻辑。 |
| [L70](../src/platform/win/FfmpegHardwareH264Encoder.h#L70) | `RunFfmpegHardwareEncoderSelfTest` | 声明 | `FfmpegHardwareEncoderSelfTestResult RunFfmpegHardwareEncoderSelfTest( FfmpegHardwareBackend backend = FfmpegHardwareBackend::kAutomatic)` | 执行后台循环或调度 run ffmpeg hardware encoder self test 相关逻辑。 |

## `src/platform/win/FfmpegHardwareH264EncoderFactory.cpp`

[打开源码](../src/platform/win/FfmpegHardwareH264EncoderFactory.cpp) · **文件作用：** 实现 ffmpeg hardware h264 encoder factory 相关函数与文件级辅助逻辑。

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L11](../src/platform/win/FfmpegHardwareH264EncoderFactory.cpp#L11) | `FfmpegHardwareH264EncoderFactory::FfmpegHardwareH264EncoderFactory` | 定义 | `FfmpegHardwareH264EncoderFactory::FfmpegHardwareH264EncoderFactory( FfmpegHardwareBackend backend, std::shared_ptr<VideoEncoderRuntimeState> runtimeState, FfmpegX264Preset quality) : backend_(backend), runtimeState_(s...` | 构造并初始化 FfmpegHardwareH264EncoderFactory 实例。 |
| [L20](../src/platform/win/FfmpegHardwareH264EncoderFactory.cpp#L20) | `FfmpegHardwareH264EncoderFactory::GetSupportedFormats` | 定义 | `FfmpegHardwareH264EncoderFactory::GetSupportedFormats() const` | 查询并返回 get supported formats 相关逻辑。 |
| [L29](../src/platform/win/FfmpegHardwareH264EncoderFactory.cpp#L29) | `FfmpegHardwareH264EncoderFactory::QueryCodecSupport` | 定义 | `FfmpegHardwareH264EncoderFactory::QueryCodecSupport( const webrtc::SdpVideoFormat& format, std::optional<std::string> scalabilityMode, std::optional<webrtc::Resolution> /* resolution */) const` | 发起请求或查询 query codec support 相关逻辑。 |
| [L45](../src/platform/win/FfmpegHardwareH264EncoderFactory.cpp#L45) | `FfmpegHardwareH264EncoderFactory::Create` | 定义 | `FfmpegHardwareH264EncoderFactory::Create( const webrtc::Environment& /* environment */, const webrtc::SdpVideoFormat& format)` | 创建或初始化 create 相关逻辑。 |

## `src/platform/win/FfmpegHardwareH264EncoderFactory.h`

[打开源码](../src/platform/win/FfmpegHardwareH264EncoderFactory.h) · **文件作用：** 声明 ffmpeg hardware h264 encoder factory 相关类型、接口、配置和成员状态。

### 类型

| 行 | 类型 | 种类 | 作用 |
|---:|---|---|---|
| [L13](../src/platform/win/FfmpegHardwareH264EncoderFactory.h#L13) | `FfmpegHardwareH264EncoderFactory` | class | 定义 FfmpegHardwareH264EncoderFactory 的 class 类型和相关状态。 |

### 成员与文件级变量

| 行 | 变量 | 声明 | 作用 |
|---:|---|---|---|
| [L31](../src/platform/win/FfmpegHardwareH264EncoderFactory.h#L31) | `backend_` | `FfmpegHardwareBackend backend_ = FfmpegHardwareBackend::kAutomatic;` | 保存 backend 相关配置或运行状态。 |
| [L32](../src/platform/win/FfmpegHardwareH264EncoderFactory.h#L32) | `runtimeState_` | `std::shared_ptr<VideoEncoderRuntimeState> runtimeState_;` | 保存状态机当前状态：runtime state。 |
| [L33](../src/platform/win/FfmpegHardwareH264EncoderFactory.h#L33) | `quality_` | `FfmpegX264Preset quality_ = FfmpegX264Preset::kMedium;` | 保存 quality 相关配置或运行状态。 |

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L21](../src/platform/win/FfmpegHardwareH264EncoderFactory.h#L21) | `GetSupportedFormats` | 声明 | `std::vector<webrtc::SdpVideoFormat> GetSupportedFormats() const override` | 查询并返回 get supported formats 相关逻辑。 |
| [L22](../src/platform/win/FfmpegHardwareH264EncoderFactory.h#L22) | `QueryCodecSupport` | 声明 | `CodecSupport QueryCodecSupport( const webrtc::SdpVideoFormat& format, std::optional<std::string> scalabilityMode, std::optional<webrtc::Resolution> resolution) const override` | 发起请求或查询 query codec support 相关逻辑。 |
| [L26](../src/platform/win/FfmpegHardwareH264EncoderFactory.h#L26) | `Create` | 声明 | `std::unique_ptr<webrtc::VideoEncoder> Create( const webrtc::Environment& environment, const webrtc::SdpVideoFormat& format) override` | 创建或初始化 create 相关逻辑。 |

## `src/platform/win/FfmpegX264H264Encoder.cpp`

[打开源码](../src/platform/win/FfmpegX264H264Encoder.cpp) · **文件作用：** 实现 ffmpeg x264 h264 encoder 相关函数与文件级辅助逻辑。

### 类型

| 行 | 类型 | 种类 | 作用 |
|---:|---|---|---|
| [L64](../src/platform/win/FfmpegX264H264Encoder.cpp#L64) | `FfmpegX264Api` | class | 定义 FfmpegX264Api 的 class 类型和相关状态。 |
| [L207](../src/platform/win/FfmpegX264H264Encoder.cpp#L207) | `FfmpegX264H264Encoder::Impl` | class | 定义 FfmpegX264H264Encoder::Impl 的 class 类型和相关状态。 |
| [L467](../src/platform/win/FfmpegX264H264Encoder.cpp#L467) | `FrameMetadata` | struct | 定义 FrameMetadata 的 struct 类型和相关状态。 |
| [L645](../src/platform/win/FfmpegX264H264Encoder.cpp#L645) | `Callback` | class | 定义 Callback 的 class 类型和相关状态。 |

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L38](../src/platform/win/FfmpegX264H264Encoder.cpp#L38) | `ResolveFunction` | 定义 | `bool ResolveFunction(HMODULE module, const char* name, Function* target)` | 查询并返回 resolve function 相关逻辑。 |
| [L47](../src/platform/win/FfmpegX264H264Encoder.cpp#L47) | `ExecutableDirectory` | 定义 | `std::filesystem::path ExecutableDirectory()` | 实现 executable directory 对应的业务或工具逻辑。 |
| [L66](../src/platform/win/FfmpegX264H264Encoder.cpp#L66) | `Instance` | 定义 | `static FfmpegX264Api& Instance()` | 实现 instance 对应的业务或工具逻辑。 |
| [L73](../src/platform/win/FfmpegX264H264Encoder.cpp#L73) | `EnsureLoaded` | 定义 | `bool EnsureLoaded()` | 实现 ensure loaded 对应的业务或工具逻辑。 |
| [L79](../src/platform/win/FfmpegX264H264Encoder.cpp#L79) | `Error` | 定义 | `const std::string& Error() const noexcept { return error_; }` | 实现 error 对应的业务或工具逻辑。 |
| [L99](../src/platform/win/FfmpegX264H264Encoder.cpp#L99) | `Load` | 定义 | `void Load()` | 读取或恢复 load 相关逻辑。 |
| [L178](../src/platform/win/FfmpegX264H264Encoder.cpp#L178) | `FfmpegError` | 定义 | `std::string FfmpegError(FfmpegX264Api& api, const char* operation, int result)` | 实现 ffmpeg error 对应的业务或工具逻辑。 |
| [L191](../src/platform/win/FfmpegX264H264Encoder.cpp#L191) | `CopyPlane` | 定义 | `void CopyPlane(const uint8_t* source, int sourceStride, uint8_t* destination, int destinationStride, int rowBytes, int rows)` | 实现 copy plane 对应的业务或工具逻辑。 |
| [L209](../src/platform/win/FfmpegX264H264Encoder.cpp#L209) | `Impl` | 定义 | `explicit Impl(FfmpegX264Preset preset) : preset_(preset) {}` | 实现 impl 对应的业务或工具逻辑。 |
| [L211](../src/platform/win/FfmpegX264H264Encoder.cpp#L211) | `~Impl` | 定义 | `~Impl() { Release(); }` | 停止相关活动并释放 Impl 实例拥有的资源。 |
| [L213](../src/platform/win/FfmpegX264H264Encoder.cpp#L213) | `PresetName` | 定义 | `const char* PresetName() const` | 实现 preset name 对应的业务或工具逻辑。 |
| [L218](../src/platform/win/FfmpegX264H264Encoder.cpp#L218) | `Init` | 定义 | `int Init(const webrtc::VideoCodec* codecSettings)` | 创建或初始化 init 相关逻辑。 |
| [L321](../src/platform/win/FfmpegX264H264Encoder.cpp#L321) | `RegisterCallback` | 定义 | `int RegisterCallback(webrtc::EncodedImageCallback* callback)` | 实现 register callback 对应的业务或工具逻辑。 |
| [L328](../src/platform/win/FfmpegX264H264Encoder.cpp#L328) | `Release` | 定义 | `int Release()` | 释放或取消 release 相关逻辑。 |
| [L335](../src/platform/win/FfmpegX264H264Encoder.cpp#L335) | `Encode` | 定义 | `int Encode(const webrtc::VideoFrame& frame, const std::vector<webrtc::VideoFrameType>* frameTypes)` | 编码 encode 相关逻辑。 |
| [L423](../src/platform/win/FfmpegX264H264Encoder.cpp#L423) | `SetRates` | 定义 | `void SetRates(const webrtc::VideoEncoder::RateControlParameters& parameters)` | 更新或应用 set rates 相关逻辑。 |
| [L472](../src/platform/win/FfmpegX264H264Encoder.cpp#L472) | `DrainOutput` | 定义 | `bool DrainOutput()` | 实现 drain output 对应的业务或工具逻辑。 |
| [L531](../src/platform/win/FfmpegX264H264Encoder.cpp#L531) | `ReleaseUnlocked` | 定义 | `void ReleaseUnlocked()` | 释放或取消 release unlocked 相关逻辑。 |
| [L577](../src/platform/win/FfmpegX264H264Encoder.cpp#L577) | `FfmpegX264H264Encoder::FfmpegX264H264Encoder` | 定义 | `FfmpegX264H264Encoder::FfmpegX264H264Encoder(FfmpegX264Preset preset) : impl_(std::make_unique<Impl>(preset)) {}` | 构造并初始化 FfmpegX264H264Encoder 实例。 |
| [L583](../src/platform/win/FfmpegX264H264Encoder.cpp#L583) | `FfmpegX264H264Encoder::RuntimeAvailable` | 定义 | `bool FfmpegX264H264Encoder::RuntimeAvailable(std::string* error)` | 执行后台循环或调度 runtime available 相关逻辑。 |
| [L593](../src/platform/win/FfmpegX264H264Encoder.cpp#L593) | `FfmpegX264H264Encoder::InitEncode` | 定义 | `int FfmpegX264H264Encoder::InitEncode( const webrtc::VideoCodec* codecSettings, const Settings& /* settings */)` | 创建或初始化 init encode 相关逻辑。 |
| [L600](../src/platform/win/FfmpegX264H264Encoder.cpp#L600) | `FfmpegX264H264Encoder::RegisterEncodeCompleteCallback` | 定义 | `int32_t FfmpegX264H264Encoder::RegisterEncodeCompleteCallback( webrtc::EncodedImageCallback* callback)` | 实现 register encode complete callback 对应的业务或工具逻辑。 |
| [L606](../src/platform/win/FfmpegX264H264Encoder.cpp#L606) | `FfmpegX264H264Encoder::Release` | 定义 | `int32_t FfmpegX264H264Encoder::Release()` | 释放或取消 release 相关逻辑。 |
| [L611](../src/platform/win/FfmpegX264H264Encoder.cpp#L611) | `FfmpegX264H264Encoder::Encode` | 定义 | `int32_t FfmpegX264H264Encoder::Encode( const webrtc::VideoFrame& frame, const std::vector<webrtc::VideoFrameType>* frameTypes)` | 编码 encode 相关逻辑。 |
| [L618](../src/platform/win/FfmpegX264H264Encoder.cpp#L618) | `FfmpegX264H264Encoder::SetRates` | 定义 | `void FfmpegX264H264Encoder::SetRates( const RateControlParameters& parameters)` | 更新或应用 set rates 相关逻辑。 |
| [L625](../src/platform/win/FfmpegX264H264Encoder.cpp#L625) | `FfmpegX264H264Encoder::GetEncoderInfo` | 定义 | `FfmpegX264H264Encoder::GetEncoderInfo() const` | 查询并返回 get encoder info 相关逻辑。 |
| [L643](../src/platform/win/FfmpegX264H264Encoder.cpp#L643) | `RunFfmpegX264EncoderSelfTest` | 定义 | `FfmpegX264EncoderSelfTestResult RunFfmpegX264EncoderSelfTest()` | 执行后台循环或调度 run ffmpeg x264 encoder self test 相关逻辑。 |

## `src/platform/win/FfmpegX264H264Encoder.h`

[打开源码](../src/platform/win/FfmpegX264H264Encoder.h) · **文件作用：** 声明 ffmpeg x264 h264 encoder 相关类型、接口、配置和成员状态。

### 类型

| 行 | 类型 | 种类 | 作用 |
|---:|---|---|---|
| [L16](../src/platform/win/FfmpegX264H264Encoder.h#L16) | `FfmpegX264EncoderSelfTestResult` | struct | 定义 FfmpegX264EncoderSelfTestResult 的 struct 类型和相关状态。 |
| [L26](../src/platform/win/FfmpegX264H264Encoder.h#L26) | `FfmpegX264H264Encoder` | class | CPU H264 encoder backed by the bundled FFmpeg libavcodec/libx264 runtime. It is deliberately an explicit software option; the existing automatic hardware -> FFmpeg/libx264 fallb... |
| [L46](../src/platform/win/FfmpegX264H264Encoder.h#L46) | `Impl` | class | 定义 Impl 的 class 类型和相关状态。 |

### 成员与文件级变量

| 行 | 变量 | 声明 | 作用 |
|---:|---|---|---|
| [L17](../src/platform/win/FfmpegX264H264Encoder.h#L17) | `passed` | `bool passed = false;` | 保存 passed 相关配置或运行状态。 |
| [L18](../src/platform/win/FfmpegX264H264Encoder.h#L18) | `encodedFrames` | `std::uint32_t encodedFrames = 0;` | 保存 encoded frames 相关配置或运行状态。 |
| [L19](../src/platform/win/FfmpegX264H264Encoder.h#L19) | `encodedBytes` | `std::size_t encodedBytes = 0;` | 保存计数、尺寸或速率指标：encoded bytes。 |
| [L20](../src/platform/win/FfmpegX264H264Encoder.h#L20) | `error` | `std::string error;` | 保存最近错误或失败原因：error。 |
| [L46](../src/platform/win/FfmpegX264H264Encoder.h#L46) | `Impl` | `class Impl;` | 保存 impl 相关配置或运行状态。 |
| [L47](../src/platform/win/FfmpegX264H264Encoder.h#L47) | `impl_` | `std::unique_ptr<Impl> impl_;` | 保存 impl 相关配置或运行状态。 |

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L28](../src/platform/win/FfmpegX264H264Encoder.h#L28) | `FfmpegX264H264Encoder` | 声明 | `explicit FfmpegX264H264Encoder( FfmpegX264Preset preset = FfmpegX264Preset::kMedium)` | 实现 ffmpeg x264 h264 encoder 对应的业务或工具逻辑。 |
| [L30](../src/platform/win/FfmpegX264H264Encoder.h#L30) | `~FfmpegX264H264Encoder` | 声明 | `~FfmpegX264H264Encoder() override` | 停止相关活动并释放 FfmpegX264H264Encoder 实例拥有的资源。 |
| [L32](../src/platform/win/FfmpegX264H264Encoder.h#L32) | `RuntimeAvailable` | 声明 | `static bool RuntimeAvailable(std::string* error = nullptr)` | 执行后台循环或调度 runtime available 相关逻辑。 |
| [L34](../src/platform/win/FfmpegX264H264Encoder.h#L34) | `InitEncode` | 声明 | `int InitEncode(const webrtc::VideoCodec* codecSettings, const Settings& settings) override` | 创建或初始化 init encode 相关逻辑。 |
| [L36](../src/platform/win/FfmpegX264H264Encoder.h#L36) | `RegisterEncodeCompleteCallback` | 声明 | `int32_t RegisterEncodeCompleteCallback( webrtc::EncodedImageCallback* callback) override` | 实现 register encode complete callback 对应的业务或工具逻辑。 |
| [L38](../src/platform/win/FfmpegX264H264Encoder.h#L38) | `Release` | 声明 | `int32_t Release() override` | 释放或取消 release 相关逻辑。 |
| [L39](../src/platform/win/FfmpegX264H264Encoder.h#L39) | `Encode` | 声明 | `int32_t Encode( const webrtc::VideoFrame& frame, const std::vector<webrtc::VideoFrameType>* frameTypes) override` | 编码 encode 相关逻辑。 |
| [L42](../src/platform/win/FfmpegX264H264Encoder.h#L42) | `SetRates` | 声明 | `void SetRates(const RateControlParameters& parameters) override` | 更新或应用 set rates 相关逻辑。 |
| [L43](../src/platform/win/FfmpegX264H264Encoder.h#L43) | `GetEncoderInfo` | 声明 | `EncoderInfo GetEncoderInfo() const override` | 查询并返回 get encoder info 相关逻辑。 |
| [L50](../src/platform/win/FfmpegX264H264Encoder.h#L50) | `RunFfmpegX264EncoderSelfTest` | 声明 | `FfmpegX264EncoderSelfTestResult RunFfmpegX264EncoderSelfTest()` | 执行后台循环或调度 run ffmpeg x264 encoder self test 相关逻辑。 |

## `src/platform/win/FfmpegX264H264EncoderFactory.cpp`

[打开源码](../src/platform/win/FfmpegX264H264EncoderFactory.cpp) · **文件作用：** 实现 ffmpeg x264 h264 encoder factory 相关函数与文件级辅助逻辑。

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L12](../src/platform/win/FfmpegX264H264EncoderFactory.cpp#L12) | `FfmpegX264H264EncoderFactory::GetSupportedFormats` | 定义 | `FfmpegX264H264EncoderFactory::GetSupportedFormats() const` | 查询并返回 get supported formats 相关逻辑。 |
| [L21](../src/platform/win/FfmpegX264H264EncoderFactory.cpp#L21) | `FfmpegX264H264EncoderFactory::QueryCodecSupport` | 定义 | `FfmpegX264H264EncoderFactory::QueryCodecSupport( const webrtc::SdpVideoFormat& format, std::optional<std::string> scalabilityMode, std::optional<webrtc::Resolution> /* resolution */) const` | 发起请求或查询 query codec support 相关逻辑。 |
| [L37](../src/platform/win/FfmpegX264H264EncoderFactory.cpp#L37) | `FfmpegX264H264EncoderFactory::Create` | 定义 | `FfmpegX264H264EncoderFactory::Create( const webrtc::Environment& /* environment */, const webrtc::SdpVideoFormat& format)` | 创建或初始化 create 相关逻辑。 |

## `src/platform/win/FfmpegX264H264EncoderFactory.h`

[打开源码](../src/platform/win/FfmpegX264H264EncoderFactory.h) · **文件作用：** 声明 ffmpeg x264 h264 encoder factory 相关类型、接口、配置和成员状态。

### 类型

| 行 | 类型 | 种类 | 作用 |
|---:|---|---|---|
| [L11](../src/platform/win/FfmpegX264H264EncoderFactory.h#L11) | `FfmpegX264H264EncoderFactory` | class | 定义 FfmpegX264H264EncoderFactory 的 class 类型和相关状态。 |

### 成员与文件级变量

| 行 | 变量 | 声明 | 作用 |
|---:|---|---|---|
| [L32](../src/platform/win/FfmpegX264H264EncoderFactory.h#L32) | `preset_` | `FfmpegX264Preset preset_ = FfmpegX264Preset::kMedium;` | 保存 preset 相关配置或运行状态。 |

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L14](../src/platform/win/FfmpegX264H264EncoderFactory.h#L14) | `FfmpegX264H264EncoderFactory` | 定义 | `explicit FfmpegX264H264EncoderFactory( FfmpegX264Preset preset = FfmpegX264Preset::kMedium) : preset_(preset) {}` | 实现 ffmpeg x264 h264 encoder factory 对应的业务或工具逻辑。 |
| [L20](../src/platform/win/FfmpegX264H264EncoderFactory.h#L20) | `GetSupportedFormats` | 声明 | `GetSupportedFormats() const override` | 查询并返回 get supported formats 相关逻辑。 |
| [L22](../src/platform/win/FfmpegX264H264EncoderFactory.h#L22) | `QueryCodecSupport` | 声明 | `CodecSupport QueryCodecSupport( const webrtc::SdpVideoFormat& format, std::optional<std::string> scalabilityMode, std::optional<webrtc::Resolution> resolution) const override` | 发起请求或查询 query codec support 相关逻辑。 |
| [L27](../src/platform/win/FfmpegX264H264EncoderFactory.h#L27) | `Create` | 声明 | `std::unique_ptr<webrtc::VideoEncoder> Create( const webrtc::Environment& environment, const webrtc::SdpVideoFormat& format) override` | 创建或初始化 create 相关逻辑。 |

## `src/platform/win/H264EncoderBenchmark.cpp`

[打开源码](../src/platform/win/H264EncoderBenchmark.cpp) · **文件作用：** 实现 h264 encoder benchmark 相关函数与文件级辅助逻辑。

### 类型

| 行 | 类型 | 种类 | 作用 |
|---:|---|---|---|
| [L100](../src/platform/win/H264EncoderBenchmark.cpp#L100) | `HighResolutionFramePacer` | class | 定义 HighResolutionFramePacer 的 class 类型和相关状态。 |
| [L189](../src/platform/win/H264EncoderBenchmark.cpp#L189) | `BenchmarkFrameSource` | class | 定义 BenchmarkFrameSource 的 class 类型和相关状态。 |
| [L275](../src/platform/win/H264EncoderBenchmark.cpp#L275) | `EncodeCollector` | class | 定义 EncodeCollector 的 class 类型和相关状态。 |
| [L277](../src/platform/win/H264EncoderBenchmark.cpp#L277) | `EncodedSample` | struct | 定义 EncodedSample 的 struct 类型和相关状态。 |
| [L383](../src/platform/win/H264EncoderBenchmark.cpp#L383) | `QualityCollector` | class | 定义 QualityCollector 的 class 类型和相关状态。 |

### 成员与文件级变量

| 行 | 变量 | 声明 | 作用 |
|---:|---|---|---|
| [L48](../src/platform/win/H264EncoderBenchmark.cpp#L48) | `kWidth` | `constexpr int kWidth = 1920;` | 定义 width 的编译期常量或产品边界。 |
| [L49](../src/platform/win/H264EncoderBenchmark.cpp#L49) | `kHeight` | `constexpr int kHeight = 1080;` | 定义 height 的编译期常量或产品边界。 |
| [L50](../src/platform/win/H264EncoderBenchmark.cpp#L50) | `kFrameRate` | `constexpr std::uint32_t kFrameRate = 60;` | 定义 frame rate 的编译期常量或产品边界。 |
| [L51](../src/platform/win/H264EncoderBenchmark.cpp#L51) | `kFrameCount` | `constexpr std::uint32_t kFrameCount = 108;` | 定义 frame count 的编译期常量或产品边界。 |
| [L52](../src/platform/win/H264EncoderBenchmark.cpp#L52) | `kWarmupFrames` | `constexpr std::uint32_t kWarmupFrames = 12;` | 定义 warmup frames 的编译期常量或产品边界。 |
| [L55](../src/platform/win/H264EncoderBenchmark.cpp#L55) | `kBitrateBps` | `constexpr std::uint32_t kBitrateBps = 18'662'400;` | Match ScreenStreamPolicy's 0.15 bits-per-pixel-rate ceiling for 1920x1080@60 instead of starving the quality comparison at 4 Mbps. |

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L57](../src/platform/win/H264EncoderBenchmark.cpp#L57) | `AppendWarning` | 定义 | `void AppendWarning(std::string& destination, std::string warning)` | 实现 append warning 对应的业务或工具逻辑。 |
| [L68](../src/platform/win/H264EncoderBenchmark.cpp#L68) | `HasAnnexBStartCode` | 定义 | `bool HasAnnexBStartCode(const std::uint8_t* data, std::size_t size)` | 判断 has annex b start code 相关逻辑。 |
| [L83](../src/platform/win/H264EncoderBenchmark.cpp#L83) | `ProcessCpuMilliseconds` | 定义 | `double ProcessCpuMilliseconds()` | 接收并处理 process cpu milliseconds 相关逻辑。 |
| [L102](../src/platform/win/H264EncoderBenchmark.cpp#L102) | `HighResolutionFramePacer` | 定义 | `HighResolutionFramePacer()` | 实现 high resolution frame pacer 对应的业务或工具逻辑。 |
| [L115](../src/platform/win/H264EncoderBenchmark.cpp#L115) | `~HighResolutionFramePacer` | 定义 | `~HighResolutionFramePacer()` | 停止相关活动并释放 HighResolutionFramePacer 实例拥有的资源。 |
| [L122](../src/platform/win/H264EncoderBenchmark.cpp#L122) | `IsValid` | 定义 | `bool IsValid() const noexcept { return timer_ != nullptr; }` | 判断 is valid 相关逻辑。 |
| [L124](../src/platform/win/H264EncoderBenchmark.cpp#L124) | `WaitUntil` | 定义 | `bool WaitUntil(Clock::time_point deadline)` | 实现 wait until 对应的业务或工具逻辑。 |
| [L147](../src/platform/win/H264EncoderBenchmark.cpp#L147) | `SyntheticDesktopBackground` | 定义 | `const std::vector<std::uint8_t>& SyntheticDesktopBackground()` | 实现 synthetic desktop background 对应的业务或工具逻辑。 |
| [L170](../src/platform/win/H264EncoderBenchmark.cpp#L170) | `FillMovingDesktop` | 定义 | `void FillMovingDesktop(std::vector<std::uint8_t>& pixels, std::uint32_t frameIndex)` | 实现 fill moving desktop 对应的业务或工具逻辑。 |
| [L191](../src/platform/win/H264EncoderBenchmark.cpp#L191) | `~BenchmarkFrameSource` | 定义 | `~BenchmarkFrameSource()` | 停止相关活动并释放 BenchmarkFrameSource 实例拥有的资源。 |
| [L198](../src/platform/win/H264EncoderBenchmark.cpp#L198) | `Initialize` | 定义 | `bool Initialize(H264EncoderBenchmarkInput input, std::string& error)` | 创建或初始化 initialize 相关逻辑。 |
| [L252](../src/platform/win/H264EncoderBenchmark.cpp#L252) | `Frame` | 定义 | `webrtc::scoped_refptr<webrtc::VideoFrameBuffer> Frame( std::uint32_t index)` | 实现 frame 对应的业务或工具逻辑。 |
| [L284](../src/platform/win/H264EncoderBenchmark.cpp#L284) | `OnFrameDropped` | 定义 | `void OnFrameDropped(std::uint32_t, int, bool) override` | 接收并处理 on frame dropped 相关逻辑。 |
| [L290](../src/platform/win/H264EncoderBenchmark.cpp#L290) | `Submitted` | 定义 | `void Submitted(std::uint32_t timestamp)` | 实现 submitted 对应的业务或工具逻辑。 |
| [L296](../src/platform/win/H264EncoderBenchmark.cpp#L296) | `OnEncodedImage` | 定义 | `Result OnEncodedImage( const webrtc::EncodedImage& image, const webrtc::CodecSpecificInfo*) override` | 接收并处理 on encoded image 相关逻辑。 |
| [L326](../src/platform/win/H264EncoderBenchmark.cpp#L326) | `WaitFor` | 定义 | `bool WaitFor(std::uint32_t count, std::chrono::milliseconds timeout)` | 实现 wait for 对应的业务或工具逻辑。 |
| [L334](../src/platform/win/H264EncoderBenchmark.cpp#L334) | `CopyTo` | 定义 | `void CopyTo(H264EncoderBenchmarkCandidate& result)` | 实现 copy to 对应的业务或工具逻辑。 |
| [L364](../src/platform/win/H264EncoderBenchmark.cpp#L364) | `Samples` | 定义 | `std::vector<EncodedSample> Samples() const` | 实现 samples 对应的业务或工具逻辑。 |
| [L385](../src/platform/win/H264EncoderBenchmark.cpp#L385) | `Decoded` | 定义 | `int32_t Decoded(webrtc::VideoFrame& frame) override` | 解码或解析 decoded 相关逻辑。 |
| [L433](../src/platform/win/H264EncoderBenchmark.cpp#L433) | `WaitFor` | 定义 | `bool WaitFor(std::size_t count)` | 实现 wait for 对应的业务或工具逻辑。 |
| [L441](../src/platform/win/H264EncoderBenchmark.cpp#L441) | `Average` | 定义 | `double Average() const` | 实现 average 对应的业务或工具逻辑。 |
| [L450](../src/platform/win/H264EncoderBenchmark.cpp#L450) | `Count` | 定义 | `std::size_t Count() const` | 实现 count 对应的业务或工具逻辑。 |
| [L462](../src/platform/win/H264EncoderBenchmark.cpp#L462) | `MeasureQuality` | 定义 | `double MeasureQuality( const std::vector<EncodeCollector::EncodedSample>& samples, std::string& error)` | 计算或采集 measure quality 相关逻辑。 |
| [L523](../src/platform/win/H264EncoderBenchmark.cpp#L523) | `BuiltinH264Encoder` | 定义 | `std::unique_ptr<webrtc::VideoEncoder> BuiltinH264Encoder( FfmpegX264Preset quality, std::string& error)` | 实现 builtin h264 encoder 对应的业务或工具逻辑。 |
| [L543](../src/platform/win/H264EncoderBenchmark.cpp#L543) | `BenchmarkCandidate` | 定义 | `H264EncoderBenchmarkCandidate BenchmarkCandidate( std::string id, std::string fallbackName, bool hardware, std::unique_ptr<webrtc::VideoEncoder> encoder, BenchmarkFrameSource& source)` | 实现 benchmark candidate 对应的业务或工具逻辑。 |
| [L740](../src/platform/win/H264EncoderBenchmark.cpp#L740) | `RunH264EncoderBenchmark` | 定义 | `H264EncoderBenchmarkResult RunH264EncoderBenchmark( H264EncoderBenchmarkInput input, FfmpegX264Preset ffmpegX264Preset)` | 执行后台循环或调度 run h264 encoder benchmark 相关逻辑。 |

## `src/platform/win/H264EncoderBenchmark.h`

[打开源码](../src/platform/win/H264EncoderBenchmark.h) · **文件作用：** 声明 h264 encoder benchmark 相关类型、接口、配置和成员状态。

### 类型

| 行 | 类型 | 种类 | 作用 |
|---:|---|---|---|
| [L14](../src/platform/win/H264EncoderBenchmark.h#L14) | `H264EncoderBenchmarkInput` | enum class | 定义 H264EncoderBenchmarkInput 的 enum class 类型和相关状态。 |
| [L28](../src/platform/win/H264EncoderBenchmark.h#L28) | `H264EncoderBenchmarkCandidate` | struct | 定义 H264EncoderBenchmarkCandidate 的 struct 类型和相关状态。 |
| [L50](../src/platform/win/H264EncoderBenchmark.h#L50) | `H264EncoderBenchmarkResult` | struct | 定义 H264EncoderBenchmarkResult 的 struct 类型和相关状态。 |

### 成员与文件级变量

| 行 | 变量 | 声明 | 作用 |
|---:|---|---|---|
| [L19](../src/platform/win/H264EncoderBenchmark.h#L19) | `kAutomaticEncoderMf` | `inline constexpr char kAutomaticEncoderMf[] = "mf_d3d11";` | 定义 automatic encoder mf 的编译期常量或产品边界。 |
| [L20](../src/platform/win/H264EncoderBenchmark.h#L20) | `kAutomaticEncoderOpenH264` | `inline constexpr char kAutomaticEncoderOpenH264[] = "openh264";` | 定义 automatic encoder open h264 的编译期常量或产品边界。 |
| [L21](../src/platform/win/H264EncoderBenchmark.h#L21) | `kAutomaticEncoderFfmpegX264` | `inline constexpr char kAutomaticEncoderFfmpegX264[] = "ffmpeg_x264";` | 定义 automatic encoder ffmpeg x264 的编译期常量或产品边界。 |
| [L22](../src/platform/win/H264EncoderBenchmark.h#L22) | `kAutomaticEncoderFfmpegQsv` | `inline constexpr char kAutomaticEncoderFfmpegQsv[] = "ffmpeg_qsv";` | 定义 automatic encoder ffmpeg qsv 的编译期常量或产品边界。 |
| [L23](../src/platform/win/H264EncoderBenchmark.h#L23) | `kAutomaticEncoderFfmpegNvenc` | `inline constexpr char kAutomaticEncoderFfmpegNvenc[] = "ffmpeg_nvenc";` | 定义 automatic encoder ffmpeg nvenc 的编译期常量或产品边界。 |
| [L24](../src/platform/win/H264EncoderBenchmark.h#L24) | `kAutomaticEncoderFfmpegAmf` | `inline constexpr char kAutomaticEncoderFfmpegAmf[] = "ffmpeg_amf";` | 定义 automatic encoder ffmpeg amf 的编译期常量或产品边界。 |
| [L29](../src/platform/win/H264EncoderBenchmark.h#L29) | `id` | `std::string id;` | 保存身份或作用域标识：id。 |
| [L30](../src/platform/win/H264EncoderBenchmark.h#L30) | `name` | `std::string name;` | 保存路径、地址或显示名称：name。 |
| [L31](../src/platform/win/H264EncoderBenchmark.h#L31) | `inputPath` | `std::string inputPath;` | 保存路径、地址或显示名称：input path。 |
| [L32](../src/platform/win/H264EncoderBenchmark.h#L32) | `hardware` | `bool hardware = false;` | 保存 hardware 相关配置或运行状态。 |
| [L33](../src/platform/win/H264EncoderBenchmark.h#L33) | `passed` | `bool passed = false;` | 保存 passed 相关配置或运行状态。 |
| [L34](../src/platform/win/H264EncoderBenchmark.h#L34) | `submittedFrames` | `std::uint32_t submittedFrames = 0;` | 保存 submitted frames 相关配置或运行状态。 |
| [L35](../src/platform/win/H264EncoderBenchmark.h#L35) | `encodedFrames` | `std::uint32_t encodedFrames = 0;` | 保存 encoded frames 相关配置或运行状态。 |
| [L36](../src/platform/win/H264EncoderBenchmark.h#L36) | `keyFrames` | `std::uint32_t keyFrames = 0;` | 保存 key frames 相关配置或运行状态。 |
| [L37](../src/platform/win/H264EncoderBenchmark.h#L37) | `encodedBytes` | `std::uint64_t encodedBytes = 0;` | 保存计数、尺寸或速率指标：encoded bytes。 |
| [L38](../src/platform/win/H264EncoderBenchmark.h#L38) | `averageLatencyMs` | `double averageLatencyMs = 0.0;` | 保存 average latency ms 相关配置或运行状态。 |
| [L39](../src/platform/win/H264EncoderBenchmark.h#L39) | `p95LatencyMs` | `double p95LatencyMs = 0.0;` | 保存 p95 latency ms 相关配置或运行状态。 |
| [L40](../src/platform/win/H264EncoderBenchmark.h#L40) | `cpuTimePerFrameMs` | `double cpuTimePerFrameMs = 0.0;` | 保存 cpu time per frame ms 相关配置或运行状态。 |
| [L41](../src/platform/win/H264EncoderBenchmark.h#L41) | `inputFramesPerSecond` | `double inputFramesPerSecond = 0.0;` | 保存 input frames per second 相关配置或运行状态。 |
| [L42](../src/platform/win/H264EncoderBenchmark.h#L42) | `averageLumaPsnrDb` | `double averageLumaPsnrDb = 0.0;` | 保存 average luma psnr db 相关配置或运行状态。 |
| [L43](../src/platform/win/H264EncoderBenchmark.h#L43) | `score` | `double score = 0.0;` | 保存 score 相关配置或运行状态。 |
| [L44](../src/platform/win/H264EncoderBenchmark.h#L44) | `dynamicRateControlTested` | `bool dynamicRateControlTested = false;` | 保存 dynamic rate control tested 相关配置或运行状态。 |
| [L45](../src/platform/win/H264EncoderBenchmark.h#L45) | `dynamicRateControlPassed` | `bool dynamicRateControlPassed = false;` | 保存 dynamic rate control passed 相关配置或运行状态。 |
| [L46](../src/platform/win/H264EncoderBenchmark.h#L46) | `warning` | `std::string warning;` | 保存最近错误或失败原因：warning。 |
| [L47](../src/platform/win/H264EncoderBenchmark.h#L47) | `error` | `std::string error;` | 保存最近错误或失败原因：error。 |
| [L51](../src/platform/win/H264EncoderBenchmark.h#L51) | `passed` | `bool passed = false;` | 保存 passed 相关配置或运行状态。 |
| [L52](../src/platform/win/H264EncoderBenchmark.h#L52) | `bestEncoderId` | `std::string bestEncoderId;` | 保存身份或作用域标识：best encoder id。 |
| [L53](../src/platform/win/H264EncoderBenchmark.h#L53) | `bestEncoderName` | `std::string bestEncoderName;` | 保存路径、地址或显示名称：best encoder name。 |
| [L55](../src/platform/win/H264EncoderBenchmark.h#L55) | `kCpuBgra` | `H264EncoderBenchmarkInput::kCpuBgra;` | 定义 cpu bgra 的编译期常量或产品边界。 |
| [L56](../src/platform/win/H264EncoderBenchmark.h#L56) | `error` | `std::string error;` | 保存最近错误或失败原因：error。 |
| [L57](../src/platform/win/H264EncoderBenchmark.h#L57) | `candidates` | `std::vector<H264EncoderBenchmarkCandidate> candidates;` | 保存 candidates 相关配置或运行状态。 |

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L64](../src/platform/win/H264EncoderBenchmark.h#L64) | `RunH264EncoderBenchmark` | 声明 | `H264EncoderBenchmarkResult RunH264EncoderBenchmark( H264EncoderBenchmarkInput input, FfmpegX264Preset ffmpegX264Preset)` | Executes an end-to-end 1920x1080@60 moving-desktop workload. kCpuBgra models libwebrtc DesktopCapturer output; kD3D11Bgra models the native DXGI texture path, including any conv... |

## `src/platform/win/MfD3D11H264Decoder.cpp`

[打开源码](../src/platform/win/MfD3D11H264Decoder.cpp) · **文件作用：** 实现 mf d3 d11 h264 decoder 相关函数与文件级辅助逻辑。

### 类型

| 行 | 类型 | 种类 | 作用 |
|---:|---|---|---|
| [L141](../src/platform/win/MfD3D11H264Decoder.cpp#L141) | `MfD3D11H264Decoder::Impl` | class | 定义 MfD3D11H264Decoder::Impl 的 class 类型和相关状态。 |
| [L413](../src/platform/win/MfD3D11H264Decoder.cpp#L413) | `FrameMetadata` | struct | 定义 FrameMetadata 的 struct 类型和相关状态。 |
| [L426](../src/platform/win/MfD3D11H264Decoder.cpp#L426) | `QueuedInput` | struct | 定义 QueuedInput 的 struct 类型和相关状态。 |

### 成员与文件级变量

| 行 | 变量 | 声明 | 作用 |
|---:|---|---|---|
| [L46](../src/platform/win/MfD3D11H264Decoder.cpp#L46) | `kDefaultProbeWidth` | `constexpr uint32_t kDefaultProbeWidth = 1920;` | 定义 default probe width 的编译期常量或产品边界。 |
| [L47](../src/platform/win/MfD3D11H264Decoder.cpp#L47) | `kDefaultProbeHeight` | `constexpr uint32_t kDefaultProbeHeight = 1080;` | 定义 default probe height 的编译期常量或产品边界。 |
| [L52](../src/platform/win/MfD3D11H264Decoder.cpp#L52) | `kMinimumLowLatencyOutputSurfaces` | `constexpr UINT32 kMinimumLowLatencyOutputSurfaces = 8;` | The renderer can retain the last presented sample while a newer native sample waits in its mailbox. Keep enough decoder-owned DXVA surfaces in the circular output pool so those ... |

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L54](../src/platform/win/MfD3D11H264Decoder.cpp#L54) | `HResultError` | 定义 | `std::string HResultError(const char* operation, HRESULT result)` | 实现 h result error 对应的业务或工具逻辑。 |
| [L62](../src/platform/win/MfD3D11H264Decoder.cpp#L62) | `Narrow` | 定义 | `std::string Narrow(const std::wstring& text)` | 实现 narrow 对应的业务或工具逻辑。 |
| [L94](../src/platform/win/MfD3D11H264Decoder.cpp#L94) | `ActivationName` | 定义 | `std::wstring ActivationName(IMFActivate* activation)` | 实现 activation name 对应的业务或工具逻辑。 |
| [L109](../src/platform/win/MfD3D11H264Decoder.cpp#L109) | `ActivationIsHardware` | 定义 | `bool ActivationIsHardware(IMFActivate* activation)` | 实现 activation is hardware 对应的业务或工具逻辑。 |
| [L126](../src/platform/win/MfD3D11H264Decoder.cpp#L126) | `ReleaseActivations` | 定义 | `void ReleaseActivations(IMFActivate** activations, UINT32 count)` | 释放或取消 release activations 相关逻辑。 |
| [L143](../src/platform/win/MfD3D11H264Decoder.cpp#L143) | `Impl` | 定义 | `explicit Impl(MfD3D11H264DecoderSelection selection, std::string preferredDecoderName, bool requirePreferredDecoder) : selection_(selection), preferredDecoderName_(std::move(preferredDecoderName)), requirePreferredDec...` | 实现 impl 对应的业务或工具逻辑。 |
| [L151](../src/platform/win/MfD3D11H264Decoder.cpp#L151) | `Configure` | 定义 | `bool Configure(const Settings& settings)` | 更新或应用 configure 相关逻辑。 |
| [L231](../src/platform/win/MfD3D11H264Decoder.cpp#L231) | `Decode` | 定义 | `int32_t Decode(const webrtc::EncodedImage& inputImage, int64_t renderTimeMs)` | 解码或解析 decode 相关逻辑。 |
| [L331](../src/platform/win/MfD3D11H264Decoder.cpp#L331) | `RegisterDecodeCompleteCallback` | 定义 | `int32_t RegisterDecodeCompleteCallback( webrtc::DecodedImageCallback* callback)` | 实现 register decode complete callback 对应的业务或工具逻辑。 |
| [L338](../src/platform/win/MfD3D11H264Decoder.cpp#L338) | `Release` | 定义 | `int32_t Release()` | 释放或取消 release 相关逻辑。 |
| [L395](../src/platform/win/MfD3D11H264Decoder.cpp#L395) | `IsConfigured` | 定义 | `bool IsConfigured() const noexcept { return configured_; }` | 判断 is configured 相关逻辑。 |
| [L396](../src/platform/win/MfD3D11H264Decoder.cpp#L396) | `SupportsNativeOutputSamples` | 定义 | `bool SupportsNativeOutputSamples() const noexcept` | 判断 supports native output samples 相关逻辑。 |
| [L400](../src/platform/win/MfD3D11H264Decoder.cpp#L400) | `DecoderKind` | 定义 | `MfD3D11H264DecoderKind DecoderKind() const noexcept` | 解码或解析 decoder kind 相关逻辑。 |
| [L404](../src/platform/win/MfD3D11H264Decoder.cpp#L404) | `IsHardwareAccelerated` | 定义 | `bool IsHardwareAccelerated() const noexcept` | 判断 is hardware accelerated 相关逻辑。 |
| [L408](../src/platform/win/MfD3D11H264Decoder.cpp#L408) | `IsAsynchronous` | 定义 | `bool IsAsynchronous() const noexcept { return asynchronous_; }` | 判断 is asynchronous 相关逻辑。 |
| [L409](../src/platform/win/MfD3D11H264Decoder.cpp#L409) | `LastError` | 定义 | `const std::string& LastError() const noexcept { return lastError_; }` | 实现 last error 对应的业务或工具逻辑。 |
| [L410](../src/platform/win/MfD3D11H264Decoder.cpp#L410) | `DecoderName` | 定义 | `const std::string& DecoderName() const noexcept { return decoderName_; }` | 解码或解析 decoder name 相关逻辑。 |
| [L431](../src/platform/win/MfD3D11H264Decoder.cpp#L431) | `UpdatePeakBacklog` | 定义 | `void UpdatePeakBacklog()` | 更新或应用 update peak backlog 相关逻辑。 |
| [L444](../src/platform/win/MfD3D11H264Decoder.cpp#L444) | `CreateD3D11Device` | 定义 | `bool CreateD3D11Device()` | 创建或初始化 create d3 d11 device 相关逻辑。 |
| [L500](../src/platform/win/MfD3D11H264Decoder.cpp#L500) | `CreateDeviceManager` | 定义 | `bool CreateDeviceManager()` | 创建或初始化 create device manager 相关逻辑。 |
| [L517](../src/platform/win/MfD3D11H264Decoder.cpp#L517) | `SupportsH264HardwareDecoding` | 定义 | `bool SupportsH264HardwareDecoding( UINT32 width, UINT32 height) const` | 判断 supports h264 hardware decoding 相关逻辑。 |
| [L605](../src/platform/win/MfD3D11H264Decoder.cpp#L605) | `CreateAndConfigureTransform` | 定义 | `bool CreateAndConfigureTransform(const Settings& settings)` | 创建或初始化 create and configure transform 相关逻辑。 |
| [L780](../src/platform/win/MfD3D11H264Decoder.cpp#L780) | `ConfigureTransform` | 定义 | `HRESULT ConfigureTransform(IMFTransform* transform, const Settings& settings)` | 更新或应用 configure transform 相关逻辑。 |
| [L892](../src/platform/win/MfD3D11H264Decoder.cpp#L892) | `ConfigureOutputType` | 定义 | `HRESULT ConfigureOutputType(IMFTransform* transform)` | 更新或应用 configure output type 相关逻辑。 |
| [L930](../src/platform/win/MfD3D11H264Decoder.cpp#L930) | `UpdateOutputDimensions` | 定义 | `HRESULT UpdateOutputDimensions(IMFTransform* transform)` | 更新或应用 update output dimensions 相关逻辑。 |
| [L972](../src/platform/win/MfD3D11H264Decoder.cpp#L972) | `CreateInputSample` | 定义 | `HRESULT CreateInputSample( const webrtc::EncodedImage& inputImage, int64_t renderTimeMs, IMFSample** sample, FrameMetadata& metadata)` | 创建或初始化 create input sample 相关逻辑。 |
| [L1065](../src/platform/win/MfD3D11H264Decoder.cpp#L1065) | `StartAsyncEventLoop` | 定义 | `bool StartAsyncEventLoop()` | 启动 start async event loop 相关逻辑。 |
| [L1096](../src/platform/win/MfD3D11H264Decoder.cpp#L1096) | `StopAsyncEventLoop` | 定义 | `void StopAsyncEventLoop()` | 停止 stop async event loop 相关逻辑。 |
| [L1106](../src/platform/win/MfD3D11H264Decoder.cpp#L1106) | `FailAsyncDecoder` | 定义 | `void FailAsyncDecoder(const std::string& error)` | 实现 fail async decoder 对应的业务或工具逻辑。 |
| [L1113](../src/platform/win/MfD3D11H264Decoder.cpp#L1113) | `SubmitQueuedAsyncInputs` | 定义 | `void SubmitQueuedAsyncInputs()` | 实现 submit queued async inputs 对应的业务或工具逻辑。 |
| [L1151](../src/platform/win/MfD3D11H264Decoder.cpp#L1151) | `ProcessAsyncOutput` | 定义 | `void ProcessAsyncOutput()` | 接收并处理 process async output 相关逻辑。 |
| [L1173](../src/platform/win/MfD3D11H264Decoder.cpp#L1173) | `AsyncEventLoop` | 定义 | `void AsyncEventLoop()` | 实现 async event loop 对应的业务或工具逻辑。 |
| [L1258](../src/platform/win/MfD3D11H264Decoder.cpp#L1258) | `DrainOutput` | 定义 | `HRESULT DrainOutput(int& outputCount)` | 实现 drain output 对应的业务或工具逻辑。 |
| [L1282](../src/platform/win/MfD3D11H264Decoder.cpp#L1282) | `ReadOneOutput` | 定义 | `HRESULT ReadOneOutput(bool& delivered)` | 读取或恢复 read one output 相关逻辑。 |
| [L1417](../src/platform/win/MfD3D11H264Decoder.cpp#L1417) | `TakeFrameMetadata` | 定义 | `FrameMetadata TakeFrameMetadata(LONGLONG outputTime)` | 实现 take frame metadata 对应的业务或工具逻辑。 |
| [L1489](../src/platform/win/MfD3D11H264Decoder.cpp#L1489) | `MfD3D11H264Decoder::MfD3D11H264Decoder` | 定义 | `MfD3D11H264Decoder::MfD3D11H264Decoder( MfD3D11H264DecoderSelection selection, std::string preferredDecoderName, bool requirePreferredDecoder) : impl_(std::make_unique<Impl>( selection, std::move(preferredDecoderName)...` | 构造并初始化 MfD3D11H264Decoder 实例。 |
| [L1499](../src/platform/win/MfD3D11H264Decoder.cpp#L1499) | `MfD3D11H264Decoder::~MfD3D11H264Decoder` | 定义 | `MfD3D11H264Decoder::~MfD3D11H264Decoder()` | 停止相关活动并释放 MfD3D11H264Decoder 实例拥有的资源。 |
| [L1504](../src/platform/win/MfD3D11H264Decoder.cpp#L1504) | `MfD3D11H264Decoder::Configure` | 定义 | `bool MfD3D11H264Decoder::Configure(const Settings& settings)` | 更新或应用 configure 相关逻辑。 |
| [L1509](../src/platform/win/MfD3D11H264Decoder.cpp#L1509) | `MfD3D11H264Decoder::Decode` | 定义 | `int32_t MfD3D11H264Decoder::Decode( const webrtc::EncodedImage& inputImage, int64_t renderTimeMs)` | 解码或解析 decode 相关逻辑。 |
| [L1516](../src/platform/win/MfD3D11H264Decoder.cpp#L1516) | `MfD3D11H264Decoder::RegisterDecodeCompleteCallback` | 定义 | `int32_t MfD3D11H264Decoder::RegisterDecodeCompleteCallback( webrtc::DecodedImageCallback* callback)` | 实现 register decode complete callback 对应的业务或工具逻辑。 |
| [L1522](../src/platform/win/MfD3D11H264Decoder.cpp#L1522) | `MfD3D11H264Decoder::Release` | 定义 | `int32_t MfD3D11H264Decoder::Release()` | 释放或取消 release 相关逻辑。 |
| [L1527](../src/platform/win/MfD3D11H264Decoder.cpp#L1527) | `MfD3D11H264Decoder::GetDecoderInfo` | 定义 | `webrtc::VideoDecoder::DecoderInfo MfD3D11H264Decoder::GetDecoderInfo() const` | 查询并返回 get decoder info 相关逻辑。 |
| [L1535](../src/platform/win/MfD3D11H264Decoder.cpp#L1535) | `MfD3D11H264Decoder::ImplementationName` | 定义 | `const char* MfD3D11H264Decoder::ImplementationName() const` | 实现 implementation name 对应的业务或工具逻辑。 |
| [L1548](../src/platform/win/MfD3D11H264Decoder.cpp#L1548) | `MfD3D11H264Decoder::IsConfigured` | 定义 | `bool MfD3D11H264Decoder::IsConfigured() const noexcept` | 判断 is configured 相关逻辑。 |
| [L1553](../src/platform/win/MfD3D11H264Decoder.cpp#L1553) | `MfD3D11H264Decoder::SupportsNativeOutputSamples` | 定义 | `bool MfD3D11H264Decoder::SupportsNativeOutputSamples() const noexcept` | 判断 supports native output samples 相关逻辑。 |
| [L1559](../src/platform/win/MfD3D11H264Decoder.cpp#L1559) | `MfD3D11H264Decoder::DecoderKind` | 定义 | `MfD3D11H264Decoder::DecoderKind() const noexcept` | 解码或解析 decoder kind 相关逻辑。 |
| [L1564](../src/platform/win/MfD3D11H264Decoder.cpp#L1564) | `MfD3D11H264Decoder::IsHardwareAccelerated` | 定义 | `bool MfD3D11H264Decoder::IsHardwareAccelerated() const noexcept` | 判断 is hardware accelerated 相关逻辑。 |
| [L1569](../src/platform/win/MfD3D11H264Decoder.cpp#L1569) | `MfD3D11H264Decoder::IsAsynchronous` | 定义 | `bool MfD3D11H264Decoder::IsAsynchronous() const noexcept` | 判断 is asynchronous 相关逻辑。 |
| [L1574](../src/platform/win/MfD3D11H264Decoder.cpp#L1574) | `MfD3D11H264Decoder::LastError` | 定义 | `const std::string& MfD3D11H264Decoder::LastError() const noexcept` | 实现 last error 对应的业务或工具逻辑。 |
| [L1579](../src/platform/win/MfD3D11H264Decoder.cpp#L1579) | `MfD3D11H264Decoder::DecoderName` | 定义 | `const std::string& MfD3D11H264Decoder::DecoderName() const noexcept` | 解码或解析 decoder name 相关逻辑。 |

## `src/platform/win/MfD3D11H264Decoder.h`

[打开源码](../src/platform/win/MfD3D11H264Decoder.h) · **文件作用：** 声明 mf d3 d11 h264 decoder 相关类型、接口、配置和成员状态。

### 类型

| 行 | 类型 | 种类 | 作用 |
|---:|---|---|---|
| [L14](../src/platform/win/MfD3D11H264Decoder.h#L14) | `MfD3D11H264DecoderKind` | enum class | 定义 MfD3D11H264DecoderKind 的 enum class 类型和相关状态。 |
| [L20](../src/platform/win/MfD3D11H264Decoder.h#L20) | `MfD3D11H264DecoderSelection` | enum class | 定义 MfD3D11H264DecoderSelection 的 enum class 类型和相关状态。 |
| [L26](../src/platform/win/MfD3D11H264Decoder.h#L26) | `MfD3D11H264Decoder` | class | 定义 MfD3D11H264Decoder 的 class 类型和相关状态。 |
| [L56](../src/platform/win/MfD3D11H264Decoder.h#L56) | `Impl` | class | 定义 Impl 的 class 类型和相关状态。 |

### 成员与文件级变量

| 行 | 变量 | 声明 | 作用 |
|---:|---|---|---|
| [L56](../src/platform/win/MfD3D11H264Decoder.h#L56) | `Impl` | `class Impl;` | 保存 impl 相关配置或运行状态。 |
| [L57](../src/platform/win/MfD3D11H264Decoder.h#L57) | `impl_` | `std::unique_ptr<Impl> impl_;` | 保存 impl 相关配置或运行状态。 |

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L33](../src/platform/win/MfD3D11H264Decoder.h#L33) | `~MfD3D11H264Decoder` | 声明 | `~MfD3D11H264Decoder() override` | 停止相关活动并释放 MfD3D11H264Decoder 实例拥有的资源。 |
| [L35](../src/platform/win/MfD3D11H264Decoder.h#L35) | `MfD3D11H264Decoder` | 声明 | `MfD3D11H264Decoder(const MfD3D11H264Decoder&) = delete` | 实现 mf d3 d11 h264 decoder 对应的业务或工具逻辑。 |
| [L38](../src/platform/win/MfD3D11H264Decoder.h#L38) | `Configure` | 声明 | `bool Configure(const Settings& settings) override` | 更新或应用 configure 相关逻辑。 |
| [L39](../src/platform/win/MfD3D11H264Decoder.h#L39) | `Decode` | 声明 | `int32_t Decode(const webrtc::EncodedImage& inputImage, int64_t renderTimeMs) override` | 解码或解析 decode 相关逻辑。 |
| [L41](../src/platform/win/MfD3D11H264Decoder.h#L41) | `RegisterDecodeCompleteCallback` | 声明 | `int32_t RegisterDecodeCompleteCallback( webrtc::DecodedImageCallback* callback) override` | 实现 register decode complete callback 对应的业务或工具逻辑。 |
| [L43](../src/platform/win/MfD3D11H264Decoder.h#L43) | `Release` | 声明 | `int32_t Release() override` | 释放或取消 release 相关逻辑。 |
| [L44](../src/platform/win/MfD3D11H264Decoder.h#L44) | `GetDecoderInfo` | 声明 | `DecoderInfo GetDecoderInfo() const override` | 查询并返回 get decoder info 相关逻辑。 |
| [L45](../src/platform/win/MfD3D11H264Decoder.h#L45) | `ImplementationName` | 声明 | `const char* ImplementationName() const override` | 实现 implementation name 对应的业务或工具逻辑。 |
| [L47](../src/platform/win/MfD3D11H264Decoder.h#L47) | `IsConfigured` | 声明 | `bool IsConfigured() const noexcept` | 判断 is configured 相关逻辑。 |
| [L48](../src/platform/win/MfD3D11H264Decoder.h#L48) | `SupportsNativeOutputSamples` | 声明 | `bool SupportsNativeOutputSamples() const noexcept` | 判断 supports native output samples 相关逻辑。 |
| [L49](../src/platform/win/MfD3D11H264Decoder.h#L49) | `DecoderKind` | 声明 | `MfD3D11H264DecoderKind DecoderKind() const noexcept` | 解码或解析 decoder kind 相关逻辑。 |
| [L50](../src/platform/win/MfD3D11H264Decoder.h#L50) | `IsHardwareAccelerated` | 声明 | `bool IsHardwareAccelerated() const noexcept` | 判断 is hardware accelerated 相关逻辑。 |
| [L51](../src/platform/win/MfD3D11H264Decoder.h#L51) | `IsAsynchronous` | 声明 | `bool IsAsynchronous() const noexcept` | 判断 is asynchronous 相关逻辑。 |
| [L52](../src/platform/win/MfD3D11H264Decoder.h#L52) | `LastError` | 声明 | `const std::string& LastError() const noexcept` | 实现 last error 对应的业务或工具逻辑。 |
| [L53](../src/platform/win/MfD3D11H264Decoder.h#L53) | `DecoderName` | 声明 | `const std::string& DecoderName() const noexcept` | 解码或解析 decoder name 相关逻辑。 |

## `src/platform/win/MfD3D11H264DecoderBenchmark.cpp`

[打开源码](../src/platform/win/MfD3D11H264DecoderBenchmark.cpp) · **文件作用：** 实现 mf d3 d11 h264 decoder benchmark 相关函数与文件级辅助逻辑。

### 类型

| 行 | 类型 | 种类 | 作用 |
|---:|---|---|---|
| [L139](../src/platform/win/MfD3D11H264DecoderBenchmark.cpp#L139) | `EncodedFrame` | struct | 定义 EncodedFrame 的 struct 类型和相关状态。 |
| [L146](../src/platform/win/MfD3D11H264DecoderBenchmark.cpp#L146) | `EncodeCollector` | class | 定义 EncodeCollector 的 class 类型和相关状态。 |
| [L286](../src/platform/win/MfD3D11H264DecoderBenchmark.cpp#L286) | `DecodeCollector` | class | 定义 DecodeCollector 的 class 类型和相关状态。 |
| [L288](../src/platform/win/MfD3D11H264DecoderBenchmark.cpp#L288) | `Submission` | struct | 定义 Submission 的 struct 类型和相关状态。 |

### 成员与文件级变量

| 行 | 变量 | 声明 | 作用 |
|---:|---|---|---|
| [L50](../src/platform/win/MfD3D11H264DecoderBenchmark.cpp#L50) | `kWidth` | `constexpr int kWidth = 1920;` | Match the application's common desktop workload. A decoder that only passes at 720p can still exhibit severe queueing at the actual 1080p/60 target, especially during the sparse... |
| [L51](../src/platform/win/MfD3D11H264DecoderBenchmark.cpp#L51) | `kHeight` | `constexpr int kHeight = 1080;` | 定义 height 的编译期常量或产品边界。 |
| [L52](../src/platform/win/MfD3D11H264DecoderBenchmark.cpp#L52) | `kFrameRate` | `constexpr std::uint32_t kFrameRate = 60;` | 定义 frame rate 的编译期常量或产品边界。 |
| [L53](../src/platform/win/MfD3D11H264DecoderBenchmark.cpp#L53) | `kFrameCount` | `constexpr std::size_t kFrameCount = 72;` | 定义 frame count 的编译期常量或产品边界。 |
| [L54](../src/platform/win/MfD3D11H264DecoderBenchmark.cpp#L54) | `kWarmupFrameCount` | `constexpr std::size_t kWarmupFrameCount = 8;` | 定义 warmup frame count 的编译期常量或产品边界。 |
| [L55](../src/platform/win/MfD3D11H264DecoderBenchmark.cpp#L55) | `kBitrateBps` | `constexpr std::uint32_t kBitrateBps = 6'000'000;` | 定义 bitrate bps 的编译期常量或产品边界。 |
| [L56](../src/platform/win/MfD3D11H264DecoderBenchmark.cpp#L56) | `kRealtimeFrameCount` | `constexpr std::size_t kRealtimeFrameCount = 52;` | 定义 realtime frame count 的编译期常量或产品边界。 |
| [L57](../src/platform/win/MfD3D11H264DecoderBenchmark.cpp#L57) | `kSparseFrameRate` | `constexpr std::uint32_t kSparseFrameRate = 5;` | 定义 sparse frame rate 的编译期常量或产品边界。 |
| [L58](../src/platform/win/MfD3D11H264DecoderBenchmark.cpp#L58) | `kMaximumRealtimeAverageLatencyMs` | `constexpr double kMaximumRealtimeAverageLatencyMs = 45.0;` | 定义 maximum realtime average latency ms 的编译期常量或产品边界。 |
| [L59](../src/platform/win/MfD3D11H264DecoderBenchmark.cpp#L59) | `kMaximumRealtimeP95LatencyMs` | `constexpr double kMaximumRealtimeP95LatencyMs = 80.0;` | 定义 maximum realtime p95 latency ms 的编译期常量或产品边界。 |
| [L60](../src/platform/win/MfD3D11H264DecoderBenchmark.cpp#L60) | `kMaximumSparseAverageLatencyMs` | `constexpr double kMaximumSparseAverageLatencyMs = 120.0;` | 定义 maximum sparse average latency ms 的编译期常量或产品边界。 |
| [L61](../src/platform/win/MfD3D11H264DecoderBenchmark.cpp#L61) | `kMaximumSparseP95LatencyMs` | `constexpr double kMaximumSparseP95LatencyMs = 180.0;` | 定义 maximum sparse p95 latency ms 的编译期常量或产品边界。 |

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L63](../src/platform/win/MfD3D11H264DecoderBenchmark.cpp#L63) | `WideToUtf8` | 定义 | `std::string WideToUtf8(const std::wstring& value)` | 实现 wide to utf8 对应的业务或工具逻辑。 |
| [L81](../src/platform/win/MfD3D11H264DecoderBenchmark.cpp#L81) | `ActivationName` | 定义 | `std::string ActivationName(IMFActivate* activation)` | 实现 activation name 对应的业务或工具逻辑。 |
| [L94](../src/platform/win/MfD3D11H264DecoderBenchmark.cpp#L94) | `ReleaseActivations` | 定义 | `void ReleaseActivations(IMFActivate** activations, UINT32 count)` | 释放或取消 release activations 相关逻辑。 |
| [L107](../src/platform/win/MfD3D11H264DecoderBenchmark.cpp#L107) | `EnumerateCandidateNames` | 定义 | `std::vector<std::string> EnumerateCandidateNames(std::string& error)` | 实现 enumerate candidate names 对应的业务或工具逻辑。 |
| [L148](../src/platform/win/MfD3D11H264DecoderBenchmark.cpp#L148) | `OnEncodedImage` | 定义 | `Result OnEncodedImage( const webrtc::EncodedImage& image, const webrtc::CodecSpecificInfo*) override` | 接收并处理 on encoded image 相关逻辑。 |
| [L164](../src/platform/win/MfD3D11H264DecoderBenchmark.cpp#L164) | `OnFrameDropped` | 定义 | `void OnFrameDropped(std::uint32_t, int, bool) override {}` | 接收并处理 on frame dropped 相关逻辑。 |
| [L166](../src/platform/win/MfD3D11H264DecoderBenchmark.cpp#L166) | `WaitForFrames` | 定义 | `bool WaitForFrames(std::size_t count, std::chrono::milliseconds timeout)` | 实现 wait for frames 对应的业务或工具逻辑。 |
| [L173](../src/platform/win/MfD3D11H264DecoderBenchmark.cpp#L173) | `TakeFrames` | 定义 | `std::vector<EncodedFrame> TakeFrames()` | 实现 take frames 对应的业务或工具逻辑。 |
| [L185](../src/platform/win/MfD3D11H264DecoderBenchmark.cpp#L185) | `FillSyntheticFrame` | 定义 | `void FillSyntheticFrame(webrtc::I420Buffer* buffer, std::size_t frameIndex)` | 实现 fill synthetic frame 对应的业务或工具逻辑。 |
| [L206](../src/platform/win/MfD3D11H264DecoderBenchmark.cpp#L206) | `BuildSyntheticH264` | 定义 | `std::vector<EncodedFrame> BuildSyntheticH264(std::string& error)` | 创建或初始化 build synthetic h264 相关逻辑。 |
| [L293](../src/platform/win/MfD3D11H264DecoderBenchmark.cpp#L293) | `DecodeCollector` | 定义 | `explicit DecodeCollector( std::map<std::uint32_t, Submission>* submitted) : submitted_(submitted) {}` | 解码或解析 decode collector 相关逻辑。 |
| [L298](../src/platform/win/MfD3D11H264DecoderBenchmark.cpp#L298) | `Decoded` | 定义 | `int32_t Decoded(webrtc::VideoFrame& frame) override` | 解码或解析 decoded 相关逻辑。 |
| [L319](../src/platform/win/MfD3D11H264DecoderBenchmark.cpp#L319) | `WaitForCount` | 定义 | `bool WaitForCount(std::size_t count, std::chrono::milliseconds timeout)` | 实现 wait for count 对应的业务或工具逻辑。 |
| [L327](../src/platform/win/MfD3D11H264DecoderBenchmark.cpp#L327) | `Latencies` | 定义 | `std::vector<double> Latencies() const` | 实现 latencies 对应的业务或工具逻辑。 |
| [L333](../src/platform/win/MfD3D11H264DecoderBenchmark.cpp#L333) | `RealtimeLatencies` | 定义 | `std::vector<double> RealtimeLatencies() const` | 实现 realtime latencies 对应的业务或工具逻辑。 |
| [L339](../src/platform/win/MfD3D11H264DecoderBenchmark.cpp#L339) | `SparseLatencies` | 定义 | `std::vector<double> SparseLatencies() const` | 实现 sparse latencies 对应的业务或工具逻辑。 |
| [L345](../src/platform/win/MfD3D11H264DecoderBenchmark.cpp#L345) | `NativeOutput` | 定义 | `bool NativeOutput() const` | 实现 native output 对应的业务或工具逻辑。 |
| [L361](../src/platform/win/MfD3D11H264DecoderBenchmark.cpp#L361) | `SummarizeLatencies` | 定义 | `void SummarizeLatencies( std::vector<double> latencies, std::size_t discardLeading, std::uint32_t* decodedFrames, double* averageLatencyMs, double* p95LatencyMs)` | 实现 summarize latencies 对应的业务或工具逻辑。 |
| [L392](../src/platform/win/MfD3D11H264DecoderBenchmark.cpp#L392) | `BenchmarkDecoder` | 定义 | `MfD3D11H264DecoderBenchmarkCandidate BenchmarkDecoder( const std::string& name, const std::vector<EncodedFrame>& frames, std::unique_ptr<webrtc::VideoDecoder> decoder, bool hardware, bool asynchronous, bool requireNat...` | 实现 benchmark decoder 对应的业务或工具逻辑。 |
| [L499](../src/platform/win/MfD3D11H264DecoderBenchmark.cpp#L499) | `BenchmarkHardwareCandidate` | 定义 | `MfD3D11H264DecoderBenchmarkCandidate BenchmarkHardwareCandidate( const std::string& name, const std::vector<EncodedFrame>& frames)` | 实现 benchmark hardware candidate 对应的业务或工具逻辑。 |
| [L516](../src/platform/win/MfD3D11H264DecoderBenchmark.cpp#L516) | `BenchmarkD3D11VaCandidate` | 定义 | `MfD3D11H264DecoderBenchmarkCandidate BenchmarkD3D11VaCandidate( const std::vector<EncodedFrame>& frames)` | 实现 benchmark d3 d11 va candidate 对应的业务或工具逻辑。 |
| [L527](../src/platform/win/MfD3D11H264DecoderBenchmark.cpp#L527) | `BenchmarkSoftwareCandidate` | 定义 | `MfD3D11H264DecoderBenchmarkCandidate BenchmarkSoftwareCandidate( const std::vector<EncodedFrame>& frames)` | 实现 benchmark software candidate 对应的业务或工具逻辑。 |
| [L560](../src/platform/win/MfD3D11H264DecoderBenchmark.cpp#L560) | `RunMfD3D11H264DecoderBenchmark` | 定义 | `RunMfD3D11H264DecoderBenchmark()` | 执行后台循环或调度 run mf d3 d11 h264 decoder benchmark 相关逻辑。 |

## `src/platform/win/MfD3D11H264DecoderBenchmark.h`

[打开源码](../src/platform/win/MfD3D11H264DecoderBenchmark.h) · **文件作用：** 声明 mf d3 d11 h264 decoder benchmark 相关类型、接口、配置和成员状态。

### 类型

| 行 | 类型 | 种类 | 作用 |
|---:|---|---|---|
| [L12](../src/platform/win/MfD3D11H264DecoderBenchmark.h#L12) | `MfD3D11H264DecoderBenchmarkCandidate` | struct | 定义 MfD3D11H264DecoderBenchmarkCandidate 的 struct 类型和相关状态。 |
| [L30](../src/platform/win/MfD3D11H264DecoderBenchmark.h#L30) | `MfD3D11H264DecoderBenchmarkResult` | struct | 定义 MfD3D11H264DecoderBenchmarkResult 的 struct 类型和相关状态。 |

### 成员与文件级变量

| 行 | 变量 | 声明 | 作用 |
|---:|---|---|---|
| [L13](../src/platform/win/MfD3D11H264DecoderBenchmark.h#L13) | `name` | `std::string name;` | 保存路径、地址或显示名称：name。 |
| [L14](../src/platform/win/MfD3D11H264DecoderBenchmark.h#L14) | `hardware` | `bool hardware = true;` | 保存 hardware 相关配置或运行状态。 |
| [L15](../src/platform/win/MfD3D11H264DecoderBenchmark.h#L15) | `passed` | `bool passed = false;` | 保存 passed 相关配置或运行状态。 |
| [L16](../src/platform/win/MfD3D11H264DecoderBenchmark.h#L16) | `asynchronous` | `bool asynchronous = false;` | 保存 asynchronous 相关配置或运行状态。 |
| [L17](../src/platform/win/MfD3D11H264DecoderBenchmark.h#L17) | `nativeD3D11Output` | `bool nativeD3D11Output = false;` | 保存 native d3 d11 output 相关配置或运行状态。 |
| [L18](../src/platform/win/MfD3D11H264DecoderBenchmark.h#L18) | `decodedFrames` | `std::uint32_t decodedFrames = 0;` | 保存 decoded frames 相关配置或运行状态。 |
| [L19](../src/platform/win/MfD3D11H264DecoderBenchmark.h#L19) | `averageLatencyMs` | `double averageLatencyMs = 0.0;` | 保存 average latency ms 相关配置或运行状态。 |
| [L20](../src/platform/win/MfD3D11H264DecoderBenchmark.h#L20) | `p95LatencyMs` | `double p95LatencyMs = 0.0;` | 保存 p95 latency ms 相关配置或运行状态。 |
| [L21](../src/platform/win/MfD3D11H264DecoderBenchmark.h#L21) | `realtimeDecodedFrames` | `std::uint32_t realtimeDecodedFrames = 0;` | 保存 realtime decoded frames 相关配置或运行状态。 |
| [L22](../src/platform/win/MfD3D11H264DecoderBenchmark.h#L22) | `realtimeAverageLatencyMs` | `double realtimeAverageLatencyMs = 0.0;` | 保存 realtime average latency ms 相关配置或运行状态。 |
| [L23](../src/platform/win/MfD3D11H264DecoderBenchmark.h#L23) | `realtimeP95LatencyMs` | `double realtimeP95LatencyMs = 0.0;` | 保存 realtime p95 latency ms 相关配置或运行状态。 |
| [L24](../src/platform/win/MfD3D11H264DecoderBenchmark.h#L24) | `sparseDecodedFrames` | `std::uint32_t sparseDecodedFrames = 0;` | 保存 sparse decoded frames 相关配置或运行状态。 |
| [L25](../src/platform/win/MfD3D11H264DecoderBenchmark.h#L25) | `sparseAverageLatencyMs` | `double sparseAverageLatencyMs = 0.0;` | 保存 sparse average latency ms 相关配置或运行状态。 |
| [L26](../src/platform/win/MfD3D11H264DecoderBenchmark.h#L26) | `sparseP95LatencyMs` | `double sparseP95LatencyMs = 0.0;` | 保存 sparse p95 latency ms 相关配置或运行状态。 |
| [L27](../src/platform/win/MfD3D11H264DecoderBenchmark.h#L27) | `error` | `std::string error;` | 保存最近错误或失败原因：error。 |
| [L31](../src/platform/win/MfD3D11H264DecoderBenchmark.h#L31) | `passed` | `bool passed = false;` | 保存 passed 相关配置或运行状态。 |
| [L32](../src/platform/win/MfD3D11H264DecoderBenchmark.h#L32) | `bestDecoderName` | `std::string bestDecoderName;` | 保存路径、地址或显示名称：best decoder name。 |
| [L33](../src/platform/win/MfD3D11H264DecoderBenchmark.h#L33) | `bestAverageLatencyMs` | `double bestAverageLatencyMs = 0.0;` | 保存 best average latency ms 相关配置或运行状态。 |
| [L34](../src/platform/win/MfD3D11H264DecoderBenchmark.h#L34) | `bestP95LatencyMs` | `double bestP95LatencyMs = 0.0;` | 保存 best p95 latency ms 相关配置或运行状态。 |
| [L35](../src/platform/win/MfD3D11H264DecoderBenchmark.h#L35) | `adapterName` | `std::string adapterName;` | 保存路径、地址或显示名称：adapter name。 |
| [L36](../src/platform/win/MfD3D11H264DecoderBenchmark.h#L36) | `error` | `std::string error;` | 保存最近错误或失败原因：error。 |
| [L37](../src/platform/win/MfD3D11H264DecoderBenchmark.h#L37) | `candidates` | `std::vector<MfD3D11H264DecoderBenchmarkCandidate> candidates;` | 保存 candidates 相关配置或运行状态。 |

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L45](../src/platform/win/MfD3D11H264DecoderBenchmark.h#L45) | `RunMfD3D11H264DecoderBenchmark` | 声明 | `RunMfD3D11H264DecoderBenchmark()` | 执行后台循环或调度 run mf d3 d11 h264 decoder benchmark 相关逻辑。 |

## `src/platform/win/MfD3D11H264DecoderFactory.cpp`

[打开源码](../src/platform/win/MfD3D11H264DecoderFactory.cpp) · **文件作用：** 实现 mf d3 d11 h264 decoder factory 相关函数与文件级辅助逻辑。

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L12](../src/platform/win/MfD3D11H264DecoderFactory.cpp#L12) | `MfD3D11H264DecoderPreferenceState::SetPreferredDecoderName` | 定义 | `void MfD3D11H264DecoderPreferenceState::SetPreferredDecoderName( std::string name)` | 更新或应用 set preferred decoder name 相关逻辑。 |
| [L20](../src/platform/win/MfD3D11H264DecoderFactory.cpp#L20) | `MfD3D11H264DecoderPreferenceState::PreferredDecoderName` | 定义 | `MfD3D11H264DecoderPreferenceState::PreferredDecoderName() const` | 实现 preferred decoder name 对应的业务或工具逻辑。 |
| [L26](../src/platform/win/MfD3D11H264DecoderFactory.cpp#L26) | `MfD3D11H264DecoderFactory::MfD3D11H264DecoderFactory` | 定义 | `MfD3D11H264DecoderFactory::MfD3D11H264DecoderFactory( MfD3D11H264DecoderSelection selection, std::shared_ptr<MfD3D11H264DecoderPreferenceState> preferenceState) : selection_(selection), preferenceState_(std::move(pref...` | 构造并初始化 MfD3D11H264DecoderFactory 实例。 |
| [L34](../src/platform/win/MfD3D11H264DecoderFactory.cpp#L34) | `MfD3D11H264DecoderFactory::GetSupportedFormats` | 定义 | `MfD3D11H264DecoderFactory::GetSupportedFormats() const` | 查询并返回 get supported formats 相关逻辑。 |
| [L40](../src/platform/win/MfD3D11H264DecoderFactory.cpp#L40) | `MfD3D11H264DecoderFactory::QueryCodecSupport` | 定义 | `MfD3D11H264DecoderFactory::QueryCodecSupport( const webrtc::SdpVideoFormat& format, bool referenceScaling, std::optional<webrtc::Resolution> /* resolution */) const` | 发起请求或查询 query codec support 相关逻辑。 |
| [L58](../src/platform/win/MfD3D11H264DecoderFactory.cpp#L58) | `MfD3D11H264DecoderFactory::Create` | 定义 | `MfD3D11H264DecoderFactory::Create( const webrtc::Environment& /* environment */, const webrtc::SdpVideoFormat& format)` | 创建或初始化 create 相关逻辑。 |

## `src/platform/win/MfD3D11H264DecoderFactory.h`

[打开源码](../src/platform/win/MfD3D11H264DecoderFactory.h) · **文件作用：** 声明 mf d3 d11 h264 decoder factory 相关类型、接口、配置和成员状态。

### 类型

| 行 | 类型 | 种类 | 作用 |
|---:|---|---|---|
| [L18](../src/platform/win/MfD3D11H264DecoderFactory.h#L18) | `MfD3D11H264DecoderPreferenceState` | class | 定义 MfD3D11H264DecoderPreferenceState 的 class 类型和相关状态。 |
| [L28](../src/platform/win/MfD3D11H264DecoderFactory.h#L28) | `MfD3D11H264DecoderFactory` | class | 定义 MfD3D11H264DecoderFactory 的 class 类型和相关状态。 |

### 成员与文件级变量

| 行 | 变量 | 声明 | 作用 |
|---:|---|---|---|
| [L24](../src/platform/win/MfD3D11H264DecoderFactory.h#L24) | `mutex_` | `mutable std::mutex mutex_;` | 保护跨线程共享状态：mutex。 |
| [L25](../src/platform/win/MfD3D11H264DecoderFactory.h#L25) | `preferredDecoderName_` | `std::string preferredDecoderName_;` | 保存路径、地址或显示名称：preferred decoder name。 |
| [L48](../src/platform/win/MfD3D11H264DecoderFactory.h#L48) | `selection_` | `MfD3D11H264DecoderSelection selection_;` | 保存 selection 相关配置或运行状态。 |
| [L49](../src/platform/win/MfD3D11H264DecoderFactory.h#L49) | `preferenceState_` | `std::shared_ptr<MfD3D11H264DecoderPreferenceState> preferenceState_;` | 保存状态机当前状态：preference state。 |

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L20](../src/platform/win/MfD3D11H264DecoderFactory.h#L20) | `SetPreferredDecoderName` | 声明 | `void SetPreferredDecoderName(std::string name)` | 更新或应用 set preferred decoder name 相关逻辑。 |
| [L21](../src/platform/win/MfD3D11H264DecoderFactory.h#L21) | `PreferredDecoderName` | 声明 | `std::string PreferredDecoderName() const` | 实现 preferred decoder name 对应的业务或工具逻辑。 |
| [L30](../src/platform/win/MfD3D11H264DecoderFactory.h#L30) | `MfD3D11H264DecoderFactory` | 声明 | `explicit MfD3D11H264DecoderFactory( MfD3D11H264DecoderSelection selection = MfD3D11H264DecoderSelection::kHardwareOnly, std::shared_ptr<MfD3D11H264DecoderPreferenceState> preferenceState = nullptr)` | 实现 mf d3 d11 h264 decoder factory 对应的业务或工具逻辑。 |
| [L36](../src/platform/win/MfD3D11H264DecoderFactory.h#L36) | `GetSupportedFormats` | 声明 | `std::vector<webrtc::SdpVideoFormat> GetSupportedFormats() const override` | 查询并返回 get supported formats 相关逻辑。 |
| [L38](../src/platform/win/MfD3D11H264DecoderFactory.h#L38) | `QueryCodecSupport` | 声明 | `CodecSupport QueryCodecSupport( const webrtc::SdpVideoFormat& format, bool referenceScaling, std::optional<webrtc::Resolution> resolution) const override` | 发起请求或查询 query codec support 相关逻辑。 |
| [L43](../src/platform/win/MfD3D11H264DecoderFactory.h#L43) | `Create` | 声明 | `std::unique_ptr<webrtc::VideoDecoder> Create( const webrtc::Environment& environment, const webrtc::SdpVideoFormat& format) override` | 创建或初始化 create 相关逻辑。 |

## `src/platform/win/MfD3D11H264DecoderSelfTest.cpp`

[打开源码](../src/platform/win/MfD3D11H264DecoderSelfTest.cpp) · **文件作用：** 实现 mf d3 d11 h264 decoder self test 相关函数与文件级辅助逻辑。

### 类型

| 行 | 类型 | 种类 | 作用 |
|---:|---|---|---|
| [L27](../src/platform/win/MfD3D11H264DecoderSelfTest.cpp#L27) | `DecodeCallback` | class | 定义 DecodeCallback 的 class 类型和相关状态。 |
| [L66](../src/platform/win/MfD3D11H264DecoderSelfTest.cpp#L66) | `NaluPosition` | struct | 定义 NaluPosition 的 struct 类型和相关状态。 |

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L29](../src/platform/win/MfD3D11H264DecoderSelfTest.cpp#L29) | `Decoded` | 定义 | `int32_t Decoded(webrtc::VideoFrame& frame) override` | 解码或解析 decoded 相关逻辑。 |
| [L48](../src/platform/win/MfD3D11H264DecoderSelfTest.cpp#L48) | `WaitForFrame` | 定义 | `bool WaitForFrame(std::chrono::milliseconds timeout)` | 实现 wait for frame 对应的业务或工具逻辑。 |
| [L72](../src/platform/win/MfD3D11H264DecoderSelfTest.cpp#L72) | `FindNalus` | 定义 | `std::vector<NaluPosition> FindNalus(const std::vector<uint8_t>& bytes)` | 查询并返回 find nalus 相关逻辑。 |
| [L100](../src/platform/win/MfD3D11H264DecoderSelfTest.cpp#L100) | `SplitAccessUnits` | 定义 | `std::vector<std::vector<uint8_t>> SplitAccessUnits( const std::vector<uint8_t>& bytes)` | 实现 split access units 对应的业务或工具逻辑。 |
| [L131](../src/platform/win/MfD3D11H264DecoderSelfTest.cpp#L131) | `RunMfD3D11H264DecoderSelfTest` | 定义 | `MfD3D11H264DecoderSelfTestResult RunMfD3D11H264DecoderSelfTest( const std::wstring& annexBFilePath)` | 执行后台循环或调度 run mf d3 d11 h264 decoder self test 相关逻辑。 |

## `src/platform/win/MfD3D11H264DecoderSelfTest.h`

[打开源码](../src/platform/win/MfD3D11H264DecoderSelfTest.h) · **文件作用：** 声明 mf d3 d11 h264 decoder self test 相关类型、接口、配置和成员状态。

### 类型

| 行 | 类型 | 种类 | 作用 |
|---:|---|---|---|
| [L10](../src/platform/win/MfD3D11H264DecoderSelfTest.h#L10) | `MfD3D11H264DecoderSelfTestResult` | struct | 定义 MfD3D11H264DecoderSelfTestResult 的 struct 类型和相关状态。 |

### 成员与文件级变量

| 行 | 变量 | 声明 | 作用 |
|---:|---|---|---|
| [L11](../src/platform/win/MfD3D11H264DecoderSelfTest.h#L11) | `passed` | `bool passed = false;` | 保存 passed 相关配置或运行状态。 |
| [L12](../src/platform/win/MfD3D11H264DecoderSelfTest.h#L12) | `report` | `std::string report;` | 保存 report 相关配置或运行状态。 |

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L15](../src/platform/win/MfD3D11H264DecoderSelfTest.h#L15) | `RunMfD3D11H264DecoderSelfTest` | 声明 | `MfD3D11H264DecoderSelfTestResult RunMfD3D11H264DecoderSelfTest( const std::wstring& annexBFilePath)` | 执行后台循环或调度 run mf d3 d11 h264 decoder self test 相关逻辑。 |

## `src/platform/win/MfD3D11H264Encoder.cpp`

[打开源码](../src/platform/win/MfD3D11H264Encoder.cpp) · **文件作用：** 实现 mf d3 d11 h264 encoder 相关函数与文件级辅助逻辑。

### 类型

| 行 | 类型 | 种类 | 作用 |
|---:|---|---|---|
| [L129](../src/platform/win/MfD3D11H264Encoder.cpp#L129) | `MfD3D11H264Encoder::Impl` | class | 定义 MfD3D11H264Encoder::Impl 的 class 类型和相关状态。 |

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L45](../src/platform/win/MfD3D11H264Encoder.cpp#L45) | `SupportedFrameRateForObservedInput` | 定义 | `uint32_t SupportedFrameRateForObservedInput(double framesPerSecond)` | 实现 supported frame rate for observed input 对应的业务或工具逻辑。 |
| [L65](../src/platform/win/MfD3D11H264Encoder.cpp#L65) | `ReadBooleanAttribute` | 定义 | `bool ReadBooleanAttribute(IMFAttributes* attributes, REFGUID key)` | 读取或恢复 read boolean attribute 相关逻辑。 |
| [L73](../src/platform/win/MfD3D11H264Encoder.cpp#L73) | `CreateVideoType` | 定义 | `ComPtr<IMFMediaType> CreateVideoType(REFGUID subtype, uint32_t width, uint32_t height, uint32_t frameRate)` | 创建或初始化 create video type 相关逻辑。 |
| [L107](../src/platform/win/MfD3D11H264Encoder.cpp#L107) | `IsAnnexB` | 定义 | `bool IsAnnexB(const uint8_t* bytes, size_t length)` | 判断 is annex b 相关逻辑。 |
| [L122](../src/platform/win/MfD3D11H264Encoder.cpp#L122) | `MfQualityVsSpeed` | 定义 | `std::uint32_t MfQualityVsSpeed(FfmpegX264Preset quality)` | 实现 mf quality vs speed 对应的业务或工具逻辑。 |
| [L131](../src/platform/win/MfD3D11H264Encoder.cpp#L131) | `Impl` | 定义 | `explicit Impl( std::shared_ptr<VideoEncoderRuntimeState> runtimeState = nullptr, FfmpegX264Preset quality = FfmpegX264Preset::kMedium) : runtimeState_(std::move(runtimeState)), quality_(quality)` | 实现 impl 对应的业务或工具逻辑。 |
| [L142](../src/platform/win/MfD3D11H264Encoder.cpp#L142) | `~Impl` | 定义 | `~Impl()` | 停止相关活动并释放 Impl 实例拥有的资源。 |
| [L150](../src/platform/win/MfD3D11H264Encoder.cpp#L150) | `QualityVsSpeed` | 定义 | `std::uint32_t QualityVsSpeed() const` | 实现 quality vs speed 对应的业务或工具逻辑。 |
| [L155](../src/platform/win/MfD3D11H264Encoder.cpp#L155) | `Init` | 定义 | `int Init(const webrtc::VideoCodec* codecSettings)` | 创建或初始化 init 相关逻辑。 |
| [L239](../src/platform/win/MfD3D11H264Encoder.cpp#L239) | `RegisterCallback` | 定义 | `int RegisterCallback(webrtc::EncodedImageCallback* callback)` | 实现 register callback 对应的业务或工具逻辑。 |
| [L246](../src/platform/win/MfD3D11H264Encoder.cpp#L246) | `Release` | 定义 | `int Release()` | 释放或取消 release 相关逻辑。 |
| [L253](../src/platform/win/MfD3D11H264Encoder.cpp#L253) | `Encode` | 定义 | `int Encode(const webrtc::VideoFrame& frame, const std::vector<webrtc::VideoFrameType>* frameTypes)` | 编码 encode 相关逻辑。 |
| [L338](../src/platform/win/MfD3D11H264Encoder.cpp#L338) | `SetRates` | 定义 | `void SetRates(const webrtc::VideoEncoder::RateControlParameters& parameters)` | 更新或应用 set rates 相关逻辑。 |
| [L374](../src/platform/win/MfD3D11H264Encoder.cpp#L374) | `RequestSoftwareFallback` | 定义 | `int RequestSoftwareFallback(const char* reason)` | 发起请求或查询 request software fallback 相关逻辑。 |
| [L383](../src/platform/win/MfD3D11H264Encoder.cpp#L383) | `ReportInputFormat` | 定义 | `void ReportInputFormat( const webrtc::scoped_refptr<webrtc::VideoFrameBuffer>& buffer)` | 通知或报告 report input format 相关逻辑。 |
| [L435](../src/platform/win/MfD3D11H264Encoder.cpp#L435) | `ObserveInputFrameRate` | 定义 | `void ObserveInputFrameRate()` | 实现 observe input frame rate 对应的业务或工具逻辑。 |
| [L522](../src/platform/win/MfD3D11H264Encoder.cpp#L522) | `ObserveOutputFrameRate` | 定义 | `void ObserveOutputFrameRate()` | 实现 observe output frame rate 对应的业务或工具逻辑。 |
| [L555](../src/platform/win/MfD3D11H264Encoder.cpp#L555) | `InitializeD3D` | 定义 | `bool InitializeD3D()` | 创建或初始化 initialize d3 d 相关逻辑。 |
| [L612](../src/platform/win/MfD3D11H264Encoder.cpp#L612) | `InitializeTransform` | 定义 | `bool InitializeTransform()` | 创建或初始化 initialize transform 相关逻辑。 |
| [L690](../src/platform/win/MfD3D11H264Encoder.cpp#L690) | `ConfigureTransformMediaTypes` | 定义 | `bool ConfigureTransformMediaTypes(uint32_t frameRate)` | 更新或应用 configure transform media types 相关逻辑。 |
| [L708](../src/platform/win/MfD3D11H264Encoder.cpp#L708) | `ResetVideoProcessor` | 定义 | `void ResetVideoProcessor()` | 重置或移除 reset video processor 相关逻辑。 |
| [L720](../src/platform/win/MfD3D11H264Encoder.cpp#L720) | `DiscardPendingTransformEvents` | 定义 | `void DiscardPendingTransformEvents()` | 实现 discard pending transform events 对应的业务或工具逻辑。 |
| [L735](../src/platform/win/MfD3D11H264Encoder.cpp#L735) | `ReconfigureFrameRate` | 定义 | `bool ReconfigureFrameRate(uint32_t requestedFrameRate)` | 实现 reconfigure frame rate 对应的业务或工具逻辑。 |
| [L769](../src/platform/win/MfD3D11H264Encoder.cpp#L769) | `EnsureVideoProcessor` | 定义 | `bool EnsureVideoProcessor(UINT sourceWidth, UINT sourceHeight)` | 实现 ensure video processor 对应的业务或工具逻辑。 |
| [L855](../src/platform/win/MfD3D11H264Encoder.cpp#L855) | `UploadBgraFrame` | 定义 | `bool UploadBgraFrame(const webrtc::VideoFrame& frame)` | 准备或呈现 upload bgra frame 相关逻辑。 |
| [L962](../src/platform/win/MfD3D11H264Encoder.cpp#L962) | `CreateInputSample` | 定义 | `ComPtr<IMFSample> CreateInputSample( const webrtc::VideoFrame& frame, LONGLONG sampleTime)` | 创建或初始化 create input sample 相关逻辑。 |
| [L1020](../src/platform/win/MfD3D11H264Encoder.cpp#L1020) | `WaitForNeedInput` | 定义 | `bool WaitForNeedInput()` | 实现 wait for need input 对应的业务或工具逻辑。 |
| [L1044](../src/platform/win/MfD3D11H264Encoder.cpp#L1044) | `WaitForOutput` | 定义 | `bool WaitForOutput()` | 实现 wait for output 对应的业务或工具逻辑。 |
| [L1072](../src/platform/win/MfD3D11H264Encoder.cpp#L1072) | `ReadEvent` | 定义 | `bool ReadEvent(MediaEventType& type)` | 读取或恢复 read event 相关逻辑。 |
| [L1093](../src/platform/win/MfD3D11H264Encoder.cpp#L1093) | `ReadOutput` | 定义 | `bool ReadOutput()` | 读取或恢复 read output 相关逻辑。 |
| [L1100](../src/platform/win/MfD3D11H264Encoder.cpp#L1100) | `ReadOutputSample` | 定义 | `HRESULT ReadOutputSample()` | 读取或恢复 read output sample 相关逻辑。 |
| [L1163](../src/platform/win/MfD3D11H264Encoder.cpp#L1163) | `DeliverEncodedImage` | 定义 | `void DeliverEncodedImage(IMFSample* sample, const uint8_t* bytes, size_t length)` | 实现 deliver encoded image 对应的业务或工具逻辑。 |
| [L1204](../src/platform/win/MfD3D11H264Encoder.cpp#L1204) | `SetCodecApiBoolean` | 定义 | `bool SetCodecApiBoolean(const GUID& key, bool value)` | 更新或应用 set codec api boolean 相关逻辑。 |
| [L1218](../src/platform/win/MfD3D11H264Encoder.cpp#L1218) | `SetCodecApiUInt32` | 定义 | `bool SetCodecApiUInt32(const GUID& key, uint32_t value)` | 更新或应用 set codec api u int32 相关逻辑。 |
| [L1231](../src/platform/win/MfD3D11H264Encoder.cpp#L1231) | `ReadCodecApiUInt32` | 定义 | `std::optional<uint32_t> ReadCodecApiUInt32(const GUID& key) const` | 读取或恢复 read codec api u int32 相关逻辑。 |
| [L1282](../src/platform/win/MfD3D11H264Encoder.cpp#L1282) | `ApplyBitrate` | 定义 | `void ApplyBitrate(uint32_t bitrateBps)` | 更新或应用 apply bitrate 相关逻辑。 |
| [L1292](../src/platform/win/MfD3D11H264Encoder.cpp#L1292) | `ReleaseUnlocked` | 定义 | `void ReleaseUnlocked()` | 释放或取消 release unlocked 相关逻辑。 |
| [L1402](../src/platform/win/MfD3D11H264Encoder.cpp#L1402) | `MfD3D11H264Encoder::MfD3D11H264Encoder` | 定义 | `MfD3D11H264Encoder::MfD3D11H264Encoder() : impl_(std::make_unique<Impl>()) {}` | 构造并初始化 MfD3D11H264Encoder 实例。 |
| [L1406](../src/platform/win/MfD3D11H264Encoder.cpp#L1406) | `MfD3D11H264Encoder::MfD3D11H264Encoder` | 定义 | `MfD3D11H264Encoder::MfD3D11H264Encoder( std::shared_ptr<VideoEncoderRuntimeState> runtimeState, FfmpegX264Preset quality) : impl_(std::make_unique<Impl>( std::move(runtimeState), quality)) {}` | 构造并初始化 MfD3D11H264Encoder 实例。 |
| [L1415](../src/platform/win/MfD3D11H264Encoder.cpp#L1415) | `MfD3D11H264Encoder::InitEncode` | 定义 | `int MfD3D11H264Encoder::InitEncode( const webrtc::VideoCodec* codecSettings, const Settings& /* settings */)` | 创建或初始化 init encode 相关逻辑。 |
| [L1422](../src/platform/win/MfD3D11H264Encoder.cpp#L1422) | `MfD3D11H264Encoder::RegisterEncodeCompleteCallback` | 定义 | `int32_t MfD3D11H264Encoder::RegisterEncodeCompleteCallback( webrtc::EncodedImageCallback* callback)` | 实现 register encode complete callback 对应的业务或工具逻辑。 |
| [L1428](../src/platform/win/MfD3D11H264Encoder.cpp#L1428) | `MfD3D11H264Encoder::Release` | 定义 | `int32_t MfD3D11H264Encoder::Release()` | 释放或取消 release 相关逻辑。 |
| [L1433](../src/platform/win/MfD3D11H264Encoder.cpp#L1433) | `MfD3D11H264Encoder::Encode` | 定义 | `int32_t MfD3D11H264Encoder::Encode( const webrtc::VideoFrame& frame, const std::vector<webrtc::VideoFrameType>* frameTypes)` | 编码 encode 相关逻辑。 |
| [L1440](../src/platform/win/MfD3D11H264Encoder.cpp#L1440) | `MfD3D11H264Encoder::SetRates` | 定义 | `void MfD3D11H264Encoder::SetRates( const RateControlParameters& parameters)` | 更新或应用 set rates 相关逻辑。 |
| [L1447](../src/platform/win/MfD3D11H264Encoder.cpp#L1447) | `MfD3D11H264Encoder::GetEncoderInfo` | 定义 | `MfD3D11H264Encoder::GetEncoderInfo() const` | 查询并返回 get encoder info 相关逻辑。 |

## `src/platform/win/MfD3D11H264Encoder.h`

[打开源码](../src/platform/win/MfD3D11H264Encoder.h) · **文件作用：** 声明 mf d3 d11 h264 encoder 相关类型、接口、配置和成员状态。

### 类型

| 行 | 类型 | 种类 | 作用 |
|---:|---|---|---|
| [L16](../src/platform/win/MfD3D11H264Encoder.h#L16) | `MfD3D11H264Encoder` | class | Windows H264 encoder backed by a D3D11-aware Media Foundation hardware MFT. It accepts DesktopBgraFrameBuffer without an eager I420 conversion. Other buffer types deliberately r... |
| [L36](../src/platform/win/MfD3D11H264Encoder.h#L36) | `Impl` | class | 定义 Impl 的 class 类型和相关状态。 |

### 成员与文件级变量

| 行 | 变量 | 声明 | 作用 |
|---:|---|---|---|
| [L36](../src/platform/win/MfD3D11H264Encoder.h#L36) | `Impl` | `class Impl;` | 保存 impl 相关配置或运行状态。 |
| [L37](../src/platform/win/MfD3D11H264Encoder.h#L37) | `impl_` | `std::unique_ptr<Impl> impl_;` | 保存 impl 相关配置或运行状态。 |

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L18](../src/platform/win/MfD3D11H264Encoder.h#L18) | `MfD3D11H264Encoder` | 声明 | `MfD3D11H264Encoder()` | 实现 mf d3 d11 h264 encoder 对应的业务或工具逻辑。 |
| [L19](../src/platform/win/MfD3D11H264Encoder.h#L19) | `MfD3D11H264Encoder` | 声明 | `explicit MfD3D11H264Encoder( std::shared_ptr<VideoEncoderRuntimeState> runtimeState, FfmpegX264Preset quality = FfmpegX264Preset::kMedium)` | 实现 mf d3 d11 h264 encoder 对应的业务或工具逻辑。 |
| [L22](../src/platform/win/MfD3D11H264Encoder.h#L22) | `~MfD3D11H264Encoder` | 声明 | `~MfD3D11H264Encoder() override` | 停止相关活动并释放 MfD3D11H264Encoder 实例拥有的资源。 |
| [L24](../src/platform/win/MfD3D11H264Encoder.h#L24) | `InitEncode` | 声明 | `int InitEncode(const webrtc::VideoCodec* codecSettings, const Settings& settings) override` | 创建或初始化 init encode 相关逻辑。 |
| [L26](../src/platform/win/MfD3D11H264Encoder.h#L26) | `RegisterEncodeCompleteCallback` | 声明 | `int32_t RegisterEncodeCompleteCallback( webrtc::EncodedImageCallback* callback) override` | 实现 register encode complete callback 对应的业务或工具逻辑。 |
| [L28](../src/platform/win/MfD3D11H264Encoder.h#L28) | `Release` | 声明 | `int32_t Release() override` | 释放或取消 release 相关逻辑。 |
| [L29](../src/platform/win/MfD3D11H264Encoder.h#L29) | `Encode` | 声明 | `int32_t Encode( const webrtc::VideoFrame& frame, const std::vector<webrtc::VideoFrameType>* frameTypes) override` | 编码 encode 相关逻辑。 |
| [L32](../src/platform/win/MfD3D11H264Encoder.h#L32) | `SetRates` | 声明 | `void SetRates(const RateControlParameters& parameters) override` | 更新或应用 set rates 相关逻辑。 |
| [L33](../src/platform/win/MfD3D11H264Encoder.h#L33) | `GetEncoderInfo` | 声明 | `EncoderInfo GetEncoderInfo() const override` | 查询并返回 get encoder info 相关逻辑。 |

## `src/platform/win/MfD3D11H264EncoderFactory.cpp`

[打开源码](../src/platform/win/MfD3D11H264EncoderFactory.cpp) · **文件作用：** 实现 mf d3 d11 h264 encoder factory 相关函数与文件级辅助逻辑。

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L11](../src/platform/win/MfD3D11H264EncoderFactory.cpp#L11) | `MfD3D11H264EncoderFactory::MfD3D11H264EncoderFactory` | 定义 | `MfD3D11H264EncoderFactory::MfD3D11H264EncoderFactory( std::shared_ptr<VideoEncoderRuntimeState> runtimeState, FfmpegX264Preset quality) : runtimeState_(std::move(runtimeState)), quality_(quality) {}` | 构造并初始化 MfD3D11H264EncoderFactory 实例。 |
| [L18](../src/platform/win/MfD3D11H264EncoderFactory.cpp#L18) | `MfD3D11H264EncoderFactory::GetSupportedFormats` | 定义 | `MfD3D11H264EncoderFactory::GetSupportedFormats() const` | 查询并返回 get supported formats 相关逻辑。 |
| [L24](../src/platform/win/MfD3D11H264EncoderFactory.cpp#L24) | `MfD3D11H264EncoderFactory::QueryCodecSupport` | 定义 | `MfD3D11H264EncoderFactory::QueryCodecSupport( const webrtc::SdpVideoFormat& format, std::optional<std::string> scalabilityMode, std::optional<webrtc::Resolution> /* resolution */) const` | 发起请求或查询 query codec support 相关逻辑。 |
| [L43](../src/platform/win/MfD3D11H264EncoderFactory.cpp#L43) | `MfD3D11H264EncoderFactory::Create` | 定义 | `MfD3D11H264EncoderFactory::Create( const webrtc::Environment& /* environment */, const webrtc::SdpVideoFormat& format)` | 创建或初始化 create 相关逻辑。 |

## `src/platform/win/MfD3D11H264EncoderFactory.h`

[打开源码](../src/platform/win/MfD3D11H264EncoderFactory.h) · **文件作用：** 声明 mf d3 d11 h264 encoder factory 相关类型、接口、配置和成员状态。

### 类型

| 行 | 类型 | 种类 | 作用 |
|---:|---|---|---|
| [L13](../src/platform/win/MfD3D11H264EncoderFactory.h#L13) | `MfD3D11H264EncoderFactory` | class | 定义 MfD3D11H264EncoderFactory 的 class 类型和相关状态。 |

### 成员与文件级变量

| 行 | 变量 | 声明 | 作用 |
|---:|---|---|---|
| [L34](../src/platform/win/MfD3D11H264EncoderFactory.h#L34) | `runtimeState_` | `std::shared_ptr<VideoEncoderRuntimeState> runtimeState_;` | 保存状态机当前状态：runtime state。 |
| [L35](../src/platform/win/MfD3D11H264EncoderFactory.h#L35) | `quality_` | `FfmpegX264Preset quality_ = FfmpegX264Preset::kMedium;` | 保存 quality 相关配置或运行状态。 |

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L16](../src/platform/win/MfD3D11H264EncoderFactory.h#L16) | `MfD3D11H264EncoderFactory` | 声明 | `MfD3D11H264EncoderFactory() = default` | 实现 mf d3 d11 h264 encoder factory 对应的业务或工具逻辑。 |
| [L17](../src/platform/win/MfD3D11H264EncoderFactory.h#L17) | `MfD3D11H264EncoderFactory` | 声明 | `explicit MfD3D11H264EncoderFactory( std::shared_ptr<VideoEncoderRuntimeState> runtimeState, FfmpegX264Preset quality = FfmpegX264Preset::kMedium)` | 实现 mf d3 d11 h264 encoder factory 对应的业务或工具逻辑。 |
| [L22](../src/platform/win/MfD3D11H264EncoderFactory.h#L22) | `GetSupportedFormats` | 声明 | `GetSupportedFormats() const override` | 查询并返回 get supported formats 相关逻辑。 |
| [L24](../src/platform/win/MfD3D11H264EncoderFactory.h#L24) | `QueryCodecSupport` | 声明 | `CodecSupport QueryCodecSupport( const webrtc::SdpVideoFormat& format, std::optional<std::string> scalabilityMode, std::optional<webrtc::Resolution> resolution) const override` | 发起请求或查询 query codec support 相关逻辑。 |
| [L29](../src/platform/win/MfD3D11H264EncoderFactory.h#L29) | `Create` | 声明 | `std::unique_ptr<webrtc::VideoEncoder> Create( const webrtc::Environment& environment, const webrtc::SdpVideoFormat& format) override` | 创建或初始化 create 相关逻辑。 |

## `src/platform/win/MfH264EncoderCapabilityProbe.cpp`

[打开源码](../src/platform/win/MfH264EncoderCapabilityProbe.cpp) · **文件作用：** 实现 mf h264 encoder capability probe 相关函数与文件级辅助逻辑。

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L27](../src/platform/win/MfH264EncoderCapabilityProbe.cpp#L27) | `HResultMessage` | 定义 | `std::string HResultMessage(const char* operation, HRESULT result)` | 实现 h result message 对应的业务或工具逻辑。 |
| [L35](../src/platform/win/MfH264EncoderCapabilityProbe.cpp#L35) | `WideToUtf8` | 定义 | `std::string WideToUtf8(const wchar_t* text, size_t length)` | 实现 wide to utf8 对应的业务或工具逻辑。 |
| [L63](../src/platform/win/MfH264EncoderCapabilityProbe.cpp#L63) | `GuidToString` | 定义 | `std::string GuidToString(REFGUID guid)` | 实现 guid to string 对应的业务或工具逻辑。 |
| [L73](../src/platform/win/MfH264EncoderCapabilityProbe.cpp#L73) | `ReadBooleanAttribute` | 定义 | `bool ReadBooleanAttribute(IMFAttributes* attributes, REFGUID key)` | 读取或恢复 read boolean attribute 相关逻辑。 |
| [L83](../src/platform/win/MfH264EncoderCapabilityProbe.cpp#L83) | `CreateVideoType` | 定义 | `ComPtr<IMFMediaType> CreateVideoType(REFGUID subtype, uint32_t width, uint32_t height, uint32_t frameRateNumerator, uint32_t frameRateDenominator)` | 创建或初始化 create video type 相关逻辑。 |
| [L107](../src/platform/win/MfH264EncoderCapabilityProbe.cpp#L107) | `TestCpuNv12Input` | 定义 | `bool TestCpuNv12Input(IMFTransform* transform)` | 实现 test cpu nv12 input 对应的业务或工具逻辑。 |
| [L150](../src/platform/win/MfH264EncoderCapabilityProbe.cpp#L150) | `UpdateMaximumVideoMode` | 定义 | `void UpdateMaximumVideoMode(IMFMediaType* type, MfH264EncoderCapability& capability)` | 更新或应用 update maximum video mode 相关逻辑。 |
| [L197](../src/platform/win/MfH264EncoderCapabilityProbe.cpp#L197) | `EnumerateInputModes` | 定义 | `void EnumerateInputModes(IMFTransform* transform, MfH264EncoderCapability& capability)` | 实现 enumerate input modes 对应的业务或工具逻辑。 |
| [L216](../src/platform/win/MfH264EncoderCapabilityProbe.cpp#L216) | `EnumerateH264Profiles` | 定义 | `void EnumerateH264Profiles(IMFTransform* transform, MfH264EncoderCapability& capability)` | 实现 enumerate h264 profiles 对应的业务或工具逻辑。 |
| [L248](../src/platform/win/MfH264EncoderCapabilityProbe.cpp#L248) | `CreateD3D11ProbeDevice` | 定义 | `ComPtr<ID3D11Device> CreateD3D11ProbeDevice()` | 创建或初始化 create d3 d11 probe device 相关逻辑。 |
| [L289](../src/platform/win/MfH264EncoderCapabilityProbe.cpp#L289) | `TestD3D11DeviceManager` | 定义 | `bool TestD3D11DeviceManager(IMFTransform* transform, ID3D11Device* device)` | 实现 test d3 d11 device manager 对应的业务或工具逻辑。 |
| [L307](../src/platform/win/MfH264EncoderCapabilityProbe.cpp#L307) | `QueryCodecApi` | 定义 | `void QueryCodecApi(IMFTransform* transform, MfH264EncoderCapability& capability)` | 发起请求或查询 query codec api 相关逻辑。 |
| [L324](../src/platform/win/MfH264EncoderCapabilityProbe.cpp#L324) | `InspectEncoder` | 定义 | `MfH264EncoderCapability InspectEncoder(IMFActivate* activation, ID3D11Device* d3d11Device)` | 实现 inspect encoder 对应的业务或工具逻辑。 |
| [L391](../src/platform/win/MfH264EncoderCapabilityProbe.cpp#L391) | `ProbeMfH264EncoderCapabilities` | 定义 | `MfH264EncoderCapabilityReport ProbeMfH264EncoderCapabilities()` | 实现 probe mf h264 encoder capabilities 对应的业务或工具逻辑。 |

## `src/platform/win/MfH264EncoderCapabilityProbe.h`

[打开源码](../src/platform/win/MfH264EncoderCapabilityProbe.h) · **文件作用：** 声明 mf h264 encoder capability probe 相关类型、接口、配置和成员状态。

### 类型

| 行 | 类型 | 种类 | 作用 |
|---:|---|---|---|
| [L12](../src/platform/win/MfH264EncoderCapabilityProbe.h#L12) | `MfH264EncoderCapability` | struct | 定义 MfH264EncoderCapability 的 struct 类型和相关状态。 |
| [L34](../src/platform/win/MfH264EncoderCapabilityProbe.h#L34) | `MfH264EncoderCapabilityReport` | struct | 定义 MfH264EncoderCapabilityReport 的 struct 类型和相关状态。 |
| [L45](../src/platform/win/MfH264EncoderCapabilityProbe.h#L45) | `MfH264EncoderCapabilityCache` | struct | Persistent cache stores descriptors only. Hardware MFTs, COM interfaces and D3D11 objects are recreated for every process/session. |

### 成员与文件级变量

| 行 | 变量 | 声明 | 作用 |
|---:|---|---|---|
| [L13](../src/platform/win/MfH264EncoderCapabilityProbe.h#L13) | `name` | `std::string name;` | 保存路径、地址或显示名称：name。 |
| [L14](../src/platform/win/MfH264EncoderCapabilityProbe.h#L14) | `clsid` | `std::string clsid;` | 保存身份或作用域标识：clsid。 |
| [L15](../src/platform/win/MfH264EncoderCapabilityProbe.h#L15) | `hardwareUrl` | `std::string hardwareUrl;` | 保存路径、地址或显示名称：hardware url。 |
| [L17](../src/platform/win/MfH264EncoderCapabilityProbe.h#L17) | `asynchronous` | `bool asynchronous = false;` | 保存 asynchronous 相关配置或运行状态。 |
| [L18](../src/platform/win/MfH264EncoderCapabilityProbe.h#L18) | `d3d11Aware` | `bool d3d11Aware = false;` | 保存 d3d11 aware 相关配置或运行状态。 |
| [L19](../src/platform/win/MfH264EncoderCapabilityProbe.h#L19) | `d3d11DeviceManagerAccepted` | `bool d3d11DeviceManagerAccepted = false;` | 保存 d3d11 device manager accepted 相关配置或运行状态。 |
| [L20](../src/platform/win/MfH264EncoderCapabilityProbe.h#L20) | `cpuNv12InputSupported` | `bool cpuNv12InputSupported = false;` | 保存 cpu nv12 input supported 相关配置或运行状态。 |
| [L21](../src/platform/win/MfH264EncoderCapabilityProbe.h#L21) | `lowLatencyModeSupported` | `bool lowLatencyModeSupported = false;` | 保存 low latency mode supported 相关配置或运行状态。 |
| [L22](../src/platform/win/MfH264EncoderCapabilityProbe.h#L22) | `dynamicBitrateSupported` | `bool dynamicBitrateSupported = false;` | 保存 dynamic bitrate supported 相关配置或运行状态。 |
| [L23](../src/platform/win/MfH264EncoderCapabilityProbe.h#L23) | `dynamicBitrateModifiable` | `bool dynamicBitrateModifiable = false;` | 保存 dynamic bitrate modifiable 相关配置或运行状态。 |
| [L24](../src/platform/win/MfH264EncoderCapabilityProbe.h#L24) | `dynamicFormatChangeSupported` | `bool dynamicFormatChangeSupported = false;` | 保存 dynamic format change supported 相关配置或运行状态。 |
| [L26](../src/platform/win/MfH264EncoderCapabilityProbe.h#L26) | `maximumWidth` | `uint32_t maximumWidth = 0;` | 保存计数、尺寸或速率指标：maximum width。 |
| [L27](../src/platform/win/MfH264EncoderCapabilityProbe.h#L27) | `maximumHeight` | `uint32_t maximumHeight = 0;` | 保存计数、尺寸或速率指标：maximum height。 |
| [L28](../src/platform/win/MfH264EncoderCapabilityProbe.h#L28) | `maximumFrameRateNumerator` | `uint32_t maximumFrameRateNumerator = 0;` | 保存 maximum frame rate numerator 相关配置或运行状态。 |
| [L29](../src/platform/win/MfH264EncoderCapabilityProbe.h#L29) | `maximumFrameRateDenominator` | `uint32_t maximumFrameRateDenominator = 0;` | 保存 maximum frame rate denominator 相关配置或运行状态。 |
| [L30](../src/platform/win/MfH264EncoderCapabilityProbe.h#L30) | `h264Profiles` | `std::vector<uint32_t> h264Profiles;` | 保存 h264 profiles 相关配置或运行状态。 |
| [L31](../src/platform/win/MfH264EncoderCapabilityProbe.h#L31) | `warnings` | `std::vector<std::string> warnings;` | 保存 warnings 相关配置或运行状态。 |
| [L35](../src/platform/win/MfH264EncoderCapabilityProbe.h#L35) | `mediaFoundationStarted` | `bool mediaFoundationStarted = false;` | 保存 media foundation started 相关配置或运行状态。 |
| [L36](../src/platform/win/MfH264EncoderCapabilityProbe.h#L36) | `d3d11ProbeDeviceCreated` | `bool d3d11ProbeDeviceCreated = false;` | 保存 d3d11 probe device created 相关配置或运行状态。 |
| [L37](../src/platform/win/MfH264EncoderCapabilityProbe.h#L37) | `enumerationSucceeded` | `bool enumerationSucceeded = false;` | 保存 enumeration succeeded 相关配置或运行状态。 |
| [L38](../src/platform/win/MfH264EncoderCapabilityProbe.h#L38) | `hardwareEncoderCount` | `uint32_t hardwareEncoderCount = 0;` | 保存计数、尺寸或速率指标：hardware encoder count。 |
| [L39](../src/platform/win/MfH264EncoderCapabilityProbe.h#L39) | `encoders` | `std::vector<MfH264EncoderCapability> encoders;` | 保存 encoders 相关配置或运行状态。 |
| [L40](../src/platform/win/MfH264EncoderCapabilityProbe.h#L40) | `warnings` | `std::vector<std::string> warnings;` | 保存 warnings 相关配置或运行状态。 |
| [L46](../src/platform/win/MfH264EncoderCapabilityProbe.h#L46) | `hardwareFingerprint` | `std::string hardwareFingerprint;` | 保存 hardware fingerprint 相关配置或运行状态。 |
| [L47](../src/platform/win/MfH264EncoderCapabilityProbe.h#L47) | `valid` | `bool valid = false;` | 保存身份或作用域标识：valid。 |
| [L48](../src/platform/win/MfH264EncoderCapabilityProbe.h#L48) | `hardwareEncoderAvailable` | `bool hardwareEncoderAvailable = false;` | 保存能力或开关状态：hardware encoder available。 |
| [L49](../src/platform/win/MfH264EncoderCapabilityProbe.h#L49) | `cpuNv12InputSupported` | `bool cpuNv12InputSupported = false;` | 保存 cpu nv12 input supported 相关配置或运行状态。 |
| [L50](../src/platform/win/MfH264EncoderCapabilityProbe.h#L50) | `d3d11InputCandidate` | `bool d3d11InputCandidate = false;` | 保存 d3d11 input candidate 相关配置或运行状态。 |
| [L51](../src/platform/win/MfH264EncoderCapabilityProbe.h#L51) | `hardwareEncoderCount` | `uint32_t hardwareEncoderCount = 0;` | 保存计数、尺寸或速率指标：hardware encoder count。 |
| [L52](../src/platform/win/MfH264EncoderCapabilityProbe.h#L52) | `descriptions` | `std::vector<std::string> descriptions;` | 保存 descriptions 相关配置或运行状态。 |
| [L53](../src/platform/win/MfH264EncoderCapabilityProbe.h#L53) | `warnings` | `std::vector<std::string> warnings;` | 保存 warnings 相关配置或运行状态。 |

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L56](../src/platform/win/MfH264EncoderCapabilityProbe.h#L56) | `ProbeMfH264EncoderCapabilities` | 声明 | `MfH264EncoderCapabilityReport ProbeMfH264EncoderCapabilities()` | 实现 probe mf h264 encoder capabilities 对应的业务或工具逻辑。 |

## `src/platform/win/MfH264EncoderSelfTest.cpp`

[打开源码](../src/platform/win/MfH264EncoderSelfTest.cpp) · **文件作用：** 实现 mf h264 encoder self test 相关函数与文件级辅助逻辑。

### 类型

| 行 | 类型 | 种类 | 作用 |
|---:|---|---|---|
| [L117](../src/platform/win/MfH264EncoderSelfTest.cpp#L117) | `D3D11Nv12FrameGenerator` | class | 定义 D3D11Nv12FrameGenerator 的 class 类型和相关状态。 |
| [L388](../src/platform/win/MfH264EncoderSelfTest.cpp#L388) | `HardwareEncoderRunner` | class | 定义 HardwareEncoderRunner 的 class 类型和相关状态。 |

### 成员与文件级变量

| 行 | 变量 | 声明 | 作用 |
|---:|---|---|---|
| [L31](../src/platform/win/MfH264EncoderSelfTest.cpp#L31) | `kWidth` | `constexpr uint32_t kWidth = 1920;` | 定义 width 的编译期常量或产品边界。 |
| [L32](../src/platform/win/MfH264EncoderSelfTest.cpp#L32) | `kHeight` | `constexpr uint32_t kHeight = 1080;` | 定义 height 的编译期常量或产品边界。 |
| [L33](../src/platform/win/MfH264EncoderSelfTest.cpp#L33) | `kFrameRate` | `constexpr uint32_t kFrameRate = 30;` | 定义 frame rate 的编译期常量或产品边界。 |
| [L34](../src/platform/win/MfH264EncoderSelfTest.cpp#L34) | `kFrameCount` | `constexpr uint32_t kFrameCount = 90;` | 定义 frame count 的编译期常量或产品边界。 |
| [L35](../src/platform/win/MfH264EncoderSelfTest.cpp#L35) | `kInitialBitrate` | `constexpr uint32_t kInitialBitrate = 4'000'000;` | 定义 initial bitrate 的编译期常量或产品边界。 |
| [L36](../src/platform/win/MfH264EncoderSelfTest.cpp#L36) | `kAdjustedBitrate` | `constexpr uint32_t kAdjustedBitrate = 2'500'000;` | 定义 adjusted bitrate 的编译期常量或产品边界。 |
| [L37](../src/platform/win/MfH264EncoderSelfTest.cpp#L37) | `kFrameDuration100ns` | `constexpr LONGLONG kFrameDuration100ns = 10'000'000LL / kFrameRate;` | 定义 frame duration100ns 的编译期常量或产品边界。 |

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L40](../src/platform/win/MfH264EncoderSelfTest.cpp#L40) | `HResultMessage` | 定义 | `std::string HResultMessage(const char* operation, HRESULT result)` | 实现 h result message 对应的业务或工具逻辑。 |
| [L48](../src/platform/win/MfH264EncoderSelfTest.cpp#L48) | `WideToUtf8` | 定义 | `std::string WideToUtf8(const wchar_t* text, size_t length)` | 实现 wide to utf8 对应的业务或工具逻辑。 |
| [L74](../src/platform/win/MfH264EncoderSelfTest.cpp#L74) | `ReadBooleanAttribute` | 定义 | `bool ReadBooleanAttribute(IMFAttributes* attributes, REFGUID key)` | 读取或恢复 read boolean attribute 相关逻辑。 |
| [L82](../src/platform/win/MfH264EncoderSelfTest.cpp#L82) | `HasAnnexBStartCode` | 定义 | `bool HasAnnexBStartCode(const uint8_t* bytes, size_t length)` | 判断 has annex b start code 相关逻辑。 |
| [L97](../src/platform/win/MfH264EncoderSelfTest.cpp#L97) | `CreateVideoType` | 定义 | `ComPtr<IMFMediaType> CreateVideoType(REFGUID subtype)` | 创建或初始化 create video type 相关逻辑。 |
| [L119](../src/platform/win/MfH264EncoderSelfTest.cpp#L119) | `Initialize` | 定义 | `bool Initialize(std::string& error)` | 创建或初始化 initialize 相关逻辑。 |
| [L266](../src/platform/win/MfH264EncoderSelfTest.cpp#L266) | `DeviceManager` | 定义 | `ComPtr<IMFDXGIDeviceManager> DeviceManager() const` | 实现 device manager 对应的业务或工具逻辑。 |
| [L271](../src/platform/win/MfH264EncoderSelfTest.cpp#L271) | `CreateFrameSample` | 定义 | `ComPtr<IMFSample> CreateFrameSample(uint32_t frameIndex, LONGLONG sampleTime, std::string& error)` | 创建或初始化 create frame sample 相关逻辑。 |
| [L358](../src/platform/win/MfH264EncoderSelfTest.cpp#L358) | `FillBgraPattern` | 定义 | `void FillBgraPattern(uint32_t frameIndex)` | 实现 fill bgra pattern 对应的业务或工具逻辑。 |
| [L390](../src/platform/win/MfH264EncoderSelfTest.cpp#L390) | `Initialize` | 定义 | `bool Initialize(D3D11Nv12FrameGenerator& frameGenerator, MfH264EncoderSelfTestResult& result, std::string& error)` | 创建或初始化 initialize 相关逻辑。 |
| [L517](../src/platform/win/MfH264EncoderSelfTest.cpp#L517) | `Run` | 定义 | `bool Run(D3D11Nv12FrameGenerator& frameGenerator, MfH264EncoderSelfTestResult& result, std::string& error)` | 执行后台循环或调度 run 相关逻辑。 |
| [L533](../src/platform/win/MfH264EncoderSelfTest.cpp#L533) | `Shutdown` | 定义 | `void Shutdown()` | 关闭并清理 shutdown 相关逻辑。 |
| [L551](../src/platform/win/MfH264EncoderSelfTest.cpp#L551) | `SetCodecApiBoolean` | 定义 | `bool SetCodecApiBoolean(const GUID& key, bool value)` | 更新或应用 set codec api boolean 相关逻辑。 |
| [L563](../src/platform/win/MfH264EncoderSelfTest.cpp#L563) | `SetCodecApiUInt32` | 定义 | `bool SetCodecApiUInt32(const GUID& key, uint32_t value)` | 更新或应用 set codec api u int32 相关逻辑。 |
| [L575](../src/platform/win/MfH264EncoderSelfTest.cpp#L575) | `RequestKeyFrame` | 定义 | `void RequestKeyFrame()` | 发起请求或查询 request key frame 相关逻辑。 |
| [L580](../src/platform/win/MfH264EncoderSelfTest.cpp#L580) | `SubmitFrame` | 定义 | `HRESULT SubmitFrame(D3D11Nv12FrameGenerator& frameGenerator, uint32_t frameIndex, MfH264EncoderSelfTestResult& result, std::string& error)` | 实现 submit frame 对应的业务或工具逻辑。 |
| [L614](../src/platform/win/MfH264EncoderSelfTest.cpp#L614) | `ReadOneOutput` | 定义 | `HRESULT ReadOneOutput(MfH264EncoderSelfTestResult& result, std::string& error)` | 读取或恢复 read one output 相关逻辑。 |
| [L689](../src/platform/win/MfH264EncoderSelfTest.cpp#L689) | `BeginDrain` | 定义 | `bool BeginDrain(std::string& error)` | 启动 begin drain 相关逻辑。 |
| [L708](../src/platform/win/MfH264EncoderSelfTest.cpp#L708) | `RunAsynchronous` | 定义 | `bool RunAsynchronous(D3D11Nv12FrameGenerator& frameGenerator, MfH264EncoderSelfTestResult& result, std::string& error)` | 执行后台循环或调度 run asynchronous 相关逻辑。 |
| [L775](../src/platform/win/MfH264EncoderSelfTest.cpp#L775) | `RunSynchronous` | 定义 | `bool RunSynchronous(D3D11Nv12FrameGenerator& frameGenerator, MfH264EncoderSelfTestResult& result, std::string& error)` | 执行后台循环或调度 run synchronous 相关逻辑。 |
| [L855](../src/platform/win/MfH264EncoderSelfTest.cpp#L855) | `RunMfH264EncoderD3D11SelfTest` | 定义 | `MfH264EncoderSelfTestResult RunMfH264EncoderD3D11SelfTest()` | 执行后台循环或调度 run mf h264 encoder d3 d11 self test 相关逻辑。 |

## `src/platform/win/MfH264EncoderSelfTest.h`

[打开源码](../src/platform/win/MfH264EncoderSelfTest.h) · **文件作用：** 声明 mf h264 encoder self test 相关类型、接口、配置和成员状态。

### 类型

| 行 | 类型 | 种类 | 作用 |
|---:|---|---|---|
| [L11](../src/platform/win/MfH264EncoderSelfTest.h#L11) | `MfH264EncoderSelfTestResult` | struct | 定义 MfH264EncoderSelfTestResult 的 struct 类型和相关状态。 |

### 成员与文件级变量

| 行 | 变量 | 声明 | 作用 |
|---:|---|---|---|
| [L12](../src/platform/win/MfH264EncoderSelfTest.h#L12) | `passed` | `bool passed = false;` | 保存 passed 相关配置或运行状态。 |
| [L13](../src/platform/win/MfH264EncoderSelfTest.h#L13) | `asynchronousTransform` | `bool asynchronousTransform = false;` | 保存 asynchronous transform 相关配置或运行状态。 |
| [L14](../src/platform/win/MfH264EncoderSelfTest.h#L14) | `d3d11PathUsed` | `bool d3d11PathUsed = false;` | 保存 d3d11 path used 相关配置或运行状态。 |
| [L15](../src/platform/win/MfH264EncoderSelfTest.h#L15) | `dynamicBitrateSupported` | `bool dynamicBitrateSupported = false;` | 保存 dynamic bitrate supported 相关配置或运行状态。 |
| [L16](../src/platform/win/MfH264EncoderSelfTest.h#L16) | `dynamicBitrateChanged` | `bool dynamicBitrateChanged = false;` | 保存 dynamic bitrate changed 相关配置或运行状态。 |
| [L17](../src/platform/win/MfH264EncoderSelfTest.h#L17) | `submittedFrames` | `uint32_t submittedFrames = 0;` | 保存 submitted frames 相关配置或运行状态。 |
| [L18](../src/platform/win/MfH264EncoderSelfTest.h#L18) | `encodedSamples` | `uint32_t encodedSamples = 0;` | 保存 encoded samples 相关配置或运行状态。 |
| [L19](../src/platform/win/MfH264EncoderSelfTest.h#L19) | `keyFrames` | `uint32_t keyFrames = 0;` | 保存 key frames 相关配置或运行状态。 |
| [L20](../src/platform/win/MfH264EncoderSelfTest.h#L20) | `encodedBytes` | `uint64_t encodedBytes = 0;` | 保存计数、尺寸或速率指标：encoded bytes。 |
| [L21](../src/platform/win/MfH264EncoderSelfTest.h#L21) | `elapsedMilliseconds` | `double elapsedMilliseconds = 0.0;` | 保存 elapsed milliseconds 相关配置或运行状态。 |
| [L22](../src/platform/win/MfH264EncoderSelfTest.h#L22) | `encoderName` | `std::string encoderName;` | 保存路径、地址或显示名称：encoder name。 |
| [L23](../src/platform/win/MfH264EncoderSelfTest.h#L23) | `report` | `std::string report;` | 保存 report 相关配置或运行状态。 |

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L26](../src/platform/win/MfH264EncoderSelfTest.h#L26) | `RunMfH264EncoderD3D11SelfTest` | 声明 | `MfH264EncoderSelfTestResult RunMfH264EncoderD3D11SelfTest()` | 执行后台循环或调度 run mf h264 encoder d3 d11 self test 相关逻辑。 |

## `src/platform/win/QualityOpenH264Encoder.cpp`

[打开源码](../src/platform/win/QualityOpenH264Encoder.cpp) · **文件作用：** 实现 quality open h264 encoder 相关函数与文件级辅助逻辑。

### 类型

| 行 | 类型 | 种类 | 作用 |
|---:|---|---|---|
| [L56](../src/platform/win/QualityOpenH264Encoder.cpp#L56) | `QualityOpenH264Encoder::Impl` | class | 定义 QualityOpenH264Encoder::Impl 的 class 类型和相关状态。 |

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L27](../src/platform/win/QualityOpenH264Encoder.cpp#L27) | `OpenH264Complexity` | 定义 | `ECOMPLEXITY_MODE OpenH264Complexity(FfmpegX264Preset quality)` | 启动 open h264 complexity 相关逻辑。 |
| [L41](../src/platform/win/QualityOpenH264Encoder.cpp#L41) | `OpenH264ComplexityName` | 定义 | `const char* OpenH264ComplexityName(FfmpegX264Preset quality)` | 启动 open h264 complexity name 相关逻辑。 |
| [L58](../src/platform/win/QualityOpenH264Encoder.cpp#L58) | `Impl` | 定义 | `explicit Impl(FfmpegX264Preset quality) : quality_(quality) {}` | 实现 impl 对应的业务或工具逻辑。 |
| [L59](../src/platform/win/QualityOpenH264Encoder.cpp#L59) | `~Impl` | 定义 | `~Impl() { Release(); }` | 停止相关活动并释放 Impl 实例拥有的资源。 |
| [L61](../src/platform/win/QualityOpenH264Encoder.cpp#L61) | `Init` | 定义 | `int Init(const webrtc::VideoCodec* codecSettings, const webrtc::VideoEncoder::Settings& settings)` | 创建或初始化 init 相关逻辑。 |
| [L133](../src/platform/win/QualityOpenH264Encoder.cpp#L133) | `RegisterCallback` | 定义 | `int RegisterCallback(webrtc::EncodedImageCallback* callback)` | 实现 register callback 对应的业务或工具逻辑。 |
| [L140](../src/platform/win/QualityOpenH264Encoder.cpp#L140) | `Release` | 定义 | `int Release()` | 释放或取消 release 相关逻辑。 |
| [L147](../src/platform/win/QualityOpenH264Encoder.cpp#L147) | `Encode` | 定义 | `int Encode(const webrtc::VideoFrame& frame, const std::vector<webrtc::VideoFrameType>* frameTypes)` | 编码 encode 相关逻辑。 |
| [L250](../src/platform/win/QualityOpenH264Encoder.cpp#L250) | `SetRates` | 定义 | `void SetRates( const webrtc::VideoEncoder::RateControlParameters& parameters)` | 更新或应用 set rates 相关逻辑。 |
| [L275](../src/platform/win/QualityOpenH264Encoder.cpp#L275) | `ComplexityName` | 定义 | `const char* ComplexityName() const` | 实现 complexity name 对应的业务或工具逻辑。 |
| [L281](../src/platform/win/QualityOpenH264Encoder.cpp#L281) | `ReleaseUnlocked` | 定义 | `void ReleaseUnlocked()` | 释放或取消 release unlocked 相关逻辑。 |
| [L306](../src/platform/win/QualityOpenH264Encoder.cpp#L306) | `QualityOpenH264Encoder::QualityOpenH264Encoder` | 定义 | `QualityOpenH264Encoder::QualityOpenH264Encoder(FfmpegX264Preset quality) : impl_(std::make_unique<Impl>(quality)) {}` | 构造并初始化 QualityOpenH264Encoder 实例。 |
| [L312](../src/platform/win/QualityOpenH264Encoder.cpp#L312) | `QualityOpenH264Encoder::InitEncode` | 定义 | `int QualityOpenH264Encoder::InitEncode( const webrtc::VideoCodec* codecSettings, const Settings& settings)` | 创建或初始化 init encode 相关逻辑。 |
| [L318](../src/platform/win/QualityOpenH264Encoder.cpp#L318) | `QualityOpenH264Encoder::RegisterEncodeCompleteCallback` | 定义 | `int32_t QualityOpenH264Encoder::RegisterEncodeCompleteCallback( webrtc::EncodedImageCallback* callback)` | 实现 register encode complete callback 对应的业务或工具逻辑。 |
| [L324](../src/platform/win/QualityOpenH264Encoder.cpp#L324) | `QualityOpenH264Encoder::Release` | 定义 | `int32_t QualityOpenH264Encoder::Release()` | 释放或取消 release 相关逻辑。 |
| [L329](../src/platform/win/QualityOpenH264Encoder.cpp#L329) | `QualityOpenH264Encoder::Encode` | 定义 | `int32_t QualityOpenH264Encoder::Encode( const webrtc::VideoFrame& frame, const std::vector<webrtc::VideoFrameType>* frameTypes)` | 编码 encode 相关逻辑。 |
| [L336](../src/platform/win/QualityOpenH264Encoder.cpp#L336) | `QualityOpenH264Encoder::SetRates` | 定义 | `void QualityOpenH264Encoder::SetRates( const RateControlParameters& parameters)` | 更新或应用 set rates 相关逻辑。 |
| [L343](../src/platform/win/QualityOpenH264Encoder.cpp#L343) | `QualityOpenH264Encoder::GetEncoderInfo` | 定义 | `QualityOpenH264Encoder::GetEncoderInfo() const` | 查询并返回 get encoder info 相关逻辑。 |
| [L362](../src/platform/win/QualityOpenH264Encoder.cpp#L362) | `QualityOpenH264EncoderFactory::GetSupportedFormats` | 定义 | `QualityOpenH264EncoderFactory::GetSupportedFormats() const` | 查询并返回 get supported formats 相关逻辑。 |
| [L368](../src/platform/win/QualityOpenH264Encoder.cpp#L368) | `QualityOpenH264EncoderFactory::QueryCodecSupport` | 定义 | `QualityOpenH264EncoderFactory::QueryCodecSupport( const webrtc::SdpVideoFormat& format, std::optional<std::string> scalabilityMode, std::optional<webrtc::Resolution> /* resolution */) const` | 发起请求或查询 query codec support 相关逻辑。 |
| [L381](../src/platform/win/QualityOpenH264Encoder.cpp#L381) | `QualityOpenH264EncoderFactory::Create` | 定义 | `QualityOpenH264EncoderFactory::Create( const webrtc::Environment& /* environment */, const webrtc::SdpVideoFormat& format)` | 创建或初始化 create 相关逻辑。 |

## `src/platform/win/QualityOpenH264Encoder.h`

[打开源码](../src/platform/win/QualityOpenH264Encoder.h) · **文件作用：** 声明 quality open h264 encoder 相关类型、接口、配置和成员状态。

### 类型

| 行 | 类型 | 种类 | 作用 |
|---:|---|---|---|
| [L17](../src/platform/win/QualityOpenH264Encoder.h#L17) | `QualityOpenH264Encoder` | class | Single-stream OpenH264 encoder used by RemoteC desktop sharing. WebRTC's built-in wrapper does not expose OpenH264's iComplexityMode, so this small adapter keeps WebRTC rate con... |
| [L35](../src/platform/win/QualityOpenH264Encoder.h#L35) | `Impl` | class | 定义 Impl 的 class 类型和相关状态。 |
| [L39](../src/platform/win/QualityOpenH264Encoder.h#L39) | `QualityOpenH264EncoderFactory` | class | 定义 QualityOpenH264EncoderFactory 的 class 类型和相关状态。 |

### 成员与文件级变量

| 行 | 变量 | 声明 | 作用 |
|---:|---|---|---|
| [L35](../src/platform/win/QualityOpenH264Encoder.h#L35) | `Impl` | `class Impl;` | 保存 impl 相关配置或运行状态。 |
| [L36](../src/platform/win/QualityOpenH264Encoder.h#L36) | `impl_` | `std::unique_ptr<Impl> impl_;` | 保存 impl 相关配置或运行状态。 |
| [L57](../src/platform/win/QualityOpenH264Encoder.h#L57) | `quality_` | `FfmpegX264Preset quality_ = FfmpegX264Preset::kMedium;` | 保存 quality 相关配置或运行状态。 |

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L19](../src/platform/win/QualityOpenH264Encoder.h#L19) | `QualityOpenH264Encoder` | 声明 | `explicit QualityOpenH264Encoder( FfmpegX264Preset quality = FfmpegX264Preset::kMedium)` | 实现 quality open h264 encoder 对应的业务或工具逻辑。 |
| [L21](../src/platform/win/QualityOpenH264Encoder.h#L21) | `~QualityOpenH264Encoder` | 声明 | `~QualityOpenH264Encoder() override` | 停止相关活动并释放 QualityOpenH264Encoder 实例拥有的资源。 |
| [L23](../src/platform/win/QualityOpenH264Encoder.h#L23) | `InitEncode` | 声明 | `int InitEncode(const webrtc::VideoCodec* codecSettings, const Settings& settings) override` | 创建或初始化 init encode 相关逻辑。 |
| [L25](../src/platform/win/QualityOpenH264Encoder.h#L25) | `RegisterEncodeCompleteCallback` | 声明 | `int32_t RegisterEncodeCompleteCallback( webrtc::EncodedImageCallback* callback) override` | 实现 register encode complete callback 对应的业务或工具逻辑。 |
| [L27](../src/platform/win/QualityOpenH264Encoder.h#L27) | `Release` | 声明 | `int32_t Release() override` | 释放或取消 release 相关逻辑。 |
| [L28](../src/platform/win/QualityOpenH264Encoder.h#L28) | `Encode` | 声明 | `int32_t Encode( const webrtc::VideoFrame& frame, const std::vector<webrtc::VideoFrameType>* frameTypes) override` | 编码 encode 相关逻辑。 |
| [L31](../src/platform/win/QualityOpenH264Encoder.h#L31) | `SetRates` | 声明 | `void SetRates(const RateControlParameters& parameters) override` | 更新或应用 set rates 相关逻辑。 |
| [L32](../src/platform/win/QualityOpenH264Encoder.h#L32) | `GetEncoderInfo` | 声明 | `EncoderInfo GetEncoderInfo() const override` | 查询并返回 get encoder info 相关逻辑。 |
| [L42](../src/platform/win/QualityOpenH264Encoder.h#L42) | `QualityOpenH264EncoderFactory` | 定义 | `explicit QualityOpenH264EncoderFactory( FfmpegX264Preset quality = FfmpegX264Preset::kMedium) : quality_(quality) {}` | 实现 quality open h264 encoder factory 对应的业务或工具逻辑。 |
| [L47](../src/platform/win/QualityOpenH264Encoder.h#L47) | `GetSupportedFormats` | 声明 | `std::vector<webrtc::SdpVideoFormat> GetSupportedFormats() const override` | 查询并返回 get supported formats 相关逻辑。 |
| [L48](../src/platform/win/QualityOpenH264Encoder.h#L48) | `QueryCodecSupport` | 声明 | `CodecSupport QueryCodecSupport( const webrtc::SdpVideoFormat& format, std::optional<std::string> scalabilityMode, std::optional<webrtc::Resolution> resolution) const override` | 发起请求或查询 query codec support 相关逻辑。 |
| [L52](../src/platform/win/QualityOpenH264Encoder.h#L52) | `Create` | 声明 | `std::unique_ptr<webrtc::VideoEncoder> Create( const webrtc::Environment& environment, const webrtc::SdpVideoFormat& format) override` | 创建或初始化 create 相关逻辑。 |

## `src/platform/win/VideoDecoderProbePolicy.h`

[打开源码](../src/platform/win/VideoDecoderProbePolicy.h) · **文件作用：** 声明 video decoder probe policy 相关类型、接口、配置和成员状态。

### 成员与文件级变量

| 行 | 变量 | 声明 | 作用 |
|---:|---|---|---|
| [L12](../src/platform/win/VideoDecoderProbePolicy.h#L12) | `kVideoDecoderProbePolicyVersion` | `inline constexpr int kVideoDecoderProbePolicyVersion = 6;` | Bump whenever the candidate set or the latency acceptance policy changes. Version 6 evaluates the real 1920x1080@60 desktop workload, including its sparse-frame phase. A cached ... |

## `src/platform/win/VideoEncoderProbePolicy.h`

[打开源码](../src/platform/win/VideoEncoderProbePolicy.h) · **文件作用：** 声明 video encoder probe policy 相关类型、接口、配置和成员状态。

### 成员与文件级变量

| 行 | 变量 | 声明 | 作用 |
|---:|---|---|---|
| [L14](../src/platform/win/VideoEncoderProbePolicy.h#L14) | `kVideoEncoderProbePolicyVersion` | `inline constexpr int kVideoEncoderProbePolicyVersion = 10;` | reused across policy versions. Version 10 treats any candidate that produces a decodable H264 stream as usable, ranks usable candidates by measured output FPS and then cost, and... |

## `src/platform/win/WindowsCameraCaptureSource.cpp`

[打开源码](../src/platform/win/WindowsCameraCaptureSource.cpp) · **文件作用：** 实现 windows camera capture source 相关函数与文件级辅助逻辑。

### 成员与文件级变量

| 行 | 变量 | 声明 | 作用 |
|---:|---|---|---|
| [L17](../src/platform/win/WindowsCameraCaptureSource.cpp#L17) | `kDeviceTextCapacity` | `constexpr std::size_t kDeviceTextCapacity = 512;` | 定义 device text capacity 的编译期常量或产品边界。 |

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L23](../src/platform/win/WindowsCameraCaptureSource.cpp#L23) | `WindowsCameraCaptureSource::~WindowsCameraCaptureSource` | 定义 | `WindowsCameraCaptureSource::~WindowsCameraCaptureSource()` | 停止相关活动并释放 WindowsCameraCaptureSource 实例拥有的资源。 |
| [L29](../src/platform/win/WindowsCameraCaptureSource.cpp#L29) | `WindowsCameraCaptureSource::EnumerateDevices` | 定义 | `WindowsCameraCaptureSource::EnumerateDevices()` | 实现 enumerate devices 对应的业务或工具逻辑。 |
| [L53](../src/platform/win/WindowsCameraCaptureSource.cpp#L53) | `WindowsCameraCaptureSource::StartCapture` | 定义 | `bool WindowsCameraCaptureSource::StartCapture( const std::string& preferredDeviceId, std::chrono::milliseconds firstFrameTimeout)` | 启动 start capture 相关逻辑。 |
| [L135](../src/platform/win/WindowsCameraCaptureSource.cpp#L135) | `WindowsCameraCaptureSource::StopCapture` | 定义 | `void WindowsCameraCaptureSource::StopCapture()` | 停止 stop capture 相关逻辑。 |
| [L154](../src/platform/win/WindowsCameraCaptureSource.cpp#L154) | `WindowsCameraCaptureSource::ActiveDeviceId` | 定义 | `std::string WindowsCameraCaptureSource::ActiveDeviceId() const` | 实现 active device id 对应的业务或工具逻辑。 |
| [L160](../src/platform/win/WindowsCameraCaptureSource.cpp#L160) | `WindowsCameraCaptureSource::ActiveDeviceName` | 定义 | `std::string WindowsCameraCaptureSource::ActiveDeviceName() const` | 实现 active device name 对应的业务或工具逻辑。 |
| [L166](../src/platform/win/WindowsCameraCaptureSource.cpp#L166) | `WindowsCameraCaptureSource::LastError` | 定义 | `std::string WindowsCameraCaptureSource::LastError() const` | 实现 last error 对应的业务或工具逻辑。 |
| [L173](../src/platform/win/WindowsCameraCaptureSource.cpp#L173) | `WindowsCameraCaptureSource::state` | 定义 | `WindowsCameraCaptureSource::state() const` | 实现 state 对应的业务或工具逻辑。 |
| [L179](../src/platform/win/WindowsCameraCaptureSource.cpp#L179) | `WindowsCameraCaptureSource::remote` | 定义 | `bool WindowsCameraCaptureSource::remote() const` | 实现 remote 对应的业务或工具逻辑。 |
| [L184](../src/platform/win/WindowsCameraCaptureSource.cpp#L184) | `WindowsCameraCaptureSource::is_screencast` | 定义 | `bool WindowsCameraCaptureSource::is_screencast() const` | 判断 is screencast 相关逻辑。 |
| [L189](../src/platform/win/WindowsCameraCaptureSource.cpp#L189) | `WindowsCameraCaptureSource::needs_denoising` | 定义 | `std::optional<bool> WindowsCameraCaptureSource::needs_denoising() const` | 判断 needs denoising 相关逻辑。 |
| [L194](../src/platform/win/WindowsCameraCaptureSource.cpp#L194) | `WindowsCameraCaptureSource::OnFrame` | 定义 | `void WindowsCameraCaptureSource::OnFrame(const webrtc::VideoFrame& frame)` | 接收并处理 on frame 相关逻辑。 |

## `src/platform/win/WindowsCameraCaptureSource.h`

[打开源码](../src/platform/win/WindowsCameraCaptureSource.h) · **文件作用：** 声明 windows camera capture source 相关类型、接口、配置和成员状态。

### 类型

| 行 | 类型 | 种类 | 作用 |
|---:|---|---|---|
| [L19](../src/platform/win/WindowsCameraCaptureSource.h#L19) | `CameraDeviceInfo` | struct | 定义 CameraDeviceInfo 的 struct 类型和相关状态。 |
| [L24](../src/platform/win/WindowsCameraCaptureSource.h#L24) | `WindowsCameraCaptureSource` | class | 定义 WindowsCameraCaptureSource 的 class 类型和相关状态。 |

### 成员与文件级变量

| 行 | 变量 | 声明 | 作用 |
|---:|---|---|---|
| [L20](../src/platform/win/WindowsCameraCaptureSource.h#L20) | `id` | `std::string id;` | 保存身份或作用域标识：id。 |
| [L21](../src/platform/win/WindowsCameraCaptureSource.h#L21) | `name` | `std::string name;` | 保存路径、地址或显示名称：name。 |
| [L50](../src/platform/win/WindowsCameraCaptureSource.h#L50) | `mutex_` | `mutable std::mutex mutex_;` | 保护跨线程共享状态：mutex。 |
| [L51](../src/platform/win/WindowsCameraCaptureSource.h#L51) | `firstFrameCondition_` | `std::condition_variable firstFrameCondition_;` | 保存 first frame condition 相关配置或运行状态。 |
| [L52](../src/platform/win/WindowsCameraCaptureSource.h#L52) | `captureModule_` | `webrtc::scoped_refptr<webrtc::VideoCaptureModule> captureModule_;` | 保存 capture module 相关配置或运行状态。 |
| [L53](../src/platform/win/WindowsCameraCaptureSource.h#L53) | `activeDeviceId_` | `std::string activeDeviceId_;` | 保存身份或作用域标识：active device id。 |
| [L54](../src/platform/win/WindowsCameraCaptureSource.h#L54) | `activeDeviceName_` | `std::string activeDeviceName_;` | 保存路径、地址或显示名称：active device name。 |
| [L55](../src/platform/win/WindowsCameraCaptureSource.h#L55) | `lastError_` | `std::string lastError_;` | 保存最近错误或失败原因：last error。 |
| [L56](../src/platform/win/WindowsCameraCaptureSource.h#L56) | `running_` | `bool running_ = false;` | 保存 running 相关配置或运行状态。 |
| [L57](../src/platform/win/WindowsCameraCaptureSource.h#L57) | `firstFrameReady_` | `bool firstFrameReady_ = false;` | 保存能力或开关状态：first frame ready。 |

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L28](../src/platform/win/WindowsCameraCaptureSource.h#L28) | `WindowsCameraCaptureSource` | 声明 | `WindowsCameraCaptureSource()` | 实现 windows camera capture source 对应的业务或工具逻辑。 |
| [L29](../src/platform/win/WindowsCameraCaptureSource.h#L29) | `~WindowsCameraCaptureSource` | 声明 | `~WindowsCameraCaptureSource() override` | 停止相关活动并释放 WindowsCameraCaptureSource 实例拥有的资源。 |
| [L31](../src/platform/win/WindowsCameraCaptureSource.h#L31) | `EnumerateDevices` | 声明 | `static std::vector<CameraDeviceInfo> EnumerateDevices()` | 实现 enumerate devices 对应的业务或工具逻辑。 |
| [L37](../src/platform/win/WindowsCameraCaptureSource.h#L37) | `StopCapture` | 声明 | `void StopCapture()` | 停止 stop capture 相关逻辑。 |
| [L39](../src/platform/win/WindowsCameraCaptureSource.h#L39) | `ActiveDeviceId` | 声明 | `std::string ActiveDeviceId() const` | 实现 active device id 对应的业务或工具逻辑。 |
| [L40](../src/platform/win/WindowsCameraCaptureSource.h#L40) | `ActiveDeviceName` | 声明 | `std::string ActiveDeviceName() const` | 实现 active device name 对应的业务或工具逻辑。 |
| [L41](../src/platform/win/WindowsCameraCaptureSource.h#L41) | `LastError` | 声明 | `std::string LastError() const` | 实现 last error 对应的业务或工具逻辑。 |
| [L43](../src/platform/win/WindowsCameraCaptureSource.h#L43) | `state` | 声明 | `SourceState state() const override` | 实现 state 对应的业务或工具逻辑。 |
| [L44](../src/platform/win/WindowsCameraCaptureSource.h#L44) | `remote` | 声明 | `bool remote() const override` | 实现 remote 对应的业务或工具逻辑。 |
| [L45](../src/platform/win/WindowsCameraCaptureSource.h#L45) | `is_screencast` | 声明 | `bool is_screencast() const override` | 判断 is screencast 相关逻辑。 |
| [L46](../src/platform/win/WindowsCameraCaptureSource.h#L46) | `needs_denoising` | 声明 | `std::optional<bool> needs_denoising() const override` | 判断 needs denoising 相关逻辑。 |
| [L47](../src/platform/win/WindowsCameraCaptureSource.h#L47) | `OnFrame` | 声明 | `void OnFrame(const webrtc::VideoFrame& frame) override` | 接收并处理 on frame 相关逻辑。 |

## `src/platform/win/WindowsClipboardService.cpp`

[打开源码](../src/platform/win/WindowsClipboardService.cpp) · **文件作用：** 实现 windows clipboard service 相关函数与文件级辅助逻辑。

### 类型

| 行 | 类型 | 种类 | 作用 |
|---:|---|---|---|
| [L423](../src/platform/win/WindowsClipboardService.cpp#L423) | `WindowsClipboardService::Impl` | struct | 定义 WindowsClipboardService::Impl 的 struct 类型和相关状态。 |
| [L555](../src/platform/win/WindowsClipboardService.cpp#L555) | `PendingWrite` | struct | 定义 PendingWrite 的 struct 类型和相关状态。 |
| [L560](../src/platform/win/WindowsClipboardService.cpp#L560) | `PendingAdvertisement` | struct | 定义 PendingAdvertisement 的 struct 类型和相关状态。 |

### 成员与文件级变量

| 行 | 变量 | 声明 | 作用 |
|---:|---|---|---|
| [L31](../src/platform/win/WindowsClipboardService.cpp#L31) | `kWriteMessage` | `constexpr UINT kWriteMessage = WM_APP + 0x341;` | 定义 write message 的编译期常量或产品边界。 |
| [L32](../src/platform/win/WindowsClipboardService.cpp#L32) | `kCaptureMessage` | `constexpr UINT kCaptureMessage = WM_APP + 0x342;` | 定义 capture message 的编译期常量或产品边界。 |
| [L33](../src/platform/win/WindowsClipboardService.cpp#L33) | `kAdvertiseMessage` | `constexpr UINT kAdvertiseMessage = WM_APP + 0x343;` | 定义 advertise message 的编译期常量或产品边界。 |
| [L34](../src/platform/win/WindowsClipboardService.cpp#L34) | `kClearAdvertisementMessage` | `constexpr UINT kClearAdvertisementMessage = WM_APP + 0x344;` | 定义 clear advertisement message 的编译期常量或产品边界。 |
| [L35](../src/platform/win/WindowsClipboardService.cpp#L35) | `kWindowClassName` | `constexpr wchar_t kWindowClassName[] = L"RemoteCClipboardListenerV2";` | 定义 window class name 的编译期常量或产品边界。 |
| [L36](../src/platform/win/WindowsClipboardService.cpp#L36) | `kOriginFormatName` | `constexpr wchar_t kOriginFormatName[] = L"RemoteC.ClipboardOrigin.v2";` | 定义 origin format name 的编译期常量或产品边界。 |
| [L37](../src/platform/win/WindowsClipboardService.cpp#L37) | `kPngFormatName` | `constexpr wchar_t kPngFormatName[] = L"PNG";` | 定义 png format name 的编译期常量或产品边界。 |
| [L38](../src/platform/win/WindowsClipboardService.cpp#L38) | `kHtmlFormatName` | `constexpr wchar_t kHtmlFormatName[] = L"HTML Format";` | 定义 html format name 的编译期常量或产品边界。 |
| [L39](../src/platform/win/WindowsClipboardService.cpp#L39) | `kRtfFormatName` | `constexpr wchar_t kRtfFormatName[] = L"Rich Text Format";` | 定义 rtf format name 的编译期常量或产品边界。 |
| [L426](../src/platform/win/WindowsClipboardService.cpp#L426) | `hookOwner` | `static inline Impl* hookOwner = nullptr;` | 保存 hook owner 相关配置或运行状态。 |

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L41](../src/platform/win/WindowsClipboardService.cpp#L41) | `WideToUtf8` | 定义 | `std::string WideToUtf8(std::wstring_view text)` | 实现 wide to utf8 对应的业务或工具逻辑。 |
| [L62](../src/platform/win/WindowsClipboardService.cpp#L62) | `Utf8ToWide` | 定义 | `std::wstring Utf8ToWide(std::string_view text)` | 实现 utf8 to wide 对应的业务或工具逻辑。 |
| [L82](../src/platform/win/WindowsClipboardService.cpp#L82) | `CopyGlobalBytes` | 定义 | `bool CopyGlobalBytes(UINT format, std::vector<std::uint8_t>* output)` | 实现 copy global bytes 对应的业务或工具逻辑。 |
| [L108](../src/platform/win/WindowsClipboardService.cpp#L108) | `PutGlobalBytes` | 定义 | `bool PutGlobalBytes(UINT format, const void* bytes, std::size_t size, bool appendNull)` | 实现 put global bytes 对应的业务或工具逻辑。 |
| [L135](../src/platform/win/WindowsClipboardService.cpp#L135) | `PutFileDrop` | 定义 | `bool PutFileDrop(const std::vector<std::wstring>& files)` | 实现 put file drop 对应的业务或工具逻辑。 |
| [L166](../src/platform/win/WindowsClipboardService.cpp#L166) | `StreamBytes` | 定义 | `bool StreamBytes(IStream* stream, std::vector<std::uint8_t>* output)` | 实现 stream bytes 对应的业务或工具逻辑。 |
| [L189](../src/platform/win/WindowsClipboardService.cpp#L189) | `EncodeDibToPng` | 定义 | `bool EncodeDibToPng(const void* dibMemory, std::size_t dibSize, std::vector<std::uint8_t>* png)` | 编码 encode dib to png 相关逻辑。 |
| [L283](../src/platform/win/WindowsClipboardService.cpp#L283) | `DecodePngToDib` | 定义 | `bool DecodePngToDib(const std::vector<std::uint8_t>& png, std::vector<std::uint8_t>* dib)` | 解码或解析 decode png to dib 相关逻辑。 |
| [L354](../src/platform/win/WindowsClipboardService.cpp#L354) | `OpenClipboardWithRetry` | 定义 | `bool OpenClipboardWithRetry(HWND owner)` | 启动 open clipboard with retry 相关逻辑。 |
| [L366](../src/platform/win/WindowsClipboardService.cpp#L366) | `AppendShellFileSystemItems` | 定义 | `void AppendShellFileSystemItems(WindowsClipboardContent* content)` | 实现 append shell file system items 对应的业务或工具逻辑。 |
| [L424](../src/platform/win/WindowsClipboardService.cpp#L424) | `Impl` | 定义 | `explicit Impl(WindowsClipboardService*) {}` | 实现 impl 对应的业务或工具逻辑。 |
| [L428](../src/platform/win/WindowsClipboardService.cpp#L428) | `LowLevelKeyboardProc` | 定义 | `static LRESULT CALLBACK LowLevelKeyboardProc( int code, WPARAM wparam, LPARAM lparam)` | 实现 low level keyboard proc 对应的业务或工具逻辑。 |
| [L479](../src/platform/win/WindowsClipboardService.cpp#L479) | `LowLevelMouseProc` | 定义 | `static LRESULT CALLBACK LowLevelMouseProc( int code, WPARAM wparam, LPARAM lparam)` | 实现 low level mouse proc 对应的业务或工具逻辑。 |
| [L529](../src/platform/win/WindowsClipboardService.cpp#L529) | `ConsumeContextPasteIntent` | 定义 | `bool ConsumeContextPasteIntent(HWND foreground)` | 实现 consume context paste intent 对应的业务或工具逻辑。 |
| [L566](../src/platform/win/WindowsClipboardService.cpp#L566) | `WindowProc` | 定义 | `static LRESULT CALLBACK WindowProc(HWND window, UINT message, WPARAM wparam, LPARAM lparam)` | 实现 window proc 对应的业务或工具逻辑。 |
| [L613](../src/platform/win/WindowsClipboardService.cpp#L613) | `ThreadMain` | 定义 | `void ThreadMain(std::stop_token stopToken)` | 实现 thread main 对应的业务或工具逻辑。 |
| [L652](../src/platform/win/WindowsClipboardService.cpp#L652) | `KeyboardThreadMain` | 定义 | `void KeyboardThreadMain(std::stop_token stopToken)` | 实现 keyboard thread main 对应的业务或工具逻辑。 |
| [L684](../src/platform/win/WindowsClipboardService.cpp#L684) | `Capture` | 定义 | `void Capture(bool forced)` | 采集 capture 相关逻辑。 |
| [L803](../src/platform/win/WindowsClipboardService.cpp#L803) | `Apply` | 定义 | `bool Apply(const WindowsClipboardContent& content)` | 更新或应用 apply 相关逻辑。 |
| [L870](../src/platform/win/WindowsClipboardService.cpp#L870) | `RenderFormat` | 定义 | `bool RenderFormat(const WindowsClipboardContent& content, UINT format)` | 准备或呈现 render format 相关逻辑。 |
| [L935](../src/platform/win/WindowsClipboardService.cpp#L935) | `ApplyPendingAdvertisements` | 定义 | `void ApplyPendingAdvertisements()` | 更新或应用 apply pending advertisements 相关逻辑。 |
| [L989](../src/platform/win/WindowsClipboardService.cpp#L989) | `ClearRemoteAdvertisement` | 定义 | `void ClearRemoteAdvertisement()` | 重置或移除 clear remote advertisement 相关逻辑。 |
| [L1007](../src/platform/win/WindowsClipboardService.cpp#L1007) | `RenderRemoteFormat` | 定义 | `void RenderRemoteFormat(UINT format)` | 准备或呈现 render remote format 相关逻辑。 |
| [L1050](../src/platform/win/WindowsClipboardService.cpp#L1050) | `ApplyPendingWrites` | 定义 | `void ApplyPendingWrites()` | 更新或应用 apply pending writes 相关逻辑。 |
| [L1086](../src/platform/win/WindowsClipboardService.cpp#L1086) | `WindowsClipboardService::WindowsClipboardService` | 定义 | `WindowsClipboardService::WindowsClipboardService() : impl_(std::make_unique<Impl>(this)) {}` | 构造并初始化 WindowsClipboardService 实例。 |
| [L1090](../src/platform/win/WindowsClipboardService.cpp#L1090) | `WindowsClipboardService::~WindowsClipboardService` | 定义 | `WindowsClipboardService::~WindowsClipboardService()` | 停止相关活动并释放 WindowsClipboardService 实例拥有的资源。 |
| [L1095](../src/platform/win/WindowsClipboardService.cpp#L1095) | `WindowsClipboardService::Start` | 定义 | `bool WindowsClipboardService::Start(ChangeCallback callback, PasteCallback pasteCallback, std::string* error)` | 启动 start 相关逻辑。 |
| [L1129](../src/platform/win/WindowsClipboardService.cpp#L1129) | `WindowsClipboardService::Stop` | 定义 | `void WindowsClipboardService::Stop()` | 停止 stop 相关逻辑。 |
| [L1167](../src/platform/win/WindowsClipboardService.cpp#L1167) | `WindowsClipboardService::SetEnabled` | 定义 | `void WindowsClipboardService::SetEnabled(bool enabled)` | 更新或应用 set enabled 相关逻辑。 |
| [L1172](../src/platform/win/WindowsClipboardService.cpp#L1172) | `WindowsClipboardService::RequestCurrent` | 定义 | `void WindowsClipboardService::RequestCurrent()` | 发起请求或查询 request current 相关逻辑。 |
| [L1184](../src/platform/win/WindowsClipboardService.cpp#L1184) | `WindowsClipboardService::AdvertiseRemoteClipboard` | 定义 | `std::uint32_t WindowsClipboardService::AdvertiseRemoteClipboard( std::uint32_t capabilities, const std::string& originMarker)` | 实现 advertise remote clipboard 对应的业务或工具逻辑。 |
| [L1219](../src/platform/win/WindowsClipboardService.cpp#L1219) | `WindowsClipboardService::CancelPendingRemoteRender` | 定义 | `void WindowsClipboardService::CancelPendingRemoteRender()` | 判断 cancel pending remote render 相关逻辑。 |
| [L1230](../src/platform/win/WindowsClipboardService.cpp#L1230) | `WindowsClipboardService::ClearRemoteClipboardAdvertisement` | 定义 | `void WindowsClipboardService::ClearRemoteClipboardAdvertisement()` | 重置或移除 clear remote clipboard advertisement 相关逻辑。 |
| [L1243](../src/platform/win/WindowsClipboardService.cpp#L1243) | `WindowsClipboardService::Write` | 定义 | `bool WindowsClipboardService::Write(WindowsClipboardContent content, std::string* error)` | 保存或写入 write 相关逻辑。 |

## `src/platform/win/WindowsClipboardService.h`

[打开源码](../src/platform/win/WindowsClipboardService.h) · **文件作用：** 声明 windows clipboard service 相关类型、接口、配置和成员状态。

### 类型

| 行 | 类型 | 种类 | 作用 |
|---:|---|---|---|
| [L19](../src/platform/win/WindowsClipboardService.h#L19) | `WindowsClipboardPart` | struct | 定义 WindowsClipboardPart 的 struct 类型和相关状态。 |
| [L26](../src/platform/win/WindowsClipboardService.h#L26) | `WindowsClipboardContent` | struct | 定义 WindowsClipboardContent 的 struct 类型和相关状态。 |
| [L42](../src/platform/win/WindowsClipboardService.h#L42) | `WindowsPasteTarget` | struct | 定义 WindowsPasteTarget 的 struct 类型和相关状态。 |
| [L60](../src/platform/win/WindowsClipboardService.h#L60) | `WindowsClipboardService` | class | Owns a message-only HWND and is the only component that touches the Win32 clipboard. Callbacks execute on its message thread and must return quickly. |
| [L88](../src/platform/win/WindowsClipboardService.h#L88) | `Impl` | struct | 定义 Impl 的 struct 类型和相关状态。 |

### 成员与文件级变量

| 行 | 变量 | 声明 | 作用 |
|---:|---|---|---|
| [L20](../src/platform/win/WindowsClipboardService.h#L20) | `format` | `ClipboardFormat format = ClipboardFormat::kUnicodeText;` | 保存 format 相关配置或运行状态。 |
| [L21](../src/platform/win/WindowsClipboardService.h#L21) | `name` | `std::string name;` | 保存路径、地址或显示名称：name。 |
| [L22](../src/platform/win/WindowsClipboardService.h#L22) | `data` | `std::vector<std::uint8_t> data;` | 保存 data 相关配置或运行状态。 |
| [L23](../src/platform/win/WindowsClipboardService.h#L23) | `localPath` | `std::filesystem::path localPath;` | 保存路径、地址或显示名称：local path。 |
| [L27](../src/platform/win/WindowsClipboardService.h#L27) | `parts` | `std::vector<WindowsClipboardPart> parts;` | 保存 parts 相关配置或运行状态。 |
| [L28](../src/platform/win/WindowsClipboardService.h#L28) | `originMarker` | `std::string originMarker;` | 保存 origin marker 相关配置或运行状态。 |
| [L29](../src/platform/win/WindowsClipboardService.h#L29) | `windowsSequence` | `std::uint32_t windowsSequence = 0;` | 保存单调序号，用于排序或去重：windows sequence。 |
| [L33](../src/platform/win/WindowsClipboardService.h#L33) | `explicitRequest` | `bool explicitRequest = false;` | True only for an explicit RequestCurrent() capture. Ordinary WM_CLIPBOARDUPDATE notifications stay false, allowing the controller to distinguish a requested source read from a n... |
| [L37](../src/platform/win/WindowsClipboardService.h#L37) | `localChangeGeneration` | `std::uint64_t localChangeGeneration = 0;` | Assigned by ClipboardController when this capture enters its worker. A newer ordinary user copy increments the controller generation, so a long file hash can stop before the new... |
| [L38](../src/platform/win/WindowsClipboardService.h#L38) | `empty` | `bool empty = false;` | 保存 empty 相关配置或运行状态。 |
| [L39](../src/platform/win/WindowsClipboardService.h#L39) | `captureError` | `std::string captureError;` | 保存最近错误或失败原因：capture error。 |
| [L43](../src/platform/win/WindowsClipboardService.h#L43) | `foregroundWindow` | `std::uintptr_t foregroundWindow = 0;` | 保存 foreground window 相关配置或运行状态。 |
| [L44](../src/platform/win/WindowsClipboardService.h#L44) | `focusedWindow` | `std::uintptr_t focusedWindow = 0;` | 保存 focused window 相关配置或运行状态。 |
| [L48](../src/platform/win/WindowsClipboardService.h#L48) | `injectPasteAfterMaterialization` | `bool injectPasteAfterMaterialization = true;` | Keyboard Ctrl+V is swallowed while the remote item is materialized and therefore needs one replacement injection. A delayed-render request made by an application's Paste command... |
| [L51](../src/platform/win/WindowsClipboardService.h#L51) | `explorerDestinationDirectory` | `std::filesystem::path explorerDestinationDirectory;` | Resolved by ClipboardController for Explorer targets. Empty for other applications and for virtual Shell folders without a filesystem path. |
| [L55](../src/platform/win/WindowsClipboardService.h#L55) | `replaceExplorerConflicts` | `bool replaceExplorerConflicts = false;` | Set only after the user explicitly confirms an Explorer name collision. Direct-copy mode then merges directories and replaces conflicting files instead of silently inventing a n... |
| [L88](../src/platform/win/WindowsClipboardService.h#L88) | `Impl` | `struct Impl;` | 保存 impl 相关配置或运行状态。 |
| [L89](../src/platform/win/WindowsClipboardService.h#L89) | `impl_` | `std::unique_ptr<Impl> impl_;` | 保存 impl 相关配置或运行状态。 |

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L67](../src/platform/win/WindowsClipboardService.h#L67) | `WindowsClipboardService` | 声明 | `WindowsClipboardService()` | 实现 windows clipboard service 对应的业务或工具逻辑。 |
| [L68](../src/platform/win/WindowsClipboardService.h#L68) | `~WindowsClipboardService` | 声明 | `~WindowsClipboardService()` | 停止相关活动并释放 WindowsClipboardService 实例拥有的资源。 |
| [L70](../src/platform/win/WindowsClipboardService.h#L70) | `WindowsClipboardService` | 声明 | `WindowsClipboardService(const WindowsClipboardService&) = delete` | 实现 windows clipboard service 对应的业务或工具逻辑。 |
| [L76](../src/platform/win/WindowsClipboardService.h#L76) | `Stop` | 声明 | `void Stop()` | 停止 stop 相关逻辑。 |
| [L77](../src/platform/win/WindowsClipboardService.h#L77) | `SetEnabled` | 声明 | `void SetEnabled(bool enabled)` | 更新或应用 set enabled 相关逻辑。 |
| [L78](../src/platform/win/WindowsClipboardService.h#L78) | `RequestCurrent` | 声明 | `void RequestCurrent()` | 发起请求或查询 request current 相关逻辑。 |
| [L79](../src/platform/win/WindowsClipboardService.h#L79) | `AdvertiseRemoteClipboard` | 声明 | `std::uint32_t AdvertiseRemoteClipboard( std::uint32_t capabilities, const std::string& originMarker)` | 实现 advertise remote clipboard 对应的业务或工具逻辑。 |
| [L81](../src/platform/win/WindowsClipboardService.h#L81) | `CancelPendingRemoteRender` | 声明 | `void CancelPendingRemoteRender()` | 判断 cancel pending remote render 相关逻辑。 |
| [L84](../src/platform/win/WindowsClipboardService.h#L84) | `ClearRemoteClipboardAdvertisement` | 声明 | `void ClearRemoteClipboardAdvertisement()` | Removes only the delayed RemoteC advertisement that is still the current Windows clipboard item. A newer local user copy is preserved. |
| [L85](../src/platform/win/WindowsClipboardService.h#L85) | `Write` | 声明 | `bool Write(WindowsClipboardContent content, std::string* error = nullptr)` | 保存或写入 write 相关逻辑。 |

## `src/platform/win/WindowsCursorMonitor.cpp`

[打开源码](../src/platform/win/WindowsCursorMonitor.cpp) · **文件作用：** 实现 windows cursor monitor 相关函数与文件级辅助逻辑。

### 类型

| 行 | 类型 | 种类 | 作用 |
|---:|---|---|---|
| [L22](../src/platform/win/WindowsCursorMonitor.cpp#L22) | `OwnedBitmap` | class | 定义 OwnedBitmap 的 class 类型和相关状态。 |
| [L31](../src/platform/win/WindowsCursorMonitor.cpp#L31) | `OwnedDc` | class | 定义 OwnedDc 的 class 类型和相关状态。 |

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L24](../src/platform/win/WindowsCursorMonitor.cpp#L24) | `OwnedBitmap` | 定义 | `explicit OwnedBitmap(HBITMAP value = nullptr) : value_(value) {}` | 实现 owned bitmap 对应的业务或工具逻辑。 |
| [L25](../src/platform/win/WindowsCursorMonitor.cpp#L25) | `~OwnedBitmap` | 定义 | `~OwnedBitmap() { if (value_) DeleteObject(value_); }` | 停止相关活动并释放 OwnedBitmap 实例拥有的资源。 |
| [L26](../src/platform/win/WindowsCursorMonitor.cpp#L26) | `get` | 定义 | `HBITMAP get() const { return value_; }` | 查询并返回 get 相关逻辑。 |
| [L33](../src/platform/win/WindowsCursorMonitor.cpp#L33) | `OwnedDc` | 定义 | `explicit OwnedDc(HDC value = nullptr) : value_(value) {}` | 实现 owned dc 对应的业务或工具逻辑。 |
| [L34](../src/platform/win/WindowsCursorMonitor.cpp#L34) | `~OwnedDc` | 定义 | `~OwnedDc() { if (value_) DeleteDC(value_); }` | 停止相关活动并释放 OwnedDc 实例拥有的资源。 |
| [L35](../src/platform/win/WindowsCursorMonitor.cpp#L35) | `get` | 定义 | `HDC get() const { return value_; }` | 查询并返回 get 相关逻辑。 |
| [L40](../src/platform/win/WindowsCursorMonitor.cpp#L40) | `HashShape` | 定义 | `std::uint64_t HashShape(const RemoteCursorShape& shape)` | 判断 hash shape 相关逻辑。 |
| [L57](../src/platform/win/WindowsCursorMonitor.cpp#L57) | `RenderCursor` | 定义 | `bool RenderCursor(HCURSOR cursor, int width, int height, std::uint8_t background, std::vector<std::uint8_t>* pixels)` | 准备或呈现 render cursor 相关逻辑。 |
| [L88](../src/platform/win/WindowsCursorMonitor.cpp#L88) | `CaptureShape` | 定义 | `std::optional<RemoteCursorShape> CaptureShape(HCURSOR cursor)` | 采集 capture shape 相关逻辑。 |
| [L156](../src/platform/win/WindowsCursorMonitor.cpp#L156) | `NormalizeCoordinate` | 定义 | `std::uint16_t NormalizeCoordinate(std::int64_t value, std::int64_t origin, std::uint32_t extent)` | 实现 normalize coordinate 对应的业务或工具逻辑。 |
| [L170](../src/platform/win/WindowsCursorMonitor.cpp#L170) | `WindowsCursorMonitor::~WindowsCursorMonitor` | 定义 | `WindowsCursorMonitor::~WindowsCursorMonitor()` | 停止相关活动并释放 WindowsCursorMonitor 实例拥有的资源。 |
| [L175](../src/platform/win/WindowsCursorMonitor.cpp#L175) | `WindowsCursorMonitor::Start` | 定义 | `bool WindowsCursorMonitor::Start(const DisplayDescriptor& display, std::uint64_t layoutVersion, Callback callback)` | 启动 start 相关逻辑。 |
| [L196](../src/platform/win/WindowsCursorMonitor.cpp#L196) | `WindowsCursorMonitor::UpdateTarget` | 定义 | `void WindowsCursorMonitor::UpdateTarget( const DisplayDescriptor& display, std::uint64_t layoutVersion)` | 更新或应用 update target 相关逻辑。 |
| [L205](../src/platform/win/WindowsCursorMonitor.cpp#L205) | `WindowsCursorMonitor::SetLastAppliedInputSequence` | 定义 | `void WindowsCursorMonitor::SetLastAppliedInputSequence( std::uint64_t sequence)` | 更新或应用 set last applied input sequence 相关逻辑。 |
| [L211](../src/platform/win/WindowsCursorMonitor.cpp#L211) | `WindowsCursorMonitor::Stop` | 定义 | `void WindowsCursorMonitor::Stop()` | 停止 stop 相关逻辑。 |
| [L223](../src/platform/win/WindowsCursorMonitor.cpp#L223) | `WindowsCursorMonitor::running` | 定义 | `bool WindowsCursorMonitor::running() const` | 执行后台循环或调度 running 相关逻辑。 |
| [L228](../src/platform/win/WindowsCursorMonitor.cpp#L228) | `WindowsCursorMonitor::Run` | 定义 | `void WindowsCursorMonitor::Run(std::stop_token stopToken)` | 执行后台循环或调度 run 相关逻辑。 |

## `src/platform/win/WindowsCursorMonitor.h`

[打开源码](../src/platform/win/WindowsCursorMonitor.h) · **文件作用：** 声明 windows cursor monitor 相关类型、接口、配置和成员状态。

### 类型

| 行 | 类型 | 种类 | 作用 |
|---:|---|---|---|
| [L18](../src/platform/win/WindowsCursorMonitor.h#L18) | `WindowsCursorObservation` | struct | 定义 WindowsCursorObservation 的 struct 类型和相关状态。 |
| [L26](../src/platform/win/WindowsCursorMonitor.h#L26) | `WindowsCursorMonitor` | class | Captures the process-global Windows cursor independently from desktop video. The monitor owns one lightweight 120 Hz waitable-timer thread only while a local desktop is actively... |
| [L46](../src/platform/win/WindowsCursorMonitor.h#L46) | `Target` | struct | 定义 Target 的 struct 类型和相关状态。 |

### 成员与文件级变量

| 行 | 变量 | 声明 | 作用 |
|---:|---|---|---|
| [L19](../src/platform/win/WindowsCursorMonitor.h#L19) | `position` | `RemoteCursorPosition position;` | 保存 position 相关配置或运行状态。 |
| [L20](../src/platform/win/WindowsCursorMonitor.h#L20) | `changedShape` | `std::optional<RemoteCursorShape> changedShape;` | 保存 changed shape 相关配置或运行状态。 |
| [L47](../src/platform/win/WindowsCursorMonitor.h#L47) | `display` | `DisplayDescriptor display;` | 保存 display 相关配置或运行状态。 |
| [L48](../src/platform/win/WindowsCursorMonitor.h#L48) | `layoutVersion` | `std::uint64_t layoutVersion = 0;` | 保存 layout version 相关配置或运行状态。 |
| [L53](../src/platform/win/WindowsCursorMonitor.h#L53) | `mutex_` | `mutable std::mutex mutex_;` | 保护跨线程共享状态：mutex。 |
| [L54](../src/platform/win/WindowsCursorMonitor.h#L54) | `target_` | `Target target_;` | 保存 target 相关配置或运行状态。 |
| [L55](../src/platform/win/WindowsCursorMonitor.h#L55) | `callback_` | `Callback callback_;` | 保存回调或观察者入口：callback。 |
| [L56](../src/platform/win/WindowsCursorMonitor.h#L56) | `thread_` | `std::jthread thread_;` | 拥有后台执行线程或工作器：thread。 |
| [L57](../src/platform/win/WindowsCursorMonitor.h#L57) | `running_` | `std::atomic<bool> running_{false};` | 保存 running 相关配置或运行状态。 |
| [L58](../src/platform/win/WindowsCursorMonitor.h#L58) | `lastAppliedInputSequence_` | `std::atomic<std::uint64_t> lastAppliedInputSequence_{0};` | 保存单调序号，用于排序或去重：last applied input sequence。 |

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L30](../src/platform/win/WindowsCursorMonitor.h#L30) | `WindowsCursorMonitor` | 声明 | `WindowsCursorMonitor() = default` | 实现 windows cursor monitor 对应的业务或工具逻辑。 |
| [L31](../src/platform/win/WindowsCursorMonitor.h#L31) | `~WindowsCursorMonitor` | 声明 | `~WindowsCursorMonitor()` | 停止相关活动并释放 WindowsCursorMonitor 实例拥有的资源。 |
| [L33](../src/platform/win/WindowsCursorMonitor.h#L33) | `WindowsCursorMonitor` | 声明 | `WindowsCursorMonitor(const WindowsCursorMonitor&) = delete` | 实现 windows cursor monitor 对应的业务或工具逻辑。 |
| [L36](../src/platform/win/WindowsCursorMonitor.h#L36) | `Start` | 声明 | `bool Start(const DisplayDescriptor& display, std::uint64_t layoutVersion, Callback callback)` | 启动 start 相关逻辑。 |
| [L39](../src/platform/win/WindowsCursorMonitor.h#L39) | `UpdateTarget` | 声明 | `void UpdateTarget(const DisplayDescriptor& display, std::uint64_t layoutVersion)` | 更新或应用 update target 相关逻辑。 |
| [L41](../src/platform/win/WindowsCursorMonitor.h#L41) | `SetLastAppliedInputSequence` | 声明 | `void SetLastAppliedInputSequence(std::uint64_t sequence)` | 更新或应用 set last applied input sequence 相关逻辑。 |
| [L42](../src/platform/win/WindowsCursorMonitor.h#L42) | `Stop` | 声明 | `void Stop()` | 停止 stop 相关逻辑。 |
| [L43](../src/platform/win/WindowsCursorMonitor.h#L43) | `running` | 声明 | `bool running() const` | 执行后台循环或调度 running 相关逻辑。 |
| [L51](../src/platform/win/WindowsCursorMonitor.h#L51) | `Run` | 声明 | `void Run(std::stop_token stopToken)` | 执行后台循环或调度 run 相关逻辑。 |

## `src/platform/win/WindowsDesktopCaptureSource.cpp`

[打开源码](../src/platform/win/WindowsDesktopCaptureSource.cpp) · **文件作用：** 实现 windows desktop capture source 相关函数与文件级辅助逻辑。

### 类型

| 行 | 类型 | 种类 | 作用 |
|---:|---|---|---|
| [L113](../src/platform/win/WindowsDesktopCaptureSource.cpp#L113) | `UniqueWinHandle` | class | 定义 UniqueWinHandle 的 class 类型和相关状态。 |
| [L175](../src/platform/win/WindowsDesktopCaptureSource.cpp#L175) | `CaptureWaitResult` | enum class | 定义 CaptureWaitResult 的 enum class 类型和相关状态。 |
| [L251](../src/platform/win/WindowsDesktopCaptureSource.cpp#L251) | `UpdateBounds` | struct | 定义 UpdateBounds 的 struct 类型和相关状态。 |

### 成员与文件级变量

| 行 | 变量 | 声明 | 作用 |
|---:|---|---|---|
| [L26](../src/platform/win/WindowsDesktopCaptureSource.cpp#L26) | `kMinimumCaptureFrameRate` | `constexpr std::uint32_t kMinimumCaptureFrameRate = 5;` | 定义 minimum capture frame rate 的编译期常量或产品边界。 |
| [L27](../src/platform/win/WindowsDesktopCaptureSource.cpp#L27) | `kMaximumCaptureFrameRate` | `constexpr std::uint32_t kMaximumCaptureFrameRate = 120;` | 定义 maximum capture frame rate 的编译期常量或产品边界。 |

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L35](../src/platform/win/WindowsDesktopCaptureSource.cpp#L35) | `SteadyNowUs` | 定义 | `std::int64_t SteadyNowUs()` | 实现 steady now us 对应的业务或工具逻辑。 |
| [L42](../src/platform/win/WindowsDesktopCaptureSource.cpp#L42) | `Utf8FromWide` | 定义 | `std::string Utf8FromWide(const wchar_t* text)` | 实现 utf8 from wide 对应的业务或工具逻辑。 |
| [L65](../src/platform/win/WindowsDesktopCaptureSource.cpp#L65) | `FindLibWebRtcScreenSource` | 定义 | `FindLibWebRtcScreenSource( const DisplayDescriptor& target, const webrtc::DesktopCapturer::SourceList& sources)` | 查询并返回 find lib web rtc screen source 相关逻辑。 |
| [L115](../src/platform/win/WindowsDesktopCaptureSource.cpp#L115) | `UniqueWinHandle` | 定义 | `explicit UniqueWinHandle(HANDLE handle = nullptr) noexcept : handle_(handle)` | 实现 unique win handle 对应的业务或工具逻辑。 |
| [L120](../src/platform/win/WindowsDesktopCaptureSource.cpp#L120) | `~UniqueWinHandle` | 定义 | `~UniqueWinHandle()` | 停止相关活动并释放 UniqueWinHandle 实例拥有的资源。 |
| [L130](../src/platform/win/WindowsDesktopCaptureSource.cpp#L130) | `UniqueWinHandle` | 定义 | `UniqueWinHandle(UniqueWinHandle&& other) noexcept : handle_(std::exchange(other.handle_, nullptr))` | 实现 unique win handle 对应的业务或工具逻辑。 |
| [L146](../src/platform/win/WindowsDesktopCaptureSource.cpp#L146) | `get` | 定义 | `HANDLE get() const noexcept` | 查询并返回 get 相关逻辑。 |
| [L155](../src/platform/win/WindowsDesktopCaptureSource.cpp#L155) | `CreateCaptureTimer` | 定义 | `UniqueWinHandle CreateCaptureTimer()` | 创建或初始化 create capture timer 相关逻辑。 |
| [L181](../src/platform/win/WindowsDesktopCaptureSource.cpp#L181) | `WaitForLibWebRtcCaptureDeadline` | 定义 | `CaptureWaitResult WaitForLibWebRtcCaptureDeadline( HANDLE timer, HANDLE stopEvent, HANDLE scheduleWakeEvent, std::chrono::steady_clock::time_point deadline, std::stop_token stopToken)` | 实现 wait for lib web rtc capture deadline 对应的业务或工具逻辑。 |
| [L258](../src/platform/win/WindowsDesktopCaptureSource.cpp#L258) | `BoundingUpdateRegion` | 定义 | `UpdateBounds BoundingUpdateRegion( const webrtc::DesktopRegion& region, int frameWidth, int frameHeight)` | 实现 bounding update region 对应的业务或工具逻辑。 |
| [L295](../src/platform/win/WindowsDesktopCaptureSource.cpp#L295) | `WindowsDesktopCaptureSource::WindowsDesktopCaptureSource` | 定义 | `WindowsDesktopCaptureSource::WindowsDesktopCaptureSource( DesktopCaptureImplementation implementation, DisplayDescriptor captureTarget) : configuredImplementation_(implementation), captureTarget_(std::move(captureTarg...` | 构造并初始化 WindowsDesktopCaptureSource 实例。 |
| [L303](../src/platform/win/WindowsDesktopCaptureSource.cpp#L303) | `WindowsDesktopCaptureSource::~WindowsDesktopCaptureSource` | 定义 | `WindowsDesktopCaptureSource::~WindowsDesktopCaptureSource()` | 停止相关活动并释放 WindowsDesktopCaptureSource 实例拥有的资源。 |
| [L308](../src/platform/win/WindowsDesktopCaptureSource.cpp#L308) | `WindowsDesktopCaptureSource::StartCapture` | 定义 | `bool WindowsDesktopCaptureSource::StartCapture( std::chrono::milliseconds firstFrameTimeout)` | 启动 start capture 相关逻辑。 |
| [L341](../src/platform/win/WindowsDesktopCaptureSource.cpp#L341) | `WindowsDesktopCaptureSource::StopCapture` | 定义 | `void WindowsDesktopCaptureSource::StopCapture()` | 停止 stop capture 相关逻辑。 |
| [L358](../src/platform/win/WindowsDesktopCaptureSource.cpp#L358) | `WindowsDesktopCaptureSource::SetTargetFrameRate` | 定义 | `bool WindowsDesktopCaptureSource::SetTargetFrameRate( std::uint32_t framesPerSecond)` | 更新或应用 set target frame rate 相关逻辑。 |
| [L370](../src/platform/win/WindowsDesktopCaptureSource.cpp#L370) | `WindowsDesktopCaptureSource::NotifyRemoteInputActivity` | 定义 | `void WindowsDesktopCaptureSource::NotifyRemoteInputActivity()` | 通知或报告 notify remote input activity 相关逻辑。 |
| [L397](../src/platform/win/WindowsDesktopCaptureSource.cpp#L397) | `WindowsDesktopCaptureSource::RequestRefreshFrame` | 定义 | `void WindowsDesktopCaptureSource::RequestRefreshFrame()` | 发起请求或查询 request refresh frame 相关逻辑。 |
| [L402](../src/platform/win/WindowsDesktopCaptureSource.cpp#L402) | `WindowsDesktopCaptureSource::RequestStartupFrameBurst` | 定义 | `void WindowsDesktopCaptureSource::RequestStartupFrameBurst( std::uint32_t frameCount, std::function<void()> firstDeliveredFrameCallback, std::function<void()> burstCompletedCallback)` | 发起请求或查询 request startup frame burst 相关逻辑。 |
| [L435](../src/platform/win/WindowsDesktopCaptureSource.cpp#L435) | `WindowsDesktopCaptureSource::ScheduleForcedRefreshFrames` | 定义 | `void WindowsDesktopCaptureSource::ScheduleForcedRefreshFrames( std::uint32_t frameCount)` | 执行后台循环或调度 schedule forced refresh frames 相关逻辑。 |
| [L450](../src/platform/win/WindowsDesktopCaptureSource.cpp#L450) | `WindowsDesktopCaptureSource::ConsumeForcedRefreshFrame` | 定义 | `bool WindowsDesktopCaptureSource::ConsumeForcedRefreshFrame()` | 实现 consume forced refresh frame 对应的业务或工具逻辑。 |
| [L466](../src/platform/win/WindowsDesktopCaptureSource.cpp#L466) | `WindowsDesktopCaptureSource::TargetFrameRate` | 定义 | `std::uint32_t WindowsDesktopCaptureSource::TargetFrameRate() const noexcept` | 实现 target frame rate 对应的业务或工具逻辑。 |
| [L471](../src/platform/win/WindowsDesktopCaptureSource.cpp#L471) | `WindowsDesktopCaptureSource::CapturedWidth` | 定义 | `std::uint32_t WindowsDesktopCaptureSource::CapturedWidth() const noexcept` | 采集 captured width 相关逻辑。 |
| [L476](../src/platform/win/WindowsDesktopCaptureSource.cpp#L476) | `WindowsDesktopCaptureSource::CapturedHeight` | 定义 | `std::uint32_t WindowsDesktopCaptureSource::CapturedHeight() const noexcept` | 采集 captured height 相关逻辑。 |
| [L482](../src/platform/win/WindowsDesktopCaptureSource.cpp#L482) | `WindowsDesktopCaptureSource::CaptureRuntimeStats` | 定义 | `WindowsDesktopCaptureSource::CaptureRuntimeStats() const noexcept` | 采集 capture runtime stats 相关逻辑。 |
| [L531](../src/platform/win/WindowsDesktopCaptureSource.cpp#L531) | `WindowsDesktopCaptureSource::SignalCaptureSchedule` | 定义 | `void WindowsDesktopCaptureSource::SignalCaptureSchedule()` | 实现 signal capture schedule 对应的业务或工具逻辑。 |
| [L540](../src/platform/win/WindowsDesktopCaptureSource.cpp#L540) | `WindowsDesktopCaptureSource::Backend` | 定义 | `WindowsDesktopCaptureSource::Backend() const` | 实现 backend 对应的业务或工具逻辑。 |
| [L547](../src/platform/win/WindowsDesktopCaptureSource.cpp#L547) | `WindowsDesktopCaptureSource::ConfiguredImplementation` | 定义 | `WindowsDesktopCaptureSource::ConfiguredImplementation() const noexcept` | 更新或应用 configured implementation 相关逻辑。 |
| [L552](../src/platform/win/WindowsDesktopCaptureSource.cpp#L552) | `WindowsDesktopCaptureSource::FallbackReason` | 定义 | `std::string WindowsDesktopCaptureSource::FallbackReason() const` | 实现 fallback reason 对应的业务或工具逻辑。 |
| [L558](../src/platform/win/WindowsDesktopCaptureSource.cpp#L558) | `WindowsDesktopCaptureSource::LastError` | 定义 | `std::string WindowsDesktopCaptureSource::LastError() const` | 实现 last error 对应的业务或工具逻辑。 |
| [L565](../src/platform/win/WindowsDesktopCaptureSource.cpp#L565) | `WindowsDesktopCaptureSource::CaptureTarget` | 定义 | `WindowsDesktopCaptureSource::CaptureTarget() const noexcept` | 采集 capture target 相关逻辑。 |
| [L571](../src/platform/win/WindowsDesktopCaptureSource.cpp#L571) | `WindowsDesktopCaptureSource::state` | 定义 | `WindowsDesktopCaptureSource::state() const` | 实现 state 对应的业务或工具逻辑。 |
| [L577](../src/platform/win/WindowsDesktopCaptureSource.cpp#L577) | `WindowsDesktopCaptureSource::remote` | 定义 | `bool WindowsDesktopCaptureSource::remote() const` | 实现 remote 对应的业务或工具逻辑。 |
| [L582](../src/platform/win/WindowsDesktopCaptureSource.cpp#L582) | `WindowsDesktopCaptureSource::is_screencast` | 定义 | `bool WindowsDesktopCaptureSource::is_screencast() const` | 判断 is screencast 相关逻辑。 |
| [L587](../src/platform/win/WindowsDesktopCaptureSource.cpp#L587) | `WindowsDesktopCaptureSource::needs_denoising` | 定义 | `std::optional<bool> WindowsDesktopCaptureSource::needs_denoising() const` | 判断 needs denoising 相关逻辑。 |
| [L592](../src/platform/win/WindowsDesktopCaptureSource.cpp#L592) | `WindowsDesktopCaptureSource::CaptureLoop` | 定义 | `void WindowsDesktopCaptureSource::CaptureLoop(std::stop_token stopToken)` | 采集 capture loop 相关逻辑。 |
| [L1033](../src/platform/win/WindowsDesktopCaptureSource.cpp#L1033) | `WindowsDesktopCaptureSource::OnCaptureResult` | 定义 | `void WindowsDesktopCaptureSource::OnCaptureResult( webrtc::DesktopCapturer::Result result, std::unique_ptr<webrtc::DesktopFrame> frame)` | 接收并处理 on capture result 相关逻辑。 |
| [L1116](../src/platform/win/WindowsDesktopCaptureSource.cpp#L1116) | `WindowsDesktopCaptureSource::ResetActivityTracking` | 定义 | `void WindowsDesktopCaptureSource::ResetActivityTracking()` | 重置或移除 reset activity tracking 相关逻辑。 |
| [L1133](../src/platform/win/WindowsDesktopCaptureSource.cpp#L1133) | `WindowsDesktopCaptureSource::ShouldDeliverFrame` | 定义 | `bool WindowsDesktopCaptureSource::ShouldDeliverFrame( bool desktopChanged, bool forceRefresh, std::chrono::steady_clock::time_point now, FrameDeliveryReason* reason)` | 判断 should deliver frame 相关逻辑。 |
| [L1168](../src/platform/win/WindowsDesktopCaptureSource.cpp#L1168) | `WindowsDesktopCaptureSource::ShouldDeliverLibWebRtcFrame` | 定义 | `bool WindowsDesktopCaptureSource::ShouldDeliverLibWebRtcFrame( bool desktopChanged, bool forceRefresh, std::chrono::steady_clock::time_point now, FrameDeliveryReason* reason)` | 判断 should deliver lib web rtc frame 相关逻辑。 |
| [L1248](../src/platform/win/WindowsDesktopCaptureSource.cpp#L1248) | `WindowsDesktopCaptureSource::DeliverFrame` | 定义 | `void WindowsDesktopCaptureSource::DeliverFrame( webrtc::scoped_refptr<webrtc::VideoFrameBuffer> buffer, FrameDeliveryReason reason)` | 实现 deliver frame 对应的业务或工具逻辑。 |
| [L1276](../src/platform/win/WindowsDesktopCaptureSource.cpp#L1276) | `WindowsDesktopCaptureSource::DeliverLibWebRtcFrame` | 定义 | `void WindowsDesktopCaptureSource::DeliverLibWebRtcFrame( webrtc::scoped_refptr<webrtc::VideoFrameBuffer> buffer, FrameDeliveryReason reason, const FrameUpdateRegion& updateRegion, bool repeatFrame)` | 实现 deliver lib web rtc frame 对应的业务或工具逻辑。 |
| [L1328](../src/platform/win/WindowsDesktopCaptureSource.cpp#L1328) | `WindowsDesktopCaptureSource::SetInitializationFailure` | 定义 | `void WindowsDesktopCaptureSource::SetInitializationFailure( std::string message)` | 更新或应用 set initialization failure 相关逻辑。 |

## `src/platform/win/WindowsDesktopCaptureSource.h`

[打开源码](../src/platform/win/WindowsDesktopCaptureSource.h) · **文件作用：** 声明 windows desktop capture source 相关类型、接口、配置和成员状态。

### 类型

| 行 | 类型 | 种类 | 作用 |
|---:|---|---|---|
| [L24](../src/platform/win/WindowsDesktopCaptureSource.h#L24) | `DesktopCaptureImplementation` | enum class | 定义 DesktopCaptureImplementation 的 enum class 类型和相关状态。 |
| [L32](../src/platform/win/WindowsDesktopCaptureSource.h#L32) | `WindowsDesktopCaptureSource` | class | Captures one Windows display on a dedicated thread. WebRTC selects DXGI Desktop Duplication once when the capturer is created and keeps GDI as the permanent-error fallback; ther... |
| [L36](../src/platform/win/WindowsDesktopCaptureSource.h#L36) | `CaptureActivityState` | enum class | 定义 CaptureActivityState 的 enum class 类型和相关状态。 |
| [L42](../src/platform/win/WindowsDesktopCaptureSource.h#L42) | `RuntimeStats` | struct | 定义 RuntimeStats 的 struct 类型和相关状态。 |
| [L64](../src/platform/win/WindowsDesktopCaptureSource.h#L64) | `CaptureBackend` | enum class | 定义 CaptureBackend 的 enum class 类型和相关状态。 |
| [L112](../src/platform/win/WindowsDesktopCaptureSource.h#L112) | `FrameDeliveryReason` | enum class | 定义 FrameDeliveryReason 的 enum class 类型和相关状态。 |
| [L121](../src/platform/win/WindowsDesktopCaptureSource.h#L121) | `FrameUpdateRegion` | struct | 定义 FrameUpdateRegion 的 struct 类型和相关状态。 |

### 成员与文件级变量

| 行 | 变量 | 声明 | 作用 |
|---:|---|---|---|
| [L43](../src/platform/win/WindowsDesktopCaptureSource.h#L43) | `targetFrameRate` | `std::uint32_t targetFrameRate = 0;` | 保存计数、尺寸或速率指标：target frame rate。 |
| [L45](../src/platform/win/WindowsDesktopCaptureSource.h#L45) | `kStarting` | `CaptureActivityState::kStarting;` | 定义 starting 的编译期常量或产品边界。 |
| [L46](../src/platform/win/WindowsDesktopCaptureSource.h#L46) | `adaptiveFrameDeliveryEnabled` | `bool adaptiveFrameDeliveryEnabled = false;` | 保存能力或开关状态：adaptive frame delivery enabled。 |
| [L47](../src/platform/win/WindowsDesktopCaptureSource.h#L47) | `captureAttemptsPerSecond` | `double captureAttemptsPerSecond = 0.0;` | 保存 capture attempts per second 相关配置或运行状态。 |
| [L48](../src/platform/win/WindowsDesktopCaptureSource.h#L48) | `deliveredFramesPerSecond` | `double deliveredFramesPerSecond = 0.0;` | 保存 delivered frames per second 相关配置或运行状态。 |
| [L49](../src/platform/win/WindowsDesktopCaptureSource.h#L49) | `changedFramesPerSecond` | `double changedFramesPerSecond = 0.0;` | 保存 changed frames per second 相关配置或运行状态。 |
| [L50](../src/platform/win/WindowsDesktopCaptureSource.h#L50) | `idleHeartbeatFramesPerSecond` | `double idleHeartbeatFramesPerSecond = 0.0;` | 保存 idle heartbeat frames per second 相关配置或运行状态。 |
| [L51](../src/platform/win/WindowsDesktopCaptureSource.h#L51) | `totalCaptureAttempts` | `std::uint64_t totalCaptureAttempts = 0;` | 保存 total capture attempts 相关配置或运行状态。 |
| [L52](../src/platform/win/WindowsDesktopCaptureSource.h#L52) | `totalDeliveredFrames` | `std::uint64_t totalDeliveredFrames = 0;` | 保存 total delivered frames 相关配置或运行状态。 |
| [L53](../src/platform/win/WindowsDesktopCaptureSource.h#L53) | `totalChangedFrames` | `std::uint64_t totalChangedFrames = 0;` | 保存 total changed frames 相关配置或运行状态。 |
| [L54](../src/platform/win/WindowsDesktopCaptureSource.h#L54) | `totalIdleHeartbeatFrames` | `std::uint64_t totalIdleHeartbeatFrames = 0;` | 保存 total idle heartbeat frames 相关配置或运行状态。 |
| [L55](../src/platform/win/WindowsDesktopCaptureSource.h#L55) | `totalSuppressedUnchangedFrames` | `std::uint64_t totalSuppressedUnchangedFrames = 0;` | 保存 total suppressed unchanged frames 相关配置或运行状态。 |
| [L56](../src/platform/win/WindowsDesktopCaptureSource.h#L56) | `totalActivityTransitions` | `std::uint64_t totalActivityTransitions = 0;` | 保存 total activity transitions 相关配置或运行状态。 |
| [L57](../src/platform/win/WindowsDesktopCaptureSource.h#L57) | `totalFailedCaptures` | `std::uint64_t totalFailedCaptures = 0;` | 保存 total failed captures 相关配置或运行状态。 |
| [L58](../src/platform/win/WindowsDesktopCaptureSource.h#L58) | `inputBoostActive` | `bool inputBoostActive = false;` | 保存能力或开关状态：input boost active。 |
| [L59](../src/platform/win/WindowsDesktopCaptureSource.h#L59) | `totalInputBoosts` | `std::uint64_t totalInputBoosts = 0;` | 保存 total input boosts 相关配置或运行状态。 |
| [L60](../src/platform/win/WindowsDesktopCaptureSource.h#L60) | `totalForcedRefreshFrames` | `std::uint64_t totalForcedRefreshFrames = 0;` | 保存 total forced refresh frames 相关配置或运行状态。 |
| [L61](../src/platform/win/WindowsDesktopCaptureSource.h#L61) | `latestCaptureCallMs` | `double latestCaptureCallMs = 0.0;` | 保存 latest capture call ms 相关配置或运行状态。 |
| [L122](../src/platform/win/WindowsDesktopCaptureSource.h#L122) | `offsetX` | `int offsetX = 0;` | 保存 offset x 相关配置或运行状态。 |
| [L123](../src/platform/win/WindowsDesktopCaptureSource.h#L123) | `offsetY` | `int offsetY = 0;` | 保存 offset y 相关配置或运行状态。 |
| [L124](../src/platform/win/WindowsDesktopCaptureSource.h#L124) | `width` | `int width = 0;` | 保存计数、尺寸或速率指标：width。 |
| [L125](../src/platform/win/WindowsDesktopCaptureSource.h#L125) | `height` | `int height = 0;` | 保存计数、尺寸或速率指标：height。 |
| [L156](../src/platform/win/WindowsDesktopCaptureSource.h#L156) | `mutex_` | `mutable std::mutex mutex_;` | 保护跨线程共享状态：mutex。 |
| [L157](../src/platform/win/WindowsDesktopCaptureSource.h#L157) | `firstFrameCondition_` | `std::condition_variable firstFrameCondition_;` | 保存 first frame condition 相关配置或运行状态。 |
| [L158](../src/platform/win/WindowsDesktopCaptureSource.h#L158) | `captureThread_` | `std::jthread captureThread_;` | 拥有后台执行线程或工作器：capture thread。 |
| [L159](../src/platform/win/WindowsDesktopCaptureSource.h#L159) | `configuredImplementation_` | `const DesktopCaptureImplementation configuredImplementation_;` | 保存 configured implementation 相关配置或运行状态。 |
| [L160](../src/platform/win/WindowsDesktopCaptureSource.h#L160) | `captureTarget_` | `const DisplayDescriptor captureTarget_;` | 保存 capture target 相关配置或运行状态。 |
| [L161](../src/platform/win/WindowsDesktopCaptureSource.h#L161) | `backend_` | `CaptureBackend backend_ = CaptureBackend::kGdi;` | 保存 backend 相关配置或运行状态。 |
| [L162](../src/platform/win/WindowsDesktopCaptureSource.h#L162) | `initializationFinished_` | `bool initializationFinished_ = false;` | 保存 initialization finished 相关配置或运行状态。 |
| [L163](../src/platform/win/WindowsDesktopCaptureSource.h#L163) | `firstFrameReady_` | `bool firstFrameReady_ = false;` | 保存能力或开关状态：first frame ready。 |
| [L164](../src/platform/win/WindowsDesktopCaptureSource.h#L164) | `running_` | `bool running_ = false;` | 保存 running 相关配置或运行状态。 |
| [L165](../src/platform/win/WindowsDesktopCaptureSource.h#L165) | `fallbackReason_` | `std::string fallbackReason_;` | 保存 fallback reason 相关配置或运行状态。 |
| [L166](../src/platform/win/WindowsDesktopCaptureSource.h#L166) | `lastError_` | `std::string lastError_;` | 保存最近错误或失败原因：last error。 |
| [L167](../src/platform/win/WindowsDesktopCaptureSource.h#L167) | `targetFrameRate_` | `std::atomic<std::uint32_t> targetFrameRate_{60};` | 保存计数、尺寸或速率指标：target frame rate。 |
| [L170](../src/platform/win/WindowsDesktopCaptureSource.h#L170) | `adaptiveFrameDeliveryEnabled_` | `std::atomic<bool> adaptiveFrameDeliveryEnabled_{false};` | 保存能力或开关状态：adaptive frame delivery enabled。 |
| [L171](../src/platform/win/WindowsDesktopCaptureSource.h#L171) | `capturedWidth_` | `std::atomic<std::uint32_t> capturedWidth_{0};` | 保存计数、尺寸或速率指标：captured width。 |
| [L172](../src/platform/win/WindowsDesktopCaptureSource.h#L172) | `capturedHeight_` | `std::atomic<std::uint32_t> capturedHeight_{0};` | 保存计数、尺寸或速率指标：captured height。 |
| [L173](../src/platform/win/WindowsDesktopCaptureSource.h#L173) | `totalCaptureAttempts_` | `std::atomic<std::uint64_t> totalCaptureAttempts_{0};` | 保存 total capture attempts 相关配置或运行状态。 |
| [L174](../src/platform/win/WindowsDesktopCaptureSource.h#L174) | `totalDeliveredFrames_` | `std::atomic<std::uint64_t> totalDeliveredFrames_{0};` | 保存 total delivered frames 相关配置或运行状态。 |
| [L175](../src/platform/win/WindowsDesktopCaptureSource.h#L175) | `totalChangedFrames_` | `std::atomic<std::uint64_t> totalChangedFrames_{0};` | 保存 total changed frames 相关配置或运行状态。 |
| [L176](../src/platform/win/WindowsDesktopCaptureSource.h#L176) | `totalIdleHeartbeatFrames_` | `std::atomic<std::uint64_t> totalIdleHeartbeatFrames_{0};` | 保存 total idle heartbeat frames 相关配置或运行状态。 |
| [L177](../src/platform/win/WindowsDesktopCaptureSource.h#L177) | `totalSuppressedUnchangedFrames_` | `std::atomic<std::uint64_t> totalSuppressedUnchangedFrames_{0};` | 保存 total suppressed unchanged frames 相关配置或运行状态。 |
| [L178](../src/platform/win/WindowsDesktopCaptureSource.h#L178) | `totalActivityTransitions_` | `std::atomic<std::uint64_t> totalActivityTransitions_{0};` | 保存 total activity transitions 相关配置或运行状态。 |
| [L179](../src/platform/win/WindowsDesktopCaptureSource.h#L179) | `totalFailedCaptures_` | `std::atomic<std::uint64_t> totalFailedCaptures_{0};` | 保存 total failed captures 相关配置或运行状态。 |
| [L180](../src/platform/win/WindowsDesktopCaptureSource.h#L180) | `totalInputBoosts_` | `std::atomic<std::uint64_t> totalInputBoosts_{0};` | 保存 total input boosts 相关配置或运行状态。 |
| [L181](../src/platform/win/WindowsDesktopCaptureSource.h#L181) | `totalForcedRefreshFrames_` | `std::atomic<std::uint64_t> totalForcedRefreshFrames_{0};` | 保存 total forced refresh frames 相关配置或运行状态。 |
| [L182](../src/platform/win/WindowsDesktopCaptureSource.h#L182) | `captureAttemptsPerSecondMilli_` | `std::atomic<std::uint64_t> captureAttemptsPerSecondMilli_{0};` | 保存 capture attempts per second milli 相关配置或运行状态。 |
| [L183](../src/platform/win/WindowsDesktopCaptureSource.h#L183) | `deliveredFramesPerSecondMilli_` | `std::atomic<std::uint64_t> deliveredFramesPerSecondMilli_{0};` | 保存 delivered frames per second milli 相关配置或运行状态。 |
| [L184](../src/platform/win/WindowsDesktopCaptureSource.h#L184) | `changedFramesPerSecondMilli_` | `std::atomic<std::uint64_t> changedFramesPerSecondMilli_{0};` | 保存 changed frames per second milli 相关配置或运行状态。 |
| [L185](../src/platform/win/WindowsDesktopCaptureSource.h#L185) | `idleHeartbeatFramesPerSecondMilli_` | `std::atomic<std::uint64_t> idleHeartbeatFramesPerSecondMilli_{0};` | 保存 idle heartbeat frames per second milli 相关配置或运行状态。 |
| [L186](../src/platform/win/WindowsDesktopCaptureSource.h#L186) | `latestCaptureCallUs_` | `std::atomic<std::uint64_t> latestCaptureCallUs_{0};` | 保存 latest capture call us 相关配置或运行状态。 |
| [L187](../src/platform/win/WindowsDesktopCaptureSource.h#L187) | `inputBoostUntilSteadyUs_` | `std::atomic<std::int64_t> inputBoostUntilSteadyUs_{0};` | 保存 input boost until steady us 相关配置或运行状态。 |
| [L188](../src/platform/win/WindowsDesktopCaptureSource.h#L188) | `startupPrimeUntilSteadyUs_` | `std::atomic<std::int64_t> startupPrimeUntilSteadyUs_{0};` | 保存 startup prime until steady us 相关配置或运行状态。 |
| [L189](../src/platform/win/WindowsDesktopCaptureSource.h#L189) | `forcedRefreshFramesRemaining_` | `std::atomic<std::uint32_t> forcedRefreshFramesRemaining_{0};` | 保存 forced refresh frames remaining 相关配置或运行状态。 |
| [L192](../src/platform/win/WindowsDesktopCaptureSource.h#L192) | `captureScheduleWakeEvent_` | `void* captureScheduleWakeEvent_ = nullptr;` | HANDLE is kept opaque in the header. It is created and destroyed by the active capture thread while mutex_ protects publication/lifetime. |
| [L193](../src/platform/win/WindowsDesktopCaptureSource.h#L193) | `activityHasDeliveredFrame_` | `bool activityHasDeliveredFrame_ = false;` | 保存媒体帧、图像或缓冲资源：activity has delivered frame。 |
| [L194](../src/platform/win/WindowsDesktopCaptureSource.h#L194) | `activityLastChangedAt_` | `std::chrono::steady_clock::time_point activityLastChangedAt_{};` | 保存 activity last changed at 相关配置或运行状态。 |
| [L195](../src/platform/win/WindowsDesktopCaptureSource.h#L195) | `activityLastDeliveredAt_` | `std::chrono::steady_clock::time_point activityLastDeliveredAt_{};` | 保存 activity last delivered at 相关配置或运行状态。 |
| [L198](../src/platform/win/WindowsDesktopCaptureSource.h#L198) | `startupCallbackFramesRemaining_` | `std::uint32_t startupCallbackFramesRemaining_ = 0;` | 保存 startup callback frames remaining 相关配置或运行状态。 |

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L74](../src/platform/win/WindowsDesktopCaptureSource.h#L74) | `~WindowsDesktopCaptureSource` | 声明 | `~WindowsDesktopCaptureSource() override` | 停止相关活动并释放 WindowsDesktopCaptureSource 实例拥有的资源。 |
| [L76](../src/platform/win/WindowsDesktopCaptureSource.h#L76) | `StartCapture` | 声明 | `bool StartCapture( std::chrono::milliseconds firstFrameTimeout = std::chrono::seconds(3))` | 启动 start capture 相关逻辑。 |
| [L79](../src/platform/win/WindowsDesktopCaptureSource.h#L79) | `StopCapture` | 声明 | `void StopCapture()` | 停止 stop capture 相关逻辑。 |
| [L80](../src/platform/win/WindowsDesktopCaptureSource.h#L80) | `SetTargetFrameRate` | 声明 | `bool SetTargetFrameRate(std::uint32_t framesPerSecond)` | 更新或应用 set target frame rate 相关逻辑。 |
| [L83](../src/platform/win/WindowsDesktopCaptureSource.h#L83) | `NotifyRemoteInputActivity` | 声明 | `void NotifyRemoteInputActivity()` | Chrome Remote Desktop-style short capture boost. The caller invokes this only after the remote input has been injected into Windows. |
| [L86](../src/platform/win/WindowsDesktopCaptureSource.h#L86) | `RequestRefreshFrame` | 声明 | `void RequestRefreshFrame()` | Forces the next successful capture to be delivered as a full update. Used after ICE recovery and sender/track reactivation. |
| [L91](../src/platform/win/WindowsDesktopCaptureSource.h#L91) | `RequestStartupFrameBurst` | 定义 | `void RequestStartupFrameBurst( std::uint32_t frameCount = 6, std::function<void()> firstDeliveredFrameCallback = {},` | Delivers a short, bounded run of full frames after a new sender or sink becomes active. This gives WebRTC enough stable input for its first keyframe and startup bandwidth probe ... |
| [L95](../src/platform/win/WindowsDesktopCaptureSource.h#L95) | `TargetFrameRate` | 声明 | `std::uint32_t TargetFrameRate() const noexcept` | 实现 target frame rate 对应的业务或工具逻辑。 |
| [L96](../src/platform/win/WindowsDesktopCaptureSource.h#L96) | `CapturedWidth` | 声明 | `std::uint32_t CapturedWidth() const noexcept` | 采集 captured width 相关逻辑。 |
| [L97](../src/platform/win/WindowsDesktopCaptureSource.h#L97) | `CapturedHeight` | 声明 | `std::uint32_t CapturedHeight() const noexcept` | 采集 captured height 相关逻辑。 |
| [L98](../src/platform/win/WindowsDesktopCaptureSource.h#L98) | `CaptureRuntimeStats` | 声明 | `RuntimeStats CaptureRuntimeStats() const noexcept` | 采集 capture runtime stats 相关逻辑。 |
| [L100](../src/platform/win/WindowsDesktopCaptureSource.h#L100) | `ConfiguredImplementation` | 声明 | `DesktopCaptureImplementation ConfiguredImplementation() const noexcept` | 更新或应用 configured implementation 相关逻辑。 |
| [L101](../src/platform/win/WindowsDesktopCaptureSource.h#L101) | `Backend` | 声明 | `CaptureBackend Backend() const` | 实现 backend 对应的业务或工具逻辑。 |
| [L102](../src/platform/win/WindowsDesktopCaptureSource.h#L102) | `FallbackReason` | 声明 | `std::string FallbackReason() const` | 实现 fallback reason 对应的业务或工具逻辑。 |
| [L103](../src/platform/win/WindowsDesktopCaptureSource.h#L103) | `LastError` | 声明 | `std::string LastError() const` | 实现 last error 对应的业务或工具逻辑。 |
| [L104](../src/platform/win/WindowsDesktopCaptureSource.h#L104) | `CaptureTarget` | 声明 | `const DisplayDescriptor& CaptureTarget() const noexcept` | 采集 capture target 相关逻辑。 |
| [L106](../src/platform/win/WindowsDesktopCaptureSource.h#L106) | `state` | 声明 | `SourceState state() const override` | 实现 state 对应的业务或工具逻辑。 |
| [L107](../src/platform/win/WindowsDesktopCaptureSource.h#L107) | `remote` | 声明 | `bool remote() const override` | 实现 remote 对应的业务或工具逻辑。 |
| [L108](../src/platform/win/WindowsDesktopCaptureSource.h#L108) | `is_screencast` | 声明 | `bool is_screencast() const override` | 判断 is screencast 相关逻辑。 |
| [L109](../src/platform/win/WindowsDesktopCaptureSource.h#L109) | `needs_denoising` | 声明 | `std::optional<bool> needs_denoising() const override` | 判断 needs denoising 相关逻辑。 |
| [L128](../src/platform/win/WindowsDesktopCaptureSource.h#L128) | `CaptureLoop` | 声明 | `void CaptureLoop(std::stop_token stopToken)` | 采集 capture loop 相关逻辑。 |
| [L129](../src/platform/win/WindowsDesktopCaptureSource.h#L129) | `OnCaptureResult` | 声明 | `void OnCaptureResult( webrtc::DesktopCapturer::Result result, std::unique_ptr<webrtc::DesktopFrame> frame) override` | 接收并处理 on capture result 相关逻辑。 |
| [L132](../src/platform/win/WindowsDesktopCaptureSource.h#L132) | `SetInitializationFailure` | 声明 | `void SetInitializationFailure(std::string message)` | 更新或应用 set initialization failure 相关逻辑。 |
| [L133](../src/platform/win/WindowsDesktopCaptureSource.h#L133) | `ResetActivityTracking` | 声明 | `void ResetActivityTracking()` | 重置或移除 reset activity tracking 相关逻辑。 |
| [L134](../src/platform/win/WindowsDesktopCaptureSource.h#L134) | `ShouldDeliverFrame` | 声明 | `bool ShouldDeliverFrame( bool desktopChanged, bool forceRefresh, std::chrono::steady_clock::time_point now, FrameDeliveryReason* reason)` | 判断 should deliver frame 相关逻辑。 |
| [L139](../src/platform/win/WindowsDesktopCaptureSource.h#L139) | `ShouldDeliverLibWebRtcFrame` | 声明 | `bool ShouldDeliverLibWebRtcFrame( bool desktopChanged, bool forceRefresh, std::chrono::steady_clock::time_point now, FrameDeliveryReason* reason)` | 判断 should deliver lib web rtc frame 相关逻辑。 |
| [L144](../src/platform/win/WindowsDesktopCaptureSource.h#L144) | `DeliverFrame` | 声明 | `void DeliverFrame( webrtc::scoped_refptr<webrtc::VideoFrameBuffer> buffer, FrameDeliveryReason reason)` | 实现 deliver frame 对应的业务或工具逻辑。 |
| [L147](../src/platform/win/WindowsDesktopCaptureSource.h#L147) | `DeliverLibWebRtcFrame` | 声明 | `void DeliverLibWebRtcFrame( webrtc::scoped_refptr<webrtc::VideoFrameBuffer> buffer, FrameDeliveryReason reason, const FrameUpdateRegion& updateRegion, bool repeatFrame)` | 实现 deliver lib web rtc frame 对应的业务或工具逻辑。 |
| [L152](../src/platform/win/WindowsDesktopCaptureSource.h#L152) | `ConsumeForcedRefreshFrame` | 声明 | `bool ConsumeForcedRefreshFrame()` | 实现 consume forced refresh frame 对应的业务或工具逻辑。 |
| [L153](../src/platform/win/WindowsDesktopCaptureSource.h#L153) | `ScheduleForcedRefreshFrames` | 声明 | `void ScheduleForcedRefreshFrames(std::uint32_t frameCount)` | 执行后台循环或调度 schedule forced refresh frames 相关逻辑。 |
| [L154](../src/platform/win/WindowsDesktopCaptureSource.h#L154) | `SignalCaptureSchedule` | 声明 | `void SignalCaptureSchedule()` | 实现 signal capture schedule 对应的业务或工具逻辑。 |

## `src/platform/win/WindowsDisplayTopology.cpp`

[打开源码](../src/platform/win/WindowsDisplayTopology.cpp) · **文件作用：** 实现 windows display topology 相关函数与文件级辅助逻辑。

### 类型

| 行 | 类型 | 种类 | 作用 |
|---:|---|---|---|
| [L105](../src/platform/win/WindowsDisplayTopology.cpp#L105) | `DxgiOutputIdentity` | struct | 定义 DxgiOutputIdentity 的 struct 类型和相关状态。 |
| [L154](../src/platform/win/WindowsDisplayTopology.cpp#L154) | `EnumerationContext` | struct | 定义 EnumerationContext 的 struct 类型和相关状态。 |

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L22](../src/platform/win/WindowsDisplayTopology.cpp#L22) | `Utf8` | 定义 | `std::string Utf8(const wchar_t* value)` | 实现 utf8 对应的业务或工具逻辑。 |
| [L39](../src/platform/win/WindowsDisplayTopology.cpp#L39) | `Fnv1a64` | 定义 | `std::uint64_t Fnv1a64(const void* data, std::size_t size, std::uint64_t hash = 14695981039346656037ULL)` | 实现 fnv1a64 对应的业务或工具逻辑。 |
| [L51](../src/platform/win/WindowsDisplayTopology.cpp#L51) | `HashString` | 定义 | `std::uint64_t HashString(const std::string& value, std::uint64_t hash = 14695981039346656037ULL)` | 判断 hash string 相关逻辑。 |
| [L57](../src/platform/win/WindowsDisplayTopology.cpp#L57) | `SessionDisplayId` | 定义 | `std::uint32_t SessionDisplayId(const std::string& stableKey)` | 实现 session display id 对应的业务或工具逻辑。 |
| [L65](../src/platform/win/WindowsDisplayTopology.cpp#L65) | `RotationDegrees` | 定义 | `std::uint32_t RotationDegrees(DWORD orientation)` | 实现 rotation degrees 对应的业务或工具逻辑。 |
| [L79](../src/platform/win/WindowsDisplayTopology.cpp#L79) | `ReadMonitorDpi` | 定义 | `void ReadMonitorDpi(HMONITOR monitor, std::uint32_t* dpiX, std::uint32_t* dpiY)` | 读取或恢复 read monitor dpi 相关逻辑。 |
| [L111](../src/platform/win/WindowsDisplayTopology.cpp#L111) | `EnumerateDxgiOutputs` | 定义 | `EnumerateDxgiOutputs()` | 实现 enumerate dxgi outputs 对应的业务或工具逻辑。 |
| [L160](../src/platform/win/WindowsDisplayTopology.cpp#L160) | `AddMonitor` | 定义 | `BOOL CALLBACK AddMonitor(HMONITOR monitor, HDC, LPRECT, LPARAM parameter)` | 实现 add monitor 对应的业务或工具逻辑。 |
| [L227](../src/platform/win/WindowsDisplayTopology.cpp#L227) | `ComputeLayoutVersion` | 定义 | `std::uint64_t ComputeLayoutVersion( const std::vector<DisplayDescriptor>& displays)` | 计算或采集 compute layout version 相关逻辑。 |
| [L250](../src/platform/win/WindowsDisplayTopology.cpp#L250) | `EnumerateWindowsDisplayTopology` | 定义 | `DisplayTopologySnapshot EnumerateWindowsDisplayTopology()` | 实现 enumerate windows display topology 对应的业务或工具逻辑。 |

## `src/platform/win/WindowsDisplayTopology.h`

[打开源码](../src/platform/win/WindowsDisplayTopology.h) · **文件作用：** 声明 windows display topology 相关类型、接口、配置和成员状态。

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L14](../src/platform/win/WindowsDisplayTopology.h#L14) | `EnumerateWindowsDisplayTopology` | 声明 | `DisplayTopologySnapshot EnumerateWindowsDisplayTopology()` | Enumerates the physical-pixel Windows desktop. The returned layoutVersion is a deterministic fingerprint of display identity, position, mode, DPI and rotation, and therefore cha... |

## `src/platform/win/WindowsFileTransferService.cpp`

[打开源码](../src/platform/win/WindowsFileTransferService.cpp) · **文件作用：** 实现 windows file transfer service 相关函数与文件级辅助逻辑。

### 类型

| 行 | 类型 | 种类 | 作用 |
|---:|---|---|---|
| [L23](../src/platform/win/WindowsFileTransferService.cpp#L23) | `HandleCloser` | struct | 定义 HandleCloser 的 struct 类型和相关状态。 |
| [L133](../src/platform/win/WindowsFileTransferService.cpp#L133) | `WindowsFileTransferService::Impl` | struct | 定义 WindowsFileTransferService::Impl 的 struct 类型和相关状态。 |
| [L134](../src/platform/win/WindowsFileTransferService.cpp#L134) | `OpenSource` | struct | 定义 OpenSource 的 struct 类型和相关状态。 |
| [L140](../src/platform/win/WindowsFileTransferService.cpp#L140) | `OpenDestination` | struct | 定义 OpenDestination 的 struct 类型和相关状态。 |

### 成员与文件级变量

| 行 | 变量 | 声明 | 作用 |
|---:|---|---|---|
| [L34](../src/platform/win/WindowsFileTransferService.cpp#L34) | `kDestinationOpenRetryCount` | `constexpr int kDestinationOpenRetryCount = 5;` | 定义 destination open retry count 的编译期常量或产品边界。 |

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L24](../src/platform/win/WindowsFileTransferService.cpp#L24) | `operator` | 定义 | `void operator()(void* handle) const` | 实现 operator 对应的业务或工具逻辑。 |
| [L37](../src/platform/win/WindowsFileTransferService.cpp#L37) | `SetError` | 定义 | `void SetError(std::string* error, std::string message)` | 更新或应用 set error 相关逻辑。 |
| [L44](../src/platform/win/WindowsFileTransferService.cpp#L44) | `WindowsError` | 定义 | `std::string WindowsError(const char* operation, DWORD code)` | 实现 windows error 对应的业务或工具逻辑。 |
| [L50](../src/platform/win/WindowsFileTransferService.cpp#L50) | `WideToUtf8` | 定义 | `std::string WideToUtf8(const std::wstring& value)` | 实现 wide to utf8 对应的业务或工具逻辑。 |
| [L71](../src/platform/win/WindowsFileTransferService.cpp#L71) | `Utf8ToWide` | 定义 | `std::wstring Utf8ToWide(const std::string& value)` | 实现 utf8 to wide 对应的业务或工具逻辑。 |
| [L91](../src/platform/win/WindowsFileTransferService.cpp#L91) | `IsReservedWindowsFileName` | 定义 | `bool IsReservedWindowsFileName(const std::wstring& fileName)` | 判断 is reserved windows file name 相关逻辑。 |
| [L110](../src/platform/win/WindowsFileTransferService.cpp#L110) | `GetFileSize` | 定义 | `bool GetFileSize(HANDLE file, std::uint64_t* size)` | 查询并返回 get file size 相关逻辑。 |
| [L120](../src/platform/win/WindowsFileTransferService.cpp#L120) | `SeekFile` | 定义 | `bool SeekFile(HANDLE file, std::uint64_t offset)` | 实现 seek file 对应的业务或工具逻辑。 |
| [L149](../src/platform/win/WindowsFileTransferService.cpp#L149) | `WindowsFileTransferService::WindowsFileTransferService` | 定义 | `WindowsFileTransferService::WindowsFileTransferService() : impl_(std::make_unique<Impl>()) {}` | 构造并初始化 WindowsFileTransferService 实例。 |
| [L155](../src/platform/win/WindowsFileTransferService.cpp#L155) | `WindowsFileTransferService::InspectSource` | 定义 | `bool WindowsFileTransferService::InspectSource( const std::filesystem::path& path, FileTransferSourceInfo* output, std::string* error) const` | 实现 inspect source 对应的业务或工具逻辑。 |
| [L196](../src/platform/win/WindowsFileTransferService.cpp#L196) | `WindowsFileTransferService::ReadChunk` | 定义 | `bool WindowsFileTransferService::ReadChunk( const std::filesystem::path& path, std::uint64_t offset, std::size_t maximumBytes, std::vector<std::uint8_t>* output, std::string* error)` | 读取或恢复 read chunk 相关逻辑。 |
| [L259](../src/platform/win/WindowsFileTransferService.cpp#L259) | `WindowsFileTransferService::CloseSource` | 定义 | `void WindowsFileTransferService::CloseSource( const std::filesystem::path& path)` | 关闭并清理 close source 相关逻辑。 |
| [L267](../src/platform/win/WindowsFileTransferService.cpp#L267) | `WindowsFileTransferService::PrepareDestination` | 定义 | `bool WindowsFileTransferService::PrepareDestination( const std::filesystem::path& directory, const std::string& fileName, const std::string& transferId, std::uint64_t expectedSize, bool replaceExisting, FileTransferDe...` | 实现 prepare destination 对应的业务或工具逻辑。 |
| [L338](../src/platform/win/WindowsFileTransferService.cpp#L338) | `WindowsFileTransferService::AppendChunk` | 定义 | `bool WindowsFileTransferService::AppendChunk( const std::filesystem::path& temporaryPath, std::uint64_t expectedOffset, std::span<const std::uint8_t> bytes, std::string* error)` | 实现 append chunk 对应的业务或工具逻辑。 |
| [L416](../src/platform/win/WindowsFileTransferService.cpp#L416) | `WindowsFileTransferService::VerifyAndCommit` | 定义 | `bool WindowsFileTransferService::VerifyAndCommit( const FileTransferDestination& destination, std::uint64_t expectedSize, const std::array<std::uint8_t, 32>& expectedSha256, std::string* error)` | 校验 verify and commit 相关逻辑。 |
| [L456](../src/platform/win/WindowsFileTransferService.cpp#L456) | `WindowsFileTransferService::RemovePartial` | 定义 | `void WindowsFileTransferService::RemovePartial( const std::filesystem::path& temporaryPath)` | 重置或移除 remove partial 相关逻辑。 |
| [L467](../src/platform/win/WindowsFileTransferService.cpp#L467) | `WindowsFileTransferService::CloseDestination` | 定义 | `void WindowsFileTransferService::CloseDestination( const std::filesystem::path& temporaryPath)` | 关闭并清理 close destination 相关逻辑。 |
| [L475](../src/platform/win/WindowsFileTransferService.cpp#L475) | `WindowsFileTransferService::CalculateSha256` | 定义 | `bool WindowsFileTransferService::CalculateSha256( const std::filesystem::path& path, std::array<std::uint8_t, 32>* digest, std::string* error) const` | 计算或采集 calculate sha256 相关逻辑。 |

## `src/platform/win/WindowsFileTransferService.h`

[打开源码](../src/platform/win/WindowsFileTransferService.h) · **文件作用：** 声明 windows file transfer service 相关类型、接口、配置和成员状态。

### 类型

| 行 | 类型 | 种类 | 作用 |
|---:|---|---|---|
| [L17](../src/platform/win/WindowsFileTransferService.h#L17) | `FileTransferSourceInfo` | struct | 定义 FileTransferSourceInfo 的 struct 类型和相关状态。 |
| [L24](../src/platform/win/WindowsFileTransferService.h#L24) | `FileTransferDestination` | struct | 定义 FileTransferDestination 的 struct 类型和相关状态。 |
| [L33](../src/platform/win/WindowsFileTransferService.h#L33) | `WindowsFileTransferService` | class | Performs blocking file operations. Callers must run these methods on a file worker thread, never on a Qt or WebRTC callback thread. |
| [L67](../src/platform/win/WindowsFileTransferService.h#L67) | `Impl` | struct | 定义 Impl 的 struct 类型和相关状态。 |

### 成员与文件级变量

| 行 | 变量 | 声明 | 作用 |
|---:|---|---|---|
| [L18](../src/platform/win/WindowsFileTransferService.h#L18) | `path` | `std::filesystem::path path;` | 保存路径、地址或显示名称：path。 |
| [L19](../src/platform/win/WindowsFileTransferService.h#L19) | `fileName` | `std::string fileName;` | 保存路径、地址或显示名称：file name。 |
| [L20](../src/platform/win/WindowsFileTransferService.h#L20) | `fileSize` | `std::uint64_t fileSize = 0;` | 保存计数、尺寸或速率指标：file size。 |
| [L21](../src/platform/win/WindowsFileTransferService.h#L21) | `sha256` | `std::array<std::uint8_t, 32> sha256{};` | 保存 sha256 相关配置或运行状态。 |
| [L25](../src/platform/win/WindowsFileTransferService.h#L25) | `finalPath` | `std::filesystem::path finalPath;` | 保存路径、地址或显示名称：final path。 |
| [L26](../src/platform/win/WindowsFileTransferService.h#L26) | `temporaryPath` | `std::filesystem::path temporaryPath;` | 保存路径、地址或显示名称：temporary path。 |
| [L27](../src/platform/win/WindowsFileTransferService.h#L27) | `resumeOffset` | `std::uint64_t resumeOffset = 0;` | 保存 resume offset 相关配置或运行状态。 |
| [L28](../src/platform/win/WindowsFileTransferService.h#L28) | `replaceExisting` | `bool replaceExisting = false;` | 保存 replace existing 相关配置或运行状态。 |
| [L67](../src/platform/win/WindowsFileTransferService.h#L67) | `Impl` | `struct Impl;` | 保存 impl 相关配置或运行状态。 |
| [L71](../src/platform/win/WindowsFileTransferService.h#L71) | `impl_` | `std::unique_ptr<Impl> impl_;` | 保存 impl 相关配置或运行状态。 |

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L35](../src/platform/win/WindowsFileTransferService.h#L35) | `WindowsFileTransferService` | 声明 | `WindowsFileTransferService()` | 实现 windows file transfer service 对应的业务或工具逻辑。 |
| [L36](../src/platform/win/WindowsFileTransferService.h#L36) | `~WindowsFileTransferService` | 声明 | `~WindowsFileTransferService()` | 停止相关活动并释放 WindowsFileTransferService 实例拥有的资源。 |
| [L38](../src/platform/win/WindowsFileTransferService.h#L38) | `InspectSource` | 声明 | `bool InspectSource(const std::filesystem::path& path, FileTransferSourceInfo* output, std::string* error) const` | 实现 inspect source 对应的业务或工具逻辑。 |
| [L41](../src/platform/win/WindowsFileTransferService.h#L41) | `ReadChunk` | 声明 | `bool ReadChunk(const std::filesystem::path& path, std::uint64_t offset, std::size_t maximumBytes, std::vector<std::uint8_t>* output, std::string* error)` | 读取或恢复 read chunk 相关逻辑。 |
| [L46](../src/platform/win/WindowsFileTransferService.h#L46) | `CloseSource` | 声明 | `void CloseSource(const std::filesystem::path& path)` | 关闭并清理 close source 相关逻辑。 |
| [L47](../src/platform/win/WindowsFileTransferService.h#L47) | `PrepareDestination` | 声明 | `bool PrepareDestination(const std::filesystem::path& directory, const std::string& fileName, const std::string& transferId, std::uint64_t expectedSize, bool replaceExisting, FileTransferDestination* output, std::strin...` | 实现 prepare destination 对应的业务或工具逻辑。 |
| [L54](../src/platform/win/WindowsFileTransferService.h#L54) | `AppendChunk` | 声明 | `bool AppendChunk(const std::filesystem::path& temporaryPath, std::uint64_t expectedOffset, std::span<const std::uint8_t> bytes, std::string* error)` | 实现 append chunk 对应的业务或工具逻辑。 |
| [L58](../src/platform/win/WindowsFileTransferService.h#L58) | `VerifyAndCommit` | 声明 | `bool VerifyAndCommit( const FileTransferDestination& destination, std::uint64_t expectedSize, const std::array<std::uint8_t, 32>& expectedSha256, std::string* error)` | 校验 verify and commit 相关逻辑。 |
| [L63](../src/platform/win/WindowsFileTransferService.h#L63) | `CloseDestination` | 声明 | `void CloseDestination(const std::filesystem::path& temporaryPath)` | 关闭并清理 close destination 相关逻辑。 |
| [L64](../src/platform/win/WindowsFileTransferService.h#L64) | `RemovePartial` | 声明 | `void RemovePartial(const std::filesystem::path& temporaryPath)` | 重置或移除 remove partial 相关逻辑。 |
| [L68](../src/platform/win/WindowsFileTransferService.h#L68) | `CalculateSha256` | 声明 | `bool CalculateSha256(const std::filesystem::path& path, std::array<std::uint8_t, 32>* digest, std::string* error) const` | 计算或采集 calculate sha256 相关逻辑。 |

## `src/platform/win/WindowsHardwareFingerprint.cpp`

[打开源码](../src/platform/win/WindowsHardwareFingerprint.cpp) · **文件作用：** 实现 windows hardware fingerprint 相关函数与文件级辅助逻辑。

### 类型

| 行 | 类型 | 种类 | 作用 |
|---:|---|---|---|
| [L100](../src/platform/win/WindowsHardwareFingerprint.cpp#L100) | `AdapterProfile` | struct | 定义 AdapterProfile 的 struct 类型和相关状态。 |

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L22](../src/platform/win/WindowsHardwareFingerprint.cpp#L22) | `ReadWindowsVersion` | 定义 | `RTL_OSVERSIONINFOW ReadWindowsVersion()` | 读取或恢复 read windows version 相关逻辑。 |
| [L37](../src/platform/win/WindowsHardwareFingerprint.cpp#L37) | `Utf8FromWide` | 定义 | `std::string Utf8FromWide(const wchar_t* text)` | 实现 utf8 from wide 对应的业务或工具逻辑。 |
| [L55](../src/platform/win/WindowsHardwareFingerprint.cpp#L55) | `NativeArchitectureName` | 定义 | `std::string NativeArchitectureName(WORD architecture)` | 实现 native architecture name 对应的业务或工具逻辑。 |
| [L73](../src/platform/win/WindowsHardwareFingerprint.cpp#L73) | `GraphicsVendorName` | 定义 | `std::string GraphicsVendorName(std::uint32_t vendorId)` | 实现 graphics vendor name 对应的业务或工具逻辑。 |
| [L90](../src/platform/win/WindowsHardwareFingerprint.cpp#L90) | `DriverVersionText` | 定义 | `std::string DriverVersionText(const LARGE_INTEGER& version)` | 实现 driver version text 对应的业务或工具逻辑。 |
| [L106](../src/platform/win/WindowsHardwareFingerprint.cpp#L106) | `ReadAdapterProfile` | 定义 | `std::optional<AdapterProfile> ReadAdapterProfile(IDXGIAdapter1* adapter)` | 读取或恢复 read adapter profile 相关逻辑。 |
| [L172](../src/platform/win/WindowsHardwareFingerprint.cpp#L172) | `QueryWindowsCompatibilityProfile` | 定义 | `WindowsCompatibilityProfile QueryWindowsCompatibilityProfile()` | 发起请求或查询 query windows compatibility profile 相关逻辑。 |
| [L259](../src/platform/win/WindowsHardwareFingerprint.cpp#L259) | `BuildWindowsHardwareFingerprint` | 定义 | `std::string BuildWindowsHardwareFingerprint()` | 创建或初始化 build windows hardware fingerprint 相关逻辑。 |

## `src/platform/win/WindowsHardwareFingerprint.h`

[打开源码](../src/platform/win/WindowsHardwareFingerprint.h) · **文件作用：** 声明 windows hardware fingerprint 相关类型、接口、配置和成员状态。

### 类型

| 行 | 类型 | 种类 | 作用 |
|---:|---|---|---|
| [L11](../src/platform/win/WindowsHardwareFingerprint.h#L11) | `WindowsCompatibilityProfile` | struct | 定义 WindowsCompatibilityProfile 的 struct 类型和相关状态。 |

### 成员与文件级变量

| 行 | 变量 | 声明 | 作用 |
|---:|---|---|---|
| [L12](../src/platform/win/WindowsHardwareFingerprint.h#L12) | `hardwareFingerprint` | `std::string hardwareFingerprint;` | 保存 hardware fingerprint 相关配置或运行状态。 |
| [L13](../src/platform/win/WindowsHardwareFingerprint.h#L13) | `operatingSystem` | `std::string operatingSystem;` | 保存 operating system 相关配置或运行状态。 |
| [L14](../src/platform/win/WindowsHardwareFingerprint.h#L14) | `nativeArchitecture` | `std::string nativeArchitecture;` | 保存 native architecture 相关配置或运行状态。 |
| [L15](../src/platform/win/WindowsHardwareFingerprint.h#L15) | `remoteSession` | `bool remoteSession = false;` | 保存 remote session 相关配置或运行状态。 |
| [L16](../src/platform/win/WindowsHardwareFingerprint.h#L16) | `graphicsAdapters` | `std::vector<std::string> graphicsAdapters;` | 保存 graphics adapters 相关配置或运行状态。 |
| [L17](../src/platform/win/WindowsHardwareFingerprint.h#L17) | `graphicsEnumerationError` | `std::string graphicsEnumerationError;` | 保存最近错误或失败原因：graphics enumeration error。 |

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L22](../src/platform/win/WindowsHardwareFingerprint.h#L22) | `QueryWindowsCompatibilityProfile` | 声明 | `WindowsCompatibilityProfile QueryWindowsCompatibilityProfile()` | Describes the current Windows/GPU environment using stable, copyable text. This is diagnostic metadata only; it never owns D3D/MFT/COM objects. |
| [L27](../src/platform/win/WindowsHardwareFingerprint.h#L27) | `BuildWindowsHardwareFingerprint` | 声明 | `std::string BuildWindowsHardwareFingerprint()` | Returns a stable, inexpensive identity for the current Windows media environment. It intentionally contains descriptors only; D3D/MFT/COM objects are never cached across applica... |

## `src/platform/win/WindowsInputExecutor.cpp`

[打开源码](../src/platform/win/WindowsInputExecutor.cpp) · **文件作用：** 实现 windows input executor 相关函数与文件级辅助逻辑。

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L21](../src/platform/win/WindowsInputExecutor.cpp#L21) | `ScaleNormalized` | 定义 | `LONG ScaleNormalized(std::uint16_t value, LONG extent)` | 转换或缩放 scale normalized 相关逻辑。 |
| [L31](../src/platform/win/WindowsInputExecutor.cpp#L31) | `ToSendInputAbsolute` | 定义 | `LONG ToSendInputAbsolute(LONG pixel, LONG origin, LONG extent)` | 实现 to send input absolute 对应的业务或工具逻辑。 |
| [L42](../src/platform/win/WindowsInputExecutor.cpp#L42) | `FillAbsoluteMousePosition` | 定义 | `bool FillAbsoluteMousePosition(const RemoteInputEvent& event, const DisplayDescriptor& display, std::uint64_t layoutVersion, MOUSEINPUT* mouse)` | 实现 fill absolute mouse position 对应的业务或工具逻辑。 |
| [L75](../src/platform/win/WindowsInputExecutor.cpp#L75) | `MouseButtonFlag` | 定义 | `std::optional<DWORD> MouseButtonFlag(RemoteMouseButton button, bool pressed)` | 实现 mouse button flag 对应的业务或工具逻辑。 |
| [L92](../src/platform/win/WindowsInputExecutor.cpp#L92) | `XButtonData` | 定义 | `DWORD XButtonData(RemoteMouseButton button)` | 实现 x button data 对应的业务或工具逻辑。 |
| [L97](../src/platform/win/WindowsInputExecutor.cpp#L97) | `SendOneInput` | 定义 | `bool SendOneInput(INPUT input)` | 发送或发布 send one input 相关逻辑。 |
| [L102](../src/platform/win/WindowsInputExecutor.cpp#L102) | `MakeKeyInput` | 定义 | `INPUT MakeKeyInput(const RemoteInputEvent& event, bool pressed)` | 创建或初始化 make key input 相关逻辑。 |
| [L121](../src/platform/win/WindowsInputExecutor.cpp#L121) | `KeyIdentity` | 定义 | `std::uint64_t KeyIdentity(const RemoteInputEvent& event)` | 实现 key identity 对应的业务或工具逻辑。 |
| [L130](../src/platform/win/WindowsInputExecutor.cpp#L130) | `WindowsInputExecutor::~WindowsInputExecutor` | 定义 | `WindowsInputExecutor::~WindowsInputExecutor()` | 停止相关活动并释放 WindowsInputExecutor 实例拥有的资源。 |
| [L135](../src/platform/win/WindowsInputExecutor.cpp#L135) | `WindowsInputExecutor::OnRemoteInput` | 定义 | `void WindowsInputExecutor::OnRemoteInput(const RemoteInputEvent& event)` | 接收并处理 on remote input 相关逻辑。 |
| [L198](../src/platform/win/WindowsInputExecutor.cpp#L198) | `WindowsInputExecutor::ReleaseAllRemoteInputs` | 定义 | `void WindowsInputExecutor::ReleaseAllRemoteInputs()` | 释放或取消 release all remote inputs 相关逻辑。 |
| [L204](../src/platform/win/WindowsInputExecutor.cpp#L204) | `WindowsInputExecutor::SetActiveDisplay` | 定义 | `void WindowsInputExecutor::SetActiveDisplay( const DisplayTopologySnapshot& topology, const std::string& stableDisplayKey)` | 更新或应用 set active display 相关逻辑。 |
| [L226](../src/platform/win/WindowsInputExecutor.cpp#L226) | `WindowsInputExecutor::ClearActiveDisplay` | 定义 | `void WindowsInputExecutor::ClearActiveDisplay()` | 重置或移除 clear active display 相关逻辑。 |
| [L234](../src/platform/win/WindowsInputExecutor.cpp#L234) | `WindowsInputExecutor::ReleaseAllRemoteInputsLocked` | 定义 | `void WindowsInputExecutor::ReleaseAllRemoteInputsLocked()` | 释放或取消 release all remote inputs locked 相关逻辑。 |
| [L258](../src/platform/win/WindowsInputExecutor.cpp#L258) | `WindowsInputExecutor::ApplyMouseMove` | 定义 | `bool WindowsInputExecutor::ApplyMouseMove(const RemoteInputEvent& event)` | 更新或应用 apply mouse move 相关逻辑。 |
| [L263](../src/platform/win/WindowsInputExecutor.cpp#L263) | `WindowsInputExecutor::ApplyMouseButton` | 定义 | `bool WindowsInputExecutor::ApplyMouseButton(const RemoteInputEvent& event)` | 更新或应用 apply mouse button 相关逻辑。 |
| [L268](../src/platform/win/WindowsInputExecutor.cpp#L268) | `WindowsInputExecutor::ApplyMouseWheel` | 定义 | `bool WindowsInputExecutor::ApplyMouseWheel(const RemoteInputEvent& event)` | 更新或应用 apply mouse wheel 相关逻辑。 |
| [L296](../src/platform/win/WindowsInputExecutor.cpp#L296) | `WindowsInputExecutor::ApplyPointerPositionAndState` | 定义 | `bool WindowsInputExecutor::ApplyPointerPositionAndState( const RemoteInputEvent& event)` | 更新或应用 apply pointer position and state 相关逻辑。 |
| [L360](../src/platform/win/WindowsInputExecutor.cpp#L360) | `WindowsInputExecutor::ApplyKey` | 定义 | `bool WindowsInputExecutor::ApplyKey(const RemoteInputEvent& event)` | 更新或应用 apply key 相关逻辑。 |

## `src/platform/win/WindowsInputExecutor.h`

[打开源码](../src/platform/win/WindowsInputExecutor.h) · **文件作用：** 声明 windows input executor 相关类型、接口、配置和成员状态。

### 类型

| 行 | 类型 | 种类 | 作用 |
|---:|---|---|---|
| [L19](../src/platform/win/WindowsInputExecutor.h#L19) | `WindowsInputExecutor` | class | Applies already-authorized remote input to the interactive Windows desktop. The session engine validates room identity, control grants and sequencing before invoking this sink. ... |

### 成员与文件级变量

| 行 | 变量 | 声明 | 作用 |
|---:|---|---|---|
| [L41](../src/platform/win/WindowsInputExecutor.h#L41) | `mutex_` | `std::mutex mutex_;` | 保护跨线程共享状态：mutex。 |
| [L42](../src/platform/win/WindowsInputExecutor.h#L42) | `pressedMouseButtons_` | `std::unordered_set<RemoteMouseButton> pressedMouseButtons_;` | 保存 pressed mouse buttons 相关配置或运行状态。 |
| [L43](../src/platform/win/WindowsInputExecutor.h#L43) | `pressedKeys_` | `std::unordered_map<std::uint64_t, RemoteInputEvent> pressedKeys_;` | 保存 pressed keys 相关配置或运行状态。 |
| [L44](../src/platform/win/WindowsInputExecutor.h#L44) | `activeDisplay_` | `DisplayDescriptor activeDisplay_;` | 保存 active display 相关配置或运行状态。 |
| [L45](../src/platform/win/WindowsInputExecutor.h#L45) | `activeLayoutVersion_` | `std::uint64_t activeLayoutVersion_ = 0;` | 保存 active layout version 相关配置或运行状态。 |

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L21](../src/platform/win/WindowsInputExecutor.h#L21) | `WindowsInputExecutor` | 声明 | `WindowsInputExecutor() = default` | 实现 windows input executor 对应的业务或工具逻辑。 |
| [L22](../src/platform/win/WindowsInputExecutor.h#L22) | `~WindowsInputExecutor` | 声明 | `~WindowsInputExecutor() override` | 停止相关活动并释放 WindowsInputExecutor 实例拥有的资源。 |
| [L24](../src/platform/win/WindowsInputExecutor.h#L24) | `WindowsInputExecutor` | 声明 | `WindowsInputExecutor(const WindowsInputExecutor&) = delete` | 实现 windows input executor 对应的业务或工具逻辑。 |
| [L27](../src/platform/win/WindowsInputExecutor.h#L27) | `OnRemoteInput` | 声明 | `void OnRemoteInput(const RemoteInputEvent& event) override` | 接收并处理 on remote input 相关逻辑。 |
| [L28](../src/platform/win/WindowsInputExecutor.h#L28) | `ReleaseAllRemoteInputs` | 声明 | `void ReleaseAllRemoteInputs() override` | 释放或取消 release all remote inputs 相关逻辑。 |
| [L29](../src/platform/win/WindowsInputExecutor.h#L29) | `SetActiveDisplay` | 声明 | `void SetActiveDisplay(const DisplayTopologySnapshot& topology, const std::string& stableDisplayKey)` | 更新或应用 set active display 相关逻辑。 |
| [L31](../src/platform/win/WindowsInputExecutor.h#L31) | `ClearActiveDisplay` | 声明 | `void ClearActiveDisplay()` | 重置或移除 clear active display 相关逻辑。 |
| [L34](../src/platform/win/WindowsInputExecutor.h#L34) | `ApplyMouseMove` | 声明 | `bool ApplyMouseMove(const RemoteInputEvent& event)` | 更新或应用 apply mouse move 相关逻辑。 |
| [L35](../src/platform/win/WindowsInputExecutor.h#L35) | `ApplyMouseButton` | 声明 | `bool ApplyMouseButton(const RemoteInputEvent& event)` | 更新或应用 apply mouse button 相关逻辑。 |
| [L36](../src/platform/win/WindowsInputExecutor.h#L36) | `ApplyMouseWheel` | 声明 | `bool ApplyMouseWheel(const RemoteInputEvent& event)` | 更新或应用 apply mouse wheel 相关逻辑。 |
| [L37](../src/platform/win/WindowsInputExecutor.h#L37) | `ApplyKey` | 声明 | `bool ApplyKey(const RemoteInputEvent& event)` | 更新或应用 apply key 相关逻辑。 |
| [L38](../src/platform/win/WindowsInputExecutor.h#L38) | `ApplyPointerPositionAndState` | 声明 | `bool ApplyPointerPositionAndState(const RemoteInputEvent& event)` | 更新或应用 apply pointer position and state 相关逻辑。 |
| [L39](../src/platform/win/WindowsInputExecutor.h#L39) | `ReleaseAllRemoteInputsLocked` | 声明 | `void ReleaseAllRemoteInputsLocked()` | 释放或取消 release all remote inputs locked 相关逻辑。 |

## `src/platform/win/WindowsVideoCapabilityProbe.cpp`

[打开源码](../src/platform/win/WindowsVideoCapabilityProbe.cpp) · **文件作用：** 实现 windows video capability probe 相关函数与文件级辅助逻辑。

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L23](../src/platform/win/WindowsVideoCapabilityProbe.cpp#L23) | `HResultMessage` | 定义 | `std::string HResultMessage(const char* operation, HRESULT result)` | 实现 h result message 对应的业务或工具逻辑。 |
| [L31](../src/platform/win/WindowsVideoCapabilityProbe.cpp#L31) | `ProbeD3D11` | 定义 | `void ProbeD3D11(WindowsVideoCapabilityReport& report)` | 实现 probe d3 d11 对应的业务或工具逻辑。 |
| [L102](../src/platform/win/WindowsVideoCapabilityProbe.cpp#L102) | `ProbeH264HardwareDecoders` | 定义 | `void ProbeH264HardwareDecoders(WindowsVideoCapabilityReport& report)` | 实现 probe h264 hardware decoders 对应的业务或工具逻辑。 |
| [L197](../src/platform/win/WindowsVideoCapabilityProbe.cpp#L197) | `ProbeWindowsVideoCapabilities` | 定义 | `WindowsVideoCapabilityReport ProbeWindowsVideoCapabilities()` | 实现 probe windows video capabilities 对应的业务或工具逻辑。 |

## `src/platform/win/WindowsVideoCapabilityProbe.h`

[打开源码](../src/platform/win/WindowsVideoCapabilityProbe.h) · **文件作用：** 声明 windows video capability probe 相关类型、接口、配置和成员状态。

### 类型

| 行 | 类型 | 种类 | 作用 |
|---:|---|---|---|
| [L12](../src/platform/win/WindowsVideoCapabilityProbe.h#L12) | `WindowsVideoCapabilityReport` | struct | 定义 WindowsVideoCapabilityReport 的 struct 类型和相关状态。 |

### 成员与文件级变量

| 行 | 变量 | 声明 | 作用 |
|---:|---|---|---|
| [L13](../src/platform/win/WindowsVideoCapabilityProbe.h#L13) | `d3d11DeviceCreated` | `bool d3d11DeviceCreated = false;` | 保存 d3d11 device created 相关配置或运行状态。 |
| [L14](../src/platform/win/WindowsVideoCapabilityProbe.h#L14) | `d3d11VideoDeviceAvailable` | `bool d3d11VideoDeviceAvailable = false;` | 保存能力或开关状态：d3d11 video device available。 |
| [L15](../src/platform/win/WindowsVideoCapabilityProbe.h#L15) | `nv12TextureSupported` | `bool nv12TextureSupported = false;` | 保存 nv12 texture supported 相关配置或运行状态。 |
| [L16](../src/platform/win/WindowsVideoCapabilityProbe.h#L16) | `featureLevel` | `uint32_t featureLevel = 0;` | 保存 feature level 相关配置或运行状态。 |
| [L17](../src/platform/win/WindowsVideoCapabilityProbe.h#L17) | `adapterLuidLowPart` | `uint32_t adapterLuidLowPart = 0;` | 保存 adapter luid low part 相关配置或运行状态。 |
| [L18](../src/platform/win/WindowsVideoCapabilityProbe.h#L18) | `adapterLuidHighPart` | `int32_t adapterLuidHighPart = 0;` | 保存 adapter luid high part 相关配置或运行状态。 |
| [L19](../src/platform/win/WindowsVideoCapabilityProbe.h#L19) | `adapterName` | `std::wstring adapterName;` | 保存路径、地址或显示名称：adapter name。 |
| [L21](../src/platform/win/WindowsVideoCapabilityProbe.h#L21) | `h264DecoderMftCount` | `uint32_t h264DecoderMftCount = 0;` | 保存计数、尺寸或速率指标：h264 decoder mft count。 |
| [L22](../src/platform/win/WindowsVideoCapabilityProbe.h#L22) | `h264HardwareRegisteredMftCount` | `uint32_t h264HardwareRegisteredMftCount = 0;` | 保存计数、尺寸或速率指标：h264 hardware registered mft count。 |
| [L23](../src/platform/win/WindowsVideoCapabilityProbe.h#L23) | `h264D3D11AwareDecoderCount` | `uint32_t h264D3D11AwareDecoderCount = 0;` | 保存计数、尺寸或速率指标：h264 d3 d11 aware decoder count。 |
| [L24](../src/platform/win/WindowsVideoCapabilityProbe.h#L24) | `h264DecoderMftNames` | `std::vector<std::wstring> h264DecoderMftNames;` | 保存 h264 decoder mft names 相关配置或运行状态。 |
| [L25](../src/platform/win/WindowsVideoCapabilityProbe.h#L25) | `warnings` | `std::vector<std::string> warnings;` | 保存 warnings 相关配置或运行状态。 |

### 函数

| 行 | 函数 | 类型 | 签名 | 作用 |
|---:|---|---|---|---|
| [L28](../src/platform/win/WindowsVideoCapabilityProbe.h#L28) | `ProbeWindowsVideoCapabilities` | 声明 | `WindowsVideoCapabilityReport ProbeWindowsVideoCapabilities()` | 实现 probe windows video capabilities 对应的业务或工具逻辑。 |
