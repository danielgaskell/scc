#include <symbos.h>
#include <unistd.h>
#include <errno.h>
#include "iobuf.h"

char _accessbuf[24];

int access(const char *path, int way) {
    int result;
    unsigned char attribs = ATTRIB_VOLUME | ATTRIB_DIR;

    if (*path == 0) {
        errno = ENOENT;
        return -1;
    }

    if (_shellpid)
        Shell_PathAdd(_symbank, 0, path, _io_buf);
    else
        Dir_PathAdd(0, path, _io_buf);
    if (way & W_OK)
        attribs |= ATTRIB_READONLY; // note: only checks if file is read-only, not if disk is read-only

    if (Dir_ReadRaw(_symbank, _io_buf, attribs, _symbank, _accessbuf, 24, 0))
        return 0;
    if (_fileerr == ERR_NOPATH || _fileerr == ERR_BADNAME)
        errno = ENOTDIR;
    else
        errno = ENOENT;
    return -1;
}
