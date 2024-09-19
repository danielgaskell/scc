#ifndef __VARARGS_H
#define __VARARGS_H

#ifndef __STDARG_H
typedef void *va_list;

#define va_dcl va_list va_alist;
#define va_start(a)  (a) = (va_list)&va_alist
#define va_arg(a,t)  ((t *)(((char *)(a)) += sizeof(t)))[-1]
#define va_end(a)    (a) = NULL

#endif
#endif
