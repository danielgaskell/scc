#define MSC_WDG_SIZE    188
#define MSC_WDG_CLICK   189
#define MSC_WDG_PROP    190
#define MSR_DSK_EXTDSK  191
#define FNC_DXT_WDGOKY  8

extern void Widget_Init(Ctrl_Collection* col, char* msg, unsigned char send_pid);

extern unsigned short _widget_sizes[4][2];
extern unsigned char _widget_win;
extern unsigned char _widget_col;
extern unsigned short _widget_w;
extern unsigned short _widget_h;
