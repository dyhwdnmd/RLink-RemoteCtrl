// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#include "SignalServerSupport.h"

#include <array>

#include <QFile>
#include <QRandomGenerator>

namespace remote::signaling_server::detail {
namespace {

void SetError(QString* error, const QString& message)
{
    if (error) {
        *error = message;
    }
}

}  // namespace

bool ReadFile(const QString& path,
              QByteArray* contents,
              QString* error)
{
    QFile file(path);
    if (!contents || !file.open(QIODevice::ReadOnly)) {
        SetError(error, QStringLiteral("Cannot open %1: %2")
                            .arg(path, file.errorString()));
        return false;
    }
    *contents = file.readAll();
    return true;
}

QString CreateRecoveryToken()
{
    std::array<quint32, 8> randomWords{};
    QRandomGenerator::system()->fillRange(
        randomWords.data(), randomWords.size());
    const QByteArray bytes(
        reinterpret_cast<const char*>(randomWords.data()),
        static_cast<qsizetype>(sizeof(randomWords)));
    return QString::fromLatin1(bytes.toBase64(
        QByteArray::Base64UrlEncoding |
        QByteArray::OmitTrailingEquals));
}

}  // namespace remote::signaling_server::detail
