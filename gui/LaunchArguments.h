// SPDX-License-Identifier: AGPL-3.0-or-later
#pragma once
#include <QProcess>
#include <QStringList>

namespace DesktopArguments {
inline bool balancedQuotes(const QString& value) {
    return value.count('"') % 2 == 0;
}
// QProcess receives individual arguments, never a shell command. The existing
// core consumes everything after --game/-- as BeamNG arguments.
inline QStringList build(const QString& custom, const QString& renderer) {
    auto launcher = QProcess::splitCommand(custom);
    QStringList game;
    for (qsizetype i = 0; i < launcher.size(); ++i) {
        if (launcher[i] == "--game" || launcher[i] == "--") {
            game = launcher.mid(i + 1);
            launcher = launcher.mid(0, i);
            break;
        }
    }
    if (renderer != "default" && !game.contains("-gfx")) game = QStringList{"-gfx", renderer} + game;
    if (!game.isEmpty()) launcher << "--game" << game;
    return launcher;
}
}
