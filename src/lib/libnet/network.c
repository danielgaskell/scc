#include <symbos.h>
#include <network.h>
#include "network.h"

_transfer char _netmsg[14];
unsigned char _netpid;
unsigned char _neterr;
unsigned short _nettimeout = 1500;

char* _useragent = "User-Agent: NetSCC/1.0 (SymbOS 4.0; CPC)\r\nCache-Control: no-cache\r\n";

/* ========================================================================== */
/* SymbOS Network Daemon calls (common)                                       */
/* ========================================================================== */

signed char Net_Init(void) {
    _netpid = (App_Search(_symbank, "Network Daem") >> 8);
    if (_netpid) {
        _useragent[31] = '0' + (_symversion / 10);
        _useragent[33] = '0' + (_symversion % 10);
        return 0;
    }
    _neterr = ERR_OFFLINE;
    return -1;
}

unsigned char Net_Command(void) {
    unsigned char id = _netmsg[0] + 128;
    if (_netpid == 0) {
        _neterr = ERR_OFFLINE;
        return ERR_OFFLINE;
    }
    while (Msg_Send(_msgpid(), _netpid, _netmsg) == 0);
    for (;;) {
        _netmsg[0] = 0;
        Msg_Sleep(_msgpid(), _netpid, _netmsg);
        if (_netmsg[0]) {
            if (_netmsg[0] == id)
                break;
            Msg_Send(_netpid, _msgpid(), _netmsg); // put message back on queue
            Idle();
        }
    }
    if (_netmsg[2] & 0x01) {
        _neterr = _netmsg[3];
        return _neterr;
    } else {
        _neterr = 0;
        return 0;
    }
}

signed char Net_SCommand(void) {
    unsigned char result;
    result = Net_Command();
    if (result)
        return -1;
    return 0;
}
