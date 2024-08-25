/* isatty.c
 */

// is fd a tty? we assume yes for stdin etc., no for everything else
int isatty(int fd)
{
	if (fd < 3)
		return 1;
	return 0;
}
