// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#include "src/auth/DpapiTokenStore.h"

#include <Windows.h>
#include <wincrypt.h>

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSaveFile>
#include <QStandardPaths>

#include <limits>

namespace remote::auth {
namespace {

constexpr char kEnvelopeMagic[] = "RCAUTH";
constexpr char kEnvelopeVersion = 1;
constexpr char kEntropy[] = "RemoteC.Logto.RefreshToken.v1";
constexpr qint64 kMaximumCredentialsBytes = 1024 * 1024;

void SetError(QString* errorMessage, const QString& message) {
    if (errorMessage) {
        *errorMessage = message;
    }
}

QString WindowsErrorMessage(const QString& operation) {
    return QStringLiteral("%1 failed with Windows error %2.")
        .arg(operation)
        .arg(GetLastError());
}

void SecureClear(QByteArray* data) {
    if (data && !data->isEmpty()) {
        SecureZeroMemory(data->data(), static_cast<SIZE_T>(data->size()));
        data->clear();
    }
}

bool ToBlob(QByteArray* data, DATA_BLOB* blob) {
    if (!data || !blob || data->size() < 0 ||
        static_cast<quint64>(data->size()) >
            std::numeric_limits<DWORD>::max()) {
        return false;
    }
    blob->cbData = static_cast<DWORD>(data->size());
    blob->pbData = reinterpret_cast<BYTE*>(data->data());
    return true;
}

QByteArray Protect(QByteArray* plaintext, QString* errorMessage) {
    QByteArray entropyBytes(kEntropy, sizeof(kEntropy) - 1);
    DATA_BLOB input{};
    DATA_BLOB entropy{};
    DATA_BLOB output{};
    if (!ToBlob(plaintext, &input) || !ToBlob(&entropyBytes, &entropy)) {
        SetError(errorMessage,
                 QStringLiteral("Refresh token data is too large."));
        return {};
    }
    const BOOL protectedOk = CryptProtectData(
        &input,
        L"RemoteC Logto refresh token",
        &entropy,
        nullptr,
        nullptr,
        CRYPTPROTECT_UI_FORBIDDEN,
        &output);
    SecureClear(plaintext);
    SecureClear(&entropyBytes);
    if (!protectedOk) {
        SetError(errorMessage,
                 WindowsErrorMessage(QStringLiteral("CryptProtectData")));
        return {};
    }
    QByteArray encrypted(
        reinterpret_cast<const char*>(output.pbData),
        static_cast<qsizetype>(output.cbData));
    SecureZeroMemory(output.pbData, output.cbData);
    LocalFree(output.pbData);
    return encrypted;
}

QByteArray Unprotect(QByteArray* encrypted, QString* errorMessage) {
    QByteArray entropyBytes(kEntropy, sizeof(kEntropy) - 1);
    DATA_BLOB input{};
    DATA_BLOB entropy{};
    DATA_BLOB output{};
    if (!ToBlob(encrypted, &input) || !ToBlob(&entropyBytes, &entropy)) {
        SetError(errorMessage,
                 QStringLiteral("Encrypted refresh token data is invalid."));
        return {};
    }
    const BOOL unprotectedOk = CryptUnprotectData(
        &input,
        nullptr,
        &entropy,
        nullptr,
        nullptr,
        CRYPTPROTECT_UI_FORBIDDEN,
        &output);
    SecureClear(&entropyBytes);
    if (!unprotectedOk) {
        SetError(errorMessage,
                 WindowsErrorMessage(QStringLiteral("CryptUnprotectData")));
        return {};
    }
    QByteArray plaintext(
        reinterpret_cast<const char*>(output.pbData),
        static_cast<qsizetype>(output.cbData));
    SecureZeroMemory(output.pbData, output.cbData);
    LocalFree(output.pbData);
    return plaintext;
}

}  // namespace

DpapiTokenStore::DpapiTokenStore(QString credentialsPath)
    : credentialsPath_(credentialsPath.isEmpty()
                           ? DefaultCredentialsPath()
                           : std::move(credentialsPath)) {}

QString DpapiTokenStore::DefaultCredentialsPath() {
    return QDir(QStandardPaths::writableLocation(
                    QStandardPaths::AppLocalDataLocation))
        .filePath(QStringLiteral("auth/credentials.dat"));
}

TokenStoreLoadStatus DpapiTokenStore::Load(
    StoredRefreshToken* token,
    QString* errorMessage) {
    if (!token) {
        SetError(errorMessage,
                 QStringLiteral("Refresh token output is null."));
        return TokenStoreLoadStatus::kError;
    }
    QFile file(credentialsPath_);
    if (!file.exists()) {
        return TokenStoreLoadStatus::kNotFound;
    }
    if (!file.open(QIODevice::ReadOnly)) {
        SetError(errorMessage,
                 QStringLiteral("Unable to open encrypted credentials."));
        return TokenStoreLoadStatus::kError;
    }
    if (file.size() <= 0 || file.size() > kMaximumCredentialsBytes) {
        file.close();
        SetError(errorMessage,
                 QStringLiteral("Encrypted credentials have an invalid size."));
        return TokenStoreLoadStatus::kError;
    }
    QByteArray envelope = file.readAll();
    file.close();
    const QByteArray prefix(kEnvelopeMagic, sizeof(kEnvelopeMagic) - 1);
    if (!envelope.startsWith(prefix) ||
        envelope.size() <= prefix.size() ||
        envelope.at(prefix.size()) != kEnvelopeVersion) {
        SetError(errorMessage,
                 QStringLiteral("Encrypted credentials have an unsupported "
                                "format."));
        return TokenStoreLoadStatus::kError;
    }

    QByteArray encrypted = envelope.mid(prefix.size() + 1);
    QByteArray plaintext = Unprotect(&encrypted, errorMessage);
    SecureClear(&encrypted);
    if (plaintext.isEmpty()) {
        return TokenStoreLoadStatus::kError;
    }
    QJsonParseError parseError{};
    const QJsonDocument document =
        QJsonDocument::fromJson(plaintext, &parseError);
    SecureClear(&plaintext);
    if (parseError.error != QJsonParseError::NoError ||
        !document.isObject()) {
        SetError(errorMessage,
                 QStringLiteral("Decrypted credentials are malformed."));
        return TokenStoreLoadStatus::kError;
    }
    const QJsonObject object = document.object();
    if (object.value(QStringLiteral("schemaVersion")).toInt() != 1) {
        SetError(errorMessage,
                 QStringLiteral("Encrypted credentials use an unsupported "
                                "schema."));
        return TokenStoreLoadStatus::kError;
    }

    StoredRefreshToken loaded;
    loaded.issuer = object.value(QStringLiteral("issuer")).toString();
    loaded.clientId = object.value(QStringLiteral("clientId")).toString();
    loaded.refreshToken =
        object.value(QStringLiteral("refreshToken")).toString();
    loaded.savedAtUtc = QDateTime::fromString(
        object.value(QStringLiteral("savedAtUtc")).toString(), Qt::ISODate);
    if (loaded.issuer.isEmpty() || loaded.clientId.isEmpty() ||
        loaded.refreshToken.isEmpty()) {
        SetError(errorMessage,
                 QStringLiteral("Decrypted credentials are incomplete."));
        return TokenStoreLoadStatus::kError;
    }
    *token = std::move(loaded);
    return TokenStoreLoadStatus::kLoaded;
}

bool DpapiTokenStore::Save(
    const StoredRefreshToken& token,
    QString* errorMessage) {
    if (token.issuer.isEmpty() || token.clientId.isEmpty() ||
        token.refreshToken.isEmpty()) {
        SetError(errorMessage,
                 QStringLiteral("Refusing to save incomplete credentials."));
        return false;
    }
    QJsonObject object;
    object.insert(QStringLiteral("schemaVersion"), 1);
    object.insert(QStringLiteral("issuer"), token.issuer);
    object.insert(QStringLiteral("clientId"), token.clientId);
    object.insert(QStringLiteral("refreshToken"), token.refreshToken);
    object.insert(QStringLiteral("savedAtUtc"),
                  (token.savedAtUtc.isValid()
                       ? token.savedAtUtc.toUTC()
                       : QDateTime::currentDateTimeUtc())
                      .toString(Qt::ISODate));
    QByteArray plaintext =
        QJsonDocument(object).toJson(QJsonDocument::Compact);
    if (plaintext.size() > kMaximumCredentialsBytes) {
        SecureClear(&plaintext);
        SetError(errorMessage,
                 QStringLiteral("Credentials are too large to store."));
        return false;
    }
    QByteArray encrypted = Protect(&plaintext, errorMessage);
    if (encrypted.isEmpty()) {
        return false;
    }

    const QFileInfo fileInfo(credentialsPath_);
    if (!QDir().mkpath(fileInfo.absolutePath())) {
        SecureClear(&encrypted);
        SetError(errorMessage,
                 QStringLiteral("Unable to create the credentials directory."));
        return false;
    }
    QByteArray envelope(kEnvelopeMagic, sizeof(kEnvelopeMagic) - 1);
    envelope.append(kEnvelopeVersion);
    envelope.append(encrypted);
    SecureClear(&encrypted);

    QSaveFile file(credentialsPath_);
    if (!file.open(QIODevice::WriteOnly) ||
        file.write(envelope) != envelope.size() || !file.commit()) {
        SecureClear(&envelope);
        SetError(errorMessage,
                 QStringLiteral("Unable to atomically save encrypted "
                                "credentials."));
        return false;
    }
    SecureClear(&envelope);
    QFile::setPermissions(credentialsPath_,
                          QFileDevice::ReadOwner | QFileDevice::WriteOwner);
    return true;
}

bool DpapiTokenStore::Clear(QString* errorMessage) {
    if (!QFile::exists(credentialsPath_)) {
        return true;
    }
    if (!QFile::remove(credentialsPath_)) {
        SetError(errorMessage,
                 QStringLiteral("Unable to remove encrypted credentials."));
        return false;
    }
    return true;
}

}  // namespace remote::auth
