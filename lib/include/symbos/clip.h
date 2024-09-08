#ifndef _SYMCORE
#include <symcore.h>
#endif

#ifndef _SYMCLIP
#define _SYMCLIP

extern unsigned char Clip_Put(unsigned char bank, char* addr, unsigned short len, unsigned char type);
extern unsigned short Clip_Get(unsigned char bank, char* addr, unsigned short len, unsigned char type);
extern unsigned char Clip_Type(void);
extern unsigned short Clip_Len(void);

#endif
