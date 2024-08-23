#include <symbos.h>

/* ========================================================================== */
/* SymShell                                                                   */
/* ========================================================================== */

void _Shell_MsgWait(void)  {
    unsigned char response = _symmsg[0] + 128;
    Msg_Send(_sympid, _shellpid, _symmsg);
    while (_symmsg[0] != response)
        Msg_Sleep(_sympid, _shellpid, _symmsg);
    if (_symmsg[3] != 1)
        _shellerr = _symmsg[3] + 16;
    else
        _shellerr = 0;
}

unsigned char Shell_CharIn(unsigned char channel) {
    _symmsg[0] = 64;
    _symmsg[1] = channel;
    _Shell_MsgWait();
    if (_symmsg[1])
        return 0;
    return _symmsg[2];
}

unsigned char Shell_StringIn(unsigned char channel, unsigned char bank, char* addr) {
    _symmsg[0] = 65;
    _symmsg[1] = channel;
    _symmsg[2] = bank;
    *((char**)(_symmsg + 3)) = addr;
    _Shell_MsgWait();
    return _symmsg[1];
}

unsigned char Shell_CharOut(unsigned char channel, unsigned char val) {
    _symmsg[0] = 66;
    _symmsg[1] = channel;
    _symmsg[2] = val;
    _Shell_MsgWait();
    if (_symmsg[3] != 1)
        return 1;
    return 0;
}

unsigned char Shell_StringOut(unsigned char channel, unsigned char bank, char* addr, unsigned short len) {
    _symmsg[0] = 67;
    _symmsg[1] = channel;
    _symmsg[2] = bank;
    *((char**)(_symmsg + 3)) = addr;
    *((unsigned short*)(_symmsg + 5)) = len;
    _Shell_MsgWait();
    if (_symmsg[3] != 1)
        return 1;
    return 0;
}

void Shell_Exit(unsigned char type) {
    _symmsg[0] = 68;
    _symmsg[1] = type;
    Msg_Send(_sympid, _shellpid, _symmsg);
}

void Shell_PathAdd(unsigned char bank, char* path, char* addition, char* dest) {
    _symmsg[0] = 69;
    *((char**)(_symmsg + 1)) = path;
    *((char**)(_symmsg + 3)) = addition;
    *((char**)(_symmsg + 5)) = dest;
    _symmsg[7] = bank;
    _Shell_MsgWait();
}

unsigned char Shell_CharTest(unsigned char lookahead) {
    _symmsg[0] = 70;
    _symmsg[1] = 1;
    _symmsg[2] = lookahead;
    _Shell_MsgWait();
    if (_symmsg[3] != 1)
        return 0;
    if (_symmsg[1] == 2)
        return _symmsg[2];
    return 0;
}

