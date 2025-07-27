#include <symbos.h>
#include <string.h>

int Dir_Read(char* path, unsigned char attrib, void* buf, unsigned short len, unsigned short skip) {
    char *ptr_in, *ptr_out;
    unsigned char skipped, first;
    int count, count2, count3;

    // do directory read
    count = Dir_ReadRaw(_symbank, path, attrib, _symbank, buf, len, skip);
    if (count < 0)
        return count;
    ptr_in = buf;
    ptr_out = buf + (count - 1)*sizeof(DirEntry);
    count2 = count;
    count3 = count;

    // find end of original buffer
    first = 1;
    while (count2 > 1) {
        ptr_in += 9;
        skipped = 10;
        while (*ptr_in) {
            ++ptr_in;
            ++skipped;
        }
        if (first)
            first = 0;
        else
            *ptr_in = skipped + 128; // save length, for backtracking later
        ++ptr_in;
        --count2;
    }

    // copy from end to start
    while (count3 > 1) {
        skipped = *(ptr_in - 1) - 128;
        *(ptr_in - 1) = 0;
        memmove(ptr_out, ptr_in, sizeof(DirEntry));
        ptr_in -= skipped;
        ptr_out -= sizeof(DirEntry);
        --count3;
    }

    return count;
}
