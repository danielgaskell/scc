#include <symbos.h>

/* ========================================================================== */
/* File Manager                                                               */
/* ========================================================================== */

unsigned char Dir_Delete(unsigned char bank, char* path) {
    unsigned char result;
    _msemaon();
    _symmsg[1] = 39;
    *((char**)(_symmsg + 8)) = path;
    _symmsg[11] = bank;
    result = File_Command();
    _msemaoff();
    return result;
}
