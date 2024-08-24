#ifndef __MALLOC_H
#define __MALLOC_H
#ifndef __TYPES_H
#include <types.h>
#endif

extern void *malloc(size_t size);
extern void *realloc(void *ptr, size_t size);
extern void *calloc(size_t nitems, size_t size);
extern void free(void* ptr);

#endif
