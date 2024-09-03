#include "stdio-l.h"

int fputs(const char *s, FILE * fp)
{
	int n = 0;

	while (*s) {
		if (putc(*s++, fp) == EOF)
			return (EOF);
		++n;
	}
	if (fp->fd < 3)
        fflush(fp); // flush stdin, stdout, stderr
	return (n);
}
