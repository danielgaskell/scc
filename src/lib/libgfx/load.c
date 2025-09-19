#include <symbos.h>
#include <graphics.h>
#include "gfxcore.h"

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
