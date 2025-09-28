#include <symbos.h>
#include "deskmsg.h"

void Win_Redraw_Menu(unsigned char winID) {
    _msemaon();
    _symmsg[0] = 33;
    _symmsg[1] = winID;
    _Desk_Msg();
    _msemaoff();
}

