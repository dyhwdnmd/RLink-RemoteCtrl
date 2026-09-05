// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include <QJsonArray>
#include <QJsonObject>
#include <QJsonValue>
#include <QString>

namespace remote::signaling_json {

QString ToQString(const std::string& value);
std::string ToString(const QString& value);
QJsonArray StringArray(const std::vector<std::string>& values);
std::vector<std::string> ReadStringArray(const QJsonValue& value);

bool ReadRequiredString(const QJsonObject& object,
                        const char* field,
                        std::string* value);
bool ReadRequiredBool(const QJsonObject& object,
                      const char* field,
                      bool* value);
bool ReadSafeUnsignedInteger(const QJsonObject& object,
                             const char* field,
                             std::uint64_t* value);

}  // namespace remote::signaling_json
