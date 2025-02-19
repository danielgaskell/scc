#include <symbos.h>

int Win_X(Window* win) {
    if (win->state & WIN_MAXIMIZED)
        return 0;
    return win->x + 1;
}

int Win_Y(Window* win) {
    int y;
    if (win->state & WIN_MAXIMIZED)
        y = 9;
    else
        y = win->y + 10;
    if (win->flags & WIN_MENU)
        y += 9;
    if (win->flags & WIN_TOOLBAR)
        y += win->toolheight;
    return y;
}
