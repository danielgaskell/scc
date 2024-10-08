#include <symbos.h>
#include "deskmsg.h"

signed char Systray_Add(unsigned char bank, char* addr, unsigned char code) {
    signed char result;
    _msemaon();
    _symmsg[0] = 53;
    _symmsg[1] = bank;
    *((char**)(_symmsg + 2)) = addr;
    _symmsg[4] = code;
    _Desk_Msg();
    while (_symmsg[0] != 169)
        _Desk_Wait();
    if (_symmsg[1] == 0) {
        result = _symmsg[2];
        _msemaoff();
        return result;
    }
    _msemaoff();
    return -1;
}

void Systray_Remove(unsigned char id) {
    _msemaon();
    _symmsg[0] = 54;
    _symmsg[1] = id;
    _Desk_Msg();
    _msemaoff();
}
