#include <symbos.h>
#include <string.h>

void Dir_ReadFmt(void* buf, unsigned short len, unsigned short count) {
    char* ptr_in = buf;
    char* ptr_out = buf + (count - 1)*sizeof(DirEntry);
    unsigned char skipped, skipped2, first;
    unsigned short count2 = count;

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
    while (count > 1) {
        skipped2 = *(ptr_in - 1) - 128;
        *(ptr_in - 1) = 0;
        memmove(ptr_out, ptr_in, sizeof(DirEntry));
        ptr_in -= skipped;
        ptr_out -= sizeof(DirEntry);
        skipped = skipped2;
        --count;
    }
}
