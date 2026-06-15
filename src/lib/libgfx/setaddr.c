#include <symbos.h>
#include <graphics.h>
#include "gfxcore.h"

char* Gfx_TileAddr(char* image, unsigned char tile) {
    return image + (*(unsigned short*)image * tile) + 3;
}
