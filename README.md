# WOZT GTK Standard

**Un standard de conception pour créer et moderniser des interfaces GTK sous Linux**, issu du travail réalisé sur [WUP Stream](https://github.com/wozt/wup-stream-server-linux).

Il sert de référence aux développeurs et aux assistants IA (Codex, SWE-2, etc.) pour obtenir des applications visuellement cohérentes sans sacrifier leurs fonctionnalités.

Le standard couvre notamment :

- Une fenêtre sans barre de titre GTK native, avec icône et commandes intégrées, déplaçable et redimensionnable sur ses huit bords et coins.
- Une interface compacte, responsive, privilégiant une page principale, tout en autorisant onglets et fenêtres secondaires si nécessaire.
- **7 thèmes JSON** : Graphite, Light, Ocean, Emerald, Rose, Apple Lime et Steam Legacy.
- Des traductions en **JSON**, modifiables à chaud, y compris pour les menus et les infobulles.
- Un style visuel homogène et une distribution Linux en **AppImage x86_64**.

## Utilisation

Lire **[standard.md](standard.md)** et demander à l'agent de développement de l'appliquer au projet concerné, en conservant ses fonctionnalités et ses performances.

Le dossier [`examples/`](examples/) fournit les thèmes, les fichiers de langues et un exemple de script de construction AppImage. Ces ressources sont des **références à adapter** : le dépôt n'est pas une bibliothèque GTK prête à intégrer telle quelle.
