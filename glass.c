#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>
#include <X11/cursorfont.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include "include/ewmh.h"
#include "include/config.c"
#include "include/types.h"
#include "include/env.c"
#include "include/log.c"

#define MOD Mod4Mask   // Super key

typedef struct gClient {
    Window window;
    u32 height;
    u32 width;
    u32 x;
    u32 y;
    u8 workspace;
    u8 fullscreen;
    int old_x, old_y, old_w, old_h;

    struct gClient* next;
} gClient;

typedef struct gWorkspace {
    u8 tile;
} gWorkspace;

int rx, ry, wx, wy;
unsigned int mask;
Window root_ret, child_ret;

Display *dpy;

gConfig* conf;

Window root;
Window focused;
Window grabbed;
Window target;

u32 res_x;
u32 res_y;


int moving = 0, resizing = 0;
int start_x, start_y;
int win_x, win_y, win_w, win_h;

gClient* clients = NULL;

Window panel = None;
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

void switch_focus(Window trg)  {
    gClient *f = find_client(focused);
    gClient *c = find_client(trg);

    if (focused && f) {
        XSetWindowBorderWidth(dpy, focused, 4);
        XSetWindowBorder(dpy, focused, 0x000000);
    }

    if (c) {
        XSetWindowBorderWidth(dpy, trg, 4);
        XSetWindowBorder(dpy, trg, 0xFFFFFF);
        XSetInputFocus(dpy, trg, RevertToPointerRoot, CurrentTime);
        XRaiseWindow(dpy, trg);
    } else {
        XSetInputFocus(dpy, root, RevertToPointerRoot, CurrentTime);
    }
    // bp1
    focused = trg;
    g_ewmh_set_active_window(dpy, root, &ewmh, focused);

}

void switch_workspace(u8 ws) {
    if (ws == currentWorkspace) return;
    Window next_focus = None;
    Window last = None;
    gClient* c = find_client(focused);
    if (moving && c) {
        c->workspace = ws;
        next_focus = focused;
    }


    for (gClient *c = clients; c; c = c->next) {
        XWindowAttributes attributes;
        u8 h = XGetWindowAttributes(dpy, c->window, &attributes);
        if (c->workspace == ws) {
            if (h) {
                if (!attributes.map_state) {
                    XMapWindow(dpy, c->window);
                    if (next_focus == None) next_focus = c->window;
                    last = c->window;
                }
            }
        } else {
            if (h) {
                if (attributes.map_state) XUnmapWindow(dpy, c->window);
            } else {
                 XUnmapWindow(dpy, c->window); //unmap anyway
            }

        }
    }


    currentWorkspace = ws;
    g_ewmh_set_current_desktop(dpy, root, &ewmh, ws);


    if (next_focus != None) {
        //XRaiseWindow(dpy, next_focus);
        switch_focus(next_focus);
        XRaiseWindow(dpy, focused);

        if (conf->warpPointer && !moving) {
            XWindowAttributes a;
            if (XGetWindowAttributes(dpy, next_focus, &a) )XWarpPointer(dpy, None, next_focus, 0, 0, 0, 0, a.width / 2, a.height / 2);
        }
    } else {
        XSetInputFocus(dpy, root, RevertToPointerRoot, CurrentTime);
        focused = root;
        g_ewmh_set_active_window(dpy, root, &ewmh, None);
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

/*
void toggle_fullscreen(Window w) {
    gClient *c = find_client(w);
    if (!c) return;

    if (!c->fullscreen) {
        XWindowAttributes attr;
        XGetWindowAttributes(dpy, w, &attr);
        c->old_x = attr.x; c->old_y = attr.y;
        c->old_w = attr.width; c->old_h = attr.height;
        XSetWindowBorder(dpy, Window, unsigned long)
    }
}
*/

void cycle_windows() {
    if (!clients) return;

    gClient *active = find_client(focused);

    if (active && active->next) {
        if (clients == active) {
            clients = active->next;
        } else {
            gClient *prev = clients;
            while (prev->next != active) prev = prev->next;
            prev->next = active->next;
        }
        gClient *tail = clients;
        while (tail->next) tail = tail->next;
        tail->next = active;
        active->next = NULL;
    }


    gClient *c = clients;
    while (c) {
        if (c->workspace == currentWorkspace) {
            XRaiseWindow(dpy, c->window);
            switch_focus(c->window);

            if (conf->warpPointer) {
                XWindowAttributes a;
                if (XGetWindowAttributes(dpy, c->window, &a)) XWarpPointer(dpy, None, c->window, 0, 0, 0, 0, a.width / 2, a.height / 2);
            }
            return;
        }
        c = c->next;
    }
}

void toggle_fullscreen(Window w) {
    gClient *c = find_client(w);
    if (!c) return;

    if (!conf->displayhead) {
        glog("No displays configured for fullscreen!", LOGTYPE_ERR);
        return;
    }

    if (c->fullscreen) {
        XSetWindowBorderWidth(dpy, w, 4);
        XMoveResizeWindow(dpy, w, c->old_x, c->old_y, c->old_w, c->old_h);
        c->x = c->old_x; c->y = c->old_y; c->width = c->old_w; c->height = c->old_h;
        c->fullscreen = 0;

    } else {
        XWindowAttributes attr;
        if (XGetWindowAttributes(dpy, w, &attr)) {
            c->old_x = attr.x;
            c->old_y = attr.y;
            c->old_w = attr.width;
            c->old_h = attr.height;
            int cx, cy;

            if (XQueryPointer(dpy, root, &root_ret, &child_ret, &rx, &ry, &wx, &wy, &mask)) {
                cx = rx; cy=ry;
            } else {
                cx = attr.x + (attr.width / 2);
                cy = attr.y + (attr.height / 2);
            }

            gDisplay *target_disp = conf->displayhead;

            for (gDisplay *d = conf->displayhead; d; d = d->next) {
                if (cx >= d->posx && cx < d->posx + d->width &&
                    cy >= d->posy && cy < d->posy + d->height) {
                    target_disp = d;
                    break;
                }
            }

            int nx = target_disp->posx + target_disp->gapleft;

            int ny = target_disp->posy + target_disp->gaptop;

            unsigned int nw = target_disp->width - target_disp->gapleft - target_disp->gapright;

            unsigned int nh = target_disp->height - target_disp->gaptop - target_disp->gapbottom;

            XSetWindowBorderWidth(dpy, w, 0);
            XMoveResizeWindow(dpy, w, nx, ny, nw, nh);
            XMapRaised(dpy, w);
            XSync(dpy, False);
            XRaiseWindow(dpy, w);

            c->fullscreen = 1;
        }
    }
}

int x_error_handler(Display* d, XErrorEvent*  e) {
    char msg[512];
    XGetErrorText(d, e->error_code, msg, sizeof(msg));
    glog(msg, LOGTYPE_ERR);
    return 0;
}

int main() {
    XEvent ev;

    glog("Starting Glass...", LOGTYPE_INIT);

    init_env();


    glog("Loading config...", LOGTYPE_INIT);
    conf = read_config();
    if (!conf) {
        glog("Cannot read config! Defaults will be applied for the session.", LOGTYPE_ERR);
    }

    glog("Initializing display...", LOGTYPE_INIT);
    dpy = XOpenDisplay(NULL);
    if (!dpy) {
        fprintf(stderr, "cannot open display\n");
        glog("Cannot open display! This error is fatal, exiting.", LOGTYPE_ERR);
        return 1;
    }

    XSetErrorHandler(x_error_handler);

    root = DefaultRootWindow(dpy);

    cursor = XCreateFontCursor(dpy, XC_left_ptr);

    XDefineCursor(dpy, root, cursor);

    XWindowAttributes rootattr;
    XGetWindowAttributes(dpy, root, &rootattr);

    res_x = rootattr.width;
    res_y = rootattr.height;

    //rootattr.

    glog("Initializing ewmh...", LOGTYPE_INIT);

    g_ewmh_init(dpy, &ewmh);

    wmcheck = XCreateSimpleWindow(dpy, root, 0, 0, 1, 1, 0, 0, 0);

    g_ewmh_set_check(dpy, root, wmcheck, &ewmh);
    g_ewmh_set_wm_name(dpy, wmcheck, &ewmh, "Glass"); // this does something, but it doesnt do the other thing too i guess

    //not in ewmh?!
    XChangeProperty(dpy, root, XInternAtom(dpy, "_NET_WM_NAME", 0), XInternAtom(dpy, "UTF8_STRING", 0), 32, PropModeReplace, (unsigned char*)"Glass", 7);


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

    spawn("mkdir -p ~/.glass");
    spawn("touch ~/.glass/rc.sh");
    spawn("chmod +x ~/.glass/rc.sh");
    spawn("touch ~/.glass/glass.conf");

    if (conf->shrc) {
        glog("Running rc.sh...", LOGTYPE_INIT);
        spawn("~/.glass/rc.sh");
    }

    if (conf->displays > 0) {
            gDisplay* h = conf->displayhead;
            while (h) {
                char buf[128];
                snprintf(buf, sizeof(buf), "Display %s: %dx%d at %d,%d",
                         h->name, h->width, h->height, h->posx, h->posy);
                glog(buf, LOGTYPE_INIT);
                h = h->next;
            }
        }

    glog("Done!", LOGTYPE_INIT);


    for (;;) {
        XNextEvent(dpy, &ev);

        switch (ev.type) {

        case LeaveNotify: {
            XWindowAttributes a;

            Window w = ev.xcrossing.window;


            if (w==root) break;
            if (ev.xcrossing.mode != NotifyNormal || ev.xcrossing.detail == NotifyInferior) break;
            //if (w==focused) break;


            if (XGetWindowAttributes(dpy, w, &a)) {
                XSetWindowBorderWidth(dpy, w, 4);
                XSetWindowBorder(dpy, w, 0x000000);
            }
            break;
        }

        case EnterNotify: {

            if (ev.xcrossing.window == root) break;
            if (ev.xcrossing.mode != NotifyNormal || ev.xcrossing.detail == NotifyInferior) break;

            gClient* f = NULL;
            if (focused && focused != root && focused != None) f = find_client(focused);

            gClient* c = find_client(ev.xcrossing.window);
            if (!c) break;

            if (moving || resizing) break;

            if (f != NULL) {
                XWindowAttributes attr_f;
                XWindowAttributes attr_c;
                if (XGetWindowAttributes(dpy, focused, &attr_f) && XGetWindowAttributes(dpy, ev.xcrossing.window, &attr_c)) {
                    if (XQueryPointer(dpy, root, &root_ret, &child_ret, &rx, &ry, &wx, &wy, &mask)) {
                        if (rx >= attr_f.x && rx < attr_f.x + attr_f.width
                            && ry >= attr_f.y && ry < attr_f.y + attr_f.height) {
                                break;
                        } else {
                            XRaiseWindow(dpy, c->window);
                            switch_focus(c->window);
                        }
                    }
                }
            } else {
                XRaiseWindow(dpy, c->window);
                switch_focus(c->window);
            }


            break;
        }

        case MapRequest: {
            Window w = ev.xmaprequest.window;

            if (find_client(w)) break;

            Window par = ev.xmaprequest.parent;


            XWindowAttributes attrib;
            XGetWindowAttributes(dpy, w, &attrib);
            if (attrib.override_redirect) {
                glog("Panel detected! (override_redirect)", LOGTYPE_WINDOW);
                if (!panel) {
                    panel = w;
                }
                break;
            }

            add_client(w);

            gClient *c = find_client(w);

            c->fullscreen = 0;


            XSetWindowBorder(dpy, w, 0x000000);
            XSetWindowBorderWidth(dpy, w, 4);
            XSelectInput(dpy, w, EnterWindowMask | FocusChangeMask | PropertyChangeMask | StructureNotifyMask);

            /*
            if (!grabbed) {
                grabbed = w;
            }
             */

            gClient* rparent = find_client(par);
            if (rparent) c->workspace = rparent->workspace;

            Window trans;
            if (XGetTransientForHint(dpy, w, &trans)) {
                gClient* parent = find_client(trans);
                gClient* current = find_client(w);
                if (parent && current) {
                    current->workspace = parent->workspace;
                }
            }


            if (c && c->workspace == currentWorkspace) {
                XMapWindow(dpy, c->window);
                switch_focus(c->window);

                if (XQueryPointer(dpy, root, &root_ret, &child_ret, &rx, &ry, &wx, &wy, &mask)) {
                    int pos_x = rx - attrib.width / 2;
                    int pos_y = ry - attrib.height / 2;

                    if (pos_x < 0) pos_x = 0;
                    if (pos_y < 0) pos_y = 0;
                    if (pos_x + attrib.width > res_x) pos_x = res_x - attrib.width;
                    if (pos_y + attrib.height > res_y) pos_y = res_y - attrib.height;

                    XMoveWindow(dpy, c->window, pos_x, pos_y);
                } else
                if (conf->warpPointer && c->workspace == currentWorkspace) {
                    XWarpPointer(dpy, None, focused, 0,0,0,0,attrib.width/2, attrib.height/2);
                }


                glog("New client, hello!", LOGTYPE_WINDOW);
                g_ewmh_set_window_type(dpy, c->window, &ewmh);
                g_ewmh_set_motif_hints(dpy, c->window, 1, &ewmh);
                //g_ewmh_set_gtk_frame_extents(dpy, c->window, 1, 1, 23, 1, &ewmh);

            }
            break;
        }

        case DestroyNotify:
            Window w = ev.xdestroywindow.window;
            if (focused == w) {
                focused = None;
                XSetInputFocus(dpy, root, RevertToPointerRoot, CurrentTime);
            }
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

            //if (XSet)

            XConfigureWindow(dpy, ev.xconfigurerequest.window, ev.xconfigurerequest.value_mask, &changes);
            break;

        case ButtonPress:
            if (ev.xbutton.state & MOD && (ev.xbutton.button == Button4 || ev.xbutton.button == Button5)) {
                gClient* temp = find_client(focused);
                u8 ws = 0;

                if (ev.xbutton.button == Button4) {
                    if (currentWorkspace < 8) {
                        ws = currentWorkspace + 1;
                    } else {
                        ws = 0;
                    }
                } else if (ev.xbutton.button == Button5){
                    if (currentWorkspace > 0) {
                        ws = currentWorkspace - 1;
                    } else {
                        ws = 8;
                    }
                }
                if (moving && temp) {
                    temp->workspace = ws;
                }
                switch_workspace(ws);
                break;
            }

            target = ev.xbutton.subwindow;

            if (!target) break;

            if (target == root) break;

            switch_focus(target);


            /* TODO

            if (!(ev.xbutton.state & MOD) && ev.xbutton.button == Button1) {
                if (!grabbed) {
                    grabbed=target;
                    break;
                }
                if (target != grabbed) {
                    XUngrabButton(dpy, Button1, AnyModifier, grabbed);
                    XGrabButton(dpy, Button1, AnyModifier, target, False, ButtonPressMask, GrabModeSync, GrabModeAsync, None, None);
                    grabbed=target;
                }
                XAllowEvents(dpy, ReplayPointer, CurrentTime);
                XRaiseWindow(dpy, target);
                break;
            }

             */

            XRaiseWindow(dpy, target);
            //XAllowEvents(dpy, ReplayPointer, CurrentTime);

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

        case KeyRelease:
            if (ev.xbutton.button == XK_Super_L) {

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
                    if (!focused || focused == root) break;
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
                    if (panel == None) break;
                    if (showPanel) {
                        XUnmapWindow(dpy, panel);
                    } else {
                        XMapWindow(dpy, panel);
                    }
                    showPanel = !showPanel;
                    break;
                }
                case (BFULLSCREEN): {
                    if (focused && focused != root) {
                        toggle_fullscreen(focused);
                    }
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
                case (BCYCLE):
                    cycle_windows();
                    break;
                }

            if (bind->type >= 0 && bind->type <= 8) {
                if (!(ev.xkey.state & ShiftMask)) {
                    /*
                    if (moving && focused) {
                        gClient* c = find_client(focused);
                        if (c)  {
                            XUnmapWindow(dpy, c->window);
                            c->workspace = bind->type;
                        }
                    }
                    */
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
