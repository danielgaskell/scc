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
