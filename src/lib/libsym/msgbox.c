#include <symbos.h>

typedef struct {
	char *line1;
	unsigned int pen1;
	char *line2;
	unsigned int pen2;
	char *line3;
	unsigned int pen3;
	char *icon;
} MSGBOX;

_transfer MSGBOX _msgbox;

unsigned char MsgBox(char* line1, char* line2, char* line3, unsigned int pen, unsigned char type, char* icon, void* modalWin) {
    unsigned char result;
    pen = pen*4 + 2;
    if (modalWin)
        type |= MSGBOX_MODAL;
    _msgbox.line1 = line1;
	_msgbox.pen1 = pen;
	_msgbox.line2 = line2;
	_msgbox.pen2 = pen;
	_msgbox.line3 = line3;
	_msgbox.pen3 = pen;
	_msgbox.icon = icon;
	_msemaon();
	_symmsg[0] = 29;
    *((char**)(_symmsg + 1)) = (char*)&_msgbox;
	_symmsg[3] = _symbank;
	_symmsg[4] = type;
	while (Msg_Send(_sympid, 3, _symmsg) == 0);
    while (_symmsg[0] != 157) {
        Idle();
        Msg_Receive(_sympid, 3, _symmsg);
    }
    if (modalWin != 0 && _symmsg[1] == 1) {
        ((Window*)modalWin)->modal = _symmsg[2];
        _symmsg[0] = 0;
        while (_symmsg[0] != 157) {
            Idle();
            Msg_Receive(_sympid, 3, _symmsg);
        }
        ((Window*)modalWin)->modal = 0;
    }
    result = _symmsg[1];
    _msemaoff();
    return result;
}
