#include <symbos.h>
#include "deskmsg.h"

void Win_Redraw_Title(unsigned char winID) {
    _msemaon();
    _symmsg[0] = 36;
    _symmsg[1] = winID;
    _Desk_Msg();
    _msemaoff();
}
