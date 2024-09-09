#include <symbos.h>
#include <errno.h>
#include "iobuf.h"

int unlink(const char *path) {
    unsigned char result;
    if (_shellpid)
        Shell_PathAdd(_symbank, 0, path, _io_buf); // relative to shell path if shell and unspecified
    else
        Dir_PathAdd(0, path, _io_buf); // relative to EXE path if not shell and unspecified
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
