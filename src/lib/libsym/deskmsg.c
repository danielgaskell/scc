#include <symbos.h>

void _Desk_Msg(void)  {
    Msg_Send (_sympid, 2, _symmsg);
}

void _Desk_Wait(void) {
    Msg_Sleep(_sympid, 2, _symmsg);
}
