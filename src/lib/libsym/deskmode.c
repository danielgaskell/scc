#include <symbos.h>
#include "deskmsg.h"

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
