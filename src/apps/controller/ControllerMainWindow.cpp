// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#include "ControllerMainWindow.h"

#include <QAction>
#include <QAbstractItemView>
#include <QApplication>
#include <QButtonGroup>
#include <QClipboard>
#include <QCloseEvent>
#include <QColor>
#include <QComboBox>
#include <QCryptographicHash>
#include <QCursor>
#include <QDateTime>
#include <QDesktopServices>
#include <QDir>
#include <QEasingCurve>
#include <QEvent>
#include <QFile>
#include <QFileDialog>
#include <QFrame>
#include <QFont>
#include <QFontDatabase>
#include <QFontMetrics>
#include <QGraphicsOpacityEffect>
#include <QGridLayout>
#include <QHash>
#include <QHBoxLayout>
#include <QIcon>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QListWidgetItem>
#include <QMenu>
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>
#include <QParallelAnimationGroup>
#include <QPropertyAnimation>
#include <QProcess>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QPushButton>
#include <QResizeEvent>
#include <QRegion>
#include <QRegularExpression>
#include <QRegularExpressionValidator>
#include <QScrollArea>
#include <QScrollBar>
#include <QScreen>
#include <QSettings>
#include <QSizePolicy>
#include <QSignalBlocker>
#include <QStackedWidget>
#include <QStorageInfo>
#include <QStandardItemModel>
#include <QStandardPaths>
#include <QStyle>
#include <QStyleHints>
#include <QString>
#include <QStringConverter>
#include <QStringList>
#include <QTextStream>
#include <QTemporaryFile>
#include <QTimer>
#include <QWheelEvent>
#include <QThread>
#include <QSystemTrayIcon>
#include <QToolButton>
#include <QUrl>
#include <QVariant>
#include <QVariantAnimation>
#include <QVBoxLayout>
#include <QWidget>
#include <QWidgetAction>
#include <QWindow>

#include "RoundedPopupMenu.h"

#ifdef Q_OS_WIN
#include <qt_windows.h>
#include <dbt.h>
#endif

#include <algorithm>
#include <array>
#include <chrono>
#include <cmath>
#include <condition_variable>
#include <deque>
#include <functional>
#include <filesystem>
#include <mutex>
#include <thread>
#include <tuple>
#include <utility>

#include "CameraWindow.h"
#include "FileTransferWindow.h"
#include "LoginWindow.h"
#include "RemoteSessionWindow.h"
#include "RemoteCComboBox.h"
#include "RemoteCDialog.h"
#include "RemoteCToast.h"
#include "RoomCameraWindow.h"
#include "src/apps/remote/FileTransferController.h"
#include "src/apps/remote/ClipboardController.h"
#include "src/apps/remote/EncoderBenchmarkProfileCache.h"
#include "src/apps/remote/InProcessSessionEngine.h"
#include "src/apps/controller/ui/morph/MorphIconButtonBinding.h"
#include "src/apps/controller/ui/RemoteCTheme.h"
#include "src/core/ISessionEngine.h"
#include "src/platform/win/FfmpegHardwareH264Encoder.h"
#include "src/platform/win/H264EncoderBenchmark.h"
#include "src/platform/win/WindowsHardwareFingerprint.h"
#include "src/platform/win/VideoDecoderProbePolicy.h"
#include "src/platform/win/VideoEncoderProbePolicy.h"
#include "src/platform/win/WindowsInputExecutor.h"

namespace remote::controller {
namespace {

constexpr auto kDefaultFileSaveDirectorySetting =
    "files/defaultSaveDirectory";

bool IsNineDigitPublicId(const QString& value)
{
    static const QRegularExpression pattern(
        QStringLiteral("^[1-9][0-9]{8}$"));
    return pattern.match(value).hasMatch();
}

QString LocalizedDirectSessionError(
    const std::string& errorCode,
    const std::string& /*fallbackMessage*/,
    const SessionEngineSnapshot* snapshot = nullptr)
{
    const QString code = QString::fromStdString(errorCode);
    if (code == QStringLiteral("engine_not_ready")) {
        if (snapshot) {
            if (snapshot->state == SessionEngineState::kStarting) {
                return QStringLiteral("远程服务正在初始化，请稍后重试。");
            }
            if (snapshot->room.membership != RoomMembershipState::kNone &&
                snapshot->room.membership != RoomMembershipState::kFailed) {
                return QStringLiteral(
                    "当前设备已在协助房间中，请先退出房间后再连接其他设备。");
            }
            if (snapshot->state == SessionEngineState::kConnecting ||
                snapshot->state == SessionEngineState::kAwaitingLocalApproval ||
                snapshot->state == SessionEngineState::kActive ||
                snapshot->state == SessionEngineState::kStopping) {
                return QStringLiteral(
                    "当前已有远程会话正在进行，请先结束当前会话后再连接其他设备。");
            }
        }
        return QStringLiteral("远程会话服务暂未就绪，请稍后重试。");
    }
    if (code == QStringLiteral("room_active")) {
        return QStringLiteral(
            "当前设备已在协助房间中，请先退出房间后再发起远程连接。");
    }
    if (code == QStringLiteral("device_busy")) {
        return QStringLiteral(
            "对方设备已在协助房间或其他远程会话中，请结束后再试。");
    }
    if (code == QStringLiteral("verification_code_invalid")) {
        return QStringLiteral(
            "验证码错误，请向对方核对当前显示的 6 位一次性验证码。");
    }
    if (code == QStringLiteral("target_offline")) {
        return QStringLiteral("对方设备当前不在线，请确认 RLink 已运行。");
    }
    if (code == QStringLiteral("rate_limited")) {
        return QStringLiteral("连接尝试过于频繁，请稍后再试。");
    }
    if (code == QStringLiteral("session_request_timeout")) {
        return QStringLiteral("连接请求超时，请确认对方在线后重试。");
    }
    if (code == QStringLiteral("session_rejected")) {
        return QStringLiteral("对方设备未接受本次远程连接。");
    }
    if (code == QStringLiteral("invalid_assistance_credentials")) {
        return QStringLiteral("请输入正确的设备 ID 和 6 位验证码。");
    }
    if (code == QStringLiteral("signaling_not_online")) {
        return QStringLiteral("信令服务尚未连接，请稍后重试。");
    }
    return code.isEmpty()
        ? QStringLiteral("远程连接未能建立，请稍后重试。")
        : QStringLiteral("远程连接未能建立（错误代码：%1），请稍后重试。")
              .arg(code);
}

bool IsDirectRecoveryFailureCode(const std::string& errorCode)
{
    return errorCode == "peer_reconnect_timeout" ||
           errorCode == "ice_restart_exhausted";
}
constexpr auto kVideoEncoderPreferenceSetting =
    "media/videoEncoderPreference";
constexpr auto kFfmpegX264PresetSetting =
    "media/ffmpegX264Preset";
constexpr auto kFfmpegHardwareBackendSetting =
    "media/ffmpegHardwareBackend";
constexpr auto kVideoDecoderPreferenceSetting =
    "media/videoDecoderPreference";
constexpr auto kVideoRendererPreferenceSetting =
    "media/videoRendererPreference";
constexpr auto kDesktopCaptureBackendSetting =
    "media/desktopCaptureBackend";
constexpr auto kScreenFrameRateLogEnabledSetting =
    "diagnostics/screenFrameRateCsvEnabled";
constexpr auto kInputEventStatsEnabledSetting =
    "diagnostics/inputEventStatsEnabled";
constexpr auto kBestDecoderNameSetting =
    "media/decoderProbe/bestDecoderName";
constexpr auto kBestDecoderAverageSetting =
    "media/decoderProbe/averageLatencyMs";
constexpr auto kBestDecoderP95Setting =
    "media/decoderProbe/p95LatencyMs";
constexpr auto kBestDecoderTestTimeSetting =
    "media/decoderProbe/testedAtUtc";
constexpr auto kDecoderCandidatesSetting =
    "media/decoderProbe/candidates";
constexpr auto kDecoderHardwareFingerprintSetting =
    "media/decoderProbe/hardwareFingerprint";
constexpr auto kDecoderBenchmarkCompletedSetting =
    "media/decoderProbe/completed";
constexpr auto kDecoderBenchmarkPassedSetting =
    "media/decoderProbe/passed";
constexpr auto kDecoderBenchmarkPolicyVersionSetting =
    "media/decoderProbe/policyVersion";
constexpr int kDecoderBenchmarkPolicyVersion =
    kVideoDecoderProbePolicyVersion;
constexpr auto kBestEncoderIdSetting =
    "media/encoderProbe/bestEncoderId";
constexpr auto kBestEncoderNameSetting =
    "media/encoderProbe/bestEncoderName";
constexpr auto kEncoderCandidatesSetting =
    "media/encoderProbe/candidates";
constexpr auto kEncoderHardwareFingerprintSetting =
    "media/encoderProbe/hardwareFingerprint";
constexpr auto kEncoderCaptureBackendSetting =
    "media/encoderProbe/captureBackend";
constexpr auto kEncoderX264PresetSetting =
    "media/encoderProbe/ffmpegX264Preset";
constexpr auto kEncoderBenchmarkCompletedSetting =
    "media/encoderProbe/completed";
constexpr auto kEncoderBenchmarkPassedSetting =
    "media/encoderProbe/passed";
constexpr auto kEncoderBenchmarkPolicyVersionSetting =
    "media/encoderProbe/policyVersion";
constexpr auto kBestEncoderTestTimeSetting =
    "media/encoderProbe/testedAtUtc";
constexpr int kEncoderBenchmarkPolicyVersion =
    kVideoEncoderProbePolicyVersion;
constexpr auto kCameraDeviceSetting =
    "media/cameraDeviceId";
constexpr auto kMicrophoneDeviceSetting =
    "media/microphoneDeviceId";
constexpr auto kSpeakerDeviceSetting =
    "media/speakerDeviceId";
constexpr auto kDefaultRoomCapacitySetting =
    "rooms/defaultCapacity";
constexpr auto kCloseButtonBehaviorSetting =
    "app/closeButtonBehavior";
constexpr auto kStartupVisibilitySetting =
    "app/startupVisibility";
constexpr auto kWindowsAutoStartValueName = "RemoteC";
constexpr auto kAutoOpenCameraGallerySetting =
    "media/autoOpenCameraGallery";
constexpr auto kInterfaceFontFamilySetting = "ui/systemFontFamily";

QStringList InstalledChineseInterfaceFonts()
{
    QStringList families;
    for (const QString& family : QFontDatabase::families()) {
        if (family.startsWith(QLatin1Char('@'))) {
            continue;
        }
        const auto writingSystems = QFontDatabase::writingSystems(family);
        if (writingSystems.contains(QFontDatabase::SimplifiedChinese) ||
            writingSystems.contains(QFontDatabase::TraditionalChinese)) {
            families.push_back(family);
        }
    }
    families.removeDuplicates();
    families.sort(Qt::CaseInsensitive);
    return families;
}

void ApplyInterfaceFontPreference(const QString& systemFamily,
                                  int pixelSize)
{
    QFont font;
    if (systemFamily.trimmed().isEmpty()) {
        font.setFamilies({
            QStringLiteral("Microsoft YaHei UI"),
            QStringLiteral("Segoe UI")});
    } else {
        font.setFamilies({systemFamily,
                          QStringLiteral("Microsoft YaHei UI"),
                          QStringLiteral("Segoe UI")});
    }
    font.setHintingPreference(QFont::PreferDefaultHinting);
    font.setStyleStrategy(QFont::PreferAntialias);
    font.setPixelSize(std::clamp(pixelSize, 12, 17));
    QApplication::setFont(font);
}

QString AdaptBenchmarkHtmlForTheme(QString html)
{
    if (!ui::RemoteCTheme::IsDark(ui::RemoteCTheme::LoadPreference())) {
        return html;
    }
    const std::pair<const char*, const char*> colors[] = {
        {"#172033", "#F1F5FB"},
        {"#24324b", "#E6EDF7"},
        {"#667085", "#AEBBD0"},
        {"#7a8496", "#9EACC0"},
        {"#98a2b3", "#7F8DA3"},
        {"#1769e8", "#8EA5FF"},
        {"#138b57", "#4FF0B5"},
        {"#c12b37", "#FF98A5"},
        {"#a76508", "#F2C572"},
        {"#e3e8f1", "#354861"},
        {"#e8ecf3", "#354861"},
    };
    for (const auto& [light, dark] : colors) {
        html.replace(QString::fromLatin1(light),
                     QString::fromLatin1(dark),
                     Qt::CaseInsensitive);
    }
    return html;
}

VideoEncoderPreference VideoEncoderPreferenceFromSetting(
    const QString& value)
{
    if (value == QStringLiteral("hardware")) {
        return VideoEncoderPreference::kHardwareOnly;
    }
    if (value == QStringLiteral("ffmpeg_hardware")) {
        return VideoEncoderPreference::kFfmpegHardware;
    }
    if (value == QStringLiteral("software")) {
        return VideoEncoderPreference::kSoftwareOnly;
    }
    if (value == QStringLiteral("ffmpeg")) {
        return VideoEncoderPreference::kFfmpegX264Only;
    }
    return VideoEncoderPreference::kAutomatic;
}

DesktopCaptureImplementation DesktopCaptureImplementationFromSetting(
    const QString& value)
{
    return value == QStringLiteral("libwebrtc")
        ? DesktopCaptureImplementation::kLibWebRtc
        : DesktopCaptureImplementation::kNativeDxgi;
}

FfmpegHardwareBackend FfmpegHardwareBackendFromSetting(
    const QString& value)
{
    if (value == QStringLiteral("qsv")) {
        return FfmpegHardwareBackend::kQsv;
    }
    if (value == QStringLiteral("nvenc")) {
        return FfmpegHardwareBackend::kNvenc;
    }
    if (value == QStringLiteral("amf")) {
        return FfmpegHardwareBackend::kAmf;
    }
    return FfmpegHardwareBackend::kAutomatic;
}

FfmpegX264Preset EncoderQualityFromSetting(const QString& value)
{
    if (value == QStringLiteral("veryslow")) {
        return FfmpegX264Preset::kVerySlow;
    }
    if (value == QStringLiteral("slow")) {
        return FfmpegX264Preset::kSlow;
    }
    if (value == QStringLiteral("veryfast")) {
        return FfmpegX264Preset::kVeryFast;
    }
    if (value == QStringLiteral("ultrafast")) {
        return FfmpegX264Preset::kUltraFast;
    }
    return FfmpegX264Preset::kMedium;
}

VideoDecoderPreference VideoDecoderPreferenceFromSetting(
    const QString& value)
{
    if (value == QStringLiteral("hardware")) {
        return VideoDecoderPreference::kHardwareOnly;
    }
    if (value == QStringLiteral("software")) {
        return VideoDecoderPreference::kSoftwareOnly;
    }
    return VideoDecoderPreference::kAutomatic;
}
constexpr auto kRemoteScreenQualitySetting =
    "remoteSession/qualityTier";
constexpr auto kDragPointerSampleRateSetting =
    "remoteSession/dragPointerSampleRateHz";
constexpr auto kRemotePasteEnabledSetting =
    "remoteSession/remotePasteEnabled";
constexpr auto kClipboardFormatsSetting =
    "remoteSession/clipboardFormats";
constexpr auto kClipboardFileLimitSetting =
    "remoteSession/clipboardFileLimitMiB";
constexpr auto kClipboardCacheBaseDirectorySetting =
    "transfer/remotePaste/cacheBaseDirectory";
constexpr auto kClipboardCacheRetentionSetting =
    "transfer/remotePaste/cacheRetentionMinutes";
constexpr auto kClipboardCacheCapacitySetting =
    "transfer/remotePaste/cacheLimitGiB";
constexpr std::uint64_t kRemotePastePopupThresholdBytes = 64ull * 1024ull;
constexpr int kSettingsControlWidth = 270;

QString InitialClipboardCacheBaseDirectory()
{
    const QString localAppData = qEnvironmentVariable("LOCALAPPDATA");
    return QDir::cleanPath(localAppData.isEmpty()
        ? QDir::tempPath() : localAppData);
}

QString ClipboardCacheRootForBase(const QString& baseDirectory)
{
    return QDir::cleanPath(QDir(baseDirectory).filePath(
        QStringLiteral("RemoteC/ClipboardCache")));
}

qulonglong SafeClipboardCacheCapacityGiB(const QString& baseDirectory)
{
    QStorageInfo storage(baseDirectory);
    if (!storage.isValid() || !storage.isReady()) return 0;
    constexpr qulonglong kGiB = 1024ull * 1024 * 1024;
    return static_cast<qulonglong>(storage.bytesAvailable()) /
        2ull / kGiB;
}

QString WindowsAutoStartCommand()
{
    return QStringLiteral("\"%1\"").arg(
        QDir::toNativeSeparators(QCoreApplication::applicationFilePath()));
}

bool WindowsAutoStartEnabled()
{
#ifdef Q_OS_WIN
    QSettings runKey(
        QStringLiteral(
            "HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Run"),
        QSettings::NativeFormat);
    return runKey.value(QString::fromLatin1(kWindowsAutoStartValueName))
               .toString() == WindowsAutoStartCommand();
#else
    return false;
#endif
}

bool SetWindowsAutoStartEnabled(bool enabled)
{
#ifdef Q_OS_WIN
    QSettings runKey(
        QStringLiteral(
            "HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Run"),
        QSettings::NativeFormat);
    const QString valueName =
        QString::fromLatin1(kWindowsAutoStartValueName);
    if (enabled) {
        runKey.setValue(valueName, WindowsAutoStartCommand());
    } else {
        runKey.remove(valueName);
    }
    runKey.sync();
    return runKey.status() == QSettings::NoError &&
           WindowsAutoStartEnabled() == enabled;
#else
    Q_UNUSED(enabled);
    return false;
#endif
}

std::pair<std::uint32_t, std::uint32_t> SavedScreenQualityBounds(
    ScreenQualityTier quality)
{
    switch (quality) {
    case ScreenQualityTier::k720p:
        return {1280, 720};
    case ScreenQualityTier::k1080p:
    case ScreenQualityTier::kAutomatic:
        return {1920, 1080};
    case ScreenQualityTier::k1440p:
        return {2560, 1440};
    case ScreenQualityTier::kOriginal:
        return {0, 0};
    }
    return {0, 0};
}

QString CsvCell(const std::string& value)
{
    QString escaped = QString::fromStdString(value);
    escaped.replace('"', QStringLiteral("\"\""));
    return QStringLiteral("\"%1\"").arg(escaped);
}

struct ScreenFrameRateLogRecord {
    qint64 timestampMs = 0;
    std::string pairId;
    std::string peerDeviceId;
    bool outbound = false;
    std::string configuredCaptureBackend;
    std::string activeCaptureBackend;
    std::string captureFallbackReason;
    std::string activity;
    std::uint32_t captureTargetFps = 0;
    double captureAttemptFps = 0.0;
    double captureDeliveredFps = 0.0;
    double captureChangedFps = 0.0;
    double captureHeartbeatFps = 0.0;
    std::uint64_t captureSuppressedTotal = 0;
    bool captureInputBoostActive = false;
    std::uint64_t captureInputBoostTotal = 0;
    std::uint64_t captureForcedRefreshTotal = 0;
    double sourceFps = 0.0;
    double rtpFps = 0.0;
    double encodedFps = 0.0;
    double sentFps = 0.0;
    double presentedFps = 0.0;
    std::uint64_t bitrateBps = 0;
};

class AsyncScreenFrameRateLogger final {
public:
    static AsyncScreenFrameRateLogger& Instance()
    {
        static AsyncScreenFrameRateLogger logger;
        return logger;
    }

    void Enqueue(std::vector<ScreenFrameRateLogRecord> records)
    {
        if (records.empty()) {
            return;
        }
        {
            std::lock_guard lock(mutex_);
            for (auto& record : records) {
                if (records_.size() >= kMaximumQueuedRecords) {
                    records_.pop_front();
                }
                records_.push_back(std::move(record));
            }
        }
        condition_.notify_one();
    }

private:
    static constexpr std::size_t kMaximumQueuedRecords = 4096;

    AsyncScreenFrameRateLogger()
        : worker_([this](std::stop_token stopToken) { Run(stopToken); })
    {}

    ~AsyncScreenFrameRateLogger()
    {
        worker_.request_stop();
        condition_.notify_all();
        if (worker_.joinable()) {
            worker_.join();
        }
    }

    void Run(std::stop_token stopToken)
    {
        const QString directory = QStandardPaths::writableLocation(
            QStandardPaths::AppLocalDataLocation);
        if (directory.isEmpty() || !QDir().mkpath(directory)) {
            return;
        }
        const QString logPath = QDir(directory).filePath(
            QStringLiteral("screen-frame-rate-%1.csv").arg(
                QCoreApplication::applicationPid()));
        QFile logFile(logPath);
        const bool writeHeader =
            !logFile.exists() || logFile.size() == 0;
        if (!logFile.open(
                QIODevice::WriteOnly | QIODevice::Append |
                QIODevice::Text)) {
            return;
        }

        QTextStream output(&logFile);
        output.setEncoding(QStringConverter::Utf8);
        if (writeHeader) {
            output << "time,pair_id,peer_device_id,direction,"
                      "configured_capture_backend,active_capture_backend,"
                      "capture_fallback_reason,activity,"
                      "capture_target_fps,capture_attempt_fps,"
                      "capture_delivered_fps,capture_changed_fps,"
                      "capture_heartbeat_fps,capture_suppressed_total,"
                      "capture_input_boost_active,capture_input_boost_total,"
                      "capture_forced_refresh_total,"
                      "source_fps,rtp_fps,encoded_fps,sent_fps,"
                      "presented_fps,bitrate_bps\n";
        }

        for (;;) {
            std::deque<ScreenFrameRateLogRecord> pending;
            {
                std::unique_lock lock(mutex_);
                condition_.wait(lock, [this, &stopToken] {
                    return stopToken.stop_requested() || !records_.empty();
                });
                pending.swap(records_);
            }
            for (const auto& record : pending) {
                const QString timestamp =
                    QDateTime::fromMSecsSinceEpoch(record.timestampMs)
                        .toString(Qt::ISODateWithMs);
                output << timestamp << ','
                       << CsvCell(record.pairId) << ','
                       << CsvCell(record.peerDeviceId) << ','
                       << (record.outbound ? "outbound" : "inbound")
                       << ',' << CsvCell(record.configuredCaptureBackend)
                       << ',' << CsvCell(record.activeCaptureBackend)
                       << ',' << CsvCell(record.captureFallbackReason)
                       << ',' << CsvCell(record.activity)
                       << ',' << record.captureTargetFps
                       << ',' << QString::number(record.captureAttemptFps, 'f', 3)
                       << ',' << QString::number(record.captureDeliveredFps, 'f', 3)
                       << ',' << QString::number(record.captureChangedFps, 'f', 3)
                       << ',' << QString::number(record.captureHeartbeatFps, 'f', 3)
                       << ',' << record.captureSuppressedTotal
                       << ',' << (record.captureInputBoostActive ? 1 : 0)
                       << ',' << record.captureInputBoostTotal
                       << ',' << record.captureForcedRefreshTotal
                       << ',' << QString::number(record.sourceFps, 'f', 3)
                       << ',' << QString::number(record.rtpFps, 'f', 3)
                       << ',' << QString::number(record.encodedFps, 'f', 3)
                       << ',' << QString::number(record.sentFps, 'f', 3)
                       << ',' << QString::number(record.presentedFps, 'f', 3)
                       << ',' << record.bitrateBps << '\n';
            }
            output.flush();
            if (stopToken.stop_requested()) {
                std::lock_guard lock(mutex_);
                if (records_.empty()) {
                    break;
                }
            }
        }
    }

    std::mutex mutex_;
    std::condition_variable condition_;
    std::deque<ScreenFrameRateLogRecord> records_;
    std::jthread worker_;
};

void AppendScreenFrameRateLog(
    const SessionDiagnosticsSnapshot& diagnostics)
{
    std::vector<ScreenFrameRateLogRecord> records;
    const qint64 timestampMs =
        QDateTime::currentMSecsSinceEpoch();
    for (const auto& peer : diagnostics.peerConnections) {
        for (const auto& stream : peer.stats.rtpStreams) {
            if (stream.kind != "video" ||
                stream.slot != kScreenMainVideoSlot) {
                continue;
            }
            ScreenFrameRateLogRecord record;
            record.timestampMs = timestampMs;
            record.pairId = peer.pairId;
            record.peerDeviceId = peer.peerDeviceId;
            record.outbound =
                stream.direction == RtpStreamDirection::kOutbound;
            record.configuredCaptureBackend =
                stream.captureConfiguredBackend;
            record.activeCaptureBackend = stream.captureActiveBackend;
            record.captureFallbackReason = stream.captureFallbackReason;
            record.activity = stream.captureActivityState;
            record.captureTargetFps = stream.captureTargetFrameRate;
            record.captureAttemptFps = stream.captureAttemptsPerSecond;
            record.captureDeliveredFps =
                stream.captureDeliveredFramesPerSecond;
            record.captureChangedFps =
                stream.captureChangedFramesPerSecond;
            record.captureHeartbeatFps =
                stream.captureIdleHeartbeatFramesPerSecond;
            record.captureSuppressedTotal =
                stream.captureSuppressedUnchangedFrames;
            record.captureInputBoostActive =
                stream.captureInputBoostActive;
            record.captureInputBoostTotal = stream.captureInputBoosts;
            record.captureForcedRefreshTotal =
                stream.captureForcedRefreshFrames;
            record.sourceFps = stream.sourceFramesPerSecond;
            record.rtpFps = stream.framesPerSecond;
            record.encodedFps = stream.encodedFramesPerSecond;
            record.sentFps = stream.sentFramesPerSecond;
            record.presentedFps = stream.presentedFramesPerSecond;
            record.bitrateBps = stream.bitrateBps;
            records.push_back(std::move(record));
        }
    }
    AsyncScreenFrameRateLogger::Instance().Enqueue(std::move(records));
}

QString FormatBitrate(std::uint64_t bitsPerSecond)
{
    if (bitsPerSecond >= 1'000'000) {
        return QStringLiteral("%1 Mbps")
            .arg(static_cast<double>(bitsPerSecond) / 1'000'000.0,
                 0, 'f', 2);
    }
    if (bitsPerSecond >= 1'000) {
        return QStringLiteral("%1 Kbps")
            .arg(static_cast<double>(bitsPerSecond) / 1'000.0,
                 0, 'f', 1);
    }
    return QStringLiteral("%1 bps").arg(bitsPerSecond);
}

QString FormatByteCount(std::uint64_t bytes)
{
    if (bytes >= 1024ULL * 1024ULL * 1024ULL) {
        return QStringLiteral("%1 GB")
            .arg(static_cast<double>(bytes) /
                     (1024.0 * 1024.0 * 1024.0),
                 0, 'f', 2);
    }
    if (bytes >= 1024ULL * 1024ULL) {
        return QStringLiteral("%1 MB")
            .arg(static_cast<double>(bytes) / (1024.0 * 1024.0),
                 0, 'f', 2);
    }
    if (bytes >= 1024ULL) {
        return QStringLiteral("%1 KB")
            .arg(static_cast<double>(bytes) / 1024.0,
                 0, 'f', 1);
    }
    return QStringLiteral("%1 B").arg(bytes);
}

QString SampleWindowSuffix(std::uint32_t windowMs)
{
    return windowMs > 0
               ? QStringLiteral(" · %1 ms采样窗").arg(windowMs)
               : QStringLiteral(" · 等待第二次采样");
}

QString LatestFrameTimingText(
    const RtpStreamStatsSnapshot& stream,
    const QString& action)
{
    if (!stream.latestFrameTimingAvailable) {
        return QStringLiteral("等待逐帧%1计时").arg(action);
    }
    QString text =
        QStringLiteral("%1 ms · RTP %2 · %3 ms前")
            .arg(stream.latestFrameTimeMs, 0, 'f', 3)
            .arg(stream.latestFrameRtpTimestamp)
            .arg(stream.latestFrameTimingAgeMs);
    if (stream.latestFrameWidth > 0 &&
        stream.latestFrameHeight > 0) {
        text += QStringLiteral(" · %1×%2")
                    .arg(stream.latestFrameWidth)
                    .arg(stream.latestFrameHeight);
    }
    if (stream.latestEncodedBytes > 0) {
        text += QStringLiteral(" · %1")
                    .arg(FormatByteCount(
                        stream.latestEncodedBytes));
    }
    return text;
}

QString RouteDisplayName(const std::string& route)
{
    if (route == "host") {
        return QStringLiteral("局域网/主机直连");
    }
    if (route == "stun") {
        return QStringLiteral("STUN 公网直连");
    }
    if (route == "turn") {
        return QStringLiteral("TURN 中继");
    }
    return QStringLiteral("尚未确定");
}

QString SlotDisplayName(const std::string& slot,
                        const std::string& kind)
{
    if (slot == kScreenMainVideoSlot) {
        return QStringLiteral("屏幕");
    }
    if (slot == kCameraMainVideoSlot) {
        return QStringLiteral("摄像头");
    }
    if (slot == kMicrophoneMainAudioSlot || kind == "audio") {
        return QStringLiteral("麦克风");
    }
    return kind == "video" ? QStringLiteral("视频")
                           : QStringLiteral("媒体");
}

QString CandidateDisplayText(
    const IceCandidateStatsSnapshot& candidate)
{
    if (candidate.address.empty()) {
        return QStringLiteral("未报告");
    }
    QString result = QStringLiteral("%1:%2 · %3/%4")
        .arg(QString::fromStdString(candidate.address))
        .arg(candidate.port)
        .arg(QString::fromStdString(candidate.candidateType))
        .arg(QString::fromStdString(candidate.protocol));
    if (!candidate.networkType.empty()) {
        result += QStringLiteral(" · %1")
                      .arg(QString::fromStdString(
                          candidate.networkType));
    }
    if (!candidate.adapterType.empty()) {
        result += QStringLiteral(" · %1")
                      .arg(QString::fromStdString(
                          candidate.adapterType));
    }
    return result;
}

QString DiagnosticsMetricExplanation(const QString& label)
{
    static const QHash<QString, QString> explanations{
        {QStringLiteral("状态"),
         QStringLiteral("当前对象的运行状态；等待、连接中或关闭时会随实时状态更新。")},
        {QStringLiteral("连接路径"),
         QStringLiteral("当前选中的 ICE 传输路径：局域网直连、STUN 公网直连或 TURN 中继。")},
        {QStringLiteral("当前 RTT"),
         QStringLiteral("当前候选对的网络往返时间，不包含视频编解码和界面显示时间。")},
        {QStringLiteral("采样窗上行"),
         QStringLiteral("相邻两次 Stats 采样之间实际发送字节的平均速率，不是会话累计平均值。")},
        {QStringLiteral("采样窗下行"),
         QStringLiteral("相邻两次 Stats 采样之间实际接收字节的平均速率，不是会话累计平均值。")},
        {QStringLiteral("ICE"),
         QStringLiteral("ICE 连接状态，反映候选地址检查和 P2P 网络连通情况。")},
        {QStringLiteral("DTLS"),
         QStringLiteral("DTLS 安全传输状态；connected 表示密钥协商已经完成。")},
        {QStringLiteral("连接角色"),
         QStringLiteral("本机在 ICE 协商中的 controlling 或 controlled 角色，不代表远控中的控制端身份。")},
        {QStringLiteral("候选对"),
         QStringLiteral("当前 ICE 候选地址对的状态；succeeded 表示该路径已验证可用。")},
        {QStringLiteral("本地候选"),
         QStringLiteral("当前传输路径使用的本机地址、端口、候选类型、协议和网络适配器。")},
        {QStringLiteral("远端候选"),
         QStringLiteral("当前传输路径使用的对端地址、端口、候选类型和协议。")},
        {QStringLiteral("估计可用上行"),
         QStringLiteral("WebRTC 拥塞控制估计的当前可用发送带宽，不等于运营商标称带宽。")},
        {QStringLiteral("估计可用下行"),
         QStringLiteral("WebRTC 在可获得该信息时报告的当前可用接收带宽。")},
        {QStringLiteral("安全套件"),
         QStringLiteral("当前信令/媒体连接协商使用的 TLS、DTLS 和 SRTP 加密算法。")},
        {QStringLiteral("累计路径切换"),
         QStringLiteral("本次成员对会话中选中的 ICE 候选对发生变化的累计次数。")},
        {QStringLiteral("累计发送丢弃"),
         QStringLiteral("数据交给系统网络套接字前因错误或缓冲不足而被本机丢弃的累计包数和字节数。")},
        {QStringLiteral("编码格式"),
         QStringLiteral("当前 RTP 媒体流实际协商并使用的编码格式。")},
        {QStringLiteral("采样窗码率"),
         QStringLiteral("相邻两次 Stats 采样之间该 RTP 流的有效负载码率；静止画面可能非常低。")},
        {QStringLiteral("编码器"),
         QStringLiteral("当前 RTP 发送流实际使用的编码器实现，不是设置中的期望值。")},
        {QStringLiteral("解码器"),
         QStringLiteral("当前 RTP 接收流实际使用的解码器实现，可用于确认硬件或软件路径。")},
        {QStringLiteral("累计数据包"),
         QStringLiteral("从该 RTP 流建立以来累计发送或接收的数据包数量；接收侧包含重传包。")},
        {QStringLiteral("累计丢包"),
         QStringLiteral("依据 RTP 序号估算的累计丢失包数及比例；重传恢复后仍应结合重传数据判断。")},
        {QStringLiteral("累计重传"),
         QStringLiteral("本次 RTP 流累计发送或接收的 RTX 重传包数和有效负载字节数。")},
        {QStringLiteral("分辨率"),
         QStringLiteral("最近一帧实际编码或解码完成的视频尺寸。")},
        {QStringLiteral("WebRTC当前帧率"),
         QStringLiteral("最近一秒编码或解码完成的帧数；这是媒体吞吐率，不是 Qt 窗口实际 Present 帧率。")},
        {QStringLiteral("DXGI原始采集"),
         QStringLiteral("屏幕采集线程最近采样窗内每秒调用 CaptureFrame 的次数、成功交付的原始桌面帧数，以及观看端要求的采集目标。它位于 WebRTC 自适应和编码之前。")},
        {QStringLiteral("WebRTC源输出"),
         QStringLiteral("WebRTC RTCVideoSourceStats 报告的媒体源帧率。与 DXGI 成功交付帧率之间的差值，通常来自 WebRTC 源适配或调度。")},
        {QStringLiteral("编码完成帧率"),
         QStringLiteral("根据相邻两次 Stats 的 framesEncoded 增量独立计算，表示编码器在该采样窗内每秒实际完成的帧数。")},
        {QStringLiteral("RTP发送帧率"),
         QStringLiteral("根据相邻两次 Stats 的 framesSent 增量独立计算，表示编码完成后实际交给 RTP 发送链路的帧数。")},
        {QStringLiteral("采集调用耗时"),
         QStringLiteral("屏幕采集线程最近一次 CaptureFrame 调用的同步耗时；持续接近或超过目标帧间隔表示采集或显卡驱动可能成为瓶颈。")},
        {QStringLiteral("累计编码帧"),
         QStringLiteral("本次 RTP 发送流成功编码的累计帧数，以及其中的关键帧数量。")},
        {QStringLiteral("累计解码帧"),
         QStringLiteral("本次 RTP 接收流成功解码的累计帧数、关键帧数和解码前或超时丢弃的帧数。")},
        {QStringLiteral("最近一帧 QP"),
         QStringLiteral("最近完成帧的量化参数；数值通常越低画质越高，部分硬件路径不会报告。")},
        {QStringLiteral("累计反馈"),
         QStringLiteral("NACK 请求重传；PLI 请求新关键帧；FIR 强制请求完整帧。三项均为会话累计值。")},
        {QStringLiteral("WebRTC目标码率"),
         QStringLiteral("WebRTC 拥塞控制当前分配给该发送流的目标编码码率。")},
        {QStringLiteral("WebRTC启动码率"),
         QStringLiteral("本应用在分辨率或目标帧率发生变化时写入 PeerConnection 的带宽估计起始值；它会重置当前估计，但仍可被拥塞控制继续升高或降低。")},
        {QStringLiteral("发送策略上限"),
         QStringLiteral("当前观看端为该成员对请求并成功应用的最大输出分辨率和最大帧率；实际值仍可能被 WebRTC 自适应降低。")},
        {QStringLiteral("应用码率上限"),
         QStringLiteral("本应用根据输出分辨率和目标帧率计算后写入 RTP Sender 的码率天花板；它不是 WebRTC 必须跑满的目标码率。")},
        {QStringLiteral("当前媒体 RTT"),
         QStringLiteral("由远端 RTP/RTCP 反馈得到的该媒体流往返时间。")},
        {QStringLiteral("采集源"),
         QStringLiteral("编码前的本地采集分辨率和采集帧率，用于区分采集与编码瓶颈。")},
        {QStringLiteral("最近一帧编码"),
         QStringLiteral("最近一帧从调用 Encode 到编码结果回调的精确流水线时间，包含内部排队，不是纯硬件执行时间。")},
        {QStringLiteral("采样窗平均编码"),
         QStringLiteral("最近采样窗内 totalEncodeTime 增量除以新增编码帧数得到的平均编码流水线时间。")},
        {QStringLiteral("会话平均编码"),
         QStringLiteral("从流建立至今累计编码时间除以累计编码帧数。")},
        {QStringLiteral("采样窗平均 QP"),
         QStringLiteral("最近采样窗内新增帧的平均量化参数；未报告表示当前编解码路径没有提供 QP。")},
        {QStringLiteral("当前质量限制"),
         QStringLiteral("WebRTC 当前降低画质的主要原因，例如带宽、CPU 或其他限制。")},
        {QStringLiteral("网络抖动"),
         QStringLiteral("按 RTP 到达时间计算的包间隔波动，不是 RTT，也不包含解码时间。")},
        {QStringLiteral("采样窗抖动缓冲"),
         QStringLiteral("最近采样窗内，每帧从进入到离开 WebRTC 抖动缓冲区的平均等待时间。")},
        {QStringLiteral("最近一帧解码"),
         QStringLiteral("最近一帧从调用 Decode 到解码帧回调的精确流水线时间，包含异步队列和 MFT 内部排队，不是纯 GPU 执行时间。")},
        {QStringLiteral("输入准备耗时"),
         QStringLiteral("硬件解码器最近一帧从进入 Decode 到 H264 数据复制进 MF 输入样本并准备完成的时间。")},
        {QStringLiteral("输入排队耗时"),
         QStringLiteral("硬件解码器最近一帧从输入样本准备完成，到 MFT 接受 ProcessInput 的等待时间；异步路径包含等待 NeedInput 事件。")},
        {QStringLiteral("MFT输出等待"),
         QStringLiteral("最近一帧从 MFT 接受 ProcessInput 到 ProcessOutput 返回对应输出样本的流水线时间；包含 MFT、驱动、DXVA/GPU 内部排队，不等同于纯 GPU 核心执行时间。")},
        {QStringLiteral("纹理交付耗时"),
         QStringLiteral("最近一帧从 ProcessOutput 返回，到取得 D3D11 NV12 纹理、构造 WebRTC 原生帧并准备调用解码回调的时间。")},
        {QStringLiteral("硬解积压"),
         QStringLiteral("当前等待 MFT 接收的输入帧、已经提交但尚未输出的帧，以及本次硬解实例出现过的峰值总积压；达到 8 帧会完整回退软件解码，不会静默丢弃 H264 参考帧。")},
        {QStringLiteral("采样窗平均解码"),
         QStringLiteral("最近采样窗内 totalDecodeTime 增量除以新增解码帧数；口径是送入解码器到返回完整帧。")},
        {QStringLiteral("会话平均解码"),
         QStringLiteral("从流建立至今累计解码时间除以累计成功解码帧数。")},
        {QStringLiteral("采样窗处理延迟"),
         QStringLiteral("最近采样窗内从一帧首个 RTP 包到达，到完整解码完成的平均时间。")},
        {QStringLiteral("会话平均处理延迟"),
         QStringLiteral("从流建立至今，首个 RTP 包到完整解码完成的历史平均时间。")},
        {QStringLiteral("会话平均抖动缓冲"),
         QStringLiteral("从流建立至今，每帧在 WebRTC 抖动缓冲区中的历史平均等待时间。")},
        {QStringLiteral("累计冻结/暂停"),
         QStringLiteral("冻结是异常长的帧间隔；暂停是超过 5 秒没有新渲染帧。分别显示累计次数和持续时间。")},
        {QStringLiteral("本机显示路径"),
         QStringLiteral("远端解码帧在本机实际采用的显示路径，以及 Qt 从当前显示器读取到的刷新率。D3D11 表示硬解原生 NV12 纹理直达 VideoProcessor；CPU I420/D3D11 表示软件解码的 Y/U/V 三平面直接上传并由 Pixel Shader 转换显示；CPU NV12/D3D11、CPU/D3D11 和 CPU/Qt 依次是兼容回退。")},
        {QStringLiteral("应用帧到达"),
         QStringLiteral("RemoteDesktopCanvas 最近 1 秒实际收到的解码后视频帧数及窗口生命周期累计值。它位于 WebRTC 解码之后、应用显示邮箱之前。")},
        {QStringLiteral("显示提交帧率"),
         QStringLiteral("最近 1 秒首次提交到 D3D11 交换链或首次由 Qt 绘制的新视频帧数及累计值。它是应用能够精确测量的最终显示提交率，不冒充显示器物理扫描率。")},
        {QStringLiteral("应用覆盖丢帧"),
         QStringLiteral("显示线程尚未消费旧帧时，新帧覆盖应用最新帧邮箱，或转换完成后已被更新帧淘汰的累计数量。该值不包含在 WebRTC Stats 的 framesDropped 中。")},
        {QStringLiteral("呈现帧间隔"),
         QStringLiteral("最近 240 个成功呈现帧之间的墙钟间隔：平均值、P95 和最大值。大幅拖动时若最大值或 P95 突增，说明卡顿发生在解码之后的显示调度阶段。")},
        {QStringLiteral("CPU帧格式整理"),
         QStringLiteral("软件显示路径最近一帧和最近 240 帧的 CPU 准备耗时。I420 三平面路径统计 Map、逐行上传 Y/U/V 和 Unmap；兼容回退统计 I420ToNV12 或 I420ToARGB。")},
        {QStringLiteral("I420三平面上传"),
         QStringLiteral("软件解码帧的 Y、U、V 三个平面通过动态 D3D11 纹理环上传所需的 CPU 墙钟时间，包含 Map、逐行复制和 Unmap，不包含 Pixel Shader 执行时间。")},
        {QStringLiteral("Pixel Shader提交"),
         QStringLiteral("I420 三平面显示路径调用 D3D11 Draw 的 CPU 命令提交耗时；YUV 转 RGB 与缩放实际在 GPU Pixel Shader 中执行。")},
        {QStringLiteral("VideoProcessor提交"),
         QStringLiteral("D3D11 原生路径调用 VideoProcessorBlt 的最近一次和最近 240 次 CPU 墙钟耗时。它是驱动命令提交时间，不冒充 GPU 核心执行时间。")},
        {QStringLiteral("Qt绘制提交"),
         QStringLiteral("CPU/Qt 路径第一次为新帧执行 QPainter::drawImage 的最近一次和最近 240 次 CPU 墙钟耗时。")},
        {QStringLiteral("Present调用"),
         QStringLiteral("D3D11 原生路径调用交换链 Present(0,0) 的最近一次和最近 240 次 CPU 墙钟耗时；同步或驱动阻塞会直接反映在这里。")},
        {QStringLiteral("显示失败"),
         QStringLiteral("本次远程画面窗口生命周期内，颜色转换、D3D11 VideoProcessor 或交换链 Present 失败的累计次数。")},
        {QStringLiteral("当前电平/累计隐藏"),
         QStringLiteral("当前音频电平，以及因丢包而由 WebRTC 补偿或隐藏的累计音频样本和事件。")},
        {QStringLiteral("协议"),
         QStringLiteral("当前 DataChannel 使用的上层协议标识；底层传输为 WebRTC SCTP/DTLS。")},
        {QStringLiteral("发送缓冲"),
         QStringLiteral("尚未由 SCTP 发出的 DataChannel 数据量；持续升高表示发送端产生数据快于网络发送。")},
        {QStringLiteral("累计消息"),
         QStringLiteral("该 DataChannel 从建立以来累计发送和接收的消息条数。")},
        {QStringLiteral("累计流量"),
         QStringLiteral("该 DataChannel 从建立以来累计发送和接收的有效负载字节数。")},
    };
    return explanations.value(
        label,
        QStringLiteral("该调试指标的当前实时或累计统计值。"));
}

struct DiagnosticsChip {
    QString key;
    QString label;
    QString value;
    QByteArray tone = "normal";
    bool wide = false;
};

QString DiagnosticsMetricToolTip(const DiagnosticsChip& chip)
{
    return QStringLiteral("%1\n%2")
        .arg(chip.label,
             DiagnosticsMetricExplanation(chip.label));
}

struct DiagnosticsCard {
    QString key;
    QString title;
    QString subtitle;
    QVector<DiagnosticsChip> chips;
};

struct DiagnosticsSection {
    QString key;
    QString title;
    QString description;
    QVector<DiagnosticsCard> cards;
};

class DiagnosticsCardsWidget final : public QWidget {
public:
    explicit DiagnosticsCardsWidget(QWidget* parent = nullptr)
        : QWidget(parent), layout_(new QVBoxLayout(this))
    {
        setObjectName(QStringLiteral("statsCardsHost"));
        setSizePolicy(QSizePolicy::Expanding,
                      QSizePolicy::MinimumExpanding);
        setMinimumHeight(160);
        layout_->setContentsMargins(0, 0, 0, 0);
        layout_->setSpacing(14);
        Rebuild({}, QStringLiteral("正在等待 WebRTC 连接质量数据…"));
        // Force the first real snapshot, including an empty one, to replace
        // the startup placeholder instead of comparing equal to an empty
        // structure key.
        structure_ = QStringLiteral("__startup_placeholder__");
    }

    void SetSections(const QVector<DiagnosticsSection>& sections,
                     const QString& emptyText)
    {
        QStringList structureParts;
        cardCopyTexts_.clear();
        for (const auto& section : sections) {
            structureParts << section.key;
            for (const auto& card : section.cards) {
                structureParts << card.key;
                QStringList copyLines{
                    card.title,
                    card.subtitle.isEmpty()
                        ? QStringLiteral("成员：未报告")
                        : QStringLiteral("成员：%1").arg(card.subtitle)};
                for (const auto& chip : card.chips) {
                    structureParts
                        << QStringLiteral("%1:%2")
                               .arg(chip.key)
                               .arg(chip.wide ? 1 : 0);
                    copyLines << QStringLiteral("%1：%2")
                                     .arg(chip.label, chip.value);
                }
                cardCopyTexts_.insert(
                    section.key + QLatin1Char('/') + card.key,
                    copyLines.join(QLatin1Char('\n')));
            }
        }
        const QString structure = structureParts.join(QLatin1Char('|'));
        if (structure != structure_ || layout_->count() == 0) {
            Rebuild(sections, emptyText);
            structure_ = structure;
        }

        for (const auto& section : sections) {
            for (const auto& card : section.cards) {
                const QString cardPrefix =
                    section.key + QLatin1Char('/') + card.key;
                if (auto* title =
                        titleButtons_.value(cardPrefix + "/title",
                                            nullptr)) {
                    title->setText(card.title);
                }
                if (auto* subtitle =
                        textLabels_.value(cardPrefix + "/subtitle",
                                          nullptr)) {
                    subtitle->setText(card.subtitle);
                    subtitle->setVisible(!card.subtitle.isEmpty());
                }
                for (const auto& chip : card.chips) {
                    const QString chipKey =
                        cardPrefix + QLatin1Char('/') + chip.key;
                    const QString toolTip =
                        DiagnosticsMetricToolTip(chip);
                    if (auto* value =
                            textLabels_.value(chipKey, nullptr)) {
                        value->setText(chip.value);
                        value->setToolTip(toolTip);
                    }
                    if (auto* label =
                            chipNameLabels_.value(chipKey, nullptr)) {
                        label->setToolTip(toolTip);
                    }
                    if (auto* frame =
                            chipFrames_.value(chipKey, nullptr)) {
                        frame->setToolTip(toolTip);
                        if (frame->property("tone").toByteArray() !=
                            chip.tone) {
                            frame->setProperty("tone", chip.tone);
                            frame->style()->unpolish(frame);
                            frame->style()->polish(frame);
                        }
                    }
                }
            }
        }
    }

private:
    void Rebuild(const QVector<DiagnosticsSection>& sections,
                 const QString& emptyText)
    {
        setUpdatesEnabled(false);
        while (auto* item = layout_->takeAt(0)) {
            delete item->widget();
            delete item;
        }
        textLabels_.clear();
        chipNameLabels_.clear();
        chipFrames_.clear();
        titleButtons_.clear();

        if (sections.isEmpty()) {
            auto* empty = new QLabel(emptyText, this);
            empty->setObjectName(QStringLiteral("statsEmptyText"));
            empty->setAlignment(Qt::AlignCenter);
            empty->setMinimumHeight(120);
            layout_->addWidget(empty);
            setUpdatesEnabled(true);
            update();
            return;
        }

        for (const auto& section : sections) {
            auto* sectionHost = new QWidget(this);
            auto* sectionLayout = new QVBoxLayout(sectionHost);
            sectionLayout->setContentsMargins(0, 0, 0, 0);
            sectionLayout->setSpacing(10);

            auto* sectionHeader = new QWidget(sectionHost);
            auto* sectionHeaderLayout = new QHBoxLayout(sectionHeader);
            sectionHeaderLayout->setContentsMargins(2, 4, 2, 2);
            sectionHeaderLayout->setSpacing(10);
            const bool sectionExpanded =
                sectionExpanded_.value(
                    section.key, section.key == QStringLiteral("connection"));
            sectionExpanded_.insert(section.key, sectionExpanded);
            auto* sectionToggle = new QToolButton(sectionHeader);
            sectionToggle->setObjectName(
                QStringLiteral("statsSectionToggle"));
            sectionToggle->setText(section.title);
            sectionToggle->setToolButtonStyle(
                Qt::ToolButtonTextBesideIcon);
            sectionToggle->setArrowType(
                sectionExpanded ? Qt::DownArrow : Qt::RightArrow);
            sectionToggle->setCheckable(true);
            sectionToggle->setChecked(sectionExpanded);
            sectionToggle->setCursor(Qt::PointingHandCursor);
            sectionHeaderLayout->addWidget(sectionToggle);
            auto* sectionDescription =
                new QLabel(section.description, sectionHeader);
            sectionDescription->setObjectName(
                QStringLiteral("statsSectionDescription"));
            sectionHeaderLayout->addWidget(sectionDescription);
            sectionHeaderLayout->addStretch(1);
            auto* count = new QLabel(
                QStringLiteral("%1 项").arg(section.cards.size()),
                sectionHeader);
            count->setObjectName(QStringLiteral("statsCountPill"));
            sectionHeaderLayout->addWidget(count);
            sectionLayout->addWidget(sectionHeader);

            auto* sectionContent = new QWidget(sectionHost);
            auto* sectionContentLayout =
                new QVBoxLayout(sectionContent);
            sectionContentLayout->setContentsMargins(0, 0, 0, 0);
            sectionContentLayout->setSpacing(10);
            sectionContent->setVisible(sectionExpanded);
            connect(
                sectionToggle, &QToolButton::toggled, sectionHost,
                [this, sectionContent, sectionToggle,
                 sectionKey = section.key](bool expanded) {
                    sectionExpanded_.insert(sectionKey, expanded);
                    sectionContent->setVisible(expanded);
                    sectionToggle->setArrowType(
                        expanded ? Qt::DownArrow : Qt::RightArrow);
                    updateGeometry();
                });

            for (const auto& card : section.cards) {
                auto* cardFrame = new QFrame(sectionContent);
                cardFrame->setObjectName(
                    QStringLiteral("statsMetricCard"));
                auto* cardLayout = new QVBoxLayout(cardFrame);
                cardLayout->setContentsMargins(16, 14, 16, 16);
                cardLayout->setSpacing(11);

                auto* cardHeader = new QWidget(cardFrame);
                auto* cardHeaderLayout = new QHBoxLayout(cardHeader);
                cardHeaderLayout->setContentsMargins(0, 0, 0, 0);
                cardHeaderLayout->setSpacing(9);
                const QString cardPrefix =
                    section.key + QLatin1Char('/') + card.key;
                const bool cardExpanded =
                    cardExpanded_.value(cardPrefix, true);
                cardExpanded_.insert(cardPrefix, cardExpanded);
                auto* cardToggle = new QToolButton(cardHeader);
                cardToggle->setObjectName(
                    QStringLiteral("statsCardToggle"));
                cardToggle->setText(card.title);
                cardToggle->setToolButtonStyle(
                    Qt::ToolButtonTextBesideIcon);
                cardToggle->setArrowType(
                    cardExpanded ? Qt::DownArrow : Qt::RightArrow);
                cardToggle->setCheckable(true);
                cardToggle->setChecked(cardExpanded);
                cardToggle->setCursor(Qt::PointingHandCursor);
                cardHeaderLayout->addWidget(cardToggle);
                auto* subtitle =
                    new QLabel(card.subtitle, cardHeader);
                subtitle->setObjectName(
                    QStringLiteral("statsPeerPill"));
                subtitle->setVisible(!card.subtitle.isEmpty());
                cardHeaderLayout->addWidget(subtitle);
                cardHeaderLayout->addStretch(1);
                if (card.title == QStringLiteral("屏幕接收") ||
                    card.title == QStringLiteral("屏幕发送")) {
                    const bool outboundScreen =
                        card.title == QStringLiteral("屏幕发送");
                    auto* copyButton = new QPushButton(
                        QStringLiteral("复制信息"), cardHeader);
                    copyButton->setObjectName(
                        QStringLiteral("softButton"));
                    copyButton->setCursor(Qt::PointingHandCursor);
                    copyButton->setToolTip(
                        outboundScreen
                            ? QStringLiteral(
                                  "复制当前屏幕发送卡片中的全部指标")
                            : QStringLiteral(
                                  "复制当前屏幕接收卡片中的全部指标"));
                    connect(
                        copyButton, &QPushButton::clicked,
                        copyButton,
                        [this, copyButton, cardKey = cardPrefix] {
                            QApplication::clipboard()->setText(
                                cardCopyTexts_.value(cardKey));
                            copyButton->setText(QStringLiteral("已复制"));
                            QTimer::singleShot(
                                1200, copyButton,
                                [copyButton] {
                                    copyButton->setText(
                                        QStringLiteral("复制信息"));
                                });
                        });
                    cardHeaderLayout->addWidget(copyButton);
                }
                cardLayout->addWidget(cardHeader);

                titleButtons_.insert(
                    cardPrefix + "/title", cardToggle);
                textLabels_.insert(
                    cardPrefix + "/subtitle", subtitle);

                auto* chipsHost = new QWidget(cardFrame);
                auto* chips = new QGridLayout(chipsHost);
                chips->setContentsMargins(0, 0, 0, 0);
                chips->setHorizontalSpacing(9);
                chips->setVerticalSpacing(9);
                int row = 0;
                int column = 0;
                for (const auto& chip : card.chips) {
                    if (chip.wide && column != 0) {
                        ++row;
                        column = 0;
                    }
                    auto* chipFrame = new QFrame(cardFrame);
                    chipFrame->setObjectName(
                        QStringLiteral("statsMetricChip"));
                    chipFrame->setProperty("tone", chip.tone);
                    auto* chipLayout =
                        new QHBoxLayout(chipFrame);
                    chipLayout->setContentsMargins(11, 8, 11, 8);
                    chipLayout->setSpacing(7);
                    auto* chipLabel =
                        new QLabel(chip.label, chipFrame);
                    chipLabel->setObjectName(
                        QStringLiteral("statsChipLabel"));
                    auto* chipValue =
                        new QLabel(chip.value, chipFrame);
                    chipValue->setObjectName(
                        QStringLiteral("statsChipValue"));
                    chipValue->setTextInteractionFlags(
                        Qt::TextSelectableByMouse);
                    chipValue->setWordWrap(chip.wide);
                    chipLayout->addWidget(chipLabel);
                    chipLayout->addWidget(chipValue, 1);

                    if (chip.wide) {
                        chips->addWidget(chipFrame, row, 0, 1, 2);
                        ++row;
                        column = 0;
                    } else {
                        chips->addWidget(chipFrame, row, column);
                        if (++column == 2) {
                            ++row;
                            column = 0;
                        }
                    }
                    const QString chipKey =
                        cardPrefix + QLatin1Char('/') + chip.key;
                    textLabels_.insert(chipKey, chipValue);
                    chipNameLabels_.insert(chipKey, chipLabel);
                    chipFrames_.insert(chipKey, chipFrame);
                }
                chips->setColumnStretch(0, 1);
                chips->setColumnStretch(1, 1);
                chipsHost->setVisible(cardExpanded);
                cardLayout->addWidget(chipsHost);
                connect(
                    cardToggle, &QToolButton::toggled, cardFrame,
                    [this, chipsHost, cardToggle,
                     cardKey = cardPrefix](bool expanded) {
                        cardExpanded_.insert(cardKey, expanded);
                        chipsHost->setVisible(expanded);
                        cardToggle->setArrowType(
                            expanded ? Qt::DownArrow : Qt::RightArrow);
                        updateGeometry();
                    });
                sectionContentLayout->addWidget(cardFrame);
            }
            sectionLayout->addWidget(sectionContent);
            layout_->addWidget(sectionHost);
        }
        layout_->addStretch(1);
        setUpdatesEnabled(true);
        layout_->invalidate();
        layout_->activate();
        updateGeometry();
        if (parentWidget()) {
            parentWidget()->updateGeometry();
        }
        update();
    }

    QVBoxLayout* layout_ = nullptr;
    QString structure_;
    QHash<QString, QLabel*> textLabels_;
    QHash<QString, QLabel*> chipNameLabels_;
    QHash<QString, QFrame*> chipFrames_;
    QHash<QString, QToolButton*> titleButtons_;
    QHash<QString, QString> cardCopyTexts_;
    QHash<QString, bool> sectionExpanded_;
    QHash<QString, bool> cardExpanded_;
};

QString InitialFileSaveDirectory()
{
    QString directory = QStandardPaths::writableLocation(
        QStandardPaths::DownloadLocation);
    if (directory.isEmpty()) {
        directory = QDir::homePath();
    }
    return QDir::cleanPath(directory);
}

class CurrentPageStack final : public QStackedWidget {
public:
    using QStackedWidget::QStackedWidget;

    int PageHeightForWidth(QWidget* page) const
    {
        if (!page) {
            return 0;
        }
        page->ensurePolished();
        if (page->layout()) {
            page->layout()->activate();
        }
        int height = page->sizeHint().height();
        if (page->layout() && page->layout()->hasHeightForWidth() &&
            width() > 0) {
            const int heightForWidth = page->layout()->heightForWidth(width());
            if (heightForWidth > 0) {
                height = heightForWidth;
            }
        }
        return std::max(height, page->minimumSizeHint().height());
    }

    void RefreshCurrentHeight()
    {
        QWidget* page = currentWidget();
        if (!page) {
            return;
        }
        const int height = PageHeightForWidth(page);
        if (height > 0 && this->height() != height) {
            setFixedHeight(height);
            updateGeometry();
        }
    }

    QSize sizeHint() const override
    {
        return currentWidget() ? currentWidget()->sizeHint()
                               : QStackedWidget::sizeHint();
    }

    QSize minimumSizeHint() const override
    {
        return currentWidget() ? currentWidget()->minimumSizeHint()
                               : QStackedWidget::minimumSizeHint();
    }

protected:
    void resizeEvent(QResizeEvent* event) override
    {
        const bool widthChanged = event->oldSize().width() != event->size().width();
        QStackedWidget::resizeEvent(event);
        if (widthChanged) {
            QTimer::singleShot(0, this, [this] { RefreshCurrentHeight(); });
        }
    }
};

enum class MediaStateIcon {
    kCamera,
    kMicrophone,
    kSpeaker,
    kScreen
};

class MediaDeviceButton final : public QPushButton {
public:
    explicit MediaDeviceButton(QWidget* parent = nullptr)
        : QPushButton(parent),
          arrowAnimation_(new QVariantAnimation(this))
    {
        arrowAnimation_->setEasingCurve(QEasingCurve::OutCubic);
        connect(
            arrowAnimation_, &QVariantAnimation::valueChanged,
            this, [this](const QVariant& value) {
                arrowRotation_ = value.toReal();
                update();
            });
    }

    void SetDeviceMenuHandler(std::function<void()> handler)
    {
        deviceMenuHandler_ = std::move(handler);
        setProperty("deviceMenu", static_cast<bool>(deviceMenuHandler_));
        update();
    }

protected:
    void mousePressEvent(QMouseEvent* event) override
    {
        if (event && event->button() == Qt::LeftButton &&
            deviceMenuHandler_ &&
            event->position().x() >= width() - kMenuAreaWidth) {
            setDown(false);
            SetDeviceMenuOpen(true);
            deviceMenuHandler_();
            SetDeviceMenuOpen(false);
            event->accept();
            return;
        }
        QPushButton::mousePressEvent(event);
    }

    void paintEvent(QPaintEvent* event) override
    {
        QPushButton::paintEvent(event);
        if (!deviceMenuHandler_) {
            return;
        }
        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing, true);
        painter.setPen(QPen(
            isEnabled() ? QColor(QStringLiteral("#9aa6b7"))
                        : QColor(QStringLiteral("#c5ccd5")),
            1.2));
        const int dividerX = width() - kMenuAreaWidth;
        QPainterPath arrow;
        const qreal centerX =
            dividerX + kMenuAreaWidth / 2.0;
        const qreal centerY = height() / 2.0;
        painter.translate(centerX, centerY);
        painter.rotate(arrowRotation_);
        arrow.moveTo(-4, -2);
        arrow.lineTo(0, 2);
        arrow.lineTo(4, -2);
        painter.drawPath(arrow);
    }

private:
    void SetDeviceMenuOpen(bool open)
    {
        const qreal target = open ? 180.0 : 0.0;
        const int level = CurrentUiAnimationLevel();
        arrowAnimation_->stop();
        if (level <= 0) {
            arrowRotation_ = target;
            update();
            return;
        }
        arrowAnimation_->setDuration(level == 1 ? 80 : 135);
        arrowAnimation_->setStartValue(arrowRotation_);
        arrowAnimation_->setEndValue(target);
        arrowAnimation_->start();
    }

    static constexpr int kMenuAreaWidth = 30;
    std::function<void()> deviceMenuHandler_;
    QVariantAnimation* arrowAnimation_ = nullptr;
    qreal arrowRotation_ = 0.0;
};

QIcon CreateMediaStateIcon(MediaStateIcon type, bool active)
{
    QString iconName;
    switch (type) {
    case MediaStateIcon::kCamera:
        iconName = active ? QStringLiteral("camera")
                          : QStringLiteral("camera-off");
        break;
    case MediaStateIcon::kMicrophone:
        iconName = active ? QStringLiteral("mic")
                          : QStringLiteral("mic-off");
        break;
    case MediaStateIcon::kSpeaker:
        iconName = active ? QStringLiteral("volume-2")
                          : QStringLiteral("volume-x");
        break;
    case MediaStateIcon::kScreen:
        iconName = active ? QStringLiteral("screen-share")
                          : QStringLiteral("screen-share-off");
        break;
    }

    constexpr int kLogicalSize = 24;
    constexpr qreal kBackingScale = 2.0;
    QPixmap pixmap(kLogicalSize * kBackingScale,
                   kLogicalSize * kBackingScale);
    pixmap.setDevicePixelRatio(kBackingScale);
    pixmap.fill(Qt::transparent);
    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing, true);
    QIcon(QStringLiteral(":/ui/icons/lucide/base/%1.svg").arg(iconName))
        .paint(&painter, QRect(0, 0, kLogicalSize, kLogicalSize));
    painter.setCompositionMode(QPainter::CompositionMode_SourceIn);
    const bool dark = ui::RemoteCTheme::IsDark(
        ui::RemoteCTheme::LoadPreference());
    painter.fillRect(
        QRect(0, 0, kLogicalSize, kLogicalSize),
        QColor(active
            ? (dark ? QStringLiteral("#4FF0B5")
                    : QStringLiteral("#168A5B"))
            : (dark ? QStringLiteral("#7F8DA3")
                    : QStringLiteral("#667085"))));
    painter.end();

    QIcon icon;
    icon.addPixmap(pixmap, QIcon::Normal, QIcon::Off);
    icon.addPixmap(pixmap, QIcon::Disabled, QIcon::Off);
    return icon;
}

void SetMediaStateButton(QPushButton* button, MediaStateIcon type,
                         bool active, const QString& toolTip)
{
    if (!button) {
        return;
    }
    button->setText({});
    button->setIcon(CreateMediaStateIcon(type, active));
    button->setIconSize(QSize(25, 25));
    const QString effectiveToolTip =
        button->property("deviceMenu").toBool()
            ? toolTip + QStringLiteral("；点击右侧箭头选择设备")
            : toolTip;
    button->setToolTip(effectiveToolTip);
    button->setAccessibleName(effectiveToolTip);

    QString source;
    QString target;
    switch (type) {
    case MediaStateIcon::kCamera:
        source = QStringLiteral("camera");
        target = QStringLiteral("camera-off");
        break;
    case MediaStateIcon::kMicrophone:
        source = QStringLiteral("mic");
        target = QStringLiteral("mic-off");
        break;
    case MediaStateIcon::kSpeaker:
        source = QStringLiteral("volume-2");
        target = QStringLiteral("volume-x");
        break;
    case MediaStateIcon::kScreen:
        source = QStringLiteral("screen-share");
        target = QStringLiteral("screen-share-off");
        break;
    }
    const bool dark = ui::RemoteCTheme::IsDark(
        ui::RemoteCTheme::LoadPreference());
    auto* morph = remotec::ui::morph::MorphIconButtonBinding::attach(
        button,
        QStringLiteral(":/ui/icons/lucide/base/%1.svg").arg(source),
        QStringLiteral(":/ui/icons/lucide/base/%1.svg").arg(target),
        remotec::ui::morph::MorphIconButtonBinding::Interaction::State,
        QSize(25, 25),
        QColor(dark ? QStringLiteral("#4FF0B5")
                    : QStringLiteral("#168A5B")),
        QColor(dark ? QStringLiteral("#7F8DA3")
                    : QStringLiteral("#667085")));
    if (morph) morph->setTarget(!active);
}

void SetCameraGalleryStateButton(QPushButton* button,
                                 bool camerasAvailable,
                                 bool galleryVisible,
                                 const QString& toolTip)
{
    if (!button) return;
    button->setText({});
    button->setIcon(QIcon(QStringLiteral(
        ":/ui/icons/lucide/base/eye-off.svg")));
    button->setIconSize(QSize(25, 25));
    button->setToolTip(toolTip);
    button->setAccessibleName(toolTip);
    const bool dark = ui::RemoteCTheme::IsDark(
        ui::RemoteCTheme::LoadPreference());
    const QColor stateColor(camerasAvailable
        ? (dark ? QStringLiteral("#4FF0B5")
                : QStringLiteral("#168A5B"))
        : (dark ? QStringLiteral("#7F8DA3")
                : QStringLiteral("#667085")));
    auto* morph = remotec::ui::morph::MorphIconButtonBinding::attach(
        button,
        QStringLiteral(":/ui/icons/lucide/base/eye-off.svg"),
        QStringLiteral(":/ui/icons/lucide/base/eye.svg"),
        remotec::ui::morph::MorphIconButtonBinding::Interaction::State,
        QSize(25, 25), stateColor, stateColor);
    if (morph) morph->setTarget(galleryVisible);
}

QToolButton* CreateMemberMediaIndicator(
    QWidget* parent, MediaStateIcon type, bool active,
    const QString& toolTip, bool hadPreviousState = false,
    bool previousActive = false)
{
    auto* indicator = new QToolButton(parent);
    indicator->setFixedSize(28, 28);
    indicator->setAutoRaise(true);
    indicator->setFocusPolicy(Qt::NoFocus);
    indicator->setAttribute(Qt::WA_TransparentForMouseEvents);
    indicator->setStyleSheet(QStringLiteral(
        "QToolButton{background:transparent;border:none;padding:0;}"));
    indicator->setIcon(CreateMediaStateIcon(type, active));
    indicator->setIconSize(QSize(21, 21));
    indicator->setToolTip(toolTip);
    indicator->setAccessibleName(toolTip);

    QString source;
    QString target;
    switch (type) {
    case MediaStateIcon::kCamera:
        source = QStringLiteral("camera");
        target = QStringLiteral("camera-off");
        break;
    case MediaStateIcon::kMicrophone:
        source = QStringLiteral("mic");
        target = QStringLiteral("mic-off");
        break;
    case MediaStateIcon::kSpeaker:
        source = QStringLiteral("volume-2");
        target = QStringLiteral("volume-x");
        break;
    case MediaStateIcon::kScreen:
        source = QStringLiteral("screen-share");
        target = QStringLiteral("screen-share-off");
        break;
    }
    const bool dark = ui::RemoteCTheme::IsDark(
        ui::RemoteCTheme::LoadPreference());
    auto* morph = remotec::ui::morph::MorphIconButtonBinding::attach(
        indicator,
        QStringLiteral(":/ui/icons/lucide/base/%1.svg").arg(source),
        QStringLiteral(":/ui/icons/lucide/base/%1.svg").arg(target),
        remotec::ui::morph::MorphIconButtonBinding::Interaction::State,
        QSize(21, 21),
        QColor(dark ? QStringLiteral("#4FF0B5")
                    : QStringLiteral("#168A5B")),
        QColor(dark ? QStringLiteral("#7F8DA3")
                    : QStringLiteral("#667085")));
    if (morph) {
        morph->setTarget(!(hadPreviousState ? previousActive : active), false);
        if (hadPreviousState && previousActive != active) {
            QTimer::singleShot(0, indicator, [morph, active] {
                morph->setTarget(!active, true);
            });
        }
    }
    return indicator;
}

enum class RoomStatusIcon {
    kPerson,
    kScreen,
    kController,
    kNetwork,
    kSeats
};

QLabel* CreateRoomStatusIndicator(QWidget* parent, RoomStatusIcon type)
{
    // Draw into a high-density backing store.  A 24x24 bitmap is visibly
    // resampled by Qt on Windows when the desktop scale is above 100%.
    constexpr int kLogicalSize = 24;
    constexpr qreal kBackingScale = 2.0;
    QPixmap pixmap(
        static_cast<int>(kLogicalSize * kBackingScale),
        static_cast<int>(kLogicalSize * kBackingScale));
    pixmap.setDevicePixelRatio(kBackingScale);
    pixmap.fill(Qt::transparent);
    QString iconName;
    switch (type) {
    case RoomStatusIcon::kPerson:
        iconName = QStringLiteral("circle-user-round");
        break;
    case RoomStatusIcon::kScreen:
        iconName = QStringLiteral("monitor");
        break;
    case RoomStatusIcon::kController:
        iconName = QStringLiteral("mouse-pointer-2");
        break;
    case RoomStatusIcon::kNetwork:
        iconName = QStringLiteral("wifi");
        break;
    case RoomStatusIcon::kSeats:
        iconName = QStringLiteral("users-round");
        break;
    }
    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing, true);
    QIcon(QStringLiteral(":/ui/icons/lucide/base/%1.svg").arg(iconName))
        .paint(&painter, QRect(0, 0, kLogicalSize, kLogicalSize));
    painter.setCompositionMode(QPainter::CompositionMode_SourceIn);
    const bool dark = ui::RemoteCTheme::IsDark(
        ui::RemoteCTheme::LoadPreference());
    painter.fillRect(QRect(0, 0, kLogicalSize, kLogicalSize),
                     QColor(dark ? QStringLiteral("#91ADFF")
                                 : QStringLiteral("#5365F5")));
    painter.end();
    auto* label = new QLabel(parent);
    label->setFixedSize(24, 24);
    label->setPixmap(pixmap);
    label->setAlignment(Qt::AlignCenter);
    return label;
}

void SetBusyStatusAnimation(QLabel* label, bool busy)
{
    if (!label) {
        return;
    }
    auto* previous = label->findChild<QPropertyAnimation*>(
        QStringLiteral("remoteCBusyPulse"));
    if (busy && previous && CurrentUiAnimationLevel() > 0) {
        return;
    }
    if (previous) {
        previous->stop();
        previous->deleteLater();
        label->setGraphicsEffect(nullptr);
    }
    if (!busy || CurrentUiAnimationLevel() <= 0) {
        return;
    }
    auto* effect = new QGraphicsOpacityEffect(label);
    label->setGraphicsEffect(effect);
    auto* animation = new QPropertyAnimation(effect, "opacity", label);
    animation->setObjectName(QStringLiteral("remoteCBusyPulse"));
    animation->setDuration(
        CurrentUiAnimationLevel() == 1 ? 760 : 1050);
    animation->setStartValue(1.0);
    animation->setKeyValueAt(0.5, 0.48);
    animation->setEndValue(1.0);
    animation->setEasingCurve(QEasingCurve::InOutSine);
    animation->setLoopCount(-1);
    animation->start();
}

void AnimateSmallUiChange(QWidget* widget)
{
    const int level = CurrentUiAnimationLevel();
    if (!widget || level <= 0) {
        return;
    }
    auto* effect = new QGraphicsOpacityEffect(widget);
    widget->setGraphicsEffect(effect);
    auto* animation = new QPropertyAnimation(effect, "opacity", effect);
    animation->setDuration(level == 1 ? 85 : 135);
    animation->setStartValue(0.45);
    animation->setEndValue(1.0);
    QObject::connect(animation, &QPropertyAnimation::finished, widget,
                     [widget, effect] {
                if (widget->graphicsEffect() == effect) {
                    widget->setGraphicsEffect(nullptr);
                }
            });
    animation->start(QAbstractAnimation::DeleteWhenStopped);
}

constexpr auto kMainStyle = R"(
QMainWindow {
    background: #F5F4F0;
}
QWidget#customTitleBar {
    background: #FAFAF7;
    border-bottom: 1px solid #DDE0E4;
}
QLabel#titleBarAppName {
    color: #172033;
    font-size: 13px;
    font-weight: 700;
}
QLabel#titleBarDivider, QLabel#titleBarTitle {
    color: #718096;
    font-size: 12px;
}
QToolButton#titleBarButton, QToolButton#titleBarCloseButton {
    background: transparent;
    border: none;
    color: #637087;
    font-family: "Segoe UI Symbol";
    font-size: 15px;
}
QToolButton#titleBarButton:hover {
    background: #e9eef6;
    color: #172033;
}
QToolButton#titleBarCloseButton:hover {
    background: #d84a55;
    color: white;
}
QWidget {
    color: #172033;
    font-size: 13px;
}
QFrame#sidebar {
    background: #F8F7F3;
    border-right: 1px solid #DDE0E4;
}
QFrame#navigationIndicator {
    background: #315efb;
    border: none;
    border-radius: 2px;
}
QLabel#brandMark {
    background: #5b6cf9;
    border-radius: 10px;
    color: white;
    font-size: 18px;
    font-weight: 800;
}
QLabel#brandName {
    color: #172033;
    font-size: 18px;
    font-weight: 700;
}
QLabel#brandCaption, QLabel#sidebarCaption {
    color: #8491a6;
    font-size: 11px;
}
QPushButton[nav="true"] {
    background: transparent;
    border: none;
    border-radius: 9px;
    color: #526079;
    font-size: 14px;
    outline: none;
    padding: 11px 14px;
    text-align: left;
}
QPushButton[nav="true"]:hover {
    background: #ECEFEE;
    color: #315efb;
}
QPushButton[navActive="true"] {
    background: #E5E9F7;
    color: #315efb;
    font-weight: 600;
}
QFrame#profileCard {
    background: #F1F2F0;
    border: 1px solid #D9DDE2;
    border-radius: 11px;
}
QFrame#profileCard:hover {
    background: #E9ECEE;
    border-color: #C9CED6;
}
QLabel#profileAvatar {
    background: #315EFB;
    border-radius: 17px;
    color: white;
    font-weight: 700;
}
QLabel#profileName {
    color: #20304a;
    font-weight: 600;
}
QLabel#profileState {
    color: #168a5b;
    font-size: 11px;
}
QFrame#accountMenu {
    background: #FFFEFB;
    border: 1px solid #D9DDE2;
    border-radius: 16px;
}
QWidget#accountMenuHeader { background: transparent; }
QLabel#accountMenuAvatar {
    background: #315EFB;
    border: none;
    border-radius: 19px;
    color: #FFFFFF;
    font-size: 13px;
    font-weight: 750;
}
QLabel#accountMenuName {
    color: #172033;
    font-size: 14px;
    font-weight: 750;
}
QLabel#accountMenuDetail {
    color: #667085;
    font-size: 10px;
}
QFrame#accountMenuSeparator {
    background: #E1E4E8;
    border: none;
}
QPushButton#accountMenuAction {
    background: transparent;
    border: none;
    border-radius: 10px;
    color: #344054;
    font-size: 13px;
    font-weight: 600;
    padding: 0 12px;
    text-align: left;
}
QPushButton#accountMenuAction:hover {
    background: #ECEFEE;
    color: #172033;
}
QPushButton#accountMenuAction[accountHover="true"] {
    background: #ECEFEE;
    color: #172033;
}
QPushButton#accountMenuAction[tone="danger"] { color: #B4232F; }
QPushButton#accountMenuAction[tone="danger"]:hover {
    background: #FFF0F1;
    color: #B4232F;
}
QPushButton#accountMenuAction[tone="danger"][accountHover="true"] {
    background: #FFF0F1;
    color: #B4232F;
}
QLabel#deviceLoginMark {
    background: #5668f6;
    border-radius: 19px;
    color: white;
    font-size: 30px;
    font-weight: 750;
}
QFrame#deviceLoginHero {
    background: #FFFEFB;
    border: 1px solid #DDE0E4;
    border-radius: 22px;
}
QLabel#deviceLoginBrand {
    color: #14203a;
    font-size: 18px;
    font-weight: 750;
}
QLabel#deviceLoginTitle {
    color: #14203a;
    font-size: 27px;
    font-weight: 780;
}
QLabel#deviceLoginStatus {
    color: #68758c;
    font-size: 14px;
}
QLabel#deviceLoginArtwork {
    background: #f1f4ff;
    border: 1px solid #e4e9ff;
    border-radius: 75px;
    color: #6c7df4;
    font-family: "Segoe UI Symbol";
    font-size: 31px;
    font-weight: 700;
}
QPushButton#deviceLoginButton {
    background: #5264f5;
    border: none;
    border-radius: 12px;
    color: white;
    font-size: 16px;
    font-weight: 750;
    padding: 0 30px;
}
QPushButton#deviceLoginButton:hover { background: #4356ec; }
QPushButton#deviceLoginButton:pressed { background: #3749d7; }
QPushButton#deviceLoginButton:disabled {
    background: #aab3df;
    color: #eef0fa;
}
QLabel#deviceLoginSecureNote {
    color: #63718a;
    font-size: 12px;
}
QFrame#deviceLoginFeatures {
    background: #F8F7F3;
    border: 1px solid #E1E3E6;
    border-radius: 15px;
}
QLabel#deviceFeatureIcon {
    background: #e9edff;
    border-radius: 12px;
    color: #5264f5;
    font-family: "Segoe UI Symbol";
    font-size: 20px;
    font-weight: 700;
}
QLabel#deviceFeatureTitle {
    color: #26324a;
    font-size: 13px;
    font-weight: 700;
}
QLabel#deviceFeatureDetail {
    color: #8290a6;
    font-size: 11px;
}
QLabel#pageTitle {
    color: #1d1d1f;
    font-size: 25px;
    font-weight: 750;
}
QLabel#pageSubtitle, QLabel[muted="true"] {
    color: #536176;
}
QFrame[card="true"] {
    background: #FFFEFB;
    border: 1px solid #DDE0E4;
    border-radius: 14px;
}
QMainWindow[themeRoot="light"] QFrame[card="true"]:hover {
    background: #F4F7FF;
    border-color: #AEBFDD;
}
QLabel#cardTitle {
    color: #172033;
    font-size: 16px;
    font-weight: 700;
}
QLabel#sectionTitle {
    color: #172033;
    font-size: 17px;
    font-weight: 700;
}
QLabel#eyebrow {
    color: #007aff;
    font-size: 11px;
    font-weight: 700;
}
QLineEdit#deviceIdInput, QLineEdit#roomIdInput {
    background: #F8F7F3;
    border: 1px solid #DDE0E4;
    border-radius: 10px;
    color: #111827;
    font-size: 16px;
    padding: 0 15px;
    selection-background-color: #007aff;
}
QLineEdit#deviceIdInput:focus, QLineEdit#roomIdInput:focus {
    background: #FFFEFB;
    border: 1px solid #007aff;
}
QLineEdit#assistDeviceIdInput, QLineEdit#assistVerificationInput {
    background: #F8F7F3;
    border: 1px solid #DDE0E4;
    border-radius: 12px;
    color: #172033;
    font-size: 15px;
    padding: 0 16px;
    selection-background-color: #5668f7;
}
QLineEdit#assistVerificationInput {
    background: #F3F2F7;
    border-color: #d9def8;
    color: #3442a4;
    font-size: 16px;
    font-weight: 700;
    letter-spacing: 1px;
}
QLineEdit#assistDeviceIdInput:hover,
QLineEdit#assistVerificationInput:hover {
    background: #FFFEFB;
    border-color: #b9c4dc;
}
QLineEdit#assistDeviceIdInput:focus,
QLineEdit#assistVerificationInput:focus {
    background: #FFFEFB;
    border: 2px solid #5668f7;
    padding: 0 15px;
}
QComboBox#capacitySelector {
    background: #F8F7F3;
    border: 1px solid #DDE0E4;
    border-radius: 9px;
    color: #172033;
    min-width: 88px;
    padding: 8px 34px 8px 11px;
}
QComboBox#capacitySelector::drop-down {
    border: none;
    width: 32px;
}
QComboBox#capacitySelector::down-arrow { image: none; }
QComboBox#capacitySelector:focus {
    background: #FFFEFB;
    border: 1px solid #007aff;
}
QComboBox#capacitySelector:disabled {
    background: #EEEFEF;
    color: #667085;
}
QComboBox#capacitySelector QAbstractItemView {
    background: #FFFEFB;
    border: 1px solid #DDE0E4;
    border-radius: 8px;
    padding: 4px;
    color: #172033;
    selection-background-color: #007aff;
    selection-color: white;
}
QPushButton#primaryButton {
    background: #007aff;
    border: none;
    border-radius: 10px;
    color: white;
    font-size: 14px;
    font-weight: 700;
    padding: 0 22px;
}
QPushButton#primaryButton:hover {
    background: #006ee6;
}
QPushButton#primaryButton:pressed {
    background: #005dcc;
}
QPushButton#primaryButton:disabled {
    background: #D8D5D0;
    color: #667085;
}
QPushButton#softButton {
    background: #eef5ff;
    border: 1px solid #d6e3f7;
    border-radius: 8px;
    color: #006ee6;
    font-weight: 600;
    padding: 8px 13px;
}
QPushButton#softButton:hover {
    background: #e1efff;
}
QPushButton#softButton:disabled {
    background: #EEEFEF;
    color: #667085;
}
QPushButton#screenFrameRateLogButton {
    background: #eef5ff;
    border: none;
    border-radius: 8px;
    color: #006ee6;
    font-weight: 650;
    padding: 8px 13px;
}
QPushButton#screenFrameRateLogButton:hover {
    background: #e1efff;
}
QPushButton#screenFrameRateLogButton:checked {
    background: #1769e8;
    color: #ffffff;
}
QPushButton#mediaIconButton {
    background: #F1F2F0;
    border: 1px solid #D9DDE2;
    border-radius: 9px;
    outline: none;
    padding: 5px;
}
QPushButton#mediaIconButton:hover {
    background: #e9edf4;
    border-color: #d4dae4;
}
QPushButton#mediaIconButton:pressed {
    background: #e0e6ef;
}
QPushButton#mediaIconButton:disabled {
    background: #f6f7f9;
    border-color: #eceef2;
}
QPushButton#dangerButton {
    background: #fff0f1;
    border: 1px solid #f0c5c9;
    border-radius: 8px;
    color: #b4232f;
    font-weight: 650;
    padding: 8px 14px;
}
QPushButton#dangerButton:hover {
    background: #ffe3e5;
}
QPushButton#dangerButton:disabled {
    background: #EEEFEF;
    border-color: #DDE0E4;
    color: #667085;
}
QFrame#deviceTile {
    background: #FAFAF7;
    border: 1px solid #DDE0E4;
    border-radius: 11px;
}
QMainWindow[themeRoot="light"] QFrame#deviceTile:hover {
    background: #F4F7FF;
    border: 1px solid #AEBFDD;
}
QLabel#deviceIcon {
    background: #edf0ff;
    border-radius: 10px;
    color: #5364e8;
    font-size: 18px;
    font-weight: 700;
}
QLabel#deviceName {
    color: #172033;
    font-size: 14px;
    font-weight: 650;
}
QLabel#onlineState {
    color: #169b62;
    font-size: 11px;
}
QLabel#offlineState {
    color: #667085;
    font-size: 11px;
}
QLabel#localId {
    color: #111827;
    font-size: 25px;
    font-weight: 700;
    letter-spacing: 2px;
}
QLabel#passwordValue {
    color: #26324a;
    font-family: "Consolas";
    font-size: 17px;
    font-weight: 700;
    letter-spacing: 3px;
}
QFrame#capabilityBox {
    background: #F8F7F3;
    border: 1px solid #E1E3E6;
    border-radius: 10px;
}
QFrame#settingRow, QFrame#emptyState {
    background: #FFFEFB;
    border: 1px solid #DDE0E4;
    border-radius: 14px;
}
QFrame#aboutCard {
    background: #FAFAF7;
    border: 1px solid #DDE0E4;
    border-radius: 14px;
}
QLabel#aboutLogo {
    background: transparent;
    border: none;
}
QLabel#aboutTitle {
    color: #172033;
    font-size: 17px;
    font-weight: 750;
}
QLabel#aboutVersion {
    background: #e9efff;
    border: 1px solid #d7e1ff;
    border-radius: 9px;
    color: #1769e8;
    font-size: 12px;
    font-weight: 700;
    padding: 3px 9px;
}
QFrame#settingsCategoryPanel {
    background: #F8F7F3;
    border: 1px solid #DDE0E4;
    border-radius: 14px;
}
QPushButton#settingsCategoryButton {
    background: transparent;
    border: none;
    border-radius: 10px;
    color: #5f6b80;
    font-size: 14px;
    font-weight: 600;
    padding: 0 14px;
    text-align: left;
    outline: none;
}
QPushButton#settingsCategoryButton:hover {
    background: #ECEFEE;
    color: #24324b;
}
QPushButton#settingsCategoryButton:checked {
    background: #e9efff;
    color: #1769e8;
    font-weight: 700;
}
QLabel#settingsDetailTitle {
    color: #172033;
    font-size: 21px;
    font-weight: 750;
}
QLabel#settingTitle {
    color: #1d1d1f;
    font-size: 14px;
    font-weight: 650;
}
QLabel#decoderBenchmarkSummary,
QLabel#encoderBenchmarkSummary {
    background: #F8F7F3;
    border: 1px solid #DDE0E4;
    border-radius: 11px;
    color: #344054;
    padding: 13px 15px;
}
QLabel#debugValue {
    color: #25324a;
    font-size: 14px;
    font-weight: 650;
}
QLabel#debugValue[tone="good"] {
    color: #138b57;
}
QLabel#debugValue[tone="warning"] {
    color: #a76508;
}
QLabel#debugValue[tone="error"] {
    color: #c12b37;
}
QLabel#debugValue[tone="muted"] {
    color: #536176;
    font-weight: 500;
}
QLabel#statsSectionTitle {
    color: #172033;
    font-size: 16px;
    font-weight: 750;
}
QLabel#statsSectionDescription {
    color: #53627a;
    font-size: 13px;
}
QLabel#statsCountPill, QLabel#statsPeerPill {
    background: #edf4ff;
    border: 1px solid #dbe8fb;
    border-radius: 9px;
    color: #1769e8;
    font-size: 11px;
    font-weight: 650;
    padding: 3px 8px;
}
QFrame#statsMetricCard {
    background: #FFFEFB;
    border: 1px solid #DDE0E4;
    border-radius: 13px;
}
QLabel#statsCardTitle {
    color: #172033;
    font-size: 15px;
    font-weight: 750;
}
QToolButton#statsSectionToggle, QToolButton#statsCardToggle {
    background: transparent;
    border: none;
    color: #172033;
    font-weight: 750;
    outline: none;
    padding: 3px 2px;
    text-align: left;
}
QToolButton#statsSectionToggle {
    font-size: 16px;
}
QToolButton#statsCardToggle {
    font-size: 15px;
}
QToolButton#statsSectionToggle:hover,
QToolButton#statsCardToggle:hover {
    color: #1769e8;
}
QFrame#statsMetricChip {
    background: #f6f8fb;
    border: 1px solid #e7ebf1;
    border-radius: 10px;
}
QFrame#statsMetricChip[tone="good"] {
    background: #eef9f3;
    border-color: #d5efe1;
}
QFrame#statsMetricChip[tone="warning"] {
    background: #fff8e9;
    border-color: #f4e4b8;
}
QFrame#statsMetricChip[tone="error"] {
    background: #fff1f2;
    border-color: #f4d4d8;
}
QLabel#statsChipLabel {
    color: #34435d;
    font-size: 12px;
    font-weight: 700;
}
QLabel#statsChipValue {
    color: #172033;
    font-size: 13px;
    font-weight: 650;
}
QFrame#statsMetricChip[tone="good"] QLabel#statsChipValue {
    color: #128553;
}
QFrame#statsMetricChip[tone="warning"] QLabel#statsChipValue {
    color: #9b6208;
}
QFrame#statsMetricChip[tone="error"] QLabel#statsChipValue {
    color: #bd2c38;
}
QLabel#statsEmptyText {
    color: #8a95a8;
    font-size: 13px;
}
QLabel#emptyStateIcon {
    background: #eef5ff;
    border-radius: 24px;
    color: #007aff;
    font-size: 21px;
    font-weight: 700;
}
QLabel#emptyStateTitle {
    color: #1d1d1f;
    font-size: 17px;
    font-weight: 700;
}
QFrame#roomMediaBar {
    background: #f7f8fc;
    border: 1px solid #e5e8f1;
    border-radius: 10px;
}
QFrame#roomSectionPanel {
    background: #f7f8fc;
    border: 1px solid #e5e8f1;
    border-radius: 10px;
}
QLabel#roomMediaTitle {
    color: #27334a;
    font-weight: 700;
}
QLabel#readyPill {
    background: #e8f8f0;
    border-radius: 9px;
    color: #138b57;
    font-size: 11px;
    font-weight: 700;
    padding: 4px 9px;
}
QLabel#previewHint {
    background: #fff7e6;
    border: 1px solid #f4ddb0;
    border-radius: 9px;
    color: #9a6513;
    padding: 8px 11px;
}
QLabel#roomIdValue {
    color: #111827;
    font-size: 21px;
    font-weight: 700;
    letter-spacing: 1px;
}
QLabel[roomValue="true"] {
    color: #27334a;
    font-weight: 650;
}
QLabel#roomStageHint {
    background: #eef2ff;
    border: 1px solid #d9defe;
    border-radius: 9px;
    color: #5361c9;
    padding: 9px 11px;
}
QListWidget#roomMemberList {
    background: #f8f9fc;
    border: 1px solid #e3e7ee;
    border-radius: 10px;
    color: #27334a;
    outline: none;
    padding: 5px;
}
QListWidget#roomMemberList::item {
    border-bottom: 1px solid #e8ebf0;
    padding: 0px;
}
QListWidget#roomMemberList::item:last {
    border-bottom: none;
}
QListWidget#roomMemberList::item:selected {
    background: #edf0ff;
    color: #27334a;
}
QScrollArea {
    background: transparent;
    border: none;
}
QFrame#divider {
    color: #e9edf3;
}
QScrollBar:vertical {
    background: transparent;
    width: 8px;
    margin: 2px;
}
QScrollBar::handle:vertical {
    background: #cbd2dd;
    border-radius: 4px;
    min-height: 30px;
}
QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical {
    height: 0;
}
)";

enum class NavigationIcon {
    kRoom,
    kDevice,
    kOwnedDevices,
    kRecent,
    kTransfer,
    kDebug,
    kSettings,
    kHelp,
    kAuthor,
};

QPixmap DrawNavigationIcon(NavigationIcon icon, const QColor& color)
{
    QPixmap pixmap(36, 36);
    pixmap.setDevicePixelRatio(2.0);
    pixmap.fill(Qt::transparent);
    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing);
    QString iconName;
    switch (icon) {
    case NavigationIcon::kRoom:
        iconName = QStringLiteral("users-round");
        break;
    case NavigationIcon::kDevice:
        iconName = QStringLiteral("monitor");
        break;
    case NavigationIcon::kOwnedDevices:
        iconName = QStringLiteral("monitor-smartphone");
        break;
    case NavigationIcon::kRecent:
        iconName = QStringLiteral("rotate-ccw-clock");
        break;
    case NavigationIcon::kTransfer:
        iconName = QStringLiteral("folder-sync");
        break;
    case NavigationIcon::kDebug:
        iconName = QStringLiteral("activity");
        break;
    case NavigationIcon::kSettings:
        iconName = QStringLiteral("settings");
        break;
    case NavigationIcon::kHelp:
        iconName = QStringLiteral("message-circle-question-mark");
        break;
    case NavigationIcon::kAuthor:
        iconName = QStringLiteral("circle-user-round");
        break;
    }
    QIcon(QStringLiteral(":/ui/icons/lucide/base/%1.svg").arg(iconName))
        .paint(&painter, QRect(0, 0, 18, 18));
    painter.setCompositionMode(QPainter::CompositionMode_SourceIn);
    painter.fillRect(QRect(0, 0, 18, 18), color);
    return pixmap;
}

QIcon MakeNavigationIcon(NavigationIcon icon, bool dark)
{
    QIcon result;
    const QColor normal(dark ? QStringLiteral("#AEBBD0")
                             : QStringLiteral("#64748B"));
    const QColor active(dark ? QStringLiteral("#8EA5FF")
                             : QStringLiteral("#315EFB"));
    result.addPixmap(DrawNavigationIcon(icon, normal),
                     QIcon::Normal, QIcon::Off);
    result.addPixmap(DrawNavigationIcon(icon, active),
                     QIcon::Active, QIcon::Off);
    result.addPixmap(DrawNavigationIcon(icon, active),
                     QIcon::Normal, QIcon::On);
    result.addPixmap(DrawNavigationIcon(icon, active),
                     QIcon::Active, QIcon::On);
    return result;
}

QScrollArea* MakePageSurface(const QString& title,
                             const QString& subtitle,
                             QWidget* parent,
                             QVBoxLayout** pageLayout)
{
    auto* scrollArea = new QScrollArea(parent);
    scrollArea->setWidgetResizable(true);
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    EnableSmoothWheelScrolling(scrollArea);
    auto* content = new QWidget(scrollArea);
    auto* layout = new QVBoxLayout(content);
    layout->setContentsMargins(32, 28, 32, 30);
    layout->setSpacing(20);

    auto* header = new QVBoxLayout();
    header->setSpacing(3);
    auto* titleLabel = new QLabel(title, content);
    titleLabel->setObjectName(QStringLiteral("pageTitle"));
    auto* subtitleLabel = new QLabel(subtitle, content);
    subtitleLabel->setObjectName(QStringLiteral("pageSubtitle"));
    subtitleLabel->setWordWrap(true);
    header->addWidget(titleLabel);
    header->addWidget(subtitleLabel);
    layout->addLayout(header);
    scrollArea->setWidget(content);
    *pageLayout = layout;
    return scrollArea;
}

QPushButton* MakeNavigationButton(const QString& text,
                                  NavigationIcon icon,
                                  bool active,
                                  QWidget* parent)
{
    auto* button = new QPushButton(text, parent);
    const bool dark = ui::RemoteCTheme::IsDark(
        ui::RemoteCTheme::LoadPreference());
    button->setIcon(MakeNavigationIcon(icon, dark));
    button->setIconSize(QSize(18, 18));
    button->setCheckable(true);
    button->setChecked(active);
    button->setProperty("nav", true);
    button->setProperty("navActive", active);
    button->setProperty("navigationIcon", static_cast<int>(icon));
    button->setCursor(Qt::PointingHandCursor);
    button->setFixedHeight(43);
    QString source;
    QString target;
    bool enableMorph = true;
    switch (icon) {
    case NavigationIcon::kRoom:
        source = QStringLiteral("user-round");
        target = QStringLiteral("users-round");
        break;
    case NavigationIcon::kDevice:
        source = QStringLiteral("monitor");
        target = QStringLiteral("handshake");
        break;
    case NavigationIcon::kOwnedDevices:
        source = QStringLiteral("monitor-smartphone");
        target = QStringLiteral("monitor-check");
        break;
    case NavigationIcon::kRecent:
        enableMorph = false;
        break;
    case NavigationIcon::kTransfer:
        enableMorph = false;
        break;
    case NavigationIcon::kDebug:
        enableMorph = false;
        break;
    case NavigationIcon::kSettings:
        enableMorph = false;
        break;
    case NavigationIcon::kHelp:
    case NavigationIcon::kAuthor:
        enableMorph = false;
        break;
    }
    if (enableMorph) {
        button->setProperty("navigationMorphSource", source);
        button->setProperty("navigationMorphTarget", target);
        button->setProperty("remoteCMorphCheckedColor",
                            dark ? QStringLiteral("#8EA5FF")
                                 : QStringLiteral("#315EFB"));
        remotec::ui::morph::MorphIconButtonBinding::attach(
            button,
            QStringLiteral(":/ui/icons/lucide/base/%1.svg").arg(source),
            QStringLiteral(":/ui/icons/lucide/base/%1.svg").arg(target),
            remotec::ui::morph::MorphIconButtonBinding::Interaction::Hover,
            QSize(18, 18),
            QColor(dark ? QStringLiteral("#AEBBD0")
                        : QStringLiteral("#64748B")),
            QColor(dark ? QStringLiteral("#8EA5FF")
                        : QStringLiteral("#315EFB")));
    }
    return button;
}

QFrame* MakeDivider(QWidget* parent)
{
    auto* line = new QFrame(parent);
    line->setObjectName(QStringLiteral("divider"));
    line->setFrameShape(QFrame::HLine);
    return line;
}

void AddRoomCapacityItems(QComboBox* comboBox)
{
    for (std::uint32_t capacity = kMinimumRoomMembers;
         capacity <= kProtocolMaximumRoomMembers; ++capacity) {
        comboBox->addItem(QStringLiteral("%1 人").arg(capacity),
                          QVariant::fromValue(capacity));
    }
    const std::uint32_t configuredCapacity = std::clamp(
        QSettings().value(
            QString::fromLatin1(kDefaultRoomCapacitySetting),
            QVariant::fromValue(kDefaultRoomCapacity)).toUInt(),
        kMinimumRoomMembers, kProtocolMaximumRoomMembers);
    comboBox->setCurrentIndex(comboBox->findData(
        QVariant::fromValue(configuredCapacity)));
}

QString MemberDisplayName(const RoomSnapshot& room,
                          const std::string& deviceId,
                          const std::string& localDeviceId)
{
    if (deviceId.empty()) {
        return {};
    }
    const auto member = std::find_if(
        room.members.begin(), room.members.end(),
        [&deviceId](const RoomMemberSnapshot& candidate) {
            return candidate.deviceId == deviceId;
        });
    QString displayName;
    if (member != room.members.end() && !member->deviceName.empty()) {
        displayName = QString::fromStdString(member->deviceName);
    } else {
        displayName = QString::fromStdString(deviceId);
    }
    if (deviceId == localDeviceId) {
        displayName += QStringLiteral("（本机）");
    }
    return displayName;
}

QString ConnectivityDebugName(SessionConnectivityState state)
{
    switch (state) {
    case SessionConnectivityState::kNotConfigured:
        return QStringLiteral("未配置");
    case SessionConnectivityState::kConnecting:
        return QStringLiteral("正在连接");
    case SessionConnectivityState::kOnline:
        return QStringLiteral("在线");
    case SessionConnectivityState::kOffline:
        return QStringLiteral("离线");
    case SessionConnectivityState::kFailed:
        return QStringLiteral("连接失败");
    }
    return QStringLiteral("未知");
}

QString RoomMembershipDebugName(RoomMembershipState state)
{
    switch (state) {
    case RoomMembershipState::kNone:
        return QStringLiteral("未加入房间");
    case RoomMembershipState::kCreating:
        return QStringLiteral("正在创建");
    case RoomMembershipState::kJoinPending:
        return QStringLiteral("等待加入");
    case RoomMembershipState::kActive:
        return QStringLiteral("房间活动中");
    case RoomMembershipState::kLeaving:
        return QStringLiteral("正在离开");
    case RoomMembershipState::kFailed:
        return QStringLiteral("房间操作失败");
    }
    return QStringLiteral("未知");
}

QString PeerConnectionDebugName(RoomPeerConnectionState state)
{
    switch (state) {
    case RoomPeerConnectionState::kStarting:
        return QStringLiteral("正在启动");
    case RoomPeerConnectionState::kNegotiating:
        return QStringLiteral("正在协商");
    case RoomPeerConnectionState::kConnecting:
        return QStringLiteral("正在连接");
    case RoomPeerConnectionState::kActive:
        return QStringLiteral("已连接");
    case RoomPeerConnectionState::kDisconnected:
        return QStringLiteral("网络暂时断开");
    case RoomPeerConnectionState::kRecovering:
        return QStringLiteral("正在恢复连接");
    case RoomPeerConnectionState::kFailed:
        return QStringLiteral("连接失败");
    case RoomPeerConnectionState::kClosed:
        return QStringLiteral("已关闭");
    }
    return QStringLiteral("未知");
}

QString MediaDeviceSelectionDebugName(
    MediaDeviceSelectionState state)
{
    switch (state) {
    case MediaDeviceSelectionState::kReady:
        return QStringLiteral("可用");
    case MediaDeviceSelectionState::kSwitching:
        return QStringLiteral("正在切换");
    case MediaDeviceSelectionState::kUnavailable:
        return QStringLiteral("设备不可用");
    case MediaDeviceSelectionState::kFailed:
        return QStringLiteral("切换失败");
    }
    return QStringLiteral("未知");
}

QString MediaDeviceDebugName(
    const MediaDeviceCategorySnapshot& category,
    const std::string& deviceId)
{
    if (deviceId.empty()) {
        return QStringLiteral("未激活");
    }
    if (deviceId == kSystemDefaultMediaDeviceId) {
        return QStringLiteral("跟随系统默认 [default]");
    }
    const auto device = std::find_if(
        category.devices.begin(), category.devices.end(),
        [&deviceId](const MediaDeviceDescriptor& candidate) {
            return candidate.id == deviceId;
        });
    const QString id = QString::fromStdString(deviceId);
    if (device == category.devices.end() ||
        device->name.empty()) {
        return id;
    }
    return QStringLiteral("%1 [%2]")
        .arg(QString::fromStdString(device->name), id);
}

QString MediaDeviceCategoryDebugText(
    const MediaDeviceCategorySnapshot& category)
{
    QString activeDevice = MediaDeviceDebugName(
        category, category.activeDeviceId);
    if (!category.activeDeviceName.empty()) {
        activeDevice = QString::fromStdString(
            category.activeDeviceName);
        if (category.activeDeviceId ==
            kSystemDefaultMediaDeviceId) {
            activeDevice += QStringLiteral("（系统默认）");
        }
    }
    QString result = QStringLiteral(
        "首选：%1\n活动：%2\n状态：%3")
        .arg(
            MediaDeviceDebugName(
                category, category.preferredDeviceId),
            activeDevice,
            MediaDeviceSelectionDebugName(category.state));
    if (!category.errorMessage.empty()) {
        result += QStringLiteral("\n错误：%1")
                      .arg(QString::fromStdString(
                          category.errorMessage));
    }
    return result;
}

}  // namespace

#include "ControllerMainWindow.Lifecycle.inc"
#include "ControllerMainWindow.Theme.inc"
#include "ControllerMainWindow.MediaMenu.inc"
#include "ControllerMainWindow.WindowLifecycle.inc"
#include "ControllerMainWindow.SoftwareUpdate.inc"
#include "ControllerMainWindow.UiShellRoom.inc"
#include "ControllerMainWindow.UiDeviceRecent.inc"
#include "ControllerMainWindow.UiDiagnostics.inc"
#include "ControllerMainWindow.UiSettings.inc"
#include "ControllerMainWindow.UiConnections.inc"
#include "ControllerMainWindow.NavigationRecent.inc"
#include "ControllerMainWindow.OwnedDevices.inc"
#include "ControllerMainWindow.EngineMedia.inc"
#include "ControllerMainWindow.EncoderBenchmark.inc"
#include "ControllerMainWindow.DecoderBenchmark.inc"
#include "ControllerMainWindow.Session.inc"
#include "ControllerMainWindow.DiagnosticsInput.inc"
#include "ControllerMainWindow.DiagnosticsPeers.inc"
#include "ControllerMainWindow.DiagnosticsOutbound.inc"
#include "ControllerMainWindow.DiagnosticsInbound.inc"
#include "ControllerMainWindow.DiagnosticsPublish.inc"
#include "ControllerMainWindow.SnapshotPreflight.inc"
#include "ControllerMainWindow.SnapshotConnectivity.inc"
#include "ControllerMainWindow.SnapshotLocal.inc"
#include "ControllerMainWindow.SnapshotDebug.inc"
#include "ControllerMainWindow.SnapshotCopy.inc"
#include "ControllerMainWindow.SnapshotWindows.inc"
#include "ControllerMainWindow.RoomUi.inc"
#include "ControllerMainWindow.RoomApprovals.inc"
#include "ControllerMainWindow.RoomMemberActions.inc"
#include "ControllerMainWindow.Clipboard.inc"
#include "ControllerMainWindow.AuxiliaryWindows.inc"
}  // namespace remote::controller
