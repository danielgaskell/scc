/*
 *	Internal wrapper (in theory) but used in various places it's helpful
 */
#include <stdlib.h>
#include <string.h>

/*********************** xitoa.c ***************************/

/* Don't go via long - the long version is expensive on an 8bit processor and
   can often be avoided */

char _uitoabuf[7];

const char *_uitoa(unsigned int i)
{
	char *p = _uitoabuf + sizeof(_uitoabuf);
	int c;

	*--p = '\0';
	do {
		c = i % 10;
		i /= 10;
		*--p = '0' + c;
	} while(i);
	return p;
}

const char *_itoa(int i) {
	char *p;
	if (i >= 0)
		return _uitoa(i);
	p = (char *)_uitoa(-i);
	*--p = '-';
	return p;
}

char *uitoa(unsigned int __value, char *__strP, int __radix)
{
    char *p = _uitoabuf + sizeof(_uitoabuf);
	int c;

	*--p = '\0';
	do {
		c = __value % __radix;
		__value /= __radix;
		*--p = "0123456789ABCDEF"[c];
	} while(__value);
    strcpy(__strP, p);
    return __strP;
}

char *itoa(int __value, char *__strP, int __radix)
{
    char *p;
	if (__value >= 0)
		return uitoa(__value, __strP, __radix);
	p = uitoa(__value, __strP + 1, __radix);
	*--p = '-';
	return __strP;
}
