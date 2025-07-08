#include <network.h>
#include "network.h"

#define PROTOCOLS 9

char* _netproto[PROTOCOLS] = {"HTTPS", "HTTP", "FTP", "IRC", "SFTP", "FILE", "IMAP", "POP", "NNTP"};
int _netprotoport[PROTOCOLS+1] = {0, 443, 80, 21, 6667, 22, 445, 143, 110, 119};

signed char Net_SplitURL(char* url, char* host, char** path, unsigned short* port) {
    unsigned char i;
    signed char protocol = PROTO_HTTP;
    char* ptr;
    char* dot;
    char* colon;
    char* slash;
    unsigned short len;

    // extract scheme
    dot = strchr(url, '.');
    if (!dot) {
        _neterr = ERR_BADDOMAIN;
        return -1;
    }
    colon = strstr(url, "://");
    if (colon != 0 && colon < dot) {
        len = colon - url;
        if (len > 64)
            len = 64;
        memcpy(host, url, len);
        host[len] = 0;
        for (i = 0; i < PROTOCOLS; ++i) {
            if (stricmp(host, _netproto[i]) == 0) {
                protocol = i + 1;
                break;
            }
        }
        url = colon + 3;
    }

    // extract host and port
    slash = strchr(dot, '/'); // search for first slash after first dot
    if (colon < url)
        colon = strchr(url, ':');
    if (colon != 0) {
        if (slash)
            len = slash - colon - 1;
        else
            len = strlen(colon + 1);
        if (len > 8)
            len = 8;
        memcpy(host, colon + 1, len);
        host[len] = 0;
        *port = atoi(host);
        len = colon - url;
        if (len > 64)
            len = 64;
        memcpy(host, url, len);
        host[len] = 0;
    } else {
        if (slash)
            len = slash - url;
        else
            len = strlen(url);
        if (len > 64)
            len = 64;
        memcpy(host, url, len);
        host[len] = 0;
        *port = _netprotoport[protocol];
    }

    // extract path
    if (slash)
        *path = slash + 1;
    else
        *path = host + strlen(host); // points directly to zero terminator

    return protocol;
}
