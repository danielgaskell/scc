#ifndef _SYMCORE
#include <symbos/core.h>
#endif

#ifndef _SYMHEADER
#define _SYMHEADER

typedef struct {
    unsigned char bank;
    char* addr;
    unsigned short len;
} _Memory;

typedef struct {
    unsigned short codelen;
    unsigned short datalen;
    unsigned short translen;
    char* segdata;
    char* segtrans;
    unsigned char timers[4];
    unsigned char bank;
    char name[25];
    char flags;
    unsigned short icon16off;
    char unused[5];
    _Memory memory[8];
    unsigned char appid;
    unsigned char pid;
    char iconsmall[19];
    char iconbig[147];
} App_Header;

extern App_Header _symheader;

#endif
