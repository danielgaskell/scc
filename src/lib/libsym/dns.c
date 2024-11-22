#include <symbos.h>

extern unsigned char _netpid;
extern unsigned char _neterr;

/* ========================================================================== */
/* SymbOS Network Daemon calls (DNS)                                          */
/* ========================================================================== */

unsigned long DNS_Resolve(unsigned char bank, char* addr) {
    unsigned char result;
    unsigned long result2;
    _msemaon();
    _symmsg[0] = 112;
    _symmsg[6] = bank;
    *((unsigned short*)(_symmsg + 8)) = (unsigned short)addr;
    result = Net_Command();
    if (result) {
        _msemaoff();
        return 0;
    }
    result2 = *((unsigned long*)(_symmsg + 10));
    _msemaoff();
    return result2;
}

unsigned char DNS_Verify(unsigned char bank, char* addr) {
    unsigned char result;
    _msemaon();
    _symmsg[0] = 113;
    _symmsg[6] = bank;
    *((unsigned short*)(_symmsg + 8)) = (unsigned short)addr;
    result = Net_Command();
    if (result) {
        _msemaoff();
        return 0;
    }
    result = _symmsg[8];
    _msemaoff();
    return result;
}
