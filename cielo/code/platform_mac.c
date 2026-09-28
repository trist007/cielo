#include "platform.h"

s/*
 * platform_macos.m
 *
 * macOS backend for the platform layer, mirroring platform_linux.c's API
 * (Cocoa + NSOpenGL instead of XCB + EGL). Targets OpenGL 3.3 core;
 * requesting NSOpenGLProfileVersion3_2Core gives you the highest core
 * profile available (4.1 on Apple Silicon / M4), which is a strict
 * superset of 3.3 core.
 *
 * Build: compile as Objective-C, link -framework Cocoa -framework OpenGL
 *   clang -ObjC -c platform_macos.m -o platform_macos.o
 *   clang ... -framework Cocoa -framework OpenGL -o app
 *
 * -----------------------------------------------------------------------
 * REQUIRED ADDITION TO platform.h (merge into your existing struct, or
 * add as an #ifdef __APPLE__ branch alongside your #ifdef __linux__ one):
 *
 * #ifdef __APPLE__
 * typedef struct PlatformWindow
 * {
 *   int  width;
 *   int  height;
 *   bool should_close;
 *
 *   bool keys[512];
 *   bool keywasDown[512];
 *   int  mouseX, mouseY;
 *   bool mouseButtons[3];
 *
 *   void (*framebuffer_size_callback)(struct PlatformWindow* window, int width, int height);
 *   void* userData;
 *
 *   // macOS-specific opaque handles (kept as void* so this header stays
 *   // includable from plain C translation units)
 *   void* ns_window;    // NSWindow*
 *   void* ns_view;      // PlatformView*
 *   void* ns_delegate;  // PlatformWindowDelegate*
 *   void* gl_context;   // NSOpenGLContext*
 * } PlatformWindow;
 * #endif
 * -----------------------------------------------------------------------
 */

#import <Cocoa/Cocoa.h>
#import <OpenGL/gl3.h>
#include "platform.h"
#include <dlfcn.h>

// ---------------------------------------------------------------------
// Key translation (macOS virtual keycodes -> your KEY_* enum)
// Only ESC is wired up below, same starting point as the Linux file.
// Extend this table as you add more KEY_* values to platform.h.
// ---------------------------------------------------------------------
static int platform_translate_keycode(unsigned short keycode)
{
  switch (keycode)
  {
    case 0x35: return(KEY_ESC); // kVK_Escape
    default:   return(KEY_NONE);
  }
}

// ---------------------------------------------------------------------
// NSView subclass: owns the GL context, forwards input to PlatformWindow
// ---------------------------------------------------------------------
@interface PlatformView : NSOpenGLView
{
@public
  PlatformWindow* pw;
}
@end

@implementation PlatformView

- (BOOL)acceptsFirstResponder
{
  return(YES);
}

- (void)keyDown:(NSEvent*)event
{
  int key = platform_translate_keycode([event keyCode]);
  if (pw && key != KEY_NONE)
  {
    pw->keys[key] = true;
  }
}

- (void)keyUp:(NSEvent*)event
{
  int key = platform_translate_keycode([event keyCode]);
  if (pw && key != KEY_NONE)
  {
    pw->keys[key] = false;
  }
}

- (void)mouseDown:(NSEvent*)event
{
  if (pw) pw->mouseButtons[0] = true;
}

- (void)mouseUp:(NSEvent*)event
{
  if (pw) pw->mouseButtons[0] = false;
}

- (void)rightMouseDown:(NSEvent*)event
{
  if (pw) pw->mouseButtons[1] = true;
}

- (void)rightMouseUp:(NSEvent*)event
{
  if (pw) pw->mouseButtons[1] = false;
}

- (void)otherMouseDown:(NSEvent*)event
{
  if (pw && [event buttonNumber] == 2)
  {
    pw->mouseButtons[2] = true;
  }
}

- (void)otherMouseUp:(NSEvent*)event
{
  if (pw && [event buttonNumber] == 2)
  {
    pw->mouseButtons[2] = false;
  }
}

- (void)mouseMoved:(NSEvent*)event
{
  if (!pw) return;
  NSPoint p = [self convertPoint:[event locationInWindow] fromView:nil];
  pw->mouseX = (int)p.x;
  pw->mouseY = (int)(self.bounds.size.height - p.y); // flip to top-left origin
}

- (void)mouseDragged:(NSEvent*)event
{
  [self mouseMoved:event];
}

- (void)rightMouseDragged:(NSEvent*)event
{
  [self mouseMoved:event];
}

// Called on resize / display-scale change (e.g. dragging between a
// non-Retina external display and the M4's built-in Retina panel)
- (void)reshape
{
  [super reshape];
  if (!pw) return;

  NSRect backing = [self convertRectToBacking:[self bounds]];
  int w = (int)backing.size.width;
  int h = (int)backing.size.height;

  if (w != pw->width || h != pw->height)
  {
    pw->width  = w;
    pw->height = h;
    if (pw->framebuffer_size_callback)
    {
      pw->framebuffer_size_callback(pw, w, h);
    }
  }
}

@end

// ---------------------------------------------------------------------
// Window delegate: routes the close button into should_close instead
// of letting Cocoa tear the window down itself
// ---------------------------------------------------------------------
@interface PlatformWindowDelegate : NSObject<NSWindowDelegate>
{
@public
  PlatformWindow* pw;
}
@end

@implementation PlatformWindowDelegate

- (BOOL)windowShouldClose:(NSWindow*)sender
{
  if (pw) pw->should_close = true;
  return(NO); // we own teardown via platform_terminate
}

@end

// ---------------------------------------------------------------------
// Public API
// ---------------------------------------------------------------------

int platform_init(void)
{
  [NSApplication sharedApplication];
  [NSApp setActivationPolicy:NSApplicationActivationPolicyRegular];
  [NSApp finishLaunching];
  return(1);
}

PlatformWindow* platform_create_window(int width, int height, const char* title)
{
  PlatformWindow* window = (PlatformWindow*)malloc(sizeof(PlatformWindow));
  if (!window) return(NULL);
  memset(window, 0, sizeof(PlatformWindow));

  window->width        = width;
  window->height       = height;
  window->should_close = false;

  NSOpenGLPixelFormatAttribute attrs[] =
  {
    NSOpenGLPFAOpenGLProfile, NSOpenGLProfileVersion3_2Core, // highest core profile (>=3.3)
    NSOpenGLPFAColorSize,     32,
    NSOpenGLPFAAlphaSize,     8,
    NSOpenGLPFADepthSize,     24,
    NSOpenGLPFAStencilSize,   8,
    NSOpenGLPFADoubleBuffer,
    NSOpenGLPFAAccelerated,
    0
  };

  NSOpenGLPixelFormat* pixel_format = [[NSOpenGLPixelFormat alloc] initWithAttributes:attrs];
  if (!pixel_format)
  {
    fprintf(stderr, "Failed to create NSOpenGLPixelFormat\n");
    free(window);
    return(NULL);
  }

  NSRect frame = NSMakeRect(0, 0, width, height);
  NSUInteger style = NSWindowStyleMaskTitled |
                      NSWindowStyleMaskClosable |
                      NSWindowStyleMaskMiniaturizable |
                      NSWindowStyleMaskResizable;

  NSWindow* ns_window = [[NSWindow alloc] initWithContentRect:frame
                                                     styleMask:style
                                                       backing:NSBackingStoreBuffered
                                                         defer:NO];
  if (!ns_window)
  {
    fprintf(stderr, "Failed to create NSWindow\n");
    [pixel_format release];
    free(window);
    return(NULL);
  }

  [ns_window setTitle:[NSString stringWithUTF8String:title]];
  [ns_window center];

  PlatformView* view = [[PlatformView alloc] initWithFrame:frame pixelFormat:pixel_format];
  view->pw = window;
  [view setWantsBestResolutionOpenGLSurface:YES]; // opt in to Retina backing on the M4's display

  [ns_window setContentView:view];
  [ns_window makeFirstResponder:view];

  PlatformWindowDelegate* delegate = [[PlatformWindowDelegate alloc] init];
  delegate->pw = window;
  [ns_window setDelegate:delegate];

  [[view openGLContext] makeCurrentContext];

  GLint swap_interval = 1; // vsync on, matches typical eglSwapInterval(1) usage
  [[view openGLContext] setValues:&swap_interval forParameter:NSOpenGLContextParameterSwapInterval];

  [ns_window makeKeyAndOrderFront:nil];
  [NSApp activateIgnoringOtherApps:YES];

  // Sync width/height to the actual backing-store size (Retina != point size)
  NSRect backing = [view convertRectToBacking:[view bounds]];
  window->width  = (int)backing.size.width;
  window->height = (int)backing.size.height;

  window->ns_window   = ns_window;
  window->ns_view      = view;
  window->ns_delegate  = delegate;
  window->gl_context   = [view openGLContext];

  [pixel_format release];

  printf("NSOpenGL initialized: %s\n", (const char*)glGetString(GL_VERSION));
  return(window);
}

void platform_terminate(void)
{
  // Per-window teardown is intentionally left to the caller destroying
  // the PlatformWindow (see note below) — this mirrors the Linux
  // TODO, which also leaves this as a stub for global state only.
}

void platform_destroy_window(PlatformWindow* window)
{
  if (!window) return;

  NSWindow* ns_window = (NSWindow*)window->ns_window;
  PlatformView* view = (PlatformView*)window->ns_view;
  PlatformWindowDelegate* delegate = (PlatformWindowDelegate*)window->ns_delegate;

  [ns_window setDelegate:nil];
  [ns_window setContentView:nil];
  [ns_window close];

  [delegate release];
  [view release];
  [ns_window release];

  free(window);
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
  [(NSOpenGLContext*)window->gl_context flushBuffer];
}

void* platform_get_proc_address(const char* name)
{
  static void* gl_bundle = NULL;
  if (!gl_bundle)
  {
    gl_bundle = dlopen("/System/Library/Frameworks/OpenGL.framework/OpenGL", RTLD_LAZY);
  }
  return(gl_bundle ? dlsym(gl_bundle, name) : NULL);
}

void platform_poll_events(PlatformWindow* window)
{
  // Snapshot "previous frame" key state for just_pressed/just_released
  memcpy(window->keywasDown, window->keys, sizeof(window->keys));

  NSEvent* event;
  while ((event = [NSApp nextEventMatchingMask:NSEventMaskAny
                                      untilDate:[NSDate distantPast]
                                         inMode:NSDefaultRunLoopMode
                                        dequeue:YES]))
  {
    [NSApp sendEvent:event];
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
  return(w->keys[key]);
}

bool platform_key_just_pressed(PlatformWindow* w, int key)
{
  return(w->keys[key] && !w->keywasDown[key]);
}

bool platform_key_just_released(PlatformWindow* w, int key)
{
  return(!w->keys[key] && w->keywasDown[key]);
}

void platform_set_user_data(PlatformWindow* w, void* data)
{
  if (w) w->userData = data;
}
