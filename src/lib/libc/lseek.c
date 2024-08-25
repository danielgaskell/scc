#include <symbos.h>
#include <stdlib.h>
#include <errno.h>

off_t lseek(int fd, off_t pos, int whence)
{
  int e;
  if (fd < 3) {
    errno = ESPIPE;
    return -1;
  }
  e = File_Seek(fd - 3, pos, whence);
  if (_fileerr == ERR_NOHANDLE)
    errno = EBADF;
  return (off_t)e;
}
