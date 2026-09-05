// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#include "WindowsCursorMonitor.h"

#include <Windows.h>

#include <algorithm>
#include <array>
#include <chrono>
#include <limits>
#include <utility>
#include <vector>

namespace remote {
namespace {

constexpr auto kSamplePeriod = std::chrono::microseconds(8'333);
constexpr auto kShapeRefreshPeriod = std::chrono::milliseconds(250);
constexpr auto kPositionHeartbeatPeriod = std::chrono::milliseconds(500);

class OwnedBitmap final {
public:
    explicit OwnedBitmap(HBITMAP value = nullptr) : value_(value) {}
    ~OwnedBitmap() { if (value_) DeleteObject(value_); }
    HBITMAP get() const { return value_; }
private:
    HBITMAP value_ = nullptr;
};

class OwnedDc final {
public:
    explicit OwnedDc(HDC value = nullptr) : value_(value) {}
    ~OwnedDc() { if (value_) DeleteDC(value_); }
    HDC get() const { return value_; }
private:
    HDC value_ = nullptr;
};

std::uint64_t HashShape(const RemoteCursorShape& shape)
{
    std::uint64_t hash = 1469598103934665603ULL;
    const auto append = [&hash](std::uint8_t value) {
        hash ^= value;
        hash *= 1099511628211ULL;
    };
    for (const auto value : {shape.width, shape.height,
                             shape.hotspotX, shape.hotspotY}) {
        append(static_cast<std::uint8_t>(value >> 8));
        append(static_cast<std::uint8_t>(value));
    }
    for (const auto value : shape.premultipliedBgra) append(value);
    for (const auto value : shape.invertMask) append(value);
    return hash == 0 ? 1 : hash;
}

bool RenderCursor(HCURSOR cursor,
                  int width,
                  int height,
                  std::uint8_t background,
                  std::vector<std::uint8_t>* pixels)
{
    if (!cursor || width <= 0 || height <= 0 || !pixels) return false;
    BITMAPINFO info{};
    info.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    info.bmiHeader.biWidth = width;
    info.bmiHeader.biHeight = -height;
    info.bmiHeader.biPlanes = 1;
    info.bmiHeader.biBitCount = 32;
    info.bmiHeader.biCompression = BI_RGB;
    void* bits = nullptr;
    OwnedDc dc(CreateCompatibleDC(nullptr));
    OwnedBitmap bitmap(CreateDIBSection(
        dc.get(), &info, DIB_RGB_COLORS, &bits, nullptr, 0));
    if (!dc.get() || !bitmap.get() || !bits) return false;
    const HGDIOBJ previous = SelectObject(dc.get(), bitmap.get());
    const auto byteCount = static_cast<std::size_t>(width) * height * 4;
    std::fill_n(static_cast<std::uint8_t*>(bits), byteCount, background);
    const BOOL drawn = DrawIconEx(dc.get(), 0, 0, cursor, width, height,
                                  0, nullptr, DI_NORMAL);
    GdiFlush();
    pixels->assign(static_cast<const std::uint8_t*>(bits),
                   static_cast<const std::uint8_t*>(bits) + byteCount);
    SelectObject(dc.get(), previous);
    return drawn != FALSE;
}

std::optional<RemoteCursorShape> CaptureShape(HCURSOR cursor)
{
    if (!cursor) return std::nullopt;
    ICONINFO icon{};
    if (!GetIconInfo(cursor, &icon)) return std::nullopt;
    OwnedBitmap color(icon.hbmColor);
    OwnedBitmap mask(icon.hbmMask);
    BITMAP bitmap{};
    int width = 0;
    int height = 0;
    if (color.get() && GetObject(color.get(), sizeof(bitmap), &bitmap)) {
        width = bitmap.bmWidth;
        height = bitmap.bmHeight;
    } else if (mask.get() &&
               GetObject(mask.get(), sizeof(bitmap), &bitmap)) {
        width = bitmap.bmWidth;
        height = bitmap.bmHeight / 2;
    }
    if (width <= 0 || height <= 0 ||
        width > static_cast<int>(kMaximumRemoteCursorDimension) ||
        height > static_cast<int>(kMaximumRemoteCursorDimension)) {
        return std::nullopt;
    }
    std::vector<std::uint8_t> black;
    std::vector<std::uint8_t> white;
    if (!RenderCursor(cursor, width, height, 0, &black) ||
        !RenderCursor(cursor, width, height, 255, &white) ||
        black.size() != white.size()) {
        return std::nullopt;
    }

    RemoteCursorShape shape;
    shape.width = static_cast<std::uint16_t>(width);
    shape.height = static_cast<std::uint16_t>(height);
    shape.hotspotX = static_cast<std::uint16_t>((std::min)(
        static_cast<DWORD>(width - 1), icon.xHotspot));
    shape.hotspotY = static_cast<std::uint16_t>((std::min)(
        static_cast<DWORD>(height - 1), icon.yHotspot));
    shape.premultipliedBgra.resize(black.size());
    const auto pixelCount = static_cast<std::size_t>(width) * height;
    shape.invertMask.assign(pixelCount, 0);
    bool hasInvert = false;
    for (std::size_t pixel = 0; pixel < pixelCount; ++pixel) {
        const std::size_t offset = pixel * 4;
        const bool invert = white[offset] < black[offset] ||
                            white[offset + 1] < black[offset + 1] ||
                            white[offset + 2] < black[offset + 2];
        if (invert) {
            shape.invertMask[pixel] = 1;
            hasInvert = true;
            continue;
        }
        const int backgroundContribution = (std::max)({
            static_cast<int>(white[offset]) - black[offset],
            static_cast<int>(white[offset + 1]) - black[offset + 1],
            static_cast<int>(white[offset + 2]) - black[offset + 2], 0});
        const std::uint8_t alpha = static_cast<std::uint8_t>(
            (std::clamp)(255 - backgroundContribution, 0, 255));
        shape.premultipliedBgra[offset] = black[offset];
        shape.premultipliedBgra[offset + 1] = black[offset + 1];
        shape.premultipliedBgra[offset + 2] = black[offset + 2];
        shape.premultipliedBgra[offset + 3] = alpha;
    }
    if (!hasInvert) shape.invertMask.clear();
    shape.shapeId = HashShape(shape);
    return shape;
}

std::uint16_t NormalizeCoordinate(std::int64_t value,
                                  std::int64_t origin,
                                  std::uint32_t extent)
{
    if (extent <= 1) return 0;
    const auto relative = (std::clamp)(
        value - origin, std::int64_t{0},
        static_cast<std::int64_t>(extent - 1));
    return static_cast<std::uint16_t>(
        (relative * 65535 + (extent - 1) / 2) / (extent - 1));
}

}  // namespace

WindowsCursorMonitor::~WindowsCursorMonitor()
{
    Stop();
}

bool WindowsCursorMonitor::Start(const DisplayDescriptor& display,
                                 std::uint64_t layoutVersion,
                                 Callback callback)
{
    Stop();
    if (display.sessionDisplayId == 0 || display.width == 0 ||
        display.height == 0 || layoutVersion == 0 || !callback) {
        return false;
    }
    {
        std::lock_guard lock(mutex_);
        target_ = {display, layoutVersion};
        callback_ = std::move(callback);
    }
    lastAppliedInputSequence_.store(0, std::memory_order_release);
    thread_ = std::jthread(
        [this](std::stop_token token) { Run(token); });
    running_.store(true, std::memory_order_release);
    return true;
}

void WindowsCursorMonitor::UpdateTarget(
    const DisplayDescriptor& display, std::uint64_t layoutVersion)
{
    if (display.sessionDisplayId == 0 || display.width == 0 ||
        display.height == 0 || layoutVersion == 0) return;
    std::lock_guard lock(mutex_);
    target_ = {display, layoutVersion};
}

void WindowsCursorMonitor::SetLastAppliedInputSequence(
    std::uint64_t sequence)
{
    lastAppliedInputSequence_.store(sequence, std::memory_order_release);
}

void WindowsCursorMonitor::Stop()
{
    if (thread_.joinable()) {
        thread_.request_stop();
        thread_.join();
    }
    running_.store(false, std::memory_order_release);
    std::lock_guard lock(mutex_);
    callback_ = {};
    target_ = {};
}

bool WindowsCursorMonitor::running() const
{
    return running_.load(std::memory_order_acquire);
}

void WindowsCursorMonitor::Run(std::stop_token stopToken)
{
    SetThreadDescription(GetCurrentThread(), L"RemoteC Cursor Monitor");
    HANDLE timer = CreateWaitableTimerExW(
        nullptr, nullptr, 0x00000002 /* CREATE_WAITABLE_TIMER_HIGH_RESOLUTION */,
        TIMER_ALL_ACCESS);
    if (!timer) timer = CreateWaitableTimerW(nullptr, FALSE, nullptr);
    HCURSOR lastCursor = nullptr;
    std::uint64_t lastShapeId = 0;
    bool lastVisible = false;
    POINT lastPoint{(std::numeric_limits<LONG>::min)(),
                    (std::numeric_limits<LONG>::min)()};
    std::uint32_t lastDisplayId = 0;
    std::uint64_t lastLayoutVersion = 0;
    auto nextShapeRefresh = std::chrono::steady_clock::time_point{};
    auto nextPositionHeartbeat = std::chrono::steady_clock::time_point{};

    while (!stopToken.stop_requested()) {
        Target target;
        Callback callback;
        {
            std::lock_guard lock(mutex_);
            target = target_;
            callback = callback_;
        }
        CURSORINFO info{};
        info.cbSize = sizeof(info);
        const bool queried = GetCursorInfo(&info) != FALSE;
        const bool visible = queried && (info.flags & CURSOR_SHOWING) != 0 &&
            info.ptScreenPos.x >= target.display.left &&
            info.ptScreenPos.y >= target.display.top &&
            info.ptScreenPos.x < target.display.left +
                static_cast<LONG>(target.display.width) &&
            info.ptScreenPos.y < target.display.top +
                static_cast<LONG>(target.display.height);
        const auto now = std::chrono::steady_clock::now();
        std::optional<RemoteCursorShape> shape;
        if (queried && info.hCursor &&
            (info.hCursor != lastCursor || now >= nextShapeRefresh)) {
            shape = CaptureShape(info.hCursor);
            lastCursor = info.hCursor;
            nextShapeRefresh = now + kShapeRefreshPeriod;
            if (shape && shape->shapeId == lastShapeId) shape.reset();
            if (shape) lastShapeId = shape->shapeId;
        }
        const bool changed = shape.has_value() ||
            now >= nextPositionHeartbeat || visible != lastVisible ||
            (queried && (info.ptScreenPos.x != lastPoint.x ||
                         info.ptScreenPos.y != lastPoint.y)) ||
            target.display.sessionDisplayId != lastDisplayId ||
            target.layoutVersion != lastLayoutVersion;
        if (changed && callback) {
            WindowsCursorObservation observation;
            observation.position.displayId =
                target.display.sessionDisplayId;
            observation.position.displayLayoutVersion =
                target.layoutVersion;
            observation.position.normalizedX = NormalizeCoordinate(
                queried ? info.ptScreenPos.x : target.display.left,
                target.display.left, target.display.width);
            observation.position.normalizedY = NormalizeCoordinate(
                queried ? info.ptScreenPos.y : target.display.top,
                target.display.top, target.display.height);
            observation.position.visible = visible;
            observation.position.shapeId = lastShapeId;
            observation.position.lastAppliedInputSequence =
                lastAppliedInputSequence_.load(std::memory_order_acquire);
            observation.changedShape = std::move(shape);
            callback(std::move(observation));
            nextPositionHeartbeat = now + kPositionHeartbeatPeriod;
        }
        if (queried) lastPoint = info.ptScreenPos;
        lastVisible = visible;
        lastDisplayId = target.display.sessionDisplayId;
        lastLayoutVersion = target.layoutVersion;

        if (timer) {
            LARGE_INTEGER due{};
            due.QuadPart = -static_cast<LONGLONG>(
                kSamplePeriod.count() * 10);
            if (SetWaitableTimer(timer, &due, 0, nullptr, nullptr, FALSE)) {
                (void)WaitForSingleObject(timer, 9);
            } else {
                Sleep(8);
            }
        } else {
            Sleep(8);
        }
    }
    if (timer) CloseHandle(timer);
}

}  // namespace remote
