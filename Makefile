# VMManager - Makefile
# Native GTK4 VM & Container Manager
#
# Usage:
#   make          - build the application
#   make clean    - remove build artifacts
#   make run      - build and run
#   make install  - install to /usr/local/bin
#   make debug    - build with debug symbols
#   make test     - build test programs

CC       = gcc
TARGET   = vmmanager

# Source files
SOURCES  = src/main.c \
           src/ui/window.c \
           src/ui/dialogs.c \
           src/backend/vm_manager.c \
           src/backend/ct_manager.c \
           src/backend/snapshot_manager.c \
           src/utils/system_info.c \
           src/utils/error_handling.c

# Compiler flags
PKG_DEPS = gtk4 libvirt libcurl json-glib-1.0
CFLAGS   = -Wall -Wextra -Wno-unused-parameter -Iinclude $(shell pkg-config --cflags $(PKG_DEPS))
LDFLAGS  = $(shell pkg-config --libs $(PKG_DEPS))

# Release build (default)
CFLAGS  += -O2

.PHONY: all clean run install debug test uninstall

all: $(TARGET)

$(TARGET): $(SOURCES) include/vmmanager.h
	$(CC) $(CFLAGS) -o $(TARGET) $(SOURCES) $(LDFLAGS)
	@echo ""
	@echo "  VMManager built successfully!"
	@echo "  Run with: ./vmmanager"
	@echo ""

debug: CFLAGS += -g -DDEBUG -O0
debug: clean $(TARGET)

clean:
	rm -f $(TARGET)
	rm -f tests/create_ct_test tests/create_ct_test2 tests/create_ct_test3

run: $(TARGET)
	./$(TARGET)

install: $(TARGET)
	install -Dm755 $(TARGET) /usr/local/bin/$(TARGET)
	@echo "Installed to /usr/local/bin/$(TARGET)"

uninstall:
	rm -f /usr/local/bin/$(TARGET)
	@echo "Removed /usr/local/bin/$(TARGET)"

test:
	$(CC) -Wall -Iinclude tests/create_ct_test.c -o tests/create_ct_test $(LDFLAGS) $(shell pkg-config --cflags $(PKG_DEPS))
	$(CC) -Wall -Iinclude tests/create_ct_test2.c -o tests/create_ct_test2 $(LDFLAGS) $(shell pkg-config --cflags $(PKG_DEPS))
	$(CC) -Wall -Iinclude tests/create_ct_test3.c -o tests/create_ct_test3 $(LDFLAGS) $(shell pkg-config --cflags $(PKG_DEPS))
	@echo "Test binaries built in tests/"
