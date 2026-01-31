#pragma once

#include <stdio.h>
#include <stdlib.h>
#include "types.h"
#include <unistd.h>

typedef enum LOGTYPE { LOGTYPE_DONT, LOGTYPE_WINDOW, LOGTYPE_KEY, LOGTYPE_ERR, LOGTYPE_WORKSPACE, LOGTYPE_RUNTIME, LOGTYPE_INIT, LOGTYPE_BEGIN, LOGTYPE_CONFIG
    } LOGTYPE;

static inline void glog(const char *msg, u8 type) {
    if (type == LOGTYPE_DONT) return;
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
            fprintf(f, "[viewport] ");
            break;
        case LOGTYPE_RUNTIME:
            fprintf(f, "[runtime] ");
            break;
        case LOGTYPE_BEGIN:
            fprintf(f, "---------------------------------------------------------");
            break;
    }

    fprintf(f, "%s\n", msg);
    printf("%s\n", msg);
    fclose(f);
}
