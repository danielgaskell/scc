#include <network.h>
#include "network.h"

signed char Net_PublicIP(char* ip) {
    char* ptr;
    char ipbuf[24];
    if (HTTP_GET("http://checkip.amazonaws.com", ipbuf, sizeof(ipbuf) - 1, 0, 0) == -1)
        return _neterr;
    ptr = ipbuf + strlen(ipbuf) - 1;
    if (ptr == ipbuf - 1)
        return ERR_BADDOMAIN;
    while (*ptr == 13 || *ptr == 10)
        *ptr-- = 0;
    if (DNS_Resolve(_symbank, ipbuf, ip)) // quick way to convert an IP string to a number
        return -1;
    return 0;
}
