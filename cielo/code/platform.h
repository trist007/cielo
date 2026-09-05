#ifndef PLATFORM_H
#define PLATFORM_H

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#if defined(_WIN32)

#include <windows.h>
#include "GL/glcorearb.h"
#include "wglext.h"
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
  KEY_ESC,
  KEY_Q,
  KEY_C,
  KEY_W,
  KEY_A,
  KEY_S,
  KEY_D,
  KEY_F,
  KEY_UP,
  KEY_DOWN,
  KEY_LEFT,
  KEY_RIGHT,
  KEY_COUNT
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
  bool keys[KEY_COUNT];
  bool keywasDown[KEY_COUNT];
  int  mouseX;
  int  mouseY;
  bool mouseButtons[3];
  void* userData;
} PlatformWindow;

int platform_init(void);
PlatformWindow* platform_create_window(int width, int height, const char* title);
void  platform_set_user_data(PlatformWindow* window, void* data);
void* platform_get_user_data(PlatformWindow* window);
void platform_terminate(void);
void platform_set_framebuffer_size_callback(PlatformWindow* window, 
                                            void (*callback)(PlatformWindow* window, int width, int height));
bool platform_window_should_close(PlatformWindow* window);
void platform_window_close(PlatformWindow* window);
void platform_swap_buffers(PlatformWindow* window);
void* platform_get_proc_address(const char* name);
void platform_poll_events(PlatformWindow* window);
int  platform_get_key(PlatformWindow* window, int key);

bool platform_key_down(PlatformWindow* window, int key);
bool platform_key_just_pressed(PlatformWindow* window, int key);
bool platform_key_just_released(PlatformWindow* window, int key);

#endif // PLATFORM_H
