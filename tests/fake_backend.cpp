// SPDX-License-Identifier: AGPL-3.0-or-later
// Offline subprocess fixture. Does not contact BeamMP or launch a game.
#include <QCoreApplication>
#include <QJsonDocument>
#include <QJsonObject>
#include <QThread>
#include <iostream>

static void emitEvent(const QJsonObject& object) {
    const auto packet = QByteArray("@@BEAMMP_GUI@@") + QJsonDocument(object).toJson(QJsonDocument::Compact) + '\n';
    // Deliberately split packets to exercise stream framing.
    const auto middle = packet.size() / 2;
    std::cerr.write(packet.constData(), middle);
    std::cerr.flush();
    QThread::msleep(5);
    std::cerr.write(packet.constData() + middle, packet.size() - middle);
    std::cerr.flush();
}
int main(int argc, char** argv) {
    QCoreApplication application(argc, argv);
    const auto arguments = application.arguments().mid(1).join(" ").toUtf8();
    std::cout << "[test argv] " << arguments.constData() << '\n' << std::flush;
    bool authenticated = false;
    QString name;
    emitEvent({{"event", "ready"}, {"success", true}, {"authenticated", false}});
    const auto utf8 = QString::fromUtf8("Moteur de test prêt — aucune connexion réseau.\n").toUtf8();
    std::cout.write(utf8.constData(), utf8.size() - 2);
    std::cout.flush();
    QThread::msleep(5);
    std::cout.write(utf8.constData() + utf8.size() - 2, 2);
    std::cout.flush();
    std::string line;
    while (std::getline(std::cin, line)) {
        const auto data = QJsonDocument::fromJson(QByteArray::fromStdString(line)).object();
        const auto action = data.value("action").toString();
        if (action == "login") {
            authenticated = data.value("username").toString() == "tester"
                && data.value("password").toString() == "not-a-real-password";
            name = authenticated ? "Test Driver" : "";
        } else if (action == "guest") {
            authenticated = true;
            name = "guest123";
        } else if (action == "logout") {
            authenticated = false;
            name.clear();
        } else if (action == "play") {
            emitEvent({{"event", "preparing"}});
            emitEvent({{"event", "playing"}});
            QThread::msleep(700);
            return 0; // Simulate game closing: frontend must remain open.
        }
        emitEvent({{"event", "auth"}, {"authenticated", authenticated},
            {"success", action == "logout" || authenticated}, {"username", name},
            {"message", authenticated ? "OK" : "Rejected test credentials"}});
    }
    return 0;
}
