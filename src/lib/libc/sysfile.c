#include <symbos.h>

unsigned char _fileerr;

/* ========================================================================== */
/* SymbOS File Manager calls                                                  */
/* ========================================================================== */
unsigned char File_Command(void) {
    _symmsg[0] = 26;
    while (Msg_Send(_msgpid(), 3, _symmsg) == 0);
    while (_symmsg[0] != 154) {
        Idle();
        Msg_Receive(_msgpid(), 3, _symmsg);
    }
    if (_symmsg[2] & 0x01) {
        _fileerr = _symmsg[3] + 16; // note: +16 from documented errors!
        return _fileerr;
    } else {
        _fileerr = 0;
        return 0;
    }
}

unsigned char File_New(unsigned char bank, char* path, unsigned char attrib) {
    unsigned char result;
    _msemaon();
    _symmsg[1] = 17;
    _symmsg[3] = attrib;
    *((char**)(_symmsg + 8)) = path;
    _symmsg[11] = bank;
    result = File_Command();
    if (result) {
        _msemaoff();
        return result;
    }
    result = _symmsg[3]; // file ID
    _msemaoff();
    return result;
}

unsigned char File_Open(unsigned char bank, char* path) {
    unsigned char result;
    _msemaon();
    _symmsg[1] = 18;
    *((char**)(_symmsg + 8)) = path;
    _symmsg[11] = bank;
    result = File_Command();
    if (result) {
        _msemaoff();
        return result;
    }
    result = _symmsg[3]; // file ID
    _msemaoff();
    return result;
}

unsigned char File_Close(unsigned char id) {
    unsigned char result;
    _msemaon();
    _symmsg[1] = 19;
    _symmsg[3] = id;
    result = File_Command();
    _msemaoff();
    return result;
}

unsigned short _File_Read(unsigned char id, unsigned char bank, char* addr, unsigned short len, unsigned char compressed) {
    unsigned short result;
    _msemaon();
    _symmsg[1] = compressed ? 26 : 20;
    _symmsg[2] = compressed;
    _symmsg[3] = id;
    *((unsigned short*)(_symmsg + 4)) = len;
    _symmsg[6] = bank;
    *((char**)(_symmsg + 8)) = addr;
    if (File_Command() == 0) {
        result = *((unsigned short*)(_symmsg + 4));
        _msemaoff();
        return result;
    }
    _msemaoff();
    return 0;
}

unsigned short File_Read(unsigned char id, unsigned char bank, char* addr, unsigned short len) {
    return _File_Read(id, bank, addr, len, 0);
}

unsigned short File_ReadComp(unsigned char id, unsigned char bank, char* addr, unsigned short len) {
    return _File_Read(id, bank, addr, len, 1);
}

unsigned char File_ReadLine(unsigned char id, unsigned char bank, char* addr) {
    unsigned char result;
    _msemaon();
    _symmsg[1] = 20;
    _symmsg[3] = id;
    _symmsg[6] = bank;
    *((char**)(_symmsg + 8)) = addr;
    if (File_Command() == 0) {
        result = _symmsg[6];
        _msemaoff();
        return result;
    }
    _msemaoff();
    return 0;
}

unsigned short File_Write(unsigned char id, unsigned char bank, char* addr, unsigned short len) {
    unsigned short result;
    _msemaon();
    _symmsg[1] = 21;
    _symmsg[3] = id;
    *((unsigned short*)(_symmsg + 4)) = len;
    _symmsg[6] = bank;
    *((char**)(_symmsg + 8)) = addr;
    if (File_Command() == 0) {
        result = *((unsigned short*)(_symmsg + 4));
        _msemaoff();
        return result;
    }
    _msemaoff();
    return 0;
}

long File_Seek(unsigned char id, long offset, unsigned char ref) {
    long result;
    _msemaon();
    _symmsg[1] = 22;
    _symmsg[3] = id;
    _symmsg[4] = ref;
    *((long*)(_symmsg + 10)) = offset;
    if (File_Command() == 0) {
        result = *((long*)(_symmsg + 10));
        _msemaoff();
        return result;
    }
    _msemaoff();
    return -1;
}
