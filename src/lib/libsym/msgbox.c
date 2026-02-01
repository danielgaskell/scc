#include <symbos.h>

typedef struct {
	char *line1;
	unsigned int pen1;
	char *line2;
	unsigned int pen2;
	char *line3;
	unsigned int pen3;
	char *icon;
	char *icon4;
	char *icon16;
} MSGBOX;

_transfer MSGBOX _msgbox;

unsigned char MsgBox(char* line1, char* line2, char* line3, unsigned int pen, unsigned char type, char* icon4, char* icon16, void* modalWin) {
    unsigned char result;
    pen = pen*4 + 2;
    if (modalWin)
        type |= MSGBOX_MODAL;
    _msgbox.line1 = line1 ? line1 : "";
	_msgbox.pen1 = pen;
	_msgbox.line2 = line2 ? line2 : "";
	_msgbox.pen2 = pen;
	_msgbox.line3 = line3 ? line3 : "";
	_msgbox.pen3 = pen;
	if (icon16) {
        _msgbox.icon = 0;
        _msgbox.icon4 = icon4;
        _msgbox.icon16 = icon16;
	} else {
        _msgbox.icon = icon4;
	}
	_msemaon();
	_symmsg[0] = 29;
    *((char**)(_symmsg + 1)) = (char*)&_msgbox;
	_symmsg[3] = _symbank;
	_symmsg[4] = type;
	Msg_Respond(_threadpid(), 3, _symmsg);
    if (modalWin != 0 && _symmsg[1] == 1) {
        ((Window*)modalWin)->modal = _symmsg[2];
        Msg_Wait(_threadpid(), 3, _symmsg, 157); // wait for response
        ((Window*)modalWin)->modal = 0;
    }
    result = _symmsg[1];
    _msemaoff();
    return result;
}
