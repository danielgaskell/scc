#include <symbos.h>

signed char Print_String(char* str) {
    unsigned char ch, i = 1;
    while (ch = *str++) {
        i = Print_Char(ch);
        if (i)
            break;
    }
    return i;
}
