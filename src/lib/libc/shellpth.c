#include <symbos.h>
#include "shellmsg.h"

void Shell_PathAdd(unsigned char bank, char* path, char* addition, char* dest) {
    if (_shellpid) {
        _msemaon();
        _symmsg[0] = 69;
        *((char**)(_symmsg + 1)) = path;
        *((char**)(_symmsg + 3)) = addition;
        *((char**)(_symmsg + 5)) = dest;
        _symmsg[7] = bank;
        _Shell_MsgWait();
        _msemaoff();
    }
    _shellerr = ERR_NOSHELL;
}
