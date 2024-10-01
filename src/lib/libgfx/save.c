#include <symbos.h>
#include <iobuf.h>
#include <graphics.h>
#include "gfxcore.h"

unsigned char Gfx_Save(char* filename, char* buffer) {
    unsigned char fd;
    unsigned short len = (unsigned short)buffer[0] * buffer[2];
    Dir_PathAdd(0, filename, _io_buf);
    fd = File_Open(_symbank, _io_buf);
    if (fd > 7)
        return 1;
    File_Write(fd, _symbank, buffer, 3);
    if ((buffer[1] / buffer[0]) == 2)
        File_Write(fd, _symbank, "\x00\x00\x00\x00\x20\x01\x05", 7);
    File_Write(fd, _symbank, buffer + 3, len);
    File_Close(fd);
    return 0;
}
