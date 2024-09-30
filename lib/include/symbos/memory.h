#ifndef _SYMCORE
#include <symbos/core.h>
#endif

#ifndef _SYMMEMORY
#define _SYMMEMORY

extern unsigned long Mem_Free(void);
extern unsigned char Mem_Banks(void);
extern unsigned short Mem_Longest(unsigned char bank, unsigned char type);
extern unsigned char Mem_Reserve(unsigned char bank, unsigned char type, unsigned short len, unsigned char* bankVar, char** addrVar);
extern void Mem_Release(unsigned char bank, char* addr, unsigned short len);
extern unsigned char Mem_Resize(unsigned char bank, char* addr, unsigned short oldlen, unsigned short newlen);
extern unsigned char Mem_ResizeX(unsigned char bank, unsigned char type, char* addr,
                                 unsigned short oldlen, unsigned short newlen,
                                 unsigned char* bankVar, char** addrVar);

extern unsigned short Bank_ReadWord(unsigned char bank, char* addr);
extern void Bank_WriteWord(unsigned char bank, char* addr, unsigned short val);
extern unsigned char Bank_ReadByte(unsigned char bank, char* addr);
extern void Bank_WriteByte(unsigned char bank, char* addr, unsigned char val);
extern void Bank_Copy(unsigned char bankDst, char* addrDst, unsigned char bankSrc, char* addrSrc, unsigned short len);
extern unsigned char Bank_Get(void);
extern void Bank_Decompress(unsigned char bank, char* addrDst, char* addrSrc);

#endif
