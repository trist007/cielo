#include "platform.h"

static PFNEGLGETPLATFORMDISPLAYEXTPROC eglGetPlatformDisplayEXT = NULL;

int platform_init(void)
{
  eglGetPlatformDisplayEXT = (PFNEGLGETPLATFORMDISPLAYEXTPROC)eglGetProcAddress("eglGetPlatformDisplayEXT");
  if (!eglGetPlatformDisplayEXT)
  {
    fprintf(stderr, "EGL_EXT_platform_xcb not supported\n");
    return(-1);
  }
  return(1);
}

PlatformWindow* platform_create_window(int width, int height, const char* title)
{
  PlatformWindow* window = (PlatformWindow*)malloc(sizeof(PlatformWindow));
  
  if (!window) return NULL;
  
  memset(window, 0, sizeof(PlatformWindow));
  
  window->width       = width;
  window->height      = height;
  window->should_close = false;
  
  window->conn = xcb_connect(NULL, NULL);
  if (!window->conn || xcb_connection_has_error(window->conn))
  {
    fprintf(stderr, "Failed to connect to X server\n");
    free(window);
    return(NULL);
  }
  
  window->screen = xcb_setup_roots_iterator(xcb_get_setup(window->conn)).data;
  
  window->window = xcb_generate_id(window->conn);
  uint32_t mask = XCB_CW_BACK_PIXMAP | XCB_CW_EVENT_MASK;
  uint32_t values[2] = 
  {
    XCB_NONE,
    XCB_EVENT_MASK_EXPOSURE |
    XCB_EVENT_MASK_KEY_PRESS |
    XCB_EVENT_MASK_KEY_RELEASE |
    XCB_EVENT_MASK_STRUCTURE_NOTIFY
  };

  xcb_create_window(window->conn,
                    XCB_COPY_FROM_PARENT,
                    window->window,
                    window->screen->root,
                    0,
                    0,
                    width,
                    height,
                    0,
                    XCB_WINDOW_CLASS_INPUT_OUTPUT,
                    window->screen->root_visual,
                    mask,
                    values
  );
  
  xcb_change_property(window->conn, XCB_PROP_MODE_REPLACE, window->window, XCB_ATOM_WM_NAME,
                      XCB_ATOM_STRING, 8, (uint32_t)strlen(title), title);
  
  xcb_intern_atom_cookie_t wm_delete_cookie = xcb_intern_atom(window->conn, 0, 16, "WM_DELETE_WINDOW");
  xcb_intern_atom_cookie_t wm_protocols_cookie = xcb_intern_atom(window->conn, 0, 12, "WM_PROTOCOLS");
  xcb_intern_atom_reply_t* wm_delete = xcb_intern_atom_reply(window->conn, wm_delete_cookie, NULL);
  xcb_intern_atom_reply_t* wm_protocols = xcb_intern_atom_reply(window->conn, wm_protocols_cookie, NULL);

  if (wm_delete && wm_protocols)
  {
    xcb_change_property(window->conn, XCB_PROP_MODE_REPLACE, window->window,
                        wm_protocols->atom, XCB_ATOM_ATOM, 32, 1, &wm_delete->atom);
    window->wm_delete_atom = wm_delete->atom;
  }
  free(wm_delete);
  free(wm_protocols);
  
  xcb_map_window(window->conn, window->window);
  xcb_flush(window->conn);
  
  window->egl_display = eglGetPlatformDisplayEXT(EGL_PLATFORM_XCB_EXT, window->conn, NULL);
  if (window->egl_display == EGL_NO_DISPLAY)
  {
    fprintf(stderr, "Failed to get EGL display\n");
    goto cleanup;
  }
  
  EGLint major, minor;
  if (!eglInitialize(window->egl_display, &major, &minor))
  {
    fprintf(stderr, "Failed to initialize EGL\n");
    goto cleanup;
  }
  
  static const EGLint config_attribs[] =
  {
    EGL_SURFACE_TYPE,     EGL_WINDOW_BIT,
    EGL_RED_SIZE,         8,
    EGL_GREEN_SIZE,       8,
    EGL_BLUE_SIZE,        8,
    EGL_ALPHA_SIZE,       8,
    EGL_DEPTH_SIZE,       24,
    EGL_STENCIL_SIZE,     8,
    EGL_RENDERABLE_TYPE,  EGL_OPENGL_BIT,
    EGL_NONE
  };
  
  EGLConfig config;
  EGLint num_configs;
  if (!eglChooseConfig(window->egl_display, config_attribs, &config, 1, &num_configs) || num_configs == 0)
  {
    fprintf(stderr, "Failed to choose EGL config\n");
    goto cleanup;
  }
  
  eglBindAPI(EGL_OPENGL_API);

  static const EGLint ctx_attribs[] =
  {
    EGL_CONTEXT_MAJOR_VERSION,  3,
    EGL_CONTEXT_MINOR_VERSION,  3,
    EGL_CONTEXT_OPENGL_PROFILE_MASK, EGL_CONTEXT_OPENGL_CORE_PROFILE_BIT,
    EGL_NONE
  };
  
  window->egl_context = eglCreateContext(window->egl_display, config, EGL_NO_CONTEXT, ctx_attribs);
  if (window->egl_context == EGL_NO_CONTEXT)
  {
    fprintf(stderr, "Failed toc reate EGL context\n");
    goto cleanup;
  }
  
  window->egl_surface = eglCreatePlatformWindowSurface(window->egl_display, config, &window->window, NULL);
  if (window->egl_surface == EGL_NO_SURFACE)
  {
    fprintf(stderr, "Failed to create EGL surface\n");
    goto cleanup;
  }
  
  if (!eglMakeCurrent(window->egl_display, window->egl_surface, window->egl_surface, window->egl_context))
  {
    fprintf(stderr, "Failed to make EGL context current\n");
    goto cleanup;
  }
  
  printf("EGL initialized: %s %d.%d\n", eglQueryString(window->egl_display, EGL_VENDOR), major, minor);
  return(window);

cleanup:
  platform_terminate();
  free(window);
  return(NULL);
}

void platform_terminate(void)
{
  // TODO
}

void platform_set_framebuffer_size_callback(PlatformWindow* window, void (*callback)(PlatformWindow* window, int width, int height))
{
  window->framebuffer_size_callback = callback;
}

bool platform_window_should_close(PlatformWindow* window)
{
  return(window->should_close);
}

void platform_window_close(PlatformWindow* window)
{
  window->should_close = true;
}

void platform_swap_buffers(PlatformWindow* window)
{
  eglSwapBuffers(window->egl_display, window->egl_surface);
}

void* platform_get_proc_address(const char* name)
{
  return eglGetProcAddress(name);
}

void platform_poll_events(PlatformWindow* window)
{
  xcb_generic_event_t* event;
  while ((event = xcb_poll_for_event(window->conn)))
  {
    // NOTE(trist007): what does this do!?!?
    switch (event->response_type & ~0x80)
    {
      case XCB_EXPOSE:
        break;
      
      case XCB_CONFIGURE_NOTIFY:
      {
        xcb_configure_notify_event_t* cfg = (xcb_configure_notify_event_t*)event;
        if (cfg->width != window->width || cfg->height != window->height)
        {
          window->width = cfg->width;
          window->height = cfg->height;
          if (window->framebuffer_size_callback)
          {
            window->framebuffer_size_callback(window, window->width, window->height);
          }
        }
        break;
      }
      
      case XCB_KEY_PRESS:
      case XCB_KEY_RELEASE:
      {
        xcb_key_press_event_t* kp = (xcb_key_press_event_t*)event;
        int pressed = (event->response_type & ~0x80) == XCB_KEY_PRESS;

        int key = KEY_NONE;
        switch (kp->detail)
        {
          case 9:
          {
            key = KEY_ESC;
            break;
          }
          default:
          {
            key = KEY_NONE;
          }
        }
        
        window->keys[key] = pressed;
        break;
      }
      
      case XCB_CLIENT_MESSAGE:
      {
       xcb_client_message_event_t* cm = (xcb_client_message_event_t*)event;
        if (cm->data.data32[0] == window->wm_delete_atom)
        {
          window->should_close = true;
        }
        break;
      }
    }
    free(event);
  }
}

int platform_get_key(PlatformWindow* window, int key)
{
  if (key < 0 || key >= 512)
  {
    return(0);
  }
  
  return window->keys[key] ? KEY_PRESSED : KEY_RELEASED;
}

bool platform_key_down(PlatformWindow* w, int key)
{
    return w->keys[key];
}

bool platform_key_just_pressed(PlatformWindow* w, int key)
{
    return w->keys[key] && !w->keywasDown[key];
}

bool platform_key_just_released(PlatformWindow* w, int key)
{
    return !w->keys[key] && w->keywasDown[key];
}
