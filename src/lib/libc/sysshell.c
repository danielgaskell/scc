#include <symbos.h>

/* ========================================================================== */
/* SymShell system calls                                                      */
/* ========================================================================== */

void _Shell_MsgWait(void) {
    unsigned char response = _symmsg[0] + 128;
    Msg_Send(_sympid, _shellpid, _symmsg);
    while (_symmsg[0] != response)
        Msg_Sleep(_sympid, _shellpid, _symmsg);
    if (_symmsg[3] > 1)
        _shellerr = _symmsg[3] + 16;
    else
        _shellerr = 0;
}

int Shell_CharIn(unsigned char channel) {
    if (_shellpid) {
        _symmsg[0] = 64;
        _symmsg[1] = channel;
        _Shell_MsgWait();
        if (_symmsg[3] > 1) // error
            return -2;
        if (_symmsg[1])     // EOF
            return -1;
        return _symmsg[2];  // normal char
    }
    _shellerr = ERR_NOSHELL;
    return -2;
}

signed char Shell_StringIn(unsigned char channel, unsigned char bank, char* addr) {
    if (_shellpid) {
        _symmsg[0] = 65;
        _symmsg[1] = channel;
        _symmsg[2] = bank;
        *((char**)(_symmsg + 3)) = addr;
        _Shell_MsgWait();
        if (_symmsg[3] > 1) // error
            return -2;
        if (_symmsg[1])     // EOF
            return -1;
        return 0;           // normal string
    }
    _shellerr = ERR_NOSHELL;
    return -2;
}

signed char Shell_CharOut(unsigned char channel, unsigned char val) {
    if (_shellpid) {
        _symmsg[0] = 66;
        _symmsg[1] = channel;
        _symmsg[2] = val;
        _Shell_MsgWait();
        if (_symmsg[3] != 1)
            return -2;
        return 0;
    }
    _shellerr = ERR_NOSHELL;
    return -2;
}

signed char Shell_StringOut(unsigned char channel, unsigned char bank, char* addr, unsigned char len) {
    if (_shellpid) {
        _symmsg[0] = 67;
        _symmsg[1] = channel;
        _symmsg[2] = bank;
        *((char**)(_symmsg + 3)) = addr;
        _symmsg[5] = len;
        _Shell_MsgWait();
        if (_symmsg[3] != 1)
            return -2;
        return 0;
    }
    _shellerr = ERR_NOSHELL;
    return -2;
}

void Shell_Exit(unsigned char type) {
    if (_shellpid) {
        _symmsg[0] = 68;
        _symmsg[1] = type;
        Msg_Send(_sympid, _shellpid, _symmsg);
    }
    _shellerr = ERR_NOSHELL;
}

void Shell_PathAdd(unsigned char bank, char* path, char* addition, char* dest) {
    if (_shellpid) {
        _symmsg[0] = 69;
        *((char**)(_symmsg + 1)) = path;
        *((char**)(_symmsg + 3)) = addition;
        *((char**)(_symmsg + 5)) = dest;
        _symmsg[7] = bank;
        _Shell_MsgWait();
    }
    _shellerr = ERR_NOSHELL;
}

int Shell_CharTest(unsigned char channel, unsigned char lookahead) {
    if (_shellpid) {
        if (_shellver >= 23) { // requires SymShell 2.3 or greater
            _symmsg[0] = 70;
            _symmsg[1] = channel;
            _symmsg[2] = lookahead;
            _Shell_MsgWait();
            if (_symmsg[3] > 1)   // error
                return -2;
            if (_symmsg[1] == 2)  // normal char
                return _symmsg[2];
        }
        return 0;                 // else no char
    }
    _shellerr = ERR_NOSHELL;
    return -2;
}
