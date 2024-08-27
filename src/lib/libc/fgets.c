/* stdio.c
 * Copyright (C) 1996 Robert de Bath <rdebath@cix.compulink.co.uk>
 * This file is part of the Linux-8086 C library and is distributed
 * under the GNU Library General Public License.
 */

/* This is an implementation of the C standard IO package. */

#include "stdio-l.h"
#include <symbos.h>

/* Nothing special here ... */
char *fgets(char *s, size_t count, FILE * f)
{
	register size_t i = count;
	register int ch;
	char *ret = s;

	if (f == stdin) {
        // defer to the gets_s implementation of console input, except that
        // we need the trailing newline included by fgets() but not by gets()
        ret = gets_s(s, count);
        if (ret)
            ret[strlen(ret)] = '\n';
        return ret;

	} else {
        while (i-- != 0) {
            Shell_CharOut(0, 'c');
            if ((ch = getc(f)) == EOF) {
                Shell_CharOut(0, 'd');
                if (s == ret)
                    return NULL;
                break;
            }
            *s++ = (char) ch;
            if (ch == '\r' || ch == '\n')
                break;
        }
        *s = 0;
        return ferror(f) ? NULL : ret;
}	}

