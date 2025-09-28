#include <symbos.h>
#include "kernmsg.h"

signed char Proc_Add(unsigned char bank, void* header, unsigned char priority) {
    signed char result;
    _msemaon();
    _symmsg[0] = 1;
    *((char**)(_symmsg + 1)) = header;
    _symmsg[3] = priority;
    _symmsg[4] = bank;
    _Kern_MsgWait();
    if (_symmsg[1]) {
        _msemaoff();
        return -1;
    }
    result = _symmsg[2];
    _msemaoff();
    return result;
}

void Proc_Delete(unsigned char pid) {
    _msemaon();
    _symmsg[0] = 2;
    _symmsg[1] = pid;
    _Kern_MsgWait();
    _msemaoff();
}
