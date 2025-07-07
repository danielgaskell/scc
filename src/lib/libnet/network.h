#define MAX_CLIENTS 32
#define CLIENT_NAME_LEN 16

extern char _clientnames[MAX_CLIENTS][CLIENT_NAME_LEN+1];
extern char _clientcolors[];

extern unsigned char _netpid;
extern unsigned char _neterr;

extern char _netmsg[14];
extern char _netpacket[1400];
extern char* _useragent;
extern unsigned short _nettimeout;
extern char _nethost[65];

unsigned char Net_Wait(unsigned char id);
unsigned char Net_Command(void);
signed char Net_SCommand(void);
extern void _nsemaon(void);
extern void _nsemaoff(void);
extern void _packsemaon(void);
extern void _packsemaoff(void);
