#include <symbos.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>

// FIXME handle EOF throughout?
ssize_t read(int fd, void *buf, int len) {
    char* ptr;
    int c;
    ssize_t readlen;

    switch (fd) {
    case 0: // stdin
        if (_shellpid) {
            ptr = buf;
            readlen = 0;
            while (readlen < len) {
                c = Shell_CharIn(0);
                if (c == -1) {
                    break;
                } else if (_shellerr) {
                    errno = EIO;
                    return -1;
                } else {
                    *ptr++ = (char)c;
                    ++readlen;
                }
            }
            return readlen;
        } else {
            errno = ENXIO;
            return -1;
        }
        break;

    case 1: case 2: // stdout/stderr - invalid
        errno = ENXIO;
        return -1;

    default: // normal file
        readlen = File_Read(fd, _symbank, buf, len);
        if (readlen == 0 && len != 0) {
            if (_fileerr == ERR_NOHANDLE)
                errno = EBADF;
            else
                errno = EIO;
            return -1;
        } else {
            return readlen;
        }
        break;

    }
}
