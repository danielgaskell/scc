#include <symbos.h>
#include <string.h>
#include "shellmsg.h"

/* ========================================================================== */
/* SymShell system calls 1                                                    */
/* ========================================================================== */

int Shell_CharIn(unsigned char channel) {
    int result;
    if (_shellpid) {
        _msemaon();
        _symmsg[0] = 64;
        _symmsg[1] = channel;
        _Shell_MsgWait();
        if (_symmsg[3] > 1) { // error
            _msemaoff();
            return -2;
        }
        if (_symmsg[1]) {     // EOF
            _msemaoff();
            return -1;
        }
        result = _symmsg[2];  // normal char
        _msemaoff();
        return result;
    }
    _shellerr = ERR_NOSHELL;
    return -2;
}

signed char Shell_StringIn(unsigned char channel, unsigned char bank, char* addr) {
    if (_shellpid) {
        _msemaon();
        _symmsg[0] = 65;
        _symmsg[1] = channel;
        _symmsg[2] = bank;
        *((char**)(_symmsg + 3)) = addr;
        _Shell_MsgWait();
        if (_symmsg[3] > 1) { // error
            _msemaoff();
            return -2;
        }
        if (_symmsg[1]) {     // EOF
            _msemaoff();
            return -1;
        }
        _msemaoff();
        return 0;           // normal string
    }
    _shellerr = ERR_NOSHELL;
    return -2;
}

signed char Shell_CharOut(unsigned char channel, unsigned char val) {
    if (_shellpid) {
        _msemaon();
        _symmsg[0] = 66;
        _symmsg[1] = channel;
        _symmsg[2] = val;
        _Shell_MsgWait();
        if (_symmsg[3] != 1) {
            _msemaoff();
            return -2;
        }
        _msemaoff();
        return 0;
    }
    _shellerr = ERR_NOSHELL;
    return -2;
}

signed char Shell_StringOut(unsigned char channel, unsigned char bank, char* addr, unsigned char len) {
    if (_shellpid) {
        _msemaon();
        _symmsg[0] = 67;
        _symmsg[1] = channel;
        _symmsg[2] = bank;
        *((char**)(_symmsg + 3)) = addr;
        _symmsg[5] = len;
        _Shell_MsgWait();
        if (_symmsg[3] != 1) {
            _msemaoff();
            return -2;
        }
        _msemaoff();
        return 0;
    }
    _shellerr = ERR_NOSHELL;
    return -2;
}

signed char Shell_Print(char* addr) {
    return Shell_StringOut(0, _symbank, addr, strlen(addr));
}

signed char Shell_Locate(unsigned char col, unsigned char row) {
    char buf[4];
    buf[0] = 0x1F;
    buf[1] = col;
    buf[2] = row;
    buf[3] = 0;
    return Shell_Print(buf);
}

void Shell_Exit(unsigned char type) {
    if (_shellpid) {
        _msemaon();
        _symmsg[0] = 68;
        _symmsg[1] = type;
        while(Msg_Send(_msgpid(), _shellpid, _symmsg) == 0);
        _msemaoff();
    }
    _shellerr = ERR_NOSHELL;
}
