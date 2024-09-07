#include <symbos.h>
#include <errno.h>
#include "iobuf.h"

int unlink(const char *path) {
    unsigned char result;
    Dir_PathAdd(0, path, _io_buf);
    if (Dir_Delete(_symbank, _io_buf) == 0)
        return 0;
    if (_fileerr == ERR_FORBIDDEN)
        errno = EACCES;
    else if (_fileerr == ERR_RONLY)
        errno = EROFS;
    else
        errno = ENOENT;
    return -1;
}
