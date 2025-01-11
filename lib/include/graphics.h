#define PUT_SET 1
#define PUT_AND 2
#define PUT_OR 4
#define PUT_XOR 8

extern void Gfx_ScreenInit(void);
extern void Gfx_Clear(char* canvas, unsigned char color);
extern void Gfx_Init(char* canvas, unsigned short w, unsigned char h);
extern void Gfx_Select(char* canvas);
extern void Gfx_Prep(char* buffer);
extern void Gfx_Prep_Set(char* buffer);
extern unsigned char Gfx_Load(char* filename, char* buffer);
extern unsigned char Gfx_Load_Set(char* filename, char* buffer);
extern unsigned char Gfx_Save(char* filename, char* buffer);
extern void Gfx_Pixel(unsigned short x, unsigned char y, unsigned char color);
extern unsigned char Gfx_Value(unsigned short x, unsigned char y);
extern void Gfx_Safe_Pixel(unsigned short x, unsigned short y, unsigned char color);
extern void Gfx_Line(unsigned short x0, unsigned char y0, unsigned short x1, unsigned char y1, unsigned char color);
extern void Gfx_LineB(unsigned short x0, unsigned char y0, unsigned short x1, unsigned char y1, unsigned char color, unsigned char bits);
extern void Gfx_HLine(unsigned short x, unsigned char y, unsigned short w, unsigned char color);
extern void Gfx_VLine(unsigned short x, unsigned char y, unsigned short h, unsigned char color);
extern void Gfx_Put(char* image, unsigned short x, unsigned char y, unsigned char mode);
extern void Gfx_Put_Set(char* image, unsigned short x, unsigned char y, unsigned char mode, unsigned char tile);
extern void Gfx_Get(char* image, unsigned short x, unsigned char y, unsigned char w, unsigned char h);
extern void Gfx_Box(unsigned short x0, unsigned char y0, unsigned short x1, unsigned short y1, unsigned char color);
extern void Gfx_BoxF(unsigned short x0, unsigned char y0, unsigned short x1, unsigned short y1, unsigned char color);
extern void Gfx_Circle(signed short x0, signed short y0, signed short radius, unsigned char color);
extern void Gfx_Text(unsigned short x, unsigned char y, char* text, unsigned char color, char* font);
