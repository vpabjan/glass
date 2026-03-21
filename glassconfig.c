#include <ncurses.h>
#include <stdlib.h>
#include <string.h>
#include <X11/Xlib.h>
#include <X11/keysym.h>

#include "include/types.h"
#include "include/config.c" // Pulling in your config parser and saver

#define MENU_FEATURES 0
#define MENU_BINDS    1
#define MENU_DISPLAYS 2
#define MENU_SAVE     3

typedef enum { ITEM_BOOL, ITEM_NUM } ItemType;
typedef struct menu_item {
    char* title;
    ItemType type;
    void* value;
    struct menu_item* next;
} menu_item;

menu_item* feature_head = NULL;

void add_item(menu_item** head, char* title, ItemType type, void* value) {
    menu_item* new_item = malloc(sizeof(menu_item));
    new_item->title = strdup(title);
    new_item->type = type;
    new_item->value = value;
    new_item->next = NULL;
    if (*head == NULL) *head = new_item;
    else {
        menu_item* temp = *head;
        while (temp->next) temp = temp->next;
        temp->next = new_item;
    }
}

// --- UI HELPERS ---

// Draws the top Navigation Bar
void draw_tabs(int sx, int active) {
    const char* tabs[] = { " FEATURES ", " BINDS ", " DISPLAYS ", " [ SAVE & EXIT ] " };
    int x_cursor = 2;
    attron(COLOR_PAIR(4));
    mvhline(0, 0, ' ', sx);
    for (int i = 0; i < 4; i++) {
        if (i == active) attroff(COLOR_PAIR(4));
        mvprintw(0, x_cursor, "%s", tabs[i]);
        if (i == active) attron(COLOR_PAIR(4));
        x_cursor += strlen(tabs[i]) + 2;
    }
    attroff(COLOR_PAIR(4));
}

// A text-entry box for capturing strings/numbers directly in ncurses
void prompt_input(const char* prompt, char* buffer, int max_len) {
    int sy, sx; getmaxyx(stdscr, sy, sx);
    WINDOW* win = newwin(5, 50, sy/2 - 2, sx/2 - 25);
    wbkgd(win, COLOR_PAIR(2));
    box(win, 0, 0);
    mvwprintw(win, 1, 2, "%s", prompt);
    wmove(win, 2, 2);
    wrefresh(win);

    echo();
    curs_set(1);
    wgetnstr(win, buffer, max_len - 1);
    noecho();
    curs_set(0);
    delwin(win);
    clear();
}

// A generic popup list for actions (Edit, Delete, etc.)
int popup_menu(const char* title, const char** options, int num_options) {
    int sy, sx; getmaxyx(stdscr, sy, sx);
    WINDOW* win = newwin(num_options + 4, 40, sy/2 - (num_options+4)/2, sx/2 - 20);
    keypad(win, TRUE);
    int choice = 0;
    while(1) {
        wbkgd(win, COLOR_PAIR(2));
        box(win, 0, 0);
        wattron(win, A_BOLD);
        mvwprintw(win, 1, 2, "%s", title);
        wattroff(win, A_BOLD);
        mvwhline(win, 2, 1, ACS_HLINE, 38);
        for(int i=0; i<num_options; i++) {
            if (i == choice) wattron(win, A_REVERSE);
            mvwprintw(win, 3+i, 2, "%-36s", options[i]);
            if (i == choice) wattroff(win, A_REVERSE);
        }
        wrefresh(win);
        int c = wgetch(win);
        if (c == KEY_UP && choice > 0) choice--;
        if (c == KEY_DOWN && choice < num_options-1) choice++;
        if (c == 10) break; // Enter
        if (c == 27 || c == 'q') { choice = -1; break; } // Escape/Cancel
    }
    delwin(win);
    clear();
    return choice;
}

// Count nodes in linked lists
int count_binds(gBind* h) { int c=0; while(h){ c++; h=h->next; } return c; }
int count_displays(gDisplay* h) { int c=0; while(h){ c++; h=h->next; } return c; }

// --- MAIN LOOP ---
int main() {
    gConfig* conf = read_config(".glass/glass.conf", 1);
    if (!conf) conf = default_config();

    add_item(&feature_head, "Pointer Warping", ITEM_BOOL, &conf->warpPointer);
    add_item(&feature_head, "Run RC Script", ITEM_BOOL, &conf->shrc);
    add_item(&feature_head, "Log Windows", ITEM_BOOL, &conf->logWindows);
    add_item(&feature_head, "Auto-Tile", ITEM_BOOL, &conf->autotile);
    add_item(&feature_head, "Per Display Cycle", ITEM_BOOL, &conf->perDisplayCycle);
    add_item(&feature_head, "Padding Size", ITEM_NUM, &conf->padding);

    initscr();
    start_color();
    noecho();
    cbreak();
    keypad(stdscr, TRUE);
    curs_set(0);

    init_pair(1, COLOR_GREEN, COLOR_BLACK);  // Values
    init_pair(2, COLOR_CYAN, COLOR_BLACK);   // Popups/Titles
    init_pair(3, COLOR_RED, COLOR_BLACK);    // Selection
    init_pair(4, COLOR_BLACK, COLOR_WHITE);  // Top Bar

    int active_tab = MENU_FEATURES;
    int selection = 0;
    int scroll_offset = 0;
    int running = 1;

    while (running) {
        int sy, sx; getmaxyx(stdscr, sy, sx);
        int list_max = sy - 6; // Max items visible on screen at once
        clear();
        draw_tabs(sx, active_tab);

        // --- DRAWING LOGIC ---
        if (active_tab == MENU_FEATURES) {
            menu_item* curr = feature_head;
            int i = 0;
            while (curr) {
                if (i == selection) attron(COLOR_PAIR(3) | A_BOLD);
                mvprintw(3 + i, 4, "%-20s", curr->title);
                attroff(COLOR_PAIR(3) | A_BOLD);

                attron(COLOR_PAIR(1));
                if (curr->type == ITEM_BOOL) printw(": [%s]", (*(u8*)curr->value) ? "YES" : " NO");
                else if (curr->type == ITEM_NUM) printw(": < %d >", *(u8*)curr->value);
                attroff(COLOR_PAIR(1));

                curr = curr->next;
                i++;
            }
        }
        else if (active_tab == MENU_BINDS) {
            int b_count = count_binds(conf->bindhead);
            // Handle scrolling constraints
            if (selection < scroll_offset) scroll_offset = selection;
            if (selection >= scroll_offset + list_max) scroll_offset = selection - list_max + 1;

            gBind* b = conf->bindhead;
            int drawn = 0;
            for(int i = 0; i <= b_count; i++) {
                if (i >= scroll_offset && drawn < list_max) {
                    if (i == selection) attron(COLOR_PAIR(3) | A_REVERSE | A_BOLD);
                    if (i == b_count) {
                        mvprintw(4 + drawn, 4, " [+] Add New Keybind ");
                    } else {
                        const char* ksym = XKeysymToString(b->bind);
                        if (b->type == BSPAWN) {
                            mvprintw(4 + drawn, 4, " %-30s : Key %-10s -> %s ", getbindname(b->type), ksym ? ksym : "?", (char*)b->data);
                        } else {
                            mvprintw(4 + drawn, 4, " %-30s : Key %-10s ", getbindname(b->type), ksym ? ksym : "?");
                        }
                    }
                    if (i == selection) attroff(COLOR_PAIR(3) | A_REVERSE | A_BOLD);
                    drawn++;
                }
                if (b) b = b->next;
            }
        }
        else if (active_tab == MENU_DISPLAYS) {
            int d_count = count_displays(conf->displayhead);
            // Handle scrolling constraints
            if (selection < scroll_offset) scroll_offset = selection;
            if (selection >= scroll_offset + list_max) scroll_offset = selection - list_max + 1;

            gDisplay* d = conf->displayhead;
            int drawn = 0;
            for(int i = 0; i <= d_count; i++) {
                if (i >= scroll_offset && drawn < list_max) {
                    if (i == selection) attron(COLOR_PAIR(3) | A_REVERSE | A_BOLD);
                    if (i == d_count) {
                        mvprintw(4 + drawn, 4, " [+] Add New Display ");
                    } else {
                        char* primary = (conf->primaryDisplay == d) ? "*" : " ";
                        mvprintw(4 + drawn, 4, " %s[%s] %dx%d @ %d,%d ", primary, d->name, d->width, d->height, d->posx, d->posy);
                    }
                    if (i == selection) attroff(COLOR_PAIR(3) | A_REVERSE | A_BOLD);
                    drawn++;
                }
                if (d) d = d->next;
            }
        }

        refresh();

        // --- INPUT HANDLING ---
        int c = getch();
        switch (c) {
            case KEY_LEFT:
                if (active_tab > 0) active_tab--;
                selection = 0; scroll_offset = 0;
            break;
            case KEY_RIGHT:
                if (active_tab < 3) active_tab++;
                selection = 0; scroll_offset = 0;
            break;
            case KEY_UP:
                if (selection > 0) selection--;
                break;
            case KEY_DOWN:
                if (active_tab == MENU_FEATURES && selection < 5) selection++;
                if (active_tab == MENU_BINDS && selection < count_binds(conf->bindhead)) selection++;
                if (active_tab == MENU_DISPLAYS && selection < count_displays(conf->displayhead)) selection++;
                break;
            case ' ':
            case 10:  // Enter
                if (active_tab == MENU_SAVE) {
                    save_config(conf, ".glass/glass.conf", 1);
                    running = 0;
                }
                else if (active_tab == MENU_FEATURES) {
                    menu_item* curr = feature_head;
                    for(int j=0; j<selection && curr; j++) curr = curr->next;
                    if (curr) {
                        if (curr->type == ITEM_BOOL) *(u8*)curr->value = !(*(u8*)curr->value);
                        if (curr->type == ITEM_NUM) {
                            char buf[16] = {0};
                            prompt_input("Enter new padding value:", buf, 16);
                            if (buf[0]) *(u8*)curr->value = atoi(buf);
                        }
                    }
                }
                else if (active_tab == MENU_BINDS) {
                    int bc = count_binds(conf->bindhead);
                    if (selection < bc) {
                        gBind* b = conf->bindhead;
                        for(int k=0; k<selection; k++) b = b->next;
                        const char* opts[] = {"Change Key", "Change Command (if spawn)", "Delete Bind"};
                        int ch = popup_menu("Edit Bind", opts, 3);

                        if (ch == 0) {
                            char buf[32]={0};
                            prompt_input("New X11 Key (e.g. Return, a, d):", buf, 32);
                            KeySym ks = XStringToKeysym(buf);
                            if (ks != NoSymbol) b->bind = ks;
                        }
                        else if (ch == 1 && b->type == BSPAWN) {
                            char buf[128]={0};
                            prompt_input("New Command:", buf, 128);
                            if (strlen(buf) > 0) {
                                if (b->data) free(b->data);
                                b->data = strdup(buf);
                            }
                        }
                        else if (ch == 2) remove_bind(conf, b);
                    }
                    else if (selection == bc) { // Add new
                        char aBuf[32]={0}, kBuf[32]={0};
                        prompt_input("Action Type (>>, <<<, cycle, 1, etc):", aBuf, 32);
                        prompt_input("Key (e.g. Return, p):", kBuf, 32);
                        KeySym ks = XStringToKeysym(kBuf);
                        if (ks != NoSymbol && strlen(aBuf) > 0) {
                            gBindType t = BEXIT; void* d = NULL;
                            if (!strcmp(aBuf, ">>")) {
                                t = BSPAWN;
                                char cBuf[128]={0};
                                prompt_input("Command to execute:", cBuf, 128);
                                d = strdup(cBuf);
                            }
                            else if (!strcmp(aBuf, "<<")) t = BQUIT;
                            else if (!strcmp(aBuf, "<<<")) t = BEXIT;
                            else if (!strcmp(aBuf, "cycle")) t = BCYCLE;
                            else if (!strcmp(aBuf, "1")) t = BWS1;
                            // Add node to tail
                            gBind* curr = conf->bindhead;
                            if (!curr) conf->bindhead = gBindAdd(NULL, ks, t, d);
                            else {
                                while(curr->next) curr = curr->next;
                                gBindAdd(curr, ks, t, d);
                            }
                        }
                    }
                }
                else if (active_tab == MENU_DISPLAYS) {
                    int dc = count_displays(conf->displayhead);
                    if (selection < dc) { // Edit
                        gDisplay* d = conf->displayhead;
                        for(int k=0; k<selection; k++) d = d->next;
                        const char* opts[] = {"Rename", "Set X", "Set Y", "Set W", "Set H", "Toggle Primary", "Delete"};
                        int ch = popup_menu("Edit Display", opts, 7);
                        char buf[32]={0};
                        switch(ch) {
                            case 0: prompt_input("New Name:", buf, 32); if(d->name) free(d->name); d->name=strdup(buf); break;
                            case 1: prompt_input("X:", buf, 32); d->posx = atoi(buf); break;
                            case 2: prompt_input("Y:", buf, 32); d->posy = atoi(buf); break;
                            case 3: prompt_input("W:", buf, 32); d->width = atoi(buf); break;
                            case 4: prompt_input("H:", buf, 32); d->height = atoi(buf); break;
                            case 5: conf->primaryDisplay = d; break;
                            case 6: remove_display(conf, d); break;
                        }
                    } else if (selection == dc) { // Add new
                        char nameBuf[32]={0};
                        prompt_input("Display Name:", nameBuf, 32);
                        gDisplay* nd = new_empty_display();
                        nd->name = strdup(nameBuf);
                        if (!conf->displayhead) conf->displayhead = nd;
                        else {
                            gDisplay* t = conf->displayhead;
                            while(t->next) t = t->next;
                            t->next = nd;
                        }
                        conf->displays++;
                    }
                }
                break;
                            case 'q': running = 0; break;
        }
    }

    endwin();
    return 0;
}
