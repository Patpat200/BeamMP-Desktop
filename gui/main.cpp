// SPDX-License-Identifier: AGPL-3.0-or-later
#include "LauncherWindow.h"
#include <QApplication>
#include <QFont>
#include <QIcon>

int main(int argc, char** argv) {
    QApplication app(argc, argv);
    app.setOrganizationName("BeamMPCommunity");
    app.setApplicationName("BeamMP Desktop Preview");
    app.setFont(QFont("Segoe UI", 10));
    const QIcon icon(":/assets/launcher.ico");
    if (!icon.isNull()) app.setWindowIcon(icon);
    LauncherWindow window;
    window.show();
    return app.exec();
}
