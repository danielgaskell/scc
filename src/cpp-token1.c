
#include <stdio.h>
#include <string.h>
#include "cpp-cc.h"

#ifdef __GNUC__
__inline
#endif
static unsigned int hash1(register const char *, register unsigned int);

#include "cpp-token1.h"
