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

    /* added for decorator support */
    Atom window_type;
    Atom window_type_normal;
    Atom motif_hints;
    Atom gtk_frame_extents;

    /* optional: for maximized/fullscreen support */
    Atom wm_state;
    Atom max_vert;
    Atom max_horz;
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

    /* decorator support */
    e->window_type         = XInternAtom(dpy, "_NET_WM_WINDOW_TYPE", 0);
    e->window_type_normal  = XInternAtom(dpy, "_NET_WM_WINDOW_TYPE_NORMAL", 0);
    e->motif_hints         = XInternAtom(dpy, "_MOTIF_WM_HINTS", 0);
    e->gtk_frame_extents   = XInternAtom(dpy, "_GTK_FRAME_EXTENTS", 0);

    /* optional maximized/fullscreen */
    e->wm_state            = XInternAtom(dpy, "_NET_WM_STATE", 0);
    e->max_vert            = XInternAtom(dpy, "_NET_WM_STATE_MAXIMIZED_VERT", 0);
    e->max_horz            = XInternAtom(dpy, "_NET_WM_STATE_MAXIMIZED_HORZ", 0);
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

typedef struct {
    unsigned long flags;
    unsigned long functions;
    unsigned long decorations;
    long inputMode;
    unsigned long status;
} MotifWmHints;

static inline void g_ewmh_set_window_type(Display *dpy, Window w, gEWMH *e) {
    XChangeProperty(dpy, w, e->window_type, XA_ATOM, 32,
                    PropModeReplace, (unsigned char *)&e->window_type_normal, 1);
}

static inline void g_ewmh_set_motif_hints(Display *dpy, Window w, int decorate, gEWMH *e) {
    MotifWmHints hints = {2, 0, 0, 0, 0};
    if (decorate) {
        MotifWmHints hints = {2, 0, 1, 0, 0};
    }


    XChangeProperty(dpy, w, e->motif_hints, e->motif_hints, 32,
                    PropModeReplace, (unsigned char *)&hints, sizeof(MotifWmHints)/4);
}

static inline void g_ewmh_set_gtk_frame_extents(Display *dpy, Window w,
                                                int left, int right, int top, int bottom,
                                                gEWMH *e) {
    long extents[4] = {left, right, top, bottom};
    XChangeProperty(dpy, w, e->gtk_frame_extents, XA_CARDINAL, 32,
                    PropModeReplace, (unsigned char *)extents, 4);
}

#endif
