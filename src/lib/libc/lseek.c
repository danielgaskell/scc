#include <symbos.h>
#include <stdlib.h>
#include <errno.h>

/* Correct POSIX behavior for seeking past end of a sparse file is to only fill
   in the bytes with 0 on write, not read, but since SymbOS doesn't make a
   distinction we always fill it in. */
off_t lseek(int fd, off_t pos, int whence) {
    off_t e;
    off_t expected = 0;
    unsigned char sparsefill;
    if (fd < 3) {
        errno = ESPIPE;
        return -1;
    }
    fd -= 3;
    if (whence == SEEK_SET)
        expected = pos;
    else if (pos > 0)
        expected = File_Seek(fd, 0, whence) + pos;
    e = File_Seek(fd, pos, whence);
    if (_fileerr == ERR_NOHANDLE)
        errno = EBADF;
    if (e < expected) { // seek past end of sparse file, fill in with zeros
        e = expected - File_Seek(fd, 0, SEEK_END); // warning: may leave some intervening junk on AMSDOS filesystems!
        while (e > 0) {
            sparsefill = (e > 64) ? 64 : e;
            File_Write(fd, _symbank, "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00", sparsefill);
            e -= sparsefill;
        }
        e = expected;
    }
    return e;
}
