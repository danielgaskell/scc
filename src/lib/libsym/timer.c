#include <symbos.h>
#include "kernmsg.h"

signed char _timer_wake = -1;

signed char Timer_Add(unsigned char bank, void* header) {
    signed char result;
    _msemaon();
    _symmsg[0] = 3;
    *((char**)(_symmsg + 1)) = header;
    _symmsg[4] = bank;
    _Kern_MsgWait();
    if (_symmsg[1] == 1) {
        _msemaoff();
        return -1;
    }
    result = _symmsg[2];
    _msemaoff();
    return result;
}

void Timer_Delete(unsigned char id) {
    _msemaon();
    _symmsg[0] = 4;
    _symmsg[1] = id;
    _Kern_MsgWait();
    _msemaoff();
    if (id == _timer_wake)
        _timer_wake = -1;
}

