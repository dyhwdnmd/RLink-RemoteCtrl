// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#ifndef UNICODE
#define UNICODE
#endif
#ifndef _UNICODE
#define _UNICODE
#endif
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <bcrypt.h>
#include <dwmapi.h>
#include <shellapi.h>
#include <winhttp.h>

#include <algorithm>
#include <array>
#include <atomic>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <mutex>
#include <optional>
#include <string>
#include <thread>
#include <vector>

#pragma comment(lib, "bcrypt.lib")
#pragma comment(lib, "dwmapi.lib")
#pragma comment(lib, "winhttp.lib")

namespace {

constexpr UINT kStateChangedMessage = WM_APP + 41;

struct Options {
    std::wstring packageUrl;
    std::wstring packageSha256;
    unsigned long long packageSize = 0;
    std::filesystem::path targetDirectory;
    std::wstring restartExecutable;
    std::wstring currentVersion;
    std::wstring targetVersion;
    DWORD parentProcessId = 0;
};

struct UiState {
    int progress = 0;
    std::wstring status = L"正在准备…";
    std::wstring detail = L"安装完成后，RLink 将重新启动。";
    bool finished = false;
    bool failed = false;
};

HWND g_window = nullptr;
std::mutex g_stateMutex;
UiState g_state;

std::wstring QuoteArgument(const std::wstring& value)
{
    if (value.find_first_of(L" \t\"") == std::wstring::npos) {
        return value;
    }
    std::wstring result = L"\"";
    unsigned int backslashes = 0;
    for (const wchar_t character : value) {
        if (character == L'\\') {
            ++backslashes;
            continue;
        }
        if (character == L'\"') {
            result.append(backslashes * 2 + 1, L'\\');
            result.push_back(L'\"');
            backslashes = 0;
            continue;
        }
        result.append(backslashes, L'\\');
        backslashes = 0;
        result.push_back(character);
    }
    result.append(backslashes * 2, L'\\');
    result.push_back(L'\"');
    return result;
}

void PublishState(int progress, std::wstring status,
                  std::wstring detail = {})
{
    {
        std::scoped_lock lock(g_stateMutex);
        g_state.progress = std::clamp(progress, 0, 100);
        g_state.status = std::move(status);
        if (!detail.empty()) {
            g_state.detail = std::move(detail);
        }
    }
    if (g_window) {
        PostMessageW(g_window, kStateChangedMessage, 0, 0);
    }
}

void PublishFailure(const std::wstring& detail)
{
    {
        std::scoped_lock lock(g_stateMutex);
        g_state.status = L"更新未能完成";
        g_state.detail = detail + L"\n更新器已停止，可以点击此窗口关闭。";
        g_state.finished = true;
        g_state.failed = true;
    }
    if (g_window) {
        PostMessageW(g_window, kStateChangedMessage, 0, 0);
    }
}

std::optional<Options> ParseOptions()
{
    int count = 0;
    wchar_t** arguments = CommandLineToArgvW(GetCommandLineW(), &count);
    if (!arguments) {
        return std::nullopt;
    }
    Options result;
    auto valueFor = [&](const std::wstring& name) -> std::wstring {
        for (int index = 1; index + 1 < count; ++index) {
            if (arguments[index] == name) {
                return arguments[index + 1];
            }
        }
        return {};
    };
    result.packageUrl = valueFor(L"--package-url");
    result.packageSha256 = valueFor(L"--package-sha256");
    result.targetDirectory = valueFor(L"--target-dir");
    result.restartExecutable = valueFor(L"--restart-exe");
    result.currentVersion = valueFor(L"--current-version");
    result.targetVersion = valueFor(L"--target-version");
    try {
        result.packageSize = std::stoull(valueFor(L"--package-size"));
        result.parentProcessId =
            static_cast<DWORD>(std::stoul(valueFor(L"--parent-pid")));
    } catch (...) {
        LocalFree(arguments);
        return std::nullopt;
    }
    LocalFree(arguments);
    if (result.packageUrl.empty() || result.packageSha256.size() != 64 ||
        result.packageSize == 0 || result.targetDirectory.empty() ||
        result.restartExecutable != L"RLinkAPP.exe" ||
        result.targetVersion.empty() || result.parentProcessId == 0) {
        return std::nullopt;
    }
    return result;
}

bool IsAllowedHttpsUrl(const std::wstring& url)
{
    URL_COMPONENTS parts{};
    parts.dwStructSize = sizeof(parts);
    std::array<wchar_t, 512> host{};
    parts.lpszHostName = host.data();
    parts.dwHostNameLength = static_cast<DWORD>(host.size());
    if (!WinHttpCrackUrl(url.c_str(), 0, 0, &parts) ||
        parts.nScheme != INTERNET_SCHEME_HTTPS) {
        return false;
    }
    std::wstring hostname(host.data(), parts.dwHostNameLength);
    std::transform(hostname.begin(), hostname.end(), hostname.begin(),
                   ::towlower);
    return hostname == L"github.com" ||
        hostname == L"objects.githubusercontent.com" ||
        hostname == L"release-assets.githubusercontent.com";
}

bool IsAllowedPackageUrl(const std::wstring& url)
{
    constexpr wchar_t prefix[] =
        L"https://github.com/dyhwdnmd/RLink-RemoteCtrl/releases/download/";
    return url.size() > std::size(prefix) - 1 &&
        _wcsnicmp(url.c_str(), prefix, std::size(prefix) - 1) == 0;
}

std::wstring LastErrorText(const wchar_t* operation)
{
    return std::wstring(operation) + L"（错误 " +
        std::to_wstring(GetLastError()) + L"）";
}

bool HashFileSha256(const std::filesystem::path& path,
                    std::wstring* hexDigest)
{
    BCRYPT_ALG_HANDLE algorithm = nullptr;
    BCRYPT_HASH_HANDLE hash = nullptr;
    DWORD objectLength = 0;
    DWORD digestLength = 0;
    DWORD copied = 0;
    std::vector<UCHAR> hashObject;
    std::vector<UCHAR> digest;
    bool ok = false;
    if (BCryptOpenAlgorithmProvider(&algorithm, BCRYPT_SHA256_ALGORITHM,
                                    nullptr, 0) < 0 ||
        BCryptGetProperty(algorithm, BCRYPT_OBJECT_LENGTH,
            reinterpret_cast<PUCHAR>(&objectLength), sizeof(objectLength),
            &copied, 0) < 0 ||
        BCryptGetProperty(algorithm, BCRYPT_HASH_LENGTH,
            reinterpret_cast<PUCHAR>(&digestLength), sizeof(digestLength),
            &copied, 0) < 0) {
        goto cleanup;
    }
    hashObject.resize(objectLength);
    digest.resize(digestLength);
    if (BCryptCreateHash(algorithm, &hash, hashObject.data(), objectLength,
                         nullptr, 0, 0) < 0) {
        goto cleanup;
    }
    {
        std::ifstream input(path, std::ios::binary);
        std::vector<char> buffer(1024 * 1024);
        while (input) {
            input.read(buffer.data(), buffer.size());
            const auto read = input.gcount();
            if (read > 0 && BCryptHashData(
                    hash, reinterpret_cast<PUCHAR>(buffer.data()),
                    static_cast<ULONG>(read), 0) < 0) {
                goto cleanup;
            }
        }
        if (!input.eof()) {
            goto cleanup;
        }
    }
    if (BCryptFinishHash(hash, digest.data(), digestLength, 0) < 0) {
        goto cleanup;
    }
    {
        constexpr wchar_t digits[] = L"0123456789abcdef";
        hexDigest->clear();
        hexDigest->reserve(digest.size() * 2);
        for (const UCHAR value : digest) {
            hexDigest->push_back(digits[value >> 4]);
            hexDigest->push_back(digits[value & 0x0f]);
        }
    }
    ok = true;

cleanup:
    if (hash) BCryptDestroyHash(hash);
    if (algorithm) BCryptCloseAlgorithmProvider(algorithm, 0);
    return ok;
}

bool DownloadPackage(const Options& options,
                     const std::filesystem::path& destination,
                     std::wstring* error)
{
    if (!IsAllowedPackageUrl(options.packageUrl)) {
        *error = L"安装包地址不受信任。";
        return false;
    }
    URL_COMPONENTS parts{};
    parts.dwStructSize = sizeof(parts);
    std::array<wchar_t, 512> host{};
    std::array<wchar_t, 4096> path{};
    parts.lpszHostName = host.data();
    parts.dwHostNameLength = static_cast<DWORD>(host.size());
    parts.lpszUrlPath = path.data();
    parts.dwUrlPathLength = static_cast<DWORD>(path.size());
    if (!WinHttpCrackUrl(options.packageUrl.c_str(), 0, 0, &parts)) {
        *error = L"无法解析安装包地址。";
        return false;
    }
    const std::wstring hostName(host.data(), parts.dwHostNameLength);
    const std::wstring requestPath(path.data(), parts.dwUrlPathLength);
    HINTERNET session = WinHttpOpen(
        L"RLinkUpdater/1.0", WINHTTP_ACCESS_TYPE_AUTOMATIC_PROXY,
        WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0);
    HINTERNET connection = nullptr;
    HINTERNET request = nullptr;
    HANDLE output = INVALID_HANDLE_VALUE;
    bool ok = false;
    if (!session) {
        *error = LastErrorText(L"无法初始化网络");
        goto cleanup;
    }
    WinHttpSetTimeouts(session, 15000, 15000, 30000, 30000);
    connection = WinHttpConnect(session, hostName.c_str(), parts.nPort, 0);
    if (!connection) {
        *error = LastErrorText(L"无法连接下载服务器");
        goto cleanup;
    }
    request = WinHttpOpenRequest(
        connection, L"GET", requestPath.c_str(), nullptr,
        WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES,
        WINHTTP_FLAG_SECURE | WINHTTP_FLAG_REFRESH);
    if (!request || !WinHttpSendRequest(
            request, WINHTTP_NO_ADDITIONAL_HEADERS, 0,
            WINHTTP_NO_REQUEST_DATA, 0, 0, 0) ||
        !WinHttpReceiveResponse(request, nullptr)) {
        *error = LastErrorText(L"下载安装包失败");
        goto cleanup;
    }
    {
        DWORD status = 0;
        DWORD size = sizeof(status);
        if (!WinHttpQueryHeaders(request,
                WINHTTP_QUERY_STATUS_CODE | WINHTTP_QUERY_FLAG_NUMBER,
                WINHTTP_HEADER_NAME_BY_INDEX, &status, &size,
                WINHTTP_NO_HEADER_INDEX) || status != 200) {
            *error = L"下载服务器返回异常状态。";
            goto cleanup;
        }
        DWORD finalUrlBytes = 0;
        WinHttpQueryOption(request, WINHTTP_OPTION_URL, nullptr,
                           &finalUrlBytes);
        std::vector<wchar_t> finalUrl(finalUrlBytes / sizeof(wchar_t) + 1);
        if (finalUrlBytes == 0 || !WinHttpQueryOption(
                request, WINHTTP_OPTION_URL, finalUrl.data(),
                &finalUrlBytes) || !IsAllowedHttpsUrl(finalUrl.data())) {
            *error = L"下载被重定向到不受信任的地址。";
            goto cleanup;
        }
    }
    output = CreateFileW(destination.c_str(), GENERIC_WRITE, 0, nullptr,
                         CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (output == INVALID_HANDLE_VALUE) {
        *error = LastErrorText(L"无法创建临时安装包");
        goto cleanup;
    }
    {
        unsigned long long received = 0;
        std::vector<BYTE> buffer(1024 * 1024);
        while (true) {
            DWORD available = 0;
            if (!WinHttpQueryDataAvailable(request, &available)) {
                *error = LastErrorText(L"读取安装包失败");
                goto cleanup;
            }
            if (available == 0) {
                break;
            }
            DWORD read = 0;
            const DWORD requested = std::min<DWORD>(
                available, static_cast<DWORD>(buffer.size()));
            if (!WinHttpReadData(request, buffer.data(), requested, &read) ||
                read == 0) {
                *error = LastErrorText(L"读取安装包失败");
                goto cleanup;
            }
            DWORD written = 0;
            if (!WriteFile(output, buffer.data(), read, &written, nullptr) ||
                written != read) {
                *error = LastErrorText(L"写入临时安装包失败");
                goto cleanup;
            }
            received += read;
            const int percent = options.packageSize > 0
                ? static_cast<int>(std::min<unsigned long long>(
                    100, received * 100 / options.packageSize))
                : 0;
            PublishState(5 + percent * 55 / 100,
                         L"正在下载更新",
                         L"已下载 " + std::to_wstring(percent) +
                             L"%，请保持网络连接。" );
        }
        if (received != options.packageSize) {
            *error = L"安装包大小与发布信息不一致。";
            goto cleanup;
        }
    }
    CloseHandle(output);
    output = INVALID_HANDLE_VALUE;
    PublishState(62, L"正在验证更新", L"正在校验安装包完整性…");
    {
        std::wstring actualHash;
        if (!HashFileSha256(destination, &actualHash) ||
            _wcsicmp(actualHash.c_str(), options.packageSha256.c_str()) != 0) {
            *error = L"安装包校验失败，文件可能不完整或已被修改。";
            goto cleanup;
        }
    }
    ok = true;

cleanup:
    if (output != INVALID_HANDLE_VALUE) CloseHandle(output);
    if (request) WinHttpCloseHandle(request);
    if (connection) WinHttpCloseHandle(connection);
    if (session) WinHttpCloseHandle(session);
    if (!ok) DeleteFileW(destination.c_str());
    return ok;
}

bool RunHiddenProcess(const std::wstring& commandLine, DWORD timeoutMs,
                      std::wstring* error)
{
    std::vector<wchar_t> writable(commandLine.begin(), commandLine.end());
    writable.push_back(L'\0');
    STARTUPINFOW startup{};
    startup.cb = sizeof(startup);
    PROCESS_INFORMATION process{};
    if (!CreateProcessW(nullptr, writable.data(), nullptr, nullptr, FALSE,
                        CREATE_NO_WINDOW, nullptr, nullptr, &startup,
                        &process)) {
        *error = LastErrorText(L"无法启动解包程序");
        return false;
    }
    const DWORD wait = WaitForSingleObject(process.hProcess, timeoutMs);
    DWORD exitCode = 1;
    GetExitCodeProcess(process.hProcess, &exitCode);
    CloseHandle(process.hThread);
    CloseHandle(process.hProcess);
    if (wait != WAIT_OBJECT_0 || exitCode != 0) {
        *error = L"更新包解压失败。";
        return false;
    }
    return true;
}

bool RunHiddenProcessCapture(const std::wstring& commandLine,
                             const std::filesystem::path& outputPath,
                             DWORD timeoutMs, std::wstring* error)
{
    SECURITY_ATTRIBUTES security{};
    security.nLength = sizeof(security);
    security.bInheritHandle = TRUE;
    HANDLE output = CreateFileW(
        outputPath.c_str(), GENERIC_WRITE, FILE_SHARE_READ, &security,
        CREATE_ALWAYS, FILE_ATTRIBUTE_TEMPORARY, nullptr);
    if (output == INVALID_HANDLE_VALUE) {
        *error = LastErrorText(L"无法创建更新包检查文件");
        return false;
    }
    HANDLE input = CreateFileW(
        L"NUL", GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE,
        &security, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (input == INVALID_HANDLE_VALUE) {
        CloseHandle(output);
        *error = LastErrorText(L"无法准备更新包检查进程");
        return false;
    }
    std::vector<wchar_t> writable(commandLine.begin(), commandLine.end());
    writable.push_back(L'\0');
    STARTUPINFOW startup{};
    startup.cb = sizeof(startup);
    startup.dwFlags = STARTF_USESTDHANDLES;
    startup.hStdOutput = output;
    startup.hStdError = output;
    startup.hStdInput = input;
    PROCESS_INFORMATION process{};
    const BOOL created = CreateProcessW(
        nullptr, writable.data(), nullptr, nullptr, TRUE,
        CREATE_NO_WINDOW, nullptr, nullptr, &startup, &process);
    CloseHandle(output);
    CloseHandle(input);
    if (!created) {
        *error = LastErrorText(L"无法检查更新包内容");
        return false;
    }
    const DWORD wait = WaitForSingleObject(process.hProcess, timeoutMs);
    DWORD exitCode = 1;
    GetExitCodeProcess(process.hProcess, &exitCode);
    CloseHandle(process.hThread);
    CloseHandle(process.hProcess);
    if (wait != WAIT_OBJECT_0 || exitCode != 0) {
        *error = L"无法读取更新包文件列表。";
        return false;
    }
    return true;
}

bool IsRLinkInstallation(const std::filesystem::path& target)
{
    std::error_code ec;
    if (!target.is_absolute() ||
        !std::filesystem::is_regular_file(target / L"RLinkAPP.exe", ec)) {
        return false;
    }
    const auto metadataPath = target / L"remotec-installation.json";
    if (!std::filesystem::is_regular_file(metadataPath, ec) ||
        std::filesystem::file_size(metadataPath, ec) > 4096) {
        return false;
    }
    std::ifstream metadata(metadataPath, std::ios::binary);
    const std::string text(
        (std::istreambuf_iterator<char>(metadata)),
        std::istreambuf_iterator<char>());
    return text.find("\"product\"") != std::string::npos &&
        text.find("\"RLink\"") != std::string::npos;
}

bool ValidateArchiveListing(const std::filesystem::path& tar,
                            const std::filesystem::path& package,
                            const std::filesystem::path& updateDirectory,
                            std::wstring* error)
{
    const auto listingPath = updateDirectory / L"archive-files.txt";
    const std::wstring command = QuoteArgument(tar.wstring()) + L" -tf " +
        QuoteArgument(package.wstring());
    if (!RunHiddenProcessCapture(command, listingPath, 60000, error)) {
        return false;
    }
    std::ifstream listing(listingPath);
    std::string line;
    bool hasApplication = false;
    while (std::getline(listing, line)) {
        if (!line.empty() && line.back() == '\r') {
            line.pop_back();
        }
        std::replace(line.begin(), line.end(), '\\', '/');
        if (line.empty() || line.front() == '/' || line.find(':') !=
                std::string::npos) {
            *error = L"更新包包含无效的绝对路径。";
            return false;
        }
        std::istringstream components(line);
        std::string component;
        while (std::getline(components, component, '/')) {
            if (component == "..") {
                *error = L"更新包包含越界路径，已拒绝解压。";
                return false;
            }
        }
        hasApplication |= line == "RLinkAPP.exe" ||
            line == "./RLinkAPP.exe";
    }
    if (!listing.eof() || !hasApplication) {
        *error = L"更新包文件列表无效或缺少 RLinkAPP.exe。";
        return false;
    }
    return true;
}

bool ExtractPackage(const std::filesystem::path& package,
                    const std::filesystem::path& staging,
                    std::wstring* error)
{
    std::error_code ec;
    std::filesystem::remove_all(staging, ec);
    if (!std::filesystem::create_directories(staging, ec) && ec) {
        *error = L"无法创建更新暂存目录。";
        return false;
    }
    wchar_t windowsDirectory[MAX_PATH]{};
    if (!GetWindowsDirectoryW(windowsDirectory, MAX_PATH)) {
        *error = LastErrorText(L"无法定位 Windows 目录");
        return false;
    }
    const std::filesystem::path tar =
        std::filesystem::path(windowsDirectory) / L"System32" / L"tar.exe";
    if (!std::filesystem::is_regular_file(tar, ec)) {
        *error = L"当前 Windows 缺少系统解包组件 tar.exe。";
        return false;
    }
    const std::wstring command = QuoteArgument(tar.wstring()) + L" -xf " +
        QuoteArgument(package.wstring()) + L" -C " +
        QuoteArgument(staging.wstring());
    if (!ValidateArchiveListing(tar, package, staging.parent_path(), error)) {
        return false;
    }
    if (!RunHiddenProcess(command, 5 * 60 * 1000, error)) {
        return false;
    }
    if (!std::filesystem::is_regular_file(staging / L"RLinkAPP.exe", ec)) {
        *error = L"更新包中缺少 RLinkAPP.exe。";
        return false;
    }
    return true;
}

bool WaitForParent(DWORD processId, std::wstring* error)
{
    HANDLE process = OpenProcess(SYNCHRONIZE, FALSE, processId);
    if (!process) {
        return true;
    }
    const DWORD wait = WaitForSingleObject(process, 120000);
    CloseHandle(process);
    if (wait != WAIT_OBJECT_0) {
        *error = L"RLink 未能及时退出，更新已取消。";
        return false;
    }
    return true;
}

struct ReplacedFile {
    std::filesystem::path relativePath;
    bool existed = false;
};

void RollBackFiles(const std::filesystem::path& target,
                   const std::filesystem::path& backup,
                   const std::vector<ReplacedFile>& files)
{
    std::error_code ec;
    for (auto iterator = files.rbegin(); iterator != files.rend(); ++iterator) {
        const auto destination = target / iterator->relativePath;
        if (iterator->existed) {
            std::filesystem::copy_file(
                backup / iterator->relativePath, destination,
                std::filesystem::copy_options::overwrite_existing, ec);
        } else {
            std::filesystem::remove(destination, ec);
        }
        ec.clear();
    }
}

bool ReplaceFiles(const std::filesystem::path& staging,
                  const std::filesystem::path& target,
                  const std::filesystem::path& backup,
                  std::wstring* error)
{
    std::error_code ec;
    std::filesystem::remove_all(backup, ec);
    ec.clear();
    std::filesystem::create_directories(backup, ec);
    if (ec) {
        *error = L"无法创建回滚备份目录。";
        return false;
    }
    std::vector<std::filesystem::path> sourceFiles;
    for (std::filesystem::recursive_directory_iterator iterator(staging, ec), end;
         !ec && iterator != end; iterator.increment(ec)) {
        if (iterator->is_symlink(ec) || iterator->is_other(ec)) {
            *error = L"更新包包含不支持的链接或特殊文件。";
            return false;
        }
        if (iterator->is_regular_file(ec)) {
            sourceFiles.push_back(iterator->path());
        }
    }
    if (ec || sourceFiles.empty()) {
        *error = L"无法枚举更新包内容。";
        return false;
    }

    std::vector<ReplacedFile> replaced;
    for (std::size_t index = 0; index < sourceFiles.size(); ++index) {
        const auto relative =
            std::filesystem::relative(sourceFiles[index], staging, ec);
        if (ec || relative.empty() || relative.is_absolute()) {
            *error = L"更新包包含无效路径。";
            RollBackFiles(target, backup, replaced);
            return false;
        }
        const auto destination = target / relative;
        const bool existed = std::filesystem::is_regular_file(destination, ec);
        ec.clear();
        if (existed) {
            std::filesystem::create_directories(
                (backup / relative).parent_path(), ec);
            std::filesystem::copy_file(
                destination, backup / relative,
                std::filesystem::copy_options::overwrite_existing, ec);
            if (ec) {
                *error = L"无法备份现有程序文件。";
                RollBackFiles(target, backup, replaced);
                return false;
            }
        }
        std::filesystem::create_directories(destination.parent_path(), ec);
        ec.clear();
        replaced.push_back({relative, existed});
        std::filesystem::copy_file(
            sourceFiles[index], destination,
            std::filesystem::copy_options::overwrite_existing, ec);
        if (ec) {
            *error = L"无法替换程序文件，已尝试恢复原版本。";
            RollBackFiles(target, backup, replaced);
            return false;
        }
        PublishState(76 + static_cast<int>(
            (index + 1) * 20 / sourceFiles.size()),
            L"正在安装更新",
            L"正在替换程序文件，请勿关闭电脑。" );
    }
    return true;
}

bool RestartApplication(const Options& options, std::wstring* error)
{
    const auto executable = options.targetDirectory /
        options.restartExecutable;
    std::wstring command = QuoteArgument(executable.wstring());
    std::vector<wchar_t> writable(command.begin(), command.end());
    writable.push_back(L'\0');
    STARTUPINFOW startup{};
    startup.cb = sizeof(startup);
    PROCESS_INFORMATION process{};
    if (!CreateProcessW(executable.c_str(), writable.data(), nullptr, nullptr,
                        FALSE, 0, nullptr,
                        options.targetDirectory.c_str(), &startup, &process)) {
        *error = LastErrorText(L"更新完成，但无法重新启动 RLink");
        return false;
    }
    CloseHandle(process.hThread);
    CloseHandle(process.hProcess);
    return true;
}

std::string ReadSmallFile(const std::filesystem::path& path)
{
    std::ifstream input(path, std::ios::binary);
    return std::string((std::istreambuf_iterator<char>(input)),
                       std::istreambuf_iterator<char>());
}

bool RunSelfTest()
{
    const bool policyPassed =
        IsAllowedPackageUrl(
            L"https://github.com/dyhwdnmd/RLink-RemoteCtrl/"
            L"releases/download/v0.2.0/RLink-Windows-x64-0.2.0.zip") &&
        !IsAllowedPackageUrl(
            L"https://github.com/example/other/releases/download/"
            L"v0.2.0/update.zip") &&
        !IsAllowedPackageUrl(L"http://github.com/dyhwdnmd/"
            L"RLink-RemoteCtrl/releases/download/v0.2.0/update.zip") &&
        QuoteArgument(L"C:\\Program Files\\RLink") ==
            L"\"C:\\Program Files\\RLink\"";
    if (!policyPassed) {
        return false;
    }

    wchar_t temporaryDirectory[MAX_PATH]{};
    if (!GetTempPathW(MAX_PATH, temporaryDirectory)) {
        return false;
    }
    const auto root = std::filesystem::path(temporaryDirectory) /
        (L"RLinkUpdaterSelfTest-" + std::to_wstring(GetCurrentProcessId()));
    const auto target = root / L"target";
    const auto staging = root / L"staging";
    const auto backup = root / L"backup";
    std::error_code ec;
    std::filesystem::remove_all(root, ec);
    ec.clear();
    std::filesystem::create_directories(target, ec);
    std::filesystem::create_directories(staging / L"nested", ec);
    if (ec) {
        return false;
    }
    {
        std::ofstream(target / L"RLinkAPP.exe", std::ios::binary) << "old";
        std::ofstream(target / L"remotec-installation.json", std::ios::binary)
            << "{\"product\":\"RLink\",\"version\":\"0.1.0\"}";
        std::ofstream(staging / L"RLinkAPP.exe", std::ios::binary) << "new";
        std::ofstream(staging / L"nested" / L"component.dll",
                      std::ios::binary) << "component";
        std::ofstream(root / L"hash.txt", std::ios::binary) << "abc";
    }
    std::wstring digest;
    std::wstring replaceError;
    const bool hashPassed = HashFileSha256(root / L"hash.txt", &digest) &&
        digest == L"ba7816bf8f01cfea414140de5dae2223"
                  L"b00361a396177a9cb410ff61f20015ad";
    const bool replacePassed = IsRLinkInstallation(target) &&
        ReplaceFiles(staging, target, backup, &replaceError) &&
        ReadSmallFile(target / L"RLinkAPP.exe") == "new" &&
        ReadSmallFile(target / L"nested" / L"component.dll") ==
            "component" &&
        ReadSmallFile(backup / L"RLinkAPP.exe") == "old";
    RollBackFiles(target, backup, {{L"RLinkAPP.exe", true},
                                   {L"nested/component.dll", false}});
    const bool rollbackPassed =
        ReadSmallFile(target / L"RLinkAPP.exe") == "old" &&
        !std::filesystem::exists(target / L"nested" / L"component.dll");
    std::filesystem::remove_all(root, ec);
    return hashPassed && replacePassed && rollbackPassed;
}

void RunUpdate(Options options)
{
    wchar_t modulePath[MAX_PATH * 4]{};
    GetModuleFileNameW(nullptr, modulePath,
                       static_cast<DWORD>(std::size(modulePath)));
    const auto updateDirectory = std::filesystem::path(modulePath).parent_path();
    const auto package = updateDirectory / L"RLink-update.zip";
    const auto staging = updateDirectory / L"staging";
    const auto backup = updateDirectory / L"backup";
    std::wstring error;

    if (!IsRLinkInstallation(options.targetDirectory)) {
        PublishFailure(L"目标目录不是有效的 RLink 安装目录。");
        return;
    }

    PublishState(3, L"正在准备更新",
                 L"正在连接 GitHub Releases…");
    if (!DownloadPackage(options, package, &error)) {
        PublishFailure(error);
        return;
    }
    PublishState(68, L"正在解压更新", L"正在准备新的程序文件…");
    if (!ExtractPackage(package, staging, &error)) {
        PublishFailure(error);
        return;
    }
    PublishState(75, L"正在等待 RLink 退出",
                 L"正在安全结束当前会话与后台进程…");
    if (!WaitForParent(options.parentProcessId, &error)) {
        PublishFailure(error);
        return;
    }
    if (!ReplaceFiles(staging, options.targetDirectory, backup, &error)) {
        PublishFailure(error);
        return;
    }
    PublishState(98, L"正在重新启动", L"更新安装完成，正在启动 RLink…");
    if (!RestartApplication(options, &error)) {
        PublishFailure(error + L"\n更新文件已安装，请手动启动 RLinkAPP.exe。" );
        return;
    }
    {
        std::scoped_lock lock(g_stateMutex);
        g_state.progress = 100;
        g_state.status = L"更新完成";
        g_state.detail = L"RLink 已更新到 v" + options.targetVersion +
            L"，正在重新启动。";
        g_state.finished = true;
    }
    PostMessageW(g_window, kStateChangedMessage, 0, 0);
    Sleep(1200);
    PostMessageW(g_window, WM_CLOSE, 0, 0);
}

void DrawTextLine(HDC dc, const std::wstring& text, RECT bounds,
                  HFONT font, COLORREF color, UINT format)
{
    const auto previousFont = SelectObject(dc, font);
    SetTextColor(dc, color);
    SetBkMode(dc, TRANSPARENT);
    DrawTextW(dc, text.c_str(), -1, &bounds, format);
    SelectObject(dc, previousFont);
}

LRESULT CALLBACK WindowProcedure(HWND window, UINT message,
                                 WPARAM wParam, LPARAM lParam)
{
    switch (message) {
    case WM_NCCREATE: {
        const int preference = 2;  // DWMWCP_ROUND
        DwmSetWindowAttribute(window, 33, &preference, sizeof(preference));
        return TRUE;
    }
    case WM_NCHITTEST:
        return HTCAPTION;
    case kStateChangedMessage:
        InvalidateRect(window, nullptr, FALSE);
        return 0;
    case WM_LBUTTONUP: {
        std::scoped_lock lock(g_stateMutex);
        if (g_state.failed) {
            DestroyWindow(window);
        }
        return 0;
    }
    case WM_CLOSE: {
        std::scoped_lock lock(g_stateMutex);
        if (g_state.finished) {
            DestroyWindow(window);
        } else {
            MessageBeep(MB_ICONINFORMATION);
        }
        return 0;
    }
    case WM_PAINT: {
        PAINTSTRUCT paint{};
        HDC dc = BeginPaint(window, &paint);
        RECT client{};
        GetClientRect(window, &client);
        HBRUSH background = CreateSolidBrush(RGB(52, 53, 51));
        FillRect(dc, &client, background);
        DeleteObject(background);
        UiState state;
        {
            std::scoped_lock lock(g_stateMutex);
            state = g_state;
        }
        HFONT titleFont = CreateFontW(
            -32, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
            DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
            CLEARTYPE_QUALITY, DEFAULT_PITCH, L"Microsoft YaHei UI");
        HFONT bodyFont = CreateFontW(
            -20, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
            DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
            CLEARTYPE_QUALITY, DEFAULT_PITCH, L"Microsoft YaHei UI");
        RECT title{38, 26, client.right - 38, 70};
        DrawTextLine(dc, state.status, title, titleFont,
                     RGB(248, 248, 246), DT_LEFT | DT_SINGLELINE);
        RECT detail{38, 78, client.right - 38, 125};
        DrawTextLine(dc, state.detail, detail, bodyFont,
                     state.failed ? RGB(255, 170, 160) : RGB(185, 185, 180),
                     DT_LEFT | DT_WORDBREAK);
        RECT track{38, 158, client.right - 180, 176};
        HBRUSH trackBrush = CreateSolidBrush(RGB(79, 80, 76));
        FillRect(dc, &track, trackBrush);
        DeleteObject(trackBrush);
        if (!state.failed) {
            RECT fill = track;
            fill.right = fill.left +
                (fill.right - fill.left) * state.progress / 100;
            HBRUSH fillBrush = CreateSolidBrush(RGB(208, 132, 93));
            FillRect(dc, &fill, fillBrush);
            DeleteObject(fillBrush);
        }
        RECT percent{client.right - 160, 148, client.right - 38, 182};
        const std::wstring percentText = state.failed
            ? L"点击关闭"
            : std::to_wstring(state.progress) + L"%";
        DrawTextLine(dc, percentText, percent, bodyFont,
                     RGB(220, 220, 215), DT_RIGHT | DT_VCENTER |
                         DT_SINGLELINE);
        DeleteObject(titleFont);
        DeleteObject(bodyFont);
        EndPaint(window, &paint);
        return 0;
    }
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    default:
        return DefWindowProcW(window, message, wParam, lParam);
    }
}

}  // namespace

int WINAPI wWinMain(HINSTANCE instance, HINSTANCE, PWSTR, int)
{
    if (wcsstr(GetCommandLineW(), L"--self-test") != nullptr) {
        return RunSelfTest() ? 0 : 10;
    }
    const auto options = ParseOptions();
    if (!options) {
        MessageBoxW(nullptr, L"更新参数无效，无法启动 RLink 更新。",
                    L"RLink 更新", MB_OK | MB_ICONERROR);
        return 2;
    }
    SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);
    const wchar_t className[] = L"RLinkUpdaterWindow";
    WNDCLASSEXW windowClass{};
    windowClass.cbSize = sizeof(windowClass);
    windowClass.lpfnWndProc = WindowProcedure;
    windowClass.hInstance = instance;
    windowClass.hCursor = LoadCursorW(nullptr, IDC_ARROW);
    windowClass.lpszClassName = className;
    if (!RegisterClassExW(&windowClass)) {
        return 3;
    }
    const int width = 800;
    const int height = 220;
    const int x = (GetSystemMetrics(SM_CXSCREEN) - width) / 2;
    const int y = (GetSystemMetrics(SM_CYSCREEN) - height) / 2;
    g_window = CreateWindowExW(
        WS_EX_TOPMOST | WS_EX_TOOLWINDOW, className, L"RLink 更新",
        WS_POPUP, x, y, width, height, nullptr, nullptr, instance, nullptr);
    if (!g_window) {
        return 4;
    }
    ShowWindow(g_window, SW_SHOWNORMAL);
    UpdateWindow(g_window);
    std::thread(RunUpdate, *options).detach();
    MSG message{};
    while (GetMessageW(&message, nullptr, 0, 0) > 0) {
        TranslateMessage(&message);
        DispatchMessageW(&message);
    }
    return 0;
}
