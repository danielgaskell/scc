#include <graphics.h>
#include "gfxcore.h"

void Gfx_Circle(signed short x0, signed short y0, signed short radius, unsigned char color) {
    signed short x = -radius;
    signed short y = 0;
    signed short err = 2-2*radius;
    do {
        Gfx_Safe_Pixel(x0-x, y0+y, color);
        Gfx_Safe_Pixel(x0-y, y0-x, color);
        Gfx_Safe_Pixel(x0+x, y0-y, color);
        Gfx_Safe_Pixel(x0+y, y0+x, color);
        radius = err;
        if (radius <= y)
            err += ++y*2+1;
        if (radius > x || err > y)
            err += ++x*2+1;
    } while (x < 0);
}
