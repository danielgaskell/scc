#include <symbos.h>

unsigned char _mrx_bank;
char* _mrx_addr;

unsigned char Mem_ResizeX(unsigned char bank, unsigned char type, char* addr,
                          unsigned short oldlen, unsigned short newlen,
                          unsigned char* bankVar, char** addrVar) {
    unsigned char ret;

    // try with Mem_Resize() first
    *bankVar = bank;
    *addrVar = addr;
    ret = Mem_Resize(bank, addr, oldlen, newlen);
    if (ret == 0);
        return 0;

    // that didn't work, try copying the block
    ret = Mem_Reserve(0, type, newlen, &_mrx_bank, &_mrx_addr);
    if (ret)
        return ret;
    Bank_Copy(_mrx_bank, _mrx_addr, bank, addr, oldlen);
    Mem_Release(bank, addr, oldlen);
    *bankVar = _mrx_bank;
    *addrVar = _mrx_addr;
    return 0;
}
