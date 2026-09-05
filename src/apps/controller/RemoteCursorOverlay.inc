// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

        // Thread-safe cursor state shared by the Qt paint path and the
        // dedicated D3D11 presentation thread. This is deliberately not a
        // QWidget: moving a translucent native window at pointer frequency
        // races Qt window teardown and cannot compose above a child swapchain.
        class RemoteCursorRenderState final {
        public:
            struct Snapshot {
                QImage image;
                QPoint hotspot;
                QPoint normalizedPosition;
                std::uint64_t shapeRevision = 0;
                bool visible = false;
            };

            RemoteCursorRenderState() { BuildFallbackArrow(); }

            void SetShape(const RemoteCursorShape& shape)
            {
                const auto expected = static_cast<std::size_t>(shape.width) *
                    shape.height * 4;
                if (shape.width == 0 || shape.height == 0 ||
                    shape.premultipliedBgra.size() != expected) return;
                QImage image(shape.premultipliedBgra.data(), shape.width,
                    shape.height, shape.width * 4,
                    QImage::Format_ARGB32_Premultiplied);
                image = image.copy();
                if (shape.invertMask.size() ==
                        static_cast<std::size_t>(shape.width) * shape.height) {
                    for (int y = 0; y < shape.height; ++y) {
                        auto* line = reinterpret_cast<QRgb*>(image.scanLine(y));
                        for (int x = 0; x < shape.width; ++x) {
                            if (shape.invertMask[
                                    static_cast<std::size_t>(y) *
                                        shape.width + x]) {
                                // XOR pixels have no alpha-blend equivalent.
                                // Keep them visible and deterministic.
                                line[x] = qRgba(255, 255, 255, 255);
                            }
                        }
                    }
                }
                std::lock_guard lock(mutex_);
                image_ = std::move(image);
                hotspot_ = QPoint(shape.hotspotX, shape.hotspotY);
                ++shapeRevision_;
            }

            void SetPosition(const RemoteCursorPosition& position,
                             bool predicted)
            {
                std::lock_guard lock(mutex_);
                normalizedPosition_ = QPoint(
                    position.normalizedX, position.normalizedY);
                visible_ = position.visible;
                if (predicted) {
                    localPredictionStartedAt_ =
                        std::chrono::steady_clock::now();
                    hasLocalPrediction_ = true;
                }
            }

            void ApplyRemotePosition(const RemoteCursorPosition& position)
            {
                std::lock_guard lock(mutex_);
                if (hasLocalPrediction_ &&
                    std::chrono::steady_clock::now() -
                            localPredictionStartedAt_ <
                        std::chrono::milliseconds(90)) {
                    const int dx = std::abs(
                        normalizedPosition_.x() - position.normalizedX);
                    const int dy = std::abs(
                        normalizedPosition_.y() - position.normalizedY);
                    if (dx > 256 || dy > 256) return;
                }
                normalizedPosition_ = QPoint(
                    position.normalizedX, position.normalizedY);
                visible_ = position.visible;
            }

            void Reset()
            {
                std::lock_guard lock(mutex_);
                visible_ = false;
                normalizedPosition_ = {};
                hasLocalPrediction_ = false;
            }

            void SetRenderingEnabled(bool enabled)
            {
                std::lock_guard lock(mutex_);
                renderingEnabled_ = enabled;
            }

            Snapshot GetSnapshot() const
            {
                std::lock_guard lock(mutex_);
                return Snapshot{ image_, hotspot_, normalizedPosition_,
                    shapeRevision_, visible_ && renderingEnabled_ };
            }

            void Paint(QPainter& painter, const QRect& content,
                       const QSize& sourceSize) const
            {
                const Snapshot snapshot = GetSnapshot();
                const QRect target = TargetRect(snapshot, content, sourceSize);
                if (!snapshot.visible || snapshot.image.isNull() ||
                    !target.isValid()) return;
                painter.save();
                painter.setCompositionMode(QPainter::CompositionMode_SourceOver);
                painter.setRenderHint(QPainter::SmoothPixmapTransform, true);
                painter.drawImage(target, snapshot.image);
                painter.restore();
            }

        private:
            static QRect TargetRect(const Snapshot& snapshot,
                                    const QRect& content,
                                    const QSize& sourceSize)
            {
                if (!content.isValid() || !sourceSize.isValid() ||
                    snapshot.image.isNull()) return {};
                const qreal scaleX = static_cast<qreal>(content.width()) /
                    sourceSize.width();
                const qreal scaleY = static_cast<qreal>(content.height()) /
                    sourceSize.height();
                const qreal x = content.left() +
                    snapshot.normalizedPosition.x() / 65535.0 *
                        (content.width() - 1);
                const qreal y = content.top() +
                    snapshot.normalizedPosition.y() / 65535.0 *
                        (content.height() - 1);
                return QRect(
                    QPoint(qRound(x - snapshot.hotspot.x() * scaleX),
                           qRound(y - snapshot.hotspot.y() * scaleY)),
                    QSize((std::max)(1,
                              qRound(snapshot.image.width() * scaleX)),
                          (std::max)(1,
                              qRound(snapshot.image.height() * scaleY))));
            }

            void BuildFallbackArrow()
            {
                QImage image(24, 32, QImage::Format_ARGB32_Premultiplied);
                image.fill(Qt::transparent);
                QPainter painter(&image);
                painter.setRenderHint(QPainter::Antialiasing);
                QPainterPath arrow;
                arrow.moveTo(2, 1);
                arrow.lineTo(2, 25);
                arrow.lineTo(8, 19);
                arrow.lineTo(13, 30);
                arrow.lineTo(18, 27);
                arrow.lineTo(13, 17);
                arrow.lineTo(22, 17);
                arrow.closeSubpath();
                painter.setPen(QPen(Qt::black, 3, Qt::SolidLine,
                                    Qt::SquareCap, Qt::MiterJoin));
                painter.setBrush(Qt::white);
                painter.drawPath(arrow);
                std::lock_guard lock(mutex_);
                image_ = std::move(image);
                hotspot_ = QPoint(2, 1);
                ++shapeRevision_;
            }

            mutable std::mutex mutex_;
            QImage image_;
            QPoint hotspot_;
            QPoint normalizedPosition_;
            std::chrono::steady_clock::time_point localPredictionStartedAt_{};
            std::uint64_t shapeRevision_ = 0;
            bool hasLocalPrediction_ = false;
            bool visible_ = false;
            bool renderingEnabled_ = false;
        };
