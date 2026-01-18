#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>
#include <X11/cursorfont.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include "ewmh.h"
#include "config.h"
#include "types.h"

#define MOD Mod4Mask   // Super key

int rx, ry, wx, wy;
unsigned int mask;
Window root_ret, child_ret;

typedef struct gClient {
    Window window;
    u32 height;
    u32 width;
    u32 x;
    u32 y;
    u8 workspace;
    struct gClient* next;
} gClient;

Display *dpy;

Window root;
Window focused;
Window target;

int moving = 0, resizing = 0;
int start_x, start_y;
int win_x, win_y, win_w, win_h;

gClient* clients = NULL;

Window* panel = NULL;
int showPanel = 1;

u8 currentWorkspace = 0;

gEWMH ewmh;
Window wmcheck;

Cursor cursor;

void add_client(Window w) {
    gClient *c = calloc(1, sizeof(gClient));
    c->window = w;
    c->workspace = currentWorkspace;
    c->next = clients;
    clients = c;
}

void remove_client(Window w) {
    gClient **cc = &clients;
    while (*cc) {
        if ((*cc)->window == w) {
            gClient *dead = *cc;
            *cc = dead->next;
            free(dead);
            return;
        }
        cc = &(*cc)->next;
    }
}

gClient *find_client(Window w) {
    for (gClient *c = clients; c; c = c->next)
        if (c->window == w)
            return c;
    return NULL;
}

void switch_focus(Window trg) {
    gClient *f = find_client(focused);
    gClient *c = find_client(trg);

    if (f) {
        XSetWindowBorder(dpy, focused, 0x000000);
    }

    if (c) {
        XSetWindowBorder(dpy, trg, 0xFFFFFF);
    }

    XSetInputFocus(dpy, trg, RevertToPointerRoot, CurrentTime);

    // bp1
    focused = trg;
    g_ewmh_set_active_window(dpy, root, &ewmh, focused);

}

void switch_workspace(u8 ws) {
    if (ws == currentWorkspace) return;
    Window* last = NULL;

    for (gClient *c = clients; c; c = c->next) {
        XWindowAttributes a;
        if (!XGetWindowAttributes(dpy, c->window, &a))
            continue;
        if (c->workspace == ws) {

            XMapWindow(dpy, c->window);
            last = &c->window;
            //XRaiseWindow(dpy, c->window);
        } else
            XUnmapWindow(dpy, c->window);
    }
    currentWorkspace = ws;
    g_ewmh_set_current_desktop(dpy, root, &ewmh, ws);
    if (last) {
        switch_focus(*last);
    }
}

void grab_key(KeySym sym) {
    KeyCode code = XKeysymToKeycode(dpy, sym);

    XGrabKey(dpy, code, MOD, root, True, GrabModeAsync, GrabModeAsync);
    XGrabKey(dpy, code, MOD | LockMask, root, True, GrabModeAsync, GrabModeAsync);
    XGrabKey(dpy, code, MOD | Mod2Mask, root, True, GrabModeAsync, GrabModeAsync);
    XGrabKey(dpy, code, MOD | LockMask | Mod2Mask, root, True, GrabModeAsync, GrabModeAsync);
}

void spawn(const char *cmd) {
    if (fork() == 0) {
        if (dpy) close(ConnectionNumber(dpy));
        setsid();
        execl("/bin/sh", "sh", "-c", cmd, NULL);
        exit(0);
    }
}

#define LOGTYPE_INIT 0
#define LOGTYPE_WINDOW 1
#define LOGTYPE_KEY 2
#define LOGTYPE_ERR 3
#define LOGTYPE_WORKSPACE 4
#define LOGTYPE_RUNTIME 5

void glog(const char *msg, u8 type) {
    char path[256];
    snprintf(path, sizeof(path), "%s/.glass/log", getenv("HOME"));
    FILE *f = fopen(path, "a");
    if (!f) return;

    switch (type) {
        case LOGTYPE_INIT:
            fprintf(f, "[init] ");
            break;
        case LOGTYPE_WINDOW:
            fprintf(f, "[window] ");
            break;
        case LOGTYPE_KEY:
            fprintf(f, "[key] ");
            break;
        case LOGTYPE_ERR:
            fprintf(f, "[error] ");
            break;
        case LOGTYPE_WORKSPACE:
            fprintf(f, "[workspace] ");
            break;
        case LOGTYPE_RUNTIME:
            fprintf(f, "[runtime] ");
            break;
    }

    fprintf(f, "%s\n", msg);
    fclose(f);
}

int main() {
    XEvent ev;
    gConfig* conf;
    glog("Starting Glass...", LOGTYPE_INIT);

    glog("Loading config...", LOGTYPE_INIT);
    conf = read_config();
    if (!conf) {
        glog("Cannot read config.", LOGTYPE_ERR);
    }


    glog("Opening display", LOGTYPE_INIT);
    dpy = XOpenDisplay(NULL);
    if (!dpy) {
        fprintf(stderr, "cannot open display\n");
        glog("Cannot open display! This error is fatal, exiting.", LOGTYPE_ERR);
        return 1;
    }

    root = DefaultRootWindow(dpy);

    cursor = XCreateFontCursor(dpy, XC_left_ptr);

    XDefineCursor(dpy, root, cursor);

    glog("Initializing ewmh...", LOGTYPE_INIT);

    g_ewmh_init(dpy, &ewmh);

    wmcheck = XCreateSimpleWindow(dpy, root, 0, 0, 1, 1, 0, 0, 0);

    g_ewmh_set_check(dpy, root, wmcheck, &ewmh);
    g_ewmh_set_wm_name(dpy, wmcheck, &ewmh, "Glass");
    g_ewmh_set_supported(dpy, root, &ewmh);
    g_ewmh_set_desktop_count(dpy, root, &ewmh, 9);
    g_ewmh_set_current_desktop(dpy, root, &ewmh, currentWorkspace);

    glog("Grabbing keys and buttons...", LOGTYPE_INIT);

    XGrabButton(dpy, Button1, MOD, root, True,
                ButtonPressMask | ButtonReleaseMask | PointerMotionMask,
                GrabModeAsync, GrabModeAsync, None, None);

    XGrabButton(dpy, Button3, MOD, root, True,
                ButtonPressMask | ButtonReleaseMask | PointerMotionMask,
                GrabModeAsync, GrabModeAsync, None, None);

    XGrabButton(dpy, Button4, MOD, root, True,
                ButtonPressMask | ButtonReleaseMask | PointerMotionMask,
                GrabModeAsync, GrabModeAsync, None, None);

    XGrabButton(dpy, Button5, MOD, root, True,
                ButtonPressMask | ButtonReleaseMask | PointerMotionMask,
                GrabModeAsync, GrabModeAsync, None, None);

    gBind* a = conf->bindhead;

    while (a) {
        grab_key(a->bind);
        if (a->type >= 0 && a->type <= 8) {
            grab_key(a->bind & ShiftMask);
        }
        a=a->next;
    }

    XSelectInput(dpy, root, SubstructureRedirectMask | SubstructureNotifyMask | KeyPressMask | ButtonPressMask);


    XQueryPointer(dpy, root,
                  &root_ret, &child_ret,
                  &rx, &ry, &wx, &wy,
                  &mask);

    glog("Running rc.sh...", LOGTYPE_INIT);
    spawn("bash ~/.glass/rc.sh");

    glog("Done!", LOGTYPE_INIT);


    for (;;) {
        XNextEvent(dpy, &ev);

        switch (ev.type) {

        case LeaveNotify: {
            break;
        }

        case EnterNotify: {

            if (ev.xcrossing.window == root) break;
            if (ev.xcrossing.mode != NotifyNormal || ev.xcrossing.detail == NotifyInferior) break;

            gClient* c = find_client(ev.xcrossing.window);
            if (!c) break;

            if (moving || resizing) break;

            switch_focus(c->window);
            break;
        }

        case MapRequest: {
            Window w = ev.xmaprequest.window;

            if (find_client(w)) break;

            XWindowAttributes attrib;
            XGetWindowAttributes(dpy, w, &attrib);
            if (attrib.override_redirect) {
                glog("Panel detected! (override_redirect)", LOGTYPE_WINDOW);
                if (!panel) {
                    panel = &w;
                }
                break;
            }
            XSetWindowBorderWidth(dpy, w, 4);
            XSetWindowBorder(dpy, w, BlackPixel(dpy, DefaultScreen(dpy)));
            XSelectInput(dpy, w, EnterWindowMask | FocusChangeMask | PropertyChangeMask | StructureNotifyMask);

            add_client(w);

            gClient *c = find_client(w);
            if (c && c->workspace == currentWorkspace) {
                XMapWindow(dpy, c->window);
                XRaiseWindow(dpy, c->window);
                switch_focus(c->window);
                glog("New client, hello!", LOGTYPE_WINDOW);
                g_ewmh_set_window_type(dpy, c->window, &ewmh);
                g_ewmh_set_motif_hints(dpy, c->window, 1, &ewmh);
                g_ewmh_set_gtk_frame_extents(dpy, c->window, 1, 1, 23, 1, &ewmh);

            }
            break;
        }

        case DestroyNotify:
            remove_client(ev.xdestroywindow.window);
            glog("Client destroyed.", LOGTYPE_WINDOW);
            break;

        case UnmapNotify:
            //remove_client(ev.xunmap.window);
            break;


        case ConfigureRequest:
            XWindowChanges changes;
            changes.x = ev.xconfigurerequest.x;
            changes.y = ev.xconfigurerequest.y;
            changes.width = ev.xconfigurerequest.width;
            changes.height = ev.xconfigurerequest.height;
            changes.border_width = ev.xconfigurerequest.border_width;
            changes.sibling = ev.xconfigurerequest.above;
            changes.stack_mode = ev.xconfigurerequest.detail;

            XConfigureWindow(dpy, ev.xconfigurerequest.window, ev.xconfigurerequest.value_mask, &changes);
            break;

        case ButtonPress:
            if (ev.xbutton.state & MOD && (ev.xbutton.button == Button4 || ev.xbutton.button == Button5)) {

                gClient* temp = find_client(focused);


                if (ev.xbutton.button == Button4) {
                    if (currentWorkspace < 8) {
                        switch_workspace(currentWorkspace + 1);
                    } else {
                        switch_workspace(0);
                    }
                } else {
                    if (currentWorkspace > 0) {
                        switch_workspace(currentWorkspace - 1);
                    } else {
                        switch_workspace(8);
                    }
                }
                break;
            }

            target = ev.xbutton.subwindow;

            if (!target) break;

            if (target == root) break;

            switch_focus(target);
            XRaiseWindow(dpy, focused);

            XWindowAttributes attr;
            XGetWindowAttributes(dpy, target, &attr);

            if (ev.xbutton.state & MOD && (ev.xbutton.button == Button1 || ev.xbutton.button == Button3)) {
                win_x = attr.x;
                win_y = attr.y;
                win_w = attr.width;
                win_h = attr.height;

                start_x = ev.xbutton.x_root;
                start_y = ev.xbutton.y_root;

                if (ev.xbutton.button == Button1)
                    moving = 1;
                else if (ev.xbutton.button == Button3)
                    resizing = 1;
            }

            break;

        case MotionNotify:
            if (!target) break;

            int dx = ev.xmotion.x_root - start_x;
            int dy = ev.xmotion.y_root - start_y;

            if (moving) {
                XMoveWindow(dpy, target, win_x + dx, win_y + dy);
            } else if (resizing) {
                XResizeWindow(dpy, target,
                              (win_w + dx > 50) ? win_w + dx : 50,
                              (win_h + dy > 50) ? win_h + dy : 50);
            }
            break;

        case ButtonRelease:
            if (moving || resizing && (ev.xbutton.button == Button1 || ev.xbutton.button == Button3)) {
                moving = resizing = 0;
                target = None;
            }

            break;

        case KeyPress:
            KeySym sym = XKeycodeToKeysym(dpy, ev.xkey.keycode, 0);
            //KeySym sym = XLookupKeysym(&ev.xkey,
            //    (ev.xkey.state & ShiftMask) ? 1 : 0);
            if (!(ev.xkey.state & MOD))
                break;

            gBind* bind = conf->bindhead;
            while (bind && bind->bind != sym)
                bind = bind->next;

            if (!bind)
                break;


            switch(bind->type) {
                case (BQUIT):
                    gClient* c = NULL;
                    c = find_client(focused);
                    if (c) {
                        XKillClient(dpy, focused);
                        remove_client(focused);
                        glog("Killing client on user request", LOGTYPE_WINDOW);
                        //XSetInputFocus(dpy, Window, int, Time)
                    }
                    break;
                case (BSPAWN):
                    spawn((char*)bind->data);
                    break;
                case (BPANEL): {
                    if (!panel) break;
                    if (showPanel) {
                        XUnmapWindow(dpy, *panel);
                    } else {
                        XMapWindow(dpy, *panel);
                    }
                    showPanel = !showPanel;
                    break;
                }
                case (BEXIT):
                    XCloseDisplay(dpy);
                    glog("User used exit key. Shutting down.", LOGTYPE_RUNTIME);
                    while (conf->bindhead != NULL) conf->bindhead = conf->bindhead->next;
                    for (gClient* c = clients; c; c = c->next) {
                        free(c);
                    }
                    exit(0);
                    break;
                }

            if (bind->type >= 0 && bind->type <= 8) {
                if (!(ev.xkey.state & ShiftMask)) {
                    switch_workspace(bind->type);
                } else {
                    if (focused && focused != root) {
                        gClient* hi = find_client(focused);
                        if (hi && (currentWorkspace != bind->type)) {
                            hi->workspace = bind->type;
                            XUnmapWindow(dpy, focused);
                        }
                    }
                }
            }
            break;
        }
    }
}
