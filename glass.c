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
#include "include/client.c"
#include "include/viewport.c"
#include "include/display.c"
#include "include/widgets.c"

#define MOD Mod4Mask   // Super key
#define GVIEWPORTS 9



int rx, ry, wx, wy;
unsigned int mask;
Window root_ret, child_ret;

Display *dpy;
int screen;

gConfig* conf;

Window root;
Window focused;
Window prev_focused = None;
Window grabbed;
Window target;

Window bar;
GC bargc;
i32 barx, bary;
u32 barw, barh;

u32 res_x;
u32 res_y;

u8 grunning;

int moving = 0, resizing = 0;
int start_x, start_y;
int win_x, win_y, win_w, win_h;

gClient* clients = NULL;
gClient* panels = NULL;

Window panel = None;
int showPanel = 1;


u8 currentViewport = 0;
gViewport viewports[GVIEWPORTS];

gEWMH ewmh;
Window wmcheck;

Cursor cursor;



void switch_focus(Window trg)  {
    gClient *f = find_client(clients, focused);
    gClient *c = find_client(clients, trg);

    if (focused && f && !f->fullscreen) {
        XSetWindowBorderWidth(dpy, focused, 4);
        XSetWindowBorder(dpy, focused, 0x000000);
    }

    if (c) {
        if (!c->fullscreen) {
            XSetWindowBorderWidth(dpy, trg, 4);
            XSetWindowBorder(dpy, trg, 0xFFFFFF);
        }
        XSetInputFocus(dpy, trg, RevertToPointerRoot, CurrentTime);
        XRaiseWindow(dpy, trg);
    } else {
        XSetInputFocus(dpy, root, RevertToPointerRoot, CurrentTime);
    }
    // bp1
    prev_focused = focused;
    focused = trg;
    viewports[currentViewport].last_focused = focused;
    g_ewmh_set_active_window(dpy, root, &ewmh, focused);

}

void switch_viewport(u8 vp) {
    if (vp == currentViewport) return;
    Window next_focus = None;
    Window last = None;
    gClient* c = find_client(clients, focused);
    if (moving && c) {
        c->viewport = vp;
        next_focus = focused;
    } else if (c) {
        if (viewports[currentViewport].last_focused != None) {
            gClient* l = find_client(clients, viewports[currentViewport].last_focused);
            if (l) next_focus = viewports[currentViewport].last_focused;
        }
    }


    for (gClient *c = clients; c; c = c->next) {
        XWindowAttributes attributes;
        u8 h = XGetWindowAttributes(dpy, c->window, &attributes);
        if (c->viewport == vp) {
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

    currentViewport = vp;
    g_ewmh_set_current_desktop(dpy, root, &ewmh, vp);


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
        if (conf->warpPointer && !moving) {
            if (conf->primaryDisplay != NULL) {
                XWarpPointer(dpy, None, root, 0, 0, 0, 0,
                conf->primaryDisplay->posx + conf->primaryDisplay->width / 2,
                conf->primaryDisplay->posy + conf->primaryDisplay->height / 2);
                XSync(dpy, False);
            } else {
                XWindowAttributes b;
                if (XGetWindowAttributes(dpy, root, &b)) XWarpPointer(dpy, None, root, 0, 0, 0, 0, b.width / 2, b.height / 2);
            }
        }
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

void cycle_windows() {
    if (!clients) return;

    gClient *active = find_client(clients, focused);

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
        if (c->viewport == currentViewport) {
            //XRaiseWindow(dpy, c->window)
            //
            /*
            if (conf->displays) {
                int cx, cy;
                if (XQueryPointer(dpy, root, &root_ret, &child_ret, &rx, &ry, &wx, &wy, &mask)) {
                    cx = rx; cy=ry;
                    gDisplay* hi = gGetMouseDisplay(conf->displayhead, cx,cy);
                    if (hi)
                }
            }
             */
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
    gClient *c = find_client(clients, w);
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

            gDisplay* target_disp = conf->displayhead;

            for (gDisplay* d = conf->displayhead; d; d = d->next) {
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


int check_all_clients() {
    XWindowAttributes a;
    register int n = 0;
    char buffer[256];
    gClient* prev = NULL;
    gClient* c = clients;
    gClient* pf = NULL; // previous focus
    while (c) {
        gClient* next = c->next;

        if (XGetWindowAttributes(dpy, c->window, &a)) {
            c->x = a.x; c->y = a.y; c->width = a.width; c->height = a.height; n++; prev = c;
            if (prev_focused) if (c->window == prev_focused) pf=c;
        } else {
            if (prev) prev->next = next;
            else clients = next;


            if (focused == c->window) {
                gClient* ppp = pf;
                if (!pf) ppp = find_client(clients, prev_focused);

                if (ppp && ppp->viewport == currentViewport) switch_focus(prev_focused);
                else focused = None;
            }


            free(c);
            snprintf(buffer, sizeof(buffer), "Killed ghost client");
            glog(buffer, LOGTYPE_RUNTIME);
        }
        c = next;
    }
    XSync(dpy, False);
    return n;
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

    grunning = 1;


    glog("Set up environment...", LOGTYPE_INIT);

    init_env();


    glog("Trying config path '$HOME/.glass/glass.conf'", LOGTYPE_INIT);
    conf = read_config(".glass/glass.conf", 1);
    if (conf) goto initdisp;
    if (conf == NULL) {
        glog("Trying config path '/var/lib/glass/default/glass.conf'", LOGTYPE_INIT);
        conf = read_config("/var/lib/glass/default/glass.conf", 0);
    }

    if (conf == NULL) {
        glog("Cannot read config! Defaults will be applied for the session.", LOGTYPE_ERR);
        conf = default_config();
    } else {
        glog("Successfully read config.", LOGTYPE_INIT);
    }

initdisp:

    glog("Attempting to grab display...", LOGTYPE_INIT);
    dpy = XOpenDisplay(NULL);
    if (!dpy) {
        fprintf(stderr, "cannot open display\n");
        glog("Cannot open display! This error is fatal, exiting.", LOGTYPE_ERR);
        return 1;
    } else {
        glog("Successfully obtained display...", LOGTYPE_INIT);
    }

    screen = DefaultScreen(dpy);

    glog("Successfully obtained display...", LOGTYPE_INIT);

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
    g_ewmh_set_current_desktop(dpy, root, &ewmh, currentViewport);


    glog("Initialize viewports...", LOGTYPE_INIT);

    for (int x = 0; x < GVIEWPORTS; x++) {
        viewports[x].last_focused = None;
        viewports[x].mode = conf->autotile;
    }


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

    grab_key(XK_Super_L);
    grab_key(XK_Super_L & Mod4Mask);


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
                if (h == conf->primaryDisplay) {
                    snprintf(buf, sizeof(buf), "Primary display %s: %dx%d at %d,%d",
                             h->name, h->width, h->height, h->posx, h->posy);
                } else {
                    snprintf(buf, sizeof(buf), "Display %s: %dx%d at %d,%d",
                             h->name, h->width, h->height, h->posx, h->posy);
                }

                glog(buf, LOGTYPE_INIT);
                h = h->next;
            }
        }

    if (conf->primaryDisplay) {
        XWarpPointer(dpy, None, root, 0, 0, 0, 0,
        conf->primaryDisplay->posx + conf->primaryDisplay->width / 2,
        conf->primaryDisplay->posy + conf->primaryDisplay->height / 2);
        barx = conf->primaryDisplay->posx;
        bary = conf->primaryDisplay->posy + conf->primaryDisplay->height - 16;
        barw = conf->primaryDisplay->width;
        barh = 16;
        XSync(dpy, False);
    }



    //bar = gWidgetsModMenuCreate(dpy, root, barx, bary, barw, barh);
    // = XCreateGC(dpy, bar, 0, NULL);
    //XSetForeground(dpy, bargc, 0xFFFFFF);
    //XSetBackground(dpy, bargc, 0x000000);
    //XFontStruct* font = XLoadQueryFont(dpy, "fixed");

    //XSetFont(dpy, bargc, font->fid);

    //gWidgetsModMenuDraw(dpy, bar, DefaultGC(dpy, screen));
    //XMapWindow(dpy, bar);

    glog("Done!", LOGTYPE_INIT);




    while (grunning) {
        XNextEvent(dpy, &ev);

        if (ev.type == MapRequest || ev.type == DestroyNotify || ev.type == UnmapNotify ||
            ev.type == ConfigureRequest || ev.type == ButtonRelease) check_all_clients();

        switch (ev.type) {
        case LeaveNotify: {
            XWindowAttributes a;

            Window w = ev.xcrossing.window;
            gClient* f = find_client(clients, w);

            if (w==root) break;
            if (ev.xcrossing.mode != NotifyNormal || ev.xcrossing.detail == NotifyInferior) break;
            //if (w==focused) break;



            if (XGetWindowAttributes(dpy, w, &a) && f && !f->fullscreen) {
                XSetWindowBorderWidth(dpy, w, 4);
                XSetWindowBorder(dpy, w, 0x000000);
            }
            break;
        }

        case EnterNotify: {

            if (ev.xcrossing.window == root) break;
            if (ev.xcrossing.mode != NotifyNormal || ev.xcrossing.detail == NotifyInferior) break;

            gClient* f = NULL;
            if (focused && focused != root && focused != None) f = find_client(clients, focused);


            gClient* c = find_client(clients, ev.xcrossing.window);
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

            if (find_client(clients, w)) break;

            Window par = ev.xmaprequest.parent;


            XWindowAttributes attrib;
            u8 dowegotattribs = (u8)XGetWindowAttributes(dpy, w, &attrib);
            if (dowegotattribs && attrib.override_redirect) {
                if (conf->logWindows)glog("Panel detected! (override_redirect)", LOGTYPE_WINDOW);
                add_client(&panels, w);
                break;
            }

            add_client(&clients, w);

            gClient *c = find_client(clients, w);

            c->fullscreen = 0;


            XSetWindowBorder(dpy, w, 0x000000);
            XSetWindowBorderWidth(dpy, w, 4);
            XSelectInput(dpy, w, EnterWindowMask | FocusChangeMask | PropertyChangeMask | StructureNotifyMask);

            /*
            if (!grabbed) {
                grabbed = w;
            }
             */

            gClient* rparent = find_client(clients, par);
            if (rparent) c->viewport = rparent->viewport;
            else c->viewport = currentViewport;

            Window trans;
            if (XGetTransientForHint(dpy, w, &trans)) {
                gClient* parent = find_client(clients, trans);
                gClient* current = find_client(clients, w);
                if (parent && current) {
                    current->viewport = parent->viewport;
                }
            }

            if (conf->logWindows) {
                glog("New client, hello!", LOGTYPE_WINDOW);
                char buf[256];
                if (dowegotattribs) {
                    snprintf(buf, sizeof(buf), "New client: %dx%d at %d,%d",
                             attrib.width, attrib.height, attrib.x, attrib.y);
                } else {
                    snprintf(buf, sizeof(buf), "New client! (can't get attributes)");
                }
                glog(buf, LOGTYPE_WINDOW);


            if (c && c->viewport == currentViewport) {
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
                    if (conf->warpPointer && c->viewport == currentViewport) {
                        XWarpPointer(dpy, None, focused, 0,0,0,0,attrib.width/2, attrib.height/2);
                    }
                }

                g_ewmh_set_window_type(dpy, c->window, &ewmh);
                g_ewmh_set_motif_hints(dpy, c->window, 1, &ewmh);
                //g_ewmh_set_gtk_frame_extents(dpy, c->window, 1, 1, 23, 1, &ewmh);

            }
            break;
        }

        case DestroyNotify:
            Window w = ev.xdestroywindow.window;
            if (focused == w) {
                gClient* p = find_client(clients, prev_focused);
                if (p) switch_focus(prev_focused);
                else focused = None;
                XSetInputFocus(dpy, root, RevertToPointerRoot, CurrentTime);
            }
            if (find_client(clients, w)) remove_client(&clients, ev.xdestroywindow.window);
            else if (find_client(panels, w)) remove_client(&panels, ev.xdestroywindow.window);
            if (conf->logWindows) glog("Client destroyed.", LOGTYPE_WINDOW);
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
                gClient* temp = find_client(clients, focused);
                u8 vp = 0;

                if (ev.xbutton.button == Button4) {
                    if (currentViewport < 8) {
                        vp = currentViewport + 1;
                    } else {
                        vp = 0;
                    }
                } else if (ev.xbutton.button == Button5) {
                    if (currentViewport > 0) {
                        vp = currentViewport - 1;
                    } else {
                        vp = 8;
                    }
                }
                if (moving && temp) {
                    temp->viewport = vp;
                }
                switch_viewport(vp);
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
            /*
            if (ev.xbutton.button == XK_Super_L) {
                XUnmapWindow(dpy, bar);
            }
             */
            break;

        case MotionNotify:
            if (!target) break;

            int dx = ev.xmotion.x_root - start_x;
            int dy = ev.xmotion.y_root - start_y;

            if (!(dx || dy)) break;

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


            /*
            if (ev.xbutton.button == XK_Super_L) {
                gWidgetsModMenuDraw(dpy, bar, bargc);
                XMapWindow(dpy, bar);
                XRaiseWindow(dpy, bar);
            }
             */

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
                    c = find_client(clients, focused);
                    if (c) {
                        XKillClient(dpy, focused);
                        remove_client(&clients, focused);
                        if (conf->logWindows) glog("Killing client on user request", LOGTYPE_WINDOW);
                        //XSetInputFocus(dpy, Window, int, Time)
                    }
                    break;
                case (BSPAWN):
                    spawn((char*)bind->data);
                    break;
                case (BPANEL): {
                    if (panels == NULL) break;
                    if (showPanel) {
                        for (gClient* b = panels; b; b = b->next)
                            XUnmapWindow(dpy, panels->window);
                    } else {
                        for (gClient* b = panels; b; b = b->next)
                            XMapWindow(dpy, panels->window);
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
                    for (gDisplay* d = conf->displayhead; d; d = d->next) free(d);
                    for (gBind* b = conf->bindhead; b; b = b->next) free(b);
                    for (gClient* c = clients; c; c = c->next) free(c);

                    grunning = 0;
                    continue;
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
                            c->viewport = bind->type;
                        }
                    }
                    */
                    switch_viewport(bind->type);
                } else {
                    if (focused && focused != root) {
                        gClient* hi = find_client(clients, focused);
                        if (hi && (currentViewport != bind->type)) {
                            hi->viewport = bind->type;
                            XUnmapWindow(dpy, focused);
                        }
                    }
                }
            }
            break;
        }
    }

}
