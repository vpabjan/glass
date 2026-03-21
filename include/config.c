#include "types.h"
#include <X11/X.h>
#include <X11/Xlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include "display.c"
#include "log.c"

typedef enum gBindType {
    BWS1 = 0, BWS2 = 1, BWS3 = 2,
    BWS4 = 3, BWS5 = 4, BWS6 = 5,
    BWS7 = 6, BWS8 = 7, BWS9 = 8,
    BSPAWN = 20, BQUIT = 21, BEXIT = 22,
    BPANEL = 23, BCYCLE = 24, BFULLSCREEN = 25,
    BMONO = 26, BAOT = 27, BRELOAD = 28, BFREE = 29, BTILE = 30
} gBindType;

char* getbindname(gBindType b) {
    switch (b) {
        case (BWS1): return "Switch to viewport 1"; break;
        case (BWS2): return "Switch to viewport 2"; break;
        case (BWS3): return "Switch to viewport 3"; break;
        case (BWS4): return "Switch to viewport 4"; break;
        case (BWS5): return "Switch to viewport 5"; break;
        case (BWS6): return "Switch to viewport 6"; break;
        case (BWS7): return "Switch to viewport 7"; break;
        case (BWS8): return "Switch to viewport 8"; break;
        case (BWS9): return "Switch to viewport 9"; break;
        case (BSPAWN): return "Spawn"; break;
        case (BQUIT): return "Quit"; break;
        case (BEXIT): return "Exit Glass"; break;
        case (BPANEL): return "Toggle panel"; break;
        case (BCYCLE): return "Cycle windows"; break;
        case (BFULLSCREEN): return "Switch window to fullscreen mode"; break;
        case (BMONO): return "Switch window to mono mode"; break;
        case (BFREE): return "Switch window to free mode"; break;
        case (BRELOAD): return "Reload glass"; break;
        default: return "Unknown keybind";
    }
}

typedef struct gBind {
    KeySym bind;
    gBindType type;
    void* data;
    struct gBind* next;
} gBind;

gBind* gBindAdd(gBind* tail, KeySym bind, gBindType type, void* data) {
    gBind* a = malloc(sizeof(gBind));
    if(!a) return tail;
    a->type = type;
    a->bind = bind;
    a->data = data;
    a->next = NULL;
    if (tail) tail->next = a;
    return a;
}

typedef struct gConfig {
    gBind* bindhead;
    gDisplay* displayhead;
    gDisplay* primaryDisplay;
    u8 perDisplayCycle;
    u8 warpPointer;
    u8 shrc;
    u8 logWindows;
    u8 autotile;
    u8 padding;
    u8 displays;
} gConfig;

// --- LINKED LIST HELPERS ---
void remove_bind(gConfig* conf, gBind* target) {
    if (!conf->bindhead || !target) return;
    if (conf->bindhead == target) {
        conf->bindhead = target->next;
        free(target);
        return;
    }
    gBind* curr = conf->bindhead;
    while (curr->next && curr->next != target) curr = curr->next;
    if (curr->next == target) {
        curr->next = target->next;
        free(target);
    }
}

void remove_display(gConfig* conf, gDisplay* target) {
    if (!conf->displayhead || !target) return;
    if (conf->displayhead == target) {
        conf->displayhead = target->next;
        free(target);
        conf->displays--;
        return;
    }
    gDisplay* curr = conf->displayhead;
    while (curr->next && curr->next != target) curr = curr->next;
    if (curr->next == target) {
        curr->next = target->next;
        free(target);
        conf->displays--;
    }
}

static inline gDisplay* new_empty_display() {
    gDisplay* p = calloc(1, sizeof(gDisplay));
    if (!p) return p;
    p->width = 1920; p->height = 1080;
    p->posx = 0; p->posy = 0;
    p->gapleft = 0; p->gapright = 0;
    p->next = NULL;
    p->nogaps = 0;
    return p;
}

gConfig* default_config() {
    gConfig* conf = (gConfig*)calloc(1, sizeof(gConfig));
    if (!conf) return NULL;
    conf->displayhead = NULL;
    conf->autotile = 0;
    conf->displays = 0;
    conf->bindhead = NULL;
    conf->warpPointer = 1;
    conf->primaryDisplay = NULL;
    conf->logWindows = 1;
    conf->shrc = 1;
    conf->padding = 0;
    conf->perDisplayCycle = 1;
    return conf;
}

void save_config(gConfig* conf, char* path, u8 home) {
    char finalpath[512];
    FILE* f = NULL;

    if (home) {
        snprintf(finalpath, sizeof(finalpath), "%s/%s", getenv("HOME"), path);
        f = fopen(finalpath, "w");
    } else {
        f = fopen(path, "w");
    }

    if (!f) return;

    fprintf(f, "# Glass Window Manager Configuration\n\n");
    fprintf(f, "warp_pointer %s\n", conf->warpPointer ? "yes" : "no");
    fprintf(f, "do_rc %s\n", conf->shrc ? "yes" : "no");
    fprintf(f, "log_windows %s\n", conf->logWindows ? "yes" : "no");
    fprintf(f, "per_display_cycle %s\n", conf->perDisplayCycle ? "yes" : "no");
    fprintf(f, "wm %s\n", conf->autotile ? "tile" : "float");
    fprintf(f, "padding %d\n\n", conf->padding);

    gBind* b = conf->bindhead;
    while (b) {
        char* keyname = XKeysymToString(b->bind);
        if (!keyname) keyname = "unknown";
        switch (b->type) {
            case BEXIT: fprintf(f, "suprbind <<< %s\n", keyname); break;
            case BQUIT: fprintf(f, "suprbind << %s\n", keyname); break;
            case BSPAWN: fprintf(f, "suprbind >> %s %s\n", keyname, b->data ? (char*)b->data : ""); break;
            case BPANEL: fprintf(f, "suprbind panel %s\n", keyname); break;
            case BWS1: fprintf(f, "suprbind 1 %s\n", keyname); break;
            case BWS2: fprintf(f, "suprbind 2 %s\n", keyname); break;
            case BWS3: fprintf(f, "suprbind 3 %s\n", keyname); break;
            case BWS4: fprintf(f, "suprbind 4 %s\n", keyname); break;
            case BWS5: fprintf(f, "suprbind 5 %s\n", keyname); break;
            case BWS6: fprintf(f, "suprbind 6 %s\n", keyname); break;
            case BWS7: fprintf(f, "suprbind 7 %s\n", keyname); break;
            case BWS8: fprintf(f, "suprbind 8 %s\n", keyname); break;
            case BWS9: fprintf(f, "suprbind 9 %s\n", keyname); break;
            case BCYCLE: fprintf(f, "suprbind cycle %s\n", keyname); break;
            case BFULLSCREEN: fprintf(f, "suprbind fullscreen %s\n", keyname); break;
            case BMONO: fprintf(f, "suprbind mono %s\n", keyname); break;
            case BFREE: fprintf(f, "suprbind free %s\n", keyname); break;
            case BRELOAD: fprintf(f, "suprbind reload %s\n", keyname); break;
            default: break;
        }
        b = b->next;
    }

    gDisplay* d = conf->displayhead;
    while (d) {
        fprintf(f, "\ndisplay add %s\n", d->name ? d->name : "unknown");
        if (conf->primaryDisplay == d) fprintf(f, "display primary\n");
        if (d->nogaps) fprintf(f, "display nogaps\n");
        fprintf(f, "display x %d\n", d->posx);
        fprintf(f, "display y %d\n", d->posy);
        fprintf(f, "display w %d\n", d->width);
        fprintf(f, "display h %d\n", d->height);
        if (d->gaptop) fprintf(f, "display top %d\n", d->gaptop);
        if (d->gapbottom) fprintf(f, "display bottom %d\n", d->gapbottom);
        if (d->gapleft) fprintf(f, "display left %d\n", d->gapleft);
        if (d->gapright) fprintf(f, "display right %d\n", d->gapright);
        d = d->next;
    }

    fclose(f);
}

gConfig* read_config(char* path, u8 home) {
    gConfig* conf = default_config();
    if (!conf) return NULL;
    char finalpath[512];
    FILE* f = NULL;

    if (home) {
        snprintf(finalpath, sizeof(finalpath), "%s/%s", getenv("HOME"), path);
        f = fopen(finalpath, "r");
    } else {
        f = fopen(path, "r");
    }

    if (!f) return NULL;

    gDisplay* tempdisp = NULL;
    gDisplay* selected = NULL;
    gBind* tail = NULL;
    char line[256];
    const char* delim = " \t\r\n";

    while (fgets(line, sizeof(line), f)) {
        char* arg = strtok(line, delim);
        if (!arg || arg[0] == '#') continue;

        if (strcmp(arg, "suprbind") == 0) {
            char* action = strtok(NULL, delim);
            if (!action) continue;

            char* keyStr = strtok(NULL, delim);
            if (!keyStr) continue;

            KeySym key = XStringToKeysym(keyStr);
            if (key == NoSymbol) {
                continue;
            }

            gBindType type;
            void* data = NULL;

            if (strcmp(action, "<<<") == 0) type = BEXIT;
            else if (strcmp(action, "<<") == 0) type = BQUIT;
            else if (strcmp(action, ">>") == 0) {
                type = BSPAWN;
                char* cmd = strtok(NULL, "\n"); // Get rest of line for spawn command
                if (cmd) data = strdup(cmd + (cmd[0] == ' ' ? 1 : 0)); // skip leading space
                else data = strdup("true");
            } else if (strcmp(action, "panel") == 0) type = BPANEL;
            else if (strcmp(action, "1") == 0) type = BWS1;
            else if (strcmp(action, "2") == 0) type = BWS2;
            else if (strcmp(action, "3") == 0) type = BWS3;
            else if (strcmp(action, "4") == 0) type = BWS4;
            else if (strcmp(action, "5") == 0) type = BWS5;
            else if (strcmp(action, "6") == 0) type = BWS6;
            else if (strcmp(action, "7") == 0) type = BWS7;
            else if (strcmp(action, "8") == 0) type = BWS8;
            else if (strcmp(action, "9") == 0) type = BWS9;
            else if (strcmp(action, "cycle") == 0) type = BCYCLE;
            else if (strcmp(action, "fullscreen") == 0) type = BFULLSCREEN;
            else if (strcmp(action, "mono") == 0) type = BMONO;
            else if (strcmp(action, "reload") == 0) type = BRELOAD;
            else if (strcmp(action, "free") == 0) type = BFREE;

            gBind* new_node = gBindAdd(tail, key, type, data);
            if (conf->bindhead == NULL) conf->bindhead = new_node;
            tail = new_node;

        } else if (strcmp(arg, "warp_pointer") == 0) {
            char* option = strtok(NULL, delim);
            if (option) conf->warpPointer = (strcmp(option, "yes") == 0);
        } else if (strcmp(arg, "wm") == 0) {
            char* option = strtok(NULL, delim);
            if (option) conf->autotile = (strcmp(option, "tile") == 0);
        } else if (strcmp(arg, "per_display_cycle") == 0) {
            char* option = strtok(NULL, delim);
            if (option) conf->perDisplayCycle = (strcmp(option, "yes") == 0);
        } else if (strcmp(arg, "do_rc") == 0) {
            char* option = strtok(NULL, delim);
            if (option) conf->shrc = (strcmp(option, "yes") == 0);
        } else if (strcmp(arg, "padding") == 0) {
            char* option = strtok(NULL, delim);
            if (option) conf->padding = atoi(option);
        } else if (strcmp(arg, "log_windows") == 0) {
            char* option = strtok(NULL, delim);
            if (option) conf->logWindows = (strcmp(option, "yes") == 0);
        } else if (strcmp(arg, "display") == 0) {
            char* option = strtok(NULL, delim);
            if (!option) continue;
            if (!strcmp(option, "add")) {
                char* val = strtok(NULL, delim);
                if (!conf->displayhead) {
                    conf->displayhead = new_empty_display();
                    tempdisp = conf->displayhead;
                } else {
                    tempdisp->next = new_empty_display();
                    tempdisp = tempdisp->next;
                }
                selected = tempdisp;
                selected->name = val ? strdup(val) : strdup("n/a");
                conf->displays++;
            } else if (selected) {
                char* val = strtok(NULL, " \n");
                if (!val) {
                    if (!strcmp(option, "nogaps")) selected->nogaps = 1;
                    else if (!strcmp(option, "primary")) conf->primaryDisplay = selected;
                } else {
                    if (!strcmp(option, "x")) selected->posx = atoi(val);
                    else if (!strcmp(option, "y")) selected->posy = atoi(val);
                    else if (!strcmp(option, "w")) selected->width = atoi(val);
                    else if (!strcmp(option, "h")) selected->height = atoi(val);
                    else if (!strcmp(option, "top")) selected->gaptop = atoi(val);
                    else if (!strcmp(option, "bottom")) selected->gapbottom = atoi(val);
                    else if (!strcmp(option, "right")) selected->gapright = atoi(val);
                    else if (!strcmp(option, "left")) selected->gapleft = atoi(val);
                }
            }
        }
    }
    fclose(f);
    return conf;
}
