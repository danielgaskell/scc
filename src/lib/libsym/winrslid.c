#include <symbos.h>
#include "deskmsg.h"

void Win_Redraw_Slider(unsigned char winID) {
    _msemaon();
    _symmsg[0] = 49;
    _symmsg[1] = winID;
    _Desk_Msg();
    _msemaoff();
}
