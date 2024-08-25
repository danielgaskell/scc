#include <symbos.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>

char _write_buf[256];

ssize_t write(int fd, const void *buf, int len) {
    char* ptr_in;
    char* ptr_out;
    ssize_t writelen;
    unsigned char this_len;

    switch (fd) {
    case 0: // stdin (invalid)
        errno = ENXIO;
        return -1;

    case 1: case 2: // stdout, stderr (no distinction currently)
        if (_shellpid) {
            // shell is registered, send it there in chunks
            writelen = strlen(buf);
            ptr_in = buf;
            ptr_out = _write_buf;
            this_len = writelen > 255 ? 255 : writelen;
            while (writelen > 0) {
                if (*ptr_in == '\n') { // convert \n to \r\n
                    *ptr_out++ = '\r';
                    *ptr_out++ = '\n';
                    ++ptr_in;
                } else {
                    *ptr_out++ = *ptr_in++;
                }
                --this_len;
                if (this_len == 0) {
                    *ptr_out = 0;
                    Shell_StringOut(0, _symbank, _write_buf, 255);
                    ptr_out = _write_buf;
                    this_len = 255;
                }
                --writelen;
            }
            *ptr_out = 0;
            Shell_StringOut(0, _symbank, _write_buf, 255 - this_len);
            return len;
        } else {
            errno = ENXIO;
            return -1;
        }

    default: // normal file
        writelen = File_Write(fd - 3, _symbank, buf, len);
        if (_fileerr) {
            errno = EIO; // FIXME more specific errors?
            return -1;
        } else {
            return writelen;
        }
    }
}
