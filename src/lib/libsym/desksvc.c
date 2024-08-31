#include <symbos.h>
#include "deskmsg.h"

void _Desk_Service(char id) {
    _symmsg[0] = 48;
    _symmsg[1] = id;
    _Desk_Msg();
    while (_symmsg[0] != 163 && _symmsg[1] != id)
        _Desk_Wait();
}

unsigned short Desk_GetColor(char color) {
    _symmsg[2] = color;
    _Desk_Service(3);
    return *((unsigned short*)(_symmsg + 3));
}
