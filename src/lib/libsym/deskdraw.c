#include <symbos.h>
#include "deskmsg.h"

void Desktop_Redraw(void) {
    _msemaon();
    _symmsg[0] = 48;
    _symmsg[1] = 10;
    _Desk_Msg();
    _msemaoff();
}
