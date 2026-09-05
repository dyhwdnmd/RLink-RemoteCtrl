// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#include "ClipboardController.h"

#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <bcrypt.h>
#include <objbase.h>
#include <oaidl.h>
#include <exdisp.h>
#include <servprov.h>
#include <shlobj.h>
#include <shobjidl.h>
#include <wrl/client.h>

#include <algorithm>
#include <array>
#include <cwctype>
#include <fstream>
#include <limits>
#include <system_error>
#include <unordered_set>
#include <utility>

namespace remote::app {
namespace {

constexpr std::size_t kChunkBytes = 56 * 1024;
constexpr std::uint64_t kClipboardSendBurstBytes = 1024 * 1024;
constexpr auto kClipboardProgressPublishInterval =
    std::chrono::milliseconds(50);
constexpr auto kClipboardRateWindow = std::chrono::seconds(3);
constexpr auto kClipboardRateRefreshInterval =
    std::chrono::milliseconds(250);
constexpr std::uint64_t kClipboardProgressReportBytes = 256 * 1024;
constexpr auto kClipboardProgressReportInterval =
    std::chrono::milliseconds(75);
// Never spin inside the clipboard worker waiting for SCTP capacity. A single
// failed attempt is rescheduled below, leaving Cancel/Reject free to preempt
// the next chunk immediately.
constexpr int kClipboardBackpressureRetryCount = 1;
constexpr auto kClipboardBackpressureRetryDelay =
    std::chrono::milliseconds(10);
constexpr std::uint64_t kMaximumTextBytes = 1024 * 1024;
constexpr std::uint64_t kMaximumRichTextBytes = 4 * 1024 * 1024;
constexpr std::uint64_t kMaximumPngBytes = 32 * 1024 * 1024;
constexpr auto kClipboardOfferTimeout = std::chrono::milliseconds(1500);
constexpr auto kClipboardChunkTimeout = std::chrono::milliseconds(2500);
constexpr auto kClipboardApplyTimeout = std::chrono::milliseconds(6000);
constexpr auto kClipboardCaptureTimeout = std::chrono::milliseconds(3000);
constexpr std::uint32_t kMaximumClipboardRetries = 3;

std::uint32_t CapabilityFlags(const RemotePasteConfiguration& config)
{
    std::uint32_t flags = 0;
    flags |= config.unicodeText ? kClipboardCapabilityUnicodeText : 0;
    flags |= config.html ? kClipboardCapabilityHtml : 0;
    flags |= config.rtf ? kClipboardCapabilityRtf : 0;
    flags |= config.png ? kClipboardCapabilityPng : 0;
    flags |= config.files ? kClipboardCapabilityFiles : 0;
    return flags;
}

std::uint32_t ContentFlags(const WindowsClipboardContent& content)
{
    std::uint32_t flags = 0;
    for (const auto& part : content.parts) {
        switch (part.format) {
        case ClipboardFormat::kUnicodeText:
            flags |= kClipboardCapabilityUnicodeText;
            break;
        case ClipboardFormat::kHtml:
            flags |= kClipboardCapabilityHtml;
            break;
        case ClipboardFormat::kRtf:
            flags |= kClipboardCapabilityRtf;
            break;
        case ClipboardFormat::kPng:
            flags |= kClipboardCapabilityPng;
            break;
        case ClipboardFormat::kFile:
        case ClipboardFormat::kDirectory:
            flags |= kClipboardCapabilityFiles;
            break;
        }
    }
    return flags;
}

std::string FormatName(ClipboardFormat format)
{
    switch (format) {
    case ClipboardFormat::kUnicodeText: return "text";
    case ClipboardFormat::kHtml: return "html";
    case ClipboardFormat::kRtf: return "rtf";
    case ClipboardFormat::kPng: return "png";
    case ClipboardFormat::kFile: return "file";
    case ClipboardFormat::kDirectory: return "directory";
    }
    return "unknown";
}

bool FormatEnabled(ClipboardFormat format,
                   const RemotePasteConfiguration& config)
{
    switch (format) {
    case ClipboardFormat::kUnicodeText: return config.unicodeText;
    case ClipboardFormat::kHtml: return config.html;
    case ClipboardFormat::kRtf: return config.rtf;
    case ClipboardFormat::kPng: return config.png;
    case ClipboardFormat::kFile:
    case ClipboardFormat::kDirectory: return config.files;
    }
    return false;
}

std::uint32_t FormatCapability(ClipboardFormat format)
{
    switch (format) {
    case ClipboardFormat::kUnicodeText:
        return kClipboardCapabilityUnicodeText;
    case ClipboardFormat::kHtml:
        return kClipboardCapabilityHtml;
    case ClipboardFormat::kRtf:
        return kClipboardCapabilityRtf;
    case ClipboardFormat::kPng:
        return kClipboardCapabilityPng;
    case ClipboardFormat::kFile:
    case ClipboardFormat::kDirectory:
        return kClipboardCapabilityFiles;
    }
    return 0;
}

std::uint64_t FormatLimit(ClipboardFormat format,
                          const RemotePasteConfiguration& config)
{
    switch (format) {
    case ClipboardFormat::kUnicodeText: return kMaximumTextBytes;
    case ClipboardFormat::kHtml:
    case ClipboardFormat::kRtf: return kMaximumRichTextBytes;
    case ClipboardFormat::kPng: return kMaximumPngBytes;
    case ClipboardFormat::kFile:
    case ClipboardFormat::kDirectory:
        return config.automaticFileLimitBytes;
    }
    return 0;
}

std::string WideToUtf8(std::wstring_view text)
{
    if (text.empty()) return {};
    const int count = WideCharToMultiByte(
        CP_UTF8, WC_ERR_INVALID_CHARS, text.data(),
        static_cast<int>(text.size()), nullptr, 0, nullptr, nullptr);
    if (count <= 0) return {};
    std::string result(static_cast<std::size_t>(count), '\0');
    if (WideCharToMultiByte(CP_UTF8, WC_ERR_INVALID_CHARS, text.data(),
            static_cast<int>(text.size()), result.data(), count, nullptr,
            nullptr) != count) {
        return {};
    }
    return result;
}

std::vector<ClipboardFormatDescriptor> BuildClipboardAnnouncementDescriptors(
    const WindowsClipboardContent& content)
{
    std::vector<ClipboardFormatDescriptor> descriptors;
    std::unordered_set<std::wstring> topLevelNames;
    for (const auto& sourcePart : content.parts) {
        if (sourcePart.format != ClipboardFormat::kFile &&
            sourcePart.format != ClipboardFormat::kDirectory) {
            continue;
        }
        if (sourcePart.localPath.empty() ||
            descriptors.size() >= kMaximumClipboardDescriptors) {
            continue;
        }
        std::filesystem::path remoteName =
            sourcePart.localPath.filename();
        if (remoteName.empty()) continue;
        const auto originalStem = remoteName.stem().wstring();
        const auto originalExtension = remoteName.extension().wstring();
        for (std::size_t suffix = 2;; ++suffix) {
            std::wstring folded = remoteName.wstring();
            std::transform(folded.begin(), folded.end(), folded.begin(),
                [](wchar_t character) { return std::towlower(character); });
            if (topLevelNames.insert(folded).second) break;
            remoteName = sourcePart.format == ClipboardFormat::kDirectory
                ? std::filesystem::path(originalStem + L" (" +
                    std::to_wstring(suffix) + L")")
                : std::filesystem::path(originalStem + L" (" +
                    std::to_wstring(suffix) + L")" + originalExtension);
        }

        ClipboardFormatDescriptor descriptor;
        descriptor.format = sourcePart.format;
        descriptor.name = WideToUtf8(remoteName.generic_wstring());
        descriptor.topLevel = true;
        if (descriptor.format == ClipboardFormat::kFile) {
            std::error_code sizeError;
            descriptor.size = std::filesystem::file_size(
                sourcePart.localPath, sizeError);
            if (sizeError) continue;
        }
        // V8 clipboard announcements deliberately omit SHA-256. Hashing is
        // deferred until the receiver has accepted any same-name prompt.
        descriptors.push_back(std::move(descriptor));
    }
    return descriptors;
}

std::wstring Utf8ToWide(std::string_view text)
{
    if (text.empty()) return {};
    const int count = MultiByteToWideChar(
        CP_UTF8, MB_ERR_INVALID_CHARS, text.data(),
        static_cast<int>(text.size()), nullptr, 0);
    if (count <= 0) return {};
    std::wstring result(static_cast<std::size_t>(count), L'\0');
    if (MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, text.data(),
            static_cast<int>(text.size()), result.data(), count) != count) {
        return {};
    }
    return result;
}

class IncrementalSha256 final {
public:
    IncrementalSha256()
    {
        DWORD objectBytes = 0;
        DWORD copied = 0;
        if (BCryptOpenAlgorithmProvider(&algorithm_,
                BCRYPT_SHA256_ALGORITHM, nullptr, 0) < 0 ||
            BCryptGetProperty(algorithm_, BCRYPT_OBJECT_LENGTH,
                reinterpret_cast<PUCHAR>(&objectBytes), sizeof(objectBytes),
                &copied, 0) < 0) {
            return;
        }
        object_.resize(objectBytes);
        valid_ = BCryptCreateHash(algorithm_, &hash_, object_.data(),
            objectBytes, nullptr, 0, 0) >= 0;
    }

    ~IncrementalSha256()
    {
        if (hash_) BCryptDestroyHash(hash_);
        if (algorithm_) BCryptCloseAlgorithmProvider(algorithm_, 0);
    }

    IncrementalSha256(const IncrementalSha256&) = delete;
    IncrementalSha256& operator=(const IncrementalSha256&) = delete;

    bool Valid() const { return valid_ && !finished_; }

    bool Update(std::span<const std::uint8_t> bytes)
    {
        if (!Valid()) return false;
        return bytes.empty() || BCryptHashData(hash_,
            const_cast<PUCHAR>(bytes.data()),
            static_cast<ULONG>(bytes.size()), 0) >= 0;
    }

    bool Finish(std::array<std::uint8_t, 32>* digest)
    {
        if (!digest || !Valid() || BCryptFinishHash(hash_, digest->data(),
                static_cast<ULONG>(digest->size()), 0) < 0) {
            return false;
        }
        finished_ = true;
        return true;
    }

private:
    BCRYPT_ALG_HANDLE algorithm_ = nullptr;
    BCRYPT_HASH_HANDLE hash_ = nullptr;
    std::vector<std::uint8_t> object_;
    bool valid_ = false;
    bool finished_ = false;
};

bool SafeRelativePath(const std::string& utf8,
                      std::filesystem::path* output)
{
    if (!output || utf8.empty() || utf8.size() > 1024) return false;
    const std::wstring wide = Utf8ToWide(utf8);
    if (wide.empty()) return false;
    std::filesystem::path path(wide);
    if (path.is_absolute() || path.has_root_name() || path.has_root_directory())
        return false;
    for (const auto& component : path) {
        const std::wstring name = component.native();
        if (name == L".." || name == L"." || name.empty() ||
            name.back() == L'.' || name.back() == L' ')
            return false;
        for (const wchar_t character : name) {
            if (character < 32 || character == L'<' || character == L'>' ||
                character == L':' || character == L'"' || character == L'|' ||
                character == L'?' || character == L'*') return false;
        }
        std::wstring base = name.substr(0, name.find(L'.'));
        std::transform(base.begin(), base.end(), base.begin(),
            [](wchar_t character) { return std::towupper(character); });
        const bool numberedDevice = base.size() == 4 &&
            (base.rfind(L"COM", 0) == 0 || base.rfind(L"LPT", 0) == 0) &&
            base[3] >= L'1' && base[3] <= L'9';
        if (base == L"CON" || base == L"PRN" || base == L"AUX" ||
            base == L"NUL" || numberedDevice) return false;
    }
    *output = path.lexically_normal();
    return true;
}

bool SameDescriptor(const ClipboardFormatDescriptor& left,
                    const ClipboardFormatDescriptor& right)
{
    return left.format == right.format && left.name == right.name &&
        left.size == right.size && left.sha256 == right.sha256 &&
        left.hasSha256 == right.hasSha256 &&
        left.topLevel == right.topLevel;
}

bool SameDescriptorMetadata(const ClipboardFormatDescriptor& left,
                            const ClipboardFormatDescriptor& right)
{
    return left.format == right.format && left.name == right.name &&
        left.size == right.size && left.topLevel == right.topLevel;
}

bool WindowHasClass(HWND window, const wchar_t* expected)
{
    if (!window || !expected) return false;
    wchar_t className[128]{};
    return GetClassNameW(window, className,
                         static_cast<int>(std::size(className))) > 0 &&
        _wcsicmp(className, expected) == 0;
}

bool IsDesktopPasteTarget(HWND target, HWND focused)
{
    // Keyboard paste on the desktop normally focuses FolderView. Depending
    // on the Windows version and multi-monitor layout its top-level owner is
    // either Progman or a WorkerW containing SHELLDLL_DefView.
    for (HWND current = focused; current;
         current = GetParent(current)) {
        if (WindowHasClass(current, L"SHELLDLL_DefView")) return true;
        if (current == target) break;
    }
    if (WindowHasClass(target, L"Progman")) return true;
    return WindowHasClass(target, L"WorkerW") &&
        FindWindowExW(target, nullptr, L"SHELLDLL_DefView", nullptr);
}

std::filesystem::path DesktopDirectoryForWindow(HWND target, HWND focused)
{
    if (!IsDesktopPasteTarget(target, focused)) return {};
    PWSTR path = nullptr;
    const HRESULT result = SHGetKnownFolderPath(
        FOLDERID_Desktop, KF_FLAG_DEFAULT, nullptr, &path);
    std::filesystem::path desktop;
    if (SUCCEEDED(result) && path && *path) desktop = path;
    if (path) CoTaskMemFree(path);
    return desktop;
}

std::filesystem::path ExplorerDirectoryForWindow(
    std::uintptr_t windowHandle,
    std::uintptr_t focusedWindowHandle)
{
    HWND target = reinterpret_cast<HWND>(windowHandle);
    const HWND focused = reinterpret_cast<HWND>(focusedWindowHandle);
    if (!target || !IsWindow(target)) return {};
    if (HWND root = GetAncestor(target, GA_ROOT)) target = root;

    if (const auto desktop = DesktopDirectoryForWindow(target, focused);
        !desktop.empty()) {
        return desktop;
    }

    const HRESULT initialized = CoInitializeEx(
        nullptr, COINIT_APARTMENTTHREADED);
    const bool uninitialize = SUCCEEDED(initialized);
    if (FAILED(initialized) && initialized != RPC_E_CHANGED_MODE) return {};

    using Microsoft::WRL::ComPtr;
    std::filesystem::path result;
    int bestScore = (std::numeric_limits<int>::min)();
    ComPtr<IShellWindows> shellWindows;
    if (SUCCEEDED(CoCreateInstance(
            CLSID_ShellWindows, nullptr, CLSCTX_ALL,
            IID_PPV_ARGS(&shellWindows))) && shellWindows) {
        long count = 0;
        if (SUCCEEDED(shellWindows->get_Count(&count))) {
            for (long indexValue = 0; indexValue < count; ++indexValue) {
                VARIANT index{};
                VariantInit(&index);
                index.vt = VT_I4;
                index.lVal = indexValue;
                ComPtr<IDispatch> dispatch;
                if (FAILED(shellWindows->Item(index, &dispatch)) ||
                    !dispatch) {
                    continue;
                }
                ComPtr<IWebBrowserApp> browser;
                if (FAILED(dispatch.As(&browser)) || !browser) continue;
                SHANDLE_PTR browserHandle = 0;
                if (FAILED(browser->get_HWND(&browserHandle))) continue;
                HWND browserWindow = reinterpret_cast<HWND>(browserHandle);
                if (HWND root = GetAncestor(browserWindow, GA_ROOT)) {
                    browserWindow = root;
                }
                if (browserWindow != target) continue;

                ComPtr<IServiceProvider> provider;
                ComPtr<IShellBrowser> shellBrowser;
                ComPtr<IShellView> shellView;
                ComPtr<IFolderView> folderView;
                ComPtr<IShellItem> folder;
                if (FAILED(browser.As(&provider)) || !provider ||
                    FAILED(provider->QueryService(
                        SID_STopLevelBrowser,
                        IID_PPV_ARGS(&shellBrowser))) || !shellBrowser ||
                    FAILED(shellBrowser->QueryActiveShellView(&shellView)) ||
                    !shellView || FAILED(shellView.As(&folderView)) ||
                    !folderView || FAILED(folderView->GetFolder(
                        IID_PPV_ARGS(&folder))) || !folder) {
                    continue;
                }
                HWND viewWindow = nullptr;
                (void)shellView->GetWindow(&viewWindow);
                int score = 0;
                if (viewWindow && IsWindowVisible(viewWindow)) {
                    score += 10;
                }
                if (focused && IsWindow(focused) && viewWindow &&
                    (focused == viewWindow ||
                     IsChild(viewWindow, focused))) {
                    // Windows 11 exposes every tab through IShellWindows with
                    // the same top-level Explorer HWND. The focused child is
                    // the reliable discriminator for the selected tab.
                    score += 100;
                }
                PWSTR path = nullptr;
                if (SUCCEEDED(folder->GetDisplayName(
                        SIGDN_FILESYSPATH, &path)) && path && *path) {
                    if (score > bestScore) {
                        result = path;
                        bestScore = score;
                    }
                }
                if (path) CoTaskMemFree(path);
            }
        }
    }
    if (uninitialize) CoUninitialize();
    return result;
}

bool CopyPathReplacingConflicts(
    const std::filesystem::path& source,
    const std::filesystem::path& target,
    bool isDirectory,
    bool replaceConflicts,
    std::error_code* error)
{
    if (!error) return false;
    error->clear();
    const bool targetExists = std::filesystem::exists(target, *error);
    if (*error) return false;
    if (targetExists && !replaceConflicts) {
        *error = std::make_error_code(std::errc::file_exists);
        return false;
    }

    if (!isDirectory) {
        if (targetExists && std::filesystem::is_directory(target, *error)) {
            if (*error) return false;
            std::filesystem::remove_all(target, *error);
            if (*error) return false;
        }
        (void)std::filesystem::copy_file(
            source, target,
            replaceConflicts
                ? std::filesystem::copy_options::overwrite_existing
                : std::filesystem::copy_options::none,
            *error);
        return !*error;
    }

    if (targetExists && !std::filesystem::is_directory(target, *error)) {
        if (*error) return false;
        std::filesystem::remove(target, *error);
        if (*error) return false;
    }
    std::filesystem::create_directories(target, *error);
    if (*error) return false;
    for (std::filesystem::recursive_directory_iterator iterator(
             source,
             std::filesystem::directory_options::skip_permission_denied,
             *error), end;
         !*error && iterator != end; iterator.increment(*error)) {
        const auto relative = std::filesystem::relative(
            iterator->path(), source, *error);
        if (*error) break;
        const auto childTarget = target / relative;
        if (iterator->is_directory(*error)) {
            if (*error) break;
            const bool childExists = std::filesystem::exists(
                childTarget, *error);
            if (*error) break;
            if (childExists &&
                !std::filesystem::is_directory(childTarget, *error)) {
                if (*error) break;
                std::filesystem::remove_all(childTarget, *error);
                if (*error) break;
            }
            std::filesystem::create_directories(childTarget, *error);
        } else if (iterator->is_regular_file(*error)) {
            if (*error) break;
            const bool childExists = std::filesystem::exists(
                childTarget, *error);
            if (*error) break;
            if (childExists &&
                std::filesystem::is_directory(childTarget, *error)) {
                if (*error) break;
                std::filesystem::remove_all(childTarget, *error);
                if (*error) break;
            }
            std::filesystem::create_directories(
                childTarget.parent_path(), *error);
            if (*error) break;
            (void)std::filesystem::copy_file(
                iterator->path(), childTarget,
                std::filesystem::copy_options::overwrite_existing,
                *error);
        }
    }
    return !*error;
}

}  // namespace

struct ClipboardController::TransferPart {
    ClipboardFormatDescriptor descriptor;
    std::vector<std::uint8_t> data;
    std::filesystem::path sourcePath;
    std::filesystem::path receivePath;
    std::filesystem::path temporaryPath;
    std::unique_ptr<std::ifstream> sourceStream;
    std::unique_ptr<std::ofstream> receiveStream;
    std::unique_ptr<IncrementalSha256> sendHasher;
    std::unique_ptr<IncrementalSha256> receiveHasher;
    std::filesystem::file_time_type sourceWriteTime{};
    std::uint64_t sourceSize = 0;
    bool sourceMetadataCaptured = false;
};

struct ClipboardController::OutgoingTransfer {
    struct RateSample {
        std::chrono::steady_clock::time_point at;
        std::uint64_t bytes = 0;
    };

    std::string itemId;
    std::uint64_t version = 0;
    std::vector<TransferPart> parts;
    std::size_t partIndex = 0;
    // Bytes accepted into the reliable ordered DataChannel. Transport
    // bufferedAmount, not an application ACK window, limits data in flight.
    std::uint64_t offset = 0;
    // Bytes the peer has actually written to its receive cache. This drives
    // user-visible progress; offset only measures local DataChannel enqueue.
    std::uint64_t deliveredBytes = 0;
    std::uint32_t retryCount = 0;
    bool backpressurePending = false;
    bool awaitingApplyResult = false;
    std::chrono::steady_clock::time_point progressPublishedAt =
        std::chrono::steady_clock::now();
    std::chrono::steady_clock::time_point ratePublishedAt{};
    std::deque<RateSample> rateSamples;
};

struct ClipboardController::IncomingTransfer {
    struct RateSample {
        std::chrono::steady_clock::time_point at;
        std::uint64_t bytes = 0;
    };

    std::string itemId;
    std::uint64_t version = 0;
    std::string originDeviceId;
    std::string peerSessionId;
    std::vector<TransferPart> parts;
    std::vector<std::uint64_t> received;
    std::uint64_t progressReportedBytes = 0;
    std::chrono::steady_clock::time_point progressReportedAt =
        std::chrono::steady_clock::now();
    std::chrono::steady_clock::time_point progressPublishedAt =
        std::chrono::steady_clock::now();
    std::chrono::steady_clock::time_point ratePublishedAt{};
    std::deque<RateSample> rateSamples;
    std::filesystem::path cacheDirectory;
};

ClipboardController::ClipboardController(
    SendCallback send, PasteCallback paste,
    std::unique_ptr<WindowsClipboardService> service)
    : send_(std::move(send)), paste_(std::move(paste)),
      service_(std::move(service)),
      cacheManager_(std::make_unique<ClipboardCacheManager>())
{
    worker_ = std::jthread(
        [this](std::stop_token stopToken) { WorkerMain(stopToken); });
    std::string error;
    if (!service_ || !service_->Start(
            [this](WindowsClipboardContent content) {
                const bool ordinaryUserChange = !content.explicitRequest &&
                    !content.originMarker.starts_with("RemoteC|");
                if (ordinaryUserChange) {
                    content.localChangeGeneration =
                        localClipboardGeneration_.fetch_add(
                            1, std::memory_order_acq_rel) + 1;
                } else {
                    content.localChangeGeneration =
                        localClipboardGeneration_.load(
                            std::memory_order_acquire);
                }
                const auto current = Snapshot();
                const bool priority = ordinaryUserChange &&
                    (current.transferActive ||
                     pasteRequestPending_.load(std::memory_order_acquire) ||
                     localPasteHotkeyPending_.load(
                         std::memory_order_acquire));
                auto task = [this, content = std::move(content)]() mutable {
                    HandleLocalClipboard(std::move(content));
                };
                if (priority) {
                    PostPriority(std::move(task));
                } else {
                    Post(std::move(task));
                }
            },
            [this](WindowsPasteTarget target) {
                return HandleLocalPasteHotkey(target);
            },
            &error)) {
        std::lock_guard lock(mutex_);
        snapshot_.lastErrorCode = "clipboard_listener_start_failed";
        snapshot_.lastErrorMessage = error;
    }
}

ClipboardController::~ClipboardController()
{
    if (service_) service_->Stop();
    {
        std::lock_guard lock(mutex_);
        stopping_ = true;
        workQueue_.clear();
        deferredLocalClipboard_.reset();
        pendingLocalClipboard_.reset();
    }
    workAvailable_.notify_all();
    worker_.request_stop();
    if (worker_.joinable()) worker_.join();
    // A completed cache may still be read by Explorer/QQ after RemoteC exits,
    // but an unfinished receive is never useful and must not survive as a
    // protected .part entry.
    if (incoming_) CleanupIncomingTransferCache();
}

void ClipboardController::SetObserver(IClipboardControllerObserver* observer)
{
    {
        std::lock_guard lock(mutex_);
        observer_ = observer;
    }
    PublishSnapshot();
}

void ClipboardController::SetConfiguration(
    RemotePasteConfiguration configuration)
{
    configuration.automaticFileLimitBytes = (std::min)(
        configuration.automaticFileLimitBytes,
        kMaximumClipboardFileBytes);
    Post([this, configuration] {
        configuration_ = configuration;
        if (cacheManager_) {
            ClipboardCacheConfiguration cacheConfiguration;
            cacheConfiguration.baseDirectory =
                configuration.cacheBaseDirectory;
            cacheConfiguration.retention = configuration.cacheRetention;
            cacheConfiguration.requestedLimitBytes =
                configuration.cacheLimitBytes;
            cacheManager_->SetConfiguration(
                std::move(cacheConfiguration));
            UpdateCacheSnapshot();
        }
        snapshot_.enabled = configuration.enabled;
        UpdateClipboardListenerState();
        localCapabilitiesSent_ = false;
        if (!configuration.enabled) {
            CancelTransfers("clipboard_disabled",
                            "Remote paste was disabled.", false);
            ClearRemoteAnnouncement(true);
            snapshot_.state = "disabled";
        } else {
            snapshot_.state = "negotiating";
        }
        if (context_.active && context_.transportReady) {
            SendCapabilities();
        }
        PublishSnapshot();
    });
}

void ClipboardController::UpdateSession(ClipboardSessionContext context)
{
    Post([this, context = std::move(context)]() mutable {
        const bool changed = context.active != context_.active ||
            context.localIsController != context_.localIsController ||
            context.roomId != context_.roomId ||
            context.localDeviceId != context_.localDeviceId ||
            context.peerDeviceId != context_.peerDeviceId;
        const bool transportBecameReady =
            !context_.transportReady && context.transportReady;
        context_ = std::move(context);
        snapshot_.sessionActive = context_.active;
        snapshot_.peerDeviceId = context_.peerDeviceId;
        if (changed) {
            CancelTransfers("clipboard_session_changed",
                            "The clipboard session changed.", false);
            peerSessionId_.clear();
            peerCapabilities_ = 0;
            localSessionId_ = context_.active
                ? GenerateId("clip") : std::string{};
            localCapabilitiesSent_ = false;
            snapshot_.peerCapabilitiesSeen = false;
            snapshot_.peerEnabled = false;
            logicalClock_ = 0;
            lastAppliedVersion_ = 0;
            lastAppliedOrigin_.clear();
            lastAppliedMarker_.clear();
            lastCompletedIncomingItemId_.clear();
            lastCompletedIncomingVersion_ = 0;
            lastCompletedIncomingAccepted_ = false;
            lastCompletedIncomingErrorCode_.clear();
            lastCompletedIncomingErrorMessage_.clear();
            announcedLocalItemId_.clear();
            announcedLocalVersion_ = 0;
            pendingLocalPasteTarget_ = {};
            fulfillingAnnouncedRequest_ = false;
            localPasteHotkeyPending_.store(false);
            ClearRemoteAnnouncement(true);
            if (!context_.active) {
                snapshot_.state = snapshot_.enabled
                    ? "waiting_for_control" : "disabled";
                capabilityRetryDue_.reset();
            }
        }
        UpdateClipboardListenerState();
        if (context_.active && transportBecameReady) {
            localCapabilitiesSent_ = false;
            snapshot_.peerCapabilitiesSeen = false;
            snapshot_.state = "negotiating";
        } else if (context_.active && !context_.transportReady) {
            snapshot_.state = "recovering";
            capabilityRetryDue_.reset();
            ClearRemoteAnnouncement(true);
            localPasteHotkeyPending_.store(false,
                std::memory_order_release);
        }
        if (context_.active && context_.transportReady &&
            !snapshot_.peerCapabilitiesSeen) {
            snapshot_.state = "negotiating";
            SendCapabilities();
            ScheduleCapabilityRetry();
        }
        PublishSnapshot();
    });
}

bool ClipboardController::RequestPasteFromClipboard()
{
    if (ShouldPassThroughRemotePaste()) return false;
    const auto snapshot = Snapshot();
    if (!snapshot.enabled || !snapshot.sessionActive ||
        !snapshot.peerCapabilitiesSeen || !snapshot.peerEnabled ||
        snapshot.state != "ready") return false;
    bool expected = false;
    if (!pasteRequestPending_.compare_exchange_strong(expected, true)) {
        return false;
    }
    const std::uint32_t requestedWindowsSequence =
        GetClipboardSequenceNumber();
    const bool posted = Post([this, requestedWindowsSequence] {
        if (!configuration_.enabled || !context_.active ||
            !context_.transportReady || !snapshot_.peerCapabilitiesSeen ||
            !snapshot_.peerEnabled || outgoing_) {
            pasteRequestPending_.store(false);
            return;
        }
        snapshot_.state = "capturing";
        captureRetryCount_ = 0;
        PublishSnapshot();
        // Windows permits pasting the same clipboard item repeatedly. Some
        // delayed/RDP IDataObjects, however, stop materializing CF_HDROP after
        // the first read. Rebuild a fresh transfer from our last explicit
        // capture while the Win32 clipboard sequence is unchanged.
        if (cachedLocalClipboard_ && requestedWindowsSequence != 0 &&
            cachedLocalClipboard_->windowsSequence ==
                requestedWindowsSequence &&
            !cachedLocalClipboard_->parts.empty()) {
            auto cached = *cachedLocalClipboard_;
            cached.explicitRequest = true;
            HandleLocalClipboard(std::move(cached));
            return;
        }
        service_->RequestCurrent();
        preparationWatchdogDue_ = std::chrono::steady_clock::now() +
            kClipboardCaptureTimeout;
        workAvailable_.notify_all();
    });
    if (!posted) pasteRequestPending_.store(false);
    return posted;
}

bool ClipboardController::ShouldPassThroughRemotePaste() const
{
    if (remoteClipboardAvailable_.load(std::memory_order_acquire)) {
        return true;
    }
    const std::uint32_t remoteSequence =
        lastRemoteClipboardSequence_.load(std::memory_order_acquire);
    return remoteSequence != 0 &&
        GetClipboardSequenceNumber() == remoteSequence;
}

void ClipboardController::SetRemotePastePassthroughWindow(
    std::uintptr_t windowHandle)
{
    remotePastePassthroughWindow_.store(
        windowHandle, std::memory_order_release);
}

bool ClipboardController::HandleLocalPasteHotkey(WindowsPasteTarget target)
{
    if (!remoteClipboardAvailable_.load(std::memory_order_acquire) ||
        target.foregroundWindow == 0) {
        return false;
    }
    const std::uint32_t announcementSequence =
        remoteAnnouncementLocalSequence_.load(std::memory_order_acquire);
    if (announcementSequence == 0 ||
        GetClipboardSequenceNumber() != announcementSequence) {
        // A local application copied something after the remote notice. The
        // Win32 sequence changes synchronously, so honor the local paste even
        // if WM_CLIPBOARDUPDATE has not reached the worker yet.
        remoteClipboardAvailable_.store(false, std::memory_order_release);
        remoteAnnouncementLocalSequence_.store(0,
            std::memory_order_release);
        return false;
    }
    const auto foreground = reinterpret_cast<HWND>(
        target.foregroundWindow);
    const auto excluded = reinterpret_cast<HWND>(
        remotePastePassthroughWindow_.load(std::memory_order_acquire));
    if (excluded &&
        (foreground == excluded ||
         GetAncestor(foreground, GA_ROOT) == excluded)) {
        return false;
    }
    bool expected = false;
    if (!localPasteHotkeyPending_.compare_exchange_strong(
            expected, true, std::memory_order_acq_rel)) {
        // Keep swallowing auto-repeat while the requested item is being
        // materialized; one user paste produces exactly one final injection.
        return true;
    }
    if (!Post([this, target] {
            RequestAnnouncedRemoteClipboard(target);
        })) {
        localPasteHotkeyPending_.store(false, std::memory_order_release);
        return false;
    }
    return true;
}

void ClipboardController::RequestAnnouncedRemoteClipboard(
    WindowsPasteTarget target)
{
    if (!configuration_.enabled || !context_.active ||
        !context_.localIsController || !context_.transportReady ||
        !snapshot_.peerCapabilitiesSeen || !snapshot_.peerEnabled ||
        remoteAnnouncedItemId_.empty() || remoteAnnouncedVersion_ == 0 ||
        outgoing_ || incoming_) {
        localPasteHotkeyPending_.store(false, std::memory_order_release);
        return;
    }
    if ((remoteAnnouncedCapabilities_ & kClipboardCapabilityFiles) != 0) {
        target.explorerDestinationDirectory =
            ExplorerDirectoryForWindow(
                target.foregroundWindow, target.focusedWindow);
        std::error_code directoryError;
        if (!target.explorerDestinationDirectory.empty() &&
            !std::filesystem::is_directory(
                target.explorerDestinationDirectory, directoryError)) {
            target.explorerDestinationDirectory.clear();
        }
    }
    pendingLocalPasteTarget_ = std::move(target);
    if (!pendingLocalPasteTarget_.explorerDestinationDirectory.empty() &&
        !pendingLocalPasteTarget_.replaceExplorerConflicts &&
        !remoteAnnouncedDescriptors_.empty()) {
        std::vector<std::string> conflicts;
        std::uint64_t announcedBytes = 0;
        std::uint32_t announcedItems = 0;
        for (const auto& descriptor : remoteAnnouncedDescriptors_) {
            announcedBytes += descriptor.size;
            if (!descriptor.topLevel ||
                (descriptor.format != ClipboardFormat::kFile &&
                 descriptor.format != ClipboardFormat::kDirectory)) {
                continue;
            }
            ++announcedItems;
            std::filesystem::path relative;
            if (!SafeRelativePath(descriptor.name, &relative)) continue;
            std::error_code existsError;
            const auto destination =
                pendingLocalPasteTarget_.explorerDestinationDirectory /
                relative.filename();
            if (std::filesystem::exists(destination, existsError) &&
                !existsError) {
                conflicts.push_back(descriptor.name);
            }
        }
        if (!conflicts.empty()) {
            ResetTransferSnapshot();
            pendingRemoteConflictDecision_ = true;
            snapshot_.state = "confirming_conflict";
            // This is metadata-only preflight. No transfer or hashing has
            // started, therefore the progress dialog must remain absent.
            snapshot_.transferId = remoteAnnouncedItemId_;
            snapshot_.transferContainsFiles = true;
            snapshot_.transferItemCount = announcedItems;
            snapshot_.transferTotalBytes = announcedBytes;
            snapshot_.localPasteTargetWindow =
                pendingLocalPasteTarget_.foregroundWindow;
            snapshot_.localPasteDestinationPath = WideToUtf8(
                pendingLocalPasteTarget_.explorerDestinationDirectory
                    .wstring());
            snapshot_.explorerConflictPending = true;
            snapshot_.explorerConflictNames = std::move(conflicts);
            PublishSnapshot();
            return;
        }
    }
    (void)SendAnnouncedRemoteClipboardRequest();
}

bool ClipboardController::SendAnnouncedRemoteClipboardRequest()
{
    if (!localPasteHotkeyPending_.load(std::memory_order_acquire) ||
        remoteAnnouncedItemId_.empty() || remoteAnnouncedVersion_ == 0) {
        localPasteHotkeyPending_.store(false, std::memory_order_release);
        pendingLocalPasteTarget_ = {};
        return false;
    }
    ClipboardMessage request;
    request.type = ClipboardMessageType::kRequestCurrent;
    request.itemId = remoteAnnouncedItemId_;
    request.lamportVersion = remoteAnnouncedVersion_;
    if (!SendProtocolMessage(request)) {
        localPasteHotkeyPending_.store(false, std::memory_order_release);
        pendingLocalPasteTarget_ = {};
        return false;
    }
    pendingRemoteConflictDecision_ = false;
    snapshot_.state = "requesting_remote";
    if ((remoteAnnouncedCapabilities_ & kClipboardCapabilityFiles) != 0) {
        snapshot_.transferActive = true;
        snapshot_.transferOutgoing = false;
        snapshot_.transferContainsFiles = true;
        snapshot_.transferId = remoteAnnouncedItemId_;
        snapshot_.transferItemCount = 0;
        snapshot_.transferTotalBytes = 0;
        for (const auto& descriptor : remoteAnnouncedDescriptors_) {
            snapshot_.transferTotalBytes += descriptor.size;
            if (descriptor.topLevel &&
                (descriptor.format == ClipboardFormat::kFile ||
                 descriptor.format == ClipboardFormat::kDirectory) &&
                snapshot_.transferItemCount <
                    (std::numeric_limits<std::uint32_t>::max)()) {
                ++snapshot_.transferItemCount;
            }
        }
        snapshot_.transferCompletedBytes = 0;
        snapshot_.transferBytesPerSecond = 0.0;
        snapshot_.transferEstimatedRemainingSeconds = 0;
        snapshot_.localPasteTargetWindow =
            pendingLocalPasteTarget_.foregroundWindow;
        snapshot_.localPasteDestinationPath =
            pendingLocalPasteTarget_.explorerDestinationDirectory.empty()
                ? std::string{}
                : WideToUtf8(
                    pendingLocalPasteTarget_.explorerDestinationDirectory
                        .wstring());
    }
    remoteRequestWatchdogDue_ = std::chrono::steady_clock::now() +
        kClipboardCaptureTimeout;
    workAvailable_.notify_all();
    PublishSnapshot();
    return true;
}

bool ClipboardController::PasteIntoLocalTarget(
    WindowsPasteTarget target, std::string* error)
{
    HWND foreground = reinterpret_cast<HWND>(target.foregroundWindow);
    HWND focus = reinterpret_cast<HWND>(target.focusedWindow);
    if (!foreground || !IsWindow(foreground)) {
        if (error) *error = "The original local paste target is unavailable.";
        return false;
    }
    if (HWND root = GetAncestor(foreground, GA_ROOT)) {
        foreground = root;
    }
    if (!focus || !IsWindow(focus) ||
        GetAncestor(focus, GA_ROOT) != foreground ||
        !IsWindowVisible(focus) || !IsWindowEnabled(focus)) {
        // Do not force focus onto Explorer/QQ's top-level frame. It is not a
        // paste target and merely beeps when it receives Ctrl+V. Let the
        // application restore its own active child after activation instead.
        focus = nullptr;
    }

    const DWORD currentThread = GetCurrentThreadId();
    const DWORD targetThread = GetWindowThreadProcessId(foreground, nullptr);
    HWND previousForeground = GetForegroundWindow();
    const DWORD previousThread = previousForeground
        ? GetWindowThreadProcessId(previousForeground, nullptr) : 0;
    const bool attachedTarget = targetThread != 0 &&
        targetThread != currentThread &&
        AttachThreadInput(currentThread, targetThread, TRUE) != FALSE;
    const bool attachedPrevious = previousThread != 0 &&
        previousThread != currentThread && previousThread != targetThread &&
        AttachThreadInput(currentThread, previousThread, TRUE) != FALSE;

    if (IsIconic(foreground)) {
        ShowWindow(foreground, SW_RESTORE);
    }
    (void)BringWindowToTop(foreground);
    (void)SetForegroundWindow(foreground);
    (void)SetActiveWindow(foreground);
    if (focus) (void)SetFocus(focus);

    auto targetIsForeground = [foreground] {
        HWND active = GetForegroundWindow();
        return active && (active == foreground ||
            GetAncestor(active, GA_ROOT) == foreground);
    };
    if (!targetIsForeground()) {
        // Windows foreground-lock rules can reject a delayed activation after
        // a long transfer. A synthetic, injected Alt tap is the documented
        // user-input boundary commonly used before retrying activation; our
        // low-level hook ignores injected events.
        std::array<INPUT, 2> activate{};
        activate[0].type = INPUT_KEYBOARD;
        activate[0].ki.wVk = VK_MENU;
        activate[1] = activate[0];
        activate[1].ki.dwFlags = KEYEVENTF_KEYUP;
        (void)SendInput(static_cast<UINT>(activate.size()),
                        activate.data(), sizeof(INPUT));
        (void)SetForegroundWindow(foreground);
        (void)SetActiveWindow(foreground);
        if (focus) (void)SetFocus(focus);
        Sleep(15);
    }
    if (!targetIsForeground()) {
        if (attachedPrevious) {
            (void)AttachThreadInput(currentThread, previousThread, FALSE);
        }
        if (attachedTarget) {
            (void)AttachThreadInput(currentThread, targetThread, FALSE);
        }
        if (error) {
            *error = "The original local paste target could not be restored.";
        }
        return false;
    }

    // Give the target thread time to finish WM_ACTIVATE/WM_SETFOCUS and
    // establish its real document/file-view child before injecting Ctrl+V.
    // Without this settle point SendInput can succeed while the top-level
    // frame still owns focus, producing only the Windows notification sound.
    DWORD_PTR ignored = 0;
    (void)SendMessageTimeoutW(
        foreground, WM_NULL, 0, 0,
        SMTO_ABORTIFHUNG | SMTO_NOTIMEOUTIFNOTHUNG,
        120, &ignored);
    Sleep(35);
    if (!targetIsForeground()) {
        if (attachedPrevious) {
            (void)AttachThreadInput(currentThread, previousThread, FALSE);
        }
        if (attachedTarget) {
            (void)AttachThreadInput(currentThread, targetThread, FALSE);
        }
        if (error) {
            *error = "The local paste target lost focus while activating.";
        }
        return false;
    }

    const auto makeScanCodeInput = [](WORD virtualKey, bool released) {
        INPUT value{};
        value.type = INPUT_KEYBOARD;
        value.ki.wScan = static_cast<WORD>(MapVirtualKeyW(
            virtualKey, MAPVK_VK_TO_VSC));
        value.ki.dwFlags = KEYEVENTF_SCANCODE |
            (released ? KEYEVENTF_KEYUP : 0);
        return value;
    };
    INPUT controlDown = makeScanCodeInput(VK_LCONTROL, false);
    INPUT controlUp = makeScanCodeInput(VK_LCONTROL, true);
    std::array<INPUT, 2> pasteKey{
        makeScanCodeInput(static_cast<WORD>('V'), false),
        makeScanCodeInput(static_cast<WORD>('V'), true)};

    // Establish the modifier in the target input queue before inserting V.
    // Sending the four virtual-key transitions as one batch allowed a focus
    // change/IME to observe V without Control on some Windows configurations.
    const bool controlPressed =
        SendInput(1, &controlDown, sizeof(INPUT)) == 1;
    if (controlPressed) Sleep(8);
    const bool targetStillForeground = targetIsForeground();
    const bool pasteSent = controlPressed && targetStillForeground &&
        SendInput(static_cast<UINT>(pasteKey.size()), pasteKey.data(),
                  sizeof(INPUT)) == pasteKey.size();
    if (controlPressed) {
        Sleep(4);
        (void)SendInput(1, &controlUp, sizeof(INPUT));
    }
    if (attachedPrevious) {
        (void)AttachThreadInput(currentThread, previousThread, FALSE);
    }
    if (attachedTarget) {
        (void)AttachThreadInput(currentThread, targetThread, FALSE);
    }
    if (!pasteSent) {
        if (error) {
            *error = targetStillForeground
                ? "Ctrl+V could not be delivered to the local target."
                : "The local paste target lost focus before Ctrl+V delivery.";
        }
        return false;
    }
    return true;
}

bool ClipboardController::CopyIncomingFilesToExplorerTarget(
    const std::filesystem::path& destination, std::string* error)
{
    if (!incoming_ || destination.empty()) return false;
    std::error_code filesystemError;
    if (!std::filesystem::is_directory(destination, filesystemError) ||
        filesystemError) {
        if (error) {
            *error = "The Explorer destination folder is no longer available.";
        }
        return false;
    }

    bool copiedAny = false;
    for (const auto& part : incoming_->parts) {
        if (!part.descriptor.topLevel ||
            (part.descriptor.format != ClipboardFormat::kFile &&
             part.descriptor.format != ClipboardFormat::kDirectory)) {
            continue;
        }
        const bool isDirectory =
            part.descriptor.format == ClipboardFormat::kDirectory;
        const std::filesystem::path target =
            destination / part.receivePath.filename();
        if (!CopyPathReplacingConflicts(
                part.receivePath, target, isDirectory,
                pendingLocalPasteTarget_.replaceExplorerConflicts,
                &filesystemError)) {
            if (error) {
                *error = filesystemError ==
                        std::make_error_code(std::errc::file_exists)
                    ? "The Explorer destination already contains an item with the same name."
                    : "A received file could not be copied into the Explorer destination.";
            }
            return false;
        }
        copiedAny = true;
    }
    if (!copiedAny && error) {
        *error = "The received clipboard item contains no top-level files.";
    }
    return copiedAny;
}

bool ClipboardController::RequestPasteFiles(
    std::vector<std::filesystem::path> paths)
{
    const auto snapshot = Snapshot();
    if (paths.empty() || !snapshot.enabled || !snapshot.sessionActive ||
        !snapshot.peerCapabilitiesSeen || !snapshot.peerEnabled ||
        snapshot.state != "ready") return false;
    bool expected = false;
    if (!pasteRequestPending_.compare_exchange_strong(expected, true)) {
        return false;
    }
    const std::uint32_t requestedWindowsSequence =
        GetClipboardSequenceNumber();
    const bool posted = Post([
            this, paths = std::move(paths),
            requestedWindowsSequence]() mutable {
        if (!configuration_.enabled || !configuration_.files ||
            !context_.active || !context_.transportReady ||
            !snapshot_.peerCapabilitiesSeen || !snapshot_.peerEnabled ||
            outgoing_) {
            pasteRequestPending_.store(false);
            return;
        }
        WindowsClipboardContent content;
        content.explicitRequest = true;
        content.localChangeGeneration = localClipboardGeneration_.load(
            std::memory_order_acquire);
        for (auto& path : paths) {
            std::error_code error;
            WindowsClipboardPart part;
            if (std::filesystem::is_directory(path, error)) {
                part.format = ClipboardFormat::kDirectory;
            } else if (!error && std::filesystem::is_regular_file(path, error)) {
                part.format = ClipboardFormat::kFile;
            } else {
                continue;
            }
            part.localPath = std::move(path);
            content.parts.push_back(std::move(part));
        }
        content.windowsSequence = requestedWindowsSequence;
        if (content.parts.empty()) {
            pasteRequestPending_.store(false);
            snapshot_.lastErrorCode = "remote_paste_files_invalid";
            snapshot_.lastErrorMessage =
                "The dropped files are unavailable.";
            ++snapshot_.rejectedItems;
            PublishSnapshot();
            return;
        }
        HandleLocalClipboard(std::move(content));
    });
    if (!posted) pasteRequestPending_.store(false);
    return posted;
}

bool ClipboardController::CancelActiveTransfer()
{
    const auto current = Snapshot();
    if (!current.transferActive &&
        current.state != "capturing" && current.state != "offering" &&
        current.state != "sending" &&
        current.state != "requesting_remote" &&
        current.state != "receiving" &&
        current.state != "applying" &&
        current.state != "confirming_conflict") {
        return false;
    }
    // WM_RENDERFORMAT runs on the clipboard message thread and Explorer may
    // be synchronously blocked inside GetClipboardData. Wake it before this
    // request has to compete with already queued chunk work.
    remoteClipboardAvailable_.store(false, std::memory_order_release);
    remoteAnnouncementLocalSequence_.store(0,
        std::memory_order_release);
    if (service_) {
        service_->CancelPendingRemoteRender();
        service_->ClearRemoteClipboardAdvertisement();
    }
    return PostPriority([this] {
        CancelTransfers("clipboard_user_canceled",
                        "The remote paste was canceled by the user.", true);
        pendingExplorerConflictOffer_.reset();
        ClearRemoteAnnouncement(false);
        snapshot_.state = context_.active ? "ready" : "waiting_for_control";
        snapshot_.lastErrorCode.clear();
        snapshot_.lastErrorMessage.clear();
        PublishSnapshot();
    });
}

bool ClipboardController::ResolveExplorerConflict(bool replaceAndContinue)
{
    return PostPriority([this, replaceAndContinue] {
        if (pendingRemoteConflictDecision_) {
            pendingRemoteConflictDecision_ = false;
            snapshot_.explorerConflictPending = false;
            snapshot_.explorerConflictNames.clear();
            if (!replaceAndContinue) {
                localPasteHotkeyPending_.store(false,
                    std::memory_order_release);
                pendingLocalPasteTarget_ = {};
                ClearRemoteAnnouncement(true);
                ResetTransferSnapshot();
                snapshot_.state = context_.active
                    ? "ready" : "waiting_for_control";
                snapshot_.lastErrorCode.clear();
                snapshot_.lastErrorMessage.clear();
                PublishSnapshot();
                return;
            }
            pendingLocalPasteTarget_.replaceExplorerConflicts = true;
            if (!SendAnnouncedRemoteClipboardRequest()) {
                ResetTransferSnapshot();
                snapshot_.state = context_.active
                    ? "ready" : "waiting_for_control";
                PublishSnapshot();
            }
            return;
        }
        if (!pendingExplorerConflictOffer_) return;
        ClipboardEnvelope offer = std::move(*pendingExplorerConflictOffer_);
        pendingExplorerConflictOffer_.reset();
        explorerConflictKeepaliveDue_.reset();
        snapshot_.explorerConflictPending = false;
        snapshot_.explorerConflictNames.clear();
        if (!replaceAndContinue) {
            ClipboardMessage reject;
            reject.type = ClipboardMessageType::kReject;
            reject.itemId = offer.message.itemId;
            reject.lamportVersion = offer.message.lamportVersion;
            reject.errorCode = "clipboard_user_canceled";
            reject.errorMessage =
                "The paste was canceled because the destination contains an item with the same name.";
            (void)SendProtocolMessage(reject);
            remoteRequestWatchdogDue_.reset();
            localPasteHotkeyPending_.store(false,
                std::memory_order_release);
            pendingLocalPasteTarget_ = {};
            ClearRemoteAnnouncement(true);
            ResetTransferSnapshot();
            snapshot_.state = context_.active
                ? "ready" : "waiting_for_control";
            snapshot_.lastErrorCode.clear();
            snapshot_.lastErrorMessage.clear();
            PublishSnapshot();
            return;
        }
        pendingLocalPasteTarget_.replaceExplorerConflicts = true;
        ProcessOffer(offer);
    });
}

bool ClipboardController::RequestCacheCleanup()
{
    if (!cacheManager_) return false;
    cacheManager_->RequestCleanup(true, [this] {
        (void)Post([this] {
            UpdateCacheSnapshot();
            PublishSnapshot();
        });
    });
    return true;
}

bool ClipboardController::RefreshCacheStatistics()
{
    return Post([this] {
        UpdateCacheSnapshot();
        PublishSnapshot();
    });
}

ClipboardControllerSnapshot ClipboardController::Snapshot() const
{
    std::lock_guard lock(mutex_);
    return publishedSnapshot_;
}

void ClipboardController::OnClipboardMessage(
    const ClipboardEnvelope& envelope)
{
    auto task = [this, envelope] { ProcessEnvelope(envelope); };
    if (envelope.message.type == ClipboardMessageType::kCancel ||
        envelope.message.type == ClipboardMessageType::kReject) {
        PostPriority(std::move(task));
    } else {
        Post(std::move(task));
    }
}

bool ClipboardController::Post(std::function<void()> task)
{
    {
        std::lock_guard lock(mutex_);
        if (stopping_) return false;
        workQueue_.push_back(std::move(task));
    }
    workAvailable_.notify_all();
    return true;
}

bool ClipboardController::PostPriority(std::function<void()> task)
{
    {
        std::lock_guard lock(mutex_);
        if (stopping_) return false;
        workQueue_.push_front(std::move(task));
    }
    workAvailable_.notify_all();
    return true;
}

void ClipboardController::WorkerMain(std::stop_token stopToken)
{
    CleanupClipboardCache();
    while (!stopToken.stop_requested()) {
        std::function<void()> task;
        bool processLocal = false;
        {
            std::unique_lock lock(mutex_);
            const auto localClipboardDue = pendingLocalClipboard_
                ? pendingLocalDue_ : std::chrono::steady_clock::time_point::max();
            const auto capabilityDue = capabilityRetryDue_.value_or(
                std::chrono::steady_clock::time_point::max());
            const auto transferDue = transferWatchdogDue_.value_or(
                std::chrono::steady_clock::time_point::max());
            const auto preparationDue = preparationWatchdogDue_.value_or(
                std::chrono::steady_clock::time_point::max());
            const auto remoteRequestDue = remoteRequestWatchdogDue_.value_or(
                std::chrono::steady_clock::time_point::max());
            const auto conflictKeepaliveDue =
                explorerConflictKeepaliveDue_.value_or(
                    std::chrono::steady_clock::time_point::max());
            const auto due = (std::min)(
                localClipboardDue,
                (std::min)(capabilityDue,
                    (std::min)(transferDue,
                        (std::min)(preparationDue,
                            (std::min)(remoteRequestDue,
                                       conflictKeepaliveDue)))));
            workAvailable_.wait_until(lock, stopToken, due, [this] {
                return stopping_ || !workQueue_.empty();
            });
            if (stopping_) return;
            if (!workQueue_.empty()) {
                task = std::move(workQueue_.front());
                workQueue_.pop_front();
            } else if (pendingLocalClipboard_ &&
                       std::chrono::steady_clock::now() >= pendingLocalDue_) {
                processLocal = true;
            }
        }
        try {
            if (task) task();
            if (processLocal) ProcessPendingLocalClipboard();
            ProcessCapabilityRetry();
            ProcessPreparationWatchdog();
            ProcessRemoteRequestWatchdog();
            ProcessExplorerConflictKeepalive();
            ProcessTransferWatchdog();
        } catch (const std::exception& error) {
            CancelTransfers("clipboard_worker_exception", error.what(), false);
            snapshot_.state = context_.active ? "ready" : "waiting_for_control";
            snapshot_.lastErrorCode = "clipboard_worker_exception";
            snapshot_.lastErrorMessage = error.what();
            ++snapshot_.rejectedItems;
            PublishSnapshot();
        } catch (...) {
            CancelTransfers(
                "clipboard_worker_exception",
                "The remote-paste worker encountered an unknown failure.",
                false);
            snapshot_.state = context_.active ? "ready" : "waiting_for_control";
            snapshot_.lastErrorCode = "clipboard_worker_exception";
            snapshot_.lastErrorMessage =
                "The remote-paste worker encountered an unknown failure.";
            ++snapshot_.rejectedItems;
            PublishSnapshot();
        }
    }
}

void ClipboardController::HandleLocalClipboard(WindowsClipboardContent content)
{
    if (!context_.active || !configuration_.enabled) return;
    const bool remoteOrigin = content.originMarker.starts_with("RemoteC|");
    if (!content.explicitRequest && !remoteOrigin && cacheManager_) {
        cacheManager_->RetireCurrentEntry();
        UpdateCacheSnapshot();
    }
    if (remoteOrigin && !content.explicitRequest) {
        if (content.windowsSequence != 0) {
            lastRemoteClipboardSequence_.store(
                content.windowsSequence, std::memory_order_release);
        }
        return;
    }
    if (!content.explicitRequest && !remoteOrigin && outgoing_) {
        // The current transfer owns a stable snapshot of its source. A later
        // Ctrl+C changes what the user wants to paste next, but it must not
        // cancel bytes that are already in flight. Coalesce repeated copies
        // and publish only the newest item once this send has finished.
        deferredLocalClipboard_ = std::move(content);
        return;
    }
    const bool userReplacedClipboard = !content.explicitRequest &&
        !remoteOrigin && (incoming_ ||
            pasteRequestPending_.load(std::memory_order_acquire) ||
            localPasteHotkeyPending_.load(std::memory_order_acquire));
    if (userReplacedClipboard) {
        // A real user copy still cancels an incoming or not-yet-materialized
        // paste so that RemoteC does not overwrite the user's new clipboard.
        CancelTransfers("clipboard_superseded",
            "A newer clipboard item replaced the active paste.", true);
        snapshot_.state = "ready";
        snapshot_.lastErrorCode.clear();
        snapshot_.lastErrorMessage.clear();
        if (context_.localIsController) {
            cachedLocalClipboard_ = content;
            remoteAnnouncedItemId_.clear();
            remoteAnnouncedVersion_ = 0;
            remoteAnnouncedCapabilities_ = 0;
            remoteAnnouncedDescriptors_.clear();
            pendingRemoteConflictDecision_ = false;
            remoteClipboardAvailable_.store(false,
                std::memory_order_release);
            remoteAnnouncementLocalSequence_.store(0,
                std::memory_order_release);
        } else {
            AnnounceLocalClipboard(std::move(content));
        }
        PublishSnapshot();
        return;
    }
    if (!pasteRequestPending_.load()) {
        if (context_.localIsController) {
            // A real local copy supersedes an unmaterialized remote item.
            // Keep it cached for repeated monitor-window Ctrl+V, but never
            // publish controller clipboard changes in the background.
            if (incoming_) {
                CancelTransfers("clipboard_replaced_locally",
                    "The controller copied a newer local item.", true);
            }
            cachedLocalClipboard_ = content;
            remoteAnnouncedItemId_.clear();
            remoteAnnouncedVersion_ = 0;
            remoteAnnouncedCapabilities_ = 0;
            remoteAnnouncedDescriptors_.clear();
            pendingRemoteConflictDecision_ = false;
            remoteClipboardAvailable_.store(false);
            remoteAnnouncementLocalSequence_.store(0);
            localPasteHotkeyPending_.store(false);
            pendingLocalPasteTarget_ = {};
            return;
        }
        // The controlled endpoint publishes only a tiny version notice here.
        // No hash, file read, DataChannel body, progress popup or bandwidth
        // is consumed until the controller actually pastes in a local app.
        if (outgoing_ || incoming_ || !context_.transportReady ||
            !snapshot_.peerCapabilitiesSeen || !snapshot_.peerEnabled) {
            return;
        }
        AnnounceLocalClipboard(std::move(content));
        return;
    }
    preparationWatchdogDue_.reset();
    if (!content.captureError.empty()) {
        pasteRequestPending_.store(false);
        snapshot_.state = "ready";
        snapshot_.lastErrorCode = "remote_paste_clipboard_unavailable";
        snapshot_.lastErrorMessage = std::move(content.captureError);
        ++snapshot_.rejectedItems;
        PublishSnapshot();
        return;
    }
    if (content.parts.empty()) {
        const bool remoteCLoopback = remoteOrigin;
        if (cachedLocalClipboard_ && remoteCLoopback &&
            !cachedLocalClipboard_->parts.empty()) {
            // RDP clipboard redirection can echo the item that RemoteC placed
            // on the controlled machine back to this desktop. Preserve the
            // user's last explicit local source instead of treating the echo
            // as an unsupported replacement.
            content = *cachedLocalClipboard_;
        }
    }
    if (content.parts.empty()) {
        if (!content.empty && captureRetryCount_ < 2) {
            ++captureRetryCount_;
            preparationWatchdogDue_ = std::chrono::steady_clock::now() +
                kClipboardCaptureTimeout;
            std::this_thread::sleep_for(std::chrono::milliseconds(70));
            service_->RequestCurrent();
            return;
        }
        pasteRequestPending_.store(false);
        snapshot_.state = "ready";
        snapshot_.lastErrorCode = "remote_paste_clipboard_unsupported";
        snapshot_.lastErrorMessage =
            "The local clipboard has no supported content.";
        ++snapshot_.rejectedItems;
        PublishSnapshot();
        return;
    }
    captureRetryCount_ = 0;
    cachedLocalClipboard_ = content;
    pendingLocalClipboard_ = std::move(content);
    pendingLocalDue_ = std::chrono::steady_clock::now();
    workAvailable_.notify_all();
}

void ClipboardController::ProcessDeferredLocalClipboard()
{
    if (outgoing_ || !deferredLocalClipboard_) return;
    auto content = std::move(*deferredLocalClipboard_);
    deferredLocalClipboard_.reset();
    HandleLocalClipboard(std::move(content));
}

void ClipboardController::AnnounceLocalClipboard(
    WindowsClipboardContent content)
{
    if (!content.captureError.empty() || content.empty ||
        content.parts.empty()) {
        return;
    }
    const auto announcementDescriptors =
        BuildClipboardAnnouncementDescriptors(content);
    cachedLocalClipboard_ = std::move(content);
    announcedLocalItemId_ = GenerateId("current");
    announcedLocalVersion_ = ++logicalClock_;
    ClipboardMessage changed;
    changed.type = ClipboardMessageType::kClipboardChanged;
    changed.itemId = announcedLocalItemId_;
    changed.lamportVersion = announcedLocalVersion_;
    changed.capabilities = ContentFlags(*cachedLocalClipboard_);
    changed.descriptors = announcementDescriptors;
    if (!SendProtocolMessage(changed)) {
        announcedLocalItemId_.clear();
        announcedLocalVersion_ = 0;
    }
}

void ClipboardController::ProcessPendingLocalClipboard()
{
    if (!context_.transportReady || !snapshot_.peerCapabilitiesSeen) {
        std::lock_guard lock(mutex_);
        if (pendingLocalClipboard_) {
            pendingLocalDue_ = std::chrono::steady_clock::now() +
                std::chrono::milliseconds(250);
        }
        return;
    }
    std::optional<WindowsClipboardContent> content;
    {
        std::lock_guard lock(mutex_);
        content = std::move(pendingLocalClipboard_);
        pendingLocalClipboard_.reset();
    }
    if (!content || !context_.active || !configuration_.enabled ||
        !snapshot_.peerCapabilitiesSeen || !snapshot_.peerEnabled) {
        pasteRequestPending_.store(false);
        ResetTransferSnapshot();
        snapshot_.state = context_.active ? "ready" : "waiting_for_control";
        PublishSnapshot();
        return;
    }
    if (content->empty) {
        pasteRequestPending_.store(false);
        snapshot_.state = "ready";
        snapshot_.lastErrorCode = "remote_paste_clipboard_empty";
        snapshot_.lastErrorMessage = "The local clipboard is empty.";
        ++snapshot_.rejectedItems;
        PublishSnapshot();
        return;
    }
    if (outgoing_) {
        ClipboardMessage cancel;
        cancel.type = ClipboardMessageType::kCancel;
        cancel.itemId = outgoing_->itemId;
        cancel.lamportVersion = outgoing_->version;
        cancel.errorCode = "clipboard_superseded";
        cancel.errorMessage =
            "A newer clipboard item replaced the transfer.";
        (void)SendProtocolMessage(cancel);
    }
    if (incoming_) {
        CleanupIncomingTransferCache();
    }
    outgoing_.reset();
    incoming_.reset();
    pendingExplorerConflictOffer_.reset();
    explorerConflictKeepaliveDue_.reset();
    std::string error;
    if (!BuildOutgoingTransfer(std::move(*content), &error)) {
        const bool superseded = error == "clipboard_superseded";
        if (fulfillingAnnouncedRequest_ &&
            !announcedLocalItemId_.empty()) {
            ClipboardMessage response;
            response.type = superseded ? ClipboardMessageType::kCancel
                                       : ClipboardMessageType::kReject;
            response.itemId = announcedLocalItemId_;
            response.lamportVersion = announcedLocalVersion_;
            response.errorCode = superseded ? "clipboard_superseded"
                                            : "clipboard_capture_rejected";
            response.errorMessage = superseded
                ? "A newer clipboard item replaced the transfer." : error;
            (void)SendProtocolMessage(response);
        }
        fulfillingAnnouncedRequest_ = false;
        pasteRequestPending_.store(false);
        ResetTransferSnapshot();
        snapshot_.state = "ready";
        snapshot_.lastErrorCode = superseded ? std::string{}
                                             : "clipboard_capture_rejected";
        snapshot_.lastErrorMessage = superseded ? std::string{} : error;
        if (!superseded) ++snapshot_.rejectedItems;
        PublishSnapshot();
        return;
    }
    snapshot_.state = "offering";
    outgoing_->retryCount = 0;
    UpdateOutgoingProgressSnapshot();
    (void)SendOutgoingOffer();
    PublishSnapshot();
}

void ClipboardController::SendCapabilities()
{
    if (!context_.active || !context_.transportReady ||
        localSessionId_.empty()) return;
    ClipboardMessage message;
    message.type = ClipboardMessageType::kCapabilities;
    message.capabilities = CapabilityFlags(configuration_);
    message.enabled = configuration_.enabled;
    if (SendProtocolMessage(message)) {
        localCapabilitiesSent_ = true;
    }
}

void ClipboardController::ScheduleCapabilityRetry()
{
    if (!context_.active || !context_.transportReady ||
        snapshot_.peerCapabilitiesSeen) {
        capabilityRetryDue_.reset();
        return;
    }
    capabilityRetryDue_ = std::chrono::steady_clock::now() +
        std::chrono::milliseconds(350);
}

void ClipboardController::ProcessCapabilityRetry()
{
    if (!capabilityRetryDue_ ||
        std::chrono::steady_clock::now() < *capabilityRetryDue_) {
        return;
    }
    capabilityRetryDue_.reset();
    if (!context_.active || !context_.transportReady ||
        snapshot_.peerCapabilitiesSeen) {
        return;
    }
    SendCapabilities();
    ScheduleCapabilityRetry();
}

void ClipboardController::ProcessPreparationWatchdog()
{
    if (!preparationWatchdogDue_ ||
        std::chrono::steady_clock::now() < *preparationWatchdogDue_) {
        return;
    }
    preparationWatchdogDue_.reset();
    if (snapshot_.state != "capturing" || outgoing_ ||
        !pasteRequestPending_.exchange(false)) {
        return;
    }
    {
        std::lock_guard lock(mutex_);
        pendingLocalClipboard_.reset();
    }
    snapshot_.state = context_.active ? "ready" : "waiting_for_control";
    snapshot_.lastErrorCode = "clipboard_capture_timeout";
    snapshot_.lastErrorMessage =
        "The local clipboard did not answer the remote paste request.";
    ++snapshot_.rejectedItems;
    PublishSnapshot();
}

void ClipboardController::ProcessRemoteRequestWatchdog()
{
    if (!remoteRequestWatchdogDue_ ||
        std::chrono::steady_clock::now() < *remoteRequestWatchdogDue_) {
        return;
    }
    remoteRequestWatchdogDue_.reset();
    if (!localPasteHotkeyPending_.exchange(
            false, std::memory_order_acq_rel) || incoming_) {
        return;
    }
    pendingLocalPasteTarget_ = {};
    ResetTransferSnapshot();
    snapshot_.state = context_.active ? "ready" : "waiting_for_control";
    snapshot_.lastErrorCode = "clipboard_remote_request_timeout";
    snapshot_.lastErrorMessage =
        "The controlled machine did not answer the clipboard request.";
    ++snapshot_.rejectedItems;
    PublishSnapshot();
}

void ClipboardController::ProcessExplorerConflictKeepalive()
{
    if (!explorerConflictKeepaliveDue_ ||
        std::chrono::steady_clock::now() <
            *explorerConflictKeepaliveDue_) {
        return;
    }
    explorerConflictKeepaliveDue_.reset();
    if (!pendingExplorerConflictOffer_) return;
    ClipboardMessage waiting;
    waiting.type = ClipboardMessageType::kProgress;
    waiting.itemId = pendingExplorerConflictOffer_->message.itemId;
    waiting.lamportVersion =
        pendingExplorerConflictOffer_->message.lamportVersion;
    waiting.totalSize = snapshot_.transferTotalBytes;
    waiting.errorCode = "clipboard_waiting_for_conflict_decision";
    (void)SendProtocolMessage(waiting);
    explorerConflictKeepaliveDue_ = std::chrono::steady_clock::now() +
        std::chrono::seconds(1);
    workAvailable_.notify_all();
}

void ClipboardController::ScheduleTransferWatchdog()
{
    if (!outgoing_) {
        transferWatchdogDue_.reset();
        return;
    }
    auto timeout = kClipboardChunkTimeout;
    if (snapshot_.state == "offering") {
        timeout = kClipboardOfferTimeout;
    } else if (outgoing_->awaitingApplyResult) {
        timeout = kClipboardApplyTimeout;
    }
    transferWatchdogDue_ = std::chrono::steady_clock::now() + timeout;
    workAvailable_.notify_all();
}

void ClipboardController::ScheduleBackpressureRetry()
{
    if (!outgoing_) {
        transferWatchdogDue_.reset();
        return;
    }
    outgoing_->backpressurePending = true;
    transferWatchdogDue_ = std::chrono::steady_clock::now() +
        kClipboardBackpressureRetryDelay;
    workAvailable_.notify_all();
}

void ClipboardController::ProcessTransferWatchdog()
{
    if (!transferWatchdogDue_ ||
        std::chrono::steady_clock::now() < *transferWatchdogDue_) {
        return;
    }
    transferWatchdogDue_.reset();
    if (!outgoing_) return;

    if (outgoing_->backpressurePending) {
        outgoing_->backpressurePending = false;
        if (outgoing_->awaitingApplyResult) {
            (void)SendOutgoingCommit();
        } else if (snapshot_.state == "sending") {
            SendNextChunk();
        } else {
            ScheduleTransferWatchdog();
        }
        return;
    }

    if (outgoing_->retryCount >= kMaximumClipboardRetries) {
        CancelTransfers(
            "clipboard_transfer_timeout",
            "The remote paste transfer timed out before the peer confirmed it.",
            true);
        snapshot_.state = context_.active ? "ready" : "waiting_for_control";
        snapshot_.lastErrorCode = "clipboard_transfer_timeout";
        snapshot_.lastErrorMessage =
            "The remote paste transfer timed out before the peer confirmed it.";
        ++snapshot_.rejectedItems;
        PublishSnapshot();
        return;
    }

    ++outgoing_->retryCount;
    if (snapshot_.state == "offering") {
        (void)SendOutgoingOffer();
    } else if (outgoing_->awaitingApplyResult) {
        (void)SendOutgoingCommit();
    } else if (snapshot_.state == "sending") {
        // Resume at the first chunk that was not accepted into the reliable
        // DataChannel. Already accepted chunks remain SCTP's responsibility.
        SendNextChunk();
    } else {
        ScheduleTransferWatchdog();
    }
    PublishSnapshot();
}

void ClipboardController::SendSessionStart()
{
    ClipboardMessage message;
    message.type = ClipboardMessageType::kSessionStart;
    message.capabilities = CapabilityFlags(configuration_);
    message.enabled = configuration_.enabled;
    (void)SendProtocolMessage(message);
}

bool ClipboardController::SendOutgoingOffer()
{
    if (!outgoing_) return false;
    ClipboardMessage offer;
    offer.type = ClipboardMessageType::kOffer;
    offer.itemId = outgoing_->itemId;
    offer.lamportVersion = outgoing_->version;
    for (const auto& part : outgoing_->parts) {
        auto descriptor = part.descriptor;
        descriptor.sha256.fill(0);
        descriptor.hasSha256 = false;
        offer.descriptors.push_back(std::move(descriptor));
        offer.totalSize += part.descriptor.size;
    }
    const bool sent = SendProtocolMessage(offer);
    ScheduleTransferWatchdog();
    return sent;
}

bool ClipboardController::SendOutgoingCommit()
{
    if (!outgoing_) return false;
    ClipboardMessage commit;
    commit.type = ClipboardMessageType::kCommit;
    commit.itemId = outgoing_->itemId;
    commit.lamportVersion = outgoing_->version;
    for (const auto& part : outgoing_->parts) {
        commit.descriptors.push_back(part.descriptor);
        commit.totalSize += part.descriptor.size;
    }
    const bool sent = SendProtocolMessage(commit);
    outgoing_->awaitingApplyResult = true;
    if (!sent && lastSendBackpressured_) {
        ScheduleBackpressureRetry();
    } else {
        ScheduleTransferWatchdog();
    }
    return sent;
}

bool ClipboardController::ResetOutgoingStreamingState(std::string* error)
{
    if (!outgoing_) return false;
    for (auto& part : outgoing_->parts) {
        part.sourceStream.reset();
        part.descriptor.sha256.fill(0);
        part.descriptor.hasSha256 = false;
        if (part.descriptor.format == ClipboardFormat::kDirectory) {
            part.sendHasher.reset();
            continue;
        }
        part.sendHasher = std::make_unique<IncrementalSha256>();
        if (!part.sendHasher->Valid()) {
            if (error) {
                *error =
                    "Clipboard content verification could not start.";
            }
            return false;
        }
    }
    outgoing_->partIndex = 0;
    outgoing_->offset = 0;
    return true;
}

bool ClipboardController::FinalizeOutgoingPart(
    TransferPart* part, std::string* error)
{
    if (!part) return false;
    if (part->descriptor.format == ClipboardFormat::kDirectory) {
        return true;
    }
    if (part->descriptor.hasSha256) return true;
    if (!part->sendHasher ||
        !part->sendHasher->Finish(&part->descriptor.sha256)) {
        if (error) {
            *error = "Clipboard content verification could not finish.";
        }
        return false;
    }
    part->sendHasher.reset();
    part->descriptor.hasSha256 = true;

    if (part->sourceMetadataCaptured) {
        part->sourceStream.reset();
        std::error_code sizeError;
        std::error_code timeError;
        const auto currentSize = std::filesystem::file_size(
            part->sourcePath, sizeError);
        const auto currentWriteTime = std::filesystem::last_write_time(
            part->sourcePath, timeError);
        if (sizeError || timeError || currentSize != part->sourceSize ||
            currentWriteTime != part->sourceWriteTime) {
            if (error) {
                *error =
                    "A copied file changed while it was being transferred.";
            }
            return false;
        }
    }
    return true;
}

void ClipboardController::ProcessEnvelope(ClipboardEnvelope envelope)
{
    if (!context_.active || envelope.senderDeviceId != context_.peerDeviceId ||
        envelope.receiverDeviceId != context_.localDeviceId ||
        envelope.roomId != context_.roomId) return;
    if (envelope.message.type != ClipboardMessageType::kCapabilities &&
        envelope.message.type != ClipboardMessageType::kSessionStart &&
        (peerSessionId_.empty() ||
         envelope.clipboardSessionId != peerSessionId_)) return;
    logicalClock_ = (std::max)(logicalClock_,
                               envelope.message.lamportVersion);
    switch (envelope.message.type) {
    case ClipboardMessageType::kCapabilities:
        capabilityRetryDue_.reset();
        // Capabilities already carries the sender's authenticated clipboard
        // session id. Bind it immediately; otherwise the UI can become ready
        // before SessionStart arrives and the first Offer/ACK is discarded by
        // the session-id guard above.
        if (!peerSessionId_.empty() &&
            peerSessionId_ != envelope.clipboardSessionId) {
            CancelTransfers("peer_clipboard_session_changed",
                            "The peer clipboard session changed.", false);
        }
        peerSessionId_ = envelope.clipboardSessionId;
        snapshot_.peerCapabilitiesSeen = true;
        snapshot_.peerEnabled = envelope.message.enabled;
        peerCapabilities_ = envelope.message.capabilities;
        if (!localCapabilitiesSent_) SendCapabilities();
        SendSessionStart();
        if (!outgoing_ && !incoming_) {
            snapshot_.state = snapshot_.peerEnabled
                ? "ready" : "peer_disabled";
        }
        if (outgoing_ && snapshot_.peerEnabled) {
            std::string resetError;
            if (!ResetOutgoingStreamingState(&resetError)) {
                CancelTransfers(
                    "clipboard_hash_reset_failed", resetError, true);
                snapshot_.state = "ready";
                snapshot_.lastErrorCode =
                    "clipboard_hash_reset_failed";
                snapshot_.lastErrorMessage = std::move(resetError);
                ++snapshot_.rejectedItems;
                PublishSnapshot();
                return;
            }
            outgoing_->deliveredBytes = 0;
            outgoing_->retryCount = 0;
            outgoing_->backpressurePending = false;
            outgoing_->awaitingApplyResult = false;
            snapshot_.state = "offering";
            (void)SendOutgoingOffer();
        }
        PublishSnapshot();
        return;
    case ClipboardMessageType::kSessionStart:
        capabilityRetryDue_.reset();
        if (!peerSessionId_.empty() &&
            peerSessionId_ != envelope.clipboardSessionId) {
            CancelTransfers("peer_clipboard_session_changed",
                            "The peer clipboard session changed.", false);
        }
        snapshot_.peerCapabilitiesSeen = true;
        peerSessionId_ = envelope.clipboardSessionId;
        snapshot_.peerEnabled = envelope.message.enabled;
        peerCapabilities_ = envelope.message.capabilities;
        if (!outgoing_ && !incoming_) {
            snapshot_.state = snapshot_.peerEnabled
                ? "ready" : "peer_disabled";
        }
        PublishSnapshot();
        return;
    case ClipboardMessageType::kSessionStop:
        peerSessionId_.clear();
        peerCapabilities_ = 0;
        CancelTransfers("peer_clipboard_stopped",
                        "The peer stopped remote paste.", false);
        snapshot_.state = "peer_disabled";
        ClearRemoteAnnouncement(true);
        PublishSnapshot();
        return;
    case ClipboardMessageType::kRequestCurrent:
        if (context_.localIsController ||
            envelope.message.itemId != announcedLocalItemId_ ||
            envelope.message.lamportVersion != announcedLocalVersion_ ||
            !cachedLocalClipboard_ || cachedLocalClipboard_->parts.empty() ||
            outgoing_ || incoming_) {
            ClipboardMessage reject;
            reject.type = ClipboardMessageType::kReject;
            reject.itemId = envelope.message.itemId;
            reject.lamportVersion = envelope.message.lamportVersion;
            reject.errorCode = "clipboard_current_unavailable";
            reject.errorMessage =
                "The announced clipboard item is no longer available.";
            (void)SendProtocolMessage(reject);
            return;
        }
        if (!pasteRequestPending_.exchange(true)) {
            fulfillingAnnouncedRequest_ = true;
            snapshot_.state = "capturing";
            pendingLocalClipboard_ = *cachedLocalClipboard_;
            pendingLocalDue_ = std::chrono::steady_clock::now();
            workAvailable_.notify_all();
            PublishSnapshot();
        }
        return;
    case ClipboardMessageType::kClipboardChanged: {
        if (!context_.localIsController) return;
        if (envelope.message.lamportVersion < remoteAnnouncedVersion_) {
            return;
        }
        remoteAnnouncedItemId_ = envelope.message.itemId;
        remoteAnnouncedVersion_ = envelope.message.lamportVersion;
        remoteAnnouncedCapabilities_ = envelope.message.capabilities;
        remoteAnnouncedDescriptors_ = envelope.message.descriptors;
        pendingRemoteConflictDecision_ = false;
        const std::string proxyMarker =
            "RemoteC|Proxy|" + remoteAnnouncedItemId_ + "|" +
            std::to_string(remoteAnnouncedVersion_);
        const std::uint32_t advertisedSequence = service_
            ? service_->AdvertiseRemoteClipboard(
                remoteAnnouncedCapabilities_, proxyMarker)
            : 0;
        remoteClipboardAvailable_.store(true, std::memory_order_release);
        remoteAnnouncementLocalSequence_.store(
            advertisedSequence != 0
                ? advertisedSequence : GetClipboardSequenceNumber(),
            std::memory_order_release);
        localPasteHotkeyPending_.store(false, std::memory_order_release);
        pendingLocalPasteTarget_ = {};
        if (!outgoing_ && !incoming_) {
            ResetTransferSnapshot();
            snapshot_.state = "ready";
        }
        PublishSnapshot();
        return;
    }
    case ClipboardMessageType::kOffer:
        ProcessOffer(envelope);
        return;
    case ClipboardMessageType::kAccept:
        if (outgoing_ && envelope.message.itemId == outgoing_->itemId &&
            envelope.message.lamportVersion == outgoing_->version) {
            outgoing_->retryCount = 0;
            const auto now = std::chrono::steady_clock::now();
            outgoing_->rateSamples.clear();
            outgoing_->rateSamples.push_back(
                OutgoingTransfer::RateSample{
                    now, snapshot_.transferCompletedBytes});
            outgoing_->ratePublishedAt = now;
            snapshot_.transferBytesPerSecond = 0.0;
            snapshot_.transferEstimatedRemainingSeconds = 0;
            snapshot_.state = "sending";
            SendNextChunk();
            PublishSnapshot();
        }
        return;
    case ClipboardMessageType::kReject:
    case ClipboardMessageType::kCancel: {
        const bool superseded =
            envelope.message.errorCode == "clipboard_superseded";
        if (localPasteHotkeyPending_.load(std::memory_order_acquire) &&
            envelope.message.itemId == remoteAnnouncedItemId_) {
            remoteRequestWatchdogDue_.reset();
            localPasteHotkeyPending_.store(false,
                std::memory_order_release);
            pendingLocalPasteTarget_ = {};
            ClearRemoteAnnouncement(true);
            snapshot_.state = "ready";
            snapshot_.lastErrorCode = superseded
                ? std::string{} : envelope.message.errorCode;
            snapshot_.lastErrorMessage = superseded
                ? std::string{} : envelope.message.errorMessage;
            if (!superseded) ++snapshot_.rejectedItems;
            PublishSnapshot();
            return;
        }
        if (outgoing_ && envelope.message.itemId == outgoing_->itemId) {
            outgoing_.reset();
            transferWatchdogDue_.reset();
            pasteRequestPending_.store(false);
            ResetTransferSnapshot();
            snapshot_.state = "ready";
            snapshot_.lastErrorCode = superseded
                ? std::string{} : envelope.message.errorCode;
            snapshot_.lastErrorMessage = superseded
                ? std::string{} : envelope.message.errorMessage;
            if (!superseded) ++snapshot_.rejectedItems;
            ProcessDeferredLocalClipboard();
            PublishSnapshot();
        }
        if (incoming_ && envelope.message.itemId == incoming_->itemId) {
            CleanupIncomingTransferCache();
            incoming_.reset();
            ResetTransferSnapshot();
            snapshot_.state = "ready";
            snapshot_.lastErrorCode = superseded
                ? std::string{} : envelope.message.errorCode;
            snapshot_.lastErrorMessage = superseded
                ? std::string{} : envelope.message.errorMessage;
            ProcessDeferredLocalClipboard();
            PublishSnapshot();
        }
        return;
        }
    case ClipboardMessageType::kChunk:
        ProcessChunk(envelope);
        return;
    case ClipboardMessageType::kCommit:
        ProcessCommit(envelope);
        return;
    case ClipboardMessageType::kApplyResult:
        if (outgoing_ && envelope.message.itemId == outgoing_->itemId) {
            snapshot_.lastBytes = 0;
            for (const auto& part : outgoing_->parts)
                snapshot_.lastBytes += part.descriptor.size;
            snapshot_.lastFormat = outgoing_->parts.empty()
                ? std::string{} : FormatName(outgoing_->parts.front().descriptor.format);
            if (envelope.message.accepted) {
                // Controller -> controlled remains the existing automatic
                // remote paste. The reverse direction already restored the
                // first local Ctrl+V target in ProcessCommit.
                const SessionCommandResult pasteResult =
                    !context_.localIsController
                    ? SessionCommandResult{true, {}, {}}
                    : paste_
                        ? paste_()
                        : SessionCommandResult{false,
                              "remote_paste_input_unavailable",
                              "Remote paste input is unavailable."};
                if (pasteResult.accepted) {
                    ++snapshot_.sentItems;
                    snapshot_.state = "ready";
                    snapshot_.lastErrorCode.clear();
                    snapshot_.lastErrorMessage.clear();
                    if (!context_.localIsController &&
                        fulfillingAnnouncedRequest_) {
                        announcedLocalItemId_.clear();
                        announcedLocalVersion_ = 0;
                    }
                } else {
                    ++snapshot_.rejectedItems;
                    snapshot_.state = "ready";
                    snapshot_.lastErrorCode = pasteResult.errorCode;
                    snapshot_.lastErrorMessage = pasteResult.errorMessage;
                }
            } else {
                ++snapshot_.rejectedItems;
                snapshot_.lastErrorCode = envelope.message.errorCode;
                snapshot_.lastErrorMessage = envelope.message.errorMessage;
            }
            outgoing_.reset();
            fulfillingAnnouncedRequest_ = false;
            transferWatchdogDue_.reset();
            pasteRequestPending_.store(false);
            ResetTransferSnapshot();
            ProcessDeferredLocalClipboard();
            PublishSnapshot();
        }
        return;
    case ClipboardMessageType::kProgress:
        if (outgoing_ && envelope.message.itemId == outgoing_->itemId &&
            envelope.message.lamportVersion == outgoing_->version) {
            if (snapshot_.state == "offering" &&
                envelope.message.errorCode ==
                    "clipboard_waiting_for_conflict_decision") {
                // The receiver is waiting for a local overwrite decision.
                // Keep the Offer alive without starting any file chunks.
                outgoing_->retryCount = 0;
                ScheduleTransferWatchdog();
                return;
            }
            std::uint64_t totalBytes = 0;
            for (const auto& part : outgoing_->parts) {
                totalBytes += part.descriptor.size;
            }
            if (envelope.message.totalSize == totalBytes) {
                outgoing_->deliveredBytes = (std::min)(
                    totalBytes,
                    (std::max)(outgoing_->deliveredBytes,
                               envelope.message.offset));
                UpdateOutgoingProgressSnapshot();
                PublishSnapshot();
            }
        }
        return;
    case ClipboardMessageType::kClear:
        return;
    case ClipboardMessageType::kResync:
        return;
    }
}

void ClipboardController::ProcessOffer(const ClipboardEnvelope& envelope)
{
    ClipboardMessage response;
    response.itemId = envelope.message.itemId;
    response.lamportVersion = envelope.message.lamportVersion;
    const bool answersLocalRequest =
        localPasteHotkeyPending_.load(std::memory_order_acquire) &&
        envelope.message.itemId == remoteAnnouncedItemId_ &&
        envelope.message.lamportVersion == remoteAnnouncedVersion_;
    const auto failLocalRequest = [this, &response, answersLocalRequest] {
        if (!answersLocalRequest) return;
        remoteRequestWatchdogDue_.reset();
        localPasteHotkeyPending_.store(false, std::memory_order_release);
        pendingLocalPasteTarget_ = {};
        ClearRemoteAnnouncement(true);
        ResetTransferSnapshot();
        snapshot_.state = context_.active ? "ready" : "waiting_for_control";
        snapshot_.lastErrorCode = response.errorCode;
        snapshot_.lastErrorMessage = response.errorMessage;
        ++snapshot_.rejectedItems;
        PublishSnapshot();
    };
    if (!configuration_.enabled ||
        !snapshot_.peerEnabled ||
        (!peerSessionId_.empty() &&
         envelope.clipboardSessionId != peerSessionId_)) {
        response.type = ClipboardMessageType::kReject;
        response.errorCode = "clipboard_not_authorized";
        response.errorMessage = "Clipboard receiving is disabled for this session.";
        (void)SendProtocolMessage(response);
        failLocalRequest();
        return;
    }
    const bool older = envelope.message.lamportVersion < lastAppliedVersion_ ||
        (envelope.message.lamportVersion == lastAppliedVersion_ &&
         envelope.senderDeviceId <= lastAppliedOrigin_);
    if (older) {
        response.type = ClipboardMessageType::kReject;
        response.errorCode = "clipboard_item_stale";
        response.errorMessage = "A newer clipboard item is already active.";
        (void)SendProtocolMessage(response);
        failLocalRequest();
        return;
    }
    if (incoming_ && envelope.message.itemId == incoming_->itemId &&
        envelope.message.lamportVersion == incoming_->version &&
        envelope.senderDeviceId == incoming_->originDeviceId &&
        envelope.message.descriptors.size() == incoming_->parts.size() &&
        std::equal(envelope.message.descriptors.begin(),
                   envelope.message.descriptors.end(),
                   incoming_->parts.begin(),
                   [](const ClipboardFormatDescriptor& descriptor,
                      const TransferPart& part) {
                       return SameDescriptor(descriptor, part.descriptor);
                   })) {
        response.type = ClipboardMessageType::kAccept;
        response.accepted = true;
        (void)SendProtocolMessage(response);
        snapshot_.state = "receiving";
        PublishSnapshot();
        return;
    }
    if (outgoing_) {
        ClipboardMessage cancel;
        cancel.type = ClipboardMessageType::kCancel;
        cancel.itemId = outgoing_->itemId;
        cancel.lamportVersion = outgoing_->version;
        cancel.errorCode = "clipboard_superseded";
        cancel.errorMessage =
            "A newer clipboard item replaced the transfer.";
        (void)SendProtocolMessage(cancel);
        pasteRequestPending_.store(false);
    }
    if (incoming_) {
        CleanupIncomingTransferCache();
    }
    outgoing_.reset();
    incoming_.reset();
    pendingExplorerConflictOffer_.reset();
    transferWatchdogDue_.reset();
    preparationWatchdogDue_.reset();
    ResetTransferSnapshot();
    if (answersLocalRequest &&
        !pendingLocalPasteTarget_.replaceExplorerConflicts &&
        !pendingLocalPasteTarget_.explorerDestinationDirectory.empty()) {
        std::vector<std::string> conflicts;
        for (const auto& descriptor : envelope.message.descriptors) {
            if (!descriptor.topLevel ||
                (descriptor.format != ClipboardFormat::kFile &&
                 descriptor.format != ClipboardFormat::kDirectory)) {
                continue;
            }
            std::filesystem::path relative;
            if (!SafeRelativePath(descriptor.name, &relative)) continue;
            std::error_code existsError;
            const auto destination =
                pendingLocalPasteTarget_.explorerDestinationDirectory /
                relative.filename();
            if (std::filesystem::exists(destination, existsError) &&
                !existsError) {
                conflicts.push_back(descriptor.name);
            }
        }
        if (!conflicts.empty()) {
            pendingExplorerConflictOffer_ = envelope;
            explorerConflictKeepaliveDue_ =
                std::chrono::steady_clock::now();
            workAvailable_.notify_all();
            remoteRequestWatchdogDue_.reset();
            snapshot_.state = "confirming_conflict";
            snapshot_.transferActive = true;
            snapshot_.transferOutgoing = false;
            snapshot_.transferContainsFiles = true;
            snapshot_.transferId = envelope.message.itemId;
            snapshot_.transferItemCount = 0;
            snapshot_.transferTotalBytes = 0;
            for (const auto& descriptor : envelope.message.descriptors) {
                snapshot_.transferTotalBytes += descriptor.size;
                if (descriptor.topLevel &&
                    (descriptor.format == ClipboardFormat::kFile ||
                     descriptor.format == ClipboardFormat::kDirectory)) {
                    ++snapshot_.transferItemCount;
                }
            }
            snapshot_.localPasteTargetWindow =
                pendingLocalPasteTarget_.foregroundWindow;
            snapshot_.localPasteDestinationPath = WideToUtf8(
                pendingLocalPasteTarget_.explorerDestinationDirectory
                    .wstring());
            snapshot_.explorerConflictPending = true;
            snapshot_.explorerConflictNames = std::move(conflicts);
            PublishSnapshot();
            return;
        }
    }
    std::string error;
    if (!PrepareIncomingTransfer(envelope, &error)) {
        if (cacheManager_) {
            cacheManager_->RemoveEntry(
                ClipboardCacheRoot() /
                Utf8ToWide(envelope.message.itemId));
            UpdateCacheSnapshot();
        }
        response.type = ClipboardMessageType::kReject;
        response.errorCode = "clipboard_offer_rejected";
        response.errorMessage = error;
        (void)SendProtocolMessage(response);
        if (answersLocalRequest) {
            failLocalRequest();
        } else {
            ++snapshot_.rejectedItems;
            PublishSnapshot();
        }
        return;
    }
    response.type = ClipboardMessageType::kAccept;
    response.accepted = true;
    (void)SendProtocolMessage(response);
    if (localPasteHotkeyPending_.load(std::memory_order_acquire) &&
        envelope.message.itemId == remoteAnnouncedItemId_ &&
        envelope.message.lamportVersion == remoteAnnouncedVersion_) {
        remoteRequestWatchdogDue_.reset();
    }
    UpdateIncomingProgressSnapshot();
    snapshot_.state = "receiving";
    PublishSnapshot();
}

void ClipboardController::ProcessChunk(const ClipboardEnvelope& envelope)
{
    if (!incoming_ || envelope.message.itemId != incoming_->itemId ||
        envelope.message.lamportVersion != incoming_->version ||
        envelope.message.descriptorIndex >= incoming_->parts.size()) return;
    const auto index = envelope.message.descriptorIndex;
    auto& part = incoming_->parts[index];
    if (envelope.message.totalSize != part.descriptor.size ||
        envelope.message.data.size() >
            part.descriptor.size - (std::min)(
                part.descriptor.size, envelope.message.offset) ||
        envelope.message.offset > incoming_->received[index]) {
        CancelTransfers("clipboard_chunk_invalid",
                        "The clipboard chunk range is invalid.", true);
        return;
    }
    if (envelope.message.offset < incoming_->received[index]) {
        if (envelope.message.offset + envelope.message.data.size() <=
            incoming_->received[index]) {
            // A reliable ordered DataChannel must not duplicate application
            // messages, but ignoring a fully committed duplicate is safer
            // than corrupting the temporary file during recovery teardown.
            return;
        }
        CancelTransfers("clipboard_chunk_overlap",
                        "The clipboard chunk overlaps received data.", true);
        return;
    }
    bool wrote = true;
    if (part.descriptor.format == ClipboardFormat::kFile) {
        if (!part.receiveStream) {
            part.receiveStream = std::make_unique<std::ofstream>(
                part.temporaryPath, std::ios::binary | std::ios::app);
        }
        auto& output = *part.receiveStream;
        if (!output) wrote = false;
        if (wrote) {
            output.write(reinterpret_cast<const char*>(
                envelope.message.data.data()),
                static_cast<std::streamsize>(envelope.message.data.size()));
            wrote = output.good();
        }
    } else {
        part.data.insert(part.data.end(), envelope.message.data.begin(),
                         envelope.message.data.end());
    }
    if (!wrote) {
        CancelTransfers("clipboard_cache_write_failed",
                        "The clipboard cache could not be written.", true);
        return;
    }
    if (part.receiveHasher &&
        !part.receiveHasher->Update(envelope.message.data)) {
        CancelTransfers("clipboard_hash_update_failed",
                        "Clipboard content verification could not continue.",
                        true);
        return;
    }
    incoming_->received[index] += envelope.message.data.size();
    UpdateIncomingProgressSnapshot();
    const auto now = std::chrono::steady_clock::now();
    const bool reportProgress =
        snapshot_.transferCompletedBytes == snapshot_.transferTotalBytes ||
        snapshot_.transferCompletedBytes >=
            incoming_->progressReportedBytes + kClipboardProgressReportBytes ||
        now - incoming_->progressReportedAt >=
            kClipboardProgressReportInterval;
    if (reportProgress) {
        ClipboardMessage progress;
        progress.type = ClipboardMessageType::kProgress;
        progress.itemId = incoming_->itemId;
        progress.lamportVersion = incoming_->version;
        progress.offset = snapshot_.transferCompletedBytes;
        progress.totalSize = snapshot_.transferTotalBytes;
        if (SendProtocolMessage(progress)) {
            incoming_->progressReportedBytes =
                snapshot_.transferCompletedBytes;
            incoming_->progressReportedAt = now;
        }
    }
    const bool publishProgress =
        incoming_->received[index] == part.descriptor.size ||
        now - incoming_->progressPublishedAt >=
            kClipboardProgressPublishInterval;
    if (publishProgress) {
        incoming_->progressPublishedAt = now;
        PublishSnapshot();
    }
}

void ClipboardController::ProcessCommit(const ClipboardEnvelope& envelope)
{
    if (!incoming_ || envelope.message.itemId != incoming_->itemId ||
        envelope.message.lamportVersion != incoming_->version) {
        // Commit may be retried when the final ApplyResult was accepted by
        // WebRTC but never observed by the sender. Re-emit the cached result
        // instead of leaving the sender locked in the applying state.
        if (envelope.message.itemId == lastCompletedIncomingItemId_ &&
            envelope.message.lamportVersion ==
                lastCompletedIncomingVersion_) {
            ClipboardMessage cached;
            cached.type = ClipboardMessageType::kApplyResult;
            cached.itemId = lastCompletedIncomingItemId_;
            cached.lamportVersion = lastCompletedIncomingVersion_;
            cached.accepted = lastCompletedIncomingAccepted_;
            cached.errorCode = lastCompletedIncomingErrorCode_;
            cached.errorMessage = lastCompletedIncomingErrorMessage_;
            (void)SendProtocolMessage(cached);
        }
        return;
    }
    std::string error;
    bool commitMetadataValid =
        envelope.message.descriptors.size() == incoming_->parts.size();
    if (commitMetadataValid) {
        for (std::size_t index = 0;
             index < incoming_->parts.size(); ++index) {
            const auto& committed = envelope.message.descriptors[index];
            const auto& offered = incoming_->parts[index].descriptor;
            if (!SameDescriptorMetadata(committed, offered) ||
                (committed.format == ClipboardFormat::kDirectory
                    ? committed.hasSha256
                    : !committed.hasSha256)) {
                commitMetadataValid = false;
                break;
            }
        }
    }
    if (!commitMetadataValid) {
        error =
            "The clipboard commit metadata does not match its offer.";
    } else {
        for (std::size_t index = 0;
             index < incoming_->parts.size(); ++index) {
            auto& offered = incoming_->parts[index].descriptor;
            const auto& committed = envelope.message.descriptors[index];
            offered.sha256 = committed.sha256;
            offered.hasSha256 = committed.hasSha256;
        }
    }
    const bool applied =
        commitMetadataValid && FinalizeIncoming(&error);
    bool localPasteDelivered = true;
    std::string localPasteError;
    if (applied && context_.localIsController &&
        localPasteHotkeyPending_.load(std::memory_order_acquire)) {
        if (pendingLocalPasteTarget_.injectPasteAfterMaterialization) {
            if (!pendingLocalPasteTarget_.explorerDestinationDirectory
                    .empty()) {
                localPasteDelivered =
                    CopyIncomingFilesToExplorerTarget(
                        pendingLocalPasteTarget_
                            .explorerDestinationDirectory,
                        &localPasteError);
            } else {
                localPasteDelivered = PasteIntoLocalTarget(
                    pendingLocalPasteTarget_, &localPasteError);
            }
        }
        remoteClipboardAvailable_.store(false, std::memory_order_release);
        remoteAnnouncementLocalSequence_.store(0,
            std::memory_order_release);
        remoteAnnouncedItemId_.clear();
        remoteAnnouncedVersion_ = 0;
        remoteAnnouncedCapabilities_ = 0;
        remoteAnnouncedDescriptors_.clear();
        pendingRemoteConflictDecision_ = false;
    }
    remoteRequestWatchdogDue_.reset();
    localPasteHotkeyPending_.store(false, std::memory_order_release);
    pendingLocalPasteTarget_ = {};
    ClipboardMessage result;
    result.type = ClipboardMessageType::kApplyResult;
    result.itemId = envelope.message.itemId;
    result.lamportVersion = envelope.message.lamportVersion;
    result.accepted = applied;
    if (!applied) {
        result.errorCode = "clipboard_apply_failed";
        result.errorMessage = error;
        ++snapshot_.rejectedItems;
        CleanupIncomingTransferCache();
    } else if (!localPasteDelivered) {
        snapshot_.lastErrorCode = "clipboard_local_paste_failed";
        snapshot_.lastErrorMessage = localPasteError;
        ++snapshot_.rejectedItems;
    }
    lastCompletedIncomingItemId_ = envelope.message.itemId;
    lastCompletedIncomingVersion_ = envelope.message.lamportVersion;
    lastCompletedIncomingAccepted_ = applied;
    lastCompletedIncomingErrorCode_ = result.errorCode;
    lastCompletedIncomingErrorMessage_ = result.errorMessage;
    (void)SendProtocolMessage(result);
    incoming_.reset();
    ResetTransferSnapshot();
    snapshot_.state = "ready";
    ProcessDeferredLocalClipboard();
    PublishSnapshot();
}

void ClipboardController::SendNextChunk()
{
    if (!outgoing_) return;
    std::uint64_t burstBytes = 0;
    while (outgoing_->partIndex < outgoing_->parts.size()) {
        auto& part = outgoing_->parts[outgoing_->partIndex];
        if (outgoing_->offset >= part.descriptor.size) {
            std::string finalizeError;
            if (!FinalizeOutgoingPart(&part, &finalizeError)) {
                CancelTransfers(
                    "clipboard_source_changed", finalizeError, true);
                snapshot_.state = context_.active
                    ? "ready" : "waiting_for_control";
                snapshot_.lastErrorCode = "clipboard_source_changed";
                snapshot_.lastErrorMessage = std::move(finalizeError);
                ++snapshot_.rejectedItems;
                PublishSnapshot();
                return;
            }
            ++outgoing_->partIndex;
            outgoing_->offset = 0;
            continue;
        }
        const std::size_t count = static_cast<std::size_t>((std::min)(
            static_cast<std::uint64_t>(kChunkBytes),
            part.descriptor.size - outgoing_->offset));
        std::vector<std::uint8_t> bytes;
        if (!part.sourcePath.empty()) {
            if (!part.sourceStream) {
                part.sourceStream = std::make_unique<std::ifstream>(
                    part.sourcePath, std::ios::binary);
            }
            auto& input = *part.sourceStream;
            if (!input) {
                CancelTransfers("clipboard_source_read_failed",
                                "A copied file could not be reopened.", true);
                return;
            }
            input.clear();
            input.seekg(static_cast<std::streamoff>(
                outgoing_->offset));
            bytes.resize(count);
            input.read(reinterpret_cast<char*>(bytes.data()), count);
            bytes.resize(static_cast<std::size_t>(input.gcount()));
        } else {
            bytes.assign(part.data.begin() +
                    static_cast<std::ptrdiff_t>(
                        outgoing_->offset),
                part.data.begin() + static_cast<std::ptrdiff_t>(
                    outgoing_->offset + count));
        }
        if (bytes.empty()) {
            CancelTransfers("clipboard_source_read_failed",
                            "Clipboard data could not be read.", true);
            return;
        }
        ClipboardMessage chunk;
        chunk.type = ClipboardMessageType::kChunk;
        chunk.itemId = outgoing_->itemId;
        chunk.lamportVersion = outgoing_->version;
        chunk.descriptorIndex =
            static_cast<std::uint32_t>(outgoing_->partIndex);
        chunk.offset = outgoing_->offset;
        chunk.totalSize = part.descriptor.size;
        chunk.data = std::move(bytes);
        if (!SendProtocolMessage(chunk)) {
            if (lastSendBackpressured_) {
                ScheduleBackpressureRetry();
            } else {
                ScheduleTransferWatchdog();
            }
            return;
        }
        outgoing_->backpressurePending = false;
        const auto acceptedBytes = chunk.data.size();
        if (part.sendHasher && !part.sendHasher->Update(chunk.data)) {
            CancelTransfers(
                "clipboard_hash_update_failed",
                "Clipboard content verification could not continue.",
                true);
            snapshot_.state = context_.active
                ? "ready" : "waiting_for_control";
            snapshot_.lastErrorCode = "clipboard_hash_update_failed";
            snapshot_.lastErrorMessage =
                "Clipboard content verification could not continue.";
            ++snapshot_.rejectedItems;
            PublishSnapshot();
            return;
        }
        outgoing_->offset += acceptedBytes;
        burstBytes += acceptedBytes;
        outgoing_->retryCount = 0;
        snapshot_.state = "sending";
        const auto now = std::chrono::steady_clock::now();
        const bool publishProgress =
            outgoing_->offset == part.descriptor.size ||
            now - outgoing_->progressPublishedAt >=
                kClipboardProgressPublishInterval;
        if (publishProgress) {
            outgoing_->progressPublishedAt = now;
            UpdateOutgoingProgressSnapshot();
            PublishSnapshot();
        }
        if (burstBytes >= kClipboardSendBurstBytes) {
            ScheduleTransferWatchdog();
            const auto itemId = outgoing_->itemId;
            Post([this, itemId] {
                if (outgoing_ && outgoing_->itemId == itemId &&
                    snapshot_.state == "sending") {
                    SendNextChunk();
                }
            });
            return;
        }
    }
    outgoing_->retryCount = 0;
    UpdateOutgoingProgressSnapshot();
    PublishSnapshot();
    (void)SendOutgoingCommit();
}

void ClipboardController::CancelTransfers(
    const std::string& reasonCode, const std::string& reasonMessage,
    bool notifyPeer)
{
    if (service_) {
        service_->CancelPendingRemoteRender();
    }
    if (notifyPeer) {
        const auto sendCancel = [this, &reasonCode, &reasonMessage](
                const std::string& itemId, std::uint64_t version) {
            ClipboardMessage cancel;
            cancel.type = ClipboardMessageType::kCancel;
            cancel.itemId = itemId;
            cancel.lamportVersion = version;
            cancel.errorCode = reasonCode;
            cancel.errorMessage = reasonMessage;
            (void)SendProtocolMessage(cancel);
        };
        if (outgoing_) sendCancel(outgoing_->itemId, outgoing_->version);
        if (incoming_) sendCancel(incoming_->itemId, incoming_->version);
        if (!outgoing_ && !incoming_ &&
            localPasteHotkeyPending_.load(std::memory_order_acquire) &&
            !remoteAnnouncedItemId_.empty() &&
            remoteAnnouncedVersion_ != 0) {
            sendCancel(remoteAnnouncedItemId_, remoteAnnouncedVersion_);
        }
    }
    if (incoming_) {
        CleanupIncomingTransferCache();
    }
    outgoing_.reset();
    incoming_.reset();
    pendingExplorerConflictOffer_.reset();
    pendingRemoteConflictDecision_ = false;
    explorerConflictKeepaliveDue_.reset();
    transferWatchdogDue_.reset();
    preparationWatchdogDue_.reset();
    remoteRequestWatchdogDue_.reset();
    pasteRequestPending_.store(false);
    localPasteHotkeyPending_.store(false, std::memory_order_release);
    pendingLocalPasteTarget_ = {};
    fulfillingAnnouncedRequest_ = false;
    ResetTransferSnapshot();
    ProcessDeferredLocalClipboard();
}

void ClipboardController::CleanupIncomingTransferCache()
{
    if (!incoming_) return;
    for (auto& part : incoming_->parts) {
        part.receiveStream.reset();
    }
    if (cacheManager_) {
        cacheManager_->RequestRemoveEntry(incoming_->cacheDirectory);
    } else {
        std::error_code ignored;
        std::filesystem::remove_all(incoming_->cacheDirectory, ignored);
    }
}

void ClipboardController::ClearRemoteAnnouncement(
    bool clearWindowsAdvertisement)
{
    remoteClipboardAvailable_.store(false, std::memory_order_release);
    remoteAnnouncedItemId_.clear();
    remoteAnnouncedVersion_ = 0;
    remoteAnnouncedCapabilities_ = 0;
    remoteAnnouncedDescriptors_.clear();
    pendingRemoteConflictDecision_ = false;
    remoteAnnouncementLocalSequence_.store(0,
        std::memory_order_release);
    lastRemoteClipboardSequence_.store(0,
        std::memory_order_release);
    if (clearWindowsAdvertisement && service_) {
        service_->ClearRemoteClipboardAdvertisement();
    }
}

void ClipboardController::ResetTransferSnapshot()
{
    snapshot_.transferActive = false;
    snapshot_.transferOutgoing = false;
    snapshot_.transferContainsFiles = false;
    snapshot_.transferId.clear();
    snapshot_.transferItemCount = 0;
    snapshot_.transferTotalBytes = 0;
    snapshot_.transferCompletedBytes = 0;
    snapshot_.transferBytesPerSecond = 0.0;
    snapshot_.transferEstimatedRemainingSeconds = 0;
    snapshot_.localPasteTargetWindow = 0;
    snapshot_.localPasteDestinationPath.clear();
    snapshot_.explorerConflictPending = false;
    snapshot_.explorerConflictNames.clear();
}

void ClipboardController::UpdateOutgoingProgressSnapshot()
{
    if (!outgoing_) {
        ResetTransferSnapshot();
        return;
    }
    snapshot_.transferActive = true;
    snapshot_.transferOutgoing = true;
    snapshot_.transferId = outgoing_->itemId;
    snapshot_.transferItemCount = 0;
    snapshot_.transferTotalBytes = 0;
    snapshot_.transferCompletedBytes = 0;
    snapshot_.transferContainsFiles = false;
    snapshot_.localPasteTargetWindow = 0;
    for (std::size_t index = 0; index < outgoing_->parts.size(); ++index) {
        const auto& part = outgoing_->parts[index];
        snapshot_.transferTotalBytes += part.descriptor.size;
        snapshot_.transferContainsFiles =
            snapshot_.transferContainsFiles ||
            part.descriptor.format == ClipboardFormat::kFile ||
            part.descriptor.format == ClipboardFormat::kDirectory;
        if (part.descriptor.topLevel &&
            snapshot_.transferItemCount <
                std::numeric_limits<std::uint32_t>::max()) {
            ++snapshot_.transferItemCount;
        }
    }
    snapshot_.transferCompletedBytes = (std::min)(
        outgoing_->deliveredBytes, snapshot_.transferTotalBytes);
    const auto now = std::chrono::steady_clock::now();
    if (outgoing_->rateSamples.empty()) {
        outgoing_->rateSamples.push_back(OutgoingTransfer::RateSample{
            now, snapshot_.transferCompletedBytes});
        outgoing_->ratePublishedAt = now;
        snapshot_.transferBytesPerSecond = 0.0;
    } else {
        if (outgoing_->rateSamples.back().bytes !=
            snapshot_.transferCompletedBytes) {
            outgoing_->rateSamples.push_back(OutgoingTransfer::RateSample{
                now, snapshot_.transferCompletedBytes});
        }
        const auto cutoff = now - kClipboardRateWindow;
        while (outgoing_->rateSamples.size() > 1 &&
               outgoing_->rateSamples[1].at <= cutoff) {
            outgoing_->rateSamples.pop_front();
        }
    }
    if (now - outgoing_->ratePublishedAt >=
            kClipboardRateRefreshInterval &&
        !outgoing_->rateSamples.empty()) {
        outgoing_->ratePublishedAt = now;
        const auto& baseline = outgoing_->rateSamples.front();
        const auto sampleDuration = now - baseline.at;
        const double seconds =
            std::chrono::duration<double>(sampleDuration).count();
        snapshot_.transferBytesPerSecond =
            seconds > 0.0 &&
                snapshot_.transferCompletedBytes >= baseline.bytes
            ? static_cast<double>(snapshot_.transferCompletedBytes -
                  baseline.bytes) / seconds
            : 0.0;
    }
    if (snapshot_.transferBytesPerSecond > 1.0 &&
        snapshot_.transferCompletedBytes < snapshot_.transferTotalBytes) {
        const double remainingSeconds =
            static_cast<double>(snapshot_.transferTotalBytes -
                snapshot_.transferCompletedBytes) /
            snapshot_.transferBytesPerSecond;
        snapshot_.transferEstimatedRemainingSeconds =
            static_cast<std::uint64_t>(remainingSeconds + 0.999);
    } else {
        snapshot_.transferEstimatedRemainingSeconds = 0;
    }
}

void ClipboardController::UpdateIncomingProgressSnapshot()
{
    if (!incoming_) {
        ResetTransferSnapshot();
        return;
    }
    snapshot_.transferActive = true;
    snapshot_.transferOutgoing = false;
    snapshot_.transferId = incoming_->itemId;
    snapshot_.transferItemCount = 0;
    snapshot_.transferTotalBytes = 0;
    snapshot_.transferCompletedBytes = 0;
    snapshot_.transferContainsFiles = false;
    snapshot_.localPasteTargetWindow =
        context_.localIsController &&
            localPasteHotkeyPending_.load(std::memory_order_acquire)
        ? pendingLocalPasteTarget_.foregroundWindow
        : 0;
    snapshot_.localPasteDestinationPath =
        context_.localIsController &&
            localPasteHotkeyPending_.load(std::memory_order_acquire) &&
            !pendingLocalPasteTarget_.explorerDestinationDirectory.empty()
        ? WideToUtf8(
              pendingLocalPasteTarget_.explorerDestinationDirectory.wstring())
        : std::string{};
    for (std::size_t index = 0; index < incoming_->parts.size(); ++index) {
        const auto& part = incoming_->parts[index];
        snapshot_.transferTotalBytes += part.descriptor.size;
        snapshot_.transferCompletedBytes += incoming_->received[index];
        snapshot_.transferContainsFiles =
            snapshot_.transferContainsFiles ||
            part.descriptor.format == ClipboardFormat::kFile ||
            part.descriptor.format == ClipboardFormat::kDirectory;
        if (part.descriptor.topLevel &&
            snapshot_.transferItemCount <
                std::numeric_limits<std::uint32_t>::max()) {
            ++snapshot_.transferItemCount;
        }
    }
    const auto now = std::chrono::steady_clock::now();
    if (incoming_->rateSamples.empty()) {
        incoming_->rateSamples.push_back(IncomingTransfer::RateSample{
            now, snapshot_.transferCompletedBytes});
        incoming_->ratePublishedAt = now;
        snapshot_.transferBytesPerSecond = 0.0;
    } else {
        if (incoming_->rateSamples.back().bytes !=
            snapshot_.transferCompletedBytes) {
            incoming_->rateSamples.push_back(IncomingTransfer::RateSample{
                now, snapshot_.transferCompletedBytes});
        }
        const auto cutoff = now - kClipboardRateWindow;
        while (incoming_->rateSamples.size() > 1 &&
               incoming_->rateSamples[1].at <= cutoff) {
            incoming_->rateSamples.pop_front();
        }
    }
    if (now - incoming_->ratePublishedAt >=
            kClipboardRateRefreshInterval &&
        !incoming_->rateSamples.empty()) {
        incoming_->ratePublishedAt = now;
        const auto& baseline = incoming_->rateSamples.front();
        const double seconds =
            std::chrono::duration<double>(now - baseline.at).count();
        snapshot_.transferBytesPerSecond =
            seconds > 0.0 &&
                snapshot_.transferCompletedBytes >= baseline.bytes
            ? static_cast<double>(snapshot_.transferCompletedBytes -
                  baseline.bytes) / seconds
            : 0.0;
    }
    if (snapshot_.transferBytesPerSecond > 1.0 &&
        snapshot_.transferCompletedBytes < snapshot_.transferTotalBytes) {
        const double remainingSeconds =
            static_cast<double>(snapshot_.transferTotalBytes -
                snapshot_.transferCompletedBytes) /
            snapshot_.transferBytesPerSecond;
        snapshot_.transferEstimatedRemainingSeconds =
            static_cast<std::uint64_t>(remainingSeconds + 0.999);
    } else {
        snapshot_.transferEstimatedRemainingSeconds = 0;
    }
}

void ClipboardController::UpdateClipboardListenerState()
{
    if (!service_) return;
    service_->SetEnabled(configuration_.enabled && context_.active &&
        context_.transportReady);
}

bool ClipboardController::SendProtocolMessage(
    const ClipboardMessage& message)
{
    lastSendBackpressured_ = false;
    if (!send_ || !context_.active || !context_.transportReady ||
        localSessionId_.empty()) return false;
    SessionCommandResult result;
    const int attempts = IsClipboardTransferMessage(message.type)
        ? kClipboardBackpressureRetryCount : 1;
    for (int attempt = 0; attempt < attempts; ++attempt) {
        result = send_(context_.peerDeviceId, localSessionId_, message);
        if (result.accepted) {
            return true;
        }
        const bool backpressured =
            result.errorCode == "room_clipboard_backpressure" ||
            result.errorCode == "direct_clipboard_backpressure";
        if (!backpressured) {
            break;
        }
        if (attempt + 1 < attempts) {
            std::this_thread::sleep_for(kClipboardBackpressureRetryDelay);
        }
    }
    if (result.errorCode == "room_clipboard_backpressure" ||
        result.errorCode == "direct_clipboard_backpressure") {
        // A full reliable DataChannel buffer is normal flow control, not a
        // transfer failure. Let the worker sleep until a short timed retry;
        // do not block it for seconds or consume the timeout retry budget.
        lastSendBackpressured_ = true;
        return false;
    }
    snapshot_.lastErrorCode = result.errorCode.empty()
        ? "clipboard_transport_failed" : result.errorCode;
    snapshot_.lastErrorMessage = result.errorMessage.empty()
        ? "The clipboard transport rejected the message."
        : result.errorMessage;
    PublishSnapshot();
    return false;
}

bool ClipboardController::BuildOutgoingTransfer(
    WindowsClipboardContent content, std::string* error)
{
    const auto sourceGeneration = content.localChangeGeneration;
    const auto superseded = [this, sourceGeneration] {
        return localClipboardGeneration_.load(std::memory_order_acquire) !=
            sourceGeneration;
    };
    auto transfer = std::make_unique<OutgoingTransfer>();
    if (fulfillingAnnouncedRequest_ && !announcedLocalItemId_.empty() &&
        announcedLocalVersion_ != 0) {
        transfer->itemId = announcedLocalItemId_;
        transfer->version = announcedLocalVersion_;
    } else {
        transfer->itemId = GenerateId("item");
        transfer->version = ++logicalClock_;
    }
    lastAppliedVersion_ = transfer->version;
    lastAppliedOrigin_ = context_.localDeviceId;
    std::uint64_t fileBytes = 0;
    const auto addFile = [&](const std::filesystem::path& source,
                             const std::filesystem::path& relative,
                             ClipboardFormat format,
                             bool topLevel,
                             auto&& addFileRef) -> bool {
        if (superseded()) return false;
        if (transfer->parts.size() >= kMaximumClipboardDescriptors) return false;
        TransferPart part;
        part.descriptor.format = format;
        part.descriptor.topLevel = topLevel;
        part.descriptor.name = WideToUtf8(relative.generic_wstring());
        part.sourcePath = source;
        if (format == ClipboardFormat::kFile) {
            std::error_code sizeError;
            std::error_code timeError;
            part.descriptor.size = std::filesystem::file_size(source, sizeError);
            part.sourceWriteTime =
                std::filesystem::last_write_time(source, timeError);
            if (sizeError ||
                timeError ||
                part.descriptor.size > configuration_.automaticFileLimitBytes ||
                fileBytes > configuration_.automaticFileLimitBytes -
                    part.descriptor.size) return false;
            part.sourceSize = part.descriptor.size;
            part.sourceMetadataCaptured = true;
            part.sendHasher = std::make_unique<IncrementalSha256>();
            if (!part.sendHasher->Valid()) return false;
            fileBytes += part.descriptor.size;
        }
        transfer->parts.push_back(std::move(part));
        if (format == ClipboardFormat::kDirectory) {
            std::error_code iteratorError;
            for (std::filesystem::directory_iterator iterator(source,
                     std::filesystem::directory_options::skip_permission_denied,
                     iteratorError), end;
                 !iteratorError && iterator != end; iterator.increment(iteratorError)) {
                if (superseded()) return false;
                const auto childRelative = relative / iterator->path().filename();
                std::error_code typeError;
                if (iterator->is_symlink(typeError)) {
                    continue;
                } else if (iterator->is_directory(typeError)) {
                    if (!addFileRef(iterator->path(), childRelative,
                            ClipboardFormat::kDirectory, false,
                            addFileRef)) return false;
                } else if (iterator->is_regular_file(typeError)) {
                    if (!addFileRef(iterator->path(), childRelative,
                            ClipboardFormat::kFile, false,
                            addFileRef)) return false;
                }
            }
            if (iteratorError) return false;
        }
        return true;
    };
    std::unordered_set<std::wstring> topLevelNames;
    for (auto& sourcePart : content.parts) {
        if (superseded()) {
            if (error) *error = "clipboard_superseded";
            return false;
        }
        if (!FormatEnabled(sourcePart.format, configuration_) ||
            (peerCapabilities_ & FormatCapability(sourcePart.format)) == 0) {
            continue;
        }
        if (sourcePart.format == ClipboardFormat::kFile ||
            sourcePart.format == ClipboardFormat::kDirectory) {
            if (sourcePart.localPath.empty()) {
                if (error) *error = "A copied file path is empty.";
                return false;
            }
            std::filesystem::path remoteName = sourcePart.localPath.filename();
            const auto originalStem = remoteName.stem().wstring();
            const auto originalExtension = remoteName.extension().wstring();
            for (std::size_t suffix = 2;; ++suffix) {
                std::wstring folded = remoteName.wstring();
                std::transform(folded.begin(), folded.end(), folded.begin(),
                    [](wchar_t character) { return std::towlower(character); });
                if (topLevelNames.insert(folded).second) break;
                remoteName = sourcePart.format == ClipboardFormat::kDirectory
                    ? std::filesystem::path(originalStem + L" (" +
                        std::to_wstring(suffix) + L")")
                    : std::filesystem::path(originalStem + L" (" +
                        std::to_wstring(suffix) + L")" + originalExtension);
            }
            if (!addFile(sourcePart.localPath, remoteName,
                    sourcePart.format, true, addFile)) {
                if (error) {
                    *error = superseded()
                        ? "clipboard_superseded"
                        : "Copied files exceed the configured limit or cannot be read.";
                }
                return false;
            }
            continue;
        }
        if (sourcePart.data.empty() ||
            sourcePart.data.size() > FormatLimit(sourcePart.format,
                                                  configuration_)) continue;
        TransferPart part;
        part.descriptor.format = sourcePart.format;
        part.descriptor.name = sourcePart.name;
        part.descriptor.size = sourcePart.data.size();
        part.data = std::move(sourcePart.data);
        part.sendHasher = std::make_unique<IncrementalSha256>();
        if (!part.sendHasher->Valid()) continue;
        transfer->parts.push_back(std::move(part));
    }
    if (transfer->parts.empty()) {
        if (error) *error = "The clipboard does not contain a supported format.";
        return false;
    }
    outgoing_ = std::move(transfer);
    return true;
}

bool ClipboardController::PrepareIncomingTransfer(
    const ClipboardEnvelope& envelope, std::string* error)
{
    auto transfer = std::make_unique<IncomingTransfer>();
    transfer->itemId = envelope.message.itemId;
    transfer->version = envelope.message.lamportVersion;
    transfer->originDeviceId = envelope.senderDeviceId;
    transfer->peerSessionId = envelope.clipboardSessionId;
    std::uint64_t incomingBytes = 0;
    std::uint64_t validatedFileBytes = 0;
    for (const auto& descriptor : envelope.message.descriptors) {
        if (!FormatEnabled(descriptor.format, configuration_) ||
            descriptor.size > FormatLimit(
                descriptor.format, configuration_) ||
            incomingBytes >
                (std::numeric_limits<std::uint64_t>::max)() -
                    descriptor.size) {
            if (error) {
                *error = "A clipboard format is disabled or exceeds its limit.";
            }
            return false;
        }
        incomingBytes += descriptor.size;
        if (descriptor.format == ClipboardFormat::kFile ||
            descriptor.format == ClipboardFormat::kDirectory) {
            if (descriptor.size > configuration_.automaticFileLimitBytes ||
                validatedFileBytes >
                    configuration_.automaticFileLimitBytes -
                        descriptor.size) {
                if (error) {
                    *error = "The copied files exceed the automatic transfer limit.";
                }
                return false;
            }
            validatedFileBytes += descriptor.size;
            std::filesystem::path relative;
            if (!SafeRelativePath(descriptor.name, &relative)) {
                if (error) *error = "A copied file path is unsafe.";
                return false;
            }
        }
    }
    if (!cacheManager_ || !cacheManager_->PrepareEntry(
            transfer->itemId, incomingBytes,
            &transfer->cacheDirectory, error)) {
        if (error && error->empty()) {
            *error = "The clipboard cache directory is unavailable.";
        }
        return false;
    }
    std::error_code filesystemError;
    std::uint64_t aggregateFiles = 0;
    for (const auto& descriptor : envelope.message.descriptors) {
        if (!FormatEnabled(descriptor.format, configuration_) ||
            descriptor.size > FormatLimit(descriptor.format, configuration_)) {
            if (error) *error = "A clipboard format is disabled or exceeds its limit.";
            return false;
        }
        TransferPart part;
        part.descriptor = descriptor;
        if (descriptor.format != ClipboardFormat::kDirectory) {
            part.receiveHasher = std::make_unique<IncrementalSha256>();
            if (!part.receiveHasher->Valid()) {
                if (error) {
                    *error = "Clipboard content verification could not start.";
                }
                return false;
            }
        }
        if (descriptor.format == ClipboardFormat::kFile ||
            descriptor.format == ClipboardFormat::kDirectory) {
            if (descriptor.size > configuration_.automaticFileLimitBytes ||
                aggregateFiles > configuration_.automaticFileLimitBytes -
                    descriptor.size) {
                if (error) *error = "The copied files exceed the automatic transfer limit.";
                return false;
            }
            aggregateFiles += descriptor.size;
            std::filesystem::path relative;
            if (!SafeRelativePath(descriptor.name, &relative)) {
                if (error) *error = "A copied file path is unsafe.";
                return false;
            }
            part.receivePath = transfer->cacheDirectory / relative;
            if (descriptor.format == ClipboardFormat::kDirectory) {
                std::filesystem::create_directories(part.receivePath,
                                                    filesystemError);
            } else {
                std::filesystem::create_directories(
                    part.receivePath.parent_path(), filesystemError);
                part.temporaryPath = part.receivePath;
                part.temporaryPath += L".part";
                std::filesystem::remove(part.temporaryPath, filesystemError);
                if (!filesystemError && descriptor.size == 0) {
                    std::ofstream empty(part.temporaryPath,
                                        std::ios::binary | std::ios::trunc);
                    if (!empty) {
                        filesystemError = std::make_error_code(
                            std::errc::io_error);
                    }
                }
            }
            if (filesystemError) {
                if (error) *error = "A copied file cache path could not be created.";
                return false;
            }
        } else {
            part.data.reserve(static_cast<std::size_t>(descriptor.size));
        }
        transfer->parts.push_back(std::move(part));
        transfer->received.push_back(0);
    }
    incoming_ = std::move(transfer);
    return true;
}

bool ClipboardController::FinalizeIncoming(std::string* error)
{
    if (!incoming_) return false;
    WindowsClipboardContent content;
    std::uint64_t bytes = 0;
    std::string firstFormat;
    for (std::size_t index = 0; index < incoming_->parts.size(); ++index) {
        auto& part = incoming_->parts[index];
        if (incoming_->received[index] != part.descriptor.size) {
            if (error) *error = "The clipboard transfer is incomplete.";
            return false;
        }
        if (part.receiveStream) {
            part.receiveStream->flush();
            const bool flushed = part.receiveStream->good();
            part.receiveStream.reset();
            if (!flushed) {
                if (error) *error =
                    "The clipboard cache could not be flushed.";
                return false;
            }
        }
        std::array<std::uint8_t, 32> digest{};
        if (part.descriptor.hasSha256) {
            const bool hashOk = part.receiveHasher &&
                part.receiveHasher->Finish(&digest);
            if (!hashOk || digest != part.descriptor.sha256) {
                if (error) *error = "Clipboard content verification failed.";
                return false;
            }
            part.receiveHasher.reset();
        }
        if (part.descriptor.format == ClipboardFormat::kUnicodeText) {
            const std::string utf8(part.data.begin(), part.data.end());
            if (!utf8.empty() && Utf8ToWide(utf8).empty()) {
                if (error) *error = "Clipboard text is not valid UTF-8.";
                return false;
            }
        }
        if (part.descriptor.format == ClipboardFormat::kFile) {
            std::error_code renameError;
            std::filesystem::rename(part.temporaryPath, part.receivePath,
                                    renameError);
            if (renameError) {
                if (error) *error = "A copied file could not be committed.";
                return false;
            }
        }
        const bool clipboardVisible =
            part.descriptor.format != ClipboardFormat::kFile &&
            part.descriptor.format != ClipboardFormat::kDirectory ||
            part.descriptor.topLevel;
        if (!clipboardVisible) {
            bytes += part.descriptor.size;
            continue;
        }
        WindowsClipboardPart output;
        output.format = part.descriptor.format;
        output.name = part.descriptor.name;
        output.data = std::move(part.data);
        output.localPath = part.receivePath;
        content.parts.push_back(std::move(output));
        bytes += part.descriptor.size;
        if (firstFormat.empty()) firstFormat = FormatName(part.descriptor.format);
    }
    lastAppliedMarker_ = "RemoteC|" + incoming_->originDeviceId + "|" +
        incoming_->peerSessionId + "|" + incoming_->itemId + "|" +
        std::to_string(incoming_->version);
    content.originMarker = lastAppliedMarker_;
    std::string writeError;
    if (!service_->Write(std::move(content), &writeError)) {
        if (error) *error = writeError;
        return false;
    }
    if (cacheManager_) {
        cacheManager_->MarkCompleted(incoming_->cacheDirectory);
        UpdateCacheSnapshot();
    }
    lastRemoteClipboardSequence_.store(
        GetClipboardSequenceNumber(), std::memory_order_release);
    lastAppliedVersion_ = incoming_->version;
    lastAppliedOrigin_ = incoming_->originDeviceId;
    logicalClock_ = (std::max)(logicalClock_, incoming_->version);
    snapshot_.lastBytes = bytes;
    snapshot_.lastFormat = std::move(firstFormat);
    ++snapshot_.receivedItems;
    snapshot_.lastErrorCode.clear();
    snapshot_.lastErrorMessage.clear();
    return true;
}

std::string ClipboardController::GenerateId(const char* prefix)
{
    const auto ticks = static_cast<std::uint64_t>(
        std::chrono::duration_cast<std::chrono::microseconds>(
            std::chrono::steady_clock::now().time_since_epoch()).count());
    return std::string(prefix) + "-" + std::to_string(GetCurrentProcessId()) +
        "-" + std::to_string(ticks) + "-" + std::to_string(++nextId_);
}

std::filesystem::path ClipboardController::ClipboardCacheRoot() const
{
    return cacheManager_ ? cacheManager_->Snapshot().root
                         : std::filesystem::path{};
}

void ClipboardController::CleanupClipboardCache()
{
    if (cacheManager_) cacheManager_->RequestCleanup(false);
}

void ClipboardController::UpdateCacheSnapshot()
{
    if (!cacheManager_) return;
    const auto cache = cacheManager_->Snapshot();
    snapshot_.cacheRootPath = WideToUtf8(cache.root.wstring());
    snapshot_.cacheBytes = cache.totalBytes;
    snapshot_.cacheEntryCount = cache.entryCount;
    snapshot_.cacheAvailableBytes = cache.availableBytes;
    snapshot_.cacheSafeCapacityBytes = cache.safeCapacityBytes;
    snapshot_.cacheEffectiveLimitBytes = cache.effectiveLimitBytes;
}

void ClipboardController::PublishSnapshot()
{
    IClipboardControllerObserver* observer = nullptr;
    ClipboardControllerSnapshot snapshot;
    {
        std::lock_guard lock(mutex_);
        publishedSnapshot_ = snapshot_;
        observer = observer_;
        snapshot = publishedSnapshot_;
    }
    if (observer) observer->OnClipboardStateChanged(snapshot);
}

}  // namespace remote::app
