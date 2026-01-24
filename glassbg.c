#include <X11/Xlib.h>
#include <Imlib2.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

typedef enum { SCALE, CENTER, TILE } mode_type;

int main(int argc, char **argv) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s [--scale|--center|--tile] <image>\n", argv[0]);
        return 1;
    }

    mode_t mode = SCALE;
    char *imgfile;

    if (strncmp(argv[1], "--", 2) == 0) {
        if (strcmp(argv[1], "--scale") == 0) mode = SCALE;
        else if (strcmp(argv[1], "--center") == 0) mode = CENTER;
        else if (strcmp(argv[1], "--tile") == 0) mode = TILE;
        else {
            fprintf(stderr, "Unknown mode %s\n", argv[1]);
            return 1;
        }
        if (argc < 3) {
            fprintf(stderr, "Image path required\n");
            return 1;
        }
        imgfile = argv[2];
    } else {
        imgfile = argv[1];
    }

    Display *dpy = XOpenDisplay(NULL);
    if (!dpy) return 1;

    int screen = DefaultScreen(dpy);
    Window root = RootWindow(dpy, screen);
    int sw = DisplayWidth(dpy, screen);
    int sh = DisplayHeight(dpy, screen);

    imlib_context_set_display(dpy);
    imlib_context_set_visual(DefaultVisual(dpy, screen));
    imlib_context_set_colormap(DefaultColormap(dpy, screen));
    imlib_context_set_drawable(root);

    Imlib_Image img = imlib_load_image(imgfile);
    if (!img) {
        fprintf(stderr, "Failed to load image %s\n", imgfile);
        XCloseDisplay(dpy);
        return 1;
    }
    imlib_context_set_image(img);

    switch (mode) {
        case SCALE:
            imlib_render_image_on_drawable_at_size(0, 0, sw, sh);
            break;
        case CENTER: {
            int iw = imlib_image_get_width();
            int ih = imlib_image_get_height();
            int x = (sw - iw) / 2;
            int y = (sh - ih) / 2;
            imlib_render_image_on_drawable(x, y);
            break;
        }
        case TILE: {
            int iw = imlib_image_get_width();
            int ih = imlib_image_get_height();
            for (int y = 0; y < sh; y += ih)
                for (int x = 0; x < sw; x += iw)
                    imlib_render_image_on_drawable(x, y);
            break;
        }
    }

    imlib_free_image();
    XFlush(dpy);
    XCloseDisplay(dpy);

    printf("OK");
    return 0;
}
