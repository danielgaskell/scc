#include <graphics.h>
#include "gfxcore.h"

void Gfx_LineB(unsigned short x0, unsigned char y0, unsigned short x1, unsigned char y1, unsigned char color, unsigned char bits) {
    signed char dx =  abs (x1 - x0);
    signed char sx = x0 < x1 ? 1 : -1;
    signed char dy = -abs (y1 - y0);
    signed char sy = y0 < y1 ? 1 : -1;
    signed char err = dx + dy;
    signed char e2;

    for (;;) {
        if (bits & 0x80)
            Gfx_Pixel(x0, y0, color);
        if (x0 == x1 && y0 == y1)
            break;
        e2 = 2 * err;
        if (e2 >= dy) {
            err += dy;
            x0 += sx;
        }
        if (e2 <= dx) {
            err += dx;
            y0 += sy;
        }
        bits = (bits << 1) | (bits >> 7);
    }
}
