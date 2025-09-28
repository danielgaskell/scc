#include <symbos.h>
#include "deskmsg.h"

void Win_Redraw_Ext(unsigned char winID, unsigned char what, unsigned char first) {
    _msemaon();
    _symmsg[0] = 47;
    _symmsg[1] = winID;
    _symmsg[2] = what;
    _symmsg[3] = first;
    _Desk_Msg();
    _msemaoff();
}
