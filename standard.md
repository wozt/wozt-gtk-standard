# WOZT GTK Standard

> **Version : 1.0 — 23 novembre 2026**  
> **Objet :** standard visuel, ergonomique et technique réutilisable pour les interfaces de bureau des projets WOZT.  
> **Référence initiale :** [`wozt/wup-stream-server-linux`](https://github.com/wozt/wup-stream-server-linux), en particulier `src/ui.c`, `src/i18n.c`, `src/settings.c` et `data/themes/`, `data/languages/`.  
> **Statut :** spécification pour la création et la refonte d'interfaces ; ce dépôt ne constitue pas, à lui seul, une bibliothèque GTK prête à l'emploi.

## 0. Règle générale

Créer des interfaces **cohérentes, compactes, modernes, lisibles, personnalisables, traduisibles et réellement redimensionnables**, sans dépendre d'une barre de titre GTK standard. Favoriser un **tableau de bord unique** qui expose les fonctions importantes d'un seul regard.

**Ne pas copier mécaniquement le code de WUP Stream.** En reprendre les principes, puis adapter les intitulés, fonctions, panneaux, seuils de disposition et choix techniques au logiciel concerné. La qualité de l'interaction et le fonctionnement existant ont priorité sur la ressemblance pixel par pixel.

### Priorité des exigences

1. **Ne pas casser les fonctionnalités existantes** : traitements, threads, performances, paramètres, contrôles, raccourcis, sauvegardes et accessibilité.
2. **Fenêtre personnalisée entièrement utilisable** : déplacer, réduire, maximiser/restaurer, fermer, redimensionner sur les huit bords et coins.
3. **Une page principale compacte et responsive** : accès immédiat aux réglages et indicateurs importants ; pas de défilement horizontal.
4. **Sept thèmes de base pilotés par JSON** et applicables à chaud.
5. **Internationalisation par JSON**, applicable à chaud et complète, y compris pour les menus, popups et contenus dessinés.
6. **Style homogène**, informations hiérarchisées, états visibles et tests réels.
7. **Distribution Linux en AppImage x86_64** : build reproductible, ressources embarquées, permissions saines et vérification sur d’autres systèmes.

---

## 1. Fenêtre sans barre GTK native

### Comportement obligatoire

- Désactiver la décoration de fenêtre native et **intégrer une barre supérieure personnalisée directement dans la surface de l'application**.
- Cette barre réunit, selon les besoins de l'application : **icône**, **nom**, **statut essentiel**, **action principale**, **choix du thème**, **choix de la langue** et **commandes de fenêtre**.
- Afficher dans l'ordre habituel les commandes **réduire**, **maximiser/restaurer** et **fermer**, avec des pictogrammes symboliques et une alternative textuelle si les icônes manquent.
- La zone non interactive de la barre supérieure permet le **déplacement par glisser-déposer**. Un **double-clic** sur la zone de déplacement maximise/restaure la fenêtre.
- Les menus, boutons et sélecteurs conservent leur propre gestion du clic : ils ne doivent pas déplacer la fenêtre par accident.
- Le bouton fermer respecte la sémantique du logiciel : quitter réellement, ou **masquer vers la zone de notification** seulement si cette fonction est effectivement proposée, documentée et visible.
- Préserver l'icône du programme dans les métadonnées de fenêtre, la barre des tâches et les fichiers `.desktop`, même quand le titre natif est masqué.
- Conserver la navigation clavier, les états de focus, les infobulles et des descriptions accessibles pour les boutons à icône seule.

**Exemple GTK3 :**

```c
gtk_window_set_decorated(GTK_WINDOW(window), FALSE);
gtk_window_set_resizable(GTK_WINDOW(window), TRUE);

/* Sur une zone de titre dédiée, lors du clic gauche : */
gtk_window_begin_move_drag(GTK_WINDOW(window),
                           event->button,
                           (int)event->x_root,
                           (int)event->y_root,
                           event->time);
```

Pour les commandes de la barre : `gtk_window_iconify()`, `gtk_window_maximize()`, `gtk_window_unmaximize()` et, selon le contrat de l'application, `gtk_window_close()` ou `gtk_widget_hide()`.

> **Portabilité :** la référence WUP Stream est en **GTK3/X11**. Sur GTK4, Wayland, ou d'autres gestionnaires de fenêtres, adapter les mécanismes d'entrée et vérifier les interactions sur le système cible. Ne pas supposer que les stratégies d'overlay/hit-test X11 se comportent identiquement partout.

---

## 2. Redimensionnement réel malgré l'absence de décoration

**Une fenêtre sans bordure native doit rester aussi redimensionnable qu'une fenêtre normale.** `gtk_window_set_resizable(TRUE)` ne suffit pas toujours à fournir des zones de saisie lorsque le décorateur a disparu.

### Exigences

- Prévoir **huit zones de redimensionnement** : haut, bas, gauche, droite, et quatre coins.
- Les zones sont **transparentes** et placées en surimpression sans perturber la mise en page.
- La souris affiche le curseur adapté à la direction du redimensionnement.
- Un clic-glisser initie l'opération depuis le bord sélectionné.
- Les zones de coins priment sur les zones de bord ; ne pas recouvrir les commandes de la barre supérieure.
- En mode maximisé, désactiver ces zones ou laisser le gestionnaire de fenêtres gérer le comportement sans conflit.
- Respecter une **taille minimale pratique**, déduite du contenu réellement utilisable ; ne pas imposer une taille minimale inutilement élevée.
- Tester le redimensionnement en continu, avec plusieurs thèmes et langues, sans artefacts ni widgets inaccessibles.

**Solution GTK3/X11 de référence :** `GtkOverlay` contenant huit `GtkEventBox` transparents ; `gtk_window_begin_resize_drag()` avec la valeur `GdkWindowEdge` adaptée. Dans WUP Stream, les bandes de bord mesurent environ **5 px** et les coins **12 × 12 px**. Ces valeurs sont des **points de départ**, non des constantes obligatoires.

```c
gtk_window_begin_resize_drag(GTK_WINDOW(window), edge,
                             event->button,
                             (int)event->x_root,
                             (int)event->y_root,
                             event->time);
```

**Test d'acceptation :** la fenêtre doit pouvoir être déplacée, réduite, maximisée/restaurée et redimensionnée **depuis chaque bord et chaque coin** sans barre GTK native.

---

## 3. Organisation : priorité à une page principale

La page principale est un **tableau de bord** et non une suite d'écrans de configuration à parcourir.

- Afficher autant que raisonnablement possible les réglages courants, informations d'état et commandes principales **sur une seule page**.
- Regrouper les éléments dans des **sections clairement nommées**, de préférence avec une icône cohérente et une bordure discrète.
- Organiser les sections en **paires de colonnes équilibrées** quand la largeur le permet.
- Placer l'action prioritaire dans la barre supérieure ou dans un emplacement immédiatement identifiable.
- Conserver visibles les statuts pertinents sans multiplier les rafraîchissements coûteux.
- Utiliser des libellés courts et des **infobulles explicatives** pour les options techniques.
- **Ne pas interdire les onglets, popups, menus, assistants ni fenêtres secondaires.** Ils restent appropriés pour l'aide détaillée, les opérations rares, les paramètres avancés, les processus multi-étapes et les actions dangereuses.
- Ne pas faire d'une unique page une contrainte absurde : si la densité empêche la compréhension ou l'accès aux commandes, utiliser une section repliable, un panneau secondaire ou un onglet.
- Un menu contextuel, une boîte de dialogue et une fenêtre d'aide doivent hériter du thème et des traductions en cours.

**Architecture indicative :**

```text
Fenêtre sans décorations
┌────────────────────────────────────────────────────────────────┐
│ Icône  Application  État      Thème  Langue  Action    −  □  × │
├────────────────────────────────────────────────────────────────┤
│ [Section principale]        [Section liée]                    │
│ [Section secondaire]        [Section secondaire]              │
│ [Section large : visualiseur, entrées, diagnostics...]        │
└────────────────────────────────────────────────────────────────┘
```

Le contenu peut défiler verticalement si nécessaire ; la **barre supérieure reste visible**. Ne pas utiliser le défilement horizontal pour réparer une mise en page trop large.

---

## 4. Responsive maîtrisé, avec des limites explicites

**Responsive** ne signifie pas seulement « l'utilisateur peut agrandir la fenêtre » : tous les widgets doivent rester utilisables quand la largeur diminue.

### Règles

1. Définir une taille initiale confortable et une **taille minimale testée**. Éviter les dimensions codées en dur pour chaque widget.
2. Disposer les groupes en **deux colonnes** ou plus uniquement si leur largeur réelle le permet ; **rebasculer en une colonne** sous un seuil mesuré.
3. Prévoir plusieurs seuils, plutôt qu'un unique basculement brutal. Par exemple : regrouper les sections, empiler les contrôles puis masquer les éléments purement décoratifs de l'en-tête.
4. Masquer ou raccourcir d'abord les éléments **secondaires**, comme le nom long, les mentions « Thème » et « Langue » ; **ne jamais masquer les boutons de fermeture, de restauration ou l'action critique**.
5. Utiliser `ellipsize` et des infobulles pour les statuts longs ; ne jamais tronquer silencieusement une valeur essentielle.
6. Autoriser le **défilement vertical** si le contenu dépasse la hauteur ; garder une structure stable, sans défilement horizontal.
7. Tenir compte des chaînes traduites qui peuvent être significativement plus longues que l'anglais, du zoom de police et du HiDPI.
8. Ne jamais forcer le panneau à déborder pour conserver artificiellement une disposition en deux colonnes.

### Référence mesurée dans WUP Stream (GTK3)

- Taille initiale : environ **960 × 703**.
- Taille minimale de référence : environ **680 × 530**.
- Sections empilées verticalement autour de **840–850 px** de largeur.
- Certains libellés d'en-tête sont réduits ou masqués autour de **770–830 px**.
- `GtkScrolledWindow` : **politique horizontale `NEVER`**, verticale `AUTOMATIC`.

**Ces valeurs ne sont pas des règles universelles.** Les seuils doivent être adaptés au contenu propre à chaque logiciel et validés avec les sept thèmes et toutes les langues disponibles.

**Exemple GTK3 :**

```c
/* Dans configure-event ; row est un GtkBox */
GtkOrientation mode = width < breakpoint
    ? GTK_ORIENTATION_VERTICAL
    : GTK_ORIENTATION_HORIZONTAL;
if (gtk_orientable_get_orientation(GTK_ORIENTABLE(row)) != mode)
    gtk_orientable_set_orientation(GTK_ORIENTABLE(row), mode);
```

**Attention :** ne pas créer des centaines de widgets pendant chaque `configure-event`. Réorganiser les conteneurs existants et ne modifier l'orientation que si elle change réellement.

---

## 5. Direction artistique et style visuel

Le style WOZT privilégie une interface **dense mais aérée**, lisible et contemporaine :

- **Fonds uniformes ou subtilement différenciés**, panneaux distincts et bordures fines.
- **Coins légèrement arrondis** sur les sections, boutons et champs ; éviter la surenchère d'ombres, de reliefs et d'effets décoratifs.
- **Couleur d'accent cohérente** pour les titres, les éléments actifs, le focus et les actions principales ; une couleur d'alerte distincte pour les erreurs et les actions dangereuses.
- **Contrôles compacts et alignés** sur une grille cohérente, avec marges régulières et espaces fonctionnels.
- **Icônes symboliques GTK** lorsque pertinentes ; prévoir un texte de remplacement. Les icônes propres au produit restent possibles et doivent respecter la palette active.
- **Texte hiérarchisé** : nom de l'application, titres de sections, libellés, états, détails et infobulles.
- Contrastes suffisants dans les thèmes clairs et sombres, et indications d'état qui ne reposent pas sur la couleur seule.
- Survol, clic, sélection, désactivation et focus visuellement distincts.
- Cocher une option ne doit pas rendre son contour ou son texte illisible ; ne pas supprimer les états `:hover`, `:checked`, `:disabled` et `:focus` sans remplacement.
- Appliquer la même palette aux **menus, listes déroulantes, infobulles, popovers, boîtes de dialogue et zones de texte**. Ces surfaces peuvent être rendues hors de la hiérarchie de la fenêtre principale par GTK3.
- Dans une interface de bureau, conserver un rendu sobre et rapide : **éviter les animations et rafraîchissements permanents décoratifs**.

### Densité et lisibilité

WUP Stream utilise historiquement une typographie très compacte, notamment ~11 px dans ses CSS. **Ne pas imposer 11 px comme minimum universel.** Adapter les tailles à l'écran, au facteur de mise à l'échelle et aux besoins d'accessibilité ; ne pas écraser la taille de police choisie par l'utilisateur sans raison. Une interface compacte reste une interface lisible.

### CSS et portée

- Utiliser un `GtkCssProvider` dédié, actualisé en place lors d'un changement de thème.
- Préférer des classes CSS propres à l'application, plutôt que des sélecteurs globaux qui recolorent d'autres fenêtres.
- Valider les couleurs et les propriétés avant de générer la feuille GTK CSS.
- Vérifier explicitement le thème des surfaces GTK hors hiérarchie (`window.popup`, `menu`, `popover`, `tooltip`) et, si nécessaire, leur portée, pour éviter un menu sombre sur fond clair.

---

## 6. Sept thèmes JSON de base (obligatoires)

Le standard comporte **exactement sept thèmes de base**. **Steam Legacy est un thème de base à part entière**, pas un thème facultatif ou expérimental.

| Ordre | ID stable | Nom affiché | Fichier de référence |
|---|---|---|---|
| 1 | `graphite` | Graphite | `examples/themes/01-graphite.json` |
| 2 | `light` | Light | `examples/themes/02-light.json` |
| 3 | `ocean` | Ocean | `examples/themes/03-ocean.json` |
| 4 | `emerald` | Emerald | `examples/themes/04-emerald.json` |
| 5 | `rose` | Rose | `examples/themes/05-rose.json` |
| 6 | `lime` | Apple Lime | `examples/themes/06-lime.json` |
| 7 | `steam-legacy` | Steam Legacy | `examples/themes/07-steam-legacy.json` |

**Les sept fichiers JSON sont les valeurs de référence.** Ne pas inventer de nouvelles palettes sous les mêmes IDs. Les thèmes additionnels sont autorisés, avec de nouveaux IDs.

### Format JSON de référence

Le format repris de WUP Stream possède `id`, `name`, `colors` (palette UI) et `gamepad` (palette d'un visualiseur propre à cette application). Les applications sans GamePad peuvent **ignorer ou adapter** le bloc `gamepad` ; les **onze clés `colors`** représentent le noyau portable du standard.

```json
{
  "id": "ocean",
  "name": "Ocean",
  "colors": {
    "background": "#111923",
    "panel": "#192532",
    "entry": "#0f1b27",
    "border": "#35485b",
    "text": "#dce7f1",
    "muted": "#9fb5c8",
    "accent": "#96cdf9",
    "button": "#253547",
    "hover": "#334c66",
    "active": "#285277",
    "negative": "#ffa052"
  }
}
```

L'exemple ci-dessus illustre le **noyau UI** ; le fichier `examples/themes/03-ocean.json` complet possède aussi le bloc `gamepad`.

### Règles d'implémentation

- Les thèmes sont découverts depuis un répertoire de **fichiers JSON**, plutôt que codés en dur dans un `switch`.
- Le sélecteur de thème fait partie de l'en-tête personnalisé, avec un bouton de rechargement si nécessaire.
- Le changement de thème est **instantané**, sans redémarrage, perte de réglage ou interruption des traitements.
- Sauvegarder l'**ID** (`ocean`), jamais l'index de la liste : l'ajout d'un thème ne doit pas modifier les préférences sauvegardées.
- Un thème externe portant un ID existant peut remplacer la palette fournie, si le mécanisme d'override utilisateur est choisi et documenté.
- Vérifier les IDs (`a-z`, `0-9`, `_`, `-`), l'existence des clés attendues et les couleurs **`#RRGGBB`** ; un fichier malformé doit être signalé et ignoré, sans crash.
- Prévoir un **repli fiable** si le thème sauvegardé a disparu : `ocean` est le repli historique de WUP Stream ; une autre application peut choisir son thème initial mais doit préserver les sept thèmes standards.
- Prévoir le chargement des ressources en mode développement, en installation système et en AppImage ; s'assurer qu'elles sont réellement empaquetées.

---

## 7. Multilingue JSON, rechargement et traduction à chaud

### Format et fonctionnement

- Conserver l'anglais comme **langue source de référence**, avec repli automatique quand une traduction est absente.
- Charger les langues depuis des fichiers UTF-8 contenant `id`, `name` et `translations`.
- `translations` associe une **clé source anglaise stable** à son libellé traduit.
- Sélection de langue directement dans l'en-tête, sans redémarrage du processus.
- Sauvegarder l'**ID** de langue, et non l'index dans la liste.
- Prévoir un bouton **recharger** pour intégrer les corrections des JSON sans relancer l'application.
- Autoriser des traductions utilisateur additionnelles ou des overrides, sans modifier les ressources installées.

```json
{
  "id": "fr",
  "name": "Français",
  "translations": {
    "Start stream": "Démarrer le flux",
    "Stop stream": "Arrêter le flux",
    "Quality": "Qualité",
    "Black screen": "Écran noir"
  }
}
```

### Exemples présents dans ce dépôt

Copier les neuf fichiers de `wup-stream-serveur/data/languages/` vers `examples/languages/` :

- `fr` : français ; `de` : allemand ; `it` : italien ; `es` : espagnol ; `pt` : portugais.
- `ru` : russe ; `ja` : japonais ; `ko` : coréen ; `zh-cn` : chinois simplifié.

**L'anglais est fourni par le code de l'application de référence** : aucun `en.json` n'est requis.

### Règles obligatoires pour la traduction dynamique

1. Traduire **tous les libellés visibles** : boutons, statuts, onglets, titres, sélecteurs, infobulles, menus, listes, aides et dialogues.
2. Conserver séparément la **clé source anglaise d'origine**. Ne jamais retraduire une traduction française comme si elle était le texte anglais original : cela empêche le changement aller-retour entre langues.
3. Mettre à jour les **textes générés à la volée** lors du prochain rafraîchissement normal, ou immédiatement si l'état le nécessite.
4. Traduire les **textes dessinés dans Cairo, Pango, visualiseurs, overlays ou écrans de test**. Une traduction d'arbre GTK ne suffit pas à traduire ces pixels : déclencher explicitement le rendu de l'image ou invalider sa mise en cache.
5. Lorsqu'un libellé dynamique dépend d'un état métier, le traduire à partir d'une clé logique plutôt que conserver une phrase anglaise déjà mise en forme.
6. Pour les popups générés après le changement de langue, traduire à la **création** et mettre à jour ceux qui restent ouverts.
7. Prendre en charge l'UTF-8 et les caractères non latins ; préférer **Pango/PangoCairo** au rendu limité de texte simple pour les contenus graphiques multilingues. Vérifier les polices disponibles.
8. Tester les transitions **FR → JA → DE → EN → FR** pendant l'activité normale et à nouveau après un redémarrage.
9. En cas de clé manquante, afficher l'anglais plutôt qu'un champ vide, `NULL` ou un identifiant technique.
10. Adapter le responsive à la longueur réelle des chaînes traduites.

> À terme, préférer des identifiants de message stables (`settings.quality`) aux phrases anglaises comme clés lors d'une refonte complète ; les JSON fournis restent volontairement compatibles avec la méthode de WUP Stream. Ne pas changer de format implicitement lors d'une migration : prévoir un convertisseur et des tests.

---

## 8. Séparation entre interface et fonctionnement

Le style ne doit jamais fragiliser le programme auquel on l'applique.

- Les événements GTK et le chargement des ressources s'effectuent sur le thread UI, selon les règles GTK.
- Les traitements longs (capture, encodeur, réseau, fichiers volumineux, émulation, audio) restent sur leurs threads dédiés.
- Un changement de thème, de langue ou de taille **n'initie pas de redémarrage du moteur** ni de traitement vidéo inutile.
- Mettre à jour les statuts à une cadence raisonnable, et éviter les tâches coûteuses par frame ou par `configure-event`.
- Conserver les données et préférences entre les versions. Les changements de structure sont accompagnés d'une migration explicite.
- Ne pas casser les raccourcis, l'ordre de tabulation, le focus, les actions accessibles au clavier ni les configurations personnalisées.
- Ne pas supprimer les fonctions avancées pour les faire tenir dans une page : les placer dans une section secondaire ou une boîte de dialogue.
- Ne pas ajouter de nouvelle dépendance système sans justification, documentation et compatibilité avec la distribution visée.
- Prévoir des ressources embarquées pour les distributions portables (par exemple AppImage), avec droits de lecture et d'exécution appropriés.

---

## 9. Build et distribution AppImage (obligatoire sur Linux)

L'**AppImage x86_64 fait partie du standard WOZT GTK** pour les applications de bureau distribuées sous Linux. Une application portée sur une autre architecture ou un autre système peut ajouter son format natif, mais l'AppImage est l'artefact de livraison de référence sur Linux x86_64. **La seule compilation locale ne constitue pas une livraison.**

### Résultat attendu

- Fournir dans **chaque dépôt d'application** un script versionné `tools/build-appimage.sh` (ou un chemin clairement documenté), exécutable depuis la racine du projet.
- Construire d'abord l'application à partir des sources ; ne pas empaqueter silencieusement un vieux binaire.
- Créer un `AppDir` standard et une AppImage nommée de manière explicite, par exemple `dist/<app>-x86_64.AppImage`.
- Inclure le binaire, un fichier `.desktop` valide, une **icône d'application** PNG/SVG convenable, les dépendances non présentes sur le système cible et **toutes les ressources utilisées à l'exécution** : les sept thèmes JSON, les langues JSON, images/icônes, CSS, polices embarquées si nécessaires, fichiers de configuration *non personnels* et autres données requises.
- La sélection de thème/langue et leur rechargement doivent fonctionner dans l'AppImage aussi bien qu'en développement. Ne **jamais** dépendre d'un chemin absolu tel que `/home/<utilisateur>/dev/...` pour les ressources à l'exécution.
- Éviter tout accès en écriture à l'intérieur de l'AppImage. Placer préférences et personnalisations dans les répertoires XDG de l'utilisateur (`~/.config/<app>/`, `~/.local/share/<app>/`).
- Conserver les permissions des **exécutables à 0755** et des **ressources lisibles à 0644** (répertoires traversables). Les SVG et JSON doivent être lisibles par tout utilisateur, pas seulement par la personne qui a construit l'AppImage.
- Ne pas exiger `sudo` pour construire ou démarrer l'AppImage. Les privilèges nécessaires à une fonction métier doivent être gérés séparément et explicitement.
- Ne pas déclarer le packaging réussi sur le seul code de retour zéro : vérifier les avertissements de `linuxdeploy` / `appimagetool`, l'existence du fichier, son lancement et le chargement réel des ressources.

### Chaîne de packaging de référence : Meson/Ninja + linuxdeploy

Le projet `wupstreamd` emploie `meson compile -C build`, un script `tools/build-appimage.sh` et `linuxdeploy`. Ce n'est **pas** une dépendance imposée aux applications utilisant CMake/Make : adapter la ligne de compilation au projet, en conservant le **même contrat de résultat**.

Le dépôt du standard fournit une **copie d'étude** du script actuel dans `examples/appimage/build-appimage-wupstreamd.reference.sh` : son contenu est spécifique à WUP Stream (noms, chemins, bibliothèques). Les agents doivent s'en inspirer pour créer un script propre au projet cible, **pas le copier tel quel et prétendre qu'il fonctionne partout**.

Étapes attendues dans le script de chaque application :

1. Vérifier l'architecture ciblée et les outils nécessaires (`linuxdeploy`, compilateur, système de build, éventuellement `appimagetool`). Offrir `LINUXDEPLOY=/chemin/vers/linuxdeploy-x86_64.AppImage` pour une installation locale sans modification du `PATH`.
2. Compiler (et de préférence lancer les tests) avec le système de build du projet ; arrêter immédiatement le script en cas d'échec (`set -euo pipefail`).
3. Préparer `dist/<app>.AppDir/usr/bin`, `usr/share/applications`, `usr/share/icons` et `usr/share/<app>/` ; copier la version **à jour** du programme et des ressources JSON. Si le projet utilise GTK/Pango/Cairo, vérifier leurs dépendances et données utiles au chargement de polices/icônes.
4. **Normaliser les permissions avant le packaging**, particulièrement après `cp -a` qui conserve parfois des fichiers source en `0600` :

   ```bash
   # Après la copie du binaire et des ressources dans l'AppDir :
   chmod -R a+rX "$APPDIR"
   # Les exécutables installés par install -m755 restent exécutables.
   ```

5. Construire l'AppImage avec `linuxdeploy` (et ses plugins adaptés s'ils sont réellement requis), sans oublier les ressources et les métadonnées `.desktop` / icône.
6. Confirmer que l'AppImage existe, qu'elle est exécutable, démarre réellement, trouve ses fichiers embarqués et n'affiche pas d'avertissements de permissions. Documenter le chemin final pour le contributeur et l'utilisateur.

**Exemple de commande pour le projet de référence (à ne pas appliquer aveuglément à un autre logiciel) :**

```bash
cd ~/dev/wup-stream-serveur
LINUXDEPLOY="$PWD/tools/linuxdeploy-x86_64.AppImage" \
  bash tools/build-appimage.sh
./dist/wupstreamd-x86_64.AppImage
```

### Tests AppImage de livraison

- [ ] Le script compile bien la **version courante** des sources, et échoue si la compilation échoue.
- [ ] Le fichier final est présent, nommé correctement et exécutable sans `sudo`.
- [ ] **Sept thèmes** et **toutes les langues fournies** sont visibles dans l'AppImage, y compris leurs changements **à chaud**.
- [ ] Les popups, icônes et visualiseurs se chargent, sans dépendre de la copie locale du dépôt.
- [ ] Les icônes SVG, JSON et autres ressources n'ont pas de droits `0600`; pas d'alertes de permission `appimagetool`.
- [ ] Test après déplacement de l'AppImage hors de `~/dev/`, et si possible sous un autre utilisateur et une seconde distribution Linux.
- [ ] Vérifier la compatibilité **glibc** : construire sur un environnement suffisamment ancien pour la distribution minimale annoncée, ou documenter explicitement la version minimale prise en charge. AppImage **n'embarque pas automatiquement toute la compatibilité système**.
- [ ] Tester le démarrage lorsque FUSE n'est pas disponible, par exemple via une extraction / un mode d'exécution adapté selon la version de l'AppImage.
- [ ] Préserver la configuration personnelle entre lancements et mises à jour, sans écrire dans l'image montée en lecture seule.

Une application déclarée « conforme au WOZT GTK Standard » mais livrée sous Linux **sans AppImage ni script de construction reproductible** reste **non conforme** au volet distribution, sauf exemption de plateforme documentée.

---

## 10. Plan de migration d'une application existante (pour LLM)

Pour une application existante, suivre cet ordre **sans réécrire le projet de zéro** :

1. **Auditer** les fichiers UI, thèmes, ressources, langues, paramètres persistants, callbacks, outils de construction et contraintes de distribution. Faire l'inventaire des comportements qu'il ne faut pas perdre.
2. **Établir le périmètre** de l'interface à modifier ; isoler les changements des fonctions métier et des boucles de traitement.
3. **Construire la fenêtre sans décoration native** et une barre de titre personnalisée complète : icône, déplacement, boutons de fenêtre et comportement de fermeture adapté.
4. **Rendre le redimensionnement opérationnel** sur les huit bords/coins et choisir une taille minimale validée.
5. **Réorganiser la page principale** en sections compactes et lisibles, puis ajouter les basculements responsive et le défilement vertical.
6. **Introduire les sept thèmes JSON de base** sans perdre les couleurs spécifiques au produit ; brancher le sélecteur, la sauvegarde et le rechargement à chaud.
7. **Introduire les traductions JSON** : contenu statique, états dynamiques, menus, popups et surfaces dessinées.
8. **Tester les transitions à chaud** et la conservation de l'état : redimensionnement pendant l'activité, changement de langue et thème, fermeture/restauration, reconnexion, reprise après redémarrage.
9. **Construire et tester l'AppImage** selon la section 9 (ou consigner une exemption de plateforme justifiée) : script reproductible, ressources JSON/icônes, droits et compatibilité système.
10. **Comparer avant/après** : ne livrer que si les fonctions historiques marchent encore et si l'ergonomie n'a pas régressé.

### Interdictions pour les agents de code

- **Ne pas** remplacer des fonctions réelles par des contrôles décoratifs qui ne font rien.
- **Ne pas** masquer des fonctions existantes sans accès alternatif.
- **Ne pas** introduire de `min-width`/`width` fixes généralisés empêchant le responsive.
- **Ne pas** désactiver le redimensionnement au prétexte que la barre native a disparu.
- **Ne pas** coder les thèmes ou les textes traduits uniquement dans les fichiers source si les JSON sont requis.
- **Ne pas** appliquer un thème uniquement au contenu de la fenêtre principale en oubliant les popups.
- **Ne pas** présumer qu'un changement de langue affecte automatiquement un visualiseur dessiné.
- **Ne pas** modifier les modules moteur, streaming, émulation ou réseaux pour une refonte seulement visuelle, sauf nécessité démontrée.
- **Ne pas** annoncer « terminé » sans compilation et vérification des scénarios critiques.

---

## 11. Checklist de conformité / critères d'acceptation

### Fenêtre

- [ ] Barre de titre GTK standard retirée ; icône et contrôles intégrés en haut de la fenêtre.
- [ ] Déplacement par glisser-déposer sur une zone dédiée ; double-clic maximise/restaure.
- [ ] Réduire, maximiser/restaurer et fermer fonctionnent et sont accessibles.
- [ ] Redimensionnement fonctionnel sur les **quatre bords et quatre coins** ; curseurs adaptés.
- [ ] Le comportement du bouton de fermeture est explicite et cohérent avec l'application.

### Mise en page

- [ ] Fonctions courantes accessibles depuis la page principale, avec sections lisibles.
- [ ] Les onglets/popups nécessaires restent disponibles.
- [ ] Pas de défilement horizontal ; défilement vertical seulement lorsque nécessaire.
- [ ] Aucun contrôle important ne disparaît quand la fenêtre devient étroite.
- [ ] Le contenu s'adapte aux tailles minimum, intermédiaire et maximale, y compris avec de longs textes traduits.

### Apparence

- [ ] Les **sept** thèmes sont présents : Graphite, Light, Ocean, Emerald, Rose, Apple Lime, Steam Legacy.
- [ ] Les sept JSON sont valides, chaque palette possède un ID stable.
- [ ] Changement et rechargement de thème à chaud, sans perdre l'état métier.
- [ ] Styles cohérents sur boutons, champs, sections, menus, popups, tooltips et dialogues.
- [ ] États hover, focus, active, checked et disabled visibles et contrastés.

### Traductions

- [ ] Langues chargées depuis JSON UTF-8, avec repli en anglais.
- [ ] Changement et rechargement de langue à chaud, sans redémarrage.
- [ ] Textes statiques, dynamiques, infobulles, dialogues et visualiseurs traduits.
- [ ] Les bascules de langue répétées ne dégradent pas les textes ou l'état des widgets.
- [ ] Les ressources linguistiques sont fournies dans les distributions installables.

### AppImage Linux

- [ ] Script `tools/build-appimage.sh` présent, reproductible et maintenu.
- [ ] AppImage x86_64 construite depuis la version courante et publiée comme artefact de livraison.
- [ ] Sept thèmes, traductions et ressources réellement présents dans le paquet.
- [ ] Permissions de l'AppDir valides, aucun avertissement bloquant ou de fichiers illisibles.
- [ ] Test de démarrage après déplacement du fichier, sans répertoire de développement et sur une distribution prise en charge.

### Non-régression

- [ ] Compilation sans erreurs ; avertissements introduits traités.
- [ ] Tests UI et scénarios métier historiques réussis.
- [ ] Pas de nouvelles boucles bloquantes ni de ralentissement notable du moteur.
- [ ] La configuration et les préférences persistent correctement.
- [ ] Distribution vérifiée sur les environnements annoncés, avec ressources lisibles.

---

## 12. Structure recommandée de ce dépôt

```text
wozt-gtk-standard/
├── standard.md                  # Ce document : spécification et règles pour agents/LLM
└── examples/
    ├── themes/
    │   ├── 01-graphite.json
    │   ├── 02-light.json
    │   ├── 03-ocean.json
    │   ├── 04-emerald.json
    │   ├── 05-rose.json
    │   ├── 06-lime.json
    │   └── 07-steam-legacy.json
    ├── languages/
        ├── 01-fr.json
        ├── 02-de.json
        ├── 03-it.json
        ├── 04-es.json
        ├── 05-pt.json
        ├── 06-ru.json
        ├── 07-ja.json
        ├── 08-ko.json
        └── 09-zh-cn.json
    └── appimage/
        └── build-appimage-wupstreamd.reference.sh # Exemple concret, à adapter
```

Les JSON et le script de packaging de référence sont copiés **tels quels depuis le dépôt WUP Stream**. Une application peut employer d'autres clés et sections propres à son domaine, mais doit conserver l'esprit des palettes et de l'internationalisation. Les exemples servent de base, pas de contrainte d'import direct.

### Sources de référence (lecture seule)

- [`wozt/wup-stream-server-linux/src/ui.c`](https://github.com/wozt/wup-stream-server-linux/blob/main/src/ui.c) : fenêtre custom, zones de resize, responsive, style CSS, thèmes, langue.
- [`wozt/wup-stream-server-linux/src/i18n.c`](https://github.com/wozt/wup-stream-server-linux/blob/main/src/i18n.c) : chargement JSON, traductions dynamiques, repli anglais.
- [`wozt/wup-stream-server-linux/src/settings.c`](https://github.com/wozt/wup-stream-server-linux/blob/main/src/settings.c) : persistance des IDs de thèmes/langues.
- [`wozt/wup-stream-server-linux/data/themes`](https://github.com/wozt/wup-stream-server-linux/tree/main/data/themes) : sept thèmes de base.
- [`wozt/wup-stream-server-linux/data/languages`](https://github.com/wozt/wup-stream-server-linux/tree/main/data/languages) : exemples multilingues.
- [`wozt/wup-stream-server-linux/tools/build-appimage.sh`](https://github.com/wozt/wup-stream-server-linux/blob/main/tools/build-appimage.sh) : script de packaging de référence, à adapter et à sécuriser par `chmod -R a+rX` pour les ressources.

---

## 13. Instruction courte à fournir à un agent/LLM

> **Mission :** adapter l'interface de cette application au **WOZT GTK Standard** (`standard.md`) et aux JSON de `examples/themes/` et `examples/languages/`. Audite d'abord l'application existante. Préserve intégralement les fonctions, les paramètres et les performances. Mets en place une barre supérieure sans décoration GTK native avec icône, contrôles de fenêtre et déplacement ; rends le redimensionnement opérationnel sur huit zones ; crée une page principale compacte avec responsive contrôlé, sans défilement horizontal ; intègre les sept thèmes JSON de base, dont Steam Legacy, avec changement à chaud ; ajoute des langues JSON et leur changement dynamique, notamment pour les popups et les surfaces dessinées. Conserve les onglets ou popups réellement utiles. Construis une AppImage x86_64 reproductible avec ressources embarquées et permissions vérifiées ; teste toutes les interactions, les changements à chaud et la distribution. Ne marque le travail comme terminé qu'après les vérifications de la section 11.

**Fin du standard v1.0.**
