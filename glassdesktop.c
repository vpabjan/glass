#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "include/types.h"

#define GRID_SIZE 80
#define ICON_SIZE 48

const u8 ICON_FILE = 0; const u8 ICON_FOLDER = 1; const u8 ICON_CMD = 2;

int cam_x = 0;
int cam_y = 0;

typedef struct gIcon {
    short grid_x;
    short grid_y;
    u8 type;
    char label[64];
    char command[256];
    struct gIcon *next;
} gIcon;

gIcon *icons = NULL;
gIcon *grabbed_icon = NULL;
int is_moving_icon = 0;
int is_panning = 0;

void add_icon(short x, short y, const char* name, const char* cmd, u8 type) {
    gIcon *n = malloc(sizeof(gIcon));
    n->grid_x = x;
    n->grid_y = y;
    n->type = type;
    strncpy(n->label, name, 63);
    strncpy(n->command, cmd, 255);
    n->next = icons;
    icons = n;
}


void screen_to_grid(int mx, int my, int sw, int sh, short *gx, short *gy) {
    int origin_x = (sw / 2) - cam_x;
    int origin_y = (sh / 2) - cam_y;
    *gx = (mx - origin_x) / GRID_SIZE;
    *gy = (my - origin_y) / GRID_SIZE;


    if (mx < origin_x) *gx -= 1;
    if (my < origin_y) *gy -= 1;
}


gIcon* get_icon_at(int mx, int my, int sw, int sh) {
    int origin_x = (sw / 2) - cam_x;
    int origin_y = (sh / 2) - cam_y;
    gIcon *curr = icons;
    while (curr) {
        int ix = origin_x + (curr->grid_x * GRID_SIZE);
        int iy = origin_y + (curr->grid_y * GRID_SIZE);
        if (mx >= ix && mx <= ix + GRID_SIZE && my >= iy && my <= iy + GRID_SIZE)
            return curr;
        curr = curr->next;
    }
    return NULL;
}


void get_config_path(char *buf, size_t len) {
    char *home = getenv("HOME");
    snprintf(buf, len, "%s/.glass/desktop.conf", home);
}

void save_config() {
    char path[512];
    get_config_path(path, sizeof(path));

    FILE *f = fopen(path, "w");
    if (!f) return;

    gIcon *curr = icons;
    while (curr) {
        char *type_str = "file";
        if (curr->type == ICON_FOLDER) type_str = "folder";
        if (curr->type == ICON_CMD)    type_str = "command";


        fprintf(f, "%s %s %d %d %s\n",
                type_str, curr->label, curr->grid_x, curr->grid_y, curr->command);
        curr = curr->next;
    }
    fclose(f);
}

void read_config() {
    char path[512];
    get_config_path(path, sizeof(path));

    FILE *f = fopen(path, "r");
    if (!f) return;

    char line[512];
    while (fgets(line, sizeof(line), f)) {
        if (line[0] == '#' || line[0] == '\n') continue;

        char *type_s = strtok(line, " ");
        char *name   = strtok(NULL, " ");
        char *xs     = strtok(NULL, " ");
        char *ys     = strtok(NULL, " ");
        char *cmd    = strtok(NULL, "\n");

        if (!type_s || !name || !xs || !ys || !cmd) continue;

        u8 type = ICON_FILE;
        if (!strcmp(type_s, "folder")) type = ICON_FOLDER;
        else if (!strcmp(type_s, "command")) type = ICON_CMD;

        add_icon(atoi(xs), atoi(ys), name, cmd, type);
    }
    fclose(f);
}



int main() {
    Display *dpy = XOpenDisplay(NULL);
    if (!dpy) return 1;

    int screen = DefaultScreen(dpy);
    Window root = RootWindow(dpy, screen);
    int sw = DisplayWidth(dpy, screen);
    int sh = DisplayHeight(dpy, screen);

    XSetWindowAttributes attrs;
    attrs.override_redirect = True; // glass will ignore what we do !
    attrs.background_pixel = 0x11111b;
    attrs.event_mask = ExposureMask | ButtonPressMask | ButtonReleaseMask | PointerMotionMask;

    Window win = XCreateWindow(dpy, root, 0, 0, sw, sh, 0,
                               CopyFromParent, InputOutput, CopyFromParent,
                               CWOverrideRedirect | CWBackPixel | CWEventMask, &attrs);

    XMapWindow(dpy, win);
    XLowerWindow(dpy, win);

    GC gc = XCreateGC(dpy, win, 0, NULL);
    XFontStruct *font = XLoadQueryFont(dpy, "fixed");

    read_config();

    XEvent ev;
    int start_x, start_y;
    int moved_threshold = 0;

    while (1) {
        XNextEvent(dpy, &ev);

        if (ev.type == Expose) {
            XClearWindow(dpy, win);
            int origin_x = (sw / 2) - cam_x;
            int origin_y = (sh / 2) - cam_y;

            // Draw Grid
            for (int i = (origin_x % GRID_SIZE) - GRID_SIZE; i < sw + GRID_SIZE; i += GRID_SIZE) {
                XSetForeground(dpy, gc, (i == origin_x) ? 0x45475a : 0x313244);
                XSetLineAttributes(dpy, gc, (i == origin_x) ? 3 : 1, LineSolid, CapButt, JoinMiter);
                XDrawLine(dpy, win, gc, i, 0, i, sh);
            }
            for (int j = (origin_y % GRID_SIZE) - GRID_SIZE; j < sh + GRID_SIZE; j += GRID_SIZE) {
                XSetForeground(dpy, gc, (j == origin_y) ? 0x45475a : 0x313244);
                XSetLineAttributes(dpy, gc, (j == origin_y) ? 3 : 1, LineSolid, CapButt, JoinMiter);
                XDrawLine(dpy, win, gc, 0, j, sw, j);
            }

            // Draw Icons
            gIcon *curr = icons;
            while (curr) {
                int ix = origin_x + (curr->grid_x * GRID_SIZE);
                int iy = origin_y + (curr->grid_y * GRID_SIZE);

                if (ix > -GRID_SIZE && ix < sw && iy > -GRID_SIZE && iy < sh) {
                    XSetForeground(dpy, gc, 0xcdd6f4);

                    if (is_moving_icon && grabbed_icon == curr)
                        XSetLineAttributes(dpy, gc, 2, LineOnOffDash, CapButt, JoinMiter);
                    else
                        XSetLineAttributes(dpy, gc, 2, LineSolid, CapButt, JoinMiter);

                    XDrawRectangle(dpy, win, gc, ix + 20, iy + 20, 40, 40);
                    XDrawString(dpy, win, gc, ix + 10, iy + GRID_SIZE - 5, curr->label, strlen(curr->label));
                }
                curr = curr->next;
            }
        }

        else if (ev.type == ButtonPress) {
            if (ev.xbutton.button == 1) {
                grabbed_icon = get_icon_at(ev.xbutton.x, ev.xbutton.y, sw, sh);
                is_panning = (grabbed_icon == NULL);
                start_x = ev.xbutton.x; start_y = ev.xbutton.y;
                moved_threshold = 0;
            }
            else if (ev.xbutton.button == 3) {
                grabbed_icon = get_icon_at(ev.xbutton.x, ev.xbutton.y, sw, sh);
                if (grabbed_icon) is_moving_icon = 1;
            }
            else if (ev.xbutton.button == 2) {
                cam_x = cam_y = 0;
                XClearArea(dpy, win, 0, 0, 0, 0, True);
            }
        }

        else if (ev.type == MotionNotify) {
            if (is_panning) {
                cam_x -= (ev.xmotion.x - start_x);
                cam_y -= (ev.xmotion.y - start_y);
                start_x = ev.xmotion.x; start_y = ev.xmotion.y;
                moved_threshold = 1;
                XClearArea(dpy, win, 0, 0, 0, 0, True);
            }
            else if (is_moving_icon && grabbed_icon) {
                short gx, gy;
                screen_to_grid(ev.xmotion.x, ev.xmotion.y, sw, sh, &gx, &gy);
                if (grabbed_icon->grid_x != gx || grabbed_icon->grid_y != gy) {
                    grabbed_icon->grid_x = gx;
                    grabbed_icon->grid_y = gy;
                    XClearArea(dpy, win, 0, 0, 0, 0, True);
                }
            }
        }

        else if (ev.type == ButtonRelease) {
            if (ev.xbutton.button == 1 && grabbed_icon && !moved_threshold) {
                if (fork() == 0) {
                    setsid();
                    char cmd[300];
                    snprintf(cmd, sizeof(cmd), "%s &", grabbed_icon->command);
                    execl("/bin/sh", "sh", "-c", cmd, (char *)NULL);
                    exit(0);
                }
            }

            if (is_moving_icon) save_config();
            is_panning = is_moving_icon = 0;
            grabbed_icon = NULL;
        }
    }

    XCloseDisplay(dpy);
    return 0;
}
