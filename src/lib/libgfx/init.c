#include <symbos.h>
#include <graphics.h>
#include "gfxcore.h"

unsigned char _screencolors = 255;

void Gfx_ScreenInit(void) {
    if (_screencolors == 255) {
        _screencolors = Screen_Colors();
        if (_screencolors == 16) {
            _gfx_pix = _gfx_pix16;
            _gfx_val = _gfx_val16;
            _gfx_hline = _gfx_hline16;
            _gfx_vline = _gfx_vline16;
            _gfx_put = _gfx_put16;
            _gfx_get = _gfx_get16;
        } else {
            _gfx_pix = _gfx_pix4;
            _gfx_val = _gfx_val4;
            _gfx_hline = _gfx_hline4;
            _gfx_vline = _gfx_vline4;
            _gfx_put = _gfx_put4;
            _gfx_get = _gfx_get4;
        }
    }
}

void Gfx_Clear(char* canvas, unsigned char color) {
    unsigned char bit0, bit1;
    if (_screencolors == 4) {
        bit0 = color & 1;
        bit1 = color & 2;
        color = (bit0 << 4) | (bit0 << 5) | (bit0 << 6) | (bit0 << 7) | (bit1 >> 1) | (bit1) | (bit1 << 1) | (bit1 << 2);
    } else {
        color = (color & 0x0F) | (color << 4);
    }
    memset(canvas + 24, color, (unsigned short)canvas[0] * canvas[2]);
}

// note: canvases must have a width that is a multiple of 8. Max width of 504.
void Gfx_Init(char* canvas, unsigned short w, unsigned char h) {
    Img_Header* head;
    unsigned short bytew, byteoff, len;
    unsigned char this_bytew, this_w, headoff;

    Gfx_ScreenInit();
    if (_screencolors == 16)
        bytew = w >> 1;
    else
        bytew = w >> 2;
    len = bytew * h;

    headoff = 0;
    byteoff = 24;
    *((unsigned short*)&canvas[20]) = w - 1;
    while (w) {
        this_w = (w > 252) ? 252 : w;
        head = (Img_Header*)(canvas + headoff);
        head->bytew = bytew;
        head->w = this_w;
        head->h = h;
        head->addrData = canvas + byteoff;
        head->addrEncoding = canvas + 23;
        head->len = len;
        headoff += 10;
        w -= this_w;
        byteoff += ((_screencolors == 16) ? 126 : 63);
    }
    canvas[22] = h - 1;
    canvas[23] = (_screencolors == 16) ? 5 : 0; // encoding byte
    Gfx_Clear(canvas, 8); // clear to white
}

void Gfx_Select(char* canvas) {
    _gfx_active = canvas;
    _gfx_activedat = canvas + 24;
    _gfx_activebw = canvas[0];
    _gfx_activew = *((unsigned short*)&canvas[20]);
    _gfx_activeh = canvas[22];
}
