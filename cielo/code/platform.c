#if defined(_WIN32)

#include "platform_win32.c"

#elif defined(__linux__)

#include "platform_x11.c"

#elif defined (__APPLE__)

#include "platform_mac.c"

#else
#error "Platform not implemented"
#endif
