#include <graphics.h>
#include "gfxcore.h"

void Gfx_Box(unsigned short x0, unsigned char y0, unsigned short x1, unsigned short y1, unsigned char color) {
    Gfx_HLine(x0, y0, x1-x0,   color);
    Gfx_VLine(x0, y0, y1-y0,   color);
    Gfx_VLine(x1, y0, y1-y0+1, color);
    Gfx_HLine(x0, y1, x1-x0,   color);
}
