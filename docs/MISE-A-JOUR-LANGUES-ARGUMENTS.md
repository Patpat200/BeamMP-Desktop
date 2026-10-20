# Mise à jour : français/anglais et arguments de lancement

1. Fermer BeamMP Desktop et le jeu.
2. Extraire le ZIP de mise à jour dans le dossier du projet qui contient CMakeLists.txt. Fusionner les dossiers et remplacer les fichiers concernés. Ne pas copier ces sources dans dist-desktop.
3. Relancer la commande tools/build-windows.ps1 précédemment utilisée avec la bonne installation Qt MSVC et le même dossier de build.
4. Après réussite, ouvrir dist-desktop/BeamMP-Desktop.exe. Le moteur BeamMP-Launcher.exe doit aussi être recompilé : il contient la correction des arguments Windows avec espaces.

Dans Paramètres :

- Langue : Automatique (langue du PC), Français ou English. Le choix s’applique immédiatement et reste mémorisé. En automatique, un système en français utilise le français ; les autres langues utilisent l’anglais.
- Arguments de lancement : par exemple `--game -gfx dx11`. Cliquer sur Enregistrer et redémarrer le moteur pour appliquer.
- Un argument de rendu `-gfx` personnalisé prend priorité sur le menu Rendu graphique.
- Les chemins contenant des espaces doivent être entre guillemets doubles.

Les journaux bruts du moteur et les messages du serveur d’authentification ne sont pas traduits.

Validation : compilation Linux du moteur et de l’interface Qt réussie ; tests de traduction, de sauvegarde, de transmission au processus enfant et d’échappement des arguments réussis. La partie réelle sous Windows reste à vérifier.
