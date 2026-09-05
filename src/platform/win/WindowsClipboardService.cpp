// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#include "WindowsClipboardService.h"

#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <objidl.h>
#include <ole2.h>
#include <shlobj.h>
#include <shobjidl.h>
#include <shellapi.h>
#include <wincodec.h>

#include <algorithm>
#include <array>
#include <chrono>
#include <condition_variable>
#include <cstring>
#include <deque>
#include <future>
#include <mutex>
#include <optional>
#include <utility>

namespace remote {
namespace {

constexpr UINT kWriteMessage = WM_APP + 0x341;
constexpr UINT kCaptureMessage = WM_APP + 0x342;
constexpr UINT kAdvertiseMessage = WM_APP + 0x343;
constexpr UINT kClearAdvertisementMessage = WM_APP + 0x344;
constexpr wchar_t kWindowClassName[] = L"RemoteCClipboardListenerV2";
constexpr wchar_t kOriginFormatName[] = L"RemoteC.ClipboardOrigin.v2";
constexpr wchar_t kPngFormatName[] = L"PNG";
constexpr wchar_t kHtmlFormatName[] = L"HTML Format";
constexpr wchar_t kRtfFormatName[] = L"Rich Text Format";

std::string WideToUtf8(std::wstring_view text)
{
    if (text.empty()) {
        return {};
    }
    const int bytes = WideCharToMultiByte(
        CP_UTF8, WC_ERR_INVALID_CHARS, text.data(),
        static_cast<int>(text.size()), nullptr, 0, nullptr, nullptr);
    if (bytes <= 0) {
        return {};
    }
    std::string result(static_cast<std::size_t>(bytes), '\0');
    if (WideCharToMultiByte(
            CP_UTF8, WC_ERR_INVALID_CHARS, text.data(),
            static_cast<int>(text.size()), result.data(), bytes,
            nullptr, nullptr) != bytes) {
        return {};
    }
    return result;
}

std::wstring Utf8ToWide(std::string_view text)
{
    if (text.empty()) {
        return {};
    }
    const int chars = MultiByteToWideChar(
        CP_UTF8, MB_ERR_INVALID_CHARS, text.data(),
        static_cast<int>(text.size()), nullptr, 0);
    if (chars <= 0) {
        return {};
    }
    std::wstring result(static_cast<std::size_t>(chars), L'\0');
    if (MultiByteToWideChar(
            CP_UTF8, MB_ERR_INVALID_CHARS, text.data(),
            static_cast<int>(text.size()), result.data(), chars) != chars) {
        return {};
    }
    return result;
}

bool CopyGlobalBytes(UINT format, std::vector<std::uint8_t>* output)
{
    if (!output) {
        return false;
    }
    HANDLE handle = GetClipboardData(format);
    if (!handle) {
        return false;
    }
    const SIZE_T size = GlobalSize(handle);
    const void* bytes = GlobalLock(handle);
    if (!bytes || size == 0) {
        if (bytes) {
            GlobalUnlock(handle);
        }
        return false;
    }
    const auto* first = static_cast<const std::uint8_t*>(bytes);
    output->assign(first, first + size);
    GlobalUnlock(handle);
    while (!output->empty() && output->back() == 0) {
        output->pop_back();
    }
    return true;
}

bool PutGlobalBytes(UINT format, const void* bytes, std::size_t size,
                    bool appendNull)
{
    const std::size_t allocationSize = size + (appendNull ? 1u : 0u);
    HGLOBAL memory = GlobalAlloc(GMEM_MOVEABLE, allocationSize);
    if (!memory) {
        return false;
    }
    void* target = GlobalLock(memory);
    if (!target) {
        GlobalFree(memory);
        return false;
    }
    if (size != 0) {
        std::memcpy(target, bytes, size);
    }
    if (appendNull) {
        static_cast<std::uint8_t*>(target)[size] = 0;
    }
    GlobalUnlock(memory);
    if (!SetClipboardData(format, memory)) {
        GlobalFree(memory);
        return false;
    }
    return true;
}

bool PutFileDrop(const std::vector<std::wstring>& files)
{
    if (files.empty()) return false;
    std::size_t characterCount = 1;
    for (const auto& file : files) {
        characterCount += file.size() + 1;
    }
    const std::size_t bytes = sizeof(DROPFILES) +
        characterCount * sizeof(wchar_t);
    HGLOBAL memory = GlobalAlloc(GMEM_MOVEABLE | GMEM_ZEROINIT, bytes);
    if (!memory) return false;
    auto* drop = static_cast<DROPFILES*>(GlobalLock(memory));
    if (!drop) {
        GlobalFree(memory);
        return false;
    }
    drop->pFiles = sizeof(DROPFILES);
    drop->fWide = TRUE;
    auto* target = reinterpret_cast<wchar_t*>(
        reinterpret_cast<std::uint8_t*>(drop) + sizeof(DROPFILES));
    for (const auto& file : files) {
        std::memcpy(target, file.c_str(),
                    file.size() * sizeof(wchar_t));
        target += file.size() + 1;
    }
    GlobalUnlock(memory);
    if (SetClipboardData(CF_HDROP, memory)) return true;
    GlobalFree(memory);
    return false;
}

bool StreamBytes(IStream* stream, std::vector<std::uint8_t>* output)
{
    if (!stream || !output) {
        return false;
    }
    HGLOBAL memory = nullptr;
    if (FAILED(GetHGlobalFromStream(stream, &memory)) || !memory) {
        return false;
    }
    const SIZE_T size = GlobalSize(memory);
    const void* data = GlobalLock(memory);
    if (!data || size == 0) {
        if (data) {
            GlobalUnlock(memory);
        }
        return false;
    }
    const auto* first = static_cast<const std::uint8_t*>(data);
    output->assign(first, first + size);
    GlobalUnlock(memory);
    return true;
}

bool EncodeDibToPng(const void* dibMemory, std::size_t dibSize,
                    std::vector<std::uint8_t>* png)
{
    if (!dibMemory || dibSize < sizeof(BITMAPINFOHEADER) || !png) {
        return false;
    }
    const auto* header = static_cast<const BITMAPINFOHEADER*>(dibMemory);
    if (header->biSize < sizeof(BITMAPINFOHEADER) ||
        header->biSize > dibSize) {
        return false;
    }
    std::uint32_t colors = header->biClrUsed;
    if (colors == 0 && header->biBitCount <= 8) {
        colors = 1u << header->biBitCount;
    }
    std::size_t masks = 0;
    if (header->biCompression == BI_BITFIELDS &&
        header->biSize == sizeof(BITMAPINFOHEADER)) {
        masks = 3 * sizeof(DWORD);
    }
    const std::size_t pixelOffset = header->biSize + masks +
        static_cast<std::size_t>(colors) * sizeof(RGBQUAD);
    if (pixelOffset > dibSize) {
        return false;
    }
    std::vector<std::uint8_t> bmp(sizeof(BITMAPFILEHEADER) + dibSize);
    BITMAPFILEHEADER fileHeader{};
    fileHeader.bfType = 0x4d42;
    fileHeader.bfSize = static_cast<DWORD>(bmp.size());
    fileHeader.bfOffBits = static_cast<DWORD>(
        sizeof(BITMAPFILEHEADER) + pixelOffset);
    std::memcpy(bmp.data(), &fileHeader, sizeof(fileHeader));
    std::memcpy(bmp.data() + sizeof(fileHeader), dibMemory, dibSize);

    IWICImagingFactory* factory = nullptr;
    IWICStream* inputStream = nullptr;
    IWICBitmapDecoder* decoder = nullptr;
    IWICBitmapFrameDecode* source = nullptr;
    IWICFormatConverter* converter = nullptr;
    IStream* outputStream = nullptr;
    IWICBitmapEncoder* encoder = nullptr;
    IWICBitmapFrameEncode* frame = nullptr;
    IPropertyBag2* properties = nullptr;
    bool ok = false;
    do {
        if (FAILED(CoCreateInstance(CLSID_WICImagingFactory, nullptr,
                CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&factory))) ||
            FAILED(factory->CreateStream(&inputStream)) ||
            FAILED(inputStream->InitializeFromMemory(
                bmp.data(), static_cast<DWORD>(bmp.size()))) ||
            FAILED(factory->CreateDecoderFromStream(inputStream, nullptr,
                WICDecodeMetadataCacheOnLoad, &decoder)) ||
            FAILED(decoder->GetFrame(0, &source)) ||
            FAILED(factory->CreateFormatConverter(&converter)) ||
            FAILED(converter->Initialize(source, GUID_WICPixelFormat32bppBGRA,
                WICBitmapDitherTypeNone, nullptr, 0.0,
                WICBitmapPaletteTypeCustom)) ||
            FAILED(CreateStreamOnHGlobal(nullptr, TRUE, &outputStream)) ||
            FAILED(factory->CreateEncoder(GUID_ContainerFormatPng, nullptr,
                &encoder)) ||
            FAILED(encoder->Initialize(outputStream,
                WICBitmapEncoderNoCache)) ||
            FAILED(encoder->CreateNewFrame(&frame, &properties)) ||
            FAILED(frame->Initialize(properties))) {
            break;
        }
        UINT width = 0;
        UINT height = 0;
        if (FAILED(converter->GetSize(&width, &height)) || width == 0 ||
            height == 0 || width > 16384 || height > 16384 ||
            static_cast<std::uint64_t>(width) * height > 100000000ull ||
            FAILED(frame->SetSize(width, height))) {
            break;
        }
        WICPixelFormatGUID pixelFormat = GUID_WICPixelFormat32bppBGRA;
        if (FAILED(frame->SetPixelFormat(&pixelFormat)) ||
            FAILED(frame->WriteSource(converter, nullptr)) ||
            FAILED(frame->Commit()) || FAILED(encoder->Commit())) {
            break;
        }
        ok = StreamBytes(outputStream, png);
    } while (false);
    if (properties) properties->Release();
    if (frame) frame->Release();
    if (encoder) encoder->Release();
    if (outputStream) outputStream->Release();
    if (converter) converter->Release();
    if (source) source->Release();
    if (decoder) decoder->Release();
    if (inputStream) inputStream->Release();
    if (factory) factory->Release();
    return ok;
}

bool DecodePngToDib(const std::vector<std::uint8_t>& png,
                    std::vector<std::uint8_t>* dib)
{
    if (png.empty() || !dib || png.size() > 32u * 1024u * 1024u) {
        return false;
    }
    IWICImagingFactory* factory = nullptr;
    IWICStream* stream = nullptr;
    IWICBitmapDecoder* decoder = nullptr;
    IWICBitmapFrameDecode* source = nullptr;
    IWICFormatConverter* converter = nullptr;
    bool ok = false;
    do {
        if (FAILED(CoCreateInstance(CLSID_WICImagingFactory, nullptr,
                CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&factory))) ||
            FAILED(factory->CreateStream(&stream)) ||
            FAILED(stream->InitializeFromMemory(
                const_cast<BYTE*>(png.data()),
                static_cast<DWORD>(png.size()))) ||
            FAILED(factory->CreateDecoderFromStream(stream, nullptr,
                WICDecodeMetadataCacheOnLoad, &decoder)) ||
            FAILED(decoder->GetFrame(0, &source)) ||
            FAILED(factory->CreateFormatConverter(&converter)) ||
            FAILED(converter->Initialize(source, GUID_WICPixelFormat32bppBGRA,
                WICBitmapDitherTypeNone, nullptr, 0.0,
                WICBitmapPaletteTypeCustom))) {
            break;
        }
        UINT width = 0;
        UINT height = 0;
        if (FAILED(converter->GetSize(&width, &height)) || width == 0 ||
            height == 0 || width > 16384 || height > 16384 ||
            static_cast<std::uint64_t>(width) * height > 100000000ull) {
            break;
        }
        const std::uint64_t pixelBytes =
            static_cast<std::uint64_t>(width) * height * 4;
        if (pixelBytes > std::numeric_limits<std::size_t>::max() -
                sizeof(BITMAPV5HEADER)) {
            break;
        }
        dib->assign(sizeof(BITMAPV5HEADER) +
                        static_cast<std::size_t>(pixelBytes), 0);
        auto* header = reinterpret_cast<BITMAPV5HEADER*>(dib->data());
        header->bV5Size = sizeof(BITMAPV5HEADER);
        header->bV5Width = static_cast<LONG>(width);
        header->bV5Height = -static_cast<LONG>(height);
        header->bV5Planes = 1;
        header->bV5BitCount = 32;
        header->bV5Compression = BI_BITFIELDS;
        header->bV5SizeImage = static_cast<DWORD>(pixelBytes);
        header->bV5RedMask = 0x00ff0000;
        header->bV5GreenMask = 0x0000ff00;
        header->bV5BlueMask = 0x000000ff;
        header->bV5AlphaMask = 0xff000000;
        header->bV5CSType = LCS_sRGB;
        ok = SUCCEEDED(converter->CopyPixels(
            nullptr, width * 4, static_cast<UINT>(pixelBytes),
            dib->data() + sizeof(BITMAPV5HEADER)));
    } while (false);
    if (converter) converter->Release();
    if (source) source->Release();
    if (decoder) decoder->Release();
    if (stream) stream->Release();
    if (factory) factory->Release();
    if (!ok) {
        dib->clear();
    }
    return ok;
}

bool OpenClipboardWithRetry(HWND owner)
{
    constexpr std::array<DWORD, 5> delays = {20, 40, 80, 160, 320};
    for (DWORD delay : delays) {
        if (OpenClipboard(owner)) {
            return true;
        }
        Sleep(delay);
    }
    return OpenClipboard(owner) != FALSE;
}

void AppendShellFileSystemItems(WindowsClipboardContent* content)
{
    if (!content) return;
    IDataObject* dataObject = nullptr;
    IShellItemArray* items = nullptr;
    if (FAILED(OleGetClipboard(&dataObject)) || !dataObject) return;
    if (FAILED(SHCreateShellItemArrayFromDataObject(
            dataObject, IID_PPV_ARGS(&items))) || !items) {
        dataObject->Release();
        return;
    }
    DWORD count = 0;
    if (SUCCEEDED(items->GetCount(&count))) {
        for (DWORD index = 0; index < count && index < 256; ++index) {
            IShellItem* item = nullptr;
            PWSTR fileSystemPath = nullptr;
            if (FAILED(items->GetItemAt(index, &item)) || !item) continue;
            if (SUCCEEDED(item->GetDisplayName(
                    SIGDN_FILESYSPATH, &fileSystemPath)) && fileSystemPath) {
                std::filesystem::path path(fileSystemPath);
                CoTaskMemFree(fileSystemPath);
                const bool duplicate = std::any_of(
                    content->parts.begin(), content->parts.end(),
                    [&path](const WindowsClipboardPart& part) {
                        if (part.localPath.empty()) return false;
                        std::error_code equivalentError;
                        return std::filesystem::equivalent(
                            part.localPath, path, equivalentError) &&
                            !equivalentError;
                    });
                if (!duplicate) {
                    std::error_code typeError;
                    WindowsClipboardPart part;
                    if (std::filesystem::is_directory(path, typeError)) {
                        part.format = ClipboardFormat::kDirectory;
                    } else if (!typeError &&
                               std::filesystem::is_regular_file(
                                   path, typeError)) {
                        part.format = ClipboardFormat::kFile;
                    } else {
                        item->Release();
                        continue;
                    }
                    part.localPath = path;
                    part.name = WideToUtf8(path.filename().wstring());
                    content->parts.push_back(std::move(part));
                }
            }
            item->Release();
        }
    }
    items->Release();
    dataObject->Release();
}

}  // namespace

struct WindowsClipboardService::Impl {
    explicit Impl(WindowsClipboardService*) {}

    static inline Impl* hookOwner = nullptr;

    static LRESULT CALLBACK LowLevelKeyboardProc(
        int code, WPARAM wparam, LPARAM lparam)
    {
        Impl* self = hookOwner;
        if (code != HC_ACTION || !self) {
            return CallNextHookEx(nullptr, code, wparam, lparam);
        }
        const auto* key = reinterpret_cast<KBDLLHOOKSTRUCT*>(lparam);
        if (!key || (key->flags & LLKHF_INJECTED) != 0) {
            return CallNextHookEx(nullptr, code, wparam, lparam);
        }
        const bool keyDown = wparam == WM_KEYDOWN || wparam == WM_SYSKEYDOWN;
        const bool keyUp = wparam == WM_KEYUP || wparam == WM_SYSKEYUP;
        if (key->vkCode == 'V' && keyUp && self->suppressPasteV) {
            self->suppressPasteV = false;
            return 1;
        }
        if (key->vkCode != 'V' || !keyDown ||
            (GetAsyncKeyState(VK_CONTROL) & 0x8000) == 0 ||
            (GetAsyncKeyState(VK_SHIFT) & 0x8000) != 0) {
            return CallNextHookEx(nullptr, code, wparam, lparam);
        }
        PasteCallback target;
        {
            std::lock_guard lock(self->mutex);
            target = self->pasteCallback;
        }
        if (!target) {
            return CallNextHookEx(nullptr, code, wparam, lparam);
        }
        WindowsPasteTarget pasteTarget;
        const HWND foreground = GetForegroundWindow();
        pasteTarget.foregroundWindow = reinterpret_cast<std::uintptr_t>(
            foreground);
        if (foreground) {
            GUITHREADINFO info{};
            info.cbSize = sizeof(info);
            const DWORD threadId = GetWindowThreadProcessId(
                foreground, nullptr);
            if (threadId != 0 && GetGUIThreadInfo(threadId, &info)) {
                pasteTarget.focusedWindow =
                    reinterpret_cast<std::uintptr_t>(info.hwndFocus);
            }
        }
        if (target(pasteTarget)) {
            self->suppressPasteV = true;
            return 1;
        }
        return CallNextHookEx(nullptr, code, wparam, lparam);
    }

    static LRESULT CALLBACK LowLevelMouseProc(
        int code, WPARAM wparam, LPARAM lparam)
    {
        Impl* self = hookOwner;
        if (code != HC_ACTION || !self || !lparam) {
            return CallNextHookEx(nullptr, code, wparam, lparam);
        }
        const auto* mouse = reinterpret_cast<MSLLHOOKSTRUCT*>(lparam);
        if ((mouse->flags & LLMHF_INJECTED) != 0) {
            return CallNextHookEx(nullptr, code, wparam, lparam);
        }
        HWND foreground = GetForegroundWindow();
        if (foreground) {
            if (HWND root = GetAncestor(foreground, GA_ROOT)) {
                foreground = root;
            }
        }
        const auto now = GetTickCount64();
        if (wparam == WM_RBUTTONUP) {
            self->contextMenuRoot.store(
                reinterpret_cast<std::uintptr_t>(foreground),
                std::memory_order_release);
            self->contextMenuOpenedAt.store(now,
                std::memory_order_release);
        } else if (wparam == WM_LBUTTONUP) {
            const auto openedAt = self->contextMenuOpenedAt.load(
                std::memory_order_acquire);
            const auto menuRoot = reinterpret_cast<HWND>(
                self->contextMenuRoot.load(std::memory_order_acquire));
            // Selecting an item from a mouse-opened context menu produces a
            // left-button release after the preceding right-button release.
            // Arm delayed clipboard rendering only for that short boundary;
            // ordinary Explorer focus/preview reads must never start a
            // remote transfer.
            if (openedAt != 0 && now >= openedAt &&
                now - openedAt <= 15000 && foreground == menuRoot) {
                self->contextPasteRoot.store(
                    reinterpret_cast<std::uintptr_t>(foreground),
                    std::memory_order_release);
                self->contextPasteIntentUntil.store(now + 750,
                    std::memory_order_release);
            }
            self->contextMenuOpenedAt.store(0,
                std::memory_order_release);
            self->contextMenuRoot.store(0,
                std::memory_order_release);
        }
        return CallNextHookEx(nullptr, code, wparam, lparam);
    }

    bool ConsumeContextPasteIntent(HWND foreground)
    {
        if (foreground) {
            if (HWND root = GetAncestor(foreground, GA_ROOT)) {
                foreground = root;
            }
        }
        const auto now = GetTickCount64();
        const auto until = contextPasteIntentUntil.load(
            std::memory_order_acquire);
        const auto expectedRoot = reinterpret_cast<HWND>(
            contextPasteRoot.load(std::memory_order_acquire));
        if (until == 0 || now > until || foreground != expectedRoot) {
            if (until != 0 && now > until) {
                contextPasteIntentUntil.store(0,
                    std::memory_order_release);
                contextPasteRoot.store(0,
                    std::memory_order_release);
            }
            return false;
        }
        contextPasteIntentUntil.store(0, std::memory_order_release);
        contextPasteRoot.store(0, std::memory_order_release);
        return true;
    }

    struct PendingWrite {
        WindowsClipboardContent content;
        std::promise<bool> completed;
    };

    struct PendingAdvertisement {
        std::uint32_t capabilities = 0;
        std::string originMarker;
        std::promise<std::uint32_t> completed;
    };

    static LRESULT CALLBACK WindowProc(HWND window, UINT message,
                                       WPARAM wparam, LPARAM lparam)
    {
        auto* self = reinterpret_cast<Impl*>(
            GetWindowLongPtrW(window, GWLP_USERDATA));
        if (message == WM_NCCREATE) {
            const auto* create = reinterpret_cast<CREATESTRUCTW*>(lparam);
            self = static_cast<Impl*>(create->lpCreateParams);
            SetWindowLongPtrW(window, GWLP_USERDATA,
                              reinterpret_cast<LONG_PTR>(self));
        }
        if (!self) {
            return DefWindowProcW(window, message, wparam, lparam);
        }
        if (message == WM_CLIPBOARDUPDATE) {
            const std::uint32_t sequence = GetClipboardSequenceNumber();
            if (sequence != 0 && sequence ==
                    self->advertisedSequence.load(
                        std::memory_order_acquire)) {
                return 0;
            }
            self->Capture(false);
            return 0;
        }
        if (message == WM_RENDERFORMAT) {
            self->RenderRemoteFormat(static_cast<UINT>(wparam));
            return 0;
        }
        if (message == kCaptureMessage) {
            self->Capture(true);
            return 0;
        }
        if (message == kWriteMessage) {
            self->ApplyPendingWrites();
            return 0;
        }
        if (message == kAdvertiseMessage) {
            self->ApplyPendingAdvertisements();
            return 0;
        }
        if (message == kClearAdvertisementMessage) {
            self->ClearRemoteAdvertisement();
            return 0;
        }
        return DefWindowProcW(window, message, wparam, lparam);
    }

    void ThreadMain(std::stop_token stopToken)
    {
        const HRESULT oleResult = OleInitialize(nullptr);
        WNDCLASSEXW klass{};
        klass.cbSize = sizeof(klass);
        klass.lpfnWndProc = &WindowProc;
        klass.hInstance = GetModuleHandleW(nullptr);
        klass.lpszClassName = kWindowClassName;
        RegisterClassExW(&klass);
        HWND created = CreateWindowExW(
            0, kWindowClassName, L"", 0, 0, 0, 0, 0, HWND_MESSAGE,
            nullptr, klass.hInstance, this);
        if (created) {
            AddClipboardFormatListener(created);
        }
        {
            std::lock_guard lock(mutex);
            window = created;
            startupComplete = true;
        }
        startup.notify_all();
        MSG message{};
        while (!stopToken.stop_requested() &&
               GetMessageW(&message, nullptr, 0, 0) > 0) {
            TranslateMessage(&message);
            DispatchMessageW(&message);
        }
        if (created) {
            RemoveClipboardFormatListener(created);
            DestroyWindow(created);
        }
        UnregisterClassW(kWindowClassName, klass.hInstance);
        {
            std::lock_guard lock(mutex);
            window = nullptr;
        }
        if (SUCCEEDED(oleResult)) OleUninitialize();
    }

    void KeyboardThreadMain(std::stop_token stopToken)
    {
        keyboardThreadId.store(GetCurrentThreadId(),
            std::memory_order_release);
        // Force creation of the thread message queue before Stop() can post
        // WM_QUIT to it.
        MSG message{};
        (void)PeekMessageW(&message, nullptr, WM_USER, WM_USER, PM_NOREMOVE);
        hookOwner = this;
        keyboardHook = SetWindowsHookExW(
            WH_KEYBOARD_LL, &LowLevelKeyboardProc,
            GetModuleHandleW(nullptr), 0);
        mouseHook = SetWindowsHookExW(
            WH_MOUSE_LL, &LowLevelMouseProc,
            GetModuleHandleW(nullptr), 0);
        while (!stopToken.stop_requested() &&
               GetMessageW(&message, nullptr, 0, 0) > 0) {
            TranslateMessage(&message);
            DispatchMessageW(&message);
        }
        if (keyboardHook) {
            UnhookWindowsHookEx(keyboardHook);
            keyboardHook = nullptr;
        }
        if (mouseHook) {
            UnhookWindowsHookEx(mouseHook);
            mouseHook = nullptr;
        }
        if (hookOwner == this) hookOwner = nullptr;
        keyboardThreadId.store(0, std::memory_order_release);
    }

    void Capture(bool forced)
    {
        if (!forced && !enabled.load(std::memory_order_acquire)) {
            return;
        }
        WindowsClipboardContent content;
        content.explicitRequest = forced;
        if (!OpenClipboardWithRetry(window)) {
            if (!forced) return;
            content.captureError =
                "The local clipboard is temporarily unavailable.";
            ChangeCallback target;
            {
                std::lock_guard lock(mutex);
                target = callback;
            }
            if (target) target(std::move(content));
            return;
        }
        content.windowsSequence = GetClipboardSequenceNumber();
        content.empty = CountClipboardFormats() == 0;
        const UINT originFormat = RegisterClipboardFormatW(kOriginFormatName);
        std::vector<std::uint8_t> origin;
        if (originFormat && CopyGlobalBytes(originFormat, &origin)) {
            content.originMarker.assign(origin.begin(), origin.end());
        }
        if (HANDLE textHandle = GetClipboardData(CF_UNICODETEXT)) {
            const auto* text = static_cast<const wchar_t*>(
                GlobalLock(textHandle));
            if (text) {
                const std::string utf8 = WideToUtf8(text);
                GlobalUnlock(textHandle);
                if (!utf8.empty()) {
                    WindowsClipboardPart part;
                    part.format = ClipboardFormat::kUnicodeText;
                    part.data.assign(utf8.begin(), utf8.end());
                    content.parts.push_back(std::move(part));
                }
            }
        }
        const auto readRegistered = [&content](const wchar_t* name,
                                               ClipboardFormat format) {
            const UINT id = RegisterClipboardFormatW(name);
            WindowsClipboardPart part;
            part.format = format;
            if (id && IsClipboardFormatAvailable(id) &&
                CopyGlobalBytes(id, &part.data) && !part.data.empty()) {
                content.parts.push_back(std::move(part));
            }
        };
        readRegistered(kHtmlFormatName, ClipboardFormat::kHtml);
        readRegistered(kRtfFormatName, ClipboardFormat::kRtf);
        const UINT pngFormat = RegisterClipboardFormatW(kPngFormatName);
        WindowsClipboardPart pngPart;
        pngPart.format = ClipboardFormat::kPng;
        if (!(pngFormat && IsClipboardFormatAvailable(pngFormat) &&
              CopyGlobalBytes(pngFormat, &pngPart.data))) {
            HANDLE dibHandle = GetClipboardData(CF_DIBV5);
            if (!dibHandle) {
                dibHandle = GetClipboardData(CF_DIB);
            }
            if (dibHandle) {
                const SIZE_T dibSize = GlobalSize(dibHandle);
                const void* dib = GlobalLock(dibHandle);
                if (dib) {
                    (void)EncodeDibToPng(
                        dib, static_cast<std::size_t>(dibSize),
                        &pngPart.data);
                    GlobalUnlock(dibHandle);
                }
            }
        }
        if (!pngPart.data.empty()) {
            content.parts.push_back(std::move(pngPart));
        }
        if (HDROP drop = static_cast<HDROP>(GetClipboardData(CF_HDROP))) {
            const UINT count = DragQueryFileW(drop, 0xffffffffu, nullptr, 0);
            for (UINT index = 0; index < count && index < 256; ++index) {
                const UINT length = DragQueryFileW(drop, index, nullptr, 0);
                std::wstring path(static_cast<std::size_t>(length) + 1, L'\0');
                if (DragQueryFileW(drop, index, path.data(), length + 1) == 0) {
                    continue;
                }
                path.resize(length);
                WindowsClipboardPart part;
                std::error_code pathError;
                part.format = std::filesystem::is_directory(path, pathError)
                    ? ClipboardFormat::kDirectory
                    : ClipboardFormat::kFile;
                part.localPath = path;
                part.name = WideToUtf8(
                    std::filesystem::path(path).filename().wstring());
                content.parts.push_back(std::move(part));
            }
        }
        CloseClipboard();
        const bool hasFileSystemItems = std::any_of(
            content.parts.begin(), content.parts.end(),
            [](const WindowsClipboardPart& part) {
                return part.format == ClipboardFormat::kFile ||
                    part.format == ClipboardFormat::kDirectory;
            });
        if (!hasFileSystemItems) {
            // Explorer and redirected/cloud clipboards may expose local files
            // only through the Shell IDataObject even though ordinary paste
            // in Explorer works. Resolve those filesystem-backed shell items
            // after releasing OpenClipboard.
            AppendShellFileSystemItems(&content);
        }
        ChangeCallback target;
        {
            std::lock_guard lock(mutex);
            target = callback;
        }
        if (target && (forced || !content.parts.empty() || content.empty)) {
            target(std::move(content));
        }
    }

    bool Apply(const WindowsClipboardContent& content)
    {
        if (!OpenClipboardWithRetry(window)) {
            return false;
        }
        if (!EmptyClipboard()) {
            CloseClipboard();
            return false;
        }
        bool wroteAny = false;
        std::vector<std::wstring> files;
        for (const auto& part : content.parts) {
            switch (part.format) {
            case ClipboardFormat::kUnicodeText: {
                const std::string utf8(part.data.begin(), part.data.end());
                const std::wstring wide = Utf8ToWide(utf8);
                if (!wide.empty()) {
                    const std::size_t bytes =
                        (wide.size() + 1) * sizeof(wchar_t);
                    wroteAny |= PutGlobalBytes(
                        CF_UNICODETEXT, wide.c_str(), bytes, false);
                }
                break;
            }
            case ClipboardFormat::kHtml:
                wroteAny |= PutGlobalBytes(
                    RegisterClipboardFormatW(kHtmlFormatName),
                    part.data.data(), part.data.size(), true);
                break;
            case ClipboardFormat::kRtf:
                wroteAny |= PutGlobalBytes(
                    RegisterClipboardFormatW(kRtfFormatName),
                    part.data.data(), part.data.size(), true);
                break;
            case ClipboardFormat::kPng: {
                std::vector<std::uint8_t> dib;
                if (DecodePngToDib(part.data, &dib)) {
                    wroteAny |= PutGlobalBytes(
                        RegisterClipboardFormatW(kPngFormatName),
                        part.data.data(), part.data.size(), false);
                    wroteAny |= PutGlobalBytes(
                        CF_DIBV5, dib.data(), dib.size(), false);
                }
                break;
            }
            case ClipboardFormat::kFile:
            case ClipboardFormat::kDirectory:
                if (!part.localPath.empty()) {
                    files.push_back(part.localPath.wstring());
                }
                break;
            }
        }
        wroteAny |= PutFileDrop(files);
        if (!content.originMarker.empty() &&
            (wroteAny || content.empty)) {
            wroteAny |= PutGlobalBytes(
                RegisterClipboardFormatW(kOriginFormatName),
                content.originMarker.data(), content.originMarker.size(),
                true);
        }
        CloseClipboard();
        advertisedSequence.store(0, std::memory_order_release);
        materializedRemoteContent.reset();
        return wroteAny;
    }

    bool RenderFormat(const WindowsClipboardContent& content, UINT format)
    {
        if (format == CF_UNICODETEXT) {
            const auto part = std::find_if(
                content.parts.begin(), content.parts.end(),
                [](const WindowsClipboardPart& value) {
                    return value.format == ClipboardFormat::kUnicodeText;
                });
            if (part == content.parts.end()) return false;
            const std::string utf8(part->data.begin(), part->data.end());
            const std::wstring wide = Utf8ToWide(utf8);
            return !wide.empty() && PutGlobalBytes(
                CF_UNICODETEXT, wide.c_str(),
                (wide.size() + 1) * sizeof(wchar_t), false);
        }
        const UINT htmlFormat = RegisterClipboardFormatW(kHtmlFormatName);
        if (format == htmlFormat) {
            const auto part = std::find_if(
                content.parts.begin(), content.parts.end(),
                [](const WindowsClipboardPart& value) {
                    return value.format == ClipboardFormat::kHtml;
                });
            return part != content.parts.end() && PutGlobalBytes(
                format, part->data.data(), part->data.size(), true);
        }
        const UINT rtfFormat = RegisterClipboardFormatW(kRtfFormatName);
        if (format == rtfFormat) {
            const auto part = std::find_if(
                content.parts.begin(), content.parts.end(),
                [](const WindowsClipboardPart& value) {
                    return value.format == ClipboardFormat::kRtf;
                });
            return part != content.parts.end() && PutGlobalBytes(
                format, part->data.data(), part->data.size(), true);
        }
        const UINT pngFormat = RegisterClipboardFormatW(kPngFormatName);
        if (format == pngFormat || format == CF_DIBV5) {
            const auto part = std::find_if(
                content.parts.begin(), content.parts.end(),
                [](const WindowsClipboardPart& value) {
                    return value.format == ClipboardFormat::kPng;
                });
            if (part == content.parts.end()) return false;
            if (format == pngFormat) {
                return PutGlobalBytes(
                    format, part->data.data(), part->data.size(), false);
            }
            std::vector<std::uint8_t> dib;
            return DecodePngToDib(part->data, &dib) &&
                PutGlobalBytes(CF_DIBV5, dib.data(), dib.size(), false);
        }
        if (format == CF_HDROP) {
            std::vector<std::wstring> files;
            for (const auto& part : content.parts) {
                if ((part.format == ClipboardFormat::kFile ||
                     part.format == ClipboardFormat::kDirectory) &&
                    !part.localPath.empty()) {
                    files.push_back(part.localPath.wstring());
                }
            }
            return PutFileDrop(files);
        }
        return false;
    }

    void ApplyPendingAdvertisements()
    {
        std::deque<std::shared_ptr<PendingAdvertisement>> queued;
        {
            std::lock_guard lock(mutex);
            queued.swap(pendingAdvertisements);
        }
        for (const auto& advertisement : queued) {
            std::uint32_t sequence = 0;
            const bool opened = OpenClipboardWithRetry(window);
            if (opened && EmptyClipboard()) {
                const auto capabilities = advertisement->capabilities;
                if ((capabilities & kClipboardCapabilityUnicodeText) != 0) {
                    (void)SetClipboardData(CF_UNICODETEXT, nullptr);
                }
                if ((capabilities & kClipboardCapabilityHtml) != 0) {
                    (void)SetClipboardData(
                        RegisterClipboardFormatW(kHtmlFormatName), nullptr);
                }
                if ((capabilities & kClipboardCapabilityRtf) != 0) {
                    (void)SetClipboardData(
                        RegisterClipboardFormatW(kRtfFormatName), nullptr);
                }
                if ((capabilities & kClipboardCapabilityPng) != 0) {
                    (void)SetClipboardData(
                        RegisterClipboardFormatW(kPngFormatName), nullptr);
                    (void)SetClipboardData(CF_DIBV5, nullptr);
                }
                if ((capabilities & kClipboardCapabilityFiles) != 0) {
                    (void)SetClipboardData(CF_HDROP, nullptr);
                    const DWORD copyEffect = DROPEFFECT_COPY;
                    (void)PutGlobalBytes(
                        RegisterClipboardFormatW(CFSTR_PREFERREDDROPEFFECT),
                        &copyEffect, sizeof(copyEffect), false);
                }
                if (!advertisement->originMarker.empty()) {
                    (void)PutGlobalBytes(
                        RegisterClipboardFormatW(kOriginFormatName),
                        advertisement->originMarker.data(),
                        advertisement->originMarker.size(), true);
                }
                CloseClipboard();
                sequence = GetClipboardSequenceNumber();
                advertisedSequence.store(sequence,
                    std::memory_order_release);
                materializedRemoteContent.reset();
            }
            else if (opened) {
                CloseClipboard();
            }
            advertisement->completed.set_value(sequence);
        }
    }

    void ClearRemoteAdvertisement()
    {
        const std::uint32_t advertised = advertisedSequence.load(
            std::memory_order_acquire);
        if (advertised != 0 &&
            GetClipboardSequenceNumber() == advertised &&
            OpenClipboardWithRetry(window)) {
            // Recheck after acquiring the clipboard. Never erase content the
            // user copied while this message was waiting in the queue.
            if (GetClipboardSequenceNumber() == advertised) {
                (void)EmptyClipboard();
            }
            CloseClipboard();
        }
        advertisedSequence.store(0, std::memory_order_release);
        materializedRemoteContent.reset();
    }

    void RenderRemoteFormat(UINT format)
    {
        if (materializedRemoteContent) {
            (void)RenderFormat(*materializedRemoteContent, format);
            return;
        }

        // WM_RENDERFORMAT is not proof of a paste. Explorer reads advertised
        // formats while updating menus, previews and focus state. Starting a
        // network transfer here used to freeze Explorer inside
        // GetClipboardData and also caused unsolicited paste prompts. Only a
        // recent, explicit mouse context-menu selection is allowed to turn a
        // delayed-format request into a paste action.
        const HWND foreground = GetForegroundWindow();
        if (!ConsumeContextPasteIntent(foreground)) return;

        PasteCallback target;
        {
            std::lock_guard lock(mutex);
            target = pasteCallback;
        }
        WindowsPasteTarget pasteTarget;
        // Return from WM_RENDERFORMAT immediately. Once the asynchronous
        // transfer completes, ClipboardController either copies directly to
        // the captured Explorer directory or injects one replacement Ctrl+V
        // into the captured application. Explorer is never held waiting for
        // network or disk I/O.
        pasteTarget.injectPasteAfterMaterialization = true;
        pasteTarget.foregroundWindow = reinterpret_cast<std::uintptr_t>(
            foreground);
        if (foreground) {
            GUITHREADINFO info{};
            info.cbSize = sizeof(info);
            const DWORD threadId = GetWindowThreadProcessId(
                foreground, nullptr);
            if (threadId != 0 && GetGUIThreadInfo(threadId, &info)) {
                pasteTarget.focusedWindow =
                    reinterpret_cast<std::uintptr_t>(info.hwndFocus);
            }
        }
        if (target) (void)target(pasteTarget);
    }

    void ApplyPendingWrites()
    {
        std::deque<std::shared_ptr<PendingWrite>> queued;
        {
            std::lock_guard lock(mutex);
            queued.swap(pendingWrites);
        }
        for (const auto& write : queued) {
            write->completed.set_value(Apply(write->content));
        }
    }

    std::mutex mutex;
    std::condition_variable startup;
    ChangeCallback callback;
    PasteCallback pasteCallback;
    std::deque<std::shared_ptr<PendingWrite>> pendingWrites;
    std::deque<std::shared_ptr<PendingAdvertisement>>
        pendingAdvertisements;
    std::optional<WindowsClipboardContent> materializedRemoteContent;
    std::jthread thread;
    std::jthread keyboardThread;
    HWND window = nullptr;
    HHOOK keyboardHook = nullptr;
    HHOOK mouseHook = nullptr;
    bool suppressPasteV = false;
    bool startupComplete = false;
    std::atomic_bool enabled{false};
    std::atomic<std::uint32_t> advertisedSequence{0};
    std::atomic<DWORD> keyboardThreadId{0};
    std::atomic<std::uintptr_t> contextMenuRoot{0};
    std::atomic<std::uint64_t> contextMenuOpenedAt{0};
    std::atomic<std::uintptr_t> contextPasteRoot{0};
    std::atomic<std::uint64_t> contextPasteIntentUntil{0};
};

WindowsClipboardService::WindowsClipboardService()
    : impl_(std::make_unique<Impl>(this))
{}

WindowsClipboardService::~WindowsClipboardService()
{
    Stop();
}

bool WindowsClipboardService::Start(ChangeCallback callback,
                                    PasteCallback pasteCallback,
                                    std::string* error)
{
    if (impl_->thread.joinable()) {
        return true;
    }
    {
        std::lock_guard lock(impl_->mutex);
        impl_->callback = std::move(callback);
        impl_->pasteCallback = std::move(pasteCallback);
        impl_->startupComplete = false;
    }
    impl_->keyboardThread = std::jthread(
        [implementation = impl_.get()](std::stop_token stopToken) {
            implementation->KeyboardThreadMain(stopToken);
        });
    impl_->thread = std::jthread(
        [implementation = impl_.get()](std::stop_token stopToken) {
            implementation->ThreadMain(stopToken);
        });
    std::unique_lock lock(impl_->mutex);
    if (!impl_->startup.wait_for(lock, std::chrono::seconds(3),
            [this] { return impl_->startupComplete; }) || !impl_->window) {
        if (error) {
            *error = "The Windows clipboard listener window could not be created.";
        }
        lock.unlock();
        Stop();
        return false;
    }
    return true;
}

void WindowsClipboardService::Stop()
{
    if (!impl_->thread.joinable()) {
        return;
    }
    impl_->enabled.store(false, std::memory_order_release);
    impl_->thread.request_stop();
    impl_->keyboardThread.request_stop();
    HWND window = nullptr;
    {
        std::lock_guard lock(impl_->mutex);
        window = impl_->window;
    }
    const DWORD keyboardThreadId =
        impl_->keyboardThreadId.load(std::memory_order_acquire);
    if (keyboardThreadId != 0) {
        PostThreadMessageW(keyboardThreadId, WM_QUIT, 0, 0);
    }
    if (window) {
        PostMessageW(window, WM_QUIT, 0, 0);
    }
    impl_->thread.join();
    if (impl_->keyboardThread.joinable()) {
        impl_->keyboardThread.join();
    }
    std::lock_guard lock(impl_->mutex);
    for (const auto& write : impl_->pendingWrites) {
        write->completed.set_value(false);
    }
    for (const auto& advertisement : impl_->pendingAdvertisements) {
        advertisement->completed.set_value(0);
    }
    impl_->callback = {};
    impl_->pasteCallback = {};
    impl_->pendingWrites.clear();
    impl_->pendingAdvertisements.clear();
}

void WindowsClipboardService::SetEnabled(bool enabled)
{
    impl_->enabled.store(enabled, std::memory_order_release);
}

void WindowsClipboardService::RequestCurrent()
{
    HWND window = nullptr;
    {
        std::lock_guard lock(impl_->mutex);
        window = impl_->window;
    }
    if (window) {
        PostMessageW(window, kCaptureMessage, 0, 0);
    }
}

std::uint32_t WindowsClipboardService::AdvertiseRemoteClipboard(
    std::uint32_t capabilities, const std::string& originMarker)
{
    auto advertisement =
        std::make_shared<Impl::PendingAdvertisement>();
    advertisement->capabilities = capabilities;
    advertisement->originMarker = originMarker;
    auto result = advertisement->completed.get_future();
    HWND window = nullptr;
    {
        std::lock_guard lock(impl_->mutex);
        window = impl_->window;
        if (window) {
            impl_->pendingAdvertisements.push_back(advertisement);
        }
    }
    if (!window || !PostMessageW(window, kAdvertiseMessage, 0, 0)) {
        if (window) {
            std::lock_guard lock(impl_->mutex);
            const auto iterator = std::find(
                impl_->pendingAdvertisements.begin(),
                impl_->pendingAdvertisements.end(), advertisement);
            if (iterator != impl_->pendingAdvertisements.end()) {
                impl_->pendingAdvertisements.erase(iterator);
            }
        }
        return 0;
    }
    if (result.wait_for(std::chrono::seconds(3)) !=
            std::future_status::ready) {
        return 0;
    }
    return result.get();
}

void WindowsClipboardService::CancelPendingRemoteRender()
{
    // Remote materialization is asynchronous. Clear only the short-lived
    // mouse intent; no clipboard message thread is allowed to wait on the
    // network transfer anymore.
    impl_->contextPasteIntentUntil.store(0, std::memory_order_release);
    impl_->contextPasteRoot.store(0, std::memory_order_release);
    impl_->contextMenuOpenedAt.store(0, std::memory_order_release);
    impl_->contextMenuRoot.store(0, std::memory_order_release);
}

void WindowsClipboardService::ClearRemoteClipboardAdvertisement()
{
    CancelPendingRemoteRender();
    HWND window = nullptr;
    {
        std::lock_guard lock(impl_->mutex);
        window = impl_->window;
    }
    if (window) {
        (void)PostMessageW(window, kClearAdvertisementMessage, 0, 0);
    }
}

bool WindowsClipboardService::Write(WindowsClipboardContent content,
                                    std::string* error)
{
    HWND window = nullptr;
    auto write = std::make_shared<Impl::PendingWrite>();
    write->content = std::move(content);
    auto result = write->completed.get_future();
    {
        std::lock_guard lock(impl_->mutex);
        window = impl_->window;
        if (window) {
            impl_->pendingWrites.push_back(write);
        }
    }
    if (!window || !PostMessageW(window, kWriteMessage, 0, 0)) {
        if (window) {
            std::lock_guard lock(impl_->mutex);
            const auto iterator = std::find(impl_->pendingWrites.begin(),
                impl_->pendingWrites.end(), write);
            if (iterator != impl_->pendingWrites.end()) {
                impl_->pendingWrites.erase(iterator);
            }
        }
        if (error) {
            *error = "The Windows clipboard service is not running.";
        }
        return false;
    }
    if (result.wait_for(std::chrono::seconds(3)) !=
            std::future_status::ready || !result.get()) {
        if (error) {
            *error = "The clipboard content could not be written.";
        }
        return false;
    }
    return true;
}

}  // namespace remote
