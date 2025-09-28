#include <symbos.h>
#include "deskmsg.h"

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

void Win_ContentX(unsigned char winID, unsigned short newX) {
    _msemaon();
    _symmsg[0] = 38;
    _symmsg[1] = winID;
    *((unsigned short*)(_symmsg + 2)) = newX;
    _Desk_Msg();
    _msemaoff();
}

void Win_ContentY(unsigned char winID, unsigned short newY) {
    _msemaon();
    _symmsg[0] = 39;
    _symmsg[1] = winID;
    *((unsigned short*)(_symmsg + 2)) = newY;
    _Desk_Msg();
    _msemaoff();
}
