#include <symbos.h>

/* ========================================================================== */
/* Kernel                                                                     */
/* ========================================================================== */
signed char _timer_wake = -1;

void _Kern_MsgWait(void) { Msg_Respond(_msgpid(), 1, _symmsg); }

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
