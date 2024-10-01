#include <graphics.h>
#include "gfxcore.h"

// FIXME - slow and broken.
void Gfx_Paint(signed short x, unsigned char y, unsigned char color) {
    signed short x1;
    unsigned char old_color = Gfx_Value(x, y);

    if (_screencolors == 4)
        color &= 3;

    if (old_color == color)
        return;

    // draw current scanline right
    x1 = x;
    while (x1 <= _gfx_activew && Gfx_Value(x1, y) == old_color) {
        Gfx_Pixel(x1, y, color);
        ++x1;
    }

    // draw current scanline left
    x1 = x - 1;
    while (x1 >= 0 && Gfx_Value(x1, y) == old_color) {
        Gfx_Pixel(x1, y, color);
        --x1;
    }

    // test for new scanlines above
    x1 = x;
    while (x1 <= _gfx_activew && Gfx_Value(x1, y) == color) {
        if (y > 0 && Gfx_Value(x1, y-1) == old_color)
            Gfx_Paint(x1, y - 1, color);
        ++x1;
    }
    x1 = x - 1;
    while (x1 >= 0 && Gfx_Value(x1, y) == color) {
        if (y > 0 && Gfx_Value(x1, y-1) == old_color)
            Gfx_Paint(x1, y - 1, color);
        --x1;
    }

    // test for new scanlines below
    x1 = x;
    while (x1 <= _gfx_activew && Gfx_Value(x1, y) == color) {
        if (y < _gfx_activeh && Gfx_Value(x1, y+1) == old_color)
            Gfx_Paint(x1, y + 1, color);
        ++x1;
    }
    x1 = x - 1;
    while (x1 >= 0 && Gfx_Value(x1, y) == color) {
        if (y < _gfx_activeh && Gfx_Value(x1, y+1) == old_color)
            Gfx_Paint(x1, y + 1, color);
        --x1;
    }
}
