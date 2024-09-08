#ifndef _SYMCORE
#include <symcore.h>
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

#define KEY_BACK 8
#define KEY_TAB 9
#define KEY_ENTER 13
#define KEY_RETURN 13
#define KEY_ESC 27
#define KEY_DEL 127
#define KEY_UP 136
#define KEY_DOWN 137
#define KEY_LEFT 138
#define KEY_RIGHT 139
#define KEY_F0 140
#define KEY_F1 141
#define KEY_F2 142
#define KEY_F3 143
#define KEY_F4 144
#define KEY_F5 145
#define KEY_F6 146
#define KEY_F7 147
#define KEY_F8 148
#define KEY_F9 149
#define KEY_FDOT 150
#define KEY_ALT_AT 151
#define KEY_ALT_A 152
#define KEY_ALT_B 153
#define KEY_ALT_C 154
#define KEY_ALT_D 155
#define KEY_ALT_E 156
#define KEY_ALT_F 157
#define KEY_ALT_G 158
#define KEY_ALT_H 159
#define KEY_ALT_I 160
#define KEY_ALT_J 161
#define KEY_ALT_K 162
#define KEY_ALT_L 163
#define KEY_ALT_M 164
#define KEY_ALT_N 165
#define KEY_ALT_O 166
#define KEY_ALT_P 167
#define KEY_ALT_Q 168
#define KEY_ALT_R 169
#define KEY_ALT_S 170
#define KEY_ALT_T 171
#define KEY_ALT_U 172
#define KEY_ALT_V 173
#define KEY_ALT_W 174
#define KEY_ALT_X 175
#define KEY_ALT_Y 176
#define KEY_ALT_Z 177
#define KEY_ALT_0 178
#define KEY_ALT_1 179
#define KEY_ALT_2 180
#define KEY_ALT_3 181
#define KEY_ALT_4 182
#define KEY_ALT_5 183
#define KEY_ALT_6 184
#define KEY_ALT_7 185
#define KEY_ALT_8 186
#define KEY_ALT_9 187

#define SCAN_0 32
#define SCAN_1 64
#define SCAN_2 65
#define SCAN_3 57
#define SCAN_4 56
#define SCAN_5 49
#define SCAN_6 48
#define SCAN_7 41
#define SCAN_8 40
#define SCAN_9 33
#define SCAN_A 69
#define SCAN_B 54
#define SCAN_C 62
#define SCAN_D 61
#define SCAN_E 58
#define SCAN_F 53
#define SCAN_G 52
#define SCAN_H 44
#define SCAN_I 35
#define SCAN_J 45
#define SCAN_K 37
#define SCAN_L 36
#define SCAN_M 38
#define SCAN_N 46
#define SCAN_O 34
#define SCAN_P 27
#define SCAN_Q 67
#define SCAN_R 50
#define SCAN_S 60
#define SCAN_T 51
#define SCAN_U 42
#define SCAN_V 55
#define SCAN_W 59
#define SCAN_X 63
#define SCAN_Y 43
#define SCAN_Z 71
#define SCAN_F0 15
#define SCAN_F1 13
#define SCAN_F2 14
#define SCAN_F3 5
#define SCAN_F4 20
#define SCAN_F5 12
#define SCAN_F6 4
#define SCAN_F7 10
#define SCAN_F8 11
#define SCAN_F9 3
#define SCAN_FDOT 7
#define SCAN_UP 0
#define SCAN_DOWN 2
#define SCAN_LEFT 8
#define SCAN_RIGHT 1
#define SCAN_ALT 9
#define SCAN_AT 26
#define SCAN_BSLASH 22
#define SCAN_CAPSLOCK 70
#define SCAN_CARET 24
#define SCAN_CLR 16
#define SCAN_COLON 29
#define SCAN_COMMA 39
#define SCAN_CTRL 23
#define SCAN_DEL 79
#define SCAN_ENTER 6
#define SCAN_ESC 66
#define SCAN_FIRE_1 77
#define SCAN_FIRE_2 76
#define SCAN_JOY_DOWN 73
#define SCAN_JOY_LEFT 74
#define SCAN_JOY_RIGHT 75
#define SCAN_JOY_UP 72
#define SCAN_LBRACKET 17
#define SCAN_MINUS 25
#define SCAN_PERIOD 31
#define SCAN_RBRACKET 19
#define SCAN_RETURN 18
#define SCAN_SEMICOLON 28
#define SCAN_SHIFT 21
#define SCAN_SLASH 30
#define SCAN_SPACE 47
#define SCAN_TAB 68

#define ScreenColours(x) ScreenColors(x)
extern unsigned char Screen_Mode(void);
extern unsigned char Screen_Colors(void);
extern unsigned short Screen_Width(void);
extern unsigned short Screen_Height(void);

extern unsigned short Mouse_X(void);
extern unsigned short Mouse_Y(void);
extern unsigned char Mouse_Buttons(void);

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

#endif
