#include <symbos.h>

int _argc;
char* _argv[48];
unsigned char _shellerr;
unsigned char _shellpid;
unsigned char _shellwidth;
unsigned char _shellheight;
unsigned char _shellver;

void _load_argv(void) {
    char* ptr = _segcode + _segcodelen - 0x100;
    unsigned char in_quotes = 0;
    _argc = 1;
    _argv[0] = ptr;
    _shellpid = 0;
    while (*ptr != 0) {
        if (*ptr == ' ' && in_quotes == 0) {
            while (*ptr == ' ')
                *ptr++ = 0;
            if (*ptr == '"') {
                in_quotes = 1;
                ++ptr;
            }
            _argv[_argc++] = ptr;
            if (in_quotes == 0 && ptr[0] == '%' && ptr[1] == 's' && ptr[2] == 'p') {
                // found shell ID, parse it
                _shellpid = (ptr[3]-'0')*10 + (ptr[4]-'0');
                _shellwidth = (ptr[5]-'0')*10 + (ptr[6]-'0');
                _shellheight = (ptr[7]-'0')*10 + (ptr[8]-'0');
                _shellver = (ptr[9]-'0')*10 + (ptr[10]-'0');
                --_argc; // do not include this in the argument count
                break;
            }
        } else if (*ptr == '"') {
            in_quotes ^= 1;
            *ptr++ = 0;
        } else {
            ++ptr;
        }
    }
}
