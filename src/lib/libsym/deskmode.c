#include <symbos.h>
#include "deskmsg.h"

void Screen_Mode_Set(char mode, char force, char vwidth) {
    _msemaon();
    _symmsg[0] = 48;
    _symmsg[1] = 2;
    _symmsg[2] = mode | (force << 7);
    _symmsg[3] = vwidth;
    _Desk_Msg();
    _msemaoff();
}

unsigned short Color_Get(char color) {
    unsigned short result;
    _msemaon();
    _symmsg[2] = color;
    _Desk_Service(3);
    result = *((unsigned short*)(_symmsg + 3));
    _msemaoff();
    return result;
}

void Color_Set(char color, unsigned short value) {
    _msemaon();
    _symmsg[0] = 48;
    _symmsg[1] = 4;
    _symmsg[2] = color;
    *((unsigned short*)(_symmsg + 3)) = value;
    _Desk_Msg();
    _msemaoff();
}

void Screen_Redraw(void) {
    _msemaon();
    _symmsg[0] = 48;
    _symmsg[1] = 9;
    _Desk_Msg();
    _msemaoff();
}
