#include <X11/Xlib.h>
#include "types.h"


const u8 VP_MODE_FLOAT = 0;
const u8 VP_MODE_TILE = 1;

typedef struct gViewport {
    u8 mode; // 0 float 1 tile
    Window last_focused;
} gViewport;
