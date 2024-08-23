#include <symbos.h>

unsigned char _fileerr;

/* ========================================================================== */
/* File Manager                                                               */
/* ========================================================================== */
unsigned char File_Command(void) {
    _symmsg[0] = 26;
    Msg_Send (_sympid, 3, _symmsg);
    while (_symmsg[0] != 154)
        Msg_Sleep(_sympid, 3, _symmsg);
    if (_symmsg[2] & 0x01) {
        _fileerr = _symmsg[3] + 16; // note: +16 from documented errors!
        return _fileerr;
    } else {
        _fileerr = 0;
        return 0;
    }
}

unsigned char File_New(unsigned char bank, char* path, unsigned char attrib) {
    _symmsg[1] = 17;
    _symmsg[3] = attrib;
    *((char**)(_symmsg + 8)) = path;
    _symmsg[11] = bank;
    return File_Command();
}

unsigned char File_Open(unsigned char bank, char* path) {
    unsigned char result;
    _symmsg[1] = 18;
    *((char**)(_symmsg + 8)) = path;
    _symmsg[11] = bank;
    result = File_Command();
    if (result)
        return result;
    return _symmsg[2]; // file ID
}

unsigned char File_Close(unsigned char id) {
    _symmsg[1] = 19;
    _symmsg[3] = id;
    return File_Command();
}

unsigned short File_Read(unsigned char id, unsigned char bank, char* addr, unsigned short len) {
    _symmsg[1] = 20;
    _symmsg[3] = id;
    *((unsigned short*)(_symmsg + 4)) = len;
    _symmsg[6] = bank;
    *((char**)(_symmsg + 8)) = addr;
    if (File_Command() == 0)
        return *((unsigned short*)(_symmsg + 4));
    return 0;
}

unsigned char File_ReadLine(unsigned char id, unsigned char bank, char* addr) {
    _symmsg[1] = 20;
    _symmsg[3] = id;
    _symmsg[6] = bank;
    *((char**)(_symmsg + 8)) = addr;
    if (File_Command() == 0)
        return _symmsg[6];
    return 0;
}

unsigned short File_Write(unsigned char id, unsigned char bank, char* addr, unsigned short len) {
    _symmsg[1] = 21;
    _symmsg[3] = id;
    *((unsigned short*)(_symmsg + 4)) = len;
    _symmsg[6] = bank;
    *((char**)(_symmsg + 8)) = addr;
    if (File_Command() == 0)
        return *((unsigned short*)(_symmsg + 4));
    return 0;
}

long File_Seek(unsigned char id, long offset, unsigned char ref) {
    _symmsg[1] = 22;
    _symmsg[3] = id;
    _symmsg[4] = ref;
    *((long*)(_symmsg + 10)) = offset;
    if (File_Command() == 0)
        return *((long*)(_symmsg + 10));
    return -1;
}
