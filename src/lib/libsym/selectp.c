#include <symbos.h>
#include "deskmsg.h"

char Select_Pos(unsigned short* x, unsigned short* y, unsigned short w, unsigned short h) {
    _msemaon();
    _symmsg[0] = 56;
    *((unsigned short*)(_symmsg + 2)) = *x;
    *((unsigned short*)(_symmsg + 4)) = *y;
    *((unsigned short*)(_symmsg + 6)) = w;
    *((unsigned short*)(_symmsg + 8)) = h;
    _Desk_Msg();
    Msg_Wait(_msgpid(), 2, _symmsg, 172); // note: not +128, so we can't use Msg_Respond()
    if (_symmsg[1] == 0) {
        _msemaoff();
        return 0;
    }
    *x = *((short*)(_symmsg + 2));
    *y = *((short*)(_symmsg + 4));
    _msemaoff();
    return 1;
}
