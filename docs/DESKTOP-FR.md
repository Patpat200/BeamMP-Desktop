# BeamMP Desktop — version personnelle de développement

Cette version ajoute une interface Qt au moteur C++ du launcher BeamMP :

- Accueil avec bouton Jouer et état de la session.
- Compte : connexion BeamMP, mode invité, déconnexion et reprise de la session mémorisée par le moteur.
- Console : journaux en direct, recherche, effacement de la vue et export.
- Paramètres : langue, exécutable moteur, dossier utilisateur BeamNG, DirectX 11/12/Vulkan, arguments libres, conservation du mod local et logs détaillés.

Il s’agit d’une modification communautaire, pas d’une version officielle distribuée par BeamMP.

## Important : ce dossier contient des sources

Un fichier `.cpp` ne se copie pas dans le ZIP Lua de BeamMP. Il faut compiler le programme pour obtenir les exécutables Windows. La compilation et les tests Windows sont à effectuer sur Windows ou avec le workflow fourni. Le fonctionnement réel de l’authentification et d’une partie multijoueur doit être vérifié avec une installation BeamNG et un compte réels.

## Option 1 — obtenir les exécutables avec GitHub Actions

1. Utiliser un fork **du dépôt BeamMP-Launcher** (pas le dépôt Lua BeamMP).
2. Ajouter les fichiers de ce projet sur une nouvelle branche de travail de ce fork. Conserver les workflows et la licence.
3. Vérifier que le workflow `Build desktop preview (Windows)` est disponible. GitHub peut demander d’activer Actions sur le fork. Le workflow doit être présent sur la branche par défaut pour apparaître dans la liste de lancement manuel ; on peut ensuite sélectionner la branche à compiler.
4. Ouvrir **Actions → Build desktop preview (Windows) → Run workflow**.
5. Télécharger et extraire l’artefact `BeamMP-Desktop-Windows` lorsque le build et les tests réussissent.
6. Lancer **BeamMP-Desktop.exe**, en laissant BeamMP-Launcher.exe et les DLL Qt à côté.

Aucun secret GitHub ni mot de passe BeamMP n’est nécessaire pour compiler. Le workflow ne publie pas de release.

## Option 2 — compiler sur Windows

Installer :

- Visual Studio 2022 ou Build Tools avec **Développement Desktop en C++** et le SDK Windows.
- CMake 3.21 ou plus et Git.
- Python 3 pour les tests du protocole.
- Qt **6.8.3**, composant **MSVC 2022 64-bit**, avec Qt Widgets (Qt Base, incluant Qt Test).
- vcpkg, initialisé avec `bootstrap-vcpkg.bat`.

Depuis le **Developer PowerShell for VS 2022**, à la racine du projet :

```powershell
./tools/build-windows.ps1 -QtRoot "C:\Qt\6.8.3\msvc2022_64" -VcpkgRoot "C:\dev\vcpkg"
```

Adapter les chemins à l’installation. Le script arrête la compilation dès qu’une commande échoue, lance les tests puis copie les exécutables et les dépendances Qt dans `dist-desktop`.

La première compilation peut être longue : vcpkg compile les dépendances du moteur. Utiliser un dossier de build propre si une ancienne configuration utilise un autre triplet ou un autre compilateur.

## Utiliser le launcher

1. Copier le contenu de `dist-desktop` dans un dossier personnel accessible en écriture, en conservant les exécutables ensemble.
2. Fermer les autres instances du launcher BeamMP : elles pourraient utiliser le même port local.
3. Ouvrir `BeamMP-Desktop.exe`.
4. Le moteur cherche le jeu comme le launcher officiel. Si le jeu n’est pas détecté, consulter la Console et configurer le chemin BeamNG dans le fichier `Launcher.cfg` du moteur suivant le fonctionnement officiel.
5. Ouvrir **Compte**, puis se connecter ou choisir le mode invité.
6. Ouvrir **Accueil → Jouer**. Le jeu ne démarre pas avant ce clic.
7. Conserver l’interface ouverte pendant la partie. La réduire ne coupe pas la connexion ; quitter le launcher la coupe.
8. Après fermeture du jeu, l’interface reste ouverte. Utiliser **Paramètres → Redémarrer le moteur** pour rejouer.

La Console du launcher affiche les logs du moteur, pas les logs internes ni la console Lua de BeamNG. Elle conserve les 10 000 dernières lignes ; le moteur garde aussi son journal habituel `Launcher.log`.

Les paramètres sont mémorisés via QSettings, dans un espace dédié à cette application. L’interface ne sauvegarde aucun mot de passe. Le moteur conserve sa gestion habituelle du fichier de session `key` dans son dossier de travail. La connexion graphique utilise la même fonction et le même service d’authentification que le jeu ; ce n’est pas un nouveau système de comptes.

## Langue et arguments de lancement

Dans **Paramètres → Langue**, choisir **Automatique (langue du PC)**, **Français** ou **English**. Au premier démarrage, le mode automatique utilise le français sur les systèmes en français, et l’anglais sur les autres systèmes. Le choix manuel est mémorisé et s’applique immédiatement sans redémarrer le moteur. Les messages bruts du moteur et du serveur d’authentification conservent leur langue d’origine.

Le champ **Arguments de lancement** accepte par exemple :

```text
--game -gfx dx11
```

Les arguments avant `--game` (ou `--`) sont destinés au launcher ; tous ceux qui suivent sont transmis au jeu. Un `-gfx` fourni dans ce champ prend priorité sur le menu Rendu graphique. Mettre les valeurs contenant des espaces entre guillemets doubles, par exemple `--user-path "C:\BeamNG User" --game -gfx dx11`. Les arguments sont transmis au processus directement, sans interpréteur de commandes : ce champ n’exécute pas des commandes PowerShell.

Cliquer sur **Enregistrer et redémarrer le moteur** pour appliquer les arguments. Ils sont aussi conservés à la fermeture de l’application. Les arguments et les autres réglages de lancement sont verrouillés pendant une opération ou une partie.

## Détails d’architecture

- `BeamMP-Desktop` est un frontend Qt Widgets indépendant.
- `BeamMP-Launcher --gui-backend` est le moteur existant, avec un petit contrôleur de commandes.
- Les commandes JSON transitent dans l’entrée standard privée du processus enfant. Les événements reviennent sur stderr avec le préfixe `@@BEAMMP_GUI@@`. Les logs ordinaires utilisent stdout. Aucun nouveau port réseau de contrôle n’est ajouté.
- La connexion, le mode invité et la déconnexion passent par `Login()` existant. Les réponses transmises au frontend sont limitées à l’état du compte et au message de résultat ; aucune clé d’authentification n’est transmise au frontend.
- Les changements de compte et de paramètres sont désactivés pendant une session pour ne pas ajouter de concurrence sur l’état d’authentification existant.
- Le moteur graphique désactive sa propre mise à jour automatique : une mise à jour officielle remplacerait cet exécutable modifié et supprimerait son protocole. Les mises à jour du mod restent actives par défaut.
- Le launcher console traditionnel reste disponible : compiler sans `BEAMMP_BUILD_GUI`, ou lancer BeamMP-Launcher sans l’argument interne.
- L’interface et le moteur sont deux exécutables à distribuer ensemble, pas un exécutable autonome unique.

## Vérification avant usage réel

- Connexion valide, refus de connexion et session invitée.
- Nom de compte visible dans le jeu après lancement et connexion à un serveur.
- Paramètres DirectX 11/12/Vulkan et dossier utilisateur avec espaces.
- Logs affichés sans terminal séparé, export fonctionnel.
- Fermeture de BeamNG, redémarrage du moteur et seconde partie.
- Réduction de la fenêtre pendant une partie ; confirmation lors de sa fermeture.
- Mise à jour du mod et conservation du mod local en mode développement.

Les tests automatiques ne contactent pas l’authentification officielle et ne lancent pas BeamNG. Ils utilisent un moteur factice pour tester les flux de l’interface. Ils ne prouvent pas qu’une partie réelle fonctionne.

## Pour une contribution future

Conserver l’interface facultative et discuter le choix Qt, la distribution et le fonctionnement des comptes avec les mainteneurs. La compilation nécessite de nouvelles dépendances et cette version personnelle n’a pas été acceptée par BeamMP.

Le code dérivé conserve la licence AGPL-3.0-or-later du dépôt original. Qt est distribué sous ses propres conditions ; cette configuration utilise des DLL Qt remplaçables, et non une édition Qt liée statiquement.
