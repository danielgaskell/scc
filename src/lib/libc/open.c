#include <symbos.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include "iobuf.h"

extern void* _exit_hooks[];
unsigned char open_needs_hook = 1;
unsigned char _pfds[8]; // open file handles - SymbOS only allows 8 at a time

// in same file because we always need it if we use open()
int close(int fd) {
    unsigned char i;
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
    for (i = 0; i < 8; ++i) {
        if (_pfds[i] == fd) {
            _pfds[i] = 0;
            break;
        }
    }
    return 0;
}

void __posix_close_all(void) {
    unsigned char i;
    for (i = 0; i < 8; ++i) {
        if (_pfds[i])
            close(_pfds[i]);
    }
}

int open(const char *path, int flags, ...) {
    unsigned char fd, i;

    /* save address of __posix_close_all to exit hooks (avoids it being linked unless actually needed) */
	if (open_needs_hook) {
        for (i = 0; i < 8; ++i) {
            if (!_exit_hooks[i]) {
                _exit_hooks[i] = __posix_close_all;
                open_needs_hook = 0;
                break;
            }
        }
	}

	if (_shellpid)
        Shell_PathAdd(_symbank, 0, path, _io_buf); // relative to shell path if shell and unspecified
    else
        Dir_PathAdd(0, path, _io_buf); // relative to EXE path if not shell and unspecified
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
        if (_fileerr == ERR_FORBIDDEN)
            errno = EACCES;
        else if (_fileerr == ERR_TOOMANY)
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

    for (i = 0; i < 8; ++i) {
        if (!_pfds[i]) {
            _pfds[i] = fd + 3;
            break;
        }
    }

    return fd + 3;
}
