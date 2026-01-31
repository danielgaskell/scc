#include <symbos.h>

unsigned short App_Run(char bank, char* path, char suppress) {
    unsigned short result;
    if (suppress)
        bank |= 0x80;
    _msemaon();
    _symmsg[0] = 16;
    *((char**)(_symmsg + 1)) = path;
	_symmsg[3] = bank;
	Msg_Respond(_threadpid(), 3, _symmsg);
    if (_symmsg[1] == 0) {
        result = *((unsigned short*)(_symmsg + 8)); // includes both pid and appid!
        _msemaoff();
        return result;
    }
    if (_symmsg[1] == 3)
        _fileerr = _symmsg[8] + 16;
    result = _symmsg[1] - 1; // note: -1 from documented codes
    _msemaoff();
    return result;
}

