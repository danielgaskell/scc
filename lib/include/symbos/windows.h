#ifndef _SYMCORE
#include <symbos/core.h>
#endif

#ifndef _SYMWINDOWS
#define _SYMWINDOWS

#define COLOR_YELLOW 0
#define COLOR_BLACK 1
#define COLOR_ORANGE 2
#define COLOR_RED 3
#define COLOR_CYAN 4
#define COLOR_DBLUE 5
#define COLOR_LBLUE 6
#define COLOR_BLUE 7
#define COLOR_WHITE 8
#define COLOR_GREEN 9
#define COLOR_LGREEN 10
#define COLOR_MAGENTA 11
#define COLOR_LYELLOW 12
#define COLOR_GRAY 13
#define COLOR_GREY 13
#define COLOR_PINK 14
#define COLOR_LRED 15

#define COLOUR_YELLOW 0
#define COLOUR_BLACK 1
#define COLOUR_ORANGE 2
#define COLOUR_RED 3
#define COLOUR_CYAN 4
#define COLOUR_DBLUE 5
#define COLOUR_LBLUE 6
#define COLOUR_BLUE 7
#define COLOUR_WHITE 8
#define COLOUR_GREEN 9
#define COLOUR_LGREEN 10
#define COLOUR_MAGENTA 11
#define COLOUR_LYELLOW 12
#define COLOUR_GRAY 13
#define COLOUR_GREY 13
#define COLOUR_PINK 14
#define COLOUR_LRED 15

#define WIN_CLOSED 0
#define WIN_NORMAL 1
#define WIN_MAXIMIZED 2
#define WIN_MINIMIZED 3
#define WIN_CENTERED 128

#define WIN_ICON 1
#define WIN_RESIZABLE 2
#define WIN_CLOSE 4
#define WIN_TOOLBAR 8
#define WIN_TITLE 16
#define WIN_MENU 32
#define WIN_STATUS 64
#define WIN_ADJUSTX 256
#define WIN_ADJUSTY 512
#define WIN_NOTTASKBAR 1024
#define WIN_NOTMOVEABLE 2048
#define WIN_MODAL 4096

#define TEXT_RONLY 2

#define ALIGN_LEFT 0
#define ALIGN_RIGHT 1
#define ALIGN_CENTER 2
#define TEXT_FILL 128
#define TEXT_FILL16 64
#define TEXT_16COLOR 128
#define TEXT_16COLOUR 128
#define AREA_FILL 64
#define AREA_XOR 64
#define AREA_16COLOR 128
#define FRAME_XOR 32
#define ICON_STD 0
#define ICON_EXT 16
#define ICON_4COLOR 0
#define ICON_16COLOR 32
#define ICON_EXTOPTS 64
#define ICON_MOVEABLE 128
#define ICON_MARKABLE 1
#define ICON_MARKED 2
#define SLIDER_V 0
#define SLIDER_H 1
#define SLIDER_VALUE 0
#define SLIDER_SCROLL 2
#define CSCROLL_NONE 0
#define CSCROLL_H 1
#define CSCROLL_V 2
#define CSCROLL_BOTH 3
#define INPUT_PASSWORD 1
#define INPUT_READONLY 2
#define INPUT_ALTCOLS 4
#define INPUT_ALTFONT 8
#define INPUT_MODIFIED 128
#define WRAP_WINDOW 2
#define WRAP_WIDTH 3
#define LIST_SCROLL 1
#define LIST_MULTI 2
#define SORT_AUTO 64
#define SORT_REVERSE 128
#define LTYPE_TEXT 0
#define LTYPE_IMAGE 4
#define LTYPE_16 8
#define LTYPE_32 12
#define ROW_MARKED 0x8000
#define MENU_ACTIVE 1
#define MENU_CHECKED 2
#define MENU_SUBMENU 4
#define MENU_SEPARATOR 8

#define SYSTRAY_LEFT 0
#define SYSTRAY_RIGHT 1
#define SYSTRAY_DOUBLE 2

#define C_AREA 0
#define C_TEXT 1
#define C_FRAME 2
#define C_TFRAME 3
#define C_PROGRESS 4
#define C_TEXT_FONT 5
#define C_TEXT_CTRL 6
#define C_IMAGE 8
#define C_ICON 9
#define C_IMAGE_EXT 10
#define C_IMAGE_TRANS 11
#define C_BUTTON 16
#define C_CHECK 17
#define C_RADIO 18
#define C_HIDDEN 19
#define C_TABS 20
#define C_SLIDER 24
#define C_COLLECTION 25
#define C_INPUT 32
#define C_TEXTBOX 33
#define C_LISTTITLE 40
#define C_LISTBOX 41
#define C_DROPDOWN 42
#define C_LISTFULL 43

#define MSR_DSK_WOPNER  160
#define MSR_DSK_WOPNOK  161
#define MSR_DSK_WCLICK  162
#define MSR_DSK_DSKSRV  163
#define MSR_DSK_WFOCUS  164
#define MSR_DSK_CFOCUS  165
#define MSR_DSK_WRESIZ  166
#define MSR_DSK_WSCROL  167
#define MSR_DSK_MENCTX  168
#define MSR_DSK_STIADD  169
#define MSR_DSK_EVTCLK  170
#define MSR_DSK_WMODAL  171
#define MSR_DSK_CONPOS  172
#define MSR_DSK_CONSIZ  173
#define DSK_ACT_CLOSE   5
#define DSK_ACT_MENU    6
#define DSK_ACT_CONTENT 14
#define DSK_ACT_TOOLBAR 15
#define DSK_ACT_KEY     16
#define DSK_SUB_MLCLICK 0
#define DSK_SUB_MRCLICK 1
#define DSK_SUB_MDCLICK 2
#define DSK_SUB_MMCLICK 3
#define DSK_SUB_KEY     7
#define DSK_SUB_MWHEEL  8

extern signed char Win_Open(unsigned char bank, void* addr);
extern void Win_Redraw_Menu(unsigned char winID);
extern void Win_Redraw(unsigned char winID, signed char what, unsigned char first);
extern void Win_Redraw_Toolbar(unsigned char winID, signed char what, unsigned char first);
extern void Win_Redraw_Title(unsigned char winID);
extern void Win_Redraw_Status(unsigned char winID);
extern void Win_ContentX(unsigned char winID, unsigned short newX);
extern void Win_ContentY(unsigned char winID, unsigned short newY);
extern void Win_Focus(unsigned char winID);
extern void Win_Maximize(unsigned char winID);
extern void Win_Minimize(unsigned char winID);
extern void Win_Restore(unsigned char winID);
extern void Win_Move(unsigned char winID, unsigned short newX, unsigned short newY);
extern void Win_Resize(unsigned char winID, unsigned short newW, unsigned short newH);
extern void Win_Close(unsigned char winID);
extern void Win_Redraw_Ext(unsigned char winID, unsigned char what, unsigned char first);
extern void Win_Redraw_Slider(unsigned char winID);
extern void Win_Redraw_Area(unsigned char winID, unsigned char what, unsigned char first,
                            unsigned short x, unsigned short y, unsigned short w, unsigned short h);
extern void Win_Redraw_Sub(unsigned char winID, unsigned char collection, unsigned char control);

extern unsigned char TextBox_Pos(void* win, void* textbox);
extern void TextBox_Redraw(void* win, void* textbox);
extern void TextBox_Select(void* win, void* textbox, int cursor, int selection);

typedef struct {
    unsigned short entries;
} Menu;

typedef struct {
    unsigned short flags;
    char* text;
    unsigned short value;
    unsigned short unused;
} Menu_Entry;

typedef struct {
    signed short xbase;
    unsigned char xmult;
    unsigned char xdiv;
    signed short ybase;
    unsigned char ymult;
    unsigned char ydiv;
    signed short wbase;
    unsigned char wmult;
    unsigned char wdiv;
    signed short hbase;
    unsigned char hmult;
    unsigned char hdiv;
} Calc_Rule;

typedef struct {
    unsigned short value;
    unsigned char type;
    char bank;
    unsigned short param;
    unsigned short x;
    unsigned short y;
    unsigned short w;
    unsigned short h;
    unsigned short unused;
} Ctrl;

typedef struct {
    unsigned char controls;
    unsigned char pid;
    void* first;
    void* calcrule;
    unsigned short unused1;
    unsigned char retctrl;
    unsigned char escctrl;
    char reserved1[4];
    unsigned char focusctrl;
    unsigned char reserved2;
} Ctrl_Group;

typedef struct {
    unsigned char state;
    unsigned short flags;
    unsigned char pid;
    unsigned short x;
    unsigned short y;
    unsigned short w;
    unsigned short h;
    unsigned short xscroll;
    unsigned short yscroll;
    unsigned short wfull;
    unsigned short hfull;
    unsigned short wmin;
    unsigned short hmin;
    unsigned short wmax;
    unsigned short hmax;
    char* icon;
    char* title;
    char* status;
    void* menu;
    void* controls;
    void* toolbar;
    unsigned short toolheight;
    char reserved1[9];
    unsigned char modal;
    char reserved2[140];
} Window;

typedef struct {
    char* text;
    unsigned char color;
    unsigned char color2;
} Ctrl_TFrame;

typedef struct {
    char* text;
    unsigned char color;
    unsigned char flags;
} Ctrl_Text;

typedef struct {
    char* text;
    unsigned char color;
    unsigned char flags;
    char* font;
} Ctrl_Text_Font;

typedef struct {
    char* text;
    unsigned short maxlen;
    char* font;
    unsigned char color;
    unsigned char underline;
} Ctrl_Text_Ctrl;

typedef struct {
    char* icon;
    char* line1;
    char* line2;
    unsigned char flags;
    unsigned char color16;
    unsigned char extflags;
} Ctrl_Icon;

typedef struct {
    char* status;
    char* text;
    unsigned char color;
} Ctrl_Check;

typedef struct {
    char* status;
    char* text;
    unsigned char color;
    unsigned char value;
    char* buffer;
} Ctrl_Radio;

typedef struct {
    unsigned char tabs;
    unsigned char color;
    unsigned char selected;
} Ctrl_Tabs;

typedef struct {
    char* text;
    signed char width;
} Ctrl_Tab;

typedef struct {
    unsigned char type;
    unsigned char unused;
    unsigned short value;
    unsigned short maxvalue;
    unsigned char increment;
    unsigned char decrement;
} Ctrl_Slider;

typedef struct {
    void* controls;
    unsigned short wfull;
    unsigned short hfull;
    unsigned short xscroll;
    unsigned short yscroll;
    unsigned char flags;
} Ctrl_Collection;

typedef struct {
    char* text;
    unsigned short scroll;
    unsigned short cursor;
    signed short selection;
    unsigned short len;
    unsigned short maxlen;
    unsigned char flags;
    unsigned char textcolor;
    unsigned char linecolor;
} Ctrl_Input;

typedef struct {
    char* text;
    unsigned short unused1;
    unsigned short cursor;
    signed short selection;
    unsigned short len;
    unsigned short maxlen;
    unsigned char flags;
    unsigned char textcolor;
    unsigned char unused2;
    char* font;
    unsigned char unused3;
    unsigned short lines;
    signed short wrapwidth;
    unsigned short maxlines;
    signed short xvisible;
    signed short yvisible;
    void* self;
    unsigned short xtotal;
    unsigned short ytotal;
    unsigned short xoffset;
    unsigned short yoffset;
    unsigned char wrapping;
    unsigned char tabwidth;
    int column;
    int line;
    char reserved[4];
} Ctrl_TextBox;

typedef struct {
    unsigned short lines;
    unsigned short scroll;
    void* rowdata;
    unsigned short unused1;
    unsigned char columns;
    unsigned char sorting;
    void* coldata;
    unsigned short clicked;
    unsigned char flags;
    unsigned char unused2;
} List;

typedef struct {
    unsigned char flags;
    unsigned char unused1;
    unsigned short width;
    char* text;
    unsigned short unused2;
} List_Column;

typedef struct {
    unsigned short flags;
    char* value;
} List_Row;

typedef struct {
    unsigned short flags;
    char* value1;
    char* value2;
} List_Row2Col;

typedef struct {
    unsigned short flags;
    char* value1;
    char* value2;
    char* value3;
} List_Row3Col;

typedef struct {
    unsigned short flags;
    char* value1;
    char* value2;
    char* value3;
    char* value4;
} List_Row4Col;

typedef struct {
    unsigned char bytew;
    unsigned char w;
    unsigned char h;
    char* addrData;
    char* addrEncoding;
    unsigned short len;
} Img_Header;

#endif
