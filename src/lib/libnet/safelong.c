// This small island of insanity brought to you by the fact that 32-bit data
// types are inherently thread-unsafe in SCC.

void _safeatol(char* src, char* dest) {
    unsigned short temp;
    while (*src == ' ' || *src == '\t')
        ++src;
    dest[0] = 0;
    dest[1] = 0;
    dest[2] = 0;
    dest[3] = 0;
    while (*src >= '0' && *src <= '9') {
        // perform dest = (dest * 10) + digit bytewise, carrying as we go
        temp = (dest[0] * 10) + (*src++ - '0');
        dest[0] = temp;
        temp = (dest[1] * 10) + (temp >> 8);
        dest[1] = temp;
        temp = (dest[2] * 10) + (temp >> 8);
        dest[2] = temp;
        dest[3] = (dest[3] * 10) + (temp >> 8);
    }
}

unsigned char _safegte(char* left, char* right) {
    signed char i;
    unsigned char l, r;
    for (i = 3; i >= 0; --i) {
        l = left[i];
        r = right[i];
        if (l > r) return 1;
        if (r < l) return 0;
    }
    return 1;
}
