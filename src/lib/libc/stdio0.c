/* stdio.c
 * Copyright (C) 1996 Robert de Bath <rdebath@cix.compulink.co.uk>
 * This file is part of the Linux-8086 C library and is distributed
 * under the GNU Library General Public License.
 */

/* This is an implementation of the C standard IO package. */

#include "stdio-l.h"
#include <symbos.h>

/* This needs to live with __stdio_init vars as they depend upon one another */
int fflush(FILE * fp)
{
	unsigned char *bstart;
	int len, cc, rv = 0;

	if (fp == NULL) {	/* On NULL flush the lot. */
		if (fflush(stdin) || fflush(stdout) || fflush(stderr))
			return EOF;
		fp = __IO_list;
		while (fp) {
			if (fflush(fp))
				return EOF;
			fp = fp->next;
		}
		return 0;
	}
	/* If there's output data pending */
	if (fp->mode & __MODE_WRITING) {
		if ((len = fp->bufpos - fp->bufstart) != 0) {
			bstart = fp->bufstart;
			/* The loop is so we don't get upset by signals
			 * or partial writes.
			 */
			do {
				if ((cc = write(fp->fd, bstart, len)) > 0) {
					bstart += cc;
					len -= cc;
				}
			} while (cc > 0 || (cc == -1 && errno == EINTR));
			/* If we get here with len != 0 there was an error,
			 * exactly what to do about it is another matter ...
			 *
			 * I'll just clear the buffer. */
			if (len) {
				fp->mode |= __MODE_ERR;
				rv = EOF;
			}
		}
	}
	/* If there's data in the buffer sychronise the file positions */
	else if (fp->mode & __MODE_READING) {
		/* Humm, I think this means sync the file like fpurge() ...
		 * Anyway the user isn't supposed to call this function
		 * when reading
		 */
		len = fp->bufread - fp->bufpos;	/* Bytes buffered but unread */
		/* If it's a file, make it good */
		if (len > 0 && lseek(fp->fd, (long) -len, SEEK_CUR) < 0) {
			/* Hummm - Not certain here, I don't think this is reported */
			/* fp->mode |= __MODE_ERR; return EOF; */
		}
	}
	/* All done, no problem */
	fp->mode &=
	    (~
	     (__MODE_READING | __MODE_WRITING | __MODE_EOF |
	      __MODE_UNGOT));
	fp->bufread = fp->bufpos = fp->bufstart;
	return rv;
}


/* TODO sdcc err #define buferr (unsigned char *)(stderr->unbuf)*//* Stderr is unbuffered */

FILE *__IO_list = NULL;		/* For fflush at exit */

unsigned char _bufin[BUFSIZ];
unsigned char _bufout[BUFSIZ];
#ifndef buferr
unsigned char _buferr[BUFSIZ];
#endif

FILE _stdin =
	{_bufin, _bufin, _bufin, _bufin + sizeof(_bufin),
	 0, _IOFBF | __MODE_READ};
FILE* stdin = &_stdin;

FILE _stdout =
	{_bufout, _bufout, _bufout, _bufout + sizeof(_bufout),
	 1, _IOFBF | __MODE_WRITE};
FILE* stdout = &_stdout;

FILE _stderr =
	{_buferr, _buferr, _buferr, _buferr + sizeof(_buferr),
	 2, _IOFBF | __MODE_WRITE};
FILE* stderr = &_stderr;

char marker[8] = {1, 2, 3, 4, 1, 2, 3, 4};

/* Call the stdio initialiser; it's main job it to call atexit */

STATIC void __stdio_close_all(void)
{
	FILE *fp = __IO_list;

	fflush(stdout);
	fflush(stderr);
	while (fp) {
		fflush(fp);
		/* Note we're not de-allocating the memory */
		/* There doesn't seem to be much point :-) */
		/* The actual filehandle close is handled by __posix_close_all(). */
		fp->fd = -1;
		fp = fp->next;
	}
}

/*STATIC void __stdio_init_vars(void)
{
	if (isatty(1)) {
		stdout->mode &= ~_IOFBF;
		stdout->mode |= _IOLBF;
	}
}*/
