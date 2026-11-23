# WOZT GTK Standard

> **Version:** 1.0 — November 23, 2026  
> **Purpose:** A reusable visual, usability, and technical standard for WOZT desktop interfaces.  
> **Original reference:** [`wozt/wup-stream-server-linux`](https://github.com/wozt/wup-stream-server-linux), especially `src/ui.c`, `src/i18n.c`, `src/settings.c`, `data/themes/`, and `data/languages/`.  
> **Status:** A specification for creating and redesigning interfaces; this repository is **not**, by itself, a ready-to-use GTK library.

## 0. General principle

Create interfaces that are **consistent, compact, modern, readable, customizable, translatable, and genuinely resizable**, without relying on a standard GTK title bar. Prefer a **single dashboard** that makes important features visible at a glance.

**Do not copy WUP Stream's code mechanically.** Adopt its principles, then adapt labels, features, panels, layout breakpoints, and technical decisions to the target application. Good interaction design and existing behavior take priority over pixel-perfect similarity.

### Priority of requirements

1. **Preserve existing functionality:** processing, threads, performance, settings, controls, shortcuts, saved data, and accessibility.
2. **A fully usable custom window:** move, minimize, maximize/restore, close, and resize from all four edges and four corners.
3. **A compact, responsive main page:** immediate access to important settings and indicators; no horizontal scrolling.
4. **Seven default themes defined in JSON**, switchable at runtime.
5. **JSON-based internationalization**, switchable at runtime and complete across menus, popups, and drawn content.
6. **A consistent visual style**, clear information hierarchy, visible states, and real testing.
7. **Linux x86_64 AppImage distribution:** reproducible build, bundled assets, correct permissions, and verification on other systems.

---

## 1. Window without a native GTK title bar

### Required behavior

- Disable native window decorations and **integrate a custom top bar directly into the application's content**.
- Depending on the application, that bar contains the **app icon**, **app name**, **essential status**, **primary action**, **theme selector**, **language selector**, and **window controls**.
- Present the usual **minimize**, **maximize/restore**, and **close** actions, using symbolic icons with a text fallback if those icons are unavailable.
- The non-interactive region of the top bar must support **click-and-drag window movement**. **Double-clicking** the drag area maximizes or restores the window.
- Menus, buttons, and selectors must retain their own click handling; interacting with them must not accidentally drag the window.
- The close button follows the application's actual semantics: exit the application, or **hide to the system tray** only when that behavior is available, documented, and communicated to the user.
- Preserve the app icon in window metadata, the taskbar, and `.desktop` files even when the native title bar is hidden.
- Preserve keyboard navigation, focus states, tooltips, and accessible descriptions for icon-only buttons.

**GTK3 example:**

```c
gtk_window_set_decorated(GTK_WINDOW(window), FALSE);
gtk_window_set_resizable(GTK_WINDOW(window), TRUE);

/* In the dedicated title drag area, on a primary-button press: */
gtk_window_begin_move_drag(GTK_WINDOW(window),
                           event->button,
                           (int)event->x_root,
                           (int)event->y_root,
                           event->time);
```

For the top-bar controls, use `gtk_window_iconify()`, `gtk_window_maximize()`, `gtk_window_unmaximize()`, and—depending on the application's contract—`gtk_window_close()` or `gtk_widget_hide()`.

> **Portability:** The original WUP Stream implementation uses **GTK3/X11**. For GTK4, Wayland, or other window managers, adapt input handling and verify the interactions on the target system. Do not assume X11 overlay and hit-testing techniques behave identically everywhere.

---

## 2. Real resizing without native decorations

**A borderless window must remain as resizable as a normal window.** `gtk_window_set_resizable(TRUE)` alone does not always provide interactive resize areas once the window manager's decorations have been removed.

### Requirements

- Provide **eight resize zones**: top, bottom, left, right, and all four corners.
- These zones must be **transparent**, overlaid without disrupting the layout.
- The pointer must use the appropriate resize cursor for each direction.
- Clicking and dragging a zone must initiate resizing from that edge or corner.
- Corner zones take precedence over edge zones; do not cover the top-bar controls.
- When maximized, disable these zones or defer to the window manager without conflicts.
- Honor a **practical minimum window size** based on the usable content; avoid unnecessarily large minimum dimensions.
- Test continuous resizing with multiple themes and languages, without visual artifacts or inaccessible widgets.

**Reference GTK3/X11 approach:** a `GtkOverlay` with eight transparent `GtkEventBox` elements and `gtk_window_begin_resize_drag()` using the corresponding `GdkWindowEdge`. WUP Stream uses approximately **5 px** edge strips and **12 × 12 px** corners. These are **starting points**, not mandatory constants.

```c
gtk_window_begin_resize_drag(GTK_WINDOW(window), edge,
                             event->button,
                             (int)event->x_root,
                             (int)event->y_root,
                             event->time);
```

**Acceptance test:** Without a native GTK title bar, the window can be moved, minimized, maximized/restored, and resized **from every edge and every corner**.

---

## 3. Layout: prioritize one main page

The main page should work as a **dashboard**, not a sequence of settings screens that users must navigate.

- Display common settings, status information, and primary controls **on one page** wherever practical.
- Group content into **clearly named sections**, preferably with consistent icons and subtle borders.
- Arrange sections in **balanced column pairs** when the available width allows.
- Put the primary action in the top bar or another immediately recognizable location.
- Keep relevant statuses visible without introducing frequent expensive refreshes.
- Prefer short labels with **explanatory tooltips** for technical options.
- **Do not forbid tabs, popups, menus, wizards, or secondary windows.** They remain appropriate for detailed help, infrequent tasks, advanced settings, multi-step workflows, and dangerous actions.
- Do not make the single-page preference an unreasonable constraint: if density makes the interface difficult to understand or use, add a collapsible section, secondary panel, or tab.
- Context menus, dialogs, and help windows must inherit the current theme and language.

**Illustrative structure:**

```text
Borderless window
┌────────────────────────────────────────────────────────────────┐
│ Icon  Application  Status    Theme  Language  Action   −  □  × │
├────────────────────────────────────────────────────────────────┤
│ [Primary section]           [Related section]                 │
│ [Secondary section]         [Secondary section]               │
│ [Wide section: visualizer, inputs, diagnostics, etc.]          │
└────────────────────────────────────────────────────────────────┘
```

The content may scroll vertically when necessary, but the **top bar remains visible**. Do not use horizontal scrolling as a workaround for an overly wide layout.

---

## 4. Controlled responsive behavior, with explicit limits

**Responsive** means more than letting the user enlarge the window: every widget must remain usable as the width decreases.

### Rules

1. Define a comfortable default size and a **tested minimum size**. Avoid hard-coded dimensions for every individual widget.
2. Use **two or more columns** only when there is sufficient real space; **switch back to one column** below a measured breakpoint.
3. Support several breakpoints rather than one abrupt switch. For example, rearrange sections first, stack controls next, and then hide purely decorative elements in the header.
4. Hide or shorten **secondary elements** first, such as a long product name or the words “Theme” and “Language”; **never hide the close, maximize/restore, or critical action buttons**.
5. Use text ellipsization and tooltips for lengthy statuses; never silently truncate an essential value.
6. Allow **vertical scrolling** when the content exceeds the available height; preserve a stable layout without horizontal scrolling.
7. Account for localized strings that can be substantially longer than English, larger user fonts, and HiDPI scaling.
8. Never allow a panel to overflow just to preserve an artificial two-column layout.

### Measurements from WUP Stream (GTK3)

- Default window size: approximately **960 × 703**.
- Reference minimum size: approximately **680 × 530**.
- Major sections stack vertically at around **840–850 px** wide.
- Some header labels are shortened or hidden at around **770–830 px**.
- `GtkScrolledWindow`: horizontal policy **`NEVER`**, vertical policy **`AUTOMATIC`**.

**These measurements are not universal rules.** Breakpoints must be adapted to each application's content and validated with all seven themes and every supported language.

**GTK3 example:**

```c
/* Inside configure-event; row is a GtkBox. */
GtkOrientation mode = width < breakpoint
    ? GTK_ORIENTATION_VERTICAL
    : GTK_ORIENTATION_HORIZONTAL;
if (gtk_orientable_get_orientation(GTK_ORIENTABLE(row)) != mode)
    gtk_orientable_set_orientation(GTK_ORIENTABLE(row), mode);
```

**Important:** Do not create hundreds of widgets on every `configure-event`. Rearrange existing containers, and update their orientation only when it actually changes.

---

## 5. Art direction and visual style

The WOZT style favors an interface that is **information-dense without feeling cramped**, readable, and contemporary:

- **Solid or subtly differentiated backgrounds**, clearly separated panels, and thin borders.
- **Slightly rounded corners** on sections, buttons, and fields; avoid excessive shadows, beveling, and purely decorative effects.
- **A consistent accent color** for headings, selected elements, focus, and primary actions; a separate warning/error color for failures and dangerous actions.
- **Compact, aligned controls** on a coherent grid, with regular margins and functional spacing.
- **GTK symbolic icons** where appropriate, with text fallbacks. Product-specific icons are welcome but must work with the active palette.
- **Clear typographic hierarchy:** app name, section headings, labels, statuses, secondary information, and tooltips.
- Sufficient contrast in both light and dark themes; state indications must not rely on color alone.
- Hover, press, selection, disabled, and focus states must be visually distinguishable.
- Checking a control must not make its outline or text unreadable; do not remove `:hover`, `:checked`, `:disabled`, or `:focus` states without providing replacements.
- Apply the same palette to **menus, drop-downs, tooltips, popovers, dialogs, and text areas**. GTK3 may render these surfaces outside the main window's widget hierarchy.
- Keep desktop interfaces restrained and fast: **avoid decorative animations and continuous unnecessary repaints**.

### Density and readability

WUP Stream historically uses very compact typography, including around 11 px in its CSS. **Do not mandate 11 px as a universal font size.** Adapt sizes to the display, scaling factor, and accessibility requirements; do not override the user's chosen font size without good reason. Compact must still mean readable.

### CSS and scope

- Use a dedicated `GtkCssProvider` and update it in place when the theme changes.
- Prefer CSS classes scoped to the application over global selectors that might recolor unrelated windows.
- Validate color values and properties before generating GTK CSS.
- Explicitly verify the styling and selector scope of GTK surfaces outside the main widget tree (`window.popup`, `menu`, `popover`, `tooltip`), avoiding dark menus over a light-themed application or vice versa.

---

## 6. Seven default JSON themes (mandatory)

The standard defines **exactly seven default themes**. **Steam Legacy is a full-fledged default theme**, not an optional or experimental theme.

| Order | Stable ID | Display name | Reference file |
|---|---|---|---|
| 1 | `graphite` | Graphite | `examples/themes/01-graphite.json` |
| 2 | `light` | Light | `examples/themes/02-light.json` |
| 3 | `ocean` | Ocean | `examples/themes/03-ocean.json` |
| 4 | `emerald` | Emerald | `examples/themes/04-emerald.json` |
| 5 | `rose` | Rose | `examples/themes/05-rose.json` |
| 6 | `lime` | Apple Lime | `examples/themes/06-lime.json` |
| 7 | `steam-legacy` | Steam Legacy | `examples/themes/07-steam-legacy.json` |

**The seven JSON files define the reference values.** Do not invent new palettes under those same IDs. Additional themes are allowed, using new IDs.

### Reference JSON format

The format inherited from WUP Stream contains `id`, `name`, `colors` (UI palette), and `gamepad` (palette for an application-specific visualizer). Apps without a GamePad may **ignore or adapt** the `gamepad` block; the **eleven `colors` keys** form the portable core of this standard.

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

The example above shows the **UI core**; the complete `examples/themes/03-ocean.json` file also contains the `gamepad` block.

### Implementation requirements

- Discover themes from a directory of **JSON files** instead of hard-coding them in a `switch` statement.
- Put the theme selector in the custom top bar, optionally with a reload button.
- Apply theme changes **immediately**, without restarting, losing settings, or interrupting ongoing work.
- Persist the **theme ID** (`ocean`), never its list index: adding a theme must not change saved preferences.
- A user theme with the same ID may override a bundled palette if that override behavior is intentionally supported and documented.
- Validate IDs (`a-z`, `0-9`, `_`, `-`), required keys, and **`#RRGGBB`** colors; report and ignore malformed JSON without crashing.
- Provide a **reliable fallback** when a saved theme is no longer available: `ocean` is WUP Stream's historical fallback; another app may choose its own initial theme but must preserve all seven standard themes.
- Support loading assets in development, system-installed, and AppImage builds; verify they are actually packaged.

---

## 7. JSON localization, reloading, and live language switching

### Format and behavior

- Use English as the **canonical source language**, with automatic fallback when a translation is missing.
- Load languages from UTF-8 JSON files containing `id`, `name`, and `translations`.
- `translations` maps a **stable English source key** to its localized text.
- Place language selection in the custom header, without requiring a restart.
- Persist the **language ID**, not its position in the list.
- Provide a **reload** button to load revised JSON files without restarting the app.
- Allow additional user translations and overrides without modifying installed assets.

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

### Examples included in this repository

Copy the nine files from `wup-stream-serveur/data/languages/` into `examples/languages/`:

- `fr`: French; `de`: German; `it`: Italian; `es`: Spanish; `pt`: Portuguese.
- `ru`: Russian; `ja`: Japanese; `ko`: Korean; `zh-cn`: Simplified Chinese.

**English strings are built into the reference application's code:** a separate `en.json` file is not required.

### Mandatory rules for dynamic translation

1. Translate **all visible text**: buttons, statuses, tabs, titles, selectors, tooltips, menus, lists, help, and dialogs.
2. Store the **original English source key** separately. Never attempt to retranslate an existing French translation as if it were the English original: doing so breaks switching back and forth between languages.
3. Update **dynamically generated labels** during their next normal refresh, or immediately when application state requires it.
4. Translate **text drawn with Cairo, Pango, visualizers, overlays, and test screens**. Translating the GTK widget tree does not update those pixels; explicitly redraw the image or invalidate its cache.
5. When a dynamic label depends on application state, translate it from a logical key instead of retaining an already formatted English sentence.
6. Translate popups created after a language change **at creation time**, and update any that remain open.
7. Support UTF-8 and non-Latin scripts; prefer **Pango/PangoCairo** over basic text-drawing primitives for multilingual graphical text. Verify that suitable fonts are available.
8. Test **FR → JA → DE → EN → FR** transitions while the application is running and again after a restart.
9. For missing keys, display English—not an empty field, `NULL`, or an internal identifier.
10. Adapt the responsive layout to the actual length of translated strings.

> In a comprehensive redesign, consider stable message identifiers (`settings.quality`) rather than English sentences as translation keys. The supplied JSON files intentionally remain compatible with WUP Stream's existing approach. Do not silently change formats during migration: provide a converter and tests.

---

## 8. Keep the UI separate from application behavior

Visual design must never make the underlying application less reliable.

- Run GTK events and resource loading on the UI thread, following GTK's threading rules.
- Keep long-running tasks (capture, encoding, networking, large-file operations, emulation, audio) on their dedicated threads.
- Changing the theme, language, or window size must **not restart the engine** or trigger unnecessary video processing.
- Update status indicators at a reasonable frequency, and avoid expensive work per frame or during each `configure-event`.
- Preserve user data and preferences across releases. Any structural changes require an explicit migration strategy.
- Preserve keyboard shortcuts, tab order, focus, keyboard-accessible actions, and custom configurations.
- Do not remove advanced features to squeeze everything onto one page; move them to a secondary section or dialog instead.
- Do not add system dependencies without justification, documentation, and compatibility with the target distribution.
- Include runtime assets in portable distributions such as AppImage, with appropriate read and execute permissions.

---

## 9. AppImage build and distribution (mandatory on Linux)

A **Linux x86_64 AppImage is part of the WOZT GTK Standard** for Linux desktop applications. Applications targeting additional architectures or operating systems may also ship native packages, but AppImage is the reference deliverable on Linux x86_64. **A successful local build alone is not a complete release.**

### Expected deliverables

- Include a version-controlled `tools/build-appimage.sh` script in **every application repository** (or document another unambiguous script path), runnable from the project root.
- Build the application from source before packaging; never silently bundle a stale executable.
- Produce a conventional `AppDir` and an explicitly named AppImage, for example `dist/<app>-x86_64.AppImage`.
- Bundle the binary, a valid `.desktop` file, a suitable PNG/SVG **application icon**, non-system runtime dependencies required by the target environment, and **all runtime assets**: the seven JSON themes, language JSON files, images/icons, CSS, bundled fonts if needed, *nonpersonal* configuration templates, and other required data.
- Theme/language selection and live reloading must work inside the AppImage just as they do in development. **Never** depend on absolute paths such as `/home/<username>/dev/...` to access runtime assets.
- Do not write inside the AppImage. Store preferences and customizations in the user's XDG locations (`~/.config/<app>/`, `~/.local/share/<app>/`).
- Ensure **executables have mode 0755** and **readable assets have mode 0644**, with traversable directories. SVG and JSON resources must be readable by all users, not just the person who created the AppImage.
- Do not require `sudo` to build or run the AppImage. Privileges required by application-specific features must be managed separately and explicitly.
- Do not consider packaging successful based solely on a zero exit code: inspect `linuxdeploy` / `appimagetool` warnings, verify the output file, launch it, and confirm that its runtime assets load correctly.

### Reference packaging pipeline: Meson/Ninja + linuxdeploy

`wupstreamd` uses `meson compile -C build`, a `tools/build-appimage.sh` script, and `linuxdeploy`. This does **not** mean that applications built with CMake or Make must adopt Meson; adapt the compilation commands to each project while honoring the **same delivery requirements**.

This standard repository includes a **reference copy** of the existing script at `examples/appimage/build-appimage-wupstreamd.reference.sh`. Its contents are specific to WUP Stream (names, paths, libraries). Coding agents must use it as inspiration to write an app-specific script, **not copy it unchanged and claim it works everywhere**.

Expected steps in each application's build script:

1. Verify the target architecture and required tools (`linuxdeploy`, compiler, build system, and possibly `appimagetool`). Support `LINUXDEPLOY=/path/to/linuxdeploy-x86_64.AppImage` for local tool installations without changing `PATH`.
2. Compile (and preferably run tests) with the project's build system; stop immediately if a step fails (`set -euo pipefail`).
3. Prepare `dist/<app>.AppDir/usr/bin`, `usr/share/applications`, `usr/share/icons`, and `usr/share/<app>/`; install the **latest** binary and JSON resources. If the app uses GTK/Pango/Cairo, verify relevant runtime dependencies, fonts, and icon-loading assets.
4. **Normalize permissions before packaging**, especially after `cp -a`, which may preserve files originally set to `0600`:

   ```bash
   # After copying the executable and all assets into the AppDir:
   chmod -R a+rX "$APPDIR"
   # Executables installed with install -m755 remain executable.
   ```

5. Build the AppImage using `linuxdeploy` and any plugins actually required by the app, including its assets, `.desktop` metadata, and icon.
6. Confirm that the output file exists, is executable, launches, can find its bundled resources, and produces no permission warnings. Document the final path for contributors and users.

**Reference application build command (do not apply blindly to a different project):**

```bash
cd ~/dev/wup-stream-serveur
LINUXDEPLOY="$PWD/tools/linuxdeploy-x86_64.AppImage" \
  bash tools/build-appimage.sh
./dist/wupstreamd-x86_64.AppImage
```

### AppImage release tests

- [ ] The script builds the **current source version** and fails when compilation fails.
- [ ] The final file exists, is named correctly, and runs without `sudo`.
- [ ] **All seven themes** and **all bundled languages** are available inside the AppImage, including **live switching**.
- [ ] Popups, icons, and visualizers load without depending on a local development checkout.
- [ ] SVG icons, JSON files, and other assets are not restricted to mode `0600`; `appimagetool` reports no asset permission warnings.
- [ ] The AppImage works after being moved outside `~/dev/`, and ideally when run by another user and on a second Linux distribution.
- [ ] Verify **glibc compatibility**: build on an environment old enough for the oldest supported distribution, or explicitly document the minimum supported version. AppImage **does not automatically bundle complete system compatibility**.
- [ ] Test startup where FUSE is unavailable, for example using extraction or another supported execution mode for that AppImage format.
- [ ] Preserve personal settings between launches and updates, without writing to the read-only mounted image.

An application claiming WOZT GTK Standard compliance while shipping on Linux **without an AppImage or reproducible AppImage build script** is **not compliant** with the distribution requirement unless a documented platform exemption applies.

---

## 10. Migration plan for existing applications (for coding agents/LLMs)

When redesigning an existing application, follow this sequence **without rewriting the project from scratch**:

1. **Audit** UI files, themes, resources, languages, persistent settings, callbacks, build tools, and distribution constraints. List the behaviors that must not be lost.
2. **Define the scope** of the UI changes; isolate them from business logic and processing loops.
3. **Build a fully functional borderless window** with an integrated top bar: icon, movement, window controls, and the correct close behavior.
4. **Make resizing work** from all eight edges/corners and choose a validated minimum size.
5. **Reorganize the main page** into compact, readable sections; add responsive transitions and vertical scrolling.
6. **Integrate all seven default JSON themes** without losing product-specific colors; implement selection, persistence, and live reloading.
7. **Integrate JSON translations** for static text, dynamic states, menus, popups, and drawn surfaces.
8. **Test live transitions** and state preservation: resize while active, change language and theme, close/restore, reconnect, and resume after restart.
9. **Build and test an AppImage** according to Section 9 (or record a justified platform exemption): reproducible script, bundled JSON/icons, permissions, and system compatibility.
10. **Compare before and after:** deliver only when existing features still work and usability has not regressed.

### Prohibited behaviors for coding agents

- **Do not** replace real features with decorative controls that do nothing.
- **Do not** hide existing features without an alternative way to access them.
- **Do not** introduce blanket fixed `min-width`/`width` constraints that break responsiveness.
- **Do not** disable resizing just because the native title bar has been removed.
- **Do not** hard-code themes or translated text exclusively in source files when JSON is required.
- **Do not** style only the main window while ignoring popups.
- **Do not** assume changing languages automatically updates a custom-drawn visualizer.
- **Do not** change engine, streaming, emulation, or networking code during a visual-only redesign unless there is a demonstrated technical need.
- **Do not** claim the task is complete without compiling and verifying critical scenarios.

---

## 11. Compliance checklist / acceptance criteria

### Window

- [ ] The default GTK title bar is removed; app icon and controls are integrated at the top of the window.
- [ ] Dragging a dedicated region moves the window; double-clicking maximizes/restores it.
- [ ] Minimize, maximize/restore, and close actions work and are accessible.
- [ ] Resizing works from **all four edges and four corners**, with appropriate pointer cursors.
- [ ] The close-button behavior is clear and consistent with the application.

### Layout

- [ ] Common features are available from the main page, grouped into readable sections.
- [ ] Necessary tabs and popups remain available.
- [ ] No horizontal scrolling; vertical scrolling only when needed.
- [ ] Important controls remain accessible as the window becomes narrow.
- [ ] The content adapts to minimum, intermediate, and maximum sizes, including longer localized text.

### Appearance

- [ ] All **seven** themes are present: Graphite, Light, Ocean, Emerald, Rose, Apple Lime, and Steam Legacy.
- [ ] All seven JSON files are valid, and each palette has a stable ID.
- [ ] Live theme switching and reloading work without losing application state.
- [ ] Buttons, fields, sections, menus, popups, tooltips, and dialogs use consistent styling.
- [ ] Hover, focus, active, checked, and disabled states are visible and have sufficient contrast.

### Localization

- [ ] Languages load from UTF-8 JSON files, with English fallback.
- [ ] Live language switching and reloading work without restarting the app.
- [ ] Static text, dynamic text, tooltips, dialogs, and visualizers are translated.
- [ ] Repeated language changes do not damage text or widget state.
- [ ] Language resources are included in installable distributions.

### Linux AppImage

- [ ] A maintained, reproducible `tools/build-appimage.sh` script is present.
- [ ] The x86_64 AppImage is built from current sources and provided as a release artifact.
- [ ] All seven themes, translations, and other required assets are actually included in the package.
- [ ] AppDir permissions are valid; there are no warnings about unreadable files or other blocking packaging issues.
- [ ] The AppImage launches after being moved away from the development directory, on supported Linux distributions.

### Regression testing

- [ ] The project builds successfully; newly introduced warnings are addressed.
- [ ] Existing UI tests and application-specific critical scenarios pass.
- [ ] No new blocking loops or meaningful performance regressions are introduced.
- [ ] Configuration and preferences remain persistent and correct.
- [ ] Distribution is verified on the advertised environments, with readable runtime assets.

---

## 12. Recommended repository structure

```text
wozt-gtk-standard/
├── README.md                    # Brief project introduction
├── STANDARD.md                  # This specification and rules for coding agents/LLMs
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
    │   ├── 01-fr.json
    │   ├── 02-de.json
    │   ├── 03-it.json
    │   ├── 04-es.json
    │   ├── 05-pt.json
    │   ├── 06-ru.json
    │   ├── 07-ja.json
    │   ├── 08-ko.json
    │   └── 09-zh-cn.json
    └── appimage/
        └── build-appimage-wupstreamd.reference.sh # Concrete, adaptable example
```

The reference JSON files and packaging script are copied **unchanged from the WUP Stream repository**. Applications may use additional domain-specific keys and sections, but should preserve the spirit of the standard palettes and internationalization approach. The examples are a starting point, not a mandatory direct-import dependency.

### Reference sources (read-only)

- [`wozt/wup-stream-server-linux/src/ui.c`](https://github.com/wozt/wup-stream-server-linux/blob/main/src/ui.c): custom window chrome, resize zones, responsive layout, CSS styling, themes, and language selection.
- [`wozt/wup-stream-server-linux/src/i18n.c`](https://github.com/wozt/wup-stream-server-linux/blob/main/src/i18n.c): JSON loading, dynamic translation, and English fallback.
- [`wozt/wup-stream-server-linux/src/settings.c`](https://github.com/wozt/wup-stream-server-linux/blob/main/src/settings.c): persistence of theme and language IDs.
- [`wozt/wup-stream-server-linux/data/themes`](https://github.com/wozt/wup-stream-server-linux/tree/main/data/themes): seven default themes.
- [`wozt/wup-stream-server-linux/data/languages`](https://github.com/wozt/wup-stream-server-linux/tree/main/data/languages): multilingual examples.
- [`wozt/wup-stream-server-linux/tools/build-appimage.sh`](https://github.com/wozt/wup-stream-server-linux/blob/main/tools/build-appimage.sh): reference packaging script; adapt it and secure bundled asset permissions with `chmod -R a+rX`.

---

## 13. Short instruction for a coding agent/LLM

> **Task:** Adapt this application's UI to the **WOZT GTK Standard** (`STANDARD.md`) and the JSON files in `examples/themes/` and `examples/languages/`. Audit the existing application first. Preserve all functionality, settings, and performance. Create a custom top bar without native GTK window decorations, including the app icon, window controls, and dragging. Implement resizing from all eight zones. Build a compact, responsive main page without horizontal scrolling. Integrate all seven default JSON themes, including Steam Legacy, with live switching. Add JSON-based localization and dynamic language changes, including popups and custom-drawn surfaces. Keep genuinely useful tabs and popups. Build a reproducible x86_64 AppImage with bundled assets and verified file permissions. Test all interactions, live changes, and distribution. Do not mark the task as complete until the checks in Section 11 have passed.

**End of WOZT GTK Standard v1.0.**
