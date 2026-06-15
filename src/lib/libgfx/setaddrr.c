#include <symbos.h>
#include <graphics.h>
#include "gfxcore.h"

char* Gfx_TileAddr_Raw(unsigned char bank, char* image, unsigned char tile) {
    return image + (Bank_ReadWord(bank, image) * tile) + 3;
}
