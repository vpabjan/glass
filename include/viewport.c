#include <X11/Xlib.h>
#include "types.h"


typedef struct gViewport {
    u8 mode; // 0 float
    Window last_focused;
} gViewport;
