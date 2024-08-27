#include <symbos.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include "iobuf.h"

ssize_t write(int fd, const void *buf, int len) {
    char* ptr_in;
    char* ptr_out;
    ssize_t writelen;
    unsigned char this_len;

    Shell_CharOut(0, 'p');
    switch (fd) {
    case 0: // stdin (invalid)
        Shell_CharOut(0, 'q');
        errno = ENXIO;
        return -1;

    case 1: case 2: // stdout, stderr (no distinction currently)
        if (_shellpid) {
            // shell is registered, send it there in chunks
            writelen = len;
            ptr_in = buf;
            ptr_out = _io_buf;
            this_len = 0;
            while (writelen > 0) {
                if (*ptr_in == '\n') { // convert \n to \r\n
                    *ptr_out++ = '\r';
                    *ptr_out++ = '\n';
                    ++this_len;
                } else if (*ptr_in == '\t') { // convert \t to SymShell tab character
                    *ptr_out++ = 25;
                } else {
                    *ptr_out++ = *ptr_in;
                }
                ++ptr_in;
                ++this_len;
                if (this_len >= 254) {
                    *ptr_out = 0;
                    Shell_StringOut(0, _symbank, _io_buf, this_len);
                    ptr_out = _io_buf;
                    this_len = 0;
                }
                --writelen;
            }
            *ptr_out = 0;
            Shell_StringOut(0, _symbank, _io_buf, this_len);
            Shell_CharOut(0, 'r');
            return len;
        } else {
            errno = ENXIO;
            Shell_CharOut(0, 's');
            return -1;
        }

    default: // normal file
        Shell_CharOut(0, 't');
        writelen = File_Write(fd - 3, _symbank, buf, len);
        if (_fileerr) {
            errno = EIO; // FIXME more specific errors?
            Shell_CharOut(0, 'u');
            return -1;
        } else {
            Shell_CharOut(0, 'v');
            return writelen;
        }
    }
}
