#include <string.h>

extern char* _fnum(double val, char fmt, int prec);

char* ftoa(float __value, char *__strP) {
    return strcpy(__strP, _fnum(__value, 'f', -1));
}
