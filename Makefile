CC := gcc

GTK_LIBS  := -lgtk-3 -lgdk-3 -lz -lpangocairo-1.0 -lpango-1.0 -lharfbuzz -latk-1.0 -lcairo-gobject -lcairo -lgdk_pixbuf-2.0 -lgio-2.0 -lgobject-2.0 -lglib-2.0
GTK_FLAGS := -I/usr/include/gtk-3.0 -I/usr/include/pango-1.0 -I/usr/include/glib-2.0 -I/usr/lib/x86_64-linux-gnu/glib-2.0/include \
	-I/usr/include/sysprof-6 -I/usr/include/harfbuzz -I/usr/include/freetype2 -I/usr/include/libpng16 -I/usr/include/libmount -I/usr/include/blkid \
	-I/usr/include/fribidi -I/usr/include/cairo -I/usr/include/pixman-1 -I/usr/include/gdk-pixbuf-2.0 -I/usr/include/x86_64-linux-gnu \
	-I/usr/include/webp -I/usr/include/gio-unix-2.0 -I/usr/include/atk-1.0 -I/usr/include/at-spi2-atk/2.0 -I/usr/include/at-spi-2.0 \
	-I/usr/include/dbus-1.0 -I/usr/lib/x86_64-linux-gnu/dbus-1.0/include -pthread

CCFLAGS := $(GTK_FLAGS) $(GTK_LIBS) -lssl -lreadline -lgit2

BUILD := build
SRC   := src
BIN   := bin

BINARY := HOS

# Version 1.3.0
VERSION_MAJOR := 1
VERSION_MINOR := 3
VERSION_PATCH := 0

# C files to compile
C_FILES := \
src/main.c \
src/root.c \
src/main_util.c \
	src/net/netroot.c \
	src/net/netutils.c \
	src/net/ping.c \
		src/net/NetWatch/netwatch.c \
		src/net/HTP/htp_utils.c \
		src/net/HTP/htp_client.c \
		src/net/HTP/htp_server.c \
	src/lib/ll.c \
	src/lib/memory.c \
	src/lib/tree.c \
		src/lib/Commands/cmd_basic.c \
		src/lib/Commands/cmd_hash.c \
		src/lib/Commands/cmd_hpm.c \
		src/lib/Commands/cmd_htp.c \
		src/lib/Commands/cmd_netwatch.c \
		src/lib/Commands/cmd_ping.c \
		src/lib/Commands/cmd_run.c \
		src/lib/gtk/gtk_main.c \
		src/lib/gtk/gtk_utils.c \
			src/lib/gtk/Apps/app_base.c \
				src/lib/gtk/Apps/terminal/gtk_terminal.c \
				src/lib/gtk/Apps/NetWatch/gtk_netwatch.c \
				src/lib/gtk/Apps/NetWatch/gtk_netwatch_helper.c \
	src/HPM/hpm.c \
	src/HPM/install_database.c \
	src/Crypto/crypto.c \
	src/cJSON/cJSON.c \
	src/cJSON/cJSON_Utils.c

OBJ_FILES := $(C_FILES:$(SRC)/%.c=$(BUILD)/%.c.o)

all: $(BUILD) $(BIN) $(BINARY)

$(BUILD):
	@mkdir -p $(BUILD)

$(BIN):
	@mkdir -p $(BIN)

$(BINARY): $(OBJ_FILES)
	@$(CC) $(OBJ_FILES) -o $(BIN)/$@

$(BUILD)/%.c.o: $(C_FILES)
	@$(CC) $(CCFLAGS) $< -o $@

run:
	@./bin/$(BINARY)

clean:
	rm -rf $(BUILD)/* $(BIN)/*
