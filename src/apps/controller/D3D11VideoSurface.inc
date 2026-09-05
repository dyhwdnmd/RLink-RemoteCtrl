// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

        // Presents a decoder-owned NV12 texture through a swap chain created on the
        // decoder's D3D11 device. The canvas keeps the owning WebRTC frame alive while
        // Present() runs, so the active hardware decoder cannot recycle the surface.
        class D3D11VideoSurface final : public QWidget {
        public:
            struct PresentTiming {
                bool succeeded = false;
                std::uint64_t cpuPreparationUs = 0;
                std::uint64_t videoProcessorSubmitUs = 0;
                std::uint64_t presentCallUs = 0;
            };

            void SetCursorFrame(
                const RemoteCursorRenderState::Snapshot& snapshot,
                int sourceWidth, int sourceHeight)
            {
                cursorImage_ = snapshot.image;
                cursorHotspot_ = snapshot.hotspot;
                cursorPosition_ = snapshot.normalizedPosition;
                cursorShapeRevision_ = snapshot.shapeRevision;
                cursorVisible_ = snapshot.visible;
                cursorSourceWidth_ = sourceWidth;
                cursorSourceHeight_ = sourceHeight;
            }

            explicit D3D11VideoSurface(QWidget* parent = nullptr)
                : QWidget(parent)
            {
                setAttribute(Qt::WA_NativeWindow);
                setAttribute(Qt::WA_PaintOnScreen);
                setAttribute(Qt::WA_NoSystemBackground);
                setAttribute(Qt::WA_OpaquePaintEvent);
                setAttribute(Qt::WA_TransparentForMouseEvents);
                // QWidget APIs are confined to the Qt thread. The dedicated
                // presentation thread only uses this cached HWND with Win32,
                // DXGI and D3D11 calls.
                window_ = reinterpret_cast<HWND>(winId());
            }

        protected:
            bool nativeEvent(const QByteArray& eventType,
                             void* message,
                             qintptr* result) override
            {
#ifdef Q_OS_WIN
                Q_UNUSED(eventType);
                const auto* nativeMessage = static_cast<MSG*>(message);
                if (nativeMessage &&
                    nativeMessage->message == WM_NCHITTEST) {
                    // WA_TransparentForMouseEvents only affects Qt's widget
                    // dispatch. This surface owns a real child HWND, so its
                    // default HTCLIENT result otherwise swallows passive
                    // mouse moves and prevents the frameless parent from
                    // seeing resize-border hit tests.
                    *result = HTTRANSPARENT;
                    return true;
                }
#else
                Q_UNUSED(eventType);
                Q_UNUSED(message);
                Q_UNUSED(result);
#endif
                return QWidget::nativeEvent(eventType, message, result);
            }

        public:

            PresentTiming Present(D3D11NativeFrameBuffer* frame)
            {
                if (!frame || !frame->Device() || !frame->Texture() ||
                    frame->width() <= 0 || frame->height() <= 0) {
                    return {};
                }
                return PresentTexture(
                    frame->Device(), frame->Texture(),
                    frame->SubresourceIndex(),
                    frame->width(), frame->height(), DXGI_FORMAT_NV12);
            }

            PresentTiming PresentCpuBgra(const QImage& image)
            {
                if (image.isNull() || image.width() <= 0 ||
                    image.height() <= 0 ||
                    image.format() != QImage::Format_ARGB32 ||
                    !EnsureCpuDevice() ||
                    !EnsureCpuUploadTexture(
                        static_cast<UINT>(image.width()),
                        static_cast<UINT>(image.height()),
                        DXGI_FORMAT_B8G8R8A8_UNORM)) {
                    return {};
                }
                context_->UpdateSubresource(
                    cpuUploadTexture_.Get(), 0, nullptr,
                    image.constBits(),
                    static_cast<UINT>(image.bytesPerLine()), 0);
                return PresentTexture(
                    device_.Get(), cpuUploadTexture_.Get(), 0,
                    image.width(), image.height(),
                    DXGI_FORMAT_B8G8R8A8_UNORM);
            }

            PresentTiming PresentCpuNv12(const QImage& image)
            {
                // A software-decoded I420 frame is repacked by the conversion
                // worker into one tightly-described NV12 upload image.  The
                // QImage height is the storage height (Y + UV), so recover the
                // actual video height before creating the D3D11 texture.
                if (image.isNull() || image.width() <= 0 ||
                    image.height() <= 0 ||
                    image.format() != QImage::Format_Grayscale8 ||
                    image.height() % 3 != 0) {
                    return {};
                }
                const int videoHeight = image.height() * 2 / 3;
                if ((image.width() & 1) != 0 ||
                    (videoHeight & 1) != 0 ||
                    image.bytesPerLine() < image.width() ||
                    !EnsureCpuDevice() ||
                    !EnsureCpuUploadTexture(
                        static_cast<UINT>(image.width()),
                        static_cast<UINT>(videoHeight),
                        DXGI_FORMAT_NV12)) {
                    return {};
                }
                const UINT rowPitch = static_cast<UINT>(
                    image.bytesPerLine());
                context_->UpdateSubresource(
                    cpuUploadTexture_.Get(), 0, nullptr,
                    image.constBits(), rowPitch,
                    rowPitch * static_cast<UINT>(image.height()));
                return PresentTexture(
                    device_.Get(), cpuUploadTexture_.Get(), 0,
                    image.width(), videoHeight, DXGI_FORMAT_NV12);
            }

            PresentTiming PresentCpuI420(
                webrtc::I420BufferInterface* frame)
            {
                using Clock = std::chrono::steady_clock;
                PresentTiming timing;
                if (!frame || frame->width() <= 0 || frame->height() <= 0 ||
                    (frame->width() & 1) != 0 ||
                    (frame->height() & 1) != 0 ||
                    !EnsureCpuDevice()) {
                    return timing;
                }

                RECT client{};
                if (!window_ || !GetClientRect(window_, &client)) {
                    return timing;
                }
                const UINT outputWidth = static_cast<UINT>(
                    (std::max<LONG>)(client.right - client.left, 0));
                const UINT outputHeight = static_cast<UINT>(
                    (std::max<LONG>)(client.bottom - client.top, 0));
                const UINT sourceWidth = static_cast<UINT>(frame->width());
                const UINT sourceHeight = static_cast<UINT>(frame->height());
                if (outputWidth == 0 || outputHeight == 0 ||
                    !EnsureSwapChain(window_, outputWidth, outputHeight) ||
                    !EnsureI420ShaderPipeline() ||
                    !EnsureI420UploadTextures(sourceWidth, sourceHeight) ||
                    !EnsureI420BackBufferView()) {
                    return timing;
                }

                I420UploadSlot& slot =
                    i420UploadSlots_[i420UploadSlotIndex_++ %
                                     i420UploadSlots_.size()];
                const auto uploadStartedAt = Clock::now();
                if (!UploadI420Plane(slot.y, frame->DataY(),
                        frame->StrideY(), sourceWidth, sourceHeight) ||
                    !UploadI420Plane(slot.u, frame->DataU(),
                        frame->StrideU(), sourceWidth / 2,
                        sourceHeight / 2) ||
                    !UploadI420Plane(slot.v, frame->DataV(),
                        frame->StrideV(), sourceWidth / 2,
                        sourceHeight / 2)) {
                    return timing;
                }
                timing.cpuPreparationUs = static_cast<std::uint64_t>(
                    std::chrono::duration_cast<std::chrono::microseconds>(
                        Clock::now() - uploadStartedAt)
                        .count());

                const FLOAT clearColor[] = { 0.0f, 0.0f, 0.0f, 1.0f };
                context_->ClearRenderTargetView(
                    i420BackBufferView_.Get(), clearColor);
                ID3D11RenderTargetView* renderTarget =
                    i420BackBufferView_.Get();
                context_->OMSetRenderTargets(1, &renderTarget, nullptr);
                D3D11_VIEWPORT viewport{};
                viewport.Width = static_cast<FLOAT>(outputWidth);
                viewport.Height = static_cast<FLOAT>(outputHeight);
                viewport.MinDepth = 0.0f;
                viewport.MaxDepth = 1.0f;
                context_->RSSetViewports(1, &viewport);
                context_->RSSetState(i420RasterizerState_.Get());
                context_->IASetInputLayout(nullptr);
                context_->IASetPrimitiveTopology(
                    D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
                context_->VSSetShader(i420VertexShader_.Get(), nullptr, 0);
                context_->PSSetShader(i420PixelShader_.Get(), nullptr, 0);
                ID3D11ShaderResourceView* resources[] = {
                    slot.y.view.Get(), slot.u.view.Get(), slot.v.view.Get() };
                context_->PSSetShaderResources(0, 3, resources);
                ID3D11SamplerState* sampler = i420SamplerState_.Get();
                context_->PSSetSamplers(0, 1, &sampler);

                const auto drawStartedAt = Clock::now();
                context_->Draw(3, 0);
                timing.videoProcessorSubmitUs =
                    static_cast<std::uint64_t>(
                        std::chrono::duration_cast<std::chrono::microseconds>(
                            Clock::now() - drawStartedAt)
                            .count());
                ID3D11ShaderResourceView* emptyResources[] = {
                    nullptr, nullptr, nullptr };
                context_->PSSetShaderResources(0, 3, emptyResources);

                DrawCursorOverlay(i420BackBufferView_.Get(),
                    outputWidth, outputHeight);

                const auto presentStartedAt = Clock::now();
                const HRESULT result = swapChain_->Present(0, 0);
                timing.presentCallUs = static_cast<std::uint64_t>(
                    std::chrono::duration_cast<std::chrono::microseconds>(
                        Clock::now() - presentStartedAt)
                        .count());
                if (result == DXGI_ERROR_DEVICE_REMOVED ||
                    result == DXGI_ERROR_DEVICE_RESET) {
                    ResetDevice();
                    return timing;
                }
                timing.succeeded =
                    SUCCEEDED(result) || result == DXGI_STATUS_OCCLUDED;
                return timing;
            }

        protected:
            QPaintEngine* paintEngine() const override { return nullptr; }
            void paintEvent(QPaintEvent*) override {}

        private:
            struct I420PlaneTexture {
                ComPtr<ID3D11Texture2D> texture;
                ComPtr<ID3D11ShaderResourceView> view;
            };

            struct I420UploadSlot {
                I420PlaneTexture y;
                I420PlaneTexture u;
                I420PlaneTexture v;
            };

            struct CursorConstants {
                float left;
                float top;
                float right;
                float bottom;
            };

            bool EnsureCursorPipeline()
            {
                if (cursorVertexShader_ && cursorPixelShader_ &&
                    cursorSamplerState_ && cursorRasterizerState_ &&
                    cursorBlendState_ && cursorConstantBuffer_) {
                    return true;
                }
                static constexpr char shaderSource[] = R"(
Texture2D cursorTexture : register(t0);
SamplerState cursorSampler : register(s0);
cbuffer CursorRect : register(b0) {
    float4 cursorRect;
};

struct VertexOutput {
    float4 position : SV_POSITION;
    float2 uv : TEXCOORD0;
};

VertexOutput VSMain(uint vertexId : SV_VertexID) {
    VertexOutput output;
    float2 uv = float2((vertexId << 1) & 2, vertexId & 2);
    output.position = float4(
        lerp(cursorRect.x, cursorRect.z, uv.x),
        lerp(cursorRect.y, cursorRect.w, uv.y), 0.0f, 1.0f);
    output.uv = uv;
    return output;
}

float4 PSMain(VertexOutput input) : SV_TARGET {
    return cursorTexture.Sample(cursorSampler, input.uv);
}
)";
                constexpr UINT compileFlags = D3DCOMPILE_ENABLE_STRICTNESS |
                    D3DCOMPILE_OPTIMIZATION_LEVEL3;
                ComPtr<ID3DBlob> vertexByteCode;
                ComPtr<ID3DBlob> pixelByteCode;
                ComPtr<ID3DBlob> errors;
                if (FAILED(D3DCompile(shaderSource,
                        sizeof(shaderSource) - 1, "RemoteC Cursor Shader",
                        nullptr, nullptr, "VSMain", "vs_4_0", compileFlags,
                        0, &vertexByteCode, &errors)) ||
                    FAILED(D3DCompile(shaderSource,
                        sizeof(shaderSource) - 1, "RemoteC Cursor Shader",
                        nullptr, nullptr, "PSMain", "ps_4_0", compileFlags,
                        0, &pixelByteCode, &errors)) ||
                    FAILED(device_->CreateVertexShader(
                        vertexByteCode->GetBufferPointer(),
                        vertexByteCode->GetBufferSize(), nullptr,
                        &cursorVertexShader_)) ||
                    FAILED(device_->CreatePixelShader(
                        pixelByteCode->GetBufferPointer(),
                        pixelByteCode->GetBufferSize(), nullptr,
                        &cursorPixelShader_))) {
                    ResetCursorResources();
                    return false;
                }

                D3D11_SAMPLER_DESC sampler{};
                sampler.Filter = D3D11_FILTER_MIN_MAG_LINEAR_MIP_POINT;
                sampler.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
                sampler.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
                sampler.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
                sampler.MaxLOD = D3D11_FLOAT32_MAX;
                D3D11_RASTERIZER_DESC rasterizer{};
                rasterizer.FillMode = D3D11_FILL_SOLID;
                rasterizer.CullMode = D3D11_CULL_NONE;
                rasterizer.DepthClipEnable = TRUE;
                D3D11_BLEND_DESC blend{};
                blend.RenderTarget[0].BlendEnable = TRUE;
                blend.RenderTarget[0].SrcBlend = D3D11_BLEND_ONE;
                blend.RenderTarget[0].DestBlend =
                    D3D11_BLEND_INV_SRC_ALPHA;
                blend.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
                blend.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
                blend.RenderTarget[0].DestBlendAlpha =
                    D3D11_BLEND_INV_SRC_ALPHA;
                blend.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
                blend.RenderTarget[0].RenderTargetWriteMask =
                    D3D11_COLOR_WRITE_ENABLE_ALL;
                D3D11_BUFFER_DESC constants{};
                constants.ByteWidth = sizeof(CursorConstants);
                constants.Usage = D3D11_USAGE_DEFAULT;
                constants.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
                if (FAILED(device_->CreateSamplerState(
                        &sampler, &cursorSamplerState_)) ||
                    FAILED(device_->CreateRasterizerState(
                        &rasterizer, &cursorRasterizerState_)) ||
                    FAILED(device_->CreateBlendState(
                        &blend, &cursorBlendState_)) ||
                    FAILED(device_->CreateBuffer(
                        &constants, nullptr, &cursorConstantBuffer_))) {
                    ResetCursorResources();
                    return false;
                }
                return true;
            }

            bool UpdateCursorTexture()
            {
                if (cursorImage_.isNull() ||
                    cursorImage_.format() !=
                        QImage::Format_ARGB32_Premultiplied) {
                    return false;
                }
                if (cursorTexture_ && cursorTextureView_ &&
                    uploadedCursorShapeRevision_ == cursorShapeRevision_ &&
                    cursorTextureWidth_ ==
                        static_cast<UINT>(cursorImage_.width()) &&
                    cursorTextureHeight_ ==
                        static_cast<UINT>(cursorImage_.height())) {
                    return true;
                }
                cursorTexture_.Reset();
                cursorTextureView_.Reset();
                D3D11_TEXTURE2D_DESC description{};
                description.Width = static_cast<UINT>(cursorImage_.width());
                description.Height = static_cast<UINT>(cursorImage_.height());
                description.MipLevels = 1;
                description.ArraySize = 1;
                description.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
                description.SampleDesc.Count = 1;
                description.Usage = D3D11_USAGE_DEFAULT;
                description.BindFlags = D3D11_BIND_SHADER_RESOURCE;
                D3D11_SUBRESOURCE_DATA initial{};
                initial.pSysMem = cursorImage_.constBits();
                initial.SysMemPitch =
                    static_cast<UINT>(cursorImage_.bytesPerLine());
                if (FAILED(device_->CreateTexture2D(
                        &description, &initial, &cursorTexture_)) ||
                    FAILED(device_->CreateShaderResourceView(
                        cursorTexture_.Get(), nullptr,
                        &cursorTextureView_))) {
                    cursorTexture_.Reset();
                    cursorTextureView_.Reset();
                    return false;
                }
                cursorTextureWidth_ = description.Width;
                cursorTextureHeight_ = description.Height;
                uploadedCursorShapeRevision_ = cursorShapeRevision_;
                return true;
            }

            void DrawCursorOverlay(ID3D11RenderTargetView* renderTarget,
                                   UINT outputWidth, UINT outputHeight)
            {
                if (!cursorVisible_ || !renderTarget || outputWidth == 0 ||
                    outputHeight == 0 || cursorSourceWidth_ <= 0 ||
                    cursorSourceHeight_ <= 0 || !EnsureCursorPipeline() ||
                    !UpdateCursorTexture()) return;

                const float scaleX = static_cast<float>(outputWidth) /
                    cursorSourceWidth_;
                const float scaleY = static_cast<float>(outputHeight) /
                    cursorSourceHeight_;
                const float cursorWidth = (std::max)(
                    1.0f, cursorImage_.width() * scaleX);
                const float cursorHeight = (std::max)(
                    1.0f, cursorImage_.height() * scaleY);
                const float x = cursorPosition_.x() / 65535.0f *
                    (outputWidth - 1) - cursorHotspot_.x() * scaleX;
                const float y = cursorPosition_.y() / 65535.0f *
                    (outputHeight - 1) - cursorHotspot_.y() * scaleY;
                const CursorConstants constants{
                    2.0f * x / outputWidth - 1.0f,
                    1.0f - 2.0f * y / outputHeight,
                    2.0f * (x + cursorWidth) / outputWidth - 1.0f,
                    1.0f - 2.0f * (y + cursorHeight) / outputHeight };
                context_->UpdateSubresource(
                    cursorConstantBuffer_.Get(), 0, nullptr,
                    &constants, 0, 0);

                context_->OMSetRenderTargets(1, &renderTarget, nullptr);
                const FLOAT blendFactor[] = { 0, 0, 0, 0 };
                context_->OMSetBlendState(
                    cursorBlendState_.Get(), blendFactor, 0xffffffff);
                D3D11_VIEWPORT viewport{};
                viewport.Width = static_cast<FLOAT>(outputWidth);
                viewport.Height = static_cast<FLOAT>(outputHeight);
                viewport.MinDepth = 0.0f;
                viewport.MaxDepth = 1.0f;
                context_->RSSetViewports(1, &viewport);
                context_->RSSetState(cursorRasterizerState_.Get());
                context_->IASetInputLayout(nullptr);
                context_->IASetPrimitiveTopology(
                    D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
                context_->VSSetShader(cursorVertexShader_.Get(), nullptr, 0);
                ID3D11Buffer* constantBuffer = cursorConstantBuffer_.Get();
                context_->VSSetConstantBuffers(0, 1, &constantBuffer);
                context_->PSSetShader(cursorPixelShader_.Get(), nullptr, 0);
                ID3D11ShaderResourceView* textureView =
                    cursorTextureView_.Get();
                context_->PSSetShaderResources(0, 1, &textureView);
                ID3D11SamplerState* sampler = cursorSamplerState_.Get();
                context_->PSSetSamplers(0, 1, &sampler);
                context_->Draw(3, 0);
                ID3D11ShaderResourceView* empty = nullptr;
                context_->PSSetShaderResources(0, 1, &empty);
                context_->OMSetBlendState(nullptr, nullptr, 0xffffffff);
            }

            void ResetCursorResources()
            {
                cursorTextureView_.Reset();
                cursorTexture_.Reset();
                cursorConstantBuffer_.Reset();
                cursorBlendState_.Reset();
                cursorRasterizerState_.Reset();
                cursorSamplerState_.Reset();
                cursorPixelShader_.Reset();
                cursorVertexShader_.Reset();
                cursorTextureWidth_ = 0;
                cursorTextureHeight_ = 0;
                uploadedCursorShapeRevision_ = 0;
            }

            bool EnsureI420ShaderPipeline()
            {
                if (i420VertexShader_ && i420PixelShader_ &&
                    i420SamplerState_ && i420RasterizerState_) {
                    return true;
                }
                static constexpr char shaderSource[] = R"(
Texture2D yPlane : register(t0);
Texture2D uPlane : register(t1);
Texture2D vPlane : register(t2);
SamplerState linearSampler : register(s0);

struct VertexOutput {
    float4 position : SV_POSITION;
    float2 uv : TEXCOORD0;
};

VertexOutput VSMain(uint vertexId : SV_VertexID) {
    VertexOutput output;
    float2 uv = float2((vertexId << 1) & 2, vertexId & 2);
    output.position = float4(
        uv.x * 2.0f - 1.0f, 1.0f - uv.y * 2.0f, 0.0f, 1.0f);
    output.uv = uv;
    return output;
}

float4 PSMain(VertexOutput input) : SV_TARGET {
    float y = max(yPlane.Sample(linearSampler, input.uv).r -
                  (16.0f / 255.0f), 0.0f) * 1.164383f;
    float u = uPlane.Sample(linearSampler, input.uv).r - 0.5f;
    float v = vPlane.Sample(linearSampler, input.uv).r - 0.5f;
    float3 rgb;
    rgb.r = y + 1.792741f * v;
    rgb.g = y - 0.213249f * u - 0.532909f * v;
    rgb.b = y + 2.112402f * u;
    return float4(saturate(rgb), 1.0f);
}
)";
                constexpr UINT compileFlags = D3DCOMPILE_ENABLE_STRICTNESS |
                    D3DCOMPILE_OPTIMIZATION_LEVEL3;
                ComPtr<ID3DBlob> vertexByteCode;
                ComPtr<ID3DBlob> pixelByteCode;
                ComPtr<ID3DBlob> errors;
                if (FAILED(D3DCompile(shaderSource,
                        sizeof(shaderSource) - 1, "RemoteC I420 Shader",
                        nullptr, nullptr, "VSMain", "vs_4_0", compileFlags,
                        0, &vertexByteCode, &errors)) ||
                    FAILED(D3DCompile(shaderSource,
                        sizeof(shaderSource) - 1, "RemoteC I420 Shader",
                        nullptr, nullptr, "PSMain", "ps_4_0", compileFlags,
                        0, &pixelByteCode, &errors)) ||
                    FAILED(device_->CreateVertexShader(
                        vertexByteCode->GetBufferPointer(),
                        vertexByteCode->GetBufferSize(), nullptr,
                        &i420VertexShader_)) ||
                    FAILED(device_->CreatePixelShader(
                        pixelByteCode->GetBufferPointer(),
                        pixelByteCode->GetBufferSize(), nullptr,
                        &i420PixelShader_))) {
                    ResetI420ShaderResources();
                    return false;
                }

                D3D11_SAMPLER_DESC sampler{};
                sampler.Filter = D3D11_FILTER_MIN_MAG_LINEAR_MIP_POINT;
                sampler.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
                sampler.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
                sampler.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
                sampler.MaxLOD = D3D11_FLOAT32_MAX;
                if (FAILED(device_->CreateSamplerState(
                        &sampler, &i420SamplerState_))) {
                    ResetI420ShaderResources();
                    return false;
                }
                D3D11_RASTERIZER_DESC rasterizer{};
                rasterizer.FillMode = D3D11_FILL_SOLID;
                rasterizer.CullMode = D3D11_CULL_NONE;
                rasterizer.DepthClipEnable = TRUE;
                if (FAILED(device_->CreateRasterizerState(
                        &rasterizer, &i420RasterizerState_))) {
                    ResetI420ShaderResources();
                    return false;
                }
                return true;
            }

            bool CreateI420PlaneTexture(
                UINT width, UINT height, I420PlaneTexture* plane)
            {
                if (!plane || width == 0 || height == 0) {
                    return false;
                }
                D3D11_TEXTURE2D_DESC description{};
                description.Width = width;
                description.Height = height;
                description.MipLevels = 1;
                description.ArraySize = 1;
                description.Format = DXGI_FORMAT_R8_UNORM;
                description.SampleDesc.Count = 1;
                description.Usage = D3D11_USAGE_DYNAMIC;
                description.BindFlags = D3D11_BIND_SHADER_RESOURCE;
                description.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
                return SUCCEEDED(device_->CreateTexture2D(
                           &description, nullptr, &plane->texture)) &&
                    SUCCEEDED(device_->CreateShaderResourceView(
                        plane->texture.Get(), nullptr, &plane->view));
            }

            bool EnsureI420UploadTextures(UINT width, UINT height)
            {
                if (i420UploadWidth_ == width &&
                    i420UploadHeight_ == height &&
                    i420UploadSlots_[0].y.texture) {
                    return true;
                }
                ResetI420UploadTextures();
                for (auto& slot : i420UploadSlots_) {
                    if (!CreateI420PlaneTexture(
                            width, height, &slot.y) ||
                        !CreateI420PlaneTexture(
                            width / 2, height / 2, &slot.u) ||
                        !CreateI420PlaneTexture(
                            width / 2, height / 2, &slot.v)) {
                        ResetI420UploadTextures();
                        return false;
                    }
                }
                i420UploadWidth_ = width;
                i420UploadHeight_ = height;
                return true;
            }

            bool UploadI420Plane(I420PlaneTexture& plane,
                const std::uint8_t* source, int sourceStride,
                UINT width, UINT height)
            {
                if (!plane.texture || !source || sourceStride <= 0 ||
                    static_cast<UINT>(sourceStride) < width) {
                    return false;
                }
                D3D11_MAPPED_SUBRESOURCE mapped{};
                if (FAILED(context_->Map(plane.texture.Get(), 0,
                        D3D11_MAP_WRITE_DISCARD, 0, &mapped))) {
                    return false;
                }
                auto* destination = static_cast<std::uint8_t*>(mapped.pData);
                for (UINT row = 0; row < height; ++row) {
                    std::memcpy(destination + mapped.RowPitch * row,
                        source + static_cast<std::size_t>(sourceStride) * row,
                        width);
                }
                context_->Unmap(plane.texture.Get(), 0);
                return true;
            }

            bool EnsureI420BackBufferView()
            {
                if (i420BackBufferView_) {
                    return true;
                }
                ComPtr<ID3D11Texture2D> backBuffer;
                return SUCCEEDED(swapChain_->GetBuffer(
                           0, IID_PPV_ARGS(&backBuffer))) &&
                    SUCCEEDED(device_->CreateRenderTargetView(
                        backBuffer.Get(), nullptr, &i420BackBufferView_));
            }

            void ResetI420UploadTextures()
            {
                for (auto& slot : i420UploadSlots_) {
                    slot.y = {};
                    slot.u = {};
                    slot.v = {};
                }
                i420UploadWidth_ = 0;
                i420UploadHeight_ = 0;
                i420UploadSlotIndex_ = 0;
            }

            void ResetI420ShaderResources()
            {
                i420BackBufferView_.Reset();
                i420RasterizerState_.Reset();
                i420SamplerState_.Reset();
                i420PixelShader_.Reset();
                i420VertexShader_.Reset();
                ResetI420UploadTextures();
            }

            PresentTiming PresentTexture(
                ID3D11Device* device,
                ID3D11Texture2D* texture,
                UINT subresourceIndex,
                int sourceWidth,
                int sourceHeight,
                DXGI_FORMAT expectedFormat)
            {
                using Clock = std::chrono::steady_clock;
                PresentTiming timing;
                if (!device || !texture || sourceWidth <= 0 ||
                    sourceHeight <= 0 || !EnsureDevice(device)) {
                    return timing;
                }

                RECT client{};
                if (!window_ || !GetClientRect(window_, &client)) {
                    return timing;
                }
                const UINT outputWidth = static_cast<UINT>(
                    (std::max<LONG>)(client.right - client.left, 0));
                const UINT outputHeight = static_cast<UINT>(
                    (std::max<LONG>)(client.bottom - client.top, 0));
                if (outputWidth == 0 || outputHeight == 0 ||
                    !EnsureSwapChain(window_, outputWidth, outputHeight)) {
                    return timing;
                }

                D3D11_TEXTURE2D_DESC sourceDescription{};
                texture->GetDesc(&sourceDescription);
                if (sourceDescription.Format != expectedFormat ||
                    !EnsureVideoProcessor(sourceDescription.Width,
                        sourceDescription.Height,
                        outputWidth, outputHeight)) {
                    return timing;
                }
                UINT formatSupport = 0;
                if (FAILED(processorEnumerator_->CheckVideoProcessorFormat(
                        sourceDescription.Format, &formatSupport)) ||
                    (formatSupport &
                        D3D11_VIDEO_PROCESSOR_FORMAT_SUPPORT_INPUT) == 0) {
                    return timing;
                }

                const UINT mipLevels =
                    (std::max<UINT>)(sourceDescription.MipLevels, 1);
                D3D11_VIDEO_PROCESSOR_INPUT_VIEW_DESC inputDescription{};
                inputDescription.ViewDimension = D3D11_VPIV_DIMENSION_TEXTURE2D;
                inputDescription.Texture2D.MipSlice =
                    subresourceIndex % mipLevels;
                inputDescription.Texture2D.ArraySlice =
                    subresourceIndex / mipLevels;
                ComPtr<ID3D11VideoProcessorInputView> inputView;
                if (FAILED(videoDevice_->CreateVideoProcessorInputView(
                    texture, processorEnumerator_.Get(),
                    &inputDescription, &inputView))) {
                    return timing;
                }

                ComPtr<ID3D11Texture2D> backBuffer;
                if (FAILED(swapChain_->GetBuffer(0, IID_PPV_ARGS(&backBuffer)))) {
                    return timing;
                }
                D3D11_VIDEO_PROCESSOR_OUTPUT_VIEW_DESC outputDescription{};
                outputDescription.ViewDimension = D3D11_VPOV_DIMENSION_TEXTURE2D;
                ComPtr<ID3D11VideoProcessorOutputView> outputView;
                if (FAILED(videoDevice_->CreateVideoProcessorOutputView(
                    backBuffer.Get(), processorEnumerator_.Get(),
                    &outputDescription, &outputView))) {
                    return timing;
                }

                const RECT sourceRect{ 0, 0, sourceWidth, sourceHeight };
                const RECT destinationRect{
                    0, 0, static_cast<LONG>(outputWidth),
                    static_cast<LONG>(outputHeight) };
                videoContext_->VideoProcessorSetOutputTargetRect(
                    processor_.Get(), TRUE, &destinationRect);
                videoContext_->VideoProcessorSetStreamFrameFormat(
                    processor_.Get(), 0, D3D11_VIDEO_FRAME_FORMAT_PROGRESSIVE);
                videoContext_->VideoProcessorSetStreamSourceRect(
                    processor_.Get(), 0, TRUE, &sourceRect);
                videoContext_->VideoProcessorSetStreamDestRect(
                    processor_.Get(), 0, TRUE, &destinationRect);
                videoContext_->VideoProcessorSetStreamAutoProcessingMode(
                    processor_.Get(), 0, FALSE);

                D3D11_VIDEO_PROCESSOR_STREAM stream{};
                stream.Enable = TRUE;
                stream.InputFrameOrField = frameIndex_++;
                stream.pInputSurface = inputView.Get();
                const auto videoProcessorStartedAt = Clock::now();
                if (FAILED(videoContext_->VideoProcessorBlt(
                    processor_.Get(), outputView.Get(), 0, 1, &stream))) {
                    return timing;
                }
                timing.videoProcessorSubmitUs =
                    static_cast<std::uint64_t>(
                        std::chrono::duration_cast<std::chrono::microseconds>(
                            Clock::now() - videoProcessorStartedAt)
                            .count());

                ComPtr<ID3D11RenderTargetView> backBufferView;
                if (SUCCEEDED(device_->CreateRenderTargetView(
                        backBuffer.Get(), nullptr, &backBufferView))) {
                    DrawCursorOverlay(backBufferView.Get(),
                        outputWidth, outputHeight);
                }

                const auto presentStartedAt = Clock::now();
                const HRESULT result = swapChain_->Present(0, 0);
                timing.presentCallUs = static_cast<std::uint64_t>(
                    std::chrono::duration_cast<std::chrono::microseconds>(
                        Clock::now() - presentStartedAt)
                        .count());
                if (result == DXGI_ERROR_DEVICE_REMOVED ||
                    result == DXGI_ERROR_DEVICE_RESET) {
                    ResetDevice();
                    return timing;
                }
                timing.succeeded =
                    SUCCEEDED(result) || result == DXGI_STATUS_OCCLUDED;
                return timing;
            }

            bool EnsureCpuDevice()
            {
                if (!cpuPresentationDevice_) {
                    constexpr D3D_FEATURE_LEVEL levels[] = {
                        D3D_FEATURE_LEVEL_11_1,
                        D3D_FEATURE_LEVEL_11_0,
                        D3D_FEATURE_LEVEL_10_1,
                        D3D_FEATURE_LEVEL_10_0,
                    };
                    D3D_FEATURE_LEVEL createdLevel{};
                    const UINT flags = D3D11_CREATE_DEVICE_BGRA_SUPPORT |
                        D3D11_CREATE_DEVICE_VIDEO_SUPPORT;
                    HRESULT result = D3D11CreateDevice(
                        nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, flags,
                        levels, static_cast<UINT>(std::size(levels)),
                        D3D11_SDK_VERSION, &cpuPresentationDevice_,
                        &createdLevel, nullptr);
                    if (result == E_INVALIDARG) {
                        result = D3D11CreateDevice(
                            nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, flags,
                            levels + 1,
                            static_cast<UINT>(std::size(levels) - 1),
                            D3D11_SDK_VERSION, &cpuPresentationDevice_,
                            &createdLevel, nullptr);
                    }
                    if (FAILED(result)) {
                        cpuPresentationDevice_.Reset();
                        return false;
                    }
                }
                return EnsureDevice(cpuPresentationDevice_.Get());
            }

            bool EnsureCpuUploadTexture(
                UINT width, UINT height, DXGI_FORMAT format)
            {
                if (cpuUploadTexture_ && cpuUploadWidth_ == width &&
                    cpuUploadHeight_ == height &&
                    cpuUploadFormat_ == format) {
                    return true;
                }
                cpuUploadTexture_.Reset();
                D3D11_TEXTURE2D_DESC description{};
                description.Width = width;
                description.Height = height;
                description.MipLevels = 1;
                description.ArraySize = 1;
                description.Format = format;
                description.SampleDesc.Count = 1;
                description.Usage = D3D11_USAGE_DEFAULT;
                description.BindFlags = D3D11_BIND_SHADER_RESOURCE |
                    D3D11_BIND_RENDER_TARGET;
                if (FAILED(device_->CreateTexture2D(
                        &description, nullptr, &cpuUploadTexture_))) {
                    return false;
                }
                cpuUploadWidth_ = width;
                cpuUploadHeight_ = height;
                cpuUploadFormat_ = format;
                return true;
            }

            bool EnsureDevice(ID3D11Device* device)
            {
                if (device_.Get() == device && videoDevice_ && videoContext_ &&
                    factory_) {
                    return true;
                }
                ResetDevice();
                device_ = device;
                device_->GetImmediateContext(&context_);
                if (!context_ || FAILED(device_.As(&videoDevice_)) ||
                    FAILED(context_.As(&videoContext_))) {
                    ResetDevice();
                    return false;
                }

                ComPtr<IDXGIDevice> dxgiDevice;
                ComPtr<IDXGIDevice1> dxgiDevice1;
                ComPtr<IDXGIAdapter> adapter;
                if (FAILED(device_.As(&dxgiDevice)) ||
                    FAILED(dxgiDevice->GetAdapter(&adapter)) ||
                    FAILED(adapter->GetParent(IID_PPV_ARGS(&factory_)))) {
                    ResetDevice();
                    return false;
                }

                // The decoder MFT and the renderer use the same D3D11 device
                // from different threads. Enable the runtime's immediate-
                // context serialization before issuing any video commands.
                ComPtr<ID3D10Multithread> multithread;
                if (SUCCEEDED(context_.As(&multithread))) {
                    multithread->SetMultithreadProtected(TRUE);
                }
                // Keep the flip-model presentation queue shallow. A deep
                // queue makes motion look delayed and bursty even though the
                // decoder reports a high average frame rate.
                if (SUCCEEDED(dxgiDevice.As(&dxgiDevice1))) {
                    dxgiDevice1->SetMaximumFrameLatency(1);
                }
                return true;
            }

            bool EnsureSwapChain(HWND window, UINT width, UINT height)
            {
                if (!swapChain_) {
                    DXGI_SWAP_CHAIN_DESC1 description{};
                    description.Width = width;
                    description.Height = height;
                    description.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
                    description.SampleDesc.Count = 1;
                    description.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
                    description.BufferCount = 2;
                    description.Scaling = DXGI_SCALING_STRETCH;
                    description.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
                    description.AlphaMode = DXGI_ALPHA_MODE_IGNORE;
                    if (FAILED(factory_->CreateSwapChainForHwnd(
                        device_.Get(), window, &description, nullptr, nullptr,
                        &swapChain_))) {
                        return false;
                    }
                    factory_->MakeWindowAssociation(window, DXGI_MWA_NO_ALT_ENTER);
                    swapWidth_ = width;
                    swapHeight_ = height;
                    return true;
                }
                if (swapWidth_ == width && swapHeight_ == height) {
                    return true;
                }

                processor_.Reset();
                processorEnumerator_.Reset();
                i420BackBufferView_.Reset();
                if (FAILED(swapChain_->ResizeBuffers(
                    0, width, height, DXGI_FORMAT_UNKNOWN, 0))) {
                    swapChain_.Reset();
                    swapWidth_ = 0;
                    swapHeight_ = 0;
                    return EnsureSwapChain(window, width, height);
                }
                swapWidth_ = width;
                swapHeight_ = height;
                return true;
            }

            bool EnsureVideoProcessor(UINT sourceWidth,
                UINT sourceHeight,
                UINT outputWidth,
                UINT outputHeight)
            {
                if (processor_ && sourceWidth_ == sourceWidth &&
                    sourceHeight_ == sourceHeight &&
                    processorWidth_ == outputWidth &&
                    processorHeight_ == outputHeight) {
                    return true;
                }

                processor_.Reset();
                processorEnumerator_.Reset();
                D3D11_VIDEO_PROCESSOR_CONTENT_DESC content{};
                content.InputFrameFormat = D3D11_VIDEO_FRAME_FORMAT_PROGRESSIVE;
                content.InputFrameRate = { 120, 1 };
                content.InputWidth = sourceWidth;
                content.InputHeight = sourceHeight;
                content.OutputFrameRate = { 120, 1 };
                content.OutputWidth = outputWidth;
                content.OutputHeight = outputHeight;
                content.Usage = D3D11_VIDEO_USAGE_PLAYBACK_NORMAL;
                if (FAILED(videoDevice_->CreateVideoProcessorEnumerator(
                    &content, &processorEnumerator_)) ||
                    FAILED(videoDevice_->CreateVideoProcessor(
                        processorEnumerator_.Get(), 0, &processor_))) {
                    processor_.Reset();
                    processorEnumerator_.Reset();
                    return false;
                }
                sourceWidth_ = sourceWidth;
                sourceHeight_ = sourceHeight;
                processorWidth_ = outputWidth;
                processorHeight_ = outputHeight;
                return true;
            }

            void ResetDevice()
            {
                ResetCursorResources();
                ResetI420ShaderResources();
                processor_.Reset();
                processorEnumerator_.Reset();
                cpuUploadTexture_.Reset();
                swapChain_.Reset();
                factory_.Reset();
                videoContext_.Reset();
                videoDevice_.Reset();
                context_.Reset();
                device_.Reset();
                swapWidth_ = 0;
                swapHeight_ = 0;
                sourceWidth_ = 0;
                sourceHeight_ = 0;
                processorWidth_ = 0;
                processorHeight_ = 0;
                cpuUploadWidth_ = 0;
                cpuUploadHeight_ = 0;
                cpuUploadFormat_ = DXGI_FORMAT_UNKNOWN;
            }

            ComPtr<ID3D11Device> device_;
            ComPtr<ID3D11DeviceContext> context_;
            ComPtr<ID3D11VideoDevice> videoDevice_;
            ComPtr<ID3D11VideoContext> videoContext_;
            ComPtr<IDXGIFactory2> factory_;
            ComPtr<IDXGISwapChain1> swapChain_;
            ComPtr<ID3D11VideoProcessorEnumerator> processorEnumerator_;
            ComPtr<ID3D11VideoProcessor> processor_;
            ComPtr<ID3D11Device> cpuPresentationDevice_;
            ComPtr<ID3D11Texture2D> cpuUploadTexture_;
            std::array<I420UploadSlot, 3> i420UploadSlots_;
            ComPtr<ID3D11VertexShader> i420VertexShader_;
            ComPtr<ID3D11PixelShader> i420PixelShader_;
            ComPtr<ID3D11SamplerState> i420SamplerState_;
            ComPtr<ID3D11RasterizerState> i420RasterizerState_;
            ComPtr<ID3D11RenderTargetView> i420BackBufferView_;
            ComPtr<ID3D11VertexShader> cursorVertexShader_;
            ComPtr<ID3D11PixelShader> cursorPixelShader_;
            ComPtr<ID3D11SamplerState> cursorSamplerState_;
            ComPtr<ID3D11RasterizerState> cursorRasterizerState_;
            ComPtr<ID3D11BlendState> cursorBlendState_;
            ComPtr<ID3D11Buffer> cursorConstantBuffer_;
            ComPtr<ID3D11Texture2D> cursorTexture_;
            ComPtr<ID3D11ShaderResourceView> cursorTextureView_;
            HWND window_ = nullptr;
            UINT swapWidth_ = 0;
            UINT swapHeight_ = 0;
            UINT sourceWidth_ = 0;
            UINT sourceHeight_ = 0;
            UINT processorWidth_ = 0;
            UINT processorHeight_ = 0;
            UINT frameIndex_ = 0;
            UINT cpuUploadWidth_ = 0;
            UINT cpuUploadHeight_ = 0;
            DXGI_FORMAT cpuUploadFormat_ = DXGI_FORMAT_UNKNOWN;
            UINT i420UploadWidth_ = 0;
            UINT i420UploadHeight_ = 0;
            std::size_t i420UploadSlotIndex_ = 0;
            QImage cursorImage_;
            QPoint cursorHotspot_;
            QPoint cursorPosition_;
            std::uint64_t cursorShapeRevision_ = 0;
            std::uint64_t uploadedCursorShapeRevision_ = 0;
            UINT cursorTextureWidth_ = 0;
            UINT cursorTextureHeight_ = 0;
            int cursorSourceWidth_ = 0;
            int cursorSourceHeight_ = 0;
            bool cursorVisible_ = false;
        };
