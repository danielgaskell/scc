#include <symbos.h>
#include <network.h>
#include "network.h"

/* ========================================================================== */
/* SymbOS Network Daemon calls (SSL/TLS)                                      */
/* ========================================================================== */

signed char SSL_OpenClient(char* ip, signed short lport, unsigned short rport, unsigned char bank, char* host) {
    #ifdef _NETDEBUG
    Shell_Print("[SSL_OpenClient]\r\n");
    #endif
    _nsemaon();
    _netmsg[0] = 16;
    _netmsg[3] = host ? 12 : 4;
    *((unsigned short*)(_netmsg + 6)) = rport;
    *((unsigned short*)(_netmsg + 8)) = lport;
    _netmsg[10] = ip[0];
    _netmsg[11] = ip[1];
    _netmsg[12] = ip[2];
    _netmsg[13] = ip[3];
    if (Net_Command()) {
        _nsemaoff();
        #ifdef _NETDEBUG
        Shell_Print("[fail]\r\n");
        #endif
        return -1;
    }
    #ifdef _NETDEBUG
    Shell_Print("[done]\r\n");
    #endif
    return TCP_OpenWait(_netmsg[3]);
}

signed char SSL_OpenServer(unsigned short lport) {
    #ifdef _NETDEBUG
    Shell_Print("[TCP_OpenServer]\r\n");
    #endif
    _nsemaon();
    _netmsg[0] = 16;
    _netmsg[3] = 5;
    *((unsigned short*)(_netmsg + 8)) = lport;
    if (Net_Command()) {
        _nsemaoff();
        #ifdef _NETDEBUG
        Shell_Print("[fail]\r\n");
        #endif
        return -1;
    }
    return TCP_OpenWait(_netmsg[3]);
    #ifdef _NETDEBUG
    Shell_Print("[done]\r\n");
    #endif
}
