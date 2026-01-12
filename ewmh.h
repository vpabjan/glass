#ifndef GLASS_EWMH_H
#define GLASS_EWMH_H

#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <string.h>
#include "types.h"

typedef struct {
    Atom supported;
    Atom current_desktop;
    Atom number_of_desktops;
    Atom desktop_names;
    Atom active_window;
    Atom wm_check;
    Atom wm_name;
    Atom utf8;
} gEWMH;

static inline void g_ewmh_init(Display *dpy, gEWMH *e) {
    e->supported           = XInternAtom(dpy, "_NET_SUPPORTED", 0);
    e->current_desktop     = XInternAtom(dpy, "_NET_CURRENT_DESKTOP", 0);
    e->number_of_desktops  = XInternAtom(dpy, "_NET_NUMBER_OF_DESKTOPS", 0);
    e->desktop_names       = XInternAtom(dpy, "_NET_DESKTOP_NAMES", 0);
    e->active_window       = XInternAtom(dpy, "_NET_ACTIVE_WINDOW", 0);
    e->wm_check            = XInternAtom(dpy, "_NET_SUPPORTING_WM_CHECK", 0);
    e->wm_name             = XInternAtom(dpy, "_NET_WM_NAME", 0);
    e->utf8                = XInternAtom(dpy, "UTF8_STRING", 0);
}

static inline void g_ewmh_set_check(Display *dpy, Window root, Window wm, gEWMH *e) {
    XChangeProperty(dpy, root, e->wm_check, XA_WINDOW, 32,
                    PropModeReplace, (unsigned char *)&wm, 1);
    XChangeProperty(dpy, wm, e->wm_check, XA_WINDOW, 32,
                    PropModeReplace, (unsigned char *)&wm, 1);
}

static inline void g_ewmh_set_supported(Display *dpy, Window root, gEWMH *e) {
    Atom list[] = {
        e->current_desktop,
        e->number_of_desktops,
        e->desktop_names,
        e->active_window
    };
    XChangeProperty(dpy, root, e->supported, XA_ATOM, 32,
                    PropModeReplace, (unsigned char *)list,
                    sizeof(list) / sizeof(Atom));
}

static inline void g_ewmh_set_wm_name(Display *dpy, Window wm, gEWMH *e, const char *name) {
    XChangeProperty(dpy, wm, e->wm_name, e->utf8, 8,
                    PropModeReplace, (unsigned char *)name, strlen(name));
}

static inline void g_ewmh_set_desktop_count(Display *dpy, Window root, gEWMH *e, u32 n) {
    XChangeProperty(dpy, root, e->number_of_desktops, XA_CARDINAL, 32,
                    PropModeReplace, (unsigned char *)&n, 1);
}

static inline void g_ewmh_set_current_desktop(Display *dpy, Window root, gEWMH *e, u32 d) {
    XChangeProperty(dpy, root, e->current_desktop, XA_CARDINAL, 32,
                    PropModeReplace, (unsigned char *)&d, 1);
}

static inline void g_ewmh_set_desktop_names(Display *dpy, Window root, gEWMH *e,
                                            const char *names, u32 len) {
    XChangeProperty(dpy, root, e->desktop_names, e->utf8, 8,
                    PropModeReplace, (unsigned char *)names, len);
}

static inline void g_ewmh_set_active_window(Display *dpy, Window root, gEWMH *e, Window w) {
    XChangeProperty(dpy, root, e->active_window, XA_WINDOW, 32,
                    PropModeReplace, (unsigned char *)&w, 1);
}

#endif
