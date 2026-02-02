#include <X11/Xlib.h>
#include "types.h"


typedef struct gViewport {
    u8 mode; // 0 float or tile
    Window last_focused;
} gViewport;
