#include <symbos.h>
#include <network.h>
#include "network.h"

/* ========================================================================== */
/* SymbOS Network Daemon calls (TCP)                                          */
/* ========================================================================== */

unsigned char _tcp_abort;
unsigned long _tcp_progress;

signed char TCP_Close(unsigned char handle) {
    signed char result;
    #ifdef _NETDEBUG
    Shell_Print("[TCP_Close]\r\n");
    #endif
    Net_SkipMsg(handle); // flush remaining messages pertaining to this handle
    _nsemaon();
    _netmsg[0] = 17;
    _netmsg[3] = handle;
    result = Net_SCommand();
    _nsemaoff();
    #ifdef _NETDEBUG
    Shell_Print("[done]\r\n");
    #endif
    return result;
}

signed char TCP_OpenWait(signed char handle) {
    unsigned char status;
    #ifdef _NETDEBUG
    Shell_Print("[TCP_OpenWait]\r\n");
    #endif
    while (!Net_Wait(NET_TCPEVT)) {
        if (_netmsg[3] == handle) {
            status = _netmsg[8] & 0x1F;
            if (status == TCP_OPENED) {
                // connection stayed open
                _nsemaoff();
                #ifdef _NETDEBUG
                Shell_Print("[done]\r\n");
                #endif
                return handle;
            } else if (status == TCP_CLOSING || status == TCP_CLOSED) {
                // connection immediately closed...
                if (*((unsigned short*)(_netmsg + 4))) {
                    // ...but received data first; treat as open, but requeue this as the close alert
                    #ifdef _NETDEBUG
                    msg_print("REQ: ");
                    #endif
                    Msg_Send(_netpid, _msgpid(), _netmsg);
                    _nsemaoff();
                    #ifdef _NETDEBUG
                    Shell_Print("[done]\r\n");
                    #endif
                    return handle;
                } else {
                    // ...with no data; treat as a rejected connection
                    break;
                }
            }
        }
        #ifdef _NETDEBUG
        msg_print("REQ: ");
        #endif
        Msg_Send(_netpid, _msgpid(), _netmsg); // not relevant, put back on queue
    }
    _nsemaoff();
    TCP_Close(handle);
    _neterr = ERR_CONNECT;
    #ifdef _NETDEBUG
    Shell_Print("[fail]\r\n");
    #endif
    return -1;
}

signed char TCP_OpenClient(char* ip, signed short lport, unsigned short rport) {
    #ifdef _NETDEBUG
    Shell_Print("[TCP_OpenClient]\r\n");
    #endif
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
        #ifdef _NETDEBUG
        Shell_Print("[fail]\r\n");
        #endif
        return -1;
    }
    #ifdef _NETDEBUG
    Shell_Print("[done]\r\n");
    #endif
    return TCP_OpenWait(_netmsg[3]);
}

signed char TCP_OpenServer(unsigned short lport) {
    #ifdef _NETDEBUG
    Shell_Print("[TCP_OpenServer]\r\n");
    #endif
    _nsemaon();
    _netmsg[0] = 16;
    _netmsg[3] = 1;
    *((unsigned short*)(_netmsg + 8)) = lport;
    if (Net_Command()) {
        _nsemaoff();
        #ifdef _NETDEBUG
        Shell_Print("[fail]\r\n");
        #endif
        return -1;
    }
    return TCP_OpenWait(_netmsg[3]);
    #ifdef _NETDEBUG
    Shell_Print("[done]\r\n");
    #endif
}

void TCP_Event(char* msg, NetStat* obj) {
    obj->socket = msg[3];
    obj->bytesrec = *((unsigned short*)(msg + 4));
    obj->rport = *((unsigned short*)(msg + 6));
    obj->status = msg[8] & 0x1F;
    obj->datarec = msg[8] >> 7;
    obj->ip[0] = msg[10];
    obj->ip[1] = msg[11];
    obj->ip[2] = msg[12];
    obj->ip[3] = msg[13];
}

signed char TCP_Status(unsigned char handle, NetStat* obj) {
    #ifdef _NETDEBUG
    Shell_Print("[TCP_Status]\r\n");
    #endif
    _nsemaon();
    _netmsg[0] = 18;
    _netmsg[3] = handle;
    if (Net_Command()) {
        _nsemaoff();
        #ifdef _NETDEBUG
        Shell_Print("[fail]\r\n");
        #endif
        return -1;
    }
    TCP_Event(_netmsg, obj);
    _nsemaoff();
    #ifdef _NETDEBUG
    Shell_Print("[done]\r\n");
    #endif
    return 0;
}

signed char TCP_Receive(unsigned char handle, unsigned char bank, char* addr, unsigned short len, TCP_Trans* obj) {
    #ifdef _NETDEBUG
    Shell_Print("[TCP_Receive]\r\n");
    #endif
    _nsemaon();
    _netmsg[0] = 19;
    _netmsg[3] = handle;
    *((unsigned short*)(_netmsg + 4)) = len;
    _netmsg[6] = bank;
    *((unsigned short*)(_netmsg + 8)) = (unsigned short)addr;
    if (Net_Command()) {
        _nsemaoff();
        #ifdef _NETDEBUG
        Shell_Print("[fail]\r\n");
        #endif
        return -1;
    }
    if (obj != 0) {
        obj->transferred = *((unsigned short*)(_netmsg + 4));
        obj->remaining = *((unsigned short*)(_netmsg + 8));
    }
    _nsemaoff();
    #ifdef _NETDEBUG
    Shell_Print("[done]\r\n");
    #endif
    return 0;
}

signed char TCP_Send(unsigned char handle, unsigned char bank, char* addr, unsigned short len) {
    unsigned short transferred;
    #ifdef _NETDEBUG
    Shell_Print("[TCP_Send]\r\n");
    #endif
    _nsemaon();
    _tcp_abort = 0;
    *(((unsigned short*)&_tcp_progress)) = 0;
    *(((unsigned short*)&_tcp_progress) + 1) = 0;
    for (;;) {
        _netmsg[0] = 20;
        _netmsg[3] = handle;
        *((unsigned short*)(_netmsg + 4)) = len;
        _netmsg[6] = bank;
        *((unsigned short*)(_netmsg + 8)) = (unsigned short)addr;
        if (Net_Command()) {
            _nsemaoff();
            #ifdef _NETDEBUG
            Shell_Print("[fail]\r\n");
            #endif
            return -1;
        }
        transferred = *((unsigned short*)(_netmsg + 4));
        addr += transferred;
        len -= transferred;
        *(unsigned short*)_tcp_progress += transferred;
        if (len <= 0)
            break;
        if (_tcp_abort) {
            _nsemaoff();
            _neterr = ERR_TRUNCATED;
            #ifdef _NETDEBUG
            Shell_Print("[abort]\r\n");
            #endif
            return -1;
        }
    }
    _nsemaoff();
    #ifdef _NETDEBUG
    Shell_Print("[done]\r\n");
    #endif
    return 0;
}

signed char TCP_Skip(unsigned char handle, unsigned short len) {
    signed char result;
    #ifdef _NETDEBUG
    Shell_Print("[TCP_Skip]\r\n");
    #endif
    _nsemaon();
    _netmsg[0] = 21;
    _netmsg[3] = handle;
    *((unsigned short*)(_netmsg + 4)) = len;
    result = Net_SCommand();
    _nsemaoff();
    #ifdef _NETDEBUG
    Shell_Print("[done]\r\n");
    #endif
    return result;
}

signed char TCP_Flush(unsigned char handle) {
    signed char result;
    #ifdef _NETDEBUG
    Shell_Print("[TCP_Flush]\r\n");
    #endif
    _nsemaon();
    _netmsg[0] = 22;
    _netmsg[3] = handle;
    result = Net_SCommand();
    _nsemaoff();
    #ifdef _NETDEBUG
    Shell_Print("[done]\r\n");
    #endif
    return result;
}

signed char TCP_Disconnect(unsigned char handle) {
    signed char result;
    NetStat net_stat;

    #ifdef _NETDEBUG
    Shell_Print("[TCP_Disconnect]\r\n");
    #endif

    // old versions of RSF3 erroneously leave data in RX even after disconnect, so skip anything left
    net_stat.bytesrec = 0;
    if (!TCP_Status(handle, &net_stat) && net_stat.bytesrec)
        TCP_Skip(handle, net_stat.bytesrec);

    // send disconnect message
    _nsemaon();
    _netmsg[0] = 23;
    _netmsg[3] = handle;
    result = Net_SCommand();
    _nsemaoff();

    // flush remaining messages pertaining to this handle
    Net_SkipMsg(handle);
    #ifdef _NETDEBUG
    Shell_Print("[done]\r\n");
    #endif
    return result;
}
