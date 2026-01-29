#include "types.h"
#include <X11/X.h>
#include <X11/Xlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include "log.c"

typedef enum gBindType {
    BWS1 = 0, BWS2 = 1, BWS3 = 2,
    BWS4 = 3, BWS5 = 4, BWS6 = 5,
    BWS7 = 6, BWS8 = 7, BWS9 = 8,
    BTERMINAL, BLAUNCHER, BSPAWN,
    BQUIT, BEXIT, BPANEL,
    BCYCLE, BFULLSCREEN
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

typedef struct gDisplay {
    char* name;
    u32 height;
    u32 width;
    u32 posx;
    u32 posy;
    u32 gaptop, gapbottom, gapright, gapleft;

    struct gDisplay* next;
} gDisplay;

typedef struct gConfig {
    gBind* bindhead;
    gDisplay* displayhead;
    u8 warpPointer;
    u8 shrc;
    u8 dolog;

    u8 displays;

    char** need;
    u16 needlen;
    char** exec;
    u16 execlen;


} gConfig;

gConfig* read_config() {
    char path[256];
    gConfig* conf = (gConfig*)calloc(1, sizeof(gConfig));
    if (!conf) return NULL;


    // defaults
    conf->displayhead = NULL;
    conf->displays = 0;
    conf->bindhead = NULL;
    conf->warpPointer = 1;
    conf->shrc = 1;

    gDisplay* tempdisp = NULL;

    gBind* tail = NULL;

    snprintf(path, sizeof(path), "%s/.glass/glass.conf", getenv("HOME"));
    FILE *f = fopen(path, "r");
    if (!f) {
        return conf;
    }

    char line[256];

    const char* delim = " \n\t";

    while (fgets(line, sizeof(line), f)) {

        if (line[0] == '#') continue;

        char* arg = strtok(line, delim);
        if (!arg) continue;


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
        } else if (strcmp(arg, "do_rc") == 0) {
            char* option = strtok(NULL, delim);
            if (!option) continue;
            if (strcmp(option, "no") == 0) {
                conf->shrc = 0;
            } else if (strcmp(option, "yes") == 0) {
                conf->shrc = 1;
            }
        } else if (strcmp(arg, "do_log") == 0) {
            char* option = strtok(NULL, delim);
            if (!option) continue;
            if (strcmp(option, "no") == 0) {
                conf->dolog = 0;
            } else if (strcmp(option, "yes") == 0) {
                conf->dolog = 1;
            }
        } else if (strcmp(arg, "display") == 0) {
            char* option = strtok(NULL, delim);
            if (!option) continue;
            if (strcmp(option, "add") == 0) {
                char* name = strtok(NULL, delim);
                if (!name) continue;
                gDisplay* disp = (gDisplay*)malloc(sizeof(gDisplay));
                if (!disp) continue;
                disp->name = strdup(name);
                if (!tempdisp) {
                    conf->displayhead = disp;
                    tempdisp = disp;
                } else {
                    tempdisp->next = disp;
                    tempdisp = disp;
                }
                conf->displays++;
                continue;
            }
            for (gDisplay* last = conf->displayhead; last; last = last->next) {
                if (strcmp(last->name, option) != 0) continue;

                char* setting = strtok(NULL, delim);

                if (!setting) continue;

                if (strcmp(setting, "w") == 0) {
                    char* nextsetting = strtok(NULL, delim);
                    if (!nextsetting) continue;
                    last->width = atoi(nextsetting);
                } else if (strcmp(setting, "h") == 0) {
                    char* nextsetting = strtok(NULL, delim);
                    if (!nextsetting) continue;
                    last->height = atoi(nextsetting);
                } else if (strcmp(setting, "x") == 0) {
                    char* nextsetting = strtok(NULL, delim);
                    if (!nextsetting) continue;
                    last->posx = atoi(nextsetting);
                } else if (strcmp(setting, "y") == 0) {
                    char* nextsetting = strtok(NULL, delim);
                    if (!nextsetting) continue;
                    last->posy = atoi(nextsetting);
                } else if (strcmp(setting, "top") == 0) {
                    char* nextsetting = strtok(NULL, delim);
                    if (!nextsetting) continue;
                    last->gaptop = atoi(nextsetting);
                } else if (strcmp(setting, "bottom") == 0) {
                    char* nextsetting = strtok(NULL, delim);
                    if (!nextsetting) continue;
                    last->gapbottom = atoi(nextsetting);
                } else if (strcmp(setting, "right") == 0) {
                    char* nextsetting = strtok(NULL, delim);
                    if (!nextsetting) continue;
                    last->gapright = atoi(nextsetting);
                } else if (strcmp(setting, "left") == 0) {
                    char* nextsetting = strtok(NULL, delim);
                    if (!nextsetting) continue;
                    last->gapleft = atoi(nextsetting);
                }
            }
        }
        /* else if (strcmp(arg, "exec") == 0) {

            char* cmd = strtok(NULL, "\n");
            if (cmd) {
                gExec* e = (gExec*)malloc(sizeof(gExec));
                if (!e) continue;
                e->cmd = strdup(cmd);
                if (!exec) {
                } else {
                    conf->exec->next = e;
            }

        }
        */

    }
    fclose(f);
    return conf;
}
