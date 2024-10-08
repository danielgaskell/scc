/* stdio.c
 * Copyright (C) 1996 Robert de Bath <rdebath@cix.compulink.co.uk>
 * This file is part of the Linux-8086 C library and is distributed
 * under the GNU Library General Public License.
 */

/* This is an implementation of the C standard IO package. */

#include "stdio-l.h"

/* Nothing special here ... */
char *fgets(char *s, size_t count, FILE * f)
{
	size_t i = count;
	int ch;
	char *ret = s;

	if (f == stdin) {
        // defer to the gets_s implementation of console input, except that
        // we need the trailing newline included by fgets() but not by gets()
        ret = gets_s(s, count);
        if (ret) {
            i = strlen(ret);
            ret[i] = '\n';
            ret[i+1] = 0;
        }
        return ret;

	} else {
        while (i-- != 0) {
            if ((ch = getc(f)) == EOF) {
                if (s == ret)
                    return NULL;
                break;
            }
            if (ch != '\r')
                *s++ = (char) ch;
            if (ch == '\n' || ch == 0x1A)
                break;
        }
        *s = 0;
        return ferror(f) ? NULL : ret;
    }
}

