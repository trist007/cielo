#include "platform.h"
#include "glad/gl.h"
#include "platform.h"
#include "xdg-shell-client-protocol.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

// --- XDG Shell Base Listener ---
static void xdg_wm_base_ping(void* data, struct wl_xdg_wm_base* xdg_wm_base, uint32_t serial) 
{
  wl_xdg_wm_base_pong(xdg_wm_base, serial);
}

static const struct wl_xdg_wm_base_listener xdg_wm_base_listener = {
  .ping = xdg_wm_base_ping 
};

// --- Keyboard Input Listeners ---
static void keyboard_keymap(void* data, struct wl_keyboard* wl_keyboard, uint32_t format, int32_t fd, uint32_t size) 
{
  // A minimal engine framework only checking the Escape key does not need full keyboard layouts.
  // We close the file descriptor immediately to avoid resource leaks.
  close(fd); 
}

static void keyboard_enter(void* data, struct wl_keyboard* wl_keyboard, uint32_t serial, struct wl_surface* surface, struct wl_array* keys) {}
static void keyboard_leave(void* data, struct wl_keyboard* wl_keyboard, uint32_t serial, struct wl_surface* surface) {}

static void keyboard_key(void* data, struct wl_keyboard* wl_keyboard, uint32_t serial, uint32_t time, uint32_t key, uint32_t state) 
{
  PlatformWindow* window = (PlatformWindow*)data;
  if (!window) return;

  bool pressed = (state == WL_KEYBOARD_KEY_STATE_PRESSED);
  int mapped_key = KEY_NONE;

  // Linux kernel keycode for Escape is natively 1
  if (key == KEY_ESC) 
  {
    mapped_key = KEY_ESC;
  }

  if (mapped_key != KEY_NONE) 
  {
    window->keys[mapped_key] = pressed;
  }
}

static void keyboard_modifiers(void* data, struct wl_keyboard* wl_keyboard, uint32_t serial, uint32_t mods_depressed, uint32_t mods_latched, uint32_t mods_locked, uint32_t group) {}
static void keyboard_repeat_info(void* data, struct wl_keyboard* wl_keyboard, int32_t rate, int32_t delay) {}

static const struct wl_keyboard_listener keyboard_listener = {
  .keymap = keyboard_keymap,
  .enter = keyboard_enter,
  .leave = keyboard_leave,
  .key = keyboard_key,
  .modifiers = keyboard_modifiers,
  .repeat_info = keyboard_repeat_info
};

// --- Seat Capabilities Listener ---
static void seat_capabilities(void* data, struct wl_seat* wl_seat, uint32_t capabilities) 
{
  PlatformWindow* window = (PlatformWindow*)data;
  if (!window) return;

  if (capabilities & WL_SEAT_CAPABILITY_KEYBOARD) 
  {
    window->wl_keyboard = wl_seat_get_keyboard(wl_seat);
    wl_keyboard_add_listener(window->wl_keyboard, &keyboard_listener, window);
  } 
  else if (window->wl_keyboard) 
  {
    wl_keyboard_destroy(window->wl_keyboard);
    window->wl_keyboard = NULL;
  }
}

static void seat_name(void* data, struct wl_seat* wl_seat, const char* name) {}

static const struct wl_seat_listener seat_listener = {
  .capabilities = seat_capabilities,
  .name = seat_name
};

// --- XDG Surface Listeners ---
static void xdg_surface_configure(void* data, struct xdg_surface* xdg_surface, uint32_t serial) 
{
  xdg_surface_ack_configure(xdg_surface, serial);
}

static const struct xdg_surface_listener xdg_surface_listener = {
  .configure = xdg_surface_configure
};

static void xdg_toplevel_configure(void* data, struct xdg_toplevel* xdg_toplevel, int32_t width, int32_t height, struct wl_array* states) 
{
  PlatformWindow* window = (PlatformWindow*)data;
  if (!window) return;

  // Wayland passes a size of 0,0 when it expects the application to choose its own layout sizing
  if (width > 0 && height > 0 && (width != window->width || height != window->height)) 
  {
    window->width = width;
    window->height = height;

    if (window->wl_egl_window) 
    {
      wl_egl_window_resize(window->wl_egl_window, width, height, 0, 0);
    }
    if (window->framebuffer_size_callback) 
    {
      window->framebuffer_size_callback(window, width, height);
    }
  }
}

static void xdg_toplevel_close(void* data, struct xdg_toplevel* xdg_toplevel) 
{
  PlatformWindow* window = (PlatformWindow*)data;
  if (window) 
  {
    window->should_close = true;
  }
}

static const struct xdg_toplevel_listener xdg_toplevel_listener = {
  .configure = xdg_toplevel_configure,
  .close = xdg_toplevel_close
};

// --- Wayland Global Registry Listener ---
static void registry_handle_global(void* data, struct wl_registry* registry, uint32_t id, const char* interface, uint32_t version) 
{
  PlatformWindow* window = (PlatformWindow*)data;
  if (!window) return;

  if (strcmp(interface, "wl_compositor") == 0) 
  {
    window->wl_compositor = wl_registry_bind(registry, id, &wl_compositor_interface, 4);
  } 
  else if (strcmp(interface, "wl_xdg_wm_base") == 0) 
  {
    window->xdg_wm_base = wl_registry_bind(registry, id, &wl_xdg_wm_base_interface, 1);
    wl_xdg_wm_base_add_listener(window->xdg_wm_base, &xdg_wm_base_listener, NULL);
  } 
  else if (strcmp(interface, "wl_seat") == 0) 
  {
    window->wl_seat = wl_registry_bind(registry, id, &wl_seat_interface, 7);
    wl_seat_add_listener(window->wl_seat, &seat_listener, window);
  }
}

static void registry_handle_global_remove(void* data, struct wl_registry* registry, uint32_t id) {}

static const struct wl_registry_listener registry_listener = {
  .global = registry_handle_global,
  .global_remove = registry_handle_global_remove
};

// --- Platform Layer Interface Functions ---

int platform_init(void) 
{
  return 1; // Handled per-display connection in window generation below
}

PlatformWindow* platform_create_window(int width, int height, const char* title) 
{
  PlatformWindow* window = (PlatformWindow*)malloc(sizeof(PlatformWindow));
  if (!window) return NULL;
  memset(window, 0, sizeof(PlatformWindow));

  window->width = width;
  window->height = height;
  window->should_close = false;

  window->wl_display = wl_display_connect(NULL);
  if (!window->wl_display) 
  {
    fprintf(stderr, "Failed to connect to Wayland server\n");
    free(window);
    return NULL;
  }

  window->wl_registry = wl_display_get_registry(window->wl_display);
  wl_registry_add_listener(window->wl_registry, &registry_listener, window);
  
  // Synchronous roundtrips allow registry bindings to completely finish matching
  wl_display_roundtrip(window->wl_display);
  wl_display_roundtrip(window->wl_display);

  if (!window->wl_compositor || !window->xdg_wm_base) 
  {
    fprintf(stderr, "Wayland missing critical compositor or shell globals\n");
    goto cleanup;
  }

  window->wl_surface = wl_compositor_create_surface(window->wl_compositor);
  window->xdg_surface = wl_xdg_wm_base_get_xdg_surface(window->xdg_wm_base, window->wl_surface);
  xdg_surface_add_listener(window->xdg_surface, &xdg_surface_listener, window);

  window->xdg_toplevel = xdg_surface_get_toplevel(window->xdg_surface);
  xdg_toplevel_add_listener(window->xdg_toplevel, &xdg_toplevel_listener, window);
  xdg_toplevel_set_title(window->xdg_toplevel, title);

  // Inform the server about your initial surface layout configurations
  wl_surface_commit(window->wl_surface);
  wl_display_roundtrip(window->wl_display);

  // EGL Graphics Integration
  window->egl_display = eglGetPlatformDisplay(EGL_PLATFORM_WAYLAND_KHR, window->wl_display, NULL);
  if (window->egl_display == EGL_NO_DISPLAY) goto cleanup;

  EGLint major, minor;
  if (!eglInitialize(window->egl_display, &major, &minor)) goto cleanup;

  static const EGLint attribs[] = {
    EGL_SURFACE_TYPE,    EGL_WINDOW_BIT,
    EGL_RED_SIZE,        8,
    EGL_GREEN_SIZE,      8,
    EGL_BLUE_SIZE,       8,
    EGL_ALPHA_SIZE,      8,
    EGL_DEPTH_SIZE,      24,
    EGL_RENDERABLE_TYPE, EGL_OPENGL_BIT,
    EGL_NONE
  };

  EGLConfig config;
  EGLint num_configs;
  if (!eglChooseConfig(window->egl_display, attribs, &config, 1, &num_configs) || num_configs == 0) goto cleanup;

  eglBindAPI(EGL_OPENGL_API);

  static const EGLint ctx_attribs[] = {
    EGL_CONTEXT_MAJOR_VERSION, 3,
    EGL_CONTEXT_MINOR_VERSION, 3,
    EGL_CONTEXT_OPENGL_PROFILE_MASK, EGL_CONTEXT_OPENGL_CORE_PROFILE_BIT,
    EGL_NONE
  };

  window->egl_context = eglCreateContext(window->egl_display, config, EGL_NO_CONTEXT, ctx_attribs);
  if (window->egl_context == EGL_NO_CONTEXT) goto cleanup;

  // wayland-egl dynamically bridges our raw pixel surface context boundaries
  window->wl_egl_window = wl_egl_window_create(window->wl_surface, width, height);
  if (!window->wl_egl_window) goto cleanup;

  window->egl_surface = eglCreatePlatformWindowSurface(window->egl_display, config, window->wl_egl_window, NULL);
  if (window->egl_surface == EGL_NO_SURFACE) goto cleanup;

  if (!eglMakeCurrent(window->egl_display, window->egl_surface, window->egl_surface, window->egl_context)) goto cleanup;

  printf("Wayland EGL Initialized: %s\n", eglQueryString(window->egl_display, EGL_VERSION));
  return window;

cleanup:
  if (window->egl_display != EGL_NO_DISPLAY) 
  {
    eglMakeCurrent(window->egl_display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
    if (window->egl_surface != EGL_NO_SURFACE) eglDestroySurface(window->egl_display, window->egl_surface);
    if (window->egl_context != EGL_NO_CONTEXT) eglDestroyContext(window->egl_display, window->egl_context);
    eglTerminate(window->egl_display);
  }
  if (window->wl_egl_window) wl_egl_window_destroy(window->wl_egl_window);
  if (window->xdg_toplevel) xdg_toplevel_destroy(window->xdg_toplevel);
  if (window->xdg_surface) xdg_surface_destroy(window->xdg_surface);
  if (window->wl_surface) wl_surface_destroy(window->wl_surface);
  if (window->wl_display) wl_display_disconnect(window->wl_display);
  free(window);
  return NULL;
}

void platform_poll_events(PlatformWindow* window) 
{
  if (!window || !window->wl_display) return;

  // Pipelined event dispatch processing without thread-blocking waits
  wl_display_dispatch_pending(window->wl_display);
  wl_display_flush(window->wl_display);

  if (wl_display_prepare_read(window->wl_display) == 0) 
  {
    wl_display_read_events(window->wl_display);
    wl_display_dispatch_pending(window->wl_display);
  } 
  else 
  {
    wl_display_dispatch_pending(window->wl_display);
  }
}
