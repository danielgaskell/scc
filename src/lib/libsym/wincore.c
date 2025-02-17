#include <symbos.h>
#include "deskmsg.h"

void _setpid(unsigned char bank, void* addr) {
    char* ptr = (char*)Bank_ReadWord(bank, (char*)addr);
    if (ptr)
        Bank_WriteByte(bank, &((Ctrl_Group*)ptr)->pid, _msgpid());
}

signed char Win_Open(unsigned char bank, void* addr) {
    signed char result;
    Bank_WriteByte(bank, &((Window*)addr)->pid, _msgpid());
    _setpid(bank, &((Window*)addr)->controls);
    _setpid(bank, &((Window*)addr)->toolbar);
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
