#define ERR_NOSOUND 51

#define SOUND_NONE 0
#define SOUND_PSG 1
#define SOUND_OPL4 2

#define FX_FORCE 1
#define FX_OPTCH 2
#define FX_ANY 3
#define FX_OPTIONAL 4
#define FX_ONLY 5

#define FX_PLAY 1
#define FX_SINGLE 2
#define FX_SOLO 3

#define PAN_LEFT 0
#define PAN_MIDDLE 127
#define PAN_RIGHT 255

#define FX_CLICK1 1
#define FX_CLICK2 2
#define FX_BEEP1 3
#define FX_BEEP2 4
#define FX_RING1 5
#define FX_RING2 6
#define FX_ALERT1 7
#define FX_ALERT2 8
#define FX_SLIDE1 9
#define FX_SLIDE2 10
#define FX_RAISE 11
#define FX_LOWER 12
#define FX_POPUP 13
#define FX_SHRINK 14
#define FX_TIC1 15
#define FX_TIC2 16
#define FX_SHOOT 17
#define FX_EXPLODE 18
#define FX_STEP 19
#define FX_LOSE 20
#define FX_WIN 21
#define FX_CAR 22
#define FX_PLANE 23

signed char Sound_Init(void);
signed char Music_Load(unsigned char fid, unsigned char hw);
signed char Music_Load_Mem(unsigned char bank, char* addr, unsigned short len);
void Music_Free(void);
void Music_Start(unsigned char track);
void Music_Stop(void);
void Music_Continue(void);
void Music_Volume(unsigned char vol);
signed char Effect_Load(unsigned char fid, unsigned char hw);
signed char Effect_Load_Mem(unsigned char bank, char* addr, unsigned short len);
void Effect_Free(unsigned char handle);
void Effect_Play(unsigned char handle, unsigned char id, unsigned char volume,
                 unsigned char priority, unsigned char pan, int pitch);
void Effect_Stop(unsigned char handle, unsigned char id);
