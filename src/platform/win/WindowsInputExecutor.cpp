// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#include "WindowsInputExecutor.h"

#include <Windows.h>

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <limits>
#include <optional>
#include <vector>

#include "src/core/RemoteInputTelemetry.h"

namespace remote {
namespace {

LONG ScaleNormalized(std::uint16_t value, LONG extent)
{
    if (extent <= 1) {
        return 0;
    }
    return static_cast<LONG>(std::llround(
        static_cast<double>(value) * static_cast<double>(extent - 1) /
        static_cast<double>(std::numeric_limits<std::uint16_t>::max())));
}

LONG ToSendInputAbsolute(LONG pixel, LONG origin, LONG extent)
{
    if (extent <= 1) {
        return 0;
    }
    const LONG clamped = std::clamp(pixel, origin, origin + extent - 1);
    return static_cast<LONG>(std::llround(
        static_cast<double>(clamped - origin) * 65535.0 /
        static_cast<double>(extent - 1)));
}

bool FillAbsoluteMousePosition(const RemoteInputEvent& event,
                               const DisplayDescriptor& display,
                               std::uint64_t layoutVersion,
                               MOUSEINPUT* mouse)
{
    if (!mouse || display.sessionDisplayId == 0 ||
        layoutVersion == 0 ||
        event.displayId != display.sessionDisplayId ||
        event.displayLayoutVersion != layoutVersion) {
        return false;
    }

    const LONG virtualLeft = GetSystemMetrics(SM_XVIRTUALSCREEN);
    const LONG virtualTop = GetSystemMetrics(SM_YVIRTUALSCREEN);
    const LONG virtualWidth = GetSystemMetrics(SM_CXVIRTUALSCREEN);
    const LONG virtualHeight = GetSystemMetrics(SM_CYVIRTUALSCREEN);
    if (virtualWidth <= 0 || virtualHeight <= 0) {
        return false;
    }

    const LONG pixelX = display.left +
        ScaleNormalized(
            event.normalizedX, static_cast<LONG>(display.width));
    const LONG pixelY = display.top +
        ScaleNormalized(
            event.normalizedY, static_cast<LONG>(display.height));
    mouse->dx = ToSendInputAbsolute(pixelX, virtualLeft, virtualWidth);
    mouse->dy = ToSendInputAbsolute(pixelY, virtualTop, virtualHeight);
    mouse->dwFlags = MOUSEEVENTF_MOVE | MOUSEEVENTF_ABSOLUTE |
                     MOUSEEVENTF_VIRTUALDESK;
    return true;
}

std::optional<DWORD> MouseButtonFlag(RemoteMouseButton button, bool pressed)
{
    switch (button) {
    case RemoteMouseButton::kLeft:
        return pressed ? MOUSEEVENTF_LEFTDOWN : MOUSEEVENTF_LEFTUP;
    case RemoteMouseButton::kRight:
        return pressed ? MOUSEEVENTF_RIGHTDOWN : MOUSEEVENTF_RIGHTUP;
    case RemoteMouseButton::kMiddle:
        return pressed ? MOUSEEVENTF_MIDDLEDOWN : MOUSEEVENTF_MIDDLEUP;
    case RemoteMouseButton::kX1:
    case RemoteMouseButton::kX2:
        return pressed ? MOUSEEVENTF_XDOWN : MOUSEEVENTF_XUP;
    default:
        return std::nullopt;
    }
}

DWORD XButtonData(RemoteMouseButton button)
{
    return button == RemoteMouseButton::kX1 ? XBUTTON1 : XBUTTON2;
}

bool SendOneInput(INPUT input)
{
    return SendInput(1, &input, sizeof(input)) == 1;
}

INPUT MakeKeyInput(const RemoteInputEvent& event, bool pressed)
{
    INPUT input{};
    input.type = INPUT_KEYBOARD;
    if (event.scanCode != 0) {
        input.ki.wScan = event.scanCode;
        input.ki.dwFlags = KEYEVENTF_SCANCODE;
    } else {
        input.ki.wVk = event.virtualKey;
    }
    if (event.extendedKey) {
        input.ki.dwFlags |= KEYEVENTF_EXTENDEDKEY;
    }
    if (!pressed) {
        input.ki.dwFlags |= KEYEVENTF_KEYUP;
    }
    return input;
}

std::uint64_t KeyIdentity(const RemoteInputEvent& event)
{
    return static_cast<std::uint64_t>(event.virtualKey) |
           (static_cast<std::uint64_t>(event.scanCode) << 16) |
           (static_cast<std::uint64_t>(event.extendedKey ? 1 : 0) << 32);
}

}  // namespace

WindowsInputExecutor::~WindowsInputExecutor()
{
    ReleaseAllRemoteInputs();
}

void WindowsInputExecutor::OnRemoteInput(const RemoteInputEvent& event)
{
    std::lock_guard lock(mutex_);
    // InProcessSessionEngine has already validated and de-duplicated pointer
    // sequence numbers in the scope where they are meaningful: the current
    // room pair. Do not compare them again globally here. A controller that
    // joins an already-sharing room owns a new sequence space and may start
    // below the value left by a previous controller; the old global check
    // silently discarded every otherwise-authorized mouse event.
    switch (event.type) {
    case RemoteInputMessageType::kMouseMove: {
        const bool succeeded = ApplyMouseMove(event);
        RemoteInputTelemetry::Instance().RecordInjection(
            RemoteInputTelemetry::InjectionType::kMouseMove,
            succeeded);
        break;
    }
    case RemoteInputMessageType::kMouseButton: {
        const bool succeeded = ApplyMouseButton(event);
        RemoteInputTelemetry::Instance().RecordInjection(
            RemoteInputTelemetry::InjectionType::kMouseButton,
            succeeded);
        break;
    }
    case RemoteInputMessageType::kMouseWheel: {
        const bool succeeded = ApplyMouseWheel(event);
        RemoteInputTelemetry::Instance().RecordInjection(
            RemoteInputTelemetry::InjectionType::kMouseWheel,
            succeeded);
        break;
    }
    case RemoteInputMessageType::kKey: {
        const bool succeeded = ApplyKey(event);
        RemoteInputTelemetry::Instance().RecordInjection(
            RemoteInputTelemetry::InjectionType::kKey,
            succeeded);
        break;
    }
    case RemoteInputMessageType::kReleaseAll:
        for (const auto button : pressedMouseButtons_) {
            const auto flag = MouseButtonFlag(button, false);
            if (!flag) {
                continue;
            }
            INPUT input{};
            input.type = INPUT_MOUSE;
            input.mi.dwFlags = *flag;
            if (button == RemoteMouseButton::kX1 ||
                button == RemoteMouseButton::kX2) {
                input.mi.mouseData = XButtonData(button);
            }
            (void)SendOneInput(input);
        }
        for (const auto& [identity, key] : pressedKeys_) {
            (void)identity;
            (void)SendOneInput(MakeKeyInput(key, false));
        }
        pressedMouseButtons_.clear();
        pressedKeys_.clear();
        break;
    }
}

void WindowsInputExecutor::ReleaseAllRemoteInputs()
{
    std::lock_guard lock(mutex_);
    ReleaseAllRemoteInputsLocked();
}

void WindowsInputExecutor::SetActiveDisplay(
    const DisplayTopologySnapshot& topology,
    const std::string& stableDisplayKey)
{
    std::lock_guard lock(mutex_);
    const auto* selected =
        FindDisplayByStableKey(topology, stableDisplayKey);
    if (!selected) {
        selected = FindPrimaryDisplay(topology);
    }
    const bool changed =
        !selected ||
        activeLayoutVersion_ != topology.layoutVersion ||
        activeDisplay_.sessionDisplayId != selected->sessionDisplayId;
    if (changed) {
        ReleaseAllRemoteInputsLocked();
    }
    activeDisplay_ = selected ? *selected : DisplayDescriptor{};
    activeLayoutVersion_ =
        selected ? topology.layoutVersion : 0;
}

void WindowsInputExecutor::ClearActiveDisplay()
{
    std::lock_guard lock(mutex_);
    ReleaseAllRemoteInputsLocked();
    activeDisplay_ = {};
    activeLayoutVersion_ = 0;
}

void WindowsInputExecutor::ReleaseAllRemoteInputsLocked()
{
    for (const auto button : pressedMouseButtons_) {
        const auto flag = MouseButtonFlag(button, false);
        if (!flag) {
            continue;
        }
        INPUT input{};
        input.type = INPUT_MOUSE;
        input.mi.dwFlags = *flag;
        if (button == RemoteMouseButton::kX1 ||
            button == RemoteMouseButton::kX2) {
            input.mi.mouseData = XButtonData(button);
        }
        (void)SendOneInput(input);
    }
    for (const auto& [identity, key] : pressedKeys_) {
        (void)identity;
        (void)SendOneInput(MakeKeyInput(key, false));
    }
    pressedMouseButtons_.clear();
    pressedKeys_.clear();
}

bool WindowsInputExecutor::ApplyMouseMove(const RemoteInputEvent& event)
{
    return ApplyPointerPositionAndState(event);
}

bool WindowsInputExecutor::ApplyMouseButton(const RemoteInputEvent& event)
{
    return ApplyPointerPositionAndState(event);
}

bool WindowsInputExecutor::ApplyMouseWheel(const RemoteInputEvent& event)
{
    if (!ApplyPointerPositionAndState(event)) {
        return false;
    }

    std::array<INPUT, 2> inputs{};
    UINT count = 0;
    if (event.wheelDeltaY != 0) {
        auto& input = inputs[count++];
        input.type = INPUT_MOUSE;
        input.mi.dwFlags = MOUSEEVENTF_WHEEL;
        input.mi.mouseData = static_cast<DWORD>(
            static_cast<LONG>(event.wheelDeltaY));
    }
    if (event.wheelDeltaX != 0) {
        auto& input = inputs[count++];
        input.type = INPUT_MOUSE;
        input.mi.dwFlags = MOUSEEVENTF_HWHEEL;
        input.mi.mouseData = static_cast<DWORD>(
            static_cast<LONG>(event.wheelDeltaX));
    }
    if (count != 0) {
        return SendInput(count, inputs.data(), sizeof(INPUT)) == count;
    }
    return true;
}

bool WindowsInputExecutor::ApplyPointerPositionAndState(
    const RemoteInputEvent& event)
{
    INPUT move{};
    move.type = INPUT_MOUSE;
    if (!FillAbsoluteMousePosition(
            event, activeDisplay_, activeLayoutVersion_, &move.mi) ||
        !SendOneInput(move)) {
        return false;
    }

    constexpr std::array<RemoteMouseButton, 5> kButtons = {
        RemoteMouseButton::kLeft,
        RemoteMouseButton::kRight,
        RemoteMouseButton::kMiddle,
        RemoteMouseButton::kX1,
        RemoteMouseButton::kX2};
    bool succeeded = true;
    for (std::size_t index = 0; index < kButtons.size(); ++index) {
        const RemoteMouseButton button = kButtons[index];
        const bool explicitTransition =
            event.type == RemoteInputMessageType::kMouseButton &&
            event.mouseButton == button;
        // The complete pointer-state mask repairs lost transitions during
        // ordinary movement. An explicit button packet is stronger: always
        // inject its named DOWN/UP transition even when our local bookkeeping
        // already matches. SendInput success only means that Windows accepted
        // an event into its input stream; a foreground window can change mouse
        // capture while minimizing and fail to observe that first button-up.
        // The reliable duplicate must therefore produce another real UP
        // instead of being optimized away by pressedMouseButtons_.
        const bool desiredPressed = explicitTransition
            ? event.pressed
            : (event.pressedMouseButtons & (1u << index)) != 0;
        const bool currentPressed =
            pressedMouseButtons_.contains(button);
        if (!explicitTransition && desiredPressed == currentPressed) {
            continue;
        }

        const auto flag = MouseButtonFlag(button, desiredPressed);
        if (!flag) {
            continue;
        }
        INPUT transition{};
        transition.type = INPUT_MOUSE;
        transition.mi.dwFlags = *flag;
        if (button == RemoteMouseButton::kX1 ||
            button == RemoteMouseButton::kX2) {
            transition.mi.mouseData = XButtonData(button);
        }
        if (!SendOneInput(transition)) {
            succeeded = false;
            continue;
        }
        if (desiredPressed) {
            pressedMouseButtons_.insert(button);
        } else {
            pressedMouseButtons_.erase(button);
        }
    }
    return succeeded;
}

bool WindowsInputExecutor::ApplyKey(const RemoteInputEvent& event)
{
    if (event.virtualKey == 0 && event.scanCode == 0) {
        return false;
    }
    if (!SendOneInput(MakeKeyInput(event, event.pressed))) {
        return false;
    }

    const std::uint64_t identity = KeyIdentity(event);
    if (event.pressed) {
        pressedKeys_[identity] = event;
    } else {
        pressedKeys_.erase(identity);
    }
    return true;
}

}  // namespace remote
