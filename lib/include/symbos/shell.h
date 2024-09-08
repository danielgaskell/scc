#ifndef _SYMCORE
#include <symcore.h>
#endif

#ifndef _SYMSHELL
#define _SYMSHELL

#define ERR_NOPROC 14
#define ERR_DEVFULL 13
#define ERR_RINGFULL 12
#define ERR_MOREPROC 11
#define ERR_NOSHELL 10

extern unsigned char _shellerr;
extern unsigned char _shellpid;
extern unsigned char _shellwidth;
extern unsigned char _shellheight;
extern unsigned char _shellver;

extern int Shell_CharIn(unsigned char channel);
extern signed char Shell_StringIn(unsigned char channel, unsigned char bank, char* addr);
extern signed char Shell_CharOut(unsigned char channel, unsigned char val);
extern signed char Shell_StringOut(unsigned char channel, unsigned char bank,
                                   char* addr, unsigned char len);
extern signed char Shell_Print(char* addr);
extern void Shell_Exit(unsigned char type);
extern void Shell_PathAdd(unsigned char bank, char* path, char* addition, char* dest);
extern int Shell_CharTest(unsigned char channel, unsigned char lookahead);

#endif
