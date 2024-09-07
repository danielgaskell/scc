#include <symbos.h>

/* ========================================================================== */
/* File Manager                                                               */
/* ========================================================================== */
unsigned char Dir_SetAttrib(unsigned char bank, char* path, unsigned char attrib) {
    unsigned char result;
    _msemaon();
    _symmsg[1] = 34;
    _symmsg[3] = 0;
    _symmsg[4] = attrib;
    *((char**)(_symmsg + 8)) = path;
    _symmsg[11] = bank;
    result = File_Command();
    _msemaoff();
    return result;
}

signed char Dir_GetAttrib(unsigned char bank, char* path) {
    signed char result;
    _msemaon();
    _symmsg[1] = 35;
    _symmsg[3] = 0;
    *((char**)(_symmsg + 8)) = path;
    _symmsg[11] = bank;
    if (File_Command() == 0) {
        result = _symmsg[4];
        _msemaoff();
        return result;
    }
    _msemaoff();
    return -1;
}

unsigned long Dir_GetTime(unsigned char bank, char* path, unsigned char which) {
    unsigned long result;
    _msemaon();
    _symmsg[1] = 35;
    _symmsg[3] = which;
    *((char**)(_symmsg + 8)) = path;
    _symmsg[11] = bank;
    if (File_Command() == 0) {
        result = *(unsigned long*)&_symmsg[4];
        _msemaoff();
        return result;
    }
    _msemaoff();
    return 0;
}

unsigned char Dir_SetTime(unsigned char bank, char* path, unsigned char which, unsigned long timestamp) {
    unsigned char result;
    _msemaon();
    _symmsg[1] = 34;
    _symmsg[3] = which;
    *(unsigned long*)&_symmsg[4] = timestamp;
    *((char**)(_symmsg + 8)) = path;
    _symmsg[11] = bank;
    result = File_Command();
    _msemaoff();
    return result;
}

unsigned char Dir_Rename(unsigned char bank, char* path, char* newname) {
    unsigned char result;
    _msemaon();
    _symmsg[1] = 36;
    *((char**)(_symmsg + 6)) = newname;
    *((char**)(_symmsg + 8)) = path;
    _symmsg[11] = bank;
    result = File_Command();
    _msemaoff();
    return result;
}

unsigned char Dir_New(unsigned char bank, char* path) {
    unsigned char result;
    _msemaon();
    _symmsg[1] = 37;
    *((char**)(_symmsg + 8)) = path;
    _symmsg[11] = bank;
    result = File_Command();
    _msemaoff();
    return result;
}

int Dir_ReadRaw(unsigned char bank, char* path, unsigned char attrib, unsigned char bufbank, void* addr, unsigned short len, unsigned short skip) {
    int result;
    _msemaon();
    _symmsg[1] = 38;
    _symmsg[3] = bufbank;
    *((unsigned short*)(_symmsg + 4)) = len;
    *((char**)(_symmsg + 6)) = addr;
    *((char**)(_symmsg + 8)) = path;
    _symmsg[10] = attrib;
    _symmsg[11] = bank;
    *((unsigned short*)(_symmsg + 12)) = skip;
    result = File_Command();
    if (result == 0) {
        result = *((int*)(_symmsg + 8));
        _msemaoff();
        return result;
    }
    _msemaoff();
    return -1;
}

int Dir_ReadExt(unsigned char bank, char* path, unsigned char attrib, unsigned char bufbank, void* addr, unsigned short len, unsigned short skip, unsigned char cols) {
    int result;
    _msemaon();
    _symmsg[1] = 13;
    _symmsg[3] = (bank << 4) | bufbank;
    *((unsigned short*)(_symmsg + 4)) = len;
    *((char**)(_symmsg + 6)) = addr;
    *((char**)(_symmsg + 8)) = path;
    _symmsg[10] = attrib;
    _symmsg[11] = cols;
    *((unsigned short*)(_symmsg + 12)) = skip;
    result = File_Command();
    if (result == 0) {
        result = *((int*)(_symmsg + 8));
        _msemaoff();
        return result;
    }
    _msemaoff();
    return -1;
}

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

unsigned char Dir_DeleteDir(unsigned char bank, char* path) {
    unsigned char result;
    _msemaon();
    _symmsg[1] = 40;
    *((char**)(_symmsg + 8)) = path;
    _symmsg[11] = bank;
    result = File_Command();
    _msemaoff();
    return result;
}

unsigned char Dir_Move(unsigned char bank, char* pathSrc, char* pathDst) {
    unsigned char result;
    _msemaon();
    _symmsg[1] = 41;
    *((char**)(_symmsg + 6)) = pathDst;
    *((char**)(_symmsg + 8)) = pathSrc;
    _symmsg[11] = bank;
    result = File_Command();
    _msemaoff();
    return result;
}
