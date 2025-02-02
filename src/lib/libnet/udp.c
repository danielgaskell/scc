#include <symbos.h>
#include <network.h>
#include "network.h"

/* ========================================================================== */
/* SymbOS Network Daemon calls (UDP)                                          */
/* ========================================================================== */

signed char UDP_Open(unsigned char type, unsigned short lport, unsigned char bank) {
    unsigned char result;
    _nsemaon();
    _netmsg[0] = 32;
    _netmsg[3] = type;
    _netmsg[6] = bank;
    *((unsigned short*)(_netmsg + 8)) = lport;
    result = Net_Command();
    if (result) {
        _nsemaoff();
        return -1;
    }
    result = _netmsg[3];
    _nsemaoff();
    return result;
}

signed char UDP_Close(unsigned char handle) {
    unsigned char result;
    _nsemaon();
    _netmsg[0] = 33;
    _netmsg[3] = handle;
    result = Net_Command();
    _nsemaoff();
    if (result)
        return -1;
    return 0;
}

signed char UDP_Status(unsigned char handle, NetStat* obj) {
    unsigned char result;
    _nsemaon();
    _netmsg[0] = 34;
    _netmsg[3] = handle;
    result = Net_Command();
    if (result) {
        _nsemaoff();
        return -1;
    }
    obj->bytesrec = *((unsigned short*)(_netmsg + 4));
    obj->rport = *((unsigned short*)(_netmsg + 6));
    obj->status = _netmsg[8];
    obj->ip = *((unsigned long*)(_netmsg + 10));
    _nsemaoff();
    return 0;
}

signed char UDP_Receive(unsigned char handle, char* addr) {
    signed char result;
    _nsemaon();
    _netmsg[0] = 35;
    _netmsg[3] = handle;
    *((unsigned short*)(_netmsg + 8)) = (unsigned short)addr;
    result = Net_SCommand();
    _nsemaoff();
    return result;
}

signed char UDP_Send(unsigned char handle, char* addr, unsigned short len, unsigned long ip, unsigned short rport) {
    signed char result;
    _nsemaon();
    _netmsg[0] = 36;
    _netmsg[3] = handle;
    *((unsigned short*)(_netmsg + 4)) = len;
    *((unsigned short*)(_netmsg + 6)) = rport;
    *((unsigned short*)(_netmsg + 8)) = (unsigned short)addr;
    *((unsigned long*)(_netmsg + 10)) = ip;
    result = Net_SCommand();
    _nsemaoff();
    return result;
}

signed char UDP_Skip(unsigned char handle) {
    signed char result;
    _nsemaon();
    _netmsg[0] = 37;
    _netmsg[3] = handle;
    result = Net_SCommand();
    _nsemaoff();
    return result;
}
