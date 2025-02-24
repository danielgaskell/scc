#include <symbos.h>
#include <graphics.h>
#include "gfxcore.h"

char _preptab1[4][4] =
    {{0x00, 0x40, 0x04, 0x44},
     {0x80, 0xC0, 0x84, 0xC4},
     {0x08, 0x48, 0x0C, 0x4C},
     {0x88, 0xC8, 0x8C, 0xCC}};
char _preptab2[4][4] =
    {{0x00, 0x10, 0x01, 0x11},
     {0x20, 0x30, 0x21, 0x31},
     {0x02, 0x12, 0x03, 0x13},
     {0x22, 0x32, 0x23, 0x33}};

void Gfx_Prep(char* buffer) {
    unsigned char img16 = 1;
    char *ptr, *ptr2, *ptrend;
    signed char ptroff = 0;
    unsigned short len;
    register unsigned char c, d;
    if ((buffer[0] == 0x40 && buffer[1] == 0x05)) {
        // convert (possibly incorrect) 16-color extended header (MSX Image Converter generates these)
        buffer[0] = (buffer[4] >> 1);
        buffer[1] = buffer[4];
        buffer[2] = buffer[6];
        ptroff = -2;
    }
    if (buffer[0] == (buffer[1] >> 2))
        img16 = 0;

    if (_gfx_16 && !img16) {
        // convert 4-color to 16-color
        buffer[0] = (buffer[1] >> 1);
        len = (((unsigned short)buffer[1] * buffer[2]) >> 2);
        ptrend = buffer + 3 + len;
        memcpy(ptrend, buffer + 3, len);
        ptr2 = buffer + 3;
        ptr = ptrend;
        ptrend += len;
        while (ptr < ptrend) {
            c = *ptr++;
            d = 0;
            if (c & 128)
                d |= 16;
            if (c & 64)
                d |= 1;
            if (c & 8)
                d |= 32;
            if (c & 4)
                d |= 2;
            *ptr2++ = d;
            d = 0;
            if (c & 32)
                d |= 16;
            if (c & 16)
                d |= 1;
            if (c & 2)
                d |= 32;
            if (c & 1)
                d |= 2;
            *ptr2++ = d;
        }

    } else if (_gfx_16 && img16) {
        // convert 16-color to 16-color: just remove excess header
        memcpy(buffer + 3, buffer + 10, (unsigned short)buffer[0] * buffer[2]);

    } else if (!_gfx_16 && img16) {
        // convert 16-color to 4-color
        buffer[0] = (buffer[1] >> 2);
        ptrend = buffer + 10 + (((unsigned short)buffer[1] * buffer[2]) >> 1);
        ptr = buffer + 10 + ptroff;
        ptr2 = buffer + 3;
        while (ptr < ptrend) {
            c = *ptr++;
            *ptr2 =  _preptab1[(c >> 4) & 3][c & 3];
            c = *ptr++;
            *ptr2 |= _preptab2[(c >> 4) & 3][c & 3];
            ++ptr2;
        }
    }
}

unsigned char _Gfx_Load(char* filename, unsigned char bank, char* buffer) {
    unsigned char fd;
    Dir_PathAdd(0, filename, buffer);
    fd = File_Open(_symbank, buffer);
    if (fd > 7)
        return 1;
    File_Read(fd, bank, buffer, 65535U);
    File_Close(fd);
    return 0;
}

unsigned char Gfx_Load(char* filename, char* buffer) {
    if (_Gfx_Load(filename, _symbank, buffer))
        return 1;
    Gfx_Prep(buffer);
    return 0;
}
