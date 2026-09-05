// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#pragma once

#include <atomic>
#include <cstdint>
#include <filesystem>
#include <functional>
#include <memory>
#include <string>
#include <thread>
#include <vector>

#include "src/protocol/ClipboardProtocol.h"

namespace remote {

struct WindowsClipboardPart {
    ClipboardFormat format = ClipboardFormat::kUnicodeText;
    std::string name;
    std::vector<std::uint8_t> data;
    std::filesystem::path localPath;
};

struct WindowsClipboardContent {
    std::vector<WindowsClipboardPart> parts;
    std::string originMarker;
    std::uint32_t windowsSequence = 0;
    // True only for an explicit RequestCurrent() capture. Ordinary
    // WM_CLIPBOARDUPDATE notifications stay false, allowing the controller
    // to distinguish a requested source read from a newer user copy.
    bool explicitRequest = false;
    // Assigned by ClipboardController when this capture enters its worker.
    // A newer ordinary user copy increments the controller generation, so a
    // long file hash can stop before the new clipboard task reaches the queue.
    std::uint64_t localChangeGeneration = 0;
    bool empty = false;
    std::string captureError;
};

struct WindowsPasteTarget {
    std::uintptr_t foregroundWindow = 0;
    std::uintptr_t focusedWindow = 0;
    // Keyboard Ctrl+V is swallowed while the remote item is materialized and
    // therefore needs one replacement injection. A delayed-render request
    // made by an application's Paste command resumes by itself.
    bool injectPasteAfterMaterialization = true;
    // Resolved by ClipboardController for Explorer targets. Empty for other
    // applications and for virtual Shell folders without a filesystem path.
    std::filesystem::path explorerDestinationDirectory;
    // Set only after the user explicitly confirms an Explorer name
    // collision. Direct-copy mode then merges directories and replaces
    // conflicting files instead of silently inventing a new name.
    bool replaceExplorerConflicts = false;
};

// Owns a message-only HWND and is the only component that touches the Win32
// clipboard. Callbacks execute on its message thread and must return quickly.
class WindowsClipboardService final {
public:
    using ChangeCallback = std::function<void(WindowsClipboardContent)>;
    // Called from the clipboard service's Windows message thread. Returning
    // true consumes this physical Ctrl+V; the callback must return quickly.
    using PasteCallback = std::function<bool(WindowsPasteTarget)>;

    WindowsClipboardService();
    ~WindowsClipboardService();

    WindowsClipboardService(const WindowsClipboardService&) = delete;
    WindowsClipboardService& operator=(const WindowsClipboardService&) = delete;

    bool Start(ChangeCallback callback,
               PasteCallback pasteCallback = {},
               std::string* error = nullptr);
    void Stop();
    void SetEnabled(bool enabled);
    void RequestCurrent();
    std::uint32_t AdvertiseRemoteClipboard(
        std::uint32_t capabilities, const std::string& originMarker);
    void CancelPendingRemoteRender();
    // Removes only the delayed RemoteC advertisement that is still the
    // current Windows clipboard item. A newer local user copy is preserved.
    void ClearRemoteClipboardAdvertisement();
    bool Write(WindowsClipboardContent content, std::string* error = nullptr);

private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};

}  // namespace remote
