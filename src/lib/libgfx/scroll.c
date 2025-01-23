#include "gfxcore.h"
#include <string.h>

void _gfx_scroll(int bytes) {
    int len = (_gfx_activebw * _gfx_activeh) - abs(bytes);
    if (len < 0)
        return;
    if (bytes > 0)
        memmove(_gfx_active + 24 + bytes, _gfx_active + 24, len);
    else
        memmove(_gfx_active + 24, _gfx_active + 24 - bytes, len);
}

void Gfx_ScrollX(int pixels) {
    if (pixels == 0)
        return;
    _gfx_scroll(pixels / (_gfx_16 ? 2 : 4));
}

void Gfx_ScrollY(int pixels) {
    if (pixels == 0)
        return;
    _gfx_scroll(pixels * _gfx_activebw);
}
