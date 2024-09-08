#ifndef _SYMCORE
#include <symcore.h>
#endif

#ifndef _SYMSYSTRAY
#define _SYMSYSTRAY

extern signed char Systray_Add(unsigned char bank, char* addr, unsigned char code);
extern void Systray_Remove(unsigned char id);

#endif
