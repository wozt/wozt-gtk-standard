# WOZT GTK Standard

**A design standard for building and modernizing GTK desktop interfaces on Linux**, based on the UI work carried out in [WUP Stream](https://github.com/wozt/wup-stream-server-linux).

It gives developers and AI coding agents (Codex, SWE-2, and others) a shared reference for creating visually consistent applications without sacrificing existing features or performance.

The standard covers:

- **Custom window chrome:** no native GTK title bar; integrated app icon and window controls, draggable and resizable from all four edges and four corners.
- **Compact, responsive layouts:** a single main page where practical, with tabs and secondary windows allowed when useful.
- **Seven built-in JSON themes:** Graphite, Light, Ocean, Emerald, Rose, Apple Lime, and Steam Legacy.
- **JSON-based localization:** live language switching, including menus, tooltips, dialogs, and dynamically rendered content.
- **Consistent visual design** and reproducible **Linux x86_64 AppImage** builds.

## How to use it

Read **[STANDARD.md](STANDARD.md)** and ask your coding agent to apply it to the target project while preserving its features, settings, and performance.

The [`examples/`](examples/) directory contains theme palettes, language files, and a reference AppImage build script. These are **adaptable examples**, not a drop-in GTK library.
