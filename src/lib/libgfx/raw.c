#include <symbos.h>
#include <graphics.h>
#include "gfxcore.h"

void Gfx_Prep_Raw(unsigned char bank, char* buffer) {
    // connect pointers in 16-color extended header, if present
    if (Bank_ReadByte(bank, buffer) == (Bank_ReadByte(bank, buffer + 1) >> 1)) {
        Bank_WriteWord(bank, buffer + 3, (unsigned short)(buffer + 9));
        Bank_WriteWord(bank, buffer + 5, (unsigned short)(buffer + 8));
    }
}

void Gfx_Prep_Set_Raw(unsigned char bank, char* buffer) {
    unsigned char i, t;
    unsigned short len = Bank_ReadWord(bank, buffer);
    char* ptr = buffer + 3;
    t = Bank_ReadByte(bank, buffer + 2);
    for (i = 0; i < t; ++i) {
        Gfx_Prep_Raw(bank, ptr);
        ptr += len;
    }
}

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
