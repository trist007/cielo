#ifndef PLATFORM_H
#define PLATFORM_H

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if defined(_WIN32)

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#elif defined(__linux__)

#include <xcb/xcb.h>
#include <EGL/egl.h>
#include <EGL/eglext.h>

#else
#error "Platform not implemented"
#endif

typedef enum
{
  KEY_NONE = 0,
  KEY_ESC = 1
} PlatformKeys;

typedef enum
{
  KEY_RELEASED,
  KEY_PRESSED
} PlatformKeyState;

typedef struct PlatformWindow
{
#if defined(_WIN32)
  HWND  hwnd;
  HDC   hdc;
  HGLRC hglrc;
#elif defined(__linux__)
  xcb_connection_t* conn;
  xcb_window_t      window;
  xcb_screen_t*     screen;

  EGLDisplay        egl_display;
  EGLSurface        egl_surface;
  EGLContext        egl_context;
  // NOTE(trist007): added this for delete window
  xcb_atom_t        wm_delete_atom;
#else
#error "Platform not implemented"
#endif

  bool              should_close;
  int               width;
  int               height;

  void(*framebuffer_size_callback)(struct PlatformWindow* window, int width, int height);
  bool keys[512];
} PlatformWindow;

int platform_init(void);
PlatformWindow* platform_create_window(int width, int height, const char* title);
void platform_terminate(void);
void platform_set_framebuffer_size_callback(PlatformWindow* window, 
                                            void (*callback)(PlatformWindow* window, int width, int height));
bool platform_window_should_close(PlatformWindow* window);
void platform_window_close(PlatformWindow* window);
void platform_swap_buffers(PlatformWindow* window);
void* platform_get_proc_address(const char* name);
void platform_poll_events(PlatformWindow* window);
int  platform_get_key(PlatformWindow* window, int key);

#endif // PLATFORM_H
