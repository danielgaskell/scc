#include <symbos.h>
#include "deskmsg.h"

int Menu_Context(unsigned char bank, char* addr, int x, int y) {
    _symmsg[0] = 52;
    _symmsg[1] = bank;
    *((char**)(_symmsg + 2)) = addr;
    *((int*)(_symmsg + 4)) = x;
    *((int*)(_symmsg + 6)) = y;
    _Desk_Msg();
    while (_symmsg[0] != 168)
        _Desk_Wait();
    if (_symmsg[1] == 0)
        return -1;
    return *((short*)(_symmsg + 2));
}

char Select_Pos(unsigned short* x, unsigned short* y, unsigned short w, unsigned short h) {
    _symmsg[0] = 56;
    *((unsigned short*)(_symmsg + 2)) = *x;
    *((unsigned short*)(_symmsg + 4)) = *y;
    *((unsigned short*)(_symmsg + 6)) = w;
    *((unsigned short*)(_symmsg + 8)) = h;
    _Desk_Msg();
    while (_symmsg[0] != 172)
        _Desk_Wait();
    if (_symmsg[1] == 0)
        return 0;
    *x = *((short*)(_symmsg + 2));
    *y = *((short*)(_symmsg + 4));
    return 1;
}

char Select_Size(unsigned short x, unsigned short y, unsigned short* w, unsigned short* h) {
    _symmsg[0] = 57;
    *((unsigned short*)(_symmsg + 2)) = x;
    *((unsigned short*)(_symmsg + 4)) = y;
    *((unsigned short*)(_symmsg + 6)) = *w;
    *((unsigned short*)(_symmsg + 8)) = *h;
    _Desk_Msg();
    while (_symmsg[0] != 173)
        _Desk_Wait();
    if (_symmsg[1] == 0)
        return 0;
    *w = *((short*)(_symmsg + 2));
    *h = *((short*)(_symmsg + 4));
    return 1;
}
