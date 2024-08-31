#include <symbos.h>

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
    Msg_Send(_sympid, 3, _symmsg);
    while (_symmsg[0] != 159)
        Msg_Sleep(_sympid, 3, _symmsg);
    if (_symmsg[1] != -1)       // initial open failed
        return _symmsg[1];
    if (winID) {
        // FIXME set as modal, then unset
    }
    _symmsg[0] = 0;
    while (_symmsg[0] != 159)   // wait for result
        Msg_Sleep(_sympid, 3, _symmsg);
    return _symmsg[1];
}
