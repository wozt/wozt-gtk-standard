# Capture2Cloud GTK3 skeleton (WOZT 1.3)

Standalone demonstration, not a streaming application. The theme and translation engine, JSON assets, resize helpers and proportional row allocation are derived from Capture2Cloud commit `35e0d43cfb0d7bbaa672005756543962ee7b4241`. The demo uses independent measured horizontal groups, a 2/3 utility row, compact sliders, a notebook, persistent theme/language preferences, and fixed window chrome/footer. Demo controls have no hardware side effects. Reset and maintenance are illustrative layout slots, not real hardware actions.

```sh
sudo apt install build-essential pkg-config libgtk-3-dev libjson-glib-dev xvfb xauth
make
./build/wozt-gtk-skeleton
make check
```

Run from this directory or set `WOZT_DATA_DIR` to the installed data directory. `--self-test` runs without saving preferences. The smoke test checks the reference arithmetic and exercises reflow and language switching; it is not a complete geometry or visual test. Inventory output may include demo-only strings or dynamic arguments; review and translate them before shipping a derived app. The existing standard's desktop identity, AppImage packaging and release CI gates still apply to real applications; this documentation skeleton does not publish an AppImage. Adapt all callbacks, full localization and application identity before distributing it.
