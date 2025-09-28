#include <symbos.h>
#include "deskmsg.h"

void Win_Redraw_Sub(unsigned char winID, unsigned char collection, unsigned char what, unsigned char first) {
    _msemaon();
    _symmsg[0] = 51;
    _symmsg[1] = winID;
    _symmsg[2] = collection;
    _symmsg[3] = what;
    _symmsg[4] = first;
    _Desk_Msg();
    _msemaoff();
}
