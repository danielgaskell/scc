#include <symbos.h>
#include <graphics.h>
#include "gfxcore.h"

unsigned char Gfx_Load_Raw(char* filename, unsigned char bank, char* buffer) {
    if (_Gfx_Load(filename, bank, buffer))
        return 1;
    Gfx_Prep_Raw(bank, buffer);
    return 0;
}

unsigned char Gfx_Load_Set_Raw(char* filename, unsigned char bank, char* buffer) {
    if (_Gfx_Load(filename, bank, buffer))
        return 1;
    Gfx_Prep_Set_Raw(bank, buffer);
    return 0;
}

char* Gfx_TileAddr_Raw(unsigned char bank, char* image, unsigned char tile) {
    return image + (Bank_ReadWord(bank, image) * tile) + 3;
}
