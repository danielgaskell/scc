#include <symbos.h>
#include <graphics.h>
#include "gfxcore.h"

void Gfx_Prep_Set(char* buffer) {
    unsigned char i;
    unsigned short len = *(unsigned short*)buffer;
    char* ptr = buffer + 3;
    for (i = 0; i < buffer[2]; ++i) {
        Gfx_Prep(ptr);
        ptr += len;
    }
}

unsigned char Gfx_Load_Set(char* filename, char* buffer) {
    if (_Gfx_Load(filename, _symbank, buffer))
        return 1;
    Gfx_Prep_Set(buffer);
    return 0;
}

void Gfx_Put_Set(char* image, unsigned short x, unsigned char y, unsigned char mode, unsigned char tile) {
    image += (*(unsigned short*)image * tile) + 3;
    Gfx_Put(image, x, y, mode);
}

char* Gfx_TileAddr(char* image, unsigned char tile) {
    return image + (*(unsigned short*)image * tile) + 3;
}
