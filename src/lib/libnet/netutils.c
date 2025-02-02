#include <network.h>
#include "network.h"

#define PROTOCOLS 9

char _netmsg[1024];
char* _netproto[PROTOCOLS] = {"HTTPS", "HTTP", "FTP", "IRC", "SFTP", "FILE", "IMAP", "POP", "NNTP"};
int _netprotoport[PROTOCOLS+1] = {0, 443, 80, 21, 6667, 22, 445, 143, 110, 119};
char _nethost[128];

signed char Net_SplitURL(char* url, char* host, char** path, int* port) {
    unsigned char i;
    signed char protocol = PROTO_HTTP;
    char* ptr;
    char* dot;
    char* colon;
    char* slash;

    // extract scheme
    dot = strchr(url, '.');
    if (!dot)
        return -1;
    colon = strstr(url, "://");
    if (colon != 0 && colon < dot) {
        memcpy(host, url, colon - url);
        host[colon - url] = 0;
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
    if (colon != 0 && colon < slash) {
        memcpy(host, colon + 1, slash - colon - 1);
        host[slash - colon - 1] = 0;
        *port = atoi(host);
        memcpy(host, url, colon - url - 1);
        host[slash - url] = 0;
    } else {
        if (!slash)
            slash = url + strlen(url);
        memcpy(host, url, slash - url - 1);
        host[slash - url - 1] = 0;
        *port = _netprotoport[protocol];
    }

    // extract path
    if (slash)
        *path = slash + 1;
    else
        *path = host + strlen(host); // points directly to zero terminator

    return protocol;
}
