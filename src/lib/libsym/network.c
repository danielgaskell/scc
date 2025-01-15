#include <symbos.h>

unsigned char _netpid;
unsigned char _neterr;

/* ========================================================================== */
/* SymbOS Network Daemon calls (common)                                       */
/* ========================================================================== */

unsigned char Net_Init(void) {
    _netpid = (App_Search(_symbank, "Network Daem") >> 8);
    if (_netpid)
        return 0;
    _neterr = ERR_OFFLINE;
    return ERR_OFFLINE;
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
