#include <network.h>
#include "network.h"

unsigned long Net_PublicIP(void) {
    char* ptr;
    if (HTTP_GET("checkip.amazonaws.com", _netmsg, sizeof(_netmsg), 0, 1) == -1)
        return 0;
    ptr = _netmsg + strlen(_netmsg) - 1;
    if (ptr == _netmsg - 1)
        return 0;
    while (*ptr == 13 || *ptr == 10)
        *ptr-- = 0;
    return DNS_Resolve(_symbank, _netmsg); // quick way to convert an IP string to a number
}
