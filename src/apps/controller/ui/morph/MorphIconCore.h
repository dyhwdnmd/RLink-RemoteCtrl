// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#pragma once

#include <QColor>
#include <QPainterPath>
#include <QPointF>
#include <QRectF>
#include <QString>

#include <vector>

class QPainter;

namespace remotec::ui::morph {

struct CubicPath {
    // p0, c1, c2, p1, c1, c2, p2, ...; adjacent cubics share an endpoint.
    std::vector<QPointF> points;
    bool closed = false;
};

struct SampledPath {
    std::vector<QPointF> points;
    bool closed = false;
};

struct Similarity {
    double theta = 0.0;
    double sigma = 1.0;
    double residual = 0.0;
};

struct PlanItem {
    std::vector<QPointF> source;
    std::vector<QPointF> sourceCentered;
    std::vector<QPointF> targetAligned;
    std::vector<QPointF> targetOriented;
    QPointF sourceCentroid;
    QPointF targetCentroid;
    double theta = 0.0;
    double logSigma = 0.0;
    double residual = 0.0;
    bool closed = false;
    bool hasBlockTransport = false;
    QPointF blockOffset;
    QPointF blockDrift;
};

struct MorphPlan {
    std::vector<PlanItem> items;
    int sampleCount = 0;

    [[nodiscard]] bool isValid() const noexcept
    {
        return sampleCount > 1 && !items.empty();
    }
};

// MIT-licensed C++ port of Morphicons 1.7.1 core geometry. It intentionally
// has no DOM/web dependency: SVG resources are parsed once, then only sampled
// point clouds and a cached plan are used while painting.
class MorphIconCore final {
public:
    bool configure(const QString& sourceSvgResource,
                   const QString& targetSvgResource,
                   int sampleCount = 64,
                   QString* errorMessage = nullptr);

    [[nodiscard]] bool isValid() const noexcept { return plan_.isValid(); }
    [[nodiscard]] const MorphPlan& plan() const noexcept { return plan_; }

    void interpolate(double progress,
                     std::vector<std::vector<QPointF>>& output) const;
    [[nodiscard]] QPainterPath painterPath(double progress) const;
    void paint(QPainter& painter,
               const QRectF& targetRect,
               const QColor& color,
               double progress,
               qreal strokeWidth = 2.0) const;

    static bool loadSvg(const QString& resource,
                        std::vector<CubicPath>& paths,
                        QRectF& viewBox,
                        QString* errorMessage = nullptr);
    static std::vector<SampledPath> resample(
        const std::vector<CubicPath>& paths,
        int sampleCount = 64);
    static MorphPlan buildPlan(const std::vector<SampledPath>& source,
                               const std::vector<SampledPath>& target);
    static Similarity procrustes(const std::vector<QPointF>& source,
                                 const std::vector<QPointF>& target,
                                 const QPointF& sourceCentroid,
                                 const QPointF& targetCentroid);

private:
    MorphPlan plan_;
    QRectF sharedViewBox_{0.0, 0.0, 24.0, 24.0};
};

class Spring final {
public:
    void configure(double stiffness, double damping) noexcept;
    void start() noexcept;
    bool step(double seconds) noexcept;

    [[nodiscard]] double value() const noexcept { return value_; }
    [[nodiscard]] double velocity() const noexcept { return velocity_; }

private:
    double value_ = 1.0;
    double velocity_ = 0.0;
    double stiffness_ = 250.0;
    double damping_ = 24.0;
};

} // namespace remotec::ui::morph
