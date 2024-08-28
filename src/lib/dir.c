#include <symbos.h>

/* ========================================================================== */
/* File Manager                                                               */
/* ========================================================================== */
unsigned char Dir_SetAttrib(unsigned char bank, char* path, unsigned char attrib) {
    _symmsg[1] = 34;
    _symmsg[3] = 0;
    _symmsg[4] = attrib;
    *((char**)(_symmsg + 8)) = path;
    _symmsg[11] = bank;
    return File_Command();
}

signed char Dir_GetAttrib(unsigned char bank, char* path) {
    _symmsg[1] = 35;
    _symmsg[3] = 0;
    *((char**)(_symmsg + 8)) = path;
    _symmsg[11] = bank;
    if (File_Command() == 0)
        return _symmsg[4];
    return -1;
}

unsigned char Dir_Rename(unsigned char bank, char* path, char* newname) {
    _symmsg[1] = 36;
    *((char**)(_symmsg + 6)) = newname;
    *((char**)(_symmsg + 8)) = path;
    _symmsg[11] = bank;
    return File_Command();
}

unsigned char Dir_New(unsigned char bank, char* path) {
    _symmsg[1] = 37;
    *((char**)(_symmsg + 8)) = path;
    _symmsg[11] = bank;
    return File_Command();
}

int Dir_ReadRaw(unsigned char bank, char* path, unsigned char attrib, unsigned char bufbank, void* addr, unsigned short len, unsigned short skip) {
    unsigned char result;
    _symmsg[1] = 38;
    _symmsg[3] = bufbank;
    *((unsigned short*)(_symmsg + 4)) = len;
    *((char**)(_symmsg + 6)) = addr;
    *((char**)(_symmsg + 8)) = path;
    _symmsg[10] = attrib;
    _symmsg[11] = bank;
    *((unsigned short*)(_symmsg + 12)) = skip;
    result = File_Command();
    if (result == 0)
        return *((int*)(_symmsg + 8));
    return -1;
}

int Dir_ReadExt(unsigned char bank, char* path, unsigned char attrib, unsigned char bufbank, void* addr, unsigned short len, unsigned short skip, unsigned char cols) {
    unsigned char result;
    _symmsg[1] = 13;
    _symmsg[3] = (bank << 4) | bufbank;
    *((unsigned short*)(_symmsg + 4)) = len;
    *((char**)(_symmsg + 6)) = addr;
    *((char**)(_symmsg + 8)) = path;
    _symmsg[10] = attrib;
    _symmsg[11] = cols;
    *((unsigned short*)(_symmsg + 12)) = skip;
    result = File_Command();
    if (result == 0)
        return *((int*)(_symmsg + 8));
    return -1;
}

unsigned char Dir_Delete(unsigned char bank, char* path) {
    _symmsg[1] = 39;
    *((char**)(_symmsg + 8)) = path;
    _symmsg[11] = bank;
    return File_Command();
}

unsigned char Dir_DeleteDir(unsigned char bank, char* path) {
    _symmsg[1] = 40;
    *((char**)(_symmsg + 8)) = path;
    _symmsg[11] = bank;
    return File_Command();
}

unsigned char Dir_Move(unsigned char bank, char* pathSrc, char* pathDst) {
    _symmsg[1] = 41;
    *((char**)(_symmsg + 6)) = pathDst;
    *((char**)(_symmsg + 8)) = pathSrc;
    _symmsg[11] = bank;
    return File_Command();
}
