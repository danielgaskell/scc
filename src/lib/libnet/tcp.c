#include <symbos.h>
#include <network.h>
#include "network.h"

/* ========================================================================== */
/* SymbOS Network Daemon calls (TCP)                                          */
/* ========================================================================== */

signed char TCP_OpenClient(unsigned long ip, signed short lport, unsigned short rport) {
    unsigned char result;
    _nsemaon();
    _netmsg[0] = 16;
    _netmsg[3] = 0;
    *((unsigned short*)(_netmsg + 6)) = rport;
    *((unsigned short*)(_netmsg + 8)) = lport;
    *((unsigned long*)(_netmsg + 10)) = ip;
    result = Net_Command();
    if (result) {
        _nsemaoff();
        return -1;
    }
    result = _netmsg[3]; // connection handle
    _nsemaoff();
    return result;
}

signed char TCP_OpenServer(unsigned short lport) {
    unsigned char result;
    _nsemaon();
    _netmsg[0] = 16;
    _netmsg[3] = 1;
    *((unsigned short*)(_netmsg + 8)) = lport;
    result = Net_Command();
    if (result) {
        _nsemaoff();
        return -1;
    }
    result = _netmsg[3]; // connection handle
    _nsemaoff();
    return result;
}

signed char TCP_Close(unsigned char handle) {
    signed char result;
    _nsemaon();
    _netmsg[0] = 17;
    _netmsg[3] = handle;
    result = Net_SCommand();
    _nsemaoff();
    return result;
}

extern char _netmsgsema;

void TCP_Event(NetStat* obj) {
    unsigned char result;
    obj->bytesrec = *((unsigned short*)(_netmsg + 4));
    obj->rport = *((unsigned short*)(_netmsg + 6));
    obj->status = _netmsg[8] & 0x1F;
    obj->datarec = _netmsg[8] >> 7;
    obj->ip = *((unsigned long*)(_netmsg + 10));
    return;
}

signed char TCP_Status(unsigned char handle, NetStat* obj) {
    unsigned char result;
    _nsemaon();
    _netmsg[0] = 18;
    _netmsg[3] = handle;
    result = Net_Command();
    if (result) {
        _nsemaoff();
        return -1;
    }
    TCP_Event(obj);
    _nsemaoff();
    return 0;
}

signed char TCP_Receive(unsigned char handle, unsigned char bank, char* addr, unsigned short len, TCP_Trans* obj) {
    unsigned char result;
    _nsemaon();
    _netmsg[0] = 19;
    _netmsg[3] = handle;
    *((unsigned short*)(_netmsg + 4)) = len;
    _netmsg[6] = bank;
    *((unsigned short*)(_netmsg + 8)) = (unsigned short)addr;
    result = Net_Command();
    if (result) {
        _nsemaoff();
        return -1;
    }
    if (obj != 0) {
        obj->transferred = *((unsigned short*)(_netmsg + 4));
        obj->remaining = *((unsigned short*)(_netmsg + 8));
    }
    _nsemaoff();
    return 0;
}

signed char TCP_Send(unsigned char handle, unsigned char bank, char* addr, unsigned short len) {
    unsigned char result;
    unsigned short transferred;
    unsigned short remaining;
    _nsemaon();
    for (;;) {
        _netmsg[0] = 20;
        _netmsg[3] = handle;
        *((unsigned short*)(_netmsg + 4)) = len;
        _netmsg[6] = bank;
        *((unsigned short*)(_netmsg + 8)) = (unsigned short)addr;
        result = Net_Command();
        if (result) {
            _nsemaoff();
            return -1;
        }
        transferred = *((unsigned short*)(_netmsg + 4));
        remaining = *((unsigned short*)(_netmsg + 8));
        if (remaining == 0)
            break;
        addr += transferred;
        len -= transferred;
    }
    _nsemaoff();
    return 0;
}

signed char TCP_Skip(unsigned char handle, unsigned short len) {
    signed char result;
    _nsemaon();
    _netmsg[0] = 21;
    _netmsg[3] = handle;
    *((unsigned short*)(_netmsg + 4)) = len;
    result = Net_SCommand();
    _nsemaoff();
    return result;
}

signed char TCP_Flush(unsigned char handle) {
    signed char result;
    _nsemaon();
    _netmsg[0] = 22;
    _netmsg[3] = handle;
    result = Net_SCommand();
    _nsemaoff();
    return result;
}

signed char TCP_Disconnect(unsigned char handle) {
    signed char result;
    _nsemaon();
    _netmsg[0] = 23;
    _netmsg[3] = handle;
    result = Net_SCommand();
    _nsemaoff();
    return result;
}

