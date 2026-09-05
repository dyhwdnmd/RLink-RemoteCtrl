// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#include "QtWebSocketSignalingClient.h"

#include <algorithm>
#include <atomic>
#include <deque>
#include <limits>
#include <string_view>
#include <unordered_set>
#include <utility>

#include <QAbstractSocket>
#include <QDateTime>
#include <QElapsedTimer>
#include <QHostAddress>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <QMetaObject>
#include <QNetworkProxy>
#include <QNetworkRequest>
#include <QSslCertificate>
#include <QSslConfiguration>
#include <QSslError>
#include <QStringList>
#include <QThread>
#include <QTimer>
#include <QUrl>
#include <QUuid>
#include <QWebSocket>

#include "SignalingJsonCodec.h"

namespace remote {
namespace {

constexpr int kProtocolVersion = 5;
constexpr quint64 kMaximumMessageBytes = 1024 * 1024;
constexpr std::size_t kMaximumAccessTokenBytes = 16 * 1024;
constexpr std::size_t kMaximumRememberedMessageIds = 4096;

using signaling_json::ReadStringArray;
using signaling_json::StringArray;
using signaling_json::ToQString;
using signaling_json::ToString;

SignalingOperationResult Success()
{
    return {true, {}, {}};
}

SignalingOperationResult Failure(std::string code, std::string message)
{
    return {false, std::move(code), std::move(message)};
}

bool IsNineDigitPublicId(std::string_view value)
{
    return value.size() == 9 && value.front() != '0' &&
           std::all_of(value.cbegin(), value.cend(),
                       [](char character) {
                           return character >= '0' && character <= '9';
                       });
}

bool ShouldBypassProxy(const QUrl& endpoint)
{
    QHostAddress address;
    return address.setAddress(endpoint.host()) &&
           (address.isLoopback() || address.isPrivateUse());
}

QString PurposeToString(SessionPurpose purpose)
{
    switch (purpose) {
    case SessionPurpose::kRemoteControl:
        return QStringLiteral("remote_control");
    case SessionPurpose::kCameraOnly:
        return QStringLiteral("camera_only");
    default:
        return QStringLiteral("none");
    }
}

SessionPurpose PurposeFromString(const QString& purpose)
{
    if (purpose == QStringLiteral("remote_control")) {
        return SessionPurpose::kRemoteControl;
    }
    if (purpose == QStringLiteral("camera_only")) {
        return SessionPurpose::kCameraOnly;
    }
    return SessionPurpose::kNone;
}

RoomScreenShareState ScreenShareStateFromString(const QString& state,
                                                bool* valid)
{
    *valid = true;
    if (state == QStringLiteral("idle")) {
        return RoomScreenShareState::kIdle;
    }
    if (state == QStringLiteral("switching")) {
        return RoomScreenShareState::kSwitching;
    }
    if (state == QStringLiteral("active")) {
        return RoomScreenShareState::kActive;
    }
    if (state == QStringLiteral("recovering")) {
        return RoomScreenShareState::kRecovering;
    }
    *valid = false;
    return RoomScreenShareState::kIdle;
}

bool ReadRoomSnapshot(const QJsonValue& value, RoomSnapshot* snapshot)
{
    if (!snapshot || !value.isObject()) {
        return false;
    }
    const QJsonObject object = value.toObject();
    RoomSnapshot parsed;
    parsed.membership = RoomMembershipState::kActive;
    parsed.roomId = ToString(
        object.value(QStringLiteral("roomId")).toString());
    parsed.ownerDeviceId = ToString(
        object.value(QStringLiteral("ownerDeviceId")).toString());
    const int capacity = object.value(QStringLiteral("capacity")).toInt(-1);
    bool screenStateValid = false;
    parsed.screenShareState = ScreenShareStateFromString(
        object.value(QStringLiteral("screenShareState")).toString(),
        &screenStateValid);
    parsed.screenSharerDeviceId = ToString(
        object.value(QStringLiteral("screenSharerDeviceId")).toString());
    parsed.pendingScreenSharerDeviceId = ToString(
        object.value(QStringLiteral("pendingScreenSharerDeviceId")).toString());
    const double screenShareEpoch =
        object.value(QStringLiteral("screenShareEpoch")).toDouble(-1.0);
    parsed.activeControllerDeviceId = ToString(
        object.value(QStringLiteral("activeControllerDeviceId")).toString());
    parsed.pendingControllerDeviceId = ToString(
        object.value(QStringLiteral("pendingControllerDeviceId")).toString());

    const QJsonValue membersValue = object.value(QStringLiteral("members"));
    if (!IsNineDigitPublicId(parsed.roomId) ||
        parsed.ownerDeviceId.empty() ||
        capacity < static_cast<int>(kMinimumRoomMembers) ||
        capacity > static_cast<int>(kProtocolMaximumRoomMembers) ||
        !screenStateValid || screenShareEpoch < 0.0 ||
        screenShareEpoch > 9007199254740991.0 ||
        !membersValue.isArray()) {
        return false;
    }
    parsed.capacity = static_cast<std::uint32_t>(capacity);
    parsed.screenShareEpoch =
        static_cast<std::uint64_t>(screenShareEpoch);

    const QJsonArray members = membersValue.toArray();
    if (members.isEmpty() ||
        members.size() > static_cast<int>(kProtocolMaximumRoomMembers) ||
        members.size() > capacity) {
        return false;
    }
    std::unordered_set<std::string> memberIds;
    bool ownerFound = false;
    parsed.members.reserve(static_cast<std::size_t>(members.size()));
    for (const auto& memberValue : members) {
        if (!memberValue.isObject()) {
            return false;
        }
        const QJsonObject memberObject = memberValue.toObject();
        RoomMemberSnapshot member;
        member.deviceId = ToString(
            memberObject.value(QStringLiteral("deviceId")).toString());
        member.deviceName = ToString(
            memberObject.value(QStringLiteral("deviceName")).toString());
        if (member.deviceId.empty() || member.deviceId.size() > 128 ||
            !memberObject.value(QStringLiteral("online")).isBool() ||
            !memberObject.value(QStringLiteral("cameraPublishing")).isBool() ||
            !memberObject.value(QStringLiteral("microphonePublishing")).isBool() ||
            !memberIds.insert(member.deviceId).second) {
            return false;
        }
        member.online =
            memberObject.value(QStringLiteral("online")).toBool();
        member.cameraPublishing =
            memberObject.value(QStringLiteral("cameraPublishing")).toBool();
        member.microphonePublishing =
            memberObject.value(QStringLiteral("microphonePublishing")).toBool();
        ownerFound = ownerFound || member.deviceId == parsed.ownerDeviceId;
        parsed.members.push_back(std::move(member));
    }
    const auto isMember = [&memberIds](const std::string& deviceId) {
        return deviceId.empty() || memberIds.contains(deviceId);
    };
    if (!ownerFound || !isMember(parsed.screenSharerDeviceId) ||
        !isMember(parsed.pendingScreenSharerDeviceId) ||
        !isMember(parsed.pendingControllerDeviceId) ||
        !isMember(parsed.activeControllerDeviceId)) {
        return false;
    }
    if ((parsed.screenShareState == RoomScreenShareState::kActive ||
         parsed.screenShareState == RoomScreenShareState::kRecovering) &&
        parsed.screenSharerDeviceId.empty()) {
        return false;
    }
    if (parsed.screenShareState == RoomScreenShareState::kSwitching &&
        parsed.pendingScreenSharerDeviceId.empty()) {
        return false;
    }
    if (parsed.screenShareState == RoomScreenShareState::kIdle &&
        (!parsed.screenSharerDeviceId.empty() ||
         !parsed.pendingScreenSharerDeviceId.empty() ||
         !parsed.pendingControllerDeviceId.empty() ||
         !parsed.activeControllerDeviceId.empty())) {
        return false;
    }
    if ((!parsed.pendingControllerDeviceId.empty() ||
         !parsed.activeControllerDeviceId.empty()) &&
        parsed.screenShareState != RoomScreenShareState::kActive &&
        parsed.screenShareState != RoomScreenShareState::kRecovering) {
        return false;
    }
    *snapshot = std::move(parsed);
    return true;
}

}  // namespace

class QtWebSocketSignalingClient::Impl final {
public:
#include "QtWebSocketSignalingClient.Lifecycle.inc"
#include "QtWebSocketSignalingClient.OwnedDevices.inc"
#include "QtWebSocketSignalingClient.LegacySession.inc"
#include "QtWebSocketSignalingClient.RoomCommands.inc"
#include "QtWebSocketSignalingClient.PairTransport.inc"
#include "QtWebSocketSignalingClient.Connection.inc"
#include "QtWebSocketSignalingClient.Negotiation.inc"
#include "QtWebSocketSignalingClient.Recovery.inc"
#include "QtWebSocketSignalingClient.Dispatch.inc"
    SignalingOperationResult SendEnvelope(const QString& type,
                                          const std::string& sessionId,
                                          const QJsonObject& payload)
    {
        if (socket_.state() != QAbstractSocket::ConnectedState) {
            return Failure("signaling_socket_not_connected",
                           "The signaling WebSocket is not connected.");
        }
        QJsonObject envelope;
        envelope.insert(QStringLiteral("protocolVersion"), kProtocolVersion);
        envelope.insert(
            QStringLiteral("messageId"),
            QUuid::createUuid().toString(QUuid::WithoutBraces));
        envelope.insert(QStringLiteral("type"), type);
        if (!sessionId.empty()) {
            envelope.insert(QStringLiteral("sessionId"), ToQString(sessionId));
        }
        envelope.insert(QStringLiteral("payload"), payload);
        const QString message = QString::fromUtf8(
            QJsonDocument(envelope).toJson(QJsonDocument::Compact));
        if (socket_.sendTextMessage(message) < 0) {
            return Failure("signaling_send_failed",
                           "The signaling message could not be queued.");
        }
        return Success();
    }

    SignalingOperationResult RequireRegistered() const
    {
        if (State() == SignalingConnectionState::kRegistered) {
            return Success();
        }
        return Failure("signaling_not_registered",
                       "The device is not registered with signaling.");
    }

    bool RememberMessageId(const std::string& messageId)
    {
        if (!receivedMessageIds_.insert(messageId).second) {
            return false;
        }
        receivedMessageOrder_.push_back(messageId);
        if (receivedMessageOrder_.size() > kMaximumRememberedMessageIds) {
            receivedMessageIds_.erase(receivedMessageOrder_.front());
            receivedMessageOrder_.pop_front();
        }
        return true;
    }

    void SetState(SignalingConnectionState state)
    {
        if (state_.exchange(state, std::memory_order_acq_rel) == state) {
            return;
        }
        if (observer_) {
            observer_->OnSignalingStateChanged(state);
        }
    }

    void NotifyError(const std::string& code, const std::string& message)
    {
        if (observer_) {
            observer_->OnSignalingError(code, message);
        }
    }

    void Fail(const std::string& code, const std::string& message)
    {
        if (State() == SignalingConnectionState::kFailed) {
            return;
        }
        SetState(SignalingConnectionState::kFailed);
        NotifyError(code, message);
    }

    QWebSocket socket_;
    QTimer heartbeatTimer_;
    QTimer heartbeatDeadline_;
    QTimer authenticationDeadline_;
    QTimer reconnectTimer_;
    QElapsedTimer heartbeatRoundTrip_;
    bool heartbeatOutstanding_ = false;
    bool manualDisconnect_ = true;
    bool fatalDisconnect_ = false;
    std::uint32_t reconnectAttempt_ = 0;
    ISignalingClientObserver* observer_ = nullptr;
    SignalingClientConfig config_;
    QUrl endpoint_;
    std::atomic<SignalingConnectionState> state_{
        SignalingConnectionState::kDisconnected};
    std::unordered_set<std::string> receivedMessageIds_;
    std::deque<std::string> receivedMessageOrder_;
};

QtWebSocketSignalingClient::QtWebSocketSignalingClient()
    : impl_(std::make_unique<Impl>())
{}

QtWebSocketSignalingClient::~QtWebSocketSignalingClient() = default;

void QtWebSocketSignalingClient::SetObserver(
    ISignalingClientObserver* observer)
{
    impl_->SetObserver(observer);
}

SignalingOperationResult QtWebSocketSignalingClient::Connect(
    const SignalingClientConfig& config)
{
    return impl_->Connect(config);
}

SignalingOperationResult QtWebSocketSignalingClient::UpdateAccessToken(
    const std::string& accessToken)
{
    return impl_->UpdateAccessToken(accessToken);
}

SignalingOperationResult QtWebSocketSignalingClient::RequestAccountDeletion()
{
    return impl_->RequestAccountDeletion();
}

SignalingOperationResult QtWebSocketSignalingClient::RequestOwnedDevices()
{
    return impl_->RequestOwnedDevices();
}

void QtWebSocketSignalingClient::Disconnect()
{
    impl_->Disconnect();
}

SignalingConnectionState QtWebSocketSignalingClient::State() const
{
    return impl_->State();
}

SignalingOperationResult QtWebSocketSignalingClient::RequestSession(
    const std::string& targetDeviceId,
    SessionPurpose purpose,
    const std::vector<std::string>& permissions)
{
    return impl_->RequestSession(targetDeviceId, purpose, permissions);
}

SignalingOperationResult QtWebSocketSignalingClient::RequestOwnedDeviceSession(
    const std::string& targetDeviceId,
    SessionPurpose purpose,
    const std::vector<std::string>& permissions)
{
    return impl_->RequestOwnedDeviceSession(
        targetDeviceId, purpose, permissions);
}

SignalingOperationResult QtWebSocketSignalingClient::RequestAssistedSession(
    const std::string& targetDeviceId,
    const std::string& verificationCode,
    const std::vector<std::string>& permissions)
{
    return impl_->RequestAssistedSession(
        targetDeviceId, verificationCode, permissions);
}

SignalingOperationResult QtWebSocketSignalingClient::RespondToSession(
    const std::string& sessionId,
    bool accepted,
    const std::string& reasonCode)
{
    return impl_->RespondToSession(sessionId, accepted, reasonCode);
}

SignalingOperationResult QtWebSocketSignalingClient::CancelSession(
    const std::string& sessionId,
    const std::string& reasonCode)
{
    return impl_->CancelSession(sessionId, reasonCode);
}

SignalingOperationResult QtWebSocketSignalingClient::CloseSession(
    const std::string& sessionId,
    const std::string& reasonCode)
{
    return impl_->CloseSession(sessionId, reasonCode);
}

SignalingOperationResult QtWebSocketSignalingClient::ResumeSession(
    const std::string& sessionId,
    const std::string& recoveryToken)
{
    return impl_->ResumeSession(sessionId, recoveryToken);
}

SignalingOperationResult QtWebSocketSignalingClient::CreateRoom(
    std::uint32_t capacity)
{
    return impl_->CreateRoom(capacity);
}

SignalingOperationResult QtWebSocketSignalingClient::RequestRoomJoin(
    const std::string& roomId)
{
    return impl_->RequestRoomJoin(roomId);
}

SignalingOperationResult QtWebSocketSignalingClient::QueryRoomAvailability(
    const std::vector<std::string>& roomIds)
{
    return impl_->QueryRoomAvailability(roomIds);
}

SignalingOperationResult QtWebSocketSignalingClient::RespondToRoomJoin(
    const std::string& roomId,
    const std::string& requestId,
    bool accepted,
    const std::string& reasonCode)
{
    return impl_->RespondToRoomJoin(roomId, requestId, accepted, reasonCode);
}

SignalingOperationResult QtWebSocketSignalingClient::SetRoomCapacity(
    const std::string& roomId,
    std::uint32_t capacity)
{
    return impl_->SetRoomCapacity(roomId, capacity);
}

SignalingOperationResult QtWebSocketSignalingClient::LeaveRoom(
    const std::string& roomId,
    const std::string& reasonCode)
{
    return impl_->LeaveRoom(roomId, reasonCode);
}

SignalingOperationResult QtWebSocketSignalingClient::ResumeRoom(
    const std::string& roomId,
    const std::string& recoveryToken)
{
    return impl_->ResumeRoom(roomId, recoveryToken);
}

SignalingOperationResult QtWebSocketSignalingClient::SetRoomMediaState(
    const std::string& roomId,
    bool cameraPublishing,
    bool microphonePublishing)
{
    return impl_->SetRoomMediaState(roomId, cameraPublishing,
                                    microphonePublishing);
}

SignalingOperationResult QtWebSocketSignalingClient::RequestRoomScreenShare(
    const std::string& roomId)
{
    return impl_->RequestRoomScreenShare(roomId);
}

SignalingOperationResult QtWebSocketSignalingClient::ConfirmRoomScreenShare(
    const std::string& roomId,
    const std::string& grantId)
{
    return impl_->ConfirmRoomScreenShare(roomId, grantId);
}

SignalingOperationResult QtWebSocketSignalingClient::StopRoomScreenShare(
    const std::string& roomId,
    const std::string& grantId,
    const std::string& reasonCode)
{
    return impl_->StopRoomScreenShare(roomId, grantId, reasonCode);
}

SignalingOperationResult
QtWebSocketSignalingClient::RespondToRoomScreenShareSwitch(
    const std::string& roomId,
    const std::string& requestId,
    bool accepted,
    const std::string& reasonCode)
{
    return impl_->RespondToRoomScreenShareSwitch(
        roomId, requestId, accepted, reasonCode);
}

SignalingOperationResult
QtWebSocketSignalingClient::CancelRoomScreenShareSwitch(
    const std::string& roomId,
    const std::string& requestId,
    const std::string& reasonCode)
{
    return impl_->CancelRoomScreenShareSwitch(
        roomId, requestId, reasonCode);
}

SignalingOperationResult QtWebSocketSignalingClient::RequestRoomControl(
    const std::string& roomId)
{
    return impl_->RequestRoomControl(roomId);
}

SignalingOperationResult QtWebSocketSignalingClient::RespondToRoomControl(
    const std::string& roomId,
    const std::string& requestId,
    bool accepted,
    const std::string& reasonCode)
{
    return impl_->RespondToRoomControl(roomId, requestId, accepted,
                                       reasonCode);
}

SignalingOperationResult QtWebSocketSignalingClient::ReleaseRoomControl(
    const std::string& roomId,
    const std::string& grantId,
    const std::string& reasonCode)
{
    return impl_->ReleaseRoomControl(roomId, grantId, reasonCode);
}

SignalingOperationResult QtWebSocketSignalingClient::SendDescription(
    const SignalingSessionDescription& description)
{
    return impl_->SendDescription(description);
}

SignalingOperationResult QtWebSocketSignalingClient::SendIceCandidate(
    const SignalingIceCandidate& candidate)
{
    return impl_->SendIceCandidate(candidate);
}

SignalingOperationResult QtWebSocketSignalingClient::SendIceRestartRequest(
    const SignalingIceRestartRequest& request)
{
    return impl_->SendIceRestartRequest(request);
}

SignalingOperationResult QtWebSocketSignalingClient::SendIceRestartCancel(
    const SignalingIceRestartCancel& cancel)
{
    return impl_->SendIceRestartCancel(cancel);
}

void QtWebSocketSignalingClient::AbortConnectionForTesting()
{
    impl_->AbortConnectionForTesting();
}

}  // namespace remote
