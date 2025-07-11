#ifndef _SYMCORE
#include <symbos/core.h>
#endif

#ifndef _SYMFILE
#define _SYMFILE

#define ATTRIB_READONLY 1
#define ATTRIB_HIDDEN 2
#define ATTRIB_SYSTEM 4
#define ATTRIB_VOLUME 8
#define ATTRIB_DIR 16
#define ATTRIB_ARCHIVE 32

#define TIME_MODIFIED 1
#define TIME_CREATED 2

#define SEEK_SET 0
#define SEEK_CUR 1
#define SEEK_END 2

#define ERR_DEVINIT 18
#define ERR_DAMAGED 19
#define ERR_NOPART 20
#define ERR_UNPART 21
#define ERR_READ 22
#define ERR_SEEK 23
#define ERR_ABORT 24
#define ERR_NOVOL 25
#define ERR_TOOMANY 26
#define ERR_NODEV 27
#define ERR_NOPATH 28
#define ERR_NOFILE 29
#define ERR_FORBIDDEN 30
#define ERR_BADNAME 31
#define ERR_NOHANDLE 32
#define ERR_DEVSLOT 33
#define ERR_FILEORG 34
#define ERR_BADDEST 35
#define ERR_EXISTS 36
#define ERR_BADCODE 37
#define ERR_BADATTRIB 38
#define ERR_DIRFULL 39
#define ERR_DISKFULL 40
#define ERR_PROTECT 41
#define ERR_NOTREADY 42
#define ERR_NOTEMPTY 43
#define ERR_BADDEV 44
#define ERR_FILESYS 45
#define ERR_UNDEV 46
#define ERR_RONLY 47
#define ERR_NOCHANNEL 48
#define ERR_NOTDIR 49
#define ERR_NOTFILE 50
#define ERR_UNDEFINED 15

extern unsigned char _fileerr;

extern unsigned char File_New(unsigned char bank, char* path, unsigned char attrib);
extern unsigned char File_Open(unsigned char bank, char* path);
extern unsigned char File_Close(unsigned char id);
extern unsigned short File_Read(unsigned char id, unsigned char bank, char* addr, unsigned short len);
extern unsigned char File_ReadLine(unsigned char id, unsigned char bank, char* addr);
extern unsigned short File_ReadComp(unsigned char id, unsigned char bank, char* addr, unsigned short len);
extern unsigned short File_Write(unsigned char id, unsigned char bank, char* addr, unsigned short len);
extern void File_ErrMsg(void* modalWin);
extern long File_Seek(unsigned char id, long offset, unsigned char ref);

typedef struct {
    long len;
    unsigned long time;
    unsigned char attrib;
    char name[13];
} DirEntry;

extern unsigned char Dir_SetAttrib(unsigned char bank, char* path, unsigned char attrib);
extern signed char Dir_GetAttrib(unsigned char bank, char* path);
extern unsigned long Dir_GetTime(unsigned char bank, char* path, unsigned char which);
extern unsigned char Dir_SetTime(unsigned char bank, char* path, unsigned char which,
                                 unsigned long timestamp);
extern unsigned char Dir_Rename(unsigned char bank, char* path, char* newname);
extern unsigned char Dir_New(unsigned char bank, char* path);
extern int Dir_Read(char* path, unsigned char attrib, void* buf, unsigned short len, unsigned short skip);
extern int Dir_ReadRaw(unsigned char bank, char* path, unsigned char attrib,
                       unsigned char bufbank, void* addr, unsigned short len,
                       unsigned short skip);
extern int Dir_ReadExt(unsigned char bank, char* path, unsigned char attrib,
                    unsigned char bufbank, void* addr, unsigned short len,
                    unsigned short skip, unsigned char cols);
extern unsigned char Dir_Delete(unsigned char bank, char* path);
extern unsigned char Dir_DeleteDir(unsigned char bank, char* path);
extern unsigned char Dir_Move(unsigned char bank, char* pathSrc, char* pathDst);

extern char* Dir_PathAdd(char* path, char* addition, char* dest);

#endif
