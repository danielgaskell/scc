#ifndef __SETJMP_H
#define __SETJMP_H
#ifndef __TYPES_H
#include <types.h>
#endif

typedef unsigned jmp_buf[5];	/*  IY IX BC (sp) and sp */
extern int _setjmp(jmp_buf __env);
extern void longjmp(jmp_buf __env, int __rv);
#define setjmp(x) _setjmp(x)

#endif
