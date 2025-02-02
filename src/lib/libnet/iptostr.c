#include <stdlib.h>

char* iptostr(unsigned long ip, char* dest) {
    char buf[4];
    itoa(*(((unsigned char*)&ip) + 0), buf, 10);
    strcpy(dest, buf);
    strcat(dest, ".");
    itoa(*(((unsigned char*)&ip) + 1), buf, 10);
    strcat(dest, buf);
    strcat(dest, ".");
    itoa(*(((unsigned char*)&ip) + 2), buf, 10);
    strcat(dest, buf);
    strcat(dest, ".");
    itoa(*(((unsigned char*)&ip) + 3), buf, 10);
    strcat(dest, buf);
    return dest;
}
