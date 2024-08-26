#include <fcntl.h>

extern int open(const char *path, int flags, ...);

int creat(const char *path, mode_t mode) {
    return open(path, O_WRONLY | O_CREAT | O_TRUNC, mode);
}
