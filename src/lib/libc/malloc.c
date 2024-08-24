#include <symbos.h>
#include <types.h>
#include <malloc.h>

// Adapted from an implementation by Sylvain Defresne and Kingkong Jnr, CC-BY.

typedef struct _free_block {
    size_t size;
    struct _free_block* next;
} _free_block;

// These values are loaded by crt0.
extern char* _malloc_heap;
extern char* _malloc_top;
extern char* _malloc_max;
static _free_block _malloc_free = {0, 0};

void* malloc(size_t size) {
    _free_block* block;
    _free_block** head;

    size = (size + sizeof(_free_block) + (HEAP_ALIGN - 1)) & ~ (HEAP_ALIGN - 1);
    block = _malloc_free.next;
    head = &(_malloc_free.next);
    while (block != 0) {
        if (block->size >= size) {
            *head = block->next;
            return ((char*)block) + sizeof(_free_block);
        }
        head = &(block->next);
        block = block->next;
    }

    if (_malloc_top + size >= _malloc_max)
        return NULL; // no space left

    block = (_free_block*)_malloc_top;
    _malloc_top = _malloc_top + size;
    block->size = size;
    block->next = 0;

    return ((char*)block) + sizeof(_free_block);
}

void *realloc(void *ptr, size_t size) {
    void* newptr;
    if (ptr != NULL) {
        newptr = malloc(size);
        if (newptr != NULL) {
            memcpy(newptr, ptr, size);
            free(ptr);
        }
        return newptr;
    } else {
        return malloc(size);
    }
}

void *calloc(size_t nitems, size_t size) {
    void* ptr;
    unsigned int sizebytes;
    sizebytes = nitems * size;
    ptr = malloc(sizebytes);
    if (ptr != NULL)
        memset(ptr, 0, sizebytes);
    return ptr;
}

void free(void* ptr) {
    if (ptr != NULL) {
        _free_block* block = (_free_block*)(((char*)ptr) - sizeof(_free_block));
        block->next = _malloc_free.next;
        _malloc_free.next = block;
    }
}

