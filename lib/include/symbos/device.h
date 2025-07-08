#ifndef _SYMCORE
#include <symbos/core.h>
#endif

#ifndef _SYMDEVICE
#define _SYMDEVICE

#define BUTTON_LEFT 1
#define BUTTON_RIGHT 2
#define BUTTON_MIDDLE 4

#define SHIFT_DOWN 1
#define CTRL_DOWN 2
#define ALT_DOWN 4
#define CAPSLOCK_DOWN 256

#define TYPE_OTHER 0
#define TYPE_CPC 0x01
#define TYPE_ENTERPRISE 0x02
#define TYPE_MSX 0x04
#define TYPE_PCW 0x08
#define TYPE_NC 0x10
#define TYPE_SVM 0x20
#define TYPE_CPC464 0x101
#define TYPE_CPC664 0x201
#define TYPE_CPC6128 0x301
#define TYPE_CPC464PLUS 0x401
#define TYPE_CPC6128PLUS 0x501
#define TYPE_MSX1 0x104
#define TYPE_MSX2 0x204
#define TYPE_MSX2PLUS 0x304
#define TYPE_MSXTURBOR 0x404
#define TYPE_PCW8 0x108
#define TYPE_PCW9 0x208
#define TYPE_PCW16 0x408
#define TYPE_NC100 0x110
#define TYPE_NC150 0x210
#define TYPE_NC200 0x410

#define DRIVE_NONE 0
#define DRIVE_READY 1
#define DRIVE_NOTREADY 2
#define DRIVE_CORRUPT 3

#define TYPE_FLOPPY_SS 1
#define TYPE_FLOPPY_DS 2
#define TYPE_RAMDRIVE 3
#define TYPE_HARDDRIVE 4

#define ScreenColours(x) ScreenColors(x)
extern unsigned char Screen_Mode(void);
extern unsigned char Screen_Colors(void);
extern unsigned short Screen_Width(void);
extern unsigned short Screen_Height(void);

extern unsigned short Mouse_X(void);
extern unsigned short Mouse_Y(void);
extern unsigned char Mouse_Buttons(void);
extern unsigned char Mouse_Dragging(unsigned char delay);

extern unsigned char Key_Down(unsigned char scancode);
extern unsigned short Key_Status(void);
extern unsigned char Key_Put(unsigned char keychar);
extern unsigned char Key_Multi(unsigned char scancode1, unsigned char scancode2,
                               unsigned char scancode3, unsigned char scancode4,
                               unsigned char scancode5, unsigned char scancode6);

extern void Screen_Mode_Set(char mode, char force, char vwidth);
#define Colour_Get(x) Color_Get(x)
extern unsigned short Color_Get(char color);
#define Colour_Set(x, y) ColorSet(x, y)
extern void Color_Set(char color, unsigned short value);
extern void Screen_Redraw(void);

extern unsigned short Text_Width(unsigned char bank, char* addr, int maxlen);
extern unsigned char Text_Height(unsigned char bank, char* addr, int maxlen);

extern signed char Print_Busy(void);
extern signed char Print_Char(unsigned char ch);
extern signed char Print_String(char* str);

typedef struct {
    unsigned char letter;
    unsigned short config;
    unsigned char unused;
    char name[12];
} Device_Info;

typedef struct {
    unsigned char status;
    unsigned char type;
    unsigned char removeable;
    unsigned char fs;
    unsigned char sectors;
    unsigned long clusters;
} Drive_Info;

#define Sys_Version() _sysversion
extern void Sys_GetConfig(char* dest, unsigned short offset, unsigned char len);
extern void Sys_GetDrives(void* dest);
extern void Sys_DriveInfo(char letter, Drive_Info* obj);
extern unsigned long Sys_DriveFree(char letter);
extern char* Sys_Path(void);
extern unsigned short Sys_Type(void);

#endif
