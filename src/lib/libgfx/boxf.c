#include <graphics.h>
#include "gfxcore.h"

// FIXME this can be optimized quite a lot.
void Gfx_BoxF(unsigned short x0, unsigned char y0, unsigned short x1, unsigned short y1, unsigned char color) {
    unsigned short w = x1 - x0 + 1;
    unsigned short y;
    for (y = y0; y <= y1; ++y)
        Gfx_HLine(x0, y, w, color);
}
