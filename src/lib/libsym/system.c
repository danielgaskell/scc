#include <symbos.h>

/* ========================================================================== */
/* System Manager                                                             */
/* ========================================================================== */
void _Sys_Msg(void)  { while(Msg_Send(_sympid, 3, _symmsg) == 0); }
void _Sys_Wait(void) { Idle(); Msg_Receive(_sympid, 3, _symmsg); }

unsigned short App_Run(char bank, char* path, char suppress) {
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
    if (_symmsg[1] == 3)
        _fileerr = _symmsg[8] + 16;
    return _symmsg[1] - 1; // note: -1 from documented codes
}

void App_End(char appID) {
    _symmsg[0] = 17;
    _symmsg[1] = appID;
    _Sys_Msg();
}

unsigned short App_Search(char bank, char* idstring) {
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

unsigned short App_Service(char bank, char* idstring) {
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

void App_Release(char appID) {
    _symmsg[0] = 30;
    _symmsg[3] = appID;
    _symmsg[4] = 2;
    _Sys_Msg();
    while (_symmsg[0] != 158)
        _Sys_Wait();
}
