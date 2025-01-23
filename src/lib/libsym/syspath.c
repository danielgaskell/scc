#include <symbos.h>
#include <string.h>

_transfer char _syspath[33];

char* Sys_Path(void) {
    unsigned char i;
    Sys_GetConfig(_syspath, 0, 32);
    i = strlen(_syspath) - 1;
    if (_syspath[i] == '/') {
        _syspath[i] == '\\';
    } else if (_syspath[i] != '\\') {
        _syspath[i++] = '\\';
        _syspath[i] = 0;
    }
    return _syspath;
}
