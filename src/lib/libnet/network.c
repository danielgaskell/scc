#include <symbos.h>
#include <network.h>
#include "network.h"

unsigned char _netpid;
unsigned char _neterr;
unsigned short _nettimeout = 1500;

char* _useragent = "User-Agent: NetSCC/1.0 (SymbOS 4.0; CPC)\r\nCache-Control: no-cache\r\n";

/* ========================================================================== */
/* SymbOS Network Daemon calls (common)                                       */
/* ========================================================================== */

signed char Net_Init(void) {
    unsigned short symver;
    _netpid = (App_Search(_symbank, "Network Daem") >> 8);
    if (_netpid) {
        symver = Sys_Version();
        _useragent[31] = '0' + (symver / 10);
        _useragent[33] = '0' + (symver % 10);
        return 0;
    }
    _neterr = ERR_OFFLINE;
    return -1;
}

unsigned char Net_Command(void) {
    unsigned char id = _symmsg[0] + 128;
    if (_netpid == 0) {
        _neterr = ERR_OFFLINE;
        return ERR_OFFLINE;
    }
    while (Msg_Send(_sympid, _netpid, _symmsg) == 0);
    for (;;) {
        Msg_Sleep(_sympid, _netpid, _symmsg);
        if (_symmsg[0] == id)
            break;
        Msg_Send(_netpid, _sympid, _symmsg); // put message back on queue
    }
    if (_symmsg[2] & 0x01) {
        _neterr = _symmsg[3];
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
