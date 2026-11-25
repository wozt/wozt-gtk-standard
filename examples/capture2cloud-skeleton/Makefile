CC ?= cc
PKGS = gtk+-3.0 json-glib-1.0
CFLAGS ?= -O2 -g
all: build/wozt-gtk-skeleton
build/wozt-gtk-skeleton: server/main.c server/reference_layout.h server/wozt_ui_standard.c server/wozt_ui_standard.h
	mkdir -p build
	$(CC) $(CFLAGS) -Wall -Wextra $$(pkg-config --cflags $(PKGS)) server/main.c server/wozt_ui_standard.c -o $@ $$(pkg-config --libs $(PKGS)) -lm
check: all
	xvfb-run -a ./build/wozt-gtk-skeleton --self-test
	python3 tools/check-wozt-translations.py
clean:
	rm -rf build
.PHONY: all check clean
