#include <symbos.h>
#include <network.h>
#include "network.h"

/* ========================================================================== */
/* SymbOS Network Daemon calls (TCP)                                          */
/* ========================================================================== */

// note that the RSF3 M4 implementation only accepts at most 1400-byte frames;
// older versions of the M4 daemon do this wrong, so we impose this limit here.
_transfer char _netpacket[1400];

signed char TCP_Close(unsigned char handle) {
    signed char result;
    _nsemaon();
    _netmsg[0] = 17;
    _netmsg[3] = handle;
    result = Net_SCommand();
    _nsemaoff();
    return result;
}

signed char TCP_OpenWait(signed char handle) {
    unsigned char status;
    while (!Net_Wait(159)) {
        status = _netmsg[8] & 0x1F;
        if (status == TCP_OPENED) {
            _nsemaoff();
            return handle;
        } else if (status == TCP_CLOSING || status == TCP_CLOSED) {
            break;
        }
        Msg_Send(_netpid, _msgpid(), _netmsg); // neither an open nor close message, put back on queue
    }
    TCP_Close(handle);
    _neterr = ERR_CONNECT;
    return -1;
}

signed char TCP_OpenClient(char* ip, signed short lport, unsigned short rport) {
    _nsemaon();
    _netmsg[0] = 16;
    _netmsg[3] = 0;
    *((unsigned short*)(_netmsg + 6)) = rport;
    *((unsigned short*)(_netmsg + 8)) = lport;
    _netmsg[10] = ip[0];
    _netmsg[11] = ip[1];
    _netmsg[12] = ip[2];
    _netmsg[13] = ip[3];
    if (Net_Command()) {
        _nsemaoff();
        return -1;
    }
    return TCP_OpenWait(_netmsg[3]);
}

signed char TCP_OpenServer(unsigned short lport) {
    _nsemaon();
    _netmsg[0] = 16;
    _netmsg[3] = 1;
    *((unsigned short*)(_netmsg + 8)) = lport;
    if (Net_Command()) {
        _nsemaoff();
        return -1;
    }
    return TCP_OpenWait(_netmsg[3]);
}

void TCP_Event(char* msg, NetStat* obj) {
    obj->bytesrec = *((unsigned short*)(msg + 4));
    obj->rport = *((unsigned short*)(msg + 6));
    obj->status = msg[8] & 0x1F;
    obj->datarec = msg[8] >> 7;
    obj->ip[0] = msg[10];
    obj->ip[1] = msg[11];
    obj->ip[2] = msg[12];
    obj->ip[3] = msg[13];
    return;
}

signed char TCP_Status(unsigned char handle, NetStat* obj) {
    _nsemaon();
    _netmsg[0] = 18;
    _netmsg[3] = handle;
    if (Net_Command()) {
        _nsemaoff();
        return -1;
    }
    TCP_Event(_netmsg, obj);
    _nsemaoff();
    return 0;
}

signed char TCP_Receive(unsigned char handle, unsigned char bank, char* addr, unsigned short len, TCP_Trans* obj) {
    _nsemaon();
    _netmsg[0] = 19;
    _netmsg[3] = handle;
    *((unsigned short*)(_netmsg + 4)) = len;
    _netmsg[6] = bank;
    *((unsigned short*)(_netmsg + 8)) = (unsigned short)addr;
    if (Net_Command()) {
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
    unsigned short transferred;
    unsigned short packetlen;
    _nsemaon();
    for (;;) {
        _netmsg[0] = 20;
        _netmsg[3] = handle;
        packetlen = len > sizeof(_netpacket) ? sizeof(_netpacket) : len;
        if (bank != _symbank || addr != _netpacket)
            Bank_Copy(_symbank, _netpacket, bank, addr, packetlen);
        *((unsigned short*)(_netmsg + 4)) = packetlen;
        _netmsg[6] = _symbank;
        *((unsigned short*)(_netmsg + 8)) = (unsigned short)_netpacket;
        if (Net_Command()) {
            _nsemaoff();
            return -1;
        }
        transferred = *((unsigned short*)(_netmsg + 4));
        addr += transferred;
        len -= transferred;
        if (len <= 0)
            break;
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

