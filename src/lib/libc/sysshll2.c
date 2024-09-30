#include <symbos.h>
#include <string.h>
#include "shellmsg.h"

/* ========================================================================== */
/* SymShell system calls 2                                                    */
/* ========================================================================== */

int Shell_CharTest(unsigned char channel, unsigned char lookahead) {
    if (_shellpid) {
        if (_shellver >= 23) { // requires SymShell 2.3 or greater
            _msemaon();
            _symmsg[0] = 70;
            _symmsg[1] = channel;
            _symmsg[2] = lookahead;
            _Shell_MsgWait();
            if (_symmsg[3] > 1) {  // error
                _msemaoff();
                return -2;
            }
            if (_symmsg[1] == 2) { // normal char
                _msemaoff();
                return _symmsg[2];
            }
            _msemaoff();
        }
        return 0;                 // else no char
    }
    _shellerr = ERR_NOSHELL;
    return -2;
}

signed char _Shell_Watch(unsigned char bank, char* addr, unsigned char mode) {
    if (_shellpid) {
        if (_shellver >= 23) { // requires SymShell 2.3 or greater
            _msemaon();
            _symmsg[0] = 71;
            _symmsg[1] = mode;
            _symmsg[2] = bank;
            *((char**)(_symmsg + 3)) = addr;
            _Shell_MsgWait();
            if (_symmsg[3] == 252) {
                _msemaoff();
                _shellerr = ERR_RINGFULL;
                return -2;
            }
            _msemaoff();
            return 0;
        } else {
            _shellerr = ERR_RINGFULL;
            return -2;
        }
    }
    _shellerr = ERR_NOSHELL;
    return -2;
}

signed char Shell_CharWatch(unsigned char bank, char* addr) {
    return _Shell_Watch(bank, addr, 1);
}

signed char Shell_StopWatch(unsigned char bank, char* addr) {
    return _Shell_Watch(bank, addr, 0);
}
