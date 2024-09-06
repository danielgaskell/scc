/* perror.c
 */
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <symbos.h>

void perror(const char *str)
{
	if (!str)
		str = "error";
	Shell_Print(str);
	Shell_Print(": ");
	str = strerror(errno);
	Shell_Print(str);
	Shell_Print("\r\n");
}
