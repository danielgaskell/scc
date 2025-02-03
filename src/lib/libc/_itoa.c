/*
 *	Internal wrapper (in theory) but used in various places it's helpful
 */
#include <stdlib.h>
#include <string.h>

/*********************** xitoa.c ***************************/

/* Don't go via long - the long version is expensive on an 8bit processor and
   can often be avoided */

const char *_uitoa(unsigned int i) {
    char _uitoabuf[7];
	char *p;
	int c;

	p = _uitoabuf + sizeof(_uitoabuf);
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
