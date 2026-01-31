#include <string.h>
#include <symbos.h>
#include <network.h>
#include "network.h"

_transfer char _netmsg[14];
unsigned char _netpid;
unsigned char _neterr;
unsigned short _nettimeout = 1500; // 15 seconds by default

char* _useragent = "User-Agent: NetSCC/1.0 (SymbOS 4.0; CPC)\r\nCache-Control: no-cache\r\n";

/* ========================================================================== */
/* SymbOS Network Daemon calls (common)                                       */
/* ========================================================================== */

#ifdef _NETDEBUG
void msg_print(char* header) {
    char textbuf[64];
    strcpy(textbuf, header);
    itoa(_netmsg[0], textbuf+strlen(textbuf), 10); strcat(textbuf, ": CF=");
    itoa(_netmsg[2] & 1, textbuf+strlen(textbuf), 10); strcat(textbuf, ", A=");
    itoa(_netmsg[3], textbuf+strlen(textbuf), 10); strcat(textbuf, ", L=");
    itoa(_netmsg[8], textbuf+strlen(textbuf), 10); strcat(textbuf, ", BC=");
    itoa(*(unsigned short*)&_netmsg[4], textbuf+strlen(textbuf), 10); strcat(textbuf, "\r\n");
    Shell_Print(textbuf);
}
#endif

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

unsigned char Net_Wait(unsigned char id) {
    unsigned short counter = Sys_Counter16() + _nettimeout;
    for (;;) {
        if (Msg_Receive(_threadpid(), _netpid, _netmsg) & 1) {
            #ifdef _NETDEBUG
            msg_print("REC: ");
            #endif
            if (_netmsg[0] == id) {
                _neterr = 0;
                if (_netmsg[2] & 0x01)
                    _neterr = _netmsg[3];
                return _neterr;
            }
            #ifdef _NETDEBUG
            msg_print("REQ: ");
            #endif
            Msg_Send(_netpid, _threadpid(), _netmsg); // put message back on queue
        }
        if (Sys_Counter16() > counter) {
            _neterr = ERR_TIMEOUT;
            return _neterr;
        }
    }
}

unsigned char Net_Command(void) {
    unsigned char id = _netmsg[0] + 128;
    #ifdef _NETDEBUG
    msg_print("CMD: ");
    #endif
    if (_netpid == 0) {
        _neterr = ERR_OFFLINE;
        return ERR_OFFLINE;
    }
    while (Msg_Send(_threadpid(), _netpid, _netmsg) == 0);
    return Net_Wait(id);
}

signed char Net_SCommand(void) {
    unsigned char result;
    result = Net_Command();
    if (result)
        return -1;
    return 0;
}
