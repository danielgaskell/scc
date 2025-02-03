/* Copyright (C) 1995,1996 Robert de Bath <rdebath@cix.compulink.co.uk>
 * This file is part of the Linux-8086 C library and is distributed
 * under the GNU Library General Public License.
 *
 * FIXME: helper for stdio needs to become re-entrant
 */

#include <string.h>

char * __ultostr_r(char *buf, unsigned long val, int radix) {
   char *p;
   int c;

   if( radix > 36 || radix < 2 ) return 0;

   p = buf + 34;
   *--p = '\0';

   do
   {
      c = val%radix;
      val/=radix;
      if( c > 9 ) *--p = 'a'-10+c; else *--p = '0'+c;
   }
   while(val);
   return p;
}

char * __ltostr_r(char *buf, long val, int radix) {
   char *p;
   int flg = 0;
   if( val < 0 ) { flg++; val= -val; }
   p = __ultostr_r(buf, val, radix);
   if(p && flg) *--p = '-';
   return p;
}

/* sadly we do not know the size of the user-provided buffer so we cannot write
   it backwards, so just copy the result from our own buffer whose size we know */

char *ultoa (unsigned long value, char *strP, int radix) {
    char buf[34];
    return strcpy(strP, __ultostr_r(buf, value, radix));
}

char *ltoa (long value, char *strP, int radix) {
    char buf[34];
    return strcpy(strP, __ltostr_r(buf, value, radix));
}
