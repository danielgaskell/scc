#ifndef _SYMCORE
#include <symbos/core.h>
#endif

#ifndef _SYMTIME
#define _SYMTIME

typedef struct {
    unsigned char second;
    unsigned char minute;
    unsigned char hour;
    unsigned char day;
    unsigned char month;
    unsigned short year;
    signed char timezone;
} SymTime;

extern void Time_Get(SymTime* addr);
extern void Time_Set(SymTime* addr);
extern void Time2Obj(unsigned long timestamp, SymTime* obj);
extern unsigned long Obj2Time(SymTime* obj);

#endif
