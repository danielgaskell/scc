#include <symbos.h>
#include <errno.h>

int close(int fd) {
    char e;
    if (fd < 3) {
        errno = EBADF;
        return -1;
    }
    e = File_Close(fd - 3);
    if (e) {
        if (_fileerr == ERR_NOHANDLE)
            errno = EBADF;
        else
            errno = EIO;
        return -1;
    }
    return 0;
}
