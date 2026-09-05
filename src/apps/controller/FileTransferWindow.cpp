// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#include "FileTransferWindow.h"

#include <algorithm>

#include <QCloseEvent>
#include <QApplication>
#include <QComboBox>
#include <QDesktopServices>
#include <QDir>
#include <QEasingCurve>
#include <QFileDialog>
#include <QFileInfo>
#include <QFrame>
#include <QGraphicsOpacityEffect>
#include <QHBoxLayout>
#include <QIcon>
#include <QLabel>
#include <QMetaObject>
#include <QMouseEvent>
#include <QProgressBar>
#include <QParallelAnimationGroup>
#include <QPointF>
#include <QPropertyAnimation>
#include <QPushButton>
#include <QResizeEvent>
#include <QSettings>
#include <QStandardPaths>
#include <QScrollArea>
#include <QScrollBar>
#include <QThread>
#include <QTimer>
#include <QUrl>
#include <QVBoxLayout>
#include <QWidget>

#include "RemoteCDialog.h"
#include "RemoteCComboBox.h"
#include "src/apps/controller/ui/RemoteCTheme.h"
#include "src/apps/controller/ui/morph/MorphIconButtonBinding.h"

namespace remote::controller {
namespace {

constexpr auto kDefaultFileSaveDirectorySetting =
    "files/defaultSaveDirectory";
constexpr auto kFileTransferDrawerWidthSetting =
    "files/drawerWidth";
constexpr int kDefaultFileTransferDrawerWidth = 390;
constexpr int kMinimumFileTransferDrawerWidth = 340;
constexpr int kDrawerResizeHandleWidth = 8;

const QEasingCurve& DrawerRevealEasing()
{
    static const QEasingCurve curve = [] {
        QEasingCurve value(QEasingCurve::BezierSpline);
        value.addCubicBezierSegment(
            QPointF(0.25, 0.10), QPointF(0.25, 1.0), QPointF(1.0, 1.0));
        return value;
    }();
    return curve;
}

const QEasingCurve& DrawerDismissEasing()
{
    static const QEasingCurve curve = [] {
        QEasingCurve value(QEasingCurve::BezierSpline);
        value.addCubicBezierSegment(
            QPointF(0.40, 0.0), QPointF(0.60, 1.0), QPointF(1.0, 1.0));
        return value;
    }();
    return curve;
}

QString InitialFileSaveDirectory()
{
    QString directory = QStandardPaths::writableLocation(
        QStandardPaths::DownloadLocation);
    if (directory.isEmpty()) {
        directory = QDir::homePath();
    }
    return QDir::cleanPath(directory);
}

constexpr auto kFileTransferStyle = R"(
QMainWindow { background: #F5F4F0; }
QWidget#customTitleBar {
    background: #FAFAF7;
    border-bottom: 1px solid #DDE0E4;
}
QLabel#titleBarAppName { color: #172033; font-size: 13px; font-weight: 700; }
QLabel#titleBarDivider, QLabel#titleBarTitle { color: #718096; font-size: 12px; }
QToolButton#titleBarButton, QToolButton#titleBarCloseButton {
    background: transparent; border: none; color: #637087;
    font-family: "Segoe UI Symbol"; font-size: 15px;
}
QToolButton#titleBarButton:hover { background: #e9eef6; color: #172033; }
QToolButton#titleBarCloseButton:hover { background: #d84a55; color: white; }
QWidget { color: #172033; font-size: 13px; }
QLabel#pageTitle { color: #111827; font-size: 22px; font-weight: 750; }
QLabel#pageSubtitle, QLabel[muted="true"] { color: #59677C; }
QFrame#sendCard, QFrame#transferCard {
    background: #FFFEFB; border: 1px solid #DDE0E4; border-radius: 13px;
}
QLabel#sectionTitle { color: #172033; font-size: 14px; font-weight: 700; }
QComboBox#peerSelector {
    background: #F8F7F3; border: 1px solid #DDE0E4; border-radius: 9px;
    min-height: 38px; padding: 0 34px 0 11px;
}
QComboBox#peerSelector::drop-down { border: none; width: 32px; }
QComboBox#peerSelector::down-arrow { image: none; }
QComboBox#peerSelector:focus { background: #FFFEFB; border-color: #5b6cf9; }
QComboBox#peerSelector QAbstractItemView {
    background: #FFFEFB; border: 1px solid #DDE0E4; border-radius: 8px;
    padding: 4px;
    selection-background-color: #5b6cf9; selection-color: white;
}
QPushButton#sendButton {
    background: #5b6cf9; border: none; border-radius: 9px; color: white;
    min-height: 40px; font-weight: 700; padding: 0 17px;
}
QPushButton#sendButton:hover { background: #4b5de8; }
QPushButton#sendButton:disabled { background: #aeb5d5; }
QPushButton[action="soft"] {
    background: #eef0ff; border: none; border-radius: 8px;
    color: #4e5fd7; font-weight: 650; padding: 7px 12px;
}
QPushButton[action="soft"]:hover { background: #e2e5ff; }
QPushButton[action="primary"] {
    background: #5b6cf9; border: none; border-radius: 8px;
    color: white; font-weight: 700; padding: 7px 12px;
}
QPushButton[action="primary"]:hover { background: #4b5de8; }
QPushButton[action="danger"] {
    background: #fff1f2; border: 1px solid #f3c9cd; border-radius: 8px;
    color: #b4232f; font-weight: 650; padding: 7px 12px;
}
QPushButton[action="danger"]:hover { background: #ffe4e6; }
QLabel#fileName { color: #172033; font-size: 14px; font-weight: 700; }
QLabel#directionOutgoing {
    background: #edf0ff; border-radius: 8px; color: #5364e8;
    font-size: 11px; font-weight: 700; padding: 4px 8px;
}
QLabel#directionIncoming {
    background: #e9f8f1; border-radius: 8px; color: #17865a;
    font-size: 11px; font-weight: 700; padding: 4px 8px;
}
QLabel#stateActive { color: #5364e8; font-weight: 650; }
QLabel#stateSuccess { color: #138b57; font-weight: 650; }
QLabel#stateError { color: #c23a45; font-weight: 650; }
QProgressBar {
    background: #edf0f5; border: none; border-radius: 3px; height: 6px;
    text-align: center;
}
QProgressBar::chunk { background: #5b6cf9; border-radius: 3px; }
QScrollArea { border: none; background: transparent; }
QWidget#transferViewport { background: transparent; }
QWidget#fileTransferDrawerResizeHandle { background: transparent; }
QWidget#fileTransferDrawerResizeHandle:hover {
    background: rgba(91, 108, 249, 42);
}
QScrollBar:vertical {
    background: transparent; width: 12px; margin: 2px 0 14px 0;
}
QScrollBar::handle:vertical {
    background: #8b8f98; min-height: 30px; border-radius: 5px;
    margin: 1px 2px;
}
QScrollBar::handle:vertical:hover { background: #6f747e; }
QScrollBar::add-page:vertical, QScrollBar::sub-page:vertical {
    background: transparent;
}
QScrollBar::sub-line:vertical { height: 0; }
QScrollBar::add-line:vertical {
    background: transparent; border: none; height: 12px;
    subcontrol-origin: margin; subcontrol-position: bottom;
}
QScrollBar::down-arrow:vertical {
    width: 0; height: 0;
    border-left: 4px solid transparent;
    border-right: 4px solid transparent;
    border-top: 5px solid #777b84;
}
QLabel#emptyState {
    color: #667085; font-size: 13px; padding: 32px 10px;
}
)";

constexpr auto kFileTransferDarkStyle = R"(
QMainWindow,
QWidget#fileTransferRoot,
QWidget#fileTransferBody,
QScrollArea,
QScrollArea > QWidget > QWidget,
QWidget#transferViewport {
    background: #0B1220;
    color: #F1F5FB;
}
QWidget#fileTransferDrawerResizeHandle:hover {
    background: rgba(142, 165, 255, 52);
}
QWidget#customTitleBar {
    background: #0A101B;
    border-bottom-color: #26364A;
}
QLabel#titleBarAppName { color: #F1F5FB; }
QLabel#titleBarDivider, QLabel#titleBarTitle { color: #7F8DA3; }
QToolButton#titleBarButton, QToolButton#titleBarCloseButton { color: #AEBBD0; }
QToolButton#titleBarButton:hover { background: #1A2738; color: #F1F5FB; }
QLabel#pageTitle, QLabel#sectionTitle, QLabel#fileName { color: #F1F5FB; }
QLabel#pageSubtitle, QLabel[muted="true"] { color: #AEBBD0; }
QFrame#sendCard, QFrame#transferCard {
    background: #141E2D;
    border-color: #2B3A50;
}
QComboBox#peerSelector {
    background: #101927;
    border-color: #354861;
    color: #E6EDF7;
}
QComboBox#peerSelector:focus {
    background: #192638;
    border-color: #8EA5FF;
}
QComboBox#peerSelector QAbstractItemView {
    background: #141E2D;
    border-color: #354861;
    color: #E6EDF7;
    selection-background-color: #1D2A4D;
    selection-color: #F8FAFF;
}
QPushButton#sendButton,
QPushButton[action="primary"] {
    background: #4962D7;
    color: #F8FAFF;
}
QPushButton#sendButton:hover,
QPushButton[action="primary"]:hover { background: #526BDE; }
QPushButton#sendButton:disabled {
    background: #202B3B;
    color: #7F8DA3;
}
QPushButton[action="soft"] {
    background: #1A2738;
    color: #8EA5FF;
}
QPushButton[action="soft"]:hover { background: #223249; }
QPushButton[action="danger"] {
    background: #382027;
    border-color: #6C3541;
    color: #FF98A5;
}
QPushButton[action="danger"]:hover { background: #472731; }
QLabel#directionOutgoing {
    background: #1D2A4D;
    color: #8EA5FF;
}
QLabel#directionIncoming {
    background: #162336;
    color: #4FF0B5;
}
QLabel#stateActive { color: #8EA5FF; }
QLabel#stateSuccess { color: #4FF0B5; }
QLabel#stateError { color: #FF98A5; }
QProgressBar { background: #202B3B; }
QProgressBar::chunk { background: #6F86FF; }
QLabel#emptyState {
    background: #141E2D;
    border-color: #2B3A50;
    color: #AEBBD0;
}
QScrollBar::handle:vertical { background: #4A5A70; }
QScrollBar::handle:vertical:hover { background: #60728A; }
)";

QString FormatBytes(std::uint64_t bytes)
{
    constexpr double kUnit = 1024.0;
    if (bytes < 1024) {
        return QStringLiteral("%1 B").arg(bytes);
    }
    if (bytes < 1024 * 1024) {
        return QStringLiteral("%1 KB").arg(bytes / kUnit, 0, 'f', 1);
    }
    if (bytes < std::uint64_t{1024} * 1024 * 1024) {
        return QStringLiteral("%1 MB")
            .arg(bytes / (kUnit * kUnit), 0, 'f', 1);
    }
    return QStringLiteral("%1 GB")
        .arg(bytes / (kUnit * kUnit * kUnit), 0, 'f', 2);
}

QString FormatRemainingTime(std::uint64_t seconds)
{
    if (seconds < 60) {
        return seconds == 0 ? QStringLiteral("不到 1 秒")
                            : QStringLiteral("%1 秒").arg(seconds);
    }
    const auto minutes = seconds / 60;
    if (minutes < 60) {
        return QStringLiteral("%1 分 %2 秒")
            .arg(minutes).arg(seconds % 60);
    }
    return QStringLiteral("%1 小时 %2 分")
        .arg(minutes / 60).arg(minutes % 60);
}

QString StateText(app::FileTransferState state)
{
    switch (state) {
    case app::FileTransferState::kPreparing:
        return QStringLiteral("正在准备");
    case app::FileTransferState::kAwaitingPeerAcceptance:
        return QStringLiteral("等待对方接收");
    case app::FileTransferState::kAwaitingLocalAcceptance:
        return QStringLiteral("等待你确认");
    case app::FileTransferState::kTransferring:
        return QStringLiteral("传输中");
    case app::FileTransferState::kVerifying:
        return QStringLiteral("正在校验");
    case app::FileTransferState::kAwaitingCompletion:
        return QStringLiteral("等待完成确认");
    case app::FileTransferState::kCompleted:
        return QStringLiteral("已完成");
    case app::FileTransferState::kRejected:
        return QStringLiteral("已拒绝");
    case app::FileTransferState::kRecovering:
        return QStringLiteral("网络恢复中");
    case app::FileTransferState::kPaused:
        return QStringLiteral("已暂停");
    case app::FileTransferState::kResuming:
        return QStringLiteral("正在恢复");
    case app::FileTransferState::kCanceled:
        return QStringLiteral("已取消");
    case app::FileTransferState::kFailed:
        return QStringLiteral("传输失败");
    }
    return QStringLiteral("未知状态");
}

QString FileTransferErrorText(const std::string& errorCode,
                              const std::string& errorMessage)
{
    if (errorCode.empty() && errorMessage.empty()) {
        return {};
    }

    // Keep protocol/storage error codes language-neutral. This single UI
    // boundary can be moved to Qt translation catalogs when language switching
    // is introduced without changing the transfer state machine.
    const auto tr = [](const char* text) {
        return QCoreApplication::translate("FileTransferWindow", text);
    };
    const QString code = QString::fromStdString(errorCode);

    if (code == QStringLiteral("user_paused")) {
        return tr("文件传输已暂停，可点击“继续”恢复。");
    }
    if (code == QStringLiteral("user_canceled")) {
        return tr("文件传输已取消。");
    }
    if (code == QStringLiteral("user_rejected")) {
        return tr("接收方拒绝了文件。");
    }
    if (code == QStringLiteral("invalid_progress_offset")) {
        return tr("接收端返回的文件进度无效。");
    }
    if (code == QStringLiteral("resume_not_available")) {
        return tr("当前文件传输无法继续，请重新发送。");
    }
    if (code == QStringLiteral("invalid_resume_offset")) {
        return tr("接收端返回的续传位置无效。");
    }
    if (code == QStringLiteral("file_transfer_resynchronizing")) {
        return tr("传输暂时没有进展，正在与接收端同步续传位置。");
    }
    if (code == QStringLiteral("file_transfer_network_recovering")) {
        return tr("点对点连接暂时中断，正在等待网络恢复。");
    }
    if (code == QStringLiteral("file_transfer_peer_unavailable")) {
        return tr("对方设备已离线，文件传输无法继续。");
    }
    if (code == QStringLiteral("file_transfer_stalled")) {
        return tr("文件传输长时间没有进展，恢复失败。");
    }
    if (code == QStringLiteral("file_transfer_control_backpressure_timeout") ||
        code == QStringLiteral("room_file_transfer_backpressure") ||
        code == QStringLiteral("direct_file_transfer_backpressure")) {
        return tr("文件通道繁忙，请稍后重试。");
    }
    if (code == QStringLiteral("file_transport_unavailable")) {
        return tr("文件传输通道尚未建立，请稍后重试。");
    }
    if (code == QStringLiteral("source_unavailable")) {
        return tr("无法打开要发送的文件。");
    }
    if (code == QStringLiteral("source_read_failed")) {
        return tr("读取要发送的文件时发生错误。");
    }
    if (code == QStringLiteral("destination_unavailable") ||
        code == QStringLiteral("file_destination_invalid")) {
        return tr("无法创建或打开文件保存位置。");
    }
    if (code == QStringLiteral("destination_write_failed")) {
        return tr("写入接收文件时发生错误，请检查磁盘空间和目录权限。");
    }
    if (code == QStringLiteral("integrity_check_failed") ||
        code == QStringLiteral("completion_mismatch") ||
        code == QStringLiteral("incomplete_file")) {
        return tr("文件完整性校验失败，请重新传输。");
    }
    if (code == QStringLiteral("file_send_invalid")) {
        return tr("所选文件无效或无法发送。");
    }
    if (code == QStringLiteral("file_offer_not_pending")) {
        return tr("该文件接收请求已经失效。");
    }
    if (code == QStringLiteral("file_transfer_not_active")) {
        return tr("该文件传输已经结束。");
    }
    if (code == QStringLiteral("file_transfer_not_paused")) {
        return tr("该文件传输当前不在暂停状态。");
    }
    if (code == QStringLiteral("file_transfer_stopping")) {
        return tr("文件传输正在停止，请稍候。");
    }
    if (code == QStringLiteral("duplicate_transfer_id") ||
        code == QStringLiteral("unknown_transfer") ||
        code == QStringLiteral("unexpected_message") ||
        code == QStringLiteral("unexpected_response") ||
        code == QStringLiteral("unexpected_complete") ||
        code == QStringLiteral("invalid_chunk_offset") ||
        code == QStringLiteral("overlapping_chunk_offset") ||
        code == QStringLiteral("sha256_required")) {
        return tr("文件传输协议状态异常，请重新发送。");
    }

    return tr("文件传输发生错误，请重试。错误代码：%1").arg(
        code.isEmpty() ? QStringLiteral("unknown") : code);
}

QString FileTransferErrorText(const app::FileTransferSnapshot& transfer)
{
    return FileTransferErrorText(transfer.errorCode, transfer.errorMessage);
}

}  // namespace

class FileTransferCard final : public QFrame {
public:
    struct Actions {
        std::function<void(const std::string&)> accept;
        std::function<void(const std::string&)> saveAs;
        std::function<void(const std::string&)> reject;
        std::function<void(const std::string&)> cancel;
        std::function<void(const std::string&)> resume;
        std::function<void(const std::filesystem::path&)> openFolder;
    };

    FileTransferCard(Actions actions, QWidget* parent)
        : QFrame(parent), actions_(std::move(actions))
    {
        setObjectName(QStringLiteral("transferCard"));
        auto* cardLayout = new QVBoxLayout(this);
        cardLayout->setContentsMargins(13, 12, 13, 12);
        cardLayout->setSpacing(8);

        auto* header = new QHBoxLayout();
        direction_ = new QLabel(this);
        header->addWidget(direction_);
        fileName_ = new QLabel(this);
        fileName_->setObjectName(QStringLiteral("fileName"));
        fileName_->setTextInteractionFlags(Qt::TextSelectableByMouse);
        header->addWidget(fileName_, 1);
        cardLayout->addLayout(header);

        auto* detail = new QHBoxLayout();
        peer_ = new QLabel(this);
        peer_->setProperty("muted", true);
        detail->addWidget(peer_, 1);
        state_ = new QLabel(this);
        detail->addWidget(state_);
        cardLayout->addLayout(detail);

        progress_ = new QProgressBar(this);
        progress_->setRange(0, 1000);
        progress_->setTextVisible(false);
        progress_->hide();
        progressAnimation_ = new QPropertyAnimation(
            progress_, "value", progress_);
        progressAnimation_->setEasingCurve(QEasingCurve::OutCubic);
        cardLayout->addWidget(progress_);

        rate_ = new QLabel(this);
        rate_->setProperty("muted", true);
        rate_->hide();
        cardLayout->addWidget(rate_);

        auto* footer = new QHBoxLayout();
        size_ = new QLabel(this);
        size_->setProperty("muted", true);
        footer->addWidget(size_, 1);
        actionHost_ = new QWidget(this);
        actionLayout_ = new QHBoxLayout(actionHost_);
        actionLayout_->setContentsMargins(0, 0, 0, 0);
        actionLayout_->setSpacing(6);
        footer->addWidget(actionHost_);
        cardLayout->addLayout(footer);

        error_ = new QLabel(this);
        error_->setWordWrap(true);
        error_->hide();
        cardLayout->addWidget(error_);
    }

    void Update(const app::FileTransferSnapshot& transfer, bool animate)
    {
        const bool firstUpdate = transferId_.empty();
        const bool stateChanged = !firstUpdate && stateValue_ != transfer.state;
        transferId_ = transfer.transferId;

        direction_->setText(
            transfer.direction == app::FileTransferDirection::kOutgoing
                ? QStringLiteral("发送") : QStringLiteral("接收"));
        direction_->setObjectName(
            transfer.direction == app::FileTransferDirection::kOutgoing
                ? QStringLiteral("directionOutgoing")
                : QStringLiteral("directionIncoming"));
        fileName_->setText(QString::fromStdString(
            transfer.fileName.empty() ? std::string("正在读取文件…")
                                      : transfer.fileName));
        fileName_->setToolTip(fileName_->text());
        peer_->setText(QStringLiteral("%1  ·  %2")
            .arg(transfer.direction == app::FileTransferDirection::kOutgoing
                     ? QStringLiteral("发给") : QStringLiteral("来自"),
                 QString::fromStdString(transfer.peerDeviceId)));

        state_->setText(StateText(transfer.state));
        state_->setObjectName(
            transfer.state == app::FileTransferState::kCompleted
                ? QStringLiteral("stateSuccess")
                : (transfer.state == app::FileTransferState::kFailed ||
                           transfer.state == app::FileTransferState::kRejected ||
                           transfer.state == app::FileTransferState::kCanceled
                       ? QStringLiteral("stateError")
                       : QStringLiteral("stateActive")));
        state_->style()->unpolish(state_);
        state_->style()->polish(state_);
        if (stateChanged && animate) {
            AnimateStateLabel();
        }

        const bool showProgress =
            transfer.state != app::FileTransferState::kAwaitingLocalAcceptance &&
            transfer.fileSize > 0;
        progress_->setVisible(showProgress);
        if (showProgress) {
            const int target = transfer.state == app::FileTransferState::kCompleted
                ? 1000
                : static_cast<int>(std::min<std::uint64_t>(
                    1000, transfer.transferredBytes * 1000 / transfer.fileSize));
            progressAnimation_->stop();
            if (animate && !firstUpdate && target > progress_->value()) {
                progressAnimation_->setDuration(
                    CurrentUiAnimationLevel() == 1 ? 95 : 175);
                progressAnimation_->setStartValue(progress_->value());
                progressAnimation_->setEndValue(target);
                progressAnimation_->start();
            } else {
                progress_->setValue(target);
            }
        }

        if (transfer.state == app::FileTransferState::kTransferring &&
            transfer.bytesPerSecond > 0.0) {
            displayedRate_ = displayedRate_ <= 0.0
                ? transfer.bytesPerSecond
                : displayedRate_ * 0.65 + transfer.bytesPerSecond * 0.35;
            rate_->setText(QStringLiteral("%1/s  ·  预计剩余 %2")
                .arg(FormatBytes(static_cast<std::uint64_t>(displayedRate_)),
                     FormatRemainingTime(transfer.estimatedRemainingSeconds)));
            rate_->show();
        } else {
            displayedRate_ = 0.0;
            rate_->hide();
        }

        size_->setText(transfer.fileSize > 0
            ? QStringLiteral("%1 / %2")
                  .arg(FormatBytes(transfer.transferredBytes),
                       FormatBytes(transfer.fileSize))
            : QStringLiteral("0 B"));

        if (firstUpdate || stateChanged || localPath_ != transfer.localPath) {
            localPath_ = transfer.localPath;
            RebuildActions(transfer);
        }

        const QString errorText = FileTransferErrorText(transfer);
        const bool showError = !errorText.isEmpty() &&
            transfer.state != app::FileTransferState::kCompleted;
        error_->setVisible(showError);
        if (showError) {
            error_->setText(errorText);
            error_->setStyleSheet(
                (transfer.state == app::FileTransferState::kRecovering ||
                 transfer.state == app::FileTransferState::kResuming)
                    ? QStringLiteral("color:#a15c00;")
                    : QStringLiteral("color:#b4232f;"));
        }
        stateValue_ = transfer.state;
    }

private:
    QPushButton* AddAction(const QString& text, const char* tone,
                           std::function<void()> callback)
    {
        auto* button = new QPushButton(text, actionHost_);
        QString iconPath;
        if (text == QStringLiteral("接受")) {
            iconPath = QStringLiteral(":/ui/icons/status/success.svg");
        } else if (text == QStringLiteral("另存为")) {
            iconPath = QStringLiteral(":/ui/icons/lucide/base/save.svg");
        } else if (text == QStringLiteral("拒绝")) {
            iconPath = QStringLiteral(":/ui/icons/status/error.svg");
        } else if (text == QStringLiteral("继续")) {
            iconPath = QStringLiteral(":/ui/icons/lucide/base/play.svg");
        } else if (text == QStringLiteral("暂停")) {
            iconPath = QStringLiteral(":/ui/icons/lucide/base/pause.svg");
        } else if (text == QStringLiteral("取消")) {
            iconPath = QStringLiteral(":/ui/icons/lucide/base/x.svg");
        } else if (text == QStringLiteral("打开文件夹")) {
            iconPath = QStringLiteral(
                ":/ui/icons/lucide/base/folder-open.svg");
        }
        if (!iconPath.isEmpty()) {
            button->setIcon(QIcon(iconPath));
            button->setIconSize(QSize(15, 15));
        }
        QString morphSource;
        QString morphTarget;
        if (text == QStringLiteral("接受")) {
            morphSource = QStringLiteral("circle-check-big");
            morphTarget = QStringLiteral("download");
        }
        else if (text == QStringLiteral("另存为")) {
            morphSource = QStringLiteral("save");
            morphTarget = QStringLiteral("folder-open");
        }
        else if (text == QStringLiteral("拒绝")) {
            morphSource = QStringLiteral("circle-x");
            morphTarget = QStringLiteral("trash-2");
        }
        else if (text == QStringLiteral("继续")) {
            morphSource = QStringLiteral("play");
            morphTarget = QStringLiteral("pause");
        }
        else if (text == QStringLiteral("暂停")) {
            morphSource = QStringLiteral("pause");
            morphTarget = QStringLiteral("square");
        }
        else if (text == QStringLiteral("取消")) {
            morphSource = QStringLiteral("x");
            morphTarget = QStringLiteral("circle-x");
        }
        else if (text == QStringLiteral("打开文件夹")) {
            morphSource = QStringLiteral("folder");
            morphTarget = QStringLiteral("folder-open");
        }
        if (!morphSource.isEmpty()) {
            const QColor color =
                text == QStringLiteral("接受")
                ? QColor(QStringLiteral("#FFFFFF"))
                : text == QStringLiteral("拒绝") ||
                text == QStringLiteral("暂停") ||
                text == QStringLiteral("取消")
                ? QColor(QStringLiteral("#D14343"))
                : QColor(QStringLiteral("#2563EB"));
            remotec::ui::morph::MorphIconButtonBinding::attach(
                button,
                QStringLiteral(":/ui/icons/lucide/base/%1.svg")
                    .arg(morphSource),
                QStringLiteral(":/ui/icons/lucide/base/%1.svg")
                    .arg(morphTarget),
                remotec::ui::morph::MorphIconButtonBinding::Interaction::Hover,
                QSize(15, 15), color, color);
        }
        button->setProperty("action", tone);
        button->setCursor(Qt::PointingHandCursor);
        connect(button, &QPushButton::clicked, this,
                [this, callback = std::move(callback)] {
                    actionHost_->setEnabled(false);
                    callback();
                    QTimer::singleShot(800, actionHost_, [this] {
                        if (actionHost_) {
                            actionHost_->setEnabled(true);
                        }
                    });
                });
        actionLayout_->addWidget(button);
        return button;
    }

    void RebuildActions(const app::FileTransferSnapshot& transfer)
    {
        while (QLayoutItem* item = actionLayout_->takeAt(0)) {
            delete item->widget();
            delete item;
        }
        actionHost_->setEnabled(true);
        const std::string id = transfer.transferId;
        if (transfer.state == app::FileTransferState::kAwaitingLocalAcceptance) {
            AddAction(QStringLiteral("接受"), "primary",
                      [actions = actions_, id] { actions.accept(id); });
            AddAction(QStringLiteral("另存为"), "soft",
                      [actions = actions_, id] { actions.saveAs(id); });
            AddAction(QStringLiteral("拒绝"), "danger",
                      [actions = actions_, id] { actions.reject(id); });
        } else if (transfer.state == app::FileTransferState::kPaused) {
            AddAction(QStringLiteral("继续"), "soft",
                      [actions = actions_, id] { actions.resume(id); });
        } else if (transfer.state == app::FileTransferState::kTransferring) {
            AddAction(QStringLiteral("暂停"), "danger",
                      [actions = actions_, id] { actions.cancel(id); });
        } else if (transfer.state == app::FileTransferState::kPreparing ||
                   transfer.state == app::FileTransferState::kAwaitingPeerAcceptance ||
                   transfer.state == app::FileTransferState::kResuming ||
                   transfer.state == app::FileTransferState::kRecovering) {
            AddAction(QStringLiteral("取消"), "danger",
                      [actions = actions_, id] { actions.cancel(id); });
        } else if (transfer.state == app::FileTransferState::kCompleted &&
                   !transfer.localPath.empty()) {
            const auto path = transfer.localPath;
            AddAction(QStringLiteral("打开文件夹"), "soft",
                      [actions = actions_, path] { actions.openFolder(path); });
        }
        actionHost_->setVisible(actionLayout_->count() > 0);
    }

    void AnimateStateLabel()
    {
        auto* effect = new QGraphicsOpacityEffect(state_);
        state_->setGraphicsEffect(effect);
        auto* animation = new QPropertyAnimation(effect, "opacity", effect);
        animation->setDuration(CurrentUiAnimationLevel() == 1 ? 90 : 145);
        animation->setStartValue(0.35);
        animation->setEndValue(1.0);
        connect(animation, &QPropertyAnimation::finished, state_,
                [this, effect] {
                    if (state_->graphicsEffect() == effect) {
                        state_->setGraphicsEffect(nullptr);
                    }
                });
        animation->start(QAbstractAnimation::DeleteWhenStopped);
    }

    Actions actions_;
    std::string transferId_;
    std::filesystem::path localPath_;
    app::FileTransferState stateValue_ = app::FileTransferState::kPreparing;
    double displayedRate_ = 0.0;
    QLabel* direction_ = nullptr;
    QLabel* fileName_ = nullptr;
    QLabel* peer_ = nullptr;
    QLabel* state_ = nullptr;
    QProgressBar* progress_ = nullptr;
    QPropertyAnimation* progressAnimation_ = nullptr;
    QLabel* rate_ = nullptr;
    QLabel* size_ = nullptr;
    QWidget* actionHost_ = nullptr;
    QHBoxLayout* actionLayout_ = nullptr;
    QLabel* error_ = nullptr;
};

FileTransferWindow::FileTransferWindow(
    app::FileTransferController* controller,
    QWidget* parent)
    : FramelessMainWindow(parent, true), controller_(controller)
{
    BuildUi();
    if (controller_) {
        controller_->SetObserver(this);
    }
}

FileTransferWindow::~FileTransferWindow()
{
    DetachController();
}

void FileTransferWindow::DetachController()
{
    if (!controller_) {
        return;
    }
    controller_->SetObserver(nullptr);
    controller_ = nullptr;
}

void FileTransferWindow::SyncPeers(
    const std::vector<FileTransferPeer>& peers,
    const QRect& mainWindowGeometry)
{
    anchorGeometry_ = mainWindowGeometry;
    const QString selected = peerSelector_->currentData().toString();
    peerSelector_->clear();
    for (const auto& peer : peers) {
        peerSelector_->addItem(
            peer.displayName,
            QString::fromStdString(peer.deviceId));
    }
    const int previous = peerSelector_->findData(selected);
    if (previous >= 0) {
        peerSelector_->setCurrentIndex(previous);
    }
    const bool hasPeer = peerSelector_->count() > 0;
    sendButton_->setEnabled(hasPeer);
    peerHint_->setText(
        hasPeer ? QStringLiteral("文件通过当前设备间 P2P 直传")
                : QStringLiteral("等待可用的 P2P 设备连接"));
}

void FileTransferWindow::AttachAsDrawer(QWidget* host)
{
    if (!host || drawerHost_ == host) {
        return;
    }
    StopDrawerAnimation();
    hide();
    setParent(host);
    setWindowFlags(Qt::Widget);
    setAttribute(Qt::WA_StyledBackground, true);
    drawerHost_ = host;
    UpdateDrawerGeometry();
}

int FileTransferWindow::EffectiveDrawerWidth() const
{
    if (!drawerHost_) {
        return std::max(kMinimumFileTransferDrawerWidth, drawerWidth_);
    }
    const int hostWidth = std::max(1, drawerHost_->width());
    const int minimumWidth =
        std::min(kMinimumFileTransferDrawerWidth, hostWidth);
    const int maximumWidth = std::max(
        minimumWidth, hostWidth - std::min(240, hostWidth / 3));
    return std::clamp(drawerWidth_, minimumWidth, maximumWidth);
}

void FileTransferWindow::SetDrawerWidth(int requestedWidth)
{
    drawerWidth_ = std::max(1, requestedWidth);
    if (!drawerHost_) {
        resize(drawerWidth_, height());
        return;
    }
    const int width = EffectiveDrawerWidth();
    resize(width, drawerHost_->height());
    move(isVisible() && !drawerHiding_
             ? drawerHost_->width() - width
             : drawerHost_->width(),
         0);
}

void FileTransferWindow::OpenBesideMainWindow(
    const QRect& mainWindowGeometry)
{
    anchorGeometry_ = mainWindowGeometry;
    if (!drawerHost_) {
        return;
    }
    const int drawerWidth = EffectiveDrawerWidth();
    resize(drawerWidth, drawerHost_->height());
    const QPoint shownPosition(drawerHost_->width() - drawerWidth, 0);
    const bool startOutside = !isVisible();
    StopDrawerAnimation();
    drawerHiding_ = false;
    if (startOutside) {
        move(drawerHost_->width(), 0);
        show();
    }
    raise();

    if (CurrentUiAnimationLevel() <= 0 || pos() == shownPosition) {
        move(shownPosition);
        return;
    }
    StartDrawerMotion(
        shownPosition, CurrentUiAnimationLevel() == 1 ? 165 : 210);
}

void FileTransferWindow::HideWithAnimation()
{
    if (!isVisible() || !drawerHost_) {
        return;
    }
    StopDrawerAnimation();
    drawerHiding_ = true;
    const QPoint hiddenPosition(drawerHost_->width(), 0);
    if (CurrentUiAnimationLevel() <= 0) {
        HideImmediately();
        return;
    }
    StartDrawerMotion(
        hiddenPosition, CurrentUiAnimationLevel() == 1 ? 145 : 185);
}

void FileTransferWindow::HideImmediately()
{
    StopDrawerAnimation();
    drawerHiding_ = false;
    hide();
    if (drawerHost_) {
        move(drawerHost_->width(), 0);
    }
}

void FileTransferWindow::UpdateDrawerGeometry()
{
    if (!drawerHost_) {
        return;
    }
    const bool wasVisible = isVisible();
    StopDrawerAnimation();
    const int drawerWidth = EffectiveDrawerWidth();
    resize(drawerWidth, drawerHost_->height());
    move(wasVisible && !drawerHiding_
             ? drawerHost_->width() - drawerWidth
             : drawerHost_->width(),
         0);
    if (drawerHiding_) {
        hide();
        drawerHiding_ = false;
    }
}

bool FileTransferWindow::IsHiding() const
{
    return drawerHiding_;
}

void FileTransferWindow::StartDrawerMotion(const QPoint& targetPosition,
                                           int durationMs)
{
    drawerMotionStart_ = pos();
    drawerMotionTarget_ = targetPosition;
    drawerMotionDurationMs_ = std::max(1, durationMs);
    drawerMotionClock_.restart();

    if (!drawerMotionTimer_) {
        drawerMotionTimer_ = new QTimer(this);
        drawerMotionTimer_->setTimerType(Qt::PreciseTimer);
        drawerMotionTimer_->setInterval(8);
        connect(drawerMotionTimer_, &QTimer::timeout, this, [this] {
            if (drawerMotionDurationMs_ <= 0) {
                drawerMotionTimer_->stop();
                return;
            }
            const qreal progress = std::min<qreal>(
                1.0,
                static_cast<qreal>(drawerMotionClock_.elapsed()) /
                    drawerMotionDurationMs_);
            const qreal eased = (drawerHiding_
                ? DrawerDismissEasing()
                : DrawerRevealEasing()).valueForProgress(progress);
            const QPoint delta = drawerMotionTarget_ - drawerMotionStart_;
            move(drawerMotionStart_ + QPoint(
                qRound(delta.x() * eased), qRound(delta.y() * eased)));
            if (progress < 1.0) return;

            drawerMotionTimer_->stop();
            move(drawerMotionTarget_);
            drawerMotionDurationMs_ = 0;
            if (drawerHiding_) {
                hide();
                drawerHiding_ = false;
            }
        });
    }
    drawerMotionTimer_->start();
}

void FileTransferWindow::StopDrawerAnimation()
{
    if (drawerMotionTimer_) drawerMotionTimer_->stop();
    drawerMotionDurationMs_ = 0;
}

void FileTransferWindow::closeEvent(QCloseEvent* event)
{
    HideWithAnimation();
    event->ignore();
}

void FileTransferWindow::resizeEvent(QResizeEvent* event)
{
    FramelessMainWindow::resizeEvent(event);
    if (drawerResizeHandle_) {
        drawerResizeHandle_->setGeometry(
            0, 0, kDrawerResizeHandleWidth, height());
        drawerResizeHandle_->raise();
    }
}

bool FileTransferWindow::eventFilter(QObject* watched, QEvent* event)
{
    if (watched != drawerResizeHandle_ || !event) {
        return FramelessMainWindow::eventFilter(watched, event);
    }
    if (event->type() == QEvent::MouseButtonPress) {
        auto* mouse = static_cast<QMouseEvent*>(event);
        if (mouse->button() == Qt::LeftButton) {
            drawerResizeDragging_ = true;
            drawerResizeStartGlobalX_ =
                mouse->globalPosition().toPoint().x();
            drawerResizeStartWidth_ = width();
            drawerResizeHandle_->grabMouse();
            return true;
        }
    } else if (event->type() == QEvent::MouseMove &&
               drawerResizeDragging_) {
        const auto* mouse = static_cast<QMouseEvent*>(event);
        const int delta = drawerResizeStartGlobalX_ -
            mouse->globalPosition().toPoint().x();
        SetDrawerWidth(drawerResizeStartWidth_ + delta);
        return true;
    } else if ((event->type() == QEvent::MouseButtonRelease ||
                event->type() == QEvent::UngrabMouse) &&
               drawerResizeDragging_) {
        drawerResizeDragging_ = false;
        if (drawerResizeHandle_->mouseGrabber() ==
            drawerResizeHandle_) {
            drawerResizeHandle_->releaseMouse();
        }
        drawerWidth_ = width();
        QSettings().setValue(
            QString::fromLatin1(kFileTransferDrawerWidthSetting),
            drawerWidth_);
        return true;
    }
    return FramelessMainWindow::eventFilter(watched, event);
}

void FileTransferWindow::OnFileTransfersChanged(
    const std::vector<app::FileTransferSnapshot>& transfers)
{
    bool schedule = false;
    {
        std::lock_guard lock(pendingTransfersMutex_);
        pendingTransfers_ = transfers;
        if (!transferApplyScheduled_) {
            transferApplyScheduled_ = true;
            schedule = true;
        }
    }
    if (!schedule) {
        return;
    }
    auto startTimer = [this] { ScheduleTransferApply(); };
    if (QThread::currentThread() == thread()) {
        startTimer();
    } else {
        QMetaObject::invokeMethod(this, std::move(startTimer),
                                  Qt::QueuedConnection);
    }
}

void FileTransferWindow::ScheduleTransferApply()
{
    QTimer::singleShot(200, this, [this] {
        if (QApplication::mouseButtons() != Qt::NoButton) {
            ScheduleTransferApply();
            return;
        }
        std::vector<app::FileTransferSnapshot> latest;
        {
            std::lock_guard lock(pendingTransfersMutex_);
            latest = std::move(pendingTransfers_);
            transferApplyScheduled_ = false;
        }
        ApplyTransfers(std::move(latest));
    });
}

void FileTransferWindow::BuildUi()
{
    setWindowTitle(QStringLiteral("RLink - 文件传输"));
    setMinimumSize(320, 500);
    drawerWidth_ = std::max(
        kMinimumFileTransferDrawerWidth,
        QSettings().value(
            QString::fromLatin1(kFileTransferDrawerWidthSetting),
            kDefaultFileTransferDrawerWidth).toInt());
    resize(drawerWidth_, 640);
    ApplyUiStyleSheet(QString::fromUtf8(kFileTransferStyle));

    auto* central = new QWidget(this);
    central->setObjectName(QStringLiteral("fileTransferRoot"));
    auto* outer = new QVBoxLayout(central);
    outer->setContentsMargins(0, 0, 0, 0);
    outer->setSpacing(0);
    titleBar_ = new CustomTitleBar(
        this, QStringLiteral("文件传输"), central);
    titleBar_->SetEmbeddedMode(true);
    outer->addWidget(titleBar_);

    auto* body = new QWidget(central);
    body->setObjectName(QStringLiteral("fileTransferBody"));
    auto* bodyLayout = new QVBoxLayout(body);
    bodyLayout->setContentsMargins(20, 18, 20, 18);
    bodyLayout->setSpacing(14);
    auto* title = new QLabel(QStringLiteral("文件传输"), body);
    title->setObjectName(QStringLiteral("pageTitle"));
    bodyLayout->addWidget(title);
    auto* subtitle = new QLabel(
        QStringLiteral("选择设备，文件不会经过信令服务器"), body);
    subtitle->setObjectName(QStringLiteral("pageSubtitle"));
    bodyLayout->addWidget(subtitle);

    auto* sendCard = new QFrame(body);
    sendCard->setObjectName(QStringLiteral("sendCard"));
    auto* sendLayout = new QVBoxLayout(sendCard);
    sendLayout->setContentsMargins(14, 13, 14, 14);
    sendLayout->setSpacing(9);
    auto* sendTitle = new QLabel(QStringLiteral("发送给"), sendCard);
    sendTitle->setObjectName(QStringLiteral("sectionTitle"));
    sendLayout->addWidget(sendTitle);
    auto* sendRow = new QHBoxLayout();
    sendRow->setSpacing(9);
    peerSelector_ = new RemoteCComboBox(sendCard);
    peerSelector_->setObjectName(QStringLiteral("peerSelector"));
    peerSelector_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    sendRow->addWidget(peerSelector_, 1);
    sendButton_ = new QPushButton(QStringLiteral("选择文件"), sendCard);
    sendButton_->setIcon(
        QIcon(QStringLiteral(":/ui/icons/lucide/base/file-up.svg")));
    sendButton_->setIconSize(QSize(17, 17));
    remotec::ui::morph::MorphIconButtonBinding::attach(
        sendButton_,
        QStringLiteral(":/ui/icons/lucide/base/file-up.svg"),
        QStringLiteral(":/ui/icons/lucide/base/folder-open.svg"),
        remotec::ui::morph::MorphIconButtonBinding::Interaction::Hover,
        QSize(17, 17), QColor(QStringLiteral("#FFFFFF")),
        QColor(QStringLiteral("#FFFFFF")));
    sendButton_->setObjectName(QStringLiteral("sendButton"));
    sendButton_->setCursor(Qt::PointingHandCursor);
    sendButton_->setEnabled(false);
    sendRow->addWidget(sendButton_);
    sendLayout->addLayout(sendRow);
    peerHint_ = new QLabel(
        QStringLiteral("等待可用的 P2P 设备连接"), sendCard);
    peerHint_->setProperty("muted", true);
    sendLayout->addWidget(peerHint_);
    bodyLayout->addWidget(sendCard);

    auto* taskTitle = new QLabel(QStringLiteral("传输任务"), body);
    taskTitle->setObjectName(QStringLiteral("sectionTitle"));
    bodyLayout->addWidget(taskTitle);

    transferScroll_ = new QScrollArea(body);
    transferScroll_->setWidgetResizable(true);
    transferScroll_->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    EnableSmoothWheelScrolling(transferScroll_);
    transferContainer_ = new QWidget(transferScroll_);
    transferContainer_->setObjectName(QStringLiteral("transferViewport"));
    transferLayout_ = new QVBoxLayout(transferContainer_);
    transferLayout_->setContentsMargins(0, 0, 0, 0);
    transferLayout_->setSpacing(10);
    emptyState_ = new QLabel(
        QStringLiteral("暂无传输任务\n选择成员并发送一个文件"),
        transferContainer_);
    emptyState_->setObjectName(QStringLiteral("emptyState"));
    emptyState_->setAlignment(Qt::AlignCenter);
    transferLayout_->addWidget(emptyState_);
    transferLayout_->addStretch(1);
    transferScroll_->setWidget(transferContainer_);
    bodyLayout->addWidget(transferScroll_, 1);
    outer->addWidget(body, 1);
    setCentralWidget(central);

    drawerResizeHandle_ = new QWidget(this);
    drawerResizeHandle_->setObjectName(
        QStringLiteral("fileTransferDrawerResizeHandle"));
    drawerResizeHandle_->setCursor(Qt::SizeHorCursor);
    drawerResizeHandle_->setMouseTracking(true);
    drawerResizeHandle_->installEventFilter(this);
    drawerResizeHandle_->setGeometry(
        0, 0, kDrawerResizeHandleWidth, height());
    drawerResizeHandle_->raise();

    connect(sendButton_, &QPushButton::clicked,
            this, [this] { ChooseAndSendFile(); });
    RefreshThemeStyle();
}

void FileTransferWindow::RefreshThemeStyle()
{
    const bool dark = ui::RemoteCTheme::IsDark(
        ui::RemoteCTheme::LoadPreference());
    setProperty("themeRoot", dark ? QStringLiteral("dark")
                                   : QStringLiteral("light"));
    ApplyUiStyleSheet(QString::fromUtf8(kFileTransferStyle) +
                      (dark ? QString::fromUtf8(kFileTransferDarkStyle)
                            : QString()));
    if (titleBar_) titleBar_->RefreshThemeStyle(dark);
    if (auto* selector = dynamic_cast<RemoteCComboBox*>(peerSelector_)) {
        selector->RefreshThemeStyle();
    }
}

void FileTransferWindow::ApplyTransfers(
    std::vector<app::FileTransferSnapshot> transfers)
{
    std::stable_sort(
        transfers.begin(), transfers.end(),
        [](const auto& left, const auto& right) {
            return left.displayOrder > right.displayOrder;
        });
    bool hasNewIncoming = false;
    for (const auto& transfer : transfers) {
        if (transfer.direction == app::FileTransferDirection::kIncoming &&
            transfer.state ==
                app::FileTransferState::kAwaitingLocalAcceptance) {
            const QString id = QString::fromStdString(transfer.transferId);
            if (!announcedIncomingTransfers_.contains(id)) {
                announcedIncomingTransfers_.insert(id);
                hasNewIncoming = true;
            }
        }
    }
    transfers_ = std::move(transfers);
    UpdateTransferCardsIncrementally();
    if (hasNewIncoming) {
        if (drawerHost_ || !anchorGeometry_.isNull()) {
            OpenBesideMainWindow(anchorGeometry_);
        } else {
            show();
            raise();
            activateWindow();
        }
    }
}

void FileTransferWindow::UpdateTransferCardsIncrementally()
{
    const int scrollPosition = transferScroll_
        ? transferScroll_->verticalScrollBar()->value() : 0;
    while (QLayoutItem* item = transferLayout_->takeAt(0)) {
        // Widgets are persistent and are reinserted below. Only delete the
        // transient layout item (including the final stretch).
        delete item;
    }

    if (transfers_.empty()) {
        emptyState_->show();
        transferLayout_->addWidget(emptyState_);
        transferLayout_->addStretch(1);
        const auto staleCards = transferCards_.values();
        transferCards_.clear();
        for (auto* card : staleCards) {
            card->deleteLater();
        }
        return;
    }
    emptyState_->hide();

    const int animationLevel = CurrentUiAnimationLevel();
    QSet<QString> desired;
    for (const auto& transfer : transfers_) {
        const QString id = QString::fromStdString(transfer.transferId);
        desired.insert(id);
        FileTransferCard* card = transferCards_.value(id, nullptr);
        const bool isNew = card == nullptr;
        if (!card) {
            FileTransferCard::Actions actions;
            actions.accept = [this](const std::string& transferId) {
                AcceptTransfer(transferId);
            };
            actions.saveAs = [this](const std::string& transferId) {
                SaveTransferAs(transferId);
            };
            actions.reject = [this](const std::string& transferId) {
                RejectTransfer(transferId);
            };
            actions.cancel = [this](const std::string& transferId) {
                CancelTransfer(transferId);
            };
            actions.resume = [this](const std::string& transferId) {
                ResumeTransfer(transferId);
            };
            actions.openFolder = [this](const std::filesystem::path& path) {
                OpenTransferFolder(path);
            };
            card = new FileTransferCard(std::move(actions), transferContainer_);
            transferCards_.insert(id, card);
        }
        card->Update(transfer, animationLevel > 0);
        transferLayout_->addWidget(card);

        if (isNew && animationLevel > 0) {
            card->setMaximumHeight(0);
            auto* effect = new QGraphicsOpacityEffect(card);
            effect->setOpacity(0.0);
            card->setGraphicsEffect(effect);
            QTimer::singleShot(0, card, [card, effect, animationLevel] {
                const int targetHeight = std::max(1, card->sizeHint().height());
                auto* group = new QParallelAnimationGroup(card);
                auto* height = new QPropertyAnimation(
                    card, "maximumHeight", group);
                height->setDuration(animationLevel == 1 ? 105 : 175);
                height->setStartValue(0);
                height->setEndValue(targetHeight);
                height->setEasingCurve(QEasingCurve::OutCubic);
                group->addAnimation(height);
                auto* opacity = new QPropertyAnimation(
                    effect, "opacity", group);
                opacity->setDuration(animationLevel == 1 ? 90 : 150);
                opacity->setStartValue(0.0);
                opacity->setEndValue(1.0);
                group->addAnimation(opacity);
                QObject::connect(
                    group, &QParallelAnimationGroup::finished, card,
                    [card, effect] {
                        card->setMaximumHeight(QWIDGETSIZE_MAX);
                        if (card->graphicsEffect() == effect) {
                            card->setGraphicsEffect(nullptr);
                        }
                    });
                group->start(QAbstractAnimation::DeleteWhenStopped);
            });
        }
    }

    const auto knownIds = transferCards_.keys();
    for (const auto& id : knownIds) {
        if (desired.contains(id)) {
            continue;
        }
        FileTransferCard* card = transferCards_.take(id);
        if (!card) {
            continue;
        }
        if (animationLevel <= 0) {
            card->deleteLater();
            continue;
        }
        transferLayout_->addWidget(card);
        auto* effect = new QGraphicsOpacityEffect(card);
        card->setGraphicsEffect(effect);
        auto* group = new QParallelAnimationGroup(card);
        auto* opacity = new QPropertyAnimation(effect, "opacity", group);
        opacity->setDuration(animationLevel == 1 ? 80 : 130);
        opacity->setStartValue(1.0);
        opacity->setEndValue(0.0);
        group->addAnimation(opacity);
        auto* height = new QPropertyAnimation(card, "maximumHeight", group);
        height->setDuration(animationLevel == 1 ? 90 : 145);
        height->setStartValue(card->height());
        height->setEndValue(0);
        group->addAnimation(height);
        connect(group, &QParallelAnimationGroup::finished,
                card, &QObject::deleteLater);
        group->start(QAbstractAnimation::DeleteWhenStopped);
    }
    transferLayout_->addStretch(1);
    if (transferScroll_) {
        transferScroll_->verticalScrollBar()->setValue(scrollPosition);
    }
}

void FileTransferWindow::RebuildTransferCards()
{
    const int scrollPosition = transferScroll_
        ? transferScroll_->verticalScrollBar()->value()
        : 0;
    transferContainer_->setUpdatesEnabled(false);
    while (QLayoutItem* item = transferLayout_->takeAt(0)) {
        if (QWidget* widget = item->widget()) {
            delete widget;
        }
        delete item;
    }
    if (transfers_.empty()) {
        emptyState_ = new QLabel(
            QStringLiteral("暂无传输任务\n选择成员并发送一个文件"),
            transferContainer_);
        emptyState_->setObjectName(QStringLiteral("emptyState"));
        emptyState_->setAlignment(Qt::AlignCenter);
        transferLayout_->addWidget(emptyState_);
        transferLayout_->addStretch(1);
        transferContainer_->setUpdatesEnabled(true);
        transferContainer_->update();
        return;
    }

    for (const auto& transfer : transfers_) {
        auto* card = new QFrame(transferContainer_);
        card->setObjectName(QStringLiteral("transferCard"));
        auto* cardLayout = new QVBoxLayout(card);
        cardLayout->setContentsMargins(13, 12, 13, 12);
        cardLayout->setSpacing(8);

        auto* header = new QHBoxLayout();
        auto* direction = new QLabel(
            transfer.direction == app::FileTransferDirection::kOutgoing
                ? QStringLiteral("发送")
                : QStringLiteral("接收"),
            card);
        direction->setObjectName(
            transfer.direction == app::FileTransferDirection::kOutgoing
                ? QStringLiteral("directionOutgoing")
                : QStringLiteral("directionIncoming"));
        header->addWidget(direction);
        auto* fileName = new QLabel(
            QString::fromStdString(transfer.fileName.empty()
                                       ? std::string("正在读取文件…")
                                       : transfer.fileName),
            card);
        fileName->setObjectName(QStringLiteral("fileName"));
        fileName->setTextInteractionFlags(Qt::TextSelectableByMouse);
        fileName->setToolTip(fileName->text());
        header->addWidget(fileName, 1);
        cardLayout->addLayout(header);

        auto* detailRow = new QHBoxLayout();
        auto* peer = new QLabel(
            QStringLiteral("%1  ·  %2")
                .arg(transfer.direction ==
                             app::FileTransferDirection::kOutgoing
                         ? QStringLiteral("发给")
                         : QStringLiteral("来自"),
                     QString::fromStdString(transfer.peerDeviceId)),
            card);
        peer->setProperty("muted", true);
        detailRow->addWidget(peer, 1);
        auto* state = new QLabel(StateText(transfer.state), card);
        state->setObjectName(
            transfer.state == app::FileTransferState::kCompleted
                ? QStringLiteral("stateSuccess")
                : (transfer.state == app::FileTransferState::kFailed ||
                           transfer.state ==
                               app::FileTransferState::kRejected ||
                           transfer.state ==
                               app::FileTransferState::kCanceled
                       ? QStringLiteral("stateError")
                       : QStringLiteral("stateActive")));
        detailRow->addWidget(state);
        cardLayout->addLayout(detailRow);

        const bool showProgress =
            transfer.state !=
                app::FileTransferState::kAwaitingLocalAcceptance &&
            transfer.fileSize > 0;
        if (showProgress) {
            auto* progress = new QProgressBar(card);
            progress->setRange(0, 1000);
            progress->setTextVisible(false);
            const int value = static_cast<int>(std::min<std::uint64_t>(
                1000, transfer.transferredBytes * 1000 /
                          transfer.fileSize));
            progress->setValue(
                transfer.state == app::FileTransferState::kCompleted
                    ? 1000
                    : value);
            cardLayout->addWidget(progress);
        }

        if (transfer.state == app::FileTransferState::kTransferring &&
            transfer.bytesPerSecond > 0.0) {
            auto* rate = new QLabel(
                QStringLiteral("%1/s  ·  预计剩余 %2")
                    .arg(FormatBytes(static_cast<std::uint64_t>(
                             transfer.bytesPerSecond)),
                         FormatRemainingTime(
                             transfer.estimatedRemainingSeconds)),
                card);
            rate->setProperty("muted", true);
            cardLayout->addWidget(rate);
        }

        auto* footer = new QHBoxLayout();
        const QString sizeText =
            transfer.fileSize > 0
                ? QStringLiteral("%1 / %2")
                      .arg(FormatBytes(transfer.transferredBytes),
                           FormatBytes(transfer.fileSize))
                : QStringLiteral("0 B");
        auto* size = new QLabel(sizeText, card);
        size->setProperty("muted", true);
        footer->addWidget(size, 1);

        if (transfer.state ==
            app::FileTransferState::kAwaitingLocalAcceptance) {
            auto* accept = new QPushButton(QStringLiteral("接受"), card);
            accept->setProperty("action", "primary");
            accept->setCursor(Qt::PointingHandCursor);
            connect(accept, &QPushButton::clicked, this,
                    [this, id = transfer.transferId] {
                        AcceptTransfer(id);
                    });
            footer->addWidget(accept);
            auto* saveAs = new QPushButton(QStringLiteral("另存为"), card);
            saveAs->setProperty("action", "soft");
            saveAs->setCursor(Qt::PointingHandCursor);
            connect(saveAs, &QPushButton::clicked, this,
                    [this, id = transfer.transferId] {
                        SaveTransferAs(id);
                    });
            footer->addWidget(saveAs);
            auto* reject = new QPushButton(QStringLiteral("拒绝"), card);
            reject->setProperty("action", "danger");
            reject->setCursor(Qt::PointingHandCursor);
            connect(reject, &QPushButton::clicked, this,
                    [this, id = transfer.transferId] {
                        RejectTransfer(id);
                    });
            footer->addWidget(reject);
        } else if (transfer.state == app::FileTransferState::kPaused) {
            auto* resume = new QPushButton(QStringLiteral("继续"), card);
            resume->setProperty("action", "soft");
            resume->setCursor(Qt::PointingHandCursor);
            connect(resume, &QPushButton::clicked, this,
                    [this, id = transfer.transferId] {
                        ResumeTransfer(id);
                    });
            footer->addWidget(resume);
        } else if (transfer.state == app::FileTransferState::kTransferring) {
            auto* cancel = new QPushButton(QStringLiteral("暂停"), card);
            cancel->setProperty("action", "danger");
            cancel->setCursor(Qt::PointingHandCursor);
            connect(cancel, &QPushButton::clicked, this,
                    [this, id = transfer.transferId] {
                        CancelTransfer(id);
                    });
            footer->addWidget(cancel);
        } else if (transfer.state == app::FileTransferState::kPreparing ||
                   transfer.state ==
                       app::FileTransferState::kAwaitingPeerAcceptance ||
                   transfer.state == app::FileTransferState::kResuming ||
                   transfer.state == app::FileTransferState::kRecovering) {
            auto* cancel = new QPushButton(QStringLiteral("取消"), card);
            cancel->setProperty("action", "danger");
            cancel->setCursor(Qt::PointingHandCursor);
            connect(cancel, &QPushButton::clicked, this,
                    [this, id = transfer.transferId] {
                        CancelTransfer(id);
                    });
            footer->addWidget(cancel);
        } else if (transfer.state ==
                       app::FileTransferState::kCompleted &&
                   !transfer.localPath.empty()) {
            auto* open = new QPushButton(QStringLiteral("打开文件夹"), card);
            open->setProperty("action", "soft");
            open->setCursor(Qt::PointingHandCursor);
            connect(open, &QPushButton::clicked, this,
                    [this, path = transfer.localPath] {
                        OpenTransferFolder(path);
                    });
            footer->addWidget(open);
        }
        cardLayout->addLayout(footer);

        const QString errorText = FileTransferErrorText(transfer);
        if (!errorText.isEmpty() &&
            transfer.state != app::FileTransferState::kCompleted) {
            auto* error = new QLabel(errorText, card);
            error->setWordWrap(true);
            error->setStyleSheet(
                (transfer.state == app::FileTransferState::kRecovering ||
                 transfer.state == app::FileTransferState::kResuming)
                    ? QStringLiteral("color:#a15c00;")
                    : QStringLiteral("color:#b4232f;"));
            cardLayout->addWidget(error);
        }
        transferLayout_->addWidget(card);
    }
    transferLayout_->addStretch(1);
    transferContainer_->setUpdatesEnabled(true);
    if (transferScroll_) {
        transferScroll_->verticalScrollBar()->setValue(scrollPosition);
    }
    transferContainer_->update();
}

void FileTransferWindow::ChooseAndSendFile()
{
    const QString peer = peerSelector_->currentData().toString();
    if (peer.isEmpty() || !controller_) {
        return;
    }
    const QString file = QFileDialog::getOpenFileName(
        this, QStringLiteral("选择要发送的文件"));
    if (file.isEmpty()) {
        return;
    }
    const auto result = controller_->SendFile(
        peer.toStdString(), std::filesystem::path(file.toStdWString()));
    if (!result.accepted) {
        RemoteCDialog::Alert(
            this, QStringLiteral("无法发送文件"),
            FileTransferErrorText(result.errorCode, result.errorMessage));
    }
}

void FileTransferWindow::AcceptTransfer(const std::string& transferId)
{
    QSettings settings;
    const QString directory = QDir::cleanPath(settings.value(
        QString::fromLatin1(kDefaultFileSaveDirectorySetting),
        InitialFileSaveDirectory()).toString());
    if (directory.isEmpty() || !QDir(directory).exists()) {
        RemoteCDialog::Alert(
            this, QStringLiteral("默认存储路径不可用"),
            QStringLiteral(
                "请先在主窗口的“设置”中选择有效的文件默认存储路径。"));
        return;
    }
    AcceptTransferToDirectory(transferId, directory);
}

void FileTransferWindow::SaveTransferAs(const std::string& transferId)
{
    QSettings settings;
    const QString initialDirectory = settings.value(
        QString::fromLatin1(kDefaultFileSaveDirectorySetting),
        InitialFileSaveDirectory()).toString();
    const QString directory = QFileDialog::getExistingDirectory(
        this, QStringLiteral("选择另存为位置"), initialDirectory);
    if (directory.isEmpty()) {
        return;
    }
    AcceptTransferToDirectory(transferId, directory);
}

void FileTransferWindow::AcceptTransferToDirectory(
    const std::string& transferId,
    const QString& directory)
{
    if (directory.isEmpty() || !controller_) {
        return;
    }
    const auto transfer = std::find_if(
        transfers_.begin(), transfers_.end(),
        [&transferId](const auto& current) {
            return current.transferId == transferId;
        });
    if (transfer == transfers_.end()) {
        return;
    }
    const QString targetPath = QDir(directory).filePath(
        QString::fromUtf8(transfer->fileName));
    bool replaceExisting = false;
    if (QFileInfo::exists(targetPath)) {
        const bool replace = RemoteCDialog::Confirm(
            this, QStringLiteral("文件已存在"),
            QStringLiteral("该路径已经存在同名文件：\n%1\n\n是否继续？继续后将在新文件接收并校验成功后覆盖原文件。")
                .arg(QDir::toNativeSeparators(targetPath)),
            QStringLiteral("继续并覆盖"), QStringLiteral("取消"),
            RemoteCDialog::Tone::kDanger);
        if (!replace) {
            return;
        }
        replaceExisting = true;
    }
    const auto result = controller_->AcceptIncoming(
        transferId, std::filesystem::path(directory.toStdWString()),
        replaceExisting);
    if (!result.accepted) {
        RemoteCDialog::Alert(
            this, QStringLiteral("无法接收文件"),
            FileTransferErrorText(result.errorCode, result.errorMessage));
    }
}

void FileTransferWindow::RejectTransfer(const std::string& transferId)
{
    if (!controller_) {
        return;
    }
    const auto result = controller_->RejectIncoming(transferId);
    if (!result.accepted) {
        RemoteCDialog::Alert(
            this, QStringLiteral("无法拒绝文件"),
            FileTransferErrorText(result.errorCode, result.errorMessage));
    }
}

void FileTransferWindow::CancelTransfer(const std::string& transferId)
{
    if (!controller_) {
        return;
    }
    const auto result = controller_->Cancel(transferId);
    if (!result.accepted) {
        RemoteCDialog::Alert(
            this, QStringLiteral("无法取消传输"),
            FileTransferErrorText(result.errorCode, result.errorMessage));
    }
}

void FileTransferWindow::ResumeTransfer(const std::string& transferId)
{
    if (!controller_) {
        return;
    }
    const auto result = controller_->Resume(transferId);
    if (!result.accepted) {
        RemoteCDialog::Alert(
            this, QStringLiteral("无法继续传输"),
            FileTransferErrorText(result.errorCode, result.errorMessage));
    }
}

void FileTransferWindow::OpenTransferFolder(
    const std::filesystem::path& path)
{
    const auto directory = path.has_filename() ? path.parent_path() : path;
    QDesktopServices::openUrl(QUrl::fromLocalFile(
        QString::fromStdWString(directory.wstring())));
}

}  // namespace remote::controller
