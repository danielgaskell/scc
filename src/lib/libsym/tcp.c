#include <symbos.h>
#include "network.h"

/* ========================================================================== */
/* SymbOS Network Daemon calls (TCP)                                          */
/* ========================================================================== */

signed char TCP_OpenClient(unsigned long ip, signed short lport, unsigned short rport) {
    unsigned char result;
    _msemaon();
    _symmsg[0] = 16;
    _symmsg[3] = 0;
    *((unsigned short*)(_symmsg + 6)) = rport;
    *((unsigned short*)(_symmsg + 8)) = lport;
    *((unsigned long*)(_symmsg + 10)) = ip;
    result = Net_Command();
    if (result) {
        _msemaoff();
        return -1;
    }
    result = _symmsg[3]; // connection handle
    _msemaoff();
    return result;
}

signed char TCP_OpenServer(unsigned short lport) {
    unsigned char result;
    _msemaon();
    _symmsg[0] = 16;
    _symmsg[3] = 1;
    *((unsigned short*)(_symmsg + 8)) = lport;
    result = Net_Command();
    if (result) {
        _msemaoff();
        return -1;
    }
    result = _symmsg[3]; // connection handle
    _msemaoff();
    return result;
}

signed char TCP_Close(unsigned char handle) {
    signed char result;
    _msemaon();
    _symmsg[0] = 17;
    _symmsg[3] = handle;
    result = Net_SCommand();
    _msemaoff();
    return result;
}

extern char _symmsgsema;

void TCP_Event(NetStat* obj) {
    unsigned char result;
    obj->bytesrec = *((unsigned short*)(_symmsg + 4));
    obj->rport = *((unsigned short*)(_symmsg + 6));
    obj->status = _symmsg[8] & 0x1F;
    obj->datarec = _symmsg[8] >> 7;
    obj->ip = *((unsigned long*)(_symmsg + 10));
    return;
}

signed char TCP_Status(unsigned char handle, NetStat* obj) {
    unsigned char result;
    _msemaon();
    _symmsg[0] = 18;
    _symmsg[3] = handle;
    result = Net_Command();
    if (result) {
        _msemaoff();
        return -1;
    }
    TCP_Event(obj);
    _msemaoff();
    return 0;
}

signed char TCP_Receive(unsigned char handle, unsigned char bank, char* addr, unsigned short len, TCP_Trans* obj) {
    unsigned char result;
    _msemaon();
    _symmsg[0] = 19;
    _symmsg[3] = handle;
    *((unsigned short*)(_symmsg + 4)) = len;
    _symmsg[6] = bank;
    *((unsigned short*)(_symmsg + 8)) = (unsigned short)addr;
    result = Net_Command();
    if (result) {
        _msemaoff();
        return -1;
    }
    if (obj != 0) {
        obj->transferred = *((unsigned short*)(_symmsg + 4));
        obj->remaining = *((unsigned short*)(_symmsg + 8));
    }
    _msemaoff();
    return 0;
}

signed char TCP_Send(unsigned char handle, unsigned char bank, char* addr, unsigned short len, TCP_Trans* obj) {
    unsigned char result;
    _msemaon();
    _symmsg[0] = 20;
    _symmsg[3] = handle;
    *((unsigned short*)(_symmsg + 4)) = len;
    _symmsg[6] = bank;
    *((unsigned short*)(_symmsg + 8)) = (unsigned short)addr;
    result = Net_Command();
    if (result) {
        _msemaoff();
        return -1;
    }
    if (obj != 0) {
        obj->transferred = *((unsigned short*)(_symmsg + 4));
        obj->remaining = *((unsigned short*)(_symmsg + 8));
    }
    _msemaoff();
    return 0;
}

signed char TCP_Skip(unsigned char handle, unsigned short len) {
    signed char result;
    _msemaon();
    _symmsg[0] = 21;
    _symmsg[3] = handle;
    *((unsigned short*)(_symmsg + 4)) = len;
    result = Net_SCommand();
    _msemaoff();
    return result;
}

signed char TCP_Flush(unsigned char handle) {
    signed char result;
    _msemaon();
    _symmsg[0] = 22;
    _symmsg[3] = handle;
    result = Net_SCommand();
    _msemaoff();
    return result;
}

signed char TCP_Disconnect(unsigned char handle) {
    signed char result;
    _msemaon();
    _symmsg[0] = 23;
    _symmsg[3] = handle;
    result = Net_SCommand();
    _msemaoff();
    return result;
}

