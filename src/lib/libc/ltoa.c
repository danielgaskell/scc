/* Copyright (C) 1995,1996 Robert de Bath <rdebath@cix.compulink.co.uk>
 * This file is part of the Linux-8086 C library and is distributed
 * under the GNU Library General Public License.
 */

char _ltoabuf[12];

const char *_ultoa(unsigned long val) {
    char *p;
    p = _ltoabuf + sizeof(_ltoabuf);
    *--p = '\0';
    do {
        *--p = '0' + val%10;
        val /= 10;
    } while(val);
    return p;
}

const char *_ltoa(long val) {
    char *p;
    int flg = 0;
    if (val < 0) {
        flg++;
        val= -val;
    }
    p = (char *)_ultoa(val);
    if (flg)
        *--p = '-';
    return p;
}
