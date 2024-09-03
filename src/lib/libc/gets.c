/* stdio.c
 * Copyright (C) 1996 Robert de Bath <rdebath@cix.compulink.co.uk>
 * This file is part of the Linux-8086 C library and is distributed
 * under the GNU Library General Public License.
 *
 * Substantially rewritten 2024 by Daniel E. Gaskell.
 */

/* This is an implementation of the C standard IO package. */

#include "stdio-l.h"
#include <errno.h>
#include <symbos.h>
#include <string.h>
#include "iobuf.h"

// Using Shell_StringIn gives a much nicer experience on the console, but has
// a max length of 255 characters. This is fine for normal usage. Taking
// advantage of this limitation, we avoid the infamous gets() buffer overrun
// problem by reading into the known-good 256-byte _io_buf as an intermediate.
char *gets_s(char *str, size_t maxlen) {
    signed char result;
    unsigned char len;
    if (str == NULL || maxlen == 0) {
        errno = ERANGE;
        return 0;
    }
    fflush(stdout);
    result = Shell_StringIn(0, _symbank, _io_buf);
    if (result == -1) {
        stdin->mode |= __MODE_EOF;
        return 0;
    } else if (result == -2) {
        stdin->mode |= __MODE_ERR;
        return 0;
    }
    len = strlen(_io_buf);
    if (len > maxlen)
        return 0;
    memcpy(str, _io_buf, (int)len + 1);
    return str;
}

char *gets(char *str) {
    return gets_s(str, 255);
}

int puts(const char *str) {
	int n;
	if (((n = fputs(str, stdout)) == EOF)
	    || (putc('\n', stdout) == EOF))
		return (EOF);
	return (++n);
}
