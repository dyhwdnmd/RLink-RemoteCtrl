// SPDX-License-Identifier: GPL-3.0-only
// Copyright (c) 2026 dyhwdnmd (https://github.com/dyhwdnmd)

#include <QApplication>
#include <QTextStream>

#include "ControllerMainWindow.h"
#include "src/webrtc/WebRtcRuntime.h"

int main(int argc, char* argv[])
{
    QApplication application(argc, argv);
    application.setApplicationName(QStringLiteral("ControllerApp"));
    application.setApplicationDisplayName(QStringLiteral("RLink"));
    application.setOrganizationName(QStringLiteral("RemoteC"));

    if (application.arguments().contains(QStringLiteral("--status-once"))) {
        remote::WebRtcRuntime runtime;
        const bool ready = runtime.Initialize();
        QTextStream output(stdout);
        output << "CONTROLLER_PRODUCT_SHELL_READY="
               << (ready ? "YES" : "NO") << Qt::endl;
        if (!ready) {
            output << QString::fromStdString(runtime.CapabilityReport().error)
                   << Qt::endl;
        }
        runtime.Shutdown();
        return ready ? 0 : 1;
    }

    remote::controller::ControllerMainWindow window;
    window.show();
    return application.exec();
}
