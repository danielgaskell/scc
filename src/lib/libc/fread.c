/* stdio.c
 * Copyright (C) 1996 Robert de Bath <rdebath@cix.compulink.co.uk>
 * This file is part of the Linux-8086 C library and is distributed
 * under the GNU Library General Public License.
 */

/* This is an implementation of the C standard IO package. */

#include "stdio-l.h"
#include <symbos.h>

/*
 * fread will often be used to read in large chunks of data calling read()
 * directly can be a big win in this case. Beware also fgetc calls this
 * function to fill the buffer.
 *
 * This ignores __MODE__IOTRAN; probably exactly what you want.
 * (It _is_ what fgetc wants)
 */
int fread(void *buf, size_t size, size_t nelm, FILE * fp)
{
	register int len, v;
	unsigned bytes, got = 0;

    Shell_CharOut(0, 'j');
	if (!buf || !size || !nelm || !fp)
		return 0;
	v = fp->mode;
	/* Want to do this to bring the file pointer up to date */
    Shell_CharOut(0, 'k');
	if (v & __MODE_WRITING)
		fflush(fp);
	/* Can't read or there's been an EOF or error then return zero */
    Shell_CharOut(0, 'l');
	if ((v & (__MODE_READ | __MODE_EOF | __MODE_ERR)) != __MODE_READ)
		return 0;
	/* This could be long, doesn't seem much point tho */
	bytes = size * nelm;
	len = fp->bufread - fp->bufpos;
	if (len >= bytes) {	/* Enough buffered */
        Shell_CharOut(0, 'm');
		memcpy(buf, fp->bufpos, bytes);
		fp->bufpos += bytes;
		return nelm;
	} else if (len > 0) {	/* Some buffered */
        Shell_CharOut(0, 'n');
		memcpy(buf, fp->bufpos, len);
		got = len;
		fp->bufpos += bytes;
	}
	/* Need more; do it with a direct read */
    Shell_CharOut(0, 'o');
	len = read(fp->fd, (char *) buf + got, bytes - got);
	/* Possibly for now _or_ later */
	if (len < 0) {
		fp->mode |= __MODE_ERR;
		len = 0;
	} else if (len == 0)
		fp->mode |= __MODE_EOF;
	return (got + len) / size;
}
