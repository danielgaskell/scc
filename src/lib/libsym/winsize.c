#include <symbos.h>

unsigned short Win_Width(Window* win) {
    unsigned short w;
    if (win->flags & WIN_RESIZABLE) {
        if (win->state == WIN_MAXIMIZED) {
            w = Screen_Width();
            if ((win->flags & WIN_RESIZABLE) && (!(win->flags & WIN_ADJUSTX) || _symversion < 40))
                w -= 8;
            if (w > win->wmax)
                return win->wmax;
            return w;
        }
    }
    if (win->w < win->wmin)
        return win->wmin;
    if (win->w > win->wmax)
        return win->wmax;
    return win->w;
}

unsigned short Win_Height(Window* win) {
    unsigned short h;
    if (win->flags & WIN_RESIZABLE) {
        if (win->state == WIN_MAXIMIZED) {
            h = Screen_Height() - 23;
            if ((win->flags & WIN_RESIZABLE) && (!(win->flags & WIN_ADJUSTY) || _symversion < 40))
                h -= 8;
            if (win->flags & WIN_MENU)
                h -= 9;
            if (win->flags & WIN_TOOLBAR)
                h -= win->toolheight;
            if (win->flags & WIN_STATUS)
                h -= 9;
            if (h > win->hmax)
                return win->hmax;
            return h;
        }
    }
    if (win->h < win->hmin)
        return win->hmin;
    if (win->h > win->hmax)
        return win->hmax;
    return win->h;
}
