#include <symbos.h>
#include "deskmsg.h"

void _Desk_Service(char id) {
    _symmsg[0] = 48;
    _symmsg[1] = id;
    _Desk_Msg();
    while (_symmsg[0] != 163 && _symmsg[1] != id)
        _Desk_Wait();
}
