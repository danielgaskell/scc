#include <symbos.h>
#include "deskmsg.h"

void Screen_Mode_Set(char mode, char force, char vwidth) {
    _symmsg[0] = 48;
    _symmsg[1] = 2;
    _symmsg[2] = mode | (force << 7);
    _symmsg[3] = vwidth;
    _Desk_Msg();
}

unsigned short Color_Get(char color) {
    _symmsg[2] = color;
    _Desk_Service(3);
    return *((unsigned short*)(_symmsg + 3));
}

void Color_Set(char color, unsigned short value) {
    _symmsg[0] = 48;
    _symmsg[1] = 4;
    _symmsg[2] = color;
    *((unsigned short*)(_symmsg + 3)) = value;
    _Desk_Msg();
}

void Screen_Redraw(void) {
    _symmsg[0] = 48;
    _symmsg[1] = 9;
    _Desk_Msg();
}
