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

void UDP_Event(char* msg, NetStat* obj) {
    obj->bytesrec = *((unsigned short*)(msg + 4));
    obj->rport = *((unsigned short*)(msg + 6));
    obj->status = msg[8];
    obj->ip[0] = msg[10];
    obj->ip[1] = msg[11];
    obj->ip[2] = msg[12];
    obj->ip[3] = msg[13];
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
    UDP_Event(_netmsg, obj);
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

signed char UDP_Send(unsigned char handle, char* addr, unsigned short len, char* ip, unsigned short rport) {
    signed char result;
    _nsemaon();
    _netmsg[0] = 36;
    _netmsg[3] = handle;
    *((unsigned short*)(_netmsg + 4)) = len;
    *((unsigned short*)(_netmsg + 6)) = rport;
    *((unsigned short*)(_netmsg + 8)) = (unsigned short)addr;
    _netmsg[10] = ip[0];
    _netmsg[11] = ip[1];
    _netmsg[12] = ip[2];
    _netmsg[13] = ip[3];
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
