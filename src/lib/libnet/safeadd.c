// This small island of insanity brought to you by the fact that 32-bit data
// types are inherently thread-unsafe in SCC.

void _safeadd(char* dest, unsigned short add) {
    unsigned short temp;
    temp = dest[0] + (unsigned char)add;
    dest[0] = temp;
    temp = dest[1] + (add >> 8) + (temp >> 8);
    dest[1] = temp;
    temp = dest[2] + (temp >> 8);
    dest[2] = temp;
    dest[3] += (temp >> 8);
}
