// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#pragma once

#include <algorithm>
#include <utility>

#include <QDateTime>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSettings>
#include <QString>

namespace remote::app {

inline constexpr auto kEncoderBenchmarkProfilesSetting =
    "media/encoderProbe/profiles";

inline bool EncoderBenchmarkProfileMatches(
    const QJsonObject& profile,
    const QString& hardwareFingerprint,
    const QString& captureBackend,
    const QString& quality,
    int policyVersion)
{
    return profile.value(QStringLiteral("hardwareFingerprint")).toString() ==
               hardwareFingerprint &&
           profile.value(QStringLiteral("captureBackend")).toString() ==
               captureBackend &&
           profile.value(QStringLiteral("quality")).toString() == quality &&
           profile.value(QStringLiteral("policyVersion")).toInt() ==
               policyVersion &&
           profile.value(QStringLiteral("completed")).toBool();
}

inline QJsonObject LoadEncoderBenchmarkProfile(
    const QSettings& settings,
    const QString& hardwareFingerprint,
    const QString& captureBackend,
    const QString& quality,
    int policyVersion)
{
    const QJsonArray profiles = QJsonDocument::fromJson(
        settings.value(QString::fromLatin1(
                           kEncoderBenchmarkProfilesSetting))
            .toByteArray())
                                    .array();
    for (const auto& value : profiles) {
        const QJsonObject profile = value.toObject();
        if (EncoderBenchmarkProfileMatches(
                profile, hardwareFingerprint, captureBackend, quality,
                policyVersion)) {
            return profile;
        }
    }

    // Read the former single-slot cache so existing installations retain
    // their most recent result after upgrading to the profile cache.
    if (settings.value(QStringLiteral("media/encoderProbe/completed"), false)
            .toBool() &&
        settings.value(QStringLiteral("media/encoderProbe/policyVersion"), 0)
                .toInt() == policyVersion &&
        settings.value(
                    QStringLiteral("media/encoderProbe/hardwareFingerprint"))
                .toString() == hardwareFingerprint &&
        settings.value(QStringLiteral("media/encoderProbe/captureBackend"))
                .toString() == captureBackend &&
        settings.value(QStringLiteral("media/encoderProbe/ffmpegX264Preset"))
                .toString() == quality) {
        QJsonObject legacy;
        legacy.insert(QStringLiteral("hardwareFingerprint"),
                      hardwareFingerprint);
        legacy.insert(QStringLiteral("captureBackend"), captureBackend);
        legacy.insert(QStringLiteral("quality"), quality);
        legacy.insert(QStringLiteral("policyVersion"), policyVersion);
        legacy.insert(QStringLiteral("completed"), true);
        legacy.insert(
            QStringLiteral("passed"),
            settings.value(QStringLiteral("media/encoderProbe/passed"), false)
                .toBool());
        legacy.insert(
            QStringLiteral("bestEncoderId"),
            settings.value(QStringLiteral("media/encoderProbe/bestEncoderId"))
                .toString());
        legacy.insert(
            QStringLiteral("bestEncoderName"),
            settings.value(
                        QStringLiteral("media/encoderProbe/bestEncoderName"))
                .toString());
        legacy.insert(
            QStringLiteral("testedAtUtc"),
            settings.value(QStringLiteral("media/encoderProbe/testedAtUtc"))
                .toDateTime()
                .toUTC()
                .toString(Qt::ISODateWithMs));
        legacy.insert(
            QStringLiteral("candidates"),
            QJsonDocument::fromJson(
                settings.value(QStringLiteral("media/encoderProbe/candidates"))
                    .toByteArray())
                .array());
        return legacy;
    }
    return {};
}

inline void SaveEncoderBenchmarkProfile(
    QSettings& settings, QJsonObject profile)
{
    QJsonArray existing = QJsonDocument::fromJson(
        settings.value(QString::fromLatin1(
                           kEncoderBenchmarkProfilesSetting))
            .toByteArray())
                              .array();
    QJsonArray updated;
    updated.push_back(profile);
    for (const auto& value : existing) {
        const QJsonObject candidate = value.toObject();
        if (EncoderBenchmarkProfileMatches(
                candidate,
                profile.value(QStringLiteral("hardwareFingerprint"))
                    .toString(),
                profile.value(QStringLiteral("captureBackend")).toString(),
                profile.value(QStringLiteral("quality")).toString(),
                profile.value(QStringLiteral("policyVersion")).toInt())) {
            continue;
        }
        // A small upper bound prevents obsolete driver/configuration history
        // from growing QSettings indefinitely while retaining many profiles.
        if (updated.size() >= 32) {
            break;
        }
        updated.push_back(candidate);
    }
    settings.setValue(
        QString::fromLatin1(kEncoderBenchmarkProfilesSetting),
        QJsonDocument(updated).toJson(QJsonDocument::Compact));
}

}  // namespace remote::app
