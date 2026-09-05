// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#pragma once

#include <QByteArray>
#include <QString>

namespace remote::signaling_server::detail {

bool ReadFile(const QString& path,
              QByteArray* contents,
              QString* error);
QString CreateRecoveryToken();

}  // namespace remote::signaling_server::detail
