#include "types.h"
#include <X11/X.h>
#include <X11/Xlib.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

typedef enum gBindType {
    BWS1 = 0, BWS2 = 1, BWS3 = 2, BWS4 = 3, BWS5 = 4, BWS6 = 5, BWS7 = 6, BWS8 = 7, BWS9 = 8, BTERMINAL, BLAUNCHER, BSPAWN, BQUIT, BEXIT, BPANEL
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
} gConfig;

gConfig* read_config() {
    char path[256];
    gConfig* conf = (gConfig*)malloc(sizeof(gConfig));
    if (!conf) return NULL;

    conf->bindhead = NULL;

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
            } else {
                continue;
            }
            gBind* new_node = gBindAdd(tail, key, type, data);

            if (conf->bindhead == NULL) {
                conf->bindhead = new_node;
            }

            tail = new_node;
        }
    }
    fclose(f);
    return conf;
}
