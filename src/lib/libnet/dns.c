#include <symbos.h>
#include <network.h>
#include "network.h"

extern unsigned char _netpid;
extern unsigned char _neterr;

/* ========================================================================== */
/* SymbOS Network Daemon calls (DNS)                                          */
/* ========================================================================== */

signed char DNS_Resolve(unsigned char bank, char* addr, char* ip) {
    unsigned char result;
    _nsemaon();
    _netmsg[0] = 112;
    _netmsg[6] = bank;
    *((unsigned short*)(_netmsg + 8)) = (unsigned short)addr;
    result = Net_Command();
    if (result) {
        _nsemaoff();
        return -1;
    }
    ip[0] = _netmsg[10];
    ip[1] = _netmsg[11];
    ip[2] = _netmsg[12];
    ip[3] = _netmsg[13];
    _nsemaoff();
    return 0;
}

unsigned char DNS_Verify(unsigned char bank, char* addr) {
    unsigned char result;
    _nsemaon();
    _netmsg[0] = 113;
    _netmsg[6] = bank;
    *((unsigned short*)(_netmsg + 8)) = (unsigned short)addr;
    result = Net_Command();
    if (result) {
        _nsemaoff();
        return 0;
    }
    result = _netmsg[8];
    _nsemaoff();
    return result;
}
