#include <X11/Xlib.h>
#include "types.h"
#include <stdlib.h>

typedef struct gClient {
    Window window;
    u32 height;
    u32 width;
    u32 x;
    u32 y;
    u8 viewport;
    u8 fullscreen;
    int old_x, old_y, old_w, old_h;

    struct gClient* next;
} gClient;


void add_client(gClient** clients, Window w) {
    gClient *c = calloc(1, sizeof(gClient));
    if (!c) return;
    c->window = w;
    c->viewport = 0;

    c->next = *clients;
    *clients = c;
}

void remove_client(gClient** clients, Window w) {
    gClient **curr = clients;
    while (*curr) {
        gClient *entry = *curr;
        if (entry->window == w) {
            *curr = entry->next;
            free(entry);
            return;
        }
        curr = &entry->next;
    }
}

gClient* find_client(gClient* clients, Window w) {
    for (gClient *c = clients; c; c = c->next)
        if (c->window == w)
            return c;
    return NULL;
}
