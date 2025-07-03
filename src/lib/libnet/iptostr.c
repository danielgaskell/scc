#include <stdlib.h>

char* iptoa(char* ip, char* dest) {
    char buf[4];
    itoa(ip[0], buf, 10);
    strcpy(dest, buf);
    strcat(dest, ".");
    itoa(ip[1], buf, 10);
    strcat(dest, buf);
    strcat(dest, ".");
    itoa(ip[2], buf, 10);
    strcat(dest, buf);
    strcat(dest, ".");
    itoa(ip[3], buf, 10);
    strcat(dest, buf);
    return dest;
}
