// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

        class RemoteDesktopCanvas final
            : public QWidget,
            public webrtc::VideoSinkInterface<webrtc::VideoFrame> {
        public:
            using InputSender = std::function<bool(const RemoteInputEvent&)>;
            // Returns true when RemoteC consumed the action. False means a
            // keyboard Ctrl+V must continue to the controlled machine as an
            // ordinary remote key event.
            using PasteSender =
                std::function<bool(const QStringList&, bool keyboardPaste)>;
            using FirstPresentationCallback =
                std::function<void(std::uint64_t)>;

            explicit RemoteDesktopCanvas(InputSender inputSender,
                PasteSender pasteSender,
                FirstPresentationCallback firstPresentationCallback,
                std::string telemetryPeerDeviceId,
                QWidget* parent = nullptr)
                : QWidget(parent),
                inputSender_(std::move(inputSender)),
                pasteSender_(std::move(pasteSender)),
                firstPresentationCallback_(
                    std::move(firstPresentationCallback)),
                presentationTelemetryId_(
                    VideoPresentationTelemetryRegistry::Instance().Register(
                        std::move(telemetryPeerDeviceId))),
                nativeRenderingEnabled_(
                    QSettings().value(
                        QStringLiteral("media/videoRendererPreference"),
                        QStringLiteral("auto")).toString() !=
                    QStringLiteral("cpu"))
            {
                setMinimumSize(720, 430);
                setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
                setMouseTracking(true);
                setAcceptDrops(true);
                setFocusPolicy(Qt::StrongFocus);
                setCursor(Qt::ArrowCursor);
                setAttribute(Qt::WA_NativeWindow);
                pointerSamplingWindow_ =
                    reinterpret_cast<HWND>(winId());

                pointerMoveScheduler_ =
                    std::make_unique<
                        HighResolutionPointerMoveScheduler>(
                        inputSender_,
                        [this](const RemoteInputEvent& fallback) {
                            return SampleCurrentDragPointer(
                                fallback);
                        });
                SetTargetFrameRate(30);

                nativeSurface_ = new D3D11VideoSurface(this);
                nativeSurface_->hide();
                cursorRenderState_ =
                    std::make_unique<RemoteCursorRenderState>();
                UpdateCpuCanvasMetrics();
                UpdateCpuConversionInterval();

                // Window moving/resizing enters a Windows modal loop on the
                // Qt GUI thread. A QTimer-driven Present loop therefore loses
                // most frames while any RemoteC window is dragged. Keep D3D11
                // presentation on a persistent worker that consumes the same
                // latest-frame mailbox independently of the GUI event loop.
                nativePresentationThread_ = std::jthread(
                    [this](std::stop_token stopToken) {
                        NativePresentationLoop(stopToken);
                    });

                // FFmpeg produces CPU I420 frames. Converting a 2K frame to
                // ARGB on the Qt thread allocates and writes roughly 14 MiB
                // before every paint, which starves mouse/window events at a
                // high frame rate. A single persistent worker converts only
                // the newest frame at the local display cadence.
                cpuConversionThread_ = std::jthread(
                    [this](std::stop_token stopToken) {
                        CpuConversionLoop(stopToken);
                    });
            }

            ~RemoteDesktopCanvas() override
            {
                ShutdownInputScheduler();
                nativePresentationThread_.request_stop();
                nativeFrameCondition_.notify_all();
                if (nativePresentationThread_.joinable()) {
                    nativePresentationThread_.join();
                }
                cpuConversionThread_.request_stop();
                cpuFrameCondition_.notify_all();
                if (cpuConversionThread_.joinable()) {
                    cpuConversionThread_.join();
                }
                VideoPresentationTelemetryRegistry::Instance().Unregister(
                    presentationTelemetryId_);
            }

            void SetControlEnabled(bool enabled)
            {
                const bool changed = controlEnabled_ != enabled;
                controlEnabled_ = enabled;
                // The responsive pointer is composed from the independent
                // cursor stream. Hide the local QWidget cursor only while
                // remote input is active to avoid a double cursor.
                setCursor(actualPixelPanning_
                    ? Qt::ClosedHandCursor
                    : (enabled ? Qt::BlankCursor : Qt::ArrowCursor));
                if (cursorRenderState_) {
                    cursorRenderState_->SetRenderingEnabled(enabled);
                }
                if (!enabled) {
                    CancelPendingPointerMove();
                    clearFocus();
                }
                if (changed) {
                    CursorVisualChanged();
                }
            }

            void SetActualPixelDisplayMode(bool enabled)
            {
                if (actualPixelDisplayMode_.exchange(
                        enabled, std::memory_order_acq_rel) == enabled) {
                    return;
                }
                actualPixelPanning_ = false;
                actualPixelPanOffset_ = {};
                setCursor(controlEnabled_
                    ? Qt::BlankCursor
                    : Qt::ArrowCursor);
                // Full source resolution is retained in 100% mode; fit mode
                // returns to the existing worker-side pre-scale path.
                UpdateCpuCanvasMetrics();
                UpdateNativeSurfaceGeometry();
                RequestNativeRedraw();
                update();
            }

            bool ActualPixelDisplayMode() const
            {
                return actualPixelDisplayMode_.load(
                    std::memory_order_acquire);
            }

            void ApplyRemoteCursor(const RemoteCursorEnvelope& envelope)
            {
                if (!cursorRenderState_) return;
                switch (envelope.type) {
                case RemoteCursorMessageType::kShape:
                    cursorRenderState_->SetShape(envelope.shape);
                    break;
                case RemoteCursorMessageType::kPosition:
                    if (envelope.position.displayId == remoteDisplayId_ &&
                        envelope.position.displayLayoutVersion ==
                            remoteDisplayLayoutVersion_) {
                        cursorRenderState_->ApplyRemotePosition(
                            envelope.position);
                    }
                    break;
                case RemoteCursorMessageType::kReset:
                    cursorRenderState_->Reset();
                    break;
                }
                CursorVisualChanged();
            }

            void ResetRemoteCursor()
            {
                if (!cursorRenderState_) return;
                cursorRenderState_->Reset();
                CursorVisualChanged();
            }

            void SetConnectionStage(QString stage)
            {
                stage = stage.trimmed();
                if (stage.isEmpty()) {
                    stage = QStringLiteral("正在建立远程会话");
                }
                if (connectionStage_ == stage) {
                    return;
                }
                connectionStage_ = std::move(stage);
                update();
            }

            void BeginPresentationGeneration(std::uint64_t generation)
            {
                presentationGeneration_.store(
                    generation, std::memory_order_release);
                firstPresentationNotified_.store(
                    false, std::memory_order_release);
            }

            void NotifyFirstPresentation()
            {
                const std::uint64_t generation =
                    presentationGeneration_.load(
                        std::memory_order_acquire);
                if (generation == 0 ||
                    firstPresentationNotified_.exchange(
                        true, std::memory_order_acq_rel)) {
                    return;
                }
                if (firstPresentationCallback_) {
                    firstPresentationCallback_(generation);
                }
            }

            void SetTargetFrameRate(std::uint32_t framesPerSecond)
            {
                // Input is sampled at roughly twice the visible frame rate,
                // with a useful 120-Hz floor and a bounded 240-Hz ceiling.
                // Button-held dragging always uses the 240-Hz ceiling.
                pointerMoveRateLimitHz_ = std::clamp(
                    framesPerSecond * 2u, 120u, 240u);
                RemoteInputTelemetry::Instance()
                    .SetMoveDispatchRateLimit(pointerMoveRateLimitHz_);
            }

            void SetDragPointerSampleRate(std::uint32_t hertz)
            {
                if (std::find(
                        kSupportedDragPointerSampleRates.begin(),
                        kSupportedDragPointerSampleRates.end(),
                        hertz) ==
                    kSupportedDragPointerSampleRates.end()) {
                    hertz = 240;
                }
                dragPointerSampleRateHz_ = hertz;
                if (pointerMoveScheduler_) {
                    pointerMoveScheduler_
                        ->SetActiveDragSampleRate(hertz);
                }
            }

            void ShutdownInputScheduler()
            {
                if (pointerMoveScheduler_) {
                    pointerMoveScheduler_->Shutdown();
                }
            }

            void SetRemoteDisplayIdentity(
                std::uint32_t displayId,
                std::uint64_t layoutVersion)
            {
                if (remoteDisplayId_ == displayId &&
                    remoteDisplayLayoutVersion_ == layoutVersion) {
                    return;
                }
                ReleaseRemoteInputs();
                remoteDisplayId_ = displayId;
                remoteDisplayLayoutVersion_ = layoutVersion;
                ResetRemoteCursor();
            }

            void ReleaseRemoteInputs()
            {
                CancelPendingPointerMove();
                if (!controlEnabled_ || !inputSender_) {
                    return;
                }
                RemoteInputEvent input;
                input.type = RemoteInputMessageType::kReleaseAll;
                (void)inputSender_(input);
            }

            void OnFrame(const webrtc::VideoFrame& frame) override
            {
                std::optional<std::int64_t> latestPacketReceiveUs;
                for (const auto& packetInfo : frame.packet_infos()) {
                    const auto receiveTime = packetInfo.receive_time();
                    if (!receiveTime.IsFinite()) {
                        continue;
                    }
                    const std::int64_t receiveUs = receiveTime.us();
                    if (!latestPacketReceiveUs ||
                        receiveUs > *latestPacketReceiveUs) {
                        latestPacketReceiveUs = receiveUs;
                    }
                }
                if (latestPacketReceiveUs) {
                    const std::int64_t nowUs = webrtc::TimeMicros();
                    if (nowUs >= *latestPacketReceiveUs) {
                        VideoPresentationTelemetryRegistry::Instance()
                            .RecordReceiverPipeline(
                                presentationTelemetryId_,
                                static_cast<std::uint64_t>(
                                    nowUs - *latestPacketReceiveUs));
                    }
                }
                const auto source = frame.video_frame_buffer();
                if (source) {
                    pointerSourceWidth_.store(
                        source->width(), std::memory_order_release);
                    pointerSourceHeight_.store(
                        source->height(), std::memory_order_release);
                }
                if (nativeRenderingEnabled_ &&
                    D3D11NativeFrameBuffer::From(source.get())) {
                    QueueNativeFrame(source);
                    return;
                }
                if (source) {
                    if (nativeRenderingEnabled_ &&
                        !cpuI420D3D11PresentationFailed_.load(
                            std::memory_order_acquire)) {
                        QueueI420Frame(source);
                    }
                    else {
                        QueueCpuFrame(source);
                    }
                }
            }

        protected:
            void mouseMoveEvent(QMouseEvent* event) override
            {
                if (actualPixelPanning_) {
                    const QPoint delta =
                        (event->position() -
                            actualPixelPanStartPosition_).toPoint();
                    actualPixelPanOffset_ =
                        actualPixelPanStartOffset_ + delta;
                    UpdateNativeSurfaceGeometry();
                    RequestNativeRedraw();
                    update();
                    event->accept();
                    return;
                }
                const auto point = MapPoint(
                    event->position(), event->buttons() != Qt::NoButton);
                if (point && QueuePointerMove(
                    *point, MapMouseButtons(event->buttons()))) {
                    event->accept();
                    return;
                }
                QWidget::mouseMoveEvent(event);
            }

            void mousePressEvent(QMouseEvent* event) override
            {
                if (ActualPixelDisplayMode() &&
                    event->button() == Qt::LeftButton &&
                    event->modifiers().testFlag(Qt::AltModifier)) {
                    ReleaseRemoteInputs();
                    actualPixelPanning_ = true;
                    actualPixelPanStartPosition_ = event->position();
                    actualPixelPanStartOffset_ = actualPixelPanOffset_;
                    setCursor(Qt::ClosedHandCursor);
                    event->accept();
                    return;
                }
                const auto button = MapMouseButton(event->button());
                const auto point = MapPoint(event->position(), false);
                if (button && point && controlEnabled_) {
                    FlushPendingPointerMove();
                    setFocus(Qt::MouseFocusReason);
                    RemoteInputEvent input;
                    input.type = RemoteInputMessageType::kMouseButton;
                    input.displayId = remoteDisplayId_;
                    input.displayLayoutVersion =
                        remoteDisplayLayoutVersion_;
                    input.normalizedX = point->first;
                    input.normalizedY = point->second;
                    input.mouseButton = *button;
                    input.pressed = true;
                    input.pressedMouseButtons =
                        MapMouseButtons(event->buttons());
                    RemoteInputTelemetry::Instance()
                        .RecordGeneratedMouseButton();
                    if (inputSender_ && inputSender_(input)) {
                        if (pointerMoveScheduler_) {
                            RemoteInputEvent dragInput = input;
                            dragInput.type =
                                RemoteInputMessageType::kMouseMove;
                            pointerMoveScheduler_->BeginDrag(
                                dragInput,
                                dragPointerSampleRateHz_);
                        }
                        event->accept();
                        return;
                    }
                }
                QWidget::mousePressEvent(event);
            }

            void mouseReleaseEvent(QMouseEvent* event) override
            {
                if (actualPixelPanning_ &&
                    event->button() == Qt::LeftButton) {
                    actualPixelPanning_ = false;
                    setCursor(controlEnabled_
                        ? Qt::BlankCursor
                        : Qt::ArrowCursor);
                    event->accept();
                    return;
                }
                const auto button = MapMouseButton(event->button());
                const auto point = MapPoint(event->position(), true);
                if (button && point && controlEnabled_) {
                    FlushPendingPointerMove();
                    if (pointerMoveScheduler_) {
                        pointerMoveScheduler_->EndDrag();
                    }
                    RemoteInputEvent input;
                    input.type = RemoteInputMessageType::kMouseButton;
                    input.displayId = remoteDisplayId_;
                    input.displayLayoutVersion =
                        remoteDisplayLayoutVersion_;
                    input.normalizedX = point->first;
                    input.normalizedY = point->second;
                    input.mouseButton = *button;
                    input.pressed = false;
                    input.pressedMouseButtons =
                        MapMouseButtons(event->buttons());
                    RemoteInputTelemetry::Instance()
                        .RecordGeneratedMouseButton();
                    if (inputSender_ && inputSender_(input)) {
                        if (pointerMoveScheduler_ &&
                            input.pressedMouseButtons != 0) {
                            RemoteInputEvent dragInput = input;
                            dragInput.type =
                                RemoteInputMessageType::kMouseMove;
                            pointerMoveScheduler_->BeginDrag(
                                dragInput,
                                dragPointerSampleRateHz_);
                        }
                        event->accept();
                        return;
                    }
                }
                QWidget::mouseReleaseEvent(event);
            }

            void wheelEvent(QWheelEvent* event) override
            {
                const auto point = MapPoint(event->position(), false);
                if (point && controlEnabled_) {
                    FlushPendingPointerMove();
                    const QPoint delta = event->angleDelta();
                    RemoteInputEvent input;
                    input.type = RemoteInputMessageType::kMouseWheel;
                    input.displayId = remoteDisplayId_;
                    input.displayLayoutVersion =
                        remoteDisplayLayoutVersion_;
                    input.normalizedX = point->first;
                    input.normalizedY = point->second;
                    input.wheelDeltaX = ClampWheelDelta(delta.x());
                    input.wheelDeltaY = ClampWheelDelta(delta.y());
                    input.pressedMouseButtons =
                        MapMouseButtons(event->buttons());
                    if ((input.wheelDeltaX != 0 || input.wheelDeltaY != 0) &&
                        inputSender_) {
                        RemoteInputTelemetry::Instance()
                            .RecordGeneratedMouseWheel();
                        if (inputSender_(input)) {
                            event->accept();
                            return;
                        }
                    }
                }
                QWidget::wheelEvent(event);
            }

            void keyPressEvent(QKeyEvent* event) override
            {
                if (controlEnabled_ && !event->isAutoRepeat() &&
                    event->key() == Qt::Key_V &&
                    event->modifiers().testFlag(Qt::ControlModifier) &&
                    !event->modifiers().testFlag(Qt::ShiftModifier) &&
                    pasteSender_) {
                    QStringList clipboardFiles;
                    if (const QClipboard* clipboard =
                            QApplication::clipboard()) {
                        if (const QMimeData* mimeData =
                                clipboard->mimeData(
                                    QClipboard::Clipboard);
                            mimeData && mimeData->hasUrls()) {
                            for (const QUrl& url : mimeData->urls()) {
                                if (!url.isLocalFile()) continue;
                                const QString path = url.toLocalFile();
                                if (!path.isEmpty()) {
                                    clipboardFiles.push_back(path);
                                }
                            }
                        }
                    }
                    // File URLs are captured synchronously on the UI thread.
                    // This remains stable across repeated Ctrl+V even when an
                    // RDP/delayed IDataObject changes its native formats after
                    // the first read. Text and virtual formats still use the
                    // one-shot native clipboard service below.
                    if (pasteSender_(clipboardFiles, true)) {
                        // Ctrl may already have reached the controlled
                        // machine. Release it before the asynchronous
                        // transfer; a fresh Ctrl+V is injected only after the
                        // remote clipboard is ready.
                        ReleaseRemoteInputs();
                        suppressPasteKeyRelease_ = true;
                        event->accept();
                        return;
                    }
                }
                if (SendKeyEvent(event, true)) {
                    event->accept();
                    return;
                }
                QWidget::keyPressEvent(event);
            }

            void keyReleaseEvent(QKeyEvent* event) override
            {
                if (suppressPasteKeyRelease_ &&
                    event->key() == Qt::Key_V) {
                    suppressPasteKeyRelease_ = false;
                    event->accept();
                    return;
                }
                // Qt may emit synthetic auto-repeat releases. The real release follows
                // with isAutoRepeat() == false, so sending the synthetic one would make
                // held keys flicker on the controlled machine.
                if (controlEnabled_ && event->isAutoRepeat()) {
                    event->accept();
                    return;
                }
                if (SendKeyEvent(event, false)) {
                    event->accept();
                    return;
                }
                QWidget::keyReleaseEvent(event);
            }

            void dragEnterEvent(QDragEnterEvent* event) override
            {
                if (controlEnabled_ && pasteSender_ && event->mimeData() &&
                    event->mimeData()->hasUrls()) {
                    const QList<QUrl> urls = event->mimeData()->urls();
                    const bool hasLocalFile = std::any_of(
                        urls.begin(), urls.end(),
                        [](const QUrl& url) { return url.isLocalFile(); });
                    if (hasLocalFile) {
                        event->setDropAction(Qt::CopyAction);
                        event->accept();
                        return;
                    }
                }
                QWidget::dragEnterEvent(event);
            }

            void dropEvent(QDropEvent* event) override
            {
                QStringList files;
                if (event->mimeData()) {
                    const QList<QUrl> urls = event->mimeData()->urls();
                    for (const QUrl& url : urls) {
                        if (url.isLocalFile() && !url.toLocalFile().isEmpty()) {
                            files.push_back(url.toLocalFile());
                        }
                    }
                }
                if (!files.isEmpty() && controlEnabled_ && pasteSender_) {
                    if (pasteSender_(files, false)) {
                        FocusRemotePasteTarget(event->position());
                        event->setDropAction(Qt::CopyAction);
                        event->accept();
                        return;
                    }
                }
                QWidget::dropEvent(event);
            }

            void focusOutEvent(QFocusEvent* event) override
            {
                actualPixelPanning_ = false;
                ReleaseRemoteInputs();
                setCursor(controlEnabled_
                    ? Qt::BlankCursor
                    : Qt::ArrowCursor);
                QWidget::focusOutEvent(event);
            }

            void resizeEvent(QResizeEvent* event) override
            {
                QWidget::resizeEvent(event);
                UpdateCpuCanvasMetrics();
                pointerStretchToCanvas_.store(
                    false,
                    std::memory_order_release);
                UpdateCpuConversionInterval();
                UpdateNativeSurfaceGeometry();
                RequestNativeRedraw();
            }

            bool event(QEvent* event) override
            {
                const bool handled = QWidget::event(event);
#if QT_VERSION >= QT_VERSION_CHECK(6, 6, 0)
                if (event &&
                    event->type() == QEvent::DevicePixelRatioChange) {
                    // A cross-monitor move can change DPR without changing
                    // the QWidget's logical dimensions.
                    UpdateCpuCanvasMetrics();
                }
#endif
                return handled;
            }

            void paintEvent(QPaintEvent*) override
            {
                QPainter painter(this);
                painter.setRenderHint(QPainter::Antialiasing);

                if (nativeFrameActive_) {
                    painter.fillRect(rect(), Qt::black);
                    return;
                }
                if (!frameImage_.isNull()) {
                    painter.fillRect(rect(), Qt::black);
                    const QRect target = VideoContentRect();
                    // The CPU worker normally hands Qt an image already sized
                    // for this content rectangle. Avoid enabling the costly
                    // painter scaling path for the steady-state 1:1 blit.
                    painter.setRenderHint(
                        QPainter::SmoothPixmapTransform,
                        target.size() !=
                            frameImage_.deviceIndependentSize().toSize());
                    const auto drawStartedAt =
                        std::chrono::steady_clock::now();
                    painter.drawImage(target, frameImage_);
                    if (cursorRenderState_) {
                        const QSize cursorSourceSize(
                            pointerSourceWidth_.load(
                                std::memory_order_acquire),
                            pointerSourceHeight_.load(
                                std::memory_order_acquire));
                        cursorRenderState_->Paint(
                            painter, target,
                            cursorSourceSize.isValid()
                                ? cursorSourceSize
                                : frameImage_.size());
                    }
                    if (frameImageSequence_ != 0 &&
                        frameImageSequence_ != lastPresentedCpuSequence_) {
                        const auto drawUs = static_cast<std::uint64_t>(
                            std::chrono::duration_cast<
                                std::chrono::microseconds>(
                                std::chrono::steady_clock::now() -
                                drawStartedAt)
                                .count());
                        lastPresentedCpuSequence_ = frameImageSequence_;
                        VideoPresentationTelemetryRegistry::Instance()
                            .RecordPresented(presentationTelemetryId_,
                                VideoPresentationPath::kCpuQt,
                                drawUs, 0);
                        NotifyFirstPresentation();
                    }
                    return;
                }

                QLinearGradient background(0, 0, width(), height());
                background.setColorAt(0.0, QColor(24, 49, 91));
                background.setColorAt(0.52, QColor(34, 79, 139));
                background.setColorAt(1.0, QColor(16, 30, 56));
                painter.fillRect(rect(), background);

                QPainterPath glow;
                glow.addEllipse(QPointF(width() * 0.73, height() * 0.38),
                    width() * 0.29, height() * 0.48);
                QLinearGradient glowGradient(width() * 0.5, height() * 0.2,
                    width(), height() * 0.75);
                glowGradient.setColorAt(0.0, QColor(96, 143, 255, 155));
                glowGradient.setColorAt(1.0, QColor(82, 93, 217, 15));
                painter.fillPath(glow, glowGradient);

                painter.setPen(QPen(QColor(255, 255, 255, 25), 1));
                for (int x = -height(); x < width(); x += 84) {
                    painter.drawLine(x, height(), x + height(), 0);
                }

                const int taskbarHeight = 42;
                painter.fillRect(0, height() - taskbarHeight, width(), taskbarHeight,
                    QColor(9, 17, 30, 220));
                painter.setBrush(QColor(90, 108, 242));
                painter.setPen(Qt::NoPen);
                painter.drawRoundedRect(width() / 2 - 58, height() - 32, 22, 22, 5, 5);
                painter.setBrush(QColor(232, 238, 248, 210));
                painter.drawEllipse(width() / 2 - 25, height() - 30, 18, 18);
                painter.drawRoundedRect(width() / 2 + 5, height() - 30, 24, 18, 4, 4);
                painter.setBrush(QColor(232, 238, 248, 170));
                painter.drawRoundedRect(width() / 2 + 42, height() - 30, 20, 18, 4, 4);

                QRectF overlay(24, 22, 190, 34);
                painter.setBrush(QColor(8, 14, 25, 150));
                painter.setPen(QPen(QColor(255, 255, 255, 30)));
                painter.drawRoundedRect(overlay, 8, 8);
                painter.setPen(QColor(221, 229, 242));
                QFont displayFont = QApplication::font();
                displayFont.setPixelSize(12);
                displayFont.setWeight(QFont::Medium);
                painter.setFont(displayFont);
                painter.drawText(overlay.adjusted(12, 0, -10, 0), Qt::AlignVCenter,
                    QStringLiteral("远程画面  ·  连接中"));

                const int panelWidth = qMin(480, width() - 80);
                QRectF emptyState((width() - panelWidth) / 2.0,
                    (height() - 155) / 2.0 - 12, panelWidth, 155);
                painter.setBrush(QColor(8, 16, 30, 165));
                painter.setPen(QPen(QColor(255, 255, 255, 32)));
                painter.drawRoundedRect(emptyState, 16, 16);

                painter.setPen(QColor(245, 248, 253));
                QFont previewTitleFont = QApplication::font();
                previewTitleFont.setPixelSize(21);
                previewTitleFont.setWeight(QFont::DemiBold);
                painter.setFont(previewTitleFont);
                painter.drawText(emptyState.adjusted(25, 28, -25, -70),
                    Qt::AlignHCenter | Qt::AlignTop,
                    QStringLiteral("正在连接中"));
                painter.setPen(QColor(167, 181, 203));
                QFont previewHintFont = QApplication::font();
                previewHintFont.setPixelSize(13);
                painter.setFont(previewHintFont);
                painter.drawText(emptyState.adjusted(34, 75, -34, -24),
                    Qt::AlignHCenter | Qt::AlignTop,
                    connectionStage_);
            }

        private:
