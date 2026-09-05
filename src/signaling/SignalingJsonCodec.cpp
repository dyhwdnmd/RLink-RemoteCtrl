// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#include "SignalingJsonCodec.h"

#include <QByteArray>

namespace remote::signaling_json {

QString ToQString(const std::string& value)
{
    return QString::fromUtf8(
        value.data(), static_cast<qsizetype>(value.size()));
}

std::string ToString(const QString& value)
{
    const QByteArray utf8 = value.toUtf8();
    return {utf8.constData(), static_cast<std::size_t>(utf8.size())};
}

QJsonArray StringArray(const std::vector<std::string>& values)
{
    QJsonArray array;
    for (const auto& value : values) {
        array.append(ToQString(value));
    }
    return array;
}

std::vector<std::string> ReadStringArray(const QJsonValue& value)
{
    std::vector<std::string> values;
    if (!value.isArray()) {
        return values;
    }
    const auto array = value.toArray();
    values.reserve(static_cast<std::size_t>(array.size()));
    for (const auto& entry : array) {
        if (entry.isString()) {
            values.push_back(ToString(entry.toString()));
        }
    }
    return values;
}

bool ReadRequiredString(const QJsonObject& object,
                        const char* field,
                        std::string* value)
{
    const QJsonValue jsonValue =
        object.value(QString::fromLatin1(field));
    if (!value || !jsonValue.isString()) {
        return false;
    }
    *value = ToString(jsonValue.toString());
    return !value->empty();
}

bool ReadRequiredBool(const QJsonObject& object,
                      const char* field,
                      bool* value)
{
    const QJsonValue jsonValue =
        object.value(QString::fromLatin1(field));
    if (!value || !jsonValue.isBool()) {
        return false;
    }
    *value = jsonValue.toBool();
    return true;
}

bool ReadSafeUnsignedInteger(const QJsonObject& object,
                             const char* field,
                             std::uint64_t* value)
{
    const QJsonValue jsonValue =
        object.value(QString::fromLatin1(field));
    if (!value || !jsonValue.isDouble()) {
        return false;
    }
    const double number = jsonValue.toDouble(-1.0);
    constexpr double kMaximumSafeJsonInteger = 9007199254740991.0;
    if (number < 0.0 || number > kMaximumSafeJsonInteger ||
        number != static_cast<double>(
            static_cast<std::uint64_t>(number))) {
        return false;
    }
    *value = static_cast<std::uint64_t>(number);
    return true;
}

}  // namespace remote::signaling_json
