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

#define Sys_Version() _sysversion
extern void Sys_GetConfig(char* dest, unsigned short offset, unsigned char len);
extern char* Sys_Path(void);

#endif
