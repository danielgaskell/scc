#ifndef _SYMCORE
#define _SYMCORE

/* ========================================================================== */
/* Internal features                                                          */
/* ========================================================================== */
extern char* _symmsg;
extern unsigned char _sympid;
extern unsigned char _symappid;
extern unsigned char _symbank;
extern unsigned short _symversion;
extern char* _segcode;
extern char* _segdata;
extern char* _segtrans;
extern unsigned short _segcodelen;
extern unsigned short _segdatalen;
extern unsigned short _segtranslen;
extern unsigned short _debugtrace;
extern unsigned short _debugstack;

extern int _argc;
extern char _argv[];

extern unsigned short Msg_Sleep(char rec_pid, char send_pid, char* msg);
extern unsigned char Msg_Send(char rec_pid, char send_pid, char* msg);
extern unsigned short Msg_Receive(char rec_pid, char send_pid, char* msg);
extern unsigned char Msg_Wait(char rec_pid, char send_pid, char* msg, char id);
extern void Msg_Respond(char rec_pid, char send_pid, char* msg);
extern void Idle(void);

extern unsigned long Sys_Counter(void);
extern unsigned short Sys_Counter16(void);
extern unsigned short Sys_IdleCount(void);

extern signed char _msgpid(void);
extern void _msemaon(void);
extern void _msemaoff(void);

#endif
