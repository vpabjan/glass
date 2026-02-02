#include <stdlib.h>
#include <unistd.h>
#include "types.h"

static inline void init_env(void) {
    setenv("XDG_CURRENT_DESKTOP", "Glass", 1);
    //setenv("XDG_RUNTIME_DIR", "$HOME", 0);
    setenv("XDG_SESSION_DESKTOP", "GlassWM", 1);
    setenv("XDG_SESSION_TYPE", "x11", 0);
    setenv("DESKTOP_SESSION", "glass", 1);
    setenv("GLASS_WM", "1", 1);
}
