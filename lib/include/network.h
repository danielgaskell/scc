#ifndef _SYMCORE
#include <symbos/core.h>
#endif

#ifndef _SYMNETWORK
#define _SYMNETWORK

#define ERR_NOHW 1
#define ERR_NOIP 2
#define ERR_NOFUNC 3
#define ERR_HARDWARE 4
#define ERR_OFFLINE 5
#define ERR_WIFI 6
#define ERR_NOSOCKET 8
#define ERR_BADSOCKET 9
#define ERR_SOCKETTYPE 10
#define ERR_SOCKETUSED 11
#define ERR_BADDOMAIN 16
#define ERR_TIMEOUT 17
#define ERR_RECURSION 18
#define ERR_TRUNCATED 19
#define ERR_TOOLARGE 20
#define ERR_CONNECT 24

#define TCP_OPENING 1
#define TCP_OPENED 2
#define TCP_CLOSING 3
#define TCP_CLOSED 4

#define DNS_INVALID 0
#define DNS_IP 1
#define DNS_DOMAIN 2

#define PROTO_OTHER 0
#define PROTO_HTTPS 1
#define PROTO_HTTP 2
#define PROTO_FTP 3
#define PROTO_IRC 4
#define PROTO_SFTP 5
#define PROTO_FILE 6
#define PROTO_IMAP 7
#define PROTO_POP 8
#define PROTO_NNTP 9

typedef struct {
    unsigned char status;
    unsigned long ip;
    unsigned short rport;
    unsigned char datarec;
    unsigned short bytesrec;
} NetStat;

typedef struct {
    unsigned short transferred;
    unsigned short remaining;
} TCP_Trans;

extern unsigned char _netpid;
extern unsigned char _neterr;
extern unsigned char _netthread;
extern signed char Net_Init(void);

extern signed char TCP_OpenClient(unsigned long ip, signed short lport, unsigned short rport);
extern signed char TCP_OpenServer(unsigned short lport);
extern signed char TCP_Close(unsigned char handle);
extern void TCP_Event(NetStat* obj);
extern signed char TCP_Status(unsigned char handle, NetStat* obj);
extern signed char TCP_Receive(unsigned char handle, unsigned char bank, char* addr, unsigned short len, TCP_Trans* obj);
extern signed char TCP_Send(unsigned char handle, unsigned char bank, char* addr, unsigned short len);
extern signed char TCP_Skip(unsigned char handle, unsigned short len);
extern signed char TCP_Flush(unsigned char handle);
extern signed char TCP_Disconnect(unsigned char handle);

extern signed char UDP_Open(unsigned char type, unsigned short lport, unsigned char bank);
extern signed char UDP_Close(unsigned char handle);
extern signed char UDP_Status(unsigned char handle, NetStat* obj);
extern signed char UDP_Receive(unsigned char handle, char* addr);
extern signed char UDP_Send(unsigned char handle, char* addr, unsigned short len, unsigned long ip, unsigned short rport);
extern signed char UDP_Skip(unsigned char handle);

extern unsigned long DNS_Resolve(unsigned char bank, char* addr);
extern unsigned char DNS_Verify(unsigned char bank, char* addr);

extern int HTTP_GET(char* url, char* dest, unsigned short maxlen, char* headers, unsigned char bodyonly);

extern void Net_ErrMsg(void* modalWin);
extern signed char Net_SplitURL(char* url, char* host, char** path, int* port);
extern unsigned long Net_PublicIP(void);
extern unsigned char Net_ServerWin(void* modalWin, unsigned char clients, unsigned short port, unsigned short bufsize);

extern char* iptoa(unsigned long ip, char* dest);

#endif

