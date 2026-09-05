// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

    void SendAccountDeletionResult(
        QWebSocket* socket,
        bool deleted,
        const QString& code,
        const QString& message,
        bool retryable)
    {
        QJsonObject payload;
        payload.insert(QStringLiteral("deleted"), deleted);
        payload.insert(QStringLiteral("code"), code);
        payload.insert(QStringLiteral("message"), message);
        payload.insert(QStringLiteral("retryable"), retryable);
        SendEnvelope(socket, QStringLiteral("account_delete_result"), {},
                     payload);
    }

    void HandleAccountDelete(ClientState* client)
    {
        if (config_.authenticationMode !=
                SignalServerAuthenticationMode::kLogtoUserInfo ||
            client->logtoSubject.isEmpty() || client->trustedUserId <= 0) {
            SendAccountDeletionResult(
                client->socket, false,
                QStringLiteral("account_deletion_unavailable"),
                QStringLiteral("Account deletion requires Logto authentication."),
                false);
            return;
        }
        if (!managementClient_.IsConfigured()) {
            SendAccountDeletionResult(
                client->socket, false,
                QStringLiteral("account_deletion_unavailable"),
                QStringLiteral(
                    "The server administrator has not configured Logto account deletion."),
                false);
            return;
        }
        if (client->accountDeletionPending) {
            SendAccountDeletionResult(
                client->socket, false,
                QStringLiteral("account_deletion_pending"),
                QStringLiteral("Account deletion is already in progress."),
                false);
            return;
        }
        if (!AllowAccountDeletion(
                client, QDateTime::currentMSecsSinceEpoch())) {
            SendAccountDeletionResult(
                client->socket, false, QStringLiteral("rate_limited"),
                QStringLiteral(
                    "Too many account deletion attempts. Try again later."),
                true);
            return;
        }

        client->accountDeletionPending = true;
        const QString subject = client->logtoSubject;
        const QPointer<QWebSocket> socket = client->socket;
        managementClient_.DeleteUser(
            subject,
            [this, socket, subject](
                remote::server_auth::UserDeletionResult result) {
                ClientState* current = socket ? FindClient(socket) : nullptr;
                if (current && current->logtoSubject == subject) {
                    current->accountDeletionPending = false;
                }
                if (!result.deleted()) {
                    if (socket) {
                        SendAccountDeletionResult(
                            socket, false, result.code, result.message,
                            result.retryable);
                    }
                    return;
                }

                if (socket) {
                    SendAccountDeletionResult(
                        socket, true,
                        result.status ==
                                remote::server_auth::UserDeletionStatus::kNotFound
                            ? QStringLiteral("account_already_deleted")
                            : QStringLiteral("account_deleted"),
                        QStringLiteral("The RemoteC account was deleted."),
                        false);
                }
                QString localError;
                if (!DeleteLocalAccount(subject, &localError) && socket) {
                    SendError(
                        socket, {}, QStringLiteral("local_cleanup_failed"),
                        QStringLiteral(
                            "The Logto account was deleted, but local server cleanup must be retried."));
                    QTimer::singleShot(100, socket, [socket] {
                        if (socket) {
                            socket->close(
                                QWebSocketProtocol::CloseCodeNormal,
                                QStringLiteral("account deleted"));
                        }
                    });
                }
            });
    }
