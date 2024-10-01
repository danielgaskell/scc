#include <graphics.h>
#include "gfxcore.h"

unsigned char Gfx_Load_Set(char* filename, char* buffer) {
    unsigned char i;
    unsigned short len;
    char* ptr;
    if (_Gfx_Load(filename, buffer))
        return 1;
    ptr = buffer + 3;
    len = *(unsigned short*)buffer;
    for (i = 0; i < buffer[2]; ++i) {
        Gfx_Prep(ptr);
        ptr += len;
    }
    return 0;
}

void Gfx_Put_Set(char* image, unsigned short x, unsigned char y, unsigned char mode, unsigned char tile) {
    image += (*(unsigned short*)image * tile) + 3;
    Gfx_Put(image, x, y, mode);
}
