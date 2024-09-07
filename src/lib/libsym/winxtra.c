#include <symbos.h>
#include "deskmsg.h"

void Win_Redraw_Menu(unsigned char winID) {
    _msemaon();
    _symmsg[0] = 33;
    _symmsg[1] = winID;
    _Desk_Msg();
    _msemaoff();
}

void Win_Redraw_Toolbar(unsigned char winID, signed char what, unsigned char first) {
    _msemaon();
    _symmsg[0] = 35;
    _symmsg[1] = winID;
    _symmsg[2] = what;
    _symmsg[3] = first;
    _Desk_Msg();
    _msemaoff();
}

void Win_Redraw_Title(unsigned char winID) {
    _msemaon();
    _symmsg[0] = 36;
    _symmsg[1] = winID;
    _Desk_Msg();
    _msemaoff();
}

void Win_Redraw_Status(unsigned char winID) {
    _msemaon();
    _symmsg[0] = 37;
    _symmsg[1] = winID;
    _Desk_Msg();
    _msemaoff();
}

void Win_Focus(unsigned char winID) {
    _msemaon();
    _symmsg[0] = 40;
    _symmsg[1] = winID;
    _Desk_Msg();
    _msemaoff();
}

void Win_Maximize(unsigned char winID) {
    _msemaon();
    _symmsg[0] = 41;
    _symmsg[1] = winID;
    _Desk_Msg();
    _msemaoff();
}

void Win_Minimize(unsigned char winID) {
    _msemaon();
    _symmsg[0] = 42;
    _symmsg[1] = winID;
    _Desk_Msg();
    _msemaoff();
}

void Win_Restore(unsigned char winID) {
    _msemaon();
    _symmsg[0] = 43;
    _symmsg[1] = winID;
    _Desk_Msg();
    _msemaoff();
}

void Win_Move(unsigned char winID, unsigned short newX, unsigned short newY) {
    _msemaon();
    _symmsg[0] = 44;
    _symmsg[1] = winID;
    *((unsigned short*)(_symmsg + 2)) = newX;
    *((unsigned short*)(_symmsg + 4)) = newY;
    _Desk_Msg();
    _msemaoff();
}

void Win_Resize(unsigned char winID, unsigned short newW, unsigned short newH) {
    _msemaon();
    _symmsg[0] = 45;
    _symmsg[1] = winID;
    *((unsigned short*)(_symmsg + 2)) = newW;
    *((unsigned short*)(_symmsg + 4)) = newH;
    _Desk_Msg();
    _msemaoff();
}

void Win_Redraw_Ext(unsigned char winID, unsigned char what, unsigned char first) {
    _msemaon();
    _symmsg[0] = 47;
    _symmsg[1] = winID;
    _symmsg[2] = what;
    _symmsg[3] = first;
    _Desk_Msg();
    _msemaoff();
}

void Win_Redraw_Slider(unsigned char winID) {
    _msemaon();
    _symmsg[0] = 49;
    _symmsg[1] = winID;
    _Desk_Msg();
    _msemaoff();
}

void Win_Redraw_Area(unsigned char winID, unsigned char what, unsigned char first,
                    unsigned short x, unsigned short y, unsigned short w, unsigned short h) {
    _msemaon();
    _symmsg[0] = 50;
    _symmsg[1] = winID;
    _symmsg[2] = what;
    _symmsg[3] = first;
    *((unsigned short*)(_symmsg + 4)) = x;
    *((unsigned short*)(_symmsg + 6)) = y;
    *((unsigned short*)(_symmsg + 8)) = w;
    *((unsigned short*)(_symmsg + 10)) = h;
    _Desk_Msg();
    _msemaoff();
}

void Win_Redraw_Sub(unsigned char winID, unsigned char collection, unsigned char control) {
    _msemaon();
    _symmsg[0] = 51;
    _symmsg[1] = winID;
    _symmsg[2] = collection;
    _symmsg[3] = control;
    _Desk_Msg();
    _msemaoff();
}
