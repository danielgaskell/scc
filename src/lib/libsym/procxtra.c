#include <symbos.h>
#include "kernmsg.h"

void Proc_Sleep(unsigned char pid) {
    _msemaon();
    _symmsg[0] = 5;
    _symmsg[1] = pid;
    _Kern_MsgWait();
    _msemaoff();
}

void Proc_Wake(unsigned char pid) {
    _msemaon();
    _symmsg[0] = 6;
    _symmsg[1] = pid;
    _Kern_MsgWait();
    _msemaoff();
}

void Proc_Priority(unsigned char pid, unsigned char priority) {
    _msemaon();
    _symmsg[0] = 10;
    _symmsg[1] = pid;
    _symmsg[2] = priority;
    _Kern_MsgWait();
    _msemaoff();
}
