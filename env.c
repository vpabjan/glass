#include <stdlib.h>
#include <unistd.h>
#include "types.h"

void init_env(void) {
    setenv("XDG_CRRENT_DESKTOP", "Glass", 1);
    setenv("XDG_SESSION_DESKTOP", "Glass", 1);
    setenv("XDG_SESSION_TYPE", "x11", 0);
    setenv("GLASS_WM", "1", 1);
}
