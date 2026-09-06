# ---- Flags (MSVC -> clang translation) ----
# /std:c17         -> -std=c17
# /Zc:__STDC__     -> default on clang, no flag needed
# /MD              -> dynamic CRT is the default on Linux, no flag needed
# -fp:fast         -> -ffast-math
# -Od              -> -O0
# -WX -W4          -> -Werror -Wall -Wextra
# -wd4XXX          -> matching -Wno-* where an equivalent exists
# -Z7 -FC          -> -g (embed debug info, clang always emits full paths)
CC=clang
CFLAGS := -std=c17 -ffast-math -O0 -g -Werror -Wall -Wextra \
    -Wno-unused-parameter \
    -Wno-unused-variable \
    -Wno-unused-but-set-variable \
    -Wno-sign-compare \
    -Wno-deprecated-declarations \
    -Wno-uninitialized \
    $(INCLUDES)

# ---- Libraries ----
# meshoptimizer.lib -> -lmeshoptimizer
# opengl32.lib      -> -lGL
# user32.lib/gdi32.lib have no Linux equivalent; they were pulled in for Win32
# window/GDI plumbing. If platform.c talks to X11/Wayland/GLFW instead, link
# that here (e.g. -lX11, or -lglfw, -lSDL2, etc).
LDFLAGS := -L$(VCPKG_LIB) -L$(LIB_DIR) -lGL -lEGL -lX11 -lxcb -lm
# LDFLAGS := -L$(VCPKG_LIB) -L$(LIB_DIR) -lGL -lEGL -lX11 -lxcb -lwayland-client -lwayland-egl -lxkbcommon -lm

SRCS := gl_load.c platform.c terrain_demo1.c terrain.c
OBJS := $(SRCS:.c=.o)
TARGET := terrain

BUILD_COUNT_FILE := build_count.txt

.PHONY: all clean

all: buildcount $(TARGET)

buildcount:
	@if [ ! -f $(BUILD_COUNT_FILE) ]; then echo 0 > $(BUILD_COUNT_FILE); fi
	@count=$$(cat $(BUILD_COUNT_FILE)); \
	count=$$((count + 1)); \
	echo $$count > $(BUILD_COUNT_FILE); \
	echo "BUILD #$$count"; \
	echo "cwd: $$(pwd)"

$(TARGET): $(OBJS)
	@echo "[MAIN] Linking $(TARGET)..."
	$(CC) -o $(TARGET) $(OBJS) $(LDFLAGS)
	@echo "[MAIN] $(TARGET) built with OpenGL."

%.o: %.c
	@echo "[MAIN] Compiling $<..."
	$(CC) -c $(CFLAGS) -o $@ $<

clean:
	rm -f $(OBJS) $(TARGET) $(BUILD_COUNT_FILE)
