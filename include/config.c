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
    BSPAWN, BQUIT, BEXIT,
    BPANEL, BCYCLE, BFULLSCREEN,
    GTILE
} gBindType;

typedef struct gBind {
    KeySym bind;
    gBindType type;
    void* data;
    struct gBind* next;
} gBind;

gBind* gBindAdd(gBind* tail, KeySym bind, gBindType type, void* data) {
    gBind* a = malloc(sizeof(gBind));
    if(!a) {
        return tail;
    }

    a->type = type;
    a->bind = bind;
    a->data = data;
    a->next = NULL;

    if (tail) {
        tail->next = a;
    }

    return a;
}

typedef struct gConfig {
    gBind* bindhead;
    gDisplay* displayhead;
    gDisplay* primaryDisplay;

    // bools
    u8 warpPointer;
    u8 shrc;
    u8 logWindows;
    u8 autotile;
    u8 modmenu;
    u8 displays;
} gConfig;

static inline gDisplay* new_empty_display() {
    gDisplay* p = calloc(1, sizeof(gDisplay));
    if (!p) return p;
    p->width = 1920; p->height = 1080;
    p->posx = 0; p->posy = 0;
    p->gapleft = 0; p->gapright = 0;
    p->next = NULL;
    return p;
}

gConfig* read_config() {
    char path[256];
    gConfig* conf = (gConfig*)calloc(1, sizeof(gConfig));
    if (!conf) return NULL;

    snprintf(path, sizeof(path), "%s/.glass/glass.conf", getenv("HOME"));
    FILE *f = fopen(path, "r");


    // defaults
    conf->displayhead = NULL;
    conf->autotile = 0;
    conf->displays = 0;
    conf->bindhead = NULL;
    conf->warpPointer = 1;
    conf->primaryDisplay = NULL;
    conf->logWindows = 1;
    conf->shrc = 1;
    conf->modmenu = 1;

    if (!f) {
        return conf;
    }

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
                printf("Warning: Invalid key '%s'\n", keyStr);
                continue;
            }

            gBindType type;
            void* data = NULL;

            if (strcmp(action, "<<<") == 0) {
                type = BEXIT;
            } else if (strcmp(action, "<<") == 0) {
                type = BQUIT;
            } else if (strcmp(action, ">>") == 0) {
                type = BSPAWN;
                char* cmd = strtok(NULL, "\n");
                if (cmd) {
                    data = strdup(cmd);
                } else {
                    data = strdup("true");
                }
            } else if (strcmp(action, "panel") == 0) {
                type = BPANEL;
            } else if (strcmp(action, "1") == 0){
                type = BWS1;
            } else if (strcmp(action, "2") == 0){
                type = BWS2;
            } else if (strcmp(action, "3") == 0){
                type = BWS3;
            } else if (strcmp(action, "4") == 0){
                type = BWS4;
            } else if (strcmp(action, "5") == 0){
                type = BWS5;
            } else if (strcmp(action, "6") == 0){
                type = BWS6;
            } else if (strcmp(action, "7") == 0){
                type = BWS7;
            } else if (strcmp(action, "8") == 0){
                type = BWS8;
            } else if (strcmp(action, "9") == 0){
                type = BWS9;
            } else if (strcmp(action, "cycle") == 0){
                type = BCYCLE;
            } else if (strcmp(action, "fullscreen") == 0) {
                type = BFULLSCREEN;
            }
            gBind* new_node = gBindAdd(tail, key, type, data);

            if (conf->bindhead == NULL) {
                conf->bindhead = new_node;
            }

            tail = new_node;
        } else if (strcmp(arg, "warp_pointer") == 0) {
            char* option = strtok(NULL, delim);
            if (!option) continue;
            if (strcmp(option, "no") == 0) {
                conf->warpPointer = 0;
            } else if (strcmp(option, "yes") == 0) {
                conf->warpPointer = 1;
            }
        } else if (strcmp(arg, "wm") == 0) {
            char* option = strtok(NULL, delim);
            if (!option) continue;
            if (strcmp(option, "float") == 0) {
                conf->autotile = 0;
            } else if (strcmp(option, "tile") == 0) {
                conf->autotile = 1;
            }
        } else if (strcmp(arg, "do_rc") == 0) {
            char* option = strtok(NULL, delim);
            if (!option) continue;
            if (strcmp(option, "no") == 0) {
                conf->shrc = 0;
            } else if (strcmp(option, "yes") == 0) {
                conf->shrc = 1;
            }
        } else if (strcmp(arg, "log_windows") == 0) {
            char* option = strtok(NULL, delim);
            if (!option) continue;
            if (strcmp(option, "no") == 0) {
                conf->logWindows = 0;
            } else if (strcmp(option, "yes") == 0) {
                conf->logWindows = 1;
            }
        } else if (strcmp(arg, "display") == 0) {
            char* option = strtok(NULL, delim);
            if (!option) continue;
            if (!strcmp(option, "add")) {
                char* val = strtok(NULL, delim);
                if (!val) continue;
                if (!conf->displayhead) {
                    conf->displayhead = new_empty_display();
                    tempdisp = conf->displayhead;
                } else {
                    tempdisp->next = new_empty_display();
                    tempdisp = tempdisp->next;
                }
                selected = tempdisp;
                if (val) selected->name = strdup(val);
                else selected->name = strdup("n/a");
                conf->displays++;
            } else if (selected) {
                char* val = strtok(NULL, " \n");
                if (!val) {
                    if (!strcmp(option, "nogaps")) selected->gapleft = selected->gapright = selected->gaptop = selected->gapbottom = 0;
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
