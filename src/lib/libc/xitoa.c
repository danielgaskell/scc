/*
 *	Internal wrapper (in theory) but used in various places it's helpful
 */
#include <stdlib.h>
#include <string.h>

/*********************** xitoa.c ***************************/

/* Don't go via long - the long version is expensive on an 8bit processor and
   can often be avoided */

char *uitoa(unsigned int __value, char *__strP, int __radix) {
    char _uitoabuf[7];
    char *p;
	int c;

	p = _uitoabuf + sizeof(_uitoabuf);
	*--p = '\0';
	do {
		c = __value % __radix;
		__value /= __radix;
		*--p = "0123456789ABCDEF"[c];
	} while(__value);
    strcpy(__strP, p);
    return __strP;
}

char *itoa(int __value, char *__strP, int __radix) {
    char *p;
	if (__value >= 0)
		return uitoa(__value, __strP, __radix);
	p = uitoa(-__value, __strP + 1, __radix);
	*--p = '-';
	return __strP;
}
