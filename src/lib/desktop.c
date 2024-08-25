#include <symbos.h>

/* ========================================================================== */
/* Desktop Manager                                                            */
/* ========================================================================== */
void _Desk_Msg(void)  { Msg_Send (_sympid, 2, _symmsg); }
void _Desk_Wait(void) { Msg_Sleep(_sympid, 2, _symmsg); }
void _Desk_Service(char id) {
    _symmsg[0] = 48;
    _symmsg[1] = id;
    _Desk_Msg();
    while (_symmsg[0] != 163 && _symmsg[1] != id)
        _Desk_Wait();
}

char Win_Open(unsigned char bank, void* addr) {
    ((Window*)addr)->pid = _sympid;
    if (((Window*)addr)->controls != 0)
        ((Ctrl_Group*)(((Window*)addr)->controls))->pid = _sympid;
    _symmsg[0] = 32;
    _symmsg[1] = bank;
    *((char**)(_symmsg + 2)) = addr;
    _Desk_Msg();
    while (1) {
        _Desk_Wait();
        if (_symmsg[0] == 160) // failure: return -1
            return -1;
        if (_symmsg[0] == 161) // success: return window ID
            return _symmsg[4];
    }
}

void Win_Redraw_Menu(unsigned char winID) {
    _symmsg[0] = 33;
    _symmsg[1] = winID;
    _Desk_Msg();
}

void Win_Redraw(unsigned char winID, unsigned char what, unsigned char first) {
    _symmsg[0] = 34;
    _symmsg[1] = winID;
    _symmsg[2] = what;
    _symmsg[3] = first;
    _Desk_Msg();
}

void Win_Redraw_Toolbar(unsigned char winID, unsigned char what, unsigned char first) {
    _symmsg[0] = 35;
    _symmsg[1] = winID;
    _symmsg[2] = what;
    _symmsg[3] = first;
    _Desk_Msg();
}

void Win_Redraw_Title(unsigned char winID) {
    _symmsg[0] = 36;
    _symmsg[1] = winID;
    _Desk_Msg();
}

void Win_Redraw_Status(unsigned char winID) {
    _symmsg[0] = 37;
    _symmsg[1] = winID;
    _Desk_Msg();
}

void Win_ContentX(unsigned char winID, unsigned short newX) {
    _symmsg[0] = 38;
    _symmsg[1] = winID;
    *((unsigned short*)(_symmsg + 2)) = newX;
    _Desk_Msg();
}

void Win_ContentY(unsigned char winID, unsigned short newY) {
    _symmsg[0] = 39;
    _symmsg[1] = winID;
    *((unsigned short*)(_symmsg + 2)) = newY;
    _Desk_Msg();
}

void Win_Focus(unsigned char winID) {
    _symmsg[0] = 40;
    _symmsg[1] = winID;
    _Desk_Msg();
}

void Win_Maximize(unsigned char winID) {
    _symmsg[0] = 41;
    _symmsg[1] = winID;
    _Desk_Msg();
}

void Win_Minimize(unsigned char winID) {
    _symmsg[0] = 42;
    _symmsg[1] = winID;
    _Desk_Msg();
}

void Win_Restore(unsigned char winID) {
    _symmsg[0] = 43;
    _symmsg[1] = winID;
    _Desk_Msg();
}

void Win_Move(unsigned char winID, unsigned short newX, unsigned short newY) {
    _symmsg[0] = 44;
    _symmsg[1] = winID;
    *((unsigned short*)(_symmsg + 2)) = newX;
    *((unsigned short*)(_symmsg + 4)) = newY;
    _Desk_Msg();
}

void Win_Resize(unsigned char winID, unsigned short newW, unsigned short newH) {
    _symmsg[0] = 45;
    _symmsg[1] = winID;
    *((unsigned short*)(_symmsg + 2)) = newW;
    *((unsigned short*)(_symmsg + 4)) = newH;
    _Desk_Msg();
}

void Win_Close(unsigned char winID) {
    _symmsg[0] = 46;
    _symmsg[1] = winID;
    _Desk_Msg();
}

void Win_Redraw_Ext(unsigned char winID, unsigned char what, unsigned char first) {
    _symmsg[0] = 47;
    _symmsg[1] = winID;
    _symmsg[2] = what;
    _symmsg[3] = first;
    _Desk_Msg();
}

void Win_Redraw_Slider(unsigned char winID) {
    _symmsg[0] = 49;
    _symmsg[1] = winID;
    _Desk_Msg();
}

void Win_Redraw_Area(unsigned char winID, unsigned char what, unsigned char first,
                    unsigned short x, unsigned short y, unsigned short w, unsigned short h) {
    _symmsg[0] = 50;
    _symmsg[1] = winID;
    _symmsg[2] = what;
    _symmsg[3] = first;
    *((unsigned short*)(_symmsg + 4)) = x;
    *((unsigned short*)(_symmsg + 6)) = y;
    *((unsigned short*)(_symmsg + 8)) = w;
    *((unsigned short*)(_symmsg + 10)) = h;
    _Desk_Msg();
}

void Win_Redraw_Sub(unsigned char winID, unsigned char collection, unsigned char control) {
    _symmsg[0] = 51;
    _symmsg[1] = winID;
    _symmsg[2] = collection;
    _symmsg[3] = control;
    _Desk_Msg();
}

short Menu_Context(unsigned char bank, char* addr, short x, char y) {
    _symmsg[0] = 52;
    _symmsg[1] = bank;
    *((char**)(_symmsg + 2)) = addr;
    *((unsigned short*)(_symmsg + 4)) = x;
    *((unsigned short*)(_symmsg + 6)) = y;
    _Desk_Msg();
    while (_symmsg[0] != 168)
        _Desk_Wait();
    if (_symmsg[1] == 0)
        return -1;
    return *((short*)(_symmsg + 2));
}

char Systray_Add(unsigned char bank, char* addr, unsigned char code) {
    _symmsg[0] = 53;
    _symmsg[1] = bank;
    *((char**)(_symmsg + 2)) = addr;
    _symmsg[4] = code;
    _Desk_Msg();
    while (_symmsg[0] != 169)
        _Desk_Wait();
    if (_symmsg[1] == 0)
        return _symmsg[2];
    return -1;
}

void Systray_Remove(unsigned char id) {
    _symmsg[0] = 54;
    _symmsg[1] = id;
    _Desk_Msg();
}

char Select_Pos(short* x, short* y, short w, short h) {
    _symmsg[0] = 56;
    *((unsigned short*)(_symmsg + 2)) = *x;
    *((unsigned short*)(_symmsg + 4)) = *y;
    *((unsigned short*)(_symmsg + 6)) = w;
    *((unsigned short*)(_symmsg + 8)) = h;
    _Desk_Msg();
    while (_symmsg[0] != 172)
        _Desk_Wait();
    if (_symmsg[1] == 0)
        return 0;
    *x = *((short*)(_symmsg + 2));
    *y = *((short*)(_symmsg + 4));
    return 1;
}

char Select_Size(short x, short y, short* w, short* h) {
    _symmsg[0] = 57;
    *((unsigned short*)(_symmsg + 2)) = x;
    *((unsigned short*)(_symmsg + 4)) = y;
    *((unsigned short*)(_symmsg + 6)) = *w;
    *((unsigned short*)(_symmsg + 8)) = *h;
    _Desk_Msg();
    while (_symmsg[0] != 173)
        _Desk_Wait();
    if (_symmsg[1] == 0)
        return 0;
    *w = *((short*)(_symmsg + 2));
    *h = *((short*)(_symmsg + 4));
    return 1;
}

char Desk_Mode(void) {
    _Desk_Service(1);
    return _symmsg[2];
}

void Desk_SetMode(char mode) {
    _symmsg[0] = 48;
    _symmsg[1] = 2;
    _symmsg[2] = mode;
    _Desk_Msg();
}

unsigned short Desk_GetColor(char color) {
    _symmsg[2] = color;
    _Desk_Service(3);
    return *((unsigned short*)(_symmsg + 3));
}

void Desk_SetColor(char color, unsigned short value) {
    _symmsg[0] = 48;
    _symmsg[1] = 4;
    _symmsg[2] = color;
    *((unsigned short*)(_symmsg + 3)) = value;
    _Desk_Msg();
}

void Desk_Redraw_Back(void) {
    _symmsg[0] = 48;
    _symmsg[1] = 8;
    _Desk_Msg();
}

void Desk_Redraw_All(void) {
    _symmsg[0] = 48;
    _symmsg[1] = 9;
    _Desk_Msg();
}
