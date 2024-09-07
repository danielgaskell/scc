#include <symbos.h>

unsigned char _netpid;
unsigned char _neterr;

/* ========================================================================== */
/* SymbOS Network Daemon calls                                                  */
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
    if (_netpid == 0)
        return ERR_OFFLINE;
    while (Msg_Send(_sympid, _netpid, _symmsg) == 0);
    while (_symmsg[0] != id)
        Msg_Sleep(_sympid, _netpid, _symmsg);
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

signed char TCP_OpenClient(unsigned long ip, unsigned short lport, signed short rport) {
    unsigned char result;
    _msemaon();
    _symmsg[0] = 16;
    _symmsg[3] = 0;
    *((unsigned short*)(_symmsg + 6)) = rport;
    *((unsigned short*)(_symmsg + 8)) = lport;
    *((unsigned long*)(_symmsg + 10)) = ip;
    result = Net_Command();
    if (result) {
        _msemaoff();
        return -1;
    }
    result = _symmsg[3]; // connection handle
    _msemaoff();
    return result;
}

signed char TCP_OpenServer(unsigned short lport) {
    unsigned char result;
    _msemaon();
    _symmsg[0] = 16;
    _symmsg[3] = 1;
    *((unsigned short*)(_symmsg + 8)) = lport;
    result = Net_Command();
    if (result) {
        _msemaoff();
        return -1;
    }
    result = _symmsg[3]; // connection handle
    _msemaoff();
    return result;
}

signed char TCP_Close(unsigned char handle) {
    signed char result;
    _msemaon();
    _symmsg[0] = 17;
    _symmsg[3] = handle;
    result = Net_SCommand();
    _msemaoff();
    return result;
}

signed char TCP_Status(unsigned char handle, NetStat* obj) {
    unsigned char result;
    _msemaon();
    _symmsg[0] = 18;
    _symmsg[3] = handle;
    result = Net_Command();
    if (result) {
        _msemaoff();
        return -1;
    }
    obj->bytesrec = *((unsigned short*)(_symmsg + 4));
    obj->rport = *((unsigned short*)(_symmsg + 6));
    obj->status = _symmsg[8] & 0x1F;
    obj->datarec = _symmsg[8] >> 7;
    obj->ip = *((unsigned long*)(_symmsg + 10));
    _msemaoff();
    return 0;
}

signed char TCP_Receive(unsigned char handle, unsigned char bank, char* addr, unsigned short len, TCP_Trans* obj) {
    unsigned char result;
    _msemaon();
    _symmsg[0] = 19;
    _symmsg[3] = handle;
    *((unsigned short*)(_symmsg + 4)) = len;
    _symmsg[6] = bank;
    *((unsigned short*)(_symmsg + 8)) = (unsigned short)addr;
    result = Net_Command();
    if (result) {
        _msemaoff();
        return -1;
    }
    if (obj != 0) {
        obj->transferred = *((unsigned short*)(_symmsg + 4));
        obj->remaining = *((unsigned short*)(_symmsg + 8));
    }
    _msemaoff();
    return 0;
}

signed char TCP_Send(unsigned char handle, unsigned char bank, char* addr, unsigned short len, TCP_Trans* obj) {
    unsigned char result;
    _msemaon();
    _symmsg[0] = 20;
    _symmsg[3] = handle;
    *((unsigned short*)(_symmsg + 4)) = len;
    _symmsg[6] = bank;
    *((unsigned short*)(_symmsg + 8)) = (unsigned short)addr;
    result = Net_Command();
    if (result) {
        _msemaoff();
        return -1;
    }
    if (obj != 0) {
        obj->transferred = *((unsigned short*)(_symmsg + 4));
        obj->remaining = *((unsigned short*)(_symmsg + 8));
    }
    _msemaoff();
    return 0;
}

signed char TCP_Skip(unsigned char handle, unsigned short len) {
    signed char result;
    _msemaon();
    _symmsg[0] = 21;
    _symmsg[3] = handle;
    *((unsigned short*)(_symmsg + 4)) = len;
    result = Net_SCommand();
    _msemaoff();
    return result;
}

signed char TCP_Flush(unsigned char handle) {
    signed char result;
    _msemaon();
    _symmsg[0] = 22;
    _symmsg[3] = handle;
    result = Net_SCommand();
    _msemaoff();
    return result;
}

signed char TCP_Disconnect(unsigned char handle) {
    signed char result;
    _msemaon();
    _symmsg[0] = 23;
    _symmsg[3] = handle;
    result = Net_SCommand();
    _msemaoff();
    return result;
}

signed char UDP_Open(unsigned char type, unsigned short lport, unsigned char bank) {
    unsigned char result;
    _msemaon();
    _symmsg[0] = 32;
    _symmsg[3] = type;
    _symmsg[6] = bank;
    *((unsigned short*)(_symmsg + 8)) = lport;
    result = Net_Command();
    if (result) {
        _msemaoff();
        return -1;
    }
    result = _symmsg[3];
    _msemaoff();
    return result;
}

signed char UDP_Close(unsigned char handle) {
    unsigned char result;
    _msemaon();
    _symmsg[0] = 33;
    _symmsg[3] = handle;
    result = Net_Command();
    _msemaoff();
    if (result)
        return -1;
    return 0;
}

signed char UDP_Status(unsigned char handle, NetStat* obj) {
    unsigned char result;
    _msemaon();
    _symmsg[0] = 34;
    _symmsg[3] = handle;
    result = Net_Command();
    if (result) {
        _msemaoff();
        return -1;
    }
    obj->bytesrec = *((unsigned short*)(_symmsg + 4));
    obj->rport = *((unsigned short*)(_symmsg + 6));
    obj->status = _symmsg[8];
    obj->ip = *((unsigned long*)(_symmsg + 10));
    _msemaoff();
    return 0;
}

signed char UDP_Receive(unsigned char handle, char* addr) {
    signed char result;
    _msemaon();
    _symmsg[0] = 35;
    _symmsg[3] = handle;
    *((unsigned short*)(_symmsg + 8)) = (unsigned short)addr;
    result = Net_SCommand();
    _msemaoff();
    return result;
}

signed char UDP_Send(unsigned char handle, char* addr, unsigned short len, unsigned long ip, unsigned short rport) {
    signed char result;
    _msemaon();
    _symmsg[0] = 36;
    _symmsg[3] = handle;
    *((unsigned short*)(_symmsg + 4)) = len;
    *((unsigned short*)(_symmsg + 6)) = rport;
    *((unsigned short*)(_symmsg + 8)) = (unsigned short)addr;
    *((unsigned long*)(_symmsg + 10)) = ip;
    result = Net_SCommand();
    _msemaoff();
    return result;
}

signed char UDP_Skip(unsigned char handle) {
    signed char result;
    _msemaon();
    _symmsg[0] = 37;
    _symmsg[3] = handle;
    result = Net_SCommand();
    _msemaoff();
    return result;
}

unsigned long DNS_Resolve(char* addr) {
    unsigned char result;
    unsigned long result2;
    _msemaon();
    _symmsg[0] = 112;
    *((unsigned short*)(_symmsg + 8)) = (unsigned short)addr;
    result = Net_Command();
    if (result) {
        _msemaoff();
        return 0;
    }
    result2 = *((unsigned long*)(_symmsg + 10));
    _msemaoff();
    return result;
}

unsigned char DNS_Verify(char* addr) {
    unsigned char result;
    _msemaon();
    _symmsg[0] = 113;
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
