#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include "ewmh.h"
#include "types.h"

#define MOD Mod4Mask   // Super key
#define KEY_CLOSE XK_q
#define KEY_EXIT XK_e
#define KEY_FULLSCREEN XK_f
#define KEY_TERMINAL XK_Return
#define KEY_DRUN XK_d

#define CMD_TERM "alacritty"



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
int moving = 0, resizing = 0;
int start_x, start_y;
int win_x, win_y, win_w, win_h;
Window target;
gClient *clients = NULL;
u8 currentWorkspace = 0;
gEWMH ewmh;
Window wmcheck;


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

void switch_workspace(u8 ws) {
    if (ws == currentWorkspace) return;

    for (gClient *c = clients; c; c = c->next) {
        if (c->workspace == ws) {
            XMapWindow(dpy, c->window);
            XRaiseWindow(dpy, c->window);
        } else
            XUnmapWindow(dpy, c->window);
    }
    currentWorkspace = ws;
    g_ewmh_set_current_desktop(dpy, root, &ewmh, ws);
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
    }

    fprintf(f, "%s\n", msg);
    fclose(f);
}

int main() {
    XEvent ev;
    system("mkdir -p ~/.glass");
    system("touch ~/.glass/log");
    system("touch ~/.glass/rc.sh");
    system("chmod +x ~/.glass/rc.sh");
    glog("Starting Glass...", LOGTYPE_INIT);


    dpy = XOpenDisplay(NULL);
    if (!dpy) {
        fprintf(stderr, "cannot open display\n");
        glog("Cannot open display! This error is fatal, exiting.", LOGTYPE_ERR);
        return 1;
    }

    root = DefaultRootWindow(dpy);

    g_ewmh_init(dpy, &ewmh);

    wmcheck = XCreateSimpleWindow(dpy, root, 0, 0, 1, 1, 0, 0, 0);

    g_ewmh_set_check(dpy, root, wmcheck, &ewmh);
    g_ewmh_set_wm_name(dpy, wmcheck, &ewmh, "Glass");
    g_ewmh_set_supported(dpy, root, &ewmh);
    g_ewmh_set_desktop_count(dpy, root, &ewmh, 9);
    g_ewmh_set_current_desktop(dpy, root, &ewmh, currentWorkspace);

    XGrabButton(dpy, Button1, MOD, root, True,
                ButtonPressMask | ButtonReleaseMask | PointerMotionMask,
                GrabModeAsync, GrabModeAsync, None, None);

    XGrabButton(dpy, Button3, MOD, root, True,
                ButtonPressMask | ButtonReleaseMask | PointerMotionMask,
                GrabModeAsync, GrabModeAsync, None, None);

    grab_key(KEY_DRUN);
    grab_key(KEY_EXIT);
    grab_key(KEY_CLOSE);
    grab_key(KEY_FULLSCREEN);
    grab_key(KEY_TERMINAL);


    //XGrabButton(dpy, Button1, 0, root, True,
    //            ButtonPressMask,
    //            GrabModeAsync, GrabModeAsync, None, None);


    /*
    XGrabKey(dpy, XKeysymToKeycode(dpy, KEY_DRUN), MOD, root,
             True, GrabModeAsync, GrabModeAsync);

    XGrabKey(dpy, XKeysymToKeycode(dpy, KEY_EXIT), MOD, root,
            True, GrabModeAsync, GrabModeAsync);

    XGrabKey(dpy, XKeysymToKeycode(dpy, KEY_CLOSE), MOD, root,
            True, GrabModeAsync, GrabModeAsync);

    XGrabKey(dpy, XKeysymToKeycode(dpy, KEY_FULLSCREEN), MOD, root,
            True, GrabModeAsync, GrabModeAsync);
    */
    XSelectInput(dpy, root, SubstructureRedirectMask | SubstructureNotifyMask | KeyPressMask);

    for (int i = 0; i < 9; i++) {
        grab_key(XK_1 + i);
        XGrabKey(dpy, XKeysymToKeycode(dpy, XK_1 + i), MOD | ShiftMask, root, True, GrabModeAsync, GrabModeAsync);
    }

    glog("Running rc.sh", LOGTYPE_INIT);
    spawn("bash ~/.glass/rc.sh");


    for (;;) {
        XNextEvent(dpy, &ev);

        switch (ev.type) {

        case MapRequest: {
            Window w = ev.xmaprequest.window;

            XWindowAttributes attrib;
            XGetWindowAttributes(dpy, w, &attrib);
            if (attrib.override_redirect) {
                // this is probably a panel or menu; don't track for workspaces
                break;
            }
            XSetWindowBorderWidth(dpy, w, 4);
            XSetWindowBorder(dpy, w, BlackPixel(dpy, DefaultScreen(dpy)));

            add_client(w);

            gClient *c = find_client(w);
            if (c->workspace == currentWorkspace) {
                XMapWindow(dpy, w);
                XRaiseWindow(dpy, w);
            } else {
                XUnmapWindow(dpy, w);
            }

            break;
        }

        case DestroyNotify:
            remove_client(ev.xdestroywindow.window);
            break;

        case UnmapNotify:
            //remove_client(ev.xunmap.window);
            break;


        case ButtonPress:
            target = ev.xbutton.subwindow;
            if (!target) break;

            XRaiseWindow(dpy, target);
            XSetInputFocus(dpy, target, RevertToPointerRoot, CurrentTime);

            g_ewmh_set_active_window(dpy, root, &ewmh, target);


            XWindowAttributes attr;
            XGetWindowAttributes(dpy, target, &attr);

            if (ev.xbutton.state & MOD) {
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
            moving = resizing = 0;
            target = None;
            break;

        case KeyPress:
            KeySym sym = XKeycodeToKeysym(dpy, ev.xkey.keycode, 0);
            Window focused;
            int revert;
            XGetInputFocus(dpy, &focused, &revert);


            if ((ev.xkey.state & MOD)) { // this needs to detect mod
                switch (sym) {
                    case KEY_CLOSE:
                        if(focused && focused != root && focused != None) {
                            XKillClient(dpy, focused);
                        }
                        break;
                    case KEY_DRUN:
                        spawn("rofi -show drun");
                        break;
                    case KEY_FULLSCREEN:
                        XMoveWindow(dpy, focused, 0, 0);
                        XResizeWindow(dpy, focused, XDisplayWidth(dpy, XDefaultScreen(dpy)), XDisplayHeight(dpy, XDefaultScreen(dpy)));
                        break;
                    case KEY_EXIT:
                        XCloseDisplay(dpy);
                        exit(0);
                        break;
                    case KEY_TERMINAL:
                        spawn(CMD_TERM);
                        break;
                }
                if (sym >= XK_1 && sym <= XK_9) {

                    if (ev.xkey.state & ShiftMask) {
                        if (focused && focused != root && focused != None) {
                            gClient* hi = find_client(focused);
                            if (!(sym - XK_1 == currentWorkspace)) {
                                hi->workspace = sym - XK_1;
                                XUnmapWindow(dpy, focused);
                            }
                        }
                    } else {
                        switch_workspace(sym - XK_1);
                    }

                }

            }
            break;
        }
    }
}
