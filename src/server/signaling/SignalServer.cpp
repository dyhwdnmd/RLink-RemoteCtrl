// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#include "SignalServer.h"

#include <algorithm>
#include <array>
#include <cstdio>
#include <memory>
#include <unordered_map>
#include <utility>

#ifdef _WIN32
#include <io.h>
#include <windows.h>
#else
#include <unistd.h>
#endif

#include <QAbstractSocket>
#include <QDateTime>
#include <QElapsedTimer>
#include <QFile>
#include <QHash>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QPointer>
#include <QQueue>
#include <QRandomGenerator>
#include <QSet>
#include <QSslCertificate>
#include <QSslConfiguration>
#include <QSslKey>
#include <QSslSocket>
#include <QTextStream>
#include <QTimer>
#include <QUuid>
#include <QWebSocket>
#include <QWebSocketProtocol>
#include <QWebSocketServer>

#include "AccessTokenService.h"
#include "SignalServerSupport.h"
#include "SlidingWindowRateLimiter.h"
#include "src/server/auth/LogtoWebhookServer.h"
#include "src/server/auth/LogtoManagementClient.h"
#include "src/server/auth/LogtoUserInfoClient.h"
#include "src/server/persistence/IdentityStore.h"

namespace remote::signaling_server {
namespace {

constexpr int kProtocolVersion = 5;
constexpr quint64 kMaximumMessageBytes = 1024 * 1024;
constexpr int kMaximumRememberedMessageIds = 4096;
constexpr int kMaximumSessions = 4096;
constexpr int kMaximumRooms = 4096;
constexpr qsizetype kInitialStateReserve = 2048;
constexpr int kMaximumPermissions = 16;
constexpr int kProtocolMaximumRoomMembers = 5;
constexpr int kRoomJoinRequestTimeoutMs = 30000;
constexpr int kRoomControlRequestTimeoutMs = 30000;
constexpr int kRoomScreenShareSwitchRequestTimeoutMs = 30000;
constexpr qint64 kRateLimitWindowMs = 60000;
constexpr int kRateLimitMaximumKeys = 4096;
constexpr SlidingWindowRateLimitPolicy kAvailabilityIpRateLimit{
    120, kRateLimitWindowMs, kRateLimitMaximumKeys};
constexpr SlidingWindowRateLimitPolicy kAvailabilityUserRateLimit{
    80, kRateLimitWindowMs, kRateLimitMaximumKeys};
constexpr SlidingWindowRateLimitPolicy kAvailabilityDeviceRateLimit{
    60, kRateLimitWindowMs, kRateLimitMaximumKeys};
constexpr SlidingWindowRateLimitPolicy kRoomJoinIpRateLimit{
    20, kRateLimitWindowMs, kRateLimitMaximumKeys};
constexpr SlidingWindowRateLimitPolicy kRoomJoinUserRateLimit{
    12, kRateLimitWindowMs, kRateLimitMaximumKeys};
constexpr SlidingWindowRateLimitPolicy kRoomJoinDeviceRateLimit{
    8, kRateLimitWindowMs, kRateLimitMaximumKeys};

bool StdoutIsTerminal()
{
#ifdef _WIN32
    return _isatty(_fileno(stdout)) != 0;
#else
    return isatty(fileno(stdout)) != 0;
#endif
}

void ClearStdoutTerminal()
{
#ifdef _WIN32
    const HANDLE output = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_SCREEN_BUFFER_INFO info{};
    if (output == INVALID_HANDLE_VALUE ||
        !GetConsoleScreenBufferInfo(output, &info)) {
        return;
    }
    const DWORD cells = static_cast<DWORD>(info.dwSize.X) *
                        static_cast<DWORD>(info.dwSize.Y);
    const COORD origin{0, 0};
    DWORD written = 0;
    FillConsoleOutputCharacterW(output, L' ', cells, origin, &written);
    FillConsoleOutputAttribute(output, info.wAttributes, cells, origin,
                               &written);
    SetConsoleCursorPosition(output, origin);
#else
    QTextStream(stdout) << "\x1b[2J\x1b[H";
#endif
}

QString FormatBytes(qint64 bytes)
{
    static constexpr std::array<const char*, 4> units{
        "B", "KiB", "MiB", "GiB"};
    double value = static_cast<double>(std::max<qint64>(0, bytes));
    std::size_t unit = 0;
    while (value >= 1024.0 && unit + 1 < units.size()) {
        value /= 1024.0;
        ++unit;
    }
    const int precision = unit == 0 ? 0 : 2;
    return QStringLiteral("%1 %2")
        .arg(QString::number(value, 'f', precision),
             QString::fromLatin1(units[unit]));
}

QString FormatBitRate(double bitsPerSecond)
{
    static constexpr std::array<const char*, 4> units{
        "bps", "Kbps", "Mbps", "Gbps"};
    double value = std::max(0.0, bitsPerSecond);
    std::size_t unit = 0;
    while (value >= 1000.0 && unit + 1 < units.size()) {
        value /= 1000.0;
        ++unit;
    }
    const int precision = unit == 0 ? 0 : 2;
    return QStringLiteral("%1 %2")
        .arg(QString::number(value, 'f', precision),
             QString::fromLatin1(units[unit]));
}

QString FormatDuration(qint64 milliseconds)
{
    const qint64 totalSeconds = std::max<qint64>(0, milliseconds / 1000);
    const qint64 days = totalSeconds / 86400;
    const qint64 hours = (totalSeconds % 86400) / 3600;
    const qint64 minutes = (totalSeconds % 3600) / 60;
    const qint64 seconds = totalSeconds % 60;
    if (days > 0) {
        return QStringLiteral("%1天 %2小时 %3分")
            .arg(days).arg(hours).arg(minutes);
    }
    if (hours > 0) {
        return QStringLiteral("%1小时 %2分 %3秒")
            .arg(hours).arg(minutes).arg(seconds);
    }
    return QStringLiteral("%1分 %2秒").arg(minutes).arg(seconds);
}
constexpr SlidingWindowRateLimitPolicy kAccountDeletionUserRateLimit{
    3, 60 * 60 * 1000, kRateLimitMaximumKeys};
constexpr SlidingWindowRateLimitPolicy kAssistanceIpRateLimit{
    30, kRateLimitWindowMs, kRateLimitMaximumKeys};
constexpr SlidingWindowRateLimitPolicy kAssistanceRequesterRateLimit{
    8, kRateLimitWindowMs, kRateLimitMaximumKeys};
constexpr SlidingWindowRateLimitPolicy kAssistanceTargetRateLimit{
    20, kRateLimitWindowMs, kRateLimitMaximumKeys};

void SetError(QString* error, const QString& message)
{
    if (error) {
        *error = message;
    }
}

bool IsValidPurpose(const QString& purpose)
{
    return purpose == QStringLiteral("remote_control") ||
           purpose == QStringLiteral("camera_only");
}

bool IsNineDigitPublicId(const QString& value)
{
    return value.size() == 9 && value.front() != QChar('0') &&
           std::all_of(value.cbegin(), value.cend(),
                       [](QChar character) { return character.isDigit(); });
}

QString GenerateNineDigitPublicId()
{
    constexpr quint32 kFirstPublicId = 100000000;
    constexpr quint32 kPublicIdCount = 900000000;
    return QString::number(
        kFirstPublicId +
        QRandomGenerator::system()->bounded(kPublicIdCount));
}

}  // namespace

using detail::CreateRecoveryToken;
using detail::ReadFile;

class SignalServer::Impl final {
public:
#include "SignalServer.Lifecycle.inc"
#include "SignalServer.Authentication.inc"
#include "SignalServer.AccountManagement.inc"
#include "SignalServer.Connection.inc"
#include "SignalServer.OwnedDevices.inc"
#include "SignalServer.RoomMembership.inc"
#include "SignalServer.RoomLeases.inc"
#include "SignalServer.RoomPairs.inc"
#include "SignalServer.RoomState.inc"
#include "SignalServer.LegacySession.inc"
#include "SignalServer.Relay.inc"
#include "SignalServer.Recovery.inc"
    bool RememberMessageId(ClientState* client, const QByteArray& messageId)
    {
        if (client->receivedMessageIds.contains(messageId)) {
            return false;
        }
        client->receivedMessageIds.insert(messageId);
        client->receivedMessageOrder.enqueue(messageId);
        ++totalRememberedMessageIds_;
        if (client->receivedMessageOrder.size() >
            kMaximumRememberedMessageIds) {
            if (client->receivedMessageIds.remove(
                    client->receivedMessageOrder.dequeue())) {
                --totalRememberedMessageIds_;
            }
        }
        return true;
    }

    void CompleteClientAuthentication(ClientState* client)
    {
        if (!client || client->authenticated) {
            return;
        }
        client->authenticated = true;
        client->authenticationDeadlineMs = 0;
        unauthenticatedConnectionCount_ =
            std::max(0, unauthenticatedConnectionCount_ - 1);
    }

    bool DeviceHasSession(const QString& deviceId) const
    {
        return deviceSessions_.contains(deviceId);
    }

    bool InsertSession(SessionState session)
    {
        if (session.sessionId.isEmpty() ||
            session.requesterDeviceId.isEmpty() ||
            session.targetDeviceId.isEmpty() ||
            sessions_.contains(session.sessionId) ||
            DeviceHasSession(session.requesterDeviceId) ||
            DeviceHasSession(session.targetDeviceId)) {
            return false;
        }
        const QString sessionId = session.sessionId;
        const QString requesterDeviceId = session.requesterDeviceId;
        const QString targetDeviceId = session.targetDeviceId;
        sessions_.insert(sessionId, std::move(session));
        deviceSessions_.insert(requesterDeviceId, sessionId);
        deviceSessions_.insert(targetDeviceId, sessionId);
        return true;
    }

    void RemoveSession(const QString& sessionId)
    {
        const auto sessionIt = sessions_.find(sessionId);
        if (sessionIt == sessions_.end()) {
            return;
        }
        const QString requesterDeviceId = sessionIt->requesterDeviceId;
        const QString targetDeviceId = sessionIt->targetDeviceId;
        sessions_.erase(sessionIt);
        if (deviceSessions_.value(requesterDeviceId) == sessionId) {
            deviceSessions_.remove(requesterDeviceId);
        }
        if (deviceSessions_.value(targetDeviceId) == sessionId) {
            deviceSessions_.remove(targetDeviceId);
        }
    }

    void ClearSessions()
    {
        sessions_.clear();
        deviceSessions_.clear();
    }

    void AddAccountConnection(ClientState* client)
    {
        if (!client || !client->socket || client->trustedUserId <= 0) {
            return;
        }
        accountConnections_[client->trustedUserId].insert(client->socket);
    }

    void RemoveAccountConnection(const ClientState* client)
    {
        if (!client || !client->socket || client->trustedUserId <= 0) {
            return;
        }
        auto accountIt = accountConnections_.find(client->trustedUserId);
        if (accountIt == accountConnections_.end()) {
            return;
        }
        accountIt->remove(client->socket);
        if (accountIt->isEmpty()) {
            accountConnections_.erase(accountIt);
        }
    }

    bool DeviceHasRoom(const QString& deviceId) const
    {
        return deviceRooms_.contains(deviceId);
    }

    bool ReleaseDisconnectedRoomForFreshStart(const QString& deviceId)
    {
        const QString roomId = deviceRooms_.value(deviceId);
        auto roomIt = rooms_.find(roomId);
        if (roomIt == rooms_.end()) {
            if (!roomId.isEmpty()) {
                deviceRooms_.remove(deviceId);
            }
            return false;
        }
        const auto memberIt = roomIt->members.constFind(deviceId);
        if (memberIt == roomIt->members.cend() ||
            memberIt->disconnectedAtMs == 0) {
            return false;
        }

        const QString reasonCode =
            QStringLiteral("stale_membership_replaced");
        if (roomIt->ownerDeviceId == deviceId) {
            CloseRoom(roomId, deviceId, reasonCode);
            return true;
        }
        if (roomIt->screenSharerDeviceId == deviceId ||
            roomIt->pendingScreenSharerDeviceId == deviceId) {
            ResetRoomScreenShare(*roomIt, deviceId, reasonCode);
        } else {
            if (roomIt->pendingControllerDeviceId == deviceId) {
                CancelPendingRoomControl(
                    *roomIt, reasonCode,
                    QStringLiteral("The stale room member was replaced."));
            }
            if (roomIt->activeControllerDeviceId == deviceId) {
                RevokeRoomControl(*roomIt, deviceId, reasonCode);
            }
        }
        CloseRoomPairsForMember(*roomIt, deviceId, deviceId, reasonCode);
        roomIt->members.remove(deviceId);
        deviceRooms_.remove(deviceId);
        BroadcastRoomState(*roomIt);
        return true;
    }

    bool DeleteLocalAccount(const QString& subject, QString* error)
    {
        bool existed = false;
        if (!identityStore_.DeleteUserByLogtoSubject(
                subject, &existed, error)) {
            return false;
        }

        QList<QPointer<QWebSocket>> accountSockets;
        QSet<QString> deviceIds;
        for (const auto& [socket, state] : clients_) {
            if (state->logtoSubject == subject) {
                accountSockets.append(socket);
                if (state->registered && !state->claims.deviceId.isEmpty()) {
                    deviceIds.insert(state->claims.deviceId);
                }
            }
        }

        for (const QString& deviceId : deviceIds) {
            CancelPendingRoomJoinForDevice(deviceId);
            const QString roomId = deviceRooms_.value(deviceId);
            auto roomIt = rooms_.find(roomId);
            if (roomIt != rooms_.end()) {
                const QString reason = QStringLiteral("account_deleted");
                if (roomIt->ownerDeviceId == deviceId) {
                    CloseRoom(roomId, deviceId, reason);
                } else {
                    if (roomIt->screenSharerDeviceId == deviceId ||
                        roomIt->pendingScreenSharerDeviceId == deviceId) {
                        ResetRoomScreenShare(*roomIt, deviceId, reason);
                    } else {
                        if (roomIt->pendingControllerDeviceId == deviceId) {
                            CancelPendingRoomControl(
                                *roomIt, reason,
                                QStringLiteral(
                                    "The account was deleted."));
                        }
                        if (roomIt->activeControllerDeviceId == deviceId) {
                            RevokeRoomControl(*roomIt, deviceId, reason);
                        }
                    }
                    CloseRoomPairsForMember(
                        *roomIt, deviceId, deviceId, reason);
                    roomIt->members.remove(deviceId);
                    deviceRooms_.remove(deviceId);
                    BroadcastRoomState(*roomIt);
                }
            }

            const QString sessionId = deviceSessions_.value(deviceId);
            const auto sessionIt = sessions_.constFind(sessionId);
            if (sessionIt != sessions_.cend()) {
                const QString peerDeviceId =
                    sessionIt->requesterDeviceId == deviceId
                        ? sessionIt->targetDeviceId
                        : sessionIt->requesterDeviceId;
                if (QWebSocket* peer = devices_.value(peerDeviceId, nullptr)) {
                    SendSessionEnded(
                        peer, sessionIt.key(), deviceId,
                        QStringLiteral("account_deleted"),
                        QStringLiteral("closed"));
                }
                RemoveSession(sessionId);
            }
            devices_.remove(deviceId);
        }

        for (const QPointer<QWebSocket>& socket : accountSockets) {
            if (!socket) {
                continue;
            }
            SendEnvelope(socket, QStringLiteral("account_deleted"), {},
                         QJsonObject{{QStringLiteral("subject"), subject}});
            QTimer::singleShot(100, socket, [socket] {
                if (socket) {
                    socket->close(
                        QWebSocketProtocol::CloseCodeNormal,
                        QStringLiteral("account deleted"));
                }
            });
        }
        Q_UNUSED(existed);
        return true;
    }

    void SendError(QWebSocket* socket,
                   const QString& sessionId,
                   const QString& code,
                   const QString& message)
    {
        QJsonObject payload;
        payload.insert(QStringLiteral("code"), code);
        payload.insert(QStringLiteral("message"), message);
        SendEnvelope(socket, QStringLiteral("error"), sessionId, payload);
    }

    QString PeerRateLimitKey(const QWebSocket* socket) const
    {
        if (!socket) {
            return QStringLiteral("unknown");
        }
        const QHostAddress address = socket->peerAddress();
        bool isIpv4 = false;
        const quint32 ipv4 = address.toIPv4Address(&isIpv4);
        if (isIpv4) {
            return QHostAddress(ipv4).toString();
        }
        const QString text = address.toString().toLower();
        return text.isEmpty() ? QStringLiteral("unknown") : text;
    }

    bool AllowAuthenticationAttempt(QWebSocket* socket, qint64 now)
    {
        return authenticationIpRateLimiter_.TryAcquire(
            PeerRateLimitKey(socket), 1, now);
    }

    bool AllowRoomAvailabilityQuery(ClientState* client,
                                    int requestedRoomCount,
                                    qint64 now)
    {
        if (config_.disableBusinessRateLimitsForTest) {
            return true;
        }
        const QString ipKey = PeerRateLimitKey(client->socket);
        const QString userKey = client->trustedUserId > 0
            ? QString::number(client->trustedUserId)
            : QString();
        const QString deviceKey = client->claims.deviceId;
        const bool allowedByIp = availabilityIpRateLimiter_.CanAcquire(
            ipKey, requestedRoomCount, now);
        const bool allowedByUser = userKey.isEmpty() ||
            availabilityUserRateLimiter_.CanAcquire(
                userKey, requestedRoomCount, now);
        const bool allowedByDevice = deviceKey.isEmpty() ||
            availabilityDeviceRateLimiter_.CanAcquire(
                deviceKey, requestedRoomCount, now);
        if (!allowedByIp || !allowedByUser || !allowedByDevice) {
            return false;
        }
        availabilityIpRateLimiter_.Record(
            ipKey, requestedRoomCount, now);
        if (!userKey.isEmpty()) {
            availabilityUserRateLimiter_.Record(
                userKey, requestedRoomCount, now);
        }
        if (!deviceKey.isEmpty()) {
            availabilityDeviceRateLimiter_.Record(
                deviceKey, requestedRoomCount, now);
        }
        return true;
    }

    bool AllowRoomJoinAttempt(ClientState* client, qint64 now)
    {
        if (config_.disableBusinessRateLimitsForTest) {
            return true;
        }
        const QString ipKey = PeerRateLimitKey(client->socket);
        const QString userKey = client->trustedUserId > 0
            ? QString::number(client->trustedUserId)
            : QString();
        const QString deviceKey = client->claims.deviceId;
        const bool allowedByIp = roomJoinIpRateLimiter_.CanAcquire(
            ipKey, 1, now);
        const bool allowedByUser = userKey.isEmpty() ||
            roomJoinUserRateLimiter_.CanAcquire(userKey, 1, now);
        const bool allowedByDevice = deviceKey.isEmpty() ||
            roomJoinDeviceRateLimiter_.CanAcquire(deviceKey, 1, now);
        if (!allowedByIp || !allowedByUser || !allowedByDevice) {
            return false;
        }
        roomJoinIpRateLimiter_.Record(ipKey, 1, now);
        if (!userKey.isEmpty()) {
            roomJoinUserRateLimiter_.Record(userKey, 1, now);
        }
        if (!deviceKey.isEmpty()) {
            roomJoinDeviceRateLimiter_.Record(deviceKey, 1, now);
        }
        return true;
    }

    bool AllowAccountDeletion(ClientState* client, qint64 now)
    {
        return client->trustedUserId > 0 &&
               accountDeletionUserRateLimiter_.TryAcquire(
                   QString::number(client->trustedUserId), 1, now);
    }

    bool AllowAssistanceAttempt(ClientState* client,
                                const QString& targetDeviceId,
                                qint64 now)
    {
        if (config_.disableBusinessRateLimitsForTest) {
            return true;
        }
        const QString ipKey = PeerRateLimitKey(client->socket);
        const QString requesterKey = client->claims.deviceId;
        const bool allowedByIp = assistanceIpRateLimiter_.CanAcquire(
            ipKey, 1, now);
        const bool allowedByRequester =
            assistanceRequesterRateLimiter_.CanAcquire(
                requesterKey, 1, now);
        const bool allowedByTarget =
            assistanceTargetRateLimiter_.CanAcquire(
                targetDeviceId, 1, now);
        if (!allowedByIp || !allowedByRequester || !allowedByTarget) {
            return false;
        }
        assistanceIpRateLimiter_.Record(ipKey, 1, now);
        assistanceRequesterRateLimiter_.Record(requesterKey, 1, now);
        assistanceTargetRateLimiter_.Record(targetDeviceId, 1, now);
        return true;
    }

    void PruneRateLimiters(qint64 now)
    {
        authenticationIpRateLimiter_.Prune(now);
        availabilityIpRateLimiter_.Prune(now);
        availabilityUserRateLimiter_.Prune(now);
        availabilityDeviceRateLimiter_.Prune(now);
        roomJoinIpRateLimiter_.Prune(now);
        roomJoinUserRateLimiter_.Prune(now);
        roomJoinDeviceRateLimiter_.Prune(now);
        accountDeletionUserRateLimiter_.Prune(now);
        assistanceIpRateLimiter_.Prune(now);
        assistanceRequesterRateLimiter_.Prune(now);
        assistanceTargetRateLimiter_.Prune(now);
    }

    void ClearRateLimiters()
    {
        authenticationIpRateLimiter_.Clear();
        availabilityIpRateLimiter_.Clear();
        availabilityUserRateLimiter_.Clear();
        availabilityDeviceRateLimiter_.Clear();
        roomJoinIpRateLimiter_.Clear();
        roomJoinUserRateLimiter_.Clear();
        roomJoinDeviceRateLimiter_.Clear();
        accountDeletionUserRateLimiter_.Clear();
        assistanceIpRateLimiter_.Clear();
        assistanceRequesterRateLimiter_.Clear();
        assistanceTargetRateLimiter_.Clear();
    }

    qsizetype RateLimiterKeyCount() const
    {
        return authenticationIpRateLimiter_.keyCount() +
               availabilityIpRateLimiter_.keyCount() +
               availabilityUserRateLimiter_.keyCount() +
               availabilityDeviceRateLimiter_.keyCount() +
               roomJoinIpRateLimiter_.keyCount() +
               roomJoinUserRateLimiter_.keyCount() +
               roomJoinDeviceRateLimiter_.keyCount() +
               accountDeletionUserRateLimiter_.keyCount() +
               assistanceIpRateLimiter_.keyCount() +
               assistanceRequesterRateLimiter_.keyCount() +
               assistanceTargetRateLimiter_.keyCount();
    }

    void ProbeEventLoopLag()
    {
        if (!diagnosticsClock_.isValid()) {
            return;
        }
        const qint64 now = diagnosticsClock_.elapsed();
        if (diagnosticProbeExpectedMs_ > 0) {
            maximumEventLoopLagMs_ = std::max(
                maximumEventLoopLagMs_,
                std::max<qint64>(0, now - diagnosticProbeExpectedMs_));
        }
        diagnosticProbeExpectedMs_ = now + 1000;
    }

    void WriteDiagnostics()
    {
        const qint64 nowMs = diagnosticsClock_.isValid()
                                 ? diagnosticsClock_.elapsed()
                                 : 0;
        const qint64 intervalMs = lastDiagnosticsAtMs_ > 0
                                      ? nowMs - lastDiagnosticsAtMs_
                                      : 0;
        const qint64 receivedMessages =
            receivedMessagesTotal_ - lastReceivedMessagesTotal_;
        const qint64 receivedBytes =
            receivedBytesTotal_ - lastReceivedBytesTotal_;
        const qint64 sentMessages =
            sentMessagesTotal_ - lastSentMessagesTotal_;
        const qint64 sentBytes = sentBytesTotal_ - lastSentBytesTotal_;
        const double seconds = intervalMs > 0 ? intervalMs / 1000.0 : 0.0;
        const double receivedMessagesPerSecond =
            seconds > 0 ? receivedMessages / seconds : 0.0;
        const double sentMessagesPerSecond =
            seconds > 0 ? sentMessages / seconds : 0.0;
        const double receivedBitsPerSecond =
            seconds > 0 ? receivedBytes * 8.0 / seconds : 0.0;
        const double sentBitsPerSecond =
            seconds > 0 ? sentBytes * 8.0 / seconds : 0.0;
        qint64 queuedOutgoingBytes = 0;
        qint64 maximumClientQueuedOutgoingBytes = 0;
        for (const auto& [socket, state] : clients_) {
            Q_UNUSED(state);
            const qint64 queued = socket ? socket->bytesToWrite() : 0;
            queuedOutgoingBytes += queued;
            maximumClientQueuedOutgoingBytes =
                std::max(maximumClientQueuedOutgoingBytes, queued);
        }

        QJsonObject snapshot;
        snapshot.insert(QStringLiteral("timestampMs"),
                        QDateTime::currentMSecsSinceEpoch());
        snapshot.insert(
            QStringLiteral("timestampUtc"),
            QDateTime::currentDateTimeUtc().toString(Qt::ISODateWithMs));
        snapshot.insert(QStringLiteral("connections"),
                        static_cast<qint64>(clients_.size()));
        snapshot.insert(
            QStringLiteral("authenticatedConnections"),
            static_cast<qint64>(clients_.size()) -
                unauthenticatedConnectionCount_);
        snapshot.insert(QStringLiteral("unauthenticatedConnections"),
                        unauthenticatedConnectionCount_);
        snapshot.insert(QStringLiteral("registeredDevices"), devices_.size());
        snapshot.insert(QStringLiteral("accounts"),
                        accountConnections_.size());
        snapshot.insert(QStringLiteral("onlineAccounts"),
                        accountConnections_.size());
        snapshot.insert(QStringLiteral("sessions"), sessions_.size());
        snapshot.insert(QStringLiteral("rooms"), rooms_.size());
        snapshot.insert(QStringLiteral("roomPairs"), pairRooms_.size());
        snapshot.insert(QStringLiteral("roomJoinRequests"),
                        roomJoinRequests_.size());
        snapshot.insert(QStringLiteral("rememberedMessageIds"),
                        totalRememberedMessageIds_);
        snapshot.insert(QStringLiteral("rateLimiterKeys"),
                        RateLimiterKeyCount());
        snapshot.insert(QStringLiteral("maximumEventLoopLagMs"),
                        maximumEventLoopLagMs_);
        snapshot.insert(QStringLiteral("acceptedConnectionsTotal"),
                        acceptedConnectionsTotal_);
        snapshot.insert(QStringLiteral("rejectedConnectionsTotal"),
                        rejectedConnectionsTotal_);
        snapshot.insert(QStringLiteral("receivedMessagesTotal"),
                        receivedMessagesTotal_);
        snapshot.insert(QStringLiteral("receivedBytesTotal"),
                        receivedBytesTotal_);
        snapshot.insert(QStringLiteral("sentMessagesTotal"),
                        sentMessagesTotal_);
        snapshot.insert(QStringLiteral("sentBytesTotal"), sentBytesTotal_);
        snapshot.insert(QStringLiteral("queuedOutgoingBytes"),
                        queuedOutgoingBytes);
        snapshot.insert(QStringLiteral("maximumClientQueuedOutgoingBytes"),
                        maximumClientQueuedOutgoingBytes);
        snapshot.insert(QStringLiteral("receivedMessagesPerSecond"),
                        receivedMessagesPerSecond);
        snapshot.insert(QStringLiteral("sentMessagesPerSecond"),
                        sentMessagesPerSecond);
        snapshot.insert(QStringLiteral("receivedBitsPerSecond"),
                        receivedBitsPerSecond);
        snapshot.insert(QStringLiteral("sentBitsPerSecond"),
                        sentBitsPerSecond);
        const QByteArray line = QByteArrayLiteral("SIGNAL_SERVER_DIAGNOSTICS=") +
                                QJsonDocument(snapshot).toJson(
                                    QJsonDocument::Compact) +
                                '\n';
        if (StdoutIsTerminal()) {
            QTextStream output(stdout);
            output.flush();
            ClearStdoutTerminal();
            output << "RemoteC 信令服务器运行状态\n"
                   << "========================================\n"
                   << "更新时间："
                   << QDateTime::currentDateTime().toString(
                          QStringLiteral("yyyy-MM-dd HH:mm:ss"))
                   << '\n'
                   << "运行时间：" << FormatDuration(nowMs) << '\n'
                   << "服务地址：wss://" << config_.listenAddress.toString()
                   << ':' << server_.serverPort() << "/signaling\n\n"
                   << "【在线状态】\n"
                   << "在线账号：" << accountConnections_.size()
                   << "    在线设备：" << devices_.size() << '\n'
                   << "WSS 连接：" << clients_.size()
                   << "    已认证："
                   << static_cast<qint64>(clients_.size()) -
                          unauthenticatedConnectionCount_
                   << "    待认证：" << unauthenticatedConnectionCount_
                   << '\n'
                   << "累计接入：" << acceptedConnectionsTotal_
                   << "    累计拒绝：" << rejectedConnectionsTotal_ << "\n\n"
                   << "【业务状态】\n"
                   << "直接会话：" << sessions_.size()
                   << "    协助房间：" << rooms_.size()
                   << "    房间连接对：" << pairRooms_.size() << '\n'
                   << "等待入房：" << roomJoinRequests_.size() << "\n\n"
                   << "【最近一个统计周期】\n"
                   << "接收：" << FormatBitRate(receivedBitsPerSecond)
                   << "    "
                   << QString::number(receivedMessagesPerSecond, 'f', 1)
                   << " 条/秒\n"
                   << "发送：" << FormatBitRate(sentBitsPerSecond)
                   << "    "
                   << QString::number(sentMessagesPerSecond, 'f', 1)
                   << " 条/秒\n\n"
                   << "【累计流量】\n"
                   << "接收：" << FormatBytes(receivedBytesTotal_)
                   << "    " << receivedMessagesTotal_ << " 条消息\n"
                   << "发送：" << FormatBytes(sentBytesTotal_)
                   << "    " << sentMessagesTotal_ << " 条消息\n\n"
                   << "【运行健康】\n"
                   << "事件循环最大延迟：" << maximumEventLoopLagMs_
                   << " ms\n"
                   << "发送积压：" << FormatBytes(queuedOutgoingBytes)
                   << "    单连接最大积压："
                   << FormatBytes(maximumClientQueuedOutgoingBytes) << '\n'
                   << "限流记录：" << RateLimiterKeyCount() << '\n'
                   << "========================================\n"
                   << "状态每 " << config_.diagnosticsIntervalMs / 1000
                   << " 秒自动刷新；详细记录保存在 logs 文件夹。"
                   << Qt::endl;
        } else {
            QTextStream(stdout) << line;
        }
        if (diagnosticsFile_.isOpen()) {
            diagnosticsFile_.write(line);
            diagnosticsFile_.flush();
        }
        lastDiagnosticsAtMs_ = nowMs;
        lastReceivedMessagesTotal_ = receivedMessagesTotal_;
        lastReceivedBytesTotal_ = receivedBytesTotal_;
        lastSentMessagesTotal_ = sentMessagesTotal_;
        lastSentBytesTotal_ = sentBytesTotal_;
        maximumEventLoopLagMs_ = 0;
    }

    void SendEnvelope(QWebSocket* socket,
                      const QString& type,
                      const QString& sessionId,
                      const QJsonObject& payload)
    {
        if (!socket ||
            socket->state() != QAbstractSocket::ConnectedState) {
            return;
        }
        QJsonObject envelope;
        envelope.insert(QStringLiteral("protocolVersion"), kProtocolVersion);
        envelope.insert(
            QStringLiteral("messageId"),
            QUuid::createUuid().toString(QUuid::WithoutBraces));
        envelope.insert(QStringLiteral("type"), type);
        if (!sessionId.isEmpty()) {
            envelope.insert(QStringLiteral("sessionId"), sessionId);
        }
        envelope.insert(QStringLiteral("payload"), payload);
        const QByteArray encoded =
            QJsonDocument(envelope).toJson(QJsonDocument::Compact);
        if (socket->sendTextMessage(QString::fromUtf8(encoded)) >= 0) {
            ++sentMessagesTotal_;
            sentBytesTotal_ += encoded.size();
        }
    }

    SignalServerConfig config_;
    AccessTokenService tokenService_;
    remote::server_auth::LogtoUserInfoClient userInfoClient_;
    remote::server_auth::LogtoManagementClient managementClient_;
    remote::server_auth::LogtoWebhookServer webhookServer_;
    remote::server_persistence::IdentityStore identityStore_;
    SlidingWindowRateLimiter authenticationIpRateLimiter_;
    SlidingWindowRateLimiter availabilityIpRateLimiter_;
    SlidingWindowRateLimiter availabilityUserRateLimiter_;
    SlidingWindowRateLimiter availabilityDeviceRateLimiter_;
    SlidingWindowRateLimiter roomJoinIpRateLimiter_;
    SlidingWindowRateLimiter roomJoinUserRateLimiter_;
    SlidingWindowRateLimiter roomJoinDeviceRateLimiter_;
    SlidingWindowRateLimiter accountDeletionUserRateLimiter_{
        kAccountDeletionUserRateLimit};
    SlidingWindowRateLimiter assistanceIpRateLimiter_{
        kAssistanceIpRateLimit};
    SlidingWindowRateLimiter assistanceRequesterRateLimiter_{
        kAssistanceRequesterRateLimit};
    SlidingWindowRateLimiter assistanceTargetRateLimiter_{
        kAssistanceTargetRateLimit};
    QWebSocketServer server_;
    QTimer maintenanceTimer_;
    QTimer diagnosticProbeTimer_;
    QTimer diagnosticsTimer_;
    QFile diagnosticsFile_;
    QElapsedTimer diagnosticsClock_;
    qint64 diagnosticProbeExpectedMs_ = 0;
    qint64 maximumEventLoopLagMs_ = 0;
    qint64 lastDiagnosticsAtMs_ = 0;
    qint64 lastReceivedMessagesTotal_ = 0;
    qint64 lastReceivedBytesTotal_ = 0;
    qint64 lastSentMessagesTotal_ = 0;
    qint64 lastSentBytesTotal_ = 0;
    qint64 acceptedConnectionsTotal_ = 0;
    qint64 rejectedConnectionsTotal_ = 0;
    qint64 receivedMessagesTotal_ = 0;
    qint64 receivedBytesTotal_ = 0;
    qint64 sentMessagesTotal_ = 0;
    qint64 sentBytesTotal_ = 0;
    std::unordered_map<QWebSocket*, std::unique_ptr<ClientState>> clients_;
    int unauthenticatedConnectionCount_ = 0;
    QHash<QString, int> connectionCountByIp_;
    qint64 totalRememberedMessageIds_ = 0;
    QHash<QString, QWebSocket*> devices_;
    QHash<qint64, QSet<QWebSocket*>> accountConnections_;
    quint64 ownedDevicesRevision_ = 1;
    QHash<QString, SessionState> sessions_;
    QHash<QString, QString> deviceSessions_;
    QHash<QString, RoomState> rooms_;
    QHash<QString, QString> deviceRooms_;
    QHash<QString, QString> pairRooms_;
    QHash<QString, RoomJoinRequestState> roomJoinRequests_;
    QHash<QString, QString> pendingRoomJoinRequestByDevice_;
};

SignalServer::SignalServer(SignalServerConfig config)
    : impl_(std::make_unique<Impl>(std::move(config)))
{}

SignalServer::~SignalServer()
{
    impl_->Stop();
}

bool SignalServer::Start(QString* error)
{
    return impl_->Start(error);
}

void SignalServer::Stop()
{
    impl_->Stop();
}

bool SignalServer::IsListening() const
{
    return impl_->IsListening();
}

quint16 SignalServer::ServerPort() const
{
    return impl_->ServerPort();
}

quint16 SignalServer::WebhookPort() const
{
    return impl_->WebhookPort();
}

}  // namespace remote::signaling_server
