#include <symbos.h>
#include "kernmsg.h"

unsigned char Counter_Add(unsigned char bank, char* addr, unsigned char pid, unsigned char speed) {
    unsigned char result;
    _msemaon();
    _symmsg[0] = 7;
    *((char**)(_symmsg + 1)) = addr;
    _symmsg[3] = bank;
    _symmsg[4] = pid;
    _symmsg[5] = speed;
    _Kern_MsgWait();
    result = _symmsg[1];
    _msemaoff();
    return result;
}

void Counter_Delete(unsigned char bank, char* addr) {
    _msemaon();
    _symmsg[0] = 8;
    *((char**)(_symmsg + 1)) = addr;
    _symmsg[3] = bank;
    _Kern_MsgWait();
    _msemaoff();
}

void Counter_Clear(unsigned char pid) {
    _msemaon();
    _symmsg[0] = 9;
    _symmsg[1] = pid;
    _Kern_MsgWait();
    _msemaoff();
}
