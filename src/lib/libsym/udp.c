#include <symbos.h>
#include "network.h"

/* ========================================================================== */
/* SymbOS Network Daemon calls (UDP)                                          */
/* ========================================================================== */

signed char UDP_Open(unsigned char type, unsigned short lport, unsigned char bank) {
    unsigned char result;
    _msemaon();
    _symmsg[0] = 32;
    _symmsg[3] = type;
    _symmsg[6] = bank;
    *((unsigned short*)(_symmsg + 8)) = lport;
    result = Net_Command();
    if (result) {
        _msemaoff();
        return -1;
    }
    result = _symmsg[3];
    _msemaoff();
    return result;
}

signed char UDP_Close(unsigned char handle) {
    unsigned char result;
    _msemaon();
    _symmsg[0] = 33;
    _symmsg[3] = handle;
    result = Net_Command();
    _msemaoff();
    if (result)
        return -1;
    return 0;
}

signed char UDP_Status(unsigned char handle, NetStat* obj) {
    unsigned char result;
    _msemaon();
    _symmsg[0] = 34;
    _symmsg[3] = handle;
    result = Net_Command();
    if (result) {
        _msemaoff();
        return -1;
    }
    obj->bytesrec = *((unsigned short*)(_symmsg + 4));
    obj->rport = *((unsigned short*)(_symmsg + 6));
    obj->status = _symmsg[8];
    obj->ip = *((unsigned long*)(_symmsg + 10));
    _msemaoff();
    return 0;
}

signed char UDP_Receive(unsigned char handle, char* addr) {
    signed char result;
    _msemaon();
    _symmsg[0] = 35;
    _symmsg[3] = handle;
    *((unsigned short*)(_symmsg + 8)) = (unsigned short)addr;
    result = Net_SCommand();
    _msemaoff();
    return result;
}

signed char UDP_Send(unsigned char handle, char* addr, unsigned short len, unsigned long ip, unsigned short rport) {
    signed char result;
    _msemaon();
    _symmsg[0] = 36;
    _symmsg[3] = handle;
    *((unsigned short*)(_symmsg + 4)) = len;
    *((unsigned short*)(_symmsg + 6)) = rport;
    *((unsigned short*)(_symmsg + 8)) = (unsigned short)addr;
    *((unsigned long*)(_symmsg + 10)) = ip;
    result = Net_SCommand();
    _msemaoff();
    return result;
}

signed char UDP_Skip(unsigned char handle) {
    signed char result;
    _msemaon();
    _symmsg[0] = 37;
    _symmsg[3] = handle;
    result = Net_SCommand();
    _msemaoff();
    return result;
}
