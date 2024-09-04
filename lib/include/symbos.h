#ifndef _SYMBOS
#define _SYMBOS

/* ========================================================================== */
/* Useful codes and bitmasks                                                  */
/* ========================================================================== */
#define SEEK_SET 0
#define SEEK_CUR 1
#define SEEK_END 2

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

#define ATTRIB_READONLY 1
#define ATTRIB_HIDDEN 2
#define ATTRIB_SYSTEM 4
#define ATTRIB_VOLUME 8
#define ATTRIB_DIR 16
#define ATTRIB_ARCHIVE 32

#define TIME_MODIFIED 1
#define TIME_CREATED 2

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
#define ERR_NOPROC 14
#define ERR_DEVFULL 13
#define ERR_RINGFULL 12
#define ERR_MOREPROC 11
#define ERR_NOSHELL 10

#define APPERR_NOFILE 0
#define APPERR_UNKNOWN 1
#define APPERR_LOAD 2
#define APPERR_NOMEM 3

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

/* ========================================================================== */
/* SymbOS messages                                                            */
/* ========================================================================== */
#define MSC_GEN_QUIT    0
#define MSC_GEN_FOCUS   255
#define MSC_KRL_MTADDP  1
#define MSC_KRL_MTDELP  2
#define MSC_KRL_MTADDT  3
#define MSC_KRL_MTDELT  4
#define MSC_KRL_MTSLPP  5
#define MSC_KRL_MTWAKP  6
#define MSC_KRL_TMADDT  7
#define MSC_KRL_TMDELT  8
#define MSC_KRL_TMDELP  9
#define MSC_KRL_MTPRIO  10
#define MSR_KRL_MTADDP  129
#define MSR_KRL_MTDELP  130
#define MSR_KRL_MTADDT  131
#define MSR_KRL_MTDELT  132
#define MSR_KRL_MTSLPP  133
#define MSR_KRL_MTWAKP  134
#define MSR_KRL_TMADDT  135
#define MSR_KRL_TMDELT  136
#define MSR_KRL_TMDELP  137
#define MSR_KRL_MTPRIO  138
#define MSC_SYS_PRGRUN  16
#define MSC_SYS_PRGEND  17
#define MSC_SYS_SYSWNX  18
#define MSC_SYS_SYSWPR  19
#define MSC_SYS_PRGSTA  20
#define MSC_SYS_SYSSEC  21
#define MSC_SYS_SYSQIT  22
#define MSC_SYS_SYSOFF  23
#define MSC_SYS_PRGSET  24
#define MSC_SYS_PRGTSK  25
#define MSC_SYS_SYSFIL  26
#define MSC_SYS_SYSHLP  27
#define MSC_SYS_SYSCFG  28
#define MSC_SYS_SYSWRN  29
#define MSC_SYS_PRGSRV  30
#define MSC_SYS_SELOPN  31
#define MSR_SYS_PRGRUN  144
#define MSR_SYS_SYSFIL  154
#define MSR_SYS_SYSWRN  157
#define MSR_SYS_PRGSRV  158
#define MSR_SYS_SELOPN  159
#define MSC_DSK_WINOPN  32
#define MSC_DSK_WINMEN  33
#define MSC_DSK_WININH  34
#define MSC_DSK_WINTOL  35
#define MSC_DSK_WINTIT  36
#define MSC_DSK_WINSTA  37
#define MSC_DSK_WINMVX  38
#define MSC_DSK_WINMVY  39
#define MSC_DSK_WINTOP  40
#define MSC_DSK_WINMAX  41
#define MSC_DSK_WINMIN  42
#define MSC_DSK_WINMID  43
#define MSC_DSK_WINMOV  44
#define MSC_DSK_WINSIZ  45
#define MSC_DSK_WINCLS  46
#define MSC_DSK_WINDIN  47
#define MSC_DSK_DSKSRV  48
#define MSC_DSK_WINSLD  49
#define MSC_DSK_WINPIN  50
#define MSC_DSK_WINSIN  51
#define MSC_DSK_MENCTX  52
#define MSC_DSK_STIADD  53
#define MSC_DSK_STIREM  54
#define MSC_DSK_STIUPD  55
#define MSC_DSK_CONPOS  56
#define MSC_DSK_CONSIZ  57
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
#define MSR_DSK_EXTDSK  191
#define MSC_WDG_SIZE    188
#define MSC_WDG_CLICK   189
#define MSC_WDG_PROP    190
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
#define DSK_SRV_MODGET  1
#define DSK_SRV_MODSET  2
#define DSK_SRV_COLGET  3
#define DSK_SRV_COLSET  4
#define DSK_SRV_DSKSTP  5
#define DSK_SRV_DSKCNT  6
#define DSK_SRV_DSKPNT  7
#define DSK_SRV_DSKBGR  8
#define DSK_SRV_DSKPLT  9
#define DSK_SRV_DSKOPN  11
#define DSK_SRV_SCRCNV  12
#define DSK_SRV_DSKBIN  13

/* ========================================================================== */
/* Internal features                                                          */
/* ========================================================================== */
#define HEAP_ALIGN 8

typedef struct {
    unsigned char bank;
    char* addr;
    unsigned short len;
} _Memory;

typedef struct {
    unsigned short codelen;
    unsigned short datalen;
    unsigned short translen;
    char* segdata;
    char* segtrans;
    unsigned char timers[4];
    unsigned char bank;
    _Memory memory[8];
    unsigned char appid;
    unsigned char pid;
    char iconsmall[19];
    char iconbig[147];
} _SymHeader;

extern _SymHeader _symheader;

extern char* _symmsg;
extern unsigned char _sympid;
extern unsigned char _symappid;
extern unsigned char _symbank;
extern char* _segcode;
extern char* _segdata;
extern char* _segtrans;
extern unsigned short _segcodelen;
extern unsigned short _segdatalen;
extern unsigned short _segtranslen;

extern unsigned char _fileerr;
extern unsigned char _shellerr;
extern unsigned char _shellpid;
extern unsigned char _shellwidth;
extern unsigned char _shellheight;
extern unsigned char _shellver;
extern int _argc;
extern char _argv[];

extern unsigned short Msg_Sleep(char rec_pid, char send_pid, char* msg);
extern unsigned char Msg_Send(char rec_pid, char send_pid, char* msg);
extern unsigned short Msg_Receive(char rec_pid, char send_pid, char* msg);
extern void Idle(void);

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

extern unsigned char Clip_Put(unsigned char bank, char* addr, unsigned short len, unsigned char type);
extern unsigned short Clip_Get(unsigned char bank, char* addr, unsigned short len, unsigned char type);
extern unsigned char Clip_Type(void);
extern unsigned short Clip_Len(void);

extern unsigned long Sys_Counter(void);
extern unsigned short Sys_IdleCount(void);

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

extern unsigned short Text_Width(unsigned char bank, char* addr, int maxlen);
extern unsigned char Text_Height(unsigned char bank, char* addr, int maxlen);

typedef struct {
    unsigned char second;
    unsigned char minute;
    unsigned char hour;
    unsigned char day;
    unsigned char month;
    unsigned short year;
    signed char timezone;
} SymTime;

extern void Time_Get(SymTime* addr);
extern void Time_Set(SymTime* addr);

/* ========================================================================== */
/* System Manager                                                             */
/* ========================================================================== */
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

extern char FileBoxExt[4];
extern char FileBoxPath[256];

extern unsigned char MsgBox(char* line1, char* line2, char* line3, unsigned int pen,
                            unsigned char type, char* icon, void* modalWin);
extern unsigned char FileBox(char* path, char* filter, unsigned char flags, unsigned char attribs,
                             unsigned short entries, unsigned short bufsize, void* modalWin);

extern unsigned char File_Command(void);
extern unsigned char File_New(unsigned char bank, char* path, unsigned char attrib);
extern unsigned char File_Open(unsigned char bank, char* path);
extern unsigned char File_Close(unsigned char id);
extern unsigned short File_Read(unsigned char id, unsigned char bank, char* addr, unsigned short len);
extern unsigned char File_ReadLine(unsigned char id, unsigned char bank, char* addr);
extern unsigned short File_Write(unsigned char id, unsigned char bank, char* addr, unsigned short len);
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

/* ========================================================================== */
/* Desktop Manager                                                            */
/* ========================================================================== */
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

extern int Menu_Context(unsigned char bank, char* addr, int x, int y);

extern signed char Systray_Add(unsigned char bank, char* addr, unsigned char code);
extern void Systray_Remove(unsigned char id);
extern char Select_Pos(unsigned short* x, unsigned short* y, unsigned short w, unsigned short h);
extern char Select_Size(unsigned short x, unsigned short y, unsigned short* w, unsigned short* h);
extern void Screen_Mode_Set(char mode, char force, char vwidth);
#define Colour_Get(x) Color_Get(x)
extern unsigned short Color_Get(char color);
#define Colour_Set(x, y) ColorSet(x, y)
extern void Color_Set(char color, unsigned short value);
extern void Screen_Redraw(void);

extern unsigned char TextBox_Pos(void* win, void* textbox);
extern void TextBox_Redraw(void* win, void* textbox);
extern void TextBox_Select(void* win, void* textbox, int cursor, int selection);

/* ========================================================================== */
/* Desktop Data Records                                                       */
/* ========================================================================== */
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
    unsigned char h;
    unsigned char w;
    char* addrData;
    char* addrEncoding;
    unsigned short len;
} Img_Header;

/* ========================================================================== */
/* SymShell                                                                   */
/* ========================================================================== */
extern int Shell_CharIn(unsigned char channel);
extern signed char Shell_StringIn(unsigned char channel, unsigned char bank, char* addr);
extern signed char Shell_CharOut(unsigned char channel, unsigned char val);
extern signed char Shell_StringOut(unsigned char channel, unsigned char bank,
                                   char* addr, unsigned char len);
extern signed char Shell_Print(char* addr);
extern void Shell_Exit(unsigned char type);
extern void Shell_PathAdd(unsigned char bank, char* path, char* addition, char* dest);
extern int Shell_CharTest(unsigned char channel, unsigned char lookahead);

/* ========================================================================== */
/* Time utility functions                                                     */
/* ========================================================================== */
extern void Time2Obj(unsigned long timestamp, SymTime* obj);
extern unsigned long Obj2Time(SymTime* obj);

#endif
