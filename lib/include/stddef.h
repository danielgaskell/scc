#ifndef _STDDEF_H
#define	_STDDEF_H

#include <stdint.h>

#define	NULL ((void *)0)

#define	offsetof(t, i)	((size_t) &((t *)0)->i)

#endif
