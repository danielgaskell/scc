#include <network.h>
#include "network.h"

signed char Net_PublicIP(char* ip) {
    char* ptr;
    if (HTTP_GET("checkip.amazonaws.com", _netpacket, sizeof(_netpacket) - 1, 0, 1) == -1)
        return _neterr;
    ptr = _netpacket + strlen(_netpacket) - 1;
    if (ptr == _netpacket - 1)
        return ERR_BADDOMAIN;
    while (*ptr == 13 || *ptr == 10)
        *ptr-- = 0;
    if (DNS_Resolve(_symbank, _netpacket, ip)) // quick way to convert an IP string to a number
        return -1;
    return 0;
}
