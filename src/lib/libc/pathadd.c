#include <string.h>

extern char* _argv[];

char* Dir_PathAdd(char* path, char* addition, char* dest) {
    char* ptr;
    char* ptr2;

    // set root
    if (addition[0] == '\\' || addition[1] == '/') {
        // addition is a complete path (without drive)
        dest[0] = _argv[0][0];
        dest[1] = 0;
        strcat(dest, ":\\");
    } else if (addition[1] == ':') {
        // addition is a complete path (with drive)
        dest[0] = 0;
    } else if (path == NULL) {
        // mode 0: relative to EXE path
        strcpy(dest, _argv[0]);
        *(strrchr(dest, '\\') + 1) = 0;
    } else {
        // otherwise: relative to root path
        strcpy(dest, path);
        strcat(dest, "\\");
    }
    strcat(dest, addition);
    ptr = dest + 2;
    ptr2 = ptr;

    // clean up path
    while (*ptr) {
        if (*ptr == '/')
            *ptr = '\\';
        if (ptr[0] == '\\' && (ptr[1] == '\\' || *(ptr2 - 1) == '\\')) {
            ++ptr;
            continue;
        } else if (ptr[0] == '.' && ptr[1] == '\\') {
            ++ptr;
            continue;
        } else if (ptr[0] == '.' && ptr[1] == '.' && ptr[2] == '\\') {
            --ptr2;
            while (*(ptr2 - 1) != '\\')
                --ptr2;
            ptr += 2;
            continue;
        }
        *ptr2++ = *ptr++;
    }
    while (*(ptr2 - 1) == '\\') // do not end on a backslash
        --ptr2;
    *ptr2 = 0;
    return dest;
}
