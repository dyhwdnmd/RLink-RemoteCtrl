// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

            static bool IsCpuNv12Image(const QImage& image)
            {
                return !image.isNull() &&
                    image.format() == QImage::Format_Grayscale8 &&
                    image.height() > 0 && image.height() % 3 == 0;
            }

            void UpdateCpuCanvasMetrics()
            {
                cpuCanvasWidth_.store(width(), std::memory_order_release);
                cpuCanvasHeight_.store(height(), std::memory_order_release);
                cpuCanvasDevicePixelRatioMilli_.store(
                    (std::clamp)(static_cast<int>(std::lround(
                        devicePixelRatioF() * 1000.0)), 500, 8000),
                    std::memory_order_release);
            }

            QSize CpuQtOutputSize(int sourceWidth, int sourceHeight) const
            {
                // At 100%, retain every decoded source pixel. QImage's DPR
                // maps these physical pixels to the correct logical QWidget
                // size without any resampling.
                if (actualPixelDisplayMode_.load(
                        std::memory_order_acquire)) {
                    return QSize(sourceWidth, sourceHeight);
                }
                const int logicalCanvasWidth = cpuCanvasWidth_.load(
                    std::memory_order_acquire);
                const int logicalCanvasHeight = cpuCanvasHeight_.load(
                    std::memory_order_acquire);
                const int dprMilli = (std::clamp)(
                    cpuCanvasDevicePixelRatioMilli_.load(
                        std::memory_order_acquire), 500, 8000);
                const int canvasWidth = static_cast<int>(
                    (static_cast<std::int64_t>(logicalCanvasWidth) *
                        dprMilli + 500) / 1000);
                const int canvasHeight = static_cast<int>(
                    (static_cast<std::int64_t>(logicalCanvasHeight) *
                        dprMilli + 500) / 1000);
                if (sourceWidth <= 0 || sourceHeight <= 0 ||
                    canvasWidth <= 0 || canvasHeight <= 0) {
                    return QSize(sourceWidth, sourceHeight);
                }

                int outputWidth = canvasWidth;
                int outputHeight = canvasHeight;
                if (!pointerStretchToCanvas_.load(
                        std::memory_order_acquire)) {
                    const std::int64_t widthLimitedHeight =
                        static_cast<std::int64_t>(canvasWidth) *
                        sourceHeight / sourceWidth;
                    if (widthLimitedHeight <= canvasHeight) {
                        outputHeight = static_cast<int>(widthLimitedHeight);
                    }
                    else {
                        outputWidth = static_cast<int>(
                            static_cast<std::int64_t>(canvasHeight) *
                            sourceWidth / sourceHeight);
                    }
                }

                // I420 chroma planes are most efficient and least surprising
                // when both dimensions are even. The Qt canvas is never
                // smaller than two pixels in normal operation, but keep the
                // fallback valid during transient layout changes as well.
                outputWidth = (std::max)(2, outputWidth & ~1);
                outputHeight = (std::max)(2, outputHeight & ~1);
                return QSize(outputWidth, outputHeight);
            }

            void QueueCpuFrame(
                webrtc::scoped_refptr<webrtc::VideoFrameBuffer> frame,
                bool recordArrival = true)
            {
                if (recordArrival) {
                    const bool d3d11Available =
                        nativeRenderingEnabled_ &&
                        !cpuD3D11PresentationFailed_.load(
                            std::memory_order_acquire);
                    VideoPresentationTelemetryRegistry::Instance()
                        .RecordArrival(presentationTelemetryId_,
                            !d3d11Available
                                ? VideoPresentationPath::kCpuQt
                                : (!cpuNv12D3D11PresentationFailed_.load(
                                        std::memory_order_acquire)
                                    ? VideoPresentationPath::kCpuNv12D3D11
                                    : VideoPresentationPath::kCpuD3D11));
                }
                const std::uint64_t sequence =
                    receivedFrameSequence_.fetch_add(1, std::memory_order_acq_rel) + 1;
                bool superseded = false;
                {
                    std::lock_guard lock(cpuFrameMutex_);
                    superseded = pendingCpuFrame_ != nullptr;
                    pendingCpuFrame_ = std::move(frame);
                    pendingCpuSequence_ = sequence;
                }
                if (superseded) {
                    VideoPresentationTelemetryRegistry::Instance()
                        .RecordSuperseded(presentationTelemetryId_);
                }
                cpuFrameCondition_.notify_one();
            }

            void CpuConversionLoop(std::stop_token stopToken)
            {
                using Clock = std::chrono::steady_clock;
                SetThreadDescription(
                    GetCurrentThread(), L"RemoteC CPU Frame Conversion");
                SetThreadPriority(
                    GetCurrentThread(), THREAD_PRIORITY_ABOVE_NORMAL);
                Clock::time_point nextConversionAt{};
                bool hasDeadline = false;
                webrtc::scoped_refptr<webrtc::I420Buffer>
                    scaledI420Buffer;

                while (!stopToken.stop_requested()) {
                    webrtc::scoped_refptr<webrtc::VideoFrameBuffer> frame;
                    std::uint64_t sequence = 0;
                    {
                        std::unique_lock lock(cpuFrameMutex_);
                        cpuFrameCondition_.wait(lock, [this, &stopToken] {
                            return stopToken.stop_requested() ||
                                pendingCpuFrame_ != nullptr;
                        });
                        if (stopToken.stop_requested()) {
                            break;
                        }

                        if (hasDeadline) {
                            cpuFrameCondition_.wait_until(
                                lock, nextConversionAt, [&stopToken] {
                                    return stopToken.stop_requested();
                                });
                            if (stopToken.stop_requested()) {
                                break;
                            }
                        }

                        // Frames that arrived while waiting replaced the old
                        // mailbox value. Only the frame that can actually be
                        // shown at this display deadline is converted.
                        frame = std::move(pendingCpuFrame_);
                        sequence = pendingCpuSequence_;
                        pendingCpuSequence_ = 0;
                    }

                    const auto interval = std::chrono::microseconds(
                        cpuConversionIntervalUs_.load(
                            std::memory_order_acquire));
                    const auto now = Clock::now();
                    if (!hasDeadline || now > nextConversionAt + interval) {
                        nextConversionAt = now + interval;
                        hasDeadline = true;
                    }
                    else {
                        nextConversionAt += interval;
                    }

                    if (!frame || sequence != receivedFrameSequence_.load(
                            std::memory_order_acquire)) {
                        if (frame) {
                            VideoPresentationTelemetryRegistry::Instance()
                                .RecordSuperseded(presentationTelemetryId_);
                        }
                        continue;
                    }
                    const auto conversionStartedAt = Clock::now();
                    const auto sourceI420 = frame->ToI420();
                    if (!sourceI420) {
                        VideoPresentationTelemetryRegistry::Instance()
                            .RecordPresentFailure(presentationTelemetryId_);
                        continue;
                    }

                    const bool useNv12GpuPath =
                        nativeRenderingEnabled_ &&
                        (sourceI420->width() & 1) == 0 &&
                        (sourceI420->height() & 1) == 0 &&
                        !cpuNv12D3D11PresentationFailed_.load(
                            std::memory_order_acquire) &&
                        !cpuD3D11PresentationFailed_.load(
                            std::memory_order_acquire);
                    const bool useQtCpuPath =
                        !nativeRenderingEnabled_ ||
                        cpuD3D11PresentationFailed_.load(
                            std::memory_order_acquire);
                    webrtc::scoped_refptr<webrtc::I420BufferInterface> i420 =
                        sourceI420;
                    if (!useNv12GpuPath && useQtCpuPath) {
                        const QSize outputSize = CpuQtOutputSize(
                            sourceI420->width(), sourceI420->height());
                        if (outputSize.width() != sourceI420->width() ||
                            outputSize.height() != sourceI420->height()) {
                            if (!scaledI420Buffer ||
                                scaledI420Buffer->width() !=
                                    outputSize.width() ||
                                scaledI420Buffer->height() !=
                                    outputSize.height()) {
                                scaledI420Buffer =
                                    webrtc::I420Buffer::Create(
                                        outputSize.width(),
                                        outputSize.height());
                            }
                            if (!scaledI420Buffer || libyuv::I420Scale(
                                    sourceI420->DataY(), sourceI420->StrideY(),
                                    sourceI420->DataU(), sourceI420->StrideU(),
                                    sourceI420->DataV(), sourceI420->StrideV(),
                                    sourceI420->width(), sourceI420->height(),
                                    scaledI420Buffer->MutableDataY(),
                                    scaledI420Buffer->StrideY(),
                                    scaledI420Buffer->MutableDataU(),
                                    scaledI420Buffer->StrideU(),
                                    scaledI420Buffer->MutableDataV(),
                                    scaledI420Buffer->StrideV(),
                                    scaledI420Buffer->width(),
                                    scaledI420Buffer->height(),
                                    libyuv::kFilterBilinear) != 0) {
                                VideoPresentationTelemetryRegistry::Instance()
                                    .RecordPresentFailure(
                                        presentationTelemetryId_);
                                continue;
                            }
                            i420 = scaledI420Buffer;
                        }
                    }
                    const QImage::Format targetFormat = useNv12GpuPath
                        ? QImage::Format_Grayscale8
                        : QImage::Format_ARGB32;
                    const QSize targetStorageSize(
                        i420->width(),
                        useNv12GpuPath
                            ? i420->height() + i420->height() / 2
                            : i420->height());
                    QImage image;
                    {
                        std::lock_guard lock(cpuImageMutex_);
                        if (recycledCpuImage_.size() ==
                                targetStorageSize &&
                            recycledCpuImage_.format() == targetFormat) {
                            image = std::move(recycledCpuImage_);
                        }
                    }
                    if (image.isNull()) {
                        image = QImage(targetStorageSize, targetFormat);
                    }
                    // The storage size is in physical pixels. Recording DPR
                    // keeps its device-independent size equal to the QWidget
                    // content rectangle and prevents a second enlargement.
                    image.setDevicePixelRatio(useQtCpuPath
                        ? static_cast<qreal>((std::clamp)(
                            cpuCanvasDevicePixelRatioMilli_.load(
                                std::memory_order_acquire),
                            500, 8000)) / 1000.0
                        : 1.0);
                    int conversionResult = -1;
                    if (!image.isNull()) {
                        if (useNv12GpuPath) {
                            const int rowPitch = image.bytesPerLine();
                            conversionResult = libyuv::I420ToNV12(
                                i420->DataY(), i420->StrideY(),
                                i420->DataU(), i420->StrideU(),
                                i420->DataV(), i420->StrideV(),
                                image.bits(), rowPitch,
                                image.bits() +
                                    rowPitch * i420->height(),
                                rowPitch,
                                i420->width(), i420->height());
                        }
                        else {
                            conversionResult = libyuv::I420ToARGB(
                                i420->DataY(), i420->StrideY(),
                                i420->DataU(), i420->StrideU(),
                                i420->DataV(), i420->StrideV(),
                                image.bits(), image.bytesPerLine(),
                                i420->width(), i420->height());
                        }
                    }
                    if (image.isNull() || conversionResult != 0 ||
                        sequence != receivedFrameSequence_.load(
                            std::memory_order_acquire)) {
                        if (sequence != receivedFrameSequence_.load(
                                std::memory_order_acquire)) {
                            VideoPresentationTelemetryRegistry::Instance()
                                .RecordSuperseded(presentationTelemetryId_);
                        }
                        else {
                            VideoPresentationTelemetryRegistry::Instance()
                                .RecordPresentFailure(
                                    presentationTelemetryId_);
                        }
                        RecycleCpuImage(std::move(image));
                        continue;
                    }
                    const auto conversionUs = static_cast<std::uint64_t>(
                        std::chrono::duration_cast<std::chrono::microseconds>(
                            Clock::now() - conversionStartedAt)
                            .count());
                    VideoPresentationTelemetryRegistry::Instance()
                        .RecordConversion(
                            presentationTelemetryId_, conversionUs);
                    QueueConvertedCpuImage(std::move(image), sequence);
                }
            }

            void RecycleCpuImage(QImage image)
            {
                if (image.isNull()) {
                    return;
                }
                std::lock_guard lock(cpuImageMutex_);
                recycledCpuImage_ = std::move(image);
            }

            void QueueConvertedCpuImage(QImage image, std::uint64_t sequence)
            {
                if (nativeRenderingEnabled_ &&
                    !cpuD3D11PresentationFailed_.load(
                        std::memory_order_acquire)) {
                    QueueD3D11CpuImage(std::move(image), sequence);
                    return;
                }
                QueueQtCpuImage(std::move(image), sequence);
            }

            void QueueD3D11CpuImage(QImage image, std::uint64_t sequence)
            {
                const int frameWidth = image.width();
                // NV12 is stored in one owned byte buffer whose QImage height
                // covers both the Y plane and the half-height interleaved UV
                // plane. Geometry and aspect-ratio calculations must use the
                // actual video height rather than this 3/2 storage height.
                const int frameHeight = IsCpuNv12Image(image)
                    ? image.height() * 2 / 3
                    : image.height();
                bool superseded = false;
                QImage supersededCpuImage;
                {
                    std::lock_guard lock(nativeFrameMutex_);
                    superseded = pendingNativeFrame_ != nullptr ||
                        !pendingD3D11CpuImage_.isNull();
                    pendingNativeFrame_ = nullptr;
                    pendingNativeSequence_ = 0;
                    supersededCpuImage =
                        std::move(pendingD3D11CpuImage_);
                    pendingD3D11CpuImage_ = std::move(image);
                    pendingD3D11CpuSequence_ = sequence;
                }
                if (superseded) {
                    VideoPresentationTelemetryRegistry::Instance()
                        .RecordSuperseded(presentationTelemetryId_);
                }
                RecycleCpuImage(std::move(supersededCpuImage));

                const int oldWidth = nativeFrameWidth_.exchange(
                    frameWidth, std::memory_order_acq_rel);
                const int oldHeight = nativeFrameHeight_.exchange(
                    frameHeight, std::memory_order_acq_rel);
                const bool firstActivation =
                    !nativeUiActiveRequested_.exchange(
                        true, std::memory_order_acq_rel);
                if (firstActivation || oldWidth != frameWidth ||
                    oldHeight != frameHeight) {
                    ScheduleNativeUiUpdate();
                }
                nativeFrameCondition_.notify_one();
            }

            void QueueQtCpuImage(QImage image, std::uint64_t sequence)
            {
                bool dispatch = false;
                {
                    std::lock_guard lock(cpuImageMutex_);
                    if (!pendingCpuImage_.isNull()) {
                        VideoPresentationTelemetryRegistry::Instance()
                            .RecordSuperseded(presentationTelemetryId_);
                        recycledCpuImage_ = std::move(pendingCpuImage_);
                    }
                    pendingCpuImage_ = std::move(image);
                    pendingCpuImageSequence_ = sequence;
                    if (!cpuImageDispatchPending_) {
                        cpuImageDispatchPending_ = true;
                        dispatch = true;
                    }
                }
                if (dispatch) {
                    QMetaObject::invokeMethod(
                        this, [this] { ApplyPendingCpuImage(); },
                        Qt::QueuedConnection);
                }
            }

            void ApplyPendingCpuImage()
            {
                QImage image;
                std::uint64_t sequence = 0;
                {
                    std::lock_guard lock(cpuImageMutex_);
                    image = std::move(pendingCpuImage_);
                    sequence = pendingCpuImageSequence_;
                    pendingCpuImageSequence_ = 0;
                    cpuImageDispatchPending_ = false;
                }
                if (image.isNull() ||
                    sequence != receivedFrameSequence_.load(
                        std::memory_order_acquire)) {
                    if (!image.isNull()) {
                        VideoPresentationTelemetryRegistry::Instance()
                            .RecordSuperseded(presentationTelemetryId_);
                    }
                    RecycleCpuImage(std::move(image));
                    return;
                }

                if (!DeactivateNativePresentation(sequence)) {
                    VideoPresentationTelemetryRegistry::Instance()
                        .RecordSuperseded(presentationTelemetryId_);
                    RecycleCpuImage(std::move(image));
                    return;
                }
                nativeFrameSize_ = {};
                nativeFrameActive_ = false;
                if (nativeSurface_) {
                    nativeSurface_->hide();
                }

                QImage previous = std::move(frameImage_);
                frameImage_ = std::move(image);
                frameImageSequence_ = sequence;
                RecycleCpuImage(std::move(previous));
                update();
            }

            void QueueNativeFrame(
                webrtc::scoped_refptr<webrtc::VideoFrameBuffer> frame)
            {
                auto* native = D3D11NativeFrameBuffer::From(frame.get());
                if (!native) {
                    QueueCpuFrame(std::move(frame));
                    return;
                }
                const int frameWidth = native->width();
                const int frameHeight = native->height();
                VideoPresentationTelemetryRegistry::Instance().RecordArrival(
                    presentationTelemetryId_,
                    VideoPresentationPath::kD3D11);
                const std::uint64_t sequence =
                    receivedFrameSequence_.fetch_add(1, std::memory_order_acq_rel) + 1;
                bool superseded = false;
                QImage supersededCpuImage;
                {
                    std::lock_guard lock(nativeFrameMutex_);
                    superseded = pendingNativeFrame_ != nullptr ||
                        !pendingD3D11CpuImage_.isNull();
                    supersededCpuImage =
                        std::move(pendingD3D11CpuImage_);
                    pendingD3D11CpuSequence_ = 0;
                    pendingNativeFrame_ = std::move(frame);
                    pendingNativeSequence_ = sequence;
                }
                if (superseded) {
                    VideoPresentationTelemetryRegistry::Instance()
                        .RecordSuperseded(presentationTelemetryId_);
                }
                RecycleCpuImage(std::move(supersededCpuImage));

                const int oldWidth = nativeFrameWidth_.exchange(
                    frameWidth, std::memory_order_acq_rel);
                const int oldHeight = nativeFrameHeight_.exchange(
                    frameHeight, std::memory_order_acq_rel);
                const bool firstActivation =
                    !nativeUiActiveRequested_.exchange(
                        true, std::memory_order_acq_rel);
                if (firstActivation || oldWidth != frameWidth ||
                    oldHeight != frameHeight) {
                    ScheduleNativeUiUpdate();
                }
                nativeFrameCondition_.notify_one();
            }

            void QueueI420Frame(
                webrtc::scoped_refptr<webrtc::VideoFrameBuffer> frame)
            {
                if (!frame || frame->width() <= 0 || frame->height() <= 0) {
                    return;
                }
                const int frameWidth = frame->width();
                const int frameHeight = frame->height();
                VideoPresentationTelemetryRegistry::Instance().RecordArrival(
                    presentationTelemetryId_,
                    VideoPresentationPath::kCpuI420D3D11);
                const std::uint64_t sequence =
                    receivedFrameSequence_.fetch_add(
                        1, std::memory_order_acq_rel) + 1;
                bool superseded = false;
                QImage supersededCpuImage;
                {
                    std::lock_guard lock(nativeFrameMutex_);
                    superseded = pendingNativeFrame_ != nullptr ||
                        !pendingD3D11CpuImage_.isNull();
                    supersededCpuImage =
                        std::move(pendingD3D11CpuImage_);
                    pendingD3D11CpuSequence_ = 0;
                    pendingNativeFrame_ = std::move(frame);
                    pendingNativeSequence_ = sequence;
                }
                if (superseded) {
                    VideoPresentationTelemetryRegistry::Instance()
                        .RecordSuperseded(presentationTelemetryId_);
                }
                RecycleCpuImage(std::move(supersededCpuImage));

                const int oldWidth = nativeFrameWidth_.exchange(
                    frameWidth, std::memory_order_acq_rel);
                const int oldHeight = nativeFrameHeight_.exchange(
                    frameHeight, std::memory_order_acq_rel);
                const bool firstActivation =
                    !nativeUiActiveRequested_.exchange(
                        true, std::memory_order_acq_rel);
                if (firstActivation || oldWidth != frameWidth ||
                    oldHeight != frameHeight) {
                    ScheduleNativeUiUpdate();
                }
                nativeFrameCondition_.notify_one();
            }

            void ScheduleNativeUiUpdate()
            {
                nativeUiRevision_.fetch_add(1, std::memory_order_acq_rel);
                if (nativeUiDispatchPending_.exchange(
                        true, std::memory_order_acq_rel)) {
                    return;
                }
                QMetaObject::invokeMethod(this, [this] {
                    const std::uint64_t revision = nativeUiRevision_.load(
                        std::memory_order_acquire);
                    if (nativeUiActiveRequested_.load(
                            std::memory_order_acquire)) {
                        const int width = nativeFrameWidth_.load(
                            std::memory_order_acquire);
                        const int height = nativeFrameHeight_.load(
                            std::memory_order_acquire);
                        if (width > 0 && height > 0) {
                            nativeFrameSize_ = QSize(width, height);
                            nativeFrameActive_ = true;
                            frameImage_ = {};
                            UpdateNativeSurfaceGeometry();
                            nativeSurface_->show();
                            nativeSurface_->raise();
                            nativeSurfaceReady_.store(
                                true, std::memory_order_release);
                            nativeFrameCondition_.notify_one();
                            update();
                        }
                    }
                    nativeUiDispatchPending_.store(
                        false, std::memory_order_release);
                    if (revision != nativeUiRevision_.load(
                            std::memory_order_acquire)) {
                        ScheduleNativeUiUpdate();
                    }
                }, Qt::QueuedConnection);
            }

            void UpdateCpuConversionInterval()
            {
                const QScreen* currentScreen = screen();
                const qreal reportedRefreshRate = currentScreen
                    ? currentScreen->refreshRate()
                    : 60.0;
                const qreal refreshRate = (std::clamp)(
                    reportedRefreshRate, 30.0, 240.0);
                VideoPresentationTelemetryRegistry::Instance()
                    .SetLocalRefreshRate(
                        presentationTelemetryId_, reportedRefreshRate);
                cpuConversionIntervalUs_.store(
                    static_cast<std::uint32_t>((std::max)(
                        1.0, std::round(1'000'000.0 / refreshRate))),
                    std::memory_order_release);
                nativePresentationIntervalUs_.store(
                    static_cast<std::uint32_t>((std::max)(
                        1.0, std::round(1'000'000.0 / refreshRate))),
                    std::memory_order_release);
            }

            void RequestNativeRedraw()
            {
                if (!nativeUiActiveRequested_.load(
                        std::memory_order_acquire)) {
                    return;
                }
                {
                    std::lock_guard lock(nativeFrameMutex_);
                    nativeRedrawRequested_ = true;
                }
                nativeFrameCondition_.notify_one();
            }

            void CursorVisualChanged()
            {
                // Qt painting reuses frameImage_; D3D11 presentation reuses
                // the retained last frame. No video frame or extra cursor
                // window is required for a pointer-only update.
                update();
                RequestNativeRedraw();
            }

            bool DeactivateNativePresentation(std::uint64_t cpuSequence)
            {
                QImage retiredCpuImage;
                {
                    std::lock_guard lock(nativeFrameMutex_);
                    if (cpuSequence != receivedFrameSequence_.load(
                            std::memory_order_acquire)) {
                        return false;
                    }
                    pendingNativeFrame_ = nullptr;
                    pendingNativeSequence_ = 0;
                    retiredCpuImage =
                        std::move(pendingD3D11CpuImage_);
                    pendingD3D11CpuSequence_ = 0;
                    nativeRedrawRequested_ = false;
                    nativeResetRequested_ = true;
                    nativeUiActiveRequested_.store(
                        false, std::memory_order_release);
                    nativeSurfaceReady_.store(
                        false, std::memory_order_release);
                }
                RecycleCpuImage(std::move(retiredCpuImage));
                nativeFrameCondition_.notify_one();
                return true;
            }

            void NativePresentationLoop(std::stop_token stopToken)
            {
                using Clock = std::chrono::steady_clock;
                const HRESULT comResult = CoInitializeEx(
                    nullptr, COINIT_MULTITHREADED);
                SetThreadDescription(
                    GetCurrentThread(), L"RemoteC D3D11 Present");
                SetThreadPriority(
                    GetCurrentThread(), THREAD_PRIORITY_ABOVE_NORMAL);

                webrtc::scoped_refptr<webrtc::VideoFrameBuffer>
                    lastNativeFrame;
                QImage lastCpuImage;
                std::uint64_t lastSequence = 0;
                bool lastWasCpu = false;
                Clock::time_point nextPresentationAt{};
                bool hasDeadline = false;

                while (!stopToken.stop_requested()) {
                    webrtc::scoped_refptr<webrtc::VideoFrameBuffer> frame;
                    QImage cpuImage;
                    std::uint64_t sequence = 0;
                    bool newFrame = false;
                    bool cpuFrame = false;
                    bool cpuNv12Frame = false;
                    bool cpuI420Frame = false;
                    bool presentationFallbackQueued = false;
                    {
                        std::unique_lock lock(nativeFrameMutex_);
                        nativeFrameCondition_.wait(lock, [this, &stopToken] {
                            return stopToken.stop_requested() ||
                                pendingNativeFrame_ != nullptr ||
                                !pendingD3D11CpuImage_.isNull() ||
                                nativeRedrawRequested_ ||
                                nativeResetRequested_;
                        });
                        if (stopToken.stop_requested()) {
                            break;
                        }

                        if (nativeResetRequested_) {
                            nativeResetRequested_ = false;
                            lastNativeFrame = nullptr;
                            lastCpuImage = {};
                            lastSequence = 0;
                            lastWasCpu = false;
                            hasDeadline = false;
                        }

                        nativeFrameCondition_.wait(lock,
                            [this, &stopToken] {
                                return stopToken.stop_requested() ||
                                    nativeSurfaceReady_.load(
                                        std::memory_order_acquire) ||
                                    nativeResetRequested_;
                            });
                        if (stopToken.stop_requested()) {
                            break;
                        }
                        if (nativeResetRequested_) {
                            nativeResetRequested_ = false;
                            lastNativeFrame = nullptr;
                            lastCpuImage = {};
                            lastSequence = 0;
                            lastWasCpu = false;
                            hasDeadline = false;
                            continue;
                        }

                        if (hasDeadline &&
                            Clock::now() < nextPresentationAt) {
                            nativeFrameCondition_.wait_until(
                                lock, nextPresentationAt, [&stopToken] {
                                    return stopToken.stop_requested();
                                });
                            if (stopToken.stop_requested()) {
                                break;
                            }
                            if (nativeResetRequested_) {
                                nativeResetRequested_ = false;
                                lastNativeFrame = nullptr;
                                lastCpuImage = {};
                                lastSequence = 0;
                                lastWasCpu = false;
                                hasDeadline = false;
                            }
                        }

                        const bool redraw = nativeRedrawRequested_;
                        nativeRedrawRequested_ = false;
                        if (pendingNativeFrame_) {
                            frame = std::move(pendingNativeFrame_);
                            sequence = pendingNativeSequence_;
                            pendingNativeSequence_ = 0;
                            newFrame = true;
                        }
                        else if (!pendingD3D11CpuImage_.isNull()) {
                            cpuImage = std::move(pendingD3D11CpuImage_);
                            sequence = pendingD3D11CpuSequence_;
                            pendingD3D11CpuSequence_ = 0;
                            newFrame = true;
                            cpuFrame = true;
                            cpuNv12Frame = IsCpuNv12Image(cpuImage);
                        }
                        else if (redraw && lastWasCpu &&
                            !lastCpuImage.isNull()) {
                            cpuImage = lastCpuImage;
                            sequence = lastSequence;
                            cpuFrame = true;
                            cpuNv12Frame = IsCpuNv12Image(cpuImage);
                        }
                        else if (redraw && lastNativeFrame) {
                            frame = lastNativeFrame;
                            sequence = lastSequence;
                        }
                    }

                    if (!frame && cpuImage.isNull()) {
                        continue;
                    }
                    if (newFrame &&
                        sequence != receivedFrameSequence_.load(
                            std::memory_order_acquire)) {
                        VideoPresentationTelemetryRegistry::Instance()
                            .RecordSuperseded(presentationTelemetryId_);
                        if (cpuFrame) {
                            RecycleCpuImage(std::move(cpuImage));
                        }
                        continue;
                    }

                    if (cursorRenderState_) {
                        const auto cursor =
                            cursorRenderState_->GetSnapshot();
                        int cursorSourceWidth = 0;
                        int cursorSourceHeight = 0;
                        if (cpuFrame) {
                            cursorSourceWidth = cpuImage.width();
                            cursorSourceHeight = cpuNv12Frame
                                ? cpuImage.height() * 2 / 3
                                : cpuImage.height();
                        }
                        else if (frame) {
                            cursorSourceWidth = frame->width();
                            cursorSourceHeight = frame->height();
                        }
                        nativeSurface_->SetCursorFrame(cursor,
                            cursorSourceWidth, cursorSourceHeight);
                    }

                    D3D11VideoSurface::PresentTiming presentTiming;
                    if (cpuFrame) {
                        presentTiming = cpuNv12Frame
                            ? nativeSurface_->PresentCpuNv12(cpuImage)
                            : nativeSurface_->PresentCpuBgra(cpuImage);
                    }
                    else {
                        auto* native =
                            D3D11NativeFrameBuffer::From(frame.get());
                        if (native) {
                            presentTiming = nativeSurface_->Present(native);
                        }
                        else if (!cpuI420D3D11PresentationFailed_.load(
                                     std::memory_order_acquire)) {
                            const auto i420 = frame->ToI420();
                            if (i420) {
                                cpuI420Frame = true;
                                presentTiming =
                                    nativeSurface_->PresentCpuI420(i420.get());
                            }
                            else if (newFrame) {
                                QueueCpuFrame(std::move(frame), false);
                                presentationFallbackQueued = true;
                            }
                        }
                        else if (newFrame) {
                            QueueCpuFrame(std::move(frame), false);
                            presentationFallbackQueued = true;
                        }
                    }
                    if (presentationFallbackQueued) {
                        continue;
                    }
                    if (newFrame && presentTiming.cpuPreparationUs > 0) {
                        VideoPresentationTelemetryRegistry::Instance()
                            .RecordConversion(presentationTelemetryId_,
                                presentTiming.cpuPreparationUs);
                    }

                    if (presentTiming.succeeded) {
                        if (newFrame) {
                            if (cpuFrame) {
                                QImage previousCpuImage =
                                    std::move(lastCpuImage);
                                lastCpuImage = std::move(cpuImage);
                                lastNativeFrame = nullptr;
                                RecycleCpuImage(
                                    std::move(previousCpuImage));
                            }
                            else {
                                QImage previousCpuImage =
                                    std::move(lastCpuImage);
                                lastNativeFrame = frame;
                                RecycleCpuImage(
                                    std::move(previousCpuImage));
                            }
                            lastSequence = sequence;
                            lastWasCpu = cpuFrame;
                            VideoPresentationTelemetryRegistry::Instance()
                                .RecordPresented(presentationTelemetryId_,
                                    cpuNv12Frame
                                        ? VideoPresentationPath::kCpuNv12D3D11
                                        : (cpuFrame
                                            ? VideoPresentationPath::kCpuD3D11
                                            : (cpuI420Frame
                                                ? VideoPresentationPath::kCpuI420D3D11
                                                : VideoPresentationPath::kD3D11)),
                                    presentTiming.videoProcessorSubmitUs,
                                    presentTiming.presentCallUs);
                            NotifyFirstPresentation();
                        }
                    }
                    else if (newFrame) {
                        VideoPresentationTelemetryRegistry::Instance()
                            .RecordPresentFailure(
                                presentationTelemetryId_);
                        if (cpuFrame) {
                            if (cpuNv12Frame) {
                                // Some older drivers expose D3D11 video but
                                // reject application-uploaded NV12 input
                                // views. Drop this one frame and preserve the
                                // proven ARGB/D3D11 path for subsequent frames.
                                cpuNv12D3D11PresentationFailed_.store(
                                    true, std::memory_order_release);
                                RecycleCpuImage(std::move(cpuImage));
                            }
                            else {
                                cpuD3D11PresentationFailed_.store(
                                    true, std::memory_order_release);
                                QueueQtCpuImage(
                                    std::move(cpuImage), sequence);
                            }
                        }
                        else {
                            if (cpuI420Frame) {
                                cpuI420D3D11PresentationFailed_.store(
                                    true, std::memory_order_release);
                            }
                            QueueCpuFrame(std::move(frame), false);
                        }
                    }

                    const auto interval = std::chrono::microseconds(
                        nativePresentationIntervalUs_.load(
                            std::memory_order_acquire));
                    const auto now = Clock::now();
                    if (!hasDeadline ||
                        now > nextPresentationAt + interval) {
                        nextPresentationAt = now + interval;
                        hasDeadline = true;
                    }
                    else {
                        nextPresentationAt += interval;
                    }
                }

                RecycleCpuImage(std::move(lastCpuImage));

                if (SUCCEEDED(comResult)) {
                    CoUninitialize();
                }
            }
