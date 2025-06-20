WAYLAND_SCANNER=wayland-scanner
CFLAGS=-Wall -Wextra -g $(shell pkg-config --cflags wayland-client) -I protocols.out
LDFLAGS=$(shell pkg-config --libs wayland-client)
PREFIX=/usr/local

PROTO_XML=/usr/share/wayland-protocols/staging/ext-idle-notify/ext-idle-notify-v1.xml

PROTO_OUT=protocols.out
PROTO_C=$(PROTO_OUT)/ext-idle-notify-client-protocol.c
PROTO_H=$(PROTO_OUT)/ext-idle-notify-client-protocol.h
SRC=src/idle_watch.c
OUT=idle_watch

all: $(OUT)

$(PROTO_OUT):
	mkdir -p $@

$(PROTO_C): $(PROTO_XML) | $(PROTO_OUT)
	$(WAYLAND_SCANNER) private-code $< $@

$(PROTO_H): $(PROTO_XML) | $(PROTO_OUT)
	$(WAYLAND_SCANNER) client-header $< $@

$(OUT): $(SRC) $(PROTO_C) $(PROTO_H)
	$(CC) $(CFLAGS) -o $@ $(SRC) $(PROTO_C) $(LDFLAGS)

format:
	clang-format -i $(SRC)

install:
	install $(OUT) $(PREFIX)/bin/$(OUT)

uninstall:
	rm $(PREFIX)/bin/$(OUT)

clean:
	-rm -f $(OUT) $(PROTO_C) $(PROTO_H)

.PHONY: all clean install uninstall format
