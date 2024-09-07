#include <symbos.h>
#include "deskmsg.h"

signed char Win_Open(unsigned char bank, void* addr) {
    signed char result;
    ((Window*)addr)->pid = _sympid;
    if (((Window*)addr)->controls != 0)
        ((Ctrl_Group*)(((Window*)addr)->controls))->pid = _sympid;
    if (((Window*)addr)->toolbar != 0)
        ((Ctrl_Group*)(((Window*)addr)->toolbar))->pid = _sympid;
    _msemaon();
    _symmsg[0] = 32;
    _symmsg[1] = bank;
    *((char**)(_symmsg + 2)) = addr;
    _Desk_Msg();
    while (1) {
        _Desk_Wait();
        if (_symmsg[0] == 160) { // failure: return -1
            _msemaoff();
            return -1;
        }
        if (_symmsg[0] == 161) { // success: return window ID
            _msemaoff();
            result = _symmsg[4];
            return result;
        }
    }
}

void Win_Redraw(unsigned char winID, signed char what, unsigned char first) {
    _msemaon();
    _symmsg[0] = 34;
    _symmsg[1] = winID;
    _symmsg[2] = what;
    _symmsg[3] = first;
    _Desk_Msg();
    _msemaoff();
}

void Win_Close(unsigned char winID) {
    _msemaon();
    _symmsg[0] = 46;
    _symmsg[1] = winID;
    _Desk_Msg();
    _msemaoff();
}
