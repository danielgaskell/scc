#include <symbos.h>
#include <graphics.h>
#include "gfxcore.h"

// FIXME: Experimental/buggy, do not use.
char* Gfx_Init_Split(char* canvas, int x, int y, unsigned char w, unsigned char h, void* ctrl1, void* ctrl2) {
    Img_Header* head;
    unsigned short bytew, len;
    unsigned char offset;
    unsigned short head1, data1, data2, head2;
    unsigned char h1, h2;

    if (_gfx_16 == 255)
        _gfx_16 = (Screen_Colors() == 16);
    if (_gfx_16)
        bytew = w >> 1;
    else
        bytew = w >> 2;
    len = bytew * h;

    if ((((unsigned short)canvas + len + 24) & 0xC000) < (unsigned short)canvas) {
        // fits within one page
        head = (Img_Header*)canvas;
        head->bytew = bytew;
        head->w = w;
        head->h = h;
        head->addrData = canvas + 24;
        head->addrEncoding = canvas + 23;
        head->len = len;
        ((Ctrl*)ctrl1)->param = (unsigned short)canvas;
        ((Ctrl*)ctrl1)->x = x;
        ((Ctrl*)ctrl1)->y = y;
        ((Ctrl*)ctrl1)->w = w;
        ((Ctrl*)ctrl1)->h = h;
        ((Ctrl*)ctrl2)->x = 10000;
        ((Ctrl*)ctrl2)->param = 0;
    } else {
        // split across two pages
        offset = ((0x4000 % bytew) - (((unsigned short)canvas + 24) % bytew) + bytew) % bytew;
        head1 = (unsigned short)(canvas + offset);
        data1 = head1 + 24;
        data2 = ((data1 + 0x4000 - 1) & ~(0x4000 - 1));
        h1 = (data2 - data1) / bytew;
        h2 = h - h1;
        head2 = data2 + h2 * bytew;

        // construct first canvas
        head = (Img_Header*)head1;
        head->bytew = bytew;
        head->w = w;
        head->h = h1;
        head->addrData = (char*)data1;
        head->addrEncoding = (char*)(head1 + 23);
        head->len = len;
        ((Ctrl*)ctrl1)->param = (unsigned short)head1;
        ((Ctrl*)ctrl1)->x = x;
        ((Ctrl*)ctrl1)->y = y;
        ((Ctrl*)ctrl1)->w = w;
        ((Ctrl*)ctrl1)->h = h1;

        // construct second canvas
        head = (Img_Header*)head2;
        head->bytew = bytew;
        head->w = w;
        head->h = h2;
        head->addrData = (char*)data2;
        head->addrEncoding = (char*)(head2 + 11);
        head->len = len;
        ((Ctrl*)ctrl2)->param = (unsigned short)head2;
        ((Ctrl*)ctrl2)->x = x;
        ((Ctrl*)ctrl2)->y = y+h1;
        ((Ctrl*)ctrl2)->w = w;
        ((Ctrl*)ctrl2)->h = h2;

        // update address
        canvas = (char*)head1;
    }
    canvas[22] = h - 1;
    canvas[23] = _gfx_16 ? 5 : 0; // encoding byte
    *(char*)(head2 + 11) = _gfx_16 ? 5 : 0; // encoding byte
    Gfx_Clear(canvas, 8); // clear to white
    return canvas;
}
