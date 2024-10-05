#include <symbos.h>

unsigned char Mem_Resize(unsigned char bank, unsigned char type, char* addr,
                         unsigned short oldlen, unsigned short newlen,
                         unsigned char* bankVar, char** addrVar) {
    unsigned char ret;
    unsigned char _mrx_bank;
    char* _mrx_addr;

    ret = Mem_Reserve(0, type, newlen, &_mrx_bank, &_mrx_addr);
    if (ret)
        return ret;
    Bank_Copy(_mrx_bank, _mrx_addr, bank, addr, oldlen);
    Mem_Release(bank, addr, oldlen);
    *bankVar = _mrx_bank;
    *addrVar = _mrx_addr;
    return 0;
}
