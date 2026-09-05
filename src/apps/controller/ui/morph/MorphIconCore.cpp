// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#include "MorphIconCore.h"

#include <QFile>
#include <QPainter>
#include <QPen>
#include <QXmlStreamReader>

#include <algorithm>
#include <array>
#include <cmath>
#include <functional>
#include <limits>
#include <mutex>
#include <numeric>
#include <stdexcept>
#include <utility>
#include <unordered_map>

namespace remotec::ui::morph {
namespace {

constexpr double kPi = 3.1415926535897932384626433832795;
constexpr double kTau = 2.0 * kPi;
constexpr double kCornerThreshold = kPi / 8.0;
constexpr double kLengthWeight = 0.35;
constexpr double kRotationTieBreak = 0.05;
constexpr double kGlobalResidualThreshold = 5e-3;
constexpr double kEpsilon = 1e-12;

struct CachedMorphPlan {
    MorphPlan plan;
    QRectF viewBox;
};

std::mutex& morphPlanCacheMutex()
{
    static std::mutex mutex;
    return mutex;
}

std::unordered_map<std::string, CachedMorphPlan>& morphPlanCache()
{
    static std::unordered_map<std::string, CachedMorphPlan> cache;
    return cache;
}

double cross(const QPointF& a, const QPointF& b)
{
    return a.x() * b.y() - a.y() * b.x();
}

double dot(const QPointF& a, const QPointF& b)
{
    return a.x() * b.x() + a.y() * b.y();
}

double norm(const QPointF& p)
{
    return std::hypot(p.x(), p.y());
}

QPointF centroid(const std::vector<QPointF>& points)
{
    QPointF value;
    for (const QPointF& point : points) {
        value += point;
    }
    return points.empty() ? value : value / double(points.size());
}

double polylineLength(const std::vector<QPointF>& points)
{
    double length = 0.0;
    for (size_t index = 1; index < points.size(); ++index) {
        length += norm(points[index] - points[index - 1]);
    }
    return length;
}

std::vector<QPointF> reversePoints(const std::vector<QPointF>& points)
{
    return {points.rbegin(), points.rend()};
}

std::vector<QPointF> rotatePoints(const std::vector<QPointF>& points, int offset)
{
    std::vector<QPointF> output(points.size());
    const int count = int(points.size());
    for (int index = 0; index < count; ++index) {
        output[index] = points[(index + offset) % count];
    }
    return output;
}

struct RawSegment {
    QChar command;
    std::array<double, 7> values{};
};

struct RawSubpath {
    QPointF start;
    std::vector<RawSegment> segments;
    bool closed = false;
};

class PathParser final {
public:
    explicit PathParser(QString data) : data_(std::move(data)) {}

    std::vector<RawSubpath> parse()
    {
        while (true) {
            skipSeparators();
            if (atEnd()) {
                break;
            }
            if (isCommand(peek())) {
                command_ = take();
            }
            else if (command_.isNull()) {
                fail(QStringLiteral("路径必须从 M/m 开始"));
            }
            else if (command_ == QLatin1Char('M')) {
                command_ = QLatin1Char('L');
            }
            else if (command_ == QLatin1Char('m')) {
                command_ = QLatin1Char('l');
            }
            else if (command_.toUpper() == QLatin1Char('Z')) {
                fail(QStringLiteral("Z 后存在多余数据"));
            }

            const bool relative = command_.isLower();
            switch (command_.toUpper().unicode()) {
            case 'M': {
                started_ = true;
                current_ = point(relative);
                subpathStart_ = current_;
                paths_.push_back({current_, {}, false});
                active_ = &paths_.back();
                previousKind_ = QChar();
                break;
            }
            case 'L': {
                const QPointF end = point(relative);
                append('L', {end.x(), end.y()});
                current_ = end;
                previousKind_ = QChar();
                break;
            }
            case 'H': {
                const double x = number() + (relative ? current_.x() : 0.0);
                append('L', {x, current_.y()});
                current_.setX(x);
                previousKind_ = QChar();
                break;
            }
            case 'V': {
                const double y = number() + (relative ? current_.y() : 0.0);
                append('L', {current_.x(), y});
                current_.setY(y);
                previousKind_ = QChar();
                break;
            }
            case 'C':
            case 'S': {
                const QPointF origin = current_;
                QPointF control1;
                if (command_.toUpper() == QLatin1Char('C')) {
                    control1 = pointFromOrigin(relative, origin);
                }
                else {
                    control1 = previousKind_ == QLatin1Char('C')
                        ? current_ * 2.0 - previousControl_
                        : current_;
                }
                const QPointF control2 = pointFromOrigin(relative, origin);
                const QPointF end = pointFromOrigin(relative, origin);
                append('C', {control1.x(), control1.y(), control2.x(),
                             control2.y(), end.x(), end.y()});
                previousControl_ = control2;
                current_ = end;
                previousKind_ = QLatin1Char('C');
                break;
            }
            case 'Q':
            case 'T': {
                const QPointF origin = current_;
                QPointF control;
                if (command_.toUpper() == QLatin1Char('Q')) {
                    control = pointFromOrigin(relative, origin);
                }
                else {
                    control = previousKind_ == QLatin1Char('Q')
                        ? current_ * 2.0 - previousControl_
                        : current_;
                }
                const QPointF end = pointFromOrigin(relative, origin);
                append('Q', {control.x(), control.y(), end.x(), end.y()});
                previousControl_ = control;
                current_ = end;
                previousKind_ = QLatin1Char('Q');
                break;
            }
            case 'A': {
                const QPointF origin = current_;
                const double rx = number();
                const double ry = number();
                const double rotation = number();
                const int large = flag();
                const int sweep = flag();
                const QPointF end = pointFromOrigin(relative, origin);
                append('A', {rx, ry, rotation, double(large), double(sweep),
                             end.x(), end.y()});
                current_ = end;
                previousKind_ = QChar();
                break;
            }
            case 'Z':
                if (active_) {
                    active_->closed = true;
                    active_ = nullptr;
                }
                current_ = subpathStart_;
                previousKind_ = QChar();
                break;
            default:
                fail(QStringLiteral("不支持的 SVG 路径命令 %1").arg(command_));
            }
        }

        paths_.erase(std::remove_if(paths_.begin(), paths_.end(),
                                    [](const RawSubpath& path) {
                                        return path.segments.empty();
                                    }),
                     paths_.end());
        return paths_;
    }

private:
    bool atEnd() const { return index_ >= data_.size(); }
    QChar peek() const { return atEnd() ? QChar() : data_[index_]; }
    QChar take() { return data_[index_++]; }

    static bool isCommand(QChar value)
    {
        return QStringLiteral("MmLlHhVvCcSsQqTtAaZz").contains(value);
    }

    void skipSeparators()
    {
        while (!atEnd() && (peek().isSpace() || peek() == QLatin1Char(','))) {
            ++index_;
        }
    }

    [[noreturn]] void fail(const QString& message) const
    {
        throw std::runtime_error(
            QStringLiteral("morphicons: %1，位置 d[%2]")
                .arg(message)
                .arg(index_)
                .toStdString());
    }

    double number()
    {
        skipSeparators();
        const int start = index_;
        if (!atEnd() && (peek() == QLatin1Char('+') || peek() == QLatin1Char('-'))) {
            ++index_;
        }
        bool hasDigit = false;
        while (!atEnd() && peek().isDigit()) {
            ++index_;
            hasDigit = true;
        }
        if (!atEnd() && peek() == QLatin1Char('.')) {
            ++index_;
            while (!atEnd() && peek().isDigit()) {
                ++index_;
                hasDigit = true;
            }
        }
        if (!hasDigit) {
            fail(QStringLiteral("应为数字"));
        }
        if (!atEnd() && (peek() == QLatin1Char('e') || peek() == QLatin1Char('E'))) {
            const int exponentStart = index_++;
            if (!atEnd() && (peek() == QLatin1Char('+') || peek() == QLatin1Char('-'))) {
                ++index_;
            }
            bool exponentDigit = false;
            while (!atEnd() && peek().isDigit()) {
                ++index_;
                exponentDigit = true;
            }
            if (!exponentDigit) {
                index_ = exponentStart;
            }
        }
        bool ok = false;
        const double value = data_.mid(start, index_ - start).toDouble(&ok);
        if (!ok || !std::isfinite(value)) {
            fail(QStringLiteral("数字无效"));
        }
        return value;
    }

    int flag()
    {
        skipSeparators();
        if (!atEnd() && (peek() == QLatin1Char('0') || peek() == QLatin1Char('1'))) {
            return take() == QLatin1Char('1') ? 1 : 0;
        }
        fail(QStringLiteral("圆弧标志应为 0 或 1"));
    }

    QPointF point(bool relative)
    {
        return pointFromOrigin(relative, current_);
    }

    QPointF pointFromOrigin(bool relative, const QPointF& origin)
    {
        const double x = number();
        const double y = number();
        QPointF result(x, y);
        if (relative) {
            result += origin;
        }
        return result;
    }

    void ensureOpen()
    {
        if (!started_) {
            fail(QStringLiteral("路径必须从 M/m 开始"));
        }
        if (!active_) {
            paths_.push_back({current_, {}, false});
            active_ = &paths_.back();
        }
    }

    void append(QChar kind, std::initializer_list<double> values)
    {
        ensureOpen();
        RawSegment segment;
        segment.command = kind;
        std::copy(values.begin(), values.end(), segment.values.begin());
        active_->segments.push_back(segment);
    }

    QString data_;
    int index_ = 0;
    QChar command_;
    QPointF current_;
    QPointF subpathStart_;
    QPointF previousControl_;
    QChar previousKind_;
    bool started_ = false;
    std::vector<RawSubpath> paths_;
    RawSubpath* active_ = nullptr;
};

class CubicBuilder final {
public:
    explicit CubicBuilder(QPointF start) : current_(start)
    {
        path_.points.push_back(start);
    }

    void cubic(const QPointF& c1, const QPointF& c2, const QPointF& end)
    {
        path_.points.push_back(c1);
        path_.points.push_back(c2);
        path_.points.push_back(end);
        current_ = end;
    }

    void line(const QPointF& end)
    {
        if (norm(end - current_) < kEpsilon) {
            return;
        }
        const QPointF delta = end - current_;
        cubic(current_ + delta / 3.0, current_ + delta * (2.0 / 3.0), end);
    }

    void quadratic(const QPointF& control, const QPointF& end)
    {
        cubic(current_ + (control - current_) * (2.0 / 3.0),
              end + (control - end) * (2.0 / 3.0), end);
    }

    void arc(double rx0,
             double ry0,
             double rotationDegrees,
             int large,
             int sweep,
             const QPointF& end)
    {
        const QPointF start = current_;
        if (norm(end - start) < kEpsilon) {
            return;
        }
        double rx = std::abs(rx0);
        double ry = std::abs(ry0);
        if (rx < kEpsilon || ry < kEpsilon) {
            line(end);
            return;
        }
        const double phi = rotationDegrees * kPi / 180.0;
        const double cosPhi = std::cos(phi);
        const double sinPhi = std::sin(phi);
        const QPointF half = (start - end) / 2.0;
        const double x1p = cosPhi * half.x() + sinPhi * half.y();
        const double y1p = -sinPhi * half.x() + cosPhi * half.y();
        const double lambda = x1p * x1p / (rx * rx) + y1p * y1p / (ry * ry);
        if (lambda > 1.0) {
            const double scale = std::sqrt(lambda);
            rx *= scale;
            ry *= scale;
        }
        const double rx2 = rx * rx;
        const double ry2 = ry * ry;
        const double x2 = x1p * x1p;
        const double y2 = y1p * y1p;
        const double denominator = rx2 * y2 + ry2 * x2;
        double radicand = denominator > kEpsilon
            ? (rx2 * ry2 - rx2 * y2 - ry2 * x2) / denominator
            : 0.0;
        radicand = std::max(0.0, radicand);
        const double coefficient = (large == sweep ? -1.0 : 1.0) * std::sqrt(radicand);
        const double cxp = coefficient * rx * y1p / ry;
        const double cyp = -coefficient * ry * x1p / rx;
        const QPointF center(cosPhi * cxp - sinPhi * cyp + (start.x() + end.x()) / 2.0,
                             sinPhi * cxp + cosPhi * cyp + (start.y() + end.y()) / 2.0);
        const double theta0 = std::atan2((y1p - cyp) / ry, (x1p - cxp) / rx);
        double deltaTheta = std::atan2((-y1p - cyp) / ry, (-x1p - cxp) / rx) - theta0;
        if (!sweep && deltaTheta > 0.0) {
            deltaTheta -= kTau;
        }
        else if (sweep && deltaTheta < 0.0) {
            deltaTheta += kTau;
        }
        const int slices = std::max(1, int(std::ceil(std::abs(deltaTheta) / (kPi / 2.0) - 1e-9)));
        const double delta = deltaTheta / slices;
        const double alpha = (4.0 / 3.0) * std::tan(delta / 4.0);
        const auto position = [&](double angle) {
            return QPointF(center.x() + rx * std::cos(angle) * cosPhi - ry * std::sin(angle) * sinPhi,
                           center.y() + rx * std::cos(angle) * sinPhi + ry * std::sin(angle) * cosPhi);
        };
        const auto tangent = [&](double angle) {
            return QPointF(-rx * std::sin(angle) * cosPhi - ry * std::cos(angle) * sinPhi,
                           -rx * std::sin(angle) * sinPhi + ry * std::cos(angle) * cosPhi);
        };
        double angle0 = theta0;
        QPointF point0 = start;
        for (int slice = 1; slice <= slices; ++slice) {
            const double angle1 = theta0 + delta * slice;
            const QPointF point1 = slice == slices ? end : position(angle1);
            cubic(point0 + tangent(angle0) * alpha,
                  point1 - tangent(angle1) * alpha,
                  point1);
            angle0 = angle1;
            point0 = point1;
        }
    }

    CubicPath finish(bool closed)
    {
        if (closed && !path_.points.empty()) {
            line(path_.points.front());
        }
        path_.closed = closed;
        return path_;
    }

private:
    QPointF current_;
    CubicPath path_;
};

CubicPath lowerPath(const RawSubpath& raw)
{
    CubicBuilder builder(raw.start);
    for (const RawSegment& segment : raw.segments) {
        const auto& value = segment.values;
        switch (segment.command.unicode()) {
        case 'L': builder.line({value[0], value[1]}); break;
        case 'C': builder.cubic({value[0], value[1]}, {value[2], value[3]},
                                {value[4], value[5]}); break;
        case 'Q': builder.quadratic({value[0], value[1]}, {value[2], value[3]}); break;
        case 'A': builder.arc(value[0], value[1], value[2], int(value[3]),
                              int(value[4]), {value[5], value[6]}); break;
        }
    }
    return builder.finish(raw.closed);
}

double attributeNumber(const QXmlStreamAttributes& attributes,
                       const QString& name,
                       double fallback = 0.0)
{
    if (!attributes.hasAttribute(name)) {
        return fallback;
    }
    bool ok = false;
    const double value = attributes.value(name).toDouble(&ok);
    return ok && std::isfinite(value) ? value : fallback;
}

std::vector<double> parsePointList(QString text)
{
    text.replace(QLatin1Char(','), QLatin1Char(' '));
    std::vector<double> values;
    for (const QString& token : text.split(QChar::Space, Qt::SkipEmptyParts)) {
        bool ok = false;
        const double value = token.toDouble(&ok);
        if (!ok) {
            throw std::runtime_error("morphicons: invalid SVG point list");
        }
        values.push_back(value);
    }
    return values;
}

CubicPath polyPath(const std::vector<double>& values, bool closed)
{
    if (values.size() < 4) {
        return {};
    }
    CubicBuilder builder({values[0], values[1]});
    for (size_t index = 2; index + 1 < values.size(); index += 2) {
        builder.line({values[index], values[index + 1]});
    }
    return builder.finish(closed);
}

CubicPath ellipsePath(double cx, double cy, double rx, double ry)
{
    if (rx < kEpsilon || ry < kEpsilon) {
        return {};
    }
    const double kappa = (4.0 / 3.0) * std::tan(kPi / 8.0);
    const double kx = kappa * rx;
    const double ky = kappa * ry;
    CubicBuilder builder({cx + rx, cy});
    builder.cubic({cx + rx, cy + ky}, {cx + kx, cy + ry}, {cx, cy + ry});
    builder.cubic({cx - kx, cy + ry}, {cx - rx, cy + ky}, {cx - rx, cy});
    builder.cubic({cx - rx, cy - ky}, {cx - kx, cy - ry}, {cx, cy - ry});
    builder.cubic({cx + kx, cy - ry}, {cx + rx, cy - ky}, {cx + rx, cy});
    return builder.finish(true);
}

CubicPath rectPath(const QXmlStreamAttributes& attributes)
{
    const double x = attributeNumber(attributes, QStringLiteral("x"));
    const double y = attributeNumber(attributes, QStringLiteral("y"));
    const double width = attributeNumber(attributes, QStringLiteral("width"));
    const double height = attributeNumber(attributes, QStringLiteral("height"));
    if (width < kEpsilon || height < kEpsilon) {
        return {};
    }
    const bool hasRx = attributes.hasAttribute(QStringLiteral("rx"));
    const bool hasRy = attributes.hasAttribute(QStringLiteral("ry"));
    double rx = attributeNumber(attributes, QStringLiteral("rx"),
                                std::numeric_limits<double>::quiet_NaN());
    double ry = attributeNumber(attributes, QStringLiteral("ry"),
                                std::numeric_limits<double>::quiet_NaN());
    if (!hasRx) rx = hasRy ? ry : 0.0;
    if (!hasRy) ry = rx;
    rx = std::clamp(rx, 0.0, width / 2.0);
    ry = std::clamp(ry, 0.0, height / 2.0);
    if (rx < kEpsilon || ry < kEpsilon) {
        return polyPath({x, y, x + width, y, x + width, y + height,
                         x, y + height}, true);
    }
    const double kappa = (4.0 / 3.0) * std::tan(kPi / 8.0);
    const double kx = kappa * rx;
    const double ky = kappa * ry;
    const double xa = x + rx;
    const double xb = x + width - rx;
    const double right = x + width;
    const double ya = y + ry;
    const double yb = y + height - ry;
    const double bottom = y + height;
    CubicBuilder builder({xa, y});
    builder.line({xb, y});
    builder.cubic({xb + kx, y}, {right, ya - ky}, {right, ya});
    builder.line({right, yb});
    builder.cubic({right, yb + ky}, {xb + kx, bottom}, {xb, bottom});
    builder.line({xa, bottom});
    builder.cubic({xa - kx, bottom}, {x, yb + ky}, {x, yb});
    builder.line({x, ya});
    builder.cubic({x, ya - ky}, {xa - kx, y}, {xa, y});
    return builder.finish(true);
}

int segmentCount(const CubicPath& path)
{
    return path.points.size() >= 4 ? int((path.points.size() - 1) / 3) : 0;
}

QPointF cubicPoint(const CubicPath& path, int segment, double t)
{
    const int offset = segment * 3;
    const QPointF& p0 = path.points[offset];
    const QPointF& p1 = path.points[offset + 1];
    const QPointF& p2 = path.points[offset + 2];
    const QPointF& p3 = path.points[offset + 3];
    const double u = 1.0 - t;
    return p0 * (u * u * u) + p1 * (3.0 * u * u * t) +
           p2 * (3.0 * u * t * t) + p3 * (t * t * t);
}

double cubicSpeed(const CubicPath& path, int segment, double t)
{
    const int offset = segment * 3;
    const double u = 1.0 - t;
    const QPointF derivative =
        (path.points[offset + 1] - path.points[offset]) * (3.0 * u * u) +
        (path.points[offset + 2] - path.points[offset + 1]) * (6.0 * u * t) +
        (path.points[offset + 3] - path.points[offset + 2]) * (3.0 * t * t);
    return norm(derivative);
}

double segmentLength(const CubicPath& path, int segment, double end = 1.0)
{
    static constexpr std::array<double, 4> nodes{
        0.18343464249564978, 0.525532409916329,
        0.7966664774136267, 0.9602898564975363};
    static constexpr std::array<double, 4> weights{
        0.362683783378362, 0.31370664587788727,
        0.22238103445337448, 0.10122853629037626};
    const double half = end / 2.0;
    double value = 0.0;
    for (int index = 0; index < 4; ++index) {
        value += weights[index] *
            (cubicSpeed(path, segment, half + half * nodes[index]) +
             cubicSpeed(path, segment, half - half * nodes[index]));
    }
    return value * half;
}

QPointF endpointTangent(const CubicPath& path, int segment, bool atEnd)
{
    const int offset = segment * 3;
    const QPointF base = path.points[offset + (atEnd ? 3 : 0)];
    const std::array<int, 3> candidates = atEnd
        ? std::array<int, 3>{2, 1, 0}
        : std::array<int, 3>{1, 2, 3};
    for (int candidate : candidates) {
        QPointF tangent = atEnd
            ? base - path.points[offset + candidate]
            : path.points[offset + candidate] - base;
        if (dot(tangent, tangent) > 1e-18) {
            return tangent;
        }
    }
    return {};
}

std::vector<int> detectCorners(const CubicPath& path)
{
    const int count = segmentCount(path);
    std::vector<int> active;
    for (int segment = 0; segment < count; ++segment) {
        if (segmentLength(path, segment) > 1e-9) active.push_back(segment);
    }
    std::vector<int> corners;
    const auto test = [&](int left, int right) {
        const QPointF incoming = endpointTangent(path, left, true);
        const QPointF outgoing = endpointTangent(path, right, false);
        if (dot(incoming, incoming) < 1e-18 || dot(outgoing, outgoing) < 1e-18) return;
        const double angle = std::abs(std::atan2(cross(incoming, outgoing),
                                                dot(incoming, outgoing)));
        if (angle > kCornerThreshold) corners.push_back(right);
    };
    for (size_t index = 0; index + 1 < active.size(); ++index) {
        test(active[index], active[index + 1]);
    }
    if (path.closed && active.size() > 1) {
        test(active.back(), active.front());
    }
    std::sort(corners.begin(), corners.end());
    corners.erase(std::unique(corners.begin(), corners.end()), corners.end());
    return corners;
}

double invertLength(const CubicPath& path, int segment, double target, double length)
{
    if (target <= 0.0) return 0.0;
    if (target >= length) return 1.0;
    double low = 0.0;
    double high = 1.0;
    double t = target / length;
    for (int iteration = 0; iteration < 12; ++iteration) {
        const double difference = segmentLength(path, segment, t) - target;
        if (std::abs(difference) < 1e-10 * length + 1e-14) break;
        if (difference > 0.0) high = t; else low = t;
        const double speed = cubicSpeed(path, segment, t);
        double next = speed > kEpsilon ? t - difference / speed : (low + high) / 2.0;
        if (!(next > low && next < high)) next = (low + high) / 2.0;
        t = next;
    }
    return t;
}

std::vector<QPointF> resamplePath(const CubicPath& path, int sampleCount)
{
    std::vector<QPointF> output(sampleCount,
        path.points.empty() ? QPointF() : path.points.front());
    const int count = segmentCount(path);
    if (count < 1 || sampleCount < 2) return output;
    std::vector<double> lengths(count);
    double totalLength = 0.0;
    for (int segment = 0; segment < count; ++segment) {
        lengths[segment] = segmentLength(path, segment);
        totalLength += lengths[segment];
    }
    if (totalLength < kEpsilon) return output;

    std::vector<int> anchors = detectCorners(path);
    if (path.closed) {
        if (anchors.empty()) anchors.push_back(0);
    }
    else {
        anchors.insert(anchors.begin(), 0);
        anchors.push_back(count);
        std::sort(anchors.begin(), anchors.end());
        anchors.erase(std::unique(anchors.begin(), anchors.end()), anchors.end());
    }
    std::vector<std::pair<int, int>> runs;
    if (path.closed) {
        for (size_t index = 0; index < anchors.size(); ++index) {
            runs.emplace_back(anchors[index], index + 1 < anchors.size()
                ? anchors[index + 1] : anchors.front() + count);
        }
    }
    else {
        for (size_t index = 0; index + 1 < anchors.size(); ++index) {
            runs.emplace_back(anchors[index], anchors[index + 1]);
        }
    }
    const int intervalCount = path.closed ? sampleCount : sampleCount - 1;
    if (int(runs.size()) > intervalCount) {
        throw std::runtime_error("morphicons: sample count is too small");
    }
    std::vector<double> runLengths;
    for (const auto& [start, end] : runs) {
        double value = 0.0;
        for (int segment = start; segment < end; ++segment) value += lengths[segment % count];
        runLengths.push_back(value);
    }
    const double sum = std::accumulate(runLengths.begin(), runLengths.end(), 0.0);
    std::vector<double> ideal(runLengths.size());
    std::vector<int> allocations(runLengths.size());
    for (size_t index = 0; index < runLengths.size(); ++index) {
        ideal[index] = intervalCount * runLengths[index] / (sum > 0.0 ? sum : 1.0);
        allocations[index] = std::max(1, int(std::floor(ideal[index])));
    }
    int remainder = intervalCount -
        std::accumulate(allocations.begin(), allocations.end(), 0);
    std::vector<int> order(ideal.size());
    std::iota(order.begin(), order.end(), 0);
    std::sort(order.begin(), order.end(), [&](int left, int right) {
        const auto fraction = [&](int value) {
            return std::llround((ideal[value] - std::floor(ideal[value])) * 1e9);
        };
        return fraction(left) != fraction(right)
            ? fraction(left) > fraction(right) : left < right;
    });
    for (int index = 0; remainder > 0; ++index, --remainder) {
        ++allocations[order[index % order.size()]];
    }
    while (remainder < 0) {
        const auto largest = std::max_element(allocations.begin(), allocations.end());
        if (*largest <= 1) break;
        --*largest;
        ++remainder;
    }

    int write = 0;
    for (size_t run = 0; run < runs.size(); ++run) {
        const auto [first, last] = runs[run];
        const int allocation = allocations[run];
        const double runLength = runLengths[run];
        output[write++] = path.points[3 * (first % count)];
        int segment = first;
        double accumulated = 0.0;
        for (int index = 1; index < allocation; ++index) {
            const double target = runLength * index / allocation;
            while (segment < last - 1 &&
                   accumulated + lengths[segment % count] < target) {
                accumulated += lengths[segment % count];
                ++segment;
            }
            const int actual = segment % count;
            const double localLength = lengths[actual];
            const double t = localLength > kEpsilon
                ? invertLength(path, actual, target - accumulated, localLength)
                : 0.0;
            output[write++] = cubicPoint(path, actual, t);
        }
    }
    if (!path.closed && write < sampleCount) {
        output[write] = path.points[3 * count];
    }
    return output;
}

struct Alignment {
    Similarity similarity;
    QPointF sourceCentroid;
    QPointF targetCentroid;
    std::vector<QPointF> source;
    std::vector<QPointF> target;
};

Alignment alignPair(const SampledPath& source, const SampledPath& target)
{
    const QPointF sourceCenter = centroid(source.points);
    const QPointF targetCenter = centroid(target.points);
    const bool varySource = source.closed && !target.closed;
    const std::vector<QPointF>& base = varySource ? source.points : target.points;
    const int offsets = source.closed || target.closed ? int(base.size()) : 1;
    double bestScore = std::numeric_limits<double>::infinity();
    std::vector<QPointF> best = base;
    Similarity bestSimilarity;
    for (int direction = 0; direction < 2; ++direction) {
        const std::vector<QPointF> walk = direction ? reversePoints(base) : base;
        for (int offset = 0; offset < offsets; ++offset) {
            const std::vector<QPointF> candidate = offset ? rotatePoints(walk, offset) : walk;
            const Similarity similarity = varySource
                ? MorphIconCore::procrustes(candidate, target.points,
                                            sourceCenter, targetCenter)
                : MorphIconCore::procrustes(source.points, candidate,
                                            sourceCenter, targetCenter);
            const double score = similarity.residual +
                kRotationTieBreak * std::abs(similarity.theta) / kPi;
            if (score < bestScore) {
                bestScore = score;
                best = candidate;
                bestSimilarity = similarity;
            }
        }
    }
    return varySource
        ? Alignment{bestSimilarity, sourceCenter, targetCenter, best, target.points}
        : Alignment{bestSimilarity, sourceCenter, targetCenter, source.points, best};
}

std::vector<std::vector<double>> costMatrix(const std::vector<SampledPath>& source,
                                            const std::vector<SampledPath>& target)
{
    std::vector<std::vector<double>> costs(source.size(),
        std::vector<double>(target.size()));
    for (size_t left = 0; left < source.size(); ++left) {
        const QPointF leftCenter = centroid(source[left].points);
        const double leftLength = polylineLength(source[left].points);
        for (size_t right = 0; right < target.size(); ++right) {
            costs[left][right] = norm(leftCenter - centroid(target[right].points)) +
                kLengthWeight * std::abs(leftLength - polylineLength(target[right].points));
        }
    }
    return costs;
}

std::vector<int> bestPermutation(const std::vector<std::vector<double>>& costs)
{
    const int count = int(costs.size());
    std::vector<int> current(count), best(count);
    std::iota(current.begin(), current.end(), 0);
    best = current;
    if (count > 8) {
        std::vector<bool> used(count, false);
        for (int row = 0; row < count; ++row) {
            int selected = -1;
            for (int column = 0; column < count; ++column) {
                if (!used[column] && (selected < 0 || costs[row][column] < costs[row][selected])) {
                    selected = column;
                }
            }
            best[row] = selected;
            used[selected] = true;
        }
        return best;
    }
    double bestCost = std::numeric_limits<double>::infinity();
    do {
        double value = 0.0;
        for (int row = 0; row < count; ++row) value += costs[row][current[row]];
        if (value < bestCost) {
            bestCost = value;
            best = current;
        }
    } while (std::next_permutation(current.begin(), current.end()));
    return best;
}

std::vector<int> bestSurjection(const std::vector<std::vector<double>>& costs)
{
    const int big = int(costs.size());
    const int small = int(costs.front().size());
    std::vector<int> best(big), current(big), multiplicity(small);
    double bestCost = std::numeric_limits<double>::infinity();
    const bool exhaustive = std::pow(double(small), double(big)) <= 1e5;
    if (exhaustive) {
        std::function<void(int, double, int)> visit = [&](int row, double cost, int covered) {
            if (cost >= bestCost || small - covered > big - row) return;
            if (row == big) {
                bestCost = cost;
                best = current;
                return;
            }
            for (int column = 0; column < small; ++column) {
                current[row] = column;
                ++multiplicity[column];
                visit(row + 1, cost + costs[row][column],
                      covered + (multiplicity[column] == 1 ? 1 : 0));
                --multiplicity[column];
            }
        };
        visit(0, 0.0, 0);
        return best;
    }
    for (int row = 0; row < big; ++row) {
        best[row] = int(std::min_element(costs[row].begin(), costs[row].end()) -
                        costs[row].begin());
        ++multiplicity[best[row]];
    }
    for (int missing = 0; missing < small; ++missing) {
        if (multiplicity[missing] > 0) continue;
        int donor = -1;
        double extra = std::numeric_limits<double>::infinity();
        for (int row = 0; row < big; ++row) {
            if (multiplicity[best[row]] > 1 &&
                costs[row][missing] - costs[row][best[row]] < extra) {
                donor = row;
                extra = costs[row][missing] - costs[row][best[row]];
            }
        }
        --multiplicity[best[donor]];
        best[donor] = missing;
        ++multiplicity[missing];
    }
    return best;
}

void applyGlobalAlignment(MorphPlan& plan)
{
    std::vector<QPointF> source;
    std::vector<QPointF> target;
    for (const PlanItem& item : plan.items) {
        source.insert(source.end(), item.source.begin(), item.source.end());
        target.insert(target.end(), item.targetOriented.begin(), item.targetOriented.end());
    }
    const QPointF globalSourceCenter = centroid(source);
    const Similarity global = MorphIconCore::procrustes(
        source, target, globalSourceCenter, centroid(target));
    if (global.residual >= kGlobalResidualThreshold) return;
    const double inverseCos = std::cos(-global.theta);
    const double inverseSin = std::sin(-global.theta);
    const double forwardCos = std::cos(global.theta);
    const double forwardSin = std::sin(global.theta);
    for (PlanItem& item : plan.items) {
        double error2 = 0.0;
        double targetEnergy = 0.0;
        for (int index = 0; index < plan.sampleCount; ++index) {
            const QPointF targetPoint = item.targetOriented[index] - item.targetCentroid;
            item.targetAligned[index] = QPointF(
                (targetPoint.x() * inverseCos - targetPoint.y() * inverseSin) / global.sigma,
                (targetPoint.x() * inverseSin + targetPoint.y() * inverseCos) / global.sigma);
            const QPointF sourcePoint = item.sourceCentered[index];
            const QPointF transported(
                global.sigma * (forwardCos * sourcePoint.x() - forwardSin * sourcePoint.y()),
                global.sigma * (forwardSin * sourcePoint.x() + forwardCos * sourcePoint.y()));
            const QPointF error = transported - targetPoint;
            error2 += dot(error, error);
            targetEnergy += dot(targetPoint, targetPoint);
        }
        item.theta = global.theta;
        item.logSigma = std::log(global.sigma);
        item.residual = targetEnergy > kEpsilon
            ? std::sqrt(error2 / targetEnergy) : 0.0;
        item.hasBlockTransport = true;
        item.blockOffset = item.sourceCentroid - globalSourceCenter;
        const double scale = std::exp(item.logSigma);
        const double c = std::cos(item.theta) * scale;
        const double s = std::sin(item.theta) * scale;
        const QPointF rotated(item.blockOffset.x() * c - item.blockOffset.y() * s,
                              item.blockOffset.x() * s + item.blockOffset.y() * c);
        item.blockDrift = item.targetCentroid - item.sourceCentroid -
                          (rotated - item.blockOffset);
    }
}

void setError(QString* output, const QString& value)
{
    if (output) *output = value;
}

} // namespace

bool MorphIconCore::loadSvg(const QString& resource,
                            std::vector<CubicPath>& paths,
                            QRectF& viewBox,
                            QString* errorMessage)
{
    paths.clear();
    QFile file(resource);
    if (!file.open(QIODevice::ReadOnly)) {
        setError(errorMessage, QStringLiteral("无法读取 SVG：%1").arg(resource));
        return false;
    }
    try {
        QXmlStreamReader xml(&file);
        while (!xml.atEnd()) {
            xml.readNext();
            if (!xml.isStartElement()) continue;
            const QString tag = xml.name().toString().toLower();
            const QXmlStreamAttributes attributes = xml.attributes();
            if (tag == QStringLiteral("svg")) {
                const QString value = attributes.value(QStringLiteral("viewBox")).toString();
                const std::vector<double> numbers = parsePointList(value);
                if (numbers.size() == 4 && numbers[2] > 0.0 && numbers[3] > 0.0) {
                    viewBox = QRectF(numbers[0], numbers[1], numbers[2], numbers[3]);
                }
            }
            else if (tag == QStringLiteral("path")) {
                for (const RawSubpath& raw : PathParser(
                         attributes.value(QStringLiteral("d")).toString()).parse()) {
                    CubicPath path = lowerPath(raw);
                    if (segmentCount(path) > 0) paths.push_back(std::move(path));
                }
            }
            else if (tag == QStringLiteral("line")) {
                CubicBuilder builder({attributeNumber(attributes, QStringLiteral("x1")),
                                      attributeNumber(attributes, QStringLiteral("y1"))});
                builder.line({attributeNumber(attributes, QStringLiteral("x2")),
                              attributeNumber(attributes, QStringLiteral("y2"))});
                paths.push_back(builder.finish(false));
            }
            else if (tag == QStringLiteral("circle")) {
                const double radius = attributeNumber(attributes, QStringLiteral("r"));
                CubicPath path = ellipsePath(attributeNumber(attributes, QStringLiteral("cx")),
                                             attributeNumber(attributes, QStringLiteral("cy")),
                                             radius, radius);
                if (segmentCount(path) > 0) paths.push_back(std::move(path));
            }
            else if (tag == QStringLiteral("ellipse")) {
                CubicPath path = ellipsePath(attributeNumber(attributes, QStringLiteral("cx")),
                                             attributeNumber(attributes, QStringLiteral("cy")),
                                             attributeNumber(attributes, QStringLiteral("rx")),
                                             attributeNumber(attributes, QStringLiteral("ry")));
                if (segmentCount(path) > 0) paths.push_back(std::move(path));
            }
            else if (tag == QStringLiteral("rect")) {
                CubicPath path = rectPath(attributes);
                if (segmentCount(path) > 0) paths.push_back(std::move(path));
            }
            else if (tag == QStringLiteral("polyline") || tag == QStringLiteral("polygon")) {
                CubicPath path = polyPath(parsePointList(
                    attributes.value(QStringLiteral("points")).toString()),
                    tag == QStringLiteral("polygon"));
                if (segmentCount(path) > 0) paths.push_back(std::move(path));
            }
        }
        if (xml.hasError()) {
            throw std::runtime_error(xml.errorString().toStdString());
        }
        if (paths.empty()) {
            throw std::runtime_error("SVG does not contain supported geometry");
        }
    }
    catch (const std::exception& exception) {
        setError(errorMessage,
                 QStringLiteral("SVG 解析失败：%1").arg(QString::fromUtf8(exception.what())));
        paths.clear();
        return false;
    }
    return true;
}

std::vector<SampledPath> MorphIconCore::resample(
    const std::vector<CubicPath>& paths, int sampleCount)
{
    std::vector<SampledPath> result;
    result.reserve(paths.size());
    for (const CubicPath& path : paths) {
        result.push_back({resamplePath(path, sampleCount), path.closed});
    }
    return result;
}

Similarity MorphIconCore::procrustes(const std::vector<QPointF>& source,
                                     const std::vector<QPointF>& target,
                                     const QPointF& sourceCentroid,
                                     const QPointF& targetCentroid)
{
    double sxx = 0.0;
    double sxy = 0.0;
    double syx = 0.0;
    double syy = 0.0;
    double sourceEnergy = 0.0;
    double targetEnergy = 0.0;
    for (size_t index = 0; index < source.size(); ++index) {
        const QPointF a = source[index] - sourceCentroid;
        const QPointF b = target[index] - targetCentroid;
        sxx += a.x() * b.x();
        syy += a.y() * b.y();
        sxy += a.x() * b.y();
        syx += a.y() * b.x();
        sourceEnergy += dot(a, a);
        targetEnergy += dot(b, b);
    }
    const double theta = std::atan2(sxy - syx, sxx + syy);
    const double numerator = std::cos(theta) * (sxx + syy) +
                             std::sin(theta) * (sxy - syx);
    double sigma = sourceEnergy > kEpsilon ? numerator / sourceEnergy : 1.0;
    if (!(sigma > 1e-6)) sigma = 1e-6;
    const double residual2 = std::max(
        0.0, sigma * sigma * sourceEnergy - 2.0 * sigma * numerator + targetEnergy);
    return {theta, sigma, targetEnergy > kEpsilon
        ? std::sqrt(residual2 / targetEnergy) : 0.0};
}

MorphPlan MorphIconCore::buildPlan(const std::vector<SampledPath>& source,
                                   const std::vector<SampledPath>& target)
{
    if (source.empty() || target.empty()) {
        throw std::runtime_error("morphicons: icon has no subpaths");
    }
    std::vector<std::pair<int, int>> pairs;
    if (source.size() == target.size()) {
        const std::vector<int> permutation = bestPermutation(costMatrix(source, target));
        for (int index = 0; index < int(source.size()); ++index) {
            pairs.emplace_back(index, permutation[index]);
        }
    }
    else if (source.size() < target.size()) {
        const std::vector<int> assignment = bestSurjection(costMatrix(target, source));
        for (int index = 0; index < int(target.size()); ++index) {
            pairs.emplace_back(assignment[index], index);
        }
    }
    else {
        const std::vector<int> assignment = bestSurjection(costMatrix(source, target));
        for (int index = 0; index < int(source.size()); ++index) {
            pairs.emplace_back(index, assignment[index]);
        }
    }

    MorphPlan plan;
    plan.sampleCount = int(source.front().points.size());
    for (const auto& [sourceIndex, targetIndex] : pairs) {
        const Alignment alignment = alignPair(source[sourceIndex], target[targetIndex]);
        PlanItem item;
        item.source = alignment.source;
        item.sourceCentroid = alignment.sourceCentroid;
        item.targetCentroid = alignment.targetCentroid;
        item.theta = alignment.similarity.theta;
        item.logSigma = std::log(alignment.similarity.sigma);
        item.residual = alignment.similarity.residual;
        item.closed = source[sourceIndex].closed && target[targetIndex].closed;
        item.sourceCentered.resize(plan.sampleCount);
        item.targetAligned.resize(plan.sampleCount);
        item.targetOriented = alignment.target;
        const double c = std::cos(-item.theta);
        const double s = std::sin(-item.theta);
        for (int index = 0; index < plan.sampleCount; ++index) {
            item.sourceCentered[index] = item.source[index] - item.sourceCentroid;
            const QPointF targetPoint = item.targetOriented[index] - item.targetCentroid;
            item.targetAligned[index] = QPointF(
                (targetPoint.x() * c - targetPoint.y() * s) / alignment.similarity.sigma,
                (targetPoint.x() * s + targetPoint.y() * c) / alignment.similarity.sigma);
        }
        plan.items.push_back(std::move(item));
    }
    if (plan.items.size() > 1) applyGlobalAlignment(plan);
    return plan;
}

bool MorphIconCore::configure(const QString& sourceSvgResource,
                              const QString& targetSvgResource,
                              int sampleCount,
                              QString* errorMessage)
{
    const std::string cacheKey = QStringLiteral("%1\n%2\n%3")
        .arg(sourceSvgResource, targetSvgResource)
        .arg(sampleCount)
        .toStdString();
    {
        std::lock_guard lock(morphPlanCacheMutex());
        const auto cached = morphPlanCache().find(cacheKey);
        if (cached != morphPlanCache().end()) {
            plan_ = cached->second.plan;
            sharedViewBox_ = cached->second.viewBox;
            return true;
        }
    }
    std::vector<CubicPath> sourcePaths;
    std::vector<CubicPath> targetPaths;
    QRectF sourceViewBox(0.0, 0.0, 24.0, 24.0);
    QRectF targetViewBox(0.0, 0.0, 24.0, 24.0);
    if (!loadSvg(sourceSvgResource, sourcePaths, sourceViewBox, errorMessage) ||
        !loadSvg(targetSvgResource, targetPaths, targetViewBox, errorMessage)) {
        plan_ = {};
        return false;
    }
    // Match SVG's xMidYMid meet behavior if packs use different grids.
    const auto fit = [](std::vector<CubicPath>& paths, const QRectF& from,
                        const QRectF& to) {
        const double scale = std::min(to.width() / from.width(),
                                      to.height() / from.height());
        const QPointF translation(
            to.left() + (to.width() - from.width() * scale) / 2.0 - from.left() * scale,
            to.top() + (to.height() - from.height() * scale) / 2.0 - from.top() * scale);
        for (CubicPath& path : paths) {
            for (QPointF& point : path.points) point = point * scale + translation;
        }
    };
    sharedViewBox_ = QRectF(0.0, 0.0, 24.0, 24.0);
    fit(sourcePaths, sourceViewBox, sharedViewBox_);
    fit(targetPaths, targetViewBox, sharedViewBox_);
    try {
        plan_ = buildPlan(resample(sourcePaths, sampleCount),
                          resample(targetPaths, sampleCount));
    }
    catch (const std::exception& exception) {
        plan_ = {};
        setError(errorMessage, QString::fromUtf8(exception.what()));
        return false;
    }
    {
        std::lock_guard lock(morphPlanCacheMutex());
        morphPlanCache().emplace(
            cacheKey, CachedMorphPlan{plan_, sharedViewBox_});
    }
    return true;
}

void MorphIconCore::interpolate(
    double progress, std::vector<std::vector<QPointF>>& output) const
{
    output.resize(plan_.items.size());
    for (size_t itemIndex = 0; itemIndex < plan_.items.size(); ++itemIndex) {
        const PlanItem& item = plan_.items[itemIndex];
        std::vector<QPointF>& points = output[itemIndex];
        points.resize(plan_.sampleCount);
        const double scale = std::exp(item.logSigma * progress);
        const double angle = item.theta * progress;
        const double c = std::cos(angle) * scale;
        const double s = std::sin(angle) * scale;
        QPointF center;
        if (item.hasBlockTransport) {
            const QPointF& offset = item.blockOffset;
            center = item.sourceCentroid + item.blockDrift * progress +
                QPointF(offset.x() * c - offset.y() * s - offset.x(),
                        offset.x() * s + offset.y() * c - offset.y());
        }
        else {
            center = item.sourceCentroid +
                     (item.targetCentroid - item.sourceCentroid) * progress;
        }
        for (int index = 0; index < plan_.sampleCount; ++index) {
            const QPointF local = item.sourceCentered[index] +
                (item.targetAligned[index] - item.sourceCentered[index]) * progress;
            points[index] = center + QPointF(local.x() * c - local.y() * s,
                                              local.x() * s + local.y() * c);
        }
    }
}

QPainterPath MorphIconCore::painterPath(double progress) const
{
    std::vector<std::vector<QPointF>> output;
    interpolate(progress, output);
    QPainterPath result;
    for (size_t itemIndex = 0; itemIndex < output.size(); ++itemIndex) {
        const auto& points = output[itemIndex];
        if (points.empty()) continue;
        result.moveTo(points.front());
        for (size_t index = 1; index < points.size(); ++index) result.lineTo(points[index]);
        if (plan_.items[itemIndex].closed) result.closeSubpath();
    }
    return result;
}

void MorphIconCore::paint(QPainter& painter,
                          const QRectF& targetRect,
                          const QColor& color,
                          double progress,
                          qreal strokeWidth) const
{
    if (!isValid()) return;
    painter.save();
    painter.setRenderHint(QPainter::Antialiasing, true);
    const double scale = std::min(targetRect.width() / sharedViewBox_.width(),
                                  targetRect.height() / sharedViewBox_.height());
    const QPointF offset(targetRect.left() + (targetRect.width() - sharedViewBox_.width() * scale) / 2.0,
                         targetRect.top() + (targetRect.height() - sharedViewBox_.height() * scale) / 2.0);
    painter.translate(offset);
    painter.scale(scale, scale);
    QPen pen(color, strokeWidth, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
    pen.setCosmetic(false);
    painter.setPen(pen);
    painter.setBrush(Qt::NoBrush);
    painter.drawPath(painterPath(progress));
    painter.restore();
}

void Spring::configure(double stiffness, double damping) noexcept
{
    stiffness_ = stiffness;
    damping_ = damping;
}

void Spring::start() noexcept
{
    value_ = 0.0;
    velocity_ = std::clamp(velocity_, -14.0, 14.0);
}

bool Spring::step(double seconds) noexcept
{
    const double fixedStep = 1.0 / 240.0;
    const int steps = std::clamp(int(std::ceil(seconds / fixedStep)), 1, 16);
    const double step = seconds / steps;
    for (int index = 0; index < steps; ++index) {
        const double acceleration = stiffness_ * (1.0 - value_) - damping_ * velocity_;
        velocity_ += acceleration * step;
        value_ += velocity_ * step;
    }
    return std::abs(1.0 - value_) < 0.001 && std::abs(velocity_) < 0.02;
}

} // namespace remotec::ui::morph
