// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

            void UpdateNativeSurfaceGeometry()
            {
                if (nativeSurface_ && nativeFrameActive_) {
                    nativeSurface_->setGeometry(VideoContentRect());
                }
            }

            QRect VideoContentRect() const
            {
                const QSize sourceSize = VideoSourceSize();
                if (!sourceSize.isValid()) {
                    return {};
                }
                if (actualPixelDisplayMode_.load(
                        std::memory_order_acquire)) {
                    const qreal dpr = (std::max)(
                        devicePixelRatioF(), 0.5);
                    const QSize targetSize(
                        (std::max)(1, static_cast<int>(std::lround(
                            sourceSize.width() / dpr))),
                        (std::max)(1, static_cast<int>(std::lround(
                            sourceSize.height() / dpr))));
                    int left = (width() - targetSize.width()) / 2 +
                        actualPixelPanOffset_.x();
                    int top = (height() - targetSize.height()) / 2 +
                        actualPixelPanOffset_.y();
                    left = targetSize.width() <= width()
                        ? (width() - targetSize.width()) / 2
                        : std::clamp(left,
                            width() - targetSize.width(), 0);
                    top = targetSize.height() <= height()
                        ? (height() - targetSize.height()) / 2
                        : std::clamp(top,
                            height() - targetSize.height(), 0);
                    return QRect(QPoint(left, top), targetSize);
                }
                const QSize targetSize =
                    sourceSize.scaled(size(), Qt::KeepAspectRatio);
                return QRect(
                    QPoint((width() - targetSize.width()) / 2,
                        (height() - targetSize.height()) / 2),
                    targetSize);
            }

            QSize VideoSourceSize() const
            {
                const QSize reportedSourceSize(
                    pointerSourceWidth_.load(std::memory_order_acquire),
                    pointerSourceHeight_.load(std::memory_order_acquire));
                if (reportedSourceSize.isValid()) {
                    return reportedSourceSize;
                }
                return nativeFrameActive_
                    ? nativeFrameSize_
                    : frameImage_.size();
            }

            std::optional<RemoteInputEvent>
                SampleCurrentDragPointer(
                    const RemoteInputEvent& fallback) const
            {
                if (!pointerSamplingWindow_ ||
                    !IsWindow(pointerSamplingWindow_)) {
                    return std::nullopt;
                }
                POINT pointer{};
                RECT client{};
                if (!GetCursorPos(&pointer) ||
                    !ScreenToClient(
                        pointerSamplingWindow_, &pointer) ||
                    !GetClientRect(pointerSamplingWindow_, &client)) {
                    return std::nullopt;
                }

                const int clientWidth = client.right - client.left;
                const int clientHeight = client.bottom - client.top;
                const int sourceWidth = pointerSourceWidth_.load(
                    std::memory_order_acquire);
                const int sourceHeight = pointerSourceHeight_.load(
                    std::memory_order_acquire);
                if (clientWidth <= 0 || clientHeight <= 0 ||
                    sourceWidth <= 0 || sourceHeight <= 0) {
                    return std::nullopt;
                }

                int contentLeft = 0;
                int contentTop = 0;
                int contentWidth = clientWidth;
                int contentHeight = clientHeight;
                if (!pointerStretchToCanvas_.load(
                        std::memory_order_acquire)) {
                    const std::int64_t widthLimitedHeight =
                        static_cast<std::int64_t>(clientWidth) *
                        sourceHeight / sourceWidth;
                    if (widthLimitedHeight <= clientHeight) {
                        contentHeight = static_cast<int>(
                            widthLimitedHeight);
                        contentTop =
                            (clientHeight - contentHeight) / 2;
                    } else {
                        contentWidth = static_cast<int>(
                            static_cast<std::int64_t>(
                                clientHeight) *
                            sourceWidth / sourceHeight);
                        contentLeft =
                            (clientWidth - contentWidth) / 2;
                    }
                }
                if (contentWidth <= 1 || contentHeight <= 1) {
                    return std::nullopt;
                }

                pointer.x = std::clamp<LONG>(
                    pointer.x,
                    contentLeft,
                    contentLeft + contentWidth - 1);
                pointer.y = std::clamp<LONG>(
                    pointer.y,
                    contentTop,
                    contentTop + contentHeight - 1);
                const auto normalize = [](
                    LONG position, int origin, int extent) {
                    const auto offset = static_cast<std::uint64_t>(
                        position - origin);
                    return static_cast<std::uint16_t>(
                        offset * 65'535ULL /
                        static_cast<std::uint64_t>(extent - 1));
                };

                RemoteInputEvent sampled = fallback;
                sampled.normalizedX = normalize(
                    pointer.x, contentLeft, contentWidth);
                sampled.normalizedY = normalize(
                    pointer.y, contentTop, contentHeight);
                return sampled;
            }

            std::optional<std::pair<std::uint16_t, std::uint16_t>> MapPoint(
                const QPointF& point,
                bool clampToContent) const
            {
                if (!controlEnabled_) {
                    return std::nullopt;
                }
                const QRect content = VideoContentRect();
                if (content.width() <= 0 || content.height() <= 0) {
                    return std::nullopt;
                }

                QPointF mapped = point;
                if (!content.contains(mapped.toPoint())) {
                    if (!clampToContent) {
                        return std::nullopt;
                    }
                    mapped.setX(std::clamp(
                        mapped.x(), static_cast<qreal>(content.left()),
                        static_cast<qreal>(content.right())));
                    mapped.setY(std::clamp(
                        mapped.y(), static_cast<qreal>(content.top()),
                        static_cast<qreal>(content.bottom())));
                }

                const auto normalize = [](qreal position, int origin, int extent) {
                    if (extent <= 1) {
                        return std::uint16_t{ 0 };
                    }
                    const double ratio = std::clamp(
                        (position - origin) / static_cast<double>(extent - 1),
                        0.0, 1.0);
                    return static_cast<std::uint16_t>(
                        std::lround(ratio * 65535.0));
                    };
                return std::pair{
                    normalize(mapped.x(), content.left(), content.width()),
                    normalize(mapped.y(), content.top(), content.height()) };
            }

            static std::optional<RemoteMouseButton> MapMouseButton(
                Qt::MouseButton button)
            {
                switch (button) {
                case Qt::LeftButton:
                    return RemoteMouseButton::kLeft;
                case Qt::RightButton:
                    return RemoteMouseButton::kRight;
                case Qt::MiddleButton:
                    return RemoteMouseButton::kMiddle;
                case Qt::BackButton:
                    return RemoteMouseButton::kX1;
                case Qt::ForwardButton:
                    return RemoteMouseButton::kX2;
                default:
                    return std::nullopt;
                }
            }

            static std::uint8_t MapMouseButtons(Qt::MouseButtons buttons)
            {
                std::uint8_t mask = 0;
                if (buttons.testFlag(Qt::LeftButton)) {
                    mask |= 1u << 0;
                }
                if (buttons.testFlag(Qt::RightButton)) {
                    mask |= 1u << 1;
                }
                if (buttons.testFlag(Qt::MiddleButton)) {
                    mask |= 1u << 2;
                }
                if (buttons.testFlag(Qt::BackButton)) {
                    mask |= 1u << 3;
                }
                if (buttons.testFlag(Qt::ForwardButton)) {
                    mask |= 1u << 4;
                }
                return mask;
            }

            static std::int16_t ClampWheelDelta(int value)
            {
                return static_cast<std::int16_t>(std::clamp(
                    value,
                    static_cast<int>(std::numeric_limits<std::int16_t>::min()),
                    static_cast<int>(std::numeric_limits<std::int16_t>::max())));
            }

            static bool IsExtendedVirtualKey(std::uint32_t virtualKey)
            {
                switch (virtualKey) {
                case VK_RCONTROL:
                case VK_RMENU:
                case VK_INSERT:
                case VK_DELETE:
                case VK_HOME:
                case VK_END:
                case VK_PRIOR:
                case VK_NEXT:
                case VK_LEFT:
                case VK_RIGHT:
                case VK_UP:
                case VK_DOWN:
                case VK_NUMLOCK:
                case VK_DIVIDE:
                case VK_SNAPSHOT:
                case VK_LWIN:
                case VK_RWIN:
                case VK_APPS:
                    return true;
                default:
                    return false;
                }
            }

            bool QueuePointerMove(
                const std::pair<std::uint16_t, std::uint16_t>& point,
                std::uint8_t pressedMouseButtons)
            {
                if (!controlEnabled_ || !inputSender_ ||
                    !pointerMoveScheduler_) {
                    return false;
                }
                RemoteInputEvent input;
                input.type = RemoteInputMessageType::kMouseMove;
                input.displayId = remoteDisplayId_;
                input.displayLayoutVersion =
                    remoteDisplayLayoutVersion_;
                input.normalizedX = point.first;
                input.normalizedY = point.second;
                input.pressedMouseButtons = pressedMouseButtons;
                auto& telemetry = RemoteInputTelemetry::Instance();
                telemetry.RecordGeneratedMouseMove();
                if (cursorRenderState_) {
                    RemoteCursorPosition predicted;
                    predicted.displayId = remoteDisplayId_;
                    predicted.displayLayoutVersion =
                        remoteDisplayLayoutVersion_;
                    predicted.normalizedX = point.first;
                    predicted.normalizedY = point.second;
                    predicted.visible = true;
                    cursorRenderState_->SetPosition(predicted, true);
                    CursorVisualChanged();
                }
                const std::uint32_t rateLimitHz =
                    pressedMouseButtons
                    ? dragPointerSampleRateHz_
                    : pointerMoveRateLimitHz_;
                return pointerMoveScheduler_->Queue(
                    input, rateLimitHz);
            }

            void FlushPendingPointerMove()
            {
                if (pointerMoveScheduler_) {
                    (void)pointerMoveScheduler_->Flush();
                }
            }

            void CancelPendingPointerMove()
            {
                if (pointerMoveScheduler_) {
                    pointerMoveScheduler_->Cancel();
                }
            }

            bool SendKeyEvent(QKeyEvent* event, bool pressed)
            {
                if (!controlEnabled_ || !inputSender_) {
                    return false;
                }
                const std::uint32_t nativeVirtualKey = event->nativeVirtualKey();
                const std::uint32_t nativeScanCode = event->nativeScanCode();
                if ((nativeVirtualKey == 0 && nativeScanCode == 0) ||
                    nativeVirtualKey > std::numeric_limits<std::uint16_t>::max()) {
                    return false;
                }

                RemoteInputEvent input;
                input.type = RemoteInputMessageType::kKey;
                input.displayId = 0;
                input.virtualKey = static_cast<std::uint16_t>(nativeVirtualKey);
                input.scanCode = static_cast<std::uint16_t>(nativeScanCode & 0xffu);
                input.extendedKey = (nativeScanCode & 0x100u) != 0 ||
                    IsExtendedVirtualKey(nativeVirtualKey);
                input.pressed = pressed;
                input.repeat = pressed && event->isAutoRepeat();
                RemoteInputTelemetry::Instance().RecordGeneratedKey();
                return inputSender_(input);
            }

            void FocusRemotePasteTarget(const QPointF& position)
            {
                const auto point = MapPoint(position, false);
                if (!point || !controlEnabled_ || !inputSender_) return;
                FlushPendingPointerMove();
                setFocus(Qt::MouseFocusReason);
                for (const bool pressed : {true, false}) {
                    RemoteInputEvent input;
                    input.type = RemoteInputMessageType::kMouseButton;
                    input.displayId = remoteDisplayId_;
                    input.displayLayoutVersion = remoteDisplayLayoutVersion_;
                    input.normalizedX = point->first;
                    input.normalizedY = point->second;
                    input.mouseButton = RemoteMouseButton::kLeft;
                    input.pressed = pressed;
                    input.pressedMouseButtons = pressed ? 1u : 0u;
                    RemoteInputTelemetry::Instance()
                        .RecordGeneratedMouseButton();
                    if (!inputSender_(input)) break;
                }
            }
