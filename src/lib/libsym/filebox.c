#include <symbos.h>

_transfer char FileBoxExt[4];
_transfer char FileBoxPath[256];

unsigned char FileBox(char* path, char* filter, unsigned char flags, unsigned char attribs, unsigned short entries, unsigned short bufsize, void* modalWin) {
    // format path buffer as expected
    strcpy(FileBoxExt, filter);
    strcpy(FileBoxPath, path);

    // send message
    _symmsg[0] = 31;
    _symmsg[6] = flags | _symbank;
    _symmsg[7] = attribs;
    *((char**)(_symmsg + 8)) = FileBoxExt;
    *((unsigned short*)(_symmsg + 10)) = entries;
    *((unsigned short*)(_symmsg + 12)) = bufsize;
    Msg_Send(_sympid, 3, _symmsg);
    while (_symmsg[0] != 159)
        Msg_Sleep(_sympid, 3, _symmsg);
    if (_symmsg[1] != 255)       // initial open failed
        return _symmsg[1];
    if (modalWin)
        ((Window*)modalWin)->modal = _symmsg[2];
    _symmsg[0] = 0;
    while (_symmsg[0] != 159)   // wait for result
        Msg_Sleep(_sympid, 3, _symmsg);
    if (modalWin)
        ((Window*)modalWin)->modal = 0;
    return _symmsg[1];
}
