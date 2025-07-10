#ifndef _SYMCORE
#include <symbos/core.h>
#endif

#ifndef _SYMPROC
#define _SYMPROC

#define APPERR_NOFILE 0
#define APPERR_UNKNOWN 1
#define APPERR_LOAD 2
#define APPERR_NOMEM 3

typedef struct {
	unsigned short ix;
	unsigned short iy;
	unsigned short hl;
	unsigned short de;
	unsigned short bc;
	unsigned short af;
	void* startAddr;
	unsigned char pid;
} ProcHeader;

extern signed char Timer_Add(unsigned char bank, void* header);
extern signed char Timer_Wake(unsigned char pid, unsigned char msgid, unsigned short cycles);
extern void Timer_Delete(unsigned char id);
extern unsigned char Counter_Add(unsigned char bank, char* addr, unsigned char pid, unsigned char speed);
extern void Counter_Delete(unsigned char bank, char* addr);
extern void Counter_Clear(unsigned char pid);
extern signed char Proc_Add(unsigned char bank, void* header, unsigned char priority);
extern void Proc_Delete(unsigned char pid);
extern void Proc_Sleep(unsigned char pid);
extern void Proc_Wake(unsigned char pid);
extern void Proc_Priority(unsigned char pid, unsigned char priority);
extern unsigned short App_Run(char bank, char* path, char suppress);
extern void App_End(char appID);
extern unsigned short App_Search(char bank, char* idstring);
extern unsigned short App_Service(char bank, char* idstring);
extern void App_Release(char appID);

#endif
