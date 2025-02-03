#include <network.h>
#include "network.h"

unsigned long Net_PublicIP(void) {
    char* ptr;
    if (HTTP_GET("checkip.amazonaws.com", _netpacket, sizeof(_netpacket) - 1, 0, 1) == -1)
        return 0;
    ptr = _netpacket + strlen(_netpacket) - 1;
    if (ptr == _netpacket - 1)
        return 0;
    while (*ptr == 13 || *ptr == 10)
        *ptr-- = 0;
    return DNS_Resolve(_symbank, _netpacket); // quick way to convert an IP string to a number
}
