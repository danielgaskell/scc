#include <symbos.h>
#include "deskmsg.h"

void Win_Redraw_Area(unsigned char winID, unsigned char what, unsigned char first,
                     unsigned short x, unsigned short y, unsigned short w, unsigned short h) {
    _msemaon();
    _symmsg[0] = 50;
    _symmsg[1] = winID;
    _symmsg[2] = what;
    _symmsg[3] = first;
    *((unsigned short*)(_symmsg + 4)) = x;
    *((unsigned short*)(_symmsg + 6)) = y;
    *((unsigned short*)(_symmsg + 8)) = w;
    *((unsigned short*)(_symmsg + 10)) = h;
    _Desk_Msg();
    _msemaoff();
}
