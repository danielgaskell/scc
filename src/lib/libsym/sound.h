#define SOUND_MAX_FX 16

extern unsigned char _soundpid;
extern unsigned char _soundhw;
extern unsigned char _soundpref;
extern unsigned char _sounderr;
extern unsigned char _soundmus;
extern unsigned char _soundfx[];
extern unsigned char _soundfxhw[];

unsigned char Sound_Command(void);
