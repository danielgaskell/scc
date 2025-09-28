#include <symbos.h>
#include "deskmsg.h"

void Menu_Context(unsigned char bank, void* addr, int x, int y) {
    int result;
    _msemaon();
    _symmsg[0] = 52;
    _symmsg[1] = bank;
    *((void**)(_symmsg + 2)) = addr;
    *((int*)(_symmsg + 4)) = x;
    *((int*)(_symmsg + 6)) = y;
    _Desk_Msg();
    _msemaoff();
}
