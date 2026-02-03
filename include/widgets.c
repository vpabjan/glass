#include <X11/X.h>
#include <X11/Xlib.h>

#include <stdlib.h>
#include "types.h"

Window gWidgetsModMenuCreate(Display* dpy, Window root, i32 x, i32 y, u32 w, u32 h) {
    Window mm = XCreateSimpleWindow(dpy, root, x, y, w, h, 4, 0xFFFFFF, 0x000000);
    XSetWindowAttributes wa;
    wa.override_redirect = True;
    XChangeWindowAttributes(dpy, mm, CWOverrideRedirect, &wa);
    return mm;
}

void gWidgetsModMenuDraw(Display* dpy, Window bar, GC gc) {
    XClearWindow(dpy, bar);
    XSetForeground(dpy, gc, 0xFFFFFF);
    XDrawString(dpy, bar, gc, 2, 2, "Hello", 6);

}
