extern unsigned char _Gfx_Load(char* filename, char* buffer);

extern void* _gfx_pix;
extern void _gfx_pix4(unsigned short x, unsigned char y, unsigned char color);
extern void _gfx_pix16(unsigned short x, unsigned char y, unsigned char color);

extern void* _gfx_val;
extern unsigned char _gfx_val4(unsigned short x, unsigned char y);
extern unsigned char _gfx_val16(unsigned short x, unsigned char y);

extern void* _gfx_hline;
extern void _gfx_hline4(unsigned short x, unsigned char y, unsigned short w, unsigned char color);
extern void _gfx_hline16(unsigned short x, unsigned char y, unsigned short w, unsigned char color);

extern void* _gfx_vline;
extern void _gfx_vline4(unsigned short x, unsigned char y, unsigned char h, unsigned char color);
extern void _gfx_vline16(unsigned short x, unsigned char y, unsigned char h, unsigned char color);

extern void* _gfx_put;
extern void _gfx_put4(char* image, unsigned short x, unsigned char y, unsigned char mode);
extern void _gfx_put16(char* image, unsigned short x, unsigned char y, unsigned char mode);

extern void* _gfx_get;
extern void _gfx_get4(char* image, unsigned short x, unsigned char y, unsigned char w, unsigned char h);
extern void _gfx_get16(char* image, unsigned short x, unsigned char y, unsigned char w, unsigned char h);

extern char* _gfx_pixtab;
extern char* _gfx_active;
extern char* _gfx_activedat;
extern unsigned char _gfx_activebw;
extern unsigned short _gfx_activew;
extern unsigned char _gfx_activeh;
extern unsigned char _screencolors;
