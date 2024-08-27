#include <symbos.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>

ssize_t read(int fd, void *buf, int len) {
    char* ptr;
    int c;
    int readlen;

    switch (fd) {
    case 0: // stdin
        if (_shellpid) {
            ptr = buf;
            readlen = 0;
            while (readlen < len) {
                c = Shell_CharIn(0);
                if (c == -1) {
                    break;
                } else if (c == -2) {
                    errno = EIO;
                    return -1;
                } else {
                    *ptr++ = (char)c;
                    ++readlen;
                    if (fd == 0)
                        Shell_CharOut(0, c); // FIXME: only echo for keyboard in, not pipe
                }
                break;  // FIXME: currently only returns one character - if there's
                        // a way to distinguish the keyboard from a pipe, can
                        // improve edge cases by reading more
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
        readlen = File_Read(fd - 3, _symbank, buf, len);
        if (_fileerr) {
            if (_fileerr == ERR_NOHANDLE)
                errno = EBADF;
            else
                errno = EIO;
            return -1;
        } else {
            return readlen;
        }

    }
}
