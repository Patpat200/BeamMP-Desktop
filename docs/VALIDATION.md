# Validation du prototype Desktop

Base : BeamMP/BeamMP-Launcher, commit `8a7d63e175911ed00c7d46a6101c24f9b68f2b55`.

Vérifications effectuées dans l'environnement Linux de développement :

- Compilation complète du moteur C++ modifié.
- Compilation de l'interface Qt Widgets avec Qt 6.8.3.
- Test du contrôleur de commandes réel avec services moteur simulés : authentification requise, connexion refusée et acceptée, invité, déconnexion, lancement et verrouillage du compte après lancement.
- Vérification de l'absence du mot de passe et des clés de test dans les sorties du contrôleur.
- Test de l'interface avec un processus enfant simulé : événements fragmentés, logs UTF-8, mot de passe masqué et effacé, connexion, invité, lancement, arrêt et redémarrage du moteur.
- Tests du choix automatique français/anglais, des choix manuels et du changement immédiat des textes statiques et de l’état du compte.
- Tests des arguments personnalisés, de leur sauvegarde et de leur transmission au processus enfant, des valeurs contenant des espaces et de la priorité du rendu personnalisé.
- Tests unitaires de l’échappement des arguments Windows, sans lancement réel de Windows.
- Inspection visuelle d'une capture de la fenêtre en mode de test.

Les tests ne contactent pas les services BeamMP et ne lancent pas BeamNG. La compilation Windows, l'authentification réelle, la détection de l'installation du jeu et une partie multijoueur restent à vérifier sur Windows. Le workflow Windows fourni n'a pas été exécuté depuis cet environnement.

L'archive contient les sources, pas un exécutable Windows précompilé. Voir DESKTOP-FR.md pour la compilation et les essais réels.
