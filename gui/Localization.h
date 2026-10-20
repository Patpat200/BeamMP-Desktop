// SPDX-License-Identifier: AGPL-3.0-or-later
#pragma once
#include <QHash>
#include <QLocale>
#include <QString>

namespace DesktopI18n {
inline bool useEnglish(const QString& preference, const QLocale& system = QLocale::system()) {
    return preference == "en" || (preference != "fr" && system.language() != QLocale::French);
}
inline QString text(const QString& source, bool english) {
    if (!english) return source;
    static const QHash<QString, QString> messages{
        {"DESKTOP PREVIEW\nVersion communautaire", "DESKTOP PREVIEW\nCommunity version"},
        {"Accueil", "Home"}, {"Compte", "Account"}, {"Paramètres", "Settings"},
        {"Compte non connecté", "Not signed in"}, {"Site BeamMP ↗", "BeamMP website ↗"},
        {"Initialisation…", "Initializing…"},
        {"Le moteur ne répond pas. Vérifie la console et utilise le moteur compilé avec cette interface.", "The backend is not responding. Check the console and use the backend built with this interface."},
        {"Opération trop longue. Le moteur a été arrêté ; tu peux le redémarrer.", "The operation timed out. The backend was stopped; you can restart it."},
        {"Impossible de démarrer le moteur : %1", "Unable to start the backend: %1"},
        {"\n[Moteur arrêté — code %1]\n", "\n[Backend stopped — code %1]\n"},
        {"Session terminée. Redémarre le moteur pour rejouer.", "Session ended. Restart the backend to play again."},
        {"Moteur arrêté. Consulte la console puis redémarre-le.", "Backend stopped. Check the console, then restart it."},
        {"MULTIJOUEUR POUR BEAMNG.DRIVE", "MULTIPLAYER FOR BEAMNG.DRIVE"},
        {"La route se partage.", "The road is better together."},
        {"Ton compte, le lancement du jeu et les journaux, réunis dans une seule fenêtre.", "Your account, game launch and logs, together in one window."},
        {"Prêt pour la prochaine\nvirée ensemble ?", "Ready for the next\ndrive together?"},
        {"Préparation du launcher…", "Preparing the launcher…"}, {"Jouer à BeamMP  →", "Play BeamMP  →"},
        {"Préparation du mod et démarrage de BeamNG…", "Preparing the mod and starting BeamNG…"},
        {"Gérer mon compte", "Manage my account"}, {"Voir la console", "View console"},
        {"Laisse cette fenêtre ouverte pendant la partie. Les serveurs et les favoris restent accessibles dans le jeu.", "Keep this window open while playing. Servers and favorites remain available in the game."},
        {"Ton compte", "Your account"},
        {"Connecte-toi avec ton compte BeamMP ou continue en invité. Le mot de passe n’est pas enregistré par l’interface.", "Sign in with your BeamMP account or continue as a guest. The interface does not save your password."},
        {"Nom d’utilisateur ou adresse e-mail", "Username or email address"},
        {"Mot de passe", "Password"}, {"Identifiant", "Username"}, {"Se connecter", "Sign in"},
        {"Renseigne ton identifiant et ton mot de passe.", "Enter your username and password."},
        {"Connexion au service BeamMP…", "Signing in to BeamMP…"},
        {"Continuer en invité", "Continue as guest"}, {"Ouverture d’une session invitée…", "Starting a guest session…"},
        {"Se déconnecter", "Sign out"},
        {"La session mémorisée est gérée par le launcher BeamMP existant. Les paramètres de cette interface ne contiennent aucun mot de passe.", "Saved sessions are managed by the existing BeamMP launcher. The interface settings contain no passwords."},
        {"Console du launcher", "Launcher console"},
        {"Les messages du moteur BeamMP s’affichent ici en direct. Il s’agit d’un journal, pas d’une console Lua du jeu.", "Live BeamMP backend messages appear here. This is a log, not the in-game Lua console."},
        {"Rechercher dans les messages…", "Search messages…"}, {"Suivant", "Next"}, {"Effacer", "Clear"},
        {"Exporter…", "Export…"}, {"Exporter la console", "Export console"},
        {"Journal (*.log);;Texte (*.txt)", "Log (*.log);;Text (*.txt)"},
        {"Impossible d’écrire le journal.", "Unable to write the log."}, {"Journal exporté.", "Log exported."},
        {"Ces réglages prennent effet au redémarrage du moteur. Le jeu est détecté par le launcher BeamMP.", "Launch settings take effect when the backend restarts. The BeamMP launcher detects the game."},
        {"Parcourir…", "Browse…"}, {"Moteur BeamMP compilé avec l’interface", "BeamMP backend built with this interface"},
        {"Automatique — laisser vide", "Automatic — leave blank"}, {"Par défaut", "Default"},
        {"Moteur BeamMP", "BeamMP backend"}, {"Dossier utilisateur BeamNG", "BeamNG user folder"},
        {"Rendu graphique", "Graphics renderer"}, {"Langue", "Language"},
        {"Automatique (langue du PC)", "Automatic (PC language)"},
        {"Arguments de lancement", "Launch arguments"},
        {"Exemple : --game -gfx dx11", "Example: --game -gfx dx11"},
        {"Arguments du launcher ; après --game, arguments du jeu. Entoure les chemins contenant des espaces de guillemets. Un -gfx personnalisé remplace le rendu choisi ci-dessus.", "Launcher arguments; after --game, game arguments. Put paths containing spaces in double quotes. A custom -gfx overrides the renderer selected above."},
        {"Ferme le guillemet dans les arguments de lancement.", "Close the double quote in the launch arguments."},
        {"Conserver mon mod local sans le retélécharger (développement)", "Keep my local mod without downloading it again (development)"},
        {"Afficher les messages de débogage du launcher", "Show launcher debug messages"},
        {"Enregistrer et redémarrer le moteur", "Save and restart backend"}, {"Redémarrer le moteur", "Restart backend"},
        {"Cette version personnelle ne met pas automatiquement à jour son exécutable moteur, afin de conserver l’interface. Le mod reste mis à jour normalement, sauf si l’option de développement est cochée.", "This personal version does not automatically update its backend executable, to preserve the interface. The mod updates normally unless development mode is enabled."},
        {"Moteur introuvable. Dans Paramètres, sélectionne le BeamMP-Launcher compilé avec ce projet.", "Backend not found. In Settings, select the BeamMP-Launcher built with this project."},
        {"Sélectionne BeamMP-Launcher, pas BeamMP-Desktop lui-même.", "Select BeamMP-Launcher, rather than BeamMP-Desktop itself."},
        {"Le dossier utilisateur indiqué n’existe pas.", "The selected user folder does not exist."},
        {"Initialisation du moteur BeamMP…", "Initializing the BeamMP backend…"},
        {"\n[Démarrage du moteur]\n", "\n[Starting backend]\n"},
        {"[Message moteur trop volumineux ignoré]\n", "[Oversized backend message ignored]\n"},
        {"Connexion refusée.", "Sign-in rejected."}, {"Compte prêt. Tu peux lancer le jeu.", "Account ready. You can launch the game."},
        {"Choisis ton compte ou le mode invité.", "Sign in or choose guest mode."},
        {"Vérification du mod et préparation du jeu…", "Checking the mod and preparing the game…"},
        {"Lancement demandé. Garde le launcher ouvert pendant la partie.", "Launch requested. Keep the launcher open while playing."},
        {"Erreur du moteur.", "Backend error."}, {"Session invitée", "Guest session"},
        {"Session en cours — laisse le launcher ouvert.", "Session in progress — keep the launcher open."},
        {"Une opération est en cours…", "An operation is in progress…"},
        {"Connecté : %1. Le jeu est prêt à être lancé.", "Signed in as %1. The game is ready to launch."},
        {"Connecte-toi dans l’onglet Compte pour commencer.", "Sign in on the Account tab to get started."},
        {"Le moteur doit être démarré depuis l’onglet Paramètres.", "Start the backend from the Settings tab."},
        {"Quitter BeamMP ?", "Quit BeamMP?"},
        {"Fermer le launcher coupe la connexion multijoueur. Quitter quand même ?", "Closing the launcher disconnects multiplayer. Quit anyway?"}
    };
    return messages.value(source, source);
}
}
