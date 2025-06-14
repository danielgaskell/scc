#ifndef _SYMCORE
#include <symbos/core.h>
#endif

#ifndef _SYMPOPUPS
#define _SYMPOPUPS

#define BUTTON_OK 1
#define BUTTON_YN 2
#define BUTTON_YNC 3
#define TITLE_DEFAULT 0
#define TITLE_ERROR 8
#define TITLE_INFO 16
#define TITLE_WARNING 24
#define TITLE_CONFIRM 32
#define MSGBOX_MODAL 64
#define MSGBOX_ICON 128

#define MSGBOX_FAILED 0
#define MSGBOX_OK 2
#define MSGBOX_YES 3
#define MSGBOX_NO 4
#define MSGBOX_CANCEL 5

#define FILEBOX_OPEN 0
#define FILEBOX_SAVE 64
#define FILEBOX_FILE 0
#define FILEBOX_DIR 128

#define FILEBOX_OK 0
#define FILEBOX_CANCEL 1
#define FILEBOX_FAILED 2
#define FILEBOX_NOMEM 3
#define FILEBOX_NOWIN 4

#ifndef ATTRIB_READONLY
#define ATTRIB_READONLY 1
#define ATTRIB_HIDDEN 2
#define ATTRIB_SYSTEM 4
#define ATTRIB_VOLUME 8
#define ATTRIB_DIR 16
#define ATTRIB_ARCHIVE 32
#endif

extern char FileBoxExt[4];
extern char FileBoxPath[256];

extern unsigned char MsgBox(char* line1, char* line2, char* line3, unsigned int pen,
                            unsigned char type, char* icon, void* modalWin);
extern signed char InputBox(char* title, char* line1, char* line2, char* buffer,
                            unsigned short buflen, void* modalWin);
extern unsigned char FileBox(char* path, char* filter, unsigned char flags, unsigned char attribs,
                             unsigned short entries, unsigned short bufsize, void* modalWin);
extern void Menu_Context(unsigned char bank, void* addr, int x, int y);
extern char Select_Pos(unsigned short* x, unsigned short* y, unsigned short w, unsigned short h);
extern char Select_Size(unsigned short x, unsigned short y, unsigned short* w, unsigned short* h);

#endif
