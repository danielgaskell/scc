#ifndef _SYMCORE
#include <symbos/core.h>
#endif

#ifndef _SYMTHREADS
#define _SYMTHREADS

extern signed char thread_start(void* routine, char* env, unsigned short envlen);
extern void thread_quit(char* env);

#endif
