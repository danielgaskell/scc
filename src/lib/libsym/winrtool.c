#include <symbos.h>
#include "deskmsg.h"

void Win_Redraw_Toolbar(unsigned char winID, signed char what, unsigned char first) {
    _msemaon();
    _symmsg[0] = 35;
    _symmsg[1] = winID;
    _symmsg[2] = what;
    _symmsg[3] = first;
    _Desk_Msg();
    _msemaoff();
}
