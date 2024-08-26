#include <symbos.h>
#include <errno.h>
#include <fcntl.h>
#include "iobuf.h"

int open(const char *path, int flags, ...) {
    unsigned char fd;
    Dir_PathAdd(0, path, _io_buf); // relative to EXE path if not specified
    if (flags & O_TRUNC)
        fd = File_New(_symbank, _io_buf, 0);
    else
        fd = File_Open(_symbank, _io_buf);
    if (flags & O_CREAT) {
        if (fd == ERR_NOFILE) {
            fd = File_New(_symbank, _io_buf, 0);
        } else if (fd <= 8 && (flags & O_EXCL)) {
            File_Close(fd); // FIXME probably should use Dir_Read for this to avoid using a filehandle.
            errno = EEXIST;
            return -1;
        }
    }
    if (_fileerr) {
        if (fd == ERR_FORBIDDEN)
            errno = EACCES;
        else if (fd == ERR_TOOMANY)
            errno = ENFILE;
        else if (flags & O_CREAT)
            errno = ENOSPC;
        else
            errno = ENOENT;
        return -1;
    }
    if (flags & O_APPEND)
        File_Seek(fd, 0, SEEK_END);
    errno = 0;
    return fd + 3;
}
