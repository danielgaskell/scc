#ifndef __ASSERT_H
#define __ASSERT_H
#ifndef __TYPES_H
#include <types.h>
#endif

/* ANSI compilers only version ! */

/* If NDEBUG is defined, do nothing.
   If not, and EXPRESSION is zero, print an error message and abort.  */

#ifdef	NDEBUG

#ifdef __GNUC__
#define assert(e)		((void)(0))
#else
#define assert(e)		(0)
#endif

#else /* NDEBUG */

extern void __assert(const char *__expr, const char *__file, const int __line);

#define assert(e)	\
  ((void) ((e) || (__assert (__STRING(e),  __FILE__, __LINE__), 0)))

#endif /* NDEBUG */

extern void __errput(const char *__txt);

#endif /* __ASSERT_H */

