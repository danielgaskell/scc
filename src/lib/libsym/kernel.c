#include <symbos.h>

/* ========================================================================== */
/* Kernel                                                                     */
/* ========================================================================== */
void _Kern_MsgWait(void) {
    unsigned char response;
    response = _symmsg[0] + 128;
    Msg_Send(_sympid, 1, _symmsg);
    while (_symmsg[0] != response)
        Msg_Sleep(_sympid, 1, _symmsg);
}

char Timer_Add(unsigned char bank, char* stack) {
    _symmsg[0] = 3;
    *((char**)(_symmsg + 1)) = stack;
    _symmsg[4] = bank;
    _Kern_MsgWait();
    if (_symmsg[1] == 1)
        return -1;
    return _symmsg[1];
}

void Timer_Delete(unsigned char id) {
    _symmsg[0] = 4;
    _symmsg[1] = id;
    _Kern_MsgWait();
}

unsigned char Counter_Add(unsigned char bank, char* addr, unsigned char pid, unsigned char speed) {
    _symmsg[0] = 7;
    *((char**)(_symmsg + 1)) = addr;
    _symmsg[3] = bank;
    _symmsg[4] = pid;
    _symmsg[5] = speed;
    _Kern_MsgWait();
    return _symmsg[1];
}

void Counter_Delete(unsigned char bank, char* addr) {
    _symmsg[0] = 8;
    *((char**)(_symmsg + 1)) = addr;
    _symmsg[3] = bank;
    _Kern_MsgWait();
}

void Counter_Clear(unsigned char pid) {
    _symmsg[0] = 9;
    _symmsg[1] = pid;
    _Kern_MsgWait();
}

void Proc_Sleep(unsigned char pid) {
    _symmsg[0] = 5;
    _symmsg[1] = pid;
    _Kern_MsgWait();
}

void Proc_Wake(unsigned char pid) {
    _symmsg[0] = 6;
    _symmsg[1] = pid;
    _Kern_MsgWait();
}

void Proc_Priority(unsigned char pid, unsigned char priority) {
    _symmsg[0] = 10;
    _symmsg[1] = pid;
    _symmsg[2] = priority;
    _Kern_MsgWait();
}
