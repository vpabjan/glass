#pragma once
#include <X11/Xlib.h>
#include "types.h"

typedef struct gDisplay {
    char* name;
    u32 height;
    u32 width;
    u32 posx;
    u32 posy;
    u32 gaptop, gapbottom, gapright, gapleft;

    struct gDisplay* next;
} gDisplay;

static inline u8 gGetMouseDisplayIndex(gDisplay* head, i32 mx, i32 my) {
    register u8 a = 128;
    gDisplay* b = head;
    while (b) {
        if (mx > b->posx && mx < b->posx + b->width &&
            my > b->posy && my < b->posy + b->height) return a;
        b = b->next;
        a++;
    }
    return a;
}

static inline gDisplay* gGetMouseDisplay(gDisplay* head, i32 mx, i32 my) {
    gDisplay* a = head;
    while (a) {
        if (mx > a->posx && mx < a->posx + a->width &&
            my > a->posy && my < a->posy + a->height) return a;
        head = head->next;
    }
    return NULL;
}
