#include <symbos.h>

/* ========================================================================== */
/* System Manager                                                             */
/* ========================================================================== */
void _Sys_Msg(void)  { Msg_Send (_sympid, 3, _symmsg); }
void _Sys_Wait(void) { Msg_Sleep(_sympid, 3, _symmsg); }

unsigned short Prog_Run(char bank, char* path, char suppress) {
    if (suppress)
        bank |= 0x80;
    _symmsg[0] = 16;
    *((char**)(_symmsg + 1)) = path;
	_symmsg[3] = bank;
    _Sys_Msg();
    while (_symmsg[0] != 144)
        _Sys_Wait();
    if (_symmsg[1] == 0)
        return *((unsigned short*)(_symmsg + 8)); // includes both pid and appid!
    return _symmsg[1] - 1; // note: -1 from documented codes
}

void Prog_End(char appID) {
    _symmsg[0] = 17;
    _symmsg[1] = appID;
    _Sys_Msg();
}

unsigned short Prog_Search(char bank, char* idstring) {
    _symmsg[0] = 30;
    *((char**)(_symmsg + 1)) = idstring;
    _symmsg[3] = bank;
    _symmsg[4] = 0;
    _Sys_Msg();
    while (_symmsg[0] != 158)
        _Sys_Wait();
    if (_symmsg[1] == 0)
        return *((unsigned short*)(_symmsg + 8)); // includes both pid and appid!
    return 0;
}

unsigned short Prog_SearchStart(char bank, char* idstring) {
    _symmsg[0] = 30;
    *((char**)(_symmsg + 1)) = idstring;
    _symmsg[3] = bank;
    _symmsg[4] = 1;
    _Sys_Msg();
    while (_symmsg[0] != 158)
        _Sys_Wait();
    if (_symmsg[1] == 0)
        return *((unsigned short*)(_symmsg + 8)); // includes both pid and appid!
    return _symmsg[1] - 1; // note: -1 from documented codes
}

void Prog_Release(char bank, char* idstring) {
    _symmsg[0] = 30;
    *((char**)(_symmsg + 1)) = idstring;
    _symmsg[3] = bank;
    _symmsg[4] = 2;
    _Sys_Msg();
    while (_symmsg[0] != 158)
        _Sys_Wait();
}

void Run_Dialogue(char appID) {
    _symmsg[0] = 20;
    _symmsg[1] = appID;
    _Sys_Msg();
}

void Run_Control(char module) {
    _symmsg[0] = 20;
    _symmsg[1] = module;
    _Sys_Msg();
}

void Run_TaskMgr(void) {
    _symmsg[0] = 25;
    _Sys_Msg();
}

void Run_Security(void) {
    _symmsg[0] = 21;
    _Sys_Msg();
}

void Run_Shutdown(void) {
    _symmsg[0] = 22;
    _Sys_Msg();
}

void Sys_Shutdown(void) {
    _symmsg[0] = 23;
    _Sys_Msg();
}

void Sys_Config(char command) {
    _symmsg[0] = 28;
    _symmsg[1] = command;
    _Sys_Msg();
}

typedef struct {
	char *line1;
	unsigned int pen1;
	char *line2;
	unsigned int pen2;
	char *line3;
	unsigned int pen3;
	char *icon;
} MSGBOX;

MSGBOX _msgbox;

unsigned char MsgBox(char* line1, char* line2, char* line3, unsigned int pen, unsigned char type, char* icon) {
    pen = pen*4 + 2;
    _msgbox.line1 = line1;
	_msgbox.pen1 = pen;
	_msgbox.line2 = line2;
	_msgbox.pen2 = pen;
	_msgbox.line3 = line3;
	_msgbox.pen3 = pen;
	_msgbox.icon = icon;
	_symmsg[0] = 29;
    *((char**)(_symmsg + 1)) = (char*)&_msgbox;
	_symmsg[3] = _symbank;
	_symmsg[4] = type;
	_Sys_Msg();
    while (_symmsg[0] != 157)
        _Sys_Wait();
    return _symmsg[1];
}

// returns: 0 on success, other errors as documented
unsigned char FileBox(unsigned char winID, unsigned char flags, unsigned char attribs, unsigned char bank, char* buffer) {
    _symmsg[0] = 31;
    _symmsg[6] = flags | bank;
    _symmsg[7] = attribs;
    *((char**)(_symmsg + 8)) = (char*)&buffer;
    _symmsg[10] = 0x80; // 128 entries
    _symmsg[11] = 0;
    _symmsg[12] = 0;    // 8k buffer
    _symmsg[13] = 0x20;
    _Sys_Msg();
    while (_symmsg[0] != 159)
        _Sys_Wait();
    if (_symmsg[1] != -1)       // initial open failed
        return _symmsg[1];
    if (winID) {
        // FIXME set as modal, then unset
    }
    _symmsg[0] = 0;
    while (_symmsg[0] != 159)   // wait for result
        _Sys_Wait();
    return _symmsg[1];
}
