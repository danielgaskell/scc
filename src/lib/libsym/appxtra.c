#include <symbos.h>

/* ========================================================================== */
/* System Manager                                                             */
/* ========================================================================== */
void _Sys_Msg(void) { Msg_Respond(_threadpid(), 3, _symmsg); }

void App_End(char appID) {
    _msemaon();
    _symmsg[0] = 17;
    _symmsg[1] = appID;
    Msg_Send(_threadpid(), 3, _symmsg);
    _msemaoff();
}

unsigned short App_Search(char bank, char* idstring) {
    unsigned short result;
    _msemaon();
    _symmsg[0] = 30;
    *((char**)(_symmsg + 1)) = idstring;
    _symmsg[3] = bank;
    _symmsg[4] = 0;
	_Sys_Msg();
    if (_symmsg[1] == 0) {
        result = *((unsigned short*)(_symmsg + 8)); // includes both pid and appid!
        _msemaoff();
        return result;
    }
    _msemaoff();
    return 0;
}

unsigned short App_Service(char bank, char* idstring) {
    unsigned short result;
    _msemaon();
    _symmsg[0] = 30;
    *((char**)(_symmsg + 1)) = idstring;
    _symmsg[3] = bank;
    _symmsg[4] = 1;
	_Sys_Msg();
    if (_symmsg[1] == 0) {
        result = *((unsigned short*)(_symmsg + 8)); // includes both pid and appid!
        _msemaoff();
        return result;
    }
    result = _symmsg[1] - 1; // note: -1 from documented codes
    _msemaoff();
    return result;
}

void App_Release(char appID) {
    _msemaon();
    _symmsg[0] = 30;
    _symmsg[3] = appID;
    _symmsg[4] = 2;
	_Sys_Msg();
    _msemaoff();
}
