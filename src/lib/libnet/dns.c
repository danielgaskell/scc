#include <symbos.h>
#include <network.h>
#include "network.h"

extern unsigned char _netpid;
extern unsigned char _neterr;

/* ========================================================================== */
/* SymbOS Network Daemon calls (DNS)                                          */
/* ========================================================================== */

unsigned long DNS_Resolve(unsigned char bank, char* addr) {
    unsigned char result;
    unsigned long result2;
    _nsemaon();
    _netmsg[0] = 112;
    _netmsg[6] = bank;
    *((unsigned short*)(_netmsg + 8)) = (unsigned short)addr;
    result = Net_Command();
    if (result) {
        _nsemaoff();
        return 0;
    }
    result2 = *((unsigned long*)(_netmsg + 10));
    _nsemaoff();
    return result2;
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
