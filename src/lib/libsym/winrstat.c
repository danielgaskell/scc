#include <symbos.h>
#include "deskmsg.h"

void Win_Redraw_Status(unsigned char winID) {
    _msemaon();
    _symmsg[0] = 37;
    _symmsg[1] = winID;
    _Desk_Msg();
    _msemaoff();
}
