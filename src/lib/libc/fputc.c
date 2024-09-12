#include "stdio-l.h"

int fputc(int ch, FILE * fp)
{
	int v;

	v = fp->mode;
	/* If last op was a read ... */
	if ((v & __MODE_READING) && fflush(fp))
		return EOF;
	/* Can't write or there's been an EOF or error then return EOF */
	if ((v & (__MODE_WRITE | __MODE_EOF | __MODE_ERR)) != __MODE_WRITE)
		return EOF;
	/* Buffer is full */
	if (fp->bufpos >= fp->bufend && fflush(fp))
		return EOF;
	/* Right! Do it! */
	*(fp->bufpos++) = ch;
	fp->mode |= __MODE_WRITING;

	/* Correct return val */
	return (unsigned char) ch;
}
