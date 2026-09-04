#include "glad/gl.h"
#include "wglext.h"
#include "platform.h"

static PFNWGLCREATECONTEXTATTRIBSARBPROC wglCreateContextAttribsARB = NULL;
static PFNWGLCHOOSEPIXELFORMATARBPROC    wglChoosePixelFormatARB    = NULL;

static void init_wgl_extensions(void)
{
  WNDCLASSW wc = {
  .style = CS_HREDRAW |  CS_VREDRAW | CS_OWNDC,
  .lpfnWndProc = DefWindowProcA,
  .hInstance = GetModuleHandle(NULL),
  .lpszClassName = L"DummyWGLWindow",
  };
  RegisterClassW(&wc);

  HWND dummy = CreateWindowExW(0,
                               wc.lpszClassName,
                               L"Dummy",
                               0,
                               CW_USEDEFAULT,
                               CW_USEDEFAULT,
                               CW_USEDEFAULT,
                               CW_USEDEFAULT,
                               NULL,
                               NULL,
                               wc.hInstance,
                               NULL
  );
  
  HDC dc = GetDC(dummy);
  
  PIXELFORMATDESCRIPTOR pfd = {
    .nSize = sizeof(pfd),
    .nVersion = 1,
    .dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER,
    .iPixelType = PFD_TYPE_RGBA,
    .cColorBits = 32,
    .cAlphaBits = 8,
    .cDepthBits = 24,
    .cStencilBits = 8,
    .iLayerType = PFD_MAIN_PLANE,
  };
  
  int pf = ChoosePixelFormat(dc, &pfd);
  SetPixelFormat(dc, pf, &pfd);

  HGLRC rc = wglCreateContext(dc);
  wglMakeCurrent(dc, rc);
  
  wglCreateContextAttribsARB = (PFNWGLCREATECONTEXTATTRIBSARBPROC)wglGetProcAddress("wglCreateContextAttribsARB");;
  wglChoosePixelFormatARB    = (PFNWGLCHOOSEPIXELFORMATARBPROC)wglGetProcAddress("wglChoosePixelFormatARB");
  
  wglMakeCurrent(NULL, NULL);
  wglDeleteContext(rc);
  ReleaseDC(dummy, dc);
  DestroyWindow(dummy);
}

int platform_init(void)
{
  init_wgl_extensions();
  if (!wglCreateContextAttribsARB || !wglChoosePixelFormatARB)
  {
    fprintf(stderr, "Failed to load WGL extensions for modern OpenGL\n");
    return(-1);
  }
  return(1);
}

static LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
  // PlatformWindow* window = (PlatformWindow*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
  
  if (msg == WM_NCCREATE) {
      CREATESTRUCTW* cs = (CREATESTRUCTW*)lparam;
      SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)cs->lpCreateParams);
  }
  PlatformWindow* window = (PlatformWindow*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
  
  switch(msg)
  {
    case WM_SIZE:
    {
      if (window)
      {
        int new_width = LOWORD(lparam);
        int new_height = HIWORD(lparam);
        if (new_width != window->width || new_height != window->height)
        {
          window->width = new_width;
          window->height = new_height;
          if (window->framebuffer_size_callback)
          {
            window->framebuffer_size_callback(window, new_width, new_height);
          }
        }
      }
      break;
    }
    case WM_KEYDOWN:
    case WM_KEYUP:
    {
      if (!window) break;
      bool pressed = (msg == WM_KEYDOWN);
      bool is_repeat = (msg == WM_KEYDOWN) && ((lparam & (1 << 30)) != 0);

      if (!is_repeat) {
        int key = KEY_NONE;
        switch (wparam) {
          case VK_ESCAPE: key = KEY_ESC; break;
          case 'Q':       key = KEY_Q;   break;
          case 'C':       key = KEY_C;   break;
          case 'W':       key = KEY_W;   break;
          case 'A':       key = KEY_A;   break;
          case 'S':       key = KEY_S;   break;
          case 'D':       key = KEY_D;   break;
          case 'F':       key = KEY_F;   break;
        }
        if (key != KEY_NONE)
          window->keys[key] = pressed;
      }
      break;
    }
    
    case WM_MOUSEMOVE:
    {
      if (window) {
        window->mouseX = (short)LOWORD(lparam);   // signed
        window->mouseY = (short)HIWORD(lparam);
      }
      break;
    }
      
    case WM_CLOSE:
    case WM_DESTROY:
      if (window) window->should_close = true;
      PostQuitMessage(0);
      return(0);
  }
  return DefWindowProcW(hwnd, msg, wparam, lparam);
}
                
PlatformWindow* platform_create_window(int width, int height, const char* title)
{
  PlatformWindow* window = (PlatformWindow*)malloc(sizeof(PlatformWindow));
  if(!window)
  {
    fprintf(stderr, "Failed to register window class\n");
    return(NULL);
  }
  memset(window,0, sizeof(PlatformWindow));
  
  window->width = width;
  window->height = height;

  WNDCLASSW wc = { 0 };
  wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
  wc.lpfnWndProc = WindowProc;
  wc.hInstance = GetModuleHandle(NULL);
  wc.hCursor = LoadCursor(NULL, IDC_ARROW);
  wc.lpszClassName = L"LearnOpenGL";
  
  if (!RegisterClassW(&wc))
  {
    fprintf(stderr, "Failed to register window class\n");
    goto cleanup;
  }
  
  int title_size = MultiByteToWideChar(CP_UTF8, 0, title, -1, NULL, 0);
  wchar_t* title_wide = (LPWSTR)malloc(sizeof(wchar_t) * title_size);
  
  MultiByteToWideChar(CP_UTF8, 0, title, -1, title_wide, title_size);
  
  window->hwnd = CreateWindowExW(
    0,
    L"LearnOpenGL",
    title_wide,
    WS_OVERLAPPEDWINDOW | WS_VISIBLE,
    CW_USEDEFAULT, CW_USEDEFAULT,
    width, height,
    NULL, NULL, GetModuleHandle(NULL), window
  );
  
  free(title_wide);
  
  if (!window->hwnd)
  {
    fprintf(stderr, "Failed to create window\n");
    goto cleanup;
  }
  
  SetWindowLongPtr(window->hwnd, GWLP_USERDATA, (LONG_PTR)window);
  
  window->hdc = GetDC(window->hwnd);
  
  int pixel_attribs[] = {
    WGL_DRAW_TO_WINDOW_ARB, GL_TRUE,
    WGL_SUPPORT_OPENGL_ARB, GL_TRUE,
    WGL_DOUBLE_BUFFER_ARB,  GL_TRUE,
    WGL_ACCELERATION_ARB,   WGL_FULL_ACCELERATION_ARB,
    WGL_PIXEL_TYPE_ARB,     WGL_TYPE_RGBA_ARB,
    WGL_COLOR_BITS_ARB,     32,
    WGL_DEPTH_BITS_ARB,     24,
    WGL_STENCIL_BITS_ARB,   8,
    0
  };
  
  int pixel_format;
  UINT num_formats;
  wglChoosePixelFormatARB(window->hdc, pixel_attribs, NULL, 1, &pixel_format, &num_formats);
  if (!num_formats)
  {
    fprintf(stderr, "Failed to choose pixel format\n");
    goto cleanup;
  }
  
  PIXELFORMATDESCRIPTOR pfd;
  DescribePixelFormat(window->hdc, pixel_format, sizeof(pfd), &pfd);
  SetPixelFormat(window->hdc, pixel_format, &pfd);

  int context_attribs[] = {
    WGL_CONTEXT_MAJOR_VERSION_ARB,  3,
    WGL_CONTEXT_MINOR_VERSION_ARB,  3,
    WGL_CONTEXT_PROFILE_MASK_ARB, WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
    0
  };
  
  window->hglrc = wglCreateContextAttribsARB(window->hdc, NULL, context_attribs);
  if (!window->hglrc)
  {
    fprintf(stderr, "Failed to create OpenGL 3.3 core context\n");
    goto cleanup;
  }
  
  if (!wglMakeCurrent(window->hdc, window->hglrc))
  {
    fprintf(stderr, "Failed to make context current\n");
    goto cleanup;
  }
  
  printf("WGL + OpenGL 3.3 core context initialized\n");
  return(window);
  
cleanup:
  if (window->hglrc) wglDeleteContext(window->hglrc);
  if (window->hdc) ReleaseDC(window->hwnd, window->hdc);
  DestroyWindow(window->hwnd);
  free(window);
  return(NULL);
}

void platform_terminate(void)
{
  // TODO
}
                 
int platform_get_key(PlatformWindow* window, int key)
{
  if (key < 0 ||key >= 512)
  {
    return KEY_RELEASED;
  }
  return window->keys[key] ? KEY_PRESSED : KEY_RELEASED;
}

void* platform_get_proc_address(const char* name)
{
  void* proc = (void*)wglGetProcAddress(name);
  if (proc) return(proc);

  return (void*)(GetProcAddress(GetModuleHandleA("opengl32.dll"), name));
}
                
void platform_poll_events(PlatformWindow* window)
{
  memcpy(window->keywasDown, window->keys, sizeof(window->keys));

  MSG msg;
  while (PeekMessageW(&msg, NULL, 0, 0, PM_REMOVE))
  {
    TranslateMessage(&msg);
    DispatchMessageW(&msg);
  }
}

void platform_swap_buffers(PlatformWindow* window)
{
  if (window && window->hdc)
  {
    SwapBuffers(window->hdc);
  }
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

void platform_set_user_data(PlatformWindow* w, void* data)
{
  if (w) w->userData = data;
}

bool platform_window_should_close(PlatformWindow* window)
{
  return window ? window->should_close : true;
}

void platform_window_close(PlatformWindow* window)
{
  if (window) window->should_close = true;
}
